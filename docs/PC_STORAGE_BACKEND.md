# PC storage operation backend

`PcStorageBackend` implements the actual PC manager progress target at
`00beb9b0`, its read/write requests, Win32 file effects, compressed block reads,
and Lua archive execution. It is a typed host projection, not a 0x540-byte native
object or a binary hook. Descriptive names below are hypotheses.

The relationship is verified, rather than inferred from nearby addresses:
`BSP_SaveStorage_Initialize` (`00beb2c0`) writes vtable `00d68c10`, whose word at
`00d68c54` (+44h) is `00beb9b0`. `00bd3580` tail-dispatches that exact slot.
The initializer derives the root from `SHGetSpecialFolderPathA` with
`CSIDL_PERSONAL=5`, creates `Battlestations-Pacific` and then `save`, and stores
the root at +530h. This packet accepts that already initialized root explicitly.
It never chooses or modifies the installed save directory automatically.

## Files and operation states

`00e15320` points to `valid`. The three DWORDs from `00e15324` point to `quick`,
`player`, and `game`. Paths are `root\slot\leaf`. A query through vtable+1Ch
(`00beb1e0`) requires both `valid` and the indexed file to be openable for read.
The native kind is a DWORD: the existing profile interface's bool describes only
its kinds 0 and 1; profile-manager requests also use kind 2.

| +04 operation | Trigger and +08 transition |
| --- | --- |
| 1 | PC capability slot+08 returns true; set +21, state 0, operation 0. |
| 2 | Read/decode/run Lua; on initial error raise `globals.loadfailed`; otherwise stamp `valid`. State becomes error byte ? 1 : 0; operation clears. |
| 3 | From state 0/1: create slot directory, remove `valid`, raise busy `globals.saving_pc`, begin 64KB writer at kind 0, state 2. Next update finalizes/writes `quick`, begins kind 1 and retains state 2. Next update finalizes/writes `player`, stamps `valid`, sets state 0 and clears operation. |
| 4 | From state 0/1: raise busy, remove `valid`, begin the requested kind without creating a slot directory. From state 2: finalize/write, stamp `valid`, complete. |
| 5 | From state 0/1: raise yes/no `globals.deleteslot`, state 3. Response 1 deletes marker, all three data files, then directory; other responses complete with state 1. |

Requests copy the relevant name, clear +20, and write +04. They do **not** reset
+08 or prompt fields. `00bd3dc0` initially leaves +508 untouched; the first
operation-3 update sets kind 0. `00bd3e10` sets +508 immediately. `00bd3590`
only clears +04. The driver treats state 2 as a continuation boundary, allowing
the settings/profile serializers to fill each newly opened writer.

The implementation preserves several unusual native results:

- CreateDirectory's return is ignored. A later data-file write reports failure.
- Single-file initialization continues after failure to remove the marker;
  `00bd37b0` clears the error byte before writing the file.
- A final write's marker-stamp failure sets +20 but still completes with state 0.
  A read's marker-stamp failure completes with state 1 and does not raise the
  earlier load-failure prompt.
- Deletion judges success only by absence of an openable `valid`; unrelated
  files can prevent directory removal without changing that result.
- Win32 writes test the API BOOL and ignore its written-byte count.

## Read and writer ownership

`00bd3ec0` calls vtable+28h (`00beaba0`), using `CreateFileA` with GENERIC_READ,
FILE_SHARE_READ and OPEN_EXISTING, GetFileSizeEx, then ReadFile. Fewer than six
bytes is an error. The first four bytes are compared case-insensitively with
`Opti`, `Play`, `BSP_`, and `Save`; matching files are plain Lua text.

Otherwise each block begins with a four-byte little-endian compressed length
followed by zlib bytes. The six bytes at each block start are XORed with
`C7 04 0F 48 FE 4C`, affecting the length and the first two zlib bytes. A first
pass reverses those bytes and counts blocks; a second inflates each block with
an initial 65536-byte destination limit. The resulting +30 allocation and +34
used length are passed to `00bd3470`.

