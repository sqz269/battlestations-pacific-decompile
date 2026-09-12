#include "bsp/gui_widget_clip_refresh.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_section.hpp"
#include <exception>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_services(GuiWidgetOwner& widget, GuiWidgetClipRefreshServices& s) {
    require(gui_widget_uses_base_clip70_profile(widget.layout().type) &&
        static_cast<std::int32_t>(widget.layout().type) == widget.layout().transform.type_id &&
        &s.widgets.owner(widget.layout()) == &widget,
        "base GUI clip refresh requires its supported canonical current70 profile");
    require(&s.widgets.environment().models.retained_owners == &s.materials.retained_owners &&
        &s.parameters.parameter_slots == &s.materials.parameter_slots &&
        &s.parameters.parameter_names == &s.materials.parameter_names,
        "base GUI clip refresh requires the same actual material and string domains");
}
NativeModelOwner& captured_model(NativeNodeBinding& node, GuiWidgetClipRefreshServices& s) {
    static_assert(sizeof(void*) == 4, "base actual clip refresh requires Win32");
    auto& environment = s.widgets.environment().models;
    auto* actual = environment.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node.storage));
    auto* reference = dynamic_cast<NativeModelReference*>(actual);
    require(reference != nullptr, "base GUI clip drawable requires its actual Model reference");
    auto& model = reference->model_owner();
    require(&model.node == &node && &model.storage.node == &node.storage &&
        &model.environment == &environment && model.phase == NativeModelOwner::Phase::live &&
        reference->reference_count.load() > 0,
        "base GUI clip drawable requires the same captured live Model storage");
    return model;
}
NativeMeshStorage& actual_mesh(void* raw, NativeRenderActualOwners& owners) {
    require(raw != nullptr, "base GUI clip mesh is null after its native geometry gate");
    auto* reference = dynamic_cast<NativeMeshReference*>(&owners.resolve_actual(raw));
    require(reference && &reference->storage() == raw && reference->reference_count.load() > 0,
        "base GUI clip mesh requires its canonical actual owner");
    require(reference->storage().draw_sections_54.count_04 >= 0,
        "base GUI clip mesh has an invalid negative section count");
    return reference->storage();
}
void refresh_primary(GuiWidgetOwner& widget, GuiWidgetClipRefreshServices& s) {
    //00AAA3E5 captures the primary once, before any material/name callback.
    auto* captured = widget.node_binding();
    if (!captured) return;
    auto& model = captured_model(*captured, s);
    if (!gui_model_has_geometry_00b74650(model)) return;
    auto& owners = s.materials.retained_owners;
    void* raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    actual_mesh(raw_mesh, owners);
    if (!gui_mesh_element_count_00b72b40(raw_mesh)) return;
    //00AAA411 reloads geometry on the SAME captured Model before section0.
    raw_mesh = gui_model_geometry_00b74640(model.storage.model, 0);
    auto& mesh = actual_mesh(raw_mesh, owners);
    require(mesh.draw_sections_54.data_00 != nullptr && mesh.draw_sections_54.count_04 > 0,
        "base GUI clip mesh requires actual section0 storage");
    void* raw_section = gui_geometry_element_00b732c0(raw_mesh, 0);
    require(raw_section != nullptr, "base GUI clip section0 is null");
    auto* section = dynamic_cast<NativeMeshSectionReference*>(&owners.resolve_actual(raw_section));
    require(section && &section->storage() == raw_section && section->reference_count.load() > 0,
        "base GUI clip section requires its canonical actual owner");
    void* raw_material = section->storage().material_20;
    require(raw_material != nullptr, "base GUI clip section material is null");
    auto* material = dynamic_cast<NativeMaterialReference*>(&owners.resolve_actual(raw_material));
    require(material && &material->storage() == raw_material && material->reference_count.load() > 0 &&
        material->storage().vtable_00 == 0x00d5e520u,
        "base GUI clip material requires its actual supported live profile");
    register_native_gui_clip_parameters_00aa9f10(widget, material->storage(),
        s.widgets, s.aspect_ratio_00e12fc0, s.parameters);
}
std::optional<GuiWidgetClipRefreshContinuation> select_child(
    GuiWidgetOwner& parent, GuiWidgetOwnerRuntime& widgets, std::size_t index) {
    auto& children = parent.layout().transform.children;
    require(index <= children.size(), "base GUI clip traversal lost its native list entry");
    if (index == children.size()) return std::nullopt;
    auto* child = children[index];
    require(child != nullptr, "base GUI clip traversal found a null child payload");
    widgets.owner(*child);
    return GuiWidgetClipRefreshContinuation{parent, widgets, index, child};
}
struct ActiveCall {
    bool& active;
    explicit ActiveCall(bool& value) noexcept : active(value) { active = true; }
    ~ActiveCall() { active = false; }
};
} // namespace

