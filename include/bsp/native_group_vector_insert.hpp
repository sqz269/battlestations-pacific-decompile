#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual group-vector insertion requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;

// Complete0069B360..0069B3B0 and0069F1A0..0069F208 at the established
// actual current-CRT/owning length-error boundaries. Native allocator ECX
// unsigned16B count, EDX unconsumed, plainRET/EAX allocation; zero still
// allocates0. Source reserves ignored EDX. The no-input length throw never
// returns and preserves the actual28h legacy payload/count18 SBO schedule.
// Complete original bodies, message/vtable/throw data and own EH maps match
// accepted553300/556C90 before their Source is reused. No generic
// std::length_error, callback, invented allocator or payload is introduced.
void* __fastcall allocate_native_group_vector_0069b360(
    std::uint32_t count, void* unused_edx);
[[noreturn]] void __cdecl throw_native_group_vector_length_error_0069f1a0();

// Complete006A7690 ordinary body, catches006A7805/006A788E, prototype unwind
// C7F5C0 and normal cleanup6A78E8. Native ECX receiver, EDX unconsumed;
// stack(dead iterator owner,position,count,value ADDRESS),RET10h. Source
// reserves those inputs and adds actual raw-pool context,RET14h; void result.
// Receiveractual16B owns16B outer headers owning16B inner headers/8B strings.
// Always deep-construct an owned prototype through55B150 before count test,
// even count0. No write0, new proxy, validation, snapshot or self shortcut.
void __fastcall insert_native_group_vector_count_006a7690(
    void* header, std::uint32_t unused_edx, const void* unused_iterator_owner,
    void* position, std::uint32_t count, const void* value,
    NativeStringRawPoolContext& strings);

// Reallocation transfers original nested identities/capacities, fills from
// the owned prototype and transfers the suffix. Its catch owns only previously
// completed phases and raw new backing. After that scope is disarmed, old
// CURRENT backing destruction/free precedes capacity/end/begin publication.
// Short tail transfers first, then fills under a separate catch that reloads
// CURRENT receiver end. Long tail publishes end before backward swaps and
// gap assignment. Neither path gains rollback. Gap assignment uses the real
// accepted006A6CA0/006A6680 chain, not borrowed/header copying.
// Exceptional prototype cleanup uses actual55B110 CURRENT-begin Tidy. Normal
// cleanup instead destroys a captured range, frees the cached begin and does
// not clear fields; it is outside the prototype cleanup scope.
// DWORD wrap/SAR4 and ordered current reads/stores are retained. All reached
// storage, ownership, endpoint, actual pool and current CRT contracts apply;
// exclude native/Source private-frame aliases. Unconsumed private tag high
// bytes are qualified, not read from indeterminate C++ objects. Extra RawCtx,
// current C++ catch/owning exception transport and private frames do not bind
// native register/FH3/SEH/RTTI/fault/cleanup-failure/heap identity or gameplay.
} // namespace bsp
