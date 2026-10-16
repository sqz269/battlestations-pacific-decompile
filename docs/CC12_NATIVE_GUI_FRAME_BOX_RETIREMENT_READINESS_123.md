# Raw FrameBox retirement readiness 123

Whole raw `AD2880` and its actual current-profile scalar `AD29B0` are the next
small Source bodies ready to compose after packet 119, within its explicit
supplied domain. They are not yet present as that concrete composition. The
existing `gui_framebox_destroy_00ad2880` uses `GuiWidgetMemberHost` and the pure
`GuiFrameBoxHost::destroy_state_vector_00ad21b0` seam; the generic scalar delegates
to `GuiWidgetDestructionHost` and receives flags by value. Their older
reconstructed/build-tested ledger status does not close those raw services or
the cleanup schedule. No duplicate tidy, record, base or pool algorithm is needed.

This packet is read-only. Baseline `f0953198363a34181965ea12c0309b075a77a035`
is retained clean; no merge/build is performed. Packet 119 is read only from
the other worker's sealed commit `2f9394312ca953ce8541657fb754a73164219062`,
not merged here. Its reviewed Source is an input, not a new execution claim.

## Actual entries and whole evidence

The genuine eight bytes at `D5D130` are `e0 30 bd 00 b0 29 ad 00`: current
slot00 is `BD30E0`, and current slot04 is **AD29B0**, discovered from the table.
The latter is not inferred from a typed class or a guessed adjacent address.

| Whole body | Extent | Bytes / starts | SHA-256 |
| --- | --- | --- | --- |
| FrameBox destructor | `[AD2880,AD28FD)` | 125 / 33 | `e17795d2d1c9545e06b01b5f3f0193c6839441b9234d1c903239508a57c74bb1` |
| FrameBox scalar | `[AD29B0,AD29D0)` | 32 / 11 | `a0ad4a34f91b609499e16e1db4161e3337fe15d091e0512bc3cc8cd357fc3874` |
| Existing FrameBox pool return | `[ACF610,ACF678)` | 104 / 33 | `c3caa27c341f6019787a0da0f8e047bde7c9c97c9b01dea7249a7a46f4567519` |

AD2880 takes the actual receiver in ECX, no stacked arguments, and plain RET;
there is no semantic EAX result. AD29B0 takes the actual receiver in ECX and a
stacked flags DWORD, returns the saved receiver identity in EAX, and RET4.
ACF610 takes the real pool in ECX and the actual slot on the stack, RET4.
These facts come from complete physical listings, rather than Ghidra's invented
zero-parameter prototypes. Nine fresh live spans total 353 bytes, including the
table, handler, FuncInfo, unwind map and both actions. All equal installed
Original bytes; the three whole bodies total 261 bytes / 77 starts.

## Normal and exception schedule

1. AD2880 installs its FH3 frame and saves the receiver. Stamp DWORD `D5D130`
   at owner+00. Capture CURRENT name data at +108 **before** storing state 1.
2. Only captured-nonnull data reads CURRENT length+104 and adds one with DWORD
   wrap. Prepare the pending return operands `(captured data, size, 1)`, call
   real `419CC0`, then real `BD1510` on the returned pool. Do not reread the name
   data or clear either header word. Getter/reentry can change current headers;
   the captured operands remain the pending return's identities.
3. Form actual owner+F0 and consume state 1 to state 0 at AD28D4, **before**
   normal `AD21B0` at AD28D9. Packet 119 captures its current begin/end here.
4. Consume state 0 to -1 at AD28E0, **before** real `AA9730` at AD28E8. The
   outer destructor adds no repeated base cleanup if that call escapes.
5. Only after AD2880 returns does AD29B0 read CURRENT LOWBYTE of its stacked
   flags at AD29B8. Bit0 calls real ACF610 with pool `F8C188` and the saved
   receiver. Return the saved identity afterward, without reading its payload.
   No widget count read/decrement, ancestor gate or ownership token is present.

Handler `CB9306` loads actual FuncInfo `DF08BC` and tails to `BF6B43`.
FuncInfo magic is `19930522`, maxState2, map `DF08AC`:

| Entered state | Consumed next state | Actual action |
| --- | --- | --- |
| 1 | 0 | `CB92F8`: saved owner `[EBP-10]`, add F0, tail `AD2320` |
| 0 | -1 | `CB92F0`: SAME saved owner `[EBP-10]`, tail `AA9730` |

Thus a name getter/return escape in state1 cleans the vector then the base;
there is **no name retry**. A normal tidy escape in state0 cleans only the base,
never replays tidy. The established AD2320 five-byte tail reaches AD21B0.
Future ordinary Source C++ cleanup must consume each action before entering it
and terminate on cleanup failure. This static Native map does not prove Native
FH3 runtime behavior, compiler state placement, private-frame aliasing or SEH.

## Genuine available services and remaining authority

| Needed service | Current concrete Source / admission boundary |
| --- | --- |
| Name return | Existing raw `NativeStringRawPoolContext`, genuine 419CC0 and BD1510; SAME publication01090AA8, gate01090AA4 and actual-manager01090AA0. No semantic pool or cached getter. |
| Vector normal / EH tidy | Sealed119 `tidy_native_gui_frame_box_vector_00ad21b0` / `cleanup_native_gui_frame_box_vector_00ad2320`, SAME actual header+F0, owners, strings and persistent prepared operation. |
| Actual record release | Whole111 with distinct fresh per-record metadata; only its established completed canonical registered2D D61948/BD30E0/B3F590 creator family, SAME registry/texture context/pool. No arbitrary terminal/profile. Failed103 records are excluded. |
| Base retirement | Existing whole `destroy_native_gui_widget_base_00aa9730` and real AA8320, same actual node runtime and widget lifetime context. Current child20/class0C/child04/timed00 and real A9BD50 transport remain mandatory genuine bindings. No typed widget cast, default terminal or manufactured child progress. |
| Pool return | Existing `NativeGuiWidgetPool<GuiFrameBoxPoolTraits>::free`, explicitly instantiated over the actual shared pool engine. Borrow the SAME initialized pool that allocated this slot; do not duplicate the pool or add a callback. |
| Concrete production graph | Not established by this packet or119: actual initialized FrameBox ownership, supported child/timed/current-profile dispatch, real node/resource relationships, independent graph retention, preparation and matching pool/slab lifetime must already exist. A context alone does not produce them. |

