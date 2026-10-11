# C059A8: six disjoint body ranges, listing metadata and physical decode

This packet reviews all 399 saved body addresses at `00C059A8` as 150 whole instructions in six disjoint ranges. Two independent typed length walks preceded Root's byte/text admission. The admitted byte and context-zero text captures agree, and a closing typed pass matches both original walks at every instruction. This is evidence and conditional x86 semantics; it adds no reconstructed C++ or authenticated library identity.

The eight positive inputs are the preceding entry report/document and their PRIMARY pair, `config/target.json`, and the three existing CLI producers `tools/bsp.py`, `tools/ghidra_export.py`, `tools/ghidra_typed_flow.py`. The actual frozen commit is `dea25e8d8b07300efb6db6ba04088d1b7f8f3964` with 14 actual commit/tree/blob objects. `src` and `include` are retained as subtree identities only. Their contents were not read. Main may advance after the frozen observation.

## Exact admission and captures

| Start | Last admitted byte | Bytes | Whole instructions | Unqueried end + 1 |
|---|---|---:|---:|---|
| `00C059A8` | `00C05A29` | 130 | 44 | `00C05A2A` |
| `00C05A2D` | `00C05A6D` | 65 | 22 | `00C05A6E` |
| `00C05A71` | `00C05AB0` | 64 | 27 | `00C05AB1` |
| `00C05AB6` | `00C05AD4` | 31 | 14 | `00C05AD5` |
| `00C05AD8` | `00C05AF6` | 31 | 13 | `00C05AF7` |
| `00C05AFA` | `00C05B47` | 78 | 30 | `00C05B48` |

The unqueried gaps are `00C05A2A..00C05A2C`, `00C05A6E..00C05A70`, `00C05AB1..00C05AB5`, `00C05AD5..00C05AD7`, and `00C05AF7..00C05AF9`. The final next cursor `00C05B48` was also unqueried. Each independent range started at its explicitly admitted start. No successor was inferred across a gap.

The packet retains 470 successful CLI invocations: 450 typed responses (two initial walks and one closing pass), 12 exact-range byte reads, two context-zero text batches containing precisely the 150 proven starts, and six count checks. All three existing CLI files were physically pinned before and after every invocation. Ghidra remained project `bsp`, program `/battlestationspacific.exe`, language `x86:LE:32:default`, image base `00400000`, function count 64,730 and modification number 98. Complete original typed JSON text/base64 and transport metadata are retained, including all listing aliases, body ranges, owner flags, target arrays and default/effective flow fields. The raw bytes of all three typed responses at each address agree.

Typed metadata uses the existing GET producer. Context text uses the existing `get_assembly_context` POST with `context_instructions=0`. These are pinned producer and response records, not a separate network trace; the text producer's internal transport retry loop is not independently instrumented. All 470 observed CLI calls succeeded on their first invocation.

Root admitted the six ranges after its complete original-payload review. All 1,984 actual Root artifacts were copied and pinned, including its 1,975 metadata preimages, gate producer/prepin/execution/original streams/receipt, copied manifest and inspection incident provenance. The count is from the actual directory contents.

## Physical fields and segment-sensitive effects

The separate manual decoder accounts for every byte: no prefixes; exact opcode bytes; every ModR/M `mod/reg/rm`; each SIB scale/index/base including no-base forms; little-endian displacement and immediate width, raw value and sign; operand/address widths; register and memory operands; and relative destinations computed from the next EIP. Fresh detailed Capstone 5.0.7 ran outside the audit, with the actual module and native library hashes retained. All 150 instructions agree with the manual fields. Pure replay also checks every complete Ghidra text operand against the manual result, allowing only the recorded condition-code spelling aliases.

`8B 5C 24 0C` at `00C059AA` has ModR/M `5C` (mod 1, reg EBX, SIB) and SIB `24` (scale 1, no index, ESP base): it reads a dword from current SS:ESP+12. If the preceding two pushes completed with current 32-bit SS stack addressing, initial ESP `S` becomes `S-8` before this read, so its offset is `S+4` modulo 2^32. The read occurs after both stores. This establishes no argument role or calling convention.

