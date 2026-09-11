#pragma once

#include "bsp/input_binding_poll.hpp"
#include "bsp/input_settings.hpp"

#include <array>
#include <cstdint>

struct IDirectInputDevice8A;

namespace bsp {

// Typed projections, not native layouts or replacement COM objects. The
// caller owns an already-created/configured ANSI DirectInput device and keeps
// it alive. Native offsets, ABI, and failure behavior: INPUT_DEVICE_STATE.md.
class InputStateDevice : public InputDevice {
public:
    void on_slot_reset() override; // 00a93e80: bare RET, does not clear state
    virtual std::uint8_t query_1c(std::uint32_t code) const;
    virtual std::uint8_t query_20(std::uint32_t code) const = 0;
    virtual float value_24(std::uint32_t code) const = 0;

    std::array<std::uint8_t, 256> current_down{};  // native +00Ch
    std::array<std::uint8_t, 256> previous_down{}; // native +10Ch

protected:
    void update_history_00a99e90();
};

class KeyboardInputDevice final : public InputStateDevice {
public:
    explicit KeyboardInputDevice(::IDirectInputDevice8A* borrowed_device);
    int device_class() const override; // 00a96350, class 0
    // Native 00a95e70 has no bounds check. Adapter requires code < 256 and
    // reports out-of-range instead of reading past the native byte array.
    std::uint8_t query_20(std::uint32_t code) const override;
    float value_24(std::uint32_t code) const override; // 00a95e90, float 0 or 1
    // 00a9a4a0, device vtable +10h. Native one float stack argument is ignored.
    // suppress_keyboard is the actual platform byte at +170h.
    bool poll_00a9a4a0(const bool& suppress_keyboard);

    ::IDirectInputDevice8A* direct_input; // native +30Ch, borrowed
    std::array<std::uint8_t, 256> state{}; // +20Ch, normalized after GetDeviceState
};

// Layout matches DIMOUSESTATE2, passed directly to GetDeviceState(size=20).
struct MouseInputSample {
    std::int32_t x;
    std::int32_t y;
    std::int32_t z;
    std::array<std::uint8_t, 8> buttons;
};

// These are live external globals, read on every value query. Callers supply
// them instead of manufacturing native globals in this reconstruction.
struct MouseInputGlobals {
    float axis_scale; // 00e12fb0 (image value 1.0), divided by double 100
    bool invert_y;    // 00f8bc04
};

struct MouseInputPlatform {
    std::uintptr_t window; // actual HWND from platform getter 00bec230
    bool& settings_changed; // platform +2Ch: consumed and cleared by mouse poll
};

class MouseInputDevice final : public InputStateDevice {
public:
    // Native constructor leaves raw +214h..+227h uninitialized. Supply an
    // explicit initial sample so a failed first GetDeviceState has defined
    // typed storage; no claim is made about those native allocation bytes.
    MouseInputDevice(::IDirectInputDevice8A* borrowed_device,
        MouseInputGlobals& globals, MouseInputSample initial_sample);
    int device_class() const override; // 00a9a0f0, class 1
    std::uint8_t query_20(std::uint32_t code) const override; // 00a99f70
    float value_24(std::uint32_t code) const override; // 00a99fe0
    bool poll_00a9a180(MouseInputPlatform platform);

    ::IDirectInputDevice8A* direct_input; // native +20Ch, borrowed
    bool valid{false}; // +210h, set only by successful GetDeviceState
    MouseInputSample sample; // +214h, raw signed axes and high-bit buttons
    std::array<std::uint32_t, 3> accumulated{}; // +228h, wrapping ADDs
    bool cooperative_configured{false}; // +234h
    bool swapped_buttons{false}; // +235h, GetSystemMetrics(SM_SWAPBUTTON)
    float double_click_seconds{}; // +238h; not used by these three query slots
    MouseInputGlobals& globals;

private:
    void refresh_system_settings();
};

// Concrete bridge for the already reconstructed complete binding poll. Every
// cached device must be an InputStateDevice: unsupported joystick/XInput/base
// devices throw instead of receiving fabricated query results. CRT sqrt is
// the caller's genuine library/FPU obligation; this module does not port it.
class KeyboardMouseBindingPollHost final : public InputBindingPollHost {
public:
    using CrtSqrt = float (*)(float);
    explicit KeyboardMouseBindingPollHost(CrtSqrt crt_sqrt);
    std::uint8_t device_query_1c(InputDevice&, std::uint32_t code) override;
    std::uint8_t device_query_20(InputDevice&, std::uint32_t code) override;
    float device_value_24(InputDevice&, std::uint32_t code) override;
    float crt_sqrt_00bf7030(float value) override;

private:
    CrtSqrt sqrt_;
};

} // namespace bsp
