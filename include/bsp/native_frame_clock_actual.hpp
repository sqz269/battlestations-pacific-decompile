#pragma once
#include <cstddef>
#include <cstdint>
#include <type_traits>
#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native frame-clock actual storage requires MSVC Win32.
#endif
namespace bsp {
// Actual 80h allocation. Deliberately has no default member initializers.
// Timestamp pairs are signed ticks/frequency, each exactly 16 bytes.
struct alignas(8) NativeFrameClockActualStorage final {
    std::uint32_t profile_00;
    std::uint32_t accumulated_float_bits_04;
    std::uint64_t updates_08;
    std::int64_t origin_10[2];
    std::int64_t current_20[2];
    std::int64_t previous_30[2];
    std::int64_t interval_40[2];
    std::int64_t pause_snapshot_50[2];
    std::int64_t frequency_60;
    std::uint8_t paused_68;
    std::uint8_t fixed_69;
    std::byte untouched_6a[6];
    std::int64_t increment_70;
    std::int64_t synthetic_78;
};
static_assert(sizeof(NativeFrameClockActualStorage)==0x80);
static_assert(std::is_trivially_default_constructible_v<NativeFrameClockActualStorage>);
static_assert(offsetof(NativeFrameClockActualStorage, origin_10)==0x10);
static_assert(offsetof(NativeFrameClockActualStorage, current_20)==0x20);
static_assert(offsetof(NativeFrameClockActualStorage, previous_30)==0x30);
static_assert(offsetof(NativeFrameClockActualStorage, interval_40)==0x40);
static_assert(offsetof(NativeFrameClockActualStorage, pause_snapshot_50)==0x50);
static_assert(offsetof(NativeFrameClockActualStorage, frequency_60)==0x60);
static_assert(offsetof(NativeFrameClockActualStorage, paused_68)==0x68);
static_assert(offsetof(NativeFrameClockActualStorage, fixed_69)==0x69);
static_assert(offsetof(NativeFrameClockActualStorage, increment_70)==0x70);
static_assert(offsetof(NativeFrameClockActualStorage, synthetic_78)==0x78);
// Borrow the actual current D68D50 table, containing original address words.
// Storage +0 must identify D68D50. Only its verified +08 update and +20 sample
// targets are admitted. No table, process binding, publication or owner is made.
struct NativeFrameClockActualContext final {
    const volatile std::uint32_t* profile_d68d50;
};
// Original ECX/RET methods gain an explicit EDX context where dispatch needs it.
// Required storage is the genuine 80h layout; no FrameClock projection accepted.
void __fastcall pause_native_frame_clock_00bedae0(void*, const NativeFrameClockActualContext*);
void __fastcall initialize_native_frame_clock_00bedbd0(void*, const NativeFrameClockActualContext*);
void __fastcall resume_native_frame_clock_00beddc0(void*, const NativeFrameClockActualContext*);
// ECX actual clock; enable's signed milliseconds is on stack, RET 4.
void __fastcall enable_fixed_native_frame_clock_00bedb20(void*, void*, std::int32_t milliseconds);
void __fastcall disable_fixed_native_frame_clock_00bedb60(void*) noexcept;
void __fastcall update_native_frame_clock_00bedc30(void*);
const void* __fastcall get_raw_timer_current_00bee050(const void*) noexcept;
const void* __fastcall get_raw_timer_previous_00bee060(const void*) noexcept;
const void* __fastcall get_raw_timer_interval_00bee070(const void*) noexcept;
// BEDB70 remains add_timestamp_00bedb70 in frame_clock.hpp. These methods reuse
// it on complete, properly aligned, disjoint timestamp objects. BEE080 remains
// sample_native_frame_clock_00bee080 in native_renderer_control_worker.hpp.
} // namespace bsp
