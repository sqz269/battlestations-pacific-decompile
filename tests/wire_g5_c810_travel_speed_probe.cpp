#include "bsp/cc13_seg48.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
#include <float.h>
#include <limits>
#include <stdexcept>

namespace {
void require(bool condition) {
    if (!condition) throw std::runtime_error("007ED5D0 travel-speed publication failed");
}
std::uint32_t bits(float value) {
    std::uint32_t result;
    std::memcpy(&result, &value, sizeof result);
    return result;
}
struct View final : bsp::PlaneSquadronTravelSpeedView {
    View(float& countdown, const bool& freeze, bool& lock, float& speed, const float& class_speed)
        : PlaneSquadronTravelSpeedView(freeze, lock, speed), timer{countdown, freeze}, source(class_speed) {}
    float countdown_388() const override {
        ++countdown_reads;
        return timer.read();
    }
    const float& class_speed_190() const override {
        ++reads;
        return source;
    }
    bsp::PlaneSquadronTravelSpeedHostCountdown timer;
    const float& source;
    mutable unsigned countdown_reads{};
    mutable unsigned reads{};
};
}

int main() {
    unsigned old_control;
    require(_controlfp_s(&old_control, 0, 0) == 0);
    unsigned control;
    require(_controlfp_s(&control, _PC_24, _MCW_PC) == 0);
    bool frozen = true;
    bool locked = true;
    float remaining = -1.0f;
    float speed = 80.0f;
    const float class_speed = -0.0f;
    View view(remaining, frozen, locked, speed, class_speed);
    // A frozen override preserves its lock and never reads the class.
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(locked && speed == 80.0f && view.reads == 0 && view.countdown_reads == 0);
    frozen = false;
    // First unfrozen head transition consumes only the lock.
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(!locked && speed == 80.0f && view.reads == 0);
    // The old host used >= on its deadline. Native COMISS/JBE excludes zero
    // and unordered values, and must not consult the class on either path.
    remaining = 0.0f;
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    remaining = std::numeric_limits<float>::quiet_NaN();
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(speed == 80.0f && view.reads == 0);
    remaining = -1.0f;
    require(bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(!locked && bits(speed) == bits(class_speed) && view.reads == 1);

    // Review 1766: go through the SAME retained-countdown adapter used by the
    // host setter, RefreshView and group-3 phase. Pilot/group-1 must read before
    // each group-3 decrement; elapsed-wall-clock subtraction restored on step 10.
    view.timer.reset_script_override();
    require(bits(remaining) == 0x3f000000u);
    frozen = true;
    view.timer.advance_group3(0.05f);
    require(bits(remaining) == 0x3f000000u);
    frozen = false;
    view.timer.advance_group3(0.0f);
    view.timer.advance_group3(-0.05f);
    view.timer.advance_group3(std::numeric_limits<float>::quiet_NaN());
    require(bits(remaining) == 0x3f000000u);
    locked = true; // The setter's +3ACh store, separate from its timer store.
    speed = 80.0f;
    view.reads = 0;
    for (unsigned step = 1; step <= 10; ++step) {
        if (step == 1 || step == 10) {
            if (step == 10) require(bits(view.timer.read()) == 0x3d4cccc3u);
            require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
            require(speed == 80.0f && !locked && view.reads == 0);
        }
        view.timer.advance_group3(0.05f);
    }
    require(bits(remaining) == 0xb3200000u);
    require(bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(bits(speed) == bits(class_speed) && view.reads == 1);
    // A later setter replaces the retained countdown, including after expiry.
    view.timer.reset_script_override();
    require(bits(remaining) == 0x3f000000u);
    require(_controlfp_s(&control, old_control, _MCW_PC) == 0);
    std::puts("007ED5D0 gates + host countdown: step10 3D4CCCC3 holds; step11 B3200000 restores; freeze/reset/PC24 passed");
}
