#pragma once
#include <cstdint>

namespace bsp {
struct ClockTimestamp {
    std::int64_t ticks{0};
    std::int64_t frequency{1};
};
static_assert(sizeof(ClockTimestamp) == 16);

// Native: ECX=left, stack destination/right, EAX=destination, RET 8.
// Preserves low-64-bit arithmetic and exact aliases. Zero right frequency faults
// only on the unequal-frequency path, as in the native signed division helper.
ClockTimestamp& subtract_timestamp_00530890(ClockTimestamp& destination,
    const ClockTimestamp& left, const ClockTimestamp& right) noexcept;
float timestamp_seconds_x87(const ClockTimestamp& value) noexcept;

// Typed state projection, not the original singleton/vtable object ABI.
// Fixed-counter fields must be supplied by the recovered control path before use.
struct FrameClock {
    float accumulated{}; // native +4, game update owns accumulation
    std::uint64_t updates{}; // +8
    ClockTimestamp start; // +10
    ClockTimestamp current; // +20
    ClockTimestamp previous; // +30
    ClockTimestamp interval; // +40
    ClockTimestamp pause_snapshot; // +50
    std::int64_t frequency{}; // +60
    bool paused{}; // +68
    bool fixed_counter{}; // +69
    std::int64_t increment{}; // +70, host default only
    std::int64_t synthetic_counter{}; // +78, host default only
};

// Native ECX=this, no stack arguments. False reports a failed Win32 QPC call;
// native ignores that failure. No singleton registration or virtual overrides.
bool initialize_frame_clock_00bedbd0(FrameClock& clock) noexcept;
bool update_frame_clock_00bedc30(FrameClock& clock) noexcept;
// Native ECX=this, signed int32 milliseconds on stack, RET 4. Startup supplies50.
bool enable_fixed_clock_00bedb20(FrameClock& clock, std::int32_t milliseconds) noexcept;
}
