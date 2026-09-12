#pragma once
#include <cstdint>

// The party-level AI commander's think, reconstructed from the read-only
// analysis of packet cc2_ai_group_think. docs/AI_GROUP_THINK.md carries the
// address evidence; every descriptive name here is a hypothesis, not a
// recovered symbol. The two genuine recovered names are the Lua tuning keys
// `AutoMerge_MergeDist` and `Defend_ResourcePercent`, which are string
// literals in the image.
//
// Coverage: the three passes the fixed step runs (00A32D50 -> 00A2E720 and
// 00A182C0 -> 00A181A0) are complete as rules. The bodies the rules call into
// are contracts on the host, not reconstructions: the planner objects behind
// the brain's eight slots, the AI command objects at group+564Ch, and the
// party brain's own planning tail 00A179E0 are all unread here.
namespace bsp {

// ---------------------------------------------------------------------------
// The group object (00A2DFA0 allocates 5660h at 00A38CAE and 00A2E41A)
// ---------------------------------------------------------------------------

// Offsets the constructor writes. The 0x5604 bytes between +24h and +5623h are
// written by neither 00A2DFA0 nor its caller's allocation (00BF681B does not
// zero), so this projection deliberately stops at the fields with a producer.
inline constexpr std::uint32_t kAiGroupPrimaryVtable = 0x00u;         // 00D23084
inline constexpr std::uint32_t kAiGroupObserverSubobject = 0x10u;     // 00D2306C
inline constexpr std::uint32_t kAiGroupMemberList = 0x563Cu;          // std::list object
inline constexpr std::uint32_t kAiGroupMemberListRoot = 0x5640u;      // 004C1630 sentinel
inline constexpr std::uint32_t kAiGroupPopulation = 0x5644u;          // list size
inline constexpr std::uint32_t kAiGroupGroupingEnabled = 0x5648u;     // byte, 1
inline constexpr std::uint32_t kAiGroupCurrentCommand = 0x564Cu;      // owned, 8 bytes
inline constexpr std::uint32_t kAiGroupClaimingPlanner = 0x5654u;     // set by 00A22750
inline constexpr std::uint32_t kAiGroupPartySlot = 0x5634u;           // 009FFD20(first member)
inline constexpr std::uint32_t kAiGroupTeamId = 0x5638u;              // firstMember+54h
inline constexpr std::uint32_t kAiGroupId = 0x5658u;                  // 00E0E348, wraps 1..999
inline constexpr std::uint32_t kAiGroupInstanceSize = 0x5660u;

// The entity's back-pointer to its group, written at 00A2D8F5 and cleared at
// 00A2E3AF (split) and 00A2DE7E (eviction).
inline constexpr std::uint32_t kAiGroupEntityBackPointer = 0x016Cu;

// Registries the constructor links a group into. Each is a list container whose
// root node pointer sits at base+4h; 00F8AA78's size lives at 00F8AA80.
inline constexpr std::uint32_t kAiGroupPerPartyListBase = 0x00F8A9E8u;  // + party*12
inline constexpr std::uint32_t kAiGroupPerTeamListBase = 0x00F8AA48u;   // + team*12
inline constexpr std::uint32_t kAiGroupGlobalList = 0x00F8AA6Cu;
inline constexpr std::uint32_t kAiGroupEmptiedList = 0x00F8AA78u;       // pending delete

// Per-party AI state. 00A182C0 walks 0xF8A8C8 to <0xF8A9A8 with stride 1Ch, and
// 00A16B20 bounds the brain array with CMP EDI,0xF8A8BC, so there are exactly
// eight party slots and the Defend_ResourcePercent array is three dwords wide.
inline constexpr int kAiGroupPartySlotCount = 8;
inline constexpr std::uint32_t kAiGroupPartyRecordBase = 0x00F8A8C8u;
inline constexpr std::uint32_t kAiGroupPartyRecordStride = 0x1Cu;
inline constexpr std::uint32_t kAiGroupNextThinkTimeBase = 0x00F8A87Cu; // + party*4
inline constexpr std::uint32_t kAiGroupBrainSlotBase = 0x00F8A89Cu;     // + party*4
inline constexpr std::uint32_t kAiGroupDefendResourcePercentBase = 0x00F8A8BCu;
inline constexpr int kAiGroupDefendResourcePercentCount = 3; // guarded at 00A360F5

// The ambient party index the whole think publishes. 00A182C0 and 00A2E720 set
// it per party and restore -1 on the way out; 00A25C06 reads it to pick the
// quick-spawn record, so a consumer never receives the party as an argument.
inline constexpr std::uint32_t kAiGroupCurrentPartyGlobal = 0x00E0E344u;

// The per-party 1Ch record at 00F8A8C8 + party*1Ch.
struct AiPartyRecord {
    bool ai_enabled{false};          // +0h,  tested at 00A1834F
    float attack_ratio{0.0f};        // +4h,  written at 00A163D6/00A32F29; no read site found
    float aggressive_ratio{0.0f};    // +8h,  read at 00A1A73E through 00A184F0
    bool quick_spawn_valid{false};   // +0Ch, tested at 00A25C1B
    float quick_spawn_pos[3]{};      // +10h..+18h, address taken at 00A25C23
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// 00A18510 and 00A18520, two routines Ghidra has no function for. The second
// stores through memory before returning, so it truncates to float twice.
float ai_defend_resource_percent(const float* table, int difficulty) noexcept;
float ai_attack_resource_percent(const float* table, int difficulty) noexcept;

// 00A182C0's schedule gate and reschedule. 00CE3854 is 3.0f and 00CE3850 is
// 5.0f, and 00BD2F10 is BSP_Random_UniformFloatRange, so a party thinks on a
// three to five second jitter rather than on the fixed step.
inline constexpr float kAiGroupThinkIntervalMin = 3.0f; // 00CE3854
inline constexpr float kAiGroupThinkIntervalMax = 5.0f; // 00CE3850
bool ai_party_think_due(float now, float next_think_time) noexcept;
float ai_party_next_think_time(float now, float random_interval) noexcept;

// 009FFE50. In modes 0 through 3 only party 0 and party 4 get a brain, which is
// what makes those two the two sides' commander slots. Above mode 3 every party
// is enabled. The caller has already established that world+61Ch is set.
bool ai_party_ai_enabled(int game_mode, int party_slot) noexcept;

// 00A181A0's mode dispatch. Modes 4 through 7 tick one mode-specific planner
// and skip the group walk entirely; anything else runs the group walk.
enum class AiPartyThinkMode {
    GroupWalk,       // 004BCA50 not in 4..7
    ModeSpecific4,   // brain+10h
    ModeSpecific5,   // brain+14h
    ModeSpecific6,   // brain+18h
    ModeSpecific7,   // brain+1Ch
};
AiPartyThinkMode ai_party_think_mode(int game_mode) noexcept;

// The two group predicates 00A181A0 combines to pick a planner, and the choice
// itself (00A1825C..00A18272). `brain+0h` is the first planner and `brain+0Ch`
// the fourth.
enum class AiPlannerChoice { FirstPlanner, FourthPlanner };
AiPlannerChoice ai_planner_for_group(bool has_groupable_combatant,
                                     bool has_member_in_world_set) noexcept;

// 00A2C8D0's family gate, the part that does not delegate. HasShip is 00A2C6C0
// (any member of class id 06h) and HasAir is 00A2C660 (any member of class id
// 0Fh or 18h). A ship group and an air group never merge; every other pair is
// handed to the absorbed group's command object.
bool ai_group_families_may_merge(bool into_has_ship, bool into_has_air,
                                 bool from_has_ship, bool from_has_air) noexcept;

// 00A2EDB6..00A2EE11. The auto-merge distance test uses x and z only and
// compares the squared horizontal distance against the square of the tuning
// record's +208h, `AutoMerge_MergeDist` (shipped default 650).
bool ai_group_within_auto_merge_distance(const float leader_a[3],
                                         const float leader_b[3],
                                         float auto_merge_dist) noexcept;

// 00A2E260's split gate. The pass only moves members when the detached subset
// is a proper non-empty subset of the population.
bool ai_group_split_applies(std::uint32_t detached_count,
                            std::uint32_t population) noexcept;

// 00A2E850's seeding gate and 00A2DE26's eviction gate, which share four entity
// flag bytes. A seed candidate must also be ungrouped and on team 0 or 1.
struct AiGroupCandidateFlags {
    bool active{false};      // +5Ch must be set
    bool flag_5d{false};     // +5Dh must be clear
    bool flag_5e{false};     // +5Eh must be clear
    bool flag_60{false};     // +60h must be clear
};
bool ai_group_entity_flags_ok(const AiGroupCandidateFlags& flags) noexcept;
inline constexpr int kAiGroupMaxSeedTeam = 2; // CMP [ESI+54h],EBP with EBP = 2
bool ai_group_seed_candidate(const AiGroupCandidateFlags& flags, bool already_grouped,
                             int team_id) noexcept;
bool ai_group_member_still_belongs(const AiGroupCandidateFlags& flags, int entity_party_slot,
                                   int group_party_slot, int entity_team, int group_team) noexcept;

// 00A2DB80's command retarget. The absorbed group's dependents keep their class:
// 7, 8 and 9 map to 00A10890 (20h), 00A109B0 (2Ch) and 00A10AE0 (20h); any
// other class id leaves the command alone.
enum class AiCommandRetarget { None, Class7, Class8, Class9 };
AiCommandRetarget ai_command_retarget_for_class(int command_class_id) noexcept;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// The five world entity collections phase 3 of 00A2E720 seeds from, hung off
// world+19CCh at +64h, +13Ch and three more offsets in the same block.
inline constexpr int kAiGroupSeedCollectionCount = 5;

// One method per native call site the think reaches. Nothing here has a default
// implementation; none of these stands in for unrecovered game behaviour.
struct AiGroupThinkHost {
    virtual ~AiGroupThinkHost() = default;

