#pragma once

namespace bsp {
// Actual C30470/BBC6F0/BBC810 storage, current D64478/D644B4 slot+2C.
// ECX source, EAX borrowed texture, RET; no retain or index validation.
void* __fastcall native_texture_source_current_texture_00c302f0(const void*) noexcept;
// D64478+30: ECX source, five native stack arguments, RET14. VS bytes1E/1F
// index the SAME caller-supplied VS header+4 bank. Entry/PS/PSheader unused.
void __fastcall native_caustics_source_constants_00bbcc40(const void*, void*,
    void* entry, const void* vertex_shader, const void* pixel_shader,
    void* vertex_bank_header, void* pixel_bank_header) noexcept;
// D644B4+30 is exactly RET14. This is the recovered native empty body.
void __fastcall native_shore_source_constants_00bbcbd0(const void*, void*,
    void*, const void*, const void*, void*, void*) noexcept;
} // namespace bsp
