# Persistent cockpit construction block, BJ

Addresses: none (host storage and admission design). Native context is
`00B3C800`, with its existing camera/viewport callees; native body credit is zero.
Names below are C++ hypotheses, not recovered native symbols. This packet defines
the smallest persistent caller-owned block and a bounded implementation packet.
It does not implement the constructor or claim whole render-service readiness.

Worker base `9b890a53e587ae0c62bfbfdc10d3f1c9502cfad8`, branch
`agent/orch3-cockpit-block-bj`, 2026-09-13. At that published base, scene/lifetime
credits, camera owner/reference admitted constructors, stable viewport records,
and admitted viewport allocation exist. Raw camera first-viewport admission is
implemented separately at `5f2574ac2b107f71002141afe52f938e8ef6edcd`; its exact
source diff was reviewed here. Parent combined revision is
`b811585de88e3bf9a4f7f7bb8d5d15484dda8ae0`; build/publication validation was pending
when supplied. This worker does not merge it or represent it as published.

Read prerequisites: `NATIVE_COCKPIT_CONSTRUCTION_COMPOSITION_BG.md`,
`NATIVE_COCKPIT_REGISTRY_ADMISSION_BH.md`, and
`NATIVE_COCKPIT_VIEWPORT_LIFETIME_BH.md`. The BG native contract is unchanged
except its plural address heading; BH registry design is unchanged from
`33940421`. Current source, rather than the earlier proposal signatures, governs
viewport admission/cancellation. BSP CLI live prototype checks and the report
verifier use its target-verified client for `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. No Ghidra writes or exports were made.

## Native boundaries that determine storage ownership

| Routine / evidence | Original ABI and coverage in this packet |
| --- | --- |
| B3C800..B3C952 | ECX actual 24h helper, no stack arguments, EAX same, RET. Complete current decompilation and 108-instruction normal listing read; existing BG owns complete scoped EH evidence. No implementation. |
| B71930..B71939 | No args; replaces ECX with actual 0108FFB0, tail B71770. Full wrapper read; pool allocation semantics use the existing concrete provider. |
| B71A80 | ECX actual 45Ch slot, stacked local name header, EAX actual camera, RET4. Partial native listing: entry through B71B50, including first viewport allocation/publication. Existing raw source and BI diff reviewed; remaining body/EH contract reused from BG/BH. |
| B1F850..B1F8EB | ECX actual 34h viewport, EAX same, RET. Complete normal decompilation/listing read; constructor writes and real renderer callback inputs match existing source. |
| B71990..B719D0 | ECX current camera, stacked actual viewport, RET4. Complete decompilation/listing: identity skip, publish new, retain new, release captured old. |
| BF681B | cdecl size, EAX raw pointer; full normal decompilation/listing reviewed. Existing malloc/new-handler retry or throwing exhaustion contract is retained. No added host allocator call. |

| Site, containing function | Native call and consequence |
| --- | --- |
| B3C84F, B3C800 | B71930 obtains the actual raw camera slot. Block preparation must not call it early. |
| B3C86C, B3C800 | 41E870 constructs the actual local 8h name header; RET4. Its callee body was read. No camera owner companion is needed before this succeeds. |
| B3C886, B3C800 | B71A80 receives that same header unchanged, ECX captured slot; RET4. |
| B71ABA, B71A80 | BF681B, PUSH34h; ADD ESP,4 at B71ABF. Native first viewport allocation remains here. |
| B71AD1, B71A80 | B1F850 constructs that allocation; B71ADD publishes it to camera+180 after success, without an extra retain. |
| B3C8F8, B3C800 | BF681B, PUSH34h; ADD ESP,4 at B3C8FD. This is the replacement allocation. |
| B3C90F, B3C800 | B1F850 returns the actual replacement captured into EDI at B3C914. |
| B3C923, B3C800 | B71990 receives EDI and the camera freshly loaded from helper+0C at B3C91A; RET4. |

B3C892 publishes the camera before state0 at B3C895 and before the local string
return. Later helper failure does not release that published camera. B3C92C
decrements **local EDI+4** through current CE2220; B3C93C invokes that same local
owner's current slot0 only on zero. It does not release a newly reloaded
camera+180. Those two indirect instructions were inspected in B3C800's listing.
Other raw-string/leaf calls and the five-state EH mapping retain the full BG
contract; this packet does not assign them new semantics or body coverage.

The helper's actual 24h allocation is external. B3C800 preserves words
+10/+14/+1C/+20 while initializing the evidenced fields; the block adds no native
fields, stored counts, or replacement object layout. In particular the block is
not allocated in place of the 24h helper or 45Ch camera slot.

## The block and its one-attempt capability

Implement `NativeCockpitConstructionBlock` in new
`include/bsp/native_cockpit_construction_block.hpp` and
`src/native_cockpit_construction_block.cpp`. It is noncopyable and nonmovable.
Its complete persistent payload is:

```cpp
NativeViewportRegistry::Storage viewport_records_[2];
std::optional<NativeCameraOwner> camera_owner_;
std::optional<NativeCameraReference> camera_reference_;
NativeViewportRegistry* viewport_registry_{}; // borrowed through final forgetting
enum class Phase { idle, preparing, prepared, executing, settled };
Phase phase_{Phase::idle};
bool camera_retired_{};                       // host notification, not a count
```

Optional emplacement reserves the actual host object size/alignment inline and
does not allocate. Neither companion is created merely by constructing the
block. No helper companion, raw native allocation, identity map, dynamically
growing list, reference count or allocator ownership is embedded here. The two
records are role-labelled first/replacement for admission routing only; they
never become permanent children of the block's original camera.

Recommended public API:

```cpp
NativeCockpitConstructionBlock() noexcept;
~NativeCockpitConstructionBlock() noexcept;
[[nodiscard]] Admission admit(NativeCameraEnvironment&,
    const NativeNodeRawConstants&, NativeViewportRegistry&);
