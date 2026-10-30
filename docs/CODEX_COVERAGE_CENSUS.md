# USN04 execution coverage census

Status: preparing the two execution traces. No original or rebuilt execution
totals have been measured by this packet yet. Board `orig_live` and `bsp_live`
are therefore null, displayed as **Not measured**, rather than zero.

The scope follows `local/CODEX_BRIEF_coverage_run.md` revision 2 in the
cc16-progress tree. The clock/histogram prerequisite follows sampler-gaps
revision 2. Long maneuver captures, campaign/direct-scene equivalence and
motion-trace comparisons are deferred.

## Tracer provenance

The official [DynamoRIO release](https://github.com/DynamoRIO/dynamorio/releases/tag/cronbuild-11.91.20749)
was downloaded into the empty ignored `local/tools/dynamorio/` directory.
There was no installer or system PATH change.

| Item | Value |
| --- | --- |
| Release | `cronbuild-11.91.20749`, published 2026-10-24 |
| Archive | `DynamoRIO-Windows-11.91.20749.zip` |
| Archive bytes | 467,145,305 |
| SHA-256 | `9ef2d8f990a6acb4d303385fec0105bd45e54d36e92269494ac8d45e966c3844` |
| Digest check | Matches the official GitHub release asset digest |
| Receipt | `local/tools/dynamorio/download-receipt.json` |
| Win32 runner | `DynamoRIO-Windows-11.91.20749/bin32/drrun.exe` |

The [official drcov documentation](https://dynamorio.org/page_drcov.html)
describes basic-block execution records and a separate output log for each
process. Basic-block reach is the observation; it is not a function-entry call
count. Attribution must include the actual game child process.

## Build and run evidence

The opt-in PDB build, same-commit normal build and code-layout comparison are
pending. Default build flags must remain unchanged. Coverage runs are serial,
at Normal priority through the shared launcher/slot/DRAIN protocol.

| Side | Build/hash | Start path | Window | Exit | Execution total |
| --- | --- | --- | --- | --- | --- |
| Original | Installed EXE SHA `b682a82c52f81f957b2c70222077305a933f72481686c88843077f714b956dd6` | Native USN04 direct scene, planned | Not traced | No run | Unknown |
| Reconstruction | Separate opt-in coverage build, pending | `--menu-select USN04`, planned | 4,800 mission frames at 0.05 s/frame, planned | No run | Unknown |

Original tracing is permitted only if the guard retains its restricted token,
source read locks, job ownership, private Documents redirect and bounded
cleanup. The current guard uses a debug connection to establish its redirect;
safe tracer integration is under review. A fallback must retain these same
protections. This packet will not run the installation outside that contract.

## Attribution and limits

The census will map original-main-module blocks through saved Ghidra function
ranges and preserve undefined-code, EH-funclet and unledgered buckets. Rebuilt
attribution uses PDB symbols and inline-site records, then original identifiers
through wire-readiness resolution. Missing symbols, incomplete inline handling
and unresolved identifiers must remain explicit.

An unavailable side is unknown in the per-address table, not false. The
original-yes/rebuilt-no wiring frontier cannot be measured until both sides
have attributable observations. One mission's unreached functions are
**not reached in USN04**, not dead code. Differences can also reflect the
different start paths, windows, seeds, instrumentation and host behavior.
Execution coverage proves reach, not correctness.

The committed `config/coverage_census_summary.json` supplies the board figures;
large raw logs and detailed per-address results stay under ignored `local/`.
