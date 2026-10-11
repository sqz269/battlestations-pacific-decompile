# BFFB8B direct-target entry evidence

The Root-admitted five-byte unit at `00BFFB8B` is `E8 27 56 00 00`: a near relative CALL to numeric offset `00C051B7`, with encoded return offset `00BFFB90`. This packet establishes that single physical instruction. Neither the return site nor the destination body/provider/prototype/cell was queried.

The positive incoming edge is the accepted C26B57 body instruction at `00C26B80`, bytes `E8 06 90 FD FF`: signed rel32 `-159738` from next offset `00C26B85` yields `00BFFB8B`. The accepted body and independent PRIMARY reports are frozen Git inputs; no prior archive is opened.

Frozen actual Main at worktree creation is `7fad03aeb7b64a6fe2ea67f0f3e955e66d92f3f4`, tree `24f66641b241f8c41ccab600714402dd004710db`. The finite initial object set contains one commit, the root tree, four immediate directory trees and eight positive input blobs. All eight actual working files, LF bytes and stage-0 index objects match those blobs. Main may advance; the recorded ancestor check is the authority. `src` and `include` are compared by subtree identity only, without reading Source bodies or making a Source absence census.

Two independently captured original typed payloads agree, and the closing capture is identical. All four aliases (`instruction_at`, `instruction_containing`, `code_unit_at`, `code_unit_containing`) identify the exact five-byte `InstructionDB`, kind `instruction`, defined null. Function-at and containing-function are both `00BFFB8B`. The saved owner has a complete one-range, nineteen-byte body `00BFFB8B–00BFFB9D`; the other fourteen bytes remain unqueried.

The saved name `LIBCRT_unmatched_00bffb8b`, no-return false, nonthunk flag, recursive-thunk-target false and null direct thunk target are recorded metadata. They do not authenticate library/service/type/convention behavior. The saved flow override is NONE, fallthrough override false. Default and effective flows, targets and fallthroughs are retained and compared independently with the physical decode: unconditional CALL, target `00C051B7`, encoded return/fallthrough `00BFFB90`.

| Encoding field | Manual result |
|---|---|
| Prefixes | None |
| Opcode | E8, near CALL rel32 |
| ModRM / SIB / memory displacement | Absent |
| Encoded immediate | `27 56 00 00`, little-endian signed rel32 `22055` |
| Transfer | `(00BFFB8B + 5 + 22055) mod 2^32 = 00C051B7` |
| Implicit stack value | Return offset `00BFFB90`, four bytes |
| Default stack segment | Actual current SS |
| EFLAGS | Unchanged by the successfully completed CALL instruction itself |

The selected instruction decode has 32-bit operand/address defaults. The implicit stack formula requires a separate renewed predicate that the actual current SS descriptor has B=1 at this CALL. Under that predicate and successful normal instruction completion, decrement actual current ESP by four modulo 2^32, write `00BFFB90` through actual current SS at the new offset and transfer EIP to `00C051B7`. Explicit code address size does not establish SS.B, and the incoming CALL's stack predicate is not inherited automatically.

The CALL instruction itself changes ESP and EIP, and leaves EFLAGS and other ordinary registers unchanged; the target execution is opaque. Any later return to `00BFFB90` requires all actual current registers, flags, SS/DS descriptors, stack contents and memory to be considered unknown afresh. SS/DS aliases and stack-write aliases are possible; no original return-slot integrity, preserved register, argument type, provider service, calling convention, completed return or nonreturn behavior follows.

The complete manual opcode/operand/register/flags/transfer fields were saved and pinned before a fresh actual Capstone 5.0.7 detailed CS_ARCH_X86/CS_MODE_32 child. Every field matched. Capstone executes outside the pure replay. The byte pair and context0 text pair agree, with no other context instruction.

Eleven read-only Native CLI commands retain original argv, preexecution records and original stdout/stderr. All use autostart off with the existing `bsp.gpr` and `/battlestationspacific.exe` verified by the frozen CLI. Four count brackets remain 64730 functions / modification 98. Each command has separate before/after physical reads of all three frozen CLI files: 22 observations and 66 reads, retaining exact source bytes and timestamps. All 63 Root artifacts, including 55 original preimages and the Root display incident, are copied and pinned.

The pure checker reads exactly one pinned ZIP, executes exactly its precompiled code object and exclusively writes one receipt; every other audit event is denied and `denied == []` is asserted. After the retained checker failure described below, original and actual distinct-copy runs in the fresh core_retry namespace passed with byte-identical receipts. The final seal adds the actual final Git commit/tree/blob objects, all ten working/LF/index file identities and exact two-output tree differences, for twenty finite unique Git objects across both snapshots. Delivery original/copy receipts are retained for independent review.

Current Native, capture-producer and decoder failures are zero. The first strict core replay failed because its Git command validator accepted bare object IDs but did not resolve actual Root commit:path blob references. Its exact helper, archive, receipt and original streams are retained; the corrected validator resolves only approved frozen input paths and was rerun in a fresh namespace. Two rejected private recovery lookup requests are recorded as recovery-only tool-schema incidents, with no invented process status or original streams. Two successful conversation-only template inspections exceeded display caps; their commands, exact budgets/counts and bounded follow-ups are retained. Root's successful metadata inspection had one separate display truncation, and its actual gate passed first execution. All nested prior BODY/PRIMARY failure histories remain verbatim in the report.

Only this document and `reports/cc12_bffb8b_direct_target_entry.json` are committed. No Source change, new test, build, Original/game/audio/runtime execution or GPR mutation is part of this packet. This is bounded physical evidence, not ABI compatibility or game validation.
