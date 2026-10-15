#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene path point requires MSVC Win32 x87 and MOVSS.
#endif

namespace bsp {
// Complete 007AF800..007AF8D2, 211B. Original ECX actual scene path,
// stack output/raw uint32 index, RET8/EAX actual output. Source reserves
// an ignored EDX binding word, which is not an Original input.
// Captures path+14 pose once, conditionally refreshes that actual owner,
// captures its +CC matrix, validates current +8/+C slots, then reloads +8
// after returning real SDK validation. Index is captured before validation.
// MOVSS snapshots XYZ plus verified +1.0f, genuine B62D10 transforms into
// real local storage, then the full x87 reciprocal/spill/output order runs.
// No w guard, default pose, repaired index, callback or semantic view.
// Every reached raw entity/slot/point/output/control cell needs backing;
// full dirty pose ancestry must terminate. Canonical pose/matrix providers
// retain their x87 stack requirements. Production leaves FP state intact.
// Current CRT policy and Source stack/frames are not Native ABI/game proof.
void* __fastcall transform_native_scene_path_point_007af800(
    void* actual_scene_path, void* unused_edx, void* actual_output_float3,
    std::uint32_t index);
} // namespace bsp
