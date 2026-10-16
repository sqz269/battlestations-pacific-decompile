#pragma once

#include "bsp/native_input_preset_tree_erase.hpp"

namespace bsp {
class NativeCrtStricmpCallees;

// Whole 006A15E0..006A17DB, 508B/210 Native instructions. Descriptive Source
// name; retain STL_Tree_InsertHint_StrKeyCI_N2C_006a15e0 in the Original.
// Native ECX tree/dead EDX, stack(out8,hint.owner,hint.node,pair), RET10h/EAXout.
// Source adds four borrowed actual inputs: eight stack DWORDs/RET20h. This is
// a new C++ interface, not the Original register/private-frame binary ABI.
NativeInputPresetTreeIterator* __fastcall
insert_hint_native_input_preset_tree_006a15e0(
    void* actual_tree, std::uint32_t unused_edx,
    NativeInputPresetTreeIterator* actual_output,
    NativeInputPresetTreeIterator actual_hint, const void* actual_pair,
    NativeStringRawPoolContext& strings,
    const volatile std::uint32_t& actual_feature_word_0109eea4,
    const volatile std::int32_t& actual_locale_changed_0109de1c,
    NativeCrtStricmpCallees& actual_callees);

// Count-first empty arm; fixed nil29 and current raw DWORD/BYTE reads. Returning
// SDK diagnostics do not revalidate retained owners/nodes/heads. Seven FBC0
// paths ignore child EAX and return their retained output. Unique fallback reads
// child OWNER, current OUT, stores OWNER, then freshly reads/stores child NODE.
// Its real 12B result flag/padding is ignored; no added preimage input or snapshot.
// Transitive child/provider writes and overlapping aliases remain inherited;
// this does not promise whole-call output/padding entry preimages. No new guard,
// owner, cleanup, rollback, catch, noexcept policy, adapter or default service.
// Reached storage must admit ordered volatile Win32 accesses and finite child
// walks. Existing RawCtx/feature/CRT-parent-provided-callee/SDK/CurrentCpp owning
// exception contracts remain. Compiler private locals/predicates/argument reads,
// frame aliases, flags/registers, faults/partial writes, Original FH3/SEH/RTTI,
// heap/locale/TLS/DF/runtime/game identity are unproved. Map caller is separate.
} // namespace bsp
