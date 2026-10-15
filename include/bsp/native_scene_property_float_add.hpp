#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Whole Original [008F3770,008F37D5), 101 bytes/35 instructions, through a
// new context-bearing C++ interface. Name is a hypothesis, not a recovered
// class/symbol. Original ECX=bag, stack key/float32 bits, RET8, EAX=actual bag.
// MOVSS copies all32 bits; there is no ST0/x87 or numeric evaluation. This API
// takes uint32 bits and preserves signedzero/NaN/Inf representations as bytes,
// without a typed float conversion, filter, comparison or payload ownership.
//
// Supply a genuine raw61-produced live114h bag/map+4 and the SAME retained
// concrete string/physical processes in NativeScenePropertyInsertionContext.
// Explicit E175B0 startup precedes context construction. Keep actual nodes,
// E188B4 list, raw keys, E177E4/E17654 fallback cells and pool generations live.
// Stable closed ASCII/current-C-locale keys and finite, unique, unshared,
// acyclic owning graphs are required. Fresh root/key/query/map/node/frame and
// replaced graph are disjoint; externally serialize shared-list mutations.
// Retire records, keys and nodes before pool teardown. This starts nothing.
//
// Allocate canonical current-CRT 38h root. Physically store phase00CE89D4 at
// +0, then FULL payload DWORD at+C, THEN tag1 at+4; zero DWORDs at+18/+1C/
// +20/+24/+30/+34 in order, then BYTE1 at+2C. Writes37B, preserves19B at
// [08,0C),[10,18),[28,2C),[2D,30). No zero-fill, store coalescing or raw
// 008EF170 reuse (its phase/tag/payload order differs). Phase bits are never
// dispatched as a Source vtable. Type1 owns no child allocation; successful
// whole008F33F0 publication transfers only fresh root ownership to the bag.
// This implementation has no dependency on the integer3710 implementation.
//
// Null root still reaches insertion, with no new guard/default/recovery or
// cleanup transaction. Ordinary admission requires successful nonthrowing
// providers and valid stable/disjoint memory; OOM, fault, overflow, aliasing,
// reentry and concurrent trim/mutation are excluded by existing contracts.
// New C++ ABI only: no Original ECX/RET8/register/EFLAGS/XMM/FP/FS/EH/class,
// historical private-CRT, startup/global binding, wholeparser or runtime/game
// equivalence. Original body has no local FS/EH registration.
void* add_native_scene_property_float_bits_008f3770(
    void* actual_bag, const char* actual_key, std::uint32_t actual_payload_bits,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
