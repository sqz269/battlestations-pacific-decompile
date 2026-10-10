# Grid providers: primary readiness acceptance

Root accepts worker `364be1a35`. A separate implementation may change only
`NativeGameGridCpuCalls::allocate_00bf55be` to canonical singleton allocation
and `free_00bf65ac` to canonical singleton free. `BF6989` remains std::free.
This closes the two reviewed default Source allocation/free families without
assuming operator-new/free interchangeability.

Sizes, including zero, must pass unchanged to the canonical malloc/new-handler
policy. Preserve current virtual overrides, six independent allocations, current
field reads, cleanup-before-zeroing, scalar flags and retained failure state.
Foreign storage still needs matching overrides at every reached service.
No new noexcept, validation, rollback, retry or allocation adapter is admitted.

Root copied and hash-checked all 79 finite pins, replayed the guarded audit,
and verified 4,250 actual Git blob occurrences. The proof retains 4,208 Source
files, eight read-only test search inputs, 24 accepted dependency blobs, 18
complete queries, 20 excerpts and five distinct historical build epochs.
All current Root Source inputs match the freeze. No new build or test is
credited to this readiness packet.

The callable raw-grid table remains unready. Numeric CFD484, the CPU service's
vptr and an ordinary three-argument scalar helper do not supply the required
slot-zero thiscall method. Genuine table/context ownership, installation and
destruction schedule, partial failures and retirement must be established
separately. This packet admits neither game construction nor activation.

Evidence: `reports/cc12_grid_provider_readiness_primary.json` and
`local/cc12_grid_provider_readiness_primary/Root_game_review.json`.
