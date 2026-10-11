# FindHandler outbound jump target: bounded saved body

The two saved ranges for owner `00C07A75` contain 12 physical instructions (44 bytes). The final `E8` is a direct call to numeric `00C04DE2`, with encoded return cursor `00C07AA8`. Saved `CALL_RETURN` / `CALL_TERMINATOR` metadata and null saved fallthrough do not prove nonreturn.

Frozen Main: `508e69254ec6fc2bab6b3a2f5c48b99068330ad8`. The only positive inputs are the accepted ENTRY/ENTRY_PRIMARY document/report pairs, target configuration, and three existing CLI source files (eight blobs). Actual commit/tree/blob payloads and raw working files are retained; LF-normalized working bytes equal their blobs. `src`/`include` are tree identities only. No earlier archive was opened.

Two independent metadata-only walks began at each saved range start and advanced only by just-captured InstructionDB lengths. All four listing aliases, complete owner/ranges, and paired original response bytes agree. Root admitted the whole 44-byte body before bytes or context text were requested. Paired exact reads, paired context-zero text at the 12 starts, and closing exact typed responses agree. Total: 37 Native CLI commands, 172 original HTTP responses, epoch 98, count 64730; existing project `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`.

Admitted ranges are `00C07A75..00C07A94` (32 bytes) and `00C07A9C..00C07AA7` (12 bytes). Gap `00C07A95..00C07A9B` and range-end cursors `00C07A95`/`00C07AA8` were never queried or classified. All external targets and data/provider/descriptor cells remain unqueried.

| Start | Physical bytes | Saved context-zero text |
| --- | --- | --- |
| 00C07A75 | `6a08` | `PUSH 0x8` |
| 00C07A77 | `685835e000` | `PUSH 0xe03558` |
| 00C07A7C | `e87f010000` | `CALL 0x00c07c00` |
| 00C07A81 | `e8a8d7ffff` | `CALL 0x00c0522e` |
| 00C07A86 | `8b4078` | `MOV EAX,dword ptr [EAX + 0x78]` |
| 00C07A89 | `85c0` | `TEST EAX,EAX` |
| 00C07A8B | `7416` | `JZ 0x00c07aa3` |
| 00C07A8D | `8365fc00` | `AND dword ptr [EBP + -0x4],0x0` |
| 00C07A91 | `ffd0` | `CALL EAX` |
| 00C07A93 | `eb07` | `JMP 0x00c07a9c` |
| 00C07A9C | `c745fcfeffffff` | `MOV dword ptr [EBP + -0x4],0xfffffffe` |
| 00C07AA3 | `e83ad3ffff` | `CALL 0x00c04de2` |

Every opcode, length, prefix absence, ModRM field, SIB absence, displacement, immediate, and relative target agrees between the manual decoder and actual detailed Capstone 5.0.7. Default memory segments, operand access, and instruction flag effects are retained and independently checked in pure replay. The two exact byte blocks have SHA-256 `eb271829478822604a351ef787f25a32836c966b6cac718f8026d6db08275675` and `16952dddbc793efd41a2c94718e46f9cec2fee1ac42f3ea55a2e3371d4a184c2`.

Unprefixed x86-32 instruction interpretation in the saved language. Live segment descriptors and execution are unverified. Effects below require successful normal execution of the stated instructions.

Accepted E9 at C07AF3 transfers numerically to C07A75 without changing ESP or pushing a return address. This body does not infer a call frame from that incoming jump.

For each PUSH or near CALL, impose the actual instruction-time SS.B=1 condition anew before using ESP arithmetic: new ESP=(current ESP-4) mod 2^32 and a dword write through current SS at that offset. Unknown post-call SS attributes cannot inherit a prior stack-size assumption. These equations do not assert a flat segment, valid mapping, successful access, or live 32-bit stack.

C07A75 sign-extends imm8 08 to dword 00000008; C07A77 pushes literal 00E03558. With a current 32-bit stack and uninterrupted normal execution, the two successive dword writes precede the first CALL return-address write. No pointer, argument, descriptor, or object meaning is assigned to 00E03558.

C07A7C E8 has signed rel32 +383, next/encoded return C07A81, target C07C00. The push uses the instruction-time SS and stack-size condition. Target bytes, prototype, convention, effects, and return are unknown.

If control reaches C07A81 after the opaque call, every GPR including EBP/ESP, segment selector and hidden attributes, EFLAGS, memory, and x87 state is unknown again. Its E8 rel32 -10328 has next/encoded return C07A86 and target C0522E. Apply the current SS.B=1 condition anew to any ESP stack equation; return and cleanup are unproved.

If control reaches C07A86 after that opaque call, use fresh unknown current state again. 8B 40 78 reads a dword through current DS at address-size-32 offset (old EAX+0x78) mod 2^32, then writes EAX with that value. There is no segment override; neither the original EAX value nor the memory provenance is known. MOV leaves EFLAGS unchanged.

C07A89 TEST EAX,EAX reads the loaded EAX without modifying it. CF=OF=0; SF/ZF/PF follow the result EAX, AF is undefined. C07A8B JE has rel8 +22 from C07A8D to C07AA3 and tests ZF without changing EFLAGS or the stack. If these instructions execute normally without an intervening state change, the zero case skips AND, indirect CALL, JMP, and the later MOV.

