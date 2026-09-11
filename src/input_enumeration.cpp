#include "bsp/input_enumeration.hpp"

#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

#include <cstring>
#include <memory>
#include <new>
#include <stdexcept>
#include <string>

// SDK data formats and GUIDs, not locally fabricated DirectInput structures.
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace bsp {
namespace {
static_assert(sizeof(GUID) == sizeof(InputInstanceGuid));
static_assert(offsetof(DIDEVICEINSTANCEA, guidInstance) == 4);
static_assert(offsetof(DIDEVICEINSTANCEA, dwDevType) == 0x24);
static_assert(offsetof(DIDEVICEINSTANCEA, tszProductName) == 0x12c);

struct ScopedProductName {
    NativeString value;
    NativeStringStorage& storage;
    explicit ScopedProductName(NativeStringStorage& s) : storage(s) {}
    ~ScopedProductName() { value.release_to(storage); }
};

void attach_created_device(InputDeviceTable& table, InputDevice* device, int slot) {
    std::string error;
    if (!table.attach(device, slot, error))
        throw std::runtime_error("enumerated input device cannot be attached: " + error);
}

struct RawStorageDelete {
    void operator()(void* block) const noexcept { ::operator delete(block); }
};
}

bool input_instance_guids_equal_00a972e0(const InputInstanceGuid& first,
    const InputInstanceGuid& second) noexcept {
    return std::memcmp(first.data(), second.data(), first.size()) == 0;
}

int on_input_device_enumerated_00a98030(InputEnumerationContext& context,
    const ::DIDEVICEINSTANCEA& instance) {
    auto& slots = context.backend.slots;
    const auto type = instance.dwDevType & 0xffu;
    if (type == DI8DEVTYPE_KEYBOARD) {
        if (!slots.slot(0, 0))
            attach_created_device(slots, context.devices.create_keyboard_00a9a3e0(), 0);
        return DIENUM_CONTINUE;
    }
    if (type == DI8DEVTYPE_MOUSE) {
        if (!slots.slot(1, 0))
            attach_created_device(slots, context.devices.create_mouse_00a9a290(), 0);
        return DIENUM_CONTINUE;
    }
    if (type != DI8DEVTYPE_JOYSTICK && type != DI8DEVTYPE_GAMEPAD)
        return DIENUM_CONTINUE;

    ScopedProductName product(context.strings);
    product.value.assign_0041e870(context.strings, instance.tszProductName);
    const auto* name = product.value.data();
    const bool is_xbox_360 = name &&
        (std::strstr(name, "Xbox") || std::strstr(name, "XBOX") ||
            std::strstr(name, "XBox")) && std::strstr(name, "360");
    if (is_xbox_360) context.backend.xbox_360_present_f4 = true;

    InputInstanceGuid instance_guid;
    std::memcpy(instance_guid.data(), &instance.guidInstance, instance_guid.size());
    auto& seen = context.seen_instance_guids;
    std::size_t index = 0;
    for (; index < seen.size(); ++index)
        if (input_instance_guids_equal_00a972e0(seen[index], instance_guid)) break;
    if (index != seen.size()) return DIENUM_CONTINUE;

    // 00A97FA0 is the native GUID-vector append. Use the standard container;
    // do not reproduce its allocation/growth/iterator library implementation.
    seen.push_back(instance_guid);
    if (is_xbox_360) return DIENUM_CONTINUE;
    // The insertion index, including skipped Xbox entries, is the slot. Native
    // reads beyond class2 for index>=8; this projection reports that boundary.
    if (index >= static_cast<std::size_t>(input_device_slot_count))
        throw std::out_of_range("enumerated GUID index exceeds the native eight joystick slots");
    const auto slot = static_cast<int>(index);
    if (!slots.slot(2, slot))
        attach_created_device(slots,
            context.devices.create_joystick_00a99940(instance), slot);
    return DIENUM_CONTINUE;
}

int __stdcall input_enum_devices_callback_00a982b0(
    const ::DIDEVICEINSTANCEA* instance, void* opaque) noexcept {
    auto* context = static_cast<InputEnumerationContext*>(opaque);
    if (!context) return DIENUM_STOP;
    if (context->callback_failure) return DIENUM_STOP;
    try {
        if (!instance) throw std::invalid_argument("DirectInput enumeration instance is null");
        return on_input_device_enumerated_00a98030(*context, *instance);
    } catch (...) {
        context->callback_failure = std::current_exception();
        return DIENUM_STOP;
    }
}

std::int32_t enumerate_direct_input_devices(InputEnumerationContext& context,
    ::IDirectInput8A& direct_input, std::uint32_t device_type, std::uint32_t flags) {
    context.callback_failure = nullptr;
    const auto result = direct_input.EnumDevices(device_type,
        &input_enum_devices_callback_00a982b0, &context, flags);
    if (context.callback_failure) std::rethrow_exception(context.callback_failure);
    return result;
}

KeyboardInputDevice* create_keyboard_input_device_00a9a3e0(
    ::IDirectInput8A& direct_input, InputEnumerationWindowHost& platform) {
    // The canonical constructor clears current/previous/state (A962F0's three
    // 100h arrays). The typed object has no native refcount or vtable image.
    std::unique_ptr<KeyboardInputDevice> device(
        new (std::nothrow) KeyboardInputDevice(nullptr));
    if (!device) return nullptr;
    direct_input.CreateDevice(GUID_SysKeyboard, &device->direct_input, nullptr);
    if (!device->direct_input)
        throw std::runtime_error("keyboard CreateDevice did not supply its required COM device");
    device->direct_input->SetDataFormat(&c_dfDIKeyboard);
    const auto window = platform.current_platform_window_00bec230();
    device->direct_input->SetCooperativeLevel(window, 6);
    return device.release();
}

MouseInputDevice* create_mouse_input_device_00a9a290(
    ::IDirectInput8A& direct_input, MouseInputGlobals& globals,
    MouseInputSample initial_sample) {
    // Allocate before CreateDevice, but delay the existing typed constructor's
    // system-settings queries until after SetDataFormat, as A9A32A..A9A345.
    std::unique_ptr<void, RawStorageDelete> memory(
        ::operator new(sizeof(MouseInputDevice), std::nothrow));
    if (!memory) return nullptr;
    ::IDirectInputDevice8A* com_device = nullptr;
    direct_input.CreateDevice(GUID_SysMouse, &com_device, nullptr);
    if (!com_device)
        throw std::runtime_error("mouse CreateDevice did not supply its required COM device");
    com_device->SetDataFormat(&c_dfDIMouse2);
    auto* result = new (memory.get()) MouseInputDevice(com_device, globals, initial_sample);
    memory.release();
    return result;
}

} // namespace bsp
