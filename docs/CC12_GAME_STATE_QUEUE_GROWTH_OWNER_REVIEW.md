# CC12 game-state queue growth owner review

Status: **read-only owner/order review; no C++ GO**. The only newly analyzed Native body is `004D20D0..004D2224` (341 bytes). The frozen Source baseline is `f5b0141fc3b96a5066b33f64b93726a7d3296621`. No C++, tests, build, game execution, provider analysis or Ghidra mutation is included.

The growth routine preserves a concrete raw queue contract that the existing semantic helper does not establish: a bounded growth preference, a register-input allocation call, late map/capacity/argument reloads, four copy-provider sites, explicit zero fills, then **free current old map, add to current capacity, publish replacement map**. A general production binding remains blocked on the provider and owner contracts. The next smallest Native frontier is `004B8C90`, under a separate lease.

## Evidence and listing boundary

Ten recorded typed queries use the configured `C:/Users/sqz269/bsp.gpr`, program `/battlestationspacific.exe`, x86 language and `00400000` image base, with autostart disabled. Each live client verifies project/program/language/image base; the project file path is checked configuration, not a separately exposed server fact. The packet retains complete prototype, xref, caller, callee, bytes, three disassembly pages and two pseudocode pages. It does not open any provider body or data table.

The live 341 bytes exactly match the retained original PE. Independent decode covers **129 instructions**, while the live Ghidra listing has **128 rows**. Its sole gap is:

```text
004D2209  CALL 00BF65AC
004D220E  83 C4 04       ADD ESP,4   <- absent from listing, present in verified bytes
004D2211  ADD [EBX+8],EDI
...
004D221B  MOV [EBX+4],EDX
004D2222  RET 4
```

The pseudocode's non-null-map branch ends immediately after `_free`. The verified bytes establish the normal-return continuation through argument cleanup and both publication writes. This packet records the mismatch without repairing the listing, signature or no-return metadata. The live prototype remains `undefined FUN_004d20d0(void)` and is not the recovered ABI.

One direct caller/xref is reported: `004D3EEF` within `004D3ED0`. The four named callees are `004B8C90`, `004CFE20`, `00BF65AC` and `00BF67A7`. Those names are routing evidence, not fresh proofs of allocation, exception or CRT behavior. The accepted enqueue report is retained whole; its 117-byte body is checked only for PE identity, without fresh caller decoding.

## Receiver, argument and growth preference

Entry `ECX` is the raw queue, captured in `EBX`; entry stack `[S+4]` is the requested increment `R0`. The body saves four registers, reserves eight local bytes and returns with `RET 4`. This closes the enqueue caller's prior growth-cleanup assumption. It does not establish a drop-in C++ ABI or unwind contract.

Let `B0` be the initial DWORD at queue `+08`, and `M=0x0FFFFFFF`. The initial unsigned test is exactly `u32(M-B0) >= R0`. If false, the body calls `004CFE20`. Its name suggests a length-failure helper, but its behavior, exception type and no-return property are unopened. If it hypothetically returns, the body uses its resulting `EAX` and `EDX`; the original values are not restored. No returning error behavior is invented.

On the admitted path, the preferred increment is `I=max(8,B0>>1)`. The routine promotes `R0` to `I` only when both `R0<I` and unsigned `B0<=u32(M-I)` hold. Otherwise it retains the requested increment. Promotion overwrites the actual incoming stack argument at `[S+4]`. This is a remaining-capacity fallback, not unconditional promotion. For example, with well-formed `B0=0x0FFFFFFE` and `R0=1`, the preferred `0x07FFFFFF` does not fit, so the increment stays one. This is an arithmetic illustration, not executed allocation evidence. For malformed `B0>M`, the initial subtraction still wraps; the body does not add a separate invariant check.

At `004D2113`, the head DWORD at queue `+0C` is captured and unsigned-shifted by two into `H`. Call `004B8C90` receives `ECX=u32(EAX+EDX)`, `EDX=0` and no stack arguments. Its `EAX` result is saved at `[S-8]` as the new map pointer. The call does not establish a byte count, allocation family, null/failure behavior, initialization or matching release. There is no local null-result test.

