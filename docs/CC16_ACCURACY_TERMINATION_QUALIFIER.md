# Current accuracy cohort: termination qualification

The sealed four-run cohort supports its existing scoped numerical/event/teardown gates and **four launcher-reported logical exit-zero results**. The frozen legacy launcher's 1,200-second force-stop branch was not reached on the recorded short runs. The receipts do not independently record all eight parent/child OS exit codes or the later `NoForceStop` natural-drain flag. This is an additive qualification, not evidence of forced cleanup or a reason to rewrite the sealed results.

Reviewed source baseline: `605373bf732e4bd3089e8f07ddca226ab4d635a3`. The actual runtime host remains sealed at `2c25911ef30940f93ef3a56c6202198013b59b45`. Its `runtime_results.json` rehashes to `2f5dfc16cf391d4271d49d950a1cfc880ba781bc3cbb75e037046743e3f49571`; all 52 artifacts listed by that report were rehashed without mismatch. Authentic retained ON/OFF EXEs/maps were used; changing root build objects and the later movie-publication build were not used.

## What actually ran

The exact binaries remain ON `7c22b561ae0f6cc38276d62e1468ceab358827ca043ff634842a8aa0aab9056a`, OFF `8a83cf434cdd6d525696fb4574ce2db760b2c64dbff8781143b850653a1ba049`, with backend `f11299db80cd48a355c9db2bf3de5ebc4707ba46d38d0ae5e8eb0da616f87fb3`. The frozen external controller is PowerShell `7.6.9`, with its recorded executable hash; the reconstructed EXEs are Windows GUI subsystem images.

The actual frozen legacy launcher invokes:

```powershell
& $Exe @arguments 2>&1 | Select-Object -Last 1
$code = $LASTEXITCODE
```

It writes `manifest.exit_code` and `finished_utc` from that pipeline observation **before** its separate tagged-process drain. This records one logical wrapper result, not an explicit held-handle `WaitForExit/ExitCode` receipt for each process. The launcher's comments about a previously observed early pipe close do not establish that it happened in these four runs, nor that the actual bootstrap-parent function returns before its child.

The legacy `Get-TaggedGame` selector uses executable name plus instance-tag regex, not the path asserted by its obsolete comment. Its CIM query uses `-ErrorAction SilentlyContinue`; an empty legacy query alone is not a strict natural-completion certificate. The sole launcher `Stop-Process -Force` branch follows 600 two-second sleeps (`waited >=1200`) with tagged survivors. All four runs finished far earlier, and neither controller stdout contains its `stopped lingering` message.

| Run | Manifest elapsed (s) | Manifest finish → `.controller_done` (s) | Manifest finish → controller finish event (s) |
| --- | ---: | ---: | ---: |
| native OFF | 89.314203 | 0.522762 | 2.626515 |
| native ON | 89.210543 | 0.538733 | 1.701907 |
| IJN18 OFF | 169.796983 | 0.853205 | 2.317632 |
| IJN18 ON | 172.518872 | 0.457440 | 1.430206 |

All four manifests/report events contain logical code zero. The marker is written in the controller's `finally` after the launcher returns, **before** waiting for the watchdog to retire; the later finish event is a different timestamp. Neither marker is an independent game-process termination status.

## Independent evidence and its limit

The frozen owned watchdog did hold and identity-check all eight sampled process handles, binding expected EXE/hash, log/tag, owner, start time, parent relationship and Normal priority. Its successful boundary requires a nonnull manifest exit code, a timely finish timestamp and a fresh strict full-EXE/log/tag CIM query with zero matching candidates. All four watchdog receipts report `finished_before_hard_limit`, logical `game_exit=0`, and empty actions arrays; the four-hour termination branch was not entered. This provides useful independent absence/identity evidence, but the successful branch does not save each held process's terminal `ExitCode`.

The primary also reported a later strict CIM query finding none of the eight historical PIDs present. That is corroborating later absence, not a historical per-process exit-status record; this audit makes no new live process query.

The actual ordinary bootstrap-parent implementation holds the real child process handle, waits `INFINITE`, checks `WAIT_OBJECT_0`, obtains `GetExitCodeProcess`, destroys its already-mapped child wrapper and returns the retrieved child code. Authentic ON map/PE evidence confirms parent `1000C990`, wait call `1000CAB7` through IAT `1059A134`, exit-code call `1000CAD6` through `1059A140`, and the retrieved value moved to `EAX` at `1000CAFA`. The source child wrapper's normal mapped close releases handles; its separate mapping-failure termination is not the ordinary successful return path.

Thus a genuinely measured normal parent return forwards the child's status. That code property does not retroactively add eight independent terminal observations to the old pipeline/manifest receipts. It also supplies no basis for claiming these short runs were force-terminated.

## Preserved gates and future strict drain claims

The recorded original/strict sampled-projection checks remain valid for their exact frozen cases: matching sampled RNG/event sequences, zero ordered index differences (762 native /636 IJN18 lines) and zero teardown index differences (10,749 native /238 IJN18 lines). IJN18 records the required accuracy/completed-rating/bearing activity and a source-format sample after a completed return. These are the existing scoped gates, not broad retail parity, objective completion or general gameplay proof.

No repeat is required merely to preserve those numerical claims or to rebut an unsupported allegation that the legacy stop branch ran. A **new strict natural-drain claim** needs a fresh witness using the explicitly selected `-NoForceStop` protocol; its flag cannot be backfilled into this sealed cohort. Peer source `631c9a3d50` adds that opt-in policy while retaining the old default. Future gates should require `drain.no_force_stop=true`, `natural_completed=true`, `timed_out=false`, empty stop/retained PID arrays, original and effective launcher exit codes zero, successful exact-identity queries and the pinned real processes. A claim that all eight OS codes are zero additionally needs independently retained terminal status records for those held identities; the new manifest flag alone does not supply them.

The old receipts remain unchanged. No game, probe, process control, launcher edit, Ghidra mutation, build or test was run for this audit. No source activation, native/vendor-body, ABI or additional gameplay credit is added. Exact hashes, timestamps, static PE bytes and qualification scope are in [the audit report](../reports/cc16_accuracy_termination_qualifier.json).
