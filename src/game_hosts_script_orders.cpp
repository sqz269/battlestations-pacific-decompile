// bsp_game.exe milestone 2m: the mission script's navigator, formation, skill,
// repair and role bindings, run through their recovered bodies.
//
// See include/bsp/game_hosts_script_orders.hpp for the address list and for what
// this file supplies rather than recovers.

#include "bsp/game_hosts_script_orders.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_units.hpp"
#include "bsp/lua_binding_navigator.hpp"
#include "bsp/mission_lua_bindings.hpp"
#include "bsp/attack_target_classify.hpp"
#include "bsp/pilot_order_bindings.hpp"
#include "bsp/mission_lua_host.hpp"

extern "C" {
#include "lua.h"
}

#include <cstdio>
#include <cstring>

namespace bsp::game {
namespace {

// The eight rows src/lua_binding_navigator.cpp reconstructs, with the binding
// table row address 006b8610 registers each one under. The three names that
// share 008a2f20's body are listed with their own rows because the table does.
struct ScriptOrderBinding {
    const char* name;
    std::uint32_t row_address;
};

constexpr ScriptOrderBinding kScriptOrderBindings[] = {
    {"NavigatorAttackMove", 0x008a30d0u},
    // Packet cc7_pilot_order_bindings. The scripts' most-used order by an order
    // of magnitude - 1125 lines across 197 shipped files - and the one USN01
    // actually calls (`native PilotSetTarget argc=2 phase=luaStageInit`).
    {"PilotSetTarget", 0x008a4c90u},
    {"NavigatorMoveToRange", 0x008a2f20u},
    {"NavigatorMoveToPos", 0x008a2bc0u},
    {"NavigatorDirectMoveToRange", 0x008a2d70u},
    {"JoinFormation", 0x00899d10u},
    {"SetSkillLevel", 0x00895250u},
    {"RepairEnable", 0x008ad330u},
    {"SetRoleAvailable", 0x008ab850u},
    // Packet cc_lua_binding_audit: the delayed-call scheduler and the four state
    // queries usn_2_java's objective checker reads. src/lua_binding_mission.cpp
    // and, for SetThink, src/lua_binding_core.cpp.
    {"CreateScript", 0x00898750u},
    {"SetThink", 0x00897fb0u},
    {"SetWait", 0x00898150u},
    {"ClearThink", 0x00898490u},
    {"DeleteScript", 0x00898ac0u},
    {"GetHpPercentage", 0x0088e9d0u},
    {"GetPosition", 0x008a7b00u},
    {"GetMeasure", 0x0088d8e0u},
    {"GameTime", 0x008a9320u},
    {"random", 0x0088c160u},
    // Packet cc_mission_blackout: the fade whose completion callback is the only
    // route from the intro movie to `luaIn`. src/mission_blackout.cpp.
    {"Blackout", 0x008d1340u},
    // Packet cc_lua_find_entity: usn_2_java.lua:54 is `this.Party = SetParty(this,
    // PARTY_ALLIED)`, so the mission's own party number is this binding's return
    // value and nothing else in the shipped scripts assigns it. The body is
    // already reconstructed in src/lua_binding_core.cpp.
    {"SetParty", 0x008a8930u},
};

// The id the first script entity takes. The created scene instances number from 1
// (milestone 2l), so the script entities start past any plausible instance count
// and a `Ptr` from one family is never mistaken for the other.
constexpr std::uint32_t kScriptEntityIdBase = 100000u;

// The records are handed to Lua as light userdata, so the storage must not move
// under a live `Ptr`. The capacity is taken once and creation stops at it; the
// native has no such bound, which is why the cap is reported rather than silent.
constexpr std::size_t kScriptEntityCapacity = 512;

// The one call site of SetThink's reconstructed body, 008980E8 / 0088A330. Every
// other method of bsp::LuaBindingCoreHost belongs to a different binding and is
// not reached from here; each records itself if it ever is.
class SetThinkCoreHost final : public bsp::LuaBindingCoreHost {
public:
    explicit SetThinkCoreHost(GameScriptOrdersHost& owner) : owner_(owner) {}

    void entity_set_think_script_name(void* entity, const std::string& name) override {
        owner_.entity_set_think_script_name_0088a330(entity, name);
    }

    int game_non_campaign_flag() override { return 0; }
    int game_effective_difficulty() override { return 0; }
    void log_prepare_class(int) override {}
    bool resolve_global_integer(const std::string&, int&) override { return false; }
    void vehicle_class_mark_party_required(int, int) override {}
    void vehicle_class_get_or_create(int, bool) override {}
    void music_director_set_level(int) override {}
    void session_send_music_level(int) override {}
    // The two SetParty virtuals and the session route. docs/LUA_BINDING_CORE.md
    // records that neither virtual has a resolved concrete vtable, so both are
    // named by slot and both are reported rather than answered; the false here
    // is the no-session-message arm taken by choice, not a recovered answer.
    bool entity_vcall_5c(void* entity, int selector) override {
        static_cast<void>(entity);
        static_cast<void>(selector);
        owner_.record_unimplemented("LuaBindingCore::entity_party_query_vtable_5c",
            "008a8a83");
        return false;
    }
    void entity_vcall_2c(void* entity, int party, std::uint32_t field_58) override {
        static_cast<void>(entity);
        static_cast<void>(party);
        static_cast<void>(field_58);
        owner_.record_unimplemented("LuaBindingCore::entity_set_party_vtable_2c",
            "008a8ae3");
    }
    void session_route_party_message(void* entity, int party) override {
        static_cast<void>(entity);
        static_cast<void>(party);
        owner_.record_unimplemented("LuaBindingCore::session_route_party_message",
            "008a8ac4");
    }
    void scoring_set_real_play_time_running(bool) override {}
    void scoring_set_final_scoring_function_name(const std::string&) override {}
    void message_map_load(const std::string&, int) override {}
    void call_0088b6d0_0076a9f0_00765590(const std::string&, int) override {}
    void set_entity_message_suppression(void*, bool) override {}
    void set_global_message_suppression(bool) override {}
    void message_system_drain_queue() override {}

private:
    GameScriptOrdersHost& owner_;
};

const ScriptOrderBinding* find_binding(const char* name) noexcept {
    if (name == nullptr) return nullptr;
    for (const ScriptOrderBinding& row : kScriptOrderBindings) {
        if (std::strcmp(row.name, name) == 0) return &row;
    }
    return nullptr;
}

// bsp::LuaCommandTargetSource and bsp::LuaBindingNavigatorHost index arguments
// the way the native's call frame does: index 0 is Lua stack slot 1.
int stack_slot(int index) noexcept { return index + 1; }

void format_address(std::uint32_t value, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(value));
}

// The two command objects the four navigator bindings name. Both constants come
// from include/bsp/unit_commanded_speed.hpp and lua_binding_navigator.hpp.
const char* command_name_of(std::uint32_t object) noexcept {
    if (object == bsp::kCommandObjectAttackMove) return "attackmove";
    if (object == bsp::kCommandObjectMoveTo) return "moveto";
    return "";
}

}  // namespace

GameScriptOrdersHost::GameScriptOrdersHost(GameHostLog& log, GameUnitsHost& units)
    : log_(log), units_(units) {}

bool GameScriptOrdersHost::handles(const char* binding_name) noexcept {
    return find_binding(binding_name) != nullptr;
}

// ---------------------------------------------------------------------------
// The argument reads
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::record_unimplemented(const char* method,
    const char* address) {
    log_.unimplemented(method, address);
}

void GameScriptOrdersHost::register_scene_marker(int id, const std::string& name,
    const float world_position[3]) {
    if (id <= 0) return;
    SceneMarker marker;
    marker.id = id;
    marker.name = name;
    marker.position[0] = world_position[0];
    marker.position[1] = world_position[1];
    marker.position[2] = world_position[2];
    markers_.push_back(marker);
}

