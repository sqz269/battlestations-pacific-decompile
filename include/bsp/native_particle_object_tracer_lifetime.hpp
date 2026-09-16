#pragma once
#include <cstdint>

namespace bsp {
struct NativeParticleTypeLifetimeContext;

// Complete AF83B0[80]: ECX actual0Ch {pointer,count,capacity}; stack signed
// count; RET4. Zero new pointer cells; shrinking only decrements current count.
// The genuine AF8350 raw overload uses the fixed CRT allocation/free providers.
void resize_native_object_particle_models_00af83b0(void*, std::int32_t count);
// Complete AF89C0[23]: shrink0/free current pointer, retain stale pointer/capacity.
void destroy_native_object_particle_model_vector_00af89c0(void*);
// Complete869B10[26]: free captured current pointer if nonnull, then clear it.
void destroy_native_particle_tracer_buffer_00869b10(void* actual_pointer_cell) noexcept;

// Complete AF8A40[182]/B0A4E0[555], ECX actual definition, RET. The context
// borrows the SAME raw string publications and actual F8D344 parameter pool.
// Parameter slots are captured individually, disposed/returned, then cleared.
// Object releases models in reverse current-count order before vector cleanup;
// model terminal calls require actual callable Win32 vtables. Numeric image
// model profiles are not bound here: a future binding must use the canonical
// NativeRenderActualOwners/NativeModelOwner domain, not invent a second owner.
// FH3 member/base cleanup is preserved with noexcept true-unwind guards;
// a second exception during unwind terminates. Native hardware SEH/register
// ABI and gameplay equivalence are outside these new C++ interfaces.
void destroy_native_object_particle_type_00af8a40(void*, NativeParticleTypeLifetimeContext&);
void destroy_native_tracer_particle_type_00b0a4e0(void*, NativeParticleTypeLifetimeContext&);
// Complete30B scalars: stack flags, EAX original object, RET4; free iff bit0,
// only after successful destruction. Physical return is the fixed CRT provider.
void* delete_native_object_particle_type_00af8bb0(void*, std::uint32_t flags,
    NativeParticleTypeLifetimeContext&);
void* delete_native_tracer_particle_type_00b0a820(void*, std::uint32_t flags,
    NativeParticleTypeLifetimeContext&);
} // namespace bsp