The SIB `FD` forms at `00C059B4`, `00C05AD8`, and `00C05B29` have scale 8, EDI index and no base. The data accesses use current DS, with displacements `00E15B10` or `00E15B14`. The latter instruction is LEA and performs no data read or segment translation. `00C05B24` is also LEA: its ESP-based address expression is an offset computation, not an SS memory load.

`C6 05 6D DF 09 01 00` at `00C05A3A` is a byte store of zero to current DS:`0109DF6D`. The `FF /6` operations at `00C05AD8`, `00C05B30`, and `00C05B39` read their current DS dword operand before storing it on the current SS stack. The three `FF 15` operations read current DS dwords at `00CE2244`, `00CE20AC`, and `00CE2290` before the CALL return-address store. Their values and dynamic destinations are unknown. Prior stores may alias later reads; neither DS=SS nor disjoint physical backing is assumed. These are architectural value dependencies, not a bus access or fault-order claim.

Every PUSH, POP, near CALL and RET row has its own predicate: **the current SS uses 32-bit stack addressing, and the instruction completes normally**. With current ESP `Q`, PUSH/CALL store four bytes at SS:`Q-4` and update ESP modulo 2^32; POP/RET read SS:`Q` and advance ESP by four. A CALL pushes its arithmetic next EIP. Actual segment descriptors, memory mappings and exception outcomes remain unexamined.

All 19 CALLs are opaque. A numeric direct target does not authenticate its function name or behavior. On any return from a call, all registers, flags, segments and memory are unknown, including ESP, EBP, ESI, EDI and previously observed values. A current-SS 32-bit predicate must be renewed before any subsequent stack formula. In particular, the five ESI pushes after `00C05A14`, `00C05ABF`, and `00C05AE1` do not inherit an earlier ESI=0; `00C05B39` does not inherit the pre-`00C05B32` ESI or memory value. `ADD ESP,12/20` uses the returned current ESP and does not prove callee cleanup or frame restoration.

At `00C05B42`, conditional on entry there with current ESP `Q`, the renewed current-SS predicate, and normal completion of the straight-line suffix, the five POPs assign EBP/EDI/ESI/EBX/ECX from current SS offsets `Q`, `Q+4`, `Q+8`, `Q+12`, `Q+16`. RET then reads EIP from SS:`Q+20` and leaves ESP=`Q+24`, all modulo 2^32. Entry at `00C05B43` begins a separate four-POP/RET suffix. No relationship to an earlier call's stack or original register values is established.

The short `6A F4` push sign-extends -12 to `FFFFFFF4`; `83 FD FF` compares EBP with sign-extended -1. The 16 E8 calls and all conditional/unconditional relative jumps have individually checked signed displacements. Their numeric destinations are listed below; none caused a target query. The complete per-instruction JSON retains symbolic arithmetic-flag effects and the separate Capstone register/flag records.

## Saved metadata differences

Five E8 instructions at `00C05A25`, `00C05A69`, `00C05AAC`, `00C05AD0`, and `00C05AF2` have saved `CALL_RETURN` overrides. Their default flow is `UNCONDITIONAL_CALL` with `is_terminal=false`; effective flow is `CALL_TERMINATOR` with `is_terminal=true`. Both fallthroughs are null, `fallthrough_overridden=false`, and both target arrays contain RAM `00BF65BB`. These fields and the automatically attached target name/no-return flag are retained as metadata. They do not prove that the physical call cannot return, or authorize reading its arithmetic return address in an unadmitted gap.

The computed calls at `00C05A41`, `00C05B11`, and `00C05B3C` have no override and identical default/effective `COMPUTED_CALL` flags. Their default targets are empty; effective targets contain EXTERNAL offsets `00000166`, `00000100`, and `00000179`. Fallthroughs are respectively `00C05A47`, `00C05B17`, and `00C05B42`, with no fallthrough override. EXTERNAL scalar values and automatic names are not RAM addresses, instruction operands, pointer-cell values, or provider proof.

## Complete admitted instruction table

The original Ghidra text is shown without converting saved names into semantics. The JSON report supplies the full manual fields, all detailed Capstone records, and pins for each complete original typed payload.

