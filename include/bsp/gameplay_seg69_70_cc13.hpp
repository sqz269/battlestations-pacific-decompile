#pragma once

// Segments 69 (009FE130-00A16030) and 70 (00A16050-00A371A0) reference models:
// the AI party brain, its commands and planners. Packet cc13-gpr69;
// docs/CC13_GPR_69.md, docs/CC13_GP_009F_00A3.md (the naming pass).
//
// These are semantic interfaces read from the disk bytes (Capstone disasm-raw,
// x87 read from the opcode bytes) and the Ghidra pseudocode, not native layouts
// or ABI-compatible replacements. Field names are hypotheses; the "_NNN" suffix
// is the native byte offset. Every engine service the image calls is a host
// callback, so nothing here invents a global or stubs a call. Gameplay runs at
// 24-bit x87 precision (docs/CC13_FPU_PRECISION.md), so the models compute in
// float where the image rounds to float. Build-tested only; not wired, not
// game-run. cc9 owns every host these would sit in.
//
// ADOPTION NOTE (cc9 terms; the full table is in docs/CC13_GPR_69.md section 2):
// - Power-up use (00A179E0 and its scorers 00A046C0, 00A04860, 00A04910,
//   00A0F680). Replaces the record-only stub AiParties::party_brain_plan_tail
//   (src/game_hosts_ai.cpp:5078). Inputs: the party's ready power-up entries
//   (008EA0C0, empty unless the byte 00E0C978 is set), the own and enemy team
//   group lists (00F8AA48 + team*0Ch), each group's population +5644h, party
//   +5634h, first-member pose (+FCh/+104h), strength 00A2C4C0, and the
//   power-up class (+8h kind, +84h scope, +8Ch sixteen multipliers). RNG: every
//   00A0F680 call draws ONE stream-1 uniform U(tuning+50h, tuning+54h)
//   (00A0F737), in the image's order: per entry, per pair, only after the scope
//   test accepts. No stream-0 draw. OFF: nothing changes. ON (with a power-up
//   manager bound): an AI party that holds a ready power-up and has at least one
//   engagement pair fires its best one each think (008EADA0), and every
//   air-support entry shifts stream 1 by one draw per accepted pair.
// - Further models are listed in the doc's adoption table.

#include <array>
#include <cstdint>
#include <optional>
#include <utility>
#include <vector>

