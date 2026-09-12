#pragma once

#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Actual eight-byte allocation published at 00F8C27C, with primary lifetime
// table 00CECA14. 0051FA50 labels +04 "Don't update particles" and +05
// "Hide particles". Nonzero producers are unresolved; retain neutral names.
// Getter initialization deliberately preserves the two allocation bytes +06/+07.
struct NativeModelEffectOptionsStorage {
    volatile std::uint32_t native_table_00;
    volatile std::uint8_t option_04;
    volatile std::uint8_t option_05;
    std::byte untouched_06[2];
};
static_assert(sizeof(NativeModelEffectOptionsStorage) == 8);
static_assert(offsetof(NativeModelEffectOptionsStorage, option_04) == 4);
static_assert(offsetof(NativeModelEffectOptionsStorage, option_05) == 5);
static_assert(offsetof(NativeModelEffectOptionsStorage, untouched_06) == 6);

// Complete 0051F6B0..0051F765, native cdecl()/RET/EAX current global.
// Share the application's actual 01090AA0 domain. Capture its section, lock,
// recheck, allocate8, initialize table/+04/+05, publish, get manager again,
// THEN reload and register the current primary global. Unlock captured section
// and return current global. Exception cleanup only unlocks; publication survives
// registration failure. The actual CRT allocation service throws on exhaustion.
// Directly supplies RegisteredModelEffectCallees::call_0051f6b0 without an owner
// wrapper, fallback options, additional singleton domain or count changes.
NativeModelEffectOptionsStorage* model_effect_options_singleton_0051f6b0(
    NativeModelEffectOptionsStorage* volatile& actual_global_00f8c27c,
    SingletonLifetimeDomain& actual_domain_01090aa0);

// Complete 0051F770..0051F798, native ECX owner/stack flags/RET4/EAX original.
// Clear the current global unconditionally, restore base table00CE3818, free
// iff flags&1. Does not unregister: lifetime shutdown pops the registered
// primary before dispatching its CURRENT table00CECA14 word0=0051F770.
NativeModelEffectOptionsStorage* delete_model_effect_options_0051f770(
    NativeModelEffectOptionsStorage*, std::uint32_t flags,
    NativeModelEffectOptionsStorage* volatile& actual_global_00f8c27c) noexcept;

// Actual storage and complete established C++ behavior; new C++ interfaces,
// not drop-in native ABI/EH implementations. Gameplay remains unvalidated.
} // namespace bsp
