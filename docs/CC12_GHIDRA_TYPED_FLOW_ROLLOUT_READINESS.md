# CC12 typed-flow Root rollout readiness

**Readiness only; no save, shutdown, restart, deployment, configuration change,
new endpoint call, GPR write, or Native opening was performed.** The mandatory
`bsp.py brief` performed its existing health/identity probes. All further live
evidence came from process, listener, configuration, file metadata, and logs.
Implementation and rollback pins remain in
`CC12_GHIDRA_TYPED_FLOW_IMPLEMENTATION.md` and its report; this note resolves the
shutdown/restart mechanics without authorizing their execution.

## Observed session and provenance limits

At this inspection, the GUI JVM is PID **33644**, Corretto **21.0.9**, started
2026-09-28T04:36:33.918Z. Its command line names
`J:/tools/ghidra_12.0.4_PUBLIC` and `C:/Users/sqz269/bsp.gpr`; the window title is
`Ghidra: bsp`, and PID 33644 owns the `127.0.0.1:8089` listener. Recheck PID, start
time, command line, window, and listener immediately before a future close.
Never target the separate Gradle JVM or all Java processes.

The saved `bsp.rep/projectState` already records a running CodeBrowser with
`/battlestationspacific.exe`, and `_code_browser.tcd` enables `GhidraMCP`.
The September 27 startup log reports GhidraMCP 5.6.0 build `20260504-235055`.
The installed disk JAR still has SHA-256
`3eb3999c930e844ddda1f74572b2d414cbacc645356593bf2cc44ccc038aad48`.
These agree with the expected installation; they do **not** attest the loaded
class's CodeSource or prove current unsaved program state. No JVM attach,
class inspection, or endpoint call was used to strengthen that claim.

The configured MCP bridge remains `uv run --script
J:/tools/ghidra-mcp/bridge_mcp_ghidra.py`. That checkout still has its three
preexisting dirty files; no bridge or configuration edit is needed for the BSP
client's direct HTTP capture. No script permission setting was enabled.

## Save and close semantics

There is no `save_project` tool or source route in the inspected GUI surface.
Available tools are `save_program(program=...)` and `save_all_programs()`.
The former's `GET /save_program?program=battlestationspacific.exe` resolves the
explicit program and invokes `DomainFile.save` on the Swing event thread. A
successful program save does not itself save all GUI workspace state or close
the project. `save_all_programs` saves each currently open program and returns
per-program errors; use it only after Root has reviewed that broader scope.

`close_program` closes a program, not the JVM; its GUI path increments its
closed count without checking the `ProgramManager.closeProgram` return value.
Its response is therefore insufficient shutdown evidence. `open_project` and
`close_project` belong to the headless-only service and are absent from the GUI
schema. Do not route this GUI rollout through them.

`open_program(auto_analyze=False)` still calls `suppressAnalysisPrompt`, which
changes prompt flags and saves the program. It is not a read-only fallback.
Prefer reopening the saved project through its existing clean-exit state. If
that fails, stop for Root's explicit recovery decision rather than opening or
analyzing a program automatically.

## Concrete coordinated sequence for Root

1. Finish implementation integration and review the chosen full/overlay package.
   Coordinate a maintenance window with all harness owners and the user of the
   GUI. The inspection saw independent resume-integrator and Type10/11 harness
   leases. Quiesce their Ghidra queries and mutations, including automated readers
   that might relaunch the server. A free write lock is not evidence of quiescence.
2. In one live Root orchestration process, hold
   `coordination.ghidra_lock(ttl_seconds=1800, purpose=...)` for the complete
   save/close/backup/install/start sequence. Acquire
   `ghidra_launch._take_launch_lock()` too, and abort if it is held elsewhere.
   The launch lock expires after 12 minutes and has no automatic renewal; plan
   within that window and stop if ownership or time remaining becomes uncertain.
   Do not invoke `ensure()` while holding its launch lock yourself.
3. Under the write lock, reverify project/program identity and the explicit
   program list, then call `save_program` with the BSP program name and retain
   the successful response. If other programs need saving, coordinate and save
   them explicitly. On any save error, leave Ghidra running and stop.
