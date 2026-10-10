# CC12 particle model profile callee ownership readiness

Source-only verdict: **not ready for forwarding C++**. The paired
`bind_particle_profile` / `retire_failed_particle_profile` family has complete
existing companion and cleanup providers. Its first missing step is persistent
application-owned per-model preparation/admission in the existing renderer
ownership domain, before AF74A0 enters the nonthrowing profile hook.

Capture: `8d818952c2faca638f206bd1ba372c327ede6e85`. Accepted C++ baseline: `99b170231e937da304703fe01221bcc8b8b8aef7`.
Separate actual Git/current-file verification also checked main `ee48bf2ecaf95e0af38197ab135ea01d2ec996ae`:
`src`, `include`, `cmake`, `scripts` and `CMakeLists.txt` root entries are identical.

## Actual owner and storage contract

`GameStartupHost::native_renderer_` owns `GameNativeRendererApplication::Impl`.
That Impl already owns `NativeRenderActualOwnerRegistry owners` and its
`DestructionGraph`: one `ActualSceneRuntime scenes`, one
`GeneratedModelLifetimeRuntime attachments{scenes}`, and one
`NativeNodeDestructionRuntime nodes` borrowing those same objects and actual raw
strings. The Impl passes the SAME actual-owner registry to its resource graphs.
These are existing application domains; do not introduce private registries,
scene runtimes, native counts or model pools to make a wrapper callable.

The whole retained Source query finds no concrete production
`NativeModelEnvironment` initializer, particle lifetime-access initializer or
construction-callee implementation. The existing renderer node graph does not
by itself establish a model environment. Current base table/constants, a model
type wrapper over the existing canonical type words, parameter-name storage and
particle lifetime views remain explicit composition requirements.
`GameNativeTypeStorage` already initializes model type words through
`ModelTypeBootstrap`. D5DA50/F8D308/F8D2C8 particle views are required bindings,
not copied defaults or new IDs inferred by this packet.

`NativeModelOwner` prepares one aligned actual slot of at least 188h bytes. It
saves/restores the 184h node/model prefix while establishing typed lifetimes and
one external scene association; preparation neither reads nor increments the
native reference count. Its prepared destructor forgets that scene association.
A live owner must complete explicit native destruction before companion disposal.

A particle allocation is 2E0h with 2DCh payload from F8D2D0. The derived tail is
+184..+2DB, so +184 is a derived float. Its physical slab ID is +2DC. The distinct
188h `NativeModelPool` has its ID at +184: its B75290 deleting path must never
handle this derived object. The accepted pool-borrow packet is retained whole
by committed blob, document and report. Its existing F8D2D0 owner remains the
allocation/return owner; no pool accessor or implementation change is proposed.

## Binding and successful lifetime

AF74A0 constructs B75030 on the SAME owner, publishes D5DA50/early derived fields,
and calls noexcept `bind_particle_profile`. The later try block covers variant
preparation and subsequent construction, not that hook. Emitted Source code
stores D5DA50 at section +CD, calls virtual+4 at +10B, then sets try state at +111.

The existing `NativeParticleModelReference` constructor requires live owner
phase, a positive SAME native+04, actual particle pool, D5DA50 table and disposal
callback. It checks table slots 0/4, binds itself to the existing generated
lifetime runtime, and replaces callbacks on the SAME scene attachment. Context
stays `NativeModelOwner*`, because base destruction reinstalls base callbacks.
It neither initializes nor retains native+04. Scene callbacks resolve this same
companion through the existing runtime.

That constructor does **not** insert a canonical actual-owner registry entry.
The future persistent host record must bind the actual raw slot to this exact
companion in the existing registry, without an extra retain. A stack companion,
`NativeModelReference`, second scene association or no-op callback is insufficient.

Successful zero-reference retirement validates the current derived profile,
marks the companion destroying, performs AF6C50 and returns the actual particle
slot. AF6C50 releases current arrays, emitters, variant and meshes through the
same actual-owner domain; it captures the current manager before count decrement
and unregister, then completes member/base destruction. The companion subsequently
unbinds generated lifetime metadata, marks itself retired and invokes disposal.
Disposal must unbind the SAME canonical registry entry and dispose host companions
without reading returned native bytes. All borrowed services must survive
callbacks. Since physical return precedes metadata retirement, the owner must
also serialize slot reuse; the registry mutex alone does not prove native-object
lifetime or concurrent slot safety.

## Constructor failure

AF74A0's catch performs emitter-pointer member cleanup and B750C0 base destruction,
then calls noexcept `retire_failed_particle_profile` and rethrows. It does not
run successful derived destruction, decrement the model count or return the raw
slot there. Preserve this even for late failure after partial successful work;
adding broad rollback would change current behavior.

`retire_after_failed_construction` requires a bound companion and DEAD base owner.
It unbinds generated lifetime metadata, marks itself retired and invokes the SAME
disposal callback. That callback must support both successful pool-return
retirement and this **no-pool-return** path. Registry unbind checks the exact
identity/reference without reading destroyed native+04. Base destruction already
forgot the scene association.

The outer `RegisteredModelEffect` catch calls AF62F0 exactly once after model
construction unwinds. Failed-profile cleanup must not decrement references, call
AF7F40/B75290, return the slot or repeat base destruction. Before-profile failures
need appropriate prepared/base-owner cleanup, without pretending a companion was
bound. No successful-model cleanup is added to later effect/parenting failures.

