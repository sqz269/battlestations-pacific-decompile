#include "bsp/native_gui_text_font_lifetime_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <cstdint>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Font lifetime storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Context = NativeGuiTextFontLifetimeStorageContext;
using Frame = NativeGuiTextFontLifetimeStorageFrame;
using Slot = NativeGuiTextFontLifetimeSelector;
using Name = NativeGuiTextFontInputNameProgress;
using Selector = NativeGuiTextRawFontNameStorageFrame;
using Publication = NativeGuiTextRawFontNameStoragePublication;
using Shader = NativeGuiTextShaderStorageFrame;
using Progress = Shader::CreditProgress;
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::tuple_size<decltype(NativeGuiTextFontLifetimeRegions::entries)>::value >= 42);
constexpr Word bit6 = Word{1} << 6;
void require(bool v, const char* why) { if (!v) throw std::invalid_argument(why); }
std::uintptr_t bits(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
const void* offset(const void* p, Word n) noexcept { return reinterpret_cast<const void*>(bits(p) + n); }
Word read(const void* p, Word n) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + n);
}
bool region(const void* p, std::size_t n) noexcept {
    return p && n && static_cast<std::uint64_t>(bits(p)) + n <= (std::uint64_t{1} << 32);
}
bool separate(const void* a, std::size_t an, const void* b, std::size_t bn) noexcept {
    return static_cast<std::uint64_t>(bits(a)) + an <= bits(b) ||
        static_cast<std::uint64_t>(bits(b)) + bn <= bits(a);
}
const NativeGuiTextPrimaryPrefixLifetimeContext* prefix(const Context& c) noexcept { return c.geometry.prefix; }
bool untouched(const Name& n) noexcept {
    return !n.construct_entered && !n.construct_returned && !n.construct_failed &&
        !n.return_entered && !n.returned && !n.return_failed;
}
bool returned(const Name& n) noexcept {
    return n.construct_entered && n.construct_returned && !n.construct_failed &&
        n.return_entered && n.returned && !n.return_failed;
}
bool matches(const Slot& s, const Context& c, Selector::Phase phase) noexcept {
    return &s.context.fonts == &c.fonts &&
        s.frame.matches(s.context, s.input.header8, c.reference_frame, c.shader, s.acquired, phase) &&
        !s.acquired.failed() && s.frame.acquisition() == &s.acquired &&
        s.frame.prepared_input_header8() == s.input.header8 && region(s.input.header8, 8) &&
        !(bits(s.input.header8) & 3u);
}
bool aliases(const Context& c) noexcept {
    const auto* p = prefix(c);
    if (!p) return false;
    auto& f = p->text.factory_context;
    const auto& r = f.publication;
    const auto& strings = r.construction_context.sections.strings;
    return p->operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::geometry_only &&
        p->operation.geometry() == &c.geometry &&
        &f.reference_frame == &c.reference_frame && &r.construction == &f.construction &&
        &r.retirement.model == &f.construction.sections.model &&
        &r.retirement.resources == &r.construction_context.sections &&
        &r.retirement.base == &p->authentication.context() &&
        &r.construction_context.lifetime == &p->authentication.context() &&
        &p->widget.text == &p->text && &p->widget.authentication == &p->authentication &&
        &p->widget.widget == &p->authentication.context() &&
        &c.geometry.construction.primary == &p->widget.primary.record &&
        &c.geometry.construction.internal188 == &p->widget.internal188.record &&
        &p->producer_context.factory_context == &f && &p->producer_context.factory == &p->text.factory &&
        &p->producer_context.publication == &p->text.publication &&
        &p->producer_context.internal188 == &p->widget.internal188.record &&
        &p->producer_context.primary.actual == &p->widget.primary.record.actual &&
        &p->producer_context.primary.reference == &p->widget.primary.record.reference &&
        &p->producer_context.primary.construction == &p->widget.primary.record.construction &&
        c.geometry.frame.matches_backing(c.geometry.construction, c.geometry.arguments, c.geometry.position,
            c.geometry.transform_scratch, c.geometry.bounds_scratch) &&
        r.font_shader_frame == &c.shader && r.retirement.font_shader_frame == &c.shader &&
        &c.shader_context.fonts == &c.fonts && !c.shader_context.unknown_renderer48 &&
        c.shader.matches_credit_backing(c.shader_context, c.reference_frame) &&
        same_native_string_raw_pool_cells(c.initial.input.strings, strings) &&
        c.initial.input.storage == Name::Storage::caller_owned && !c.initial.frame.prior_header() &&
        (!c.second || (&c.second->frame != &c.initial.frame && &c.second->acquired != &c.initial.acquired &&
            &c.second->input != &c.initial.input &&
            (c.second->input.storage == Name::Storage::caller_owned || c.second->input.storage == Name::Storage::text_header) &&
            c.second->frame.prior_header() == c.initial.acquired.publication_storage() &&
            same_native_string_raw_pool_cells(c.second->input.strings, strings)));
}
bool selector_fresh(const Slot& s, const Context& c) noexcept {
    return matches(s, c, Selector::Phase::fresh) && untouched(s.input) &&
        !s.acquired.publication() && s.acquired.diagnostics.phase == NativeGuiTextFieldStorageAcquired::Phase::fresh &&
        s.acquired.diagnostics.entry == NativeGuiTextFieldStorageAcquired::Entry::none;
}
bool shader_fresh(const Context& c) noexcept {
    const auto p = c.shader.credit_progress();
    return p.producer == Shader::RawPhase::fresh && p.credit == Shader::CreditPhase::none &&
        p.consumer == Shader::Consumer::none && !p.failed && !p.active && !p.retirement_started &&
        !p.actual_text && !p.reference && !p.reference_context && !p.reference_frame && !p.cache &&
        c.shader.phase == Shader::Phase::fresh && c.shader.boundary == Shader::Boundary::none &&
        c.shader.escaped_boundary == Shader::Boundary::none && !c.shader.destination &&
        !c.shader.attempted && !c.shader.result && !c.shader.shader_published &&
        c.shader.cache.phase == NativeMaterialEffectCacheAcquired::Phase::not_started &&
        !c.shader.cache.caller_acquired && c.shader.cache.loader.phase == NativeMaterialEffectLoadPhase::not_started;
}
bool cache_quiescent(const Context& c) noexcept {
    // CPP records remain alive even after their native Effect returns. Never
    // resolve owner_record/companion/result or inspect native counters/payloads.
    for (const auto* q = &c.shader.cache; q; q = q->loader.fallback.get()) {
        if (q->phase != NativeMaterialEffectCacheAcquired::Phase::not_started &&
            q->phase != NativeMaterialEffectCacheAcquired::Phase::complete) return false;
        if (q->loader.phase != NativeMaterialEffectLoadPhase::not_started &&
            q->loader.phase != NativeMaterialEffectLoadPhase::complete) return false;
        if (q->key_resolution && q->key_resolution->phase != NativeMaterialEffectCacheKeyResolutionAcquired::Phase::fresh &&
            q->key_resolution->phase != NativeMaterialEffectCacheKeyResolutionAcquired::Phase::complete) return false;
    }
    return true;
}
bool publication(const Publication& q, const Slot& s, const Context& c,
    const void* actual, const Publication* previous) noexcept {
    const auto* p = prefix(c);
    return p && q.issued() && q.text_identity() == actual && q.header_identity() == offset(actual, 0x1c8) &&
        q.reference_context() == &p->text.factory_context.publication && q.reference_frame() == &c.reference_frame &&
        q.context() == &s.context && q.frame() == &s.frame && q.acquisition() == &s.acquired &&
        q.fonts() == &c.fonts && q.strings() &&
        same_native_string_raw_pool_cells(*q.strings(), s.input.strings) && q.previous() == previous &&
        q.length() != 0xffffffffu && (q.data_identity() ?
            region(q.data_identity(), std::size_t(q.length()) + 1u) : q.length() == 0);
}
// Only private COMPLETE/publication states choose a header. No pending/failure
// reaches an empty/default branch; equal-name carries its exact earlier proof.
bool header(const Context& c, const void* actual, const Publication*& current, bool& used) noexcept {
    current = nullptr; used = false;
    const auto one = [&](const Slot& s, const Publication* prior, bool initial) noexcept {
        if (selector_fresh(s, c)) return initial || prior != nullptr;
        if (!matches(s, c, Selector::Phase::complete) ||
            s.acquired.diagnostics.phase != NativeGuiTextFieldStorageAcquired::Phase::complete ||
            s.acquired.diagnostics.entry != NativeGuiTextFieldStorageAcquired::Entry::font_name ||
            s.frame.entry_header() != (initial ? Selector::HeaderEntry::constructor_empty : Selector::HeaderEntry::prior_normal))
            return false;
        const bool self = s.input.storage == Name::Storage::text_header;
        if (self ? (!prior || s.input.header8 != offset(actual, 0x1c8) || !untouched(s.input)) :
            (!returned(s.input) || !separate(s.input.header8, 8, actual, 0x1f8))) return false;
        used = true;
        const auto* q = s.acquired.publication();
        if (q) {
            if (!publication(*q, s, c, actual, prior) ||
                (q->origin() == Publication::Origin::inherited_self) != self) return false;
            current = q;
            return true;
        }
        if (!s.frame.normal_equal_name()) return false;
        return prior ? (s.frame.entry_header() == Selector::HeaderEntry::prior_normal) :
            (initial && s.frame.entry_header() == Selector::HeaderEntry::constructor_empty);
    };
    if (!one(c.initial, nullptr, true)) return false;
    if (!c.second) return true;
    if (selector_fresh(*c.second, c)) return true;
    const auto* first = c.initial.acquired.publication();
    return first && current == first && one(*c.second, first, false);
}
bool header_returned(const NativeGuiTextRetirementAcquired& d, const Publication* q) noexcept {
    const auto& s = d.strings[0];
    if (d.normal_actions_returned & bit6) {
        if (!(d.normal_actions_entered & bit6) || !s.data_captured ||
            s.captured_data != (q ? q->data_identity() : nullptr)) return false;
        return !s.captured_data || (q && s.length_captured && s.captured_bytes == q->length() + 1u &&
            s.getter_entered && s.getter_returned && s.return_entered && s.returned);
    }
    return (d.unwind_actions_consumed & bit6) && (d.unwind_actions_returned & bit6) && d.exception_cleanup_started;
}
bool header_entered(const NativeGuiTextRetirementAcquired& d) noexcept {
    return (d.normal_actions_entered & bit6) || (d.normal_actions_returned & bit6) ||
        (d.unwind_actions_consumed & bit6) || (d.unwind_actions_returned & bit6) ||
        d.strings[0].return_entered || d.strings[0].returned;
}
bool active_retirement(const Context& c, const Progress& p) noexcept {
    const auto* g = prefix(c);
    const auto& d = c.reference_frame.scalar.destruction;
    return g && p.active && p.active_consumer == Shader::Consumer::retirement &&
        p.retirement_started &&
        !p.active_font_context && !p.active_font_frame && !p.active_font_acquired &&
        p.active_retirement_context == &g->text.factory_context.publication.retirement &&
        p.active_retirement_acquired == &d && p.active_retirement_shader == &d.shader;
}
bool shader(const Context& c, const void* actual, const NativeGuiTextRawModelReference* reference,
    bool used, bool raw, bool post) noexcept {
    const auto p = c.shader.credit_progress();
    if (p.failed || p.producer == Shader::RawPhase::entered || p.producer == Shader::RawPhase::failed ||
        c.shader.phase == Shader::Phase::entered || c.shader.phase == Shader::Phase::failed || !cache_quiescent(c)) return false;
    if (raw ? !active_retirement(c, p) : p.active) return false;
    if (post ? !p.retirement_started : (!raw && p.retirement_started)) return false;
    const auto* g = prefix(c);
    const auto& r = g->text.factory_context.publication;
    if ((p.actual_text && p.actual_text != actual) || (p.reference && p.reference != reference) ||
        (p.reference_context && p.reference_context != &r) ||
        (p.reference_frame && p.reference_frame != &c.reference_frame) || (p.cache && p.cache != &c.shader.cache)) return false;
    if (p.producer == Shader::RawPhase::complete && (!used || p.actual_text != actual ||
        p.reference != reference || p.reference_context != &r || p.reference_frame != &c.reference_frame ||
        p.cache != &c.shader.cache)) return false;
    if (!used && p.producer == Shader::RawPhase::fresh &&
        (p.actual_text || p.reference || p.reference_context || p.reference_frame || p.cache)) return false;
    if (p.credit == Shader::CreditPhase::none) return !p.effect && p.consumer == Shader::Consumer::none;
    if (p.producer != Shader::RawPhase::complete || !p.effect) return false;
    if (p.credit == Shader::CreditPhase::outstanding)
        return !post && p.consumer == Shader::Consumer::none && !p.capture_entered;
    if (!p.capture_entered ||
        (p.credit == Shader::CreditPhase::returned && (!p.decrement_returned || !p.slot_cleared ||
            (p.decrement_result == 0 && (!p.zero_entered || !p.zero_returned))))) return false;
    if (p.credit == Shader::CreditPhase::reserved && (!raw || p.slot_cleared || p.consumer_returned)) return false;
    if (p.consumer == Shader::Consumer::font_name) {
        const auto is_slot = [&](const Slot& s) noexcept {
            return p.font_context == &s.context && p.font_frame == &s.frame && p.font_acquired == &s.acquired &&
                matches(s, c, Selector::Phase::complete) && !s.frame.normal_equal_name();
        };
        return p.credit == Shader::CreditPhase::returned && p.consumer_returned &&
            (is_slot(c.initial) || (c.second && is_slot(*c.second))) &&
            !p.retirement_context && !p.retirement_acquired && !p.retirement_shader;
    }
    const auto& d = c.reference_frame.scalar.destruction;
    return p.consumer == Shader::Consumer::retirement && p.retirement_context == &r.retirement &&
        p.retirement_acquired == &d && p.retirement_shader == &d.shader &&
        !p.font_context && !p.font_frame && !p.font_acquired && d.shader.identity == p.effect &&
        (raw || (post && p.credit == Shader::CreditPhase::returned && p.consumer_returned));
}
void append(NativeGuiTextFontLifetimeRegions& r, const void* p, std::size_t n) noexcept {
    if (!p || !n) return;
    if (r.count == r.entries.size()) std::terminate(); // bounded schema, never truncate.
    r.entries[r.count++] = {p, n};
}
bool calls_settled(const NativeGuiTextPrimaryPrefixCalls& s) noexcept {
    using P = NativeModelStorageNodeCall::Phase;
    if (!s.data && s.count) return false;
    for (std::size_t i = 0; i < s.count; ++i)
        if (s.data[i].phase == P::entered || s.data[i].phase == P::failed) return false;
    return true;
}
bool route_settled(const NativeGuiWidgetRawModelRoute& r, const NativeGuiTextPrimaryPrefixCalls& calls) noexcept {
    using P = NativeModelStorageDispatchCallAcquired::Phase;
    return r.tree_acquired.bound && !r.tree_acquired.poisoned &&
        r.tree_acquired.current18.phase == P::complete && r.tree_acquired.current0.phase == P::complete &&
        r.tree_acquired.current04_entered && r.tree_acquired.current04_returned &&
        r.node_acquired.bound && !r.node_acquired.poisoned && !r.node_acquired.entry_failed &&
        !r.node_acquired.capacity_failed && r.node_acquired.used <= calls.count && calls_settled(calls);
}
bool prebody(const NativeGuiTextRetirementAcquired& d) noexcept {
    return !d.started && !d.complete && !d.destination && !d.exception_cleanup_started &&
        d.native_eh_state == -1 && !d.base_entered && !d.base_returned &&
        !d.normal_actions_entered && !d.normal_actions_returned && !d.unwind_actions_consumed &&
        !d.unwind_actions_returned && !d.shader.identity && !d.shader.decrement_entered &&
        !d.shadow.started && !d.glyphs.started;
}
} // namespace

