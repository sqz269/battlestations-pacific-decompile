#pragma once

#include "bsp/native_resource_registry_destroy.hpp"

#include <cstdint>

namespace bsp {

// Fixed source bindings only. Each reference identifies actual current state;
// the publication's value, pool state and invalid-handler effects are not
// copied into this aggregate. These three bindings must remain valid and
// unchanged throughout either call. The pointed-to native state may change.
struct NativeResourceRegistryDeleteBindings {
    void* volatile& actual_publication_00f8d41c;
    ActualNativeStringPoolStorage& strings;
    const SingletonLifetimeCallbacks& invalid_parameters;
};
static_assert(sizeof(NativeResourceRegistryDeleteBindings) == 12);

// Complete distinct30-byte entries B1B660/B1B710. Native ECX=registry,
// stack+4=flags, EAX=captured registry, RET4. Both call the complete destructor
// before reading bit0 of the current LOW BYTE in the actual argument slot.
// If set, they free the captured registry allocation; no current-publication
// dereference or flag snapshot is substituted. Both preserve ESI and return
// the captured pointer bits, including after its allocation has been freed.
// Their profile slots are respectively D5E594 and D5E59C; identical bodies
// remain distinct entries. Profile identities do not establish construction.
//
// New fastcall source interface adds EDX=the fixed bindings above; ECX and
// the flags stack slot keep their raw positions. A fixed fastcall bridge
// reaches the full existing destructor, and a fixed cdecl bridge reaches
// actual singleton_lifetime_free. Those concrete C++/CRT provider contracts,
// actual registry/tree/pool validity and normal-return obligations remain.
// No generic destroy/free callback, null guard, extra cleanup or new EH frame
// is introduced in the raw bodies. Native SEH/original caller ABI, provider
// volatile-register equality, constructor/getter and game validation are
// outside this interface. The returned freed pointer must not be dereferenced.
void* __fastcall delete_native_resource_registry_00b1b660(
    void* registry, const NativeResourceRegistryDeleteBindings* bindings,
    std::uint32_t flags);
void* __fastcall delete_native_resource_registry_00b1b710(
    void* registry, const NativeResourceRegistryDeleteBindings* bindings,
    std::uint32_t flags);

} // namespace bsp
