# Orchestrator 6 reconstruction batch H

Five complete normal bodies and one inline fragment now compile together: the
engine world factory, selected-zone arc and circle/segment geometry, profile
child append, profile scope entry, and the solver scope-exit fragment. The
ownership assessment identifies the actual director bytes and persistent ship
requests needed to connect these routines to the running host.

The original project remains `C:/Users/sqz269/bsp.gpr`, program
`/battlestationspacific.exe`. Root repaired the returning-free continuation in
`00C50390`, defined the verified director getter/setter at `00720E40` and
`00720E50`, saved annotations with prior values, and refreshed seven exports.
The inline `00C5BCD7..00C5BD03` remains inside `00C5BB30`.

## Verification

- Win32 Release and both existing CTests passed at `ba2bf73418153d9248053d393b2ef6a9a23ae54a`.
- World factory: four worlds per side, 280 live-buffer comparisons, matching
  growth and allocation/free publication events.
- Arc geometry: 21 circle-helper and 22 arc cases; 148 canonical words match.
- Profile scopes: eight enter/leave pairs per side, 265 buffer comparisons,
  32 hardware timestamp brackets and 16 modulo arithmetic checks.
- All 73 reviewed direct call rows passed. Two property-sink virtual calls in
  the ownership assessment remain outside that direct-call check.
- The exact compiled executable ran all 120 requested mission frames and
  exited zero. Its controlled-unit motion still contains NaNs, also observed
  in batch G. This validates the loop, not finite physics or gameplay.

The worker reports retain the ABI, original address, native-byte fixture,
source hashes and uncertainty for each reconstruction:
[factory](DYN_WORLD_FACTORY.md), [arc](AVOID_ZONE_ARC.md),
[profile](DYN_PROFILE_SCOPES.md), [ownership](SHIP_AI_AVOIDANCE_OWNERS.md).
`reports/orch6_reconstruction_h.json` records the combined build/run identity.

## Evidence and limits

Root archived 192 files under ignored `local/worker_evidence_h/`. Of these,
191 match their recorded historical hash; the arc worker's regenerable
`bsp_index.sqlite` changed after its manifest. Its old hash and current copy
are recorded separately; that current cache is not claimed as the historical
image. No native fixture mismatch was waived.

These are contextual C++ interfaces, not drop-in ABI replacements. Arc
solver-failure scratch, original CRT behavior, allocation failure, native
SEH/unwind and teardown remain bounded or unvalidated. Profile tests use
actual RDTSC and validate timing before normalization; equal cycle counts are
not claimed. Director/request/search-list runtime wiring continues in batch I.
