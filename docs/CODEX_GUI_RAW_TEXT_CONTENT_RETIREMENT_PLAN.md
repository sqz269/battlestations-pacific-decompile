# Raw Text content retirement: bounded source plan

Read-only audit against post-86d `3595cc294` and the accepted one-lifecycle
qualification source `c0ef2def152b340d6edb6fef687fa45b6024a791`. The audited
93/118/122/widget/owner files have no diff between those commits. This document
adds no C++, Native body credit, activation, build, ABI or further runtime proof.
The accepted fixture is one real-provider **empty DEFAULT** Text lifecycle;
`GameNativeRawDefaultTextOwner` remains DEFAULT-only and the main route is OFF.

## Current boundary

| State | Current support and exact boundary |
| --- | --- |
| Completed AB9650/raw93 and AB8530/raw85 | One mandatory internal `+188` raw Model, real mesh/sections/GuiDefault; one persistent nested `NativeGuiTextRawModelRecord`. DEFAULT unwind stays empty and must not inspect a not-yet-live188. |
| Initial raw122 publication | `native_gui_text_reference.cpp:253-301` authenticates the same93/85/B75170/188 record, canonical owners/pool/type/string cells and empty payload. Keep this admission: later operations can mutate only AFTER successful initial binding; no rebind or broader ctor admission is needed. |
| Raw AB8250/AB8EE0 | `native_gui_text_retirement.cpp:90-106,131-146` additionally requires empty4C, parent/children/timed, glyph/cursor/content/shader and no internal-Model hierarchy/scene/retained/point-light state. It is not general nonempty retirement. |
| Private widget provider | `native_gui_text_default_widget_lifetime.cpp:226-265,286-307` repeats empty-domain checks and rejects the genuine TRUE canonical Text-descriptor branch. No false predicate, fabricated type ID or zero special case may replace that rejection. |
| Scalar custody | Existing raw122/AB8EE0 reads the real flags LOW BYTE only after AB8250, unbinds and calls saved disposal LAST. Owner count1 is sole-private admission, never a copied/decremented count. Text and independent Model actual scalar/pool-return/late-flags evidence must settle (`game_native_raw_default_text_owner.cpp:253-267`). |

Nonempty fields require separate treatment, not one relaxed DEFAULT predicate:

| Native field | Retirement meaning / remaining admission |
| --- | --- |
| `EC/F0` UTF16 text; `F4/F8` narrow source; `1A4/1A8` auxiliary wide string; `1C0/1C4` shader name; `1C8/1CC` font name | AB8250 already has genuine owned-header return bodies, but raw DEFAULT and private widget admission reject these headers. Require real canonical string-pool producers; no semantic string projection, external pointer or hand-authored allocation. |
| `1EC` cached shader | Owned actual effect credit. Existing release captures CURRENT CE2220 at AB828D, loads profile AFTER decrement, selects canonical D5E534/D61A00 table, captures CURRENT0 BD30E0, then uses the same owner terminal. Clear1EC only after return (`retirement.cpp:204-233`); no second decrement. |
| `108` Font pointer | Borrowed actual image; AB8C30 stores it without a Font retain (`field_storage.cpp:235-249`). DEFAULT constructs it null; the empty guard is not a font-borrower protocol. Real Font collection must outlive all uses and retire only after raw borrowers quiesce. |
| `180`, `184` | Borrowed associations; AB8250 does not independently release them (`gui_text_lifetime.hpp:31-41`). Cursor184 needs its genuine Model/parenting graph. Do not add a separate delete/decrement merely because these slots are nonnull. |
| `198/19C/1A0` pointer vector and glyph Text children | AB80C0 performs real detach, reloads current begin/end, captures child/table/current04, deletes flags1 and clears the CAPTURED slot only after return (`glyph_children.cpp:64-110`). Nonempty raw child/parent/list/scene recursion remains unsupported. Tidy194 is backing cleanup, not proof that children retired. |
| primary `4C`, internal `188`, metrics `18C/190` | Main4C needs a DIFFERENT stable raw Model record. Default2049 admits only its one bound actual Model (`node_dispatch.cpp:173-199`); do not route a primary/shadow hierarchy through that single188 provider. Nonzero layout metrics are also outside the present DEFAULT preflight. |

## Destruction schedule that must remain exact

AB8250 stamps the Text table, captures/releases CURRENT1EC, reloads CURRENT188,
calls B6DFA0 and clears188 AFTER return, clears glyph children, then returns
`1C8 -> 1C0 -> 1A4 -> vector194 -> F4 -> EC`, finally AA9730
(`retirement.cpp:301-346`). Header return captures DATA first, disarms the outer
EH action, loads LENGTH only for nonnull data, gets the actual pool00419CC0 and
returns through BD1510 with the live disabled cell (`235-263`). Native arithmetic
and EH-consumption order remain unchanged.

Crucial: normal header return does **not** zero the returned header. Final raw
boundary337 and later widget/base callbacks cannot reinterpret nonzero consumed
header bits as live ownership, read returned data, or require those bits to be
zero. Admission must use the SAME independent scalar destruction record and its
reached/consumed/returned actions. A consumed EH action alone is not cleanup
success; any escaped invocation retains the whole graph. Preserve the separate
host-metadata rejection boundary, native provider failure and genuine existing
unwind; never infer cleanup, retry, reset fields or add speculative returns.

