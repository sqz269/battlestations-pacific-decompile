#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>
namespace bsp {
// Complete BE4360 (26 bytes): ECX stream, stack optional actual pointer,
// x87 ST0, RET4. One current slot24 read4 into pointer-seeded buffer, then
// FLD float32. The source entry preserves that x87 load without an extra store.
float __cdecl read_native_stream_float_00be4360(void* stream,
    std::uint32_t* actual, NativeResourceStreamReadContext&);
// Complete BF02C0 (39 bytes): ECX reader, stack budget, ST0, RET4. Current
// stream slot44; actual local is uninitialized before the call. FSTP32/FLD32
// precede the wrapping current-budget debit. Known target BE4360 is concrete;
// a different reached slot44 target is an unresolved binding and throws.
float __cdecl read_native_resource_float_00bf02c0(void* reader,
    std::uint32_t* budget, NativeResourceStreamReadContext&);
// Complete BE99D0 (15 bytes): ECX handle, ST0, RET. Capture current node,
// reader and remaining20 for one BF02C0 call; no extra floating-point store.
float __cdecl read_native_resource_node_float_00be99d0(void* handle,
    NativeResourceStreamReadContext&);
// ECX handle, EDX destination, RET. Sequential float32 stores after 4, 6 or
// 16 node reads. Reload the handle's node on EACH scalar call. No validation,
// sorting, transpose, early EOF exit or default matrix is introduced.
void read_native_resource_sphere_00b932e0(void* handle, void* output,
    NativeResourceStreamReadContext&);
void read_native_resource_bounds_00b93310(void* handle, void* output,
    NativeResourceStreamReadContext&);
void read_native_resource_matrix_00b936e0(void* handle, void* output,
    NativeResourceStreamReadContext&);
// Complete BE9A00 (15 bytes): ECX handle, EAX DWORD, RET. BF0280 actual debit.
std::uint32_t read_native_resource_node_dword_00be9a00(void* handle,
    NativeResourceStreamReadContext&);
// Complete BE9FE0 (34 bytes)/BEA010 (29 bytes). ECX node/handle respectively;
// stack output8h header; EAX captured output, RET4. Unused zeroed stack locals
// do not initialize output or the BF0510 actual-count argument slot.
void* read_native_resource_node_string_00be9fe0(void* node, void* output,
    NativeResourceStreamReadContext&);
void* read_native_resource_handle_string_00bea010(void* handle, void* output,
    NativeResourceStreamReadContext&);
// These are new MSVC Win32 C++ interfaces with explicit borrowed services.
// Assembly bridges retain x87 load/store order; original stack addresses,
// general registers/EFLAGS, native exceptions/SEH and fault delivery are not
// a binary ABI promise. The caller supplies live storage and x87 stack space.
}
