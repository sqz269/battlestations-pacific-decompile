#include "bsp/gui_text_content.hpp"
#include "bsp/font_registry.hpp"
#include "bsp/gui_native_clip_parameters.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include "bsp/gui_text_material.hpp"
#include "bsp/gui_text_style.hpp"
#include "bsp/native_font_resources.hpp"
#include "bsp/native_mesh_owner.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>
#include <type_traits>

namespace bsp {
static_assert(std::is_same_v<decltype(GuiTextWidget::shadowed), std::uint8_t>,
    "Text content requires the canonical raw shadow-enable byte");
namespace {
void require_text_owner(GuiTextContentBinding& binding, GuiWidgetOwnerRuntime& owners) {
    if (binding.widget.layout().type != GuiWidgetType::Text ||
        binding.widget.layout().transform.type_id != 3 ||
        &owners.owner(binding.widget.layout()) != &binding.widget)
        throw std::logic_error("Text content requires its canonical type3 GUI owner");
}
void require_native_text_domain(std::u16string_view text) {
    if (text.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) ||
        text.find(u'\0') != std::u16string_view::npos)
        throw std::invalid_argument("Text content requires null-free native-length UTF16");
}
bool equal_content(const std::u16string& stored, const std::u16string& copied,
    GuiTextContentEnvironment& e) {
    //ABA921..ABA937 dispatches on wrapper lengths before calling the CRT.
    if (stored.empty()) return copied.empty();
    if (copied.empty()) return false;
    if (e.crt_locale_changed_0109de1c != 0)
        return e.calls.compare_current_locale_00c0392a(stored.c_str(), copied.c_str()) == 0;
    //C03A74..C03AB2: terminated comparison, ASCII A..Z only; no length check
    //and no substitute Unicode/country locale mapping in this native arm.
    const auto fold = [](char16_t c) {
        return c > u'@' && c < u'[' ? static_cast<char16_t>(c + 0x20) : c;
    };
    const char16_t* a = stored.c_str();
    const char16_t* b = copied.c_str();
    for (;;) {
        const char16_t left = fold(*a++);
        const char16_t right = fold(*b++);
        if (left != right) return false;
        if (!left) return true;
    }
}
struct ActualSection {
    NativeMeshStorage* mesh;
    NativeMeshSectionStorage* section;
};
ActualSection section_zero(NativeModelReference& model,
    NativeRenderActualOwners& actual_owners) {
    //B74640 returns the SAME actual model+180. Never overlay the semantic
    //FontGeometryOwner snapshot/GeneratedInstanceGeometry onto raw mesh bytes.
    void* raw_mesh = gui_model_geometry_00b74640(model.model_owner().storage.model, 0);
    if (!raw_mesh) throw std::logic_error("Text drawable has no actual mesh");
    auto* mesh_owner = dynamic_cast<NativeMeshReference*>(&actual_owners.resolve_actual(raw_mesh));
    if (!mesh_owner || &mesh_owner->storage() != raw_mesh || mesh_owner->reference_count.load() <= 0)
        throw std::logic_error("Text mesh has no matching canonical owner");
    auto& mesh = mesh_owner->storage();
    if (!mesh.draw_sections_54.data_00 || mesh.draw_sections_54.count_04 < 1)
        throw std::logic_error("Text mesh has no actual section0");
    void* raw_section = gui_geometry_element_00b732c0(raw_mesh, 0);
    if (!raw_section) throw std::logic_error("Text section0 is null");
    auto* section_owner = dynamic_cast<NativeMeshSectionReference*>(
        &actual_owners.resolve_actual(raw_section));
    if (!section_owner || &section_owner->storage() != raw_section ||
        section_owner->reference_count.load() <= 0)
        throw std::logic_error("Text section has no matching canonical owner");
    return {&mesh, &section_owner->storage()};
}
void require_nonempty_environment(GuiTextLifetime& text, GuiTextNonemptyEnvironment& e) {
    auto binding = text.content_binding();
    auto& buffers = e.shader.buffers;
    require_text_owner(binding, buffers.widgets);
    if (&e.style.widgets != &buffers.widgets ||
        &e.style.actual_owners != &buffers.materials.retained_owners ||
        &e.parameters.parameter_slots != &buffers.materials.parameter_slots ||
        &e.parameters.parameter_names != &buffers.materials.parameter_names ||
        &buffers.strings != &e.parameters.parameter_names)
        throw std::logic_error("Text content must share actual widget/material/string domains");
}
NativeModelReference& current_shadow(GuiTextContentBinding& binding,
    GuiTextBufferServices& buffers) {
    auto* shadow = binding.shadow_188;
    if (!shadow) throw std::logic_error("Text shadow drawable is missing");
    auto* lifetime = buffers.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&shadow->storage));
    auto* model = dynamic_cast<NativeModelReference*>(lifetime);
    if (!model || &model->model_owner().node != shadow ||
        &model->model_owner().storage.node != &shadow->storage ||
        &model->model_owner().environment.nodes != &buffers.parenting.nodes ||
        &model->model_owner().environment.retained_owners != &buffers.materials.retained_owners ||
        model->model_owner().phase != NativeModelOwner::Phase::live ||
        model->reference_count.load() <= 0)
        throw std::logic_error("Text shadow requires its same live actual Model owner");
    return *model;
}
NativeMaterialStorage& actual_material(void* raw, NativeRenderActualOwners& owners) {
    if (!raw) throw std::logic_error("Text section material is null");
    auto* material = dynamic_cast<NativeMaterialReference*>(&owners.resolve_actual(raw));
    if (!material || &material->storage() != raw || material->reference_count.load() <= 0 ||
        material->storage().vtable_00 != 0x00d5e520u)
        throw std::logic_error("Text material lacks its matching actual storage/reference");
    return material->storage();
}
void register_parameter(NativeMaterialStorage& material, const char* spelling,
    const void* source, std::uint32_t words, GuiTextNonemptyEnvironment& e) {
    NativeString name;
    name.assign_0041e870(e.shader.buffers.strings, spelling);
    try {
        switch (words) {
        case 1: register_native_material_float_00b18b20(material, &name, source, e.parameters); break;
        case 2: register_native_material_float2_00b18b00(material, &name, source, e.parameters); break;
        case 4: register_native_material_float4_00b18aa0(material, &name, source, e.parameters); break;
        default: throw std::logic_error("Text parameter has no recovered wrapper");
        }
    } catch (...) {
        destroy_native_string_header_0041dd20(&name, e.shader.buffers.strings);
        throw;
    }
    destroy_native_string_header_0041dd20(&name, e.shader.buffers.strings);
}
void copy_x87(float& destination, const float& source) noexcept {
    auto* to = &destination;
    const auto* from = &source;
    __asm {
        mov eax, from
        mov edx, to
        fld dword ptr [eax]
        fstp dword ptr [edx]
    }
}
void copy_word(float& destination, const float& source) noexcept {
    std::uint32_t bits;
    std::memcpy(&bits, &source, 4);
    std::memcpy(&destination, &bits, 4);
}
float shadow_displacement(std::int32_t height, const float& offset) noexcept {
    const double reference_height = 720.0; //00CEF1B8, x87 double divisor.
    const auto* scale = &offset;
    float result;
    __asm {
        mov eax, scale
        fild dword ptr [height]
        fdiv qword ptr [reference_height]
        fmul dword ptr [eax]
        fstp dword ptr [result]
    }
    return result;
}
} // namespace

