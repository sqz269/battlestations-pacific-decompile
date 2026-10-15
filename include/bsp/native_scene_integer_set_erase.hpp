#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene integer-set erase requires MSVC Win32.
#endif

namespace bsp {

// Actual native entry+4/+8/+C output/owner/node argument cells and entry-54h
// saved-NODE DWORD. Bindings/metadata are stable and disjoint from described
// cells/payload. Saved-node backing is required only after the nonnil path
// is reached. Actual described cells/payload can alias where all accesses and
// releases remain valid; Source/private-frame coincidences are not implicit.
struct NativeSceneIntegerSetEraseFrame {
    void* actual_parameters12;
    void* actual_saved_node4;
};

// Whole0046A740..0046A9E6 and completed-message Source unwind00C61C90.
// Native ECX tree;stack output,owner,node;RET0Ch/EAX current output.
// Source's explicit frame is a new interface. Initial nil throws the existing
// actual owning D6926C payload before saved-node access. Otherwise capture
// initial node, publish saved-node, increment actual owner/node and splice.
// Reread saved-node separately for minimum,maximum,color and final actual free;
// keep captured near-child pointers through repair. Captured tree stays fixed.
// After free read current count;read owner,output,node before ordered stores.
void* erase_native_scene_integer_set_iterator_0046a740(void* actual_tree,
    const NativeSceneIntegerSetEraseFrame&);

// Complete returning current-CRT diagnostic paths compose through increment.
// No string/pool/key destructor,callback,rollback or success-only guard is added.
// Current CRT heap and owning C++ transport are concrete; original RTTI/base
// catches/FH3/SEH/ABI/flags/register/byte identity and game validation are not.
} // namespace bsp
