#include "bsp/gui_text_sections.hpp"
#include "bsp/gui_text_buffers.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_material_binding.hpp"
#include <stdexcept>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Tracked Text section operation requires MSVC Win32.
#endif

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
NativeModelOwner& actual_model(NativeNodeBinding* node, GuiTextBufferServices& services) {
    require(node != nullptr, "Text sections require an actual Model node");
    auto* lifetime = services.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node->storage));
    auto* reference = dynamic_cast<NativeModelReference*>(lifetime);
    require(reference != nullptr, "Text sections require the canonical Model reference");
    auto& model = reference->model_owner();
    require(&model.node == node && &model.storage.node == &node->storage &&
        &model.environment.nodes == &services.parenting.nodes &&
        &model.environment.retained_owners == &services.geometry.actual_owners() &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text Model storage and native lifetime/resource domains must agree");
    return model;
}
// AB8609..AB8620 loads the one current DWORD into x87, captures live +188,
// spills17C, disarms raw-mesh cleanup, then spills178. Preserve signaling NaN
// handling and the receiver capture between FLD and the argument stores.
__declspec(noinline) NativeNodeBinding* capture_geometry_arguments(
    const volatile std::uint32_t& constant, NativeNodeBinding*& shadow,
    float& scalar_178, float& scalar_17c, std::int32_t& state) noexcept {
    const volatile void* input = &constant;
    auto* shadow_slot = &shadow;
    auto* first = &scalar_178;
    auto* second = &scalar_17c;
    auto* state_slot = &state;
    NativeNodeBinding* result;
    __asm {
        mov eax, input
        fld dword ptr [eax]
        mov edx, shadow_slot
        mov edx, dword ptr [edx]
        mov result, edx
        mov eax, second
        fst dword ptr [eax]
        mov ecx, state_slot
        mov dword ptr [ecx], -1
        mov eax, first
        fstp dword ptr [eax]
    }
    return result;
}
template<class Resource>
void release_creator(Resource*& creator, std::uint32_t site,
    NativeRenderActualOwners& owners, GuiTextSectionOperation& frame) {
    frame.native_site = site;
    frame.last_release_identity = creator;
    frame.release_phase = GuiTextSectionReleasePhase::entered;
    auto* const consumed = std::exchange(creator, nullptr);
    // This helper decrements actual+04 before a potentially throwing zero
    // lookup. A thrown lookup has consumed the reference; do not restore it.
    release_native_render_actual_owner(owners, consumed);
    frame.release_phase = GuiTextSectionReleasePhase::returned;
}
void append_default_section(NativeMeshStorage& mesh, GuiTextBufferServices& services,
    GuiTextSectionOperation& frame, bool shadow_branch) {
    auto& owners = services.geometry.actual_owners();
    frame.section_appended = false;
    frame.material_published = false;
    frame.native_site = shadow_branch ? 0x00ab862b : 0x00ab86fe;
    frame.factory_in_flight = GuiTextSectionFactory::section;
    frame.section = services.geometry.create_section();
    frame.factory_in_flight = GuiTextSectionFactory::none;
    frame.native_site = shadow_branch ? 0x00ab8635 : 0x00ab8708;
    append_native_mesh_draw_section_00b73c60(mesh, frame.section);
    frame.section_appended = true;
    frame.native_site = shadow_branch ? 0x00ab8643 : 0x00ab8716;
    frame.name_constructing = true;
    frame.temporary_name.assign_0041e870(services.strings, "guidefault.mshd");
    frame.name_constructing = false;
    frame.name_live = true;
    frame.name_cleanup_armed = true;
    frame.native_unwind_state = shadow_branch ? 4 : 5;
    frame.native_site = shadow_branch ? 0x00ab8654 : 0x00ab8727;
    frame.factory_in_flight = GuiTextSectionFactory::material;
    frame.material = services.geometry.create_material_for_effect_00535320(frame.temporary_name,
        services.current_renderer_00f8d394, services.materials, services.material_vtable_00d5e520);
    frame.factory_in_flight = GuiTextSectionFactory::none;
    frame.native_unwind_state = -1;
    frame.name_cleanup_armed = false;
    frame.name_live = false;
    frame.native_site = shadow_branch ? 0x00ab867d : 0x00ab8750;
    destroy_native_string_header_0041dd20(&frame.temporary_name, services.strings);
    frame.native_site = shadow_branch ? 0x00ab8685 : 0x00ab8758;
    set_native_mesh_section_material_00b864c0(*frame.section, owners, frame.material);
    frame.material_published = true;
    release_creator(frame.section, shadow_branch ? 0x00ab868e : 0x00ab8767, owners, frame);
    release_creator(frame.material, shadow_branch ? 0x00ab86aa : 0x00ab8779, owners, frame);
}
} // namespace

