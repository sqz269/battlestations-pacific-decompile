#pragma once

#include "bsp/native_world_chain_headers.hpp"

namespace bsp {

struct NativeGamePhysicsLifetimeCalls;

// New ordinary C++ interface for complete 009041A0 (100 bytes): native ECX
// is the actual 12-byte first/last/count header, with no stacked arguments.
// Borrow the genuine header object produced by the World chain-header producer
// or an equivalent qualified producer. It must remain alive for the entire
// call, including every entity deletion; its address is retained throughout.
// Nonempty headers supply actual entities with pointer links at +34/+38 and
// callable current primary-table slot0 methods accepting __thiscall flags1.
// Their scalar lifetimes must be valid for their original allocation domain.
// Numeric original-image tables and projected/token entities do not qualify.
//
// Calls the existing concrete NativeGamePhysicsLifetimeCalls default dispatcher
// explicitly. Service overrides are not used. The supplied service outlives
// this call. Deletion may free its entity and change retained-header fields.
// This routine frees neither header nor World. Exceptions propagate after any
// completed unlink writes, without rollback; there is no progress guard.
// Single-threaded valid-storage schedule only: no concurrent mutation, native
// fault/SEH equivalence, original binary ABI or full World readiness is claimed.
void drain_native_world_entity_chain_009041a0(
    NativeWorldChainHeader* actual_header,
    NativeGamePhysicsLifetimeCalls& lifetime_calls);

} // namespace bsp
