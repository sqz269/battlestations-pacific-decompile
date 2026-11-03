#pragma once

// Goal 4 chain 286 (kWireGoal4Chain286Bound). Adapters that let the mission Lua
// host's listener dispatchers (src/game_hosts_lua.cpp) test a subscription with
// the image's own matchers instead of inline set and bracket comparisons:
//
//  - `kill` (009813A0 -> 0097B8C0 -> vtable 00D1B68C slot 3):
//    0096ACE0 bsp::cc13_seg49_64_78::kill_matches_0096ace0, params
//    (victim, lastAttacker, lastAttackerPlayerIndex) against the sets
//    +0Ch / +1Ch / +2Ch; all three filters run, then combine.
//  - `entityKilled` (00986480 -> 0097B8C0 -> vtable 00D1B7D0 slot 3):
//    00968FF0 bsp::cc13_seg49_64_78::entity_killed_matches_00968ff0, params
//    (entity, killReason) against +0Ch (entity ids) and +1Ch (strings, _stricmp
//    order). Checked against the listing: 00968FFB..0096901C slot 0 on +0Ch with
//    params[0], 00969022..0096904D slot 0 on +1Ch with params[1], each behind the
//    00BF6713 size test.
//  - `hit` ranges (009685B0 keys 5..7 against +5Ch / +68h / +74h): the range
//    predicate 00968440 bsp::cc13_seg49_64_78::range_filter_matches_00968440,
//    ordered and inclusive, an unordered value fails.
//
// What stays with the host: its AddListener parse (0097E360's Load slots are not
// run; the host reads the block itself and keeps the sets in ListenerEntry), so
// each call builds the subscription's filters from those sets. Host entity ids
// (unit index + 1; scene markers from 50000) are copied into the native 16-bit id
// set; a member that does not fit a 16-bit handle can never equal the unit ids
// these dispatchers box, so it is left out, and a declared set left with no member
// matches nothing, as the host comparison it replaces did. Not ABI compatible:
// the subscription and parameter objects are the value models of
// bsp/engine_seg49_64_78_cc13.hpp, not the image's 14h-byte LuaObject boxes.

#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

#include "bsp/engine_seg49_64_78_cc13.hpp"
#include "bsp/oct18b_nc_low.hpp"

namespace bsp::lua_listener_native_matchers {

namespace seg64 = bsp::cc13_seg49_64_78;

// The EventSubscriptionHost both Matches slots take. entity_id_174 is a Load-slot
// service (009721C0) that no Matches slot reaches; 00BF6713 fires only on a
// parameter vector shorter than the slot reads, and these adapters always build
// the full vector.
class MatchesHost final : public seg64::EventSubscriptionHost {
public:
    std::uint16_t entity_id_174(std::int32_t) override {
        throw std::logic_error("lua_listener_native_matchers: 009721C0 is not reached by a Matches slot");
    }
    void invalid_parameter_00bf6713() override {
        throw std::logic_error("lua_listener_native_matchers: short parameter vector (00BF6713)");
    }
};

// Copies host entity ids into a native id set. False when the host set was
// declared non-empty but kept no member that fits a 16-bit handle.
inline bool fill_entity_ids(seg64::EntityIdFilter& filter, const std::vector<int>& ids,
                            bool declared) {
    for (const int id : ids) {
        if (id >= 0 && id <= 0xFFFF) filter.ids.insert(static_cast<std::uint16_t>(id));
    }
    return !declared || !filter.ids.empty();
}

// One boxed parameter: an entity id or an integer in the word at +4h, a float
// at +4h, or the NativeString at +4h.
inline seg64::EventParam word_param(int value) {
    seg64::EventParam param;
    param.word04 = static_cast<std::uint32_t>(value);
    return param;
}
inline seg64::EventParam number_param(float value) {
    seg64::EventParam param;
    param.number04 = value;
    return param;
}
inline seg64::EventParam text_param(const char* value) {
    seg64::EventParam param;
    param.text04 = value != nullptr ? value : "";
    return param;
}

// 0096ACE0's subscription from one host `kill` entry: entity +0Ch, lastAttacker
// +1Ch, lastAttackerPlayerIndex +2Ch. `last_attacker_declared` is the host's
// last_attacker_set (the block's lastAttacker set was non-empty even when none
// of its members was an entity table). False: the entry matches nothing.
inline bool build_kill_subscription(seg64::KillSubscription& subscription,
                                    const std::vector<int>& entity_ids,
                                    const std::vector<int>& last_attacker_ids,
                                    bool last_attacker_declared,
                                    const std::vector<int>& last_attacker_player_indices) {
    if (!fill_entity_ids(subscription.entity, entity_ids, !entity_ids.empty())) return false;
    if (!fill_entity_ids(subscription.last_attacker, last_attacker_ids,
                         last_attacker_declared || !last_attacker_ids.empty())) {
        return false;
    }
    subscription.last_attacker_player_index.values.insert(last_attacker_player_indices.begin(),
                                                          last_attacker_player_indices.end());
    return true;
}

// 00968FF0's subscription from one host `entityKilled` entry: entity +0Ch and
// killReason +1Ch. False: the entry matches nothing.
inline bool build_entity_killed_subscription(seg64::EntityKilledSubscription& subscription,
                                             const std::vector<int>& entity_ids,
                                             const std::vector<std::string>& kill_reasons) {
    if (!fill_entity_ids(subscription.entity, entity_ids, !entity_ids.empty())) return false;
    subscription.kill_reason.values.insert(kill_reasons.begin(), kill_reasons.end());
    return true;
}

// 00968440's filter from one host [min, max] bracket (the host keeps a range only
// when the block gave two values; native Load fills absent bounds with -1e10 / 1e10).
inline seg64::RangeFilter range_filter(float min, float max) {
    seg64::RangeFilter filter;
    filter.min = min;
    filter.max = max;
    return filter;
}

}  // namespace bsp::lua_listener_native_matchers
