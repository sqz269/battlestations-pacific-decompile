#include "bsp/gui_widget_copy.hpp"
#include "bsp/gui_widget_owner.hpp"
#include "bsp/gui_text_lifetime.hpp"
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {
// AA9557..AA95BA/AA963A/AA96A7 are FLD/FSTP, not raw DWORD copies.
// In particular, an unmasked signaling NaN can fault and a masked one quiets.
__declspec(noinline) void copy_x87(float& destination, const float& source) noexcept {
    auto* output_address = &destination;
    auto* input_address = &source;
    __asm {
        mov eax, input_address
        mov ecx, output_address
        fld dword ptr [eax]
        fstp dword ptr [ecx]
    }
}
__declspec(noinline) void copy_word(float& destination, const float& source) noexcept {
    std::memcpy(&destination, &source, sizeof(float));
}
// AA95BA loads rotation before clearing4C, then AA95C0 spills it. The three
// host publications all represent that SAME native4C; none creates a node.
void copy_rotation(float& destination, const float& source,
    NativeNodeBinding*& node, void*& scene, std::uint32_t& identity) noexcept {
    auto* output_address = &destination;
    auto* input_address = &source;
    auto* node_slot = &node;
    auto* scene_slot = &scene;
    auto* identity_slot = &identity;
    __asm {
        mov eax, input_address
        fld dword ptr [eax]
        mov ecx, node_slot
        mov dword ptr [ecx], 0
        mov ecx, scene_slot
        mov dword ptr [ecx], 0
        mov ecx, identity_slot
        mov dword ptr [ecx], 0
        mov ecx, output_address
        fstp dword ptr [ecx]
    }
}
struct ActiveCopy {
    bool& source;
    bool& destination;
    ActiveCopy(bool& from, bool& to) : source(from), destination(to) {
        source = destination = true;
    }
    ~ActiveCopy() { source = destination = false; }
};
bool empty_creators(const NativeGuiTextModelCloneAcquired& acquired) noexcept {
    const auto& stream = acquired.mesh.stream;
    return !acquired.model && !acquired.mesh.mesh && !acquired.mesh.section && !acquired.mesh.material &&
        !stream.creator && !stream.companion &&
        (stream.phase == NativeStreamClonePhase::empty || stream.phase == NativeStreamClonePhase::consumed);
}
}

GuiWidgetCopySourceBorrow::GuiWidgetCopySourceBorrow(GuiWidgetOwner& source)
    : source_(source) {
    source_.require_no_active_owned_operation();
    if (&source_.runtime_.owner(source_.layout_) != &source_ || !source_.implementation_)
        throw std::logic_error("copy source borrow requires the admitted canonical owner");
    source_.base_copy_source_borrow_ = this;
}
GuiWidgetCopySourceBorrow::~GuiWidgetCopySourceBorrow() noexcept {
    if (source_.base_copy_source_borrow_ != this) std::terminate();
    source_.base_copy_source_borrow_ = nullptr;
}

