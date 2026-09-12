#pragma once
#include <cstddef>
#include <cstdint>

// The scoring tails of the AI commander's Defend and Capture planners, the AI
// command factory and the script group spawner, reconstructed from the
// read-only analysis of packet cc2_ai_planner_tails. docs/AI_PLANNER_TAILS.md
// carries the address evidence. Every descriptive name here is a hypothesis,
// not a recovered symbol, with one exception that is a genuine string literal
// in the image: the group-name prefixes "[defend]" (00D22C9C) and "[capture]"
// (00D22CA8).
//
// Coverage. Complete as rules: the capture target score 00A1E250 with its two
// clamps, the per-unit arrival value 00A03760, the horizontal length 009FFC10,
// the Defend merge pass 00A29860-00A29BE7, the Capture spawn-delay ramp, the
// attack-weight interpolation and the spawn budget test, the record price at
// rec+1Ch, the command factory 00A13340, the script spawner's prefix routing
// 00A16EF0 and the two command merge tests 00A10D50 / 00A10C60. Partial: the
// Defend think 00A28A60 outside its merge pass and the Capture think 00A29FD0
// outside its four scoring blocks.
//
// This projection reuses bsp/ai_planners.hpp and bsp/ai_group_think.hpp rather
// than redeclaring the planner layout, the command type space or the auto-merge
// distance rule, all of which those headers already carry.
#include "bsp/ai_planners.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// Tuning record offsets the two thinks read through 00A371A0. The reader side
// is `coordinator + 4 + mode*23Ch`, so these are the `record` column of
// docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md.
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kAiTailTuningArriveToRangeTime = 0x198u;  // Capture_ArriveToRangeTime
inline constexpr std::uint32_t kAiTailTuningCapturePointValue = 0x19Cu;  // Capture_CapturePointResourceValue
inline constexpr std::uint32_t kAiTailTuningCaptureMinResource = 0x1A0u; // Capture_MinimalResource
inline constexpr std::uint32_t kAiTailTuningActAttackMul2 = 0x1A8u;      // ActAttackTargetWeightMul[2]
inline constexpr std::uint32_t kAiTailTuningActAttackMul1 = 0x1ACu;      // ActAttackTargetWeightMul[1]
inline constexpr std::uint32_t kAiTailTuningActAttackDist1 = 0x1B0u;     // ActAttackTargetWeightMulDist[1]
inline constexpr std::uint32_t kAiTailTuningActAttackDist2 = 0x1B4u;     // ActAttackTargetWeightMulDist[2]
inline constexpr std::uint32_t kAiTailTuningMinCbTargetWeight = 0x1B8u;  // Capture_MinimalCBTargetWeight
inline constexpr std::uint32_t kAiTailTuningCbStrategicMul = 0x1BCu;     // Capture_CommandBuildingStrategicWeightMul
inline constexpr std::uint32_t kAiTailTuningSpawnDelayStart = 0x1C0u;    // Capture_SpawnDelay[1]
inline constexpr std::uint32_t kAiTailTuningSpawnDelayEnd = 0x1C4u;      // Capture_SpawnDelay[2]
inline constexpr std::uint32_t kAiTailTuningSpawnDelayTime = 0x1C8u;     // Capture_SpawnDelayTime
inline constexpr std::uint32_t kAiTailTuningDefendMergeTargetDist = 0x1DCu; // Defend_MergeTargetDist
inline constexpr std::uint32_t kAiTailTuningDefendMergeGroupsDist = 0x1E0u; // Defend_MergeGroupsDist

// The shipped defaults of scripts\datatables\highlvlaiglobals.lua, for the
// rules below to be exercised with the values the game actually runs.
inline constexpr float kAiTailDefaultArriveToRangeTime = 30.0F;
inline constexpr float kAiTailDefaultCapturePointValue = 35.0F;
inline constexpr float kAiTailDefaultCaptureMinResource = 150.0F;
inline constexpr float kAiTailDefaultMinCbTargetWeight = 0.5F;
inline constexpr float kAiTailDefaultCbStrategicMul = 0.8F;
inline constexpr float kAiTailDefaultSpawnDelayStart = 30.0F;
inline constexpr float kAiTailDefaultSpawnDelayEnd = 5.0F;
inline constexpr float kAiTailDefaultSpawnDelayTime = 120.0F;
inline constexpr float kAiTailDefaultDefendMergeTargetDist = 2000.0F;
inline constexpr float kAiTailDefaultDefendMergeGroupsDist = 500.0F;

// Literal constants the two bodies load.
inline constexpr float kAiTailEnemyHalfWeight = 0.5F;        // 00D7A280, a double
inline constexpr float kAiTailDistanceReference = 1000.0F;   // 00CE3804 and 00CE47A0
inline constexpr float kAiTailNoUnitDistance = 1.0e10F;      // seed 0x501502F9 at 00A1E3D5
inline constexpr float kAiTailMinArrivalSpeed = 0.1F;        // 00D7A3A0, a double
inline constexpr double kAiTailLengthEpsilon = 1.0e-11;      // 00CE3820
inline constexpr int kAiTailTeamCount = 3;                   // the [ESP+34h] array is three wide

