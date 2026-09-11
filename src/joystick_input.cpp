#include "bsp/joystick_input.hpp"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <limits>
#include <memory>
#include <new>
#include <stdexcept>

#pragma comment(lib, "dxguid.lib")

namespace bsp {
namespace {
static_assert(sizeof(DIDATAFORMAT) == 0x18);
static_assert(sizeof(DIOBJECTDATAFORMAT) == 0x10);
static_assert(offsetof(DIOBJECTDATAFORMAT, dwOfs) == 4);
static_assert(offsetof(DIOBJECTDATAFORMAT, dwType) == 8);
static_assert(offsetof(DIOBJECTDATAFORMAT, dwFlags) == 12);
static_assert(offsetof(DIDEVICEOBJECTINSTANCEA, dwType) == 0x18);
static_assert(offsetof(DIDEVICEOBJECTINSTANCEA, dwFlags) == 0x1c);
static_assert(offsetof(DIDEVICEOBJECTINSTANCEA, tszName) == 0x20);
static_assert(sizeof(DIPROPRANGE) == 0x18);
static_assert(sizeof(DIPROPDWORD) == 0x14);
static_assert(sizeof(DIEFFECT) == 0x38);
static_assert(offsetof(DIEFFECT, rgdwAxes) == 0x20);
static_assert(offsetof(DIEFFECT, lpvTypeSpecificParams) == 0x30);
static_assert(sizeof(DICONSTANTFORCE) == 4 && sizeof(DIRAMPFORCE) == 8);
static_assert(sizeof(XINPUT_STATE) == 16);
static_assert(offsetof(XINPUT_STATE, Gamepad) + offsetof(XINPUT_GAMEPAD, bLeftTrigger) == 6);

std::int32_t wrap_sub(std::int32_t a, std::int32_t b) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) -
        static_cast<std::uint32_t>(b));
}
std::int32_t wrap_mul3(std::int32_t a) noexcept {
    return static_cast<std::int32_t>(static_cast<std::uint32_t>(a) * 3u);
}
const JoystickBinding& checked_binding(const JoystickInputDevice& d, std::uint32_t code) {
    if (code >= d.bindings.size()) throw std::out_of_range("joystick binding code must be below90");
    return d.bindings[code];
}
std::size_t checked_object(const JoystickInputDevice& d, const JoystickBinding& binding) {
    const auto index = static_cast<std::size_t>(binding.object);
    if (index >= d.objects.size() || index >= d.current_state.size() ||
        index >= d.previous_state.size())
        throw std::out_of_range("joystick binding refers to unavailable native object state");
    return index;
}
const JoystickAxisRange& checked_range(const JoystickObject& object) {
    if (!object.range) throw std::runtime_error("joystick axis range was not supplied by DirectInput");
    return *object.range;
}
bool pov_direction(std::uint32_t kind, std::int32_t value) noexcept {
    switch (kind) {
    case 5: return value == 22500 || value == 27000 || value == 31500;
    case 6: return value == 4500 || value == 9000 || value == 13500;
    case 7: return value == 31500 || value == 0 || value == 4500;
    case 8: return value == 13500 || value == 18000 || value == 22500;
    default: return false;
    }
}
float axis_value_x87(std::int32_t numerator, std::int32_t span, std::uint32_t kind) {
    float result;
    const double one = 1.0;
    __asm {
        fild numerator
        fidiv span
        cmp kind,2
        jne not_full_axis
        fadd st(0),st(0)
        fsub one
        jmp store_axis
    not_full_axis:
        cmp kind,3
        jne store_axis
        fld1
        fsubrp st(1),st(0)
    store_axis:
        fstp result
    }
    return result;
}
std::int32_t apply_deadzone(std::int32_t value, std::int32_t span) {
    const std::int32_t midpoint = span / 2;
    float rounded_midpoint;
    const double deadzone = 0.20000000298023224; // CE3D10 exact double
    const double gain = 1.2500000046566129; // D5B7E8 exact double
    double threshold;
    __asm {
        fild midpoint
        fstp rounded_midpoint
        fld rounded_midpoint
        fmul deadzone
        mov eax,value
        cmp eax,midpoint
        jle negative_threshold
        fadd rounded_midpoint
        jmp store_threshold
    negative_threshold:
        fsubr rounded_midpoint
    store_threshold:
        fstp threshold
    }
    // The native BF7420 CRT boundary truncates ST0 (SSE path first stores a
    // double). Use the compiler's actual floating-to-int conversion library.
    const auto limit = static_cast<std::int32_t>(threshold);
    if ((value > midpoint && value < limit) || (value <= midpoint && value > limit))
        return midpoint;
    const auto distance = value > midpoint ? wrap_sub(value, limit) : wrap_sub(limit, value);
    double remapped;
    __asm {
        fild distance
        fmul gain
        mov eax,value
        cmp eax,midpoint
        jle negative_result
        fadd rounded_midpoint
        jmp store_result
    negative_result:
        fsubr rounded_midpoint
    store_result:
        fstp remapped
    }
    return static_cast<std::int32_t>(remapped);
}
float add_activity_window(float seconds) noexcept {
    const double duration = 60.0; // CE3D68
    float result;
    __asm {
        fld seconds
        fadd duration
        fstp result
    }
    return result;
}
float trigger_fraction(std::uint8_t trigger) noexcept {
    const std::int32_t value = trigger;
    const double maximum = 255.0;
    float result;
    __asm {
        fild value
        fdiv maximum
        fstp result
    }
    return result;
}
JoystickInputDevice& joystick(InputDevice& d) {
    auto* result = dynamic_cast<JoystickInputDevice*>(&d);
    if (!result) throw std::invalid_argument("joystick focus host requires an actual joystick projection");
    return *result;
}
}

