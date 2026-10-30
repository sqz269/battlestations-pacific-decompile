# Native Game application provider and custody readiness (CC16)

This read-only source audit is pinned to published Main
`42185e9219bd4e5f771410e1072e69ebd2bd54b1`. **No complete, genuine Game
application aggregate is ready to invoke or attach at this revision.** The
existing normal constructor and storage destructor are substantially supplied;
the missing edge is actual provider, publication, receiver and retained-lifetime
composition. Writing another current-table forwarding wrapper would not close it.

The smallest independently closed preparation is the real initialization-mode
name epoch in the existing `GameStartupHost`: actual `mode` -> existing
`duplicate_native_string_00438e40` -> one retained E1AE78 authority -> the existing
Game application frame's temporary name -> matching free/current-cell clear.
This supplies a genuine required input, but does not make Game construction,
shutdown, physics attachment or gameplay ready. The private qualified-engine
candidate `5c320a1f5b508a59862db6d8d055c2878c0a643a` was inspected only for its
additive header contract; it is not Main421, adopted or validated by this audit.

Only this document and its [machine-readable report](../reports/cc16_native_game_application_binding_readiness.json)
are changed. No C++, CMake, guard, ledger, Ghidra, peer tree or installation is
changed. No build, test, probe, game, executable or DLL was run. There is zero
new body, vendor, ABI, activation or gameplay credit. Descriptive names remain
source hypotheses.

## Actual application edge

`GameStartupHost::application_construct()` already constructs its genuine 1Ch
`native_application_[7]` through `construct_native_application_00737970`, using
the canonical process E1AE90 and AA0 cells. Application+14 is word5 of this
storage; `ApplicationFrameState` and `GameStateSlot` are separate projections.
No new Application allocation is necessary.

A complete `src`/`include` symbol search finds runtime/frame/attachment matches
in only their six source/header files. There is no application construction of
`GameNativeGameRuntime` or `GameNativeGameApplicationFrame`, and no call to
`GameFixedStepHost::attach_native_game`. Its null member still records the
physics row at 875E0C and returns. The real Dyn process started by `game_main`
does not change that result.

The existing frame allocates exactly 71A0h in the canonical malloc/new-handler
domain, begins the byte shell's lifetime, clears the whole allocation, captures
E1AE78, builds the actual 8h pooled-string header, invokes 4DDB90, then publishes
Application+14 and returns the temporary string. The runtime owns its operations
and one-slot callable scalar table; it borrows Game storage and every context.
Its report table comes from the canonical Dyn process. That one slot does not
supply Game entry, state, mission, pause or other original-table methods.

The runtime checks identical Game/three-grid publication references, identical
engine/world context references, and the same physics lifetime call service. It
does not validate all other references or authenticate arbitrary children. Only
a completed frame and `live` runtime may be attached, with runtime storage,
actual E188A8 and Application+14 all identifying the same Game. E188A8 is
published at 4DE105 **before** engine and World construction, so publication
alone is not construction success.

## Reached construction services

All 26 effective `NativeGameConstructionCalls` methods have source defaults.
They are not 26 remaining abstract bodies, and no production instance is selected.

| Reached methods | Actual source provider / custody |
| --- | --- |
| `array_construct_00bf7cd1`, `construct_empty_strings_00bf7cd1` | Exact three callback/stride families and real raw element/string constructors. Four retained array operations; no invented STL owner or original CRT/FH3 unwind. |
| `call_004c2700/2750/27a0/2830/26b0`, `call_004c1950/1a40` | Concrete raw tree/list allocation defaults in `native_game_storage_defaults.cpp`, through canonical `allocate_00bf681b`. They preserve untouched allocator/payload bytes. |
| `allocate_00bf681b`, `allocate_00bf55be`, `free_00bf6989` | Source malloc/new-handler domain and its matching frees; inherited defaults are real services, not application ownership. |
| `call_007ff9d0`, `call_008882d0` | Real race prefix and 14h raw mission-Lua host. The latter creates a 2Ch sentinel and writes numeric D0E7A8; it does not create a callable host deletion table. |
| `call_0076ede0`, `call_00bd1860` | Concrete embedded-state/peer initialization and tracked Win32 section. Borrow live constants; retain partial allocation/lock custody on failure. |
| `call_008d9150`, `call_00727bd0` | Concrete unit append/rank builders over `GameNativeGameTables`. Its actual storage exists, but no application retains this owner or supplies its required opaque input. |
| `call_00432650`, `call_0087d7b0` | Real GlobalConfig getter and complete normal Lua loader on the same 2E8h owner. No application GlobalConfig context, F878E4 authority or effects admission is composed. |
| `call_00717e80` | Real five-parser registration over the VFS application's existing manager and parser contexts; borrow those exact contexts, not projected parser registrations. |
| `call_0070bd70` | Real 84h grid constructor over the existing renderer's grid context. Three retained operations and three distinct explicit descriptor-preimage pairs are required. |
| `call_00be4800`, `call_00c55f50`, `call_00c420e0`, `call_00c31a40` | Actual processor environment helper, legacy normal engine ensure, real 48Ch Dyn World creation, and exact World+24 callback-owner store. The current default does not call the private qualified ensure candidate. |