| Address | Bytes | Original context-zero instruction |
|---|---|---|
| `00C059A8` | `51` | `PUSH ECX` |
| `00C059A9` | `53` | `PUSH EBX` |
| `00C059AA` | `8B5C240C` | `MOV EBX,dword ptr [ESP + 0xc]` |
| `00C059AE` | `56` | `PUSH ESI` |
| `00C059AF` | `57` | `PUSH EDI` |
| `00C059B0` | `33F6` | `XOR ESI,ESI` |
| `00C059B2` | `33FF` | `XOR EDI,EDI` |
| `00C059B4` | `3B1CFD105BE100` | `CMP EBX,dword ptr [EDI*0x8 + 0xe15b10]` |
| `00C059BB` | `7406` | `JZ 0x00c059c3` |
| `00C059BD` | `47` | `INC EDI` |
| `00C059BE` | `83FF17` | `CMP EDI,0x17` |
| `00C059C1` | `72F1` | `JC 0x00c059b4` |
| `00C059C3` | `83FF17` | `CMP EDI,0x17` |
| `00C059C6` | `0F8377010000` | `JNC 0x00c05b43` |
| `00C059CC` | `55` | `PUSH EBP` |
| `00C059CD` | `6A03` | `PUSH 0x3` |
| `00C059CF` | `E883110200` | `CALL 0x00c26b57` |
| `00C059D4` | `83F801` | `CMP EAX,0x1` |
| `00C059D7` | `59` | `POP ECX` |
| `00C059D8` | `0F8431010000` | `JZ 0x00c05b0f` |
| `00C059DE` | `6A03` | `PUSH 0x3` |
| `00C059E0` | `E872110200` | `CALL 0x00c26b57` |
| `00C059E5` | `85C0` | `TEST EAX,EAX` |
| `00C059E7` | `59` | `POP ECX` |
| `00C059E8` | `750D` | `JNZ 0x00c059f7` |
| `00C059EA` | `833D8C55E10001` | `CMP dword ptr [0x00e1558c],0x1` |
| `00C059F1` | `0F8418010000` | `JZ 0x00c05b0f` |
| `00C059F7` | `81FBFC000000` | `CMP EBX,0xfc` |
| `00C059FD` | `0F843F010000` | `JZ 0x00c05b42` |
| `00C05A03` | `68B8A5D600` | `PUSH 0xd6a5b8` |
| `00C05A08` | `BB14030000` | `MOV EBX,0x314` |
| `00C05A0D` | `53` | `PUSH EBX` |
| `00C05A0E` | `BD50DE0901` | `MOV EBP,0x109de50` |
| `00C05A13` | `55` | `PUSH EBP` |
| `00C05A14` | `E88B39FFFF` | `CALL 0x00bf93a4` |
| `00C05A19` | `83C40C` | `ADD ESP,0xc` |
| `00C05A1C` | `85C0` | `TEST EAX,EAX` |
| `00C05A1E` | `740D` | `JZ 0x00c05a2d` |
| `00C05A20` | `56` | `PUSH ESI` |
| `00C05A21` | `56` | `PUSH ESI` |
| `00C05A22` | `56` | `PUSH ESI` |
| `00C05A23` | `56` | `PUSH ESI` |
| `00C05A24` | `56` | `PUSH ESI` |
| `00C05A25` | `E8910BFFFF` | `CALL 0x00bf65bb` |
| `00C05A2D` | `6804010000` | `PUSH 0x104` |
| `00C05A32` | `BE69DE0901` | `MOV ESI,0x109de69` |
| `00C05A37` | `56` | `PUSH ESI` |
| `00C05A38` | `6A00` | `PUSH 0x0` |
| `00C05A3A` | `C6056DDF090100` | `MOV byte ptr [0x0109df6d],0x0` |
| `00C05A41` | `FF154422CE00` | `CALL dword ptr [0x00ce2244]` |
| `00C05A47` | `85C0` | `TEST EAX,EAX` |
| `00C05A49` | `7526` | `JNZ 0x00c05a71` |
| `00C05A4B` | `68A0A5D600` | `PUSH 0xd6a5a0` |
| `00C05A50` | `68FB020000` | `PUSH 0x2fb` |
| `00C05A55` | `56` | `PUSH ESI` |
| `00C05A56` | `E84939FFFF` | `CALL 0x00bf93a4` |
| `00C05A5B` | `83C40C` | `ADD ESP,0xc` |
| `00C05A5E` | `85C0` | `TEST EAX,EAX` |
| `00C05A60` | `740F` | `JZ 0x00c05a71` |
| `00C05A62` | `33C0` | `XOR EAX,EAX` |
| `00C05A64` | `50` | `PUSH EAX` |
| `00C05A65` | `50` | `PUSH EAX` |
| `00C05A66` | `50` | `PUSH EAX` |
| `00C05A67` | `50` | `PUSH EAX` |
| `00C05A68` | `50` | `PUSH EAX` |
| `00C05A69` | `E84D0BFFFF` | `CALL 0x00bf65bb` |
| `00C05A71` | `56` | `PUSH ESI` |
| `00C05A72` | `E869E3FFFF` | `CALL 0x00c03de0` |
| `00C05A77` | `40` | `INC EAX` |
| `00C05A78` | `83F83C` | `CMP EAX,0x3c` |
| `00C05A7B` | `59` | `POP ECX` |
| `00C05A7C` | `7638` | `JBE 0x00c05ab6` |
| `00C05A7E` | `56` | `PUSH ESI` |
| `00C05A7F` | `E85CE3FFFF` | `CALL 0x00c03de0` |
| `00C05A84` | `83EE3B` | `SUB ESI,0x3b` |
| `00C05A87` | `03C6` | `ADD EAX,ESI` |
| `00C05A89` | `6A03` | `PUSH 0x3` |
| `00C05A8B` | `B964E10901` | `MOV ECX,0x109e164` |
| `00C05A90` | `68983FCF00` | `PUSH 0xcf3f98` |
| `00C05A95` | `2BC8` | `SUB ECX,EAX` |
| `00C05A97` | `51` | `PUSH ECX` |
| `00C05A98` | `50` | `PUSH EAX` |
| `00C05A99` | `E8E284FFFF` | `CALL 0x00bfdf80` |
| `00C05A9E` | `83C414` | `ADD ESP,0x14` |
| `00C05AA1` | `85C0` | `TEST EAX,EAX` |
| `00C05AA3` | `7411` | `JZ 0x00c05ab6` |
| `00C05AA5` | `33F6` | `XOR ESI,ESI` |
| `00C05AA7` | `56` | `PUSH ESI` |
| `00C05AA8` | `56` | `PUSH ESI` |
| `00C05AA9` | `56` | `PUSH ESI` |
| `00C05AAA` | `56` | `PUSH ESI` |
| `00C05AAB` | `56` | `PUSH ESI` |
| `00C05AAC` | `E80A0BFFFF` | `CALL 0x00bf65bb` |
| `00C05AB6` | `33F6` | `XOR ESI,ESI` |
| `00C05AB8` | `68801DD200` | `PUSH 0xd21d80` |
| `00C05ABD` | `53` | `PUSH EBX` |
| `00C05ABE` | `55` | `PUSH EBP` |
| `00C05ABF` | `E8073DFFFF` | `CALL 0x00bf97cb` |
| `00C05AC4` | `83C40C` | `ADD ESP,0xc` |
| `00C05AC7` | `85C0` | `TEST EAX,EAX` |
| `00C05AC9` | `740D` | `JZ 0x00c05ad8` |
| `00C05ACB` | `56` | `PUSH ESI` |
| `00C05ACC` | `56` | `PUSH ESI` |
| `00C05ACD` | `56` | `PUSH ESI` |
| `00C05ACE` | `56` | `PUSH ESI` |
| `00C05ACF` | `56` | `PUSH ESI` |
| `00C05AD0` | `E8E60AFFFF` | `CALL 0x00bf65bb` |
| `00C05AD8` | `FF34FD145BE100` | `PUSH dword ptr [EDI*0x8 + 0xe15b14]` |
| `00C05ADF` | `53` | `PUSH EBX` |
| `00C05AE0` | `55` | `PUSH EBP` |
| `00C05AE1` | `E8E53CFFFF` | `CALL 0x00bf97cb` |
| `00C05AE6` | `83C40C` | `ADD ESP,0xc` |
| `00C05AE9` | `85C0` | `TEST EAX,EAX` |
| `00C05AEB` | `740D` | `JZ 0x00c05afa` |
| `00C05AED` | `56` | `PUSH ESI` |
| `00C05AEE` | `56` | `PUSH ESI` |
| `00C05AEF` | `56` | `PUSH ESI` |
| `00C05AF0` | `56` | `PUSH ESI` |
| `00C05AF1` | `56` | `PUSH ESI` |
| `00C05AF2` | `E8C40AFFFF` | `CALL 0x00bf65bb` |
| `00C05AFA` | `6810200100` | `PUSH 0x12010` |
| `00C05AFF` | `6878A5D600` | `PUSH 0xd6a578` |
| `00C05B04` | `55` | `PUSH EBP` |
| `00C05B05` | `E8900E0200` | `CALL 0x00c2699a` |
| `00C05B0A` | `83C40C` | `ADD ESP,0xc` |
| `00C05B0D` | `EB33` | `JMP 0x00c05b42` |
| `00C05B0F` | `6AF4` | `PUSH -0xc` |
| `00C05B11` | `FF15AC20CE00` | `CALL dword ptr [0x00ce20ac]` |
| `00C05B17` | `8BE8` | `MOV EBP,EAX` |
| `00C05B19` | `3BEE` | `CMP EBP,ESI` |
| `00C05B1B` | `7425` | `JZ 0x00c05b42` |
| `00C05B1D` | `83FDFF` | `CMP EBP,-0x1` |
| `00C05B20` | `7420` | `JZ 0x00c05b42` |
| `00C05B22` | `6A00` | `PUSH 0x0` |
| `00C05B24` | `8D442414` | `LEA EAX,[ESP + 0x14]` |
| `00C05B28` | `50` | `PUSH EAX` |
| `00C05B29` | `8D34FD145BE100` | `LEA ESI,[EDI*0x8 + 0xe15b14]` |
| `00C05B30` | `FF36` | `PUSH dword ptr [ESI]` |
| `00C05B32` | `E8A9E2FFFF` | `CALL 0x00c03de0` |
| `00C05B37` | `59` | `POP ECX` |
| `00C05B38` | `50` | `PUSH EAX` |
| `00C05B39` | `FF36` | `PUSH dword ptr [ESI]` |
| `00C05B3B` | `55` | `PUSH EBP` |
| `00C05B3C` | `FF159022CE00` | `CALL dword ptr [0x00ce2290]` |
| `00C05B42` | `5D` | `POP EBP` |
| `00C05B43` | `5F` | `POP EDI` |
| `00C05B44` | `5E` | `POP ESI` |
| `00C05B45` | `5B` | `POP EBX` |
| `00C05B46` | `59` | `POP ECX` |
| `00C05B47` | `C3` | `RET` |

