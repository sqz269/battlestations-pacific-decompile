#pragma once

#include "bsp/input_action_classifier.hpp"
#include "bsp/platform_window.hpp"
#include "bsp/session_polls.hpp"

#include <cstdint>

namespace bsp {

// Full 004BA6D0: ECX=input backend; stack=(signed class,unsigned index), RET8.
// Select backend+6Ch+class*24h, return indexed device or null. These are the
// canonical binding vectors, NOT InputDeviceTable's fixed attachment slots.
// The class must index groups, exactly as in the native unchecked class access.
// Stable valid vectors are required; no device virtual methods are invoked.
InputDevice* get_input_class_device_004ba6d0(const InputBindingDeviceGroups&,
    std::int32_t device_class, std::uint32_t device_index) noexcept;

// Bind the actual three process byte slots. Image initial values are all zero;
// binding this projection does not initialize or replace their current state.
struct PlatformCursorGlobals {
    std::uint8_t& cursor_shown_0109db8e;
    std::uint8_t& focus_reset_pending_0109db8f;
    std::uint8_t& previous_system_ui_0109db90;
};

// Required engine/OS operations; no default callbacks or shadow manager state.
// Accessors are plain reads of current singleton bindings. A nonnull groups
// pointer means an actual backend exists and refers to its canonical vectors.
// After the initial guards, called services must preserve the singleton/device
// validity required by subsequent native dereferences. Pointers are reloaded
// at each original boundary; vectors must remain stable during the getter.
class PlatformCursorHost {
public:
    virtual ~PlatformCursorHost() = default;
    virtual PlatformManagerFlags* current_platform_manager_00f8abe8() noexcept = 0;
    virtual const InputBindingDeviceGroups* current_input_device_groups_00f8bbf4()
        noexcept = 0;
    virtual void pump_platform_manager_00a409f0(PlatformManagerFlags&) = 0;
    virtual void reset_focus_input_00beca40(Win32PlatformState&) = 0;
    virtual void set_mouse_cooperative_level_00a9a140(InputDevice&,
        std::uint32_t flags) = 0;
    // Must reload the actual current F8BBF4 backend before its virtual +4 call.
    virtual void update_current_input_backend_vslot_04(float seconds) = 0;
    // Call the real ShowCursor for a concrete Windows host, preserving its
    // signed display count. The policy owns both repeated-call loops.
    virtual int show_cursor(bool visible) = 0;
};

// Exact binary32 bytes CD CC CC 3D at D7A2F0; FLD/FSTP preserves this finite value.
inline constexpr float kCursorLoadInputStep_00d7a2f0 = 0.1f;

// Full normal 00BECB20; ECX=platform, stack loading byte, RET4 at BECCC5.
// Uses canonical platform.byte_041 and PlatformManagerFlags.system_ui_visible;
// captures both policy inputs after the optional loading pump. State changes
// survive propagated callback exceptions exactly up to the completed calls.
// This typed interface is not the original physical layout, vtable or ABI.
void update_platform_cursor_focus_00becb20(Win32PlatformState&, bool loading,
    PlatformCursorGlobals, PlatformCursorHost&);

class PlatformApplicationServiceHost {
public:
    virtual ~PlatformApplicationServiceHost() = default;
    // Invoke virtual +10 on the supplied actual platform.application object.
    // Its substantive implementation is 00737A50; no fallback frame is supplied.
    virtual void run_application_frame_vslot_10(void* application) = 0;
};

// Full 00BECE70; ECX=platform, no stack args, RET at BECE87. Captures this,
// calls its current application virtual +10, then cursor policy with false.
void run_platform_application_service_00bece70(Win32PlatformState&,
    PlatformCursorGlobals, PlatformCursorHost&, PlatformApplicationServiceHost&);

} // namespace bsp
