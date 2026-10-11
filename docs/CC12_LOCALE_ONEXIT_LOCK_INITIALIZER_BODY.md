# Locale onexit target: three saved body ranges

This packet establishes saved Native evidence for exactly three disjoint ranges at `C11B5E`: `C11B5E..C11BE6` (137 bytes, 47 instructions), `C11BF8..C11C01` (10 bytes, 4 instructions), and `C11C03..C11C17` (21 bytes, 5 instructions). The total is 168 bytes and 56 whole instructions. `C11BE7..C11BF7` and `C11C02` remain unread. The saved name `LIBCRT_unmatched_00c11b5e` and the packet's initializer label establish no routine, locking, provider, CRT or ABI meaning.

Two complete per-byte metadata sweeps preceded Root's independent boundary gate. Root admitted paired exact reads of the three ranges, one context-zero saved-text sweep over the 56 starts, and one complete closing metadata sweep. All three complete sweeps agree at epoch 98/count 64,730 under `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. They preserve all four listing aliases, complete three-range ownership, flow flags and address scalars. Complete automatic target objects remain opaque.

The physical bytes agree between reads. An independent opcode/ModRM/SIB/displacement decoder, actual detailed Capstone 5.0.7, and saved instruction text agree on every unit. Each range was decoded separately at its actual base; no gap bytes were supplied.

| Address | Physical bytes | Independent decode, matching saved text |
|---|---|---|
| 00C11B5E | 6A 0C | PUSH 0xc |
| 00C11B60 | 68 78 36 E0 00 | PUSH 0xe03678 |
| 00C11B65 | E8 96 60 FF FF | CALL 0x00c07c00 |
| 00C11B6A | 33 FF | XOR EDI,EDI |
| 00C11B6C | 47 | INC EDI |
| 00C11B6D | 89 7D E4 | MOV dword ptr [EBP + -0x1c],EDI |
| 00C11B70 | 33 DB | XOR EBX,EBX |
| 00C11B72 | 39 1D BC E1 09 01 | CMP dword ptr [0x0109e1bc],EBX |
| 00C11B78 | 75 18 | JNZ 0x00c11b92 |
| 00C11B7A | E8 E9 3F FF FF | CALL 0x00c05b68 |
| 00C11B7F | 6A 1E | PUSH 0x1e |
| 00C11B81 | E8 22 3E FF FF | CALL 0x00c059a8 |
| 00C11B86 | 68 FF 00 00 00 | PUSH 0xff |
| 00C11B8B | E8 C3 9E FE FF | CALL 0x00bfba53 |
| 00C11B90 | 59 | POP ECX |
| 00C11B91 | 59 | POP ECX |
| 00C11B92 | 8B 75 08 | MOV ESI,dword ptr [EBP + 0x8] |
| 00C11B95 | 8D 34 F5 78 64 E1 00 | LEA ESI,[ESI*0x8 + 0xe16478] |
| 00C11B9C | 39 1E | CMP dword ptr [ESI],EBX |
| 00C11B9E | 74 04 | JZ 0x00c11ba4 |
| 00C11BA0 | 8B C7 | MOV EAX,EDI |
| 00C11BA2 | EB 6E | JMP 0x00c11c12 |
| 00C11BA4 | 6A 18 | PUSH 0x18 |
| 00C11BA6 | E8 6F 2C FF FF | CALL 0x00c0481a |
| 00C11BAB | 59 | POP ECX |
| 00C11BAC | 8B F8 | MOV EDI,EAX |
| 00C11BAE | 3B FB | CMP EDI,EBX |
| 00C11BB0 | 75 0F | JNZ 0x00c11bc1 |
| 00C11BB2 | E8 D4 DF FE FF | CALL 0x00bffb8b |
| 00C11BB7 | C7 00 0C 00 00 00 | MOV dword ptr [EAX],0xc |
| 00C11BBD | 33 C0 | XOR EAX,EAX |
| 00C11BBF | EB 51 | JMP 0x00c11c12 |
| 00C11BC1 | 6A 0A | PUSH 0xa |
| 00C11BC3 | E8 59 00 00 00 | CALL 0x00c11c21 |
| 00C11BC8 | 59 | POP ECX |
| 00C11BC9 | 89 5D FC | MOV dword ptr [EBP + -0x4],EBX |
| 00C11BCC | 39 1E | CMP dword ptr [ESI],EBX |
| 00C11BCE | 75 2C | JNZ 0x00c11bfc |
| 00C11BD0 | 68 A0 0F 00 00 | PUSH 0xfa0 |
| 00C11BD5 | 57 | PUSH EDI |
| 00C11BD6 | E8 78 5A 00 00 | CALL 0x00c17653 |
| 00C11BDB | 59 | POP ECX |
| 00C11BDC | 59 | POP ECX |
| 00C11BDD | 85 C0 | TEST EAX,EAX |
| 00C11BDF | 75 17 | JNZ 0x00c11bf8 |
| 00C11BE1 | 57 | PUSH EDI |
| 00C11BE2 | E8 E1 81 FE FF | CALL 0x00bf9dc8 |
| 00C11BF8 | 89 3E | MOV dword ptr [ESI],EDI |
| 00C11BFA | EB 07 | JMP 0x00c11c03 |
| 00C11BFC | 57 | PUSH EDI |
| 00C11BFD | E8 C6 81 FE FF | CALL 0x00bf9dc8 |
| 00C11C03 | C7 45 FC FE FF FF FF | MOV dword ptr [EBP + -0x4],0xfffffffe |
| 00C11C0A | E8 09 00 00 00 | CALL 0x00c11c18 |
| 00C11C0F | 8B 45 E4 | MOV EAX,dword ptr [EBP + -0x1c] |
| 00C11C12 | E8 2E 60 FF FF | CALL 0x00c07c45 |
| 00C11C17 | C3 | RET |

