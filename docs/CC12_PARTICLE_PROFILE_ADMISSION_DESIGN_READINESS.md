# CC12 particle profile admission design readiness

**Design conclusion:** registry bucket reservation alone is insufficient. A
bounded design is feasible with (1) an inactive prepared entry in the SAME
canonical actual-owner registry, (2) a particle-reference overload consuming the
existing generated-lifetime admission, and (3) stable caller-owned record storage
prepared before AF74A0. None is implemented here; this is not construction or
application activation approval.

Source capture: worker `5a237ee0ff9b5116c2c54f88e2615807af91d670`, Root `b7727f1286c325d3bb594d74bc4a22657cda2f9c`.
The worker retains accepted C++ Source3352 at `99b170231e937da304703fe01221bcc8b8b8aef7`. Root has exactly two
newer C++ files: `include/bsp/native_wide_string.hpp` and
`src/native_wide_string.cpp`. Their raw-context 00436430 overload is absent from
the older frozen Source and present in the captured Root bytes. Both complete
versions and their exact Git patch are retained. The other 3350 old epoch project
files match that captured Root; this packet does not claim later whole-Root or
compiled-artifact equality.

## Existing owner boundary and smallest changes

`GameStartupHost::native_renderer_` already owns the renderer Impl, its one
`NativeRenderActualOwnerRegistry`, and the graph's same scene/generated-lifetime/
node-destruction runtimes. `GameNativeRendererApplication::actual_owners()` already
borrows the actual registry. Do not create another registry, scene domain, pool,
reference count or `NativeModelReference` to implement the particle hook.

The future stable record needs the existing `NativeModelOwner`, borrowed particle
lifetime/access/disposal dependencies, one generated-lifetime credit, one prepared
registry-entry token, and disengaged in-place particle-reference storage. A
`std::optional<NativeParticleModelReference>` is a concrete storage pattern already
used for camera companions; it creates no reference object or diagnostic count
while disengaged. The application must allocate and retain the record itself and
its bookkeeping linkage before AF74A0, so successful handoff needs no allocation.

The existing `NativeModelOwner` can be prepared before AF74A0 using its ordinary
constructor: scene-vector growth may throw there while the caller still owns the
raw slot. Its existing 184h byte-preserving preparation remains unchanged. This
proposal adds no semantic native+04 load, validation, retain or reference-interface
construction before the native base has made that word live. It does not move
AF74A0's current volatile loads or native initialization into preparation.

For this narrow design, no new scene API or generated-lifetime storage API is
required. `GeneratedModelLifetimeRuntime::reserve_binding()` and its consuming
`bind(node, BindingAdmission&&)` already exist. The particle companion needs a
consuming constructor path; its present constructor calls ordinary `bind(node)`.
An outstanding credit passed nowhere does not help: ordinary bind protects that
credit by reserving an additional slot.

`NativeCameraReference` already demonstrates the precise constructor-overload
shape: it accepts `GeneratedModelLifetimeRuntime::BindingAdmission&&`, retains
existing live-owner checks, and calls the consuming bind. This is a Source design
precedent, not a ready particle implementation or an unconditional no-fail API.
The existing particle constructor may remain for current callers; the new path
must not delegate through its ordinary allocating bind.

The minimum registry change is preparation/publication/cancellation metadata in
`native_render_context.hpp/.cpp`. The particle constructor change belongs in
`native_particle_model_lifetime.hpp/.cpp`. The stable record and actual environment
composition belong to the existing renderer/application owner boundary. The
specific application composition is still missing and remains a separate hold.
No original native function body, pool allocator or AF74A0 order needs changing
for this design.

## Why reserve and extracted-node shortcuts are insufficient

The pinned installed MSVC 14.51.36231 headers show:

- `unordered_map` uses `_Hash` with list nodes storing `pair<const key, value>`.
- `_Hash::reserve` only calls `rehash(_Min_load_factor_buckets(...))`.
- A new `emplace`/`try_emplace` still constructs `_List_node_emplace_op2`.
- That constructor calls `_Allocate()`, which invokes the allocator's
  `allocate(1)` independently of bucket capacity.
- Insertion of an extracted `node_type` avoids that new-node allocation, but
  still checks load and may call `_Rehash_for_1()` before inserting it.

Thus `entries.reserve(n)` plus the current `entries.emplace(identity, &reference)`
can allocate in the noexcept hook. A detached-node design would additionally need
correct pending-capacity accounting across every competing insertion, compatible
allocators and synchronized insertion; bucket reserve alone is not that proof.
The smaller proposed design puts the inactive entry in the actual map during
preparation, paying both node and bucket allocation before native construction.
These are current installed-header facts, not reconstructed historical compiler
preimages or new compilation results.

## Proposed prepared registry entry

These are proposed API roles, not declarations added by this packet:

