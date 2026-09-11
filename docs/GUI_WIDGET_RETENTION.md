# Widget parameter-source retention and direct deletion

The native material retains the widget's actual +04 word, but that reference
does **not** defer every widget destruction path. The page manager and base
widget destructor also call current deleting virtual+04 directly. The current
`GuiMaterialBindingServices::retain_widget` cannot therefore be implemented
coherently as an isolated shared-pointer token over the existing unique page
tree. This packet is analysis-only; it adds no counter, token or lifetime API.

## Verified ownership paths

AA9390 writes base CEB130, initializes actual widget+04 to 1 at AA93B5, then
publishes base widget table D5C130. B18A40 stores the raw source at material+0C
and, when exact byte+10D is nonzero, increments that source's actual +04 through
InterlockedIncrement. It first releases an old retained source, even when old
and incoming are equal. Incoming must independently survive that release;
neither native code nor a host adapter may silently insert a protective retain.

At zero, the old source's CURRENT virtual0 runs. The common BD30E0 entry calls
the object's CURRENT virtual+04 with flag 1. BD30E0 itself does not decrement a
count. It reaches the actual derived destructor and the matching physical pool
return, not a host `GuiWidgetOwner` delete or a generic native base destructor.

| Current table | virtual0 | deleting virtual+04 | destructor | pool-return call/global |
|---|---|---|---|---|
| D5C130, base widget | BD30E0 | AA9A90 | AA9730 | AA75F0 / F8BC94 |
| D5BBF8, derived identity not established here | BD30E0 | A9E210 | A9E130 | A9B150 / F8BC10 |
| D5C4C0, Icon | BD30E0 | AB6100 | AB5E70 | AB2150 / F8BDA0 |
| D5D130, FrameBox | BD30E0 | AD29B0 | AD2880 | ACF610 / F8C188 |

The initial packet hypothesis that A9E130 was a deleting wrapper was wrong.
It is the D5BBF8 derived destructor. Each verified 32-byte deleting wrapper has
ECX=this, one stack flags DWORD, EAX=original address and RET4. It calls its
destructor first and returns the SAME physical object to that class's pool iff
flags&1. None reads or decrements widget+04. Class allocation extents and the
four pool implementations are dependencies, not reconstructed by this audit.

Icon AB5E70 publishes D5C4C0, calls AB5700/AB50E0, releases the shader string,
calls AB5850, then AA9730. FrameBox AD2880 publishes D5D130, releases its shader
string, calls AD21B0, then AA9730. Those derived helper bodies remain required;
the trace does not replace their actions with generic container destruction.

## Material destruction can reenter the widget

The independent native-material packet owns B192F0/B194B0. Read-only assembly
and that worker's contract agree: material destructor B192F0 publishes D5E520,
releases/clears effect+7C, then walks textures+10 using its live signed16+34
end. Only then it reloads byte+10D and source+0C. If retained/non-null, it
decrements the source's actual +04 and calls CURRENT virtual0 on zero. The
material's +0C is cleared **after** that callback. Parameter records are released
later; base CEB130 is installed last (also by state 0 unwind).

Thus a final material-owner release can reenter widget destruction while
material+0C still names that widget. A host token must not pre-clear the raw
field, queue a delayed release or move this release ahead of effect/textures.
B194B0 calls B192F0 and, for flags&1, returns its 0x114-byte material slot
through B17A80/F8D3AC. The material packet owns that implementation; this packet
does not edit or re-register its addresses.

## Direct page/tree destruction is a separate terminal path

AA31F0 removes the page from the manager vector, calls current virtual+20 at
AA326A, reloads the current table, then calls deleting virtual+04(1) at AA3276.
There is no widget+04 decrement or zero-count check between them.

Base destructor AA9730 publishes D5C130 and calls AA8320 at AA9760. AA8320
recursively calls children's current+20 and unlinks/releases widget+4C through
B6DFA0 before clearing that field. For glyph owners it also runs the established
secondary-node path. AA9730 then walks the current child list and calls each
child's current deleting+04(1) at AA9797, again without a count check. The child
destructor removes itself from its parent's list, so the parent reloads the
list head/count after each call. A copied list or a count-driven deferred
child deletion would change this contract.

AA9730 detaches from parent+70, performs any remaining +4C release, disposes
the +88 vector's objects, frees vector/list storage and calls BD30F0 after
publishing D5C104. Its actual RET is AA99B8; no late +04 decrement was found in
that complete body. AA9A90 then returns storage regardless of residual count
when called with flag 1. Direct destruction therefore is not interchangeable
with releasing a creator reference.

