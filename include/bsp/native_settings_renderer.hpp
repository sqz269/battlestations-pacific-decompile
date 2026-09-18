#pragma once
#include "bsp/native_settings_choices.hpp"
#include <cstdint>

namespace bsp {
// Complete original ECX-only leaf entries. Pointer addition wraps as x86.
void* __fastcall native_renderer_resolution_header_00b1fff0(void*) noexcept;
void* __fastcall native_renderer_antialias_header_00b20000(void*) noexcept;
std::int32_t __fastcall native_renderer_shader_ceiling_00b200b0(const void*) noexcept;
// B200C0 is exactly RET4; selecting a model has no effect in this executable.
void __fastcall select_native_renderer_shader_00b200c0(void*,std::uint32_t unused_edx,std::int32_t) noexcept;
// B295C0: ECX renderer, stack format, RET4. Actual vector at +28h, API +1990h.
// Reset, append zero, then call CURRENT IDirect3D9 slot2Ch for samples 2..15.
// Only HRESULT==0 appends; the captured format remains fixed while its old
// argument word becomes a persistent quality-output slot, initially format.
// ChoiceCalls defaults use the complete raw 86A430/86A220 array operations.
// Source interface adds that context; no original ABI/FH3/SEH claim.
void rebuild_native_renderer_antialias_00b295c0(void*,std::uint32_t format,NativeSettingsChoiceCalls&);
} // namespace bsp