GuiWidgetOwner& GuiWidgetOwnerRuntime::construct_base_copy_00aa9520(
    GuiLayoutWidget& destination, GuiWidgetOwner& source,
    const GuiWidgetBaseCopyPreimage& preimage, const GuiWidgetCopyServices& services,
    NativeGuiTextModelCloneAcquired& acquired, const GuiWidgetCopySourceBorrow* authorized_source) {
    if (&source.runtime_ != this || &owner(source.layout_) != &source ||
        !source.implementation_ || &destination == &source.layout_)
        throw std::invalid_argument("base copy requires a distinct destination and admitted source owner");
    if (authorized_source && (&authorized_source->source_ != &source ||
        source.base_copy_source_borrow_ != authorized_source))
        throw std::logic_error("base copy requires its matching held source borrow");
    source.require_no_active_owned_operation_impl(authorized_source, false);
    if (widgets_.count(&destination) || destination.before_destroy || destination.parent ||
        destination.transform.parent || !destination.children.empty() ||
        !destination.transform.children.empty() || !empty_creators(acquired))
        throw std::invalid_argument("base copy requires fresh destination storage and empty acquired creators");
    if (source.layout_.transform.type_id != static_cast<std::int32_t>(source.layout_.type) ||
        source.layout_.transform.type_id < 0 || source.layout_.transform.type_id >= 19 ||
        source.layout_.transform.parent != (source.layout_.parent ? &source.layout_.parent->transform : nullptr))
        throw std::logic_error("base copy source projections disagree with its actual type/parent");

    auto companion = std::unique_ptr<GuiWidgetOwner>(new GuiWidgetOwner(destination, *this, preimage));
    auto& result = *companion;
    widgets_.emplace(&destination, std::move(companion));
    try {
        destination.before_destroy = [this](GuiLayoutWidget& dying) { retire_tree(dying); };
    } catch (...) {
        widgets_.erase(&destination);
        throw;
    }
    ActiveCopy active(source.base_copy_active_, result.base_copy_active_);
    auto& from = source.layout_.transform;
    auto& to = destination.transform;
    auto& extra = result.extra_;
    auto& source_extra = source.extra_;
    copy_x87(to.position.x, from.position.x); //0C
    copy_x87(to.position.y, from.position.y); //10
    copy_x87(to.position.z, from.position.z); //14
    copy_x87(to.pivot_x, from.pivot_x); //18
    copy_x87(to.pivot_y, from.pivot_y); //1C
    copy_x87(to.size.width, from.size.width); //20
    copy_x87(to.size.height, from.size.height); //24
    copy_x87(to.scale_x, from.scale_x); //28
    copy_x87(to.scale_y, from.scale_y); //2C
    for (unsigned index = 0; index != 6; ++index)
        copy_x87(extra.fields_30_44[index], source_extra.fields_30_44[index]);
    copy_rotation(to.rotate, from.rotate, result.node_, result.scene_.scene_node, destination.node_id);
    for (unsigned index = 0; index != 4; ++index)
        copy_word(destination.color[index], source.layout_.color[index]);
    copy_word(to.alpha, destination.color[3]); // same5C in the transform projection
    to.type_id = from.type_id;
    destination.type = static_cast<GuiWidgetType>(to.type_id);
    // AA95E3 A9B720 establishes a new empty sentinel/count. The canonical
    // layout's already-empty collections represent that list, without a CRT port.
    destination.parent = source.layout_.parent;
    to.parent = destination.parent ? &destination.parent->transform : nullptr;
    to.bounds_enabled = from.bounds_enabled;
    result.scene_.visibility_recurses = source.scene_.visibility_recurses;
    result.scene_.copy_constructed = true;
    result.scene_.hidden = source.scene_.hidden;
    to.mouse_hit = from.mouse_hit;
    extra.byte_79 = source_extra.byte_79;
    to.mouse_block = from.mouse_block;
    extra.pointers_88_90[0] = extra.pointers_88_90[1] = extra.pointers_88_90[2] = nullptr;
    copy_x87(extra.overbright_94, source_extra.overbright_94);
    for (unsigned index = 0; index != 4; ++index)
        copy_word(destination.low_color[index], source.layout_.low_color[index]);
    for (unsigned index = 0; index != 4; ++index)
        copy_word(destination.high_color[index], source.layout_.high_color[index]);
    copy_x87(destination.blend_factor, source.layout_.blend_factor);
    extra.byte_d4 = source_extra.byte_d4;
    extra.pointer_d8 = source_extra.pointer_d8;
    extra.layout_listener_dc = source_extra.layout_listener_dc;
    to.widescreen_align = from.widescreen_align;

    // AA96E4 source4C, AA96E7 CURRENT type60, AA96EA current node profile,
    // AA96EC live flags table, AA96F3 current10. Capture no source node earlier.
    auto* const current_node = source.node_;
    const auto current_type = from.type_id;
    if (!current_node || current_type < 0 || current_type >= 19)
        throw std::logic_error("AA9520 requires a live source Model and in-range current type");
    const auto profile = current_node->storage.vtable_00;
    const auto flags = services.clone_flags_00d5c0b8[current_type];
    if (profile != 0x00d62de8u || !environment_.models.vtable_00d62de8 ||
        environment_.models.vtable_00d62de8[0x10 / 4] != 0x00b752b0u)
        throw std::logic_error("AA9520 source current10 is not the established actual Model profile");
    auto* const source_reference = source.model_reference();
    if (!source_reference || &source_reference->model_owner().node != current_node)
        throw std::logic_error("AA9520 source4C has no canonical Model companion");
    require_model_copy_reference(*source_reference);
    auto* const copied = services.models.clone_current10(source_reference->model_owner(), flags, nullptr, acquired);
    const auto& stream = acquired.mesh.stream;
    if (copied != acquired.model || acquired.mesh.mesh || acquired.mesh.section || acquired.mesh.material ||
        stream.creator || stream.companion ||
        (stream.phase != NativeStreamClonePhase::empty && stream.phase != NativeStreamClonePhase::consumed))
        throw std::logic_error("completed current10 must publish its creator and consume temporary creators");
    if (copied) {
        require_model_copy_reference(*copied);
        if (copied == source_reference)
            throw std::logic_error("Model current10 must produce a distinct actual Model");
    }
    // AA9700 publishes4C, then AA9705 masks actual model138. Transfer the
    // existing creator into the widget, without retain/release or another owner.
    result.bind_scene_00aa6720(copied ? &copied->model_owner().node : nullptr);
    acquired.model = nullptr;
    result.base_copy_complete_ = true;
    return result;
}

