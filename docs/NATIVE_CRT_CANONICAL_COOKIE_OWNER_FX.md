# Canonical cookie initializer in the rebuilt process owner

`GameNativeCanonicalDataOwner` now calls the complete no-argument native
`00C1815E` initializer supplied by FU. The former call constructed an additional
two-reference context for the qualified BM interface. That interface remains
available; the process owner now uses the original entry contract and actual
`00E15590` cookie and `00E15594` complement directly.

The source change replaces one include and that one call block in
`src/game_native_mutable_crt_data.cpp`. FZ independently reviewed the exact
replacement against the original entry, FU's complete 148-byte body, and the
existing owner/bootstrap contract. FU retains the real five KERNEL32 imports,
original frame/register/flag schedule, ignored QPC result and uninitialized QPC
output slot, original adjustments including a possible zero, and ordered cookie
then complement publication. See `NATIVE_CRT_CANONICAL_COOKIE_INITIALIZATION_FU.md`.

The call occurs after both disjoint owners transfer their reservations, initialize
the verified original read-only and writable pages, and pass `verify_joint()`.
The existing complement/default predicate and final page check follow the call.
The caller then completes the parent readiness acknowledgment before releasing
the permanent owner and publishing its pointer. Construction or acknowledgment
failure retains the existing rollback and process-claim release. Successful
ownership remains valid through process teardown. No second initialization,
new seed policy, synchronization, validation predicate or lifetime change is added.

Original `entry` at `00BFD2BD` calls `00C1815E` without arguments and jumps to
`00BFD0DD`. The rebuilt C++ owner is an integration boundary with its own API;
its placement does not reconstruct that entire original startup continuation.
The host compiler's cookie remains a separate domain. Native SEH fault recovery
is not established by the C++ rollback policy.

The combined candidate requires a full diagnostic Win32 build and existing CTests,
the actual linked native body and five resolved imports, the constructor's real
CALL target without the removed context argument, current compiler/object/archive
ownership, the actual `game_main` owner path, and exclusive rebuilt startup/drain
validation. Exact-commit receipts are retained under `local/fu_fv_integration`;
this source document does not substitute for those receipts. The math CTests do
not execute the initializer. No new test case is added.

FV's canonical small-block-heap entry remains a separate provider: its actual
heap must first be created by the original startup policy. Page admission alone
does not make it callable. Full CRT/PTD/SEH readiness, original instruction and
fault-address identity, and gameplay validation remain separate requirements.

The source and ordering evidence are recorded in
`reports/native_crt_canonical_cookie_owner_fx.json`.
