# Native CRT five-word tail-dispatch boundary at 00BF66EF

The complete `00BF66EF..00BF6712` body is 36 bytes / 14 instructions. It
temporarily saves EBP, passes the current DWORD at `0109DD64` to a direct
provider, and tests that provider's returned EAX. A nonzero result becomes
an indirect tail target. Zero selects a second direct provider with literal
argument two, followed by a direct tail jump to saved `__invoke_watson`.

Both tails restore entry ESP under compatible child returns and retain the
caller's current return word and argument positions. The body never reads,
copies or re-pushes the five words supplied by accepted caller `00BF6713`.
Those positions survive; their values need not remain zero if a provider
changes them. There is no owned RET. Provider preservation and failure policy
remain open, including the saved no-return assertion on the final named target.

## Scope and complete-body gate

Packet `cc12_native_crt_five_zero_argument_target_ABI_readiness` owns only
`00BF66EF` and this document/report pair. The worktree remains at published
baseline `e115a399d53b152e3bba3b4201b814a7bf57986b`. A later Root build at
`fbaefa43f0135f71e019bb995c48078c8fa3306f` is pinned separately below.

Each live `bsp.py ghidra` batch verified project `bsp`, program
`/battlestationspacific.exe`, `x86:LE:32:default` and image base `00400000`.
The configured existing `C:/Users/sqz269/bsp.gpr` exists; this does not prove
a stronger server-side absolute GPR path. Live and saved counts are 64,729.

The actual saved extent was checked before reading its complete bytes:

- Body SHA-256: `0e1f2416d2f25c96b8d07f18d401ef61528110d4b452a1b3a7336bb1cad0e4b3`.
- RVA/file offset: `007F66EF`; all 36 live bytes equal the configured PE.
- PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
- All 14 independently decoded instructions have saved listing entries.
- Saved metadata: 14 instructions, three blocks, two edges, three calls and
  zero parameters; saved name/prototype is `LIBCRT_unmatched_00bf66ef(void)`.
- Physical transfers: **two CALLs, one indirect tail JMP, one direct tail JMP,
  one conditional branch and no RET**. The metadata call count is kept separate.

The conditional path union covers all 36 bytes / 14 instructions with
compatible direct-provider returns. There is no gap, loop, omitted byte or
hidden continuation. The last direct jump ends exactly at `00BF6712`; no
adjacent byte or caller body was opened. The decompiler's unrecovered-jumptable
warning at `00BF6703` corresponds to `JMP EAX`; no jump table was inferred or read.
Its call-like rendering and inferred wide-character parameter types do not
replace the physical tail/stack contract. No listing or no-return repair is needed.

The three direct targets were queried only for function metadata. The sole
absolute data operand, DWORD `0109DD64`, was identified from the owned
instruction and `DAT_0109dd64` decompiler label. Its contents, initialization,
encoding, IAT identity and lifetime were not inspected. No extra data lease
or Native data window was needed. No subordinate body, handler, caller or
CRT data, Ghidra mutation, Source, CMake, ledger, build, test or probe is supplied.

## Complete ordered schedule

Let S be entry ESP, G=`0109DD64`, W the value loaded from G, and H the EAX
value after the first provider returns. `A1..A5` name only the observed
caller DWORD slots `[S+4]..[S+20]`; they imply no semantic field types.

| Address | Operation | Ordered effect |
| --- | --- | --- |
| `00BF66EF` | `PUSH EBP` | Save incoming EBP at S-4 before reading G. |
| `00BF66F0` | `MOV EBP,ESP` | EBP=S-4. No owned argument load uses EBP. |
| `00BF66F2` | `PUSH DWORD[G]` | Read current G once; store W at S-8. |
| `00BF66F8` | `CALL 00C04FDE` | Child entry S-12, return=`00BF66FD`, one pushed word W. |
| `00BF66FD` | `TEST EAX,EAX` | Set branch flags from H. |
| `00BF66FF` | `POP ECX` | Read the current S-8 word, then restore ESP to S-4. |
| `00BF6700` | `JZ 00BF6705` | Select zero arm while retaining TEST flags. |
| `00BF6702` | `POP EBP` | Nonzero arm: read current S-4 word; ESP=S. |
| `00BF6703` | `JMP EAX` | Tail to H with the caller's current stack. |
| `00BF6705` | `PUSH 2` | Zero arm: reuse S-8 for literal two. |
| `00BF6707` | `CALL 00C04EF3` | Child entry S-12, return=`00BF670C`, one pushed word two. |
| `00BF670C` | `POP ECX` | Read the current S-8 word; ESP=S-4. |
| `00BF670D` | `POP EBP` | Read current S-4 word; ESP=S. |
| `00BF670E` | `JMP 00BF65BB` | Direct tail to saved `__invoke_watson`. |

