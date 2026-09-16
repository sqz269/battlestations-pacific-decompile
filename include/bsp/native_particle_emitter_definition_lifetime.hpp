#pragma once
#include <cstdint>
namespace bsp {
struct NativeParticleTypeLifetimeContext;
// AFA100[378]: ECX actual80h emitter definition, RET. Borrow the SAME raw
// string cells and F8D344 parameter pool used by its producers. Release and
// clear seven parameter slots, then retained rows+3C/+54 in forward current
// signed count order. Retained row pointers/counts and name fields stay stale.
// Known particle-type/Layer/base-emitter numeric profiles dispatch through
// their proven BD30E0 scalar chain with flags1, without a second decrement.
// Other profiles require actual callable Win32 slot0; Object model owners
// retain their existing callable-vtable boundary. New source API, not FH3 ABI.
void destroy_native_particle_definition_00afa100(void*, NativeParticleTypeLifetimeContext&);
// AFA350[30]: ECX owner, stack flags; RET4/EAX same owner, free iff flags&1.
void* delete_native_particle_definition_00afa350(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
}
