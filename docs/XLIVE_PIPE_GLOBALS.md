# XLive pipe startup globals

This packet reconstructs twenty complete initializer, constructor and cleanup
functions in `src/xlive_pipe_globals.cpp`. `XLivePipeGlobalsOwner` owns one
stable set of protocol/framing values and the actual acquisition lock.
`protocol_globals()` returns references to that same state. Framing binds the
public members and `acquisition_lock_f8b778()` directly; it creates no copies.

The owned domain has explicit storage lifetime. It must outlive active protocol
and framing users and every registered CRT callback. Native cleanup occurs in
those callbacks, not an added C++ owner destructor. Reinitialization, concurrent
startup/shutdown, and repeated cleanup do not acquire invented guards or state.

## Original data and preimages

The installed original executable is 12223752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Its `.data` section begins at RVA `A08000`, has raw size `10000h` and virtual
size 2719452. The selected global objects lie beyond the raw end `A18000`,
within the virtual section. Their initial zero representation is therefore
actual PE loader zero-fill. `original_xlive_pipe_loader_preimage()` exposes
that proven pre-startup image. It is not an initialized encoded-value default
and does not supply the unrelated protocol heap allocation preimages.

The owner requires an explicit preimage argument. Constructors then replace
all encoded payload bytes with the fixed original data below. They allocate
and store each lock pointer **before** the forward byte copy. Null allocation
still permits the native copy; a thrown allocation leaves the destination's
preimage intact. Existing lock pointers are not released or cleared first.

| Global | Shape | Source | Constructor | Initializer | Cleanup |
| --- | --- | --- | --- | --- | --- |
| `F8B774`, section `F8B778` | `1Ch` native lock object | Loader preimage | `A5FA27` | `CD6E16` | `CE099A` |
| `F8B9A0` | Lock + 72 bytes | `E12AC8` | `A60038` | `CD6E2C` | `CE09A4` |
| `F8B858` | Lock + 72 bytes | `E12B10` | `A5FFAC` | `CD6E47` | `CE09B5` |
| `F8B8CC` | Lock + 40 bytes | `E12B58` | `A60015` | `CD6E62` | `CE09C6` |
| `F8BBA0` | Lock + 40 bytes | `E12B80` | `A5FFF2` | `CD6E7D` | `CE09D7` |
| `F8BADC` | Lock + 40 bytes | `E12BA8` | `A5FFCF` | `CD6E98` | `CE09E8` |
| `F8B8F8` | Raw 72 bytes, no lock | `E12CB8` | `A600AB` | `CD6F03` | None |

The checked `XLivePipeGlobalData` view spans VA `E12AC8` through exclusive
`E12D00`, RVA `A12AC8..A12D00`, exactly 568 bytes at raw file offset 10562248.
It verifies SHA-256
`20436ee57ad0c179e4cde3ffd80ea75b53339dea4d500cda75769177176eae21`
using actual CryptoAPI and checks requested address bounds. The original data
must remain alive and immutable while borrowed; the image binding must verify
the documented original PE identity. No generated replacement constants or
zero payloads stand in for these bytes.

The raw constructor `A600AB` first calls `A60095`, copying 72 bytes from
`D55230` in the checked protocol table view. It then forward-copies the caller's
72-byte source and returns the raw destination. Keeping both operations matters
when source aliases destination: the second copy sees the first copy's bytes.
`F8B8F8` has neither a hidden lock pointer nor a four-byte prefix.

## Registration and cleanup

The first six native initializers construct their target, call `BF6FF5` with
the exact cleanup address, pop its argument, and return the actual registration
result in EAX. Registration failure does not roll back construction. Their
entries in the original CRT initializer array at `CE33E0..CE33F4` prove the
order shown above; this is not inferred only from code address order.
The raw initializer is at CRT table slot `CE340C`, after five other raw-data
initializers. It has no cleanup registration and returns its constructor's raw
destination pointer in EAX.

`XLivePipeGlobalsStartupHost::register_cleanup_00bf6ff5` is the required genuine
CRT registration boundary. It receives the native callback identity, recovered
typed callback and retained owner; the application binds these to actual CRT
registration and returns its real result. The source does not copy CRT internals,
pretend registration succeeded, invoke callbacks immediately, or substitute a
private shutdown list. The other required host operation allocates a real value
lock, normally by forwarding to `create_xlive_pipe_value_lock_00a5fa5a()`.
Application-specific context-to-CRT callback binding remains integration work.

`initialize_xlive_pipe_globals` is a projection convenience that calls the seven
owned initializers in their original relative order and returns all six
registration results. It continues after nonzero results, preserving each
initializer's behavior. It does not claim to implement the intervening unrelated
CRT entries or the whole CRT dispatcher.

`CE099A` tail-jumps to `A5F91E`: it destroys the placement-owned acquisition
lock object and its actual section without freeing global storage. The five
encoded-value cleanup thunks reload their current global lock pointer and, when
nonnull, invoke virtual slot zero with flag 1. They do not clear the pointer or
payload. The public nonvirtual `Win32XLivePipeValueLock::native_section()`
accessor exposes the same actual section for protocol/framing contexts without
duplicating storage or casting private object layout. This is a new C++ object
projection, not binary ABI compatibility.

## Evidence and verification

Read-only analysis used `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`; final verified live count 63071. Assembly and
raw disk disassembly establish register inputs, forward copies, callback
arguments, return behavior and true missing starts. CRT-array xrefs and raw
bytes establish registration order. PE section/file extraction establishes
loader preimages and fixed-data identity. `PIPEIPC_*` and new descriptive
names are hypotheses; retained `CG_static_init_*` names are analyst inventory
names. Only actual CRT/Win32 operations are treated as standard boundaries.

All seven initializer entries are missing in Ghidra; their exact inclusive
terminals and exclusive ends are in `reports/xlive_pipe_globals.json` with
parent-only definition commands. The thirteen existing constructor/cleanup
functions have no flow gaps. The `CE099A` terminal is a five-byte tail jump,
not an invented local RET. No Ghidra mutation or interior-block definition was
performed by the worker.

Strict MSVC Win32 Release build, both existing CTests, all eight seed comparisons
and one ignored synchronous fixture passed. The fixture checks exact source
payloads, canonical reference identity, actual Win32 lock use, registration
ordering and failure without rollback, explicit cleanup, null/throwing lock
allocation, raw copy aliasing, and hash rejection. CRT registration is scripted
in the fixture; no real process-exit callback registration, pipe, thread,
network endpoint, DLL or game runtime was exercised.
