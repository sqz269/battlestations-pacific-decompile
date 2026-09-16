#pragma once
#include "bsp/native_particle_type_property_raw.hpp"
#include <cstdint>
#include <optional>

namespace bsp {
struct NativeParticleParameterBuilderRawContext;
struct NativeParticleTypeParameterRawContext;
struct NativeStringRawPoolContext;

struct NativeParticleAxialAxisRawContext {
    const volatile double* angle_scale_00d5daf8;
    const volatile double* axis_base_00ce3830;
    const volatile float* one_00d7a24c;
    const volatile float* negative_zero_00d7a208;
};

// Complete B05D00[200]. Actual A4h definition; native ECX, RET.
// Current numeric cells and existing actual matrix/affine kernels; original
// x87 heading/elevation spills and final +94/+98/+9C stores are preserved.
void refresh_native_axial_particle_axis_00b05d00(void*, NativeParticleAxialAxisRawContext&);

// Complete B062F0[426]. Native ECX definition, stack C string, RET4.
// Actual temporary8h header and same raw pool. The native state stays -1;
// there is no invented failure cleanup. Matched first four names return the
// captured pointer/current length; Right/unknown use current-header DD20.
void set_native_axial_particle_alignment_00b062f0(void*, const char*, NativeStringRawPoolContext&);

struct NativeParticleAxialRawContext {
    NativeParticleParameterBuilderRawContext& builder;
    NativeParticleTypeParameterRawContext& parameters;
    NativeParticleTypePropertyRawContext& properties;
    char* text_scratch_00f8c2c8;
    NativeParticleAxialAxisRawContext& axis;
    std::uint32_t initial_builder_kind_0c;
};

struct NativeParticleAxialRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    NativeParticleAxialRawAcquired() = default;
    NativeParticleAxialRawAcquired(const NativeParticleAxialRawAcquired&) = delete;
    NativeParticleAxialRawAcquired& operator=(const NativeParticleAxialRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    // Actual source-frame local_48 through local_10. These precede the child
    // so every referenced header outlives child destruction. Only line and
    // supplied builder kind are initialized at entry; other slots are sparse.
    std::uint32_t native_locals_48_10[15];
    std::optional<NativeParticleTypePropertyRawAcquired> property;
};

// Complete B064A0[1428]. Original ECX actual A4h definition, stacked actual
// TextBuffer, RET4/AL=true (also EOF without braces). SAME actual string pool,
// runtime pool, publications and shader tables across all borrowed contexts.
// Repeated normal property calls may replace a completed child; a failed
// unresolved child and its enclosing frame must remain alive and never be
// reset/retried. Exact caller unwind states are modeled by a C++ catch; this
// new source interface does not replace the original FH3/register ABI.
bool load_native_axial_particle_definition_00b064a0(void*, void*,
    NativeParticleAxialRawContext&, NativeParticleAxialRawAcquired&);
} // namespace bsp
