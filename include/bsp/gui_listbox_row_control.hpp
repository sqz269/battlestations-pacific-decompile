#pragma once
#include <cstdint>

namespace bsp {
class GuiWidgetOwner;
class GuiListboxRuntime;

// Complete normal-body00A9B340: ECX unused; actual row and signed state DWORD
// stack, RET8. Calls the current actual type5C twice on the non-Text path.
// Text uses its canonical current80, except UTF-16 lengthEC==1 plus the live
// cached sourceF4 equal to globals.live: current50 receives four copies of ONE
// live D7A24C load. Icon current88 receives low16(state),0,literal FLD1.
// Other current types are inert. A required actual Text/Icon companion must
// exist; no synthetic provider or state is substituted for missing behavior.
// one may be null only when the globals.live branch is not reached.
void apply_gui_listbox_widget_state_00a9b340(GuiWidgetOwner&, std::int32_t state,
    const volatile float* one_00d7a24c);

// Partial callback-domain projection00A9BA90: native ECX Listbox, checked FC iterator
// {container,node} and state DWORD stack, RETC. Caller dereferences its SAME
// canonical FC node and supplies that actual borrowed row here. Current Group2
// visits its SAME borrowed transform.children GUI+64 list in live order,
// including duplicates; owning layout.children is not that native list.
// Nested Groups are not recursed.
// Row/group/current child and their actual owner domain must survive callbacks.
// The prefix through the current ordinal must retain membership/order across
// callbacks; unvisited suffix and appends may change. Current-ordinal checks
// cannot prove native list-node identity, especially for equal-pointer entries.
// Earlier-prefix removal/reordering and native node identity are not modelled.
void apply_gui_listbox_row_state_00a9ba90(GuiWidgetOwner& actual_row,
    std::int32_t state, const volatile float* one_00d7a24c);

// Complete valid FC-list00A9BA40..00A9BA8D: ECX Listbox; AL Boolean; RET.
// Walks the SAME canonical FC rows; true at the first row whose actual77 is
// zero. Empty/all-hidden false. No current38 call or ancestor visibility test.
bool has_selectable_gui_listbox_row_00a9ba40(const GuiListboxRuntime&) noexcept;
} // namespace bsp
