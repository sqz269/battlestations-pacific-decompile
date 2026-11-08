#pragma once

#include <memory>
#include <string>
#include <vector>

namespace bsp::cc13_seg49_64_78 {
struct EventSubscription;
struct ReconSubscription;
struct KillSubscription;
struct HitSubscription;
struct EntityKilledSubscription;
struct ExitZoneSubscription;
}

namespace bsp {
// Canonical payload of manager+F8h's channel/id maps. AddListener 00980C10
// produces the callback and filters from its actual Lua block (0097E360).
// The host fallback fields remain explicit projections of unsupported loads;
// no native dispatcher may reinterpret this C++ record as an image node.
struct WarningListenerEntry {
    std::string channel;
    std::string id;
    std::string callback;                 // subscription+4h
    std::vector<int> entity_ids;          // the `entity` set (+0Ch)
    bool attacker_filters_set{false};     // +1Ch or +2Ch not empty
    // `kill` under kLuaKillAttackerBound: lastAttacker (+1Ch, 009721C0) and
    // lastAttackerPlayerIndex (+2Ch, 009722D0). A non-empty set whose members
    // carry no entity id keeps last_attacker_set and matches nothing.
    bool last_attacker_set{false};
    std::vector<int> last_attacker_ids;
    std::vector<int> last_attacker_player_indices;
    // `recon` (00972450): oldLevel +1Ch, newLevel +2Ch, party +3Ch.
    std::vector<int> old_levels;
    std::vector<int> new_levels;
    std::vector<int> parties;
    // `hit` (009725B0): target +0Ch, attacker +2Ch, attackType +3Ch (strings),
    // damageCaused +5Ch as a [min, max] pair; the other keys only as non-empty flags.
    std::vector<int> attacker_ids;
    std::vector<std::string> attack_types;
    std::vector<float> damage_range;
    bool hit_filters_unmodelled{false};
    // Packet cc9_hit_listener_filters, under kLuaHitFilterFieldsBound.
    bool hit_device_filter{false};      // targetDevice (+1Ch), non-empty
    std::vector<float> fire_range;      // fireCaused (+68h)
    std::vector<float> leak_range;      // leakCaused (+74h)
    // Packet cc9_hit_attacker_player_index: attackerPlayerIndex (+4Ch).
    std::vector<int> attacker_player_indices;
    // `entityKilled` (00972F00) under kLuaEntityKilledListenersBound: entity +0Ch
    // (009721C0, kept in entity_ids) and killReason +1Ch (00970FF0).
    std::vector<std::string> kill_reasons;
    // `exitzone` (009726D0) under kLuaExitZoneListenersBound: entity +0Ch (in
    // entity_ids) and exited +1Ch, a boolean set held as 0/1.
    std::vector<int> exited_values;
    // Goal 4 chain 352 (kWireGoal4Chain352Bound): the subscription 0097E360
    // constructs for the channel and loads through its vtable slot 1 over the
    // block's reader. Set only when that load ran; the host sets above are then
    // left empty and the dispatchers match these filters instead.
    std::shared_ptr<const bsp::cc13_seg49_64_78::KillSubscription> native_kill;
    std::shared_ptr<const bsp::cc13_seg49_64_78::EntityKilledSubscription>
        native_entity_killed;
    std::shared_ptr<const bsp::cc13_seg49_64_78::ExitZoneSubscription> native_exit_zone;
    std::shared_ptr<const bsp::cc13_seg49_64_78::ReconSubscription> native_recon;
    std::shared_ptr<const bsp::cc13_seg49_64_78::HitSubscription> native_hit;
    // The other eighteen channels 0097E360 accepts (input ... hpEvent), which no
    // host dispatcher fires: the loaded subscription, held as the channel map
    // holds it (the owner keeps the derived type's deleter).
    std::shared_ptr<const bsp::cc13_seg49_64_78::EventSubscription> native_other;
};
} // namespace bsp
