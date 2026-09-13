# Native Model clone fragments for Text

Addresses: `00B752B0`, `00B6F150`; analyzed prerequisites `00B742A0`,
`00B73F50`, `00B7BE40`. Names are reconstruction hypotheses.

This packet implements concrete base-copy and post-geometry fragments over the
existing `NativeModelOwner`, its actual pool storage, retained references and
scene bindings. It does **not** implement the complete Model factory/clone or
return an empty model as a clone. The remaining allocation transport and mesh
copy prerequisites are explicit below.

| Native routine | ABI and final instruction | Coverage |
|---|---|---|
| `B752B0` Model current10 | ECX source; stack flags,parent; EAX allocated clone; `B753EF RET8`, length3, endB753F1 | Partial: implemented B75331..B753F1 post-geometry operations; allocation/base dispatch/current geometry10 B752B0..B75330 remain external |
| `B6F150` node base copy | ECX source; stack destination,flags,parent; `B6F309 RET0C`, length3, endB6F30B | Partial: no-positive-point-light branch for Text flags26h,parent0; positive-light loop and non-Text flags/parent paths remain external |
| `B742A0` mesh current10 | ECX source; stack flags,ignored second argument; `B742F7 RET8`, length3, endB742F9 | Analyzed branch/ownership prerequisite only |
| `B73F50` mesh copy | ECX source; stack destination,flags; `B7426E RET8`, length3, endB74270 | Analyzed outer flow and Text flags26 route only; no reconstruction of its remaining copy callees |
| `B7BE40` point-light reverse append | ECX actual light; stack raw node; `B7BE80 RET4`, length3, endB7BE82 | Analyzed actual-array prerequisite only |

## Native Model route

At Text writer `AB9D87`, the current template's model receives virtual10 with
flags26h and parent0. Model profile `D62DE8+10` contains `B752B0`. It allocates
through the **same** model pool01090054 at `B752D1`, obtains the current source
name header via `B6D800` at `B752EA`, and constructs the new Model at `B752F2`.
The name is borrowed source+54, not a copied host string captured before
allocation. Construction unwind `CC1C70` returns the saved raw allocation via
`B748C0`; there is no clone-wide rollback after `B7530A` disarms that region.

`B75312` calls `B6F150` with source in ECX and destination, flags, parent on the
stack. Ghidra's apparent two-argument function and `unaff_retaddr` flags are
incorrect. Assembly loads destination at entry `ESP+10` after three pushes,
flags at `ESP+18` after all four pushes, and parent at `ESP+1C`; `RET0C` proves
three stack arguments. All five direct callers pass this contract, including
the additional saved EBP in the `B912E0` caller, which is not a fourth argument.

After base copy, the routine reloads source+180. A nonnull geometry receives
its **current** virtual10 at `B7532F`, with the original flags and zero second
argument. Only after that callback returns does native read source+17C then
source+178 using x87, call `B75170(0,acquired_geometry,178,17C)` at `B7534E`,
and release the clone's creator reference. The new association fragment performs
these exact operations on an already acquired actual `NativeMeshStorage`,
validated against its canonical `NativeMeshReference` in the same owner domain.
It does not produce that geometry or snapshot source values before cloning.

The final fragment enters at `B75365`, after the whole geometry branch or the
native null-source-geometry test. It captures incoming source+174 and old
destination+174, publishes incoming, retains it, and releases the captured old
owner. Only then does it execute ten separate source+08..2C x87 FLD/FSTP pairs.
The old owner's terminal callback may change those source fields. A byte copy
or pre-callback snapshot would lose that behavior and x87 NaN/exception behavior.

## Concrete base-copy branch

`copy_native_gui_text_model_base_00b6f150_fragment` receives two distinct live
canonical Models in the same `NativeModelEnvironment`. The destination must
already be the actual newly constructed clone destination. It verifies the
existing scene associations; it allocates or retains neither model.

The native signed source+168 comparison controls the point-light loop. A
positive count returns `point_light_owners_required` before any base-copy
effect. Zero and native signed-negative counts follow the native skip branch;
no finite/count clamp or invented successful light copy is added.

On that branch it copies source+AC/+4C using individual x87 loads/stores and
invokes actual destination current50(source.scene170,true). It then assigns
retained+130 with publish/retain/release ordering. Every nonnull retained130
must have the existing `NativeNodeDestructionRuntime` actual retained-owner
binding; no second count or fallback owner is created.

Flags26h include20h, so source child cloning is skipped. The fragment uses the
existing complete null-parent path of `B6E680`, reloads source+A4 for real root
propagation `B6D890`, then dispatches current38 (`D62DE8+38 = B6DB10`) using the
live source+B0 matrix. It requires the actual attached-object notification
binding when that matrix operation needs current3C. Existing scene/root and
attachment callbacks remain observable; they may mutate the source or clone.