The profile constructor is called directly at Game+650. Its 16 named service
methods and inherited collection services have concrete defaults.
`GameNativePlayerProfileContext` is a real retained provider, currently
definitions-only. It borrows the exact VFS string wrapper, initialized BCh
settings, mapped CE3A0C/CEF794/CEF15C, and `settings.profile_context()`. That
process context contains the actual shared E188A8/F8ABE8 cells and a genuine SDK
provider. `SoundServices` binds its live XLive library before this native Game
position; missing SDK custody throws rather than returning synthetic success.
The projected title/menu profile is not the F8h Game+650 owner. Teardown still
requires a raw profile-lifetime context with the Game lifetime call service.

## Private bytes: provenance and actual reads

These inputs are neither loader-zero globals nor fields of the zeroed Game.

| Input | Producer and reached observations | Current binding consequence |
| --- | --- | --- |
| Unit `unit_frame_word_preimage` | 8D9150 reserves 10h private stack. At 8D9186 it writes only BYTE `[ESP+10h]`; 8D9195/99 read/store the whole DWORD into row+C. Its upper 24 bits are preexisting private-frame bytes. The recognized converter at 8D9750 uses `CMP BYTE [row+C]` (8D9851; also the inverse branch), not those upper bytes. | `GameNativeGameTables` preserves the upper 24 bits in all 23 rows. There is no application producer for that argument. The byte-only recognized consumer is not a whole-program absence proof or a recovered value. No zero/PE constant is inferred. |
| Three grid pairs `word20/word24` | 70BD70 reserves 50h private stack. Its descriptor is based at original local ESP+1Ch; the two words receive no stores before 70B330. 709C90 explicitly executes FLD/FSTP at descriptor+20/+24, copying to Grid+5C/+60 and potentially quieting signaling NaNs/changing x87 status. Six CPU allocations follow before `grid_steps` overwrites +60 then +5C. | The canonical renderer grid provider owns real default CPU allocation calls; they receive a byte count, not a grid pointer, and do not explicitly read those fields. Nevertheless the x87 reads and interim bytes are real, including allowed alias/handler observation. Geometry later reads the overwritten steps. No application producer or selected preimage policy exists; clearing 71A0h or 84h does not clear this private descriptor frame. |

The native evidence above is from bounded **saved exports**, not a new live
Ghidra/installed-PE comparison. The report pins their complete triples and
checks saved project `bsp` / program `/battlestationspacific.exe`. Config still
selects existing `C:/Users/sqz269/bsp.gpr`; no analysis mutation occurred.

## Genuine available context domains, and the unclosed receivers

The application already retains the canonical AA0/AA8/AA4 string/manager domain;
settings E188A8 and profile refs; actual VFS resource-manager/five-parser
contexts; renderer declarations, geometry registry, pools, device recreation
and `game_grid_context()`; native Lua bootstrap/files and activation binding;
and the initialized settings+34 FOV reference. `GameNativeReadOnlyData` supplies
the constructor/array/embedded constant references. It is read-only adopted
image storage, not writable publication authority.

GlobalConfig is an identifiable **aggregate-composition** gap. Its getter,
constructor/destructor/loader, raw manager deletion branch for CE3D98 and
`NativeGlobalConfigSampleDispatch` already exist. A retained owner would need
one actual F878E4 cell; `singletons.sound_lifetime()`; the exact VFS wrapper;
`sound.core.samples()` as component lifetime; its real cache context and
`core.lifetime_bindings().global_00f8bbe8`; the existing sample dispatch;
Lua bootstrap/files; the settings FOV reference; and the SAME context installed
in `native_deletion_bindings().global_configuration` before registration.
No application presently creates this aggregate or installs that binding.

