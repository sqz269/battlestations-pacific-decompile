# Dyn flag-selected endpoint allocation

This packet reconstructs complete normal `00C55A60..00C55D26`: **711 bytes,
207 listed instructions, 25 branches and nine normal calls**. The public
descriptive name is `bsp::cc16_dyn_endpoint_allocate::allocate_endpoint_00c55a60`.
It operates on the actual untyped manager and composes the real
`cc16_dyn_endpoint_init::initialize_endpoint_00c4ce60` with the same actual CRT.
No native class, vtable, allocator, fallback initializer or state owner is invented.

The implementation uses ordinary C++ with volatile raw-word access. It preserves
native storage/callback epochs, not original machine instructions or register/
frame identity. The two flag-selected pool paths share a source helper with
different actual offsets; each depleted selected pool makes its real 68000-byte
malloc request. Three vector-growth paths share their actual live-field schedule.
Root still owns source registration, integrated build and actual caller binding.

## Verified target and ABI

Every live query used the verifying `bsp.py ghidra` client for project `bsp`,
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, language
`x86:LE:32:default`, image base `00400000`. All three contiguous assembly pages
were read; two bounded fresh byte reads cover all 711 bytes and match the
installed PE. Native-span SHA-256:
`8f96d8eaf4f36fcb710d8f5bf1c86cf4d6ad36da2f7ecccd5e1ed3a0bcc5ea60`.

Native ECX is manager; its three stack arguments are uint32 token, bounds
pointer and the low byte of a third word. Return is EAX = captured endpoint,
`RET0Ch`. EBX/EBP/ESI/EDI are saved/restored. Normal frame allocation is 20 bytes
plus four saved registers; allocator sites have ESP entry-minus-40, while the
initializer has ESP entry-minus-44 and consumes its two arguments with RET8.
The one-argument allocator calls retain caller `ADD ESP,4`. Private source
helpers use ordinary C++ cdecl arguments, not a private native register adapter.
A complete native CFG
audit reaches all 207 instructions, maps all 25 branches and verifies every stack
join and the final RET0Ch balance. There are no omitted listing gaps or native
FP/SSE instructions.

| Flag | Free head | Page vector data/count/capacity | Live count | Sentinel | Tail |
| --- | --- | --- | --- | --- | --- |
| nonzero | `104h` | `F8h/FCh/100h` | `190h` | `14Ch` | `188h` |
| zero | `68h` | `5Ch/60h/64h` | `F4h` | `B0h` | `ECh` |

All offsets are actual manager byte offsets; these names describe observed use,
not recovered class identities. The pending vector is independently at
`194h/198h/19Ch` for data/count/capacity.

## Preserved normal schedule

For the selected layout, test its live free head. If empty, call actual
`BF9F1A` malloc with 68000 (`109A0h`) bytes. Write 999 free links, starting at
pool+40h: each link holds current-link-address+4, and the next link advances
44h. Store zero at pool+1099Ch. This creates 1000 actual 44h slots; there is
no null-allocation branch before those writes.

Capture page capacity, then capture the count used for the equality decision.
Publish the new pool as free head after those reads and before any subsequent
new/free callback. If the captured values were equal, publish wrapping
`2*capacity+2`, then call actual `BF55BE` new with wrapping capacity-times-four
bytes. Retain its returned pointer. Copy entries using a fresh live count guard,
per-iteration data reload and live unsigned count reload. Suppress only a zero
computed destination; do not turn that into a base/null failure guard. Afterwards
reload current old data, conditionally free it through actual `BF6989`, and only
then publish retained new data. A callback can mutate the fields later read by
the native sequence; the source does not cache those fields across calls.

Append the captured pool: load current count before current data, compute the
wrapping slot address, conditionally write that nonzero computed address, and
read/increment the count again after the write. None of the malloc/new/free
results is repaired, defaulted or rolled back.

Reload selected free head and capture endpoint+40h as its next free slot.
Increment selected live count, publish the captured next head, then link the
endpoint in this exact order: endpoint+40h = manager+sentinel; endpoint+3Ch =
current tail; reload tail; old-tail+40h = endpoint; manager tail = endpoint.
The duplicate tail read remains explicit, including alias-sensitive effects.

Retain that endpoint and, if nonzero, call the real initializer once with
`(token, flag, endpoint, bounds, same CRT)`. Native ECX/DL and stack(endpoint,
bounds) are represented by that new C++ interface. The initializer's return
value is ignored, matching the caller's retained EDI. Bounds remain an actual
borrowed pointer until this call; no early six-word snapshot is introduced.

After normal initialization, independently load pending capacity/count for its
equality guard, apply the same growth schedule if full, and append the captured
endpoint using fresh pending count/data reads. Increment count after the slot
write and return the captured endpoint. Original count, capacity and byte/index
arithmetic wrap at uint32. The source adds no capacity, ring, pointer, extent,
allocation-success, initialized-object or count-consistency guard.

## Native outer unwind scope

