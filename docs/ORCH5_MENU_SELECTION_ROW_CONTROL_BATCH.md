# Menu selection, row control and canonical listeners

Addresses: `005966F0`, `00584750`, `004260B0`, `004263B0`, `005C35D0`,
`00B67690`, `00B65FB0`, `00B66250`, `00A9B340`, `00A9BA40`, `00A9BA90`,
`00A9CD20`, `00A9DA30`, `00AB6BD0`, `00A9E120`, `00AAA3E0`.

This batch reconstructs the normal menu Listbox selection listener and vehicle
unlock caller, then connects established operations through the existing owners.
The selection callback reads the screen's freshly current main Listbox; both
native callback arguments are unused. Mission and objective pages retain the
actual Text/Icon operations and required unresolved resource/page providers.
The unlock routine uses actual Lua 5.1.1 objects and native string lifetimes,
then reloads the current profile after the second string allocation.

Listbox current60 now calls base activation first and walks actual FC rows when
clearing active with live auto-control enabled. It reads current selection before
each state1/state3 dispatch and advances the current iterator after callbacks.
BA90 walks the canonical borrowed Group child list, preserving duplicate entries.
Its supported callback domain preserves visited-prefix membership, order and
owners; unvisited suffix and appends may change. This remains a partial projection
of native list-node behavior. Text globals.live loads D7A24C once for all four
color channels; Icon uses native literal FLD1. No fallback constant is supplied.
The callback-free BA40 predicate tests row77 only, without effective visibility.

The D8 producer stores the actual row field before the established append path.
Only native null-position insertion is implemented. Non-null A9D750 forwards one
iterator to the distinct GUI+64 and FC lists. Its actual caller/node relationship
must be established before implementing before/after insertion semantics.

Host listener registries connect current Listbox114 to the actual selection
implementation and the same screen's separate widget listener identity. Combined
registration binds Listbox first so rollback stays valid during another active
frame callback. Canonical layout calls reach actual Listbox and Text owners.
Seven command services now use existing profile, mission and tactical-library
storage. Text height exposes the existing lifetime/font implementation. Native
Listbox table slots64/70 also enable existing bounds and clip providers; full
Listbox current40 remains unavailable.

Evidence and final validation are recorded in
`reports/orch5_menu_selection_row_control_batch.json`, the three parent reports,
and worker docs `MAIN_MENU_SELECTION_LISTENER.md`, `MAIN_MENU_VEHICLE_UNLOCK.md`
and `GUI_LISTBOX_ROW_CONTROL.md`. Descriptive names are hypotheses. These are
C++ interfaces with explicit ownership contracts, not original ABI replacements.
No full menu construction, Text/Icon resource effect, rendering or gameplay
validation is claimed. Remaining work includes concrete page/checkpoint/texture,
transform/prompt/Movie providers and Listbox properties, full frame and teardown.

## Initial combined validation

MSVC Win32 build passed both existing tests. Three focused fixtures linked only their fixture object against the combined production library and passed: actual Listbox/Group/dispatcher ownership, actual Lua/profile unlock sequencing, and NativeString contents/allocation order. The call reports contain245 rows with0failures, and16 annotations were saved with prior names/comments retained. The report hashes sources, library and logs and records narrower fixture limits.
