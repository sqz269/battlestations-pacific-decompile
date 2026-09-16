#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTypeLifetimeContext;

// Complete original ECX-owner/RET bodies, with genuine raw pool composition.
// Cone94h: D5DEBC, release +80/+84/+88/+8C/+90 without clearing members.
// Sphere8Ch: D5DE88, release +80/+84/+88 without clearing members.
// SmartArea90h: D5DE48, release +80/+84/+88/+8C; only nonnull +88/+8C are
// cleared, after their captured slot has successfully returned to the pool.
// Every next member is loaded after prior cleanup. First capture precedes EH
// arming; failure unwinds the genuine AFA100 emitter-definition base once.
void destroy_native_particle_cone_definition_00b039f0(void*, NativeParticleTypeLifetimeContext&);
void destroy_native_particle_sphere_definition_00b02c40(void*, NativeParticleTypeLifetimeContext&);
void destroy_native_particle_smartarea_definition_00b01d60(void*, NativeParticleTypeLifetimeContext&);

// Original ECX-owner, stack flags, EAX same owner, RET4. Fixed CRT free follows
// successful full destruction only when flags bit0 is set.
void* delete_native_particle_cone_definition_00b03b40(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_particle_sphere_definition_00b02fb0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
void* delete_native_particle_smartarea_definition_00b01ea0(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);

// These raw overloads coexist with NativeParticleDefinitionBindings APIs.
// Borrow the same actual parameter/string pools; no additional owner/callback.
// A second C++ cleanup exception terminates. Original FH3/SEH/binary thunks,
// asynchronous faults, concurrency and gameplay remain unvalidated.
} // namespace bsp
