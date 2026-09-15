# Damageable-class Lua reader: complete body audit and category lookup

Addresses: 0087CA80 007149D0 0087C870 0087C920 0087B750 004B1400
00870CD0 00871BA0 00878EF0 0041DE40 00C96A7D 00DC8D9C 00DC8DC0.

## Result and evidence boundary

`0087CA80..0087D725` is **source absent**. This packet audits its entire
3,238-byte, 858-instruction body, all direct and indirect call instructions,
descriptor writes, native Lua-object lifetimes, numeric operations, and all
37 FH3 unwind states. There is no reader wrapper, callback, profile conversion,
or function declaration pretending to implement that body. Device and vehicle
reader/factory closure still requires it.

The independent `007149D0..00714A0E` category lookup is implemented in
`src/native_damageable_class_lua.cpp`. It borrows the actual null-terminated
pointer table and directly calls existing `compare_insensitive_00438e10`.
The old fixed-name predicate in `part_damage_reachability.cpp` remains a
separate legacy consumer. Names here are descriptive hypotheses.

The machine-readable report is `reports/native_damageable_class_lua_orch4.json`.
Its call rows come from the complete installed PE instructions, checked against
the reader's saved listing and live Ghidra. Live questions used `bsp.py ghidra`,
whose client verifies project `bsp`, program `/battlestationspacific.exe`, x86
language and image base against `config/target.json`; the configured project is
`C:/Users/sqz269/bsp.gpr`. No Ghidra mutation was performed.

## Original ABI and frame

`0087CA80`: ECX is the actual descriptor, stack argument is the live row
`LuaObject*`, and `RET 4` is at `0087D723`. Ghidra's one-argument fastcall
pseudocode omits that stack argument and is not an API contract. EBX, EBP,
ESI and EDI are saved. FS:[0] chains through handler `00C96A7D`; fixed local
allocation is E4h bytes. The source interface must borrow the actual descriptor,
row, current Lua owner, string-pool publication, CRT conversion mode and effect
and class registries. A copied table or copied descriptor breaks visible behavior.

Let `S` be ESP after the prologue and four saved registers. Descriptor is saved
at `S+40h`; row argument is at `S+104h`; EH state is at `S+FCh`. Lua-object
addresses are stable and reused only after cleanup:

| S offset | Role and lifetime |
| --- | --- |
| +94h | `Unique`, constructed once and destroyed last at `0087D706` |
| +44h | initial `Name`, then ordinary scalar fields, `Damage`, fake-key lookup temporaries |
| +80h | `Damage.Sections`, later `Emberkek` iteration key |
| +58h | section iteration key, fake-effects table, later crew value |
| +2Ch | section value, fake current row, then crew table |
| +6Ch | section scalar/effect fields and fake-effects indexed fields |
| +E0h | section `MshCategory` Lua temporary |
| +10h | mesh/comment/BigFire NativeString; later decimal-key buffer |
| +18h | section category NativeString; later crew integer key |
| +20h | temporary effect handle; later crew NativeString |
| +28h | temporary section effect handle; later fake-key ordinal |
| +A8h | 30h-byte temporary section |

`00B66420` takes the object from its **stack argument**, ignoring incoming ECX;
the iteration tests target the key, not the table. Tracked Lua destructors can
shift every subsequent live object's index. In particular, `Unique` remains live
across sections, fake effects and crew. Do not clean it up after writing +34h.

## Complete normal schedule and fields

The reader appends to existing containers. It does not clear them. All writes
are published as executed; a later failure does not roll back earlier fields.

| Sequence / key | Storage and exact behavior | Sites |
| --- | --- | --- |
| `Name` | GetString, duplicate C string, overwrite descriptor +54h without releasing its previous pointer | CAB8, CACA, CAD1, CADA |
| `Unique` | exact boolean predicate then GetBoolean; store normalized byte at +34h; preserve its live Lua object | CAFC, CB13, CB23, CB41 |
| `Mesh` | string-or-empty temporary, resize descriptor NativeString +38h/+3Ch, copy length bytes; release temporary through current raw pool | CB44..CBCA |
| `Comment` | same schedule at +58h/+5Ch | CBDB..CC61 |
| `HP` | number-or float32 100.0; x87 store to +48h before Lua cleanup | CC72, CC8B, CC98 |
| `Armour` | number-or +0.0; x87 store to +4Ch | CCB4, CCC9, CCCE |
| `ExplosionType` | get/probe/destroy; only if IsInteger succeeds, get the key AGAIN then GetInteger and store +40h | CCEE, CCFD, CD10, CD25, CD34, CD3D |
| `Damage` then `Sections` | nested exact table gates, native first/next iteration until key is unbound | CD59..D218 |
| `FakeExplosionEffects` | exact table gate, string keys `"0"`, `"1"`, ... until the first non-table row | D229..D55C |
| `Emberkek` | exact table gate, native key/value iteration, value is SoldierClass name and key converts to int32 | D56D..D6EF |
| final cleanup | destroy the still-live Unique object and restore FS:[0] | D6F4..D725 |

