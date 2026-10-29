#include "bsp/native_gui_text_reference.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/native_gui_text_primary_prefix_lifetime.hpp"
#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/native_material_effect_programs.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/native_vfs_name_resolution.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_gui_text_process.hpp"
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
        !d.normal_actions_entered && !d.normal_actions_returned && !d.unwind_actions_consumed &&
        !d.unwind_actions_returned;
}
template<class Check>
void shader_texture_backing(const NativeTextureCacheAcquired& a, Check check) {
    check(Region{&a, sizeof(a)});
    // Still-owned C++ objects only. A base extent is not the opaque derived
    // operation's full extent, which remains the existing caller obligation.
    if (a.resolution.operation) check(Region{a.resolution.operation.get(), sizeof(*a.resolution.operation)});
    if (a.fallback_resolution.operation)
        check(Region{a.fallback_resolution.operation.get(), sizeof(*a.fallback_resolution.operation)});
    if (a.fallback) shader_texture_backing(*a.fallback, check);
}
template<class Check>
void shader_cache_backing(const NativeMaterialEffectCacheAcquired& a, Check check) {
    check(Region{&a, sizeof(a)});
    if (a.key_resolution) {
        const auto& k = *a.key_resolution;
        check(Region{&k, sizeof(k)});
        if (k.changed_resolution.acquired)
            check(Region{k.changed_resolution.acquired.get(), sizeof(*k.changed_resolution.acquired)});
        if (k.original_resolution.acquired)
            check(Region{k.original_resolution.acquired.get(), sizeof(*k.original_resolution.acquired)});
    }
    const auto& loader = a.loader;
    if (loader.name_resolution.acquired)
        check(Region{loader.name_resolution.acquired.get(), sizeof(*loader.name_resolution.acquired)});
    if (loader.programs) check(Region{loader.programs.get(), sizeof(*loader.programs)});
    if (loader.constructor_texture) shader_texture_backing(*loader.constructor_texture, check);
    if (loader.fallback) shader_cache_backing(*loader.fallback, check);
}
void require_shader_quiescent(const NativeGuiTextRawModelReferenceContext& c) {
    const auto* const frame = c.font_shader_frame;
    if (!frame) return;
    require(representable({frame, sizeof(*frame)}) && !(bits(frame) & 3u),
        "Raw Text shader reservation requires aligned representable retained Frame storage");
    require(frame->phase == NativeGuiTextShaderStorageFrame::Phase::fresh ||
        frame->phase == NativeGuiTextShaderStorageFrame::Phase::complete,
        "Raw Text binding/scalar excludes an entered or failed reserved shader invocation");
}
template<class Check>
void known_shader_borrowed(const NativeGuiTextRawModelReferenceContext& c, Check check) {
    require_shader_quiescent(c);
    const auto* const frame = c.font_shader_frame;
    if (!frame) return;
    check(Region{frame, sizeof(*frame)});
    const auto* const context = frame->prepared_raw_context();
    require(context && representable({context, sizeof(*context)}),
        "Raw Text shader reservation needs its immutable real prepared context");
    check(Region{context, sizeof(*context)});
    check(Region{&context->fonts, sizeof(context->fonts)});
    const Region platform{context->configuration_0109cf04.identity(), sizeof(void*)};
    require(representable(platform), "Raw Text shader needs its actual retained platform publication CELL");
    check(platform); // No current platform/Font/native resource/header read.
    shader_cache_backing(frame->cache, check);
}
void fresh_shader_binding(const NativeGuiTextRawModelReferenceContext& c) {
    const auto* const f = c.font_shader_frame;
    if (!f) return;
    using Frame = NativeGuiTextShaderStorageFrame;
    require(f->prepared_raw_context() && f->phase == Frame::Phase::fresh &&
        f->boundary == Frame::Boundary::none && f->escaped_boundary == Frame::Boundary::none &&
        !f->destination && !f->selected_name && !f->captured_configuration && !f->captured_font &&
        !f->captured_renderer && !f->result && !f->renderer_profile && !f->renderer_target &&
        !f->active_call_site && !f->escaped_call_site && f->native_eh_state == -1 &&
        !f->cached && !f->override_selected && !f->height_matches && !f->font_scale_matches &&
        !f->empty_constructed && !f->empty_compared && !f->empty_returned && !f->default_constructed &&
        !f->renderer_entered && !f->renderer_returned && !f->shader_published &&
        !f->cleanup_entered && !f->cleanup_returned && !f->exception_cleanup_started && !f->attempted &&
        !f->captured_name_data && !f->captured_name_bytes &&
        f->cache.phase == NativeMaterialEffectCacheAcquired::Phase::not_started &&
        !f->cache.loaded && !f->cache.result && !f->cache.cache_published && !f->cache.caller_acquired &&
        !f->cache.key_resolution && f->cache.loader.phase == NativeMaterialEffectLoadPhase::not_started &&
        !f->cache.loader.raw_slot && !f->cache.loader.creator && !f->cache.loader.companion &&
        !f->cache.loader.owner_record && !f->cache.loader.constructor_complete &&
        !f->cache.loader.registered && !f->cache.loader.returned &&
        !f->cache.loader.name_resolution.acquired && !f->cache.loader.programs &&
        !f->cache.loader.fallback && !f->cache.loader.constructor_texture,
        "Raw Text initial DEFAULT bind reserves only a genuinely fresh shader producer Frame");
    // names[] intentionally has no initialization; inspect no native header.
}
template<class Check>
void live_prefix_mesh_borrowed(const NativeGuiTextPrimaryPrefixLifetimeContext& g,
    const NativeModelStorageDispatchContext& model, const GuiNativeMeshAcquired& mesh,
    const void* identity, Check check) {
    // These independent Model records are tested BEFORE canonical lookup or a
    // companion cast. No branch follows the self-erased MeshEntry after release.
    if (!identity || !model.reference || model.reference->retired() || model.lifetime.started ||
        model.lifetime.pool_returned || !model.construction.complete ||
        mesh.raw_slot || mesh.creator != identity || !mesh.constructor_complete || !mesh.registered)
        return;
    auto* const canonical = g.text.factory_context.publication.owners.find(const_cast<void*>(identity));
    require(canonical && canonical == mesh.companion,
        "live prefix Mesh extent requires its SAME canonical producer companion");
    // The genuine GuiNativeGeometryOwners producer constructs this exact C++
    // family. This live downcast adjusts a real base pointer; it is never a
    // native payload projection or an ended-Mesh rediscovery.
    auto* const reference = static_cast<NativeMeshReference*>(canonical);
    check(Region{identity, sizeof(NativeMeshStorage)});
    check(Region{reference, sizeof(*reference)});
}
template<class Check>
void prefix_geometry_borrowed(const NativeGuiTextPrimaryPrefixLifetimeContext& g, Check check) {
    const auto* const c = g.operation.geometry();
    if (g.operation.kind() == NativeGuiTextPrimaryPrefixOperation::Kind::none) return;
    require(c, "chosen prefix geometry graph must be installed before raw122");
    require_native_gui_text_prefix_geometry_reservation(g);
    const auto& construction = c->construction;
    for (const Region r : {Region{c, sizeof(*c)}, {&construction, sizeof(construction)},
        {&c->frame, sizeof(c->frame)}, {&c->acquired, sizeof(c->acquired)}, {&c->progress, sizeof(c->progress)},
        {c->arguments, sizeof(c->arguments)}, {c->position, sizeof(c->position)},
        {&c->transform_scratch, sizeof(c->transform_scratch)}, {&c->bounds_scratch, sizeof(c->bounds_scratch)},
        {&construction.association, sizeof(construction.association)},
        {&construction.geometry, sizeof(construction.geometry)}, {&construction.transforms, sizeof(construction.transforms)},
        {&construction.association.unchanged_00d7a260, sizeof(Word)},
        {&construction.one_00d7a24c, sizeof(float)}, {&construction.negative_zero_00d7a208, sizeof(float)},
        {&construction.y_scale_00e12fc4, sizeof(float)}, {&construction.half_00d7a280, sizeof(double)}})
        check(r);
    const auto& a = c->acquired;
    if (c->progress.entered && c->progress.returned && !c->progress.failed &&
        c->frame.matches(construction, a, NativeGuiTextMainGeometryStoragePhase::complete))
        live_prefix_mesh_borrowed(g, construction.primary, a.mesh, a.captured_mesh, check);
    const auto& shadow = g.text.factory_context.publication.construction.sections;
    if (shadow.complete && shadow.mesh_release.complete && shadow.mesh_release.decremented &&
        shadow.mesh_release.result > 0 && !shadow.mesh_release.zero_entered)
        live_prefix_mesh_borrowed(g, construction.internal188, shadow.mesh, shadow.mesh_release.identity, check);
    // BOTH Model records/providers/flags/call arrays are enumerated by the
    // enclosing prefix route. Private MeshEntry/Impl/derived/transitive backing
    // remains the genuine caller extent contract, never a guessed sizeof.
}
template<class Check>
void prefix_route_borrowed(NativeGuiWidgetRawModelRoute& q,
    NativeModelStorageNodeDispatchContext& nc, Check check) {
    auto& m = q.record;
    auto& s = m.model.storage;
    auto& n = s.node;
    for (const Region r : {Region{&q, sizeof(q)}, {&m, sizeof(m)}, {&m.model, sizeof(m.model)},
        {&m.actual, sizeof(m.actual)}, {&m.reference, sizeof(m.reference)},
        {&m.construction, sizeof(m.construction)}, {&m.lifetime_frame, sizeof(m.lifetime_frame)},
        {&m.lifetime, sizeof(m.lifetime)}, {&m.release_frame, sizeof(m.release_frame)}, {&m.release, sizeof(m.release)},
        {&q.tree, sizeof(q.tree)}, {&q.tree_acquired, sizeof(q.tree_acquired)},
        {&q.node, sizeof(q.node)}, {&q.node_acquired, sizeof(q.node_acquired)}, {&nc, sizeof(nc)},
        {&nc.parenting, sizeof(nc.parenting)}, {&nc.shared_node, sizeof(nc.shared_node)},
        {&nc.base_nodes, sizeof(nc.base_nodes)}, {&s, sizeof(s)}, {&s.constants, sizeof(s.constants)},
        {&s.pool_01090054, sizeof(s.pool_01090054)}, {&s.pool_01090054.storage(), sizeof(s.pool_01090054.storage())},
        {&n, sizeof(n)}, {&n.strings, sizeof(n.strings)}, {&n.world, sizeof(n.world)},
        {&n.scenes, sizeof(n.scenes)}, {&n.scenes.gates, sizeof(n.scenes.gates)},
        {&n.scenes.increment_00ce221c, sizeof(n.scenes.increment_00ce221c)},
        {&n.scenes.decrement_00ce2220, sizeof(n.scenes.decrement_00ce2220)},
        {&n.trees, sizeof(n.trees)}, {&n.trees.frames, sizeof(n.trees.frames)},
        {&n.trees.point_lights, sizeof(n.trees.point_lights)}, {m.model.actual_profile_00d62de8, 0x58}})
        check(r);
    if (m.reference) check(Region{&*m.reference, sizeof(*m.reference)}); // C++ companion, including retired metadata
    // Never rediscover ended native payloads. These addresses are included only
    // while independent actual lifetime records and retained metadata are live.
    if (m.actual && m.reference && !m.reference->retired() && !m.lifetime.started && !m.lifetime.pool_returned)
        check(Region{m.actual, 0x188});
}
template<class Check>
void known_prefix_borrowed(const NativeGuiTextRawModelReferenceContext& c, Check check) {
    const auto* const g = c.retirement.primary_prefix;
    if (!g) return;
    require(representable({g, sizeof(*g)}) && &g->text.factory_context.publication == &c,
        "raw122 prefix graph must belong to its EXACT immutable factory publication");
    (void)native_gui_text_primary_prefix_selected(*g); // rejects pending/failed work; no native payload/header read
    const auto& f = g->text.factory_context;
    for (const Region r : {Region{g, sizeof(*g)}, {&g->text, sizeof(g->text)},
        {&g->authentication, sizeof(g->authentication)}, {&g->authentication_acquired, sizeof(g->authentication_acquired)},
        {&f, sizeof(f)}, {&g->text.factory, sizeof(g->text.factory)},
        {&g->text.publication, sizeof(g->text.publication)},
        {&g->producer_context, sizeof(g->producer_context)}, {&g->producer_frame, sizeof(g->producer_frame)},
        {&g->producer, sizeof(g->producer)}, {&g->producer_context.model_pools, sizeof(g->producer_context.model_pools)},
        {&g->name, sizeof(g->name)}, {&g->name.strings, sizeof(g->name.strings)}, {g->name.header8, 8},
        {&g->widget, sizeof(g->widget)}, {&g->widget.slots, sizeof(g->widget.slots)},
        {&g->widget_acquired, sizeof(g->widget_acquired)}})
        check(r);
    prefix_route_borrowed(g->widget.primary, g->producer_context.primary_node_context, check);
    prefix_route_borrowed(g->widget.internal188, g->internal_node_context, check);
    for (const auto calls : {g->primary_calls, g->internal_calls}) {
        const auto bytes = static_cast<std::uint64_t>(calls.count) * sizeof(NativeModelStorageNodeCall);
        require(calls.data && bytes && bytes <= 0xffffffffu &&
            static_cast<std::uint64_t>(bits(calls.data)) + bytes <= 0x100000000ull,
            "prefix flags need actual retained prepared-call array extents");
        check(Region{calls.data, static_cast<std::size_t>(bytes)});
    }
    if (f.disposal.context) check(Region{f.disposal.context, f.disposal.context_bytes});
    prefix_geometry_borrowed(*g, check);
    // Header ADDRESS/extent only, even after successful41DD20. Never inspect
    // its returned data words. Hidden provider arrays/Impl extents are caller proof.
}
template<class Check>
void known_borrowed(NativeGuiTextRawModelReferenceContext& c,
    const volatile Word* profile, NativeGuiTextRawModelReferenceDisposal disposal, Check check) {
    auto& p = c.construction_context;
    auto& f = c.construction_frame.sections;
    auto& a = c.construction.sections;
    auto& t = c.retirement;
    auto& r = t.resources;
    auto& model = t.model;
    auto& storage = r.model.storage;
    auto& node = storage.node;
    for (const Region region : {Region{&c, sizeof(c)}, {&c.owners, sizeof(c.owners)},
        {&c.data, sizeof(c.data)}, {profile, 8}, {&p, sizeof(p)},
        {&c.construction_frame, sizeof(c.construction_frame)}, {&c.construction, sizeof(c.construction)},
        {&f, sizeof(f)}, {&f.locals[0], sizeof(f.locals)}, {&f.arguments[0], sizeof(f.arguments)},
        {&f.association, sizeof(f.association)}, {&f.parenting, sizeof(f.parenting)},
        {&model, sizeof(model)}, {model.actual, 0x188},
        {model.reference ? &*model.reference : nullptr, sizeof(NativeModelStorageReference)},
        {&model.construction, sizeof(model.construction)}, {&model.lifetime_frame, sizeof(model.lifetime_frame)},
        {&model.lifetime, sizeof(model.lifetime)}, {&model.association, sizeof(model.association)},
        {&model.release_frame, sizeof(model.release_frame)}, {&model.release, sizeof(model.release)},
        {&p.state, sizeof(p.state)}, {&p.state.shadow_color_00e12fd8()[0], 4 * sizeof(float)},
        {&p.shadow_offset_00d5c5c0, 4}, {&p.normal_rgb_00ce3e18, 4},
        {&p.one_00d7a24c, 4}, {&p.disabled_alpha_00ce3800, 4},
        {&t, sizeof(t)}, {&r, sizeof(r)}, {&t.base, sizeof(t.base)}, {&t.glyphs, sizeof(t.glyphs)},
        {&r.model, sizeof(r.model)}, {r.model.actual_profile_00d62de8, 0x58},
        {&storage, sizeof(storage)}, {&storage.pool_01090054, sizeof(storage.pool_01090054)},
        {&storage.pool_01090054.storage(), sizeof(storage.pool_01090054.storage())},
        {&storage.constants, sizeof(storage.constants)}, {&storage.constants.positive_bound_00ce4970, 4},
        {&storage.constants.negative_bound_00ce4adc, 4}, {&storage.constants.one_00d7a24c, 4},
        {&storage.negative_zero_00d7a208, 4}, {&r.association, sizeof(r.association)},
        {&r.association.unchanged_00d7a260, 4},
        {&r.geometry, sizeof(r.geometry)}, {&r.materials, sizeof(r.materials)},
        {&r.materials.material_slots, sizeof(r.materials.material_slots)},
        {&r.materials.parameter_slots, sizeof(r.materials.parameter_slots)},
        {r.material_vtable_00d5e520, 8}, {&r.material_strings, sizeof(r.material_strings)},
        {&r.strings, sizeof(r.strings)}, {&r.string_process, sizeof(r.string_process)},
        {&r.effects, sizeof(r.effects)}, {&r.effects.effects, sizeof(r.effects.effects)},
        {&r.effects.effects.owners, sizeof(r.effects.effects.owners)},
        {&r.effects.effects.construction, sizeof(r.effects.effects.construction)},
        {&r.effects.effects.construction.current_renderer_00f8d394, sizeof(void*)},
        {r.effects.effects.renderer_profile_00d5f0a8, 0x4c},
        {r.effects.effect_base_vtable_00d5e534, 8}, {r.effects.effect_derived_vtable_00d61a00, 8},
        {&r.parenting, sizeof(r.parenting)}, {&r.parenting.dispatch, sizeof(r.parenting.dispatch)},
        {&r.parenting.group_type_0109032c, 4}, {&node, sizeof(node)},
        {&node.world, sizeof(node.world)}, {&node.strings, sizeof(node.strings)},
        {&node.scenes, sizeof(node.scenes)}, {&node.scenes.gates, sizeof(node.scenes.gates)},
        {&node.scenes.children, sizeof(node.scenes.children)},
        {&node.scenes.increment_00ce221c, sizeof(node.scenes.increment_00ce221c)},
        {&node.trees, sizeof(node.trees)}, {&node.trees.dispatch, sizeof(node.trees.dispatch)},
        {&node.trees.frames, sizeof(node.trees.frames)}, {&node.trees.point_lights, sizeof(node.trees.point_lights)},
        {&node.trees.decrement_00ce2220, sizeof(node.trees.decrement_00ce2220)},
        {&t.base.nodes, sizeof(t.base.nodes)}, {&t.base.bindings, sizeof(t.base.bindings)},
        {t.base.actual_base_table_00d5c130, 9 * 4},
        {&t.base.actual_base_lineage_00f8bc88[0], sizeof(t.base.actual_base_lineage_00f8bc88)},
        {&t.base.actual_text_descriptor_00f8be28, 4},
        {&t.actual_pool_00f8bdf0, sizeof(t.actual_pool_00f8bdf0)},
        {&t.actual_pool_00f8bdf0.storage(), sizeof(t.actual_pool_00f8bdf0.storage())},
        {a.mesh.creator, sizeof(NativeMeshStorage)}, {a.mesh.companion, sizeof(RenderCommandReference)},
        {a.shadow_section.section.creator, sizeof(NativeMeshSectionStorage)},
        {a.shadow_section.section.companion, sizeof(RenderCommandReference)},
        {a.shadow_section.material.material, sizeof(NativeMaterialStorage)},
        {a.shadow_section.material.companion, sizeof(RenderCommandReference)},
        {a.shadow_section.material.cache.get(), sizeof(NativeMaterialEffectCacheAcquired)},
        {disposal.context, disposal.context_bytes}}) {
        if (region.begin) {
            require(representable(region), "Raw Text reference borrowed region requires a valid extent");
            check(region);
        }
    }
    known_shader_borrowed(c, [&](Region region) {
        require(representable(region), "Raw Text shader borrowed record needs a valid retained extent");
        check(region);
    });
    known_prefix_borrowed(c, [&](Region region) {
        require(representable(region), "raw122 prefix metadata requires valid retained backing extents");
        check(region);
    });
    // Abstract/opaque provider implementations, actual pool/slab allocations,
    // effect loader frames and retained creator records have transitive caller
    // extent contracts. A base sizeof above never proves a derived extent.
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
Admission preflight(void* companion, void* raw, NativeGuiTextRawModelReferenceContext& c,
    NativeGuiTextReferenceFrame& frame, NativeGuiTextRawModelReferenceDisposal disposal) {
    require(companion && bits(companion) % alignof(NativeGuiTextRawModelReference) == 0 &&
        raw && bits(raw) % 4 == 0 && disposal.retire &&
        ((disposal.context != nullptr) == (disposal.context_bytes != 0)),
        "Raw Text binding requires aligned separate companion/Text and explicit sized disposal");
    const auto* profile = static_cast<const volatile Word*>(c.data.data_at(text_profile, 8));
    const Region writes[] = {{raw, 0x1f8}, {companion, sizeof(NativeGuiTextRawModelReference)}, {&frame, sizeof(frame)}};
    for (std::size_t i = 0; i != 3; ++i) {
        require(representable(writes[i]), "Raw Text binding caller regions require valid extents");
        for (std::size_t j = i + 1; j != 3; ++j)
            require(disjoint(writes[i], writes[j]), "Raw Text/companion/scalar frame must be separate");
    }
    known_borrowed(c, profile, disposal, [&](Region borrowed) {
        for (const auto region : writes)
            require(disjoint(region, borrowed), "Raw Text binding writes cannot alias the retained producer graph");
    });
    fresh_shader_binding(c);
    const auto& a = c.construction;
    auto& p = c.construction_context;
    auto& f = c.construction_frame.sections;
    auto& t = c.retirement;
    auto& r = t.resources;
    auto& m = t.model;
    auto& node = r.model.storage.node;
    require(a.started && a.complete && a.base_complete && a.derived_complete && a.sections_entered &&
        !a.exception_cleanup_started && !a.base_lifetime_ended && !a.consumed_cleanup_actions &&
        a.failure == decltype(a.failure)::none && a.native_eh_state == 6 && a.destination == raw &&
        a.sections.started && a.sections.complete && !a.sections.exception_cleanup_started &&
        a.sections.failure == decltype(a.failure)::none && &a.sections.model == &m &&
        a.sections.model_registered && a.sections.model_constructed && a.sections.shadow_published &&
        m.actual && m.reference && !m.reference->retired() && m.construction.complete &&
        m.construction.metadata_bound && m.construction.actual_model == m.actual &&
        m.construction.context == &r.model.storage && !m.lifetime.started && !m.lifetime.scalar_started &&
        !m.release.started && m.association.phase == NativeModelStorageGeometryPhase::complete &&
        m.association.actual_model == m.actual && m.association.context == &r.association &&
        m.association.frame == &f.association && f.association.phase() == NativeModelStorageGeometryPhase::complete &&
        fresh(frame.scalar), "Raw Text binding requires the SAME genuine completed93/raw Model/B75170 records");
    require(&p.sections == &r && &p.lifetime == &t.base && &r.association.storage == &r.model.storage &&
        &r.parenting.node == &node && &c.owners == &r.geometry.actual_owners() &&
        &r.materials.retained_owners == &c.owners && &node.scenes.owners == &c.owners &&
        &node.trees.owners == &c.owners && &node.trees.point_lights == &t.base.nodes.point_lights &&
        &t.glyphs.nodes == &t.base.nodes && &t.glyphs.children == &t.base.bindings &&
        t.base.nodes.uses_raw_name_pool() && &t.base.nodes.require_raw_name_pool() == &r.strings &&
        &node.strings == &r.strings && same_native_string_raw_pool_cells(r.strings, r.string_process.raw_context()) &&
        same_native_string_storage_raw_pool_cells(r.material_strings, r.strings) &&
        &node.scenes.decrement_00ce2220 == &node.trees.decrement_00ce2220 && node.trees.decrement_00ce2220 &&
        &r.effects.effects.owners.actual_owners() == &c.owners &&
        &r.effects.effects.owners.string_storage() == &r.material_strings &&
        &r.effects.strings == &r.material_strings && &r.materials.parameter_names == &r.material_strings &&
        &r.effects.effects.construction.strings == &r.material_strings && r.effects.effects.cache == &r.effects &&
        t.base.actual_base_table_00d5c130 && &p.state == &game::game_native_gui_text_state() &&
        &t.actual_pool_00f8bdf0 == &game::game_native_gui_text_process().pool_00f8bdf0() &&
        &t.base.actual_base_lineage_00f8bc88 == &game::game_native_gui_text_process().element_lineage_00f8bc88() &&
        &t.base.actual_text_descriptor_00f8be28 == &game::game_native_gui_text_process().text_lineage_00f8be28()[0],
        "Raw Text binding must preserve the actual93/118 Model/pool/import/type/point-light/provider identities");
    auto raw_word = [&](Word offset) { return *reinterpret_cast<const volatile Word*>(bits(raw) + offset); };
    void* const head = reinterpret_cast<void*>(raw_word(0x68));
    require(raw_word(0x188) == bits(m.actual) && !raw_word(0x4c) && !raw_word(0x70) &&
        !raw_word(0x6c) && head && read(head) == bits(head) &&
        *reinterpret_cast<const volatile Word*>(bits(head) + 4) == bits(head) &&
        !raw_word(0x88) && !raw_word(0x8c) && !raw_word(0x90) &&
        !raw_word(0x180) && !raw_word(0x184) && !raw_word(0x18c) && !raw_word(0x190) &&
        !raw_word(0x198) && !raw_word(0x19c) && !raw_word(0x1a0) && !raw_word(0x1ec) &&
        !raw_word(0xec) && !raw_word(0xf0) && !raw_word(0xf4) && !raw_word(0xf8) &&
        !raw_word(0x1a4) && !raw_word(0x1a8) && !raw_word(0x1c0) && !raw_word(0x1c4) &&
        !raw_word(0x1c8) && !raw_word(0x1cc) &&
        !native_gui_widget_is_kind_of_00a9e070(native_gui_text_descriptor_00ab6a30(
            t.base.actual_text_descriptor_00f8be28), t.base.actual_base_lineage_00f8bc88),
        "Raw Text binding admits DEFAULT payload and CURRENT ordinary lineage comparisons only");
    require(read(raw) == text_profile && bits(profile) == text_profile &&
        profile[0] == zero_target && profile[1] == scalar_target &&
        &c.owners.resolve_actual(m.actual) == &*m.reference,
        "Raw Text binding requires the genuine current profile aliases and SAME canonical internal Model");
    auto& count = *std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(bits(raw) + 4));
    require(count.load(std::memory_order_relaxed) > 0 && !c.owners.find(raw),
        "Raw Text binding needs live positive actual+04 and an unregistered actual identity");
    return {profile, count};
}
template<class Reference, class Context, class Disposal>
void flags_preflight(const volatile Word& flags, Reference* companion,
    void* raw, Context& c, NativeGuiTextReferenceFrame& frame,
    const volatile Word* profile, Disposal disposal) {
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

NativeGuiTextRawModelReference& bind_native_gui_text_raw_model_reference(void* companion, void* raw,
    NativeGuiTextRawModelReferenceContext& context, NativeGuiTextReferenceFrame& frame,
    NativeGuiTextRawModelReferenceDisposal disposal) {
    const auto admission = preflight(companion, raw, context, frame, disposal);
    return *::new (companion) NativeGuiTextRawModelReference(raw, context, frame,
        disposal, admission.profile, admission.count);
}
NativeGuiTextRawModelReference::NativeGuiTextRawModelReference(void* raw,
    NativeGuiTextRawModelReferenceContext& context, NativeGuiTextReferenceFrame& frame,
    NativeGuiTextRawModelReferenceDisposal disposal, const volatile Word* profile,
    std::atomic<std::int32_t>& count)
    : RenderCommandReference(count), identity_(raw), context_(context), frame_(frame),
      disposal_(disposal), profile_(profile) {
    context_.owners.bind(identity_, *this); // transactional metadata only, no retain
}
NativeGuiTextRawModelReference::~NativeGuiTextRawModelReference() {
    if (phase_ != Phase::retired) std::terminate();
}
void NativeGuiTextRawModelReference::require_slot(Word profile, unsigned slot, Word target) const {
    if (profile != text_profile || profile_[slot] != target)
        throw std::logic_error("Raw Text lacks its genuine CURRENT scalar/zero selector");
}
NativeGuiTextRawModelReference::Completion NativeGuiTextRawModelReference::completion() const noexcept {
    return {identity_, &context_.owners, disposal_};
}
void NativeGuiTextRawModelReference::finish_success(Completion saved) noexcept {
    phase_ = Phase::retired;
    saved.owners->unbind(saved.identity, *this);
    saved.disposal.retire(saved.disposal.context, *this); // LAST, independent Model graph stays retained
}
void NativeGuiTextRawModelReference::delete_vslot04(void* raw, Word captured_profile, Word flags) {
    if (phase_ != Phase::zero_dispatch)
        throw std::logic_error("Raw Text zero scalar adapter rejects reentry before raw access");
    if (raw != identity_ || flags != 1)
        throw std::logic_error("Raw Text BD30E0 needs SAME actual identity/literal1");
    require_shader_quiescent(context_);
    require_slot(captured_profile, 1, scalar_target);
    auto& storage = context_.retirement;
    auto& acquired = frame_.scalar;
    auto& actual_flags = frame_.zero_flags;
    phase_ = Phase::retiring;
    actual_flags = 1;
    delete_native_gui_text_storage_00ab8ee0(raw, actual_flags, storage, acquired);
}
void NativeGuiTextRawModelReference::release_zero_references() noexcept {
    if (phase_ != Phase::bound) std::terminate();
    const auto saved = completion();
    phase_ = Phase::zero_dispatch;
    try {
        require_shader_quiescent(context_);
        if (context_.owners.find(identity_) != this || reference_count.load(std::memory_order_relaxed) != 0)
            throw std::logic_error("Raw Text zero terminal requires SAME canonical observed-zero binding");
        require_slot(read(identity_), 0, zero_target);
        invoke_native_ref_counted_delete_00bd30e0(identity_, *this);
        finish_success(saved); // no payload/profile/count reads after scalar/pool return
    } catch (...) {
        phase_ = Phase::failed;
        std::terminate();
    }
}
void* NativeGuiTextRawModelReference::delete_current_scalar_00ab8ee0(const volatile Word& flags) {
    if (phase_ != Phase::bound)
        throw std::logic_error("Raw Text direct scalar rejects reentry before raw access");
    const auto saved = completion();
    phase_ = Phase::retiring;
    try {
        flags_preflight(flags, this, identity_, context_, frame_, profile_, disposal_);
        if (context_.owners.find(identity_) != this)
            throw std::logic_error("Raw Text direct scalar needs SAME canonical binding");
        require_slot(read(identity_), 1, scalar_target);
        void* const result = delete_native_gui_text_storage_00ab8ee0(
            identity_, flags, context_.retirement, frame_.scalar); // no count gate/decrement
        finish_success(saved);
        return result; // saved local, no companion/raw access after disposal
    } catch (...) {
        phase_ = Phase::failed;
        throw; // retain entire graph, actual registry binding and incomplete invocation
    }
}
} // namespace bsp