namespace bsp {
namespace gameplay_seg69_70_cc13 {

// ---------------------------------------------------------------------------
// The power-up class fields the AI reads (docs/GAMEPLAY_MODIFIERS.md).
// ---------------------------------------------------------------------------

inline constexpr int kPowerupMultiplierCount = 16;

struct PowerupClassView {
    const void* id{nullptr};      // the class object, handed back to callbacks
    int kind_008{0};              // 1: air support (a flight); 2: multipliers
    int scope_084{0};             // 1 own units, 2 enemy units, 4 party, 5 team
    std::array<float, kPowerupMultiplierCount> multipliers_08c{};  // 1.0 = none
};

// ---------------------------------------------------------------------------
// 00A046C0 BSP_AiPowerup_ScoreForUnit, __fastcall(unit, class) -> float (ST0).
// ---------------------------------------------------------------------------

// The unit queries 00A046C0 makes. All are side-effect-free reads; the image
// evaluates each only in its own case, the model takes them eagerly.
struct PowerupUnitInputs {
    float health{1.0f};               // 00923BE0(unit), cases 2 and 3
    bool torpedo_proof_small{false};  // 00828EC0([unit+538h]), cases 4 and 5
    bool is_kind_06{false};           // vtable+5Ch(6), case 10
    float class_804{0.0f};            // [unit+538h]+804h, case 10 (ships only)
    bool is_kind_1c{false};           // vtable+5Ch(1Ch), case 11
    float hint_multiplier{1.0f};      // 00A04240(unit)
    float class_weight{1.0f};         // 009FDF30([unit+C4h])
};

// One multiplier's term before the case rule: 1 - m when scope is 2 (enemy
// units), m - 1 otherwise; m == 1.0f exactly contributes nothing (LAHF/TEST
// AH,44h/JNP, so a NaN multiplier is processed).
float ai_powerup_multiplier_term_00a046c0(const PowerupClassView& cls, int index,
                                          const PowerupUnitInputs& unit) noexcept;
float ai_powerup_score_for_unit_00a046c0(const PowerupClassView& cls,
                                         const PowerupUnitInputs& unit) noexcept;

// ---------------------------------------------------------------------------
// 00A04860 BSP_AiPowerup_ScoreForSide / 00A04910 BSP_AiPowerup_ScoreForTeam.
// Both walk the world list [[00E188A8]+19CCh]+34h (node +4 next, +8 unit).
// ---------------------------------------------------------------------------

struct PowerupWorldUnit {
    const void* unit{nullptr};
    int side_180{0};   // unit+180h
    int team_054{0};   // unit+54h
};

// The party 00A04893..00A048C8 derives inline: side 8 -> -1; side <= 7
// (unsigned) -> side; otherwise -1 on a multiplayer session (game+1FE4h != 0)
// and else 4 when unit+54h differs from the local player's team
// ([game+18CCh + 4*[game+18ECh]]+28h), 0 when it matches.
int ai_unit_party_00a04893(int side_180, int team_054, std::uint32_t game_1fe4,
                           int local_player_team) noexcept;

class PowerupScoreHost {
public:
    virtual ~PowerupScoreHost() = default;
    // 008E35F0 BSP_PowerupClass_ScopeAcceptsUnit(unit, class).
    virtual bool scope_accepts(const void* unit, const PowerupClassView& cls) = 0;
    // 00A046C0 for that unit (ai_powerup_score_for_unit_00a046c0 over the
    // unit's queries).
    virtual float unit_score(const void* unit, const PowerupClassView& cls) = 0;
    // 00A0F680 BSP_AiPartyBrain_CandidateAttackValue(enemy group, class); draws
    // stream 1 once (see ai_candidate_attack_value_00a0f680).
    virtual float candidate_attack_value(const void* enemy_group, const PowerupClassView& cls) = 0;
};

// sum over units whose derived party == party and the scope accepts, * 0.25
// ([00D7A348]); the sum and the product are rounded to float.
float ai_powerup_score_for_side_00a04860(int party, const PowerupClassView& cls,
                                         const std::vector<PowerupWorldUnit>& world,
                                         std::uint32_t game_1fe4, int local_player_team,
                                         PowerupScoreHost& host);
// sum over units with unit+54h == team and the scope accepts, * 0.125 ([00D04380]).
float ai_powerup_score_for_team_00a04910(int team, const PowerupClassView& cls,
                                         const std::vector<PowerupWorldUnit>& world,
                                         PowerupScoreHost& host);

// ---------------------------------------------------------------------------
// 00A0F680 BSP_AiPartyBrain_CandidateAttackValue, __fastcall(group, class).
// ---------------------------------------------------------------------------

// The flight block the class carries for this group: class+0Ch when the
// group's +5638h is non-zero, class+44h when it is zero (00A0F6A7 SETZ, *38h).
struct CandidateAttackBlock {
    const void* class_object_020{nullptr};   // block+20h: vtable+1Ch gives the class id
    std::uint32_t dword_01c{0};              // block+1Ch, copied to record+10h
};

// The one attacker record 00A0F6EF..00A0F73C builds (20h bytes, the layout of
// 00A04560's records: src/game_hosts_ai.cpp GroupValueRecord).
struct AiValueRecord {
    const void* object_000{nullptr};   // block+20h
    float x_004{0.0f};                 // 0.0
    float z_008{0.0f};                 // 0.0
    bool byte_00c{false};              // 0
    std::uint32_t dword_010{0};        // block+1Ch
    float weight_014{0.0f};            // 009FDF30(class id); no 00A04240 factor
    float spread_018{0.0f};            // U(tuning+50h, tuning+54h) on STREAM 1
    std::uint32_t dword_01c{0};        // 0 (00A04560 keeps its third-party byte here)
};

class CandidateAttackHost {
public:
    virtual ~CandidateAttackHost() = default;
    // [[block+20h]]+1Ch() then 009FDF30 BSP_Ai_ClassWeightForClassId.
    virtual float class_weight(const void* class_object) = 0;
    // 00A371A0 BSP_Ai_ActiveModeTuning +50h / +54h (ValueRandomMul [1], [2]).
    virtual float tuning_spread_lo_050() = 0;
    virtual float tuning_spread_hi_054() = 0;
    // 00BD2F10 BSP_Random_UniformFloatRange(stream, lo, hi); stream is 1 here.
    virtual float uniform(int stream, float lo, float hi) = 0;
    // 00A07E40(group, &defenders, 0) then 00A0C650(&{record}, &defenders, 0,
    // -1.0 [00D7A260], 0, 0, 1.0) (host: compose_attack_value_00a0c650).
    virtual float compose_against_group(const AiValueRecord& attacker,
                                        const void* group) = 0;
};

AiValueRecord ai_candidate_attack_record_00a0f680(const CandidateAttackBlock& block,
                                                          CandidateAttackHost& host);
float ai_candidate_attack_value_00a0f680(std::uint32_t group_5638,
                                         const CandidateAttackBlock& block_00c,
                                         const CandidateAttackBlock& block_044,
                                         const void* group, CandidateAttackHost& host);

// ---------------------------------------------------------------------------
// 00A179E0 BSP_AiPartyBrain_EngagementPass, __fastcall(brain): power-up use.
// ---------------------------------------------------------------------------

struct EngagementGroup {
    const void* id{nullptr};
    std::uint32_t population_5644{0};
    int party_5634{0};
    float leader_x{0.0f};          // first member +FCh (after 00414DB0 refresh)
    float leader_z{0.0f};          // first member +104h
    float strength{0.0f};          // 00A2C4C0 BSP_AiGroup_TotalLeaderWeight
    std::vector<const void*> members;   // the +563Ch list, in order
};

// The 1Ch-byte pair record 00A17880 appends.
struct EngagementPair {
    const EngagementGroup* own{nullptr};     // +0h
    const EngagementGroup* enemy{nullptr};   // +4h
    float dist2_00c{0.0f};                   // +0Ch, planar squared distance
    float own_strength_010{0.0f};            // +10h
    float enemy_strength_014{0.0f};          // +14h
    float weight_018{0.0f};                  // +18h, enemy strength / max(500^2, d^2)
};

// 00A17A34..00A17D1B. own_team is the snapshot of 00F8AA48 + brain+24h * 0Ch,
// enemy_team that of (brain+24h == 0) * 0Ch, both in list order (own-major).
std::vector<EngagementPair> ai_engagement_pairs_00a179e0(
    int party, const std::vector<EngagementGroup>& own_team,
    const std::vector<EngagementGroup>& enemy_team);

struct PowerupReadyEntry {
    const void* entry{nullptr};   // the vector element handed to 008EADA0
    PowerupClassView cls;         // [entry+4]
};

struct PowerupUseDecision {
    const void* entry{nullptr};
    const void* target{nullptr};   // a unit, or null for scopes 4 and 5
    float score{0.0f};
};

// 00A17D5A..00A180A8: the choice; nullopt when no entry scores above 0.0.
// party is brain+20h, team brain+24h. Callbacks run in image order.
std::optional<PowerupUseDecision> ai_powerup_choose_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementPair>& pairs, const std::vector<PowerupWorldUnit>& world,
    std::uint32_t game_1fe4, int local_player_team, PowerupScoreHost& host);

// The whole pass: empty entries or no pair -> nullopt (00A17A3A, 00A17D41); the
// caller fires 008EADA0(manager [00F88C30], entry, party, target) on a result.
std::optional<PowerupUseDecision> ai_party_brain_engagement_pass_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementGroup>& own_team, const std::vector<EngagementGroup>& enemy_team,
    const std::vector<PowerupWorldUnit>& world, std::uint32_t game_1fe4, int local_player_team,
    PowerupScoreHost& host);

