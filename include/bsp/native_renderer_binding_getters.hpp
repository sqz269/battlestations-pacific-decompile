#pragma once

#include <cstdint>

namespace bsp {
// Complete native ECX-input/EAX-result leaves. Storage and returned pointers are
// borrowed. These entries do not validate, retain, create or release anything.

// B3CEA0: texture profile D61948+1C; actual +10 is IDirect3DTexture9*.
void* __fastcall native_texture_get_com_00b3cea0(
    const void* actual_texture) noexcept;

// B48CE0: logical vertex profile D61D6C+24; +68 is an engine declaration,
// not an IDirect3DVertexDeclaration9*. B48D10 returns the raw +5C offset word.
void* __fastcall native_logical_vertex_stream_get_declaration_00b48ce0(
    const void* actual_logical_vertex_stream) noexcept;
std::uint32_t __fastcall native_logical_vertex_stream_get_offset_00b48d10(
    const void* actual_logical_vertex_stream) noexcept;

// B48CF0/B48DC0: load physical owner at +58/+08, then its CURRENT table,
// then tail-jump through +1C with the physical owner in ECX. Native RET,
// EAX result and any exception behavior belong to the selected target.
// Rebuilt-host callers must supply callable relocated physical getter tables:
// D61E34/D61E7C +1C -> native_physical_vertex_buffer_get_com_00b4b9f0;
// D61E10/D61E58 +1C -> native_physical_index_buffer_get_com_00b4b840.
// Numeric original vtable addresses alone are not callable host bindings.
void* __fastcall native_logical_vertex_stream_get_buffer_00b48cf0(
    const void* actual_logical_vertex_stream);
void* __fastcall native_logical_index_stream_get_buffer_00b48dc0(
    const void* actual_logical_index_stream);

// Both physical profiles read +28. Heap and pooled owners share getters but
// have different lifetime paths; these entries do not select an allocator.
void* __fastcall native_physical_vertex_buffer_get_com_00b4b9f0(
    const void* actual_physical_vertex_buffer) noexcept;
void* __fastcall native_physical_index_buffer_get_com_00b4b840(
    const void* actual_physical_index_buffer) noexcept;

// B5FF00: hardware layout profile D62AF4+08; +40 is the borrowed
// IDirect3DVertexDeclaration9*. No lazy creation occurs.
void* __fastcall native_vertex_layout_get_com_00b5ff00(
    const void* actual_vertex_layout) noexcept;
} // namespace bsp
