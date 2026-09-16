#pragma once

#include "bsp/game_native_readonly_data.hpp"
#include "bsp/game_native_vfs_runtime.hpp"

#include <array>
#include <cstdint>
#include <memory>

namespace bsp {
class NativeD3dx9Float32To16Import;
class NativeRenderActualOwners;
class NativeResourceContainerReferences;
class NativeWeakHandlePool;
struct CameraAxesCrtAccess;
struct NativeParticleEmitterFactoryRawContext;
struct NativeParticleResourceAcquisitionRawContext;
struct NativeParticleResourceCacheContext;
struct NativeParticleResourceLoaderRawContext;
struct NativeParticleResourceParserRawContext;
struct NativeParticleTypeFactoryRawContext;
struct NativeParticleTypeLifetimeContext;
struct NativeResourceLoadCacheContext;
struct NativeResourceManagerContext;
struct NativeStringRawPoolContext;
struct NativeTextureCacheContext;
struct ResourceLoadEventHost;
}

namespace bsp::game {

// These are the five fixed-address .rdata bands used by the composed particle
// graph. Merge them with every other application's required spans before
// constructing GameNativeReadOnlyData or accepting the bootstrap handoff.
inline constexpr std::array<GameNativeDataSpan, 5>
    game_native_particle_runtime_data_spans{{
        {0x00ce2000u, 1}, {0x00cf0000u, 1}, {0x00d00000u, 1},
        {0x00d50000u, 1}, {0x00d70000u, 1},
    }};

// The recovered constructors deliberately leave these words unwritten. Each
// value is a distinct caller observation; no zero/default substitution exists.
struct GameNativeParticleRuntimeResidues {
    std::int32_t resource_emitter_child_builder_kind;
    std::int32_t sphere_child_builder_kind;
    std::int32_t cone_child_builder_kind;
    std::int32_t smartarea_child_builder_kind;
    std::int32_t axial_builder_kind;
    std::uint32_t type_record_stack_word18;
    std::uint32_t first_texture_record_stack_word18;
    std::uint32_t later_texture_record_stack_word18;
};

// Borrow the application's existing owners and live publications. The VFS,
// resource manager/cache, texture cache, string pool, F8D344 parameter pool,
// renderer companions and platform host must all remain alive through every
// acquired frame and loaded-resource destruction. This input creates no owner,
// manager, allocator, callback family or replacement publication.
struct GameNativeParticleRuntimeInputs {
    GameNativeReadOnlyData& data;
    NativeStringRawPoolContext& strings;
    NativeWeakHandlePool& parameter_pool_00f8d344;
    GameNativeVfsRawServices vfs;
    NativeResourceManagerContext& resource_manager;
    NativeResourceLoadCacheContext& resource_load_cache;
    NativeResourceContainerReferences& resource_references;
    NativeTextureCacheContext& texture_cache;
    NativeRenderActualOwners& render_actual_owners;
    const NativeD3dx9Float32To16Import& half_import;
    const CameraAxesCrtAccess& crt;

    void* volatile& actual_particle_cache_publication_00f87668;
    void* volatile& actual_model_factory_alias_00f8d31c;
    void* volatile& actual_atlas_manager_00f8c26c;
    void* volatile& actual_platform_publication_0109cf04;
    volatile std::uint32_t& actual_feature_word_0109eea4;
    const void* bound_platform_identity;
    ResourceLoadEventHost& load_events;

    char* actual_text_scratch_00f8c2c8;
    const char* empty_particle_name_00f8766c;
    const char* empty_atlas_stem_00f8c2c1;
    const char* empty_model_stem_00f8d320;
    const char* empty_texture_name_00f8d37c;
    const char* empty_tracer_frame_name_00f8d390;
    const char* null_pattern_00e17bf0;
    GameNativeParticleRuntimeResidues residues;
};

// Application-owned storage for the complete raw particle parser/loader graph.
// The object owns context nodes and recursive pointer wiring only. Invocation
// metadata stays caller-owned so failed VFS/provider frames can be retained for
// their existing process-lifetime obligation.
class GameNativeParticleRuntime final {
public:
    explicit GameNativeParticleRuntime(const GameNativeParticleRuntimeInputs&);
    ~GameNativeParticleRuntime();
    GameNativeParticleRuntime(const GameNativeParticleRuntime&) = delete;
    GameNativeParticleRuntime& operator=(const GameNativeParticleRuntime&) = delete;
    GameNativeParticleRuntime(GameNativeParticleRuntime&&) = delete;
    GameNativeParticleRuntime& operator=(GameNativeParticleRuntime&&) = delete;

    NativeParticleResourceLoaderRawContext& loader() noexcept;
    NativeParticleResourceParserRawContext& parser() noexcept;
    NativeParticleEmitterFactoryRawContext& emitter_factory() noexcept;
    NativeParticleTypeFactoryRawContext& type_factory() noexcept;
    NativeParticleTypeLifetimeContext& loaded_resource_lifetime() noexcept;
    NativeParticleResourceCacheContext& resource_cache() noexcept;
    NativeParticleResourceAcquisitionRawContext& acquisition() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};

} // namespace bsp::game
