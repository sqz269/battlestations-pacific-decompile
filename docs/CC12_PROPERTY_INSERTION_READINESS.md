# Raw property insertion readiness (CC12)

Addresses: 008F33F0 008F28F0

Both complete Original bodies are established, but both remain **NOT_READY**
for whole current raw Source registration. The wrapper's missing direct
provider is `008F28F0`; that provider lacks raw lookup `0043B8B0` and the
actual old-record scalar lifetime required on replacement. A fresh-key-only
implementation would omit an ordinary native path and is not promoted.

Current raw bag construction has advanced since the earlier iterator audit:
whole `008F41A0` Source is now present on main `0fa8ebb1`. Its code and header
were inspected as a dependency without claiming or expanding that address.
This worktree was refreshed from main before the final Source inspection.

## Complete native coverage

| Entry | Inclusive end | Exclusive end | Bytes / instructions | Calls | Coverage |
| --- | --- | --- | --- | --- | --- |
| `008F33F0` | `008F348A` | `008F348B` | 155 / 46 | 4 direct | complete |
| `008F28F0` | `008F299B` | `008F299C` | 172 / 65 | 4 direct + 1 indirect | complete |

Neither body has a listing gap or requires a Ghidra repair. Complete saved
current pseudocode, assembly and body bytes matched the installed PE:

- `008F33F0`: `546af6bc24638da2025274f378467e5e8ef4be515a2e80b0892b4ddc653cdffc`.
- `008F28F0`: `352616a3d3506a800c42a875c8ea4d86ccdeb788250d77a8dcec8abee5520878`.

Every live batch used `bsp.py`, which verifies project `bsp`, program
`/battlestationspacific.exe`, x86 language and `00400000` image base.
Configuration pins `C:/Users/sqz269/bsp.gpr` and the original installed PE.
No Ghidra, shared metadata, CMake, ledger or export mutation occurred.

## Wrapper contract and temporary ownership

`008F33F0` receives ECX = actual `114h` bag and stack arguments
CString key, actual `38h` incoming record. It returns the original bag in EAX
with `RET 8`, preserves ESI/EDI, and restores its normal x86 FH3/FS linkage.
EBX/EBP are untouched; volatile registers/flags otherwise remain unspecified.

It builds an actual fresh eight-byte stack key header with
`008F3415 -> 0041E870`. Despite its older descriptive “Assign” label, the
callee is a constructor: it clears length/data before using the CString.
The two initially unspecified stack words are therefore valid fresh storage.
`008F342F -> 008F28F0` gets ECX = bag `+4`, that temporary header, and the
actual supplied record. The wrapper marks its unwind state around this call.

After insertion returns, the wrapper releases the temporary key's genuine
pooled allocation when its data pointer is nonnull. At `008F3448..008F344E`
it pushes unused `1`, length `+1`, and data. `008F344F -> 00419CC0` consumes
**no arguments** and returns the actual string pool in EAX. `008F3454` puts
that EAX into ECX; `008F3456 -> 00BD1510` consumes the three pending words
with `RET 0C`. The pseudocode's apparent three-argument getter is misleading.

Only after this release does the wrapper stamp record `+30 = bag`.
If record `+34` is zero, it takes bag `+10C`, increments that counter, and
writes the old value to the record. Existing nonzero ordinals remain intact.
The first ordinal is zero, also the unstamped sentinel; no stable unique-ID
claim or added counter guard follows. The wrapper allocates no record and
rewrites no record type/profile/value. Original exception/failure identity
is not reproduced or tested in this audit.

## Actual map provider, hit and miss

`008F28F0` receives ECX = actual embedded `108h` map, an actual eight-byte
query header and an incoming mapped-record DWORD on the stack. It returns
the actual current count in EAX with `RET 8`, preserving EBX/ESI/EDI.
It performs no SEH registration itself. It first calls `0043B8B0` at
`008F28FF`, passing the query plus an out-bucket address. The query pointer
is retained in EBX; the first argument's stack cell becomes the bucket
output. This does not overwrite the query header. Lookup's body and complete
callee ABI were not newly expanded; its whole current raw Source is absent.

On a hit, it reads the existing node `+8` record. When that pointer is
nonnull, it loads the actual record profile and slot 0, pushes flags `1`,
and executes `008F2917 CALL EAX`. Only after that actual scalar returns does
it zero node `+8`, then publish the incoming record there. Existing key,
chain, bucket and count remain intact. There is no identity guard: replacing
with the same owning record still destroys the old pointer first. A valid
Source admission must retain genuine disjoint replacement ownership; it
cannot silently turn this path into a pointer assignment.

