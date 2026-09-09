#pragma once

namespace bsp {
struct Vec3d { double x, y, z; };
static_assert(sizeof(Vec3d) == 24);

// Semantic C++ interfaces, NOT the original register-based ABI.
float abs_00401170(float value);
void cross_reversed_00401c20(Vec3d& out, const Vec3d& a, const Vec3d& b);
void subtract_reversed_00401cb0(Vec3d& out, const Vec3d& a, const Vec3d& b);
void scale_00401cd0(Vec3d& value, double factor);
double length_squared_00401cf0(const Vec3d& value);
}
