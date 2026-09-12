#include "bsp/native_input_device_tail_virtuals.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_append.hpp"
#include <cstddef>
#include <cstring>
#include <exception>

namespace bsp {
namespace {
const std::byte* at(const void* p, std::uint32_t offset) noexcept {
    return static_cast<const std::byte*>(p) + offset;
}
template<class T> T read(const void* p, std::uint32_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, at(p, offset), sizeof(value));
    return value;
}
void write(void* p, std::uint32_t offset, std::uint32_t value) noexcept {
    std::memcpy(static_cast<std::byte*>(p) + offset, &value, sizeof(value));
}
class ResultUnwind {
public:
    ResultUnwind(NativeString& value, NativeStringStorage& storage) noexcept
        : value_(value), storage_(storage), exceptions_(std::uncaught_exceptions()) {}
    ~ResultUnwind() {
        if (std::uncaught_exceptions() > exceptions_)
            destroy_native_string_header_0041dd20(&value_, storage_);
    }
private:
    NativeString& value_;
    NativeStringStorage& storage_;
    int exceptions_;
};
class Suffix {
public:
    Suffix(const char* text, NativeStringStorage& storage) : storage_(storage) {
        construct_native_string_cstring_0041e870(&value, text, storage_);
    }
    ~Suffix() {
        if (armed_) destroy_native_string_header_0041dd20(&value, storage_);
    }
    void release_captured(char* pointer, std::uint32_t length) noexcept {
        // A998C8..D6 uses EBX/EDI captured BEFORE output resize. EH instead
        // uses the actual temporary header through CB6AA1 ->41DD20.
        armed_ = false;
        if (pointer) storage_.release(pointer, length + 1u);
    }
    NativeString value;
private:
    NativeStringStorage& storage_;
    bool armed_{true};
};
} // namespace

std::uint32_t native_mouse_accumulated_x_00a99f10(const void* p) noexcept { return read<std::uint32_t>(p, 0x228); }
std::uint32_t native_mouse_accumulated_y_00a99f20(const void* p) noexcept { return read<std::uint32_t>(p, 0x22c); }
std::uint32_t native_mouse_accumulated_z_00a99f30(const void* p) noexcept { return read<std::uint32_t>(p, 0x230); }
void set_native_mouse_accumulated_x_00a99f40(void* p, std::uint32_t value) noexcept { write(p, 0x228, value); }
void set_native_mouse_accumulated_y_00a99f50(void* p, std::uint32_t value) noexcept { write(p, 0x22c, value); }
void set_native_mouse_accumulated_z_00a99f60(void* p, std::uint32_t value) noexcept { write(p, 0x230, value); }

NativeString& native_joystick_control_name_00a99710(void* device,
    NativeString& output, std::uint32_t code, NativeStringStorage& storage) {
    construct_native_string_cstring_0041e870(&output, "", storage); // CE3A0C
    ResultUnwind result_unwind(output, storage); // constructed-result flag
    const void* const binding = at(device, 0x29cu + code * 12u);
    if (read<std::uint32_t>(binding) == 0) return output;
    const auto index = read<std::uint32_t>(binding, 4);
    const void* const name = at(read<const void*>(device, 0x290), index * 28u);
    copy_native_string_header_00be0a30_fragment(&output, storage, name);
    // A997AB reloads the original binding kind after all name storage calls.
    switch (read<std::uint32_t>(binding)) {
    case 3: case 5: {
        Suffix suffix("/Left", storage); // D5B830
        const auto length = suffix.value.length();
        auto* const pointer = suffix.value.data();
        if (length != 0) {
            const auto old_length = output.length();
            output.resize_0041dd40(storage, old_length + length, true);
            std::memcpy(output.data() + old_length, pointer, length);
        }
        suffix.release_captured(pointer, length);
        break;
    }
    case 4: case 6: {
        Suffix suffix("/Right", storage); // D5B838
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    case 7: {
        Suffix suffix("/Up", storage); // D5B848
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    case 8: {
        Suffix suffix("/Down", storage); // D5B840
        append_native_string_00425e10(output, suffix.value, storage);
        break;
    }
    default: break;
    }
    return output;
}
} // namespace bsp
