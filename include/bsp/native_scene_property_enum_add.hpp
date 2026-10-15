#pragma once

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Whole Original 008F39B0..008F3A18 (104B/36 instructions) and
// 008F3A20..008F3AB9 (153B/49 instructions), exclusive ends. Names are
// hypotheses. Original ECX=actual114h bag; stack key/declaration/value-or-
// symbol, RET0C; EAX=actual bag returned by whole insertion008F33F0.
// These are new context-bearing ordinary C++ interfaces, not Original ABI.
//
// Supply a genuine raw61-produced bag/map+4, same canonical current heap,
// same retained concrete string and physical processes as insertion/lifetime
// keys, actual initialized E175B0 property nodes/common E188B4 list, raw-key
// manager gate, and distinct E177E4/property,E17654/query,E186ED/enum cells.
// Explicit startups precede context construction; these leaves start nothing.
// Keep processes, pool generations, actual nodes, keys and cells live.
// Owning graphs are finite, unique, unshared, acyclic and disjoint from fresh
// root, key/query/map/node, replaced graph, enum/symbol and caller frame.
// Externally serialize shared mutation. Retire all borrowers before enum
// retirement and records/keys/nodes before pool teardown. Closed stable
// NUL-free ASCII keys/symbols, valid matching buffers, current C locale,
// nonoverflowing successful nonthrowing providers and no reentry/alias/
// concurrent trim are required. Borrowed key/symbol strings are not consumed.
//
// Both allocate canonical uninitialized38h root, write opaque phaseCE89D4
// DWORD+0 and tag4 DWORD+4, and eventually transfer only fresh root to bag.
// Type4 declaration/value words are borrowed/opaque, never owned or filtered.
// Both write41B/preserve15B: [08,0C),[10,18),[2D,30). No zero-fill,
// coalescing, phase dispatch, raw8EF230 reuse/patching or value narrowing.
// Null allocation skips all stores/getter and still passes literal null to
// whole insertion; no added default, recovery, cleanup or transaction.
// Stable disjoint by-value inputs permit ordinary comparison, without exact
// Original postallocation/per-store argument-load timing or observer claims.
//
// 39B0 exact stores after phase/tag: zero18/1C/20/24, full declaration28,
// zero30, zero34, full value0C, BYTE1at2C. Neither input word is dereferenced;
// later declaration consumers require a genuine retained enum when used.
void* add_native_scene_property_enum_word_008f39b0(
    void* actual_bag, const char* actual_key,
    std::uint32_t declaration_identity_bits, std::uint32_t value_bits,
    NativeScenePropertyInsertionContext& context);

// 3A20 exact stores after phase/tag: zero18/1C/20/24, zero30, full enum28;
// ONLY NOW whole48E840 getter using SAME actual_strings.raw_context() and
// its distinct E186ED/E17654 cells; then full result0C, zero34, BYTE1at2C.
// Require genuine live19Ch enum/+4 map, canonical14h E17578 symbol nodes
// (never E175E8 property/table substitute), coherent finite chains and a
// KNOWN-FOUND symbol. Getter captures full node+8 BEFORE temporary raw8B
// key release. No missing-key/default/type/numeric/range policy is added.
// Original saved-allocation state0/FuncInfoDD769C/handlerCA49CB/actionCA49C0
// invokes realBF65AC free, disarmed before insertion. Successful nonthrowing
// Source deliberately adds no RAII rollback, Original FS/EH/fault/privateCRT
// cleanup transport or failure equivalence. No ABI/startup/parser/game claim.
void* add_native_scene_property_enum_symbol_008f3a20(
    void* actual_bag, const char* actual_key, const void* actual_enum_owner,
    const char* actual_symbol, game::GameNativeStringProcess& actual_strings,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
