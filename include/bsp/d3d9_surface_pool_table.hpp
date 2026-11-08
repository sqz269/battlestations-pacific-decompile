#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual three-word header at D3D9 surface pool+28h. Byte access
// keeps allocator/new-handler mutations visible without a copied header or
// aliasing the pool's pointer/count/capacity fields as another C++ struct.
struct D3D9SurfacePoolTableView {
    std::byte* header;

    std::byte* data() const noexcept {
        std::byte* result;
        std::memcpy(&result, header, sizeof(result));
        return result;
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
        std::uint32_t result;
        std::memcpy(&result, header + offset, sizeof(result));
        return result;
    }
    void word(std::size_t offset, std::uint32_t value) const noexcept {
        std::memcpy(header + offset, &value, sizeof(value));
    }
};

// Original ECX=header; RET 4 except grow (RET). C++ view interfaces, not
// register-ABI replacements. Names are hypotheses. Evidence: bsp.py show
// 00B3DA70/00B3DAC0/00B3E800/00B3E880. The original surface constructor
// 00B3EC60 and allocation path 00B3ED40 inline these identical operations.
void set_d3d9_surface_pool_table_capacity_00b3da70(D3D9SurfacePoolTableView, std::uint32_t);
void grow_d3d9_surface_pool_table_00b3dac0(D3D9SurfacePoolTableView);
void append_d3d9_surface_pool_table_00b3e800(D3D9SurfacePoolTableView, const void* pointer_cell);
void reserve_d3d9_surface_pool_table_00b3e880(D3D9SurfacePoolTableView, std::uint32_t);

} // namespace bsp
