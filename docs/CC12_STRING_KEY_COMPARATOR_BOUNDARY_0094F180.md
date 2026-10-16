# String-key comparator boundary repair: blocked and rolled back

The reviewed `0094F180` boundary repair was not completed. The callback remains
without a Ghidra function after a guarded rollback. Source credit remains zero.

The candidate is exactly `[0094F180,0094F1AB)`, 43 bytes and 14 instructions,
with proposed name `BSP_StringKeyList_CompareInsensitive` and two stack slot
parameters returning signed order in EAX. The existing tail instruction at
`0094F1A6` retains its `CALL_RETURN` override to `00438E10`.

## Mechanism failure and rollback

The supported `run_script_inline` endpoint rejected a read-only snapshot because
Java script execution is disabled. The ordinary function-documentation endpoint
also rejected the unowned address, so the required complete symbol/comment
preimage was unavailable.

While checking the purpose-built fallback, the direct HTTP `create_function`
endpoint ignored the literal `dry_run: true` field. With
`disassemble_first: false`, it created the requested named 43-byte function.
This operation occurred under `coordination.ghidra_lock`. The request, response,
and typed readback are retained; this is not claimed to describe MCP client-side
dry-run wrappers.

The worker then removed only that newly created function under the write lock,
guarded by its exact entry, name, single body range, and 43-byte size. No
prototype, comment, save, export, script enablement, or restart was requested.
Function counts were `76774 -> 76775 -> 76774`.

## Established preservation and limits

All 14 callback instructions are again unowned. Typed listing, flow, ownership,
and function metadata for all 17 captured addresses match the preflight,
including the padding start, next function, tail target, and existing
`CALL_RETURN`. Callback bytes before and after match the current disk and the
prior sealed Original: SHA-256
`6642a84136c7429cb52b322dd89022b5951cb83b6f34059b5f0a0aaa60bae50e`.
The current disk comparison occurred after rollback; a fresh disk-read gate
immediately before the unexpected write is not claimed.

The intermediate created-function documentation contained no comments and the
two labels `LAB_0094F192` and `LAB_0094F1A4`. It cannot prove preservation of
the unavailable complete pre-create symbol/comment state. A pristine
whole-database rollback is not claimed. The old 60-file readiness family remains
unchanged.

The actual list ownership/binding, canonical empty-data role, private CRT locale
comparison, and qsort callback contracts remain blockers. No Source, CMake,
build, test, probe, or target execution work occurred. Shared snapshots, indexes,
ledgers, and packet registry were not edited by this worker.

See `reports/cc12_string_key_comparator_boundary_0094f180.json` for exact
requests, responses, counts, proof paths, and remaining uncertainties. Local
evidence is under `local/SceneStringKeyComparatorBoundary01/`. The primary
integrator must review the blocked mechanism before any further repair attempt.
