# Main-menu objective Text/Listbox builder

Addresses: `00594BF0`, `00582280`, `005823B0`, `005896F0`.

`build_main_menu_objective_rows_00594bf0` reconstructs the complete ordinary
synchronous caller. It binds the actual authored objective Group pairs through
`0058F5B0`, then clones Text widgets into the existing main Listbox and the same
three screen-owned linked lists read by the command and selection listeners.
No substitute GUI hierarchy, Text state, mission record or Listbox is created.

| Routine | Native ABI | Coverage |
| --- | --- | --- |
| 00594BF0..005966E4 | ECX screen; no stack arguments; RET at5966E4 length1 | complete ordinary synchronous caller; native STL/SEH and resource continuations excluded |
| 00582280..00582299 | ECX unused; no stack args; EAX sole node; RET length1 | complete successful12-byte sentinel allocation |
| 005823B0..005823E2 | ECX unused; stack next/previous/payload-address; EAX node; RET0C at5823E0 length3 | complete successful12-byte element allocation |
| 005896F0..00589782 | ECX list; unsigned increment stack; RET4 at589780 length3 | complete ordinary arithmetic and throw decision; native length_error/SEH represented by C++ exception |

The three lists are `MainMenuObjectiveWidgetList` owners. Each exposes a
`command_view()` borrowing its only head and count. The builder rejects views
which point at different storage. `00590423/3B/53` call `00582280` and store
the three sentinels and zero counts at main-menu+280/+298/+2A4. `00582280`
initializes next/previous to itself, leaving its unused payload unwritten.
`005823B0` initializes the three words of an element; `005896F0` checks
`uint32(0x3fffffff-count) < increment`, then adds the increment. The builder
publishes the captured sentinel's previous pointer, then the previous node's
next pointer. C++ ownership releases list nodes, never their borrowed widgets.
This is the one screen list, not an auxiliary allocation registry.

The native builder does not clear these description lists or delete earlier
header clones. Their entry/exit lifetime remains with the containing screen.
Only the main Listbox is cleared here. Its concrete `clear_rows_00a9bec0`
dependency uses current4(1) through `GuiTextChildDeletion`, with listener114
temporarily null, and resets the same row/selection storage. It does not add
the visibility, layout or notifications from individual remove-row calls.
The parent integration owns that helper and its GUI runtime files.

The constructor `005902E0` does not write +2B0/+2B4. Their actual producer is
still unresolved. These two floats therefore remain required references with
neutral field names and no invented defaults. Header slots +2B8/+2BC/+2C0 and
byte +2C4 are also supplied as references to the actual containing screen.

The function captures the selected mission, indexes the actual profile
manager+6B4 `MissionProgress` by mission `id` at+0, and sends the `name` at+8
to `00518D60`. Its call is ECX10, EDX0, stack(0,title); `RET8` proves both
stack arguments. This callee writes three transition globals, gets the
front-end frame, invokes518C80, and invokes5189B0 for a nonempty title. These
children remain an explicit addressed provider; returning from a dummy
implementation does not establish that this transition has completed.

The caller then publishes page12/selection0, hides the world map, shows the
briefing widgets, invokes58F5B0, shows its actual BG_01_Group, and captures
the original mission's current side block. It stops the actual movie only
when AAC8E0 reports playing. Both Group vectors receive recursive factor
CE3800, then their current selected elements are shown and receive factor1.
The selected index is reloaded for each operation.

`00594FB3` passes five command triples to54B530: `(A2,globals.continue,0)`,
`(A7,FE.naval,1)`, `(B3,globals.navigate,1)`, `(A3,globals.back,2)`,
`(0,empty,1)`. Callee RET3C consumes fifteen words; the decompiler's shorter
argument list is wrong. The code composes the existing command-bar owner.

| Category | Main Listbox Text from+238 | Positioned Text from+258 |
| --- | --- | --- |
| primary with authored entry | `N. |<primaryObjectives[i]>` | same source; first highlighted/color270, remaining color260 |
| primary missing entry | `N. primary Objective` | `Primary Objective Placeholder` |
| secondary with entry | `N. |<secondaryObjectives[i]>` | same source/color260 |
| secondary missing entry | `Secondary Objective` | `N. secondary Objective` |
| hidden with entry | authored hidden objective without prefix | `N. |<entry>` or `N. |globals.obj_hid_mask` |
| hidden missing entry | `Hidden objective` | `Secondary Objective` |

