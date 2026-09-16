#pragma once
#include "bsp/native_particle_parameter_loading.hpp"
#include "bsp/native_particle_type_property_raw.hpp"
#include "bsp/native_pooled_text.hpp"
#include <optional>

namespace bsp {
struct NativeParticleTextureNamesRawContext;
struct NativeParticleTypeParameterRawContext;

// Borrow the same raw text pool, parameter builder/runtime pool, atlas cell and
// common-property providers used by the actual definition. Numeric cells remain
// live, including percentage_scale through parameters after runtime conversion.
struct NativeParticleTracerRawContext {
    NativeParticleParameterBuilderRawContext& builders;
    NativeParticleTypeParameterRawContext& parameters;
    NativeParticleTypePropertyRawContext& properties;
    NativeParticleTextureNamesRawContext& names;
    char* text_scratch_00f8c2c8;
    const char* empty_frame_name_00f8d390;
};

// Retain this invocation until failed child obligations have been resolved.
// Native four-byte headers precede the child so their addresses stay valid during
// child teardown. Only a completed child may be replaced. No destructor rollback,
// reference increment or replay. Only
// caller-supplied builder kind represents the original unwritten incoming slot;
// constructors and successful parses establish the other builder fields.
struct NativeParticleTracerRawAcquired {
    enum class Phase { fresh, running, complete, failed };
    explicit NativeParticleTracerRawAcquired(std::int32_t incoming_builder_kind) {
        builder.kind_0c = incoming_builder_kind;
    }
    NativeParticleTracerRawAcquired(const NativeParticleTracerRawAcquired&) = delete;
    NativeParticleTracerRawAcquired& operator=(const NativeParticleTracerRawAcquired&) = delete;
    Phase phase{Phase::fresh};
    std::uint32_t native_site{};
    int unwind_state{-1};
    NativePooledTextStorage name, line;
    float scalar;
    NativePooledTextStorage keyword, common_suffix, boolean_value, head, tail,
        texture, scalar_token, scalar_suffix, inner_suffix, outer_suffix;
    NativeParticleParameterBuilderStorage builder;
    std::optional<NativeParticleTypePropertyRawAcquired> property;
};

// Complete B0AD50[2172], ECX actual E8h Tracer, stack actual1Ch text buffer,
// RET4/AL true through closing brace or EOF. Genuine raw helpers only. Original
// x87 stores and late percentage load are retained; existing pointer fields are
// overwritten without release. Child failure metadata remains caller-owned.
// New source interface, not original FH3/SEH, stack alias or gameplay identity.
bool load_native_tracer_particle_definition_00b0ad50(void* actual_definition,
    void* actual_text_buffer, NativeParticleTracerRawContext&,
    NativeParticleTracerRawAcquired&);

// Complete B0A920[619], ECX Tracer, stack filename, RET4. Clears count only;
// appends borrowed atlas items to actual +98/+9C/+A0, growing with wrapped
// unsigned 2*capacity+2 and saturated allocation byte size. No extra owner.
void load_native_tracer_particle_textures_00b0a920(void* actual_definition,
    const char* filename, NativeParticleTextureNamesRawContext&,
    const char* empty_frame_name_00f8d390);
} // namespace bsp
