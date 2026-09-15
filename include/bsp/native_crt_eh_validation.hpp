#pragma once

#include <cstdint>

namespace bsp {
// Complete _ValidateScopeTableHandlers, C168A0..C16950, 177 native bytes.
// Original custom register/stack entry: ECX=raw scope index, EDI=actual borrowed
// image base, [entry ESP+4]=actual borrowed scope-table pointer. The cdecl
// declaration describes only that one original stacked word; an ordinary C++
// call does not establish ECX/EDI and cannot supply the native entry contract.
// No extra argument, dummy EDX, binding, callback, or image owner is introduced.
//
// Returns EAX=1 after reaching index -1, otherwise 0 for a missing/non-executable
// section. Reads the handler then optional nonzero filter, page-aligns each
// wrapping 32-bit RVA and reuses the previous section/page cache. Uses the real
// EK __FindPESection provider and its original borrowed-header/fault contract.
// Saves/restores EBX/EBP/ESI, preserves EDI; ECX/EDX and arithmetic flags have
// path-dependent native effects. DF is unchanged. RET leaves the one argument
// for caller cleanup. There are no range, cycle, overflow, null or fault guards.
// A -1 initial index succeeds without reading the table contents or image.
// Actual caller-owned table/header lifetimes and raw registers remain required.
std::int32_t __cdecl validate_native_crt_scope_handlers_00c168a0(
    const void* native_scope_table);

// C16960 __ValidateEH3RN is deliberately not implemented: it needs the actual
// SEH4 dispatcher/scope/funclet domain. DD already owns its canonical data pages.
} // namespace bsp
