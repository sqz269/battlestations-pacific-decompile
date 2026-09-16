#pragma once
#include "bsp/native_texture_loading_cache.hpp"

namespace bsp {
struct NativeParticleTextureNamesRawContext;
class NativeD3dx9Float32To16Import;

// Borrow the SAME actual string publications and canonical texture-owner
// domain used by cache.textures. The renderer publication is read from that
// existing cache context at the native capture site. No new pool or owner map.
struct NativeParticleTypeTextureRawContext {
    NativeParticleTextureNamesRawContext& names;
    // Both required on an initial atlas miss; untouched on atlas-only paths.
    NativeTextureCacheContext* cache;
    NativeRenderActualOwners* owners;
    const NativeD3dx9Float32To16Import& half_import;
    const volatile std::uint32_t* renderer_profile_00d5f0a8;
    const volatile std::uint32_t* one_00d7a24c;
    const char* empty_texture_name_00f8d37c;
    // Explicit incoming native stack residues; the producer never writes +18.
    std::uint32_t first_record_stack_word18;
    std::uint32_t later_record_stack_word18;
};

// Caller-retained single invocation. Native local headers and any failed cache
// child must remain alive until their existing obligations are resolved. This
// metadata adds no texture reference and performs no destructor rollback.
struct NativeParticleTypeTextureRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    NativeParticleTypeTextureRawAcquired() = default;
    NativeParticleTypeTextureRawAcquired(const NativeParticleTypeTextureRawAcquired&) = delete;
    NativeParticleTypeTextureRawAcquired& operator=(const NativeParticleTypeTextureRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    alignas(4) unsigned char native_locals_14_5b[0x48];
    void* texture{};
    void* captured_texture{};
    NativeTextureCacheAcquired cache;
};

// Complete B01350..B015BF,624 bytes; ECX definition, stacked filename, RET4/AL.
// Raw strings/atlas/records, actual D3DX import, concrete renderer B319B0/cache
// and canonical +04 decrement/current terminal dispatch. Supported renderer
// profile D5F0A8/+64=B319B0; other profiles are explicit source boundaries.
// Name-only native FH3 states are preserved; texture/record effects do not
// acquire invented exception cleanup. New source interface, not native EH/ABI.
bool load_native_particle_type_texture_00b01350(void* actual_definition,
    const char* filename, NativeParticleTypeTextureRawContext&,
    NativeParticleTypeTextureRawAcquired&);
} // namespace bsp
