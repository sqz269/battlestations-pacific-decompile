#pragma once

#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#include <cstddef>
#include <vector>

namespace bsp {

// Source SDK/reference boundary shared by raw keyboard, mouse and joystick
// constructors. Output parameters are the ACTUAL native member/stack slots.
// Every nonnull SDK return must name one returned COM reference, including
// failure HRESULTs with a valid nonnull output. No shadow output is substituted.
// Arbitrary COM implementations violating that output contract are unsupported.
class NativeInputDeviceSdk final {
public:
    NativeInputDeviceSdk() = default;
    NativeInputDeviceSdk(const NativeInputDeviceSdk&) = delete;
    NativeInputDeviceSdk& operator=(const NativeInputDeviceSdk&) = delete;
    // The destructor does not release: references outlive native owner/member
    // cleanup and every callback that borrows a device/effect.
    HRESULT create_device(IDirectInput8A&, const GUID&,
        IDirectInputDevice8A** actual_output, IUnknown* outer = nullptr);
    HRESULT create_effect(IDirectInputDevice8A&, const GUID&, const DIEFFECT&,
        IDirectInputEffect** actual_output, IUnknown* outer = nullptr);
    std::size_t pending_references() const noexcept;
    void release_tracked_references();
private:
    // Reserve a stable INDEX before calling the SDK. Nested acquisitions may
    // reallocate this vector; recording after SDK return never allocates.
    std::size_t prepare_reference();
    std::vector<IUnknown*> references_;
    std::size_t active_acquisitions_{};
};

} // namespace bsp
