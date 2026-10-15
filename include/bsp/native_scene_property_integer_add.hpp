#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Whole Original [008F3710,008F376D), 93 bytes/34 instructions, through a
// new ordinary context-bearing C++ interface. Name is a hypothesis, not a
// recovered symbol/class. Original ECX=bag, stack key/full DWORD value, RET8;
// both exits return actual bag EAX from whole insertion 008F33F0.
//
// Supply a genuine raw61-produced live 114h bag and its actual map at +4.
// Use the SAME retained concrete string/physical processes through the
// existing insertion context: actual E175B0 nodes, common E188B4 list, raw
// keys and E177E4/E17654 fallback cells. Complete explicit pool startup
// before context construction; keep processes, cells and generations live.
// This function starts nothing and creates no callback or substitute owner.
// Keys obey the stable closed ASCII/current-C-locale insertion contract.
// Owning graphs are finite, unique, unshared and acyclic. Fresh root and
// key/query/node/map/frame/replaced graph are disjoint; externally serialize
// list mutation. Retire actual records, keys and nodes before pool teardown.
//
// Allocate canonical current-CRT 38h root. On nonnull, physically store
// DWORD tag0 at +4 BEFORE opaque phase00CE89D4 at +0, full value at +C,
// DWORDzero at +18/+1C/+20/+24/+30/+34 in that order, then BYTE1 at +2C.
// Writes37B and preserves19B: [08,0C),[10,18),[28,2C),[2D,30). No root
// zero-fill, store coalescing or phase-first raw008EF140 reuse is supplied.
// Value bits are never converted, narrowed, dereferenced or treated as an
// owned payload pointer. Phase bits are never invoked as a Source vtable.
// Type0 has no owned child; successful insertion transfers only fresh root
// ownership to the actual bag under its existing whole lifetime context.
//
// A null allocation still reaches insertion as in Original, without added
// recovery/cleanup/default. Admission requires successful, nonthrowing
// providers and valid disjoint memory; no OOM/fault/overflow/reentry/alias/
// concurrent trim is admitted. No transaction or exception rollback is added.
// New C++ ABI only: Original ECX/RET8/register/EFLAGS/FS/EH/class transport,
// historical private CRT, application/global binding, parser and game/runtime
// equivalence remain unproved. Original body has no local FS/EH registration.
void* add_native_scene_property_integer_word_008f3710(
    void* actual_bag, const char* actual_key, std::uint32_t actual_value_bits,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
