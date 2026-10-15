# Reconstruction goal: first batch, 2026-10-15

The goal began from the live repository directions and `bsp.py brief` on
`main`. There were no ready queued packets. Existing leases and unrelated
untracked files were preserved. Three unclaimed addresses received bounded
ad hoc packets, separate worktrees and explicit file/address leases.

| Address | Deliverable | Coverage | Source review |
| --- | --- | --- | --- |
| `0041DED0` | Registry terrain-layer owning-list cleanup | Complete source schedule with required external services | Independent assembly review; worker x86 compile; 11 report call rows and two markdown call claims pass |
| `0041F820` | Existing-byte record buffer and pool cleanup | Complete source composition through existing raw pool services | Independent assembly/provider review; worker x86 compile; three report call rows pass |
| `00423640` | Manager-driven zone generation orchestration | Complete bounded source orchestration with required direct bindings | Strict x86 compile; emitted arithmetic inspected; 21 direct report calls pass; independent non-arithmetic review |

All three sources passed independent review. The third packet resolves the
manager receiver from its first callee, preserves the actual x87 arithmetic
and mode-dependent integer conversion, and uses the recovered `0.5` constant
and `all` name literal. There is no installed production adapter for its
required service bindings.

The shared avoid-zone host comments no longer require the `00BF6713`
checked-iterator handler to be non-returning. Its wrapper has a plain return;
handler return/failure policy is external, and a returning handler must leave
valid state for its caller's continuation. No existing implementation behavior
was changed by that comment correction.

The new interfaces are source reconstructions, not binary replacements.
External services, allocation domains and unknown owner identity remain
explicit. Build, original ABI compatibility and gameplay evidence are separate
claims. The registered MSVC Win32 build passed, and all three existing CTest
suites passed: reconstructed math, native math differential and tool tests.
The differential suite covers its existing seed routines, not these three new
entries. All 35 direct report call rows passed. No new tests were added.

The three source files compiled into `bsp_core`. Ghidra names and evidence
comments were applied under the write lock, prior annotations were recorded,
the project was saved, and all three standard exports were refreshed. See
[the machine-readable receipt](../reports/goal15_reconstruction_batch.json)
for log paths, source commits and remaining limits.

The integration helper stopped at seven pre-existing duplicate header names.
All six implicated headers were byte-identical to current `main`; none of the
new headers introduces a reported duplicate. The required build and CTests
were therefore run directly before normal git integration. Existing library
link warning LNK4006 for `spawn_request_id_matches` also remains outside these
packets; it did not fail either build.
