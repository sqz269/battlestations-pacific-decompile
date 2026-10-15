# Ordinary enum declaration from Lua

Whole Original `008F2F70..008F3199` is553 bytes and152 instructions
(SHA256`7da0908e4652cdd91600fa0e9486aebdbd83af44cfd8fa7e6d0bed8b84d73d18`).
It receives a genuine19Ch enum in ECX and one owning8B NativeString by value
on the stack, returns with RET8, and has no established semantic EAX result.
The descriptive name and new context-bearing C++ API are not recovered
symbols, class layout/ABI or a drop-in binary replacement.

`declare_native_enum_symbols_from_lua_008f2f70` takes the actual consumed key
by reference to expose its ownership transfer without an accidental extra
header copy. Its actual pooled buffer is returned once, last, after all Lua
locals. The released header is not zeroed. The enum survives and owns any
new keys through the existing whole declaration/insertion/lifetime services.

The body copies the incoming C string, or the same distinct F89450 clone-empty
authority, byte by byte through NUL into enum+11C. The actual128B inline area
must fit; no truncation or extra clearing is added. It resizes the owning type
header at+114 to5, preservefalse, then reads its CURRENT data+118 and, when
nonnull, CURRENT length+114. It copies exactly that length from actual immutable
D162D8 (`LUA_S` and NUL, six accessible bytes). No extra terminator is copied.
Only after this work does it read the current E188A8 publication and select the
SAME constructed4C8h NativeLuaStateStorage at world+1A0C.

All Lua locals are genuine14h NativeLuaObjectStorage objects whose tracked
addresses remain stable on that same owner. Globals are constructed, the
incoming key selects the table, and globals are destroyed before the table
test. A table constructs key then value, starts raw iteration and stops when
the current key is unbound. Each value's actual `Name` field is retrieved with
raw B67800 and displayed through fixed GameNativeLuaDisplayStringBindings into
a genuine8B NativeString. Null display data selects the same F89450 authority.
The current key is converted and whole8F2E40 declares on the same enum. There
is no added key/value/type/finite/duplicate filter or alternate publication.

After each declaration the display string's CURRENT data and length are
returned through raw0041DD20: fresh00419CC0 getter, then BD1510 mode1. Its Name
Lua object is destroyed only afterward. Raw iterate-next preserves actual
Lua stack/tracking behavior. At loop end value then key are destroyed; the
selected object is destroyed on table and nontable paths; the consumed input
string is last. No private Lua state, registry-ref model, protected-reader
adapter, new default or callback replaces those actual APIs.

Integer conversion calls linked Lua5.1.1 `lua_tonumber` on the actual key's
owner/state/index, THEN samples the full volatile32-bit0109EEA4 word for
nonzero into a genuine C++ bool, THEN calls accepted
`lua_number_integer_00b66290`. No bool-reference alias of the DWORD, CPU guess,
C++ integer cast, saturation or replacement is used. Original spills/reloads
float32 before testing0109EEA4. This Source samples the word before the existing
tail's local float32 spill/reload. Its ordinary contract requires a stable
feature word and no FP traps, reentry or concurrent mutation; literal x87,
memory-read ordering, FP exception/status and private conversion parity are
not claimed. The accepted conversion tail and all its qualifications remain.

Display uses the already admitted fixed CURRENT-CRT formatter, including
Lua-number float32 rounding, explicit double materialization, unbounded
`std::sprintf(char*,"%f",double)`, ignored return and full NUL scan. The result
including NUL must fit its actual accessible52B Source scratch/compared Original
region. That prefix is not a recovered Original declared-array extent.
No numeric/NaN/Inf/zero/locale filter or forced locale is introduced. Private
CRT formatting, numeric text, rounding, errno, locale/PTD, FP and failure
equivalence remain unproved; current service semantics are explicit.

The noncopyable/nonmovable GameNativeEnumLuaDeclarationBindings borrows actual
retained string, physical, settings, read-only data and mutable CRT services.
Its declaration context and fixed display binding use the SAME raw/string
process. It takes already-initialized E17578 on the common E188B4 allocator
list, the distinct E186ED/E17654 fallback roles, and distinct F89450 clone role.
It borrows `settings.game_00e188a8()` by reference rather than caching the world.
Its feature reference comes from `crt.feature_word()`. Required existing mapped
read-only spans supply D162D8/6 and CE8ED0/5 (`Name` NUL). Construction creates no
world, Lua owner, pool or process, and performs no startup or publication.

Native handler CA48C8 selects FuncInfo DD74FC, magic19930522, eight states and
unwind map DD7520. Actions cover input, globals, selected (states2/3), key,
value, Name and display. Source guards arm only completed objects and disarm
before explicit cleanup calls. A failed display output is not armed. There
is no enum/type/name/output rollback, stream/resource transaction or retry
when cleanup throws. C++ cleanup failure/double exception uses the new
interface's behavior. Native FS/FH/SEH, hardware faults, Lua longjmp/metamethod
error transport and exceptional-unwind equivalence are not supplied.

Require genuine current-CRT enum/raw-string/symbol-pool provenance, coherent
finite valid ownership/chains, stable/disjoint accessible memory, fitting names,
closed ASCII/current-C-locale declaration keys, live actual Lua stack/owner and
tracking capacity, successful nonthrowing ordinary services, and synchronized
shared mutation. All process cells, mapped spans, owners, pool generations and
keys remain live through use and cleanup; retire enum keys/nodes before pool
teardown. These are caller/lifecycle contracts, not startup or owner proofs.
Original ABI/class/privateCRT, whole parser/loader, application wiring and game
or runtime equivalence remain outside this packet.

Compiler, complete object and required normal-build evidence is recorded in
`reports/native_enum_lua_declaration_cc12.json` and the sealed local family.
Both new TUs remain unregistered; their strict compile is separate from the
normal build's existing checks. Root owns independent review, registration and
Main admission. No new tests, probes, runtime replay or game launch are added.