// ---------------------------------------------------------------------------
// 00A163D0 BSP_Ai_InitPartyThinkSchedule (from the controller constructor
// 00A31730, which 00A32350 BSP_AiController_Create calls).
// ---------------------------------------------------------------------------

struct PartyThinkSlotInit {
    bool ai_enabled_000{false};       // 00F8A8C8 + p*1Ch, +0h = 0
    float attack_ratio_004{0.5f};     // +4h = 0.5 [00CE3800]
    float aggressive_ratio_008{0.5f}; // +8h = 0.5
    float next_think{0.0f};           // 00F8A87C + p*4 = now + U(3, 5)
    const void* brain{nullptr};       // 00F8A89C + p*4 = 0
};

class ThinkScheduleRng {
public:
    virtual ~ThinkScheduleRng() = default;
    // 00BD2F10(stream, lo, hi).
    virtual float uniform(int stream, float lo, float hi) = 0;
};

// Eight slots in party order; each draws U(3.0 [00CE3854], 5.0 [00CE3850]) on
// STREAM 1 and adds the float clock [00F876A4] (re-read per slot; constant
// within the call).
std::array<PartyThinkSlotInit, 8> ai_init_party_think_schedule_00a163d0(float now,
                                                                        ThinkScheduleRng& rng);

// ---------------------------------------------------------------------------
// Speeds: 00A02180 (named BSP_UnitList_MaxSpeed) and 00A07C10 (named
// BSP_AiGroupRecords_MaxSpeed) return the MINIMUM: the seed is 1e10 [00CE4970]
// and a value replaces it unless it is greater (FCOMI/JA, so NaN replaces).
// ---------------------------------------------------------------------------

