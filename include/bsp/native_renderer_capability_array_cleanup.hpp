#pragma once
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native capability array cleanup requires MSVC Win32.
#endif

namespace bsp {
// Actual 0Ch DWORD array: data+00, signed count+04, signed capacity+08.
// Every reserve/resize/destructor instruction and actual CALL target has
// been compared with the existing raw chain. These entries share that chain
// without a projected container, retained diagnostic operation, or pointee work.
// Native B260B0 is ECX header, signed stack count, RET4; source fastcall reserves
// EDX explicitly. Initialization captures current count after reserve, reloads
// current base per cell, skips null destinations, and publishes count last.
// Shrink decrements current count; removed DWORD cells retain their bits.
void __fastcall resize_native_capability_dwords_00b260b0(
    void*, std::uint32_t unused_edx, std::int32_t count);

// Native B29E40 is ECX header, plain RET. Resize0 precedes CURRENT data load
// and free. Preserve stale data/capacity; no header or pointee destruction.
// Allocation failure bypasses subsequent count publication/final free without
// rollback. Raw extents/lifetimes and wrapping addresses are preconditions.
// New source CRT/private frames; original SEH/exception identity not established.
void __fastcall destroy_native_capability_dwords_00b29e40(void*);
} // namespace bsp
