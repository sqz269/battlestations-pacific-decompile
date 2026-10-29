#pragma once

#include "bsp/cc15_dyn_live.hpp"

namespace bsp::cc16_dyn_storage {

// Descriptive names are hypotheses. These functions operate on actual Win32
// native object storage; no class identity or fabricated object is supplied.
// Endpoint +30h/+34h/+38h is {four-byte references, size, capacity}.
// 00C37390..00C373FC: ESI endpoint, one pair pointer on stack, RET4. The
// returned pointer preserves observed EAX (the computed insertion slot),
// without asserting that callers use it as a semantic return value.
void** append_pair_reference_00c37390(void* endpoint, void* pair,
                                    const cc15_dyn_live::DynLiveCrt& crt);

// 00C40A30..00C40B2D: EDI storage, stack(first endpoint, second endpoint),
// RET8. Storage uses +28h/+2Ch/+30h for its page-pointer vector, +34h for
// the free head, an intrusive-list sentinel at +48h (tail at +50h), and
// +58h for live count. Nodes have endpoint words at +0/+4 and links +8/+Ch.
// Both actual endpoint objects are passed intact to the append leaf.
// EAX after the second append is preserved as the returned insertion slot.
//
// Required CRT entries: operator_new_00bf55be, free_00bf6989 and
// malloc_00bf9f1a, all from the caller's real heap custody. No default,
// allocation-failure guard, rollback, exception translation or native
// register/stack ABI adapter is provided. These are new C++ interfaces.
void** add_endpoint_pair_00c40a30(void* storage, void* first_endpoint,
                                void* second_endpoint,
                                const cc15_dyn_live::DynLiveCrt& crt);

}  // namespace bsp::cc16_dyn_storage
