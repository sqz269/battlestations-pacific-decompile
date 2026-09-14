#pragma once

#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {

// Native ECX is the actual renderer; the one callee stack slot contains a
// borrowed logical shader (physical IDirect3D*Shader9 pointer at +8), RET 4.
// EDX adds the shared actual synchronization globals to this source interface.
// The original caller argument slot is read after optional guard entry.
// Native renderer/device/COM layout, lifetime, and current lock state are caller
// obligations. Cache identity and HRESULT-ignoring counter behavior are retained.
// Entry-disabled/exit-enabled guard storage and binary FH3 identity are outside
// the valid source interface; no validation or ownership operations are added.
void __fastcall bind_native_renderer_pixel_shader_00b21c20(
    void* actual_renderer, NativeRendererSynchronizationGlobals*,
    const void* actual_logical_shader);
void __fastcall bind_native_renderer_vertex_shader_00b21d10(
    void* actual_renderer, NativeRendererSynchronizationGlobals*,
    const void* actual_logical_shader);

} // namespace bsp
