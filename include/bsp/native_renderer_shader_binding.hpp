#pragma once
#include "bsp/native_renderer_synchronization_actual.hpp"

namespace bsp {
// Actual renderer/logical storage: cached vertex+1770 or pixel+176C,
// incoming and cached logical COM pointer+8, current device+1A10.
// Borrow the same actual synchronization bytes0108D6DC..E3 as the parent.
// Optional guard entry precedes cached-logical capture; equal COM pointers
// retain the old logical identity. Other paths publish incoming first, even
// null-null (which skips COM). Current device slots170/1AC ignore HRESULT;
// counters+1BC0/+1BBC wrap after returning dispatch. No AddRef/Release.
//
// Entry-disabled/exit-enabled transitions can read an unwritten native guard
// record and are outside the valid native domain; no initialization is added.
// Current exit mode governs leave. Armed C++ exceptional cleanup uses actual
// B21110 and terminates on a second C++ exception, with no cache rollback.
// Native ECX renderer, stack logical, RET4; these new C++ interfaces do not
// reproduce original register/private-frame/FH3/SEH identity.
void bind_native_renderer_vertex_shader_00b21d10(void* actual_renderer,
    void* actual_logical_shader,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);
void bind_native_renderer_pixel_shader_00b21c20(void* actual_renderer,
    void* actual_logical_shader,
    NativeRendererSynchronizationGlobals& actual_synchronization_0108d6dc);
} // namespace bsp
