#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_sap_update {

// Descriptive hypotheses over actual raw Win32 storage. Each axis has an
// eight-byte-entry vector at storage+4+axis*0Ch: {data,size,capacity}. Each
// entry is {sort key, endpoint pointer with a low minimum/maximum tag}.
// Endpoint bounds are at +4/+10h, indices at +20h/+26h, and flag at +1Ch.
// No class identity, object, global or default CRT custody is fabricated.
//
// 00C40900..00C40A2A: ECX storage, stack(endpoint,axis), RET8. Appends two
// entries with the captured old count as both initial keys, then stores
// their truncated indices. Returns observed EAX (old count+1), without
// asserting a semantic native return contract. Capacity/byte arithmetic
// wraps at 32 bits; only equality triggers growth and only a zero computed
// destination suppresses an entry write. Actual new/free are mandatory.
std::uint32_t append_axis_endpoints_00c40900(
    void* storage, void* endpoint, std::uint32_t axis,
    const cc15_dyn_live::DynLiveCrt& crt);

// 00C4C700..00C4CBA4: ESI axis, stack(storage,endpoint), RET8. Copies each
// float bound through ambient x87 FLD/FSTP, then transforms the stored bits
// and orders minimum before maximum. The copied endpoint tags determine
// which wrapping ushort index changes; crossings compose the real recovered
// add/retire functions with these same objects and this same CRT. Required
// CRT entries are new/free/malloc because add/retire can allocate.
//
// This source requires MSVC x86 inline assembly for the two x87 copies. No
// control-word change, allocation guard, pair guard, rollback, exception
// translation, original register-ABI adapter or activation is supplied.
// These public declarations are new C++ interfaces, not binary replacements.
void update_axis_endpoints_00c4c700(
    void* storage, void* endpoint, std::uint32_t axis,
    const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_sap_update
