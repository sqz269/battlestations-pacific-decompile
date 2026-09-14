# Actual renderer destruction

Addresses: `00B32920`, `00B339F0`, `00B32900`.

This packet reconstructs the full actual renderer destructor schedule and both
deleting entries. It borrows the existing concrete raw child providers. It does
not establish a complete renderer runtime graph or complete shutdown behavior.

| Entry | Native span | Coverage | Original ABI |
|---|---|---|---|
| B32920 | B32920..B339C0, 4257 bytes | Complete body in the qualified actual provider domain | ECX actual 1D94h primary, plain RET |
| B339F0 | B339F0..B33A0D, 30 bytes | Complete scalar wrapper | ECX primary, stacked flags, EAX captured primary bits, RET4 |
| B32900 | B32900..B32907, 8 bytes | Complete secondary adjustor | SUB ECX,0Ch; tail JMP B339F0 |

The new source interface adds a borrowed `NativeRendererDestructorContext*` in
EDX. It is not a drop-in replacement for the original binary ABI. Scalar free
uses only flags bit0 and happens only after a returning destructor. The returned
pointer bits identify the captured primary even after its memory was freed.

## Actual application bindings

The context borrows one `F8D394` renderer publication and the same `01090AA0`
manager, `01090AA8` string-pool and DWORD `01090AA4` small-return gate. It also
borrows the actual state, system and Lua publications at `0108FE90`, `0108FE94`
and `F8D434`. The raw string adapter owns no substitute pool or manager.

Binding reset, binding-cache clear, query terminal, declaration/effect/texture
cache cleanup and actual node contexts must use those same application cells,
canonical owner bindings, pools and allocation domains. The resource-support
consumers require actual `SoundLifetimeAccess`; a projected singleton manager
must never be reinterpreted as the raw manager. The fixed control-worker process
binding and its substantive event/clock/frame providers outlive every joined
worker. These are explicit caller lifetime requirements, not setup performed by
this destructor.

Sixteen original numeric profile views are checked against fresh live and PE
words. They select reconstructed source routines; original EXE addresses are
never executed by this packet's finite dispatch. The parent directly covers
query `D62AD0`, surface `D619A0`, three material-state profiles `D61A2C/34/3C`,
and private/pooled physical-buffer profiles `D61E10/34/58/7C`. Existing B241C0
supplies its ten concrete owner profiles. State, system and Lua scalar dispatch
supports the evidenced derived and base profiles, and control uses `D5F1F0`.
Unsupported identities have no successful default behavior.

For nonnull renderer `+19E0/+19E4/+19E8`, this packet reuses B6DFA0 through an
existing canonical `NativeModelReference`. Its raw node identity, actual node
runtime and raw name-pool domain are checked before the shared hierarchy body.
No projected `GeneratedModelLifetime`, copied transform, fresh owner map or
replacement count is admitted. Original B4C700 return/store gates identify
model owners at `+19E4` (DebugSpheres) and `+19E8` (2DSprites). The writer/type
of `+19E0` remains unproved. Canonical host-binding and factory composition
remain separate from that original return-value evidence.

## Normal schedule and observable order

The source stamps the two renderer profiles, arms state24, destroys current
owned section `+199C`, joins/deletes captured control `+1970`, and clears that
current field only after return. It executes actual B24BF0, then traverses the
query table forward with an unsigned index and current count. Every query slot
is captured before atomic decrement; final zero uses current slot0 and the
BD30E0 current-profile/slot4 schedule. The captured slot is cleared after the
terminal returns, even if unregister changed the current table or count.

B241C0 clears the actual binding cache. Five surface fields `+197C..+198C`
follow. The three owner arrays pop in order `+1ADC`, `+1AF4`, `+1AE8`; the count
is reloaded after a returning release before its conditional decrement. Actual
model unlink/release precedes declaration, effect and texture cache flushes.
The destructor then reloads and scalar-deletes the three current singleton
publications; their child bodies retain their own early publication,
captured-section and current-publication unregister rules.

Eight conditional COM AddRef/Release pairs retain the same captured receiver
inside each pair and reload its current vtable after AddRef. Between pairs,
the renderer's current device/factory fields are reloaded. Two unconditional
Release calls remain unconditional. The physical vertex/index fields
`+1974/+1978` are released between the last pairs. The source neither nulls the
device/factory fields nor adds protective COM references.

The worker at `+1D2C` is followed by four record arrays and the capability nested
headers. Later normal raw DWORD array cleanup differs from the ordinary member
destructor wrappers: thirteen inline blocks capture the data pointer **before**
publishing final count zero. Capability `+44` and the four record arrays publish
count zero before reading their final data pointer. These sequences are retained
separately. A negative capacity invokes actual reserve-to-one behavior; current
source/count copying and old-data free precede replacement publication. The
negative-count DWORD initialization loop branches on the sign of the wrapped
byte offset after adding four, including the first unconditional iteration.
It does not reinterpret that loop as ordinary signed-count growth.

