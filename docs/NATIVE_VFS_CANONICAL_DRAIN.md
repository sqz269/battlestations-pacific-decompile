# Actual VFS owners in canonical singleton drain

Addresses: 00BD0400 (existing source adapter only).

The raw singleton deletion adapter now accepts the actual derived VFS manager
profile D68D04 and the FileStore factory secondary profile D688B0. It borrows
the existing `NativeVfsManagerLifetimeContext` and `NativeFileStoreFactoryContext`.
The bindings grow from 36 to 44 bytes by appending two zero-initialized pointers;
all previous fields, profile cases and missing/unsupported-binding errors remain.
This extends an existing reconstructed body; it adds no complete routine count.

| Existing routine | Coverage | Original ABI | Source boundary |
|---|---|---|---|
| BD0400..BD04C4, 197 bytes | Complete normal drain schedule and existing source unwind cleanup | ECX raw14h manager, no stacked args, RET | EDX additionally borrows the explicit deletion bindings; finite profile dispatch replaces native arbitrary virtual dispatch |

Live bytes in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe` agree with
the installed PE, SHA256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Every live capture checks the project and program first. No Ghidra definition,
annotation or flow mutation is part of this packet.

| Current profile | Producer and native slot zero | Actual source dispatch |
|---|---|---|
| D68D04 | BEDA8A writes the completed derived owner; D68D04[0] = BEDAC0 | `delete_native_vfs_derived_manager_00bedac0(owner, flags, context)` calls the complete BE1F60 destructor, frees for bit0 and returns the captured owner |
| D688B0 | BE532F writes factory+4 after its transient D688AC profile; D688B0[0] = BE5340 | `delete_native_filestore_factory_secondary_00be5340` subtracts four before BE5790; publication clear, secondary CE3818, primary CFE9F4, optional primary allocation free; cache+8 stays untouched |

Both scalar deleters inherit the single DWORD flags / RET4 contract. BD0400
passes the native constant 1. The source adapter forwards the entire flags value;
the consumed deleters implement bit0 ownership. These are borrowed source
contexts, not original ABI additions to the game's objects.

## Pop, cleanup and reentrant registration

BD0478 reduces the live vector end before BD0485 calls the captured owner's
current slot-zero target. BD0489 then counts the live vector again. The loop
does not cache its original count or defer newly registered owners.

The focused scenario constructs the actual derived VFS manager, canonical
string pool, physical factory and FileStore factory through their real source
constructors/getters, all using one raw 01090AA0 publication. Its initial vector
is `[VFS, pool, physical+4, FileStore+4]`. The VFS owner contains one pooled short
name and one large string in its actual string vector. Its six member heads and
factory-list node are real allocations. FileStore cache+8 holds a non-owning
sentinel to check the deleter's untouched-byte contract.

Drain deletes FileStore, physical factory and the original pool in reverse order.
Pool destruction clears its publication and sets the real 01090AA4 gate to 1.
The next pop removes the VFS owner before invoking its destructor. A string
release calls the real 419CC0 getter even with returns disabled: this constructs
and registers a replacement pool into the same now-empty manager. Large strings
still free their CRT allocations; small returns leave the replacement ring
untouched while the gate stays set.

BDA790 subsequently asks BCFCA0 to remove the current VFS publication. The VFS
slot has already been popped; BCFCA0 scans the replacement pool slot and returns
without modifying it. VFS publication clears, its captured critical-section
depth returns to zero, and its allocation frees. BD0400 recounts and deletes
the replacement pool, then releases its section and vector. Manager allocation
and publication remain caller-owned until the fixture's final caller free.

## Verification

The strict MSVC Win32 build and both existing CTests passed.
The first configure attempt failed fetching Lua over TLS; its log is retained.
The subsequent build uses physically copied, hash-verified existing dependency
sources through local CMake cache overrides. No dependency source changes or new
library ports are introduced.

One focused native/source composition fixture passed with 258 checks. Both sides
produced identical 6148-byte results: 17 observed allocations and 17 drain events,
including the replacement pool allocation. Final owner publications are null,
the shutdown gate stays 1, and no observed allocation remains after caller free.
The FileStore cache sentinel and final cleared VFS member heads were checked at
the real allocation-free boundary. Two earlier fixture-only missing-declaration
compile failures are retained; neither executed.

It replays the unchanged original 197-byte BD0400 body at a uniform 30000000
shift, checking all seven direct calls and 17 relative transfers; its four
distinct external callees and four numeric virtual targets
bridge to actual reconstructed source. Original D60000 PE bytes retain the
profile slots and six path literals. A dedicated launcher reserves only the
BD0000, BE0000 and D60000 pages in its own suspended probe; the child verifies
their allocation bases, extents and ownership markers before filling them.

No VFS owner is deleted before drain, no alternative lifetime domain is used,
and no deleting callback is replaced by a no-op. Empty logger/mount containers
make the separate manager virtual calls unreachable in this scenario; their
required bindings fail if unexpectedly called.

The fixture observes real CRT allocation/free, raw registration counts, current
publications, gate state and critical-section depth. Inputs include the actual
source and BSP headers, selected linked objects checked against archive members,
libraries, probe/launcher sources and binaries, and native bytes. Inputs are
physically retained before first execution and rehashed afterward: all 506 inputs,
including 123 linked objects, remained unchanged. Passing attempt03 has manifest
SHA256 `af68d73751773b58e6abbbba8f325c9096ca7e63fc081626a2edcfca5cc09404`.
The result SHA256 is
`0ec4663927b553e8d1e3cf1827a3aeaab94c5fa82d9b2f564eb2b1405ddfaf36`.
Every failed attempt remains separate. The reusable local driver takes `--repo` and
`--attempt` to relink a fresh immutable attempt against another current build.

Original FH3/SEH handlers, exceptional cleanup, arbitrary virtual targets,
original CRT identity, binary ABI replacement and game startup/gameplay remain
unvalidated. Successful bounded composition does not establish those properties.