Addresses abbreviated in this and following schedule tables have prefix `0087`.
String copies resize before checking source length; empty strings still mutate
the destination. Mesh/comment copy exactly length bytes after resize establishes
termination. Section and crew inline constructors scan the C string and copy
length+1 bytes, with their distinct EH-state transitions. No extra null guard is
present before those scans.

### Damage.Sections: descriptor +18h, 30h-byte rows

The 10h-byte vector header has allocator/opaque +0, begin +4, end +8 and
capacity end +Ch. Thus descriptor begin/end/capacity are +1Ch/+20h/+24h.
For every Lua value, the reader zeroes all 30h temporary bytes and writes
float32 **10.0** at temporary +28h/+2Ch, then calls `0087C870` to append.
It destroys that temporary before obtaining the appended row as end-30h.
It runs three native checked-iterator failure tests before field loading.

| Row offset | Value / schedule |
| --- | --- |
| +00h | 0 in input temporary; copy constructor `00878B40` supplies the native row vptr, which needs its own source closure |
| +04h | category result from `007149D0` over copied `MshCategory`; this is not the Index field |
| +08h | `Index`: GetNumber rounds Lua double to float32, then `00BF7420` converts ST0 according to current 0109EEA4; store EAX |
| +0Ch..+23h | zero input bytes, no Lua writes in this reader; later model binding fills position data |
| +24h | intrusive effect pointer resolved from FireEfx, with fallback by name `BigFire` |
| +28h | `FailureChance`, default **-100.0f**, x87 FDIV by the binary64 **100.0** at D7A220, then FSTP float32; default result -1.0f |
| +2Ch | `FailureDamageThreshold`, default **-1.0f**, FSTP float32 |

`MshCategory` GetString happens at `CEDB`, copied into the raw native string.
Its Lua object is destroyed before reading `Index`; the string remains alive
until after both failure fields. When the copied string's data pointer is null,
the caller passes the current empty bytes at `00F878E0` to category lookup.

FireEfx calls IntegerOrDefault(0), then `00870CD0(out,id,1)` at `CFAF`.
Assignment first publishes the new pointer, increments new +4, decrements old
+4 and calls old vtable[0] with no explicit arguments at zero. Equal pointers
skip all of that. Release of the acquired temporary precedes FireEfx Lua cleanup.
Only then does the reader reload row +24h and test null (`NEG/SBB/TEST E17BF8`,
not a dereference of E17BF8). A null result constructs `BigFire`, acquires by
name with flag 1, repeats assignment and temporary release, then frees BigFire.
The new pointer may itself still be null.

The failure-chance division stays in x87 between the Lua float-return and final
float store. SSE division, pre-rounded arithmetic, ordinary integer casts,
or a cached CRT conversion flag need separate proof for NaNs, infinities,
large values and control-word behavior.

### FakeExplosionEffects: descriptor +08h, 10h-byte rows

Vector begin/end/capacity are descriptor +0Ch/+10h/+14h. The outer Lua table
is live at S+58h. Decimal keys begin at **0**, formatted by sprintf `%d`;
lookup temporary is assigned into a persistent row object at S+2Ch, then
destroyed. Missing/non-table rows terminate the loop, leaving later keys unread.

Before reading each row's values, `0087C920` resizes to current_count+1 with a
**by-value 16-byte record**: float -1.0 at +0, integer -1 at +4, untouched
stack bytes at +8, null handle at +Ch. That +8 must not be silently zeroed.
The callee owns destruction of this by-value handle and returns with RET14h.

