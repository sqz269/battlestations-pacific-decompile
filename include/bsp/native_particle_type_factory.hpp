#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTypeBaseBindings;
struct NativeParticleTypeLoadingBindings;

// The original particle-definition family has eleven vtable slots and an 80h
// common prefix. These are the actual application profiles, borrowed without
// replacing their owners, parsers, or lifetime services.
struct NativeParticleTypeFactoryBindings {
    NativeParticleTypeBaseBindings& base;
    const volatile std::uint32_t* sprite_base_profile_00d5dff4;
    const volatile std::uint32_t* sprite_profile_00d5dd18;
    const volatile std::uint32_t* axial_base_profile_00d5df30;
    const volatile std::uint32_t* axial_profile_00d5dcc0;
    const volatile std::uint32_t* floating_base_profile_00d5dfb0;
    const volatile std::uint32_t* floating_profile_00d5dcec;
    const volatile std::uint32_t* object_profile_00d5db00;
    const volatile std::uint32_t* tracer_profile_00d5e048;
    void* context;
    // Real application dispatcher for an unrecognized CURRENT +08 target,
    // or for callers that have not yet supplied concrete loading domains.
    void (*parser_virtual08)(void*, void* actual_child,
        std::uint32_t captured_target, void* actual_text);
    // AQ: when provided, the five reviewed native +08 targets dispatch
    // directly through concrete parsers using these SAME native domains.
    NativeParticleTypeLoadingBindings* loading = nullptr;
};

// Complete derived constructors, original ECX raw owner,
// stack(name8h,word,parent), EAX owner, RET0C. Sparse native stores leave every
// other byte untouched; the common constructor owns string/record cleanup.
void* construct_native_sprite_particle_base_00b08830(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);
void* construct_native_axial_particle_base_00b058e0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);
void* construct_native_floating_particle_base_00b076f0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);
void* construct_native_floating_particle_definition_00b00770(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);
void* construct_native_object_particle_definition_00af89e0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);
void* construct_native_tracer_particle_definition_00b0a0b0(void*, const void*,
    std::uint32_t, void*, NativeParticleTypeFactoryBindings&);

// Complete B00CE0 normal flow and constructor-failure ownership ordering.
// Original fastcall ECX kind8h, EDX name8h, stack(parent,text), RET8/EAX child.
// Each allocation precedes reading current parent+10. Unknown kind reuses
// parent and dispatches its current +08; null allocation still dereferences
// the null child. Parser exceptions do not release the constructed child.
// New C++ API; original FH3 exception ABI and gameplay are not validated.
void* create_native_particle_type_definition_00b00ce0(const void* actual_kind,
    const void* actual_name, void* actual_parent, void* actual_text,
    NativeParticleTypeFactoryBindings&);
} // namespace bsp
