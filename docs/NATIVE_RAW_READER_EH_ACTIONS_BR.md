# Raw-reader EH actions and destruction readiness

Discovery only, based on `27f97f57`. Six owned bodies are completely recovered: `CC70C0[8]`, `CC70C8[11]`, `CC7810[11]`, `CC7790[23]`, `BF7C6E[75]`, and `BF7CB9[24]`. Their 152 bytes match fresh target-verified live Ghidra and the installed PE. Read-only direct dependencies add `BE9F00[16]`, corrected `BF0980[23]`, `401130[1]`, and `BF6989[5]`. No source, Ghidra mutation, build or runtime test is supplied.

The BO destruction discovery `36195e5c03b8240cd5d21229b01dd0a68fe63fbe` and BM unwind discovery `f57a2fba9bef3751f78c8f7ed062c1680f26c573` remain the parent/provider evidence. Selected complete parent/dispatcher spans, decoded instructions, scope tables and their full reports are retained and hash checked. This packet does not repeat or claim closure of their entire CRT dependency graphs. The installed PE SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. Every fresh Ghidra CLI invocation verifies project `bsp`, program `/battlestationspacific.exe`, language and base using the retained configuration for `C:/Users/sqz269/bsp.gpr`, bridge8089.

## Action frame and calls

Let S be the original parent entry ESP. All three reader/storage parents install their exception registration R at S-0Ch. The retained C069A2 dispatcher publishes the next unwind state at C06A09 before calling C07B10. C07B10 sets action EBP=R+0Ch=S before CALL EAX. Thus these action operands use the dispatcher-installed frame, not a normal argumentless C++ function frame or the parent's current EBP register. BF05D0 uses its own EBP as a loop index during normal execution; that value is not the action's EBP.

| Action | Parent state transition | Actual operation and ABI |
| --- | --- | --- |
| CC70C0 | BE9F10 state0 -> -1 | Load ECX=[action EBP-10h], the saved reader; tail JMP BF09B0. No explicit stack arguments. |
| CC70C8 | BE9F10 state1 -> 0 | Load the saved reader, add10h, tail JMP BE9F00. BE9F00 pushes destructor41DD20, count10, stride8, ECX destination, then calls BF7C6E; its RET10h consumes all four words, and wrapper RET returns to action caller. |
| CC7810 | BF09B0 state0 -> -1 | Load saved reader, add4, tail JMP BF0980. BF0980 calls BF0700(header,0), reloads CURRENT [header], calls BF6989(data), cleans4 argument bytes and returns. |
| CC7790 | BF05D0 state0 -> -1 | Compute wrapping DWORD end=[EBP-18h]+12*[EBP-14h]; push end, then begin=[EBP-10h]; CALL401130; ADD ESP,8; RET. |

BE9F10 saves the reader at S-10h at BE9F29; BF09B0 saves it at S-10h at BF09CA. Their FuncInfos have magic19930522, EHFlags1, and maps E01AB4 (`{-1,CC70C0},{0,CC70C8}`) and E0245C (`{-1,CC7810}`). These state transitions are published by the dispatcher before action entry; the tiny actions do not write the state themselves.

For CC7790, the slots are different: BF0623 saves the new allocation at S-18h; BF0627 initializes and BF0692 updates completed count at S-14h; BF063B publishes current destination at S-10h. BF0643 arms state0 after that publication, and BF068A disarms before publishing the next completed count. The action's begin is the current candidate, not allocation start. Its end is the completed boundary; at ordinary copy-throw sites these are the same position. Crucially, the actual 401130 body is exactly C3 (RET), with no reads, destruction or free. The action therefore supplies no allocation rollback or completed-prefix string cleanup, even though it computes and passes two range pointers. This finding was shared with the disjoint BF05D0/BF0700 storage worker.

## Corrected BF0980 physical boundary

Ghidra's BF0980 body ends at BF0991, the end of CALL BF6989. Fresh PE/live bytes show the returning continuation `BF0992 ADD ESP,4; BF0995 POP ESI; BF0996 RET`; padding begins at BF0997. The complete physical body is 23 bytes, not the initial 18-byte listing span. Both captures are retained; no listing mutation occurred.

BF0980 saves header in ESI, pushes zero and calls BF0700 with ECX=header (callee RET4). Only after that returns does it load the current data DWORD from [ESI], push it and call BF6989. The complete five-byte BF6989 thunk jumps to BF65AC. It is a native CRT-free boundary, not evidence that a host free service owns these allocations. If BF0700 does not return, this action does not independently free the buffer or retry itself. The corrected tail is required to account for its stack cleanup.

## Reverse destruction and failing-element schedule