Both direct calls require a compatible return with ESP=S-8 for their following
POP cleanup. A callee that consumes its pushed argument would shift the later
POPs and tail frame; the owned code neither detects nor repairs that mismatch.
The deepest owned stack write is S-12, including each CALL's return word.

The frame save is observable before the absolute read. If S-4 aliases G, the
saved-EBP store changes the word subsequently loaded as W. Every POP is a late
read of current stack memory. Thus ECX after the first POP is not guaranteed
to equal the originally loaded W, and after the second POP it is not guaranteed
to equal two. EBP is restored from the current saved slot, not from a cached
register copy. No provider mutation, alias or fault is hidden by an invented
immutable snapshot, rollback, type or lifetime guarantee.

## Registers, flags and tail argument delivery

Before the first CALL, EAX, ECX, EDX, EBX, ESI and EDI retain function-entry
values, EBP is S-4, and incoming flags are unchanged. Their relevance to the
unopened provider is unknown. H is only the returned EAX used as a test and
transfer target; the saved decoder name alone does not prove its algorithm.

TEST H,H sets CF=OF=0, ZF according to H, SF from bit31 and PF from the low
byte, with undefined AF. POP ECX preserves those flags. The nonzero path's
POP EBP and JMP also preserve them, so the indirect target enters with these
nonzero TEST flags. Its EAX is H, ECX is the late argument-slot value, and
EDX/EBX/ESI/EDI retain first-provider outputs. EBP is the current saved word.

On the zero arm, the second provider receives EAX=0, ECX from the first POP,
other first-provider outputs, and the zero TEST flags. EBP is not reset between
calls. After that provider returns, its EAX is not forced back to zero. The
two POPs and final JMP retain its returned flags; they do not recreate TEST
flags. ECX and EBP come from their respective current stack slots.

Both tails enter with ESP=S, current `[S]` as the caller return word, and the
entire current argument area in place. The observed five-word prefix is
neither consumed nor copied by this body. It is also not re-zeroed after a
provider call. No exact arity, string/wide-character field type, line-number
meaning, reserved field, handler signature or safe function-pointer property
is established by these owned instructions.

The indirect tail target is unguarded except for H being nonzero. A compatible
target can return through `[S]` directly to the original caller; there is no
owned post-tail continuation. Original EBP reaches a tail only if the save
word survives. EBX/ESI/EDI are neither modified nor saved locally, so complete
preservation still depends on the reached providers and final tail target.
There is no local EH, x87/SSE work, object owner or failure conversion.

## Metadata edges and accepted caller composition

| Target | Preserved saved name and metadata | Physical owned delivery |
| --- | --- | --- |
| `00C04FDE..00C0504B` | `BSP_CRT_DecodePointerCurrentState`; 110 bytes / 36 instructions; saved zero parameters | One pushed DWORD W; EAX result tested and possibly jumped to. |
| `00C04EF3..00C04EFA` | `BSP_Crt_ClearDebuggerHookState`; eight bytes / two instructions; saved zero parameters | One pushed DWORD two; no literal-two semantics or clearing effect independently proved here. |
| `00BF65BB..00BF66B6` | `__invoke_watson`; 252 bytes / 65 instructions; saved five-parameter cdecl, no-return prototype | Direct tail with the original current return/argument positions. |

The saved `__invoke_watson` library label and prototype are preserved as
metadata. Its no-return annotation explains the decompiler warning but is
not fresh evidence of Native termination, throwing, default-handler choice
or any particular runtime failure path. The owned function's nonzero tail
also prevents declaring the whole wrapper nonreturning from that one label.
`LIBCRT_unmatched_00bf66ef` remains unchanged; a familiar CRT pattern does not
by itself establish a recovered `_invalid_parameter` symbol or typed API.

