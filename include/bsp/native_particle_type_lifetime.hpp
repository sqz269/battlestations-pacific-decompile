#pragma once
#include <cstdint>

namespace bsp {
struct NativeStringRawPoolContext;
class NativeWeakHandlePool;

// Borrow the application's actual raw string publication/gate/manager cells
// and SAME F8D344 parameter pool used by the parameter producers. This owns
// no storage and introduces no allocator or destructor callback.
struct NativeParticleTypeLifetimeContext {
    NativeStringRawPoolContext& strings;
    NativeWeakHandlePool& parameter_pool_00f8d344;
};

// AFFDF0: ECX actual0Ch parameter payload, RET. For current type1/2, free
// captured nonnull +4 then clear CURRENT+4. Other fields/types stay untouched.
void destroy_native_particle_parameter_00affdf0(void*) noexcept;
// B00090: ECX actual slot, RET. Generic924420 return into actual F8D344 pool.
void return_native_particle_parameter_00b00090(void*, NativeWeakHandlePool&);
// B00C20: ECX actual0Ch descriptor, stack signed capacity, RET4. Minimum1,
// forward seven-DWORD copies, current count/source reloads, fixed CRT storage.
void reserve_native_particle_type_records_00b00c20(void*, std::int32_t);
// B00F70: ECX actual descriptor, RET. May reserve1 for negative capacity;
// reduce current count to0, capture data BEFORE the final count0 store, free.
// Leaves data/capacity stale. Used by the base's genuine unwind action.
void destroy_native_particle_type_records_00b00f70(void*);
// B00FB0: ECX actual80h base, RET. Ordered parameter returns, descriptor/name
// cleanup and base identity. Current raw pool getter exceptions propagate;
// secondary exceptions during native-equivalent C++ unwind terminate.
void destroy_native_particle_type_base_00b00fb0(void*, NativeParticleTypeLifetimeContext&);
// B01130: ECX owner, stack flags, RET4/EAX same owner; free iff flags&1.
void* delete_native_particle_type_base_00b01130(void*, std::uint32_t,
    NativeParticleTypeLifetimeContext&);
} // namespace bsp
