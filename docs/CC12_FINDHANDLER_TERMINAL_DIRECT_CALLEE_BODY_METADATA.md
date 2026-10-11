# CC12 terminal direct-callee complete saved-body evidence

The exact saved range `00C04DE2–00C04ED4` contains 61 complete instructions and 243 bytes. Paired original bytes, paired context-zero text and an independent physical decoder agree with fresh detailed Capstone 5.0.7. The final byte is `CC` / `INT3`; Ghidra's saved `COMPUTED_CALL_TERMINATOR` classification does not turn it into a physical near CALL.

The accepted incoming edge remains the earlier `E8 3A D3 FF FF` at `00C07AA3`, whose signed displacement reaches `00C04DE2` and whose encoded return offset is `00C07AA8`. This packet admits the complete saved owner after two independently restarted, length-driven metadata walks. The owner name `_abort`, `no_return=true`, automatic direct-target companion objects and saved EXTERNAL targets remain metadata, without library, provider, ABI or semantic nonreturn authentication.

## Capture and review boundary

The branch freezes actual Main `d84ba6573158785940cc2a865b891e93c5503223`; later Main advancement is permitted by the recorded ancestry relation. Eight positive inputs are the accepted ENTRY/ENTRY_PRIMARY document/report pairs, target configuration and three CLI sources. Fourteen actual initial Git objects establish their tree and blob identities; Source trees are identity-only. Each of 133 Native CLI commands physically reread all three CLI working sources before and after execution, comparing the original bytes and LF-normalized actual Git blobs. Repeated identical raw source payloads use content-addressed storage with separate observation records.

