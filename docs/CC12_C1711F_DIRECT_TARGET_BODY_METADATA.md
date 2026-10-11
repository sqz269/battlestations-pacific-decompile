# CC12 complete C1711F saved-body evidence

The complete saved range `00C1711F–00C1712B` contains four instructions and 13 bytes: an absolute memory PUSH, direct near CALL, POP ECX and near RET. Paired original bytes and context-zero text agree, and a manual decoder completed before Capstone import agrees with fresh detailed Capstone 5.0.7 on every field and access effect.

| Address | Bytes | Instruction |
| --- | --- | --- |
| `00C1711F` | `FF 35 44 E4 09 01` | `PUSH dword ptr [0x0109E444]` |
| `00C17125` | `E8 B4 DE FE FF` | `CALL 0x00C04FDE` |
| `00C1712A` | `59` | `POP ECX` |
| `00C1712B` | `C3` | `RET` |

The call displacement is -74,060 from its encoded return offset `00C1712A`; this gives numeric destination `00C04FDE`. Neither that destination nor source cell `0109E444` was queried. The RET's current stack-supplied destination is unknown. No ordinary fallthrough or classification is assigned to numeric end cursor `00C1712C`.

## Admission and finite evidence

The branch freezes actual Main `f98350bc7ad56c1d00bc30b526b71e47a80c7bf7`; later advancement is permitted by the captured ancestry relation. The eight positive files are the accepted ENTRY/ENTRY_PRIMARY document and report pairs, target configuration and three CLI sources. Fourteen actual initial Git objects authenticate frozen tree/blob membership, original working files after LF normalization and exact stage-zero index records. Source subtrees are identity-only. The accepted incoming E8 at `C04E0F` reaches `C1711F` from encoded return offset `C04E14` with signed displacement +74,507; its accepted physical evidence is carried forward without opening an earlier proof archive.

Two independently restarted metadata-only walks advanced solely by each just-captured complete unit length: 6, 5, 1, 1. They stopped after 13 bytes without querying `C1712C`. All four aliases agree at every start and both original raw replies match at each address. The owner remains the complete saved 13-byte range, name `___get_sigabrt`, `no_return=false`, non-thunk. Saved flows are two FALL_THROUGH, one UNCONDITIONAL_CALL and one TERMINATOR, all override NONE. Saved labels and automatic target companion data are not service, prototype or provider evidence.

Root's first actual gate child passed after reviewing the originals and admitted only these 13 bytes and four proven starts. All 290 metadata preimages and eight Root top-level artifacts were physically copied and pinned. Phase 2 read paired exact 13-byte blocks, paired context-zero text only at those starts, closing typed metadata at all four starts and count bookends. Across both phases, 19 Native CLI commands and 78 original HTTP responses passed, with all 12 typed replies agreeing per start. The three CLI working sources were physically reread before and after every command; separate observations retain each read, while identical raw payloads are stored once by digest.

Project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, language `x86:LE:32:default`, modification number 98 and function count 64,730. Prefixes, FF /6 ModR/M, unsigned absolute displacement, signed E8 displacement/target/return offset, POP opcode register, C3 form, operand widths/access, segments, implicit/total register access and EFLAGS all agree with the fresh actual detailed decoder result.

## Current-state contract

Physical interpretation is x86-32. Current execution mode, segment descriptors, mappings, permissions and memory values are unqueried. All stated normal effects require normal completion and applicable architectural checks.

Renew current SS.B=1 separately at PUSH, near CALL, POP and near RET. No predicate or register, flag, memory or segment value is propagated through the opaque callee.

At C1711F read the dword through current DS at offset 0109E444 before this PUSH writes it through current SS at current ESP-4, modulo 2^32 under the current SS.B=1 predicate. The cell was not read by this analysis.

DS and SS bases are not equated. The current source and stack destinations may alias. The incoming CALL at C04E0F wrote its return offset before this source read if execution reached the entry normally; that earlier write may also alias the source cell.

E8 at C17125 has signed displacement -74060 from C1712A, giving numeric target C04FDE and encoded return offset C1712A. Under the current stack predicate and normal completion it pushes that return offset through current SS. C04FDE was not queried and remains opaque.

Whether the callee returns is unproved. If control next reaches C1712A, treat all current registers, flags, memory and segments as unknown. POP ECX then reads the dword through current SS at its then-current ESP, writes ECX and increments current ESP by four under renewed SS.B=1.

C3 at C1712B is an operand-size-32 near RET with no encoded immediate adjustment. With renewed current SS.B=1 and normal completion, its target comes from the dword at its then-current SS:ESP and the stack pointer advances by four. That target is unknown and need not equal C04E14. C1712C is only a numeric end cursor, not an ordinary RET fallthrough; it was never queried or classified.

The four instructions have no normal-completion arithmetic/status EFLAGS modification. Opaque callee effects, exceptions and asynchronous events are outside that statement. Capstone register-access metadata is recorded separately from the architectural control-transfer description.

The bytes do not establish a prototype, calling convention, argument identity/count, balanced stack, caller cleanup, service/library/provider binding, original caller return address or result value. The saved ___get_sigabrt name, no_return=false and automatic callee companion are metadata only.

## Retained failures and verification limits

The worker's initial lease claim used an unsupported colon separator and failed in the range parser before any Native payload query. The corrected hyphen range claim succeeded. Its exact command and observed error excerpt are retained as an explicitly labeled tool-output transcription; no separate original process stdout/stderr file exists. The combined shell reported zero only because it continued to a subsequent successful Git command, not because the first lease claim succeeded.

Root separately had one bootstrap copy failure before a preexecution pin or gate child ran: the destination primary directory was absent. That history records the exact command, observed error and original combined-tool-only stream provenance. Root created only its leased directory and then the first actual gate child passed. These two setup failures remain separate from zero Native capture, decoder, captured-producer, actual Root gate and Root inspection failures.

The accepted prior ENTRY_PRIMARY also retains one post-strict receipt-comparison correction: its first actual gate, decoder, final review and strict replay passed; only a subsequent comparison initially removed the wrong path field. The corrected comparison permits exactly `helper.path` and `supplied_zip.path` differences, with pins/audit counts/results otherwise equal. That accepted history remains in the positive input files; no old private archive was reopened.

Private finite supplied-ZIP replay authenticates the original HTTP/CLI evidence, Root copies, retained setup histories, manual records and stored actual Capstone result, plus the initial fourteen and final six actual Git objects and exactly two published outputs. The pinned pure helper runs under a guard permitting one supplied ZIP read, one execution of its exact code object and one exclusive receipt write, denying every other audited event and asserting an empty denial list. Original and distinct physical-copy receipts remain separate from this public document. No Capstone import/construction or archive-contained code execution occurs under that guard.

Only this document and its report are published. No Source payload, new C++, test, build, probe, Ghidra write, Original executable read, game run or OS/audio runtime experiment is part of this packet. ABI compatibility, return-value meaning and runtime behavior remain unproved.
