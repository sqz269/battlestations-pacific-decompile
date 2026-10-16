# GOAL16F whole traffic size and float normalization

The complete Native pair is reconstructed as a chosen MSVC Win32 Source interface over actual borrowed header/data/stack storage. Coverage is whole19B/9 instructions plus154B/57, total173B/66. Actual strict and registered Source are whole19B plus150B, total169B/66, with one qualified LEA encoding difference. Names and uint32 return types describe the new Source interface; Original C++ declarations/classes/templates are not recovered.

| Entry | Coverage | Native | Entire Source | Physical inputs / exit |
| --- | --- | --- | --- | --- |
| 0043EC30 | complete | 19B / 9 | 19B / 9 | ECX raw header, no EDX/stack inputs, plain RET, EAX raw count bits |
| 004809F0 | complete | 154B / 57 | 150B / 57 | ECX raw header, no EDX/stack inputs, plain RET, EAX final size bits |

Both public COFF symbols start at offset0 in their own complete executable sections; there are no other own executable sections. The leaf is byte-exact. Every one of the normalizer's57 Native instructions maps in order to an emitted Source instruction with identical mnemonic and data operands, FP widths and relative branch target instruction index. Native00480A09 is seven-byte `8D A4 24 00 00 00 00` LEA ESP,[ESP]; named Source LEA emits three-byte `8D 24 24`. The initial JBE displacement follows the four-byte shortening. This is a whole instruction mapping, not a173B byte-identity claim, NativeSize prefix, or arbitrary byte mask. All six true E8 operands independently bind real public Source providers. Native fixed bytes excluding those operands are149; Source fixed bytes are145.

Code-fetch/instruction addresses, child return-address values, saved x87 FIP and fault addresses follow Source placement, including the shortened layout. Actual child/return-frame cells may alias header/data and expose different code addresses. Even identical raw instruction effects do not establish universal equal-initial-state behavior, original fault/stack-frame identity, drop-in binary compatibility or full Native physical ABI. The Source schedule retains actual borrowed cells; it does not synthesize Native instruction addresses.

## Actual size leaf

Read [ECX+4] into EDX once. If zero, XOR EAX,EAX and RET without reading +8. Otherwise read [ECX+8], SUB the begin using DWORD wrap, SAR EAX,2 and RET. No header-null, alignment, nonnegative-size or valid-range guard exists. EDX remains the loaded begin and ECX is unchanged. The chosen uint32 interface exposes the result bits, including negative SAR results. XOR/SAR supply their actual architectural flag effects, with no invented values for undefined flags.

## Actual normalizer frame, memory and FP order

Let S be entry ESP. PUSH ECX at S-4, PUSH ESI at S-8 and PUSH EDI at S-12 are actual writable cells. ESI captures entry ECX; XORPS and MOVSS overwrite S-4 with +0. This destroys the saved receiver word. Initial EDI is0. No meaningful incoming EDX or stack word is consumed.

Each pass first calls the size leaf and uses TEST/JBE, which skips only a newly read zero size because TEST clears CF. Every loop reloads current begin; null begin diagnoses, otherwise current end is read, wrapped SUB/SAR2 yields size bits, and unsigned CMP/JC checks the current index. The genuine diagnostic may return. After it, begin is reloaded before element access. There is no snapshot or repaired guard; index/pointer arithmetic retains DWORD wrap and late mutable bounds. Physical data/header/actual stack cells can alias.

Accumulation executes FLD m32real element, MOV ECX,ESI, FADD m32real actual S-4, ADD EDI,1, FSTP m32real actual S-4, then the next current-header size call and unsigned loop comparison. Each iteration spills and rounds to float32 before that call. The second pass starts with a fresh size call and index0. It reloads begin, FLDs the element, LEAs and captures its actual destination, FDIVs by CURRENT m32real S-4, restores ECX=ESI, increments index, FSTPs32 to that captured destination, then calls size again. A destination alias can change the sum used by later divisions. There is no zero-sum, NaN, infinity, denormal, malformed-header, negative-size or exceptional-state repair.

POP EDI, POP ESI, POP ECX read the CURRENT actual saved cells. ECX is the current S-4 word, which need not be entry ECX or an unchanged sum. Plain RET uses the actual current return cell. EAX/EDX are final leaf outputs; the last TEST/CMP supplies returned flags. EBX/EBP have no owned writes. XMM0 is initially zero, but a real diagnostic can affect volatile state; no fabricated final XMM0 value is promised.

