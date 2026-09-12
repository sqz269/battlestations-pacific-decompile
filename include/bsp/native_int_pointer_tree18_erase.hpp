#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native IntPointerTree18 erasure requires MSVC Win32.
#endif

namespace bsp {

// 86E8A0[679]: ECX actual tree, incoming EDX unconsumed, stack output,
// iterator owner, iterator node; EAX output, RET0Ch. Preserve the actual
// by-value iterator argument slots, successor/transplant/rebalance, actual
// original-node free, current unsigned count decrement and output owner/node
// capture/store order. No payload release and no tree/input-owner equality
// check is added. Returning invalid handlers retain native continuation.
// Nil input restores the entry stack/nonvolatile registers and tails a fixed
// owning source out_of_range helper. This helper performs full27-byte string
// assignment before arming temporary cleanup, constructs actual28h D6926C
// exception storage, and uses its completed native copy/destruction providers.
void* __fastcall erase_native_int_pointer_tree18_iterator_0086e8a0(
    void* tree, void* unused_edx, void* output, void* input_owner, void* input_node);

// 86EE50[201]: ECX actual tree, EDX unconsumed, stack output, first owner/node,
// last owner/node; EAX output, RET14h. Full-range reset plus complete checked
// partial erase loop. First advances in its actual argument slots before a
// separate by-value old iterator is erased. Output may alias live raw storage.
void* __fastcall erase_native_int_pointer_tree18_range_0086ee50(
    void* tree, void* unused_edx, void* output, void* first_owner, void* first_node,
    void* last_owner, void* last_node);

// 86FDE0[52]: ECX actual tree header, EDX unconsumed, RET. Invoke full current
// begin/end range, free current head, zero current head/count; no payload free.
void __fastcall destroy_native_int_pointer_tree18_0086fde0(void* tree);

// Raw header/node/iterator layouts are in native_int_pointer_tree18_leaves.hpp.
// Memory and returning validation use the actual source CRT domain. Native
// FH3 stack identity, mutable native EH-spill aliases, hardware-fault cleanup,
// provider volatile-register/exception identity and original C++ RTTI/catch ABI
// remain explicit boundaries. The owning exception class is the established
// NativeHardwareLayoutInvalidIterator transport; no hardware tree is used.
// No partial-range shortcut, replacement std::map or no-op provider is added.
} // namespace bsp
