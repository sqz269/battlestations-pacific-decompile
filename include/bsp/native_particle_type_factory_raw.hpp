#pragma once
#include "bsp/native_particle_axial_raw.hpp"
#include "bsp/native_particle_object_raw.hpp"
#include "bsp/native_particle_sprite_floating_raw.hpp"
#include "bsp/native_particle_tracer_raw.hpp"
#include <cstdint>
#include <optional>

namespace bsp {
struct NativeParticleTypeConstructionContext;

// Borrow the SAME actual string publications, runtime pools, scratch and
// current profile cells across these genuine constructor/parser domains.
// The particle profile cells are borrowed through sprite_floating.properties.
struct NativeParticleTypeFactoryRawContext {
    NativeParticleTypeConstructionContext& construction;
    NativeParticleSpriteFloatingRawContext& sprite_floating;
    NativeParticleAxialRawContext& axial;
    NativeParticleObjectRawContext& object;
    NativeParticleTracerRawContext& tracer;
    // Actual emitter tables through +08. Unknown kind reuses the parent;
    // these slots create records (RET8), they are NOT the +14 parsers.
    const volatile std::uint32_t* smartarea_profile_00d5de48;
    const volatile std::uint32_t* sphere_profile_00d5de88;
    const volatile std::uint32_t* cone_profile_00d5debc;
};

// One caller-retained invocation. Parser children retain their own native
// headers and provider storage. Never reset/replay a failed frame, free its
// owner, or destroy a failed child before its obligations are resolved.
// The frame destructor performs no native rollback. Axial's incoming builder
// residue is explicit in its context; the other parsers use this frame's word.
// In particular, an Object child's failed VFS invocation has no discharge API:
// its existing destructor terminates, so retain that complete graph for life.
struct NativeParticleTypeFactoryRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleTypeFactoryRawAcquired(std::int32_t incoming_builder_kind) noexcept
        : initial_builder_kind(incoming_builder_kind) {}
    NativeParticleTypeFactoryRawAcquired(const NativeParticleTypeFactoryRawAcquired&) = delete;
    NativeParticleTypeFactoryRawAcquired& operator=(const NativeParticleTypeFactoryRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    std::int32_t initial_builder_kind;
    void* allocation{}; // Cleared only after constructor-failure raw free.
    void* owner{};      // Published constructed/reused owner; parser failure keeps it.
    void* actual_text{}; // Borrowed identity of the one native stacked argument.
    std::uint32_t captured_profile{}, captured_target{};
    std::optional<NativeParticleSpriteFloatingRawAcquired> sprite_floating;
    std::optional<NativeParticleAxialRawAcquired> axial;
    std::optional<NativeParticleObjectRawAcquired> object;
    std::optional<NativeParticleTracerRawAcquired> tracer;
};

// Complete B00CE0 control flow with five genuine raw particle parsers.
// Native ECX kind8h, EDX name8h, stack(parent,text), RET8/EAX child. Allocation
// precedes CURRENT parent+10; constructor cleanup precedes raw allocation free.
// Parser failures retain owner/child, and null allocation still dereferences
// null. Unknown kind captures the reused parent's CURRENT profile/+08 target;
// emitter record creation or another unsupported target is an explicit source
// boundary, without callback fallback. New C++ ABI, not native FH3/SEH/fault,
// arbitrary dispatch, full unknown-kind execution or gameplay proof.
void* create_native_particle_type_definition_00b00ce0(const void* actual_kind,
    const void* actual_name, void* actual_parent, void* actual_text,
    NativeParticleTypeFactoryRawContext&, NativeParticleTypeFactoryRawAcquired&);
} // namespace bsp
