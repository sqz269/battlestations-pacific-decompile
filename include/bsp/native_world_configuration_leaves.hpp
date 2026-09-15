#pragma once

#include <cstdint>

namespace bsp {

// Complete raw Win32 entries: original ECX receiver, public DWORD at ESP+4,
// RET4. Source fastcall EDX is unused; pass nullptr. Caller supplies actual
// writable backing/lifetime. These entries perform no validation or setup.

// B6FE50..B6FE5C: copy the full word to receiver+190h. EAX receives the word.
void __fastcall set_native_camera_clear_color_00b6fe50(
    void* actual_camera, void* unused_source_edx, std::uint32_t word);

// B1FFB0..B1FFBC: copy the full word to receiver+1D84h. EAX receives the word.
// Any detail-bias calculation belongs to the caller.
void __fastcall set_native_renderer_texture_detail_bias_00b1ffb0(
    void* actual_renderer, void* unused_source_edx, std::uint32_t word);

// AD5750..AD5759: copy only the public word's low byte to receiver+10h.
// AL receives that byte; upper EAX bits are untouched. No bool normalization.
void __fastcall set_native_foliage_byte_00ad5750(
    void* actual_owner, void* unused_source_edx, std::uint32_t word);

} // namespace bsp