void ensure_gui_text_draw_sections_00ab8530(GuiWidgetOwner& widget, GuiTextWidget& text,
    NativeNodeBinding*& shadow, GuiTextBufferServices& services, GuiTextSectionOperation& frame) {
    require(!frame.has_incomplete(), "failed or active Text section operation cannot replay");
    require(!frame.mesh && !frame.section && !frame.material && !frame.model_creator &&
        !frame.constructed_model_owner && !frame.unconstructed_model_slot &&
        !frame.name_live && !frame.name_cleanup_armed,
        "Text section operation still owns acquired identities or a temporary name");
    require(!frame.widget_ || (frame.widget_ == &widget && frame.text_ == &text &&
        frame.shadow_ == &shadow && frame.services_ == &services),
        "Text section frame must stay on the same lifetime and services");
    require(widget.layout().type == GuiWidgetType::Text && widget.layout().transform.type_id == 3 &&
        &services.widgets.owner(widget.layout()) == &widget,
        "Text sections require the same canonical Text3 widget");
    auto* const lifetime = widget.text_lifetime();
    require(lifetime && &lifetime->text() == &text && &lifetime->shadow_slot_188() == &shadow,
        "Text sections must borrow the same canonical lifetime fields");
    require(&services.widgets.environment().models.nodes == &services.parenting.nodes &&
        &services.widgets.environment().models.retained_owners == &services.geometry.actual_owners() &&
        &services.materials.retained_owners == &services.geometry.actual_owners(),
        "Text section services must use the same native lifetime/resource domains");
    frame.widget_ = &widget;
    frame.text_ = &text;
    frame.shadow_ = &shadow;
    frame.services_ = &services;
    frame.temporary_name_storage = &services.strings;
    frame.phase = GuiTextSectionPhase::running;
    frame.native_site = 0x00ab8553;
    frame.failure_site = frame.cleanup_site = 0;
    frame.native_unwind_state = -1;
    frame.name_constructing = false;
    frame.shadow_branch = false;
    frame.section_appended = frame.material_published = false;
    frame.last_release_identity = nullptr;
    frame.release_phase = GuiTextSectionReleasePhase::not_called;
    frame.model_phase = GuiTextSectionModelPhase::not_started;
    frame.model_failure_phase = GuiTextSectionModelPhase::not_started;
    frame.model_metadata_failure = false;
    try {
        if (!shadow) {
            frame.shadow_branch = true;
            services.widgets.create_auxiliary_model_00ab8530_fragment(shadow, services.strings, frame);
            frame.native_site = 0x00ab85db;
            actual_model(shadow, services).storage.node.auxiliary_flags_138 &= ~std::uint32_t{3};
            frame.native_site = 0x00ab85e7;
            frame.factory_in_flight = GuiTextSectionFactory::mesh;
            frame.mesh = services.geometry.create_mesh();
            frame.factory_in_flight = GuiTextSectionFactory::none;
            frame.native_unwind_state = 3;
            frame.native_site = 0x00ab8609;
            float scalar_178, scalar_17c;
            auto* const receiver = capture_geometry_arguments(
                services.widgets.environment().models.constants.unchanged_00d7a260,
                shadow, scalar_178, scalar_17c, frame.native_unwind_state);
            frame.native_site = 0x00ab8626;
            set_native_model_geometry_00b75170(actual_model(receiver, services), 0,
                frame.mesh, scalar_178, scalar_17c);
            append_default_section(*frame.mesh, services, frame, true);
            release_creator(frame.mesh, 0x00ab86bd, services.geometry.actual_owners(), frame);
        }
        frame.native_site = 0x00ab86cb;
        auto* const initial_main = widget.node_binding();
        if (initial_main) {
            frame.native_site = 0x00ab86d6;
            if (gui_model_has_geometry_00b74650(actual_model(initial_main, services))) {
                frame.native_site = 0x00ab86e8;
                auto& main = actual_model(widget.node_binding(), services); // Fresh +4C after current call.
                auto* const current_mesh = static_cast<NativeMeshStorage*>(
                    gui_model_geometry_00b74640(main.storage.model, 0));
                frame.native_site = 0x00ab86f1;
                if (gui_mesh_element_count_00b72b40(current_mesh) == 0)
                    append_default_section(*current_mesh, services, frame, false);
            }
            frame.native_site = 0x00ab8787;
            auto* const requested_parent = text.shadowed ? widget.node_binding() : nullptr;
            auto* const current_shadow = shadow;
            frame.native_site = 0x00ab879e;
            set_native_node_parent_00b6e680(services.parenting,
                actual_model(current_shadow, services).node.transform,
                requested_parent ? &requested_parent->transform : nullptr);
            frame.native_site = 0x00ab87a3;
            if (!text.shadowed) {
                auto* const fresh_shadow = shadow;
                frame.native_site = 0x00ab87b4;
                propagate_native_node_root_00b6d890(services.parenting.nodes,
                    actual_model(fresh_shadow, services).node.transform, nullptr);
            }
        }
        frame.native_site = 0x00ab87cb;
        frame.phase = GuiTextSectionPhase::complete;
    } catch (...) {
        if (!frame.failure_site) frame.failure_site = frame.native_site;
        // FH3 states4/5 own only the currently constructed effect name.
        // Model states0/1 are handled by the tracked canonical owner producer.
        // Mesh state3 is inside create_mesh's existing provider boundary.
        if ((frame.native_unwind_state == 4 || frame.native_unwind_state == 5) &&
            frame.name_cleanup_armed) {
            frame.cleanup_site = frame.native_unwind_state == 4 ? 0x00cb7f4c : 0x00cb7f54;
            frame.name_cleanup_armed = false;
            frame.name_live = false;
            frame.native_unwind_state = -1;
            destroy_native_string_header_0041dd20(&frame.temporary_name, services.strings);
        }
        frame.phase = GuiTextSectionPhase::failed;
        throw;
    }
}
} // namespace bsp