The parent name action precedes normal **and potentially EH** vector tidy.
Caller must prepare the SAME119 operation and every111 record frame before
entering the parent. Its expected first/end correspondence must survive all
earlier reached name getter/return callbacks until the reached tidy captures
begin/end. This is a supplied-domain restriction, not general support for a
callback-replaced vector. There is no prepared replacement selector, late
allocation or extra early raw-header snapshot. After capture,119's own cached
iteration/current-free rules apply: retain captured backing and any current
free allocation independently; do not invent an extra cached-begin free.

Metadata owns no graph. Retain the actual storage, all candidate backing,
creator/resource/name/node/provider graph and stable metadata independently,
outside the parent's failing scope, BEFORE callbacks. In particular, do not
own119's operation as an automatic failed-unwind local: its entered/failed
destructor deliberately terminates. On tidy failure the parent can still enter
base cleanup; independent retention cannot be inferred from widget/container
links that cleanup may remove. Failed/partly retired identities are diagnostics,
not replay capabilities; even another fresh operation cannot retry that lifetime.
Metadata, contexts, flags and provider scratch stay live/disjoint and access is
externally serialized. Caller excludes a second/reentrant destruction entry on
this same owning lifetime. No count rollback, automatic retain or repair is added.

## Constructor and real pool provenance

Whole raw96 AD2600 is available and calls genuine AA9390(type18). It initializes
begin/end/capacity at +F4/+F8/+FC and the name +104/+108, but leaves vector
allocator/proxy +F0, +EE..+F3, padding +101..+103 and base-unwritten bytes alone.
Do not blanket-zero them or treat a zeroed buffer as constructor evidence.
Only successfully constructed owners belong in the destruction domain;
whole copy/factory/AD1910 rollback or populated production ownership is not
inferred from103 record copy or a constructor prefix.
Parent storage must be live, DWORD-aligned actual FrameBox storage covering
00..11B with its genuine raw base/containers/name/vector lifetimes; a semantic
FrameBox/vector/resource projection is not that storage. A deleting slot also
needs the genuine SAME pool's initialized trailer beyond the payload.

F8C188 is the actual 38h FrameBox pool role. Payload is11Ch, stride120h,
16 slots, trailer DWORD+11C, slab1224h, WORD free indices1200h/count1220h.
ACF610 enters the real pool+0C lock, increments current recursion+24, THEN
reads CURRENT slot+11C and CURRENT slab table+28. Signed `(slot-slab)/120h`
becomes a WORD index; append at current freecount, increment that WORD, lower
current first-free+34 if needed, decrement recursion, Leave. Do not cache the
trailer before destruction/locking; trimming can move tables/rewrite trailers.
The existing explicit process startup uses the SAME physical allocator-list
domain; it requires completed startup and destroys slabs without payload
destructors. No late getter, private owner, blanket-initialized payload, foreign
heap slot or use after pool teardown is admitted. Source Win32 calls do not
preserve Original IAT/private ABI identity.

## One proposed Source packet

Only whole AD2880 and AD29B0 in new
`include/bsp/native_gui_frame_box_lifetime.hpp` and
`src/native_gui_frame_box_lifetime.cpp`, under a fresh assignment/lease.
A required borrowed `NativeGuiFrameBoxLifetimeContext` can carry exactly
`NativeGuiWidgetLifetimeContext&`, `NativeStringRawPoolContext&`,
`NativeRenderActualOwners&`, and the SAME persistent
`NativeGuiFrameBoxVectorTidyOperation&`. Proposed functions are
`destroy_native_gui_frame_box_00ad2880(void*, context&)` and
`scalar_delete_native_gui_frame_box_00ad29b0(void*, const volatile uint32_t&,
context&, NativeGuiWidgetPool<GuiFrameBoxPoolTraits>&)`.
Preflight metadata before native effects; retain explicit raw capture/state/
call order, no name clear, normal-only scalar return and a current low-byte read.
The scalar's required pool is used only after successful destruction and bit0.
These would be new Source interfaces, not Native ECX/RET4/private-EH replacements.
No all-type dispatcher, producer, generic capability binder, new counter/global,
normal-host activation or production-ready claim is proposed.

All live batches used verified BSP wrappers for `C:/Users/sqz269/bsp.gpr` and
`/battlestationspacific.exe`. Six direct call rows mechanically verify; the two
pool IAT calls and three EH tails are separately listed/byte-audited. Original
SHA-256 remains `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Twenty-nine Source hashes, exact119 commit/blobs, prior96/103/111/116/117/119
receipts, whole bytes/listings and wrapper/config evidence are sealed in the
companion report and private `local/cc12_native_gui_frame_box_retirement_readiness_123_artifact_manifest.json`.
No Source/CMake/ledger/Ghidra/Original change, build, test, probe, runtime,
Native ABI/FH3/SEH, visual/gameplay or production activation proof is claimed.
