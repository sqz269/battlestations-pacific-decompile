#pragma once
#include <cstddef>
#include <cstdint>
#include <vector>

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

// Packet cc9_forced_target_weights (docs/SQUADRON_LAND_TASK.md 5cu). The
// per-mode tables the loader tail inserts (00A36FD5 -> 00A32500, DL = 0, the
// mode table at 00F8AB08 + mode*0Ch, appended in script order; 00A32500's
// identity test replaces an identical rule, and the shipped tables hold none).
// A string selector is its index in the 97-entry name table 00E0CD80 (the
// entity type-query id space, 00A36BE4) with +1Dh/+1Eh clear; a number is an
// exact vehicle class id with the byte set (00A36C7A, 00A36E34). These are
// this installation's ForcedTargetWeightValues (highlvlaiglobals.lua, mtime
// 2024-07-13): Rookie lines 165-183, Regular 353-369, Veteran 539-555, Escort
// 866-880; Duel, Siege and Competitive are empty. No shipped row is relative.
// LABELLED: transcribed from the script, not read from a running loader.
const std::vector<AiForcedTargetWeightRule>& ai_shipped_forced_rules(int mode) noexcept;

// One side of a 00A31DB0 query: the class descriptor's +70h class id and its
// vtable[+18h] type query.
struct AiForcedRuleSubject {
    virtual ~AiForcedRuleSubject() = default;
    virtual int class_id() const = 0;
    virtual bool is_type(int type_code) const = 0;
};

// 00A31DB0's scan of one table (00A31E14..00A31EE5): a rule whose neutral byte
// agrees with the flag scores its two selectors; a zero on either side skips it;
// a strictly higher sum takes the rule's weight (+0Ch, added to the zeroed out
// slot at 00A31E9E); 4 stops the scan. Answers whether any rule matched, and
// the matched rule's index. A relative rule (+10h) recurses without end in the
// image (docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md 3); it is not projected and is
// skipped here.
bool ai_forced_rule_scan_00a31db0(const std::vector<AiForcedTargetWeightRule>& table,
                                  const AiForcedRuleSubject& attacker,
                                  const AiForcedRuleSubject& target, bool query_flag,
                                  float& weight, int& matched_index);

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

// Packet cc9_plane_attacker_weight (docs/SQUADRON_LAND_TASK.md 5cq). The other
// vtable[+18h] codes 00A08460 pushes, read from the listing; the meanings are
// docs/ENTITY_CLASS_IDS.md's id space (a hypothesis for the class descriptor's
// slot, which this process answers with the unit's own kind test).
inline constexpr int kAiTypeShip = 0x06;          // 00A085E9, 00A08712, 00A092EF
inline constexpr int kAiTypeSubmarine = 0x08;     // 00A08637, 00A092CD, 00A09320
inline constexpr int kAiTypeLandingShip = 0x0C;   // 00A0927A (target), 00A09232 (attacker)
inline constexpr int kAiTypeTorpedoBoat = 0x0E;   // 00A0926B
inline constexpr int kAiTypeKamikazePlane = 0x17; // 00A08AAE
inline constexpr int kAiTypeGunDevice = 0x20;     // 00A08BA0 / 00A09391, a device class
inline constexpr int kAiTypeBombPlatform = 0x25;  // 00A08BB3 / 00A093A4, a device class
inline constexpr int kAiTypeLevelBomber = 0x10;   // 00A08980, the attacker (loadout arm)

// The non-plane walk's per-target gates, 00A0924C..00A0932B. All five are false
// for a neutral target ([ESP+1Eh], target_is_neutral == 1).
struct AiBarrelTargetGates {
    bool soft{false};       // [ESP+2Bh]: plane, 0Eh or 0Ch (bullet sub-types 1-3)
    bool flak{false};       // [ESP+13h]: plane (sub-type 10h)
    bool artillery{false};  // [ESP+11h]: not a plane, or 08h (4-7, 12h)
    bool torpedo{false};    // [ESP+1Fh]: 06h and not 08h (0Ah)
    bool depth{false};      // [ESP+12h]: 08h (0Bh)
};
AiBarrelTargetGates ai_barrel_target_gates(bool neutral, bool plane, bool torpedo_boat,
                                           bool landing_ship, bool ship,
                                           bool submarine) noexcept;
