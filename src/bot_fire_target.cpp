// Reconstruction of the weapon director's automatic target selector and the
// gun-side aim rules. Evidence in docs/BOT_FIRE_TARGET.md; every routine here
// is a projection of one native body, and the coverage of each is recorded in
// that document's routine table.
#include "bsp/bot_fire_target.hpp"

#include <cmath>

namespace bsp {

// ---------------------------------------------------------------------------
// 009F65E0, the per-owner priority list
// ---------------------------------------------------------------------------
namespace {

void push_kind(AutoTargetSearchConfig& config, bool flag, int kind)
{
    AutoTargetPriorityEntry entry;
    entry.flag = flag;
    entry.entity_kind = kind;
    config.priority.push_back(entry);
}

} // namespace

AutoTargetSearchConfig auto_target_build_priority_009f65e0(
    const AutoTargetOwnerKinds& kinds) noexcept
{
    AutoTargetSearchConfig config;

    // 009F65F4: IsKindOf(0Eh) first.
    if (kinds.kind_0e) {
        config.max_range = kAutoTargetRangeKindE;
        push_kind(config, true, 0x08); // 009F6626
        push_kind(config, true, 0x0e); // 009F663E, EDI still holds 0Eh
        push_kind(config, true, 0x07); // 009F6652
        push_kind(config, true, 0x0b); // 009F666A
        push_kind(config, true, 0x0a); // 009F667E
        push_kind(config, true, 0x09); // 009F669A
        push_kind(config, true, 0x0d); // 009F66AE
        return config;
    }

    // 009F66CD: IsKindOf(7).
    if (kinds.kind_07) {
        config.max_range = kAutoTargetRangeKind7;
        push_kind(config, true, 0x09); // 009F66FB
        push_kind(config, true, 0x08); // 009F6713
        push_kind(config, true, 0x0d); // 009F672B
        push_kind(config, true, 0x07); // 009F6743
        push_kind(config, true, 0x0a); // 009F6750, then the shared tail
        push_kind(config, true, 0x0b); // 009F6808
        push_kind(config, true, 0x0e); // 009F681C
        return config;
    }

    // 009F6762: IsKindOf(0Ah).
    if (kinds.kind_0a) {
        config.max_range = kAutoTargetRangeKindA;
        push_kind(config, true, 0x07); // 009F6790
        push_kind(config, true, 0x0a); // 009F67A8
        push_kind(config, true, 0x09); // 009F67C0
        push_kind(config, true, 0x08); // 009F67D8
        push_kind(config, true, 0x0d); // 009F67E5, then the shared tail
        push_kind(config, true, 0x0b); // 009F6808
        push_kind(config, true, 0x0e); // 009F681C
        return config;
    }

    // 009F6837: IsKindOf(0Dh). The only branch besides kind 8 that pushes
    // entries with the flag clear.
    if (kinds.kind_0d) {
        config.max_range = kAutoTargetRangeKind7;
        push_kind(config, true, 0x0d);  // 009F6865
        push_kind(config, true, 0x09);  // 009F687D
        push_kind(config, true, 0x0a);  // 009F6895
        push_kind(config, true, 0x07);  // 009F68AD
        push_kind(config, false, 0x08); // 009F68C1 writes 0 to the flag byte
        push_kind(config, true, 0x0b);  // 009F68DE
        push_kind(config, false, 0x0e); // 009F68F2
        return config;
    }

    // 009F690E: IsKindOf(8). This branch pushes through 009F65B0.
    if (kinds.kind_08) {
        config.max_range = kAutoTargetRangeKindA;
        push_kind(config, true, 0x09);  // 009F6932
        push_kind(config, true, 0x0b);  // 009F693C
        push_kind(config, true, 0x0d);  // 009F6946
        push_kind(config, true, 0x0a);  // 009F6950
        push_kind(config, true, 0x07);  // 009F695A
        push_kind(config, false, 0x08); // 009F6965, the 0 pushed at 009F6961
        push_kind(config, false, 0x0e); // 009F696F
        return config;
    }

    // 009F6982: kinds 0Bh, 9 and 0Ch leave the list empty and the range zero,
    // so the scan can never accept a candidate. Every other owner falls out
    // without even writing the range.
    config.max_range = 0.0f;
    return config;
}

// ---------------------------------------------------------------------------
// 009F5B70, the per-candidate score
// ---------------------------------------------------------------------------
float auto_target_score_009f5b70(int entry_count, int index, float distance) noexcept
{
    // 009F5CB2 subtracts the 1-based counter, so the first entry is worth
    // (entry_count - 1) tiers. 009F5CC4 is FSUBRP: tiers*10000 - distance.
    const int tiers = entry_count - (index + 1);
    return static_cast<float>(tiers) * kAutoTargetTierWeight - distance;
}

bool auto_target_distance_in_range_009f5b70(float distance, float max_range) noexcept
{
    // 009F5C9B compares max_range against distance and rejects on JBE.
    return distance < max_range;
}

bool auto_target_score_beats_best_009f5b70(float score, float best) noexcept
{
    // 009F5CD1, rejected on JBE.
    return score > best;
}

bool auto_target_switch_allowed_009f52f0(float candidate_score,
                                         float retained_score) noexcept
{
    // 009F52F8 multiplies the retained score by the 00CE3D40 double before the
    // FCOMIP at 009F52FE; JBE returns 0.
    return candidate_score < retained_score * kGunAimSnapThreshold;
}

bool auto_target_owner_can_engage_009f59f0(
    bool candidate_is_kind_8,
    bool candidate_blocked,
    const std::vector<AutoTargetOwnerWeapon>& owner_devices) noexcept
{
    // 009F59F0 returns 1 unless the candidate is of kind 8 and 00852820 says
    // the extra check applies; only then does it walk the owner's +48h list.
    if (!candidate_is_kind_8 || candidate_blocked) {
        return true;
    }
    for (const AutoTargetOwnerWeapon& device : owner_devices) {
        if (!device.is_weapon_device) {
            continue;
        }
        if (device.weapon_class == kOwnerWeaponClassA ||
            device.weapon_class == kOwnerWeaponClassB) {
            return true; // the loop breaks and falls into the return 1
        }
    }
    return false; // 009F5A4x, the list ran out
}

AutoTargetScanResult auto_target_scan_009f5d30(
    const AutoTargetSearchConfig& config,
    const std::vector<AutoTargetCandidate>& candidates) noexcept
{
    AutoTargetScanResult result;
    // 009F5D42 and 009F5D49 clear the best pair before the walk.
    result.best = nullptr;
    result.best_score = 0.0f;

    const int entry_count = static_cast<int>(config.priority.size());
    for (const AutoTargetCandidate& candidate : candidates) {
        if (!candidate.passes_candidate_gate) {
            continue; // 009F5B89
        }
        if (candidate.zone_blocked && candidate.state_blocked) {
            continue; // 009F5BC8, both answers must hold to skip
        }
        if (candidate.skip_all_entries) {
            continue; // 009F5BFA forces every entry down the increment path
        }
        const int index = candidate.entity_kind_matches;
        if (index < 0 || index >= entry_count) {
            continue; // the walk hit the end of the list, 009F5BF4
        }
        if (!auto_target_distance_in_range_009f5b70(candidate.distance,
                                                    config.max_range)) {
            continue;
        }
        const float score =
            auto_target_score_009f5b70(entry_count, index, candidate.distance);
        if (!auto_target_score_beats_best_009f5b70(score, result.best_score)) {
            continue;
        }
        if (!candidate.owner_can_engage) {
            continue; // 009F5CE5
        }
        result.best = candidate.entity;      // 009F5CF1
        result.best_score = score;           // 009F5CF4
        result.issue_move_flag = candidate.issue_move_flag; // 009F5D02
    }
    return result;
}

// ---------------------------------------------------------------------------
// The gun-side rules
// ---------------------------------------------------------------------------
GunAimAngles gun_bot_angles_from_local_008fdaf0(float local_horz,
                                                float local_vert) noexcept
{
    GunAimAngles angles;
    // 008FDB0x: 00521370 writes the pair, then the horizontal half is
    // subtracted from -0.0f.
    angles.horz = kGunBotHorzAngleNegateBase - local_horz;
    angles.vert = local_vert;
    return angles;
}

float gun_bot_apply_aim_error_008ffa20(float angle, float draw) noexcept
{
    // 008FFE9x: draw * pi / 180 added to the angle. The draw itself comes from
    // 00BD2F10(0, errorDegrees) and is a contract, not a port.
    return angle + draw * kGunBotDegreesToRadiansNumerator /
                       kGunBotDegreesToRadiansDenominator;
}

bool gun_bot_wants_fire_008ffa20(bool firing, float distance, float max_range,
                                 float horz_error, float vert_error) noexcept
{
    const float error_sum = std::fabs(horz_error) + std::fabs(vert_error);
    if (!firing) {
        // 008FFEC8 and 008FFEF3: strict tests, the tighter pair.
        return distance < max_range - kGunBotOpenFireRangeMargin &&
               error_sum < kGunBotOpenFireAngleSum;
    }
    // 008FFEFB and 008FFF01: the wider pair, both non-strict.
    return distance <= max_range + kGunBotCeaseFireRangeMargin &&
           error_sum <= kGunBotCeaseFireAngleSum;
}

void gun_bot_trigger_step_008fef40(GunBotTriggerState& state, bool request,
                                   float dt) noexcept
{
    if (state.requested == request) {
        // 008FEF5x: only a non-negative timer counts down.
        if (state.timer >= 0.0f) {
            state.timer -= dt;
        }
        if (state.timer <= 0.0f) {
            state.committed = state.requested;
        }
        return;
    }
    state.requested = request;
    state.timer = request ? kGunBotTriggerPressDelay : kGunBotTriggerReleaseDelay;
}

// ---------------------------------------------------------------------------
// 009F5610 and 009F5DA0
// ---------------------------------------------------------------------------
bool auto_target_selection_enabled_009f5610(bool allow_move, bool has_command_slot,
                                            int command_slot_kind) noexcept
{
    if (!allow_move) {
        return false; // 009F5618
    }
    if (has_command_slot && (command_slot_kind == 1 || command_slot_kind == 2)) {
        return false; // 009F563x
    }
    return true;
}

void auto_target_tick_009f5da0(BotFireTargetHost& host, AutoTargetState& state,
                               void* unit, float dt)
{
    // 009F5DB5: the countdown is spent before anything else happens.
    if (dt < state.think_countdown) {
        state.think_countdown -= dt; // 009F5DF7
        return;
    }
    state.think_countdown += state.think_interval - dt; // 009F5DB9

    if (host.controller_belongs_to_another(unit)) {
        void* slot = host.director_command_slot();
        if (slot == nullptr) {
            return; // 009F5DD5
        }
        if (slot == reinterpret_cast<void*>(
                        static_cast<std::uintptr_t>(kTargetSelectorFollowCommandObject))) {
            return; // 009F5DE0, a follow order is left alone
        }
        host.release_controller(unit, 0); // 009F5DEB
        return;
    }

    if (host.unit_suppresses_targeting(unit)) {
        return; // 009F5E0D
    }

    if (!host.selection_enabled()) {
        if (host.director_command_state() == 2) {
            host.send_command_state(nullptr, 2); // 009F5F41, a tail call
        }
        return;
    }

    void* current = host.director_current_target(); // 009F5E2C
    void* chosen = nullptr;
    bool kept_current = false;
    if (current != nullptr && host.director_target_locked()) {
        void* command_target = host.build_command_target(current, 0.0f); // 009F5E4B
        if (host.command_accepts_target(kCommandObjectAttackMoveAddr,
                                        command_target)) {
            chosen = current; // 009F5E62
            kept_current = true;
        }
    }

    if (!kept_current) {
        if (host.director_command_state() != 2) {
            state.retained_score = kAutoTargetRetainedScoreReset; // 009F5E77
        }
        const AutoTargetScanResult scan = host.scan_party_list(); // 009F5E7F
        state.issue_move_flag = scan.issue_move_flag;
        chosen = scan.best;
        if (chosen == nullptr) {
            return; // 009F5E89
        }
        if (!auto_target_switch_allowed_009f52f0(scan.best_score,
                                                 state.retained_score)) {
            return; // 009F5EAE
        }
        state.retained_score = scan.best_score; // 009F5EB6
    }

    if (chosen == host.resolve_command_target_object()) {
        return; // 009F5ECB, the director is already pointed at it
    }
    if (!host.director_accepts_new_target()) {
        return; // 009F5ED7
    }
    if (state.issue_move_flag) {
        void* command_target = host.build_command_target(chosen, 0.0f); // 009F5EEA
        host.issue_command(kCommandObjectAttackMoveAddr, command_target); // 009F5EF8
    }
    // 009F5EFD: the fire target is only written when the director is not
    // already holding a locked one.
    const bool has_locked_target =
        (current == chosen || host.director_current_target() != nullptr) &&
        host.director_target_locked();
    if (!has_locked_target) {
        host.set_fire_target(chosen, false); // 009F5F21
    }
}

FireTargetOrder fire_target_order_008438b0(std::uint32_t command) noexcept
{
    FireTargetOrder order;
    order.command = command;
    if (command == kCommandObjectSetTargetAddr) {
        order.applies = true;
        order.clears = false; // 008438D5, the resolved object with force set
        return order;
    }
    if (command == kCommandObjectClearTargetAddr ||
        command == kCommandObjectClearOrdersAddr) {
        order.applies = true;
        order.clears = true; // 0084390x, a null target with force set
    }
    return order;
}

} // namespace bsp
