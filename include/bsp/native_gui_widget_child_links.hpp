#pragma once
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_string.hpp"

namespace bsp {

// AA2490: ECX actual8h string, EAX type ID, RET. Capture data before the
// existing raw reverse search. The two borrowed native literals must remain
// live; normal original contents are empty and "_". Comparisons use the current
// CRT _stricmp and the exact seventeen native ASCII class names in order.
// This does not make a string_view or copy the counted header/data.
std::uint32_t native_gui_widget_type_for_key_00aa2490(const NativeString&,
    const char* actual_empty_00f8bc60, const char* actual_underscore_00ce7890);

// A9B790: unused ECX, three stack DWORDs, RET0C. Instruction-equivalent alias
// of existing8665F0: allocate0Ch, publish next/previous, then read source cell.
NativeEffectDeletionNode* create_native_gui_widget_list_node_00a9b790(
    NativeEffectDeletionNode* next, NativeEffectDeletionNode* previous,
    const void* actual_source_pointer_cell);

// A9D480: ECX actual0Ch list, stack increment, RET4. Existing8675E0 alias;
// unsigned 3FFFFFFF-count predicate and captured-count store are identical.
// Reuses the existing owning host length-error transport, not native FH3.
void grow_native_gui_widget_list_count_00a9d480(
    NativeEffectDeletionListStorage&, std::uint32_t increment);

// A9BD50: ECX actual list, stack pointer to payload DWORD, RET4. Capture the
// payload and initial end once; remove every matching node, never the payload.
// Header+0/payload objects stay untouched. Valid retained finite rings with
// matching allocation/free domain only; native invalid-parameter ABI excluded.
void remove_native_gui_widget_list_value_00a9bd50(
    NativeEffectDeletionListStorage&, const void* actual_payload_pointer_cell);

// AA83A0: ECX actual parent widget, stack actual child widget, RET4. Null child
// returns before accessing parent. Each CURRENT node address is resolved to
// its existing canonical scene companion; none is fabricated or cast from raw.
// Callbacks may replace widget+4C or its list before the next native reread.
// When the first child+4C gate passes, later required node pointers must remain
// valid and bound. No logical GuiWidgetOwner, outer ABI or game claim.
void detach_native_gui_widget_child_00aa83a0(void* actual_parent,
    void* actual_child, NativeNodeDestructionRuntime&);

} // namespace bsp
