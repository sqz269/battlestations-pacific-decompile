#pragma once
#include "bsp/gui_native_geometry.hpp"
#include "bsp/gui_text.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/native_node_parenting.hpp"
#include "bsp/gui_text_native_renderer.hpp"

namespace bsp {
// Actual existing owner domains, not fake factory callbacks or Text storage.
// All services/profile/global slots must outlive the resources they create.
struct GuiTextBufferServices {
    GuiWidgetOwnerRuntime& widgets;
    GuiNativeGeometryOwners& geometry;
    NativeMaterialDestructionAccess& materials;
    NativeNodeParentingRuntime& parenting;
    NativeStringStorage& strings;
    void* const volatile& current_renderer_00f8d394;
    const volatile std::uint32_t* material_vtable_00d5e520;
    // Required by AB8400/AB8910 when creating actual buffers. Missing binding
    // is an explicit source boundary; no numeric vtable is called as host code.
    GuiTextNativeRendererServices* native_renderer{};
};

enum class GuiTextGlyphBuffersPhase { not_started, running, complete, failed };
struct GuiTextGlyphBuffersAcquired {
    GuiTextGlyphBuffersPhase phase{GuiTextGlyphBuffersPhase::not_started};
    std::uint32_t native_site{};
    GuiNativeDeclarationAcquired declaration;
    NativeStreamCloneAcquired vertex;
    NativeStreamCloneAcquired index;
};

//00AB8400 complete normal path: two stack DWORDs (capacity, actual mesh),
// ECX unused, RET8. Reload current renderer before38/5C/60, request actual
// declaration and registered streams, publish via existing raw mesh setters,
// release vertex then declaration BEFORE index creation, then release index.
// Counts use native DWORD multiplication, with no capacity reuse or clamp.
// Concrete current38/5C/60 adapters dispatch actual cache/vertex/index factories.
// Acquired is caller-owned and cannot be replayed after a partial failure.
void create_gui_text_glyph_buffers_00ab8400(std::uint32_t glyph_capacity,
    NativeMeshStorage&, GuiTextBufferServices&, GuiTextGlyphBuffersAcquired&);
void create_gui_text_glyph_buffers_00ab8400(std::uint32_t glyph_capacity,
    NativeMeshStorage&, void* const volatile& current_renderer_00f8d394,
    NativeStringStorage&, GuiTextNativeRendererServices&, GuiTextGlyphBuffersAcquired&);
// Legacy signature cannot supply actual decoder/factory/terminal contexts.
// It throws explicitly until the caller migrates to the operation above.
void create_gui_text_glyph_buffers_00ab8400(std::uint32_t glyph_capacity,
    NativeMeshStorage&, void* const volatile& current_renderer_00f8d394,
    NativeStringStorage&, NativeRenderActualOwners&);

//00AB8530 complete supported normal path, native ECX Text/RET. The widget,
// existing GuiTextWidget and LIVE shadow+188 association describe one Text.
// Creates actual Shadow/model geometry/sections/materials using canonical
// pools and owners; no secondary hierarchy or Text state. Existing shadow
// skips all shadow creation/repair. Main has no geometry -> no section added.
// Publish Shadow before name release; append section BEFORE material creation;
// release creator references in native order; reread flags/shadow after calls.
// No buffer upload, shader-font setup, draw or Text destructor is supplied.
// Valid actual owners/callable renderer profiles and successful native
// allocations are prerequisites; pool/SEH failure behavior is not claimed.
void ensure_gui_text_draw_sections_00ab8530(GuiWidgetOwner&, GuiTextWidget&,
    NativeNodeBinding*& shadow_188, GuiTextBufferServices&);
} // namespace bsp
