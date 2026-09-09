#include "bsp/math.hpp"
#include <cmath>

namespace bsp {
float abs_00401170(float value) { return std::fabs(value); }

void cross_reversed_00401c20(Vec3d& out, const Vec3d& a, const Vec3d& b) {
    // Preserve original store order, including behavior when out aliases inputs.
    out.x = a.z * b.y - b.z * a.y;
    out.y = b.z * a.x - a.z * b.x;
    out.z = a.y * b.x - a.x * b.y;
}

void subtract_reversed_00401cb0(Vec3d& out, const Vec3d& a, const Vec3d& b) {
    out.x = b.x - a.x;
    out.y = b.y - a.y;
    out.z = b.z - a.z;
}

void scale_00401cd0(Vec3d& value, double factor) {
    value.x = value.x * factor;
    value.y = value.y * factor;
    value.z = factor * value.z;
}

double length_squared_00401cf0(const Vec3d& value) {
    // Assembly adds y^2 + x^2 first, then z^2. Pseudocode reassociated this.
    return (value.y * value.y + value.x * value.x) + value.z * value.z;
}
}