const GameScriptOrdersHost::SceneMarker* GameScriptOrdersHost::marker_for_id(
    int id) const noexcept {
    for (const SceneMarker& marker : markers_) {
        if (marker.id == id) return &marker;
    }
    return nullptr;
}

void* GameScriptOrdersHost::entity_from_argument(int index) {
    // 00888aa0 BSP_ObjectHandle_FromLuaTable. The native validates the table and
    // converts its `Ptr` to the entity object; this process has no such object,
    // so the resolve is the `ID` field 00928a00 seeds turned into the created
    // instance of that index. Milestone 2l established that identity.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return nullptr;
    if (lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "ID");
    // 00928BA5 seeds `ID` as the key text; lua_tonumber converts it.
    const int type = lua_type(state_, -1);
    const bool number = type == LUA_TNUMBER || type == LUA_TSTRING;
    const int id = number ? static_cast<int>(lua_tonumber(state_, -1)) : 0;
    lua_settop(state_, argument_count_);
    if (!number || id <= 0) return nullptr;
    const std::size_t unit = static_cast<std::size_t>(id - 1);
    // Packet cc_lua_find_entity: the native's 00888AA0 validates the table and
    // converts its `Ptr`, and every entity that reached virtual slot 39 has one,
    // unit or not. A scene marker is therefore as resolvable here as a unit is.
    if (unit >= units_.count() && marker_for_id(id) == nullptr) return nullptr;
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(id));
}

std::size_t GameScriptOrdersHost::index_of(void* entity) const noexcept {
    const std::uintptr_t id = reinterpret_cast<std::uintptr_t>(entity);
    if (id == 0) return static_cast<std::size_t>(-1);
    return static_cast<std::size_t>(id - 1);
}

std::string GameScriptOrdersHost::name_of(void* entity) const {
    const std::size_t index = index_of(entity);
    const GameUnitRow* row = (index < units_.count()) ? units_.unit_row(index) : nullptr;
    if (row != nullptr) return row->name;
    const SceneMarker* marker
        = marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(entity)));
    return (marker != nullptr) ? marker->name : std::string();
}

bool GameScriptOrdersHost::argument_id_field_is_nil(int index) {
    // 00b67910 fetching "ID" at 0088a83b, then 00b65fb0 at 0088a84a.
    if (state_ == nullptr) return true;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return true;
    lua_getfield(state_, slot, "ID");
    const bool nil = lua_type(state_, -1) == LUA_TNIL;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaCommandTarget::argument_id_field", "0088a83b");
    return nil;
}

void* GameScriptOrdersHost::argument_entity(int index) {
    void* entity = entity_from_argument(index);
    log_.implemented("LuaBinding::argument_entity", "00888aa0");
    return entity;
}

bool GameScriptOrdersHost::argument_vector3(int index, float out[3]) {
    // 00888760 at 0088a8a1 reads the value as a Vector3. The shipped scripts
    // pass an entity table to every one of these rows, so this arm is reached
    // only for a target that carries no `ID`.
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return false;
    static const char* const kKeys[3] = {"x", "y", "z"};
    bool complete = true;
    for (int lane = 0; lane < 3; ++lane) {
        lua_getfield(state_, slot, kKeys[lane]);
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            out[lane] = static_cast<float>(lua_tonumber(state_, -1));
        } else {
            complete = false;
        }
        lua_settop(state_, argument_count_);
    }
    log_.implemented("LuaCommandTarget::argument_vector3", "00888760");
    return complete;
}

std::uint16_t GameScriptOrdersHost::entity_object_id(void* entity) {
    // entity+174h, the handle-table ordinal. The two tables at 00f89a0c and
    // 00f89a60 are not built here, so the executable's own entity number stands
    // in for it, which is the same substitution GameCommandUnit already makes.
    log_.unimplemented("LuaBinding::entity_object_id", "0088a88e");
    const std::size_t index = index_of(entity);
    const int id = static_cast<int>(reinterpret_cast<std::uintptr_t>(entity));
    if (index >= units_.count() && marker_for_id(id) == nullptr) return 0;
    return static_cast<std::uint16_t>(index + 1);
}

// 008A4C90 `PilotSetTarget(unit, target [, attackType])`, the order the shipped
// scripts use more than any other - 1125 lines across 197 files - and the only
// `Pilot*` row USN01 calls. docs/PILOT_ORDER_BINDINGS.md has the recovered body.
//
// This resolves the call and reports it. **It deliberately does not issue the
// command**, and the reason is worth stating plainly rather than hiding behind a
// partial implementation: the command class comes from `007EEC50`, whose
// `AttackFeasibilityInputs` (include/bsp/attack_commands.hpp) need about a dozen
// per-class capability answers - is the unit a level bomber, is the target a
// submarine or bomb-excluded, does the unit carry ordnance of kinds 31h/2Ah/2Fh/
// 2Bh - each of which is a `vt[5Ch](n)` query on a class object this process does
// not build. Guessing them would produce orders that look like they work, which
// is worse than none.
//
// What it does prove is the argument path, which was the open question: the
// native's `00888AA0` reads the Lua table's `Ptr` field and not its `ID`, and
// this host seeds `Ptr` with the entity id (src/game_hosts_lua.cpp) and
// represents an entity as that id cast to a pointer - so the native's own field
// is the correct one to read here, and `entity_from_argument`'s `ID`-as-number
// path is the odd one out.
int GameScriptOrdersHost::run_pilot_set_target(GameScriptOrderRow& row) {
    void* unit = argument_ptr_field(0);
    if (unit == nullptr) unit = entity_from_argument(0);
    row.unit_index = index_of(unit);
    row.unit = name_of(unit);

    const bsp::SceneCommandTarget target = bsp::lua_read_command_target(*this, 1);
    const int attack_type = bsp::pilot_set_target_attack_type_008a4e0b(
        argument_count_, argument_integer(2));
    const bsp::PilotAttackSelectorFlags flags =
        bsp::pilot_attack_selector_flags_008a4e54(attack_type);

    // docs/ATTACK_CAPABILITY_INPUTS.md: eight of 007EEC50's eleven feasibility
    // inputs are answerable from an authored class id alone, and this host
    // already holds the machinery - unit_is_kind_of walks the 88 compiled
    // vt[5Ch] bodies, which is the same question the native asks. Report them so
    // the remaining gap is a measured list rather than an assertion.
    const std::size_t target_index = target.object != nullptr
        ? index_of(target.object)
        : (target.object_id > 0 ? static_cast<std::size_t>(target.object_id - 1)
                                : ~static_cast<std::size_t>(0));
    const int self_class = units_.unit_class_id(row.unit_index);
    const int target_class = units_.unit_class_id(target_index);
    // docs/ATTACK_CAPABILITY_INPUTS.md Part 2: the two classifiers behind
    // target_is_air and target_is_surface. Both need the target's live +5Dh
    // byte, which this host does model - unit_flag_005d, and the same byte
    // unit_alive_and_visible reads - so they are answerable here rather than
    // refused, which is what that document's contract assumed a host could not
    // do. The surface walk still reports kUnreadSetBranch if it reaches
    // 008DDF90, which no ship or aircraft target does.
    bsp::EntityTargetFacts tf;
    tf.present = target_index < units_.count();
    if (tf.present) {
        tf.not_engageable = units_.unit_flag_005d(target_index);
        tf.is_plane = units_.unit_is_kind_of(target_index, 0x0f);
        tf.is_plane_squadron = units_.unit_is_kind_of(target_index, 0x18);
        tf.is_ship_family = units_.unit_is_kind_of(target_index, 0x06);
        tf.is_submarine = units_.unit_is_kind_of(target_index, 0x08);
        tf.is_airfield = units_.unit_is_kind_of(target_index, 0x45);
        tf.is_shipyard = units_.unit_is_kind_of(target_index, 0x46);
        tf.is_command_building = units_.unit_is_kind_of(target_index, 0x1c);
        tf.is_dummy_target = units_.unit_is_kind_of(target_index, 0x35);
        tf.is_land_fort = units_.unit_is_kind_of(target_index, 0x1b);
        float r[3], u[3], f[3], t[3];
        if (units_.unit_pose(target_index, r, u, f, t)) tf.world_y = t[1];
    }
    const bool target_air = bsp::entity_is_airborne_00922b10(tf);
    const bsp::SurfaceTargetAnswer target_surface =
        bsp::entity_is_surface_target_00922c80(tf);
    log_.notef("  PilotSetTarget classify: target_is_air=%d target_is_surface=%s "
        "(+5Dh=%d y=%.1f)", target_air ? 1 : 0,
        target_surface == bsp::SurfaceTargetAnswer::kYes ? "yes"
            : (target_surface == bsp::SurfaceTargetAnswer::kNo ? "no" : "UNREAD-SET-BRANCH"),
        tf.not_engageable ? 1 : 0, static_cast<double>(tf.world_y));
    log_.notef("  PilotSetTarget caps: self_class=%d level_bomber=%d kamikaze_capable=%d "
        "dogfight_excluded=%d | target_class=%d structure=%d bomb_excluded=%d "
        "submarine=%d ship_family=%d | REFUSED: weapon_controller, target_is_air, "
        "target_is_surface (need live +5Dh); gates 0047B850/00604A50/00828EC0 unread",
        self_class,
        units_.unit_is_kind_of(row.unit_index, 0x10) ? 1 : 0,
        units_.unit_is_kind_of(row.unit_index, 0x17) ? 1 : 0,
        units_.unit_is_kind_of(row.unit_index, 0x16) ? 1 : 0,
        target_class,
        units_.unit_is_kind_of(target_index, 0x1c) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x0e) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x08) ? 1 : 0,
        units_.unit_is_kind_of(target_index, 0x06) ? 1 : 0);
    log_.notef("  PilotSetTarget: unit=%s target_object_id=%u target_valid=%d "
        "pos=(%.1f %.1f %.1f) attack_type=%d prefer_ordnance=%d allow_guns=%d "
        "-> NOT ISSUED, 007EEC50 needs per-class capability inputs this host "
        "does not build (docs/PILOT_ORDER_BINDINGS.md)",
        row.unit.empty() ? "(unresolved)" : row.unit.c_str(),
        static_cast<unsigned>(target.object_id),
        target.position_valid ? 1 : 0,
        static_cast<double>(target.position[0]),
        static_cast<double>(target.position[1]),
        static_cast<double>(target.position[2]),
        attack_type, flags.prefer_ordnance ? 1 : 0, flags.allow_guns ? 1 : 0);
    ++pilot_set_target_calls_;
    if (!row.unit.empty()) ++pilot_set_target_unit_resolved_;
    if (target.object_id != 0 || target.position_valid) ++pilot_set_target_target_resolved_;
    return 0;  // 00B66400: the binding pushes nothing.
}

