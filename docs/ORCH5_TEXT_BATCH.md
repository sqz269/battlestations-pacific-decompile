# Orch5 Text and glyph ownership work

Addresses: 00AA83A0 00A9BD50 00AB8F00 00ABB000 00AB6B50 00AB6C30 00AB7200

The integrator remains `agent/orch5-20260911`; workers use separate worktrees
and leases. This batch continues the published menu owner/layout/map work.

| Module | Proven implementation and remaining boundary |
| --- | --- |
| `gui_widget_detach` | Actual retained GUI/native-node detachment with ownership transfer. Text deleting-destructor/lifetime transport remains required. |
| `gui_text_ellipsis` (`68ac50d5`, then primary correction) | Native width arithmetic and suffix trimming; cached/localized source preparation is an explicit fragment before real content/geometry and final color assignment. |
| `gui_text_style` (`5696c25a`) | Actual model/material color and shadow-parenting operations. State80 covers hidden widgets or non-hidden indices0..3; unchecked native memory-index inputs are excluded. |

The primary reviewed the native bodies and actual-owner bindings. Direct
`FISTP` now handles nonfinite/overflow inputs using the caller's x87 masks;
the worker's added finite/range guards were removed. A focused public ellipsis
probe verifies NaN width, native low16/dot-reservation wrapping, invalid flag
and control-word restoration. A second focused probe verifies detachment
against actual retained GUI/model objects, including allocation identity and
both GUI/native hierarchy changes. These are bounded fixtures, not native
whole-function differential or game validation.

The strengthened call verifier passes 92 numeric rows:90 direct CALL sites,
plus2 indirect sites whose targets rely on separate vtable evidence. One
additional detach xref at005D1F2E has no live containing function; its direct
target bytes are verified without attributing it to the nearest candidate.

Ghidra's false `CALL_RETURN` at00A9BDC0 hid seven bytes after `_free`, making
the list helper's pseudocode stop after one match. The locked flow tool repaired
the continuation, saved the project and refreshed its export. Raw Text state80
was defined over00AB7200..00AB7283 after disk/live byte agreement. Six affected
names/comments were applied and refreshed; prior values and definition/flow
events are retained in the batch's reports.

The existing registered Win32 targets and both existing CTests pass after the
shared Text byte-field change (`local/orch5_text_existing_targets_build.log`).
Each new source compiled with strict MSVC Win32 flags. Normal registration of
the new modules is still pending: `cmake/startup.cmake` is leased by
`agent/orch3-20260910:orch3_gamepad_event_owners_ad`, so it was not modified.
This existing-target build does not claim that the three new sources were
compiled by the normal CMake build. No new persistent test targets were added.

The content prefix/glyph clear (`c79cb75b`) depends on the next actual buffer
initializer. Workers are closing buffer/section/material creation, canonical
Text lifetime and the nonempty content continuation. The canonical factory
must remain disabled until its required virtual behavior is implemented;
these modules do not yet establish a functioning menu or gameplay rebuild.
