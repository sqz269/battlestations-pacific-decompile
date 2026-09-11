#include "bsp/native_string_append.hpp"
#include <cstring>

namespace bsp {
void append_native_string_00425e10(NativeString& destination, const NativeString& source,
    NativeStringStorage& storage) {
    const auto size = source.length();
    if (size == 0) return;
    const auto old_length = destination.length();
    destination.resize_0041dd40(storage, old_length + size, true);
    std::memcpy(destination.data() + old_length, source.data(), size);
}
} // namespace bsp
