# Directional shadow construction storage

Addresses: none. DY implements host storage and admission only, with **zero
native functions and zero native bytes**. A8E2E0, A8FA30 and A8FD30 remain separate
constructor-body work. The proposed entry declarations are not implementations
or original binary ABI replacements.

`NativeDirectionalShadowConstructionBlock` holds the durable state needed by
those three bodies. It is noncopyable and nonmovable, lives outside the actual
508h owner, and must survive every retained camera, viewport view and cache
operation. There is no second native identity registry, native count, allocator,
successful-provider callback or destructor/factory implementation in this module.

## Preparation and execution

`admit(environment, constants, installed_registry)` requires idle storage and the
existing raw node-name mode, matching node/camera D7A24C cell, canonical scene
runtime, exact viewport resolver, CRT bindings and camera tables. It emplaces one
persistent `NativeTextureCacheAcquired`, then reserves **four scene, four lifetime
and nine viewport credits**. All 17 credits precede native constructor events.
The actual registry's `admit` enforces installed identity. The scene and lifetime
providers account for pending credits before reserving their existing vectors.

The caller must separately bind the same actual0108FFB0 camera pool used by the
environment, wire the cache's actual string storage to the same raw pool cells,
bind the real VFS-name provider, and supply the original current-global contexts.
DY does not install or probe these domains. Their existing source contracts and
qualification limits remain unchanged.

The movable `Admission` owns all credits. An outer constructor first validates it,
then moves it locally and calls `begin_execution()` exactly once. Moving an
executing token terminates. Its private credits may then move to the real child
providers. Internal factory/derived/base helpers share that executing token;
they do not reenter the public gate. The separate constructor module defines the
single friend `NativeDirectionalShadowConstructionExecution` and the three
public entries declared in this header.

Preparation failure or unused cancellation cancels acquired credits, forgets
only unused/cancelled viewport records, and destroys only fresh cache metadata.
After execution starts, token cancellation only cancels unused credits and marks
the block settled. The constructor must finish its native normal/EH actions
before this token leaves scope. Settlement never performs native cleanup, forgets
records, destroys companions or discards the persistent acquisition.

## Durable slots and native lane aliases

Four indexed camera slots each contain optional `NativeCameraOwner` and
`NativeCameraReference`, a stable retirement cookie, `camera_completed`, and
`retired`. Completion is a host fact recorded immediately after B71A80 succeeds;
it protects a live camera from raw-slot return if later host reference binding
rejects it. It is not a new native EH state. Each reference binds the existing
canonical lifetime runtime with its matching credit and uses that slot's cookie.

| Record | Constructor site in the planned body | Role |
|---|---|---|
| V0 | A8E41C → B1F850 | standalone viewport, owner+504 |
| V1 | A8E46A → B71A80 | C0 first viewport, camera at owner+20 |
| V2 | A8E4DD → B71A80 | C1 first viewport, camera at owner+24 |
| V3 | A8E550 → B71A80 | C2 first viewport, camera at owner+28 |
| V4 | A8E5C3 → B71A80 | C3 first viewport, camera at owner+2C |
| V5 | A8E692 → B1F850 | standalone replacement, owner+10 |
| V6 | A8E6BB → B1F850 | standalone replacement, owner+14 |
| V7 | A8E6E4 → B1F850 | standalone replacement, owner+18 |
| V8 | A8E70D → B1F850 | standalone replacement, owner+1C |

These sites are inherited DX/DV evidence, not native transfers emitted by DY.
Camera raw allocations occur at A8E434/A8E4AA/A8E51D/A8E590; actual slot size is
45Ch. The companions are emplaced after that slot and its local name succeed.
No native allocation uses `sizeof(NativeCameraOwner)`.

The block contains aligned raw byte lanes, deliberately left unwritten by its
user-provided constructor. Reset also preserves their consumed bytes. The native
body must initialize only reached stores and use raw providers or byte copies
for scalar/header access; no executable FH3 frame or private-stack alias claim
is made.

| Storage | Original frame mapping | Offsets within the byte array (hex) |
|---|---|---|
| `base_lanes_[4C]` | EBP-44 through EBP+7 | flags00; saved owner04; white header08; camera headers10/18/20/28; late size pair30/34; mutable public word48 |
| `derived_lanes_[60]` | EBP-58 through EBP+7 | owner00; pair04/08 then loop raw slot04; matrix0C..4B; public word5C: light, count4, then fifth raw target |
| `factory_raw_lane_[4]` | EBP-10 DWORD | current raw508h allocation; consumed by the factory's own cleanup |

Separate base/derived/factory `BodyState` records contain host phase, native
unwind state initially -1, and native-site marker. They are not automatically
advanced by admission. The native bodies must preserve the DV base map's19
states, including map-only5/8/11/14, derived states0/1/2 and factory state0.
In particular, base failure invokes BD30F0 only; previously published children
survive. DY provides no cleanup action or normal state5/8/11/14 invention.

## Retirement and explicit reset

The indexed callback runs through the existing `NativeCameraReference` terminal
after native destruction, pool return and lifetime unbinding. It checks the
exact slot/reference/owner identity, completed-camera fact, dead owner phase and
nonduplicate retirement. It marks one host flag and reads no ended native bytes.

`reset_after_host_quiescence()` is an explicit caller assertion that every native
obligation and every host camera/frame/viewport/cache borrow has ended. It also
checks all observable storage before disposal: settled phase; every present
owner dead; every present reference retired; every viewport record unused,
cancelled or retired; and every cache/fallback and loader phase not-started or
complete, with no failed or started-but-unreturned name-resolution child.
It then destroys all references before owners, forgets quiescent records,
disposes safe cache metadata, resets host diagnostics and returns to idle.

A live orphan first viewport after a late camera failure still pins its record.
The registry deliberately exposes only record phase; this block neither invents
a recovery accessor nor reads a returned camera slot to discover that viewport.
A failed/running cache tree likewise remains pinned, possibly indefinitely:
there is no generic recovery API and DY never forges `complete`. Completed cache
metadata owns no extra native reference; destroying it does not release the
published fallback texture. Early host reference-binding rejection can leave a
completed live camera without a reference; reset refuses its live owner.

Block destruction requires idle storage and otherwise terminates. Thus callers
cannot use lexical scope exit as automatic native cleanup. Providers and this
block require serialized use; retirement/reset are noexcept and invalid lifetime
use terminates. Native hardware faults, arbitrary FH3/SEH, original
private-frame ABI, renderer execution and gameplay parity are not established.

## Evidence and verification

The source baseline is `d68f8770ce5ee5108dbd636badf515e610a1b47e`. DX's JSON SHA256
is `77675f12e46af94ccdf6cdb9099cd685d2825efe89737f85e249e2f28179d05c`; its design
SHA256 is `31bbb897e801724abd3f6e16cbe1223785d43586333a2c790003ff1c40dede11`.
The report pins the concrete cockpit/admission/lifetime/cache provider inputs.
No Ghidra query or mutation, new native recovery, ledger change, repository test
addition or fixture is part of DY. Build results are recorded in the report
after the primary's source review; no source execution claim follows from a
successful build alone.
