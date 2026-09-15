#pragma once

namespace bsp {
// Complete native ___security_init_cookie, 00C1815E[148], original no-argument
// cdecl entry and plain RET. Requires the caller's actual writable canonical
// E15590 cookie and E15594 complement in the admitted E15000 page; never binds
// the host CRT cookie or creates/maps/initializes a data owner.
// Reads the current cookie before clearing FILETIME locals. Fast path writes
// only its complement; reseeding calls the five original real Win32 imports
// in order, ignores QPC's BOOL with its output slot uninitialized beforehand,
// applies the exact original adjustments, then writes cookie and complement.
// Original frame, registers, flags, branches and fixed operands are retained.
// Code/import addresses relocate; no original code-address/fault-continuation
// identity, service readiness, SEH, startup integration or gameplay is claimed.
void __cdecl initialize_native_crt_canonical_security_cookie_00c1815e();
} // namespace bsp