struct SpeedClassView {
    bool is_kind_06{false};   // vtable+18h(6): a ship class
    float speed_500{0.0f};    // +500h
    bool is_kind_0f{false};   // vtable+18h(0Fh): a plane class
    float speed_188{0.0f};    // +188h
};
float ai_class_speed_00a02180(const SpeedClassView& c) noexcept;   // 0 for neither kind
float ai_unit_list_min_speed_00a02180(const std::vector<SpeedClassView>& list) noexcept;

// ---------------------------------------------------------------------------
// 00A03900 BSP_AiTuning_FindClassPairRow, __fastcall(class a, class b).
// ---------------------------------------------------------------------------

// The 0Ch-byte rows of [004C1C50() + mode*10h + FACh .. +FB0h].
struct ClassPairRow {
    std::uint32_t class_id_a{0};
    std::uint32_t class_id_b{0};
    float value{0.0f};
};
// mode_index is 009FFC80; above 2 the answer is 0.0. The first row with a == the
// first class's id and b == the second's (vtable+1Ch) gives the value; else 0.0.
float ai_class_pair_value_00a03900(int mode_index, const std::vector<ClassPairRow>& rows_of_mode,
                                   std::uint32_t class_id_a, std::uint32_t class_id_b) noexcept;

// ---------------------------------------------------------------------------
// 00A0D1D0 BSP_AiGroup_MatchFinalWeight: the greedy force composition.
// __fastcall(ECX = composition out, EDX = threat records, classes, budget,
// target, pos, byte5, flag, arg7), RET 1Ch, returns ST0.
// ---------------------------------------------------------------------------

struct SpawnClassView {
    const void* id{nullptr};
    std::uint32_t class_id{0};   // vtable+1Ch
    int cost_124{0};             // +124h (an int: FILD / CVTSI2SS)
    std::uint32_t dword_134{0};  // +134h, copied to record+10h
    bool is_plane{false};        // vtable+18h(0Fh)
    SpeedClassView speed;        // for 00A07C10 over a composition
};

