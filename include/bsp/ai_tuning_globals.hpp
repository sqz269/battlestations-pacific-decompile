#pragma once
// The AI coordinator's tuning block: 00A335D0's loader, its mode selector
// 009FFC80, and the shipped values the coordinator's rules read out of it.
//
// Addresses: 00A335D0 (the loader), 00A371A0 (the reader side, which settles
// the base and stride), 009FFC80 BSP_Ai_EffectiveGameModeIndex (which of the
// seven records a rule reads), 004BCA50 BSP_Game_GetEffectiveGameMode and
// 00A15950 (the difficulty the first arm clamps).
//
// Packet cc8_ai_tuning_globals. docs/AI_TUNING_GLOBALS.md.
//
// docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md read the loader in full and carries the
// whole 143-slot key table. This header does not restate it. It reconstructs
// the part the coordinator's own rules consume, so that GameAiCoordinatorHost
// stops answering zero: the four fields ai_planner_choose_attack_target reads
// and the two the merge phases read.
//
// No name here collides with bsp/ai_group_think.hpp, bsp/ai_planners.hpp or
// bsp/ai_command_lifetime.hpp; every name is prefixed `ai_tuning_` or
// `AiTuning`.

#include <cstddef>
#include <cstdint>

namespace bsp {

// 00A338DD IMUL EDX,EDX,23Ch and 00A338E3 LEA ESI,[EDX+ECX], against 00A371B3
// LEA EAX,[EAX+ESI*1+4]: seven records of 23Ch bytes starting at
// coordinator+4, so the loader's store at +004h is the record's +000h.
inline constexpr std::size_t kAiTuningRecordStride = 0x23C;
inline constexpr std::size_t kAiTuningRecordBase = 0x4;
inline constexpr int kAiTuningModeCount = 7;

// The seven sub-tables 00A3379D..0080385C selects, in the loader's own loop
// order (XOR EDI,EDI at 00A3374B, ADD EDI,1 / CMP EDI,7 / JL at 00A370D4).
enum class AiTuningMode : int {
    IslandCaptureRookie = 0,   // "IslandCaptureParams_Rookie"
    IslandCaptureRegular = 1,  // "IslandCaptureParams_Regular"
    IslandCaptureVeteran = 2,  // "IslandCaptureParams_Veteran"
    Duel = 3,                  // "DuelParams"
    Escort = 4,                // "EscortParams"
    Siege = 5,                 // "SiegeParams"
    Competitive = 6,           // "CompetitiveParams"
};

const char* ai_tuning_mode_table_name(AiTuningMode mode) noexcept;

// 009FFC80. The jump table at 009FFCFC sends effective game modes 0, 1, 2 and 3
// to the difficulty arm at 009FFC9E, which clamps 00A15950's difficulty into
// 0..2; modes 4, 5, 6 and 7 take the literals 3, 4, 5 and 6 at 009FFCD0,
// 009FFCD9, 009FFCE2 and 009FFCEB. A game mode above 7 falls through
// 009FFC95 JA to the zero at 009FFCF4.
AiTuningMode ai_tuning_mode_009ffc80(int effective_game_mode, int difficulty) noexcept;

// ---------------------------------------------------------------------------
// The fields the coordinator's rules read
// ---------------------------------------------------------------------------
//
// Record offsets, which are the loader's store offsets minus 4. The key names
// are the shipped script's, and they settle the four names
// bsp/ai_planners.hpp had to guess: kAiPlannerTuningOwnSetFactor is
// FreeAttack_ObjectiveTargetMul, RangeNear is FreeAttack_NearDist, RangeFar is
// FreeAttack_FarDist and StickyFactor is FreeAttack_ExistingTargetMul.

inline constexpr std::uint32_t kAiTuningFreeAttackObjectiveTargetMul = 0x1CC;
inline constexpr std::uint32_t kAiTuningFreeAttackNearDist = 0x1D0;
inline constexpr std::uint32_t kAiTuningFreeAttackFarDist = 0x1D4;
inline constexpr std::uint32_t kAiTuningFreeAttackExistingTargetMul = 0x1D8;
inline constexpr std::uint32_t kAiTuningCautionMoveDist = 0x1F0;
inline constexpr std::uint32_t kAiTuningCloseAttackCollectDist = 0x1F4;
inline constexpr std::uint32_t kAiTuningCloseAttackNearDist = 0x1F8;
inline constexpr std::uint32_t kAiTuningCloseAttackFarDist = 0x1FC;
inline constexpr std::uint32_t kAiTuningCloseAttackExistingTargetMul = 0x200;
inline constexpr std::uint32_t kAiTuningCloseAttackTargetGroupMemberMul = 0x204;
inline constexpr std::uint32_t kAiTuningAutoMergeMergeDist = 0x208;
inline constexpr std::uint32_t kAiTuningAutoMergeLeaveDist = 0x20C;
inline constexpr std::uint32_t kAiTuningFormationUnitDist = 0x210;

// One key of the reconstructed subset. `image_default` is the float
// BSP_LuaReference_GetFloatOrDefault (00B66330) answers when the key is absent,
// read out of the image at the address named in docs/AI_TUNING_GLOBALS.md.
struct AiTuningKey {
    std::uint32_t record_offset;
    const char* key;
    float image_default;
};

inline constexpr std::size_t kAiTuningKeyCount = 33;
const AiTuningKey* ai_tuning_keys() noexcept;

// The block, addressed by record offset. Every slot outside the reconstructed
// subset stays at zero, which is what an unloaded block holds, so a caller that
// reads one gets the same answer it got before this packet rather than an
// invented number.
struct AiTuningBlock {
    float field[kAiTuningRecordStride / sizeof(float)]{};
    bool loaded{false};
    AiTuningMode mode{AiTuningMode::IslandCaptureRookie};

