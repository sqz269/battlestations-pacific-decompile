// Packet cc13-powerup-bind: the power-up category lists and their product. See
// include/bsp/game_hosts_powerups.hpp. Re-derived from the image bytes
// (disasm-raw); src/powerup_seg58_argon.cpp was a cross-check only. Names are
// hypotheses, not recovered symbols.

#include "bsp/game_hosts_powerups.hpp"

#include "bsp/cc13_powerup.hpp"
#include "bsp/cc13_x87_model.hpp"
#include "bsp/game_hosts_ai.hpp"

#include <iterator>
#include <stdexcept>
#include <string>

namespace bsp::game_powerups {

namespace {

const PowerupUnitResolver* g_unit_resolver = nullptr;

// [00CE6A04], 1000000.0f: what 006F58D0, slot 0 of the command-building binding
// vtable 00D16194, loads. 008E477E..008E4786 reach it as the centre unit's
// vtable[64h]() (its binding at +758h) then that object's slot 0.
// PROVISIONAL: a centre id names the binding's building, the only unit kind
// whose vtable[64h] is not the null 0042B920 of the root vtable 00CE6290.
constexpr float kBindingRadiusSquared = 1000000.0f;

}  // namespace

// [BSP reconstruction 008e4680]
// Original ABI: __thiscall(ECX = manager, const record* = node+8h, unit*), RET 8;
// returns AL.
bool powerup_target_applies_to_unit_008e4680(const PowerupModifier& record,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E4689..008E46D3: the unit-id arm, only for a live unit. 008E46B9 calls
    // vtable[140h] (0047F320 in the root vtable: MOV EAX,ECX) and compares its
    // +174h word with record+0Ch.
    bool matched = record.unit_id != 0 && unit.live && record.unit_id == unit.id;
    // 008E46D5..008E46EB: the party arm, a signed record+8h below 2 equal to
    // unit+54h (read with no liveness test).
    if (record.party < 2 && record.party == unit.party) matched = true;
    // 008E46ED..008E4708: the slot arm, 009FFD20(unit) when record+10h is not -1.
    if (record.slot != -1 && record.slot == unit.slot) matched = true;
    if (matched) return true;  // 008E470A JNE 008E47CE
    // 008E4710..008E4717: no centre, no match.
    if (record.centre_id == 0) return false;
    // 008E471D..008E4755: the id table lookup; a missing unit fails.
    PowerupUnitView centre;
    if (!resolver.view_unit_by_id(record.centre_id, centre)) return false;
    // 008E4757..008E4775: a live centre on the unit's party.
    if (!centre.live || centre.party != unit.party) return false;
    // 008E4777..008E47A9: the radius (FSTP qword, kBindingRadiusSquared above),
    // then 00427C90 (centre position minus unit position, three float stores)
    // and 00427E30, the squared length (y*y + x*x) + z*z, FSTP to float at
    // 00427E49. Per x87 operation at the main thread's 24-bit precision.
    namespace x8 = bsp::cc13x87;
    const float dx = centre.position[0] - unit.position[0];
    const float dy = centre.position[1] - unit.position[1];
    const float dz = centre.position[2] - unit.position[2];
    const float length_sq = static_cast<float>(x8::x87_add24(
        x8::x87_add24(x8::x87_mul24(dy, dy), x8::x87_mul24(dx, dx)),
        x8::x87_mul24(dz, dz)));
    // 008E47AE..008E47B6: FLD radius; FCOMI ST(1); JBE fails. A match is
    // radius > length_sq.
    return static_cast<double>(kBindingRadiusSquared) > static_cast<double>(length_sq);
}

// [BSP reconstruction 008e6430]
// Original ABI: __thiscall(ECX = manager, category, unit*), RET 8; ST0 result.
float gameplay_modifier_product_008e6430(const std::list<PowerupModifier>& list,
    const PowerupUnitView& unit, const PowerupUnitResolver& resolver) {
    // 008E6435: the product starts at 1.0f (00D7A24C); 008E6460..008E6490 walk
    // the list from the head's next in order; each accepted record's +14h
    // (node+1Ch) multiplies it, FSTP to the float slot at 008E6480.
    float product = 1.0f;
    for (const PowerupModifier& record : list) {
        if (powerup_target_applies_to_unit_008e4680(record, unit, resolver)) {
            product = record.multiplier * product;
        }
    }
    return product;
}

namespace {

// The sites' gate (00470440 shows it whole): CMP BYTE [00E0C978],0, then the
// size at [00F88C30]+88h+category*0Ch. The host has no manager when the class
// registry did not load; its lists are then empty. Null when the gate is shut.
const std::list<PowerupModifier>* gated_list(int category) {
    cc13_powerup::PowerupRuntime& runtime = cc13_powerup::powerup_runtime();
    if (runtime.manager == nullptr || runtime.host == nullptr ||
        !runtime.host->powerups_enabled_00e0c978() || category < 0 ||
        category >= kPowerupCategoryCount) {
        return nullptr;
    }
    const std::list<PowerupModifier>& list =
        runtime.category_lists[static_cast<std::size_t>(category)];
    return list.empty() ? nullptr : &list;
}

}  // namespace

std::optional<float> powerup_site_product(int category, std::size_t unit_index) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if (const std::list<PowerupModifier>* list = gated_list(category)) {
            PowerupUnitView unit;
            if (g_unit_resolver == nullptr || !g_unit_resolver->view_unit(unit_index, unit)) {
                throw std::logic_error("008E6430 without a unit view");
            }
            result = gameplay_modifier_product_008e6430(*list, unit, *g_unit_resolver);
        }
    } else {
        (void)category;
        (void)unit_index;
    }
    return result;
}

