#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native Lua variant small link helpers require MSVC Win32.
#endif

namespace bsp {

// Provisional descriptive Source names for complete 006ED9D0..006ED9EB
// (28 bytes) and 006ED9F0..006EDA0A (27 bytes). No recovered tree type.
// One fastcall pointer occupies ECX; no dummy EDX or public stack argument.
// Read the current node's DWORD link (+8 for 006ED9D0, +0 for 006ED9F0),
// then the linked child's byte+31h. Any nonzero flag ends traversal. A zero
// flag selects that child and repeats the same ordered reads. The initial
// node's own flag is never tested. On return, EAX is the predecessor of the
// stopping child and EDX contains that child; ECX remains the initial node.
// Plain RET consumes only the return address (entry ESP+4 after return).
//
// No pointed-data writes, calls, pushes, pops or stack adjustment. EBX's
// value, ESI, EDI and EBP are preserved. The second body's six-byte identity
// LEA reads/writes EBX once on initial-zero traversal, without dereferencing
// EBX or changing flags. The first body's skipped three-byte identity LEA
// remains physically present. Last nonzero byte CMP flags survive RET:
// ZF=CF=OF=0, with SF/PF following the tested byte.
//
// Initial backing supplies a readable DWORD+8 (12-byte highest-address
// span) or DWORD+0 (4-byte span). Each tested child supplies readable byte
// +31h (50-byte span); a zero-flag child also supplies the next link DWORD.
// These are accessed spans, not object sizes, ownership or initialization.
// Reads may alias other backing, including the stack. The helper makes no
// writes to such aliases. Caller supplies a usable current return address.
// No null, cycle, alignment, type, tree, membership, lifetime or range guard.
// Zero-flag cycles can hang; invalid reads can fault. No concurrent-mutation
// consistency, fault recovery, allocation, owner, provider or raw noexcept
// promise is introduced. Source placement and the physical EAX result do
// not prove Original ABI, EH, runtime or gameplay equivalence.
void* __fastcall follow_native_lua_variant_links_006ed9d0(void* actual_node);

void* __fastcall follow_native_lua_variant_links_006ed9f0(void* actual_node);

} // namespace bsp
