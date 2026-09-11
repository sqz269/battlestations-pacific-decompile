#include "bsp/gui_text_glyph_child.hpp"
#include <cstddef>
#include <cstring>

namespace bsp {
namespace {
template<class T> T load(const void* memory, std::size_t offset = 0) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(memory) + offset, sizeof(value));
    return value;
}
template<class T> void store(void* memory, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<std::byte*>(memory) + offset, &value, sizeof(value));
}
}
void* construct_gui_text_code_unit_00ab81c0(void* header,
    std::uint32_t code_unit_word, ActualNativeStringPoolStorage& storage) {
    static_assert(sizeof(void*) == 4 && sizeof(char16_t) == 2);
    store<std::uint32_t>(header, 0, 0);
    store<char*>(header, 4, nullptr); // AB81C4/CA, before singleton/allocation.
    char* const allocation = storage.allocate(4); // AB81D1 then AB81DC, RET8.
    if (load<std::uint32_t>(header) >= 1u) {
        // AB81FC..8202. Reload data AFTER the current unsigned length test.
        const auto code_unit = load<std::uint16_t>(load<char*>(header, 4));
        store(allocation, 0, code_unit);
    }
    // The other branch is native memcpy(...,0), since unsigned length<1 is0.
    // Omit that zero-byte operation, as in existing actual-header string code.
    if (char* const previous = load<char*>(header, 4)) {
        const auto byte_count = load<std::uint32_t>(header) * 2u + 2u;
        storage.release(previous, byte_count); // AB8216 singleton -> AB821D RET0Ch.
    }
    // Preserve the returned allocation through release callbacks, then publish.
    store(header, 4, allocation); // AB8227 before the length store atAB822A.
    store<std::uint32_t>(header, 0, 1);
    store<std::uint16_t>(allocation, 2, 0); // AB8230 before writing code unit.
    char* const published = load<char*>(header, 4); // AB8236 live header reload.
    store(published, 0, static_cast<std::uint16_t>(code_unit_word));
    return header;
}
} // namespace bsp
