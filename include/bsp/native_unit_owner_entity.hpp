#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native unit-owner storage requires MSVC Win32.
#endif

namespace bsp {

// Qualified normal continuation 0077EF27..0077EFA3 of 0077EED0.
// Requires the genuine containing allocation, its original factory preimage,
// successful 00928630(root, 1, unchanged full flag), and all three actual 34h
// recon records successfully constructed by the BF7CD1/805A70/804300 pair.
// The aligned allocation is writable through +30Fh. Bind the record owners to
// root+1E4, install the numeric owner profiles, and construct the first raw
// 0Ch sentinel through the accepted 0077B960/BF681B allocation boundary.
// Numeric profiles are evidence words, not Source-callable virtual tables.
// On success root+29C owns that sentinel; on allocation failure no list is
// acquired and the actual caller still owes the recon/base unwind. This is
// neither the base/record constructor nor its Native EH frame or whole root.
void construct_native_unit_owner_first_list_0077ef27(void* actual_root_after_recon);

// Qualified normal continuation 0077EFA9..0077F053. Requires the successful
// first stage with its live empty 0Ch list. Acquire the 18h sentinel through
// 0077B940, then perform the exact mixed-width observer/physical stores.
// Load the borrowed actual CE4ADC word once, after the +2C8 WORD store and
// before +2E8. On allocation failure the first list remains owned by the
// enclosing caller; this function performs no guessed rollback/unwind.
// Success owns both lists and stops before the actual E188A8/network-mode /
// E0AF20 read and possible 0077EB50 call. It cannot activate a receiver.
void construct_native_unit_owner_second_list_and_fields_0077efa9(
    void* actual_root_after_first_list,
    const volatile std::uint32_t& actual_initial_word_00ce4adc);

// Complete normal raw list leaves. The actual list header has sentinel at
// +4 and DWORD size at +8; nodes contain next/previous at +0/+4. These own
// real CRT allocations from the same allocation domain as the head producers.
// 0077B980 frees all 0Ch nodes, then sentinel, and zeros header+4.
// 0077D930 frees each non-null 18h node+14 payload (zeroing it before the
// node free), then each node. It retains the self-linked sentinel.
// Both clear size and re-read the current sentinel as the Original does.
// No header synthesis, invalid/null-list handling or fault-equivalence claim.
void tidy_native_unit_owner_peer_list_0077b980(void* actual_list_header) noexcept;
void clear_native_unit_owner_holder_list_0077d930(void* actual_list_header) noexcept;

// Qualified normal 0077E42C..0077E456 destructor continuation. Requires the
// actual preceding session/global withdrawal and +2B0 observer destruction,
// and live list ownership. Clear/free the +2A4 holder list, zero +2A8, then
// tidy +298 peer list. Leaves +1E8 recon array, GameEntity/SceneNode, root
// allocation, scalar flags and embedded tick/child destruction to their real
// owner. Not a complete destructor and never a scalar-delete substitute.
void destroy_native_unit_owner_lists_0077e42c(void* actual_root_after_observer) noexcept;

} // namespace bsp
