// cc13_seg68: segment 68 (009F6090-009FE120) reference models. Packet
// cc13-seg68; evidence: docs/CC13_SEG68.md. Reconstructed and build-tested
// only; not wired into the game, not fixture- or game-validated.

#include "bsp/cc13_seg68.hpp"
#include "bsp/attack_commands.hpp"
#include "bsp/observer_edges.hpp"

#include <cmath>
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace bsp {
namespace cc13_seg68 {
namespace {

// 00CE3820, the double the squared-distance tests compare against.
constexpr double kDistanceEpsilon = 1e-10;

// The squared distance 009F7150 / 009F72A0 form: three FSUB/FSTP float deltas,
// then FLD ST1 / FMULP ST2 / FLD ST2 / FMULP ST3 / FXCH / FADDP ST2 / FMUL
// ST0,ST0 / FADDP, i.e. (dx*dx + dy*dy) + dz*dz, stored float (009F71CC).
float squared_distance3(const std::array<float, 3>& a, const std::array<float, 3>& b) noexcept
{
    const float dx = a[0] - b[0];
    const float dy = a[1] - b[1];
    const float dz = a[2] - b[2];
    return (dx * dx + dy * dy) + dz * dz;
}

// 009F71D0..009F720D: FCOMI against 1e-10 (JBE: equal or NaN gives 0), the
// CRT sqrt 00BF7030 on the stored float, FSTP float.
float distance_or_zero(float squared) noexcept
{
    if (static_cast<double>(squared) > kDistanceEpsilon) return std::sqrt(squared);
    return 0.0f;
}

std::array<float, 3> position_fc(SquadronAutoTargetHost& host, Handle entity)
{
    return {host.f32(entity, 0xfc), host.f32(entity, 0x100), host.f32(entity, 0x104)};
}

void refresh_if_stale(SquadronAutoTargetHost& host, Handle entity)
{
    // CMP byte [entity+C8h], 0 / JNZ, then 00414DB0 with ECX = entity.
    if (host.u8(entity, 0xc8) == 0) host.refresh_pose_00414db0(entity);
}

bool entity_slot_140_blocks(SquadronAutoTargetHost& host, Handle entity)
{
    // vtable[140h]() twice: non-null and the byte at its +1D4h set.
    if (host.entity_vt140(entity) == 0) return false;
    return host.u8(host.entity_vt140(entity), 0x1d4) != 0;
}

}  // namespace

// ===========================================================================
// Weapon-director auto-target
// ===========================================================================

void auto_target_search_init_009f69c0(WeaponDirectorAutoTargetHost& host,
                                      WeaponDirectorAutoTarget& self, Handle unit)
{
    // 009F69C0: the vector triple +4h/+8h/+0Ch and the range +10h cleared,
    // owner +14h, best +18h = 0, then 009F65E0(ECX = searchState, unit).
    self.search = AutoTargetSearchConfig{};
    self.search_owner_24 = unit;
    self.best_candidate_28 = 0;
    // 009F65E0 replaces the list and, on the kinds that set it, the range; the
    // kinds 0Bh/9/0Ch store 0 and every other owner leaves the range at the 0
    // stored above (bot_fire_target.cpp).
    const AutoTargetSearchConfig built =
        auto_target_build_priority_009f65e0(host.owner_kinds_009f65e0(unit));
    self.search = built;
}

WeaponDirectorAutoTarget& weapon_director_auto_target_construct_009f6a20(
    WeaponDirectorAutoTargetHost& host, WeaponDirectorAutoTarget& self, Handle director)
{
    self.owner_director_04 = director;                     // 009F6A42
    self.vtable_00 = kAutoTargetVtableDerived;             // 009F6A45
    self.unit_08 = host.director_unit_24c(director);       // 009F6A51
    // 009F6A54..009F6A66: [director+24Ch] reloaded as the argument.
    auto_target_search_init_009f69c0(host, self, host.director_unit_24c(director));
    self.state.think_interval = 1.0f;                      // 009F6A8A, 00D7A24C
    // 009F6A8F: 00BD2F10 with ECX = 1 (stream 1), (0.0f, 1.0f); FCHS; FSTP.
    self.state.think_countdown = -host.random_uniform_00bd2f10(1, 0.0f, 1.0f);
    self.director_0c = director;                           // 009F6AA5
    self.state.retained_score = kAutoTargetRetainedScoreReset;  // 009F6AA8, 00D7A248
    return self;
}

void weapon_director_auto_target_destruct_009f60c0(WeaponDirectorAutoTarget& self)
{
    // 009F60C0: derived vtable, _free(+14h) when non-null, +14h/+18h/+1Ch
    // cleared, base vtable 00D21B40.
    self.vtable_00 = kAutoTargetVtableDerived;
    self.search.priority.clear();
    self.search.priority.shrink_to_fit();
    self.vtable_00 = kAutoTargetVtableBase;
}

bool command_controller_fire_forbidden_009f6ad0(std::uint8_t byte_3c) noexcept
{
    // CMP byte [ECX+3Ch], 0 / SETE AL / RET.
    return byte_3c == 0;
}

Handle auto_target_ref_inner_owner_009f6af0(SquadronAutoTargetHost& host, Handle ref)
{
    // MOV EAX,[ECX] / MOV EAX,[EAX+4] / RET.
    return host.u32(host.u32(ref, 0), 4);
}

// ===========================================================================
// Squadron free-attack auto-target
// ===========================================================================

void filter_c_construct_009f7210(SquadronAutoTargetHost& host, SquadronAutoTarget& self)
{
    // 009F7231..009F725B: owner +4h, vtable 00D21BE4, then the byte at +8h is
    // [[owner+8]+35Ch]->vtable[18h](13h).
    self.filter_c_vtable_1c = kFilterCVtable;
    const Handle cls = host.u32(self.squadron_08, 0x35c);
    self.filter_c_leader_class_13_24 = host.class_query_18(cls, 0x13) ? 1 : 0;
}

SquadronAutoTarget& squadron_auto_target_construct_009f7590(
    SquadronAutoTargetHost& host, SquadronAutoTarget& self, Handle director,
    Handle squadron, std::uintptr_t selector_native_address)
{
    self.director_04 = director;                       // 009F75B3
    self.vtable_00 = kSquadronAutoTargetVtable;        // 009F75BA
    self.squadron_08 = squadron;                       // 009F75C0
    self.filter_a_vtable_0c = kFilterAVtable;          // 009F75CC, owner +10h = this
    self.filter_b_vtable_14 = kFilterBVtable;          // 009F75D9, owner +18h = this
    filter_c_construct_009f7210(host, self);           // 009F75E8
    self.rockets_previous_28 = 0;                      // 009F75F4
    self.selector_2c.filter_04 = SquadronFilter::At14; // 009F75F7, +30h = this+14h
    self.selector_2c.target_08 = 0;                    // 009F75FA
    self.selector_2c.owner_00 = &self;                 // 009F75FD
    self.selector_2c.interval_0c = 0.0f;               // 009F7600
    self.selector_2c.native_address = selector_native_address;
    self.latch_3c = 0;                                 // 009F7605
    self.latch_3d = 0;
    self.latch_3e = 0;
    return self;
}

void squadron_auto_target_clear_latches_009f6dd0(SquadronAutoTarget& self) noexcept
{
    self.latch_3e = 0;
    self.latch_3c = 0;
    self.latch_3d = 0;
}

Handle squadron_choose_attack_command_009f6d60(SquadronAutoTargetHost& host,
                                               const SquadronAutoTarget& self,
                                               Handle target, std::uint8_t flag)
{
    // ECX = [this+8]; PUSH 1, the flag, the target; CALL 007EEC50; RET 8.
    return host.choose_attack_command_007eec50(self.squadron_08, target, flag, 1);
}

bool filter_enabled_slot4(SquadronFilter filter) noexcept
{
    // 009F6C10 (A, C): MOV AL,1. 009F7070 (B): XOR AL,AL.
    return filter != SquadronFilter::At14;
}

bool filter_slot10(SquadronFilter filter) noexcept
{
    // 009F70E0 (A): true. 009F7090 (B) and 009F7270 (C): false.
    if (filter == SquadronFilter::At0C) return true;
    return filter == SquadronFilter::At14 ? filter_slot10_false_009f7090()
                                          : filter_c_slot10_false_009f7270();
}

float filter_default_score_009f6b10() noexcept
{
    return 0.0f;  // FLDZ / RET 4
}

bool filter_slot10_false_009f7090() noexcept { return false; }
bool filter_c_slot10_false_009f7270() noexcept { return false; }

bool filter_accept_unit_009f7010(SquadronAutoTargetHost& host, Handle entity)
{
    if (entity == 0) return false;                     // 009F7015
    return !entity_slot_140_blocks(host, entity);      // 009F7019..009F703C
}

float filter_distance_to_target_009f7150(SquadronAutoTargetHost& host,
                                         const SquadronAutoTarget& self, Handle target)
{
    const Handle squadron = self.squadron_08;          // [[filter+4]+8]
    refresh_if_stale(host, squadron);
    refresh_if_stale(host, target);
    return distance_or_zero(
        squared_distance3(position_fc(host, target), position_fc(host, squadron)));
}

float filter_c_distance_score_009f72a0(SquadronAutoTargetHost& host,
                                       const SquadronAutoTarget& self, Handle target)
{
    Handle squadron = self.squadron_08;
    refresh_if_stale(host, squadron);
    refresh_if_stale(host, target);
    // 009F72D2..009F734A: the squadron-to-target distance.
    const float d1 = distance_or_zero(
        squared_distance3(position_fc(host, target), position_fc(host, squadron)));
    // 009F7350: 0071E4C0 on [[filter+4]+4] (the director).
    const PathPoint path = host.current_path_point_0071e4c0(self.director_04);
    std::array<float, 3> from{};
    if (path.valid) {
        from = path.p;                                 // 009F736F..009F738D
    } else {
        squadron = self.squadron_08;                   // 009F738F reloads [[filter+4]+8]
        refresh_if_stale(host, squadron);
        from = position_fc(host, squadron);
    }
    refresh_if_stale(host, target);                    // 009F73C9
    // 009F73DF..009F7454: the target-to-path-point distance, same order.
    const float d2 = distance_or_zero(squared_distance3(position_fc(host, target), from));
    // 009F745A FLD d2 / FMUL double 0.5 (00D7A280) / FADD d1 / FSTP float.
    return d2 * 0.5f + d1;
}

bool filter_c_accept_guard_circle_009f7f40(SquadronAutoTargetHost& host,
                                           const SquadronAutoTarget& self, Handle target)
{
    // 009F7F40: 0071E4C0 on [[filter+4]+4]; no path point accepts (009F7FD3).
    const PathPoint path = host.current_path_point_0071e4c0(self.director_04);
    if (!path.valid) return true;
    refresh_if_stale(host, target);
    const float dx = host.f32(target, 0xfc) - path.p[0];   // 009F7F82
    const float dz = host.f32(target, 0x104) - path.p[2];  // 009F7F90
    const Handle first = host.game_tuning_0042e740();      // 009F7F94 -> ESI
    const Handle second = host.game_tuning_0042e740();     // 009F7F9B -> EAX
    // 009F7FA0..009F7FB0: FMUL ST0,ST0 on dx, FLD ST1 / FMULP ST2 on dz,
    // FADDP: dx*dx + dz*dz, FSTP float.
    const float d2 = dx * dx + dz * dz;
    // 009F7FB8: FLD [second+594h] / FMUL [first+594h] (GuardDist squared).
    const float r2 = host.f32(second, 0x594) * host.f32(first, 0x594);
    // 009F7FC5 FCOMPI ST0 = r2, ST1 = d2 / JA: inside when r2 > d2.
    return r2 > d2;
}

bool filter_accept_slotc(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                         SquadronFilter filter, Handle target)
{
    if (filter == SquadronFilter::At1C) return filter_c_accept_guard_circle_009f7f40(host, self, target);
    return filter_accept_unit_009f7010(host, target);
}

float filter_distance_slot18(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                             SquadronFilter filter, Handle target)
{
    // Slot +18h: 009F7150 in A, 009F72A0 in C. B has no slot +18h and its
    // score (009F7080) never reaches one.
    if (filter == SquadronFilter::At1C) return filter_c_distance_score_009f72a0(host, self, target);
    return filter_distance_to_target_009f7150(host, self, target);
}

int ai_attack_list_rank_009f7b00(SquadronAutoTargetHost& host, Handle vector, Handle entity)
{
    // 009F7B08..009F7B18: count = (end - begin) / 4, or 0 for a null begin.
    // The checked-iterator traps (00BF6713) of the image are not reproduced.
    const Handle begin = host.u32(vector, 4);
    const Handle end = host.u32(vector, 8);
    int remaining = begin == 0 ? 0 : static_cast<int>((end - begin) >> 2);
    for (Handle it = begin; it != end; it += 4) {
        // 009F7B40: entity->vtable[5Ch](*it); a hit returns the remaining count.
        if (host.is_kind_of_5c(entity, static_cast<int>(host.u32(it, 0)))) return remaining;
        --remaining;  // 009F7B52
    }
    return 0;         // 009F7B70
}

bool unit_is_kind17_with_flag_c24_009f6be0(SquadronAutoTargetHost& host, Handle unit)
{
    // IsKindOf(17h), then CMP byte [unit+C24h], 0 (009F6BF0). The Ghidra name
    // says +824h; the displacement in the bytes is C24h.
    if (!host.is_kind_of_5c(unit, 0x17)) return false;
    return host.u8(unit, 0xc24) != 0;
}

int ai_attack_class_weight_009f9770(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                                    Handle target, std::uint32_t arg)
{
    if (target == 0) return 0;                                   // 009F977A
    if (entity_slot_140_blocks(host, target)) return 0;          // 009F9780..009F97A3
    // 009F97A9..009F97B7: 007EEC50(ECX = [[filter+4]+8], target, arg, 1).
    const Handle cmd = host.choose_attack_command_007eec50(
        self.squadron_08, target, static_cast<std::uint8_t>(arg), 1);
    std::uint32_t offset = 0;
    if (cmd == kCmdLevelBomb) offset = 0x24;              // LevelBomb
    else if (cmd == kCmdDiveBomb) offset = 0x14;          // DiveBomb
    else if (cmd == kCmdTorpedo) offset = 0x34;           // TorpedoBomb
    else if (cmd == kCmdDepthCharge) offset = 0x04;       // DepthCharge
    else if (cmd == kCmdKamikaze) offset = 0x64;          // Kamikaze
    else if (cmd == kCmdDropKamikaze) offset = 0x64;      // Kamikaze
    else if (cmd == kCmdRocket) offset = 0x74;            // Strike
    else if (cmd == kCmdDogfight || cmd == kCmdStrafe) {
        // 009F9895..009F98C1: the leader [[filter+4]+8]+3D0h; IsKindOf(13h) or
        // 009F6BE0 picks FighterGun (+44h), otherwise NonFighterGun (+54h).
        const Handle leader = host.u32(self.squadron_08, 0x3d0);
        const bool fighter = host.is_kind_of_5c(leader, 0x13) ||
                             unit_is_kind17_with_flag_c24_009f6be0(host, host.u32(self.squadron_08, 0x3d0));
        offset = fighter ? 0x44 : 0x54;
    } else {
        return 0;                                          // 009F98EF
    }
    // 004D6CE0 is called inside each arm, then 009F7B00(ECX = block, block+off, target).
    const Handle block = host.ai_weights_block_004d6ce0();
    return ai_attack_list_rank_009f7b00(host, block + offset, target);
}

float filter_score_target_009f7ba0(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                                   SquadronFilter filter, Handle target,
                                   SquadronCandidateTrace* trace)
{
    // Packet cc13-fapick: `trace` only records which return was taken.
    auto note = [trace](SquadronCandidateNote n) {
        if (trace != nullptr) trace->note = n;
        return 0.0f;
    };
    if (target == 0) return note(SquadronCandidateNote::NotUnitOrSquadron);   // 009F7BAD
    if (!host.is_kind_of_5c(target, 5)) {                      // 009F7BB8
        if (!host.is_kind_of_5c(target, 0x18))                 // 009F7BC7
            return note(SquadronCandidateNote::NotUnitOrSquadron);
        target = host.u32(target, 0x3d0);                      // 009F7BD5, the leader
        if (target == 0) return note(SquadronCandidateNote::NotUnitOrSquadron);
    }
    if (!filter_accept_slotc(host, self, filter, target))      // vtable[0Ch]
        return note(filter == SquadronFilter::At1C ? SquadronCandidateNote::LeaderGuardOut
                                                   : SquadronCandidateNote::Slot140Blocks);
    // 009F7BF5..009F7C08: 00804350(ECX = [[filter+4]+8]+3D0h, EDX = target, 0FFh).
    const Handle own_leader = host.u32(self.squadron_08, 0x3d0);
    if (!host.sensors_detect_unit_00804350(own_leader, target, 0xff))
        return note(SquadronCandidateNote::NoDetect);
    // vtable[10h]() then vtable[14h](target, it): the AttackLists rank.
    const bool slot10 = filter_slot10(filter);
    const int weight = ai_attack_class_weight_009f9770(host, self, target, slot10 ? 1u : 0u);
    if (weight == 0) return note(SquadronCandidateNote::NoRank);   // 009F7C2B
    if (trace != nullptr) trace->rank = weight;
    // vtable[18h](target), FSTP float; 00415550(&1.0f, &d) answers 1.0 when
    // 1.0 > d, else d (a NaN d is kept).
    const float d = filter_distance_slot18(host, self, filter, target);
    const float m = 1.0f > d ? 1.0f : d;
    // 009F7C56 FDIVR double 0.5 / FSTP float, FIADD weight / FSTP float.
    const float inverse = 0.5f / m;
    return inverse + static_cast<float>(weight);
}

float filter_score_slot8(SquadronAutoTargetHost& host, const SquadronAutoTarget& self,
                         SquadronFilter filter, Handle target, SquadronCandidateTrace* trace)
{
    // Slot +8h: 009F7BA0 in A and C, 009F7080 (FLDZ) in B.
    if (filter == SquadronFilter::At14) return filter_default_score_009f6b10();
    return filter_score_target_009f7ba0(host, self, filter, target, trace);
}

Handle unit_resolve_kind0f_owner_009f7b80(SquadronAutoTargetHost& host, Handle unit)
{
    if (unit != 0 && host.is_kind_of_5c(unit, 0x0f)) return host.u32(unit, 0x9d4);
    return unit;
}

bool selector_candidate_allowed_009f7480(SquadronAutoTargetHost& host,
                                         const SquadronSelector& selector, Handle entity,
                                         SquadronCandidateTrace* trace)
{
    // Packet cc13-fapick: `trace` only records which return was taken.
    auto drop = [trace](SquadronCandidateNote n) {
        if (trace != nullptr) trace->note = n;
        return false;
    };
    if (host.is_kind_of_5c(entity, 0x0f)) return drop(SquadronCandidateNote::SinglePlane);   // 009F7491
    if (host.is_kind_of_5c(entity, 6)) {                                 // 009F74A7
        refresh_if_stale(host, entity);
        // 009F74BD: 0071C4F0(ECX = [00E188A8], entity+FCh).
        if (host.point_outside_map_0071c4f0(host.ggame_00e188a8(), entity + 0xfc) &&
            host.command_blocks_auto_action_00811f50(entity, 0))
            return drop(SquadronCandidateNote::OutsideMapBlocked);
    }
    if (host.is_kind_of_5c(entity, 0x18) &&                              // 009F74E9
        host.squadron_outside_map_detached_007ee8b0(entity))
        return drop(SquadronCandidateNote::SquadronDetached);
    if (entity_slot_140_blocks(host, entity))                            // 009F74FA
        return drop(SquadronCandidateNote::Slot140Blocks);
    if (host.is_kind_of_5c(entity, 0x1b)) {                              // 009F752D
        const Handle inner = host.entity_vt140(entity);
        if (host.is_kind_of_5c(inner, 0x46) || host.is_kind_of_5c(inner, 0x45))
            return drop(SquadronCandidateNote::Kind1bCarrier);
    }
    // 009F7567: [selector+4] null accepts; else filter->vtable[0Ch](entity).
    // The selector's filter is never null in this model.
    if (!filter_accept_slotc(host, *selector.owner_00, selector.filter_04, entity))
        return drop(selector.filter_04 == SquadronFilter::At1C ? SquadronCandidateNote::GuardOut
                                                               : SquadronCandidateNote::Slot140Blocks);
    return true;
}

void selector_pick_best_009f7810(SquadronAutoTargetHost& host, SquadronSelector& selector,
                                 float /*dt*/)
{
    // 009F781A..009F7838: SAR the selector's own address by 4, AND 0FFh, FILD,
    // FMUL double 0.001f (00D7A318), FADD double 0.9f (00D7A390), FSTP float.
    const int spread = static_cast<int>((static_cast<std::int32_t>(selector.native_address) >> 4) & 0xff);
    selector.interval_0c = static_cast<float>(spread) * 0.001f + 0.9f;
    const SquadronAutoTarget& owner = *selector.owner_00;
    // 009F783B: 008053C0(ECX = [[owner+8]+54h]) and the list at +DE8h.
    const Handle slot = host.recon_slot_008053c0(host.u32(owner.squadron_08, 0x54));
    Handle best = 0;
    float best_score = 0.0f;
    for (Handle node = host.u32(slot, 0xde8); node != 0; node = host.u32(node, 4)) {
        const Handle entity = host.u32(host.u32(node, 8), 4);
        if (entity == 0) continue;
        // Packet cc13-fapick: the trace is filled by the same calls, then reported.
        SquadronCandidateTrace trace;
        if (!selector_candidate_allowed_009f7480(host, selector, entity, &trace)) {
            host.trace_candidate_009f7810(entity, trace);
            continue;
        }
        if (entity_slot_140_blocks(host, entity)) {                      // 009F7876..009F7899
            trace.note = SquadronCandidateNote::Slot140Blocks;
            host.trace_candidate_009f7810(entity, trace);
            continue;
        }
        const float score = filter_score_slot8(host, owner, selector.filter_04, entity, &trace);
        trace.score = score;
        host.trace_candidate_009f7810(entity, trace);
        // 009F78B2 FCOMPI ST0 = score, ST1 = best / JBE: strictly greater wins.
        if (score > best_score) {
            best_score = score;
            best = entity;
        }
    }
    if (selector.target_08 != 0) {
        // 009F78D5..009F78F6: the current target keeps the slot unless the new
        // best beats its score + 0.05 (double 00D7A270, 0.05f widened).
        const float current = filter_score_slot8(host, owner, selector.filter_04, selector.target_08);
        if (!(best_score > current + 0.05f)) return;
    }
    selector.target_08 = best;                                           // 009F78F8
}

void selector_update_009f7910(SquadronAutoTargetHost& host, SquadronSelector& selector,
                              float dt)
{
    if (filter_enabled_slot4(selector.filter_04)) {
        selector_pick_best_009f7810(host, selector, dt);  // 009F792B, dt forwarded
        return;
    }
    selector.target_08 = 0;                               // 009F7934
}

bool bot_within_guard_distance_009f7fe0(SquadronAutoTargetHost& host,
                                        const SquadronAutoTarget& self, Handle descriptor,
                                        float scale)
{
    // 009F7FE0: 0071E4C0 on [this+4]; no path point answers true.
    const PathPoint path = host.current_path_point_0071e4c0(self.director_04);
    if (!path.valid) return true;
    float tx = 0.0f;
    float tz = 0.0f;
    if (host.u8(descriptor, 0) != 0) {
        // 009F8021: 00521EA0(ECX = descriptor), the entity's position.
        const Handle entity = host.command_target_resolve_00521ea0(descriptor);
        refresh_if_stale(host, entity);
        tx = host.f32(entity, 0xfc);
        tz = host.f32(entity, 0x104);
    } else {
        // 009F8056: descriptor+8h when byte +1 is set, else 00F87574.
        const Handle point = host.u8(descriptor, 1) != 0 ? descriptor + 8
                                                          : host.static_zero_vector_00f87574();
        tx = host.f32(point, 0);
        tz = host.f32(point, 8);
    }
    const float dx = tx - path.p[0];                     // 009F807D
    const float dz = tz - path.p[2];                     // 009F8089
    const Handle first = host.game_tuning_0042e740();    // 009F8091 -> ESI
    const Handle second = host.game_tuning_0042e740();   // 009F8098 -> EAX
    // 009F80A5..009F80B3: FLD ST1 / FMULP ST2 (dx*dx), FLDZ / FMUL ST0,ST0 /
    // FADDP ST2 (+ 0*0), FMUL ST0,ST0 (dz*dz), FADDP: (dx*dx + 0) + dz*dz.
    const float d2 = (dx * dx + 0.0f * 0.0f) + dz * dz;
    // 009F80BB..009F80D0: ((first.GuardDist * second.GuardDist) * scale) * scale.
    const float r2 = ((host.f32(first, 0x594) * host.f32(second, 0x594)) * scale) * scale;
    return r2 > d2;                                      // 009F80D2 FCOMPI / JBE
}

void squadron_auto_target_tick_009f8470(SquadronAutoTargetHost& host, SquadronAutoTarget& self,
                                        Handle self_native, float dt)
{
    // 009F847B: 009F6C80(ECX = this) picks the filter into the selector,
    // the target is cleared, then 009F8160 and 009F7C90 run.
    self.selector_2c.filter_04 = host.pick_filter_009f6c80(self_native);
    self.selector_2c.target_08 = 0;
    host.revalidate_current_command_009f8160(self_native);
    host.ordnance_latch_009f7c90(self_native);

    const Handle director = self.director_04;
    const std::uint32_t state = host.u32(director, 0x30);
    if (state != 1) {
        // 009F8640: state 2 with [director+3Ch] clear tail-jumps to 0071D9E0(2).
        if (state == 2 && host.u8(director, 0x3c) == 0)
            host.raise_override_stage_0071d9e0(director, 2);
        return;
    }
    const Handle leader = host.u32(self.squadron_08, 0x3d0);
    const std::uint32_t party = host.u32(leader, 0x1b0);
    bool run = false;
    if (party == 8 || host.party_slot_is_ai_held_00927f10(leader, party))
        run = host.u8(host.u32(self.squadron_08, 0x3d0), 0x184) == 0;   // 009F84CC
    if (!run) {
        // 009F84D9..009F84F8.
        if (host.u8(director, 0x3c) == 0) return;
        if (host.u8(director, 0x3d) == 0) return;
        if (host.u32(self.squadron_08, 0x370) != 2) return;
    }
    // 009F84FE: selector filter->vtable[4]().
    if (filter_enabled_slot4(self.selector_2c.filter_04))
        selector_pick_best_009f7810(host, self.selector_2c, dt);
    else
        self.selector_2c.target_08 = 0;
    if (self.selector_2c.target_08 == 0) return;                          // 009F8520
    if (!host.director_accepts_new_target_0071df70(self.director_04)) return;
    if (host.u32(self.squadron_08, 0x3d0) == 0) return;                   // 009F853C
    const Handle target = self.selector_2c.target_08;
    if (target == 0) return;
    if (host.command_target_resolve_00521ea0(self.director_04 + 0x58) == target) return;
    if (!host.is_kind_of_5c(host.u32(self.squadron_08, 0x3d0), 0x13)) {    // 009F8576
        if (host.director_vt50(self.director_04) == 1) {                   // 009F8584
            if (!host.is_kind_of_5c(target, 0x0f) && !host.is_kind_of_5c(target, 0x18))
                return;
        }
    }
    // 009F85AD..009F85C0: the flag byte is [director+3Dh] (the dword's upper
    // bytes are whatever the dt slot held).
    const Handle cmd = squadron_choose_attack_command_009f6d60(
        host, self, target, host.u8(self.director_04, 0x3d));
    const Handle current = host.u32(self.director_04, 0x54);
    // 009F6D80, attack_commands.cpp's attack_command_is_ordnance_class.
    if (attack_command_is_ordnance_class(current) && attack_command_is_ordnance_class(cmd))
        return;                                                            // 009F85E1
    if (current == kCmd8F88) {
        // 009F85EB..009F8615: clear, then moveto the squadron's own position.
        host.send_clear_commands_0071d880(self.director_04);
        const Handle position = host.world_position_refreshed_00427eb0(self.squadron_08);
        const Handle here = host.command_target_init_position_00468560(position, 0.0f);
        host.issue_command_0071ecf0(self.director_04, kCmdMoveTo, here);
    }
    // 009F861A..009F8631.
    const Handle aim = host.command_target_from_entity_00465080(target, 0.0f);
    host.send_override_command_0071d980(self.director_04, cmd, aim);
}

// ===========================================================================
// The AttackLists loader
// ===========================================================================

int format_string4_009f6f40(char* buffer, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    const int written = vsprintf_s(buffer, 4, format, args);  // 009F6F51 -> 00BF7289
    va_end(args);
    return written;
}

namespace {

struct AttackListRow {
    const char* name;         // the pushed string
    std::uint32_t offset;     // the vector at record+offset
};

// The image's order (009F8B79, 009F8CEF, 009F8E5F, ...).
constexpr AttackListRow kAttackLists[] = {
    {"DepthCharge", 0x04},    // 00CFA700, 009F8CD7
    {"Strike", 0x74},         // 00D06CA4, 009F8E47
    {"DiveBomb", 0x14},       // 00D06E1C, 009F8FB7
    {"LevelBomb", 0x24},      // 00D06EB0, 009F9127
    {"TorpedoBomb", 0x34},    // 00D21C24, 009F9297
    {"NonFighterGun", 0x54},  // 00D21C14, 009F9407
    {"FighterGun", 0x44},     // 00D21C08, 009F9577
    {"Kamikaze", 0x64},       // 00CFA714, 009F96E7
};

}  // namespace

void squadron_free_attack_targets_load_009f89f0(FreeAttackLoaderHost& host, Handle record)
{
    host.lua_state_construct_00b66bd0();                  // 009F8A1A
    host.lua_state_open_00b6a020(0x41);                   // 009F8A31
    // 009F8A46..009F8AA5 / 009F8ABA..009F8B19: each path goes through a
    // NativeString temporary, run with argument 0, then returned to the pool.
    host.lua_run_script_00b69d40("Scripts\\global\\luaMW_init.lua", 0);
    host.lua_run_script_00b69d40("Scripts\\datatables\\SquadronFreeAttackTargets.lua", 0);
    // 009F8B2D..009F8B5A: the globals into ESP+B0h, AttackLists looked up
    // straight into the ESP+5Ch local, the globals destroyed.
    const Handle globals = host.lua_globals_00b67980(host.local_globals_object());
    const Handle lists =
        host.lua_get_by_name_00b67800(globals, host.local_lists_object(), "AttackLists");
    host.lua_object_destruct_00b67700(globals);
    const Handle list = host.local_list_object();
    const Handle value = host.local_value_object();
    const Handle temp = host.local_temp_object();
    host.lua_object_construct_00b65f50(list);              // 009F8B63
    host.lua_object_construct_00b65f50(value);             // 009F8B74
    for (const AttackListRow& row : kAttackLists) {
        // 009F8B79..009F8BB2: lists[name] into the temp, assigned to the list.
        Handle tmp = host.lua_get_by_name_00b67800(lists, temp, row.name);
        host.lua_object_assign_00b67690(list, tmp);
        host.lua_object_destruct_00b67700(tmp);
        for (int i = 0; i < kAttackListKeyCount; ++i) {
            char key[4] = {};
            format_string4_009f6f40(key, "%d", i);        // 00CE3A34
            tmp = host.lua_get_by_name_00b67800(list, temp, key);
            const bool nil = host.lua_is_nil_00b65fb0(tmp);
            host.lua_object_destruct_00b67700(tmp);
            if (nil) continue;
            tmp = host.lua_get_by_name_00b67800(list, temp, key);
            host.lua_object_assign_00b67690(value, tmp);
            host.lua_object_destruct_00b67700(tmp);
            int entry = 0;
            if (host.lua_is_string_00b660a0(value)) {
                // 009F8C5A..009F8CC5: -1, the string copied unbounded into a
                // 64-byte local (ESP+70h), the first equal name of 97 wins.
                entry = -1;
                char name[64] = {};
#pragma warning(suppress : 4996)  // the image's copy is unbounded too
                std::strcpy(name, host.lua_get_string_00b662b0(value));
                for (int k = 0; k < kClassNameCount; ++k) {
                    if (std::strcmp(name, host.class_name_00e0cd80(k)) == 0) {
                        entry = k;
                        break;
                    }
                }
            } else {
                entry = host.lua_get_integer_00b66290(value);   // 009F8CC7
            }
            host.push_back_009f8960(record, row.offset, entry);
        }
    }
    // 009F96FF..009F9744: value, list, AttackLists destroyed, the state closed.
    host.lua_object_destruct_00b67700(value);
    host.lua_object_destruct_00b67700(list);
    host.lua_object_destruct_00b67700(lists);
    host.lua_state_close_00b669a0();
}

// ===========================================================================
// Converging-leg lock and manager plumbing
// ===========================================================================

void array16_grow_009f9b60(LegSingletonHost& host, Array16& array)
{
    // capacity = capacity * 2 + 2, stored before the allocation.
    array.capacity_08 = array.capacity_08 * 2 + 2;
    const Handle fresh = host.operator_new(array.capacity_08 * 0x10);
    for (std::uint32_t i = 0; i < array.count_04; ++i) {
        // The placement-new null test of the image (a null fresh block skips
        // only element 0's copy) is not reproduced; operator_new does not
        // return null.
        host.copy16(fresh + i * 0x10, array.data_00 + i * 0x10);
    }
    if (array.data_00 != 0) host.free_00bf65ac(array.data_00);
    array.data_00 = fresh;
}

void array16_push_back_009fa430(LegSingletonHost& host, Array16& array, Handle record)
{
    if (array.count_04 == array.capacity_08) array16_grow_009f9b60(host, array);  // inlined
    const Handle slot = array.data_00 + array.count_04 * 0x10;
    if (slot != 0) host.copy16(slot, record);
    ++array.count_04;
}

void free_owned_buffer_009f9ad0(LegSingletonHost& host, Handle& buffer)
{
    if (buffer != 0) host.free_00bf65ac(buffer);
}

LegLock& leg_lock_construct_009fa9f0(LegSingletonHost& host, LegLock& self)
{
    self.vtable_00 = kLegLockVtable;
    self.section_04 = host.critical_section_create();
    return self;
}

void leg_lock_reset_singleton_009f9a70(LegSingletonHost& host, LegLock& self)
{
    host.global_00f8a724() = 0;
    self.vtable_00 = kSingletonBaseVtable;
}

void leg_lock_destruct_009fb120(LegSingletonHost& host, LegLock& self)
{
    self.vtable_00 = kLegLockVtable;
    host.critical_section_destroy_owned(self.section_04);
    host.global_00f8a724() = 0;
    self.vtable_00 = kSingletonBaseVtable;
}

Handle leg_lock_get_009fb010(LegSingletonHost& host)
{
    if (host.global_00f8a724() != 0) return host.global_00f8a724();
    const Handle section = host.manager_section_10(host.singleton_manager_get());
    if (section != 0) {
        host.enter_critical_section(section);        // [00CE2218]
        host.section_depth_add_18(section, 1);
    }
    if (host.global_00f8a724() == 0) {
        const Handle block = host.operator_new(8);   // 00BF681B
        Handle lock = 0;
        if (block != 0) {
            leg_lock_construct_009fa9f0(host, host.lock_at(block));
            lock = block;
        }
        host.global_00f8a724() = lock;
        host.singleton_register(host.singleton_manager_get(), host.global_00f8a724());
    }
    if (section != 0) {
        host.section_depth_add_18(section, -1);
        host.leave_critical_section(section);        // [00CE2210]
    }
    return host.global_00f8a724();
}

void leg_manager_construct_009faa40(LegManager& self) noexcept
{
    self.vtable_00 = kLegManagerVtable;
    self.records_04 = Array16{};
}

void leg_manager_destruct_009faa60(LegSingletonHost& host, LegManager& self)
{
    if (self.records_04.data_00 != 0) host.free_00bf65ac(self.records_04.data_00);
    host.global_00f8a720() = 0;
    self.vtable_00 = kSingletonBaseVtable;
}

// ===========================================================================
// Unit helpers and the world-edge overrun
// ===========================================================================

float unit_blast_accumulator_get_009f9be0(const float* unit_460, int index) noexcept
{
    return unit_460[index];  // FLD [ECX + EAX*4 + 460h]
}

float unit_blast_accumulator_sum_009f9bf0(float f464, float f474, float f478) noexcept
{
    // FLD [+474h] / FADD [+464h] / FADD [+478h] / FSTP float.
    return (f474 + f464) + f478;
}

float* random_unit_circle_direction_009f9c80(RandomHost& host, float* out)
{
    // 009F9C9D: 00BD2F10 with ECX = 1, (0.0f, 2pi as float 00CE3D9C).
    const float a = host.random_uniform_00bd2f10(1, 0.0f, 6.2831854820251465f);
    // FCOS then FSIN of the stored float, each FSTP float; out = (sin, cos).
    const float c = static_cast<float>(std::cos(static_cast<double>(a)));
    const float s = static_cast<float>(std::sin(static_cast<double>(a)));
    out[0] = s;
    out[1] = c;
    return out;
}

float ggame_world_edge_overrun_009fa510(const WorldEdges& edges, const float* p,
                                        float margin) noexcept
{
    const float x = p[0];
    float worst = 0.0f;
    // 009FA523..009FA553: lo = x_min + margin (FADDP, not stored); x < lo
    // (FCOMI lo, x / JBE) gives d = lo - x, FSTP float, kept unless 0 > d.
    const float x_lo = edges.x_min_711c + margin;
    if (x_lo > x) {
        const float d = x_lo - x;
        if (!(0.0f > d)) worst = d;
    }
    // 009FA55C..009FA58C: hi = x_max - margin; x > hi gives d = x - hi; the
    // stored worst survives only when worst > d (FCOMI / JA).
    const float x_hi = edges.x_max_7128 - margin;
    if (x > x_hi) {
        const float d = x - x_hi;
        if (!(worst > d)) worst = d;
    }
    const float z = p[2];
    // 009FA59C..009FA5D2: the z-max edge, same rule.
    const float z_hi = edges.z_max_7124 - margin;
    if (z > z_hi) {
        const float d = z - z_hi;
        if (!(worst > d)) worst = d;
    }
    // 009FA5D4..009FA606: lo = margin + z_min (FADDP ST3).
    const float z_lo = margin + edges.z_min_7130;
    if (z_lo > z) {
        const float d = z_lo - z;
        if (!(worst > d)) worst = d;
    }
    return worst;
}

int unit_kind18_count_009fe060(SquadronAutoTargetHost& host, Handle unit)
{
    if (host.is_kind_of_5c(unit, 0x18)) return static_cast<int>(host.u32(unit, 0x3cc));
    return 1;
}

// ===========================================================================
// The plane gun object
// ===========================================================================

BotTaskGun& bot_task_gun_construct_009faad0(GunHost& host, BotTaskGun& self, Handle owner)
{
    self.owner_00 = owner;                        // 009FAAE2
    self.wander_04.accel = 0.5f;                  // 00CE3800
    self.wander_04.scale = 0.019999999552965164f; // 00CE9BAC
    self.wander_04.radius = 0.0f;
    host.reseed_aim_wander_009fa620(self.wander_04);  // 009FAAFD, ECX = this+4
    self.f28 = -1.0f;                             // 00D7A260
    self.b48 = 0;
    self.b49 = 0;
    self.b4b = 0;
    self.u74 = 0;
    self.f2c = 0.0f;
    self.f30 = 0.0f;
    self.f34 = 0.0f;
    self.f38 = 0.0f;
    self.f3c = 0.0f;
    self.cone_40 = 0.0f;
    self.f44 = 999999.0f;                         // 00CF87D0
    self.f4c = 0.0f;
    self.f50 = 0.0f;
    return self;
}

void bot_task_gun_init_aim_009fab60(GunHost& host, BotTaskGun& self, float accel, float scale,
                                    float reach, float f3c)
{
    self.wander_04.accel = accel;                                  // 009FAB7A
    // 009FAB6A FLD double 1e-5f (00D7A330) / FCOMPI: below it takes 00D7A310.
    const float s = 9.999999747378752e-06 > static_cast<double>(scale)
                        ? 9.999999747378752e-06f : scale;
    const float r = reach / s;                                     // 009FAB9A, FSTP float
    self.wander_04.scale = s;                                      // 009FAB9E
    // 009FABA7 FLD double 0.9f (00D7A390) / FCOMPI ST0 = r: above takes 00CE3860.
    self.wander_04.radius = static_cast<double>(r) > 0.8999999761581421 ? 0.8999999761581421f : r;
    host.reseed_aim_wander_009fa620(self.wander_04);              // 009FABCC
    self.f3c = f3c;                                                // 009FABD7
}

void bot_set_direction_from_heading_pitch_009fabe0(GunHost& host, BotTaskGun& self,
                                                   float heading, float pitch)
{
    // 009FABE8 FSUBR double pi/2 (00CE3830, a float value), FSTP float.
    float a = 1.5707963705062866f - heading;
    // 009FABF9 FLDZ / FCOMPI / JBE: 0 > a adds 2pi (00CE3828), FSTP float.
    if (0.0f > a) a = a + 6.2831854820251465f;
    const float c = static_cast<float>(std::cos(static_cast<double>(a)));   // 009FAC11
    const float s = static_cast<float>(std::sin(static_cast<double>(a)));   // 009FAC1B
    self.direction_68[0] = c;
    self.direction_68[2] = s;
    self.direction_68[1] = host.tangent_00412e20(pitch);                    // 009FAC41
    const float length = host.vector_length_00419440(self.direction_68);    // 009FAC4B
    // 009FAC5A FCOMI length, 0 / JBE: positive gives FLD1 / FDIVRP, else 0.
    const float inverse = length > 0.0f ? 1.0f / length : 0.0f;
    // 009FAC75..009FACA7: each component times the inverse, FSTP float.
    self.direction_68[0] = self.direction_68[0] * inverse;
    self.direction_68[1] = self.direction_68[1] * inverse;
    self.direction_68[2] = inverse * self.direction_68[2];
}

// ===========================================================================
// The approach target ref
// ===========================================================================

std::array<float, 3> approach_ref_target_velocity_009fa2e0(ApproachRefHost& host,
                                                           const NativeApproachTargetRef& self)
{
    if (self.target_18 != nullptr) {
        if (self.vehicle_14 != nullptr) return host.entity_velocity_34(self.vehicle_14);
        return host.entity_velocity_34(self.target_18);
    }
    return host.static_zero_vector_00f87574();
}

void approach_ref_reset_aim_offset_009fa350(ApproachRefHost& host, NativeApproachTargetRef& self)
{
    self.body_offset_28 = host.static_zero_vector_00f87574();
    self.dirty_41 = 1;
}

void approach_ref_set_aim_error_009fa380(NativeApproachTargetRef& self,
                                         const std::array<float, 3>& error) noexcept
{
    self.bias_34 = error;
    self.dirty_41 = 1;
}

void approach_ref_save_009facc0(ApproachRefHost& host, const NativeApproachTargetRef& self,
                                Handle writer_ref, Handle writer)
{
    // writer->vtable[0Ch](0, key, type, value): type 5 passes the field
    // address, type 2 the float bits, type 4 the pointer.
    host.archive_write_0c(writer, 0, "precision", 5,
                          reinterpret_cast<std::uintptr_t>(self.spread_48.data()));
    host.archive_write_0c(writer, 0, "bullpos", 5,
                          reinterpret_cast<std::uintptr_t>(self.centre_54.data()));
    host.archive_write_0c(writer, 0, "error", 5,
                          reinterpret_cast<std::uintptr_t>(self.bias_34.data()));
    std::uint32_t bits = 0;
    std::memcpy(&bits, &self.projtime_44, sizeof bits);
    host.archive_write_0c(writer, 0, "projtime", 2, bits);
    host.archive_write_0c(writer, 0, "vehicle", 4,
                          reinterpret_cast<std::uintptr_t>(self.vehicle_14));
    host.archive_writer_ref_destroy_004107b0(writer_ref);   // 009FAD84, the by-value ref
}

void approach_ref_on_target_destroyed_009faf90(ApproachRefHost& host,
                                               NativeApproachTargetRef& self, Handle message)
{
    NativeObserverOwnerStorage* const subject = host.message_subject_04(message);
    NativeObserverOwnerStorage* const target = self.target_18;
    if (subject != target) return;
    if (!host.pose_valid_c8(target)) host.refresh_pose_00414db0(target);
    // 009FAFBB..009FAFCC: 004142E0(ECX = ref+1Ch, out, target+CCh): the
    // stored point itself goes through the target's matrix once more.
    self.world_point_1c = host.transform_affine_point_004142e0(self.world_point_1c, target);
    self.has_target_40 = 0;
    self.target_18 = nullptr;
    self.vehicle_14 = nullptr;
    self.dirty_41 = 0;
}

void approach_ref_set_vehicle_009fb0f0(ApproachRefHost& host, NativeApproachTargetRef& self,
                                       NativeObserverOwnerStorage* vehicle)
{
    if (self.vehicle_14 == vehicle) return;
    if (self.vehicle_14 != nullptr)
        host.observers().unregister_pair_006952a0(*self.vehicle_14, self.observer_00);
    self.vehicle_14 = vehicle;
    if (vehicle != nullptr)
        register_observer_pair_00694a60(*vehicle, self.observer_00, host.observers());
}

NativeApproachTargetRef& approach_ref_assign_vehicle_009fb6c0(
    ApproachRefHost& host, NativeApproachTargetRef& self, NativeObserverOwnerStorage* vehicle)
{
    approach_ref_set_vehicle_009fb0f0(host, self, vehicle);  // the same body, EAX = this
    return self;
}

void approach_ref_bind_target_009fb190(ApproachRefHost& host, NativeApproachTargetRef& self,
                                       NativeObserverOwnerStorage* target)
{
    if (target == nullptr) {
        self.has_target_40 = 0;
        self.target_18 = nullptr;
        self.vehicle_14 = nullptr;
        return;
    }
    self.has_target_40 = 1;                                         // 009FB1B2
    self.repick_timer_60 = host.random_uniform_00bd2f10(1, 0.0f, 2.0f);  // 009FB1B8, 00CE3958
    self.target_18 = target;
    // 009FB1CC..009FB1D6: the target itself when IsKindOf(5), else null; no
    // observer is registered for +14h here.
    self.vehicle_14 = host.is_kind_of_5c(target, 5) ? target : nullptr;
    host.pick_body_frame_aim_offset_009fa260(self);                 // 009FB1D9
    register_observer_pair_00694a60(*self.target_18, self.observer_00, host.observers());
}

void approach_ref_set_target_009fb5e0(ApproachRefHost& host, NativeApproachTargetRef& self,
                                      NativeObserverOwnerStorage* target)
{
    if (target == self.target_18) return;
    if (self.target_18 != nullptr)
        host.observers().unregister_pair_006952a0(*self.target_18, self.observer_00);
    approach_ref_bind_target_009fb190(host, self, target);
}

void approach_ref_on_vehicle_destroyed_009fb640(ApproachRefHost& host,
                                                NativeApproachTargetRef& self, Handle message)
{
    NativeObserverOwnerStorage* const subject = host.message_subject_04(message);
    if (subject != self.vehicle_14) return;
    self.vehicle_14 = nullptr;
    // ECX = the subject, EDX = this.
    host.observers().unregister_pair_006952a0(*subject, self.observer_00);
}

void approach_ref_on_vehicle_destroyed_009fb670(ApproachRefHost& host,
                                                NativeApproachTargetRef& self, Handle message)
{
    approach_ref_on_vehicle_destroyed_009fb640(host, self, message);  // byte-identical body
}

NativeApproachTargetRef& approach_ref_construct_from_stream_009fb3e0(
    ApproachRefHost& host, NativeApproachTargetRef& self, Handle reader_ref, Handle reader)
{
    self.observer_00.edges_04.data_00 = nullptr;           // 009FB404..009FB40A
    self.observer_00.edges_04.count_04 = 0;
    self.observer_00.edges_04.capacity_08 = 0;
    self.observer_byte_10 = 0;
    self.observer_00.native_vtable_00 = kApproachTargetRefVtable;  // 009FB41A
    self.bias_34 = host.static_zero_vector_00f87574();     // 009FB432..009FB458
    self.repick_timer_60 = host.random_uniform_00bd2f10(1, 0.0f, 2.0f);  // 009FB45D
    self.body_offset_28 = host.static_zero_vector_00f87574();  // 009FB471..009FB48E
    self.world_point_1c = self.body_offset_28;             // 009FB476..009FB4AB
    self.section_chance_64 = -1.0f;                        // 009FB4B2, 00D7A260
    self.has_target_40 = 0;
    self.weight_68 = 1.0f;                                 // 00D7A24C
    self.weight_6c = 1.0f;
    self.weight_70 = 1.0f;
    // reader->vtable[10h](0, key, type, destination).
    host.archive_read_10(reader, 0, "precision", 5, self.spread_48.data());
    host.archive_read_10(reader, 0, "bullpos", 5, self.centre_54.data());
    host.archive_read_10(reader, 0, "error", 5, self.bias_34.data());
    host.archive_read_10(reader, 0, "projtime", 2, &self.projtime_44);
    // 009FB52A..009FB552: the ref's own vtable[10h] with the {4, &slot} pair.
    NativeObserverOwnerStorage* vehicle = nullptr;
    host.archive_ref_read_pointer_10(reader_ref, 0, "vehicle", &vehicle);
    self.vehicle_14 = vehicle;
    if (vehicle != nullptr) {
        approach_ref_bind_target_009fb190(host, self, vehicle);    // 009FB562
        host.update_009fada0(self, 0.0f);                          // 009FB56F
    }
    self.section_chance_64 = -1.0f;                        // 009FB589
    self.weight_68 = 1.0f;
    self.weight_6c = 1.0f;
    self.weight_70 = 1.0f;
    self.repick_timer_60 = host.random_uniform_00bd2f10(1, 0.0f, 2.0f);  // 009FB5AF
    host.archive_reader_ref_destroy_00410630(reader_ref);  // 009FB5C3
    return self;
}

ObservedPointerHolder& opportunistic_rocket_set_rocket_ref_009fcf90(
    NativeObserverLifetime& observers, ObservedPointerHolder& self,
    NativeObserverOwnerStorage* rocket)
{
    if (self.subject_14 == rocket) return self;
    if (self.subject_14 != nullptr)
        observers.unregister_pair_006952a0(*self.subject_14, self.observer_00);
    self.subject_14 = rocket;
    if (rocket != nullptr) register_observer_pair_00694a60(*rocket, self.observer_00, observers);
    return self;
}

void land_state_observer_vector_push_back_009fcfd0(
    NativeLandStateObserverVector& vector, const NativeLandStateObserverElement24& source,
    NativeObserverLifetime& lifetime)
{
    if (vector.count_04 == vector.capacity_08) {
        // 009FCFF8..009FD005: twice the capacity, at least 1 (CMP 1 / JG).
        std::int32_t capacity = vector.capacity_08 * 2;
        if (!(capacity > 1)) capacity = 1;
        reserve_native_land_state_elements_007b4400(vector, capacity, lifetime);
    }
    NativeLandStateObserverElement24* const slot = vector.data_00 + vector.count_04;
    if (slot != nullptr) {
        copy_native_land_state_element_007b3fc0(*slot, source, lifetime);  // 009FD02C
        slot->callback_00.native_vtable_00 = 0x00cf5c94u;                   // 009FD031
    }
    vector.count_04 = vector.count_04 + 1;
}

}  // namespace cc13_seg68
}  // namespace bsp
