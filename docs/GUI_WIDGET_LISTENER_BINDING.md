# Canonical base GUI listener

Addresses: 004fa100, 004fa110, 004fa120, 004fa130, 004fa140, 004fa150,
004fa160, 005902e0, 005861b0, 00aa6bc0.

The seven entries in table 00ceb110 are actual empty native methods. Slots 00
through 14 return with RET4; slot 18 returns with RET8. Their complete three-byte
bodies were verified against installed bytes and defined in the existing BSP
Ghidra program. These are recovered empty bodies, not unresolved-call stubs.

`GuiBaseWidgetListener` implements the five callbacks exposed by the canonical
frame-listener interface (04, 0C, 10, 14 and 18). It inherits the existing listener
identity and binding behavior. Slots 00 and 08 are analyzed only because the
current frame interface does not expose them. This C++ interface is not the
original listener vtable or calling convention.

The existing AA6BC0 binder writes the same widget's DC listener and forwards the
existing child binding. Main-menu construction first installs CEB110 at screen+40
(590320), then the derived CEFC04 table (59033A). The existing 5861B0 layout binder
passes that same screen+40 identity. No second screen, widget registry, or copied
listener state is introduced here. Callers must keep the listener and bound
storage alive across callbacks and obey the existing frame ownership guards.

The derived main-menu table is not entirely empty: current04 is 005993a0,
current0C is 00581970 and current18 is 00581b20. See
[MAIN_MENU_WIDGET_LISTENER.md](MAIN_MENU_WIDGET_LISTENER.md). The main-menu adapter
requires its unresolved command handler rather than inheriting the base empty04.

Validation: combined MSVC Win32 build and both existing tests passed. The local
menu fixture exercises real canonical Group/frame owners and the existing screen
and scroller storage. It does not instantiate a complete main-menu listener or
validate a running game. Definition evidence is retained in
`reports/gui_widget_listener_binding_definitions.json`; source/report hashes and
the final combined evidence are in `reports/orch5_texture_icon_listener_batch.json`.

## Correction from docs/GUI_POINTER_LISTENER_RUNTIME.md

Canonical GuiBaseWidgetListener now supplies current00 (4FA100) and current08
(4FA120) as well. Both verified native bodies are RET4, and both CEB110 and
CEFC04 point to them. AA7190 invokes fresh listener00 before looking up its
mouse and fresh listener04/08 on captured mouse rising button histories.
