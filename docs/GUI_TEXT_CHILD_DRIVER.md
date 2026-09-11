# Text content and recursive child completion

Address: `00ABA8D0`. Native ECX is Text; one UTF16-wrapper stack argument;
RET4. The public functions use new C++ interfaces, not native binary ABI.

`GuiTextRuntimeContentServices::glyph_children` optionally binds the actual
`GuiTextGlyphChildTailServices`. Construct the shared owner, font, buffer,
material, string-pool and Text factory services first, then bind this pointer
before submitting text. Keep them alive with all pending operations. A null
pointer preserves the previous explicit glyph-child continuation boundary.

The existing prefix selects the actual single-line `AB9FD0` or wrapped
`ABA270` builder. When that builder reaches the optional child arm of
`AB98F0`, its continuation now owns the actual child tail. The saved call
retains the same glyph, stream mapping, index buffer, quad, vertex index and
stable float3 allocation. It uses the original placement code unit, not a
later reload through the cursor. Arguments 2, 5 and 6 alias the original
float3; the wrapped height preserves the native low word.

`complete_gui_text_content_children_00aba8d0` executes the tail and recursively
completes nested child Text content. It advances each existing builder only
after its actual child tail completes. The child implementation's existing
submission continuation performs the caller's final current50 exactly once;
only then does the tail release its actual UTF16 header allocation and attach,
pivot and position the child. The normal builder continuation owns stream
unlocks and the outer content material/color/shadow tail. No synthetic child,
duplicate scene tree or alternate resource-reference domain is introduced.

The locale comparator adapter and concrete `GuiTextChildDeletion` need not be
the same C++ object. `GuiTextContentCalls::glyph_child_calls()` identifies the
actual deletion transport. A forwarding locale adapter overrides this accessor;
combined implementations default to themselves. Both the prefix clear and the
factory/domain checks use that same reference. The constructor, wrapped
builder and scalar deletion therefore retain the same actual lifetime owner;
the scalar deletion identity check is preserved.

A host-only per-frame guard prevents same-frame completion requests during
native callbacks from reading a moved builder or replaying cleanup. Reentrant
driving leaves the frame pending; direct builder resume rejects reentry before
effects. The guard clears its pointer before the frame is destroyed. Child-tail
resume marks cleanup in progress before freeing its unchanged UTF16 header,
so attach/pivot callbacks cannot reenter and free it twice. Pending Text scalar
deletion is separately rejected before native phase/flag stores or effects.

Explicit pending states retain the same frame and allocations. Positive native
point-light arrays, missing services and undefined wrapped alignment/format
remain boundaries. A started child-tail frame cannot be abandoned; destruction
while incomplete terminates. Native faults, C++ allocation failure, destructive
callbacks and exceptions after moving a builder are not resumable transactions.
The loader still needs a retained outer frame if a property path suspends; only
the supported synchronous completion path fits its existing void protocol.

The integration report is `reports/gui_text_child_driver.json`. Independent
reviews checked ownership transfer, saved arguments, nested-frame lifetime,
final50 ordering and callback reentry. The combined Win32 build and existing
tests are recorded in `docs/ORCH5_TEXT_FACTORY_BATCH.md`. No new tests, native
glyph-tail execution, raw ABI compatibility, menu/render or gameplay validation
are claimed by this driver.
