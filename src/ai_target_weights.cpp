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
    const bool attacker_model_type =
        host.entity_is_type(key.attacker, kAiTypeQueryAttackerModel); // 00A085AD

    float total = 0.0f;
    if (!attacker_model_type) {
        // 00A09228..00A09733. The barrel loop and the capture terms.
        float capture_accumulator = 0.0f;
        const int subsystems = host.subsystem_count(key.attacker);
        for (int index = 0; index < subsystems; ++index) {
            const void* subsystem = nullptr; // resolved natively at 00A09379
            float best = 0.0f;
            const int barrels = host.barrel_count(subsystem);
            for (int barrel = 0; barrel < barrels; ++barrel) {
                const float accuracy = host.barrel_accuracy(subsystem, barrel, key.target);
                if (!(accuracy > 0.0f)) {
                    continue; // 00A094F5 FCOMIP / JNC
                }
                const int shots = host.barrel_shots(subsystem, barrel);
                const float factor =
                    ai_barrel_time_factor(tuning.damage_calc_time,
                                          host.barrel_reload(subsystem, barrel));
                const float damage = ai_barrel_damage(factor, accuracy, shots);
                const float weighted = damage * host.distance_falloff(0.0f, 0.0f, 0.0f, 0.0f);
                if (weighted > best) {
                    best = weighted; // 00A09593 FCOMIP / JBE
                }
                capture_accumulator += damage; // 00A095A9 FMUL slot+0BCh, FADD
            }
            total += best; // 00A095D8
        }
        capture_accumulator =
            ai_clamp_capture_accumulator(capture_accumulator, tuning.damage_calc_time);
        total += host.capture_scale() * capture_accumulator; // 00A09629
        total = ai_clamp_total_damage(total, target_hit_points, tuning.max_target_kill_ratio);
    }
    // The attacker-is-type-0Fh branch at 00A0861F..00A09222 is not projected.

    const bool bonus = attacker_model_type &&
                       !host.entity_is_type(key.attacker, kAiTypeQueryAttackerNoBonus);
    const float result = ai_target_weight_result(total, target_hit_points,
                                                 tuning.max_target_kill_ratio, bonus);
    host.memo_store(key, result); // 00A097CC
    return result;
}

} // namespace bsp
