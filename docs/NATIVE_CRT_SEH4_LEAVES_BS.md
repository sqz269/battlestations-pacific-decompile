# Native CRT EH4 helper boundaries

This discovery retains complete fresh PE/live bodies for the four requested
helpers (97 bytes) and three actual direct providers (181 bytes). The total
is seven functions, 278 bytes, plus 20 bytes of data. All owned instructions
are present in the live listings. No source, Ghidra mutation, build, test or
runtime experiment is included.

Base: `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`. Each live CLI batch verifies
`C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. The installed PE
SHA256 is `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
BR discovery `062331c340b9d8d19bd77c2ba528f20a2f44197a` supplies the complete
handler/frame/caller evidence; all 75 prior artifacts were checked by exact
path set, size, SHA256 and SHA512 before selected spans were reused. The
prior worktree is unchanged. Reused spans also match the current installed
PE; they were not recaptured live.

Exact span hashes, call rows, original ABI, source readiness and the whole
packet-local double-hash inventory are in
`reports/native_crt_seh4_leaves_bs.json`. Correct existing library names are
preserved. Current indexed lookups and a bounded source scan found no exact
source implementations for the seven owned bodies.

## Requested complete helpers

| Entry | Body | Original interface |
| --- | --- | --- |
| `00C0DCB6 _EH4_CallFilterFunc` | 23 bytes, through `00C0DCCC` | ECX actual filter entry, EDX actual establisher frame; plain RET, filter EAX result |
| `00C0DCCD _EH4_TransferToHandler` | 25 bytes, through `00C0DCE5` | ECX actual handler entry, EDX frame; no return path, JMP to retained handler |
| `00C0DCE6 _EH4_GlobalUnwind` | 26 bytes, through `00C0DCFF` | ECX actual target registration; ordinary helper return after actual RtlUnwind continuation |
| `00C0DD00 _EH4_LocalUnwind` | 23 bytes, through `00C0DD16` | ECX registration, EDX target level, entry ESP+4 frame, ESP+8 actual cookie-word pointer; RET 8 |

The filter helper pushes incoming EBP, ESI, EDI and EBX in that order, sets
EBP to the incoming EDX frame, then clears EAX, EBX, EDX, ESI and EDI with
XOR instructions. It calls ECX without stacked arguments. At the filter's
entry, EBP is the actual frame, ECX still holds its code address, those five
registers are zero, and flags are from the final zeroing XOR: CF/OF/SF zero,
ZF/PF one, AF undefined. If helper entry ESP is `S`, the filter's return
word is at `S-14h`; four saved registers follow it.

After an ordinary balanced filter return, POP EBX/EDI/ESI/EBP and RET restore
the saved registers. EAX and arithmetic flags are exactly those left by the
filter; ECX/EDX remain its incidental volatile outputs. There is no null
check, result normalization, catch or FS-chain manipulation in the helper.
It does not repair FS if the actual filter changes it. The full BR dispatcher
uses the signed EAX result to select continue-search, continue-execution or
handler transfer. This contract is for an actual native frame-based funclet,
not an arbitrary ordinary C++ callback.

The handler-transfer helper sets EBP to EDX, keeps the handler in ESI and
EAX, pushes code 1 and calls actual `__NLG_Notify` at `00C16879`. It then
zeros EAX, EBX, ECX, EDX and EDI, preserving handler-in-ESI and frame-in-EBP,
and jumps to ESI. At that jump flags are again from a zero XOR. No stack
restoration, cookie check or RET occurs. BR's dispatcher reaches this entry
with a JMP while its own dispatcher stack remains present; the selected
native handler restores its required stack from the actual frame. A normal
C++ function call/return model would lose that contract.

The global-unwind helper pushes EBP, sets EBP to ESP, and pushes EBX/ESI/EDI.
It pushes `ReturnValue=0`, `ExceptionRecord=0`, actual internal continuation
`TargetIp=00C0DCFB`, then ECX as `TargetFrame`. Its direct call reaches
`00C2F25C`, a complete six-byte JMP through IAT `00CE21B8`, imported from
`KERNEL32.dll!RtlUnwind`. The current Windows SDK declaration is retained:
VOID NTAPI with those four pointer arguments. There is no fake unwind
callback or host CRT wrapper in this native boundary.

The continuation is the first POP EDI, followed by POP ESI/EBX/EBP and RET.
If helper entry ESP is `S`, the unwind call pushes four arguments below the
four saved registers; the continuation requires ESP=`S-10h`. On completion
the helper returns with ESP=`S+4`, the original nonvolatile values restored,
and API-produced EAX/flags unchanged. No particular API return value or OS
exception behavior is invented. A source translation must use its real
internal restoration label for TargetIp and preserve this stack schedule;
the original numeric code address cannot become a host continuation.

