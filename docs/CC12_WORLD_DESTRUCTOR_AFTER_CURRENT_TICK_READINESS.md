# World destructor readiness after the current-storage tick pair

Read-only packet `cc12_world_destructor_after_current_tick_readiness`, address
`00904C40`. The complete ordinary body is **522 bytes / 182 encoded
instructions / 21 physical calls**, through the plain `RET` at `00904E49`.
Fresh target-verified Ghidra bytes equal the installed PE for the whole span.
The current listing still supplies only **156 starts**, with **26 starts /
82 bytes missing**. No listing repair or Source destructor is supplied here.

The current-storage tick candidate at `b0d645f6b2dbc8dad866032a5c652e7a92d32b0c`
supplies concrete `00904BF0` and `00904600` C++ entry points. Its Root build and
whole emitted-body acceptance remain separate. Even if that candidate is
accepted, it closes only the matrix-call dependency below: **the World
destructor and callable four-slot World table are still unready**.

## Exact ordinary schedule

Native entry has ECX=actual World, no stacked argument, and ordinary plain
`RET`; EBX/ESI/EDI and the FS exception chain are restored. It installs handler
`00CA54CF`, retains the World pointer, publishes base table `00CE7784`, clears
actual `World+4AC`, and clears shared byte `00E0AF20`. An original-image table
number is not a callable Source table. The exception handler and unwind graph
are not recovered or admitted by this ordinary-body inspection.

| Phase | Exact behavior and observable reloads |
| --- | --- |
| `904C73..904C9A` | Dereference header `World+8` without a null guard, walk its actual first entity, call `926D90(entity,7)`, then read that entity's `+44` after the call. Run `874D00(CL=0)`. |
| `904C9B..904CCE` | Read the current header `World+4`. For each entity, call `922FD0` only when byte `+5E != 0`, DWORD `+6C == 0`, and parent `+3C` is null or its byte `+5E == 0`. Read entity `+38` after the possible call. Run `874D00(CL=0)` again. |
| `904CCF..904D81` | Three explicit repetitions. Each starts at the current `[[World+4]]`, calls each active entity's current vslot `+DC` with an x87-produced stack `+0.0f`, and then reads entity `+38`. Each walk is followed by `904600(World,+0.0f)` and `874D00(CL=0)`, including empty walks. Five extra-step calls and three matrix calls occur in total. |
| `904D82..904DA7` | If nonnull, free the separate `World+8` header through `BF65AC`, then clear the current field after return. Repeat for `World+4`. These frees do not free the entity payloads. |
| `904DA8..904DCD` | Capture `World+4A8`; if nonnull, call imported `InterlockedDecrement` on captured owner `+4`. On returned zero, load its current table and invoke slot0 with ECX=owner and no stacked argument. Clear `World+4A8` only after the release sequence returns. No post-call owner read occurs. |
| `904DCE..904E26` | Capture current matrix sentinel's next; set its next to itself, reload `World+4B4`, set that sentinel's previous to itself, compare saved next to the current sentinel, and zero count `World+4B8`. For each old node capture next before `BF65AC`, then compare that saved next against current `World+4B4`. Finally free the current sentinel unconditionally and clear `World+4B4`. |
| `904E27..904E49` | Change EH state to0, call `BF7C6E(World+18,12,97,4C2D30)` for reverse category-header cleanup, clear root `World+C` with `4BF8E0`, restore the FS chain/registers, and return. The normal body does not free the World allocation. |

The entity callbacks must leave each visited entity readable for the Native
post-call link load. Matrix-node free has the opposite capture rule: its next
is saved **before** freeing the node. Preserve the current-header and current-
sentinel reloads; no saved snapshot, iteration cap, skipped empty pass or
generic vector teardown establishes this schedule. The World and the required
headers/sentinel must already have completed their actual construction. The
early dereferences make this body unsuitable for base-only constructor failure.

## Current Source dependencies

