#include "bsp/frame_clock.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <cstring>

namespace bsp {
namespace {
std::int64_t signed_bits(std::uint64_t bits) noexcept {
    std::int64_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}

std::uint64_t divide_signed_bits(std::uint64_t numerator,
    std::int64_t denominator) noexcept {
    if (denominator == 0) {
        // Native __alldiv faults with unsigned DIV, not signed overflow.
        __asm {
            xor eax, eax
            xor edx, edx
            xor ecx, ecx
            div ecx
        }
        return 0; // unreachable unless an external exception handler changes EIP
    }
    const bool negative_numerator = (numerator >> 63) != 0;
    const auto divisor_bits = static_cast<std::uint64_t>(denominator);
    const auto magnitude = negative_numerator ? 0ULL - numerator : numerator;
    const auto divisor = denominator < 0 ? 0ULL - divisor_bits : divisor_bits;
    const auto quotient = magnitude / divisor;
    return negative_numerator != (denominator < 0) ? 0ULL - quotient : quotient;
}
}

ClockTimestamp& subtract_timestamp_00530890(ClockTimestamp& destination,
    const ClockTimestamp& left, const ClockTimestamp& right) noexcept {
    const auto frequency = left.frequency;
    auto right_ticks = static_cast<std::uint64_t>(right.ticks);
    if (frequency != right.frequency) {
        right_ticks = divide_signed_bits(right_ticks *
            static_cast<std::uint64_t>(frequency), right.frequency);
    }
    const auto ticks = static_cast<std::uint64_t>(left.ticks) - right_ticks;
    destination = {signed_bits(ticks), frequency};
    return destination;
}

float timestamp_seconds_x87(const ClockTimestamp& value) noexcept {
    float seconds;
    const auto* pair = &value;
    __asm {
        mov eax, pair
        fild qword ptr [eax]
        fild qword ptr [eax + 8]
        fdivp st(1), st(0)
        fstp seconds
    }
    return seconds;
}

bool update_frame_clock_00bedc30(FrameClock& clock) noexcept {
    if (clock.paused) {
        subtract_timestamp_00530890(clock.interval, clock.interval, clock.interval);
        return true;
    }
    ++clock.updates;
    clock.previous = clock.current;
    ClockTimestamp now;
    if (clock.fixed_counter) {
        clock.synthetic_counter = signed_bits(
            static_cast<std::uint64_t>(clock.synthetic_counter) +
            static_cast<std::uint64_t>(clock.increment));
        now.ticks = clock.synthetic_counter;
    } else {
        LARGE_INTEGER counter;
        if (!QueryPerformanceCounter(&counter)) return false;
        now.ticks = counter.QuadPart;
    }
    now.frequency = clock.frequency;
    subtract_timestamp_00530890(clock.current, now, clock.start);
    subtract_timestamp_00530890(clock.interval, clock.current, clock.previous);
    // Native spills ratio to float32, then compares 0 > ratio. Unordered skips
    // rollback. Preserve the x87 comparison too (including exception behavior).
    const float seconds = timestamp_seconds_x87(clock.interval);
    unsigned char negative;
    __asm {
        fld seconds
        fldz
        fcomip st(0), st(1)
        fstp st(0)
        seta negative
    }
    if (negative) {
        clock.current = clock.previous;
        subtract_timestamp_00530890(clock.interval, clock.current, clock.previous);
    }
    return true;
}

bool initialize_frame_clock_00bedbd0(FrameClock& clock) noexcept {
    clock.paused = false;
    clock.updates = 0;
    LARGE_INTEGER frequency, counter;
    if (!QueryPerformanceFrequency(&frequency)) return false;
    clock.frequency = frequency.QuadPart;
    if (!QueryPerformanceCounter(&counter)) return false;
    clock.start = {counter.QuadPart, clock.frequency};
    return update_frame_clock_00bedc30(clock) && update_frame_clock_00bedc30(clock);
}

bool enable_fixed_clock_00bedb20(FrameClock& clock, std::int32_t milliseconds) noexcept {
    clock.fixed_counter = true;
    LARGE_INTEGER counter;
    if (!QueryPerformanceCounter(&counter)) return false;
    clock.synthetic_counter = counter.QuadPart;
    const auto product = static_cast<std::uint64_t>(clock.frequency) *
        static_cast<std::uint64_t>(static_cast<std::int64_t>(milliseconds));
    clock.increment = signed_bits(divide_signed_bits(product, 1000));
    return true;
}
}