The accepted 16-byte caller audit is the sole source of incoming caller facts;
no caller Native body was reopened. Let U be `00BF6713` entry ESP. Its five
zero pushes and CALL enter this function with S=U-24, `[S]=00BF671F`, and
`A1..A5=0` initially. Each inner CALL can reach U-36; either tail restores
U-24 under the stated contracts. The five words remain at their original
positions, with zero values conditional on provider nonmutation.

A compatible final target's plain RET reaches `00BF671F` at ESP=U-20. The
accepted outer `ADD ESP,14h; RET` then leaves U+4 and supplies the outer
returning arithmetic flags. A tail target that cleans the five argument words
would not compose with that later ADD. That target cleanup/preservation
contract remains unproved. Accepted99 and parent691 call/tail composition
stays qualified through the pinned 16-byte receipt; no raw caller migration
or production storage is supplied.

## Current Source boundary and versioned receipts

The accepted 16-byte audit at `548cbb243055558bab7cb8b56ba6f4789402bb46` and its
document are unchanged at this packet's baseline. Its 79 repository input
pins replay exactly; adding that report/document gives 81 baseline pins.
The accepted 65-input Source domain remains unchanged in this worktree.

The actual `NativeInputEnumerationCalls` declaration, concrete
`NativeInputDeviceRuntime` declaration and its `_invalid_parameter_noinfo()`
method body remain byte-identical. These three files are separately pinned
and are outside both the older65 and newer67 build-input sets. The method
retains its actual Source object API even though its body reads no receiver
fields. No fake runtime receiver or pair-to-runtime cast is warranted.

The selected SDK 10.0.26100.0 `corecrt.h:371` still has exactly the accepted
header hash and declares `_invalid_parameter_noinfo(void)` as `void __cdecl`
without `noreturn` or `noexcept`. The generated project changed during the
new Root registration, while SDK/toolset v145 selections remain unchanged.

During this audit the integrator published the separate Root67 domain at
`fbaefa43f0135f71e019bb995c48078c8fa3306f`. Its committed receipt is
`reports/cc12_native_tick_subnode_base_cleanup_primary_review.json`. All 67
inputs replay against that revision and current Root files. Compared with
the baseline65 set, only common CMake changes; the two additional files are
the tick-subnode cleanup header/body. The new receipt/document are pinned
at that later revision without rebasing this packet's baseline.

All four current Core/executable/map/test-log files now match Root67 and
differ from the older65 artifact hashes. The earlier build's hashes are
retained as historical, never relabeled current. Root67's registered build
ran `2026-10-09T15:51:21Z..15:51:40Z`, passed three existing checks and records
12 whole objects / 14 positive Core roots, with the prior 11 whole objects
and 20 Legacy/eight EH contracts unchanged. These are inherited build/review
claims, not a worker build, object-graph reparse or Native36 credit.

Fresh bounded inspection of the Root67 Source map and Source PE import
metadata still finds the typed runtime member and free
`_invalid_parameter_noinfo` import from `api-ms-win-crt-runtime-l1-1-0.dll`.
The executable/map hashes were checked around those metadata reads. No
Native IAT contents, Source CRT provider body or specific compiled member
operand was inspected. Source import availability does not establish the
Native child signature, global decoding, debugger-hook effect or either tail
provider's identity/policy.

That free zero-argument Source API can remain a proposed policy borrower for
the observed zero-information use. It does not implement this generic
current-five-word tail forwarding or prove Native failure/return/throw/type
behavior. Handler state, lifetime, reentrancy and provider choice remain
explicit external contracts. The accepted older99/691 reports and historical
runtime fixture/aggregate were not broadly reopened or promoted to current
execution evidence. No Source candidate or new function/byte credit is added.

The decoder, debugger-hook provider, dynamic tail target, `__invoke_watson`,
unread G state, Native EH and production backing/lifetime remain open. The
complete owned body needs no extra Native address or data window to close
this bounded audit.
