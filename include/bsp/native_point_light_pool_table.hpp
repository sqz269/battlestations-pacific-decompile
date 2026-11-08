#pragma once
#include <cstdint>

namespace bsp {

// A borrowed view of the actual {data,count,capacity} header at pool+28h.
// No copied header is published: allocation/new-handler callbacks observe the
// same capacity and old data pointer as the pool owner and generic dispatch.
struct NativePointLightPoolTableView { void* header; };

// Native __thiscall(ECX=header, stack=capacity), RET 4. These source interfaces
// are not binary ABI thunks. Names are hypotheses, not recovered symbols.
void allocate_native_point_light_pool_table_00b7ade0(
    NativePointLightPoolTableView table, std::uint32_t capacity);
// 00B7AE30: ECX=header, no stack arguments. 00B7B410: ECX=header,
// stack=pointer-cell address, RET4. Append reads that cell after any growth.
void grow_native_point_light_pool_table_00b7ae30(NativePointLightPoolTableView table);
void append_native_point_light_pool_table_00b7b410(
    NativePointLightPoolTableView table, const void* pointer_cell);
void reserve_native_point_light_pool_table_00b7b4a0(
    NativePointLightPoolTableView table, std::uint32_t minimum);

} // namespace bsp
