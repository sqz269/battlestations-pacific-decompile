# CC12 C04FDE direct-target entry evidence

The sole admitted byte at `00C04FDE` is `56`: `PUSH ESI` in the saved x86-32 interpretation. ESI is the explicit four-byte source; the write uses the current stack segment. All 109 remaining saved-owner bytes are unqueried. The saved name does not establish function-level meaning.

The accepted incoming instruction at `00C17125` is `E8 B4 DE FE FF`. Signed displacement −74,060, added to encoded return offset `00C1712A`, reaches `00C04FDE`. This arithmetic is independently checked from the current accepted C1711F body report and primary review, without opening earlier proof archives or querying another target.

## Admission and provenance

The worktree freezes actual Main `13b1c3a79a956f13458d0ecbef5589a02c8867bf`; the recorded ancestry check permits Main to advance. Eight explicit positive inputs are the current C1711F BODY_METADATA and BODY_METADATA_PRIMARY document/report pairs, target configuration, and three CLI sources. Fourteen actual Git commit/tree/blob objects establish their frozen membership, original working bytes after LF normalization, and exact stage-zero index blobs. Source subtree identities are retained without reading their payloads.

Phase 1 captured count bookends and two original typed responses: four CLI calls and 16 HTTP responses. All four listing aliases agree on an InstructionDB unit of length one. The complete saved owner is one 110-byte range `00C04FDE–00C0504B`, named `BSP_CRT_DecodePointerCurrentState`, with `no_return=false` and no thunk. Default and effective flow were retained independently: FALL_THROUGH to `00C04FDF`, override NONE, empty target arrays and no direct-call companions.

Root admitted exactly the first byte after reviewing the original metadata. Before phase 2, 223 Root artifacts were physically copied and verified: 208 metadata preimages, 14 listed top files, and the admission manifest. Every preimage also equals its worker original. Phase 2 captured paired exact one-byte reads, paired context-zero text, closing typed metadata and count bookends: seven CLI calls and 23 HTTP responses. Both byte/text pairs and all three original typed replies match.

All 11 CLI invocations physically reread each of the three CLI source files before and after execution. Separate observations are retained; identical raw source payloads use content-addressed storage. Project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, language `x86:LE:32:default`, modification number 98 and function count 64,730.

## Physical decode and current-state limits

| Field | Value |
| --- | --- |
| Prefixes | None |
| Opcode | `56` = `50+rd`, register index 6 |
| Operand | Explicit four-byte ESI read |
| ModR/M / SIB / displacement / immediate | Absent |
| Relative transfer | Absent |
| Implicit registers | ESP read and write |
| Stack segment | Current SS |
| EFLAGS | No normal-completion modification |
| Numeric next cursor | `00C04FDF`, unqueried |

The complete manual field artifact was written before importing or constructing Capstone. Fresh detailed Capstone 5.0.7 agrees on byte, length, opcode, empty encoding fields, operand size/access, explicit and implicit registers, EFLAGS and instruction group. Decoder group `not64bitmode` does not authenticate live execution mode.

Renew the predicate current SS.B=1 at this instruction. Under that predicate and normal successful completion, if S is current ESP and V is current ESI, ESP becomes `(S−4) mod 2^32` and four-byte V is written through current SS at that offset. ESI remains unchanged. No linear address, segment equality, stack writability or memory value is established. Exceptional transitions fall outside this normal-completion claim.

Current ESI is not identified as an argument, handle, pointer, function result or callee-saved contract. No earlier opaque-call state is reused. Optional composition with the accepted incoming CALL requires an uninterrupted normal CALL-to-entry transition and the relevant current code and stack predicates for both instructions. The entry alone establishes no stack balance, prototype, calling convention, library identity, provider binding or return behavior.

## Failures and validation boundary

One worker bootstrap failed before producer creation or Native queries: `mkdir` lacked parent creation in the new worktree. Its combined tool output is retained as an explicitly labeled transcription; separate original stdout/stderr files were not captured. Corrected directory creation preceded the pinned collector. One unrelated prior-template source display was truncated; no evidence conclusion relied on its missing portion.

Root's first offline metadata checker failed because its template still asserted the old C1711F raw-response pin (2,147 bytes), while the actual C04FDE response is 2,168 bytes. The original failed producer, preexecution pin, stdout, stderr and execution record were preserved before correcting that literal. Attempt 2 passed. These are separate from the worker's Native, captured-producer and decoder executions, which all passed on their first runs. Root inspection and display counters are zero.

Historical failures in the accepted body and primary reports remain scoped to those positive inputs; they are copied into the machine report without rerunning prior archives. No Source reconstruction, Ghidra mutation, build, new test, probe, Original/game execution or OS/audio experiment occurred.

The private package binds the actual final commit, its single frozen parent, exact two public outputs and their working/index blobs. The fully pinned helper is supplied outside the ZIP. Independent original/copy strict receipts allow exactly one ZIP read, one execution of that precompiled helper, and one exclusive receipt write, with all other audit events denied. The helper uses finite supplied bytes, executes no archived code, imports no decoder under the hook, and opens no previous archive. These are static physical and provenance checks, not ABI or game validation.
