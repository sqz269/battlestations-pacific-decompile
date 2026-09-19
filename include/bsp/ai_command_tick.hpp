#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/ai_command_object.hpp"
#include "bsp/ai_planners.hpp"
#include "bsp/ai_tuning_globals.hpp"

// How an AI command reaches its member units: the `vt+0Ch` tick that
// 00A2C790 runs on the group's 2 to 4 second schedule, and the one routine,
// 00A02020, by which any of it becomes a scene command. Reconstructed from the
// read-only analysis of packet cc8_ai_command_tick. docs/AI_COMMAND_TICK.md
// carries the address evidence. Every descriptive name here is a hypothesis,
// not a recovered symbol.
//
// Coverage. Complete as rules: the order bridge 00A02020 (body
// 00A02020-00A02175), the leader-position helper 00A10C20, the follower pass
// 00A10DC0 (body 00A10DC0-00A10EB6), and the tick bodies of MOVETO
// (00A124E0-00A1253F), CAUTIOUSMOVE (00A152B0-00A152D1), MOVETOATTACK
// (00A12A90-00A12C53), CLOSEATTACK (00A15490-00A154FB) and DEFENDPOSITION
// (00A15500-00A1556A). Partial, by address range: the engagement helper
// 00A13B60-00A14D9F, the cautious helper 00A14DD0-00A152A7, the formation pass
// 00A11070-00A113C1 below its two named calls, 00A11690-00A11AEC,
// 00A11AF0-00A11B73, 00A11B80-00A11F63, and the REGROUPINGMOVE
// (00A126C0-00A12A4C), PATROLTO (00A15570-00A156D3), RETREAT
// (00A156E0-00A158FF), SELLING (00A11FF0-00A1242D) and CAUTIOUSATTACK
// (00A152E0 onward) bodies, which were read only to their first dispatch.

