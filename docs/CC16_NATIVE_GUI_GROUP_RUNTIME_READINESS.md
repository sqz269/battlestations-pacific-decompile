# Current GuiGroup Host runtime readiness

Baseline: `5dc9e9a018c8cb59b606fe904fca95ed7f9c574a`. **No runtime window or launch clearance is established.** This packet queried existing process metadata and read the current harness/coordination inputs. It launched, controlled, stopped and restarted nothing. Candidate `6772843657c3ab62f7bb4f9ae613adbac0770c80` and independent review `41106def0cae436ea117b563ce5470bb96f4c161` remain frozen; Root owns common/current-Host admission of the staged default startup caller.

## Existing process and ownership

At `2026-10-30T11:40:30.579Z`, direct unelevated `OpenProcess(40812)` failed with **Win32 error 5, Access is denied**, for each permitted mask: `101000h` (query-limited plus synchronize), `1000h` (query-limited), and `100000h` (synchronize). No handle was acquired. Consequently `GetProcessTimes`, `GetExitCodeProcess`, `WaitForSingleObject(handle,0)`, path and handle-bound creation identity were not obtained. Access failure is not exit, and no terminal/relaunch inference is made.

Historical parent PID31836 returned error87 (invalid parameter) for all three opens; the fresh CIM query returned no parent row. Neither result proves the historical parent's terminal identity or identifies a current controller. PID reuse must be checked against creation time whenever an actual handle becomes available.

Fresh CIM at `12:01:22Z` still reports `bsp_game.exe`, PID40812, parent31836, creation `2026-10-19T03:00:25.0926700Z` (the original `2026-10-18T20:00:25.09267-07:00`), session1, null command line/path, and zero reported threads/handles. These are CIM observations, not successful wait/exit/loaded-image proof. Root's earlier .NET HasExited/path/start query was also unavailable. The exact expected creation FILETIME at CIM microsecond precision is `134368524250926700`; a future query must preserve the exact Win32 FILETIME as well as PID and handle, rather than treating a later PID match as this process.

Named `C:/Users/sqz269/.bsp/bsp_game*.lock` records were empty at observation. This establishes no drain. No current owner/controller/creation-bound process handle was found for 40812. Coordinator message1053 at `01:25:40Z` explicitly listed it among five old PIDs to preserve; that historical preparation/capacity record grants no current runtime authority and none of its old counts are treated as current.

## Current coordinator route and harness limits

Messages1101/1102 were addressed to bare `cc16-progress`. The actual `coordination.addressed_to` predicate does not match those rows to canonical `agent/cc16-progress`; it matches only the bare owner. This explains a routing mismatch, not a coordinator decision. Root corrected the route with message1103 to canonical `agent/cc16-progress`, reporting Source/common/PE progress and requesting the current window/controller command. No new launch clearance follows from the correction. No acknowledgement was forged or mailbox state changed here.

The working entry is `tools/run_game.ps1`. It atomically reserves a numbered slot, records owner/tag/core/arguments/PE hash/checkout/input snapshots in `<log>.manifest.json`, invokes the target, and drains tagged bootstrap/child processes. `-NoForceStop` is mandatory: on a retained-survivor timeout it returns failure and retains the processes. Default operation can force-stop tagged survivors; it is excluded from this proposal.

The helper's tag query is CIM `CommandLine` matching. A null command line cannot attribute PID40812 to a tag. Its `finally` removes its slot file even after retained survivors, and manifest failures are warnings. Thus an empty slot set, launcher return, manifest `natural_completed`, or an empty tag query alone is not authoritative terminal proof. Use Root/coordinator's current controller/session and creation-bound bootstrap AND child handles. Re-poll the SAME actual handle on observation timeout; never restart because the observer timed out. Do not delete locks or mutate DRAIN to manufacture a window, impersonate the coordinator, or reuse stale lease/capacity permissions. The helper's existing affinity/tag settings also require the coordinator's current cohort agreement.

## Bounded Host invocation, conditional on admission

The new readiness checkout is not the executable's Source admission target. Root's staged tree is `J:/PROG/battlestations-pacific-decompile-codex-function-source-integration`. Read-only snapshots at `11:51:08Z` found its new Main status literal in Source and the on-disk PE, plus the relevant MAP symbols. This is not loaded-code or launch approval. The observed inputs were:

| Input | SHA256 |
| --- | --- |
| `build/win32/Release/bsp_game.exe` | `3e978e1458d38707442b97abd0c658f5d1a324ee3afd5bc4794eaad609f35ba0` |
| `build/win32/bsp_game.map` | `cf71f08d97212453bc00ecc3e51234cc67cf0c84f79045addfd6123d09546100` |
| `build/win32/Release/xlive_stub.dll` | `f11299db80cd48a355c9db2bf3de5ebc4707ba46d38d0ae5e8eb0da616f87fb3` |

