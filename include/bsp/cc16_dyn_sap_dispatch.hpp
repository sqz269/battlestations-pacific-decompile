#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_sap_dispatch {

// Descriptive hypotheses; these are explicit new C++ interfaces over the
// actual raw Win32 manager/endpoint objects and caller's genuine CRT.
// No class, vtable, object, global or default provider is fabricated.
//
// 00C4CBB0..00C4CBE5: RTTI Dyn::SAPBroadPhaseManager slot 2; ECX manager,
// stack endpoint, RET4. Checks endpoint+2Ch once, then calls the actual
// recovered axis updater for 0,1,2. Does not reread or change that flag.
void update_flagged_endpoint_00c4cbb0(
    void* manager, void* endpoint, const cc15_dyn_live::DynLiveCrt& crt);

// 00C4CBF0..00C4CC82: EDI manager, no stack arguments, RET0; original
// stack frame aligns ESP down to eight bytes. Pending data/count are at
// manager+194h/+198h. Each endpoint is captured once per iteration, then
// append axes 0,1,2; move keys left axes 0,1; update axis 2; flag+2Ch=1.
// Data reloads each iteration; the unsigned live count reload follows the
// flag store. Count is cleared only after normal loop/empty completion.
void process_pending_endpoints_00c4cbf0(
    void* manager, const cc15_dyn_live::DynLiveCrt& crt);

// 00C5A850..00C5A8AC: RTTI SAPBroadPhaseManager slot3; ECX manager, RET.
// A single unsigned pending-count >10 test selects the real full radix rebuild.
// Otherwise process pending endpoints, then walk actual ring+AC to sentinel+B0
// and update axes0/1/2 before reloading each current endpoint's next+40.
// The two volatile cells must be the actual canonical0109EEA4/D7A280 bindings
// required by that rebuild. No default or early value snapshot is supplied.
void update_broadphase_00c5a850(
    void* manager, const cc15_dyn_live::DynLiveCrt& crt,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile double& actual_half_00d7a280);

// The same actual manager/endpoint/CRT is passed directly to the recovered
// leaves. Required CRT new/free/malloc custody is mandatory; no allocation
// guard, rollback, exception translation or original register-ABI adapter
// is supplied. Native incidental EAX values are not semantic return claims.

}  // namespace bsp::cc16_dyn_sap_dispatch
