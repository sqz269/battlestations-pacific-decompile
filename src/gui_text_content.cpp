#include "bsp/gui_text_content.hpp"
#include "bsp/font_registry.hpp"
#include "bsp/native_mesh_owner.hpp"
#include <limits>
#include <stdexcept>

namespace bsp {
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
} // namespace bsp
