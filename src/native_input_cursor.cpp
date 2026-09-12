#include "bsp/native_input_cursor.hpp"

#include <cstdlib>
#include <cstring>

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "native input cursor requires Win32 storage");

void* offset(void* base, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + bytes);
}
std::uint32_t word(void* base, std::uint32_t bytes) noexcept {
    return *static_cast<const volatile std::uint32_t*>(offset(base, bytes));
}
std::int32_t count(void* base, std::uint32_t bytes) noexcept {
    return static_cast<std::int32_t>(word(base, bytes));
}
void* pointer(void* base, std::uint32_t bytes) noexcept {
    return reinterpret_cast<void*>(word(base, bytes));
}
void put_pointer(void* base, std::uint32_t bytes, void* value) noexcept {
    std::memcpy(offset(base, bytes), &value, sizeof(value));
}
bool empty_from(void* header, std::uint32_t captured_begin) noexcept {
    return captured_begin == 0 ||
        (static_cast<std::int32_t>(word(header, 8) - captured_begin) >> 2) == 0;
}
// The first caller reloads ECX after a returning invalid-parameter handler;
// the second caller preserves only its old vector address across that call.
template<class AfterInvalid>
void* first_mouse(void* header, AfterInvalid after_invalid) {
    const auto begin = word(header, 4);
    if (empty_from(header, begin)) return nullptr;
    if (empty_from(header, begin)) {
        _invalid_parameter_noinfo();
        after_invalid();
    }
    return pointer(pointer(header, 4), 0);
}
void update_loading_backend(NativeInputCursorContext& c) {
    void* const backend = c.backend_00f8bbf4;
    const volatile float* const step = &c.loading_step_00d7a2f0;
    float seconds;
    std::uint32_t profile;
    // Preserve FLD, profile capture, then FSTP (including x87 quieting/rounding)
    // rather than relying on the compiler's choice of an SSE/bitwise transfer.
    __asm {
        mov eax, step
        fld dword ptr [eax]
        mov eax, backend
        mov eax, dword ptr [eax]
        mov profile, eax
        fstp seconds
    }
    c.calls.backend_vslot04(backend, profile, seconds);
}
} // namespace

void retarget_native_mouse_input_bindings_00a92840(void* owner, void* replacement) noexcept {
    auto action_count = word(owner, 8);
    auto* action = pointer(owner, 4);
    auto* end = offset(action, action_count * 0x30u);
    while (action != end) {
        std::int32_t index = 0;
        std::uint32_t displacement = 0;
        while (index < count(action, 0x14)) {
            void* const binding = offset(pointer(action, 0x10), displacement);
            if (word(binding, 4) == 1) put_pointer(binding, 0xc, replacement);
            std::int32_t modifier_index = 0;
            std::uint32_t modifier_displacement = 0;
            while (modifier_index < count(binding, 0x1c)) {
                void* const modifier = offset(pointer(binding, 0x18), modifier_displacement);
                if (word(modifier, 0) == 1) put_pointer(modifier, 8, replacement);
                ++modifier_index;
                modifier_displacement += 0x14;
            }
            modifier_index = 0;
            modifier_displacement = 0;
            while (modifier_index < count(binding, 0x28)) {
                void* const modifier = offset(pointer(binding, 0x24), modifier_displacement);
                if (word(modifier, 0) == 1) put_pointer(modifier, 8, replacement);
                ++modifier_index;
                modifier_displacement += 0x14;
            }
            ++index;
            displacement += 0x34;
        }
        action_count = word(owner, 8);
        end = offset(pointer(owner, 4), action_count * 0x30u);
        action = offset(action, 0x30);
    }
}

void reset_native_input_focus_00beca40(NativeInputCursorContext& c) {
    void* backend = c.backend_00f8bbf4;
    if (!backend) return;
    void* const first_header = offset(backend, 0x90);
    void* const old_mouse = first_mouse(first_header, [&] { backend = c.backend_00f8bbf4; });
    c.calls.call_00a90ee0(backend, old_mouse);
    backend = c.backend_00f8bbf4;
    c.calls.call_00bebf30(backend, 1);
    backend = c.backend_00f8bbf4;
    const auto profile = word(backend, 0);
    c.calls.backend_vslot0c(backend, profile);
    backend = c.backend_00f8bbf4;
    c.calls.call_00a91620(backend, 1, 0);
    backend = c.backend_00f8bbf4;
    void* const replacement = first_mouse(offset(backend, 0x90), [] {});
    void* const actions = get_native_input_action_owner_004bec00(c.actions);
    retarget_native_mouse_input_bindings_00a92840(actions, replacement);
}

void update_native_input_cursor_00becb20(Win32PlatformState& platform, bool loading,
    NativeInputCursorContext& c) {
    if (!c.calls.current_platform_manager_00f8abe8()) return;
    void* const initial_backend = c.backend_00f8bbf4;
    if (!initial_backend) return;
    if (!c.calls.call_004ba6d0(initial_backend, 1, 0)) return;
    if (loading)
        c.calls.pump_platform_manager_00a409f0(*c.calls.current_platform_manager_00f8abe8());

    const bool focused = platform.byte_041;
    const bool system_ui = c.calls.current_platform_manager_00f8abe8()->system_ui_visible;
    const bool should_show = !focused || system_ui;
    bool focus_was_reset = false;
    auto& globals = c.globals;
    if (system_ui && !globals.previous_system_ui_0109db90) {
        reset_native_input_focus_00beca40(c);
        focus_was_reset = true;
        void* const backend = c.backend_00f8bbf4;
        void* const mouse = c.calls.call_004ba6d0(backend, 1, 0);
        c.calls.call_00a9a140(mouse, 6);
        if (loading) update_loading_backend(c);
    }
    if (!system_ui && globals.previous_system_ui_0109db90 && should_show)
        globals.focus_reset_pending_0109db8f = 1;
    if (!should_show && (globals.previous_system_ui_0109db90 ||
        globals.focus_reset_pending_0109db8f)) {
        reset_native_input_focus_00beca40(c);
        focus_was_reset = true;
        if (loading) update_loading_backend(c);
        globals.focus_reset_pending_0109db8f = 0;
    }
    if (should_show) {
        if (!globals.cursor_shown_0109db8e) {
            const auto show = c.show_cursor_00ce2344;
            globals.cursor_shown_0109db8e = 1;
            while (show(1) < 0) {}
        }
    } else if (globals.cursor_shown_0109db8e) {
        if (!focus_was_reset) {
            reset_native_input_focus_00beca40(c);
            if (loading) update_loading_backend(c);
        }
        const auto show = c.show_cursor_00ce2344;
        globals.cursor_shown_0109db8e = 0;
        while (show(0) >= 0) {}
    }
    globals.previous_system_ui_0109db90 = static_cast<std::uint8_t>(system_ui);
}

void run_native_platform_application_service_00bece70(Win32PlatformState& platform,
    NativeInputCursorContext& cursor, PlatformApplicationServiceHost& application) {
    application.run_application_frame_vslot_10(platform.application);
    update_native_input_cursor_00becb20(platform, false, cursor);
}
} // namespace bsp
