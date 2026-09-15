# Native NLG descriptor and handler-transfer frontier

This packet retains complete PE/live bodies for `C16879 __NLG_Notify` (31 bytes) and `C0DCCD _EH4_TransferToHandler` (25 bytes), plus the essential second writer `C16870 __NLG_Notify1` (nine-byte prefix). Notify1 reaches the same 20-byte tail at C16884..C16897: its complete path has 29 instruction bytes, while its current saved function body contains only the prefix. Unique owned code coverage is 65 bytes. The actual descriptor E16830..E1683F is 16 mutable bytes. Existing library names were preserved; no source or Ghidra metadata was changed.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. Verified BSP CLI batches checked the configured bsp project, `/battlestationspacific.exe`, language and image base. Fresh full bytes agree with the installed PE, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. BS `4b524e551c0f86956ec118e60b4d304a61ab817d` (119 artifacts) and BU `0cbad82f4fdc1551be711fc61e9ea07d5b731bba` (251 artifacts) were fully verified and retained unchanged. Their complete parent/dispatcher/frame evidence is reused, not rediscovered as new ownership.

## Actual descriptor writes and initialization boundary

PE/live database bytes are `{19930520h,0,0,0}` at E16830. The address is in writable `.data` (section characteristics C0000040), not the supported read-only native table area. Both entry points bind EBX to the same absolute address. The common tail performs these three stores in order:

| Native site | Descriptor field | Exact value |
| --- | --- | --- |
| C16884 | +8 | Notification code in ECX |
| C16887 | +4 | Raw incoming EAX notification value |
| C1688A | +Ch | Actual incoming EBP frame value |

Neither path writes +0, validates the descriptor, allocates it, resets it, locks it or supplies an initialization owner. These are separate ordinary DWORD stores; no atomic snapshot or notification callback is present. The later PUSH EBP/ECX/EAX and POP EAX/ECX/EBP sequence is preserved instruction behavior, not a call to an external notification service.

Bounded current xrefs identify base references in C16870 and C16879 and the three common-tail writes above. A whole-file literal search for E16830/+4/+8/+Ch finds only the two EBX-immediate operands (C16873 and C1687C). Relocation-directory presence/results are recorded separately; the absence of relocation entries is not used as proof that no writer exists. These observations do not exclude arbitrary indirect writers, debugger observers or consumers outside the analyzed program. PE initialization bytes establish image contents, not a source-owned runtime mapping or a proven current process value. No separate initialization/writer body for the magic DWORD was found by these bounded checks.

The descriptor's +4 field must remain a raw DWORD: at the second Notify1 call in C07B10 it receives the actual funclet's returned EAX. It is not universally a validated executable destination pointer. No TLS/private descriptor substitution is supported by these shared process-address writes.

## C16879 and the shared Notify1 entry

C16879 takes EAX as the raw notification value, EBP as the actual frame and one code DWORD at entry ESP+4. It pushes EBX then ECX, binds EBX=E16830, and reloads the code from adjusted ESP+Ch into ECX. The shared tail writes code, EAX, EBP in that order. It pushes EBP/ECX/EAX, pops EAX/ECX/EBP, then pops the originally saved ECX and EBX and executes RET4. The original code word is consumed. EAX, EBP, EBX, ECX, EDX, ESI, EDI and arithmetic flags are unchanged on normal return; temporary register values and stack accesses remain part of the native schedule. A fault during a later store can leave earlier descriptor stores visible; no rollback exists.

C16870 instead pushes EBX and ECX, binds the same EBX address, and JMPs to C16884. It uses incoming ECX as the code and never loads the caller's code stack word. Nevertheless the shared RET4 still consumes that one stack word. The shared tail is not an independent function call or a replacement entry at C16879. This distinction is essential to source stack and register ABI.

