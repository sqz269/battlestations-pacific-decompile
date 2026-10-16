# CC12 native Text current24 dispatch 67

Parent: `a2b8a08c52d07e8ed84c2d0db242c282822d15b2` (packet65), over Root
`f30e0bd9348c7d9273c0639c698a728ccfd4db55`. This packet closes the established
Source dispatch omission only. It does not activate normal native Text.

`dispatch_gui_layout_finished24` now accepts type3 Text only when the SAME
owner's companion is the existing final `GuiTextRuntimeImplementation`.
The normal shared base-target path then calls the existing
`base_gui_layout_finished24_00a9e0a0`; no Text-specific method or new no-op is
introduced. The complete runtime type is included only by the `.cpp`. Public
header changes just add Text to the documented recovered-table/profile lists.

The live evidence recorded in packet65 is exact: Text table `00D5C6C8` slot+24
at `00D5C6EC` contains `A0 E0 A9 00`, selecting `00A9E0A0` with complete body
`C3` (`RET`). The caller `00AA8710` restores ECX=the same widget and tail-jumps
the current slot without stack parameters. This Source void companion call
does not define an Original return value or provide an Original vtable/ABI.
The descriptive function name remains a hypothesis, not a recovered symbol.

Both existing type-tag checks and the common mismatched-companion failure are
unchanged. Screen/Group/Icon/FrameBox admissions, ClipBox's same-owner
`update24_00ace120`, and every unsupported-type failure remain unchanged.
The implementation apart from the one complete-type include and the three
Text-case lines is equal to the parent after newline normalization.

The terminal seam is unchanged: `apply_widescreen_layout` visits children
first, applies the current position, notifies the listener, recomposes the
transform, refreshes bounds, then invokes `GuiLayoutFinishedHost`'s current24
dispatch. It is still a serialized live-owner pass with stable nonnull child
payloads. Text loaded78 remains the separate existing `00AB6AA0` tail into
`00AA7170` (CURRENT type60(false), then bounds). No loading or child order,
pending-operation frame or callback is moved or completed by this addition.

The caller still retains the canonical owner/runtime, SAME Text factory,
layouts/child list, script data, font resources and clip source ancestors
through pending continuations and the last actual material consumer.
Entered failures must remain retained; a completed current24 leaf does not
authorize retry, rollback, page destruction or resource retirement. Packet65
records the distinct legacy-loader and lifetime gaps, and pending packet63 is
not part of this change. No factory, loader, host activation, CMake, Original
global, Ghidra annotation, ledger or native body is changed here.

Canonical `scripts/build.ps1` passed for MSVC Win32 Release, including all
three existing checks: `reconstructed_math`, `native_math_differential` and
`tool_tests`. The primary agent's complete two-file Source review passed.
The companion JSON records Source/build hashes and the unrelated observed
`spawn_request_id_matches` duplicate-definition linker warning. No new tests or dummy
Text factory are added: the bounded case is checked by live target/leaf/order
evidence, preservation audit, Source review, and the canonical Win32 build
with its existing checks. This does not establish a full native page load,
drawing, failure retirement, gameplay or binary ABI compatibility.
