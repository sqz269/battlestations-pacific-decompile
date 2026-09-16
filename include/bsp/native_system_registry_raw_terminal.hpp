#pragma once

#include "bsp/native_system_constant_registry.hpp"

namespace bsp {

// Full B5DF00, native ECX actual10h owner, plain RET. Stamp D62A3C, arm
// state0 for B5BA80(captured owner), destroy CURRENT array/count and free its
// CURRENT data, then disarm before normal base cleanup. Released array fields
// remain stale. No retained Operation, binding admission or owner free.
// The caller/context must satisfy the raw constructor's actual-cell/provider
// contract; the owner can differ from CURRENT0108FE94. Source C++ exceptions
// run the armed cleanup; a second cleanup exception terminates. Original FH3,
// SEH/hardware faults and mutable EH-spill aliases are not this source ABI.
void destroy_native_system_constant_registry_00b5df00(
    NativeSystemConstantRegistryStorage&, NativeSystemConstantRegistryRawContext&);

// Full B5DF70 and B5BB20, respectively30bytes. Native ECX captured owner,
// stack flags, EAX captured owner bits, RET4. Always call the respective raw
// derived/base destructor first; only a returning destructor permits flags&1
// to free captured owner through the matching source CRT domain. A throwing
// destructor leaves that allocation caller-owned, with native partial effects.
NativeSystemConstantRegistryStorage* delete_native_system_constant_registry_00b5df70(
    NativeSystemConstantRegistryStorage*, NativeSystemConstantRegistryRawContext&,
    std::uint32_t flags);
NativeSystemConstantRegistryStorage* delete_native_system_constant_base_00b5bb20(
    NativeSystemConstantRegistryStorage*, NativeSystemConstantRegistryRawContext&,
    std::uint32_t flags);

} // namespace bsp