// 00A093E1..00A0943D: whether a bullet sub-type passes those gates. Any other
// sub-type (bomb 9, 0Ch-0Fh, kamikaze 11h, 13h) is skipped.
bool ai_barrel_gate_admits(const AiBarrelTargetGates& gates, int bullet_sub_type) noexcept;

// The type-0Fh attacker branch 00A0861F..00A09222. Attacker key +4h <= 0
// (00A0864F JLE 00A08A93) takes the arm with no loadout; above 0 the loadout arm
// 00A08655..00A08A8E, which 00A04560 reaches with record+10h = [plane+C54h]
// while the plane holds a rack round (00A04619..00A0464E, 007B9140(plane, 1)).
// The option record is 14h bytes (00A08D7A's divide by 5 over a 4-byte stride).
inline constexpr std::uint32_t kAiPlaneOptionKamikaze = 0x00E08F50u;  // 00A08B0C
inline constexpr std::uint32_t kAiPlaneOptionStrafe = 0x00E08F40u;    // 00A08C6C
inline constexpr std::uint32_t kAiPlaneOptionDogfight = 0x00E08F58u;  // 00A08C38, 00A08D1C
// device +80h, the Function category AAMACHINEGUN (00A08CA0 CMP [EDI+80h],1).
inline constexpr int kAiPlaneTailGunFunction = 1;

// Packet cc9_ai_plane_loadout_arm (docs/SQUADRON_LAND_TASK.md 5df). The loadout
// arm 00A08655..00A08A8E builds at most one option from the rack ordnance of the
// attacker class's loadout record+10h. Names are hypotheses from the arm that
// stores each descriptor; the kamikaze one (00E08F50) is shared with the
// no-loadout arm.
inline constexpr std::uint32_t kAiPlaneOptionTorpedo = 0x00E08F18u;     // 00A08743, sub-type 0Ah
inline constexpr std::uint32_t kAiPlaneOptionDiveBomb = 0x00E08F20u;    // 00A089E2, sub-type 9
inline constexpr std::uint32_t kAiPlaneOptionLevelBomb = 0x00E08F28u;   // 00A089A8 (9), 00A08826 (0Fh)
inline constexpr std::uint32_t kAiPlaneOptionDepthCharge = 0x00E08F38u; // 00A08787, sub-type 0Bh
inline constexpr std::uint32_t kAiPlaneOptionRocket = 0x00E08F48u;      // 00A088D7, 00A08926, sub-type 12h
// [00CE4C04] 9999.0f, the reload minimum's seed (00A08662); [00D7A280] 0.5,
// the attack-pass half weight the arm multiplies Params [1] by (00A08754).
inline constexpr float kAiLoadoutReloadSeed = 9999.0f;
inline constexpr double kAiLoadoutParamsHalf = 0.5;

// Packet cc9_ai_loadout_carried_terms (docs/SQUADRON_LAND_TASK.md 5dl): a host
// fills the paratrooper fields (007AC780) and the carried kamikaze's class
// (006FF170) of AiPlaneBulletFacts only with this ON; OFF they stay unknown and
// both options score 0, as before.
inline constexpr bool kAiLoadoutCarriedTermsBound = false;

// Packet cc9_ai_rocket_accuracy (docs/SQUADRON_LAND_TASK.md 5dm): the loadout
// arm's rocket option asks the host's rocket_accuracy (009FE4F1 read whole)
// instead of bullet_accuracy, which answers 0 for sub-type 12h.
inline constexpr bool kAiRocketAccuracyBound = false;

// 009FE4F1, 009FE270's arm for sub-type 12h, as a pure function to the tuning
// record offset (0 = the reject arm 009FE6BB).
// - An attacker that is not plane-based (EBP->vtable[18h](0Fh) false at
//   009FE4FF) jumps into the Artillery arm (009FE632 -> 009FE322 / 009FE334):
//   120h, 124h, 128h, 12Ch by target group.
// - A plane with a small rocket (006E3260: IgnitionDelay <= 0, 009FE507):
//   a plane target needs `air_ok` (007B80A0, 009FE516) and reads 160h; any
//   other target needs `ground_ok` (007B80C0, 009FE530) and reads 164h, 168h
//   or 16Ch; otherwise 0.
// - A plane with a big rocket: 170h, 174h, 178h, 17Ch (009FE5C1..009FE62F).
// `group` is AiAccuracyTargetGroup (ai_tuning_globals.hpp) cast to int.
std::uint32_t ai_rocket_accuracy_offset_009fe4f1(bool attacker_plane, bool small_rocket,
                                                 bool air_ok, bool ground_ok,
                                                 int group) noexcept;

