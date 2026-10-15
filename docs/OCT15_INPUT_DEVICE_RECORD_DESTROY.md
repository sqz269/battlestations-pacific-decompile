# Actual InputSettings record, pair and scalar-delete cleanup

This packet reconstructs complete `0055B690`, `0055B8B0` and `006A6EB0`, plus the ten parent-bound unwind action bodies. Names describe recovered behavior; the existing STL/compiler names and older actual AV projection domains remain. The new Source borrows `NativeStringRawPoolContext` and uses accepted actual storage services. It adds no callbacks, global providers, allocation model or `NativeStringStorage` noexcept bridge.

The record is an actual84h ownership object. Its embedded headers occupy00/0C/18/24/30/40/50/5C/6C/78. The pair has an8B pooled key followed by the record at+8; scalar deletion receives the containing allocation with the pair at+C. All reached records, finite uniquely owned nodes/sentinels, vectors and pool cells must be valid, with matching current-CRT allocation ownership. Full selections require stable exclusive storage through selection.

## Complete bodies and original interfaces

| Entry | Bytes / instructions | Original interface |
|---|---:|---|
| `0055b690` | 534 / 195 | Native ECX actual84h record; EDX unused; no stack input; RET; EAX unspecified residue. Source adds stacked RawCtx; normal/EH order and CurrentCpp failure policy recovered, not original FH3/frame ABI. |
| `0055b8b0` | 97 / 29 | Native ECX actual pair with key at0/4 and record+8; EDX unused; no stack input; RET; EAX unspecified record/pool residue, including null key. Source adds stacked RawCtx/private guard. |
| `006a6eb0` | 33 / 12 | Native ECX original allocation; EDX unused; stacked DWORD flags; RET4; defined EAX original allocation. Pair+C first; flags low BYTE bit0 read only after pair returns; conditional current-CRT free. Source adds stacked RawCtx. |
| `00c6dc30` | 8 / 2 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc38` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc43` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc4e` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc59` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc64` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc6f` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc7a` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dc85` | 11 / 3 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |
| `00c6dca0` | 8 / 2 | Parent-bound runtime EBP receiver; tail-JMP to child. Source explicit logical receiver/context. |

The two destructor parents have plain RET and no recovered semantic EAX result. In particular the null-key pair branch retains record residue; it does not establish EAX0. Scalar deletion has RET4 and returns the original allocation even after freeing it. Incoming EDX is unused. The new APIs add context and change private frames; action APIs do not pretend to recover native EBP from a new C++ frame.

## Normal schedule and captures

| Member | Capture before state | State arm | Native child entry | CURRENT free | Ordered clears |
|---|---|---:|---|---|---|
| +78 | 0055b6b1, 0055b6b4 | 8 at 0055b6c4 (DWORD) | 004cef40 | 0055b6d5 | 0055b6dc, 0055b6df |
| +6c | 0055b6e2, 0055b6e5 | 7 at 0055b6f8 (BYTE) | 00559280 | 0055b706 | 0055b70b, 0055b70e |
| +5c | 0055b711 | 6 at 0055b71c (BYTE) | 0055a3a0 | 0055b735 | 0055b73d, 0055b740, 0055b743 |
| +50 | 0055b746, 0055b749 | 5 at 0055b759 (BYTE) | 0055b230 | 0055b767 | 0055b76c, 0055b76f |
| +40 | 0055b772 | 4 at 0055b77d (BYTE) | 00432050 | 0055b796 | 0055b79e, 0055b7a1, 0055b7a4 |
| +30 | 0055b7a7 | 3 at 0055b7af (BYTE) | 00432050 | 0055b7c8 | 0055b7d0, 0055b7d3, 0055b7d6 |
| +24 | 0055b7d9, 0055b7dc | 2 at 0055b7ec (BYTE) | 00444b10 | 0055b7fa | 0055b7ff, 0055b802 |
| +18 | 0055b805, 0055b808 | 1 at 0055b81b (BYTE) | 00559a90 | 0055b829 | 0055b82e, 0055b831 |
| +0c | 0055b834, 0055b837 | 0 at 0055b84a (BYTE(BL0)) | 005599c0 | 0055b857 | 0055b85f, 0055b862 |
| +00 | 0055b865, 0055b868 | -1 at 0055b875 (DWORD) | 00559110 | 0055b886 | 0055b892, 0055b895 |

