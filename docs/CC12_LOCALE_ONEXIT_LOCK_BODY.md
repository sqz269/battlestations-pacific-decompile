# Locale onexit lock-body evidence

The saved `00C11C21..00C11C51` body is 49 bytes containing 20 whole instructions. Two complete typed sweeps queried every address unit before any body byte or instruction-text read. Root independently admitted their complete partition. Two subsequent exact 49-byte reads agree, and one context-zero saved-text sweep covers only the admitted starts. Project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, epoch 98 and count 64,730 remained consistent. No target, data cell, provider or companion body was queried.

The saved function name `__lock`, prefix `void __cdecl __lock(int _File)`, complete ownership range, non-thunk status and false no-return flag remain metadata. They establish no argument meaning, locking behavior, actual CRT identity or ABI. All automatic target objects remain opaque. This packet begins from the accepted `BFBA68` physical PUSH 8 / CALL `C11C21` / POP ECX / RET evidence; it adds no claim about that caller's returned state.

| Start | Physical bytes | Independently decoded instruction |
|---|---|---|
| C11C21 | 55 | PUSH EBP |
| C11C22 | 8B EC | MOV EBP, ESP |
| C11C24 | 8B 45 08 | MOV EAX, [EBP+8] |
| C11C27 | 56 | PUSH ESI |
| C11C28 | 8D 34 C5 78 64 E1 00 | LEA ESI, [EAX*8+E16478] |
| C11C2F | 83 3E 00 | CMP dword [ESI], 0 |
| C11C32 | 75 13 | JNZ C11C47 |
| C11C34 | 50 | PUSH EAX |
| C11C35 | E8 24 FF FF FF | CALL C11B5E |
| C11C3A | 85 C0 | TEST EAX, EAX |
| C11C3C | 59 | POP ECX |
| C11C3D | 75 08 | JNZ C11C47 |
| C11C3F | 6A 11 | PUSH 17 |
| C11C41 | E8 C3 9D FE FF | CALL BFBA09 |
| C11C46 | 59 | POP ECX |
| C11C47 | FF 36 | PUSH dword [ESI] |
| C11C49 | FF 15 18 22 CE 00 | CALL dword [CE2218] |
| C11C4F | 5E | POP ESI |
| C11C50 | 5D | POP EBP |
| C11C51 | C3 | RET |

The SIB byte `C5` at `C11C28` selects scale eight, EAX index and no base; the displacement is `E16478`. The `FF 36` ModRM selects PUSH through current ESI. The `FF 15` ModRM selects an indirect near CALL through the absolute memory cell `CE2218`. Neither that cell's contents nor its possible import/provider identity was read. Saved default/effective flow kinds agree throughout, with NONE overrides and no fallthrough overrides. At `C11C49`, however, the default target array is empty while the saved effective array contains `EXTERNAL:0000015b`. This scalar annotation is retained separately from the physical memory-dispatch operand; it is not an observed executable target.

For a normally executing prefix, let entry ESP be `S`, initial EBP/ESI be `B0`/`I0`, and each load observe memory at its actual instruction time. `C11C21` writes `B0` at `S-4`; EBP becomes `S-4`; `C11C24` loads EAX from `S+4`; `C11C27` writes `I0` at `S-8`. ESI then becomes `8*EAX+E16478` modulo 32 bits. `C11C2F` compares the current dword at that address, including any effects of earlier stack writes if addresses alias. JNZ selects `C11C47` if the comparison is nonzero.

On the zero path, PUSH EAX writes at `S-12`; CALL `C11B5E` pushes `C11C3A` at `S-16`. If that external call returns to the continuation, its entire returned ESP/register/flag/memory state is unknown. TEST uses its returned EAX; POP ECX reads through its returned ESP and increments that ESP by four without changing flags. JNZ therefore tests the returned EAX, conditional on these instructions completing. If zero, PUSH 17 and CALL `BFBA09` follow; that call pushes `C11C46`. Its possible returned state is independently unknown, and the next POP ECX again reads through the actual returned ESP.

Every path reaching `C11C47` rereads through the then-current ESI before decrementing ESP and writing the pushed word. It does not reuse the earlier comparison value or assume ESI survived a call. `C11C49` subsequently reads the then-current dword at `CE2218`, which may reflect the preceding push if addresses alias, then pushes `C11C4F` and dispatches through the loaded target. No external return, register preservation, stack cleanup or unchanged memory is assumed.

If that indirect call returns at `C11C4F` with ESP `Q`, POP ESI reads at `Q`, POP EBP reads at `Q+4`, and RET reads the return target at `Q+8`, leaving ESP `Q+12` if all complete. Each load is at its actual time. This establishes no equality with entry values, no eventual target identity, and no caller cleanup contract. The final `C3` has no immediate adjustment. Faults, exceptions, non-returning paths and execution are not validated.

The [report](../reports/cc12_locale_onexit_lock_body.json) pins raw HTTP JSON/base64/digests, all four listing aliases at every address, complete owner metadata, flow scalars, byte/text streams, Root's gate, finite Git inputs and pure replay helpers. Strict original/copy core replays each read only the supplied ZIP and write one receipt after helper pins were verified. The proof retains the preliminary successful core archive separately; the final core archive corrects only the delivery object's record-count label. No failed or interrupted Native capture occurred. A post-commit delivery proof checks actual base/final Git objects and exact current/staged/committed bytes for the two owned outputs.

Frozen worker base is `2b72d2d0b958f8d86c52d1d047ae7c7238b35d74`; Source tree IDs are `src=af8efebaa30365fa0f7f2f591c45861a55b4b1b2` and `include=09add3be594e63edcce6a16ec041ed965441e898`. These are worker identity only, with no Source census or current/future Root equality. No Source implementation, build, tests, executable probe, Ghidra mutation, Original game/OS/audio run or gameplay validation occurred. The continuing startup/gameplay goal remains open. Any target or data work requires a separate Root scope decision.
