#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp {

// Borrow the actual three words at Mesh pool+28h (01090020). Byte access
// also admits the legacy PointerArray header without a copied header or an
// alias between unrelated C++ struct types. This view owns no storage.
struct NativeMeshPoolTableView {
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

// bsp.py show 00B72900/00B72950/00B72E80/00B72F10 --asm. Original ECX
// receives {pointer,count,capacity}; stack n/pointer-cell and RET4 except
// grow (RET). These are new C++ view interfaces, not native ABI thunks.
// Capacity changes before allocation; a throwing allocator leaves that
// changed capacity and the old pointer/count. Names remain hypotheses.
void allocate_native_mesh_pool_table_00b72900(NativeMeshPoolTableView, std::uint32_t capacity);
void grow_native_mesh_pool_table_00b72950(NativeMeshPoolTableView);
void append_native_mesh_pool_table_00b72e80(NativeMeshPoolTableView, const void* pointer_cell);
void reserve_native_mesh_pool_table_00b72f10(NativeMeshPoolTableView, std::uint32_t capacity);

} // namespace bsp
