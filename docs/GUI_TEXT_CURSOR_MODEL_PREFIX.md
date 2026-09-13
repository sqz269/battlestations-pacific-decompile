# Text cursor Model acquisition and caller cleanup

`00AB8910..00AB8C24` contains 236 listed instructions and zero flow gaps.
The entry takes the SAME Text in ECX, takes no stack arguments, and ends in
RET at `AB8C24` (789 bytes including that instruction). Its three direct callers
are `AB9246`, `AB95FD`, and `ABB5FA`; the last reloads destination ESI into ECX
at `ABB5ED`. Names below describe reconstructed behavior, not recovered symbols.
This is a source interface for MSVC Win32, not a native Text entry replacement.

The old Cursor call used the untracked auxiliary-Model helper. That helper
could hide a completed native Model from the cursor frame if canonical host
registration failed. `GuiTextCursorAcquired` now inherits the SAME shared
`GuiTextAuxiliaryModelAcquired` bookkeeping used by the existing section frame.
It does not create another Model, owner, reference count, or registry. The
tracked Cursor overload and tracked Shadow overload share one implementation
inside `GuiWidgetOwnerRuntime`, with independently recovered sites and names.
The legacy string-taking overload remains for unrelated callers.

## Native prefix and publication

| Site | Proven operation |
| --- | --- |
| AB8935 | Test live Text+184; nonnull skips the whole producer. |
| AB8947 | B74EB0 overwrites incoming ECX=184h with the canonical Model pool at 1090054, then tail-calls B74D00. Save raw slot in ESI and `[ESP+14]`; state0 is armed at AB8954. |
| AB8963 | Construct the eight-byte temporary name header from D5C788, `gui_cursor`. NativeString constructor consumes one stack argument with RET4. |
| AB896C..AB8979 | Set temporary bit1 and state1 only after the name constructor returned. The call's push changes the displayed ESP offsets. |
| AB897D | ECX is the allocated Model slot; stack argument points to that SAME name header. B75030 constructs the actual Model and returns it with RET4. |
| AB898C | Publish returned EAX to the SAME live Text+184. The null allocation branch publishes null as native does; the subsequent dereference is outside valid inputs. |
| AB8992 | Disarm the caller's state before normal name destruction. |
| AB89AB/AB89B2 | Resolve the actual string pool, then release captured `(data,length+1,1)` with RET0Ch. The earlier singleton call takes none of these pending stack arguments. |
| AB89B7..AB89CE | Reload current +184, clear flags138 bits1/2, read current Text+4C, reload current +184 and parent it via B6E680. |

The helper allocates a provisional host ModelRecord before native acquisition,
then prepares the actual owner at the returned slot in the SAME map. Its
temporary uses the explicit `buffers.strings` domain; a nonnull Model
`actual_names` provider must be identical. Native construction precedes
canonical NativeModelReference registration. If that host registration throws,
the live ModelRecord, actual owner identity, and live temporary remain retained
and observable in the SAME caller frame. No Model destructor, slot return,
publication, invented native exception address, or successful admission follows.

After successful registration, the creator transfers into +184 before temporary
name cleanup. Frame creator cells are cleared before the noexcept string-release
callback. The helper accesses neither the ModelRecord nor its node after that
callback, which may reenter through the publication and retire it. The caller
then reloads the current field as native does. Host metadata failures before
construction remain distinguishable from failures after the native constructor.

## Cursor's own FH3 evidence

The prologue loads handler CB7FC9. Its exact raw bytes cover
`CB7FC9..CB7FD2` (10 bytes, two instructions); Ghidra has no function there.
They load FuncInfo DEEDBC and tail-jump to the existing CRT dispatcher BF6B43.
The read-only bytes at DEEDBC identify six states and the unwind map DEEDE0.
No handler function was invented or created in Ghidra.

| State | Next state | Action and full body |
| --- | --- | --- |
| 0 | -1 | CB7F90..CB7F97: ECX=`[EH EBP-58]`; tail B748C0 at CB7F93 returns the unconstructed Model slot. |
| 1 | 0 | CB7F98..CB7FB0: test bit1 at `[EH EBP-5C]`, clear it at CB7FA4, form SAME header `[EH EBP-54]`, tail 41DD20 at CB7FAB. |
| 2 | -1 | SAME conditional-name funclet CB7F98; no state2 store occurs on this normal body. |
| 3 | -1 | CB7FB1..CB7FB8: captured raw mesh at `[EH EBP-58]`, tail B72F70 at CB7FB4. |
| 4 | -1 | CB7FB9..CB7FC0: SAME header at `[EH EBP-54]`, tail 41DD20 at CB7FBC. |
| 5 | -1 | CB7FC1..CB7FC8: SAME header at `[EH EBP-54]`, tail 41DD20 at CB7FC4. |

This table was read independently of Shadow's DEED58 table. On a Model
constructor exception, the tracked helper clears the conditional-name bit,
destroys the header, removes only prepared/dead host associations, and returns
the raw canonical slot. The actual Model constructor owns its internal node
unwind. A second exception while returning the raw slot terminates, rather than
replacing the first exception with a fabricated successful cleanup.

AB8910 reuses one temporary header for `gui_cursor`, `simplecolor.mvfm`, and
`GuiCursor.mshd`. The cursor frame now does likewise. A successful name constructor
arms state4 or state5; a thrown name constructor does not arm destruction.
Normal release disarms the state before calling the noexcept storage release.
Failures at an armed declaration/material call destroy this SAME header through
the corresponding Cursor funclet action. Header bits remain intact after
destruction, matching 41DD20; booleans track liveness without clearing native
bytes. The next native name constructor clears the header before allocation.

## Retained tail and limits

The normal tail remains on the existing actual renderer38/5C, mesh, material,
layout and Text-identity providers. The renderer is freshly read after format
name release. Declaration is released before vertex; material before layout;
section is appended after layout; section then mesh creators are released.
There is no index factory in AB8910. Numeric direct-call rows, every indirect
site, and their register/stack operands are in the accompanying report.

This packet adds native Model-prefix cleanup and caller temporary-name cleanup;
it does not claim complete factory-internal unwind for composed mesh, section,
declaration, or material producers. In particular state3 lies inside the current
mesh factory. A throw records that composed entry, not a fictitious raw mesh
identity. Renderer declaration/vertex acquisitions continue using their existing
tracked records. Material/effect factory migration is a separate integration.
Completed native creators and publications persist across interruption; a failed
cursor frame cannot rerun. This is diagnostic retained state, not a native SEH
ABI, recovery API, automatic rollback, or successful failed-constructor admission.

The existing tail's matrix and x87 geometry-argument projection is preserved;
this prefix packet does not establish an additional floating-point equivalence
claim. Build and the focused actual Model-prefix probe are recorded in the JSON
report. The probe uses actual Model/pool/string allocations and interrupts the
Model's name copy; type/profile globals are explicit fixture inputs. It does not
exercise complete AB8910, renderer/material/layout work, actual Text admission,
post-construction host registration failure, or the game.