void clear_gui_text_glyph_children_00ab80c0(GuiTextContentBinding& binding,
    GuiWidgetOwnerRuntime& owners, NativeNodeParentingRuntime& parenting,
    GuiTextGlyphChildCalls& calls) {
    require_text_owner(binding, owners);
    auto& children = binding.glyph_children_198;
    for (std::size_t index = 0; index < children.size(); ++index) {
        //00AB8118 receives null too; detach's own body handles it.
        auto detached = detach_gui_widget_child_00aa83a0(
            owners, parenting, binding.widget.layout(), children[index]);
        if (detached) calls.accept_detached_child(std::move(detached));
        //00AB811D..00AB813B reloads the array and bounds AFTER detach.
        if (index >= children.size())
            throw std::out_of_range("Text glyph-child slot disappeared during detach");
        GuiLayoutWidget*& captured_slot = children[index];
        if (captured_slot) {
            //00AB8144 captures the slot before deleting;00AB8153 clears it
            //only after the actual destructor/pool return has completed.
            calls.delete_text_child_virtual4(*captured_slot, 1);
            captured_slot = nullptr;
        }
    }
    //00AB8161 captures end; no callback intervenes before the erase. Thus its
    //memmove length is zero for a valid serialized vector, and end=begin.
    children.clear(); // Retains capacity/allocation; no child-object deletion.
}

