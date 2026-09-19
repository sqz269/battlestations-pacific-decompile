// The AI coordinator's tuning block. docs/AI_TUNING_GLOBALS.md carries the
// listing and the provenance of the authored rows.

#include "bsp/ai_tuning_globals.hpp"

#include <cstring>

namespace bsp {
namespace {

// The six keys this packet reconstructs, with the float
// BSP_LuaReference_GetFloatOrDefault answers when the key is absent. The
// default addresses are docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md's, section
// "Key, slot, destination and default".
constexpr AiTuningKey kKeys[kAiTuningKeyCount] = {
    {kAiTuningFreeAttackObjectiveTargetMul, "FreeAttack_ObjectiveTargetMul", 2.0f},   // 00CE3958
    {kAiTuningFreeAttackNearDist,           "FreeAttack_NearDist",           5000.0f}, // 00D1AF84
    {kAiTuningFreeAttackFarDist,            "FreeAttack_FarDist",            12000.0f},// 00CE3968
    {kAiTuningFreeAttackExistingTargetMul,  "FreeAttack_ExistingTargetMul",  1.5f},   // 00CE380C
    {kAiTuningAutoMergeMergeDist,           "AutoMerge_MergeDist",           650.0f}, // 00D20180
    {kAiTuningAutoMergeLeaveDist,           "AutoMerge_LeaveDist",           1200.0f},// 00CFD714
    {kAiTuningCautionMoveDist,              "CautionMove_Dist",              8000.0f},// 00D02F60
    {kAiTuningCloseAttackCollectDist,       "CloseAttack_CollectDist",       5000.0f},// 00D1AF84
    {kAiTuningCloseAttackNearDist,          "CloseAttack_NearDist",          3000.0f},// 00CFA424
    {kAiTuningCloseAttackFarDist,           "CloseAttack_FarDist",           8000.0f},// 00D02F60
    {kAiTuningCloseAttackExistingTargetMul, "CloseAttack_ExistingTargetMul", 1.5f},   // 00CE380C
    {kAiTuningCloseAttackTargetGroupMemberMul,
                                            "CloseAttack_TargetGroupMemberMul", 2.0f},// 00CE3958
    {kAiTuningFormationUnitDist,            "Formation_UnitDist",            300.0f}, // 00CE3AE8
    // +0h..+4Ch, GetNumber with no image default; an absent key leaves zero.
    {0x000u, "MotherShip", 0.0f},
    {0x004u, "BattleShip", 0.0f},
    {0x008u, "CommandBuilding", 0.0f},
    {0x00Cu, "Landfort", 0.0f},
    {0x010u, "Cruiser", 0.0f},
    {0x014u, "Destroyer", 0.0f},
    {0x018u, "Submarine", 0.0f},
    {0x01Cu, "LandingShip", 0.0f},
    {0x020u, "Cargo", 0.0f},
    {0x024u, "TBoat", 0.0f},
    {0x028u, "LevelBomber", 0.0f},
    {0x02Cu, "KamikazePlane", 0.0f},
    {0x030u, "TorpedoBomber", 0.0f},
    {0x034u, "DiveBomber", 0.0f},
    {0x038u, "Fighter", 0.0f},
    {0x03Cu, "ReconPlaneSmall", 0.0f},
    {0x040u, "ReconPlaneLarge", 0.0f},
    {0x044u, "OtherShip", 0.0f},
    {0x048u, "OtherPlane", 0.0f},
    {0x04Cu, "Other", 0.0f},
};

// scripts/datatables/highlvlaiglobals.lua of the installed game, 1194 lines,
// modified 2024-07-13, which is that installation's untouched bulk date. The
// seven sub-tables appear in the file in the loader's own mode order at lines
// 3, 191, 377, 563, 718, 887 and 1042.
//
// Two of the six keys are NOT the image default in the shipped table, which is
// why reading the script matters and a default-only block would be wrong:
// FreeAttack_ExistingTargetMul is 2 in every mode against a 1.5 default, and
// EscortParams widens FreeAttack_NearDist to 9000 and FreeAttack_FarDist to
// 15000 against 5000 and 12000.
constexpr AiTuningAuthoredRow kAuthored[kAiTuningModeCount] = {
    // The seven trailing values come from the same sub-tables, at script lines
    // 112-125, 300-313, 486-499, 670-683, 825-838, 994-1007 and 1149-1162.
    // Three diverge from the image default: CloseAttack_CollectDist is 3000
    // against 5000 everywhere but Escort, CloseAttack_ExistingTargetMul is 1.8
    // against 1.5, CloseAttack_TargetGroupMemberMul is 10 against 2, and
    // IslandCaptureRookie widens CautionMove_Dist to 40000 against 8000.
    {AiTuningMode::IslandCaptureRookie,  2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     40000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 500.0f,
     {25.0f, 12.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 1.0f, 2.0f, 0.001f, 2.0f, 2.0f, 8.0f, 8.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::IslandCaptureRegular, 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 500.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 1.0f, 2.0f, 0.001f, 2.0f, 2.0f, 8.0f, 8.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::IslandCaptureVeteran, 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 500.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 1.0f, 2.0f, 0.001f, 2.0f, 2.0f, 8.0f, 8.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::Duel,                 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 300.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 1.5f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::Escort,               2.0f, 9000.0f, 15000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 6000.0f, 6000.0f, 12000.0f, 1.8f, 10.0f, 300.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 8.0f, 8.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::Siege,                2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 300.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 1.5f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
    {AiTuningMode::Competitive,          2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f,
     8000.0f, 3000.0f, 3000.0f, 6000.0f, 1.8f, 10.0f, 300.0f,
     {25.0f, 20.0f, 15.0f, 1.0f, 12.0f, 8.0f, 4.0f, 2.0f, 2.0f, 2.0f, 2.0f, 2.0f, 1.5f, 1.5f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f}},
};

bool mode_in_range(AiTuningMode mode) noexcept {
    const int value = static_cast<int>(mode);
    return value >= 0 && value < kAiTuningModeCount;
}

}  // namespace

const char* ai_tuning_mode_table_name(AiTuningMode mode) noexcept {
    switch (mode) {
    case AiTuningMode::IslandCaptureRookie:  return "IslandCaptureParams_Rookie";
    case AiTuningMode::IslandCaptureRegular: return "IslandCaptureParams_Regular";
    case AiTuningMode::IslandCaptureVeteran: return "IslandCaptureParams_Veteran";
    case AiTuningMode::Duel:                 return "DuelParams";
    case AiTuningMode::Escort:               return "EscortParams";
    case AiTuningMode::Siege:                return "SiegeParams";
    case AiTuningMode::Competitive:          return "CompetitiveParams";
    }
    return "IslandCaptureParams_Rookie";
}

AiTuningMode ai_tuning_mode_009ffc80(int effective_game_mode, int difficulty) noexcept {
    // 009FFC92 CMP EAX,7 / 009FFC95 JA leaves the zero ESI was set to.
    if (effective_game_mode < 0 || effective_game_mode > 7) {
        return AiTuningMode::IslandCaptureRookie;
    }
    switch (effective_game_mode) {
    case 4: return AiTuningMode::Duel;         // 009FFCD0, literal 3
    case 5: return AiTuningMode::Escort;       // 009FFCD9, literal 4
    case 6: return AiTuningMode::Siege;        // 009FFCE2, literal 5
    case 7: return AiTuningMode::Competitive;  // 009FFCEB, literal 6
    default: break;                            // 0, 1, 2, 3 share 009FFC9E
    }
    // 009FFC9E's arm: 00A15950's difficulty, clamped into the three
    // IslandCapture records.
    int clamped = difficulty;
    if (clamped < 0) clamped = 0;
    if (clamped > 2) clamped = 2;
    return static_cast<AiTuningMode>(clamped);
}

const AiTuningKey* ai_tuning_keys() noexcept { return kKeys; }

void ai_tuning_load_00a335d0(AiTuningLuaReader& reader, AiTuningMode mode,
                             AiTuningBlock& out) {
    out = AiTuningBlock{};
    out.mode = mode;
    for (std::size_t i = 0; i < kAiTuningKeyCount; ++i) {
        const AiTuningKey& key = kKeys[i];
        float value = key.image_default;
        // 00B66330 BSP_LuaReference_GetFloatOrDefault: the supplied float when
        // the key is absent, the script's number when it is present.
        if (!reader.read_number(mode, key.key, value)) {
            value = key.image_default;
        }
        out.field[key.record_offset / sizeof(float)] = value;
    }
    // 00A335D0 loads the whole record, so the BulletTypeAccuracy block belongs
    // in the same pass; it only takes a different read path because its entries
    // are Lua array elements. docs/AI_TARGET_WEIGHT_TERMS.md.
    ai_tuning_load_bullet_type_accuracy_00a335d0(mode, out);
    ai_tuning_load_attacker_vs_target_00a335d0(mode, out);
    out.loaded = true;
}

// The BulletTypeAccuracy block out of scripts/datatables/highlvlaiglobals.lua,
// in record order: MachineGun[1..4] at 110h, Artillery[1..4] at 120h,
// Bomb[1..4] at 130h, Torpedo[1..2] at 140h, DepthCharge at 148h, Paratroopers
// at 14Ch, Kamikaze[1..4] at 150h, SmallRocket[1..4] at 160h, BigRocket[1..4]
// at 170h, Flak[1..4] at 180h. The four-entry rows are indexed
// Repulore/Kishajora/Nagyhajora/Landfortra, the script's own comment.
//
// All seven mode tables are byte-identical but for three cells, so they are two
// variants rather than seven rows: the three IslandCapture tables (script lines
// 72, 260, 446) hold MachineGun[4] 0.0, DepthCharge 0.50 and Paratroopers 0.30,
// and Duel, Escort, Siege and Competitive (632, 787, 956, 1111) hold 0.1, 0.20
// and 0.4. Diffed across all seven, not sampled.
constexpr std::size_t kAccCount = kAiTuningBulletTypeAccuracyCount;
constexpr float kAccIslandCapture[kAccCount] = {
    0.10f, 0.15f, 0.00f, 0.00f,   // MachineGun
    0.00f, 0.50f, 0.70f, 0.55f,   // Artillery
    0.00f, 0.50f, 0.70f, 0.20f,   // Bomb
    0.75f, 0.45f,                 // Torpedo: ship, submarine
    0.50f,                        // DepthCharge
    0.30f,                        // Paratroopers
    0.00f, 0.50f, 0.70f, 0.80f,   // Kamikaze
    0.15f, 0.25f, 0.70f, 0.50f,   // SmallRocket
    0.00f, 0.25f, 0.90f, 0.75f,   // BigRocket
    0.50f, 0.20f, 0.00f, 0.00f,   // Flak
};
constexpr float kAccOtherModes[kAccCount] = {
    0.10f, 0.15f, 0.00f, 0.10f,   // MachineGun[4] differs
    0.00f, 0.50f, 0.70f, 0.55f,
    0.00f, 0.50f, 0.70f, 0.20f,
    0.75f, 0.45f,
    0.20f,                        // DepthCharge differs
    0.40f,                        // Paratroopers differs
    0.00f, 0.50f, 0.70f, 0.80f,
    0.15f, 0.25f, 0.70f, 0.50f,
    0.00f, 0.25f, 0.90f, 0.75f,
    0.50f, 0.20f, 0.00f, 0.00f,
};

void ai_tuning_load_bullet_type_accuracy_00a335d0(AiTuningMode mode,
                                                  AiTuningBlock& out) noexcept {
    const bool island_capture = mode == AiTuningMode::IslandCaptureRookie ||
                                mode == AiTuningMode::IslandCaptureRegular ||
                                mode == AiTuningMode::IslandCaptureVeteran;
    const float* values = island_capture ? kAccIslandCapture : kAccOtherModes;
    for (std::size_t i = 0; i < kAccCount; ++i) {
        const std::uint32_t offset =
            kAiTuningBulletTypeAccuracyBase + static_cast<std::uint32_t>(i * 4);
        out.field[offset / sizeof(float)] = values[i];
    }
}

void ai_tuning_load_attacker_vs_target_00a335d0(AiTuningMode mode,
                                                AiTuningBlock& out) noexcept {
    // Script lines 33-34, 221-222, 407-408, 593-594, 748-749, 917-918 and
    // 1072-1073: every one of the seven mode tables authors DamageCalcTime 60
    // and MaxTargetKillRatio 150.0, so there is no per-mode variation to carry.
    // Diffed across all seven, not sampled.
    (void)mode;
    out.field[kAiTuningMaxTargetKillRatio / sizeof(float)] = 150.0f;
    out.field[kAiTuningDamageCalcTime / sizeof(float)] = 60.0f;
}

std::uint32_t ai_bullet_type_accuracy_offset_009fe270(int sub_type,
                                                      AiAccuracyTargetGroup group,
                                                      bool& resolved) noexcept {
    resolved = true;
    // 009FE288 SUB 2 / 009FE28E JA 10h, then the byte table at 009FE6EC picks
    // the arm. Sub-types 1 and 4 never reach here on a live projectile: they
    // are the constructor defaults 006E9890 rewrites into 2/3 and 5/6/7.
    // A four-entry arm indexes Plane, SmallShip, BigShip, Other in that order.
    auto four = [group](std::uint32_t base) -> std::uint32_t {
        switch (group) {
        case AiAccuracyTargetGroup::Plane:     return base + 0x0u;
        case AiAccuracyTargetGroup::SmallShip: return base + 0x4u;
        // 009FE2C8 PUSH 6 admits the ship base and 009FE2D4 00827F70 splits it;
        // a submarine is ship-base and is not small surface, so it lands here.
        case AiAccuracyTargetGroup::Submarine: return base + 0x8u;
        case AiAccuracyTargetGroup::BigShip:   return base + 0x8u;
        case AiAccuracyTargetGroup::Other:     return base + 0xCu;
        }
        return 0u;
    };
    switch (sub_type) {
    case 0x02: case 0x03:            return four(0x110u);  // 009FE2A2 MachineGun
    case 0x05: case 0x06: case 0x07: return four(0x120u);  // 009FE313 Artillery
    case 0x09:                       return four(0x130u);  // 009FE384 Bomb
    case 0x0A:
        // 009FE3F5, read whole: PUSH 8 answers Torpedo[2] at 144h; otherwise
        // PUSH 6 must hold or it rejects, and 00827F70 true ALSO rejects, so a
        // torpedo has no accuracy against a torpedo boat or small landing ship.
        switch (group) {
        case AiAccuracyTargetGroup::Submarine: return 0x144u;
        case AiAccuracyTargetGroup::BigShip:   return 0x140u;
        default:                               return 0u;
        }
    case 0x0B:                       return 0x148u;  // 009FE44A DepthCharge, unconditional
    case 0x0F:                       return 0x14Cu;  // 009FE465 Paratroopers, unconditional
    case 0x0D: case 0x11:            return four(0x150u);  // 009FE64A Kamikaze
    case 0x10:                       return four(0x180u);  // 009FE480 Flak
    case 0x12:
        // 009FE4F1 reaches SmallRocket at 160h and BigRocket at 170h through a
        // chain of target-state predicates (006E3260, 007B80A0, 007B80C0) that
        // this packet did not read, so the pair cannot be chosen here.
        resolved = false;
        return 0u;
    default:
        // 009FE6BB: 4 and 8 in range, 0Ch DummyTarget, 0Eh DummySubmarine, and
        // 13h WaterMine outside the range check.
        return 0u;
    }
}

const AiTuningAuthoredRow* ai_tuning_authored_row(AiTuningMode mode) noexcept {
    if (!mode_in_range(mode)) return nullptr;
    return &kAuthored[static_cast<std::size_t>(static_cast<int>(mode))];
}

bool AiTuningAuthoredReader::read_number(AiTuningMode mode, const char* key, float& out) {
    const AiTuningAuthoredRow* row = ai_tuning_authored_row(mode);
    if (row == nullptr || key == nullptr) return false;
    if (std::strcmp(key, "FreeAttack_ObjectiveTargetMul") == 0) {
        out = row->free_attack_objective_target_mul;
        return true;
    }
    if (std::strcmp(key, "FreeAttack_NearDist") == 0) {
        out = row->free_attack_near_dist;
        return true;
    }
    if (std::strcmp(key, "FreeAttack_FarDist") == 0) {
        out = row->free_attack_far_dist;
        return true;
    }
    if (std::strcmp(key, "FreeAttack_ExistingTargetMul") == 0) {
        out = row->free_attack_existing_target_mul;
        return true;
    }
    if (std::strcmp(key, "AutoMerge_MergeDist") == 0) {
        out = row->auto_merge_merge_dist;
        return true;
    }
    if (std::strcmp(key, "CautionMove_Dist") == 0) {
        out = row->caution_move_dist;
        return true;
    }
    if (std::strcmp(key, "CloseAttack_CollectDist") == 0) {
        out = row->close_attack_collect_dist;
        return true;
    }
    if (std::strcmp(key, "CloseAttack_NearDist") == 0) {
        out = row->close_attack_near_dist;
        return true;
    }
    if (std::strcmp(key, "CloseAttack_FarDist") == 0) {
        out = row->close_attack_far_dist;
        return true;
    }
    if (std::strcmp(key, "CloseAttack_ExistingTargetMul") == 0) {
        out = row->close_attack_existing_target_mul;
        return true;
    }
    if (std::strcmp(key, "CloseAttack_TargetGroupMemberMul") == 0) {
        out = row->close_attack_target_group_member_mul;
        return true;
    }
    if (std::strcmp(key, "Formation_UnitDist") == 0) {
        out = row->formation_unit_dist;
        return true;
    }
    {
        static const char* const kClassKeys[] = {"MotherShip", "BattleShip", "CommandBuilding", "Landfort", "Cruiser", "Destroyer", "Submarine", "LandingShip", "Cargo", "TBoat", "LevelBomber", "KamikazePlane", "TorpedoBomber", "DiveBomber", "Fighter", "ReconPlaneSmall", "ReconPlaneLarge", "OtherShip", "OtherPlane", "Other"};
        for (std::size_t i = 0; i < 20; ++i) {
            if (std::strcmp(key, kClassKeys[i]) == 0) {
                out = row->class_weight[i];
                return true;
            }
        }
    }
    if (std::strcmp(key, "AutoMerge_LeaveDist") == 0) {
        out = row->auto_merge_leave_dist;
        return true;
    }
    return false;
}

}  // namespace bsp