// A bullet class record as the plane arm reads it.
struct AiPlaneBulletFacts {
    bool present{false};
    int sub_type{0};         // +8h, 009FE270's selector
    float damage_min{0.0f};  // +ACh, 00A08F35
    float damage_max{0.0f};  // +B0h, 00A08F20
    float blast_min{0.0f};   // +B4h, 00A08E10
    float blast_max{0.0f};   // +B8h, 00A08E06
    // Loadout arm only. MRocket IgnitionDelay +E0h (006E3260 answers +E0h <= 0,
    // the small-rocket test at 00A08870) and AntiAir +E4h (007B80A0 / 007B80C0).
    float ignition_delay{0.0f};
    bool anti_air{false};
    // MParatrooper +D8h, +F8h and +FCh, read by the 0Fh scoring at 00A08EBA..
    // 00A08ED9. Their reader 007AC780 (cc9-lua38, SQUADRON_LAND_TASK 5dl) reads
    // plain Lua numbers: +D8h `CapturePower` (007AC91D), +F8h `CaptureDuration`
    // (007AC844), +FCh `Damage` (007AC8FA). A host leaves the flag false until
    // it has them (kAiLoadoutCarriedTermsBound), and the option then scores 0.
    bool paratrooper_terms_known{false};
    float paratrooper_d8{0.0f};
    float paratrooper_f8{0.0f};
    float paratrooper_fc{0.0f};
    // MDummyKamikazePlane: the 0Dh option scores the blast pair of [bullet+DCh]'s
    // class +210h (00A08DE4). The +DCh reader 006FF170 stores
    // VehicleClass_GetOrCreate(`KamikazePlaneClass`) (006FF1C1..006FF1EE), whose
    // +210h is that class's `KamikazeBulletClass`.
    bool carried_kamikaze_known{false};
    float carried_blast_min{0.0f};
    float carried_blast_max{0.0f};
    int carried_sub_type{0};
};
// One entry of 009552E0's loadout record (+4h list, node +4h next, +8h entry),
// as the loadout arm reads it (00A08686..00A086F3).
struct AiPlaneLoadoutEntryFacts {
    int ammo{0};                  // entry +8h, summed over rack entries (00A086EA)
    float reload{0.0f};           // entry +0Ch, minimum over rack entries (00A086C6)
    bool device_is_rack{false};   // 00443490(entry +4h) answers vtable[+18h](25h)
    // Device class +F4h (00A08720), compared with 0.0 [00D7A218]: above it a
    // torpedo may score against a submarine. A BombPlatform class is an E8h
    // allocation (00443273), so the read lies past the object; no producer.
    float device_f4{0.0f};
    AiPlaneBulletFacts bullet;    // 00731040: [class+74h] entry 0's +34h, absent when +78h is 0
};
// One entry of the attacker class's platform vector +94h/+98h (00A08B60).
struct AiPlanePlatformFacts {
    bool present{false};          // the slot is non-null (00A08B69)
    bool has_default_gun{false};  // +38h != -1 (00A08B71)
    int gun_count{0};             // +18h, the Gun list's size; must be 1 (00A08B7B)
    bool device_is_gun{false};    // first device answers 20h and not 25h (00A08BA0/00A08BB3)
    int device_function{-1};      // device +80h (00A08CA0)
    bool pilot_fires{false};      // +0Ch PilotFires (00A08BE0)
    float reload{0.0f};           // the device's first bullet entry +2Ch (00A08C54)
    AiPlaneBulletFacts bullet;    // that entry's +34h; absent fails 00A08BC6/00A08BD5
};
struct AiPlaneOption {
    std::uint32_t descriptor{0};  // +0Ch
    AiPlaneBulletFacts bullet;    // +4h (null for the kamikaze option)
    float factor{0.0f};           // +10h
};

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
    // Target +48h and +4Ch, read as floats at 00A08593 and 00A085A8. The target
    // is the vehicle CLASS descriptor (00A04560 record+0h, [entity+538h] or
    // [+35Ch]), so these are the class HP and the class Armour
    // (docs/VEHICLE_CLASS_FIELDS.md); +4Ch was once read as a capture state.
    virtual float target_hit_points(const void* target) = 0;
    virtual float target_armour(const void* target) = 0;
    // 00A085F8: when the target class answers vtable[+18h](6), vtable[+24h]'s
    // answer replaces the +4Ch copy in the frame slot the sub-type 0Ah barrels
    // read (00A09448); for a ship class that slot is 009635D0, UnderwaterArmour.
    virtual float target_underwater_armour(const void* target) = 0;
    // The attacker's subsystem list, +94h and +98h (00A095E3).
    virtual int subsystem_count(const void* attacker) = 0;
    // One barrel of a subsystem: the 48h-stride entries at +74h/+78h.
    virtual int barrel_count(const void* subsystem) = 0;
    virtual float barrel_reload(const void* subsystem, int barrel) = 0;
    // 0072AB80 at 00A09501: shots the barrel lands in one pass.
    virtual int barrel_shots(const void* subsystem, int barrel) = 0;
    // 009FE270 at 00A094E6: the accuracy of this barrel against the target.
    virtual float barrel_accuracy(const void* subsystem, int barrel, const void* target) = 0;
    // The barrel's bullet class record ([entry+34h]): sub-type +8h,
    // max(DamageMin +ACh, BlastDamageMin +B4h) and max(DamageMax +B0h,
    // BlastDamageMax +B8h) (00A09460..00A094C9, 00415550 BSP_Math_MaxFloatByRef),
    // and WaterDamage +BCh (00A095A9).
    virtual int barrel_sub_type(const void* subsystem, int barrel) = 0;
    virtual float barrel_damage_low(const void* subsystem, int barrel) = 0;
    virtual float barrel_damage_high(const void* subsystem, int barrel) = 0;
    virtual float barrel_water_damage(const void* subsystem, int barrel) = 0;
    // 00424C40 +3B0h at 00A09624, WaterTickDamage: the scale of the water term.
    virtual float water_damage_scale() = 0;
    // Packet cc9_ai_target_weight_damage_terms. False keeps the pre-bind
    // substitutions: no armour gate, a per-hit damage of 1.0 where 009FE200
    // stands, a water term of hits x 1 at scale 1.0.
    virtual bool damage_terms_bound() = 0;
    // A census hook, no native counterpart: one call per barrel the bound loop
    // reaches, with whether the armour gate refused it and 009FE200's answer.
    virtual void note_damage_terms_barrel(bool refused, float per_hit) {
        (void)refused;
        (void)per_hit;
    }
    // Packet cc9_plane_attacker_weight. Defaults keep a host that does not
    // implement them on the earlier behaviour: no gates, no plane arm.
    // True: the non-plane walk applies 00A0924C..00A0943D's target gates and
    // its platform filters (00A09362..00A093A8).
    virtual bool barrel_target_gates_bound() { return false; }
    // 00A09362..00A093A8 for the platform the flattened barrel came from:
    // +38h != -1, +18h == 1, device 20h and not 25h.
    virtual bool barrel_platform_admitted(const void* subsystem, int barrel) {
        (void)subsystem;
        (void)barrel;
        return true;
    }
    // True: an attacker answering 0Fh takes the plane arm (00A08619) and the
    // 00A09771 bonus test asks the real type; false keeps both off.
    virtual bool plane_arm_bound() { return false; }
    virtual int plane_platform_count(const void* attacker) {
        (void)attacker;
        return 0;
    }
    virtual bool plane_platform(const void* attacker, int index, AiPlanePlatformFacts& out) {
        (void)attacker;
        (void)index;
        (void)out;
        return false;
    }
    // class+210h, the KamikazeBulletClass record 00A08DEE reads.
    virtual bool kamikaze_bullet(const void* attacker, AiPlaneBulletFacts& out) {
        (void)attacker;
        (void)out;
        return false;
    }
    // Packet cc9_ai_plane_loadout_arm. False keeps the earlier behaviour: a key
    // with +4h > 0 builds no option at all.
    virtual bool plane_loadout_arm_bound() { return false; }
    // 009552E0(ECX = attacker class, loadout) at 00A0865A: the class's
    // Equipments[loadout] entries (class +128h count, 00954DF0 index loadout-1),
    // in list order. False when 009552E0 answers null (loadout past the count).
    virtual bool plane_loadout(const void* attacker, int loadout,
                               std::vector<AiPlaneLoadoutEntryFacts>& out) {
        (void)attacker;
        (void)loadout;
        (void)out;
        return false;
    }
    // [00F874FD], read by 007B80A0 / 007B80C0: set by the Lua binding
    // SetRocketAirGroundTypeDifferent(false) (008C172D), cleared at mission load
    // (004DFBE5). No reference-row script calls it.
    virtual bool rocket_air_ground_same() { return false; }
    // 009FE270(ECX = attacker, EDX = bullet, target) for a bullet sub-type.
    virtual float bullet_accuracy(const void* attacker, int sub_type, const void* target) {
        (void)attacker;
        (void)sub_type;
        (void)target;
        return 0.0f;
    }
    // Packet cc9_ai_rocket_accuracy: 009FE270 for a sub-type 12h bullet, whose
    // arm 009FE4F1 also reads the bullet (006E3260, 007B80A0, 007B80C0) and the
    // attacker's plane base. A host without that answer keeps bullet_accuracy.
    virtual float rocket_accuracy(const void* attacker, const AiPlaneBulletFacts& bullet,
                                  const void* target) {
        return bullet_accuracy(attacker, bullet.sub_type, target);
    }
    // Census hooks, no native counterpart.
    virtual void note_plane_option(std::uint32_t descriptor, float value) {
        (void)descriptor;
        (void)value;
    }
    virtual void note_plane_option_terms(float factor, double accuracy, float damage) {
        (void)factor;
        (void)accuracy;
        (void)damage;
    }
    virtual void note_plane_arm(bool loadout_arm, bool no_options) {
        (void)loadout_arm;
        (void)no_options;
    }
    // One call per bound loadout-arm visit: the rack bullet sub-type it found
    // (0 for no list or no rack) and the descriptor it built (0 for none).
    virtual void note_loadout_arm(int bullet_sub_type, std::uint32_t descriptor) {
        (void)bullet_sub_type;
        (void)descriptor;
    }
    virtual void note_barrel_gate(int bullet_sub_type, bool admitted) {
        (void)bullet_sub_type;
        (void)admitted;
    }
};