At `C11BE2` and `C11BFD`, the physical E8 calls target `BF9DC8` and push return addresses `C11BE7` and `C11C02`. Both return addresses lie in excluded gaps. Saved default flow is UNCONDITIONAL_CALL, but a CALL_RETURN override produces effective CALL_TERMINATOR; both saved fallthrough fields are null. These are annotations, not proof of actual non-return. No padding, cleanup instruction or path through either gap is inferred. The remaining 54 units have no flow override and their saved flow classes/targets agree with physical decoding. The saved ranges do not establish complete actual control-flow closure.

The following effects are conditional on normal instruction completion under the saved default-32 decode and a 32-bit stack model. Every external call's possible returned registers, ESP, EBP, flags, segment state and memory are independently unknown. Every memory operand uses registers and memory at its own instruction time; no entry-memory snapshot, stable frame base or unaliased memory region is assumed. PUSH writes through current SS at ESP-4; POP reads through current SS at ESP before adding four. The POP ECX instructions do not prove argument cleanup after unknown external returns.

`C11B5E..C11B65`: PUSH 12; PUSH 00E03678; E8 C07C00 pushes return C11B6A. The second immediate is only a literal; no descriptor data was read. R65 is wholly unknown. No EBP/ESP frame relationship or helper ABI is inferred.

`C11B6A..C11B78`: After an actual R65 return, XOR EDI,EDI then INC EDI establishes EDI=1 locally; store through current SS:[EBP-1C]. XOR EBX,EBX establishes EBX=0 locally. CMP reads current DS:[0109E1BC] and compares it with EBX. JNZ reaches C11B92 if unequal; equal reaches C11B7A. The EBP-relative store may alias the absolute comparison cell; the comparison observes the actual current value.

`C11B7A..C11B91`: Conditional slow path calls C05B68, pushes 30, calls C059A8, pushes 255, calls BFBA53, then executes two POP ECX instructions. Each of the three returns independently destroys every assumed register/ESP/EBP/flag/memory value in the model. The final two POPs load through the last returned ESP and advance it by eight only relative to that unknown returned value. The direct branch to C11B92 retains locally established EDI=1/EBX=0 if complete; the call path does not. Those constants are not universal at the merge.

`C11B92..C11BA2`: Read X from current SS:[EBP+8] into ESI; LEA sets ESI=low32(8*X+00E16478), without reading that indexed memory. CMP then reads current DS:[ESI] and compares with current EBX. Equality reaches C11BA4. Inequality moves current EDI to EAX and jumps C11C12. The EBP-based load is not proved to be the original caller argument. No table type, bound, pointer validity or meaning of the comparison is established; later C11C12 can change EAX.

