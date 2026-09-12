# Canonical widget frame sequence

Addresses: `00AA87B0`, `00AA6A40`, `00A9E120`, `00A9A380`.
Descriptive names are hypotheses. The interfaces preserve canonical C++ owner
identities; they are not physical native widget, listener, list, or vtable ABI.

`GuiWidgetFrameRuntime` executes elapsed accumulation, child traversal, the
existing `GuiTimedEntryOwner` fragment, and the listener tail on the same owner.
Its only retained collections map existing listener identities to required
adapters. The active-frame stack is borrowed call-lifetime metadata. There is
no second widget tree, timed-entry array, clock, button history, or input sample.

| Routine | Original ABI | Coverage |
| --- | --- | --- |
| AA87B0..AA8AF4 | ECX widget, float seconds stack; RET4 | Complete normal sequence on the domain below; includes existing AA884D..AA892C timed owner fragment |
| AA6A40..AA6B08 | ECX widget; EAX boolean; RET | Complete for the established current64 profiles and existing AA6750 transform projection |
| A9E120 | ECX widget, four float pointers stack; RET10 | Complete, actual empty bounds virtual |
| A9A380..A9A386 | ECX mouse; ST0 float; RET | Complete field getter, matched live/disk bytes; currently undefined as a saved Ghidra function |

Ghidra was read through the verified existing `bsp.gpr` /
`/battlestationspacific.exe` wrappers. No functions, names, comments, or bodies
were changed. AA6A40 and A9E120 exports were refreshed without annotation edits.
A9A380's seven bytes are `d9 81 38 02 00 00 c3`, or
`FLD dword ptr [ECX+238h]; RET`. Its current38 word is D5B8E8 in the mouse table
D5B8B0, whose constructor producer is A9A290. The existing `MouseInputDevice`
already owns the +C/+10C histories and +238 double-click threshold.

## Actual profiles

| Canonical type | current40 word | Native target | current64 word / target |
| --- | --- | --- | --- |
| Screen1 | D5BE78 | AA87B0 | D5BE9C / A9E120 |
| Group2 | D5CBC0 | AA87B0 | D5CBE4 / A9E120 |
| Text3 | D5C708 | AA87B0 | D5C72C / AB6D70 |
| ClipBox16 | D5D098 | AA87B0 | D5D0BC / A9E120 |
| Section17 | D5C928 | AA87B0 | D5C94C / A9E120 |
| FrameBox18 | D5D170 | AA87B0 | D5D194 / A9E120 |
| Icon6 | D5C500 | AB1150, unsupported derived tail | D5C524 / A9E120 |

Icon must not inherit the complete base current40 route: AB1150 calls AA87B0 at
AB115C, then performs rotation and delayed work. Its body was read to establish
this boundary. Other unreviewed type profiles reject current40 as well. The
explicit `update_base_00aa87b0` entry is available for future recovered derived
continuations. Text hit testing dynamically resolves the existing
`GuiTextRuntimeImplementation` and calls its actual `align_bounds64_00ab6d70`;
it never substitutes A9E120 for Text.

## Sequence and original data

AA87B9 captures the original widget in EBP; no subsequent write changes that
register before its pops. EDI captures the child count at AA87C3 and decrements
once per iteration. ESI is the native list entry, later reused for timed-array
and mouse pointers. Every child lookup resolves the same owning layout and its
transform-list identity through `GuiWidgetOwnerRuntime::owner`.

The first visible effect is `FLD seconds; FADD [widget+80h]; FSTP [widget+80h]`.
For every captured child, current38 runs first. If false, AA87FD captures the
child's current signed timed count and scans its actual +88 backing for any
nonnull slot. A qualifying child receives the original seconds after the
AA882E/AA8838 float spill, not its parent's newly accumulated time. The same
timed-entry owner performs AA884D..AA892C after all children, using the same
constant storage and original seconds.

The listener tail requires live MouseHit +78 and a nonnull +DC pointer. AA6A40
loads Size and Pivot, calls existing AA6750, spills both origin subtractions,
adds Size, then dispatches four pointers to current64 in left/top/right/bottom
order. The caller pushes bottom, right, top, left; A9E120 and Text AB6D70 both
consume RET10. AA6750's full listing does not write EDX, preserving AA6A43's
widget through the recursive helper; this resolves its misleading pseudocode.
Four x87 inclusive comparisons reject unordered values. The pointer globals
F8BC74/F8BC78 are read at those native comparison phases after bounds dispatch.

AA9390 initializes +78/+79/+7C/+80/+88/+8C/+90/+D4/+DC. The existing transform
projection holds MouseHit, existing extra fields hold both clocks and +D4/+DC,
and AA6BC0 writes the same listener pointer and raw +79 mode byte. Existing
input history producers are A99E90 and the keyboard/mouse polling functions;
the code does not reinterpret a consumer-only guessed layout.

## Listener and input contracts

`GuiWidgetFrameListenerOwner` binds one actual existing +DC subobject identity.
Its pure methods identify current slots and deliberately do not assign event
semantics to unread listener bodies. The host must bind real handlers; an
unbound identity throws at the reached call. An implementation with empty
placeholder handlers does not establish this contract. These listener target
bodies remain external dependencies, distinct from the recovered dispatch.

