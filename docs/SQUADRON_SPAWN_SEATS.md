# Squadron spawn seats: the image spawns the wing stacked (packet cc9_squadron_spawn_seats)

2026-09-25. Ghidra read-only. Names are hypotheses. The brief assumed the image places each member at
its formation seat when the squadron spawns, and that the host stacks them by mistake. It is the other
way round.

## 1. The image's placement

`007F4580` (`BSP_PlaneSquadron_AttachLuaSelfAndSpawnPlanes`) creates each member at
`007F4800`-`007F48D2`. It creates the unit through `[this]->vtable[28h]` at `007F4811`, then places it
through `vtable[98h]`, in one of two arms:
- **With a parent** (`EBP` non-null, `007F481D`-`007F48BA`): an identity 4x4 built on the stack,
  local to the parent.
- **Without one** (`007F48BE`-`007F48D2`): the squadron's own matrix, `squadron+74h`.

Neither arm reads a member index or a seat offset. The spawn tail (`007F4B43`-`007F4C20`) does
three things:
- attaches the member (`007F4B43`: the spawn index and `+3D0h`/`+3CCh`);
- calls `0077FAD0` and a `+170h` virtual;
- assigns the formation indices with `007ED260` (`BSP_PlaneSquadron_AssignFormationIndices`).

None of these moves a member. **Every member of a squadron starts at the same point**, and the
separation is flown by each member's follow state, `009C1FD0` -> `009BFD70` / `009BFEE0` / `009BEE30`
(docs/PLANE_FORMATION.md). The host's own spawn records the same:
`007F4813 has no per-wing offset: the formation spacing belongs to the pilot bot, not to the spawn`
(`src/game_hosts_scene_contents.cpp`).

## 2. The host's stand-in

`kPlaneFormationPlacementEnabled` (`src/game_hosts_units.cpp`, ON) teleports each member onto its
station on its first step. The Zero wingman in `local\ZT_9000.log` sits exactly 25 m lower one second
after spawn with `vy` = 0.
- **Why it exists.** Its comment says it stood in for `009BFEE0` because "nothing in this host
  enters BotStateFollow".
- **That is no longer true.** The torpedo and dogfight wingmen now run their follow states for
  hundreds of ticks each (for example `states[follow=853 aim=265]` for the Kates, and the dogfight
  follow cells in the planner census).
- **So the stand-in is now the divergence.** In the image, members start stacked and fly apart.

## 3. The ~30 s Zero repair

The ~30 s vehicle-avoidance repair on Zero #4.2 was on the **leader**, reacting to its own wingman
about 96 m away. That is the station distance the teleport put it at.
- **The avoidance itself is the image's.** The host's vehicle-avoidance list keeps squadron mates, as
  the image's `007E11D0` refresh does: every kind-6 or kind-0Fh entity within 1080 m.
- **The freeze that followed was the task-less planner gap,** fixed in docs/TASKLESS_PLAN_ARMS.md.
- **The Zero pair has no task and no follow state,** so with the teleport off it stays stacked until
  its avoidance separates it.

## 4. The pair, and predictions written before the runs

The pair is `kPlaneFormationPlacementEnabled` ON (the current main) against OFF (the image's stacked
spawn), one tree, E2 9000, `BSP_GUNNERY_RNG_STREAMS=1`, `BSP_DEATH_TABLE=1`.

| row | ON (teleport) | OFF prediction |
| --- | --- | --- |
| tick-0 pairwise distance, members 0 and 1 | 0.000 m in every squadron (logged before the first step) | 0.000 m in every squadron; the brief's "> 0" row is replaced, since the image spawns stacked |
| pairwise distance at tick 400 (20 s), squadrons whose wingmen follow | station spacing, 60-100 m | nonzero and growing toward station spacing, 20-200 m |
| task-less Zero pairs at tick 400 | station spacing | nonzero after vehicle avoidance separates them; no Zero depth kill |
| Zero and US fighter depth kills | 0 / 0 | 0-1 / 0-1 |
| Kate / Val deaths | 16 / 16 | 12-16 each |
| hit records | 450-750 | 450-750 |
| torpedo / dive-bomb releases | 2-8 / 0-4 | 2-8 / 0-4 |
| Lexington moved | 5.5-7.5 km | 5.5-7.5 km |
| mission end | none | none |

