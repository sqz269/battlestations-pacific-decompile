#include "bsp/game_native_raw_default_text_owner.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
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
#include "bsp/game_native_font_application.hpp"
#include "bsp/native_gui_text_default_widget_lifetime.hpp"
#include "bsp/native_gui_text_source_storage.hpp"
#include "bsp/native_gui_text_empty_unchanged_storage.hpp"
#include "bsp/native_gui_text_shader_storage.hpp"
#include "bsp/native_gui_text_font_lifetime_storage.hpp"
#include "bsp/native_gui_text_primary_prefix_lifetime.hpp"
#include "bsp/native_gui_text_prefix_geometry_lifetime.hpp"
#include "bsp/native_gui_text_main_section_lifetime_storage.hpp"
#include "bsp/native_model_storage_node_dispatch.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/gui_text_native_renderer.hpp"
#include "bsp/native_material_owner.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_wide_string.hpp"
#include "native_gui_text_cache_backing.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_camera_owner.hpp"
#include "bsp/native_lua_service_bindings.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include <cstdint>
#include <exception>
#include <new>
#include <new.h>
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
    enum class Mode { default_lifecycle, primary_prefix, primary_prefix_geometry,
        primary_prefix_geometry_font, primary_prefix_geometry_main_section,
        primary_prefix_geometry_font_main_section, default_source_equal,
        primary_prefix_geometry_font_main_section_empty_unchanged };
    enum class FontLink { none, entered, bound, detached, failed };
    struct Services;
    struct Attempt;
    struct PrimaryPrefix;
    struct Geometry;
    struct MainSection;
    struct FontShader;
    struct FontWorkGraph;
    struct FontMainSelection;
    struct SourceEqualGraph;
    struct EmptyUnchangedGraph;
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameSingletonHost& singletons;
    GameNativeReadOnlyData& data;
    GameNativeGuiSamplerApplicationBindings sampler_bindings;
    Phase phase{Phase::fresh};
    std::exception_ptr failure;
    FontLink font_link{FontLink::none};
    const GameNativeFontApplication* font_producer{};
    NativeFontResourceOwners* font_resources{};
    const void* font_registry_image{}; // identity only after native Font retirement
    std::unique_ptr<Services> services;
    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& v,
        GameSingletonHost& s, GameNativeReadOnlyData& d, GameNativeGuiSamplerApplicationBindings b)
        : log(l), renderer(r), vfs(v), singletons(s), data(d), sampler_bindings(b) {}
    ~Impl();
    void fail() noexcept {
        failure = std::current_exception();
        phase = Phase::failed;
        // Keep the original failure observable before the host's sticky exit.
        // Logging invokes no native cleanup and cannot replace disposition.
        try {
            if (failure) std::rethrow_exception(failure);
        } catch (const std::exception& error) {
            try { log.notef("raw DEFAULT Text owner failure: %s", error.what()); } catch (...) {}
        } catch (...) {
            try { log.note("raw DEFAULT Text owner failure: nonstandard exception"); } catch (...) {}
        }
        observe("failed");
    }
    void fail_font_link(const GameNativeFontApplication& producer) noexcept {
        if (!font_producer) font_producer = &producer; // never replace a different admitted producer
        font_link = FontLink::failed;
        fail();
    }
    void require_font_work_creating() const {
        require(phase == Phase::creating && !failure && font_link == FontLink::bound &&
            font_producer && font_resources && font_registry_image &&
            font_producer->ready() && !font_producer->failure(),
            "caught raw Font owner/producer failure cannot resume its creating operation");
    }
    void require_main_section_creating() const {
        require(phase == Phase::creating && !failure,
            "caught raw main-section owner failure cannot resume its creating operation");
    }
    void require_source_equal_creating() const {
        require(phase == Phase::creating && !failure && font_link == FontLink::none &&
            !font_producer && !font_resources && !font_registry_image,
            "caught DEFAULT source-equal failure or foreign Font link cannot resume construction");
    }
    bool custody_settled() const noexcept;
    static bool attempt_settled(const Attempt&) noexcept;
    bool retain() const noexcept;
    void observe(const char*) noexcept;
};

struct GameNativeRawDefaultTextOwner::Impl::Services {
    Impl& owner;
    const Mode mode; // request fixed before effects; never native-credit authority
    const FontWork font_work;
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
    explicit Services(Impl& p, Mode selected = Mode::default_lifecycle,
        FontWork work = FontWork::shader_then_retirement)
        : owner(p), mode(selected), font_work(work), raw(p.vfs.borrow_raw_services()), domain(p.renderer.startup_scene_domain()),
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

struct GameNativeRawDefaultTextOwner::Impl::FontShader {
    Impl& owner;
    const bool work_selected;
    NativeGuiTextShaderStorageContext context;
    NativeGuiTextShaderStorageFrame frame;
    FontShader(Services& s, const NativeGuiTextReferenceFrame& reference_frame)
        : owner(s.owner), work_selected(s.mode == Mode::primary_prefix_geometry_font ||
              s.mode == Mode::primary_prefix_geometry_font_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged),
          context{*s.owner.font_resources, s.owner.sampler_bindings.actual_platform_0109cf04, nullptr},
          frame(context, reference_frame) {} // both old metadata/new work: pure immutable addresses only
    ~FontShader() noexcept { if (retain()) std::terminate(); }
    bool retain() const noexcept {
        if (!work_selected) return !untouched(); // old metadata mode stays strictly FRESH
        const auto p = frame.credit_progress();
        return p.failed || p.active || p.producer == NativeGuiTextShaderStorageFrame::RawPhase::entered ||
            p.producer == NativeGuiTextShaderStorageFrame::RawPhase::failed ||
            frame.phase == NativeGuiTextShaderStorageFrame::Phase::entered ||
            frame.phase == NativeGuiTextShaderStorageFrame::Phase::failed;
        // Healthy bound/outstanding is not failure. FontWorkGraph/owner terminal
        // proof is separately mandatory before any provider can be destroyed.
    }
    bool untouched() const noexcept {
        using Frame = NativeGuiTextShaderStorageFrame;
        const auto progress = frame.credit_progress();
        if (progress.producer != Frame::RawPhase::fresh || progress.credit != Frame::CreditPhase::none ||
            progress.consumer != Frame::Consumer::none || progress.failed || progress.active || progress.retirement_started ||
            progress.actual_text || progress.reference || progress.reference_context ||
            progress.reference_frame || progress.cache) return false;
        const auto& c = frame.cache;
        const auto& l = c.loader;
        return frame.prepared_raw_context() == &context && &context.fonts == owner.font_resources &&
            context.configuration_0109cf04.identity() == owner.sampler_bindings.actual_platform_0109cf04.identity() &&
            !context.unknown_renderer48 && frame.phase == Frame::Phase::fresh &&
            frame.boundary == Frame::Boundary::none && frame.escaped_boundary == Frame::Boundary::none &&
            !frame.destination && !frame.selected_name && !frame.captured_configuration && !frame.captured_font &&
            !frame.captured_renderer && !frame.result && !frame.renderer_profile && !frame.renderer_target &&
            !frame.active_call_site && !frame.escaped_call_site && frame.native_eh_state == -1 &&
            !frame.cached && !frame.override_selected && !frame.height_matches && !frame.font_scale_matches &&
            !frame.empty_constructed && !frame.empty_compared && !frame.empty_returned && !frame.default_constructed &&
            !frame.renderer_entered && !frame.renderer_returned && !frame.shader_published &&
            !frame.cleanup_entered && !frame.cleanup_returned && !frame.exception_cleanup_started && !frame.attempted &&
            !frame.captured_name_data && !frame.captured_name_bytes &&
            c.phase == NativeMaterialEffectCacheAcquired::Phase::not_started && !c.loaded && !c.result &&
            !c.cache_published && !c.caller_acquired && !c.key_resolution &&
            l.phase == NativeMaterialEffectLoadPhase::not_started && !l.raw_slot && !l.creator && !l.companion &&
            !l.owner_record && !l.constructor_complete && !l.registered && !l.returned &&
            !l.name_resolution.acquired && !l.programs && !l.fallback && !l.constructor_texture;
        // names[] remain uninitialized. No native Font/image/header is read.
        // This metadata-only packet admits NO completed shader operation.
    }
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
    std::unique_ptr<SourceEqualGraph> source_equal;
    // Only the new mode needs this address BEFORE immutable Prefix construction.
    // The credited ctor stores the already-live reference_frame identity only.
    std::unique_ptr<FontShader> font_main_shader;
    std::unique_ptr<PrimaryPrefix> primary_prefix;
    std::unique_ptr<FontShader> font_shader;
    std::unique_ptr<FontWorkGraph> font_work;
    std::unique_ptr<EmptyUnchangedGraph> empty_unchanged;
    NativeGuiTextRawModelRetirementContext retirement;
    NativeGuiTextRawModelReferenceContext reference_context;
    alignas(NativeGuiTextRawModelReference) std::byte companion[sizeof(NativeGuiTextRawModelReference)];
    Disposal disposal;
    NativeGuiTextDefaultFactoryStorageContext factory_context;
    NativeGuiTextDefaultWidgetLifetimeRecord widget_record;
    volatile Word scalar_flags{}; // separate from companion, scalar frame and disposal extent

    Attempt(Services&, std::size_t);
    ~Attempt();
    void bind();
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

struct GameNativeRawDefaultTextOwner::Impl::SourceEqualGraph {
    Services& services;
    const NativeGuiTextReferenceFrame& reference_frame;
    alignas(4) std::byte input[8]; // uninitialized until genuine41E870
    NativeGuiTextSourceInputProgress input_progress;
    NativeGuiTextSourceStorageContext context; // reference bindings cannot retarget; Frame pins this object
    NativeGuiTextSourceStorageAcquired acquired;
    NativeGuiTextSourceStorageFrame frame;
    bool entered{}, returned{}, failed{};

