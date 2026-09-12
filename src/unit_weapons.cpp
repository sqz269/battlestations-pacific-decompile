// Weapon devices on a unit instance and the mission Lua bindings that drive
// them. Evidence and coverage: docs/UNIT_WEAPON_DEVICES.md.
//
// Every routine here is a projection of one native body. The shared mission
// binding prologue (native-string scratch, the borrowed Lua state, the call
// frame and the SEH unwind slots) is deliberately absent: it is the same in all
// thirteen bindings and belongs to docs/LUA_BINDING_CORE.md, not here.
#include "bsp/unit_weapons.hpp"

namespace bsp {
namespace {

// 008CF350 caps the seed loop at the five-slot array even when the count field
// says more, and 0081F8B0 never reads the array at all.
int clamped_root_count(int count) noexcept {
    if (count <= 0) {
        return 0;
    }
    return count;
}

} // namespace

// ---------------------------------------------------------------------------
// Reload timers
// ---------------------------------------------------------------------------

int next_ready_barrel_007298d0(const float* timers, int barrel_num, int start) noexcept {
    if (timers == nullptr || barrel_num <= 0) {
        return 0;
    }
    int barrel = start % barrel_num;
    if (barrel < 0) {
        barrel += barrel_num;
    }
    for (int step = 0; step < barrel_num; ++step) {
        const int candidate = (barrel + step) % barrel_num;
        if (timers[candidate] <= 0.0f) {
            return candidate;
        }
    }
    // 00729904 falls through to `mov eax,ebx`: with no ready barrel the routine
    // hands back the caller's unreduced argument, not the wrapped cursor.
    return start;
}

BarrelReloadWrite barrel_reload_write_0072cf00(float time, float sentinel_threshold,
                                               float scaled_rate, bool set_full) noexcept {
    BarrelReloadWrite result{};
    // The rate divide at 0072CF0F..0072CF4D runs only below the threshold, so
    // the sentinel the no-ammo branch writes survives unchanged.
    float value = time;
    if (time < sentinel_threshold && scaled_rate != 0.0f) {
        value = time / scaled_rate;
    }
    result.timer = value;
    if (set_full) {
        result.duration = value;
        result.wrote_duration = true;
    }
    return result;
}

// ---------------------------------------------------------------------------
// Fire stance
// ---------------------------------------------------------------------------

bool stance_allows_fire(FireStance stance) noexcept {
    return stance == FireStance::FreeFire || stance == FireStance::FreeAttack;
}

bool stance_allows_move(FireStance stance) noexcept {
    return stance == FireStance::FreeAttack || stance == FireStance::MoveOnly;
}

// ---------------------------------------------------------------------------
// Enable messages
// ---------------------------------------------------------------------------

WeaponEnableMessage artillery_enable_message_0071dfd0(bool enabled) noexcept {
    WeaponEnableMessage message{};
    message.sub_kind = kWeaponEnableSubKindArtillery;
    message.enabled = enabled;
    return message;
}

WeaponEnableMessage torpedo_enable_message_0071e0d0(bool enabled) noexcept {
    WeaponEnableMessage message{};
    message.sub_kind = kWeaponEnableSubKindTorpedo;
    message.enabled = enabled;
    return message;
}

// ---------------------------------------------------------------------------
// Device lookup: 008CF350
// ---------------------------------------------------------------------------

NativeHandle find_gun_008cf350(UnitWeaponHost& host, NativeHandle unit, int index) {
    if (unit == 0 || !host.class_test(unit, kClassTestUnit)) {
        return 0;
    }
    // Seed: up to five root nodes from unit +3D0h, count at +3CCh. Slots at or
    // past the fifth are pushed as null by the native loop, which is why the
    // walk below has to tolerate a null entry.
    NativeHandle worklist[64] = {};
    int count = 0;
    const int roots = clamped_root_count(host.device_root_count(unit));
    for (int slot = 0; slot < roots && count < 64; ++slot) {
        worklist[count++] =
            slot < kUnitDeviceRootArrayCapacity ? host.device_root(unit, slot) : 0;
    }

    int remaining = index;
    for (int cursor = 0; cursor < count; ++cursor) {
        const NativeHandle node = worklist[cursor];
        if (node == 0) {
            continue;
        }
        // Children first, then the class test on the node itself: the native
        // loop appends every child before testing, so the walk is breadth-first.
        for (NativeHandle child = host.node_first_child(node); child != 0;
             child = host.node_next_sibling(child)) {
            if (count < 64) {
                worklist[count++] = child;
            }
        }
        if (host.class_test(node, kClassTestGun)) {
            remaining -= 1;
            if (remaining < 1) {
                return node;
            }
        }
    }
    return 0;
}

// ---------------------------------------------------------------------------
// Weapon director: 0071BE80
// ---------------------------------------------------------------------------

void director_set_stance_0071be80(UnitWeaponHost& host, NativeHandle director, int stance) {
    if (director == 0) {
        return;
    }
    // Both questions are asked before either answer is applied; the native body
    // spills the first result across the second call.
    const bool allow_fire = host.director_stance_allows_fire(director, stance);
    const bool allow_move = host.director_stance_allows_move(director, stance);
    host.director_apply_fire_permission(director, allow_fire);
    host.director_apply_move_permission(director, allow_move);
}

// ---------------------------------------------------------------------------
// Torpedo stock: 0081F8B0
// ---------------------------------------------------------------------------

void set_torpedo_stock_0081f8b0(UnitWeaponHost& host, NativeHandle unit, int stock) {
    if (unit == 0) {
        return;
    }
    int wanted = stock;
    const int live = host.torpedo_count(unit);
    if (wanted < live) {
        // Fewer than are already out: the spare counter is cleared and the
        // difference is spawned up to the requested number.
        host.unit_set_torpedo_spare(unit, 0);
        if (wanted < 0) {
            wanted = 0;
        }
        int current = host.torpedo_count(unit);
        while (wanted < current) {
            host.torpedo_spawn_one(unit);
            const int next = host.torpedo_count(unit);
            if (next == current) {
                break; // the native loop has no such guard; see the doc's note
            }
            current = next;
        }
    } else {
        host.unit_set_torpedo_spare(unit, wanted - live);
    }

    // Then every direct child that is a torpedo gun has its empty barrels
    // re-armed. Only direct children: this walk does not recurse.
    const float empty = host.gun_reload_empty_threshold();
    for (NativeHandle node = host.node_first_child(unit); node != 0;
         node = host.node_next_sibling(node)) {
        if (!host.class_test(node, kClassTestGun)) {
            continue;
        }
        if (host.gun_weapon_type(node) != kWeaponTypeTorpedo) {
            continue;
        }
        const int barrels = host.gun_barrel_num(node);
        for (int barrel = 0; barrel < barrels; ++barrel) {
            if (host.gun_reload_timer(node, barrel) >= empty) {
                host.gun_rearm_barrel(node, barrel, true);
            }
        }
    }
}

// ---------------------------------------------------------------------------
// The thirteen bindings
// ---------------------------------------------------------------------------

int lua_gun_force_fire_008be440(UnitWeaponHost& host) {
    // Argument 0 is a gun, not a unit: the shipped scripts pass GetGun(...).
    // It arrives through the raw Ptr field reader, not the entity resolver.
    const NativeHandle gun = host.entity_from_lua_ptr_field(0);
    if (gun == 0) {
        return host.lua_result_count();
    }
    const int barrels = host.gun_barrel_num(gun);
    for (int barrel = 0; barrel < barrels; ++barrel) {
        // Zero, not the sentinel: every barrel is declared ready this instant.
        host.gun_set_reload_timer(gun, barrel, 0.0f, false);
    }
    host.gun_fire(gun, 1, host.gun_throw_a(gun), host.gun_throw_b(gun));
    return host.lua_result_count();
}

int lua_gun_force_fire_with_angle_008be600(UnitWeaponHost& host) {
    const NativeHandle gun = host.entity_from_lua_ptr_field(0);
    if (gun == 0) {
        return host.lua_result_count();
    }
    // No reload sweep here: the angle variant fires whatever barrel is ready.
    const float throw_a = host.lua_argument_number(1);
    const float throw_b = host.lua_argument_number(2);
    host.gun_fire(gun, 1, throw_a, throw_b);
    return host.lua_result_count();
}

int lua_has_fired_008be7e0(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    bool fired = false;
    NativeHandle director = 0;
    if (unit != 0 && host.class_test(unit, kClassTestUnit)) {
        director = host.weapon_director(unit);
        if (director == 0) {
            // Fallback owner at +9D4h: a carried or attached unit answers for
            // its parent. The condition guarding it is read from the
            // pseudocode only; the branch layout was not checked in assembly.
            const NativeHandle owner = host.unit_secondary_owner(unit);
            if (owner != 0) {
                director = host.weapon_director(owner);
            }
        }
    }
    if (director != 0) {
        const char* weapon_name = host.lua_argument_string(1);
        const float window = host.lua_argument_number(2);
        fired = host.director_has_fired(director, weapon_name, window);
    }
    host.lua_push_boolean(fired);
    return host.lua_result_count();
}

int lua_unit_free_fire_008a6950(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    director_set_stance_0071be80(host, host.weapon_director(unit),
                                 static_cast<int>(FireStance::FreeFire));
    return host.lua_result_count();
}

int lua_unit_hold_fire_008a6ac0(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    director_set_stance_0071be80(host, host.weapon_director(unit),
                                 static_cast<int>(FireStance::HoldFire));
    return host.lua_result_count();
}

int lua_unit_set_fire_stance_008a6490(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const NativeHandle director = host.weapon_director(unit);
    // The stance integer is read after the director, and is not range-checked
    // by the binding; the two predicates behind the director see it raw.
    const int stance = host.lua_argument_int(1);
    director_set_stance_0071be80(host, director, stance);
    return host.lua_result_count();
}

int lua_artillery_enable_0089c590(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const NativeHandle director = host.weapon_director(unit);
    const bool enabled = host.lua_argument_bool(1);
    // The director is only a gate: the message carries no pointer to it.
    if (director != 0) {
        host.route_weapon_enable(artillery_enable_message_0071dfd0(enabled));
    }
    return host.lua_result_count();
}

int lua_torpedo_enable_0089c8f0(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const NativeHandle director = host.weapon_director(unit);
    const bool enabled = host.lua_argument_bool(1);
    if (director != 0) {
        host.route_weapon_enable(torpedo_enable_message_0071e0d0(enabled));
    }
    return host.lua_result_count();
}

int lua_ship_set_torpedo_stock_0089eee0(UnitWeaponHost& host) {
    // No director and no class test: the unit goes straight to 0081F8B0.
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const int stock = host.lua_argument_int(1);
    set_torpedo_stock_0081f8b0(host, unit, stock);
    return host.lua_result_count();
}

int lua_get_ship_torpedoes_008bed80(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    host.lua_new_result_table();
    // The live-projectile registry, not the unit's devices: a torpedo already
    // in the water is owned through its +3BCh back-pointer.
    const int total = host.live_torpedo_count();
    int key = 1;
    for (int index = 0; index < total; ++index) {
        const NativeHandle torpedo = host.live_torpedo_at(index);
        if (torpedo == 0) {
            continue;
        }
        if (host.torpedo_is_dead(torpedo)) {
            continue;
        }
        if (host.torpedo_owner(torpedo) != unit) {
            continue;
        }
        host.lua_result_table_set(key, torpedo);
        key += 1;
    }
    return host.lua_result_count();
}

int lua_set_fire_target_0089a8b0(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const NativeHandle director = host.weapon_director(unit);
    if (director == 0) {
        return host.lua_result_count();
    }
    if (host.lua_argument_is_nil(1)) {
        host.director_clear_fire_target(director);
    } else if (host.lua_argument_is_entity(1)) {
        host.director_set_fire_target_entity(director, host.entity_from_lua_argument(1));
    } else {
        float position[3] = {0.0f, 0.0f, 0.0f};
        host.lua_argument_vector3(1, position);
        host.director_set_fire_target_position(director, position);
    }
    return host.lua_result_count();
}

int lua_get_fire_target_0089c360(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    const NativeHandle director = host.weapon_director(unit);
    const NativeHandle target = director != 0 ? host.director_fire_target(director) : 0;
    if (target == 0) {
        host.lua_push_nil();
    } else {
        host.lua_push_entity(target);
    }
    return host.lua_result_count();
}

int lua_get_firepower_0088dc10(UnitWeaponHost& host) {
    const NativeHandle unit = host.entity_from_lua_argument(0);
    host.lua_push_number(host.unit_firepower(unit));
    return host.lua_result_count();
}

} // namespace bsp
