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
    {AiTuningMode::IslandCaptureRookie,  2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::IslandCaptureRegular, 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::IslandCaptureVeteran, 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::Duel,                 2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::Escort,               2.0f, 9000.0f, 15000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::Siege,                2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
    {AiTuningMode::Competitive,          2.0f, 5000.0f, 12000.0f, 2.0f, 650.0f, 1200.0f},
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
    out.loaded = true;
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
    if (std::strcmp(key, "AutoMerge_LeaveDist") == 0) {
        out = row->auto_merge_leave_dist;
        return true;
    }
    return false;
}

}  // namespace bsp