int GameScriptOrdersHost::argument_integer(int index) {
    if (state_ == nullptr) return 0;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return 0;
    log_.implemented("LuaBinding::argument_integer", "00b66290");
    return static_cast<int>(lua_tonumber(state_, slot));
}

bool GameScriptOrdersHost::argument_boolean(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return false;
    log_.implemented("LuaBinding::argument_boolean", "00b66250");
    return lua_toboolean(state_, slot) != 0;
}

void* GameScriptOrdersHost::argument_ptr_field(int index) {
    // 00888d20 BSP_LuaTable_GetPtrField: the table's `Ptr` without 00888aa0's
    // entity validation. This process seeds `Ptr` with the entity's own id, so
    // the same identity comes back.
    if (state_ == nullptr) return nullptr;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return nullptr;
    lua_getfield(state_, slot, "Ptr");
    void* value = (lua_type(state_, -1) == LUA_TLIGHTUSERDATA)
        ? lua_touserdata(state_, -1) : nullptr;
    lua_settop(state_, argument_count_);
    log_.implemented("LuaBinding::argument_ptr_field", "00888d20");
    return value;
}

// ---------------------------------------------------------------------------
// The effects
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::entity_issue_command(void* entity,
    std::uint32_t command_object, const bsp::SceneCommandTarget& target, int flags) {
    const std::size_t index = index_of(entity);
    if (index >= units_.count()) return;
    if (!logged_path_) {
        logged_path_ = true;
        log_.notef("the mission script's navigator bindings now issue through the same "
            "path an authored scene command takes: 008a30d0 / 008a2f20 hand 0077d600 a "
            "fixed command object and the descriptor 0088a810 built, and 0077d600 builds "
            "MT_COMMAND and routes it. What it reaches is 00816e30, whose own arms "
            "00816f7c..00817330 hold `moveto` and `attackmove` and are projected nowhere "
            "(docs/CRUISE_COMMAND.md), so the command stops one hop short of a weapon "
            "director slot. That block is packet `entity_command_arms`");
    }
    const char* name = command_name_of(command_object);
    std::string target_name;
    if (target.object != nullptr) {
        target_name = name_of(target.object);
    } else if (target.position_valid != 0) {
        target_name = "(position)";
    }
    std::string source("script:");
    source += (row_ != nullptr) ? row_->binding : std::string("navigator");
    const GameCommandRow* issued = units_.issue_script_command(index, command_object,
        target, flags, source, target_name);
    if (row_ != nullptr) {
        row_->command = name;
        row_->target = target_name;
        row_->issued = issued != nullptr && issued->issued;
        row_->reached_director = issued != nullptr && issued->projected_arm;
        if (issued != nullptr) row_->blocked = issued->blocked;
    }
    if (issued != nullptr && issued->issued) ++summary_.issued;
    if (issued != nullptr && issued->projected_arm) ++summary_.reached_director;
    if (command_object == bsp::kCommandObjectAttackMove) ++summary_.attack_moves;
    if (command_object == bsp::kCommandObjectMoveTo) ++summary_.move_tos;
    bool seen = false;
    for (std::size_t existing : ordered_units_) {
        if (existing == index) { seen = true; break; }
    }
    if (!seen) {
        ordered_units_.push_back(index);
        ++summary_.units_ordered;
    }
}

bool GameScriptOrdersHost::entity_command_is_available(void* entity,
    const char* command_name, void* target) {
    static_cast<void>(entity);
    static_cast<void>(command_name);
    static_cast<void>(target);
    if (!logged_predicate_) {
        logged_predicate_ = true;
        log_.notef("JoinFormation stops at its first question. 0077c8d0 asks the follower's "
            "vtable 16Ch whether it may `follow` the leader, which for MDestroyer is "
            "008162b0; that body was not read by the packet that reconstructed the binding "
            "(docs/LUA_BINDING_NAVIGATOR.md, follow-up 3), so the host answers the neutral "
            "false and 0077c902 ends the routine with no effect. Answering true would "
            "invent the predicate");
    }
    log_.unimplemented("Formation::command_is_available", "0077c8fe");
    ++summary_.formations_refused;
    return false;
}

int GameScriptOrdersHost::entity_route_slot(void* entity) {
    static_cast<void>(entity);
    // entity+1ACh, the index 00905300 counts against. No field of a created
    // instance carries it, so the record answers outside the 0..7 bound 0077c90a
    // tests, which is the value that skips the counter.
    log_.unimplemented("Formation::entity_route_slot", "0077c904");
    return -1;
}

void GameScriptOrdersHost::slot_counter_increment(int slot) {
    static_cast<void>(slot);
    log_.unimplemented("Formation::slot_counter_increment", "00905300");
}

void GameScriptOrdersHost::session_route_formation_message(void* follower,
    std::uint16_t leader_object_id) {
    static_cast<void>(follower);
    static_cast<void>(leader_object_id);
    log_.unimplemented("Formation::route_join_message", "0077c964");
}

