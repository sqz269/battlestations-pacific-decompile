#pragma once

#include <cstdint>

namespace bsp {

// Borrow the live three-DWORD {data,count,capacity} header, here at the actual
// 0108FE18 logical-vertex pool +28h. These are ordinary C++ interfaces, not
// Original ECX/RET4 thunks (grow has RET0). Names are hypotheses.
// Evidence: bsp.py show 00B498A0/00B498F0/00B4A7D0/00B4A850 and 00B498A0
// --asm. The unused template bodies match the operations inlined by B4ADA0
// and B4AE80; this specialization does not assert recovered template types.
// Capacity is written before allocation; live count/data are reloaded while
// copying, the old allocation is freed, and data is published last. Arithmetic
// wraps as DWORDs. Append consumes its pointer cell only after possible growth.
// Caller owns synchronization, header/storage lifetime and valid input cells.
void allocate_native_logical_vertex_pool_table_00b498a0(void* header, std::uint32_t capacity);
void grow_native_logical_vertex_pool_table_00b498f0(void* header);
void append_native_logical_vertex_pool_table_00b4a7d0(void* header, const void* pointer_cell);
void reserve_native_logical_vertex_pool_table_00b4a850(void* header, std::uint32_t capacity);

} // namespace bsp