    float at(std::uint32_t record_offset) const noexcept {
        const std::size_t index = record_offset / sizeof(float);
        if (record_offset % sizeof(float) != 0 ||
            index >= kAiTuningRecordStride / sizeof(float)) {
            return 0.0f;
        }
        return field[index];
    }
};

// What 00A335D0 reads one key with. A host supplies the shipped table, either
// the authored rows below or, when one exists, the live `HighLvlAIGlobals`
// global. `false` means the key is absent, which is the GetFloatOrDefault arm.
struct AiTuningLuaReader {
    virtual ~AiTuningLuaReader() = default;
    virtual bool read_number(AiTuningMode mode, const char* key, float& out) = 0;
};

// 00A335D0's per-record loop over the reconstructed subset: read each key, and
// take the image default when the reader says it is absent.
void ai_tuning_load_00a335d0(AiTuningLuaReader& reader, AiTuningMode mode,
                             AiTuningBlock& out);

// ---------------------------------------------------------------------------
// The shipped content
// ---------------------------------------------------------------------------
//
// scripts/datatables/highlvlaiglobals.lua out of the installed game, in its
// authored form. docs/AI_TUNING_GLOBALS.md carries the provenance, including
// that file's modification date. A reader over these rows is what the host uses
// while nothing in this process runs the script on a Lua state.
struct AiTuningAuthoredRow {
    AiTuningMode mode;
    float free_attack_objective_target_mul;
    float free_attack_near_dist;
    float free_attack_far_dist;
    float free_attack_existing_target_mul;
    float auto_merge_merge_dist;
    float auto_merge_leave_dist;
    // The contiguous GetFloatOrDefault run 00A335D0 loads at +1F0h..+204h and
    // +210h, added by packet cc8_ai_command_inputs so the move and attack ticks
    // read real numbers.
    float caution_move_dist;
    float close_attack_collect_dist;
    float close_attack_near_dist;
    float close_attack_far_dist;
    float close_attack_existing_target_mul;
    float close_attack_target_group_member_mul;
    float formation_unit_dist;
    // +0h..+4Ch, the twenty per-unit-class target weights 009FDF30 indexes.
    // These are GetNumber keys with no image default, so an unloaded block
    // leaves them at zero and every leader weight collapses to zero with it.
    float class_weight[20];
};

// The twenty per-class weight offsets, in record order.
inline constexpr std::uint32_t kAiTuningClassWeightBase = 0x000u;
inline constexpr std::size_t kAiTuningClassWeightCount = 20;

const AiTuningAuthoredRow* ai_tuning_authored_row(AiTuningMode mode) noexcept;

// An AiTuningLuaReader over the rows above.
struct AiTuningAuthoredReader final : AiTuningLuaReader {
    bool read_number(AiTuningMode mode, const char* key, float& out) override;
};

}  // namespace bsp
