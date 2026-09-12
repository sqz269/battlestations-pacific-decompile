#include "bsp/native_keyboard_mouse.hpp"
#include "bsp/input_focus_reset.hpp"
#include "bsp/native_ref_counted.hpp"

#include <cstring>
#include <new>
#include <stdexcept>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

namespace bsp {
namespace {
static_assert(sizeof(void*) == 4, "native input storage requires Win32");
static_assert(sizeof(DIMOUSESTATE2) == 0x14);
static_assert(offsetof(DIMOUSESTATE2, rgbButtons) == 0x0c);

template<class T> T read(const void* object, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
std::byte* bytes(void* object, std::size_t offset) noexcept {
    return static_cast<std::byte*>(object) + offset;
}
IDirectInputDevice8A* device(void* object, std::size_t offset) {
    auto* result = read<IDirectInputDevice8A*>(object, offset);
    if (!result) throw std::invalid_argument("native input requires its current DirectInput device");
    return result;
}
Win32PlatformState& platform(NativeKeyboardMouseContext& context) {
    auto* result = context.platform_0109cf04;
    if (!result) throw std::invalid_argument("native input requires published Win32 platform");
    return *result;
}

// The native callers capture the VTABLE before BEC230 and read slot34 from that
// captured table afterward, while reloading the actual COM this argument.
void cooperate(void* object, std::size_t offset, DWORD flags,
    NativeKeyboardMouseContext& context) {
    const auto* table = read<const std::byte*>(device(object, offset), 0);
    const auto window = get_platform_window_00bec230(platform(context));
    using Call = HRESULT (__stdcall*)(IDirectInputDevice8A*, HWND, DWORD);
    const auto call = read<Call>(table, 0x34);
    call(device(object, offset), window, flags);
}

float click_seconds(DWORD milliseconds, NativeKeyboardMouseGlobals& globals) {
    // Unsigned return is converted with signed FILD then conditional 2^32 bias.
    // Direct volatile-address reads keep each original x87 instruction boundary.
    auto* bias = &globals.unsigned_bias_00ce3978;
    auto* divisor = &globals.millisecond_divisor_00ce47a0;
    float result;
    __asm {
        fild milliseconds
        cmp milliseconds,0
        jge divide_ms
        mov eax,bias
        fadd dword ptr [eax]
    divide_ms:
        mov eax,divisor
        fdiv qword ptr [eax]
        fstp result
    }
    return result;
}
void system_settings(void* object, NativeKeyboardMouseGlobals& globals) {
    write<std::uint8_t>(object, 0x235, GetSystemMetrics(SM_SWAPBUTTON) != 0);
    const auto milliseconds = GetDoubleClickTime();
    write(object, 0x238, click_seconds(milliseconds, globals));
}
float axis_float(std::int32_t value, bool negate) noexcept {
    float result;
    __asm {
        mov eax,value
        cmp negate,0
        je convert_axis
        neg eax
    convert_axis:
        cvtsi2ss xmm0,eax
        movss result,xmm0
    }
    return result;
}
float subtract_axis(float axis, volatile float& negative_zero) noexcept {
    auto* zero = &negative_zero;
    float result;
    __asm {
        mov eax,zero
        movss xmm1,dword ptr [eax]
        subss xmm1,axis
        movss result,xmm1
    }
    return result;
}
float scale_axis(float axis, NativeKeyboardMouseGlobals& globals) noexcept {
    auto* scale = &globals.mouse_scale_00e12fb0;
    auto* divisor = &globals.axis_divisor_00d7a220;
    float result;
    __asm {
        mov eax,scale
        fld dword ptr [eax]
        mov eax,divisor
        fdiv qword ptr [eax]
        fmul axis
        fstp result
    }
    return result;
}
std::uint8_t axis_down(float value) noexcept {
    std::uint8_t result;
    // Preserve the x87 unordered comparison and its exception domain; an SSE
    // C++ comparison would instead use MXCSR. Native unordered counts as down.
    __asm {
        fld value
        fldz
        fxch
        fucomip st(0),st(1)
        fstp st(0)
        lahf
        test ah,044h
        jnp equal_value
        mov result,1
        jmp compared_value
    equal_value:
        mov result,0
    compared_value:
    }
    return result;
}
void reset_base_profile(void* object) noexcept {
    // A95E60/A99EF0 each tail the shared root implementation.
    write<std::uint32_t>(object, 0, 0x00d5b638);
    destroy_native_ref_counted_base_00bd30f0(object);
}
}

void* construct_native_keyboard_base_00a962f0(void* storage) noexcept {
    write<std::uint32_t>(storage, 0, 0x00ceb130);
    write<std::uint32_t>(storage, 4, 1);
    std::memset(bytes(storage, 0x0c), 0, 0x100);
    std::memset(bytes(storage, 0x10c), 0, 0x100);
    write<std::uint32_t>(storage, 0, 0x00d5b6c4);
    std::memset(bytes(storage, 0x20c), 0, 0x100);
    return storage;
}
void* construct_native_keyboard_00a9a3e0(void* storage, IDirectInput8A& input,
    NativeInputDeviceSdk& sdk, NativeKeyboardMouseContext& context) {
    construct_native_keyboard_base_00a962f0(storage);
    write(storage, 0, native_keyboard_profile);
    try {
        sdk.create_device(input, GUID_SysKeyboard,
            reinterpret_cast<IDirectInputDevice8A**>(bytes(storage, 0x30c)));
        device(storage, 0x30c)->SetDataFormat(&c_dfDIKeyboard);
        cooperate(storage, 0x30c, 6, context);
    } catch (...) {
        destroy_native_keyboard_base_00a95e60(storage);
        throw;
    }
    return storage;
}
void* construct_native_mouse_00a9a290(void* storage, IDirectInput8A& input,
    NativeInputDeviceSdk& sdk, NativeKeyboardMouseContext& context) {
    write<std::uint32_t>(storage, 0, 0x00ceb130);
    write<std::uint32_t>(storage, 4, 1);
    std::memset(bytes(storage, 0x0c), 0, 0x100);
    std::memset(bytes(storage, 0x10c), 0, 0x100);
    write(storage, 0, native_mouse_profile);
    write<std::uint8_t>(storage, 0x210, 0);
    write<std::uint32_t>(storage, 0x230, 0);
    write<std::uint32_t>(storage, 0x22c, 0);
    write<std::uint32_t>(storage, 0x228, 0);
    try {
        sdk.create_device(input, GUID_SysMouse,
            reinterpret_cast<IDirectInputDevice8A**>(bytes(storage, 0x20c)));
        device(storage, 0x20c)->SetDataFormat(&c_dfDIMouse2);
        write<std::uint8_t>(storage, 0x234, 0);
        system_settings(storage, context.globals);
    } catch (...) {
        destroy_native_mouse_base_00a99ef0(storage);
        throw;
    }
    return storage;
}
void destroy_native_keyboard_base_00a95e60(void* object) noexcept { reset_base_profile(object); }
void destroy_native_mouse_base_00a99ef0(void* object) noexcept { reset_base_profile(object); }
void* delete_native_keyboard_00a9a470(void* object, std::uint8_t flags) noexcept {
    void* identity = object;
    write(object, 0, native_keyboard_profile);
    destroy_native_keyboard_base_00a95e60(object);
    if ((flags & 1) != 0) ::operator delete(object);
    return identity;
}
void* delete_native_mouse_00a9a390(void* object, std::uint8_t flags) noexcept {
    void* identity = object;
    reset_base_profile(object);
    if ((flags & 1) != 0) ::operator delete(object);
    return identity;
}
std::int32_t native_keyboard_class_00a96350() noexcept { return 0; }
std::int32_t native_mouse_class_00a9a0f0() noexcept { return 1; }
void update_native_input_history_00a99e90(void* object, NativeInputHistoryHost& host) {
    for (std::uint32_t code = 0; code != 256; ++code) {
        write(object, 0x10c + code, read<std::uint8_t>(object, 0x0c + code));
        const auto value = host.query_20(object, code);
        write(object, 0x0c + code, value);
    }
}
std::uint8_t query_native_keyboard_00a95e70(void* object, std::uint32_t code) noexcept {
    return read<std::uint8_t>(object, 0x20c + code) != 0;
}
float value_native_keyboard_00a95e90(void* object, std::uint32_t code,
    NativeKeyboardMouseContext& context) {
    if (context.dispatch.query_20(object, code) != 0) return context.globals.one_00d7a24c;
    return 0.0f;
}
std::uint8_t query_native_mouse_00a99f70(void* object, std::uint32_t code,
    NativeKeyboardMouseContext& context) {
    if (read<std::uint8_t>(object, 0x210) == 0) return 0;
    if (code <= 7) {
        if (read<std::uint8_t>(object, 0x235) != 0 && code <= 1) code = code == 0;
        return read<std::uint8_t>(object, 0x220 + code) >> 7;
    }
    if (code - 8 <= 2) return axis_down(context.dispatch.value_24(object, code));
    return 0;
}
float value_native_mouse_00a99fe0(void* object, std::uint32_t code,
    NativeKeyboardMouseGlobals& globals) {
    if (read<std::uint8_t>(object, 0x210) == 0) return 0.0f;
    if (read<std::uint8_t>(object, 0x235) != 0 && code <= 1) code = code == 0;
    if (code <= 7) return (read<std::uint8_t>(object, 0x220 + code) & 0x80) != 0
        ? globals.one_00d7a24c : 0.0f;
    float axis = 0.0f;
    if (code == 8) axis = axis_float(read<std::int32_t>(object, 0x214), false);
    else if (code == 9) {
        const bool invert = globals.invert_y_00f8bc04 != 0;
        axis = axis_float(read<std::int32_t>(object, 0x218), false);
        if (invert) axis = subtract_axis(axis, globals.negative_zero_00d7a208);
    } else if (code == 10) axis = axis_float(read<std::int32_t>(object, 0x21c), true);
    return scale_axis(axis, globals);
}
bool poll_native_keyboard_00a9a4a0(void* object, NativeKeyboardMouseContext& context) {
    auto* state = bytes(object, 0x20c);
    std::memset(state, 0, 0x100);
    if (!read<IDirectInputDevice8A*>(object, 0x30c)) return false;
    if (FAILED(device(object, 0x30c)->Poll())) {
        device(object, 0x30c)->Acquire();
        return false;
    }
    device(object, 0x30c)->GetDeviceState(0x100, state);
    for (std::size_t i = 0; i != 0x100; ++i)
        write<std::uint8_t>(state, i, read<std::uint8_t>(state, i) >> 7);
    if (platform(context).byte_170) std::memset(state, 0, 0x100);
    update_native_input_history_00a99e90(object, context.dispatch);
    return true;
}
bool poll_native_mouse_00a9a180(void* object, NativeKeyboardMouseContext& context) {
    auto* initial_device = read<IDirectInputDevice8A*>(object, 0x20c);
    write<std::uint8_t>(object, 0x210, 0);
    if (!initial_device) return false;
    if (read<std::uint8_t>(object, 0x234) == 0) {
        cooperate(object, 0x20c, 5, context);
        write<std::uint8_t>(object, 0x234, 1);
    }
    auto& current_platform = platform(context);
    if (current_platform.settings_changed_2c) {
        current_platform.settings_changed_2c = false;
        system_settings(object, context.globals);
    }
    if (FAILED(device(object, 0x20c)->Poll())) {
        device(object, 0x20c)->Acquire();
        return false;
    }
    const auto result = device(object, 0x20c)->GetDeviceState(0x14, bytes(object, 0x214));
    if (SUCCEEDED(result)) write<std::uint8_t>(object, 0x210, 1);
    // GetDeviceState failure still adds all retained/partially written axes.
    const auto x = read<std::uint32_t>(object, 0x214);
    write(object, 0x228, read<std::uint32_t>(object, 0x228) + x);
    const auto y = read<std::uint32_t>(object, 0x218);
    const auto z = read<std::uint32_t>(object, 0x21c);
    write(object, 0x22c, read<std::uint32_t>(object, 0x22c) + y);
    write(object, 0x230, read<std::uint32_t>(object, 0x230) + z);
    update_native_input_history_00a99e90(object, context.dispatch);
    return true;
}
void set_native_mouse_cooperative_level_00a9a140(void* object, std::uint32_t flags,
    NativeKeyboardMouseContext& context) {
    cooperate(object, 0x20c, flags, context);
    write<std::uint8_t>(object, 0x234, 1);
}

} // namespace bsp
