#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_pair_retire {

// Descriptive hypotheses over actual raw Win32 object bases. Endpoint
// +30h/+34h/+38h is reference data/size/capacity. No class or object is made.
// 00C40E50..00C40EE2: ESI endpoint, stack index, RET4. The full observed EAX
// bits are returned: normally the copied last reference, with a different
// register value on the retained unsigned-underflow resize path. A semantic
// native return contract is not asserted by this public C++ interface.
std::uint32_t erase_pair_reference_00c40e50(
    void* endpoint, std::uint32_t index, const cc15_dyn_live::DynLiveCrt& crt);

// 00C4C5D0..00C4C67D: EAX first endpoint, stack(storage, other endpoint),
// RET8. Finds a pair from first endpoint's references, removes its references
// from both endpoints, unlinks it and pushes it on storage+34h; storage+58h
// decrements. Missing/invalid pairs are not guarded and can fault. Observed
// EAX is the reloaded old next link, preserved as the return pointer here.
// Required actual CRT new/free custody is forwarded directly to the leaf.
// No default provider, exception rollback or original register ABI adapter.
void* retire_endpoint_pair_00c4c5d0(
    void* first_endpoint, void* storage, void* other_endpoint,
    const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_pair_retire