## Copies, fills and current reads

All arithmetic below is the body's 32-bit arithmetic. `s32(x)>>2` denotes the signed arithmetic shift; `u32` denotes wrap. Simplified forward ranges require valid nonoverlapping queue/stack/new-map storage, stable relevant fields and argument, successful providers, preserved nonvolatile registers and DF clear at each repeated store.

After allocation, the body reads **current** map `M1` at `+04` and **current** capacity `B1` at `+08`. These are not snapshots taken with `B0`. With returned pointer `D`, it computes:

```text
L1 = s32(u32(4*B1 - 4*H)) >> 2
N1 = u32(4*L1)
T1 = u32(D + 4*H + N1)
```

It saves `T1` at `[S-4]` and, if `L1!=0`, calls `00BF67A7(D+4*H, N1, M1+4*H, N1)` at `004D2159`. The argument order is destination, destination size, source, count. The saved tail is reloaded while the four call arguments are still on the stack, then those arguments are discarded.

Only after this optional copy, `004D2165` reloads increment `R` from the incoming `[S+4]` slot. It need not equal the allocator's earlier increment if provider or alias effects changed that memory. The branch tests unsigned `H>R`.

For `R>=H`:

- Reload current map `M2` at `004D216D`. With `Lh=s32(u32(4*H))>>2`, optionally copy `u32(4*Lh)` bytes from `M2` to `T1` at `004D2183`.
- Zero `R-H` DWORDs from `T1+u32(4*Lh)` at `004D2195` if the difference is nonzero, then reload `[S+4]` into `EDI`.
- If `H!=0`, zero `H` DWORDs from the saved new-map pointer through shared `004D21FB`, then reload `[S+4]` again.

For `R<H`:

- Reload current map `M2` at `004D21A7`. With `Lr=s32(u32(4*R))>>2`, optionally copy `u32(4*Lr)` bytes from `M2` to `T1` at `004D21C0`.
- Reload current map **again** at `004D21C8`, including after that provider call. Compute `L2=s32(u32(4*H-4*R))>>2`, `N2=u32(4*L2)`. If `L2!=0`, call `00BF67A7(saved D,N2,current map+4*R,N2)` at `004D21E9`.
- If held `R!=0`, zero `R` DWORDs from `saved D+N2` through shared `004D21FB`, then reload `[S+4]`.

The four copy sites pass equal destination-size and count arguments and do not test `EAX` results. They move four-byte map entries; they do not locally copy the pointed 16-byte element blocks. The two `REP STOSD` sites write zeros, use unsigned `ECX` repetition counts and advance or decrement `EDI` according to DF. There is no `CLD` in this body. Malformed spans, alias effects, partial writes, provider errors and DF changes are not converted into safe STL behavior.

Under the stated stable valid-state assumptions, the rotations simplify to:

| Arm | Map entries copied | New map entries zeroed |
|---|---|---|
| `R>=H` | old `[H,B)` to new `[H,B)`; old `[0,H)` to new `[B,B+H)` | `[B+H,B+R)` and `[0,H)` |
| `R<H` | old `[H,B)` to new `[H,B)`; old `[0,R)` to new `[B,B+R)`; old `[R,H)` to new `[0,H-R)` | `[H-R,H)` |

These ranges explain the semantic rotation. They do not replace the exact signed shifts, current reads or provider boundaries above.

## Free and publication order

After every copy/fill arm joins, `004D2201` reloads current queue map once more. If nonzero, it calls `00BF65AC` on that value while the old map remains published in the queue. On normal return the missing-listing `ADD ESP,4` discards the argument.

Then `004D2211` adds held `EDI` to the **current** queue capacity. Only afterward does `004D2214` reload the saved pointer slot `[S-8]`, and `004D221B` publishes that value at queue `+04`. A provider may have changed current map/capacity; the body does not force them back to initial values. Stack slots are memory too, so arbitrary aliases are not silently excluded. Free sees the old publication; capacity changes before the replacement map is published. There is no local rollback.

