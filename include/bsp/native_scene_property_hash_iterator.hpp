#pragma once

#include <cstdint>

namespace bsp {

// Descriptive hypotheses for the distinct whole entries 008EEB10/008EEB60.
// Ordinary MSVC Win32 fastcall: ECX=actual aligned writable raw12 iterator;
// incoming EDX unused, no stacked arguments, plain RET. Words are table+0,
// current+4 and index+8. Initialize only table; Begin establishes current,
// and establishes index only for a nonzero table count. Begin has no common
// semantic EAX result; Next returns the full DWORD 0/1 after rereading current.
//
// Borrow the genuine live raw61 property bag's map at bag+4 (108h), or the
// accepted raw enum owner's map at owner+4. Retain their canonical process,
// E188B4 list, distinct E175B0 property/E17578 symbol pools, generations,
// owning keys, nodes and lifetimes. Nodes have next+C; allocator+10 survives.
// These entries allocate, construct, dispatch and release nothing.
//
// Require valid aligned reached DWORD ranges, coherent stable finite unshared
// acyclic maps/chains, and iterator/map/nodes/private frames disjoint. Exclude
// faults, aliasing, reentry, mutation, concurrency, pool trim and teardown.
// Begin's actual scan can read head64; retain that load, never manufacture a
// sentinel from map+108 owner metadata. Authentic nonempty maps reach a real
// nonnull head among the 64 buckets. Unsigned DWORD arithmetic is preserved.
//
// Independent naked bodies retain all 157 Original bytes/55 instructions,
// including Next's four-byte LEA ESP,[ESP]. Byte equality is static evidence;
// Original full register/flags/private-frame/CRT/EH ABI and caller, parser,
// startup, lifetime, runtime and game equivalence are separate unproved gates.
void __fastcall begin_native_scene_property_hash_008eeb10(void* actual_iterator12);
std::uint32_t __fastcall next_native_scene_property_hash_008eeb60(void* actual_iterator12);

} // namespace bsp
