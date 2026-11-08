// Focused Win32 probe over the production gameplay/tuning sources.
// After scripts/build.ps1, from an x86 MSVC developer prompt at the repo root:
// cl /std:c++17 /EHsc /MD /O2 /Gy /W4 /WX /fp:strict /Iinclude
//   tests/wire_g5_c861_min_speed_probe.cpp src/gameplay_seg69_70_cc13.cpp
//   src/ai_tuning_globals.cpp src/band_009f_cc13.cpp src/ai_planners.cpp
//   build/win32/Release/bsp_core.lib /Febuild/win32/c861_probe.exe
//   /link /MANIFEST:EMBED /OPT:REF
#include "bsp/ai_tuning_globals.hpp"
#include "bsp/gameplay_seg69_70_cc13.hpp"

#include <cstdio>
#include <vector>

namespace {
struct MissingKeys final : bsp::AiTuningLuaReader {
    bool read_number(bsp::AiTuningMode, const char*, float&) override { return false; }
};
}

int main() {
    // A mixed ship/plane group must use its slowest class, not its fastest.
    using namespace bsp::gameplay_seg69_70_cc13;
    const std::vector<SpeedClassView> classes{
        {true, 34.0f, false, 0.0f},
        {false, 0.0f, true, 75.0f},
        {false, 0.0f, true, 20.0f},
    };
    if (ai_unit_list_min_speed_00a02180(classes) != 20.0f ||
        ai_unit_list_min_speed_00a02180({}) != 1.0e10f) return 1;

    // Authored zero is a present key; absence uses the binary's nonzero
    // defaults. This distinguishes loading these fields from zeroed padding.
    bsp::AiTuningAuthoredReader authored;
    for (int mode = 0; mode < bsp::kAiTuningModeCount; ++mode) {
        bsp::AiTuningBlock block;
        bsp::ai_tuning_load_00a335d0(authored, static_cast<bsp::AiTuningMode>(mode), block);
        if (block.at(bsp::kAiTuningComposeGroupSpeedBonusWeightRatio) != 0.0f ||
            block.at(bsp::kAiTuningComposeGroupSpeedBonus) != 0.0f) return 2;
    }
    MissingKeys missing;
    bsp::AiTuningBlock fallback;
    bsp::ai_tuning_load_00a335d0(missing, bsp::AiTuningMode::IslandCaptureRegular, fallback);
    if (fallback.at(bsp::kAiTuningComposeGroupSpeedBonusWeightRatio) != 0.5f ||
        fallback.at(bsp::kAiTuningComposeGroupSpeedBonus) != 1.0f) return 3;
    std::puts("c861: mixed-class minimum, empty seed, seven authored modes and missing-key defaults passed");
    return 0;
}
