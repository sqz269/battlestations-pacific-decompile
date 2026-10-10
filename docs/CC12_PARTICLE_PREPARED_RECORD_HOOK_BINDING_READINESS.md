# CC12 particle prepared-record hook binding readiness

**GO for one dormant borrowed hook adapter. NO-GO for a concrete particle factory,
an application-owned persistent factory record, or application activation.**
The new `NativeParticleModelPreparedRecord` already implements the two host
operations required by AF74A0. A narrow adapter can route those operations and
forward the five remaining methods through a required borrowed callee. That
callee remains an unresolved dependency: current Source supplies no concrete
implementation of either particle construction interface, no invocation of the
record, and no custodian that retains it through Native retirement.

This is a Source-only decision. It grants no compiled, runtime, Native ABI,
factory-completion or shutdown evidence. The preceding record implementation is
commit `6b7ce80ef081a796a40e529c52782f77531112cf`; this packet captures worker
`1e7ec2fb2b9c9159876ddb9915cba2deafccff15` after merging Main `8cf81aaf5`.
At capture, Root reported a fresh merged-record build with three existing checks
passing while complete primary compiled review remained pending. That status is
preserved separately from this packet's Source findings and earns no artifact
credit here.

## Existing frontier and exact hook contract

The actual Source call frontier is in
`src/registered_model_effect.cpp:53`: allocate through
`allocate_model_slot_00af6b60`, pass that exact result and the selected variant to
`construct_model_00af74a0`, and call `return_model_slot_00af62f0` only if that
construction call throws. The interface requires a real F8D2D0 allocation, its
2E0h stride and intact +2DC pool ID, the same returned identity, complete Native
member unwind on throw, and a canonical scene binding before normal return.
This is the pre-Native admission frontier. It is not a new allocation hook.

A future concrete construction callee must arrange stable host storage there:
prepare its sole `NativeModelOwner`, construct its prepared record, and finish
both metadata admissions before entering `construct_native_particle_model_00af74a0`.
Passing a pointer and 2E0h does not establish pool provenance. The new record
borrows the actual slot and services; neither it nor an adapter can manufacture
an initialized pool, an allocation receipt, or live variant-name storage.

`NativeParticleModelConstructionCallees` declares exactly seven methods in
`include/bsp/native_particle_model_construction.hpp:76`:

| Method | Supported narrow adapter action |
| --- | --- |
| `bind_particle_profile(owner) noexcept` | Call the bound record's `publish_particle_profile(owner)`. |
| `retire_failed_particle_profile(owner) noexcept` | Call the same record's `retire_after_failed_construction(owner)`. |
| `bind_mesh(mesh) noexcept` | Forward the exact storage reference to the required borrowed callee. |
| `bind_material(material) noexcept` | Forward the exact storage reference to the required borrowed callee. |
| `bind_section(section) noexcept` | Forward the exact storage reference to the required borrowed callee. |
| `renderer_virtual5c(renderer, entry, first, bytes, descriptor)` | Forward all five captured values and return the same result. |
| `call_00bd2f10(random, stream, minimum, maximum)` | Forward the same random state, stream and scalar arguments; add no arithmetic or sampling. |

AF74A0 checks its prepared owner, shared actual-owner domain and shared name
domain, constructs B75030, stores D5DA50, initializes the relevant derived
fields, and invokes the profile hook **before** its following resource-body
`try`. The hook is nonthrowing. An invalid record/reference/profile or registry
mutex failure terminates across that boundary; it is not recoverable allocation
unwind. After later resource-body failure, AF74A0 destroys the emitter pointer
member and real B750C0 base, then invokes the failure hook with a dead owner.
An adapter must preserve these two positions and their exception boundaries.

The renderer call has another explicit capture boundary. `finish_mesh` reads
the current renderer and its current virtual5C entry before AF10B0 obtains the
descriptor. Forward those saved values; do not reload either publication or
virtual entry inside the adapter. The current call passes first=0 and
bytes=1000h. The adapter is not authority to replace another virtual entry with
B287C0 or to return a fabricated registered stream.

## Providers that exist, and authority they do not supply