| Operation | Required behavior |
|---|---|
| `prepare_binding(raw_identity)` | May throw. Check the aligned raw address and absence of ANY existing key, then allocate/insert `{identity, nullptr}` in the existing map. Return a move-only token. Do not dereference native storage or accept a reference companion. |
| `publish(admission, reference)` | At the valid post-D5DA50 point, consume the exact token and fill the existing mapped pointer. No `emplace`, `operator[]`, node allocation, rehash, native retain or replacement. |
| `admission.cancel()` | If still prepared, erase only that exact null entry and invalidate the token. No native read, release, destructor or pool return. Inactive cancellation is idempotent. |
| Existing `unbind(identity, reference)` | Retire the exact published entry through current metadata-only matching; prepared cancellation must not use this live-entry operation. |

A token identifies its registry, raw key and exact prepared entry. A pointer to
the mapped value can remain stable across the map's rehashes; an iterator must
not be retained across rehash. The installed rehash implementation grows buckets
and relinks existing list nodes. Ownership must prohibit erase/extract/replacement
of a live token's entry; only its move-only token can cancel or publish it.
Cancellation and publication must reject stale, foreign or already-consumed
capabilities before touching any native count.

Use the existing registry mutex for short metadata operations, and the existing
serialized model-owner contract for the whole preparation/construction sequence.
Never hold that mutex across native construction, resource callbacks, registry
resolution or a terminal callback. Token movement and unrelated reentrant
preparations must not invalidate the stable owner/entry storage or consume this
model's lifetime credit.

Current `find` already returns the mapped pointer; a prepared null entry therefore
looks unavailable to callers. `resolve_actual` continues to reject it. Ordinary
`bind` must continue to reject the duplicate key, including a prepared key: testing
`find(identity) != nullptr` alone would miss it. `size`/`empty` must count prepared
entries as owned metadata so registry destruction cannot silently discard a
pending token. No caller may treat that metadata count as a live native count.

## Post-base publication and exception boundary

Preparation validates host identities, borrowed contexts, stable storage and
reservations. It cannot perform the particle constructor's live owner/profile/
native+04 checks early. Those checks stay after B75030 and D5DA50 publication.
The proposed admitted particle-reference path then consumes the same runtime's
lifetime credit and installs the same scene callbacks. The hook immediately
publishes the prepared canonical registry pointer to that exact companion before
returning. No native callback or external model exposure belongs between these
metadata steps; the constructor's creator reference remains owned by its caller.

The existing consuming lifetime bind validates identity and can throw on a bad
capability or duplicate. Its reserved append does not allocate for a valid credit.
Likewise the installed `std::mutex::lock` has error paths that throw. Therefore
this design's nonallocating/nonthrowing publication claim is explicitly limited
to **valid, serialized admitted contracts**. The already-noexcept profile hook
makes invariant/lock failure fatal, not recoverable. Adding `noexcept` alone
neither removes allocation nor proves successful completion.

An implementation must retain post-live invariant checks and the exact token/
identity checks, using the current fatal-contract boundary if they fail. It must
not swallow an exception and report a usable model, perform speculative raw-slot
return after partial publication, or promise recovery that AF74A0 does not have.
A requirement for recoverable rejection after D5DA50 would require a different
constructor/hook contract and is outside this bounded design.

## Companion and disposal lifetime

`NativeParticleModelReference` must remain unconstructed during preparation.
Placement/emplacement occurs only in the admitted hook after the same owner is
live. The borrowed `RenderCommandReference(actual_count)` constructor is then
used; never instantiate its diagnostic default constructor or add a retain.
The particle slot remains F8D2D0's 2E0h allocation, with the slab ID at +2DC;
the base 188h pool/deleting-reference path remains inapplicable.

The stable record must survive AF74A0 and its exception frames. Its disposal
callback saves/uses the raw identity as metadata, unbinds that exact canonical
registry entry, and records retirement. It should not free/reset the record or
companion from inside an active constructor callback. The existing cockpit block
provides a concrete precedent: retirement only sets a host flag, and a separate
`reset_after_host_quiescence()` destroys retired/dead companions after all borrows
end. Reuse that bounded discipline, not the cockpit's whole state machine or its
camera domain.

A particle record can use the existing owner phase, optional engagement and one
retirement fact rather than introducing a broad second lifetime machine. Unused
credits cancel independently. A reset of an engaged companion requires its real
retirement notification and a DEAD owner; a prepared but unconstructed owner uses
its existing prepared destructor. No reset may infer cleanup from the raw count,
read already returned bytes, delete a live companion, or replay native cleanup.
The persistent owner must also retain every actual environment/service dependency
and serialize raw-slot reuse through metadata retirement.

## Required failure accounting