That helper calls owner-open `00b6a020(1)`, then `00b65ef0`, which loads the
buffer with chunk name `DoBuffer` and executes it with `lua_pcall(0,
LUA_MULTRET,0)`. Neither successful return values nor the error are popped by
`00b65ef0`. On error, +20 is set, +30 is freed and cleared, and the owner closes.
`PcStorageBackend` executes these real Lua 5.1.1 calls. `PcStorageLuaHost` must
supply the real owner-open environment: base libraries, platform/region globals,
DoFile and the game's `Scripts/fundamentals.lua`. An empty Lua state is not a
production implementation of that dependency. The focused fixture deliberately
uses a small, explicit environment solely for its self-contained archive text.

`00bd34c0` creates 64KB scratch storage, zeroes +520 and sets mode +524 to 1;
it leaves the pending-block list untouched. Writer output uses the shared
`ArchiveCompressionState` and `ArchiveCompressedSink` from
`archive_compression.hpp`. `00bd4a70` finalizes framed bytes. `00bd37b0` writes
them through vtable+2Ch (`00beacb0`) using GENERIC_WRITE, FILE_SHARE_WRITE and
CREATE_ALWAYS. Success frees/clears +50C and sets +524 to 0; failure leaves the
allocation for the update's `00bd3500` cleanup.

## ABI and evidence

| Native entries | Native calling convention |
| --- | --- |
| 00beb9b0, 00bd3ec0, 00bd3470, 00bd34c0, 00bd37b0, 00bd3500, 00bd3530, 00bd3590, 00bea9e0, 00beaa70 | ECX=manager, RET; predicates return AL/EAX as shown in assembly. |
| 00bd3d70, 00bd3e10 | ECX=manager; NativeString*, DWORD kind on stack; RET 8. |
| 00bd3dc0, 00bd3e70, 00beab50, 00beaf00, 00beaf70, 00bead60, 00beb1c0 | ECX=manager; NativeString* on stack; RET 4. |
| 00beaba0 | ECX=manager; name*, kind, buffer-out**, size-out*; RET 10h. |
| 00beacb0 | ECX=manager; name*, kind, buffer*, size; RET 10h. |
| 00beb1e0, 00beb040 | ECX=manager; name*, kind or leaf C-string; RET 8. |
| 00bd4140 | ECX=manager; message*, three byte flags in stack words; RET 10h. |

The raw disk listing was checked where `_free` had incorrectly hidden returning
code: `00bd3470` through `00bd34b9`, `00bd3500` through `00bd352b`, `00bd37b0`
through `00bd380a`, and three gaps inside `00bd3ec0`. The integrator applied
locked flow repairs and saved the existing `bsp.gpr` program
`/battlestationspacific.exe`. Forced exports refresh the repaired pseudocode;
an ordinary cached export had misleadingly retained the old gaps.

The host rejects paths exceeding native 256-byte buffers, invalid kind indexes,
lengths beyond the readable compressed allocation, 32-bit output-allocation
overflow, files above DWORD size, and short successful reads. These cases would
overflow buffers, truncate sizes or consume uninitialized/out-of-bounds memory
in the native implementation. RAII also closes read handles on GetFileSizeEx
failure, where the native helper leaks them. Those are explicit host-domain
differences; no behavior claim is made for native memory-corrupting inputs.

## Verification and remaining composition

MSVC Win32 Release builds; the existing two CTests pass after all eight seed
byte checks match. One ignored integration fixture performs real isolated-file
two-stage writing, a multiblock 70KB Lua read, plain Lua loading, malformed-block
rejection, Lua-error buffer/owner cleanup, single `game` writing, a locked final
marker's state-0/error-1 quirk, both delete responses, and real write failure.
Its explicit Lua owner records three opens and three closes. Fixture artifacts
remain in the worker's `local/storage-fixture-*` and `local/storage-backend-roundtrip-2`.

Bind `StorageOperationState.manager_0109cecc` to `backend.operation()` and use
`PcStorageOperationHost` to combine its progress with the existing prompt/render
host. Profile read/manager code can call the public query/request/buffer/Lua
accessors; archive writers share the exact same compression state. The broader
ProfileIoHost reset/reader/settings/game notification services, native Lua
bootstrap and real rendering remain required composition. This packet is
reconstructed, build-tested and host-fixture-tested, not ABI-compatible,
native-differential-tested, gameplay-tested, or a complete profile host.
