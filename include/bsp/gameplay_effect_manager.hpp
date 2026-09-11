#pragma once

#include "bsp/singleton_lifetime.hpp"

#include <cstdint>
#include <map>
#include <optional>

namespace bsp {
// Native 10h owner: vtable0, untouched allocator4, map head8/countC.
// The standard map is the canonical host projection of that native container.
// Its definition pointers are NON-OWNING: clearing nodes never releases them.
struct GameplayEffectManager {
    std::uint32_t vtable_00;
    std::uint32_t allocator_04;
    std::optional<std::map<std::int32_t, void*>> definitions;
};
struct GameplayEffectManagerAllocationWords {
    std::uint32_t allocator_04;
};
struct GameplayEffectManagerContext {
    // Share the application's actual 01090AA0 domain and 00F87664 publication.
    SingletonLifetimeDomain& lifetime;
    GameplayEffectManager* volatile& singleton_00f87664;
    const GameplayEffectManagerAllocationWords& allocation_words;
};

// 00870370: ECX=fresh 10h native owner; EAX=this; RET. Allocation words supply
// the uninitialized allocator representation; only vtable/head/count are set.
GameplayEffectManager& construct_gameplay_effect_manager_00870370(
    GameplayEffectManager&, const GameplayEffectManagerAllocationWords&);
// 004C1650: no consumed inputs; EAX=singleton; RET. Uses the concrete lifetime
// manager, a captured section, publication, registration and final slot reload.
GameplayEffectManager* get_gameplay_effect_manager_004c1650(
    GameplayEffectManagerContext&);
// 0086FE20: ECX=owner; RET. Clear the weak map, free its head, unconditionally
// clear 00F87664 and store base vtable CE3818. No unregister or payload release.
void destroy_gameplay_effect_manager_0086fe20(GameplayEffectManager&,
    GameplayEffectManagerContext&) noexcept;
// D0DA64[0] ->008703E0: ECX=owner, stack flags; EAX=original address; RET4.
// Flags&1 frees storage. Dispatch this from the shared lifetime domain.
GameplayEffectManager* scalar_delete_gameplay_effect_manager_008703e0(
    GameplayEffectManager*, std::uint32_t flags,
    GameplayEffectManagerContext&) noexcept;
// 0086B0B0: ECX=owner, one unused stack label; RET4. Traverses valid iterators
// without reading payloads or producing output. Native checked-STL invalid
// iterator behavior is outside this standard-container projection.
void probe_gameplay_effect_registry_0086b0b0(const GameplayEffectManager&,
    const char* label);
} // namespace bsp
