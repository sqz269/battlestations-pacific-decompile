#include "bsp/game_native_raw_default_text_owner.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/native_vfs_owner_services.hpp"
#include "bsp/game_native_type_storage.hpp"
#include "bsp/game_native_resource_pools.hpp"
#include "bsp/game_native_material_process.hpp"
#include "bsp/game_native_string_process.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_model_storage_node_dispatch.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include <cstdint>
#include <exception>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw DEFAULT Text owner requires MSVC Win32.
#endif

namespace bsp::game {
namespace {
using Word = std::uint32_t;
static_assert(sizeof(void*) == sizeof(Word));
void require(bool value, const char* message) {
    if (!value) throw std::logic_error(message);
}
const volatile Word& cell(GameNativeReadOnlyData& data, Word address) {
    return *static_cast<const volatile Word*>(data.data_at(address, sizeof(Word)));
}
const volatile Word* table(GameNativeReadOnlyData& data, Word address, std::size_t bytes) {
    return static_cast<const volatile Word*>(data.data_at(address, bytes));
}
} // namespace

struct GameNativeRawDefaultTextOwner::Impl {
    enum class Phase { fresh, preparing, prepared, creating, empty, bound, retiring, retired, closing, closed, failed };
    struct Services;
    struct Attempt;
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    GameNativeGuiSamplerApplicationBindings sampler_bindings;
    Phase phase{Phase::fresh};
    std::exception_ptr failure;
    std::unique_ptr<Services> services;
    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& v,
        GameSingletonHost& s, GameNativeReadOnlyData& d, GameNativeGuiSamplerApplicationBindings b)
        : log(l), renderer(r), vfs(v), singletons(s), data(d), sampler_bindings(b) {}
    ~Impl();
    void fail() noexcept { failure = std::current_exception(); phase = Phase::failed; }
    bool custody_settled() const noexcept;
    bool retain() const noexcept;
    void observe(const char*) noexcept;
};

struct GameNativeRawDefaultTextOwner::Impl::Services {
    Impl& owner;
    GameNativeVfsRawServices raw;
    GameNativeRendererSceneDomain domain;
    NativeNodeDestructionRuntime& base_nodes;
    GuiNativeGeometryOwners& geometry;
    const volatile Word* material_profile;
    NativeMaterialDestructionAccess materials;
    NativeLuaServiceBindings& lua;
    NativeShaderBinaryCacheContext& shader_cache;
    GameNativeGuiTextProcess& process;
    GameNativeStringProcess& string_process;
    GameNativeGuiTextState& text_state;
    std::unique_ptr<GameNativeGuiSamplerApplication> samplers;
    std::unique_ptr<GameNativeGuiEffectApplication> effects;
    NativeMaterialEffectCacheContext* effect_cache{};
    std::unique_ptr<Attempt> attempt; // destroys BEFORE the facade/provider borrows
    explicit Services(Impl& p)
        : owner(p), raw(p.vfs.borrow_raw_services()), domain(p.renderer.startup_scene_domain()),
          base_nodes(p.renderer.camera_environment().nodes),
          geometry(p.renderer.game_grid_context().graphics.streams.geometry),
          material_profile(p.renderer.borrow_gui_material_destruction_profile_00d5e520()),
          materials{p.renderer.actual_owners(), raw.strings,
              game_native_material_process().parameter_pool_00f8d3e4(),
              game_native_material_process().material_pool_00f8d3ac()},
          lua(p.renderer.lua_service_binding()), shader_cache(p.renderer.shader_cache_context()),
          process(game_native_gui_text_process()), string_process(game_native_string_process()),
          text_state(game_native_gui_text_state()) {
        require(p.sampler_bindings.actual_platform_0109cf04.identity() ==
            p.renderer.borrow_platform_publication_0109cf04().identity(),
            "raw DEFAULT Text requires the renderer's SAME real platform/input binding");
        auto& names = p.vfs.raw_strings();
        auto& process_names = string_process.raw_context();
        require(&names.actual_published_01090aa8 == &process_names.actual_published_01090aa8 &&
            &names.actual_small_returns_disabled_01090aa4 == &process_names.actual_small_returns_disabled_01090aa4 &&
            &names.actual_manager_publication_01090aa0 == &process_names.actual_manager_publication_01090aa0 &&
            &raw.strings == &p.vfs.native_owners().strings() &&
            p.vfs.native_owners().strings().uses_raw_pool_cells(names) &&
            &base_nodes.require_raw_name_pool() == &p.vfs.raw_strings() &&
            &domain.node.strings == &p.vfs.raw_strings() &&
            &domain.node.scenes.owners == &p.renderer.actual_owners() &&
            &geometry.actual_owners() == static_cast<NativeRenderActualOwners*>(&p.renderer.actual_owners()),
            "raw DEFAULT Text services must preserve actual renderer/VFS/string/owner identities");
        (void)process.pool_00f8bdf0();
        (void)process.element_lineage_00f8bc88();
        (void)process.text_lineage_00f8be28(); // ready-only aliases, no type initializer/predicate
    }
    ~Services();
    void prepare(std::size_t);
};

