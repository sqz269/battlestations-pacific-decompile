# CC13 hiddencarry: Command and StartSpeed for GenerateObject spawns

Packet cc13-hiddencarry. Switch `kSceneHiddenSpawnCarryCc13Bound`
(`include/bsp/game_hosts_scene_contents.hpp`), **OFF**. Descriptive names are hypotheses,
not recovered symbols. Built and build-tested; not yet run ON.

## The gap

A scene entity with `Hidden = B true` is held back for `GenerateObject`. This host's hold-back
branch (`src/game_hosts_scene_contents.cpp`, "Hidden: held back for GenerateObject") put the
record in `scene_spawn_pool()` and returned before the two reads the created path makes from
the entity's property bag:

- the creator's `Command` / `CommandTarget` queue (`004E6B30` -> `00469610`);
- `retain_start_speed`, the reader for the `StartSpeed` key that `00822C20`'s arm `0082356C`
  seeds the order ring from.

So every script-spawned unit reached `create_units` with no command and no start speed. Found on
USN21 (`usn_sibuyan.scn`): the Intrepid group (`Command = Cruise`, `StartSpeed = F 10.0`) stood
still with state=stop, and its followers held station around it at throttle 0.

## The image chain

`GenerateObject` `00944FD0` has two creation arms that join at `0094530F`:

1. With a position: `00944FD0` calls `004C6BE0` at `009452CA`. `004C6BE0` calls `0046DC10` at
   `004C6C01`.
   - In `0046DC10` the class creator is the indirect `CALL EAX` at `0046DE9A`. Its third
     argument is `[record+4]`, the authored bag of the scene database record. The
     MotherShipGen creator `004F0860` calls `004E6B30` at `004F0913` with that bag, which
     queues the command.
   - `0046DC10` calls `00922E20` at `0046DEC1` to wrap the same `[record+4]` bag at
     entity+C0h. It then calls `00925F20` at `0046DEDD`. Pass B (`00822C20`, arm `0082356C`)
     reads `StartSpeed` from that holder and seeds ring+148h through `0080D9B0`.
2. Without a position: `00944FD0` calls `004C6BA0` at `00945308`, which forwards to `0046D930`.
   That is the same sequence: the creator at `0046DB4B` with `[record+4]`, the holder at
   `0046DB88` and `00925F20` at `0046DBE8`.

Both arms then go through `00945311`, where `00944FD0` calls `00874D00`. After that,
`00944FD0` calls `0046AAB0` at `009453BB`. That resolves the queued command, and its target
lookup runs over the entities that exist at that moment. The director's cruise latch
(`00835E17`) then captures thrust = ring+148h, and `009E1170` orders that throttle on the
spawn heading every AI step (docs/CRUISE_SPEED_SETTING.md).

## The host change

- **Hold-back:** after `scene_spawn_pool().add(record)`, the carry goes onto
  `SceneSpawnPoolEntry`, and only when `add` inserted a new entry.
  - The command is taken through `scene_unit_command_004e6b30(bag)`, only for creator rows
    with `applies_command_property`.
  - The start speed is taken through `retain_start_speed`.
  - This runs whatever the switch says. It changes nothing on its own.
- **ON, in `run_generate_object_00944fd0`:**
  - Before `create_unit_from_scene_record_0046db4b`, the carry is copied into the spawn's local
    record. `create_units` then sets the row's command and runs the `0082356C` seed behind its
    `runs_ship_base()` gate, exactly as at load.
  - After InitAll, `00874D00` and the deck hand-over,
    `GameUnitsHost::issue_generated_authored_command` issues the command. This is `0046AAB0`
    at `009453BB`. It goes to the first slot at or after the pre-spawn count that carries the
    spawned name.
- **Summary line (always on):** `summary mission generate object hidden carry bound= carried=
  applied= commands= ... unattached_host_only=`.

## Ordering in luaSpawnAndJoinGroup

