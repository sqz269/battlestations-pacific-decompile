#pragma once
#include <cstddef>
#include <cstdint>

namespace bsp {
// Hypothesis names for 00B74570/00B745C0/00B747D0/00B74860. Native ECX is
// the actual {data, count, capacity} header at Model pool 01090054 + 28h.
// Allocate/reserve/push are __thiscall RET4; grow is __fastcall RET.
// These new C++ entry points borrow that header; no copied table state.
// Capacity is published before the allocator/new-handler can reenter, and
// source/count are read again afterwards. Allocation/free use BF55BE/BF6989's
// existing process services. The selected scene2 dispatcher cases delegate
// here too, so each address has one implementation. Native ABI/gameplay unproven.
void set_native_model_pool_table_capacity_00b74570(void* header, std::uint32_t capacity);
void grow_native_model_pool_table_00b745c0(void* header);
void append_native_model_pool_slab_00b747d0(void* header, std::byte* const& slab);
void reserve_native_model_pool_table_00b74860(void* header, std::uint32_t capacity);
} // namespace bsp