JoystickInputDevice::JoystickInputDevice(JoystickInputServices& services) : services_(services) {
    left_trigger = services.initial_trigger_values[0];
    right_trigger = services.initial_trigger_values[1];
}

JoystickInputDevice::~JoystickInputDevice() {
    // A991F0 order: previous/current arrays, reverse object-name destruction,
    // format array, effect Unload in index order, product name, empty base tree.
    std::vector<std::int32_t>().swap(previous_state);
    std::vector<std::int32_t>().swap(current_state);
    for (auto it = objects.rbegin(); it != objects.rend(); ++it)
        it->name.release_to(services_.strings);
    std::vector<JoystickObject>().swap(objects);
    std::vector<DIOBJECTDATAFORMAT>().swap(format_objects);
    for (auto* effect : effects) if (effect) effect->Unload();
    product_name.release_to(services_.strings);
    // No Release on direct_input or either effect; these are externally owned.
    // Force-request insertion/processing is a separate, unexposed game family;
    // A95D70 constructed its empty registry with zero channel amplitudes.
}

int JoystickInputDevice::device_class() const { return 2; }
void JoystickInputDevice::on_slot_reset() {
    if (!direct_input) throw std::runtime_error("joystick reset requires its COM device");
    direct_input->SendForceFeedbackCommand(2);
    direct_input->SendForceFeedbackCommand(0x20);
    direct_input->SendForceFeedbackCommand(1);
}
void JoystickInputDevice::set_relative_binding_00a98bb0(std::uint32_t code, bool relative) {
    checked_binding(*this, code);
    bindings[code].relative = relative;
}
std::uint8_t JoystickInputDevice::query_1c(std::uint32_t code) const {
    const auto& binding = checked_binding(*this, code);
    return binding.kind != 0 && binding.relative ? 1 : 0;
}
std::uint8_t JoystickInputDevice::binding_down_00a98780(const JoystickBinding& binding) const {
    if (binding.kind == 0 || binding.kind > 8) return 0;
    const auto index = checked_object(*this, binding);
    const auto value = current_state[index];
    if (binding.kind == 1) return value != 0 ? 1 : 0;
    if (binding.kind >= 5) return pov_direction(binding.kind, value) ? 1 : 0;
    if (binding.kind == 2 && binding.relative)
        return value != previous_state[index] ? 1 : 0;
    const auto& range = checked_range(objects[index]);
    if (binding.kind == 2) return value != range.span / 2 ? 1 : 0;
    if (binding.kind == 3) return value < range.span / 4 ? 1 : 0;
    return value > wrap_mul3(range.span) / 4 ? 1 : 0;
}
float JoystickInputDevice::binding_value_00a98940(const JoystickBinding& binding) const {
    if (binding.kind == 0 || binding.kind > 8) return 0.0f;
    const auto index = checked_object(*this, binding);
    const auto value = current_state[index];
    if (binding.kind == 1) return value != 0 ? 1.0f : 0.0f;
    if (binding.kind >= 5) return pov_direction(binding.kind, value) ? 1.0f : 0.0f;
    const auto& range = checked_range(objects[index]);
    const auto midpoint = range.span / 2;
    if ((binding.kind == 2 && value == midpoint) ||
        (binding.kind == 3 && value >= midpoint) || (binding.kind == 4 && value <= midpoint))
        return 0.0f;
    return axis_value_x87(wrap_sub(value, range.minimum), range.span, binding.kind);
}
std::uint8_t JoystickInputDevice::query_20(std::uint32_t code) const {
    if (xbox_360) {
        if (code == 21 || code == 22) {
            const auto& trigger = code == 21 ? left_trigger : right_trigger;
            if (!trigger) throw std::runtime_error("Xbox trigger state is unavailable");
            return *trigger > 0.5f ? 1 : 0;
        }
        if (code == 64) return 0;
    }
    if (!valid || inhibited) return 0;
    return binding_down_00a98780(checked_binding(*this, code));
}
float JoystickInputDevice::value_24(std::uint32_t code) const {
    if (xbox_360) {
        if (code == 21 || code == 22) {
            const auto& trigger = code == 21 ? left_trigger : right_trigger;
            if (!trigger) throw std::runtime_error("Xbox trigger state is unavailable");
            return *trigger;
        }
        if (code == 64) return 0.0f;
    }
    if (!valid || inhibited) return 0.0f;
    return binding_value_00a98940(checked_binding(*this, code));
}
bool JoystickInputDevice::activity_00a93f30() const {
    for (std::uint32_t code = 0; code < 90; ++code) if (query_20(code)) return true;
    return false;
}

