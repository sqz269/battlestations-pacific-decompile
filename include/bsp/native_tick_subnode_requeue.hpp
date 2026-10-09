#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick subnode requeue composition requires MSVC Win32.
#endif

namespace bsp {

// Qualified ordinary C++ Source composition of 00876120..00876175[86].
// Read current receiver+24h as signed int32 exactly once. If <=1, return:
// no getter, incoming-node/F/M cell contents, raw guard or child is accessed.
// Source reference arguments still require valid C++ bindings. No node/list
// validation, count clamp, producer, publication or ownership is supplied.
//
// On the active path, pass the actual F/M cells to the real registry getter;
// capture its result+4 section K once. Optional SDK Enter(K) and current
// K+18h modulo increment precede the ONLY incoming-node cell read.
// Native reads a raw Node value from its first stack argument word. This
// Source API instead borrows a volatile cell to retain that late access;
// it does not recover a Native pointer-to-cell argument or stack identity.
//
// Call actual raw unlink 00874E60 with receiver+1Ch and captured Node VALUE,
// not a cell reference. Its explicit EDX=0 is Source placement only. Ignore
// its semantic return, then clear captured Node+4 even when K is null.
// Normal release decrements current captured K+18h and calls SDK Leave(K).
// After release, lend a NEW actual volatile node word initialized from the
// captured value to real reparent 00876020, with original guard/F/M inputs.
// The original incoming cell is neither reread nor forwarded to reparent.
//
// actual_guard_prefix is borrowed live eight-byte Source backing, used only
// by the reached reparent child under its current normal/failure contract.
// Keep that guard and both actual publication cells alive at stable addresses
// through every reached call. Do not substitute private publication snapshots
// or claim the guard/new node word as a Native frame or saved-stack slot.
//
// The receiver count requires compatible aligned int32 storage. The active
// path additionally requires actual receiver/list backing through +27h,
// reached node/neighbor fields, registry+4 pointer storage, captured Win32
// section and tracked DWORD+18h, plus all reparent backing/lifetime needs.
// Raw fields selected by this C++ body must support their corresponding
// aligned volatile int32_t/uint32_t/void* views. Preserve child nonvolatile
// and control backing requirements; arbitrary Native stack aliases are not
// modeled by captured C++ values. Volatile adds no atomicity guarantee.
//
// No outer guard, catch, rollback or failure release is added. Earlier
// effects remain if a later operation fails. Reparent supplies only its own
// admitted cleanup. C++ EH through the naked raw leaf remains unexecuted.
// May-throw void API; no semantic EAX, Original ECX/EDX/RET4/frame/flags,
// exception, hardware-fault, production-binding or gameplay equivalence.
void requeue_native_tick_subnode_00876120(
    void* actual_receiver,
    void* volatile& actual_incoming_node_word,
    void* actual_guard_prefix,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
