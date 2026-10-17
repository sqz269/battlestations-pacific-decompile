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

// The power-up use the pass itself performs at 00A180C9.
class PowerupUseHost {
public:
    virtual ~PowerupUseHost() = default;
    // 008EADA0 BSP_PowerUpManager_UseItem, __thiscall(manager [00F88C30],
    // entry, party = brain+20h, target) RET 0Ch (reconstruction:
    // powerup_seg58_argon power_up_manager_use_item_008eada0).
    virtual void use_item(const void* entry, int party, const void* target) = 0;
};

// The whole pass, void like the image: empty entries (00A17A3A, 00A17A49) or no
// pair (00A17D41) return without a call; otherwise, when an entry scored above
// 0.0 (00A180B3 tests the chosen entry pointer), it calls
// use.use_item(entry, party, target) once, after the scoring loop.
void ai_party_brain_engagement_pass_00a179e0(
    int party, int team, const std::vector<PowerupReadyEntry>& entries,
    const std::vector<EngagementGroup>& own_team, const std::vector<EngagementGroup>& enemy_team,
    const std::vector<PowerupWorldUnit>& world, std::uint32_t game_1fe4, int local_player_team,
    PowerupScoreHost& host, PowerupUseHost& use);

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

class DangerHost {
public:
    virtual ~DangerHost() = default;
    // 00A010F0 __fastcall(point, team) -> ST0 (host:
    // AiCommandTickHost::tick_danger_cost_00a010f0).
    virtual float danger_cost(const float point[3], int team) = 0;
    // The first member's +FCh (after 00414DB0), or 00F87574 when +5644h is 0;
    // 00A10C20 is the same read.
    virtual void leader_point(float out[3]) = 0;
};

class RetreatHost : public DangerHost {
public:
    // 00A156E7 00A10DC0 (follower pass), 00A156EE 00A11690 (formation wedge),
    // 00A15742 00A13B60(0.75 [00CEE07C], leader point, 0, 0).
    virtual void retreat_prelude() = 0;
    // 00A158F5 00A02020(ECX = the first member of +563Ch, EDX = &point).
    virtual void issue_move_first_member(const float point[3]) = 0;
};

// float(00A010F0(p, team) - 00A010F0(p, team == 0)) (FSUBR qword).
float ai_net_danger_at_point_00a01230(const float point[3], int team, DangerHost& host);

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

// 00A2C9F0 BSP_AiGroup_ShouldRouteCautiously, __thiscall(group)(point) RET 4,
// the gate of 00A2CBD0's cautious arm: false for group+5638h >= 2 (signed JL)
// and when float(tuning+1F0h)^2 > the planar d^2 from the leader to the point
// (FCOMI/JA); otherwise 00A01230 at the midpoint (leader + point) * 0.5
// (DC C9 / D8 C9 / FMUL m32) with team = (+5638h == 0), and true when
// 0.0 < it (FCOMI/JB: a NaN answers true).
bool ai_should_route_cautiously_00a2c9f0(std::int32_t group_5638, float tuning_1f0,
                                         const float point[3], DangerHost& host);

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

// ---------------------------------------------------------------------------
// 00A32DF0 BSP_AiController_InitPartySlots: slot 0 of the controller entity's
// vtable 00D23240 (its activation). It also zeroes +1A4h, runs 00923840 and
// names the entity "HighLvlAI" (00D232B8, 9 bytes into the string at +154h).
// ---------------------------------------------------------------------------

// The player slot record [game+18CCh + 4*slot], bytes +8h..+0Ah.
struct PlayerSlotFlags {
    bool present_008{false};
    bool flag_009{false};      // set on the AI-capable slots (with +8h, it enables the party)
    bool elected_00a{false};   // written here in modes 0..3
};

struct PartyInitResult {
    std::array<bool, 8> ai_enabled{};     // 00F8A8C8 + p*1Ch, +0h
    std::array<bool, 8> ratios_reset{};   // +4h and +8h were set to 0.5 [00CE3800]
};

// In effective game modes 0..3 (004BCA50, unsigned JA): for each half of the
// slots (0..3, then 4..7), when any slot of the half has +8h set and +9h clear,
// every +0Ah of the half is cleared; otherwise the half's first slot gets
// +0Ah = 1 and the others 0. Then each party record's +0h = +8h && +9h && +0Ah,
// and an enabled record's +4h/+8h = 0.5. [00F8AB6A] = 0; when [game+61Ch] is
// set, record 0 is enabled (00A32F80) regardless.
PartyInitResult ai_controller_init_party_slots_00a32df0(std::uint32_t effective_game_mode,
                                                         std::array<PlayerSlotFlags, 8>& slots,
                                                         bool game_61c) noexcept;