    // 00A32D50
    virtual bool high_level_ai_enabled() = 0;         // byte 00E0E34C, statically 1

    // 00A2E720, the composition pass
    virtual std::uint32_t emptied_group_count() = 0;               // 00F8AA80
    virtual void* first_emptied_group() = 0;                       // 00F8AA7C
    virtual void* first_group_of_global_registry() = 0;            // 00F8AA70
    virtual void* first_group_of_proximity_list() = 0;             // 00F8AA64
    virtual void release_group_reference(void* holder, void* group) = 0; // 00A2B8F0
    virtual void unlink_and_free_emptied_node(void* group) = 0;    // 00BF65AC at 00A2E7BF
    virtual void destroy_group(void* group) = 0;                   // vtable+0h with flag 1
    virtual void evict_invalid_members(void* group) = 0;           // 00A2DDE0
    virtual void split_detached_members(void* group) = 0;          // 00A2E260
    virtual void* create_group(void* first_member) = 0;            // 00BF681B + 00A2DFA0
    virtual bool can_auto_merge(void* into, void* from) = 0;        // 00A2C8D0
    virtual void merge_group(void* into, void* from) = 0;           // 00A2DB80
    virtual void group_member_pass(void* group) = 0;               // 00A2C790
    virtual float auto_merge_dist() = 0;                           // 00A371A0()->+208h
    virtual int game_mode() = 0;                                   // 004BCA50