Fresh complete C07B10[76] caller evidence establishes both Notify1 sites. At C07B2E, EAX is the actual funclet entry, EBP is registration+Ch, and ECX plus the stacked word carry the caller's third code argument. C07B35 then CALLs EAX. After balanced normal return, C07B50 records the actual returned EAX and current funclet-frame EBP; ECX is the original code except 100h is converted to 2. The matching code is pushed even though Notify1 consumes rather than reads it. NLG preserves the flags supplied by this caller; in particular it must not insert its own flag-producing code test.

Other current direct Notify consumers are C0DBC4/C167C9 (code101h, actual cleanup value/frame), C03570 (code0, saved jump-buffer value/frame), C0DCCD (code1, handler/frame), and an undefined-parent callsite BFB993 (code1 and EAX loaded from `[EDI+ECX*4+8]`). The full C03570[173] caller is retained, but its longjmp/unwind ownership is outside this packet. BFB993 has only a bounded, explicitly partial argument/call window; its containing function and earlier EBP provenance remain unresolved. An initial misaligned BFB98B window was rejected and retained as failure history; only the corrected BFB98D[11] PUSH/MOV/CALL window supports this callsite contract.

## C0DCCD transfer and stack continuation

Native inputs are ECX actual selected handler and EDX actual establisher frame. The body first installs EBP=EDX, keeps the handler in ESI and EAX, pushes code1 and directly calls actual C16879 at C0DCD5. After its RET4, ESP is back to the transfer entry value. It zeros EAX, EBX, ECX, EDX and EDI in that order, preserving ESI=handler and EBP=frame, then JMPs ESI at C0DCE4. At the jump CF/OF/SF are zero, ZF/PF one, AF undefined from the final XOR. There is no RET, validation, stack restoration or cookie check in this body.

The complete retained C07C90 dispatcher reaches this helper by JMP C07E00, with ECX reloaded from the selected scope handler and EDX=registration+10h. Thus the actual dispatcher stack remains present. A source caller that introduces an ordinary additional call/return frame does not automatically satisfy the handler continuation. The native handler must own its required stack restoration and nonlocal continuation. Descriptor writes occur before the zeroing and handler jump; a notification fault does not guarantee transfer, and a handler exception does not cause any compensating descriptor reset. No general callback or synthetic frame can replace this contract.

## Source availability and smallest missing owner

A bounded current source/header/CMake scan found no E16830/NLG descriptor or helper implementation. The existing `GameNativeReadOnlyData` service explicitly maps only CE2000..E07B23, verifies read-only `.rdata`, and protects committed pages read-only. Its header excludes mutable globals/owners. E16830 is outside that range and requires writes; `GameNativeDataReservation`/bootstrap shares the same span validation. Retained complete source/header hashes document this actual limitation. Reusing that mapper or changing its protections is not a ready descriptor provider.

The smallest missing owner is a canonical, correctly initialized and writable **16-byte process descriptor at actual E16830**, with lifetime spanning all three helper entries and their consumers. Its initial image words and all observed field stores are established, but no existing source service supplies that mutable mapping/lifetime. A private static object, per-call callback, copied read-only image bytes or per-thread record would invent a different owner. Borrowing an already-established actual descriptor could be an explicitly qualified future source interface, but this packet does not claim that such a provider currently exists.

Once that actual owner is concrete, a complete NLG source packet must include both writer entries/shared tail or an explicitly compatible provider for Notify1, preserving its ECX code and RET4 behavior. A complete transfer source additionally requires the real NLG entry and the actual selected handler/frame/dispatcher-stack continuation. The instructions are fully recovered; the missing owner and caller domain prevent declaring the two-helper composition ready today. No source proposal was implemented, and the root retains integration/metadata ownership.

Eight direct/tail call rows are physically verified, including the one undefined-parent BFB993 site; the shared-tail JMP is qualified separately as an intra-provider edge. Standard live verification is expected to reject only BFB993's missing container. Owned JMP ESI and contextual CALL EAX are separate indirect contracts. This packet does not infer flow overrides or no-return flags from missing decompilation, execute the disabled inline getter, define functions or save Ghidra. No build, tests, native execution or push occurred. Whole packet-local evidence, source copies and failure history are inventoried twice with SHA256/SHA512 in the report outside local storage.