// ===========================================================================
// Packet cc13-gpr69b (docs/CC13_GPR_69.md section 7).
// ===========================================================================

// ---------------------------------------------------------------------------
// 00A0C330 BSP_AiEntityRecord_PairBaseValue, 00A0C3C0
// BSP_AiEntityRecord_PairAttackValue and 00A0C650 BSP_AiGroup_ComposeAttackValue
// with every argument. The host's compose_attack_value_00a0c650 /
// group_value_pair_00a0c3c0 (src/game_hosts_ai.cpp:5375, :5229) are the
// specialisation (0, -1.0, no debug, 0, 1.0); 00A23980 also calls
// (1, arg7, debug, byte5, factor) and (0, 100.0, 0, 0, 1.0).
// ---------------------------------------------------------------------------

class ComposeAttackHost {
public:
    virtual ~ComposeAttackHost() = default;
    // 00A08460(ECX = a+0h, EDX = a+10h, t+0h, t+1Ch) -> ST0.
    virtual float target_weight_00a08460(const AiValueRecord& attacker,
                                         const AiValueRecord& target) = 0;
    // [a+0h] vtable+18h(code): 6 ship class, 0Fh plane class, 1Ch (00A0C35B).
    virtual bool class_is_kind(const AiValueRecord& record, int code) = 0;
    // [a+0h]+500h for a ship class, +188h for a plane class (the FDIV operands).
    virtual float class_speed_500(const AiValueRecord& record) = 0;
    virtual float class_speed_188(const AiValueRecord& record) = 0;
    // 00A371A0 BSP_Ai_ActiveModeTuning + offset.
    virtual float tuning(std::uint32_t offset) = 0;
    // 00A07C10 over the attacker list (ai_unit_list_min_speed_00a02180 over the
    // records' classes); only its product with tuning +22Ch is read.
    virtual float records_min_speed_00a07c10(const std::vector<AiValueRecord>& attackers) = 0;
    // 00419010(x0, y0, x1, y1, x) (bsp::clamped_interpolate_00419010).
    virtual float interpolate_00419010(float x0, float y0, float x1, float y1, float x) = 0;
};

// 00A0C330 (ECX = attacker, EDX = target, debug, rnd flag), RET 8:
// w = 00A08460; w = 0 when a+1Ch != 0 and the class answers 1Ch; then
// f(f(rnd * w) * t+14h) with rnd = f(a+18h * t+18h) when the flag is set, else 1.0.
float ai_pair_base_value_00a0c330(const AiValueRecord& attacker, const AiValueRecord& target,
                                  bool rnd_flag, ComposeAttackHost& host);

// 00A0C3C0 (ECX = attacker, EDX = target, arg1 (unread), distance, debug, rnd
// flag), RET 10h. A negative distance is replaced by 00414C60 over the pose
// delta when both +0Ch bytes are set, else by 0. A ship class (6, tried first)
// uses +D8h and 00419010(+DCh, +E8h, +E0h, +E4h, time) over class +500h; a plane
// class (0Fh) +ECh and 00419010(+F0h, +FCh, +F4h, +F8h, time) over +188h. The
// multiplier applies only while (distance - arrive) > 0 (FCOMI/JBE, NaN skips).
float ai_pair_attack_value_00a0c3c0(const AiValueRecord& attacker, const AiValueRecord& target,
                                    float distance, bool rnd_flag, ComposeAttackHost& host);

struct ComposeAttackArgs {
    bool penalties{false};     // arg1 (byte): 0 zeroes the three penalties (00A0CE71)
    float distance{-1.0f};     // arg2, passed to every 00A0C3C0
    bool byte5{false};         // arg4: every attacker but the last scores with rnd 0
    float factor{1.0f};        // arg5: those pairs x factor, or x 0.001 [00D7A23C] when
                               // 0.001 [00D7A318] > factor (FCOMPI/JBE: NaN is kept)
};

