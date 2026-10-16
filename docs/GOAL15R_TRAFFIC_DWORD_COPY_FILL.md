# Whole raw traffic DWORD copy/fill helpers

This packet reconstructs four complete entries, totaling 171 Native bytes and
71 instructions. Their existing STL/library names are retained. The Source
names describe measured behavior; they do not recover a class or game symbol.

| Original | Bytes | Physical input and cleanup | Physical EAX |
| --- | ---: | --- | --- |
| `00486800` fill | 30 | Three stack DWORDs: first, last, value-cell address; plain RET, caller cleanup. | Reached endpoint. |
| `00486820` backward copy | 47 | Three stack DWORDs: first, last, destination end; plain RET, caller cleanup. | Captured destination begin. |
| `0048D660` forward copy | 48 | Three stack DWORDs: first, last, destination; RET 0Ch. | Captured destination end. |
| `0048E4A0` fill N | 46 | Three stack DWORDs: first, count, value-cell address; RET 0Ch. | First plus wrapped four times count. |

No body consumes incoming ECX or EDX. Some callers set ECX to their ESI header;
the children overwrite it before use. Source exposes cdecl for the plain-RET
entries and stdcall for the RET-0Ch entries. These express the measured physical
stack contracts; they do not establish an original formal convention or an
unused implicit class receiver. In particular, fill's observed EAX is exposed
by the raw Source API without claiming a recovered semantic return type for the
original STL fill. Both Native fill callers ignore that physical result.

## Whole-body behavior

Fill captures first and last, tests exact equality, and reads its third actual
stack cell only for unequal endpoints. It captures the value-cell address once,
then rereads the borrowed DWORD on every iteration, stores it, adds four with
DWORD wrap, and repeats until exact endpoint equality. It does not replace
equality with an ordered comparison or repair a malformed range.

Backward copy loads last and first, subtracts with DWORD wrap and performs signed
SAR 2. It then reads the actual destination-end cell, computes the wrapped byte
span and captures destination end minus span. It calls the real copy service
only for a signed positive count. Zero and negative counts still return that
computed pointer; no copy or pointer repair is inserted.

Forward copy reads all three actual argument words, performs wrapped subtraction
and signed SAR 2, and retains the SAR flags across LEA/PUSH/LEA. Its branch skips
only a zero count. A negative count therefore invokes the real service with
the wrapped DWORD byte span as both capacity and count. It captures destination
plus span before the service, ignores the service return, and returns that
captured pointer. Turning its test into a positivity guard would change behavior.

Fill N reads the count before first and reads the actual value-cell argument
only for a nonzero count. TEST clears CF, so the original JBE/JA pair implements
DWORD zero/nonzero tests, including high-bit counts. The value-cell address is
captured once; contents are reread each iteration. The decrement and pointer
advance wrap as DWORD operations. Its final LEA uses captured first and original
count. Zero-count execution still reads first but never reads the third cell.

The readable literal instructions preserve measured register saves, argument
read points, arithmetic/flags, cleanup, and fault order. This is the reason for
assembly here; equal byte counts are not a general Source requirement. There
is no byte embedding, padding, invented receiver, authored mutable global,
callback provider, null guard, snapshot, rollback, or noexcept/noreturn promise.
Every reached cell needs real backing, including aliased outgoing argument cells.

## Genuine copy service

At `00486843` the backward helper calls library `_memmove_s` at `00BF67A7`.

At `0048D682` the forward helper calls `00BF67A7`.
Source calls actual selected SDK `::memmove_s`
directly. The installed SDK header emits the real implementation: zero count
returns zero, null source/destination uses EINVAL, insufficient capacity uses
ERANGE, and successful copies use real memmove. Its actual current errno and
invalid-parameter services are retained. There is no simulated copy or handler.
Original private CRT/EH/errno/handler identity is not claimed by this boundary.

## All current caller setups

Fresh live xrefs identify only whole `00493120` (436 bytes) as caller, at nine
sites. All its instructions were reviewed and captured. On entry it dereferences
the caller's value-cell address and overwrites its actual fourth argument cell
with that captured DWORD. Later fill calls borrow the address of that captured
cell. Their value is not a zero-fill constant. Other supplied words remain raw
addresses/counts, with no recovered semantic class.

