#include "bsp/native_renderer_texture_name_notification.hpp"
#include "bsp/native_string.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <cstring>
#include <exception>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4);

void* address(const void* base, std::uint32_t offset) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}
__forceinline std::uint32_t read_word(const void* location) noexcept {
    std::uint32_t value;
    __asm { mov eax, location }
    __asm { mov eax, dword ptr [eax] }
    __asm { mov value, eax }
    return value;
}
__forceinline void write_word(void* location, std::uint32_t value) noexcept {
    __asm { mov eax, location }
    __asm { mov edx, value }
    __asm { mov dword ptr [eax], edx }
}
char* data_at(const void* location) noexcept {
    return reinterpret_cast<char*>(read_word(location));
}

int terminate_cpp_cleanup_exception(unsigned long code) noexcept {
    if (code == 0xe06d7363u) std::terminate();
    return EXCEPTION_CONTINUE_SEARCH;
}
// FH3 terminates during second-exception SEARCH, before nested cleanup runs.
// A catch/rethrow or noexcept alone changes the observable unwind behavior.
void unwind_notification(void* name, const NativeRendererOptionalGuardStorage& guard,
    NativeRendererTextureNameNotificationContext& context, bool string_armed) noexcept {
    __try {
        if (string_armed)
            destroy_native_string_header_0041dd20(name, context.actual_string_storage);
        destroy_native_renderer_optional_guard_00b21110(guard, context.synchronization);
    } __except (terminate_cpp_cleanup_exception(GetExceptionCode())) {
        __assume(0);
    }
}
struct NotificationCleanup {
    void* name;
    const NativeRendererOptionalGuardStorage& guard;
    NativeRendererTextureNameNotificationContext& context;
    bool string_armed = false;
    bool armed = true;
    ~NotificationCleanup() noexcept {
        if (armed) unwind_notification(name, guard, context, string_armed);
    }
};
} // namespace

void notify_native_renderer_texture_name_removal_00b32250(
    void* actual_receiver, const void* original_name,
    NativeRendererTextureNameNotificationContext& context) {
    NativeRendererOptionalGuardStorage guard;
    const bool entry_enabled = context.synchronization.mode_00 != 0;
    if (entry_enabled) {
        guard.renderer_04 = context.actual_renderer_00f8d394;
        guard.entered_00 = enter_native_renderer_optional_guard_00b33ad0(
            guard.renderer_04, context.synchronization);
    }
    // Entry failure precedes state 0. Native EDI starts at zero and only the
    // completed resize path captures a different pointer for normal cleanup.
    char* captured_data = nullptr;
    alignas(4) unsigned char name[8];
    const bool distinct = static_cast<const void*>(name) != original_name;
    // The original has cleanup states and no catch map. This armed destructor
    // preserves first-exception search and its in-flight exception count.
    NotificationCleanup cleanup{name, guard, context};
    write_word(name, 0);
    write_word(name + 4, 0);
    if (distinct) {
        resize_native_string_header_0041dd40(name, context.actual_string_storage,
            read_word(original_name), true);
        const auto current_source_length = read_word(original_name);
        captured_data = data_at(name + 4);
        if (current_source_length != 0) {
            const auto count = read_word(name);
            const auto* source = data_at(address(original_name, 4));
            if (count != 0) std::memcpy(captured_data, source, count);
        }
    }
    // State 1 begins after copying, not when storage first becomes owned.
    cleanup.string_armed = true;
    lowercase_native_string_header_004bcc00(name);
    remove_native_render_resource_by_alias_00b31dc0(
        address(actual_receiver, 0x1a74), original_name,
        context.actual_string_pool, context.callbacks, context.accounting_tables);

    cleanup.string_armed = false; // State 0 before normal string release.
    if (captured_data) {
        const auto current_size = read_word(name) + 1u;
        context.actual_string_storage.release(captured_data, current_size);
    }

    // The current mode check precedes native state -1; leave itself is unarmed.
    const bool leave_enabled = context.synchronization.mode_00 != 0;
    cleanup.armed = false;
    if (leave_enabled) {
        __assume(entry_enabled); // Stated initialized-guard domain, no fallback.
        // Native MOV loads the whole saved DWORD, including unwritten padding.
        // The callee ignores it. An isolated MOV avoids a C++ indeterminate read.
        const auto ignored_saved_word = read_word(&guard);
        const auto* captured_renderer = reinterpret_cast<const void*>(
            read_word(address(&guard, 4)));
        leave_native_renderer_optional_guard_00b33b00(
            captured_renderer, ignored_saved_word, context.synchronization);
    }
}

} // namespace bsp
