# CC16 remaining binding gaps

Source snapshot: published Main `a6f59bb18bc124d208eea75588333b0931aefa33`,
with the reviewed Mesh low-provider Source integration in progress.
S1's scoped Source/Core/link/build admission is recorded in
[the Model/Matrix integration record](CODEX_RAW_MODEL_RESOURCE_BORROW.md).
The continuing goal still includes the full handoff's bindings, switch gates
and runtime checks. The observations below identify concrete missing seams.

The actual Mesh pool releases its native critical section inside
`NativeMeshPool::return_slot_00b72da0`, before the whole B74280 scalar returns
and the canonical companion retires its registry entry. The next valid
allocation can reuse that returned slot. Exact `(identity, reference)` erasure
is payload-free metadata, but its mutex does not reserve the physical
generation. Current GUI factories construct before `bind` checks duplicates;
the existing `prepare_binding` is not consumed by those paths. Trim/destroy
also require their actual caller's quiescence. A qualified alternative design
can retain the old C++ record while a genuine new allocation replaces its active
address entry. It requires private old normal payload-end and new allocation/birth
issuers, nonmoving records through complete outer calls, and exact-record retirement
which leaves a newer active record intact. Whole-pool serialization is not
logically required by that conditional design. Current registry, owning lists,
resolvers and all producer/failure paths must still consume it together. No race
is claimed observed. Full scalar order stays intact; moving a throwing unbind
before pool return would change failure order.

The separately reviewed [Mesh low provider](CODEX_CC16_MESH_STORAGE_LOW_PROVIDER.md)
adds genuine static/explicit-pool allocation, common constructor-return and normal
payload-end facts. The existing constructor has no demonstrated defect in its
audited MSVC Win32/installed-atomic domain. This prerequisite does not publish a
canonical or select a producer. Real scalar action/credit custody, all eight direct
construction sites, external/prepared issuers, early ordinary-zero failure
transport and complete owning synchronization remain required. Official compilation
and retained-link checks pass; final independent Core and joined integration
admission for the additive provider are pending at this checkpoint.

The real Mission factory cannot consume S1 by substituting its new accessors.
`GameNativeModelDomain::make_mission_client` supplies `raw_storage=nullptr`;
only the Page client supplies the real immutable raw lifetime context.
`prepare_model` constructs `NativeModelRawLifetimeBinding` only in that latter
case. Mission's genuine raw B75030 constructor alone therefore does not grant
checked S1 admission. Mission also consumes `instance_access`, which the raw
Page client rejects. Its Matrix records still construct the legacy
`NativeMatrixIndexedModelOwner` and companion, rather than the new storage
receiver. Existing live records cannot be adopted by profile or retargeted.

The canonical-family contract distinguishes publication from creator-credit
discharge. A later canonical borrow needs its own genuine positive-live issuer;
it cannot impersonate the old creator after release. Full zero must observe
actual count zero and dispatch through the current profile's real slot 04,
without another decrement. Native physical retirement, registry retirement and
C++ holder collection remain separate: all holders survive the complete outer
return/catch fence, and flags-zero or failed disposition retains real debt.
The contract's 39 snapshot inputs are pinned; its SHA256 is
`817554b33fc90d126e7b02b7bd39fa824ca73ff0b13d31e6c044b9de9475356e`.

The instance upload still uses logical `SceneAttachmentRuntime`, with the
explicit actual-lighting containment switch. Mission root dispatch admits
only null lighting. Lighting retirement still expects exactly two resource
credits and assumes all model `+170` values stayed null. Faithful actual
B6ED80 composition requires real factory/service admission, complete canonical
families, persistent caller frames, authoritative actual scene associations,
matching detach/zero/destruction paths and genuine per-model resource credits.
Changing the expected count or skipping another call is not that composition.

The worker/thread review establishes specific conditional edges. The actual
FrameGraph binds ordinary queue execution to both main end and worker end.
Raw worker frame helpers bypass the Application phase wrapper. The worker's
shutdown flag is outside its inner run loop; its stop protocol still matters.
Raw stop/destruction wait results are discarded, whereas `after_native_drain`
independently checks the retained handle for `WAIT_OBJECT_0`. Host retires
GUI/text/font borrowers before that later drain. These are phase-specific
observations, not a common guard around every Mesh caller. The consumed
run/rate/callback/wake installer and complete error-interval exclusion remain
unproved. Thread evidence SHA256:
`a2e93f467e8ccd0b9fd3163f74157370620103173ede201e999938ffd08e98a1`;
all 31 snapshot Source pins and the eleven recorded call edges are retained.

The Mesh generation contract was fully read and its 23 Source pins rechecked
at this snapshot: SHA256
`a5c923ee745e8f69f7b2d7115f821afc29eed96b988de01a57bf7122f605eb8a`.
Root independently inspected the pool allocation/return/trim, registry
bind/prepare/erase/resolve and Mesh scalar/disposal seams. The current Mission
factory/consumer review separately pins eleven Source inputs. Evidence is in
`local/mesh_generation_primary_cc16_20261031/`.

The existing USN12 diagnostic proposal also remains unapplied. Independent
review found an omitted geometry-load cache invalidation, missing immutable
joins from each retained clearance writer to its actual query/cache/geometry
epochs, and no selected-group/segment identity from list-nonempty status.
The stronger causal trace now has a concrete surviving-Mission design amendment.
The actual Mission Impl owns the diagnostic arena before its Frame owner; Frame
and AI borrow it. Child replacement permanently invalidates the same session,
and export reads the parent directly. Independent review of that new design and
a complete unapplied Source/inverse with actual compiler/Core checks remain
required. The earlier independent Source-ASM review found seventeen feasible
observation windows, not emitted proof. The historical
four-field experiment retains twelve matching pairs and its failed USN12 pair; its
separate baseline control is inconclusive. The qualified non-switch lifetime
Source admission remains published. Neither endpoint danger zero nor a new
trace can replace the original failed result. Independent review SHA256:
`1fcd87a58e8b01fc940f7f93393f2f17e3f90725d6d0ef8dc1cb0417c02e79d7`.

No Source implementation or activation follows from this readiness review.
All nine raw GUI switches remain false at the snapshot. Actual caller/thread
exclusion, whole failure ownership, family bindings and required gameplay
validation remain technical work within the user's existing authorization.

The separately admitted [normal zero interface prerequisite](CODEX_CC16_ORDINARY_ZERO_INTERFACE.md)
relaxes only the base/normal-helper exception types. Current concrete callbacks
and legacy queue/destructor policies remain unchanged; the full owning/family
bindings above remain required.
