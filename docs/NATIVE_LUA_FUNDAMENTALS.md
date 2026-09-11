# Actual Lua fundamentals cache

Addresses: `00884770`, `00B68340`, `00B667D0`, `00B66B80`.

The cache now has its actual 12-byte storage, source allocation and singleton
lifetime. `NativeLuaFundamentalsOwner` aliases the existing bootstrap view so
producer and consumer share one layout. `src/vfs_lua_scripts.cpp` remains an
earlier `std::string` projection; it is not used by this implementation.

| Routine | Native ABI and disk extent | Coverage |
| --- | --- | --- |
| `00884770` | No arguments; EAX owner; RET; `00884770..0088482C` | complete |
| `00B68340` | ECX raw owner; EAX same owner; RET; `00B68340..00B68458` | complete |
| `00B667D0` | ECX owner; RET; `00B667D0..00B667E0` | complete |
| `00B66B80` | ECX owner, stack flags; EAX original owner; RET4; `00B66B80..00B66BC5` | complete disk body; two stored-listing gaps |

These are readable C++ interfaces with explicit external context. Complete
coverage describes the four bodies and their inspected cleanup transitions;
it does not supply original FH3 exception identity or a binary replacement.

## Construction and source contract

The producer writes `D62C18` at `+00` (`B6836F`). Only the successful gate
branch writes the payload: `B683EB` stores the low DWORD of slot30's EDX:EAX
length at `+08`; `B683FE` stores the allocation at `+04`. The source allocation
is exactly that low DWORD via `BF55BE -> BF681B`, including a zero-byte request.
There is no extra byte or terminator. The path is a separate native string:
resize to `18h` with preserve1, then copy its current length+1 bytes from
`Scripts\fundamentals.lua`. Allocation and release use the supplied existing
`NativeStringStorage`; `ActualNativeStringPoolStorage` connects the real pool.

At `B683B7` the current VFS owner's callable slot04 receives ECX manager and
two stack arguments, path and mode2. The returned stream is unchecked. Slot18
returns the gate byte. A zero gate leaves both payload fields untouched,
retains the returned stream reference, releases the path and returns the owner.
It does not raise the missing-file exception used by the older projection.

On a nonzero gate, slot30 is called once. Slot24 receives the allocation,
current owner size and a pointer to an uninitialized DWORD count. Its return
and count are ignored, including a short read. A real `InterlockedDecrement`
at stream+04 follows. Only zero invokes the stream's current slot00, with
ECX stream and no stack argument. Path cleanup follows stream cleanup.

The actual VFS and stream implementations remain external. The required
objects must provide callable original-ABI tables; numeric table identities
alone cannot satisfy this boundary. This packet adds no VFS fallback or
replacement script content. The existing actual Lua file loader consumes the
same slot conventions. It also has different early-return and repeated-length
behavior, so the cache does not call that loader as a shortcut.

Both complete caller sets were checked. `00884C74/00884C7C` in `00884BE0` and
`00B6A323/00B6A32B` in `00B6A020` call the getter twice: capture first result's
size08, then read second result's data04. This confirms the prior bootstrap
contract independently from the writes, rather than inferring layout from
the consumers alone. The callback adapter plugs into `NativeLuaBootstrapInputs`.

## Lifetime and failure behavior

The getter borrows the application's one `0108FF1C` slot and canonical
`SingletonLifetimeDomain`. It returns a cached pointer directly, otherwise
captures the first manager's section10, enters it and increments recursion18,
checks publication again, allocates exactly `0Ch`, constructs without value
initialization, publishes, looks up the manager again, and registers the
current published pointer. Unlock uses the captured section. Return reloads
publication after leaving it. It does not add registration rollback.

`NativeLuaFundamentalsLifetimeBinding` composes D62C18 destruction into that
same domain and forwards every other owner and validation call to required
callbacks. It owns no singleton slot or separate domain. It can be composed
with `NativeStringPoolLifetimeBinding`; both bindings must outlive shutdown.

`B66B80` writes D62C18, captures a nonnull data pointer, frees it through
`BF6989 -> BF65AC`, and then clears data04. It clears publication
unconditionally, writes base CE3818, and frees the owner when flags&1. It
returns the original address and never clears size08 or unregisters itself.
The failed-open branch therefore requires the actual allocation preimage to
be meaningful when later destroyed; this implementation does not zero it.

The constructor's FuncInfo at `DFA3B4` has two states: state1 releases the
path via `CC1538 -> 41DD20`, then state0 invokes `CC1530 -> B667D0` to clear
publication and set CE3818. State1 starts just before the VFS call. Unwind
does not release stream or source bytes, even after allocation/read failure.
The getter's FuncInfo `DC9440` frees an unpublished raw allocation in state1
(`C970B8 -> BF65AC`), then destroys its captured guard in state0
(`C970B0 -> 411EE0`). Registration runs in state0. The C++ implementation uses
`__finally` for these transitions; allocation/OS/exception identities remain
the existing host service boundaries.

## Instruction and boundary evidence

The exact Ghidra project/program is verified by every `bsp.py ghidra` query:
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Worker operations are
read-only. Nine code, literal and EH spans were compared with the installed
PE bytes; the report records hashes and inclusive/exclusive ends.

The stored scalar-destructor envelope ends at B66BC5, but its listing skips
`[B66B96,B66BA0)` and `[B66BBD,B66BC0)` after mistaken free call-return edges.
Disk decoding establishes `ADD ESP,4; MOV [ESI+4],0` and `ADD ESP,4` respectively.
The full disk extent is `[B66B80,B66BC6)`. The getter state1 unwind also has
stored end C970C0 while disk `[C970B8,C970C3)` includes `POP ECX; RET` after free.
No Ghidra repair or saved-body extension is claimed by this worker.

Stack cleanup confirms memcpy's three arguments (`B6839D ADD ESP,0Ch`),
allocation's single argument (`B683F3` and `8847D1 ADD ESP,4`), scalar flags
(`B66BC3 RET4`) and each free's single argument in the disk gaps. Native
string resize ends RET8; pool return BD1510 ends RET0Ch while its preceding
pool getter consumes none of those three pushed arguments. The virtual
manager/stream boundary uses ECX and its observed stack arguments, matching
the existing actual Lua-file interface. All EDI/ESI/EBX writes in both full
listings were inspected: EDI remains the constructor owner, ESI its captured
stream (getter: captured section), and constructor EBX remains zero.

## Verification limits

One ignored native differential fixture relocates and executes all four full
disk bodies on MSVC Win32. It compares five constructor cases (closed stream,
zero length, ordinary source, nonzero high length DWORD and retained stream
reference), base cleanup, scalar flags0/1, and cached singleton behavior.
Original calls use ABI adapters to the same real allocation/string-pool and
lifetime services; fixture VFS streams are explicit test services. Source
bytes are fixture data, not the installed game's fundamentals asset.

The fixture composes the actual string-pool owner and fundamentals binding
with a single real singleton domain, checks shutdown of both owner types,
and exercises a rebuilt read exception without adding native byte/stream
rollback. Original FH3 throwing paths are inspected rather than executed.
The strict Win32 build, both existing CTests and the focused fixture passed.
All 17 direct report call rows passed live verification; the eight indirect
rows retain their explicit slot/import boundaries. No permanent test suite,
runtime game integration or game validation is claimed.
