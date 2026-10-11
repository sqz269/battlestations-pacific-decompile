# Cleanup direct-call target: exact entry at C11B31

The accepted complete cleanup caller at `C11C18` numerically calls `C11B31`. Root gated the original paired typed metadata before admitting one exact entry byte. That byte is `55`, independently decoded and confirmed by actual detailed Capstone 5.0.7 as `PUSH EBP`. Its context-zero saved text and closing metadata agree. No behavior beyond this one instruction is established; Root final review remains pending.

Frozen base: `022395d820ddaf58360b6c551b7d7657417e6a2b`. The saved owner name `__unlock` is metadata only. Its complete recorded range is `C11B31..C11B45` (21 bytes); the remaining 20 bytes are unread. No prototype was queried.

## Admission and physical evidence

The two initial complete direct HTTP payloads and the closing payload agree at epoch 98, function count 64730, project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`. All four exact listing aliases identify the same one-byte InstructionDB at `C11B31`. Default and effective flow are FALL_THROUGH to `C11B32`, overrides NONE, and target arrays empty. Function-at and function-containing identify the same complete saved owner. Automatically returned call-target metadata remains opaque.

All ten existing-CLI commands passed: four count bookends, three typed-flow responses, two exact one-byte reads and one context-zero text request. The initial four commands preceded Root's gate; the remaining six followed its explicit admission. `C11B32` itself was never queried. Original direct-response base64/digests, command streams, helpers, revisions and failed-producer preimages are retained. The Root producer and receipt, all 33 Root metadata preimages and all 23 Root frozen positive Git preimages are copied unchanged under distinct paths, totaling 59 copied artifacts.

`55` is the `50+r` opcode class with r=5 (EBP). It has no prefix, ModRM, displacement or immediate. Under 32-bit operand decoding it pushes four bytes. Manual fields and actual detailed Capstone agree both for this one-byte entry and for all four instructions of the finite accepted nine-byte caller `6A0AE812FFFFFF59C3`. Caller bytes come from the frozen accepted BODY and BODY_PRIMARY report pair; no new Native caller query was made.

## Conditional instruction-time contract

Every ESP formula here requires an actual 32-bit SS stack-address size at that instruction, 32-bit code/operand decoding and normal completion. Live SS descriptors were not queried. Arithmetic is modulo 2^32, and memory uses the segment, registers and contents current at each instruction. Invalid accesses, faults and non-return do not count as successful completion.

For the target entry, let P and B be ESP and EBP immediately before `C11B31`. Under those conditions, `PUSH EBP` sets ESP=P-4 and writes B through current SS:[P-4]. EBP and the other general registers remain unchanged over this one instruction, and flags are not written. Physical fallthrough is `C11B32`; its instruction bytes remain unknown. This push alone does not establish a new EBP frame, a saved caller identity, a parameter contract or function-wide preservation.

The following formulas for the positive caller retain the same actual stack-size conditions at each instruction, including after any opaque return:

| Address | Bytes | Instruction | Qualified physical effect |
| --- | --- | --- | --- |
| 00C11C18 | 6A0A | PUSH 10 | Under the actual 32-bit SS stack-size condition, let S be current ESP. Sign-extend imm8 0A to 0000000A; ESP := S-4; Write32(current SS, S-4, 10). |
| 00C11C1A | E812FFFFFF | CALL 00C11B31 | Signed rel32 is -238; physical next address 00C11C1F plus -238 gives 00C11B31. Under actual 32-bit SS stack size and the normal preceding PUSH, ESP := S-8; Write32(current SS, S-8, 00C11C1F). |
| 00C11C1F | 59 | POP ECX | Only if the opaque callee returns here and actual 32-bit SS stack size holds anew: let Q be the unknown returned ESP; ECX := current Read32(SS, Q); ESP := Q+4. |
| 00C11C20 | C3 | RET | If this instruction normally follows that POP and actual 32-bit SS stack size holds: target := current Read32(SS, Q+4); ESP := Q+8. The target remains unknown. |

The relative call has no ModRM or target-cell dereference: `00C11C1F + (-238) = 00C11B31`. Only if the caller PUSH, CALL and known target PUSH execute consecutively in the modeled state with actual 32-bit SS stack size and no intervening mutation can they be composed: target entry P=S-8, then the current target EBP is written through SS:[S-12] and ESP becomes S-12. These are instruction-time writes; underlying storage aliases or later opaque operations can change their contents.

After that one target PUSH, its remaining body is opaque. If execution eventually returns at `C11C1F`, every returned general register, ESP, EBP, flag, segment and memory value is independently unknown. Q has no established relation to S, and the actual stack-size conditions must hold again for the POP and RET formulas. The reads are not tied to previously pushed literal 10 or a particular Original return address. No argument cleanup, stack balance, lock number or unlock effect is inferred.

## Finite copied-archive proof

Exactly eight positive files are frozen: accepted LOCK_INITIALIZER_CLEANUP_BODY and BODY_PRIMARY doc/report pairs, `config/target.json`, and the three existing CLI tools. Six actual commit/tree objects and selected blob bytes are verified against physical copies. Root's separate 23 frozen Git copies are also verified against those actual objects. Source trees are recorded by identity only; no Source contents, census or old proof archive was read, and equality with current Main is not claimed.

The core ZIP and distinct physical copy each contain 152 members and are 2,114,515 bytes, SHA-256 `34435879348fec09a3f4635534a044b1dabe2a9d860117afd02cfdcbe71aca1a`. Both strict supplied-ZIP replays passed on their first attempt. Each audit reports one archive read, one pinned helper execution and one exclusive receipt write, with zero denials. The JSON report records complete producer, artifact and receipt pins. Post-commit delivery is a separate finite actual-Git proof of exactly the two owned output paths, unchanged positive files and working/index/committed byte equality.

## Retained failure and limits

There was one offline validation failure: a broad body-size text replacement also changed the HTTP 200 check to HTTP 210. It raised before manifest creation, with no Native query or GPR operation. The failed preparer, validator and manifest helper are preserved as full physical preimages, along with the exact displayed original exec-tool traceback and the repair producer. That offline invocation supplied combined tool output rather than separate raw stdout/stderr files; the failure record states this limit. Restricting the replacement to the intended arithmetic assertion restored HTTP 200, and one corrected offline retry passed. Native captures, Root gate execution and both guarded core replays had zero failures; the packet is not described as error-free.

No `_unlock` semantic conclusion, Original caller-frame identity, ABI compatibility, binary replacement claim, Source implementation, build, test, native probe, Original/game execution, OS/audio behavior or GPR mutation is credited. Root must independently review this entry before any new boundary is admitted.
