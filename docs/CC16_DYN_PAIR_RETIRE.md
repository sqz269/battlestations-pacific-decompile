# Dyn pair-reference removal and retirement

Packet `codex-dyn-pair-retire` recovers `00C40E50` and `00C4C5D0` in
`src/cc16_dyn_pair_retire.cpp`, declared in its matching header. Names are
hypotheses. Interfaces use actual raw Win32 object bases and the caller's
genuine `DynLiveCrt`; no class, object, global, default heap or service is
fabricated. Accepted upstream `893cf58e6` was merged before this packet.

## Original bodies and ABI

The configured project is `C:/Users/sqz269/bsp.gpr`. Before live batches,
the provided client verified project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and image base
`00400000`. Evidence covers both complete saved assembly listings, live
prototypes/callees/xrefs, full byte spans and bounded caller contexts.

| Body | Inclusive span | Bytes | Original inputs/return |
|---|---|---:|---|
| reference erase | `00C40E50..00C40EE2` | 147 | ESI endpoint, stack index, `RET 4` |
| pair retire | `00C4C5D0..00C4C67D` | 174 | EAX first endpoint, stack storage/other endpoint, `RET 8` |

The leaf preserves ESI, saves/restores EDI, and saves/restores EBX only in
the allocation arm. Normally EAX returns the copied last-reference word;
the unusual growth arm leaves its final size/loop value. The source
returns raw uint32 bits to preserve both cases without a semantic return
claim. The parent saves/restores EBX/EBP/ESI/EDI and leaves EAX as the old
next link reloaded after its first neighbor write. The pointer return
preserves that observation. These are new C++ interfaces, not native
register/stack adapters. Neither body has an x87 instruction or SEH frame.

## Exact removal and mutation epochs

The leaf loads the last reference from endpoint data `+30h`, using size
`+34h`, and writes it at the caller's index before reloading size. Desired
size is wrapping `size - 1`. Its comparison is unsigned: the ordinary
positive-size case simply assigns the decremented size, retaining capacity
and the stale old-last slot. Zero/aliased sizes can enter the original
growth path; this is not replaced by an early return or assertion.

Growth uses desired size as capacity and requests wrapping `desired * 4`
bytes. Capacity is assigned before required CRT new. Old references copy
with live unsigned size reloads and computed-address null checks, then
the nonnull old buffer is freed and the new pointer published. The zero
fill also reloads data for each computed slot. Final size is always the
captured desired value. Invalid indices, zero-size last-element reads,
allocation failure and arithmetic wrap retain their native consequences.

The parent captures storage and other endpoint, then caches the first
endpoint's size. Signed-positive/JL loops scan its references for the first
record whose payload `+0` or `+4` equals the other endpoint. A match is
reloaded from the cached original array. A second search, with that same
cached count and a fresh data pointer, removes the reference by identity.
Other endpoint size/data are loaded after the first leaf call, so aliasing
endpoints or callback mutations do not use a stale second count.

The leaf is directly composed at both native call positions under the
same actual CRT custody. The parent then unconditionally unlinks the
record through `+8/+Ch`, with fresh record-link reads after its first
neighbor write. Record next becomes storage's old free head at `+34h`,
the live count at `+58h` decrements with uint32 wrap, and free head becomes
the record. Payload and previous-link words remain untouched. No pool
page or record is freed by this normal retirement operation.

If no pair is found, the captured pair remains zero; searches still occur
as in the listing and the unconditional unlink can fault. No missing-pair
fallback, duplicate cleanup, guard, exception translation or rollback is
introduced. Volatile four-byte raw accesses and uint32 address arithmetic
preserve the explicit reload/store and overflow contracts.

## Caller evidence and validation boundary

Live xrefs identify leaf site `00C4C591` in `00C4C550`, and parent sites
`00C4C83C`/`00C4CB55` in `00C4C700`. Leaf sites `00C4C62E`/`00C4C653`
belong to this recovered parent. Disk bytes separately establish leaf
site `00C40F11` in Ghidra-undefined `00C40EF0..00C40F17`. Earlier bounded
disk evidence establishes parent site `00C4C6E1` in undefined
`00C4C680..00C4C6F5`. Those undefined bodies are neither assigned to a
neighboring function nor reconstructed/defined here.

Validation includes full instruction review and optimized MSVC x86 C++17
object compilation with `/permissive- /EHsc /fp:strict /O2 /W4 /WX`.
The live call-report verifier checked all four rows with zero failures.
No tests, game execution, fixture result, binary ABI compatibility or
activation is claimed. Genuine caller wiring and host validation remain
outstanding. The primary owns registration, shared metadata, annotations
and common build; this packet makes no Ghidra or ledger mutation.

## Primary integration

The primary reviewed both full source/assembly bodies, registered the module
in `bsp_core` and verified all four call rows. The common Win32 Release build
on accepted `74074cc65` plus candidate `9a2ece4c` and both existing suites
passed (11.32 seconds). Receipt: `local/cc16_dyn_pair_retire_build.log`.
Both metadata-only records now name compiled/unwired source. Locked Ghidra
annotations were saved and exports refreshed; prior values remain in
`local/ghidra-annotations-20261029T044659Z.json`. No new fixture, original
ABI, root reachability or game activation is claimed.