| Native site | Containing body | Call | Required binding / contract |
| --- | --- | --- | --- |
| AA87EA | AA87B0 | child current38 | Existing actual type implementation; Screen AA38E0 or supported base A9E0D0 |
| AA883B | AA87B0 | child current40 | The profile table above, same owner and seconds |
| AA88E6 | AA87B0 | AD39A0 | Existing actual timed owner and current CRT-backed entry |
| AA890E | AA87B0 | entry current0 | Existing actual timed owner deletion, flags1 |
| AA8946 | AA87B0 | AA6A40 | This owner's current hit rectangle |
| AA896A | AA87B0 | listener current18 | Same widget, captured new inside boolean; target body not recovered here |
| AA8A15 | AA87B0 | listener current04 | Same widget; target body not recovered here |
| AA8ACE | AA87B0 | listener current0C | Same widget; target body not recovered here |
| AA8AEB | AA87B0 | listener current10 | Same widget; target body not recovered here |
| AA8A5C | AA87B0 | listener current14 | Same widget; target body not recovered here |
| AA8A36, AA8A95 | AA87B0 | mouse current38 / A9A380 | Same captured MouseInputDevice, live +238 float through x87 |
| AA6A6D | AA6A40 | AA6750 | Existing canonical resolved-position helper |
| AA6AC2 | AA6A40 | current64 | Four live float references; base RET10 or actual Text implementation |

The inside result stays in BL across current18. Only after its successful
return does AA896C overwrite the current +D4. Then AA8972 reloads the current
input backend and captures class1's first device. An empty/null first element
returns. A missing backend, malformed class vector, or foreign device is an
explicit host-domain error, instead of manufacturing a mouse state. Later
callbacks keep the captured mouse even if the singleton binding changes.

For mode +79=0, a released button0 calls current0C, then button1's history is
read afresh and a release calls current10. This mode does not require +D4 to be
true. The +DC listener identity is reloaded at each call, so changing it in
current18/current0C affects the subsequent handler.

For nonzero mode, an outside result clears +7C to positive zero. A button0
down-edge with ordered-zero +7C stores the current +80 and calls current04.
A subsequent down-edge calls current14 and clears +7C only when threshold is
strictly greater than elapsed difference. Without a down-edge, a nonzero/NaN
stamp clears only when elapsed difference is strictly greater than threshold.
Equality clears nothing. Both unordered x87 comparisons clear nothing. The
stamp comparison uses the native UCOMISS/LAHF/TEST parity rule, so NaN takes
the non-equal arm. The elapsed subtraction spills to **double**, at AA8A32 or
AA8A91, before the mouse threshold load. These paths do not subtract in float.

## Domain and verification

The canonical owning list and transform list must agree in order and retain
membership and surviving widget owners throughout callbacks. Count is captured
once. Local checks detect changed size/current identity but do not prove a
stable whole tree, protect arbitrary direct writes, or implement native list
mutation/invalidation behavior. Native malformed counts/sentinel aborts are
outside this valid-owner domain. Current adapters must keep their actual owner,
mouse and service storage alive. In particular replacing the frame service
while its call stack is active is unsupported.

Active metadata rejects reentry into the same owner while allowing ordinary
parent-to-child recursion. The runtime now registers with the same canonical
widget runtime, and shared retirement checks its `operation_active` state.
The update preflight runs before entering a frame; standalone pointer testing
borrows the owner for its current64 callback. The frame binding must be destroyed
before its widget runtime. Callback exceptions propagate immediately with completed
effects retained (for example +80 or a first-click stamp). There is no resumed
native exception frame or automatic retry. Listener unbinding is forbidden
while any frame callback is active. Text operations retain their existing
ownership, idle-state, allocator, and numerical domain requirements.

Standalone compilation passed with MSVC Win32 `/std:c++17 /EHsc /W4 /WX
/fp:strict`. One focused local fixture linked the parent's real `bsp_core` and
`bsp_lua511` plus this new source, with an embedded manifest. It passed canonical
child-before-entry ordering, skipped invisible child, original delta, actual
timed allocation, listener identity and button-history reloads, equality/NaN
timing, active reentry rejection, and partial stores after a callback exception.
Its bound listener handlers are synthetic observers, not native listener-body
validation. The parent combined `scripts/build.ps1` and final-library fixture
rerun also passed; see docs/ORCH5_SECTION_FRAME_LIGHT_BATCH.md. No permanent tests were added.
`tools/verify_report_calls.py` passed 15 numeric rows with zero failures; five
are resolved indirect calls needing the profile evidence above. Eight symbolic
rows (including raw A9A380) remain separately evidenced dependencies.
There is no new native-byte differential, drop-in ABI, game execution, or
rendering claim. The existing executable does not configure this new runtime.

## Correction from docs/ORCH5_TEXTURE_ICON_LISTENER_BATCH.md

The earlier unsupported Icon40 entry is historical. Canonical Icon40 now runs
AA87B0 followed by the actual AB1150 tail under one active owner guard. The base
gets the native outgoing x87 float copy; the tail retains the original delta.
AutoRotate, rotation44 and temporary-state84 are reconstructed. Delayed texture
loading and complete resource-backed Icon frame validation remain open. The
combined Win32 build and scoped numeric/frame probes passed. See
`reports/gui_icon_frame_runtime.json` and the batch report for exact limits.
