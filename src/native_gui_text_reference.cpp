#include "bsp/native_gui_text_reference.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include <exception>
#include <initializer_list>
#include <new>
#include <stdexcept>
#include <type_traits>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual Text references require MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
constexpr Word text_profile = 0x00d5c6c8u;
constexpr Word zero_target = 0x00bd30e0u;
constexpr Word scalar_target = 0x00ab8ee0u;
static_assert(sizeof(void*) == 4 && sizeof(std::atomic<std::int32_t>) == 4);
static_assert(std::is_trivially_destructible_v<NativeGuiTextReferenceFrame>);
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
Word read(void* raw) noexcept { return *static_cast<const volatile Word*>(raw); }
void require(bool condition, const char* message) {
    if (!condition) throw std::invalid_argument(message);
}
struct Region { const volatile void* begin; std::size_t size; };
bool disjoint(Region left, Region right) noexcept {
    const auto l = static_cast<std::uint64_t>(bits(left.begin));
    const auto r = static_cast<std::uint64_t>(bits(right.begin));
    return l + left.size <= r || r + right.size <= l;
}
bool representable(Region r) noexcept {
    return r.begin && r.size && static_cast<std::uint64_t>(bits(r.begin)) + r.size <= 0x100000000ull;
}
template<class Check>
void known_borrowed(NativeGuiTextReferenceContext& c,
    const volatile Word* profile, NativeGuiTextReferenceDisposal disposal, Check check) {
    auto& p = c.construction_context;
    auto& f = c.construction_frame.sections;
    auto& a = c.construction.sections;
    auto& t = c.retirement;
    auto& r = t.resources;
    auto& node = r.parenting.node;
    for (const Region region : {Region{&c, sizeof(c)}, {&c.owners, sizeof(c.owners)},
        {&c.data, sizeof(c.data)}, {profile, 8}, {&p, sizeof(p)},
        {&c.construction_frame, sizeof(c.construction_frame)},
        {&c.construction, sizeof(c.construction)}, {&f, sizeof(f)},
        {&f.locals[0], sizeof(f.locals)}, {&f.parenting, sizeof(f.parenting)},
        {&p.state, sizeof(p.state)}, {&p.shadow_offset_00d5c5c0, 4},
        {&p.normal_rgb_00ce3e18, 4}, {&p.one_00d7a24c, 4}, {&p.disabled_alpha_00ce3800, 4},
        {&t, sizeof(t)}, {&r, sizeof(r)}, {&t.base, sizeof(t.base)}, {&t.glyphs, sizeof(t.glyphs)},
        {&r.actual, sizeof(r.actual)}, {&r.actual.models, sizeof(r.actual.models)},
        {&r.actual.materials, sizeof(r.actual.materials)}, {&r.actual.geometry, sizeof(r.actual.geometry)},
        {&r.node_constants, sizeof(r.node_constants)}, {&r.strings, sizeof(r.strings)},
        {&r.string_process, sizeof(r.string_process)}, {&r.effects, sizeof(r.effects)},
        {&r.parenting, sizeof(r.parenting)}, {&r.parenting.dispatch, sizeof(r.parenting.dispatch)},
        {&r.parenting.group_type_0109032c, 4}, {&node, sizeof(node)},
        {&node.trees, sizeof(node.trees)}, {&node.scenes, sizeof(node.scenes)},
        {&t.base.nodes, sizeof(t.base.nodes)}, {&t.base.bindings, sizeof(t.base.bindings)},
        {t.base.actual_base_table_00d5c130, 9 * 4},
        {&t.base.actual_base_lineage_00f8bc88[0], sizeof(t.base.actual_base_lineage_00f8bc88)},
        {&t.base.actual_text_descriptor_00f8be28, 4},
        {&r.decrement_00ce2220, sizeof(r.decrement_00ce2220)},
        {&r.actual.unchanged_00d7a260, sizeof(r.actual.unchanged_00d7a260)},
        {&r.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)},
        {r.effects.effect_base_vtable_00d5e534, 8}, {r.effects.effect_derived_vtable_00d61a00, 8},
        {&t.actual_pool_00f8bdf0, sizeof(t.actual_pool_00f8bdf0)},
        {&t.actual_pool_00f8bdf0.storage(), sizeof(t.actual_pool_00f8bdf0.storage())},
        {a.model_owner, sizeof(*a.model_owner)}, {a.model_creator, sizeof(*a.model_creator)},
        {a.mesh_creator, sizeof(*a.mesh_creator)},
        {a.shadow_section.section_creator, sizeof(*a.shadow_section.section_creator)},
        {a.shadow_section.material_creator, sizeof(*a.shadow_section.material_creator)},
        {a.main_section.section_creator, sizeof(*a.main_section.section_creator)},
        {a.main_section.material_creator, sizeof(*a.main_section.material_creator)},
        {a.mesh.companion, sizeof(RenderCommandReference)},
        {a.shadow_section.section.companion, sizeof(RenderCommandReference)},
        {a.main_section.section.companion, sizeof(RenderCommandReference)},
        {disposal.context, disposal.context_bytes}}) {
        // Null optional creator aliases have no region. Their presence/absence
        // does not prove the retained transitive graph or adopt native ownership.
        if (region.begin) {
            require(representable(region), "Text reference borrowed region must have a valid extent");
            check(region);
        }
    }
}
bool fresh(const NativeGuiTextScalarRetirementAcquired& a) noexcept {
    const auto& d = a.destruction;
    return !a.started && !a.complete && !a.destructor_entered && !a.destructor_returned &&
        !a.flags_captured && !a.pool_return_entered && !a.pool_returned && !a.captured_flags_byte &&
        !d.started && !d.complete && !d.exception_cleanup_started && d.native_eh_state == -1 &&
        !d.destination && d.boundary == NativeGuiTextRetirementAcquired::Boundary::none &&
        !d.shader.decrement_entered && !d.shadow.started && !d.glyphs.started &&
        !d.normal_actions_entered && !d.normal_actions_returned && !d.unwind_actions_consumed;
}
struct Admission {
    const volatile Word* profile;
    std::atomic<std::int32_t>& count;
};
Admission preflight(void* companion, void* raw, NativeGuiTextReferenceContext& c,
    NativeGuiTextReferenceFrame& frame, NativeGuiTextReferenceDisposal disposal) {
    require(companion && bits(companion) % alignof(NativeGuiTextReference) == 0 &&
        raw && bits(raw) % 4 == 0 && disposal.retire &&
        ((disposal.context != nullptr) == (disposal.context_bytes != 0)),
        "Text binding requires aligned separate companion/Text and explicit sized disposal");
    const auto* profile = static_cast<const volatile Word*>(c.data.data_at(text_profile, 8));
    const Region writes[] = {{raw, 0x1f8}, {companion, sizeof(NativeGuiTextReference)}, {&frame, sizeof(frame)}};
    for (std::size_t i = 0; i != 3; ++i) {
        require(representable(writes[i]), "Text reference caller region must have a valid extent");
        for (std::size_t j = i + 1; j != 3; ++j)
            require(disjoint(writes[i], writes[j]), "Text, companion and scalar frame must be separate");
    }
    known_borrowed(c, profile, disposal, [&](Region borrowed) {
        for (const auto region : writes)
            require(disjoint(region, borrowed), "Text binding caller regions cannot alias retained providers");
    }); // ALL known checks precede placement construction/member/vptr stores.
    const auto& a = c.construction;
    require(a.started && a.complete && a.base_complete && a.derived_complete && a.sections_entered &&
        !a.exception_cleanup_started && !a.base_lifetime_ended && !a.consumed_cleanup_actions &&
        a.native_eh_state == 6 && a.destination == raw && a.sections.started && a.sections.complete &&
        !a.sections.exception_cleanup_started && a.sections.failure == NativeGuiTextSectionStorageAcquired::Failure::none &&
        fresh(frame.scalar), "Text binding requires SAME genuine completed93 and fresh persistent scalar diagnostics");
    auto& p = c.construction_context;
    auto& t = c.retirement;
    auto& r = t.resources;
    auto& node = r.parenting.node;
    require(&p.sections == &r && &p.lifetime == &t.base &&
        &c.owners == &r.actual.geometry.actual_owners() &&
        &t.base.nodes == &r.actual.models.nodes && &t.glyphs.nodes == &t.base.nodes &&
        &t.glyphs.children == &t.base.bindings && !r.actual.models.actual_names &&
        t.base.nodes.uses_raw_name_pool() && &t.base.nodes.require_raw_name_pool() == &r.strings &&
        &r.strings == &r.string_process.raw_context() && &r.actual.strings == &r.string_process.strings() &&
        &r.actual.models.retained_owners == &c.owners && &r.actual.materials.retained_owners == &c.owners &&
        &node.strings == &r.strings && &node.scenes.owners == &c.owners && &node.trees.owners == &c.owners &&
        &node.trees.point_lights == &t.base.nodes.point_lights &&
        &node.scenes.decrement_00ce2220 == &r.decrement_00ce2220 &&
        &node.trees.decrement_00ce2220 == &r.decrement_00ce2220 && r.decrement_00ce2220 &&
        &r.effects.effects.owners.actual_owners() == &c.owners &&
        &r.effects.effects.owners.string_storage() == &r.actual.strings && &r.effects.strings == &r.actual.strings &&
        r.effects.effect_base_vtable_00d5e534 && r.effects.effect_derived_vtable_00d61a00 &&
        t.base.actual_base_table_00d5c130,
        "Text binding must retain SAME genuine93/118 registry, raw Node, strings and model/effect domains");
    require(read(raw) == text_profile && bits(profile) == text_profile &&
        profile[0] == zero_target && profile[1] == scalar_target,
        "Text binding requires the genuine current D5C6C8 Original-VA alias");
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(bits(raw) + 4u));
    require(count.load(std::memory_order_relaxed) > 0 && !c.owners.find(raw),
        "Text binding requires live positive actual+04 and an unregistered identity");
    return {profile, count};
}
void flags_preflight(const volatile Word& flags, NativeGuiTextReference* companion,
    void* raw, NativeGuiTextReferenceContext& c, NativeGuiTextReferenceFrame& frame,
    const volatile Word* profile, NativeGuiTextReferenceDisposal disposal) {
    const Region flags_region{&flags, sizeof(flags)};
    require(bits(&flags) % 4 == 0 && representable(flags_region), "Direct Text scalar requires stable DWORD flags backing");
    for (const Region region : {Region{raw, 0x1f8}, {companion, sizeof(*companion)}, {&frame, sizeof(frame)}})
        require(disjoint(flags_region, region), "Direct Text caller flags must be separate from Text/companion/frame");
    known_borrowed(c, profile, disposal, [&](Region borrowed) {
        require(disjoint(flags_region, borrowed), "Direct Text flags cannot alias retained providers");
    }); // ADDRESS/extent only; real118 performs the sole late flags BYTE read.
}
} // namespace

