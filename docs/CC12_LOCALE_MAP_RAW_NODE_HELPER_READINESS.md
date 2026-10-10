# Locale raw node payload helper: Source readiness

A small dormant helper can compose the existing raw wide and narrow string
destructors for the **ordinary C++ callback domain with valid readable headers**.
It needs an armed narrow-header unwind guard around the wide call, followed by
disarming and the normal narrow call. This preserves the covered provider
sequence; it does not reproduce the native fault boundaries or EH frame ABI.
This packet is a readiness review, not C++ implementation or activation approval.

## Accepted inputs and exact native schedule

Root accepted the complete `00737CE0` node body in `f1bc40d1d` and its selected
narrow unwind routing in `ee48bf2ec`. Those whole committed reports and their
worker reports are retained. The existing 119-byte node body and 62 bytes of
selector/FuncInfo/map/action evidence are reused, with their accepted pins;
there were no new Native queries, listing/flow/prototype changes or Ghidra edits.

The native sequence remains:

1. Retain the original node receiver. Load its wide pointer at `node+0Ch`
   (`00737CFD`) **before** state 0 is armed (`00737D02`). Null skips the length
   read and providers. Otherwise capture current `node+08h`, compute wrapping
   `2*length+2`, then call `00419CC0` and `00BD1510` with the captured packet.
2. After the wide stage, reload the current narrow pointer at `node+04h`
   (`00737D23`) **before** state -1 (`00737D28`). Null skips the length read and
   providers. Otherwise capture current `node+00h`, compute wrapping `length+1`,
   then perform a separate getter and return.
3. Under the accepted correspondence premise `EBP=S` (node entry ESP), action
   `C86260` loads the saved original receiver and tail-jumps to `0041DD20`.
   Cleanup therefore reads that original node's **current narrow header**. The
   external runtime rebase itself remains unproved; the action does not select
   a captured old narrow buffer or the wide header.

Neither normal helper composition nor this selected cleanup adds wide cleanup
repetition, node-slot recycling, map clearing, header zeroing, publication repair,
receiver freeing, alias deduplication, or overflow validation. The node-slot pool
`E17698/E176B4/E176C0` remains distinct from the string payload pool.

## Smallest normalized candidate

A provisional normalized signature is
`void destroy_native_locale_map_node_payload_00737ce0(void*, NativeStringRawPoolContext&)`,
without `noexcept`. The name is descriptive, not a recovered symbol. Its minimal
shape is: retain the original node and borrowed context; arm a local cleanup
object containing those references; call raw wide cleanup on `node+8`; disarm;
call raw narrow cleanup on the original `node+0`. Guard construction and disarming
must be nonthrowing and invoke no service. The guard runs raw narrow cleanup only
while armed; no new pool, domain, storage bridge, callback family or public guard
framework is needed.

There is an existing Source pattern: `NameUnwind` in
`src/native_loading_queue_submission.cpp:25` holds the original header,
`NativeStringRawPoolContext&`, and `armed=true`; its `noexcept(false)` destructor
calls raw `0041DD20` when armed. It is local to that translation unit, so the
candidate can reuse the pattern, not call it as an existing shared API. That file
is tracked and included in `startup.cmake` and the generated Core project, but
falls outside the selected Source3352 manifest. Its complete 5,858-byte current
file and exact Git blob are separately retained; no guard-specific compiled or
runtime proof is borrowed.

The minimal composition deliberately has a narrower claim than exact native
state timing. Its wide helper reads the wide pointer **after** the guard is
armed; its normal narrow helper reads the narrow pointer **after** disarming.
The existing header helpers own those reads. With valid memory, no concurrent
mutation outside covered provider calls, and a nonthrowing guard transition,
there is no callback or C++ failure between the displaced boundary and its read.
Each getter still sees the required armed/disarmed state, and each helper still
captures its pointer and size before its own getter. Thus all covered callback
boundaries are preserved. Faulting reads are a separate domain; the current Core
project uses `ExceptionHandling=Sync`.

Do not add speculative duplicate reads, copied temporary headers, or early narrow
snapshots to make the source look closer to the listing. Such changes would add
observations or lose mutation behavior. Literal preservation of the raw-read
fault boundaries would require a separately reviewed captured-value interface or
native exception mechanism; it is not established by this candidate.

## Failure and mutation constraints

