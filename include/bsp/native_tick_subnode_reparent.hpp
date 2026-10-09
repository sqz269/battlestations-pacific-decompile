#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native tick subnode reparent composition requires MSVC Win32.
#endif

namespace bsp {

// Qualified plain C++ Source composition of 00876020..00876112[243]. Borrow
// actual receiver backing through +27h, the actual volatile incoming-node cell,
// and the actual F/M publication cells. Read the incoming cell exactly once,
// only after the real registry getter, optional Enter and tracked depth+1.
// Retain reached node/neighbor backing, their pointer/priority/count words,
// actual section storage and publication/domain lifetimes under child contracts.
// This API creates no complete receiver/node type or replacement publication.
//
// actual_guard_prefix is caller-owned live eight-byte Source backing for this
// invocation: profile DWORD at +0 and pointer word at +4. It is never a claimed
// Native EBP frame slot. Write profile CE37FC, then captured section K at +4.
// The backing and every raw field used here must support the corresponding
// aligned volatile uint32_t/int32_t/void* view. Aliases are supported only where
// those C++ storage, lifetime and access requirements remain compatible; there
// is no arbitrary Native stack/control-word or concurrency equivalence claim.
//
// After the late node and old-parent reads, arm private Source cleanup. A
// different nonnull old parent invokes the existing unlink API using a separate
// actual volatile child-input word initialized from the captured node: this
// represents Native PUSH ESI's value copy, not a reread of the incoming cell.
// Pass the real F/M references directly; EDX=0 is an unused Source placement.
// A nonnull old parent equal to receiver skips mutation. Otherwise preserve
// the raw signed-priority traversal and ordered current-pointer/count accesses.
// No validation, bounds check, cycle repair, clamp or rollback is supplied.
//
// Normal cleanup decrements current K+18h and calls actual Leave(K), using K
// captured before guard publication. Cleanup stays armed through that call and
// is disarmed only after it returns, or at null-K normal completion. On C++
// unwind, the private explicit-noexcept destructor calls only the real
// destroy_native_singleton_guard_00411ee0(actual_guard_prefix), which rereads
// current guard+4. A normal-release failure can therefore reach guard cleanup
// with its prior effects intact; cleanup throwing there terminates under current
// C++ policy. No catch, retry, extra unregister/free or mutation repair is added.
//
// Public may-throw void interface: no promised EAX result or Original ECX/EDX,
// RET4, FS/FH3, current-stack alias, register/flags, hardware-fault, production
// binding, startup or gameplay equivalence. The caller retains actual guard
// backing through every reached failure cleanup; private control is Source-only.
void reparent_native_tick_subnode_00876020(
    void* actual_receiver,
    void* volatile& actual_incoming_node_word,
    void* actual_guard_prefix,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0);

} // namespace bsp
