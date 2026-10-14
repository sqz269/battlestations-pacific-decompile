#include "bsp/game_native_gui_effect_application.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/game_native_material_compiler_owners.hpp"
#include "bsp/game_native_material_compiler_sources.hpp"
#include "bsp/game_native_material_effect_owners.hpp"
#include "bsp/game_native_shader_process.hpp"
#include "bsp/native_effect_registry_destroy.hpp"
#include "bsp/native_material_effect_runtime.hpp"
#include "bsp/native_material_program_compiler_actual.hpp"
#include "bsp/native_shader_binary_cache.hpp"
#include <cstring>
#include <stdexcept>
#include <vector>

namespace bsp::game {
namespace {
void require(bool condition, const char* message) {
    if (!condition) throw std::logic_error(message);
}
} // namespace
struct GameNativeGuiEffectApplication::Impl {
    enum class Phase { ready, loading, draining, closed, failed };
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameVfsHost& vfs;
    GameNativeGuiEffectApplicationBindings bindings;
    GameNativeVfsRawServices raw;
    NativeTextureCacheContext& textures;
    NativeEffectRegistryDestructionContext& registry;
    GameNativeMaterialCompilerOwners compiler_owners;
    GameNativeMaterialCompilerSources sources;
    GameNativeMaterialEffectOwners effect_owners;
    NativeShaderBinaryCacheContext& shader_cache;
    NativeMaterialProgramCompilerActualContext actual_compiler_context;
    NativeMaterialProgramCompilerActual actual_compiler;
    NativeMaterialProgramCompileContext compile_context;
    NativeMaterialEffectCompilerBinding compiler;
    NativeMaterialEffectNativeChildren children;
    NativeMaterialEffectProgramsContext programs;
    NativeMaterialEffectLoadingContext effects;
    NativeMaterialEffectCacheContext cache;
    std::vector<std::unique_ptr<GameNativeGuiEffectLoadOperation>> operations;
    Phase phase{Phase::ready};
    std::exception_ptr failed;