The routine has no explicit head/count write and no explicit current-game or callback-global read. It retains the passed queue owner in `EBX`. Effects through providers or aliases remain possible. `EDX` ends with the late loaded map value; `EAX` is zero when free is skipped and otherwise has the unresolved free-call result. No semantic return type is inferred.

The audit verifies all 129 instructions, seven explicit memory-write sites (including two REP sites), 17 explicit reads, seven calls, 13 conditional branches, one direct jump and the flag sources used across intervening LEA/MOV/PUSH instructions. It enumerates 768 syntactic normal-return paths and verifies unique stack joins with `S+8` after `RET 4`. Calls are explicitly assumed to return normally with zero callee argument pop; REP is a symbolic operation. These counts do not establish path feasibility, callee behavior or unwind safety.

## Frozen Source comparison and binding boundary

All 4,206 tracked `src`/`include` files were checked against the frozen Git tree and working bytes. The eleven accepted overlays remain unchanged: three Source3350, six Source3352 and the two Source3352-particle-startup files (`game_hosts.hpp`, `game_hosts.cpp`). The packet retains both complete Source archives and manifests plus exact overlays. Pending separate header work is excluded. The selected whole-file closure has 393 project files from 20 roots, with 725 quoted-include edges. External angle-bracket SDK includes are listed, not claimed as a frozen SDK closure. Nine complete-corpus queries reproduce 297 matches; the `004B8C90`/`004CFE20` identifier query has zero matches. Twenty-seven Source excerpts and three retained-document excerpts are replayed from whole files.

`grow_state_request_map_004d20d0` currently chooses `max(8,blocks.size()/2)`, unconditionally promotes a smaller request, allocates an initialized vector of `unique_ptr` entries, moves them using modular indices, and replaces the vector. This explains the stable valid-state rotation. It does not prove the native remaining-capacity fallback, failure provider, raw layout, register allocator, checked-memory call boundaries, ignored results, alias timing or free/capacity/map publication order. The accepted enqueue review's late pointer/value-read difference remains separate; a literal title request is not thereby declared wrong.

Existing Native construction Source initializes the raw queue header at game `+5DC/+5E0/+5E4/+5E8`. Teardown routes game `+5D8` through `004C4A50` and a supplied free adapter. That is retained Source evidence, not fresh proof of matching allocation/release. `GameStartupHost` still owns a separate `GameStateSlot` passed to `GameMenuHost`, whose requests use the semantic queue. A production raw queue mutation owner has not been established by this packet.

The four automatic `MenuPressStartHost` constructions and borrowed `Impl` remain; menu is deleted before frontend. Current versus captured profile owners, synchronous/deferred nested completion and the three shared callback slots retain their previously accepted boundaries. Queue growth does not provide missing lifetime, cancellation, teardown or late-owner resolution policy. No such behavior is invented.

The next proposed bounded read is **`004B8C90` only**, to close its `ECX`/`EDX` allocation boundary. `004CFE20` failure behavior and matching release/copy behavior remain separate frontiers. Any production C++ packet still needs raw ownership, provider and lifetime closure; this review authorizes none.

## Reproduction

The report is `reports/cc12_game_state_queue_growth_owner_review.json`. Its evidence is self-contained under `local/cc12_game_state_queue_growth_owner_review/`. Preserve that directory's relative paths and the report when relocating. Whole prior enqueue and startup worker/primary reports are pinned Git dependencies; their historical Root proof trees are not recursively opened or replayed.

```text
python local/cc12_game_state_queue_growth_owner_review/verify.py
python local/cc12_game_state_queue_growth_owner_review/verify.py --original-worktree
```

The first command guards all reads to the retained directory and report, and rejects writes, subprocesses, network access and library loading after initialization. It rebuilds Source identities, overlays, queries, quoted-include closure, PE/body identity, complete decode, listing-gap and control-flow audit using retained artifacts. The second separately checks actual Git objects and current working Source in the original worktree. The packet also records a physical copy to a fresh directory followed by the guarded verifier there.

These are evidence-replay checks, not compiler, fixture, Native, SDK/OS, game or drop-in ABI validation. No GPR write or additional Native body analysis is part of any replay.
