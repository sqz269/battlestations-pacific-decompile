# Native physical buffer Lock

This packet reconstructs the complete physical index Lock at `00B4B850`
(`00B4B8FC` exclusive) and physical vertex Lock at `00B4BA00` (`00B4BAAC`
exclusive), 172 bytes each. Both expose the same behavior through independently
named C++ entries and one private implementation in
`src/native_physical_buffer_lock.cpp`. Names are reconstruction hypotheses.

The original ABI takes the actual 2Ch physical owner in ECX, then five stack
arguments: requested byte count, extra byte offset, unused DWORD, output DWORD
address, and read-only byte. EAX returns the mapped data pointer; `RET 14h`
removes the arguments. The new `__fastcall` bindings preserve these five stack
arguments and additionally require `NativePhysicalBufferLockContext` in EDX.
That context binds the actual diagnostic publication `0109CF14`, actual
`SingletonLifetimeDomain` corresponding to `01090AA0`, and the ADDRESS of the
null-buffer sentinel `00F8D4B8`. This added context is a new host interface;
original callers require an explicit binding and are not binary-compatible.

The implementation reads and writes the actual owner storage. DWORD accesses
use MSVC x86 instructions so unaligned storage and partial output aliases retain
the native access width and ordering. It introduces no synthetic physical owner,
COM object, diagnostic provider, mapping pointer, or resource lifetime.

| Owner offset | Observed use |
| --- | --- |
| `+14` | Flags; bit `1000h` selects dynamic behavior |
| `+18` | Signed capacity comparison operand |
| `+1C` | Current byte cursor |
| `+20` | Wrapped lock depth counter |
| `+24` | Wrapped dynamic Lock count |
| `+28` | Current actual COM buffer pointer |

The initial check adds cursor and extra offset with DWORD wrap, then compares
the resulting signed value with signed capacity. It does not include requested
bytes. A greater result calls the complete actual diagnostic getter
`004C14C0`; execution then continues. Dynamic selection reads flags after that
call. Static Lock flags are `800h`, or `810h` for any nonzero read-only byte.
Dynamic flags are `2000h` at cursor zero and `1000h` otherwise, regardless of
read-only. A nonzero extra offset then calls the diagnostic getter again. The
already selected Lock flags survive that second call, while subsequent owner
loads observe its mutations. Dynamic count increments after this call.

The routine next captures current COM `+28`. For nonnull COM it captures the
actual COM table, reads current cursor plus extra offset, and only then loads
stdcall Lock from table `+2C`. The mapping output starts null. The call receives
the captured actual COM receiver, current offset, requested byte count, mapping
output, and selected flags. HRESULT is discarded. For null COM, the result is
the sentinel ADDRESS and the owner cursor is cleared; sentinel contents are
never read as a pointer.

After the COM or null branch, the routine writes current cursor to the supplied
output address. It then freshly tests owner flags and, if dynamic, adds requested
bytes plus extra offset to the then-current cursor with DWORD wrap. Finally it
increments then-current depth. Thus an output store overlapping flags or cursor,
including a partial unaligned overlap, can affect the later operations. Changes
made by actual provider calls also remain visible. A provider exception propagates
and prevents later operations; no `noexcept`, automatic Unlock, rollback,
HRESULT fallback, retention, or validation guard is added.

## Evidence and verification

The worker verified project `bsp`, program `/battlestationspacific.exe`, x86
language, and image base `00400000` before guarded live Ghidra batches. Seven
spans totaling 588 bytes matched the installed PE, including the 344 complete
owned bytes and diagnostic references. The two four-byte globals are verified
loader-zero virtual section tails. Assembly establishes stack arguments, signed
comparison, instruction-width accesses, call/store ordering, and `RET 14h`.
This worker made no Ghidra or shared-ledger mutations.

The actual diagnostic provider was copied unchanged from worker commit
`ec95cac38e7f7d83c40c48a593cb81dbd9c27ca5` into ignored private build inputs.
Its source and header hashes equal the subsequently integrated main sources.
The public header depends on `bsp/native_diagnostic_sink_lifetime.hpp`, so that
provider must be integrated before this packet. Private deferred CMake source
registration built both actual sources without editing tracked CMake files.
`scripts/build.ps1` passed the strict MSVC Win32 Release build and both existing
CTests after all eight native seed spans were verified.

