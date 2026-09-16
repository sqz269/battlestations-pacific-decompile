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

// Borrow the same actual raw string publications, parameter pool, scalar cell
// and scratch. Wire concrete factories after constructing the cyclic contexts.
// child_builder_kind is the distinct incoming slot of each child parser; it is
// not derived from this parser's current builder or its explicit initial kind.
struct NativeParticleSmartAreaRawContext {
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleParameterRuntimeRawContext& parameters;
    const volatile double* percentage_scale_00d7a358;
    char* actual_text_scratch_00f8c2c8;
    std::int32_t child_builder_kind;
    NativeParticleEmitterFactoryRawContext* emitters{};
    NativeParticleTypeFactoryRawContext* particles{};
};

// One invocation, with native headers preceding retained child frames. Only
// complete children can be replaced. No replay or destructor rollback; retain
// failed children until their existing obligations resolve. Existing failed VFS
// children have no discharge API and currently require process lifetime.
struct NativeParticleSmartAreaRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleSmartAreaRawAcquired(std::int32_t incoming_builder_kind);
    ~NativeParticleSmartAreaRawAcquired();
    NativeParticleSmartAreaRawAcquired(const NativeParticleSmartAreaRawAcquired&) = delete;
    NativeParticleSmartAreaRawAcquired& operator=(const NativeParticleSmartAreaRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    NativeParticleEmitterFactoryRawAcquired* emitter_child() noexcept;
    NativeParticleTypeFactoryRawAcquired* particle_child() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool load_native_smartarea_emitter_definition_00b02210(void*, void*,
        NativeParticleSmartAreaRawContext&, NativeParticleSmartAreaRawAcquired&);
};

// Complete B02210..B02B24, ECX actual90h definition, stacked TextBuffer,
// RET4/AL true on normal EOF or closing brace. Actual raw providers, typed
// recursive factories, 15 cleanup states, current late x87 double scale.
// New C++ interface, not native FH3/SEH, unrestricted fault or gameplay proof.
bool load_native_smartarea_emitter_definition_00b02210(void*, void*,
    NativeParticleSmartAreaRawContext&, NativeParticleSmartAreaRawAcquired&);
} // namespace bsp