Root's final common-admission Source/PE/MAP receipt must match the actual executable to be launched; re-pin after any rebuild. Current typed Source is independently pinned in the snapshot and inherits no frozen candidate/cohort result merely because interfaces match. Installed data root remains `I:/SteamLibrary/steamapps/common/Battlestations Pacific`; personal/settings/log outputs use a fresh private directory. The stand-in DLL is the existing Source Host harness provider, not Original/GFWL parity proof.

After the coordinator supplies an exclusive current window/controller and `ApprovedSlot`, the existing helper invocation is:

```powershell
$Stage = 'J:/PROG/battlestations-pacific-decompile-codex-function-source-integration'
Set-Location -LiteralPath $Stage
& "$Stage/tools/run_game.ps1" `
  -Exe "$Stage/build/win32/Release/bsp_game.exe" `
  -GameRoot 'I:/SteamLibrary/steamapps/common/Battlestations Pacific' `
  -XLiveDll "$Stage/build/win32/Release/xlive_stub.dll" `
  -Log "$Stage/local/function_source_batch17/group_pool_host_current.log" `
  -NoForceStop -Slots 14 -SlotIndex $ApprovedSlot -WaitSeconds 0 `
  -GameArgs @('--frames','1','--difficulty','1',
    '--settings-personal-root',"$Stage/local/function_source_batch17/group_pool_host_personal",
    '--window-monitor','smallest','--window-resolution','fit',
    '--present-interval','immediate')
```

This command was not executed. `$ApprovedSlot` is deliberately not invented here; the log/input directory must be fresh and owned by that approved cohort. Do not set `BSP_AGENT` to the coordinator's identity, bypass DRAIN, or use a qualification/replay/frozen execution tree. Exact current controller/observer command and authoritative handle custody remain unresolved pending the coordinator; this helper command alone does not close that gap.

`--frames 1` follows real `run_win_main`: application construct/initialize, platform loop, application shutdown/destruct, singleton drain, random-thread shutdown, then normal Main host reset/log close/return. It bounds this Host run without mission/input injection. `--qualify-vfs-physical-read-owner` and `--qualify-vfs-failure-owner` return BEFORE the pool subset and cannot prove the new slot. Omit both. A single-instance early return, parser/data/Host failure or `_Exit` retention is not a successful normal shutdown.

## Required observations and proof limits

Require the same admitted current image/Source, real bootstrap/child PID+creation+handle ownership, and ordered logs:

- `native GUI Text pool CRT CD71F0: ...`, then `native GUI Group pool CRT CD7440: atexit=0 storage_bytes=56 before_particle_pool=1`, then particle startup.
- `frame limit 1 reached, requesting loop exit`; summary `window_created=1 device_created=1`, at least one presented/skipped frame, `loop_finished=1 exit_code=0`.
- Host method summary has concrete calls for application initialize, platform loop, `StartupHost::application_shutdown`/`application_destruct`, singleton lifetime drain and random-thread shutdown. Source still reports the unbound native singleton-shutdown portion separately; method invocation is not full Original shutdown parity.
- Normal current child and launcher terminal observations, final zero exits, complete manifest/inputs and no retained owner, startup failure or forced-stop path. The controller must corroborate real handles; tag/lock/pipe observations alone are insufficient.

A successful registration0 run would prove this current Main path called real startup and received successful CRT registration, then completed the bounded actual Source Host lifecycle. Normal CRT callback discharge follows the admitted code/normal-return contract; the Group callback currently has no dedicated log marker, so direct execution of its instruction body is not claimed from the status line alone. No such run occurred here. It would not exercise nonzero registration, constructor interruption, allocation failure, arbitrary fault transport, UI Group payload creation, full ABI or gameplay parity.

The nonzero branch is supported separately by static proof: the SAME process backing/profile remains alive without an automatic C++ unbind, no native teardown callback is registered, the returned initialized state/status is retained, and the shared root retires after dependent callbacks without map-driven native traversal. This is not runtime credit for a branch never executed. No fake status/atexit injection framework, fallback, manual teardown or new terminal policy is proposed.

Runtime readiness remains false until current controller/owner/handle agreement and exact common/current-Host admission are complete. User authorization permits required gameplay; this packet performs none. Preserve `NoForceStop` and all frozen dc2/d8/d58 trees. Only this document and its report change; Source/shared/CMake/Ghidra files, compiler/link/test tools and game execution were untouched. Ignored receipts retain exact API errors, CIM/routing/lock observations and copied current Root inputs.
