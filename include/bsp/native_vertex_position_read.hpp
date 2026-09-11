#pragma once
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <cstdint>

namespace bsp {
// Actual named d3dx9_40 import, not a conversion callback or alternate DLL.
// Caller keeps that module loaded for this binding and every active read.
// The uint16 input has the native D3DXFLOAT16 two-byte representation.
class NativeD3dx9Float16Import final {
public:
    explicit NativeD3dx9Float16Import(HMODULE actual_d3dx9_40);
    NativeD3dx9Float16Import(const NativeD3dx9Float16Import&) = delete;
    NativeD3dx9Float16Import& operator=(const NativeD3dx9Float16Import&) = delete;
    float* convert(float* output, const std::uint16_t* input, UINT count) const;
private:
    using Function = float* (WINAPI*)(float*, const std::uint16_t*, UINT);
    Function function_;
};

//00475F80..00475FE3, ECX unused, packed DWORD and out float3 stack, RET8.
// Extracts nine bits at shifts0/10/20, individually FILD/FSTP to output.
// This is the observed native operation, not a ten-bit normalized decoder.
void unpack_native_position_9bit_00475f80(std::uint32_t packed, float* output);

//004768D0..00476B40, ECX logical vertex stream, out float3 and index stack,
// native EAX=out pointer, RET8. New C++ interface consumes the SAME live raw
// logical stream/mapping and its retained +50 metadata; no stream/vertex copy.
// Complete for +50==0 and initialized native type cases2,3,5,7,8,10,12,13,16.
// false leaves output untouched for other nonzero+50 types: native takes an
// uninitialized-local path. No default format or replacement values supplied.
// Reads current +18/+50 AFTER decoding, including the actual half import.
// Preserves type13's base+4*(stride*index+position_offset) address arithmetic.
// Caller supplies valid mapping/metadata/output extents and keeps owners and
// import module live; no locks, retains, allocation or implicit DLL load occur.
// Output may overlap source/metadata: native input staging and final store
// order are retained. Invalid pointer faults/unmasked FP traps are not wrapped.
bool read_native_vertex_position_004768d0(void* actual_logical_vertex_stream,
    std::uint32_t vertex_index, float (&output)[3],
    const NativeD3dx9Float16Import& actual_half_import);
} // namespace bsp
