# CRT lock acquisition readiness

The complete `__lock` body at `00C11C21..00C11C51` is verified: **49 bytes, 20 instructions, two direct calls, one imported call and one plain return**. New Source reconstruction remains held on the genuine indexed initializer `00C11B5E` and failure child `00BFBA09`. The next bounded prerequisite is a complete read-only review of `00C11B5E`; its implementation is not yet Source-ready.

## Evidence and ownership

This packet owns only Native `00C11C21` and the two readiness files, based on `4abacea00284480297303147ba6916a4b9ba8ae7`. Verified read-only CLI batches use `C:/Users/sqz269/bsp.gpr`, project `bsp`, program `/battlestationspacific.exe`. Preserve the existing library name **`__lock`**, including its `void __cdecl __lock(int _File)` metadata; `_File` does not establish a game file identifier.

All 49 installed-PE bytes match live Ghidra bytes, and all 20 physical Capstone starts match exact live instructions and the fresh saved local listing. No historical shared export existed. The saved evidence is explicitly a new local copy of this verified live listing; no shared export or GPR mutation occurred. Native SHA-256: `b02c4573212abb78581b610296b1ae8a133d9790a58512b09ab646b4a78e2798`. Original PE SHA-256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.

The full listing has no gaps. Typed metadata reports 20 instructions and two direct calls; the third physical call is through `[00CE2218]`. Exact Ghidra flow-override/no-return properties were not exposed by the typed flow check and are not asserted. Lookup graph labels do not replace the physical call evidence. No child bodies, fixed data contents, handlers or other Native addresses were opened.

## Original ABI and ordered behavior

The caller supplies one raw 32-bit index word at entry ESP+4 and removes it after the plain `RET`. The body saves EBP and ESI, captures the index in EAX, and computes `ESI = 00E16478 + 8*index` with x86 wrapping. There is no index-range or sign check; neither the table extent nor the second descriptor DWORD's meaning is established. The computed descriptor address remains captured across calls.

| Route or operation | Exact behavior |
|---|---|
| Initial descriptor check | `CMP DWORD[ESI],0`; nonzero jumps to acquisition |
| Zero descriptor | Push captured index, call `00C11B5E` |
| Initializer result | **`TEST EAX,EAX` precedes `POP ECX`**; `JNZ` consumes those preserved test flags |
| Zero initializer result | Push `0x11`, call existing-name `__amsg_exit` at `00BFBA09` |
| Failure-child continuation | Physical `POP ECX` remains, followed by acquisition **if that child returns** |
| Acquisition | Push **current** `DWORD[ESI]`, call `[00CE2218]`, labelled `EnterCriticalSection` |
| Normal return | `POP ESI; POP EBP; RET`; no meaningful EAX status |

The final descriptor load must remain a fresh read even when the initial pointer was nonzero. Initializer success is decided by EAX, so a nonzero result reaches the API even if the current descriptor is zero. Conversely, zero EAX invokes the failure child even if it published a nonzero pointer. If the failure child returns normally, the API receives the current descriptor without another check. Its actual termination, callback, exception and return policy remains unproved.

Both direct children are called with one caller-cleaned word. The imported API route requires callee cleanup of its pointer word before the local register pops. EAX/ECX/EDX and arithmetic flags retain the final import's residual state; own pops/return do not change arithmetic flags. ESI/EBP restore their entry values, and own code does not write EBX/EDI. Child/import preservation contracts remain external.

There is no own descriptor write, lock allocation, retry/spin loop, SEH frame, rollback or exception conversion. No extra callback dispatch is visible beyond the two direct children and the IAT call. Any lazy publication, race handling, callback selection or partial failure effects inside the unopened children remain unresolved.

## Current Source availability

The canonical `00C11B31` unlock and admitted `00BF9E1E` helper call real `LeaveCriticalSection`, requiring an initialized already-acquired section. They provide release behavior and establish no indexed initialization/acquisition owner.

`native_crt_critical_section_primitives` implements `00C17643` over caller-provided actual storage and `00C17639` as an already-encoded-word store. These are concrete lower-level Source providers, with no demonstrated `00C11B5E` mapping, allocation, publication or policy. The tracked renderer factory `00BD1860` allocates a separate 1Ch object through its host allocation service, initializes it and clears depth+18h; it is not the canonical CRT indexed descriptor domain.

`GameNativeRendererScalarProcess` resolves actual kernel32 `EnterCriticalSection` and exposes a stable current Source import cell. That proves a genuine Source API provider, without proving the original fixed IAT contents or CRT lock lifetime. `GameNativeMutableCrtData` explicitly distinguishes mapped image data from initialized callable CRT state. Its page ownership is insufficient here.

No `00C11B5E` or `00BFBA09`/`__amsg_exit` implementation or declaration was found by targeted address/name searches in current `src` and `include/bsp`. Existing `_invalid_parameter_noinfo` Source policy belongs to a different entry and is not a substitute for `__amsg_exit(17)`. Twelve inspected provider files were pinned and match the integrator after LF normalization; no provider artifacts or runtime behavior were replayed.

## Smallest next prerequisite and limits

Proposed next packet: `cc12_crt_canonical_lock_initializer_readiness`, owning only `00C11B5E`, `docs/CC12_CRT_CANONICAL_LOCK_INITIALIZER_READINESS.md` and `reports/cc12_crt_canonical_lock_initializer_readiness.json`. Its caller-derived contract is one captured raw index word with caller cleanup, EAX zero/nonzero failure selection, preserved ESI descriptor address, and publication observable through the subsequent current descriptor read. Establish the full body, real allocation/initialization/callback domain, ordering, publication and failure policy before choosing any Source implementation. No size, thunk status, child list or Source readiness is assumed.

`00BFBA09` remains a separate required failure dependency for the complete parent. Restricting the parent to a preinitialized fast path, declaring unresolved children, using host replacement locks, or substituting another failure service does not close it. New Source, original ABI, lock-owner, full-free/destructor, runtime/fault, faithful-startup and gameplay credits remain zero. This packet performs no C++, CMake, ledger, GPR, build or test work.

Historical context is kept separate: accepted Root Source534 receipt `reports/cc12_model_class_unlock_source_primary_review.json`, SHA-256 `9fc3a11764a53ed6c100471213d97509b47f510617d6bea9c32f39c5b4c7f218`, records 534 selected project inputs, 56 Core plus 2 application objects, 85 positive Core definitions and 3 checks from the `2026-10-09T22:50:47.603655..22:51:08.134682Z` normal build. Its smoke recorded 3 frames, 2 Presents, exit 0 in PressStartPoll/state 2 and zero mission frames. This historical receipt was pinned, not replayed against current artifacts; current provider Source facts have their own pins.
