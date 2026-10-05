#include "bsp/ai_target_weights.hpp"

// Evidence: docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md.
// Target MSVC Win32. Nothing here is a binary-compatible replacement: the native
// routines are __fastcall over game objects this project does not own, so the
// loader and the model are sequences over an injected host instead.

namespace bsp {
namespace {

// The seven HighLvlAIGlobals sub-tables the loop at 00A337xx reads, in the order
// the CMP EDI,n chain tests them (00A3379D through 00A3385C).
const char* const kAiModeTableNames[kAiGameModeCount] = {
    "IslandCaptureParams_Rookie",
    "IslandCaptureParams_Regular",
    "IslandCaptureParams_Veteran",
    "DuelParams",
    "EscortParams",
    "SiegeParams",
    "CompetitiveParams",
};

// One field store in 00A335D0. `index` is the 1-based Lua array index when the
// field is an array element and 0 when it is read directly. `defaulted` picks
// 00B66330 (read with a fallback) over 00B66270 (plain read, 0 when absent).
struct AiModeTuningField {
    const char* section;
    const char* key;
    int index;
    std::size_t field;
    bool defaulted;
    float fallback;
};

const AiModeTuningField kAiModeTuningFields[] = {
    {"UnitWeights", "MotherShip", 0, 0x000 / 4, false, 0.0f},
    {"UnitWeights", "BattleShip", 0, 0x004 / 4, false, 0.0f},
    {"UnitWeights", "CommandBuilding", 0, 0x008 / 4, false, 0.0f},
    {"UnitWeights", "Landfort", 0, 0x00C / 4, false, 0.0f},
    {"UnitWeights", "Cruiser", 0, 0x010 / 4, false, 0.0f},
    {"UnitWeights", "Destroyer", 0, 0x014 / 4, false, 0.0f},
    {"UnitWeights", "Submarine", 0, 0x018 / 4, false, 0.0f},
    {"UnitWeights", "LandingShip", 0, 0x01C / 4, false, 0.0f},
    {"UnitWeights", "Cargo", 0, 0x020 / 4, false, 0.0f},
    {"UnitWeights", "TBoat", 0, 0x024 / 4, false, 0.0f},
    {"UnitWeights", "LevelBomber", 0, 0x028 / 4, false, 0.0f},
    {"UnitWeights", "KamikazePlane", 0, 0x02C / 4, false, 0.0f},
    {"UnitWeights", "TorpedoBomber", 0, 0x030 / 4, false, 0.0f},
    {"UnitWeights", "DiveBomber", 0, 0x034 / 4, false, 0.0f},
    {"UnitWeights", "Fighter", 0, 0x038 / 4, false, 0.0f},
    {"UnitWeights", "ReconPlaneSmall", 0, 0x03C / 4, false, 0.0f},
    {"UnitWeights", "ReconPlaneLarge", 0, 0x040 / 4, false, 0.0f},
    {"UnitWeights", "OtherShip", 0, 0x044 / 4, false, 0.0f},
    {"UnitWeights", "OtherPlane", 0, 0x048 / 4, false, 0.0f},
    {"UnitWeights", "Other", 0, 0x04C / 4, false, 0.0f},
    {"AttackerVSTarget", "ValueRandomMul", 1, 0x050 / 4, false, 0.0f},
    {"AttackerVSTarget", "ValueRandomMul", 2, 0x054 / 4, false, 0.0f},
    {"AttackerVSTarget", "DogfightEquipmentPenalty", 0, 0x058 / 4, false, 0.0f},
    {"AttackerVSTarget", "MaxTargetKillRatio", 0, 0x05C / 4, false, 0.0f},
    {"AttackerVSTarget", "DamageCalcTime", 0, 0x060 / 4, false, 0.0f},
    {"AttackerVSTarget", "AttackerReferenceSpeed", 0, 0x064 / 4, true, 1000.0f},
    {"AttackerVSTarget", "Capture_SmallLandingShipSurviveMul", 0, 0x068 / 4, true, 0.75f},
    {"AttackerVSTarget", "Capture_InRangeCaptureMul", 0, 0x06C / 4, true, 0.5f},
    {"AttackerVSTarget", "Capture_LandedCaptureMul", 0, 0x070 / 4, true, 0.8f},
    {"AttackerVSTarget", "Capture_LandedDamageMul", 0, 0x074 / 4, true, 0.8f},
    {"AttackerVSTarget", "DogfightParams", 1, 0x078 / 4, false, 0.0f},
    {"AttackerVSTarget", "DogfightParams", 2, 0x07C / 4, false, 0.0f},
    {"AttackerVSTarget", "DogfightParams", 3, 0x080 / 4, false, 0.0f},
    {"AttackerVSTarget", "StrafeParams", 1, 0x084 / 4, false, 0.0f},
    {"AttackerVSTarget", "StrafeParams", 2, 0x088 / 4, false, 0.0f},
    {"AttackerVSTarget", "StrafeParams", 3, 0x08C / 4, false, 0.0f},
    {"AttackerVSTarget", "TailGunParams", 1, 0x090 / 4, false, 0.0f},
    {"AttackerVSTarget", "TailGunParams", 2, 0x094 / 4, false, 0.0f},
    {"AttackerVSTarget", "DivebombParams", 1, 0x09C / 4, false, 0.0f},
    {"AttackerVSTarget", "DivebombParams", 2, 0x0A0 / 4, false, 0.0f},
    {"AttackerVSTarget", "LevelbombParams", 1, 0x0A8 / 4, false, 0.0f},
    {"AttackerVSTarget", "LevelbombParams", 2, 0x0AC / 4, false, 0.0f},
    {"AttackerVSTarget", "TorpedoParams", 1, 0x0B4 / 4, false, 0.0f},
    {"AttackerVSTarget", "TorpedoParams", 2, 0x0B8 / 4, false, 0.0f},
    {"AttackerVSTarget", "DCParams", 1, 0x0C0 / 4, false, 0.0f},
    {"AttackerVSTarget", "DCParams", 2, 0x0C4 / 4, false, 0.0f},
    {"AttackerVSTarget", "BigRocketParams", 1, 0x0CC / 4, false, 0.0f},
    {"AttackerVSTarget", "BigRocketParams", 2, 0x0D0 / 4, false, 0.0f},
    {"AttackerVSTarget", "ShipDistWeight_AriveDist", 0, 0x0D8 / 4, true, 5000.0f},
    {"AttackerVSTarget", "ShipDistWeight_TravelTime", 1, 0x0DC / 4, true, 60.0f},
    {"AttackerVSTarget", "ShipDistWeight_TravelTime", 2, 0x0E0 / 4, true, 300.0f},
    {"AttackerVSTarget", "ShipDistWeight_WeightMul", 2, 0x0E4 / 4, true, 0.2f},
    {"AttackerVSTarget", "ShipDistWeight_WeightMul", 1, 0x0E8 / 4, true, 1.0f},
    {"AttackerVSTarget", "PlaneDistWeight_AriveDist", 0, 0x0EC / 4, true, 4000.0f},
    {"AttackerVSTarget", "PlaneDistWeight_TravelTime", 1, 0x0F0 / 4, true, 1.0f},
    {"AttackerVSTarget", "PlaneDistWeight_TravelTime", 2, 0x0F4 / 4, true, 90.0f},
    {"AttackerVSTarget", "PlaneDistWeight_WeightMul", 2, 0x0F8 / 4, true, 0.2f},
    {"AttackerVSTarget", "PlaneDistWeight_WeightMul", 1, 0x0FC / 4, true, 1.0f},
    {"SpawnAgainstTargets", "TravelTimeValue", 1, 0x100 / 4, false, 0.0f},
    {"SpawnAgainstTargets", "TravelTimeValue", 2, 0x104 / 4, false, 0.0f},
    {"SpawnAgainstTargets", "TravelTimeMul", 2, 0x108 / 4, false, 0.0f},
    {"SpawnAgainstTargets", "TravelTimeMul", 1, 0x10C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "MachineGun", 1, 0x110 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "MachineGun", 2, 0x114 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "MachineGun", 3, 0x118 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "MachineGun", 4, 0x11C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Artillery", 1, 0x120 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Artillery", 2, 0x124 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Artillery", 3, 0x128 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Artillery", 4, 0x12C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Bomb", 1, 0x130 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Bomb", 2, 0x134 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Bomb", 3, 0x138 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Bomb", 4, 0x13C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Torpedo", 1, 0x140 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Torpedo", 2, 0x144 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "DepthCharge", 0, 0x148 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Paratroopers", 0, 0x14C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Kamikaze", 1, 0x150 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Kamikaze", 2, 0x154 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Kamikaze", 3, 0x158 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Kamikaze", 4, 0x15C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "SmallRocket", 1, 0x160 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "SmallRocket", 2, 0x164 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "SmallRocket", 3, 0x168 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "SmallRocket", 4, 0x16C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "BigRocket", 1, 0x170 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "BigRocket", 2, 0x174 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "BigRocket", 3, 0x178 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "BigRocket", 4, 0x17C / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Flak", 1, 0x180 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Flak", 2, 0x184 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Flak", 3, 0x188 / 4, false, 0.0f},
    {"BulletTypeAccuracy", "Flak", 4, 0x18C / 4, false, 0.0f},
    {"Constants", "PartyPresence_DistanceMin", 0, 0x190 / 4, true, 4000.0f},
    {"Constants", "PartyPresence_DistanceMax", 0, 0x194 / 4, true, 10000.0f},
    {"Constants", "Capture_ArriveToRangeTime", 0, 0x198 / 4, true, 30.0f},
    {"Constants", "Capture_CapturePointResourceValue", 0, 0x19C / 4, true, 35.0f},
    {"Constants", "Capture_MinimalResource", 0, 0x1A0 / 4, true, 150.0f},
    {"Constants", "Capture_CollectDefendersDist", 0, 0x1A4 / 4, true, 5000.0f},
    {"Constants", "Capture_ActAttackTargetWeightMul", 2, 0x1A8 / 4, true, 1.5f},
    {"Constants", "Capture_ActAttackTargetWeightMul", 1, 0x1AC / 4, true, 100.0f},
    {"Constants", "Capture_ActAttackTargetWeightMulDist", 1, 0x1B0 / 4, true, 3000.0f},
    {"Constants", "Capture_ActAttackTargetWeightMulDist", 2, 0x1B4 / 4, true, 4500.0f},
    {"Constants", "Capture_MinimalCBTargetWeight", 0, 0x1B8 / 4, true, 0.5f},
    {"Constants", "Capture_CommandBuildingStrategicWeightMul", 0, 0x1BC / 4, true, 0.8f},
    {"Constants", "Capture_SpawnDelay", 1, 0x1C0 / 4, true, 30.0f},
    {"Constants", "Capture_SpawnDelay", 2, 0x1C4 / 4, true, 5.0f},
    {"Constants", "Capture_SpawnDelayTime", 0, 0x1C8 / 4, true, 120.0f},
    {"Constants", "FreeAttack_ObjectiveTargetMul", 0, 0x1CC / 4, true, 2.0f},
    {"Constants", "FreeAttack_NearDist", 0, 0x1D0 / 4, true, 5000.0f},
    {"Constants", "FreeAttack_FarDist", 0, 0x1D4 / 4, true, 12000.0f},
    {"Constants", "FreeAttack_ExistingTargetMul", 0, 0x1D8 / 4, true, 1.5f},
    {"Constants", "Defend_MergeTargetDist", 0, 0x1DC / 4, true, 2000.0f},
    {"Constants", "Defend_MergeGroupsDist", 0, 0x1E0 / 4, true, 500.0f},
    {"Constants", "Defend_CollectEnemiesDist", 0, 0x1E4 / 4, true, 6000.0f},
    {"Constants", "Defend_AgainstEnemyResourceMul", 0, 0x1E8 / 4, true, 1.0f},
    {"Constants", "Defend_MinimalResource", 0, 0x1EC / 4, true, 200.0f},
    {"Constants", "CautionMove_Dist", 0, 0x1F0 / 4, true, 8000.0f},
    {"Constants", "CloseAttack_CollectDist", 0, 0x1F4 / 4, true, 5000.0f},
    {"Constants", "CloseAttack_NearDist", 0, 0x1F8 / 4, true, 3000.0f},
    {"Constants", "CloseAttack_FarDist", 0, 0x1FC / 4, true, 8000.0f},
    {"Constants", "CloseAttack_ExistingTargetMul", 0, 0x200 / 4, true, 1.5f},
    {"Constants", "CloseAttack_TargetGroupMemberMul", 0, 0x204 / 4, true, 2.0f},
    {"Constants", "AutoMerge_MergeDist", 0, 0x208 / 4, true, 650.0f},
    {"Constants", "AutoMerge_LeaveDist", 0, 0x20C / 4, true, 1200.0f},
    {"Constants", "Formation_UnitDist", 0, 0x210 / 4, true, 300.0f},
    {"Constants", "ComposeGroup_ReferenceWeight", 0, 0x214 / 4, true, 10.0f},
    {"Constants", "ComposeGroup_AttackSumMul", 0, 0x218 / 4, true, 0.25f},
    {"Constants", "ComposeGroup_AttackerDontAttackPenalty", 0, 0x21C / 4, true, 2.0f},
    {"Constants", "ComposeGroup_TargetDontAttackedPenalty", 0, 0x220 / 4, true, 2.0f},
    {"Constants", "ComposeGroup_RepeatPenalty", 0, 0x224 / 4, true, 1.0f},
    {"Constants", "ComposeGroup_SpeedBonusWeightRatio", 0, 0x228 / 4, true, 0.5f},
    {"Constants", "ComposeGroup_SpeedBonus", 0, 0x22C / 4, true, 1.0f},
    {"Constants", "ComposeGroup_GroupCostModifier", 0, 0x230 / 4, true, 0.4f},
    {"Constants", "ComposeGroup_AttackerAgainstTargetMul", 0, 0x234 / 4, true, 0.8f},
    {"Constants", "ComposeGroup_TargetAgainstAttackerMul", 0, 0x238 / 4, true, 0.4f},
};

} // namespace

const char* ai_mode_table_name(int mode) noexcept {
    if (mode < 0 || mode >= kAiGameModeCount) {
        return nullptr;
    }
    return kAiModeTableNames[mode];
}

bool ai_globals_loader_fills_defend_resource_percent(int mode) noexcept {
    // 00A360F5: CMP EDI,3 / JGE 00A3613E.
    return mode >= 0 && mode < kAiDefendResourcePercentSlotCount;
}

bool ai_defend_resource_percent_slot_in_bounds(int mode) noexcept {
    return mode >= 0 && mode < kAiDefendResourcePercentSlotCount;
}

std::uint32_t ai_defend_resource_percent_slot_address(int mode) noexcept {
    return kAiDefendResourcePercentArrayAddress +
           static_cast<std::uint32_t>(mode) * sizeof(float);
}

int ai_forced_rule_selector_score(bool selector_is_class_id, bool matched) noexcept {
    if (!matched) {
        return 0;
    }
    return selector_is_class_id ? kAiForcedRuleExactScore : kAiForcedRuleGroupScore;
}

int ai_forced_rule_score(int attacker_score, int target_score) noexcept {
    // 00A31E93: ADD ESI,EAX, with a zero on either side already rejected.
    if (attacker_score == 0 || target_score == 0) {
        return 0;
    }
    return attacker_score + target_score;
}

bool ai_forced_rule_flag_applies(bool rule_target_is_neutral, bool query_flag) noexcept {
    return rule_target_is_neutral == query_flag;
}

namespace {

// Type ids from the name table 00E0CD80.
constexpr int kFtShip = 0x06;
constexpr int kFtSubmarine = 0x08;
constexpr int kFtCargo = 0x0B;
constexpr int kFtLandingShip = 0x0C;
constexpr int kFtBattleship = 0x0D;
constexpr int kFtTorpedoBoat = 0x0E;
constexpr int kFtPlane = 0x0F;
constexpr int kFtTorpedoBomber = 0x11;
constexpr int kFtDiveBomber = 0x12;
constexpr int kFtFighter = 0x13;
constexpr int kFtLandFort = 0x1B;
constexpr int kFtCommandBuilding = 0x1C;

AiForcedTargetWeightRule forced_pair(int attacker, int target, bool target_class_id,
                                     bool neutral, float weight) {
    AiForcedTargetWeightRule r;
    r.attacker_selector = attacker;
    r.target_selector = target;
    r.target_is_neutral = neutral;
    r.weight = weight;
    r.target_selector_is_class_id = target_class_id;
    return r;
}

// Each script row in order; the script lists most pairs as false then true.
void add_rows(std::vector<AiForcedTargetWeightRule>& t, int attacker, int target,
              bool target_class_id, bool true_first, float weight) {
    t.push_back(forced_pair(attacker, target, target_class_id, true_first, weight));
    t.push_back(forced_pair(attacker, target, target_class_id, !true_first, weight));
}

std::vector<AiForcedTargetWeightRule> island_capture_rules(bool rookie) {
    std::vector<AiForcedTargetWeightRule> t;
    add_rows(t, kFtBattleship, 88, true, false, 0.0f);   // VehicleClass[88], Command Post
    add_rows(t, kFtTorpedoBoat, kFtCommandBuilding, false, false, 0.0f);
    add_rows(t, kFtSubmarine, kFtCommandBuilding, false, false, 0.0f);
    add_rows(t, kFtLandingShip, kFtCommandBuilding, false, false, rookie ? 0.18f : 0.12f);
    if (rookie) add_rows(t, kFtCargo, kFtCommandBuilding, false, false, 1.0f);
    add_rows(t, kFtTorpedoBoat, kFtPlane, false, true, 0.0f);
    add_rows(t, kFtTorpedoBoat, kFtShip, false, true, 0.0f);
    add_rows(t, kFtTorpedoBomber, kFtShip, false, true, 4.5f);
    add_rows(t, kFtDiveBomber, kFtShip, false, true, 5.0f);
    return t;
}

std::vector<AiForcedTargetWeightRule> escort_rules() {
    std::vector<AiForcedTargetWeightRule> t;
    add_rows(t, kFtFighter, kFtShip, false, true, 0.0f);
    add_rows(t, kFtFighter, kFtTorpedoBoat, false, true, 5.0f);
    add_rows(t, kFtTorpedoBomber, kFtFighter, false, true, 0.0f);
    add_rows(t, kFtTorpedoBomber, kFtTorpedoBomber, false, true, 0.0f);
    add_rows(t, kFtDiveBomber, kFtFighter, false, true, 0.0f);
    add_rows(t, kFtDiveBomber, kFtDiveBomber, false, true, 0.0f);
    add_rows(t, kFtFighter, kFtLandFort, false, true, 0.0f);
    return t;
}

} // namespace

const std::vector<AiForcedTargetWeightRule>& ai_shipped_forced_rules(int mode) noexcept {
    static const std::vector<AiForcedTargetWeightRule> rookie = island_capture_rules(true);
    static const std::vector<AiForcedTargetWeightRule> regular = island_capture_rules(false);
    static const std::vector<AiForcedTargetWeightRule> escort = escort_rules();
    static const std::vector<AiForcedTargetWeightRule> none;
    switch (mode) {
    case 0: return rookie;
    case 1:
    case 2: return regular;   // Regular and Veteran are identical
    case 4: return escort;
    default: return none;
    }
}

bool ai_forced_rule_scan_00a31db0(const std::vector<AiForcedTargetWeightRule>& table,
                                  const AiForcedRuleSubject& attacker,
                                  const AiForcedRuleSubject& target, bool query_flag,
                                  float& weight, int& matched_index) {
    int best = 0;   // [ESP+14h], zeroed at 00A31DFF
    matched_index = -1;
    for (std::size_t i = 0; i < table.size(); ++i) {
        const AiForcedTargetWeightRule& r = table[i];
        if (!ai_forced_rule_flag_applies(r.target_is_neutral, query_flag)) continue;
        if (r.relative) continue;   // not projected
        // 00A31E31..00A31E5E: attacker+70h equality scores 2, the type query 1.
        const int a = ai_forced_rule_selector_score(
            r.attacker_selector_is_class_id,
            r.attacker_selector_is_class_id ? r.attacker_selector == attacker.class_id()
                                            : attacker.is_type(r.attacker_selector));
        if (a == 0) continue;
        const int t = ai_forced_rule_selector_score(
            r.target_selector_is_class_id,
            r.target_selector_is_class_id ? r.target_selector == target.class_id()
                                          : target.is_type(r.target_selector));
        const int score = ai_forced_rule_score(a, t);
        if (score <= best) continue;   // 00A31E95 JLE
        weight = r.weight;
        matched_index = static_cast<int>(i);
        best = score;
        if (score == kAiForcedRuleBestScore) break;   // 00A31EBF
    }
    return best != 0;   // 00A31EEB
}

float ai_barrel_time_factor(float damage_calc_time, float barrel_reload) noexcept {
    // 00A0950B COMISS against 00D7A218, JBE to the FLD1 at 00A09531.
    if (barrel_reload <= kAiBarrelReloadEpsilon) {
        return 1.0f;
    }
    return damage_calc_time / barrel_reload;
}

float ai_barrel_damage(float time_factor, float accuracy, int shots) noexcept {
    // 00A09544 FMUL accuracy, 00A09548 FIMUL the integer shot count.
    return time_factor * accuracy * static_cast<float>(shots);
}

float ai_clamp_capture_accumulator(float accumulated, float damage_calc_time) noexcept {
    // 00A09606 FCOMIP / JBE: the larger of the two is replaced by the cap.
    return accumulated > damage_calc_time ? damage_calc_time : accumulated;
}

float ai_clamp_total_damage(float total, float target_hit_points,
                            float max_target_kill_ratio) noexcept {
    // 00A0964F FDIVP then FCOMIP at 00A0965A; only the ratio decides.
    if (target_hit_points == 0.0f) {
        return total;
    }
    if (total / target_hit_points > max_target_kill_ratio) {
        return target_hit_points * max_target_kill_ratio;
    }
    return total;
}

float ai_target_weight_result(float total, float target_hit_points,
                              float max_target_kill_ratio,
                              bool attacker_type_bonus) noexcept {
    // 00A09740 COMISS against zero: a total at or below zero answers zero.
    if (!(total > 0.0f)) {
        return 0.0f;
    }
    if (attacker_type_bonus) {
        total *= kAiAttackerTypeBonus; // 00A09771, double 3.0 at 00D7A2B0
    }
    if (target_hit_points == 0.0f) {
        return 0.0f;
    }
    const float ratio = total / target_hit_points; // 00A09783
    if (ratio < 0.0f) {
        return 0.0f; // 00A0979C FCOMIP / JBE, the negative arm
    }
    return ratio > max_target_kill_ratio ? max_target_kill_ratio : ratio;
}

float ai_expected_hit_damage_009fe200(float low, float high, float armour,
                                      float hit_points) noexcept {
    // 009FE211 FCOMI / JBE: high <= armour answers the 0.0 in the local.
    if (high <= armour) {
        return 0.0f;
    }
    float result;
    if (low >= armour) {
        // 009FE21D..009FE225: (low + high) * 0.5 - armour.
        result = static_cast<float>((static_cast<double>(low) + high) * 0.5 - armour);
    } else {
        // 009FE229..009FE239: (high - armour) / (high - low) * ((high - armour) * 0.5).
        const double over = static_cast<double>(high) - armour;
        result = static_cast<float>(over / (static_cast<double>(high) - low) * (over * 0.5));
    }
    // 009FE245 FCOMI / JBE: the result is capped at the hit points.
    return result > hit_points ? hit_points : result;
}

AiBarrelTargetGates ai_barrel_target_gates(bool neutral, bool plane, bool torpedo_boat,
                                           bool landing_ship, bool ship,
                                           bool submarine) noexcept {
    AiBarrelTargetGates g;
    if (neutral) {
        return g; // 00A0924C / 00A09287 / ...: every flag cleared
    }
    g.soft = plane || torpedo_boat || landing_ship;  // 00A09253..00A09280
    g.flak = plane;                                  // 00A09293..00A092A2
    g.artillery = !plane || submarine;               // 00A092B5..00A092D3
    g.torpedo = ship && !submarine;                  // 00A092E6..00A09304
    g.depth = submarine;                             // 00A09317..00A09324
    return g;
}

bool ai_barrel_gate_admits(const AiBarrelTargetGates& g, int sub_type) noexcept {
    switch (sub_type) {
        case 0x0A: return g.torpedo;                 // 00A093E1
        case 0x04: case 0x05: case 0x06: case 0x07:
        case 0x12: return g.artillery;               // 00A093ED..00A09404 -> 00A09438
        case 0x01: case 0x02: case 0x03: return g.soft;  // 00A09406..00A09415
        case 0x0B: return g.depth;                   // 00A0941C
        case 0x10: return g.flak;                    // 00A09428
        default: return false;                       // 00A0942B JNZ 00A095B7
    }
}

namespace {

// tuning+10h / p1 * p2, stored to a float (00A08C4C), then / the entry's +2Ch
// (00A08C54), stored again.
float ai_plane_option_factor(float damage_calc_time, float p1, float p2, float reload) {
    const float scaled = static_cast<float>(static_cast<double>(damage_calc_time) / p1 * p2);
    return static_cast<float>(static_cast<double>(scaled) / reload);
}

// 00415510 BSP_Math_MinFloatByRef(ECX = a, EDX = b): a when b > a, else b.
float min_by_ref_00415510(float a, float b) {
    return b > a ? a : b;
}

// 00415550 BSP_Math_MaxFloatByRef(ECX = a, EDX = b): a when a > b, else b.
float max_by_ref_00415550(float a, float b) {
    return a > b ? a : b;
}

// The shared body of the torpedo, depth-charge, big-rocket and dive-bomb arms
// (00A0873C..00A0876B and its three copies): the rate ammo / (Params [1] x 0.5
// + the reload minimum) through x87 (FILD, FMUL [00D7A280], FADD, FDIVP ST2 =
// DE FA) stored to [ESP+20h], and the cap Params [2] / Params [1] (FDIVR)
// stored to [ESP+18h] at 00A08A0A.
void loadout_rate_and_cap(int ammo, float p1, float p2, float min_reload, float& rate,
                          float& cap) {
    rate = static_cast<float>(static_cast<double>(ammo) /
                              (kAiLoadoutParamsHalf * static_cast<double>(p1) +
                               static_cast<double>(min_reload)));
    cap = static_cast<float>(static_cast<double>(p2) / static_cast<double>(p1));
}

// 00A08655..00A08A8E with the arm bound. Builds at most one option; `gun_walk`
// and `penalty` carry [ESP+11h] and [ESP+2Bh] out as the arm leaves them.
void ai_plane_loadout_arm(AiTargetWeightModelHost& host, const AiTargetWeightKey& key,
                          const AiModeTuning& tuning, bool target_plane, bool target_sub,
                          std::vector<AiPlaneOption>& options, bool& gun_walk, bool& penalty) {
    // 00A0865A 009552E0(attacker class, +4h); 00A08680 JZ: no list, no option.
    std::vector<AiPlaneLoadoutEntryFacts> entries;
    host.plane_loadout(key.attacker, key.attacker_class, entries);
    // 00A08686..00A086F3: over the entries whose device class answers 25h, the
    // reload minimum (seed 9999.0, 00A086D5 FCOMIP / JBE) and the ammo sum; the
    // first such entry whose 00731040 answers a bullet keeps its device and
    // bullet ([ESP+2Ch], [ESP+44h], 00A086B3 CMP / JNZ).
    float min_reload = kAiLoadoutReloadSeed;
    int ammo = 0;
    const AiPlaneLoadoutEntryFacts* rack = nullptr;
    for (const AiPlaneLoadoutEntryFacts& e : entries) {
        if (!e.device_is_rack) continue;
        if (rack == nullptr && e.bullet.present) rack = &e;
        if (min_reload > e.reload) min_reload = e.reload;
        ammo += e.ammo;
    }
    if (rack == nullptr) {
        host.note_loadout_arm(0, 0u); // 00A086FB JZ 00A08B21
        return;
    }
    const AiPlaneBulletFacts& bullet = rack->bullet;
    const double dct = tuning.damage_calc_time; // tuning +10h
    float rate = 0.0f;      // [ESP+20h]
    float cap = 0.0f;       // [ESP+18h]
    float factor = 0.0f;    // [ESP+18h] after 00A08A28
    bool use_min = true;    // 00A08A13's min, skipped by the 0Fh and level-bomb arms
    std::uint32_t descriptor = 0u;
    switch (bullet.sub_type) {
    case 0x0A: // 00A08701, torpedo
        if (!host.entity_is_type(key.target, kAiTypeShip)) break; // 00A08712
        // 00A08720 COMISS [device+F4h], 0.0 / JA: only above it may a torpedo
        // score against a submarine.
        if (!(rack->device_f4 > 0.0f) && target_sub) break;
        loadout_rate_and_cap(ammo, tuning.torpedo_params_1, tuning.torpedo_params_2,
                             min_reload, rate, cap);
        descriptor = kAiPlaneOptionTorpedo;
        gun_walk = false; // 00A08A0E
        break;
    case 0x0B: // 00A08770, depth charge
        if (!target_sub) break;
        loadout_rate_and_cap(ammo, tuning.d_c_params_1, tuning.d_c_params_2, min_reload, rate,
                             cap);
        descriptor = kAiPlaneOptionDepthCharge;
        gun_walk = false;
        break;
    case 0x0D: // 00A087B4, a carried kamikaze plane
        if (target_plane || target_sub) break;
        // 00A087D3..00A087EB: 1 / LevelbombParams [1] (FDIVRP ST2 = DE F2) to
        // [ESP+18h], 1 / the reload minimum to [ESP+20h].
        cap = static_cast<float>(1.0 / static_cast<double>(tuning.levelbomb_params_1));
        rate = static_cast<float>(1.0 / static_cast<double>(min_reload));
        descriptor = kAiPlaneOptionKamikaze;
        gun_walk = false;
        break;
    case 0x0F: { // 00A087F4, paratroopers
        if (!host.entity_is_type(key.target, kAiTypeCommandBuildingTarget)) break; // 00A08802
        const float fammo = static_cast<float>(ammo);  // [ESP+3Ch]
        const double half = static_cast<double>(tuning.levelbomb_params_1) * kAiLoadoutParamsHalf;
        gun_walk = false;                              // 00A08831
        // 00A08840 FADD ST0,ST2 / 00A08846 FDIVP (DE F9): ammo / (min + half).
        rate = static_cast<float>(static_cast<double>(fammo) /
                                  (static_cast<double>(min_reload) + half));
        const float slack = static_cast<float>(dct - half); // 00A0884C FSUBR [+10h]
        // 00A08853 00415550(0.0, slack), x rate, + ammo.
        factor = static_cast<float>(
            static_cast<double>(max_by_ref_00415550(0.0f, slack)) * static_cast<double>(rate) +
            static_cast<double>(fammo));
        use_min = false;
        descriptor = kAiPlaneOptionLevelBomb;
        break;
    }
    case 0x12: // 00A08865, rocket
        if (bullet.ignition_delay <= 0.0f) {
            // 006E3260 true (IgnitionDelay <= 0): the small rocket. 00A08882
            // clears [ESP+2Bh] before either test can fail.
            penalty = false;
            const bool same = host.rocket_air_ground_same();
            const bool air = target_plane && (bullet.anti_air || same);          // 007B80A0
            const bool ground = (!bullet.anti_air || same) && !target_plane && !target_sub; // 007B80C0
            if (!air && !ground) break; // 00A088C4
            rate = static_cast<float>(static_cast<double>(ammo) / static_cast<double>(min_reload));
            gun_walk = true; // 00A088E0: the gun walk still runs
            cap = target_plane
                      ? static_cast<float>(static_cast<double>(tuning.dogfight_params_3) /
                                           tuning.dogfight_params_1) // 00A088EB
                      : static_cast<float>(static_cast<double>(tuning.strafe_params_3) /
                                           tuning.strafe_params_1);  // 00A088FA
        } else {
            if (target_plane || target_sub) break; // 00A08909..00A08919
            loadout_rate_and_cap(ammo, tuning.big_rocket_params_1, tuning.big_rocket_params_2,
                                 min_reload, rate, cap);
            gun_walk = false;
        }
        descriptor = kAiPlaneOptionRocket;
        break;
    case 0x09: // 00A08956, bomb
        if (target_plane || target_sub) break;
        if (host.entity_is_type(key.attacker, kAiTypeLevelBomber)) { // 00A08980
            if (!host.entity_is_type(key.target, kAiTypeShip)) break; // 00A0898F
            // 009FF3A0(ECX = ammo, LevelbombParams [2]): (int) min, ties to the
            // parameter (FCOMIP / JBE, CVTTSS2SI).
            const float fa = static_cast<float>(ammo);
            const float p2 = tuning.levelbomb_params_2;
            const int drops = static_cast<int>(p2 > fa ? fa : p2);
            gun_walk = false; // 00A089AD
            rate = static_cast<float>(static_cast<double>(drops) /
                                      (static_cast<double>(tuning.levelbomb_params_1) *
                                           kAiLoadoutParamsHalf +
                                       static_cast<double>(min_reload)));
            factor = static_cast<float>(dct * static_cast<double>(rate)); // 00A089D2
            use_min = false;
            descriptor = kAiPlaneOptionLevelBomb;
        } else {
            loadout_rate_and_cap(ammo, tuning.divebomb_params_1, tuning.divebomb_params_2,
                                 min_reload, rate, cap);
            gun_walk = false;
            descriptor = kAiPlaneOptionDiveBomb;
        }
        break;
    default: // 00A08959 JNZ 00A08B21
        break;
    }
    if (descriptor != 0u && use_min) {
        // 00A08A1B 00415510(&[ESP+20h], &[ESP+18h]) x DamageCalcTime (00A08A20).
        factor = static_cast<float>(static_cast<double>(min_by_ref_00415510(rate, cap)) * dct);
    }
    // 00A08A23: a target with +1Ch = 1 admits only a paratrooper option.
    if (descriptor != 0u && key.target_is_neutral == 1 && bullet.sub_type != 0x0F) {
        descriptor = 0u;
    }
    host.note_loadout_arm(bullet.sub_type, descriptor);
    if (descriptor == 0u) return;
    // 00A08A3C..00A08A8B: one 14h record, +0h device, +4h bullet, +10h factor,
    // +0Ch descriptor; 00A08B18 EBX = 1.
    AiPlaneOption o;
    o.descriptor = descriptor;
    o.bullet = bullet;
    o.factor = factor;
    options.push_back(o);
}

} // namespace

std::vector<AiPlaneOption> ai_plane_attack_options(AiTargetWeightModelHost& host,
                                                   const AiTargetWeightKey& key,
                                                   const AiModeTuning& tuning,
                                                   bool* equipment_penalty) {
    std::vector<AiPlaneOption> options;
    const bool target_plane = host.entity_is_type(key.target, kAiTypeQueryAttackerModel); // 00A08628
    const bool target_sub = host.entity_is_type(key.target, kAiTypeSubmarine);            // 00A08637
    bool gun_walk = key.target_is_neutral != 1; // [ESP+11h], 00A08641
    bool penalty = key.attacker_class > 0;      // [ESP+2Bh], 00A08648 SETG
    if (equipment_penalty != nullptr) *equipment_penalty = penalty;
    if (key.attacker_class > 0) {
        if (!host.plane_loadout_arm_bound()) {
            // The loadout arm unbound: no option, the earlier behaviour.
            host.note_plane_arm(true, true);
            return options;
        }
        ai_plane_loadout_arm(host, key, tuning, target_plane, target_sub, options, gun_walk,
                             penalty);
        if (equipment_penalty != nullptr) *equipment_penalty = penalty;
    } else if (target_sub) {
        gun_walk = false; // 00A08A9A
    } else if (host.entity_is_type(key.attacker, kAiTypeKamikazePlane) && !target_plane) {
        // 00A08ABB..00A08B13: one kamikaze option, factor 1.0 [00D7A24C].
        AiPlaneOption o;
        o.descriptor = kAiPlaneOptionKamikaze;
        o.factor = 1.0f;
        options.push_back(o);
        gun_walk = false;
    }
    if (gun_walk) {
        // 00A08B36..00A08D40, the attacker class's platforms in slot order.
        bool pilot_option = false;  // [ESP+12h], cleared at 00A08B26
        bool tail_seen = false;     // [ESP+1Fh], cleared at 00A08B2B
        // EBX, also kept in [ESP+14h] at 00A08B41: 1 after a loadout option
        // (00A08B18, the small rocket's), else 0.
        const std::size_t base = options.size();
        std::size_t count = base;
        const int platforms = host.plane_platform_count(key.attacker);
        for (int i = 0; i < platforms; ++i) {
            AiPlanePlatformFacts p;
            if (!host.plane_platform(key.attacker, i, p) || !p.present) continue;
            if (!p.has_default_gun || p.gun_count != 1 || !p.device_is_gun) continue;
            if (!p.bullet.present) continue;
            if (p.pilot_fires) {
                // 00A08BF3 CMOVNZ EBX,[ESP+14h]: once a tail gun was seen the
                // pilot option overwrites from the walk's first slot.
                if (tail_seen) count = base;
                options.resize(count + 1); // 00A08C01 -> 00A07A60
                pilot_option = true;
                AiPlaneOption& o = options[count];
                o.bullet = p.bullet;
                if (target_plane) {
                    o.descriptor = kAiPlaneOptionDogfight; // 00A08C38, +28h/+2Ch
                    o.factor = ai_plane_option_factor(tuning.damage_calc_time,
                                                      tuning.dogfight_params_1,
                                                      tuning.dogfight_params_2, p.reload);
                } else {
                    o.descriptor = kAiPlaneOptionStrafe; // 00A08C6C, +34h/+38h
                    o.factor = ai_plane_option_factor(tuning.damage_calc_time,
                                                      tuning.strafe_params_1,
                                                      tuning.strafe_params_2, p.reload);
                }
                ++count;
            } else if (p.device_function == kAiPlaneTailGunFunction) {
                tail_seen = true; // 00A08CAE, set before the two tests
                if (target_plane && !pilot_option) {
                    options.resize(count + 1);
                    AiPlaneOption& o = options[count];
                    o.bullet = p.bullet;
                    o.descriptor = kAiPlaneOptionDogfight; // 00A08D1C, +40h/+44h
                    o.factor = ai_plane_option_factor(tuning.damage_calc_time,
                                                      tuning.tail_gun_params_1,
                                                      tuning.tail_gun_params_2, p.reload);
                    ++count;
                }
            }
        }
        options.resize(count);
    }
    // 00A08D4F: a 14h class (recon) keeps no option.
    if (host.entity_is_type(key.attacker, kAiTypeQueryAttackerNoBonus)) options.clear();
    host.note_plane_arm(false, options.empty());
    return options;
}

float ai_plane_attack_total(AiTargetWeightModelHost& host, const AiTargetWeightKey& key,
                            const AiModeTuning& tuning, float target_hit_points,
                            float target_armour, float target_underwater_armour) {
    bool loadout = false; // [ESP+2Bh]
    const std::vector<AiPlaneOption> options =
        ai_plane_attack_options(host, key, tuning, &loadout);
    float total = 0.0f;   // [ESP+24h]
    for (const AiPlaneOption& o : options) {
        float value = 0.0f;
        if (o.descriptor == kAiPlaneOptionKamikaze) {
            // 00A08DC8..00A08E74: class+210h's blast pair, 009FE270, x +10h.
            // 00A08DE4: with a bullet in +4h (the loadout arm's carried
            // kamikaze plane, 0Dh) the class is [bullet+DCh], not the attacker.
            AiPlaneBulletFacts b;
            bool known = false;
            if (o.bullet.present) {
                known = o.bullet.carried_kamikaze_known;
                b.present = known;
                b.sub_type = o.bullet.carried_sub_type;
                b.blast_min = o.bullet.carried_blast_min;
                b.blast_max = o.bullet.carried_blast_max;
            } else {
                known = host.kamikaze_bullet(key.attacker, b) && b.present;
            }
            if (known) {
                const float damage = ai_expected_hit_damage_009fe200(
                    b.blast_min, b.blast_max, target_armour, target_hit_points);
                const double accuracy = host.bullet_accuracy(key.attacker, b.sub_type, key.target);
                value = static_cast<float>(accuracy * o.factor * damage);
            }
        } else if (o.descriptor == kAiPlaneOptionLevelBomb && o.bullet.sub_type == 0x0F) {
            // 00A08E81..00A08EDF, paratroopers: Paratroopers accuracy (00A371A0
            // +14Ch) x +10h, then x (+1Ch = 1 ? +D8h : +FCh) x +F8h, added
            // unrounded at 00A09209.
            const float scaled = static_cast<float>(static_cast<double>(o.factor) *
                                                    static_cast<double>(tuning.paratroopers));
            double v = 0.0;
            if (o.bullet.paratrooper_terms_known) {
                const float first = key.target_is_neutral == 1 ? o.bullet.paratrooper_d8
                                                               : o.bullet.paratrooper_fc;
                v = static_cast<double>(first) * scaled * o.bullet.paratrooper_f8;
            }
            host.note_plane_option(o.descriptor, static_cast<float>(v));
            total = static_cast<float>(static_cast<double>(total) + v);
            continue;
        } else if (o.descriptor == kAiPlaneOptionDiveBomb ||
                   o.descriptor == kAiPlaneOptionRocket ||
                   o.descriptor == kAiPlaneOptionLevelBomb) {
            // 00A0910B..00A09201: 009FE200(max(DamageMin, BlastMin), max(DamageMax,
            // BlastMax), Armour, HP) x 009FE270 x +10h.
            const float low = max_by_ref_00415550(o.bullet.damage_min, o.bullet.blast_min);
            const float high = max_by_ref_00415550(o.bullet.damage_max, o.bullet.blast_max);
            const float damage =
                ai_expected_hit_damage_009fe200(low, high, target_armour, target_hit_points);
            const double accuracy =
                host.bullet_accuracy(key.attacker, o.bullet.sub_type, key.target);
            value = static_cast<float>(accuracy * o.factor * damage);
        } else if (o.descriptor == kAiPlaneOptionTorpedo ||
                   o.descriptor == kAiPlaneOptionDepthCharge) {
            // 00A0904E..00A09106: the same pair through 00415550, against the
            // underwater armour [ESP+48h]; +10h x 009FE270 x the damage.
            const float high = max_by_ref_00415550(o.bullet.damage_max, o.bullet.blast_max);
            const float low = max_by_ref_00415550(o.bullet.damage_min, o.bullet.blast_min);
            const float damage = ai_expected_hit_damage_009fe200(
                low, high, target_underwater_armour, target_hit_points);
            const double accuracy =
                host.bullet_accuracy(key.attacker, o.bullet.sub_type, key.target);
            value = static_cast<float>(static_cast<double>(o.factor) * accuracy * damage);
        } else if (o.descriptor == kAiPlaneOptionStrafe ||
                   o.descriptor == kAiPlaneOptionDogfight) {
            // 00A08F20..00A08FA8 / 00A08FBD..00A09045.
            const double accuracy =
                host.bullet_accuracy(key.attacker, o.bullet.sub_type, key.target);
            const float damage = ai_expected_hit_damage_009fe200(
                o.bullet.damage_min, o.bullet.damage_max, target_armour, target_hit_points);
            value = static_cast<float>(static_cast<double>(o.factor) * accuracy * damage);
            host.note_plane_option_terms(o.factor, accuracy, damage);
            if (loadout) {
                // 00A08F97 x tuning+8h, DogfightEquipmentPenalty.
                value = static_cast<float>(static_cast<double>(value) *
                                           tuning.dogfight_equipment_penalty);
            }
        }
        host.note_plane_option(o.descriptor, value);
        total = static_cast<float>(static_cast<double>(total) + value); // 00A09209
    }
    return total;
}

void ai_load_globals_00a335d0(AiGlobalsLoaderHost& host) {
    host.run_script(kAiGlobalsInitScript); // 00A33600
    host.run_script(kAiGlobalsDataScript); // 00A3365C
    if (!host.push_field(kAiGlobalsRootTable)) { // 00A338xx
        return;
    }
    for (int mode = 0; mode < kAiGameModeCount; ++mode) {
        const char* table = ai_mode_table_name(mode);
        if (table == nullptr || !host.push_field(table)) {
            continue;
        }
        AiModeTuning* record = host.mode_record(mode);
        float* fields = record == nullptr ? nullptr : &record->mother_ship;
        const char* open_section = nullptr;
        for (const AiModeTuningField& entry : kAiModeTuningFields) {
            if (open_section == nullptr || entry.section != open_section) {
                if (open_section != nullptr) {
                    host.pop_field();
                }
                open_section = entry.section;
                if (!host.push_field(open_section)) {
                    open_section = nullptr;
                    continue;
                }
            }
            if (!host.push_field(entry.key)) {
                continue;
            }
            bool have_value = true;
            if (entry.index != 0) {
                have_value = host.push_index(entry.index);
            }
            if (have_value) {
                const float value = entry.defaulted ? host.read_number_or(entry.fallback)
                                                    : host.read_number();
                if (fields != nullptr) {
                    fields[entry.field] = value;
                }
                if (entry.index != 0) {
                    host.pop_field();
                }
            }
            host.pop_field();
        }
        if (open_section != nullptr) {
            host.pop_field();
        }
        // The guarded store at 00A36126. The key sits in Constants, which the
        // loop above already walked, so this repeats the push pair.
        if (ai_globals_loader_fills_defend_resource_percent(mode) &&
            host.push_field("Constants")) {
            if (host.push_field("Defend_ResourcePercent")) {
                host.store_defend_resource_percent(mode, host.read_number_or(0.35f));
                host.pop_field();
            }
            host.pop_field();
        }
        host.pop_field(); // the mode table
    }
    host.pop_field(); // HighLvlAIGlobals
}

float ai_target_weight_00a08460(AiTargetWeightModelHost& host,
                                const AiTargetWeightKey& key) {
    float memo = 0.0f;
    if (host.memo_lookup(key, memo)) {
        return memo; // 00A084EF, the hit arm at 00A0851A
    }
    if (key.attacker == nullptr || key.target == nullptr) {
        return 0.0f; // 00A08522 / 00A0852A, both to the FLDZ at 00A097F2
    }
    float forced = 0.0f;
    if (host.forced_rule_weight(key, forced)) {
        host.memo_store(key, forced); // 00A08556
        return forced;
    }

    const AiModeTuning& tuning = host.mode_tuning(); // 00A08574
    const float target_hit_points = host.target_hit_points(key.target); // 00A08593
    // 00A085AD. Packet cc9_plane_attacker_weight: the branch at 00A08619 is
    // taken only while the plane arm is bound; unbound, a plane attacker keeps
    // the earlier substitution and walks its barrels like a ship.
    const bool attacker_is_plane = host.entity_is_type(key.attacker, kAiTypeQueryAttackerModel);
    const bool attacker_model_type = host.plane_arm_bound() && attacker_is_plane;

    // 00A085A8 +4Ch into two frame slots; 00A085F8 overwrites one of them with
    // vtable[+24h] when the target class answers vtable[+18h](6).
    const bool terms = host.damage_terms_bound();
    const float class_armour = terms ? host.target_armour(key.target) : 0.0f;
    const float underwater_armour = terms ? host.target_underwater_armour(key.target) : 0.0f;

    float total = 0.0f;
    if (!attacker_model_type) {
        // 00A09228..00A09733. The barrel loop and the water terms.
        // 00A0924C..00A0932B, the per-target gates, bound with the type queries.
        // The queries are asked in both states (the image asks them), so an
        // unbound host's census still sees each one; only the bound host uses
        // the answers.
        const bool gated = host.barrel_target_gates_bound();
        const AiBarrelTargetGates gates = ai_barrel_target_gates(
            key.target_is_neutral == 1,
            host.entity_is_type(key.target, kAiTypeQueryAttackerModel),
            host.entity_is_type(key.target, kAiTypeTorpedoBoat),
            host.entity_is_type(key.target, kAiTypeLandingShip),
            host.entity_is_type(key.target, kAiTypeShip),
            host.entity_is_type(key.target, kAiTypeSubmarine));
        float capture_accumulator = 0.0f;
        const int subsystems = host.subsystem_count(key.attacker);
        for (int index = 0; index < subsystems; ++index) {
            // 00A09379 resolves a real subsystem object and 00A095E3 walks its
            // +74h/+78h barrel array. A host that flattens the barrels onto one
            // subsystem per unit - which is what subsystem_count returning 1
            // means - has to be handed something that resolves back to the
            // attacker, and a null handle is not it: a host keying on the
            // pointer answers no barrels, the loop body never runs, `total`
            // stays 0 and every weight comes out 0. Measured before this line
            // was fixed: IJN01 answered a zero weight for all 465500
            // candidates, and the 4900 that still scored were admitted by
            // 00A146D9's target-group arm rather than by their weight.
            // docs/AI_TARGET_WEIGHT_TERMS.md.
            const void* subsystem = key.attacker;
            float best = 0.0f;
            const int barrels = host.barrel_count(subsystem);
            for (int barrel = 0; barrel < barrels; ++barrel) {
                if (gated) {
                    // 00A09362..00A093A8 per platform, then 00A093E1..00A0943D
                    // per bullet sub-type.
                    if (!host.barrel_platform_admitted(subsystem, barrel)) continue;
                    const int sub_type = host.barrel_sub_type(subsystem, barrel);
                    const bool admitted = ai_barrel_gate_admits(gates, sub_type);
                    host.note_barrel_gate(sub_type, admitted);
                    if (!admitted) continue;
                }
                float low = 0.0f;
                float high = 0.0f;
                float armour = 0.0f;
                if (terms) {
                    // 00A09443: sub-type 0Ah reads the vtable[+24h] slot, every
                    // other sub-type the class Armour copy.
                    armour = host.barrel_sub_type(subsystem, barrel) == 0x0A
                                 ? underwater_armour
                                 : class_armour;
                    low = host.barrel_damage_low(subsystem, barrel);
                    high = host.barrel_damage_high(subsystem, barrel);
                    if (armour > high) {
                        host.note_damage_terms_barrel(true, 0.0f);
                        continue; // 00A094D5 FCOMIP / JA: the barrel cannot pierce
                    }
                }
                const float accuracy = host.barrel_accuracy(subsystem, barrel, key.target);
                if (!(accuracy > 0.0f)) {
                    continue; // 00A094F5 FCOMIP / JNC
                }
                const int shots = host.barrel_shots(subsystem, barrel);
                const float factor =
                    ai_barrel_time_factor(tuning.damage_calc_time,
                                          host.barrel_reload(subsystem, barrel));
                const float damage = ai_barrel_damage(factor, accuracy, shots);
                // 00A09578: 009FE200(low, high, armour, hp). OFF: 1.0.
                const float per_hit =
                    terms ? ai_expected_hit_damage_009fe200(low, high, armour, target_hit_points)
                          : 1.0f;
                if (terms) host.note_damage_terms_barrel(false, per_hit);
                const float weighted = damage * per_hit;
                if (weighted > best) {
                    best = weighted; // 00A09593 FCOMIP / JBE
                }
                // 00A095A9 FMUL [bullet+0BCh] WaterDamage, FADD. OFF: x 1.
                capture_accumulator +=
                    damage * (terms ? host.barrel_water_damage(subsystem, barrel) : 1.0f);
            }
            total += best; // 00A095D8
        }
        capture_accumulator =
            ai_clamp_capture_accumulator(capture_accumulator, tuning.damage_calc_time);
        // 00A09624 settings+3B0h WaterTickDamage. OFF: 1.0.
        total += (terms ? host.water_damage_scale() : 1.0f) * capture_accumulator; // 00A09629
        total = ai_clamp_total_damage(total, target_hit_points, tuning.max_target_kill_ratio);
    }
    else {
        // 00A0861F..00A09222; the loadout arm behind plane_loadout_arm_bound.
        total = ai_plane_attack_total(host, key, tuning, target_hit_points,
                                      host.target_armour(key.target),
                                      host.target_underwater_armour(key.target));
    }

    const bool bonus = attacker_model_type &&
                       !host.entity_is_type(key.attacker, kAiTypeQueryAttackerNoBonus);
    const float result = ai_target_weight_result(total, target_hit_points,
                                                 tuning.max_target_kill_ratio, bonus);
    host.memo_store(key, result); // 00A097CC
    return result;
}

} // namespace bsp
