# EH vector-copy iterator readiness

`source_ready=false`; Source authorization is false and Source credit is **0**.
This is a bounded static readiness review of the existing library function
`eh_vector_copy_constructor_iterator` at `00BF9A96`, its frame-based cleanup
`LIBCRT_unmatched_00bf9ae7`, and the 28-byte window at `00E02DD0`. The library
names are retained. No Source implementation or provider/caller expansion is
authorized by this report.

## Exact evidence boundary

| Object | Inclusive selected range | Size / instructions |
| --- | --- | --- |
| Iterator | `00BF9A96..00BF9AE6` | 81 bytes / 23 |
| Cleanup | `00BF9AE7..00BF9AFE` | 24 bytes / 8 |
| Scope window | `00E02DD0` through `00E02DEB` | 28 bytes / data |

The installed Original PE SHA-256 is
`b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6`.
The 105 code bytes (PE `.text`, RVA `007F9A96`, raw offset 8362646) and
28 scope bytes (`.rdata`, RVA `00A02DD0`, raw offset 10497488) match the old
worker's recorded live bytes exactly. Independent Capstone 5.0.7 x86-32
decoding agrees with all 31 listing starts, instruction lengths and mnemonics
(including the equivalent JNZ/JNE spelling), and all captured typed call,
jump, conditional, computed, terminal, target and fallthrough fields. No
flow/fallthrough override occurs at these captured starts.

Inherited live captures belong to old worker
`01a142f1-e60a-7802-b542-86516327b708`; their 22 actual tool pairs / 44 records
were authenticated against that worker's original session prefix, not relabeled
as this worker's queries. The 338-file frozen handoff and the earlier
142+18+179 sealed files retain exact bytes, hashes, file mtimes, read-only
regular single-link identity and recorded directory mtimes.

The metadata, body and flow/scope batches captured modification counter 6343;
the later dependency metadata batch captured 6345. Each batch records equal
before/after counters. This does **not** establish global or cross-batch Ghidra
immutability. Both recorded body-range observations identify the same exact
iterator/cleanup ranges. The scope query's genuine `No instruction at address`
result is retained; its typed code unit is undefined data of length one.

## Ordered behavior and physical ABI

Let E be ESP on entry to the iterator and F its parent EBP after the qualified
SEH4 prolog. The iterator pushes local size `10h`, then scope address
`00E02DD0`. The prolog transfer is `00BF9A9D CALL 00C07C00`.
Instantiating the existing qualified frame contract gives F=E-4 and saved
ESP P=F-30h. Saved EBX/ESI/EDI are at F-24h/F-28h/F-2Ch, the cookie at
F-30h, registration at F-10h, encoded scope at F-8, and try level at F-4.
The actual cookie, handler and valid frame ownership remain external obligations.

The iterator sets completion flag `[F-20h]=0`, try level `[F-4]=0`, and
completed counter `[F-1Ch]=0`. At `00BF9AB0` it compares the counter with
`[F+14h]`; signed JGE exits the loop. A nonpositive initial count therefore
reaches normal completion without invoking a callback, under the valid-frame
contract.

Each iteration pushes `[F+0Ch]`, places `[F+8]` in ECX, then calls through
`[F+18h]` at `00BF9ABB`. Source, destination, stride, count, constructor and
destructor are interpretations of the six physical parameter homes, supported
by existing library metadata; actual callback providers are unreviewed. The
observed constructor call shape requires a provider accepting destination in
ECX and source on the stack, discharging the pushed source DWORD and preserving
the parent frame. There is no caller ADD ESP. Its return value is ignored.

Only after a normal callback return does the iterator read stride `[F+10h]`,
add it to both physical pointer homes `[F+8]` and `[F+0Ch]`, then increment
the counter. These raw 32-bit additions also occur after the last successful
construction. The count, callback and homes are reread on subsequent iterations.
Positive stride, pointer validity, nonaliasing and freedom from callback mutation
of this frame are unproved. Replacing these homes with stable C++ value
parameters would silently add a contract.

