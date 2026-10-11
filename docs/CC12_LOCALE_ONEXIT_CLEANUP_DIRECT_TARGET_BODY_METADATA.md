# Cleanup direct-call target: complete saved-body metadata

The exact saved body `C11B31..C11B45` contains 21 bytes and seven whole instructions. Two independent metadata walks were gated by Root before the full bytes and paired text were read. Paired exact byte captures, paired context-zero listings, closing metadata, independent opcode/ModRM/SIB decoding and actual detailed Capstone 5.0.7 agree. This establishes physical instruction behavior with explicit memory and call boundaries; Root final review remains pending.

Frozen base: `29cbf21392cce9f8d5594e5442476266766097b7`. Saved label `__unlock` is metadata only. Raw body:

`558BEC8B4508FF34C57864E100FF151022CE005DC3`

## Bounded evidence and method provenance

All 21 complete original direct metadata responses verify `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, epoch 98, function count 64730, all four exact InstructionDB aliases and the complete saved 21-byte owner range. Each initial walk independently derives its next query from the accepted unit length and stops before `C11B46`. Closing reads query only the seven known starts. All three walks agree.

Partition: `C11B31/1, C11B32/2, C11B34/3, C11B37/7, C11B3E/6, C11B44/1, C11B45/1`. Five units are FALL_THROUGH. `C11B3E` is COMPUTED_CALL with default targets empty, saved effective target `EXTERNAL:00000159`, and fallthrough `C11B44`. `C11B45` is TERMINATOR with null fallthrough. Every override is NONE. The EXTERNAL scalar is not a RAM target or authenticated provider and was never queried; automatically returned call-target objects remain opaque.

There are 31 existing-CLI commands: 21 typed-flow requests, two exact 21-byte reads, two context-zero text requests and six count bookends. Seventeen precede Root admission; fourteen follow it. Original direct-response base64/digests, exact argv and original stdout/stderr are retained. GET-method provenance comes from the frozen existing `fetch_raw` implementation, Root's AST check, recorded URLs/status and per-command provenance; no separate network trace is claimed.

Root's producer, receipt, stdout/stderr, metadata manifest, 93 metadata/preimage files and 23 frozen Git files were copied unchanged, totaling 121 artifacts. Local helper producers are pinned before execution and their original streams are captured from the first attempt. No GPR mutation occurred.

## Physical fields and instruction-time contract

`FF 34 C5 78 64 E1 00` is FF /6 PUSH m32. ModRM `34` selects mod=0, group=6, r/m=4; SIB `C5` has scale bits 3 (eight), index EAX and base bits 5. With mod=0, those base bits mean no base, followed by displacement `00E16478`; the memory operand defaults to DS. It is not an EBP-based SS operand. `FF 15 10 22 CE 00` is FF /2 CALL m32 with mod=0/rm=5, no SIB and absolute displacement `00CE2210`, also default DS. That is an encoded pointer-cell offset, not its contents or the callee address. No immediate argument is embedded in either FF instruction.

Every ESP formula below requires actual 32-bit SS stack-address size at that instruction, 32-bit code/operand decoding, valid accesses and normal execution of the listed instruction. No live descriptor was queried. Arithmetic is modulo 2^32. Every read uses the segment, registers and memory current at its own instruction. After an opaque return, these conditions must hold anew. The source/target ordering below expresses architectural operand-value dependencies, not measured bus order or fault precedence.

| Address | Bytes | Instruction | Qualified effect |
| --- | --- | --- | --- |
| 00C11B31 | 55 | PUSH EBP | Let entry ESP=S and entry EBP=B. Under the stated actual 32-bit SS stack-size condition, ESP:=S-4 and current SS:[S-4]:=B. |
| 00C11B32 | 8BEC | MOV EBP,ESP | EBP:=current ESP. In the consecutive normal prefix this is S-4; this is a register assignment, not proof of an Original caller frame. |
| 00C11B34 | 8B4508 | MOV EAX,[EBP+8] | A:=Read32(current SS, current EBP+8); EAX:=A. In the qualified prefix the offset is S+4. This read occurs after the entry stack write. |
| 00C11B37 | FF34C57864E100 | PUSH [EAX*8+E16478] | V:=Read32(current DS, (current EAX*8+00E16478) mod 2^32), before the stack destination write. Under actual 32-bit SS stack size in the prefix, ESP:=S-8 and current SS:[S-8]:=V. |
| 00C11B3E | FF151022CE00 | CALL [00CE2210] | K:=Read32(current DS, 00CE2210) after the preceding memory-PUSH write and before this CALL return-address write. Under actual 32-bit SS stack size in the prefix, ESP:=S-12 and current SS:[S-12]:=00C11B44; the dynamic branch target is K. |
| 00C11B44 | 5D | POP EBP | Only if control returns here and actual 32-bit SS stack size holds anew, let Q be returned ESP. Bnew:=Read32(current SS,Q); EBP:=Bnew; ESP:=Q+4. Q has no established relation to S. |
| 00C11B45 | C3 | RET | If it normally follows that POP and actual 32-bit SS stack size holds, R:=Read32(current SS,Q+4); ESP:=Q+8 and EIP:=R. R is unknown. |

The first stack write precedes the EBP-relative read. The scaled DS source is read before its own stack write. That stack write may alias `DS:[00CE2210]`, so the CALL reads K after it rather than from an earlier snapshot. The CALL's return-address write may also alias the pointer cell; K is the operand value determined before that write. No final contents of the cell or earlier stack words are asserted. DS and SS separation, table bounds, object identity and a particular caller argument are not assumed.

If the computed callee returns at `C11B44`, every returned general register, EBP, ESP, flag, segment and memory value is unknown. Q has no established relation to S. `POP EBP` is therefore a current-memory load, not proven restoration of entry B; RET uses a subsequent current-memory value, not a proven Original return address. The listed local instructions do not change arithmetic flags on normal completion, but flags after the opaque call are independently unknown. The body does not prove argument cleanup, stack balance, an unlock operation or a callee convention.

## Finite copied-archive proof and limits

The eight positive files are exactly the accepted ENTRY and ENTRY_PRIMARY doc/report pairs, `config/target.json` and the three existing CLI tools. Six actual commit/tree objects and selected blob bytes are checked against physical working copies. Root's 23 separate frozen copies are checked against the same objects. Source trees are recorded by identity only: no Source contents or census, prior proof-archive recursion, current-Main equality claim or Source comparison is part of this packet.

The core ZIP and distinct physical copy each contain 325 members and are 2,804,663 bytes, SHA-256 `a63f3ce849d9af7cae6fa7c7a518b73223c7fa4cdce9dd8852a8776547162b72`. Both supplied-ZIP replays passed on their first attempt. Each audited helper uses one physical archive read, one pinned execution and one exclusive receipt write, with zero denials. Artifact, producer, receipt and local execution pins are in the JSON report. The packaging invocation is pinned before execution; its actual result/stdout/stderr are retained separately after ZIP creation. Delivery is a separate post-commit actual-Git proof of exactly the two public output paths, unchanged eight positive inputs and working/index/committed byte equality.

This packet and its current Root listing gate had zero Native, offline or guarded core failures. Prior producer bytes are retained. This statement does not reclassify historical incidents in accepted input packets. No `C11B46`, prototype, data/table, pointer cell, descriptor, EXTERNAL or callee query was made. No C++ change, build, test, native probe, Original/game execution, OS/audio validation, runtime result or Original ABI equivalence is credited. Any Source comparison or provider contract requires separate explicit scope after Root review.