Effect and declaration base cleanup each arm their alternate state while
clearing the registry, then disarm before normal current-array resize/free.
The embedded section drains current positive depth and calls real
LeaveCriticalSection before DeleteCriticalSection, without freeing its storage.
The normal state then skips directly from6 to4. The remaining arrays precede
secondary B25FE0 unregister; primary B33D90 is armed as alternate28 only while
that secondary destructor executes.

## Original exception map

Fresh live/PE evidence covers handler `CBE07B`, FuncInfo `DF67F8`, map `DF681C`
and all29 cleanup actions in `CBDF00..CBE084`. The source implements their exact
normal state transitions and cleanup descent. States0..24 retain the existing
member destructor providers. Alternate states use the native captured nested
owner, not an assumed primary-relative replacement:

| State | Action | Next state |
|---|---|---|
| 25 | B29E40 at captured capability owner+44 | 18 |
| 26 | B317C0 at captured effect owner+4 | 8 |
| 27 | B316A0 at captured declaration owner+4 | 6 |
| 28 | B33D90 on captured primary | -1 |

There is no whole-parent rollback. Earlier native writes, releases, stale
pointer/capacity bits and partial failures remain visible. A second source C++
exception during cleanup terminates. Original private FH3 frame aliases,
asynchronous SEH and original exception object identity are not reproduced.

## Dependency frontier

Canonical nested terminals retain their existing `noexcept` and retained
operation failure limits. In particular, existing B41B10 destruction of a
nonnull material effect descriptor at `+C4` dispatches its current table[0] as
a host callable. That path requires a genuine reconstructed descriptor provider
or a null descriptor; a numeric original EXE vtable is invalid. This packet does
not recover that descriptor lifetime. Root's separate descriptor packet traces
the actual effect `+C4` producer through B45EE0/B43700 to `D61A44`, whose scalar
B46930 calls B458A0. Its nested sampler children still reach a host-table
boundary: B41830/B57B50 produces 2Ch `D621F4` owners, scalar B56FC0 calls
B56EA0/B56DE0 for their `+24/+28` state-list holders. Those bindings are being
recovered separately; this packet does not widen its supported domains from
that producer evidence. Shader construction and physical-lock
diagnostics remain separate lifetime-domain migration frontiers. Exact original
dependency `e2d96f61` adds `D62B64/B61D60` to canonical manager-drain dispatch,
using the popped actual owner even when current `0108FEDC` differs. It closes
the support-profile gap identified during this packet. The shared binding now
has its typed actual support publication and size104; full application shutdown
still requires the complete actual graph and its other terminal providers.

The parent body is fully scheduled within those explicit domains. This does
not establish closure of every nested owner lifetime, a fully initialized
renderer/material/VFS/node graph, original ABI interchangeability or gameplay.

## Evidence and verification

The report preserves full live-equals-PE spans, all146 normal/unwind transfer
rows, four read-only model allocation/return call gates, the29-state map,
16 profile views and a static instruction/source audit.
The full parent contains1255 instructions and114 CALLs. Static checks confirm
the normal state order and all13 data-before-count-zero inline blocks. The
numeric call gate checked110 rows with zero failures; the40 indirect transfers
are described separately with their current receiver/slot contracts.

Before evidence preserves the stale B32920 export ending after the first free,
the absent B32900 function and B339F0's three-byte returning-free gap. Root
explicitly authorized the official locked definition tool for B32900 and the
official call-site-only flow repair for B339F0. Both were saved and refreshed;
all three final listings have zero gaps. No global CRT noreturn property was
changed and no other Ghidra address was mutated by this packet.

Final configured build, dependency commits, link/import-only probe and immutable
artifact hashes are recorded in the report. No full parent destructor or parent
exception path was executed. Child fixtures and linked symbols remain distinct
from complete renderer runtime or shutdown validation.

## Integrated validation at 7cbd532e

Complete renderer destructor and both deleting wrappers passed final-library linking and import resolution. The probe did NOT invoke the destructor. Independent static review checked all29 cleanup receivers, ten COM call patterns, thirteen inline array cleanup blocks, sixteen profiles and four model-return call gates. The combined strict Win32 build, eight seed checks and both CTests passed.
The four final-library probes,134 direct/tail rows, twelve saved/read-back
annotations and58 live/PE spans are retained in `local/checkpoints/7cbd532e/native-renderer-destructor-wave/validation.json`
(SHA256 `559269e15cbe10ee773e9bc8dcd8372695dc890a33b9254553319ff8e6a39fb2`). Full parent execution and application/gameplay
validation remain open.
