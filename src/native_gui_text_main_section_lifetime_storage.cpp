#include "bsp/native_gui_text_main_section_lifetime_storage.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include <cstdint>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw Text main-section lifetime storage requires MSVC Win32.
#endif

namespace bsp {
namespace {
using Context = NativeGuiTextMainSectionLifetimeStorageContext;
using Frame = NativeGuiTextMainSectionLifetimeStorageFrame;
using MainPhase = NativeGuiTextMainSectionStoragePhase;
using GeometryPhase = NativeGuiTextMainGeometryStoragePhase;
using Shader = NativeGuiTextShaderStorageFrame;
using Word = std::uint32_t;
constexpr Word bit6 = Word{1} << 6;
static_assert(sizeof(void*) == sizeof(Word));
static_assert(std::tuple_size<decltype(NativeGuiTextMainSectionLifetimeRegions::entries)>::value >= 12);
void require(bool value, const char* why) { if (!value) throw std::invalid_argument(why); }
std::uintptr_t bits(const void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
const void* offset(const void* p, Word n) noexcept { return reinterpret_cast<const void*>(bits(p) + n); }
Word read(const void* p, Word n = 0) noexcept {
    return *reinterpret_cast<const volatile Word*>(bits(p) + n);
}
bool region(const void* p, std::size_t n) noexcept {
    return p && n && static_cast<std::uint64_t>(bits(p)) + n <= (std::uint64_t{1} << 32);
}
bool separate(const void* a, std::size_t an, const void* b, std::size_t bn) noexcept {
    return static_cast<std::uint64_t>(bits(a)) + an <= bits(b) ||
        static_cast<std::uint64_t>(bits(b)) + bn <= bits(a);
}
void append(NativeGuiTextMainSectionLifetimeRegions& r, const void* p, std::size_t n) noexcept {
    if (!p || !n) return;
    if (r.count == r.entries.size()) std::terminate(); // fixed schema; never truncate.
    r.entries[r.count++] = {p, n};
}
const NativeGuiTextPrimaryPrefixLifetimeContext* prefix(const Context& c) noexcept { return c.geometry.prefix; }
bool with_font(const Context& c) noexcept {
    const auto* p = prefix(c);
    return p && p->operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::geometry_main_section_font;
}
bool aliases(const Context& c, const Frame& frame) noexcept {
    const auto* p = prefix(c);
    if (!p) return false;
    const auto& f = p->text.factory_context;
    const auto& r = f.publication;
    const auto& m = c.construction;
    return (p->operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::geometry_main_section || with_font(c)) &&
        p->operation.geometry() == &c.geometry && p->operation.main_section() == &frame &&
        &c.reference_frame == &f.reference_frame && &m.text == &p->text &&
        &m.authentication == &p->authentication && &m.primary_context == &p->producer_context &&
        &m.primary_frame == &p->producer_frame && &m.primary == &p->producer &&
        &m.geometry_context == &c.geometry.construction && &m.geometry_frame == &c.geometry.frame &&
        &m.geometry == &c.geometry.acquired && &m.resources == &r.construction_context.sections &&
        &r.construction == &f.construction && &r.retirement.resources == &m.resources &&
        &r.retirement.model == &f.construction.sections.model &&
        r.retirement.primary_prefix == p && (with_font(c) ?
            p->operation.font_lifetime() && p->operation.font_lifetime() == r.retirement.font_lifetime :
            !r.retirement.font_lifetime) &&
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
        &c.geometry.construction.internal188.model == &m.resources.model &&
        &c.geometry.construction.geometry == &m.resources.geometry &&
        &m.resources.geometry.actual_owners() == &r.owners &&
        (!r.retirement.font_shader_frame || r.font_shader_frame == r.retirement.font_shader_frame) &&
        c.geometry.frame.matches_backing(c.geometry.construction, c.geometry.arguments,
            c.geometry.position, c.geometry.transform_scratch, c.geometry.bounds_scratch);
}
bool release_fresh(const NativeGuiTextSectionStorageRelease& a) noexcept {
    return !a.identity && !a.captured_decrement && !a.result && !a.entered && !a.decremented &&
        !a.zero_entered && !a.zero_returned && !a.complete;
}
bool main_fresh(const Context& c) noexcept {
    const auto& a = c.acquired; const auto& s = a.section;
    return c.frame.matches(c.construction, a, MainPhase::fresh) && a.phase == MainPhase::fresh &&
        a.boundary == decltype(a.boundary)::none && a.escaped_boundary == decltype(a.boundary)::none &&
        !a.active_call_site && !a.escaped_call_site && !a.destination && !a.captured188 &&
        !a.first_primary && !a.current_primary && !a.first_geometry && !a.current_geometry &&
        !a.section_count && !a.mesh_gate_read && !a.creation_skipped && !a.failure &&
        !a.name_construct_entered && !a.name_construct_returned && !a.captured_name_data &&
        !a.name_return_entered && !a.name_cleanup_returned && a.native_eh_state == -1 &&
        !s.section.started && !s.section.creator && !s.section.companion && !s.section.owner_record &&
        !s.section.registered && !s.section_creator && !s.material_creator &&
        s.material.phase == NativeMaterialFactoryAcquired::Phase::empty && !s.material.native_site &&
        !s.material.cache && !s.material.effect && !s.material.raw_slot && !s.material.material &&
        !s.material.companion && !s.material.owner_record && !s.material.registered &&
        !s.appended && !s.material_published && release_fresh(s.section_release) && release_fresh(s.material_release) &&
        !a.pair_decrement && !a.parenting_flag_read && !a.parenting_entered && !a.parenting_returned &&
        !a.root_flag_read && !a.root_entered && !a.root_returned && !a.parenting_receiver && !a.root_receiver &&
        !a.parenting.started && !a.parenting.complete && !a.parenting.prepend.started &&
        !a.parenting.prepend.complete && !a.parenting.recursive_unregister.started &&
        !a.parenting.recursive_unregister.complete;
}
bool prebody(const NativeGuiTextRetirementAcquired& d) noexcept {
    return !d.started && !d.complete && !d.destination && !d.exception_cleanup_started &&
        d.native_eh_state == -1 && !d.base_entered && !d.base_returned &&
        !d.normal_actions_entered && !d.normal_actions_returned && !d.unwind_actions_consumed &&
        !d.unwind_actions_returned && !d.shader.identity && !d.shader.decrement_entered &&
        !d.shadow.started && !d.glyphs.started;
}
bool cache_quiescent(const NativeMaterialEffectCacheAcquired* q) noexcept {
    // Actual Effect/entry/companion pointers may already be dead. CPP phase
    // records alone are inspected; a cache HIT need not load/publish anything.
    for (; q; q = q->loader.fallback.get()) {
        if (q->phase != NativeMaterialEffectCacheAcquired::Phase::not_started &&
            q->phase != NativeMaterialEffectCacheAcquired::Phase::complete) return false;
        if (q->loader.phase != NativeMaterialEffectLoadPhase::not_started &&
            q->loader.phase != NativeMaterialEffectLoadPhase::complete) return false;
        if (q->key_resolution && q->key_resolution->phase != NativeMaterialEffectCacheKeyResolutionAcquired::Phase::fresh &&
            q->key_resolution->phase != NativeMaterialEffectCacheKeyResolutionAcquired::Phase::complete) return false;
    }
    return true;
}
bool shader_unused(const Context& c, const Shader* shader, bool raw, bool post) noexcept {
    const auto* p = prefix(c); if (!p) return false;
    const auto& r = p->text.factory_context.publication;
    if (r.font_shader_frame != shader || (r.retirement.font_shader_frame && r.retirement.font_shader_frame != shader)) return false;
    if (!shader) return true;
    const auto* prepared = shader->prepared_raw_context();
    if (!prepared || shader->prepared_reference_frame() != &c.reference_frame ||
        !shader->matches_credit_backing(*prepared, c.reference_frame)) return false;
    const auto a = shader->credit_progress();
    if (a.failed || a.producer != Shader::RawPhase::fresh || a.credit != Shader::CreditPhase::none ||
        a.consumer != Shader::Consumer::none || a.effect || a.capture_entered || a.decrement_entered ||
        a.decrement_returned || a.zero_entered || a.zero_returned || a.slot_cleared || a.consumer_returned ||
        a.actual_text || a.reference || a.reference_context || a.reference_frame || a.cache ||
        a.font_context || a.font_frame || a.font_acquired || a.retirement_context || a.retirement_acquired || a.retirement_shader ||
        shader->phase != Shader::Phase::fresh || shader->boundary != Shader::Boundary::none ||
        shader->escaped_boundary != Shader::Boundary::none || shader->destination || shader->attempted ||
        shader->result || shader->shader_published ||
        shader->cache.phase != NativeMaterialEffectCacheAcquired::Phase::not_started ||
        shader->cache.caller_acquired || shader->cache.loader.phase != NativeMaterialEffectLoadPhase::not_started) return false;
    // Metadata-only Font export can reserve a FRESH shader Frame in122 while
    // leaving the raw118 consumer pointer null. That genuine old path never
    // enters a token and must stay privately untouched even after retirement.
    if (raw && r.retirement.font_shader_frame) {
        const auto& d = c.reference_frame.scalar.destruction;
        return a.active && a.retirement_started && a.active_consumer == Shader::Consumer::retirement &&
            !a.active_font_context && !a.active_font_frame && !a.active_font_acquired &&
            a.active_retirement_context == &r.retirement && a.active_retirement_acquired == &d &&
            a.active_retirement_shader == &d.shader;
    }
    return !a.active && a.active_consumer == Shader::Consumer::none &&
        !a.active_font_context && !a.active_font_frame && !a.active_font_acquired &&
        !a.active_retirement_context && !a.active_retirement_acquired && !a.active_retirement_shader &&
        (post && r.retirement.font_shader_frame ? a.retirement_started : !a.retirement_started);
}
bool no_shader_release(const NativeGuiTextRetirementShader& s) noexcept {
    return !s.identity && !s.captured_decrement && !s.result && !s.captured_profile && !s.captured_target &&
        !s.decrement_entered && !s.decremented && !s.zero_entered && !s.zero_returned && !s.slot_cleared;
}
bool header_entered(const NativeGuiTextRetirementAcquired& d) noexcept {
    return (d.normal_actions_entered & bit6) || (d.normal_actions_returned & bit6) ||
        (d.unwind_actions_consumed & bit6) || (d.unwind_actions_returned & bit6) ||
        d.strings[0].return_entered || d.strings[0].returned;
}
bool normal_header_returned(const NativeGuiTextRetirementAcquired& d) noexcept {
    // Structural return epoch only. The separate Font slice authenticates the
    // captured NULL/non-NULL header identity, length, getter and actual return.
    return (d.normal_actions_entered & bit6) && (d.normal_actions_returned & bit6) &&
        !d.exception_cleanup_started && d.strings[0].data_captured;
}
bool null_header_returned(const NativeGuiTextRetirementAcquired& d) noexcept {
    const auto& s = d.strings[0];
    return (d.normal_actions_entered & bit6) && (d.normal_actions_returned & bit6) &&
        !d.exception_cleanup_started && s.data_captured && !s.captured_data &&
        !s.captured_bytes && !s.length_captured && !s.getter_entered && !s.getter_returned &&
        !s.return_entered && !s.returned;
}
bool mesh_metadata(const GuiNativeMeshAcquired& m, const void* mesh) noexcept {
    return mesh && m.started && m.constructor_complete && m.registered && !m.raw_slot &&
        !m.published && m.creator == mesh;
}
bool associated(const NativeModelStorageGeometryAcquired& a, const void* model, const void* mesh,
    const NativeModelStorageGeometryContext& c) noexcept {
    return a.phase == NativeModelStorageGeometryPhase::complete && a.context == &c && a.frame &&
        a.actual_model == model && a.requested_captured && a.previous_captured && a.captured_requested == mesh &&
        !a.captured_previous && !a.same_geometry && a.geometry_published && a.increment_completed &&
        a.captured_increment && !a.decrement_completed && !a.terminal_completed;
}
bool geometry_complete(const Context& c, const void* actual, const void* mesh) noexcept {
    const auto* p = prefix(c); if (!p) return false;
    const auto& g = c.geometry; const auto& a = g.acquired; const auto* issued = p->producer.publication();
    return g.progress.entered && g.progress.returned && !g.progress.failed &&
        g.frame.matches(g.construction, a, GeometryPhase::complete) && a.phase == GeometryPhase::complete && !a.failure &&
        issued && p->producer_frame.matches(p->producer_context, p->producer, NativeGuiTextPrimaryModelStoragePhase::complete) &&
        issued->text_identity() == actual && issued->model_identity() == a.captured_primary &&
        issued->factory_context() == &p->text.factory_context && issued->factory_acquisition() == &p->text.factory &&
        issued->primary_record() == &p->producer_context.primary && issued->primary_dispatch_record() == &g.construction.primary &&
        issued->construction() == &g.construction.primary.construction && issued->pool() == p->producer.actual_pool &&
        issued->pool() == &g.construction.primary.model.storage.pool_01090054 &&
        p->name.construct_entered && p->name.construct_returned && !p->name.construct_failed &&
        p->name.return_entered && p->name.returned && !p->name.return_failed &&
        a.destination == actual && a.captured_primary == g.construction.primary.actual && a.primary_captured &&
        a.mesh_entered && a.mesh_returned && a.arguments_prepared && mesh_metadata(a.mesh, mesh) && a.captured_mesh == mesh &&
        a.association_entered && a.association_returned && associated(a.association, a.captured_primary, mesh, g.construction.association) &&
        a.creator_release_entered && a.creator_decremented && a.creator_release_returned && a.creator_decrement &&
        a.creator_result == 1 && !a.creator_zero_entered && !a.creator_zero_returned &&
        !a.creator_profile_captured && !a.creator_target_captured && a.position_prepared && a.position_entered && a.position_returned &&
        a.transform.entered && a.transform.body_entered && a.transform.body_returned && !a.transform.failed &&
        a.transform.receiver == a.captured_primary && a.transform.matrix &&
        a.transform.captured_target == 0x00b6db10 && a.transform.admitted_profile == 0x00d62de8;
}
bool returned_creator(const NativeGuiTextSectionStorageRelease& a, const void* identity,
    NativeTextureSurfaceReferenceIncrement target) noexcept {
    return identity && a.identity == identity && target && a.captured_decrement == target && a.entered &&
        a.decremented && a.complete && a.result == 1 && !a.zero_entered && !a.zero_returned;
}
bool main_complete(const Context& c, const void* actual, const void* mesh, const void* section, const void* material) noexcept {
    const auto& a = c.acquired; const auto& s = a.section; const auto* p = prefix(c);
    return p && c.frame.matches_created_normal(c.construction, a, actual, mesh, section, material) &&
        a.phase == MainPhase::complete && !a.failure && !a.escaped_call_site &&
        a.escaped_boundary == decltype(a.boundary)::none && a.destination == actual &&
        a.captured188 == c.geometry.construction.internal188.actual &&
        a.first_primary == c.geometry.construction.primary.actual && a.current_primary == a.first_primary &&
        a.first_geometry == mesh && a.current_geometry == mesh && a.mesh_gate_read && !a.section_count && !a.creation_skipped &&
        a.name_construct_entered && a.name_construct_returned && a.name_return_entered && a.name_cleanup_returned &&
        a.native_eh_state == -1 && s.section.started && s.section.registered && s.section.creator == section &&
        s.material.phase == NativeMaterialFactoryAcquired::Phase::complete && s.material.material == material &&
        s.material.registered && !s.material.effect && !s.material.raw_slot && s.material.cache &&
        s.material.cache->phase == NativeMaterialEffectCacheAcquired::Phase::complete && !s.material.cache->caller_acquired &&
        cache_quiescent(s.material.cache.get()) && s.appended && s.material_published &&
        !s.section_creator && !s.material_creator &&
        returned_creator(s.section_release, section, a.pair_decrement) && returned_creator(s.material_release, material, a.pair_decrement) &&
        a.parenting_flag_read && !a.parenting_flag && a.parenting_entered && a.parenting_returned &&
        a.parenting.started && a.parenting.complete && a.parenting_receiver == a.captured188 &&
        a.root_flag_read && !a.root_flag && a.root_entered && a.root_returned && a.root_receiver == a.captured188;
}
bool shadow_complete(const Context& c, const void* mesh) noexcept {
    const auto* p = prefix(c); if (!p) return false;
    const auto& s = p->text.factory_context.construction.sections;
    const auto& r = c.construction.resources;
    const auto& a = s.mesh_release;
    return s.started && s.complete && !s.exception_cleanup_started &&
        s.failure == NativeGuiTextRawModelSectionStorageAcquired::Failure::none &&
        s.model_constructed && s.shadow_published && s.model_registered && mesh_metadata(s.mesh, mesh) &&
        returned_creator(a, mesh, a.captured_decrement) && associated(s.model.association, s.model.actual, mesh, r.association);
}
RenderCommandReference& canonical(const Context& c, void* actual, RenderCommandReference* expected,
    Word profile, Word terminal, bool sole) {
    auto& owners = prefix(c)->text.factory_context.publication.owners;
    require(actual && region(actual, 8), "Main-section live identity needs representable actual prefix");
    auto* const reference = owners.find(actual); // BEFORE actual/companion access.
    require(reference && reference == expected && static_cast<const void*>(&reference->reference_count) == offset(actual, 4) &&
        (sole ? reference->reference_count.load(std::memory_order_relaxed) == 1 :
            reference->reference_count.load(std::memory_order_relaxed) > 0),
        "Main-section requires SAME still-live canonical actual+4 and sole owner where admitted");
    auto& data = prefix(c)->text.factory_context.publication.data;
    const auto* table = static_cast<const volatile Word*>(data.data_at(profile, 2 * sizeof(Word)));
    require(read(actual) == profile && table[0] == 0x00bd30e0 && table[1] == terminal,
        "Main-section requires genuine CURRENT canonical flags1 terminal family");
    return *reference;
}
void live_model(const Context& c, NativeModelStorageDispatchContext& m, void* expected) {
    require(expected && m.actual == expected && m.reference && !m.reference->retired() &&
        m.construction.complete && m.construction.metadata_bound && m.construction.context == &m.model.storage &&
        !m.lifetime.started && !m.lifetime.scalar_started && !m.lifetime.complete && !m.lifetime.pool_returned &&
        !m.release.started && !m.release.complete, "Main-section live Model must precede ANY owning release");
    (void)canonical(c, expected, &*m.reference, 0x00d62de8, 0x00b75290, false);
}
bool primary_release_started(const Context& c) noexcept {
    const auto& p = *prefix(c); const auto& m = c.geometry.construction.primary;
    return m.release.started || m.lifetime.started || m.lifetime.scalar_started ||
        p.widget_acquired.scene4c.entered || p.widget_acquired.late4c.entered;
}
void live_graph(const Context& c, void* primary, void* internal, void* mesh, void* shadow_mesh,
    void* section, void* material) {
    const auto& p = *prefix(c); const auto& a = c.acquired;
    live_model(c, c.geometry.construction.primary, primary);
    require(read(primary, 0x180) == bits(mesh), "Main-section primary must retain SAME actual379 geometry");
    (void)canonical(c, mesh, c.geometry.acquired.mesh.companion, 0x00d62d60, 0x00b74280, true);
    (void)canonical(c, section, a.section.section.companion, 0x00d63194, 0x00b86690, true);
    (void)canonical(c, material, a.section.material.companion, 0x00d5e520, 0x00b194b0, true);
    const auto& m = *static_cast<const NativeMeshStorage*>(mesh);
    const auto draw_bytes = static_cast<std::uint64_t>(m.draw_sections_54.capacity_08) * 4u;
    require(m.draw_sections_54.count_04 == 1 && m.draw_sections_54.capacity_08 >= 1 &&
        draw_bytes <= 0xffffffffu && region(m.draw_sections_54.data_00, static_cast<std::size_t>(draw_bytes)) &&
        !(bits(m.draw_sections_54.data_00) & 3u) && m.draw_sections_54.data_00[0] == section &&
        !m.index_stream_60 && !m.vertex_stream_count_7c,
        "Main-section Mesh is exactly one created Section with no stream/index/content extension");
    const auto& s = *static_cast<const NativeMeshSectionStorage*>(section);
    require(s.material_20 == material && !s.next_section_38 && !s.instance_generator_binding_5c &&
        !s.vertex_stream_count_4c && !s.vertex_layout_50 &&
        static_cast<const NativeMaterialStorage*>(material)->effect_7c,
        "Main-section sole material requires empty next/generator/streams/layout and genuine Effect");
    // Shadow may have ENDED before the header/Base stage. Its record, never
    // its native fields or erased Mesh companion, governs that boundary.
    const auto& i = c.geometry.construction.internal188;
    if (!p.text.factory_context.reference_frame.scalar.destruction.shadow.started &&
        !i.release.started && !i.lifetime.started && !i.lifetime.scalar_started) {
        live_model(c, c.geometry.construction.internal188, internal);
        require(read(internal, 0x180) == bits(shadow_mesh), "Main-section preserves original Shadow association");
        (void)canonical(c, shadow_mesh, p.text.factory_context.construction.sections.mesh.companion,
            0x00d62d60, 0x00b74280, true);
    }
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
    return r.tree_acquired.bound && !r.tree_acquired.poisoned && r.tree_acquired.current18.phase == P::complete &&
        r.tree_acquired.current0.phase == P::complete && r.tree_acquired.current04_entered && r.tree_acquired.current04_returned &&
        r.node_acquired.bound && !r.node_acquired.poisoned && !r.node_acquired.entry_failed && !r.node_acquired.capacity_failed &&
        r.node_acquired.used <= calls.count && calls_settled(calls);
}
bool model_mesh_returned(const NativeModelStorageDispatchContext& m, const void* mesh) noexcept {
    const auto& a = m.lifetime;
    return mesh && m.reference && m.reference->retired() && m.release.started && m.release.complete &&
        a.started && a.complete && !a.exception_cleanup_started && a.scalar_started && a.scalar_complete &&
        a.flags_read && (a.flags_low_byte & 1u) && a.pool_returned && !a.captured_retained &&
        a.captured_geometry == mesh && a.geometry_decrement_completed && a.geometry_terminal_completed && a.geometry_cleared &&
        !a.geometry_unwind.started && a.node.complete && a.node.base_cleanup_completed;
}
} // namespace

NativeGuiTextMainSectionLifetimeStorageFrame::NativeGuiTextMainSectionLifetimeStorageFrame(const Context& c) noexcept
    : context_(c) {}
NativeGuiTextMainSectionLifetimeStorageFrame::~NativeGuiTextMainSectionLifetimeStorageFrame() noexcept {
    // During partial C++ construction the forward prefix may not exist yet.
    if (phase_ == Phase::fresh && !active_ && !failure_) return;
    if (active_ || phase_ == Phase::failed || !post_text_quiescent()) std::terminate();
}
void NativeGuiTextMainSectionLifetimeStorageFrame::enter() const {
    require(!active_ && phase_ != Phase::failed, "Main-section lifetime cannot reenter or resume a failed graph");
    active_ = true;
}
void NativeGuiTextMainSectionLifetimeStorageFrame::leave() const {
    require(active_ && phase_ != Phase::failed && !failure_, "Caught main-section rejection cannot resume outer admission");
    active_ = false;
}
void NativeGuiTextMainSectionLifetimeStorageFrame::fail() const noexcept {
    if (!failure_) failure_ = std::current_exception();
    phase_ = Phase::failed; active_ = false;
}
void NativeGuiTextMainSectionLifetimeStorageFrame::prepare_before_text_allocation() {
    try {
        enter();
        require(phase_ == Phase::fresh && !actual_ && !reference_ && aliases(context_, *this) && main_fresh(context_),
            "Main-section preparation needs exact whole-Attempt fresh metadata");
        const auto& p = *prefix(context_); const auto& f = p.text.factory_context;
        const auto& s = p.text.publication; const auto& a = p.text.factory; const auto& scalar = context_.reference_frame.scalar;
        const bool font = with_font(context_);
        if (!font) shader_ = f.publication.font_shader_frame;
        require((font || shader_unused(context_, shader_, false, false)) &&
            a.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::fresh && !a.started && !a.complete &&
            !a.raw_slot && !a.failure && !f.construction.started &&
            !s.bind_entered && !s.bind_complete && !s.bind_failed && !s.reference && !s.scalar_entered &&
            !s.companion_disposal_entered && !s.disposed &&
            !scalar.started && !scalar.complete && !scalar.destructor_entered && !scalar.destructor_returned &&
            !scalar.flags_captured && !scalar.pool_return_entered && !scalar.pool_returned && prebody(scalar.destruction) &&
            p.authentication_acquired.bound && !p.authentication_acquired.active && !p.authentication_acquired.poisoned &&
            p.widget_acquired.bound && !p.widget_acquired.active && !p.widget_acquired.poisoned &&
            p.producer_frame.matches(p.producer_context, p.producer, NativeGuiTextPrimaryModelStoragePhase::prepared) &&
            context_.geometry.frame.phase() == GeometryPhase::fresh && !context_.geometry.progress.entered &&
            !context_.geometry.progress.returned && !context_.geometry.progress.failed,
            "Pre-pool main reservation excludes native/type/name/cache/shader/constructor effects");
        const auto regions = metadata_regions();
        for (std::size_t i = 0; i < regions.count; ++i)
            require(region(regions.entries[i].begin, regions.entries[i].bytes), "Main metadata needs representable full extents");
        require(!(bits(&context_.frame) & 3u) && !(bits(&context_.acquired) & 3u) &&
            separate(&context_.frame, sizeof(context_.frame), &context_.acquired, sizeof(context_.acquired)),
            "Main native-call writable Frame/Acquired must be separate aligned metadata");
        phase_ = Phase::prepared; leave();
    } catch (...) { fail(); throw; }
}
void NativeGuiTextMainSectionLifetimeStorageFrame::seal_after_main_section(void* actual) {
    try {
        enter();
        const bool font = phase_ == Phase::prepared && with_font(context_);
        require(phase_ == Phase::prepared && !actual_ && !reference_ && aliases(context_, *this) &&
            actual && (font || shader_unused(context_, shader_, false, false)),
            font ? "Main structural seal follows one prepared constructor graph" :
                "Main-section seal follows one prepared unused Font domain");
        const auto& p = *prefix(context_); const auto& f = p.text.factory_context;
        const auto& pub = p.text.publication; const auto& a = context_.acquired;
        void* const mesh = context_.geometry.acquired.captured_mesh;
        void* const section = a.section.section.creator; void* const material = a.section.material.material;
        require((font || f.publication.retirement.font_lifetime == nullptr) && pub.bind_complete && !pub.bind_failed &&
            !pub.scalar_entered && !pub.scalar_failed && !pub.companion_disposal_entered && !pub.disposed &&
            p.text.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete && p.text.factory.complete &&
            !p.text.factory.failure && p.text.factory.raw_slot == actual &&
            geometry_complete(context_, actual, mesh) && main_complete(context_, actual, mesh, section, material),
            "Main-section seal requires exact private db55/379/created-normal provenance and returned creator obligations");
        const auto& old = f.construction.sections;
        void* const shadow_mesh = old.mesh_release.identity;
        require(shadow_complete(context_, shadow_mesh), "Main-section seal retains the genuine independent Shadow history");
        p.authentication.require_raw_model_transport_receiver(actual, p.text); // exact record; no whole-prefix cycle.
        auto* const reference = pub.reference;
        require(reference && reference->phase() == NativeGuiTextRawModelReference::Phase::bound &&
            reference->matches_reference_frame(context_.reference_frame) && reference->storage() == actual &&
            &reference->storage_context() == &f.publication &&
            static_cast<const void*>(&reference->reference_count) == offset(actual, 4) &&
            reference->reference_count.load(std::memory_order_relaxed) == 1 && prebody(context_.reference_frame.scalar.destruction),
            "Main-section seal needs the SAME live canonical122 generation before scalar");
        if (!font)
            require(!read(actual, 0x108) && !read(actual, 0x1c8) && !read(actual, 0x1cc) && !read(actual, 0x1ec),
                "First main-section domain precedes Font/header/shader/content publication");
        // This helper never reads Text188. The ordinary descriptor predicate
        // remains before that slot in Root's existing prefix/body schedules.
        live_graph(context_, context_.geometry.construction.primary.actual, context_.geometry.construction.internal188.actual,
            mesh, shadow_mesh, section, material);
        const auto regions = metadata_regions();
        for (const auto* model : {context_.geometry.construction.primary.actual, context_.geometry.construction.internal188.actual})
            for (std::size_t i = 0; i < regions.count; ++i)
                require(region(model, 0x188) && separate(model, 0x188, regions.entries[i].begin, regions.entries[i].bytes),
                    "Both native Model payloads must be disjoint from all stable main metadata");
        for (const auto native : {NativeGuiTextMainSectionLifetimeRegion{actual, 0x1f8},
            {mesh, NativeMeshPool::slot_bytes}, {section, NativeMeshSectionPool::slot_bytes},
            {material, NativeMaterialPool::slot_bytes}})
            for (std::size_t i = 0; i < regions.count; ++i)
                require(region(native.begin, native.bytes) &&
                    separate(native.begin, native.bytes, regions.entries[i].begin, regions.entries[i].bytes),
                    "Native Text/Mesh/Section/Material slots cannot alias main lifetime metadata");
        actual_ = actual; reference_ = reference; mesh_ = mesh; shadow_mesh_ = shadow_mesh;
        section_ = section; material_ = material; primary_ = context_.geometry.construction.primary.actual;
        internal_ = context_.geometry.construction.internal188.actual; empty_font_sealed_ = !font;
        phase_ = Phase::sealed; leave();
    } catch (...) { fail(); throw; }
}
void NativeGuiTextMainSectionLifetimeStorageFrame::admit(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) const {
    require(aliases(context_, *this), "Main-section operation/context/reference aliases cannot change");
    const bool font = with_font(context_);
    const auto& p = *prefix(context_); const auto& f = p.text.factory_context; const auto& pub = p.text.publication;
    const auto& scalar = context_.reference_frame.scalar; const auto& d = scalar.destruction;
    if (stage == Stage::bound && !progress && phase_ == Phase::prepared) {
        require(!actual_ && !reference_ && main_fresh(context_) && (font || shader_unused(context_, shader_, false, false)),
            "Prepared Main bound is only an exact untouched reservation, never a fallback after entry");
        if (!actual) return; // pure pre122/db55/379 reservation; no native/getter access.
        require(pub.bind_complete && !pub.bind_failed && pub.reference && !pub.scalar_entered && !pub.scalar_failed &&
            !pub.companion_disposal_entered && !pub.disposed && (font || f.publication.font_shader_frame == shader_) &&
            p.text.factory.raw_slot == actual && p.text.factory.complete && !p.text.factory.failure &&
            !scalar.started && !scalar.complete && prebody(d), "Prepared bound Text must be its canonical unentered122");
        require(pub.reference->phase() == NativeGuiTextRawModelReference::Phase::bound &&
            pub.reference->storage() == actual && &pub.reference->storage_context() == &f.publication &&
            pub.reference->matches_reference_frame(context_.reference_frame), "Prepared bound reservation cannot substitute122");
        p.authentication.require_raw_model_transport_receiver(actual, p.text);
        return; // no main Section/Mesh/current-field permission.
    }
    require(phase_ == Phase::sealed && actual_ && reference_ && (font || empty_font_sealed_) &&
        geometry_complete(context_, actual_, mesh_) && shadow_complete(context_, shadow_mesh_) &&
        main_complete(context_, actual_, mesh_, section_, material_), "Main permission requires its private sealed whole graph");
    if (stage == Stage::bound && !actual && !progress) return; // metadata reservation only, including after producers.
    require(actual && actual == actual_ && !pub.bind_failed && pub.bind_complete && pub.reference == reference_ &&
        !pub.scalar_failed && !pub.scalar_returned && !pub.companion_disposal_entered && !pub.disposed &&
        !scalar.complete && !scalar.destructor_returned && !scalar.pool_returned &&
        !d.complete && !d.base_returned && !d.exception_cleanup_started &&
        p.text.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete &&
        p.text.factory.complete && !p.text.factory.failure &&
        !p.authentication_acquired.poisoned && !p.authentication_acquired.active,
        "Main independent records reject failed/returned objects BEFORE any pointer access");
    const bool raw = stage == Stage::raw118_preentry || stage == Stage::raw118_body || stage == Stage::base;
    if (!font)
        require(shader_unused(context_, shader_, raw, false) && no_shader_release(d.shader),
            "Font-empty Main requires private unused/actual-null-retirement shader history");
    if (raw) require(progress == &d && pub.scalar_entered && scalar.started && scalar.destructor_entered,
        "Main raw118 stage requires exact canonical scalar.destruction");
    else require(!progress, "Main pre-scalar stage cannot accept a replacement destruction record");
    if (stage != Stage::raw118_body && stage != Stage::base)
        require(prebody(d) && !scalar.flags_captured && !scalar.pool_return_entered,
            "Main pre-body stage cannot replay member/late-flags work");
    switch (stage) {
    case Stage::bound: case Stage::before_scalar:
        require(!pub.scalar_entered && !scalar.started && !scalar.destructor_entered && !d.started,
            "Main bound/scalar permission precedes owner scalar entry"); break;
    case Stage::scalar_preentry:
        require(pub.scalar_entered && !scalar.started && !scalar.destructor_entered && !d.started,
            "Main scalar preentry precedes real scalar/destructor effects"); break;
    case Stage::raw118_preentry:
        require(!d.started && !d.destination && !d.base_entered && d.native_eh_state == -1,
            "Main raw118 preentry has no destructor body yet"); break;
    case Stage::raw118_body:
        require(d.started && d.destination == actual && !d.base_entered,
            "Main raw118 body needs exact actual entry before base"); break;
    case Stage::base:
        require(d.started && d.destination == actual && d.base_entered && d.native_eh_state == -1 &&
            (d.normal_actions_returned & 0x7eu) == 0x7eu,
            "Main base follows real normal member returns"); break;
    }
    if (header_entered(d) || stage == Stage::base) {
        require(font ? normal_header_returned(d) : null_header_returned(d),
            font ? "Main structural boundary requires actual bit6 normal return" :
                "Main ended empty header requires actual bit6 normal NULL return");
        return; // No four Font words, ended188, native Mesh/Section/Material or companion lookup.
    }
    require(reference_->storage() == actual && &reference_->storage_context() == &f.publication &&
        reference_->matches_reference_frame(context_.reference_frame) &&
        (stage == Stage::bound || stage == Stage::before_scalar ?
            reference_->phase() == NativeGuiTextRawModelReference::Phase::bound :
            reference_->phase() == NativeGuiTextRawModelReference::Phase::retiring), "Main live stage cannot replace canonical122");
    p.authentication.require_raw_model_transport_receiver(actual, p.text);
    if (!font)
        require(!read(actual, 0x108) && !read(actual, 0x1c8) && !read(actual, 0x1cc) && !read(actual, 0x1ec),
            "Main before-header domain still requires actual empty Font/header/shader");
    require(!primary_release_started(context_), "Main live graph cannot be polled after its owning release can enter");
    live_graph(context_, primary_, internal_, mesh_, shadow_mesh_, section_, material_);
}
void NativeGuiTextMainSectionLifetimeStorageFrame::require_live(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) const {
    try { enter(); admit(actual, stage, progress); leave(); }
    catch (...) { fail(); throw; }
}
NativeGuiTextMainSectionLifetimeRegions NativeGuiTextMainSectionLifetimeStorageFrame::metadata_regions() const noexcept {
    NativeGuiTextMainSectionLifetimeRegions r; const auto& c = context_;
    append(r, this, sizeof(*this)); append(r, &c, sizeof(c)); append(r, &c.geometry, sizeof(c.geometry));
    append(r, &c.construction, sizeof(c.construction)); append(r, &c.frame, sizeof(c.frame));
    append(r, &c.acquired, sizeof(c.acquired)); append(r, &c.reference_frame, sizeof(c.reference_frame));
    append(r, &c.construction.resources, sizeof(c.construction.resources));
    return r; // exact eight; no native name/header or unconstructed forward prefix read.
}
NativeGuiTextMainSectionLifetimeRegions NativeGuiTextMainSectionLifetimeStorageFrame::live_regions(void* actual, Stage stage,
    const NativeGuiTextRetirementAcquired* progress) const {
    try {
        enter(); admit(actual, stage, progress); auto r = metadata_regions();
        if (phase_ == Phase::sealed && actual && !header_entered(context_.reference_frame.scalar.destruction) &&
            stage != Stage::base && !primary_release_started(context_)) {
            append(r, mesh_, NativeMeshPool::slot_bytes); append(r, section_, NativeMeshSectionPool::slot_bytes);
            append(r, material_, NativeMaterialPool::slot_bytes);
            const auto& mesh = *static_cast<const NativeMeshStorage*>(mesh_); // admit authenticated it alive.
            append(r, mesh.draw_sections_54.data_00, std::size_t(mesh.draw_sections_54.capacity_08) * 4u);
        }
        leave(); return r;
    } catch (...) { fail(); throw; }
}
bool NativeGuiTextMainSectionLifetimeStorageFrame::post_text_quiescent() const noexcept {
    if (active_ || failure_ || phase_ == Phase::failed || !aliases(context_, *this)) return false;
    const bool font = with_font(context_);
    if (phase_ == Phase::prepared && !actual_ && !reference_)
        return main_fresh(context_) && (font || shader_unused(context_, shader_, false, false)); // unused MAIN debt only.
    if (phase_ != Phase::sealed || !actual_ || !reference_ || (!font && !empty_font_sealed_) ||
        !geometry_complete(context_, actual_, mesh_) || !shadow_complete(context_, shadow_mesh_) ||
        !main_complete(context_, actual_, mesh_, section_, material_)) return false;
    const auto& p = *prefix(context_); const auto& a = p.text.factory; const auto& pub = p.text.publication;
    const auto& scalar = context_.reference_frame.scalar; const auto& d = scalar.destruction;
    if (pub.reference != reference_ || a.phase != NativeGuiTextDefaultFactoryStorageAcquired::Phase::complete ||
        !a.complete || a.failure || !pub.bind_complete || pub.bind_failed || !pub.scalar_entered ||
        !pub.scalar_returned || pub.scalar_failed || !pub.companion_disposal_entered || !pub.disposed ||
        !scalar.started || !scalar.complete || !scalar.destructor_entered || !scalar.destructor_returned ||
        !scalar.flags_captured || !(scalar.captured_flags_byte & 1u) || !scalar.pool_return_entered || !scalar.pool_returned ||
        !d.started || !d.complete || d.destination != actual_ || d.exception_cleanup_started ||
        !d.base_entered || !d.base_returned || (font ? !normal_header_returned(d) :
            !null_header_returned(d) || !no_shader_release(d.shader) || !shader_unused(context_, shader_, false, true)) ||
        p.authentication_acquired.active || p.authentication_acquired.poisoned ||
        p.widget_acquired.active || p.widget_acquired.poisoned ||
        p.widget_acquired.base != NativeGuiWidgetRawModelLifetimeAcquired::Phase::complete ||
        p.widget_acquired.scene != NativeGuiWidgetRawModelLifetimeAcquired::Phase::complete ||
        p.widget_acquired.glyph != NativeGuiWidgetRawModelLifetimeAcquired::Phase::fresh ||
        !p.widget_acquired.scene4c.entered || !p.widget_acquired.scene4c.returned || !p.widget_acquired.scene4c.unlink.complete ||
        (p.widget_acquired.late4c.entered && (!p.widget_acquired.late4c.returned || !p.widget_acquired.late4c.unlink.complete)) ||
        !route_settled(p.widget.primary, p.primary_calls) || !route_settled(p.widget.internal188, p.internal_calls)) return false;
    // No geometry_meshes_settled wrapper: Root may compose this checker there.
    // These enclosing zero returns include canonical flags1 Mesh->Section->
    // Material pool returns for the sealed unchanged sole graph. No erased
    // entry/companion/registry absence or ended actual payload is inspected.
    return model_mesh_returned(context_.geometry.construction.primary, mesh_) &&
        model_mesh_returned(context_.geometry.construction.internal188, shadow_mesh_);
}
void NativeGuiTextMainSectionLifetimeStorageFrame::require_post_text_quiescent() const {
    if (!post_text_quiescent()) {
        try { throw std::invalid_argument("Main retirement requires whole Text/Models/sole nested pool custody"); }
        catch (...) { fail(); throw; }
    }
}
bool NativeGuiTextMainSectionLifetimeStorageFrame::requires_process_retention() const noexcept {
    if (active_ || phase_ == Phase::failed || failure_) return true;
    if (phase_ == Phase::fresh) return false; // forward graph may not be constructed yet.
    if (!aliases(context_, *this)) return true;
    const bool font = with_font(context_);
    if (phase_ == Phase::prepared) return !main_fresh(context_) || (!font && !shader_unused(context_, shader_, false, false));
    if (!main_complete(context_, actual_, mesh_, section_, material_) || !cache_quiescent(context_.acquired.section.material.cache.get())) return true;
    const auto& p = *prefix(context_); const auto& scalar = context_.reference_frame.scalar;
    const auto bad_route = [](const NativeGuiWidgetRawModelRoute& r) noexcept {
        return r.tree_acquired.poisoned || r.node_acquired.poisoned || r.node_acquired.entry_failed ||
            r.node_acquired.capacity_failed || (r.record.lifetime.started && !r.record.lifetime.complete);
    };
    if (p.text.publication.bind_failed || p.text.publication.scalar_failed || (scalar.started && !scalar.complete) ||
        p.authentication_acquired.active || p.authentication_acquired.poisoned ||
        p.widget_acquired.active || p.widget_acquired.poisoned || bad_route(p.widget.primary) || bad_route(p.widget.internal188) ||
        !calls_settled(p.primary_calls) || !calls_settled(p.internal_calls)) return true;
    if (!font && shader_) {
        const auto s = shader_->credit_progress();
        if (s.failed || s.active || s.producer != Shader::RawPhase::fresh || s.credit != Shader::CreditPhase::none ||
            s.consumer != Shader::Consumer::none || shader_->phase != Shader::Phase::fresh) return true;
    }
    return false; // healthy bound debt is NOT post-Text or provider/reset permission.
}
} // namespace bsp