void GameScriptOrdersHost::entity_set_skill_level(void* entity, int level) {
    static_cast<void>(entity);
    // entity->vtable[128h] at 0089539a. The leaf body belongs to the unit class
    // family and has no reconstruction, so the value is recorded, not applied.
    log_.unimplemented("UnitInstance::set_skill_level", "0089539a");
    if (row_ != nullptr) row_->skill_level = level;
    ++summary_.skills;
}

bool GameScriptOrdersHost::entity_is_kind_of(void* entity, int class_id) {
    // entity->vtable[5Ch] at 008ad44f, the IsKindOf the corpus uses everywhere.
    // This process runs it over the recovered class chain of the created
    // instance, the same one 0068aca0's 20h arm and the minimap walk use.
    const std::size_t index = index_of(entity);
    log_.implemented("UnitInstance::is_kind_of", "008ad44f");
    if (index >= units_.count()) return false;
    return units_.unit_is_kind_of(index, class_id);
}

void GameScriptOrdersHost::entity_set_repair_enabled_field(void* entity, bool enabled) {
    static_cast<void>(entity);
    // The byte at entity+378h, 008ad4e8. No recovered field of a created
    // instance covers it, so the write is recorded.
    log_.unimplemented("UnitInstance::set_repair_enabled_field", "008ad4e8");
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
}

void GameScriptOrdersHost::session_route_repair_enable_message(void* entity, bool enabled) {
    static_cast<void>(entity);
    log_.unimplemented("Session::route_repair_enable_message", "008ad4cd");
    if (row_ != nullptr) row_->repair = enabled ? 1 : 0;
    ++summary_.repairs;
}

int GameScriptOrdersHost::game_session_mode() {
    // [00e188a8]+1fe4h, the field every other host in this executable reports as
    // the single-player value.
    log_.implemented("Game::session_mode", "008ab9d1");
    return 0;
}

int GameScriptOrdersHost::game_effective_game_mode() {
    // 004bca50 at 008ab9de. game+614h and game+61Ch resolve a single-player
    // campaign load to 8, which is the value the scene contents pass already
    // runs its gate on.
    log_.implemented("Game::effective_game_mode", "004bca50");
    return 8;
}

void GameScriptOrdersHost::role_owner_set_role_available(void* owner, int role, int value) {
    static_cast<void>(owner);
    log_.unimplemented("RoleOwner::set_role_available", "008aba51");
    if (row_ != nullptr) {
        row_->role = role;
        row_->role_value = value;
    }
    ++summary_.roles;
}

void GameScriptOrdersHost::session_route_role_message(void* owner, int role, int value) {
    static_cast<void>(owner);
    static_cast<void>(role);
    static_cast<void>(value);
    log_.unimplemented("Session::route_role_message", "008aba2f");
}

void GameScriptOrdersHost::game_assign_party_player_slots(int value) {
    static_cast<void>(value);
    log_.unimplemented("Game::assign_party_player_slots", "008aba60");
}

// ---------------------------------------------------------------------------
// The dispatch
// ---------------------------------------------------------------------------