After matrix callbacks it reloads source+130 and destination+130 and repeats
the retained assignment. This second assignment is not redundant. It then
uses the originally supplied null parent to clear destination+A0, clears only
the matching C++ notification dispatch metadata, copies source+138, copies
four sphere floats+13C..148 through x87, and finally reloads/stores source+48.
There is no added backlink removal, bounds calculation, child clone or
geometry operation in this base fragment.

Unsupported current profiles or missing actual owner bindings throw at their
required phase. Prior native effects remain in place. These exceptions are
diagnostic boundaries, not native exception equivalence or resumable frames;
restarting the helper would repeat already completed callbacks. The only
explicit returned boundary, positive source light count, occurs before effects.
Both models and every required borrowed service must survive all callbacks.

## Remaining geometry and ownership prerequisites

The constructed GUI mesh has profile `D62D60`; its current10 at `D62D70` is
`B742A0`. Flag2 **is set** in26h, so that call allocates a new mesh from0108FFF8,
constructs it, and calls `B73F50`. The alternative branch, flag2 clear, merely
retains and returns the same mesh. It cannot substitute for this Text route.
Both nonnull and null allocation paths call the mesh-copy body; inventing an
empty successful clone on allocation failure would change the native path.

`B73F50` copies its LOD state and then uses flags8 and10h to choose stream clone
versus retained sharing. Both are clear for26h: actual index and vertex streams
remain shared. Draw sections are different: every current source section gets
a new copied section via `B85EF0`, and flags&6 causes a new material clone via
`B18B60`. The copied section is appended and retained in the new mesh, then its
creator reference is released. Counts and source slots are read live through
the loop. Optional flags/payloads and the weight-name vector follow afterward.

Existing `clone_native_material_00b18b60` already reconstructs actual110h
material storage. Its algorithm is **not** an absent dependency. What is still
missing is mesh-copy composition, actual section-copy construction `B85EF0`,
and creation/registration/creator transport of those copied sections and
materials in `GuiNativeGeometryOwners`. The semantic `MaterialCloneState` and
`GeneratedInstanceGeometry` cannot be used as their raw identities. LOD and
weight-name copy leaves also need concrete composition; flags8/10h stream-copy
branches are outside this packet's Text route.

The positive-light path first appends the actual source light to the
destination+164 array, then `B7BE40` appends the destination's raw node address
to the actual light+1E0/+1E4/+1E8 array. The existing
`GeneratedModelPointLightLinks::models` vector is a semantic backlink projection;
it is not that physical array and cannot complete the producer. No such
point-light owner or duplicate array is created here.

The Model pool, `NativeModelOwner` constructor and `NativeModelReference` are
available, but `GuiWidgetOwnerRuntime::model_reference()` resolves its own
canonical `models_` records. Its creation/adoption method is private and only
creates fresh named models. A future full clone must transport the one newly
allocated owner/reference into that same registry before widget binding. A
separate raw-slot cast, independent model map or fresh empty-model fallback
would not satisfy that lifetime contract. This packet does not modify those
leased owner modules or supply a forwarding-only factory.

## Verification

Read-only Ghidra batches verified the existing project and program. The full
`B752B0`/`B6F150` assembly was inspected, including ESI/EDI/EBX provenance,
current-profile data and all five base-copy callers. The complete mesh-clone
outer body and actual section/material copy contracts were inspected without
reconstructing or annotating their separately owned callees. Native names and
call rows are recorded in `reports/native_gui_text_model_clone.json`.

Strict MSVC Win32 `/std:c++17 /W4 /WX /O2 /MD /fp:strict` compilation passed.
The strong parent call verifier passed38 numeric rows with zero failures:
34 direct calls and four resolved indirect calls reported separately.
No test framework, new test or mock callback probe was added. Full canonical
clone creation is not reachable in the rebuilt executable, so no runtime,
native-byte differential or gameplay/visual claim is made. The parent handles
combined build and integration. These are new C++ interfaces, not native ABI
entry replacements.

## 2026-09-12 timed and clip ownership integration

Positive point-light lists no longer return the previous owner-required boundary
when supplied with actual live light bindings and proven backing. The source
array holds raw light identities; each light's actual +1E0 descriptor holds raw
node identities. Clone copying uses the same physical lists and preserves live
count/reload and captured-source ordering. Five actual node destruction adapters
remove backlinks through this runtime. Full PointLight construction/type/pool
ownership and other Model clone branches remain external. See
NATIVE_POINT_LIGHT_LINKS.md and reports/orch5_timed_clip_batch.json.

## Correction from docs/NATIVE_STREAM_CLONE_3E.md

The later stream packet implements actual B729A0/B72A70 factory/map/copy/unmap and extends the supported mesh/Model composition from flags26 to exact flags26/3E,parent0. `GuiWidgetModelCopyRuntime` connects AA9520 to that composition. The earlier flags3E boundary in this document describes its original scope. Current implementation and validation are in `docs/NATIVE_STREAM_CLONE_3E.md`, `docs/GUI_WIDGET_MODEL_COPY.md` and `reports/orch5_stream_text_copy_batch.json`. Full retained raw Text identity, native SEH, other flags/parent and gameplay remain separate.
