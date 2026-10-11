# C26B57 complete saved body: bounded physical evidence

The complete saved range C26B57..C26B9C contains 70 bytes and 28 whole instructions. Two independent typed length walks, paired exact bytes, paired context-zero listings, and closing typed metadata agree. Fully manual x86 fields match fresh detailed Capstone 5.0.7 output. The saved owner label `__set_error_mode` remains metadata; the packet does not authenticate a library function, global variable, argument type, provider or calling convention.

## Capture and scope

The worktree froze actual Main commit `c10e6a0097f669defca49449adfa5f95ee608b27`, tree `55ea72f0c44e9ed08754b861e7dc785f5abb5479`. Main advanced concurrently, and the retained actual ancestry command succeeded. The eight positive Git inputs are the accepted ENTRY doc/report, its PRIMARY doc/report, target configuration and three CLI source files. Each working file is LF-only and equals its stage-zero index blob; fourteen actual objects and original Git streams establish this finite input set. Source subtree identities are recorded without reading their bodies or making a source-absence census. No prior packet ZIP is an input.

Phase 1 made two independent length-driven typed walks over the saved single 70-byte owner range. Each walk captured and validated a whole saved instruction before advancing by that capture's length. Both covered 28 starts with no gaps or overlaps and stopped at C26B9D without querying it. Complete original HTTP response text and base64, digests, requested GET URLs, all four InstructionDB aliases, complete owner ranges, override, default/effective flows, fallthroughs and target arrays are retained. Default/effective target arrays were kept independently rather than forced equal. Root's first actual gate execution accepted the full range before new byte or text reads.

Phase 2 read exactly the 70 bytes twice, captured context-zero text only at the 28 proven starts twice, and captured all 28 typed units again. Every closing original response matches both initial responses byte-for-byte. The saved owner remains a non-thunk with `no_return=false`; these are saved flags, not authenticated runtime behavior. The body range ends in the saved unit at C26B9C. No C26B9D, callee destination, cell, provider or prototype was queried.

