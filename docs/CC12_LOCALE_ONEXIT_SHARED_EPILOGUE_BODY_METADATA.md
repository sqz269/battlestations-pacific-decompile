# Shared epilogue complete saved-body metadata and physical contract

The exact saved owner `00C07C45..00C07C58` contains 20 bytes and 11 whole instructions. Two length-driven metadata walks were accepted by Root before any remaining bytes or text were read. The admitted paired 20-byte reads, context-zero listing, closing walk, independent physical decoder and actual detailed Capstone 5.0.7 agree. This establishes the listed body and its conditional instruction-time effects; Root final review remains pending.

Frozen base: `c4a7bb35971fd8dcf397c54871fb0e0c2ccca943`. Saved library label: `__SEH_epilog4`, treated only as metadata. No prototype was queried in this packet. Raw bytes:

`8B4DF064890D00000000595F5F5E5B8BE55D51C3`

## Bounded evidence

Every typed response verifies `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, modification epoch 98, count 64730, all four exact InstructionDB aliases, complete owner range and default/effective flow. The two independent initial walks derive the next address only from the accepted whole-unit length and stop at `00C07C59` without querying it. The closing pass queries only the same 11 known starts. All three walks agree. The first ten units fall through to their successors; `C07C58` is a terminator with null fallthrough. All overrides are NONE and all target arrays are empty.

There are 42 captured existing-CLI commands: 33 typed-flow responses, two exact byte reads, one context-zero text request and six count bookends. Root independently gated the initial 25 commands; the remaining 17 follow that admission. Raw streams, command records, producers, revisions, all 102 metadata members and their 102 distinct copied Root preimages are retained. No query crosses the admitted range.

## Instruction-time CPU contract

Let B be entry EBP and S be entry ESP. Read32 and Write32 refer to current little-endian 32-bit memory after segment translation at that instruction, not an entry-memory snapshot. This contract assumes 32-bit code, operand and stack-address sizes and normal in-order completion of the listed instructions. The bytes do not capture the live SS descriptor; the stack-address-size condition is explicit. Offset and ESP arithmetic is modulo 2^32. Invalid accesses, faults and intervening asynchronous changes are outside the normal-completion result.

| Address | Bytes | Instruction | Ordered effect |
| --- | --- | --- | --- |
| 00C07C45 | 8B4DF0 | MOV ECX,[EBP-16] | L := current Read32(SS, B-16); ECX := L. |
| 00C07C48 | 64890D00000000 | MOV FS:[0],ECX | Write32(FS, 0, L). This precedes every following stack read. |
| 00C07C4F | 59 | POP ECX | X := current Read32(SS, S); ECX := X; ESP := S+4. |
| 00C07C50 | 5F | POP EDI | D0 := current Read32(SS, S+4); EDI := D0; ESP := S+8. |
| 00C07C51 | 5F | POP EDI | D := current Read32(SS, S+8); EDI := D; ESP := S+12. The first EDI value is overwritten. |
| 00C07C52 | 5E | POP ESI | I := current Read32(SS, S+12); ESI := I; ESP := S+16. |
| 00C07C53 | 5B | POP EBX | K := current Read32(SS, S+16); EBX := K; ESP := S+20. |
| 00C07C54 | 8BE5 | MOV ESP,EBP | ESP := B, where B is entry EBP, unchanged up to this instruction. |
| 00C07C56 | 5D | POP EBP | Bnew := current Read32(SS, B); EBP := Bnew; ESP := B+4. |
| 00C07C57 | 51 | PUSH ECX | ESP := B; Write32(SS, B, X). This overwrites the memory just read into EBP. |
| 00C07C58 | C3 | RET | Y := current Read32(SS, B); ESP := B+4; EIP := Y. Under ordinary stable memory after the preceding PUSH, Y = X. |

There is no LEAVE opcode. `MOV ESP,EBP` and `POP EBP` are separate instructions. The first `POP EDI` is a real read and possible fault point, even though the next pop overwrites EDI. The sequence contains no CALL instruction.

The FS store happens before all later SS reads. FS and SS storage may overlap, including partially; X, D0, D, I, K and Bnew therefore observe memory after that store. Segment selectors do not prove separation. No segment bases or page aliases were queried. The final PUSH writes SS:[B] after POP EBP has read it, and it may overlap FS:[0]. Final FS:[0] is consequently not unconditionally L. The ESP result depends on old B, not Bnew.

On normal completion ECX=X, EDI=D, ESI=I, EBX=K, EBP=Bnew, ESP=B+4 and EIP=Y. Under ordinary sequential memory with no intervening change to SS:[B] after PUSH, Y=X. EAX, EDX, EFLAGS and segment registers are not written by these instructions. No equality between X and an Original caller return address, between Bnew and an earlier caller frame, or between any loaded register and an earlier saved value is established.

## Finite copied-archive proof

The frozen proof reads exactly eight positive files: the accepted ENTRY and ENTRY_PRIMARY doc/report pairs, `config/target.json` and the three existing CLI tools. Six actual commit/tree objects and selected blob bytes are verified against physical copies. The `src` and `include` trees are recorded by identity only; there is no Source content read, census, prior-archive traversal or claim of equality with current Main.

`core_v1.zip` and its distinct physical copy each contain 323 members and are 1,876,057 bytes, SHA-256 `1c5abc4f4112e838fca52fb512960531bd6278edbcd1de9619eec0719dd4d1c9`. Both strict replays passed on the first attempt. Each audited execution permits one supplied ZIP read, one exclusive receipt write and one pinned precompiled helper execution; both receipts show zero denials. The detailed producer, archive, receipt and physical-decoder pins are recorded in the paired JSON report. Delivery after the exact two-output commit is checked separately using actual Git objects, unchanged positive inputs and equality of working, index and committed output bytes.

## Retained incidents and limits

Root had one offline schema-inspection AttributeError before gate generation because a list was treated as a mapping; its failure record, reconstructed exact executed producer and stdout/stderr are preserved. Root gate execution itself had no failures or Native queries. The worker had one local process-creation failure (OS error 267) while listing Root artifacts because the working-directory path duplicated `decompile`. No process, command, Native query or GPR operation ran in that attempt; the corrected listing succeeded. Its exact request/error record is retained. Worker Native captures, helper executions and both core replays had zero failures. A filename suffix was corrected in the core fragment before execution, with the previous full physical fragment preserved.

No claim is made that L is a valid exception-chain link or that the FS store restores an earlier value. No prologue state was imported and no inverse-prologue or Original ABI assertion is made. Full saved-body coverage does not prove FH3 behavior, library equivalence, startup success, Original execution or gameplay behavior. No C++ implementation, build, test, probe, Original/game process, OS/audio operation, neighboring/data/callee/prototype/decompile query or GPR mutation was performed. Root must independently review this packet before any subsequent boundary is admitted.
