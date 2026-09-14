# Neighboring native-model pool readiness BG

Addresses: 00CD7F00, 00B74B80, 00CE0E50. Evidence only; zero new native bodies.

The neighboring static pool is already implemented and integrated. Reuse
`NativeModelPool` from `include/bsp/native_model_pool.hpp` and
`src/native_model_pool.cpp`; do not open another body-implementation packet for
these addresses. The remaining work is application ownership, startup and exit
wiring, plus proof of the original CRT walker and invocation order.

## Current evidence and chronology correction

Worker base: `5dc8942a112555d25dde817af2bc7c6bbdd736db`. Latest model-base
bootstrap documentation was read from the parent integration worktree at
`daa9706c969f965ceb70790ccd2ef68f68ca0b1b`, without changing its files.
The neighbor provider was introduced by
`7372836d92c03f93cf01c0e53a860dcafe9a465e` and integrated by
`4ff70e294b22fdd8a9318e974a8bae137f48eaad`. Worker and parent copies have
identical SHA-256 hashes, recorded in the companion report. CMake already
registers `src/native_model_pool.cpp` in `bsp_core`.

The current sharded reconstruction records for all three owned entries say
`actual_storage_integrated_strict_win32_build_and_original_native_differential_fixture_checked`.
The integrated audit is `reports/native_render_storage_integration_audit.json`;
`reports/native_model_pool_audit.json` preserves the earlier worker audit.
Its absent-CD7F00/pending-integration statements are historical: live Ghidra
now has the complete function. Do not erase that original evidence chronology.

Append this correction to the parent bootstrap readiness doc/report and its
implementation doc: **CD7F00/B74B80/CE0E50 are complete existing NativeModelPool
providers, not outstanding body reconstruction. Reuse canonical 01090054 with
profile D62DD0 and the application's shared AllocatorListDomain. CRT walker,
invocation order and application startup/lifetime wiring remain unverified.**
In particular, the readiness report's remaining string "neighbor
CD7F00/B74B80/CE0E50 not reconstructed" has been superseded by this recheck.

All live questions used `bsp.py ghidra`, whose `client()` calls
`Client.verify()` for every command. Config selects project `bsp`,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 little-endian Win32,
image base 00400000 and bridge `http://127.0.0.1:8089`. Discovery also confirmed
that program on the live bridge. No Ghidra mutation or export was performed.

## Complete owned entries and exact caller contracts

| Existing native name / C++ provider | Inclusive body | Coverage | Original ABI |
| --- | --- | --- | --- |
| BSP_NativeModelPool_StaticInitialize / initialize_static_native_model_pool_00cd7f00 | CD7F00..CD7F15 | complete existing body, rechecked | no stack arguments; RET; EAX is atexit result |
| BSP_NativeModelPool_Construct / NativeModelPool::initialize_00b74b80 | B74B80..B74C52 | complete existing body, normal listing rechecked; EH audit reused | ECX actual pool, EAX same pool, RET |
| BSP_NativeModelPool_StaticDestroy / destroy_static_native_model_pool_00ce0e50 | CE0E50..CE0E59 | complete existing body, rechecked | no stack arguments; selects ECX then tail JMP |

Names are existing descriptive hypotheses. CD7F00's current pseudocode returns
`void`; the listing proves EAX is retained from BF6FF5, and the source adapter
correctly returns `int`. CD7F00 selects ECX=01090054, calls B74B80 at CD7F05,
pushes CE0E50 at CD7F0A, calls BF6FF5 at CD7F0F, uses POP ECX at CD7F14 to
remove that one argument, and returns at CD7F15. It has no local guard or
rollback. Failure to register leaves the constructed pool in place.

The complete xref set is small: CD7F00 has data reference CE3570; B74B80 has
one direct call CD7F05; CE0E50 has the pushed callback reference CD7F0A.
No alternative call-site arguments were found. CE0E50 explicitly overwrites
ECX with 01090054, then CE0E55 tail-jumps to existing B74690. That provider's
only current caller is CE0E50. Data references do not establish who invokes
the initializer or callback at runtime.

Live bytes at CE356C, CE3570 and CE3574 contain CD7EB0, CD7F00 and CD7F20.
This proves membership and relative storage order only. This packet has not
identified the table walker, its bounds, direction, phase or actual execution
order; it makes no startup-order or process-exit scheduling claim.

## Producer layout and concrete provider distinction

