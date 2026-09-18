# Native game container cleanup (R125)

Addresses: `004BF8E0`, `004BF930`, `004C2CE0`, `004C4A50`, `004C4B40`,
`004C8180`, `004C86A0`, `004CB220`, `004CF3F0`; parent `004DCF90`.

## Result

Nine complete normal bodies (878 bytes) now implement raw list, pointer-block
and reference-array cleanup. Seven concrete `NativeGameLifetimeCalls` defaults
replace required address-only bindings at fourteen parent call sites. The parent
retains one `operation.containers` record for observations of partial work.
Descriptive names are hypotheses, not recovered symbols.

| Entry | Bytes | Recovered behavior |
| --- | ---: | --- |
| 4BF8E0 / 4C2CE0 | 70 each | `{count0,head4,tail8}`; capture current head, reload links, unlink, decrement current count, free node; repeat against current count. Payload retained; empty input preserves head/tail. |
| 4BF930 | 101 | Same counted list, with captured node+8 scalar delete through slot0/flags1, then clear node+8 after the callback before reading links. |
| 4C4B40 / 4C8180 | 72 / 5 | `{opaque0,head4,count8}`; reset sentinel links/count, capture next before node free, compare against current head, free current sentinel and clear head. 4C8180 is a JMP thunk. |
| 4CF3F0 | 86 | Sentinel node walk, retaining the sentinel. Unsigned node20 >=16 frees nodeC; capture next before that call, then set20=15,1C=0 and only byteC=0 before node free. |
| 4C4A50 | 104 | Drain depth10 and clearC only on transition to zero. Capture count8, free slots in reverse while reloading base4; free current base, clear4/8. |
| 4C86A0 | 244 | Signed requested<1 clamps1; signed capacity gate. Allocate wrapped requested*4; copy and retain references ascending before releasing old cells ascending. Each loop rereads count/base. Free current old buffer before publishing captured replacement/capacity. |
| 4CB220 | 126 | Reserve if signed requested exceeds capacity; zero newly exposed slots ascending. Shrink by decrementing current count before release, retaining captured cell/payload across callbacks; clear captured cell afterward. Finally publish requested count. |

All entries receive the owner in ECX. Reserve/resize receive one stack DWORD and
return with `RET4`; other bodies have no stack arguments and `RET` (or the thunk
JMP). Atomic and virtual calls remain explicit boundaries. Source interfaces are
not original binary entry points or a port of native FH3/SEH cleanup.

The `4C1950` and `4C1A40` producers establish 24h and 0Ch self-linked sentinel
allocations respectively. The source describes consumed fields without inventing
a new STL class. Existing `unit_list_clear_004bf8e0(std::vector<UnitRef>&)` remains
a separate semantic projection; it does not provide this raw-header behavior.

## Analysis repairs

Eight returning-call gaps totaling 70 bytes were restored across five functions.
`4C4B40` also had a truncated function body ending at `4C4B7B`: the verified
12-byte final-free tail ends with `RET` at `4C4B87`. The function was recreated
with previous metadata preserved. Its `4C8180` thunk survived and was read back.
Final flow auditing found no returning-call gaps. Three unreachable padding bytes
after `4C4A5B`'s unconditional jump were intentionally left alone.

Prior values, repairs, body ranges, live/PE bytes, call rows and refreshed names
are recorded in `reports/native_game_container_lifetime_r125*.json` and the
immutable local evidence archives referenced by the main report.

## Validation

Strict MSVC Win32 build and all three existing CTests pass. The copied-original
fixture verifies all 878 bytes against the live program and installed PE, then
relocates the nine bodies with explicit allocator/free and Win32 IAT boundaries.
It exercises seven actual parent defaults, real CRT/Win32 atomics, and the existing
raw sentinel producers. Payload virtual methods are observable fixture methods.

The 71 paired cases match 315 observations and 46,400 normalized bytes. They cover
empty/populated lists, null payloads, scalar callbacks replacing fields/count,
sentinel substitution, inline/heap node capacity boundaries, captured next links,
reverse pointer-block frees and reloaded bases, reference shrink/growth, signed
reserve clamps, and reference values 0/1/2/FFFFFFFF. A shrink callback changes the
base/count while both lanes still clear the captured old cell. All remaining
fixture allocations are explicitly drained after observation.

Two source-only failure cases retain the expected graph: allocation failure leaves
the owner/cells unchanged; old-buffer free failure leaves the old cells cleared
and the replacement populated but unpublished. Progress is borrowed diagnostic
storage; it does not own allocations or perform rollback. These direct helpers
are repeatable operations, while the encompassing game lifetime remains one-shot.

The separate parent schedule comparison passes 52 paired cases, 3,537 snapshots,
141,402,076 normalized bytes and four source failure/replay cases. It controls
container calls and checks that all seven bindings receive the same retained
progress record; the body fixture supplies their concrete implementation evidence.

## Remaining work

Resolve remaining game destructor dependencies and actual payload virtual bodies,
then bind the raw game owner into the application with a matching lifetime domain.
The ordinary executable still does not reach these bodies, so no application run
was repeated for this packet. Arbitrary invalid graphs, overflow-sized allocation
stress, native exception cleanup, binary ABI compatibility, concurrency, real
session ownership and gameplay remain unvalidated.
