#include "bsp/cc13_left61.hpp"
#include <cmath>
#include <cstdio>
#include <limits>
struct Budget final : bsp::cc13_left61::TeamResourceBudgetHost {
    float support_points_total_00e0cfb4() override { return 0.0f; }
    void visit_live_resource_units(const std::function<void(std::int32_t, float)>& f) override {
        f(1, 0.0f);
        f(2, std::numeric_limits<float>::quiet_NaN());
        f(1, 16777216.0f);
        f(1, 1.0f);
        f(1, -16777216.0f);
    }
    float queued_resource_usage_for_team_009469f0(std::int32_t) override { return 3.0f; }
};
int main() {
    Budget budget;
    const float actual = bsp::cc13_left61::team_available_resources_00946fc0(1, budget);
    // PC24: -2^24 - 1 rounds to -2^24 before cancellation. A double
    // accumulator gives -4 here; subtracting the foreign NaN poisons it.
    if (actual != -3.0f) { std::printf("FAIL budget=%g expected=-3\n", actual); return 1; }
    std::puts("PASS 00946FC0 ordered PC24 cancellation, team filtering and queued usage");
}
