# Actual input configuration cleanup

Addresses: `00698730`, `00A93920`, `00A92260`, `00A93880`.

The packet supplies the game-body cleanup and unregister schedules on the same
actual embedded input configuration, shared action publication, actual24h owner,
actual30h actions, actual34h bindings and actual24h listeners. It does not create
a settings projection or a second owner. Two checked-STL operations remain
explicit required storage providers; this is not complete library or application
configuration ownership.

| Entry | Native ABI | Coverage and exact end |
| --- | --- | --- |
| 698730 | ECX configuration; no stack input/result; RET | Complete game-body schedule with explicit checked-storage contracts; RET6988E9, inclusive6988E9, exclusive6988EA |
| A93920 | ECX action owner; stack uint32 index; no result; RET4 | Complete wrapper; RETA93934, inclusiveA93936, exclusiveA93937 |
| A92260 | ECX action owner; stack uint32 index; EAX0/1; RET4 | Complete raw query; RETA9227D/A92282, inclusiveA92284, exclusiveA92285 |
| A93880 | ECX action record; no stack input/result; RET | Complete unregister body with existing storage/listener services; RETA938D6, inclusiveA938D6, exclusiveA938D7 |

These explicit C++ interfaces are not original register/stack/FH3/SEH replacements.
The four routines own no local EH rollback. A reached provider or real CRT
exception preserves completed changes. Required ranges must be valid for the
native operation, including any state repaired by a returning CRT handler.
Asynchronous races, arbitrary hardware faults and native argument-spill aliasing
are outside this source contract.

## Producer and storage evidence

The 698680 constructor is called from game constructor4DDB90 at4DDBE2 with
ECX=game+3C. Its unchanged ECX reaches B66BD0 at698699: the existing actual4C8h
NativeLuaStateStorage is at configuration+0. It is **not** at4CC. The independent
owner packet establishes the524h embedded span ending at the next game member
at560h. Cleanup does not construct that span or its Lua base.

698680 explicitly initializes only the pointer triplets at4D4/4D8/4DC,
4E4/4E8/4EC,4F4/4F8/4FC,504/508/50C,514/518/51C. The checked-vector leading
words at4D0/4E0/4F0/500/510 remain untouched. Byte4C8,4CB,4CA,4C9 and520 are
cleared separately. The constructor leaves4CC untouched;699BCD later writes
its low byte from the fourth modifier argument. None of these flags or the Lua
base is changed by698730.

698A10 appends actual10h checked DWORD vectors to the outer vector at4D0 using
698980, whose placement-copy path698040 invokes557590. That copy constructor
initializes row+4/+8/+C as begin/end/capacity and leaves row+0 alone. The inner
row is a vector, not a semantic record with an ID at+0. Four flat checked DWORD
vectors at500,510,4E0,4F0 are populated by697F40 at698DE6,698E87,698F28,698FC9.
The original action/owner producers A93DA0/A93940 and binding/modifier storage
providers already establish the separate count-based12h array headers.

## Unregister and publication order

A92260 compares the index against owner+8 **unsigned** before reading owner+4
and the selected30h record's registered byte00. It returns full EAX0/1. It does
not test enabled byte01. A93920 has no bounds check: it derives the current
action address with DWORD index*30h and calls A93880.

A93880 first resizes the context vector ataction+4 to zero, then the binding
vector ataction+10 to zero. Those outer allocations/capacities remain retained;
existing binding destruction releases their nested modifier buffers. It tests
listener2C and reads that cell again. On a nonnull captured listener it performs
InterlockedDecrement on actual+4; zero invokes the current captured profile's
slot0 with ECX listener and **no flags argument**. Slot0 may reach the existing
BD30E0 finite deletion binding. The action's listener cell is cleared after
return and again by the outer nonnull arm. Only then is registered00 cleared.
Enabled01, previous/current values/latches and the owner context words are not
reset. If listener dispatch throws, later clears do not occur.

As in the other recovered action routines, `TEST(-!!listener,F8BC00)` is a
nonnull pointer test. It does not load F8BC00. The IAT slotCE2220 contains RVA
A056E8 in the saved image; the hint/name entry atVA E056E8 is
`InterlockedDecrement`, confirming the32-bit stdcall reference operation.

