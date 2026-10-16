# GOAL16W implementation of the GOAL16U scan_tree design

Only **00BCE180..00BCE263**, the complete **228-byte / 72-instruction** `scan_tree`, receives Native reconstruction credit. Two real Source functions emit **245 bytes / 76 instructions**: a private complete register kernel and a public 17-byte transfer adapter. Strict C++17 and the actual registered Win32 object reproduce all 228 kernel bytes. This is evidence for this artifact, not a universal Source identity requirement.

The MAX-recovered U design was sealed before implementation: handoff `98af49a328d163e457a198d25c813a48fc14e034039a142a876c2529a3e11746`, design `b1f05012dc27e48319cfd8ff0f7cb39baab0525b5625dfeede3a1411978393f7`, manifest `37bf8a6aee00d83cd4c0bda5ae3968833d69f83b014b56af50b065aa07ce6347`. All 33 declared files / 211,980 bytes were verified and copied into ignored author evidence. Fresh typed Ghidra evidence verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, I386/base00400000, one complete range, no thunk, entry flow override NONE and **Original no_return=false**. All live bytes match the installed PE and U body, SHA256 `09a355e27274208ea48f319701213595580fbdbf5c73d13741967b13868e8b83`.

## Source interface and Original physical inputs

```cpp
extern "C" __declspec(noreturn) void __cdecl
bsp_native_zlib_scan_tree_frame_00bce180(
    void* tree, std::int32_t max_code, void* kernel_entry_stack) noexcept;
```

These are three genuine inputs: writable tree address bits, all 32 signed max_code bits, and the actual chosen kernel-entry ESP. The naked adapter reads its actual stack arguments in order with MOV EAX,[ESP+4], MOV ECX,[ESP+8], MOV ESP,[ESP+C], then JMPs to the defined private Source symbol. It introduces three real fault-capable reads and changes EAX/ECX/ESP, preserving flags and incoming EDX/EBX/EBP/ESI/EDI before kernel entry. Its old call frame is abandoned, never copied into the chosen frame. These frames may alias.

The private body takes **EAX=raw writable tree, ECX=signed max_code bits and [entry ESP+4]=raw backing pointer**, ending with RET4. Its internal `void __stdcall scan_tree_body(void*)` spelling describes one actual stack word/RET4, not an Original standard register ABI or formal return type. Ordinary C++ must not call this private body. The public Source-only `noreturn` describes transfer away from its C++ call site; it does not apply to the private kernel or Original metadata. Original `scan_tree` remains returning/no_return=false.

The chosen frame must carry a real Source execution continuation and stack protocol. RET4 reads its current continuation, normally leaving ESP=entry+8. No fourth backing input, frame copy, continuation snapshot, saved ESP global, ordinary-return trampoline, fake class, provider, guard or caller adoption is added. Formal Original types, private zlib layout/version and build options remain unknown.

## Complete ordered body and aliases

All address arithmetic wraps modulo 32. With entry ESP=S, the deepest ESP is T=S-24. These are actual memory cells, not a C++ struct or an immutable save area.

| Cell | Actual use |
|---|---|
| S | RET4 reads the current continuation. |
| S+4 / T+1C | Backing pointer read once before the ESI/EDI saves; then overwritten with cursor, reloaded each iteration and DWORD-RMW incremented by 4. |
| S-4 / T+14 | Nonnegative path writes wrapped max_code+1; actual DWORD SUB1 controls the loop. |
| S-8 / T+10 | Previous length, initially FFFFFFFF; read and updated on the flush paths. |
| S-12, S-16, S-20, S-24 | Actual incoming EBX, EBP, ESI, EDI saves and reverse-order POPs. |