Phase 1 used 126 CLI commands and 622 original HTTP responses for two 61-unit walks and four count bookends. Root admitted the whole range after reviewing the full originals. Phase 2 used seven CLI commands and 83 responses: paired exact 243-byte reads, paired context-zero text only at the 61 proven starts, all 61 closing typed replies and count bookends. All paired/closing originals agree at modification number 98 and 64,730 functions. The project is `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, language `x86:LE:32:default`. The stop cursor `00C04ED5` was never queried or classified.

Root's first gate failed because it incorrectly required default and effective target arrays to match. At `00C04EBD` and `00C04EC7`, default targets are empty while saved effective targets are `EXTERNAL:00000160` and `EXTERNAL:00000118`; overrides remain NONE and flow/fallthrough agree. All four original failed-gate files are retained among 1,553 physically copied Root artifacts. The failed gate's in-memory Git streams were not separately retained. The corrected Root gate passed; these saved external identities do not identify a current callable provider.

The first offline decoder process completed and recorded one comparison difference: the manual INT3 flag annotation omitted Capstone's coarse interrupt effects. Its complete producers, output artifacts, preexecution pins and actual process streams are retained. A separate corrected offline attempt compares all 61 units with no differences; Native bytes were not recaptured. This is one recorded comparison correction, not an unreported first-pass success.

## Physical instruction inventory

Every row is a complete admitted unit. The JSON report retains manual prefix/opcode/ModRM/SIB and scalar fields, relative-target arithmetic, default segments, operand access, register effects, EFLAGS metadata, saved flow and the actual detailed decoder record. LEA's displayed addressing form performs no memory access.

| Address | Bytes | Saved instruction text |
| --- | --- | --- |
| `00C04DE2` | `55` | `PUSH EBP` |
| `00C04DE3` | `8dac2458fdffff` | `LEA EBP,[ESP + 0xfffffd58]` |
| `00C04DEA` | `81ec28030000` | `SUB ESP,0x328` |
| `00C04DF0` | `a19055e100` | `MOV EAX,[0x00e15590]` |
| `00C04DF5` | `33c5` | `XOR EAX,EBP` |
| `00C04DF7` | `8985a4020000` | `MOV dword ptr [EBP + 0x2a4],EAX` |
| `00C04DFD` | `f605f85ae10001` | `TEST byte ptr [0x00e15af8],0x1` |
| `00C04E04` | `56` | `PUSH ESI` |
| `00C04E05` | `7408` | `JZ 0x00c04e0f` |
| `00C04E07` | `6a0a` | `PUSH 0xa` |
| `00C04E09` | `e89a0b0000` | `CALL 0x00c059a8` |
| `00C04E0E` | `59` | `POP ECX` |
| `00C04E0F` | `e80b230100` | `CALL 0x00c1711f` |
| `00C04E14` | `85c0` | `TEST EAX,EAX` |
| `00C04E16` | `7408` | `JZ 0x00c04e20` |
| `00C04E18` | `6a16` | `PUSH 0x16` |
| `00C04E1A` | `e85c250100` | `CALL 0x00c1737b` |
| `00C04E1F` | `59` | `POP ECX` |
| `00C04E20` | `f605f85ae10002` | `TEST byte ptr [0x00e15af8],0x2` |
| `00C04E27` | `0f84a0000000` | `JZ 0x00c04ecd` |
| `00C04E2D` | `898588000000` | `MOV dword ptr [EBP + 0x88],EAX` |
| `00C04E33` | `898d84000000` | `MOV dword ptr [EBP + 0x84],ECX` |
| `00C04E39` | `899580000000` | `MOV dword ptr [EBP + 0x80],EDX` |
| `00C04E3F` | `895d7c` | `MOV dword ptr [EBP + 0x7c],EBX` |
| `00C04E42` | `897578` | `MOV dword ptr [EBP + 0x78],ESI` |
| `00C04E45` | `897d74` | `MOV dword ptr [EBP + 0x74],EDI` |
| `00C04E48` | `668c95a0000000` | `MOV word ptr [EBP + 0xa0],SS` |
| `00C04E4F` | `668c8d94000000` | `MOV word ptr [EBP + 0x94],CS` |
| `00C04E56` | `668c5d70` | `MOV word ptr [EBP + 0x70],DS` |
| `00C04E5A` | `668c456c` | `MOV word ptr [EBP + 0x6c],ES` |
| `00C04E5E` | `668c6568` | `MOV word ptr [EBP + 0x68],FS` |
| `00C04E62` | `668c6d64` | `MOV word ptr [EBP + 0x64],GS` |
| `00C04E66` | `9c` | `PUSHFD` |
| `00C04E67` | `8f8598000000` | `POP dword ptr [EBP + 0x98]` |
| `00C04E6D` | `8bb5ac020000` | `MOV ESI,dword ptr [EBP + 0x2ac]` |
| `00C04E73` | `8d85ac020000` | `LEA EAX,[EBP + 0x2ac]` |
| `00C04E79` | `89859c000000` | `MOV dword ptr [EBP + 0x9c],EAX` |
| `00C04E7F` | `c745d801000100` | `MOV dword ptr [EBP + -0x28],0x10001` |
| `00C04E86` | `89b590000000` | `MOV dword ptr [EBP + 0x90],ESI` |
| `00C04E8C` | `8b40fc` | `MOV EAX,dword ptr [EAX + -0x4]` |
| `00C04E8F` | `6a50` | `PUSH 0x50` |
| `00C04E91` | `89858c000000` | `MOV dword ptr [EBP + 0x8c],EAX` |
| `00C04E97` | `8d4580` | `LEA EAX,[EBP + -0x80]` |
| `00C04E9A` | `6a00` | `PUSH 0x0` |
| `00C04E9C` | `50` | `PUSH EAX` |
| `00C04E9D` | `e84e2bffff` | `CALL 0x00bf79f0` |
| `00C04EA2` | `8d4580` | `LEA EAX,[EBP + -0x80]` |
| `00C04EA5` | `83c40c` | `ADD ESP,0xc` |
| `00C04EA8` | `8945d0` | `MOV dword ptr [EBP + -0x30],EAX` |
| `00C04EAB` | `8d45d8` | `LEA EAX,[EBP + -0x28]` |
| `00C04EAE` | `6a00` | `PUSH 0x0` |
| `00C04EB0` | `c7458015000040` | `MOV dword ptr [EBP + -0x80],0x40000015` |
| `00C04EB7` | `89758c` | `MOV dword ptr [EBP + -0x74],ESI` |
| `00C04EBA` | `8945d4` | `MOV dword ptr [EBP + -0x2c],EAX` |
| `00C04EBD` | `ff152c22ce00` | `CALL dword ptr [0x00ce222c]` |
| `00C04EC3` | `8d45d0` | `LEA EAX,[EBP + -0x30]` |
| `00C04EC6` | `50` | `PUSH EAX` |
| `00C04EC7` | `ff150c21ce00` | `CALL dword ptr [0x00ce210c]` |
| `00C04ECD` | `6a03` | `PUSH 0x3` |
| `00C04ECF` | `e8f86effff` | `CALL 0x00bfbdcc` |
| `00C04ED4` | `cc` | `INT3` |

## Bounded control flow and state

Each local operation is conditional on normal completion, sufficient privilege, valid current segment limits, and readable/writable memory as applicable.

Renew current SS.B=1 before every implicit stack description. Under that predicate PUSH/PUSHFD/near CALL decrement current ESP by four and write via current SS; POP reads via current SS at current ESP and increments it by four. No relation between separately returned current states is assumed.

Every direct and computed call is opaque. Whether it returns is unknown; if control next reaches a saved return cursor, all registers, flags, memory and segment state are newly unknown. No callee convention, argument count, preserved EBP/ESI, preserved SS.B or stack balancing is inferred.

EBP/ESP-based explicit memory operands default to current SS; absolute and EAX-based memory operands default to current DS. LEA computes an offset and reads no memory; its decoded addressing form does not perform a segment access. SS and DS are never equated.

Three JE targets are in-range proven starts. Numeric direct-call targets come only from signed displacement arithmetic. The two FF /2 calls read their target from current DS memory at encoded offsets CE222C and CE210C; those cells and their current contents were not read. Separate preceding PUSH instructions occur before those cell reads and may alias the cells under unknown current DS/SS mappings. Each near CALL reads its target operand before its own implicit return-offset stack write; no earlier cell value or non-alias assumption is propagated.

Six 66 8C /r instructions store the current 16-bit SS/CS/DS/ES/FS/GS selector values via current SS at EBP-relative offsets. They do not authenticate any segment descriptor or equality of segment bases.

PUSHFD uses its architectural flag image under the applicable mode/privilege conditions; no assertion equates that image with every current EFLAGS bit. The following POP memory is a current-SS operation, conditional on its own current state and normal completion.

CC at C04ED4 is INT3, a software breakpoint exception, not a near CALL. Capstone lists MODIFY_IF/NT/RF/TF as coarse interrupt effects; actual mode/gate/handler effects and any later continuation are unknown. C04ED5 was neither queried nor classified.

No recovered function prototype, calling convention, argument layout, library identity, provider binding or semantic nonreturn proof is established.

The branch tests are local: `C04E05` uses the preceding byte TEST's ZF when reached normally through the intervening PUSH; `C04E16` uses `TEST EAX,EAX`; `C04E27` uses the separate later byte TEST. A returned call state is independently unknown. The five direct numeric destinations are `C059A8`, `C1711F`, `C1737B`, `BF79F0` and `BFBDCC`; none was queried. The two computed calls dereference current DS offsets `CE222C` and `CE210C`; no cell content, provider or current target is established.

`LEA EAX,[EBP+2AC]` computes an offset. The later `MOV EAX,[EAX-4]` dereferences current DS, whereas EBP-relative stores use current SS. This does not identify an incoming return address or argument without additional segment and state evidence. After each opaque call, later EBP/ESI/ESP values are freshly unknown; the visible ADD/POP instructions do not prove a calling convention or whole-body stack balance.

Private finite supplied-ZIP proof checks reconstruct the manual records, validate all original HTTP envelopes and command/CLI pins, compare the 183 typed responses, verify the retained Root gate and comparison correction, and authenticate the actual final six Git objects together with the initial fourteen. The strict wrapper allows one supplied ZIP read, one execution of the exact separately pinned helper code object and one exclusive receipt write, denying every other audited event. Original and distinct physical-copy receipts are separate from the public outputs. No archived code or earlier proof archive is executed or opened for this packet.

Only this document and its report are published. No Source payload, new C++, test, build, probe, Ghidra write, Original executable read, game run or OS/audio runtime experiment is part of this packet. This is complete bounded listing evidence, not ABI compatibility or game validation.
