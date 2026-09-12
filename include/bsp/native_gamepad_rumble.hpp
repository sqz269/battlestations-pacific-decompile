#pragma once

#include "bsp/native_gamepad_xinput.hpp"

namespace bsp {

// Select the native slot38 body from this captured profile, even if a saved
// amplitude write has subsequently changed raw storage. The application binds
// its existing finite XInput/joystick/base dispatch; no fake force success.
struct NativeGamepadRumbleOutput {
    virtual ~NativeGamepadRumbleOutput() = default;
    virtual void set_force_vslot38(void* actual_device, std::uint32_t captured_profile,
        std::uint32_t channel, float value) = 0;
};

struct NativeGamepadRumbleContext {
    void* volatile& backend_00f8bbf4;
    // SAME canonical bool borrowed by NativeGamepadContext/XInputDeviceGlobals.
    // All seven verified native setter call sites produce CL=0 or1. This source
    // bool interface models that caller domain, not arbitrary noncanonical bytes.
    volatile bool& enabled_00e12f2c;
    NativeGamepadDispatch& requests;
    NativeGamepadRumbleOutput& output;
};

// A949A0: native ECX actual220h-or-larger gamepad, stack channel DWORD, RET4.
// Positive maximum of current raw tree requests for this channel. Value is read
// before reloading the request for channel; no tick, expiration, erase or retain.
// Compares via x87 and stores changed amplitude before captured-profile output.
void refresh_native_gamepad_force_channel_00a949a0(void* actual_device,
    std::uint32_t channel, NativeGamepadRumbleContext&);

// A94C50: native CL normalized enable byte, no stack arguments, RET0. Publishes
// the flag BEFORE unguarded current-backend access. Captures initial active
// class2 count, reloads backend only after each nonnull device, rereads the flag
// for each channel. Disabling outputs zero BEFORE clearing that saved amplitude.
void set_native_gamepad_rumble_enabled_00a94c50(bool enabled,
    NativeGamepadRumbleContext&);

// Actual backend+B4 vector, raw device+20C tree,18h nodes and request allocations
// remain the sole state/owners. Reuses existing recognized raw iterator869A20
// as the equivalent A94230 storage contract and real returning CRT validation.
// Channel arithmetic is unchecked; caller must provide addressable native
// storage. No new map, private manager, SDK ownership, original ABI/FH3/SEH,
// malformed ranges, concurrent mutation, arbitrary stack alias or game claim.
} // namespace bsp
