# Cockpit viewport identity and lifetime design, BH

Addresses: `00B71A80`, `00B3C800`, `00B1F850`, `00B71990`, `00BD30E0`,
`00B1F8F0`. Design and static audit only; no implementation or native-body credit.
Worker base `58145d55`, 2026-09-13. BG composition contract `ff8e720f` was read
in its worktree. The current source and target-guarded BSP CLI were then checked.
Target: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`,
`x86:LE:32:default`, image base `00400000`, bridge `127.0.0.1:8089`.
BSP's client verifies project name, program path, language and base for each live
batch. This packet made no Ghidra writes, exports, ledger edits or C++ changes.

## Decision and narrow boundary

Use one persistent actual-identity registry implementing `CameraViewportResolver`.
Before the native constructor sequence, admit **two distinct stable view records**:
one for B71A80's first viewport and one for B3C800's replacement. Construct each
`CameraViewport` directly over its successfully constructed actual 34h owner,
without allocation, before native camera publication. Resolve only live entries.
Retire the identity at the common viewport destructor before its typed lifetime
ends and before physical free. Never destroy or rebind a published host view record
until their host consumers are explicitly quiescent. Exactly two records live in
each persistent construction record; these neither retain nor delay native free.

This closes registry/address-reuse lifetime without changing native ownership.
It does **not** prove that arbitrary concurrent or reentrant destruction during
a viewport field read is safe. The original renderer also borrows the raw owner.
Current code has no demonstrated production scheduler contract proving that
callback case; see the precise remaining boundary below. A new shadow count,
extra native retain, delayed native free, synthetic viewport or forced renderer
cache clear is not an implementation of the recovered sequence.

## Native allocation, publication and count evidence

| Routine | Inclusive native extent | ABI | Coverage |
| --- | --- | --- | --- |
| B71A80 | B71A80..B71CDB, 604 bytes | ECX camera, stacked name, EAX same, RET4 | Complete normal listing; scoped allocation/unwind audited; other callees retain existing contracts |
| B3C800 | B3C800..B3C952, 339 bytes | ECX helper, EAX same, RET | Complete normal listing read; BG owns the full constructor/EH contract |
| B1F850 | B1F850..B1F8EB, 156 bytes | ECX actual viewport, EAX same, RET | Complete normal listing and base unwind |
| B71990 | B71990..B719D0, 65 bytes | ECX current camera, stacked actual viewport, RET4 | Complete listing |
| BD30E0 | BD30E0..BD30ED, 14 bytes | ECX actual owner, no caller stack args, RET | Complete listing; dynamic slot+4 with flags1 |
| B1F8F0 | B1F8F0..B1F913, 36 bytes | ECX actual viewport, stacked flags, EAX original, RET4 | Complete listing |
| B71F10 / B71FE0 | B71F10..B71FD1 / B71FE0..B71FFF | ECX camera; deleting entry stacked flags, RET4 | Complete normal listings; nonviewport callees not reimplemented |
| B26770 | B26770..B26891, 290 bytes | ECX renderer, stacked viewport, RET4 | Complete listing; borrowed viewport behavior |
| B71360 | B71360..B713C5, 102 bytes | ECX camera, RET | Complete listing; current renderer and current camera+180 loads |
| B1D120 | B1D120..B1D1C7, 168 bytes | ECX context, RET | Complete listing; camera+08 release before clearing |

B71AA0 captures the incoming camera in ESI, which is never reassigned in the
complete listing; EBX is zeroed at B71AAC and stays zero. B71ABA calls BF681B
with `PUSH34h`; `ADD ESP,4` is at B71ABF. The result is saved before state1.
B71AD1 calls B1F850 on that exact nonnull allocation. Only after success does
B71ADD write EAX to camera+180. This is ownership transfer of the initial count1,
with no increment. The current raw source is `native_camera_owner.cpp:256`,
allocation/publication at280; the semantic overload at196 has the same boundary.

B1F850 establishes CEB130, actual count1, D5E5F8, origin0, provisional native
640/480 and depth words, then reloads F8D394 separately for width and height.
The calls at B1F8BB/B1F8CE use each captured renderer's current virtual+30.
Byte20 becomes zero only at B1F8DA; +21..33 remain preimage. Native constructor
defaults are preserved stores, not permission for a host fallback renderer.
Successful return is the registration point: earlier renderer callbacks must
not obtain a fabricated finished view for this under-construction allocation.

B3C8F8 is the second BF681B allocation, again `PUSH34h`, `ADD ESP,4` at B3C8FD.
B3C90F runs B1F850 and B3C914 captures the actual returned viewport in EDI.
B3C91A reloads **current helper+0C**; B3C923 passes EDI to B71990 after state0
is restored. Do not close over the original camera companion: preceding real
provider callbacks may change helper+0C. Resolve its current identity through
the canonical node/camera associations. EDI remains the local replacement
through the final decrement, regardless of later camera+180 contents.

| Site | Actual native effect |
| --- | --- |
| B71995 / B7199B | Capture old camera+180 in ESI; identical incoming identity skips all writes and counts |
| B719A1 | Publish incoming actual viewport to camera+180 |
| B719AD | Current CE221C increment of incoming+04 when nonnull |
| B719BB | Current CE2220 decrement of captured old+04 when nonnull |
| B719CB | Only zero: captured old's current profile slot0, ECX captured old |
| B3C92C | Current CE2220 decrement of **local EDI+04**, not current camera+180 |
| B3C93C | Only zero: local EDI's current profile slot0 |
| B71F54 / B71F60 / B71F62 | Decrement captured camera+180; zero invokes its current slot0; clear camera+180 only afterward |
| BD30EB | Current owner profile slot+4, stacked1; no further decrement |
| B1F8F9 / B1F906 | BD30F0 restores base; BF65AC frees same address iff flags&1; ADD ESP,4 at B1F90B |

Live D5E5F8 contains `[BD30E0,B1F8F0]`. Current viewport release code implements
this concrete domain directly; it does not dispatch arbitrary replacement
profiles. Unknown profiles need an evidenced provider, not a fallback. On an
ordinary untouched constructor, first count1 reaches zero on replacement;
replacement count1 becomes2 in B71990 and1 at B3C92C. These are examples of
actual arithmetic, not registry-maintained counts. A callback or other owner
may make any particular release nonterminal. Retirement follows the observed
native destructor, never an expectation that a viewport is the first or second.

## Failure and cancellation

| Failure/event | Native result | Host admission/registry action |
| --- | --- | --- |
| Preparation fails before native events | No constructor event occurred | Cancel both unused records and other admission credits |
| First BF681B allocation fails | Existing CRT new-handler/throw path; no viewport | Cancel unused first record after native cleanup; do not invent null success |
| First B1F850 throws | CBCCB0..CBCCB7 jumps BD30F0; CC1AD8..CC1AE2 frees exact saved raw allocation at CC1ADC, POP ECX at CC1AE1 | No published registry entry; cancel reserved record; no release of unconstructed owner |
| B71A80 fails after B71ADD | CC1AE3..CC1AF0 optionally performs +438 action; CC1AD0..CC1AD7 calls node base; **no viewport release** | Preserve successful first entry independently of dead camera preparation; do not free viewport or recycle its view record |
| B71A80 succeeds | First actual viewport belongs to camera+180 | Persistent first entry already live; bind camera lifetime before helper+0C publication |
| B3C800 fails after B3C892 | Published camera survives; outer state0 is helper base only | Keep camera owner/reference, environment and first viewport entry; cancel only unconsumed replacement admission |
| Replacement B1F850 throws | Own base unwind, then CBED09..CBED13 raw free at CBED0D, POP ECX at CBED12 | Replacement remains unregistered; first viewport/camera preserved |
| Replacement construction succeeds | Local actual count1 exists | Consume second record before B71990; never let ticket destruction release it |
| B71990 or later throws | State0; no automatic local viewport cleanup | Preserve every native-surviving entry; do not synthesize a local RAII release |
| Any actual B1F8F0 entry, including flags0 | Owner typed lifetime ends; only flags1 physically frees | Retire identity even for flags0; inert host view record survives until quiescence |

The first-viewport survivor after failed B71A80 can be orphaned by native unwind;
the registry records that reality without taking native ownership. Returning
the raw camera slot with B71350 does not retire the separate heap viewport.
The probe's extra cleanup of this allocation (`native_camera_probe.cpp:165`)
is explicitly diagnostic cleanup and must not be copied into the constructor.
Cancellation only changes host reservation state; it cannot unregister a
consumed live entry. Do not inspect ended camera typed storage to rediscover
the viewport: retain its already-known registry identity independently.

## Minimal file/API proposal

New `include/bsp/native_viewport_registry.hpp` and
`src/native_viewport_registry.cpp` contain only the following concrete provider:

```cpp
class NativeViewportRegistry final : public CameraViewportResolver {
public:
    class Storage;   // stable caller-owned storage for one view/association
    class Admission; // move-only, one reserved record; not a native owner
    Admission admit(Storage&);          // host preparation and uniqueness check
    void cancel(Admission&) noexcept;   // only reserved -> cancelled
    void constructed(Admission&, NativeViewportOwner&) noexcept;
    const CameraViewport* resolve_viewport(NativeViewportOwner*) override;
    void retire_before_destroy(NativeViewportOwner&) noexcept;
    void forget_quiescent(Storage&) noexcept; // after explicit host borrow end
};
```

Allocate the persistent cockpit construction record outside native events. Embed
exactly `Storage viewport_records[2]` in it, alongside the separate camera/helper
companions and admission state. Link these caller-owned records into one intrusive
registry with state `reserved/live/retired/cancelled`. An admission identifies
its exact runtime and reserved record. `constructed` consumes it exactly once,
checks duplicate identity/concrete profile as admission invariants, and emplaces
`CameraViewport(actual_owner)` in reserved storage. Neither the record nor view
is copied or moved afterward; copying `CameraViewport` would create diagnostic
storage in current code. No hash insertion, vector growth, callback or allocation
occurs in `constructed`, lookup, cancellation or retirement. Missing lookup
returns no view; existing `CameraViewportSlot::get` rejects it, never substitutes
dimensions or another identity. Normal admitted lookup is nonthrowing.

Remove retired entries from the live key chain before native destruction/free.
Keep each retired record and its view object inert, with no field dereference,
rebinding or reuse until explicit host quiescence. This needs no auxiliary
refcount or borrow allocator and no indefinitely growing generic tombstone arena. A newly
allocated owner at the same raw address receives a different reserved record;
lookup never returns the earlier tombstone. Unconsumed cancelled records may be
forgotten because their view address was never exposed. The caller may invoke
`forget_quiescent` on a retired record only after all semantic frame calls using
that view returned and all host `viewport()` consumers/caches released their
borrow; then end its view lifetime, unlink it and reclaim its construction block
once the block's other companions are also retired. A live or orphaned native
viewport prevents that block's reclamation. The runtime must explicitly retain
the block on the failure-survivor path independently of helper, camera, scene and
node-admission ticket lifetimes. Registry shutdown does not force-release owners.

Because existing final-release APIs accept only the native viewport address,
add one **module-scoped borrowed registry installation** during host preparation,
with one canonical registry for the managed native viewport domain. Install it
before any admitted construction and keep it through the last native destruction
and host borrow. It is host provider plumbing, not a new native global or field.
Use `NativeViewportRegistryBinding(registry)` outside the native sequence to
publish a host-only pointer in `native_viewport_registry.cpp`. Its constructor
rejects a competing installation; a second construction runtime explicitly borrows
the same registry instead of installing another. All managed actual heap owners
then share one address domain and intrusive key chain, whose records also name
their caller-owned construction block. The common destructor consults this pointer
once and invokes allocation-free retirement by exact owner address; unregistered
diagnostic owners have no entry to retire. Direct scalar destruction, ordinary
zero-count release, camera replacement, camera teardown and the local B3C800
release all route through this same source destructor. No environment argument
or callback record is read out of dead native storage. Direct raw CRT freeing of
a registered live owner is an invalid provider bypass and must not be introduced.
Binding destruction requires no registered/reserved records, no pending producers
and explicit host quiescence; it never swaps to another registry while managed
objects remain. Do not create per-camera registries or add a backpointer to the
exact 34h native allocation. This is one narrow host service domain, not a claim
that an uninstrumented original executable's destructor reaches a C++ registry.

Narrow coordinated edits:

| Files | Exact change boundary |
| --- | --- |
| native_viewport_owner.hpp/.cpp | Add an admitted allocation overload. Preserve BF681B allocation and B1F850 cleanup; after successful initialization call `constructed` before returning. Keep legacy diagnostic overload. At start of common `delete_native_viewport_owner_00b1f8f0`, retire a registered identity before either vtable store, typed destructor or physical free. No external callbacks in retirement. |
| native_camera_owner.hpp/.cpp | Carry an explicit first-viewport admission in persistent camera preparation, or add an explicit admitted raw B71A80 overload. At existing allocation/publication expression use admitted allocator. Do not move preparation allocations inside B71A80. Leave native setter and teardown count/publication order intact. |
| new native_cockpit_construction_runtime.hpp/.cpp | Own/share registry installation, embed two stable records in persistent construction storage and admit each explicitly; bind NativeCameraEnvironment.viewport_views to the registry. Retain construction blocks and providers independently of helper disposal, with explicit failure-survivor and host-quiescence disposal paths. Consume second admission via same viewport allocation provider for B3C800. |
| future native_cockpit_helper_construction.cpp | Forward explicit admissions; preserve B3C91A current-helper-camera lookup and B3C92C local-EDI release. No source implementation in this packet. |

A mutable shared `next_admission` slot in NativeViewportEnvironment is unsafe:
renderer callbacks can reenter construction. Each attempt must carry its own
two records; a nested admitted attempt consumes its own records only. The
registry bookkeeping itself invokes no user callbacks. A nested host preparation
may allocate its separate block but must preserve existing records. Cross-thread
registry synchronization and raw camera+180 publication must share the actual
provider scheduling contract; a mutex around lookup alone cannot make a raw
owner borrow safe. No new generalized registry framework is proposed.

The parallel scene/lifetime design owns separate reservation credits. Cancelling
those credits cannot retire a viewport; consuming a viewport admission cannot
bind or unbind a camera. Camera companion registration may be retired only by
the actual camera lifetime path described next.

## Native renderer borrow versus host view borrow

Complete B26770 has no viewport increment or decrement. Optional guard entry
precedes loading the actual argument into ESI at B267A2. Four address-getter
calls read origin/dimensions; B2680F calls current device SetViewport. Only
after it returns does B2681A publish **actual ESI** to renderer+1904. B26820
reads byte20, B26830 calls B24460, B26837 reads byte20 again, and B2684B supplies
actual+24 to SetScissorRect at B2685E. Native callbacks may change fields and
publications; those captures/reloads must remain. Neither the renderer guard
nor camera queue retention proves an extra retained viewport snapshot.

The production raw shim `native_renderer_viewport_clear.cpp:70` follows that
actual-owner path directly. `native_camera_frame_command.cpp:45` loads current
camera+180 and reaches it; it does not resolve a `CameraViewport` wrapper.
Thus the host registry must not rewrite native renderer+1904 when retiring a
wrapper. Current native draw/indexed providers load +1904 and call B1F740, but
that complete leaf is only `LEA EAX,[ECX+10h]; RET`; these callers discard the
returned address. This is not evidence of a later viewport-field dereference.

Renderer destruction's direct body B32920..B32D72 was read in decompilation,
with its prefix listing through B329DA. At B329C7/B329CA it passes renderer+34
to B241C0. Therefore renderer+1904 is **cache+18D0**. Complete B241C0's normal
decompilation and terminal listing preserve +18D0; its +1904 zero is renderer
+1938, an unrelated field. B29430 initializes cache+18D0 at B294E4 without
retention. B339F0 calls B32920 then conditionally frees the renderer. B32920's
other indirect cleanup callbacks and B24BF0's nested providers are not a full
renderer teardown reconstruction here; no general lifetime guarantee is inferred
from their names or from their lack of a direct +1904 read.

The semantic `D3D9CameraFrameAccess::bind_viewport_00b26770` separately caches
`&value` at `camera_frame_state.cpp:1113`; public `viewport()` exposes this host
pointer indefinitely. It is not the native renderer word. Keeping both construction
records until their explicit host quiescence prevents this extra pointer from
pointing into freed companion storage. It does not make the dead native field
references usable. Current semantic API provides no cache-detachment/borrow-end
proof, so block reclamation must remain pending while it still exposes that view.
The narrow optional follow-up, if this semantic API is used by production, is a
host-only `cached_viewport_identity` query/diagnostic policy which never
dereferences a retired view. It must preserve actual renderer+1904 and must be
coordinated with the camera-frame file owner. It is unnecessary for the raw shim.

An active view borrow must end before its actual owner's destruction. Native
B26770 itself would read freed storage if a device/state callback terminally
freed its captured viewport before its later scissor reads. Source/runtime
evidence here establishes no such permitted destructive callback or exclusion
protocol. **Remaining runtime blocker for claiming arbitrary callback-safe field
borrowing:** audit the actual renderer/device callback and execution domain and
show the captured owner stays alive through its last native read. Do not satisfy
this by adding native retention or delaying physical free. Field mutation,
current helper+0C replacement, nested preparation, and unrelated release remain
valid supported reentry; they are not blanket-disabled by this design.

## Queued camera lifetime and limits

`NativeCameraReference` borrows the actual camera+04 count. A context retains the
camera, not a snapshot of its viewport: semantic context setup is
`render_command_queue.cpp:127`; B1D120 at B1D15D decrements its captured camera,
then B1D169 performs terminal slot0 before clearing context+08 at B1D16B.
Helper destruction uses the canonical current node binding and logical release;
if a queue still retains the camera, its companion and environment remain alive.
The actual camera terminal callback calls B71FE0, whose B71FE3 calls B71F10;
B71FF5 returns the pool slot only after destruction. The companion retirement
callback runs after canonical unbind, with no subsequent use of the dead owner.
Freeing the 24h helper therefore cannot own or invalidate this registry, camera
companion, disposal context, node runtime, renderer access or constant cells.

Queue camera retention protects the camera's lifetime but does not retain a
previous viewport after B71990 changes +180. Two admitted view records cannot
turn the first into a permanent camera child. Conversely, a first viewport
retained elsewhere stays registered when replacement releases only one count.

All-callers check: live B1F850 xrefs enumerate20 direct sites, and B71990 xrefs
enumerate28. This packet inspects the two B1F850 cockpit paths and the B3C923
setter caller, not the other constructors' setup. B1F8F0 has no direct callers;
its concrete table edge was verified by bytes. B26770's indirect call graph is
not a complete caller enumeration. No universal caller-domain or whole-renderer
claim is made. No missing function boundaries were found in the scoped viewport
allocation, release and EH set. Unexpanded callee contracts, full renderer EH,
all native viewport producers, concurrent mutation and runtime/gameplay behavior
remain outside this design's proof.

Validation: exact report call rows are checked with
`python tools/verify_report_calls.py reports/native_cockpit_viewport_lifetime_bh.json`.
Result:22 direct/tail rows checked,0 failed;15 indirect rows manually inspected
in their containing listings and excluded by the mechanical checker.
No build or tests were run because this packet edits only this design and report.
