# Base GUI current70 clip refresh

`GuiWidgetClipRefreshOperation` implements `00AAA3E0` using the existing
`GuiWidgetOwner`, its one layout/transform tree, actual Model/Mesh/Section/
Material owners and `register_native_gui_clip_parameters_00aa9f10`. The runner
calls each child's actual current70 and retains its borrowed continuation if
that call throws. This packet adds no layout, material, font or resource owner.

| Routine | Original ABI / inclusive body | Coverage |
| --- | --- | --- |
| `00AAA3E0` | ECX widget, no stack arguments, RET at `00AAA465` length 1; `00AAA3E0..00AAA465` | complete in the supported domain below |

All 53 instructions were read. Every live analysis batch used `bsp.py ghidra`,
which verifies `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe` before
querying. Ghidra remained read-only; existing name `FUN_00aaa3e0` and comments
are unchanged. `BSP_GuiWidget_RefreshClip` is a proposed descriptive hypothesis;
annotation and refreshed annotated exports remain integrator work. The new
C++ interfaces are not binary ABI replacements.

## Actual material sequence

`EDI` captures entry ECX at `00AAA3E3`. `ESI` captures the primary node+4C at
`00AAA3E5` and remains that same node for both geometry getters. A null node
skips the material work. A nonnull node must resolve through the canonical
attachment registry to the same live `NativeModelReference` and Model
environment; a non-Model node is explicitly outside this interface.

| Call site | Native callee | Body-verified contract |
| --- | --- | --- |
| `00AAA3EE` | `00B74650` | Test captured Model+180, ECX Model, RET; null skips material work |
| `00AAA3FB` | `00B74640` | Read captured Model+180, unused stack DWORD 0, RET4 |
| `00AAA402` | `00B72B40` | Actual mesh DWORD+58 count, ECX mesh, RET; zero skips material work |
| `00AAA411` | `00B74640` | Reload geometry on the same captured Model, unused DWORD 0, RET4 |
| `00AAA418` | `00B732C0` | Actual mesh pointer array+54[index0], index stack, RET4 |
| `00AAA423` | `00AA9F10` | ECX original widget, captured section material+20 stack, RET4 |
| `00AAA452` | current child virtual70 | Reload current child payload, table and slot; ECX child, no stack arguments |

The two PUSH 0 instructions at `00AAA40B/40D` supply the section index and
unused geometry argument; their separate callees each perform RET4. Nothing
is inferred from an apparent decompiler register argument. The full listing
was filtered for every ESI/EDI/EBX use to establish register provenance.

`00AA9F10` is the existing actual name/pool/parameter implementation. Its full
body, overlapping-global warning and floating-point branch assembly were checked. It writes the
same widget+E8, constructs/registers/releases `cClip`, then reloads +E8. The
active arm registers `cClipCenter`, `cClipBorder`, then `cAspectRatio`, with
temporary-name cleanup between phases. Center/border remain borrowed from
the captured canonical ClipBox; aspect is the same supplied live global slot.
This caller adds neither the separate B18A40 owner operation nor a retain.
The model resource registry, material destruction access, parameter pool and
string service identities must match. Material and all borrowed sources must
outlive callbacks and every later parameter consumer.

## Current profiles and child order

Both live callers and xrefs were queried: there are no direct callers, and all
18 references are DATA table cells. The supported factory constructors prove
the following class/table associations. Each listed current70 cell was read
live and contains `E0 A3 AA 00`.

| Existing factory profile | Constructor table store | Table | Current70 cell |
| --- | --- | --- | --- |
| Screen1 | `00AC6640` in `00AC6600` | `00D5BE38` | `00D5BEA8` |
| Group2 | `00AC6F5A` in `00AC6F50` | `00D5CB80` | `00D5CBF0` |
| Icon6 | `00AB5C91` in `00AB5C60` | `00D5C4C0` | `00D5C530` |
| ClipBox16 | `00ACE0AA` in `00ACE0A0` | `00D5D058` | `00D5D0C8` |
| FrameBox18 | `00AD262C` in `00AD2600` | `00D5D130` | `00D5D1A0` |

