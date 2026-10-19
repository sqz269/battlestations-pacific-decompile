#pragma once
#include <cstdint>

// Four bounded leaves with unknown class owners. Generic array, placement-copy
// and allocator operations reviewed in this lane are out of scope, with no new
// bodies here. Evidence: docs/OCT18B_NB_HIGH.md. New source interfaces only.
namespace bsp::oct18b_nb_high {
// ECX -> EAX; RET / RET4 / RET8. Ignored native stack words remain explicit.
void* identity_00800810(void*) noexcept;
void* identity_00800820(void*, std::uint32_t) noexcept;
void* identity_0088ab20(void*, std::uint32_t, std::uint32_t) noexcept;
// ECX owner -> EAX=[owner+4], RET. Exact owning type is unknown.
std::uint32_t owner_word4_008721f0(const void*) noexcept;
}
