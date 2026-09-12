// 0078CF20, the water height sampler. See docs/OCEAN_HEIGHT.md.

#include "bsp/ocean_height.hpp"

namespace bsp {

float ocean_water_height_0078cf20(float x, float z, OceanHeightHost& host) {
    // 0078CF3E and 0078CF5F call the two field methods with the same (x, z) pair and the
    // same receiver, [world+A8h]. The first result is spilled as a double at 0078CF49 and
    // the second multiplies it at 0078CF64 (FMUL qword), so the product is formed at x87
    // register precision and rounded to float once by the FSTP at 0078CF69.
    const double wave = static_cast<double>(host.wave_height_0078c890(x, z));
    const double mask = static_cast<double>(host.coverage_mask_00b9cf50(x, z));
    return static_cast<float>(mask * wave);
}

}  // namespace bsp
