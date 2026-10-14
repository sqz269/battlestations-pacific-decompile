#include "bsp/game_native_gui_sampler_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_hosts_singletons.hpp"
#include "bsp/game_input_runtime.hpp"
#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_renderer_scalars.hpp"
#include "bsp/game_native_material_compiler_owners.hpp"
#include "bsp/native_particle_clock_shutdown.hpp"
#include "bsp/native_procedural_resource_lifetime.hpp"
#include "bsp/native_resource_registry_destroy.hpp"
#include "bsp/native_resource_registry_scalar_delete.hpp"
#include "bsp/native_singleton_destruction.hpp"
#include "bsp/native_shadow_depth_target_owner.hpp"
#include <list>
#include <stdexcept>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
const void* profile(GameNativeReadOnlyData& data, std::uintptr_t address) {
    return data.data_at(address, 8); // Current virtual0/04 numeric words only.
}
} // namespace
struct GameNativeGuiSamplerApplication::Impl final : NativeMaterialDescriptorSamplersCallFactory {
    struct Creator {
        Impl* domain{};
        void* raw{};
        bool registered{};
        std::unique_ptr<NativeProceduralResourceReference> reference;
    };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameSingletonHost& singletons;
    GameNativeGuiSamplerApplicationBindings bindings;
    GameNativeVfsRawServices raw;
    GameNativeMaterialCompilerOwners compiler;
    NativeResourceRegistryLookupContext factories;
    NativeSamplerCacheContext cache;
    NativeSamplerCacheEntryContext entry;
    NativeProceduralResourceLifetimeContext lifetime;
    std::list<Creator> creators; // Canonical companion lifetime metadata, not another lookup registry.
    std::size_t live_scopes{};
    bool entered{}, exported{}, failed{}, closed{};

    struct CallScope final : NativeMaterialDescriptorSamplersCallScope {
        Impl& domain;
        bool counted;
        // Intentionally uninitialized until caller evidence/native writes.
        volatile std::uint32_t word;
        NativeMaterialDescriptorSamplerStackKnowledge knowledge;
        NativeMaterialDescriptorSamplersContext root_context, mode_context;
        CallScope(Impl& d, const GameNativeGuiSamplerCallEvidence& evidence, bool invocation = true)
            : domain(d), counted(invocation), knowledge{false},
              root_context{d.compiler.passes.lifetime, d.singletons.manager_publication_01090aa0(),
                  d.singletons.particle_clock_publication_00f8d420(), d.entry, word,
                  evidence.root_release, evidence.root_release_count, &knowledge},
              mode_context{d.compiler.passes.lifetime, d.singletons.manager_publication_01090aa0(),
                  d.singletons.particle_clock_publication_00f8d420(), d.entry, word,
                  evidence.mode_release, evidence.mode_release_count, &knowledge} {
            if (evidence.root_entry_word) { word = *evidence.root_entry_word; knowledge.known = true; }
            if (counted) ++domain.live_scopes;
        }
        ~CallScope() override { if (counted) --domain.live_scopes; }
        NativeMaterialDescriptorSamplersContext& root() noexcept override { return root_context; }
        NativeMaterialDescriptorSamplersContext& mode() noexcept override { return mode_context; }
    };
    // Same borrowed service domain, never used to execute compiler calls. The
    // production factory always owns fresh state in the actual compiler frame.
    std::unique_ptr<CallScope> domain_view;

    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& files,
        GameSingletonHost& host, GameNativeReadOnlyData& data,
        GameNativeGuiSamplerApplicationBindings inputs)
        : log(l), renderer(r), vfs(files), singletons(host), bindings(inputs),
          raw(vfs.borrow_raw_services()), compiler(renderer.material_compiler_owners()),
          factories{profile(data, 0x00d64470), profile(data, 0x00d644ac),
              profile(data, 0x00d644e8), profile(data, 0x00d644f0), raw.invalid_parameters},
          cache{raw.strings, raw.invalid_parameters, singletons.manager_publication_01090aa0(),
              singletons.resource_registry_publication_00f8d41c(), factories, raw.dates,
              profile(data, 0x00ce7d24), this, &register_creator},
          entry{cache, bindings.actual_platform_0109cf04, bindings.input.platform_load_context(),
              bindings.native_message_preimage, data.data_at(0x00ce3c5c, 8)},
          lifetime{renderer.actual_owners(),
              static_cast<const volatile std::uint32_t*>(profile(data, 0x00d64478)),
              static_cast<const volatile std::uint32_t*>(profile(data, 0x00d644b4))} {
        validate_domain();
        domain_view = std::make_unique<CallScope>(*this, GameNativeGuiSamplerCallEvidence{}, false);
    }
    void validate_domain() {
        (void)renderer.section_layout_services();
        auto current = vfs.borrow_raw_services();
        const auto& deletion = singletons.native_deletion_bindings();
        require(&current.strings == &raw.strings && &current.dates == &raw.dates &&
            &current.name_resolution == &raw.name_resolution &&
            &compiler.passes.lifetime.retained_owners == &renderer.actual_owners() &&
            &compiler.passes.lifetime.strings == &raw.strings &&
            deletion.particle_clock && deletion.particle_clock->actual_resource_owners == &renderer.actual_owners() &&
            &deletion.particle_clock->actual_publication_00f8d420 ==
                &singletons.particle_clock_publication_00f8d420() &&
            deletion.resource_registry && &deletion.resource_registry->strings == &raw.strings &&
            &deletion.resource_registry->actual_publication_00f8d41c == &cache.actual_factory_registry_00f8d41c &&
            &cache.actual_manager_01090aa0 == &singletons.manager_publication_01090aa0() &&
            &cache.actual_factory_registry_00f8d41c == &singletons.resource_registry_publication_00f8d41c() &&
            &entry.messages == &bindings.input.platform_load_context() &&
            entry.messages.input == &bindings.input.cursor_context(),
            "GUI sampler parent requires the same real renderer/VFS/loader retirement domain");
    }
    std::unique_ptr<NativeMaterialDescriptorSamplersCallScope> begin(
        NativeMaterialProgramBuilderStorage& builder, NativeMaterialPassStorage& pass,
        NativeShaderDescriptorStorage& root, NativeShaderDescriptorStorage& mode) override {
        require(!failed && !closed, "GUI sampler parent cannot replay failed or closed compiler work");
        validate_domain();
        entered = true;
        try {
            const auto evidence = bindings.evidence ? bindings.evidence(bindings.evidence_context,
                builder, pass, root, mode) : GameNativeGuiSamplerCallEvidence{};
            auto scope = std::make_unique<CallScope>(*this, evidence);
            log.notef("actual GUI sampler scope builder=%p pass=%p root=%p mode=%p entry=%s shared_word=%p",
                static_cast<void*>(&builder), static_cast<void*>(&pass), static_cast<void*>(&root),
                static_cast<void*>(&mode), scope->knowledge.known ? "captured" : "unknown",
                static_cast<const volatile void*>(&scope->word));
            return scope;
        } catch (...) { failed = true; throw; }
    }
    static void register_creator(void* context, void* resource) {
        auto& d = *static_cast<Impl*>(context);
        d.entered = true;
        try {
            require(!d.failed && !d.closed && !d.renderer.actual_owners().find(resource),
                "GUI sampler creator must have one fresh actual canonical identity");
            d.creators.emplace_back(); // Persistent before companion construction/bind.
            auto& creator = d.creators.back();
            creator.domain = &d; creator.raw = resource;
            creator.reference = std::make_unique<NativeProceduralResourceReference>(resource,
                d.lifetime, NativeProceduralResourceCompanionDisposal{&creator, &retire_creator});
            d.renderer.actual_owners().bind(resource, *creator.reference);
            creator.registered = true;
        } catch (...) { d.failed = true; throw; }
    }
    static void retire_creator(void* context, NativeProceduralResourceReference& reference) noexcept {
        auto& creator = *static_cast<Creator*>(context);
        auto& d = *creator.domain;
        if (creator.reference.get() != &reference || !creator.registered) std::terminate();
        d.renderer.actual_owners().unbind(creator.raw, reference);
        for (auto it = d.creators.begin(); it != d.creators.end(); ++it)
            if (&*it == &creator) { d.creators.erase(it); return; }
        std::terminate();
    }
};

