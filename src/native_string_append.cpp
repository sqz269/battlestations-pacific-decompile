#include "bsp/native_string_append.hpp"
#include <cstddef>
#include <cstring>

namespace bsp {
void append_native_string_00425e10(NativeString& destination, const NativeString& source,
    NativeStringStorage& storage) {
    append_native_string_00425e10(&destination, &source, storage);
}
void append_native_string_00425e10(void* destination, const void* source,
    NativeStringStorage& storage) {
    std::uint32_t size;
    std::memcpy(&size, source, sizeof size);
    if (size == 0) return;
    std::uint32_t old_length;
    std::memcpy(&old_length, destination, sizeof old_length);
    resize_native_string_header_0041dd40(destination, storage, old_length + size, true);
    char* target;
    const char* input;
    std::memcpy(&target, static_cast<std::byte*>(destination) + 4, sizeof target);
    std::memcpy(&input, static_cast<const std::byte*>(source) + 4, sizeof input);
    std::memcpy(target + old_length, input, size);
}
} // namespace bsp