The base constructor `00AA9390` stores table `00D5C130` at `00AA93CB`; its
cell `00D5C1A0` also contains AAA3E0. `00D5BBF8+70` is another observed
reference, not the base constructor's table. Text table `00D5C6C8+70` instead
contains `40 7A AB 00` (`00AB7A40`), so Text must keep its actual main/shadow
override. The profile predicate accepts exactly the five existing supported
non-Text types. Other observed tables are evidence, not added factory support.

After material registration and all name cleanup, `00AAA428` reads the live
child-list head. ESI now holds the current list node, EDI becomes widget+64,
and `00AAA432` reloads the current sentinel into EBX each iteration.
`00AAA44A..452` reloads the current payload, its current table, and current70.
After the child's full call, `00AAA454` checks the entry against the current
sentinel, `00AAA45E` advances through the next link, and the loop reloads its
end/payload. The parent never caches a child's implementation across calls.

The existing `transform.children` vector represents borrowed payloads. This
interface requires stable membership/order from the first child selection to
completion and surviving frame owners. It reloads the list size and next
payload after each callback; duplicate payloads remain distinct positions.
Material callbacks before traversal can change the list. Native intrusive
iterator identities, insertion/removal during traversal, and self-deletion
are outside this projection. The three BF6713 list-consistency calls at
`00AAA437/445/459` were inspected; the first is unreachable after CMP EDI,EDI.
The forwarding library body was read, but invalid-list CRT behavior is excluded.

`begin()` performs the material prefix once and calls actual child70 for every
selected entry. A throwing child leaves the exact before-call frame retained.
Calling `begin()` again is rejected. `pending_child_owner()` identifies the
child whose actual current70, including nested continuations, must complete
before `resume_after_child70()` may advance. Resume consumes that frame,
selects the next live child, and continues actual dispatch. It does not retry
the previous child or skip an unknown implementation. Running calls also count
as pending; reentrant begin/resume is rejected. Owner integration must reject
retirement before destructive work when `has_pending()` is true. Destruction
with unfinished work terminates rather than discard a required continuation.
An exception in the material prefix, first-child selection, current-child
resolution, or post-child advancement records an explicit terminal failure.
That failure counts as pending, so neither a new begin nor deletion can replay
or discard partial native effects. Explicit resume rejects it. Only an
exception from the actual child current70 retains a resumable child frame;
native exception recovery beyond that continuation contract is not invented.

## Supported domain and validation

Null primary nodes, null Model geometry and zero section count are native
ordinary skip paths; children still run. Nonnull resources require live actual
owners, nonnegative extents and section0 with material profile D5E520. A Screen
table inheriting this slot does not prove that its primary node is a Model:
plain page roots may be cGroup, whose +180 is an integer capacity. Such roots
are rejected before reading that field as Model geometry. Corrupt storage,
unknown profiles, native allocation/SEH failure behavior and material-source
lifetime violations are outside the supported domain.

The isolated new source passed MSVC Win32 C++17 `/W4 /WX /EHsc /permissive-`
and `/fp:strict`, including the terminal-failure guard. Nine exact direct call
rows passed `verify_report_calls.py`; the one indirect child dispatch is
manually traced from its current payload/table/slot loads. The ordinary build
was invoked twice but failed MSBuild tracking-directory writes. A serial
`/m:1 /nr:false` Release Win32 build with `MSBUILDDISABLENODEREUSE=1` passed,
followed by the existing CTest `reconstructed_math` test (1/1). The new module
was separately strict-compiled; that baseline build does not link it yet.
Native differential tests are not enabled in this fresh worktree. The exact
results are recorded in `reports/gui_widget_clip_refresh.json`.
No new tests were added. This worker does not edit owner integration or CMake;
until those integrator changes land the executable cannot reach this module.
No focused execution, native ABI differential or game/render validation is
claimed, and no explanation about a particular game frame is inferred.
