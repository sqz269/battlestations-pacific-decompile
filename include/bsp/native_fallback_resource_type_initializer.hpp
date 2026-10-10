#pragma once

#include "bsp/native_mesh_subset_loading.hpp"

#include <cstdint>

namespace bsp {

struct NativeFallbackResourceTypeInitializerContext {
    // Actual process guard, independent of the caller-supplied receiver.
    volatile std::uint8_t& guard_0109020d;

    // Stable actual four-word receiver [own, Scene, root, name]. The observed
    // startup entry supplies 0109021C; this interface does not create backing.
    volatile std::uint32_t* const receiver;

    // Exact numeric original name token 00D631F4. Do not substitute a local
    // string, interpret its contents or dereference it through this interface.
    const std::uint32_t literal_address_00d631f4;

    // Canonical actual three-word Scene storage [own, root, name], identical
    // to scene_types.storage().scene_01090210; never a copied parent-ID view.
    volatile std::uint32_t* const scene_01090210;
    NativeMeshResourceTypeIds& scene_types;
    TypeIdCounterLifetime& counter;
};

// Complete observed B86A00..B86A7C behavior, exposed through a new ordinary
// Source ABI. Original ECX receiver, preserved ESI, RET and native EH/fault
// behavior are not supplied by this C++ interface. The real Scene provider
// represents the observed inline Scene initialization schedule.
//
// Caller contracts: correct distinct guard/receiver/Scene/counter storage,
// stable identities, exact name token, sufficient lifetimes, and the SAME
// retained counter/root domain already used by scene_types. These are binding
// preconditions, not runtime checks. No private storage, validation, rollback,
// production owner, application caller or CRT placement is provided here.
// A nonzero guard performs no stores. A dependency exception preserves the
// sticky guard and every earlier store; retry does not repair partial state.
void initialize_native_fallback_resource_type_00b86a00(
    const NativeFallbackResourceTypeInitializerContext&);

} // namespace bsp