Each initial head/head.left or vector begin/null-test precedes its new state. The previous state remains active through current child work, CURRENT-pointer free, all clears and the following initial captures. The first state is8; states7 through0 use native byte stores; final state-1 uses a DWORD store. Source retains those logical values with a volatile int32 guard state, with original byte slot/frame identity qualified.

Seven tree members receive captured head.left/head, their own owner twice and a private writable8B iterator output. On valid stable ownership these arguments select the accepted full prefix before any subtree getter. The ordinary diagnostic/partial paths are not promoted. Those exact prefix services retain native read/reset/output order. After completion the parent reloads CURRENT head, frees it, then zeroes+4 and+8; opaque+0 is untouched.

The vectors at+5C/+40/+30 capture begin before arming, read end only on the nonnull branch, destroy the captured range, reload CURRENT begin for free and always clear+4/+8/+C after successful work. Null begin skips the end read, range and free, but still clears. +5C uses the actual16B owning-header range; its two dead metadata words are member-header and saved-record. The two key-header ranges use the accepted8B service, whose Source interface omits its two physically dead stack words. Normal +40 range432050 and unwind whole-vector4324A0 concern the same actual16B owning string-header vector. Fresh C6DC64 loads the saved record, adds40h and tail-transfers to4324A0 at C6DC6A. The earlier +40 key41DD20 interpretation is corrected; pair actionC6DCA0 is the separate41DD20 key cleanup. Source already used the correct4324A0 target, and no native asymmetry is inferred.

The pair arms state0 before record+8 destruction. After record completion it captures CURRENT data at+4 and tests it, then disarms at state-1 before the optional length read/getter. Nonnull data captures current length+1 with DWORD wrap before pool lookup and return of the captured pointer/size; null skips length/getter. Native pre-pushes unused literal1, size and data before the getter; the pool return consumes those three words. Accepted Source services explicitly borrow pool cells and omit the unused1, with private stack/argument aliases qualified. The key header stays untouched. Scalar deletion reaches the flags LOW BYTE bit0 test only after pair+C completes; failure skips that read and outer allocation free.

## EH maps, actions and failure policy

Native record handlerC6DC90 selects FuncInfoD986C8 and mapD986EC, maxState9. Pair handlerC6DCA8 selects FuncInfoD9873C and mapD98734, maxState1. Both raw FuncInfo records have magic19930522, no try or IP map, no exception-type list and raw EHFlags1. This is native evidence, not Source RTTI/type-matching identity. Runtime CallSettingFrame establishes EBP=entryESP; record actions read saved receiver[EBP-18h], pair action[EBP-10h].

| State | Advance before action | Action | Logical receiver | Child |
|---:|---:|---|---|---|
| 0 | -1 | `00c6dc30` | record+00 | `0055a3f0` |
| 1 | 0 | `00c6dc38` | record+0c | `0055aab0` |
| 2 | 1 | `00c6dc43` | record+18 | `0055aaf0` |
| 3 | 2 | `00c6dc4e` | record+24 | `00445900` |
| 4 | 3 | `00c6dc59` | record+30 | `004324a0` |
| 5 | 4 | `00c6dc64` | record+40 | `004324a0` |
| 6 | 5 | `00c6dc6f` | record+50 | `0055b490` |
| 7 | 6 | `00c6dc7a` | record+5c | `0055b110` |
| 8 | 7 | `00c6dc85` | record+6c | `0055a430` |
| pair0 | -1 | `00c6dca0` | CURRENT pair key | `0041dd20` |

A normal child/getter C++ failure retains its completed effects and triggers earlier-member actions, excluding the current member. There is no retry or rollback. If record unwind completes, pair state0 applies CURRENT key cleanup. Scalar outer free is skipped on failure. The first normal member+78 has no separate record action.