GuiTextContentContinuation prepare_gui_text_content_00aba8d0_fragment(
    GuiTextContentBinding& binding, GuiTextContentEnvironment& e,
    std::u16string_view input) {
    require_text_owner(binding, e.buffers.widgets);
    ensure_gui_text_draw_sections_00ab8530(binding.widget, binding.text,
        binding.shadow_188, e.buffers); //00ABA8EC, even equal-empty
    require_native_text_domain(input);
    std::u16string copied(input); //004C8DD0 at00ABA8FA, separate owned wrapper
    auto& text = binding.text;
    if (!text.font) throw std::logic_error("Text content has no live font descriptor");
    if (text.font->uppercase_only) {
        //00ABA91C/00A9EC30: repeat this even after ellipsis already uppercased.
        //The current sidecar mapping is not assumed to be idempotent.
        for (char16_t& code_unit : copied)
            code_unit = locale_uppercase_00a9eba0(e.locale, e.locale_runtime, code_unit);
    }
    require_native_text_domain(text.text);
    require_native_text_domain(copied);
    if (equal_content(text.text, copied, e)) //00ABA921..00ABA94F
        return {GuiTextContentBranch::unchanged, {}, nullptr, nullptr};

    text.measured_width = 0.0f; //00ABA95C, before child callbacks
    clear_gui_text_glyph_children_00ab80c0(
        binding, e.buffers.widgets, e.buffers.parenting, e.calls); //00ABA964
    auto* main_model = binding.widget.model_reference();
    if (!main_model || &main_model->model_owner().node != binding.widget.node_binding() ||
        &main_model->model_owner().environment.nodes != &e.buffers.parenting.nodes ||
        main_model->model_owner().phase != NativeModelOwner::Phase::live ||
        main_model->reference_count.load() <= 0)
        throw std::logic_error("Text main drawable lacks its canonical model owner");
    auto& owners = main_model->model_owner().environment.retained_owners;
    const ActualSection main = section_zero(*main_model, owners); //00ABA96D/00ABA97B
    text.text = copied; //004C5E20 at00ABA98D: after main section lookup
    if (!copied.empty())
        return {GuiTextContentBranch::needs_nonempty_geometry,
            std::move(copied), main.section, main.mesh}; //Resume at00ABA9BF.

    //Changed-empty: only native+18 then+10, retaining primitive kind, stream
    //identities/material/layout and stale line/height/origin metrics.
    main.section->range_words_0c[3] = 0; //00ABA99A
    main.section->range_words_0c[1] = 0; //00ABA99D
    auto* shadow = binding.shadow_188; //Reload after main writes,00ABA9A0
    if (!shadow) throw std::logic_error("Text shadow drawable is missing");
    static_assert(sizeof(void*) == 4, "Text content requires Win32 actual storage");
    auto* lifetime = e.buffers.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&shadow->storage));
    auto* shadow_model = dynamic_cast<NativeModelReference*>(lifetime);
    if (!shadow_model || &shadow_model->model_owner().node != shadow ||
        &shadow_model->model_owner().storage.node != &shadow->storage ||
        &shadow_model->model_owner().environment.nodes != &e.buffers.parenting.nodes ||
        &shadow_model->model_owner().environment.retained_owners != &owners ||
        shadow_model->model_owner().phase != NativeModelOwner::Phase::live ||
        shadow_model->reference_count.load() <= 0)
        throw std::logic_error("Text shadow lacks its canonical model owner");
    const ActualSection shadow_section = section_zero(*shadow_model, owners); //00ABA9A7/00ABA9AF
    shadow_section.section->range_words_0c[3] = 0; //00ABA9B4
    shadow_section.section->range_words_0c[1] = 0; //00ABA9B7
    return {GuiTextContentBranch::cleared, {}, nullptr, nullptr};
}

void* native_mesh_vertex_stream_00b73260(const NativeMeshStorage& mesh,
    std::uint32_t index) {
    if (index >= 6) throw std::out_of_range("Native mesh stream index exceeds six slots");
    return mesh.vertex_streams_64[index]; //No count test in00B73260.
}