## Precise readiness hold

Both hooks are noexcept. The existing companion constructor may throw validation
errors or allocate during ordinary generated-lifetime vector binding. Canonical
registry binding also validates live+04 and uses allocating `unordered_map::emplace`.
A direct noexcept wrapper can terminate after native base construction; swallowing
failure leaves required dispatch absent. Neither is a completed integration.

Scene and generated-lifetime `BindingAdmission` APIs already provide host-only
credits and consuming bind overloads, with canonical identity checks and a
serialized owning-thread contract. The particle companion calls the ordinary
bind with no admission argument. Holding a credit makes that ordinary bind
reserve an additional slot to preserve outstanding credits; it does not consume
the reservation. The actual-owner registry currently has no equivalent prepared
insertion/admission API.

The minimum next bounded work is a persistent application record that prepares
its owner and borrowed environment/access/disposal dependencies before AF74A0,
proves the same slot/scene/registry identities, and permits a nonallocating,
nonthrowing derived publication after D5DA50 becomes live. Solve admission at
that actual owner boundary: premature registry binding of unconstructed native+04
is not a shortcut. Preserve cleanup of abandoned preparation, real base failure
and post-bind failure. This packet specifies no new state machine or storage API.

Models must finish before manager weak-array teardown, renderer scene/registry
destruction and particle-pool CRT cleanup. The existing host drains raw singletons
while renderer/VFS bindings live, then resets the renderer and later VFS/random
owners. Its retention gate covers current renderer phases, shaders, shadow and
layout acquisitions, with no particle profile-record state. `_Exit` retention is
not normal model-retirement proof. Future activation requires actual application
lifetime/failure integration.

The other methods (`bind_mesh`, `bind_material`, `bind_section`,
`renderer_virtual5c`, `call_00bd2f10`) remain separate contracts. This packet neither
claims the full construction class ready nor changes the renderer family's hold.

## Retained evidence and verification limits

All 3356 epoch pins (3352 project inputs plus four build artifacts) matched frozen
Source3352 and actual Root bytes before retention. The selected project readset
also has 3356 paths, a separate count from that epoch pin count. Fifteen actual
compiler TUs contribute 582 tracked dependency paths. The whole tracked query
corpus has 4207 C/C++/`.inc` paths; their union retains 4214 complete committed,
Root and worker files. Seven worker files differ only by CRLF/LF. The 303 external
compiler paths are enumerated only; SDK/toolchain preimages are not retained.

All 56 compiler read/write/command/item logs, full 1991-entry archive (1988 COFF
members), all 80 actual App objects and current App project list are retained.
The earlier Root metadata's 70 App records are a reviewed subset; this packet
creates no previous snapshots for the other ten objects.

Fifteen complete object indices retain 8410 sections, 4711 code sections and
295815 code bytes. Twenty declared-storage sections with raw pointer zero have
empty payload. Twenty-seven complete Source function sections decode to 9208
bytes / 2653 instructions. These are not total class/interface sizes or original
function-size claims.

The full Core/App symbol census has zero definitions or named code relocations
for either hook or a construction-callee implementation. Each companion
constructor/failure-retirement provider has one definition and two substring
relocations; those two are its own EH handler/funcinfo labels. There are zero
direct relocations to either provider in this corpus. EH labels do not prove
application activation. Twelve whole-corpus Source queries and 38 exact excerpts
supply complementary evidence; negative conclusions remain bounded to this corpus.

Complete compiled checks include the 363-byte companion constructor's +108 call
to ordinary generated bind; ordinary/credit generated binds (50/188 bytes);
ordinary/credit scene binds (72/134 bytes); the 118-byte failed retirement without
native destructor/pool-return relocation; and the 231-byte zero-reference path's
destruction, raw-return, runtime-unbind and disposal ordering. Complete supporting
owner, registry, scene, caller and application sections are retained as well.

One copied ZIP/helper replays Source pins, retained Git object/tree/blob identity,
actual compiler closure, full COFF census/indices, complete disassembly, queries,
excerpts and explicit call/store invariants. After dependency imports, its audit
guard permits reads only from the supplied ZIP, rejecting outside opens, writes,
subprocesses, network, dynamic loading and directory scans. A physically copied
ZIP/helper gives identical results. Actual Git/current-file verification is a
separate operation. Old Root proof trees are not recursively followed.

- ZIP: 42237988 bytes, SHA-256 `e54ab065958f38b25ab71195a2006c932372a26fba8378fbaa9259d42ae9d7d7`.
- Helper: 25135 bytes, SHA-256 `47b56b26d4dcf91b90659673969936efbb3990d727eace53927e257740483f1b`.
- ZIP payloads: 4420; checked unique pins: 4415.
- Local proof: `local/cc12_particle_model_profile_callee_owner_readiness/`.
- Machine report: `reports/cc12_particle_model_profile_callee_owner_readiness.json`.

No C++/build-system edits, compiler invocation, tests, target/Native execution,
Ghidra queries/mutations or model activation occurred. Prior accepted build/test
artifacts are retained evidence only. Address suffixes describe existing Source
contracts; original register/SEH ABI, live rendering and gameplay were not
revalidated. The reviewable result is the ownership/admission hold and existing
success/failure providers, with no ready-forward or executable-ABI claim.
