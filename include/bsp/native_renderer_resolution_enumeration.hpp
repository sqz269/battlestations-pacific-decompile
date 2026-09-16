#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native renderer resolution enumeration requires MSVC Win32.
#endif

namespace bsp {
// Actual pair-array header: data+0, DWORD count+4, signed capacity+8;
// each row is two ordered DWORDs. Native ECX header, stack pair pointer,
// RET4/EAX first index or -1. Captures base/end and key first word once;
// reloads key second word only when a candidate first word matches.
std::int32_t __fastcall find_native_resolution_pair_008d46c0(
    const void* header, std::uint32_t unused_edx, const void* pair);

// Native ECX header, signed stack request, RET4. Signed clamp1/growth;
// wrapping request*8 allocation, current base/count ordered pair copy,
// current old data free BEFORE replacement/capacity publication; count retained.
void __fastcall reserve_native_resolution_pairs_008d4750(
    void* header, std::uint32_t unused_edx, std::int32_t capacity);

// Native cdecl(two row pointers), RET/EAX signed wrapped width difference,
// or height difference when widths match. Genuine CRT qsort callback.
int __cdecl compare_native_resolution_pairs_00b1ffd0(const void*, const void*);

// Native B27D80 consumes ECX renderer and plain RET. New explicit EDX input
// supplies the native function's otherwise-uninitialized 16-byte mode scratch.
// Caller owns independent valid DWORD-aligned scratch with initialized preimage;
// it is reused without clearing and exposes all COM writes on return/failure.
// Actual renderer+1990 is current IDirect3D9, +1C/+20/+24 is the pair header,
// and +19DC is cleared. Capture mode count once; reload current COM interface
// for each mode. Accept HRESULT exactly0 and unsigned width>=640,height>=480;
// append absent pairs and unconditionally qsort current storage/count.
// No projected settings state, diagnostic operation, COM retain/release, or
// original EXE calls. Valid raw extents and wrapping-address domains required.
// No rollback after exceptions; new CRT/private-frame/SEH boundary.
void __fastcall enumerate_native_renderer_resolutions_00b27d80(
    void* actual_renderer, void* caller_owned_mode_scratch_16);
} // namespace bsp
