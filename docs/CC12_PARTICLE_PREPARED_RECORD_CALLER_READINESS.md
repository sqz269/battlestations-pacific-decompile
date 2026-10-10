# CC12 particle prepared-record caller readiness

The existing registered-effect interface exposes the particle raw slot before
AF74A0. That is a genuine preparation point for one persistent host record over
the existing `NativeModelOwner`, the admitted `NativeParticleModelReference`,
one `GeneratedModelLifetimeRuntime` credit and one preparation token from the
same canonical `NativeRenderActualOwnerRegistry`.

**GO for a separately accepted, particle-only host-record implementation at that
boundary. NO-GO for a complete construction adapter, factory wiring or application
activation.** The required concrete particle construction callees are absent in
the finite current Source corpus. Reached mesh, section and material hooks receive
identities after construction; the renderer stream factory exposes a completed
creator after construction. Those hooks do not currently provide a raw-key
preparation point. This packet implements no record, provider or hook.

## Scope and accepted epochs

Only this document and
`reports/cc12_particle_prepared_record_caller_readiness.json` are committed.
The ignored evidence directory is
`local/cc12_particle_prepared_record_caller_readiness`.
No C++, CMake, application binding, Native body, Ghidra state, ledger, build,
fixture, game process or operating-system integration was changed or executed.
Python work is limited to retaining, searching and replaying Source/Git evidence.

Current Source was frozen at worker commit
`bf2eb2954553f0f0f57a0dd12c2398ddc52170d8`: all 4,226 tracked paths below
`src`, `include`, `cmake`, `scripts`, `tests` and `CMakeLists.txt`; the lexical
query corpus contains all 4,209 C/C++/header/inline files in those Source roots.
Ten whole Git tree objects, the commit object, whole tree listing and every
selected committed/current preimage are retained. Three historical epoch
metadata paths and four prerequisite document/report paths are retained too.

The admitted particle constructor was already accepted on Main `295b982af`.
Registry commit `8579262d4` had Source approval but primary acceptance was pending
at the initial capture; that original statement remains immutable. Root later
accepted and pushed the registry at Main `98b5113614`. Its separate
`Source3354-render-prepared-entry` manifest is 1,243,633 bytes, SHA-256
`35a3f9ac2765d19554b7cc2e06dcb3b31ad2b6b93cb1b18005625721b7a2fdaa`.
All 3,354 whole project preimages match the frozen worker Source after line-ending
normalization. The earlier typed-view 3,352, wide-string 3,352 and admitted-particle
3,354 epochs remain separate: respectively 11, 9 and 2 current path differences,
enumerated in the report. Their four build declarations per epoch are retained
as historical metadata, with no artifact or test credit in this packet.

## Genuine raw-slot frontier

`src/registered_model_effect.cpp:53` obtains the raw slot from
`allocate_model_slot_00af6b60`; line 56 passes that same identity and selected
variant to `construct_model_00af74a0`. Its catch at line 58 invokes
`return_model_slot_00af62f0(raw)` before the outer event-base unwind. The callee
contract in `include/bsp/registered_model_effect.hpp:27` identifies the existing
F8D2D0 pool, 2E0h stride, 2DCh object payload and trailing pool ID. It requires
complete Native constructor/member unwind before throwing. It already exposes
the identity needed by `prepare_binding`; no additional particle allocator or
allocation hook is necessary.

The future callee must use the existing initialized
`GameNativeParticlePoolProcess::model_pool_00f8d2d0()` owner for allocation,
failed raw return and successful terminal return. Startup, allocator-list
registration and CRT pool destruction already exist. Do not create another pool,
initialize it again, substitute the ordinary 188h model pool, or add ownership to
the borrowed lifetime/construction contexts.

The persistent host storage must have a stable address and be fully available
before entering AF74A0. It contains or borrows the sole stable `NativeModelOwner`
over this raw slot, inactive storage for the particle reference, a saved opaque
raw identity, the two existing admissions, borrowed lifetime/registry services,
and enough host state to distinguish prepared, published and retired outcomes.
These are required roles, not a newly implemented class or alternative domain.
The caller remains responsible for the raw allocation until construction returns.

