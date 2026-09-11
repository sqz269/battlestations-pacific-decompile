# Native renderer frame-target binding

Complete source for `B24E70..B24FAE` (318 bytes) uses actual renderer and
frame-target owner storage, the established concrete lifetime providers,
actual synchronization globals and the application byte at `F8D398`.
The original interface is ECX renderer, stack group, RET4. The new C++
interface adds explicit context and is not an original caller ABI replacement.

Optional guard entry precedes the identity read at renderer+1908; the read
at `B24EAF` precedes cleanup arming at `B24EB5`. A changed incoming group
causes a second old-pointer load, a second identity test, publication and
incoming atomic increment before outgoing atomic decrement. Final zero
uses the captured outgoing group's current profile and virtual zero, then
full `BD30E0` rereads the profile and selects scalar destruction at +4.
The established concrete two-slot `D5E600` profile reaches full `B1FCF0`,
including its vector/surface/COM/CRT lifetime and native exception behavior.
Foreign profiles are outside this explicit application contract.

Null incoming does no GPU work. Nonnull incoming checks the CURRENT
application enable byte after outgoing destruction. If enabled, the exact
incoming +3C byte becomes DWORD render state C2 through full `B24460`.
Then full `B23D80` unbinds slots1..3, then binds slots0..3 using the
incoming group's current borrowed getters. Full `B21690` binds its current
depth getter. Callbacks may mutate the renderer's retained slot, but all
these child reads continue using the original incoming argument. Each
child getter happens immediately before its corresponding child call.
Earlier publication, reference updates and GPU work are not rolled back.

Both normal branches read current mode before disarming cleanup and call
full `B33B00` with the saved renderer if enabled. Native handler `CBD0A8`
uses FuncInfo `DF5754`, unwind map `DF574C` and guard funclet `CBD0A0`
passing EBP-14 to full `B21110`. A second C++ cleanup exception terminates.
The skipped-entry record is uninitialized, with no invented default fields.

The vector and owner prerequisites are integrated. The strict main Win32
build and both existing CTests passed, followed by an independent original
parent/full-library comparison. Whole renderer and gameplay validation
remain separate.


## Primary validation

The primary checked 1,447 immutable worker pins and 37 unchanged current
provider source/header files. Nineteen fresh guarded live/PE spans total
589 bytes and include the complete 318-byte parent. Eight fresh native
seed ranges matched. The unchanged fixture linked only the frozen actual
main library `9be94991d23f1951ad8913d1cc7cc0390f9a3d416cecea12eda78ac7601610db`,
also independently frozen by the primary during owner validation.

All ten comparisons passed: 692,652 compared DWORDs, including 692,419
literal matches. The remaining 233 differences are mapped only at proven
temporary old-group, old-surface-wrapper and vector pointer fields. Every
raw trace is retained; no other renderer/group/registry/CS/global/device
words are normalized. The 357 snapshots cover identity and null paths,
complete old-owner deletion, changed application enable byte, original
incoming retention across slot mutation, current device/getter changes,
null surfaces, and entry/owner/nested-child exception paths.

Fourteen exact archive objects, 497 complete COFF sections and all their
relocations match the linked image. The proof covers 242 linked entries
and 27,401 unique code bytes including fixture helpers; the linked parent
is 460 bytes. Twenty-one complete postimage phases verify original code,
linked text/read-only data, explicit child bridges and observed COM tables.
All 262 recorded provider calls resolve to proven original/compiled call
sites and real D3D9, Windows critical-section and CRT providers.

The private original parent executes its complete guards, getters, current
scalar invoker, FH3 handler and unwind map. One EH registration operand is
relocated; declared child ABI bridges call the complete established source
providers. Real operations run before observer mutations or exceptions.
These cases do not establish untouched pool arenas, arbitrary foreign
profiles, every possible exception, native caller ABI, or gameplay.

The original Ghidra project retains its previous name and comments with
new reviewed evidence appended and saved. The export is refreshed and a
complete function reconstruction record is registered.
