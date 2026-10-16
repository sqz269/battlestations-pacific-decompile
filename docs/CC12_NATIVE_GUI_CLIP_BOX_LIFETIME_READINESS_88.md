# Raw ClipBox lifetime readiness — packet 88

Read-only review of published Main `6647c210a528bea78d5a8e73ec023ba02749a801`, merged without conflict into worker `69ee77cf0f12f4af060761b2ff2141eac44d4485`, preserving packet83. No Source, Ghidra, ledger, shared CMake or Main changes, builds or tests were performed. All live queries verified the existing `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 LE32 target. Installed Original SHA256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

`ACE620` is the whole 38-byte scalar deleting destructor, exclusive end `ACE646`, 12 instructions. Native ECX is the actual ClipBox; its caller supplies a DWORD flags slot on the stack; EAX returns the saved identity; RET4 starts at ACE643. The actual nondeleting callee is **directly AA9730**, not a separate ClipBox wrapper. Table D5D058+04 contains ACE620, matching the vtable installed by the real packet83 constructors.

| Site | Exact action/order |
|---|---|
| ACE621 | Save ECX in ESI |
| ACE623 | Stamp D5D058 |
| ACE629 | Whole raw base destructor AA9730 |
| ACE62E | Only after base success, test low byte of the actual caller flags slot, `[ESP+8]&1` |
| ACE635/636/63B | If bit0 set, pass the SAME saved widget to ACE4A0 with pool F8C13C |
| ACE640/643 | Return saved identity, RET4; no payload access after pool return |

This wrapper has no own EH frame, constructor-style slot-return catch, reference-count load/decrement/gate, derived-tail cleanup or intervening nondeleting ClipBox body. The derived EC..108 float fields are not read or cleared. Flags0 still ends base lifetime but retains storage; flags1 additionally returns the slot. The returned address after flags1 is an identity, not a live object. A nonzero +04 count does not prevent explicit deletion.

AA9730 is the full 649-byte body, exclusive end AA99B9, real RET at AA99B8. Full paged live listing and disk disassembly both contain 204 identical instruction starts with no gap; the live proto statistics field reports203, which is recorded separately without repair. It stamps D5C130, releases descendant/current scene nodes through AA8320, repeatedly deletes the current first child through current slot04(flags1), and reloads the child list after callbacks. The child must detach itself for native progress; null/no-progress entries are not repaired. Then current parent/node links are detached, any replacement main node released, and timed entries deleted through their current slot00(flags1). Timed deletion captures the actual slot address before the callback and clears that original slot afterward. Current counts/headers and callback-written fields are reloaded according to the existing raw body.

Normal completion frees timed backing, destroys child-list nodes/sentinel, stamps D5C104, then BD30F0 stamps CEB130. It preserves +04 and never returns the widget slot. Timed backing +88 is not generally cleared after free, so retained raw storage is not a valid restartable ClipBox. The existing Source `destroy_native_gui_widget_base_00aa9730` implements this schedule over `NativeGuiWidgetLifetimeContext`; no typed `GuiWidgetOwner` or ClipBox cast is required.

The native base handler is CB7421 → info DEDCA0, maxState3/map DEDC88. State2 → state1 uses CB7413, AA7F50(widget+88), backing only; state1 → state0 uses CB7408, A9BCE0(widget+64), list storage only; state0 → -1 uses CB7400, AA6E10(widget), ref-base stamps. State2 is armed before AA8320; state1 at AA9895 before final backing work; state-1 at AA9993 before terminal ref-base destruction. Source `unwind_base` is noexcept and terminates on a second cleanup exception. This is the supported C++ exception projection, not private FH3/SEH or double-exception ABI equivalence.

| Outcome | Slot/graph obligation |
|---|---|
| Base succeeds, flags0 | Base lifetime ended, slot retained; no automatic reinitialization or pool return |
| Base succeeds, flags1 | Return the same F8C13C slot; no subsequent payload access |
| Base throws while state2 | Unwind backing/list/ref only; no scalar flags read or pool return; unfinished child/timed/derived payloads require previously retained external owners |
| Base throws while state1 | Unwind list/ref only; earlier effects and any unfinished backing allocation state remain authoritative |
| Cleanup throws again | Source terminates; no resumable cleanup claim |

The owner must retain the partial raw graph and services **before** entry. Container unwind can destroy the links needed to rediscover unfinished children or entries, so retention cannot be reconstructed by traversing the failed object's old headers. Retaining the raw slot alone does not retain all payloads or authorize replay. Actual node/model companions, dynamic child/timed destructor bindings, parent-list removal, borrowed tables/type cells, raw-name/VFS domains and shared renderer allocation owners must survive every reached callback. Callbacks must not free the active widget slot. No new count gate, semantic shared_ptr alias, fake manager, cleanup resumption or inferred successful host is justified.

`NativeGuiWidgetLifetimeContext` requires the SAME `NativeNodeDestructionRuntime`, live base table D5C130, initialized two-DWORD F8BC88 lineage, live F8BE28 Text descriptor and exact current-table/class/child/timed/parent-list bindings. No concrete production `NativeGuiWidgetLifetimeBindings` subclass was found in current `src/`; full populated-graph composition therefore remains required. Existing native-material clip registration uses retained `GuiWidgetOwner`/ClipBox companions and explicitly borrows parameter source addresses without adding a widget retain. It cannot be cast onto packet83's raw ClipBox slot. Any eventual raw material-source integration must close that consumer lifetime separately; this destructor contains no extra material-source retirement step.

Pool return is genuine ACE4A0: SAME initialized `GameNativeGuiWidgetPoolProcess<GuiClipBoxPoolTraits>`, F8C13C, 10C payload/110 stride in the shared E188B4 domain. It loads the live hidden slab index at+10C under the pool lock, pushes the slot index onto the slab free stack and updates its count/first-free slab. It neither destroys the payload nor supplies a foreign-slot check. The process/header/slabs/allocator domain must remain live through return; do not construct a second pool. Source's metadata accessor can reject incomplete startup; invoking it only after successful base destruction does not turn a late Source error into healthy preflight or allow destructor replay.

The smallest truthful future Source packet is only whole ACE620: tentatively append `delete_native_gui_clip_box_00ace620(void* actual_clip_box, const volatile std::uint32_t& actual_flags_slot, NativeGuiWidgetLifetimeContext&)` to the existing `native_gui_clip_box_factory.hpp/.cpp`, with files/lease finalized by Root. Stamp D5D058, invoke the genuine raw base destructor, then load only the flags cell's low byte and conditionally return through the SAME process pool; return the saved address. The stable borrowed flags cell allows callback changes to be observed after base success. No wrapper catch, retry/reset/destructor fallback or invented partial cleanup is added. Caller serialization, initialized canonical pool, raw owner graph retention and existing base supported domains remain preconditions.

The existing generic scalar helper already records the correct callee/vtable/pool row, but accepts flags by value and an abstract destructor host; it does not provide this exact live caller-slot binding. A new narrow raw wrapper can be Source-complete under genuine supplied domains without claiming populated-graph retirement, native activation, binary replacement ABI or runtime proof. This packet proposes that wrapper only. The [JSON receipt](../reports/cc12_native_gui_clip_box_lifetime_readiness_88.json) records exact bytes, transfer sites, Source hashes and explicit remaining contracts.
