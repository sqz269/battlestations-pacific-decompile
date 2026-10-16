# CC12 native GUI retirement chain readiness 70

Requested Root baseline `f025c2315d3e370a7730839e898cad438a4e3d70` was merged
cleanly with exit0 at worker `5cd52d190156c24a600886015b88da2ed59723c2`.
Packet67 remains an ancestor. This receipt is read-only: no Source, Ghidra,
loader, constructor, current24, activation or ownership policy is changed.

## Actual destruction and reference paths

All live batches used the BSP wrappers, verifying `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`. The relevant live table words are:

| Profile | Slot | Actual word | Target |
| --- | --- | --- | --- |
| Screen `00D5BE38` | +20 | `00D5BE58` | `00AA8320` |
| Screen | +04 | `00D5BE3C` | `00AA38F0` |
| ClipBox `00D5D058` | +20 | `00D5D078` | `00AA8320` |
| ClipBox | +04 | `00D5D05C` | `00ACE620` |
| base widget `00D5C130` | +04 | `00D5C134` | `00AA9A90` |

`00518250` demonstrates a real page-handle release policy, rather than a
general rule inferred from a reference count. For each selected outgoing
nonnull handle it reads the actual page+04. Exactly one selects manager getter
`004C12B0` and `00AA31F0`; every other value selects the current import
`[00CE2220]` on page+04, followed by the captured page's CURRENT virtual0 if
the decrement returns zero. The caller clears its handle after the callback.
This is a page-owner policy; other callers need not use the same decision.
For example, recovered `00517D90` directly calls `00AA31F0` for its owned
nonnull pages without that count test.

`00AA31F0` itself removes the first identical page pointer from manager+14's
vector, shifts following entries and reduces its end BEFORE current20 at
`00AA326A`. It reloads the page table and calls current04(1) at `00AA3276`.
It does not decrement/check page+04. The recovered raw Source schedule in
`native_gui_page_registry.cpp` already preserves those separate calls through
the required `NativeGuiPageLifetimeHost`; no default page provider exists.

Current20 `00AA8320` walks child scene operations before releasing the current
widget's drawable. Its Text class check additionally releases the existing
glyph/shadow slot at +188. Real scene/model/material teardown can therefore
release internal cursor material references before scalar widget deletion.
The material owner is precise: `00B18A40` retains its selected +0C identity
only under byte+10D; `00B192F0` releases that owner after effect/textures and
before parameter destruction. The admitted Text companion borrows the SAME
actual Text+04. None of these operations retains a ClipBox ancestor merely
because a parameter record points into its fields.

The scalar paths are direct destruction:

- Screen `00AA38F0`: ECX=page, one flags DWORD, `RET4`; call `00AC5480`, then
  read the flags LOW BYTE and return the SAME slot through `00AC4C40` iff bit0
  is set. EAX returns the original identity. It does not decrement page+04.
- ClipBox `00ACE620`: ECX=widget, flags DWORD, `RET4`; stamp `00D5D058`, call
  `00AA9730`, then iff flags bit0 call `00ACE4A0` with the SAME widget and
  pool `00F8C13C`. It does not inspect its count.
- Base current04 `00AA9A90`: base destructor `00AA9730`, then flags&1 pool
  return `00AA75F0`. Its direct-delete contract is already recorded/recovered.

Whole raw `00AA9730` now exists in `native_gui_widget_lifetime.cpp`. It stamps
the base profile and calls `00AA8320`; while current child count+6C is nonzero
it reloads the first child and invokes its current04(1). The child must detach
itself; neither Original nor Source fabricates progress. The child's own
destructor reads its parent+70, performs the reached current node detach/root
operations, calls `00A9BD50` on the parent's SAME +64 list with the child
identity cell, then clears its parent link. `00A9BD50` is also now a complete
raw reconstruction: remove ALL matching 0Ch list nodes, decrement current list
count, free nodes, and never release payloads. The base finishes scene/timed/
container/base cleanup; it neither decrements widget+04 nor returns the slot.
Do not propose either already-complete raw body again.

Thus actual pool return after the recursive destructor cascade closes widget
storage in this inspected path. There is no child+70 reference increment or
ancestor count barrier in this chain. Page-handle references can govern a
particular owner's decision to start deletion; they do not make a retained
Text count into ownership of its parent/page/ClipBox. A blanket count>1
rejection would also reject legitimate internal material retains before their
normal scene release. Nothing here proves the external renderer/material
consumer boundary that makes eventual ancestor deletion safe.

## First missing raw Screen retirement unit

