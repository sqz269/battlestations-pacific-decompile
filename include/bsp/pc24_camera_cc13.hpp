#pragma once

// PC24 wave 1, lane camera (packet pc24w1-camera-1). The main thread runs the
// x87 at 24-bit precision control, so every FADD/FSUB/FMUL/FDIV in these chains
// rounds to a 24-bit significand (extended exponent) before the next operation.
// OFF keeps the double chains, which round once at the float store; ON
// evaluates each operation with the cc13x87 x87_*24 helpers in the listing's
// order. Camera pose, except 007AF150: GameUnitsHost::bind_land_convoy_motion
// builds land convoy paths with it, so convoy knots move too.
//
// Sites (file, address, listing range):
// - cc13_camera_controllers.cpp, 00791C90: the curve scan fraction
//   (00791D8B..00791DA3 and its copies), the eased blend (00791E39..00791E90 and
//   copies), the spherical offset (00792202..00792262) and the linear curve
//   (00792386..0079239F, 007924C6..007924DF). modifier_ease (the inlined
//   0078FCF0) is left alone: sq + sq is exact, so 1 - (sq + sq) is one rounding.
//   The 0078FCF0 body itself is ease_0078fcf0 in hud_movie_camera.cpp, already
//   per operation and fixture-tested at 24-bit.
// - hud_movie_camera.cpp: 00795650 the ship offset (007956EB..007956FC);
//   00798130 the zoom and window blends (007987C6..0079881E) and the up length
//   (00798A87..00798AB9); 0079A3B0 the forward length (0079B6D9..0079B70C).
// - camera_path_sampler.cpp, 007AF150: the closed-path test (007AF241..007AF279)
//   and the knot length (007AF565..007AF5A5).
// - mission_camera.cpp: 0042F0C0 the height above the unit (0042F1A3..0042F1C5)
//   and the water midpoint (0042F41B..0042F43B); 0064DA40 the pitch limits
//   (0064DB94..0064DBBE); 00432ED0 the sway (00432F35..00432F81) and the two
//   basis dots (0043325C..0043328C, 004332B4..004332DA); 004DC940 the FOV
//   (004DC9BC..004DC9CE; the other copy, game_set_view_fov_004dc940 in
//   seg10_cc13.cpp, is not switched here); 0064E2D6 the pipe-sight FOV scale (0064E2D6..0064E2E3,
//   inside 0064DD30; the host passes zoom state 0, so ON and OFF agree today).
//   0087EC0F the config FOV in radians (0087EC0F..0087EC2F, inside 0087D7B0).
//   24 bits because the image runs 0087D7B0 once in app-init phase 11
//   (0073E1A1), after CreateDevice in phase 6 (0073DC25) has set PC24 on the
//   WinMain thread; the host recomputes per frame what the loader stored at
//   config+F8h. Its other copy, fov() in
//   native_global_config_load.cpp, is inline x87 and takes the thread's
//   precision control as it is.
inline constexpr bool kPc24CameraCc13Bound = false;
