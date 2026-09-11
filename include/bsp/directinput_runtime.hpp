#pragma once

#include "bsp/input_enumeration.hpp"
#include "bsp/joystick_input.hpp"
#include "bsp/xinput_device.hpp"

namespace bsp {

enum class DirectInputReferenceOrigin {
    interface_create, interface_add_ref, device_create, effect_create
};
struct DirectInputReference {
    IUnknown* object;
    DirectInputReferenceOrigin origin;
};

// Tracks each actual returned/added COM reference, without adding another one.
// It deliberately does not Release on destruction: the caller chooses the final
// release boundary after every borrowed wrapper and callback has stopped using
// the objects. One registry belongs to one runtime's reference acquisitions.
class DirectInputReferenceRegistry {
public:
    DirectInputReferenceRegistry() = default;
    DirectInputReferenceRegistry(const DirectInputReferenceRegistry&) = delete;
    DirectInputReferenceRegistry& operator=(const DirectInputReferenceRegistry&) = delete;
    const std::vector<DirectInputReference>& references() const noexcept;
    bool contains(const IUnknown*) const noexcept;
    void release_all(); // reverse acquisition order, actual IUnknown::Release
private:
    std::vector<DirectInputReference> references_;
    void prepare(std::size_t additional);
    void record(IUnknown&, DirectInputReferenceOrigin);
    friend class DirectInputRuntime;
};

// Concrete SDK host plus the same enumeration context's device factory. This
// binds existing state; it does not own/copy the canonical device table/groups.
// All referenced services/preimage storage must outlive the runtime and wrappers.
class DirectInputRuntime final : public DirectInputHost, public InputEnumerationDeviceFactory {
public:
    DirectInputRuntime(IDirectInput8A*& actual_interface_slot,
        DirectInputReferenceRegistry&, MouseInputGlobals&, MouseInputSample& initial_mouse_sample,
        JoystickInputServices&, XInputApi&, XInputDeviceGlobals&) noexcept;
    DirectInputRuntime(const DirectInputRuntime&) = delete;
    DirectInputRuntime& operator=(const DirectInputRuntime&) = delete;
    void bind_enumeration_context(InputEnumerationContext&);

    bool create_interface(std::uint32_t version) override;
    void add_ref() override;
    void enum_devices(std::uint32_t device_type, std::uint32_t flags) override;
    InputDevice* create_pad_device(int index) override;
    KeyboardInputDevice* create_keyboard_00a9a3e0() override;
    MouseInputDevice* create_mouse_00a9a290() override;
    InputDevice* create_joystick_00a99940(const DIDEVICEINSTANCEA&) override;

    HRESULT last_create_result() const noexcept;
    HRESULT last_enumeration_result() const noexcept;
    // Explicit final cleanup only; never called by runtime/wrapper destruction.
    // Clears the bound interface slot if it names a tracked object, then releases
    // all recorded references. Wrappers must already be destroyed or detached.
    void release_tracked_references();

private:
    IDirectInput8A& current_interface() const;
    IDirectInput8A*& interface_slot_;
    DirectInputReferenceRegistry& references_;
    MouseInputGlobals& mouse_globals_;
    MouseInputSample& mouse_preimage_;
    JoystickInputServices& joystick_services_;
    XInputApi& xinput_api_;
    XInputDeviceGlobals& xinput_globals_;
    InputEnumerationContext* enumeration_{};
    HRESULT create_result_{E_PENDING};
    HRESULT enumeration_result_{E_PENDING};
};

} // namespace bsp