int GameScriptOrdersHost::dispatch(lua_State* state, const char* binding_name,
    int argument_count) {
    const ScriptOrderBinding* binding = find_binding(binding_name);
    if (binding == nullptr) return 0;
    // CreateScript runs the named global inside its own body, and that global
    // calls further bindings, so a dispatch can re-enter this function. The three
    // per-call fields are saved and restored rather than cleared at the end.
    lua_State* const outer_state = state_;
    const int outer_argument_count = argument_count_;
    GameScriptOrderRow* const outer_row = row_;
    state_ = state;
    machine_state_ = state;
    argument_count_ = argument_count;

    GameScriptOrderRow row;
    row.binding = binding->name;
    row.row_address = binding->row_address;
    row_ = &row;

    char address[16];
    format_address(binding->row_address, address);
    char method[96];
    std::snprintf(method, sizeof(method), "MissionLuaNative::%s", binding->name);
    log_.implemented(method, address);

    void* subject = entity_from_argument(0);
    if (subject == nullptr && std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        subject = argument_ptr_field(0);
    }
    row.unit_index = index_of(subject);
    row.unit = name_of(subject);

    int results = 0;
    if (std::strcmp(binding->name, "PilotSetTarget") == 0) {
        results = run_pilot_set_target(row);
    } else if (std::strcmp(binding->name, "NavigatorAttackMove") == 0) {
        results = bsp::lua_binding_navigator_attack_move(*this, *this);
    } else if (std::strcmp(binding->name, "JoinFormation") == 0) {
        void* leader = entity_from_argument(1);
        row.formation_leader = name_of(leader);
        ++summary_.formations_requested;
        results = bsp::lua_binding_join_formation(*this);
    } else if (std::strcmp(binding->name, "SetSkillLevel") == 0) {
        results = bsp::lua_binding_set_skill_level(*this);
    } else if (std::strcmp(binding->name, "RepairEnable") == 0) {
        bsp::RepairEnableArm arm = bsp::RepairEnableArm::kLocalFieldWrite;
        results = bsp::lua_binding_repair_enable(*this, arm);
    } else if (std::strcmp(binding->name, "SetRoleAvailable") == 0) {
        bsp::SetRoleAvailableArm arm = bsp::SetRoleAvailableArm::kDirectCall;
        results = bsp::lua_binding_set_role_available(*this, arm);
    } else if (std::strcmp(binding->name, "CreateScript") == 0) {
        results = bsp::lua_binding_create_script(*this, *this);
    } else if (std::strcmp(binding->name, "SetThink") == 0) {
        // 00897FB0's body is bsp::lua_binding_set_think (docs/LUA_BINDING_CORE.md);
        // this file supplies its one callee rather than restating the binding.
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_set_think(*this, core);
    } else if (std::strcmp(binding->name, "SetParty") == 0) {
        // 008A8930's own body, over the same adapter SetThink uses. The two
        // entity virtuals it dispatches through have no resolved concrete
        // vtable (docs/LUA_BINDING_CORE.md), so the adapter records both and
        // takes the no-session-message arm; the number the binding pushes back
        // is the argument, which is what usn_2_java.lua:54 stores in
        // Mission.Party and what recon[Mission.Party] is then indexed with.
        SetThinkCoreHost core(*this);
        results = bsp::lua_binding_set_party(*this, *this, core);
    } else if (std::strcmp(binding->name, "SetWait") == 0) {
        results = bsp::lua_binding_set_wait(*this, *this);
    } else if (std::strcmp(binding->name, "ClearThink") == 0) {
        results = bsp::lua_binding_clear_think(*this, *this);
    } else if (std::strcmp(binding->name, "DeleteScript") == 0) {
        results = bsp::lua_binding_delete_script(*this, *this, nullptr);
    } else if (std::strcmp(binding->name, "GetHpPercentage") == 0) {
        results = bsp::lua_binding_get_hp_percentage(*this, *this);
    } else if (std::strcmp(binding->name, "GetPosition") == 0) {
        results = bsp::lua_binding_get_position(*this, *this);
    } else if (std::strcmp(binding->name, "GetMeasure") == 0) {
        results = bsp::lua_binding_get_measure(*this);
    } else if (std::strcmp(binding->name, "GameTime") == 0) {
        results = bsp::lua_binding_game_time(*this);
    } else if (std::strcmp(binding->name, "random") == 0) {
        results = bsp::lua_binding_random(*this, *this, *this);
    } else if (std::strcmp(binding->name, "Blackout") == 0) {
        // 008D142F..008D1612 reads the frame; the marshalling stays here and the
        // decode, the arm and the immediate step are src/mission_blackout.cpp.
        bsp::MissionBlackoutLuaCall call;
        call.argument_count = count();
        call.enable = get_boolean(0);
        if (call.argument_count > 1) call.callback = get_string(1);
        if (call.argument_count > 2) {
            call.duration_is_boolean
                = lua_type(state_, stack_slot(2)) == LUA_TBOOLEAN;  // 00B66000
            if (call.duration_is_boolean) {
                call.duration_boolean = get_boolean(2);
            } else {
                call.duration_number = static_cast<float>(get_number(2));
            }
        }
        if (call.argument_count > 3) {
            call.level_number = static_cast<float>(get_number(3));
        }
        if (call.argument_count <= 2
            || (call.duration_is_boolean && !call.duration_boolean)) {
            // The one path that consumes *(00432650() + E0h), which this packet
            // did not read. Reported rather than silently taken as zero.
            blackout_configured_duration_used_ = true;
        }
        bsp::mission_blackout_binding_008d1340(call, blackout_configured_duration_,
            blackout_, *this);
        ++blackout_summary_.arms;
        blackout_summary_.level = blackout_.level;
        blackout_summary_.remaining = blackout_.remaining;
        log_.notef("  Blackout(%s, \"%s\") target=%.3f remaining=%.4f",
            call.enable ? "true" : "false", call.callback.c_str(),
            static_cast<double>(blackout_.target),
            static_cast<double>(blackout_.remaining));
        results = 0;  // 008D163E, nothing pushed
    } else {
        // 008a2f20, 008a2bc0 and 008a2d70 are one body with one command object.
        results = bsp::lua_binding_navigator_move_to(*this, *this);
    }

    ++summary_.calls;
    rows_.push_back(row);
    row_ = outer_row;
    state_ = outer_state;
    argument_count_ = outer_argument_count;
    return results;
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: the argument reader
// ---------------------------------------------------------------------------

int GameScriptOrdersHost::count() { return argument_count_; }

int GameScriptOrdersHost::get_integer(int index) { return argument_integer(index); }

double GameScriptOrdersHost::get_number(int index) {
    if (state_ == nullptr) return 0.0;
    const int slot = stack_slot(index);
    if (slot > argument_count_) return 0.0;
    if (lua_type(state_, slot) != LUA_TNUMBER) return 0.0;
    return static_cast<double>(lua_tonumber(state_, slot));
}

bool GameScriptOrdersHost::get_boolean(int index) { return argument_boolean(index); }

std::string GameScriptOrdersHost::get_string(int index) {
    if (state_ == nullptr) return std::string();
    const int slot = stack_slot(index);
    if (slot > argument_count_) return std::string();
    if (lua_type(state_, slot) != LUA_TSTRING) return std::string();
    const char* text = lua_tolstring(state_, slot, nullptr);
    return text != nullptr ? std::string(text) : std::string();
}

bool GameScriptOrdersHost::is_string(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    return slot <= argument_count_ && lua_type(state_, slot) == LUA_TSTRING;
}

bool GameScriptOrdersHost::is_nil(int index) {
    if (state_ == nullptr) return true;
    const int slot = stack_slot(index);
    return slot > argument_count_ || lua_type(state_, slot) == LUA_TNIL;
}

bool GameScriptOrdersHost::is_entity_table(int index) {
    if (state_ == nullptr) return false;
    const int slot = stack_slot(index);
    if (slot > argument_count_ || lua_type(state_, slot) != LUA_TTABLE) return false;
    lua_getfield(state_, slot, "Ptr");
    const bool has_ptr = lua_type(state_, -1) == LUA_TLIGHTUSERDATA;
    lua_settop(state_, argument_count_);
    if (has_ptr) return true;
    lua_getfield(state_, slot, "ID");
    const int id_type = lua_type(state_, -1);
    const bool has_id = id_type == LUA_TNUMBER || id_type == LUA_TSTRING;
    lua_settop(state_, argument_count_);
    return has_id;
}

void* GameScriptOrdersHost::entity_at(int index) {
    // 00888AA0 reads `Ptr`, the light userdata 00928A00 seeded. A script entity
    // this process created carries its own record pointer there; a created scene
    // instance carries the `ID`-derived pointer game_hosts_lua.cpp seeds, which
    // entity_from_argument already resolves.
    if (state_ != nullptr) {
        const int slot = stack_slot(index);
        if (slot <= argument_count_ && lua_type(state_, slot) == LUA_TTABLE) {
            lua_getfield(state_, slot, "Ptr");
            void* raw = (lua_type(state_, -1) == LUA_TLIGHTUSERDATA)
                ? lua_touserdata(state_, -1) : nullptr;
            lua_settop(state_, argument_count_);
            if (raw != nullptr && script_entity(raw) != nullptr) return raw;
        }
    }
    return entity_from_argument(index);
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: the script entities
// ---------------------------------------------------------------------------

GameScriptEntity* GameScriptOrdersHost::script_entity(void* handle) noexcept {
    for (GameScriptEntity& entity : script_entities_) {
        if (static_cast<void*>(&entity) == handle) return &entity;
    }
    return nullptr;
}

const GameScriptEntity* GameScriptOrdersHost::script_entity(void* handle) const noexcept {
    for (const GameScriptEntity& entity : script_entities_) {
        if (static_cast<const void*>(&entity) == handle) return &entity;
    }
    return nullptr;
}

bool GameScriptOrdersHost::build_script_self_table(const GameScriptEntity& entity) {
    // 00928A00's three seeded fields, the same shape game_hosts_lua.cpp builds for
    // the created scene instances: `ID` as a value, `Dead` false, `Ptr` as light
    // userdata. Here `Ptr` is the record's own address, which entity_at resolves.
    if (state_ == nullptr) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
        static_cast<int>(entity.id));
    lua_createtable(state_, 0, 3);
    // Corrected by packet cc_lua_find_entity: 00928BA5 seeds `ID` with the key
    // text through 00B67630's lua_pushlstring, and the shipped helpers index
    // thisTable with it.
    lua_pushstring(state_, key);
    lua_setfield(state_, -2, "ID");
    lua_pushboolean(state_, entity.dead ? 1 : 0);
    lua_setfield(state_, -2, "Dead");
    lua_pushlightuserdata(state_, const_cast<GameScriptEntity*>(&entity));
    lua_setfield(state_, -2, "Ptr");
    lua_setfield(state_, -2, key);
    lua_settop(state_, lua_gettop(state_) - 1);
    return true;
}

bool GameScriptOrdersHost::push_script_self_table(const GameScriptEntity& entity) {
    if (state_ == nullptr) return false;
    lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kMissionLuaSelfTable);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 1);
        return false;
    }
    char key[16];
    std::snprintf(key, sizeof(key), bsp::kMissionLuaEntityKeyFormat,
        static_cast<int>(entity.id));
    lua_getfield(state_, -1, key);
    if (!lua_istable(state_, -1)) {
        lua_settop(state_, lua_gettop(state_) - 2);
        return false;
    }
    // Leave the slot on the stack and drop the table it came from.
    lua_insert(state_, -2);
    lua_settop(state_, lua_gettop(state_) - 1);
    return true;
}

