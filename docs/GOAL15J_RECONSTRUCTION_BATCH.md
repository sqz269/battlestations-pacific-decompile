# GOAL15J complete raw avoidance clipping

This batch integrates the whole `0041A540..0041AC53` clipping algorithm from worker commit `80309742ed6e03ac961655ef05a58df98b96b6e1`. The new raw Source entry is `clip_native_avoid_zone_world_bounds_0041a540`; the earlier typed implementation and historical records remain intact.

All four planes, rejection and empty-result paths, three inlined scratch-growth sites, record creation, pointer-array growth, partial-state behavior and the two current-cell cleanup operations are covered. No callback, defensive empty guard, pending-record cleanup or rollback replaces an Original operation. The entry borrows its actual incoming bounds-pointer argument cells.

Fresh target-verified live bytes match the worker capture and installed PE for all 1,812 bytes. Complete emitted mapping covers 535 Native instructions, 54 branches, 120 algorithm-cell rebases, two phase stores, 27 virtual-EBP lowerings, 16 real provider calls and five exact constant bindings. All 210 Native FP/SSE rows are accounted for: 209 literal/rebased/bound operations and the separately lowered previous-point MOVSS. Fifteen unreachable padding bytes are retained.

This mapping deliberately replaces the Native frame/EH machinery with persistent Source locals and two current C++ guards. The 2,096-byte private kernel and 17-byte public thunk are not a whole-function byte match. Current emitted EH has four states, including noexcept termination states, versus the Native two-state unwind map. Native FH3/SEH, local addresses, private CRT, binary replacement ABI, factory binding and gameplay identity remain unproved.

Two independent reviews checked the arithmetic/branch lowering and the persistent-cell/guard lifetime. The primary reviewed the interfaces, wrapper, generation/mapping logic and fixture scope, refreshed the live byte comparison, and corrected the Original Ghidra prototype with old values preserved. Source-only EDX remains excluded from the Original prototype.

The focused fixture uses actual current heap and cleanup providers for rejection, all four planes with bounds aliased into a record freed during the call, scratch and 3-to-8 pointer-array growth, rebuilt defaults, empty output, and a real Win32 allocation failure through phase0. Phase1/mid-rebuild failures and double exceptions were not forced. Registered build, existing CTests, final archive-only execution, artifact hashes and publication checks are recorded in [the receipt](../reports/goal15j_reconstruction_batch.json).

The registered Win32 build, all three existing CTests and the archive-only clip fixture passed. A separately recorded correction fixes the existing typed `0049F260` host projection to forward its captured current inner receiver, matching its complete 13-byte Native body. This adds no raw entry or concrete traffic service; see [the receiver repair](GOAL15J_TRAFFIC_LAYER_RECEIVER_REPAIR.md).

The whole constructor `0041CCD0` and group-add `0041D1E0` are separate, explicitly dependent work packets. Their Source implementations and production world-pointer/bounds producer wiring are not counted here. See [the clipping evidence](GOAL15I_AVOID_ZONE_CLIP.md) and [parent readiness](GOAL15I_AVOID_CONSTRUCTION_READINESS.md).

The upstream audit found a genuine persistent canonical game-pointer cell and a raw game lifecycle that can share it. Production composition and a complete raw bounds writer remain open. The entire 1,666-byte writer and 756-byte layout were audited, identifying exact node/count/span helper gaps plus the traffic and mutable authored-data bindings. A new bounded worker packet advances those three helpers; the existing typed bounds projection is not counted as that whole writer.
