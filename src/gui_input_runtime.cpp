#include "bsp/gui_input_runtime.hpp"
#include "bsp/gui_listbox_runtime.hpp"
#include "bsp/input_device_state.hpp"
#include "bsp/input_focus_reset.hpp"
#include "bsp/native_input_device_runtime.hpp"
#include "bsp/platform_cursor.hpp"
#include <cstddef>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
template<class T> T read(const void* p,std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value,static_cast<const std::byte*>(p)+offset,sizeof value);
    return value;
}
const MouseInputDevice& mouse(const InputDevice* p) {
    auto* result=dynamic_cast<const MouseInputDevice*>(p);
    if(!result) throw std::logic_error("GUI input requires the actual typed mouse profile");
    return *result;
}
std::int32_t signed_word(std::uint32_t value) noexcept {
    std::int32_t result;std::memcpy(&result,&value,4);return result;
}
}
void* gui_raw_input_device_004ba6d0(void* backend,std::uint32_t type,std::uint32_t index) {
    if(!backend || type>=3) throw std::logic_error("GUI raw input requires its live backend/class");
    const auto offset=0x6cu+type*0x24u;
    const auto begin=read<std::uintptr_t>(backend,offset+4);
    if(!begin) return nullptr;
    const auto end=read<std::uintptr_t>(backend,offset+8);
    if(end<begin || ((end-begin)&3u))
        throw std::logic_error("GUI input class vector is outside the live native domain");
    const auto count=(end-begin)/4u;
    if(index>=count) return nullptr;
    // The native repeats its bounds check then reloads begin before indexing.
    // No user callback occurs between those loads on the valid vector domain.
    const auto* fresh=read<const std::byte*>(backend,offset+4);
    return read<void*>(fresh,index*4u);
}
GuiInputSource::GuiInputSource(InputBindingDeviceGroups* const& typed) noexcept : typed_(&typed) {}
GuiInputSource::GuiInputSource(InputBindingDeviceGroups* const& typed,
    InputFocusDeviceHost& activity) noexcept : typed_(&typed), typed_activity_(&activity) {}
GuiInputSource::GuiInputSource(GuiNativeInputSource raw) noexcept
    :raw_(&raw.backend_00f8bbf4),native_(&raw.devices) {}
GuiInputDeviceRef GuiInputSource::device(std::uint32_t type,std::uint32_t index) const {
    GuiInputDeviceRef result;
    if(raw_) {
        result.raw_=gui_raw_input_device_004ba6d0(*raw_,type,index);
        result.native_=native_;
    } else {
        auto* groups=*typed_;
        if(!groups || type>=groups->size())
            throw std::logic_error("GUI input requires its actual typed class vectors");
        result.typed_=get_input_class_device_004ba6d0(*groups,static_cast<std::int32_t>(type),signed_word(index));
        result.typed_activity_=typed_activity_;
    }
    return result;
}
void GuiInputDeviceRef::require_mouse() const {
    if(raw_) {
        if(read<std::uint32_t>(raw_,0)!=0x00d5b8b0u)
            throw std::logic_error("GUI input mouse signature requires actual D5B8B0 allocation");
    } else (void)mouse(typed_);
}
std::uint8_t GuiInputDeviceRef::mouse_current_down(std::uint32_t button) const {
    require_mouse();
    if(button>=8) throw std::out_of_range("GUI mouse button index");
    return raw_?read<std::uint8_t>(raw_,0xcu+button):mouse(typed_).current_down[button];
}
std::uint8_t GuiInputDeviceRef::mouse_previous_down(std::uint32_t button) const {
    require_mouse();
    if(button>=8) throw std::out_of_range("GUI mouse button index");
    return raw_?read<std::uint8_t>(raw_,0x10cu+button):mouse(typed_).previous_down[button];
}
const float* GuiInputDeviceRef::mouse_double_click_storage() const {
    require_mouse();
    return raw_?reinterpret_cast<const float*>(static_cast<const std::byte*>(raw_)+0x238)
        :&mouse(typed_).double_click_seconds;
}
std::int32_t GuiInputDeviceRef::mouse_accumulated_x() const {
    require_mouse();
    return signed_word(raw_?native_->mouse_accumulated_x_vslot3c(raw_):mouse(typed_).accumulated[0]);
}
std::int32_t GuiInputDeviceRef::mouse_accumulated_y() const {
    require_mouse();
    return signed_word(raw_?native_->mouse_accumulated_y_vslot40(raw_):mouse(typed_).accumulated[1]);
}
std::uint8_t GuiInputDeviceRef::buttons_active(GuiListboxFrameCalls& provider) const {
    if(raw_) return native_->buttons_active_vslot2c(raw_);
    if(!typed_) throw std::logic_error("GUI input has no device at current2C");
    return provider.device_current2c(*typed_);
}
std::uint8_t GuiInputDeviceRef::activity_current28() const {
    if(raw_) return native_->activity_vslot28(raw_,read<std::uint32_t>(raw_,0));
    if(!typed_ || !typed_activity_)
        throw std::logic_error("GUI current28 requires its actual typed activity provider");
    return static_cast<std::uint8_t>(typed_activity_->activity_vslot28(*typed_));
}
std::uint8_t GuiInputDeviceRef::query_current20(std::uint32_t code) const {
    if(raw_) return native_->query_vslot20(raw_,read<std::uint32_t>(raw_,0),code);
    const auto* state=dynamic_cast<const InputStateDevice*>(typed_);
    if(!state) throw std::logic_error("GUI current20 requires its actual typed state device");
    return state->query_20(code);
}
float GuiInputDeviceRef::value_current24(std::uint32_t code) const {
    if(raw_) return native_->value_vslot24(raw_,read<std::uint32_t>(raw_,0),code);
    const auto* state=dynamic_cast<const InputStateDevice*>(typed_);
    if(!state) throw std::logic_error("GUI current24 requires its actual typed state device");
    return state->value_24(code);
}
} // namespace bsp