namespace bsp {

// ---------------------------------------------------------------------------
// The scene command an AI command issues
// ---------------------------------------------------------------------------

// 00A02020 pushes this one class descriptor at 00A0214B. It is `moveto`,
// ordinal 15, category 3 of docs/SCENE_COMMAND_TYPES.md. No AI command issues
// any other scene command.
inline constexpr std::uint32_t kAiSceneCommandMoveTo = 0x00E08F68u;

// The flags argument 00A02020 hands 0077D600 at 00A02136.
inline constexpr int kAiSceneCommandFlags = 1;

// 00A02020's distance gate at 00A0206E.
//
// CORRECTION, packet cc8_ship_command. This was recorded as 0.0f, read as the
// DWORD at the address, with the conclusion that "the comparison `d2 >= value`
// always holds and the order is issued whenever the class gate passes". The
// loading instruction is `00A02098 FLD double ptr [0x00D21530]`, eight bytes,
// and 6400.0 is 40 B9 00 00 00 00 00 00 big-endian, i.e. `00 00 00 00` in the
// low dword and `00 00 B9 40` in the high one. The old reading took the low
// half of a double. At the instruction's own width the value is 6400.0
// (`tools/pe_const_read.py d:00d21530`), so the gate is a real 80 m radius:
// 00A02020 issues nothing to a member already within 80 m of the point it
// would be ordered to. The Ghidra overlapping symbol the old note mentions is
// what makes the dword reading look plausible; the width of the load settles
// it. docs/SHIP_COMMAND_LIFETIME.md.
inline constexpr std::uint32_t kAiOrderIssueDistanceSquaredAddress = 0x00D21530u;
inline constexpr float kAiOrderIssueDistanceSquared = 6400.0f;  // 80 m squared

// The tuning fields the ticks read live in bsp/ai_tuning_globals.hpp, which
// packet cc8_ai_command_inputs taught the loader to fill:
// kAiTuningCautionMoveDist (+1F0h) and kAiTuningCloseAttackCollectDist (+1F4h).

// ---------------------------------------------------------------------------
// 00A02020, the order bridge
// ---------------------------------------------------------------------------

// The class gate at 00A02026-00A02040: a plane squadron passes only when
// 007EDA90 rejects it, and anything else passes only when it is a ship base.
// It is the same shape as 009FE080 but on a different pair of ids.
bool ai_order_bridge_accepts_00a02020(bool is_plane_squadron, bool squadron_excluded,
                                      bool is_ship_base) noexcept;

// Which point the descriptor carries. A ship is routed through
// 00417B10 BSP_AvoidZoneGroup_OffsetPointSequential at 00A020E8, with its y
// forced to zero; anything else goes straight to the requested point.
enum class AiOrderBridgePoint { Direct, AvoidZoneOffset };
AiOrderBridgePoint ai_order_bridge_point_00a02020(bool is_ship_base) noexcept;

// The descriptor 00A02020 builds: kind 0 (a position), position_valid 1,
// object_id 0, object null, trailing 0, and the point above.
struct AiOrderBridgeResult {
    bool issued{false};
    AiOrderBridgePoint point_source{AiOrderBridgePoint::Direct};
    float position[3]{0.0f, 0.0f, 0.0f};
};

// `member_position` is the entity's +FCh/+100h/+104h pose, `target` the point
// the caller asked for. `avoid_zone_point` is what 00417B10 returned, used only
// on the ship branch, where its two floats land in x and z and y becomes 0.
AiOrderBridgeResult ai_order_bridge_00a02020(bool is_plane_squadron, bool squadron_excluded,
                                             bool is_ship_base, const float member_position[3],
                                             const float target[3],
                                             const float avoid_zone_point[2]) noexcept;

// ---------------------------------------------------------------------------
// 00A10C20 and 00A10DC0
// ---------------------------------------------------------------------------

// 00A10C20, __thiscall(group), RET 0: the group's first member's +FCh pose, or
// the zero vector at 00F87574 when the group is empty. True means the caller
// must substitute the origin.
bool ai_group_leader_point_is_origin_00a10c20(std::uint32_t population) noexcept;

// 00A10DC0's per-follower decision at 00A10E3E-00A10E98. The pass runs only
// when the population is above one and skips the leader; a ship follower is
// told to join the leader's formation through 0077C8D0, and a plane squadron
// that 009FFEB0 does not exclude is issued a moveto at the leader's point.
// Anything else is left alone.
enum class AiFollowerAction { None, JoinLeaderFormation, MoveToLeaderPoint };
AiFollowerAction ai_command_follower_action_00a10dc0(bool is_ship_base, bool is_plane_squadron,
                                                     bool squadron_excluded_009ffeb0) noexcept;
bool ai_command_follower_pass_runs_00a10dc0(std::uint32_t population) noexcept;

// ---------------------------------------------------------------------------
// 009FFD80, the member ordering key that defines the leader
// ---------------------------------------------------------------------------

// 009FDF30, __fastcall(int class_id) -> float. A jump table at 009FDFF0 over
// class ids 6 through 1Ch, each arm returning one float of the tuning block;
// every other id, and 14h, 18h, 19h and 1Ah, take the FLD1 default at 009FDFED.
// Returns the record offset, or 0xFFFFFFFF when the default applies.
std::uint32_t ai_entity_class_weight_offset_009fdf30(int class_id) noexcept;

// 009FFD80 BSP_Entity_AiLeaderWeight, __thiscall(entity), body
// 009FFD80-009FFDFB. The class weight above times a multiplier: 100.0f
// (00CE3D08) when the entity IsKindOf(6), 0.01f (00D7A238) when it IsKindOf
// 1Bh, 45h or 46h, and 1.0f (00D7A24C) otherwise.
inline constexpr float kAiLeaderWeightShipMultiplier = 100.0f;
inline constexpr float kAiLeaderWeightGroupClassMultiplier = 0.01f;
inline constexpr float kAiLeaderWeightDefaultMultiplier = 1.0f;
float ai_entity_leader_weight_009ffd80(float class_weight, bool is_ship_base,
                                       bool is_one_of_three_group_classes) noexcept;

// 00A2D8E0's insert at 00A2D941-00A2D94E: the new member's weight is compared
// against each existing member's with FCOMPI and the walk stops on JA, so the
// list is ordered by DESCENDING weight and an equal weight inserts after. The
// first member is therefore the highest-weighted one, and that is the leader
// every tick and every merge test reads. True means the candidate goes before.
bool ai_group_member_sorts_before_00a2d8e0(float candidate_weight,
                                           float existing_weight) noexcept;

// ---------------------------------------------------------------------------
// The class ticks
// ---------------------------------------------------------------------------

// What a tick did, for the census.
struct AiCommandTickResult {
    std::uint32_t orders_issued{0};      // 00A02020 reached 0077D600
    std::uint32_t formation_requests{0}; // 0077C8D0
    std::uint32_t followers_walked{0};
    bool leader_ordered{false};
    bool promoted{false};                // the command replaced itself
    AiCommandType promoted_to{AiCommandType::CloseAttack};
};

// MOVETOATTACK's transition at 00A12B55-00A12BA8: the tick reads tuning +1F4h
// `CloseAttack_CollectDist` (shipped 5000) and, while the horizontal distance
// between the two groups' leaders is not above it, replaces itself with a
// CLOSEATTACK built by new(20h) + 00A10710 and installed through 00A2BD00.
// True means keep closing, false means promote.
bool ai_moveto_attack_keeps_closing(bool leader_is_groupable_combatant,
                                    float leader_distance,
                                    float close_attack_collect_dist) noexcept;

// One method per native call a tick makes.
struct AiCommandTickHost {
    virtual ~AiCommandTickHost() = default;

