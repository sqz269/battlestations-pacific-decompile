# Persistent cockpit construction block implementation

Addresses: none (host preparation, storage and retirement bookkeeping).
Native context is B3C800 and its existing camera/viewport providers; no native
body, original ABI or FH3/SEH credit is assigned here.

Implemented the reviewed `NATIVE_COCKPIT_CONSTRUCTION_BLOCK_BJ.md` design in
`include/bsp/native_cockpit_construction_block.hpp` and
`src/native_cockpit_construction_block.cpp`. Worker integration base is
`eba2b75696b382f216b23f22d49ca22236d7a00f`: design `7b2c2a53` plus the explicitly
requested BI revision `20ace135`. The block packet changes no shared registry,
native body, ledger, design document, CMake registration or test.

The parent subsequently reported BI validation/publication at `9442ed64`, with
combined tested source `0307b963`: full build, two existing CTests, eight seeds,
coupled camera admission fixtures and independent review passed. Those are
dependency results, not runtime verification of this new block. This worker
does not merge the moving main branch while editing its source.

## Concrete storage and API

The block embeds exactly two stable `NativeViewportRegistry::Storage` records,
optional in-place `NativeCameraOwner`/`NativeCameraReference` companions, one
borrowed registry pointer, a host phase and a retirement notification bit.
It is noncopyable/nonmovable. It owns no actual 24h helper, raw camera/viewport
allocation, identity map, native refcount or helper companion.

`admit(environment, raw_constants, registry)` returns a move-only nested
`Admission`. It owns one scene credit, one lifetime credit and two viewport
admissions, with borrowed block/environment/constant pointers. The caller owns
the block persistently before invoking `admit`, and all referenced providers
outlive the attempt and any resulting camera/view lifetime.

Preparation checks reuse the concrete existing contracts: raw name mode;
the same actual D7A24C cell for node/viewport; the same scene runtime for node
lifetime dispatch; a real node type predicate; exact viewport resolver identity;
and actual CRT/table pointers. `NativeViewportRegistry::admit` establishes the
installed-registry check. No renderer callback, native allocator, pool probe,
constant-value snapshot or new pool identity mechanism is called. The caller
must wire the same actual 0108FFB0 owner for static allocation/raw return and
environment terminal return; a reference's name cannot prove that identity.

## Phase, token and retirement behavior

| Operation | Actual behavior |
| --- | --- |
| Prepare | Require idle/empty storage; mark preparing before validation/allocation; reserve scene then lifetime capacity; admit first then replacement record; mark prepared only after all four succeed. |
| Failed or cancelled preparation | Cancel acquired credits/admissions, forget only unused/cancelled unexposed records, clear registry pointer and return idle. No companion was emplaced. |
| Move prepared admission | Transfer exact pointers/tokens; source becomes empty. Assignment first cancels its destination's old prepared attempt. Self-move is a no-op. Moving an executing outer capability terminates; only its contained tokens move then. |
| Entry validation | Check prepared phase, nonempty node credits, current raw/provider invariants and both exact installed viewport admissions before the constructor moves the caller token or performs native helper stores. |
| Begin execution | After the one local move, switch prepared to executing. The constructor is the only friend able to do this and consume private reservations. |
| End executing admission | Cancel unused reservations and mark settled. Preserve optional companions and live, retired or cancelled records. Do not reset, release, return native storage or perform native EH cleanup. |
| Camera terminal notification | After existing native destruction/pool return/lifetime unbind, verify exact optional reference/owner identity and dead owner phase, then set the host retirement bit. No raw field read or disposal occurs. |
| Explicit quiescent reset | Require settled state; absent/retired reference; absent/dead owner; no live/reserved record. After the caller has ended every host view/cache borrow, destroy retired reference before dead owner, forget eligible records and return idle. |
| Block destruction | Require idle with empty companions, unused records and no registry pointer. Otherwise terminate before automatic member teardown can act on surviving companions. |

These guards inspect only host companions/record phases. In particular, an
orphaned live first viewport after late camera failure prevents reset without
recovering its private key or reading ended camera+180. Its block can remain
pinned indefinitely until actual native destruction occurs. Reset does not
invent an orphan release, delay native free or change native counts.

## Exact constructor handoff

The new header declares, but does not define or stub, this C++ entry:

```cpp
void* construct_native_cockpit_helper_00b3c800(
    void* actual_helper, std::size_t helper_bytes,
    const volatile std::uint32_t& near_00d7a2f0,
    const volatile std::uint32_t& far_00ce38b8,
    const NativeCockpitViewportReleaseContext& viewport_release,
    NativeCockpitConstructionBlock::Admission&& admission);
```

