# Native local-unwind source proposal

The complete `C16898` cleanup-call primitive is independently ready for a qualified three-byte naked source entry. The complete 70-byte `C0DC54` handler can retain its native layout with two REL32 call bindings, but its actual cookie-checker and recursive local-unwind providers are not source-compatible or complete today. Adding context arguments to those providers does not close the original handler edge.

This proposal starts from published `5c80806cc011d29830cde3a87ff46c7dd5bd5c26`. Fresh supported BSP queries verify the configured `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`; the complete 73 owned bytes equal the installed PE, SHA256 `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. C0DC54 still has no defined live function. Its listing error is retained explicitly; no function definition, code-unit/flow repair, getter script, setting or saved analysis was changed.

Accepted BU `0cbad82f4fdc1551be711fc61e9ea07d5b731bba` supplies the complete handler, both cleanup callers and cookie/frame evidence. Accepted NLG `626c8878563f1a05644c71590f44a5fc212e02dd`, report SHA256 `841fad4e251f95e24b2957862833a9378ab13ec560a939aa65b86b349d07fc4c`, supplies both NLG writers, their shared tail and handler-transfer contract. All 470 NLG artifacts, including the nested 251 BU and 119 BS artifacts, were copied and verified unchanged. The fresh owned bytes also equal those accepted bytes.

## Exact handler and native frame contract

Let `H` be handler-entry ESP. Its four cdecl words are exception record at H+4, nested registration R at H+8, unused context at H+Ch, and dispatcher-output pointer at H+10h. The handler tests exception flags with mask 6 before accessing R. The mask-zero path returns EAX=1 without registration, cookie or output access.

On the other path, EAX becomes R and ECX becomes `[R+8] XOR R`. CALL C0DC6F invokes BFE120 with no stacked argument. The actual 15-byte checker compares ECX with actual E15590, returns through `F3 C3` on equality, and tail-jumps C185A4 on mismatch. It preserves EAX=R on the normal return; the handler immediately needs that value to load `[R+18h]` into EBP after saving its incoming EBP.

The handler pushes `[R+Ch]`, `[R+10h]`, `[R+14h]` and calls C0DBC4 at C0DC81. The callee receives the captured scope-cookie pointer, outer registration and target level, with the actual inherited frame in EBP. Only a normal recursive return is followed by ADD ESP,0Ch, POP EBP, reloading R and the output pointer, storing R through that pointer, and returning EAX=3. There is no catch, retry, output prepublication or promised restoration after a nonlocal exit. The handler itself does not write FS:[0].

The actual C0DBC4 producer saves EBX/ESI/EDI and builds its registration at entry ESP minus 28h. R+0 is the old FS link; R+4 is C0DC54; R+8 is actual E15590 XOR R; R+Ch/+10h/+14h are captured target/outer-registration/scope-cookie-pointer; R+18h is the inherited EBP. The cookie is written before FS:[0] is published. The live loop separately reloads the original argument slots, including the pointed-to scope-cookie word. That scope decoder and the nested registration's global authentication cookie are distinct reads and roles.

C0DBC4 stops at level -2, or unsigned current level <= target when target is not -2. Before testing a scope's filter, it publishes the enclosing level into the outer registration. A zero filter triggers NLG code101h, then ECX=1 and a fresh cleanup-pointer load before CALL C16898. A cleanup exception therefore follows state advancement; no automatic retry of the selected scope is supplied. The full loop must retain its actual nested FS registration and unlink, not use a host unwind loop or C++ cleanup collection.

## Concrete source interfaces and relocation gate

The immediate minimal packet is `src/native_crt_local_cleanup_call.cpp` with `include/bsp/native_crt_local_cleanup_call.hpp`, exposing a documented register entry `void __cdecl bsp::call_native_crt_cleanup_00c16898()`. Its entire naked body is CALL EAX; RET (`FF D0 C3`), with no relocation, save/restore, argument setup or result normalization. EAX is the actual executable cleanup entry; EBP and ECX are inherited. A balanced actual cleanup return is required. C0DBC4 supplies ECX=1, while the complete C167C9 caller supplies the scope's enclosing level in ECX and actual NLG preserves it. The primitive cannot turn its declaration into an ordinary C++ callback interface or supply a funclet/frame owner.

A future handler interface can keep the original four words: a naked uint32 cdecl entry taking exception-record, actual nested-registration, unused-context and dispatcher-output pointers. It must retain all 21 instructions. The physical span contains no tables, padding or absolute state operands:

| Original call | Opcode offset | Operand offset | Required actual target |
| --- | --- | --- | --- |
| C0DC6F | 1Bh | 1Ch, four bytes | BFE120, ECX cookie / EAX-preserving, no extra stack word |
| C0DC81 | 2Dh | 2Eh, four bytes | C0DBC4, three original cdecl words and inherited EBP |

The proposed COFF relocations are `IMAGE_REL_I386_REL32`. All other 62 bytes can remain identical, including the `74 33` internal branch to the final RET at offset45h. `handler_relocation_proposal.json` and the masked 70-byte artifact record the old operands and exact offsets. This is a feasibility proposal, not emitted-code or native ABI validation. Future acceptance must prove the exact body, both actual provider bindings and native call/frame contracts; merely naming external symbols is insufficient.

The parallel CO proposal covers complete BF65BB[252], BFE120[15] and C185A4[260] through a new context-stack interface. It reportedly preserves EAX around the checker comparison but adds a cdecl context word and tail-transfers to a corresponding reporter. At the original C0DC6F call there is no such word: the checker sees its own return word, then the handler's return word and existing four OS words. CO therefore cannot directly fill this unchanged handler edge. No wrapper or synthetic failure-report frame is proposed. `co_checker_compatibility.json` records the coordinated result and distinguishes a proposed qualified source provider from an actual native-compatible one.

## Required canonical NLG owner and callable entries

The actual descriptor is the 16 writable `.data` bytes at E16830, section flags C0000040, initialized in the image to `{19930520h,0,0,0}`. Both writers use that same process cell. Its fields are raw DWORDs: +0 magic, +4 raw value, +8 code, +Ch frame. The common tail writes +8=ECX, then +4=EAX, then +Ch=EBP. Neither writer initializes/resets +0, locks the record, allocates storage, issues an external notification callback or rolls back earlier stores. +4 is not universally an executable pointer: a Notify1 caller records a funclet's returned EAX.

The minimum owner contract is one correctly initialized, writable canonical process descriptor with lifetime spanning both entry points and all consumers. A stable borrowed view may describe that existing cell; it does not create the cell or its lifetime. A private static/TLS copy or the read-only data mapper is not an actual owner. The current mapper and bootstrap are byte-identical to accepted NLG evidence: they support read-only CE2000..E07B23, whereas E16830 is outside that range and requires writes.

| Provider | Complete native boundary | Readiness for this composition |
| --- | --- | --- |
| C16879 `__NLG_Notify` | 31 bytes; EAX raw value, EBP frame, stacked code; preserves registers/flags; RET4 | Needs actual canonical mutable cell and native callable entry |
| C16870 `__NLG_Notify1` | Nine-byte prefix plus shared 20-byte tail; ECX code, EAX raw value, EBP frame; consumes one existing stack word with RET4 | Prefix alone is incomplete; must share the same canonical state and tail semantics |
| C0DCCD `_EH4_TransferToHandler` | 25 bytes; ECX handler, EDX frame; notify code1, zero five registers, JMP ESI with actual dispatcher stack | Adjacent continuation boundary, not a callee of the local handler/core; remains dependent on actual NLG and handler continuation |
| BFE120 / C185A4 | Actual cookie check and native mismatch-report owner | CO's added context ABI does not supply the unchanged native edge |
| C0DBC4 / C0DC54 | Mutually connected native nested-registration loop and handler | Full bytes established; actual cookie/NLG/failure/funclet/FS ownership still required |
| C0DD00 and other native callers | Wrapper/frame/caller contracts remain part of full integration | Not expanded or represented as newly completed source here |

An added borrowed-state argument to NLG, the checker or recursive local-unwind changes those call contracts. Preserving the original 70-byte handler requires native callable providers that resolve actual canonical state without inserting context words at its call sites. This distinction is part of the proposed acceptance gate.

Existing source supplies the qualified C1815E cookie initializer, C07C45 epilog, C0DCB6 filter-call primitive, C0DCE6 global-unwind helper and C2F25C actual RtlUnwind thunk. Their current headers/sources are retained. They own neither canonical cookie/NLG storage nor this local loop/checker/failure chain; global unwind is not a local-scope substitute.

## Evidence limits and handoff

Both owned direct calls are verified from complete fresh physical bytes. Their live function membership remains unavailable because C0DC54 is undefined; accepted BU's standard call-verifier failures remain retained. C16898's CALL EAX and NLG's indirect/shared-tail sites remain explicitly qualified. Saved flow overrides, exact code-unit membership and no-return properties were not guessed or repaired.

No C++ source, build, test, helper/native execution, Ghidra mutation or push occurred. The report includes the complete proposals, retained provider/caller evidence, source compatibility and two whole-local SHA256/SHA512 inventories. Sealed sibling CG473 and CN74 artifacts were rehashed unchanged. Root review is required before any implementation; the three-byte cleanup primitive is the only immediately independent source packet proposed here.
