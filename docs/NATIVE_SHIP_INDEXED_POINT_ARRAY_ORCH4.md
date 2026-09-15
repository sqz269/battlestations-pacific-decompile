# Native counted point/index array reserve

Address: `00829180`.

## Coverage and interface

| Routine | Coverage | Source |
| --- | --- | --- |
| `00829180–00829203` | Complete ordinary body, 132 bytes, 52 listed instructions | `reserve_native_ship_indexed_point_array_00829180` |

The source accepts the actual Win32 header through `void*`, plus a signed
32-bit requested capacity. The native entry takes that header in ECX, one
stacked signed capacity, and ends with `RET 4` at `00829201`. It has no stable
return value. This is a new C++ interface, not a native ABI/FH3 bridge.

The live project was `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. The worker made no Ghidra mutations. The proposed
descriptive name `BSP_ShipIndexedPointArray_ReserveCapacity` is a hypothesis,
not a recovered symbol; the parent owns annotation and saving.

## Storage established by writers

The header contains DWORD data at `+0`, signed count at `+4`, and signed
capacity at `+8`. Elements occupy `10h` bytes: binary32 coordinates at
`+0/+4/+8`, followed by a raw DWORD index at `+Ch`. The source uses those
actual words; it introduces no competing owning container or struct layout.

The complete small producer listings `00829CD0` and `0082A590` use count
times `10h` plus data to locate an appended record, transfer three floats and
one DWORD, then increment count. The ship producer at `00830FD0–0083104C`
uses class header `+678h/+67Ch/+680h` and stores EBX as the index at
`00831042`. Register-write filtering across the complete ship listing shows
EDI is the class argument from `0082FE4F`, and the relevant EBX loop starts
at 1 (`00830F22`) and increments at `00831049`. That loop supplies the model
lookup index before writing the record. These are producer excerpts; no
additional caller reconstruction or caller ownership is claimed.

## Native schedule

1. Clamp requests below signed 1 to 1. Return when current signed capacity
   is at least that value (`00829185–00829195`).
2. Allocate `uint32(capacity) << 4`, retaining DWORD wrap. The allocator call
   at `0082919E` receives one stack argument; cleanup is `ADD ESP,4` at
   `008291A5`.
3. While the signed loop counter is below the current signed count, skip a
   zero destination address; otherwise reload the current source data and
   perform `FLD/FSTP` at offsets 0, 4, and 8, in that order, then the raw
   DWORD index read/write (`008291CC–008291DF`). Each coordinate is stored
   before the following coordinate is loaded. This preserves x87 handling
   of signalling NaNs, denormals, control/status and ordered memory effects.
4. Increment the counter and destination in DWORD arithmetic, rereading
   count at the loop test (`008291E2–008291EB`). Count itself is unchanged.
5. Reload and free current data at `008291EE–008291F1`; cleanup at
   `008291F6` is `ADD ESP,4`. Only after free returns, publish replacement
   data at `008291F9`, then capacity at `008291FB`.

The former listing gap `008291F6–008291FE` is present in the current live
listing. Its bytes are `83 c4 04 89 1e 89 6e 08 5b`, independently matching
the installed PE. The parent's repair receipt is
`reports/vehicle_binding_dependency_flow_repairs_orch4.json`.

The allocator and free thunks were read: `00BF55BE` jumps to `00BF681B`, and
`00BF6989` jumps to `00BF65AC`. The reconstruction uses the existing
`singleton_lifetime_allocate/free` source CRT boundary. Allocation retries
the host new handler and otherwise throws; original CRT heap identity,
native bad_alloc object identity, errno behavior and failure equivalence
are outside this packet. No fabricated allocation fallback is introduced.

## All current call sites

Live xrefs returned five calls; `ghidra proto` confirmed every containing
function. All five argument setups were read.

| Site | Containing function | Requested value and receiver |
| --- | --- | --- |
| `00829CE8` | `00829CD0` | Current header, signed max of doubled capacity and 1, when count equals capacity |
| `0082A5A8` | `0082A590` | Same append policy |
| `00831013` | `0082FE30` | Class+678h header, same append policy |
| `0095709B` | `00957090` | Current header, 0 only when capacity is signed-negative |
| `0095863B` | `00958630` | Current header, 0 only when capacity is signed-negative |

Doubling wraps in a DWORD before the signed comparison. The callee's
`RET 4` accounts for each pushed request. The last two callers demonstrate
that the minimum clamp is part of the reserve contract, not merely a
caller-side policy. The cached snapshot/index also lists `00963600`, but
current live xrefs omit it and that destructor's live callees omit
`00829180`. It is recorded as a stale snapshot edge, not a sixth live call.
`00958630`'s unrelated post-free continuation was not reconstructed.

## Validation and limits

- All eight `ghidra_export.py verify-seeds` comparisons matched disk.
- All 132 routine bytes matched the live Ghidra bytes and installed PE;
  SHA-256: `d320ea7f6a1418ac5ae64f60db7b21bd84f0bb0750938bdca08e8dabd176ff69`.
- The focused ignored `local/ship_indexed_point_probe.cpp` compares the
  original bytes, with only the two CALL operands rebound, to the compiled
  source. It passed 120 pairs: ten cases across twelve masked x87 precision
  and rounding settings. It compares the whole fixture backing buffer,
  header, allocation/free observations, x87 status/control, and a preserved
  incoming x87 stack sentinel. Cases include signed no-grow/minimum paths,
  nonpositive counts, wrapped byte sizes, signalling/quiet NaNs, infinities,
  denormal and signed-zero inputs, raw index bits, and publication order.
- Twelve of those comparisons deliberately overlap fixture source and
  destination to expose reordered transfers. This is an injected-boundary
  diagnostic, not a claim that the canonical allocator returns overlap.
  The fixture does not execute original CRT allocation/free, OOM handling,
  unmasked exceptions, or a game class-loading path.
- Build and report verification receipts are recorded in the JSON report.

This helper is reconstructed and has bounded original/source fixture
evidence. It is not a drop-in binary replacement and does not establish
gameplay, live class parsing, hardware-fault recovery, original CRT failure
behavior, or native unwind equivalence. Every reached address must be valid;
native overflow, signed header values, and destination skips are retained.