The local-unwind adapter pushes EBP, then loads its frame from adjusted
`[ESP+8]`. It pushes EDX target level, ECX registration and the cookie-word
pointer reloaded from adjusted `[ESP+14h]`, calling `00C0DBC4` with cdecl
`(actual_cookie_word_pointer, registration, target_level)` and EBP set to
the supplied frame. `ADD ESP,0Ch; POP EBP; RET 8` removes its inner arguments,
restores incoming EBP and consumes its two stacked outer arguments. EAX is
left as the provider returned it; final arithmetic flags come from that
ADD ESP,0Ch, not from a fabricated status test.

## Actual direct-provider ownership

`00C16879 __NLG_Notify` is a complete 31-byte body. EAX supplies the transfer
address, EBP the current frame, and entry ESP+4 the code DWORD. It preserves
EBX and ECX, binds the actual descriptor at `00E16830`, then writes code to
`+8`, EAX to `+4`, and EBP to `+Ch`, in that order. The subsequent
PUSH EBP/ECX/EAX and POP EAX/ECX/EBP sequence is retained; POP ECX/EBX and
RET 4 finish. EAX/EBP and arithmetic flags are preserved. No memory barrier,
allocation, callback or validation is present.

The actual 16-byte descriptor currently contains
`{19930520h,0,0,0}` in PE/live database bytes; `+0` is not overwritten by
this entry. A second native entry, `__NLG_Notify1`, also references this
descriptor. These values and addresses do not provide a private replacement
descriptor, runtime initialization or debugger/notification ownership.

`00C0DBC4 __local_unwind4` has a complete listed 144-byte normal body. It
receives the three stacked arguments above and the actual frame in EBP.
It saves EBX/ESI/EDI and builds a temporary native registration, including
the current EBP, arguments, handler literal `00C0DC54`, and previous FS:[0].
It reads actual `00E15590`, XORs that cookie with the new registration's
stack address, stores it at registration+8, and publishes the registration
to FS:[0]. That cookie source is distinct from its pointer argument, whose
current pointed-to value decodes each outer scope table. Both domains must
remain the actual compatible native state.

Each loop reloads the outer registration, encoded scope and cookie-pointer
argument. It stops at try level `-2`, or, when requested level is not `-2`,
when the current level is **unsigned <=** the requested level. It publishes
the scope's enclosing level to the outer registration before inspecting the
filter word. A nonzero filter word skips invocation. For a zero filter word,
it notifies with code `101h` and EAX=handler, then reloads that handler into
EAX, sets ECX=1 and calls actual `00C16898`. That provider's complete body
was not expanded here.

Normal cleanup first pops the old FS link, then adds ESP,18h and restores
EDI/ESI/EBX before RET. The saved EBP is part of the discarded registration
storage, not popped on this normal path; the outer adapter restores its own
incoming EBP. No normal-path cookie check occurs in the listed core. The
native registered exception handler and actual cleanup-call provider remain
necessary for full closure.

The live `00C0DBC4` prototype reports two direct calls, matching its 48
instructions and 144 bytes. The indexed call graph additionally attributes
a cookie-check call and a recursive local-unwind call to this candidate.
`00C0DC54` currently has no defined Ghidra function, even though the core
installs that exact handler address. This is an ownership/listing boundary,
not evidence that the 144-byte normal body contains four direct calls.
No handler body, missing call site or listing repair was invented. Complete
fresh raw bytes and live instructions agree for every owned body.

## Source recommendation and limits

The smallest next source packet is the complete **23-byte filter helper
`00C0DCB6`**. Its real register ABI, full save/zero/call/restore schedule and
single indirect target contract are established. A qualified MSVC Win32 naked
fastcall entry can accept the actual filter entry in ECX and actual frame in
EDX, with no private globals or missing direct provider. It must retain the
original register setup, flags, balanced funclet-return requirement and raw
EAX result. It does not create a scope table, filter body, native frame owner
or general-purpose callback service, and does not close C17653 execution.

The 26-byte global-unwind helper has a concrete real OS provider and is a
separate source candidate after explicitly preserving the self-continuation
and actual registration contract. Its import and native argument schedule
are established; OS unwind/FS behavior and running-process registration
validity remain unexecuted. It should not be silently grouped into a pure
filter-call packet.

Handler transfer still needs actual NLG descriptor ownership and a native
non-returning handler continuation. Local unwind still needs the undefined
`00C0DC54` handler, actual `00C16898` cleanup-call contract and compatible
cookie/FS state. Existing qualified `00C1815E` cookie initialization source
does not supply that canonical domain. No host SEH implementation, generic
allocator or invented state/callback closes these obligations.

Five owned direct calls plus five reused BR dispatcher CALL/tail-JMP rows
pass `verify_report_calls.py` (10 rows). Filter CALL ECX, handler JMP ESI and
RtlUnwind IAT JMP are qualified separately. BR's omitted C17653 filter/handler
evidence remains retained without mutation. These are discovery and static
identity checks, not source compilation, native execution, unwind/runtime,
binary drop-in or game validation.
