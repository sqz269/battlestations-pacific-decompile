#include "bsp/gui_text_lifetime.hpp"
#include "bsp/native_render_context.hpp"
#include <cstring>
#include <limits>
#include <stdexcept>

namespace bsp {
namespace {
void copy_x87(float& destination, const float& source) noexcept {
    auto* to = &destination;
    const auto* from = &source;
    __asm {
        mov eax, from
        mov ecx, to
        fld dword ptr [eax]
        fstp dword ptr [ecx]
    }
}
template<class String> void require_copy_string(const String& value) {
    if (value.size() > static_cast<std::size_t>((std::numeric_limits<std::int32_t>::max)()) ||
        value.find(typename String::value_type{}) != String::npos)
        throw std::invalid_argument("Text copy requires the native terminated-string domain");
}
} // namespace
bool gui_text_matches_type0c_00ab83d0(std::uint32_t descriptor,
    const volatile std::uint32_t (&lineage)[3]) noexcept {
    for (std::size_t i = 0; i != 3; ++i)
        if (lineage[i] == descriptor) return true;
    return false;
}

void GuiTextLifetime::require_owner() const {
    if (widget_.layout().type != GuiWidgetType::Text ||
        widget_.layout().transform.type_id != 3 ||
        &buffers_.widgets.owner(widget_.layout()) != &widget_)
        throw std::logic_error("Text lifetime requires its same retained type3 widget owner");
}

GuiTextLifetime::GuiTextLifetime(GuiTextDeferredDefaultAdmission,
    GuiWidgetOwner& widget, GuiTextBufferServices& buffers,
    GuiTextGlyphChildCalls& children, const GuiTextConstructorConstants& constants)
    : widget_(widget), buffers_(buffers), child_calls_(children) {
    require_owner();
    if (widget_.text_lifetime_)
        throw std::logic_error("widget already has its canonical Text companion");
    // AA9390 has already initialized the one base owner. These two legacy
    // reader projections do not create another base/layout or scene node.
    text_.size = widget.layout().transform.size;
    const auto* color = widget.layout().color;
    text_.color = {color[0], color[1], color[2], color[3]};
    text_.align = GuiTextAlign::Left;
    text_.vertical_align = GuiTextVerticalAlign::Top;
    text_.font = nullptr;
    text_.distance_between_lines = 0.0f;
    text_.line_count = 0;
    text_.measured_width = 0.0f;
    text_.has_state_colors = false;
    text_.shadowed = 0;
    text_.shadow_pos = GuiTextShadowPos::Behind;
    text_.shadow_offset = constants.shadow_offset_00d5c5c0;
    text_.multiline = true;
    text_.shadow_color.r = constants.shadow_color_00e12fd8[0];
    text_.shadow_color.g = constants.shadow_color_00e12fd8[1];
    text_.shadow_color.b = constants.shadow_color_00e12fd8[2];
    text_.shadow_color.a = constants.shadow_color_00e12fd8[3];
    // All other constructor-written extension words are initialized in their
    // canonical members above. No reset of constructor-unwritten fields.
    const float normal = constants.normal_rgb_00ce3e18;
    const float one = constants.one_00d7a24c;
    text_.has_cached_shader = false;
    text_.state_colors.normal = {normal, normal, normal, one};
    text_.default_shadow = -1;
    text_.font_scale = one;
    text_.state_colors.focus = {one, one, one, one};
    text_.state_colors.selected = {one, one, one, one};
    text_.state_colors.disabled = {0.0f, 0.0f, 0.0f, constants.disabled_alpha_00ce3800};
    // All derived defaults exist before AB8530 can call back into this owner.
    // The companion may be queried here, but scalar deletion rejects construction.
    widget_.text_lifetime_ = this;
}

void GuiTextLifetime::complete_default_construction_00ab9650() {
    require_owner();
    if (after_base_copy_ || default_completion_entered_ || phase_ != Phase::constructing ||
        widget_.text_lifetime_ != this)
        throw std::logic_error("Text default completion requires its retained fresh lifetime");
    default_completion_entered_ = true;
    ensure_gui_text_draw_sections_00ab8530(widget_, text_, shadow_188_, buffers_, sections_);
    phase_ = Phase::live;
}

GuiTextLifetime::GuiTextLifetime(GuiTextAfterBaseCopy00aa9520,
    GuiWidgetOwner& widget, GuiTextBufferServices& buffers,
    GuiTextGlyphChildCalls& children, const GuiTextLifetime& source)
    : widget_(widget), buffers_(buffers), child_calls_(children) {
    require_owner();
    source.require_owner();
    if (!widget.base_copy_complete_00aa9520() ||
        &widget == &source.widget_ || widget.text_lifetime_ ||
        source.phase_ != Phase::live || source.widget_.text_lifetime_ != &source ||
        source.scalar_phase_ != GuiTextScalarDeletionPhase::not_started ||
        source.has_incomplete_copy() || source.has_incomplete_native_resources() ||
        &buffers != &source.buffers_ || &children != &source.child_calls_ ||
        !widget.node_binding() || !source.widget_.node_binding() ||
        widget.node_binding() == source.widget_.node_binding())
        throw std::logic_error("Text copy admission requires distinct already-copied owners and the same live source domain");
    // The canonical base producer publishes completion only after its field
    // copies and actual primary-node current10 return. The tag selects this
    // derived admission; an ordinary default owner cannot pass the guard.
    require_copy_string(source.text_.text);
    require_copy_string(source.text_.source);
    require_copy_string(source.text_.shader_name);
    require_copy_string(source.text_.font_name);
    text_.size = widget.layout().transform.size;
    const auto* color = widget.layout().color;
    text_.color = {color[0], color[1], color[2], color[3]};

    text_.text = source.text_.text; // 4C8DD0, distinct zeroed destination header.
    text_.source = source.text_.source; // 41DD40 terminates, then length-byte memcpy.
    text_.multiline = source.text_.multiline;
    text_.align = source.text_.align;
    text_.vertical_align = source.text_.vertical_align;
    text_.font = source.text_.font;
    copy_x87(text_.distance_between_lines, source.text_.distance_between_lines);
    text_.line_count = source.text_.line_count;
    copy_x87(text_.measured_width, source.text_.measured_width);
    text_.has_state_colors = source.text_.has_state_colors;
    text_.shadowed = source.text_.shadowed;
    text_.shadow_pos = source.text_.shadow_pos;
    copy_x87(text_.shadow_offset, source.text_.shadow_offset);
    std::memcpy(&text_.shadow_color, &source.text_.shadow_color, sizeof(text_.shadow_color));
    copy_x87(fields_.field_178, source.fields_.field_178);
    fields_.field_17c = source.fields_.field_17c;
    fields_.pointer_180 = source.fields_.pointer_180;
    // +184/+188 and the glyph vector start empty, never shared from source.
    copy_x87(fields_.field_18c, source.fields_.field_18c);
    copy_x87(fields_.field_190, source.fields_.field_190);
    // +1A4/+1A8/+1AC/+1B0 are zero/empty in their canonical fields.
    fields_.byte_1b4 = source.fields_.byte_1b4;
    text_.shader_name = source.text_.shader_name;
    text_.font_name = source.text_.font_name;
    text_.default_shadow = source.text_.default_shadow;
    copy_x87(text_.font_scale, source.text_.font_scale);
    text_.has_cached_shader = false; // Same empty actual +1EC slot.
    fields_.byte_1f0 = source.fields_.byte_1f0;
    std::memcpy(&text_.state_colors, &source.text_.state_colors, sizeof(text_.state_colors));
    // No read/copy of +1B8/+1BC/+1D4/+1DC..1E8; glyph validity remains false.
    // Typed string allocation/cleanup is a new C++ ABI, not native pool/SEH.
    after_base_copy_ = true;
    widget_.text_lifetime_ = this;
    // Native derived construction has not returned yet. Existing scalar
    // deletion rejects constructing BEFORE its phase/flag stores. Only the
    // final successful ABB1D0 continuation may make this companion live.
}

GuiTextLifetime::~GuiTextLifetime() noexcept {
    // A failed/pending copy still owns native caller effects/continuations.
    // Destroying this canonical companion cannot silently discard them.
    if (has_incomplete_copy() || has_incomplete_native_resources()) std::terminate();
    // Explicit scalar completion already removed the borrowed association and
    // may have erased the owner record; an externally embedded companion must
    // not dereference that dead owner during its later C++ member destruction.
    if (scalar_phase_ == GuiTextScalarDeletionPhase::complete) return;
    // An incomplete scalar deletion must keep the owner/companion alive.
    // Destruction here would lose an unimplemented native continuation.
    if (scalar_phase_ != GuiTextScalarDeletionPhase::not_started) std::terminate();
    destroy_derived_00ab8250_fragment();
    if (widget_.text_lifetime_ == this) widget_.text_lifetime_ = nullptr;
}

GuiTextContentBinding GuiTextLifetime::content_binding() noexcept {
    return {widget_, text_, shadow_188_, glyph_children_198_};
}
GuiTextStyleBinding GuiTextLifetime::style_binding(const GuiMaterialBindingServices& materials) {
    require_owner();
    if (&materials.widgets != &buffers_.widgets ||
        &materials.actual_owners != &buffers_.geometry.actual_owners())
        throw std::logic_error("Text style must use the same widget and actual resource domains");
    return {widget_, text_, shadow_188_, materials, buffers_.parenting};
}

void GuiTextLifetime::release_shadow_00ab73b0() {
    require_owner();
    auto* captured = shadow_188_;
    if (!captured) return;
    auto* lifetime = buffers_.parenting.nodes.attachments.find_actual_node(
        reinterpret_cast<std::uint32_t>(&captured->storage));
    auto* reference = dynamic_cast<NativeModelReference*>(lifetime);
    if (!reference) throw std::logic_error("Text shadow has no canonical model lifetime");
    auto& model = reference->model_owner();
    if (&model.node != captured || &model.storage.node != &captured->storage ||
        &model.environment.nodes != &buffers_.parenting.nodes ||
        &model.environment.retained_owners != &buffers_.geometry.actual_owners() ||
        model.phase != NativeModelOwner::Phase::live || reference->reference_count.load() <= 0)
        throw std::logic_error("Text shadow must be the same live actual model and service domains");
    unlink_and_release_render_model_00b6dfa0(*reference);
    shadow_188_ = nullptr;
}

void GuiTextLifetime::release_secondary_scene_nodes_00aa8320_fragment(
    const volatile std::uint32_t (&lineage)[3]) {
    // The embedding before_scene_release completed derived destruction;
    // subsequent AA9730 installs the base vtable before its AA8320 call.
    // Retaining a C++ Text companion must not resurrect the Text override.
    if (phase_ == Phase::destroyed) return;
    // AB6A30 loads the current first descriptor; AB83D0 then independently
    // reloads each table entry. Do not reduce this to unconditional true.
    const auto text_descriptor = lineage[0];
    if (gui_text_matches_type0c_00ab83d0(text_descriptor, lineage))
        release_shadow_00ab73b0();
}

void GuiTextLifetime::destroy_derived_00ab8250_fragment() {
    if (has_incomplete_copy() || has_incomplete_native_resources())
        throw std::logic_error("Text native construction/resource operation must complete before derived retirement");
    if (phase_ == Phase::destroyed) return; // C++ destructor after explicit teardown.
    if (phase_ != Phase::live)
        throw std::logic_error("Text destruction requires a live, non-reentrant companion");
    require_owner();
    phase_ = Phase::destroying;
    if (auto* captured = cached_shader_1ec_) {
        release_native_render_actual_owner(buffers_.geometry.actual_owners(), captured);
        cached_shader_1ec_ = nullptr; // Native clears after any terminal callback.
    }
    text_.has_cached_shader = false;
    release_shadow_00ab73b0();
    auto binding = content_binding();
    clear_gui_text_glyph_children_00ab80c0(binding, buffers_.widgets,
        buffers_.parenting, child_calls_);
    // C++ allocations, released NOW in the native five-string/vector order.
    // Native dangling header words and native sized-pool/CRT calls are not
    // projected by these semantic containers; no replacement owners exist.
    std::string{}.swap(text_.font_name);
    std::string{}.swap(text_.shader_name);
    std::u16string{}.swap(fields_.string_1a4);
    std::vector<GuiLayoutWidget*>{}.swap(glyph_children_198_);
    std::string{}.swap(text_.source);
    std::u16string{}.swap(text_.text);
    phase_ = Phase::destroyed;
}
} // namespace bsp
