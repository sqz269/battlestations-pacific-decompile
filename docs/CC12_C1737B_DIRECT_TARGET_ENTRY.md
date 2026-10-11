# C1737B direct-target entry: gated immediate push

The accepted C04DE2 body contains `E8 5C 25 01 00` at `00C04E1A`. Its signed rel32 is `75100` (`0x1255C`); adding it to next EIP `00C04E1F`, modulo 2^32, reaches `00C1737B`. This packet establishes only the newly admitted entry instruction at that numeric target.

Root's independent metadata gate admitted exactly `00C1737B..00C1737C`, two bytes. Both byte acquisitions returned **`6A 20`**, and both context-zero acquisitions returned `PUSH 0x20`. The manual decode was completed and preserved before a fresh offline Capstone execution. No instruction at `00C1737D`, remaining body byte, owner-gap byte, runtime value, or provider was acquired.

## Frozen evidence and authorization boundary

The named worker worktree started at actual main `7fad03aeb7b64a6fe2ea67f0f3e955e66d92f3f4`. Its eight positive files are the accepted C04DE2 BODY and PRIMARY doc/report pairs, `config/target.json`, and the three physical CLI producers `tools/bsp.py`, `tools/ghidra_export.py`, and `tools/ghidra_typed_flow.py`. Their actual committed, index, and LF-normalized working identities agree. The finite baseline contains 14 actual Git objects: one commit, five trees, and eight blobs.

Only the root-tree identifiers for `src` (`dfb2ee135e7b642465d448dcc1616fd92686ad51`) and `include` (`7bb7ed2fc76e16eb09ff91be796c5b264aab9ca9`) were used. Their subtree objects and Source contents were not read. The accepted positive files and original Git streams are preserved in the proof package, including their historical incident records.

Before any target byte or instruction-text read, two complete original typed payloads were captured between count checks. Root physically reviewed the finite metadata and issued its gate on the first execution. The proof retains all **172 actual Root artifacts**, including **165 copied preimages**, the pinned gate producer, its preexecution pin, original execution streams, receipt, and manifest. These counts come from the actual files and receipt.

The gate was followed by another count bracket enclosing paired exact-length byte reads, paired context-zero reads, and one closing typed capture. All three complete original typed payloads are byte-identical: 2253 bytes, SHA-256 `ac2480c3a865f51b7132e5155d2a978a4316282370150e99924313c0499b345d`. All 11 Native CLI invocations succeeded; each checked all three physical CLI producers before and after execution. Autostart was disabled. The saved program remained `bsp` / `/battlestationspacific.exe`, x86 little-endian 32-bit, base `00400000`, default RAM, function count **64730**, modification number **98**. No GPR mutation occurred.

## Complete entry metadata

All four listing aliases identify the same `InstructionDB` at `00C1737B`, length 2. Default and effective flow are `FALL_THROUGH`, with call, jump, conditional, computed, and terminal flags false. Override is `NONE`; both fallthrough fields are RAM `00C1737D`, the override flag is false, and both target arrays are empty.

Function-at and function-containing are both `00C1737B`. The one saved owner is named `_raise`, with no-return false, thunk false, recursive-thunk false, and no direct thunk target. Its complete saved body metadata contains two inclusive ranges:

| Range | Address count |
| --- | ---: |
| `00C1737B..00C174E8` | 366 |
| `00C174FE..00C1752A` | 45 |
| Total | 411 |

These ranges and the saved name/flags do not authenticate a library symbol, provider, or behavior. Only the first two bytes were acquired. The remaining **409 owner addresses**, gap `00C174E9..00C174FD`, and next cursor `00C1737D` remain unqueried.

## Physical decode and local stack predicate

| Field | Decoded value |
| --- | --- |
| Prefixes | None |
| Opcode | `6A`, `PUSH imm8` |
| ModRM / SIB / displacement | Absent |
| Encoded immediate | `20`, one byte at offset 1 |
| Unsigned and signed immediate | 32; sign bit 0 |
| Sign extension to operand size | `00000020`, 32 bits |
| Length and arithmetic next EIP | 2; `00C1737D` |

Under the local predicates of 32-bit operand size, **the actual current SS having a 32-bit stack-address size**, and normal completion, let `S` be current ESP. This instruction sets ESP to `(S - 4) mod 2^32` and stores four little-endian bytes `20 00 00 00` at current `SS:((S - 4) mod 2^32)`. Other general registers and arithmetic EFLAGS are unchanged by this instruction. Incoming registers, flags, segments, and memory are otherwise unconstrained. The actual SS descriptor/mapping, access or fault outcome, argument role, calling convention, and relation to any earlier caller stack are unestablished. These predicates must be renewed after an opaque call.

The complete manual result precedes the separate fresh Capstone 5.0.7 execution. That execution decodes exactly one instruction and matches the opcode, prefix, absent addressing fields, immediate offset/width/value, operand width, length, and zero EFLAGS effect. The decoder records pins for its actual Python module and loaded DLL. Capstone is neither imported nor executed inside the strict replay; that replay independently regenerates all manual fields and checks the pinned offline result.

## Finite replay and limits

The original and distinct copied evidence ZIPs each contain **408 members**, size **7500339 bytes**, SHA-256 `afa651e0b303c8c8443d291b27fc91e2f22ac2846c7bf00ba4179a38b7d89008`. Both first executions passed with exactly one archive read, one exclusive receipt write, and one execution of the pinned pure helper; denied events are `[]`. The audit rejects every other event. It opens no repository, Git, Ghidra, Native provider, Source path, nested ZIP, or Capstone module during replay.

Current-packet Native, helper, Root-gate, inspection, display-truncation, and strict-replay failures are all zero. Earlier incidents in the accepted positive evidence remain retained under their original scope. The exact payloads, pins, manual and decoder results, original process streams, and gate are supplied in the finite evidence artifacts; the companion JSON records the public receipt and bounded claims.

The final delivery captures only this document and `reports/cc12_c1737b_direct_target_entry.json`, their actual new commit/tree/blob objects, and the final unchanged eight positive identities. Its copied-ZIP replay is a separate post-commit receipt. This packet adds no Source implementation and claims no build, fixture, ABI, runtime, or game validation. Root integration/review is separate from this worker result.