| Boundary | Genuine current Source and remaining condition |
| --- | --- |
| `926D90` | `native_pending_entity_kill_00926d90` has the actual recursive body, current sibling reloads, canonical list-node helpers, and captured-lock unwind. It still requires the actual pending owners, actual field resolver, lock-owner getter and callable child/entity virtual providers. The existing projectile facade borrows those services; it is not a complete World entity owner. Raw cause7 is required even though the method stores cause2. |
| `922FD0` | **Missing actual-storage body.** `scene_node_kill_00922fd0(SceneNodeFlags&)` sets only three projected booleans. The known complete Native 66-byte body also writes `+6C`, recurses over current children and dispatches current entity vslot `+84`. Its ledger label alone does not establish those operations in Source. |
| `874D00`, five calls | **Missing qualified actual shutdown service.** `GameFixedStepHost::run_extra_fixed_step_00874d00` reads cached `last_world_active_`, whereas the Native gate reads the current Game/World path and World byte `+4AC`. This destructor clears that byte before the first call. The CL=0 queued/think/deferred/queue/tail operations still need their actual services; closing the world gate does not turn the extra-step call into a no-op. No whole-callee sweep was performed. |
| `904600`, three calls | The pinned tick candidate has an actual-storage body and callable matrix/subtree/free operations. Use the same borrowed clock cell and writer/reset protocol, constants, returning validation service, actual entity tables and actual matrix records. The destructor's zero argument is passed even though the matrix entry leaves it unused. The candidate does not produce those lifetimes or a World table. |
| `World+4`, `World+8` | `allocate_native_world_chain_headers_009037f0` provides separate actual 12-byte headers in the `singleton_lifetime_allocate/free` domain. Its existence does not qualify the entities, their shutdown progress, or arbitrary foreign allocations. The final header frees must pair with their real producer domain. |
| `World+4A8` | The real Win32 `NativeGameLifetimeCalls::interlocked_decrement` and concrete `NativeGameCurrentVirtualCalls::virtual_terminal` provide the operations. The latter fresh-loads current slot0, uses a no-argument thiscall and performs no extra decrement or post-call receiver read. The owner, reference count, callable table, terminal method and allocation domain remain external qualifications. |
| Matrix list `World+4B0/+4B4/+4B8` | The actual 6Ch sentinel producer and paired `singleton_lifetime_free` exist. Pending tick can free expired actual record nodes. Normal destructor list teardown still needs its own exact reset/free sequence for remaining nodes and sentinel, with producer-domain and coherence qualifications. A sentinel producer alone does not prove record production. |
| 97 category headers plus root | Existing `clear_native_world_parent_headers` reverses all97 real headers through `4C2D30` and then clears the root; `4C2D30` reaches the actual raw `4BF8E0` clear. This is reusable ordinary cleanup for coherent, initialized headers with canonical nodes and borrowed payloads. Its documented constructor-cleanup, noexcept and DF0 contract does not supply the whole destructor's EH state machine. |
| Shared byte `E0AF20` | The current search finds a projected `MissionSceneLoadState::scene_resident` and an abstract unit-damage reader, not a demonstrated common actual-cell binding. A future interface must borrow the real shared byte; a new private flag is insufficient. |
| World table and scalar wrapper | `CE7784` has exactly four entries: `4CB0B0`, `904390`, `9035D0`, `904BF0`; adjacent `CE7794` is another profile. Current Source supplies the constant-byte leaf and actual `9041A0` chain-drain primitive; the tick candidate supplies the last method body. The context delivery and receiver/ABI adapters, `904390` wrapper, normal destructor, and outer scalar flags/free path still need to be real before table publication. |

The existing canonical allocator/free pair and current-table dispatch are real
operations. They do not authorize mixing allocations between providers or
treating `NativeGameLifetimeCalls::free_00bf65ac` (`operator delete`) as a
universal disposer for arbitrary borrowed storage. The World allocation's
scalar free belongs to `4CB0B0`; it is absent from the normal body audited here.

## Listing and support-evidence holds

| Missing current listing span | Encoded instructions | Bytes |
| --- | ---: | ---: |
| `[00904D8F,00904D95)` | 2 | 6 |
| `[00904DA2,00904DA8)` | 2 | 6 |
| `[00904DF8,00904E05)` | 4 | 13 |
| `[00904E11,00904E4A)` | 18 | 57 |

The exact bytes contain ordinary fallthrough, cleanup and return. They agree
with the existing listing-repair design. This does not establish the cause of
the listing gaps: direct flow/fallthrough overrides remain unverified. No
no-return change, script/endpoint retry, listing import, function recreation,
shared-free mutation or repair was attempted. Listing repair is its own held
work item; it cannot supply the missing Source services.

Held supplement `0166b106e` describes cookie14 plus two padding bytes and
stack43 plus five padding bytes. The current parent review reports support
bodies16/48. This packet preserves that **14/43 versus16/48 discrepancy** and
does not reclassify padding or accept the held supplement as current support
closure. Those bodies were not independently revalidated here. No direct
cookie/check-stack call occurs among this destructor's21 physical calls; the
held graph is not authority to repair this listing or implement its services.

## Minimum next packet and limits

The next bounded packet is a **read-only actual-storage contract audit of
`00922FD0..00923011` (66 bytes)**: current child-link reload order, `+6C` and
flag writes, recursion, current vslot84 ABI, receiver lifetime, and whether an
existing real Source provider can be reused. Claim only that address and its
dedicated outputs. Its known footprint and the concrete projected-Source gap
justify the scope; this is not permission to reconstruct it before its whole
contract is known. The actual CL=0 extra-step service is a separate later packet.

No new Source, build registration, test, probe, Ghidra mutation, runtime run,
Original-ABI credit, full-World readiness or application admission is claimed.
The [report](../reports/cc12_world_destructor_after_current_tick_readiness.json)
pins the fresh522-byte evidence, exact call/gap rows, Source inputs, pending
tick commit and held supplement independently of any later Root integration.