| Site | Child | Actual setup |
| --- | --- | --- |
| `004931DE` | `0048D660` | Current begin to captured insertion position, destination new allocation EBP. |
| `004931EC` | `0048E4A0` | Previous EAX endpoint, captured count EBX, address of actual captured-value cell. |
| `004931FD` | `0048D660` | Current insertion-position cell to current receiver+8, destination prior fill-N result. |
| `00493260` | `0048D660` | Position EDI to captured old end EBP, destination position plus wrapped requested bytes. |
| `0049327A` | `0048E4A0` | Current receiver+8, remaining count, actual captured-value cell address. |
| `00493292` | `00486800` | Position EDI to reread published end minus requested bytes, actual captured-value cell. Caller adds ESP 0Ch. |
| `004932A8` | `0048D660` | Old end minus requested bytes to old end, destination old end. |
| `004932B3` | `00486820` | Position EDI to old end minus requested bytes, destination end EBP; receiver+8 is already published. |
| `004932C5` | `00486800` | Position to position plus saved byte span, actual captured-value cell. Prior backward-copy arguments remain on stack; caller adds ESP 18h for both cdecl calls. |

The stdcall Source interfaces introduce no EDX placeholder. Physical caller
ECX header preimages are recorded as unused evidence, not invented inputs.
No Source implementation of this 436-byte parent is included or implied.

## Validation and scope

The tracked report records full Native/emitted instruction and operand maps;
ignored `byte_audit.json` includes every nonempty executable COFF section, its
actual symbol offsets, complete extents, and relocations. Nothing is clipped to
a NativeSize prefix. Fresh live/installed equality covers all 171 owned bytes,
the whole 436-byte caller, and the 91-byte qualified CRT provider.

Strict compilation matches all 163 fixed Native bytes; only two genuine CALL
rel32 operands differ. The strict object is 263 executable bytes: 171 owned
bytes plus the real SDK's 92-byte routine. The registered object matches the
same 163 fixed bytes and contains 265 executable bytes (171 plus the SDK's
94-byte frame-pointer variant). All five linked function extents and every
provider relocation match the registered object. SDK code is accounted
separately and is not presented as original CRT byte identity.

One ignored focused case exercises overlapping copies and a concrete capture
risk shared by both fill entries: the first destination is the actual outgoing
third argument cell, so the first store destroys its value-pointer word. The
next store must use the already captured pointer. Real caller scratch provides
backing. The fixture checks EAX, payloads and both cleanup conventions. It links
the registered archive and compares the public Source entries with all captured
Native body bytes, normalizing only the two true calls to the actual SDK API.
All four public Source and captured Native entries pass this focused case.
MSVC 19.51.36244 x86 /MD /O2 /Gy /W4 /WX /fp:strict /EHsc compilation passes.
The actual registered `scripts/build.ps1` passes both available CTest checks,
`reconstructed_math` and `tool_tests`; the native seed-reference test is not
enabled in this worktree. No broad test suite or tracked test is added. Full branch correspondence covers
zero/negative/high-bit behavior; no huge negative-span copy or billion-word fill
is executed. Runtime fixtures do not establish all flags, faults or environments.

The Source stack shape, integer instruction schedule and measured cleanup are
preserved. This does not establish a complete Original ABI/drop-in replacement,
original private CRT/FP-environment identity, reached game behavior, visual or
gameplay validation. There are no floating-point instructions in these bodies.

`00493120` still needs its whole parent reconstruction and exact allocator
`0047E970` and length-failure `00490240` entries. Qualified current allocation,
free and legacy logic-error building blocks do not close those bodies. Whole
`00495AE0`, `0049E560`, `004C71C0`, `004D5BD0` and authored mutable-global
producers remain gates. No allocator/error Source expansion is part of this packet.

## Reproduction and annotation handoff

Ignored `local/goal15r_traffic_dword_copy_fill/` contains the self-contained
handoff, receipt, strict compiler script, full Native/COFF captures, focused
fixture, archive provenance and wall-time record. Run the registered
`./scripts/build.ps1`, `build_probe.ps1 -CompileOnly`, `python audit.py`, then
`build_probe.ps1` and `python audit.py --reuse-native` using those local paths.
The last audit refreshes the strict-object hash and verifies linked code and
actual provider relocations without repeating live queries or runtime tests.
Full wall time starts before brief/merge/ownership work and ends with the
commit/release/handoff; the final receipt records that interval, including
qualification, compilation, registered build, fixture, documentation and overhead.

Old names, formal prototypes and comments are captured. This worker performs
no Ghidra mutation; primary will annotate under the write lock, save and refresh.
Prior erase packet `2bc75e516` Source/doc/report and all 51 local artifact hashes
were preserved. Its original registered archive, object and registration text
are copied under `prior_goal15p/` with verified hashes before this build.
