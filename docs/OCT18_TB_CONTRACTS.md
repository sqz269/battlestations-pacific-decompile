# October 18 tB: the twelve remaining contracts

This follow-up closes the twelve rows left open in `OCT18_TAIL_B.md`.
All twelve now have complete source orchestration in `src/leftovers_cc13.cpp`,
namespace `bsp::oct18_tail_b_contracts`; declarations and binding requirements
are in `include/bsp/oct18_tail_b_contracts.hpp`. No row is skipped or excluded.
Names remain descriptive hypotheses. The score-record interpretation retains
the earlier mission-progress caller evidence and is provisional.

These are Win32 source interfaces over actual storage. Required application,
virtual, CRT/STL and Lua bindings have no fabricated fallback. The Lua bodies
reuse the existing actual 14h Lua objects and 4C8h state owner. They do not
substitute a host table, registry-reference object or fake interpreter. Valid
native objects/ranges, stable tracked-object addresses and the named canonical
callee contracts are preconditions. The supplied string storage must preserve
the sized native allocation/release domain. Terminal reference releases and
member destructors must not throw in the C++ cleanup domain.

| Address | Original entry/return contract | Established body |
|---|---|---|
| 00784C40 | ECX network, stack float threshold, RET4 | Unsigned count>1; skip first peer; compare the real unspilled elapsed ST0 with threshold; ordered greater kicks via784B10(network,peer,0). List heads are reloaded. |
| 007850E0 | ECX client, stack float threshold, RET4 | Signed state>=7. Default CF4888 or first peer elapsed spilled to binary32. Require game and ordered elapsed>CE3814; warn, compare threshold, reload game and call transport+207Ch slot3Ch. |
| 00787850 | ECX statistics, stack data/length/unused DWORD/direction, RET10 | Clock/ring update, two WORD bit reads(16,3), signed consumed-bit termination, message factory, packet sequence bounds, per-tag counts/bits/min/max and final+8 overhead. Direction uses the fourth argument's low byte. |
| 0081B630 | ECX section record, RET | Owner from endpoint+4 minus310h, pose refresh, affine point transform, consumed effect reference, release returned temporary before effect byte9, owner slot1ACh(float record+2Ch), record slot10h, Explosion warning(owner,string,0). |
| 0081B7C0 | ECX unit, stack unsigned camo index, RET4 | Native checked camo/entry vectors; two resource slot64h references per entry, model texture call, reverse releases. Optional18h color-ref allocation and child kind4 gun-color calls use live camo+1Ch RGBA. |
| 00836510 | ECX director, stack visitor, RET4 | _gameUnit begin/bare-RET base/end; bool references +220h..223h; aliased target/force temporaries and StoreFireTarget. |
| 00892660 | ECX lua_State*, EAX result count, RET | Borrowed owner/frame; optional camera coordinates/three extracted angles; x87 degree conversion and discarded sprintf buffer; no Lua position push. |
| 008CBCB0 | ECX lua_State*, EAX result count, RET | Convert arg0 object handle and arg1 boolean, release each Lua temporary; insert or checked find/erase GUI+54h set+78h; result/frame/owner cleanup. |
| 008D08B0 | ECX lua_State*, EAX result count, RET | Optional integer player prefix, string or iterated table values, native checked string vector, optional handles, local warning or eligible nonlocal E7 message, complete temporary cleanup. |
| 008D2F50 | ECX settings owner, RET | Owned Lua state/script overrides, latency/bandwidth/ping tables, nested lobby option maps, command-line overrides, drop/capture settings, loaded flag and reverse Lua cleanup. |
| 008E2EE0 | ECX integer mode, stack float x/y, RET8 | Eight-second x87 cooldown; native message36 payload and client/server dispatch; pooled chat temporaries, truncated coordinates/100, mode suffix and channel9/10. ECX is the mode, not a screen pointer. |
| 0091CE90 | ECX fresh284h record, EAX same, RET |36 heterogeneous tree heads, three checked vector headers, explicit scalar/byte defaults, original member ordering and reverse completed-member cleanup. |

## x87 evidence and precision

The main thread uses 24-bit x87 precision control. The source deliberately does
not force that setting: it inherits the caller's PC/RC. The focused probe also
checks PC53/PC64 and all four rounding modes. This is an arithmetic contract,
not a claim to recreate the whole process FP environment or every unmasked
exception/fault timing.

