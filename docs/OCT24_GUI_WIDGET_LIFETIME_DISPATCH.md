# Closed raw GUI Group / ClipBox lifetime transport

This packet supplies `NativeGuiWidgetLifetimeDispatch`, a concrete
`NativeGuiWidgetLifetimeBindings` for an explicitly closed hierarchy of raw
GUI Group and ClipBox allocations. These are cGuiElement-derived widgets,
not the renderer cGroup used by the accepted mission Text root.

It supplies no runtime activation, retained page/type factory, GUI manager,
logical `GuiWidgetOwner` conversion, second reference count or owning map.
Required native type-phase class queries and actual timed-entry custody remain
mandatory services. This is not a claim of complete GUI lifetime closure.

## Native evidence and allocation authority

The saved `bsp.gpr` program `/battlestationspacific.exe` and existing exports
were checked before the native-table/read batches. Actual table bytes establish:

| Actual profile | current0 | current4 | current0C | current20 |
| --- | --- | --- | --- | --- |
| D5CB80, raw GUI Group | BD30E0 | AC73E0 | AC6F70 | AA8320 |
| D5D058, raw ClipBox | BD30E0 | ACE620 | ACE0C0 | AA8320 |
| D5C130, base during AA9730 | BD30E0 | AA9A90 | A9E070 | AA8320 |

The existing AA12F0/AC6F50/AC6FA0 factories construct Group over the SAME
F8BFA0 pool, EC-byte payload / F0-byte slot. Existing AA1920/ACE0A0/ACE0F0
construct ClipBox over the SAME initialized F8C13C process pool, 10C-byte
payload / 110-byte slot. Both default and copy routes use the existing raw
AA9390 / AA9520 bodies, respectively. This packet does not construct or copy
either payload and does not initialize either pool.

`NativeGuiGroupReference` already borrows actual+04 and admits the completed
allocation in `NativeRenderActualOwnerRegistry`. Only opaque identity and
lifetime-context accessors were added. New `NativeGuiClipBoxReference` follows
the same model and additionally requires the SAME initialized ClipBox process
pool reference used by AA1920/ACE620. Neither a matching profile/count nor the
pool-reference check proves slot provenance, constructor completion or custody:
the factory caller must establish those before admission. An unfinished or
returned factory slot must never be admitted. Failed copies can retain
independent Model acquisitions in their existing caller frame; that does not
authorize accessing the failed widget slot.

## Dispatch and native destruction order

The dispatch object owns only its explicit `NativeGuiWidgetLifetimeContext`.
Group/ClipBox identities must use that SAME context and the supplied canonical
registry. Lookup validates the existing companion class, opaque identity and
context; it adds no native retain/store. It does not discover or own a tree.
Every reachable parent/child must already belong to this closed raw domain.
The caller also retains all actual Model companions, resources, services,
container backing and required callbacks through nested calls and failure.

Known child20 is the genuine raw AA8320 body. Child04 accepts only the native
AA9730 call's flags1 and its captured AC73E0 / ACE620 target, then invokes the
SAME canonical companion's selected scalar body. It performs no reference
count test/decrement, preliminary detach or post-delete payload inspection.

AA9730 remains authoritative: publish the base profile, recursively release
scene nodes through current child20, delete the CURRENT first child through
current4(1), detach from the raw parent, handle any replacement main node,
delete actual timed entries through current0(1), then end the containers/base.
The real child destructor removes itself from its parent's SAME +64 list.
The parent reloads head/count; no fallback erase manufactures progress.

`remove_all_00a9bd50` delegates to the existing full
`remove_native_gui_widget_list_value_00a9bd50`: capture the supplied pointer
cell and initial sentinel, remove every matching node+08, update links/current
count and free each real 0C-byte node in its original allocation domain.
The cast is the already-established physical raw-list alias, not a logical
widget/layout cast. Existing AA83A0 remains the producer's genuine explicit
detach helper. Calling it before an AA9730 child-delete site would change the
native schedule, so this adapter does not add that call.

Zero-reference terminal delivery does not decrement again: validate CURRENT0
BD30E0, run that real provider so it freshly selects CURRENT4, and execute the
same selected scalar body with flags1. Explicit scalar deletion can occur at
a nonzero count. ClipBox uses genuine ACE620 -> AA9730 -> current flags low
byte -> SAME process-pool return. Metadata retirement/unbind occurs after the
body and reads no returned payload/count. Flags0 ends payload lifetime while
leaving physical slot custody with the caller, as in the raw deleter.

## Required native-phase providers

`NativeGuiWidgetLifetimeRequiredCalls` is nonoptional and abstract:

- Class-query calls must execute genuine captured Group AC6F70 / ClipBox
  ACE0C0 with the untouched descriptor and CURRENT initialized native lineage.
- Timed-table lookup must recognize only actual caller-retained timed
  allocations and return their stable genuine current table.
- Timed current0(1) must use that SAME allocation, matching allocator and
  existing custody. AA9730 itself clears the captured vector slot afterward.

The ACE0C0 export reads F8C174, F8C178 and F8C17C in order, stopping before
F8C180. ACE080 reads CURRENT[F8C174], while ACE090 reads CURRENT[F8C180],
the name word at lineage+0C. Existing `GuiTypeDescriptor<3>` explicitly places
its name word after IDs[3]; it is not a fourth tag. Group AC6F70 similarly
compares CURRENT F8BFD8[3]. No numeric widget-type enum, executable loader-zero
preimage, copied descriptor or name word substitutes for those IDs.

The current production `GameNativeGuiTextProcess` exposes only completed
Element/Text/Layer type lineages, not a completed Group/ClipBox type-phase
producer. The required class-query provider must therefore retain the genuine
missing native-phase authority. The existing generic volatile-lineage helpers
can implement the comparison once that authority exists; this packet does
not fabricate it. The semantic `GuiTimedEntryStorage` contains a logical
layout pointer and is also not an actual timed-allocation provider. A timed
entry's remaining-time +04 must never become an atomic reference count.

Unknown canonical owners/widgets/targets reject. Only unregistered non-widget
table requests can reach the required timed provider, which must reject any
allocation it does not genuinely recognize. No successful default table,
false class answer, no-op timed delete, placeholder allocator or lazy metadata
construction is supplied.

## Failure and validation boundaries

Companions, contexts and flags must be disjoint from the actual slot and other
mutable/consumed backing, address-stable, and externally serialized against
reentry and pool reuse. Every explicit or zero-terminal deletion after
admission must pass through the same canonical companion. An entered scalar
failure retires its identity and propagates; independently retain the partial
raw graph and its providers. AA9730 unwind may have consumed list/vector
backing without finishing child/timed payloads. Never replay or synthesize
rollback from links/counts or from a retired companion. A terminal escape
still terminates at its existing noexcept boundary.

These are new Source interfaces over existing bodies. Native private FH3/SEH,
upper-EAX/register ABI, malformed/racing graphs, all18 types, active pages and
game validation remain unproved. No new tests or game runs are part of this
packet. `scripts/build.ps1` completed for MSVC Win32 Release: both new CPPs
compiled, `bsp_game` linked, and the two existing CTest checks passed 2/2.
They do not exercise this new, deliberately unwired raw dispatch domain.

The private `bsp_core` registration of `src/native_gui_clip_box_identity.cpp`
and `src/native_gui_widget_lifetime_dispatch.cpp` was removed by restoring
the exact original CMakeLists bytes, SHA256
`831ED8EDA85792A511610BA430B56A783001EB86BAB52675F5A6B4699297A29A`.
Permanent build registration remains the primary integrator's responsibility.
