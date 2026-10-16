#pragma once

namespace bsp {
struct NativeIntPointerTree18Iterator;

// Whole AD4040[137B/48 starts] aliases established 869990, with only its three
// BF6713 rel32 operands relocated. Both are ECX=actual mutable iterator, RET
// (or invalid-parameter tail); neither has local EH. This is a new ordinary
// Source __cdecl interface directly reusing that genuine complete provider.
//
// The required existing Source iterator is the actual eight-byte owner/node
// storage (offsets0/4), never a copied iterator or semantic-container cast.
// Caller independently retains that valid object and all reached actual
// nodes/owner/handler metadata through calls and failure. Nodes use genuine
// links0/4/8 and nil BYTE15; this operation reads no key, payload or color.
// Same SDK CRT handler/retained-graph obligations as 869990 remain in force.
// Its real invalid-parameter handler may return; current node reads/writes and
// partial climb writes are preserved by the existing provider, without replay,
// added validation/default, cleanup, owner, reference count or allocation.
//
// Externally serialize; valid handler changes must leave each later reached
// operand live. BOTH Native and Source callees' private stack/argument/scratch
// aliases, invalid object lifetimes, native heap/handler ABI, fault/SEH/private
// frame/register fidelity and game execution are outside this Source domain.
// No new 869990 reconstruction credit or whole insertion/Font load is claimed.
void __cdecl decrement_native_font_tree_iterator_00ad4040(
    NativeIntPointerTree18Iterator& actual_iterator);
} // namespace bsp