`00784C40` has `FLD threshold; FXCH; FCOMIP; FSTP`, whereas `007850E0`
first executes `FSTP m32`. The explicit10-byte `X87Value` transports the real
ST0 without a C++ double spill; MSVC's `long double` is unsuitable. The supplied
capture adapter calls the original ECX/RET4 `00A3BF50` and executes `FSTP m80`.
Its channel input is the actual peer+D74h field, as passed by these callers.
The canonical callee is the elapsed channel-time-by-IPv4 query, not a replacement
timer. Ordered-greater excludes unordered values. Capturing/reloading m80 retains
the finite extended value; no claim is made about pseudo-denormal encodings or
identical exception timing for invalid/unmasked x87 states.

Disk bytes `00787880..007878E4` establish:

1. FILD64 counter/frequency, FDIVP, FSTP32, reload that binary32.
2. FILD32 origin(+762968h); a negative signed word adds exact float2^32.
3. FSUBP, FDIV32 quantum(+762964h).
4. Save CW, OR0C00h to select truncation, FISTP64, use its LOW DWORD.
5. Unsigned DIV by upper-lower; restore CW after DIV; publish remainder.

The decompiler's `ROUND` is wrong. No C++ cast of an unsigned origin or double
division replaces this schedule. Valid storage and nonzero ring width are
required, just as for the unchecked native DIV. The later window updates are
literal+1000. The stream's header is18h with cursor at+4 and owned byte+14h.
Factory allocations are not released by this function in the listing; the
source retains that behavior. It does not invent message deletion or a
non-progress guard. Min/max message sizes use MOVSX16 followed by UNSIGNED
comparison, then a low16 store. The cursor's native wrapping+7 accounting is
preserved. Raw cursor reads reuse00428CB0.

The camera binding also contains a misleading DC-form display. Disk bytes
`008927CC: DC CA` are **FMUL ST(2),ST(0)**. The source keeps the full schedule:
angle1/pi, multiply it by180 in ST2, convert/pop it, then angle2/pi*180 and
convert/pop. The exact binary64 pi cell is `0x400921FB60000000`,180 is exact.
`00BF7420` is a required original-ABI conversion entry: its SSE2 branch spills
to binary64 then CVTTSD2SI; its legacy branch returns EDX:EAX. Only EAX is used
here. The camera formatter receives horizontal=second conversion and
vertical=first; position Y,X,Z conversions occur in native instruction order.
Matrix0042D2E0 receives three output addresses (EDX plus two stack arguments),
although only two angles are subsequently formatted.

## Ownership and caller contracts

The effect creator8685E0 consumes the retained definition on every path. Its
returned reference temporary is released before testing the retained raw effect
pointer and setting byte9. The manager owns any surviving effect. The warning
call's additional owner and float-zero arguments are established by assembly.

Camo's texture call is ECX model, EDX first resource, stacked second resource;
color is ECX model, EDX native key, stacked live RGBA. The allocated color
reference is initialized/refcounted/released but is not the color argument.
`"gun"` comes from00CEB6BC. These details must not be inferred from the
provisional GetDiffuseColor name. Second texture ref releases before first.

For the director, the local force word starts with the saved director pointer,
and the target starts pointing at the value pair's own stack storage. The same
pair storage is then reused for the bool field. The Source representation keeps
that alias and the untouched upper force bytes. `00BD63B0` type4 writes only for
integer/table values; absent/nil fields do NOT zero the target. Type3 writes a
single byte. Thus inventing zero-initialized locals changes the contract.
StoreFireTarget836240 reads only the low force byte, as its listing confirms;
the binding exposes the full pushed word so its transport is reviewable.
Physical native stack addresses and the old stack ABI are not reproduced.

All three Lua callbacks retain their diagnostic string allocation/release and
their separate once-flag/`luakod`/zero-log initialization. Frame result count is
queried before vector/frame/owner destruction. Borrowed state close does not
close the underlying interpreter. The raw Lua primitives retain Lua's native
nonlocal error route; this change does not add protected-call translation or
claim native FH3/nonlocal-unwind compatibility.