| Point | Host bookkeeping and native obligation |
|---|---|
| Raw allocation returns null | Follow the existing caller's null path; no fabricated record/companion or raw return. |
| Host preparation fails or is abandoned before AF74A0 | Cancel unused credits/null entry; destroy only prepared host views. No companion exists. The caller retains the raw-slot return obligation. |
| AF74A0 rejects its prepared-owner/domain preconditions | Same unused-admission cleanup; no successful derived lifetime or native delete. |
| B75030 fails before entering native base construction | Keep the actual prepared phase and cancel unused reservations; do not invent base cleanup. The outer caller still owns the raw slot. |
| B75030 native base construction throws | Existing base unwind leaves owner DEAD and forgets its scene association. Cancel unused reservations, keep the companion disengaged, and let the outer caller return the raw slot once. |
| Valid profile hook | Emplace the admitted companion, consume the lifetime credit, publish the same canonical entry, return without a native reference delta. |
| Profile invariant/lock failure | Existing noexcept boundary is fatal. Do not invent a recoverable cleanup/success path after partial publication. |
| Later AF74A0 failure | Existing emitter-member/base unwind runs first. Failed-profile retirement unbinds generated/canonical metadata and records retirement without pool return. After the constructor frame is quiescent, destroy retired/dead host objects; outer AF62F0 owns exactly one raw return. |
| AF74A0 succeeds | Keep the already-owned stable record. No post-native container allocation is allowed to turn metadata handoff into a new failure point. Later effect/parenting failure does not gain an invented successful-model destructor. |
| Final zero reference | Preserve AF6C50 destruction, particle-pool return, generated-lifetime unbind, canonical unbind and retirement notification. Reset host companions only after quiescence. |

If native cleanup itself throws in the existing protected cleanup/noexcept
terminal path, preserve its fatal behavior; do not retry destruction or return.
Late AF74A0 failure also preserves its current partial native effects, including
its lack of successful-derived count/manager rollback. Admission is host metadata,
not permission to broaden native unwind.

All live/retired records and unused tokens must settle before the renderer scene/
registry domains and actual pool/manager services die. Existing renderer failure
and retention integration still needs the real owner record. No new application
failure flag, forced registration, replacement publication or activation is
introduced by this document. Other construction callees remain independent holds.

## Copied evidence and limits

The whole accepted profile, RNG, renderer and pool reports are retained by exact
committed blob and Git commit/tree proof:

| Contract | Exact committed report | Bytes / SHA-256 |
|---|---|---|
| profile | `01de2fdbc5006b34b59c04f36508ede34e157c16` / `reports/cc12_particle_model_profile_callee_owner_readiness.json` | 516350 / `d5cd2f537dd2563ee9c8d2383d5d587b147dcdcf85a56aa20b0191d65c0ca76e` |
| RNG | `51d3206884e66faef62f25df3a19bbd69c5d5993` / `reports/cc12_particle_model_constructor_callee_owner_readiness.json` | 783022 / `76e42fd9faf61621b1a1a30fb47e25e348f75ac77f9a9346381803005bf2a1a0` |
| renderer | `d9a9e64736b69b0e77d374fb8dffee014b020b6e` / `reports/cc12_particle_model_second_callee_owner_readiness.json` | 1182598 / `fa322bd755578516ef732cb7026c0a6f43ae3c807a44eca26ea5d4c9623553de` |
| pool | `cbac550781e09b6d96a68c46895a24e429a8ad79` / `reports/cc12_particle_model_pool_borrow_readiness.json` | 200589 / `b71bd9f7f358de986dcff796a6035887064cdd853759556e6b19b2430cffa1a5` |

The packet retains 4214 complete baseline and captured Root Source files, with
actual committed blobs for each side; each query corpus contains 4207 paths.
Both Source3352 manifests and all 3352 project-file preimages on each side are
qualified. Their four build-artifact declarations are deliberately not copied or
checked: no new artifact or compiled credit is substituted. The two complete
wide files explicitly qualify the old/new boundary; seven worker files differ
from the old frozen bytes only by CRLF/LF.

Six complete currently installed SDK headers supply the container/mutex evidence.
Eleven queries are rerun on both Source versions, with 32 exact Source excerpts,
ten SDK excerpts and both complete wide-file versions. Their paths, sizes and
hashes are pinned. Four accepted reports are copied whole; their old proof trees
are not recursively followed. Prior accepted compiled evidence remains in those
reports and is not a fresh compiled claim by this packet.

The standalone guarded replay recomputes retained Git object/tree/blob identity,
both Source epochs, the precise two-file difference, Source queries/excerpts and
SDK allocation-chain anchors. After imports it permits reads only from its ZIP,
rejecting outside opens, writes, subprocesses, network and dynamic loading.
A physically copied ZIP/helper replay gives the same result. Actual Git and live
worker/Root-source verification is separate, with its own captured commit IDs.
It does not claim equality to an unspecified later Root state.

- ZIP: 15134683 bytes, SHA-256 `9399e64abd05777633c6843ac2ae3212a89d93089845a8c21b6d109355e9af5c`.
- Helper: 8735 bytes, SHA-256 `19febd914af349b5d11d58e50c6e1d4fe6abaa44c2cfaef90212f9545de6b0e8`.
- ZIP payloads: 4270; checked unique pins: 4268.
- Local evidence: `local/cc12_particle_profile_admission_design_readiness/`.
- Machine report: `reports/cc12_particle_profile_admission_design_readiness.json`.

No C++/build/test/runtime/Native/Ghidra work or new native body was performed.
The result is a concrete reviewable API/storage design and exact failure contract,
not an executed implementation, original ABI proof or application GO.