std::optional<float> powerup_site_product_for_view(int category, const PowerupUnitView& view) {
    std::optional<float> result;
    if constexpr (kPowerUpSubsystemCc13Bound) {
        if (const std::list<PowerupModifier>* list = gated_list(category)) {
            if (g_unit_resolver == nullptr) {
                throw std::logic_error("008E6430 without a unit resolver");
            }
            result = gameplay_modifier_product_008e6430(*list, view, *g_unit_resolver);
        }
    } else {
        (void)category;
        (void)view;
    }
    return result;
}

void publish_powerup_unit_resolver(const PowerupUnitResolver* resolver) {
    g_unit_resolver = resolver;
}

// --- phase 2a ----------------------------------------------------------------
// Packet cc13-powerup-bind phase 2a: the writers a scene reaches without player
// input, re-derived from disasm-raw of each body. Only the single-player
// (session word 0) arms are modelled; each hosted-session arm throws.

namespace {

namespace cp = bsp::cc13_powerup;

// [game+1FE4h] == 1 sends message 67 or 68 at 008EA466, 008ED5AB, 008E8AC0 and
// 008E8CE6. A mission-tree or skirmish run is session 0; hosting is not modelled.
void require_unhosted_session(cp::PowerupRuntime& runtime, const char* where) {
    if (runtime.host->session_word_1fe4() == 1) {
        throw std::logic_error(std::string(where) + ": session 1 power-up messages not modelled");
    }
}

const PowerupUnitResolver& resolver_or_throw(const char* where) {
    if (g_unit_resolver == nullptr) {
        throw std::logic_error(std::string(where) + " without a unit resolver");
    }
    return *g_unit_resolver;
}

PowerupUnitView view_or_throw(std::size_t unit, const char* where) {
    PowerupUnitView view;
    if (!resolver_or_throw(where).view_unit(unit, view)) {
        throw std::logic_error(std::string(where) + " without a unit view");
    }
    return view;
}

// The list size at manager+28h + slot*0Ch is 008EB110's notify gate for the
// active slot; the host caches it, so every inventory change refreshes it.
void refresh_notify_gate(cp::PowerupRuntime& runtime, std::int32_t slot) {
    if (cp::is_active_slot(runtime, slot)) {
        runtime.host->set_active_slot_notify_gate(
            !runtime.inventories[static_cast<std::size_t>(slot)].empty());
    }
}

// 008E4660: [00E198C4] non-null and its +C0h screen's byte +5 set, then 0061B0F0.
void refresh_hud_if_shown_008e4660(cp::PowerupRuntime& runtime) {
    if (runtime.host->powerup_screen_shown_00e198c4()) {
        runtime.host->powerup_screen_rebuild_lists_0061b0f0();
        ++runtime.counts.screen_rebuilds;
    }
}

// 008E35F0 __fastcall(ECX = unit, EDX = class): the class scope for a unit.
bool class_scope_accepts_unit_008e35f0(const PowerupClassConfig& cls, bool unit_present) {
    // 008E35F0..008E3606: scopes other than PUTT_FRIENDUNIT / PUTT_ENEMYUNIT pass.
    if (cls.target_type_84 != 1 && cls.target_type_84 != 2) return true;
    if (!unit_present) return false;                 // 008E3607..008E360E
    if (cls.target_filter_88 == 0x0F) return true;   // 008E3616..008E361F
    // 008E3620..008E367C: IsKindOf 8 / 6 / 18h and 009FE0B0 select a mask bit.
    // Every class 008ED4B0 uses at once is PUT_PASSIVE, scoped RANGE or PARTY.
    throw std::logic_error("008E35F0 unit-kind mask arms not modelled");
}

// 008E8C30 __thiscall(ECX = manager, record*), RET 4.
void expire_modifier_008e8c30(cp::PowerupRuntime& runtime, PowerupModifier* record) {
    cp::PowerupItem* const item = record->item;      // 008E8C4E
    if (item != nullptr) {
        if (item->cls->group_08 == 2) {
            item->word_00 -= 1;                      // 008E8C63, an active item
        } else {
            // 008E8C69..008E8C98: the item's pointer to this record goes, and
            // record+0 is cleared with it.
            for (auto it = item->modifiers.begin(); it != item->modifiers.end(); ++it) {
                if (*it == record) {
                    record->item = nullptr;
                    item->modifiers.erase(it);
                    break;
                }
            }
        }
    }
    // 008E8C9D..008E8D57: the record's node leaves its category list (008E8370).
    std::list<PowerupModifier>& list =
        runtime.category_lists.at(static_cast<std::size_t>(record->category));
    for (auto it = list.begin(); it != list.end(); ++it) {
        if (&*it == record) {
            require_unhosted_session(runtime, "008E8C30");  // 008E8CE0, message 68
            list.erase(it);
            break;
        }
    }
    // 008E8D5C..008E8D7A: an active item with no live record and no use left goes
    // through 008E87C0. No active item is used in this packet's reach.
    if (item != nullptr && item->cls->group_08 == 2 && static_cast<std::int32_t>(item->word_00) < 1 &&
        item->use_limit == 0) {
        throw std::logic_error("008E87C0 active item removal not modelled");
    }
}

// 008EA250 __thiscall(ECX = manager, category, multiplier, class, item, slot,
// unit), RET 18h.
void insert_modifier_008ea250(cp::PowerupRuntime& runtime, std::int32_t category,
    float multiplier, const PowerupClassConfig& cls, cp::PowerupItem* item,
    std::int32_t slot, const PowerupUnitView& unit) {
    // 008EA27B..008EA2FB: the template record appended to the category's list
    // (008E8480 links it before the head, so after the last record).
    std::list<PowerupModifier>& list =
        runtime.category_lists.at(static_cast<std::size_t>(category));
    list.push_back(PowerupModifier{});
    PowerupModifier* const record = &list.back();
    record->category = category;                     // 008EA353
    record->multiplier = multiplier;                 // 008EA362
    record->owner_slot = slot;                       // 008EA367
    record->cls = &cls;                              // 008EA35D..008EA38D, the name
    // 008EA390..008EA3AE: an active class ends `duration` after the clock (one
    // FADD, FSTP float); every other class at the 1.0e10 double 00D16220.
    if (cls.group_08 == 2) {
        namespace x8 = bsp::cc13x87;
        record->end_time = static_cast<float>(
            x8::x87_add24(cls.duration_7c, runtime.host->clock_00f876a4()));
    } else {
        record->end_time = static_cast<float>(1.0e10);
    }
    // 008EA3B1..008EA409: a passive item lists the record (008E5AA0 append);
    // another item counts it. record+0 names the item either way.
    if (item != nullptr) {
        if (cls.group_08 == 3) {
            item->modifiers.push_back(record);
        } else {
            item->word_00 += 1;
        }
        record->item = item;
    }
    // 008EA40B..008EA45E, by class+84h (table 008EA4C4).
    switch (cls.target_type_84) {
    case 1:
    case 2:
        record->unit_id = unit.id;                   // 008EA420, unit+174h
        break;
    case 3: {
        // 008EA434: [[record+0]+8]+4 is the item's binding's building; its +174h.
        if (item == nullptr || item->binding == cp::PowerupItem::kNoBinding) {
            throw std::logic_error("008EA434 range record without a binding");
        }
        record->centre_id = view_or_throw(item->binding, "008EA434").id;
        break;
    }
    case 4:
        record->slot = slot;                         // 008EA449
        break;
    case 5: {
        // 008EA44E: [[game+18CCh + slot*4]+28h], the slot's player's party.
        const int party = bsp::game::scene_slot_party(slot, -1);
        if (party < 0) {
            throw std::logic_error("008EA44E slot without a published party");
        }
        record->party = party;
        break;
    }
    default:
        break;                                       // 008EA417 JA
    }
    require_unhosted_session(runtime, "008EA250");   // 008EA461, message 68
}

// 008EADA0 __thiscall(ECX = manager, item, slot, unit), RET 0Ch, as 008ED4B0
// reaches it: a single-player use of a just-granted item.
void use_item_008eada0(cp::PowerupRuntime& runtime, cp::PowerupItem& item,
    std::int32_t slot, const PowerupUnitView& unit) {
    cp::PowerupManagerHost& host = *runtime.host;
    if (!host.powerups_enabled_00e0c978()) return;   // 008EADB8
    if (host.session_word_1fe4() == 2) {
        throw std::logic_error("008EADDA client use not modelled");
    }
    const PowerupClassConfig& cls = *item.cls;
    // 008EAE58..008EAEA7: the slot's +14Ch record for the class (00617030 inserts
    // a zeroed one), ready unless its deadline is still ahead of the clock.
    PowerUpSlot& ready = cp::slot_record_00617030(runtime, slot, &cls);
    const float now = host.clock_00f876a4();
    float ready_at = ready.absolute_expiry;
    if (ready.relative_deadline) {
        namespace x8 = bsp::cc13x87;
        ready_at = static_cast<float>(x8::x87_add24(ready.source_offset, now));
    }
    if (ready_at > now) return;
    // 008EAEAD: the air-support arm (class+8 == 1). 008ED4B0 uses passives only.
    if (cls.group_08 == 1) {
        throw std::logic_error("008EAEC3 air-support use not modelled");
    }
    if (!class_scope_accepts_unit_008e35f0(cls, true)) return;  // 008EAF6F
    // 008EAF7E..008EAFEB: item+0 cleared, then one insert per category whose
    // multiplier is not 1.0f (UCOMISS: an unordered compare inserts too).
    const bool active = cls.group_08 == 2;
    bool consumed = false;
    item.word_00 = 0;
    for (std::int32_t category = 0; category < kPowerupCategoryCount; ++category) {
        const float multiplier = cls.multipliers_8c[static_cast<std::size_t>(category)];
        if (!(multiplier == 1.0f)) {
            insert_modifier_008ea250(runtime, category, multiplier, cls, &item, slot, unit);
            consumed = active;
        }
    }
    // 008EAFED: only an active class goes on to the cooldown and use count.
    if (consumed) {
        throw std::logic_error("008EAFF8 active use tail not modelled");
    }
}

// 008ED4B0 __thiscall(ECX = manager, binding), RET 4: the binding's class to its
// building's slot, used at once when passive.
void grant_008ed4b0(cp::PowerupRuntime& runtime, std::size_t building,
    const PowerupClassConfig* cls) {
    cp::PowerupManagerHost& host = *runtime.host;
    if (!host.powerups_enabled_00e0c978()) return;   // 008ED4C8
    if (host.session_word_1fe4() == 2) return;       // 008ED4E1
    if (cls == nullptr) return;                      // 008ED4F3, binding+8
    const PowerupUnitView view = view_or_throw(building, "008ED4B0");
    const std::int32_t slot = view.slot;             // 008ED504, 009FFD20
    if (static_cast<std::uint32_t>(slot) > 7u) return;  // 008ED50B JA
    // 008ED514..008ED554: a fresh 008E6A80 item appended to the slot's list,
    // +4h the class, +8h the binding.
    std::list<cp::PowerupItem>& inventory = runtime.inventories[static_cast<std::size_t>(slot)];
    inventory.push_back(cp::PowerupItem{});
    cp::PowerupItem& item = inventory.back();
    item.cls = cls;
    item.binding = building;
    refresh_notify_gate(runtime, slot);
    ++runtime.counts.grants;
    // 008ED557..008ED597: 009789A0(slot, item, name) on [00F8A0C4], "pup_lost"
    // (00D1627C) for a building on party 2, else "pup_gain" (00D16270).
    host.report_powerup_009789a0(slot, item, view.party == 2 ? "pup_lost" : "pup_gain");
    ++runtime.counts.pup_gain_reports;
    refresh_hud_if_shown_008e4660(runtime);          // 008ED5A0
    require_unhosted_session(runtime, "008ED4B0");   // 008ED5AB, message 67
    // 008ED5F1..008ED601: a passive class (class+8 == 3) is used now.
    if (cls->group_08 == 3) use_item_008eada0(runtime, item, slot, view);
}

// 008E6230 __thiscall(ECX = item): each listed record expires (passive) or is
// released (+0 cleared), while the manager exists.
void release_item_modifiers_008e6230(cp::PowerupRuntime& runtime, cp::PowerupItem& item) {
    if (runtime.manager == nullptr) return;          // 008E6230
    for (auto it = item.modifiers.begin(); it != item.modifiers.end();) {
        PowerupModifier* const record = *it;
        ++it;                                        // 008E6275, next before the call
        if (item.cls->group_08 == 3) {
            expire_modifier_008e8c30(runtime, record);
        } else {
            record->item = nullptr;                  // 008E6287
        }
    }
}

// 008E8A30 __thiscall(ECX = manager, binding), RET 4: the first item (slots 0..7
// in order) granted through the binding leaves its slot.
void remove_items_bound_to_008e8a30(cp::PowerupRuntime& runtime, std::size_t building) {
    for (std::int32_t slot = 0; slot < static_cast<std::int32_t>(cp::kPowerupPlayerSlots);
         ++slot) {
        std::list<cp::PowerupItem>& inventory =
            runtime.inventories[static_cast<std::size_t>(slot)];
        for (auto it = inventory.begin(); it != inventory.end(); ++it) {
            if (it->binding != building) continue;   // 008E8A6D, item+8
            release_item_modifiers_008e6230(runtime, *it);  // 008E8AB1
            require_unhosted_session(runtime, "008E8A30");  // 008E8AC0, message 67
            // 008E8B37..008E8B88: an active item's records anywhere drop it.
            if (it->cls != nullptr && it->cls->group_08 == 2) {
                for (std::list<PowerupModifier>& list : runtime.category_lists) {
                    for (PowerupModifier& record : list) {
                        if (record.item == &*it) record.item = nullptr;
                    }
                }
            }
            inventory.erase(it);                     // 008E8B8E..008E8BF0
            refresh_notify_gate(runtime, slot);
            refresh_hud_if_shown_008e4660(runtime);  // 008E8BF4..008E8C09
            return;
        }
    }
}

}  // namespace

