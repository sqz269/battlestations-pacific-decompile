# Raw unit-list erase at 004837D0

`erase_native_unit_list_004837d0` reconstructs the complete **79-byte / 31-instruction** entry at `[004837D0,0048381F)`. The descriptive name is inferred. Source preserves every byte outside the two free CALL operands and calls the existing complete canonical `singleton_lifetime_free` directly; this entry needs no adapter.

## Native evidence and repaired boundary

The installed executable and the live saved `bsp` program agree on the whole entry, SHA-256 `0909c4202ea2816202a70c2f3dc821e0512f88069d0377378aa1adb85f3f1fc2`. The installed executable SHA-256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Both the erase bytes and accepted append producer's native reference bytes matched before and after this fixture. Each supported Ghidra batch verified `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`; the worker made no Ghidra mutations.

Root previously cleared the two erroneous CALL_RETURN overrides and restored both nine-byte returning epilogues. The saved body now contains all 31 instructions, including both `ADD ESP,4 / MOV EAX,ESI / POP ESI / RET 4` sequences. Root corrected the inferred prototype to one implicit ECX receiver and one explicit stacked node. The first duplicate-receiver attempt remains historical evidence, not accepted ABI evidence. Original bytes and the global `_free` metadata stayed unchanged. See the bounded readiness document and report for those separate repairs.

The native CALLs at `004837FA` and `00483811` target `00BF65AC`. Source has exactly two `IMAGE_REL_I386_REL32` relocations, at operand offsets **43 and 66**, both to the same actual freshly compiled canonical free. Original-bound execution changes only offsets `43..46` and `66..69` to that same actual free. The other **71 bytes**, both branches, instruction widths, fresh node reads, root/link/count/tail writes and their order remain literal.

## Physical contract and lifetime

The external MSVC Win32 signature is `void* __fastcall erase_native_unit_list_004837d0(void* actual_list_ecx, void* unused_edx, void* actual_node) noexcept`. ECX is the actual stable 12-byte `{count, head, tail}` root; EDX is unused padding; the one stacked DWORD is the actual coherent live node owned by that root. Nodes are actual 12-byte `{previous, next, borrowed_payload}` allocations produced by accepted append Source in the canonical current-CRT domain. The caller owns the root and payload lifetimes. Erase consumes the node allocation through actual free.

Source splices the predecessor or head, freshly reloads successor and predecessor, then updates successor/count or count/tail before free. A live successor is retained in ESI **before free**; the tail path clears ESI. Both paths return that pre-free successor in EAX, restore caller ESI and execute `RET 4`. No node is read after free. Nonvolatile GPR preservation follows the explicit ESI save plus the actual CDECL free contract. **ECX and EDX are volatile across free** and have no asserted output values.

Final arithmetic flags come from **`ADD ESP,4`**, after the real free call, rather than the count decrement. If the raw caller's argument-top ESP is `T`, target CALL, saved ESI and the pushed free argument produce free-return ESP `T-12`; the final ADD produces `T-8`. MOV/POP/RET then preserve those arithmetic flags. The raw fixture derives CF/PF/AF/ZF/SF/OF from each actual captured `(T-12)+4`, and checks final after-ESP `T+4`. DF is clear on CRT entry; no blanket FP, DF or segment-state preservation is claimed.

This contract admits valid normal current-free behavior. It does not establish original private CRT/EH identity, malformed/unowned/null node handling, native class/parent/world ownership, or arbitrary binary drop-in compatibility. No guard, callback, fake node, alternate heap, or new allocation adapter is introduced by erase.

## Fresh connected worker family

Family: `J:/PROG/battlestations-pacific-decompile-cc12_raw_unit_list_erase/local/cc12_raw_unit_list_erase_worker20261008a`.

The component is frozen at main `2642b8c129ff3a16123fa2b636fa9010f2cf2235`. Exactly four fresh TUs compiled once each: erase Source, unchanged accepted append Source with its private size adapter, unchanged current `singleton_lifetime.cpp`, and a fresh ignored caller probe. The executable linked once. No old object, BSP archive, old executable, old recipe, Original append execution or new tracked test was used. Common binary-parsing/CRT-attestation source text was reused read-only in the new recipe/probe; the old accepted families were never executed or modified.

MSVC 14.51.36231 targeted I386 with `/std:c++17 /MD /O2 /W4 /WX /fp:strict /permissive- /EHsc /Gy /GL-`. The link used `/OPT:REF /OPT:NOICF /INCREMENTAL:NO /BASE:0x23000000 /DYNAMICBASE:NO /MANIFEST:EMBED`. The embedded manifest required `asInvoker`, `uiAccess=false`, before launch. All actual Source/header/recipe/probe inputs, compiler/backend/link/resource tools, 185 consumed headers including extensionless standard headers, seven resolved input libraries, original PE, four objects and executable were pinned before and after.