// The plane arm's option list for one (attacker, target): 00A0861F..00A08D5F.
// `equipment_penalty` receives [ESP+2Bh]: key +4h > 0, cleared by the small
// rocket arm (00A08882). Coverage: complete with the loadout arm bound; unbound,
// the loadout arm (00A08655..00A08A8E) builds nothing.
std::vector<AiPlaneOption> ai_plane_attack_options(AiTargetWeightModelHost& host,
                                                   const AiTargetWeightKey& key,
                                                   const AiModeTuning& tuning,
                                                   bool* equipment_penalty = nullptr);
// 00A0861F..00A09222: the options scored and summed (00A08D70..00A0921D).
// Returns the total 00A09737's epilogue normalises. `target_underwater_armour`
// is [ESP+48h] (00A085F8), which the torpedo and depth-charge options read.
float ai_plane_attack_total(AiTargetWeightModelHost& host, const AiTargetWeightKey& key,
                            const AiModeTuning& tuning, float target_hit_points,
                            float target_armour, float target_underwater_armour);

// 009FE200 (__stdcall, RET 10h, body 009FE200-009FE26A): the expected damage one
// hit deals above the armour when the damage is uniform on [low, high], capped
// at the target's hit points. 0 when high <= armour; the mean minus the armour
// when low >= armour; otherwise (high - armour)^2 / (2 (high - low)). The 0.5
// is the double at 00D7A280. Called at 00A09578 as (low, high, armour, hp).
float ai_expected_hit_damage_009fe200(float low, float high, float armour,
                                      float hit_points) noexcept;

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
