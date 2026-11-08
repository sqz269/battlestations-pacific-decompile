#pragma once

#include <cstdint>

namespace bsp {

// Borrow the actual three-DWORD {data, count, capacity} header. The canonical
// tracer pool 0109049C stores it at +28h; there is no copied table authority.
struct NativeTracerPoolTableView {
    void* header;
};

// Complete BA9D10/BAC190/BAC210 bodies. Original ECX is the header; grow has
// RET, append takes a pointer CELL and RET4, reserve takes a DWORD and RET4.
// Preserve unsigned wrap, capacity-before-allocation, live count/data reloads,
// old-table free before publication, and append's source load AFTER growth.
// Names are hypotheses; these source interfaces are not binary ABI bridges.
void grow_native_tracer_pool_table_00ba9d10(NativeTracerPoolTableView);
void append_native_tracer_pool_table_00bac190(NativeTracerPoolTableView,
    const void* pointer_cell);
void reserve_native_tracer_pool_table_00bac210(NativeTracerPoolTableView,
    std::uint32_t requested);

} // namespace bsp
