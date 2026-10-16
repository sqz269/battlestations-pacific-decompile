#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene-command record construction requires MSVC Win32.
#endif

#include "bsp/native_string.hpp"

namespace bsp {

// Bind the native live argument words {owner, command, target} and saved-record
// word. At entry ESP=P they occupy distinct ranges P+4..P+0F and P-10..P-0D.
// The two bound addresses stay stable. Source frame/context metadata are
// disjoint from payload and all actual backing cells. Payload aliases with the argument, saved
// word and record regions remain admitted when every reached access and the
// existing raw providers are valid. No original helper/child/compiler stack,
// spill, return, FH3, register-save or asynchronous alias identity is claimed.
struct NativeSceneCommandRecordConstructFrame {
    const void* actual_parameters12;
    void* actual_saved_record4;
};

// Qualified complete cleanup-action mappings. Each independently reads the
// CURRENT saved-record word, adds its member offset with DWORD wrap and uses
// the real raw 0041DD20 destructor. These are ordinary explicit-cell interfaces,
// not callable replacements for the original EBP-frame tail entries.
void unwind_native_scene_command_record_command_00c61a70(
    const void* actual_saved_record4, NativeStringRawPoolContext& strings);
void unwind_native_scene_command_record_target_00c61a7b(
    const void* actual_saved_record4, NativeStringRawPoolContext& strings);

// Whole 004690D0: native ECX captures record20, stack owner/command/target,
// RET0Ch, EAX original record. Save record first, zero both actual8h headers,
// capture command BEFORE owner, then store owner. Inline byte walks and the
// complete raw 0041DD40 preserve0 provider precede each conditional genuine
// current-SDK memmove, using current destination data/length. Target is read
// only AFTER the first resize/copy. No caller preinitialization, normal cleanup,
// global empty-string fallback, snapshot owner or new callback is introduced.
//
// On ordinary C++ unwind, state1 releases current saved-record+0Ch and state0
// then independently reloads saved-record for +4. No native FH3/SEH, secondary
// cleanup exception, fault timing, private-frame, CRT/heap or binary identity
// is inferred. Native calls BF7680; the concrete overlap/zero-count boundary
// here is the actual current SDK memmove, not an invented unresolved provider.
// Evidence: docs/GOAL_OCT15_SCENE_COMMAND_RECORD_CONSTRUCT.md.
void* construct_native_scene_command_record_004690d0(void* actual_record20,
    const NativeSceneCommandRecordConstructFrame& frame,
    NativeStringRawPoolContext& strings);

} // namespace bsp
