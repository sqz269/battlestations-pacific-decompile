# Chance / Threshold ordinary Source fragment

This packet implements only the two adjacent, independently closed field phases
inside `0087CA80`: Chance `[0087D0FB,0087D141)` (70 bytes, 16 instructions) and
Threshold `[0087D141,0087D181)` (64 bytes, 15 instructions). It uses the retained
134-byte capture; no new Native byte, data-cell, callee, handler or Ghidra scope
was opened. The former readiness receipt remains immutable historical evidence.

The ordinary MSVC Win32 Source function is
`read_native_damageable_class_chance_threshold_fragment_0087d0fb`. Its actual
Release emission passes the mandatory x87/state/guard gate. This is a new C++
interface, not an original function entrypoint or Native ABI replacement.
Integration, CMake registration and normal-build admission remain Root work.

## Contract and ownership

The caller supplies the actual selected raw 30h-byte ESI row and the SAME
successful, live Msh owner/scratch and iterator value S2C from this enclosing
invocation. Earlier Index/Fire/fallback phases must already have restored state
17. Msh S18 and its saved captures stay live through this function; its normal
release at D181 is outside this packet. The explicit owner reference records a
caller precondition; this fragment adds no state accessor or fabricated proof.

S6C is the actual fresh aligned 14h-byte Lua-object slot, reused only after its
previous object has died. Each real protected lookup constructs that object;
the fragment never invents another Lua ownership API. Lookup uses the actual S2C
object each time. Compiled EDI retains its address, so index changes caused by
the first field's real tracked cleanup are observed by the second lookup.

The scratch borrows `actual_failure_chance_key_00d0ac88` and
`actual_failure_damage_threshold_key_00d0ac70`. They must name the genuine live
key storage, with valid NUL-terminated spans. Root's accepted Msh correction is
followed: there are no replacement key literals in this object. Retained key
names/identities are metadata, not newly read or verified Native cell bytes.

The row must remain valid, writable and alive across both fields and callbacks.
Earlier returning invalid-parameter callbacks do not establish that condition.
Do not reselect the row, repair its cursor or roll back completed writes.
Row, S6C, outer Lua/string storage, keys, bindings and private Source frames must
be appropriately aligned, disjoint and stable. Exclude callback reentry, alias
into private guard/capture/frame storage, and rebinding; preserve otherwise
valid callback writes to the actual row/header. Actual Lua tracking, capacity,
owner/current-index and error-handler-below-removed-slots contracts still apply.

## Retained behavior and Source sequence

Chance does the actual protected named lookup into S6C at D109. It stages the
binary32 default -100 (`C2C80000`, retained CE65D8 metadata) before arming 23 at
D11A. The genuine B66330 exact-number-or provider runs at D122. D127 divides
ST0 by a binary64 100 (`4059000000000000`, retained D7A220 metadata), D131 lowers
to 17 while the quotient remains in ST0, D139 stores binary32 to raw row+28,
then D13C destroys the actual Lua field.

Threshold does its lookup at D14F, stages binary32 -1 (`BF800000`, retained
D7A260 metadata) before arming 24 at D160, and calls the same provider at D168.
D16D stores to raw row+2C while 24 remains active. D174 then lowers to 17 before
D17C destroys S6C. A Threshold failure preserves the completed Chance phase
and any valid callback changes. Neither phase performs Msh cleanup.

The private guard starts at 17 before the first lookup. On ordinary C++ unwind
it destroys actual S6C only for 23 or 24, first lowering its state to 17. Each
normal path also lowers before actual destruction, so a failing cleanup is not
retried. Retained metadata connects 23/24 to parent 17 and Lua cleanup at
C969E6/C969F1; 17 connects to parent 13 and raw Msh cleanup at C969A4. These are
retained metadata edges, not newly inspected handler bodies. The actual outer
Msh owner remains the caller's responsibility.

The genuine B66330 provider accepts only kind 2 and exact `LUA_TNUMBER` (3),
otherwise returning the staged fallback. Numeric strings therefore use the
fallback. Its number path calls actual `lua_tonumber`, then actual
`lua_number_float32_00b66270`; no fake protected numeric facade is introduced.
After the exact-number gate, the current Lua C numeric read is nonallocating
and has no metamethod conversion. Lookup still uses the real protected Lua
operation and its existing stack/tracking restoration and C++ error contract.

## Actual compiled evidence

The packet froze the actual Integrator 05:55 normal-build Core command tlog and
project, then selected its `native_lua_objects.cpp` and `lua_numeric.cpp` records.
Focused candidate/provider compiles preserve `/O2 /Ob2 /Oy- /MD /W4 /WX`
`/fp:strict /EHsc /std:c++17`, definitions and actual include roots, localizing
workspace paths and adding object/listing/showIncludes output. They compile
successfully with VC 14.51.36231 Hostx86/x86. No normal build, link, test, probe,
fixture or runtime execution was performed for this packet.