// ---------------------------------------------------------------------------
// 009FFC10. The XZ length every AI distance rule uses; y is never read, and the
// result is exactly zero at or below the epsilon.
// ---------------------------------------------------------------------------

float ai_tail_horizontal_length(const float v[3]) noexcept;

// The enemy team of a team, the (team == 0) flip at 00A1E27D. Teams 0 and 1 are
// distinguished; team 2 maps onto team 0.
int ai_tail_enemy_team(int own_team) noexcept;

// ---------------------------------------------------------------------------
// 00A03760, one unit's contribution to a capture target's value.
// ---------------------------------------------------------------------------

struct AiTailArrivalValueInputs {
    float capture_weight = 0.0F;    // 00A03510, the unit's Lua CaptureWeight
    float distance = 0.0F;          // horizontal length of unit - target
    float capture_radius = 0.0F;    // (float)(int)target[+7A0h]
    float speed = 0.0F;             // 0 when the unit is neither IsType(6) nor IsType(18h)
    float arrive_to_range_time = kAiTailDefaultArriveToRangeTime;
};

float ai_tail_unit_arrival_value(const AiTailArrivalValueInputs& in) noexcept;

// ---------------------------------------------------------------------------
// 00A1E250, the capture target score. The six floats are written to out[0..5]
// in the order the format string at 00D22CC8 prints them.
// ---------------------------------------------------------------------------

struct AiTailCaptureScoreTerms {
    float total = 0.0F;       // out[0], the return value
    float own_value = 0.0F;   // out[1], `a`
    float enemy_value = 0.0F; // out[2], `b`
    float own_reach = 0.0F;   // out[3], `c`
    float enemy_reach = 0.0F; // out[4], `d`
    float strategic_gain = 0.0F; // out[5], `e`, the target's Lua StrategicGain
};

struct AiTailCaptureScoreInputs {
    float own_value = 0.0F;
    float enemy_value = 0.0F;
    float own_resources = 0.0F;
    float enemy_resources = 0.0F;
    float own_nearest_distance = kAiTailNoUnitDistance;
    float enemy_nearest_distance = kAiTailNoUnitDistance;
    float strategic_gain = 0.0F;
    float capture_point_value = kAiTailDefaultCapturePointValue;
    float min_cb_target_weight = kAiTailDefaultMinCbTargetWeight;
};

// 1000 / max(distance, 1000), the reach factor at 00A1E654 and 00A1E692.
float ai_tail_reach_factor(float nearest_distance) noexcept;

AiTailCaptureScoreTerms ai_tail_capture_score(const AiTailCaptureScoreInputs& in) noexcept;

// The record price at rec+1Ch, 00A2A18B-00A2A1E2.
float ai_tail_capture_record_price(const AiTailCaptureScoreTerms& terms, float target_capture_weight,
                                   float capture_point_value, float min_resource) noexcept;

// ---------------------------------------------------------------------------
// The Defend merge pass, 00A29860-00A29BE7.
// ---------------------------------------------------------------------------

// dist2D(leader, defended)^2 <= merge_target_dist^2. A group that fails this is
// skipped before any pairing is considered.
bool ai_tail_defend_group_near_target(const float leader[3], const float defended[3],
                                      float merge_target_dist) noexcept;

// merge_groups_dist^2 > dist2D(a, b)^2, the pair test that triggers the merge.
bool ai_tail_defend_groups_mergeable(const float leader_a[3], const float leader_b[3],
                                     float merge_groups_dist) noexcept;

// ---------------------------------------------------------------------------
// The Capture think's scoring blocks.
// ---------------------------------------------------------------------------

// 00A2A046-00A2A0C1. The spawn gate; `planner_age` drives the ramp and
// `since_last_spawn` is compared against it.
float ai_tail_capture_spawn_delay(float planner_age, float delay_start, float delay_end,
                                  float delay_time) noexcept;
bool ai_tail_capture_spawn_due(float planner_age, float since_last_spawn, float delay_start,
                               float delay_end, float delay_time) noexcept;

// 00A2A5FA-00A2A646. 100x inside 3000 units falling to 1.5x at 4500 and beyond,
// with the shipped defaults.
float ai_tail_capture_attack_weight(float distance, float near_dist, float near_mul,
                                    float far_dist, float far_mul) noexcept;

// 00A2AC11-00A2AC4E. The blend the best-target pick scores.
float ai_tail_capture_blend(float own_weight, float strategic_weight,
                            float command_building_mul) noexcept;

