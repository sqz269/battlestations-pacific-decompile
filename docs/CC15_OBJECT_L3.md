# cc15 object bring-up, lane L3: loading queue, application, VFS pump

Lane cc15-ob3 covers ranks 10, 12 and 13 of the ranking in `docs/CC15_WIRE_FRONTIER.md`. All three objects sit
in `game_hosts.cpp`. Each one has its own switch. `kLoadingQueueCc15Bound` is ON (see its "Flipped ON" note); the
others are OFF.

## Native loading queue (rank 10), `kLoadingQueueCc15Bound`

**Evidence.**
- 004FDE20 `BSP_LoadingQueue_GetInstance` is a lazy singleton with publication cell E18D4C. It allocates 20h and
  constructs through 004FDBA0. Construction stores table CEB198, a manual-reset event at +0C (00BD1970), and
  zeroes at +04/+08/+10..+1C. The getter then registers the owner through 00BD0C30. All of this is already in
  `src/native_loading_queue_owner.cpp`.
- 00737A50 RunFrame calls 004FDE20 and then 00509190 on the result, every frame.
- 00509190 Update returns at once when `[ECX+14h]` (the job count) is 0.
- 0050ACE0 is the CEB198 scalar deleting destructor and calls 00509FD0. That destructor:
  - restamps CEB198;
  - calls 005092E0, which runs a trivial 004254B0 diagnostic and returns when the count is 0;
  - calls 004FB4B0(+10, 0) and frees +10 through 00BF6989;
  - when +0C is set, calls slot 0 of the event with 1 (BD19B0), then clears +0C;
  - clears E18D4C and stamps CE3818.

  0050ACE0 then frees the owner when flags bit 0 is set (00BF65AC).

**Lifetime as wired.** `GameFrameHost::update_loading_queue` (the 00737A50 slot) calls
`get_native_loading_queue_owner_004fde20` with the application manager cell 01090AA0 and an E18D4C cell owned by
`GameSingletonHost`, then calls `update_native_loading_queue_00509190`. The first frame builds and registers the
queue. The singleton manager drain (00BD0400) deletes it through the new CEB198 profile, which borrows the same
E18D4C cell through `NativeSingletonDeletionBindings::actual_loading_queue_publication_00e18d4c`. That field is
appended at +208, and the size becomes 212.

**ABI.** The new Source interfaces take explicit owner, flags and cell arguments. They are not ABI-compatible with
the native ECX/RET4 forms.

**Scope.** Only the empty queue is admitted. A non-empty queue throws a source contract error in 00509190 and in
the drain. The native bindings that path needs do not exist:
- the FileStore request;
- 00506BF0 retirement inside the update;
- 00501620 work start;
- the 005092E0 loop, which sleeps 100 ms, then pumps 00BDB0B0 and 00509190.

The host never enqueues into the native queue, so in a mission the queue stays empty.

**Rows wired.** `ApplicationFrameHost::update_loading_queue 004fde20` goes from UNIMPLEMENTED to concrete (5000
calls). Of the frontier's 3 "unlocks", the 00509190 update is wired. The enqueue paths (005055C0/00504D20 from the
frontend) remain host-interface bodies.

**Pairs (USN04, 5000 frames / 4800 mission frames).**
- OFF (6e34d1010 tree build) vs baseline (a84b1ddac export): `pair_diff` exit 0. The only noise is pretranslate.
- ON (`pair_export --flip kLoadingQueueCc15Bound=true`) vs OFF: exit 1, gameplay identical. Death rows, plane death
  modes, the 90-row unit table and summary lines are all identical. The moved rows are:
  - `update_loading_queue` UNIMPLEMENTED -> concrete, so the counts go 1075/474 -> 1076/473;
  - `singleton lifetime drained: registered_slots` 43 -> 44, which is the queue's registration. It drained
    through CEB198 without error.

