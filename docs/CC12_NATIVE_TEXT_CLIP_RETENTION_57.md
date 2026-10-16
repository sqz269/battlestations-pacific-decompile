# CC12 native Text clipping and retention 57

Review baseline: `f9dd0d899d2fd6082adfee779f5a843420574847`. This is a Source
trace and ownership proposal; no Source, Ghidra or runtime change is made.

## The native clip provider already exists

`GameNativeTextApplication::Impl` constructs
`GuiTextClipRefreshServices{buffers, parameters, style.aspect_ratio_00e12fc0}`.
Both nonempty native content and `begin_gui_text_clip_refresh_00ab7a40` call
`register_native_gui_clip_parameters_00aa9f10` directly. That function resolves
the canonical material and widget, walks `GuiLayoutWidget::parent` from self to
the nearest type16, and borrows these exact Source addresses:

- `GuiWidgetOwner::extra_fields().clip_enabled_e8` for `cClip`;
- the existing `GuiClipBoxTypeImplementation::fields().center_ec` and
  `border_f4`, resolved by `gui_clip_box_parameter_sources`;
- the same live aspect-ratio cell supplied to the Text application.

There is no missing rectangle/view adapter in this path. The derived ClipBox
fields are the canonical typed Source projection, not a recovered raw Original
ClipBox layout or an invented rectangle. `NativeMaterialEnvironment` is not a
current Source identifier; the relevant native contracts are
`NativeMaterialDestructionAccess` and `NativeMaterialParameterAccess`.

The `retain_widget` shared-pointer callback belongs to the separate
`GuiMaterialBindingServices`/`MaterialCloneState` path in
`bind_gui_material_callbacks`. A complete Source occurrence check finds its
invocation only there. Current native Text uses the style aggregate's domain,
aspect and raw material operations; it does not invoke that callback or its
semantic clip resolver. Those unused callbacks can remain explicitly unprovided
for this native profile. Installing a no-op token would prove nothing and is
unnecessary. A future call to the semantic binder must still supply its genuine
owning token; its existing admission rejects absent callbacks.

## Load, update and consumer chronology

`build_widget_children_00aaa710` creates a unique child layout, invokes node/type
construction, then stores its parent/list membership and runs constructed74.
It binds base properties, recurses into children, then runs the parent's derived
properties and loaded78. Consequently a child Text can register addresses in
its parent ClipBox before that parent's `ACE650` derived reader establishes
BorderWidth and calls `ACE120`. ClipBox construction intentionally leaves those
fields unwritten. Registration stores pointers without reading source words;
adding dummy initial values or an early "initialized rectangle" check would
misrepresent this valid order. Consumption must wait for the completed load.

`GuiClipBoxTypeImplementation::properties_bound` and
`update24_00ace120` update those SAME arrays in place. The existing
`dispatch_gui_layout_finished24` routes ClipBox to that method. Its switch
currently rejects Text, so this review does not establish a complete normal
Text page's current24 composition.

Native parameter records contain borrowed `source_08` pointers, not widget or
page tokens. `B44D60`/`B17E10` add no source retain. `AB7A40` registers main,
then reloads/registers the live shadow, then traverses child70 through borrowed
continuations. Parent/child/runtime identities and child-list membership must
survive a pending continuation; no snapshot or ownership transfer occurs.

When no ClipBox exists, `AA9F10` changes `cClip` but deliberately leaves previous
center/border/aspect records intact. `native_material_constant_build.cpp` reads
each selected vertex/pixel parameter source without a `cClip` lifetime test.
Therefore setting clipping to zero or reparenting does not authorize deleting
an old source ancestor. It must survive its last actual consumer or replacement
of every record still pointing into it.

## Real Text count, borrowed layout lifetime

Cursor-copy construction already calls the SAME factory's
`bind_retained_text_00b18a40`. Its `NativeGuiTextIdentityReference` borrows the
actual Text prefix+04 atomic registered in the canonical actual-owner domain.
`B18A40` really retains that raw Text identity; final release can dispatch the
existing same-owner deleting transport. This is real native-count retention,
not a shared-pointer substitute.

It does not own the C++ page/layout or any ClipBox ancestor. The companion is
explicitly non-owning. `GuiPageRegistry` owns unique pages, pages own a unique
root, layouts own unique children, and `GuiWidgetOwnerRuntime` separately owns
unique companions. The page's integer `reference_count` has an increment but no
release/ownership transport. Layouts and their parent/child fields are public;
holding a page alone would not prevent explicit child erasure or `retire_tree`.

The precise retirement gap is in `GuiWidgetOwnerRuntime::retire_tree`: its
whole-tree preflight checks active owned operations and derived preflight hooks,
but Text has no host-tree override checking retained material/source lifetime.
It releases scene nodes, then `erase_tree` destroys companions. Text
`implementation_destroyed`/`return_completed_allocation` can return the raw slot
and unbind its identity without gating on its count or tracking outstanding
parameter consumers. A caller that closes layouts while such external uses
remain can invalidate borrowed fields or retained raw identities. This is an
unsatisfied caller lifetime contract, not an observed failure of today's
inactive native Text path.

A blanket `count > 1` entry rejection is not an honest replacement. Cursor
materials can contribute an internal retain that normal owned-node cleanup
releases. Explicit Original-style `AB8EE0` scalar deletion intentionally does
not decrement/test the count. Changing that stub would change its established
contract rather than supply page ownership.

The failure side also needs a real owner. `load_gui_page_00aa5840` keeps its root
and script table as locals until late publication; child construction starts
before transfer into the parent's unique list. `LoadScope` was constructed
before those locals, so their destructors can run before its no-argument
`on_page_load_failed()`. Pending Text construction may reject that teardown and
terminate; the hook cannot acquire the already-unwinding unique graph. Font
host53's export-gap guard preserves its parent graph but does not make these
loader locals into durable page owners.

## Minimal actual closure proposal

No clip-provider or semantic-retainer implementation is proposed. A separate
Source lifecycle packet should add a persistent page-load/retirement operation
to the concrete native GUI host, coordinated with `gui_layout_loader.hpp/.cpp`:

1. Own the existing unique page/root, evaluated table and pending child layout
   BEFORE entering their native factory effects. Preserve recovered list/parent
   publication order; move C++ ownership into the operation without inventing
   another native owner or count. Transfer completed pages to the registry only
   on successful completion. Retain entered failures without replay/rollback.
2. Keep that operation, canonical widget runtime, SAME Text factory and real
   clip ancestors alive through every parameter consumer and continuation.
   Guard explicit tree retirement, child removal and reparenting against these
   uses; an alias shared_ptr over externally unique storage cannot do this.
3. Stop submissions and establish a real last-consumer boundary. Release owned
   scene/material edges in the existing order and close external retained
   cursor-material uses before returning Text slots. Do not substitute the
   later shared singleton drain for this proof: frame-job
   `retire_after_manager_drain` only retires bindings after that drain.
4. Then retire layouts/Text storage, fonts using the SAME empty factory, and
   perform shared drain while retaining required registry/VFS/renderer metadata.

This crosses the loader, its concrete host and factory lifecycle; no small
disjoint non-Text patch currently supplies that ownership. The real Original
page/ancestor reference-release chain and full consumer/teardown scheduling are
not established here. There is no fabricated raw ClipBox count, rectangle,
manager, runtime completion or native drawing claim. No new tests or builds are
needed for this read-only review.