No ST0 semantic return is invented. Each completed nonfaulting own FLD has an FSTP; the arithmetic uses temporary ST0. No FP control word, status, rounding/precision mode, MXCSR, x87 stack clearing or exception mask is installed. The existing lower stack, pending exceptions and real SDK effects remain actual machine state. Identical FP instructions/operand widths preserve their instruction-driven sequencing, but no unexecuted mode/NaN/denormal/exception payload or runtime result is asserted.

## Six real Native calls and qualified Source bindings

| Actual Native CALL | Actual Native target | Source instruction offset in150B body | Genuine Source symbol |
| --- | --- | --- | --- |
| 00480A00 | 0043EC30 | 16 | `?size_native_traffic_dwords_0043ec30@bsp@@YIIPBX@Z` |
| 00480A23 | 00BF6713 | 47 | `?invoke_native_invalid_parameter_00bf6713@bsp@@YAXXZ` |
| 00480A3B | 0043EC30 | 71 | `?size_native_traffic_dwords_0043ec30@bsp@@YIIPBX@Z` |
| 00480A48 | 0043EC30 | 84 | `?size_native_traffic_dwords_0043ec30@bsp@@YIIPBX@Z` |
| 00480A64 | 00BF6713 | 112 | `?invoke_native_invalid_parameter_00bf6713@bsp@@YAXXZ` |
| 00480A7D | 0043EC30 | 137 | `?size_native_traffic_dwords_0043ec30@bsp@@YIIPBX@Z` |

The only outside provider is the existing public cdecl diagnostic. Its complete Native16B/9 instructions contain CALL00BF671A to00BF66EF. The complete current Source17B/9 uses the actual SDK `_invalid_parameter_noinfo(void)` declaration (corecrt.h:371), distinct from the noreturn API at372. It executes XOR, five PUSH0 padding words, imported returning zero-argument API call, ADD ESP,14h and RET. Those five words are not fabricated API parameters. Its actual DIR32 IAT relocation names `__imp___invalid_parameter_noinfo`, linked from `bsp_core:native_invalid_parameter_noinfo_call.obj` to the real UCRT runtime import. The whole actual object is byte-identical to the accepted Z provider. No fake handler/global/stub, host callback, guard, snapshot or restricted allocator domain is introduced.

Current SDK policy does not establish Native00BF66EF encoded handler/debugger/Watson identity, Original private CRT/FH3/RTTI, original instruction/fault addresses or C++ exception propagation through this naked parent. Source does not mark the diagnostic noreturn/noexcept or convert exceptions.

## Every indexed incoming call

Fresh whole typed caller ranges and full installed/live bytes are retained. There are eight normalizer callsites from four functions and six outside size callsites from two functions; the normalizer's four size calls above complete the size leaf's ten actual callsites in three indexed callers.

| Containing function | Actual CALL site | Actual target |
| --- | --- | --- |
| 004A3610 | 004A3BA5 | 004809F0 |
| 004A3610 | 004A3C8D | 004809F0 |
| 004A4B70 | 004A4EFB | 004809F0 |
| 004A4B70 | 004A4FEF | 004809F0 |
| 004A5850 | 004A599E | 004809F0 |
| 004A9BD0 | 004AA0C5 | 004809F0 |
| 004A9BD0 | 004AA0CF | 004809F0 |
| 004A9BD0 | 004AA0D9 | 004809F0 |
| 00480A90 | 00480ACC | 0043EC30 |
| 00480A90 | 00480B0E | 0043EC30 |
| 00A0C650 | 00A0CC5B | 0043EC30 |
| 00A0C650 | 00A0CCAB | 0043EC30 |
| 00A0C650 | 00A0CCBE | 0043EC30 |
| 00A0C650 | 00A0CCFD | 0043EC30 |

Normalizer ECX setups are actual ESP+78h / ESP+68h in004A3610, ESI+100h / ESI+120h in004A4B70, EDI=ESI+30h after real004951A0 assignment in004A5850, and actual E1876Ch/E1879Ch/E1877Ch header addresses in004A9BD0. No normalization stack arguments are pushed; the previous assignment owns its separate pushed source-header word. The two size calls in00480A90 receive captured ESI (entry EDX of that OTHER function), and the four in00A0C650 receive actual ESP+68h or+54h. Their differing interfaces are not imported into either owned entry. Callsite windows and all read ranges are retained; partial typed-body alignment holes are excluded rather than silently treated as instructions.

Those external mutable header producers/globals and whole physical parents remain OPEN. LegacyLeft8bHost's abstract weights_normalize callback is not a complete physical Source provider. This pair does not close the named insertion/resize specializations, and accepted T/X/Z standalone fixtures do not prove this parent's exception/runtime paths.

## Build, one focused fixture and exact limits

