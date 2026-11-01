#include "bsp/game_native_gui_page_icon_services_application.hpp"
#include "bsp/game_native_gui_text_process.hpp"
#include "bsp/game_native_gui_text_state.hpp"
#include "bsp/game_native_mutable_crt_data.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_vfs_runtime.hpp"
#include "bsp/gui_text_native_layout.hpp"
#include "bsp/gui_widgets_cc13.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_gui_icon_constructed_storage.hpp"
#include "bsp/native_gui_icon_delayed_delivery_storage.hpp"
#include "bsp/native_gui_icon_identity.hpp"
#include "bsp/native_gui_icon_loaded_storage.hpp"
#include "bsp/native_gui_icon_property_state_invocations.hpp"
#include "bsp/native_gui_icon_rebuild_storage.hpp"
#include "bsp/native_gui_icon_visibility_storage.hpp"
#include "bsp/native_material_effect_cache.hpp"
#include "bsp/native_material_parameters.hpp"
#include "bsp/native_particle_texture_names_raw.hpp"
#include "bsp/native_platform_focus_owners.hpp"
#include "bsp/native_renderer_texture_find_storage.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_string_raw_domain.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include <exception>
#include <limits>
#include <stdexcept>
#include <utility>

namespace bsp::game {
namespace {
void icon_services_require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
template<class T>
const volatile T& icon_services_ro(const GameNativeReadOnlyData& data, std::uint32_t address) {
    return *static_cast<const volatile T*>(data.data_at(address, sizeof(T)));
}
}

struct GameNativeGuiPageIconServicesApplication::Impl {
    enum class Kind { bind, constructed, loaded, raw80, visibility, class_query };
    enum class Phase { preparing, prepared, entered, normal_returned, failed };

    struct ConstructedPrivate {
        std::uint32_t association_homes[4]; // No seeds: whole74 stages these.
        std::uint32_t position_homes[3];
        NativeGuiWidgetTransformScratch transform;
        GuiWidgetBounds bounds;
        explicit ConstructedPrivate(const GameNativeGuiPageIconPrivateInputs& input)
            : transform(input.initialized_transform), bounds(input.initialized_bounds) {}
    };
    struct ConstructedBacking {
        ConstructedPrivate backing;
        NativeGuiIconConstructedStorageContext context;
        NativeGuiIconConstructedStorageAcquired acquired;
        NativeGuiIconConstructedStorageFrame frame;
        explicit ConstructedBacking(Impl&);
    };
    struct RawPrivate {
        NativeGuiWidgetTransformScratch transform;
        std::unique_ptr<std::uint32_t[]> key;
        explicit RawPrivate(const GameNativeGuiPageIconPrivateInputs& input)
            : transform(input.initialized_transform),
              key(new std::uint32_t[input.initialized_layout_key_word_extent]) {
            // Private initialized input, not a snapshot of an actual descriptor
            // key or Native count. The SAME mutable allocation reaches factory.
            for (std::size_t i = 0; i < input.initialized_layout_key_word_extent; ++i)
                key[i] = input.initialized_layout_key_words[i];
        }
    };
    struct Raw80Backing {
        RawPrivate backing;
        GuiTextNativeRawLayoutInvocation layout;
        NativeGuiIconRebuildStorageContext context;
        NativeGuiIconRebuildStorageFrame frame;
        std::uint32_t state_home; // Actual outgoing HOME; first staged by caller.
        explicit Raw80Backing(Impl&);
    };
    struct LoadedBacking {
        GuiWidgetBounds bounds;
        NativeGuiIconLoadedStorageContext context;
        NativeGuiIconLoadedStorageFrame frame;
        LoadedBacking(Impl&, void* actual, Raw80Backing&);
    };
    struct VisibilityBacking {
        std::uint32_t visible_home;
        NativeGuiIconVisibilityStorageFrame frame;
        explicit VisibilityBacking(std::uint8_t value) : visible_home(value) {}
    };
    struct Call {
        const Kind kind;
        std::unique_ptr<Call> next;
        Call* parent{};
        Phase phase{Phase::preparing};
        NativeGuiIconReference* companion{}; // Historical borrow, no retain.
        void* actual{};
        std::uint32_t descriptor{};
        bool query_result{};
        std::unique_ptr<ConstructedBacking> constructed;
        std::unique_ptr<LoadedBacking> loaded;
        std::unique_ptr<Raw80Backing> raw80;
        std::unique_ptr<VisibilityBacking> visibility;
        std::exception_ptr failure;
        explicit Call(Kind value) : kind(value) {}
    };
    struct ConstructedView final : GameNativeGuiPageIconConstructedInvocation {
        explicit ConstructedView(ConstructedBacking& b)
            : GameNativeGuiPageIconConstructedInvocation(b.frame, b.context, b.acquired) {}
    };
    struct LoadedView final : GameNativeGuiPageIconLoadedInvocation {
        explicit LoadedView(LoadedBacking& b)
            : GameNativeGuiPageIconLoadedInvocation(b.frame, b.context) {}
    };
    struct Raw80View final : NativeGuiIconTextureApplyRebuildInvocation {
        explicit Raw80View(Raw80Backing& b)
            : NativeGuiIconTextureApplyRebuildInvocation(b.frame, b.context, b.state_home) {}
    };

