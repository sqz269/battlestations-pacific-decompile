#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleDefinitionBindings;
struct NativeParticleParameterLoadingBindings;
struct NativeParticleTypeFactoryBindings;
struct NativeStringRawPoolContext;
struct NativeParticleParameterRuntimeRawContext;

// Borrow the SAME owner services and incoming native TextBuffer. The shared
// scratch is the relocated F8C2C8 buffer; capacity and reentrancy are native
// application obligations. D7A358 is read as a double AFTER curve conversion.
struct NativeParticleDefinitionLoadingBindings {
    NativeParticleDefinitionBindings& owners;
    NativeParticleParameterLoadingBindings& parameters;
    char* text_scratch_00f8c2c8;
    const volatile double* percentage_scale_00d7a358;
    // AF44C0 suffix extraction calls its concrete implementation in the same
    // strings domain. Particle entries call concrete B00CE0 with this required
    // binding; its current virtual08 parser remains an application service.
    // Must borrow the SAME owners through particle_types->base.owners.
    // May be null while parsing text that never selects a Particle entry.
    NativeParticleTypeFactoryBindings* particle_types;
};

// Original ECX definition, stack(pooled name,builder,float), RET0C/AL bool.
// BornRatio consumes builder's first value; the seven other base fields own
// actual pooled parameters. Repeated properties overwrite without disposal.
bool load_native_particle_base_parameter_00af9d00(void* actual_definition,
    const void* actual_name, void* actual_builder, float scalar,
    NativeParticleDefinitionLoadingBindings&);

// ECX parent, stack child, RET4. No added refcount or array-capacity check.
// AF9F00 ignores null; AF9F20 dereferences it. AF9F20 reloads the updated
// count and returns the original address word at 50h+count*4, even on aliasing.
void publish_native_particle_emitter_member_00af9f00(void*, void*) noexcept;
void* publish_native_particle_particle_member_00af9f20(void*, void*) noexcept;

// ECX definition, stack C string, RET4. Preserve actual temporary NativeString
// construction/comparison/release ordering. Unknown values leave the field.
void set_native_particle_part_emission_type_00afa370(void*, const char*,
    NativeParticleDefinitionLoadingBindings&);
void set_native_particle_emit_emission_type_00afa4e0(void*, const char*,
    NativeParticleDefinitionLoadingBindings&);
// ECX definition, stack four-byte pooled suffix header, RET4/AL bool.
bool load_native_particle_definition_flag_00afa650(void*, const void*,
    NativeParticleDefinitionLoadingBindings&);

// Complete raw-domain overloads of the same four bodies. Base-parameter input
// is the actual definition, 4h pooled name, 10h builder and original binary32
// scalar. The runtime context borrows the SAME F8D344 pool. The scale pointer
// itself is retained for the invocation; its volatile D7A358 double is loaded
// only AFTER each genuine conversion. BornRatio stores first-value ST0 directly;
// curve multipliers use FLD32/FMUL64/FSTP32 without a C++ float-return spill.
bool load_native_particle_base_parameter_00af9d00(void*, const void*, void*, float,
    NativeParticleParameterRuntimeRawContext&, const volatile double* actual_00d7a358);

// Borrow the SAME raw string-pool/manager/gate cells. Enum input is a nonnull
// C string; flag input is an actual 4h pooled header. Enum stores target +74/+78;
// flag bytes are +15/+1D/+1C. Looping true also writes CURRENT *(owner+10)+66.
// Current pointer captures, length reloads and recovered true-unwind ownership
// are retained. Normal getter exceptions propagate; secondary unwind exceptions
// terminate. No validation, rollback or cleanup for unowned temporaries is added.
// These are C++ interfaces, not native stack-slot/register/FH3 replacements.
void set_native_particle_part_emission_type_00afa370(void*, const char*,
    NativeStringRawPoolContext&);
void set_native_particle_emit_emission_type_00afa4e0(void*, const char*,
    NativeStringRawPoolContext&);
bool load_native_particle_definition_flag_00afa650(void*, const void*,
    NativeStringRawPoolContext&);

// ECX actual derived definition, stack actual TextBuffer, RET4, AL=1 on every
// normal completion (including EOF without braces). High EAX is unspecified.
// These NEW C++ interfaces retain native field and text-consumption order;
// they are not FH3/thiscall binary replacements. Nested Emitter uses the
// concrete AF9FB0 factory and owners.parser_virtual14 must route its captured
// current target to these actual parsers or another real application target.
bool load_native_particle_cone_definition_00b03ec0(void*, void*,
    NativeParticleDefinitionLoadingBindings&);
bool load_native_particle_sphere_definition_00b02fd0(void*, void*,
    NativeParticleDefinitionLoadingBindings&);
bool load_native_particle_smartarea_definition_00b02210(void*, void*,
    NativeParticleDefinitionLoadingBindings&);
} // namespace bsp
