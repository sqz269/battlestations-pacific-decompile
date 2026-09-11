#include "bsp/native_renderer_synchronization_actual.hpp"
#include "bsp/random_threads.hpp"

#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);
static_assert(sizeof(TrackedCriticalSection) == 0x1c);
static_assert(offsetof(TrackedCriticalSection, native) == 0);
static_assert(offsetof(TrackedCriticalSection, depth) == 0x18);

volatile std::uint32_t& word(const void* actual, std::uint32_t offset) noexcept {
    const auto location = static_cast<std::uint32_t>(
        reinterpret_cast<std::uintptr_t>(actual)) + offset;
    return *reinterpret_cast<volatile std::uint32_t*>(location);
}
TrackedCriticalSection* lock_at(const void* renderer, std::uint32_t offset) noexcept {
    return reinterpret_cast<TrackedCriticalSection*>(word(renderer, offset));
}
std::int32_t signed_word(std::uint32_t bits) noexcept {
    std::int32_t value;
    std::memcpy(&value, &bits, sizeof(value));
    return value;
}
} // namespace

void set_native_renderer_synchronization_00b33aa0(
    NativeRendererSynchronizationGlobals& globals, std::uint8_t raw_mode) noexcept {
    globals.mode_00 = raw_mode;
    globals.observed_mode_01 = raw_mode;
}

std::uint8_t enter_native_renderer_optional_guard_00b33ad0(
    const void* actual_renderer, NativeRendererSynchronizationGlobals& globals) {
    globals.nesting_04 = globals.nesting_04 + 1u;
    if (globals.mode_00 == 0) return 0;
    auto* const captured_lock = lock_at(actual_renderer, 4);
    if (!captured_lock) return 0;
    EnterCriticalSection(&captured_lock->native);
    // The renderer's field may differ after entry returns. Increment the lock
    // captured before the real API call, preserving its exact DWORD bits.
    word(captured_lock, 0x18) = word(captured_lock, 0x18) + 1u;
    return 1;
}

void leave_native_renderer_optional_guard_00b33b00(const void* actual_renderer,
    std::uint32_t /*ignored_saved_result*/, NativeRendererSynchronizationGlobals& globals) {
    std::uint8_t mode = globals.mode_00;
    globals.nesting_04 = globals.nesting_04 - 1u;
    if (mode != 0) {
        auto* const captured_lock = lock_at(actual_renderer, 4);
        if (captured_lock) {
            word(captured_lock, 0x18) = word(captured_lock, 0x18) - 1u;
            LeaveCriticalSection(&captured_lock->native);
            // The native reload exists only after an actual Leave API call.
            mode = globals.mode_00;
        }
    }
    if (globals.nesting_04 == 0 && globals.observed_mode_01 != mode)
        globals.observed_mode_01 = mode;
}

void destroy_native_renderer_optional_guard_00b21110(
    const NativeRendererOptionalGuardStorage& actual_guard,
    NativeRendererSynchronizationGlobals& globals) {
    if (globals.mode_00 != 0) {
        const std::uint32_t saved_result = actual_guard.entered_00;
        const void* const captured_renderer = actual_guard.renderer_04;
        leave_native_renderer_optional_guard_00b33b00(captured_renderer,
            saved_result, globals);
    }
}

std::uint8_t native_renderer_device_lifecycle_busy_00b20220(
    const void* actual_renderer) noexcept {
    auto* const lifecycle_lock = lock_at(actual_renderer, 0x199c);
    return signed_word(word(lifecycle_lock, 0x18)) > 0 ? 1u : 0u;
}
} // namespace bsp