**The risk.** Every squadron now starts stacked. Vehicle avoidance fires between wingmates in the
first seconds, and the bombers' formations take time to open. If the run-in geometry depends on
the teleported stations, the torpedo and dive-bomb rows move. They are judged by band.

## 5. The pair, measured

`local\SS_ON_9000.log` (binary `local\ss_on`, the teleport ON, current main) and `local\SS_OFF_9000.log`
(`local\ss_off`, OFF), one tree, window 1600x900 in both. Pairwise distances come from
`local/seats.py`, members 0 and 1 at ticks 0 and 400 (20 s).

| row | ON | OFF | prediction (OFF) | verdict |
| --- | --- | --- | --- | --- |
| tick-0 pairwise, every squadron | 0.0 | 0.0 | 0.0 | held |
| Kate pairs at 20 s | 71 / 181 / 71 / 181 m | 75 / **499** / 75 / **499** m | 20-200 m | #4.1 and #8.1 **missed** |
| Val and movieval pairs at 20 s | 109-112 m | 113-125 m | 20-200 m | held |
| Lexington sqn01 / Yorktown sqn02 at 20 s | 68 / 168 m | 62 / 186 m | 20-200 m | held |
| **task-less Zero pairs at 20 s (all eight)** | 95-96 m | **0.0 m** | nonzero after avoidance | **missed** |
| **Lexington sqn03 / Yorktown sqn04 at 20 s** | 95.5 m | **0.0 m** | nonzero | **missed** |
| Zero / US fighter depth kills | 0 / 0 | 0 / 0 | 0-1 / 0-1 | held |
| Kate / Val deaths | 16 / 16 | 16 / 16 | 12-16 each | held |
| hit records | 586 | 539 | 450-750 | held |
| torpedo / dive-bomb releases | 6 / 2 | 3 / 2 | 2-8 / 0-4 | held |
| fighter kills | 9 | 10 | - | - |
| Lexington moved | 6616 m | 6691 m | 5.5-7.5 km | held |

**What OFF shows.**
- **Squadrons that follow separate on their own.** Every squadron whose wingmen run a follow state
  opens its formation within 20 s: the Kates, the Vals, the movievals, Lexington sqn01 and Yorktown
  sqn02.
- **Two sets stay exactly stacked.** The eight task-less Zero pairs and two carrier flights
  (Lexington sqn03, Yorktown sqn04) are still 0.0 m apart at 20 s. They fly as one point, and
  identical members with identical controls stay coincident.
- **Vehicle avoidance does not split them.** A candidate at zero offset gives a degenerate band.
  Whether the image's `007DF4F0` produces a direction for a zero offset, or also leaves the pair
  coincident, is not read.
- **The Kate #4.1 and #8.1 wingmen open to 499 m.** That is a follow-law question: the image's
  follow state with a stacked start. It is not measured against anything here.

## 6. Verdict

- **No spawn-seat term is missing.** The image spawns every member at one point, and the host's
  spawn matches it.
- **`kPlaneFormationPlacementEnabled` stays ON.** Turning the teleport off is the image's own spawn,
  but its pair missed two separation rows: the task-less Zero pairs and two carrier flights never
  separate. The run-in rows hold their bands either way.
- **The teleport remains a labelled stand-in.** Its comment is stale on one point: the host now
  enters the follow state.
- **Next reads, before the teleport can go:**
  1. Does `007DF4F0` separate two aircraft at zero offset?
  2. Do the task-less Zeros have a task, or an escort follow state, in the image?
  3. Why do the Lexington sqn03 and Yorktown sqn04 wingmen not follow in their first 20 s?
