# Main-menu map runtime adapter

Addresses newly reconstructed: 00AB1EF0.
Existing contracts consumed: 00588C70, 00AA7E00, 00AA8240, 00AA6750,
00AA7970, 00AB10D0 and 00AB3CB0. No saved Ghidra changes.

`MainMenuMapGeometryRuntime` binds the existing complete typed map sequence to
the retained `GuiWidgetOwnerRuntime`, `MissionTreeTables`, published
`MainMenuMissionSelection`, caller-owned five point vectors and three live
widget pointer slots. It implements the original host directly. It adds no
second service interface, copied widget tree, generated points or replacement
mission records. Descriptive names are hypotheses, not recovered symbols.

| Routine | Coverage | Original ABI | Implementation |
| --- | --- | --- | --- |
| 00588C70 | Existing complete typed sequence reused; adapter supports retained Icon backdrop | ECX screen; six stack DWORDs; RET18h | `MainMenuMapGeometryRuntime::update` |
| 00AB1EF0 | Complete body for the retained Icon implementation; other derived virtual overrides are outside this adapter | ECX Icon; stack size-pair pointer; RET4 at 00AB1F53, length 3, inclusive end 00AB1F55 | `GuiIconRuntime::set_size58_00ab1ef0` |

The adapter receives **references to pointer slots**, not copies of the three
widget pointers. Pass the layout binder's `backdrop_328`,
`selected_map_point_330` and `selector_338` lvalues. The binder owns discovery;
the adapter never substitutes a page root for `input.lookup_root`. That input
and lookup results are `GuiLayoutWidget*` identities represented as `uintptr_t`,
not original process addresses. A missing retained owner fails explicitly.
The layout binder identifies +338h as `selector_Icon`; its evidence remains
in the separate binder packet.

`update` derives its selected-widget gate from the borrowed +330h slot. The
existing driver updates zoom before this gate and owns that delta entirely;
an outer caller must not run the older delta-only projection afterward.
Geometry state is updated in place. Only the immutable per-call input record
is copied to replace the gate; published indices, vector contents, pointer
slots and GUI state are never snapshotted.

The mission-count callback rereads `selection.group_index` and the current
`missions.groups[group].missions.size()` each time. The selected index is
also reread at each existing driver request. The group's count is independent
of the supplied point-list index. `MissionTreeTables` is the canonical
005CAAF0 reader result, with 34h groups and 434h missions; this adapter adds no
offset declaration. Existing mission-detail producer 0058C662..0058C6B3 fills
the point vectors from resolved icon positions minus the backdrop position.
The adapter never fills an empty list. Out-of-range access throws instead of
calling native iterator-failure 00BF6713; no clamping/default entry is added.

| Containing function / call site | Native callee | Concrete binding |
| --- | --- | --- |
| 00588C70 / 00588F9C | 00AA7E00 | Existing direct-child, node-present, case-insensitive lookup |
| 00588C70 / 005890FE | 00AA7E00 | Same operation, performed even if point lookup missed |
| 00588C70 / 005892F0 | 00AA8240 | Existing inverse-parent arithmetic then retained owner local write, matrix publication and bounds refresh |
| 00588C70 / 00589326 | 00AA8240 | Same operation for flag |
| 00588C70 / 00589378 | 00AA8240 | Reload +328h and move same backdrop owner |
| 00588C70 / 005893B5 | Current +58h, Icon 00AB1EF0 | Require the actual `GuiIconTypeImplementation` and use its own runtime |
| 00588C70 / 005893C5 | 00AA6750 | Existing resolved-position getter over live +330h owner |
| 00588C70 / 0058940B | 00AA8240 | Reload +338h and move same selector owner |
| 00AB1EF0 / 00AB1EF9 | 00AA7970 | Base size pair write, then actual owner recomposition; no bounds refresh |
| 00AB1EF0 / 00AB1F4E | Current +8Ch, Icon 00AB10D0 | Existing runtime rebuild of freshly read current state through 00AB3CB0 |

## Icon size evidence

Live bytes at Icon vtable 00D5C4C0+58h (00D5C518) are `F0 1E AB 00`.
00AB1EF0 first calls 00AA7970, whose full seven-instruction body copies two
floats with FLD/FSTP and calls 00AA7220. It does not call the bounds routine.
After that call, 00AB1EFE reloads the current word at +ECh. -1 alone skips the
state branch. The vector check sign-extends the index then compares it
unsigned; every other negative index fails. The supported C++ explicitly
checks nonnegative and in range, writes the same selected state's size pair,
then rebuilds the runtime's current state. It retains the original order even
for an invalid state: the base size/matrix change happens before the failure.

EBX is saved, assigned the caller's pair pointer at 00AB1EF1, used after the
base call at 00AB1F33/40, and restored at 00AB1F52. ESI captures ECX at
00AB1EF7 and survives the call. This establishes that the input is borrowed
across base publication; it is not an early size snapshot. No pushed vararg
list is transcribed: both size setters end in RET4. Live xrefs to 00AB1EF0
are the three vtable cells 00D5BD38, 00D5C518, 00D5CC70; there are no direct
callers. Only the verified concrete Icon type is accepted here.

The widget-size and icon-state-size producers already have canonical
definitions in `gui_widget.hpp` and `gui_icon.hpp` (00AAAED0 Size descriptor
and 00AB66C0 state writer). The new method preserves these exact objects and
uses explicit x87 float loads/stores. Position arithmetic reuses existing
rounded kernels; map interpolation keeps its established binary32 spills.
Native exception ABI, SEH/allocator behavior, signaling-NaN trap/status timing
and arbitrary concurrent/reentrant lifetime invalidation are not claimed.

## Verification and remaining dependencies

Ghidra queries/export use the verified existing `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`. The complete 00AB1EF0 body was read both as
pseudocode and assembly; 00AA7970 and 00AB10D0 were inspected before binding.
The three existing GUI contracts were reread, including x87 assembly for
the resolved setter/getter. Existing map driver evidence supplies its two
call sites and full sequence; this adapter changes no native contract.

The adapter needs the actual layout/owner/resource setup and mission-detail
point producer. `GuiIconRuntime` retains its existing real D3D geometry,
texture, material, clip and owner-lifetime service requirements. It does not
replace those services when resize triggers a rebuild. Page/owner/table/list
and pointer-slot storage must outlive the adapter; retire the adapter before
destroying those objects, and prevent their invalidation during a call.

Source registration and standard Win32 build are handled by primary
integration. A single local owner probe compiled both changed C++ sources
with MSVC Win32 `/W4 /WX /O2 /fp:strict`, linked the existing `bsp_core.lib`,
and passed. It verified actual native-model matrix publication and identity,
rebinding the borrowed backdrop/selected pointer slots, rereading selection
and list state, rejecting a missing point, and applying zoom with +330h null.
It used existing real owner construction with supplied fixture state; the
Icon size/rebuild path was compiled and inspected but not executed. There
were no mock size/rebuild callbacks. The strengthened report verifier checked
10 call rows with zero failures; two indirect targets require the separately
recorded vtable byte evidence (00D5C518 and 00D5C54C).

No
installed-page map execution, rendered-map parity, native differential or
binary-compatible replacement is claimed. No raw undefined routine was
introduced; `no_ghidra_function` is empty.