class SpawnCompositionHost {
public:
    virtual ~SpawnCompositionHost() = default;
    // 009FDF30 BSP_Ai_ClassWeightForClassId.
    virtual float class_weight(std::uint32_t class_id) = 0;
    // 00A371A0 BSP_Ai_ActiveModeTuning fields.
    virtual float tuning_050() = 0;
    virtual float tuning_054() = 0;
    virtual float tuning_100() = 0;
    virtual float tuning_104() = 0;
    virtual float tuning_108() = 0;
    virtual float tuning_10c() = 0;
    virtual float tuning_230() = 0;
    virtual float tuning_234() = 0;
    virtual float tuning_238() = 0;
    // 00BD2F10(stream, lo, hi); stream 1 here.
    virtual float uniform(int stream, float lo, float hi) = 0;
    // 00A0C650(ECX = composition, EDX = threats, 1, arg7, debug buffer, byte5,
    // factor): the composition attacking the threats.
    virtual float compose_against_threats(const std::vector<AiValueRecord>& composition,
                                          float arg7, bool byte5, float factor) = 0;
    // 00A0C650(ECX = threats, EDX = composition, 0, 100.0 [00CE3D08], 0, 0, 1.0):
    // the threats attacking the composition.
    virtual float compose_threats_against(const std::vector<AiValueRecord>& composition) = 0;
    // 00A03900 over the two classes (ai_class_pair_value_00a03900).
    virtual float class_pair_value(const SpawnClassView& a, const SpawnClassView& b) = 0;
};

struct MatchFinalWeightInputs {
    bool mode_le_3{true};        // 004BCA50() <= 3 (SETBE)
    float budget{0.0f};          // stack arg 2: 00942130(party)
    float target{0.0f};          // stack arg 3: the cost to buy
    float pos_x{0.0f};           // stack arg 4 -> +0h
    float pos_z{0.0f};           //             -> +8h
    bool byte5{false};           // stack arg 5 (0 from 00A23980)
    float factor_e0e35c{1.0f};   // [00E0E35C] (1.0 on disk) when byte5, else 1.0
    bool allow_mixed{false};     // stack arg 6: skips the plane/surface match
    float arg7{-1.0f};           // stack arg 7 (-1.0 from 00A23980)
};

struct MatchFinalWeightResult {
    std::vector<AiValueRecord> composition;   // ECX's vector, appended in pick order
    float value{0.0f};                        // the last accepted pick's total
};

MatchFinalWeightResult ai_match_final_weight_00a0d1d0(const std::vector<SpawnClassView>& classes,
                                                      const MatchFinalWeightInputs& in,
                                                      SpawnCompositionHost& host);

// ---------------------------------------------------------------------------
// 00A236F0 BSP_AiPlanner_SortSpawnPointsByDistance and 00A23980
// BSP_AiPlanner_ChooseSpawnComposition, 00A25A30 BSP_AiPlanner_TrySpawnGroup.
// ---------------------------------------------------------------------------

struct SpawnSiteRef {
    const void* id{nullptr};
    float x_0fc{0.0f};   // world pose after the 00414DB0-style refresh
    float z_104{0.0f};
};

// 0066E590's list of lists (per command building, its adopted sites), walked in
// order, flattened and ordered by planar squared distance from (x, z). The image
// uses std::sort (introsort, insertion sort at <= 32 entries); ties keep list
// order only on that small path. This model uses the insertion sort throughout,
// which matches the image for distinct distances and up to 32 entries.
std::vector<const SpawnSiteRef*> ai_sort_spawn_sites_00a236f0(
    const std::vector<std::vector<SpawnSiteRef>>& lists, float x, float z);

struct SpawnSite {
    const SpawnSiteRef* ref{nullptr};
    std::vector<SpawnClassView> classes;   // 0084D560(site)(&list, brain+24h, 0) then
                                           // 00964790 GetOrCreate(name, 1) per entry
};

