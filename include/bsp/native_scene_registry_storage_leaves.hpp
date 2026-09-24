#pragma once
#include <cstdint>

namespace bsp {

// Required actual BF681B allocator entry: cdecl(size DWORD), malloc/new-handler
// retry and bad_alloc throw contract. No success-only allocator or null default
// is supplied. The owning runtime keeps this borrowed entry slot valid; it is
// read at the original allocation call. The entry may throw through these
// leaves, which have no native cleanup state. Allocations remain caller-owned.
using NativeSceneRegistryAllocator = void* (__cdecl*)(std::uint32_t);

// Complete B82390..B823A9[26]. Native ECX ignored, no stack arguments, EAX
// allocation, RET. The new API uses ECX for the borrowed allocator-entry slot.
// Allocate Ch; if result!=0 store [result]=result; independently test wrapped
// result+4 before its self-link store. Preserve the allocator's +8 preimage.
void* __fastcall allocate_native_scene_registry_sentinel_00b82390(
    NativeSceneRegistryAllocator const volatile& actual_00bf681b);

// Complete B823B0..B823E2[51]. Native ECX ignored, three stack DWORD slots
// next, previous, key-source pointer, EAX allocation, RET Ch. ECX now borrows
// the actual allocator entry; the explicit unused EDX keeps all three original
// arguments stacked. Allocate Ch, then separately test wrapped result+0/+4/+8.
// Read each current stack argument only at its native store stage; dereference
// current key source after the preceding link writes. No key snapshot, link
// splicing or light retain is added. Pointer values are original-width words.
void* __fastcall allocate_native_scene_registry_node_00b823b0(
    NativeSceneRegistryAllocator const volatile& actual_00bf681b,
    void* unused_edx, std::uint32_t next, std::uint32_t previous,
    const void* key_source);

// Complete B82570..B8259B[44]. Original ECX destination, EDX unsigned count,
// four stack slots, RET10h. Only the first stacked pair-source pointer is read,
// and only when count!=0. For each nonnull current destination copy source+0,
// then freshly read/copy source+4; source and destination may overlap. Decrement
// the unsigned count and wrap destination+=8 even when destination is null.
// No count clamp, pair snapshot or allocation. Incidental EAX is not a result.
void __fastcall fill_native_scene_registry_iterators_00b82570(
    void* destination, std::uint32_t count, const void* pair_source,
    std::uint32_t unused_slot2, std::uint32_t unused_slot3,
    std::uint32_t unused_slot4) noexcept;

// Complete B821C0..B821EB[44]. Original ECX first, EDX last, four stack
// slots, RET10h; EAX is the advanced destination. Only the first stacked
// destination is read. Walk the half-open range in wrapping 8-byte steps;
// for each nonnull destination copy the first DWORD before reading the second.
// Overlap therefore has forward, componentwise semantics, not memmove semantics.
// A null current destination skips both source reads but still advances.
// The caller supplies a finite reachable range and accessible reached words.
void* __fastcall copy_native_scene_registry_iterators_00b821c0(
    const void* first, const void* last, void* destination,
    std::uint32_t unused_slot2, std::uint32_t unused_slot3,
    std::uint32_t unused_slot4) noexcept;

// Complete B82B80..B82BB1[50]. Original ECX supplies the scratch preimage;
// EDX is overwritten. Three stack words are destination, count, pair source;
// RET Ch, EAX is captured destination + captured count*8, wrapping at 32 bits.
// This wrapper preserves the native scratch/argument load schedule and calls
// the actual B82570 provider above. No allocation, count credit or host record.
void* __fastcall fill_native_scene_registry_iterators_end_00b82b80(
    std::uint32_t incoming_ecx, void* unused_edx, void* destination,
    std::uint32_t count, const void* pair_source) noexcept;

// Raw Ch nodes use next+0, previous+4, borrowed key+8; iterator pairs are two
// DWORDs. Existing SceneNodeRegistry has private typed Entry/Iterator records
// and a different owner layout; these leaves never cast its owner or initialize
// an absent 3Ch scene resource. Names are hypotheses. New allocator context
// interfaces and focused fixtures do not claim whole-registry or gameplay proof.
} // namespace bsp