| Lua index / order | Appended record offset | Conversion |
| --- | --- | --- |
| 1 | +00h | GetNumber, float32/x87 store |
| 2 | +08h | GetInteger |
| 3 | +04h | GetInteger |
| 4 | +0Ch | GetInteger then effect-ID acquire flag1, intrusive assignment/release |

For indices 1..3, lookup occurs first, bounds checks next, begin is captured,
getter is called, and the field is stored. For index 4, getter and effect
acquisition precede bounds checks and the begin reload. Each temporary is
destroyed before the next indexed lookup. This distinction matters if Lua or
effect loading reenters and mutates storage. `%d` uses a signed 32-bit ordinal;
the original ADD wraps rather than invoking C++ signed-overflow behavior.

### Emberkek: descriptor +60h tree, 58h-byte nodes

The native key is converted with GetInteger at `D645`; the native value is
GetString at `D5F4`, copied to a raw NativeString before conversion. The factory
`004B1400` is called at `D652`, then `0087B750` obtains the map value and the
returned pointer is written at mapped-value +40h (`D665`). No old pointer
release or extra retain occurs in this caller. The factory invokes virtual +Ch
before returning. Describing that virtual as retain requires separate evidence.

Tree +4 is sentinel; node links are +0/+4/+8, signed int32 key +0Ch, 44h-byte
mapped value +10h..+53h, color +54h and nil +55h. `0087B750` uses a signed
lower-bound walk and inserts on miss. Its stack default mapped value is copied
as 17 dwords of **uninitialized bytes**, not a zero-initialized struct. The
assigned SoldierClass pointer ultimately occupies node +50h. The other mapped
bytes are for later model binding; this reader does not fill them.

## Exception and cleanup audit

Handler `00C96A7D` loads FH3 info `00DC8D9C` and jumps `00BF6B43`.
Magic is 19930522h, maxState is 25h (37 states), unwind map is `00DC8DC0`,
try/catch and IP maps are empty, EH flags are 1. The report enumerates every
state's parent, funclet, target and frame offset, and every state store in the
ordinary body. State 10h has a table entry but no ordinary state-store site;
states 10h/11h share string-cleanup funclet `00C969A4` with different parents.

All Lua funclets target `00B67700`; native-string funclets target `0041DD20`.
Three effect-handle states (14h,16h,20h) target `0041DE40`; section temporary
state Eh targets `00878EF0`. These last two ordinary bodies are additional
dependencies absent from the reader's 35 direct callee set. `00878EF0`
publishes D0DF04, releases +24h and clears it; `0041DE40` releases [this] and
clears it. Both use InterlockedDecrement and zero-argument vtable[0] at zero.

Normal state is lowered before every cleanup. Completed descriptor/vector/map
writes survive exceptions. There is no descriptor rollback and no whole-loop
transaction. Matching only scope-based destruction is insufficient: inline
string allocation, copied unknown bytes, handle publication, Lua-index shifts,
and cleanup calls that throw all have observable ordering. Original FH3
personality, longjmp, faults, and double exceptions are not validated here.

## Provider assessment and exact next source packets

Existing native Lua providers in `native_lua_objects.hpp` supply the actual
14h stack objects, tracked owner storage, accessors and iteration. Native string
header/pool providers exist; a full reader must use their raw-pool context rather
than temporary host strings. Duplicate `00438E40` is available but uses a host
allocation boundary; BF7420 supplies a qualified original-instruction numeric
fragment. Memcpy, memset, sprintf, interlocked calls and invalid-parameter
dispatch remain the explicit CRT/Win32 boundaries described by their providers.

The following are concrete source packets, not claims that their dependencies
are already implemented. Each must expand actual direct **and EH** callees and
lease every address/file before writing. Files listed below do not yet exist.

