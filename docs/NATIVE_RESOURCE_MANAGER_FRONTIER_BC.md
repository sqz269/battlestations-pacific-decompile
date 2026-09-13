# Resource manager frontier BC

Addresses: 004c1400, 00b81040, 00b80f10, 00b81150, 00b80720, 00b7f290.

The immediate independent packet is the 161-byte owned-name/cache-pair
constructor `00b7f290`. The raw resource-manager owner and complete load route
remain a dependency frontier. The existing `GameVfsHost` parser-map projection
is useful source behavior, but its reconstruction-ledger entry at `004c1400`
does not establish the original 28h-byte singleton or its cache lifetime.

This is read-only discovery at worker baseline
`99cdd7fdfd48ff167cf8b13a17d9752a40a8d4fc`. The companion
[report](../reports/native_resource_manager_frontier_bc.json) records six full
body spans, exact direct-call sites, ABIs, current source pins, and dependencies.
All 1,638 body bytes match both live `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, and the installed x86 PE with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The configured CLI verifies project/program before each live read batch.
All six flow reports show zero gaps. The direct-call verifier passes 61 rows:
57 calls within the bodies and four explicitly selected startup/load edges.
Indirect virtual and Win32 import calls are recorded separately.

## Existing implementation boundary

`include/bsp/game_hosts_init_tail.hpp:131-136` explicitly says the native 28h
manager is not reconstructed; `GameResourceManager` owns a
`StructuredResourceRegistry parsers` member. `GameVfsHost` at
`src/game_hosts_vfs.cpp:281-291` creates this object once per host and returns it
for both application parser registrations. That preserves a stable parser-map
identity without the actual singleton publication, native trees, factory or
cache layout.

`ResourceParserMap` already provides useful semantic key ownership and
registration behavior. Its header explicitly declares a new ABI, without native
iterators, allocation layout or exception state. The typed structured reader
and retained model-root traversal likewise exist. None should be cast into the
raw owner to claim completion of `00b80720`. Existing native particle-resource
consumers still expose manager/load callbacks, another explicit open boundary.

Live caller evidence confirms `004c1400` at application sites `0073db41` and
`0073db55`; `00717e80` also registers game parsers through this owner, and
`007188ab` obtains it for the game factory load route. Other live containing
callers include GUI page loading, settings loading, resource destruction and
session teardown. The xref capture is capped at 30 sites, not a total count.

The initial pointer `00b88260` is the resource **base constructor**, called by
`0071b810` and the default creator `00b88340`; it is not manager construction.
Root's `71xxxx` factory packet owns that route. This packet adds no duplicate
contract or source proposal there, or at root's `005efba0` lookup work.

## Six inspected contracts

| Address | Bytes | Original ABI | Role and readiness |
| --- | ---: | --- | --- |
| `004c1400` | 189 | No arguments; EAX owner; RET | Actual publication and locked lazy owner creation; current host entry is a projection |
| `00b81040` | 260 | ECX owner; EAX same owner; RET | Raw owner construction and six parser registrations; dependencies incomplete |
| `00b80f10` | 187 | ECX owner; RET | Default-factory and two-tree destruction; dependencies incomplete |
| `00b81150` | 30 | ECX owner, stack flags; EAX same owner; RET4 | Scalar deletion; depends on complete destructor |
| `00b80720` | 811 | ECX manager, stack name/factory; EAX resource; RET8 | Full cache hit/miss loader; broad raw dependencies remain |
| `00b7f290` | 161 | ECX pair, EDX resource, stack by-value name; EAX pair; RET8 | Owned-name/raw-resource pair; ready against existing concrete string services |

Names are descriptive hypotheses, not recovered symbols. The tree helpers'
historical `BSP_` names do not establish that their red-black-tree algorithms
are game code; preserve them as STL library contracts.

## Owner lifetime and layout

`004c1400` captures `010901c4` for the fast return. On a miss it obtains the
actual lifetime manager `00415350`, captures its critical-section pointer at
`+10h`, enters it, increments section `+18h`, then rechecks publication. It
allocates exactly 28h through `00bf681b`, calls `00b81040` if nonnull, publishes
the result, obtains the lifetime manager again, reloads publication and calls
`00bd0c30` even when the allocation result is null. It leaves the originally
captured section and reloads publication for the slow return.

| Owner field | Established contract |
| --- | --- |
| `+00` | Profile `00d63128`, slot 0 = `00b81150` |
| `+04` | Separately allocated 4h default-factory object, profile `00d63060` |
| `+08/+0c/+10` | Parser tree allocator word / head / count |
| `+14/+18/+1c` | Resource-cache tree allocator word / head / count |
| `+20` | Current factory, assigned by loader; untouched by constructor |
| `+24` | Current resource, assigned by loader; untouched by constructor |

The constructor also leaves the two tree allocator words themselves untouched.
For each sentinel it writes nil byte `+19h`, self links `+0/+4/+8`, and zero
count. Built-in parser getter order is `00b7e1d0` Mesh, `00b7e2a0` SkinedMesh,
`00b7e530` SkinedMeshAnimation, `00b7e370` MatrixIndexedMesh, `00b7e460` Camera,
then `00b7e100` GroupParams; each result reaches `00b80a50`. Registration return
values are ignored. These identities and order match the named
[registration evidence](RESOURCE_MANAGER_REGISTRATION.md); raw getters and
registration are not closed by the semantic map implementation.

The default factory's verified slots are `00b7d290`, `00b88340`, `00b7d9a0`.
The first and third currently lack Ghidra function entries. The middle creator
calls the resource base constructor. Keep this default factory distinct from
the game factory under root's `71xxxx` work.

Destruction invokes current default-factory slot 0 with flag 1, clears the
resource tree, frees its current sentinel and zeros head/count, then does the
same for the parser tree. Finally it clears `010901c4` unconditionally and
writes base profile `00ce3818`. It does not explicitly release `+20/+24`.
The established [cache ownership](RESOURCE_CACHE_OWNERSHIP.md) is owned keys and
nodes with borrowed resource pointers. Scalar deletion frees the captured
owner only for `flags & 1`. A future raw lifetime binding uses the owner pointer
without adjustment. Complete owner FH3 cleanup schedules still need review;
this report claims normal-path body recovery, not owner exception closure.

## Immediate packet: native_resource_cache_pair_bc

Own only `00b7f290`, with new files
`include/bsp/native_resource_cache_pair.hpp`,
`src/native_resource_cache_pair.cpp`, `docs/NATIVE_RESOURCE_CACHE_PAIR.md`, and
`reports/native_resource_cache_pair.json`. Add only its own source registration
and address ledger records during implementation. This file/address set is
independent of the root factory and lookup packets.

The function captures incoming length/data and EDX resource before clearing
the two destination name words. It compares the destination with the native
by-value input-header address, resizes using captured length, and copies from
captured data using current destination fields. Then it stores the captured
resource at pair `+8`. No resource reference is added or released. Normal input
cleanup returns the captured buffer with captured length plus one, with DWORD
wrap, through the current pool getter and return helper.

The EH evidence is `00cc2051 -> 00dfb1c8`, with two entries at `00dfb1b8`:

| State/action | Cleanup contract |
| --- | --- |
| 1 / `00cc2030` | Destroy the current by-value input header at EBP+4 through `0041dd20` |
| 0 / `00cc2038` | Check and clear completed-pair flag; only when set destroy the saved destination name through `00b7e8f0` |

The completed flag is set only after the resource-pointer store, and state 0
is selected before normal input cleanup. Thus initial resize/copy failure
cleans the input but does not add a new destination rollback. Failure during
normal input-buffer return cleans the completed destination. Normal cleanup's
captured fields and unwind cleanup's current headers must remain distinct.
`00b7e8f0` uses the same current-header pool-return schedule as the existing
raw `0041dd20` helper; it need not become a new algorithm or callback stub.

Use the existing `NativeStringRawPoolContext` overloads of `0041dd40` and
`0041dd20`, backed by actual `00419cc0/00bd1120/00bd1510`. This avoids narrowing
getter exceptions through `NativeStringStorage::release noexcept`. Preserve
the existing BF7680 overlap policy. The boundary is an actual 0Ch pair plus
input-header identity and concrete pool publications, not an STL tree port.
Template provenance remains uncertain; this recommendation does not authorize
generic `std::pair` or container implementation.

## Deferred owner and load integration

Reserve a later `native_resource_manager_owner_bc` packet for the four owner
bodies (666 bytes), with distinct `native_resource_manager_owner.hpp/.cpp`
and matching documentation/report. It is **not source-ready as a closed
owner** until the six raw parser singletons, `00b80a50`, default factory
selectors, STL storage contracts, and owner unwind cleanup are established.
Do not fill these with callbacks that do nothing or advertise a constructed
empty owner as original startup behavior.

`00b80720` first writes current factory, then looks up the original requested
name. A cache hit publishes and retains the found resource, and leaves factory
`+20` set. A miss calls the metric service at `0109cefc` virtual `+4`, invokes
current factory virtual `+4`, resolves a separate VFS name and opens flags 2,
constructs an actual reader/root and dispatches against the actual manager.
Its cache insertion still uses the original requested name. Only the normal
miss tail clears `+20`; it writes the metric difference at current resource
`+40`, captures the resource, then destroys root, reader and name temporaries.
Metric meaning remains unknown.

The load dependency list includes raw name-tree lookup/insertion, VFS resolved
name/open, `00bea150`, `00bf0430`, `00bea700`, `00b7f430`, `00be9ed0`, and
`00be9f10`. Existing typed implementations remain useful but do not prove
compatible actual storage. Assembly was required because current pseudocode
has unaffiliated register inputs and incorrect stack string expressions,
including an unreachable warning at `00b80986` despite a complete listing.

No source, ledger, Ghidra analysis, installed files, tests or game state changed.
Local CLI captures, decoded bytes, source pins and generation helpers remain
in the worker `local/` directory for integrator retention. Implementation should
use strict Win32 build and existing checks first; any additional fixture must
target a concrete cleanup/current-field risk and preserve the distinction from
native FH3 ABI or gameplay proof.
