#include "bsp/native_dynamic_buffer_device_release.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* address(const void* base, std::uint32_t byte_offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + byte_offset);
}
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void clear_word(void* location) noexcept {
    __asm { mov eax, location }
    __asm { mov dword ptr [eax], 0 }
}
void* pointer_at(const void* location) noexcept {
    return reinterpret_cast<void*>(read_word(location));
}
using ReferenceOperation = unsigned long (__stdcall*)(void*);
void reference_operation(void* object, std::uint32_t table_offset) {
    const auto table = pointer_at(object);
    reinterpret_cast<ReferenceOperation>(read_word(address(table, table_offset)))(object);
}
void release_wrapper(void* captured_wrapper) {
    void* const captured_object = pointer_at(address(captured_wrapper, 0x28));
    if (captured_object) {
        reference_operation(captured_object, 4);
        reference_operation(captured_object, 8); // Reload captured object's table.
    }
    void* const current_object = pointer_at(address(captured_wrapper, 0x28));
    if (current_object) {
        reference_operation(current_object, 8);
        clear_word(address(captured_wrapper, 0x28)); // Only after it returns.
    }
}
int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
void unwind_guard(const NativeRendererOptionalGuardStorage& guard,
    NativeRendererSynchronizationGlobals& globals) noexcept {
    __try {
        destroy_native_renderer_optional_guard_00b21110(guard, globals);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct GuardCleanup {
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererSynchronizationGlobals& globals;
    bool armed = true;
    ~GuardCleanup() noexcept { if (armed) unwind_guard(guard, globals); }
};
} // namespace

void release_native_dynamic_buffers_for_reset_00b237d0(
    void* actual_renderer, NativeRendererSynchronizationGlobals& actual_globals) {
    NativeRendererOptionalGuardStorage guard;
    const bool entry_enabled = actual_globals.mode_00 != 0;
    if (entry_enabled) {
        guard.renderer_04 = actual_renderer;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            actual_renderer, actual_globals);
    }
    const bool ready = *static_cast<volatile unsigned char*>(
        address(actual_renderer, 0x1d8c)) != 0;
    GuardCleanup cleanup{guard, actual_globals}; // Native state 0 starts here.
    if (ready) {
        void* const vertex_wrapper = pointer_at(address(actual_renderer, 0x1974));
        *static_cast<volatile unsigned char*>(address(actual_renderer, 0x1d8c)) = 0;
        release_wrapper(vertex_wrapper);
        void* const index_wrapper = pointer_at(address(actual_renderer, 0x1978));
        release_wrapper(index_wrapper);
    }
    const bool leave_enabled = actual_globals.mode_00 != 0;
    cleanup.armed = false; // Native state -1 precedes normal leave.
    if (leave_enabled) {
        __assume(entry_enabled);
        const auto ignored_saved_word = read_word(&guard);
        const auto* saved_renderer = pointer_at(address(&guard, 4));
        leave_native_renderer_optional_guard_00b33b00(
            saved_renderer, ignored_saved_word, actual_globals);
    }
}

} // namespace bsp