One ignored focused fixture links a frozen byte-identical copy of that strict
build's `bsp_core.lib`; it compiles only the fixture. Its map attributes both
29-byte public entries and their complete 388-byte private implementation to
`bsp_core.linked:native_physical_buffer_lock.obj`. The actual diagnostic getter,
deleter and guard entries come from that same library. COFF relocation records,
full linked bodies, source/object/library hashes and runtime postimages are
retained in `reports/native_physical_buffer_lock_audit.json`.

The fixture executes the 344 original owned bytes unchanged at a uniform code
delta `1F4C0000`, preserving both relative diagnostic call operands. The one
external diagnostic entry at relocated `004C14C0` has a five-byte jump to a
context bridge calling the actual compiled diagnostic getter. That bridge does
not alter any owned byte or operand. Absolute sentinel/publication storage is
mapped at its original addresses. The native execution pages are read/execute.
The EXE and D3D9/user32 delay loading permit those bounded address mappings.

A real D3D9 HAL device on NVIDIA GeForce RTX 5090 creates four 64-byte buffers:
static vertex/index buffers in SYSTEMMEM and dynamic WRITEONLY vertex/index
buffers in DEFAULT pool. GetDesc verifies their properties. Each actual COM
object temporarily uses a cloned table whose sole changed slot is Lock; that
observer forwards the saved real `d3d9.dll` Lock with the same actual receiver
and arguments. It records actual HRESULT, mapping identity and 16 mapped bytes.
READONLY samples read bytes previously seeded through real Lock/Unlock; writable
samples explicitly write their observed mapped buffer before recording it.
These writes and owner mutations belong to the fixture observer. Tables are
restored before final Release, and successful mappings are unlocked during
cleanup. No frame is rendered or presented and no window is shown.

The diagnostic path uses the actual compiled getter, actual lifetime domain,
actual registration and destruction, and real Win32 Enter/LeaveCriticalSection.
Two temporary fixture IAT observers forward those real Win32 calls and record
critical-section recursion and publication timing. Selected observers mutate
the owner after the real Enter or Lock to expose the subsequent native loads.
Actual publication, native profile word, registered pointer identity, manager
count, released section, and cleared publication after actual shutdown are
checked. The fixture does not replace the diagnostic provider with a callback.

Ten states execute for each native entry and its corresponding library entry:

| State | Behavior observed |
| --- | --- |
| 0 | Static READONLY with a high-bit nonzero read-only byte |
| 1 | First cold diagnostic changes dynamic flags, cursor and actual COM; Lock mutation and full output alias affect later cursor/depth |
| 2 | Second cold diagnostic changes flags/cursor after dynamic Lock flags were selected |
| 3 | Dynamic NOOVERWRITE with a nonzero cursor and ignored read-only request |
| 4 | Unaligned output partially overlaps flags; subsequent fresh test sees the change |
| 5 | Null COM returns the sentinel address, diagnoses dynamic extra offset, and wraps requested-byte advance |
| 6 | Wrapped signed capacity check avoids diagnostic and null COM clears cursor |
| 7 | Real WRITEONLY buffer receives READONLY; actual HRESULT and mapping are retained |
| 8 | Observer throws after successful real Lock; output and depth stores do not occur |
| 9 | First cold and second hot diagnostic calls with the updated cursor |

All 20 paired cases match: 5,396 DWORD observations and 128 event frames per
implementation. Sixteen pairs invoke real D3D9 Lock; four pairs use null COM.
All real Lock HRESULTs in this run were S_OK, including state 7. No failed-driver
HRESULT runtime result is claimed; its unguarded behavior is established by the
original and optimized library assembly, which have no HRESULT-dependent branch.
State 8 is an explicit fixture observer exception after a successful driver
call, not a driver exception.

Raw 48-byte owner storage and 12-byte output storage compare literally, including
unaligned padding. Mapping addresses normalize only to null/sentinel/actual
driver output after identity verification; diagnostic publication normalizes to
presence with its actual profile and registration checked separately. Before
execution and after each of 40 individual calls, nine entire code spans are
captured: six actual library entries, two original bodies and the external
bridge. All 369 span snapshots match their initial bytes, and the six initial
library bodies match the fixed-base linked PE.

This establishes the bounded actual-provider Lock behavior through the explicit
host service binding. It does not establish original-caller drop-in ABI,
complete buffer-owner lifetime, allocator failure or concurrent publication
coverage, renderer integration, device reset, game compatibility or visual
parity. Existing semantic buffer bindings and permanent tests are unchanged.
