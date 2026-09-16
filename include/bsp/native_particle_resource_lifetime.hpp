#pragma once
#include <cstdint>
namespace bsp {
struct NativeStringRawPoolContext;
struct NativeParticleTypeLifetimeContext;
// Actual resource90h: reference count+4, name header+8, eight inline emitter
// pointers+10/count+30, eight inline Layer pointers+34/count+54. Other bytes
// remain untouched. Known Layer D5DC38 uses the genuine BD30E0 -> AFACE0(flags1)
// source chain with this raw pool. Other child profiles require a callable
// current thiscall terminal slot0; emitter numeric profiles remain boundaries.
// AF4280 native ECX resource, RET. Stamp D5D958, release children FORWARD using
// current signed counts, clear each count, release actual name, stamp CEB130.
// Name/base are genuine unwind actions; second cleanup exceptions terminate.
void destroy_native_particle_resource_00af4280(void*, NativeStringRawPoolContext&);
// AF46E0 native ECX resource, stack flags, EAX original pointer, RET4.
void* delete_native_particle_resource_00af46e0(void*, std::uint32_t flags, NativeStringRawPoolContext&);
// Genuine emitter-aware companions. Borrow the SAME actual F8D344 parameter
// pool as the producers, in addition to raw string cells. Known Base/Cone/
// Sphere/SmartArea emitter and Layer profiles use their exact BD30E0 scalar
// chains, including nested particle type cleanup. Other terminal profiles
// still require callable Win32 tables; Object models retain their canonical
// renderer binding boundary. Same native body/state schedule, new source API.
void destroy_native_particle_resource_00af4280(void*, NativeParticleTypeLifetimeContext&);
void* delete_native_particle_resource_00af46e0(void*, std::uint32_t flags,
    NativeParticleTypeLifetimeContext&);
}
