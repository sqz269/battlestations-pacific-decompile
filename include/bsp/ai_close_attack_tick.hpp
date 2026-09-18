#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/ai_command_object.hpp"
#include "bsp/ai_tuning_globals.hpp"

// 00A13B60, the CLOSEATTACK and DEFENDPOSITION tick: the routine that gives a
// group's members an actual enemy. Reconstructed from the read-only analysis of
// packet cc8_ai_close_attack_tick. docs/AI_CLOSE_ATTACK_TICK.md carries the
// address evidence. Every descriptive name here is a hypothesis, not a
// recovered symbol.
//
// Coverage. Complete as rules: the collect radius, the candidate admission
// test, the four-factor score and its tuning fields, the per-member gate, and
// the order class and descriptor the winner receives. Partial, by address
// range: 00A14A78-00A14D4C, the no-candidate fallback, whose offset point is
// assembled from six stack slots that were not traced to their producers; and
// 00A13BFF-00A14380, the collection walk that fills the candidate list, read
// for its bounds and its filter but not instruction by instruction.

namespace bsp {

// ---------------------------------------------------------------------------
// The three scene commands an AI command can issue
// ---------------------------------------------------------------------------

// docs/AI_COMMAND_TICK.md said no AI command class issues any scene command but
// `moveto`, and that the descriptor carries a point and never an object. Both
// are wrong: 00A13B60 issues one of these two at 00A14A6E with an OBJECT
// descriptor, and 00A14DD0 issues `clearorders` at 00A14EA7.
inline constexpr std::uint32_t kAiSceneCommandAttackMove = 0x00E08F78u;  // ordinal 17
inline constexpr std::uint32_t kAiSceneCommandSetTarget = 0x00E08EF8u;   // ordinal 1
inline constexpr std::uint32_t kAiSceneCommandClearOrders = 0x00E08F08u; // ordinal 3

// 00A149EF asks the member vtable[+5Ch](6) one last time: a ship base gets
// `attackmove`, anything else `settarget`.
std::uint32_t ai_close_attack_order_class(bool member_is_ship_base) noexcept;

// ---------------------------------------------------------------------------
// Which members the tick serves
// ---------------------------------------------------------------------------

// 00A143ED-00A14444. A plane squadron is served unless its carrier link passes
// the 007EDA90 shape inline at 00A143F7-00A14413; anything else is served only
// when it is a ship base whose +538h object answers false to its vtable[+2Ch].
bool ai_close_attack_member_served(bool is_plane_squadron, bool squadron_carrier_excluded,
                                   bool is_ship_base, bool controller_busy) noexcept;

// ---------------------------------------------------------------------------
// The candidate set
// ---------------------------------------------------------------------------

// 00A13B8A-00A13BC3. The collect radius is tuning +1F4h CloseAttack_CollectDist
// times the caller's radius argument, which is 1.5f for CLOSEATTACK (00CE380C
// at 00A154D3) and 1.0f for DEFENDPOSITION (FLD1 at 00A1554E). The tick keeps
// the square.
float ai_close_attack_collect_radius(float collect_dist, float radius_argument) noexcept;

// 00A146C9-00A146DB. A candidate whose target weight is not above zero is
// dropped unless it belongs to the target group, which is the only way a
// zero-weight group member still gets attacked.
bool ai_close_attack_candidate_admitted(float target_weight, bool in_target_group) noexcept;

// The four factors, in the order 00A14942-00A1499C multiplies them.
struct AiCloseAttackScoreInputs {
    float target_weight{0.0f};     // 00A0F810, which wraps 00A08460
    float distance{0.0f};          // leader-to-candidate, planar
    float near_dist{0.0f};         // tuning +1F8h CloseAttack_NearDist, 3000
    float far_dist{0.0f};          // tuning +1FCh CloseAttack_FarDist, 6000
    bool in_target_group{false};   // 00A2C720 on the command's +1Ch group
    bool is_existing_target{false};// the member's own current target
    float target_group_member_mul{1.0f};  // tuning +204h, 10
    float existing_target_mul{1.0f};      // tuning +200h, 1.8
};

// 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x), RET 14h, with
// x0 = NearDist, y0 = 1.0f, x1 = FarDist, y1 = 0.0f and x the distance. So the
// factor is 1.0 at or inside NearDist and falls linearly to 0.0 at FarDist.
float ai_close_attack_range_factor(float distance, float near_dist, float far_dist) noexcept;

float ai_close_attack_score(const AiCloseAttackScoreInputs& in) noexcept;

// 00A149A0-00A149B8. Strictly greater wins, so the first candidate of an equal
// pair keeps the slot. The seed is 00CE4ADC.
bool ai_close_attack_candidate_wins(float score, float best) noexcept;

// ---------------------------------------------------------------------------
// The sequence
// ---------------------------------------------------------------------------

struct AiCloseAttackTickHost {
    virtual ~AiCloseAttackTickHost() = default;

    virtual std::size_t close_member_count(void* group) = 0;
    virtual void* close_member_at(void* group, std::size_t index) = 0;
    virtual bool close_member_is_ship_base(void* member) = 0;
    virtual bool close_member_is_plane_squadron(void* member) = 0;
    virtual bool close_member_squadron_excluded(void* member) = 0;
    virtual bool close_member_controller_busy(void* member) = 0;   // +538h vtable[+2Ch]
    virtual bool close_member_position(void* member, float out[3]) = 0;

