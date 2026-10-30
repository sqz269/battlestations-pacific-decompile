#include "bsp/native_gui_text_empty_unchanged_storage.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_wide_string.hpp"
#include "native_gui_text_cache_backing.hpp"
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw EMPTY unchanged Text storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextEmptyUnchangedStorageContext;
using Frame = NativeGuiTextEmptyUnchangedStorageFrame;
using Acquired = NativeGuiTextEmptyUnchangedStorageAcquired;
using Input = NativeGuiTextEmptyUnchangedInputProgress;
using Region = NativeGuiTextEmptyUnchangedStorageRegion;
using Regions = NativeGuiTextEmptyUnchangedStorageRegions;
using MainPhase = NativeGuiTextMainSectionStoragePhase;
using Stage = NativeGuiTextEmptyUnchangedStorageStage;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::tuple_size<decltype(Regions::entries)>::value >= 16 +
    std::tuple_size<decltype(NativeGuiTextMainSectionLifetimeRegions::entries)>::value +
    std::tuple_size<decltype(NativeGuiTextFontLifetimeRegions::entries)>::value + 6);
constexpr std::size_t text_bytes = 0x1f8;
std::uintptr_t address(const volatile void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
const void* at(const void* p, std::size_t offset = 0) noexcept {
    return reinterpret_cast<const void*>(address(p) + offset);
}
Word read(const void* p, std::size_t offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
std::uint8_t byte(const void* p, std::size_t offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
bool valid(Region r) noexcept {
    return r.begin && r.bytes && static_cast<std::uint64_t>(address(r.begin)) + r.bytes <= 0x100000000ull;
}
bool disjoint(Region a, Region b) noexcept {
    const auto x = static_cast<std::uint64_t>(address(a.begin));
    const auto y = static_cast<std::uint64_t>(address(b.begin));
    return x + a.bytes <= y || y + b.bytes <= x;
}
void require(bool yes, const char* message) { if (!yes) throw std::logic_error(message); }
void append(Regions& r, const volatile void* p, std::size_t n) noexcept {
    if (!p || !n) return;
    if (r.count == r.entries.size()) std::terminate(); // fixed schema; never truncate.
    r.entries[r.count++] = {p, n};
}
bool input_unused(const Input& p) noexcept {
    return !p.construct_entered && !p.construct_returned && !p.construct_failed &&
        !p.return_entered && !p.returned && !p.return_failed;
}
bool input_live(const Input& p) noexcept {
    return p.construct_entered && p.construct_returned && !p.construct_failed &&
        !p.return_entered && !p.returned && !p.return_failed;
}
bool input_returned(const Input& p) noexcept {
    return p.construct_entered && p.construct_returned && !p.construct_failed &&
        p.return_entered && p.returned && !p.return_failed;
}
bool release_fresh(const NativeGuiTextSectionStorageRelease& r) noexcept {
    return !r.identity && !r.captured_decrement && !r.result && !r.entered && !r.decremented &&
        !r.zero_entered && !r.zero_returned && !r.complete;
}
bool ensure_fresh(const Context& c) noexcept {
    const auto& a = c.ensure_acquired;
    const auto& s = a.section;
    const auto& u = a.parenting.recursive_unregister;
    return c.ensure_frame.matches(c.construction, a, MainPhase::fresh) && a.phase == MainPhase::fresh &&
        a.boundary == decltype(a.boundary)::none && a.escaped_boundary == decltype(a.boundary)::none &&
        !a.active_call_site && !a.escaped_call_site && !a.destination && !a.captured188 && !a.first_primary &&
        !a.current_primary && !a.first_geometry && !a.current_geometry && !a.section_count &&
        !a.mesh_gate_read && !a.creation_skipped && !a.name_construct_entered && !a.name_construct_returned &&
        !a.captured_name_data && !a.name_return_entered && !a.name_cleanup_returned && a.native_eh_state == -1 &&
        !s.section.started && !s.section.creator && !s.section.companion && !s.section.owner_record && !s.section.registered &&
        !s.section_creator && s.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
        !s.material.native_site && !s.material.cache && !s.material.effect && !s.material.raw_slot &&
        !s.material.material && !s.material.companion && !s.material.owner_record && !s.material.registered &&
        !s.material_creator && !s.appended && !s.material_published && release_fresh(s.section_release) &&
        release_fresh(s.material_release) && !a.pair_decrement && !a.parenting_flag && !a.root_flag &&
        !a.parenting_receiver && !a.root_receiver && !a.parenting_flag_read && !a.parenting_entered &&
        !a.parenting_returned && !a.root_flag_read && !a.root_entered && !a.root_returned &&
        !a.parenting.started && !a.parenting.complete && !a.parenting.active_call_site &&
        !a.parenting.prepend.started && !a.parenting.prepend.complete && !a.parenting.prepend.active_call_site &&
        !u.started && !u.complete && !u.active_call_site && !u.hierarchy_unlinked && !u.point_lights_cleared &&
        !u.release_flag_stored && !u.attachment_cleared && !u.decrement_completed && !u.terminal_completed &&
        !u.completed_child_calls && !a.failure;
}
bool ensure_skipped(const Context& c, const void* raw) noexcept {
    const auto& a = c.ensure_acquired;
    const auto& s = a.section;
    return c.ensure_frame.matches(c.construction, a, MainPhase::complete) && a.phase == MainPhase::complete &&
        !a.failure && a.destination == raw && a.captured188 == c.construction.geometry_context.internal188.actual &&
        a.first_primary == c.construction.geometry_context.primary.actual && a.current_primary == a.first_primary &&
        a.first_geometry && a.current_geometry == a.first_geometry && a.mesh_gate_read && a.section_count == 1 &&
        a.creation_skipped && !a.name_construct_entered && !a.name_construct_returned && !a.name_return_entered &&
        !a.name_cleanup_returned && a.native_eh_state == -1 && !s.section.started && !s.section_creator &&
        s.material.phase == NativeMaterialFactoryAcquired::Phase::empty && !s.material_creator &&
        !s.material.cache && !s.appended && !s.material_published && release_fresh(s.section_release) &&
        release_fresh(s.material_release) && !a.pair_decrement &&
        a.parenting_flag_read && !a.parenting_flag && a.parenting_entered && a.parenting_returned &&
        a.parenting.started && a.parenting.complete && a.parenting_receiver == a.captured188 &&
        a.root_flag_read && !a.root_flag && a.root_entered && a.root_returned && a.root_receiver == a.captured188;
}
void bound_markers(const Context& c, const NativeGuiTextReferenceFrame& expected) {
    const auto& t = c.prefix.text;
    const auto& f = t.factory_context;
    const auto& p = t.publication;
    const auto& a = t.factory;
    const auto& s = expected.scalar;
    const auto& d = s.destruction;
    const auto& construction = f.construction;
    // Independent retained C++ fences BEFORE saved companion/payload reads.
    require(p.bind_entered && p.bind_complete && !p.bind_failed && p.reference &&
        !p.scalar_entered && !p.scalar_returned && !p.scalar_failed && !p.companion_disposal_entered && !p.disposed &&
        !s.started && !s.complete && !s.destructor_entered && !s.destructor_returned && !s.pool_returned &&
        !d.started && !d.complete && !d.base_entered && !d.base_returned && !d.exception_cleanup_started &&
        !construction.exception_cleanup_started && !construction.base_lifetime_ended &&
        !construction.consumed_cleanup_actions && construction.failure == decltype(construction.failure)::none,
        "EMPTY Text rejects disposed/failed/scalar/constructor-cleanup history before companion access");
    require(&f.reference_frame == &expected && a.started && a.complete && !a.failure &&
        a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete && a.raw_slot &&
        a.allocation_entered && a.allocation_returned && a.constructor_entered && a.constructor_returned &&
        a.prepared_context == &f && a.construction_record == &construction && a.constructor_result == a.raw_slot &&
        a.actual_pool == &f.publication.retirement.actual_pool_00f8bdf0 &&
        construction.started && construction.complete && construction.base_complete && construction.derived_complete &&
        construction.destination == a.raw_slot && construction.native_eh_state == 6 && construction.sections_entered &&
        construction.sections.complete && !construction.sections.exception_cleanup_started &&
        construction.sections.failure == decltype(construction.sections.failure)::none,
        "EMPTY Text needs the exact completed factory93/internal188 and separate canonical122 record");
}
void node_budget(const NativeGuiTextPrimaryPrefixLifetimeContext& p) {
    const auto check = [](const NativeGuiWidgetRawModelRoute& r, NativeGuiTextPrimaryPrefixCalls calls) {
        const auto& a = r.node_acquired;
        require(a.bound && !a.poisoned && !a.entry_failed && !a.capacity_failed && calls.data && calls.count &&
            calls.count <= 0xffffffffu / sizeof(*calls.data) && a.used < calls.count,
            "EMPTY Ensure needs explicitly retained remaining node-call backing; no spare/reset assumption");
        for (std::size_t i = 0; i < calls.count; ++i) {
            const auto& call = calls.data[i];
            require(i < a.used ? call.phase == NativeModelStorageDispatchCallAcquired::Phase::complete &&
                    call.kind != NativeModelStorageNodeCall::Kind::none && call.body_returned :
                call.phase == NativeModelStorageDispatchCallAcquired::Phase::fresh &&
                    call.kind == NativeModelStorageNodeCall::Kind::none && !call.receiver && !call.target &&
                    !call.admitted_profile && !call.first_argument && !call.second_argument && !call.result_al &&
                    !call.body_entered && !call.body_returned,
                "EMPTY Ensure cannot reuse a consumed/active/failed node-call slot");
        }
    };
    require(p.widget.primary.node.matches(p.producer_context.primary_node_context,
        p.widget.primary.node_acquired, p.primary_calls.data, p.primary_calls.count) &&
        p.widget.internal188.node.matches(p.internal_node_context, p.widget.internal188.node_acquired,
            p.internal_calls.data, p.internal_calls.count), "EMPTY Ensure must use SAME bound call arrays/providers");
    check(p.widget.primary, p.primary_calls); check(p.widget.internal188, p.internal_calls);
    // Actual next() enforces every reached call. Total added+retirement capacity
    // and opaque recursive use remain caller obligations, never a magic64 cap.
}
bool diagnostics_fresh(const Acquired::Diagnostics& d) noexcept {
    return d.boundary == Stage::none && d.escaped_boundary == Stage::none && !d.native_site && !d.text && !d.input &&
        !d.captured_data && !d.captured_size && !d.started && !d.ensure_entered && !d.ensure_returned &&
        !d.copy_entered && !d.copy_returned && !d.font_read && !d.font_flag_read && !d.text_length_read &&
        !d.temporary_length_read && !d.data_captured && !d.disarmed && !d.getter_entered && !d.getter_returned &&
        !d.return_entered && !d.return_returned && !d.normal_null_cleanup && !d.complete && !d.failure;
}
} // namespace

NativeGuiTextEmptyUnchangedStorageFrame::NativeGuiTextEmptyUnchangedStorageFrame(const Context& c,
    const void* input, const Input& progress, const NativeGuiTextReferenceFrame& reference, Acquired& acquired) noexcept
    : context_(&c), input_(input), input_progress_(&progress), reference_frame_(&reference), acquired_(&acquired) {}
NativeGuiTextEmptyUnchangedStorageFrame::~NativeGuiTextEmptyUnchangedStorageFrame() noexcept {
    if (requires_process_retention()) std::terminate(); // own metadata only; no automatic native cleanup.
}
bool NativeGuiTextEmptyUnchangedStorageFrame::matches(const Context& c, const void* input, const Input& progress,
    const NativeGuiTextReferenceFrame& reference, const Acquired& acquired, Phase expected) const noexcept {
    return context_ == &c && input_ == input && input_progress_ == &progress && reference_frame_ == &reference &&
        acquired_ == &acquired && phase_ == expected;
}
bool NativeGuiTextEmptyUnchangedStorageFrame::completed_without_temp_debt() const noexcept {
    return phase_ == Phase::unchanged && !active_ && !failure_ && entered_ && ensure_entered_ && ensure_returned_ &&
        copy_entered_ && copy_returned_ && compared_empty_ && disarmed_ && normal_ &&
        (cleanup_ == Cleanup::normal_null ? !captured_data_ && !getter_entered_ && !getter_returned_ &&
            !return_entered_ && !return_returned_ : cleanup_ == Cleanup::returned && captured_data_ &&
            getter_entered_ && getter_returned_ && return_entered_ && return_returned_);
}
void NativeGuiTextEmptyUnchangedStorageFrame::poison(std::exception_ptr failure) const noexcept {
    if (!failure_) failure_ = failure;
    phase_ = Phase::failed;
    active_ = false;
    if (acquired_admitted_) {
        acquired_->failed_ = true; acquired_->active_ = false;
        acquired_->diagnostics.escaped_boundary = stage_;
        if (!acquired_->diagnostics.failure) acquired_->diagnostics.failure = failure;
    }
}
void NativeGuiTextEmptyUnchangedStorageFrame::require_fixed_tuple() const {
    const auto& c = *context_;
    const auto& m = c.main.prepared_context();
    const auto& font = c.font.prepared_context();
    const auto& p = c.prefix;
    const auto& f = p.text.factory_context;
    const auto& r = f.publication.retirement;
    require(p.operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::geometry_main_section_font &&
        p.operation.main_section() == &c.main && p.operation.font_lifetime() == &c.font &&
        p.operation.geometry() == &m.geometry && &font.geometry == &m.geometry && m.geometry.prefix == &p &&
        &m.construction == &c.construction && &m.reference_frame == reference_frame_ &&
        &font.reference_frame == reference_frame_ && &f.reference_frame == reference_frame_ &&
        &c.construction.text == &p.text && &c.construction.authentication == &p.authentication &&
        r.primary_prefix == &p && r.font_lifetime == &c.font && r.font_shader_frame == &font.shader &&
        r.empty_unchanged == this &&
        f.publication.font_shader_frame == &font.shader && &r.resources == &c.construction.resources &&
        &c.ensure_frame != &m.frame && &c.ensure_acquired != &m.acquired,
        "EMPTY operation needs SAME original FontMain authority and a distinct per-call Ensure graph");
    require(input_ && input_progress_->header8 == input_ && &input_progress_->strings == &c.strings &&
        &input_progress_->raw_strings == &c.raw_strings && &c.raw_strings == &c.construction.resources.strings &&
        same_native_string_raw_pool_cells(c.raw_strings, input_progress_->raw_strings) &&
        same_native_string_raw_pool_cells(c.raw_strings, c.construction.resources.strings) &&
        same_native_string_storage_raw_pool_cells(c.strings, c.raw_strings),
        "EMPTY input/copy/return need the same actual-manager typed storage and THREE live cell addresses");
}
Regions NativeGuiTextEmptyUnchangedStorageFrame::metadata_regions() const noexcept {
    Regions r;
    // FIRST five roles have explicit containment/identity handling in the
    // disjointness checker. Never select a role merely by a matching address.
    append(r, this, sizeof(*this)); append(r, context_, sizeof(*context_)); append(r, acquired_, sizeof(*acquired_));
    append(r, temporary_, sizeof(temporary_)); append(r, input_, 8);
    append(r, input_progress_, sizeof(*input_progress_)); append(r, reference_frame_, sizeof(*reference_frame_));
    const auto& c = *context_;
    append(r, &c.ensure_frame, sizeof(c.ensure_frame)); append(r, &c.ensure_acquired, sizeof(c.ensure_acquired));
    append(r, &c.strings, sizeof(c.strings)); append(r, &c.raw_strings, sizeof(c.raw_strings));
    append(r, &input_progress_->strings, sizeof(input_progress_->strings));
    append(r, &input_progress_->raw_strings, sizeof(input_progress_->raw_strings));
    append(r, &c.raw_strings.actual_published_01090aa8, sizeof(c.raw_strings.actual_published_01090aa8));
    append(r, &c.raw_strings.actual_small_returns_disabled_01090aa4, sizeof(c.raw_strings.actual_small_returns_disabled_01090aa4));
    append(r, &c.raw_strings.actual_manager_publication_01090aa0, sizeof(c.raw_strings.actual_manager_publication_01090aa0));
    return r; // no old peer walker, duplicate WidgetAcquired, native field or getter.
}
void NativeGuiTextEmptyUnchangedStorageFrame::require_record_disjoint(const Acquired& candidate) const {
    const auto& c = *context_;
    const Region writes[] = {{this, sizeof(*this)}, {&candidate, sizeof(candidate)},
        {&c.ensure_frame, sizeof(c.ensure_frame)}, {&c.ensure_acquired, sizeof(c.ensure_acquired)}};
    const Region input{input_, 8};
    require(valid(input) && !(address(input_) & 3u), "EMPTY input needs an aligned actual8B header extent");
    for (std::size_t i = 0; i != 4; ++i) {
        require(valid(writes[i]) && !(address(writes[i].begin) & 3u), "EMPTY records need aligned representable extents");
        for (std::size_t j = i + 1; j != 4; ++j)
            require(disjoint(writes[i], writes[j]), "EMPTY caller and Ensure writable records must be independent");
        require(disjoint(input, writes[i]), "EMPTY input header cannot alias writable records or the temporary");
    }
    const auto metadata = metadata_regions();
    for (std::size_t i = 0; i < metadata.count; ++i) {
        const auto borrowed = metadata.entries[i];
        require(valid(borrowed), "EMPTY metadata needs representable retained backing");
        for (std::size_t j = 0; j != 4; ++j) {
            const bool exact_role = (j == 0 && (i == 0 || i == 3)) ||
                (j == 1 && i == 2 && &candidate == acquired_) || (j == 2 && i == 7) || (j == 3 && i == 8);
            require(exact_role || disjoint(writes[j], borrowed), "EMPTY writes cannot overlap another constructor-bound role");
        }
        require(i == 4 || disjoint(input, borrowed), "EMPTY input header cannot alias control/provider metadata");
    }
    const auto check = [&](Region borrowed) {
        require(valid(borrowed) && disjoint(input, borrowed), "EMPTY input cannot overlap retained peer/provider backing");
        for (const auto write : writes)
            require(disjoint(write, borrowed), "EMPTY records cannot overwrite retained peer/provider backing");
    };
    const auto main = c.main.metadata_regions(); for (std::size_t i = 0; i < main.count; ++i) check({main.entries[i].begin, main.entries[i].bytes});
    const auto font = c.font.metadata_regions(); for (std::size_t i = 0; i < font.count; ++i) check({font.entries[i].begin, font.entries[i].bytes});
    const auto& p = c.prefix;
    for (const Region region : {Region{&p, sizeof(p)}, {&c.main, sizeof(c.main)}, {&c.font, sizeof(c.font)},
        {&c.construction, sizeof(c.construction)}, {p.primary_calls.data, p.primary_calls.count * sizeof(*p.primary_calls.data)},
        {p.internal_calls.data, p.internal_calls.count * sizeof(*p.internal_calls.data)}}) check(region);
    const auto& f = p.text.factory_context;
    if (f.disposal.context_bytes) check({f.disposal.context, f.disposal.context_bytes});
    else require(!f.disposal.context, "EMPTY disposal context must have its actual retained extent");
    // Walk recursive late material/shader cache metadata separately: fixed
    // returned Region128 is not a cap on recursive or opaque allocations.
    const auto* cache = c.main.prepared_context().acquired.section.material.cache.get();
    if (cache) gui_text_backing_detail::shader_cache_backing<Region>(*cache, check);
    gui_text_backing_detail::shader_cache_backing<Region>(c.font.prepared_context().shader.cache, check);
}
void* NativeGuiTextEmptyUnchangedStorageFrame::require_bound(NativeGuiTextRawModelReference& reference) const {
    const auto& c = *context_;
    require_fixed_tuple();
    bound_markers(c, *reference_frame_);
    const auto& t = c.prefix.text;
    const auto& f = t.factory_context;
    const auto& r = f.publication;
    require(t.publication.reference == &reference && f.companion_storage == &reference &&
        f.companion_storage_bytes >= sizeof(reference) && reference.phase() == NativeGuiTextRawModelReference::Phase::bound &&
        reference.matches_reference_frame(*reference_frame_) && &reference.storage_context() == &r,
        "EMPTY entry is only its SAME completed bound canonical122 companion");
    void* const raw = reference.storage();
    require(raw == t.factory.raw_slot && valid({raw, text_bytes}) && !(address(raw) & 3u) &&
        r.owners.find(raw) == &reference && static_cast<const void*>(&reference.reference_count) == at(raw, 4),
        "EMPTY Text identity/count cell needs SAME live canonical owner, never another reference");
    require(c.main.phase() == NativeGuiTextMainSectionLifetimeStorageFrame::Phase::sealed &&
        c.font.phase() == NativeGuiTextFontLifetimeStorageFrame::Phase::sealed &&
        c.main.text_identity() == raw && c.font.text_identity() == raw,
        "EMPTY caller requires original Main-created and Font slices already SEALED for this actual Text");
    c.main.require_live(raw, NativeGuiTextMainSectionLifetimeStorageFrame::Stage::bound);
    c.font.require_live(raw, NativeGuiTextFontLifetimeStorageFrame::Stage::bound);
    bound_markers(c, *reference_frame_); // helper authentication can reach callbacks; fence independently again.
    require(reference.phase() == NativeGuiTextRawModelReference::Phase::bound && reference.storage() == raw &&
        t.publication.reference == &reference && r.owners.find(raw) == &reference && read(raw) == 0x00d5c6c8u,
        "EMPTY caller cannot resume after a caught callback/lifetime transition");
    const auto& m = c.main.prepared_context();
    require(m.frame.matches_created_normal(m.construction, m.acquired, raw, m.acquired.current_geometry,
        m.acquired.section.section_creator, m.acquired.section.material_creator),
        "EMPTY Ensure never replaces original private created-normal Main authority");
    return raw;
}
Regions NativeGuiTextEmptyUnchangedStorageFrame::live_regions(NativeGuiTextRawModelReference& reference) const {
    try {
        require(phase_ != Phase::failed && !failure_, "EMPTY live view cannot resume failed work");
        void* const raw = require_bound(reference);
        auto r = metadata_regions();
        const auto main = context_->main.live_regions(raw, NativeGuiTextMainSectionLifetimeStorageFrame::Stage::bound);
        const auto font = context_->font.live_regions(raw, NativeGuiTextFontLifetimeStorageFrame::Stage::bound);
        for (std::size_t i = 0; i < main.count; ++i) append(r, main.entries[i].begin, main.entries[i].bytes);
        for (std::size_t i = 0; i < font.count; ++i) append(r, font.entries[i].begin, font.entries[i].bytes);
        append(r, raw, text_bytes); append(r, &reference, sizeof(reference));
        append(r, context_->construction.geometry_context.primary.actual, NativeModelPool::slot_bytes);
        append(r, context_->construction.geometry_context.internal188.actual, NativeModelPool::slot_bytes);
        const auto add_dynamic = [&](Region dynamic) {
            require(valid(dynamic), "EMPTY live dynamic extent needs representable genuine acquisition backing");
            for (std::size_t i = 0; i < r.count; ++i)
                require(disjoint(dynamic, r.entries[i]), "EMPTY live input/temporary backing must be independent of known native/control backing");
            const auto& p = context_->prefix;
            for (const Region calls : {Region{p.primary_calls.data, p.primary_calls.count * sizeof(*p.primary_calls.data)},
                {p.internal_calls.data, p.internal_calls.count * sizeof(*p.internal_calls.data)}})
                require(valid(calls) && disjoint(dynamic, calls), "EMPTY live buffer cannot overlap retained node-call arrays");
            const auto check_cache = [&](Region backing) {
                require(valid(backing) && disjoint(dynamic, backing), "EMPTY live buffer cannot alias recursive cache operation backing");
            };
            if (const auto* cache = context_->main.prepared_context().acquired.section.material.cache.get())
                gui_text_backing_detail::shader_cache_backing<Region>(*cache, check_cache);
            gui_text_backing_detail::shader_cache_backing<Region>(context_->font.prepared_context().shader.cache, check_cache);
            append(r, dynamic.begin, dynamic.bytes);
        };
        // Only our real copy's normal issuance permits these live dynamic
        // views. After capture/cleanup no temporary header/data is inspected.
        if (copy_returned_ && input_live(*input_progress_)) {
            const auto* data = reinterpret_cast<const void*>(read(input_, 4));
            if (data) add_dynamic({data, read(input_) * 2u + 2u});
        }
        if (copy_returned_ && cleanup_ == Cleanup::none && !return_entered_) {
            const auto* data = reinterpret_cast<const void*>(read(temporary_, 4));
            if (data) add_dynamic({data, read(temporary_) * 2u + 2u});
        } else if (cleanup_ == Cleanup::captured && captured_data_ && captured_size_ && !return_entered_)
            add_dynamic({captured_data_, captured_size_}); // saved still-live capture, NO header rediscovery.
        return r;
    } catch (...) { poison(std::current_exception()); throw; }
}
void NativeGuiTextEmptyUnchangedStorageFrame::require_retirement_quiescent() const {
    try {
        require_fixed_tuple(); // C++ identities/cell ADDRESSES only, never native values or peer live/post.
        require(!active_ && phase_ != Phase::failed && !failure_, "EMPTY active/failed work blocks retirement");
        const auto& a = *acquired_;
        if (phase_ == Phase::fresh) {
            require(!entered_ && stage_ == Stage::none && cleanup_ == Cleanup::none && !ensure_entered_ && !ensure_returned_ &&
                !copy_entered_ && !copy_returned_ && !compared_empty_ && !disarmed_ && !getter_entered_ &&
                !getter_returned_ && !return_entered_ && !return_returned_ && !normal_ && !captured_data_ && !captured_size_ &&
                !a.bound_frame_ && !a.active_ && !a.failed_ && !a.completed_ && diagnostics_fresh(a.diagnostics) &&
                ensure_fresh(*context_) && input_unused(*input_progress_),
                "Unused EMPTY reservation must have wholly untouched leaf/Ensure/caller input history");
        } else require(completed_without_temp_debt() && a.bound_frame_ == this && !a.active_ && !a.failed_ && a.completed_ &&
            context_->ensure_frame.matches(context_->construction, context_->ensure_acquired, MainPhase::complete) &&
            context_->ensure_acquired.phase == MainPhase::complete && !context_->ensure_acquired.failure &&
            input_returned(*input_progress_), "EMPTY normal temporary completion cannot excuse unreturned/failed caller input");
    } catch (...) { poison(std::current_exception()); throw; }
}

NativeGuiTextEmptyUnchangedStorageResult submit_native_gui_text_empty_unchanged_storage_00aba8d0(
    NativeGuiTextRawModelReference& reference, const Context& c, Frame& f, Acquired& acquired) {
    const bool fresh = f.phase_ == Frame::Phase::fresh && !f.active_ && !f.failure_;
    f.active_ = true; f.phase_ = Frame::Phase::entered; f.entered_ = true; f.stage_ = Stage::admission;
    bool candidate_admitted = false;
    try {
        f.require_record_disjoint(acquired); candidate_admitted = true;
        f.require_record_disjoint(*f.acquired_); f.acquired_admitted_ = true;
        require(fresh && f.context_ == &c && f.acquired_ == &acquired &&
            !acquired.bound_frame_ && !acquired.active_ && !acquired.failed_ && !acquired.completed_ &&
            diagnostics_fresh(acquired.diagnostics), "EMPTY leaf rejects replacement/reentry/replay/failed invocation metadata");
        acquired.bound_frame_ = &f; acquired.active_ = true;
        auto& a = acquired.diagnostics; a.started = true; a.boundary = Stage::admission; a.input = f.input_;
        f.require_fixed_tuple();
        require(input_live(*f.input_progress_) && ensure_fresh(c), "EMPTY entry needs real live wide input and a fresh DISTINCT Ensure");
        void* const raw = f.require_bound(reference); a.text = raw;
        node_budget(c.prefix);
        // Alias checks use bound native extents without substituting sampled
        // input/temporary values for the later native copy/compare operands.
        const auto check_live = [&]() {
            const auto regions = f.live_regions(reference);
            const auto metadata_count = f.metadata_regions().count;
            const Region writes[] = {{&f, sizeof(f)}, {&acquired, sizeof(acquired)},
                {&c.ensure_frame, sizeof(c.ensure_frame)}, {&c.ensure_acquired, sizeof(c.ensure_acquired)}};
            for (std::size_t i = 0; i < regions.count; ++i) {
                const auto region = regions.entries[i];
                require(valid(region), "EMPTY live region must be representable with genuine custody");
                // Own roles were checked by role in require_record_disjoint;
                // peer/native regions follow the exact metadata prefix.
                if (i < metadata_count) continue;
                for (const auto write : writes)
                    require(disjoint(write, region), "EMPTY records cannot overlap live native/header/provider backing");
                require(disjoint({f.input_, 8}, region), "EMPTY caller header cannot overlap live native backing");
            }
        };
        const auto healthy = [&]() {
            require(f.active_lexical_stage() && acquired.active_ && !acquired.failed_ &&
                acquired.bound_frame_ == &f && input_live(*f.input_progress_),
                "EMPTY outer call cannot seal after caught reentry/failure/input return");
        };
        const auto location = [&](Stage stage, Word site) {
            f.stage_ = stage; a.boundary = stage; a.native_site = site;
        };
        check_live(); healthy();
        location(Stage::ensure, 0x00aba8ec); a.ensure_entered = f.ensure_entered_ = true;
        ensure_native_gui_text_post_constructor_sections_storage_00ab8530(reference,
            c.construction, c.ensure_frame, c.ensure_acquired);
        a.ensure_returned = f.ensure_returned_ = true;
        healthy(); require(ensure_skipped(c, raw), "EMPTY entry requires real normal Ensure SKIP and both late callbacks");
        require(f.require_bound(reference) == raw, "EMPTY Ensure cannot replace original live Main/Font graph");
        check_live(); healthy();
        location(Stage::copy, 0x00aba8fa); a.copy_entered = f.copy_entered_ = true;
        copy_construct_native_wide_string_header_004c8dd0(f.temporary_, f.input_, c.strings);
        a.copy_returned = f.copy_returned_ = true; // private issue ONLY after real copy normal return.
        healthy(); require(f.require_bound(reference) == raw, "EMPTY copy callback cannot retire/replace the receiver");
        check_live(); healthy();
        location(Stage::font, 0x00aba8ff);
        void* const font = reinterpret_cast<void*>(read(raw, 0x108)); a.font_read = true; // CURRENT AFTER actual copy.
        require(font && valid({font, 0xac}), "EMPTY continuation requires nonnull actual CURRENT Font");
        auto& resource = c.font.prepared_context().fonts.resolve_image(font);
        require(resource.live() && resource.native_image() == font, "EMPTY Font must be the real live published image, not a semantic field");
        a.native_site = 0x00aba907; const auto font_flag = byte(font, 0x48); a.font_flag_read = true;
        require(font_flag == 0, "EMPTY continuation rejects reached uppercase branch after prior Ensure/copy effects");
        location(Stage::comparison, 0x00aba921); const auto text_length = read(raw, 0xec); a.text_length_read = true;
        require(text_length == 0, "EMPTY continuation retains unsupported prior nonempty Text before changed work");
        const auto temporary_length = read(f.temporary_); a.temporary_length_read = true;
        require(temporary_length == 0, "EMPTY continuation rejects changed/nonempty copied text before ABA955");
        f.compared_empty_ = true; // actual empty native decision; no wcsicmp/TextF0 read.
        location(Stage::capture, 0x00abae8c);
        void* const captured = reinterpret_cast<void*>(read(f.temporary_, 4)); // ONCE BEFORE disarm.
        f.captured_data_ = a.captured_data = captured; a.data_captured = true; f.cleanup_ = Frame::Cleanup::captured;
        location(Stage::disarmed, 0x00abae93); f.disarmed_ = a.disarmed = true; // native state-1 BEFORE conditional length.
        if (captured) {
            const Word size = read(f.temporary_) * 2u + 2u; // AE9E CURRENT length, wrapping BEFORE getter.
            f.captured_size_ = a.captured_size = size;
            location(Stage::getter, 0x00abaeaa); f.getter_entered_ = a.getter_entered = true;
            auto* const pool = native_string_pool_get_or_create_00419cc0(c.raw_strings.actual_published_01090aa8,
                c.raw_strings.actual_manager_publication_01090aa0);
            f.getter_returned_ = a.getter_returned = true;
            healthy(); // caught nested failure retains captured allocation; never return twice.
            require(f.require_bound(reference) == raw, "EMPTY getter callback cannot end pending receiver/captured allocation custody");
            location(Stage::returning, 0x00abaeb1); f.return_entered_ = a.return_entered = true;
            return_native_string_pool_00bd1510(pool, captured, size, c.raw_strings.actual_small_returns_disabled_01090aa4);
            f.return_returned_ = a.return_returned = true; f.cleanup_ = Frame::Cleanup::returned;
        } else {
            // AE9C null path: no length/getter/return call or fake return credit.
            a.normal_null_cleanup = true; f.cleanup_ = Frame::Cleanup::normal_null;
        }
        healthy(); require(f.require_bound(reference) == raw, "EMPTY normal footer cannot replace original Main/Font authority");
        require(ensure_skipped(c, raw), "EMPTY normal tail cannot hide failed/reentered Ensure metadata");
        require(f.entered_ && f.ensure_returned_ && f.copy_returned_ && f.compared_empty_ && f.disarmed_ &&
            (f.cleanup_ == Frame::Cleanup::normal_null ? !f.captured_data_ && !f.getter_entered_ && !f.getter_returned_ &&
                !f.return_entered_ && !f.return_returned_ : f.cleanup_ == Frame::Cleanup::returned && f.captured_data_ &&
                f.getter_entered_ && f.getter_returned_ && f.return_entered_ && f.return_returned_),
            "EMPTY normal result needs actual private temporary cleanup completion");
        location(Stage::normal_return, 0x00abaeb6);
        a.complete = acquired.completed_ = f.normal_ = true;
        acquired.active_ = f.active_ = false; f.phase_ = Frame::Phase::unchanged;
        return NativeGuiTextEmptyUnchangedStorageResult::unchanged_empty;
    } catch (...) {
        const auto failure = std::current_exception();
        f.poison(failure);
        if (candidate_admitted) {
            acquired.failed_ = true; acquired.active_ = false; acquired.diagnostics.escaped_boundary = f.stage_;
            if (!acquired.diagnostics.failure) acquired.diagnostics.failure = failure;
        }
        throw; // no input return, temporary cleanup attempt, native unwind inference, retry or rollback.
    }
}
} // namespace bsp
