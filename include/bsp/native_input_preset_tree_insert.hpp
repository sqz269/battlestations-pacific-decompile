#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset-tree insertion requires MSVC Win32.
#endif

namespace bsp {

// Whole0069FBC0..0069FDAB:492B/167 instructions. Preserve established
// STL_Tree_Insert_StrKeyCI_N2C_0069fbc0 identity; Source name is a hypothesis.
// Native ECXtree/EDXunconsumed, stack(out8,directionword,parent,pair28),
// RET10h/EAXactualout. Source adds borrowed actualRawCtx/109EEA4ref:
// six stack words/RET18h, a new Source ABI. Only direction LOWBYTE is read.
// Reuse the actual trivial8B owner/node carrier, no node/container overlay.
NativeInputPresetTreeIterator* __fastcall
insert_native_input_preset_tree_node_0069fbc0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeIterator* actual_output,
    std::uint32_t insert_left_word, void* actual_parent,
    const void* actual_pair, NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4);

// Logical8B/2-instruction completed-message action00C7EFE0. Original
// runtimeEH EBP-50h is not a Source argument/frame. Borrow the completed
// real1Ch SBO storage directly; no RawCtx, EBP reader or normal key cleanup.
void __fastcall cleanup_native_input_preset_insert_message_00c7efe0(
    NativeLegacySboStringStorage& completed_message) noexcept;

// Fixed treehead4/count8; links0/4/8;color28/nil29. Unsignedcount>=09249248
// uses actual19B SBO assignment before arming completed-message cleanup, then
// the published owning NativeHardwareLayoutTreeLengthError transport.
// Source preserves CurrentCpp storage/cleanup, not OriginalD83F98 throw ABI.
// OriginalBF6885 has physicalRET8 if its RaiseException import returns;
// the CurrentCpp throw path does not implement that returning continuation.
// Normalpath actualF5 receives the SAME RawCtx/realfeature reference. Capture
// currentpostfactoryhead before currentcount RMW; preserve ordered root/head,
// parent/uncle/slot reloads and three actualrotation calls plus ownedinlineleft.
// Keep ORIGINALnewnode distinct from mutablefixupnode; rootblack precedes
// currentprivateout capture; publish node BEFORE owner. Publicraw accesses and
// reached Sourceprivateargument reads are volatile. No guard/free/rollback is
// added to normal insertion; existing AV/typed providers remain independent.
// Valid reached storage/finite loops and actual provider/feature lifetimes are
// required. Sourceprivateframes/argument aliases, CurrentCRT allocation/copy,
// exception copy/elision/RTTI, Originalheap/DF/flags/FH3SEH/fault/partialwrite,
// binary ABI/runtime/game identity remain unproved.
} // namespace bsp
