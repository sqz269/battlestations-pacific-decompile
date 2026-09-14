# Native SBH allocation (BW discovery)

`C12968` is now recovered as a complete 739-byte allocation body, together with its only two direct helpers: `C1207C` (176-byte new region) and `C1212C` (262-byte new group). All 1,177 bytes match the installed PE and live Ghidra; all 405 instructions are present in the stored listings. There are two direct CALLs and five actual Win32 IAT calls, with no omitted edges or indirect callbacks. This is discovery, not a source implementation or runtime validation.

The three bodies are candidates for one complete source packet using actual borrowed native heap/state and real HeapReAlloc, HeapAlloc, HeapFree and VirtualAlloc. Their original bootstrap, canonical state ownership, lock-4 acquisition, matching free integration and native exception-frame identity remain separate. Existing `singleton_lifetime_allocate` / `singleton_lifetime_free` use a host CRT policy and do not implement these native SBH bodies.

Base `e9c7792df0bc8a213b6b13c88c206575f2b1eb4d`; worktree `orch2-native-crt-sbh-alloc-bw`. Every live batch used the supported BSP CLI and verified `C:/Users/sqz269/bsp.gpr`, `/battlestationspacific.exe`. Original PE SHA256: `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`. The report records exact spans, explicit memory-write rows, complete instruction artifacts, call rows, source boundaries and two complete SHA256/SHA512 local inventories.

Corrected allocator discovery `40d4693c9a94f236ebb932a91ca89e2bc10eb801` is retained unchanged: report SHA256 `e9e30237c1ee8c72e8240fab16afb5e50f907b29f747d50dec50f232eedcf5e7`, 1,259 original artifacts plus its doc/report. This includes the prior storage, PTD and lock evidence. Its 13 source snapshots still match this worktree's pinned base. The free worker owns `C11D68`; this packet neither claims nor reconstructs that body or separately owned shared-state providers.

## ABI and physical coverage

| Entry | Complete range | Interface established by the bytes |
| --- | --- | --- |
| `C12968` | `[C12968,C12C4B)`; 739 bytes / 261 instructions | cdecl `void* alloc_block(uint32_t requested_bytes)`; original argument at entry ESP+4; EAX payload pointer or zero; RET without stack-argument cleanup. |
| `C1207C` | `[C1207C,C1212C)`; 176 bytes / 53 instructions | cdecl `descriptor* alloc_new_region()`; no argument; EAX descriptor or zero. |
| `C1212C` | `[C1212C,C12232)`; 262 bytes / 91 instructions | cdecl `int32_t alloc_new_group(descriptor*)`; EAX group index or -1; RET. |

Normal exits preserve the nonvolatile registers used by each body. Saved Ghidra prototypes omit parameters and the allocator pseudocode types its size as a pointer; neither is a valid source interface. `C12968` computes its size before repeatedly overwriting its argument slot with the selected descriptor; later `[EBP+0Bh]` is scratch for an old byte count. Those stack writes are retained in the instruction evidence, not mistaken for stores into the allocation caller's object. Descriptive source names and an added borrowed context would remain new C++ interfaces, not recovered symbols or drop-in ABI proof.

## Native layout and state

Let `D` be a 20-byte descriptor, `R = [D+10h]` its 41C4h-byte metadata allocation, `g` a group index, and `G = R+144h+g*204h` the group metadata. The 1 MiB region at `[D+0Ch]` contains 32 groups of 8000h bytes. Each committed group contains eight 1000h-byte pages.

| Storage | Meaning supported by allocation instructions |
| --- | --- |
| `D+0`, `D+4` | Summary masks for nonempty size classes 0..31 and 32..63. Class order is MSB first. |
| `D+8` | Available/uncommitted group mask, also MSB first. |
| `D+0Ch`, `D+10h` | Reserved region base and metadata pointer. |
| `R+0` | Cached current group index, with -1 sentinel. |
| `R+4..R+43h` | 64 byte counters: number of groups with a nonempty list in each size class. |
| `R+44h+4*g`, `R+0C4h+4*g` | Group low/high class masks. |
| `G+0` | Allocated-block count for this group. |
| `S = G+8*b`; `S+4`, `S+8` | Virtual sentinel for class `b`, with next and previous links. The sentinel's nominal size word is unused. |
| Free block `F+0`, `F+4`, `F+8`, `F+size-4` | Size, next, previous, and matching size footer. |

Actual global words read or written here are heap `0109E1BC`, descriptor count `0109ED64`, descriptor-array base `0109ED68`, allocation rover `0109ED70`, descriptor capacity `0109ED74`, retained-empty group index `0109ED78`, and retained-empty descriptor `0109E310`. No private copies or new owners of these words are introduced.

## Allocation search and provider failures

