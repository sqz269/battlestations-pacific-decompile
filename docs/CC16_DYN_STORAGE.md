# Dyn pair-reference storage at 00C37390 and 00C40A30

Packet `codex-dyn-storage` recovers two in-house bodies in
`src/cc16_dyn_storage.cpp`, declared in `include/bsp/cc16_dyn_storage.hpp`.
Names are descriptive hypotheses. The interfaces take actual raw native
object bases and a required caller-supplied `DynLiveCrt`; no object, class
identity, geometry, global allocator or default service is fabricated.
The worktree imported accepted main `90d44021a` before implementation.

## Evidence and original ABI

The configured saved project is `C:/Users/sqz269/bsp.gpr`. Before live
batches, the provided client verified project `bsp`, program
`/battlestationspacific.exe`, language `x86:LE:32:default` and image base
`00400000`. Recovery used both complete saved assembly bodies, live
prototypes/callees, full byte spans and bounded caller instruction contexts.
No Ghidra, ledger or build-registration mutation was made.

| Body | Inclusive span | Bytes | Original interface |
|---|---|---:|---|
| reference append | `00C37390..00C373FC` | 109 | ESI endpoint, one stack pair pointer, `RET 4` |
| pair allocation/link | `00C40A30..00C40B2D` | 254 | EDI storage, stack first/second endpoint pointers, `RET 8` |

The leaf preserves ESI and saves/restores EDI only on its growth arm. Its
last computed EAX is the insertion-slot address, including zero when the
slot write was skipped. The parent saves/restores EBX, EBP and ESI, leaves
EDI unchanged, and returns the second append's EAX. The C++ interfaces
preserve these observed values as `void**`; a semantic native return
contract is not established. Raw caller `00C4C680` forwards EAX on return,
while the three inspected defined call sites do not immediately consume it.
These public C++ calls are not original register/stack ABI adapters.

## Recovered storage and ordered operations

The leaf uses endpoint `+30h/+34h/+38h` as data/size/capacity for four-byte
pair references. Growth occurs only on equality, with wrapping capacity
`old * 2 + 2` and byte request `capacity * 4`. Capacity is assigned before
new; old entries copy with unsigned live-count comparisons and per-slot
null checks, then the nonnull old buffer is freed and the new pointer
published. The append slot is null-checked; size increments even if skipped.

The parent uses storage `+28h/+2Ch/+30h` for a page-pointer vector and
`+34h` for the free head. On an empty free list, malloc receives `3E80h`
(1,000 records of 16 bytes). The first 999 records' `+0Ch` words point to
the next record; the last is zero. After reading the page-vector capacity
and equality condition, the free head is published before any page-vector
growth allocation. Page-vector growth/append has the same unsigned,
wrapping, per-slot behavior, with capacity stored before new. The free
head is reloaded after those callbacks before a record is popped.

The live count at `+58h` increments, then the free head advances. The node
is linked before the sentinel at storage `+48h`: node next gets sentinel,
node previous gets storage `+50h`, that tail is reloaded before writing
its next, and storage `+50h` becomes node. Payload words `+0/+4` contain
endpoint pointers sorted by their unsigned Win32 addresses. In the
greater/equal arm, the second payload word is written first, as in the
listing. The append leaf is composed twice in original argument order:
first endpoint, then second endpoint, regardless of sorting. Equal
endpoints are not rejected and receive two appends.

Every raw load/store uses the proven four-byte offsets with volatile
Win32 words and integer address arithmetic, retaining callback-sensitive
reloads and wrapping address products. No portable C++ class identity or
unrelated-pointer ordering is assumed. All allocation/free calls use the
same actual CRT reference. Only `operator_new_00bf55be`, `free_00bf6989`
and `malloc_00bf9f1a` are required; their bodies are not reconstructed here.

## Failure, callers and activation limits

Neither body has an SEH/FH frame or rollback. There is no added null,
overflow, size/capacity, duplicate, sentinel or pool-exhaustion guard.
Malloc failure proceeds to the original invalid-address writes. A later
new exception can leave a newly malloced page, changed capacity/free head,
linked node, incremented live count or first endpoint append visible.
The source adds no catch, cleanup, transaction or failure result.

Live xrefs establish defined parent sites `00C4C938`, `00C4CA4A` (caller
`00C4C700`) and `00C552FB` (caller `00C54DA0`). Disk disassembly also
confirms `00C4C6EC` in currently Ghidra-undefined `00C4C680..00C4C6F5`.
That body is only bounded caller evidence, not reconstructed or defined
by this packet. These references do not establish a genuine activated
host path; the primary owns caller wiring and reachability review.

Validation includes complete instruction review and an optimized MSVC x86
C++17 object compile with `/permissive- /EHsc /fp:strict /O2 /W4 /WX`.
The live call-report verifier checked all nine rows with zero failures.
This packet adds no tests, game execution, fixture claim, binary ABI
compatibility or activation acceptance. Primary integration owns central
metadata, annotations, common build and genuine host validation.

## Primary integration

The primary reviewed both complete source/assembly bodies, verified all nine
native call rows, registered the module directly in `bsp_core` and built the
combined tree over accepted `893cf58e6` plus worker `c6d625c1`. The Win32
Release build and both existing suites passed (11.03 seconds); receipt:
`local/cc16_dyn_storage_build.log`. Both metadata-only placeholders now name
the compiled, unwired bodies. Locked Ghidra naming/comments were saved, both
exports refreshed and prior annotations preserved in
`local/ghidra-annotations-20261029T043641Z.json`. No original ABI, fixture,
root reachability or game-activation acceptance follows from this batch.