bool JoystickInputDevice::poll_00a98e30(float seconds) {
    (void)seconds; // Native RET4 consumes it but never reads it.
    valid = false;
    if (!direct_input) return false;
    if (FAILED(direct_input->Poll())) { direct_input->Acquire(); return false; }
    if (objects.size() != object_count || current_state.size() != object_count ||
        previous_state.size() != object_count)
        throw std::runtime_error("joystick sample storage disagrees with its native object count");
    std::copy(current_state.begin(), current_state.end(), previous_state.begin());
    if (FAILED(direct_input->GetDeviceState(data_format.dwDataSize, current_state.data()))) return false;
    valid = true;
    for (std::uint32_t i = 0; i < object_count; ++i) {
        auto& object = objects[i];
        auto& value = current_state[i];
        if (object.range) value = apply_deadzone(value, object.range->span);
        if (last_input_object != -1) continue;
        if (!object.kind) throw std::runtime_error("unclassified joystick object has no native detection kind");
        const auto kind = *object.kind;
        if (kind == 0 && value != 0) { last_input_object = i; last_input_direction = 0; }
        else if (kind == 1 && value != checked_range(object).span / 2) {
            last_input_object = i;
            last_input_direction = value >= checked_range(object).span / 2 ? 1 : 0;
        } else if (value != previous_state[i]) { last_input_object = i; last_input_direction = 0; }
        else if (kind == 2 && static_cast<std::uint16_t>(value) != 0xffff &&
            (value == 0 || value == 9000 || value == 18000 || value == 27000)) {
            last_input_object = i;
            last_input_direction = value == 27000 ? 0 : value == 9000 ? 1 : value == 0 ? 2 : 3;
        }
    }
    for (std::uint32_t code = 0; code < 90; ++code) {
        if (std::fabs(value_24(code)) > 0.25f) {
            if (!services_.current_clock_01090ab0_vslot14)
                throw std::runtime_error("joystick activity needs the current canonical clock");
            activity_deadline = add_activity_window(timestamp_seconds_x87(
                services_.current_clock_01090ab0_vslot14()));
            break;
        }
    }
    if (xbox_360) {
        if (!services_.xinput_get_state) throw std::runtime_error("joystick Xbox branch requires XINPUT1_3 ordinal2");
        XINPUT_STATE state;
        if (services_.xinput_get_state(0, &state) != ERROR_SUCCESS)
            throw std::runtime_error("XInputGetState did not supply its required trigger output");
        left_trigger = trigger_fraction(state.Gamepad.bLeftTrigger);
        right_trigger = trigger_fraction(state.Gamepad.bRightTrigger);
    }
    return true;
}

