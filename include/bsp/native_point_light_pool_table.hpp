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
void reserve_native_point_light_pool_table_00b7b4a0(
    NativePointLightPoolTableView table, std::uint32_t minimum);

} // namespace bsp
