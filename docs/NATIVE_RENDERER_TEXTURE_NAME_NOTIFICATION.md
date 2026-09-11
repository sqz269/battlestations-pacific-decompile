# Native renderer texture-name notification

`notify_native_renderer_texture_name_removal_00b32250` reconstructs the complete
240-byte function `[00B32250,00B32340)` against actual native storage. It uses the
existing string, optional-guard and resource-container implementations. The
receiver's container receives the original name after a separate temporary name
has been allocated, copied and lowercased. No resource retain/release operation
is added by this wrapper.

The strict MSVC Win32 repository build and both existing tests passed. One
private original-caller comparison matched 4,700 normalized words across nine
states. Two terminal scenarios, each compared in separate original/host processes,
matched another 658 words while terminating with exit 86. These results establish
the tested storage/callback/EH behavior, not native ABI compatibility or game
execution. No new tracked test, Ghidra annotation, ledger or shared build edit is
included in this packet.

## ABI and shared domains

Original ABI: ECX actual renderer receiver, one stack pointer to an eight-byte
native name header, RET 4. EAX has no stable semantic result. The complete native
function is still undefined in Ghidra at this checkout; its old signature/comment
lookup results are preserved in the audit. The proposed descriptive name is
`BSP_D3D9Renderer_NotifyTextureNameRemoval`, a reconstruction hypothesis.

The new C++ entry takes the actual receiver, original name header and
`NativeRendererTextureNameNotificationContext`. That context borrows:

- The application's actual `00F8D394` renderer publication and
  `NativeRendererSynchronizationGlobals` for `0108D6DC..0108D6E3`.
- Actual `NativeStringStorage` and the same `00419CC0` `SizedStoragePool` used
  by nested resource-container removal. A `PooledStringStorage` or an observing
  adapter over that same pool meets this requirement.
- Actual `SingletonLifetimeCallbacks` and borrowed immutable
  `NativeRenderResourceAccountingTables` for the existing removal implementation.

The actual receiver is independent of the global renderer captured for the guard.
The receiver supplies its actual container at +1A74h. At a resource match, the
existing container domain requires current `D61948`, `D61870` or `D618B0` profile
and the verified current accounting slot. Unused accounting tables need not be
available. This wrapper supplies no arbitrary-profile fallback.

## Exact capture and cleanup order

| Native sites | Reconstructed action |
| --- | --- |
| `B32268..B32285` | Test entry-time raw mode; if enabled, capture current global renderer, enter its actual guard, save AL only |
| `B32289..B322A9` | Capture original name; compare temporary/input addresses; arm state 0; zero the actual temporary header |
| `B322AB..B322D0` | Resize from original length, reread original length, capture temporary data, copy using current temporary length and current input data |
| `B322D7..B322E8` | Arm state 1 after copy; lowercase temporary; call full `B31DC0` with original header on captured receiver +1A74h |
| `B322EF..B32308` | Disarm string cleanup; return captured temporary data using current temporary length + 1 |
| `B3230D..B3232A` | Test current mode, disarm guard cleanup, and conditionally leave using the saved renderer and whole ignored saved DWORD |

An isolated DWORD MOV reads the saved guard word, including its unwritten padding.
The existing leave routine ignores that value. The code does not evaluate an
indeterminate C++ scalar or normalize the original raw mode. The initialized-guard
domain is unchanged: cleanup must not become enabled after entry was skipped.
If mode becomes zero after successful entry, cleanup is skipped and the acquired
lock/depth remain as the native leaves them.

The temporary's normal return uses the data pointer captured after resize, even
if a later callback replaces its current header. The return size comes from the
current length. Exception cleanup instead calls the complete existing
`destroy_native_string_header_0041dd20`, which reads the current header's data
and length. This distinction was tested with separate actual pool allocations.
The source preserves native DWORD arithmetic and the existing zero-length copy
boundary, while evaluating the original current-field reads. The direct call at
`B322CB` targets `BF7680`. Its complete 869-byte body, including embedded jump
tables, is pinned: `BF7694..BF769A` selects backward copying when the destination
lies inside the source range, and `BF7844` begins that path. The library's
`_memcpy` label therefore does not impose the C++ non-overlap precondition.
The source and the declared CRT fixture bridge use `std::memmove`.

## Native exception states

