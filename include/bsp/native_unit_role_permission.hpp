#pragma once
#include <cstdint>

namespace bsp {
// Complete 0059BBD0..0059BBF2 (35 bytes). Native ECX is the actual unit,
// stack arguments are role then candidate slot, RET8, full EAX is exactly 0/1.
// Borrow valid unit storage and a role in 0..8. Read its actual DWORD policy
// at +188h+4*role; allow policy 9 or an exact candidate match. No ownership,
// initialization, bounds/null guard, or additional slot policy is supplied.
// New C++ interface; this does not reproduce the original calling convention.
std::uint32_t native_unit_role_open_to_slot_0059bbd0(const void* actual_unit,
    std::uint32_t role, std::uint32_t candidate_slot) noexcept;
} // namespace bsp
