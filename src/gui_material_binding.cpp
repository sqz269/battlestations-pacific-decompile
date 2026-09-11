#include "bsp/gui_material_binding.hpp"
#include <cstring>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
template<class T> T read_word(const void* base, std::size_t offset) noexcept {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(base) + offset, sizeof(result));
    return result;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void copy_color_words(float* destination, const float* source) noexcept {
    for (std::size_t i = 0; i != 4; ++i) {
        const auto word = read_word<std::uint32_t>(source, i * sizeof(float));
        std::memcpy(destination + i, &word, sizeof(word));
    }
}
void register_parameter(MaterialCloneState& material, const char* name,
    const float* words, std::uint32_t count) {
    std::string error;
    const auto result = material.parameters.register_words_00b17e10_00b44d60(
        name, {words, count, count, false}, error);
    if (result.status == MaterialParameterRegistrationStatus::unsupported)
        throw std::runtime_error(std::string("GUI clip parameter ") + name + ": " + error);
    // no_match is a legitimate native effect result, including a null shader.
}
void require_base_color_slot(const GuiLayoutWidget& widget) {
    switch (widget.type) {
    case GuiWidgetType::Screen:
    case GuiWidgetType::Group:
    case GuiWidgetType::Icon:
    case GuiWidgetType::FrameBox:
        require(widget.transform.type_id == static_cast<std::int32_t>(widget.type),
            "GUI material binding requires coherent same-owner type fields");
        return;
    default:
        throw std::logic_error("GUI current color virtual50 is not verified for this type");
    }
}
} // namespace

void set_material_parameter_owner_00b18a40(MaterialCloneState& material,
    const void* incoming, std::uint8_t retain_byte, const MaterialParameterOwnerAcquire& acquire) {
    if (material.byte10d && material.pointer0c) {
        require(material.owner0c.get() == material.pointer0c && material.owner0c.use_count() != 0,
            "Material retained parameter source lacks its actual owner");
        material.owner0c.reset();
        material.pointer0c = nullptr;
    } else {
        require(material.owner0c.use_count() == 0,
            "Material borrowed parameter source unexpectedly owns a token");
    }
    material.pointer0c = incoming;
    material.byte10d = retain_byte;
    if (retain_byte && incoming) {
        require(bool(acquire), "Material parameter owner requires actual lifetime acquisition");
        auto retained = acquire(incoming);
        require(retained.get() == incoming && retained.use_count() != 0,
            "Material lifetime token must retain the same actual parameter source");
        material.owner0c = std::move(retained);
    }
}

void register_gui_clip_parameters_00aa9f10(GuiWidgetOwner& widget,
    MaterialCloneState& material, const GuiMaterialBindingServices& services) {
    GuiLayoutWidget* ancestor = &widget.layout();
    while (ancestor && ancestor->transform.type_id != 16) ancestor = ancestor->parent;
    float& enabled = widget.extra_fields().clip_enabled_e8;
    enabled = ancestor ? 1.0f : 0.0f;
    register_parameter(material, "cClip", &enabled, 1);
    if (enabled != 0.0f) {
        require(ancestor && services.clip_box_sources,
            "Active GUI clipping needs actual type16 ancestor fields");
        const auto sources = services.clip_box_sources(services.widgets.owner(*ancestor));
        register_parameter(material, "cClipCenter", sources.center_ec, 2);
        register_parameter(material, "cClipBorder", sources.border_f4, 4);
        register_parameter(material, "cAspectRatio", &services.aspect_ratio_00e12fc0, 1);
    }
}

bool gui_model_has_geometry_00b74650(const NativeNodeBinding& node) noexcept {
    static_assert(sizeof(void*) == 4, "GUI native material access requires Win32");
    return read_word<void*>(&node.storage, 0x180) != nullptr;
}
std::uint32_t gui_mesh_element_count_00b72b40(const void* mesh) noexcept {
    return read_word<std::uint32_t>(mesh, 0x58);
}

void set_gui_color_00aa6870(GuiWidgetOwner& widget, const float (&rgba)[4],
    const GuiMaterialBindingServices& services) {
    // Native scalar DWORD copy order also handles the common self-source call.
    copy_color_words(widget.layout().color, rgba);
    std::memcpy(&widget.layout().transform.alpha, &widget.layout().color[3], sizeof(float));
    const auto* node = widget.node_binding();
    require(node != nullptr, "GUI color publication needs the same bound native node");
    if (!gui_model_has_geometry_00b74650(*node)) return;
    void* geometry = read_word<void*>(&node->storage, 0x180);
    if (gui_mesh_element_count_00b72b40(geometry) == 0) return;
    // Native reloads model geometry before resolving element zero.
    geometry = read_word<void*>(&node->storage, 0x180);
    const void* element = gui_geometry_element_00b732c0(geometry, 0);
    require(element != nullptr, "GUI material publication needs actual geometry element0");
    const void* raw_material = read_word<void*>(element, 0x20);
    require(raw_material && services.material,
        "GUI color publication needs the canonical native material owner");
    auto& material = services.material(raw_material);
    float* diffuse = material.lighting.diffuse_color_00b179f0(0);
    copy_color_words(diffuse, rgba);
}

void bind_gui_material_callbacks(GuiGeometryRuntimeServices& geometry,
    GuiWidgetOwner& widget, GuiMaterialBindingServices services) {
    require_base_color_slot(widget.layout());
    require(services.clip_box_sources &&
        services.retain_widget && services.material,
        "GUI material callbacks require real clip, widget lifetime and material associations");
    require(&services.widgets.owner(widget.layout()) == &widget,
        "GUI material callbacks must use the existing canonical widget owner");
    geometry.register_clip_and_owner = [&widget, services](MaterialCloneState& material) {
        register_gui_clip_parameters_00aa9f10(widget, material, services);
        set_material_parameter_owner_00b18a40(material, &widget, 1,
            [&widget, &services](const void* same_owner) {
                require(same_owner == &widget, "GUI material owner identity changed");
                return services.retain_widget(widget);
            });
    };
    geometry.publish_color_virtual50 = [&widget, services] {
        require_base_color_slot(widget.layout());
        set_gui_color_00aa6870(widget, widget.layout().color, services);
    };
}
} // namespace bsp
