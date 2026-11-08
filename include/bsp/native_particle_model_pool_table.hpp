#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual three DWORDs at F8D2D0+28h. No copied header: a CRT new
// handler must observe the capacity publication and may change count/data.
struct NativeParticleModelPoolTableView {
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

// Evidence: bsp.py show / --asm 00AF5D30, 00AF5D80, 00AF61E0, 00AF6270.
// Native ECX = header; exact capacity, reserve and append take one stack
// argument and RET4, grow RET0. Capacity/byte arithmetic wraps as DWORDs.
// Capacity is stored before allocation, data after copy/free, count after
// append. No rollback on allocation failure. Append borrows a pointer cell
// and reads it AFTER growth. Names are hypotheses; these C++ view interfaces
// do not provide the original register or exception ABI.
void allocate_native_particle_model_pool_table_00af5d30(NativeParticleModelPoolTableView, std::uint32_t);
void grow_native_particle_model_pool_table_00af5d80(NativeParticleModelPoolTableView);
void append_native_particle_model_pool_table_00af61e0(NativeParticleModelPoolTableView, const void* pointer_cell);
void reserve_native_particle_model_pool_table_00af6270(NativeParticleModelPoolTableView, std::uint32_t);

} // namespace bsp
