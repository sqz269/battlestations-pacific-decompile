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

// Same borrowed storage/import contract. Normal fields are +1C/+20/+24;
// initialized decoded types are2,4,5,7,8,10,12,13,16. Type3 is NOT initialized.
// The first scale multiplication loads the decoded lane before its scale.
// Native ECX stream, stack output/index, EAX output, RET8; new C++ ABI.
bool read_native_vertex_normal_0070fdb0(void* actual_logical_vertex_stream,
    std::uint32_t vertex_index, float (&output)[3],
    const NativeD3dx9Float16Import& actual_half_import);

// UV fields +28/+2C/+30, initialized decoded types1,6,9,11,15. Short types
// read TWO DWORDs and convert THREE lanes even though output has two lanes.
// Unsupported decoded types return false without output writes, matching the
// position reader's explicit boundary around native uninitialized scratch.
bool read_native_vertex_uv_007100a0(void* actual_logical_vertex_stream,
    std::uint32_t vertex_index, float (&output)[2],
    const NativeD3dx9Float16Import& actual_half_import);

// Packed offset+34 >=0 copies a DWORD. Otherwise actual float offsets+38/3C/
// 40/44 supply output byte2/1/0/3. Each value is multiplied by double255 then
// converted with temporary x87 truncation; low bytes wrap, without clamping.
// Native control word is restored after each conversion. Valid backing and
// masked floating exceptions are required; exception/fault delivery unproved.
void read_native_vertex_colour_00476180(void* actual_logical_vertex_stream,
    std::uint32_t vertex_index, std::uint32_t& output);
} // namespace bsp
