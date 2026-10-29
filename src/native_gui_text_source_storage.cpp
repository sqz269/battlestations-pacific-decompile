#include "bsp/native_gui_text_source_storage.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include <cstddef>
#include <cstring>
#include <initializer_list>
#include <stdexcept>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextSourceStorageContext;
using Frame = NativeGuiTextSourceStorageFrame;
using Acquired = NativeGuiTextSourceStorageAcquired;
using Progress = NativeGuiTextSourceInputProgress;
static_assert(sizeof(void*) == 4, "raw Text source storage requires Win32");
// SAME protected Text span used by existing raw122 admission, not new pool or
// allocator/body credit. This leaf never writes the native span.
constexpr std::size_t text_span = 0x1f8;
struct Region { const volatile void* begin; std::size_t bytes; };
std::uintptr_t address(const volatile void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
const void* at(const void* p, std::size_t offset) noexcept {
    return reinterpret_cast<const void*>(address(p) + offset);
}
bool representable(Region r) noexcept {
    return r.begin && r.bytes && static_cast<std::uint64_t>(address(r.begin)) + r.bytes <= 0x100000000ull;
}
bool disjoint(Region a, Region b) noexcept {
    const auto x = static_cast<std::uint64_t>(address(a.begin));
    const auto y = static_cast<std::uint64_t>(address(b.begin));
    return x + a.bytes <= y || y + b.bytes <= x;
}
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
Word read(const void* p, std::size_t offset = 0) noexcept {
    Word value;
    std::memcpy(&value, at(p, offset), sizeof value);
    return value;
}
void caller_regions(const Context& c, const Frame& frame, const Acquired& acquired,
    const Progress& input, const NativeGuiTextReferenceFrame& reference_frame, const void* header) {
    const Region writes[] = {{&frame, sizeof frame}, {&acquired, sizeof acquired}};
    const Region source{header, 8};
    require(representable(writes[0]) && representable(writes[1]) &&
        disjoint(writes[0], writes[1]) && representable(source) && !(address(header) & 3u),
        "Source-equal needs separate valid Frame/Acquired and aligned actual input8B");
    auto& f = c.text.factory_context;
    auto& r = f.publication;
    auto& resources = r.retirement.resources;
    for (const Region borrowed : {
        Region{&c, sizeof c}, {&c.text, sizeof c.text}, {&c.widgets, sizeof c.widgets},
        {&c.strings, sizeof c.strings}, {&input, sizeof input}, {&input.strings, sizeof input.strings},
        {&reference_frame, sizeof reference_frame}, {&f, sizeof f}, {&c.text.factory, sizeof c.text.factory},
        {&c.text.publication, sizeof c.text.publication}, {&r, sizeof r},
        {&r.construction_context, sizeof r.construction_context}, {&r.construction_frame, sizeof r.construction_frame},
        {&r.construction, sizeof r.construction}, {&r.retirement, sizeof r.retirement},
        {&resources, sizeof resources}, {&resources.model, sizeof resources.model},
        {&resources.association, sizeof resources.association}, {&resources.parenting, sizeof resources.parenting},
        {&resources.strings, sizeof resources.strings}, {&r.retirement.model, sizeof r.retirement.model},
        {&c.strings.actual_published_01090aa8, sizeof(c.strings.actual_published_01090aa8)},
        {&c.strings.actual_small_returns_disabled_01090aa4, sizeof(c.strings.actual_small_returns_disabled_01090aa4)},
        {&c.strings.actual_manager_publication_01090aa0, sizeof(c.strings.actual_manager_publication_01090aa0)}}) {
        require(representable(borrowed), "Source-equal retained control region has an invalid extent");
        for (const auto writable : writes)
            require(disjoint(writable, borrowed), "Source-equal metadata cannot overwrite its producer/input controls");
        require(disjoint(source, borrowed), "Source-equal actual input cannot alias its retained control graph");
    }
    for (const auto writable : writes)
        require(disjoint(writable, source), "Source-equal input cannot alias Frame/Acquired writes");
    // Addresses from retained factory metadata ONLY: no saved companion method
    // or native payload read precedes the independent ended-history fences.
    const Region native{c.text.factory.raw_slot, text_span};
    const Region companion{f.companion_storage, f.companion_storage_bytes};
    require(representable(native) && representable(companion),
        "Source-equal needs the retained factory Text/companion extents");
    for (const auto writable : writes)
        require(disjoint(writable, native) && disjoint(writable, companion),
            "Source-equal metadata cannot overwrite native Text/companion storage");
    require(disjoint(source, native) && disjoint(source, companion),
        "Source-equal caller input cannot alias native Text/companion storage");
    if (f.disposal.context) {
        const Region disposal{f.disposal.context, f.disposal.context_bytes};
        require(representable(disposal) && disjoint(source, disposal),
            "Source-equal input cannot alias its saved disposal backing");
        for (const auto writable : writes)
            require(disjoint(writable, disposal), "Source-equal records cannot alias saved disposal backing");
    } else require(!f.disposal.context_bytes, "Source-equal disposal extent lacks storage");
    // Opaque allocations behind these objects and the owner's later flags cell
    // are NOT discovered here. Their disjoint live storage is a caller contract.
}
void bound_markers(const Context& c, const NativeGuiTextReferenceFrame& expected) {
    const auto& f = c.text.factory_context;
    const auto& a = c.text.factory;
    const auto& p = c.text.publication;
    const auto& construction = f.construction;
    const auto& scalar = f.reference_frame.scalar;
    // Independent C++ history FIRST: never follow a disposed diagnostic Ref.
    require(p.bind_entered && p.bind_complete && !p.bind_failed && p.reference &&
        !p.scalar_entered && !p.scalar_returned && !p.scalar_failed &&
        !p.companion_disposal_entered && !p.disposed &&
        !scalar.started && !scalar.complete && !scalar.destructor_entered && !scalar.destructor_returned &&
        !scalar.pool_returned && !scalar.destruction.started && !scalar.destruction.complete &&
        !scalar.destruction.base_entered && !scalar.destruction.base_returned &&
        !construction.exception_cleanup_started && !construction.base_lifetime_ended &&
        !construction.consumed_cleanup_actions && construction.failure == decltype(construction.failure)::none,
        "Source-equal rejects constructor/scalar/failed/disposing/ended receiver history");
    require(&f.reference_frame == &expected && a.started && a.complete && !a.failure &&
        a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete &&
        a.allocation_entered && a.allocation_returned && a.constructor_entered && a.constructor_returned &&
        a.prepared_context == &f && a.construction_record == &construction && a.raw_slot &&
        a.constructor_result == a.raw_slot && a.actual_pool == &f.publication.retirement.actual_pool_00f8bdf0 &&
        construction.started && construction.complete && construction.base_complete && construction.derived_complete &&
        construction.sections_entered && construction.destination == a.raw_slot && construction.native_eh_state == 6 &&
        construction.sections.complete && !construction.sections.exception_cleanup_started &&
        construction.sections.failure == decltype(construction.sections.failure)::none &&
        construction.sections.model_constructed && construction.sections.shadow_published && construction.sections.model_registered,
        "Source-equal requires SAME completed factory93/85 and private canonical122 Frame");
}
void* canonical_receiver(NativeGuiTextRawModelReference& reference, Context& c,
    const NativeGuiTextReferenceFrame& expected, const Frame& frame, const Acquired& acquired, const void* input) {
    bound_markers(c, expected);
    auto& f = c.text.factory_context;
    auto& r = f.publication;
    require(c.text.publication.reference == &reference && f.companion_storage == &reference &&
        f.companion_storage_bytes >= sizeof reference && reference.phase() == NativeGuiTextRawModelReference::Phase::bound &&
        reference.matches_reference_frame(expected) && &reference.storage_context() == &r,
        "Source-equal requires its SAME live bound companion, never a legacy/scalar prefix");
    void* const raw = reference.storage();
    const Region native{raw, text_span}, companion{&reference, sizeof reference};
    require(raw == c.text.factory.raw_slot && representable(native) && !(address(raw) & 3u) &&
        representable(companion) && disjoint(native, companion) &&
        r.owners.find(raw) == &reference && static_cast<const void*>(&reference.reference_count) == at(raw, 4),
        "Source-equal requires actual Text/count CELL identity and SAME canonical registry, never another owner");
    for (const Region caller : {Region{&frame, sizeof frame}, {&acquired, sizeof acquired}, {input, 8}})
        require(disjoint(caller, native) && disjoint(caller, companion),
            "Source-equal records/input must be independent of native Text and companion storage");
    require(&r.construction == &f.construction && &r.construction_context.sections == &r.retirement.resources &&
        &r.construction.sections.model == &r.retirement.model &&
        &r.construction_context.lifetime == &c.widgets.context() && &r.retirement.base == &c.widgets.context() &&
        &r.retirement.base.bindings == &c.widgets &&
        same_native_string_raw_pool_cells(c.strings, r.retirement.resources.strings),
        "Source-equal requires SAME raw93/widget/resource/string provider graph");
    const auto& resources = r.retirement.resources;
    const auto& model = r.retirement.model;
    require(!model.lifetime.started && !model.lifetime.scalar_started && !model.release.started &&
        model.actual && model.reference && !model.reference->retired() &&
        model.construction.complete && model.construction.metadata_bound &&
        model.construction.actual_model == model.actual && model.construction.context == &resources.model.storage &&
        model.association.phase == NativeModelStorageGeometryPhase::complete &&
        model.association.context == &resources.association &&
        model.association.frame == &r.construction_frame.sections.association &&
        &resources.geometry.actual_owners() == &r.owners && r.owners.find(model.actual) == &*model.reference,
        "Source-equal requires SAME live completed internal188 producer/geometry/owner records");
    c.widgets.require_raw_model_transport_receiver(raw, c.text);
    bound_markers(c, expected); // raw transport auth also admits other phases: fence bound ONLY again
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound &&
        c.text.publication.reference == &reference && reference.storage() == raw &&
        r.owners.find(raw) == &reference && read(raw) == 0x00d5c6c8u &&
        read(raw, 0x188) == address(model.actual),
        "Source-equal cannot resume after a caught callback/lifetime transition or foreign current Text profile");
    return raw;
}
bool live_input(const Progress& p) noexcept {
    return p.construct_entered && p.construct_returned && !p.construct_failed &&
        !p.return_entered && !p.returned && !p.return_failed;
}
} // namespace

