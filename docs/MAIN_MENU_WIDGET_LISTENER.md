# Main-menu widget listener callbacks

Addresses: 00581970, 00581b20, 005902e0, 005993a0, 005861b0, 00acf070.

This packet reconstructs the normal current0C and current18 bodies over the
existing canonical widget profiles. Names describe table slots and are hypotheses,
not recovered mouse-event names. Native ECX is main-menu screen+40; the widget is
on the stack, with a second Boolean argument for current18. The functions end at
581B16/581CD3 with RET4/RET8. The derived listener table is CEFC04, installed at
59033A: 4FA100, 5993A0, 4FA120, 581970, 4FA140, 4FA150, 581B20.

The bindings reference the existing layout, screen, two scrollers, arrow-enable
bytes, zoom accumulator and live page/alpha globals. Native listener-relative
offsets are adjusted by 40h. In particular 194/314 are the existing screen
scrollers 1D4/354; 1DC/35C are their dragging bytes; 528/52C are the existing
background widget 568 and screen input gate 56C. The zoom accumulator at screen
19C is a required reference, not newly initialized or copied screen state.

For Text, both handlers compare the low byte of the current UTF-16 first code
unit. The installed E19508 empty-string sentinel begins with zero. Current0C
handles B4/B6, clears the same zoom accumulator to positive zero on pages 4..8,
and selects live alpha according to current D4. Current18 handles A7/A5/A3/A2/B4/B6
and selects alpha from the incoming Boolean. Both preserve the outgoing x87
float spill before actual Text current4C.

For other widgets, pages 4..8 use the existing mission scroller and page9 uses the
briefing scroller. Current0C stops the selected scroller or ends its thumb drag.
Current18 respects arrow-enable and active-thumb guards, and the background
updates the same screen+56C input gate. Branch priority remains significant if
multiple layout fields refer to the same widget. After these operations the
handler invokes actual Icon current88 or FrameBox current84. It preserves the
second current5C call on the FrameBox path. Current0C reloads D4 after those type
callbacks. The FrameBox adapter uses its existing state, layout and services.

`MainMenuWidgetListener` implements this handler pair and inherits the proven
empty10/14 methods. It remains abstract at current04: the 5993A0 listbox, prompt,
mission and screen-service command chain is analyzed but not reconstructed here.
This prevents binding an incomplete menu as though commands were implemented.
The complete screen provider and original vtable/string/SEH ABI remain open.

Validation: independent source/listing review passed after correcting the D4
reload position. Sixteen numeric call rows were recorded (four direct and twelve
resolved indirect); dispatch-table evidence remains necessary for indirect calls.
The combined Win32 build and existing tests passed. One local fixture linked the
combined library and checked actual Group/frame owners, mission/briefing scroller
selection, alias-sensitive branch priority and the same 56C input gate. It does
not exercise full resource-backed Text/Icon/FrameBox dispatch, a concrete5993A0
handler, original native bytes, rendering or gameplay.

Follow-up packet: reconstruct 005993a0 only after identifying the actual listbox,
prompt, selected-mission and screen-service owners; preserve its same screen
bindings and every native call contract. A runnable menu also needs the actual
screen constructor/provider configuration and resource-backed validation.

## Correction from docs/ORCH5_MENU_LISTBOX_GROUP_BATCH.md

The normal5993A0 current04 caller now has a reconstructed function over the same canonical owners, reviewed twice against assembly. The original callback interface still needs actual menu/listbox listener wiring and concrete page/profile providers;5966F0 is the distinct Listbox current08 target. See MAIN_MENU_COMMAND_LISTENER.md and reports/orch5_menu_listbox_group_batch.json; no complete menu/game claim.
