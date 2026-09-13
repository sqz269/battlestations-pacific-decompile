# AW actual renderer recreation integration

Addresses: 00b3d7b0, 00b3e190, 00b3d7c0, 00b3d800, 00b3e1f0, 00b3e230, 00b28a90, 00b26920, 00b33bf0, 00b29670

Ten complete normal bodies across retained texture callbacks, worker stop and
device recreation use the existing actual storage and service domains.
Reviewed source `287b7d158c919f1ce2f75e0eff2e94786a2fed81` was merged with current main;
exact combined source `e30488f4a1059a41f17cc8836b64fa7c901b95a9` passed the strict Win32 build, both existing
tests and four current-library-only original-byte probes.

The integrator saved ten reviewed names/original signatures and full body
readbacks, defined the seven-byte 2D dispatch and two analysis-only EH handlers,
and refreshed exports. All56 numeric report call rows pass; current indirect
profile targets require the separate assembly/producer/fixture evidence.

See `reports/native_renderer_device_recreation_aw_validation.json` for exact
source/library hashes, preserved fixture archives, branch coverage and limits.
No complete Reset, game startup/render submission, gameplay, arbitrary worker
concurrency or binary ABI compatibility is claimed.

Next ready work is documented in local/aw-queue-readiness.md and
local/aw-reset-readiness.md. Queue construction and presentation-mode changes
have independent worker files; canonical scene-head mapping, Reset, startup
and BeginFrame/EndFrame dependencies still need their own evidence.
