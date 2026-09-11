#pragma once
#include "bsp/gui_text_buffers.hpp"
#include "bsp/native_material_parameters.hpp"

namespace bsp {
class GuiTextLifetime;

// Complete00B19210 valid-storage sequence. ECX material, stack actual effect,
// RET4. Publish/retain new before release old, reload effect before dirty+B4,
// then destroy parameter names/return real pool slots even for equal effects.
// Reload parameter count after each return; leave stale slot cells intact.
// Uses the SAME canonical actual owners/parameter pool/string storage.
void set_native_material_shader_00b19210(NativeMaterialStorage&, void* actual_effect,
    NativeMaterialDestructionAccess&);

// Complete00B189F0 within actual material slots0..8/count0..9. ECX material,
// stack index and actual texture, RET8. Grow count BEFORE identity equality;
// publish then retain incoming/release captured old. No gap clearing.
void set_native_material_texture_00b189f0(NativeMaterialStorage&, std::uint32_t index,
    void* actual_texture, NativeRenderActualOwners&);

// Exact leaf wrappers, ECX material, name/source stack, RET8. Sources are
// borrowed DWORD ranges, not copied float values; names use native8h headers.
// Preserve EAX from the existing actual B17E10/B44D60 registration implementation.
NativeMaterialParameterStorage* register_native_material_float4_00b18aa0(
    NativeMaterialStorage&, const void* name_header, const void* source,
    NativeMaterialParameterAccess&);
NativeMaterialParameterStorage* register_native_material_float2_00b18b00(
    NativeMaterialStorage&, const void* name_header, const void* source,
    NativeMaterialParameterAccess&);
NativeMaterialParameterStorage* register_native_material_float_00b18b20(
    NativeMaterialStorage&, const void* name_header, const void* source,
    NativeMaterialParameterAccess&);

struct GuiTextShaderServices {
    GuiTextBufferServices& buffers;
    // SAME live pointer global; signed DWORD+28 is read only without override.
    void* const volatile& configuration_0109cf04;
};

// Complete00AB8CE0 supported normal path, ECX Text/no stack/RET/AL attempted.
// Cached nonnull returnsfalse. Override borrows existing string bytes; default
// names use the actual NativeString pool. Publish returned owned shader into
// lifetime's SAME+1EC before releasing a temporary name, even for null result.
// True means selection attempted, NOT load success. No second shader cache.
// Renderer48 must be a CURRENT callable native-ABI factory returning an actual
// registered owned effect; original numeric addresses are not callable hosts.
// Native Text string/vector ABI and SEH failure behavior are not reproduced.
bool ensure_gui_text_font_shader_00ab8ce0(GuiTextLifetime&, GuiTextShaderServices&);
} // namespace bsp
