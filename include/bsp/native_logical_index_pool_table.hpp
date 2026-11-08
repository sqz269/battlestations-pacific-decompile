#pragma once

#include <cstdint>

namespace bsp {

// Borrowed actual pool+28h: pointer/count/capacity at +0/+4/+8.
// Original ABI: ECX header; exact/reserve take one stack DWORD (RET 4),
// append takes a borrowed source-cell pointer (RET 4), grow takes none (RET).
// These C++ interfaces preserve state/order; they are not binary ABI shims.
void set_native_logical_index_pool_table_capacity_00b49800(
    void* header, std::uint32_t capacity);
void grow_native_logical_index_pool_table_00b49850(void* header);
void append_native_logical_index_pool_table_00b4a6f0(
    void* header, void* const* source_cell);
void reserve_native_logical_index_pool_table_00b4a770(
    void* header, std::uint32_t capacity);

} // namespace bsp