`C11BA4..C11BBF`: PUSH 24; call C0481A; after an actual return POP ECX, copy returned EAX into EDI, then compare EDI with returned EBX. Inequality reaches C11BC1. Equality calls BFFB8B; after that independent unknown return, write 12 through current DS:[EAX], XOR EAX to zero, and jump C11C12. Returned EBX is not assumed zero. EAX from BFFB8B is not authenticated as a valid pointer or status cell. The later call prevents a proof that the whole function returns zero.

`C11BC1..C11BCE`: PUSH 10; call C11C21; after an actual unknown return POP ECX, write current EBX through current SS:[EBP-4], then compare a fresh read from current DS:[ESI] with current EBX. Inequality reaches C11BFC. Equality reaches C11BD0. The immediately preceding EBP-relative write may alias the indexed comparison read. Neither ESI, EBX nor the compared memory is frozen from C11B9C.

`C11BD0..C11BE2`: PUSH 4000; PUSH current EDI; call C17653. If it returns, two POP ECX instructions advance that returned ESP by eight. TEST uses the returned EAX, which those POPs do not change. Nonzero EAX reaches C11BF8. Zero EAX reaches PUSH current EDI at C11BE1 and E8 BF9DC8 at C11BE2. C11BE2 pushes C11BE7. A physical return there reaches the unread BE7..BF7 gap. Saved CALL_RETURN/CALL_TERMINATOR metadata does not prove actual non-return.

`C11BF8..C11C01`: At C11BF8 write current EDI through current DS:[ESI], then JMP C11C03. The separate C11BFC entry pushes current EDI and calls BF9DC8 at C11BFD. C11BFD pushes C11C02. That byte is unread; no instruction, padding, cleanup or path through it is inferred. The C11BF8 store uses the ESI/EDI values actually current after the C17653 return, not values assumed preserved from earlier calls.

`C11C03..C11C17`: Write FFFFFFFE through current SS:[EBP-4]; call C11C18 at C11C0A. If it returns at C11C0F, load EAX through then-current SS:[EBP-1C]. Call C07C45 at C11C12, also reached by the direct jumps at C11BA2 and C11BBF. The final EBP-relative load need not address or retain the value written at C11B6D. Both calls and intervening writes may alter EBP, segment state and memory. If the C11C12 call returns at C11C17 with ESP=Q, C3 reads the return target from current SS:[Q] and sets ESP=Q+4 on normal completion. That target, returned registers/flags and relation to entry ESP remain unknown.

Only numeric operand locations were recovered. The literals `00E03678`, `0109E1BC` and `00E16478`, the pointer-shaped register values and the EBP-relative stores have no authenticated descriptor, table, status, argument or exception-frame interpretation. No callee, data cell, gap, prototype, decompile or continuation beyond the admitted ranges was queried.

One original second-pass metadata chunk failed while its local `.tmp` file was being atomically replaced, with Windows error 5. Its 46 complete HTTP responses, the preceding successful 64-response chunk, all command streams and the unchanged producer are retained. The underlying file-lock cause is unknown. A full second pass used distinct new names. Closing validation checks 504 authoritative responses plus those 110 retained earlier responses, totaling 614; all retained raw JSON/base64/digests agree where they overlap. No failed evidence was overwritten.

The [report](../reports/cc12_locale_onexit_lock_initializer_body.json) pins the full captures, manual and actual Capstone results, Root gate, 23 CLI command streams, nine immediate positive Git/physical inputs and six frozen Git objects. Strict original/copy core replays each perform one supplied ZIP read, one receipt write and three separately pinned pure helper executions, with no other audited effects or denials. The strict replay compares retained Capstone output with independent decoding; it does not rerun the Capstone engine. A guard-helper revision occurred before its first execution; the exact preimage and revision producer/receipt are retained. Post-commit delivery proof checks actual base/final Git objects and exact working/index/committed bytes for the two owned outputs. Earlier archives were not traversed and prior proofs remain unchanged.

Frozen worker base is `ff6ff37216e9460c7f99af17da4cee4d96ca4cf0`. Source IDs `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2` and `include=09add3be594e63edcce6a16ec041ed965441e898` establish worker revision identity only, with no Source census, implementation credit or current/future Root equality. No Source edit, build, test, executable probe, Ghidra mutation, Original game/OS/audio run or gameplay validation occurred. The startup/gameplay goal remains open. Further gaps, targets, providers or data require a separate Root scope decision.