void GameScriptOrdersHost::call_script_global(const GameScriptEntity& entity,
    const std::string& name, int stack_first, int stack_last) {
    // 009290A0 -> 00887E50 -> 00887750. The self-key block pushes
    // thisTable[entity+178h] as argument 1 (00887750 nargs starts at 1 only on this
    // path, docs/MISSION_NAMED_CALL_ARGS.md); a non-zero stack_first then appends
    // the caller's own Lua stack slots stack_first..stack_last.
    if (state_ == nullptr || name.empty()) return;
    const int base = lua_gettop(state_);
    // The native passes errfunc 0 (docs/MISSION_NAMED_CALL_ARGS.md) and keeps only
    // the message. This process installs `debug.traceback` as the handler purely
    // so a failure names the shipped-script call chain in the log; the traceback
    // is the executable's diagnostic, not a change to what the mission sees,
    // because a failed call still returns nothing either way.
    lua_getfield(state_, LUA_GLOBALSINDEX, "debug");
    int errfunc = 0;
    if (lua_istable(state_, -1)) {
        lua_getfield(state_, -1, "traceback");
        lua_remove(state_, -2);
        if (lua_isfunction(state_, -1)) {
            errfunc = lua_gettop(state_);
        } else {
            lua_settop(state_, base);
        }
    } else {
        lua_settop(state_, base);
    }
    lua_getfield(state_, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(state_, -1)) {
        lua_settop(state_, base);
        return;
    }
    int pushed = 0;
    if (push_script_self_table(entity)) {
        ++pushed;
    } else {
        lua_createtable(state_, 0, 0);
        ++pushed;
    }
    if (stack_first != 0) {
        const int last = (stack_last < 0) ? base : stack_last;
        for (int slot = stack_first; slot <= last; ++slot) {
            lua_pushvalue(state_, slot);
            ++pushed;
        }
    }
    if (lua_pcall(state_, pushed, 0, errfunc) != 0) {
        ++timers_.call_failures;
        const char* message = lua_tolstring(state_, -1, nullptr);
        if (timers_.first_error.empty() && message != nullptr) {
            timers_.first_error = message;
        }
        log_.notef("  script call %s failed: %s", name.c_str(),
            message != nullptr ? message : "(no message)");
    }
    lua_settop(state_, base);
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: bsp::LuaBindingMissionHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::push_number(int value) {
    if (state_ == nullptr) return;
    lua_pushnumber(state_, static_cast<lua_Number>(value));
}

void GameScriptOrdersHost::push_boolean(bool value) {
    if (state_ == nullptr) return;
    lua_pushboolean(state_, value ? 1 : 0);
}

void GameScriptOrdersHost::push_nil() {
    if (state_ == nullptr) return;
    lua_pushnil(state_);
}

void GameScriptOrdersHost::push_number_float_00b66480(float value) {
    if (state_ == nullptr) return;
    lua_pushnumber(state_, static_cast<lua_Number>(value));
}

float GameScriptOrdersHost::random_uniform_00bd2f10(float minimum, float maximum) {
    // 00BD2F10 draws from the thread's random state object (00BD2ED0). This
    // process has no such object, so the draw is this generator's: a 32-bit
    // linear congruential sequence with a fixed seed, which makes a headless run
    // reproducible. The distribution and the half-open bounds are the native's.
    ++random_draws_;
    random_state_ = random_state_ * 1664525u + 1013904223u;
    const float unit = static_cast<float>(random_state_ >> 8) / 16777216.0f;
    return minimum + (maximum - minimum) * unit;
}

bool GameScriptOrdersHost::unit_health_gate_5d_00923be4(void* entity) {
    // include/bsp/game_hosts_units.hpp carries +5Ch but not +5Dh, so the gate is
    // the neutral clear and the record is the health itself, below.
    static_cast<void>(entity);
    return false;
}

float GameScriptOrdersHost::unit_health_vtable_110_00923bf6(void* entity) {
    // The virtual at the unit's vtable +110h. No concrete vtable is resolved in
    // this process and no created instance carries a health field, so this is a
    // record and the binding answers the unclamped zero the rule then returns.
    static_cast<void>(entity);
    log_.unimplemented("UnitInstance::health_vtable_110", "00923bf6");
    return 0.0f;
}

void GameScriptOrdersHost::unit_health_cache_store_00923c16(void* entity, float value) {
    static_cast<void>(entity);
    static_cast<void>(value);
}

bool GameScriptOrdersHost::entity_pose_stale_008a7c24(void* entity) {
    // This process's instances publish their position every motion tick, so the
    // pose is never stale here and the refresh at 008A7C37 is not reached.
    static_cast<void>(entity);
    return false;
}

void GameScriptOrdersHost::entity_pose_refresh_00414db0(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("EntityPose::refresh_world", "00414db0");
}

bool GameScriptOrdersHost::entity_pose_translation_008a7c3c(void* entity, float out[3]) {
    out[0] = 0.0f;
    out[1] = 0.0f;
    out[2] = 0.0f;
    const std::size_t index = index_of(entity);
    const GameUnitRow* row = (index < units_.count()) ? units_.unit_row(index) : nullptr;
    if (row != nullptr) {
        out[0] = row->position[0];
        out[1] = row->position[1];
        out[2] = row->position[2];
        return true;
    }
    // Packet cc_lua_find_entity: 008A7C3C reads entity+0FCh, the world matrix
    // translation row, for whatever entity the argument resolved to. A scene
    // marker's is the composed `localframe` the scene file authored.
    const SceneMarker* marker
        = marker_for_id(static_cast<int>(reinterpret_cast<std::uintptr_t>(entity)));
    if (marker == nullptr) return false;
    out[0] = marker->position[0];
    out[1] = marker->position[1];
    out[2] = marker->position[2];
    return true;
}

void GameScriptOrdersHost::push_vector3_table_0088ba30(const float xyz[3]) {
    if (state_ == nullptr) return;
    lua_createtable(state_, 0, 3);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[0]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyX);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[1]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyY);
    lua_pushnumber(state_, static_cast<lua_Number>(xyz[2]));
    lua_setfield(state_, -2, bsp::kPositionTableKeyZ);
}

bool GameScriptOrdersHost::measure_is_imperial_0088d9bd() {
    // 00F88988. Nothing in this process writes it, so it keeps its zero and the
    // binding takes the metric arm, which is the executable's own state and not a
    // substitute for one.
    return measure_imperial_;
}

void GameScriptOrdersHost::push_global_path_value_00b672b0(const char* dotted_path) {
    // Corrected by packet cc_lua_find_entity. 00B672B0 does not walk the path:
    // its two callees are 00A672F0 `lua_checkstack(L, 1)` and 00A67A10
    // `lua_pushlstring(L, data, length)` over the NativeString the caller built
    // at 0088D9D8 / 0088DA0D, with the empty literal at 0108FF2C standing in for
    // a null data pointer (00B672CD). GetMeasure therefore answers the text key
    // "globals.kilometer" or "globals.mile" itself, which is what usn_2_java's
    // own luaMetric (line 923) compares against and what the "#Mission.Measure#"
    // substitution in its score line expects.
    if (state_ == nullptr || dotted_path == nullptr) return;
    lua_pushstring(state_, dotted_path);
}

float GameScriptOrdersHost::game_clock_seconds_008a93fe() { return mission_clock_; }

void* GameScriptOrdersHost::script_entity_create_00898841() {
    // The 0x1E4-byte allocation and 00928630's construct. This process keeps only
    // the fields the five script bindings and 00929460 read; every other byte of
    // the native block has no reader here.
    if (script_entities_.capacity() < kScriptEntityCapacity) {
        script_entities_.reserve(kScriptEntityCapacity);
    }
    if (script_entities_.size() >= kScriptEntityCapacity) {
        // 00898841 has no bound; this process does, because the records are handed
        // to Lua as light userdata and must not move.
        log_.notef("script entity cap %zu reached; CreateScript answers no entity "
            "from here on, which the native never does", kScriptEntityCapacity);
        return nullptr;
    }
    GameScriptEntity entity;
    entity.id = kScriptEntityIdBase + static_cast<std::uint32_t>(script_entities_.size());
    script_entities_.push_back(entity);
    ++timers_.scripts_created;
    return static_cast<void*>(&script_entities_.back());
}

void GameScriptOrdersHost::entity_set_think_script_name_0088a330(void* entity,
    const std::string& name) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        // A created scene instance, not a script entity: this process has no
        // think slot on those, so the registration is a record.
        log_.unimplemented("Entity::set_think_script_name", "0088a330");
        return;
    }
    // 0088A35B/0088A363 free and null the old name first; 0088A373 strdups the new
    // one; 0088A34B appends to the pending list only on the null-to-name edge.
    const bool was_null = script->think_name.empty();
    script->think_name = name;
    if (was_null && !name.empty()) {
        bsp::register_pending_think_entity_0088a240(think_pending_, script->id);
        ++timers_.think_registrations;
    }
    if (name.empty()) script->delay_armed = false;  // 0088A37F
}

