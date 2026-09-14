#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;

// BE99F0[15]: ECX current node-handle, EAX DWORD, RET. Capture node+20
// budget and node+8 reader, then use actual BF02A0/current stream slot38.
std::uint32_t read_native_resource_node_control_dword_00be99f0(
    void* actual_handle, NativeResourceStreamReadContext&);

// B72710[14]: ECX actual mesh, stacked float bits, RET4. MOVSS bit copy
// to mesh+0C; no arithmetic or structured-node read.
void set_native_mesh_lod_00b72710(void* actual_mesh, std::uint32_t float_bits) noexcept;

// B73270[47]: ECX actual mesh, stacked pointer to four DWORDs, RET4.
// Capture count+50 and destination+10+count*16, then copy each CURRENT
// source word in order. Increment CURRENT count after all four stores.
// No physical four-slot bound check; source/destination may overlap.
void append_native_mesh_lod_phase_00b73270(void* actual_mesh, const void* actual_phase) noexcept;

// B73D50[11]: ECX mesh, stacked actual counted-string header, tail RET4.
// Actual 4CDC20 deep-copy append at mesh+B0, with the same raw string pool
// publication/lifetime/gate as the reader. Its existing NativeStringStorage
// boundary terminates if the returning pool getter throws during release.
void append_native_mesh_weight_name_00b73d50(void* actual_mesh,
    const void* actual_name, NativeStringRawPoolContext&);

// B93590[45]/B935C0[63]: incoming ECX unused, stacked mesh (ignored) and
// handle, RET8. Four/six BE99D0 reads, each popped directly from x87 ST0;
// no extra float memory store, bounds publication, skip or early EOF guard.
void consume_native_mesh_sphere_00b93590(void* actual_mesh,
    void* actual_handle, NativeResourceStreamReadContext&);
void consume_native_mesh_bounds_00b935c0(void* actual_mesh,
    void* actual_handle, NativeResourceStreamReadContext&);

// B93710[130]: incoming ECX unused, stacked mesh/handle, RET8. Unsigned
// BE99F0 count; each phase reads current constants before replacing them
// via two direct FSTP32 stores, then two BE9A00 DWORDs and actual append.
// Each scalar reloads the current node. Already appended phases survive
// later read failure. No capacity, remaining-byte or count validation.
void read_native_mesh_lod_phases_00b93710(void* actual_mesh,
    void* actual_handle, NativeResourceStreamReadContext&,
    const volatile std::uint32_t& minimum_00ce4adc,
    const volatile std::uint32_t& maximum_00ce4970);

// B93F90[142]: stacked mesh/handle, RET8. While CURRENT node+20 !=0,
// BEA010 into local8h; append its RETURNED header, then return captured
// local data through the current pool. EH owns only a completed local
// string, not previously appended names; disarm before normal return.
void read_native_mesh_weight_names_00b93f90(void* actual_mesh,
    void* actual_handle, NativeResourceStreamReadContext&);

// New explicit-context MSVC Win32 interfaces, not original ABI/FH3/SEH or
// gameplay proof. Complete mesh geometry field decoding remains separate.
} // namespace bsp
