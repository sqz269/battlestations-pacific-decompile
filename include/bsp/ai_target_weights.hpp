#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/lua_binding_ai.hpp"

// Reconstruction of the high-level AI globals loader 00A335D0, the per-mode
// tuning record it fills, the forced-target-weight rule table 00A32500/00A31DB0
// and the target-weight model 00A08460.
//
// docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md carries the evidence. Every descriptive
// name here is a hypothesis, not a recovered symbol.

namespace bsp {

// ---------------------------------------------------------------------------
// Addresses
// ---------------------------------------------------------------------------

inline constexpr std::uint32_t kAiGlobalsLoaderAddress = 0x00A335D0u;
inline constexpr std::uint32_t kAiGlobalsReloadAddress = 0x00A371C0u;
inline constexpr std::uint32_t kAiActiveModeTuningAddress = 0x00A371A0u;
inline constexpr std::uint32_t kAiGameModeSelectorAddress = 0x009FFC80u;
inline constexpr std::uint32_t kAiTargetWeightModelAddress = 0x00A08460u;
inline constexpr std::uint32_t kAiForcedRuleLookupAddress = 0x00A31DB0u;
inline constexpr std::uint32_t kAiForcedRuleInsertAddress = 0x00A32500u;

// Both scripts 00A335D0 runs, in order (00A33600 and 00A3365C).
inline constexpr const char* kAiGlobalsInitScript = "Scripts\\global\\luaMW_init.lua";
inline constexpr const char* kAiGlobalsDataScript = "Scripts\\datatables\\HighLvlAIGlobals.lua";
inline constexpr const char* kAiGlobalsRootTable = "HighLvlAIGlobals";

// ---------------------------------------------------------------------------
// Game mode index
// ---------------------------------------------------------------------------

// 009FFC80 answers 0..6. The loop in 00A335D0 (XOR EDI,EDI at 00A3374B,
// ADD EDI,1 / CMP EDI,7 at 00A370D4) reads one HighLvlAIGlobals sub-table per
// value, in this order, so the two share one index space.
enum class AiGameMode : int {
    IslandCaptureRookie = 0,
    IslandCaptureRegular = 1,
    IslandCaptureVeteran = 2,
    Duel = 3,
    Escort = 4,
    Siege = 5,
    Competitive = 6,
};
inline constexpr int kAiGameModeCount = 7;

// The sub-table name 00A335D0 reads for a mode, or nullptr when out of range.
const char* ai_mode_table_name(int mode) noexcept;

// ---------------------------------------------------------------------------
// The per-mode tuning record
// ---------------------------------------------------------------------------

// 00A371A0 returns 004C1C50()'s coordinator + mode * 23Ch + 4, so the record is
// exactly 23Ch bytes and the loader's first store (+004h) is field +000h here.
// Offsets in the comments are record-relative; subtract nothing to compare them
// with a consumer, add 4 to compare them with a store in 00A335D0.
inline constexpr std::size_t kAiModeTuningStride = 0x23C;
inline constexpr std::size_t kAiModeTuningLoaderBias = 4;
inline constexpr std::size_t kAiModeTuningFieldCount = 0x23C / sizeof(float);

struct AiModeTuning {
    float mother_ship{0.0f}; // +000h MotherShip
    float battle_ship{0.0f}; // +004h BattleShip
    float command_building{0.0f}; // +008h CommandBuilding
    float landfort{0.0f}; // +00Ch Landfort
    float cruiser{0.0f}; // +010h Cruiser
    float destroyer{0.0f}; // +014h Destroyer
    float submarine{0.0f}; // +018h Submarine
    float landing_ship{0.0f}; // +01Ch LandingShip
    float cargo{0.0f}; // +020h Cargo
    float t_boat{0.0f}; // +024h TBoat
    float level_bomber{0.0f}; // +028h LevelBomber
    float kamikaze_plane{0.0f}; // +02Ch KamikazePlane
    float torpedo_bomber{0.0f}; // +030h TorpedoBomber
    float dive_bomber{0.0f}; // +034h DiveBomber
    float fighter{0.0f}; // +038h Fighter
    float recon_plane_small{0.0f}; // +03Ch ReconPlaneSmall
    float recon_plane_large{0.0f}; // +040h ReconPlaneLarge
    float other_ship{0.0f}; // +044h OtherShip
    float other_plane{0.0f}; // +048h OtherPlane
    float other{0.0f}; // +04Ch Other
    float value_random_mul_1{0.0f}; // +050h ValueRandomMul [1]
    float value_random_mul_2{0.0f}; // +054h ValueRandomMul [2]
    float dogfight_equipment_penalty{0.0f}; // +058h DogfightEquipmentPenalty
    float max_target_kill_ratio{0.0f}; // +05Ch MaxTargetKillRatio
    float damage_calc_time{0.0f}; // +060h DamageCalcTime
    float attacker_reference_speed{0.0f}; // +064h AttackerReferenceSpeed
    float capture_small_landing_ship_survive_mul{0.0f}; // +068h Capture_SmallLandingShipSurviveMul
    float capture_in_range_capture_mul{0.0f}; // +06Ch Capture_InRangeCaptureMul
    float capture_landed_capture_mul{0.0f}; // +070h Capture_LandedCaptureMul
    float capture_landed_damage_mul{0.0f}; // +074h Capture_LandedDamageMul
    float dogfight_params_1{0.0f}; // +078h DogfightParams [1]
    float dogfight_params_2{0.0f}; // +07Ch DogfightParams [2]
    float dogfight_params_3{0.0f}; // +080h DogfightParams [3]
    float strafe_params_1{0.0f}; // +084h StrafeParams [1]
    float strafe_params_2{0.0f}; // +088h StrafeParams [2]
    float strafe_params_3{0.0f}; // +08Ch StrafeParams [3]
    float tail_gun_params_1{0.0f}; // +090h TailGunParams [1]
    float tail_gun_params_2{0.0f}; // +094h TailGunParams [2]
    float unread_098{0.0f}; // +098h: no store in 00A335D0
    float divebomb_params_1{0.0f}; // +09Ch DivebombParams [1]
    float divebomb_params_2{0.0f}; // +0A0h DivebombParams [2]
    float unread_0a4{0.0f}; // +0A4h: no store in 00A335D0
    float levelbomb_params_1{0.0f}; // +0A8h LevelbombParams [1]
    float levelbomb_params_2{0.0f}; // +0ACh LevelbombParams [2]
    float unread_0b0{0.0f}; // +0B0h: no store in 00A335D0
    float torpedo_params_1{0.0f}; // +0B4h TorpedoParams [1]
    float torpedo_params_2{0.0f}; // +0B8h TorpedoParams [2]
    float unread_0bc{0.0f}; // +0BCh: no store in 00A335D0
    float d_c_params_1{0.0f}; // +0C0h DCParams [1]
    float d_c_params_2{0.0f}; // +0C4h DCParams [2]
    float unread_0c8{0.0f}; // +0C8h: no store in 00A335D0
    float big_rocket_params_1{0.0f}; // +0CCh BigRocketParams [1]
    float big_rocket_params_2{0.0f}; // +0D0h BigRocketParams [2]
    float unread_0d4{0.0f}; // +0D4h: no store in 00A335D0
    float ship_dist_weight_arive_dist{0.0f}; // +0D8h ShipDistWeight_AriveDist
    float ship_dist_weight_travel_time_1{0.0f}; // +0DCh ShipDistWeight_TravelTime [1]
    float ship_dist_weight_travel_time_2{0.0f}; // +0E0h ShipDistWeight_TravelTime [2]
    float ship_dist_weight_weight_mul_2{0.0f}; // +0E4h ShipDistWeight_WeightMul [2]
    float ship_dist_weight_weight_mul_1{0.0f}; // +0E8h ShipDistWeight_WeightMul [1]
    float plane_dist_weight_arive_dist{0.0f}; // +0ECh PlaneDistWeight_AriveDist
    float plane_dist_weight_travel_time_1{0.0f}; // +0F0h PlaneDistWeight_TravelTime [1]
    float plane_dist_weight_travel_time_2{0.0f}; // +0F4h PlaneDistWeight_TravelTime [2]
    float plane_dist_weight_weight_mul_2{0.0f}; // +0F8h PlaneDistWeight_WeightMul [2]
    float plane_dist_weight_weight_mul_1{0.0f}; // +0FCh PlaneDistWeight_WeightMul [1]
    float travel_time_value_1{0.0f}; // +100h TravelTimeValue [1]
    float travel_time_value_2{0.0f}; // +104h TravelTimeValue [2]
    float travel_time_mul_2{0.0f}; // +108h TravelTimeMul [2]
    float travel_time_mul_1{0.0f}; // +10Ch TravelTimeMul [1]
    float machine_gun_1{0.0f}; // +110h MachineGun [1]
    float machine_gun_2{0.0f}; // +114h MachineGun [2]
    float machine_gun_3{0.0f}; // +118h MachineGun [3]
    float machine_gun_4{0.0f}; // +11Ch MachineGun [4]
    float artillery_1{0.0f}; // +120h Artillery [1]
    float artillery_2{0.0f}; // +124h Artillery [2]
    float artillery_3{0.0f}; // +128h Artillery [3]
    float artillery_4{0.0f}; // +12Ch Artillery [4]
    float bomb_1{0.0f}; // +130h Bomb [1]
    float bomb_2{0.0f}; // +134h Bomb [2]
    float bomb_3{0.0f}; // +138h Bomb [3]
    float bomb_4{0.0f}; // +13Ch Bomb [4]
    float torpedo_1{0.0f}; // +140h Torpedo [1]
    float torpedo_2{0.0f}; // +144h Torpedo [2]
    float depth_charge{0.0f}; // +148h DepthCharge
    float paratroopers{0.0f}; // +14Ch Paratroopers
    float kamikaze_1{0.0f}; // +150h Kamikaze [1]
    float kamikaze_2{0.0f}; // +154h Kamikaze [2]
    float kamikaze_3{0.0f}; // +158h Kamikaze [3]
    float kamikaze_4{0.0f}; // +15Ch Kamikaze [4]
    float small_rocket_1{0.0f}; // +160h SmallRocket [1]
    float small_rocket_2{0.0f}; // +164h SmallRocket [2]
    float small_rocket_3{0.0f}; // +168h SmallRocket [3]
    float small_rocket_4{0.0f}; // +16Ch SmallRocket [4]
    float big_rocket_1{0.0f}; // +170h BigRocket [1]
    float big_rocket_2{0.0f}; // +174h BigRocket [2]
    float big_rocket_3{0.0f}; // +178h BigRocket [3]
    float big_rocket_4{0.0f}; // +17Ch BigRocket [4]
    float flak_1{0.0f}; // +180h Flak [1]
    float flak_2{0.0f}; // +184h Flak [2]
    float flak_3{0.0f}; // +188h Flak [3]
    float flak_4{0.0f}; // +18Ch Flak [4]
    float party_presence_distance_min{0.0f}; // +190h PartyPresence_DistanceMin
    float party_presence_distance_max{0.0f}; // +194h PartyPresence_DistanceMax
    float capture_arrive_to_range_time{0.0f}; // +198h Capture_ArriveToRangeTime
    float capture_capture_point_resource_value{0.0f}; // +19Ch Capture_CapturePointResourceValue
    float capture_minimal_resource{0.0f}; // +1A0h Capture_MinimalResource
    float capture_collect_defenders_dist{0.0f}; // +1A4h Capture_CollectDefendersDist
    float capture_act_attack_target_weight_mul_2{0.0f}; // +1A8h Capture_ActAttackTargetWeightMul [2]
    float capture_act_attack_target_weight_mul_1{0.0f}; // +1ACh Capture_ActAttackTargetWeightMul [1]
    float capture_act_attack_target_weight_mul_dist_1{0.0f}; // +1B0h Capture_ActAttackTargetWeightMulDist [1]
    float capture_act_attack_target_weight_mul_dist_2{0.0f}; // +1B4h Capture_ActAttackTargetWeightMulDist [2]
    float capture_minimal_c_b_target_weight{0.0f}; // +1B8h Capture_MinimalCBTargetWeight
    float capture_command_building_strategic_weight_mul{0.0f}; // +1BCh Capture_CommandBuildingStrategicWeightMul
    float capture_spawn_delay_1{0.0f}; // +1C0h Capture_SpawnDelay [1]
    float capture_spawn_delay_2{0.0f}; // +1C4h Capture_SpawnDelay [2]
    float capture_spawn_delay_time{0.0f}; // +1C8h Capture_SpawnDelayTime
    float free_attack_objective_target_mul{0.0f}; // +1CCh FreeAttack_ObjectiveTargetMul
    float free_attack_near_dist{0.0f}; // +1D0h FreeAttack_NearDist
    float free_attack_far_dist{0.0f}; // +1D4h FreeAttack_FarDist
    float free_attack_existing_target_mul{0.0f}; // +1D8h FreeAttack_ExistingTargetMul
    float defend_merge_target_dist{0.0f}; // +1DCh Defend_MergeTargetDist
    float defend_merge_groups_dist{0.0f}; // +1E0h Defend_MergeGroupsDist
    float defend_collect_enemies_dist{0.0f}; // +1E4h Defend_CollectEnemiesDist
    float defend_against_enemy_resource_mul{0.0f}; // +1E8h Defend_AgainstEnemyResourceMul
    float defend_minimal_resource{0.0f}; // +1ECh Defend_MinimalResource
    float caution_move_dist{0.0f}; // +1F0h CautionMove_Dist
    float close_attack_collect_dist{0.0f}; // +1F4h CloseAttack_CollectDist
    float close_attack_near_dist{0.0f}; // +1F8h CloseAttack_NearDist
    float close_attack_far_dist{0.0f}; // +1FCh CloseAttack_FarDist
    float close_attack_existing_target_mul{0.0f}; // +200h CloseAttack_ExistingTargetMul
    float close_attack_target_group_member_mul{0.0f}; // +204h CloseAttack_TargetGroupMemberMul
    float auto_merge_merge_dist{0.0f}; // +208h AutoMerge_MergeDist
    float auto_merge_leave_dist{0.0f}; // +20Ch AutoMerge_LeaveDist
    float formation_unit_dist{0.0f}; // +210h Formation_UnitDist
    float compose_group_reference_weight{0.0f}; // +214h ComposeGroup_ReferenceWeight
    float compose_group_attack_sum_mul{0.0f}; // +218h ComposeGroup_AttackSumMul
    float compose_group_attacker_dont_attack_penalty{0.0f}; // +21Ch ComposeGroup_AttackerDontAttackPenalty
    float compose_group_target_dont_attacked_penalty{0.0f}; // +220h ComposeGroup_TargetDontAttackedPenalty
    float compose_group_repeat_penalty{0.0f}; // +224h ComposeGroup_RepeatPenalty
    float compose_group_speed_bonus_weight_ratio{0.0f}; // +228h ComposeGroup_SpeedBonusWeightRatio
    float compose_group_speed_bonus{0.0f}; // +22Ch ComposeGroup_SpeedBonus
    float compose_group_group_cost_modifier{0.0f}; // +230h ComposeGroup_GroupCostModifier
    float compose_group_attacker_against_target_mul{0.0f}; // +234h ComposeGroup_AttackerAgainstTargetMul
    float compose_group_target_against_attacker_mul{0.0f}; // +238h ComposeGroup_TargetAgainstAttackerMul
};
static_assert(sizeof(AiModeTuning) == kAiModeTuningStride,
              "00A371A0 strides the record by 23Ch");

// ---------------------------------------------------------------------------
// Defend_ResourcePercent: the three-slot array at 00F8A8BC
// ---------------------------------------------------------------------------

// 00A360F5 is CMP EDI,3 / JGE 00A3613E, so the loader reads
// Constants.Defend_ResourcePercent and stores FSTP [EDI*4 + 0F8A8BCh] only for
// modes 0..2. The array is three floats wide; 00F8A8C8 (twelve bytes on) is the
// first record of the 1Ch-stride per-party array of lua_binding_ai.hpp.
inline constexpr int kAiDefendResourcePercentSlotCount = 3;

// True when the loader fills the slot for this mode (mode < 3).
bool ai_globals_loader_fills_defend_resource_percent(int mode) noexcept;

// True when a slot index stays inside the three-float array. AISetDefendResource-
// Percent (MOVSS [ESI*4+0F8A8BCh] at 00A37E4B) and the read at 00A29C38 both
// index it with 009FFC80's 0..6, so this is false for modes 3..6 in the shipped
// build and the access lands in the per-party record instead.
bool ai_defend_resource_percent_slot_in_bounds(int mode) noexcept;

// The address a slot index actually reaches, in bounds or not.
std::uint32_t ai_defend_resource_percent_slot_address(int mode) noexcept;

// ---------------------------------------------------------------------------
// The forced-target-weight rule
// ---------------------------------------------------------------------------

// 20h bytes, eight dwords. Settled from the identity comparison at the head of
// 00A32500 (which compares [0], [1], [2] low byte, [4] low byte, +1Dh, +1Eh and,
// only when [4] is set, [5], [6] and [7] low byte) and from the match loop of
// 00A31DB0. This is the byte layout behind lua_binding_ai.hpp's write-order
// AiTargetWeightRule.
struct AiForcedTargetWeightRule {
    int attacker_selector{kAiClassNameNotFound}; // +00h
    int target_selector{kAiClassNameNotFound};   // +04h
    bool target_is_neutral{false};               // +08h low byte
    float weight{0.0f};                          // +0Ch, FLD at 00A31EC2
    bool relative{false};                        // +10h low byte
    int reference_attacker_selector{kAiClassNameNotFound}; // +14h
    int reference_target_selector{kAiClassNameNotFound};   // +18h
    bool reference_target_is_neutral{false};     // +1Ch low byte
    bool attacker_selector_is_class_id{false};   // +1Dh, 00A31E3A
    bool target_selector_is_class_id{false};     // +1Eh, 00A31E64
};
inline constexpr std::size_t kAiForcedRuleStride = 0x20;

// A selector scores 2 on an exact class-id comparison against the entity's +70h,
// 1 on a type-group query through the entity vtable slot +18h, 0 on no match
// (00A31E3A..00A31E93). A rule's score is the sum of the two, so 2..4, and the
// highest-scoring rule wins; 4 stops the scan (00A31EBF).
inline constexpr int kAiForcedRuleExactScore = 2;
inline constexpr int kAiForcedRuleGroupScore = 1;
inline constexpr int kAiForcedRuleBestScore = 4;
int ai_forced_rule_selector_score(bool selector_is_class_id, bool matched) noexcept;
int ai_forced_rule_score(int attacker_score, int target_score) noexcept;

// A rule applies only when its neutral byte agrees with the query's flag
// (00A31E20 / 00A31E2C, both directions).
bool ai_forced_rule_flag_applies(bool rule_target_is_neutral, bool query_flag) noexcept;

// The global table is scanned first, then the per-mode table; the first table
// that yields any match returns (00A31EEB, 00A32024).
enum class AiForcedRuleTable { Global, PerMode };

// ---------------------------------------------------------------------------
// The target-weight model 00A08460
// ---------------------------------------------------------------------------

// float __fastcall(ECX = attacker, EDX = attacker vehicle class, target, flag),
// RET 8. The four values are copied into a 16-byte key at 00A084A3..00A084BC and
// that key addresses the memo map at 00F8A734 (end sentinel 00F8A738).
struct AiTargetWeightKey {
    const void* attacker{nullptr}; // +00h, ECX
    int attacker_class{0};         // +04h, EDX
    const void* target{nullptr};   // +08h, first stack argument
    int target_is_neutral{0};      // +0Ch, second stack argument
};
inline constexpr std::uint32_t kAiTargetWeightMemoMapAddress = 0x00F8A734u;
inline constexpr std::size_t kAiTargetWeightMemoValueOffset = 0x1C; // FLD at 00A0851A

// 00D7A218, the reload epsilon of the barrel test at 00A0950B.
inline constexpr float kAiBarrelReloadEpsilon = 1.0e-30f;
// 00D7A2B0, a double 3.0 applied at 00A09771.
inline constexpr float kAiAttackerTypeBonus = 3.0f;
// The vtable slot +18h type queries the model asks.
inline constexpr int kAiTypeQueryAttackerModel = 0x0F; // 00A085AD, the branch
inline constexpr int kAiTypeQueryAttackerNoBonus = 0x14; // 00A09763
inline constexpr int kAiTypeCommandBuildingTarget = 0x1C; // CMP at 00A0966A

// Per barrel: DamageCalcTime / reload, or 1.0 when the reload is at or below the
// epsilon (00A0950B..00A09533).
float ai_barrel_time_factor(float damage_calc_time, float barrel_reload) noexcept;

// The per-barrel contribution before the range falloff: time factor * accuracy *
// shot count (FMUL/FIMUL at 00A09544/00A09548).
float ai_barrel_damage(float time_factor, float accuracy, int shots) noexcept;

// The capture accumulator is capped at DamageCalcTime before it is scaled
// (00A09602..00A0961E).
float ai_clamp_capture_accumulator(float accumulated, float damage_calc_time) noexcept;

// The in-branch cap: when total / target hit points exceeds MaxTargetKillRatio
// the total becomes hit points * MaxTargetKillRatio (00A09645..00A09660).
float ai_clamp_total_damage(float total, float target_hit_points,
                            float max_target_kill_ratio) noexcept;

// The epilogue at 00A09737: a non-positive total answers 0, otherwise the total
// is divided by the target's hit points and clamped into
// [0, MaxTargetKillRatio]. The result is a ratio, not a damage figure.
float ai_target_weight_result(float total, float target_hit_points,
                              float max_target_kill_ratio,
                              bool attacker_type_bonus) noexcept;

// ---------------------------------------------------------------------------
// Host boundaries: one virtual method per native call site
// ---------------------------------------------------------------------------

// The loader's Lua reads. 00A335D0 uses two shapes: a plain number read
// (BSP_LuaObject_GetNumber, 00B66270) and a defaulted read
// (BSP_LuaReference_GetFloatOrDefault, 00B66330) that answers the default when
// the key is absent. Both take the field by name, or by 1-based index inside an
// array-valued field.
struct AiGlobalsLoaderHost {
    virtual ~AiGlobalsLoaderHost() = default;
    // 00B65EE0 / 00B66020: open a Lua state and run one script by path.
    virtual void run_script(const char* path) = 0;
    // 00B67800: fetch a named field of the object currently on top.
    virtual bool push_field(const char* key) = 0;
    // 00B67720: fetch a 1-based index of the object currently on top.
    virtual bool push_index(int one_based) = 0;
    // 00B67700: drop the object pushed by the two calls above.
    virtual void pop_field() = 0;
    // 00B66270, the undefaulted read. Answers 0 for a missing key.
    virtual float read_number() = 0;
    // 00B66330, the defaulted read at every GetFloatOrDefault site.
    virtual float read_number_or(float fallback) = 0;
    // 004C1C50: the AI coordinator that owns the mode record array.
    virtual AiModeTuning* mode_record(int mode) = 0;
    // The store at 00A36126, FSTP [EDI*4 + 0F8A8BCh].
    virtual void store_defend_resource_percent(int mode, float value) = 0;
    // 00A32500 at 00A36FD5, one call per ForcedTargetWeightValues entry.
    virtual void insert_forced_rule(const AiForcedTargetWeightRule& rule, int mode) = 0;
};

// Runs the loader's mode loop over an injected host. Coverage: the script runs,
// the seven mode tables, every field of AiModeTuning and the guarded
// Defend_ResourcePercent store. The SupportValues and ForcedTargetWeightValues
// tails (00A36930..00A37127) are not projected; see the doc.
void ai_load_globals_00a335d0(AiGlobalsLoaderHost& host);

// The model's native call sites. The entity queries keep the native vtable slot
// numbers because no name for them is established.
struct AiTargetWeightModelHost {
    virtual ~AiTargetWeightModelHost() = default;
    // 00A03B90: look the key up in the memo map at 00F8A734.
    virtual bool memo_lookup(const AiTargetWeightKey& key, float& weight) = 0;
    // 00A079B0: address the key's slot in the same map, inserting it.
    virtual void memo_store(const AiTargetWeightKey& key, float weight) = 0;
    // 00A31DB0 at 00A08540: the forced-rule override.
    virtual bool forced_rule_weight(const AiTargetWeightKey& key, float& weight) = 0;
    // 00A371A0 at 00A08574: the tuning record for the current mode.
    virtual const AiModeTuning& mode_tuning() = 0;
    // Entity vtable slot +18h, the type-group query.
    virtual bool entity_is_type(const void* entity, int type_code) = 0;
    // Entity vtable slot +1Ch, the entity's kind; compared with 1Ch at 00A0966A.
    virtual int entity_kind(const void* entity) = 0;
    // Target +48h and +4Ch, read as floats at 00A08593 and 00A085A8.
    virtual float target_hit_points(const void* target) = 0;
    virtual float target_capture_state(const void* target) = 0;
    // The attacker's subsystem list, +94h and +98h (00A095E3).
    virtual int subsystem_count(const void* attacker) = 0;
    // One barrel of a subsystem: the 48h-stride entries at +74h/+78h.
    virtual int barrel_count(const void* subsystem) = 0;
    virtual float barrel_reload(const void* subsystem, int barrel) = 0;
    // 0072AB80 at 00A09501: shots the barrel lands in one pass.
    virtual int barrel_shots(const void* subsystem, int barrel) = 0;
    // 009FE270 at 00A094E6: the accuracy of this barrel against the target.
    virtual float barrel_accuracy(const void* subsystem, int barrel, const void* target) = 0;
    // 009FE200 at 00A09578: the four-argument distance falloff.
    virtual float distance_falloff(float a, float b, float c, float d) = 0;
    // 00424C40 +3B0h at 00A09624: the global capture-to-damage scale.
    virtual float capture_scale() = 0;
};

// The model's memo, override and normalisation spine plus the barrel loop of the
// attacker-is-not-type-0Fh branch. Coverage is partial: see the doc's routine
// table for the ranges this does not cover.
float ai_target_weight_00a08460(AiTargetWeightModelHost& host,
                                const AiTargetWeightKey& key);

// ---------------------------------------------------------------------------
// AIGetGroupInfo
// ---------------------------------------------------------------------------

// 00A378C0 creates a table with 00B67930 (lua_createtable plus a registry
// reference, which pops it), fills it with 00A2EEE0 and answers 00B66400's
// stack delta (lua_gettop minus the saved top, 00B66403..00B6640D). Every Lua
// callee of 00A2EEE0 is a LuaObject setter, so nothing is left on the stack.
inline constexpr int kAiGetGroupInfoResultCount = 0;

} // namespace bsp
