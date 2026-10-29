# Raw main-section reservation and retirement plan

Source plan at root `5c8c14dc04`; main-section335+199 is compiled but UNCALLED.
No source/build/test/game/Ghidra change or caller/prefix-only fixture expansion.

## Existing compatibility and actual blockers

d403 B75170 publishes the real geometry credit.481 B750C0 captures CURRENT180,
decrements it and marks geometry_terminal_completed only AFTER a reached real
terminal returns. e769/2ee admit nonnull Model180: their empty hierarchy/retained/
point-light guards do not require empty geometry. No native body rewrite is needed.

The canonical Mesh terminal executes B74280(flags1)/B73E60; B73C10 releases its
CURRENT draw sections before array destruction. Canonical Section B86690(flags1)
executes B86420/B86390, releasing material, streams and layout before pool return.
Canonical Material B194B0(flags1) runs B192F0 before its pool return. Actual
GuiNativeGeometryOwners disposal unbinds/erases its entry AFTER terminal return.
See `native_mesh_owner.cpp:170,204`, `native_mesh_section.cpp:428,458,490`,
`native_material_owner.cpp:159,188`, and `gui_native_geometry.cpp:86,97,107`.

Real00533FA0 initializes layout50=null, vertex_stream_count=0 and the other
owned section fields.379 creates the mesh; main-section appends its genuine
section/material. Null layout/no streams are already valid destruction branches;
do not insert layout/stream acquisition to make retirement appear complete.

Current `native_gui_text_primary_prefix_lifetime.cpp` primary_shape REJECTS180.
The prefix owner has no379/main-section graph; raw122 known_prefix_borrowed
does not reserve it. A successful library call alone therefore grants no scalar
permission. Native initial93/122 DEFAULT admission must remain unchanged.

## Concrete retained graph and successful receipt

Use one NEW concrete `NativeGuiTextMainSectionLifetimeStorage` owning the379
argument/position arrays, transform/bounds scratch, contexts, acquisitions/Frames
and main-section context/acquisition/Frame. Construct it without native effects,
over the SAME existing primary/internal records/providers; no second Model record.
Install an immutable default-null typed pointer in the existing prefix context
BEFORE Text allocation/raw122 binding. Existing DEFAULT/prefix callers keep null.

Coordinate with the geometry-lifetime plan as one closed immutable operation
choice, not two independently selectable optional shortcuts. A pending/failed
main-section graph cannot fall back to geometry-only or empty-prefix admission.
The choice alone grants no authority; only the chosen graph's exact complete
affinities/provenance admit retirement. Initial raw122 reserves its fully
installed untouched graph before those effects.

Add a pure main-section Frame.matches(context,acquisition,phase) view, analogous
to3c217. Its current phase() alone cannot prove private affinity. Require BOTH
exact379 COMPLETE affinity and exact main-section COMPLETE affinity, !failure
and no escaped/entered/poisoned invocation. Sticky199 failure cannot resume.

First admission requires mesh_gate_read, original zero mesh58 and !creation_skipped;
real section started/registered, appended; material complete/registered and
material_published; BOTH creator release records entered/decremented/complete
with their SAME captured CE2220, result1 and no zero-terminal arm. Require real
name construct-return and normal name-return completion, consumed state-1;
parenting/root returned on the actual late zero15C path with complete real B6E680.
Returned name header/data are stale; owner_record/companion pointers become
diagnostic after THEIR terminal callbacks. Never follow either after that return.

Expected section identity is section.section.creator, material identity is
section.material.material. section_creator/material_creator were exchanged to
null BEFORE decrement; null is NOT absence of the publication/credit. Keep their
GuiNativeSectionAcquired/NativeMaterialFactoryAcquired and release records alive.
379 must have its genuine Model-held mesh credit and completed creator return.

At pre-scalar admission, while payloads are live, require CURRENT primary180 to
be that mesh, CURRENT mesh58==1 and its sole draw_sections54 entry equal the real
created section; section.material20 equals that created material. Require genuine
canonical Mesh/Section/Material bindings and exclusive actual counts1, backed by
the actual acquisition/append/retain/release history. Counts/profile bits alone
grant nothing. Read only initialized fields: layout50=null and stream counts0,
not native-unwritten unused stream slots. Reject replaced/extra/foreign entries,
extra credits, streams/layout/hierarchy/content/Font or changed flags; never fix them.
Skipped-creation has no new receipt for existing entries and is excluded here.

## raw122, adapter and owner work

Extend known_prefix_borrowed with this concrete storage BEFORE binding and for
late flags. Reserve the whole object, exact Frames/acquisitions/operand scratch,
contexts/providers and still-owned nested material.cache key/name/program/texture/
fallback metadata. Reuse the existing C++ cache-backing walker pattern. Never
follow captured native result/creator/name-data/owner_record identities, read
returned headers, or infer hidden derived extents from a base sizeof.
Initial binding accepts exact fresh metadata; scalar admission requires COMPLETE.

Keep the old null-option prefix shape EXACT. A separate typed branch authenticates
the actual93/122/private db55 receipt, BOTH operation affinities and the created-only
graph above before permitting nonnull180. Retain the genuine false descriptor
predicate BEFORE188 reads, exact scalar/destruction stage fences and current2ee
base route. No e769/2ee or raw118 native cleanup schedule needs replacement.

Owner metadata is prepared while renderer idle; all records preexist raw122.
An eventual separately reviewed entry must preserve the real later caller order
and genuine open cache/Lua. It is not a call from today's prefix-only fixture.
Noexcept guards inspect direct failed/entered/name/release/cache markers and pure
affinity only; no throwing selector, native pointer rediscovery or count sampling.
Text plus BOTH Model scalar/late-flags/pool-return/disposal proof remains mandatory.
Additionally require primary.lifetime.captured_geometry==expected mesh, completed
geometry decrement AND reached geometry_terminal_completed on its canonical Mesh.
With the exclusive, unchanged graph and literal flags1 terminal chain above, that
normal return encloses nested section/material cleanup/pool-return. There is no
public per-entry retired getter: never dereference their erased companions or use
registry absence alone as physical-return proof. Broader/replaced/shared entries
need their own real retained terminal/provenance evidence before admission.

Keep every invocation/cache/provider through credits, flags0, failure and full
renderer verification. Extra Model credit or any missing return retains the WHOLE
graph. Never destroy storage in Text companion disposal. Renderer verification
stays FIRST before effect/sampler closure; only then reset dependent metadata.

Bounded files: NEW lifetime HPP/CPP; pure matches in main-section HPP/CPP; typed
option/branch in primary-prefix lifetime HPP/CPP; reservation in reference.cpp;
optional concrete storage/settlement in raw-default owner HPP/CPP. Root owns CMake
and integration. Split leases across those owners; do not broaden constructor85,
native bodies, layout/streams/Font/content/hierarchy, AAA710/current74 dispatch
or flag forcing. Explicit379 still executes the actual AB7700 body and retains
the geometry plan's genuine late bounds-zero obligation. Actual linking, gates,
callback/failure/reuse and runtime/ABI proof remain outstanding.

Root and independent Model/owner review accept this source plan and the closed
operation-choice clarification. The terminal-chain proof depends on the exact
provenance-backed sole graph remaining unchanged through callbacks. No new
operation implementation, owner admission or runtime gate is accepted here.