int __stdcall count_joystick_objects_00a98b90(const DIDEVICEOBJECTINSTANCEA* object, void* context) noexcept {
    if (!object || !context) return DIENUM_STOP;
    auto& device = *static_cast<JoystickInputDevice*>(context);
    if ((object->dwType & 0x1fu) != 0) ++device.object_count;
    return DIENUM_CONTINUE;
}
int describe_joystick_object_00a992f0(JoystickInputDevice& device, const DIDEVICEOBJECTINSTANCEA& object) {
    if ((object.dwType & 0x1fu) == 0) return DIENUM_CONTINUE;
    const auto index = device.described_count;
    if (index >= device.objects.size() || index >= device.format_objects.size())
        throw std::out_of_range("DirectInput object enumeration changed between passes");
    auto& metadata = device.objects[index];
    const auto length = static_cast<std::uint32_t>(std::strlen(object.tszName));
    metadata.name.resize_0041dd40(device.services_.strings, length, false);
    if (metadata.name.data()) std::memcpy(metadata.name.data(), object.tszName, length);
    auto& format = device.format_objects[index];
    format = {nullptr, index * 4u, object.dwType, object.dwFlags};
    const auto instance = static_cast<std::uint16_t>(object.dwType >> 8);
    auto set_button_binding = [&](std::size_t slot, std::uint32_t kind) {
        device.used_button_slots[slot] = 1;
        device.bindings[slot].kind = kind;
        device.bindings[slot].object = static_cast<std::int32_t>(index);
    };
    if ((object.dwType & 0xcu) != 0 && instance < 60) {
        metadata.kind = 0;
        for (std::size_t slot = 0; slot < 60; ++slot) {
            if (slot >= 4 && slot <= 7) continue;
            if (device.used_button_slots[slot] != 1) { set_button_binding(slot, 1); break; }
        }
    }
    if ((object.dwType & 3u) != 0 && instance < 30) {
        metadata.kind = 1;
        auto& axis = device.next_axis_binding;
        if (axis < 0 || axis >= 30) throw std::out_of_range("joystick axis mapping exceeds90 native bindings");
        auto& binding = device.bindings[60 + static_cast<std::size_t>(axis)];
        binding.kind = 2; binding.object = static_cast<std::int32_t>(index);
        if (axis == 1) axis = 0;
        else if (axis == 0) axis = 3;
        else if (axis == 3) axis = 2;
        else if (axis == 2) axis = 4;
        else ++axis;
    }
    if ((object.dwType & 0x10u) != 0) {
        metadata.kind = 2;
        std::size_t slot = device.used_button_slots[4] == 1 ? 20 : 4;
        for (std::uint32_t kind = 5; kind <= 8; ++kind) {
            while (slot < 60 && device.used_button_slots[slot] == 1) ++slot;
            if (slot == 60) break;
            set_button_binding(slot, kind);
        }
    }
    if ((object.dwFlags & 1u) != 0) {
        if (device.feedback_axis_offsets[0] == 0xffffffffu) {
            device.feedback_axis_offsets[0] = format.dwOfs; device.feedback_axis_count = 1;
        } else if (device.feedback_axis_offsets[1] == 0xffffffffu) {
            device.feedback_axis_offsets[1] = format.dwOfs; device.feedback_axis_count = 2;
        }
    }
    ++device.described_count;
    return DIENUM_CONTINUE;
}
int __stdcall joystick_object_callback_00a99920(const DIDEVICEOBJECTINSTANCEA* object, void* context) noexcept {
    auto* device = static_cast<JoystickInputDevice*>(context);
    if (!device || device->callback_failure) return DIENUM_STOP;
    try {
        if (!object) throw std::invalid_argument("joystick enumeration object is null");
        return describe_joystick_object_00a992f0(*device, *object);
    } catch (...) { device->callback_failure = std::current_exception(); return DIENUM_STOP; }
}