| Existing Source provider | Positive evidence and remaining boundary |
| --- | --- |
| `GameNativeParticlePoolProcess` | `src/game_native_particle_pools.cpp:28` performs explicit once-only F8D2D0 initialization; `model_pool_00f8d2d0()` rejects a borrow before completed startup. `src/game_main.cpp:1052` actually invokes startup. The getter has only its declaration/definition in the finite corpus; no current particle factory borrows it. |
| AF6B60 and AF62F0 Source thunks | `src/native_particle_model_pool_allocate.cpp:158` forwards allocation/return to AF69E0/AF60B0 with the supplied pool. They establish executable Source operations, not a concrete `RegisteredModelEffectCallees` implementation or identity proof for an arbitrary incoming pointer. |
| Renderer canonical registry | `GameNativeRendererApplication::Impl` owns one registry and `actual_owners()` returns it at `src/game_native_renderer_application.cpp:550`. The record can require this exact registry through its existing owner's environment. A second private registry is not equivalent. |
| Actual string service | `GameNativeStringProcess::strings()` returns its existing `ActualNativeStringPoolStorage` over the same publication cells. Its interface documents the `noexcept` release boundary. This service exists; the missing work is a matching persistent model environment and all other borrowed domains, not creation of a substitute name pool. |
| Actual manager publication and deletion access | `GameNativeStringProcess` owns the permanent F8C274 cell. `GameSingletonHost::bind_particle_manager_domain` validates the mapped manager deletion profiles and stores the existing `NativeParticleModelManagerAccess` using that cell and actual lifetime publication. A bound cell/context is not a constructed model, a model reference, or a record custodian. |
| Model-options singleton body | `model_effect_options_singleton_0051f6b0` is a concrete existing Source body with required publication/lifetime inputs. It has no current Source callsite. Do not replace the event's required current getter read with a cached suppression flag. |
| Particle record and lifetime bodies | The record, AF74A0, AF7F40 and AF6C50 exist as bounded Source implementations through required dependencies. Their presence does not instantiate `NativeParticleModelConstructionAccess`, `NativeParticleModelLifetimeAccess`, a complete particle model environment, or concrete resource callees. |

The finite Source evidence finds 20 uses of the prepared-record class, all in
its existing lifetime header/source. Its publication and disposal methods each
have only a declaration and definition. AF74A0 likewise has only its declaration
and definition. Each original particle profile hook has only the interface
declaration and AF74A0 invocation. Neither `RegisteredModelEffectCallees` nor
`NativeParticleModelConstructionCallees` has a matched concrete subclass.

The 15 `NativeModelEnvironment` occurrences are declarations, borrowed fields
and function signatures across the retained bounded set; none assembles this
particle path. The 15 particle lifetime-access occurrences and nine particle
construction-access occurrences are confined to their owning header/source.
The current live-count and type-view fields are borrowed contracts and reached
reads/writes, not supplied application storage. The CD7850 name occurs only in
a removed comment, so it has no retained lexical Source implementation match.
That is a future evidence question, not permission to initialize arbitrary type
IDs, copy mapped data into a new type domain, or add a zero counter here.

`GameNativeParticleRuntime` is an existing parser/resource-loader composition.
Its inputs and public API do not supply the missing model environment, profile
hooks, persistent model records or quiescent record disposal. Likewise the
existing post-effect20 construction block owns its distinct model/reference
fields and explicit host-quiescence protocol; its existence is not authority to
store F8D2D0 particle models in the ordinary 188h model path.

These negative observations are finite lexical findings, checked against full
retained preimages and bounded declarations/call bodies. They do not prove an
absence in arbitrary macro expansion, another checkout, or a running program.

## Stable custody, failure, and quiescent disposal

The prepared record must have a stable address before Native construction and
remain alive through all references, callbacks and borrowers. It borrows the
sole owner, lifetime access, canonical registry, and Generated runtime. Its
constructor validates the saved raw identity, owner phase, scene key/context,
canonical scene lookup and registry/scene agreement, then reserves the Generated
credit and raw-key registry admission. Preparation reads no typed Native +04
count. The actual profile-word validation is not a count load from the slot.

Publication creates the admitted reference in inline storage, then immediately
publishes that exact reference through the pre-existing registry token. No
Native call, external callback or yield may be inserted between those operations.
The scene binding's context remains the `NativeModelOwner`; an adapter must not
replace it with itself, the record, or another owner. The record's own disposal
context remains its stable address.

| Outcome | Required custody and cleanup |
| --- | --- |
| Owner/record admission fails before AF74A0 | Cancel only prepared host metadata, end the prepared/dead owner as applicable, and let the existing outer AF62F0 allocation catch return the raw slot exactly once. |
| AF74A0 rejects before profile publication or B75030 fails | A future caller must distinguish its still-prepared or dead base from a published record. Cancel unused host admissions after the construction frame has unwound; do not invoke the derived failure hook or scalar destructor speculatively. |
| Profile hook fails its nonthrowing contract | Terminal invariant failure; no invented recoverable raw cleanup, retry, or alternate registration. |
| Later AF74A0 resource body throws | Real emitter/base unwind precedes the failure hook. Existing reference retirement removes Generated lifetime; the record callback unbinds the saved opaque registry key and marks retirement. Outer event construction then performs its existing raw return. |
| AF74A0 returns, then event transformation/parenting/registration throws | Preserve the completed model, creator reference and persistent record. The event's state-0 unwind does not destroy the completed model. A local automatic record would terminate during destruction and is not a successful factory implementation. |
| Successful last Native reference reaches zero | AF7F40 performs derived/base destruction and physical slot return before Generated unbind and the record callback. Callback work uses only saved opaque identity and host state; no Native slot read, second return or active-reference reset. |
| Record is retired and every borrower has returned | A real external custodian may explicitly call `dispose_retired_at_quiescence()`, then destroy host storage. Retirement alone does not prove quiescence. |

