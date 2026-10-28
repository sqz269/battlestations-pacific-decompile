#pragma once

// PC24 wave switch, lane "frontend" (cc13 pc24 wave 2, packet P8, cc13-p8-frontend).
// Header-only; later waves of the same lane reuse the switch.

namespace bsp {

// kPc24FrontendCc13Bound: the frontend runs on the main thread, whose x87 runs at
// 24-bit precision control, so every FADD/FSUB/FMUL/FDIV/FIDIV result rounds to 24
// bits (extended exponent). OFF keeps the bodies' double chains that round once;
// ON rounds per x87 op with the include/bsp/cc13_x87_model.hpp helpers. Sites:
//   00530A60 layout_prompt_buttons_00530a60 (frontend_prompt_layout.cpp):
//            gap = (available - total) / spaces, 00530B2E FLD / 00530B37 FSUB /
//            00530B3B FIDIV / 00530B3F FSTP; x = (width + gap) + x, 00530B6C..00530B7A
//            (the first step, FLD ST0 / FADDP ST2 / FADDP ST1) and 00530BAE..00530BBA
//            (FADD gap, FADD x) and the later steps of the same form.
//   00683440 thumb_progress_from_scroll (frontend_screen_animation.cpp):
//            offset / range * travel, 00683461 FLD / 00683464 FDIV / 00683467 FMUL /
//            0068346A FSTP: the quotient rounds before the multiply.
//   006834A0 compute_scroll_bar_geometry (frontend_screen_animation.cpp): the
//            thumb height candidate (track + min_height) - range, 006834FD DE C2
//            FADDP ST2 / 006834FF DE E9 FSUBP ST1 / 00683501 FSTP float.
//   Not switched: 00683820's dragged_thumb_y (00683918) and
//   scroll_offset_from_thumb_y (00683980) have no host caller
//   (update_front_end_screen_scroller is never called), so they are pc24_debt.
// Movers: prompt button x positions, the scroll thumb position and height (pixel
// layout only).
inline constexpr bool kPc24FrontendCc13Bound = false;

}  // namespace bsp
