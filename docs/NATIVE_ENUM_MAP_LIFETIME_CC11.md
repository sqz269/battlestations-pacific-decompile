# Native enum map lifetime

The packet reconstructs three distinct non-deleting bodies: `004BA130`
(32 bytes/11 instructions), `004D25F0` (39/12), and `004D0EA0` (11/2),
totaling 82 bytes and 25 instructions. The first two Ghidra names are descriptive
hypotheses. The correct compiler-derived `CG_adjustor_thunk_004d0ea0` is preserved;
its actual body has no receiver adjustment.

| Body | Complete native SHA-256 |
|---|---|
| 004BA130 | `6efac5e16e91c5d85e690df8e419911921ae393da2330283029d3800fcad590c` |
| 004D25F0 | `ad7488b38a3620ca79f73bf668d48aee90e717b6faf54de8e44ce52768b0552f` |
| 004D0EA0 | `d86a4c669f437f895e987cc8518da51d08a5b18a587ae86bc9653771d90d0a7d` |

Both initializers use ECX storage, save/restore EDI, require DF=0, use `REP STOSD`
for exactly 64 heads, return the original address in EAX, and use plain RET.
They contain no calls, relocations, allocation or release. The MSVC Win32 naked
Source bodies and their complete linked bytes exactly equal Original32/39.
The map initializer writes CE7514 at +0, count zero at +4, and heads +8..+104h.
The outer initializer writes CE78BC at +0, CE7514 at +4, count zero at +8,
and heads +Ch..+108h. It does not clear outside the 10Ch root.

Native ordinary11 writes CE7514 at ECX and tail-jumps from `004D0EA6` to
`004D0760`. The new Source ordinary uses an explicit C++ context interface,
writes that phase word, then binds genuine current complete clear with actual
map, distinct table/symbol pools and raw-string context. Its complete Source
body is 15 bytes/3 instructions.
The compiler emits a tail JMP forwarding the same four C++ cdecl arguments;
it loads the receiver from `[ESP+4]` before the profile store. The three new
Source bodies total 86 bytes/26 instructions, distinct from native82/25.
The linked relative dependency resolves to the fresh canonical clear symbol.
There is no raw tail bridge to an incompatible facade and Original11 is not
executed. Native class/global/EH ABI remains separate from this contextual ABI.

The genuine borrowed map is 108h (264 decimal) at outer+4 inside separately
allocated 10Ch (268 decimal) compatible current-CRT storage. Initializing storage
requires fresh or cleared/unowned fields. The native factory's actual 10Ch
allocation is proved; raw initializer caller reachability is still unresolved.
No standalone 108h allocation or interior free is introduced. Full39 replaces
the prior 34-byte `004D3069..004D308B` setup fragment; that earlier fragment is
excluded from the new whole-body count.

One new ignored family `local/cc11_enum_map_lifetime/run01` compiles 17 fresh
TUs and uses the same two genuine roots/current pools/current-CRT19Ch CEnums/
owning raw strings. On each fresh root, complete Source39 and unchanged Original39
RX bytes run on the same address from the same poison input. After genuine
clear A plus repeat-empty clear, Source32 and unchanged Original32 run on the
same cleared A+4, each from fresh poison, before fresh22symbol reinsertion.
All three comparisons check full EAX identity, EDI sentinel, equal ESP, DF0,
every 10Ch/108h output byte, real pool guards, the outer phase word where
applicable, and other live-root state. These are storage/lifetime and initializer
ABI checks, not a whole original constructor or virtual-class execution claim.

The connected installed 22+6 symbol case retains genuine nonnull-before-null
collision and empty table key, fresh22symbol reinsertion and intact B. New
ordinaryMapA clears actual A+4; genuine ordinaryRegistryB performs both its
real clears. All 50 symbol/five table slots and 131 temporary-pool prefix checks
pass. Both whole roots remain live until explicit fixture frees after all root
checks. Actual pool trim/unlink and string shutdown complete. No header is
overwritten while owned and no dead CEnum/root is read or freed again.