struct ComposeAttackTerms {
    float attack_sum{0.0f};           // every pair, attacker-major
    float attack_maxes_sum{0.0f};     // each attacker's best (start 0.0)
    float base{0.0f};                 // f((+218h * sum + maxes) / +214h)
    float speed_bonus{0.0f};          // min(+22Ch * 00A07C10, +228h * base); JBE keeps a NaN
    int attackers_without_target{0};  // attacker best <= 0
    int targets_unattacked{0};        // target best <= 0
    int repeat_pairs{0};              // i < j with a[i]+0h == a[j]+0h
    float result{0.0f};
};

// 00A0C650 __fastcall(ECX = attackers, EDX = targets, arg1..arg5), RET 14h.
// result = f(speed + base) - ((+220h*unattacked + +21Ch*idle) + +224h*repeat),
// clamped to 0 when 0 > result (NaN passes).
ComposeAttackTerms ai_compose_attack_value_00a0c650(const std::vector<AiValueRecord>& attackers,
                                                    const std::vector<AiValueRecord>& targets,
                                                    const ComposeAttackArgs& args,
                                                    ComposeAttackHost& host);

// ---------------------------------------------------------------------------
// 00A13B60 BSP_AiCommand_CloseAttackTargetPass, __thiscall(command)(float
// radius, const float* centre, AiGroup* target_group, int), RET 10h; AL is the
// "a member found a candidate" byte [ESP+1Bh]. The host's reconstruction is
// src/ai_close_attack_tick.cpp:71; section 7.2 of the doc lists where it
// differs from this listing-read model.
// ---------------------------------------------------------------------------

// One entity of the five world-registry lists [[00E188A8]+19CCh] + 1Ch + id*0Ch
// the collection walks, in this order: id 6 (+64h, ships), id 24 (+13Ch, plane
// squadrons), id 45h (+358h), id 46h (+364h), id 1Bh (+160h). Poses are the
// refreshed world pose (00414DB0 / the 00413920 copy into +CCh..+108h).
struct CloseAttackEntity {
    const void* id{nullptr};
    int list_slot{0};          // 0..4 in the walk order above
    bool byte_05c{false};      // must be set
    bool byte_05d{false};      // must be clear
    bool byte_05e{false};      // must be clear
    bool byte_060{false};      // must be clear
    int team_054{0};
    float x_0fc{0.0f};
    float z_104{0.0f};
};

struct CloseAttackMember {
    const void* id{nullptr};
    bool squadron_18{false};          // vtable+5Ch(18h)
    bool leader_kind_17{false};       // [+3D0h] vtable+5Ch(17h) (no null test inline)
    bool leader_byte_c24{false};      // [+3D0h]+C24h
    bool ship_06{false};              // vtable+5Ch(6)
    bool lands_troops_538_2c{false};  // [+538h] vtable+2Ch
    float pos[3]{0.0f, 0.0f, 0.0f};   // +FCh, +100h, +104h
    // vtable+114h -> 0071EB60 -> the resolved instance, kept only when it
    // answers vtable+5Ch(2) and its +54h is the enemy team (00A144C8..00A144F1).
    const CloseAttackEntity* current_target{nullptr};
    bool current_target_kind_2{false};
};

class CloseAttackPassHost {
public:
    virtual ~CloseAttackPassHost() = default;
    // 009FFEB0 BSP_AiCommand_SendEmptySquadronHome (may issue the squadron's
    // return order itself); true skips the member.
    virtual bool send_empty_squadron_home_009ffeb0(const void* squadron) = 0;
    // 00A0F810(ECX = member, EDX = candidate) -> ST0.
    virtual float candidate_weight_00a0f810(const void* member, const void* candidate) = 0;
    // 00A2C720(target_group, candidate); asked only with a target group.
    virtual bool target_group_contains_00a2c720(const void* candidate) = 0;
    virtual float tuning(std::uint32_t offset) = 0;   // 00A371A0 + offset
    virtual float interpolate_00419010(float x0, float y0, float x1, float y1, float x) = 0;
    // 0077D600(member, class, {kind 1, candidate +174h}, 1); class 00E08F78
    // (attackmove) for a ship, 00E08EF8 (settarget) otherwise.
    virtual void issue_order_0077d600(const void* member, std::uint32_t command_class,
                                      const void* target) = 0;
    // 00A02020(member, point).
    virtual void issue_moveto_00a02020(const void* member, const float point[3]) = 0;
};

struct CloseAttackPassResult {
    bool candidate_found{false};
    std::vector<const void*> candidates;   // the list after the pass (with appended targets)
};

