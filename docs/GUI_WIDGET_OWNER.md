# Retained GUI widget and native scene composition

Packet `orch2_gui_widget_scene_k`, 2026-09-11. Analysis was read-only against
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; every live batch used
`tools/bsp.py ghidra`, which verifies the selected project and program.

`GuiWidgetOwnerRuntime` binds one companion to the existing `GuiLayoutWidget`.
Its `GuiWidgetTransform`, parent and child lists remain the existing page tree.
The scene flags belong to that same logical owner. A child node is a real
`NativeModelOwner` over the supplied canonical pool's 188h slot, with a
`NativeModelReference` borrowing the slot's actual +04 atomic. `node_id` is that
actual Win32 address, not a generated identifier. Model construction writes
184h bytes and preserves the pool's separate slab index at +184.

The widget itself is a reconstructed C++ interface, not a raw native 100h
object. Its layout/color/alpha projections predate this owner and are kept
coherent at the binder boundary; the native widget destructor, copy constructor,
reference-count-driven GUI destruction, and arbitrary native widget pointers
are not implemented by this packet. The model's native reference counting,
logical release, destruction and physical pool return are the existing concrete
implementations, composed here rather than duplicated.

Correction from `docs/GUI_NATIVE_SCENE.md`: `retire_tree` now follows the
confirmed `00AA31F0` disposal fragment: current20 logical node release before
the deleting-destructor route. The latter runs derived teardown before the
base `00AA9730` release pass. `00AA8320` itself never destroys derived state.
The real outer-scene/group/model fixture verifies this ordering without an
extra node reference. Native widget storage and full base destruction remain
outside this semantic companion, as stated above.

## Evidence and original ABI

All names are hypotheses unless already established library names. Final
addresses below identify the inclusive final instruction, not its last byte.

| Entry | Original ABI | Final instruction | Evidence and implemented boundary |
| --- | --- | --- | --- |
| 00AA9390 | ECX widget, stack type, EAX widget, RET4 | 00AA9511 | Base constructor fields over the same projected owner; +08, +85 and +E4 are not written |
| 00AA6560 | ECX type, EDX source/null; RET or tail constructor | 00AA65F3 | Actual per-type factory callback runs before node allocation/binding; new runtime supports Group2, Icon6 and FrameBox18 only |
| 00AA6640 | ECX type, EDX NativeString pointer, stack source/null, RET4 | 00AA66B5 | Type creation, canonical model slot/constructor, bind, virtual74; standalone helper |
| 00AA6720 | ECX widget, stack node, RET4 | 00AA6732 | Store +4C then clear node+138 mask3, without retain/release |
| 00AA7170 | ECX widget, RET via tail jump | 00AA717F | Dispatch current virtual60 with0, then bounds refresh |
| 00AA6A30 | ECX widget, stack byte, RET4 | 00AA6A3A | Base virtual60 writes +85; new interface accepts bool |
| 00A9AC00 | ECX widget, RET | 00A9AC00 | Base virtual74 is a single C3 byte; **no Ghidra function** exists there |
| 00B6DA70 | ECX node, stack float and recursion byte, RET8 | 00B6DAA9 | Stamp actual +AC and recurse through the actual node hierarchy; preserve child x87 argument spill |
| 00B64780 | ECX destination, EDX float pointer, RET | 00B6481E | Separate FSIN/FCOS and float32 spills, SSE negative-zero subtraction, full Z rotation matrix |
| 00B74640 | ECX model, stack unused DWORD, RET4 | 00B74646 | Read the actual model geometry pointer +180 |
| 00B732C0 | ECX geometry, stack element index, RET4 | 00B732CA | Read the actual geometry pointer array +54 and selected element |
| 00B855B0 | ECX element, stack float4 pointer, RET4 | 00B855CB | Sequential x87 load/store to actual element +24/+28/+2C/+30 |

The old scene-visibility note called the AA6640 name a C string. Assembly and
B75030's concrete NativeString signature show it is the native string pointer;
this runtime builds that string through the same supplied string pool.

The saved AA6560 pseudocode inlines the Listbox path and appears to contain SEH
work. Its own assembly body is a jump table ending at AA65F3. Source/null moves
from EDX to ECX before each constructor-stub tail jump. This packet does not
claim its unsupported copy or remaining class constructors.

The base vtable at 00D5C130 has +74=00A9AC00, +78=00AA7170,
+60=00AA6A30, +38=00A9E0D0 and +3C=00A9E100. Raw bytes prove the two empty
base hooks: C3 at 00A9AC00 and C2 04 00 at 00A9E100. They are not placeholder
success implementations. Every concrete type supplies all six dispatch
callbacks through `GuiWidgetTypeImplementation`; the factory must reject any
unimplemented class. A new glyph-owner class also needs its real secondary-node
release before this runtime can support it.