BF7C6E takes four stack words `(base,stride,signed_count,destructor)` and returns RET10h. Its SEH4 prolog uses scope E02D30: GS offset -2, EH cookie offset -2Ch, state0 enclosing -2, null filter and finally BF7CB9. No ordinary stack parameter is supplied to the finally.

After the prolog, EBP+8 is a mutable cursor, +Ch stride, +10h remaining count, +14h callback. EBP-1Ch is success, initially zero; EBP-4 is state. ESI captures stride. The iterator advances cursor by stride*count, arms state0, then decrements count BEFORE each element, exits if signed-negative, subtracts stride from cursor, loads ECX=current element and calls [EBP+14h] with no explicit stack argument. Only a normal return proceeds to the next decrement.

On full success it stores success=1, state=-2, calls BF7CB9 normally (which skips cleanup), restores the SEH4 frame and RET10h. On unwind, BF7CB9 inherits this same EBP; if success is still zero, it pushes callback, CURRENT remaining count, stride and CURRENT cursor, then calls BF7C10. BF7C10 consumes those four words with RET10h and destroys the earlier prefix in reverse order.

For actual reader count10/stride8, if header k throws, higher headers9..k+1 have completed destruction, cursor points to k and remaining is k. BF7C10 decrements first and moves back before each callback, visiting k-1..0. Header k is never retried; already destroyed higher headers are not revisited. When k=0, no earlier callback occurs. Valid count/pointer arithmetic and matching ECX/plain-RET callback ABI are required; arbitrary invalid signed counts/overflow or callbacks mutating frame slots are not a reader-source contract.

The fixed callback is 41DD20: capture data at+4, skip all remaining work if null, otherwise capture wrapping length+1, invoke the actual argumentless419CC0 pool getter, and return data through BD1510. The existing raw `NativeStringRawPoolContext` overload in native_string.cpp preserves that potentially throwing getter. Its `NativeStringStorage` noexcept overload is not the same contract.

BM's retained BF7C10 filter uses inherited exception pointers at EBP-14h: C++ code E06D7363 selects native C07A75 terminate; other codes return zero/continue search. If a remaining-prefix destructor throws a C++ exception, this is not a continue-cleaning or retry policy. Native terminate/PTD/abort ownership remains separate. The outer C07991 eligibility gate also matters: ordinary asynchronous faults can bypass outer cleanup under EHFlags1. A generic catch-all or host terminate callback is not a replacement for these native runtime contracts.

## Full-reader readiness and remaining providers

BE9F10 arms state1 before releasing its separate +68h/+6Ch name. A name-release exception selects all ten path headers via CC70C8; the failed name release is not retried. Normal path destruction runs after setting state0, so an exception there is handled first by BF7C6E's remaining-prefix finally, then eligible outer unwind selects the base via CC70C0. BE9F10 sets state=-1 BEFORE its normal BF09B0 call, preventing an outer retry of that base call. Subsequent action execution depends on the real exception dispatcher continuing; no guarantee is invented if a cleanup action itself throws.

BF09B0 captures the attached stream, arms state0, decrements it and invokes current slot0 only at zero. For a nonnull captured stream, only successful return from that release path writes reader+0=0; a throwing terminal leaves that write undone and selects CC7810. A null captured stream skips the write. The normal header cleanup phase is disarmed before BF0700(header,0), then frees the CURRENT header data after successful return. It therefore differs from BF0430 attachment, which publishes new first and never writes reader after release; directly calling the attachment setter would change destruction ordering.

The action and frame-slot contracts are now concrete, but full BE9F10/BF09B0 source is not declared ready by this packet. Remaining requirements are: complete BF0700 storage behavior (the sibling discovery owns it); a correctly paired actual buffer free/allocator domain behind BF6989/BF65AC; terminal dispatch using the established memory flags1 and physical flags0/recycle contexts without copying attachment publication order; and a stated source exception contract for raw string cleanup and cleanup failure. Native FH3/SEH4, cookie/TLS/exception/terminate ownership and asynchronous-fault identity remain separately open. Current raw string providers are reusable evidence, not proof that every cleanup is nonthrowing. No narrowed success-only destructor, invented generic callback, allocator, or fake frame provider is proposed.

The machine-readable report records all 12 direct call/tail-jump rows in the owned and newly expanded dependency bodies, plus the iterator's separately qualified indirect callback. Parent/dispatcher call inventories remain in the pinned earlier reports. Validation is discovery/PE/live/call evidence only; no reconstruction, build, fixture, ABI or game validation is claimed. Local artifacts are inventoried twice with size/SHA256/SHA512; the report is outside that hashed directory.
