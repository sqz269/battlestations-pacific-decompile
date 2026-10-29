#include "bsp/native_gui_text_main_section_storage.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_string_pool_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_node_root_propagation.hpp"
#include <atomic>
#include <exception>
#include <stdexcept>
#include <utility>

namespace bsp {
namespace {
using Word = std::uint32_t;
using Context = NativeGuiTextMainSectionStorageContext;
using Frame = NativeGuiTextMainSectionStorageFrame;
using Acquired = NativeGuiTextMainSectionStorageAcquired;
using Phase = NativeGuiTextMainSectionStoragePhase;
constexpr Word text_profile = 0x00d5c6c8, model_profile = 0x00d62de8;
static_assert(sizeof(void*) == sizeof(Word));
Word bits(const volatile void* p) noexcept { return reinterpret_cast<Word>(p); }
void* at(const void* p, Word offset = 0) noexcept { return reinterpret_cast<void*>(bits(p) + offset); }
Word read(const void* p, Word offset = 0) noexcept {
    return *static_cast<const volatile Word*>(at(p, offset));
}
std::uint8_t byte(const void* p, Word offset) noexcept {
    return *static_cast<const volatile std::uint8_t*>(at(p, offset));
}
void require(bool yes, const char* reason) { if (!yes) throw std::logic_error(reason); }
struct Region { const volatile void* data; std::size_t size; };
bool valid(Region r) noexcept {
    return r.data && static_cast<std::uint64_t>(bits(r.data)) + r.size <= 0x100000000ull;
}
bool disjoint(Region l, Region r) noexcept {
    const auto a = static_cast<std::uint64_t>(bits(l.data));
    const auto b = static_cast<std::uint64_t>(bits(r.data));
    return a + l.size <= b || b + r.size <= a;
}
void location(Acquired& a, Word site, Acquired::Boundary boundary) noexcept {
    a.active_call_site = site; a.boundary = boundary;
}
bool fresh(const NativeGuiTextSectionStorageRelease& a) noexcept {
    return !a.identity && !a.captured_decrement && !a.entered && !a.decremented &&
        !a.zero_entered && !a.zero_returned && !a.complete;
}
void require_fresh(const Acquired& a) {
    const auto& s = a.section;
    require(a.phase == Phase::fresh && a.boundary == Acquired::Boundary::none &&
        a.escaped_boundary == Acquired::Boundary::none && !a.active_call_site && !a.escaped_call_site &&
        !a.destination && !a.captured188 && !a.first_primary && !a.current_primary &&
        !a.first_geometry && !a.current_geometry && !a.section_count && !a.mesh_gate_read &&
        !a.creation_skipped && !a.name_construct_entered && !a.name_construct_returned && !a.captured_name_data &&
        !a.name_return_entered && !a.name_cleanup_returned && a.native_eh_state == -1 &&
        !s.section.started && !s.section.creator && !s.section.companion &&
        !s.section.owner_record && !s.section.registered && !s.section_creator &&
        s.material.phase == NativeMaterialFactoryAcquired::Phase::empty && !s.material.native_site && !s.material.cache &&
        !s.material.effect && !s.material.raw_slot && !s.material.material && !s.material.companion &&
        !s.material.owner_record && !s.material.registered && !s.material_creator &&
        !s.appended && !s.material_published && fresh(s.section_release) && fresh(s.material_release) &&
        !a.pair_decrement && !a.parenting_flag_read && !a.parenting_entered && !a.parenting_returned &&
        !a.root_flag_read && !a.root_entered && !a.root_returned &&
        !a.parenting_receiver && !a.root_receiver && !a.parenting.started && !a.parenting.complete &&
        !a.parenting.prepend.started && !a.parenting.prepend.complete &&
        !a.parenting.recursive_unregister.started && !a.parenting.recursive_unregister.complete && !a.failure,
        "post-constructor Text main sections require fresh retained invocation records");
}
template<class Check>
void model_backing(const NativeModelStorageDispatchContext& m, Check check) {
    auto& s = m.model.storage; auto& n = s.node;
    for (const Region r : {Region{&m, sizeof(m)}, {&m.model, sizeof(m.model)},
        {&m.actual, sizeof(m.actual)}, {&m.reference, sizeof(m.reference)},
        {&m.construction, sizeof(m.construction)}, {&m.lifetime_frame, sizeof(m.lifetime_frame)},
        {&m.lifetime, sizeof(m.lifetime)}, {&m.release_frame, sizeof(m.release_frame)},
        {&m.release, sizeof(m.release)}, {&s, sizeof(s)}, {&s.pool_01090054, sizeof(s.pool_01090054)},
        {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())},
        {&s.constants, sizeof(s.constants)}, {&n, sizeof(n)}, {&n.world, sizeof(n.world)},
        {&n.scenes, sizeof(n.scenes)}, {&n.trees, sizeof(n.trees)},
        {&n.trees.dispatch, sizeof(n.trees.dispatch)}, {&n.scenes.children, sizeof(n.scenes.children)},
        {&n.scenes.gates, sizeof(n.scenes.gates)}, {&n.trees.frames, sizeof(n.trees.frames)},
        {&n.trees.point_lights, sizeof(n.trees.point_lights)}, {&n.strings, sizeof(n.strings)},
        {m.model.actual_profile_00d62de8, 0x58}}) check(r);
    if (m.reference) check({&*m.reference, sizeof(*m.reference)});
}
template<class Check>
void borrowed(const Context& c, Check check) {
    auto& f = c.text.factory_context; auto& t = f.publication; auto& r = c.resources;
    for (const Region region : {Region{&c, sizeof(c)}, {&c.text, sizeof(c.text)},
        {&c.authentication, sizeof(c.authentication)}, {&f, sizeof(f)}, {&c.text.factory, sizeof(c.text.factory)},
        {&c.text.publication, sizeof(c.text.publication)}, {&t, sizeof(t)}, {&t.data, sizeof(t.data)},
        {&t.owners, sizeof(t.owners)}, {&t.construction_context, sizeof(t.construction_context)},
        {&t.construction_frame, sizeof(t.construction_frame)}, {&t.construction, sizeof(t.construction)},
        {&t.retirement, sizeof(t.retirement)}, {&f.reference_frame, sizeof(f.reference_frame)},
        {f.companion_storage, f.companion_storage_bytes},
        {&c.primary_context, sizeof(c.primary_context)}, {&c.primary_frame, sizeof(c.primary_frame)},
        {&c.primary, sizeof(c.primary)}, {c.primary_context.actual_name_header8, 8},
        {&c.geometry_context, sizeof(c.geometry_context)}, {&c.geometry_frame, sizeof(c.geometry_frame)},
        {&c.geometry, sizeof(c.geometry)}, {&r, sizeof(r)}, {&r.geometry, sizeof(r.geometry)},
        {&r.materials, sizeof(r.materials)}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.strings, sizeof(r.strings)}, {&r.string_process, sizeof(r.string_process)},
        {&r.effects, sizeof(r.effects)}, {&r.effects.effects, sizeof(r.effects.effects)},
        {&r.effects.effects.construction, sizeof(r.effects.effects.construction)},
        {&r.effects.effects.programs, sizeof(r.effects.effects.programs)},
        {&r.effects.effects.owners, sizeof(r.effects.effects.owners)},
        {r.guidefault_00d5c25c, 11}, {r.material_vtable_00d5e520, 4},
        {t.data.data_at(text_profile, 0x84), 0x84},
        {&r.parenting, sizeof(r.parenting)}, {&r.parenting.dispatch, sizeof(r.parenting.dispatch)},
        {&r.strings.actual_published_01090aa8, sizeof(void*)},
        {&r.strings.actual_small_returns_disabled_01090aa4, sizeof(Word)},
        {&r.strings.actual_manager_publication_01090aa0, sizeof(void*)},
        {&r.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)}}) check(region);
    if (f.disposal.context_bytes) check({f.disposal.context, f.disposal.context_bytes});
    model_backing(c.geometry_context.primary, check);
    model_backing(c.geometry_context.internal188, check);
}
void live_model(void* actual, const NativeModelStorageDispatchContext& m) {
    auto& s = m.model.storage;
    require(actual && !(bits(actual) & 3u) && m.actual == actual && m.reference &&
        m.construction.started && m.construction.complete && m.construction.metadata_bound &&
        m.construction.node_constructor_returned && m.construction.tail_constructed &&
        m.construction.actual_model == actual && m.construction.context == &s &&
        !m.lifetime.started && !m.lifetime.complete && !m.lifetime.scalar_started &&
        !m.lifetime.scalar_complete && !m.lifetime.flags_read && !m.lifetime.pool_returned &&
        !m.lifetime.node.started && !m.lifetime.node.complete && !m.release.started && !m.release.complete,
        "main section Model must have its SAME independent completed unretired records BEFORE payload");
    require(!m.reference->retired() && s.node.scenes.owners.find(actual) == &*m.reference &&
        static_cast<void*>(&m.reference->reference_count) == at(actual, 4),
        "main section Model needs its SAME canonical481 actual+4 binding");
    const auto& pool = s.pool_01090054.storage();
    const Word index = NativeModelPool::live_slab_index(actual);
    require(pool.slabs_28 && index < pool.slab_count_2c, "main section Model needs its actual pool slab");
    const auto* slab = pool.slabs_28[index];
    const Word offset = bits(actual) - bits(slab);
    require(slab && offset < NativeModelPool::slot_bytes * NativeModelPool::slots_per_slab &&
        offset % NativeModelPool::slot_bytes == 0, "main section Model needs its exact allocated pool slot");
    const Word count = *reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3140);
    require(count <= NativeModelPool::slots_per_slab, "main section Model pool free-count is invalid");
    const auto* free = reinterpret_cast<const volatile std::uint16_t*>(slab + 0x3100);
    for (Word i = 0; i != count; ++i)
        require(free[i] < NativeModelPool::slots_per_slab && free[i] != offset / NativeModelPool::slot_bytes,
            "main section cannot read a returned Model payload");
    require(m.reference->reference_count.load(std::memory_order_relaxed) > 0 && read(actual) == model_profile,
        "main section requires CURRENT live Model family and credit");
}
void empty_node(void* actual) {
    require(!read(actual, 0x30) && !read(actual, 0x34) && !read(actual, 0x38) &&
        !read(actual, 0x3c) && !read(actual, 0x40) && !byte(actual, 0x44) &&
        !read(actual, 0xa0) && !read(actual, 0xa4) && !read(actual, 0x130) &&
        !read(actual, 0x164) && !read(actual, 0x168) && !read(actual, 0x16c) &&
        !read(actual, 0x170) && !read(actual, 0x174),
        "main section parenting excludes reached hierarchy/attachment/scene/retained/point-light state");
}
void live_mesh(void* actual, const Context& c) {
    const auto& g = c.geometry;
    require(actual && actual == g.captured_mesh && !g.creator_zero_entered && !g.creator_zero_returned &&
        g.creator_result > 0 && g.mesh.companion &&
        c.text.factory_context.publication.owners.find(actual) == g.mesh.companion,
        "main section requires SAME still-owned real379 mesh, not a terminal diagnostic identity");
    auto* const typed = dynamic_cast<NativeMeshReference*>(g.mesh.companion);
    require(typed && &typed->storage() == actual && static_cast<void*>(&typed->reference_count) == at(actual, 4),
        "main section geometry needs its genuine canonical actual Mesh object");
    require(typed->reference_count.load(std::memory_order_relaxed) > 0 && read(actual) == 0x00d62d60,
        "main section geometry cannot be an entered or completed terminal payload");
}
void authenticate(NativeGuiTextRawModelReference& reference, Context& c, void* raw) {
    auto& f = c.text.factory_context; auto& p = c.text.publication;
    c.authentication.require_raw_model_transport_receiver(raw, c.text);
    require(p.bind_complete && p.reference == &reference && !p.scalar_entered &&
        reference.phase() == NativeGuiTextRawModelReference::Phase::bound &&
        reference.storage() == raw && &reference.storage_context() == &f.publication &&
        &f.publication.construction == &f.construction && &f.publication.retirement.resources == &c.resources &&
        &f.publication.retirement.base == &c.authentication.context(),
        "main section requires exact completed canonical raw122 outside scalar/constructor entry");
}
void provenance(Context& c, void* raw) {
    auto& f = c.text.factory_context; auto& r = c.resources;
    const auto& pc = c.primary_context; const auto& p = c.primary;
    const auto* receipt = p.publication();
    auto& m = c.geometry_context.primary; auto& i = c.geometry_context.internal188;
    const auto& g = c.geometry;
    const auto& nested = f.construction.sections.model;
    require(&f.publication.construction_context.sections == &r && f.construction.started &&
        f.construction.complete && f.construction.base_complete && f.construction.derived_complete &&
        f.construction.sections_entered && f.construction.destination == raw &&
        !f.construction.base_lifetime_ended && !f.construction.exception_cleanup_started &&
        !f.construction.consumed_cleanup_actions && f.construction.failure == decltype(f.construction.failure)::none &&
        f.construction.sections.started && f.construction.sections.complete &&
        !f.construction.sections.exception_cleanup_started &&
        f.construction.sections.failure == decltype(f.construction.sections.failure)::none &&
        f.construction.sections.model_registered && f.construction.sections.model_constructed &&
        f.construction.sections.shadow_published && nested.association.phase == NativeModelStorageGeometryPhase::complete &&
        nested.association.context == &r.association &&
        nested.association.frame == &f.publication.construction_frame.sections.association,
        "main section requires SAME actual completed raw93/85/188/B75170 records");
    require(c.primary_frame.matches(pc, p, NativeGuiTextPrimaryModelStoragePhase::complete) && receipt &&
        !p.failure && p.published && p.flags_cleared && p.constructor_returned && p.canonical_bound &&
        receipt->text_identity() == raw && receipt->model_identity() == m.actual &&
        receipt->factory_context() == &f && receipt->factory_acquisition() == &c.text.factory &&
        &pc.factory_context == &f && &pc.factory == &c.text.factory && &pc.publication == &c.text.publication &&
        receipt->primary_record() == &pc.primary && receipt->primary_dispatch_record() == &m &&
        receipt->construction() == &m.construction && receipt->pool() == &m.model.storage.pool_01090054 &&
        &pc.primary_node_context.record == &m &&
        &pc.primary.actual == &m.actual && &pc.primary.reference == &m.reference &&
        &pc.primary.construction == &m.construction && &pc.primary.lifetime_frame == &m.lifetime_frame &&
        &pc.primary.lifetime == &m.lifetime && &pc.primary.release_frame == &m.release_frame &&
        &pc.primary.release == &m.release && &pc.internal188 == &i &&
        &i.model == &r.model && &i.actual == &nested.actual && &i.reference == &nested.reference &&
        &i.construction == &nested.construction && &i.lifetime_frame == &nested.lifetime_frame &&
        &i.lifetime == &nested.lifetime && &i.release_frame == &nested.release_frame && &i.release == &nested.release &&
        &f.publication.retirement.model == &nested && m.actual != i.actual && &m.actual != &i.actual &&
        &m.reference != &i.reference && &m.construction != &i.construction &&
        &m.lifetime_frame != &i.lifetime_frame && &m.lifetime != &i.lifetime &&
        &m.release_frame != &i.release_frame && &m.release != &i.release,
        "main section requires private db55 publication and exact distinct primary/internal188 aliases");
    require(c.geometry_frame.matches(c.geometry_context, g, NativeGuiTextMainGeometryStoragePhase::complete) &&
        g.phase == NativeGuiTextMainGeometryStoragePhase::complete && !g.failure && g.destination == raw &&
        g.primary_captured && g.captured_primary == m.actual && g.mesh_entered && g.mesh_returned &&
        g.mesh.started && g.mesh.creator == g.captured_mesh &&
        g.mesh.constructor_complete && g.mesh.registered && g.mesh.owner_record &&
        g.association_entered && g.association_returned && g.association.phase == NativeModelStorageGeometryPhase::complete &&
        g.association.context == &c.geometry_context.association && g.association.actual_model == m.actual &&
        g.association.frame && g.association.frame->phase() == NativeModelStorageGeometryPhase::complete &&
        g.association.requested_captured && g.association.captured_requested == g.captured_mesh &&
        g.association.previous_captured && !g.association.captured_previous &&
        g.association.geometry_published && g.association.increment_completed && g.association.captured_increment &&
        g.creator_decrement && g.creator_release_entered && g.creator_decremented && g.creator_release_returned &&
        !g.creator_zero_entered && !g.creator_zero_returned && g.creator_result > 0 &&
        g.position_prepared && g.position_entered && g.position_returned && g.transform.entered &&
        g.transform.body_returned && g.transform.captured_target == 0x00b6db10 &&
        g.transform.admitted_profile == model_profile && !g.transform.failed,
        "main section requires exact completed379 affinity and real settled association/creator provenance");
    auto& s = m.model.storage; auto& n = s.node; auto& sn = r.model.storage.node;
    auto& effects = r.effects.effects;
    auto* const node_dispatch = dynamic_cast<NativeModelStorageNodeDispatch*>(&r.parenting.dispatch);
    require(&c.geometry_context.geometry == &r.geometry && &c.geometry_context.association.storage == &s &&
        &r.association.storage == &r.model.storage && &r.parenting.node == &sn &&
        &s.pool_01090054 == &r.model.storage.pool_01090054 &&
        &n.strings == &r.strings && &sn.strings == &r.strings && &n.world == &sn.world &&
        &n.scenes.owners == &f.publication.owners && &sn.scenes.owners == &f.publication.owners &&
        &n.trees.owners == &f.publication.owners && &sn.trees.owners == &f.publication.owners &&
        &n.trees.point_lights == &f.publication.retirement.base.nodes.point_lights &&
        &sn.trees.point_lights == &n.trees.point_lights && node_dispatch &&
        dynamic_cast<NativeModelStorageNodeDispatch*>(&sn.scenes.children) == node_dispatch &&
        dynamic_cast<NativeModelStorageDispatch*>(&sn.trees.dispatch) &&
        &n.scenes.decrement_00ce2220 == &n.trees.decrement_00ce2220 &&
        &sn.scenes.decrement_00ce2220 == &n.scenes.decrement_00ce2220 &&
        &r.geometry.actual_owners() == &f.publication.owners &&
        &r.materials.retained_owners == &f.publication.owners &&
        &r.materials.parameter_names == &r.material_strings &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()) &&
        effects.cache == &r.effects && &r.effects.strings == &r.material_strings &&
        &effects.construction.strings == &r.material_strings && &effects.programs.strings == &r.material_strings &&
        &effects.owners.string_storage() == &r.material_strings &&
        &effects.owners.actual_owners() == &f.publication.owners &&
        &effects.construction.current_renderer_00f8d394 == &effects.programs.current_renderer_00f8d394 &&
        r.guidefault_00d5c25c == f.publication.data.data_at(0x00d5c25c, 11) &&
        r.material_vtable_00d5e520 == f.publication.data.data_at(0x00d5e520, 4) &&
        m.model.actual_profile_00d62de8 == f.publication.data.data_at(model_profile, 0x58) &&
        i.model.actual_profile_00d62de8 == m.model.actual_profile_00d62de8,
        "main section requires SAME genuine raw strings/geometry/material/cache/renderer/provider domains");
    live_model(m.actual, m); live_model(i.actual, i); empty_node(m.actual); empty_node(i.actual);
}
void return_name(void* captured_data, const volatile Word* name, Context& c) {
    if (!captured_data) return;
    const Word size = name[0] + 1u; // AFTER state disarm, BEFORE current getter.
    auto& strings = c.resources.strings;
    auto* const pool = native_string_pool_get_or_create_00419cc0(
        strings.actual_published_01090aa8, strings.actual_manager_publication_01090aa0);
    return_native_string_pool_00bd1510(pool, captured_data, size, strings.actual_small_returns_disabled_01090aa4);
}
template<class Resource>
void release_creator(Resource*& creator, NativeTextureSurfaceReferenceIncrement target,
    NativeGuiTextSectionStorageRelease& a, Context& c) {
    void* const raw = std::exchange(creator, nullptr);
    require(raw && target && !a.entered, "main section creator release needs one actual captured obligation");
    a.identity = raw; a.captured_decrement = target; a.entered = true;
    a.result = target(static_cast<volatile long*>(at(raw, 4))); a.decremented = true;
    if (a.result == 0) {
        a.zero_entered = true;
        dispatch_native_render_actual_owner_zero(c.resources.geometry.actual_owners(), raw);
        a.zero_returned = true;
    }
    a.complete = true; // metadata ONLY after actual terminal return
}
} // namespace

