#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual three words at retained Texture2D pool 0108DB38+28h.
// Byte access preserves the header's identity without an unrelated struct alias.
// This view neither owns nor copies the table, count or capacity.
struct D3D9Texture2DPoolTableView {
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

// Evidence: bsp.py show 00B3DB10/00B3DB60/00B3E8E0/00B3E960 --asm.
// Original ECX is {data,count,capacity}; capacity/pointer-cell on the stack,
// RET4 except grow (RET). Source view interfaces, not native ABI thunks.
// These COMDAT bodies match the inline table operations in EE80/EF60;
// their native image has no calls to these out-of-line entries.
void set_d3d9_texture2d_pool_table_capacity_00b3db10(
    D3D9Texture2DPoolTableView, std::uint32_t capacity);
void grow_d3d9_texture2d_pool_table_00b3db60(D3D9Texture2DPoolTableView);
void append_d3d9_texture2d_pool_table_00b3e8e0(
    D3D9Texture2DPoolTableView, const void* pointer_cell);
void reserve_d3d9_texture2d_pool_table_00b3e960(
    D3D9Texture2DPoolTableView, std::uint32_t capacity);

} // namespace bsp
