# Dyn SAP pair cleanup and endpoint recycling

`src/cc16_dyn_sap_retire.cpp` recovers complete `00C4C550..00C4C5C6`
(119 bytes) and RTTI slot1 `00C4CC90..00C4CD9C` (269 bytes). Names describe
observed behavior. The existing `Dyn::SAPBroadPhaseManager` RTTI identity
and vtable `00D7A13C` are retained, without inventing a table or class.

The helper receives manager in EBX and endpoint on the stack, RET4. It
preserves EBX and saves EBP, with ESI/EDI saved on its nonempty path. The
wrapper receives manager in ECX and endpoint on the stack, RET4, saving
EBX/ESI. No consumed semantic EAX result is established; these explicit
void C++ interfaces are not native register/stack or fault-state adapters.

The helper iterates the endpoint's live signed count at +34h, reloading
its reference data +30h each iteration. The selected pair's first payload
word determines the other endpoint, falling back to its second payload
word only when the first equals this endpoint. The other endpoint's signed
count is captured for a bounded identity search. A match directly invokes
the recovered `00C40E50` with the actual endpoint, index and same required
`DynLiveCrt`. There is no missing-reference guard on subsequent pair cleanup.

Pair unlinking reads next then previous for the first neighbor store, then
reloads previous before next for the second. This order differs from
`00C4C5D0`. Pair+Ch receives the old manager free head +34h; manager count
+58h decrements, the local index increments, then the pair becomes the new
free head. The helper does not clear its own reference vector or count.
Self/aliased references retain the original live-count consequences.

The wrapper reads endpoint byte+2Ch once. An active endpoint removes its
axis entries in order 0,1,2 through the real `00C40800`, then calls the
helper above. A pending endpoint is searched in manager data/count
+194h/+198h. Search end is captured; subsequent compaction reloads count
and data after every copy. The original not-found/empty-list hazard and
wrapping decrement remain, with no early return or validation repair.

Pool selector byte+1Ch is captured before reference pointer+30h and its
nonnull `00BF6989` free. Mutation during that callback cannot change the
selected pool. Object unlinking loads next+40h then previous+3Ch, writes
the first neighbor, then reloads next before previous for the second.
The old pool free head becomes object+40h; its count decrements, then the
object becomes free head. Nonzero selector uses head+104h/count+190h;
zero uses head+68h/count+F4h. No other field or stale reference is cleared.

The source directly composes the already recovered endpoint removal and
reference erasure bodies with actual raw Win32 records and mandatory CRT
custody. No object, provider, allocator, vtable, default return, exception
translation or rollback is fabricated. Unsigned address/count wrap and
conditional raw read/store epochs are explicit.

An independent worker reviewed both complete native listings and the frozen
source/header hashes, finding no source-to-listing discrepancy. It verified
all seven original call rows. See `CC16_DYN_SAP_RETIRE_REVIEW.md` and
`reports/cc16_dyn_sap_retire_review.json` at review commit `576e6a35`.
This is static evidence, not an original ABI, fixture or game acceptance.

Both bodies are registered in the common `bsp_core` target. The combined
Win32 Release build and both existing suites passed; final object and source
hashes are in `reports/cc16_dyn_sap_retire.json`. Receipt:
`local/cc16_sap_accuracy_off_build.log` (10.69 seconds for both suites).
The source remains unwired pending genuine game/root and provider custody.

The final promotion tree also merges accepted `c086d1548`. Its fresh
Win32 build and both suites passed (13.68 seconds); receipt:
`local/cc16_sap_accuracy_off_main_build.log`. The three SAP and accuracy
core object hashes remain identical to the inspected objects. The fresh
OFF ship-AI symbol dump still contains zero conversion-getter references.
These checks add no native-ABI or game-activation acceptance.
