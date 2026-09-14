#pragma once

#include "bsp/native_string.hpp"

#include <cstdint>

namespace bsp {

// Actual vector: leading allocator/iterator-owner word is untouched; begin,
// end, capacity-end are DWORDs at+4/+8/+Ch. Records have stride12Ch.
// Original B145E0 is thiscall(vector,ignored_owner,position,count,source),
// RET10h. This new fastcall interface adds the concrete pool context in EDX
// while preserving all four original stack slots. The implementation writes
// and rereads those ACTUAL slots as native construction/cleanup state.
void __fastcall insert_native_material_records_00b145e0(
    void* actual_vector, NativeStringRawPoolContext& strings,
    void* ignored_iterator_owner, void* position, std::uint32_t count,
    const void* source);

// Original B150D0 is thiscall(vector,output_pair,iterator_owner,position,source),
// RET10h/EAX output_pair. Preserve its four actual stack slots; EDX adds the
// raw context. The result pair stores actual vector then computed position.
void* __fastcall insert_one_native_material_record_00b150d0(
    void* actual_vector, NativeStringRawPoolContext& strings,
    void* output_pair, void* iterator_owner, void* position, const void* source);

// Original B15610 is thiscall(vector,source), RET4. EDX adds the raw context.
// Spare capacity publishes entry-captured end+300 AFTER copy construction;
// the slow path uses B150D0 and its returning invalid-parameter checks.
void __fastcall append_native_material_record_00b15610(
    void* actual_vector, NativeStringRawPoolContext& strings, const void* source);

// Allocation/free and invalid-parameter handling use the concrete current
// source CRT services, paired with native_material_record_allocation. Native
// raw string/record providers retain their own contracts. New C++ EH, CRT heap,
// handler/RTTI identity and private-frame layout are not original binary ABI.
// No bounds repair, exception rollback or typed-container storage is added.
} // namespace bsp