`NativeCockpitViewportReleaseContext` is forward-declared only. Its definition
and validation belong to the separate constructor packet. The parent approved
this extra explicit parameter after review found that current CE2220 IAT and
D5E5F8 table inputs were absent from existing viewport environment APIs. The
block stores no release context and introduces no ambient next-admission slot.

The constructor friend validates its helper/release inputs and the prepared
admission before native effects, moves once to local
`attempt`, then calls `attempt.begin_execution()`. Its exact private inputs are
`block_`, `camera_environment_`, `node_constants_`, `scene_admission_`,
`lifetime_admission_`, and `viewport_admissions_[0/1]`.

The block's exact private storage names are `camera_owner_`,
`camera_reference_`, and `viewport_records_[0/1]`. Immediately before native
B71A80, after local string construction succeeds, the constructor emplaces the
owner with the scene admission over the actual saved slot. It forwards record0
to BI's raw camera constructor, then emplaces the reference with the lifetime
credit and `NativeCameraCompanionDisposal{&block, record_camera_retirement}`
before helper+0C publication. Record1 goes to the existing admitted replacement
viewport allocator. Native calls, x87 materialization, raw-slot ownership and
the exact native EH state transitions remain in the separate body packet.

The block never resolves a replacement helper+0C by using its original owner.
That later body must use fresh actual keys and checked canonical camera
projection. A viewport record can outlive this camera or belong to another
camera after callback replacement. Independent nested attempts require other
blocks and all reservations prepared before the outer native sequence starts.

Actual helper allocation and preallocated stable helper owner/reference storage
remain caller responsibilities. Bind those helper companions only after native
construction succeeds; helper failure/free does not authorize this block's
reclamation. The optional camera reference is not directly reset by that caller:
only actual retirement followed by explicit host quiescence permits reset.

## Evidence and verification

Relevant callee bodies were read before implementation: scene admission
`scene_attachment.cpp:270-336`, lifetime admission
`generated_model_lifetime.cpp:42-117`, viewport admission/forgetting
`native_viewport_registry.cpp:14-116`, raw-domain access
`native_node_destruction.cpp:68-81`, and existing camera preparation/retirement.
The current BI owner source matches the reviewed `5f2574ac` source exactly.
The report records these and new implementation anchors. No live Ghidra query
or mutation was needed for this host-only packet; it adds no native call rows.

Focused MSVC Win32 compilation of the new translation unit passed using C++17,
`/EHsc /MD /W4 /WX /permissive-`. It produced only an object in ignored local
storage, not a probe executable. No new test was added or run. The parent owns
source registration and the combined `scripts/build.ps1`/fixture validation;
this worker compile does not establish linking or runtime behavior.

Remaining limits: one serialized owning-thread domain; no allocator/identity
getter reentry during registry mutation; active tokens cannot outlive their
block/runtimes; native input/provider errors retain their existing boundaries;
and semantic host-view/cache quiescence is a caller-proven condition. Stable
inert view storage does not make dead native fields readable or prove arbitrary
destructive renderer callbacks safe. B3C800, outer provider wiring, original
binary ABI, unrestricted native EH and game validation remain outside this
implementation packet.


## BJ combined integration validation

Exact source `dd86c2152a814a352ee0e2f3c3b503fc678b0008` passed the strict MSVC Win32 build, both existing CTests, eight seed spans and the current-library actual constructor fixture. One full mapped-original/source B3C800 constructor-through-helper-terminal pair at x87CW027F: 12 live checkpoints, 13392 camera bytes and 432 helper snapshot bytes per path, 12 ordered events, 14736 identical normalized snapshot bytes per path and 36 identical normalized helper bytes before free. The 9492 mapped code bytes remain unchanged after both paths and the failure observation. One source-only renderer-call2 failure verifies unregistered first-viewport cancellation/free, both local/camera raw-string returns, exact primed camera slot/pool return, helper base/count1/zero camera representation, absence of helper companions, settled block and one explicit host-quiescent reset before caller-owned helper free. No original FH3 exception path is executed. Independent source and fixture reviews found no issues within those domains. See `reports/native_cockpit_constructor_bj_validation.json` for exact source/library pins and sealed evidence. Original replacement ABI, unrestricted native FH3/SEH, arbitrary native write-trace parity and gameplay remain unvalidated.
