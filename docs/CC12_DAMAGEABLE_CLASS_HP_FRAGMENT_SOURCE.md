# DamageableClass HP source fragment

This packet supplies an ordinary MSVC Win32 C++ fragment for the HP field in
`0087CA80`, covering `0087CC66..0087CCA7`. It ends before Armour at `0087CCA8`.
`0087CC66` is an interior setup instruction, not an original callable entrypoint.
The enclosing 3,238-byte reader remains source-absent; this adds zero whole
original functions. Worker compilation and object inspection do not constitute
integrator admission, a native ABI bridge, application wiring, or game validation.

## Retained evidence and fresh gate

The retained parent audit is
`reports/native_damageable_class_lua_orch4.json`, SHA-256
`ff0e9d14717ddfd33456b3dced444d221a6e4164f79618a23fce07865383e31e`.
Its complete 37-state FH3 model and HP constant/field schedule are reused.
No native child function, constant-data range, handler, or unwind action was
reopened. Only the parent body/prototype was queried live, after the existing
client verified project `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Ghidra was read-only.

The installed PE remains 12,223,752 bytes with SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete parent bytes decode to 858 instructions, ending exactly at
`0087D726`, with SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
All 858 instruction starts match both the saved export and the fresh live
listing. The selected fragment is 66 bytes and 16 instructions. The machine
report records its bytes, every instruction, and artifact hashes.

## Actual storage and ownership

The original parent takes the actual descriptor in ECX and an actual Lua row
pointer on the stack, returns with `RET 4`, and establishes E4h local bytes.
Let S be ESP after those locals and the four saved registers. At this fragment:

* The actual descriptor is retained at S+40h; its HP bytes are +48h..+4Bh.
* The row remains the actual current Lua table/owner/index, held by EBP.
* Unique at S+94h remains a live tracked LuaObject under parent state 1.
* The prior Comment object at S+44h is dead. Those 14h aligned bytes are fresh
  storage for HP even if their contents are stale.

The source receives these actual, disjoint, stable storage regions. It does not
copy a row, table, index, owner, or descriptor. The descriptor supplies writable
raw bytes through +4Bh with at least four-byte alignment. No C++ float subobject
lifetime is assumed in that raw receiver: final publication uses an explicit
x87 memory instruction. A local float has ordinary C++ lifetime. The enclosing
owner retains Unique and all previously published descriptor fields; this
fragment neither destroys Unique nor replays earlier fields on propagation.

Existing protected-lookup contracts remain in force: actual Lua owner and table
storage, stable scratch address and scratch pointer, valid current table index,
sufficient tracking slots/references, and the inherited error-handler stack
position. A callback must not violate those contracts. The protected provider
uses the existing Lua 5.1.1 `luaD_pcall` boundary and the original lookup source,
without introducing another Lua C frame. On its reported Lua failure it restores
the entry stack top and throws the existing `NativeLuaOperationError`.
It consumes that error value. This source boundary must not be placed inside a
Lua C callback whose caller expects `lua_pcall` to receive the original error.

## Native schedule and source mapping

| Native instruction | Retained behavior | Source |
| --- | --- | --- |
| CC66..CC72 | Key HP, fresh S+44h, actual row, call B67800 | Existing protected named lookup on actual objects |
| CC77..CC80 | FLD CE3D08, FSTP float stack fallback | Explicit x87 stage of retained 100.0f, bits 42C80000h |
| CC83 | Enter parent state 6 | Guard state becomes 6 after default staging |
| CC8B | B66330 exact-number-or-default | Existing `native_lua_number_or_00b66330` |
| CC90..CC98 | Recover actual descriptor; FSTP at +48h | Provider float return spills locally, then explicit FLD/FSTP to raw +48h |
| CC9B..CCA3 | Lower state to 1; destroy actual HP object | State 1 before the existing tracked Lua destructor |

The retained FH3 state 6 has parent 1 and action `00C9693F`, which destroys
S+44h through B67700. State 1 ultimately cleans Unique at S+94h through
`00C9690B`. The fragment guard only models its own state 6 cleanup; the enclosing
caller remains responsible for the still-live Unique during later propagation.
Lookup/default staging precedes state 6 exactly as in the parent. A normal
destructor call follows state lowering, preventing a second cleanup attempt by
this guard if that call reports a C++ exception. During guard unwinding,
secondary failure follows ordinary `noexcept` termination, without native
double-exception identity credit.

The numeric provider accepts only kind 2 with exact `LUA_TNUMBER` (3). Numeric
strings and all other kinds retain the observed HP default. The existing
provider calls the real `lua_tonumber` API and existing
`lua_number_float32_00b66270`; that helper explicitly loads double through x87,
stores float32, and returns the narrowed float. This packet adds no conversion
policy, semantic stand-in descriptor, or callback replacement.

## Compiled evidence and limits

The candidate and actual Lua/numeric provider translation units were freshly
compiled using MSVC Win32 `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17`
and the repository's pinned Lua 5.1.1 headers. Indexed COFF inspection resolves
physical section numbers and symbol indexes, including auxiliary records, rather
than pairing duplicate `.text$mn` section names. Every candidate code byte is
decoded: five code sections, 329 bytes total, within 15 physical sections.

Main section 7 is 159 bytes. Relevant offsets are default FLD +4Dh, fallback
FSTP +53h, state 6 +62h, numeric-provider call +69h, returned float FSTP to a
local +6Eh, FLD local +77h, FSTP raw descriptor+48h +7Ah, state 1 +80h, and
actual HP destructor call +87h. The compiler barrier preserves publication
before state lowering and emits no helper dependency. MOVSS on the default
argument only copies its float bits; it performs no numeric conversion.

Guard section 5 is 80 bytes; its state-6 branch lowers state before the actual
Lua destructor. Section 6 is its 29-byte FH3 handler. Section 8 contains the
42-byte main unwind funclet/handler and targets that guard. Section 9 is the
19-byte out-of-line release helper. The report includes all candidate sections,
relocations, raw bytes, safe-handler indexes, and EH data: main info at physical
section 12+10h, two cleanup records (guard and termination), and guard info at
section 13+0 with ordinary `noexcept` flags. These are the new compiler's source
unwind structures, not the parent's native 37-state table.

All three project externals resolve by exact decorated name to fresh actual
`native_lua_objects.cpp` definitions: protected lookup (section 49), tracked
Lua destructor (section 38), and numeric fallback (section 66). The latter is
83 bytes and retains exact kind/type checks, real Lua API calls, and an exact
external match to `lua_numeric.cpp` section 7. That genuine 17-byte helper
contains x87 FLD double, FSTP float32, and FLD float32 return instructions.
The source provider and fragment have extra ordinary C++ double/float spills
and reloads; this packet does not claim instruction-for-instruction native
rounding, status, fault timing, trap, or NaN-payload identity.

The standard build is run on the existing registered sources while HP remains
unregistered. Its outcome is recorded separately in the report. No new test is
added and no HP fixture is executed. Integrator registration and a subsequent
normal build remain separate acceptance steps. The supplied Source581 result
and FMOD-before-window runtime failure are historical context, including the
same frozen Source539 control failure; neither supplies startup credit here.

Native register/stack/RET ABI, FH3 fault identity, SEH, Lua longjmp, active
unmasked FP traps, asynchronous observation, error-object preservation,
application receiver wiring, startup, and gameplay equivalence remain held.
No shared CMake, ledger, or Ghidra metadata was changed by this worker.
