#pragma once
#include "bsp/native_string.hpp"

namespace bsp {
// Complete ordinary normal 008F4B60 body through a new Win32 interface.
// Receiver is the genuine CEnum owner's nested map at owner+4: count+4,
// 64 heads+8. Save next+C before releasing each owning8h key, then return its
// real14h slot to the actual00E17578-compatible pool. Ignore mappedword+8.
// Clear a head only after its chain; clear count only after all64 heads.
void clear_native_enum_symbol_map_008f4b60(void* actual_map_receiver,
    void* actual_initialized_symbol_pool_00e17578,
    NativeStringRawPoolContext& actual_strings);

// Complete ordinary normal 008F4E70 body, not a scalar deleting destructor.
// Borrow the genuine initialized19Ch owner. Stamp D16508, clear owner+4,
// release current owning header114/118, stamp nested D162C0, clear again.
// Leave header/payload/statistics/name bytes stale; never free the owner.
void destroy_native_scene_enum_owner_008f4e70(void* actual_19c_owner,
    void* actual_initialized_symbol_pool_00e17578,
    NativeStringRawPoolContext& actual_strings);

// Successful coherent finite chains of unique valid aligned allocated slots,
// current genuine raw string/pool providers, stable disjoint borrowed owners;
// nonoverflowing and externally synchronized, without alias/reentry/mutation.
// Ordinary destruction is once per live owning header. No reset/default,
// mapped payload destruction, root free, native EH/SEH/failure or class ABI.
// Scalar008F59C0, global startup/cleanup, enum namespace/declaration identity,
// table mapped virtual destruction, traffic and game remain external.
} // namespace bsp
