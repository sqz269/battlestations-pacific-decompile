#pragma once
#include "bsp/native_live_effect_manager.hpp"
#include "bsp/native_node_destruction.hpp"
#include "bsp/native_string.hpp"
#include <array>

namespace bsp {

// AA2490: ECX actual8h string, EAX type ID, RET. Capture data before the
// existing raw reverse search. The two borrowed native literals must remain
// live; normal original contents are empty and "_". Comparisons use the current
// CRT _stricmp and the exact seventeen native ASCII class names in order.
// This does not make a string_view or copy the counted header/data.
std::uint32_t native_gui_widget_type_for_key_00aa2490(const NativeString&,
    const char* actual_empty_00f8bc60, const char* actual_underscore_00ce7890);

using NativeGuiWidgetTypeLiteralViews = std::array<const char*, 17>;
using NativeGuiWidgetTypeNameCompare = int (__cdecl*)(const char*, const char*);

// Explicit AA2490 inputs: borrow actual empty/underscore bytes and class literal
// identities in native order: Icon, Text, Group, Progbar, Line, Scrollbar, Grid,
// Model, Movie, Listbox, AnimIcon, Curve, Sound, Button, ClipBox, Section,
// FrameBox. IDs are fixed by the body, not caller data. No strings or headers
// are copied and no globals, locale/error owner or default comparer is supplied.
// Construction preflights nonnull pointers outside the raw operation. Caller
// keeps this context, the SAME array/pointers/terminated bytes and key backing
// live/stable through serialized comparisons. A supplied owning-UCRT comparer
// is a Source substitution, not proof of Original BF7FBF CRT ownership/ABI.
class NativeGuiWidgetTypeKeyContext {
public:
    NativeGuiWidgetTypeKeyContext(const char* actual_empty_00f8bc60,
        const char* actual_underscore_00ce7890,
        const NativeGuiWidgetTypeLiteralViews& actual_class_literals,
        NativeGuiWidgetTypeNameCompare required_comparer);
private:
    const char* empty_;
    const char* underscore_;
    const NativeGuiWidgetTypeLiteralViews& class_literals_;
    NativeGuiWidgetTypeNameCompare compare_;
    friend std::uint32_t native_gui_widget_type_for_key_00aa2490(
        const NativeString&, const NativeGuiWidgetTypeKeyContext&);
    friend std::uint32_t native_gui_widget_type_for_key_00aa2490(
        const void*, const NativeGuiWidgetTypeKeyContext&);
};

// Same actual key/header and real raw004BCB80; capture data before search and
// retain that SAME suffix pointer for the ordered short-circuit comparisons.
// Valid current inputs only; ordinary Source provider failures propagate, with
// no new cleanup or native EH/SEH/private-stack/binary-ABI equivalence claim.
std::uint32_t native_gui_widget_type_for_key_00aa2490(const NativeString&,
    const NativeGuiWidgetTypeKeyContext&);
// SAME actual8B header/source body without materializing a NativeString in
// raw scratch. Data is captured before the raw4BCB80 search, as Original does.
std::uint32_t native_gui_widget_type_for_key_00aa2490(const void* actual_header8,
    const NativeGuiWidgetTypeKeyContext&);

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
