#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer pointer-array cleanup requires MSVC Win32.
#endif

namespace bsp {
// Actual 0Ch header: data+0, signed count+4, signed capacity+8. Only DWORD
// pointer cells are read/written: none of these bodies accesses pointees.
// Native reserves/resizes consume ECX=header, one stack word, RET4; the source
// fastcall adapters reserve EDX explicitly. Destructors use ECX and plain RET.
// Valid raw extents/lifetimes and wrapping address arithmetic are preconditions.
// Source CRT/private frames are new; no original SEH or game-validation claim.

// B22530..B2258E. Exact instruction/side-effect equivalent of existing B22D10,
// reused here: clamp request>=1, allocate/copy, free CURRENT old data, then
// publish new data/capacity. Count unchanged; allocation failure propagates.
void __fastcall reserve_native_renderer_query_pointers_00b22530(
    void*, std::uint32_t unused_edx, std::int32_t capacity);

// Reserve only if signed requested>current capacity. Capture current count
// after reserve; zero newly exposed cells using current base on each iteration,
// without publishing count during initialization. Then decrement CURRENT count
// while requested<count and finally store requested. No pointee release.
void __fastcall resize_native_renderer_query_pointers_00b22cc0(
    void*, std::uint32_t unused_edx, std::int32_t count);
void __fastcall resize_native_renderer_vertex_pointers_00b25940(
    void*, std::uint32_t unused_edx, std::int32_t count);
void __fastcall resize_native_renderer_index_pointers_00b259d0(
    void*, std::uint32_t unused_edx, std::int32_t count);

// Resize0, then load and free CURRENT data. Preserve stale data/capacity bits.
// A resize exception bypasses this body's final free, without rollback.
void __fastcall destroy_native_renderer_query_pointers_00b27f50(void*);
void __fastcall destroy_native_renderer_vertex_pointers_00b29b20(void*);
void __fastcall destroy_native_renderer_index_pointers_00b29b40(void*);
} // namespace bsp
