#pragma once

#include <cstddef>
#include <cstdint>

namespace bsp {

// Borrow the live {data, count, capacity} DWORDs at Node pool 0108FF58+28h.
// Also admits the legacy PointerArray header without a copied state snapshot.
struct NativeNodePoolTableView {
    void* header;

    std::uint32_t word(std::uint32_t offset) const noexcept {
        return *reinterpret_cast<volatile std::uint32_t*>(
            reinterpret_cast<std::uintptr_t>(header) + offset);
    }
    void word(std::uint32_t offset, std::uint32_t value) const noexcept {
        *reinterpret_cast<volatile std::uint32_t*>(
            reinterpret_cast<std::uintptr_t>(header) + offset) = value;
    }
};

// Specializations of the existing scene_graph2 shared-shape bodies. Original
// ECX is the three-DWORD header; RET4 except grow (RET0). These are source
// interfaces, not original register-ABI thunks. Names remain hypotheses.
void allocate_native_node_pool_table_00b6dea0(NativeNodePoolTableView, std::uint32_t);
void grow_native_node_pool_table_00b6def0(NativeNodePoolTableView);
void append_native_node_pool_table_00b6e580(NativeNodePoolTableView, const void* pointer_cell);
void reserve_native_node_pool_table_00b6e610(NativeNodePoolTableView, std::uint32_t);

} // namespace bsp