class SpawnChoiceHost : public SpawnCompositionHost {
public:
    // 00942130(brain+20h), read per site.
    virtual float budget() = 0;
    // 004BCA50, read per site; the arm for modes >= 4 (00A07D40, 00A06260) is
    // not modelled: it is the host's answer.
    virtual std::uint32_t effective_game_mode() = 0;
    // In that arm the image multiplies by [ESP+34h], which there still holds the
    // last GetOrCreate pointer's bits; the host supplies both the records and the score.
    virtual MatchFinalWeightResult mode4_composition(const SpawnSite& site, float budget) = 0;
    // 00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x).
    virtual float interpolate_clamped(float x0, float y0, float x1, float y1, float x) = 0;
    // 0084CF80 BSP_SupportSite_SquadSizeForClass(site, class).
    virtual std::uint32_t squad_size(const SpawnSiteRef& site, const void* class_object) = 0;
};

struct SpawnRequest {
    const SpawnSiteRef* site{nullptr};
    std::vector<const void*> classes;      // record+0h
    std::vector<std::uint32_t> values;     // record+10h (class+134h)
    std::vector<std::uint32_t> squads;     // 0084CF80 (overwrites the dead 1/3 kind)
    std::vector<AiValueRecord> composition;
    float weight{0.0f};
};

// nullopt: no site produced a composition (the image returns 0). Otherwise the
// caller sends 0094C830(ECX [00F89B3C], party, &classes, site, [slot[party]]+2Ch,
// &values, &squads, 0, flag) and the image returns 1.
std::optional<SpawnRequest> ai_choose_spawn_composition_00a23980(
    const float* pos_xz /* nullable: arg 1 */, const std::vector<SpawnSite>& sorted_sites,
    float weight, bool allow_mixed, SpawnChoiceHost& host);

// 00A25A30: false when weight < 0.5 [00CE3800] (COMISS/JBE: NaN continues), when
// the threat records are empty, or when 0066E590 yields no site.
bool ai_try_spawn_group_gate_00a25a30(float weight, std::size_t threat_records,
                                      std::size_t sources) noexcept;

// ---------------------------------------------------------------------------
// The RETREAT command: 00A01230 BSP_Ai_NetDangerAtPoint, 00A11530
// BSP_AiCommand_RetreatPickDirection, 00A156E0 BSP_AiCommand_Retreat_Tick.
// ---------------------------------------------------------------------------

class RetreatHost {
public:
    virtual ~RetreatHost() = default;
    // 00A010F0 __fastcall(point, team) -> ST0 (host:
    // AiCommandTickHost::tick_danger_cost_00a010f0).
    virtual float danger_cost(const float point[3], int team) = 0;
    // The first member's +FCh (after 00414DB0), or 00F87574 when +5644h is 0.
    virtual void leader_point(float out[3]) = 0;
    // 00A156E7 00A10DC0 (follower pass), 00A156EE 00A11690 (formation wedge),
    // 00A15742 00A13B60(0.75 [00CEE07C], leader point, 0, 0).
    virtual void retreat_prelude() = 0;
    // 00A158F5 00A02020(ECX = the first member of +563Ch, EDX = &point).
    virtual void issue_move_first_member(const float point[3]) = 0;
};

// float(00A010F0(p, team) - 00A010F0(p, team == 0)) (FSUBR qword).
float ai_net_danger_at_point_00a01230(const float point[3], int team, RetreatHost& host);

// The ring of 00A113D0 (750 [00D22C70] around the leader, pi/6 steps while
// 2pi [00CE3828] > a, a stored as float: 13 samples) scored by 00A01230 with
// team = (group+5638h == 0); the strict best above -1e10 [00CE4ADC]. nullopt
// when no sample beats the seed: the image then leaves the caller's out
// unwritten (uninitialised stack in 00A156E0).
std::optional<std::array<float, 3>> ai_retreat_pick_direction_00a11530(int team, RetreatHost& host);

// 00A156E0 after its prelude: d = 00A11530; when
// float(00A010F0(leader + d, team) - 8.0 [00CE3DB0]) > 00A010F0(leader - d, team)
// (FCOMI/JBE), the first member is ordered to leader - d. Returns whether it was.
bool ai_retreat_tick_00a156e0(std::uint32_t group_5638, RetreatHost& host);

