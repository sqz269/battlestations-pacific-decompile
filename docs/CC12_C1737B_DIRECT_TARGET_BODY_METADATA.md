# C1737B: complete two-range body metadata and physical evidence

Root admitted exactly two complete saved owner ranges after independently checking both metadata walks. Paired bytes and context-zero captures, a third complete typed pass, a completed manual decode, and fresh offline Capstone agree on **411 bytes and 141 instructions**. The saved `_raise` name remains unauthenticated metadata.

| Admitted inclusive range | Bytes | Instructions | Last instruction |
| --- | ---: | ---: | --- |
| `00C1737B..00C174E8` | 366 | 123 | `00C174E7`, length 2 |
| `00C174FE..00C1752A` | 45 | 18 | `00C1752A`, length 1 |

The gap `00C174E9..00C174FD` and arithmetic stop cursors `00C174E9` / `00C1752B` were never queried. The first range ends with a physical jump into the second range; capture of the second range was an independently admitted restart, not a walk through the gap.

## Frozen positive evidence and independent gate

The worker worktree was created from actual main `bd1ba5d9cc77dde06c26a728b810caa5c55cbf10`. Eight positive files were frozen: accepted C1737B ENTRY and ENTRY_PRIMARY document/report pairs, `config/target.json`, and the existing `bsp.py`, `ghidra_export.py`, and `ghidra_typed_flow.py` CLI producers. Actual committed, index, and LF-normalized working identities agree. The finite baseline contains 14 actual Git objects: one commit, five trees, and eight blobs.

Only root-tree identity was read for `src` (`dfb2ee135e7b642465d448dcc1616fd92686ad51`) and `include` (`7bb7ed2fc76e16eb09ff91be796c5b264aab9ca9`). Neither Source subtree object nor Source content was read. The original positive-file histories and Git streams remain in the supplied finite proof.

Two independent metadata walks used each just-returned exact `InstructionDB` length before advancing. Every start retained all four listing aliases, complete owner and body ranges, default/effective flow and target arrays, original HTTP wrapper and direct payload, and original CLI streams. The 282 original payloads match pairwise. Root checked all 1,857 manifest members plus three post-manifest streams/results, then issued its gate on its first execution. Before any new byte or text read, the worker verified and copied **1,870 actual Root artifacts**: 1,860 preimages, nine listed top-level artifacts, and the admission manifest.

Phase 1 used 286 Native CLI commands. Phase 2 used 149: four exact-range byte reads, two context-zero batches covering the 141 starts, 141 closing typed reads, and two count checks. All **435 commands** succeeded with separate physical checks of all three CLI producers before and after every invocation. All three complete original typed payloads match at every start. Count checks remained **64730**; program `/battlestationspacific.exe` in project `bsp` remained modification **98**, x86 little-endian 32-bit, base `00400000`, default RAM. No GPR mutation occurred.

Range 1 bytes have SHA-256 `b467d8570c32e0a62e3808c59233a2f77985dd23deb05ebe4008aadb2da38437`; range 2 bytes have SHA-256 `1bf44ee576e20972ca53ccf6a54aff318d2f5c6c28a100b6d50184700f4a4874`. Paired context files each contain 22,439 bytes with SHA-256 `1a7a6e82b6d0bf46f33ece48b23e86b519202b92031ccbe3b2920c603384c5b2`.

## Physical decode and control transfers

All 141 instructions were manually decoded from the admitted bytes, including opcode/prefix fields, ModRM and SIB fields, displacement and immediate offsets/widths/signs, register and memory operands, default segments, relative arithmetic, branch conditions, and conditional stack effects. That complete result was written before the separate fresh Capstone 5.0.7 process imported Capstone. Its actual Python module and DLL are pinned in the result. The physical fields and operands match; actual decoder register/flag/group metadata is retained separately. No Capstone import or execution occurs under the strict replay audit.

There are 24 conditional jumps, eight unconditional jumps, 11 direct calls, two computed calls, and one return. Default and effective saved flow agree at every start; there are no flow overrides or differing target arrays. Saved flags and auto-added target-owner companions do not authenticate behavior or providers.