void command_building_powerup_init_008ed720(std::size_t building,
    std::string_view powerup_class_id) {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        // The host has no manager when the class registry did not load.
        if (runtime.manager != nullptr && runtime.host != nullptr) {
            // 008ED73E..008ED759: sessions 0 and 1 only; others clear binding+8.
            const std::int32_t session = runtime.host->session_word_1fe4();
            if (session == 0 || session == 1) {
                // 008ED7AB..008ED7C5: a "[" past the first character takes the
                // bracket arm, 008EC650's pick per "]" chunk, which draws on the
                // shared random stream (BSP_RandomThreads, 00BD2F10).
                const std::size_t bracket = powerup_class_id.find('[');
                if (bracket != std::string_view::npos && bracket > 0) {
                    throw std::logic_error("008EC650 shared-stream draw not modelled");
                }
                // 008ED964..008ED99D: the plain name (008E6CB0) into binding+8;
                // on a hit, the grant and the observer registration (00694A60).
                const PowerupClassConfig* cls =
                    cp::find_powerup_class_008e6cb0(runtime, powerup_class_id);
                if (cls != nullptr) {
                    grant_008ed4b0(runtime, building, cls);
                    runtime.observed_bindings[building] = cls;
                }
            }
        }
    } else {
        (void)building;
        (void)powerup_class_id;
    }
}

