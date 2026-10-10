# CC12 game-state queue allocator owner review

Status: **read-only owner/ABI review; no C++ GO**. Fresh Native scope is only `004B8C90..004B8CE4`: 85 bytes and 25 instructions. Source is frozen at `a8ee7e1b5ba9c2e071327a9e9c3f176cee8c2a8c`, which merges main `68cfbc7d5` with the preceding growth worker commit. No C++, build, test, runtime, SDK/OS provider execution or Ghidra mutation is included.

This body closes the growth caller's allocation scaling: **ECX is a DWORD count; admitted counts request count × 4 bytes**. Incoming EDX is unused. Zero count still calls the allocation provider. Normal return forwards its EAX without a null check or initialization. Overflow constructs a stack exception and reaches a throw-provider boundary. An existing Source allocation service is available for reuse; this does not establish a production queue binding. The next proposed Native frontier is `004CFE20`, under a separate lease.

## Evidence and dependency epochs

Seven retained typed live queries capture prototype, xrefs, callers, callees, complete bytes, complete listing and pseudocode. Each live client verifies project/program/language/image base against the checked configuration: `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`, x86 and `00400000`; autostart is disabled. The project file path is configuration evidence, not an independently exposed server fact.

The complete raw decode matches the live listing and the retained original PE exactly. There is no listing gap in this body. No exception metadata, vtable, callee body, or byte beyond `004B8CE4` was opened. The last instruction is the five-byte call at `004B8CE0`.

The sole reported direct caller/xref is growth call `004D211E`. The capture still names its owner `FUN_004d20d0`: it predates Root's later annotation of that caller. The packet preserves that metadata epoch rather than silently refreshing labels. The three callee names are `operator_new` (`00BF681B`), `exception` (`00BF6340`) and `__CxxThrowException@8` (`00BF6885`). Names and pseudocode labels are qualified separately from fresh caller-body observations.

Growth worker `2d7031ba3609c8cba61cc6c81e94f5d1897cdfc1` was accepted and integrated by Root at `37866e793bf90064ec0a083fa8799b6935997c0b`. Root's whole primary report/document and the unchanged whole worker report are pinned Git dependencies. Acceptance arrived while this packet's Source capture was running: `integration_acceptance.json` supersedes the capture's earlier pending scheduling status. Growth's 341-byte body is retained for PE identity only; neither its decode nor Root's historical proof trees are replayed here.

## Count, byte size and incoming registers

The effective count is incoming ECX. No incoming stack argument is loaded. Incoming EDX is overwritten on both paths before a value-dependent use; its zero value from growth is not interpreted as a flag, pointer or mode. This establishes the effective input dependency, not the original source declaration's parameter arity.

`SUB ESP,10h` reserves local storage. `TEST ECX,ECX` clears carry, so `JA` at `004B8C95` means count is nonzero, including counts with the high bit set. It is not a signed positivity test.

For count zero, `XOR ECX,ECX` falls through to `LEA EDX,[ECX*4]`, pushes zero and calls `00BF681B`. There is no zero-size early return. Neither a null result nor a non-null result is guaranteed by this wrapper.

For a nonzero count, the body executes:

```text
004B8CAD  OR EAX,FFFFFFFFh
004B8CB0  XOR EDX,EDX
004B8CB2  DIV ECX
004B8CB4  CMP EAX,4
004B8CB7  JNC 004B8C99
```

The dividend is unsigned `0:FFFFFFFF`. Division by zero is excluded by the preceding branch; the quotient fits a DWORD. For positive `C`, `floor(UINT32_MAX/C) >= 4` is equivalent to `C <= floor(UINT32_MAX/4)`.

| Incoming count | Local outcome |
|---|---|
| `0` | Call allocation provider with byte size zero |
| `1..0x3FFFFFFF` | Call allocation provider with byte size `count*4` |
| `0x40000000..0xFFFFFFFF` | Reach exception construction and throw-provider call |

The highest admitted byte size is `0xFFFFFFFC`; admitted multiplication does not wrap. This interval derivation is arithmetic evidence, not Native execution or an exhaustive input test.

The allocator's `0x3FFFFFFF` count limit is distinct from growth's accepted `0x0FFFFFFF` capacity limit and conditional promotion fallback. It must not be used to relax or replace the growth guard.

## Normal allocation and return

At `004B8C99`, `LEA EDX,[ECX*4]` computes the byte request. `PUSH EDX` supplies one stack DWORD to `operator_new` at `004B8CA1`. On normal provider return:

```text
004B8CA6  ADD ESP,4
004B8CA9  ADD ESP,10h
004B8CAC  RET
```

EAX is unchanged after the provider. A returned null pointer would be forwarded; the wrapper neither checks it nor substitutes a fallback. Whether the provider returns null, retries or throws remains its contract. The body performs no local initialization of returned storage, count prefix, per-element construction, ownership publication or free. Growth performs its own later map copies, fills, release and publication.

The ordinary return is plain `RET`, with no argument pop. Under the observed cdecl provider convention (zero callee argument pop), the wrapper restores entry ESP and consumes only the return address. This closes the growth caller's allocation cleanup assumption. EBX, ESI, EDI and EBP are not referenced locally; provider preservation remains an external ABI requirement. ECX, EDX and flags have no promised result contract. The final stack-adjust instructions themselves affect flags.

Live metadata prints `undefined FUN_004b8c90(void)` while the decompiler emits a fastcall count interface. Neither replaces the instruction-level effective ABI evidence or establishes a drop-in C++ declaration.