struct GameNativeRawDefaultTextOwner::Impl::Attempt {
    struct Disposal { Impl* owner; Attempt* attempt; };
    Services& services;
    NativeGuiTextRawModelDefaultStorageAcquired construction;
    NativeGuiTextDefaultFactoryStorageAcquired factory;
    NativeGuiTextReferenceFrame reference_frame{};
    NativeGuiTextDefaultWidgetLifetimeAcquired widget_acquired;
    NativeGuiTextDefaultWidgetPublicationState publication;
    std::unique_ptr<NativeGuiTextDefaultWidgetLifetime> widgets;
    std::unique_ptr<NativeModelStorageNodeCall[]> node_calls;
    const std::size_t node_call_capacity;
    NativeModelStorageNodeDispatchAcquired node_acquired;
    NativeModelStorageDispatchAcquired tree_acquired;
    NativeModelStorageDispatch tree_dispatch;
    NativeModelStorageNodeDispatch node_dispatch;
    NativeSceneRegistrationGateContext gates;
    NativeNodeSceneAttachmentContext scenes;
    NativeNodeTreeRetirementContext trees;
    NativeNodeBaseDestructionContext node;
    NativeModelStorageContext model_storage;
    NativeModelStorageIdentityContext model_identity;
    NativeModelStorageGeometryContext association;
    NativeNodeParentingContext parenting;
    NativeModelStorageDispatchContext tree_record;
    NativeModelStorageNodeDispatchContext node_record;
    NativeGuiTextGlyphChildrenContext glyphs;
    volatile Word locals[4]{}, arguments[4]{};
    NativeNodeParentingFrame parenting_frame{};
    NativeGuiTextRawModelSectionStorageContext sections;
    NativeGuiTextRawModelSectionStorageFrame section_frame;
    NativeGuiTextRawModelDefaultStorageContext default_context;
    NativeGuiTextRawModelDefaultStorageFrame default_frame;
    NativeGuiTextRawModelRetirementContext retirement;
    NativeGuiTextRawModelReferenceContext reference_context;
    alignas(NativeGuiTextRawModelReference) std::byte companion[sizeof(NativeGuiTextRawModelReference)];
    Disposal disposal;
    NativeGuiTextDefaultFactoryStorageContext factory_context;
    NativeGuiTextDefaultWidgetLifetimeRecord widget_record;
    volatile Word scalar_flags{}; // separate from companion, scalar frame and disposal extent

