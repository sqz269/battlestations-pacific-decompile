#include "bsp/xinput_device.hpp"

#include <cmath>
#include <cstring>
#include <filesystem>
#include <stdexcept>

namespace bsp {
namespace {
static_assert(sizeof(XINPUT_STATE) == 16 && sizeof(XINPUT_VIBRATION) == 4);
static_assert(offsetof(XINPUT_STATE, Gamepad) == 4);
static_assert(offsetof(XINPUT_GAMEPAD, sThumbLX) == 4);
static_assert(offsetof(XINPUT_GAMEPAD, sThumbRY) == 10);

float trigger_value(std::int32_t value) {
    const double divisor = 255.0;
    float result;
    __asm {
        fild value
        fdiv divisor
        fstp result
    }
    return result;
}
float axis_value(std::int32_t value, bool negate) {
    const double scale = 0.000030517578125;
    float result;
    __asm {
        fild value
        cmp negate, 0
        je positive
        fchs
    positive:
        fmul scale
        fstp result
    }
    return result;
}

std::int16_t deadzone_axis(std::int16_t sample, bool sse2) {
    if (sample > -6553 && sample < 6553) return 0;
    const std::int32_t adjusted = sample > 0 ? sample - 6553 : sample + 6553;
    const double scale = 1.249980926513671875; // native D5BB90, exact binary value
    double spill;
    std::int32_t result;
    std::int64_t wide;
    std::uint16_t previous_control, truncation_control;
    // Preserve the native call-site x87 multiplication and runtime SSE2 spill.
    // For the finite signed16 input domain, hardware truncation supplies the
    // same integer as the CRT fallback. No CRT implementation is copied.
    __asm {
        fild adjusted
        fmul scale
        cmp sse2, 0
        je extended_path
        fstp spill
        cvttsd2si eax, spill
        mov result, eax
        jmp converted
    extended_path:
        fnstcw previous_control
        mov ax, previous_control
        or ax, 0xc00
        mov truncation_control, ax
        fldcw truncation_control
        fistp wide
        fldcw previous_control
        mov eax, dword ptr wide
        mov result, eax
    converted:
    }
    return static_cast<std::int16_t>(result);
}

std::uint16_t motor_word(float value) {
    const double scale = 65535.0;
    std::uint16_t previous_control, truncation_control;
    std::int32_t integer;
    // Recovered A9A9C0 instructions: multiply BEFORE changing rounding; truncate
    // to signedDWORD, retain LOW WORD, restore CW. No clamp or abs is present.
    __asm {
        fld value
        fmul scale
        fnstcw previous_control
        mov ax, previous_control
        or ax, 0xc00
        mov truncation_control, ax
        fldcw truncation_control
        fistp integer
        fldcw previous_control
    }
    return static_cast<std::uint16_t>(integer);
}
}

struct XInputLibrary::Impl {
    HMODULE module{};
    XInputGetStateFunction get{};
    using SetState = DWORD (WINAPI*)(DWORD, XINPUT_VIBRATION*);
    SetState set{};
    explicit Impl(const std::wstring& path) {
        if (!std::filesystem::path(path).is_absolute())
            throw std::invalid_argument("XInput DLL path must be absolute");
        module = LoadLibraryExW(path.c_str(), nullptr, LOAD_WITH_ALTERED_SEARCH_PATH);
        if (!module) throw std::runtime_error("Cannot load XInput DLL: Win32 error " +
            std::to_string(GetLastError()));
        auto get_address = GetProcAddress(module, MAKEINTRESOURCEA(2));
        auto set_address = GetProcAddress(module, MAKEINTRESOURCEA(3));
        if (!get_address || !set_address) {
            FreeLibrary(module);
            throw std::runtime_error("XInput DLL lacks imported ordinal2 or ordinal3");
        }
        static_assert(sizeof(get) == sizeof(get_address) && sizeof(set) == sizeof(set_address));
        std::memcpy(&get, &get_address, sizeof(get));
        std::memcpy(&set, &set_address, sizeof(set));
    }
    ~Impl() { FreeLibrary(module); }
};
XInputLibrary::XInputLibrary(const std::wstring& path) : impl_(std::make_unique<Impl>(path)) {}
XInputLibrary::~XInputLibrary() = default;
DWORD XInputLibrary::get_state(DWORD user, XINPUT_STATE& state) { return impl_->get(user, &state); }
DWORD XInputLibrary::set_state(DWORD user, XINPUT_VIBRATION& state) { return impl_->set(user, &state); }
XInputGetStateFunction XInputLibrary::get_state_function() const noexcept { return impl_->get; }

XInputDevice::XInputDevice(std::uint32_t user, XInputApi& api, XInputDeviceGlobals& globals) noexcept
    : GamepadInputDevice(&globals.rumble_enabled), user_index(user), api_(api), globals_(globals) {}
void XInputDevice::write_gamepad_force(std::uint32_t channel, float value) {
    set_motor_value_00a9a9c0(channel, value);
}
int XInputDevice::device_class() const { return 2; }
std::int32_t XInputDevice::identifier_00a9a5f0() const noexcept { return 0; }
float XInputDevice::value_24(std::uint32_t code) const {
    if (code == 12) return trigger_value(state.Gamepad.bLeftTrigger);
    if (code == 13) return trigger_value(state.Gamepad.bRightTrigger);
    if (code < 16) {
        // Native zero-extends this mask then compares against -1: the skip
        // branch is unreachable even for FFFF. Keep ordinary bit testing.
        return (state.Gamepad.wButtons & globals_.button_masks[code]) ? 1.0f : 0.0f;
    }
    switch (code) {
    case 60: return axis_value(state.Gamepad.sThumbLX, false);
    case 61: return axis_value(state.Gamepad.sThumbLY, true);
    case 62: return axis_value(state.Gamepad.sThumbRX, false);
    case 63: return axis_value(state.Gamepad.sThumbRY, true);
    default: return 0.0f;
    }
}
std::uint8_t XInputDevice::query_20(std::uint32_t code) const {
    const float magnitude = std::fabs(value_24(code));
    const double threshold = code < 16 ? static_cast<double>(0.1f) : 0.5;
    return magnitude >= threshold ? 1 : 0; // native unordered comparison is false
}
bool XInputDevice::activity_00a93f30() const {
    for (std::uint32_t code = 0; code != 90; ++code)
        if (query_20(code)) return true;
    return false;
}
bool XInputDevice::poll_00a9a7f0(float) {
    const DWORD result = api_.get_state(user_index, state);
    connected = result != ERROR_DEVICE_NOT_CONNECTED;
    if (result != ERROR_SUCCESS) return false;
    if (!globals_.rumble_enabled) vibration = {};
    (void)api_.set_state(user_index, vibration);
    state.Gamepad.sThumbLX = deadzone_axis(state.Gamepad.sThumbLX, globals_.crt_sse2_conversion);
    state.Gamepad.sThumbLY = deadzone_axis(state.Gamepad.sThumbLY, globals_.crt_sse2_conversion);
    state.Gamepad.sThumbRX = deadzone_axis(state.Gamepad.sThumbRX, globals_.crt_sse2_conversion);
    state.Gamepad.sThumbRY = deadzone_axis(state.Gamepad.sThumbRY, globals_.crt_sse2_conversion);
    return true;
}
DWORD XInputDevice::stop_vibration_00a9a790() {
    vibration = {};
    return api_.set_state(user_index, vibration);
}
void XInputDevice::set_motor_value_00a9a9c0(std::uint32_t motor, float value) {
    if (motor == 0) vibration.wRightMotorSpeed = motor_word(value);
    else if (motor == 1) vibration.wLeftMotorSpeed = motor_word(value);
}
NativeString& XInputDevice::control_name_00a9aa40(NativeString& result, std::uint32_t code,
    NativeStringStorage& strings) const {
    const char* text = "Unknown";
    if (code < 16) text = globals_.button_names[code];
    else if (code - 60 < 4) text = globals_.axis_names[code - 60];
    return result.assign_0041e870(strings, text);
}
XInputDevice* delete_xinput_device_00a9a7c0(XInputDevice& device, std::uint32_t flags) {
    auto* const original = &device;
    if (flags & 1) delete &device;
    else device.~XInputDevice();
    return original;
}

XInputFocusDeviceHost::XInputFocusDeviceHost(InputFocusDeviceHost* other) noexcept : other_(other) {}
InputFocusDeviceHost& XInputFocusDeviceHost::other() {
    if (!other_) throw std::logic_error("XInput focus adapter requires the actual other device host");
    return *other_;
}
std::int32_t XInputFocusDeviceHost::query_identifier_vslot34(InputDevice& device) {
    if (auto* pad = dynamic_cast<XInputDevice*>(&device)) return pad->identifier_00a9a5f0();
    return other().query_identifier_vslot34(device);
}
void XInputFocusDeviceHost::delete_device_vslot04(InputDevice& device, std::uint32_t flags) {
    if (auto* pad = dynamic_cast<XInputDevice*>(&device)) delete_xinput_device_00a9a7c0(*pad, flags);
    else other().delete_device_vslot04(device, flags);
}
void XInputFocusDeviceHost::poll_device_vslot10(InputDevice& device, float seconds) {
    if (auto* pad = dynamic_cast<XInputDevice*>(&device)) (void)pad->poll_00a9a7f0(seconds);
    else other().poll_device_vslot10(device, seconds);
}
bool XInputFocusDeviceHost::activity_vslot28(InputDevice& device) {
    if (auto* pad = dynamic_cast<XInputDevice*>(&device)) return pad->activity_00a93f30();
    return other().activity_vslot28(device);
}
} // namespace bsp