NativeGuiTextMainSectionStorageFrame::NativeGuiTextMainSectionStorageFrame(Context& c, Acquired& a) noexcept
    : context_(c), acquired_(a) {}
NativeGuiTextMainSectionStorageFrame::~NativeGuiTextMainSectionStorageFrame() noexcept {
    if (phase_ == Phase::entered || phase_ == Phase::failed) std::terminate();
}
void ensure_native_gui_text_post_constructor_sections_storage_00ab8530(
    NativeGuiTextRawModelReference& reference, Context& c, Frame& f, Acquired& a) {
    require(&f.context_ == &c && &f.acquired_ == &a, "main section needs SAME installed frame/context/acquisition");
    try {
        require(f.phase_ == Phase::fresh, "main section Frame cannot replay"); require_fresh(a);
        const Region writes[] = {{&f, sizeof(f)}, {&a, sizeof(a)}};
        require(valid(writes[0]) && valid(writes[1]) && !(bits(&f) & 3u) && !(bits(&a) & 3u) &&
            disjoint(writes[0], writes[1]), "main section writable records need separate aligned32bit extents");
        borrowed(c, [&](Region r) {
            require(valid(r) && disjoint(writes[0], r) && disjoint(writes[1], r),
                "main section writes cannot alias known retained provider/record backing");
        });
        for (const Region native : {Region{c.text.factory.raw_slot, 0x1f8},
            {c.geometry_context.primary.actual, 0x188}, {c.geometry_context.internal188.actual, 0x188},
            {c.geometry.captured_mesh, sizeof(NativeMeshStorage)}})
            require(valid(native) && disjoint(writes[0], native) && disjoint(writes[1], native),
                "main section writes cannot alias actual Text/Model/mesh storage");
        f.phase_ = a.phase = Phase::entered;
        location(a, 0, Acquired::Boundary::admission);
        void* const raw = c.text.factory.raw_slot;
        authenticate(reference, c, raw); provenance(c, raw); a.destination = raw;
        auto& r = c.resources; auto& primary = c.geometry_context.primary; auto& internal = c.geometry_context.internal188;
        location(a, 0, Acquired::Boundary::initial188); // initial188 read; no guessed instruction VA
        a.captured188 = reinterpret_cast<void*>(read(raw, 0x188));
        require(a.captured188 == internal.actual, "post-constructor sections never allocate or replace constructor188");
        location(a, 0x00ab86cb, Acquired::Boundary::model);
        a.first_primary = reinterpret_cast<void*>(read(raw, 0x4c));
        require(a.first_primary == primary.actual, "main arm requires CURRENT genuine published primary4C");
        live_model(a.first_primary, primary);
        a.first_geometry = reinterpret_cast<void*>(read(a.first_primary, 0x180)); // B74650
        require(a.first_geometry, "main arm excludes missing actual379 geometry");
        a.current_primary = reinterpret_cast<void*>(read(raw, 0x4c)); // AB86E8 reload
        require(a.current_primary == primary.actual, "main arm rejects foreign CURRENT4C replacement");
        live_model(a.current_primary, primary);
        a.current_geometry = reinterpret_cast<void*>(read(a.current_primary, 0x180)); // B74640
        live_mesh(a.current_geometry, c);
        a.section_count = read(a.current_geometry, 0x58); a.mesh_gate_read = true;
        if (a.section_count == 0) {
            auto& s = a.section;
            location(a, 0x00ab86fe, Acquired::Boundary::section);
            s.section_creator = r.geometry.create_native_section_00533fa0(s.section);
            require(s.section_creator, "main arm excludes native null section/fault continuation");
            location(a, 0x00ab8708, Acquired::Boundary::append);
            append_native_mesh_draw_section_00b73c60(*static_cast<NativeMeshStorage*>(a.current_geometry), s.section_creator);
            s.appended = true;
            location(a, 0x00ab8716, Acquired::Boundary::name_construct); a.name_construct_entered = true;
            construct_native_string_header_0041e870(const_cast<Word*>(&f.name_[0]), r.strings, r.guidefault_00d5c25c);
            a.name_construct_returned = true; a.native_eh_state = 5;
            location(a, 0x00ab8727, Acquired::Boundary::material);
            s.material_creator = create_native_material_from_effect_cache_00535320(
                const_cast<const Word*>(&f.name_[0]), r.materials.material_slots,
                r.geometry.actual_owners(), r.effects, s.material);
            require(s.material_creator && s.material.phase == NativeMaterialFactoryAcquired::Phase::complete &&
                s.material_creator->effect_7c, "main arm requires real nonnull GuiDefault material/effect");
            r.geometry.register_native_material_creator(s.material, r.materials, r.material_vtable_00d5e520);
            a.captured_name_data = reinterpret_cast<void*>(f.name_[1]); // AB872E BEFORE disarm
            a.native_eh_state = -1;
            location(a, 0x00ab8750, Acquired::Boundary::name_return); a.name_return_entered = true;
            return_name(a.captured_name_data, &f.name_[0], c); a.name_cleanup_returned = true;
            location(a, 0x00ab8758, Acquired::Boundary::material_publication);
            set_native_mesh_section_material_00b864c0(*s.section_creator, r.geometry.actual_owners(), s.material_creator);
            s.material_published = true;
            a.pair_decrement = r.model.storage.node.scenes.decrement_00ce2220; // AB875D ONCE
            location(a, 0x00ab8767, Acquired::Boundary::section_release);
            release_creator(s.section_creator, a.pair_decrement, s.section_release, c);
            location(a, 0x00ab8779, Acquired::Boundary::material_release);
            release_creator(s.material_creator, a.pair_decrement, s.material_release, c);
        } else a.creation_skipped = true; // no new creator/credit/name in this branch
        location(a, 0, Acquired::Boundary::parenting); // late15C branch; no guessed instruction VA
        a.parenting_flag = byte(raw, 0x15c); a.parenting_flag_read = true; // actual LATE epoch
        require(a.parenting_flag == 0, "main arm retains prior effects on reached unsupported nonzero15C");
        a.parenting_receiver = reinterpret_cast<void*>(read(raw, 0x188));
        require(a.parenting_receiver == internal.actual, "main arm rejects foreign CURRENT188 parenting receiver");
        live_model(a.parenting_receiver, internal); empty_node(a.parenting_receiver);
        f.parenting_.parent_argument = 0;
        location(a, 0x00ab879e, Acquired::Boundary::parenting); a.parenting_entered = true;
        set_native_raw_parent_00b6e680(a.parenting_receiver, f.parenting_, r.parenting, a.parenting);
        a.parenting_returned = true;
        a.root_flag = byte(raw, 0x15c); a.root_flag_read = true; // FRESH AFTER parenting
        require(a.root_flag == 0, "main arm retains reached parenting on changed unsupported15C");
        a.root_receiver = reinterpret_cast<void*>(read(raw, 0x188));
        require(a.root_receiver == internal.actual, "main arm rejects foreign CURRENT188 root receiver");
        live_model(a.root_receiver, internal); empty_node(a.root_receiver);
        location(a, 0x00ab87b4, Acquired::Boundary::root); a.root_entered = true;
        propagate_native_node_root_storage_00b6d890(a.root_receiver, nullptr, r.parenting.node.scenes.children);
        a.root_returned = true;
        require(f.phase_ == Phase::entered && a.phase == Phase::entered && !a.failure,
            "main section cannot complete after caught reentry or an escaped invocation");
        a.phase = f.phase_ = Phase::complete;
    } catch (...) {
        if (!a.failure) {
            a.escaped_boundary = a.boundary; a.escaped_call_site = a.active_call_site;
            a.failure = std::current_exception();
        }
        a.phase = f.phase_ = Phase::failed;
        throw; // ALL escapes retain: no inferred native EH, name cleanup, pool rollback or retry.
    }
}
} // namespace bsp