USN21's `luaSpawnAndJoinGroup` calls `GenerateObject` for every name first, then
`JoinFormation(unit, spawnedTable[1])` for each unit.

- Each spawn's Cruise is therefore resolved inside its own `GenerateObject`, at `009453BB`,
  before any join.
- The followers' joins then issue `follow` (`00720CD0`, from the join `0077F940`), which
  replaces their Cruise.
- The leader's self-join is refused by the same-entity test of `00779820`, as the host models
  it in `src/game_hosts_ai.cpp`. So the leader keeps its Cruise and the group moves.

## Data lifetimes (ON)

- **The pool entry:** process-wide, cleared at scene load. The carry is copied into each call's
  local record. Nothing on the spawn path adds to the pool, so the entry pointer stays valid
  across `create_units`.
- **Repeated name in the scene (review N1):** `add` keeps the first entry, so the carry is
  taken only when `add` inserted. A duplicate's bag never mixes into the first entry's carry.
- **A CommandTarget naming a unit not yet spawned:** `create_units` re-registers every slot
  with the commands host. The lookup therefore sees exactly the entities that exist after this
  spawn, and an unspawned target resolves to nothing, as at `009453BB`.
- **Repeated `GenerateObject` of one record:** `kGenerateObjectEveryCallBound` is ON, so each
  call makes a new slot and the carry applies again. That matches `0046DC10` / `0046D930`
  queueing from the bag on every call. The issue searches from the pre-spawn slot count, so it
  reaches the new slot. A call answered by the `spawned && !respawn` early return never
  reaches the carry.
- **The `!attached` exit (review N2):** this exit is **host-only**; the image has no such exit.
  A carried spawn that ends there keeps its seed, but its command is not issued. It is counted
  as `unattached_host_only`.
- **Before the load-time issue (review N4):** not reachable.
  - `issue_authored_commands` runs inside the `load_scene_contents` step
    (`src/game_hosts_mission_frame.cpp`, both the OFF and ON positions). That is the same step
    that clears and fills the pool.
  - The mission script is loaded later, at `run_mission_script` (`008860B0`), and the stage
    inits run after that. The order is in `src/mission_load_path.cpp`.
  - The earlier Lua steps (global script folders, lobby settings sync) run before the pool is
    filled, so a `GenerateObject` there finds no record.
  - No slot is issued twice, so no guard was added.

## Reach census

This census was made with the lead's `hc_census.py`. It counts Hidden entities that author a
`StartSpeed` or an inner `Command` in this installation's mission scenes: **153 scenes, 3601
entities**.

| class | entities |
|---|---|
| DestroyerGen | 3177 |
| MotherShipGen | 284 |
| PlaneSquadronGen | 77 |
| SubmarineGen | 31 |
| TBoatGen | 23 |
| LandingShipGen | 9 |

Not every one is a `Cruise`. Some examples:

| scene | entity | command | StartSpeed |
|---|---|---|---|
| usn_sibuyan (USN21) | Intrepid (MotherShipGen) | Cruise | 10.0 |
| bsm_09_battle_of_the_coral_sea | D3A Val II (PlaneSquadronGen) | SetTarget -> Yorktown | - |
| bsm_02_defense_of_the_philippines | Kasumi (DestroyerGen) | MoveOnPath -> path_DD | - |
| COTP-IJN/PRCPIJN/ijn_11_invasion_of_fiji | USS Kidd 2 (DestroyerGen) | Follow -> USS Savo Island | - |
| bsm_04_vengance_at_luzon | Lexington (MotherShipGen) | Stop | 3.0 |
| ijn_11_operation_to | NC (DestroyerGen) | - | 10.0 |

ON therefore changes more than cruise speeds. Spawned squadrons get their authored targets,
spawned ships take their authored paths and follows, and a `Stop` with a start speed latches
its own thrust. A pair run should diff the per-unit command table, not only the aggregates.
