#pragma once

#include "bsp/live_effect_manager_lifetime.hpp"
#include <atomic>
#include <cstddef>

namespace bsp {

// Common native 1Ch event prefix, as written by the registered event producers.
// The two borrowed DWORDs retain neutral names here; each concrete producer
// establishes its argument meanings. No default initialization or host vtable.
struct NativeRegisteredEffectPrefixStorage {
    std::uint32_t table_00;
    std::atomic<std::int32_t> references_04;
    std::uint32_t secondary_table_08;
    std::uint8_t active_0c;
    std::byte untouched_0d[3];
    void* borrowed_10;
    void* borrowed_14;
    std::uint32_t type_18;
};
static_assert(sizeof(NativeRegisteredEffectPrefixStorage) == 0x1c);
static_assert(offsetof(NativeRegisteredEffectPrefixStorage, references_04) == 4);
static_assert(offsetof(NativeRegisteredEffectPrefixStorage, borrowed_10) == 0x10);
static_assert(offsetof(NativeRegisteredEffectPrefixStorage, type_18) == 0x18);

// Borrow the SAME application globals/domain used by manager frames and pending
// deletion. F87654 is the deletion/registry lock, distinct from insertion F87650.
struct LiveEffectEventRegistryBindings {
    NativeLiveEffectManagerStorage* volatile& actual_manager_00f8765c;
    EffectManager* volatile& actual_lock_00f87654;
    SingletonLifetimeDomain& domain;
    EffectManagerLifetimeAccess& lock_lifetime;
};

// Complete866A10..866AF5, ECX manager, stack raw event, RET4. Capture actual
//866500 lock; unsigned capacity growth (2*capacity+2) publishes capacity BEFORE
// allocation, copies raw cells without retain, frees current old backing and
// publishes replacement, appends even null/duplicate input, increments count.
// Allocation failure unlocks without rolling capacity back. No event ownership.
void register_live_effect_event_00866a10(NativeLiveEffectManagerStorage&, void* event,
    EffectManager* volatile& actual_lock_00f87654, EffectManagerLifetimeAccess&);

// Complete866B00..866B6E, ECX manager, stack raw event, RET4. Same actual lock;
// replace FIRST matching cell with current last cell then decrement live count.
// No retain/release, tail clear, backing shrink, uniqueness rule or null filter.
void unregister_live_effect_event_00866b00(NativeLiveEffectManagerStorage&, void* event,
    EffectManager* volatile& actual_lock_00f87654, EffectManagerLifetimeAccess&);

} // namespace bsp
