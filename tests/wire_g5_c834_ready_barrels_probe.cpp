#include "bsp/director_geommesh_seg38_39_argon.hpp"

#include <float.h>
#include <cstdio>
#include <limits>

int main() {
    unsigned int control = 0;
    _controlfp_s(&control, _PC_24, _MCW_PC);
    // One fixture covers the native four-at-a-time loop and its remainder.
    // FCOMI/JC skips unordered in both; the removed live duplicate counted it.
    const float nan = std::numeric_limits<float>::quiet_NaN();
    const float timers[] = {0.0f, nan, 1.0f, -1.0f, nan, 2.0f};
    const auto count = bsp::director_geommesh_seg38_39_argon::bsp_gun_count_ready_barrels_00727d70;
    const int finite_horizon = count(timers, 6, 1.0f);
    const int unordered_horizon = count(timers, 6, nan);
    if (finite_horizon != 3 || unordered_horizon != 0) {
        std::fprintf(stderr, "00727D70 unordered regression: finite=%d unordered=%d\n",
            finite_horizon, unordered_horizon);
        return 1;
    }
    std::puts("00727D70 ordered reload-count probe passed (semantic fixture, not gameplay proof).");
    return 0;
}
