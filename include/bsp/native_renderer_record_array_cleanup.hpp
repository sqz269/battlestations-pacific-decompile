#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer member-array cleanup requires MSVC Win32.
#endif

namespace bsp {
// Native resize entries: ECX=actual header, one signed stack word, RET4;
// source fastcall reserves EDX explicitly. Captured initialization index,
// current base reloads, final count publication and failure path are shared
// with the instruction-equivalent existing raw B25940 implementation.
void __fastcall resize_native_renderer_vertex_shader_registry_00b25cf0(
    void*, std::uint32_t unused_edx, std::int32_t count);
void __fastcall resize_native_renderer_pixel_shader_registry_00b25d40(
    void*, std::uint32_t unused_edx, std::int32_t count);
void __fastcall resize_native_renderer_third_state_cache_00b22e90(
    void*, std::uint32_t unused_edx, std::int32_t count);

// Actual 0Ch header: data+00, signed count+04, signed capacity+08. Native
// bodies consume ECX=header and return with plain RET. These entries share
// the existing raw pointer cleanup implementation after full instruction
// equivalence of every resize and reserve dependency was established.
// Resize0 precedes CURRENT data load/free; stale data/capacity bits remain.
// No pointee layout, reference operation, or record destructor is involved.
// Reserve failure bypasses final count/free continuation without rollback.
// Raw extents/lifetimes and wrapping address arithmetic are preconditions;
// source CRT/private frames differ from original native SEH boundaries.
void __fastcall destroy_native_renderer_vertex_shader_registry_00b29b60(void*);
void __fastcall destroy_native_renderer_pixel_shader_registry_00b29b80(void*);
void __fastcall destroy_native_renderer_third_state_cache_00b28090(void*);
} // namespace bsp
