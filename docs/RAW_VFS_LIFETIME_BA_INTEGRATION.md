# Raw stream ownership and MPAK container integration

Physical-stream pool and render-batch pool/lock contexts now accept the same
borrowed `SoundLifetimeAccess`. Their getter paths capture the first manager's
section, recheck publication, construct and publish an owner, resolve the manager
again for registration, release the captured section and reload publication.
The raw singleton deletion dispatcher retains the matching contexts and passes
each popped owner to its existing destructor. Existing semantic-domain callers
remain supported through the lifetime adapter.

`NativeMpakContainerStorage` supplies actual 10h vector headers, 24h file records,
14h directory records and DWORD offset insertion. Parser append paths preserve
allocator words and padding, protect aliases with a temporary before growth,
use the observed 1.5 capacity rule and destroy completed prefixes on source C++
copy failure. It borrows existing string, stream, allocation and offset-copy
services. General helper insertion, original FH3 rethrow/partial-element cleanup
and CRT exception identity remain outside the reconstructed scope.

Primary review checked twelve complete live/disk function envelopes, 37 direct
CALL sites and four table spans. It repaired saved ownership at `BF4398`,
`B1E958`, `B1D55E`, `BB623D`, `BB6E82`, `BB721A` and `BB721D`, preserving prior
names, comments and labels. The last address is the `POP EDI` following the
directory-vector cleanup. Clearing flow overrides alone restored decoded
listings but did not restore containing-function ownership; full readback after
body recreation verified every instruction. Callee no-return flags were retained.
All twelve roots have saved evidence/ABI annotations and refreshed exports;
the existing shared `BSP_StlVectorInt_Insert` name remains unchanged.

This corrects the earlier render-batch report's implication that live bytes
established saved ownership at `B1E958` and `B1D55E`. Both instructions were
unowned before primary repair. It also supersedes the physical packet's claim
that the batch dependency still requires a semantic-only lifetime: both
adaptations are integrated together here.

Combined validation is pinned in `reports/raw_vfs_lifetime_ba_validation.json`.
The raw-manager fixture exercises registered batch-owner drain. Container
fixtures exercise populated source records and copied-native growth/insertion
with explicit lower-level source bridges. VFS/Lua fixtures retain seeded valid
records and a semantic manager while exercising actual HANDLE reads, nested
`DoFile`, stream reference/cursor preservation and complete source shutdown.

These are source and fixture results. The type counter and retained VFS owner
bundle are separate BB packets. Production raw VFS manager/provider construction,
real archive loading, original ABI compatibility and gameplay validation remain
incomplete. The batch lock's existing `TrackedCriticalSection` representation
also retains its documented source ABI boundary.
