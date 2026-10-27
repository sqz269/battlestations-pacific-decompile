#pragma once

// PC24 wave 1, packet pc24w1-misc-1. These bodies run on the main thread, whose x87 runs at
// 24-bit precision control: every FADD/FSUB/FMUL/FDIV rounds to a 24-bit significand (with
// the extended exponent) before the next op. ON evaluates each listed chain per op through
// the cc13x87 helpers (include/bsp/cc13_x87_model.hpp); OFF keeps the earlier double chains
// rounded once. One switch per flip lane, so cc9 can pair each separately.
namespace bsp {

// AI planners: src/ai_close_attack_tick.cpp (00A13B60), src/ai_command_tick.cpp (00A14DD0,
// 004F2F40, 00A11690), src/ai_planner_tails.cpp (009FFC10), src/ai_target_weights.cpp
// (009FE200, 00A08460). Besides the rounding, ON in 00A13B60 takes two non-rounding edge
// cases from the image: the collection walk drops a candidate at exactly d2 == collect^2
// (JBE at 00A13C81, no root), and the scoring distance is 0 unless d2 > 1e-10 [00CE3820].
// Untouched same-address copies (not under this switch): 00A13B60 in
// src/gameplay_seg69_70_cc13.cpp and 004F2F40 in src/avoid_zone_polygon_partition.cpp.
inline constexpr bool kPc24AiPlannersCc13Bound = false;

// Application frame: src/app_frame_game_state.cpp (00BE34D0 profiler end frame).
inline constexpr bool kPc24AppFrameCc13Bound = false;

// Movie decoder: src/movie_decoder.cpp (00A4CC10 frame seconds).
inline constexpr bool kPc24MovieCc13Bound = false;

}  // namespace bsp