| Situation | Covered Source behavior / limit |
| --- | --- |
| Wide getter returns and mutates header fields/publications | Return the captured wide buffer/size through its returned owner; then read current narrow fields and perform a new getter. Do not cache the pool across stages. |
| Wide getter throws a C++ exception | Its existing internal cleanup completes first; the armed node guard invokes raw narrow cleanup on the original node's current header. If that cleanup returns, propagate the original exception. Preserve prior provider effects. |
| Normal narrow getter throws | The node guard is already disarmed. Propagate once; do not retry either stage or run a second narrow cleanup. |
| Narrow getter throws during wide unwind | A throwing `noexcept(false)` guard destructor during C++ unwinding terminates. This is the C++ rule, not proof of the native double-exception policy. The successful-unwind correspondence requires cleanup to return. |
| C++ exception attempts to escape `00BD1510` | The existing Source return provider is `noexcept`; outer cleanup cannot be promised. This inherited provider limitation is outside the covered propagating-getter domain. |
| Header read/access violation, asynchronous exception, or invalid receiver lifetime | No ordinary C++/Sync equivalence is claimed. Native pre-state wide-pointer and pre-disarm narrow-pointer fault timing is retained as a separate boundary. |
| Aliased or repeated payloads | Preserve both independent conditional returns and all mutations. Do not deduplicate, clear stale headers, or claim double-free/repeat safety. |

A `catch (...)` that runs narrow cleanup inside the handler is not an equivalent
substitute for the unwind guard: a second exception there can replace the first,
whereas a destructor throwing during active C++ unwind terminates. Neither
swallowing cleanup failure nor inventing an SEH translator is justified here.

## Source service identity and retained lifetime

Both stages and unwind must borrow the same supplied context object. Its fields
refer to the canonical Source cells: `01090AA8` pool publication, `01090AA4` live
small-return gate, and `01090AA0` raw-manager publication. `GameNativeStringProcess`
retains those cells; `GameSingletonHost` borrows its manager cell, and
`GameNativeVfsApplication`/`GameVfsHost::raw_strings()` expose a context referencing
the same cells. These context objects need not themselves be the same object as
the process's `raw_context()`; their publication-cell identity is what is shared.

The caller must keep the supplied context object and original node readable
through normal return or guard cleanup. Process-retained cells alone do not
extend a VFS context object's lifetime. Existing raw manager/deletion bindings
and payload ownership must remain valid; no new singleton or semantic
`LocaleTables`/`NativeStringStorage` owner may be substituted. Provider mutations
of the publications and later narrow header are intentionally observed.

## Qualification and portable evidence

The initial capture at worker `2a52ce957` retains all 3,352 selected current
Source inputs, each exact to its actual committed Git blob and the prior raw-wide
post-build capture. Against the old typed-view Root epoch, 3,343 are byte-exact and
3,350 LF-equivalent: exactly the wide header and implementation are logical
changes. These are selected epoch inputs, not a claim that the manifest contains
every source in the generated project; the guard dependency above is separate.

Later acceptance is separate: Root `b7727f128` accepted the wide helper, ran its
three checks and froze `Source3352-native-wide-raw-context`. All 3,352 worker inputs
are LF-equivalent to that new frozen Source cohort; 3,345 are byte-exact. Whole
committed acceptance files, the new epoch manifest and Root's build result/log
are retained without rewriting earlier pending/historical statements. This
readiness packet ran no build or behavioral test. Epoch artifact pins remain
historical metadata rather than a new complete-artifact claim.

Root then corrected the wide report's documentary native length from 31 to 30
bytes in `e83f27a85`, using its existing exclusive endpoint `0043644E`. The
earlier `b7727f128` report is preserved alongside the whole correction and its
supporting documents. The corrected report's Source, 57-byte compiled helper,
artifact and build fields are unchanged. Root's later annotation save
`f0fec19d8` is a parent notification only here; earlier captured Native names and
metadata are not rewritten or freshly queried.

`local/cc12_locale_map_raw_node_helper_readiness/evidence.zip` contains 3,423
pinned files, 14,233,574 bytes, SHA-256
`71d149fe939e43485ab4549487284cadf8e629e8fa6393d83bde8e682cf250fb`.
A physical copy passed the same ZIP-only review under a read-only audit guard
that rejects external files, processes, networking, dynamic loading and directory
access. The review checks pins, actual captured Git blobs, selected Source
cohorts, whole accepted reports, prior bytes and complete prior raw-helper COFF
sections; it does not prove the proposed helper's behavior or native ABI.

The candidate is ready for a separate bounded Source implementation decision
under the stated valid-memory and exception contract. There is no node/map/full
locale or application activation approval, no fresh Native or game evidence, and
no new ledger record. See the [machine-readable report](../reports/cc12_locale_map_raw_node_helper_readiness.json).
