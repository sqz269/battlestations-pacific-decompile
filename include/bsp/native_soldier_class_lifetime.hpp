#pragma once

#include "bsp/native_soldier_class_resolution.hpp"
#include "bsp/singleton_lifetime.hpp"

namespace bsp {

// Complete128B destructor: ECX class, RET. Install the derived table, erase the
// actual LoopLengths tree, free its CURRENT sentinel and clear head/count;
// then perform genuine489E80 base cleanup. State0 also cleans the base during
// C++ unwind. A second unwind exception terminates. No owner storage release.
void destroy_native_soldier_class_004b1120(
    void* actual_class, const NativeSoldierClassConstructionAccess&,
    const SingletonLifetimeCallbacks&);

// Complete30B scalar deletion: ECX class, stack flags, EAX original class,
// RET4. Complete destruction precedes bit0-controlled raw storage release.
void* delete_native_soldier_class_004b1310(
    void* actual_class, std::uint32_t flags,
    const NativeSoldierClassConstructionAccess&,
    const SingletonLifetimeCallbacks&);

// New C++ interfaces; native table identities are not callable host vtables.
// Original thiscall/FH3/SEH compatibility and gameplay remain unvalidated.
} // namespace bsp
