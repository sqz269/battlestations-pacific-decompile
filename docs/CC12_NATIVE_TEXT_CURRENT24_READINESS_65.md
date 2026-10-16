# CC12 native Text current24 readiness 65

Baseline: `f30e0bd9348c7d9273c0639c698a728ccfd4db55`, merged with checked
exit status in the worker worktree. This packet is a read-only Source/live
analysis receipt. It changes no Source, Ghidra state, dispatch or activation.

## The Text target is already reconstructed

Live BSP queries verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Text table `00D5C6C8` has these exact words:

| Slot | Word address | Live bytes | Target |
| --- | --- | --- | --- |
| current24 | `00D5C6EC` | `A0 E0 A9 00` | `00A9E0A0` |
| constructed74 | `00D5C73C` | `00 77 AB 00` | `00AB7700` |
| loaded78 | `00D5C740` | `A0 6A AB 00` | `00AB6AA0` |

`00A9E0A0` is the complete one-byte body `C3` (`RET`), already reconstructed
as `base_gui_layout_finished24_00a9e0a0` in `src/gui_layout_finished.cpp`.
It does not inspect ECX or widget data and does not establish a return value.
The current24 caller supplies ECX=the same widget, with no stack arguments;
this void Source interface is a typed companion projection, not an Original
vtable or drop-in ABI replacement. Its descriptive name remains a hypothesis.
There is no missing Text-specific body to recover and no reason to invent one.

The actual Source gap is the switch in `dispatch_gui_layout_finished24`:
Screen, Group, Icon and FrameBox use this existing leaf, and ClipBox calls its
existing `update24_00ace120`, but type3 Text reaches the unsupported-type
exception. `GuiTextRuntimeImplementation` is the existing final concrete Text
companion. There is no separate `GuiTextTypeImplementation` or Text current24
method that needs to be duplicated. `GuiLayoutFinishedHost` already supplies
the terminal callback from the transform walk and forwards the earlier real
host operations. This receipt establishes no normal-host installation of it.

## Ordering must stay at the existing seam

Live `00AA8710` assembly establishes ECX=widget/no stack parameters and walks
the child list first, in stored-count/list order. Only after child recursion
does it process the current widget's authored/widescreen position, optional
listener virtual+1C, `00AA7220` transform, and `00AA70E0` bounds. At `00AA87A1`
it reloads the current vtable; `00AA87A3` reads slot+24, `00AA87A7` restores
ECX=the same widget and `00AA87AB` tail-jumps to that target. Existing
`apply_widescreen_layout` preserves this child-first terminal-callback order.
The recovered manager `00AA4EF0` supplies a separate relayout pass; current24
is not a replacement for loading and must not be inserted into loaded78.

Loaded78 is independently recovered: `00AB6AA0..00AB6AA4` is solely
`JMP 00AA7170`. The latter calls the CURRENT type's slot+60 with argument zero
before tail-jumping to bounds `00AA70E0`; it is not a base-only active store.
Existing `load_gui_text78_00ab6aa0` and
`GuiTextRuntimeImplementation::loaded78` already implement this over the same
owner/lifetime and reject unfinished runtime operations.

The live `00AAA710` tail orders child parent/list publication and node
parenting before current74, then visitor descent, the child's current18
property reader (including descendant loading), visitor ascent, then current78.
The existing Source builder likewise publishes the SAME child into the
parent, invokes constructed74, reads base properties, recurses, runs derived
properties and finally loaded78. A Text property/content continuation must
complete before that loaded78 call; neither a current24 leaf nor its dispatch
admission completes a pending glyph child, content submission or child70.

## Failure and borrowed lifetime remain separate obligations

`GuiTextRuntimeImplementation` retains pending constructor/property/content/
clip/copy state. Its destructor terminates if a pending operation is discarded.
`GuiWidgetOwnerRuntime::retire_tree` preflights active owned operations over
the whole tree before releasing scene nodes and destroying companions. These
checks do not create ownership of public page/layout graphs, clip ancestors,
material parameter source pointers or external native Text references.
Serialized callers must retain the same owners, layouts, child-list payloads,
factory/resources, script data and clip source ancestors through all pending
continuations and actual parameter consumers. They must not replay an entered
failure or treat a dispatcher success as permission to unwind/retire that graph.

At this baseline the legacy loader still keeps page/root/script locals until
late publication. Its `LoadScope` failure callback runs after later-created
locals can already unwind. Packet57's failure-retention gap is therefore not
closed by current24 admission. Worker63 reports a separately pending, opt-in
retained loader operation with a required noexcept failure sink that takes the
SAME unique entered graph before rethrow, a sticky failed state and process
retention. That report is coordination information, not integrated or verified
packet65 code, and does not claim ordinary cleanup or successful normal page
retirement. Packet65 makes no loader edits.

## One concrete next packet

Approved next packet67 should claim only `include/bsp/gui_layout_finished.hpp`,
`src/gui_layout_finished.cpp`, its unique receipt files, and the exact related
current24 evidence addresses. Include the complete Text runtime type in the
`.cpp` and admit type3 only when its companion is the final
`GuiTextRuntimeImplementation`; route it to the EXISTING `00A9E0A0` leaf.
Preserve both owner/layout type checks, ClipBox's existing update, every other
supported profile and all mismatch/unsupported exceptions. Update only the
public supported-profile comment. No new virtual method, factory/loader hook,
native table patch, no-op, CMake entry or normal activation is needed.

The meaningful checks are the verified Text table/leaf/caller evidence, review
of this narrow dispatch diff, and canonical MSVC Win32 build/existing checks.
A dummy full Text factory or a test that merely repeats the switch would add
no ownership, ABI or runtime evidence. Packet65 itself ran no build or tests:
its two tracked output files are documentation only. No game, native drawing,
full-page load, failure retirement or Original ABI compatibility is claimed.