All 94 Native commands passed with empty error streams. Six count checkpoints stayed at 64730 and typed modification numbers stayed at 98 in `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. Every command has separate before/after physical reads of all three CLI source files: 188 timestamped observation records and 564 physical source reads. Content-addressed raw bytes, per-command pre-execution invocation records, actual command records and original streams are retained. The proof checks that each before read finished before its command and each after read began after command completion. This is retained producer observation evidence, not a separate wire trace.

All 459 actual Root gate artifacts were copied, including 448 original worker preimages, complete gate evidence and the disclosed failure/inspection records.

## Physical units

| Address | Bytes | Instruction | Length |
|---|---|---|---:|
| 00C26B57 | `8b4c2404` | MOV | 4 |
| 00C26B5B | `56` | PUSH | 1 |
| 00C26B5C | `33f6` | XOR | 2 |
| 00C26B5E | `3bce` | CMP | 2 |
| 00C26B60 | `7c1e` | JL | 2 |
| 00C26B62 | `83f902` | CMP | 3 |
| 00C26B65 | `7e0c` | JLE | 2 |
| 00C26B67 | `83f903` | CMP | 3 |
| 00C26B6A | `7514` | JNE | 2 |
| 00C26B6C | `a1d0dd0901` | MOV | 5 |
| 00C26B71 | `5e` | POP | 1 |
| 00C26B72 | `c3` | RET | 1 |
| 00C26B73 | `a1d0dd0901` | MOV | 5 |
| 00C26B78 | `890dd0dd0901` | MOV | 6 |
| 00C26B7E | `5e` | POP | 1 |
| 00C26B7F | `c3` | RET | 1 |
| 00C26B80 | `e80690fdff` | CALL | 5 |
| 00C26B85 | `56` | PUSH | 1 |
| 00C26B86 | `56` | PUSH | 1 |
| 00C26B87 | `56` | PUSH | 1 |
| 00C26B88 | `56` | PUSH | 1 |
| 00C26B89 | `56` | PUSH | 1 |
| 00C26B8A | `c70016000000` | MOV | 6 |
| 00C26B90 | `e85afbfcff` | CALL | 5 |
| 00C26B95 | `83c414` | ADD | 3 |
| 00C26B98 | `83c8ff` | OR | 3 |
| 00C26B9B | `5e` | POP | 1 |
| 00C26B9C | `c3` | RET | 1 |

The machine-readable report records every opcode, absent prefix, operand/address size, ModR/M split, SIB split, displacement, immediate, default segment, operand access, register access, flag effect and transfer. Operand and address sizes are 32 bits; implicit stack address size separately depends on the actual SS descriptor.

The entry `8B 4C 24 04` reads current SS at explicit offset `(ESP+4) modulo 2^32` into ECX. ModR/M 4C is mod01/reg001/rm100; SIB 24 is scale1/no index/ESP base; disp8 is +4. No immediate or segment override appears. This explicit 32-bit ESP address does not establish current SS.B for implicit stack operations.

PUSH/POP ESI use opcodes 56/5E. XOR `33 F6` has mod11/reg110/rm110 and makes ESI zero locally. CMP `3B CE` compares ECX to ESI; the `83 /7` instructions compare ECX to sign-extended imm8 values 2 and 3. CMP writes no operand and sets OF/SF/ZF/AF/PF/CF from subtraction. XOR clears CF/OF, derives SF/ZF/PF from its zero result, and leaves AF undefined.

JL at C26B60 tests `SF != OF` and has signed rel8 +30 to C26B80. JLE at C26B65 tests `ZF == 1 or SF != OF` and has rel8 +12 to C26B73. JNE/JNZ at C26B6A tests `ZF == 0` and has rel8 +20 to C26B80. These branches do not modify flags. Along uninterrupted successful local instructions, the first edge selects signed ECX below zero, the second selects 0..2 after the nonnegative edge, and equality with 3 selects C26B6C after the >2 edge.

The two A1 instructions load EAX through current DS at moffs32 offset 0109DDD0. `89 0D D0 DD 09 01` stores current ECX through DS at the same encoded offset: mod00/reg001/rm101 selects absolute disp32 without a SIB. The 0..2 route reads then stores; the equality-to-3 route only reads, before their respective POP/RET pairs. The cell itself was not read, and its role, value and type are unproved. Unknown DS/SS bases mean potential aliases with stack storage cannot be excluded.

## Opaque calls and current state

The rel32 CALL at C26B80 numerically targets BFFB8B and encodes return site C26B85. The rel32 CALL at C26B90 numerically targets BF66EF and encodes return site C26B95. Both numeric calculations are in the report; neither destination was queried or assigned a service.

Renew the actual current SS.B=1 predicate at each implicit PUSH, POP, CALL and RET before applying ESP +/-4 formulas. A successful CALL under that predicate decrements current ESP by four, stores its return dword through current SS, and enters its numeric target. This describes the CALL instruction itself, not callee behavior. After either opaque callee, registers including ESI/EBP/ESP, flags, segment descriptors, memory and provider state must be treated as current unknown state.

The five PUSH ESI instructions at C26B85..C26B89 occur after the first opaque call. Earlier `XOR ESI,ESI` does not establish that these pushes contain zero or that ESI was preserved. Each pushes actual current ESI using its current stack predicate. `C7 00 16 00 00 00` then writes dword22 through actual current DS:[EAX]: mod00 with opcode extension /0 and EAX base, imm32=22, no displacement. EAX is not authenticated as an errno pointer, returned provider or valid address. This store can alias other memory, including pushed values.

After the second opaque call, `83 C4 14` adds sign-extended imm8 +20 to actual current ESP modulo 2^32 and sets arithmetic flags. This does not prove whole-function cleanup or a calling convention. `83 C8 FF` sign-extends imm8 FF to FFFFFFFF and ORs it into EAX, making EAX=FFFFFFFF with CF=OF=0, SF=PF=1, ZF=0 and AF undefined on successful completion. The subsequent local POP ESI and near RET do not modify that EAX value or those flags on normal completion; their actual stack accesses and return target still depend on current state.

C3 is a near RET without immediate adjustment. Under renewed current SS.B=1 and normal completion, it reads its destination dword from current SS:ESP and increases ESP by four. No earlier caller destination or return-slot integrity is established. Likewise, an earlier PUSH/later POP pair alone is insufficient to establish ESI preservation when opaque calls, changed descriptors/stack state or aliasing stores may intervene. No completed whole-function path is claimed.

## Proof and retained incidents

Actual detailed Capstone 5.0.7 ran outside the pure replay and matched all 28 manually derived records, including operands, register access, prefixes, ModR/M/SIB/displacements/immediates, groups and flag descriptors. The replay uses the retained decoder output as inert evidence. Saved flow is separately compared against physical transfer calculations; all current comparisons agree, with no saved overrides or default/effective target-array differences. This agreement does not turn saved names into recovered symbols.

The pure guard allows exactly one archive read, execution of the exact reviewed precompiled code object and one exclusive receipt write. Every other audit event is denied and any recorded denial fails the run. First executions against the original core archive and an actual distinct byte-identical copy passed with matching receipts and empty denial lists. Final delivery repeats this closure with actual committed output objects, exact two-path diff and all ten LF/working/index checks. Final commit/archive identities live in the external handoff to avoid circular hashes.

Worker capture, Native, decoder and core replay executions had no failures. Root's first actual gate child passed, but its earlier shell bootstrap failed with exit1 and empty combined tool output because of a nested here-string terminator; Root's record states that only the script existed then, without a prepin or child execution. The exact failed command transcription is retained, without claiming separate original failed stdout/stderr files. Root also retained one conversation-only schema display truncation: 1917 tokens at a 1600-token cap, omitting317; bounded followups and complete-file validation followed. Both `Root_bootstrap_failure_history.json` and `Root_inspection_display_incident.json` are included in the proof and report. Prior ENTRY inspection history remains explicitly carried forward.

No Source implementation, build, test, probe, Original/game/audio/runtime execution or Ghidra write occurred. The packet establishes bounded metadata and physical instruction behavior; it does not establish ABI compatibility or game equivalence.

Machine-readable evidence: [cc12_crt_banner_first_direct_target_body_metadata.json](../reports/cc12_crt_banner_first_direct_target_body_metadata.json).
