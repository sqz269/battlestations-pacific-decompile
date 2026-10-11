# CC12 C1711F direct-target entry evidence

The exact six admitted bytes at `00C1711F–00C17124` are `FF 35 44 E4 09 01`: `PUSH dword ptr [0x0109E444]` in the saved x86-32 interpretation. This establishes an absolute-offset memory source and an implicit stack write. The source cell, source value and seven remaining saved-owner bytes were not read.

The accepted incoming instruction at `00C04E0F` is `E8 0B 23 01 00`. Its signed displacement is +74,507, added to encoded return offset `00C04E14` to reach `00C1711F`. That arithmetic is independently checked from the current accepted body report, without querying another target or reopening an earlier proof archive.

## Admission and evidence

The worktree freezes actual Main `5122687cac595819d4fb517b319e68a6a3b6cf3f`; the recorded ancestry check permits later Main advancement. Eight positive inputs are the accepted C04 body/BODY_PRIMARY document and report pairs, target configuration, and three CLI sources. Fourteen actual Git commit/tree/blob objects establish the frozen files, their original working bytes after LF normalization and exact stage-zero index blobs. Source subtrees are identity-only.

Phase 1 captured count bookends and two complete original typed replies at `00C1711F`: four CLI commands and 16 HTTP responses. All four listing aliases agree on a six-byte InstructionDB unit. The complete saved owner is 13 bytes, `00C1711F–00C1712B`, named `___get_sigabrt`, with `no_return=false` and no thunk. Saved default/effective flow is FALL_THROUGH to `00C17125`, override NONE, empty target arrays and no direct-call companions. These are saved metadata, without semantic authentication of the name or return behavior.

Root independently reviewed the originals and admitted only this first unit. Its gate passed on the first execution; all 206 metadata preimages and seven current Root top-level files were physically copied and pinned. Phase 2 captured paired exact six-byte reads, paired context-zero text at the entry only, the closing typed reply and count bookends: seven CLI commands and 23 HTTP responses. Both byte/text pairs and all three original typed replies match. All 11 commands physically reread all three CLI working sources before and after execution, retaining separate observations; identical raw source payloads are stored by content digest.

The project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, language `x86:LE:32:default`, modification number 98 and function count 64,730. No native, decoder, process, Root gate or Root inspection failure occurred in this packet.

## Independent physical decode

| Field | Value |
| --- | --- |
| Prefixes | None |
| Opcode | `FF`, group `/6`: PUSH r/m32 |
| ModR/M | `35`: mod=`00`, group=`110`, r/m=`101` |
| SIB | None |
| disp32 | Offset 2, four bytes `44 E4 09 01`, unsigned `0109E444` |
| Operand | Four-byte memory read, no base/index, default current DS |
| Immediate / relative target | None |
| Implicit stack | Current SS; current ESP read/write under renewed SS.B=1 |
| EFLAGS | No normal-completion flag modification |
| Numeric next cursor | `00C17125`, unqueried |

The complete manual field record was written before importing or constructing Capstone. Fresh detailed Capstone 5.0.7 agrees on bytes, length, opcode, ModR/M, displacement encoding, operand width/access/default segment, implicit and total register access, and EFLAGS. Its group is `not64bitmode`; that decoder metadata does not authenticate the live execution mode.

## Current-state contract

Physical decoding uses x86-32. Live execution mode, segment descriptors, mappings, permissions and memory values are unqueried.

On normal completion under the decoded operand/address-size interpretation, read the four-byte source through current DS at offset 0109E444 before this PUSH performs its own implicit stack write.

Renew the predicate current SS.B=1. Under that predicate, let S be current ESP and V be the source value just read; the instruction sets ESP to (S-4) modulo 2^32 and writes V through current SS at that new offset. No linear address or flat-segment equality is inferred.

Current DS source and current SS stack storage may alias. The source read precedes this instruction's own write, but the earlier incoming CALL stack write occurred before this source read and could alias it. Neither a value nor a non-alias property is established.

The memory operand is explicit read access. ESP is the implicit register read and write; no other GPR is read by the absolute addressing form. Normal instruction completion leaves arithmetic/status flags unchanged. Fault, interrupt and other exceptional transitions are outside that normal-completion statement.

The previously accepted E8 at C04E0F has encoded return offset C04E14 and numeric target C1711F. Optional two-instruction composition requires a normal uninterrupted direct CALL-to-entry transition and the relevant current segment/stack predicates; no aggregate stack delta is asserted without them.

The dword is not authenticated as an argument, function pointer, service value, library state or provider binding. The saved owner name ___get_sigabrt and no_return=false are metadata, not a recovered prototype, calling convention or proof of return.

Numeric next cursor C17125 and remaining seven saved-owner bytes C17125..C1712B are unqueried and unclassified.

Private finite supplied-ZIP replay validates the complete originals, Root copies, manual reconstruction and stored actual Capstone result, then authenticates the initial fourteen and final six actual Git objects, eight positive input files and exactly two published outputs. The strong audit wrapper admits one supplied ZIP read, one execution of the exact separately pinned helper code object and one exclusive receipt write; all other audited events are denied. Independently generated receipts for the original ZIP and a distinct physical copy remain separate from this public document. No Capstone import/construction, archive-contained code execution or earlier archive traversal occurs under the hook.

Only this document and its report are published. No Source payload, new C++, test, build, probe, Ghidra write, Original executable read, game run or OS/audio runtime experiment is part of this packet. This is bounded physical entry evidence; ABI compatibility, service/provider identity, source-cell value and runtime behavior remain unproved.