## Proof and remaining boundary

The evidence archive has 4970 members, 24331472 bytes, SHA-256 `3c4bbd72afcd8a30785c898592b75d5788a19d7afa7d31106bb864a7c1719a8e`. Its separate physical copy is byte-identical and has a distinct file ID. Both strict replays passed on their first execution with exactly one supplied-ZIP read, one exact precompiled helper execution and one exclusive receipt write; every other audit event is denied and the actual denied-event list is empty. The complete replay helper was physically reviewed and pinned before execution. Actual Capstone loading/execution occurred outside that audit. The report records the helper, guard, manifest, archive and receipt pins.

No functional Native CLI, offline helper, Root gate or strict proof execution failed through publication. Two worker inspection displays and one Root inspection display exceeded their output caps; their command/provenance records are retained and the complete underlying artifacts were inspected with bounded follow-ups. These transcripts are not represented as separately captured original stdout/stderr. Historical incidents inside positive earlier reports keep their original scope.

Only this document and its JSON report are public outputs. Final delivery adds the actual worker commit/tree/blob objects, original Git command streams, eight final positive input comparisons and both output comparisons, then repeats strict replay on two distinct delivery ZIP files. The delivery receipts are private post-commit artifacts and are reported separately; this document does not predict their result.

No gaps/end-plus-one addresses, target bodies, prototypes, pointer cells, external providers, Source contents, Original runtime, game behavior, build/test harness or Ghidra mutations were examined or executed by this packet. Its evidence does not establish an ABI, calling convention, authenticated CRT/OS behavior, register preservation across calls, a binary replacement, or whole-function runtime equivalence.