void set_native_model_local_position_00b6dab0_fragment(NativeModelOwner& model,
    const std::array<float, 3>& position) {
    if (model.phase != NativeModelOwner::Phase::live ||
        model.node.transform.raw_node_key() != reinterpret_cast<std::uint32_t>(&model.storage.node))
        throw std::logic_error("Model position requires the same live native transform");
    auto& transform = model.node.transform;
    const auto* table = model.environment.vtable_00d62de8;
    if (model.storage.node.vtable_00 != 0x00d62de8u || !table)
        throw std::logic_error("Model position requires its current Model table view");
    copy_x87(transform.local[12], position[0]);
    copy_x87(transform.local[13], position[1]);
    const auto* source_z = &position[2];
    auto* destination_z = &transform.local[14];
    std::uint32_t target;
    __asm {
        mov eax, source_z
        fld dword ptr [eax]
        mov eax, table
        mov eax, dword ptr [eax + 0x38]
        mov target, eax
        mov eax, destination_z
        fstp dword ptr [eax]
    }
    //B6DAD4 captures current+38 between the z load/store. This Model profile
    //uses B6DB10; do not call it for a derived camera override.
    if (target != 0x00b6db10u)
        throw std::logic_error("Model position has no supported current virtual38");
    set_transform_local_matrix_00b6db10(transform, transform.local);
}

GuiTextNonemptyContinuation prepare_gui_text_nonempty_00aba8d0_fragment(
    GuiTextLifetime& lifetime, GuiTextContentContinuation&& pending,
    GuiTextNonemptyEnvironment& e) {
    require_nonempty_environment(lifetime, e);
    if (pending.branch != GuiTextContentBranch::needs_nonempty_geometry ||
        pending.transformed_text.empty() || !pending.main_mesh || !pending.main_section)
        throw std::logic_error("Text nonempty stage requires the exact pending prefix");
    auto binding = lifetime.content_binding();
    auto& buffers = e.shader.buffers;
    auto& owners = buffers.materials.retained_owners;
    auto& main_mesh = *static_cast<NativeMeshStorage*>(pending.main_mesh);
    auto& main_section = *pending.main_section;
    create_gui_text_glyph_buffers_00ab8400(
        static_cast<std::uint32_t>(pending.transformed_text.size()), main_mesh,
        buffers.current_renderer_00f8d394, buffers.strings, owners); //ABA9C3.
    auto& main_material = actual_material(main_section.material_20, owners); //ABA9C8.
    main_section.primitive_08 = 4;
    main_section.range_words_0c[0] = 0;
    main_section.range_words_0c[2] = 0;
    main_section.range_words_0c[1] = 0;
    main_section.range_words_0c[3] = 0;
    const bool attempted = ensure_gui_text_font_shader_00ab8ce0(lifetime, e.shader);
    if (attempted) {
        set_native_material_shader_00b19210(main_material,
            lifetime.cached_shader_slot_1ec(), buffers.materials);
        auto& pair = lifetime.fields().overbright_alphatex_1dc;
        copy_x87(pair[0], binding.widget.extra_fields().overbright_94);
        copy_x87(pair[1], binding.text.alpha_texture_scale);
        register_parameter(main_material, "cOverbrightAlphatex", pair, 2, e);
        register_parameter(main_material, "cLowColor", binding.widget.layout().low_color, 4, e);
        register_parameter(main_material, "cHighColor", binding.widget.layout().high_color, 4, e);
        register_parameter(main_material, "cBlendFactor", &binding.widget.layout().blend_factor, 1, e);
        register_native_gui_clip_parameters_00aa9f10(binding.widget, main_material,
            buffers.widgets, e.style.aspect_ratio_00e12fc0, e.parameters);
    }
    rebuild_native_mesh_section_vertex_layout_00b865a0(main_section, owners, &main_mesh, e.layouts);
    //Read multiline only AFTER all preceding callbacks; retain the native
    //temporary, captured section/material and saved attempted byte across the gap.
    const auto builder = binding.text.multiline ? GuiTextGeometryBuilder::wrapped_00aba270 :
        GuiTextGeometryBuilder::single_00ab9fd0;
    return {&lifetime, std::move(pending.transformed_text), &main_mesh, &main_section,
        &main_material, attempted, builder};
}