    SourceEqualGraph(Attempt& a, Services& s)
        : services(s), reference_frame(a.reference_frame), input_progress(input, s.owner.vfs.raw_strings()),
          context{a.widget_record, *a.widgets, s.owner.vfs.raw_strings()},
          frame(context, input, input_progress, a.reference_frame, acquired) {
        // Forward widget/factory/publication references are ADDRESSES only.
        // Frame construction follows no context and reads no input header.
    }
    ~SourceEqualGraph() noexcept { if (!unused() && !settled()) std::terminate(); }
    bool unused() const noexcept {
        const auto& d = acquired.diagnostics;
        using B = NativeGuiTextSourceStorageAcquired::Boundary;
        return !entered && !returned && !failed &&
            !input_progress.construct_entered && !input_progress.construct_returned && !input_progress.construct_failed &&
            !input_progress.return_entered && !input_progress.returned && !input_progress.return_failed &&
            frame.matches(context, input, input_progress, reference_frame, acquired,
                NativeGuiTextSourceStorageFrame::Phase::fresh) &&
            !frame.entered() && !frame.comparison_entered() && !frame.comparison_returned() &&
            !frame.normal_unchanged_returned() && !frame.requires_process_retention() && !frame.failure() &&
            !acquired.bound_frame_identity() && !acquired.requires_process_retention() &&
            !acquired.failed() && !acquired.normal_unchanged_returned() &&
            d.boundary == B::none && d.escaped_boundary == B::none && !d.started && !d.comparison_entered &&
            !d.comparison_returned && !d.equal && !d.complete && !d.text && !d.input && !d.native_site && !d.failure;
    }
    bool retain() const noexcept {
        if (failed || frame.requires_process_retention() || frame.failure() || acquired.requires_process_retention() ||
            acquired.failed() || input_progress.construct_failed || input_progress.return_failed) return true;
        if (!entered) return !unused();
        return !returned || !input_progress.construct_entered || !input_progress.construct_returned ||
            !input_progress.return_entered || !input_progress.returned ||
            !frame.matches(context, input, input_progress, reference_frame, acquired,
                NativeGuiTextSourceStorageFrame::Phase::unchanged) ||
            !frame.entered() || !frame.comparison_entered() || !frame.comparison_returned() ||
            !frame.normal_unchanged_returned() || !acquired.normal_unchanged_returned() ||
            acquired.bound_frame_identity() != &frame;
    }
    bool settled() const noexcept { return unused() || !retain(); } // C++ metadata ONLY after native retirement
    void construct(NativeGuiTextRawModelReference& reference) {
        const bool was_unused = unused();
        entered = true; // selected operation BEFORE validation/input effects
        try {
            require(was_unused, "DEFAULT source-equal input/Frame graph cannot replay or resume");
            services.owner.require_source_equal_creating();
            input_progress.construct_entered = true;
            try {
                construct_native_string_header_0041e870(input, input_progress.strings, "");
                input_progress.construct_returned = true;
            } catch (...) { input_progress.construct_failed = true; throw; }
            services.owner.require_source_equal_creating();
            const auto result = submit_native_gui_text_source_unchanged_storage_00abaed0(reference,
                context, frame, acquired);
            services.owner.require_source_equal_creating();
            require(result == NativeGuiTextSourceStorageResult::unchanged &&
                frame.matches(context, input, input_progress, reference_frame, acquired,
                    NativeGuiTextSourceStorageFrame::Phase::unchanged) &&
                frame.normal_unchanged_returned() && frame.comparison_returned() &&
                !frame.requires_process_retention() && !frame.failure() &&
                acquired.bound_frame_identity() == &frame && acquired.normal_unchanged_returned() &&
                !acquired.requires_process_retention() && !acquired.failed(),
                "DEFAULT source-equal needs actual healthy private unchanged completion before input return");
            input_progress.return_entered = true;
            try {
                destroy_native_string_header_0041dd20(input, input_progress.strings);
                input_progress.returned = true;
            } catch (...) { input_progress.return_failed = true; throw; }
            // Header/data are identity/extent only after actual return.
            services.owner.require_source_equal_creating();
            returned = true;
        } catch (...) { failed = true; throw; } // retain reached input/Text/provider graph; no cleanup or retry
    }
};

struct GameNativeRawDefaultTextOwner::Impl::Geometry {
    const NativeNodeMappedRawTransformProfiles transforms;
    NativeModelStorageGeometryContext association;
    NativeGuiTextMainGeometryStorageContext construction;
    volatile Word arguments[4]{};
    volatile Word position[3]{};
    NativeGuiWidgetTransformScratch transform_scratch{};
    GuiWidgetBounds bounds_scratch{};
    NativeGuiTextMainGeometryStorageAcquired acquired;
    NativeGuiTextPrefixGeometryProgress progress;
    NativeGuiTextMainGeometryStorageFrame frame;
    const NativeGuiTextPrefixGeometryLifetimeContext lifetime;

    Geometry(Services& s, NativeModelStorageDispatchContext& primary,
        NativeModelStorageDispatchContext& internal,
        const NativeGuiTextPrimaryPrefixLifetimeContext* prefix)
        : transforms(s.owner.data), association{primary.model.storage, cell(s.owner.data, 0x00d7a260)},
          construction{primary, internal, association, s.geometry, transforms,
              *static_cast<const volatile float*>(s.owner.data.data_at(0x00d7a24c, 4)),
              *static_cast<const volatile float*>(s.owner.data.data_at(0x00d7a208, 4)),
              s.text_state.widget_y_scale_00e12fc4(),
              *static_cast<const volatile double*>(s.owner.data.data_at(0x00d7a280, 8))},
          frame(construction, arguments, position, transform_scratch, bounds_scratch),
          lifetime{prefix, construction, frame, acquired, progress,
              arguments, position, transform_scratch, bounds_scratch} {
        // prefix is a forward ADDRESS only. All native operands/scratch have
        // real initialized private backing; no native flag or payload is set.
    }
    ~Geometry() noexcept {
        if (retain() || (progress.returned &&
            (!lifetime.prefix || !native_gui_text_prefix_geometry_meshes_settled(*lifetime.prefix))))
            std::terminate();
    }
    bool unused() const noexcept {
        using P = NativeGuiTextMainGeometryStoragePhase;
        return frame.matches_backing(construction, arguments, position, transform_scratch, bounds_scratch) &&
            frame.phase() == P::fresh && acquired.phase == P::fresh && !acquired.failure &&
            !progress.entered && !progress.returned && !progress.failed && !acquired.destination &&
            !acquired.primary_captured && !acquired.mesh_entered && !acquired.mesh.started &&
            !acquired.association_entered && !acquired.creator_release_entered && !acquired.position_entered;
    }
    bool retain() const noexcept {
        using P = NativeGuiTextMainGeometryStoragePhase;
        if (!frame.matches_backing(construction, arguments, position, transform_scratch, bounds_scratch) ||
            progress.failed || acquired.failure || frame.phase() == P::entered || frame.phase() == P::failed ||
            acquired.phase == P::entered || acquired.phase == P::failed) return true;
        if (!progress.entered) return !unused();
        return !progress.returned || !frame.matches(construction, acquired, P::complete) || acquired.phase != P::complete;
    }
    void construct_geometry(NativeGuiTextRawModelReference& reference) {
        require(unused(), "private geometry requires its exact unused installed operation graph");
        progress.entered = true; // BEFORE379 preflight, even if its own acquisition stays fresh on failure.
        try {
            require(_query_new_handler() == nullptr && std::get_new_handler() == nullptr,
                "private geometry requires both real allocation handlers absent at its actual entry");
            const auto* const prefix = lifetime.prefix;
            require(prefix && prefix->text.publication.bind_complete && !prefix->text.publication.bind_failed &&
                !prefix->text.publication.companion_disposal_entered && !prefix->text.publication.disposed &&
                prefix->text.publication.reference == &reference &&
                reference.phase() == NativeGuiTextRawModelReference::Phase::bound && reference.storage() &&
                reference.storage() == prefix->text.factory.raw_slot &&
                &reference.storage_context() == &prefix->text.factory_context.publication &&
                prefix->text.factory_context.publication.owners.find(reference.storage()) == &reference,
                "private geometry bounds entry needs its SAME live canonical bound Text");
            const auto* const actual = static_cast<const volatile unsigned char*>(reference.storage());
            require(actual[0x74] == 0,
                "private geometry requires CURRENT Text bounds flag zero at its actual entry");
            construct_native_gui_text_main_geometry_storage_00ab7700(reference, construction, frame, acquired);
            progress.returned = true; // NORMAL body return FIRST, before Source metadata validation.
            require_native_gui_text_prefix_geometry_reservation(*lifetime.prefix);
        } catch (...) { progress.failed = true; throw; }
    }
};

struct GameNativeRawDefaultTextOwner::Impl::MainSection {
    NativeGuiTextMainSectionStorageContext construction;
    NativeGuiTextMainSectionStorageAcquired acquired;
    NativeGuiTextMainSectionStorageFrame frame;
    const NativeGuiTextMainSectionLifetimeStorageContext context;
    NativeGuiTextMainSectionLifetimeStorageFrame lifetime;
    // Independent owner progress also covers rejection before the lower
    // Frame enters, and validation failure after its genuine normal return.
    bool entered{}, returned{}, failed{};

    MainSection(Attempt& a, Geometry& geometry,
        const NativeGuiTextPrimaryModelStorageContext& primary_context,
        const NativeGuiTextPrimaryModelStorageFrame& primary_frame,
        const NativeGuiTextPrimaryModelStorageAcquired& primary)
        : construction{a.widget_record, *a.widgets, primary_context, primary_frame, primary,
              geometry.construction, geometry.frame, geometry.acquired, a.sections},
          frame(construction, acquired),
          context{geometry.lifetime, construction, frame, acquired, a.reference_frame},
          lifetime(context) {
        // Forward factory/widget references are ADDRESSES only until bind().
        // Real raw name preimages and the callee-owned cache remain untouched.
    }
    ~MainSection() noexcept {
        if (!unused() && !settled()) std::terminate();
    }
    bool unused() const noexcept {
        using P = NativeGuiTextMainSectionStoragePhase;
        using L = NativeGuiTextMainSectionLifetimeStorageFrame::Phase;
        return !entered && !returned && !failed && frame.matches(construction, acquired, P::fresh) &&
            acquired.phase == P::fresh && !acquired.failure && !acquired.destination &&
            !acquired.mesh_gate_read && !acquired.section.section.started && !acquired.section.appended &&
            acquired.section.material.phase == NativeMaterialFactoryAcquired::Phase::empty &&
            !acquired.section.material.cache && !acquired.section.material_published &&
            !acquired.name_construct_entered && !acquired.name_return_entered &&
            !acquired.parenting_entered && !acquired.root_entered &&
            &lifetime.prepared_context() == &context && !lifetime.failure() &&
            (lifetime.phase() == L::fresh || lifetime.phase() == L::prepared);
    }
    bool retain() const noexcept {
        using P = NativeGuiTextMainSectionStoragePhase;
        using L = NativeGuiTextMainSectionLifetimeStorageFrame::Phase;
        if (failed || acquired.failure || frame.phase() == P::entered || frame.phase() == P::failed ||
            acquired.phase == P::entered || acquired.phase == P::failed ||
            &lifetime.prepared_context() != &context || lifetime.requires_process_retention()) return true;
        if (!entered) return !unused();
        return !returned || !frame.matches(construction, acquired, P::complete) ||
            acquired.phase != P::complete || lifetime.phase() != L::sealed;
    }
    bool settled() const noexcept {
        if (unused()) return true;
        return !retain() && lifetime.post_text_quiescent(); // persistent C++ metadata ONLY
    }
    void construct(NativeGuiTextRawModelReference& reference, Impl& owner) {
        const bool was_unused = unused(); // pure owned metadata, cannot throw
        entered = true; // BEFORE validation or the lower native entry
        try {
            require(was_unused && lifetime.phase() == NativeGuiTextMainSectionLifetimeStorageFrame::Phase::prepared,
                "main section requires its exact unused prepared graph");
            owner.require_main_section_creating();
            ensure_native_gui_text_post_constructor_sections_storage_00ab8530(reference, construction, frame, acquired);
            returned = true; // genuine WHOLE body return BEFORE Source validation/seal
            owner.require_main_section_creating();
            lifetime.seal_after_main_section(construction.text.factory.raw_slot);
            owner.require_main_section_creating();
        } catch (...) { failed = true; throw; }
    }
};

struct GameNativeRawDefaultTextOwner::Impl::FontMainSelection {
    using Lifetime = NativeGuiTextFontLifetimeStorageFrame;
    FontShader& shader;
    const NativeGuiTextReferenceFrame& reference_frame;
    alignas(4) std::byte input[8]; // real41E870 is the sole initializer
    NativeGuiTextFontInputNameProgress input_progress;
    NativeGuiTextRawFontNameStorageContext selection_context;
    NativeGuiTextRawFontNameStorageAcquired selection;
    NativeGuiTextRawFontNameStorageFrame selection_frame;
    const NativeGuiTextFontLifetimeSelector selector;
    const NativeGuiTextFontLifetimeStorageContext context;
    Lifetime frame;
    bool entered{}, returned{}, failed{};

