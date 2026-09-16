#pragma once
#include <cstdint>

namespace bsp {
class LightTypeBootstrap;
class NativePointLightTypes;
struct NativeMaterialPassExecutionContext;
struct NativeMaterialPassExecutionFrame;
struct NativeRendererRecordGuardContext;
struct NativeTracelineRenderAccess;

// Borrow the SAME actual globals and owner/provider domains used by command,
// renderer and pass execution. Optional provider pointers are required only
// when their native branch is reached. Profile views contain original numeric
// targets, are never called as host tables, and remain live through callbacks.
struct NativeMaterialEntryDispatchContext {
    void* const volatile& actual_renderer_00f8d394;
    void* volatile& actual_cached_effect_0108fbf4;
    const volatile float& actual_one_00d7a24c;
    const volatile std::uint32_t* actual_renderer_profile_00d5f0a8;
    const volatile std::uint32_t* actual_effect_profile_00d61a00;
    NativeMaterialPassExecutionContext* passes{};
    const NativeTracelineRenderAccess* models{};
    NativeRendererRecordGuardContext* sphere_records{};
    LightTypeBootstrap* light_types{};
    NativePointLightTypes* point_light_types{};
    const volatile std::uint32_t* actual_light_profile_00d62f58{};
    const volatile std::uint32_t* actual_directional_profile_00d62fb0{};
    const volatile std::uint32_t* actual_point_profile_00d63008{};
};

// Caller-prepared persistent pass/constant frames, one for each reached B44750.
// No allocation, native ownership, cleanup or replay occurs here. Used frames
// and their constants survive failures; unused slots remain caller-owned.
struct NativeMaterialEntryDispatchFrame {
    NativeMaterialPassExecutionFrame* const* passes{};
    std::uint32_t capacity{};
    std::uint32_t used{};
};

// Full B45360[359]. Native ECX effect, stack entry, RET4. Mode2 special pass
// precedes descriptor/fade checks. Preserve COMISS/JBE (including unordered),
// unchecked mode indexing, captured collection/renderer table/camera, repeated
// CURRENT front and count reads, and CURRENT directional-type query. The light
// list uses genuine Light/DirectionalLight/PointLight predicates. Model virtual48
// uses the existing canonical world-sphere service; known B6E8C0 is direct.
// Mode0 debug sphere output is genuine B29270(sphere,camera,selector).
void dispatch_native_material_entry_00b45360(void* actual_effect,
    void* actual_entry, NativeMaterialEntryDispatchContext&,
    NativeMaterialEntryDispatchFrame*);

// Full B55550[70]. Native ECX actual18B batch, two unused stack words, RET8.
// Query current renderer+2C; inactive returns without clearing effect cache.
// Active clears SAME FB F4, then signed current-count loop over current list.
// Each entry follows section+20/material+7C to current D61A00 effect+14/B45360.
void execute_native_render_batch_entries_00b55550(void* actual_batch,
    std::uint32_t unused_index, void* unused_camera,
    NativeMaterialEntryDispatchContext&, NativeMaterialEntryDispatchFrame*);

// Source APIs add explicit contexts and persistent frame storage. Unknown
// concrete profile bindings are errors; no empty dispatcher or second owner
// domain is created. Original private-frame, FH3/SEH/register/fault ABI and
// application rendering are not established by these source interfaces.
} // namespace bsp
