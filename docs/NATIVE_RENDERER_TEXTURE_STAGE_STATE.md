# Actual renderer texture-stage state setter

`native_renderer_texture_stage_state1` reconstructs the complete 241-byte
`00B24510..00B24600` body. The descriptive name is an interpretation, not a
recovered original symbol. Native callers use ECX for the renderer and three
stack DWORDs for stage, state and value; the entry returns with RET 0Ch.
The new C++ interface also requires the actual synchronization-global storage.
It is not a binary drop-in replacement and is not `noexcept`.

## Storage and ordering

The actual cache addresses are:

| Field | Byte offset from renderer |
| --- | --- |
| Validity byte | `45Ch + stage * ACh + state` |
| Value DWORD | `480h + (stage * 2Bh + state) * 4` |
| Current COM device | `1A10h` |
| Current call counter | `1BA8h` |

Every expression wraps at 32 bits, including the final address calculation.
The source uses single x86 byte/DWORD loads and stores; an unaligned renderer
is supported. It adds no bounds checks, enum validation, stage remapping,
atomic operations, HRESULT policy, cache reconstruction, or synthetic device.
Every reached address and COM receiver must be valid actual storage.

The entry checks the current raw synchronization mode and, when nonzero,
stores the actual renderer in a native-shaped local guard record before
calling the full actual `00B33AD0` optional entry. Its returned AL is saved.
Mode zero leaves the guard record uninitialized, as in the original.

The original compares the validity byte at `00B24562` **before** arming state
zero at `00B24571`. The source preserves that boundary: the optimized validity
load is at function offset `65h`, before cleanup initialization and the EH
state-zero store at `7Eh`. A nonzero validity byte short-circuits to the value
comparison; any nonzero byte counts as valid. The invalid path does not read
the cached DWORD.

An invalid entry or changed value stores validity one, then the requested
DWORD. It subsequently reloads the current device, current device table and
slot `+10Ch`, and calls actual SetTextureStageState with the original stage,
state and value. There is no sampler-number conversion. After the call returns,
regardless of HRESULT, it reads and increments the current `+1BA8` counter.
It does not rewrite cache fields modified during that call. An exception
prevents the counter update.

On both native normal exits, the current mode is read before cleanup is
disarmed and before either saved guard field is read. The source's optimized
mode load at `11Ch` precedes disarming at `11Eh` and saved-field reads at
`126h/12Ah`. A nonzero mode calls full actual `00B33B00`, whose current mode,
current renderer lock and wrapping counters retain their existing contracts.
A normal Leave exception occurs after disarming, so it does not call Leave
again through this guard.

The complete native FH3 handler is `00CBCFB8` (10 bytes), with FuncInfo
`00DF5610` (36 bytes: magic `19930522h`, max state one, EH flags one) and
unwind map `00DF5608` containing `[-1, 00CBCFB0]`. The complete eight-byte
funclet computes guard address `[EBP-14h]` and jumps to `00B21110`. The source
unwind path invokes that full actual guard destructor, which reads the
current mode before the saved record. A second C++ exception during unwind
terminates; other SEH exceptions continue searching. No default guard repair
is added when entry was skipped and the mode subsequently changes.

## Verification

Twelve fresh guarded Ghidra/installed-PE spans total 508 bytes: the full setter,
all three actual guard bodies, complete handler/funclet/maps, the two original
OS import cells, synchronization globals, and a complete reference for the
original FH3 runtime entry. Each query verified project `bsp` and program
`/battlestationspacific.exe`. The worker made no Ghidra or shared-ledger edits.

Strict MSVC Win32 `/W4 /WX /fp:strict`, both existing CTests, and eight native
seed checks passed. Source, header and the actual built library were frozen
before compiling the ignored fixture. Source providers were not replaced.

Fourteen original/compiled pairs match 226,438 DWORDs and 124 event frames per
implementation. The cases cover disabled mode, noncanonical mode with a null
lock, invalid/same/changed cache entries, entry-time cache mutation, current
device and lock changes, cache mutation during Set, current counter wrap,
and current-mode changes during Set and Leave. Exceptions injected only after
real successful Enter, Set or Leave verify the distinct unarmed, armed and
disarmed boundaries. Mode zero during an armed unwind skips Leave even though
the original FH3 funclet runs.

The original setter, all three original guard bodies, full original FH3
handler, funclet and maps execute in isolated mapped pages. Declared operand
relocations bind their original globals/import cells and maps. The setter's
handler push points to a static registered trampoline; the original handler
then supplies its mapped FuncInfo to the real current `__CxxFrameHandler3`
through a register-preserving observer. This executes the complete native
cleanup graph; it does not substitute a direct cleanup callback. All native
changes are explicit in the manifest, and all other bytes match the PE.

Two real NVIDIA GeForce RTX 5090 D3D9 HAL devices supply SetTextureStageState
and GetTextureStageState. Each observed Set forwards the saved actual method,
receiver and arguments, then verifies the real Get result. Both source imports
and native import cells invoke observers that forward actual Windows critical
section calls. The fixture records complete renderer storage, global bytes,
OS recursion and projected lock depths, and restores the fixture's outstanding
locks and COM/import tables after observation. It draws and presents nothing.

Both stage `40000000h` and ordinary out-of-range stage eight returned S_OK and
round-tripped through the actual Get on this driver. The first also verifies
wrapped cache addressing and that the original stage reaches the API intact.
These observations do not establish a failed-HRESULT path; the source's
unconditional counter update is separately established by native/compiled
assembly. No failure result was fabricated or substituted.

The real FH3 import target is also checked against the loaded runtime export,
with separate module provenance for the executable thunk and the actual DLL.
The audit verifies 14 selected complete COFF functions against the linked PE,
the exact native operand/map relocations, and 29 stages of 27 complete
code/map/import spans (783 unchanged snapshots). Evidence and immutable artifact pins
are in `reports/native_renderer_texture_stage_state_audit.json`; fixture and
build files remain ignored under `local/` and `build/`. No permanent test or
shared CMake change was added.

Remaining boundaries include original-caller ABI, arbitrary invalid addresses,
mode enabling after an uninitialized guard was skipped, asynchronous SEH or
secondary-exception termination subprocesses, exhaustive state/stage/value
combinations, a failed real HRESULT, concurrent mutation between pure reads,
complete renderer integration, gameplay and visual validation.

## Primary integration

The primary registered this source in CMake and passed the strict Win32 build,
both existing CTests and eight fresh native seeds. It independently verified
49 worker artifact pins, five current source/provider files and twelve fresh
live-Ghidra/PE spans (508 bytes). The actual main library
`3ca9a0274a0ce0fb9c7e0855bf75c1d8d2b70e70d005c7b3da1fcdcf6404600b`
and two exact archive objects passed the unchanged fixture: fourteen pairs,
226,438 literal DWORDs and 124 frames match. Fourteen complete COFF/linked
functions and all 783 whole runtime code/map/import postimages are verified.
The full native guards and FH3 handler/funclet/maps execute with declared
bindings. Actual FH3 export/import agreement and its six-byte thunk/four-byte
cell are pinned alongside real HAL/OS callback and exception behavior.
All real HRESULTs were S_OK, including stages 40000000h and 8; failed returning
HRESULT behavior remains instruction-audited. The existing descriptive name
and appended evidence are saved in Ghidra with prior comments preserved; the
full ledger record and forced export are registered. No permanent tests,
original-caller ABI, full renderer, gameplay or visual claims were added.