## Composition and lifetime

`GuiLayoutHost` now has widget-reference overloads for child/root creation.
Existing string overloads remain compatibility boundaries for diagnostic hosts.
The real host calls `construct_child_00aa6560` there: this performs the derived
constructor, creates and binds the model, but does not run virtual74. The loader
then links its existing widget lists, parents the actual nodes and dispatches
`constructed74`. The distinct standalone `create_with_scene_00aa6640` calls74
immediately after binding, as that native function does.

The loader separates the base-property completion callback from the derived
reader continuation: `on_widget_base_properties_bound` runs before child
traversal, then `on_widget_properties_bound` runs after every child finishes,
followed by loaded78. The actual host wires these to `base_properties_bound`
and `properties_bound` respectively. This preserves the Icon reader's observed
base00AAA710 call before its own properties; the earlier diagnostic loader
issued the derived callback too soon.

AA6640 explicitly tests the model allocation result, binds null on failure and
still calls74. `construct_child_00aa6560` preserves that null binding rather than
manufacturing a node. Required parenting or a derived74 that dereferences the
node then encounters the explicit new host null-node error. A no-node base74
can return because its native body is the proven single RET.

For the page root, `construct_root` borrows the parent's already-constructed
`NativeNodeBinding`. The actual root can be a group in its distinct pool; this
runtime never substitutes a child model for it. That root must already have its
real lifetime binding in `models.nodes.attachments` so the existing B6DFA0
unlink-and-release path can dispatch its virtual18.

The model reference's terminal callback removes its one companion after native
destruction, pool return and lifetime unbinding. GUI tree release calls the
existing B6DFA0 implementation for each child and then the owner node, clearing
the widget pointer only after the call. The layout's `before_destroy` callback
does this while its fields and children still exist, including failures during
node creation, property parsing, descendant construction or page registration.
It prevents a later page-failure callback from resolving already-dead layouts.
Explicit `retire_tree` is also available. The runtime and native environment
must outlive layouts and any outstanding retained render references.

Visibility walks the existing GUI hierarchy to issue effective-visibility edge
callbacks before changing any factor. It preserves the original short-circuit
and possible double virtual38 dispatch, rather than using the older diagnostic
helper's cached read. Factor recursion follows the actual native scene-node
hierarchy, so non-widget descendants also require actual node bindings.
The authored Visible field is not overwritten by SetVisible itself.

Transform publication uses the existing local-transform kernel, this packet's
B64780 rotation, existing x87-order 00413920 matrix multiplication and existing
B6DB10 local-matrix invalidation. The actual node must have that virtual38
contract. The property binder still projects all base fields before this
composition callback; this is not a claim that the native per-property callback
trace has been reproduced. Existing kernel floating-point boundaries remain
those documented in GUI_WIDGET_TRANSFORM.md.

Bounds use the native +74 byte gate. AA70E0 pushes two zero words before
B74640: the first is that accessor's unused argument; the second remains for
B732C0's element index. There is no renderer/device singleton on this path.
The actual geometry header and element storage must exist. Missing geometry or
element zero throws at the corresponding new host boundary; a diagnostic
GeneratedInstanceGeometry address cannot replace either native header.

## Remaining environment and validation

The supplied `NativeModelEnvironment` owns the canonical model pool, shared
node destruction/lifetime/scene runtimes, string pool, type bootstrap, actual
retained-owner resolver, native constants and current model/node vtable views.
`GuiWidgetNativeCalls` supplies full B6E680 parenting and actual node resolution
for descendants outside the GUI tree. The parent integrator is implementing
that parenting against the same native node bindings. Per-type constructors,
derived properties, geometry/material resources and page-root allocation remain
their respective packets' implementations, required through explicit bindings.

Validation: all eight native seed comparisons matched; MSVC Win32 Release
build passed, followed by both existing CTests. One ignored, manifested
`local/gui_owner_probe.cpp` / `build/gui_owner_probe.exe` exercised real model
slot identity, preservation of the pool slab index, visibility +AC, active/base78,
actual local-matrix publication, layout-driven logical release, an extra retained
reference surviving layout destruction and final pool return, plus exact raw
element bounds placement with surrounding canaries. It passed. No permanent
test suite was added. The fixture uses explicit current table captures and
fixture type tokens; it does not run the game, attach a page scene, create Icon
or FrameBox resources, or prove visual rendering.

Ghidra was not mutated under this packet's read-only instruction. Ledger names
and evidence are prepared for the integrator's later locked annotation batch.
