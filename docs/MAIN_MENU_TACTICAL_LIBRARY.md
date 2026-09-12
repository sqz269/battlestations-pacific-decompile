# Tactical-library request ordering

Addresses: `005885D0`, `005886C0`; reached existing dependencies `005806A0`,
`005C27E0`, `004CC460`.

The new routines use the same existing `MainMenuManager`, mission tables,
interface lock and payload ownership. Required pure accessors expose borrowed
fields of the actual screens in manager slots1 (`+5C`) and6 (`+70`); they do not
allocate, callback or synthesize a screen. All borrowed owners remain alive.

`5885D0` independently looks up the selected mission twice. The first lookup
supplies the side index; the second supplies the mission pointer. It writes the
second pointer at library `+9C`, the first side at `+A0`, and zero at `+A4`, then
reloads the current library to write mode5 and selector99. Only then does it
reload the manager and push request `0B` with null payload.

`5886C0` pushes the request first. Payload release can rebind the global manager,
so its subsequent mode4/selector99 stores use the newly current library screen.
Selection fields are preserved. The previous `TacticalLibraryRequest` value
summary had reversed `+9C/+A0` comments and implied both functions requested
first; those comments are corrected. That value API alone proves no ordering.

Native entry uses ECX caller screen, no stack arguments and RET. Actual selected
indices are `E194D8/E194DC`, not the other menu globals `E08870/E08878`. `5806A0`
reads current manager `+5C` mission tables. Its invalid STL-iterator path remains
outside the supported domain. The analyzed `5098B0` TacticalLibrary constructor
does not establish these fields in its inspected body; no defaults are invented.

The focused local fixture uses existing real manager/table/payload objects and
borrowed library-field storage. It observes selection writes before payload
release and rebinds the manager during release to verify the post-request reload.
It passed together with the initial combined Win32 build (2/2 existing tests).
Final combined validation is tracked by `reports/orch5_menu_listbox_group_batch.json`.
This validates C++ storage/order, not extracted native bytes, the full screen,
binary-compatible layouts or gameplay. See `reports/main_menu_tactical_library.json`.
