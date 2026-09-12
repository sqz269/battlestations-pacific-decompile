# Main-menu current04 command listener

Addresses: 005993A0.

`main_menu_listener_current04_005993a0` reconstructs the full normal caller
sequence in 005993A0..00599D57. It shares the existing widget registry, main-menu
layout, screen flags and two scrollers. The caller still requires the addressed
page/profile/navigation services listed below. This is source reconstruction
with a new C++ interface, not a complete concrete menu provider or a native ABI
replacement. No Ghidra mutation was performed by this worker.

The native constructor at 0059033A installs CEFC04 at screen+40; its current04
cell is CEFC08 = 005993A0. ECX is screen+40, the widget is a stack argument,
and every normal exit has RET4. Ghidra reports the complete body ending at
00599D57; code stops with RET4 at 00599D55 and following jump-table data is
outside that body. The SEH handler is C713E0. All report call rows name this
containing function, including 5997C1 and 5998D0, previously misattributed to
599340 in historical mission-detail commentary.

## Caller behavior

Current4C receives literal 1.0 from FLD1/FSTP at 5993BB/5993CE before the first
current5C type query. This occurs for every widget. The implementation uses the
same actual derived implementation and resolves type again after alpha's side
effects. Empty Text maps to byte zero, matching E19508; otherwise comparison is
the low byte of the current UTF-16 first code unit. It preserves the separate
current-text reads between A2, A3, B4, B6 and A7 comparisons.

| Text code | Page | Action |
| --- | --- | --- |
| A2 | 1 | 588A80 top-level selection |
| A2 | 2,3,11 | 425E50 selected row, then actual screen+8 current04/598B60 |
| A2 | 4..7 | Publish listbox D8 selection; profile/checkpoint/prompt sequence below |
| A2 | 8,9,12 | 5922F0, 594BF0, 5885D0 respectively |
| A3 | 2,3,11 | 584AE0(false) |
| A3 | 4..8 | Same backdrop328 and position124 through existing AA8240, then584F50 |
| A3 | 9,12 | 599340,58C010 respectively |
| B4/B6 | 4..8 | Bit-copy CE69CC/CE54A0 to same zoom19C, set same zoom-axis latch1A0 |
| A7 | 12 | Fresh selected mission; current tactical9C/A0/A4 = mission,2,0;5886C0 |
| other | 12 | Find clicked widget in screen lists280/298/2A4, then listbox selection |

The constructor 59041B..59045B creates those three lists with 582280's 0Ch
self-linked sentinel and a zero count. 582130 returns current node+8 after
debug checks; 582160 increments to current node+0. The borrowed head/count/node
views introduce no replacement vector. Their provider must supply the same
screen-owned collections and canonical widget payloads; constructing/populating
that provider is still open. All count and sentinel reads remain current.

Native 425E50 returns a selected widget pointer. A9C990 returns its D8 data bits,
or FFFFFFFF for no selection; A9C920 returns its list ordinal, or -1. The caller
uses these different values as written, even when D8 differs from ordinal.
For the page12 fallback, it captures clicked D8 before A9C920, compares that
to the ordinal, and reloads clicked D8 for A9C7C0. Page4..8 arithmetic also uses
the actual A9C990 data bits, with DWORD wrapping and signed comparisons.

For non-Text widgets, pages4..8 call A9C990 before any widget-identity gate.
Enabled up/down arrows change selection and invoke current80(1), reloading the
same screen's1B8 slot after selection callbacks. Down tests the next actual
row's77 hidden byte. Mission and briefing scroll arrows call683300 with
direction2/1, x87-spilled live CEC178 and override1. Thumb branches call683A40
on the same scroller and actual cursor/widget-position host. Alias-sensitive
branch priority is retained. Native pressed-state tail calls Icon88(2,0,1),
or a second fresh type5C and FrameBox84(2). It does not collapse the two type
queries or replace pressed2 with a Boolean state.

## Checkpoints and strings

59946E writes A9C990 to E194DC before599473 writes E08878. Pages5/7 set the
existing screen.us_campaign (+564); this is not the DLC565 byte. Each profile
owner read from E188A8+650 occurs before its fresh selected-mission call. A
missing checkpoint clears the same screen5C byte and calls58C010. Otherwise
7FC490 produces a stack14h value, 7FC370 checks the vehicle-class table, and
584750 conditionally checks the class/unlock requirement. The implementation
retains the first584750 Boolean operand even though that callee does not read
it, and reads vehicle id from temporary+8 before reading current screen564.

437490 establishes the temporary's layout: three words plus a deep-copied
NativeString at0C. It is a returned value, not another profile owner. The
profile methods remain required boundaries against the same ProfileResetState;
they must preserve native lookup/insert/deep-copy behavior and current globals.

