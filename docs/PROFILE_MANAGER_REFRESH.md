# Profile manager checkpoint refresh
Addresses: 004374f0, 00425c20, 004c1e90, 00435cc0, 00426250, 00436c50, 00436690, 00437490, 007fc2c0, 007fbed0, 007f8ca0, 00436710, 00bd53c0

`ConcreteProfileArchiveManagerServices` supplies the two remaining manager effects
in `MissionProfileArchiveHost`: the actual shared hints owner's raw `+08h` value,
and checkpoint refresh through `004374f0`. Source is `src/profile_manager.cpp`;
public contracts are in `include/bsp/profile_manager.hpp`. Descriptive names are
hypotheses; save-table and field names below are literal binary strings.

The refresh can repopulate `ProfileResetState::transient_records_94` immediately
after `007fdf00` clears that collection. It does not clear `+94h` itself. The
profile archive reader clears it at `src/profile_archive.cpp` immediately before
calling the manager; other refresh callers preserve records not overwritten by
the newly loaded checkpoint table. A failed name/query gate therefore leaves an
already populated collection unchanged, while the archive-reader path leaves it
empty. The game Lua table is replaced unconditionally in both cases.

## Established normal flow

1. Create an empty `game+1A0Ch` Lua global named `BSP_Chk_Save` (`00CE4124`), then
   retain a Lua reference to that particular table. This precedes every gate.
2. `007f8ca0` compares profile `+34h` with the empty NativeString constructed by
   `00436710`. Empty/nonempty string headers decide this particular comparison;
   its normal result is exactly whether the profile save-name header is nonempty.
3. Ask storage singleton `0109CECC` through virtual `+1Ch`, passing the profile
   save name and integer kind **2**. This is not the profile I/O API's Boolean
   argument; the PC storage packet identifies 2 as the game-storage category.
4. If present, free storage buffer `+30h` and zero it. Close/reset storage Lua
   owner `+38h` through `00b65e80`. Request `00bd3d70(name, 2)`.
5. Call the existing `006adb50` driver with callback `00bd53c0`. That callback is
   exactly one `RET` byte (`C3`), followed by `CC` padding. Its empty implementation
   is established native behavior, not an invented success or unresolved stub.
6. Immediately inspect storage Lua globals `BSP_Chk_Save`. If it is not a table,
   stop. Otherwise iterate it using actual Lua 5.1.1 `lua_next` order, copy each
   entry into the **retained** game table, and upsert the corresponding `+94h`
   record. No storage-result check exists between the driver and this lookup.

The driver may return while an interactive continuation remains installed.
Refresh nevertheless proceeds with the immediate lookup. The retained callback
does not perform another import when the prompt later completes. The caller must
supply the actual shared driver state and prompt/render host. A null Lua state
raises an explicit host error; it is not treated as a successful empty archive.

The destination table reference is captured before storage work. If a storage
callback replaces the same game global, the native import still writes the
earlier retained table. The C++ implementation preserves that reference on the
Lua stack instead of performing a fresh global lookup after the callback.

## Records and recursive copy

| Record word | Binary key | Missing/wrong-type default |
| --- | --- | --- |
| `words[0]` | `__difficulty` at `00CE4160` | 0 |
| `words[1]` | `__unlockFrom` at `00CE4150` | -1 |
| `words[2]` | `__unlockTo` at `00CE4144` | -1 |
| `text` | `__unlockName` at `00CE4134` | empty string |

`00436690` constructs exactly these three scalar defaults and an empty owned
string. `00437490` copies that 14h record by value for `007fc2c0`. The latter adds
94h to profile ECX, finds/inserts through `007fbed0`, writes the three words, and
copies the text. Its tree comparator reaches
`BSP_NativeString_LessCaseInsensitive`; C++ upserts use the Windows CRT's
case-insensitive C-string comparison, preserve the first key spelling, and do
not claim to reproduce native tree order or allocation layout. Empty/nonempty
headers remain distinct even when a nonempty string begins with NUL.

`00436c50` recursively copies Boolean, number, string and table values across
Lua states. String keys use owned NativeString construction, hence C-string
prefixes; non-string keys go through `00b66290` integer conversion. New child
tables are assigned before their entries are copied. Other value types are
ignored. Shared child tables are independently copied; no alias-preservation
mechanism exists. Cycles have no terminating native traversal.

