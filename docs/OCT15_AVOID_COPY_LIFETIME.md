# Avoid-zone copy construction and owned-storage destruction

Addresses: `0041A030`, `0041A1E0`. Packet `oct15_avoid_copy_lifetime`.

`0041A030` has a hazardous native count schedule. A nonempty source with N
records produces a destination with an opaque N-slot prefix, followed by N
new record pointers, and count 2*N. The routine establishes no valid record
identities for that prefix. Its result cannot be treated as an ordinary safe
owned polygon or passed to a traversal/destructor without independently
satisfying that operation's readability and ownership preconditions. The source
retains the observed behavior; it does not zero-fill, repair the count, invent
prefix identities or repair allocation-failure leaks. `0041A1E0` is independently
callable for valid owned native storage.

| Address / proposed name | Body inclusive | Native ABI | Coverage |
| --- | --- | --- | --- |
| `0041A030` / `BSP_AvoidZone_CopyConstructWithOpaquePrefix` | `0041A030..0041A1DE` | ECX destination; stack source; EAX destination; `RET 4` at `0041A1DC` | complete normal flow and allocator-exception cleanup; FH3/SEH ABI excluded |
| `0041A1E0` / `BSP_AvoidZone_DestroyOwnedStorage` | `0041A1E0..0041A1FE` | ECX zone; no stack arguments; `RET` at `0041A1FE` | complete |

The existing `AvoidZoneNativeStorage`, `ShipAiPathLateralRecord` and
`AvoidZoneAllocationAccess` types are reused. Producer `0041CCD0` initializes
the three-word pointer/count/capacity header, layer at `+0Ch`, entity at `+10h`,
and bounds at `+14h..+20h`. Its record allocation is 24h bytes; the read producer
branch writes x/z, six zero derived fields, and `-1` at `+20h`. Those stores and
the concrete headers' existing size/offset assertions reconcile the layout
before this consumer's implementation. This packet introduces no overlay.
The entity word is shared by copying its representation, without ownership or
reference-count operations. New corner records have distinct allocation
identities; the source records and geometry are not mutated.

Both complete packet listings were read after pseudocode. Live queries verified
project `bsp`, configured at `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`, language and image base. Live caller queries found
no callers for either address. The listings contain 156 and 12 instructions,
respectively, with no listing gaps and explicit returns after the returning
free calls. The actual free thunk/body and `00412DC0` continuation were also
read; no no-return repair or Ghidra mutation was needed. Names are hypotheses;
old names are `FUN_0041a030` and `FUN_0041a1e0`.

The copy first clears the destination header without releasing prior storage,
copies the entity/layer words, and copies min-x through x87. EH state 0 becomes
active at `0041A070`, before the remaining three bounds. A fresh destination is
required. Source count, pointer array and records must stay readable and stable
across allocation callbacks; successful accesses must fit their actual buffers.
Allocator callbacks are the existing required actual services, with their real
new-handler/retry/throw and matching-free contracts.

`0041A0E1` sets count to source N after reserving N slots. The first clone is
allocated before the capacity check; count==capacity then grows capacity to
`2*capacity+2`. The copy moves the original N untouched slot representations,
frees that first array, and appends at index N. For an ordinary representable
nonempty successful input, final count is 2*N and capacity is 2*N+2. Empty input
retains a null pointer/zero count/zero capacity. Source capacity is not copied.
Unsigned growth wraps, the `new > old` guard is retained, and 4*capacity overflow
requests `FFFFFFFFh` bytes. No guard repairs an insufficient/wrapped allocation.
Raw installed `.text` disassembly independently confirmed both the count store
at `0041A0E1` and growth/guard at `0041A145..0041A15A`; this is not solely a
pseudocode interpretation.

Opaque pointer slots use representation-safe four-byte copies, matching native
MOV words without evaluating indeterminate C++ pointer values. All nine record
words instead retain FLD/FSTP at `0041A107..0041A13C`; bounds likewise retain
their four x87 loads/stores. Signaling NaN quieting and the ambient x87 control
state therefore matter. Whole-record memcpy or default initialization would
change that behavior. Complete FP exception/status and native stack/register
identity equivalence are not established.

| Function | Call site | Callee | Body-backed contract |
| --- | --- | --- | --- |
| `0041A030` | `0041A0A4`, `0041A16C` | `00BF55BE` | Array-allocation thunk to `00BF681B`; unsigned saturated size; caller cleanup `ADD ESP,4`. |
| `0041A030` | `0041A0F8` | `00BF681B` | Actual 24h record allocation; malloc/new-handler/retry/throw; caller `ADD ESP,4`. Native null branch is retained. |
| `0041A030` | `0041A0D5`, `0041A199` | `00BF6989` | Returning array free; thunk through `00BF65AC` to `00BF9DC8`; caller `ADD ESP,4`. |
| `0041A1E0` | `0041A1E3` | `004167C0` | Existing clear: free every counted record through actual free service and set count=0. |
| `0041A1E0` | `0041A1EF` | `00BF6989` | Free nonnull pointer array, `ADD ESP,4`, then zero only its pointer. |

EH evidence is separate from direct-call attribution: handler `00C5E138` loads
FuncInfo `00D84218` and jumps to `00BF6B43`. Its header begins
`22 05 93 19 01 00 00 00 10 42 D8 00`: FH3 magic, one state, unwind map
`00D84210`. That map is `FF FF FF FF 30 E1 C5 00`, mapping state 0 to -1 with
action `00C5E130`. The action loads the destination from `[EBP-10h]` and jumps
to `00412DC0`. The complete latter body frees/zeros only the pointer buffer.
The C++ catch reproduces this allocator-throw cleanup and leaves count,
capacity, metadata, completed clones and any just-allocated clone alone. It
does not promise the original CRT exception object, FH3 frame or SEH behavior.

The destructor reuses existing `004167C0`, then frees and nulls its pointer
buffer. Capacity and all trailing metadata survive. The convenience
`avoid_zone_release_owned_storage` also clears capacity, so it is deliberately
not used. Every counted slot supplied to the destructor must independently be
a valid uniquely releasable record pointer or actual CRT-accepted null, and its
array must match the supplied free service. A nonempty copy does not establish
these conditions for its opaque prefix.

Validation: full `./scripts/build.ps1` passed all three existing suites
(`reconstructed_math`, `native_math_differential`, `tool_tests`), 3/3, in 7.20
test seconds. Log: `local/oct15_avoid_copy_lifetime_build.log`. The native math
suite covers existing seeds, not these lifetime routines. Call-report and diff
checks are recorded in the JSON report. No new test, deterministic replacement
prefix, packet-native differential fixture or gameplay validation was added.
