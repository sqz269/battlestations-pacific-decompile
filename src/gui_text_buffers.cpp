#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_material_binding.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

} // namespace

void create_gui_text_glyph_buffers_00ab8400(std::uint32_t,
    NativeMeshStorage&, void* const volatile&, NativeStringStorage&, NativeRenderActualOwners&) {
    throw std::logic_error("Text glyph buffers require concrete native renderer services and acquired state");
}

void create_gui_text_glyph_buffers_00ab8400(std::uint32_t capacity,
    NativeMeshStorage& mesh, GuiTextBufferServices& buffers, GuiTextGlyphBuffersAcquired& acquired) {
    require(buffers.native_renderer && &buffers.native_renderer->streams.geometry == &buffers.geometry,
        "Text glyph buffers require same-domain native services");
    create_gui_text_glyph_buffers_00ab8400(capacity, mesh, buffers.current_renderer_00f8d394,
        buffers.strings, *buffers.native_renderer, acquired);
}

void create_gui_text_glyph_buffers_00ab8400(std::uint32_t capacity,
    NativeMeshStorage& mesh, void* const volatile& renderer_slot, NativeStringStorage& strings,
    GuiTextNativeRendererServices& native, GuiTextGlyphBuffersAcquired& acquired) {
    require(acquired.phase == GuiTextGlyphBuffersPhase::not_started,
        "Text glyph buffers require one fresh operation");
    auto& owners = native.streams.geometry.actual_owners();
    require_gui_text_native_renderer_domain(native, renderer_slot, strings, owners);
    acquired.phase = GuiTextGlyphBuffersPhase::running;
    try {
    NativeString format;
    format.resize_0041dd40(strings, 0x10, true);
    if (format.data()) std::memcpy(format.data(), "simplecolor.mvfm", format.length() + 1u);
    try {
        void* renderer = renderer_slot;
        acquired.native_site = 0x00ab8465;
        (void)load_gui_text_native_declaration_current38(renderer, format, native, acquired.declaration);
    } catch (...) {
        destroy_native_string_header_0041dd20(&format, strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&format, strings);
    void* renderer = renderer_slot;
    acquired.native_site = acquired.vertex.native_site = 0x00ab84aa;
    void* vertex = create_gui_text_native_vertex_current5c(renderer, capacity * 4u, 1u,
        acquired.declaration.reference, native, acquired.vertex);
    acquired.native_site = 0x00ab84b4;
    set_native_mesh_vertex_stream_00b73bb0(mesh, owners, 0, vertex);
    acquired.vertex.creator = nullptr;
    acquired.vertex.companion = nullptr;
    acquired.vertex.phase = NativeStreamClonePhase::consumed;
    acquired.native_site = 0x00ab84c3;
    release_native_render_actual_owner(owners, vertex);
    void* declaration = acquired.declaration.reference;
    acquired.declaration = {};
    acquired.native_site = 0x00ab84d5;
    release_native_render_actual_owner(owners, declaration);
    renderer = renderer_slot;
    acquired.native_site = acquired.index.native_site = 0x00ab84f9;
    void* index = create_gui_text_native_index_current60(renderer, capacity * 6u, 1u,
        0x65u, native, acquired.index);
    acquired.native_site = 0x00ab8502;
    set_native_mesh_index_stream_00b73b70(mesh, owners, index);
    acquired.index.creator = nullptr;
    acquired.index.companion = nullptr;
    acquired.index.phase = NativeStreamClonePhase::consumed;
    acquired.native_site = 0x00ab850b;
    release_native_render_actual_owner(owners, index);
    acquired.phase = GuiTextGlyphBuffersPhase::complete;
    } catch (...) {
        acquired.phase = GuiTextGlyphBuffersPhase::failed;
        throw;
    }
}

} // namespace bsp
