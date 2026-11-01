# DEFAULT Text lifetime with the actual Page context

Source148 adds one constructor to `NativeGuiTextDefaultWidgetLifetime`. It
borrows an already prepared `NativeGuiWidgetLifetimeContext` and its actual
`NativeGuiWidgetLifetimeBindings` dispatcher. This is the first slice of the
approved plan148, based on Main `deb39273101dda6b8a7b43874a6a415fab899232`.
It adds no Page caller, Native body, activation or runtime claim.

The existing constructor still owns its context and uses this helper as its
dispatcher. The new constructor leaves that optional owned context empty,
retains a reference to the supplied context, and retains the separate actual
dispatcher. `context()` returns that same supplied object. The dispatcher must
be distinct from the helper delegate, and `context.bindings` must name that
dispatcher. These are current C++ Source roles and layout changes only.

Both routes authenticate the canonical ready Text process, the exact base-table
view from the supplied RO owner, the Element-lineage array address and the Text
descriptor cell address. Values copied from those cells do not satisfy the
address checks. The same checks run before record binding and during receiver
authentication, after the existing independent failure/disposal guards.

Binding still requires the same factory/raw93/raw122/independent Model record,
sole owner registry, RO domain, construction lifetime, retirement base, resource
context, point-light runtime and glyph node runtime. `glyphs.children` must name
the retained dispatcher: this helper for the standalone route, the supplied
dispatcher for the external route. Binding also checks the canonical Text
F8BDF0 pool, actual geometry owner registry, actual node/raw-string pool and
shared raw-string process/storage cells, using the existing factory identity
requirements. Its writable records must not overlap the borrowed external
context or the known dispatcher subobject. No context or process cell is copied.

The existing one-shot binding, constructor-prefix/bound/retiring histories,
sticky failure handling and owner retention obligations remain in force. The
caller must keep the actual dispatcher, context, helper, diagnostics and complete
factory/Model/section/string/renderer/retirement/companion graph stable through
all callbacks and any failure. Metadata alone cannot establish that custody.
This change adds no cleanup, retry or failure recovery policy.

The actual A9E070 and AB83D0 predicates are unchanged. In particular, a genuine
true canonical Text-descriptor result still poisons and throws before the
legacy188 cleanup route. DEFAULT-only empty, unparented, nonprimary, nonshader
admission remains mandatory; child/timed dispatch remains rejected. The existing
native list body and captured list/pointer-cell order are unchanged.

Strict MSVC x86 compilation covers the changed Core helper and its sole existing
direct Game caller, `game_native_raw_default_text_owner.cpp`. The official
registered Win32 Release build and its existing checks are recorded in the
sealed candidate. The candidate also records actual Core/Game objects, archive
and link inputs. The external overload is currently unused; compiled object
emission does not establish final executable retention or production reach.
No reference anchor, new test, probe, Game run or Ghidra mutation is added.

Actual Page Type3 birth and publication remain unbound. A later slice must
provide the private Page borrow/delegation seam, genuine pending and completed
`NativeGuiTextRawModelReference` records, the complete DEFAULT factory graph,
actual frame/opaque/sampler inputs and shader-cache/Lua entry scope, then the
separate canonical companion binding. No legacy Text/GuiWidgetOwner cast or
extra retain is permitted. Text Type3 uses F8BDF0; FrameBox Type18/F8C188 is a
separate producer.

Nondefault Text property/hierarchy/lifetime arms, the typed full-DWORD whole129
caller and its captured-child146 borrow, Page Font production and the genuine
queued Text/Font/section/material last-use cut remain separate unresolved joins.
All nine GUI selectors stay off. Native/vendor body credit is zero; Original
ABI, FH3/fault, Origin, FP and gameplay validation are outside this slice.