NativeGuiTextSourceStorageFrame::NativeGuiTextSourceStorageFrame(const Context& context,
    const void* input, const Progress& progress, const NativeGuiTextReferenceFrame& reference,
    Acquired& acquired) noexcept
    : context_(&context), input_(input), input_progress_(&progress), reference_frame_(&reference), acquired_(&acquired) {}
NativeGuiTextSourceStorageFrame::~NativeGuiTextSourceStorageFrame() noexcept {
    if (requires_process_retention()) std::terminate();
}
bool NativeGuiTextSourceStorageFrame::matches(const Context& context, const void* input,
    const Progress& progress, const NativeGuiTextReferenceFrame& reference, const Acquired& acquired,
    Phase phase) const noexcept {
    return context_ == &context && input_ == input && input_progress_ == &progress &&
        reference_frame_ == &reference && acquired_ == &acquired && phase_ == phase;
}

NativeGuiTextSourceStorageResult submit_native_gui_text_source_unchanged_storage_00abaed0(
    NativeGuiTextRawModelReference& reference, Context& context, Frame& frame, Acquired& acquired) {
    const bool fresh = frame.phase_ == Frame::Phase::fresh && !frame.active_ && !frame.failure_;
    frame.active_ = true;
    frame.phase_ = Frame::Phase::entered; // BEFORE any selected-path validation
    frame.entered_ = true;
    bool acquired_writable = false;
    bool prepared_acquired_writable = false;
    auto failure_boundary = Acquired::Boundary::admission;
    try {
        require(frame.context_ && frame.input_progress_ && frame.reference_frame_ && frame.acquired_,
            "Source-equal prepared Frame lacks its fixed metadata backing");
        const auto& progress = *frame.input_progress_;
        // Pinned C++ backings must still be alive even for a rejected tuple.
        // These checks use only addresses/sizes, never input/native payload.
        caller_regions(*frame.context_, frame, acquired, progress, *frame.reference_frame_, frame.input_);
        acquired_writable = true; // bad metadata/input-control aliases were rejected without scribbling Acquired
        caller_regions(*frame.context_, frame, *frame.acquired_, progress, *frame.reference_frame_, frame.input_);
        prepared_acquired_writable = true;
        require(fresh && frame.context_ == &context && frame.acquired_ == &acquired,
            "Source-equal rejects replaced, replayed, reentered or failed prepared Frame/backing");
        require(!acquired.bound_frame_ && !acquired.active_ && !acquired.failed_ && !acquired.unchanged_ &&
            !acquired.diagnostics.started && !acquired.diagnostics.comparison_entered &&
            !acquired.diagnostics.comparison_returned && !acquired.diagnostics.equal && !acquired.diagnostics.complete &&
            !acquired.diagnostics.text && !acquired.diagnostics.input && !acquired.diagnostics.native_site &&
            !acquired.diagnostics.failure && acquired.diagnostics.boundary == Acquired::Boundary::none &&
            acquired.diagnostics.escaped_boundary == Acquired::Boundary::none,
            "Source-equal needs one fresh Acquired, never mutable diagnostics as completion authority");
        acquired.bound_frame_ = &frame;
        acquired.active_ = true;
        auto& a = acquired.diagnostics;
        a.started = true;
        a.boundary = Acquired::Boundary::admission;
        a.input = frame.input_;
        require(progress.header8 == frame.input_ && live_input(progress) &&
            same_native_string_raw_pool_cells(context.strings, progress.strings),
            "Source-equal requires SAME genuinely constructed live caller input8B/pool history, not returned/header projection");
        void* const raw = canonical_receiver(reference, context, *frame.reference_frame_, frame, acquired, frame.input_);
        a.text = raw;
        require(frame.active_ && frame.phase_ == Frame::Phase::entered && !frame.failure_ &&
            acquired.active_ && !acquired.failed_ && acquired.bound_frame_ == &frame && live_input(progress),
            "Source-equal cannot resume a caught nested failure or input return");
        // Admission only, not native comparison epochs: retain the initial
        // constructor-owned empty cache. No F8 pointer is ever followed.
        require(read(raw, 0xf4) == 0 && read(raw, 0xf8) == 0,
            "Source-equal excludes nonempty/foreign cache without a real raw F4 publisher receipt");
        a.boundary = Acquired::Boundary::comparison;
        failure_boundary = Acquired::Boundary::comparison;
        a.native_site = 0x00abaefau;
        a.comparison_entered = true;
        frame.comparison_entered_ = true;
        const bool equal = equal_native_string_headers_00435c40(at(raw, 0xf4), frame.input_); // ONE actual native-schedule port
        a.comparison_returned = true;
        frame.comparison_returned_ = true;
        a.equal = equal;
        require(frame.active_ && frame.phase_ == Frame::Phase::entered && !frame.failure_ &&
            acquired.active_ && !acquired.failed_ && acquired.bound_frame_ == &frame,
            "Source-equal comparison cannot seal after a caught nested failure");
        if (!equal) {
            a.boundary = Acquired::Boundary::unsupported_changed;
            failure_boundary = Acquired::Boundary::unsupported_changed;
            a.native_site = 0x00abaf07u; // reject BEFORE this native changed arm; no flag/cache/current50 action
            throw std::logic_error("raw ABAED0 changed source is unsupported; retain caller/input/Text graph");
        }
        a.boundary = Acquired::Boundary::normal_return;
        a.native_site = 0x00abafe5u;
        a.complete = true;
        acquired.unchanged_ = true;
        acquired.active_ = false;
        frame.active_ = false;
        frame.unchanged_ = true;
        frame.phase_ = Frame::Phase::unchanged; // private normal-only seal, no TextF4 ownership is issued
        return NativeGuiTextSourceStorageResult::unchanged;
    } catch (...) {
        if (!frame.failure_) frame.failure_ = std::current_exception();
        frame.active_ = false;
        frame.phase_ = Frame::Phase::failed;
        const auto poison = [failure_boundary](Acquired& target) noexcept {
            target.failed_ = true;
            target.active_ = false;
            target.diagnostics.escaped_boundary = failure_boundary;
            if (!target.diagnostics.failure) target.diagnostics.failure = std::current_exception();
        };
        if (acquired_writable) poison(acquired);
        if (prepared_acquired_writable && frame.acquired_ != &acquired) poison(*frame.acquired_);
        throw; // no input return, Text cleanup, slot return, speculative unwind or retry
    }
}
} // namespace bsp