    // The collection walk at 00A13BFF: every entity of the world list the tick
    // scans, filtered to the enemy side and the collect radius by the caller.
    virtual std::size_t close_candidate_count() = 0;
    virtual void* close_candidate_at(std::size_t index) = 0;
    virtual bool close_candidate_position(void* candidate, float out[3]) = 0;
    virtual int close_candidate_team(void* candidate) = 0;
    virtual bool close_candidate_alive(void* candidate) = 0;

    virtual float close_target_weight(void* member, void* candidate) = 0;   // 00A0F810
    virtual bool close_in_target_group(void* target_group, void* candidate) = 0;  // 00A2C720
    virtual void* close_member_current_target(void* member) = 0;  // vt+114h then 0071EB60

    // 0077D600 with the class above and a kind-1 descriptor naming the target.
    virtual bool close_issue_order(void* member, std::uint32_t command_class,
                                   void* target) = 0;
    // 00A02020's moveto, the no-candidate fallback at 00A14D48.
    virtual bool close_issue_moveto(void* member, const float point[3]) = 0;

    virtual float close_tuning_field(std::uint32_t offset) = 0;
    virtual int close_own_team(void* group) = 0;
};

struct AiCloseAttackTickResult {
    std::uint32_t members_served{0};
    std::uint32_t attack_move_orders{0};
    std::uint32_t set_target_orders{0};
    std::uint32_t fallback_movetos{0};
    std::uint32_t candidates_scored{0};
};

// 00A13B60, __thiscall(command)(float radius, const float* point,
// AiGroup* targetGroup, int flag). `point` is the centre the candidates are
// collected around and `flag` was not traced past its store.
AiCloseAttackTickResult ai_close_attack_tick_00a13b60(AiCloseAttackTickHost& host,
                                                      const AiCommandObject& command,
                                                      float radius_argument,
                                                      const float centre[3]);

// ---------------------------------------------------------------------------
// 00A0F810 BSP_AiCommand_CandidateTargetWeight, body 00A0F810..00A0F961
// ---------------------------------------------------------------------------
// The weight 00A13B60's candidate loop scores with. It wraps 00A08460
// BSP_Ai_TargetWeight (modelled in bsp/ai_target_weights.hpp) in four
// multipliers, and its stack slots SHIFT across the argument pushes of the two
// class tests, so the offsets below are normalised against a single frame with
// tools/stack_frame_walk.py --indirect-pops 4. Read naively, 00A0F8EE looks
// like it overwrites the objective multiplier 00A0F8C6 just wrote; it does not,
// because it runs four bytes deeper and lands on a different slot.
//
//   slot A (00A0F848 FSTP [ESP+10h] at depth 104)  the raw 00A08460 weight,
//          zeroed at 00A0F864 when the attacker's +1Ch is set and its
//          vtable[+18h](1Ch) answers true; consumed at 00A0F897 and then
//          REUSED as the class multiplier, seeded 1.0f at 00A0F8EE (depth 108).
//   slot B (00A0F889 FSTP [ESP+14h] at depth 104)  attacker+18h * target+18h,
//          reloaded at 00A0F88D and then REUSED as the objective multiplier,
//          seeded 1.0f at 00A0F891 and raised to 10.0f at 00A0F8C6.
//   slot C (00A0F89E FSTP [ESP+18h] at depth 104)  slot B's product * slot A's
//          weight * target+14h.
//   slot D (00A0F8E6 FSTP [ESP+1Ch])               2.0 - 00923BE0(target).
//
// The epilogue 00A0F943..00A0F956 multiplies B * C * D * A, with the pops
// between them accounted for.
struct AiCandidateTargetWeightInputs {
    // 00A08460's answer for (attacker, attacker class, target, target neutral).
    float base_weight{0.0f};
    // 00A0F84C [attacker record +1Ch] and 00A0F859 PUSH 1Ch through the
    // attacker's vtable[+18h]: both true zero the weight.
    bool attacker_record_flag_1c{false};
    bool attacker_is_command_building{false};
    float attacker_factor{1.0f};   // attacker record +18h, 00A0F86A
    float target_factor{1.0f};     // target record +18h, 00A0F872
    float target_scale{1.0f};      // target record +14h, 00A0F89B
    // 00A0F8B5 008DDF90 against game+21A4h when the local player's party equals
    // the attacker's +54h (00A0F87E) and game+21B4h otherwise: objective sets 0
    // and 4, not the brain's slot.
    bool target_is_objective{false};
    // 00A0F8F4 / 00A0F903 / 00A0F912: IsKindOf 1Bh, 45h, 46h, the 009FE0B0 trio.
    bool target_matches_009fe0b0{false};
    // 00A0F92F PUSH 1Ch: only a NON command building takes the 0.01f arm.
    bool target_is_command_building{false};
    // 00A0F8CE 00923BE0(target), subtracted from the double 2.0 at 00D7A308.
    float target_term{0.0f};
};

// 00D7A24C, the seed of both reused slots.
inline constexpr float kAiCandidateWeightDefaultMultiplier = 1.0f;
// 00CE38B8, the objective multiplier: an objective target is worth ten times.
inline constexpr float kAiCandidateWeightObjectiveMultiplier = 10.0f;
// 00D7A2F0, the 009FE0B0 trio's multiplier.
inline constexpr float kAiCandidateWeightFortMultiplier = 0.1f;
// 00D228A0, the weight a trio target that is NOT a command building gets.
inline constexpr float kAiCandidateWeightNonCommandWeight = 0.01f;
// 00D7A308, the double the target term is subtracted from.
inline constexpr float kAiCandidateWeightTermBase = 2.0f;

float ai_candidate_target_weight_00a0f810(
    const AiCandidateTargetWeightInputs& in) noexcept;


}  // namespace bsp