GameNativeGuiSamplerApplication::GameNativeGuiSamplerApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs, GameSingletonHost& singletons,
    GameNativeReadOnlyData& data, GameNativeGuiSamplerApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, singletons, data, bindings)) {}
GameNativeGuiSamplerApplication::~GameNativeGuiSamplerApplication() noexcept {
    auto& p = *impl_;
    if (p.failed || p.live_scopes || !p.creators.empty() || ((p.entered || p.exported) && !p.closed)) std::terminate();
}
GameNativeGuiEffectApplicationBindings GameNativeGuiSamplerApplication::effect_bindings() {
    auto& p = *impl_;
    require(!p.failed && !p.closed, "GUI sampler parent has no live production binding");
    p.validate_domain();
    p.exported = true;
    return {p.domain_view->root(), p.domain_view->mode(),
        game_native_renderer_scalar_process().shadow_target_publication_00f8bbf0(), &p};
}
std::size_t GameNativeGuiSamplerApplication::live_call_scopes() const noexcept { return impl_->live_scopes; }
std::size_t GameNativeGuiSamplerApplication::live_procedural_creators() const noexcept { return impl_->creators.size(); }
bool GameNativeGuiSamplerApplication::requires_process_retention() const noexcept {
    return impl_->failed || impl_->live_scopes != 0;
}
void GameNativeGuiSamplerApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    require(!p.failed && !p.closed && !p.live_scopes && p.creators.empty(),
        "GUI sampler parent has failed/live call state or actual procedural creators");
    require(!p.renderer.publication_00f8d394() && !p.singletons.particle_clock_publication_00f8d420() &&
        !p.singletons.resource_registry_publication_00f8d41c(),
        "GUI sampler parent must survive its actual loader/registry drain");
    try { p.renderer.after_native_drain(); p.closed = true; }
    catch (...) { p.failed = true; throw; }
}
} // namespace bsp::game