| Call address | Physical bytes | Encoded target or operand |
| --- | --- | --- |
| `00C17382` | `E87908FFFF` | `00C07C00` |
| `00C173AE` | `E804DEFEFF` | `00C051B7` |
| `00C173D5` | `E811FDFFFF` | `00C170EB` |
| `00C173F2` | `E89487FEFF` | `00BFFB8B` |
| `00C17404` | `E8E6F2FDFF` | `00BF66EF` |
| `00C17438` | `E8A1DBFEFF` | `00C04FDE` |
| `00C17454` | `E87349FEFF` | `00BFBDCC` |
| `00C1745F` | `E8BDA7FFFF` | `00C11C21` |
| `00C174C7` | `E809DBFEFF` | `00C04FD5` |
| `00C174D5` | `E815000000` | `00C174EF` |
| `00C174E3` | `FF55E0` | Current 32-bit operand at `SS:[EBP-0x20]` |
| `00C174FF` | `FF55E0` | Current 32-bit operand at `SS:[EBP-0x20]` |
| `00C17525` | `E81B07FFFF` | `00C07C45` |

Every destination remains opaque. The `00C174D5` rel32 call reaches numeric target **`00C174EF` inside the excluded gap**; that address was not queried. At both computed near calls, `FF 55 E0` selects the current 32-bit value at `SS:[EBP-0x20]`. The processor selects that operand before storing the return address; its destination, mapping, access outcome, and provider are unestablished.

## Current stack, conditional paths, and aliases

The 34 implicit stack operations are evaluated individually under a fresh predicate: actual current SS has 32-bit stack addressing and the instruction completes normally. For current ESP `S`, a 32-bit PUSH or near CALL stores four bytes at `SS:((S-4) mod 2^32)` and sets ESP to that offset. POP or near RET reads the current four-byte stack value and advances ESP by four. A CALL's store is the arithmetic next EIP. No stack-frame, argument, convention, pre-call register, or preserved-register contract is inferred. If any opaque call returns, every register, flag, segment, memory value, ESP, and EBP is again unknown.

The entry's two pushes precede an opaque call at `00C17382`. Subsequent `[EBP+8]` and negative-EBP offsets use the current EBP and SS; their appearances do not prove caller arguments or established locals. Likewise, absolute moffs and ModRM addresses are decoded numeric offsets; no corresponding data cell was read.

The `00C174A2` backedge reloads absolute memory operands, compares a mutable memory counter, forms a low-32-bit signed multiply by 12, and stores through an address with both a base and index. The encoded SIB at `00C174BE` is `11` (scale 1, index EDX, base ECX), displacement `08`. Stores may alias the counter, bounds, stack, or other state; no fixed iteration count, nonaliasing, or termination is proved. Conditional jumps use the current arithmetic flags recorded in the per-instruction evidence.

After the opaque gap call, the `EBX == 8` comparison selects a path with an additional memory PUSH before one of the two computed calls. Those physical sequences do not establish argument roles or a calling convention. Loads and stores using matching offsets on opposite sides of an opaque call do not prove restoration of the same object or value.

At `00C17523`, `XOR EAX,EAX` precedes the opaque call at `00C17525`; `RET` at `00C1752A` therefore **does not prove a zero return value**. Earlier call effects and the actual return destination remain unknown.

## Finite replay, history, and delivery

Original and distinct copied evidence ZIPs contain **4656 members**, **22435798 bytes**, SHA-256 `f1d757a5b5b301b15d3be0a5084b4a5f3edea59eb48aeba02e01c2663d0e4551`. Both first executions passed with exactly one archive read, one execution of the exact pinned pure helper, one exclusive receipt write, and denied events `[]`. Every other audit event is rejected, including events caught by helper code. Replay independently regenerates all manual fields and verifies the finite saved evidence; it opens no repository, Git process, Ghidra provider, Source path, nested ZIP, or Capstone module.

Current-packet Native, helper, strict-replay, and Root gate/inspection failures are zero. One pre-Native worker inspection failed because it named the prior helper `freeze_inputs.py`; the actual filename was `freeze.py`. Its original combined tool output and scope were saved before continuing, with no invented separate stdout/stderr stream. Display truncations are zero. Accepted ENTRY/PRIMARY histories remain retained under their original scopes.

Exactly this document and `reports/cc12_c1737b_direct_target_body_metadata.json` are public outputs. The separate post-commit delivery captures the six new actual Git objects, for 20 baseline-plus-output objects, final unchanged eight positive identities, and both output identities. Root integration/review is separate. This packet adds no Source implementation and claims no build, fixture, probe, ABI, runtime, OS/audio, or game validation.
