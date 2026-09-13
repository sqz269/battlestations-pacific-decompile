# Text factory identity integration

Addresses: `00AA1380`, `00AA9390`, `00AA9520`, `00AB9650`, `00ABB2C0`,
`00AB8250`, `00AB8EE0`, `00AA9730`. Names remain hypotheses.

The existing Text factory allocation record now owns the sole native identity
companion for its same raw pool slot. The eight-byte prefix and its actual
atomic `+04` come from the proved producers in `NATIVE_GUI_TEXT_IDENTITY.md`.
The widget runtime continues owning the sole semantic Text body/lifetime.
No Text body fields beyond `+04`, native container headers or original ABI are
constructed in raw storage. A material property reader must call
`factory.canonical_owner(raw)` to reach represented fields.

`GuiNativeGeometryOwners::registration()` supplies the exact existing
bind/find/unbind/context/actual-owner domain. The Text factory creates no other
native owner registry. Its allocation map holds the companion, raw transport
and construction-failure diagnostics. Binding does not retain, and both the
companion and the material use the exact raw `+04` atomic.

The factory itself implements `GuiTextCursorParameterOwnerServices`.
`bind_retained_text_00b18a40` verifies the same widget/allocation/registry,
then invokes the existing actual B18A40 helper. Copy admission requires this
concrete factory adapter, its same cursor buffers, layouts and constant storage.
Old retained owner release still precedes incoming publication/increment,
including equal identity. There is no protective count increment.

Default construction first reserves the host allocation record, then produces
the actual prefix. A retained empty implementation shell is published through
`begin_default_type_admission`. The factory binds its identity, publishes
D5C6C8, and admits the deferred default `GuiTextLifetime`. That same lifetime
exists before `complete_default_construction_00ab9650()` runs tracked AB8530.
Only successful completion transfers the shell to `construct_base`, which
clears its constructor borrow after receiving the exact same implementation.
The current default entry still receives an already initialized semantic base;
it does not reproduce allocation/prefix ordering against earlier AA9390 base
effects or a complete native constructor ABI. Copy has a stronger prefix
ordering: AA9520 prefix construction precedes `construct_base_copy_00aa9520`,
and D5C6C8 publication precedes the derived copied lifetime/string operations.

A default error preserves the allocation, registration, shell, lifetime,
exception and all acquired native resources. The runtime retains its same
owner and `before_destroy` callback while the constructor borrow remains.
`construction_failure(layout)` exposes the saved exception, not a retry or
successful constructor result. For `construct_unbound_glyph_child`, the factory
also retains the same locally owned layout wrapper on failure. External
caller-owned layouts/page frames still must remain alive across failure.
Host allocation failure before a shell exists can retain only the already
produced prefix/slot diagnostic; native constructor unwind is not reproduced.
Incomplete tracked section/glyph operations join the implementation's existing
pending guard, preventing scalar deletion, mutation and copying after failure.

Explicit child scalar deletion publishes D5C6C8 before AB8250 derived effects,
D5C130 before AA9730 base scene releases, then D5C104/CEB130 at the base end.
No direct scalar route decrements or gates the actual Text count. Thus a
cursor material can drop a direct-deleting Text from2 to1 during subtree
destruction; a residual creator1 does not prevent AB75A0 pool return. Count-zero
release instead uses the same registered companion's BD30E0/current04 route,
which calls the same `GuiTextChildDeletion` and canonical widget owner.

Ordinary `retire_tree/erase_tree` remains the existing bounded host tree route:
its implementation hook publishes Text before derived cleanup and base before
subsequent scene releases; implementation retirement finalizes the prefix after
those represented base effects. This does not upgrade that route to complete
native container/SEH equivalence. Explicit scalar flags0 ends the body and
retains storage/companion until `release_completed_storage`; body lookup rejects
completed storage. Flags1 returns the same slot first, then unbinds and retires
the companion. Registration remains present throughout native resource releases
and pool return, and unbind never reads returned pool storage.

The semantic `GuiWidgetBaseExtraFields::references_04` removal/binding is owned
by the primary integrator. This packet never uses it for material retention or
terminal deletion; it must not be treated as a second authoritative native count.

Validation is recorded in `reports/native_gui_text_factory_identity.json`.
The default constructor reaches actual material/effect construction; no fake
successful effect48/material was introduced to enable a positive fixture.
Build and call-site checks are distinct from canonical terminal execution,
whole raw Text ABI, native exception unwind, renderer or gameplay validation.
