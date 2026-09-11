#include "bsp/joystick_labels.hpp"
#include "bsp/native_string_append.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
class SuffixString {
public:
    SuffixString(const char* text, NativeStringStorage& storage) : storage_(storage) {
        value.assign_0041e870(storage_, text);
    }
    ~SuffixString() { value.release_to(storage_); }
    NativeString value;
private:
    NativeStringStorage& storage_;
};
}

NativeString& joystick_control_name_00a99710(const JoystickInputDevice& device,
    NativeString& output, std::uint32_t code, NativeStringStorage& storage) {
    output.assign_0041e870(storage, ""); // CE3A0C is NUL, not a display fallback
    if (code >= device.bindings.size()) throw std::out_of_range("Native joystick binding index");
    const auto& binding = device.bindings[code];
    if (binding.kind == 0) return output;
    const auto object_index = binding.object;
    if (object_index < 0 || static_cast<std::size_t>(object_index) >= device.objects.size())
        throw std::out_of_range("Native joystick object index");
    output.copy_from_00be0a30_fragment(storage, device.objects[object_index].name);
    // Native reloads kind after the allocation/copy, rather than snapshotting
    // it before a potentially reentrant storage operation.
    switch (binding.kind) {
    case 3: case 5: {
        SuffixString suffix("/Left", storage);
        const auto size = suffix.value.length();
        const auto* data = suffix.value.data();
        if (size != 0) {
            const auto old_length = output.length();
            output.resize_0041dd40(storage, old_length + size, true);
            std::memcpy(output.data() + old_length, data, size);
        }
        break;
    }
    case 4: case 6: {
        SuffixString suffix("/Right", storage);
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    case 7: {
        SuffixString suffix("/Up", storage);
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    case 8: {
        SuffixString suffix("/Down", storage);
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    default: break;
    }
    return output;
}
} // namespace bsp
