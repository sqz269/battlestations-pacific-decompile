#pragma once

#include <cstdint>

namespace bsp {

// Complete six native bodies, 502 bytes; evidence and exact inclusive ranges
// in docs/NATIVE_RENDER_SERVICE_TEXTURE_VECTORS.md. Actual storage is three
// DWORDs: pointer+0, signed count+4, signed capacity+8. No host vector or owner
// lifetime is constructed. Record strides are 24h and 0Ch, respectively.
// Native ECX header, one requested-count/capacity stack DWORD, RET4; destructors
// have no stack arguments and RET. The new fastcall count input is EDX; a
// private entry shim supplies the native argument slot. Private argument/stack
// aliases, original binary caller ABI, EH/SEH identity and gameplay are unproved.
// Reserve uses signed minimum1 and DWORD allocation-size wrap, the established
// CRT new-handler allocator and matching free. Keep native current field reads,
// forward copies, computed-null element skips, free-before-publication order
// and shrinking count stores. Destruction leaves pointer/capacity untouched.
// Accessed storage must remain valid at native accesses; no null/header/count
// validation or allocator failure fallback is added. Allocator exceptions escape.
void __fastcall reserve_native_texture_vector24_00b51fc0(void* actual_header, std::int32_t requested);
void __fastcall reserve_native_texture_vector12_00b52040(void* actual_header, std::int32_t requested);
void __fastcall resize_native_texture_vector24_00b52170(void* actual_header, std::int32_t requested);
void __fastcall resize_native_texture_vector12_00b521e0(void* actual_header, std::int32_t requested);
void __fastcall destroy_native_texture_vector24_00b523c0(void* actual_header);
void __fastcall destroy_native_texture_vector12_00b523e0(void* actual_header);

} // namespace bsp