The saved handler is `00CBDCF0`, FuncInfo `00DF6694`, unwind map `00DF6684`.
The full map has two states and no catch map:

| State | Next | Original action |
| --- | --- | --- |
| 1 | 0 | `CBDCE8`: current temporary header -> full `41DD20` |
| 0 | -1 | `CBDCE0`: saved guard record -> full `B21110` |

Guard entry happens before state 0 is armed. String state 1 begins only after the
copy finishes. A resize failure can therefore leave partial temporary storage
without string cleanup, while still unwinding the already-entered guard. A later
failure destroys the current temporary, then the guard. Normal string return
executes with state 0; normal leave executes after state -1.

The C++ implementation uses an armed cleanup-only object with explicit string
state. Its compiled FuncInfo has zero try blocks and no catch map. A synthetic
catch/rethrow would change the in-flight exception count and could intercept a
second exception before the outer native termination filter. The cleanup action
uses an SEH filter for `E06D7363` that terminates during exception search, before
nested cleanup runs; `noexcept` alone does not express that timing. Normal leave
runs after the cleanup object is disarmed, so its first exception propagates
without another guard leave.

## Original-caller verification

Fresh target-guarded CLI reads verified the selected `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, then matched 37 complete/prefix spans to
the installed executable. Complete code spans decoded to their exact extents;
26 absolute instruction/data relocations have checked preimages. The audit
preserves original bytes, ABI, native map values and source/artifact hashes.

The private sparse image executes complete original `B32250`, nested `B31DC0`,
guard enter/leave/unwind, `41DD20`, `4BCC00`, the accounting leaves, original EH
handlers, maps and unwind actions. Code, maps and tables are protected after
loading; unverified committed gaps contain traps. Handler registration uses two
host-image jump-only trampolines that immediately enter the complete relocated
native handlers. Registering heap addresses directly did not propagate the
injected C++ exception through this host runtime; the final adapter preserves
the original handler instructions and state maps. Its runtime requirement is
specific to this private fixture and is not a game ABI claim.

Declared ABI bridges invoke the unchanged existing actual C++ string resize,
normalized-path copy, record assignment/destruction, same real sized pool and
CRT comparison and overlap-preserving copy functions. The `BF7680` bridge uses
host `std::memmove`; its pinned original body is evidence, not executed by this
fixture. They do not replace container behavior with a
vector erase or a synthetic result. Native IAT and corresponding host calls use
observation wrappers around real Win32 Enter/LeaveCriticalSection. Fixture-only
faults and field changes occur at those explicit boundaries. Ordinary storage
uses the existing real allocator/free bodies; intrusive resource state is seeded
as actual native storage, with current native table identities preserved.

The compared states cover disabled entry with invalid unused global publication;
distinct receiver/global renderer plus a publication change during entry;
mutation of the original key after the outer temporary is lowercased; normal
captured-data/current-size return; resize failure before string state; failure
after string state with a replaced current header; entry failure before guard
state; normal leave failure after disarm; and a current-mode change that retains
the lock. One added overlap state changes the original input pointer to the
first actual temporary allocation plus one byte at allocation time. Its 31-byte
source and destination ranges overlap in the forward direction (destination is
below source); all resulting bytes are checked before
nested allocation, and the actual base allocation is returned normally at size
32. This mutation tests the admitted storage contract, not observed game usage.
The first terminal comparison throws from guard leave during cleanup:
the current temporary was already returned, the lock is released, and a nested
cleanup probe must remain untouched. The second invokes notification during an
outer unwind and throws in its body: termination precedes notification's local
cleanup, so both temporary allocations and its lock remain live. Both comparisons
check in-flight exception counts, exact allocation/lock side effects and the
absence of nested cleanup, in addition to the exit code. Residual fixture locks
and live allocations are cleaned up after normal observations; terminal process
state ends with that process.

Reproduction inputs remain under the worker's ignored `local/` directory:
`prepare_native_renderer_texture_name_notification.py`,
`build_native_renderer_texture_name_notification.ps1`, and
`build_native_renderer_texture_name_notification_check.ps1`. The build registers
only the new source through a private CMake deferred include. The integrator must
register `src/native_renderer_texture_name_notification.cpp` in the shared target
and owns later Ghidra definition, naming, comment, export and ledger work.
