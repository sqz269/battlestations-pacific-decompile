# CC12 C04EFB direct-target body metadata and physical evidence

The admitted range `00C04EFB–00C04F66` contains 48 instructions and 108 bytes. Both complete metadata walks and paired byte/context captures agree. All physical fields, operands, register access and flags agree with fresh detailed Capstone 5.0.7. A separately preserved retry corrects three decoder group expectations. The saved name `BSP_CRT_PointerDecodeModuleGate` remains metadata, without a recovered library identity, calling convention or function prototype.

The current accepted entry reports carry the incoming `E8 D3 FE FF FF` at `00C05023`: return offset `00C05028` plus signed displacement -301 gives numeric target `00C04EFB`. Previous proof archives were not reopened.

## Admission and original evidence

Actual Main is frozen at `7f17b3d462f9d2de5e94df22a04b855b73d702d9`, with recorded ancestry permitting later Main advancement. Eight positive inputs are current C04EFB ENTRY/ENTRY_PRIMARY document/report pairs, target configuration and three CLI sources. Fourteen actual commit/tree/blob objects establish their frozen membership, working-byte LF equality and exact stage-zero index blobs. Source subtrees are identities only.

Each metadata walk restarts at `00C04EFB` and advances by newly captured InstructionDB lengths, independently covering 48 starts and exactly 108 bytes. Four redundant aliases agree at every start. Each paired original reply is identical, including the complete owner, saved flags, separate default/effective flow and automatic companions. The saved owner is one complete 108-byte range, no_return=false and nonthunk. Numeric end cursor `00C04F67` is unqueried. Phase 1 retained 100 CLI invocations and 492 original HTTP responses.

Root's complete-original gate passed on its first child execution. Before phase 2, all 1,268 Root artifacts were copied and verified: 1,257 metadata preimages, ten listed top files and the control manifest. Every preimage equals its worker original. Phase 2 retained seven CLI invocations and 70 HTTP responses for paired exact-range reads, paired context zero at the 48 admitted starts, closing typed metadata and count bookends. All three originals per start agree, totaling 144 typed responses.

All 107 Native invocations physically reread each of the three CLI sources before and after execution, retaining 214 separate observations and content-addressed identical raw payloads. Project/program remained `C:/Users/sqz269/bsp.gpr` and `/battlestationspacific.exe`, x86:LE:32:default, modification number 98 and function count 64,730.

## Physical instructions

| Address | Bytes | Saved context-zero text |
| --- | --- | --- |
| `00C04EFB` | `55` | `PUSH EBP` |
| `00C04EFC` | `8BEC` | `MOV EBP,ESP` |
| `00C04EFE` | `51` | `PUSH ECX` |
| `00C04EFF` | `51` | `PUSH ECX` |
| `00C04F00` | `53` | `PUSH EBX` |
| `00C04F01` | `56` | `PUSH ESI` |
| `00C04F02` | `33F6` | `XOR ESI,ESI` |
| `00C04F04` | `8D45FC` | `LEA EAX,[EBP + -0x4]` |
| `00C04F07` | `46` | `INC ESI` |
| `00C04F08` | `33DB` | `XOR EBX,EBX` |
| `00C04F0A` | `50` | `PUSH EAX` |
| `00C04F0B` | `8975F8` | `MOV dword ptr [EBP + -0x8],ESI` |
| `00C04F0E` | `895DFC` | `MOV dword ptr [EBP + -0x4],EBX` |
| `00C04F11` | `E84B6CFFFF` | `CALL 0x00bfbb61` |
| `00C04F16` | `837DFC05` | `CMP dword ptr [EBP + -0x4],0x5` |
| `00C04F1A` | `59` | `POP ECX` |
| `00C04F1B` | `7E04` | `JLE 0x00c04f21` |
| `00C04F1D` | `8BC6` | `MOV EAX,ESI` |
| `00C04F1F` | `EB42` | `JMP 0x00c04f63` |
| `00C04F21` | `57` | `PUSH EDI` |
| `00C04F22` | `53` | `PUSH EBX` |
| `00C04F23` | `FF155C21CE00` | `CALL dword ptr [0x00ce215c]` |
| `00C04F29` | `8B703C` | `MOV ESI,dword ptr [EAX + 0x3c]` |
| `00C04F2C` | `03F0` | `ADD ESI,EAX` |
| `00C04F2E` | `66395E06` | `CMP word ptr [ESI + 0x6],BX` |
| `00C04F32` | `0FB74614` | `MOVZX EAX,word ptr [ESI + 0x14]` |
| `00C04F36` | `8D7C3018` | `LEA EDI,[EAX + ESI*0x1 + 0x18]` |
| `00C04F3A` | `7623` | `JBE 0x00c04f5f` |
| `00C04F3C` | `57` | `PUSH EDI` |
| `00C04F3D` | `68B89FD600` | `PUSH 0xd69fb8` |
| `00C04F42` | `E8D9090000` | `CALL 0x00c05920` |
| `00C04F47` | `85C0` | `TEST EAX,EAX` |
| `00C04F49` | `59` | `POP ECX` |
| `00C04F4A` | `59` | `POP ECX` |
| `00C04F4B` | `740E` | `JZ 0x00c04f5b` |
| `00C04F4D` | `0FB74606` | `MOVZX EAX,word ptr [ESI + 0x6]` |
| `00C04F51` | `43` | `INC EBX` |
| `00C04F52` | `83C728` | `ADD EDI,0x28` |
| `00C04F55` | `3BD8` | `CMP EBX,EAX` |
| `00C04F57` | `72E3` | `JC 0x00c04f3c` |
| `00C04F59` | `EB04` | `JMP 0x00c04f5f` |
| `00C04F5B` | `8365F800` | `AND dword ptr [EBP + -0x8],0x0` |
| `00C04F5F` | `8B45F8` | `MOV EAX,dword ptr [EBP + -0x8]` |
| `00C04F62` | `5F` | `POP EDI` |
| `00C04F63` | `5E` | `POP ESI` |
| `00C04F64` | `5B` | `POP EBX` |
| `00C04F65` | `C9` | `LEAVE` |
| `00C04F66` | `C3` | `RET` |