At `C1296E..C12985`, the allocator captures an end pointer from current count times 14h plus current array base, and calculates `N = (requested_bytes + 17h) & FFFFFFF0` with 32-bit wrap. It uses signed arithmetic shift of `N` by four, then subtracts one to form the requested class. For class less than 32 by signed comparison, the low mask is `FFFFFFFF >> class` and the high mask is all ones; otherwise the low mask is zero and the high mask is `FFFFFFFF >> (class-32)`. x86 shift counts use their low five bits. There is no overflow, range or zero-mask guard. Under the valid SBH class domain, `N` includes eight bytes of boundary tags and is rounded to 16 bytes.

The descriptor search captures the current rover once, scans rover to captured end for any eligible summary bit, then rereads the current array base and scans to the captured rover. If no class is available, it searches nonzero `D+8` masks in the same two ranges, beginning from the captured rover. Comparisons are unsigned; end/count/address arithmetic wraps. Repeated argument-slot stores do not reread the original requested size. A valid entry state must supply an in-range rover and consistent masks; the body adds no corruption recovery.

If no descriptor has an available group, CALL `C1207C` at `C12A28` selects a new region. A null result returns zero without publishing the allocator rover. Otherwise CALL `C1212C(D)` at `C12A3E` creates a group. The caller then rereads `[D+10h]`, stores the returned group index into that current metadata's first DWORD, rereads `[D+10h]` again, and tests the stored first DWORD for -1. Failure therefore publishes -1 before returning zero. It does not free the region or roll back the helper's effects. Only after that check, or after finding an already available class, does `C12A51` publish `0109ED70 = D`.

The allocator next reads `R.current_group`. If it is not -1 and its group masks contain an eligible bit, it uses that group. Otherwise it scans group masks from index zero until a match; there is no explicit 32-group termination check. It takes the first eligible class by shifting the chosen nonzero mask left until its sign bit is set, using a base class of zero or 32. This is a concrete MSB-first search, not a generic best-fit container.

## Ordered free-list, bitmap and tag mutations

The selected free block is the first node `[G+8*b+4]`. Read its size and compute remainder `T = old_size-N`; the new remainder class is signed `min((T >> 4)-1,63)`. The allocator carves the allocated block from the **high end**, at `F+T`.

If the remainder class equals the selected class, `C12AF2` bypasses all unlink/reinsert and class-mask updates. The block remains linked and its remainder tags are updated when `T != 0`. In the ordinary valid class domain this includes large remainders remaining in saturated class 63.

Otherwise, before unlinking, compare the selected node's next and previous links. Equality marks the last node in that class. The allocator clears the selected group's class bit first, decrements the corresponding byte counter in `R`, and clears the descriptor's class bit only if that decrement reaches zero. The byte decrement and masks are exact x86 operations; no saturation or defensive check is added.

Unlink order at `C12B5C..C12B6F` is significant: compare `T` to zero; load current `F.prev` and `F.next`; store `F.prev->next = F.next`; then **reload** current `F.next` and `F.prev`; store `F.next->prev = F.prev`. The later `JZ` consumes the earlier remainder comparison flags. A zero remainder skips reinsertion.

For nonzero `T`, use sentinel `S = G+8*remainder_class`. Capture `S.next`, then store `F.prev=S`, `F.next=captured_next`, `S.next=F`, reload `F.next`, and store that node's previous link to `F`. Reload `F.next` and compare with `F.prev` to determine whether the new class was empty. If it was, capture and increment the class's byte counter, publish the incremented byte, set the descriptor summary bit only if the old byte was zero, and then set the selected group's class bit. The high and low mask paths retain their original address/bit calculations.

Only after those list/mask changes, `C12BFD/C12BFF` write nonzero remainder tags `T` at `F` and `F+T-4`. `C12C10/C12C12` then write `N+1` at the allocated block's header and footer. The low bit marks allocation; no payload bytes are zeroed here. `C12C19..20` reads the old group allocation count and writes old+1. If the old count was zero and the current retained-empty descriptor and group words match `D,g`, `C12C37` clears only `0109E310`; it does not reset `0109ED78`. Finally `C12C41` publishes `R.current_group=g`, and EAX becomes the payload address `F+T+4`.

There are no local locks, cleanup frames, callbacks, new-handler calls, errno writes or catches in these three bodies. Retained `BF9E56` supplies lock 4 and its scope outside `C12968`; this discovery does not broaden that to native asynchronous-fault or frame identity. A faithful source packet must not add transactional rollback, zero-filled payloads, an allocation exception on these null returns, or cached field accesses that change the documented order.

## New-region helper and partial publication

`C1207C` compares the current descriptor count with current capacity. Equality calls actual HeapReAlloc (`CE20E0`) with current heap, flags 0, current array, and wrapped `(capacity+16)*20` bytes. Null returns zero. Success first increments **current** capacity by 16, reloads current count into ESI, then publishes the returned array base. Those updates remain even if a later operation fails. The helper contains no rover adjustment.

