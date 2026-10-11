# CC12 BF66EF bounded body evidence

The admitted 36 bytes at `00BF66EF..00BF6712` decode to 14 x86-32 instructions. The physical body has two direct near CALLs, a zero-condition branch, an indirect `JMP EAX` and a final direct `E9` JMP. Ghidra's saved `CALL_RETURN` override at BF670E is retained separately from that encoded JMP. It adds no physical CALL return-slot push.

## Admission and complete original evidence

Actual Main is frozen at `0fae7b7fa6a6fe2d7d5d5b9edfe363700a9d5fee`. Eight positive inputs are the current BF66EF ENTRY and independent PRIMARY document/report pairs, target configuration and three CLI files. Fourteen actual Git objects establish their frozen membership, LF-normalized working bytes and exact stage-zero index blobs. Source subtree identities are retained without reading their contents or conducting an absence census.

Two independently restarted, exact-length metadata walks each cover all 36 saved-owner bytes with 14 whole InstructionDB units. All four aliases agree at each start; paired raw replies match. Each walk has its own count bookends. Phase one contains 32 Native invocations and 152 original HTTP replies. The saved owner is one complete range, with name `LIBCRT_unmatched_00bf66ef`, no_return=false and nonthunk; these are identity metadata.

Root's full-original gate passed its first execution. Before phase-two Native work, all 522 exact admitted artifacts were copied and independently read back: 511 metadata preimages, ten listed top files and the control. Original preimages also equal their worker originals. The copy chronology and a second full pre-Native verification are pinned. Phase two captured paired exact 36-byte reads, paired context-zero replies at only the 14 established starts, a closing typed batch and count bookends: seven Native invocations and 36 HTTP replies.

Across both phases, all 39 Native invocations physically read and pin each of the three CLI files before and after each call. All 42 original typed replies agree by corresponding address. Both byte/context pairs match. Project/program stayed `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, with saved language `x86:LE:32:default`, modification number 98 and function count 64,730.

## Physical instructions

| Address | Bytes | Physical instruction |
| --- | --- | --- |
| BF66EF | `55` | PUSH EBP |
| BF66F0 | `8B EC` | MOV EBP,ESP |
| BF66F2 | `FF 35 64 DD 09 01` | PUSH dword through current DS at offset 0109DD64 |
| BF66F8 | `E8 E1 E8 00 00` | CALL C04FDE, signed rel32 +59,617; next/encoded return BF66FD |
| BF66FD | `85 C0` | TEST EAX,EAX |
| BF66FF | `59` | POP ECX |
| BF6700 | `74 03` | JZ BF6705, signed rel8 +3 from BF6702 |
| BF6702 | `5D` | POP EBP |
| BF6703 | `FF E0` | JMP EAX |
| BF6705 | `6A 02` | PUSH sign-extended literal 2 |
| BF6707 | `E8 E7 E7 00 00` | CALL C04EF3, signed rel32 +59,367; next/encoded return BF670C |
| BF670C | `59` | POP ECX |
| BF670D | `5D` | POP EBP |
| BF670E | `E9 A8 FE FF FF` | JMP BF65BB, signed rel32 -344 from numeric next BF6713 |

The complete manual physical record was written before the first fresh detailed Capstone 5.0.7 import/construction. All 14 units passed the first comparison with zero differences in physical fields, operands/access, prefix/opcode bytes, ModR/M and displacement/immediate encoding, explicit and implicit register access, numeric EFLAGS/effects and groups. Both complete byte captures have SHA-256 `0e1f2416d2f25c96b8d07f18d401ef61528110d4b452a1b3a7336bb1cad0e4b3`.

There are no prefixes or SIB bytes. FF /6 with mod=0/rm=5 supplies the absolute disp32 DS source at BF66F2; FF /4 with mod=3/rm=0 supplies register EAX at BF6703. Explicit operand/address decoding is 32 bits. The indirect jump's current target value is unknown.

## Current-state and saved-flow boundaries

Current SS.B=1 must be required separately at every PUSH, POP and near CALL to use ESP-based stack arithmetic modulo 2^32. It is renewed after each opaque call. Current SS.B=0 instead uses SP. MOV EBP,ESP itself copies the explicit current register and requires no stack-address-size predicate. No actual descriptor, stack value, linear address, flat-segment equality or access permission is established.

The dword read through DS at offset 0109DD64 remains unknown. Neither direct callee is interpreted here. If either CALL reaches its encoded returned continuation, all returned register, flag, segment and stack state are unknown; no ABI preservation or aggregate stack balance is imported. In particular, later POP EBP cannot be identified as restoring the original caller's value.

TEST EAX,EAX tests the current post-call EAX. It sets ZF/SF/PF from that value, clears CF/OF and leaves AF undefined. Under uninterrupted normal completion through POP ECX and JZ, POP preserves EAX and EFLAGS, so the branch reaches BF6705 when that EAX is zero. The other path pops current EBP then jumps through unchanged current EAX; there is no established executable target, function signature or callback identity. The zero path pushes literal 2 and calls the second opaque target; after a possible return, both POPs again read current stack state under renewed predicates.

Saved default flow counts are nine FALL_THROUGH, two UNCONDITIONAL_CALL, one CONDITIONAL_JUMP, one COMPUTED_JUMP and one UNCONDITIONAL_JUMP. At BF670E only, default UNCONDITIONAL_JUMP (is_jump=true, is_call=false, is_terminal=false) is overridden to effective CALL_TERMINATOR (is_call=true, is_jump=false, is_terminal=true). Both independently retained target arrays contain BF65BB; both fallthroughs are null and fallthrough_overridden=false. The encoded E9 does not push a return offset. Saved companion names/no-return/thunk flags, including the final no_return=true, do not authenticate identities or target behavior.

## Histories and proof boundary

Current worker Native, decoder, captured-producer, inspection and display failure counts are all zero. Root's current gate passed first with zero inspection failures and one display truncation; its omitted original stdout and preexecution pin remain unavailable, and later compact views are recorded as separate observations.

Accepted ENTRY history is preserved distinctly: the original worker decoder group-name failure (`not64` versus `not64bitmode`), the only actual worker Capstone execution and alias-only disposition; independent Root first manual/actual comparison PASS; prior Root gate inspection/display 1/1; a preparation assertion; both missing-driver and existing-driver failures before any helper execution; and the original Root final-review assertion with a separate minimal retry. The missing-driver record retains the correction to actual tool-reported exit code 1 and the unavailable separate child-process code. These remain accepted earlier histories, without becoming successful-first-execution histories or current-body failures.

The private proof binds the exact two-output commit, one frozen parent, 20 actual Git objects and ten current working/index/LF file checks. A complete pure helper supplied outside the ZIP validates originals and all retained scopes. Strict original/copy receipts permit one ZIP read, one execution of that exact precompiled helper and one exclusive receipt write, denying all other audit events. No archived code or earlier archive is executed or opened.

No Source reconstruction or absence census, Ghidra mutation, build, new test/probe or Original/game/OS/audio/runtime execution occurred. BF6713, numeric callees/jump targets, cells, providers and prototypes remain unqueried. This is static physical/provenance evidence, without ABI or game validation.