    Attempt(Services& s, std::size_t capacity)
        : services(s), widgets(std::make_unique<NativeGuiTextDefaultWidgetLifetime>(s.base_nodes,
              s.owner.renderer.actual_owners(), s.owner.data, s.process, widget_acquired)),
          node_calls(std::make_unique<NativeModelStorageNodeCall[]>(capacity)), node_call_capacity(capacity),
          tree_dispatch(tree_acquired),
          node_dispatch(s.owner.data, s.owner.vfs.native_types(), s.owner.vfs.native_owners().types(),
              s.domain.node.scenes.gates.light_types, node_calls.get(), capacity, node_acquired),
          gates{s.domain.node.scenes.gates.registry, s.domain.node.scenes.gates.light_types,
              s.domain.node.scenes.gates.camera_types, s.domain.node.scenes.gates.actual_node_profile_00d62c88,
              s.domain.node.scenes.gates.actual_camera_profile_00d62cf0,
              s.domain.node.scenes.gates.actual_light_profile_00d62f58,
              s.domain.node.scenes.gates.actual_directional_profile_00d62fb0, &node_dispatch},
          scenes{gates, s.owner.renderer.actual_owners(), s.domain.node.scenes.increment_00ce221c,
              s.domain.node.scenes.decrement_00ce2220, node_dispatch},
          trees{s.base_nodes.point_lights, s.owner.renderer.actual_owners(), s.domain.node.trees.decrement_00ce2220,
              tree_dispatch, s.domain.node.trees.frames},
          node{scenes, trees, s.owner.vfs.raw_strings(), s.domain.node.world},
          model_storage{node, game_native_resource_pool_process().model_pool_01090054(),
              s.owner.renderer.node_constants(), cell(s.owner.data, 0x00d7a208)},
          model_identity{model_storage, table(s.owner.data, 0x00d62de8, 0x58)},
          association{model_storage, cell(s.owner.data, 0x00d7a260)},
          parenting{node, node_dispatch, s.owner.vfs.native_types().group_types().group_0109032c.own_id},
          tree_record{model_identity, construction.sections.model.actual, construction.sections.model.reference,
              construction.sections.model.construction, construction.sections.model.lifetime_frame,
              construction.sections.model.lifetime, construction.sections.model.release_frame, construction.sections.model.release},
          node_record{tree_record, tree_dispatch, parenting, s.domain.node, s.base_nodes},
          glyphs{s.base_nodes, *widgets},
          sections{model_identity, association, s.geometry, s.materials, s.raw.strings,
              s.owner.vfs.raw_strings(), s.string_process, *s.effect_cache, parenting, s.material_profile,
              static_cast<const char*>(s.owner.data.data_at(0x00d15ec4, 7)),
              static_cast<const char*>(s.owner.data.data_at(0x00d5c25c, 11))},
          section_frame{locals, arguments, parenting_frame},
          default_context{sections, widgets->context(), s.text_state, cell(s.owner.data, 0x00d5c5c0),
              cell(s.owner.data, 0x00ce3e18), cell(s.owner.data, 0x00d7a24c), cell(s.owner.data, 0x00ce3800)},
          default_frame{section_frame},
          retirement{sections, construction.sections.model, widgets->context(), glyphs, s.process.pool_00f8bdf0()},
          reference_context{s.owner.renderer.actual_owners(), s.owner.data, default_context, default_frame, construction, retirement},
          disposal{&s.owner, this},
          factory_context{s.process, reference_context, construction, reference_frame, companion, sizeof(companion),
              {&disposal, sizeof(disposal), &Attempt::dispose}},
          widget_record{factory_context, factory, publication} {}
    ~Attempt() {
        // Explicit metadata destruction while every borrowed context/record is
        // still alive. No native cleanup; only a settled owner may reach here.
        if (publication.bind_complete && !publication.disposed) std::terminate();
        widgets.reset();
        construction.sections.model.reference.reset();
    }
    void bind() {
        tree_dispatch.bind(tree_record);
        node_dispatch.bind(node_record);
        widgets->bind(widget_record); // entire stable graph exists; no native pool effect
    }
    static void dispose(void* context, NativeGuiTextRawModelReference& reference) noexcept {
        auto& d = *static_cast<Disposal*>(context);
        auto& a = *d.attempt;
        auto& p = a.publication;
        if (d.owner->phase != Phase::retiring || !d.owner->services ||
            d.owner->services->attempt.get() != &a || p.reference != &reference ||
            !p.bind_complete || !p.scalar_entered || p.companion_disposal_entered || p.disposed) std::terminate();
        p.companion_disposal_entered = true;
        reference.~NativeGuiTextRawModelReference();
        p.disposed = true; // reference pointer is now diagnostic ONLY
        // LAST disposal never erases Attempt, Model record, frames or providers.
    }
};