`NativeModelOwner` preparation (`src/native_model_owner.cpp:150`) preserves the
first 184h bytes, places the existing views, creates the preconstruction node
binding and registers that binding in the existing scene runtime. It may allocate
and throw. Its opaque preimage copy preserves +04; it does not establish a live
Native count or perform a typed reference-count load.
Its prepared destructor forgets that scene binding and ends placement lifetimes;
it does not run Native destruction. Supplying the actual 2E0h extent leaves the
derived tail and +2DC pool ID intact. The minimum 188h size check in this reusable
owner is not evidence that this particle allocation comes from the 188h pool.

Before any Native construction, finish all host storage allocation, owner/scene
preparation, `reserve_binding()` on `owner.environment.nodes.attachments` and
`prepare_binding(raw)` on the exact canonical actual-owner registry. Validate
that the registry is the one borrowed by `owner.environment.retained_owners`,
and that the mesh/material/section/string contexts use the identities required
by AF74A0. The exact order among independent host preparations can follow RAII,
but all must finish before calling `construct_native_particle_model_00af74a0`.
On any preparation failure, cancel whichever admissions exist, end the prepared
owner/host record and rethrow into the existing raw-return catch. No +04 decrement,
Native destructor, extra retain, alternate pool return or retry belongs there.

## Publication order and reference lifetime

| Boundary | Required state and operation |
| --- | --- |
| Before AF74A0 | Stable record, prepared owner/scene binding, one lifetime credit and exact still-null registry entry. `find(raw)` is null; `resolve_actual(raw)` cannot use the entry. |
| Base construction | AF74A0 validates its borrowed domains, then B75030 constructs the node and sole +04, publishes the base profile and makes the owner live. |
| Derived profile | AF74A0 writes D5DA50 and the early derived fields, then calls the existing nonthrowing `bind_particle_profile(base)` hook, before its later-body `try`. |
| Lifetime publication | In already available reference storage, construct the admitted `NativeParticleModelReference` over that same owner, lifetime access and retirement context. It validates the live owner, +04, pool/profile/disposal and current virtual0/4, consumes the existing Generated credit and replaces the four callbacks on the existing scene binding. |
| Canonical actual-owner publication | Immediately call the same registry's `publish(std::move(token), reference)`. Its exact-entry, alias and positive-count checks precede storing the companion and consuming the token. There is no insertion, rehash, retain or Native callback on valid publication. |
| Hook return | Both lifetime lookup and canonical actual-owner lookup must now describe this same reference. No Native call, external callback or yield may be inserted between the two publication operations. |
| Remaining constructor | Only now does AF74A0 prepare/retain the variant and construct/register the reached geometry, materials, stream, emitters and manager entry. |

The Generated credit protects capacity for a single binding; it does not reserve
the canonical hash entry, own Native storage or add a Native reference. Registry
preparation reserves an opaque aligned key without touching +04; it does not
construct a companion or make lookup available. Both capabilities are move-only
and independently cancellable before consumption. Prepared registry entries count
toward `size()`; that count is not the number of live companions.

The reference borrows the one atomic at actual +04. Native node construction
creates its initial value; the host record, profile hook, credit and registry do
not increment it. A successful creator remains a real Native ownership fact even
if later event parenting or event-registry work throws. The outer constructor
stores `effect.model_1c` after AF74A0 returns and its later state-0 unwind performs
no model release. An adapter must preserve that behavior and retain its persistent
record; it must not synthesize a successful-model rollback.

The profile hook is `noexcept`, and the reference constructor retains its runtime
validation. Registry publication may throw on mutex failure; invalid capability,
alias or count terminates. Therefore this design removes ordinary host allocation
from valid profile publication but does not turn every failure into a recoverable
error or prove a universal no-failure/heap claim. A publication contract violation
or mutex exception crossing the existing hook is terminal, outside AF74A0's later
catch. Do not forward this hook to allocating ordinary `bind` or promise that its
failure returns the raw slot.

## Failure, retirement and quiescence

