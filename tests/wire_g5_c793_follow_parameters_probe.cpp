#include "bsp/game_tuning_singleton.hpp"
#include "bsp/native_land_follow_parameters.hpp"
#include <cstdio>
#include <stdexcept>

int main() {
    bsp::GameTuningBlock tuning{};
    bsp::FollowParameterSnapshot snapshot;
    try {
        static_cast<void>(snapshot.safe_alt());
        return 1;
    } catch (const std::logic_error&) {}

    tuning.pilot_follow_safe_alt = 125.0f;
    if (!snapshot.observe_follow_think(tuning, 1) || snapshot.safe_alt() != 125.0f)
        return 2;
    tuning.pilot_follow_safe_alt = 250.0f;
    // The second arm in one think and the next follow think reuse the copied
    // state even if its tuning source changed. A think outside follow ends it.
    if (snapshot.observe_follow_think(tuning, 1) ||
        snapshot.observe_follow_think(tuning, 2) || snapshot.safe_alt() != 125.0f)
        return 3;
    if (!snapshot.observe_follow_think(tuning, 4) || snapshot.safe_alt() != 250.0f)
        return 4;
    std::puts("c793 follow parameter snapshot: initial copy, retained state, re-entry passed");
}
