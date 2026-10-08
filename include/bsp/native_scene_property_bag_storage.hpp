#pragma once

#include <cstddef>
#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native scene property bag storage requires MSVC Win32.
#endif

namespace bsp {

// 008F41A0..008F41DC: whole 61-byte, 15-instruction raw storage constructor.
// The name is a hypothesis. This interface supplies fresh, unowned writable
// 0x114-byte storage; it does not establish a C++ class or bind native dispatch.
inline constexpr std::size_t native_scene_property_bag_storage_bytes = 0x114;

// Physical ABI: ECX=root, ONE owner DWORD at entry ESP+4, EAX=root, RET4.
// The unused EDX formal is essential: a two-formal __fastcall declaration would
// put the owner in EDX. Incoming EDX is overwritten, never used as the owner.
// Preconditions: DF=0 and valid flat ES. There is no CLD, allocation or guard.
// Every storage byte is initialized. Writes at +0/+4 are literal original DATA
// phase addresses, not Source vtables: they must never be dispatched here.
// root+4 is a borrowed 0x108-byte map interior, not an allocation to free.
// Owner bits are stored at +0x110 without dereferencing; +0x10C is zero ordinal.
void* __fastcall initialize_native_scene_property_bag_storage_008f41a0(
    void* root, std::uint32_t unused_edx, std::uint32_t borrowed_owner_bits) noexcept;

// Only the successful EMPTY-source branch of Original 008F41F0, using its
// real 114h allocation and whole raw61 initialization with owner bits zero.
// Borrow a stable valid unaliased raw61-produced 114h source: count at +8=0,
// all 64 heads zero, DF=0. Output initialization precedes the source count read.
// Nonzero count is rejected; no populated clone/iterator/insert is supplied.
// Return a DISTINCT mutable allocation in the canonical allocation/free domain.
// Input remains borrowed/unowned. Later whole record/bag lifetime handoff is
// separate; phase literals are not callable Source profiles or class ownership.
// Only successful allocations/normal valid memory are Native branch admission;
// this ordinary interface does not recover Native ABI/EH/failure or full clone.
void* clone_empty_native_scene_property_bag_008f41f0_fragment(
    const void* actual_source_bag);

} // namespace bsp