Strict C++17 `/MD /O2 /Gy /W4 /WX /fp:strict /EHsc` and actual registered C++17 `/O2 /Oy- /MD /W4 /WX /fp:strict /EHsc` both emit the complete169B/66 pair. Two warm `./scripts/build.ps1` invocations pass: initial 19.3446022s and final EOF-only refresh 18.9247230s, total 38.2693252s. Initial existing reconstructed_math/tool_tests pass in0.21/7.20s; final checks pass in0.22/8.01s. Initial build runs 2026-10-16T06:40:56.0805868Z to 2026-10-16T06:41:15.4287016Z; final runs 2026-10-16T06:51:38.3734130Z to 2026-10-16T06:51:57.3005875Z. Native reference seeds are not enabled in this worker. Root's later three-check build is separate. No new registered tests or broad suite was added.

One ignored focused fixture addresses the concrete loss-of-spill/actual-cell-alias risk. It links the actual Core archive only, with embedded manifest, and executes the public Source plus captured whole Native154 parent whose six genuine Source provider operands alone are rebound. Data spans actual S-12 saved EDI, S-8 saved ESI and S-4 sum, initialized by the parent's own PUSHes/MOVSS to16777216,1,0. Float32 spills make the first two additions16777216; the third reads the current sum through its data alias and makes33554432. Division writes0.5,2^-25,1 into the three actual cells. Both executions observe EDI3F000000, ESI33000000, ECX3F800000, EAX3, EDXactualdata and plain-RET ESPdelta0. The fixture's FNINIT-controlled CW037F yields observed SW0020 and final arithmetic flags44 under mask8D5; FX state is saved/restored by the fixture, never initialized by Source. First fixture compilation/run PASS; no rerun.

This case executes both normal loops with nonzero sum, including destination capture and actual aliased saved-cell publication/POPs. It does NOT execute diagnostic return, empty/malformed/negative/wrapped ranges, zero-sum division, NaN/infinity/denormal, unmasked traps, alternate control modes or deeper x87 stacks. It establishes no full fault/unwind/EH/Original ABI/game result.

All own executable sections and the genuine17B provider, their complete non-code sections/symbols/relocations, two exact actual archive members, all eight linked executable section contributions and four unselected fixture COMDAT sections are retained/accounted. Every selected code relocation resolves to the actual linked operand and whole PE code. Full captured Native154 is present as fixture data before rebinding; it is not a Source byte-embedding implementation. Fresh installed/live equality covers13,294 bytes in ten complete typed functions, including all173 owned bytes. Source/header/SDK declarations, actual C++17 TLOG/project, archive, objects, map and fixture are frozen.

After the first successful fixture, a staged whitespace check found trailing blank Source lines. The shell command still made provisional commit145ad2d73 instead of stopping. Only those EOF lines were removed, then strict compilation and the required registered build were refreshed. No second runtime fixture or relink was run solely for whitespace. The entire first actual archive/object/map/fixture/build/Source inputs are preserved under pre_eof_formatting. All final current executable section bytes, every relocation and all symbols are independently equal to those original fixture archive inputs; each whole object has one unique member in its respective actual archive. The existing fixture PE also matches the final current code with all actual operands resolved. Source/COFF/archive hashes differ as recorded; this is explicit unchanged-code provenance, not a claim that the old fixture linked the later archive.

## Preserved inputs and corrections

Warm author base remains e4605665f60c2c9622887d9360d1aee106de19d7; no Main merge/new baseline. Before reuse, all134 Z files (all131 receipt artifacts verified) plus14 actual prior registered inputs were copied immutably under this packet. Readiness HANDOFF SHA256 is380f739faa727876c9da891f0410f65cc829474b5b12ef1a6fccb42c769705ff; its full directory is retained.

Initial ALIGN16 compiled but emitted a seven-byte NOP and was rejected; full Source/object/listing retained. A symbolic label subtraction was rejected C2425; that Source is retained. Final direct LEA compiles and is qualified as three bytes. One shell-quoting attempt to append CMake failed before writing; exact registration script appends one owned line, verified against base. Static map parsing initially included its `f` function flag in origin; corrected parser strips only that flag and validates actual archive providers/operands. There are four strict compilation invocations (one rejected encoding, one C2425, one direct LEA success, one EOF-only refresh), two registered build/check invocations and one fixture compile/run. All classification/rejection/reporting/terminal-check correction time is included in the full wall clock.

Only own address records in43/48 name/reconstruction shards and one CMake append accompany four new module/doc/report files: nine owned tracked files. Old correct size name/tag/comments and current Ghidra placeholders are preserved; author makes no Ghidra writes. Root owns independent review, later annotation, Main integration and game adoption decisions. Final ignored HANDOFF/receipt contain exact commit/hash pins and inclusive elapsed time from06:33:59Z before first capture through commit/release.