Rows receive D8 primary index, primary-count+secondary index, or
secondary-count+hidden index+primary-count and MouseHit78=0 before native
A9D750(null insertion,after0). RET0C proves all three arguments. Description
clones receive MouseHit78=1, borrowed listener screen+40/raw flag0, and D8
equal to the current Listbox row count minus1. Primary/hidden show before
these writes; secondary shows afterwards. Secondary/hidden color before
publication; primary color after layout and first-row highlight.

All three header Text clones use the authored+24C prototype. Only the first
changes its shader to `GuiFontBilinear.mshd`. Secondary and hidden headers
are created and shown even for zero category counts. Their resolved X comes
from+24C; Y and Z come from+258. Description XYZ comes from+258. Each position
reads Z, Y, then X using three native AA6750 operations and adds the running
offset to Y. Every lookup uses current screen slots.

The offset starts0. After each description, AB6BD0 returns a float32 value
in ST0, then the caller computes `height + extended(spacing2B4 + offset)` and
stores one float32 result. A category header adds2B0 before placement, then
adds `headerHeight + extended(spacing2B0 + offset)` afterwards. Explicit
Win32 x87 helpers preserve this order and its spills. The final Listbox
position adds positive zero to saved XY and live CE47A0 double to saved Z.
It selects index0, forces current80(1), positions, then clears widget DC/79;
it does not clear the separate Listbox listener114.

The hidden score map is copied only after the hidden header callbacks, from
the captured score record's +21C or+240 using a fresh side test on the captured
mission. These are the existing `objectives_204[2]` and `[5]` maps produced
by the mission-progress reader. Empty map or its first node's value1C==0
masks every authored hidden description. There is no iterator advance:
596026 is the only write to stack+A4 after the copy, and59625B/59626D only
read it. Earlier uses of that stack slot belong to released string temporaries.
This repeats the first saved value, regardless of objective index or key.

The decompiler's supposedly unreachable native-string copy branches are
retained: each tests a buffer returned by0041DD40. They are not proven dead.
The self-comparisons at594CFD/594D63 do prove their adjacent invalid-iterator
calls unreachable. Invalid native iterators and allocator/SEH failure are
outside this new C++ transport; missing elements/owners fail explicitly.

Two bypassed alignment gaps are separate from the real tail:
59589D..59589F=`8D4900`, and596039..59603F=`8DA42400000000`; branches jump
over them. Both have no Ghidra function. The stored594BF0 body ended5966CB
after `_free`, but raw verified bytes5966CC..5966E4 restore FS/stack/registers
and RET. That25-byte fall-through epilogue is real executed code, also with
no Ghidra function at inspection time. True inclusive end is5966E4, not5966CB.
This worker makes no Ghidra writes; parent may separately repair the flow.

The canonical clone host still requires actual Text copy/type factory and
resource behavior. `GuiTextRuntimeImplementation` content, shader ownership,
Listbox layout and concrete deletion each retain their own supported domains.
Any pending or unsupported child exception stops this synchronous caller;
resuming only that child does not resume the row builder. All currently
borrowed owners, providers, mission/score records and traversed node storage
must remain alive across callbacks. Mutating other live slots is observed.
No original ABI, executable menu reachability, rendering or gameplay parity
is claimed. Typed string transport requires stable null-free DWORD-length
mission text; native string allocation schedules for those transported records
and C++ map/list storage are not claimed. Validation status and exact call-site
evidence are in the report.

Validation: strict MSVC Win32 compilation passed. `scripts/build.ps1` passed
with `reconstructed_math` and `native_math_differential` (2/2); all eight
native seeds matched the installed executable before enabling the latter.
The report's247 direct call rows passed live verification. Its21 indirect
sites retain native setup evidence and dispatch through actual canonical
types. One ignored fixture passed for shared head/count identity, duplicate
payload ordering and the count-limit decision; it uses no GUI substitute.
That fixture compiles the final source and links existing parent build
dependencies. It does not execute the complete resource-backed builder.
`src/game_hosts_mission.cpp:763` still reports this action as unimplemented,
so this packet does not claim executable menu, render or gameplay validation.


## Correction from docs/ORCH5_MENU_INPUT_BATCH.md

The spacing producer described as unresolved above has now been found in
00582F30, after its current14 layout-binding call. The bounded582F45 fragment
writes the same optional2B0/2B4 fields with the original double difference
spills. It is separate from5861B0. The layout also owns the sole intrusive
description lists; command views and row bindings alias those cells. The
remaining registration phases and limits are in MAIN_MENU_SPACING.md.
