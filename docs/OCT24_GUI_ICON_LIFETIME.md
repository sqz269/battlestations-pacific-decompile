# Raw Icon lifetime provider boundary

This follow-up audits the authoritative allocation/identity/terminal provider
required by native geometry adapter `460d884db`. No provider or incomplete raw
Icon image is implemented here. The first adapter commit remains independent.
There is no page/manager/runtime activation and no claim that the raw Icon
lifetime prerequisite is closed.

## Existing reusable pieces

* The actual Icon process pool is already reconstructed:
  `GameNativeGuiWidgetPoolProcess<GuiIconPoolTraits>` / F8BDA0. The allocator
  stub is AB2B60, allocation AB2930, return AB2150; payload is138h, slot13Ch,
  and its allocator slab index is the distinct DWORD at+138.
* AA1410's table entry chooses AB5C60 default construction and AB5D30 copy.
  The existing `gui_widget_factory` still requires a real
  `GuiWidgetConstructionHost`; that interface is not a bound application
  implementation.
* Raw AA9390 base construction is available as
  `construct_native_gui_widget_base_00aa9390`. It supplies the real+04 count,
  base fields and a native child-list sentinel. Its constructor-unwritten
  fields remain preimages; it does not create a canonical C++ widget owner.
* AB5C60's derived stores are retained in `cc13_front7.cpp`. They publish
  D5C4C0, current signed16 stateEC=-1, empty F4/F8/FC state-vector pointers,
  HasTexture100=1, delayed101=0, empty ShaderName104/108, mode110=0,
  ratio114=one, identity crop118..124 and zero timer/filter fields. F0 is
  deliberately unwritten. That implementation still requires a bound raw
  AA9390 host and uses its existing literal-one qualification.
* Raw AA9730/AA8320 normal schedules exist in
  `native_gui_widget_lifetime.cpp`. They require real current raw child/type/
  timed-entry/list dispatch through `NativeGuiWidgetLifetimeBindings`.

None of these pieces licenses a second page, widget state vector, native count,
layout hierarchy or model owner.

## Reached destructor order

The AB5E70 listing establishes the following unconditional schedule:

| Native site | Effect |
|---|---|
| AB5E8D | Publish Icon profile D5C4C0. |
| AB5E9C | Acquire the actual F8BD98 singleton through AB5700. |
| AB5EA3 | AB50E0 cancels entries matching this actual raw Icon pointer. |
| AB5EA8..5ECB | Read actual shader buffer108; return it through 419CC0/BD1510 using length104+1 when nonnull. |
| AB5EDB | AB5850 destroys the vector at raw+F0: beginF4/endF8,40h records, AB3280 record destruction, then frees backing and clears F4/F8/FC. |
| AB5EEA | Enter raw base AA9730. |
| AB6108..6115 | AB6100 tests flags bit0 only after the destructor; flags1 returns the same slot through AB2150/F8BDA0. |

AB6100 is ECX=this, stacked flags DWORD (low byte tested), RET4 at AB611D.
AB5E70 is ECX=this, no stack arguments, plain RET at AB5EFE. Neither scalar
wrapper nor base destruction decrements/tests widget+04. A direct manager or
parent deleting04(1) therefore runs independently of the count.

At canonical reference zero, BD30E0 dispatches the object's current deleting04
with flags1. It also does not add another decrement. An Icon terminal must run
the same derived/base schedule before returning the actual pool slot; deferring
the callback or substituting a host tree delete would change that behavior.

## Concrete missing contracts

1. **Actual delayed registry and drain dispatch.** AB5700 lazily allocates a
   real10h D5C4B8 object, zeros its raw array/count/capacity at4/8/C, publishes
   F8BD98, then registers it through the SAME process manager's BD0C30 while
   holding its captured section. AB5560 resizes to0, frees raw array4, clears
   F8BD98 and stamps CE3818; AB57C0 applies flags1 free. This is reached even
   for ordinary non-delayed Icons. Existing `GuiIconDelayedLoads` and
   `gui_icon_delayed_loads_00ab5700` use semantic std::vector/std::string data;
   no actual10h producer/publication or D5C4B8 arm in
   `NativeSingletonDeletionBindings` is currently supplied. The actual manager
   and captured-section services exist, so a bounded initially-empty service
   is feasible, but it still needs that real publication and drain binding.
   It is distinct from the parked cc15 GUI manager chain.

2. **One authoritative derived state.** `GuiIconRuntime` currently owns the
   canonical derived state and raw texture references in C++ containers.
   Native adapter460 does not publish those states or ShaderName into raw
   F4/F8/FC and104/108. Initializing an empty second raw object and calling its
   destructor would bypass the live state. Mirroring the live vector would
   create another owning representation/reference sequence. A provider needs
   either explicit raw-state adoption by that SAME runtime or a verified
   semantic lifetime composition over its SAME state, including delayed
   cancellation, name/state release order and exactly one texture release per
   owned state. A raw eight-byte prefix by itself cannot stand in for this
   reached full-image destructor.

3. **Same-tree scalar transport.** `GuiTextChildDeletion::delete_widget_virtual4`
   currently admits Group/Text/Section and explicitly rejects Icon. Its base
   phase and allocation-transport operations are private. Icon needs the same
   authoritative child unique_ptr, parent detachment, timed entries and base
   teardown, followed by its matching real pool return. `retire_tree` is not a
   replacement: it begins with the manager-style scene release before derived
   teardown, while a direct zero-count Icon terminal enters derived teardown
   first. Extending this shared transport requires coordination with the other
   native widget factory work, including Line.

4. **Admitted retirement phases.** Adapter460 intentionally admits only
   node-null/actual-count1 retirement after queued material borrowers finish.
   Its destructor callback also requires live D5C4C0. A genuine zero-count
   terminal starts at count0 and may still need AA9730 to release its node;
   base destruction then changes the raw profile. It cannot be forced through
   that count1 guard. A complete provider must introduce explicit derived,
   base-node, guard-release, base-finish and raw-return phases without a
   protective retain, reference resurrection or delayed terminal callback.
   The future sole-page authority can protect fields during those phases but
   does not itself supply the native scalar destructor.

The first missing singleton service is bounded, but it alone would not close
the authoritative Icon factory/terminal provider. These remaining state and
same-tree scalar contracts are why no prefix-only allocation adapter was added.

## Verification boundary

This audit used current source plus original exports/listings for AB5C60,
AB5E70, AB6100, AB5850, AB5700, AB50E0, AB5560 and AB57C0. Live Ghidra remained
on `/battlestationspacific.exe` in the configured `C:/Users/sqz269/bsp.gpr`
domain. No Ghidra mutation, new body reconstruction, C++ change, test or game run
was performed. The native geometry build proof in `OCT24_GUI_ICON.md` is
unchanged; it is not proof of this missing lifecycle provider.
