#include "bsp/gui_text_style.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
void require_text(GuiTextStyleBinding& binding) {
    auto& owner = binding.widget;
    require(owner.layout().type == GuiWidgetType::Text && owner.layout().transform.type_id == 3,
        "Text style requires the same canonical type3 GUI widget.");
    require(&binding.materials.widgets.owner(owner.layout()) == &owner,
        "Text style received a widget from a different owner runtime.");
}
void copy_word(float& destination, const float& source) noexcept {
    // A native DWORD load precedes its store, including identical/overlapping
    // input fields. Do not pass overlapping ranges directly to memcpy.
    std::uint32_t bits;
    std::memcpy(&bits, &source, sizeof(bits));
    std::memcpy(&destination, &bits, sizeof(bits));
}
void copy_color(GuiTextColor& destination, const float (&source)[4]) noexcept {
    copy_word(destination.r, source[0]); copy_word(destination.g, source[1]);
    copy_word(destination.b, source[2]); copy_word(destination.a, source[3]);
}
template<class T> T word(const void* source, std::size_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(source) + offset, sizeof(value));
    return value;
}
NativeModelOwner& shadow_model(GuiTextStyleBinding& binding) {
    static_assert(sizeof(void*) == 4, "Text native material access requires Win32");
    auto* node = binding.shadow_188;
    require(node != nullptr, "Text operation requires its actual shadow node+188.");
    // Actual storage key -> existing lifetime association, never a cast from
    // a NativeNodeBinding wrapper pointer to a model-owner wrapper pointer.
    auto* lifetime = binding.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&node->storage));
    auto* reference = dynamic_cast<NativeModelReference*>(lifetime);
    require(reference != nullptr, "Text shadow requires its canonical model reference.");
    auto& model = reference->model_owner();
    require(&model.node == node && &model.storage.node == &node->storage &&
        &model.environment.nodes == &binding.parenting.nodes &&
        &model.environment.retained_owners == &binding.materials.actual_owners &&
        model.phase == NativeModelOwner::Phase::live && reference->reference_count.load() > 0,
        "Text shadow must retain the same live model storage and service domains.");
    return model;
}
NativeMaterialStorage& section_material(GuiTextStyleBinding& binding, const void* element) {
    require(element != nullptr, "Text shadow geometry requires actual element0.");
    void* raw = word<void*>(element, 0x20);
    require(raw != nullptr, "Text shadow section requires an actual material.");
    auto& actual = binding.materials.actual_owners.resolve_actual(raw);
    auto* material = dynamic_cast<NativeMaterialReference*>(&actual);
    require(material && &material->storage() == raw &&
        &actual.reference_count == &material->storage().references_04 &&
        material->storage().vtable_00 == 0x00d5e520 && actual.reference_count.load() > 0,
        "Text shadow material must be the same live native material storage/reference.");
    return material->storage();
}
float multiplied_alpha(const float& shadow, const float& incoming) noexcept {
    const auto* left = &shadow;
    const auto* right = &incoming;
    float result;
    __asm {
        mov eax, left
        mov edx, right
        fld dword ptr [eax]
        fmul dword ptr [edx]
        fstp dword ptr [result]
    }
    return result;
}
void brighten(float (&color)[4]) noexcept {
    const double rgb_delta = 0.2823529541492462158203125; //00D5C5D0 exact bits.
    const double alpha_delta = 0.0; //00D7A258.
    auto* value = color;
    __asm {
        mov eax, value
        fld dword ptr [eax]
        fld qword ptr [rgb_delta]
        fadd st(1), st(0)
        fxch st(1)
        fstp dword ptr [eax]
        fld dword ptr [eax + 4]
        fadd st(0), st(1)
        fstp dword ptr [eax + 4]
        fadd dword ptr [eax + 8]
        fstp dword ptr [eax + 8]
        fld dword ptr [eax + 12]
        fadd qword ptr [alpha_delta]
        fstp dword ptr [eax + 12]
    }
}
}

void set_gui_text_color50_00ab6b50(GuiTextStyleBinding& binding, const float (&rgba)[4]) {
    require_text(binding);
    // Validate the base model before mirroring either existing base projection.
    auto* reference = binding.widget.model_reference();
    auto* node = binding.widget.node_binding();
    require(reference && node && &reference->model_owner().node == node &&
        reference->model_owner().phase == NativeModelOwner::Phase::live &&
        &reference->model_owner().environment.retained_owners == &binding.materials.actual_owners,
        "Text base color requires its canonical live model and material domain.");
    copy_color(binding.text.color, rgba);
    set_gui_color_00aa6870(binding.widget, rgba, binding.materials); //00AB6B59.
    if (!binding.shadow_188) return; //Reload after the base call.
    if (!gui_model_has_geometry_00b74650(shadow_model(binding))) return;
    void* geometry = gui_model_geometry_00b74640(shadow_model(binding).storage.model, 0);
    if (gui_mesh_element_count_00b72b40(geometry) == 0) return;
    geometry = gui_model_geometry_00b74640(shadow_model(binding).storage.model, 0);
    void* element = gui_geometry_element_00b732c0(geometry, 0);
    const float alpha = multiplied_alpha(binding.text.shadow_color.a, rgba[3]);
    float* destination = native_material_diffuse_00b179f0(section_material(binding, element), 0) + 3;
    __asm {
        mov eax, destination
        fld dword ptr [alpha]
        fstp dword ptr [eax]
    }
}

void configure_gui_text_shadow_00ab6c30(GuiTextStyleBinding& binding, std::uint8_t enabled,
    GuiTextShadowPos position, float offset, const GuiTextColor& color) {
    require_text(binding);
    auto& text = binding.text;
    text.shadow_pos = position;
    text.shadowed = enabled;
    copy_word(text.shadow_offset, offset);
    copy_word(text.shadow_color.r, color.r); copy_word(text.shadow_color.g, color.g);
    copy_word(text.shadow_color.b, color.b); copy_word(text.shadow_color.a, color.a);
    auto* parent = enabled != 0 ? binding.widget.node_binding() : nullptr;
    auto& shadow = shadow_model(binding);
    set_native_node_parent_00b6e680(binding.parenting, shadow.node.transform,
        parent ? &parent->transform : nullptr);
    // Parenting may reenter and replace the slot or enable byte.
    if (text.shadowed == 0)
        propagate_native_node_root_00b6d890(binding.parenting.nodes,
            shadow_model(binding).node.transform, nullptr);
}

void set_gui_text_state80_00ab7200(GuiTextStyleBinding& binding, std::int32_t state) {
    require_text(binding);
    const GuiTextColor* selected = &binding.text.state_colors.disabled;
    if (!binding.widget.scene_flags().hidden) {
        switch (state) {
        case 0: selected = &binding.text.state_colors.normal; break;
        case 1: selected = &binding.text.state_colors.focus; break;
        case 2: selected = &binding.text.state_colors.selected; break;
        case 3: break;
        default: throw std::out_of_range("Text state80 index is outside the four color records.");
        }
    }
    float color[4];
    copy_word(color[0], selected->r); copy_word(color[1], selected->g);
    copy_word(color[2], selected->b); copy_word(color[3], selected->a);
    if (state == 1) brighten(color);
    set_gui_text_color50_00ab6b50(binding, color);
}
} // namespace bsp
