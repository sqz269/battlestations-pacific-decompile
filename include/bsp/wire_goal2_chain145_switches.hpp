#pragma once

// Goal 2, chain 145 (slot g2-w1), segment 61 range sweep. Every wire of this
// chain sits behind this one switch; it lands OFF. reports/wire-g2-c145.json
// lists the rows, their host sites and the rows skipped with their reasons.
namespace bsp {
// ON: the scene reader's explicit S quoted-run value (scene_file.cpp) is read
// by 008EFB00 and built in its chunk-list text writer (009502E0 reset,
// 0094FF10 write, 009500A0 join) instead of one std::string.
// OFF: chain 115's arm and the inline run loop.
inline constexpr bool kWireGoal2Chain145Bound = true;
} // namespace bsp