// ---------------------------------------------------------------------------
// 00A07D40 BSP_Ai_CollectTeamEntityRecords, __fastcall(team, &records;
// exclude filter, exclude_static), RET 8; and its caller 00A24870
// BSP_AiPlanner_CollectThreatRecordsNear, __thiscall(planner)(entity, &records).
// ---------------------------------------------------------------------------

struct TeamEntityView {
    const void* id{nullptr};
    bool byte_05d{false};          // +5Dh: set -> skipped
    int team_054{0};               // +54h
    std::array<float, 3> pos{};    // +FCh..+104h (after 00414DB0)
    bool kind_06{false};           // vtable+5Ch(6), ships
    bool kind_18{false};           // (18h)
    bool kind_1b{false};           // (1Bh), forts
    bool kind_45{false};           // (45h), airfields
    bool kind_46{false};           // (46h), shipyards
};

// The world list [[00E188A8]+19CCh]+34h in order. An entity is kept when +5Dh
// is clear, +54h == team, the filter (vtable slot 0 on &pos; null skips the
// test) answers false, it is kind 6, 18h or 1Bh, and, with exclude_static,
// not kind 1Bh, 45h or 46h. Each kept entity becomes 00A04560(entity, 1).
std::vector<const TeamEntityView*> ai_collect_team_entities_00a07d40(
    int team, const std::vector<TeamEntityView>& world, const std::array<float, 3>* sphere_centre,
    float sphere_radius_squared, bool exclude_static);

// 00A24870: the filter is vtable 00D22CC0 (slot 0 = 00A1A660, outside the
// sphere) centred on the entity with r^2 = float(tuning+1A4h squared); team is
// planner+34h; exclude_static 0. With nothing collected, the entity itself is
// the one record (00A248FD).
std::vector<const TeamEntityView*> ai_collect_threats_near_00a24870(
    int planner_team_034, const TeamEntityView& entity, float tuning_1a4,
    const std::vector<TeamEntityView>& world);

// ---------------------------------------------------------------------------
// The planners' spawn arms: Capture 00A2B477..00A2B7A6 (in 00A29FD0) and Defend
// 00A29BE7..00A29E2A (in 00A28A60); 00A1C900 (tagged STL_inst_00a1c900) is the
// planner's owned resource.
// ---------------------------------------------------------------------------

// 00A1C900, __thiscall(planner): the sum, in list order, of 00A2C530
// (BSP_AiGroup_ResourceSum) over the planner's owned groups (list +20h).
float ai_planner_owned_resource_00a1c900(const std::vector<float>& group_resources) noexcept;

struct PlannerSpawnBudgetInputs {
    float queued_946970{0.0f};     // 00946970([00F89B3C], brain+20h): queued spawn requests
    float defend_pct{0.0f};        // [00F8A8BC + 4*009FFC80()]
    float budget_942130{0.0f};     // 00942130(brain+20h)
    float owned_resource{0.0f};    // 00A1C900(planner)
};

// Capture: float(float((1 - pct) * budget) - owned) (DE E1 at 00A2B4BA).
float ai_capture_spawn_budget_00a2b4a6(const PlannerSpawnBudgetInputs& in) noexcept;
// Defend: float(float(pct * budget) - owned).
float ai_defend_spawn_budget_00a29c33(const PlannerSpawnBudgetInputs& in) noexcept;

// A Capture plan target (the tree node): +0Ch the entity, +14h the score, +1Ch
// the cap, +2Ch the value that must be positive.
struct CaptureSpawnTarget {
    const void* entity{nullptr};
    float score_014{0.0f};
    float cap_01c{0.0f};
    float value_02c{0.0f};
};

struct PlannerSpawnCall {
    const void* entity{nullptr};   // pos = entity+FCh; tag "[capture]"/"[defend]" + entity+174h
    float amount{0.0f};            // the weight handed to 00A25A30
    int flag{0};                   // 00A25A30 arg 5: 0 for Capture, 1 for Defend
    std::size_t index{0};          // the chosen target or record
};