Assembly `00436e94..00436f61` and its numeric-key arm spill numbers to float32,
compare that number against the float32 result of the CRT floor-like helper
`00bf85b0`, then choose the integer or float setter. The helper's negative
fractional branch and equality test were inspected; it remains under its
existing CRT name. `00b66290` and `00b66380` also spill to float32 before CRT
`__ftol`. Integer defaults require actual Lua type NUMBER: a numeric string is
not accepted. String defaults require actual type STRING.

The host implements finite signed-32-bit integer conversions and normal acyclic
Lua archive tables. It rejects exceptional/out-of-range conversions and cycles,
which otherwise have undefined C++ conversion or nonterminating recursion. Each
outer value must be a table, and its key must have a C-string representation;
the native path blindly indexes and constructs from those values. Malformed
outer values receive explicit host errors rather than invented empty records.
Lua allocation/panic behavior, malicious metatable side effects, original x87
exception modes, allocator addresses, native LuaObject layout and SEH are not
claimed equivalent. The normal archive domain uses plain generated tables.

## Ownership, singleton access and hints

`00425c20` reads/publishes `00E17680`, double-checks it under the singleton
lifetime lock, allocates 1Ch bytes natively, calls `00435cc0`, then registers the
published pointer, including the native null-result path. `00435cc0` sets vtable
`00CE411C` and constructs an unbound LuaObject at `+04h`. The manager ECX passed
to refresh is only threaded through recursive `00436c50` calls; those calls do
not read manager fields.

`004c1e90` uses the same locking/registration pattern with `00E17664`, native size
50h and constructor `00426250`. That constructor writes `+08h=0` at `00426285`.
The archive reader checks the actual current field before processing
`AllSeenHints`; the adapter therefore reads shared mutable state, not a constant
zero. The interpretation of that field and its other update paths remain open.
Other constructor fields are not needed by this service and are not projected.

The two C++ getters use actual nothrow allocations of the partial host owner
types, explicit caller-owned publication slots, and the existing
`SingletonLifetimeManager` locking/registration contract. They do not allocate
native-sized fake objects. The configured lifetime manager's deletion callback
must own destruction of the registered host objects. Native deleting destructor
dispatch and all other owner methods remain separate work; no binary drop-in
singleton replacement is claimed.

`ProfileManagerRefreshHost` is the actual storage/game boundary. The PC storage
backend supplies query, read, buffer ownership, archive close and raw Lua state;
the game supplies its real Lua state. The existing storage driver supplies all
continuation and prompt ordering. The actual storage Lua bootstrap (standard
libraries, engine bindings and scripts), UI and rendering remain required host
services, with no synthesized successful bootstrap.

## Evidence and ABI

The worker used `bsp.py ghidra` queries, whose client verifies project `bsp` and
program `/battlestationspacific.exe` before each batch. The saved project is
`C:/Users/sqz269/bsp.gpr`. Assembly was inspected for register arguments, x87
spills and the false no-return gap. Primary independently verified and repaired
the six bytes after the free call at `00437653..00437658`: `ADD ESP,4` and
`MOV [ESI+30h],EDI`. The whole refresh body ends at RET `004379be`. Primary also
defined the disk-verified one-byte function at `00bd53c0`. Refreshed exports live
under ignored shared `exports/bsp/functions/`.

| Address | Native ABI | Reconstruction boundary |
| --- | --- | --- |
| `004374f0` | ECX manager, no stack args, RET | complete normal flow with actual Lua/storage services |
| `00436c50` | ECX manager, stack destination/key/value LuaObjects, RET 0Ch | recursive normal table domain |
| `007fc2c0` | ECX profile, stack key pointer + 14h by-value record, RET 18h | owned record upsert, host collection |
| `00436690` | ECX record, RET/EAX this | constructor defaults represented by import |
| `00437490` | ECX destination, stack source, RET 4/EAX this | host value/string ownership |
| `00425c20`, `004c1e90` | no arguments, RET/EAX owner | getters, partial host owners |
| `00435cc0`, `00426250` | ECX owner, RET/EAX this | only fields needed by this packet |
| `00bd53c0` | no args, RET | exact empty callback |

Build, fixture and seed verification results are recorded in
`reports/profile_manager_refresh.json`. These are reconstructed and build-tested
host interfaces, not ABI-compatible hooks or game-validated replacements.