## Preferred smallest next packet: owned members only

Prepare a library-only raw **owned-member retirement admission** over the
existing AB8250/AB8EE0 body. Still require empty4C/parent/children/timed/vector,
cursor/borrowed geometry and the same unattached mandatory188 Model. Admit only
genuinely produced owned headers and an authenticated cached effect; do not
claim a layout/content builder, a Font borrower, main geometry or rendering.

Proposed common API before implementation:

```cpp
enum class NativeGuiTextRawRetirementDomain { default_empty, owned_members };
// Final member of NativeGuiTextRawModelRetirementContext:
const NativeGuiTextRawRetirementDomain domain{
    NativeGuiTextRawRetirementDomain::default_empty};
// Keep require_default_before_scalar() strict and unchanged for existing owner.
// Add one explicit provider entry for the admitted owned_members scalar domain.
void require_owned_members_before_scalar();
```

The domain is fixed with the stable graph BEFORE pool effects, not retargeted
after construction. Initial93/122 publication and constructor-unwind admission
stay DEFAULT. The new entry authenticates the SAME complete factory/raw93/122,
canonical initialized current lineages/table aliases and quiescent real producer
records before parent effects. Widget callbacks admit bound/retiring member
states only; legitimate active base prefixes consult independent destruction
progress before any ended member read. Genuine TRUE Text/child/timed branches
still poison/throw. Reauthenticate after reached callbacks without rereading
ended storage or changing captured Native target/descriptor epochs.

Likely ownership: `native_gui_text_retirement.hpp/.cpp` (AB8250, AB8EE0) and
`native_gui_text_default_widget_lifetime.hpp/.cpp` (private admission over the
already-recovered A9E070/AB83D0/A9BD50 contracts), plus one packet evidence doc.
If new retained producer backing is introduced, raw122 known-borrowed-region
enumeration in `native_gui_text_reference.cpp:101-168,308-320` must include it
before entry; claim that file explicitly rather than weakening its separation
checks. No legacy interface, DEFAULT owner, host, CMake or main-switch change.
No new Native bodies are reconstructed by this admission packet.

Real providers remain the same Text process/pool, VFS raw THREE-cell string
domain and actual typed VFS string wrapper, real CE2220 cell, actual geometry/
effect/material owners/cache/table aliases, original imports/world/point-light
contexts, and the independent188 construction/B75170/481/e769/2049 graph.
Allocate all metadata/frames before entry; keep it outside Text pool storage.
Raw flags0, surviving Model credits or entered failure retain backing through
actual retirement; disposal still destroys ONLY the Text companion LAST.

## Named dependencies for nonempty main geometry and FE_initial

1. **Raw main4C transport, separate packet.** Frame worker's
   `b95c6fadcb50100bb007c88451b815f53cecc4e5`,
   `CODEX_GUI_RAW_TEXT_CONTENT_NEXT_PACKET.md`, owns AB7700 current74 planning.
   Keep its primary Model/acquired/frame distinct from internal188. Existing
   AA8320 and AA9730 hardcode `GeneratedModelNodeLifetime` for node slots
   (`widget_lifetime.cpp:41-49,123-130,149-163`): substituting only a widget
   callback cannot close raw4C cleanup. A genuine narrowly typed raw variant of
   those shared algorithms, with explicit primary/internal record routing and
   real current Model type/scene/tree methods, is a further prerequisite. Do
   not preclear4C/188, cast to the legacy lifetime, or add an extra release.
2. **Raw Font borrower admission.** `GameNativeFontApplication::native_fonts()`
   latches export, while retire_fonts accepts borrower proof only from the
   existing GuiTextRuntimeFactory (`game_native_font_application.cpp:221-243`).
   Raw Text needs an explicit same-collection, retained lifetime/quiescence
   binding, separately planned by the gameplay worker. Healthy bound owner
   `requires_process_retention()==false` is NOT terminal borrower proof. Require
   independent retired state plus settled Text/ALL Font-using Model/content/
   glyph operations, then post-parent-drain unlink before owner reset. Do not
   require108 null or read its bits after Text destruction. An unrelated empty
   legacy factory is not evidence; no Font retain or second registry is implied.
3. **Raw mutations/content.** Existing field and shader entries accept only
   `NativeGuiTextReference`, not RawModelReference (field HPP88-112; shader
   HPP96); cursor context also requires that legacy reference and Model hooks.
   Reuse genuine bodies via typed raw overloads only after common providers and
   retirement admission close. AB80C0 additionally needs authentic recursive
   child scalar/parent/list/scene transport and stable vector slots. Preserve
   real GuiFont/GuiDefault/cache/Lua/config readiness; no early callback shortcut.
4. **P4 joint activation.** Main geometry, property/content/font/shader/glyph,
   widget parent/scene/child ownership, Page provider and complete teardown must
   meet together on actual FE_initial. Member-only source preparation and the
   accepted empty fixture provide no independent-ready or FE_initial credit.

Next source dispatch should be the owned-members admission packet after root
reviews this API; main geometry and raw primary/widget routing remain separately
leased work. Build/existing checks and one later real-provider lifecycle gate
are future evidence, not results of this document-only audit.
