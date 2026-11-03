#pragma once

// Goal 2, chain 115 (slot g2-w4), segment 59 range sweep. Every wire of this
// chain sits behind this one switch; it lands OFF. reports/wire-g2-c115.json
// lists the rows, their host sites and the rows skipped with their reasons.
namespace bsp {
// ON: the scene reader's explicit S quoted-run value (scene_file.cpp) is read
// by 008EFB00 read_quoted_runs over the host lexer. OFF: the inline run loop.
inline constexpr bool kWireGoal2Chain115Bound = false;
} // namespace bsp
