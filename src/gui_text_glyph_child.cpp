#include "bsp/gui_text_glyph_child.hpp"
#include <cstddef>
#include <cstring>
#include <intrin.h>

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

namespace {
template<class T> T raw_code_unit_read(const void* memory, std::size_t offset = 0) {
    _ReadWriteBarrier();
    const auto value = load<T>(memory, offset);
    _ReadWriteBarrier();
    return value;
}
template<class T> void raw_code_unit_write(void* memory, std::size_t offset, T value) {
    _ReadWriteBarrier();
    store(memory, offset, value);
    _ReadWriteBarrier();
}
} // namespace

void* construct_gui_text_code_unit_00ab81c0(void* header,
    std::uint32_t code_unit_word, NativeStringRawPoolContext& context) {
    static_assert(sizeof(void*) == 4 && sizeof(std::uint32_t) == 4
        && sizeof(std::uint16_t) == 2);
    raw_code_unit_write<std::uint32_t>(header, 0, 0); //AB81C4
    raw_code_unit_write<void*>(header, 4, nullptr); //AB81CA, no old-buffer return
    auto* const allocation_pool = native_string_pool_get_or_create_00419cc0(
        context.actual_published_01090aa8,
        context.actual_manager_publication_01090aa0); //AB81D1
    void* const allocation = allocate_native_string_pool_00bd1120(
        allocation_pool, 4u); //AB81DC; unused native stack DWORD is1
    const auto current_length = raw_code_unit_read<std::uint32_t>(header); //AB81E3
    if (current_length >= 1u) { //AB81E5/E8, unsigned
        const auto* const source = raw_code_unit_read<void*>(header, 4); //AB81FC
        const auto unit = raw_code_unit_read<std::uint16_t>(source); //AB81FF
        raw_code_unit_write(allocation, 0, unit); //AB8202; forward single WORD
    } else {
        const auto* const source = raw_code_unit_read<void*>(header, 4); //AB81EA
        (void)source; //AB81ED..81F7: unsigned length0; omit memcpy(...,0)
    }
    void* const previous = raw_code_unit_read<void*>(header, 4); //AB8205
    if (previous != nullptr) {
        const auto bytes = raw_code_unit_read<std::uint32_t>(header) * 2u + 2u;
        auto* const return_pool = native_string_pool_get_or_create_00419cc0(
            context.actual_published_01090aa8,
            context.actual_manager_publication_01090aa0); //AB8216, operands captured
        return_native_string_pool_00bd1510(return_pool, previous, bytes,
            context.actual_small_returns_disabled_01090aa4); //AB821D
    }
    const auto unit = raw_code_unit_read<std::uint16_t>(&code_unit_word); //AB8222
    raw_code_unit_write(header, 4, allocation); //AB8227, captured across return
    raw_code_unit_write<std::uint32_t>(header, 0, 1); //AB822A
    raw_code_unit_write<std::uint16_t>(allocation, 2, 0); //AB8230
    void* const published = raw_code_unit_read<void*>(header, 4); //AB8236
    raw_code_unit_write(published, 0, unit); //AB823C; CURRENT data, not allocation
    return header; //AB823A/8240: identity, RET4 in Native
}
} // namespace bsp