NativeGuiTextReference& bind_native_gui_text_reference(void* companion_storage, void* raw,
    NativeGuiTextReferenceContext& context, NativeGuiTextReferenceFrame& frame,
    NativeGuiTextReferenceDisposal disposal) {
    const auto admission = preflight(companion_storage, raw, context, frame, disposal);
    return *::new (companion_storage) NativeGuiTextReference(raw, context, frame,
        disposal, admission.profile, admission.count);
}
NativeGuiTextReference::NativeGuiTextReference(void* raw, NativeGuiTextReferenceContext& context,
    NativeGuiTextReferenceFrame& frame, NativeGuiTextReferenceDisposal disposal,
    const volatile Word* profile, std::atomic<std::int32_t>& count)
    : RenderCommandReference(count), identity_(raw), context_(context), frame_(frame),
      disposal_(disposal), profile_(profile) {
    // A throwing bind unwinds base metadata only: the derived destructor is not
    // called for a failed constructor, and no actual count/creator is consumed.
    context_.owners.bind(identity_, *this);
}
NativeGuiTextReference::~NativeGuiTextReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeGuiTextReference::require_slot(Word profile, unsigned slot, Word target) const {
    if (profile != text_profile || profile_[slot] != target)
        throw std::logic_error("actual Text lacks its current native scalar/zero selector");
}
NativeGuiTextReference::Completion NativeGuiTextReference::completion() const noexcept {
    return {identity_, &context_.owners, disposal_};
}
void NativeGuiTextReference::finish_success(Completion saved) noexcept {
    phase_ = Phase::retired;
    saved.owners->unbind(saved.identity, *this); // Metadata only; raw may be pooled.
    saved.disposal.retire(saved.disposal.context, *this); // LAST; may destroy this.
}
void NativeGuiTextReference::delete_vslot04(void* raw, Word captured_profile, Word flags) {
    if (phase_ != Phase::zero_dispatch)
        throw std::logic_error("Text zero scalar adapter rejects reentry before raw access");
    if (raw != identity_ || flags != 1)
        throw std::logic_error("Text BD30E0 adapter requires same actual identity and literal1");
    require_slot(captured_profile, 1, scalar_target); // CURRENT numeric+4 capture.
    auto& storage = context_.retirement;
    auto& acquired = frame_.scalar;
    auto& actual_flags = frame_.zero_flags;
    phase_ = Phase::retiring; // Consume before the real destructor/callbacks.
    actual_flags = 1; // Reached PUSH1 projection, not an old prepared flag value.
    delete_native_gui_text_storage_00ab8ee0(raw, actual_flags, storage, acquired);
}
void NativeGuiTextReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate(); // No raw read on reentry/failure.
    const auto saved = completion(); // Independent metadata BEFORE real calls.
    phase_ = Phase::zero_dispatch;
    try {
        if (context_.owners.find(identity_) != this || reference_count.load(std::memory_order_relaxed) != 0)
            throw std::logic_error("Text zero terminal requires same canonical observed-zero binding");
        require_slot(read(identity_), 0, zero_target);
        invoke_native_ref_counted_delete_00bd30e0(identity_, *this); // Fresh current profile reload.
        finish_success(saved); // No profile/count/payload validation after scalar.
    } catch (...) {
        phase_ = Phase::failed;
        std::terminate(); // Source zero-terminal boundary, not Native EH proof.
    }
}
void* NativeGuiTextReference::delete_current_scalar_00ab8ee0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Text direct scalar rejects reentry before raw access");
    const auto saved = completion(); // Capture disposal/registry/identity first.
    phase_ = Phase::retiring;
    try {
        flags_preflight(flags, this, identity_, context_, frame_, profile_, disposal_);
        if (context_.owners.find(identity_) != this)
            throw std::logic_error("Text direct scalar requires its same canonical binding");
        require_slot(read(identity_), 1, scalar_target);
        void* const result = delete_native_gui_text_storage_00ab8ee0(
            identity_, flags, context_.retirement, frame_.scalar); // NO count read/gate.
        finish_success(saved); // Only metadata; callback may destroy this.
        return result; // Saved local, no companion/raw access after disposal.
    } catch (...) {
        phase_ = Phase::failed; // Retain binding, companion, diagnostics and graph.
        throw; // No Scene-style retire, raw cleanup, pool return or retry.
    }
}
} // namespace bsp