That aggregate is not fully closed merely by filling references. The first
three effect slots admit canonical Source samples for final release. Source
D5B074 samples have no valid stop slot. The second three slots require genuine
raw callable stop+8/final-release receivers; no nonnull producer is attributed.
The real ObjectiveSounds loader has no three-entry bound, so input/override
output admission matters: additional rows can reach that second array. Neither
an empty array assumption nor calling original numeric table entries closes
this domain. Current dispatch is explicitly nonthrowing and domain-qualified.

`NativeGameCurrentVirtualCalls` is a concrete, historically build-reviewed
service. It owns no receiver, table, heap or publication. Source D0E7A8 Lua hosts
and CFD484 grids contain **numeric identities**, so they cannot be handed to
its generic current-table path as if callable tables had been produced.
Grid normal destruction/scalar bodies and the real renderer context already
exist; eventual lifetime composition must admit the exact constructed grid
allocations and route their real source retirement, preserving parent capture
and post-callback clear order. This audit proposes no extra forwarding class.

The raw host's scalar 00888380 / destructor 00888300 / queue 00887D90 chain has no
complete raw production lifetime composition. The existing mission-Lua host,
queue and deferred payload APIs use `std::string`/`std::vector` projections.
Earlier exact raw vector/payload audits do not promote those APIs. The newly
admitted raw 8844F0 machine-release helper requires genuine current callable
machine storage and does not supply the host scalar path. In particular, its
Game OnDestroy call has no host-null guard; a fake null host is not an input.

Lifetime additionally needs the actual E18678/E1867C/E19900/E18DB0/E19698/
E1930C/E198BC/movie cells; E18E6C/E18D80/F89B34 singleton cells; E187BC registry
and six 10h vector headers at E1875C; E0AF14 byte; and two actual 10h Lua headers
at108FF30/40. Current application matches are contracts/projections, not a
canonical composed set of those raw authorities. The Lua process's genuine
0Ch region object is not those two 10h headers. New arbitrary zero cells or
casts of projected managers cannot establish producer/lifetime identity.

Arrays, embedded state, profile, singletons, Lua globals, class cleanup and
physics must all borrow the SAME lifetime call object. Array observers come
from `singletons.observers().lifetime()`, and resources must share the exact
AA0/AA8/AA4 cells. Defaults enforce these identities at reached calls. Those
checks do not produce missing raw children or validate foreign allocation.

## Dyn is a real process provider, not a constructed Game

`game_main` explicitly starts CC8950 then CC89C0. The canonical Dyn process
owns engine/profile publication cells, dispatch/static/SAP2/task/solver/report
tables, actual scalar `::operator new/delete` memory, and convex-pool bindings
over the shared E188B4 allocator list and actual CRT conversion cell. Accessors
require completed startup and reject rebinding. `dynamics()`, `world_step()`,
`contact_reports()` and `body_creation()` are genuine reusable suppliers.

Engine/profile/World are still made by the Game constructor's reached calls.
On Main421 `call_00c55f50` uses the old normal-only ensure. The process's
`profile_partial_node_delete_00bf65ac()` is declaration/definition-only, guarded
to its actual default scalar-new domain. It has no caller. The private C55
candidate adds named qualified APIs requiring that genuine cleanup provider;
its presence supplies no Game caller or adoption. Captured9C/358/14 storage,
Root 48h and current-vector cleanup scopes, and the late volatile count must
remain qualified when that source is later selected. A nonnull delete function
does not authenticate a foreign allocator/context, and C++ catches do not
establish native fault/FH3/SEH or emitted ABI equivalence.

Physics lifetime must borrow these same engine/world/scene memory contexts and
the exact body-creation binding for convex retirement. Ordinary policy remains
48Ch Dyn World -> E8h scene -> 248h SAP2; unused SAPRadix families do not replace
it. Native terrain/convex and report runtimes are available; application
entity/body/shape/hull production on this new World is not thereby supplied.
The separate 4BCh mission World at Game+19CC and its host service contracts
must not be confused with the Dyn World at Game+18.

## Shutdown and failed-operation custody

The real normal application order is full OnDestroy 4DC5C0, scalar 4DE270(flags1),
then clear current Application+14. The frame's scalar method calls storage
destruction; it neither calls OnDestroy nor clears Application+14. Current
ordinary startup/shutdown selects none of this new Game graph.

