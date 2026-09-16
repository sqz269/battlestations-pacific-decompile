#pragma once

#include "bsp/native_particle_emitter_construction.hpp"

#include <cstdint>
#include <memory>

namespace bsp {
struct NativeParticleSphereRawContext;
struct NativeParticleConeRawContext;
struct NativeParticleSmartAreaRawContext;
struct NativeParticleSphereRawAcquired;
struct NativeParticleConeRawAcquired;
struct NativeParticleSmartAreaRawAcquired;

// Borrow actual construction/pool domains and current physical profile cells
// through +14h. Set the concrete parser pointers after constructing the cyclic
// parser/factory contexts. Only the reached parser context must be present.
// Tables contain native target tokens, never host C++ callable addresses.
struct NativeParticleEmitterFactoryRawContext {
    NativeParticleEmitterConstructionContext& construction;
    NativeParticleSphereRawContext* sphere{};
    NativeParticleConeRawContext* cone{};
    NativeParticleSmartAreaRawContext* smartarea{};
    const volatile std::uint32_t* cone_profile_00d5debc{};
    const volatile std::uint32_t* sphere_profile_00d5de88{};
    const volatile std::uint32_t* smartarea_profile_00d5de48{};
};

// One invocation, no replay or destructor rollback. Constructor failure frees
// allocation; parser failure retains owner and the concrete failed parser frame.
// Retain this frame until the child's existing obligations have been resolved.
// The explicit kind supplies the selected parser's unwritten incoming slot.
struct NativeParticleEmitterFactoryRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleEmitterFactoryRawAcquired(std::int32_t incoming_builder_kind);
    ~NativeParticleEmitterFactoryRawAcquired();
    NativeParticleEmitterFactoryRawAcquired(const NativeParticleEmitterFactoryRawAcquired&) = delete;
    NativeParticleEmitterFactoryRawAcquired& operator=(const NativeParticleEmitterFactoryRawAcquired&) = delete;

    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    void* allocation{}; // Captured identity; stale if allocation_released.
    std::uint32_t allocation_bytes{};
    bool allocation_released{};
    void* owner{}; // Borrowed for unknown kind; stale on constructor failure.
    std::uint32_t current_profile{};
    std::uint32_t captured_parser{};

    NativeParticleSphereRawAcquired* sphere_child() noexcept;
    NativeParticleConeRawAcquired* cone_child() noexcept;
    NativeParticleSmartAreaRawAcquired* smartarea_child() noexcept;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
    friend void* create_native_particle_definition_00af9fb0(const void*, const void*,
        std::uint32_t, std::uint32_t, void*, NativeParticleEmitterFactoryRawContext&,
        NativeParticleEmitterFactoryRawAcquired&);
};

// Complete AF9FB0[329B], native ECX kind8h/EDX name8h; stack(word10,word70,
// actual text), EAX owner, RET0C. Unknown kind reuses word70 as the owner.
// Current profile/+14 capture follows constructor cleanup disarm; parser result
// is ignored. Genuine raw constructors/parsers only, with no provider callback.
// New source interface, not original FH3/SEH, unrestricted fault or gameplay proof.
void* create_native_particle_definition_00af9fb0(const void* actual_kind8h,
    const void* actual_name8h, std::uint32_t word10, std::uint32_t word70,
    void* actual_text, NativeParticleEmitterFactoryRawContext&,
    NativeParticleEmitterFactoryRawAcquired&);
} // namespace bsp