| Fresh complete runtime span | Bytes | Instructions |
| --- | ---: | ---: |
| Erase Source | 79 | 31 |
| Accepted append Source | 87 | 36 |
| Append private size adapter | 61 | 17 |
| Canonical allocator | 90 | 34 |
| Canonical free | 6 | 1 |
| Ordinary erase caller | 18 | 5 |
| Ordinary producer caller | 16 | 5 |
| Raw erase capture caller | 140 | 52 |
| `std::bad_alloc` constructor | 24 | 6 |
| Security-cookie helper | 14 | 4 |

These counts were derived from the newly compiled/linked artifacts, not inherited from an older 90-byte/37-instruction or 90-byte/34-instruction allocator receipt. The first nine spans were checked against complete COFF bytes and every resolved relocation. The tenth is the complete actual linked `MSVCRT:secchk.obj` helper, with its extent independently derived from reachable instructions through successful RET and the failure tail JMP. All ten complete spans were inspected and checked live before entry and after the last erase. Both erase and append have unique normalized linked occurrences. The complete bound Original erase bytes were also checked before/after; only its two CALL operands differ from original.

## Actual CRT gate and sole execution

Before **zero producer or erase entries**, the process required actual `malloc`, `free` and `_callnewh` IAT values to equal `GetProcAddress` and mapped I386 module base plus physical export RVA. It verified MEM_IMAGE/base, live/physical PE metadata, mapped and opened NT paths, volume/file identity, complete physical DLL SHA-256 and 32-byte export prefixes adjusted for actual image relocations. Read-only handles denied write/delete sharing through the final check. Startup/probe API allocations are separate from target entry counts.

All three heap API-set imports resolved to physical `C:/Windows/SysWOW64/ucrtbase.dll`, SHA-256 `60c5a497b52de80a3a0677564270dbea7e486086637debd567b4dffb28584c1b`. WOW64 reported a System32 loader DOS path, while both the mapped and opened physical NT path proved `\Device\HarddiskVolume3\Windows\SysWOW64\ucrtbase.dll`. These checks passed again afterward. The actual allocator retains its real retry/new-handler/exception code; no forced OOM, handler invocation, reentry or failure simulation was used.

One process ran once and passed **9 genuine producer allocations and 9 erase dispositions**. The same declared aligned caller root and four actual borrowed payload objects were reused across Original-bound erase and Source erase phases. Each phase used freshly compiled accepted append Source to create four real nodes, then erased middle, head, tail and singleton. A ninth appended singleton exercised the ordinary compiler-emitted Source erase caller. Fresh node addresses differed naturally across phases and were never forced to match.

Every removal checked actual root count/head/tail, every surviving node's previous/next/payload fields, the returned actual successor, declared caller guards and unchanged borrowed payload bytes. Survivor arrays contain only captured genuine live pointers and serve only as an observation oracle; they never supply fake node storage, target count or allocator results. No erased node was dereferenced, and no extra teardown free was required after the root became empty.

All eight raw captures observed `T=1730368`, free-return ESP `1730356`, final ADD result `1730360`, after-ESP `1730372`, and EFLAGS `0x202`; arithmetic mask `0x8D5` was zero as derived from that actual stack addition. The two successor-producing removals in each phase returned the actual remaining successor; tail/singleton returned null. EBX/EBP/ESI/EDI were preserved. ECX was genuinely clobbered by free to `4043682357`, and was correctly recorded without a preservation assertion. This run demonstrates its admitted cases and actual stack addresses, not unexecuted failure paths or every possible flag combination.

All 476 older artifacts remained unchanged: the prior 271 artifacts, worker append final seal's 80 artifacts plus final seal, Root append's 78 artifacts plus seal, and erase readiness's 44 artifacts plus manifest. Their frozen historical Source/config associations were preserved without repinning old receipts to subsequently changed main files.

Core seal: `70f7f076f9509d186a26fa93a271811c2678b68c4fa7adf9bdcb80723b140030`. The companion report records exact Source/header hashes, full static/runtime observations and receipts. This worker establishes strict standalone four-TU compilation and bounded current-CRT behavior. Root owns the independent fixture, shared CMake/ledgers, locked Ghidra annotation/save/export, full Win32/all existing CTests and main integration. Only the 79-byte/31-instruction erase body receives new native credit after that acceptance; append/glue counts are unchanged. Historical private CRT/EH, parent/class/world ownership, startup and gameplay remain unestablished.
