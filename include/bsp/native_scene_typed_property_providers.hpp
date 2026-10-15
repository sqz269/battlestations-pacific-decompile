#pragma once

#include "bsp/game_native_string_process.hpp"
#include "bsp/native_scene_property_insertion.hpp"

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Native typed-property providers require MSVC Win32.
#endif

namespace bsp {

// Three whole ordinary bodies with NEW C++ interfaces. Names are descriptive
// hypotheses, not recovered symbols/classes or Original binary replacements.
// Genuine raw producers/current-CRT owners are required; a CE89D4 phase word
// alone is neither provenance nor a callable Source vtable.
//
// Whole [008F3370,008F33B8),72B/25 instructions. Original ECX actual38h record,
// stack CString, EAX same record, RET4. Read tag+4 once. For tag2, free nonnull
// owned+C, physically clear+C AFTER return, then call the genuine raw nullable
// duplicate00438E40 and store its result. Other tags read the genuine enum
// declaration at+28, call known-found0048E840, and store ALL32 result bits+C.
// No extra tag4 guard, previous-value free on that branch, or absence default.
//
// Tag2+C is null or a unique same-current-CRT owned allocation. Nullable text
// is valid ONLY on tag2; otherwise require a stable closed ASCII CString whose
// key is known-found in the genuine live declaration/map. The raw type4
// constructor's opaque+28 bits alone do not establish that provenance. Retain
// the SAME canonical string process/raw-key generation and distinct E186ED /
// E17654 authorities. Token, record and providers remain valid across frees;
// no old-owned-string/input alias, reentry, overflow, mutation, fault or throw.
void* assign_native_scene_property_from_token_008f3370(void* actual_record,
    const char* actual_token, game::GameNativeStringProcess& actual_strings);

// Whole [008F3490,008F3504) and [008F3510,008F3584), each116B/36 instructions.
// Original ECX genuine raw61-produced114h bag; stack key,data,count,FULL DWORD
// copy flags; RET10h; final insertion result is EAX=bag. Allocate exactly38h
// uninitialized bytes in the canonical current heap, call the actual raw8/9
// constructor, then whole008F33F0 with the SAME insertion context. Byte ctor
// order is data/count/flags; four-byte ctor order is count/data/flags. Preserve
// every flag bit; only the LOWER provider tests the low byte. There is no
// floating operation in the four-byte array wrapper.
//
// Type8 requires a positive nonwrapping byte span. Type9 requires count in
// [1,3FFFFFFF] and exactly4*count readable bytes. Root/input/frame are disjoint.
// Low flag byte nonzero copies into a fresh current-CRT child; input remains
// caller-owned. Low byte zero ONLY retains the exact input: for this owning
// publication, input must already be a unique genuine same-current-CRT owned
// allocation. Caller manages it through construction, then transfers that
// allocation with the record on successful publication. Never publish a
// borrowed/stack/static/foreign/shared span into this owning graph. The raw
// constructor itself does not create that ownership transfer.
//
// Retain the existing insertion context's SAME canonical string/physical
// processes, initialized E175B0/E188B4 membership, E177E4/E17654 roles and live
// return gates. Explicit physical-pool startup precedes that context. There is
// no explicit process/physical-pool/application startup here; raw-string
// getters retain their accepted lazy-pool behavior. Incoming and replaced graphs/key/map/node
// are disjoint, finite, unshared and acyclic. Retire payloads/records/keys/nodes
// before pool teardown; externally serialize common-list mutations.
//
// Successful nonthrowing providers only, C locale, stable closed ASCII keys,
// no alias, zero-size, overflow, reentry/concurrent mutation, trim or fault.
// Original null allocation still passes null to insertion; no recovery is
// invented. The ordinary domain excludes it. Original EH frees the saved
// allocation while construction is active; this new Source interface supplies
// no exceptional cleanup/rollback, Original ECX/RET/register/FS/EH/class ABI,
// private CRT/FP/error-state, loader/startup or game equivalence.
void* add_native_scene_property_byte_array_008f3490(void* actual_bag,
    const char* key, const void* data, std::uint32_t byte_count,
    std::uint32_t copy_flag_bits, NativeScenePropertyInsertionContext& context);

void* add_native_scene_property_four_byte_array_008f3510(void* actual_bag,
    const char* key, const void* data, std::uint32_t element_count,
    std::uint32_t copy_flag_bits, NativeScenePropertyInsertionContext& context);

} // namespace bsp
