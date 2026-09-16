#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
class NativeD3dx9Float32To16Import;

// Actual string publication/gate/manager cells and loaded d3dx9_40 import.
// The constant cells are reread at the native points on every construction.
// Storage uses the concrete native string pool and record reserve; no host
// allocator, destructor, parser, profile or definition binding is introduced.
struct NativeParticleTypeConstructionContext {
    NativeStringRawPoolContext& strings;
    const NativeD3dx9Float32To16Import& half_import;
    const volatile std::uint32_t* one_00d7a24c;
    const volatile std::uint32_t* scalar_00ce3804;
    const volatile std::uint32_t* record_scalar_00ce6650;
    // Original B01150 never initializes this incoming stack DWORD, copied
    // to record+18. Borrow the observed residue explicitly; no default exists.
    std::uint32_t initial_record_stack_word18;
};

// Complete raw particle constructors (distinct from AFA280 emitter types).
// Native ECX owner, stack(name8h,word14,parent18), EAX owner, RET0C.
// Untouched owner bytes stay untouched. Base FH3 states clean descriptor,
// name, then BD30F0 using the genuine raw leaves. C++ unwind composition
// does not implement native FH3/SEH transport or make these binary exports.
void* construct_native_particle_type_base_00b01150(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_sprite_particle_base_00b08830(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_axial_particle_base_00b058e0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_floating_particle_base_00b076f0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_floating_particle_definition_00b00770(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_object_particle_definition_00af89e0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
void* construct_native_tracer_particle_definition_00b0a0b0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeConstructionContext&);
} // namespace bsp