**Uncertainties.**
- In the image, the first 004FDE20 call may come earlier than the first RunFrame. For example, a frontend enqueue
  or the 005092E0 stop on menu screens could call it first. That would change the queue's registration slot, and
  therefore its drain order. Within this host, RunFrame is the first caller.
- The event is deleted through `delete_native_event_owner_00bd19b0` directly, not through slot 0 of its table.

**Flipped ON (cc15-flip, base f906d5934, 2026-10-19).** `kLoadingQueueCc15Bound` is now true. The pairs were run
in the ref env (`BSP_DEATH_TABLE=1`, `BSP_GUNNERY_RNG_STREAMS=1`, `--stop-after-end 40`; each log has the
in-process "rng streams: on" line). They cover USN04 plus the 12 wins rows: u01, u02, l6, b9, l10, u12, p01, b1,
i18, u21, j6 and e8.
- All 13 rows give `pair_diff` exit 1 with gameplay identical. Death rows, unit tables and summary lines are
  identical, and every row keeps its mission-end line and time (BSM09 285.05 s).
- Each row moves exactly three lines:
  - `update_loading_queue 004fde20` goes from UNIMPLEMENTED to concrete, with the call count unchanged;
  - `singleton lifetime drained: registered_slots` goes 43 -> 44 (the line is removed on OFF and added on ON).
    The queue drained through CEB198 without error.
- No contract error appears. Both sides have 671 lines that mention "contract", all of them pre-existing summary
  lines, and no loading-queue or non-empty-queue line. The queue stayed empty.
- I ran a strict index-by-index diff of the activation, draw, power-up, weather, voice, warning, clip, message,
  dialog, hint, objective and Lua lines, with `registered_slots` dropped. It shows 0 mismatched positions on all
  13 rows.
- The save and profile files under Documents\Battlestations-Pacific are unchanged: the SHA-256 and mtime of all
  6 entries are the same before and after.

## Native application object (rank 12), `kApplicationCc15Bound`

**Evidence.**
- 00737970 (RET, ECX receiver) calls BEA970 and BEA810. These register the receiver in the singleton manager and
  publish E1AE90. It then stores CFEAB0 at +0, zero at +08/+0C/+10/+14, 0 at +19/+1A and 1 at +18.
- WinMain constructs this 1Ch object on its stack at 008F8496. At 008F8444, after 00737F30, it retires the object
  through 007379A0. 007379A0 runs resize(0) on the +08 vector, frees the vector, then calls BEA990/BEA8B0, which
  unregister through BCFCA0 and clear E1AE90. All of that happens before the 008F8449 manager drain.

**Lifetime as wired.**
- `GameStartupHost::application_construct` claims the process Application domain and calls
  `construct_native_application_00737970` on a 1Ch DWORD array owned by the startup host. That array stands in for
  WinMain's stack frame. The call borrows the 01090AA0 manager cell and the 00E1AE90 cell.
- `application_destruct` calls `retire_native_application_007379a0`. The array services there are
  `singleton_lifetime_allocate` and `singleton_lifetime_free`, the same pair the renderer uses for 00BF55BE and
  00BF6989.

These are the existing explicit-context Source APIs, not the native ECX ABI.

**Pairs (USN04).**
- OFF (6fd894f70) vs baseline: exit 0.
- ON vs OFF: exit 1, gameplay identical. The native table has no status or count changes. The moved lines are:
  - `native application constructed: storage=X e1ae90=X`;
  - `native application retired: e1ae90=null`;
  - drained `registered_slots` 43 -> 44. BCFCA0 clears the Application's slot but keeps the vector length, so the
    count is expected.

**Rows wired.** None. The frontier listed 0 unlocks, and `application_construct` was already counted as
implemented.

**Uncertainties and incompatibilities.**
- The frame loop still reads the host's `ApplicationFrameState` projection, not native +18/+19/+1A. Those native
  bytes are written once by the constructor and never read.
- The `BSP_NATIVE_MISSION_SHIP=1` preload clash is resolved; see the next section. The switch stays OFF until the
  lead schedules its own flip commit.