The next raw boundary reached by Screen current04 is `00AC5480`, complete
range `[00AC5480,00AC5555)`: 213 bytes, 59 instructions, ECX=raw124h page,
no stack arguments, plain `RET`. Existing C++ is a typed successful-call
composition over `GuiLayerImage`/`GuiScreenLayerState` and
`GuiScreenSceneRuntime`. It supplies useful established scene/store behavior,
but is not a raw page destructor: `destroy_layer` clears semantic store/name
state, and the runtime owns a `std::string` rather than operating on raw
page+100/+104. Source lookup finds no complete raw `00AC5480` implementation.
`frees_block_00aa38f0` is only the flags predicate, not its complete wrapper.

The actual `00AC5480` schedule is important for a future raw implementation:

1. Stamp `00D5BE38`; read the LOW BYTE at page+F4; enter native EH state1.
2. Owned-store arm: call manager getter first, THEN read page+F0 and call
   `00AA4B30`; reload page+EC afterwards. If nonnull call `00B72250`, THEN clear
   page+EC. Shared arm: capture page+EC; if nonnull call CURRENT `[00CE2220]`
   on that scene+04, then load its CURRENT virtual0 on zero; clear page+EC
   only after the callback. This releases scene counts, not widget ancestors.
3. Capture page+104; set EH state0 before the name-return path. If nonnull,
   load page+100, add one with DWORD arithmetic, and keep the captured data,
   size and argument1 across `00419CC0` before `00BD1510`. No normal store
   clears page+F0, page+100 or page+104.
4. Set EH state-1 BEFORE `00AA9730`. Normal return restores the native frame.

Native handler `00CB8836` selects FuncInfo `00DEF7DC`, whose two-entry unwind
map at `00DEF7CC` is `(to=-1, action=00CB8820)`,
`(to=0, action=00CB8828)`. Funclet `00CB8828` tail-calls
`0041DD20(page+100)`; `00CB8820` tail-calls `00AA9730(page)`. Consequently a
state1 exception unwinds name then base, and state0 unwinds base. This is
observed FH3 data, not a claim that Source C++ exceptions/hardware faults are
already equivalent or that an interrupted destructor can be retried.

## One bounded proposal and its limits

Propose a new `include/bsp/native_gui_layer_lifetime.hpp` and
`src/native_gui_layer_lifetime.cpp` for whole raw `00AC5480` plus its 28-byte
`00AA38F0` wrapper, with unique future receipt files and address leases only
for those two established functions. Borrow the existing actual raw base
lifetime and string domains; reuse `return_native_gui_layer_slot_00ac4c40`
and its initialized existing layer-pool owner, with no duplicate pool/global.
Keep the wrapper's post-destructor flags read explicit and return only the
original numeric slot identity after a reached free.

Actual manager getter, camera-store removal, scene release/decrement and
current-zero dispatch must be explicit REQUIRED callee bindings, with no
dummy implementation or typed/raw cast. `00AA4B30` and `00B72250` currently
have typed ownership implementations, not complete arbitrary-raw providers.
Existing `NativeGuiSceneReference` can supply canonical actual scene identity
and its real count/terminal domain when genuinely admitted; it does not itself
prove those missing call compositions. Raw `00AA9730` still requires genuine
current child/timed providers; its `00A9BD50` binding can use the recovered raw
function. A whole parent over controlled services must be labelled as such,
without claiming the complete populated page subtree or current ABI is ready.

Audit all body bytes, widths, call/field reload timing and the observed unwind
states before implementation. Canonical Win32 checks and the smallest useful
raw parent/controlled-service comparison can establish the new bounded unit;
they cannot establish native FH3/hardware-exception, arbitrary alias/reentry,
gameplay or real consumer-retirement equivalence. This is independent of
packet69's `00AC6600` constructor and packet63's opt-in loader retention.
Neither their implementation nor activation belongs in this proposal.

The separate Source admission gap remains real: `GuiPageRegistry` uniquely
owns typed pages, `release_screen_page` logs native release as unimplemented
and leaves them loaded, and the menu's projected last-handle path hides a
page. Its plain integer decrement is not an Original page+04 transport.
Retaining failed loads or a raw Text count does not own these typed layouts
or freeze public child removal/reparenting. Real last-consumer scheduling and
layout/ancestor retention must be supplied before normal native Text retirement;
neither the proposed raw destructor nor a no-op token supplies it.

No new build/tests were run for this documentation-only packet. The report
records the fixed-baseline Source hashes and exact live body/EH evidence.
