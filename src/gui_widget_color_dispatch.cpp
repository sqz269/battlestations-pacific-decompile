#include "bsp/gui_widget_color_dispatch.hpp"
#include "bsp/gui_material_binding.hpp"
#include "bsp/native_mesh_owner.hpp"
#include "bsp/native_mesh_section.hpp"
#include "bsp/native_material_owner.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
std::uint32_t word(const float& source) noexcept {
    std::uint32_t result; std::memcpy(&result, &source, 4); return result;
}
void store(float& destination, std::uint32_t value) noexcept {
    std::memcpy(&destination, &value, 4);
}
NativeModelOwner& model(GuiWidgetOwner& widget) {
    auto* reference = widget.model_reference();
    if (!reference || reference->reference_count.load() <= 0 ||
        widget.node_binding() != &reference->model_owner().node ||
        reference->model_owner().phase != NativeModelOwner::Phase::live)
        throw std::logic_error("GUI color requires the same live actual Model profile");
    return reference->model_owner();
}
float* material_color(GuiWidgetOwner& widget) {
    auto& owner = model(widget);
    if (!gui_model_has_geometry_00b74650(owner)) return nullptr;
    auto& actual = owner.environment.retained_owners;
    auto* mesh = gui_model_geometry_00b74640(owner.storage.model, 0);
    auto* mesh_ref = dynamic_cast<NativeMeshReference*>(&actual.resolve_actual(mesh));
    if (!mesh_ref || &mesh_ref->storage() != mesh || mesh_ref->reference_count.load() <= 0)
        throw std::logic_error("GUI color requires actual current mesh ownership");
    if (gui_mesh_element_count_00b72b40(mesh) == 0) return nullptr;
    mesh = gui_model_geometry_00b74640(owner.storage.model, 0);
    auto* section = static_cast<NativeMeshSectionStorage*>(gui_geometry_element_00b732c0(mesh, 0));
    auto* section_ref = dynamic_cast<NativeMeshSectionReference*>(&actual.resolve_actual(section));
    if (!section_ref || &section_ref->storage() != section || section_ref->reference_count.load() <= 0)
        throw std::logic_error("GUI color requires actual current section0 ownership");
    auto* material = static_cast<NativeMaterialStorage*>(section->material_20);
    auto* material_ref = dynamic_cast<NativeMaterialReference*>(&actual.resolve_actual(material));
    if (!material_ref || &material_ref->storage() != material || material_ref->reference_count.load() <= 0)
        throw std::logic_error("GUI color requires actual current material ownership");
    return native_material_diffuse_00b179f0(*material, 0);
}
}
bool gui_widget_has_base_color54_profile(GuiWidgetType type) noexcept {
    return type == GuiWidgetType::Text || gui_widget_has_base_alpha4c_profile(type);
}
bool gui_widget_has_base_alpha4c_profile(GuiWidgetType type) noexcept {
    switch (type) {
    case GuiWidgetType::Group: case GuiWidgetType::Icon: case GuiWidgetType::Screen:
    case GuiWidgetType::ClipBox: case GuiWidgetType::FrameBox:
    case GuiWidgetType::Section: return true;
    default: return false;
    }
}
float* read_gui_widget_color_00aa68f0(GuiWidgetOwner& widget, float (&output)[4]) {
    if (auto* source = material_color(widget)) {
        store(output[0], word(source[0]));
        store(output[1], word(source[1]));
        const auto third = word(source[2]);
        const auto fourth = word(source[3]);
        store(output[2], third); store(output[3], fourth);
    } else {
        const auto first = word(widget.layout().color[0]);
        const auto second = word(widget.layout().color[1]);
        store(output[0], first);
        const auto third = word(widget.layout().color[2]);
        store(output[1], second);
        const auto fourth = word(widget.layout().color[3]);
        store(output[2], third); store(output[3], fourth);
    }
    return output;
}
void set_gui_widget_alpha_00aa6980(GuiWidgetOwner& widget, float alpha) {
    (void)model(widget); // host ownership validation before any native stores
    store(widget.layout().color[3], word(alpha));
    store(widget.layout().transform.alpha, word(alpha)); // same projected field
    if (auto* destination = material_color(widget)) store(destination[3], word(alpha));
}
} // namespace bsp
