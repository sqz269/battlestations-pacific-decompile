#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit tick receiver fragments require MSVC Win32.
#endif

namespace bsp {

// Qualified business fragment 0087B699..0087B728, after the successful
// 0077EED0 parent construction. The actual receiver has live, aligned backing
// through +38Bh; the embedded node begins at +310h and payload is this same
// receiver. Borrow the genuine pending-registry/manager cells, fixed pending
// tail and timer word required by construct_native_tick_registration_00875890.
// Borrow the current D0E12C word; load it after +348/+34C/+350 writes and
// before the +35C write. Profiles are opaque DWORDs, never callable tables.
// No parent call, final +C4 class-id store, Native frame cleanup, validation,
// rollback, owner, fault equivalence or ECX/RET4 ABI is supplied here.
void register_native_unit_tick_receiver_0087b699(
    void* actual_parent_constructed_receiver,
    void* volatile& actual_registry_publication_00f878cc,
    void* volatile& actual_manager_publication_01090aa0,
    void* actual_fixed_pending_tail_00e0b704,
    const volatile std::uint32_t& actual_timer_word_00d7a260,
    const volatile std::uint32_t& actual_initial_word_00d0e12c);

} // namespace bsp
