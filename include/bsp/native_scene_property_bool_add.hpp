#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Whole Original [008F3940,008F39A1), 97 bytes/34 instructions, through a
// new ordinary context-bearing C++ interface. Name is a hypothesis, not a
// recovered symbol/class. Original ECX=actual bag, stack key/DWORD slot,
// LOWBYTE(slot) only, RET8; both exits return insertion's actual bag EAX.
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
// Allocate canonical current-CRT uninitialized 38h root. On nonnull, store
// opaque phase00CE89D4 DWORD at +0, tag3 DWORD at +4, payload BYTE at +C,
// DWORDzero at +18/+1C/+20/+24/+30/+34 in order, then BYTE1 at +2C.
// Writes34B and preserves22B: [08,0C),[0D,18),[28,2C),[2D,30). No root
// zero-fill, coalescing, DWORD payload store or clearing of +D..F occurs.
// All 256 byte patterns survive, without C++bool conversion/normalization;
// Original's upper24 slot bits are ignored. Tag3 has no owned child.
// Phase bits are opaque DATA, never invoked as a Source vtable. Successful
// publication transfers only the fresh root to the actual lifetime context.
//
// A null allocation still reaches whole insertion as in Original, without
// added recovery/cleanup/default. Admission requires successful, nonthrowing
// providers and valid disjoint memory; no OOM/fault/overflow/reentry/alias/
// concurrent trim is admitted. No transaction or exception rollback is added.
// New C++ ABI only: Original ECX/RET8/register/EFLAGS/FS/EH/class transport,
// historical private CRT, application/global binding, parser and game/runtime
// equivalence remain unproved. Original body has no local FS/EH registration.
void* add_native_scene_property_bool_byte_008f3940(
    void* actual_bag, const char* actual_key, std::uint8_t actual_value_bits,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