- The +05..07 and +1B bytes are zero here. In the image they are stack garbage that no store touches.

### The mission ship preload borrows the startup Application (cc15-app)

**Evidence.**
- 0073D410 saves its ECX at 0073D42F/0073D437 and reloads it into EBP at 0073DBCD. At 0073DD09 it passes EBP as ECX
  to 0073BF80 (call at 0073DD12). So the shader preload fills WinMain's stack Application, not an object of its own.
- 0073BF80 reads and writes only the +08 pointer vector of that Application: +0C count, +10 capacity, the 00735EC0
  reserve and +08 data. It appends one row per `FileNames` entry.
- 00737F30 releases those rows at 00737F72..00737FB0, right after the renderer vtable +0C/+14 calls. Each nonnull row
  is decremented through CE2220, deleted at zero, then cleared. Later, 008F8444 runs 007379A0 on the same object.

**Change (`97d2974e4`).**
- With `kApplicationCc15Bound` ON, `MissionShipShaderPreloadGraph::create` neither claims the domain nor constructs.
  It takes the Application that the startup host's 00737970 published at E1AE90 and requires it to hold an empty
  vector, then runs 0073BF80 on it.
- `GameStartupHost::application_shutdown` (00737F30) calls `GameNativeRendererApplication::release_application_preload_rows`
  at the 00737F80 marker. That call runs the same 737F80..737FB0 release loop on the borrowed rows.
  `application_destruct` then runs 007379A0, which frees the vector the preload grew.
- The vector allocator pair is the same `singleton_lifetime_allocate/free` in both owners.
- The preload's later `retire_before_native_drain` sees its rows already released and skips 007379A0 for a
  borrowed object.
- With the switch OFF, the preload still constructs, releases and retires its own Application, as before.

**Remaining difference.** The image preloads during 0073D410. The diagnostic preload still runs at mission-ship
prepare, so the rows are appended later. The object, the vector and the release/retire order now match the image.

**Runs** (exes in `local/cc15app_{ref,off,on}`; `ref` is origin/main `7f4f75b85`):
- USN04, ref env, OFF vs ref: pair_diff exit 0. Dialog/message/Lua lines, strict index diff: 678 = 678. The only
  mismatch is the renderer worker thread id.
- USN04, ref env, ON vs OFF: pair_diff exit 1, gameplay identical. Native table 1589 rows with no changes, 793 summaries
  identical, and death, plane-death and unit tables identical. The moved lines are the expected three:
  `native application constructed`, `native application retired` and `registered_slots` 44 -> 45. The strict
  sequence diff matches the OFF/ref result (only the worker id).
- ON with `BSP_NATIVE_MISSION_SHIP=1` (USN04, 600 frames, press-start 30, 181 mission frames at 0.05 s,
  `--screenshot`) exits 0 and draws the Fletcher at mission frame 180 (`local/cc15app_ship_on_N180.png`):

  ```
  native application constructed: storage=006C3B14 e1ae90=006C3B14
  native mission preload borrows the startup Application: storage=006C3B14
  native mission preload complete: application=006C3B14 entries=179 site=0073c204 cache=closed
  native mission ship capture landmark: mission_frame=180 simulated_seconds=9.000016 ... models=26
  native mission preload rows released: application=006C3B14 rows=179
  native application retired: e1ae90=00000000
  native mission ship closed: last_mission_frame=181 ...
  ```

- The save files under Documents\Battlestations-Pacific were identical before and after the runs.

## Native VFS manager pump (rank 13): already native, no change

`GameFrameHost::tick_vfs_providers` -> `GameVfsHost::pump_pending` -> `GameNativeVfsRuntime::pump_pending` already
calls `pump_native_vfs_pending_00bdb0b0` on the live 0109CEEC VFS publication, reloaded just before the call as
737B79 does. The VFS manager is the native one that the VFS runtime builds. Adding another pump would call
00BDB0B0 twice per frame, which duplicates the existing pump. Following the brief, nothing was changed.
