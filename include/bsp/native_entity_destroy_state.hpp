#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native entity destroy state requires MSVC Win32 assembly.
#endif

namespace bsp {

// Full 00922FD0..00923011: ECX actual receiver, no explicit stack arguments.
// The descriptive name is provisional; no complete entity layout is implied.
// Capture/test child +48 before byte +5E/+5D/+5C = 1/1/0 and DWORD +6C = 1.
// Recurse only for current child DWORD +6C == 0, then reload that child's +44.
// Fetch the receiver's current vptr/+84 only after the child walk; restore EDI
// and ESI, then tail-jump with ECX=receiver, EAX=vptr and EDX=current target.
// The real target receives no stack arguments and uses the caller's return
// address. No typed result or complete target implementation is supplied here.
// Caller supplies actual writable storage, current links and a callable +84
// target compatible with that dispatch. The receiver must survive the late
// table load; each child must survive its parent's post-return +44 read. Other
// callers may also read the receiver after return (World reads +38).
// Link/callback behavior must permit termination; no cycle policy is added.
// Null/invalid storage can fault, with prior effects retained. No ownership,
// recovery, cleanup, exception translation or new noexcept guarantee is added.
void __fastcall mark_native_entity_destroy_state_00922fd0(void* actual_node);

} // namespace bsp