bool gui_widget_uses_base_clip70_profile(GuiWidgetType type) noexcept {
    switch (type) {
    case GuiWidgetType::Screen:
    case GuiWidgetType::Group:
    case GuiWidgetType::Icon:
    case GuiWidgetType::ClipBox:
    case GuiWidgetType::FrameBox:
        return true;
    default:
        return false;
    }
}
GuiWidgetOwner& GuiWidgetClipRefreshContinuation::child_owner() const {
    const auto& children = parent.layout().transform.children;
    require(pending_child && child_index < children.size() &&
        children[child_index] == pending_child,
        "base GUI clip continuation is consumed or its current list entry changed");
    return widgets.owner(*pending_child);
}
std::optional<GuiWidgetClipRefreshContinuation>
begin_gui_widget_clip_refresh_00aaa3e0(GuiWidgetOwner& widget, GuiWidgetClipRefreshServices& s) {
    require_services(widget, s);
    refresh_primary(widget, s);
    //00AAA428 reads the live child head AFTER material callbacks/name cleanup.
    return select_child(widget, s.widgets, 0);
}
std::optional<GuiWidgetClipRefreshContinuation>
resume_gui_widget_clip_refresh_after_child70_00aaa3e0(GuiWidgetClipRefreshContinuation& frame) {
    frame.child_owner(); //00AAA454 current-entry check after actual child70.
    const auto next = frame.child_index + 1u;
    frame.pending_child = nullptr;
    return select_child(frame.parent, frame.widgets, next);
}
GuiWidgetClipRefreshOperation::GuiWidgetClipRefreshOperation(
    GuiWidgetOwner& owner, GuiWidgetClipRefreshServices& services)
    : owner_(owner), services_(services) {
    require_services(owner_, services_);
}
GuiWidgetClipRefreshOperation::~GuiWidgetClipRefreshOperation() {
    if (has_pending()) std::terminate();
}
bool GuiWidgetClipRefreshOperation::has_pending() const noexcept {
    return active_ || failed_ || continuation_.has_value();
}
GuiWidgetOwner& GuiWidgetClipRefreshOperation::pending_child_owner() const {
    require(!failed_ && continuation_.has_value(),
        "base GUI clip operation has no resumable pending child70");
    return continuation_->child_owner();
}
void GuiWidgetClipRefreshOperation::begin() {
    require(!has_pending(), "base GUI clip operation is already running or awaiting child70");
    ActiveCall active(active_);
    try {
        auto pending = begin_gui_widget_clip_refresh_00aaa3e0(owner_, services_);
        if (pending) continuation_.emplace(*pending);
    } catch (...) {
        // No child frame records the partial material/name effects. Retain a
        // terminal guard rather than permit their replay through another begin.
        failed_ = true;
        throw;
    }
    continue_children();
}
void GuiWidgetClipRefreshOperation::advance_after_child() {
    try {
        auto next = resume_gui_widget_clip_refresh_after_child70_00aaa3e0(*continuation_);
        continuation_.reset();
        if (next) continuation_.emplace(*next);
    } catch (...) {
        // The old child already completed, and resume may have consumed its
        // frame. It must never become another pending child call or restart.
        failed_ = true;
        throw;
    }
}
void GuiWidgetClipRefreshOperation::continue_children() {
    while (continuation_) {
        // Resolve CURRENT child implementation at its actual call boundary.
        // Throw leaves this frame intact; never retry/advance automatically.
        GuiWidgetOwner* child;
        try {
            child = &continuation_->child_owner();
        } catch (...) {
            failed_ = true;
            throw;
        }
        child->refresh_clip70();
        advance_after_child();
    }
}
void GuiWidgetClipRefreshOperation::resume_after_child70() {
    require(!active_ && !failed_ && continuation_.has_value(),
        "base GUI clip resume requires an inactive pending child70 frame");
    ActiveCall active(active_);
    advance_after_child();
    continue_children();
}
} // namespace bsp
