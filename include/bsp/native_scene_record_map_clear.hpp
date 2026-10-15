#pragma once

#include "bsp/native_string.hpp"

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Raw scene-map cleanup requires MSVC Win32.
#endif

namespace bsp {

// Complete0046B460. Native ECX actual tree; RET/no semantic result. Destroy
// current root before resetting head.parent/count/head.left/head.right, retaining
// the original head captures and reloads. Tree+0 and mapped values are untouched.
void clear_native_scene_record_map_0046b460(void* actual_tree,
    NativeStringRawPoolContext&);

// Complete0046B680. Native ECX database, RET/no semantic result. Tree is db+18h.
// Zero count skips all work. Otherwise validate/visit current iterator, capture
// mapped+14h, zero that slot BEFORE its current vtable slot0 deleting call(flag1),
// then increment the actual iterator. Finally destroy nodes and reset head/count.
// On reached nonzero-count work, actual_iterator8 provides stable writable
// backing for native private
// {owner,node} cells. It is written NODE then OWNER on entry, read after each
// increment; payload aliases are admitted, original physical-stack aliases are
// unproved. No initial iterator contents are read. It must remain valid through
// reached handlers, virtual calls and cleanup; zero count does not inspect it.
void clear_native_scene_database_hidden_records_0046b680(void* actual_database,
    void* actual_iterator8, NativeStringRawPoolContext&);

// Borrow valid reached native-layout storage and matching current-CRT string
// allocation domains. Object vtables must contain actual callable Win32
// __thiscall deleting-destructor entries. Current invalid-parameter handlers may
// return; all continuation branches remain. No ownership adapter or callback is
// introduced. New Source context/frame interfaces do not establish original
// private-frame/CRT/binary ABI, application adoption or game behavior.
} // namespace bsp