It computes candidate descriptor `array + captured_count*20`, then calls actual HeapAlloc (`CE20F8`) with current heap, flags 8, size 41C4h. It stores the result into `D+10h` even when null, then checks for failure. A nonnull result leads to actual VirtualAlloc (`CE2080`) for address zero, size 100000h, type 2000h (reserve), protection 4. Its result is likewise stored into `D+0Ch` before the null check.

Reservation failure calls actual HeapFree (`CE20FC`) with current heap, flags 0, and the then-current `D+10h`. Its Boolean result is ignored. The helper returns zero without clearing that descriptor word, rolling back array growth, or adjusting the allocation rover. On success, it writes `D+8=FFFFFFFF`, then `D+0=0`, `D+4=0`, increments current descriptor count, reloads `D+10h`, writes `R.current_group=-1`, and returns `D` in that order. Flags 8 request zeroed metadata through the real provider; a host calloc substitute is not this interface.

## New-group helper and partial initialization

`C1212C(D)` captures `D+8` and `R=D+10h`, finds the first set group bit by shifting left until negative, and calculates `G`. It initializes **63** empty sentinels (classes 0..62), writing each sentinel's previous link before its next link. These writes precede any VirtualAlloc call. A zero available-group mask would never terminate the search; the native caller's nonzero mask contract is required.

It computes requested group address `P = [D+0Ch] + (g << 15)` and calls actual VirtualAlloc (`CE2080`) with `P`, 8000h bytes, type 1000h (commit), protection 4. A null result returns -1 with the 63 sentinel pairs still initialized and without clearing the available-group bit or resetting those fields. A nonnull result is only tested; all subsequent pointers derive from the **requested** address `P`.

For a valid region whose address addition does not wrap, eight pages are initialized. Each page `Q` gets `FFFFFFFF` boundary sentinels at `Q+8` and `Q+FFCh`; a free block at `Q+0Ch` gets size FF0h in its header and `Q+FF8h` footer, next link `Q+100Ch`, and previous link `Q-FF4h`. Exact store order is first boundary sentinel, last boundary sentinel, next link, size header, previous link, footer. The physical code explicitly compares unsigned `P <= P+7000h`; this is retained rather than replaced with an unconditional loop for invalid wrapped addresses.

Class 63 is then linked to the page chain in order: its sentinel next becomes `P+0Ch`, the first node previous becomes that sentinel, its sentinel previous becomes `P+700Ch`, and the last node next becomes that sentinel. Transient first/last links written during page initialization are therefore patched afterward.

The helper writes the selected group's low mask to zero, high mask to one, captures and increments byte `R+43h`, rereads its descriptor argument, and publishes the incremented byte. Only if the old byte was zero does it OR bit 0 into current `D+4`. It clears the chosen group bit in current `D+8` last, then returns `g`. It does not write the group allocation count or `R.current_group`; the zeroed/released metadata state and caller's post-call publication are separate contracts.

## Source readiness and remaining ownership

| Boundary | Readiness supported here |
| --- | --- |
| `C12968` allocation body | Complete code, arithmetic, ordered list/bitmap/tag updates and helper-failure paths; source candidate with actual locked native state. |
| `C1207C` / `C1212C` | Both complete; only actual Win32 IAT dependencies. Their partial-failure stores are part of the source contract. |
| Win32 heap / virtual-memory services | Call sites and import identities verified; future source must use real HeapReAlloc/HeapAlloc/HeapFree/VirtualAlloc with the observed arguments. No host CRT allocator adapter. |
| Canonical heap, descriptor array, counters, rover and retained-empty words | Borrowed actual state required. Original bootstrap, shutdown and global ownership remain external and were not claimed. |
| Lock 4 | Retained BF9E56 owner acquires it before arming its scope and calling C12968. These three bodies do not acquire it themselves. |
| Matching `C11D68` free | Owned by the disjoint free worker; not reconstructed or source-completed here. Integration must use its actual shared state/domain. |
| `BF9F1A` malloc / `BF681B` operator-new | Retained native wrappers and retry/fallback/error policies; this packet does not implement them or the broader PTD/lock/throw owners. |

The smallest complete next source candidate is all three functions together, 1,177 native bytes, for example `src/native_crt_sbh_allocation.cpp` and `include/bsp/native_crt_sbh_allocation.hpp`. A stable borrowed context would identify the original heap/count/capacity/array/rover/retained-empty words; raw 32-bit views would preserve native records and actual Win32 calls. It would require coherent native SBH state and a supported requested class on entry, sufficient writable mapped extents, the established external locking contract, and normal provider behavior including allocation failures. It must retain observed partial failure state rather than promise that every failed operation restores entry invariants.

This is source readiness for bounded bodies, not full native heap ownership. Original startup and teardown, all calling paths, native FH3/SEH execution, asynchronous faults, game integration and runtime behavior remain unclaimed. No source, tests, builds, probes, game execution, Ghidra rename/repair/settings/save, shared-global mutation, merge or push occurred.