At normal completion it sets flag=1 and try level=-2. The ordered direct
transfers are `00BF9ADA CALL 00BF9AE7`, then `00BF9ADF CALL 00C07C45`.
Cleanup observes flag!=0 and returns directly. The qualified epilog requires
ESP=P before its call, restores the parent frame, and returns at ESP=F+4.
The iterator's `RET 18h` at `00BF9AE4` cleans **six DWORD arguments**; a
cdecl caller-cleanup assumption is incompatible with these bytes.

## Cleanup boundary and failure trace

Cleanup borrows F; its ordinary zero-parameter prototype does not express its
physical frame dependency. If `[F-20h]==0`, it pushes destructor `[F+1Ch]`,
completed count `[F-1Ch]`, stride `[F+10h]`, and current destination `[F+8]`,
then enters ArrayUnwind via `00BF9AF9 CALL 00BF7C10`. There is no caller
stack adjustment before cleanup's plain RET, so a returning unwind provider
must discharge those four words and preserve the borrowed frame.

If construction raises before its normal return, the iterator itself has not
advanced its homes or counter for that attempt and its flag remains zero.
Conditional on actual EH dispatch entering this cleanup with the valid parent
frame, it forwards the current homes and count of completed callbacks to
`__ArrayUnwind`. Callback frame mutation, exact dispatch/stack restoration,
destruction order, destructor physical ABI, and a second exception or
termination policy remain unresolved. No exception path was executed here.

The scope window DWORDs are
`FFFFFFFE, 00000000, FFFFFFD0, 00000000, FFFFFFFE, 00000000, 00BF9AE7`.
An EH4 interpretation is consistent with a cookie offset -30h and cleanup
entry, but this bounded opaque window does not prove a complete typed scope
table or dispatcher ownership.

## Named but incomplete dependencies and next contracts

| Dependency | Established boundary | Still required for whole-entry Source readiness |
| --- | --- | --- |
| `__SEH_prolog4` / `__SEH_epilog4` | Existing qualified frame formulas only | Actual frame, registration, FS chain and cookie/handler owner composition |
| Scope / handler / local unwind | Exact 28-byte window and cleanup pointer | Dispatcher entry, cookie validation, parent EBP/ESP restoration and exception ownership |
| Constructor callback | ECX destination, one pushed source word, indirect call | Real provider identity, callee cleanup, frame/register preservation, alias/mutation and exception contract |
| `__ArrayUnwind` (`00BF7C10`) | Metadata only: 35+15 bytes, ranges `00BF7C10..00BF7C32` and `00BF7C5F..00BF7C6D` | Complete 50-byte split-body contract, destruction order, stack cleanup and exceptional termination behavior |
| Destructor callback | Opaque address forwarded from `[F+1Ch]` | Real provider identity and register/stack/exception contract through ArrayUnwind |
| Callers `005C81F0`, `005FE010`, `007042B0`, `008E79D0` | Indexed labels only | Actual caller argument/provider/frame ownership; caller bodies remain unreviewed |

The 94-byte convex span printed for ArrayUnwind is **not** its 50-byte body.
No provider body or caller body was imported into this readiness packet.

A prospective interface must retain the six physical homes in observed order
(destination, source, raw DWORD stride, signed count, constructor entry,
destructor entry), outer callee cleanup of 24 bytes, mutable frame semantics,
and the actual SEH/callback providers. This is an interface constraint, not an
implemented C++ API. A normal-only callback projection cannot receive whole
Source credit while unwind and provider contracts are open.

Evidence is under `local/NativeEhVectorCopyReadinessFinish01`; inherited evidence
remains in the old worktree's `local/NativeEhVectorCopyReadiness01`.
The companion JSON records exact direct call sites for the repository checker.
Static verification and call-site checks grant **0** Source, build, fixture,
physical-ABI, caller, Native-runtime, original-game or gameplay credit. No
compiler, test, probe, target-code execution or Ghidra mutation was performed.