698730 iterates every index0..128h inclusive. Each query first calls the real
004BEC00 accessor using the borrowed NativeInputActionOwnerContext. A true
query calls that accessor **again** before unregistering. A listener callback
may replace the action publication; subsequent lookups use the new pointer.
The application must supply the same actual F8BBF8 publication and existing
raw lifetime access, together with its canonical records/listener context.

## Configuration container schedule

For outer4D0,698730 captures end4D8 before its first returning CRT validation,
then captures begin4D4 before the second check. If they differ it reloads the
current end and computes the retained suffix count with SAR4/SHL4. When a CRT
repair or another supported callback changed the range, it copies each captured
source row into its captured destination via697BD0. The original end and
destination delta remain captured throughout this loop.

It then reloads4D8, frees each discarded row's current+4 allocation through the
existing CRT allocation domain, zeroes row+4/+8/+C, and advances by10h against
that captured end. The leading row word is preserved. Only after this finishes
does it store the resulting end4D8. The enclosing allocation, begin and capacity
are retained. The repaired native6987ED `ADD ESP,4` makes the nonnull free path
continue into these stores; it is not an early return.

Finally it clears flat500,510,4E0,4F0 in that order. Each stage captures end,
validates current begin, captures begin, validates current end again, then calls
6977F0 with ECX the same actual10h header and five stack DWORDs: actual8h output,
first owner, captured first, last owner, captured last. Both owners are the
header pointer. The returned iterator is unused. No buffer or opaque leading
word is replaced by a shadow container.

## Explicit library boundary

`NativeInputConfigurationStorageCalls` has no default implementation:

| Native operation | Required contract | Source readiness |
| --- | --- | --- |
| 697BD0 | ECX destination10h; source10h stack; EAX destination; RET4. Checked DWORD-vector deep assignment, self-alias no-op, exact retained-capacity/copy/erase/allocation behavior | Body inspected; private compatible storage contracts exist in native_input_class_configuration.cpp, but no reusable public raw adapter is exposed. Required provider remains explicit |
| 6977F0 | ECX header; five stack DWORDs described above; EAX output; RET14h. Validate nonnull equal iterator owners, move current suffix to captured first, update end and write iterator | Body/cleanup inspected. Required stateless actual-storage provider; no STL implementation imported or ported |

The existing native_input_class_configuration source is a comparison and reuse
candidate, not a license to call private functions or substitute typed vectors.
Production composition must supply these contracts, including nonempty suffix
assignment and returning diagnostics. The fixture implementations below are
deliberately restricted fixture services and are not production replacements.

## Evidence and validation

All23 direct CALL sites and the two indirect sites have report rows. The
indirect Interlocked callA938B1 and no-flags listener callA938C1 were checked
against their containing body, import/name bytes and caller argument schedule.
Incoming698A2F and699BD7 pass the same actual configuration without stack
arguments.699BD7 is guarded by changed modifier bytes and existing byte520;
it immediately precedes another698A10 call. The other raw query caller6AA68F
checks action128h after the settings-loaded gates. No extra query semantics
were inferred from only the cleanup caller.

All four implemented entries and reachable tails are defined. Unexecuted
alignment gaps69873D..3F (`8D4900`) and6987B9..BF (`8DA42400000000`) are jumped
over and intentionally unchanged. The primary repaired6987ED..EF before this
packet; the worker made no Ghidra writes.

Win32 Release, eight seed matches and both existing CTests pass. One ignored
manifested original-native/actual-storage differential verifies four complete
live/disk spans and relocates23 direct CALL operands plus one IAT operand,
without changing branch instructions. Two cleanup passes exercise597 native
lazy accessor calls, two real listener zero-reference releases with publication
replacement, one real returning CRT repair, two bounded checked-vector
assignments, eight erases and all three nonempty nested buffer cleanups.
Source/native snapshots match; capacities, enabled/value fields, configuration
flags and opaque header words are preserved. The action arrays and listeners
come from the recovered actual producers. The fast publication path is tested;
lazy manager creation/registration is not exercised by this fixture.

Runner: `local/run_native_input_cleanup_probe_ag.ps1 -PrimaryWorktree <tree>`.
It links only the selected tree's headers and archives. No SDK, window, input
polling, force, cursor or game run occurred. No permanent tests were added.
