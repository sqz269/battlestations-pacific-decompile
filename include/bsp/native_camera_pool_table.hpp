#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual {pointer, count, capacity} bytes at Camera pool+28h.
// Byte access also admits the legacy PointerArray interface without aliasing a
// different C++ struct or copying the header away from a reentrant new handler.
struct NativeCameraPoolTableView {
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

// bsp.py show 00B6FFF0/00B70040/00B71260/00B712F0: native ECX is the
// three-word table, RET 4 except grow (RET 0). These are C++ view interfaces,
// not register-ABI thunks. Names are hypotheses. Capacity is published BEFORE
// allocation; pointer publication follows copying and freeing the old table.
void allocate_native_camera_pool_table_00b6fff0(NativeCameraPoolTableView, std::uint32_t);
void grow_native_camera_pool_table_00b70040(NativeCameraPoolTableView);
void append_native_camera_pool_table_00b71260(NativeCameraPoolTableView, const void* pointer_cell);
void reserve_native_camera_pool_table_00b712f0(NativeCameraPoolTableView, std::uint32_t);

} // namespace bsp
