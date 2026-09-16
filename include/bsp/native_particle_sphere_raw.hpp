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
struct NativeParticleSphereRawContext {
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleParameterRuntimeRawContext& parameters;
    const volatile double* percentage_scale_00d7a358;
    char* actual_text_scratch_00f8c2c8;
    // Separate unwritten stack word for a reached nested parser invocation.
    // It is not the current parent builder kind.
    std::int32_t child_builder_kind;
    NativeParticleEmitterFactoryRawContext* emitters{};
    NativeParticleTypeFactoryRawContext* particles{};
};

// One retained invocation. Its persistent native locals precede child frames.
// Completed children may be replaced; a failed child and all borrowed contexts
// must survive until its provider obligations resolve. No replay or destructor
// rollback. Existing failed VFS children currently require process lifetime.
struct NativeParticleSphereRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleSphereRawAcquired(std::int32_t incoming_builder_kind);
    ~NativeParticleSphereRawAcquired();
    NativeParticleSphereRawAcquired(const NativeParticleSphereRawAcquired&) = delete;
    NativeParticleSphereRawAcquired& operator=(const NativeParticleSphereRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    NativeParticleEmitterFactoryRawAcquired* emitter_child() noexcept;
    NativeParticleTypeFactoryRawAcquired* particle_child() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool load_native_sphere_emitter_definition_00b02fd0(void*, void*,
        NativeParticleSphereRawContext&, NativeParticleSphereRawAcquired&);
};

// Complete B02FD0..B03894. ECX actual8Ch definition, stacked TextBuffer,
// RET4/AL true on normal EOF or closing brace. Actual raw providers and typed
// recursive factories, 15 native cleanup states, late x87 scale, and partial
// publication. New C++ interface, not native FH3/SEH/CRT or gameplay proof.
bool load_native_sphere_emitter_definition_00b02fd0(void*, void*,
    NativeParticleSphereRawContext&, NativeParticleSphereRawAcquired&);
} // namespace bsp
