#pragma once
#include <array>

namespace bsp {

// Native ECX points to two floats [x,y], RET, float result in ST0. The actual
// CRT _CIatan2 boundary consumes ST0=x/ST1=y and returns atan2(y,x). Round it to
// float, subtract from double(float(pi/2)), round to float, then add
// double(float(2*pi)) once if ordered-negative. No normalization loop/clamp;
// final float rounding can produce the full-turn constant itself.
// Uses the current host CRT, not a reconstructed legacy CRT implementation;
// its diagnostics and exceptional-result details may differ from the game CRT.
float heading_angle_00414eb0(const std::array<float, 2>& direction);

// Native ECX=bounds [minX,minY,maxX,maxY], stack point pointer, RET4, EAX bool.
// Lower-inclusive/upper-exclusive. Unordered comparisons reject. Reads/rounds
// pointX first; reads pointY only after both X tests pass. New typed interface,
// not a native object layout. point_xy borrows float storage and may alias bounds.
bool contains_point_00414f50(const std::array<float, 4>& bounds,
    const float* point_xy) noexcept;

// Same native input ABI, RET4; return register is not consumed. Four ordered
// comparisons/conditional stores: minX, maxX, minY, maxY. Reload the point before
// each comparison, preserving effects of prior stores when point aliases bounds.
// NaNs do not cause a bound write; equal values (including signed zero) retain
// the original bound bits. x87 float spill and comparison ordering is retained.
void include_point_00415010(std::array<float, 4>& bounds,
    const float* point_xy) noexcept;

} // namespace bsp
