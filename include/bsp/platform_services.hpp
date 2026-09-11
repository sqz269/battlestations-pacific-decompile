#pragma once

#include "bsp/app_frame.hpp"
#include "bsp/input_focus_reset.hpp"
#include "bsp/platform_cursor.hpp"
#include "bsp/resource_load_events.hpp"
#include "bsp/xlive_library.hpp"
#include "bsp/xlive_system_pump.hpp"

namespace bsp {

struct XLiveManagerOwner;

// Concrete native call composition. Lookup, reset and tick all reload the same
// published input binding. InputFocusResetHost supplies only the genuine lazy
// action-manager getter. The caller owns construction, registration, lifetime
// and any asynchronous SDK work.
class PlatformServices final : public PlatformCursorHost,
    public ResourceLoadEventHost, public PlatformLoopCallbacks,
    private InputFocusResetHost {
public:
    PlatformServices(Win32PlatformState&, PlatformCursorGlobals,
        InputFocusBackendState* volatile& current_input,
        XLiveSystemPumpContext* volatile& current_online,
        InputFocusResetHost&, InputFocusDeviceHost&,
        PlatformApplicationServiceHost&, XLiveLibrary&) noexcept;
    // Full owner construction publishes this one F8ABE8 slot. Every lookup
    // derives the current context from that owner, including callback changes.
    PlatformServices(Win32PlatformState&, PlatformCursorGlobals,
        InputFocusBackendState* volatile& current_input,
        XLiveManagerOwner* volatile& current_online_owner,
        InputFocusResetHost&, InputFocusDeviceHost&,
        PlatformApplicationServiceHost&, XLiveLibrary&) noexcept;
    bool pretranslate(MSG&) override;
    void frame() override;
    void update_cursor_focus_00becb20(bool loading) override;
    PlatformManagerFlags* current_platform_manager_00f8abe8() noexcept override;
    const InputBindingDeviceGroups* current_input_device_groups_00f8bbf4()
        noexcept override;
    void pump_platform_manager_00a409f0(PlatformManagerFlags&) override;
    void reset_focus_input_00beca40(Win32PlatformState&) override;
    void set_mouse_cooperative_level_00a9a140(InputDevice&, std::uint32_t) override;
    void update_current_input_backend_vslot_04(float seconds) override;
    int show_cursor(bool visible) override;
private:
    InputFocusBackendState* current_backend_00f8bbf4() override;
    InputTickState& input_manager_004bec00() override;
    XLiveSystemPumpContext* current_online_context() const noexcept;
    Win32PlatformState& platform_;
    PlatformCursorGlobals cursor_;
    InputFocusBackendState* volatile& input_;
    XLiveSystemPumpContext* volatile* online_{};
    XLiveManagerOwner* volatile* online_owner_{};
    InputFocusResetHost& reset_;
    InputFocusDeviceHost& devices_;
    PlatformApplicationServiceHost& application_;
    XLiveLibrary& library_;
};

// Binds a real application identity to its existing frame fields/host. A
// mismatched identity is an invalid host binding, not a native null fallback.
class ApplicationFrameService final : public PlatformApplicationServiceHost {
public:
    ApplicationFrameService(void* actual_application, ApplicationFrameState&,
        FrameMarkerColor&, ApplicationFrameHost&) noexcept;
    void run_application_frame_vslot_10(void* application) override;
private:
    void* application_;
    ApplicationFrameState& frame_;
    FrameMarkerColor& marker_;
    ApplicationFrameHost& host_;
};

} // namespace bsp