JoystickInputDevice* create_joystick_input_device_00a99940(IDirectInput8A& direct_input,
    const DIDEVICEINSTANCEA& instance, JoystickInputServices& services) {
    std::unique_ptr<JoystickInputDevice> device(new (std::nothrow) JoystickInputDevice(services));
    if (!device) return nullptr;
    const auto length = static_cast<std::uint32_t>(std::strlen(instance.tszProductName));
    device->product_name.resize_0041dd40(services.strings, length, false);
    if (device->product_name.data()) std::memcpy(device->product_name.data(), instance.tszProductName, length);
    const auto* name = device->product_name.data();
    device->xbox_360 = name && (std::strstr(name, "Xbox") || std::strstr(name, "XBOX") ||
        std::strstr(name, "XBox")) && std::strstr(name, "360");
    direct_input.CreateDevice(instance.guidInstance, &device->direct_input, nullptr);
    if (!device->direct_input) throw std::runtime_error("joystick CreateDevice did not supply a COM device");
    device->direct_input->EnumObjects(&count_joystick_objects_00a98b90, device.get(), 0);
    device->objects.resize(device->object_count);
    device->previous_state.assign(device->object_count, services.initial_object_value);
    device->current_state.assign(device->object_count, services.initial_object_value);
    device->format_objects.resize(device->object_count);
    device->data_format = {sizeof(DIDATAFORMAT), sizeof(DIOBJECTDATAFORMAT), 1,
        device->object_count * 4u, device->object_count, device->format_objects.data()};
    device->direct_input->EnumObjects(&joystick_object_callback_00a99920, device.get(), 0);
    if (device->callback_failure) std::rethrow_exception(device->callback_failure);
    if (device->described_count != device->object_count)
        throw std::runtime_error("DirectInput object count changed between enumeration passes");
    device->direct_input->SetDataFormat(&device->data_format);
    for (std::uint32_t i = 0; i < device->described_count; ++i) {
        DIPROPRANGE range{};
        range.diph = {sizeof(DIPROPRANGE), sizeof(DIPROPHEADER), i * 4u, DIPH_BYOFFSET};
        if (device->direct_input->GetProperty(DIPROP_RANGE, &range.diph) == DI_OK)
            device->objects[i].range = JoystickAxisRange{range.lMin, range.lMax, wrap_sub(range.lMax, range.lMin)};
    }
    device->direct_input->SetCooperativeLevel(services.window.current_platform_window_00bec230(), 5);
    if (device->feedback_axis_count) {
        DIPROPDWORD auto_center{};
        auto_center.diph = {sizeof(DIPROPDWORD), sizeof(DIPROPHEADER), 0, DIPH_DEVICE};
        device->direct_input->SetProperty(DIPROP_AUTOCENTER, &auto_center.diph);
        LONG directions[2]{};
        DICONSTANTFORCE constant{};
        DIRAMPFORCE ramp{};
        DIEFFECT effect{};
        effect.dwSize = sizeof(DIEFFECT); effect.dwFlags = 0x12;
        effect.dwDuration = 0xffffffffu; effect.dwGain = 10000;
        effect.dwTriggerButton = 0xffffffffu; effect.cAxes = device->feedback_axis_count;
        effect.rgdwAxes = device->feedback_axis_offsets.data(); effect.rglDirection = directions;
        effect.cbTypeSpecificParams = device->effect_kind == 0 ? sizeof(constant) : sizeof(ramp);
        effect.lpvTypeSpecificParams = device->effect_kind == 0 ? static_cast<void*>(&constant) : &ramp;
        for (auto& output : device->effects) {
            if (device->effect_kind == 0) device->direct_input->CreateEffect(GUID_ConstantForce, &effect, &output, nullptr);
            else if (device->effect_kind == 1) device->direct_input->CreateEffect(GUID_RampForce, &effect, &output, nullptr);
        }
    }
    device->poll_00a98e30(0.0f);
    device->poll_00a98e30(0.0f);
    return device.release();
}
void delete_joystick_input_device_00a99900(JoystickInputDevice& device, std::uint32_t flags) {
    if ((flags & 1u) != 0) delete &device;
    else device.~JoystickInputDevice();
}
std::int32_t JoystickFocusDeviceHost::query_identifier_vslot34(InputDevice& device) {
    joystick(device); return input_device_identifier_zero_00a93eb0();
}
void JoystickFocusDeviceHost::delete_device_vslot04(InputDevice& device, std::uint32_t flags) {
    delete_joystick_input_device_00a99900(joystick(device), flags);
}
void JoystickFocusDeviceHost::poll_device_vslot10(InputDevice& device, float seconds) {
    joystick(device).poll_00a98e30(seconds);
}
bool JoystickFocusDeviceHost::activity_vslot28(InputDevice& device) {
    return joystick(device).activity_00a93f30();
}

} // namespace bsp
