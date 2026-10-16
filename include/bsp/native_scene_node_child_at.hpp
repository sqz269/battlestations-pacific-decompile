#pragma once

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Actual scene-node child lookup requires MSVC Win32.
#endif

namespace bsp {

// Complete00467F30 against actual list/index storage. Native ECX=list,
// stacked signed index,RET4/rawEAX. The ordinary API explicitly binds that
// index word; it is not a native stack/register/private-frame replacement.
// Read current index FIRST. Negative skips every list access. Otherwise read
// current signed count+8;count<=index skips head. Read current head+0,then
// follow current node+44h links index times,stopping on null. Count is captured
// once;no nominal node/list type,Host,std::function,SDK or provider is used.
//
// Binding addresses remain stable. Source metadata is disjoint from actual
// cells and reached payload. Reached payload aliases are admitted;require only reached
// accesses valid. No whole-list validity,alignment/order guard,ownership,
// lifetime,cycle check or implicit compiler/helper/argument-home alias is added.
// Source receiver/cell interface and ordinary raw accesses give no Original
// ABI,fault timing,flags/register/private-frame or game execution proof.
// Evidence: docs/GOAL_OCT15_SCENE_NODE_CHILD_AT.md.
void* child_at_native_scene_node_list_00467f30(void* actual_list,
    const void* actual_index4);

} // namespace bsp