Phase phase() const noexcept;
void reset_after_host_quiescence() noexcept;
```

Nested `Admission` is default-empty, move-only, with nonthrowing moves,
destruction and cancellation. It holds a block pointer, borrowed camera
environment/raw-constant pointers, one `SceneAttachmentRuntime::BindingAdmission`,
one `GeneratedModelLifetimeRuntime::BindingAdmission`, and exactly two
`NativeViewportRegistry::Admission` values. Its public operations are move,
`cancel() noexcept`, and an active test; its state and consumption are private to
the block and the future constructor. Moving an active destination first cancels
its old unused attempt; self-move is a no-op. No move relocates the block/records.

The later native constructor should accept the capability explicitly, for
example this minimal new C++ interface, declared for friendship without a stub:

```cpp
void* construct_native_cockpit_helper_00b3c800(
    void* actual_helper, std::size_t helper_bytes,
    const volatile std::uint32_t& near_00d7a2f0,
    const volatile std::uint32_t& far_00ce38b8,
    NativeCockpitConstructionBlock::Admission&&);
```

The raw name context, node constants, pool and camera/viewport providers come
from the prepared capability. The known CockpitCamera literal and native helper
stores belong to that later body. Near/far are references to actual current
cells, never snapshots taken by admission; their x87 materialization stays at
the native sites. The native routine still returns the actual helper address.
This host signature adds explicit context and is not the original caller ABI.

## Preparation and consuming the attempt

The caller first gives the block a stable owner outside the constructor's stack
lifetime: an existing service-owned fixed slot/arena or another already-published
persistent owner is sufficient. If an owning container needs an insertion or
allocation, perform it now. Do not leave ownership solely in a local smart
pointer that will delete the block when construction throws. The block does not
install a registry: all blocks borrow the one already-installed
`NativeViewportRegistryBinding`, which outlives every record and host borrow.

`admit` requires idle/empty/quiescent block storage. Set `preparing` before any
operation that can throw or allocate. Check exact camera resolver/registry
identity, the existing scene/lifetime runtime relationship, raw name mode, and
the same actual D7A24C cell. Preserve all existing CRT/table/pool/provider
validation; do not call renderer methods early to manufacture a successful
preflight or freeze their later results. Reserve scene then lifetime capacity;
admit record0 then record1. Both records must be unused. Return the capability
and mark `prepared` only after all four operations succeed.

Failure during preparation cancels acquired node credits and viewport admissions,
then forgets the cancelled viewport records (their views were never exposed).
It clears the borrowed registry pointer and returns the block to idle. No raw
helper/camera/viewport word, native allocation, native retain or release occurs.
Earlier vector capacity growth need not be undone. Destroying/cancelling an
unused prepared capability follows this same path. A token cannot outlive its
block or borrowed runtimes while active.

At constructor entry, validate the actual helper extent/alignment and the
capability's prepared state, active credits, current installed registry/resolver
and raw-domain invariants **before the first helper native store**. Invalid entry
leaves the caller capability unchanged. Then move it into a local active attempt
and mark the block executing before any native callback. The caller's token is
empty. The executing outer capability is not moved again; only its contained
tokens move at their specific callee boundaries. The constructor is a friend solely to consume these exact four tokens and
emplace the already-reserved companions; there is no public ambient next-token
slot and no new generic callback provider.

The caller must wire `camera_environment.pool_0108ffb0` to the same canonical
pool owner installed for B71930/B71350 and used by camera terminal return.
Admission does not prove that identity from the field's name or allocate a probe
slot; it is an explicit requirement on the concrete caller/provider construction.

After successful 41E870 and immediately before B71A80, emplace `camera_owner_`
over the already-captured actual raw slot using the scene credit. This late host
preparation means a preceding string-construction failure has no scene companion
to abandon. Call BI's raw camera overload with the same local header, prepared
raw constants and **record0 admission**. BI moves it into its own local token
before its native callbacks, checks exact installed resolver identity, and uses
admitted allocation before camera+180 publication.

After B71A80 succeeds, emplace `camera_reference_` with the lifetime credit and
`NativeCameraCompanionDisposal{&block, record_camera_retirement}`. Bind it before
the native helper+0C publication. The valid path performs no allocation and no
extra +04 retain. Existing live/count/profile/canonical checks remain; wrong
identity/profile or duplicate companion errors are outside the admitted native
equivalence domain, not permission to fake success or return a completed camera
as an unconstructed pool slot. No other producer may bind a competing companion
for this attempt's privately allocated camera during its construction.

At B3C8F8/B3C90F, call existing admitted viewport allocation with **record1
admission**, which moves that token before native callbacks and registers the
successfully constructed viewport before returning. Keep its returned actual
address as the native local replacement. Neither token release nor block
settlement adds a viewport decrement.

On function exit, the local capability cancels only still-unused reservations
and marks the block settled. It leaves cancelled/retired/live records in the
block for explicit host reset. It does not reset optional companions, call native
cleanup, return a pool slot, forget a published view, or free storage. Native
state/string/raw-allocation cleanup remains explicitly in the later B3C800 EH
projection, before the local host capability ends. The host phase enum is not a
second representation of the five native unwind states.

## Early rollback and surviving native objects

| Event | Native/caller duty and block state after the attempt settles |
| --- | --- |
| Raw camera allocation or local string construction fails | Keep BG helper-base/string/raw-slot obligations in order. No owner/reference was emplaced before string success; cancel unused credits and both records. |
| Owner preparation fails before B71A80 | Its existing catch ends typed preparation. No completed camera exists. Preserve the applicable local string and raw-slot obligations; a contract diagnostic is not a new native event. |
| B71A80 fails before first successful viewport publication | Existing camera cleanup leaves owner dead; lifetime reference is absent. First record is unused/cancelled. Preserve string cleanup, exact raw return and helper base cleanup. |
| B71A80 fails after successful first viewport publication | Existing camera cleanup does not release camera+180. Owner becomes dead, reference remains absent, **record0 stays live**, and only unused reservations cancel. Keep the whole block despite camera-slot return. Do not reread the ended camera to recover the viewport. |
| B71A80 succeeds, later helper work fails | Bound camera and its first viewport may survive. Keep companions and all live/retired records. Cancel only admissions never consumed. No implicit release of published helper+0C. |
| Replacement allocation/constructor fails | Existing viewport allocator frees only its exact failed raw allocation; record1 cancels. Preserve the actual surviving camera/viewports. |
| Replacement constructed, B71990 or later work fails | Record1 is already consumed. State0 has no automatic local viewport release: preserve it even if no camera owns it. |
| Constructor returns | Normal counts often retire record0 and leave record1 live, but callbacks/other references can change that. Track actual record phases and retirement notification, never assumed counts. |

Keep a dead owner companion until explicit host reset; its phase is host state
and can be checked without reading dead raw fields. Existing owner destruction
only removes abandoned prepared bindings; dead-owner destruction is harmless.
If a supported failure leaves native survivors, the block's persistent caller
owner must retain the block indefinitely until their **actual** terminal paths
occur. An orphaned native viewport can therefore pin this block indefinitely.
Diagnostic forced cleanup is a separate operation, not automatic reconstruction
of a missing native release.

## Retirement, reset, and helper ownership outside the block

`record_camera_retirement` is a fixed nonthrowing function in the block source.
It checks that the supplied reference is the block's emplaced reference, its
camera owner is the same companion and is dead, and notification is not repeated;
then it sets `camera_retired_`. It never destroys either optional, the block, or
a viewport. The existing terminal callback has already destroyed the camera,
returned the pool slot and unbound the lifetime interface. No raw fields are
read. Notification is allowed while another native call/constructor is still
active; it does not permit immediate reset of the executing block.

Common viewport destruction retires its exact managed identity, including flags0,
before typed lifetime end and possible free. The block needs no viewport callback
or new count: inspect each record's host-only `phase()`. Its record stays stable
even when the actual viewport moves into another camera or outlives this camera.
Current Storage keeps its actual key/view private. No orphan-pointer getter or
registry accessor change is required by this block: a live phase alone prohibits
reset. Do not copy a fixture's live-callback observation into production ownership
or recover that key through ended/recycled camera fields.

`reset_after_host_quiescence` is an explicit caller assertion, not a scheduler or
an invented borrow token. It requires settled state, no active attempt, absent
or retired camera reference, absent or dead owner, and both records in
unused/cancelled/retired states. **Live/reserved records prohibit reset.** The
caller also proves all semantic frame calls returned and all host view/cache
borrows ended. Reset destroys the retired reference before its dead owner,
forgets each cancelled/retired record through the same registry, clears the
retirement flag/registry pointer and returns to idle. It performs no native
release or allocator operation. A next attempt may then admit the same two
records; no generation counter is needed because no old capability or borrow
survives this boundary.

The block destructor accepts only idle with empty companions, unused records
and no borrowed registry; otherwise terminate before member destructors can
silently act on surviving companions. Cancelling a prepared token restores idle;
settled blocks require explicit reset even when they have no surviving natives.
The registry installation/runtime teardown remains after all blocks are reset.

Native raw rendering does not borrow `CameraViewport` wrappers. The semantic
`D3D9CameraFrameAccess::viewport()` cache can expose a host view indefinitely;
BH's exact source audit establishes no general cache-detachment protocol.
Keeping inert records does not permit reads through their dead native references.
Use the existing raw path's established no-host-borrow domain or supply an actual
semantic borrow-end/cache-detachment provider before reclaiming those records.
Arbitrary destructive callbacks during native viewport field reads still need
the separate renderer/device liveness proof; do not add retains or delay free.

Before B3C800, the caller separately preallocates stable storage for the existing
`NativeCockpitHelperOwner` and `NativeCockpitHelperReference`, a real D61854 table
binding and retirement context. It keeps actual 24h helper allocation ownership
explicit. Do **not** construct those companions yet: they require already-live
profile/count and unique reference binding. After successful B3C800 return,
placement-bind owner then reference without allocation/retain; publish the usable
helper interface only after both bindings succeed. On failed native construction,
those helper companions were never constructed; caller cleanup owns only the
applicable helper allocation/EH duties. It cannot free this construction block
on account of helper failure.

On later helper zero, its existing disposal may reclaim its separate retired
companions and native helper allocation. It never reclaims this block directly.
Queue-retained cameras, foreign-camera viewport ownership, late native failure
survivors and inert host views all have independent completion conditions. A
helper companion contract error after native success is not an allocation failure
and must not be reported as successful publication; the integration needs an
explicit fail-stop/diagnostic policy that preserves existing native ownership.

## Nested callbacks and current camera replacement

All registry/block mutations use the existing serialized owning-thread domain;
identity getters and registry internal mutation are not callback-reentrant.
For nested native construction inside an outer callback, prepare a **different
block and all four of its admissions before entering the outer native sequence**.
The callback captures that capability explicitly. No host malloc, vector growth,
owner-container insertion or fresh node credit reservation is added mid-sequence.
Finite nesting needs that many prepared blocks; unbounded demand is not admitted.
The one installed viewport registry and canonical scene/lifetime runtime are
shared as required by the real native identity domain. There is no nested registry
installation and no mutation of a shared environment to pass a next admission.

Callback changes to helper+0C are supported when they publish another valid
canonical live camera. Before each later camera operation use the actual freshly
loaded key, `GeneratedModelLifetimeRuntime::find_actual_node`, and an explicit
checked `NativeCameraReference` projection; then obtain that reference's owner.
Do not cast raw native storage to a companion or use this block's original owner
as a fallback. A missing/noncamera/retired binding is an explicit unsupported
current-input error. No new identity map is required. The replacement viewport
can thus belong to another block's camera while its view record stays in this
block. Retiring the original camera does not cancel or release that replacement.
The local replacement address remains the final-decrement receiver throughout.

Field mutation, real renderer-publication replacement, independently prepared
nested construction and unrelated release remain permitted. They are not
blanket-disabled to avoid the separate captured-native-owner liveness question.

## Ready implementation packet and remaining integration

First packet: implement only the new block header/source and its evidence, using
existing optional storage, credits and viewport registry. Include phase guards,
all-or-nothing host preparation, move/cancel behavior, persistent retirement
notification, explicit quiescent reset and friend declaration for the later body.
No raw native allocation/destruction helper, scene/lifetime/viewport registry
change, generic framework, source stub, Ghidra/ledger body credit or new test is
needed. The integrator adds the new source to existing CMake registration and
runs the normal Win32 build and applicable existing checks after implementation.

The later body packet owns `native_cockpit_helper_construction.hpp/.cpp`, B3C800
evidence/annotation and the integration points consuming these private tokens.
It must implement the complete BG native normal/EH order and inspect generated
x87 materialization. The block packet itself owns no native address lease.

Still unclosed: validated/published BI dependency at the exact source revision;
the B3C800 body itself; caller-owned persistent block and separate helper-companion
allocation/publication/disposal integration; actual provider/constant/table and
outer helper allocation wiring; checked current-camera projection in that body;
and semantic-view quiescence or renderer destructive-callback liveness wherever
those domains are exercised. Existing APIs alone prove none of these consumers.
No build, fixture, game run, original ABI or full FH3/SEH validation is claimed by
this document/report-only packet.

## Integration validation

BI dependency at exact combined source `0307b963f66a80caa7f9308cecbfc2d5d6b780da` passed the strict Win32 build, both existing CTests, eight seed checks and the current-library successful native/source and source-only failure fixture. See `reports/native_camera_viewport_admission_bi_validation.json`. This closes the pending BI validation dependency above; this BJ packet remains design only.
