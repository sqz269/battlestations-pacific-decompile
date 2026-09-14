#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Original B13B00: ECX output, EDX unsigned count, four stack DWORD slots
// (source, ignored owner, ignored output, ignored tag), RET10h. The source
// interface keeps ECX/EDX and the live first stack slot, adds the raw context
// in the second stack slot, and uses RET8. Source is reread before each copy.
void __fastcall fill_construct_native_material_records_00b13b00(
    void* output, std::uint32_t count, const void* source,
    NativeStringRawPoolContext&);

// Original B13920: ECX first, EDX last, four stack slots (output and three
// ignored iterator/tag words), RET10h, EAX final output. This source retains
// the actual first output-argument slot: it is advanced after each successful
// iteration and reread on catch entry. Its context replaces the ignored words,
// yielding two stack slots and RET8. Native equality termination is retained.
void* __fastcall copy_construct_native_material_records_00b13920(
    const void* first, const void* last, void* output,
    NativeStringRawPoolContext&);

// Original B14360: ECX ignored owner/tag word; stack output/count/source,
// RET0Ch, EAX output + 300*count with DWORD wrap. The new source interface
// adds the raw context as a fourth stack argument (__stdcall RET10h).
void* __stdcall fill_construct_native_material_record_tail_00b14360(
    void* output, std::uint32_t count, const void* source,
    NativeStringRawPoolContext&);

// Original B143A0: __stdcall first/last, RET8. New third argument is context.
// Both construction loops clean up completed output in ascending address order
// on a C++ exception, then rethrow. Destruction failure replaces that exception;
// there is no catch-body retry or additional terminate wrapper.
void __stdcall destroy_native_material_record_range_00b143a0(
    void* first, const void* last, NativeStringRawPoolContext&);

// Actual record storage, valid ranges and clear DF are caller obligations.
// No bounds repair, rollback of partial record storage, or allocation is added.
// Native placement-failure funclets call verified RET-only 00401130. Their
// private frame, ignored padding, hardware SEH, and binary FH3 identity are
// outside these source interfaces; they are not binary drop-in replacements.

} // namespace bsp