All 17 TUs passed `/O2 /GL- /EHsc /MD /GS- /W4 /WX /fp:strict /showIncludes`
with the exact Community Hostx64/x86 compiler/linker and C1xx/C2 pins.
Actual logs record 46 project headers,
202 host headers and 16
searched libraries. The manifested Win32 probe links full fresh Source bodies;
all 1681 code sections (125161 bytes,
3348 relocations) and 264
unique application spans have complete COFF/linked evidence. The whole32/39
and canonical-provider COFF/link assertions gate execution. The existing native
call verifier confirms the exact original tail-JMP row.

Supports were frozen before compilation from Root's Source273980cf3 full build:
Core `20a5e7feda63309a9c86746e4e248f5fe78c5f270fc215cb627288d8f5e362d4`,
Lua `e9786d9484ea0689bf98ce2ce83437e749d60a05859cfe028e356f12442c08de`,
zlib `c6b5a17d184c45a80e5a9fb6d269a35b85c9732e434396cb31dcc9754a37536f`.
Their consumed copies, actual Source/headers/tool backends/searched libraries,
341 old consumed pins and 4982 historical files
are bookended. The mutable main support state is reported separately. The
sealed pre-execution manifest is preserved; final inventory explicitly includes
the additional consumed COFF-inspector recipe. No old family was replayed and
no failed compile/link/execution attempt occurred in this packet.

Nested `004D0EB0` is a repaired complete 36-byte/12-instruction listing only.
Its Source deleting route remains unready because independent108 allocation
and compatible release lineage are unproved. Outer+4 must never be freed;
flag0-only execution would not complete that body. Workers made no Ghidra
mutations and added no tracked tests. Primary integration, full build/CTest and
independent review remain pending. Historical native allocation/global startup,
ordinary11 original ABI, private EH, foreign classes/heaps, whole runtime/world
and gameplay are outside this packet's evidence.

Sealed receipt SHA-256: `dae76f19013a68e47e3408784f8a9f982eb7874fc53523f312bad99ecefa0742`.
Complete COFF/link receipt SHA-256: `0a779f4914127edee940fde9d8db0e91f48338027bd968581ce6dccfb842d3fe`.

## Primary integration

Three complete bodies: Original 82 bytes / 25 instructions, Source 86 bytes / 26 instructions. The 32-byte map and 39-byte outer initializers are literal raw ECX entries with exact full bytes, preserved EDI, full EAX receiver identity, plain RET and DF0 REPSTOSD. One independent fresh 17-TU family executes Original32/39 versus Source on SAME genuine current-CRT 10C roots and the properly cleared interior map. Real installed 22+6 symbols, collision/null/empty keys, clear twice, fresh22 reinsertion, ordinary map A and ordinary registry B destruction, 50 symbol/five table slot returns and real pool shutdown pass. Source map ordinary destructor is complete 15-byte cdecl with a genuine direct tail to current clear; its extra pool/string context means it is not the native 11-byte ECX ABI, and Original11 is NEVER executed. No interior map free, fake phase provider or old-family replay. Native class, original allocation/private EH/historical heap, nested deleting4D0EB0, parent factory, global namespace, world and gameplay remain external.

Current main Source build `dd9061fc5` passed MSVC Win32 and all three existing CTests. Independent receipt: `local/cc11_enum_map_current_primary/inputs_after.json`. The new fixture used 17 fresh translation units, 300 actual included headers, 19 searched libraries, four pinned compiler/linker/backend files and 5796 stable historical paths. Rebuilt Core `be0bb037e18debf8fbd8e56f2bcfb271480d1d6f663628c1cca69dbe99baddc4` is consumed only by the enum-map family; the base and group families consume no BSP archives. Raw ABI, current source contracts, native class and gameplay evidence remain separately qualified. Saved Ghidra annotation/export/snapshot receipts follow in the report.
