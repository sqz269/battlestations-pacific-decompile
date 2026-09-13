#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_material_binding.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

NativeModelOwner& actual_model(NativeNodeBinding* node, GuiTextBufferServices& services) {
    require(node != nullptr, "Text sections require an actual model node");
    auto* lifetime = services.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node->storage));
    auto* reference = dynamic_cast<NativeModelReference*>(lifetime);
    require(reference != nullptr, "Text sections require the canonical model reference");
    auto& model = reference->model_owner();
    require(&model.node == node && &model.storage.node == &node->storage &&
        &model.environment.nodes == &services.parenting.nodes &&
        &model.environment.retained_owners == &services.geometry.actual_owners() &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text model storage, lifetime and retained-owner domains must agree");
    return model;
}

void append_default_section(NativeMeshStorage& mesh, GuiTextBufferServices& services) {
    auto& owners = services.geometry.actual_owners();
    auto* section = services.geometry.create_section();
    append_native_mesh_draw_section_00b73c60(mesh, section);
    NativeString effect_name;
    effect_name.assign_0041e870(services.strings, "guidefault.mshd");
    NativeMaterialStorage* material;
    try {
        material = services.geometry.create_material_for_effect_00535320(effect_name,
            services.current_renderer_00f8d394, services.materials,
            services.material_vtable_00d5e520);
    } catch (...) {
        destroy_native_string_header_0041dd20(&effect_name, services.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&effect_name, services.strings);
    set_native_mesh_section_material_00b864c0(*section, owners, material);
    release_native_render_actual_owner(owners, section);
    release_native_render_actual_owner(owners, material);
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

void ensure_gui_text_draw_sections_00ab8530(GuiWidgetOwner& widget, GuiTextWidget& text,
    NativeNodeBinding*& shadow, GuiTextBufferServices& services) {
    require(widget.layout().type == GuiWidgetType::Text &&
        widget.layout().transform.type_id == 3 &&
        &services.widgets.owner(widget.layout()) == &widget,
        "Text sections require the same canonical Text3 widget");
    require(&services.widgets.environment().models.nodes == &services.parenting.nodes &&
        &services.widgets.environment().models.retained_owners == &services.geometry.actual_owners() &&
        &services.materials.retained_owners == &services.geometry.actual_owners(),
        "Text section services must use the same native lifetime/resource domains");
    if (!shadow) {
        services.widgets.create_auxiliary_model_00ab8530_fragment(shadow, "Shadow");
        auto& shadow_model = actual_model(shadow, services); // Reload after name release.
        shadow_model.storage.node.auxiliary_flags_138 &= ~std::uint32_t{3};
        auto* mesh = services.geometry.create_mesh();
        const auto sentinel_bits = services.widgets.environment().models.constants.unchanged_00d7a260;
        float sentinel;
        std::memcpy(&sentinel, &sentinel_bits, sizeof(sentinel));
        set_native_model_geometry_00b75170(actual_model(shadow, services), 0,
            mesh, sentinel, sentinel);
        append_default_section(*mesh, services);
        release_native_render_actual_owner(services.geometry.actual_owners(), mesh);
    }
    if (!widget.node_binding()) return;
    if (gui_model_has_geometry_00b74650(actual_model(widget.node_binding(), services))) {
        auto& model = actual_model(widget.node_binding(), services); // Native reload+4C.
        auto* mesh = static_cast<NativeMeshStorage*>(gui_model_geometry_00b74640(model.storage.model, 0));
        if (gui_mesh_element_count_00b72b40(mesh) == 0) append_default_section(*mesh, services);
    }
    auto* requested_parent = text.shadowed ? widget.node_binding() : nullptr;
    auto& child = actual_model(shadow, services).node.transform;
    set_native_node_parent_00b6e680(services.parenting, child,
        requested_parent ? &requested_parent->transform : nullptr);
    if (!text.shadowed)
        propagate_native_node_root_00b6d890(services.parenting.nodes,
            actual_model(shadow, services).node.transform, nullptr);
}
} // namespace bsp