    FontMainSelection(Services& s, Geometry& geometry, FontShader& f,
        const NativeGuiTextReferenceFrame& reference)
        : shader(f), reference_frame(reference), input_progress(input, s.owner.vfs.raw_strings()),
          selection_context{f.context.fonts},
          selection_frame(selection_context, input, reference, f.frame, selection),
          selector{selection_context, selection_frame, selection, input_progress},
          context{geometry.lifetime, f.context.fonts, f.context, f.frame, reference, selector, nullptr},
          frame(context) {
        // Owned metadata and addresses only: no not-yet-constructed Attempt
        // factory/reference/publication object is inspected by these ctors.
    }
    ~FontMainSelection() noexcept { if (!unused() && !settled()) std::terminate(); }
    bool unused() const noexcept {
        return !entered && !returned && !failed &&
            !input_progress.construct_entered && !input_progress.construct_returned &&
            !input_progress.construct_failed && !input_progress.return_entered &&
            !input_progress.returned && !input_progress.return_failed && !selection.failed() &&
            !selection.publication_storage()->issued() &&
            selection_frame.matches(selection_context, input, reference_frame, shader.frame, selection,
                NativeGuiTextRawFontNameStorageFrame::Phase::fresh) &&
            &frame.prepared_context() == &context && !frame.failure() &&
            (frame.phase() == Lifetime::Phase::fresh || frame.phase() == Lifetime::Phase::prepared);
    }
    bool retain() const noexcept {
        using S = NativeGuiTextRawFontNameStorageFrame::Phase;
        if (failed || input_progress.construct_failed || input_progress.return_failed || selection.failed() ||
            selection_frame.phase() == S::entered || selection_frame.phase() == S::failed ||
            frame.requires_process_retention() || shader.retain()) return true;
        if (!entered) return !unused();
        return !returned || !input_progress.construct_entered || !input_progress.construct_returned ||
            !input_progress.return_entered || !input_progress.returned ||
            !selection_frame.matches(selection_context, input, reference_frame, shader.frame, selection, S::complete) ||
            frame.phase() != Lifetime::Phase::sealed;
    }
    bool settled() const noexcept {
        return unused() || (!retain() && frame.post_text_quiescent()); // retained C++ metadata only
    }
    void construct(NativeGuiTextRawModelReference& reference) {
        const bool was_unused = unused();
        entered = true; // BEFORE helper/input/native validation; ALL escapes retain
        try {
            require(was_unused && frame.phase() == Lifetime::Phase::prepared,
                "Font-before-Main selection requires its exact unused prepared graph");
            shader.owner.require_font_work_creating();
            frame.seal_after_prefix_geometry(reference.storage()); // BEFORE any Font input effect
            shader.owner.require_font_work_creating();
            input_progress.construct_entered = true;
            try {
                construct_native_string_header_0041e870(input, input_progress.strings, "Viper19");
                input_progress.construct_returned = true;
            } catch (...) { input_progress.construct_failed = true; throw; }
            shader.owner.require_font_work_creating();
            set_native_gui_text_font_name_storage_00ab8c30(reference, input,
                selection_context, selection_frame, selection);
            shader.owner.require_font_work_creating();
            input_progress.return_entered = true;
            try {
                destroy_native_string_header_0041dd20(input, input_progress.strings);
                input_progress.returned = true;
            } catch (...) { input_progress.return_failed = true; throw; }
            // The input's header/data are identity/extent only after return.
            shader.owner.require_font_work_creating();
            const auto* const published = selection.publication();
            const auto credit = shader.frame.credit_progress();
            using Shader = NativeGuiTextShaderStorageFrame;
            require(published && published->font_identity() && !selection_frame.normal_equal_name() &&
                credit.producer == Shader::RawPhase::fresh && credit.credit == Shader::CreditPhase::none &&
                !credit.effect && !credit.failed && !credit.active &&
                shader.frame.phase == Shader::Phase::fresh && !shader.frame.attempted && !shader.frame.shader_published,
                "Font-before-Main requires actual Font publication without any AB8CE0 producer/Effect credit");
            frame.require_live(reference.storage(), Lifetime::Stage::bound); // actual live Font/collection/header proof
            shader.owner.require_font_work_creating();
            returned = true;
        } catch (...) { failed = true; throw; } // no speculative input/native cleanup or retry
    }
};

struct GameNativeRawDefaultTextOwner::Impl::PrimaryPrefix {
    Services& services;
    NativeGuiTextRawModelRecord primary; // destroyed LAST, after metadata providers
    std::unique_ptr<NativeModelStorageNodeCall[]> calls;
    const std::size_t capacity;
    NativeModelStorageNodeDispatchAcquired node_acquired;
    NativeModelStorageDispatchAcquired tree_acquired;
    NativeModelStorageDispatch tree_dispatch;
    NativeModelStorageNodeDispatch node_dispatch;
    NativeSceneRegistrationGateContext gates;
    NativeNodeSceneAttachmentContext scenes;
    NativeNodeTreeRetirementContext trees;
    NativeNodeBaseDestructionContext node;
    NativeModelStorageContext storage;
    NativeModelStorageIdentityContext identity;
    NativeNodeParentingContext parenting;
    NativeModelStorageDispatchContext record;
    NativeModelStorageNodeDispatchContext node_record;
    alignas(4) std::byte input_name[8]; // NO initializer: only real41E870 writes this header
    NativeGuiTextPrimaryPrefixNameProgress name;
    NativeGuiTextPrimaryModelStorageContext producer_context;
    NativeGuiTextPrimaryModelStorageAcquired producer;
    NativeGuiTextPrimaryModelStorageFrame producer_frame;
    NativeGuiWidgetRawModelRoute primary_route, internal_route;
    NativeGuiWidgetRawModelLifetimeAcquired slots_acquired;
    NativeGuiWidgetRawModelSlots slots;
    NativeGuiWidgetRawModelLifetimeContext widget_context;
    std::unique_ptr<Geometry> geometry;
    std::unique_ptr<MainSection> main_section;
    std::unique_ptr<FontMainSelection> font_main_selection;
    const NativeGuiTextPrimaryPrefixLifetimeContext context;

    PrimaryPrefix(Attempt& a, Services& s, std::size_t count)
        : services(s), calls(std::make_unique<NativeModelStorageNodeCall[]>(count)), capacity(count),
          tree_dispatch(tree_acquired),
          node_dispatch(s.owner.data, s.owner.vfs.native_types(), s.owner.vfs.native_owners().types(),
              s.domain.node.scenes.gates.light_types, calls.get(), count, node_acquired),
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
          storage{node, game_native_resource_pool_process().model_pool_01090054(),
              s.owner.renderer.node_constants(), cell(s.owner.data, 0x00d7a208)},
          identity{storage, table(s.owner.data, 0x00d62de8, 0x58)},
          parenting{node, node_dispatch, s.owner.vfs.native_types().group_types().group_0109032c.own_id},
          record{identity, primary.actual, primary.reference, primary.construction, primary.lifetime_frame,
              primary.lifetime, primary.release_frame, primary.release},
          node_record{record, tree_dispatch, parenting, s.domain.node, s.base_nodes},
          name(input_name, s.owner.vfs.raw_strings()),
          producer_context{game_native_resource_pool_process(), a.factory_context, a.factory, a.publication,
              primary, node_record, node_dispatch, a.tree_record, input_name},
          producer_frame(producer_context, producer),
          primary_route{record, tree_dispatch, tree_acquired, node_dispatch, node_acquired},
          internal_route{a.tree_record, a.tree_dispatch, a.tree_acquired, a.node_dispatch, a.node_acquired},
          slots(slots_acquired), widget_context{a.widget_record, *a.widgets, a.widgets->context(),
              primary_route, internal_route, slots},
          geometry(s.mode == Mode::primary_prefix_geometry || s.mode == Mode::primary_prefix_geometry_font ||
              s.mode == Mode::primary_prefix_geometry_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
              std::make_unique<Geometry>(s, record, a.tree_record, &context) : nullptr),
          main_section(s.mode == Mode::primary_prefix_geometry_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
              std::make_unique<MainSection>(a, *geometry, producer_context, producer_frame, producer) : nullptr),
          font_main_selection(s.mode == Mode::primary_prefix_geometry_font_main_section ||
              s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
              std::make_unique<FontMainSelection>(s, *geometry, *a.font_main_shader, a.reference_frame) : nullptr),
          context{a.widget_record, *a.widgets, a.widget_acquired, producer_context, producer_frame, producer,
              name, widget_context, a.node_record, slots_acquired,
              {calls.get(), count}, {a.node_calls.get(), a.node_call_capacity},
              font_main_selection ? NativeGuiTextPrimaryPrefixOperation(geometry->lifetime,
                  main_section->lifetime, font_main_selection->frame) :
              main_section ? NativeGuiTextPrimaryPrefixOperation(geometry->lifetime, main_section->lifetime) :
              geometry ? NativeGuiTextPrimaryPrefixOperation(geometry->lifetime) : NativeGuiTextPrimaryPrefixOperation{}} {
        // Forward references above take addresses ONLY. In particular the
        // Attempt's factory/history objects are not inspected until bind().
    }
    void bind() {
        producer_frame.prepare_before_text_allocation();
        slots.bind(widget_context); // BOTH exact Model providers now bound; Context already installed
        if (font_main_selection) font_main_selection->frame.prepare_before_text_allocation();
        if (main_section) main_section->lifetime.prepare_before_text_allocation();
        if (geometry) require_native_gui_text_prefix_geometry_reservation(context);
    }
    ~PrimaryPrefix() {
        if (!unused() && !settled()) std::terminate();
        font_main_selection.reset(); // independent Font post, BEFORE Main/Geometry/provider lifetimes end
        main_section.reset(); // while Geometry, immutable Context and BOTH Model/provider records remain alive
        geometry.reset(); // while immutable Context and BOTH Model records/providers remain alive
        primary.reference.reset(); // already retired, while every provider/context is still alive
    }
    void construct() {
        name.construct_entered = true;
        try {
            construct_native_string_header_0041e870(input_name, name.strings, "NativeGuiPrimaryPrefix");
            name.construct_returned = true;
        } catch (...) { name.construct_failed = true; throw; }
        produce_native_gui_text_primary_model_00aaad95(producer_context, producer_frame, producer);
        require(producer.publication() != nullptr, "primary prefix did not issue genuine provenance");
        name.return_entered = true;
        try {
            destroy_native_string_header_0041dd20(input_name, name.strings);
            name.returned = true;
        } catch (...) { name.return_failed = true; throw; }
        require(native_gui_text_primary_prefix_selected(context), "primary prefix name/receipt did not complete");
        // input_name/header/data are diagnostic ONLY after the actual return.
    }
    bool unused() const noexcept {
        using P = NativeGuiTextPrimaryModelStoragePhase;
        using S = NativeGuiWidgetRawModelLifetimeAcquired::Phase;
        return ((producer.phase == P::fresh && producer_frame.matches(producer_context, producer, P::fresh)) ||
            (producer.phase == P::prepared && producer_frame.matches(producer_context, producer, P::prepared))) &&
            !producer.allocation_entered && !producer.constructor_entered && !producer.publication() &&
            !primary.actual && !primary.reference && !primary.construction.started &&
            !primary.lifetime.started && !primary.release.started &&
            !name.construct_entered && !name.construct_returned && !name.construct_failed &&
            !name.return_entered && !name.returned && !name.return_failed &&
            !slots_acquired.active && !slots_acquired.poisoned && slots_acquired.scene == S::fresh &&
            slots_acquired.base == S::fresh && slots_acquired.glyph == S::fresh &&
            !tree_acquired.poisoned && !node_acquired.poisoned && !node_acquired.used &&
            (!geometry || geometry->unused()) && (!main_section || main_section->unused()) &&
            (!font_main_selection || font_main_selection->unused());
    }
    bool retain() const noexcept {
        using P = NativeGuiTextPrimaryModelStoragePhase;
        using S = NativeGuiWidgetRawModelLifetimeAcquired::Phase;
        if (font_main_selection && font_main_selection->retain()) return true;
        if (main_section && main_section->retain()) return true;
        if (geometry && geometry->retain()) return true;
        if ((producer.phase == P::fresh && !producer_frame.matches(producer_context, producer, P::fresh)) ||
            (producer.phase == P::prepared && !producer_frame.matches(producer_context, producer, P::prepared)) ||
            (producer.phase == P::complete && !producer_frame.matches(producer_context, producer, P::complete))) return true;
        if (name.construct_failed || name.return_failed || (name.construct_entered && !name.returned) ||
            producer.failure || producer.phase == P::preparing || producer.phase == P::entered ||
            producer.phase == P::failed || slots_acquired.active || slots_acquired.poisoned ||
            slots_acquired.scene == S::entered || slots_acquired.scene == S::failed ||
            slots_acquired.base == S::entered || slots_acquired.base == S::failed ||
            slots_acquired.glyph == S::entered || slots_acquired.glyph == S::failed ||
            tree_acquired.poisoned || node_acquired.poisoned || node_acquired.used > capacity) return true;
        using C = NativeModelStorageDispatchCallAcquired::Phase;
        if (tree_acquired.current18.phase == C::entered || tree_acquired.current18.phase == C::failed ||
            tree_acquired.current0.phase == C::entered || tree_acquired.current0.phase == C::failed) return true;
        for (std::size_t i = 0; i < node_acquired.used; ++i)
            if (calls[i].phase == NativeModelStorageNodeCall::Phase::entered ||
                calls[i].phase == NativeModelStorageNodeCall::Phase::failed) return true;
        return false;
    }
    bool settled() const noexcept {
        using P = NativeGuiTextPrimaryModelStoragePhase;
        using S = NativeGuiWidgetRawModelLifetimeAcquired::Phase;
        using C = NativeModelStorageDispatchCallAcquired::Phase;
        if (node_acquired.used > capacity) return false;
        for (std::size_t i = 0; i < node_acquired.used; ++i)
            if (calls[i].phase != NativeModelStorageNodeCall::Phase::complete) return false;
        return !retain() && producer.phase == P::complete && producer.publication() &&
            producer_frame.matches(producer_context, producer, P::complete) &&
            name.construct_entered && name.construct_returned && !name.construct_failed &&
            name.return_entered && name.returned && !name.return_failed &&
            primary.reference && primary.reference->retired() && primary.release.started && primary.release.complete &&
            primary.lifetime.complete && primary.lifetime.scalar_complete && primary.lifetime.flags_read &&
            primary.lifetime.node.complete && primary.lifetime.node.base_cleanup_completed &&
            primary.lifetime.pool_returned && (primary.lifetime.flags_low_byte & 1u) &&
            tree_acquired.current18.phase == C::complete && tree_acquired.current0.phase == C::complete &&
            tree_acquired.current04_entered && tree_acquired.current04_returned &&
            slots_acquired.bound && slots_acquired.scene == S::complete && slots_acquired.base == S::complete &&
            slots_acquired.scene4c.entered && slots_acquired.scene4c.returned && slots_acquired.scene4c.unlink.complete &&
            (!slots_acquired.late4c.entered || (slots_acquired.late4c.returned && slots_acquired.late4c.unlink.complete)) &&
            (!font_main_selection || font_main_selection->settled()) && (!main_section || main_section->settled()) &&
            (!geometry || native_gui_text_prefix_geometry_meshes_settled(context));
    }
};

struct GameNativeRawDefaultTextOwner::Impl::FontWorkGraph {
    using Lifetime = NativeGuiTextFontLifetimeStorageFrame;
    using Shader = NativeGuiTextShaderStorageFrame;
    struct Input {
        alignas(4) std::byte header[8]; // ONLY genuine41E870 initializes this header
        NativeGuiTextFontInputNameProgress progress;
        explicit Input(NativeStringRawPoolContext& strings) noexcept : progress(header, strings) {}
        bool unused() const noexcept {
            return !progress.construct_entered && !progress.construct_returned && !progress.construct_failed &&
                !progress.return_entered && !progress.returned && !progress.return_failed;
        }
        bool retain() const noexcept {
            return progress.construct_failed || progress.return_failed ||
                (progress.construct_entered && !progress.returned);
        }
        void construct(const char* literal) {
            require(unused(), "raw Font input constructor is once-only over its actual external header");
            progress.construct_entered = true;
            try {
                construct_native_string_header_0041e870(header, progress.strings, literal);
                progress.construct_returned = true;
            } catch (...) { progress.construct_failed = true; throw; }
        }
        void return_header() {
            require(progress.construct_entered && progress.construct_returned && !progress.construct_failed &&
                !progress.return_entered && !progress.returned && !progress.return_failed,
                "raw Font input return requires its SAME genuine normal constructor");
            progress.return_entered = true;
            try {
                destroy_native_string_header_0041dd20(header, progress.strings);
                progress.returned = true;
            } catch (...) { progress.return_failed = true; throw; }
            // No current header/data access after this actual return.
        }
    };
    struct Setter {
        FontShader& shader;
        const NativeGuiTextReferenceFrame& reference_frame;
        Input input;
        NativeGuiTextRawFontNameStorageContext context;
        NativeGuiTextRawFontNameStorageAcquired acquired;
        NativeGuiTextRawFontNameStorageFrame frame;
        const NativeGuiTextFontLifetimeSelector selector;
        Setter(Services& s, FontShader& f, const NativeGuiTextReferenceFrame& reference,
            const NativeGuiTextRawFontNameStoragePublication* prior = nullptr)
            : shader(f), reference_frame(reference), input(s.owner.vfs.raw_strings()), context{f.context.fonts},
              frame(context, input.header, reference, f.frame, acquired, prior),
              selector{context, frame, acquired, input.progress} {}
        bool unused() const noexcept {
            return input.unused() && !acquired.failed() && !acquired.publication_storage()->issued() &&
                frame.matches(context, input.header, reference_frame, shader.frame, acquired,
                    NativeGuiTextRawFontNameStorageFrame::Phase::fresh);
        }
        bool retain() const noexcept {
            using P = NativeGuiTextRawFontNameStorageFrame::Phase;
            return input.retain() || acquired.failed() || frame.phase() == P::entered || frame.phase() == P::failed;
        }
        void run(NativeGuiTextRawModelReference& reference, const char* literal) {
            shader.owner.require_font_work_creating();
            input.construct(literal);
            shader.owner.require_font_work_creating();
            set_native_gui_text_font_name_storage_00ab8c30(reference, input.header, context, frame, acquired);
            shader.owner.require_font_work_creating();
            input.return_header();
            shader.owner.require_font_work_creating();
        }
    };
    const FontWork choice;
    FontShader& shader;
    Setter initial;
    std::unique_ptr<Setter> second;
    const NativeGuiTextFontLifetimeStorageContext context;
    Lifetime frame;
    bool entered{}, returned{}, failed{};

