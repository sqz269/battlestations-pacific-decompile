#pragma once
#include <cstdint>
#include <cstddef>

namespace bsp {
struct NativeNodeStorage;
class NativeStringStorage;
class NativeWeakHandlePool;

// Borrow the application's actual tables and allocator domains. Profiles are
// the relocated counterparts of CEB130/D5DBC4/D5DEBC/D5DE88/D5DE48; derived
// tables have six DWORD entries, and are data, never host C++ vtables. The
// parameter pool is the SAME actual F8D344 pool used by parameter producers;
// return_raw_slot_00924420 is the existing generic 10h-slot implementation.
// No replacement pool, parser, retained owner, or shadow count is created.
struct NativeParticleDefinitionBindings {
    NativeStringStorage& strings;
    NativeWeakHandlePool& parameter_pool_00f8d344;
    const volatile std::uint32_t* reference_profile_00ceb130;
    const volatile std::uint32_t* base_profile_00d5dbc4;
    const volatile std::uint32_t* cone_profile_00d5debc;
    const volatile std::uint32_t* sphere_profile_00d5de88;
    const volatile std::uint32_t* smartarea_profile_00d5de48;
    void* (__cdecl* allocate_00bf681b)(std::size_t);
    void (__cdecl* free_00bf65ac)(void*) noexcept;
    void (__cdecl* free_array_00bf6989)(void*) noexcept;
    void* context;
    // Execute the already captured target on the SAME actual object. Slot00
    // has no stack arguments; parser14 takes the actual input text object.
    void (*member_virtual00)(void*, void* actual_member, std::uint32_t target);
    void (*parser_virtual14)(void*, void* actual_definition,
        std::uint32_t target, void* actual_text);
};

// AFA280 ECX raw definition; stack(actual8h name,word10,word70,flag14), RET10.
// Only flag14's low byte is used. Sparse initialization preserves all other
// allocation bytes, including derived curve slots. These are NEW C++ APIs;
// host C++ unwind follows recovered cleanup, not the original FH3 ABI.
void* construct_native_particle_definition_00afa280(void*, const void* actual_name,
    std::uint32_t word10, std::uint32_t word70, std::uint32_t flag14,
    NativeParticleDefinitionBindings&);
void* construct_native_particle_cone_definition_00b03940(void*, const void*,
    std::uint32_t, std::uint32_t, std::uint32_t, NativeParticleDefinitionBindings&);
void* construct_native_particle_sphere_definition_00b02b90(void*, const void*,
    std::uint32_t, std::uint32_t, std::uint32_t, NativeParticleDefinitionBindings&);
void* construct_native_particle_smartarea_definition_00b01cb0(void*, const void*,
    std::uint32_t, std::uint32_t, std::uint32_t, NativeParticleDefinitionBindings&);
// AF9FB0 native ECX kind header, EDX name header, stack(word10,word70,text),
// RET0C/EAX actual owner. Unknown kind reuses word70 as an object address;
// null allocation still reaches virtual14. Neither case is made successful.
// Constructor failure frees raw allocation; parser failure leaves the owner.
void* create_native_particle_definition_00af9fb0(const void* actual_kind,
    const void* actual_name, std::uint32_t word10, std::uint32_t word70,
    void* actual_text, NativeParticleDefinitionBindings&);

// AFFDF0 actual0Ch parameter payload: free +04 only for type16 at +0A 1 or2,
// then clear current +04 after free; B00090 returns that captured slot through
// the canonical physical pool. Parameter fields have not been given guessed
// C++ owner types. Their scalar/vector interpretation belongs to producers.
void destroy_native_particle_parameter_00affdf0(void*, NativeParticleDefinitionBindings&) noexcept;
void return_native_particle_parameter_00b00090(void*, NativeParticleDefinitionBindings&);
void destroy_native_particle_definition_00afa100(void*, NativeParticleDefinitionBindings&);
void destroy_native_particle_cone_definition_00b039f0(void*, NativeParticleDefinitionBindings&);
void destroy_native_particle_sphere_definition_00b02c40(void*, NativeParticleDefinitionBindings&);
void destroy_native_particle_smartarea_definition_00b01d60(void*, NativeParticleDefinitionBindings&);
void* delete_native_particle_definition_00afa350(void*, std::uint32_t flags, NativeParticleDefinitionBindings&);
void* delete_native_particle_cone_definition_00b03b40(void*, std::uint32_t flags, NativeParticleDefinitionBindings&);
void* delete_native_particle_sphere_definition_00b02fb0(void*, std::uint32_t flags, NativeParticleDefinitionBindings&);
void* delete_native_particle_smartarea_definition_00b01ea0(void*, std::uint32_t flags, NativeParticleDefinitionBindings&);
// Complete AFE0A0..AFE192. Native ECX actual 108h record, stack(model,
// definition,time), RET0C, EAX same record. Added EDX borrows current scalar.
// Sparse raw/SSE stores preserve all untouched allocation bytes and aliases.
void* __fastcall construct_native_particle_record_00afe0a0(void* actual_record,
    const volatile std::uint32_t* one_00d7a24c, NativeNodeStorage* actual_model,
    void* actual_definition, float time);
} // namespace bsp