void GuiWidgetOwnerRuntime::begin_base_copy_type_admission(GuiWidgetOwner& retained,
    GuiWidgetTypeImplementation& implementation) {
    if (&retained.runtime_ != this || &owner(retained.layout_) != &retained ||
        !retained.base_copy_untyped_ || !retained.base_copy_complete_ ||
        retained.implementation_ || retained.base_copy_constructor_)
        throw std::logic_error("constructor dispatch requires its completed untyped base copy");
    // A copied Text is constructing here. This is the sole admission that may
    // publish constructor dispatch while its lifetime continuation is pending.
    retained.require_no_active_owned_operation_impl(nullptr, true);
    retained.base_copy_constructor_ = &implementation;
}

void GuiWidgetOwnerRuntime::finish_base_copy_type_admission(GuiWidgetOwner& retained,
    std::unique_ptr<GuiWidgetTypeImplementation>& implementation) {
    if (&retained.runtime_ != this || &owner(retained.layout_) != &retained ||
        !retained.base_copy_untyped_ || !retained.base_copy_complete_ ||
        retained.implementation_ || !implementation ||
        (retained.base_copy_constructor_ && retained.base_copy_constructor_ != implementation.get()) ||
        (retained.text_lifetime_ && retained.text_lifetime_->has_incomplete_copy()))
        throw std::logic_error("derived copy admission requires the completed canonical base copy");
    retained.require_no_active_owned_operation_impl(nullptr, true);
    // Consume only after every preflight. A rejected pending derived copy must
    // leave the caller's sole implementation/lifetime owned and resumable.
    retained.implementation_ = std::move(implementation);
    retained.base_copy_constructor_ = nullptr;
    retained.base_copy_untyped_ = false;
}

void GuiWidgetOwnerRuntime::retire_base_copy_admission(GuiWidgetOwner& retained) {
    if (&retained.runtime_ != this || &owner(retained.layout_) != &retained ||
        !retained.base_copy_untyped_ || retained.implementation_ || retained.text_lifetime_ ||
        !retained.layout_.children.empty() || !retained.layout_.transform.children.empty())
        throw std::logic_error("base copy host cleanup requires an untyped owner without derived lifetime/children");
    retained.require_no_active_owned_operation();
    retained.require_timed_entry_ownership();
    if (retained.timed_entries_)
        throw std::logic_error("base copy host cleanup cannot consume derived timed state");
    // A provider exception does not rollback its independent acquired creators.
    // Only an already-transferred primary belongs to this admission shell.
    if (retained.node_) {
        auto& lifetime = environment_.models.nodes.attachments.resolve(retained.node_->transform);
        retained.scene_release_active_ = true;
        try { unlink_and_release_render_model_00b6dfa0(lifetime); }
        catch (...) { retained.scene_release_active_ = false; throw; }
        retained.scene_release_active_ = false;
        retained.bind_scene_00aa6720(nullptr);
    }
    auto* const layout = &retained.layout_;
    layout->before_destroy = {};
    widgets_.erase(layout);
}
} // namespace bsp
