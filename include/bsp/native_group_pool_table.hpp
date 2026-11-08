#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual {slabs, count, capacity} header at Group pool 010902f4+28h.
// Byte access also admits the legacy PointerArray without a second header or
// aliasing its C++ type. A CRT new handler therefore sees each published write.
struct NativeGroupPoolTableView {
    std::byte* header;

    std::byte* data() const noexcept {
        std::byte* value;
        std::memcpy(&value, header, sizeof(value));
        return value;
    }
    void data(std::byte* value) const noexcept {
        std::memcpy(header, &value, sizeof(value));
    }
    std::uint32_t count() const noexcept { return word(4); }
    void count(std::uint32_t value) const noexcept { word(4, value); }
    std::uint32_t capacity() const noexcept { return word(8); }
    void capacity(std::uint32_t value) const noexcept { word(8, value); }

private:
    std::uint32_t word(std::size_t offset) const noexcept {
        std::uint32_t value;
        std::memcpy(&value, header + offset, sizeof(value));
        return value;
    }
    void word(std::size_t offset, std::uint32_t value) const noexcept {
        std::memcpy(header + offset, &value, sizeof(value));
    }
};

// Evidence: bsp.py show (and --asm) 00B8E8E0/00B8E930/00B8EDC0/00B8EE50.
// Native ECX is the three-DWORD header; capacity/reserve and append take one
// stack argument and RET 4, grow RET 0. These are new C++ view interfaces, not
// binary ABI replacements. Names are descriptive hypotheses.
void allocate_native_group_pool_table_00b8e8e0(NativeGroupPoolTableView, std::uint32_t);
void grow_native_group_pool_table_00b8e930(NativeGroupPoolTableView);
void append_native_group_pool_table_00b8edc0(NativeGroupPoolTableView, const void* pointer_cell);
void reserve_native_group_pool_table_00b8ee50(NativeGroupPoolTableView, std::uint32_t);

} // namespace bsp