// group_team_5638 is [command+4]+5638h; population_5644 [command+4]+5644h.
CloseAttackPassResult ai_close_attack_pass_00a13b60(
    int group_team_5638, int population_5644, float radius, const float centre[3],
    bool has_target_group, const std::vector<CloseAttackEntity>& world_lists,
    const std::vector<CloseAttackMember>& members, CloseAttackPassHost& host);

// ---------------------------------------------------------------------------
// The Defend think 00A28A60: its candidate test and its record sort.
// ---------------------------------------------------------------------------

// 00A2DEF0 BSP_AiGroup_CollectMembersInWorldSet(ECX = group; team, list),
// RET 8: a member is a defend candidate when it answers vtable+5Ch(1Ch) OR
// 008DDF90 finds it in the set [[00E188A8] + team*4 + 21A4h] (team is the
// planner's +30h). The host's pass A keeps only the first arm.
bool ai_defend_member_is_candidate_00a2def0(bool member_kind_1c, bool in_team_set) noexcept;

// The 34h-byte record 00A28A60 builds: +0h entity, +4h..+1Ch 00A243D0's out[0..6]
// (+4h the threat T, +8h the weight gain*T, +10h the enemy value b), +30h the
// remaining requirement. Only +4h and +8h order the records.
struct DefendRecordKey {
    const void* entity{nullptr};
    float threat_004{0.0f};
    float weight_008{0.0f};
};

// 00A28300 STL_IntroSortLoop52 over the records with the predicate the
// insertion pass 00A25750 inlines: x goes before y when 0.0 < x+4h (COMISS
// against the double [00D7A218], JBE) and y+8h < x+8h (FCOMPI, JBE). With 32 or
// fewer records (00A28320 CMP EAX,20h / JLE) only the insertion pass runs; that
// is what this returns, as the new order of the input indices. More than 32
// records take the partition and heap paths (00A26CD0, 00A256B0), which were not
// read: nullopt.
std::optional<std::vector<std::size_t>> ai_defend_sort_records_00a28300(
    const std::vector<DefendRecordKey>& records);

// ---------------------------------------------------------------------------
// The Capture think 00A29FD0: its clock prologue and its assignment score.
// ---------------------------------------------------------------------------

struct CapturePlannerClock {
    float age_038{0.0f};           // planner +38h
    float since_spawn_03c{0.0f};   // planner +3Ch
    float last_clock_040{-1.0f};   // planner +40h
};

// 00A29FF7..00A2A0C1, run at the top of EVERY Capture think, with or without a
// target: dt = now - +40h when +40h >= 0 (COMISS/JB), 0 when that is negative
// (FCOMPI/JA); +40h = now; +38h += dt; +3Ch += dt. The spawn gate is
// +3Ch > 00419010(0.0, +1C0h, +1C8h, +1C4h, +38h) (FCOMPI/JA). Returns the gate.
bool ai_capture_planner_clock_00a29fd0(CapturePlannerClock& clock, float now,
                                       float tuning_1c0, float tuning_1c4, float tuning_1c8);

// 00A2AC11..00A2AC3E: f(+1BCh * s + d((1 - +1BCh) * w)), each x87 step at the
// 24-bit precision the game runs (docs/CC13_FPU_PRECISION.md); s is the target's
// 00A1E250 total (rec+4h), w the group's weight for it (> 0 only, 00A2AC02).
float ai_capture_assignment_score_00a2ac11(float k_1bc, float target_total, float group_weight) noexcept;

// ---------------------------------------------------------------------------
// 00A11070 BSP_AiCommand_FormationShapePass, __thiscall(command), the tail of
// the MOVETO, MOVETOATTACK, PATROLTO and IDLE ticks. Not in the host (its
// IDLE arm records it, src/game_hosts_ai.cpp:2185).
// ---------------------------------------------------------------------------

// The static slot table 00F8A7B0..00F8A86F, 24 (x, z) pairs filled once under
// bit 0 of [00F8A870] (00A110FE..00A112B1).
inline constexpr int kFormationShapeSlots = 24;
extern const std::array<std::array<float, 2>, kFormationShapeSlots> kFormationShapeTable00f8a7b0;

struct FormationShapeFollower {
    const void* member{nullptr};
    bool ship_06{false};          // vtable+5Ch(6)
    bool has_record{false};       // 0070D080(formation, member) != 0
};

struct FormationShapeSlot {
    const void* member{nullptr};
    float record_010{0.0f};       // f(-x * spacing)
    float record_020{0.0f};       // f(-z * spacing)
    int slot{0};
};