    const GameNativeGuiPageIconServicesInputs inputs;
    std::unique_ptr<Call> first;
    Call* current{};
    bool bind_started{}, bound{};
    NativeGuiWidgetLifetimeContext* lifetime{};
    NativeRenderActualOwnerRegistry* registry{};
    NativeGuiWidgetTransformBindings* transform{};
    NativeGuiWidgetLoadedDispatch* loaded{};
    const GameNativeReadOnlyData* data{};
    const volatile std::uint32_t* feature{};
    const volatile std::int32_t* locale{};
    const volatile std::uint32_t (*lineage)[3]{};
    GuiTextNativeRendererServices* graphics{};
    NativeStringStorage* facade_strings{};
    GuiNativeGeometryOwners* geometry{};
    GuiTextNativeLayoutServices* layouts{};
    NativeTextureCacheContext* cache{};
    std::unique_ptr<NativeGuiIconStorageContext> storage;
    std::unique_ptr<NativeGuiIconPropertyStateServices> states;
    std::unique_ptr<NativeGuiIconTextureApplyStorageContext> apply;
    std::unique_ptr<NativeRendererTextureFindStorageContext> find;
    std::unique_ptr<NativeGuiIconDelayedRequestStorageContext> request;
    std::unique_ptr<NativeGuiIconVisibilityStorageContext> visibility;
    std::unique_ptr<NativeGuiIconDelayedDeliveryStorageContext> delivery;
    std::exception_ptr failure;
    std::uint32_t first_failure_site{};

    explicit Impl(const GameNativeGuiPageIconServicesInputs& supplied) : inputs(supplied) {}

