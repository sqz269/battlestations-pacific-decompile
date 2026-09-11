#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual renderer synchronization requires MSVC Win32.
#endif

namespace bsp {

// Actual contiguous global bytes at 0108D6DCh..0108D6E3h. Borrow this storage;
// no initialization, ownership, bool normalization or atomic policy is added.
struct NativeRendererSynchronizationGlobals {
    volatile std::uint8_t mode_00;
    volatile std::uint8_t observed_mode_01;
    std::uint8_t preserved_02[2];
    volatile std::uint32_t nesting_04;
};
static_assert(sizeof(NativeRendererSynchronizationGlobals) == 8);
static_assert(offsetof(NativeRendererSynchronizationGlobals, nesting_04) == 4);

// Actual native local guard record. The caller only writes its fields when
// its entry-time mode check enables entry: renderer first, then returned AL.
// A skipped entry leaves this record uninitialized. No constructor is added.
struct NativeRendererOptionalGuardStorage {
    volatile std::uint8_t entered_00;
    std::uint8_t preserved_01[3];
    const void* volatile renderer_04;
};
static_assert(sizeof(NativeRendererOptionalGuardStorage) == 8);
static_assert(offsetof(NativeRendererOptionalGuardStorage, renderer_04) == 4);

// Complete 00B33AA0, native stack low byte/RET4; ECX is unused.
void set_native_renderer_synchronization_00b33aa0(
    NativeRendererSynchronizationGlobals&, std::uint8_t raw_mode) noexcept;

// Complete 00B33AD0, native ECX renderer/RET/AL0 or1. Read the actual lock
// pointer at renderer+04 and operate on its real 1Ch tracked critical section.
// Only the low-byte result has meaning; this is a new C++ entry interface.
std::uint8_t enter_native_renderer_optional_guard_00b33ad0(
    const void* actual_renderer, NativeRendererSynchronizationGlobals&);

// Complete 00B33B00, native ECX renderer/RET4. Its saved-result stack word is
// consumed but ignored. Current mode and the current renderer+04 lock govern
// leaving. Counters retain native non-atomic DWORD wrapping.
void leave_native_renderer_optional_guard_00b33b00(const void* actual_renderer,
    std::uint32_t ignored_saved_result, NativeRendererSynchronizationGlobals&);

// Complete 00B21110: native ECX actual8-byte record/RET. Tests CURRENT mode
// before reading either saved field. It does not repair an uninitialized
// record if mode changed after entry was skipped. No record writes occur.
void destroy_native_renderer_optional_guard_00b21110(
    const NativeRendererOptionalGuardStorage&, NativeRendererSynchronizationGlobals&);

// Complete 00B20220: native ECX renderer/RET/AL=(signed depth>0). Reads the
// separate actual lifecycle lock at renderer+199Ch, then its depth+18h.
// No acquisition, null guard, whole renderer overlay or synchronization.
std::uint8_t native_renderer_device_lifecycle_busy_00b20220(
    const void* actual_renderer) noexcept;

} // namespace bsp