On a miss, `008F2930` sets ECX to the distinct actual record-node pool
`00E175B0`; `008F2935 -> 004E7C00` obtains a genuine `14h` slot. The older
“EnumNodePool” descriptive label names a shared allocator body, not the
specific enum owner `E175E8` or symbol owner `E17578`. Only node `+0/+4`
are cleared before owning-key construction. If slot differs from query,
`008F295C -> 0041DD40` resizes its actual key header with the query length
and preserve flag `1`; a nonempty query then copies the destination's actual
length bytes through `008F2971 -> 00BF7680`. Resize supplies the terminator.
The key remains an owning copied allocation rather than a borrowed pointer.

Publication order is exact:

1. Store incoming mapped record at node `+8`.
2. Store the actual previous bucket head at node `+0C`.
3. Publish the new node into that actual bucket head.
4. Increment actual map count `+4` and return the reloaded count.

The node's `+10` page identity is allocator-owned and untouched. Genuine
successful native insertion therefore produces the node/key/next/head/count
shape consumed by the previous clone and iterator evidence. A null slot
still reaches unguarded writes/calls and can fault; no null/default/rollback
policy is invented. Concurrent mutation, invalid bucket output, forged pool
slots and arbitrary key/node aliasing are not admitted.

## Actual replacement scalar binding

The general replacement call stays indirect. The previously complete actual
type-6 constructor `008EF780` publishes `CE89D4` in a real `38h` record;
current live/installed `CE89D4[0]` bytes are `30 67 4e 00`, selecting
`004E6730` for that admitted profile. The four-byte slot hash is
`3de3efdb8a215744a620e6c6baedd08c74142af5b8d6706af395ff9cdcd754f4`.
The actual chain `004E6730 -> 008F0DE0 -> 008F0640` still lacks a complete
current raw lifetime provider, including real nested/map/type ownership.
Other possible old profiles remain unresolved. The array release leaf and
empty raw bag constructor do not close this replacement path.

## Fresh Source provider readiness

The ten inspected Source/header files match live main byte-for-byte. Existing
provider availability was checked without replaying tests or old fixtures:

| Original dependency | Current actual raw Source status |
| --- | --- |
| `0041E870` constructor | available raw-header overload with genuine `NativeStringRawPoolContext` |
| `00419CC0` getter | available actual publication/raw-manager overload |
| `00BD1510` sized release | available actual pool/rings/disable cell/OS section |
| `004E7C00` node allocation | available conditional whole ordinary raw `14h` allocator on a genuine initialized sibling pool and same allocator-list domain |
| `0041DD40` resize | available actual eight-byte header/raw-pool overload |
| `00BF7680` copy | real overlap-safe host `memmove` boundary already used by the genuine providers |
| `0043B8B0` lookup | whole current raw Source absent; unexpanded here |
| actual old record slot 0 | whole genuine scalar/type/nested Source lifetime absent |

The new raw `008F41A0` constructor supplies real writable `114h` empty
storage, including count/64 heads/ordinal/owner. Its literal Original profile
DWORDs are phase markers, not Source vtables to dispatch. Its EDX formal is
unused so the owner remains the one stack DWORD. Its Source admission is
preserved independently from map insertion and lifetime closure.

Raw string and slot services are real memory providers. Their availability
does not manufacture Original static `E175B0` publication, a whole raw lookup,
record producers, class/global/private-EH identity, or replacement lifetime.
Existing semantic property models and abstract host methods are not accepted
as substitutes, nor is an unrelated enum owner installed into a bag.

## Caller proof, verification and next work

The prior sealed complete clone body has ECX = actual new bag at
`008F426A`, actual cloned record EAX pushed at `008F4268`, and actual node
key ESI pushed at `008F4269`, followed by `008F426C -> 008F33F0`.
Its complete body hash is retained. All 21 wrapper xrefs and the sole lower
provider xref are saved; other caller bodies were not newly expanded.

The existing verifier checked nine exact direct call-site/native/containing-
function rows with zero failures. `008F2917 CALL EAX` is pinned separately without an
invented static target row. No C++ changed, so no build, native/source
execution, fixtures or new tests were run.

There is no ready whole implementation packet. The next bounded read-only
packet is the missing actual lookup `0043B8B0`, with its direct hash
`0043B760` only if separately agreed. It must establish whole body/ABI and
genuine current key/map/node admission before a raw Source recommendation.
The existing record scalar chain must remain explicit. Fresh-key-only,
null-old-record-only, semantic copies and opaque release callbacks cannot
close either owned routine. Factory/world/game and binary/class ABI remain
outside this audit.

Ignored raw evidence is sealed under
`J:/PROG/battlestations-pacific-decompile-cc12_property_payload/local/cc12_property_insertion_readiness/`:
complete owned pseudocode/listings/bytes, PE/flow receipts, actual profile
slot, all xrefs/callees, exact call instructions/body ranges, prior sealed
clone/type-6 producer evidence, current dependency lookups, fresh Source
search/hash receipt, and the artifact manifest pinned in the tracked report.