GameNativeRawDefaultTextOwner::Impl::~Impl() = default;
GameNativeRawDefaultTextOwner::Impl::Services::~Services() = default;
void GameNativeRawDefaultTextOwner::Impl::Services::prepare(std::size_t capacity) {
    samplers = std::make_unique<GameNativeGuiSamplerApplication>(owner.log, owner.renderer,
        owner.vfs, owner.singletons, owner.data, owner.sampler_bindings);
    const auto inputs = samplers->effect_bindings(); // export only AFTER parent stored above
    effects = std::make_unique<GameNativeGuiEffectApplication>(owner.log, owner.renderer, owner.vfs, inputs);
    effect_cache = &effects->native_effect_cache(); // capture once BEFORE native/cache entry
    attempt = std::make_unique<Attempt>(*this, capacity);
    attempt->bind(); // installed before all remaining pure admission
}
bool GameNativeRawDefaultTextOwner::Impl::custody_settled() const noexcept {
    if (!services || !services->attempt) return true;
    const auto& a = *services->attempt;
    if (!a.factory.started) return true;
    if (a.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::empty)
        return a.factory.complete && !a.factory.constructor_entered && !a.factory.raw_slot;
    const auto& p = a.publication;
    const auto& scalar = a.reference_frame.scalar;
    if (!p.bind_complete || p.bind_failed || !p.scalar_entered || !p.scalar_returned || p.scalar_failed ||
        !p.companion_disposal_entered || !p.disposed || !scalar.complete || !scalar.destructor_returned ||
        !scalar.destruction.complete || !scalar.flags_captured || !scalar.pool_returned ||
        !(scalar.captured_flags_byte & 1u)) return false;
    const auto& m = a.construction.sections.model;
    return m.reference && m.reference->retired() && m.lifetime.complete && m.lifetime.scalar_complete &&
        m.lifetime.flags_read && m.lifetime.pool_returned && (m.lifetime.flags_low_byte & 1u);
}
bool GameNativeRawDefaultTextOwner::Impl::retain() const noexcept {
    if (failure || phase == Phase::preparing || phase == Phase::creating || phase == Phase::retiring ||
        phase == Phase::closing || phase == Phase::failed) return true;
    if (services) {
        if ((services->samplers && services->samplers->requires_process_retention()) ||
            (services->effects && services->effects->requires_process_retention())) return true;
        if (services->attempt) {
            const auto& a = *services->attempt;
            using CallPhase = NativeModelStorageDispatchCallAcquired::Phase;
            if (a.widget_acquired.active || a.widget_acquired.poisoned || a.tree_acquired.poisoned ||
                a.node_acquired.poisoned || a.tree_acquired.current18.phase == CallPhase::entered ||
                a.tree_acquired.current0.phase == CallPhase::entered ||
                a.node_acquired.used > a.node_call_capacity) return true;
            for (std::size_t i = 0; i < a.node_acquired.used; ++i)
                if (a.node_calls[i].phase == CallPhase::entered) return true;
        }
    }
    return phase == Phase::retired && !custody_settled();
}
void GameNativeRawDefaultTextOwner::Impl::observe(const char* event) noexcept {
    try { log.notef("raw DEFAULT Text owner: %s phase=%u custody_settled=%d",
        event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
    catch (...) {} // diagnostics never change native ownership/disposition
}

GameNativeRawDefaultTextOwner::GameNativeRawDefaultTextOwner(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameNativeGuiSamplerApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, bindings)) {}
GameNativeRawDefaultTextOwner::~GameNativeRawDefaultTextOwner() noexcept {
    if (impl_->phase != Impl::Phase::fresh && impl_->phase != Impl::Phase::closed) std::terminate();
}
void GameNativeRawDefaultTextOwner::prepare_metadata_in_ready_renderer(std::size_t capacity) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::fresh && capacity &&
        static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu,
        "raw DEFAULT Text metadata requires a fresh owner and explicit prepared call budget");
    p.phase = Impl::Phase::preparing;
    try {
        p.services = std::make_unique<Impl::Services>(p);
        p.services->prepare(capacity);
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_default_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt,
        "raw DEFAULT Text construction requires its once-prepared complete graph");
    auto& s = *p.services;
    auto& a = *s.attempt;
    p.phase = Impl::Phase::creating;
    try {
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention(), "raw DEFAULT Text requires the real caller-owned open shader cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true; // SEPARATE from successful raw93/AA1380
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private");
        return true;
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::retire_before_native_drain() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired || p.phase == Impl::Phase::closed) return;
    if (p.phase == Impl::Phase::fresh || p.phase == Impl::Phase::prepared || p.phase == Impl::Phase::empty) {
        require(!p.retain(), "incomplete DEFAULT metadata cannot retire");
        p.phase = Impl::Phase::retired;
        return;
    }
    require(p.phase == Impl::Phase::bound && !p.retain(), "raw DEFAULT Text retirement rejects active/failed/replayed entry");
    auto& s = *p.services;
    auto& a = *s.attempt;
    p.phase = Impl::Phase::retiring;
    try {
        NativeLuaServiceBindings::Activation activation(s.lua);
        a.widgets->require_default_before_scalar();
        auto& reference = *a.publication.reference;
        require(reference.reference_count.load(std::memory_order_relaxed) == 1,
            "forced DEFAULT scalar requires this sole unpublished private Text creator"); // OWNER admission, no decrement
        a.publication.scalar_entered = true;
        a.scalar_flags = 1;
        void* const result = reference.delete_current_scalar_00ab8ee0(a.scalar_flags);
        a.publication.scalar_returned = true;
        require(result == a.factory.raw_slot && a.publication.disposed,
            "raw DEFAULT scalar did not return its same identity and dispose the Text companion");
        p.phase = Impl::Phase::retired;
        p.observe("text_retired"); // metadata ONLY, including surviving Model/flags0 custody
    } catch (...) { a.publication.scalar_failed = true; p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::requires_process_retention() const noexcept { return impl_->retain(); }
void GameNativeRawDefaultTextOwner::after_native_renderer_drain() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::closed) return;
    require(p.phase == Impl::Phase::retired && !p.retain() && p.custody_settled(),
        "raw DEFAULT Text must settle actual custody before parent drain acknowledgement");
    p.phase = Impl::Phase::closing;
    try {
        p.renderer.after_native_drain(); // FIRST, even when raw cache borrows made no facade operation entries
        if (p.services) {
            if (p.services->effects) p.services->effects->after_native_renderer_drain();
            if (p.services->samplers) p.services->samplers->after_native_renderer_drain();
        }
        p.phase = Impl::Phase::closed;
        p.observe("closed_after_actual_parent_drain");
    } catch (...) { p.fail(); throw; }
}
} // namespace bsp::game
