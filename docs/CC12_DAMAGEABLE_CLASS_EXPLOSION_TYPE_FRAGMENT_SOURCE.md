# DamageableClass ExplosionType source fragment

This packet supplies ordinary MSVC Win32 C++ for ExplosionType in `0087CA80`,
covering `0087CCE2..0087CD4C`. Successor `0087CD4D` is excluded. CCE2 is an
interior setup instruction, not an original entrypoint. The enclosing reader
remains source-absent; this adds zero whole original functions. Registration,
native ABI compatibility and application execution are separate acceptance
steps, not consequences of worker compilation.

## Retained evidence and ownership

The exact boundary and selected instruction starts were established read-only
before a lease or source edit. After Root published group descriptor/Armour on
main `ea35ae1c0`, this worktree synced and claimed CCE2/CCEE/CCFD/CD10/CD25/
CD34/CD3D/CD48 plus the four ExplosionType files. No parent/successor claim or
shared CMake, ledger or Ghidra mutation was made.

The complete retained audit is `reports/native_damageable_class_lua_orch4.json`,
SHA-256 `ff0e9d14717ddfd33456b3dced444d221a6e4164f79618a23fce07865383e31e`.
Its complete 37-state FH3 table, fields, keys and child contracts are reused.
Fresh queries opened only the parent prototype/body through the existing
client verification of `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. No native child, data, handler or unwind action
was reopened. Existing reconstructed provider source and its fresh objects
were inspected; this is not a new native-child audit.

The installed PE is unchanged: 12,223,752 bytes, SHA-256
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
All 3,238 parent bytes decode to 858 instructions ending at `0087D726`, hash
`00d679920bd1b8f8653e9a0ab1de627f954cfadaf48b8bf0642676fde5162808`.
Every instruction start matches both saved and fresh live listings. The
selected fragment has 107 bytes, 26 instructions and SHA-256
`a4cab3316fdda32e9e2dee1d70dfefd6f3e36eb9ae05678734f65d7798d23310`.
The report retains every selected instruction and complete evidence pins.

## Storage, lookup and cleanup contracts

The original parent takes the descriptor in ECX and the Lua row on the stack,
returns with RET4, and allocates E4h local bytes. Let S be ESP after those
locals and four saved registers. At entry, ESI already holds the actual
descriptor and EBP the actual Lua row. Unique at S+94h is still live under
state 1. Armour at S+44h is dead; its aligned 14h storage is fresh for the
first probe despite stale bytes. The same address is reused only after that
probe is destroyed.

The source borrows actual disjoint, stable descriptor/row/owner/Unique/scratch
storage and a stable scratch pointer. It never substitutes copied tables,
indexes or descriptors. Writable raw receiver bytes through +43h, aligned at
least four, are sufficient for this field. `memcpy` publishes the int32 bits
without asserting an established C++ int32 subobject. No owned NativeString,
enum mapping or fallback value is introduced.

The existing protected lookup keeps its actual owner/index/tracking-capacity
and inherited error-handler-position contracts across callbacks. Its real
Lua5.1.1 `luaD_pcall` wrapper does not add a Lua C frame. A reported Lua failure
restores entry stack height, consumes the error value and throws the existing
`NativeLuaOperationError`. Do not use this boundary inside a Lua C callback
whose caller expects `lua_pcall` to receive the original error.

| Native instructions | Retained behavior | Source |
| --- | --- | --- |
| CCE2..CCEE | Lookup ExplosionType into S+44h | Actual protected named lookup |
| CCF5..CCFD | State 8, B66A60 integer-number predicate | Existing exact predicate on actual probe |
| CD06..CD10 | Save AL, state 1, destroy probe | Save bool and lower before actual cleanup |
| CD15..CD17 | False skips to CD4D | Return without writing descriptor+40h |
| CD19..CD25 | Lookup the same key again | Fresh second lookup after probe destruction |
| CD2C..CD34 | State 9, B66290 integer accessor | Existing accessor on the new value, live mode alias |
| CD3D..CD48 | Store EAX at +40h; state 1; destroy field | Raw int32 publication, barrier, lower, actual cleanup |

A successful first predicate does not cache a value or validate the second
lookup. Metamethods and tracked-index changes can make the second result
different. It is converted directly without another predicate. A false probe
leaves all existing +40h bytes unchanged. Prior descriptor writes are retained
if later cleanup reports an exception.

Retained states 8 and 9 both have parent 1 and clean S+44h through B67700,
using actions `00C96955` and `00C96960` respectively. State 1 ultimately cleans
Unique at S+94h via `00C9690B`. This fragment guards only state 8/9 fields;
the enclosing owner retains Unique. Both normal destructors follow state
lowering, so an exception from normal destruction does not retry that object.
Secondary failure during guard unwinding follows ordinary `noexcept`
termination, without native double-exception identity.

## Genuine numeric providers and live mode

All four provider translation units are already registered:
`native_lua_objects.cpp`, `gui_lua_reader.cpp`, `lua_numeric.cpp` and
`native_render_batch_keys.cpp`. B66A60 requires kind 2 and exact Lua NUMBER,
then reads that value through real `lua_tonumber`. The existing Win32 helper
narrows it with x87 FSTP/FLD float32, uses CVTTSS2SI, FILD and an ordered
FUCOMIP comparison, then checks the flags. A nonintegral double rounded to an
integral float can qualify; numeric strings fail this first predicate. The
predicate is independent of the CRT conversion mode.

B66290 reads the newly fetched value through real `lua_tonumber`, without
adding a kind/type gate. The existing numeric helper narrows to float32 and
reloads it before reading its live mode alias. Nonzero takes FSTP double and
CVTTSD2SI; zero calls the existing complete naked BF7456 x87 kernel and takes
its low EAX word. There is no new saturation, zero fallback, C++ out-of-range
cast or invented enum conversion.

The mandatory `const bool&` is the established Source binding documented in
`docs/GUI_LUA_NUMERIC.md`. It must refer to the genuine live Source decision
corresponding to native `0109EEA4`, remain alive, and be disjoint from the other
borrowed storage. It may change during callbacks. No temporary, default or
copied snapshot is permitted, and the wrapper never reads it early. This
contract does not reinterpret the native DWORD as a bool object or provide
an application/native-global adapter; that binding remains held.

## Compiled review and validation

The candidate and four actual providers compile under MSVC Win32 with
`/O2 /Ob2 /Oy- /MD /W4 /WX /fp:strict /EHsc /std:c++17` and pinned Lua5.1.1
headers. Physical COFF section indexes, raw symbol indexes including auxiliary
records, raw bytes and relocations were used. All candidate code was decoded:
379 bytes in five code sections, 14 physical sections.

Main section 6 is 202 bytes/62 instructions. First lookup is +46h, state 8
+4Ch, predicate +53h, state 1 +5Dh, destructor +64h, then the false branch at
+6Eh targets +B9h. True relookup is +7Bh; +80h forwards the mode address,
state 9 is +83h, the actual integer call +8Bh, raw `[ECX+40h]` MOV +93h,
state 1 +99h and final destruction +A0h. No out-of-line memcpy or barrier
helper is emitted. The first probe is dead before the second lookup.

The 87-byte guard in section 4 tests states 8/9 and lowers before cleanup.
Section 5 is its 29-byte handler; section 7 is the 42-byte main unwind
funclet/handler; section 8 is the 19-byte release helper. Main FH3 info is
section 11+18h, maxState 3/flags 1: the encoded records contain a guard action,
a null action and termination. Guard info is section 12+0, maxState 0/flags 5.
Safe-handler indexes are 31/35. These are compiler-generated Source unwind
structures, not the parent's 37-state native table.

All four exact project externals resolve to fresh real Lua definitions:
protected lookup section 49, destructor 38, predicate 55 (72 bytes), and
integer accessor 52 (35 bytes). The transitive predicate helper is GUI section
459 (51 bytes); conversion is numeric section 8 (54 bytes); and the genuine
naked x87 kernel is section 10 (117 bytes). Each selected body was inspected
completely. The conversion's mode CMP at +18h follows float32 narrowing/reload;
its two branches use the verified instructions and actual kernel definition.
Both special-path x87 stores remain. Extra C++ double spills and native-global
binding differences preclude native FP status/trap/fault/NaN-payload identity.

The normal registered baseline build passed the worker's two available checks.
It includes the accepted Armour predecessor and excludes this new fragment.
No new tests or candidate fixtures were added. Root's published Source595
review separately records 595 inputs, 70 whole Core plus 3 App objects, 133
unique positive Core definitions and three checks. Its reported VFS-phase
then FMOD startup failure supplies no ExplosionType execution credit.

Integrator registration and a subsequent normal registered build remain due.
Full parent Source, native register/stack/RET/FH3 ABI, actual DWORD binding,
FP status/traps/fault timing, SEH, Lua longjmp/error-object identity, asynchronous
observation, receiver binding, startup and gameplay equivalence remain held.