Available/unlocked uses `globals.checkpoint_available`, callback592AD0, then
focus-first530670. Other arms concatenate either
`globals.checkpoint_unit_locked|.\n|.` or
`globals.checkpoint_unit_notavailable|.\n|.` with the checkpoint's native text,
then `|.\n|globals.continue_without_checkpoint`; callback58F540 and focus-last
530C20 follow. Bytes at CEFE6C..CEF F00 were read directly. The43C130 prefix
helper's constructor/concatenation/destructor sequence is inlined using the
existing actual-header string bodies, and the outer4261A0 is reused.

All prompts use the existing actual FrontEndPromptScreen/Host and531B00:
slot5, kind1, callback, unused0, empty title, timeout+0, result0, an owned empty
NativeString and dismissible1. Native RET2C consumes all ten logical arguments,
including the eight-byte by-value string. The combined string, prefix result,
suffix and title are destroyed in that order before the fresh425D10 getter and
focus call. The checkpoint's current native string is released last (436490).
No std::string formatting or copied prompt screen replaces these effects.

## Required service contracts and coverage

| Callee/binding | Evidence and current coverage |
| --- | --- |
| 425E50/A9C990/A9C920/A9C7C0/A9BE00/A9C220 | Actual canonical GuiListboxRuntime, separately reconstructed packet; no second listbox |
| 598B60 | CEFC48+04=598B60, installed59032D; ECXscreen+8, RET8 at59932F. Required selected-widget AND original-listbox operands; dispatch body partly read, not reconstructed here |
| 588A80 | Seven-way top-level dispatch body read; existing policy plus required concrete services |
| 584AE0 | Page1 builder, Boolean select-first, RET4 at584F44; existing source/provider boundary |
| 584F50 | Page2/list builder; partially read, addressed required boundary |
| 58C010 | Existing normal mission-detail reconstruction; concrete host required |
| 5922F0 | Existing movie/direct mission-start reconstruction; actual host required |
| 594BF0 | Large mission-detail accept body only partly read; addressed required boundary |
| 599340 | Existing complete page9 return-to-selected-list routine; same screen/provider required |
| 5885D0/5886C0 | Full bodies read; parent supplies actual tactical-field/request composition |
| 7F8D60 | Full body read: checkpoint-map find and iterator/sentinel comparison, RET4 |
| 7FC490 | Full body/listing read: 7FBED0 find-or-insert then437490 copy, RET8 |
| 7FC370 | Full body/listing read: exists, class-1 fast path, live VehicleClass lookup, RET4 |
| 584750 | Full body/listing read: class/unlock predicate, first operand unread, RET8 |
| 425D10/531B00 | Actual canonical prompt owner and existing prompt implementation |
| 530670/530C20 | Parent-owned same prompt input-mode/navigation-focus helpers |

Identity resolvers (`listbox_runtime`, `transform_host`, `prompt_host`) only
resolve associations; they must not perform game actions or synthesize a second
owner. Listbox resolution validates its owner identity. Other services are
required and have no default successful/no-op implementation. The parent
supplies the final current04 listener adapter and lifetime protection.

ABI cleanup is checked at the callee, including the previously hidden second
navigation operand: PUSH1B8 at59943F survives425E50's bare RET; PUSHselected at
599448 precedes598B60's RET8. Source register provenance was checked from the
whole containing listing: initial ESI=screen+40, EBX=widget, EDI=screen on the
checkpoint arm; ESI later becomes a prompt message only on terminal prompt
branches, and becomes the current listbox only before the common pressed tail.

Coverage is complete normal5993A0 caller sequencing with required services.
Native iterator-debug traps, SEH unwinding, original object/vtable/string ABI,
complete listbox producer/teardown/provider, page/profile service implementations
and executable/gameplay integration are not claimed. Current executable host
sources contain no5993A0 entry; therefore no new runtime reachability claim is
made from the build.

Validation: strict MSVC Win32 compilation passed with /W4 /WX /fp:strict,
using the worker's actual GuiListboxRuntime header. No permanent tests added.
All native CALL instructions, including string cleanup and debug-trap paths,
are included in the report for the mechanical containing-body/callee check:
82 numeric rows passed with zero failures (72 direct and10 resolved indirect,
81 distinct instruction sites; current4C has two established profile targets).
An independent Listbox worker reviewed the D8/ordinal/arithmetic, fresh1B8,
hidden-row and pressed-tail paths against599B2A..599D0E and found no mismatch.
Parent combined build and any focused/runtime validation are separate evidence.
