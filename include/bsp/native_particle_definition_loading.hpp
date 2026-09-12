#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleDefinitionBindings;
struct NativeParticleParameterLoadingBindings;

// Borrow the SAME owner services and incoming native TextBuffer. The shared
// scratch is the relocated F8C2C8 buffer; capacity and reentrancy are native
// application obligations. D7A358 is read as a double AFTER curve conversion.
struct NativeParticleDefinitionLoadingBindings {
    NativeParticleDefinitionBindings& owners;
    NativeParticleParameterLoadingBindings& parameters;
    char* text_scratch_00f8c2c8;
    const volatile double* percentage_scale_00d7a358;
    void* context;
    // Required real AF44C0 implementation: ECX actual four-byte pooled line,
    // stack(output header,index), RET8, EAX output. It constructs a suffix
    // owner using AF4450/AEE2E0; it is not a single-token extraction.
    void* (*suffix_00af44c0)(void*, const void* actual_line,
        void* actual_output, std::int32_t index);
    // Required real B00CE0 particle factory: ECX actual8h kind, EDX actual8h
    // name, stack(parent definition,text), RET8, EAX actual child. Includes
    // the child's current virtual08 parser. Unknown kind reuses the parent;
    // allocation failure still reaches dispatch. No fabricated child/result.
    void* (*particle_factory_00b00ce0)(void*, const void* actual_kind,
        const void* actual_name, void* actual_parent, void* actual_text);
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
