#pragma once
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeParticleParameterBuilderRawContext;
struct NativeParticleEmitterFactoryRawContext;
struct NativeParticleEmitterFactoryRawAcquired;

// Borrow the SAME actual string/pool publications, scratch, and native CRT
// feature cell as the recursive raw emitter/type parser family. The feature
// cell is read at each MaxEmitters conversion; FrameRate uses CVTTSS2SI.
struct NativeParticleResourceParserRawContext {
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleEmitterFactoryRawContext& emitters;
    char* actual_text_scratch_00f8c2c8;
    const volatile std::uint32_t* actual_feature_word_0109eea4;
    std::int32_t child_builder_kind;
};

// One retained invocation, with actual native locals before its typed child.
// Incoming builder kind represents the native constructor's unwritten +Ch.
// No replay or destructor rollback. Preserve failed children and all contexts
// until their provider obligations resolve (failed VFS currently needs process
// lifetime). Completed children may be replaced on the next Emitter line.
struct NativeParticleResourceParserRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleResourceParserRawAcquired(std::int32_t incoming_builder_kind);
    ~NativeParticleResourceParserRawAcquired();
    NativeParticleResourceParserRawAcquired(const NativeParticleResourceParserRawAcquired&) = delete;
    NativeParticleResourceParserRawAcquired& operator=(const NativeParticleResourceParserRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    NativeParticleEmitterFactoryRawAcquired* emitter_child() noexcept;
private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend bool parse_native_particle_resource_00af4ba0(void*, void*,
        NativeParticleResourceParserRawContext&, NativeParticleResourceParserRawAcquired&);
};

// Complete AF4BA0[2574], ECX actual90h resource, stack actual1Ch TextBuffer,
// RET4/AL. Rewinds, consumes ParticleSystem, then bounds and preparation.
// Genuine raw dependencies, partial publication and native cleanup ownership.
// New source interface; not original register/FH3/SEH ABI or gameplay proof.
bool parse_native_particle_resource_00af4ba0(void* actual_resource,
    void* actual_text_buffer, NativeParticleResourceParserRawContext&,
    NativeParticleResourceParserRawAcquired&);
} // namespace bsp
