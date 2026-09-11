#pragma once

#include "bsp/native_resource_registry_erase.hpp"

namespace bsp {

// B19760[17]: ECX captured registry; RET, no semantic result. Clear the
// caller-borrowed actual F8D41C publication DWORD, then publish CE3818 on
// this registry. Do not follow the former publication or touch tree fields.
void reset_native_resource_registry_00b19760(
    void* registry, void* volatile& actual_publication_00f8d41c) noexcept;

// B1B5F0[111]: ECX actual registry; plain RET, no semantic result. Direct
// accesses require a 10h-byte registry prefix: profile0, preserved4,
// tree head8, count0C. Capture
// current head/minimum and by-value range iterators, then arm unwind reset.
// Complete current range erase precedes current-head free. After free clear
// head/count, actual publication, then captured registry profile, in order.
// Registry allocation itself and factory values are not owned here.
void destroy_native_resource_registry_00b1b5f0(
    void* registry, void* volatile& actual_publication_00f8d41c,
    ActualNativeStringPoolStorage& strings,
    const SingletonLifetimeCallbacks& invalid_parameters);

// The publication is a borrowed reference to actual storage, not a private
// singleton or a copy of its current value. Pool and CRT services are the
// complete existing actual-provider composition. New host C++ EH applies
// the B19760 reset to captured registry on propagation; native SEH/throw ABI
// and inherited noexcept pool-release/free failures are outside that domain.
// No registry getter, construction, population, scalar deletion or game claim.
} // namespace bsp