    Impl(GameHostLog& l, GameNativeRendererApplication& r, GameVfsHost& files,
        GameNativeGuiEffectApplicationBindings inputs)
        : log(l), renderer(r), vfs(files), bindings(inputs), raw(vfs.borrow_raw_services()),
          textures(renderer.texture_cache()), registry(renderer.effect_registry_domain()),
          compiler_owners(renderer.material_compiler_owners()),
          sources(renderer.material_compiler_sources()), effect_owners(renderer.material_effect_owners()),
          shader_cache(renderer.shader_cache_context()),
          actual_compiler_context{raw.strings, shader_cache.load_variants_0108d6f0,
              shader_cache.source_mode_0108d6f1, shader_cache.actual_cache_0108d6ec,
              renderer.publication_00f8d394(), compiler_owners.passes, textures,
              sources.source, sources.vertex, sources.pixel, compiler_owners.reflection,
              compiler_owners.shaders, compiler_owners.registration,
              bindings.root_samplers, bindings.mode_samplers,
              compiler_owners.numeric_renderer.renderer_profile_00d5f0a8,
              shader_cache.actual_physical_profile_00d691b0,
              game_native_shader_process().source_empty_0108d6f2(),
              game_native_shader_process().stream_empty_0109db64()},
          actual_compiler(actual_compiler_context),
          compile_context{raw.strings, shader_cache.load_variants_0108d6f0,
              shader_cache.source_mode_0108d6f1, actual_compiler},
          compiler(effect_owners.lifetime, compile_context),
          children(effect_owners.lifetime, renderer.shader_descriptor_reader(), raw.name_resolution,
              compiler_owners.state_cache, compiler),
          programs{raw.strings, effect_owners.lifetime, compiler_owners.passes,
              compiler_owners.copies, compiler_owners.secondary_registration, children,
              renderer.publication_00f8d394(), bindings.material_manager_00f8bbf0,
              raw.actual_vfs_publication_0109ceec, shader_cache.load_variants_0108d6f0,
              &compiler_owners.numeric_renderer},
          effects{effect_owners.construction, programs, effect_owners.owners,
              raw.actual_vfs_publication_0109ceec,
              compiler_owners.numeric_renderer.renderer_profile_00d5f0a8,
              nullptr, nullptr, textures.textures.empty_string_0108d5a4,
              nullptr, &textures, nullptr},
          cache{registry.actual_records.strings, registry.actual_records.validation,
              effects, raw.dates, textures.platform_0109cf04, textures.synchronization_0108d6dc,
              registry.actual_derived_profile_00d5f074,
              registry.actual_effect_base_profile_00d5e534,
              registry.actual_effect_derived_profile_00d61a00,
              registry.actual_records.allocate_array_00bf55be,
              registry.actual_records.free_array_00bf6989} {
        effects.cache = &cache;
        bind_native_material_effect_vfs_name_resolution(effects, raw.name_resolution);
        validate_domain();
    }
    void validate_domain() {
        (void)renderer.section_layout_services(); // Existing genuine ready gate.
        auto current = vfs.borrow_raw_services();
        require(&current.bindings == &raw.bindings && &current.strings == &raw.strings &&
            &current.name_resolution == &raw.name_resolution && &current.dates == &raw.dates &&
            &current.actual_vfs_publication_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &registry == &renderer.effect_registry_domain() &&
            &registry.actual_owners == &renderer.actual_owners() &&
            &registry.actual_records.strings == &raw.strings && &textures.strings == &raw.strings &&
            &textures.textures.streams == &raw.bindings && &textures.dates == &raw.dates &&
            &effect_owners.construction.strings == &raw.strings &&
            &effect_owners.lifetime.strings == &raw.strings &&
            &effect_owners.lifetime.retained_owners == &renderer.actual_owners() &&
            &effect_owners.owners.actual_owners() == &renderer.actual_owners() &&
            &compiler_owners.passes.lifetime.strings == &raw.strings &&
            &compiler_owners.passes.lifetime.retained_owners == &renderer.actual_owners() &&
            &compiler_owners.numeric_renderer.construction == &compiler_owners.passes &&
            &compiler_owners.numeric_renderer.textures == &textures &&
            &bindings.root_samplers != &bindings.mode_samplers &&
            &bindings.root_samplers.pass_lifetime == &compiler_owners.passes.lifetime &&
            &bindings.mode_samplers.pass_lifetime == &compiler_owners.passes.lifetime &&
            &bindings.root_samplers.loader.cache.strings == &raw.strings &&
            &bindings.mode_samplers.loader.cache.strings == &raw.strings &&
            &bindings.root_samplers.loader.cache.dates == &raw.dates &&
            &bindings.mode_samplers.loader.cache.dates == &raw.dates &&
            &bindings.root_samplers.actual_manager_01090aa0 ==
                &bindings.root_samplers.loader.cache.actual_manager_01090aa0 &&
            &bindings.mode_samplers.actual_manager_01090aa0 ==
                &bindings.mode_samplers.loader.cache.actual_manager_01090aa0 &&
            &bindings.root_samplers.actual_manager_01090aa0 == &bindings.mode_samplers.actual_manager_01090aa0 &&
            &bindings.root_samplers.actual_loader_00f8d420 == &bindings.mode_samplers.actual_loader_00f8d420 &&
            &shader_cache.vfs == &raw.bindings &&
            &shader_cache.actual_vfs_0109ceec == &raw.actual_vfs_publication_0109ceec &&
            &shader_cache.actual_cache_0108d6ec == &game_native_shader_process().cache_0108d6ec() &&
            &shader_cache.load_variants_0108d6f0 == &game_native_shader_process().modes().load_variants_0108d6f0 &&
            &shader_cache.source_mode_0108d6f1 == &game_native_shader_process().modes().source_mode_0108d6f1 &&
            &effect_owners.construction.current_renderer_00f8d394 == &renderer.publication_00f8d394() &&
            &compiler_owners.passes.current_renderer_00f8d394 == &renderer.publication_00f8d394(),
            "GUI effect production requires the same ready actual renderer/VFS/strings/records/compiler/sampler domains");
        require(cache.allocate_array_00bf55be && cache.free_array_00bf6989 &&
            cache.registry_vtable_00d5f074 && cache.registry_vtable_00d5f074[2] == 0x00b2ebb0 &&
            effects.renderer_profile_00d5f0a8 && effects.renderer_profile_00d5f0a8[0x48 / 4] == 0x00b318b0,
            "GUI effect production lacks its genuine record allocation or numeric effect dispatch");
    }
};

GameNativeGuiEffectApplication::GameNativeGuiEffectApplication(GameHostLog& log,
    GameNativeRendererApplication& renderer, GameVfsHost& vfs,
    GameNativeGuiEffectApplicationBindings bindings)
    : impl_(std::make_unique<Impl>(log, renderer, vfs, bindings)) {}
GameNativeGuiEffectApplication::~GameNativeGuiEffectApplication() noexcept {
    if (impl_->phase != Impl::Phase::closed &&
        (impl_->phase != Impl::Phase::ready || !impl_->operations.empty())) std::terminate();
}
void* GameNativeGuiEffectApplication::acquire_effect_00b318b0(const void* name) {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready, "GUI effect producer cannot replay/reenter/retarget or reopen after drain");
    require(name && (reinterpret_cast<std::uintptr_t>(name) & 3u) == 0,
        "GUI effect producer requires a stable initialized actual8h name header");
    p.validate_domain(); // Pure admission checks precede native entry.
    auto frame = std::make_unique<GameNativeGuiEffectLoadOperation>();
    auto& op = *frame;
    op.input_name = name;
    p.operations.push_back(std::move(frame)); // Publish BEFORE capturing/native effects.
    p.phase = Impl::Phase::loading;
    op.phase = GameNativeGuiEffectLoadPhase::running;
    try {
        op.captured_renderer = p.renderer.publication_00f8d394();
        std::uint32_t profile{};
        require(op.captured_renderer != nullptr, "GUI effect producer has no live actual renderer");
        std::memcpy(&profile, op.captured_renderer, sizeof(profile));
        require(profile == 0x00d5f0a8, "GUI effect producer requires its actual D5F0A8 receiver");
        op.result = load_native_renderer_material_effect_00b318b0(
            op.captured_renderer, op.input_name, p.cache, &op.cache);
        require(op.cache.phase == NativeMaterialEffectCacheAcquired::Phase::complete,
            "GUI effect producer returned before its retained cache completed");
        if (op.result) {
            auto* actual = dynamic_cast<NativeMaterialEffectReference*>(
                &p.renderer.actual_owners().resolve_actual(op.result));
            require(actual && &actual->storage() == op.result,
                "GUI effect producer returned a foreign or semantic effect");
            auto& storage = actual->storage();
            require(&actual->reference_count == &storage.references_04,
                "GUI effect result companion does not alias its actual+04");
            op.result_profile = storage.vtable_00;
            op.result_references = storage.references_04.load();
            require(op.result_references > 0 &&
                (op.result_profile == 0x00d5e534 || op.result_profile == 0x00d61a00),
                "GUI effect result has no live supported actual profile");
        }
        op.phase = GameNativeGuiEffectLoadPhase::complete;
        p.phase = Impl::Phase::ready;
        // Log identity only. The return below transfers the held reference; a
        // logging exception still leaves that actual acquisition in this frame.
        p.log.notef("actual GUI effect acquire renderer=%p name=%p result=%p profile=%08x refs=%d cache_site=%08x cache_published=%d caller_acquired=%d",
            op.captured_renderer, op.input_name, op.result, op.result_profile, op.result_references, op.cache.native_site,
            op.cache.cache_published ? 1 : 0, op.cache.caller_acquired ? 1 : 0);
        return op.result;
    } catch (...) {
        op.failure = std::current_exception();
        op.phase = GameNativeGuiEffectLoadPhase::failed;
        p.failed = op.failure;
        p.phase = Impl::Phase::failed;
        throw;
    }
}
std::size_t GameNativeGuiEffectApplication::operation_count() const noexcept {
    return impl_->operations.size();
}
const GameNativeGuiEffectLoadOperation& GameNativeGuiEffectApplication::operation(std::size_t index) const {
    return *impl_->operations.at(index);
}
bool GameNativeGuiEffectApplication::requires_process_retention() const noexcept {
    const auto phase = impl_->phase;
    return phase == Impl::Phase::loading || phase == Impl::Phase::draining || phase == Impl::Phase::failed;
}
const std::exception_ptr& GameNativeGuiEffectApplication::failure() const noexcept {
    return impl_->failed;
}
void GameNativeGuiEffectApplication::after_native_renderer_drain() {
    auto& p = *impl_;
    require(p.phase == Impl::Phase::ready, "failed/active/closed GUI effect graph cannot retire its metadata");
    if (p.operations.empty()) { p.phase = Impl::Phase::closed; return; }
    p.phase = Impl::Phase::draining;
    try {
        require(!p.renderer.publication_00f8d394(), "GUI effect graph must survive the actual renderer cache/effect drain");
        p.renderer.after_native_drain(); // Existing owner/worker/native closure verification.
        for (const auto& op : p.operations)
            require(op->phase == GameNativeGuiEffectLoadPhase::complete,
                "GUI effect graph has an interrupted retained acquisition");
        p.phase = Impl::Phase::closed;
    } catch (...) {
        p.failed = std::current_exception();
        p.phase = Impl::Phase::failed;
        throw;
    }
}
} // namespace bsp::game