// 00A2B4A6-00A2B4EE. The Capture planner may spend what the Defend reserve
// leaves; composing a new group needs strictly more than one point.
float ai_tail_capture_spawn_budget(float party_resource_cap, float defend_resource_percent,
                                   float already_committed) noexcept;
bool ai_tail_capture_may_compose(float budget) noexcept;

// ---------------------------------------------------------------------------
// 00A10D50 and 00A10C60, the two tests both command merge overrides end with.
// ---------------------------------------------------------------------------

// 00A10D50: merge only into an equal or stronger lead unit.
bool ai_tail_merge_leader_strength_ok(float own_leader_weight, float other_leader_weight) noexcept;

// 00A10C60 is the same rule as ai_group_within_auto_merge_distance in
// bsp/ai_group_think.hpp, reading the same tuning field +208h. Callers should
// use that one; it is named here only so the doc's cross-reference has a symbol.
bool ai_tail_merge_leader_distance_ok(const float own_leader[3], const float other_leader[3],
                                      float auto_merge_dist) noexcept;

// ---------------------------------------------------------------------------
// 00A13340, the Lua command factory.
// ---------------------------------------------------------------------------

// What the factory needs from the Lua table besides the class.
enum class AiTailCommandArgument { None, TargetPosition, TargetEntity };

struct AiTailCommandRecipe {
    AiCommandType type = AiCommandType::Idle;
    std::uint32_t instance_size = 0x08u;
    AiTailCommandArgument argument = AiTailCommandArgument::None;
};

// The case-insensitive name-to-class map at 00A133CD-00A13787. An unmatched or
// absent `commandType` yields IDLE; an exactly-"NONCONTROL" name yields
// NONCONTROL. `command_type_name` may be null, which is the absent case.
AiTailCommandRecipe ai_tail_command_recipe(const char* command_type_name) noexcept;

// The Lua keys the factory reads.
inline constexpr const char* kAiTailCommandTypeKey = "commandType";  // 00D22A88
inline constexpr const char* kAiTailTargetPositionKey = "targetPos"; // 00D22C78
inline constexpr const char* kAiTailTargetEntityKey = "target";      // 00CFD964

// ---------------------------------------------------------------------------
// 00A16EF0, the script group spawner.
// ---------------------------------------------------------------------------

inline constexpr const char* kAiTailCapturePrefix = "[capture]"; // 00D22CA8, 9 chars
inline constexpr const char* kAiTailDefendPrefix = "[defend]";   // 00D22C9C, 8 chars

// Which planner slot of the party brain a freshly spawned script group lands
// in. Unclaimed means the brain's own think picks it up on the next party tick.
enum class AiTailSpawnRoute { Unclaimed, Defend, Capture, Duel, Escort, Siege, Competitive };

struct AiTailSpawnRouting {
    AiTailSpawnRoute route = AiTailSpawnRoute::Unclaimed;
    std::uint32_t brain_planner_offset = 0u; // brain+0h .. brain+1Ch
    std::size_t handle_text_offset = 0u;     // where the entity id starts in the name
};

// `game_mode` is 009FFC80's space as 004BCA50 reports it. `group_name` may be
// null; the prefix comparison is case-insensitive.
AiTailSpawnRouting ai_tail_spawn_routing(int game_mode, const char* group_name) noexcept;

// ---------------------------------------------------------------------------
// The sequence routine. One method per native call site of 00A1E250.
// ---------------------------------------------------------------------------

struct AiCaptureScoreHost {
    virtual ~AiCaptureScoreHost() = default;

    // 00A1E2A0 / 00A1E2F1 / 00A1E343 / 00A1E395: walk the four world lists and
    // report each unit's owner team.
    virtual int world_unit_count() = 0;
    virtual int world_unit_team(int index) = 0;
    // 00A03760 at the four accumulation sites.
    virtual float unit_arrival_value(int index, void* target) = 0;

    // 00A1E410-00A1E64E: the nearest-unit loop over world+19CCh+16Ch.
    virtual int near_unit_count() = 0;
    virtual int near_unit_team(int index) = 0;
    // 009FFC10 at 00A1E5E7 after BSP_EntityPose_RefreshWorld at 00A1E42E / 00A1E506.
    virtual float near_unit_distance(int index, void* target) = 0;

    // 00946FC0 at 00A1E6D9 and 00A1E701.
    virtual float available_resources(int team) = 0;
    // 00A371A0 at 00A1E6C0, 00A1E6E6 and 00A1E74B.
    virtual float tuning(std::uint32_t offset) = 0;
    // 00927B40 + 00B67800 + 00B66330 at 00A1E7A5-00A1E7DA, the StrategicGain read.
    virtual float strategic_gain(void* target) = 0;
};

AiTailCaptureScoreTerms ai_capture_target_score(AiCaptureScoreHost& host, void* target,
                                                int own_team);

}  // namespace bsp