    virtual std::uint32_t tick_group_population(void* group) = 0;          // +5644h
    virtual std::size_t tick_member_count(void* group) = 0;
    virtual void* tick_member_at(void* group, std::size_t index) = 0;
    virtual bool tick_leader_point(void* group, float out[3]) = 0;         // 00A10C20
    virtual bool tick_member_position(void* member, float out[3]) = 0;     // +FCh, 00414DB0

    virtual bool tick_member_is_ship_base(void* member) = 0;               // vt+5Ch(6)
    virtual bool tick_member_is_plane_squadron(void* member) = 0;          // vt+5Ch(18h)
    virtual bool tick_squadron_excluded_007eda90(void* member) = 0;
    virtual bool tick_squadron_excluded_009ffeb0(void* member) = 0;
    virtual bool tick_member_is_groupable_combatant(void* member) = 0;     // 009FE080

    // 00A02020's tail: 0077D600 with kAiSceneCommandMoveTo and the descriptor.
    virtual bool tick_issue_moveto(void* member, const float position[3]) = 0;
    // 0077C8D0 BSP_Entity_RequestJoinFormation(follower)(leader).
    virtual bool tick_request_join_formation(void* follower, void* leader) = 0;
    // 00417B10's sequential offset point, x and z only.
    virtual bool tick_avoid_zone_point(void* member, const float target[3],
                                       float out[2]) = 0;

    virtual float tick_tuning_field(std::uint32_t offset) = 0;             // 00A371A0
    virtual float tick_horizontal_distance(const float a[3], const float b[3]) = 0;
    // 00A2BD00 with a replacement of the given class, bound to the same target.
    virtual void tick_replace_command(void* group, AiCommandType type) = 0;
};

// The shared follower pass, 00A10DC0.
AiCommandTickResult ai_command_follower_pass_00a10dc0(AiCommandTickHost& host, void* group);

// The `vt+0Ch` tick for the five classes read in full. Any other class returns
// an empty result, because its body was not read.
AiCommandTickResult ai_command_tick_vt000c(AiCommandTickHost& host,
                                           const AiCommandObject& command);

}  // namespace bsp
