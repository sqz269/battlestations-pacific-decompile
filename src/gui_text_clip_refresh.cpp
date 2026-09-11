#include "bsp/gui_text_clip_refresh.hpp"
#include "bsp/native_mesh_owner.hpp"
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_services(GuiTextContentBinding& binding, GuiTextClipRefreshServices& s) {
    auto& b = s.buffers;
    require(binding.widget.layout().type == GuiWidgetType::Text &&
        binding.widget.layout().transform.type_id == 3 &&
        &b.widgets.owner(binding.widget.layout()) == &binding.widget,
        "Text clip refresh requires its canonical type3 owner");
    require(&b.geometry.actual_owners() == &b.materials.retained_owners &&
        &b.widgets.environment().models.retained_owners == &b.materials.retained_owners &&
        &s.parameters.parameter_slots == &b.materials.parameter_slots &&
        &s.parameters.parameter_names == &b.materials.parameter_names &&
        &b.strings == &s.parameters.parameter_names,
        "Text clip refresh requires the same actual material and string domains");
}
NativeModelOwner& captured_model(NativeNodeBinding& node, GuiTextBufferServices& b) {
    static_assert(sizeof(void*) == 4, "Text actual clip refresh requires Win32");
    auto* actual = b.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node.storage));
    auto* reference = dynamic_cast<NativeModelReference*>(actual);
    require(reference != nullptr, "Text clip drawable requires its actual Model reference");
    auto& model = reference->model_owner();
    require(&model.node == &node && &model.storage.node == &node.storage &&
        &model.environment.nodes == &b.parenting.nodes &&
        &model.environment.retained_owners == &b.materials.retained_owners &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text clip drawable requires the same captured live Model storage");
    return model;
}
NativeMeshStorage& actual_mesh(void* raw, NativeRenderActualOwners& owners) {
    require(raw != nullptr, "Text clip mesh is null after the native geometry gate");
    auto* reference = dynamic_cast<NativeMeshReference*>(&owners.resolve_actual(raw));
    require(reference && &reference->storage() == raw && reference->reference_count.load() > 0,
        "Text clip mesh requires its canonical actual owner");
    require(reference->storage().draw_sections_54.count_04 >= 0,
        "Text clip mesh has an invalid negative section count");
    return reference->storage();
}
void refresh_drawable(GuiWidgetOwner& widget, NativeNodeBinding* captured,
    GuiTextClipRefreshServices& s) {
    if (!captured) return;
    auto& model = captured_model(*captured, s.buffers);
    if (!gui_model_has_geometry_00b74650(model)) return;
    auto& owners = s.buffers.materials.retained_owners;
    void* raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    actual_mesh(raw_mesh, owners);
    if (!gui_mesh_element_count_00b72b40(raw_mesh)) return;
    //00AB7A71/7AB7 reload geometry on the SAME captured Model, before index0.
    raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    auto& mesh = actual_mesh(raw_mesh, owners);
    require(mesh.draw_sections_54.data_00 != nullptr && mesh.draw_sections_54.count_04 > 0,
        "Text clip mesh requires actual section0 storage");
    void* raw_section = gui_geometry_element_00b732c0(raw_mesh, 0);
    require(raw_section != nullptr, "Text clip section0 is null");
    auto* section = dynamic_cast<NativeMeshSectionReference*>(&owners.resolve_actual(raw_section));
    require(section && &section->storage() == raw_section && section->reference_count.load() > 0,
        "Text clip section requires its canonical actual owner");
    void* raw_material = section->storage().material_20;
    require(raw_material != nullptr, "Text clip section material is null");
    auto* material = dynamic_cast<NativeMaterialReference*>(&owners.resolve_actual(raw_material));
    require(material && &material->storage() == raw_material && material->reference_count.load() > 0 &&
        material->storage().vtable_00 == 0x00d5e520u,
        "Text clip material requires its actual supported live profile");
    register_native_gui_clip_parameters_00aa9f10(widget, material->storage(),
        s.buffers.widgets, s.aspect_ratio_00e12fc0, s.parameters);
}
std::optional<GuiTextClipRefreshContinuation> select_child(
    GuiWidgetOwner& parent, GuiWidgetOwnerRuntime& widgets, std::size_t index) {
    auto& children = parent.layout().transform.children;
    require(index <= children.size(), "Text clip traversal lost its current native list entry");
    if (index == children.size()) return std::nullopt;
    auto* child = children[index];
    require(child != nullptr, "Text clip traversal found a null child payload");
    widgets.owner(*child); // Require the existing owner; do not create a companion.
    return GuiTextClipRefreshContinuation{parent, widgets, index, child};
}
} // namespace

GuiWidgetOwner& GuiTextClipRefreshContinuation::child_owner() const {
    const auto& children = parent.layout().transform.children;
    require(pending_child && child_index < children.size() &&
        children[child_index] == pending_child,
        "Text clip continuation is consumed or its current list entry changed");
    return widgets.owner(*pending_child);
}

std::optional<GuiTextClipRefreshContinuation>
begin_gui_text_clip_refresh_00ab7a40(GuiTextLifetime& lifetime, GuiTextClipRefreshServices& s) {
    auto binding = lifetime.content_binding();
    require_services(binding, s);
    refresh_drawable(binding.widget, binding.widget.node_binding(), s); //7A45 capture.
    //7A88 loads the LIVE shadow only AFTER main registration/name cleanup.
    refresh_drawable(binding.widget, binding.shadow_188, s);
    //7ACE starts the live borrowed GUI list after both material callbacks.
    return select_child(binding.widget, s.buffers.widgets, 0);
}

std::optional<GuiTextClipRefreshContinuation>
resume_gui_text_clip_refresh_after_child70_00ab7a40(GuiTextClipRefreshContinuation& frame) {
    frame.child_owner(); //7AFA: current-entry check AFTER the actual child callback.
    const auto next = frame.child_index + 1u;
    frame.pending_child = nullptr; // This borrowed frame cannot be resumed twice.
    //7B04/7AD8/7AF0: advance/reload only after the child completed, no cached owner/vtable.
    return select_child(frame.parent, frame.widgets, next);
}
} // namespace bsp