The handler at `CC83DB` loads FuncInfo `E04290` and jumps to the actual
`__CxxFrameHandler3` entry `BF6B43`. Fresh bytes identify magic `19930522h`,
maxState 1 and unwind map `E04288`: `{previous=-1, action=CC83CA}`.
`C55C67` enters state 0 around the optional initializer, and `C55C84` restores
state -1 before pending-vector processing.

The 17-byte action reads establisher-frame-relative words at EBP-14h and EBP-10h,
pushes both, calls `00401130`, removes eight bytes, and returns. Live and PE
bytes independently confirm **00401130 is exactly `C3` (RET)**. Its actual body
returns; it is not a no-return or missing destructor dependency. Regardless of
the two captured values, that action performs no pool refund, unlink, object
destruction or allocation release. No replacement/no-op callback is supplied.

The C++ allocator adds no outer catch/cleanup or copied native registration
frame. Its real initializer provides its own recovered vector cleanup and
rethrow. Thus ordinary C++ provider exceptions leave the outer pool/link/count
mutations in place, and do not reach the pending append, without inventing an
outer rollback. Original FH3/SEH, exception matching, nested cleanup failure,
fault PCs, frame layout and propagation through a native ABI remain unproven.
This is not a claim of original binary exception equivalence.

## Independent review of the real initializer dependency

The worker read all 187 bytes/66 constructor instructions at `C4CE60..C4CF1A`,
all 14 bytes/seven instructions at `0040A050..0040A05D`, the real root source
and header, and the actual constructor EH metadata. No blocking mismatch was
found for their stated normal/C++ cleanup contract:

- Token and byte flag stores precede six interleaved bounds reads/writes, so
  actual alias effects remain; no early aggregate copy substitutes for them.
- Vector +30h/+34h/+38h zeros precede native state 0. EBX=0/EAX=10 makes the
  unsigned guard always take growth on normal entry. Capacity ten is published
  before actual new(40). The returned pointer is retained; count/data remain
  live after callbacks, and only a zero computed destination suppresses a copy.
- Current old data is conditionally freed before new pointer publication.
  Native FuncInfo `E040A0`, map `E04098` `{previous=-1, action=CC823A}`, and
  the real funclet route endpoint+30h to `0040A050`. The C++ catch invokes that
  concrete helper and rethrows. It does not clear fields, refund the retained
  new allocation or manufacture rollback.
- The 14-byte helper captures current vector data once and conditionally calls
  BF6989; it does not clear any vector word. Endpoint byte +2Ch is written zero
  only after normal constructor completion.

For ordinary C++ throws, the catch encloses the actual allocation/free operations
after vector zeroing. Native state 0 also covers the final +2Ch memory store;
no SEH/fault equivalence is inferred from that C++ catch. Cleanup-that-itself-
throws and original FH3 transport remain qualified. This review reuses root's
real source; it neither edits/copies the dependency into this worktree nor claims
new constructor/helper coverage or a new runtime test.

## Object evidence and remaining work

MSVC Win32 `19.51.36244.0` object-only compilation passed with
`/std:c++17 /permissive- /EHsc /fp:strict /O2 /W4 /WX`. Include search used this
tree first and root's real include directory second. Object SHA-256:
`d9ba2843242009b8291ab332f4ed14d4636fd029840651ea7c0b07d5901789bc`.

COFF/disassembly review confirms actual malloc slot +0Ch receives 68000, new
slot +4 receives wrapping bytes after capacity publication, and free slot +8
receives current old data before replacement. Duplicate tail and live copy/count
reloads remain in emitted code. The single real initializer relocation forwards
the five agreed arguments, and the caller retains endpoint for its return.
All emitted functions were scanned: no FP/SSE instruction was introduced.
Generated helper bodies from included C++ infrastructure earn no credit.

This is a source/epoch and object-provider review, **not** a 207-instruction
machine-code equality comparison. Compiler register allocation, integer compare
placement and source helper call frames differ. Nine actual native normal call
rows pass the report verifier. Ignored listings, bytes, object and audit data are
under `local/cc16_dyn_endpoint_allocate/`; full hashes are in the report.

Root must register/link/build the new source with the real initializer, inspect
the final target, and bind genuine manager storage and stable CRT custody at the
actual caller. Original register ABI, native stack/fault/EH identity, concurrent
mutation, allocation failure execution and gameplay remain unproved. No tests,
probe/game launches, shared CMake/ledger/Ghidra edits, installation/save/profile
or power/process changes were made. One new in-house normal body is counted;
all reused initializer, vector helper, CRT and EH/runtime bodies are excluded.

## Primary integration checkpoint

Primary inspection covers all 207 native instructions and complete source/header,
including two actual pool paths, live growth/link epochs and the genuine
initializer argument/cleanup contract. The common Win32 Release build and both
existing suites pass (11.23 seconds). A dedicated manifested link-only artifact forces
this actual entry and its real initializer/destructor/radix dependencies; no
fake object/provider or returning stub is supplied and it was never executed.
The entry is absent from the game map. Independent review9071aa5e8 found no blocking normal-body mismatch.
Native ABI/EH/caller custody and activation remain open.