B74B80 writes the actual 38h owner: D7A0C0 base profile, previous +04=0,
next +08=current E188B4, old head's +04=this when present, then E188B4=this.
It switches +00 to D62DD0 before InitializeCriticalSection(this+0C), clears
recursion +24, table +28, count +2C and capacity +30, then writes first-free
+34=FFFFFFFF. It publishes capacity 32 before calling BF55BE with 80h bytes
at B74C02; ADD ESP,4 at B74C09 proves one argument. After copying any live
entries and optional old-table free at B74C34 (ADD ESP,4 at B74C39), it
publishes the replacement at B74C3C and returns this in EAX at B74C44.
Allocation can reenter the shared allocator list; source binds concrete
D62DD0/B74C60 trim dispatch before this native publication.

ESI receives incoming ECX at B74B98 and is restored only after EAX=ESI.
The complete listing shows EBX is zeroed once at B74B9A and then only read
until its saved value is popped; its zero meaning is not inferred from one
isolated instruction. EDI first holds this+0C, then the replacement from EAX
after new; the final store uses that latter value. There are no x87 operations.

`NativeModelPoolStorage` reconciles those producer offsets with compile-time
Win32 layout assertions. The existing companion owns no second list or table.
Its 188h-slot/3144h-slab/slot+184 pool-ID contract is already documented and
implemented; those allocation bodies were not independently re-audited here.
This is a distinct storage object from model-base's 0109008C generic node
pool with D62C78/B6E980/B6E3D0, and from the plain-node 0108FF58 pool.
Adjacency and the shared 38h owner shape do not make their types interchangeable.

Existing B74690 restores D62DD0, frees every slab then its table, drains only
positive signed recursion through LeaveCriticalSection(this+0C), deletes the
section, stamps D7A0C0 and unlinks the same E188B4 element. It preserves stale
fields and invokes no model destructor. Its complete listing includes both
returning free continuations, ending at B74719. NativeModelPool's default C++
destructor does not perform this explicit lifecycle operation.

## Reuse and remaining packet

The application must supply one stable NativeModelPoolStorage, one stable
NativeModelPool companion and the same AllocatorListDomain used by other
native pools. Construct the companion to bind trim dispatch before publishing
the pool; bind it with `bind_static_native_model_pool_01090054`; call the
existing static initializer once. The storage, domain and companion must all
outlive the registered callback. The binding rejects a different companion;
it does not create storage or another allocator-list head. Manual destruction
after successful registration would leave a later callback targeting destroyed
native state, so lifecycle wiring must select one coherent shutdown path.

The existing new/free providers remain CRT contracts, not new host stubs:
BF55BE tail-jumps BF681B, BF6989 tail-jumps BF65AC, and source uses the existing
`singleton_lifetime_allocate/free` boundary. BF6FF5 calls __onexit and computes
0 or -1; the source default is `std::atexit`. Win32 section imports are retained
as library contracts. The constructor's three EH leaves are covered by the
existing audit and source `__finally`; native EH execution is not revalidated.

A source search of the parent's `src`, `include/bsp`, `tests`, CMake and
GAME_EXECUTABLE host table found the static bind/start adapter only in its
declarations/definitions, not an application call. The module being compiled
does not make this lifecycle reachable in bsp_game. Existing mesh/probe
allocator domains are not proof of the application's canonical pool owner.

Next bounded packet: **model startup/lifetime wiring readiness**. Reuse these
providers and the new model-base bootstrap. First identify the concrete
application owner/domain and the original CRT caller/walker plus the lifetime
of model consumers; then assign explicit application files and recovered
walker addresses for wiring. This packet does not yet authorize an arbitrary
startup order or shutdown registration order. There is no missing function
definition among the three owned entries, and no additional native body is
ready or necessary for the neighbor itself.

## Verification limits

The report carries exact call-site/native rows and current body ownership.
`verify_report_calls.py` passed 10 direct call/tail-jump rows with zero failures;
three imported Win32 calls remain explicitly
indirect and are inspected from the listing rather than claimed as direct edges.
Historical integrated evidence records the strict Win32 build with 2 tests and
the focused native storage/lifetime fixture passing. That fixture injects
atexit result 17 and executes the registered callback; it does not validate
real process-exit scheduling. No build, fixture, game run or new test was
needed or performed for these documentation-only changes. Exported and
reconstructed status is current; the prior build/fixture results are retained
evidence, not new measurements. New C++ interfaces are not binary ABI drop-ins;
native EH, startup order, complete rendering and gameplay remain unvalidated.