4. Recheck the exact owning PID and identity. Call
   `ghidra_launch.close([verified_pid], grace=60, force=False)`, which requests
   `CloseMainWindow()` and waits. Do not call it with a broad Java PID list.
   If it returns false, resolve the actual save/confirmation dialog through the
   GUI and recheck; never force-kill, discard changes, or replace a live JAR.
   The helper checks for *any* Ghidra JVM on exit, so another legitimate Ghidra
   process must be distinguished manually instead of terminated.
5. Confirm the target JVM has exited and port 8089 has no owner. Back up the
   complete closed project described below, plus the original extension and
   saved tool/preferences needed for restoring the workspace. Verify copied
   files against a manifest collected after clean closure; the live metadata
   snapshot in this report is not a consistent database-backup manifest.
6. Recheck installed preimages, install only the selected reviewed artifact,
   and verify its output hash using the implementation note's exact mapping.
7. Still under the launch lock, invoke `ghidra_launch.launch(config)` directly.
   It starts the configured `ghidraRun.bat` with the same `.gpr`, using
   `CREATE_NO_WINDOW` for the helper. This launches Ghidra without calling
   `inject_running_tool`; it uses the state from clean GUI exit. Poll health and
   explicit program identity with short bounded waits. Retain startup logs and
   stop for GUI recovery if a prompt appears or the program is absent.
8. Release the launch lock in `finally` and the write lock only after the
   coordinated operation has completed or safely stopped. Resume other harnesses
   after Root verifies the session. Any typed-flow call remains a separately
   approved bounded capture; this readiness work supplies no Native admission.

`ensure(restart=True)` is unsuitable for step 4: it returns immediately when
the server is healthy, before inspecting `restart`. When the server is down,
its restart branch can close existing JVMs, then `inject_running_tool` may write
`bsp.rep/projectState` and overwrite `projectState.bak`. Direct launch after an
explicit clean close avoids that additional state-edit path. No launcher code
was changed for this note.

## Exact backing paths and rollback scope

| Path | Role |
| --- | --- |
| `C:/Users/sqz269/bsp.gpr` | Zero-byte project marker; insufficient alone for backup |
| `C:/Users/sqz269/bsp.rep/idata/` | Private project data; six files in this metadata snapshot |
| `C:/Users/sqz269/bsp.rep/user/` | Program/user data; seven files |
| `C:/Users/sqz269/bsp.rep/versioned/` | Versioned storage; two files |
| `C:/Users/sqz269/bsp.rep/project.prp` | Project properties |
| `C:/Users/sqz269/bsp.rep/projectState` | Saved workspace/tool state |
| `C:/Users/sqz269/bsp.rep/projectState.bak` | Launcher state-file preimage only; not a program database backup |
| `C:/Users/sqz269/bsp.lock`, `bsp.lock~` | Active-project lock artifacts; do not manually delete or restore stale locks |

Back up the `.gpr` plus the **entire** `.rep` tree after clean exit, to a new
dated destination under Root's approved backup directory, with per-file hashes.
No existing full-project backup procedure or backup tool was found in the
bounded launcher/coordination/script inspection. Do not treat the old
`projectState.bak` as one. An ordinary plugin rollback restores the three frozen
extension files and restarts the same saved project; it does not restore or
rewind the analysis database.

The exact original extension root is
`C:/Users/sqz269/AppData/Roaming/ghidra/ghidra_12.0.4_PUBLIC/Extensions/GhidraMCP`.
Its `extension.properties`, `Module.manifest`, and `lib/GhidraMCP-5.6.0.jar` are
pinned individually in the readiness JSON, with their frozen rollback copies.
The complete implementation bundle is 46,283,299 bytes, SHA-256
`d3da681aecafa202f5058f67a97c222b6801fffa278b59278e1d7101e9b59aaa`;
retain it outside this worker worktree before removing the worktree.


## Primary historical review

Root reviewed the save/close/launch methods and retained23 artifact pins after
the separate clean rollout. The original JAR pin replays against its closed
backup. Two hashes describe pre-close live projectState/tool-template bytes;
clean GUI exit changed those files, and the worker retained no pre-close byte
copy. Root preserves the reported observations but cannot independently replay
those two historical hashes. This readiness receipt is distinct from the Root
execution/closed-backup and corrected runtime-validation receipts.