void finish_gui_text_content_after_geometry_00aba8d0_fragment(GuiTextLifetime& lifetime,
    GuiTextNonemptyContinuation&& pending, GuiTextNonemptyEnvironment& e) {
    require_nonempty_environment(lifetime, e);
    if (pending.lifetime != &lifetime || !pending.main_mesh || !pending.main_section ||
        !pending.main_material || pending.transformed_text.empty())
        throw std::logic_error("Text post-builder stage requires its same live continuation");
    auto binding = lifetime.content_binding();
    auto& buffers = e.shader.buffers;
    auto& owners = buffers.materials.retained_owners;
    auto style = lifetime.style_binding(e.style);
    set_gui_text_color50_00ab6b50(style, binding.widget.layout().color); //ABAB88.
    const ActualSection shadow = section_zero(current_shadow(binding, buffers), owners);
    set_native_mesh_vertex_stream_00b73bb0(*shadow.mesh, owners, 0,
        native_mesh_vertex_stream_00b73260(*pending.main_mesh, 0));
    //The index read is after the vertex setter's terminal callbacks.
    set_native_mesh_index_stream_00b73b70(*shadow.mesh, owners, pending.main_mesh->index_stream_60);
    shadow.section->primitive_08 = pending.main_section->primitive_08;
    shadow.section->range_words_0c[0] = pending.main_section->range_words_0c[0];
    shadow.section->range_words_0c[2] = pending.main_section->range_words_0c[2];
    shadow.section->range_words_0c[1] = pending.main_section->range_words_0c[1];
    shadow.section->range_words_0c[3] = pending.main_section->range_words_0c[3];
    rebuild_native_mesh_section_vertex_layout_00b865a0(*shadow.section, owners, shadow.mesh, e.layouts);
    //Native reuses the captured MAIN material and only now captures the
    //current SHADOW section material, after layout-release callbacks.
    auto& main_material = *pending.main_material;
    const bool has_texture = main_material.texture_count_34 > 0;
    auto& shadow_material = actual_material(shadow.section->material_20, owners);
    void* const texture0 = has_texture ? main_material.textures_10[0] : nullptr;
    set_native_material_texture_00b189f0(shadow_material, 0, texture0, owners);
    if (pending.shader_selection_attempted) {
        set_native_material_shader_00b19210(shadow_material, main_material.effect_7c, buffers.materials);
        register_native_gui_clip_parameters_00aa9f10(binding.widget, shadow_material,
            buffers.widgets, e.style.aspect_ratio_00e12fc0, e.parameters);
        register_parameter(shadow_material, "cBlendFactor", &e.blend_factor_00f8be54, 1, e);
        register_parameter(shadow_material, "cOverbrightAlphatex",
            lifetime.fields().overbright_alphatex_1dc, 2, e);
        register_parameter(shadow_material, "cLowColor", binding.widget.layout().low_color, 4, e);
        register_parameter(shadow_material, "cHighColor", binding.widget.layout().high_color, 4, e);
        //The second registration is native: it replaces the source of the
        //same matching record, after all intervening parameter operations.
        register_parameter(shadow_material, "cBlendFactor", &binding.widget.layout().blend_factor, 1, e);
    }
    const ActualSection current = section_zero(current_shadow(binding, buffers), owners);
    float* diffuse = native_material_diffuse_00b179f0(actual_material(current.section->material_20, owners), 0);
    copy_word(diffuse[0], binding.text.shadow_color.r);
    copy_word(diffuse[1], binding.text.shadow_color.g);
    copy_word(diffuse[2], binding.text.shadow_color.b);
    copy_word(diffuse[3], binding.text.shadow_color.a); //Overwrites color50's scaled alpha.
    const auto* current_font = binding.text.font; //ABADDE, after all preceding callbacks.
    const std::int32_t height = current_font ? e.fonts.resolve(current_font).signed_height_14() : 0;
    const bool behind = binding.text.shadow_pos == GuiTextShadowPos::Behind; //Before x87.
    const float displacement = shadow_displacement(height, binding.text.shadow_offset);
    const std::array<float, 3> position{displacement, displacement, behind ? 0.5f : -0.5f};
    auto& position_model = current_shadow(binding, buffers).model_owner();
    const auto* table = position_model.environment.vtable_00d62de8;
    if (position_model.storage.node.vtable_00 != 0x00d62de8u || !table ||
        table[0x2c / 4] != 0x00b6dab0u)
        throw std::logic_error("Text shadow has no supported current Model position slot");
    set_native_model_local_position_00b6dab0_fragment(position_model, position); //ABAE54.
    auto* parent = binding.text.shadowed ? binding.widget.node_binding() : nullptr;
    set_native_node_parent_00b6e680(buffers.parenting,
        current_shadow(binding, buffers).model_owner().node.transform, parent ? &parent->transform : nullptr);
    if (!binding.text.shadowed) //Reload byte and shadow after parenting callbacks.
        propagate_native_node_root_00b6d890(buffers.parenting.nodes,
            current_shadow(binding, buffers).model_owner().node.transform, nullptr);
    pending.transformed_text.clear(); //Native temporary cleanup; all native work has completed.
}
} // namespace bsp
