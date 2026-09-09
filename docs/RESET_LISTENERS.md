# Reset listeners: concrete occlusion-query wrappers

The observed renderer list `+19a0h` / count `+19a4h` / capacity `+19a8h`
contains concrete D3D9 occlusion-query wrappers. Its callbacks are recoverable
COM lifecycle operations, not an excuse for arbitrary or empty listener
interfaces. This audit establishes the observed factory/constructor path;
it does not prove that every possible array writer has been identified.

## Registration and object layout

Renderer primary virtual `+24h` resolves to `00b27c20`; `+28h` resolves to
`00b27cf0`. The former is absent from the current function snapshot, so its
complete 204-byte body was read as raw assembly without Ghidra mutation.
It receives renderer ECX, consumes no stack arguments, and returns the new
wrapper pointer in EAX. It enters the optional renderer guard, allocates
`14h` bytes, constructs through `00b5fe90`, and appends the pointer to the
renderer array. Capacity grows through `00b22530`; the append itself writes
the pointer and increments count with no intrusive retain. Even its null
allocation path reaches the append; a safe typed allocation failure path is
an explicit interface difference from the native unchecked result.

Constructor `00b5fe90` receives object ECX and returns with ordinary RET.
It sets intrusive count `+4h=1`, field `+8h=1`, field `+ch=0`, COM pointer
`+10h=0`, and concrete vtable `00d62ad0`. It obtains the renderer singleton's
device via `00b1fef0`, then calls device virtual `+1d8h`:

```text
CreateQuery(9, &wrapper.query)
```

D3D query type 9 is OCCLUSION. `00b1fef0` is exactly the seven-byte borrowed
getter `MOV EAX,[ECX+1a10h]; RET`; it does not retain the device. The constructor
ignores CreateQuery's HRESULT. This audit does not assign recovered names to
the `+8h` and `+ch` query-use state fields beyond their observed initialization.

## Concrete callbacks and destruction

Vtable `00d62ad0` entries are:

| Offset | Address | Established scope |
|---|---|---|
| `+0` | `00bd30e0` | Existing intrusive lifecycle entry; not newly audited here |
| `+4` | `00b5fe40` | Deleting destructor adapter |
| `+8`, `+c`, `+10`, `+14` | `00b5fc30`, `00b5fc60`, `00b5fca0`, `00b5fce0` | Query-use methods outside this reset audit |
| `+18` | `00b5fe20` | Release query for reset |
| `+1c` | `00b5fe60` | Recreate query after reset |

Release `00b5fe20` takes wrapper ECX, no stack arguments. If COM query `+10h`
is nonnull, it calls Release once and clears the member. It does not change
`+8h`/`+ch`, unregister the wrapper, inspect readiness/lost state, acquire a
guard, or issue a balanced AddRef/Release pair first.

Restore `00b5fe60` also takes only wrapper ECX, no stack device argument.
It fetches the current renderer singleton's device through `00b1fef0` and
calls `CreateQuery(OCCLUSION, &wrapper.query)` directly. It has no guard,
readiness/lost gate, old-pointer cleanup, or HRESULT handling. The intended
reset sequence supplies an empty COM owner after release. A typed API can
require that precondition and report failure, but should not invent query
results or silently substitute a dummy query when unsupported.

Full destructor `00b5fda0` releases/nulls the query first, then calls renderer
virtual `+28h` with this wrapper pointer. It subsequently tears down the base.
Deleting adapter `00b5fe40` calls that destructor, then frees storage if its
stack flag has bit zero set; it returns the original object address and
consumes one stack argument (`RET4`).

Unregister `00b27cf0` uses the optional renderer guard around
`00b25290(&argument_pointer)` on renderer `+19a0h`. The removal helper searches
for the first equal pointer, replaces it with the final entry if necessary,
and decrements count. Missing entries leave the array unchanged. It does not
retain, release, destroy, or preserve order. Thus factory append and destructor
removal form an observed non-owning list relationship; membership must not be
mistaken for an added intrusive reference.

## Reset ordering and mutation

General release `00b262c0` invokes these listeners after releasing default
depth and the four fixed color-owner slots, but before the generic texture
array and offscreen surface array. At `00b263a5..00b263c9`, it starts index
zero, checks signed count positive, reloads the array base before every entry,
invokes listener `+18h`, increments the index, and compares against the current
signed count. Entries are not null-checked or temporarily retained.

Restore `00b23b10` calls listener `+1ch` **last**, after default surfaces,
generic textures, and offscreen surfaces. Its loop at `00b23c26..00b23c49`
has the same signed-index/base-reload/count-reload structure. It passes no
device argument because each concrete query callback resolves the device
itself. The surrounding release has an optional guard; restore has no internal
guard and relies on its caller's lifecycle coordination.

These listener loops differ from the raw-cursor loops used for textures and
surfaces: reloading the base by index can accommodate reallocation while the
object lifetimes remain valid. That does not make arbitrary mutation safe.
Swap-with-last removal can skip the swapped-in listener, append can extend the
current traversal, and destruction during callbacks invalidates borrowed
lifetimes. No stable-order, snapshot, deduplication, or arbitrary reentrancy
contract is established. A typed implementation should require stable list
membership and object lifetimes during the reset batch rather than claim a
new mutation-safe generic event system is native behavior.

## Next implementation boundary

A concrete query owner with these two release/recreate callbacks, constructor
initialization, and explicit borrowed registration/removal is sufficient for
this observed listener class's reset participation. Use a real D3D9 query and
existing renderer synchronization; report unsupported/failed creation instead
of fabricating callbacks or visibility results. The remaining query issue,
result polling, object retention, and renderer use require their own evidence
before claiming functional occlusion handling or a complete game reset.

## Verified bytes

Each batch verified project `bsp`, program `/battlestationspacific.exe`, x86
image base `00400000`. All following complete bodies/data matched installed
PE bytes exactly:

| Address | Bytes | SHA-256 |
|---|---:|---|
| `00b27c20`, raw factory | 204 | `5c0d5e18b92c7fec024c40ce06c8992c4f8a5cd973363205e0faa079e9e65eb0` |
| `00b27cf0`, unregister | 78 | `8d7d0a784cdfcfa8e7ae2b3f2426ba699ccc2909e92ff21a70bb687671c6d791` |
| `00b5fe90`, constructor | 111 | `469ab83ca834099c95a66eaf40fced39235cc366516f045a472137cfb587d4f0` |
| `00b5fe20`, reset release | 27 | `9c45583ceed898e5c04ef5b8a6a7100afb71a45a97fc601bd4e1e7751f315ef2` |
| `00b5fe60`, reset restore | 33 | `31eeaea774758bf37204434d865460bfd1c18f2746cec2e899b08dd421b2269c` |
| `00b5fda0`, destructor | 116 | `538b5858e3b9e3bd803e78808766d60e20978094ac61257f08d5c635bd115808` |
| `00b1fef0`, device getter | 7 | `8e2f8eda5033d3403561544aec8eccd01530285ea93dcd284ef49a1308e871a6` |
| `00b25290`, pointer removal | 103 | `24d2aab156d7889ea7543c821483002b5f93b654671b47704e8253690ef1b8d0` |
| `00d62ad0`, eight vtable entries | 32 | `a30b2ff83902dc151ecf13b91fde3cc5ea8af297cd98a50488e62891c2ef24ad` |

This subtask changed only this document. It made no code, configuration,
Ghidra annotation, build, commit, or runtime-validation changes.
