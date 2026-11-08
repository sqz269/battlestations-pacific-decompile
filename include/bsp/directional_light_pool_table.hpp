#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual {data,count,capacity} words at directional pool+28h
// (0109017C), or a legacy PointerArray header. This view owns no storage.
struct DirectionalLightPoolTableView {
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

// Evidence: bsp.py show 00B7AE80/00B7AED0/00B7B510/00B7B5A0 --asm.
// Original ECX=header, stack capacity/pointer-cell and RET4; grow has RET.
// Capacity is published before allocation; data is published after freeing
// the previous table. A throw retains changed capacity and old data/count.
// These are C++ view interfaces, not native ABI thunks. Names are hypotheses.
void allocate_directional_light_pool_table_00b7ae80(DirectionalLightPoolTableView,
    std::uint32_t capacity);
void grow_directional_light_pool_table_00b7aed0(DirectionalLightPoolTableView);
void append_directional_light_pool_table_00b7b510(DirectionalLightPoolTableView,
    const void* pointer_cell);
void reserve_directional_light_pool_table_00b7b5a0(DirectionalLightPoolTableView,
    std::uint32_t capacity);

} // namespace bsp
