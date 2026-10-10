# DamageableClass Armour source fragment

This packet supplies an ordinary MSVC Win32 C++ fragment for Armour in
`0087CA80`, covering `0087CCA8..0087CCE1`. The exact successor, ExplosionType
at `0087CCE2`, is excluded. CCA8 is an interior setup instruction, not an
original callable entrypoint. The complete reader remains source-absent; this
adds zero whole original functions. Worker compilation is separate from
integrator admission, native ABI compatibility, and receiver/runtime validation.

## Evidence and ownership

The exact boundary and selected lease starts were established read-only before
any source edit or lease. HP was then published on main `2f36476f9`; this
worktree synced to it and claimed CCA8/CCB4/CCC9/CCCE/CCDD plus the four Armour
files. No parent or successor address, shared CMake, ledger, or Ghidra mutation
belongs to this packet.

The retained complete parent audit is
`reports/native_damageable_class_lua_orch4.json`, SHA-256
`ff0e9d14717ddfd33456b3dced444d221a6e4164f79618a23fce07865383e31e`.
Its full 37-state FH3 table, field schedule, key and child contracts are reused.
No native child, data, handler or unwind-action body was reopened. Fresh parent
queries used the existing client verification of `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`; analysis was read-only.

The installed PE remains 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The complete parent remains 3,238 bytes and 858 instructions, ending at
`0087D726`, body SHA-256
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Every instruction start matches the saved and fresh live listing. Armour is
58 bytes and 15 instructions, SHA-256
`dfbb74d5240d374f7999fdff662369845b91008b55e5342fe285ad78bd87d3d4`.
The report retains every selected instruction and all evidence pins.

## Actual storage and lifetime

The original parent receives the descriptor in ECX, a Lua row pointer on the
stack, returns with RET4, and allocates E4h local bytes. Let S be ESP after
those locals and the four saved registers. Armour's actual descriptor is
already held in ESI from HP; EBP still holds the actual Lua row. Unique at
S+94h remains a live tracked LuaObject under state 1. HP at S+44h is dead;
those aligned 14h bytes become fresh Armour storage even if their contents
are stale.

The source borrows these actual, disjoint and stable storage regions, including
the stable scratch pointer. It never copies a descriptor, row, owner or index.
The descriptor supplies writable raw bytes through +4Fh with at least four-byte
alignment. No C++ float subobject lifetime is assumed in that receiver: the
final x87 FSTP instruction writes directly to its raw +4Ch bytes. The extra
local result is an ordinary live C++ float.

Existing protected-lookup contracts still apply: actual stable Lua owner/table
storage across callbacks, a valid current table index, sufficient tracked
slots/references, and a valid inherited error-handler stack position. The
existing Lua5.1.1 `luaD_pcall` wrapper executes the real named lookup without
introducing another Lua C frame. On a reported Lua error it restores the entry
stack top, consumes the error value and throws `NativeLuaOperationError`.
This boundary must not run inside a Lua C callback whose caller expects
`lua_pcall` to receive the original Lua error.

## Native schedule and source mapping

| Native instructions | Retained behavior | Source |
| --- | --- | --- |
| CCA8..CCB4 | Armour key; fresh S+44h; actual row; B67800 | Existing protected lookup with actual objects |
| CCB9..CCBC | FLDZ; FSTP float fallback on stack | Explicit FLDZ/FSTP positive-zero staging |
| CCC1 | Enter state 7 | Guard state 7 after zero staging |
| CCC9 | B66330 exact-number-or-default | Existing genuine numeric provider |
| CCCE | FSTP actual descriptor ESI+4Ch | Returned float spills locally; explicit FLD/FSTP to raw +4Ch |
| CCD5..CCDD | State 1; B67700 on S+44h | Lower state before actual field destruction |

No fallback constant-data read or new conversion policy is needed. The
provider accepts only kind 2 and exact `LUA_TNUMBER` (3); numeric strings and
other kinds use the staged positive zero. For a number, actual Lua state/index
go to the linked `lua_tonumber` API and the existing
`lua_number_float32_00b66270` x87 narrowing helper.

Retained state 7 has parent 1 and action `00C9694A`, which cleans S+44h through
B67700. State 1 ultimately cleans Unique at S+94h via `00C9690B`. This fragment
guards only its own live field. Lookup and zero staging happen before state 7;
publication happens before state lowering; normal destruction happens after
state 1. Prior descriptor writes and Unique remain owned by the enclosing
invocation during propagation. A destructor failure after state lowering is
not retried by this guard. A secondary failure during guard unwinding follows
ordinary `noexcept` termination, without native double-exception identity.

## Compilation and acceptance limits

Fresh candidate and actual Lua/numeric provider translation units compile
with MSVC Win32 `/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17` and
the pinned Lua5.1.1 headers. Indexed COFF inspection uses physical section
numbers and raw symbol indexes including auxiliary records. All candidate
code bytes were decoded: 325 bytes in five code sections, 14 physical sections.

Main section 6 contains 155 bytes and 46 instructions. Offsets are FLDZ +4Dh,
fallback FSTP +4Fh, state 7 +5Eh, numeric call +65h, result FSTP local +6Ah,
FLD local +73h, raw receiver FSTP +76h, state 1 +7Ch, and actual Lua destructor
call +83h. MOVSS only copies fallback bits. The compiler barrier emits no
helper and preserves publication before state lowering.

Guard section 4 is 80 bytes and lowers state before its actual field cleanup;
section 5 is its 29-byte handler. Section 7 holds the 42-byte main unwind
funclet/handler, targeting that guard. Section 8 is the 19-byte release helper.
All data and relocation records are retained: main FH3 info is section 11+10h
with two unwind records (guard and termination), guard info is section 12+0
with ordinary `noexcept` flags, and safe-handler indexes are 30/34. These are
the new compiler's structures; they do not replace the parent's 37-state FH3.

Three exact decorated project externals resolve to freshly compiled real Lua
provider definitions: protected lookup section 49, destructor section 38 and
numeric fallback section 66 (83 bytes). The genuine numeric helper resolves
to `lua_numeric.cpp` section 7 (17 bytes), containing FLD double, FSTP float32,
then FLD float32 return. All 4,240 Lua-provider code bytes/relocations and all
211 numeric-provider code bytes/relocations match the retained HP audit.
Extra C++ double/float spills and reloads remain explicit differences; no
instruction-for-instruction native rounding, status, trap, fault-timing or
NaN-payload identity is asserted.

The existing registered baseline built successfully with `./scripts/build.ps1`:
the two available worker checks passed. Armour remains unregistered, so this
does not execute or link the new fragment in that baseline. No new tests or
fixture executions were added. Root's accepted HP/Source589 review separately
records 589 selected inputs, 68 whole Core plus 3 App objects, 129 unique
positive Core definitions and three checks; those are prior published context,
not new Armour credit. The report pins that primary review.

Integrator registration and a subsequent normal registered build remain due.
The enclosing reader, native stack/register/RET ABI, native FH3, SEH, Lua
longjmp/error-object identity, FP status/traps/fault timing, asynchronous
observation, receiver binding, startup and gameplay equivalence remain held.