## Overflow and exception boundary

Let `S` be entry ESP. On overflow the body forms a local null-message slot and constructor receiver:

- `004B8CB9..004B8CC1`: push address `S-16`, set ECX to `S-12`, write DWORD zero at `S-16`.
- `004B8CC9`: call the named exception constructor at `00BF6340`.
- `004B8CCE..004B8CD8`: push literal `00E03CC0`, form and push the exception-object pointer, then write literal `00D6923C` to its first DWORD.
- `004B8CE0`: call `__CxxThrowException@8` at `00BF6885`.

Constructor cleanup is a qualified inference. If its callee argument pop is `k`, ESP after the constructor is `S-20+k`. The later object pointer and vtable store use `S-16+k`. For those to address the constructor receiver `S-12`, `k` must be four. With that normal thiscall assumption, the throw arguments are object `S-12` and metadata `00E03CC0`, and caller ESP immediately before the throw call is `S-24`. The constructor body was not inspected.

Pseudocode labels `00D6923C` as `std::bad_alloc::vftable`. The fresh raw body proves the two literal addresses and their use; it does not prove vtable contents, throw metadata layout, catch/destructor behavior or native CRT identity. No such data was opened.

Ghidra marks the throw helper nonreturning. This body ends at the call; if the helper returned, fallthrough would begin at **`004B8CE5`, outside this packet**. There is no evidenced local cleanup/return for that case. The audit terminates at the provider boundary instead of manufacturing an epilogue or expanding the lease. No unwind or native exception compatibility is claimed.

The complete audit checks 25 instructions, two explicit memory-write sites (both stack locals), three call sites, two conditional branches and four pushes. There are no explicit memory-read operands apart from LEA address formation; implicit stack traffic and provider reads are not included in that count. It records two ordinary return paths and one external throw path. The latter's stack joins use the stated constructor cleanup assumption, not newly verified provider ABI.

## Source reuse and ownership limits

All 4,206 tracked `src`/`include` files were checked against the frozen Git tree and working bytes. Eleven prior overlays remain byte-identical. Exactly two further files match Root's accepted `Source3352-particle-model-typed-views` manifest:

- `include/bsp/native_particle_model_construction.hpp`
- `include/bsp/native_particle_model_lifetime.hpp`

They use `NativeParticleModelManagerStorage* const volatile&` for the borrowed manager publication; construction also adds its forward declaration. These are accepted epoch qualifications, not new queue behavior or fresh build credit. The packet retains both headers, the complete accepted manifest, and whole worker/primary reports without traversing their Root proof trees.

The primary whole-file Source closure contains 394 project files from 22 roots and 726 quoted-include edges. A separate closure of the two epoch headers contains 40 project files and 57 edges, reconstructed from the same complete frozen corpus and overlays. External angle-bracket SDK includes are listed, not claimed as a frozen SDK closure. Ten full-corpus queries reproduce 940 matches; no `004B8C90` or `004CFE20` identifier occurs. Thirty-three Source excerpts and six retained-document excerpts replay from whole files.

There is already a concrete allocation service in `singleton_lifetime.cpp`:

```text
singleton_lifetime_allocate({kind, native_bytes, host_bytes})
singleton_lifetime_free(pointer)
```

Its Source loops on `malloc(host_bytes)`, calls the actual host CRT `_callnewh(host_bytes)` when allocation fails, retries when permitted, and throws `std::bad_alloc` when declined. Free uses `std::free`. The API explicitly distinguishes Native byte evidence from host object size.

The same file's private `allocate_slots` checks `count>0x3FFFFFFF` and requests `count*4` Native bytes versus `count*sizeof(void*)` host bytes for the distinct `00BCFEB0` owner. This is a reuse candidate with analogous count math, not a newly analyzed Native body or an existing `004B8C90` binding. Retained `SINGLETON_LIFETIME.md` explicitly leaves allocator exhaustion/new-handler behavior and native SEH/CRT identity unvalidated. This packet inherits no historical build/fixture credit.

`game_frame_control.cpp` still grows a vector of unique pointers and does not call this allocation service. The actual raw queue owner, matching release/copy binding, growth failure route, current-map reloads and free/current-capacity/new-map publication order remain distinct requirements. Automatic press-start adapters, borrowed `Impl`, current/captured profile owners and deferred/reentrant shutdown retain their previously established limits. This allocation wrapper does not close them.

The next smallest proposed Native review is **`004CFE20`**, growth's unopened limit-failure helper. No callee expansion is part of this packet. A general C++ or production activation GO is not granted.

## Reproduction

The report is `reports/cc12_game_state_queue_allocator_owner_review.json`. All actual replay inputs are retained under `local/cc12_game_state_queue_allocator_owner_review/`; preserve its relative paths plus the report. Old absolute Source/Root paths remain provenance only.

```text
python local/cc12_game_state_queue_allocator_owner_review/verify.py
python local/cc12_game_state_queue_allocator_owner_review/verify.py --original-worktree
```

The first verifier guards reads to the retained directory/report and rejects writes, subprocesses, network access and further library loading after initialization. It reproduces complete Source identities, thirteen overlays, queries, both quoted-include closures, PE/body identity and the full instruction/stack audit. The separate original-worktree mode checks actual Git objects, current working Source and the accepted Root dependency. A physical copy to a fresh directory followed by guarded replay is also recorded.

These checks establish retained evidence identity and internal audit consistency. They do not establish Native execution, provider exhaustion, exception/unwind identity, game validation or binary replacement compatibility.