void GameScriptOrdersHost::script_entity_vcall_98_0089892c(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("ScriptEntity::place_vtable_98", "0089892c");
}

void GameScriptOrdersHost::script_entity_call_00927610_00898932(void* entity) {
    static_cast<void>(entity);
    log_.unimplemented("ScriptEntity::call_00927610", "00898932");
}

int GameScriptOrdersHost::lua_stack_top_00b65eb0() {
    return state_ != nullptr ? lua_gettop(state_) : 0;
}

void GameScriptOrdersHost::entity_call_named_009290a0(void* entity,
    const std::string& name, int stack_first, int stack_last) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) return;
    script->created_for = name;
    build_script_self_table(*script);
    call_script_global(*script, name, stack_first, stack_last);
}

bool GameScriptOrdersHost::push_self_table_slot_008989f6(void* entity) {
    const GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) return false;
    return push_script_self_table(*script);
}

void GameScriptOrdersHost::entity_arm_think_delay_008982c9(void* entity, float seconds) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("Entity::arm_think_delay", "008982c9");
        return;
    }
    script->delay_seconds = seconds;
    script->delay_armed = true;
    ++timers_.waits_armed;
}

void GameScriptOrdersHost::entity_clear_think_name_008985a6(void* entity) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("Entity::clear_think_name", "008985a6");
        return;
    }
    script->think_name.clear();
    script->delay_armed = false;
    ++timers_.clears;
}

bool GameScriptOrdersHost::entity_flag_5e_00898bd9(void* entity) {
    const GameScriptEntity* script = script_entity(entity);
    return script != nullptr ? script->blocked_5e : false;
}

void GameScriptOrdersHost::entity_kill_00926d90(void* entity, int cause) {
    GameScriptEntity* script = script_entity(entity);
    if (script == nullptr) {
        log_.unimplemented("MissionEntity::kill", "00926d90");
        return;
    }
    static_cast<void>(cause);
    // 00926D90 sets +5Fh and queues the entity for the on-killed dispatch at
    // 009273A0, which is where 00929800 sets thisTable[key].Dead and erases the
    // entity from both think lists (00929AA7, 00929AB2). The queue and the
    // dispatch are not run here; the three observable effects are.
    script->dead = true;
    script->blocked_5e = true;
    if (in_think_walk_) {
        // The kill reaches here from inside a think function, so the walk is
        // iterating the live list. The native unlinks a node under a cursor that
        // already captured its successor; this reconstruction walks a vector, so
        // the erase is held until the walk returns and the end state is the same.
        think_erase_after_walk_.push_back(script->id);
    } else {
        bsp::erase_think_entity_00928300(think_live_, script->id);
        bsp::erase_think_entity_00928300(think_pending_, script->id);
    }
    build_script_self_table(*script);
    ++timers_.deletes;
}

// ---------------------------------------------------------------------------
// Packet cc_lua_binding_audit: bsp::EntityThinkHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::run_entity_think_00929150(std::uint32_t entity) {
    for (GameScriptEntity& script : script_entities_) {
        if (script.id != entity) continue;
        ++script.thinks;
        // 00929194: args = 0 and stack_first = 0, so the think function receives
        // only the self table the key resolves.
        call_script_global(script, script.think_name, 0, -1);
        return;
    }
}

void GameScriptOrdersHost::free_think_node_0092952c(const bsp::EntityThinkNode& node) {
    static_cast<void>(node);
}

bool GameScriptOrdersHost::gc_gate_predicate_0109cefc_vtable0c() {
    // The predicate's body was not read by the packet that recovered the walk, and
    // this process has no such object. A false answer only skips collectgarbage().
    return false;
}

void GameScriptOrdersHost::lua_run_string_006b8ad0(const char* chunk, int mode) {
    static_cast<void>(chunk);
    static_cast<void>(mode);
}

void GameScriptOrdersHost::splice_pending_into_live_00928380(bsp::EntityThinkList& live,
    const bsp::EntityThinkList& pending) {
    for (const bsp::EntityThinkNode& node : pending.nodes) live.nodes.push_back(node);
}

void GameScriptOrdersHost::clear_pending_00928330(bsp::EntityThinkList& pending) {
    pending.nodes.clear();
}

// ---------------------------------------------------------------------------
// Packet cc_mission_blackout: bsp::MissionBlackoutHost
// ---------------------------------------------------------------------------

void GameScriptOrdersHost::blackout_icon_set_visible(bool visible) {
    // Widget vtable +34h at 005B99B7 and 005B9A3B. This process has no GUI page,
    // so the widget state is recorded and nothing is drawn.
    log_.implemented("BlackoutIcon::set_visible", "005b99b7");
    static_cast<void>(visible);
}

void GameScriptOrdersHost::blackout_icon_set_colour(const bsp::BlackoutFillColour& colour) {
    // Widget vtable +50h at 005B9A3B.
    log_.implemented("BlackoutIcon::set_colour", "005b9a3b");
    blackout_colour_ = colour;
}

void GameScriptOrdersHost::blackout_icon_get_colour(bsp::BlackoutFillColour& colour) {
    // Widget vtable +54h at 005B99C9. The native reads the widget's own colour;
    // this process answers with the last colour it was given, which starts at the
    // opaque black bsp::BlackoutFillColour defaults to.
    log_.implemented("BlackoutIcon::get_colour", "005b99c9");
    colour = blackout_colour_;
}

void GameScriptOrdersHost::mission_lua_call_named_00887e50(const std::string& name) {
    // 00887E50 at 005B9969 with self key 0, no argument record and no stack
    // range, so the call is `_G[name]()` with zero arguments
    // (docs/MISSION_NAMED_CALL_ARGS.md: nargs starts at 0 off the self-key path).
    log_.implemented("MissionLuaHost::call_named", "00887e50");
    ++blackout_summary_.callbacks;
    blackout_summary_.last_callback = name;
    lua_State* const machine = (state_ != nullptr) ? state_ : machine_state_;
    if (machine == nullptr || name.empty()) return;
    const int base = lua_gettop(machine);
    lua_getfield(machine, LUA_GLOBALSINDEX, name.c_str());
    if (!lua_isfunction(machine, -1)) {
        log_.notef("  blackout callback %s is not a global function", name.c_str());
        lua_settop(machine, base);
        return;
    }
    if (lua_pcall(machine, 0, 0, 0) != 0) {
        ++timers_.call_failures;
        const char* message = lua_tolstring(machine, -1, nullptr);
        if (timers_.first_error.empty() && message != nullptr) {
            timers_.first_error = message;
        }
        log_.notef("  blackout callback %s failed: %s", name.c_str(),
            message != nullptr ? message : "(no message)");
    } else {
        log_.notef("  blackout callback %s ran", name.c_str());
    }
    lua_settop(machine, base);
}

void* GameScriptOrdersHost::local_player_unit_00e188d8() {
    // 00E188D8, read at 005B9867 and 005B9893. This executable never publishes a
    // local player unit, so the interface request the completion arm would push
    // is not reached. It is a HUD transition, not a script gate.
    log_.unimplemented("Game::local_player_unit", "00e188d8");
    return nullptr;
}

bool GameScriptOrdersHost::interface_request_pending_005b66d0() {
    // 005B66D0's body: `*(manager+20h)` against 29h, 2Bh, 2Ch and 2Dh. This
    // process pushes no interface request, so no id is ever pending.
    log_.implemented("InGameInterfaceManager::request_pending", "005b66d0");
    return false;
}

void GameScriptOrdersHost::ingame_interface_store_1c_00644220(int value) {
    // 00644220's whole body is `[this+1Ch] = argument`, on `*(00E198C4 + 40h)`.
    log_.unimplemented("InGameInterface::store_1c", "00644220");
    static_cast<void>(value);
}

