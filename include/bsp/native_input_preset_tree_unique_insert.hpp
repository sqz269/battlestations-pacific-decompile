#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

#include <cstddef>
#include <cstdint>
#include <type_traits>

namespace bsp {
class NativeCrtStricmpCallees;

// Measured actual12B result carrier, not a node/container overlay. Caller owns
// its storage/lifetime. Owned publication stores touch NODE4, BYTEflag8, OWNER0
// only; they do not write9..B. Transitive child/provider writes and alias effects
// remain inherited, with no whole-call entry-preimage guarantee for9..B.
// No value initialization or snapshot.
struct NativeInputPresetTreeInsertResult {
    NativeInputPresetTreeIterator iterator_00;
    std::uint8_t inserted_08;
    std::uint8_t untouched_padding_09[3];
};
static_assert(sizeof(NativeInputPresetTreeInsertResult) == 12);
static_assert(alignof(NativeInputPresetTreeInsertResult) == 4);
static_assert(offsetof(NativeInputPresetTreeInsertResult, iterator_00) == 0);
static_assert(offsetof(NativeInputPresetTreeInsertResult, iterator_00)
    + offsetof(NativeInputPresetTreeIterator, owner) == 0);
static_assert(offsetof(NativeInputPresetTreeInsertResult, iterator_00)
    + offsetof(NativeInputPresetTreeIterator, node) == 4);
static_assert(offsetof(NativeInputPresetTreeInsertResult, inserted_08) == 8);
static_assert(offsetof(NativeInputPresetTreeInsertResult, untouched_padding_09) == 9);
static_assert(std::is_trivially_copyable_v<NativeInputPresetTreeInsertResult>);
static_assert(std::is_standard_layout_v<NativeInputPresetTreeInsertResult>);

// Whole006A1050..006A1163:276B/111starts. Preserve established descriptive
// STL_Tree_InsertUnique_StrKeyCI_N2C_006a1050 identity. NativeECXtree, incoming
// EDXunconsumed, stack(out12,pair28), EAXactualout, RET8. Source has six stack
// words/RET18h with borrowed RawCtx, actualfeature/locale cells and EXISTING
// CRTparent provided callees. This is a new Source ABI, not a binary replacement.
NativeInputPresetTreeInsertResult* __fastcall
insert_unique_native_input_preset_tree_006a1050(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeInsertResult* actual_output, const void* actual_pair,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees);

// Fixed nil29/current links0/4/8 and keylengthC/data10. DirectBF7FBF captures
// NODEdata before KEYdata, but compares KEY,NODE; final supplied443D00 compares
// selectednode+C,pair with the SAMEactualcell/callees. No CurrentCRT fallback,
// default cells/services or ASCII-only substitution. Originalcandidate remains
// separate from selectedpredecessor; actual69AEC0 may return from its currentSDK
// diagnostics. Both FBC0 calls use SAMERawCtx/feature and only defined direction
// LOWBYTE; no upper-privateflag preimage. Successful publication captures child
// OWNER, currentouterOUT, childNODE before any output store. No localEH, guard,
// cleanup, validation, count/topology operation, repair or rollback is added.
// Raw reached storage must permit ordered volatile Win32 DWORD/BYTE accesses.
// ExistingCRT provided-callee/SDK/heap/locale/TLS/errno/feature/DF0/lifetime and
// FBC0 CurrentCpp owningthrow constraints remain. SourcecompilerEBP/private
// arguments/frame aliases, flags, fault/partialwrite, OriginalBF6885 returning
// import continuation/FH3SEH/RTTI/binaryABI/runtime/game identity are unproved.
} // namespace bsp