An adapter that is used only during one synchronous AF74A0 invocation need not
outlive the resulting model: neither the existing reference nor the prepared
record retains that adapter. Its borrowed record does need to persist. Keep the
adapter and delegated services alive throughout all nested construction hooks;
keep the owner, record, scenes, Generated runtime, registry, pool, actual names,
manager/type/profile views and reached terminal providers alive through their
last borrower. This is a caller contract, not a discovered application owner.

The record must not be deleted/reset from its own retirement callback or while
the Native terminal, construction frame, failure hook or a resolved-reference
borrow remains active. Raw-slot reuse must be serialized until the old
canonical registry entry has been removed. The registry mutex only protects
metadata; it does not establish object quiescence or prevent physical slot reuse
after the successful terminal returns the slot.

AF0950 appends a weak raw model pointer without retaining it. AF0B90 releases its
critical-section reference and frees the manager's two pointer arrays; neither
operation drains model references. Existing renderer/singleton shutdown checks
renderer resources, worker completion and their own companions, not these new
particle records. Do not attach automatic record destruction to manager-array
cleanup or describe current application shutdown as a particle-client drain.

## Minimum next packet

The supported next C++ packet is **`cc12_particle_prepared_record_hook_adapter`**:

- Own only `include/bsp/native_particle_model_lifetime.hpp`,
  `src/native_particle_model_lifetime.cpp`, its new document/report and ignored
  local proof directory. Use the existing translation unit; no CMake change.
- Add one nonmoving, noncopyable dormant adapter derived from the existing
  `NativeParticleModelConstructionCallees`. Borrow one prepared record and one
  required `NativeParticleModelConstructionCallees` delegate; introduce no new
  resource interface, registry, scene runtime, type domain or allocator.
- Route the two profile hooks directly to that record and forward the other
  five methods exactly as the table above specifies. The delegate's profile
  methods are not used by this adapter. The delegate must still be real and
  valid for every reached resource method; no default/no-op implementation.
- Preserve `noexcept` on the original five nonthrowing hooks. Do not catch or
  translate failures, refetch a renderer, alter a pointer, count or random
  sample, create an ordinary `NativeModelReference`, or retain/delete anything.
- Construct no adapter in application code, call no Native constructor, own no
  record storage, and add no raw-return or automatic disposal policy. The record
  keeps enforcing exact owner identity and lifecycle. Keep all existing method
  bodies unchanged and separately review new compiled forwarding bodies.

This packet needs **no new Native address ownership**. Its dependency contracts
are AF74A0's profile/failure positions; AF6B60/AF62F0's outer raw boundary; and
the already reconstructed AF7F40/AF6C50 retirement sequence. It is a dormant
interface composition only. The required delegate remains explicit and absent
from current Source; calling the adapter a complete concrete particle callee
would be incorrect.

Do not schedule a persistent factory-owner or application packet from this
readiness result. Such a packet first needs a concrete same-domain resource
callee, a correctly initialized model environment/lifetime access, real custody
for both successful models and failed acquired frames, and a client/callback
drain. The mesh/material/section hooks are post-construction and nonthrowing;
ordinary allocating registry binds cannot simply be inserted there. B287C0's
optional identity output follows construction and precedes renderer-array
registration, so it is not raw pre-Native admission for stream owners. Those
remaining resource contracts are unchanged by a two-hook adapter.

## Source and portable evidence

The capture retains all 4,226 tracked paths under `src`, `include`, `cmake`,
`scripts`, `tests` and root `CMakeLists.txt`, plus three epoch metadata files and
eight prerequisite documents/reports. Ten actual Git tree objects, the actual
commit preimage and complete tree listing establish the selected coverage.
All 4,237 actual Git blobs and current files are checked separately against the
repository, and the final commit must preserve all six captured Source roots.

The finite query corpus contains 4,209 complete C/C++/header/inline files.
The evidence retains every match of 47 lexical queries, 45 bounded excerpts and
12 lexical ordering checks. Comments are removed without shifting line numbers;
string literals remain. These searches are not preprocessing, AST analysis,
compiled reachability, callback-exclusion or Native control-flow proof.

The earlier typed-view 3,352, wide-string 3,352, admitted-particle 3,354 and
accepted-registry 3,354 project epochs remain separate. Whole original manifest
preimages and every project pin are retained and checked; current differences
are enumerated as 13, 11, six and four paths. Original build declarations receive
no artifact credit. The complete prepared-record worker Source epoch contains
4,235 rows and differs here only in the two independently integrated grid files.

The report pins a copied ZIP, portable helper, full Source/Git capture and both
in-place and physically relocated replay results. After standard-module preload,
the guard permits only the supplied ZIP to be opened and blocks other opens,
writes, subprocesses, networking, dynamic-library access and directory listing.
Replay checks all member pins and recomputes Git/Source coverage, historical
epochs, queries, excerpts and ordering results. It executes no retained Native
code and reads no repository. Separate live Git checks establish the final
committed Source equality; the portable replay does not pretend to perform them.

No C++, CMake, ledger or Ghidra changes, fresh Native queries/exports, builds,
tests, probes, game execution or OS integration were performed for this packet.
