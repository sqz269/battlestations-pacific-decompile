#pragma once
#include <cstdint>

// The party brain's planner classes and the AI command classes, reconstructed
// from the read-only analysis of packet cc2_ai_planners. docs/AI_PLANNERS.md
// carries the address evidence. Every descriptive name here is a hypothesis,
// not a recovered symbol, with two exceptions that are string literals in the
// image and therefore genuine: the eight planner names ("Defend", "Attack",
// "Sell", "Capture", "Duel", "Escort", "Siege", "Competitive", each also
// present as "<name> Coordinator") and the fifteen command type names
// ("NONCONTROL" .. "SELLING") in the table at 00E0E308.
//
// Coverage. Complete as rules: the planner class table and its brain slots
// (00A15A70), the planner object layout (00A1EE50), the command type space and
// its IsType hierarchy (the twelve accessor pairs), the merge predicate at
// command vtable +14h (00A11F80 / 00A12450), the engaged test 00A2CB10, the
// replan-flag virtuals 00A18480 / 00A1D110..00A1D1A0, the attack order choice
// 00A2CBD0, the target scoring loop 00A1CB80 and the engagement pass 00A179E0.
// Partial: the Defend think 00A28A60 and the Capture think 00A29FD0 are read
// only for what they claim and what they issue; their scoring is not
// reconstructed here.
//
// This projection reuses bsp/ai_group_think.hpp rather than redeclaring the
// group layout, the per-team registry or the planner choice, all of which that
// header already carries.
namespace bsp {

// ---------------------------------------------------------------------------
// The planner object (00A1EE50 base ctor; 38h bytes, 44h for Capture)
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kAiPlannerVtable = 0x00u;          // base 00D22D08
inline constexpr std::uint32_t kAiPlannerNameLength = 0x14u;      // native string, length
inline constexpr std::uint32_t kAiPlannerNameData = 0x18u;        // native string, data
inline constexpr std::uint32_t kAiPlannerBrain = 0x1Cu;           // the party brain
inline constexpr std::uint32_t kAiPlannerGroupList = 0x20u;       // std::list object
inline constexpr std::uint32_t kAiPlannerGroupListRoot = 0x24u;   // 00A168A0 sentinel
inline constexpr std::uint32_t kAiPlannerGroupCount = 0x28u;      // list size
inline constexpr std::uint32_t kAiPlannerReplanFlag = 0x2Cu;      // byte, set by the claim
inline constexpr std::uint32_t kAiPlannerOwnTeam = 0x30u;         // = brain+24h
inline constexpr std::uint32_t kAiPlannerEnemyTeam = 0x34u;       // = (brain+24h == 0)
inline constexpr std::uint32_t kAiPlannerInstanceSize = 0x38u;
inline constexpr std::uint32_t kAiPlannerCaptureInstanceSize = 0x44u;

// Virtual slots of the planner vtable (thirteen entries, +0h..+30h).
inline constexpr std::uint32_t kAiPlannerVtableTick = 0x20u;
inline constexpr std::uint32_t kAiPlannerVtableNeedsReplan = 0x30u;

// The eight planner classes. The first four fill brain+0h..+0Ch and are ticked
// on every party think; the last four fill brain+10h..+1Ch, one per game mode,
// and are reached through vtable +30h as well.
enum class AiPlannerKind {
    Defend,       // brain+0h
    Attack,       // brain+4h
    Sell,         // brain+8h
    Capture,      // brain+0Ch
    Duel,         // brain+10h, game mode 4
    Escort,       // brain+14h, game mode 5
    Siege,        // brain+18h, game mode 6
    Competitive,  // brain+1Ch, game mode 7
};
inline constexpr int kAiPlannerKindCount = 8;

struct AiPlannerClass {
    AiPlannerKind kind;
    const char* name;             // the string the constructor stores at +14h
    std::uint32_t brain_slot;     // byte offset in the 28h-byte brain
    std::uint32_t vtable;
    std::uint32_t constructor;
    std::uint32_t think;          // vtable +20h
    std::uint32_t needs_replan;   // vtable +30h
    std::uint32_t instance_size;
    int game_mode;                // 004BCA50's value, or -1 for the first four
};

// Indexed by AiPlannerKind. The table is the constructor's own dispatch.
const AiPlannerClass* ai_planner_class_table() noexcept;
const AiPlannerClass& ai_planner_class(AiPlannerKind kind) noexcept;

// 00A15A70's branch on 004BCA50: modes 4..7 construct exactly one mode planner
// and leave brain+0h..+0Ch null; every other mode constructs the first four and
// leaves brain+10h..+1Ch null.
bool ai_planner_slot_constructed(AiPlannerKind kind, int game_mode) noexcept;

// ---------------------------------------------------------------------------
// The AI command classes (sixteen vtables, fifteen type ids)
// ---------------------------------------------------------------------------

// vtable +4h returns the id; the name table at 00E0E308 is indexed by it.
enum class AiCommandType : int {
    NonControl = 0,
    Idle = 1,
    Move = 2,             // abstract, vtable 00D22A94
    MoveTo = 3,
    CautiousMove = 4,
    RegroupingMove = 5,
    Attack = 6,           // abstract, vtable 00D22B7C
    MoveToAttack = 7,
    CautiousAttack = 8,
    CloseAttack = 9,
    Defend = 10,          // abstract, vtable 00D22A10
    DefendPosition = 11,
    PatrolTo = 12,
    Retreat = 13,
    Selling = 14,
};
inline constexpr int kAiCommandTypeCount = 15;
inline constexpr std::uint32_t kAiCommandTypeNameTable = 0x00E0E308u;

// Layout the ATTACK base constructor 00A10710 writes.
inline constexpr std::uint32_t kAiCommandPrimaryVtable = 0x00u;
inline constexpr std::uint32_t kAiCommandOwnerGroup = 0x04u;
inline constexpr std::uint32_t kAiCommandObserverSubobject = 0x08u;
inline constexpr std::uint32_t kAiCommandTargetGroup = 0x1Cu;

// Virtual slots of the command vtable (ten entries, +0h..+24h).
inline constexpr std::uint32_t kAiCommandVtableGetType = 0x04u;
inline constexpr std::uint32_t kAiCommandVtableIsType = 0x08u;
inline constexpr std::uint32_t kAiCommandVtableCanMergeWith = 0x14u;
inline constexpr std::uint32_t kAiCommandVtableDescribe = 0x18u;
inline constexpr std::uint32_t kAiCommandVtableNotifyEntity = 0x24u;

const char* ai_command_type_name(AiCommandType type) noexcept;

// The IsType bodies. Each concrete class answers its own id and, for the three
// families, the abstract base id as well; NonControl, Idle and Selling answer
// only themselves.
AiCommandType ai_command_family(AiCommandType type) noexcept;
bool ai_command_is_type(AiCommandType self, AiCommandType query) noexcept;

// vtable +14h defaults to false (00A0FC80); only NonControl (00A11F80) and Idle
// (00A12450) override it, so a group carrying any other command never merges.
bool ai_command_overrides_merge(AiCommandType type) noexcept;

// The shape of that override. `same_grouping_answer` is
// HasGroupableCombatant(mine) == HasGroupableCombatant(other) and
// `same_secondary_answer` the same equality on 00A2C600, which only the Idle
// arm evaluates.
struct AiCommandMergeInputs {
    AiCommandType type{AiCommandType::NonControl};
    bool other_group_present{false};      // the argument is non-null
    std::uint32_t other_population{0};    // other+5644h
    std::uint32_t own_population{0};      // ownerGroup+5644h
    bool same_grouping_answer{false};
    bool same_secondary_answer{false};
    bool extra_test_a{false};             // 00A10D50(other)
    bool extra_test_b{false};             // 00A10C60(other)
};
bool ai_command_can_merge(const AiCommandMergeInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Replanning
// ---------------------------------------------------------------------------

// 00A2CB10. A group counts as engaged while it carries any ATTACK or any DEFEND
// command, and while a MOVE command's destination is still farther than the
// threshold; NONCONTROL, IDLE and SELLING are never engaged.
inline constexpr std::uint32_t kAiCommandEngagedMoveRadiusSquared = 0x00CE3D64u;
bool ai_group_command_is_engaged(AiCommandType type, float move_target_distance_squared,
                                 float move_radius_squared) noexcept;

// The vtable +30h virtuals, which the immediate-think override 00A15970 asks.
// Both variants read and clear the flag at planner+2Ch. The base (00A18480,
// worn by the first four planners) returns it unchanged; each mode planner
// (00A1D110, 00A1D140, 00A1D170, 00A1D1A0, identical bodies) also forces a
// replan when its first owned group has stopped being engaged.
struct AiPlannerReplanInputs {
    bool flag_set{false};                 // planner+2Ch on entry
    std::uint32_t owned_group_count{0};   // planner+28h
    bool first_group_engaged{false};      // 00A2CB10 on the first owned group
};
bool ai_planner_needs_replan_base(const AiPlannerReplanInputs& in) noexcept;
bool ai_planner_needs_replan_mode(const AiPlannerReplanInputs& in) noexcept;
bool ai_planner_needs_replan(AiPlannerKind kind, const AiPlannerReplanInputs& in) noexcept;

// ---------------------------------------------------------------------------
// Ordering an attack (00A2CBD0)
// ---------------------------------------------------------------------------

// The command the group is given for a chosen target. 00A2CBD0 keeps the
// current command when the group is already attacking that very target, and
// otherwise builds a CautiousAttack only when all three of the guards hold;
// `caution_roll` is 00BD2F40's value and a higher `aggressive` ratio therefore
// makes CautiousAttack less likely.
enum class AiAttackOrder { KeepCurrent, MoveToAttack, CautiousAttack, None };
struct AiAttackOrderInputs {
    std::uint32_t own_population{0};              // group+5644h
    AiCommandType current_type{AiCommandType::NonControl};
    bool current_target_is_chosen{false};         // command+1Ch == target
    bool member_prefers_direct_attack{false};     // member vtable +5Ch (6)
    bool target_position_accepted{false};         // 00A2C9F0
    float caution_roll{0.0F};
    float aggressive{0.0F};
};
AiAttackOrder ai_group_attack_order(const AiAttackOrderInputs& in) noexcept;
AiCommandType ai_attack_order_command_type(AiAttackOrder order) noexcept;

// ---------------------------------------------------------------------------
// Target scoring (00A1CB80)
// ---------------------------------------------------------------------------

// The per-candidate weight, product of four factors. `base` is 00A0F970's
// value for the candidate, `range` the clamped interpolation of the tuning
// record's +1D0h and +1D4h over the planar distance, `own_set` the tuning
// record's +1CCh when the candidate has a member in the planner's own world
// set, and `sticky` the tuning record's +1D8h when the candidate is already
// this group's target. Both default to +1.0 (00D7A24C).
inline constexpr std::uint32_t kAiPlannerTuningOwnSetFactor = 0x1CCu;
inline constexpr std::uint32_t kAiPlannerTuningRangeNear = 0x1D0u;
inline constexpr std::uint32_t kAiPlannerTuningRangeFar = 0x1D4u;
inline constexpr std::uint32_t kAiPlannerTuningStickyFactor = 0x1D8u;
inline constexpr float kAiPlannerDefaultWeightFactor = 1.0F;   // 00D7A24C
// The loop's running maximum starts here (00D22CC4), so a candidate scoring at
// or below it is never chosen even when it is the only one.
inline constexpr float kAiPlannerScoreFloor = -999999.0F;      // 00D22CC4
// The near radius the range factor collapses to zero inside: 00CE3820 is the
// double 1.0e-11, so only coincident groups take that arm.
inline constexpr double kAiPlannerNearRadiusSquared = 1.0e-11; // 00CE3820

struct AiTargetScoreInputs {
    float base{0.0F};
    float range{0.0F};
    float own_set{kAiPlannerDefaultWeightFactor};
    float sticky{kAiPlannerDefaultWeightFactor};
};
float ai_target_score(const AiTargetScoreInputs& in) noexcept;

// The distance the range factor is evaluated at: zero inside the near radius,
// the true planar distance outside it.
float ai_target_scoring_distance(float squared_planar_distance,
                                 float near_radius_squared) noexcept;

// ---------------------------------------------------------------------------
// The brain's engagement pass (00A179E0)
// ---------------------------------------------------------------------------

// Each own-team group is paired against each enemy-team group, and a pair is
// recorded only when both are populated, the own group belongs to this brain's
// party, the two are within the engagement radius and their strengths are
// within a factor of each other. The radius and the factor are image constants.
inline constexpr std::uint32_t kAiEngagementRadiusSquared = 0x00D22CB8u;
inline constexpr std::uint32_t kAiEngagementStrengthFactor = 0x00D7A280u;
inline constexpr std::uint32_t kAiEngagementMinStrength = 0x00CF3F20u;

struct AiEngagementPairInputs {
    std::uint32_t own_population{0};
    std::uint32_t enemy_population{0};
    int own_group_party{-1};
    int brain_party{-1};
    float squared_planar_distance{0.0F};
    float radius_squared{0.0F};
    float own_strength{0.0F};
    float enemy_strength{0.0F};
    float min_strength{0.0F};
    float strength_factor{0.0F};
};
bool ai_engagement_pair_accepted(const AiEngagementPairInputs& in) noexcept;

// The enemy team index the pass and every planner use: the flip is `team == 0`,
// so it only distinguishes teams 0 and 1 and maps team 2 onto team 0.
int ai_enemy_team_index(int own_team) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site the two reconstructed passes reach. Nothing
// here has a default implementation and none of it stands in for unrecovered
// game behaviour.
struct AiPlannerHost {
    virtual ~AiPlannerHost() = default;

    // 00A1CB80's inputs
    virtual std::uint32_t enemy_team_group_count(int enemy_team) = 0;         // 00F8AA50 + t*0Ch
    virtual void* first_enemy_team_group(int enemy_team) = 0;                 // 00F8AA4C + t*0Ch
    virtual void* next_group(void* node) = 0;
    virtual bool group_is_end(int enemy_team, void* node) = 0;
    virtual std::uint32_t group_population(void* group) = 0;                  // group+5644h
    virtual AiCommandType group_command_type(void* group) = 0;                // vtable +4h
    virtual void* group_command_target(void* group) = 0;                      // command+1Ch
    virtual void clear_group_target_cache(void* group) = 0;                   // +5624h, +5628h
    virtual float candidate_base_weight(void* group) = 0;                     // 00A0F970
    virtual float squared_planar_distance(void* a, void* b) = 0;              // leader poses
    virtual float near_radius_squared() = 0;                                  // 00CE3820
    virtual float tuning_field(std::uint32_t offset) = 0;                     // 00A371A0 + off
    virtual float range_interpolation(float near_value, float far_value, float distance) = 0;
    virtual bool candidate_has_member_in_own_set(void* group, int own_team) = 0;  // 00A2C450
    virtual void order_attack(void* group, void* target, float aggressive) = 0;   // 00A2CBD0

    // 00A26510 / 00A265F0's spawn arm
    virtual void quick_spawn_named_group(const char* tag) = 0;                // 00A25B90
    virtual void publish_spawn_bias(float bias) = 0;                          // 00A16450
    virtual bool reset_target_flag() = 0;                                      // 00F8A9E0 == 3
};

// The literal the spawn arm passes to 00A16450 (00CE3800).
inline constexpr float kAiPlannerSpawnBias = 0.5F;

// The engagement pass's two image constants, decoded.
inline constexpr float kAiEngagementRadiusSquaredValue = 9000000.0F; // 00D22CB8
inline constexpr float kAiEngagementStrengthFactorValue = 0.5F;      // 00D7A280, a double
inline constexpr float kAiEngagementMinStrengthValue = 15.0F;        // 00CF3F20, a double
// 00A2CB10's MOVE radius, squared (00CE3D64).
inline constexpr float kAiCommandEngagedMoveRadiusSquaredValue = 10000.0F;

// 00A1CB80 as a sequence over the host: score every enemy-team group and order
// the best one attacked. Returns the chosen group, or null when none scored.
void* ai_planner_choose_attack_target(AiPlannerHost& host, void* group, int own_team,
                                     int enemy_team, float aggressive, bool reset_target);

// The mode planners' think (00A26510 and 00A265F0 read in full; 00A25F70 and
// 00A26210 share the arm and then add their own order). With no owned group the
// planner spawns one tagged group and stops; otherwise it runs the target pass
// on its first owned group with aggressive = 1.0.
struct AiModePlannerTickInputs {
    AiPlannerKind kind{AiPlannerKind::Siege};
    std::uint32_t owned_group_count{0};
    void* first_owned_group{nullptr};
    int own_team{0};
};
void ai_mode_planner_tick(AiPlannerHost& host, const AiModePlannerTickInputs& in);

// The bracketed tag each planner spawns its group with.
const char* ai_planner_spawn_tag(AiPlannerKind kind) noexcept;

} // namespace bsp
