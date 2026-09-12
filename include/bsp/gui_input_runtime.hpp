#pragma once
#include "bsp/input_action_classifier.hpp"
#include <cstdint>

namespace bsp {
class NativeInputDeviceRuntime;
class GuiListboxFrameCalls;

// Borrows the application's actual raw F8BBF4 publication and its finite
// device dispatcher. No backend, class vector, device or input sample is owned.
struct GuiNativeInputSource {
    void* volatile& backend_00f8bbf4;
    NativeInputDeviceRuntime& devices;
};
class GuiInputSource;
class GuiInputDeviceRef {
public:
    explicit operator bool() const noexcept { return typed_ || raw_; }
    void require_mouse() const;
    std::uint8_t mouse_current_down(std::uint32_t button) const;
    std::uint8_t mouse_previous_down(std::uint32_t button) const;
    // The frame's x87 comparison reads this same binary32 field directly,
    // without an additional return-value spill or a copied mouse sample.
    const float* mouse_double_click_storage() const;
    std::int32_t mouse_accumulated_x() const;
    std::int32_t mouse_accumulated_y() const;
    std::uint8_t buttons_active(GuiListboxFrameCalls& typed_provider) const;
private:
    friend class GuiInputSource;
    InputDevice* typed_{};
    void* raw_{};
    NativeInputDeviceRuntime* native_{};
};

// The legacy typed interface and the native owner domain remain distinct.
// Each binding selects one actual publication; no raw-to-typed cast, proxy
// device, synchronization copy or alternate backend is introduced.
class GuiInputSource {
public:
    GuiInputSource(InputBindingDeviceGroups* const& typed_publication) noexcept;
    GuiInputSource(GuiNativeInputSource) noexcept;
    GuiInputDeviceRef device(std::uint32_t device_class, std::uint32_t index = 0) const;
    bool same_publication(const GuiInputSource& other) const noexcept {
        return typed_ == other.typed_ && raw_ == other.raw_ && native_ == other.native_;
    }
private:
    InputBindingDeviceGroups* const* typed_{};
    void* volatile* raw_{};
    NativeInputDeviceRuntime* native_{};
};

// 4BA6D0 ECX backend, class/index stack, RET8. Actual24h class records at6C,
// vector begin/end at+4/+8. Valid class0..2 and live pointer arrays are required.
void* gui_raw_input_device_004ba6d0(void* actual_backend,
    std::uint32_t device_class, std::uint32_t index);
} // namespace bsp