    Call& adopt(Kind kind) {
        auto node = std::make_unique<Call>(kind);
        node->parent = current;
        // Do not publish a catch/current pointer until the owning insertion
        // returns. Read CURRENT head after allocation/new-handler returns;
        // adopting it below allocates nothing and invokes no provider.
        node->next = std::move(first);
        first = std::move(node);
        return *first;
    }
    void fail(Call* call, std::uint32_t site) noexcept {
        const auto escaped = std::current_exception();
        if (call) { call->phase = Phase::failed; call->failure = escaped; }
        if (!failure) { failure = escaped; first_failure_site = site; }
        // An entered current node is retained, not restored into usability.
    }
    static bool failed_frame(const Call& call) noexcept {
        return (call.constructed && call.constructed->acquired.phase == NativeGuiIconConstructedStoragePhase::failed) ||
            (call.loaded && call.loaded->frame.source_phase() == NativeGuiIconSourceCallPhase::failed) ||
            (call.raw80 && (call.raw80->frame.phase == NativeGuiIconRebuildStoragePhase::failed ||
                call.raw80->layout.source_failed())) ||
            (call.visibility && call.visibility->frame.observation().phase ==
                NativeGuiIconVisibilityStorageFrame::Phase::unknown_source_escape);
    }
    void require_bound() const {
        icon_services_require(bound && !failure,
            "Page Icon services require the completed SAME cold bind without a retained Source failure");
        for (const Call* call = first.get(); call; call = call->next.get())
            icon_services_require(call->phase != Phase::failed && !failed_frame(*call),
                "Page Icon services retain an unknown entered child; replay is not admitted");
    }
    void authenticate(Call& call, NativeGuiIconReference& companion) const {
        call.companion = &companion;
        call.actual = companion.actual_identity(); // Source identity only.
        icon_services_require(registry->find(call.actual) == &companion &&
            &companion.lifetime_context() == lifetime && !companion.retired() && !companion.failed(),
            "Page Icon invocation requires its SAME genuine canonical companion/lifetime");
    }
    NativeGuiIconReference& companion_for(void* actual) const {
        // Sole existing registry, no copied registration or alternate owner.
        auto* const companion = dynamic_cast<NativeGuiIconReference*>(registry->find(actual));
        icon_services_require(companion && companion->actual_identity() == actual,
            "Raw Icon current80 requires the SAME actual Page Icon reference");
        return *companion;
    }
    Raw80Backing& prepare_raw80(NativeGuiIconReference* supplied_companion, void* actual) {
        Call* call = nullptr;
        try {
            call = &adopt(Kind::raw80);
            current = call;
            require_bound();
            auto& companion = supplied_companion ? *supplied_companion : companion_for(actual);
            authenticate(*call, companion);
            call->raw80 = std::make_unique<Raw80Backing>(*this);
            call->phase = Phase::prepared;
            current = call->parent;
            return *call->raw80;
        } catch (...) { fail(call, 0x00ab3cb0); throw; }
    }
    static std::unique_ptr<NativeGuiIconTextureApplyRebuildInvocation> prepare_rebuild(
        void* provider, void* actual) {
        auto& p = *static_cast<Impl*>(provider);
        // This thunk is reached cold, before131/140 captures CURRENT80. It
        // samples no receiver/profile/state/import/Model payload.
        auto& backing = p.prepare_raw80(nullptr, actual);
        try { return std::make_unique<Raw80View>(backing); }
        catch (...) { p.fail(nullptr, 0x00ab3cb0); throw; }
    }
    static NativeGuiIconDelayedRequestFileInvocation& prepare_request(void* provider) {
        return static_cast<GameNativeVfsRuntime*>(provider)->prepare_icon_delayed_request_file_call();
    }
    static NativeGuiIconDelayedDeliveryFileInvocation& prepare_delivery(void* provider) {
        return static_cast<GameNativeVfsRuntime*>(provider)->prepare_icon_delayed_delivery_file_call();
    }
    std::uint32_t observed_failure_site() const noexcept {
        if (failure) return first_failure_site;
        for (const Call* call = first.get(); call; call = call->next.get()) {
            if (call->constructed) {
                const auto& a = call->constructed->acquired;
                if (a.phase == NativeGuiIconConstructedStoragePhase::entered ||
                    a.phase == NativeGuiIconConstructedStoragePhase::failed)
                    return a.escaped_call_site ? a.escaped_call_site :
                        (a.active_call_site ? a.active_call_site : 0x00ab2540u);
            }
            if (call->loaded) {
                const auto phase = call->loaded->frame.source_phase();
                if (phase == NativeGuiIconSourceCallPhase::entered || phase == NativeGuiIconSourceCallPhase::failed)
                    return 0x00ab10f0;
            }
            if (call->raw80 && (call->raw80->frame.phase == NativeGuiIconRebuildStoragePhase::entered ||
                call->raw80->frame.phase == NativeGuiIconRebuildStoragePhase::failed || call->raw80->layout.source_failed()))
                return call->raw80->frame.native_site ? call->raw80->frame.native_site : 0x00ab3cb0;
            if (call->visibility) {
                const auto& o = call->visibility->frame.observation();
                if (o.phase == NativeGuiIconVisibilityStorageFrame::Phase::entered ||
                    o.phase == NativeGuiIconVisibilityStorageFrame::Phase::unknown_source_escape)
                    return o.escaped_native_site ? o.escaped_native_site :
                        (o.active_native_site ? o.active_native_site : 0x00ab6120u);
            }
        }
        return bind_started && !bound ? 0x00aa6560u : 0u;
    }
};

GameNativeGuiPageIconServicesApplication::Impl::ConstructedBacking::ConstructedBacking(Impl& p)
    : backing(p.inputs.private_inputs),
      context{*p.registry, *p.geometry, *p.data, p.inputs.decrement_00ce2220,
          icon_services_ro<std::uint32_t>(*p.data, 0x00d7a260), *p.transform,
          icon_services_ro<double>(*p.data, 0x00d7a280)},
      frame(context, backing.association_homes, backing.position_homes, backing.transform, backing.bounds) {}

GameNativeGuiPageIconServicesApplication::Impl::Raw80Backing::Raw80Backing(Impl& p)
    : backing(p.inputs.private_inputs),
      layout(*p.layouts, backing.key.get(), p.inputs.increment_00ce221c, p.inputs.decrement_00ce2220),
      context{*p.registry, *p.geometry, *p.graphics, p.inputs.renderer.publication_00f8d394(),
          p.inputs.strings, *p.feature, p.inputs.effects, p.inputs.material_lifetime,
          p.inputs.material_parameters, p.inputs.clip, *p.transform, layout.context(),
          p.inputs.platform, p.inputs.texture_dimensions, p.inputs.texture_constants,
          icon_services_ro<float>(*p.data, 0x00d7a218), icon_services_ro<float>(*p.data, 0x00d5c3b8),
          icon_services_ro<double>(*p.data, 0x00d7a268), p.inputs.widget_state.icon_vertical_scale_00e12fd0()},
      frame(context, backing.transform, backing.key.get()) {}

GameNativeGuiPageIconServicesApplication::Impl::LoadedBacking::LoadedBacking(
    Impl& p, void* actual, Raw80Backing& raw)
    : bounds(p.inputs.private_inputs.initialized_bounds),
      context(raw.context, *p.loaded, bounds, icon_services_ro<double>(*p.data, 0x00d7a280)),
      frame(actual, context, raw.state_home, raw.frame) {}

GameNativeGuiPageIconServicesApplication::GameNativeGuiPageIconServicesApplication(
    const GameNativeGuiPageIconServicesInputs& inputs) : impl_(std::make_unique<Impl>(inputs)) {}
GameNativeGuiPageIconServicesApplication::~GameNativeGuiPageIconServicesApplication() noexcept = default;

NativeGuiIconStorageContext& GameNativeGuiPageIconServicesApplication::bind_page_storage(
    NativeGuiWidgetLifetimeContext& lifetime, NativeRenderActualOwnerRegistry& registry,
    NativeGuiWidgetTransformBindings& transform, NativeGuiWidgetLoadedDispatch& loaded) {
    auto& p = *impl_;
    Impl::Call* call = nullptr;
    try {
        call = &p.adopt(Impl::Kind::bind);
        p.current = call;
        icon_services_require(!p.bind_started, "Page Icon services cold bind cannot replay");
        p.bind_started = true;
        // Retain the exact Page tuple before any throwing metadata supplier.
        p.lifetime = &lifetime; p.registry = &registry; p.transform = &transform; p.loaded = &loaded;
        const auto& input = p.inputs;
        p.data = &input.canonical_v3.read_only_data();
        auto& crt = input.canonical_v3.mutable_crt_data();
        p.feature = &crt.feature_word();
        p.locale = &crt.locale_changed_0109de1c(); // V3 required; no fallback.
        p.lineage = &input.widget_types.icon_lineage_00f8bdd8();
        auto& grid = input.renderer.game_grid_context();
        p.graphics = &grid.graphics;
        p.facade_strings = &grid.strings;
        p.geometry = &p.graphics->streams.geometry;
        p.layouts = &input.renderer.section_layout_services();
        p.cache = &input.renderer.atlas_process_texture_cache(*p.data);
        const auto& private_input = input.private_inputs;
        icon_services_require(private_input.initialized_layout_key_words &&
            private_input.initialized_layout_key_word_extent >= 5 &&
            private_input.initialized_layout_key_word_extent <= (std::numeric_limits<std::size_t>::max)() / sizeof(std::uint32_t),
            "Page Icon raw layout requires an explicit initialized physically sufficient key extent");
        icon_services_require(&input.renderer.actual_owners() == &registry &&
            &input.gui.actual_resources == &registry && &p.geometry->actual_owners() == &registry &&
            &input.material_lifetime.retained_owners == &registry &&
            &input.material_lifetime.parameter_names == p.facade_strings &&
            &input.material_lifetime.parameter_names == &input.material_parameters.parameter_names &&
            &input.material_lifetime.parameter_slots == &input.material_parameters.parameter_slots &&
            same_native_string_storage_raw_pool_cells(input.material_parameters.parameter_names, input.strings) &&
            same_native_string_storage_raw_pool_cells(p.cache->strings, input.strings) &&
            same_native_string_storage_raw_pool_cells(input.effects.strings, input.strings) &&
            same_native_string_raw_pool_cells(input.atlas_names.strings, input.strings) &&
            same_native_string_raw_pool_cells(input.clip.strings, input.strings) &&
            &input.clip.actual_read_only_data == p.data && &input.clip.materials == &input.material_parameters &&
            input.singleton_deletion.gui_icon_delayed_loads == &input.delayed,
            "Page Icon services require the SAME genuine process/renderer/string/material/deletion graph");
        const auto* const profile = static_cast<const volatile std::uint32_t*>(p.data->data_at(0x00d5c4c0, 0xa8));
        p.storage = std::make_unique<NativeGuiIconStorageContext>(NativeGuiIconStorageContext{
            lifetime, registry, input.strings, input.delayed, input.singleton_deletion, profile});
        const void* const transparent = p.data->data_at(0x00d5c058, 1);
        p.states = std::make_unique<NativeGuiIconPropertyStateServices>(NativeGuiIconPropertyStateServices{
            input.gui, *p.storage, input.atlas_names, *p.cache, input.texture_dimensions,
            input.texture_constants, *p.feature, icon_services_ro<float>(*p.data, 0x00d7a24c),
            input.widget_state, transparent});
        p.apply = std::make_unique<NativeGuiIconTextureApplyStorageContext>(NativeGuiIconTextureApplyStorageContext{
            *p.storage, *p.data, input.increment_00ce221c, input.decrement_00ce2220,
            *p.locale, input.stricmp_callees, input.texture_dimensions, input.texture_constants,
            &p, &Impl::prepare_rebuild});
        p.find = std::make_unique<NativeRendererTextureFindStorageContext>(NativeRendererTextureFindStorageContext{
            *p.cache, input.strings, *p.feature});
        p.request = std::make_unique<NativeGuiIconDelayedRequestStorageContext>(NativeGuiIconDelayedRequestStorageContext{
            input.strings, *p.feature, *p.locale, input.stricmp_callees, &input.runtime, &Impl::prepare_request});
        p.visibility = std::make_unique<NativeGuiIconVisibilityStorageContext>(NativeGuiIconVisibilityStorageContext{
            *p.find, *p.apply, *p.request, input.delayed, private_input.find_guard, transparent});
        p.delivery = std::make_unique<NativeGuiIconDelayedDeliveryStorageContext>(NativeGuiIconDelayedDeliveryStorageContext{
            *p.apply, *p.cache, *p.feature, &input.runtime, &Impl::prepare_delivery});
        p.bound = true;
        call->phase = Impl::Phase::normal_returned;
        p.current = call->parent;
        return *p.storage;
    } catch (...) { p.bind_started = true; p.fail(call, 0x00aa6560); throw; }
}

const NativeGuiIconPropertyStateServices& GameNativeGuiPageIconServicesApplication::property_state_services() {
    impl_->require_bound();
    return *impl_->states;
}
const volatile GameNativeGuiPageIconServices::StatePrivatePreimages&
GameNativeGuiPageIconServicesApplication::property_private_preimages() {
    impl_->require_bound();
    return impl_->inputs.private_inputs.state; // Exact address borrow, no four-word snapshot.
}

std::unique_ptr<GameNativeGuiPageIconConstructedInvocation>
GameNativeGuiPageIconServicesApplication::prepare_constructed_74(NativeGuiIconReference& companion) {
    auto& p = *impl_;
    Impl::Call* call = nullptr;
    try {
        call = &p.adopt(Impl::Kind::constructed);
        p.current = call;
        p.require_bound(); p.authenticate(*call, companion);
        call->constructed = std::make_unique<Impl::ConstructedBacking>(p);
        auto view = std::make_unique<Impl::ConstructedView>(*call->constructed);
        call->phase = Impl::Phase::prepared;
        p.current = call->parent;
        return view;
    } catch (...) { p.fail(call, 0x00ab2540); throw; }
}
std::unique_ptr<GameNativeGuiPageIconLoadedInvocation>
GameNativeGuiPageIconServicesApplication::prepare_loaded_78(NativeGuiIconReference& companion) {
    auto& p = *impl_;
    Impl::Call* call = nullptr;
    try {
        call = &p.adopt(Impl::Kind::loaded);
        p.current = call;
        p.require_bound(); p.authenticate(*call, companion);
        auto& raw = p.prepare_raw80(&companion, nullptr);
        call->loaded = std::make_unique<Impl::LoadedBacking>(p, call->actual, raw);
        auto view = std::make_unique<Impl::LoadedView>(*call->loaded);
        call->phase = Impl::Phase::prepared;
        p.current = call->parent;
        return view;
    } catch (...) { p.fail(call, 0x00ab10f0); throw; }
}
bool GameNativeGuiPageIconServicesApplication::class_query_00aa1cc0(
    NativeGuiIconReference& companion, std::uint32_t descriptor) {
    auto& p = *impl_;
    Impl::Call* call = nullptr;
    try {
        call = &p.adopt(Impl::Kind::class_query);
        p.current = call;
        p.require_bound(); p.authenticate(*call, companion);
        call->descriptor = descriptor;
        call->phase = Impl::Phase::entered;
        call->query_result = gui_widgets_cc13::native_gui_icon_is_kind_of_00aa1cc0(*p.lineage, descriptor);
        call->phase = Impl::Phase::normal_returned;
        p.current = call->parent;
        return call->query_result;
    } catch (...) { p.fail(call, 0x00aa1cc0); throw; }
}
void GameNativeGuiPageIconServicesApplication::visibility_changed_00ab6120(
    NativeGuiIconReference& companion, std::uint8_t visible) {
    auto& p = *impl_;
    Impl::Call* call = nullptr;
    try {
        call = &p.adopt(Impl::Kind::visibility);
        p.current = call;
        p.require_bound(); p.authenticate(*call, companion);
        call->visibility = std::make_unique<Impl::VisibilityBacking>(visible);
        call->phase = Impl::Phase::entered;
        native_gui_icon_visibility_storage_00ab6120(call->actual, call->visibility->visible_home,
            call->visibility->frame, *p.visibility);
        // A swallowed nested Source failure never becomes outer normal history.
        if (p.failure) std::rethrow_exception(p.failure);
        p.require_bound(); // Also observe failed bodies entered through returned views.
        call->phase = Impl::Phase::normal_returned;
        p.current = call->parent;
    } catch (...) { p.fail(call, 0x00ab6120); throw; }
}
NativeGuiIconDelayedDeliveryStorageContext&
GameNativeGuiPageIconServicesApplication::completion_delivery_context() {
    impl_->require_bound();
    return *impl_->delivery;
}
bool GameNativeGuiPageIconServicesApplication::requires_process_retention() const noexcept {
    return impl_->failure || impl_->current || impl_->observed_failure_site() != 0;
}
std::uint32_t GameNativeGuiPageIconServicesApplication::failure_site() const noexcept {
    return impl_->observed_failure_site();
}
} // namespace bsp::game
