# Actual Text glyph-child tail

`begin_gui_text_glyph_child_tail_00ab98f0_fragment` composes the successful
`AB9D33..AB9FAD` child-creation tail using the existing Text, Model, mesh,
material, node, font and string-pool owners. Its entry is after the ordinary
quad/index writes, optional-character gate and four UV-pair clears. The
earlier mapped writer remains responsible for those operations. This tail
does not rerun them or unlock either mapped stream.

The whole original `AB98F0` receives ECX Text and ten DWORD stack arguments,
then returns with `RET28h` at `AB9FC0`. The new continuation takes the already
established `GuiTextGlyphChildCallFrame`, keeping its original position
allocation and other arguments borrowed. Its last covered instruction is
the call at `AB9FA9`; successful completion reaches `AB9FAE`.

## Concrete native sequence

| Call or stores | Composition and ownership |
| --- | --- |
| AB9D38 / AB9D4F | Actual Text pool allocation and primary-null canonical factory construction |
| AB9D70 | Publish that same child pointer in the parent's glyph vector before cloning |
| AB9D75..AB9D87 | Resolve current reference Text and its current Model; execute its flags26,parent0 clone |
| AB9D8C | Bind the acquired Model and transfer its one creator reference to the child; no extra retain or release |
| AB9D93 / AB9DA5 | Ensure actual draw sections, then dispatch current visibility34 with true |
| AB9DB2 / AB9DB7 | Read current parent listener, store listener/flag0, then mouse-hit=true |
| AB9DBA..AB9DC9 | Resolve reference Text again and use its live font-name member |
| AB9DCE..AB9E05 | x87-load/spill current shadow offset, zero RGB, current one for alpha; configure shadow enabled, position0 |
| AB9E13..AB9E40 | Construct actual pooled shader temporary, set shader/invalidate, then return its buffer through the same pool |
| AB9E51 | Construct actual eight-byte UTF16 header using low16 of original argument10 |
| AB9E61 / AB9E71 | Actual content builder then current color50; keep the UTF16 buffer alive across both |
| AB9E73..AB9E91 | Return current UTF16 data with current length*2+2, leaving header bytes untouched |
| AB9E9B | Transfer the detached layout to the existing parent owning list and perform actual node parenting |
| AB9EA0..AB9EC0 | Read current CE3800 once into both pivot cells, then actual current30 |
| AB9EC2..AB9FAD | Existing actual glyph-child position tail, including live reference reloads and original argument2.x |

Live Ghidra bytes at `CEFD78` establish the exact terminated shader spelling
`GuiFontBilinear.mshd`. Services bind the corresponding current literal and
constant storage. The shadow offset keeps its native x87 transfer; pivot and
shadow alpha use their native scalar-copy paths.

The one concrete `GuiTextRuntimeImplementation::submit_utf16_00ab6ab0`
composition already executes `ABA8D0` followed by current50. It therefore
supplies both AB9E61 and AB9E71 exactly once. The caller's actual UTF16 header
and pool buffer remain on the outer glyph-child frame until that complete
submission returns. The shader temporary is destroyed earlier, before this
UTF16 allocation. Its `std::string` argument projection is only the existing
typed shader setter's input, not another shader cache or native resource.

## Pending work remains owned

`GuiTextGlyphChildTailContinuation` owns the detached child allocation until
attachment. It also holds the acquired Model/mesh/section/material creators,
both actual string headers, original call arguments and current native phase.
The parent's glyph vector is a borrowed alias to that one allocation. There
is no second widget hierarchy, Model registry or reference counter.

`pending_content` leaves the same factory implementation's submission and
content frames intact. The caller can inspect `pending_content()` and must
complete the exact inner glyph-child operation before invoking the child's
`resume_after_glyph_child()`. If another inner child is pending, all outer
frames and UTF16 storage remain alive. Only when that implementation has no
pending operation may `resume_gui_text_glyph_child_tail_after_content`
release the UTF16 buffer, attach, pivot and position. Calling this tail resume
while the implementation is still pending has no effects.

A positive source light list returns `point_light_owners_required` after
the destination Model has been constructed, preserving its creator and the
already-published child. `domain_required` stores the thrown exception and
the current phase without repeating a callback or claiming the native call
completed. These results cannot use the content-only resume entry. The full
caller and all borrowed owner/service lifetimes must remain retained; disposal
of a started pending frame terminates instead of silently destroying an
incomplete published child. No rollback or generic exception resumption is
claimed. Native faults, failed allocations, destructive reentry and exception
ABI are outside the component modules' supported successful domains.

## Validation and remaining boundaries

The entire 1,747-byte live function matches the installed PE. The 635-byte
tail and shader literal were checked separately. Exact hashes, instruction
boundaries and the preserved prior name are recorded in
`reports/gui_text_glyph_child_runtime.json`. Ghidra remained read-only.

The new translation unit passed strict MSVC Win32 C++17 `/W4 /WX /O2 /MD`
compilation using the integrating agent's attachment/Model APIs and the Text
factory/mesh workers' current headers. The existing registered build and both
CTest cases also passed. This worker intentionally did not edit shared source
registration; integrated linking of the new module belongs to the integrator.
No new test suite, original-tail execution fixture, renderer run or game
validation was performed.

This is an executable composition for the existing supported owner domains,
not a byte-for-byte native Text/string/list layout or original calling-
convention replacement. Component boundaries such as actual renderer/font
availability, supported content formats and positive-light ownership remain
explicit. The recursive completion driver that links mapped-writer pending
frames to this tail is a separate integration task.

## Integration correction

`docs/GUI_TEXT_CHILD_DRIVER.md` records the now-integrated recursive driver and
source registration. Resume marks cleanup in progress before releasing the
UTF16 allocation, preventing callbacks from repeating release/attachment.
The content frame also guards same-frame driver and builder reentry. Combined
Win32 compilation and both existing tests pass; see
`reports/orch5_text_factory_batch.json`. Native-tail execution and game/render
validation remain unperformed.
