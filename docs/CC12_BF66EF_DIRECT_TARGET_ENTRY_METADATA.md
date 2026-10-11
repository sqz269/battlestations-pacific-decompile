# CC12 BF66EF direct-target entry evidence

The sole admitted byte at `00BF66EF` is `55`: `PUSH EBP` under x86-32 interpretation with no prefix and a 32-bit operand. At this instruction, current `SS.B=1` is a separate predicate for `ESP := (ESP-4) mod 2^32` and a four-byte current EBP store through current SS at the resulting offset on normal successful completion. EBP and EFLAGS are unchanged. The remaining 35 saved-owner bytes remain unqueried.

The accepted incoming instruction at `00BFBB7C` is `E8 6E AB FF FF`. Its signed rel32 is -21,650; adding it to next/encoded return offset `00BFBB81` gives numeric target `00BF66EF`. The binding retains both current BFBB61 BODY and independent BODY_PRIMARY physical records, Root's independent manual and first actual Capstone records, and both full paired saved default/effective flow arrays. Each saved array has only target BF66EF and fallthrough BFBB81; the recorded direct-call companion is identity metadata. No parent or other-target query was repeated.

## Admission and original evidence

Actual Main is frozen at `6b71a6f02bea608ad69b973ebb80af06f62dc341`. The eight positive inputs are the accepted BFBB61 BODY and BODY_PRIMARY document/report pairs, target configuration and three CLI files. Fourteen actual Git commit/tree/blob objects establish frozen membership, LF-normalized working bytes and stage-zero index blobs. Source subtree identities are retained without opening their payloads or conducting an absence census.

Phase 1 made four CLI invocations and preserved 16 original HTTP replies: independent count bookends and two typed replies at BF66EF. All four first-unit aliases identify the same one-byte InstructionDB unit. The complete containing owner is one 36-byte range BF66EF..BF6712, named `LIBCRT_unmatched_00bf66ef`, with `no_return=false` and no thunk. Saved default/effective flow fields independently report FALL_THROUGH to BF66F0, override NONE, empty target arrays and no companions at this instruction.

Root's metadata gate passed on its first execution. Before any phase-two query, all 219 exact Root artifacts were copied and independently read back: 208 metadata preimages, ten listed top files and the admission control. The preimages also equal their worker originals. The copied driver prepin retains its actual direct filename/size/hash/before-first-execution schema. The copy chronology is preserved.

Phase 2 made seven CLI invocations and preserved 23 original HTTP replies: paired exact one-byte reads, paired context-zero reads at BF66EF, closing typed metadata and count bookends. Both byte/text pairs and all three original typed replies are equal. Every one of the total 11 Native invocations physically reread the three CLI files before and after the call. Project/program remained `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; saved language is `x86:LE:32:default`, modification number 98 and function count 64,730.

## Physical decode and current-state contract

| Field | Value |
| --- | --- |
| Prefix / opcode | None / `55`, `50+rd` PUSH register, rd=5 EBP |
| ModR/M, SIB, displacement, immediate | All absent |
| Operand | Four-byte EBP read |
| Implicit register access | ESP read/write in the x86-32 decoder model |
| Address / operand size | 32 / 32 bits in that decode model |
| EFLAGS | No effects |
| Implicit stack segment | Current SS; current SS.B independently controls the stack address size |
| Numeric next cursor | BF66F0, unqueried |

The complete manual record was physically written before the first fresh detailed Capstone 5.0.7 import/construction. That first comparison retained one difference: the manual group name `not64` versus actual `not64bitmode`. The decoder exited with an assertion after preserving the original manual, actual result, comparison, stdout, stderr and command. A separate offline review accepts only that group vocabulary mapping and independently checks every physical field, operand/access, encoding offset/size, explicit/implicit register access and EFLAGS against the unchanged first actual artifact. Capstone executed once and was not rerun. The first failure remains a failure.

Current SS.B=1 must be required anew at this PUSH; it is not inherited from an incoming CALL or an earlier opaque call. If current SS.B=0, the stack address calculation instead uses SP. No actual descriptor, stack/register value, linear address, flat-segment equality or writability has been established. No prologue, frame, argument/prototype, service identity, calling convention, callee-saved register contract or aggregate stack balance follows from this byte. Faults, interrupts and exceptional transitions lie outside the normal-completion statement.

## Histories and validation boundary

Current worker counts are zero Native failures, one decoder failure, one captured-producer failure referring to that same decoder process, zero inspection-command failures and one display truncation. The worker display incident is a saved-source/template inspection with no original complete raw stdout file or prepin; only the missing lines were subsequently viewed. Root has zero gate failures and separate current counts of one pre-gate inspection failure and one display truncation, including their unavailable original stream/prepin limits. Distinct accepted worker, independent PRIMARY, gate-phase, later-display, publication-revision and earlier histories remain scoped and preserved in the report.

The private proof binds an actual final commit with one frozen parent, exactly two public outputs, 20 actual Git objects and ten working/index/LF file checks. The entire helper is supplied outside the archive. Its strict audit permits one supplied ZIP read, one execution of that exact precompiled helper and one exclusive receipt write; every other audit event is denied. It imports no decoder under the hook and executes no archived code. Separate original and copy paths preserve identical archive content.

No Source reconstruction or absence census, Ghidra mutation, build, new test/probe, Original/game/OS/audio execution, provider/cell/prototype query or earlier-archive traversal occurred. BF66F0, all 35 remaining owner bytes through BF6712 and end-exclusive BF6713 remain unqueried. This is static entry and provenance evidence, without ABI or game validation.