DisplayMessage preserves the initial unused B66A60 integer probe and later repeated
argument lookups. Optional handle tests use absolute argument counts>=3 and>=4,
even without a player prefix. Local warnings receive the entire checked vector;
nonlocal delivery tests peer bytes8/9/A and peer+50h and constructs an E7
message from the FIRST string only. Its20h lifetime ends through7656E0 even
when a C++ send binding throws. The vector destroys its strings forwards.

The lobby loader uses the literal spelling `BandwithLevels`. It first writes
bandwidth at+8Ch+4*key, then clears the20 DWORD defaults+40h..8Ch. Lobby outer
keys are slots, next keys are options, and the third iteration visits choice
key/value pairs. Presence of `MenuDIS` disables an option regardless of its
boolean value. Presence of `Default` stores the OPTION KEY as the default for
that slot. The maps are:

| Owner offset | Canonical subscript pair | Stored value |
|---|---|---|
|10h|575E80/5732C0|Allocated string choice key, or null for numeric choice|
|34h|575E80/5732C0|Allocated string choice value|
|1Ch|575FD0/573410|Numeric choice key, or zero for string choice|
|28h|5C6850/5C49C0|Enabled byte|

The raw strings use BF55BE and are transferred into the existing maps. Prior
entries are overwritten without invented frees. Failure between allocation
and publication retains the original lack of a caller cleanup for that raw
allocation. Command-line records stride434h; numeric names override slot1,
enable it, clear the10h map, replace34h text, and assign a consecutive ordinal
in1Ch. Drop/capture defaults are native float0.1 and20 with explicit live cells.

Ping's cooldown FCOMIP/JBE accepts unordered values, as the original does.
It stores clock+binary64(8) to binary32 before reading the game global. Its
28h message has type24h, refcount1, player+18h, mode+1Ch and float bits+20h/+24h.
The stack message is borrowed by dispatch; no invented normal-exit release is
added. Chat string construction and reverse temporary releases follow the
native expression order, including the two empty strings passed to772170.

The score constructor leaves tree/vector proxy words and unassigned payload
bytes intact. Nil bytes vary(15h/19h/1Dh/21h/3Dh); allocation helpers own their
node layouts. Its FunctionInfo is00DD9414, unwind map00DD9438:38 predecessor-
linked actions00CA60C0..00CA62AB. The source records each completed member,
including vectors12Ch/13Ch/14Ch, and calls those exact destructor entries in
reverse on C++ allocation failure. The last head+244h has no later throwing
native instruction and hence no cleanup entry in that map. No blanket memset,
blanket free or substitute STL containers are used.

## Verification and limits

`python tools/ghidra_export.py verify-seeds` confirms the saved seed bytes
against disk before the final instruction-probe run.
`verify_report_calls.py` checks406 direct call sites from the twelve original
listings against live caller bodies and exact callee addresses: zero failures.
`./scripts/build.ps1`: strict MSVC Win32 Release, `bsp_game` compilation and both
registered CTests passed(2/2). `python tools/oct18_tb_contract_probe.py` passes:
360 packet timing comparisons and60 camera comparisons against relocated
original instruction slices, across PC24/53/64 and all four RC settings;
unspilled80-versus32 and unordered checks; final-allocation failure checks all38
original FH3 cleanup actions in reverse; success checks36 allocations, identity,
proxy preimages, final sentinel and retained byte padding. The probe extracts
the tested C++ bodies verbatim, uses a qualified SSE2 conversion fixture for the
camera, and embeds its Win32 manifest. It does not load or run the game.

Seed SHA256:
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
Packet timing slice SHA256:
`b291e2017d35b58006620ae113d6e0e58ec09a2fb7cc5e5556a9e777f1b3471d`.
Camera slice SHA256:
`740579da563fb24ccef270ef60e18a134b84e85c76061e168b5b133f22d7bc95`.
Ignored local build/probe/annotation receipts remain in this worktree's local/.

No new library/generated exclusion is made. CRT conversion, native STL
allocators/subscripts/erase/destructors and linked Lua remain dependencies,
without game-reconstruction credit for their vendor bodies. Application hooks
are required bindings to their actual named entries. No game host is wired by
this packet. Build and bounded instruction/cleanup proof are distinct from
whole-function original ABI, allocator identity, native exception transport,
Lua callback integration and gameplay proof; those latter claims remain open.
