#pragma once

#include "bsp/session_message_cc13_seg42.hpp"

namespace bsp {

// Complete scalar-deleting destructors for the C3h, C5h and C7h messages.
// Original ABI: ECX=this, stack flags byte, RET4, EAX=this (including after
// free). Stamp only +0 with the root profile; free storage iff flags bit 0.
// New typed C++ interfaces, not original-ABI entry points. Bit 0 requires an
// allocation from singleton_lifetime_allocate; flags=0 admits stack senders.
// Type names describe the recovered MT_* message roles, not original symbols.
seg42::PlaneChangeStateMessage* delete_plane_change_state_message_00762920(
    seg42::PlaneChangeStateMessage*, std::uint32_t flags);
seg42::UnitMessage* delete_plane_touchdown_message_00762b00(
    seg42::UnitMessage*, std::uint32_t flags);
seg42::PlaneDisappearMessage* delete_plane_disappear_message_00762bc0(
    seg42::PlaneDisappearMessage*, std::uint32_t flags);

} // namespace bsp
