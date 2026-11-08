#include "bsp/cc13_seg48.hpp"

#include <cstdint>
#include <cstdio>
#include <cstring>
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
    View(const bool& freeze, bool& lock, float& speed, const float& class_speed)
        : PlaneSquadronTravelSpeedView(freeze, lock, speed), source(class_speed) {}
    float countdown_388() const override {
        ++countdown_reads;
        return remaining;
    }
    const float& class_speed_190() const override {
        ++reads;
        return source;
    }
    const float& source;
    float remaining{-1.0f};
    mutable unsigned countdown_reads{};
    mutable unsigned reads{};
};
}

int main() {
    bool frozen = true;
    bool locked = true;
    float speed = 80.0f;
    const float class_speed = -0.0f;
    View view(frozen, locked, speed, class_speed);
    // A frozen override preserves its lock and never reads the class.
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(locked && speed == 80.0f && view.reads == 0 && view.countdown_reads == 0);
    frozen = false;
    // First unfrozen head transition consumes only the lock.
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(!locked && speed == 80.0f && view.reads == 0);
    // The old host used >= on its deadline. Native COMISS/JBE excludes zero
    // and unordered values, and must not consult the class on either path.
    view.remaining = 0.0f;
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    view.remaining = std::numeric_limits<float>::quiet_NaN();
    require(!bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(speed == 80.0f && view.reads == 0);
    view.remaining = -1.0f;
    require(bsp::cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(view));
    require(!locked && bits(speed) == bits(class_speed) && view.reads == 1);
    std::puts("007ED5D0 frozen/lock/zero/unordered/lazy class publication passed");
}