1. SUB ESP,8; PUSH EBX/EBP; read actual backing from S+4 into EBP; PUSH ESI/EDI; read unsigned WORD[tree+2]. XOR EDX establishes count0, TEST of the cached length supplies flags through MOV/LEA and the first JNZ. Previous-length=-1 is written before the sentinel. Initial thresholds are maximum7/minimum4, or maximum138/minimum3 for initial length0.
2. TEST signed max_code, then **unconditionally store WORD FFFF at tree+4*max_code+6**, then JL to restore. Negative max_code therefore still has the preceding saves, reads, local write and sentinel store. The sentinel may alias frame, saved, backing or continuation storage.
3. For nonnegative max_code, ADD tree6, ADD max_code1 modulo32, write the actual counter and actual cursor argument cell, set EBX1 and retain the original NOP. INT32_MAX is not narrowed or repaired.
4. The loop caches current length in EAX, reloads the current actual cursor cell, reads the next unsigned WORD into EDI, then adds1 to EDX. Signed count/max and current/next comparisons determine whether to flush. The next length is cached before frequency writes.
5. Signed count below minimum performs one unLOCKed WORD ADD of DX at backing+4*length+A74. Otherwise nonzero length may ADD BX1 at that frequency when the actual previous-length cell differs, then separately ADD BX1 at backing+AB4. Zero length selects backing+AB8 for signed count<=10, otherwise backing+ABC. Each WORD RMW has its own ordered access, flags and modulo16 store. Lengths above15 are retained; no clamp or backing extent is invented.
6. A flush XORs count0, TESTs cached next length, writes actual previous length, then selects 138/3 for next0, 6/3 for equal nonzero lengths or 7/4 for differing nonzero lengths. Skipping the flush retains count, previous length and thresholds.
7. Every route DWORD-RMW ADD4 to actual cursor S+4 **before** DWORD-RMW SUB EBX1 to counter S-4. JNZ consumes that SUB's actual result. Unaligned or overlapping frequency writes can alter these cells and future accesses; additional iterations or nontermination remain possible. No bounded-loop or snapshot substitution is used.
8. POP actual EDI/ESI/EBP/EBX, ADD ESP,8, RET4. The final ADD sets arithmetic flags from S-8 plus8 modulo32; these are not the last frequency/test/loop flags. Negative-path EAX retains the tree bits, ECX initial7/138 and EDX0. On loop completion EAX is the cached final current WORD length and ECX/EDX reflect its threshold/count path. Aliased saved/control cells still govern restoration and transfer.

The report records all 72 instruction effects and the whole 22-block / 32-edge U map. Exact kernel code preserves widths, signed branches, flags and static fault-capable prefixes at corresponding states. No dynamic faults were measured. FP/SIMD, DF and non-arithmetic flag bits are untouched by these instructions, subject to code/current-stack placement qualifications.

## Existing Source and scope

The fetched zlib1.2.1 `trees.c` already has a whole private `_scan_tree` helper: its genuine 248-byte section is in the separate actual `bsp_zlib121.dir/Release/trees.obj`. It uses three stack inputs/cdecl/plain RET, different local/cursor and WORD INC schedules. That is complete algorithm Source, not this Native physical binding or a Core provider. Its presence is explicitly retained in the U evidence; no stock dispatch or public callback/parent closure is claimed. Whole src/include searches and the indexed address lookup found no standalone BCE180 binding before creation.

Strict `/std:c++17 /MD /O2 /W4 /WX /fp:strict /Gy` and actual registered `/std:c++17 /MD /O2 /W4 /WX /fp:strict` objects were inspected independently. Every executable section is a whole real function at offset0: private kernel228B and public adapter17B, no gap/helper/prefix clipping. All sections, raw data, symbols, auxiliary entries, strings and relocations are recorded. The only relocation is the public adapter's genuine IMAGE_REL_I386_REL32 JMP operand at offset13 to the defined internal kernel. No unresolved external, Native-address jump, global or FP provider exists. The private symbol is not invented as an archive export.

The actual Core has exactly one real member for this object, byte-identical to the actual registered object. The public `_bsp_native_zlib_scan_tree_frame_00bce180` occurs uniquely in both complete archive dictionaries; all dictionary member offsets/indexes resolve genuine headers. The seven prior packet providers and their Source/header pins remain unchanged. Their current objects, Core archive, project and tlogs were preserved once before cache advance (36 files / 168,667,145 bytes); P48/R70/Q1669 seals are referenced in place without recursively copying or rehashing transitive trees.

Normal `scripts/build.ps1` passed both existing checks; warm build18.777423s includes CTest7.84s. It repeated the unrelated existing LNK4006 `spawn_request_id_matches` duplicate warning, also present in the prior Q build. Strict compile passed first attempt; no C++ correction, new test, probe, Native execution or Ghidra mutation occurred. A Windows rg wildcard error and oversized read selections were corrected during evidence inspection; that overhead is included.

Absolute code/fault/return EIPs, instruction-fetch boundaries, Original code/data/self-modification aliases, current stack mapping, dynamic fault delivery, SEH/unwind, formal Original ABI and caller integration/game execution remain unproved. The adapter's additional reads are explicitly different from direct Original entry. No Source build promotes these limits into runtime proof.

U historical design time is separate:619.275835s through seal,644s through release, plus244.209856s administrative envelope; historical P readiness943.721305s is not W implementation time. W author time starts2026-10-16T09:57:43Z and includes all verification, preservation, qualification, compile/build, command corrections, proof, documentation, ledgers, commit, release and handoff. The final ignored receipt closes that interval. No controlled speedup claim is made.

Root alone may annotate after independent review: retain `scan_tree`, retain Original no_return=false, preserve old prototype/comments, and describe provisional custom EAX/ECX/[entryESP+4]/RET4 inputs plus the ordered frame/WORD schedule. Do not copy public Source noreturn onto Original. The exact Source commit, complete hashes and final receipt are delivered separately for integration.
