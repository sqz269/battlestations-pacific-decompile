// Packet cc2_kill_credit. See include/bsp/kill_credit.hpp and docs/KILL_CREDIT.md.
// Ghidra was READ-ONLY for this packet; every name is a hypothesis.

#include "bsp/kill_credit.hpp"

namespace bsp {
namespace {

constexpr bool slot_in_range(int slot) noexcept
{
    return slot >= 0 && slot <= kKillCreditMaxPlayerSlot;
}

} // namespace

// 00779A00, `int __fastcall(descriptor)`. `ECX = [desc+8h]; ECX -= 2;` then an unsigned
// bound of 0Eh and a byte index table at 00779A5C into six answers plus the default 0.
// The recovered answers, in the order the arms appear: 6, 2, 3, 4, 5, 7. Category 1 is never
// produced; the plane slot at attacker+1B0h is what fills that hole (0077D025).
int kill_credit_ordnance_category_00779a00(int ordnance_kind) noexcept
{
    // The byte index table at 00779A5C is `00 00 06 01 01 01 06 02 03 04 06 06 06 06 05`
    // and the dword jump table at 00779A40 is {00779A21, 00779A2D, 00779A1B, 00779A33,
    // 00779A39, 00779A27, 00779A3E}, whose arms return {2, 4, 6, 5, 7, 3, 0}.
    static const int kArmResult[7] = {2, 4, 6, 5, 7, 3, 0};
    static const unsigned char kIndex[15] = {0, 0, 6, 1, 1, 1, 6, 2, 3, 4, 6, 6, 6, 6, 5};
    const unsigned index = static_cast<unsigned>(ordnance_kind - 2);
    if (index > 0x0Eu) {
        return 0; // 00779A08 `CMP ECX,0Eh` then `JA`, with EAX already zeroed
    }
    return kArmResult[kIndex[index]];
}

// 009FFD20, `int __fastcall(unit)`, body 009FFD20..009FFD5C.
int kill_credit_owning_slot_009ffd20(int unit_owning_slot,
                                     bool multiplayer,
                                     int unit_side,
                                     int local_player_side) noexcept
{
    if (unit_owning_slot == kKillCreditSlotFallbackAbove) {
        return -1; // 009FFD29 `JZ` to `OR EAX,0FFFFFFFFh`
    }
    if (static_cast<unsigned>(unit_owning_slot) <= 7u) {
        return unit_owning_slot; // 009FFD2E `JBE` straight to `RET`
    }
    if (multiplayer) {
        return -1; // 009FFD3C `JNZ` to the same `OR EAX,0FFFFFFFFh`
    }
    // 009FFD4E..009FFD55: `NEG/SBB/AND 4`, so 0 on a side match and 4 otherwise.
    return unit_side == local_player_side ? 0 : 4;
}

// 0077CE60's attribution block, 0077CF23..0077D122. Listing order, later writes winning.
KillAttributionFields kill_attribution_0077ce60(const KillAttributionFields& prior,
                                                bool has_source,
                                                const KillAttributionSource& source,
                                                const KillAttributionAttacker& attacker,
                                                const KillAttributionKamikaze& kamikaze,
                                                bool prior_attacker_matches) noexcept
{
    KillAttributionFields out = prior;
    out.stamp_written = true; // 0077CED7, before the shot is even fetched
    out.block_written = false;
    if (!has_source) {
        return out;
    }

    // 0077CF29 and 0077CF3E, both ahead of the owner-id test.
    out.ordnance_kind = source.ordnance_kind;
    out.attacker_side = source.owner_side;
    out.side_and_kind_written = true;

    // 0077CF44 `CMP word ptr [EAX+0x18],0x0`, the owner id inside the source record.
    if (source.owner_id == 0) {
        return out;
    }
    out.block_written = true;

    // 0077CF4F..0077CF73, the sole-attacker tri-state.
    if (prior.sole_attacker < 0) {
        out.sole_attacker = 1;
    } else if (!prior_attacker_matches) {
        out.sole_attacker = 0;
    }

    // 0077CF7E, 0077CC80 copies the source record's +14h/+18h into victim+2C4h/+2C8h.
    out.attacker_id = source.owner_id;

    if (source.has_owner_unit) {
        out.attacker_class = attacker.unit_class; // 0077CFBA
        // 0077CFB7 `CMP EBX,0x8` then `JBE`: the fallback fires only above 8.
        out.credited_slot = attacker.owning_slot > kKillCreditSlotFallbackAbove
                                ? source.origin_slot
                                : attacker.owning_slot;

        // The +2DCh chain. Four independent tests, each overwriting the last.
        if (attacker.is_plane) {
            out.origin_slot = source.origin_slot; // 0077CFDD
        }
        if (attacker.is_ship || attacker.is_land_fort) {
            out.origin_slot = attacker.category_slot; // 0077D010, attacker[+1ACh + cat*4]
        }
        if (attacker.is_plane && slot_in_range(attacker.plane_slot)) {
            out.origin_slot = attacker.plane_slot; // 0077D02E, attacker+1B0h
        }
        // 0077D043: a land fort above slot 7 credits the unit its +71Ch names.
        if (attacker.is_land_fort && out.credited_slot > kKillCreditMaxPlayerSlot &&
            attacker.fort_has_owner) {
            out.credited_slot = attacker.fort_owner_slot;
        }
        out.attacker_type_id = attacker.type_id; // 0077D07E
    } else {
        out.credited_slot = source.origin_slot; // 0077D08A, the no-owner arm
    }

    // 0077D090..0077D0DE, decided on the shot entity, not on the attacker unit.
    const bool kamikaze_hit =
        (kamikaze.shot_is_plane && kamikaze.shot_drops_kamikaze) ||
        kamikaze.shot_is_kamikaze_plane ||
        (kamikaze.shot_is_ship && kamikaze.shot_can_ram);
    if (kamikaze_hit) {
        out.kamikaze_override = true;
        out.attacker_class = kKillCreditKamikazeAttackerClass;
        // The same site also rewrites [src+1Ch] to [shot+1ACh], which is what the parent
        // propagation at 0077D11F then copies into parent+2D8h.
    }
    return out;
}

// 0091BE90..0091BF7A, the eight-slot loss pass. Both arms share the shape.
KillCreditLossPass kill_credit_loss_pass_0091bda0(int victim_side,
                                                  int victim_owning_slot,
                                                  int slot,
                                                  int slot_side) noexcept
{
    KillCreditLossPass pass;
    if (victim_side != 0 && victim_side != 1) {
        return pass; // 0091BE95 / 0091BEF9 both leave the slot alone
    }
    if (victim_owning_slot == slot || slot_side != victim_side) {
        pass.counted = true;
        pass.allied_map = victim_side == 0;
    }
    return pass;
}

// 0091C057..0091C0CF. Both maps are indexed with the ORIGINATING slot.
KillCreditTypeTally kill_credit_type_tally_0091bda0(int originating_slot,
                                                    bool attacker_is_ship,
                                                    bool attacker_is_plane,
                                                    int victim_type_id) noexcept
{
    KillCreditTypeTally tally;
    if (!slot_in_range(originating_slot)) {
        return tally; // 0091C05A `JA`
    }
    tally.slot = originating_slot;
    tally.victim_type_id = victim_type_id;
    tally.ship_map = attacker_is_ship;
    tally.plane_map = attacker_is_plane;
    return tally;
}

bool kill_credit_counts_rua_du(const KillCreditAwardInputs& in) noexcept
{
    // 0091C0DD IsKindOf(07h), 0091C0E9 `CMP [ESI+2D4h],0Ah`, 0091C0FC the two sides.
    return in.attacker_is_destroyer && in.ordnance_kind == kKillCreditRuaDuOrdnanceKind &&
           in.attacker_side != in.victim_side;
}

KillCreditAwardDecision kill_credit_awards_0091bda0(const KillCreditAwardInputs& in) noexcept
{
    KillCreditAwardDecision out;
    out.counter_rua_du = kill_credit_counts_rua_du(in);
    // 0091C194 `MOV ECX,[EDI]; CMP ECX,[EAX]; SETZ BL`: an exact equality, so the award
    // fires on the one kill that lands on the threshold, never again.
    out.grant_rua_du = out.counter_rua_du && in.rua_du_counter_after == in.rua_du_threshold;

    // 0091C276..0091C2FD. The distance compare is strict (`JBE` skips on <=), and the
    // relative-party call here takes the victim's side as the subject (0091C2EC swaps the
    // register roles relative to the kill-tree call at 0091BFD8).
    out.rua_bu_value = static_cast<int>(in.kill_distance);
    out.grant_rua_bu =
        slot_in_range(in.originating_slot) && in.attacker_is_battleship && in.victim_is_ship &&
        in.kill_distance > static_cast<float>(in.rua_bu_threshold) &&
        scoring_relative_party_00803510(in.victim_side, in.attacker_side) == kScoringPartyEnemy;

    // 0091C3F8..0091C4B8. Four class tests joined by OR, then the attacker's live side, its
    // plane slot, and the health percentage at or below the threshold.
    const bool ga_wy_class = in.attacker_is_battleship || in.attacker_is_destroyer ||
                             in.attacker_is_cruiser || in.attacker_is_mothership;
    const double health_percent =
        static_cast<double>(in.attacker_health) * kKillCreditHealthPercentScale;
    out.grant_ga_wy = ga_wy_class && in.victim_side != in.attacker_unit_side &&
                      in.attacker_has_plane_slot &&
                      health_percent <= static_cast<double>(in.ga_wy_threshold);
    return out;
}

void kill_credit_record_unit_kill_0091bda0(KillCreditHost& host, const UnitKillInputs& in)
{
    if (host.session_mode() == kKillCreditDisabledSessionMode) {
        return; // 0091BDC4
    }
    if (host.skip_friendly_losses() &&
        in.attribution.victim_side == host.local_player_side()) {
        return; // 0091BDD8..0091BDF9
    }
    if (host.root_entity_already_scored()) {
        return; // 0091BE3F, the map at manager+1488h
    }
    if (!in.victim_is_root) {
        return; // 0091BE48..0091BE65, a land fort that is not its own root
    }

    for (int slot = 0; slot <= kKillCreditMaxPlayerSlot; ++slot) {
        const KillCreditLossPass pass = kill_credit_loss_pass_0091bda0(
            in.attribution.victim_side, in.victim_owning_slot, slot, in.slot_sides[slot]);
        if (pass.counted) {
            host.add_loss(slot, pass.allied_map, in.victim_name);
        }
    }

    // 0091BF80..0091C046, the part docs/SCORING_BODIES.md already established.
    const ScoringKillCredit credit = scoring_kill_credit_0091bda0(in.attribution);
    if (!credit.recorded) {
        return;
    }
    ++host.kill_tree_leaf(credit.record_slot, scoring_kill_tree_offset(credit.player_tree),
                          credit.key);

    if (!in.has_attacker) {
        return; // 0091C051
    }

    const KillCreditTypeTally tally = kill_credit_type_tally_0091bda0(
        in.attribution.originating_slot, in.attacker_is_ship_base, in.attacker_is_plane_base,
        in.victim_type_id);
    if (tally.ship_map) {
        host.add_type_kill(tally.slot, kKillCreditShipKillsOffset, tally.victim_type_id);
    }
    if (tally.plane_map) {
        host.add_type_kill(tally.slot, kKillCreditPlaneKillsOffset, tally.victim_type_id);
    }

    KillCreditAwardInputs awards = in.awards;
    awards.credited_slot = credit.record_slot;
    awards.originating_slot = in.attribution.originating_slot;
    if (kill_credit_counts_rua_du(awards)) {
        awards.rua_du_counter_after =
            host.add_named_counter(credit.record_slot, kKillCreditCounterRuaDu);
        awards.rua_du_threshold = host.award_threshold(kKillCreditAwardRuaDu);
    }
    awards.rua_bu_threshold = host.award_threshold(kKillCreditAwardRuaBu);
    awards.ga_wy_threshold = host.award_threshold(kKillCreditAwardGaWy);

    const KillCreditAwardDecision decision = kill_credit_awards_0091bda0(awards);
    if (decision.grant_rua_du) {
        host.grant_award(credit.record_slot, kKillCreditAwardRuaDu, 1);
    }
    if (decision.grant_rua_bu) {
        host.grant_award(in.attribution.credited_slot, kKillCreditAwardRuaBu,
                         decision.rua_bu_value);
    }

    // 0091C392..0091C3F5, always reached once the attacker exists.
    host.update_kill_list(credit.record_slot,
                          kill_credit_kill_list_key(in.victim_type_id, in.attacker_type_id),
                          in.ordnance_kind, in.attribution.credited_slot,
                          in.sole_attacker != 0);

    if (decision.grant_ga_wy) {
        host.grant_award(in.attribution.credited_slot, kKillCreditAwardGaWy, 1);
    }
}

} // namespace bsp
