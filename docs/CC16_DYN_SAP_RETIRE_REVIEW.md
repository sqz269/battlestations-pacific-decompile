# Independent static review of Dyn SAP removal

No source-to-listing discrepancy was identified in the bounded review of
`retire_endpoint_pairs_00c4c550` and `remove_and_recycle_endpoint_00c4cc90`.
This result applies to the frozen source/header hashes below and the two
complete native bodies. It is not common-build, ABI, fixture or runtime approval.

Reviewed frozen files in
`J:/PROG/battlestations-pacific-decompile-codex-no-run-integrator`:

| File | SHA-256 |
| --- | --- |
| `src/cc16_dyn_sap_retire.cpp` | `824effa7ab9cb6281efcaed1c2c062333aed0c1fedebe0b9dcf85eec7c4ad13b` |
| `include/bsp/cc16_dyn_sap_retire.hpp` | `c40c212d0cda946a0f2480e8ff5e2e1bdaacf1b37d5f9d37e4c3f8e8a319f569` |

The review freshly exported and read all 49 instructions of
`00C4C550..00C4C5C6` (119 bytes) and all 84 instructions of
`00C4CC90..00C4CD9C` (269 bytes). Every live batch used the verifying
`bsp.py ghidra` client for project `bsp`, `C:/Users/sqz269/bsp.gpr`,
program `/battlestationspacific.exe`, x86 LE 32-bit, base `00400000`.
Complete contiguous live byte spans and listing hashes are in the JSON report.

## Pair retirement comparison

The source uses the live signed owner count at +34h: native initial JLE
at `00C4C55A` and loop JL at `00C4C5BF`. Owner data at +30h reloads each
iteration. In contrast, the other endpoint's count is captured once at
`00C4C573`, tested signed-positive, then retained through the reference
search's JL at `00C4C58C`. Its data pointer is captured once for that search.
The other endpoint selection, reference index, and same genuine CRT reach
the actual recovered `erase_pair_reference_00c40e50` directly.

The pair is unlinked/recycled even if no other reference matches. At
`00C4C59A..00C4C5A9`, next is loaded before previous for the first neighbor
store, then previous is reloaded before next for the second store. The
source preserves that order. Free head +34h is read, stored in pair+0Ch,
live count +58h decrements, the local index increments, then free head is
published. Own references are not cleared by this body, matching native code.

## Object removal comparison

The active-byte check at endpoint+2Ch chooses one branch. Active objects
remove axis endpoints 0,1,2 in that order with the actual object and manager,
then directly retire their pairs. Pending objects capture an initial end,
search to equality, and compact with fresh count/data reads after every copy.
The source retains the original missing-item hazard: a search reaching end
still enters the shift loop unless end equals the computed last element.
No not-found or empty-list guard has been invented.

The pool selector at endpoint+1Ch is captured before the reference-buffer
read at +30h and any actual `free_00bf6989` call (`00C4CD26..00C4CD34` /
`00C4CD6E`). Thus mutation during free does not change the selected branch.
Endpoint unlinking loads next+40h before previous+3Ch, writes the first
neighbor, then reloads next before previous for the second neighbor. This
differs from the pair helper's second reload order and matches the source.
The nonzero selector uses manager head +104h/count +190h; zero uses +68h/+F4h.
Both publish the old free head into endpoint+40h, decrement count, and only
then publish the endpoint as the new free head. No extra field reset is added.

All seven original direct call sites passed `verify_report_calls.py`.
The reviewed code forwards actual raw objects and the caller's CRT to real
recovered dependencies; it supplies no fabricated class, vtable, global,
default provider, returning stub, cleanup or missing-item guard.

## Scope and remaining risks

The not-found compaction hazard and unguarded/missing-reference pair
recycling are preserved native preconditions, not newly introduced defects.
Reviewing those paths does not establish safe behavior for invalid inputs.
Original/public ABI differences and fault-register identity limits are
explicit: native `00C4C550` takes EBX manager plus stack endpoint/RET4;
`00C4CC90` takes ECX manager plus stack endpoint/RET4; public interfaces are
new void C++ functions and do not supply those adapters.

No source/header edits, compilation, tests, native/game launches or Ghidra
mutations were performed by this review. Root retains common registration,
annotation/save/export refresh, build/checks, genuine host/CRT binding, ABI
adapters and runtime/activation decisions. Dependency bodies beyond these
two callers were not independently re-reviewed; vendor CRT bodies receive
no reconstruction credit. Any change to either frozen hash needs a scoped
follow-up review of the changed portion.