On the nonzero fallthrough, C07A8D 83 /4 sign-extends imm8 00 and performs a dword read-modify-write at current SS:(EBP-4) using 32-bit effective-address arithmetic. This reads the prior dword even though the successful result is zero; it may fault. EAX/EBP/ESP are unchanged by the instruction. Successful AND sets CF=OF=SF=0, ZF=PF=1; AF is undefined. Explicit EBP addressing does not require SS.B=1; that condition is for implicit stack-pointer arithmetic.

C07A91 FF /2 reads the instruction-time EAX as its near-call target before its return-address stack write; the register value is still the loaded nonzero value along the uninterrupted normal path. Encoded return is C07A93. The prior AND can alias the original load cell or later stack writes without changing EAX itself. No distinctness of these memory cells is established. Apply current SS.B=1 anew for ESP-4 and current SS for the push; target behavior remains opaque.

If control reaches C07A93 after the opaque computed call, every GPR/ESP/EBP, segment and attributes, EFLAGS, memory, and x87 state is unknown again. EB 07 jumps from numeric next C07A95 to C07A9C; it does not change ESP or push a return address. C07A95 is arithmetic only, unqueried and unclassified.

C07A9C C7 /0 writes literal FFFFFFFE to current SS:(EBP-4) with a 32-bit effective-address offset, without a destination read. It preserves EFLAGS and registers. Post-call EBP and SS need not match the earlier AND, so equality of their cells is unproved; arbitrary partial or complete aliasing with other memory/stack writes remains possible. Store completion and later CALL access are separate ordered events.

C07AA3 E8 3A D3 FF FF is a physical direct CALL: signed rel32 -11462, numeric target C04DE2, next/encoded return C07AA8. Under the newly imposed actual current SS.B=1 condition, it writes C07AA8 through current SS at ESP-4 and updates ESP. CALL itself preserves arithmetic EFLAGS before transfer. C07AA8 is an unqueried cursor, not an admitted instruction or continuation. Callee behavior, return, and all state on any later return are unconstrained.

At C07AA3 saved default UNCONDITIONAL_CALL plus CALL_RETURN yields effective CALL_TERMINATOR. Both saved default/effective fallthrough are null; this is retained metadata, not physical proof of nonreturn. The owner saved name terminate, no_return=false, and nonthunk status convey no established library, EH, TLS, runtime, or ABI meaning.

Memory dependencies use each instruction-time segment and register state in execution order. No alias disjointness, pointer validity, exception route, callee preservation, stack cleanup, nonreturn, or successful whole-path execution is inferred. Flags described are instruction effects only, never promises across opaque calls.

Evidence limits and failure retention: two strict replay invocations failed before the final correction. The first retained real empty stdout/stderr and no receipt; its denial telemetry is unavailable. A separately pinned diagnostic guard produced a real failure receipt with AssertionError at original helper line212 and one ZIP read/one exact exec/one exclusive receipt/zero denials. An overbroad numeric template replacement had changed the correct Root artifact count391 into371; only that proven count assertion was corrected. The final guard catches validator exceptions into its exclusive receipt without weakening its audit restrictions. Failed proof bytes are included only as an opaque pinned object, without recursively opening them; all failed helper/producer preimages and original invocation streams are retained. Failed commit d880edb1 remains reachable on its dedicated attempt01 branch; the final two-output commit retains the original frozen parent through an unpublished amend. The worker had one offline assertion failure because Main advanced after the worktree froze; it occurred before any Native query. The failed producer, preexecution pins, and original Git command streams survive. Initial top-level collector output is a provenance-labeled copy of the received tool output; separate original local collector streams were not captured and are not synthesized. The corrected collector, body capture, and decoder have original successful process streams. Root's current metadata gate passed first execution, with one separately disclosed read-only schema inspection KeyError and one truncated inspection display. Their original combined tool outputs remain in the conversation; no separate local original streams are claimed. The accepted entry input retains its older two Root gate failures, three Root inspection failures, reconstructed first producer, and missing first separate local stream limitation unchanged. These are distinct from the current gate. A preliminary handoff arithmetic count was corrected from 173 to the actual 172 HTTP records.

Private proof under `local/cc12_findhandler_outbound_jump_target_body_metadata/` contains 385 Root metadata preimages, 391 Root gate artifacts, exact command/request/response streams, physical helpers pinned before execution, 20 selected positive Git object payloads (14 frozen plus six final), alongside explicitly negative failed-attempt preimages, and the exact two final output blobs. `proof.final.zip` and its separately created byte-identical copy are replayed in fresh processes using a reviewed local pure helper. The audit admits only one supplied ZIP read, that exact helper's execution, and one exclusive receipt write; every other event is denied, with `denied[]` required empty. Capstone is captured before the hook; archived producer code is never executed. Actual replay receipts and original streams establish replay outcome.

This is a read-only physical evidence packet. It supplies no Source implementation, build, fixture, native probe, Original/game/OS/audio execution, ABI compatibility, or runtime validation. There is no Ghidra mutation. Further targets or data meaning need a new bounded gate.
