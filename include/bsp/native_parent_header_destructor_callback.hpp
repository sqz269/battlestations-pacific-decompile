#pragma once

#include <cstdint>

namespace bsp {

// Whole native [004C2D30, 004C2D35): ECX is the actual coherent 12-byte
// header; EDX is padding, with no stack arguments. This tail jump inherits
// the physical raw clear's plain RET, current node ownership and DF0 contract.
// Header/caller storage and borrowed disjoint payloads remain caller-owned.
void __fastcall destroy_native_parent_header_004c2d30(
    void* actual_header, std::uint32_t unused_edx) noexcept;

} // namespace bsp