void GameScriptOrdersHost::push_interface_request_004cc460(int request_id, void* payload) {
    log_.unimplemented("FrontEndManager::push_interface_request", "004cc460");
    static_cast<void>(payload);
    if (request_id == bsp::kMissionBlackoutCompletionInterfaceRequest) {
        ++blackout_summary_.interface_requests;
    }
}

int GameScriptOrdersHost::game_session_kind_1fe4h() {
    // The same field game_session_mode() answers, `[00e188a8]+1fe4h`.
    return game_session_mode();
}

void GameScriptOrdersHost::session_broadcast_blackout_0076d310(float level, float duration) {
    // 0076D310 at 005B9C36, session message kind 2Bh. Offline, so unreachable.
    log_.unimplemented("Session::broadcast_blackout", "0076d310");
    static_cast<void>(level);
    static_cast<void>(duration);
}

void GameScriptOrdersHost::force_show_please_wait_screen_00e19698() {
    // 005B9A04..005B9A20 on the process-lifetime `_PleaseWait` screen. Only on
    // the `game+1FE4h != 0` arm, which this process never takes.
    log_.unimplemented("PleaseWaitScreen::force_show", "005b9a04");
}

void GameScriptOrdersHost::run_blackout_update(float step) {
    // 004C40F0 at 004C4290..004C429A: BSP_FrontEndScreen_Update(*(00E198C4+A4h),
    // game+21F0h) -> vtable 00CF0ED8 +20h = 005BC920 -> 005B9800 at 005BC9FC.
    // The native gates on the screen's visibility bytes +4h and +5h and on
    // `game+21F0h > 0` (005BC9B0); this process has neither a screen set nor a
    // separate scaled delta and steps with the frame delta it is given.
    if (machine_state_ == nullptr) return;
    if (!(step > 0.0f)) return;  // 005BC9B0, the COMISS against zero
    lua_State* const outer_state = state_;
    const int outer_argument_count = argument_count_;
    state_ = machine_state_;
    argument_count_ = 0;
    const bsp::MissionBlackoutStep record
        = bsp::mission_blackout_update_005b9800(blackout_, step, *this);
    ++blackout_summary_.updates;
    if (record.reached_target) ++blackout_summary_.completions;
    blackout_summary_.level = blackout_.level;
    blackout_summary_.remaining = blackout_.remaining;
    state_ = outer_state;
    argument_count_ = outer_argument_count;
}

void GameScriptOrdersHost::run_script_timers(float step) {
    if (machine_state_ == nullptr) return;
    if (script_entities_.empty()) {
        run_blackout_update(step);
        return;
    }
    state_ = machine_state_;
    argument_count_ = 0;
    mission_clock_ += step;
    std::vector<bsp::EntityThinkFields> fields;
    fields.reserve(script_entities_.size());
    for (const GameScriptEntity& script : script_entities_) {
        bsp::EntityThinkFields row;
        row.entity = script.id;
        row.initialised = script.initialised;
        row.blocked_5d = script.blocked_5d;
        row.blocked_5e = script.blocked_5e;
        row.blocked_60 = script.blocked_60;
        row.has_think_name = !script.think_name.empty();
        row.delay_armed = script.delay_armed;
        row.delay_seconds = script.delay_seconds;
        fields.push_back(row);
    }
    in_think_walk_ = true;
    const bsp::EntityThinkRunSummary run = bsp::run_entity_think_list_00929460(step,
        think_countdown_, think_live_, think_pending_, fields, *this);
    in_think_walk_ = false;
    for (std::uint32_t id : think_erase_after_walk_) {
        bsp::erase_think_entity_00928300(think_live_, id);
        bsp::erase_think_entity_00928300(think_pending_, id);
    }
    think_erase_after_walk_.clear();
    // 009294C7-009294D7 updates +1E0h in place and never re-arms the flag; the
    // rule's own helpers own both, so the entity records follow them here.
    for (GameScriptEntity& script : script_entities_) {
        if (!script.delay_armed) continue;
        if (script.delay_seconds <= 0.0f) continue;
        script.delay_seconds = bsp::entity_think_delay_after_step(script.delay_seconds,
            step);
    }
    ++timers_.passes;
    timers_.timed_fires += run.thinks_run;
    state_ = nullptr;
    // Packet cc_mission_blackout. GGame::OnMove step 18 runs 004C40A0, which
    // drives the fixed-step fan-out and so the think walk, and only then 004C40F0,
    // which steps the fade. Keeping that order here means a `luaDelay` that
    // expires on this frame arms its blackout before the same frame steps it,
    // exactly as the native does.
    run_blackout_update(step);
}

void GameScriptOrdersHost::report() {
    if (timers_.scripts_created != 0) {
        log_.notef("mission script timers (packet cc_lua_binding_audit): the delayed-call "
            "scheduler luaDelay -> CreateScript(\"luaDoTimeTable\") -> SetThink/SetWait, "
            "re-entered by the reconstructed think walk 00929460 once per mission frame");
        for (const GameScriptEntity& script : script_entities_) {
            log_.notef("  script entity %u created_for=%-20s think=%-16s armed=%d "
                "delay=%.2f thinks=%llu dead=%d", script.id,
                script.created_for.empty() ? "(none)" : script.created_for.c_str(),
                script.think_name.empty() ? "(none)" : script.think_name.c_str(),
                script.delay_armed ? 1 : 0,
                static_cast<double>(script.delay_seconds), script.thinks,
                script.dead ? 1 : 0);
        }
        log_.notef("summary mission script timers created=%zu think_registrations=%zu "
            "waits=%zu clears=%zu deletes=%zu passes=%zu fires=%llu failures=%llu %s",
            timers_.scripts_created, timers_.think_registrations, timers_.waits_armed,
            timers_.clears, timers_.deletes, timers_.passes, timers_.timed_fires,
            timers_.call_failures,
            timers_.first_error.empty() ? "" : timers_.first_error.c_str());
    }
    if (blackout_summary_.arms != 0 || blackout_summary_.updates != 0) {
        log_.notef("summary mission blackout (packet cc_mission_blackout, 008D1340 / "
            "005B9BA0 / 005B9800): arms=%zu updates=%zu completions=%zu callbacks=%zu "
            "interface_requests=%zu last_callback=%s level=%.3f remaining=%.3f "
            "configured_default_used=%d",
            blackout_summary_.arms, blackout_summary_.updates,
            blackout_summary_.completions, blackout_summary_.callbacks,
            blackout_summary_.interface_requests,
            blackout_summary_.last_callback.empty()
                ? "(none)" : blackout_summary_.last_callback.c_str(),
            static_cast<double>(blackout_summary_.level),
            static_cast<double>(blackout_summary_.remaining),
            blackout_configured_duration_used_ ? 1 : 0);
    }
    if (rows_.empty()) return;
    log_.notef("the mission script's own orders, run through the eight reconstructed "
        "binding bodies (docs/LUA_BINDING_NAVIGATOR.md):");
    log_.notef("  %-26s %-16s %-10s %-16s %6s %6s", "binding", "unit", "command",
        "target", "issued", "slot");
    for (const GameScriptOrderRow& row : rows_) {
        if (row.command.empty()) continue;
        log_.notef("  %-26s %-16s %-10s %-16s %6d %6d", row.binding.c_str(),
            row.unit.c_str(), row.command.c_str(), row.target.c_str(),
            row.issued ? 1 : 0, row.reached_director ? 1 : 0);
    }
    log_.notef("summary mission script bindings calls=%zu attackmove=%zu moveto=%zu "
        "issued=%zu reached_director=%zu units=%zu formations=%zu/%zu skills=%zu "
        "repairs=%zu roles=%zu", summary_.calls, summary_.attack_moves,
        summary_.move_tos, summary_.issued, summary_.reached_director,
        summary_.units_ordered, summary_.formations_requested - summary_.formations_refused,
        summary_.formations_requested, summary_.skills, summary_.repairs, summary_.roles);
}

}  // namespace bsp::game