The complete candidate COFF has 12 physical sections and four code sections,
461 bytes: private guard 87, guard handler 29, main body 303, and main unwind
funclet/handler 42. The receipt contains the complete raw object, every physical
section, symbol and AUX record, string table, relocation and decoded code byte.

Main body physical section 7 has these verified offsets:

| Operation | Emitted offset |
| --- | --- |
| Actual Chance key pointer from scratch+0C | 35 |
| Protected lookup | 52 |
| Default FLD / FSTP | 5C / 62 |
| Private state 23 / genuine number-or | 71 / 78 |
| Ordinary result binary32 spill / reload | 7D / 89 |
| FDIV qword / private 17 / raw FSTP row+28 | 8C / 92 / 98 |
| Actual Lua destruction | 9C |
| Actual Threshold key pointer from scratch+10 / lookup | A1 / A8 |
| Default FLD / FSTP | B2 / B8 |
| Private state 24 / genuine number-or | C7 / CE |
| Ordinary result binary32 spill / reload | D3 / DF |
| Raw FSTP row+2C while 24 / private 17 | E2 / E5 |
| Actual Lua destruction | EC |

The Chance quotient has no spill, call, SSE operation, reciprocal or other
floating operation between FDIV and its final raw FSTP. Threshold's inverse
store/state order is preserved. MOVSS moves the staged fallback bits for the
ordinary call; it performs no arithmetic. Inline assembly directly writes raw
row memory without asserting a typed C++ float subobject. Default constants
occupy the 16-byte .rdata section as the three retained numeric values; they do
not establish original mutable-cell bindings or fault identity.

Ordinary ABI differences remain explicit: genuine Lua Source spills its double
result before the float helper; that helper is FLD qword, FSTP binary32, FLD
binary32. B66330 has another binary32 spill/reload, and this fragment spills
and reloads the ordinary float result before its selected raw sequence. These
precede the Chance division. No cast-syntax or normal-result observation is
used to claim Native precision, status, NaN, denormal or trap equivalence.

Guard section 5 compares 23/24, lowers to 17 at +31 and invokes actual S6C
destruction at +3B. Main unwind section 8+0 addresses the guard at EBP-1C and
jumps to that destructor. The main UnwindMap has two rows: parent -1 to guard
funclet, and parent -1 to `__std_terminate`. Main FuncInfo has magic 19930522,
maxState 2, no try map and flags 1. Guard FuncInfo has maxState 0 and flags 5.
SafeSEH has physical symbol indices 30 and 34, resolving to the main and guard
handlers. The normal tail inlines the guard under C++ termination protection;
with private state 17 it skips cleanup. Secondary cleanup failure follows
ordinary `noexcept` termination, not claimed Native FH3 equivalence.

The three exact candidate project symbols resolve in the fresh genuine Lua
object: section 49 protected lookup (125 bytes), section 38 destructor (39),
and section 66 exact-number-or (83). Its float-helper relocation resolves to
fresh numeric section 7 (17 bytes). The complete three objects have 75 code
sections and 4,912 bytes, all decoded and indexed. All 66 fresh Lua code
sections and all five fresh numeric code sections match retained genuine code
and indexed relocations. Selected Lua code closure is
6,7,10,18,27,28,38,42,48,49,66,74,77,79, plus numeric section 7. Remaining
sections are whole-object inventory, not new Native reconstruction credit.
Actual Lua-library, CRT and MSVC runtime externals remain ordinary dependencies;
there was no link/runtime proof. Weak-AUX, local COMDAT and external relocation
frontiers are preserved explicitly in the receipt.

Root's earlier Source718 build is frozen context: 718 selected project inputs,
99 Core and five App object pins, 271 unique positive Core definitions, three
existing checks, and 722 retained pins verified. It includes the accepted Msh
key-storage correction. Those checks do not admit this new candidate. Complete
current Source/header snapshots, bounded read/search receipts, actual compiler
and include pins, baseline Git blobs and the full object audit accompany the
new machine-readable report.

## Limits and admission

This packet establishes bounded ordinary Source and focused Release emission
under the explicit caller contract. It does not change the FP environment or
claim all-control-mode equality. Floating status/pending exception/trap/fault
timing, Native register ABI/FH3/SEH/longjmp, actual application bindings, the
excluded Msh middle fields and normal release, whole 741-byte/3238-byte parent,
startup and gameplay remain held. No complete Native function, ledger/name,
Ghidra annotation, shared build metadata or runtime admission is added here.