    FontWorkGraph(Attempt& a, Services& s)
        : choice(s.font_work), shader(*a.font_shader), initial(s, shader, a.reference_frame),
          second(choice == FontWork::shader_then_retirement ? nullptr :
              std::make_unique<Setter>(s, shader, a.reference_frame, initial.acquired.publication_storage())),
          context{a.primary_prefix->geometry->lifetime, *s.owner.font_resources, shader.context, shader.frame,
              a.reference_frame, initial.selector, second ? &second->selector : nullptr}, frame(context) {
        // The geometry graph contains forward widget/factory addresses only;
        // no later Attempt member, input header or native payload is inspected.
    }
    ~FontWorkGraph() noexcept { if (!unused() && !settled()) std::terminate(); }
    bool unused() const noexcept {
        const auto p = frame.phase();
        return !entered && !returned && !failed && initial.unused() && (!second || second->unused()) &&
            shader.untouched() && !frame.requires_process_retention() &&
            (p == Lifetime::Phase::fresh || p == Lifetime::Phase::prepared);
    }
    bool retain() const noexcept {
        return failed || (entered && !returned) || frame.requires_process_retention() ||
            initial.retain() || (second && second->retain()) || shader.retain();
    }
    bool settled() const noexcept {
        return !retain() && frame.post_text_quiescent(); // independent metadata ONLY, also native-factory-null
    }
    void construct(NativeGuiTextRawModelReference& reference) {
        require(unused() && frame.phase() == Lifetime::Phase::prepared,
            "raw Font work requires its exact once-prepared closed operation graph");
        entered = true; // BEFORE seal or any input/native attempt; no partial-work fallback
        try {
            shader.owner.require_font_work_creating();
            frame.seal_after_prefix_geometry(reference.storage()); // real379 complete BEFORE first input
            initial.run(reference, "Viper19");
            const auto* const first = initial.acquired.publication();
            require(first && first->font_identity() && !initial.frame.normal_equal_name(),
                "raw Font work needs actual initial Viper19 lookup and normal header publication");
            const bool attempted = ensure_native_gui_text_font_shader_storage_00ab8ce0(reference, shader.context, shader.frame);
            shader.owner.require_font_work_creating();
            const auto produced = shader.frame.credit_progress();
            require(attempted && produced.producer == Shader::RawPhase::complete &&
                produced.credit == Shader::CreditPhase::outstanding && produced.effect && !produced.failed && !produced.active,
                "raw Font work needs one genuine nonnull privately issued B318B0 caller credit");
            if (second) {
                second->run(reference, choice == FontWork::invalidate_shader ? "Arial18" : "Viper19");
                const auto consumed = shader.frame.credit_progress();
                if (choice == FontWork::invalidate_shader) {
                    const auto* const publication = second->acquired.publication();
                    require(publication && publication->font_identity() &&
                        publication->font_identity() != first->font_identity() && !second->frame.normal_equal_name() &&
                        consumed.credit == Shader::CreditPhase::returned &&
                        consumed.consumer == Shader::Consumer::font_name && consumed.consumer_returned,
                        "raw Font invalidation needs actual distinct Arial18 publication and real setter credit return");
                } else {
                    require(choice == FontWork::equal_name && second->frame.normal_equal_name() &&
                        !second->acquired.publication() && consumed.credit == Shader::CreditPhase::outstanding &&
                        consumed.effect == produced.effect,
                        "raw equal-name work must retain the original header/effect credit without new publication");
                }
            }
            frame.require_live(reference.storage(), Lifetime::Stage::bound);
            shader.owner.require_font_work_creating();
            returned = true;
        } catch (...) { failed = true; throw; } // preserve ALL reached owners/headers/credit; no cleanup/retry
    }
};

struct GameNativeRawDefaultTextOwner::Impl::EmptyUnchangedGraph {
    using Leaf = NativeGuiTextEmptyUnchangedStorageFrame;
    using Ensure = NativeGuiTextMainSectionStoragePhase;
    Services& services;
    const NativeGuiTextReferenceFrame& reference_frame;
    alignas(4) std::byte input[8]; // ONLY actual4C5E60 initializes this NEW wide header
    NativeGuiTextEmptyUnchangedInputProgress input_progress;
    NativeGuiTextMainSectionStorageAcquired ensure_acquired;
    NativeGuiTextMainSectionStorageFrame ensure_frame;
    const NativeGuiTextEmptyUnchangedStorageContext context;
    NativeGuiTextEmptyUnchangedStorageAcquired acquired;
    NativeGuiTextEmptyUnchangedStorageFrame frame;
    bool prepared{}, entered{}, returned{}, failed{};
    struct Region { const volatile void* begin; std::size_t bytes; };
    static std::uintptr_t bits(const volatile void* p) noexcept { return reinterpret_cast<std::uintptr_t>(p); }
    static bool valid(Region r) noexcept {
        return r.begin && r.bytes && static_cast<std::uint64_t>(bits(r.begin)) + r.bytes <= 0x100000000ull;
    }
    static bool separate(Region a, Region b) noexcept {
        return static_cast<std::uint64_t>(bits(a.begin)) + a.bytes <= bits(b.begin) ||
            static_cast<std::uint64_t>(bits(b.begin)) + b.bytes <= bits(a.begin);
    }

