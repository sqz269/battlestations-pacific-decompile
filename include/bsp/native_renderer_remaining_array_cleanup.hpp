#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer array cleanup requires MSVC Win32.
#endif

namespace bsp {
// Actual 0Ch header: data+0, signed count+4, signed capacity+8. No routine
// reads pointees or owns a referenced resource. Valid raw extents and lifetimes
// remain required; address/count arithmetic wraps as native DWORD arithmetic.
// The complete 80-byte resizes are instruction-equivalent to existing raw
// 737390 after their reserve CALL displacement is normalized. Their concrete
// B226B0/B22710/B22850 reserves likewise match existing raw735FF0. Both full
// existing providers are reused, including genuine allocation/free behavior.
// Native: ECX header, stacked signed count, RET4. EDX is reserved explicitly
// by these source fastcall adapters. Source private frames/CRT are new.
void __fastcall resize_native_renderer_array_00b22ee0(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_count);
void __fastcall resize_native_renderer_array_00b22f30(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_count);
void __fastcall resize_native_renderer_array_00b22f80(
    void* actual_header, std::uint32_t unused_edx, std::int32_t requested_count);

// Complete 23-byte destructors: ECX header, plain RET. Resize0 first; only
// after it returns, load/free CURRENT data. Stale data/capacity bits remain.
// Resize allocation failure propagates before final free, with no rollback.
// Negative capacity can therefore allocate even on a resize-to-zero path.
// B32410 FH3 states15..18 respectively hold these actual renderer offsets.
// No original FH3/SEH, unrestricted reentrancy or gameplay claim is made.
void __fastcall destroy_native_renderer_array_00b280b0(void* actual_header); // +1AE8
void __fastcall destroy_native_renderer_array_00b280d0(void* actual_header); // +1AF4
void __fastcall destroy_native_renderer_array_00737bf0(void* actual_header); // +1B00
void __fastcall destroy_native_renderer_array_00b280f0(void* actual_header); // +1B0C
} // namespace bsp
