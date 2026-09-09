#include "bsp/random_threads.hpp"
#include "bsp/frame_clock.hpp"
#include <iostream>
#ifdef BSP_HAS_TIMESTAMP_REFERENCE
bool probe_timestamp_reference();
#endif

int main() {
#ifdef BSP_HAS_TIMESTAMP_REFERENCE
    if (!probe_timestamp_reference()) return 4;
#else
    std::cout << "Native timestamp comparison skipped: run tools/verify_timestamp_reference.py.\n";
#endif
    bsp::FrameClock clock;
    if (!bsp::initialize_frame_clock_00bedbd0(clock) || clock.updates != 2
        || !bsp::update_frame_clock_00bedc30(clock)) return 3;
    std::cout << "QPC clock initialized: updates=" << clock.updates
              << " delta=" << bsp::timestamp_seconds_x87(clock.interval) << '\n';
    if (!bsp::enable_fixed_clock_00bedb20(clock, 50)
        || !bsp::update_frame_clock_00bedc30(clock)
        || !bsp::update_frame_clock_00bedc30(clock)
        || clock.interval.ticks != clock.increment) return 5;
    // One state sequence covers the update gates without a new test target.
    const auto elapsed = clock.current.ticks;
    clock.increment = -clock.increment;
    if (!bsp::update_frame_clock_00bedc30(clock)
        || clock.current.ticks != elapsed || clock.interval.ticks != 0) return 6;
    clock.paused = true;
    const auto updates = clock.updates;
    const auto counter = clock.synthetic_counter;
    if (!bsp::update_frame_clock_00bedc30(clock) || clock.updates != updates
        || clock.synthetic_counter != counter || clock.interval.ticks != 0) return 7;
    std::cout << "Fixed 50ms increment, backward-time rollback and paused update checked.\n";
    bsp::RandomThreads random;
    auto* fallback = &random.state_00bd2ed0(bsp::RandomStream::primary);
    random.register_current_00bd2fe0();
    auto* primary = &random.state_00bd2ed0(bsp::RandomStream::primary);
    auto* secondary = &random.state_00bd2ed0(bsp::RandomStream::secondary);
    std::cout << "BSP reconstructed random-subsystem startup\n";
    if (primary == fallback || primary == secondary) return 1;
    for (int i = 0; i < 5; ++i)
        std::cout << "draw " << i << ": " << random.next_00bd2fc0(bsp::RandomStream::primary) << '\n';
    random.unregister_current_00bd3050();
    if (&random.state_00bd2ed0(bsp::RandomStream::primary) != fallback) return 2;
    std::cout << "Registration, distinct streams, fallback restoration and shutdown completed.\n"
                 "This probe does not initialize the game engine.\n";
}
