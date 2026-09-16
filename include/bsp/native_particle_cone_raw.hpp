#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeParticleParameterBuilderRawContext;
struct NativeParticleParameterRuntimeRawContext;
struct NativeParticleEmitterFactoryRawContext;
struct NativeParticleTypeFactoryRawContext;
struct NativeParticleEmitterFactoryRawAcquired;
struct NativeParticleTypeFactoryRawAcquired;

// Borrow the application's SAME raw string publications, builder/runtime pools,
// scratch and percentage cell. Wire the typed factory pointers after creating
// the mutually recursive contexts. Only a reached child needs its factory.
struct NativeParticleConeRawContext {
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleParameterRuntimeRawContext& parameters;
    const volatile double* percentage_scale_00d7a358;
    char* actual_text_scratch_00f8c2c8;
    NativeParticleEmitterFactoryRawContext* emitters{};
    NativeParticleTypeFactoryRawContext* particles{};
};

// One retained invocation. Its persistent native locals precede child frames.
// Completed children may be replaced; a failed child and all borrowed contexts
// must survive until its provider obligations resolve. No replay or destructor
// rollback. Existing failed VFS children currently require process lifetime.
struct NativeParticleConeRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleConeRawAcquired(std::int32_t incoming_builder_kind);
    ~NativeParticleConeRawAcquired();
    NativeParticleConeRawAcquired(const NativeParticleConeRawAcquired&) = delete;
    NativeParticleConeRawAcquired& operator=(const NativeParticleConeRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    NativeParticleEmitterFactoryRawAcquired* emitter_child() noexcept;
    NativeParticleTypeFactoryRawAcquired* particle_child() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool load_native_cone_emitter_definition_00b03ec0(void*, void*,
        NativeParticleConeRawContext&, NativeParticleConeRawAcquired&);
};

// Complete B03EC0..B04824. ECX actual94h definition, stacked TextBuffer,
// RET4/AL true on normal EOF or closing brace. Actual raw providers and typed
// recursive factories, 15 native cleanup states, late x87 scale, and partial
// publication. New C++ interface, not native FH3/SEH/CRT or gameplay proof.
bool load_native_cone_emitter_definition_00b03ec0(void*, void*,
    NativeParticleConeRawContext&, NativeParticleConeRawAcquired&);
} // namespace bsp