// 00A2B477: nullopt when 00946970 > 0 (FCOMI/JA; NaN passes), when the ramp
// byte [ESP+4Fh] (host ai_tail_capture_spawn_due) is clear, when 1.0 > budget,
// or when no target has +2Ch > 0 (COMISS/JBE). The target is the strict best
// +14h above -1e10 [00CE4ADC]; amount = +1Ch unless it is above the budget
// (FCOMI/JBE: NaN keeps +1Ch). The caller then runs 00A24870 for the entity
// and 00A25A30(entity+FCh, amount, &threats, tag, 0); a true answer stores
// planner+3Ch = 0.0.
std::optional<PlannerSpawnCall> ai_capture_spawn_arm_00a2b477(
    const PlannerSpawnBudgetInputs& in, bool ramp_due, const std::vector<CaptureSpawnTarget>& targets);

// A Defend record (34h bytes): +0h the entity, +20h its threat records, +30h
// the remainder.
struct DefendSpawnRecord {
    const void* entity{nullptr};
    float remaining_030{0.0f};
};

// 00A29BE7: nullopt when 00946970 > 0, when 1.0 > budget, or when no record
// has +30h > 0; otherwise the FIRST such record, amount = min as in Capture,
// and the caller runs 00A25A30(entity+FCh, amount, &record+20h, tag, 1). No
// ramp gate and no +3Ch reset in this arm.
std::optional<PlannerSpawnCall> ai_defend_spawn_arm_00a29be7(
    const PlannerSpawnBudgetInputs& in, const std::vector<DefendSpawnRecord>& records);

// ---------------------------------------------------------------------------
// Small group and planner helpers
// ---------------------------------------------------------------------------

// 00A2C530 BSP_AiGroup_ResourceSum, __thiscall(group): the float sum of
// member+304h over the +563Ch list, in order (FLD/FADD/FSTP float).
float ai_group_resource_sum_00a2c530(const std::vector<float>& member_304) noexcept;

// 00A18480 BSP_AiPlanner_TakeReplanFlag: returns planner+2Ch and clears it.
bool ai_planner_take_replan_flag_00a18480(bool& replan_02c) noexcept;

// 00A1D110 / 00A1D140 / 00A1D170, the Duel, Escort and Siege needs-replan slot
// (vtable +30h): takes and clears +2Ch; with owned groups (+28h, unsigned) it
// answers true when the first owned group's command is not engaged (00A2CB10)
// and the old flag otherwise; with none, the old flag.
bool ai_mode_planner_needs_replan_00a1d110(bool& replan_02c, std::uint32_t owned_028,
                                           bool first_owned_engaged) noexcept;

// 00A2E1E0 BSP_AiGroup_CountUnitTypes, __thiscall(group)(map&) RET 4: for each
// member, map[member+170h vtable slot 0 ()] += 009FE060(member) (00625900 is the
// map's operator[]).
struct CountedMember {
    std::uint32_t type_key{0};   // [member+170h]->vtable[0]()
    int kind18_count{0};         // 009FE060 BSP_Unit_Kind18Count(member)
};
void ai_group_count_unit_types_00a2e1e0(const std::vector<CountedMember>& members,
                                        std::vector<std::pair<std::uint32_t, int>>& map_in_out);

// 00A1A6D0 BSP_AiPlanner_OrderDefendGroup, (group, entity) RET 8: when
// 00A2C5A0 (BSP_AiGroup_HasGroupableCombatant) answers true the group is ordered
// to patrol to entity+FCh (00A2C310), otherwise to defend its position (00A2BE20).
enum class DefendOrder { PatrolToEntity, DefendPosition };
DefendOrder ai_order_defend_group_00a1a6d0(bool has_groupable_combatant) noexcept;

// 00A04530 BSP_Ai_HintedClassWeight, __thiscall(unit):
// float(00A04240(unit) * double 009FDF30([unit+C4h])).
float ai_hinted_class_weight_00a04530(float hint_multiplier, float class_weight) noexcept;

}  // namespace gameplay_seg69_70_cc13
}  // namespace bsp
