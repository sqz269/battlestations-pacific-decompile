#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene DWORD vector range erase and clear require MSVC Win32.
#endif

namespace bsp {

// Complete004687E0..00468839 (90B/37ops). Native ECX control, unused EDX;
// five stacked DWORDs output,first_owner,first_node,last_owner,last_node;
// RET14h/EAX output. The entire native body equals004685A0 except two rel32
// CALL operands with identical BF6713/BF67A7 targets. Reuse its actual complete
// Source with LIVE actual_parameters20, not a native call or binary-ABI claim.
// Capture owner before returning diagnostic, current nodes afterward; unequal
// nodes read current control+8, exact SAR2/wrap, captured end before SDK; publish
// that end even for zero/negative lengths or a returning SDK report. Read current
// output last, then publish captured node+4 BEFORE pre-diagnostic owner+0.
void* erase_native_scene_dword_vector_range_004687e0(
    void* actual_control, const void* actual_parameters20);

// Complete00468D70..00468DA8 (57B/26ops). Native ECX control, unused EDX, no
// native stacked inputs/plain RET; retains the child's actual EAX result bits.
// No semantic ownership/lifetime meaning is assigned to that raw pointer.
// Additional Source pointers bind distinct native outgoing20/private-output8
// backing, not invented input argument cells. Stable/disjoint binding metadata
// is required; only reached backing needs to be valid. Payload/control may alias
// these cells. Capture old end before current begin/first unsigned check; after
// returning report capture current begin before current end/second check. Keep
// both across returning reports; write outgoing16,C,8,4,0 in native push order,
// then compose complete004687E0. Do not copy or initialize the private output.
void* clear_native_scene_dword_vector_00468d70(void* actual_control,
    void* actual_parameters20, void* actual_private_output8);

// Opaque16 control/4-byte range arithmetic, no nominal element or ownership
// type. Actual current CRT/SDK reporting, normal return and C++ throw transport
// are inherited. No callback, extra validation, state narrowing, element cleanup,
// rollback, noexcept, native private-home aliases, concurrency, original CRT/
// register/SEH/fault/binary ABI or game equivalence is supplied.

} // namespace bsp
