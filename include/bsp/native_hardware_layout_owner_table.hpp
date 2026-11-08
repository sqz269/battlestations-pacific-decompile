#pragma once

#include <cstdint>

namespace bsp {

// Borrowed actual 0108FD70 pool+28h: data/count/capacity at +0/+4/+8.
// This is the older hardware-layout owner, distinct from 0108FE9C.
// Original B47B60/B482D0: ECX header, one stack DWORD, RET4. These source
// interfaces preserve DWORD arithmetic/order, not the original binary ABI.
void set_native_hardware_layout_owner_table_capacity_00b47b60(
    void* actual_header, std::uint32_t capacity);
void reserve_native_hardware_layout_owner_table_00b482d0(
    void* actual_header, std::uint32_t capacity);

} // namespace bsp
