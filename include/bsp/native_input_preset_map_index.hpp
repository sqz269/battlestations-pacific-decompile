#pragma once

#include <cstdint>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual preset map indexing requires MSVC Win32.
#endif

namespace bsp {
struct NativeStringRawPoolContext;
class NativeCrtStricmpCallees;

// Whole006A1E70..006A1F4A:219B/75 Native instructions. Preserve the established
// STL_Tree_MapIndex_StrKeyCI_N2C_006a1e70 identity and the old four-argument
// NativeStringStorage/AV overload. Native ECX tree, EDX unconsumed, one stacked
// key-header pointer, RET4/EAX retained-node+14h. This new overload adds the
// required private fifth-word preimage and actual borrowed providers/cells:
// six stack DWORDs/RET18h, a qualified C++ interface, not Original binary ABI.
void* __fastcall index_native_input_preset_tree_006a1e70(
    void* actual_tree, std::uint32_t unused_edx, const void* actual_key_header,
    std::uint32_t default_flag_word_preimage,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees);

// Logical completed-key effect of C7F100[8B/2], preserving Unwind@00c7f100.
// Actual completed private pair begins with its LENGTH0/DATA4 key header.
// Delegate the published raw69E090 release using this same RawCtx. No EBP
// reader, fake parent frame, mapped-value destructor or Original action ABI.
void __fastcall cleanup_native_input_preset_map_completed_key_00c7f100(
    void* actual_completed_key, NativeStringRawPoolContext& strings);

// Miss mapped words are FFFFFFFF,0,0,0,(preimage&FFFFFF00); no indeterminate
// C++ DWORD read or implicit choice of the upper24 bits. Construct actual28B
// private pair before arming cleanup. Pass its ACTUAL constructor result to
// whole hinted insertion; read returned OWNER then NODE. Capture temp DATA
// while armed; disarm/end EH scope before normal CURRENT length+1/getter/return.
// Normal cleanup uses captured DATA; unwind69E090 rereads the actual header.
// Final diagnostics use the retained returned owner and CURRENT head; returning
// handlers do not repair/revalidate the retained node, which still yields+14h.
// Guard cleanup may propagate C++ exceptions; a second exception during C++
// unwinding retains the language termination boundary, not Original FH3/SEH.
// RawCtx/currentCRT, provided CRT parent/comparator, realfeature/locale/SDK,
// private frames/preimages/aliases/flags/faults/OriginalABI/runtime/game remain
// qualified. No host map/node model, default provider, rollback or retry.
} // namespace bsp