Main421 includes the reviewed complete normal OnDestroy parent and reviewed
raw 4DA650/8844F0 helpers. That is source/build/link admission, not a selected
`NativeGameOnDestroyCalls` implementation. Raw4DA650 still requires whole
4D95F0 and its real frontend publications/lifetime; 4DA780 remains a screen-level
projection; 46F3E0 and992AC0 have host contracts without a concrete application
binding of the required actual cells/receivers. The OnDestroy online F8A2FC/
F8A300 system domain is distinct from the settings/profile F8ABE8 manager.

All borrowed storage, tables, code, SDKs and call services must survive Game
destruction, shared manager drain and any retained failed operation. Current
host destruction keeps sound alive through the raw drain, then releases Sound
before deleting the singleton host; native Lua services die after scripts and
before VFS. Process cells surviving atexit do not extend those wrappers' lives.
Game-specific retention must be checked before those existing release points.

Frame/runtime/parent/child operations are one-shot. Source failure retains the
partial graph and forbids replay; several destructors terminate if an active or
failed operation is abandoned. Grid/global-config/array/profile/embedded/
singleton/class/physics children must be resolved before parent diagnostic
acknowledgement. Scalar entry marks the outer allocation indeterminate because
a qualified child may free and then throw. Flags1 success frees once; flags0
retains storage for later explicit free. Diagnostic acknowledgement performs no
native cleanup, lock release, rollback, publication repair or lifetime proof.
Normal C++ cleanup, native terminal failure and complete FH3 remain distinct.
Retaining Lua backing does not restore interpreter invariants after an
unprotected foreign C++ escape through Lua C frames; diagnostic close/release
can execute finalizers and requires independently valid state and live services.

## Smallest next work that is justified

No new complete Game aggregate, call-service forwarding layer, or native body
is declared ready by this audit. The following are bounded gates, rather than
permission to construct a subset Game:

1. **Closed source preparation:** compose the real mode-name epoch in the
   existing host. At 73D93C call existing raw 438E40 with the actual `mode`; retain
   its returned canonical allocation in the sole E1AE78 authority at 73D945.
   The existing Game frame borrows that exact cell/string domain. After the
   actual parent reaches 73E466, free the captured current duplicate through
   matching canonical free and clear the current cell at 73E46E. Interrupted
   initialization retains custody; do not add a catch-time invented rollback.
   This requires no new native body, table, guessed private bytes or replacement
   Application. Until the full graph is admissible, it is preparation only.
2. **First constructor-composition gate:** establish an explicit justified
   source/provenance policy for the unit word and three grid pairs, then close
   the concrete GlobalConfig aggregate's sample/second-array input domain.
   Reuse the existing table/profile owners and real cache/Lua/settings services.
   Another unit/table wrapper or fabricated constants would add no provider.
3. **First lifetime gate:** qualify retirement of the exact raw grid and Lua
   host allocations produced by4DDB90, plus real mutable-global/header producers.
   Generic callable-table glue and the admitted raw teardown parents do not
   establish these receivers. Raw Lua vector/payload bodies remain dependencies,
   not a newly source-ready full host here; this audit does not reopen all EH.

Only after these gates and the genuine whole OnDestroy dependencies close is a
retained complete application aggregate justified. It must own contexts,
authorities and operations, borrow the established providers, reuse the real
frame, and remain in custody through success or terminal failure. Root can then
review the exact native init/shutdown hook points before enabling attachment.

## Evidence and validation boundary

The report pins all inspected current source contracts, selected saved export
triples, current routing receipts, and the explicit private header blob. It
rechecks available prior source pins and build-log hashes without running those
builds again. The OnDestroy and teardown-helper receipts retain their historical
Win32 build/two-suite/link-only evidence; current-dispatch and grid receipts have
their separately recorded historical validation. They are not fresh Game
composition, runtime, ABI, post-update or gameplay proof. No vendor CRT/zlib
body was reconstructed or granted credit.

All 2 OnDestroy and 10 teardown-helper current source pins and both recorded
build-log hashes match. The older current-dispatch source receipt has 5/6 raw
source pins matching; `native_game_lifetime.hpp` differs by the two later
free-domain comment lines from `93b7653af59cd8ae4337f07527e5a05a50a153ed`.
This drift is recorded, not silently treated as a fresh unchanged-input build.