NativeGuiTextFontLifetimeStorageFrame::NativeGuiTextFontLifetimeStorageFrame(const Context& c) noexcept : context_(c) {}
NativeGuiTextFontLifetimeStorageFrame::~NativeGuiTextFontLifetimeStorageFrame() noexcept {
    if (active_ || phase_ == Phase::failed ||
        (phase_ == Phase::sealed && !post_text_quiescent())) std::terminate();
}
void NativeGuiTextFontLifetimeStorageFrame::enter() {
    require(!active_ && phase_ != Phase::failed, "Font lifetime checker cannot reenter or resume a failed graph");
    active_ = true;
}
void NativeGuiTextFontLifetimeStorageFrame::leave() {
    require(active_ && phase_ != Phase::failed && !failure_, "Caught Font lifetime rejection cannot resume outer admission");
    active_ = false;
}
void NativeGuiTextFontLifetimeStorageFrame::fail() noexcept {
    if (!failure_) failure_ = std::current_exception();
    phase_ = Phase::failed;
    active_ = false;
}
void NativeGuiTextFontLifetimeStorageFrame::prepare_before_text_allocation() {
    try {
        enter();
        require(phase_ == Phase::fresh && !actual_ && !reference_ && aliases(context_),
            "Font lifetime preparation needs one exact unentered prefix/geometry/selector/shader graph");
        const auto& c = context_;
        const auto& p = *prefix(c);
        const auto& f = p.text.factory_context;
        const auto& a = p.text.factory;
        const auto& s = p.text.publication;
        require(f.publication.retirement.font_lifetime == this &&
            selector_fresh(c.initial, c) && (!c.second || selector_fresh(*c.second, c)) && shader_fresh(c) &&
            a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::fresh && !a.started && !a.complete &&
            !a.raw_slot && !a.failure && !f.construction.started &&
            !s.bind_entered && !s.bind_complete && !s.reference && !s.scalar_entered &&
            !s.companion_disposal_entered && !s.disposed &&
            !c.reference_frame.scalar.started && !c.reference_frame.scalar.complete &&
            !c.reference_frame.scalar.destructor_entered && !c.reference_frame.scalar.destructor_returned &&
            !c.reference_frame.scalar.flags_captured && !c.reference_frame.scalar.pool_return_entered &&
            !c.reference_frame.scalar.pool_returned && prebody(c.reference_frame.scalar.destruction) &&
            p.authentication_acquired.bound && !p.authentication_acquired.active && !p.authentication_acquired.poisoned &&
            c.geometry.frame.phase() == NativeGuiTextMainGeometryStoragePhase::fresh &&
            !c.geometry.progress.entered && !c.geometry.progress.returned && !c.geometry.progress.failed &&
            c.fonts.native_registry_image(),
            "Pre-pool Font reservation requires untouched selectors/inputs/shader and real ready image collection");
        phase_ = Phase::prepared;
        leave();
    } catch (...) { fail(); throw; }
}
void NativeGuiTextFontLifetimeStorageFrame::seal_after_prefix_geometry(void* actual) {
    try {
        enter();
        require(phase_ == Phase::prepared && !actual_ && !reference_ && aliases(context_) &&
            selector_fresh(context_.initial, context_) && (!context_.second || selector_fresh(*context_.second, context_)) &&
            shader_fresh(context_), "Font lifetime sealing precedes all selector/input/shader effects");
        const auto& p = *prefix(context_);
        require(actual && p.text.factory.raw_slot == actual &&
            p.text.factory_context.publication.retirement.font_lifetime == this &&
            context_.geometry.frame.matches(context_.geometry.construction, context_.geometry.acquired,
                NativeGuiTextMainGeometryStoragePhase::complete) &&
            context_.geometry.acquired.phase == NativeGuiTextMainGeometryStoragePhase::complete &&
            context_.geometry.acquired.destination == actual && !context_.geometry.acquired.failure &&
            context_.geometry.progress.entered && context_.geometry.progress.returned &&
            !context_.geometry.progress.failed,
            "Font lifetime sealing requires genuine completed prefix and geometry producers");
        const auto* const produced = p.producer.publication();
        require(produced && p.producer_frame.matches(p.producer_context, p.producer,
            NativeGuiTextPrimaryModelStoragePhase::complete) &&
            produced->text_identity() == actual && produced->model_identity() == context_.geometry.acquired.captured_primary &&
            produced->factory_context() == &p.text.factory_context && produced->factory_acquisition() == &p.text.factory &&
            produced->primary_record() == &p.producer_context.primary &&
            produced->primary_dispatch_record() == &context_.geometry.construction.primary &&
            produced->construction() == &p.producer_context.primary.construction &&
            produced->pool() == p.producer.actual_pool &&
            p.name.construct_entered && p.name.construct_returned && !p.name.construct_failed &&
            p.name.return_entered && p.name.returned && !p.name.return_failed,
            "Font lifetime seals only the genuine completed private primary publication, never DEFAULT");
        // No full prefix/shape helper here: Root's ordinary shape may delegate
        // its Font slice back to this checker. Authenticate exact record only.
        p.authentication.require_raw_model_transport_receiver(actual, p.text);
        auto* const reference = p.text.publication.reference;
        require(reference && reference->matches_reference_frame(context_.reference_frame) &&
            reference->storage() == actual && &reference->storage_context() == &p.text.factory_context.publication,
            "Font lifetime seal requires canonical122's exact hidden constructor Frame");
        require(!read(actual, 0x108) && !read(actual, 0x1c8) && !read(actual, 0x1cc) && !read(actual, 0x1ec),
            "Font lifetime seal precedes actual Font/header/shader publication");
        require(separate(context_.initial.input.header8, 8, actual, 0x1f8) &&
            (!context_.second || (context_.second->input.storage == Name::Storage::text_header ?
                context_.second->input.header8 == offset(actual, 0x1c8) :
                separate(context_.second->input.header8, 8, actual, 0x1f8))),
            "Caller Font input storage cannot alias Text; borrowed self is an exact separate annotation");
        actual_ = actual;
        reference_ = reference;
        phase_ = Phase::sealed;
        leave();
    } catch (...) { fail(); throw; }
}
void NativeGuiTextFontLifetimeStorageFrame::admit(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) {
    require(phase_ == Phase::sealed && actual && actual == actual_ && reference_ && aliases(context_),
        "Font lifetime admission needs its previously sealed canonical live Text graph");
    const auto& c = context_;
    const auto& p = *prefix(c);
    const auto& f = p.text.factory_context;
    const auto& pub = p.text.publication;
    const auto& scalar = c.reference_frame.scalar;
    const auto& d = scalar.destruction;
    const bool raw = stage == Stage::raw118_preentry || stage == Stage::raw118_body || stage == Stage::base;
    require(f.publication.retirement.font_lifetime == this &&
        !pub.bind_failed && pub.bind_complete && pub.reference == reference_ &&
        !pub.scalar_failed && !pub.scalar_returned && !pub.companion_disposal_entered && !pub.disposed &&
        !scalar.complete && !scalar.destructor_returned && !scalar.pool_returned && !d.complete && !d.base_returned &&
        p.text.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete &&
        p.text.factory.complete && !p.text.factory.failure &&
        !p.authentication_acquired.poisoned && !p.authentication_acquired.active,
        "Font lifetime independent records reject failed/disposing/returned objects BEFORE pointer access");
    const Publication* q = nullptr;
    bool used = false;
    require(header(c, actual, q, used) && shader(c, actual, reference_, used, raw, false),
        "Pending/failed/foreign selector or shader cannot fall back to earlier Font state");
    if (raw) require(progress == &d && pub.scalar_entered && scalar.started && scalar.destructor_entered,
        "Raw118 Font stage requires the exact canonical active destruction record");
    else require(!progress, "Non-raw Font stage cannot accept an arbitrary destruction record");
    if (stage != Stage::raw118_body && stage != Stage::base)
        require(prebody(d) && !scalar.flags_captured && !scalar.pool_return_entered,
            "Pre-body Font stage excludes returned/entered member or late-flags history");
    switch (stage) {
    case Stage::bound:
    case Stage::before_scalar:
        require(!pub.scalar_entered && !scalar.started && !scalar.destructor_entered && !d.started,
            "Bound Font permission precedes real owner and canonical scalar entry");
        break;
    case Stage::scalar_preentry:
        require(pub.scalar_entered && !scalar.started && !scalar.destructor_entered && !d.started,
            "Scalar preentry precedes scalar flags and private raw118 token entry");
        break;
    case Stage::raw118_preentry:
        require(!d.started && !d.destination && !d.base_entered && d.native_eh_state == -1,
            "Raw118 preentry has an active private token but no destructor body yet");
        break;
    case Stage::raw118_body:
        require(d.started && d.destination == actual && !d.base_entered,
            "Raw118 Font body needs actual entry and cannot replay a base boundary");
        break;
    case Stage::base:
        require(d.started && d.destination == actual && d.base_entered && d.native_eh_state == -1 &&
            (d.normal_actions_returned & 0x7eu) == 0x7eu,
            "Font base boundary follows the real normal member returns");
        break;
    }
    const bool ended_header = header_entered(d);
    if (ended_header || stage == Stage::base) {
        require(header_returned(d, q), "Entered1C8 return must actually RETURN before any later metadata admission");
        return; // No derived Text, Font, Effect, header words/data, registry or companion lookup.
    }
    // The sealed companion is still alive by the independent guards above.
    const auto phase = reference_->phase();
    require(reference_->storage() == actual && &reference_->storage_context() == &f.publication &&
        reference_->matches_reference_frame(c.reference_frame) &&
        (stage == Stage::bound || stage == Stage::before_scalar ?
            phase == NativeGuiTextRawModelReference::Phase::bound :
            phase == NativeGuiTextRawModelReference::Phase::retiring),
        "Live Font stage cannot replace its canonical122 generation");
    p.authentication.require_raw_model_transport_receiver(actual, p.text); // exact record BEFORE first payload read.
    const auto credit = c.shader.credit_progress();
    const void* expected_effect = credit.credit == Shader::CreditPhase::outstanding ||
        credit.credit == Shader::CreditPhase::reserved ? credit.effect : nullptr;
    require(read(actual, 0x108) == bits(q ? q->font_identity() : nullptr) &&
        read(actual, 0x1c8) == (q ? q->length() : 0u) &&
        read(actual, 0x1cc) == bits(q ? q->data_identity() : nullptr) &&
        read(actual, 0x1ec) == bits(expected_effect) && (!q || read(actual, 0x1d4) == q->alpha_bits()),
        "Live Font/header/effect fields must match their private normal publications and exact returned credit");
    if (q && q->font_identity()) (void)c.fonts.resolve_image(q->font_identity()); // ready live collection only.
}
void NativeGuiTextFontLifetimeStorageFrame::require_live(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) {
    try { enter(); admit(actual, stage, progress); leave(); }
    catch (...) { fail(); throw; }
}
NativeGuiTextFontLifetimeRegions NativeGuiTextFontLifetimeStorageFrame::metadata_regions() const noexcept {
    NativeGuiTextFontLifetimeRegions r;
    const auto& c = context_;
    append(r, this, sizeof(*this)); append(r, &c, sizeof(c)); append(r, &c.geometry, sizeof(c.geometry));
    append(r, &c.fonts, sizeof(c.fonts)); append(r, &c.shader_context, sizeof(c.shader_context));
    append(r, &c.shader, sizeof(c.shader)); append(r, &c.reference_frame, sizeof(c.reference_frame));
    const auto add_slot = [&](const Slot& s) noexcept {
        append(r, &s, sizeof(s)); append(r, &s.context, sizeof(s.context)); append(r, &s.frame, sizeof(s.frame));
        append(r, &s.acquired, sizeof(s.acquired));
        append(r, s.acquired.publication_storage(), sizeof(Publication));
        append(r, &s.frame.body_frame(), sizeof(NativeGuiTextFieldStorageFrame));
        append(r, &s.input, sizeof(s.input)); append(r, &s.input.strings, sizeof(s.input.strings));
        if (s.input.storage == Name::Storage::caller_owned) append(r, s.input.header8, 8);
    };
    add_slot(c.initial); if (c.second) add_slot(*c.second);
    if (const auto* p = prefix(c)) {
        append(r, p, sizeof(*p)); append(r, &p->text, sizeof(p->text));
        append(r, &p->text.factory_context, sizeof(p->text.factory_context));
        append(r, &p->text.factory_context.publication, sizeof(p->text.factory_context.publication));
        append(r, &p->authentication, sizeof(p->authentication));
        append(r, &p->authentication_acquired, sizeof(p->authentication_acquired));
        append(r, &p->widget, sizeof(p->widget)); append(r, &p->widget_acquired, sizeof(p->widget_acquired));
        append(r, &p->producer_frame, sizeof(p->producer_frame)); append(r, &p->producer, sizeof(p->producer));
        append(r, &p->name, sizeof(p->name)); append(r, p->name.header8, 8);
    }
    append(r, &c.geometry.frame, sizeof(c.geometry.frame)); append(r, &c.geometry.acquired, sizeof(c.geometry.acquired));
    append(r, &c.geometry.progress, sizeof(c.geometry.progress));
    return r;
}
NativeGuiTextFontLifetimeRegions NativeGuiTextFontLifetimeStorageFrame::live_regions(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) {
    try {
        enter(); admit(actual, stage, progress);
        auto r = metadata_regions();
        const auto& d = context_.reference_frame.scalar.destruction;
        if (stage != Stage::base && !header_entered(d)) {
            const Publication* q = nullptr; bool used = false;
            require(header(context_, actual, q, used), "Live Font region view lost exact private header publication");
            if (q) {
                if (q->data_identity()) append(r, q->data_identity(), std::size_t(q->length()) + 1u);
                if (q->font_identity()) append(r, q->font_identity(), 0xacu); // real actual Font allocation, never04 count.
            }
        }
        leave(); return r;
    } catch (...) { fail(); throw; }
}
bool NativeGuiTextFontLifetimeStorageFrame::post_text_quiescent() const noexcept {
    const auto& c = context_;
    if (active_ || failure_ || phase_ == Phase::failed || !aliases(c)) return false;
    const auto& p = *prefix(c);
    const auto& f = p.text.factory_context;
    const auto& a = p.text.factory;
    const auto& s = p.text.publication;
    if (f.publication.retirement.font_lifetime != this) return false;
    if (phase_ == Phase::prepared && !actual_ && !reference_) {
        // Absence of Font debt ONLY. The factory/93/Models may already exist or
        // be partial: their independent owner guards still govern retirement.
        return selector_fresh(c.initial, c) && (!c.second || selector_fresh(*c.second, c)) && shader_fresh(c);
    }
    if (phase_ != Phase::sealed || !actual_ || !reference_ || s.reference != reference_ ||
        a.phase != NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete || !a.complete || a.failure ||
        !s.bind_complete || s.bind_failed || !s.scalar_entered || !s.scalar_returned || s.scalar_failed ||
        !s.companion_disposal_entered || !s.disposed) return false;
    const auto& scalar = c.reference_frame.scalar;
    const auto& d = scalar.destruction;
    const Publication* q = nullptr; bool used = false;
    if (!scalar.started || !scalar.complete || !scalar.destructor_entered || !scalar.destructor_returned ||
        !scalar.flags_captured || !(scalar.captured_flags_byte & 1u) || !scalar.pool_return_entered || !scalar.pool_returned ||
        !d.started || !d.complete || d.destination != actual_ || d.exception_cleanup_started ||
        !d.base_entered || !d.base_returned || !header(c, actual_, q, used) || !header_returned(d, q) ||
        !shader(c, actual_, reference_, used, false, true) ||
        p.authentication_acquired.active || p.authentication_acquired.poisoned ||
        p.widget_acquired.active || p.widget_acquired.poisoned ||
        p.widget_acquired.base != NativeGuiWidgetRawModelLifetimeAcquired::Phase::complete ||
        p.widget_acquired.scene != NativeGuiWidgetRawModelLifetimeAcquired::Phase::complete ||
        p.widget_acquired.glyph != NativeGuiWidgetRawModelLifetimeAcquired::Phase::fresh ||
        !p.widget_acquired.scene4c.entered || !p.widget_acquired.scene4c.returned ||
        !p.widget_acquired.scene4c.unlink.complete ||
        (p.widget_acquired.late4c.entered && (!p.widget_acquired.late4c.returned || !p.widget_acquired.late4c.unlink.complete)) ||
        !route_settled(p.widget.primary, p.primary_calls) ||
        !route_settled(p.widget.internal188, p.internal_calls)) return false;
    return native_gui_text_prefix_geometry_meshes_settled(p); // durable Model/Mesh records only.
}
void NativeGuiTextFontLifetimeStorageFrame::require_post_text_quiescent() {
    // Do not enter the guard while evaluating this noexcept independent predicate.
    if (!post_text_quiescent()) {
        try { throw std::invalid_argument("Font retirement requires complete Text/Models/header/credit/input custody"); }
        catch (...) { fail(); throw; }
    }
}
bool NativeGuiTextFontLifetimeStorageFrame::requires_process_retention() const noexcept {
    if (active_ || phase_ == Phase::failed || failure_) return true;
    const auto p = context_.shader.credit_progress();
    if (p.failed || p.active || p.producer == Shader::RawPhase::entered || p.producer == Shader::RawPhase::failed ||
        context_.shader.phase == Shader::Phase::entered || context_.shader.phase == Shader::Phase::failed ||
        !cache_quiescent(context_)) return true;
    const auto pending = [](const Slot& s) noexcept {
        return s.acquired.failed() || s.frame.phase() == Selector::Phase::entered ||
            s.frame.phase() == Selector::Phase::failed ||
            s.acquired.diagnostics.phase == NativeGuiTextFieldStorageAcquired::Phase::entered ||
            s.acquired.diagnostics.phase == NativeGuiTextFieldStorageAcquired::Phase::failed ||
            s.input.construct_failed || s.input.return_failed ||
            (s.input.construct_entered && !s.input.returned);
    };
    if (pending(context_.initial) || (context_.second && pending(*context_.second))) return true;
    if (phase_ == Phase::sealed) {
        const auto* g = prefix(context_);
        if (!g) return true;
        const auto& scalar = context_.reference_frame.scalar;
        const auto bad_route = [](const NativeGuiWidgetRawModelRoute& r) noexcept {
            return r.tree_acquired.poisoned || r.node_acquired.poisoned || r.node_acquired.entry_failed ||
                r.node_acquired.capacity_failed || (r.record.lifetime.started && !r.record.lifetime.complete);
        };
        if (g->text.publication.bind_failed || g->text.publication.scalar_failed ||
            (scalar.started && !scalar.complete) || g->authentication_acquired.active ||
            g->authentication_acquired.poisoned || g->widget_acquired.active || g->widget_acquired.poisoned ||
            bad_route(g->widget.primary) || bad_route(g->widget.internal188) ||
            !calls_settled(g->primary_calls) || !calls_settled(g->internal_calls)) return true;
    }
    return false; // Healthy bound readiness is NEVER post-Text/Font retirement permission.
}
} // namespace bsp
