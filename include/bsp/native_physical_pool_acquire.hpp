#pragma once

#include <cstdint>

namespace bsp {
struct NativePhysicalBufferOwnerContext;

// Complete B48DF0[64]: ECX raw644h slab, stacked slab index, EAX same, RET4.
// Source reserves EDX to keep the index stacked. Initializes only free count,
// 32 free indices, and each raw30h slot's +2C slab-index word.
void* __fastcall initialize_native_physical_buffer_slab_00b48df0(
    void* actual_slab, void* unused_edx, std::uint32_t slab_index) noexcept;

// Complete B4AC60[316]: original ECX initialized raw38h pool, EAX raw30h slot,
// RET. Real pool+0C critical section, depth+24, slabs+28/count+2C/capacity+30,
// earliest available slab+34. Actual CRT allocation/free and current-word reads.
// No lock or allocation rollback on exceptions; pool initialization is separate.
void* __fastcall acquire_native_physical_buffer_slot_00b4ac60(void* actual_pool);

// Original ten-byte no-argument tail thunks select actual globals108FDA8/FDE0.
// These new interfaces borrow the same pools from the existing owner context;
// they do not read its string/support/lifetime fields or create substitute pools.
void* __fastcall acquire_native_index_buffer_slot_00b4b350(
    const NativePhysicalBufferOwnerContext& actual_context);
void* __fastcall acquire_native_vertex_buffer_slot_00b4b360(
    const NativePhysicalBufferOwnerContext& actual_context);
} // namespace bsp
