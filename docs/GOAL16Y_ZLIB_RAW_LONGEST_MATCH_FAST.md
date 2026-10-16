# GOAL16Z implementation of the GOAL16Y longest_match_fast design

Only **00BCBB40..00BCBC05**, complete **198B / 74 instructions**, receives Native reconstruction credit. Two real Source functions emit **211B / 77 instructions**: the private complete custom-register kernel198B/74ins and a new public frame-transfer adapter13B/3ins. Strict C++17 and the actual registered Win32 kernel reproduce every Native byte. This is strong evidence for this artifact, not a universal Source identity gate or a clipped Native-size prefix.

The accepted MAX Y design seals are handoff `13e18054e67e457b5f51bde78f5b72e9e152b833c1b1b1fad40b0414a4269e9a`, design `41633c691a151fe519d2367c4b8c5058982c0367277b3d115da9b79c6ae2acec`, DESIGN.md `108f1e3caf3236d91190179fd7591563a98d2ae56208da66cbbc5add5fb79f10`, manifest `c4e850480ee0ac2015d700de459bbc2f688b24046417a8283cec2fae176734b4`, receipt `698800481d962007cf3b68c3d73af5fc82f8245c7f5c62f5d774145969c067d4`. All39 declared records186,281B plus the two seal files were verified/copied:41files194,518B. Fresh author typed/live/installed-PE evidence independently verifies the whole198B body, SHA256 `d114606bb75aaf600e5535d348fe5eb6b179f47da3bcad923c6285ff3378e82c`, in `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, I386/base00400000, one complete range, no thunk, entry flow overrideNONE and **Original no_return=false**. No Ghidra metadata was written.

## Two genuine Source inputs

```cpp
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_longest_match_fast_frame_00bcbb40(
    void* backing, void* kernel_entry_stack) noexcept;
