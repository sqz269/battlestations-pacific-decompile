#pragma once

#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

namespace bsp {

// Three whole ordinary successful bodies, exposed through new C++ interfaces.
// Names describe storage roles; they are not recovered class/function symbols.
// Supply a genuine raw61-produced live 114h bag and the SAME retained concrete
// string/physical processes in its NativeScenePropertyInsertionContext. Finish
// explicit E175B0 startup before constructing that context. Its actual raw key
// domain, E177E4/E17654 cells, E175B0 generation and common E188B4 list stay live.
// Externally serialize shared-list mutations; retire records/payloads, keys and
// actual nodes before pool teardown. No owner, pool or application starts here.
//
// Keys satisfy the insertion provider's stable closed ASCII/current-C-locale
// contract. Roots/payloads are canonical current-CRT allocations. Owning graphs
// are finite, unique, unshared, acyclic and disjoint from the key/query/map/node,
// active frame and any replaced graph. Keep every borrowed input stable and
// readable through the copy. Successful publication transfers incoming owning
// root/child storage to the bag; the existing replacement/lifetime services
// retire it. Phase bits alone supply no provenance or callable Source vtable.
//
// Preserve all existing positive-size/no-overflow/no-alias/provider-success
// boundaries. No invalid memory, allocation failure, provider throw, reentry or
// concurrent trim/mutation is admitted. A null root allocation still reaches
// insertion, as in Original; no recovery/cleanup/transaction is added. The
// ordinary supported domain requires successful allocations. These interfaces
// do not reproduce Original ECX/stack/RET/register/FS/EH/class transport, native
// constructor-unwind cleanup, historical private CRT, game or parser startup.

// Whole [008F3590,008F3604), 116B/36 instructions. Original ECX=bag; stack
// key/data/count/FULL DWORD flags; RET10h; EAX returns bag through 008F33F0.
// Allocate genuine fresh 38h root, then call accepted raw type10 constructor
// with count/data/fullflags, then whole insertion. Count in [1,3FFFFFFFh];
// input is exactly 4*count opaque bytes. No integer or floating conversion.
// Every flag bit is forwarded; only its LOW BYTE determines copy vs retention.
// Low byte nonzero: copy to a fresh owning child, input stays caller-owned.
// Low byte zero: input MUST already be a sole-owned same-current-CRT allocation,
// caller-managed until successful publication transfers it with the root.
// Borrowed/stack/static/shared/foreign payloads are not owning retained inputs.
// Tags10/11 release every nonnull stored payload irrespective of copy flags or
// +2C (which is one in BOTH branches). Raw constructor retention alone does not
// transfer ownership. Preserve its 29 written/27 untouched bytes, including
// untouched +30 until insertion; do not zero-fill the fresh root.
void* add_native_scene_property_type10_array_008f3590(
    void* actual_bag, const char* actual_key, const void* actual_data,
    std::uint32_t element_count, std::uint32_t copy_flag_bits,
    NativeScenePropertyInsertionContext& context);

// Whole [008F3610,008F3684), 116B/36 instructions. Same entry/return, full
// flags and ownership rules; call accepted raw type11 with count/data/flags.
// Count in [1,15555555h]; input is exactly 12*count opaque bytes, with no
// inferred vector class, float arithmetic, count policy or overflow recovery.
void* add_native_scene_property_type11_array_008f3610(
    void* actual_bag, const char* actual_key, const void* actual_data,
    std::uint32_t element_count, std::uint32_t copy_flag_bits,
    NativeScenePropertyInsertionContext& context);

// Whole [008F3690,008F3701), 113B/41 instructions. Original ECX=bag; stack
// key/borrowed12B input; RET8; both exits return insertion's EAX=actual bag.
// New Source explicitly reuses accepted raw 008EF270 where Original inlines
// identical memory effects: phase/tag7, three interleaved DWORD loads/stores,
// zero +18/+1C/+20/+24/+30/+34, final BYTE1 at+2C. Writes45B, preserves11B:
// [08,0C),[28,2C),[2D,30). Input is never retained/freed/transferred. It must
// be disjoint from fresh root/frame and remain live for the three raw loads.
// No float formal, ST0/SSE/MXCSR arithmetic, parse/advance or native class is
// introduced. Source call/return and scratch-register differences are outside
// Original ABI claims; raw helper has no calls or observable intermediate gap
// in this ordinary disjoint domain. Tag7 owns no separate input allocation.
void* add_native_scene_property_three_word_008f3690(
    void* actual_bag, const char* actual_key, const void* actual_payload_words,
    NativeScenePropertyInsertionContext& context);

} // namespace bsp
