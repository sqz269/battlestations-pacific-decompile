# CC12 canonical CRT lock initializer readiness

`00C11B5E` remains held for full Source reconstruction. Its complete 186-byte physical body has 62 instructions and 13 direct calls, but the live Ghidra listing defines only 56 starts and 12 calls. The six missing starts account for 18 bytes after two `_free` calls. Original PE bytes match live bytes throughout. This packet changes only this document and its JSON report; it does not modify Source, build files, ledgers or Ghidra, and runs no build, test or probe.

The existing name `LIBCRT_unmatched_00c11b5e` is preserved. “Canonical CRT indexed lock initializer” is an evidenced role hypothesis, not a recovered library symbol. Analysis used the verified `bsp.gpr` project and `/battlestationspacific.exe`; no callee body, fixed-data contents, scope data or handler was opened.

## Whole-body gate and listing limitation

The body is `00C11B5E..00C11C17` inclusive, SHA-256 `4867b1073c65694f072a11986820e398fd904ef4b392db165374ace232f1d8d9`. Its 11 distinct direct targets are recorded at all 13 physical call sites in the report. There are no own indirect calls. No historical export exists; the fresh saved defined listing has 56 starts and the separately labelled physical decode has all 62.

After `_free` at `00C11BE2`, Ghidra omits `00C11BE7` POP, `00C11BE8` errno-provider CALL, `00C11BED` errno store, `00C11BF3` local-result store and `00C11BF6` JMP: 17 bytes. After `_free` at `00C11BFD`, it omits `00C11C02` POP: one byte. The pseudocode's two apparent returns of `extraout_EAX` therefore omit real cleanup and failure work. Exact flow/no-return/override properties are unavailable through the typed read endpoint; the gap does not prove its cause. Root must diagnose/repair the listing under its write lock and replay all 62 starts before admitting a complete Source gate.

## Exact physical normal-path order

1. Push `0xC` and scope operand `00E03678`, call `__SEH_prolog4`, set `EDI=1`, `[EBP-0x1C]=1`, and `EBX=0`.
2. Test current heap cell `[0109E1BC]` even if the eventual descriptor already exists. Zero calls `__FF_MSGBANNER`, pushes 30 for `00C059A8`, then pushes 255 for `___crtExitProcess`. If all return, two POPs remove the still-stacked argument words and execution continues. No no-return claim is borrowed from their names.
3. Only now load the index from `[EBP+8]`; capture `ESI=(00E16478+index*8) mod 2^32`. A nonzero current `[ESI]` returns the intended success value 1 through the epilog, with no allocation or lock.
4. For zero, allocate 24 bytes via `__malloc_crt` and capture EAX in EDI. Null calls `00BFFB8B`, stores DWORD 12 through returned EAX, zeros EAX, and goes straight to the epilog without acquisition/free/common cleanup.
5. Nonnull pushes 10 for `__lock`; after its return and caller POP, set `[EBP-4]=0` and re-read current `[ESI]`. A nonzero value takes the race-loser path: free captured EDI and POP, then common cleanup. The default result local remains 1; this body does not set errno on that path.
6. If the descriptor remains zero, push 4000 and EDI for `___crtInitCritSecAndSpinCount`. Both POPs precede TEST EAX. A nonzero result publishes captured EDI to `[ESI]` without another descriptor read. A zero result frees EDI, obtains errno storage, writes 12, and sets `[EBP-0x1C]=0` before common cleanup.
7. Common cleanup sets `[EBP-4]=-2`, calls unopened `00C11C18` without explicit pushed arguments, then reads the current result local into EAX. It calls `__SEH_epilog4` and executes plain RET. The result read must remain after the unknown cleanup child.

The earlier accepted `__lock` caller plus `[EBP+8]` and RET support one caller-owned DWORD index with cdecl argument cleanup. Current metadata's zero-argument prototype is insufficient. Actual nonvolatile/FS/frame preservation depends on the prolog and epilog contracts; this is not new original-ABI compatibility proof. Index capture occurs after the heap-unavailable child chain. The captured descriptor address is retained thereafter; no bounds guard or descriptor `+4` write exists. Free/errno work happens while the local EH state remains 0, before common cleanup. Exception, unwind and fault behavior remain unproved without the genuine frame, scope, handlers and children.

## Genuine owner and provider frontier

The allocation is an original CRT 24-byte candidate. Current host `singleton_lifetime_allocate` uses `std::malloc`/`_callnewh` and throws on terminal failure; its paired host free is a different domain. The renderer's 28-byte tracked critical-section object is also different. No matching `__malloc_crt` Source was found in targeted current address/name searches.

Current `00C17643` Source directly calls `InitializeCriticalSection`, returns 1 and pops eight bytes; `00C17639` stores an already-encoded borrowed word. Neither closes `00C17653` dispatch, its real callbacks or success/failure policy. `LegacyCrtMathRuntime::errno_location` requires a genuine owning-CRT service; it is not the original PTD provider. Canonical `_free` remains held despite its prior whole-body readiness and the admitted lock-4 cleanup helper.

Existing canonical `00C11B31` unlock requires an actual initialized/acquired descriptor. It does not establish what unopened `00C11C18` does, and `00BF9E1E` releases fixed lock 4. The Source SEH epilog requires its precise valid frame; no matching prolog implementation was found. Canonical mutable pages are storage, not proof of an initialized heap, descriptor extent, lock lifetimes or bootstrap state. The SBH initializer itself requires an already-valid heap.

The accepted acquisition schedule exposes a conditional cycle: initializer calls `__lock(10)`; an absent lock-10 descriptor makes that acquisition call this initializer for 10. Computed address `00E164C8` is only an operand-derived address; its contents and seed were not read. A real bootstrap owner is required. Do not supply a host replacement lock, assume the descriptor is preseeded, or implement only the preexisting-descriptor fast path.

The JSON records every named incomplete dependency, including heap diagnostics/exit, prolog/scope/handlers, canonical heap/descriptor owners, nullable allocation, PTD/errno, acquisition, initialization dispatch, canonical free and cleanup. Children may mutate relevant state; there is no own indirect callback call, no post-initializer recheck before publication, and no evidence for invented rollback or validation.

## Next bounded prerequisite and evidence limits

The smallest useful next packet is `cc12_crt_lock_initializer_cleanup_readiness`, sole Native address `00C11C18`, owning `docs/CC12_CRT_LOCK_INITIALIZER_CLEANUP_READINESS.md` and `reports/cc12_crt_lock_initializer_cleanup_readiness.json`. Establish the whole body, implicit register/frame/stack ABI, every effect and genuine provider before naming it Source-ready. Do not infer its size, fixed lock index or ordinary calling convention. Conditional later Source files would be `include/bsp/native_crt_lock_initializer_cleanup.hpp` and `src/native_crt_lock_initializer_cleanup.cpp`; none is written or authorized by this report. Parent listing repair and the other dependencies remain required.

Sixteen selected current Source files were independently pinned and matched Root after LF normalization. The separately pinned Source534, acquisition and `_free` receipts are historical context. Source534 records 534 selected project inputs, 56 Core and two App whole objects, 85 positive Core providers and three checks; its old normal build and three-frame/two-Present `PressStartPoll` smoke are not rerun here. That smoke has zero mission frames and 88 UNIMPLEMENTED rows. Selected project inputs are not the full compiler/SDK closure.

All new Source, build, fixture, original-ABI, runtime/fault/unwind, faithful-startup and gameplay credits are zero. Full physical-byte review establishes this readiness frontier; the missing live starts and unresolved real owners prevent full Source admission.
