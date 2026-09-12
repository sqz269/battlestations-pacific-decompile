#pragma once

#include "bsp/native_input_action_owner.hpp"
#include "bsp/platform_cursor.hpp"

namespace bsp {

using NativeInputShowCursorCall = int (__stdcall*)(int visible);

// Source services over the actual raw input allocations. The online/platform
// objects stay the existing canonical source owners; no guessed raw layout is
// imposed on them. Plain publication accessors must not synthesize state.
struct NativeInputCursorCalls {
    virtual ~NativeInputCursorCalls() = default;
    virtual PlatformManagerFlags* current_platform_manager_00f8abe8() noexcept = 0;
    virtual void pump_platform_manager_00a409f0(PlatformManagerFlags&) = 0;
    // Externally owned getter: actual backend+6C+class*24h binding vectors,
    // signed class, unsigned index. Never the fixed attachment table.
    virtual void* call_004ba6d0(void* backend, std::int32_t device_class,
        std::uint32_t index) = 0;
    virtual void call_00a90ee0(void* backend, void* device) = 0;
    virtual void call_00bebf30(void* backend, std::int32_t device_class) = 0;
    virtual void call_00a91620(void* backend, std::int32_t device_class,
        std::int32_t fixed_slot) = 0;
    // Select the target from the supplied captured native profile. Nested
    // calls in that body perform their own current-profile reads.
    virtual void backend_vslot0c(void* backend, std::uint32_t captured_profile) = 0;
    virtual void backend_vslot04(void* backend, std::uint32_t captured_profile,
        float seconds) = 0;
    // Bind the existing set_native_mouse_cooperative_level_00a9a140 body;
    // its real COM slot/receiver/window timing and +234 write remain there.
    virtual void call_00a9a140(void* mouse, std::uint32_t flags) = 0;
};

struct NativeInputCursorContext {
    void* volatile& backend_00f8bbf4;
    NativeInputActionOwnerContext& actions;
    PlatformCursorGlobals globals;
    const volatile float& loading_step_00d7a2f0;
    // Borrow the actual ShowCursor binding. Each loop captures it once before
    // changing DB8E. A concrete application supplies the real Windows export.
    NativeInputShowCursorCall const& show_cursor_00ce2344;
    NativeInputCursorCalls& calls;
};

// Actual24h action owner,30h action rows,34h bindings and14h modifiers. Changes
// only class1 cached device pointers, irrespective of enabled/resolved flags.
// Native ECX owner, stack replacement, RET4; source has an explicit receiver.
void retarget_native_mouse_input_bindings_00a92840(void* owner, void* replacement) noexcept;

// Native ECX unused, no inputs, RET. Reloads the real backend after each
// callback; captures replacement BEFORE concrete lazy004BEC00. Uses the real
// returning-capable CRT invalid-parameter boundary, never an empty-table stub.
void reset_native_input_focus_00beca40(NativeInputCursorContext&);

// Native ECX platform, stack loading byte, RET4. Canonical source platform/UI
// state is borrowed; all input receivers remain actual raw allocations.
void update_native_input_cursor_00becb20(Win32PlatformState&, bool loading,
    NativeInputCursorContext&);

// Native ECX platform, no stack arguments, RET. Current source application
// virtual10 executes before the same platform's raw-input cursor service.
void run_native_platform_application_service_00bece70(Win32PlatformState&,
    NativeInputCursorContext&, PlatformApplicationServiceHost&);

// Complete normal field/call schedules in the valid native-storage domain.
// Required external providers, original ABI/FH3/SEH/hardware-fault behavior,
// malformed ranges, concurrent mutation and full app/frame wiring are not
// supplied by these source interfaces. No new ownership is introduced.
} // namespace bsp
