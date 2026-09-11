#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_material_binding.hpp"

#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}

// Original numeric table entries are evidence, not callable host code. The
// caller binds the live table to real native-ABI factory implementations.
template<class Function> Function current_slot(void* renderer, std::size_t offset) {
    const auto* table = *static_cast<const std::uintptr_t* const*>(renderer);
    return reinterpret_cast<Function>(table[offset / 4]);
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

void create_gui_text_glyph_buffers_00ab8400(std::uint32_t capacity,
    NativeMeshStorage& mesh, void* const volatile& renderer_slot,
    NativeStringStorage& strings, NativeRenderActualOwners& owners) {
    NativeString format;
    format.resize_0041dd40(strings, 0x10, true);
    if (format.data()) std::memcpy(format.data(), "simplecolor.mvfm", format.length() + 1u);
    using Declaration = void* (__thiscall*)(void*, NativeString*);
    using Vertex = void* (__thiscall*)(void*, std::uint32_t, std::uint32_t, void*);
    using Index = void* (__thiscall*)(void*, std::uint32_t, std::uint32_t, std::uint32_t);
    void* declaration;
    try {
        void* renderer = renderer_slot;
        declaration = current_slot<Declaration>(renderer, 0x38)(renderer, &format);
    } catch (...) {
        destroy_native_string_header_0041dd20(&format, strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&format, strings);
    void* renderer = renderer_slot;
    void* vertex = current_slot<Vertex>(renderer, 0x5c)(renderer, capacity * 4u, 1u, declaration);
    set_native_mesh_vertex_stream_00b73bb0(mesh, owners, 0, vertex);
    release_native_render_actual_owner(owners, vertex);
    release_native_render_actual_owner(owners, declaration);
    renderer = renderer_slot;
    void* index = current_slot<Index>(renderer, 0x60)(renderer, capacity * 6u, 1u, 0x65u);
    set_native_mesh_index_stream_00b73b70(mesh, owners, index);
    release_native_render_actual_owner(owners, index);
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
