# Actual Lua field-value storage

Addresses: 00BD63B0 00CC5AF8

`store_native_lua_field_value_00bd63b0` reconstructs the complete normal
1143-byte body `[00BD63B0,00BD6827)`. The original takes the actual Lua object
in ECX and an actual 8-byte `{tag,destination}` pair in EDX, and ends with RET.
Its descriptive name is a hypothesis, not a recovered symbol. The explicit
C++ context and scratch arguments change the interface; this is not a binary
replacement or an implementation of the native exception handler.

| Routine | Coverage | Evidence |
|---|---|---|
| BD63B0 | complete normal body | Stored Ghidra body ends at RET BD6826; all 1143 live bytes match PE |
| CC5AF8 | boundary only, unported | No Ghidra function; 10 live/PE bytes through CC5B01 inclusive; last instruction is the 5-byte JMP at CC5AFD to BF6B43 |

The two callers establish the pair layout: BD6830 passes the address of its
adjacent third/fourth stack arguments at BD6896, while BD68D0 passes the same
field arguments at BD695D. Neither is an evaluated `GuiLuaVariant` or a
registry reference. This module reuses `NativeLuaStateStorage`, actual
`NativeLuaObjectStorage`, the owner's real five-reference tracking slots,
and the canonical raw string-pool context.

| Tag | Stores and ordering |
|---|---|
| 0 | Capture destination header, convert Lua value to string, scan to NUL (null gives length zero), resize with preserve false, then copy current header length to current data using overlap-safe BF7680 semantics |
| 1 | Capture destination before B66290, preserve that provider's float32 spill and current SSE2/x87 conversion mode, write EAX bits |
| 2 | Capture destination before B66270 and consume its ST0 return with one FSTP |
| 3 | Capture destination, get boolean, write exactly one byte |
| 4 | B66A60 first; if accepted, B66290 then capture destination and call current 109CED4 with ECX integer. Otherwise B661B0, capture destination and call current 109CED8 with ECX actual object. A failed pair of predicates leaves destination untouched |
| 5 | Construct children 3,2,1; read numbers 1,2,3 into separate float spills; capture current destination; ordered x87 copies; destroy children 1,2,3 |
| 6 | Construct 2,1; read 1,2 into native spill locations; capture destination; ordered x87 copies; destroy 1,2 |
| 7 | Capture destination before lookup. Rows and columns 1..4; each number goes directly to destination before child destruction. Destroy row after its fourth child |
| 8 | Construct 4,3,2,1, retaining child2's pointer in its native spill until consumed; read 1,2,3,4; first FLD precedes destination capture; interleave subsequent FLDs, staging FSTPs and previous-component DWORD stores; destroy 1,2,3,4 |
| A | Get string; BF8417 calls `strtol(text,nullptr,10)`; spill signed32 result and copy its DWORD bits with no integer-to-float conversion |
| 9/other | No operation |

Tag A corrects older `ParsedFloat` prose and the logical reader's `strtof`
projection. The evidence is BF8417's body and BD67F8..BD680D: EAX is written
to a stack DWORD, then MOVSS copies those bits to the destination. This
packet does not edit that older logical projection. Tag A does not guard a
null string pointer. The linked CRT parser is a library dependency; its
version-specific faults, locale and errno are not independently established.

BD4FC0 installs callable cells without validation. The two bindings here
reference current function-pointer cells with their original ECX/EAX call
shape. There is no fallback for missing providers. In particular, the table
predicate accepts every nonzero kind except kind2 without consulting Lua;
that behavior comes from the existing actual-object provider. Implementing
the installed game object resolver targets is outside this packet.

`NativeLuaFieldValueScratch` is the 74h-byte native local region, beginning
at ESP+10h after all four saved registers. Its first 10h bytes are spills;
14h-byte objects begin at offsets 10h,24h,38h,4Ch,60h. The source never
initializes the whole region. Callers supply an initialized preimage and a
stable address. Indexed construction and destruction retain the native
object padding and stale metadata. No hidden second owner, zeroed vector,
pre-evaluated table, bounds checks or rollback are introduced. Reusing the
region across matrix iterations preserves its prior byte state. The explicit
x87 loads/stores retain alias and masked exception behavior; they are not
replaced by abstract float assignments.

The function's native handler pointer CC5AF8 leads to a raw thunk which loads
DFFD2C into EAX and jumps to BF6B43. This packet records its exact boundary
but does not reconstruct its unwind metadata/funclets or claim C++/SEH
compatibility. Raw Lua errors can transfer nonlocally. Cleanup on such paths,
unmasked FP faults and the full reader's protected-call policy remain outside
the complete normal-body coverage.

Strict MSVC Win32 `/W4 /WX /fp:strict` and all three existing CTests passed.
One ignored `/MD` probe with an embedded manifest passed 25 original/source
pairs using real Lua states and actual objects. It compared every scratch
byte, output bytes, masked x87 flags, lookup order and resolver arguments;
checked live reference publication from inside `__index` and balanced stack
height/counts afterward; and used the canonical raw string pool. Cases cover
all tags, zero/denormal/NaN values, string/null handling, integer mode choices,
numeric-string rejection by tag4, decimal parser truncation/overflow bits,
forward spill aliases, resolver mutation of the field pointer, and lookup
mutation proving early matrix versus late vector destination capture.

The copied original body retains the scalar/vector instructions. All 43
direct calls are rebound to the same existing production providers/linked
library functions and the two indirect cells to explicit probe resolvers.
That proves this routine's orchestration and storage behavior for these cases,
not an independent differential of the consumed callees or installed game
resolvers. A wrapper seeds and captures the original local frame and maps
scratch-alias destinations to that frame. Native exception paths are excluded.
The report verifier checked 43 direct main-body call rows and the undefined
handler's tail target (44 total), with no failures; the two indirect cells are
recorded separately. Full hashes, invocation/build logs and the frozen local
probe archive are in `reports/native_lua_field_values_cc10.json`. No
application or game execution is claimed, and no tracked tests are added.
