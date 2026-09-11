#include "bsp/directinput_runtime.hpp"

#include <new>
#include <stdexcept>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace bsp {

const std::vector<DirectInputReference>& DirectInputReferenceRegistry::references() const noexcept {
    return references_;
}
bool DirectInputReferenceRegistry::contains(const IUnknown* object) const noexcept {
    for (const auto& reference : references_) if (reference.object == object) return true;
    return false;
}
void DirectInputReferenceRegistry::prepare(std::size_t additional) {
    if (additional > references_.max_size() - references_.size())
        throw std::length_error("DirectInput reference registry capacity exhausted");
    references_.reserve(references_.size() + additional);
}
void DirectInputReferenceRegistry::record(IUnknown& object, DirectInputReferenceOrigin origin) {
    // SDK/factory operations reserve beforehand, so ordinary nonreentrant
    // construction records its returned references without another allocation.
    references_.push_back({&object, origin});
}
void DirectInputReferenceRegistry::release_all() {
    while (!references_.empty()) {
        auto* object = references_.back().object;
        references_.pop_back();
        object->Release();
    }
}

DirectInputRuntime::DirectInputRuntime(IDirectInput8A*& actual_interface_slot,
    DirectInputReferenceRegistry& references, MouseInputGlobals& mouse_globals,
    MouseInputSample& initial_mouse_sample, JoystickInputServices& joystick_services,
    XInputApi& xinput_api, XInputDeviceGlobals& xinput_globals) noexcept
    : interface_slot_(actual_interface_slot), references_(references), mouse_globals_(mouse_globals),
      mouse_preimage_(initial_mouse_sample), joystick_services_(joystick_services),
      xinput_api_(xinput_api), xinput_globals_(xinput_globals) {}

void DirectInputRuntime::bind_enumeration_context(InputEnumerationContext& context) {
    if (&context.devices != static_cast<InputEnumerationDeviceFactory*>(this))
        throw std::invalid_argument("enumeration context must use this actual device factory");
    if (&context.backend.direct_input != static_cast<DirectInputHost*>(this))
        throw std::invalid_argument("enumeration backend must use this actual DirectInput host");
    enumeration_ = &context;
}
IDirectInput8A& DirectInputRuntime::current_interface() const {
    if (!interface_slot_) throw std::runtime_error("the actual DirectInput interface slot is null");
    return *interface_slot_;
}
bool DirectInputRuntime::create_interface(std::uint32_t version) {
    if (interface_slot_)
        throw std::logic_error("native DirectInput initial creation requires an empty interface slot");
    references_.prepare(1);
    create_result_ = DirectInput8Create(GetModuleHandleA(nullptr), version, IID_IDirectInput8A,
        reinterpret_cast<void**>(&interface_slot_), nullptr);
    // Native A9833B ignores HRESULT. The existing typed bring-up guards missing
    // output before its immediate AddRef; preserve nonnull output even on error.
    if (!interface_slot_) return false;
    references_.record(*interface_slot_, DirectInputReferenceOrigin::interface_create);
    return true;
}
void DirectInputRuntime::add_ref() {
    references_.prepare(1);
    auto& actual_interface = current_interface();
    actual_interface.AddRef();
    references_.record(actual_interface, DirectInputReferenceOrigin::interface_add_ref);
}
void DirectInputRuntime::enum_devices(std::uint32_t device_type, std::uint32_t flags) {
    if (!enumeration_) throw std::logic_error("bind the actual enumeration context before EnumDevices");
    enumeration_result_ = enumerate_direct_input_devices(*enumeration_, current_interface(),
        device_type, flags);
}
InputDevice* DirectInputRuntime::create_pad_device(int index) {
    return new (std::nothrow) XInputDevice(static_cast<std::uint32_t>(index), xinput_api_, xinput_globals_);
}
KeyboardInputDevice* DirectInputRuntime::create_keyboard_00a9a3e0() {
    references_.prepare(1);
    auto* device = create_keyboard_input_device_00a9a3e0(current_interface(), joystick_services_.window);
    if (device) references_.record(*device->direct_input, DirectInputReferenceOrigin::device_create);
    return device;
}
MouseInputDevice* DirectInputRuntime::create_mouse_00a9a290() {
    references_.prepare(1);
    auto* device = create_mouse_input_device_00a9a290(current_interface(), mouse_globals_, mouse_preimage_);
    if (device) references_.record(*device->direct_input, DirectInputReferenceOrigin::device_create);
    return device;
}
InputDevice* DirectInputRuntime::create_joystick_00a99940(const DIDEVICEINSTANCEA& instance) {
    references_.prepare(3); // one device and at most two returned effects
    auto* device = create_joystick_input_device_00a99940(current_interface(), instance, joystick_services_);
    if (device) {
        references_.record(*device->direct_input, DirectInputReferenceOrigin::device_create);
        for (auto* effect : device->effects)
            if (effect) references_.record(*effect, DirectInputReferenceOrigin::effect_create);
    }
    return device;
}
HRESULT DirectInputRuntime::last_create_result() const noexcept { return create_result_; }
HRESULT DirectInputRuntime::last_enumeration_result() const noexcept { return enumeration_result_; }
void DirectInputRuntime::release_tracked_references() {
    if (interface_slot_ && references_.contains(interface_slot_)) interface_slot_ = nullptr;
    references_.release_all();
}

} // namespace bsp