struct FormationShapeResult {
    bool ran{false};              // leader is a ship and leader+284h is set
    std::vector<FormationShapeSlot> slots;
};

// Gate: the first member answers vtable+5Ch(6) and its +284h formation is set.
// Then 0070EFD0(formation, 0); spacing = tuning +210h; every later member that
// is a ship and has a 0070D080 record takes the next table slot (the leader is
// skipped, a member without a record does not consume a slot); finally
// 0077A080 / 0077C880 build and broadcast FORMATION_SET. The image reads past
// the table from the 25th slot on (the guard dword, then .data beyond); the
// model stops at 24 and says so in `slots.size()`.
FormationShapeResult ai_formation_shape_00a11070(bool leader_ship_06, bool leader_has_formation_284,
                                                 float tuning_210,
                                                 const std::vector<FormationShapeFollower>& followers);

// ---------------------------------------------------------------------------
// 00A126C0 BSP_AiCommand_RegroupingMove_Tick, __thiscall(command) (vt+0Ch of
// REGROUPINGMOVE). Not in the host (include/bsp/ai_command_tick.hpp names it).
// ---------------------------------------------------------------------------

struct RegroupingMoveResult {
    bool regrouping{false};        // a follower is beyond AutoMerge_LeaveDist
    float point[3]{0.0f, 0.0f, 0.0f};   // what 00A02020 gets for the first member
};

// members: every +563Ch member's refreshed +FCh/+100h/+104h, the leader first.
// With a population of 1 or less (00A126CA JBE), or when no follower lies at
// f(dz^2 + dx^2) > f(+20Ch^2) from the leader (FCOMPI/JA), the point is the
// command's +8h destination. Otherwise it is f(f(dest + f(f(c/n) * 3.0)) * 0.25)
// per axis, c the sum of every member's pose (the leader too) seeded with
// origin_00f87574 and n the FILD count ([00D7A2B0] 3.0, [00D7A348] 0.25). Both
// arms then run 00A10DC0 and tail-jump to 00A11070.
RegroupingMoveResult ai_regrouping_move_tick_00a126c0(
    const std::vector<std::array<float, 3>>& members, const float destination[3],
    float tuning_20c, const float origin_00f87574[3]);

// ---------------------------------------------------------------------------
// 00A2CCF0 BSP_AiGroup_OrderCautiousMove, __thiscall(group)(point, ratio),
// RET 8; its only caller is the Duel think 00A25F70. The move twin of 00A2CBD0.
// ---------------------------------------------------------------------------

enum class OrderMoveChoice { None, Kept, MoveTo, CautiousMove };

class OrderMoveHost {
public:
    virtual ~OrderMoveHost() = default;
    // 00A2C150(group)(point): the group already holds a move to this point.
    virtual bool already_moving_to_00a2c150(const float point[3]) = 0;
    // The first member's vtable+5Ch(6).
    virtual bool leader_is_ship() = 0;
    // 00A2C9F0(group)(point) (ai_should_route_cautiously_00a2c9f0).
    virtual bool should_route_cautiously_00a2c9f0(const float point[3]) = 0;
    // 00BD2F40 with ECX = 1: a stream-1 uniform in [0, 1).
    virtual float uniform01_stream1() = 0;
};

// Empty group (+5644h == 0): None. 00A2C150 true: Kept. A ship leader, or
// 00A2C9F0 false: MoveTo (new(14h), vtable 00D22ABC) without a draw. Otherwise
// one stream-1 draw; CautiousMove (00A102D0) when draw > ratio (FCOMPI/JBE),
// else MoveTo. Both arms delete the previous +564Ch command (vtable+0(1)) and
// store the new one there (00A2CDA3..00A2CDB7).
OrderMoveChoice ai_order_cautious_move_00a2ccf0(std::uint32_t population_5644, const float point[3],
                                                float ratio, OrderMoveHost& host);

// ---------------------------------------------------------------------------
// The Duel (00A25F70) and Escort (00A26210) thinks. The host runs the
// Siege/Competitive arm (00A1CB80) for both (src/ai_planners.cpp
// ai_mode_planner_tick); neither image body calls 00A1CB80 on its main path.
// ---------------------------------------------------------------------------