```

The public adapter reads its actual arguments with MOV ESI,[ESP+4], MOV ESP,[ESP+8], then JMPs to the genuine defined private Source kernel. These are two real inputs: raw backing bits and the actual chosen kernel-entry ESP. It adds two ordered fault-capable reads and changes ESI/ESP; it preserves flags, EAX/ECX, **all EDX bits**, EBX/EBP/EDI before the kernel. No third duplicate current_match input, copied frame, saved ESP global, invented continuation/provider or ordinary-return shim exists.

The private kernel receives **ESI=backing and [actual entry ESP+4]=current_match**. Its internal `uint32_t __stdcall longest_match_fast_body(uint32_t)` spelling describes only the observed EAX result and one-stack-word/RET4 shape, not an Original declaration or a standard ESI ABI. It is reached by assembly transfer; ordinary C++ must not call it. Formal Original types, layout/version and build flags remain unproved.

The supplied frame must contain a real Source execution continuation and deliberate stack/register protocol. Either actual RET4 reads its current continuation and normally leaves ESP=entry+8. The Source caller's abandoned call frame may alias the chosen frame or input data. Public Source-only `noreturn` promises no ordinary return through that C++ call site; it does not apply to the returning private kernel or change Original `longest_match_fast`/no_return=false. A future consumer must establish its own concrete continuation contract. No caller is adopted here.

## Complete raw schedule

Let B be backing and S the chosen kernel-entry ESP. Arithmetic/effective addresses wrap modulo32, with no object extent, alignment, null/bounds, current_match<=start or lookahead>=2 condition invented.

| Cell | Actual ordered use |
|---|---|
| S | Current continuation read by either RET4; aliases may alter it. |
| S+4 | Current_match DWORD read once after PUSH EBP into cached EBP. |
| S-4 | Incoming EBP PUSH, later actual POP, potentially changed by aliases. |
| S-8 | Incoming EDI PUSH after the first byte CMP, later actual POP. |

1. Before any PUSH, load window DWORD[B+30] into ECX and start DWORD[B+64] into EAX. PUSH EBP, read current_match at S+4 into EBP, then read match byte0 at window+current_match into **DL only**. ADD EAX,ECX and ADD ECX,EBP form cached scan/match bases. CMP DL,BYTE[scan] precedes PUSH EDI and LEA EDI,[scan+258]; these preserve the comparison flags before JNZ. Even an initial mismatch saves EDI before exiting.
2. If byte0 matches, read match byte1 into DL, compare scan byte1 and JNZ to fallback2. Both comparisons read match before scan. Fallback writes EAX=2, POPs actual EDI/EBP and RET4. It never reads lookahead or stores B+68, even when lookahead would be0 or1.
3. Otherwise ADD2 to both cursors. **Byte2 is skipped.** Each of eight individual scalar slots reads scan BYTE[EAX+1] into DL, ADDs1 to EAX and ECX, then compares DL with match BYTE[ECX]. A match-side fault occurs after DL and both cursors changed. First group offsets are3..10; no memcmp, wider load, snapshot or EDX normalization is substituted.
4. Only after eight successful slots, CMP raw EAX,EDI and JC repeats. The bound is wrapped scan+258; it is not a repaired relative-length or lookahead guard. A reached group can read byte258.
5. A mismatch or exhausted raw bound executes SUB EAX,EDI; ADD EAX,258; CMP EAX,3; signed JL to fallback2. The literal branch stays even though the intact-cursor proof bounds computed length to3..258.
6. Otherwise read lookahead DWORD[B+6C] into ECX, CMP computed EAX,ECX, then store cached current_match EBP to DWORD[B+68] **before JBE**. MOV preserves CMP flags. The store happens even if lookahead0/1 eventually clamps below3 and can fault or alter saved, argument, continuation or input cells. Lookahead remains cached after an aliasing store.
7. JBE uses the common POP EDI/POP EBP/RET4 exit with computed EAX. The clamp path instead POPs EDI **before** MOV EAX,ECX, then POPs EBP and uses the other actual RET4. A first-POP fault on this route still has computed EAX. Moving a C++ min ahead of restoration would change that prefix.

EDX upper24 bits stay their real incoming value on every corresponding prefix and normal path. Only DL changes. ESI remains backing; EBX is untouched. Actual saved cells govern the restored EDI/EBP, not assumed incoming values. Early byte0/1 fallback flags come from the 8-bit match-minus-scan CMP, DL is that match byte and ECX is cached match base. Computed fallback flags come from CMP computed,3 and DL is the last scan byte. Stored-result routes retain the 32-bit computed-minus-lookahead CMP flags; ECX is cached lookahead. MOV/POP/RET do not replace them, and there is no final ADD ESP. FP/SIMD, DF and other non-arithmetic flags are untouched by these instructions, subject to execution/placement qualifications.

## All-bit cursor proof

For M=2^32, arbitrary scan A and E=(A+258) mod M, the cursor after complete group k is Ck=(A+2+8k) mod M. Only then is unsigned Ck<E tested. The three cases partition every uint32 A:

| A range | Wholly equal comparisons |
|---|---|
| 0..M-259 | No wrap before C32=E; length258. |
| M-258..M-11 | Bound wraps but C1 does not; C1>=M-248 and E<=247, so first group stops at length10. |
| M-10..M-1 | First completed cursor and bound both wrap; C1..C31<E, C32=E; length258. |

A mismatch stops earlier. Reached relative comparisons are0,1 and a prefix of3..258. Modular cursor-E+258 yields the reached relative offset3..258. This static proof is not target execution and does not justify removing instructions, restricting addresses or fixing skipped byte2. All malformed backing/window/start/current_match/lookahead/byte bits and actual frame/data aliases remain admitted. Code/self-modification and concurrent changes remain qualified separately.

## Existing Source, providers and validation

The fetched zlib1.2.1 `deflate.c` already supplies genuine complete algorithm Source: a192B private `_longest_match_fast` in the actual separate `bsp_zlib121.dir/Release/deflate.obj`. Its complete section, disassembly and real member proof are retained in Y/P evidence. It has two stack inputs/cdecl/plainRET, different saved/cursor registers including CL scratch, different PUSH/POP/INC/LEA schedules and store-before-lookahead-load order. It is not this complete198B Native physical binding, a Core provider or new address credit. Its version/layout/default-fastcall hypothesis does not establish Original facts. Whole current worker/Root/Main src/include searches and the indexed address lookup found no existing standalone Native binding. Named parents `deflate_fast`00BCBF40 and `deflate_slow`00BCC2A0 remain unresolved adoption boundaries and were not reconstructed or redirected.

Strict C++17 `/MD /O2 /W4 /WX /fp:strict /Gy` and actual registered C++17 `/MD /O2 /W4 /WX /fp:strict` objects were inspected independently. Both have exactly two whole executable sections/functions at offset0, no hidden helper/gap/prefix clipping, all74 kernel instructions and all3 adapter instructions. All17blocks/28edges, two RET4 exits, widths, partial-register/flags/frame/byte ordering match the closed design. All sections/raw data/symbols/auxiliary entries/strings/relocations are recorded. The only code relocation is IMAGE_REL_I386_REL32 at adapter operand+9 to the defined private static kernel, with zero addend. No external provider, Native-address JMP, global or FP operation is introduced.

The actual Core contains one unique genuine object member equal to the registered object. The public `_bsp_native_zlib_longest_match_fast_frame_00bcbb40` is unique in both complete archive dictionaries, whose offsets/indexes all resolve actual member headers. The private kernel is not an invented archive export. All eight previous providers remain byte-identical to their prior registered objects and Source/header pins.

Before cache advance, the already frozen W Core, eight objects, project and tlog copies were hash-matched against the current inputs and reused in place. No transitive P/R/Q/W trees or whole Core were recopied for historical preservation. Only fresh41-file Y evidence and the small current CMake text were copied. The new final actual object/Core/project/tlogs are preserved once for this review. The first preparation-only script assumed older reports had a top-level source path; it failed before tracked Source/cache changes, was retained under rejected/, then repaired to derive missing paths from genuine archive member module names. Strict Source compilation passed first attempt with no warnings or C++ corrections.

Normal `scripts/build.ps1` passed both existing checks in17.571653s, including CTest7.24s. It repeated the existing unrelated LNK4006 `spawn_request_id_matches` duplicate warning present in W. No new tests/probes/Native execution or Ghidra writes occurred. Static identical instruction/prefix schedules do not dynamically measure faults or prove absolute code/fault/return EIPs, instruction-fetch/current-stack placement, relocated code/data/self-modification aliases, concurrency, Native SEH/unwind, full Original ABI, caller adoption or gameplay. The adapter's additional argument reads are explicit differences from Original entry.

Y design time554.515533s through seal and historical P readiness943.721305s are separate from Z. Y seal10:25:23.515533Z to Z first clock10:29:04Z is220.484467s of post-seal review/administration/scheduling, not omitted or attributed as implementation. Z includes all preparation/failures, preservation verification, qualification, literal Source/strict compile, normal build/checks, whole module/Core proof, metadata/docs/commit/release/seal. The final ignored receipt closes author time; no speedup claim is made.

Root annotations after independent review must preserve old values, correct `longest_match_fast` and Original no_return=false; proposed physical evidence is custom ESI backing, actual [entryESP+4] current_match, EAX result/RET4 and the DL/frame/byte/store/flags schedule. Public Source noreturn must never be copied onto Original. Exact commit and final hashes are delivered separately.
