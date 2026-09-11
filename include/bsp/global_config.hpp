#pragma once

#include "bsp/native_string.hpp"
#include "bsp/singleton_lifetime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>

namespace bsp {
// Actual2E8h allocation, including unconsumed fields. No value initialization:
//004324E0 writes only its vtable, five vector triples, seven strings and the
//embedded effect slots. Readers/loaders retain this same native storage.
struct alignas(4) GlobalConfigOwner {
    std::array<std::byte, 0x2e8> native;
};
static_assert(sizeof(GlobalConfigOwner) == 0x2e8);

struct GlobalConfigEffects {
    virtual ~GlobalConfigEffects() = default;
    // CURRENT vtable+8, ECX=actual object, one stack argument0. The callback
    //may replace its owner's effect slot;008DBDB0 reloads it before release.
    virtual void stop_slot_08(void* actual_object, std::uint32_t flag) noexcept = 0;
    // CURRENT vtable+0, ECX=actual object, no stack arguments; called only
    //after a real InterlockedDecrement(object+4) reaches zero.
    virtual void zero_references_slot_00(void* actual_object) noexcept = 0;
};
struct GlobalConfigContext {
    // Same01090AA0 lifetime domain as every other startup singleton.
    SingletonLifetimeDomain& lifetime;
    GlobalConfigOwner* volatile& singleton_00f878e4;
    NativeStringStorage& strings;
    GlobalConfigEffects& effects;
};

//008DBCD0/008DBDB0 ECX=actual owner+2C0, RET. Two three-element pointer arrays
//and float18. Destruction performs stop/release in ascending paired order,
//then reverse destructor passes over second array and first array.
void construct_global_config_effects_008dbcd0(void* actual_effect_storage,
    GlobalConfigEffects&) noexcept;
void destroy_global_config_effects_008dbdb0(void* actual_effect_storage,
    GlobalConfigEffects&) noexcept;
//004C3810/00524180 have identical slot destruction: capture pointee, release,
//clear after callback. No clear when the initially captured pointer is null.
void destroy_global_config_pointer_slot_004c3810(void* actual_slot,
    GlobalConfigEffects&) noexcept;
void destroy_global_config_pointer_slot_00524180(void* actual_slot,
    GlobalConfigEffects&) noexcept;
//004312B0 ECX=seven contiguous native string headers, RET. Reverse destruction;
//headers are left unchanged, including after returning pooled storage.
void destroy_global_config_strings_004312b0(void* actual_headers,
    NativeStringStorage&) noexcept;
//00431210 ECX=owner+1C, RET. Four raw vector buffers released last-to-first;
//pointer triples zero after each free; allocator words remain untouched.
void destroy_global_config_vectors_00431210(void* actual_vector_headers) noexcept;
//00432050 ECX=begin, EDX=end, two unused stack arguments, RET8. Captured range
//walks forward over eight-byte native strings; allocation itself is not freed.
void destroy_global_config_name_range_00432050(void* begin, void* end,
    NativeStringStorage&) noexcept;

GlobalConfigOwner& construct_global_config_004324e0(GlobalConfigOwner&,
    GlobalConfigEffects&) noexcept;
//00432650 no arguments, EAX=current singleton, RET. Concrete allocation,
//captured critical-section scope, second manager lookup and registration.
GlobalConfigOwner* get_global_config_00432650(GlobalConfigContext&);
//004325B0 ECX=owner, RET. Full normal destruction; unconditional singleton
//clear and base vtableCE3818 last. No native manager unregister call.
void destroy_global_config_004325b0(GlobalConfigOwner&, GlobalConfigContext&) noexcept;
//CE3D98[0] ->00432710 ECX=owner, stack flags, RET4. Returns original address
//even after flags&1 frees it. Route this from the shared lifetime dispatcher;
//a domain containing other owner types must dispatch their own destructors.
GlobalConfigOwner* scalar_delete_global_config_00432710(GlobalConfigOwner*,
    std::uint32_t flags, GlobalConfigContext&) noexcept;
} // namespace bsp
