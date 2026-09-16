#pragma once
#include "bsp/native_particle_object_resources_raw.hpp"
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_loading.hpp"
#include "bsp/native_particle_type_property_raw.hpp"
#include <optional>

namespace bsp {
// Borrow the application's SAME actual string publications, shared F8C2C8
// scratch, builder/parameter numeric cells, CRT and runtime pool. Models uses
// that same string domain and the actual VFS/manager/cache/container references.
struct NativeParticleObjectRawContext {
    NativeParticleTypePropertyRawContext& properties;
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleTypeParameterRawContext& parameters;
    NativeParticleObjectResourcesRawContext& models;
    char* actual_text_scratch_00f8c2c8;
    // Explicit nine-word readable view of CURRENT D5DB00 through slot+20.
    // The common-property context promises only +10 and is not extended here.
    const volatile std::uint32_t* actual_object_profile_00d5db00_through20;
};

// One caller-retained invocation. The native headers and builder storage
// precede child frames so referenced storage survives failed child teardown.
// Completed children may be replaced; retain failed children until their
// existing provider obligations resolve. No destructor rollback or replay.
// A failed existing VFS resolution currently has no discharge API and its
// destructor terminates; retain that frame and its contexts for process life.
struct NativeParticleObjectRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleObjectRawAcquired(std::int32_t initial_builder_kind) noexcept;
    NativeParticleObjectRawAcquired(const NativeParticleObjectRawAcquired&) = delete;
    NativeParticleObjectRawAcquired& operator=(const NativeParticleObjectRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    int native_state_at_failure{-1};
    std::uint32_t captured_model_target{};
    // Actual4h pooled headers; native entry initializes only line. Kind+C is
    // explicitly supplied incoming residue, then reused across parser lines.
    std::uint32_t line, name, key_token, property_suffix, model_name;
    std::uint32_t percentage_token, percentage_suffix, curve, curve_suffix;
    std::uint32_t percentage_bits;
    NativeParticleParameterBuilderStorage builder_storage;
    std::optional<NativeParticleTypePropertyRawAcquired> property;
    std::optional<NativeParticleObjectResourcesRawAcquired> model;
};

// Complete AF8BD0..AF9094. Native ECX actual98h Object; stacked actual
// TextBuffer; RET4/AL true on normal EOF/closing brace. Current slot+20
// dispatches AF9660 with a retained child; unknown reached profile/target is
// an explicit boundary. Eight-state cleanup, raw providers, x87 stores and
// partial publication are preserved. No overwritten parameter is returned.
bool load_native_object_particle_definition_00af8bd0(void*, void*,
    NativeParticleObjectRawContext&, NativeParticleObjectRawAcquired&);
// New C++ interface, not original register/FH3/SEH/CRT/fault or gameplay proof.
} // namespace bsp