enum class ModeThinkAction {
    SpawnGroup,        // 00A25B90 quick spawn ("[duel]" 00D23000 / "[escort]" 00D23008), then 00A16450(0.5)
    Attack,            // 00A2CBD0(group)(target group, 1.0)
    CautiousMove,      // 00A2CCF0(group)(point, 1.0)
    PatrolTo,          // 00A2C310(group)(point)
    ChooseAttackTarget,// 00A1CB80(planner)(group, 1.0, 0)
    None,
};

struct ModeThinkResult {
    ModeThinkAction action{ModeThinkAction::None};
    const void* target{nullptr};
    float point[3]{0.0f, 0.0f, 0.0f};
};

struct DuelThinkInputs {
    std::uint32_t owned_groups_028{0};
    float leader[3]{0.0f, 0.0f, 0.0f};          // 00A10C20(first owned group)
    const void* nearest_enemy_group{nullptr};   // 00A1C960(leader, planner+34h)
    float nearest_enemy_d2{0.0f};               // its out value, a squared distance
    std::uint32_t own_team_group_count{0};      // [00F8AA50 + (planner+30h)*0Ch]
    float own_centroid[3]{0.0f, 0.0f, 0.0f};    // 00A1CA60(planner+30h, 0)
    float enemy_centroid[3]{0.0f, 0.0f, 0.0f};  // 00A1CA60(planner+34h, 0)
};

// No owned group: SpawnGroup. No enemy group: CautiousMove to the own centroid.
// Attack when 16e6 [00D22FF8] > d2 (FCOMPI/JA) or the own team has one group or
// fewer; otherwise CautiousMove to own * t + enemy * (1 - t), t =
// 00419010(500 [00CE397C], 0, 2000 [00CFFD60], 1, 009FFC10(leader - own)), each
// product and sum stored as a float. 00A2CCF0's ratio is 1.0, so it installs
// MOVETO, but it still draws stream 1 when its gates pass.
ModeThinkResult ai_duel_think_00a25f70(const DuelThinkInputs& in);

struct EscortSetEntity {
    const void* entity{nullptr};
    float x_0fc{0.0f}, y_100{0.0f}, z_104{0.0f};
    int team_054{0};
    const void* group_16c{nullptr};
};

struct EscortThinkInputs {
    std::uint32_t owned_groups_028{0};
    float leader[3]{0.0f, 0.0f, 0.0f};       // the first owned group's first member +FCh (00F87574 when empty)
    int own_team_030{0};
    int enemy_team_034{1};
    // The set [[00E188A8] + own*4 + 21A4h] + 18h in its tree order (008DA530).
    std::vector<EscortSetEntity> set;
    float enemy_centroid[3]{0.0f, 0.0f, 0.0f};   // 00A1CA60(planner+34h, 0)
};

// No owned group: SpawnGroup. An empty set: ChooseAttackTarget (00A1CB80 with
// reset 0). Else the set entity nearest the leader (planar f(dz^2 + dx^2),
// seed 1e10, strict) decides: own team -> PatrolTo f(f(e + c) * 0.5) per axis
// (c the enemy centroid); enemy team -> Attack its +16Ch group, or PatrolTo
// its +FCh when it has none; any other team -> None.
ModeThinkResult ai_escort_think_00a26210(const EscortThinkInputs& in);

// ---------------------------------------------------------------------------
// 00A03760 BSP_AiCapture_UnitArrivalValue, __fastcall(ECX = unit, EDX = target).
// ---------------------------------------------------------------------------

// w = 00A03510(unit); d = 009FFC10(unit pose - target pose); R = (float) FILD
// target+7A0h. d <= R (FCOMPI/JBE): w. Otherwise speed = class+500h (ship, 6) or
// squadron class+188h (18h) or 0, and the guard at 00A0383C is FCOMI speed vs
// 0.1 [00D7A3A0] / JA to the 0.0 return: a unit FASTER than 0.1 scores 0 outside
// R. Only speed <= 0.1 (or NaN) reaches time = (d - R) / speed (DE F1),
// limit = max(+198h, 1.0) (FCOMI/JBE), and f((1 - time / limit) * w) when
// time < limit (JB), else 0. The bytes (DD 05 A0A3D700 ... DB F1 DD D9 77 54)
// were read from the PE.
float ai_unit_arrival_value_00a03760(float capture_weight_00a03510, float distance_009ffc10,
                                     int target_7a0, float speed, float tuning_198) noexcept;

}  // namespace gameplay_seg69_70_cc13
}  // namespace bsp