    // Phase 3 seeding, 00A2E835..00A2EA5A
    virtual void* first_seed_candidate(int collection) = 0;        // world+19CCh block
    virtual void* next_seed_candidate(void* cursor) = 0;           // node+4h
    virtual void* seed_candidate_entity(void* cursor) = 0;         // node+8h
    virtual AiGroupCandidateFlags entity_flags(void* entity) = 0;  // +5Ch,+5Dh,+5Eh,+60h
    virtual bool entity_has_group(void* entity) = 0;               // +16Ch
    virtual int entity_team(void* entity) = 0;                     // +54h
    virtual void add_group_member(void* group, void* entity) = 0;   // 00A2D8E0

    // Phase 5 proximity merge, 00A2EB95..00A2EE3E
    virtual double group_leader_order_key(void* group) = 0;        // 009FFD70(leader)
    virtual const float* group_leader_position(void* group) = 0;   // leader+FCh, cached at +C8h

    // 00A182C0, the party pass
    virtual float fixed_step_clock() = 0;                          // 00F876A4
    virtual float random_think_interval(float lo, float hi) = 0;    // 00BD2F10
    virtual void* party_brain(int party_slot) = 0;                  // 00F8A89C + p*4
    virtual void store_party_brain(int party_slot, void* brain) = 0;
    virtual float next_think_time(int party_slot) = 0;              // 00F8A87C + p*4
    virtual void store_next_think_time(int party_slot, float when) = 0;
    virtual bool party_record_enabled(int party_slot) = 0;          // record +0h
    virtual bool brain_wants_immediate_think(void* brain) = 0;      // 00A15970
    virtual void* create_party_brain(int party_slot) = 0;           // 00BF681B(28h) + 00A15A70
    virtual void destroy_party_brain(void* brain) = 0;              // 00A16490 + 00BF65AC
    virtual void set_current_party(int party_slot) = 0;             // 00E0E344
    virtual void party_brain_think(void* brain) = 0;                // 00A181A0

    // 00A181A0, the per-party think
    virtual void* brain_planner(void* brain, int slot) = 0;         // brain+0h..+1Ch
    virtual int brain_party_slot(void* brain) = 0;                  // brain+20h
    virtual int brain_world_set_index(void* brain) = 0;             // brain+24h
    virtual void planner_tick(void* planner) = 0;                   // vtable+20h
    virtual void* first_group_of_party(int party_slot) = 0;         // 00F8A9E8 + p*12
    virtual void* next_group(void* cursor) = 0;
    virtual std::uint32_t group_population(void* group) = 0;        // +5644h
    virtual void* group_claiming_planner(void* group) = 0;          // +5654h
    virtual bool group_has_groupable_combatant(void* group) = 0;    // 00A2C5A0
    virtual bool group_has_member_in_world_set(void* group, int set_index) = 0; // 00A2C450
    virtual void planner_claim_group(void* planner, void* group) = 0; // 00A22750
    virtual void party_brain_plan_tail(void* brain) = 0;            // 00A179E0
};

// 00A32D50, the AI coordinator's fixed-step advance. Native shape is
// __thiscall(node) with one stack argument (RET 4) that neither callee
// receives, so the 0.05f step the wave passes is discarded; the sequence takes
// no time argument for that reason.
void ai_coordinator_fixed_step_00a32d50(AiGroupThinkHost& host);

// 00A2E720, the composition pass: drain, evict and split, seed, auto-merge,
// proximity-merge and the per-member pass, in that order.
void ai_groups_compose_00a2e720(AiGroupThinkHost& host);

// 00A182C0, the eight-party pass. Returns the number of parties whose brain ran
// a think on this call.
int ai_parties_think_00a182c0(AiGroupThinkHost& host);

// 00A181A0, one party brain's think.
void ai_party_brain_think_00a181a0(AiGroupThinkHost& host, void* brain);

} // namespace bsp