Native FrameUnwindToState writes the to-state before invoking each action. Its SEH4 scopeE03398 selects raw filter thunkC06A24 and raw filterC0695E; for exception codeE06D7363 the filter zeros PTD+90 and routes to terminateC07A75. A secondary C++ exception from record or key unwind terminates rather than replacing the original exception or continuing later cleanup. Other exception/managed routing remains qualified. The two filter spans are explicitly untyped raw-byte evidence; no function/listing repair is performed.

Private CurrentCpp guards reproduce that synchronous C++ policy: they are disarmed on normal completion, update state before each action and have noexcept destructors. Their action services are ordinary propagating functions; the normal subtree/getter operations remain outside a noexcept bridge. A bare catch/cleanup/rethrow that lets the second throw replace the first is not used. Original FH3 registrations, funclet frames, source RTTI, foreign SEH/managed/fault/stack-exhaustion and original terminate-handler identity remain unproved.

## Transfer evidence

Every owned-body transfer and the supporting typed runtime transfer chain is retained below and in the JSON report. Runtime-only entries remain evidence, without Source ownership promotion.

```text
0055b6cc CALL 004cef40
0055b6d5 CALL 00bf65ac
0055b6fd CALL 00559280
0055b706 CALL 00bf65ac
0055b72c CALL 0055a3a0
0055b735 CALL 00bf65ac
0055b75e CALL 0055b230
0055b767 CALL 00bf65ac
0055b78d CALL 00432050
0055b796 CALL 00bf65ac
0055b7bf CALL 00432050
0055b7c8 CALL 00bf65ac
0055b7f1 CALL 00444b10
0055b7fa CALL 00bf65ac
0055b820 CALL 00559a90
0055b829 CALL 00bf65ac
0055b84e CALL 005599c0
0055b857 CALL 00bf65ac
0055b87d CALL 00559110
0055b886 CALL 00bf65ac
0055b8d8 CALL 0055b690
0055b8f5 CALL 00419cc0
0055b8fc CALL 00bd1510
006a6eb6 CALL 0055b8b0
006a6ec3 CALL 00bf65ac
00c6dc33 JMP 0055a3f0
00c6dc3e JMP 0055aab0
00c6dc49 JMP 0055aaf0
00c6dc54 JMP 00445900
00c6dc5f JMP 004324a0
00c6dc6a JMP 004324a0
00c6dc75 JMP 0055b490
00c6dc80 JMP 0055b110
00c6dc8b JMP 0055a430
00c6dca3 JMP 0041dd20
00c6dc95 JMP 00bf6b43
00c6dcad JMP 00bf6b43
0055a406 CALL 00559110
0055a40f CALL 00bf65ac
0055aac6 CALL 005599c0
0055aacf CALL 00bf65ac
0055ab06 CALL 00559a90
0055ab0f CALL 00bf65ac
00445916 CALL 00444b10
0044591f CALL 00bf65ac
0055b4a6 CALL 0055b230
0055b4af CALL 00bf65ac
0055a446 CALL 00559280
0055a44f CALL 00bf65ac
00bf6b64 CALL 00c07991
00c079f9 CALL 00c069a2
00c06a19 CALL 00c07b10
00c07aa3 CALL 00c04de2
```

## Validation and limits

Fresh verified bsp.gpr `/battlestationspacific.exe` typed metadata at modification786 has complete bodies and no no-return/thunk flags for all13 owned entries. All768 owned bytes match the installed PE; all264 disk instruction starts and canonical mnemonics match the current live listing. Seven further read-only EH spans,306B, match PE/live, including tables, handlers, scope and raw filters. The local audit retains full hashes and start evidence.

MSVC Win32 Release build and all three existing suites pass (7.91s). The JSON53 transfer rows and Markdown53 claims pass live verification, with zero failed/unchecked/warnings. No new tests are added. Current COFF confirms guard state stores before action invocations, both noexcept guard FuncInfo flags00000005 and post-pair scalar BYTE flag testing/free/original-pointer return. The native raw EH flags are1; Source metadata/frame differences are explicit, and no fault-injection or original-runtime execution is claimed. Source/static/COFF/build evidence is distinct from original binary ABI, original heap/CRT, runtime and game validation. The four files and disjoint appended ledger records are the packet; existing projection/fixture Source stays intact.