| Packet | Entry bodies, size, ABI/EH | Required source closure and proposed files |
| --- | --- | --- |
| section vector | 87C870..87C910,161B, ECX vector/stack row*, RET4, no own EH; 87C2D0..87C37C,173B, checked iterator insert; 87B950..87BC40,753B, ECX vector and 4 stack words, RET10h, handler C967B8 | `native_damageable_section_vector.hpp/.cpp`; copy878B40, uninitialized fill8798F0, insertion87C2D0/87B950, current observed helpers744070/876A10/878AC0/879390/8794F0/8798C0/87AD20/87B610/87A580; follow their real bodies and EH actions, reuse canonical allocation/invalid/handle boundaries |
| fake-effect vector | 87C920..87CA14,245B, ECX vector/stack size+16B by-value record, RET14h, handler C968E8; 87C380..87C633,692B, ECX vector/4 stack words, RET10h, handler C96818 | `native_damageable_fake_effect_vector.hpp/.cpp`; resize/insert/erase87C920/87C380/87B8F0, 8769B0/878C40/879330/878820/879870/8799F0/87AB80/87B5B0/87B5D0/87BC50/87A510. Reuse 87AB30 once the construction packet's actual cleanup is integrated |
| crew tree insertion | 87B750..87B7E2,147B, ECX tree/stack int32*, EAX mapped-value*, RET4, no own EH; 87B260..87B41B,444B; 87ADB0..87AE68,185B | `native_damageable_crew_tree.hpp/.cpp`; signed lower-bound/insert-hint and rebalancing,876340/8767A0/87A5F0; existing8772B0 iterator and sibling877FA0 allocator may be reused after source review. Preserve58h raw nodes and uninitialized44h value |
| SoldierClass resolution | 4B1400..4B1537,312B, ECX NativeString*, EAX class*, RET; handler C64A93 | `native_soldier_class_resolution.hpp/.cpp`; singleton4B1330/4B11A0, raw map4B0CB0/4AFE50 and its dependencies, constructor4B12A0/489E60/443E20, actual class reader virtual+4 and virtual+Ch. Borrow currentE188A8+1A0C and pool. Resolve publication, cache reload and lifetime before integration |
| effect handle ownership | 870CD0..870CF9,42B, ECX out/EDX ID/stack flag, EAX out, RET4; 878EF0..878F20,49B; 41DE40..41DE68,41B, both ECX/RET | Close actual effect manager/component providers, then ID wrapper and the two cleanup bodies. Existing871BA0 calls real manager/name/ID helpers but its context still requires component services; a wrapper around it does not close the reader |

The fake and section vector insertions contain their own catch paths, including
cleanup and rethrow `00BF6885`. Live feature counts/callee summaries and the
snapshot call graph omit some restored instructions/edges: current listing has
281 instructions for 87B950 and259 for87C380, versus feature counts259/238.
87BC0A and87C5FC are internal branch targets, not callable dependencies;
744070 is a real CALL at87BA40. Build packet edge sets from current complete
instructions, and include catch blocks before accepting a source closure.

A ready small prerequisite for the section-vector packet is `00878B40..00878BAB`
(108 bytes): ECX destination, stack source, EAX destination, RET4, no own EH.
It writes D0DF04, copies +4/+8 as integers, copies +0Ch..+20h and +28h/+2Ch
through **eight ordered x87 FLD/FSTP pairs**, and zeroes destination +24h before
reading source +24h and retaining it. This is not memcpy, including on aliased
source/destination or signaling NaNs. Keep it with the section-vector files;
the matching `00878EF0` temporary destructor is another bounded prerequisite.

### Independent category lookup fidelity

Native `007149D0` checks current table[0], reloads it for compare, and on misses
increments a 32-bit index, checks the next pointer, then reloads it for compare.
It returns the first matching index or -1. The added source retains these loads,
has no15-element limit, and allows the supplied table to change between calls.
The read of each cell is volatile; this is not a synchronization guarantee.

`00438E10` preserves equal-pointer and null ordering before host `_stricmp`.
The old `std::tolower` loop is not automatically equivalent under the original
CRT's locale and byte rules. The new body directly reuses the stronger existing
provider and records the remaining **host CRT/current-locale** boundary.
No floating arithmetic occurs in this lookup, so NaN handling belongs to the
reader's numeric dependencies, not this comparison.

## Validation

The report records the strict Win32 build result, exact original body hashes,
instruction/call counts, and live call-verifier outcome. No fixture or game
validation is claimed for the full reader; it has no source body. Category
lookup is a new source interface rather than a drop-in original-ABI replacement.

`scripts/build.ps1` passed in default strict Release mode, including both existing
CTest tests. The live verifier checked168 call rows with0 failures:131 reader
calls,1 category-lookup call and36 EH cleanup jumps. The compiled Win32 object
exports the new lookup and references the existing00438E10 comparison symbol.
No new tests were added and no original-reader execution is claimed.
