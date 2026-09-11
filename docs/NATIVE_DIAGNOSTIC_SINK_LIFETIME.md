# Actual diagnostic sink lifetime

This packet reconstructs three complete native entries, totaling 236 bytes.
The diagnostic owner is exactly four bytes: its profile word. The new C++
getter borrows the application's actual `0109CF14` publication slot and shared
`SingletonLifetimeDomain`. It uses the existing CRT allocation/free and real
lifetime manager registration, together with actual Windows critical sections.

| Entry | Original ABI and extent | Reconstructed interface |
| --- | --- | --- |
| `004C14C0..004C156A` | 170 bytes; no arguments; EAX publication; `RET` | `native_diagnostic_sink_get_or_create_004c14c0` |
| `004BBCA0..004BBCC9` | 41 bytes; ECX owner; stack flags; EAX original owner; `RET 4` | `delete_native_diagnostic_sink_004bbca0` |
| `00411EE0..00411EF9` | 25 bytes; ECX raw eight-byte guard; `RET` | `destroy_native_singleton_guard_00411ee0` |

Ranges end exclusively. `BSP_DiagnosticSink_GetOrCreate` is the existing
descriptive Ghidra name and should be preserved. The diagnostic class purpose
remains a naming hypothesis. The worker made no analysis or ledger mutations;
`004BBCA0` had no Ghidra function at its entry when its preimage was captured.

## Publication and ownership

The native getter installs its exception frame before reading publication. A
nonnull first read returns that captured pointer without retrieving a manager,
entering a lock, allocating, or registering anything.

For an empty publication it calls `00415350`, captures manager `+10`, and writes
the local guard's profile `00CE37FC` and captured section pointer before entry.
A nonnull section receives actual `EnterCriticalSection`, followed by an
increment of its current physical `+18h` tracked counter. Cleanup state 0 arms
only after that sequence. The publication recheck follows entry and arming.

If still empty, `00BF681B` receives four bytes. A nonnull allocation receives
profile `00CE752C`; a null allocation remains null. The getter publishes that
result, calls the real manager getter again, then rereads publication for
`00BD0C30` registration. The current registration provider validates its pointer
container before ignoring a null object. A registration exception leaves the
allocation published; this entry has no allocation cleanup state.

Normal exit decrements the captured section's current physical counter before
actual `LeaveCriticalSection`. State 0 remains armed during that normal leave.
Only after leave returns does the getter reread and return current publication.

The scalar deleting destructor tests flag bit 0 before its owner stores. It
always clears publication, writes owner profile `00CE3818`, optionally calls
actual `00BF65AC` free for bit 0, and returns the original address. It does not
unregister the owner. Application lifetime teardown must dispatch this actual
deleter with flag 1; the published pointer and manager must belong to the same
application domain used by all other singleton consumers.

## Raw guard and exception evidence

The raw guard is eight native bytes, containing profile at `+00` and the actual
native `CRITICAL_SECTION*` at `+04`. This pointer is distinct from the C++
`SystemSingletonCriticalSection` projection. The existing manager's owned
section stores a real Win32 `CRITICAL_SECTION` followed immediately by the
tracked DWORD at `+18h`; its projection references those actual fields.

The guard destructor captures `+04` before resetting profile to `00CE37FC`.
For a nonnull capture it decrements the current counter, then calls actual
leave. It preserves `+04`, including when another observer changes that field
during the profile write. The counter operation preserves unsigned wraparound.

| Native evidence | Meaning |
| --- | --- |
| `00C64F68`, 10 bytes | EAX = `00D8D5B8`; jump to FH3 `00BF6B43` |
| `00D8D5B8`, 36 bytes | Magic `19930522`, one unwind state, no try map, flags 1 |
| `00D8D5B0`, 8 bytes | State 0 transitions to -1 through `00C64F60` |
| `00C64F60`, 8 bytes | Guard at `[EBP-14h]`; jump to `00411EE0` |