    EmptyUnchangedGraph(Attempt& a, Services& s)
        : services(s), reference_frame(a.reference_frame),
          input_progress(input, s.raw.strings, s.owner.vfs.raw_strings()),
          ensure_frame(a.primary_prefix->main_section->construction, ensure_acquired),
          context{a.primary_prefix->context, a.primary_prefix->main_section->lifetime,
              a.primary_prefix->font_main_selection->frame, a.primary_prefix->main_section->construction,
              ensure_frame, ensure_acquired, s.raw.strings, s.owner.vfs.raw_strings()},
          frame(context, input, input_progress, reference_frame, acquired) {
        // Forward factory/widget references in the existing peers are ONLY
        // addresses. Whole-Attempt preparation follows all aggregate members.
    }
    ~EmptyUnchangedGraph() noexcept { if (!settled()) std::terminate(); }
    bool closed_calls() const noexcept {
        const auto& p = context.prefix;
        const auto fresh = [](const NativeGuiWidgetRawModelRoute& route, NativeGuiTextPrimaryPrefixCalls calls) {
            const auto& n = route.node_acquired;
            if (!n.bound || n.poisoned || n.entry_failed || n.capacity_failed || n.used || !calls.data ||
                !calls.count || calls.count > 0xffffffffu / sizeof(*calls.data)) return false;
            for (std::size_t i = 0; i < calls.count; ++i) {
                const auto& c = calls.data[i];
                if (c.phase != NativeModelStorageNodeCall::Phase::fresh || c.kind != NativeModelStorageNodeCall::Kind::none ||
                    c.receiver || c.target || c.admitted_profile || c.first_argument || c.second_argument ||
                    c.result_al || c.body_entered || c.body_returned) return false;
            }
            return true;
        };
        return p.widget.primary.node.matches(p.producer_context.primary_node_context,
                p.widget.primary.node_acquired, p.primary_calls.data, p.primary_calls.count) &&
            p.widget.internal188.node.matches(p.internal_node_context,
                p.widget.internal188.node_acquired, p.internal_calls.data, p.internal_calls.count) &&
            fresh(p.widget.primary, p.primary_calls) && fresh(p.widget.internal188, p.internal_calls);
    }
    void require_closed_calls() const {
        require(closed_calls(), "EMPTY closed route rejects ANY NodeCall use/reentry; never reset, rebind or add spare budget");
    }
    void require_external(Attempt& a, Region r) const {
        require(valid(r) && separate({this, sizeof(*this)}, r) &&
            separate({&a.scalar_flags, sizeof(a.scalar_flags)}, r),
            "EMPTY whole input/Ensure/leaf graph and retained flags need separate complete external backing");
    }
    void check_live_aliases(Attempt& a, NativeGuiTextRawModelReference& reference) const {
        const auto regions = frame.live_regions(reference); // FULL BOUND, before input; Temp/header unissued, never sampled.
        const auto own_metadata_count = frame.metadata_regions().count;
        for (std::size_t i = own_metadata_count; i < regions.count; ++i)
            require_external(a, {regions.entries[i].begin, regions.entries[i].bytes});
        const auto check = [&](Region r) { require_external(a, r); };
        if (const auto* cache = context.main.prepared_context().acquired.section.material.cache.get())
            gui_text_backing_detail::shader_cache_backing<Region>(*cache, check);
        gui_text_backing_detail::shader_cache_backing<Region>(context.font.prepared_context().shader.cache, check);
    }
    bool unused() const noexcept {
        return !entered && !returned && !failed &&
            !input_progress.construct_entered && !input_progress.construct_returned && !input_progress.construct_failed &&
            !input_progress.return_entered && !input_progress.returned && !input_progress.return_failed &&
            frame.matches(context, input, input_progress, reference_frame, acquired, Leaf::Phase::fresh) &&
            !frame.requires_process_retention() && !frame.normal_empty_returned() &&
            !acquired.failed() && !acquired.requires_process_retention() && !acquired.bound_frame_identity() &&
            !acquired.normal_empty_returned() && ensure_frame.matches(context.construction, ensure_acquired, Ensure::fresh) &&
            ensure_acquired.phase == Ensure::fresh && !ensure_acquired.failure;
    }
    bool retain() const noexcept {
        if (failed || input_progress.construct_failed || input_progress.return_failed || frame.requires_process_retention() ||
            acquired.requires_process_retention() || ensure_acquired.failure ||
            ensure_frame.phase() == Ensure::entered || ensure_frame.phase() == Ensure::failed ||
            ensure_acquired.phase == Ensure::entered || ensure_acquired.phase == Ensure::failed ||
            (prepared && !closed_calls())) return true;
        if (!entered) return !unused();
        return !returned || !frame.completed_without_temp_debt() || !acquired.normal_empty_returned() ||
            !input_progress.construct_returned || !input_progress.return_entered || !input_progress.returned;
    }
    bool settled() const noexcept {
        if (unused()) return !prepared || closed_calls();
        return prepared && !retain() && ensure_frame.matches(context.construction, ensure_acquired, Ensure::complete) &&
            ensure_acquired.phase == Ensure::complete && !ensure_acquired.failure;
    }
    void prepare_metadata(Attempt& a) {
        try {
            require(!prepared && unused() && a.primary_prefix && a.primary_prefix->unused() &&
                !a.factory.started && !a.construction.started && !a.publication.bind_entered &&
                !a.tree_record.actual && !a.tree_record.reference && !a.tree_record.construction.started &&
                a.retirement.empty_unchanged == &frame && &a.retirement == &a.reference_context.retirement &&
                &context.strings == &services.raw.strings && &context.raw_strings == &a.sections.strings &&
                &context.raw_strings == &services.owner.vfs.raw_strings(),
                "EMPTY metadata binds exact unused records before pools, never a fabricated native Model binding");
            require_closed_calls(); // actual metadata slots/providers already bound; native records remain untouched.
            const auto metadata = frame.metadata_regions();
            for (std::size_t i = 0; i < metadata.count; ++i) {
                const auto& r = metadata.entries[i];
                require(r.begin && r.bytes && static_cast<std::uint64_t>(bits(r.begin)) + r.bytes <= 0x100000000ull &&
                    separate({&a.scalar_flags, sizeof(a.scalar_flags)}, {r.begin, r.bytes}) &&
                    separate({a.companion, sizeof(a.companion)}, {r.begin, r.bytes}) &&
                    separate({&a.disposal, sizeof(a.disposal)}, {r.begin, r.bytes}),
                    "EMPTY input/Temp/metadata/CELL backing must be disjoint from actual flags/companion/disposal");
            }
            const auto& p = context.prefix;
            for (const Region r : {Region{p.primary_calls.data, p.primary_calls.count * sizeof(*p.primary_calls.data)},
                {p.internal_calls.data, p.internal_calls.count * sizeof(*p.internal_calls.data)},
                {&p, sizeof(p)}, {p.name.header8, 8}, {&context.strings, sizeof(context.strings)},
                {&context.raw_strings, sizeof(context.raw_strings)},
                {&context.raw_strings.actual_published_01090aa8, sizeof(void*)},
                {&context.raw_strings.actual_small_returns_disabled_01090aa4, sizeof(Word)},
                {&context.raw_strings.actual_manager_publication_01090aa0, sizeof(void*)}})
                require_external(a, r);
            const auto main = context.main.metadata_regions();
            for (std::size_t i = 0; i < main.count; ++i) require_external(a, {main.entries[i].begin, main.entries[i].bytes});
            const auto font = context.font.metadata_regions();
            for (std::size_t i = 0; i < font.count; ++i) require_external(a, {font.entries[i].begin, font.entries[i].bytes});
            frame.require_retirement_quiescent(); // fresh full diagnostics; no native or header read.
            prepared = true;
        } catch (...) { failed = true; throw; }
    }
    void construct(NativeGuiTextRawModelReference& reference) {
        const bool was_unused = unused();
        entered = true;
        try {
            require(prepared && was_unused, "EMPTY owner requires its once-prepared independent wide/Ensure/leaf graph");
            auto& owner = services.owner;
            owner.require_font_work_creating();
            require_closed_calls();
            // Saved companion can be diagnostic after a rejected callback:
            // use retained factory identity; each helper fences ended history
            // BEFORE any companion/native access, as does the leaf itself.
            void* const actual = context.construction.text.factory.raw_slot;
            context.font.require_live(actual, NativeGuiTextFontLifetimeStorageFrame::Stage::bound);
            context.main.require_live(actual, NativeGuiTextMainSectionLifetimeStorageFrame::Stage::bound);
            owner.require_font_work_creating();
            auto& a = *services.attempt;
            require(a.empty_unchanged.get() == this, "EMPTY native entry must use its retained complete Attempt graph");
            check_live_aliases(a, reference);
            owner.require_font_work_creating();
            require_closed_calls(); // AFTER reached live-provider checks and immediately BEFORE input effects.
            input_progress.construct_entered = true;
            void* constructed;
            try {
                constructed = construct_native_wide_string_header_004c5e60(input, "", input_progress.strings);
                input_progress.construct_returned = true;
            } catch (...) { input_progress.construct_failed = true; throw; }
            require(constructed == input, "actual wide constructor must return SAME header address");
            owner.require_font_work_creating();
            require_closed_calls();
            const auto result = submit_native_gui_text_empty_unchanged_storage_00aba8d0(reference, context, frame, acquired);
            owner.require_font_work_creating();
            require(result == NativeGuiTextEmptyUnchangedStorageResult::unchanged_empty &&
                frame.completed_without_temp_debt() && !frame.requires_process_retention() &&
                acquired.normal_empty_returned() && !acquired.requires_process_retention(),
                "EMPTY owner needs private healthy normal leaf/Temp completion, not public diagnostics");
            require_closed_calls();
            input_progress.return_entered = true;
            try {
                destroy_native_wide_string_header_00436430(input, input_progress.raw_strings);
                input_progress.returned = true;
            } catch (...) { input_progress.return_failed = true; throw; }
            owner.require_font_work_creating();
            frame.require_retirement_quiescent(); // ONLY after the real caller-input return.
            require_closed_calls();
            owner.require_font_work_creating();
            returned = true;
        } catch (...) { failed = true; throw; } // retain all prior work; never speculate cleanup/retry.
    }
};

GameNativeRawDefaultTextOwner::Impl::Attempt::Attempt(Services& s, std::size_t capacity)
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
      source_equal(s.mode == Mode::default_source_equal ? std::make_unique<SourceEqualGraph>(*this, s) : nullptr),
      font_main_shader(s.mode == Mode::primary_prefix_geometry_font_main_section ||
          s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
          std::make_unique<FontShader>(s, reference_frame) : nullptr),
      primary_prefix(s.mode == Mode::default_source_equal ? nullptr :
          s.mode != Mode::default_lifecycle ? std::make_unique<PrimaryPrefix>(*this, s, capacity) : nullptr),
      font_shader(s.mode == Mode::primary_prefix_geometry_font_main_section ||
          s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ? nullptr :
          s.owner.font_link == FontLink::bound ? std::make_unique<FontShader>(s, reference_frame) : nullptr),
      font_work(s.mode == Mode::primary_prefix_geometry_font ? std::make_unique<FontWorkGraph>(*this, s) : nullptr),
      empty_unchanged(s.mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
          std::make_unique<EmptyUnchangedGraph>(*this, s) : nullptr),
      retirement{sections, construction.sections.model, widgets->context(), glyphs, s.process.pool_00f8bdf0(),
          primary_prefix ? &primary_prefix->context : nullptr,
          font_main_shader ? &font_main_shader->frame : font_work ? &font_shader->frame : nullptr,
          primary_prefix && primary_prefix->font_main_selection ? &primary_prefix->font_main_selection->frame :
              font_work ? &font_work->frame : nullptr,
          empty_unchanged ? &empty_unchanged->frame : nullptr},
      reference_context{s.owner.renderer.actual_owners(), s.owner.data, default_context, default_frame, construction, retirement,
          font_main_shader ? &font_main_shader->frame : font_shader ? &font_shader->frame : nullptr},
      disposal{&s.owner, this},
      factory_context{s.process, reference_context, construction, reference_frame, companion, sizeof(companion),
          {&disposal, sizeof(disposal), &Attempt::dispose}},
      widget_record{factory_context, factory, publication} {}
GameNativeRawDefaultTextOwner::Impl::Attempt::~Attempt() {
    if (!Impl::attempt_settled(*this) || (publication.bind_complete && !publication.disposed)) std::terminate();
    // No native cleanup. Reset dependent metadata while both Model records,
    // all acquisitions and private widget history are STILL alive.
    source_equal.reset(); // settled input/Frame metadata, BEFORE widget/Model/reference/provider backing ends
    empty_unchanged.reset(); // settled wide input/leaf, BEFORE FontSelection/Main/Geometry/backing ends
    font_work.reset(); // while helper/selector records, ShaderFrame, geometry and BOTH Model providers remain alive
    primary_prefix.reset();
    font_shader.reset(); // while collection/producer C++ metadata and real cache/provider contexts remain alive
    font_main_shader.reset(); // selection ended inside Prefix; exact122/provider backing is still alive
    widgets.reset();
    construction.sections.model.reference.reset();
}
void GameNativeRawDefaultTextOwner::Impl::Attempt::bind() {
    tree_dispatch.bind(tree_record);
    node_dispatch.bind(node_record);
    widgets->bind(widget_record);
    if (primary_prefix) primary_prefix->bind(); // full Attempt is already installed
    if (font_work) font_work->frame.prepare_before_text_allocation(); // no forward member is dereferenced early
    if (empty_unchanged) empty_unchanged->prepare_metadata(*this); // all actual records/contexts now exist
}

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
    if (mode == Mode::default_source_equal)
        require(attempt->source_equal && attempt->source_equal->unused() && !attempt->primary_prefix &&
            !attempt->font_shader && !attempt->font_main_shader && !attempt->font_work &&
            owner.font_link == FontLink::none && owner.phase == Phase::preparing && !owner.failure,
            "DEFAULT source-equal must reserve its exact fresh Source/input graph before pools, without Font/Main");
    if (mode == Mode::primary_prefix_geometry_font)
        require(attempt->font_work && attempt->font_work->unused() &&
            attempt->font_work->frame.phase() == NativeGuiTextFontLifetimeStorageFrame::Phase::prepared,
            "raw Font work must reserve its full genuine helper/input/selector graph before pools");
    if (owner.font_link != FontLink::none)
        require(owner.font_link == FontLink::bound && owner.font_producer && owner.font_resources &&
            owner.phase == Phase::preparing && !owner.failure && !owner.font_producer->failure() &&
            (mode == Mode::primary_prefix_geometry_font_main_section ||
                mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged ?
                attempt->font_main_shader && attempt->font_main_shader->untouched() && attempt->primary_prefix &&
                    attempt->primary_prefix->font_main_selection && attempt->primary_prefix->font_main_selection->unused() &&
                    attempt->primary_prefix->font_main_selection->frame.phase() ==
                        NativeGuiTextFontLifetimeStorageFrame::Phase::prepared :
                attempt->font_shader && attempt->font_shader->untouched()),
            "raw Font metadata preparation cannot resume a failed or reentrant link");
}
bool GameNativeRawDefaultTextOwner::Impl::custody_settled() const noexcept {
    if (!services || !services->attempt) return true;
    return attempt_settled(*services->attempt);
}
bool GameNativeRawDefaultTextOwner::Impl::attempt_settled(const Attempt& a) noexcept {
    if (a.source_equal && !a.source_equal->settled()) return false;
    if (a.empty_unchanged && !a.empty_unchanged->settled()) return false;
    if (a.font_main_shader && (a.font_main_shader->retain() || !a.primary_prefix ||
        !a.primary_prefix->font_main_selection || !a.primary_prefix->font_main_selection->settled())) return false;
    if (a.font_work) {
        if (!a.font_work->settled()) return false;
    } else if (a.font_shader && !a.font_shader->untouched()) return false;
    const bool prefix_unused = !a.primary_prefix || a.primary_prefix->unused();
    if (!a.factory.started) return prefix_unused;
    if (a.factory.phase == NativeGuiTextDefaultFactoryStorageAcquired::Phase::empty)
        return a.factory.complete && !a.factory.constructor_entered && !a.factory.raw_slot && prefix_unused;
    const auto& p = a.publication;
    const auto& scalar = a.reference_frame.scalar;
    if (!p.bind_complete || p.bind_failed || !p.scalar_entered || !p.scalar_returned || p.scalar_failed ||
        !p.companion_disposal_entered || !p.disposed || !scalar.complete || !scalar.destructor_returned ||
        !scalar.destruction.complete || !scalar.flags_captured || !scalar.pool_returned ||
        !(scalar.captured_flags_byte & 1u)) return false;
    const auto& m = a.construction.sections.model;
    const bool internal_returned = m.reference && m.reference->retired() && m.lifetime.complete && m.lifetime.scalar_complete &&
        m.lifetime.flags_read && m.lifetime.pool_returned && (m.lifetime.flags_low_byte & 1u);
    if (!internal_returned) return false;
    if (!a.primary_prefix) return true; // unchanged DEFAULT settlement
    using C = NativeModelStorageDispatchCallAcquired::Phase;
    if (a.node_acquired.used > a.node_call_capacity || a.node_acquired.poisoned || a.tree_acquired.poisoned) return false;
    for (std::size_t i = 0; i < a.node_acquired.used; ++i)
        if (a.node_calls[i].phase != NativeModelStorageNodeCall::Phase::complete) return false;
    return a.primary_prefix->settled() && m.release.started && m.release.complete &&
        m.lifetime.node.complete && m.lifetime.node.base_cleanup_completed &&
        a.tree_acquired.current18.phase == C::complete && a.tree_acquired.current0.phase == C::complete &&
        a.tree_acquired.current04_entered && a.tree_acquired.current04_returned;
}
bool GameNativeRawDefaultTextOwner::Impl::retain() const noexcept {
    if (font_link == FontLink::entered || font_link == FontLink::failed ||
        (font_link == FontLink::bound && (!font_producer || font_producer->failure()))) return true;
    if (failure || phase == Phase::preparing || phase == Phase::creating || phase == Phase::retiring ||
        phase == Phase::closing || phase == Phase::failed) return true;
    if (services) {
        if ((services->samplers && services->samplers->requires_process_retention()) ||
            (services->effects && services->effects->requires_process_retention())) return true;
        if (services->attempt) {
            const auto& a = *services->attempt;
            if (a.source_equal && a.source_equal->retain()) return true;
            if (a.empty_unchanged && a.empty_unchanged->retain()) return true;
            if (a.font_main_shader && a.font_main_shader->retain()) return true;
            if (a.font_work) {
                if (a.font_work->retain()) return true;
            } else if (a.font_shader && !a.font_shader->untouched()) return true;
            if (a.primary_prefix && a.primary_prefix->retain()) return true;
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
    if (services && services->mode == Mode::primary_prefix_geometry_font_main_section_empty_unchanged) {
        try { log.notef("raw EMPTY UNCHANGED Text owner: %s phase=%u custody_settled=%d",
            event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
        catch (...) {}
        return;
    }
    if (services && services->mode == Mode::default_source_equal) {
        try { log.notef("raw DEFAULT SOURCE EQUAL Text owner: %s phase=%u custody_settled=%d",
            event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
        catch (...) {}
        return;
    }
    if (services && services->mode == Mode::primary_prefix_geometry_font_main_section) {
        try { log.notef("raw FONT MAIN SECTION Text owner: %s phase=%u custody_settled=%d",
            event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
        catch (...) {}
        return;
    }
    if (services && services->mode == Mode::primary_prefix_geometry_font) {
        const char* const format = services->font_work == FontWork::shader_then_retirement ?
            "raw FONT SHADER Text owner: %s phase=%u custody_settled=%d" :
            services->font_work == FontWork::invalidate_shader ?
            "raw FONT INVALIDATE Text owner: %s phase=%u custody_settled=%d" :
            "raw FONT EQUAL Text owner: %s phase=%u custody_settled=%d";
        try { log.notef(format, event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
        catch (...) {}
        return;
    }
    const bool prefix = services && services->mode == Mode::primary_prefix;
    const bool geometry = services && services->mode == Mode::primary_prefix_geometry;
    if (services && services->mode == Mode::primary_prefix_geometry_main_section) {
        try { log.notef("raw MAIN SECTION Text owner: %s phase=%u custody_settled=%d",
            event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
        catch (...) {}
        return;
    }
    try { log.notef(geometry ? "raw PRIMARY PREFIX GEOMETRY Text owner: %s phase=%u custody_settled=%d" :
            prefix ? "raw PRIMARY PREFIX Text owner: %s phase=%u custody_settled=%d" :
            "raw DEFAULT Text owner: %s phase=%u custody_settled=%d",
        event, static_cast<unsigned>(phase), custody_settled() ? 1 : 0); }
    catch (...) {} // diagnostics never change native ownership/disposition
}

GameNativeRawDefaultTextOwner::GameNativeRawDefaultTextOwner(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameNativeGuiSamplerApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, bindings)) {}
GameNativeRawDefaultTextOwner::~GameNativeRawDefaultTextOwner() noexcept {
    if ((impl_->phase != Impl::Phase::fresh && impl_->phase != Impl::Phase::closed) ||
        (impl_->font_link != Impl::FontLink::none && impl_->font_link != Impl::FontLink::detached)) std::terminate();
}
void GameNativeRawDefaultTextOwner::begin_raw_font_link(const GameNativeFontApplication& producer,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs) {
    auto& p = *impl_;
    try {
        require(p.font_link == Impl::FontLink::none && !p.font_producer && !p.font_resources &&
            !p.font_registry_image && p.phase == Impl::Phase::fresh && !p.services && !p.failure,
            "raw Font link begins once on its SAME fresh stable unprepared owner");
        p.font_producer = &producer;
        p.font_link = Impl::FontLink::entered; // BEFORE domain/producer admission and irreversible export.
        require(&renderer == &p.renderer && &vfs == &p.vfs && producer.ready() && !producer.failure(),
            "raw Font link requires the existing SAME ready producer/renderer/VFS domains");
        require(p.font_link == Impl::FontLink::entered && p.font_producer == &producer &&
            p.phase == Impl::Phase::fresh && !p.failure,
            "caught raw Font link failure cannot resume its entered handshake");
    } catch (...) { p.fail_font_link(producer); throw; }
}
void GameNativeRawDefaultTextOwner::complete_raw_font_link(const GameNativeFontApplication& producer,
    NativeFontResourceOwners& resources, const void* image) {
    auto& p = *impl_;
    try {
        require(p.font_link == Impl::FontLink::entered && p.font_producer == &producer &&
            p.phase == Impl::Phase::fresh && !p.services && !p.failure &&
            !p.font_resources && !p.font_registry_image && producer.ready() && !producer.failure(),
            "raw Font completion requires its SAME unfailed pre-export owner handshake");
        require(image && resources.native_registry_image() == image,
            "raw Font completion requires the producer's SAME actual ready native image collection");
        p.font_resources = &resources;
        p.font_registry_image = image;
        require(p.font_link == Impl::FontLink::entered && p.font_producer == &producer &&
            p.phase == Impl::Phase::fresh && !p.failure && !producer.failure(),
            "caught raw Font completion failure cannot publish a bound borrower");
        p.font_link = Impl::FontLink::bound;
    } catch (...) { p.fail_font_link(producer); throw; }
}
void GameNativeRawDefaultTextOwner::fail_raw_font_link(const GameNativeFontApplication& producer) noexcept {
    impl_->fail_font_link(producer);
}
void GameNativeRawDefaultTextOwner::require_font_retirement(const GameNativeFontApplication& producer,
    const NativeFontResourceOwners& resources, const void* image) const {
    auto& p = *impl_;
    try {
        require(p.font_link == Impl::FontLink::bound && p.font_producer == &producer &&
            p.font_resources == &resources && p.font_registry_image && p.font_registry_image == image &&
            p.phase == Impl::Phase::retired && !p.failure && !p.retain() && p.custody_settled(),
            "Font retirement needs this SAME terminal retired raw owner and all actual custody settled");
        require(producer.ready() && !producer.failure() && resources.native_registry_image() == image,
            "Font retirement must precede native destruction of the SAME ready producer image");
        if (p.services && p.services->attempt) {
            const auto& a = *p.services->attempt;
            if (a.empty_unchanged) {
                require(p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged &&
                    a.empty_unchanged->settled() && a.retirement.empty_unchanged == &a.empty_unchanged->frame,
                    "Font retirement also needs settled EMPTY caller input/Temp/Ensure custody");
                a.empty_unchanged->frame.require_retirement_quiescent();
            }
            if (a.font_main_shader) {
                require((p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section ||
                    p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged) &&
                    a.primary_prefix && a.primary_prefix->font_main_selection && a.primary_prefix->main_section &&
                    a.primary_prefix->font_main_selection->settled() && a.primary_prefix->main_section->settled() &&
                    a.retirement.font_lifetime == &a.primary_prefix->font_main_selection->frame &&
                    a.retirement.font_shader_frame == &a.font_main_shader->frame &&
                    a.reference_context.font_shader_frame == &a.font_main_shader->frame,
                    "Font-before-Main retirement needs BOTH settled slices and SAME reserved shader/reference graph");
            } else if (a.font_work) {
                require(a.font_shader && a.font_work->settled() &&
                    a.retirement.font_lifetime == &a.font_work->frame &&
                    a.retirement.font_shader_frame == &a.font_shader->frame &&
                    a.reference_context.font_shader_frame == &a.font_shader->frame,
                    "Font retirement needs terminal typed helper plus all actual Text/Model/Mesh/header/effect custody");
            } else {
                require(a.font_shader && a.font_shader->untouched() &&
                    a.reference_context.font_shader_frame == &a.font_shader->frame,
                    "Font retirement requires the exact reserved untouched shader/provider graph");
            }
        }
    } catch (...) { p.fail_font_link(producer); throw; }
}
void GameNativeRawDefaultTextOwner::detach_raw_font_link_after_parent_drain(const GameNativeFontApplication& producer,
    const NativeFontResourceOwners& resources, const void* image) {
    auto& p = *impl_;
    try {
        require(p.font_link == Impl::FontLink::bound && p.font_producer == &producer &&
            p.font_resources == &resources && p.font_registry_image && p.font_registry_image == image &&
            p.phase == Impl::Phase::closed && !p.failure && !p.retain() && p.custody_settled(),
            "raw Font detach follows this SAME owner's actual parent-drain closure and settled custody");
        // Producer half proves its RETIRED phase. Its normal bound-retired
        // retention remains true until BOTH layers detach; do not deadlock on it.
        require(!producer.ready() && !producer.failure(),
            "raw Font detach requires the producer's completed native retirement");
        if (p.services && p.services->attempt) {
            const auto& a = *p.services->attempt;
            if (a.empty_unchanged) {
                require(p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged &&
                    a.empty_unchanged->settled() && a.retirement.empty_unchanged == &a.empty_unchanged->frame,
                    "Font detach retains terminal EMPTY input/Temp/Ensure metadata without payload polling");
                a.empty_unchanged->frame.require_retirement_quiescent();
            }
            if (a.font_main_shader) {
                require((p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section ||
                    p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged) &&
                    a.primary_prefix && a.primary_prefix->font_main_selection && a.primary_prefix->main_section &&
                    a.primary_prefix->font_main_selection->settled() && a.primary_prefix->main_section->settled() &&
                    a.retirement.font_lifetime == &a.primary_prefix->font_main_selection->frame &&
                    a.retirement.font_shader_frame == &a.font_main_shader->frame &&
                    a.reference_context.font_shader_frame == &a.font_main_shader->frame,
                    "Font-before-Main detach uses BOTH terminal C++ slices, never retired native Font/header payload");
            } else if (a.font_work) {
                require(a.font_shader && a.font_work->settled() &&
                    a.retirement.font_lifetime == &a.font_work->frame &&
                    a.reference_context.font_shader_frame == &a.font_shader->frame,
                    "raw Font detach keeps SAME terminal helper metadata without following retired native Font data");
            } else {
                require(a.font_shader && a.font_shader->untouched() &&
                    a.reference_context.font_shader_frame == &a.font_shader->frame,
                    "raw Font detach keeps its SAME quiescent prepared metadata until owner reset");
            }
        }
        require(p.font_link == Impl::FontLink::bound && p.phase == Impl::Phase::closed && !p.failure,
            "caught raw Font detach failure cannot be overwritten by completion");
        p.font_link = Impl::FontLink::detached;
        // Stored producer/resources/image are now diagnostic identities only.
        // No retired Font/header/list/texture, image getter or native slot read.
    } catch (...) { p.fail_font_link(producer); throw; }
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
void GameNativeRawDefaultTextOwner::prepare_default_source_equal_metadata_in_ready_renderer(std::size_t capacity) {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::fresh && capacity &&
            static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu &&
            p.font_link == Impl::FontLink::none && !p.font_producer && !p.font_resources && !p.font_registry_image && !p.failure,
            "DEFAULT source-equal metadata needs a fresh unlinked owner and explicit prepared call budget");
        p.phase = Impl::Phase::preparing;
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::default_source_equal);
        p.services->prepare(capacity);
        const auto& a = *p.services->attempt;
        require(p.phase == Impl::Phase::preparing && !p.failure && p.font_link == Impl::FontLink::none &&
            a.source_equal && a.source_equal->unused() && !a.primary_prefix &&
            !a.font_shader && !a.font_main_shader && !a.font_work,
            "DEFAULT source-equal must fix its unused Source/input records before pool entry without optional graphs");
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_metadata_in_ready_renderer(std::size_t capacity) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::fresh && capacity &&
        static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu,
        "raw primary-prefix metadata requires a fresh owner and explicit prepared call budget");
    p.phase = Impl::Phase::preparing;
    try {
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::primary_prefix);
        p.services->prepare(capacity);
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_geometry_metadata_in_ready_renderer(std::size_t capacity) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::fresh && capacity &&
        static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu,
        "raw prefix-geometry metadata requires a fresh owner and explicit prepared call budget");
    p.phase = Impl::Phase::preparing;
    try {
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::primary_prefix_geometry);
        p.services->prepare(capacity);
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_geometry_main_section_metadata_in_ready_renderer(
    std::size_t capacity) {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::fresh && capacity &&
            static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu,
            "raw main-section metadata requires a fresh owner and explicit prepared call budget");
        p.phase = Impl::Phase::preparing;
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::primary_prefix_geometry_main_section);
        p.services->prepare(capacity);
        auto& a = *p.services->attempt;
        require(p.phase == Impl::Phase::preparing && !p.failure && a.primary_prefix &&
            a.primary_prefix->geometry && a.primary_prefix->main_section && !a.font_work &&
            a.primary_prefix->main_section->unused() &&
            a.primary_prefix->main_section->lifetime.phase() ==
                NativeGuiTextMainSectionLifetimeStorageFrame::Phase::prepared,
            "raw main-section metadata must reserve its exact complete Font-empty graph before pools");
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_geometry_font_work_in_ready_renderer(
    std::size_t capacity, FontWork choice) {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::fresh && capacity &&
            static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu &&
            (choice == FontWork::shader_then_retirement || choice == FontWork::invalidate_shader ||
                choice == FontWork::equal_name) &&
            p.font_link == Impl::FontLink::bound && p.font_producer && p.font_resources && p.font_registry_image &&
            p.font_producer->ready() && !p.font_producer->failure() && !p.failure &&
            p.font_resources->native_registry_image() == p.font_registry_image,
            "raw Font work needs one completed genuine Font Host/App link, fresh owner and closed prepared work choice");
        p.phase = Impl::Phase::preparing;
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::primary_prefix_geometry_font, choice);
        p.services->prepare(capacity);
        require(p.phase == Impl::Phase::preparing && !p.failure && p.font_link == Impl::FontLink::bound &&
            !p.font_producer->failure(), "caught Font metadata failure cannot overwrite the owner preparation");
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_geometry_font_main_section_in_ready_renderer(
    std::size_t capacity) {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::fresh && capacity &&
            static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu &&
            p.font_link == Impl::FontLink::bound && p.font_producer && p.font_resources && p.font_registry_image &&
            p.font_producer->ready() && !p.font_producer->failure() && !p.failure &&
            p.font_resources->native_registry_image() == p.font_registry_image,
            "Font-before-Main needs one completed genuine Font Host/App link and a fresh prepared call budget");
        p.phase = Impl::Phase::preparing;
        p.services = std::make_unique<Impl::Services>(p, Impl::Mode::primary_prefix_geometry_font_main_section);
        p.services->prepare(capacity);
        auto& a = *p.services->attempt;
        require(p.phase == Impl::Phase::preparing && !p.failure && p.font_link == Impl::FontLink::bound &&
            !p.font_producer->failure() && a.font_main_shader && !a.font_shader && !a.font_work &&
            a.primary_prefix && a.primary_prefix->geometry && a.primary_prefix->main_section &&
            a.primary_prefix->font_main_selection && a.primary_prefix->main_section->unused() &&
            a.primary_prefix->font_main_selection->unused() &&
            a.primary_prefix->main_section->lifetime.phase() == NativeGuiTextMainSectionLifetimeStorageFrame::Phase::prepared &&
            a.primary_prefix->font_main_selection->frame.phase() == NativeGuiTextFontLifetimeStorageFrame::Phase::prepared,
            "Font-before-Main must reserve BOTH exact independent graphs before any native effects");
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::prepare_primary_prefix_geometry_font_main_section_empty_unchanged_in_ready_renderer(
    std::size_t capacity) {
    // Closed native NodeCall-array demand is ZERO in each array; structural
    // bind and b14 used<count require one retained fresh slot, not spare64.
    constexpr std::size_t minimum_closed_call_capacity = 1;
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::fresh && capacity >= minimum_closed_call_capacity &&
            static_cast<std::uint64_t>(capacity) * sizeof(NativeModelStorageNodeCall) <= 0xffffffffu &&
            p.font_link == Impl::FontLink::bound && p.font_producer && p.font_resources && p.font_registry_image &&
            p.font_producer->ready() && !p.font_producer->failure() && !p.failure &&
            p.font_resources->native_registry_image() == p.font_registry_image,
            "EMPTY Font/Main needs one actual Font link and the explicit closed structural callback capacity");
        p.phase = Impl::Phase::preparing;
        p.services = std::make_unique<Impl::Services>(p,
            Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged);
        p.services->prepare(capacity);
        auto& a = *p.services->attempt;
        require(p.phase == Impl::Phase::preparing && !p.failure && p.font_link == Impl::FontLink::bound &&
            !p.font_producer->failure() && a.font_main_shader && !a.font_shader && !a.font_work && !a.source_equal &&
            a.primary_prefix && a.primary_prefix->geometry && a.primary_prefix->main_section &&
            a.primary_prefix->font_main_selection && a.primary_prefix->main_section->unused() &&
            a.primary_prefix->font_main_selection->unused() && a.empty_unchanged && a.empty_unchanged->prepared &&
            a.empty_unchanged->unused() && a.retirement.empty_unchanged == &a.empty_unchanged->frame &&
            a.primary_prefix->main_section->lifetime.phase() == NativeGuiTextMainSectionLifetimeStorageFrame::Phase::prepared &&
            a.primary_prefix->font_main_selection->frame.phase() == NativeGuiTextFontLifetimeStorageFrame::Phase::prepared,
            "EMPTY Font/Main must fix ALL independent input/Ensure/leaf and original creation records before pools");
        a.empty_unchanged->require_closed_calls();
        p.phase = Impl::Phase::prepared;
        p.observe("metadata_prepared");
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_default_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
        p.services->mode == Impl::Mode::default_lifecycle && !p.services->attempt->primary_prefix,
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
bool GameNativeRawDefaultTextOwner::construct_default_source_equal_in_open_shader_cache() {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
            p.services->mode == Impl::Mode::default_source_equal && p.services->attempt->source_equal &&
            !p.services->attempt->primary_prefix && !p.services->attempt->font_shader &&
            !p.services->attempt->font_main_shader && !p.services->attempt->font_work &&
            p.font_link == Impl::FontLink::none,
            "DEFAULT source-equal construction requires its once-prepared exclusive graph");
        auto& s = *p.services;
        auto& a = *s.attempt;
        p.phase = Impl::Phase::creating;
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention(),
            "DEFAULT source-equal needs the real caller-owned open shader cache and ready facades");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        p.require_source_equal_creating();
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true; // SAME separate raw122 publication after genuine factory93
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.require_source_equal_creating();
        a.source_equal->construct(*a.publication.reference);
        p.require_source_equal_creating();
        require(a.source_equal->settled() && !a.source_equal->retain(),
            "DEFAULT source-equal cannot become bound before private unchanged completion AND actual input return");
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_default_source_equal");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
        p.services->mode == Impl::Mode::primary_prefix && p.services->attempt->primary_prefix,
        "raw primary-prefix construction requires its once-prepared complete graph");
    auto& s = *p.services;
    auto& a = *s.attempt;
    p.phase = Impl::Phase::creating;
    try {
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention(), "raw primary-prefix Text needs the real caller-owned open cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        a.primary_prefix->construct(); // genuine name -> db55 receipt -> genuine name return; no74
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_geometry_in_open_shader_cache() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
        p.services->mode == Impl::Mode::primary_prefix_geometry && p.services->attempt->primary_prefix &&
        p.services->attempt->primary_prefix->geometry,
        "raw prefix-geometry construction requires its once-prepared complete operation graph");
    auto& s = *p.services;
    auto& a = *s.attempt;
    p.phase = Impl::Phase::creating;
    try {
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention(), "raw prefix geometry needs the real caller-owned open cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        a.primary_prefix->construct(); // unchanged real name -> db55 receipt -> real name return
        a.primary_prefix->geometry->construct_geometry(*a.publication.reference); // ONE explicit379 body
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix_geometry");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_geometry_main_section_in_open_shader_cache() {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
            p.services->mode == Impl::Mode::primary_prefix_geometry_main_section &&
            p.services->attempt->primary_prefix && p.services->attempt->primary_prefix->geometry &&
            p.services->attempt->primary_prefix->main_section && !p.services->attempt->font_work,
            "raw main-section construction requires its once-prepared closed operation graph");
        auto& s = *p.services;
        auto& a = *s.attempt;
        p.phase = Impl::Phase::creating;
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention(), "raw main section needs the real caller-owned open cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        p.require_main_section_creating();
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.require_main_section_creating();
        a.primary_prefix->construct(); // unchanged real name/db55/name-return sequence
        p.require_main_section_creating();
        a.primary_prefix->geometry->construct_geometry(*a.publication.reference); // ONE real379, actual closed74 proof
        p.require_main_section_creating();
        a.primary_prefix->main_section->construct(*a.publication.reference, p); // actual AB8530 main arm, then seal
        p.require_main_section_creating();
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix_geometry_main_section");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_geometry_font_work_in_open_shader_cache() {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
            p.services->mode == Impl::Mode::primary_prefix_geometry_font &&
            p.services->attempt->primary_prefix && p.services->attempt->primary_prefix->geometry &&
            p.services->attempt->font_shader && p.services->attempt->font_work,
            "raw Font construction requires its once-prepared complete closed Font/primary/geometry graph");
        auto& s = *p.services;
        auto& a = *s.attempt;
        p.phase = Impl::Phase::creating;
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention() && p.font_link == Impl::FontLink::bound &&
            p.font_producer->ready() && !p.font_producer->failure() &&
            p.font_resources->native_registry_image() == p.font_registry_image,
            "raw Font work uses the same real ready Font producer and caller-owned open shader cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        p.require_font_work_creating();
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.require_font_work_creating();
        a.primary_prefix->construct();
        p.require_font_work_creating();
        a.primary_prefix->geometry->construct_geometry(*a.publication.reference);
        p.require_font_work_creating();
        a.font_work->construct(*a.publication.reference); // seal first; real input/setter/return/shader sequence only
        p.require_font_work_creating();
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix_geometry_font");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_geometry_font_main_section_in_open_shader_cache() {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
            p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section &&
            p.services->attempt->primary_prefix && p.services->attempt->primary_prefix->geometry &&
            p.services->attempt->primary_prefix->main_section && p.services->attempt->primary_prefix->font_main_selection &&
            p.services->attempt->font_main_shader && !p.services->attempt->font_shader && !p.services->attempt->font_work,
            "Font-before-Main construction requires its once-prepared exclusive graph");
        auto& s = *p.services;
        auto& a = *s.attempt;
        p.phase = Impl::Phase::creating;
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention() && p.font_link == Impl::FontLink::bound &&
            p.font_producer && p.font_producer->ready() && !p.font_producer->failure() &&
            p.font_resources && p.font_resources->native_registry_image() == p.font_registry_image,
            "Font-before-Main uses the SAME ready actual Font producer and caller-owned open cache");
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        p.require_font_work_creating();
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.require_font_work_creating();
        a.primary_prefix->construct(); // SAME actual name/db55/name-return sequence
        p.require_font_work_creating();
        a.primary_prefix->geometry->construct_geometry(*a.publication.reference); // ONE379, unchanged closed74 guard
        p.require_font_work_creating();
        a.primary_prefix->font_main_selection->construct(*a.publication.reference); // real Font BEFORE Main
        p.require_font_work_creating();
        a.primary_prefix->main_section->construct(*a.publication.reference, p); // real AB8530 Main, then seal
        p.require_font_work_creating();
        a.primary_prefix->font_main_selection->frame.require_live(a.factory.raw_slot,
            NativeGuiTextFontLifetimeStorageFrame::Stage::bound);
        a.primary_prefix->main_section->lifetime.require_live(a.factory.raw_slot,
            NativeGuiTextMainSectionLifetimeStorageFrame::Stage::bound);
        p.require_font_work_creating();
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix_geometry_font_main_section");
        return true;
    } catch (...) { p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::construct_primary_prefix_geometry_font_main_section_empty_unchanged_in_open_shader_cache() {
    auto& p = *impl_;
    try {
        require(p.phase == Impl::Phase::prepared && p.services && p.services->attempt &&
            p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged &&
            p.services->attempt->primary_prefix && p.services->attempt->primary_prefix->geometry &&
            p.services->attempt->primary_prefix->main_section && p.services->attempt->primary_prefix->font_main_selection &&
            p.services->attempt->font_main_shader && !p.services->attempt->font_shader && !p.services->attempt->font_work &&
            p.services->attempt->empty_unchanged && p.services->attempt->empty_unchanged->prepared &&
            p.services->attempt->empty_unchanged->unused() && !p.services->attempt->source_equal,
            "EMPTY Font/Main construction requires its exclusive once-prepared graph");
        auto& s = *p.services;
        auto& a = *s.attempt;
        p.phase = Impl::Phase::creating;
        require(s.shader_cache.actual_cache_0108d6ec && !s.samplers->requires_process_retention() &&
            !s.effects->requires_process_retention() && p.font_link == Impl::FontLink::bound &&
            p.font_producer && p.font_producer->ready() && !p.font_producer->failure() &&
            p.font_resources && p.font_resources->native_registry_image() == p.font_registry_image,
            "EMPTY Font/Main borrows the SAME ready Font producer and caller-owned real open cache");
        a.empty_unchanged->require_closed_calls();
        NativeLuaServiceBindings::Activation activation(s.lua);
        void* const result = make_native_gui_text_default_factory_storage_00aa1380(a.factory_context, a.factory);
        p.require_font_work_creating();
        a.empty_unchanged->require_closed_calls();
        if (!result) { p.phase = Impl::Phase::empty; p.observe("native_empty"); return false; }
        a.publication.bind_entered = true;
        try {
            auto& reference = bind_native_gui_text_raw_model_reference(a.companion, result, a.reference_context,
                a.reference_frame, a.factory_context.disposal);
            a.publication.reference = &reference;
            a.publication.bind_complete = true;
        } catch (...) { a.publication.bind_failed = true; throw; }
        p.require_font_work_creating();
        a.primary_prefix->construct(); // same genuine db55/name and normal name return
        p.require_font_work_creating();
        a.primary_prefix->geometry->construct_geometry(*a.publication.reference); // ONE real379 and closed late74 guard
        p.require_font_work_creating();
        a.primary_prefix->font_main_selection->construct(*a.publication.reference); // genuine Font before Main
        p.require_font_work_creating();
        a.primary_prefix->main_section->construct(*a.publication.reference, p); // ORIGINAL created Main authority
        p.require_font_work_creating();
        a.primary_prefix->font_main_selection->frame.require_live(a.factory.raw_slot,
            NativeGuiTextFontLifetimeStorageFrame::Stage::bound);
        a.primary_prefix->main_section->lifetime.require_live(a.factory.raw_slot,
            NativeGuiTextMainSectionLifetimeStorageFrame::Stage::bound);
        p.require_font_work_creating();
        a.empty_unchanged->construct(*a.publication.reference); // distinct Ensure/copy/EMPTY; real caller-input return
        p.require_font_work_creating();
        require(a.empty_unchanged->settled() && !a.empty_unchanged->retain(),
            "EMPTY caller cannot become bound with failed/pending input/Ensure/Temp debt");
        a.empty_unchanged->frame.require_retirement_quiescent();
        a.empty_unchanged->require_closed_calls();
        p.require_font_work_creating();
        p.phase = Impl::Phase::bound;
        p.observe("constructed_and_published_private_prefix_geometry_font_main_section_empty_unchanged");
        return true;
    } catch (...) { p.fail(); throw; }
}
void GameNativeRawDefaultTextOwner::retire_before_native_drain() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::retired || p.phase == Impl::Phase::closed) return;
    if (p.services && p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged) {
        try {
            require(!p.failure && !p.retain() &&
                (p.phase == Impl::Phase::fresh || p.phase == Impl::Phase::prepared ||
                    p.phase == Impl::Phase::empty || p.phase == Impl::Phase::bound),
                "EMPTY recursive/failed retirement cannot be caught and resumed by a provider");
        } catch (...) { p.fail(); throw; }
    }
    if (p.phase == Impl::Phase::fresh || p.phase == Impl::Phase::prepared || p.phase == Impl::Phase::empty) {
        require(!p.retain(), "incomplete DEFAULT metadata cannot retire");
        if (p.services && p.services->attempt && p.services->attempt->empty_unchanged) {
            p.services->attempt->empty_unchanged->require_closed_calls();
            p.services->attempt->empty_unchanged->frame.require_retirement_quiescent();
        }
        p.phase = Impl::Phase::retired;
        return;
    }
    require(p.phase == Impl::Phase::bound && !p.retain(), "raw DEFAULT Text retirement rejects active/failed/replayed entry");
    auto& s = *p.services;
    auto& a = *s.attempt;
    p.phase = Impl::Phase::retiring;
    try {
        if (a.empty_unchanged) {
            a.empty_unchanged->require_closed_calls();
            a.empty_unchanged->frame.require_retirement_quiescent();
        }
        NativeLuaServiceBindings::Activation activation(s.lua);
        // The prefix guard retains ordinary predicate/Model checks and now
        // delegates its Font slice to the exact staged helper when selected.
        if (a.primary_prefix) require_native_gui_text_primary_prefix_before_scalar(a.primary_prefix->context);
        else a.widgets->require_default_before_scalar();
        auto& reference = *a.publication.reference;
        require(reference.reference_count.load(std::memory_order_relaxed) == 1,
            "forced DEFAULT scalar requires this sole unpublished private Text creator"); // OWNER admission, no decrement
        a.publication.scalar_entered = true;
        a.scalar_flags = 1;
        void* const result = reference.delete_current_scalar_00ab8ee0(a.scalar_flags);
        a.publication.scalar_returned = true;
        require(result == a.factory.raw_slot && a.publication.disposed,
            "raw DEFAULT scalar did not return its same identity and dispose the Text companion");
        if (a.empty_unchanged) {
            a.empty_unchanged->require_closed_calls(); // durable CPP records only AFTER both native terminal chains.
            a.empty_unchanged->frame.require_retirement_quiescent();
            require(p.phase == Impl::Phase::retiring && !p.failure && p.font_link == Impl::FontLink::bound &&
                p.font_producer && !p.font_producer->failure(),
                "EMPTY caught scalar/provider rejection cannot be overwritten by retired completion");
        }
        p.phase = Impl::Phase::retired;
        p.observe("text_retired"); // metadata ONLY, including surviving Model/flags0 custody
    } catch (...) { a.publication.scalar_failed = true; p.fail(); throw; }
}
bool GameNativeRawDefaultTextOwner::requires_process_retention() const noexcept { return impl_->retain(); }
void GameNativeRawDefaultTextOwner::after_native_renderer_drain() {
    auto& p = *impl_;
    if (p.phase == Impl::Phase::closed) return;
    if (p.services && p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged) {
        try {
            require(p.phase == Impl::Phase::retired && !p.failure && !p.retain() && p.custody_settled(),
                "EMPTY recursive/premature parent acknowledgement cannot resume after rejection");
        } catch (...) { p.fail(); throw; }
    }
    require(p.phase == Impl::Phase::retired && !p.retain() && p.custody_settled(),
        "raw DEFAULT Text must settle actual custody before parent drain acknowledgement");
    p.phase = Impl::Phase::closing;
    try {
        p.renderer.after_native_drain(); // FIRST, even when raw cache borrows made no facade operation entries
        if (p.services) {
            if (p.services->effects) p.services->effects->after_native_renderer_drain();
            if (p.services->samplers) p.services->samplers->after_native_renderer_drain();
            if (p.services->attempt && p.services->attempt->empty_unchanged) {
                p.services->attempt->empty_unchanged->require_closed_calls();
                p.services->attempt->empty_unchanged->frame.require_retirement_quiescent();
            }
        }
        if (p.services && p.services->mode == Impl::Mode::primary_prefix_geometry_font_main_section_empty_unchanged)
            require(p.phase == Impl::Phase::closing && !p.failure,
                "EMPTY caught parent-drain rejection cannot be overwritten by closure");
        p.phase = Impl::Phase::closed;
        p.observe("closed_after_actual_parent_drain");
    } catch (...) { p.fail(); throw; }
}
} // namespace bsp::game