| Actual failure/terminal phase | Matching contract |
| --- | --- |
| Host preparation or AF74A0 validation before B75030 | Cancel pending admissions; destroy only prepared host/placement state; caller's existing AF62F0 raw return runs once. |
| B75030/node construction throws before profile hook | Existing node/member unwind and `end_tail` complete first; owner is dead. No particle reference was constructed. Cancel admissions and retire host state, then let the existing caller return the raw slot. |
| Admitted reference/publication invariant failure inside nonthrowing hook | Terminal contract failure. No invented recoverable Native cleanup or blanket raw-return promise. |
| Later AF74A0 body throws after publication | Its catch destroys the emitter-pointer array, runs B750C0 base unwind, then invokes `retire_failed_particle_profile`. That hook calls the reference's `retire_after_failed_construction` only with the owner dead; Generated binding is removed before the disposal callback. The callback removes the exact canonical registry binding and marks the record retired. The outer AF62F0 catch still owns the one physical raw return. |
| Successful Native zero-reference terminal | The reference validates current virtual0/4, marks itself destroying, executes AF7F40 -> AF6C50 -> base destruction and successful F8D2D0 slot return, then unbinds the Generated lifetime and invokes disposal. Disposal removes the exact registry binding using the saved identity and reference. |
| Later event/parenting failure after completed AF74A0 | Preserve the completed model, its creator reference and companion record. Native event state-0 unwind has no model destructor. |

The later AF74A0 catch is not the full successful AF6C50 destructor. In particular,
the Source increments the global live count before random/manager registration;
the catch contains no invented live-count reversal, weak-manager erase or full
variant/material/mesh destructor. Its exact reached effects remain observable.
This readiness result does not repair those semantics or supply the missing
actual emitter/variant/geometry terminal providers.

The disposal callback must handle both successful physical return and failed
construction without physical return. It unbinds metadata exactly once, marks
host retirement and does not read Native fields, decrement +04, return/free a
slot, retry a terminal call, or delete a record still borrowed by its construction
or callback frame. In the successful path the Native slot has already been
returned before disposal; use the saved identity only as an opaque key. Keep slot
reuse serialized until the old canonical entry is removed.

Destroy/reset a retired record only after all constructor, profile-hook,
retirement-hook, Native terminal and resolved-companion borrowers have returned.
The registry mutex protects metadata operations only; it neither keeps a returned
companion alive nor proves Native quiescence. The registry and Generated runtime
outlive pending tokens and live records; the model/node environment, scene domain,
actual pool, profile/type words, string services, manager publications and reached
terminal providers outlive all Native cleanup and callbacks. Do not destroy an
unretired reference: its destructor deliberately terminates.

The particle manager's array is weak. AF0950 appends the model identity without
retaining it; AF0AE0 removes it. Manager destruction frees its arrays and destroys
the manager base, without releasing each model. Keeping records until the array
storage is freed alone is insufficient. Drain actual clients and all callback
borrows before manager/pool/shared-singleton/renderer service teardown; no such
new application drain is introduced here.

## Reached factories and prepublication calls

`finish_mesh` captures the current renderer and its virtual5C entry before the
descriptor getter, then calls the abstract renderer callee with `(0, 0x1000,
descriptor)`. The current application profile checks virtual5C == B287C0, but no
concrete particle callee supplies that dispatch today. The existing implementation
`create_native_registered_vertex_stream_00b287c0` allocates from actual 108FE18,
calls B4BC00 and unwinds the raw allocation if construction throws. Only after
B4BC00 returns does its optional `acquired_before_registration` output receive
the creator, before renderer-array appends. That output is not a preconstruction
raw-slot callback. Array registration can still throw after a completed creator
has been exposed; preserve the acquired frame and real terminal services.
The instance-geometry call omits the optional output and assigns its acquired
stream only on normal factory return. Do not infer that every current caller
retains a recoverable creator identity when a renderer-array append throws.

The six concrete Source caller files found for B287C0 are generated-model,
stream-clone, GUI text, instance geometry and the two post-effect constructors.
They bind completed streams after the factory. The post-effect blocks already
have optional reference storage, while instance geometry calls a supplied
completed-stream binder; neither changes when the raw identity becomes visible.
`GuiNativeGeometryOwners::register_stream_clone_creator` adds a list entry,
allocates the reference and performs ordinary canonical registration after
construction. Its documented interrupted-acquisition contract preserves the
creator/companion on metadata failure. The new preparation token is not used by
any of these callers. A mesh-field reader reaches the same GUI helper. The
similar tracer hooks are separate abstract contracts, not particle adapters.

