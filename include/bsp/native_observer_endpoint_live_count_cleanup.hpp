#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native observer endpoint live-count cleanup requires MSVC Win32.
#endif

namespace bsp {

struct NativePendingEntityOwners;
class NativePendingEntityProducerAccess;

// Complete 007EE620..007EE665[70] through a new naked Source interface. ECX
// is the actual raw receiver; EDX is unused placement. Two stack words carry
// references to the actual pending owners and producer access. Source RET8
// consumes those references; the Native body ends in plain RET.
//
// Capture the receiver, read signed count+3CCh, then visit current inline
// DWORD entries at +3D0h with a fresh signed count read after each iteration.
// For each selected pointer, read byte+5Ch first, then DWORD+900h only when
// that byte is zero. When the DWORD is one, call the admitted actual kill
// provider with raw cause one. Preserve the conditional EDI save and write
// only byte receiver+3ECh after POP ESI and before POP EBP. No count snapshot,
// array-length clamp, pointer validation or endpoint/class type is supplied.
//
// Borrow the same actual receiver/entry backing and the same entity identities
// for the full call. The access must map each selected void* to that actual
// entity's live field lvalues and current virtual/lock providers, under the
// admitted producer contract. Supply initialized actual shared pending owners;
// keep both owners and access alive through calls, callbacks and recursive kill
// work. No storage, resolver implementation or selected runtime binding is added.
//
// On an eligible entry, load the access reference at its scheduled stack read
// before the Native cause push; load owners after pushing cause and actual ECX.
// Preserve those added argument slots until their reads, and all active save,
// return and selected-memory backing under aliases. Extra cdecl arguments and
// caller cleanup change stack placement from the Native call's single argument.
// No additional register save is introduced. Void promises no common EAX value.
//
// No local EH, RAII, catch or guaranteed final flag write is added. Existing
// Source producer/CRT/lock/service and exception policies, stack aliases,
// Original ABI/placement and runtime behavior remain separately qualified.
// This declaration deliberately lacks noexcept.
void __fastcall cleanup_native_observer_endpoint_live_count_007ee620(
    void* actual_receiver,
    std::uint32_t unused_edx,
    NativePendingEntityOwners& actual_pending_owners,
    NativePendingEntityProducerAccess& actual_producer_access);

} // namespace bsp
