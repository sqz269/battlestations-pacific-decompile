#pragma once
#include "bsp/native_resource_stream_reads.hpp"
#include <cstdint>

namespace bsp {
// Complete source bodies for the registered "SkinedMeshAnimation" item.
// Actual item18h: profile/refcount +0/+4, uninterpreted DWORD+8, vector+C.
// Vector: actual pointer/count/capacity at +0/+4/+8 (signed comparisons).
// Outer element18h: float32 bits+0, actual pooled name+4, inner vector+C.
// Inner element30h: twelve sequential float32 values; no matrix interpretation.
// All addresses remain in the caller's actual stream/string/CRT domains.
// Names are descriptive hypotheses, not recovered symbols.
void* read_native_animation_values_00b92300(void* output30h, void* handle,
    NativeResourceStreamReadContext&);
void reserve_native_animation_values_00b92440(void* vector, std::uint32_t capacity);
void* assign_native_animation_values_00b926e0(void* vector, const void* source);
void destroy_native_animation_record_00b928b0(void* record, NativeStringRawPoolContext&);
// Source interface consumes the supplied actual8h by-value name copy, including
// on constructor failure. Native ECX record; stacked float bits/name8h; RET12.
void* construct_native_animation_record_00b92970(void* record, std::uint32_t value_bits,
    void* owned_argument_name8h, NativeStringRawPoolContext&);
void* copy_native_animation_record_00b92a50(void* record, const void* source,
    NativeStringRawPoolContext&);
void reserve_native_animation_records_00b92ae0(void* vector, std::uint32_t capacity,
    NativeStringRawPoolContext&);
void resize_native_animation_records_00b92bc0(void* vector, std::uint32_t count,
    NativeStringRawPoolContext&);
void append_native_animation_record_00b92c40(void* vector, const void* source,
    NativeStringRawPoolContext&);
void read_native_skinned_animation_item_00b92cd0(void* item, void* handle,
    NativeResourceStreamReadContext&);
void destroy_native_skinned_animation_item_00b92ec0(void* item, NativeStringRawPoolContext&);
void* parse_native_skinned_animation_item_00b92f20(void* handle, NativeResourceStreamReadContext&);
void* delete_native_skinned_animation_item_00b92fa0(void* item, std::uint32_t flags,
    NativeStringRawPoolContext&);
// Source ABI only. Original register ABI, private stack aliases, FH3/SEH and
// CRT exception identity are outside this interface; x87 stores are retained.
// Parsing failures retain native partial mutations and allocation ownership.
} // namespace bsp