| Reached body before its own companion publication | Bounded Source result |
| --- | --- |
| Particle B75030/B6F5A0 before derived profile hook | The actual-name overload initializes +04, empty hierarchy/retained/light/scene fields, name and matrices. Its exception cleanup destroys name/empty-array/base state, then B75030 ends the owner tail. No explicit canonical resolution/release of the constructing particle identity occurs in those bodies. String/allocator callback behavior is still a supplied lifetime contract. |
| Mesh B73D70 / section B857F0 | Fresh construction initializes the Native record. The particle hooks receive it only after completion. Section 533FA0 hides its allocation and constructor internally. No existing raw-key admission hook is exposed there. |
| Material B18B60 | Common initialization zeros destination source/texture/effect slots; clone retains incoming actual owners. `assign_actual` releases an old field only if nonnull, not the fresh material identity itself. Constructor failure restores the base profile; the particle helper returns the raw slot through B17A80. |
| Logical stream B4BC00 on particle flags 1000h | It retains the shared physical stream and publishes its raw logical identity into physical weak registration before a canonical logical companion exists. It may release a previous declaration in +68, and base unwind may release retained +4C; those identities are field owners, not an explicit self-release. Fresh fields begin null, but no exclusion of allocator/lock/service reentrancy is claimed. |
| AF74A0 after derived profile publication | Canonical particle publication is required before variant preparation and all reached Native callbacks. Completed material/section/stream creator releases still require their own real canonical owners. |

The mesh/material/section `noexcept` hooks only receive completed storage.
Preallocating a generic host record does not reserve a hash key whose identity is
still unknown. They cannot simply forward to allocating ordinary registry bind
or GUI list/reference creation and satisfy a preconstruction-allocation contract.
A complete factory packet needs separately reviewed raw frontiers and real
failure/acquired-creator contracts for those owner families. This document does
not add a hook, replace a Native virtual call, assume reentrancy is impossible,
or infer that every reached operation is allocation-free.

## Smallest next implementation packet

Recommended packet: `cc12_particle_persistent_prepared_record`. Assign only
`include/bsp/native_particle_model_lifetime.hpp`,
`src/native_particle_model_lifetime.cpp`,
`docs/CC12_PARTICLE_PERSISTENT_PREPARED_RECORD.md`,
`reports/cc12_particle_persistent_prepared_record.json` and its ignored local
evidence directory. These are disjoint from renderer/registry/factory files;
the existing lifetime translation unit already participates in the build.
No implementation is authorized or begun by this readiness packet.

Implement only one nonmoving host-record composition with the roles above, using
the existing owner, admitted reference, lifetime runtime and same canonical
registry. Its explicit preparation accepts the caller's real raw particle slot
and borrowed domains; finish every allocating preparation before AF74A0. Its
profile-publication operation must require the same owner and live D5DA50 phase,
consume exactly the two existing admissions in the specified order, install the
reference callbacks without replacing the node's `NativeModelOwner` context, and
add no Native retain. Its failed-construction retirement must require completed
base unwind and preserve caller-owned raw return. Its disposal callback removes
metadata and marks retirement; record destruction remains an explicit quiescent
caller action. Destruction while live or inconsistently published must reject the
contract rather than invent Native cleanup. Preserve a completed creator and
record after later event failure.

Keep the record disconnected from `RegisteredModelEffectCallees`,
`NativeParticleModelConstructionCallees`, current renderer factories and all
application startup/shutdown. Do not add a second registry, Generated runtime,
Native owner, pool, model reference, factory hook or permissive fallback. The
complete producer adapter and real service/drain owner remain a later packet.
That bounded record can make the protocol concrete without pretending the
missing mandatory construction callees are implemented.

## Replay and evidentiary limit

The report records all local pins, full finite query matches, 47 bounded Source
excerpts, ten lexical order checks, the separate actual Git/current-file receipt,
and matching in-place/physically relocated replay results. The guarded helper
preloads standard modules/codecs, then reads only its supplied ZIP; it rejects
other opens/writes, subprocesses, networking, dynamic library loads and directory
enumeration. Replay recomputes complete Source/Git coverage, all historical
project pin comparisons and the Source queries from retained bytes.

This is Source readiness evidence. It does not newly establish ABI equivalence,
Native unwind ABI, compiled call closure, runtime reentrancy behavior, successful
factory construction, application availability or game validation. Primary
acceptance of the two prerequisites is recorded as a separate fact; their earlier
binary results are not relabeled as tests of an unimplemented persistent record.