The report retains every prefix, opcode byte, ModR/M and SIB field, displacement/immediate width and signed value, operand size/access/default segment, explicit and implicit register access, flag effect and decoder group. The initial complete manual record was written before the first Capstone import and construction. Original detailed decoder outputs and both comparisons remain pinned.

Saved default/effective flow classes agree: 38 fallthroughs, two direct calls, one computed call, four conditional jumps, two direct jumps and one terminator. Overrides are NONE. At `00C04F23`, default targets are empty while effective targets contain an EXTERNAL companion; the separate arrays and unauthenticated saved label remain intact.

The direct E8 at `00C04F11` has return offset `00C04F16`, signed displacement -37,813 and numeric target `00BFBB61`. The E8 at `00C04F42` has return offset `00C04F47`, displacement 2,521 and numeric target `00C05920`. The computed call at `00C04F23` reads its target through current DS offset `00CE215C`; the cell value and target are unknown. Callee bodies, providers and prototypes remain unqueried.

## Current-state contract

Renew current SS.B=1 at each PUSH, CALL, POP, LEAVE and RET. With that predicate and normal successful completion, each PUSH/near CALL decrements current ESP by four and writes through current SS; CALL writes its encoded return offset. Each POP/near RET reads four bytes at its own current SS:ESP and increments ESP by four. These local effects do not establish aggregate stack balance, caller-return identity, arguments or callee-saved registers.

For LEAVE at `00C04F65`, use current EBP=B with the saved 32-bit operand width and renewed current SS.B=1: ESP becomes B, EBP receives the current SS memory value at offset B, and ESP becomes `(B+4) mod 2^32`. No relation to an earlier EBP/ESP is assumed. RET C3 has no encoded immediate adjustment, no ordinary fallthrough to `00C04F67`, and an unknown current stack target.

Explicit EBP-4/EBP-8 memory operands default to current SS. EAX+3C, ESI+6, ESI+14 and absolute CE215C default to current DS. Every effective offset is recomputed from current registers. No flat DS=SS equality or non-alias relation is established. LEA computes offsets only: EBP-4 at `00C04F04`, and EAX+ESI+18 with SIB scale one at `00C04F36`; neither reads a memory value or applies a segment base.

Prefix 66 at `00C04F2E` makes CMP a 16-bit word/BX operation with 32-bit address size. Both MOVZX forms read a word and zero-extend it into EAX. The 83 forms sign-extend their encoded imm8 to the 32-bit operation. AND `[EBP-8],0` still reads and writes memory despite its zero result; it does not bypass access checks or establish a return-value meaning.

XOR, AND and TEST set SF/ZF/PF from the result, reset CF/OF and leave AF undefined; TEST stores no result. ADD/CMP update arithmetic flags, with CMP storing no result. INC modifies OF/SF/ZF/AF/PF while preserving CF. JLE tests `ZF || (SF != OF)`, JBE tests `CF || ZF`, JE tests ZF and JB tests CF.

On uninterrupted normal execution, self-XOR clears ESI or EBX; the intervening LEA permits INC ESI to produce one. CMP `00C04F16` supplies JLE through a POP; CMP `00C04F2E` supplies JBE through MOVZX and LEA; TEST `00C04F47` supplies JE through two POPs; CMP `00C04F55` supplies JB directly. These intervening instructions preserve comparison flags. Every opaque call breaks cross-call state assumptions: returned registers, flags, memory and segments are unknown. The literal `00D69FB8`, numeric cell offset and memory-offset pattern do not authenticate strings, imports, executable-header layouts, providers or a library routine.

## Failure history and validation boundary

The first decoder child recorded three differences only: actual Capstone includes not64bitmode for LEA at `00C04F04`/`00C04F36` and LEAVE at `00C04F65`. Encoding fields, operands, accesses, registers and flags already agreed. All ten original producer/prepin/manual/actual/comparison/process artifacts were physically copied before a separate retry changed only those group expectations. The retry passed all 48 units. Its access to the earlier actual result is explicit; it is not presented as another independent first attempt. The failed child counts once as a decoder failure and once as a captured producer-process failure.

One worker template display exceeded its output limit; missing lines were subsequently paged. It is recorded as a display incident, with no inspection-command, Native, decoder or GPR effect. Root separately records one tool-only schema-inspection KeyError, before its successful first gate child. Conversation incident records are labeled transcriptions; they are not fabricated subprocess originals. Root gate/display failures are zero. Current accepted ENTRY/PRIMARY histories and earlier C04FDE body histories remain separately scoped.

No Source reconstruction or absence census, Ghidra mutation, build, new test, probe, Original/game execution or OS/audio experiment occurred. `00C04F67`, data-cell contents, immediate referents, callee bodies, prototypes and providers remain unqueried. This is static physical and provenance evidence, without ABI, reachability or game validation.

The private proof binds the actual final commit, its single frozen parent and exactly two public outputs with working/index blobs. A fully read, pinned helper is supplied outside the ZIP. Independent original/copy strict receipts allow one ZIP read, one execution of the exact precompiled helper and one exclusive receipt write, denying all other audit events. The helper executes no archived code, imports no decoder under the hook and opens no earlier archive.
