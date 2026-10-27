# Raw Page lookup prerequisite, 2026-10-24

`find_native_gui_page_00aa3140` implements the complete normal raw lookup
schedule over an already-existing actual GUI manager. It does not construct,
publish, activate, retain or destroy a manager or Page. It is an unbound source
prerequisite for Step 4, not native Page delivery or runtime acceptance.
The existing registration/removal/lifetime APIs and typed loader are unchanged.

## Original and source boundary

The analysis used the existing `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Original listing and current primary
`f3c26eea8` source were inspected before edits; the two registry source files
matched the worker base. No new program or singleton was created.

| Original | Recovered interface and relevant listing | Source composition |
| --- | --- | --- |
| `00AA3140`, 167 bytes / 70 instructions | ECX actual manager; stack native name header; EAX borrowed Page or zero; `RET4`. `AA3142` captures manager+14, `AA3146` captures its begin, `AA3158` rereads end, `AA317E/AA3180/AA3183` capture the current Page profile, current slot+7C and invoke it. | Complete raw vector scan with required current-profile/table resolution and explicit dispatch for unknown name targets. |
| `00AC3FA0`, 7 bytes / 2 instructions | ECX Page, `LEA EAX,[ECX+100h]`, plain `RET`. D5BE38 slot+7C selects this body. | Reuses existing `gui_widgets_cc13::gui_layer_name_00ac3fa0`; no duplicate leaf or replacement name storage. |

The manager is the actual88h object constructed by AA5D70. Its vector header
at +14 has opaque/proxy +00, begin +04, end +08 and capacity +0C; therefore
absolute begin/end/capacity are manager+18/+1C/+20. Slots contain actual Page
pointers, not `GuiLayoutPage` or `GuiWidgetOwner` addresses. A Page's native
name is the eight-byte header at +100/+104, with a byte length and C-string
buffer. Incoming names have that same header representation.

Original AA3185 reads the returned name length before AA3187 reads the
incoming length. Equal empty names match without reading buffers. Equal
nonzero lengths compare actual-name bytes against incoming bytes through the
current CRT `_stricmp`, a qualified binding for original BF7FBF. First equality
wins. A miss returns zero. No name cache, descriptor registry or typed Page
lookup participates.

The assembly checks CURRENT end before callback dispatch, before advancing,
and after a successful comparison. Critically, AA31D5 reloads the selected
vector slot after the name callback; the source returns that current slot,
not its earlier captured Page receiver. AA3165's always-equal container check
has no reachable failure branch. Existing Source invalid-parameter transport
represents BF6713; accessible valid storage is the supported domain.

## Required borrowed provider

Call `find_native_gui_page_00aa3140(actual_manager, actual_name_header8,
bindings)` with a real `NativeGuiPageLookupBindings` implementation.

`current_page_table(actual_page, captured_profile)` is a side-effect-free
metadata resolution, not an Original call. It must return the SAME live table
represented by the captured current raw profile, covering at least 32 DWORDs.
The helper captures table+7C at its reached native dispatch point. A null table
is an explicit Source admission failure. Numeric profiles and entries are
identities; neither is cast to a callable host address.

For current target AC3FA0 the existing genuine source leaf returns Page+100.
Every other target is passed to required `call_page_name_7c(actual_page,
captured_target)`. That provider must execute the genuine corresponding
Source body and return its actual borrowed native name header, or reject the
unsupported target. It must not substitute AC3FA0, a typed name, a forged
table, a successful no-op or a numeric-address call.

Externally serialize lookup with GUI mutation and callbacks. The existing
manager/vector, current tables, native name bytes and provider metadata must
remain valid through their reached reads; callbacks must satisfy the native
iterator/storage lifetime contract. Metadata resolution adds no side effects,
reference credits or fallback. The result is borrowed; this helper changes
neither Page+04 nor any manager publication/count/container field.

The caller supplies the already-existing manager and matching canonical
domains. This API never invokes 004C12B0, AA5D70, singleton-manager registration,
Page allocation, Lua loading, property dispatch, material binding or terminal
retirement. Actual124h Page construction and populated-child lifetime remain
separate prerequisites.

## Validation and limits

`scripts/build.ps1` passed in the worker checkout, including MSVC Win32 Release
compilation/linking of `bsp_game.exe` and both existing CTest checks:
`reconstructed_math` 0.25 s and `tool_tests` 9.45 s; total 9.73 s. The ignored
capture is `local/gui-page-lookup-build.log`. No new tests, probes or game runs
were added; the raw lookup has not been invoked in a runtime or fixture.
AA3140/AC3FA0 evidence comments were saved in the verified BSP project and
their exports refreshed. Prior annotations are retained in
`local/ghidra-annotations-20261024T112704Z.json`.

This is a new explicit C++ interface, not the original register/call/return ABI.
Original private CRT/heap, FH3/SEH, malformed or concurrently mutated storage,
unknown-target implementation, native application activation and game/runtime
parity are not established. The complete normal algorithm is reconstructed;
its external current-table and unknown-target obligations remain explicit.
