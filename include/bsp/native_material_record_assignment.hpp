#pragma once

#include "bsp/native_string.hpp"

namespace bsp {

// Original B13310: cdecl(first,last,output_end,three ignored DWORDs), EAX
// advanced output, caller cleanup. Capture first/last, then capture output
// only for a nonempty range; decrement both cursors by300 before assignment.
// Equal inputs return the current output argument without reading a record.
// Source replaces the three unused words with the concrete raw pool context.
void* __cdecl copy_assign_native_material_records_backward_00b13310(
    const void* first, const void* last, void* output_end,
    NativeStringRawPoolContext& strings);

// Original B13720 has six cdecl stack DWORDs, but consumes only first/last/
// output_end. After copying backwards, ignore the child's EAX and return
// captured_output - 300*(signed32(captured_last-captured_first)/300), wrapping
// DWORD arithmetic. The native IMUL/SAR/sign correction is retained.
void* __cdecl copy_assign_native_material_record_head_00b13720(
    const void* first, const void* last, void* output_end,
    NativeStringRawPoolContext& strings);

// B13F50 is the three-argument cdecl forwarding wrapper for B13720; source
// adds the raw pool context and omits child iterator/tag padding proved unused.
void* __cdecl copy_assign_native_material_record_head_wrapper_00b13f50(
    const void* first, const void* last, void* output_end,
    NativeStringRawPoolContext& strings);

// B13AD0: cdecl(first,last,source). Capture source only for a nonempty range,
// assign ascending records, advance after return. No null-output skip and no
// cleanup/recovery on exception. Source adds the concrete raw pool context.
void __cdecl fill_assign_native_material_records_00b13ad0(
    void* first, void* last, const void* source,
    NativeStringRawPoolContext& strings);

// B14550: original ignored ECX owner, first/last/output stack words, RET0Ch,
// EAX child advanced output. Source is stdcall with an added context/RET10h.
// Delegates to the actual uninitialized-copy routine B13920, including its
// own output-argument slot publication and completed-prefix catch cleanup.
void* __stdcall copy_construct_native_material_record_tail_00b14550(
    const void* first, const void* last, void* output,
    NativeStringRawPoolContext& strings);

// Actual records are300 bytes with existing raw string headers at11Ch/124h.
// These are unchecked raw loops, not typed vectors or binary replacements.
// Original private stack padding/spill aliasing and hardware SEH are outside
// the new C++ context ABI. Native callee contracts and C++ exceptions remain.
} // namespace bsp