void powerup_manager_observed_event_008ed620(std::size_t unit, int code) {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        if (code == 5 && runtime.manager != nullptr && runtime.host != nullptr) {
            // Only bindings 008ED720 registered observe their building.
            const auto found = runtime.observed_bindings.find(unit);
            if (found != runtime.observed_bindings.end()) {
                // 008ED62A..008ED64C: the observed unit's vtable[4]()'s
                // vtable[64h](), its binding, then 008E8A30 and 008ED4B0.
                remove_items_bound_to_008e8a30(runtime, unit);
                grant_008ed4b0(runtime, unit, found->second);
            }
        }
    } else {
        (void)unit;
        (void)code;
    }
}

void expire_due_modifiers_008eb110() {
    if constexpr (kPowerUpSubsystemCc13Bound) {
        cp::PowerupRuntime& runtime = cp::powerup_runtime();
        if (runtime.manager != nullptr && runtime.host != nullptr) {
            // 008EB139..008EB181: FLD record+18h, FLD clock, FCOMI, JBE: a
            // record expires once the clock is past its end. The next node is
            // read before the call.
            const float now = runtime.host->clock_00f876a4();
            for (std::list<PowerupModifier>& list : runtime.category_lists) {
                for (auto it = list.begin(); it != list.end();) {
                    PowerupModifier* const record = &*it;
                    ++it;
                    if (now > record->end_time) expire_modifier_008e8c30(runtime, record);
                }
            }
        }
    }
}

}  // namespace bsp::game_powerups