The C++ getter's explicit catch begins after successful entry and increment,
and includes normal leave. It invokes the actual raw guard endpoint and
rethrows the original C++ exception. There is no invented construction state,
registration rollback, or exception-time free. The scalar free uses the
existing nonthrowing CRT-free interface; the getter and raw guard retain
throwable interfaces.

## Verification and limits

The full strict MSVC Win32 Release build passed through `scripts/build.ps1`,
with the owned source registered through an ignored CMake include. Both
existing CTests passed, and all eight installed-PE seed checks matched. No
permanent test or shared build/configuration file was added by the worker.

The focused private differential fixture links the unchanged resulting
`bsp_core.lib`, with its copy and original library hashes matched. Linker and
instruction evidence identifies this packet's object and the real
`singleton_lifetime.obj` providers. Both processes execute actual CRT
allocation/free, real pointer-container registration, and actual Windows
critical-section operations.

The original path executes all 236 owned bytes and the complete original guard
unwind, handler, and FH3 maps. All 335 copied bytes across fifteen spans match
fresh guarded Ghidra and the installed PE. Every final copied byte is verified,
allowing thirteen asserted address relocations, host exception registration,
two observed Windows IAT bindings, and explicit ABI bridges at genuine provider
boundaries. The native manager bridge supplies a twenty-byte layout carrier
whose `+10` refers to the real manager's current native section; the C++ manager
itself has a different layout. It does not implement a substitute manager.

Observers of four compiled singleton providers forward complete original
instructions through checked trampolines and then execute the unchanged
provider body. Entry/leave observers each perform the real Windows operation.
The fixture changes only its own executable IAT and private copied image.
Every trampoline byte and every original or compiled caller/write instruction
is verified. Fourteen loaded-module entry captures match the on-disk x86
modules with only loader relocations: `ntdll.dll` for synchronization,
`ucrtbase.dll` for malloc/free, and `vcruntime140.dll` for the resolved FH3.

Fifteen boundary states cover warm and cold publication; publication changes
after entry, after the second real manager getter, and after normal leave;
actual registration validation throwing or repairing the container and
returning; a changed current section; no section; scalar flags 0, 2 and 3;
and raw guards with null, captured, and wrapping-counter states. Entry and
normal-leave markers are injected after their real OS operations. The normal
leave exception case starts with two legitimate extra recursive entries, so
the second cleanup leave remains a valid real OS operation.

Both processes match **38,274 DWORDs / 153,096 bytes**, including 128 complete
snapshots, 57 actual provider observations and 17 publication/raw-guard writes.
All 114 caller PCs and 34 write PCs are verified. Snapshots include actual
publication, all tracked owner profiles, actual tracked/OS recursion, the full
256-pointer container, and guard surroundings. Seven actual domain shutdowns
also match, each dispatching the real owner deleter with flag 1 and clearing
publication. Fixture teardown repairs only deliberately injected terminal
states after the observations finish.

This is reconstructed, build-tested and fixture-tested through a new C++
interface. It is not a drop-in native ABI replacement or game validation. The
actual CRT provider's four-byte allocations succeeded; native null/OOM branches
remain byte-established rather than runtime-injected. Publication races,
asynchronous access faults/SEH, and arbitrary second exceptions during unwind
are outside the fixture claim. Replay scripts, exact hashes and trace artifacts
are recorded in `reports/native_diagnostic_sink_lifetime_audit.json`.

## Primary integration

The primary added the source to CMake, completed the strict Win32 build and
passed both existing CTests. It rechecked all 77 worker artifacts, five current
source/provider files and fifteen fresh live-Ghidra/PE spans (335 bytes). The
unchanged fixture linked the frozen main library, SHA-256
`85b7690b5a42ea0828e572c05e054ddf565282bd14f39888062655e3747131cf`. The complete 38,274 DWORD,
128 snapshot and seven actual-domain shutdown comparisons pass, including all
114 call PCs, 34 write PCs, full original and provider postimages, thirteen
relocation preimages and fourteen real loaded-module entry captures.
The missing 41-byte scalar destructor is now defined in the original Ghidra
project. Saved annotations preserve prior names/comments; all three complete
records and refreshed exports are registered. No permanent tests were added.
The fixture limits above remain unchanged.