For a simple live widget with constructor count 1 and one material retain, the
count is 2. If logical scene release destroys the last mesh/section/material
owners, the material release takes it back to 1 and direct deletion follows.
This is an illustrative count path, not proof that all queued resources drain
at that point. If a retained draw/material still survives, the observed direct
deleting path does not wait for it. The complete frame/queue contract that makes
such a case safe remains to be established. This audit does not infer a native
leak/bug or manufacture an asynchronous deletion policy from that uncertainty.

## Current host gap and minimal coordinated next packet

The current tree has no actual native widget+04 owner to borrow:

- `GuiWidgetBaseExtraFields.references_04` is an independent int32 initialized to 1.
- `GuiLayoutPage.reference_count` separately projects the root page's+04 and is
  incremented on the existing-page AA5840 path (`gui_layout_loader.cpp:734`).
- `GuiWidgetOwnerRuntime::widgets_` owns unique_ptr companions; owners borrow
  `GuiLayoutWidget&`. `GuiLayoutPage::root` and each layout's children are
  unique_ptrs. `before_destroy` calls `retire_tree` while fields still exist.
- `retire_tree` performs logical scene release, then `erase_tree` runs derived
  cleanup, the base release pass, child erasure and owner erasure. It does not
  consult either semantic widget/page count.

Aliasing a `shared_ptr<const void>` to `&GuiWidgetOwner` without ownership would
dangle. Capturing the whole page in every material could create a new ownership
cycle through page/widget/model/mesh/material and would postpone native direct
deletion. Incrementing either semantic integer or adding a third count cannot
make layout memory survive its unique owner. Retaining the model alone does
not preserve the material's borrowed widget fields or ancestor clip sources.

A coherent next packet requires coordinated ownership of
`gui_widget_owner.hpp/.cpp`, `gui_layout_loader.hpp/.cpp` and the material
bridge; those shared files were deliberately left unchanged here:

1. Establish one canonical native widget identity/+04 for each class and make
   root page and widget views borrow that SAME word. Eliminate the independent
   page/widget count projections when introducing actual storage; never add an
   extra lifetime counter to compensate. Preserve the existing layout/tree as
   the single source of its fields.
2. Bind CURRENT virtual0/+04 and complete derived teardown/pool return for
   supported profiles. Include AA9730's live child-list mutation and the exact
   derived helper contracts listed above. Do not dispatch by a remembered C++
   type after the native table changes during destruction.
3. Preserve both terminal routes: zero-reference virtual0 and explicit manager/
   parent deleting+04. Determine and enforce the actual point at which material
   parameter users stop before direct layout destruction. Prove the relevant
   render command/resource drain or retain an explicit unsupported boundary;
   do not defer deletion, clear owners early, or insert a protective cycle cut.
4. Only then expose the material bridge's retained alias token: `.get()` must
   identify the SAME widget owner, and acquisition/release must act on actual
   widget+04 with current-profile dispatch. The owner and every borrowed layout/
   clip-ancestor field must remain valid for all permitted parameter reads.
   B18A40's release-before-acquire and B192F0's reentrant clearing order remain.

Until those changes are coordinated, the existing `retain_widget` requirement
stays unresolved and must not be satisfied by a placeholder owning token.

## Evidence boundaries and validation

All Ghidra batches verified `bsp` at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, x86 LE32, image base 00400000 before the query.
Ghidra remained read-only. The report captures old names/comments, body/ABI
limits, table slots and the material lease collision.

AA9390 has 83 listed instructions and no gaps, ending 9513 (RET4 starts 9511).
A9E130 has 40 listed instructions with a three-byte free-call continuation gap
at A9E183..A9E185; disk decoding restores `ADD ESP,4` before the complete RET
at A9E1D0. AA9730's saved function ends AA9950 after a false no-return free call;
the independently decoded continuation ends at AA99B8. Its other six-byte
listing gap AA984A..AA984F is unreachable alignment after JMP AA9850. No flow
or library annotation was changed. Full base/derived destructor, constructor
and four deleting-wrapper byte spans match the installed PE; hashes are in
the report. This is source/assembly analysis, not a runtime or destructor
differential test. No C++ changes, build, tests, ABI compatibility or game
validation are claimed.
