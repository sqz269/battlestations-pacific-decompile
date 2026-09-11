#pragma once

#include "bsp/input_device_state.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/platform_window.hpp"

#include <array>

namespace bsp {

// Native optional +D8 callback receives class in ECX, index in EDX, no stack
// arguments. Removal passes index=-1; append passes its previous vector count.
using InputBackendDevicesChanged = void (__fastcall*)(std::int32_t, std::int32_t);

// References to the application's EXISTING input backend data. Active vectors
// differ from the fixed 3x8 owning table. No duplicate groups or slot arrays.
// This view is not the native backend's layout, vtable or singleton lifetime.
struct InputFocusBackendState {
    InputDeviceTable& slots; // +04..+60
    InputBindingDeviceGroups& groups; // +6C + class*24h
    std::array<std::vector<std::int32_t>, 3>& accepted_device_ids; // +7C + class*24h
    std::array<std::int32_t, 3>& requested_active_counts; // +68 + class*24h
    bool& accept_inactive_gamepads_64;
    bool& bindings_dirty_d4;
    InputBackendDevicesChanged& devices_changed_d8;
    bool& xbox_360_present_f4;
    // EnumDevices must invoke the actual enumeration/attachment callback for
    // these same slots, preserving the established DirectInputHost contract.
    DirectInputHost& direct_input;
};

// Native polymorphic device calls; game sequence/filter logic is implemented
// below. The keyboard/mouse implementation delegates to canonical real polls.
class InputFocusDeviceHost {
public:
    virtual ~InputFocusDeviceHost() = default;
    virtual std::int32_t query_identifier_vslot34(InputDevice&) = 0;
    virtual void delete_device_vslot04(InputDevice&, std::uint32_t flags) = 0;
    virtual void poll_device_vslot10(InputDevice&, float seconds) = 0;
    virtual bool activity_vslot28(InputDevice&) = 0;
};

// Supplies repeated reads of F8BBF4 and the genuine lazy action-manager getter.
// input_manager_004bec00 must honor existing singleton/lifetime ownership; a
// newly fabricated independent action table is not an implementation.
class InputFocusResetHost {
public:
    virtual ~InputFocusResetHost() = default;
    virtual InputFocusBackendState* current_backend_00f8bbf4() = 0;
    virtual InputTickState& input_manager_004bec00() = 0;
};

HWND get_platform_window_00bec230(const Win32PlatformState&) noexcept;
// ECX mouse, stack flags, RET4. Native COM+34(this,HWND,flags), HRESULT ignored;
// configured+234 becomes true even on failure. Nonnull actual COM device required.
void set_mouse_cooperative_level_00a9a140(MouseInputDevice&, HWND,
    std::uint32_t flags);
// Native mouse+4 deleting destructor A9A390: no COM Release occurs. Only
// standard-new typed mice may be passed with flags1; flags0 destroys in place.
void delete_mouse_input_device_00a9a390(MouseInputDevice&, std::uint32_t flags);
std::int32_t input_device_identifier_zero_00a93eb0() noexcept;

// ECX backend, device stack, RET4. Erase first match in its class's active
// vector, set +D4 before erasure, then optional fastcall(class,-1).
void remove_active_input_device_00a90ee0(InputFocusBackendState&, InputDevice&);
// ECX backend, class stack, RET4. Eight slots in order; deleting vslot+4(1)
// then clear that SAME slot, including any pointer written by the callback.
void delete_input_device_class_00bebf30(InputFocusBackendState&, std::int32_t,
    InputFocusDeviceHost&);
// ECX backend, RET. Clear +F4, EnumDevices(type0,flags1) through real host.
void enumerate_input_devices_00a983c0(InputFocusBackendState&);
// ECX backend, class/slot stack, RET8. Scan ALL accepted IDs; -1 accepts
// without querying, other IDs compare device+34. Append an absent pointer once.
void activate_input_device_slot_00a91620(InputFocusBackendState&, std::int32_t,
    std::int32_t, InputFocusDeviceHost&);
// ECX action singleton, replacement pointer stack, RET4. Rewrites cached
// class1 pointers in all primary/required/forbidden bindings, enabled or not.
void retarget_mouse_input_bindings_00a92840(InputTickState&, InputDevice*) noexcept;
// ECX unused, RET. First active device is class1/MOUSE (+94/+98), not keyboard.
// Only the initial absent backend is null-safe; later native-required pointers
// are explicit host guards. Capture final mouse before lazy manager getter.
void reset_focus_input_00beca40(InputFocusResetHost&, InputFocusDeviceHost&);

bool input_class_at_requested_count_00a90490(const InputFocusBackendState&,
    std::int32_t device_class);
void input_backend_pre_tick_00a97390() noexcept; // verified bare RET
// ECX backend, float seconds stack, RET4. Prepass then 3x8 poll/activation walk.
// Joystick activity is queried only for class2 when backend+64 is false.
void update_input_backend_00a918a0(InputFocusBackendState&, float seconds,
    InputFocusDeviceHost&);

// Actual keyboard/mouse +10 polls use the same live platform settings fields
// as the rest of input. Their +34 identity returns zero. Mouse deletion follows
// A9A390 and deliberately leaves its borrowed COM pointer's ownership external.
// Other device operations require an explicit delegate, otherwise throw.
class KeyboardMouseFocusDeviceHost final : public InputFocusDeviceHost {
public:
    explicit KeyboardMouseFocusDeviceHost(Win32PlatformState&,
        InputFocusDeviceHost* other_devices = nullptr) noexcept;
    std::int32_t query_identifier_vslot34(InputDevice&) override;
    void delete_device_vslot04(InputDevice&, std::uint32_t flags) override;
    void poll_device_vslot10(InputDevice&, float seconds) override;
    bool activity_vslot28(InputDevice&) override;
private:
    Win32PlatformState& platform_;
    InputFocusDeviceHost* other_devices_;
};

} // namespace bsp
