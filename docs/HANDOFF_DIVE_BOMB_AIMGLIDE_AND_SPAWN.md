# Handoff: the dive-bomb aimglide, the bomb spawn, and the ditch

Addresses: `009C5180`-`009C580B` (the aimglide tick), `009C5204`-`009C5243` (its two vectors),
`009C56C2`-`009C5755` (the gates), `009C5777` (the salvo), `009C60F1` (the aimdive's release call),
`007BBBA0`, `00A26510`.

Written by packet `cc8_dive_release` (`agent/cc8-dive-bomb`), which closed items 1, 2 and 4 of its
brief in `52ca8f5ea` (on `main` as `505089998`). Nothing below needs the session that produced it.
Read `docs/DIVE_BOMB_TASK.md`'s last section first - "`approach+D8h` is the predicted bomb impact
point, rewritten every tick" - because all of part 1 depends on it.

**What is already done, so you do not redo it.** The aimdive releases. `approach+D8h`/`+DCh`/`+E0h`
is the predicted bomb impact point (the aircraft advanced by its own velocity over `007BCC80`'s
free-fall time), not a latched run-in origin; the aim error's range and bearing are taken from it;
`00CE3880` is a 25-metre CCIP window. Measured on a same-binary pair, per `movieval` bomber: closest
aim error 209.30 m -> 8.06 m, `releases` 0 -> 2.

**Item 3 is CLOSED as a decision, not as work.** `approach+A8h` stays pinned at the low end of the
authored `uniform(350, 450)`, like every other draw in this host, labelled as the convention with
the authored range beside it. Do not bind the draw. Determinism is worth more: it is what let this
packet compare 52-tick aimdives to the digit across runs.

---

## 1. The aimglide salvo can never fire, and it is the same defect the aimdive had

**The body is bigger than this repository has been saying.** `docs/DIVE_BOMB_TASK.md` and
`src/dive_bomb_task.cpp` both name the aimglide release `009C5693`-`009C57A6`. Ghidra's body for
`BSP_BotStateDiveBombAimGlide_Tick` is **`009C5180`-`009C580B`**, and the half before `009C5693` is
where its geometry is built. That is why the release's inputs looked untraceable.

**`009C5204`-`009C5243` builds two vectors from `approach+D8h`**, from the listing:

```
009c5204  mov edi, dword ptr [esi + 4]      ; edi = approach
009c5207  fld  dword ptr [edi + 0xd8]       ; the IMPACT POINT x
009c520f  fsub dword ptr [ebp + 0xfc]       ;   - unit.x          -> [esp+0x48]
009c5222  fld  dword ptr [edi + 0xe0]       ; the IMPACT POINT z
009c5228  fsub dword ptr [ebp + 0x104]      ;   - unit.z          -> [esp+0x50]
009c5232  call edx                          ; approach->vtable[0], the aim point
009c5234  fld  dword ptr [eax]              ; aimPoint.x
009c5236  fsub dword ptr [edi + 0xd8]       ;   - the impact point -> [esp+0x50]
009c5240  fld  dword ptr [eax + 8]          ; aimPoint.z
009c5243  fsub dword ptr [edi + 0xe0]       ;   - the impact point -> [esp+0x58]
```

So the aimglide has, in its own frame, exactly two planar vectors:

* **`impactPoint - unit`** - the aircraft's own forward throw, how far ahead the bomb would land.
* **`aimPoint - impactPoint`** - the miss, the same quantity the aimdive's second `sqrt` measures.

Note the stack reuse: `009C523C` writes `[esp+0x50]` after `009C522E` wrote `[esp+0x50]`, but a
`push` at `009C521B` sits between them, so those are two different frame slots. **Walk this half with
frame bases before you name any slot.** `local\t.ps1` is seeded for `009C58D0` only and will not walk
`009C5180`; extend it or write the equivalent. Do not read `[ESP+n]` literally here - the `SUB ESP`
displacement trap is what cost this stream two retractions already.

**Why the salvo cannot fire today.** `src/game_hosts_units.cpp`'s `dive_bomb_aimglide_inputs` sets

```cpp
in.lateral_a = slot.db_planar_bc;
in.lateral_b = slot.db_planar_bc;
```

and `dive_bomb_aimglide_release_009c5777` uses them twice:

* `009C56C2`-`009C56FE`: `|lateral_a - lateral_b| < 120.0`. With the two equal this is `0 < 120`,
  so **the gate passes trivially** and tests nothing.
* `009C5704`-`009C5755`: `lead = lateral_b - cos(diveAngle) * lateral_a`. With the two equal this is
  `D * (1 - cos(angle))`, which is **always >= 0**, while the pair of gates is satisfiable only when
  `-4*travel - 5.0 < lead < -5.0`. A non-negative lead fails the first of them at every altitude,
  every angle and every travel.

So `009C5777` has never fired in any run of this host, and it cannot, for the same reason the
aimdive could not release before `52ca8f5ea`: one substituted quantity standing in for two different
ones. The fix is the same shape - give each consumer the vector the listing gives it.

**Which of the "four frame slots not traced to their producers" this settles.** That comment in
`dive_bomb_aimglide_inputs` predates two of the four being recovered (`height_above_aim_point` and
`glide_release_ceiling`, both marked RECOVERED in place). The two still standing are exactly
**`lateral_a` and `lateral_b`**, and `009C5204`-`009C5243` is their producer. Settle from the walk
which vector, or which component of which, each of the two release slots reads; the hypothesis this
packet leaves you, unverified, is `lateral_a` = the throw and `lateral_b` = the miss, which makes
`lead = miss - cos(angle) * throw` and lets it go negative as the throw overruns the target - the
only arrangement seen so far that can satisfy `lead < -5.0`.

**Why it matters more than it looks.** The aimglide owns most of the mission. In
`local\usn04_rel_impact.log`, per aircraft: `D3A Val #1.1` spends 630 ticks in aimglide against 49 in
aimdive, `#3.1` 618 against 50. Nine of the twelve dive bombers in USN04 get a dive in; three
release (`movieval`), and the six Vals do not.

**The six Vals are NOT blocked by the aimglide, and you should know that before you start.** Their
aim error reaches 0.48 m (`#1.1`) and 0.37 m (`#3.1`) against the 25 m gate - better than
`movieval`'s 8.06 m - and they still do not release. Those samples are **aimdive** ticks: the census
lives in `dive_bomb_aimdive_inputs`, which `src/dive_bomb_task.cpp:906` calls only when
`ctx.current == kAimDive`, so the `kAimDive || kAimGlide` test inside that function is nearly
vacuous. The gate that stops them is `009C60AF`, `approach+A8h > altitude`: their minima land at
**606.8 m** and **718.3 m** against the pinned 350. Binding the authored draw would not help - 450 is
still far below 606.8. What is worth reading is **why they dive from 1024.3 m** when `movieval`
enters at 650.9 m from an almost identical entry range (478.2 m against 472.6 m), and **which
`PilotBotParameters` row they draw**: `approach+14h` is `00F8A30C + index*248h + 0Ch` and this host
always takes SPNormal, so the difficulty index is unmodelled. That is a separate packet from the
aimglide; do not conflate them.

---

## 2. The bomb spawn: the request is never raised, and there is no row to spawn from

Two separate holes, in order.

**(a) The host never raises the release request.** `009C60F1` calls `007BBBA0`. The host's
`note_dive_bomb_release` (`src/game_hosts_units.cpp`) only counts:

```cpp
void note_dive_bomb_release(GameUnitSlot& slot) {
    ++slot.dive_bomb_releases;
    if (slot.db_release_alt < 0.0f) { ...census only... }
}
```

The torpedo side's `release_ordnance_007bbba0` is the generic path `009C60F1` wants, and it is
already written: the three guards, channel C of the block at `unit+DECh`, then the drop. Calling it
from the dive-bomb release is a one-line change in a dive-bomb hunk. It is correct and it is not
sufficient, because of (b).

**(b) There is no row a bomb can spawn from.** `GameGunneryHost::release_ordnance_drop`
(`src/game_hosts_gunnery.cpp:2959`) selects **the unit's torpedo-capable gun rows** - the same
`swim_speed > 0` test the water crossing uses - and clears the kind `2Bh` bit on a drop. A dive
bomber carries kind `2Ah` (general bomb; `009C7AFE` is `BSP_WeaponController_HasGeneralBombOrdnance`,
`007B9320`, kind `2Ah` excluding `2Ch`/`31h`/`2Bh`/`33h`/`2Dh`). So the selection finds nothing, the
call returns false, and `bombs_spawned` stays 0 however many requests are raised. That is exactly
what `local\usn04_rel_impact.log` shows: `releases=2 bombs_spawned=0`.

Note `bombs_spawned` is printed from `slot->torpedo_drops_spawned` - the name in the log line is not
the name of the field. Check what a summary column actually reads before you believe it.

**What is known about a bomb's row and its fall, so you do not start from nothing.**

* A bomb platform **is a `Gun` subclass**: `00730B80` calls `BSP_Gun_Construct` at `00730B88`. So the
  round should be handed to the gun spawn `0072F830` the gunnery host already implements, the same
  way a torpedo is - the selection predicate is what needs to change, not the spawn.
* The fall model is already evidence, from `007BCC80` read whole in `52ca8f5ea`: gravity is the
  qword `9.81` at `00CF9058`, and the vertical velocity carries a `-3.0f` bias from `00E08E54`
  (in `.data`, its writer untraced). The time of flight is `(sqrt(vy^2 + 2*g*h) + vy) / g` with `vy`
  upward-positive. A bomb released at the aimdive's measured point - alt 278.9 m, 66.7 m/s - should
  therefore fall for about that solution's `t` and land near the aim point, which is the check that
  tells you the spawn is right rather than merely non-zero. `approach+D8h` at the release tick IS
  the predicted impact point, so the host can print predicted-versus-actual directly.
* `007BBBA0` itself is a **bay-open command, not a spawn** (`docs/TORPEDO_RELEASE_SPAWN.md`); nothing
  in the block or its tick `007DE3A0` makes a projectile. Do not expect raising it to produce a bomb.

**Ownership.** `src/game_hosts_gunnery.cpp` is held by `cc8-torpedo-swim`. Work there by hunk
arbitration with the integrator; do not take a lease that collides. The dive-bomb call site in
`src/game_hosts_units.cpp` is a dive-bomb hunk and is free under the same arbitration.

---

## 3. The ditch is the planner hand-over gap, reached for the first time

In `local\usn04_rel_impact.log`, for each of the three `movieval` bombers:

* `arm_ticks=2112`, `states[done=303 aimdive=53 flyabove=158 turndown=71 attackrun=1527]`. The task
  reaches `done` at arm tick **1809** (2112 - 303) and the arm keeps ticking in `done` for 303 ticks.
* `plane water contact: unit=movieval alt=-0.00 water=0.00 |v|=68.72 state 7 -> 6` at **fixed step
  4284** (native frame 4322). The arm stops there because the aircraft has ditched, which is why
  `arm_ticks` is 2112 here against 2370 in the before run - the before run's task never finished and
  simply ran to the end of the mission.
* `host AiPlanners::planner_tick [00a26510] UNIMPLEMENTED, returning a neutral value` is in the same
  log. Nothing assigns a follow-up task, so the aircraft flies unsteered from `done` into the sea.

**There is no goaway attitude to report, because goaway never ran.** All three `movieval` bombers
show no `goaway` entry in their state list. That is not a vacuous negative: the printer emits the
bucket when it is non-zero, and `D3A Val #3.1` in the same log shows `goaway=1`. The last attitude
the dive-bomb task commanded is therefore the aimdive steer's, from the census line:
`pitch=-0.623 roll=0.308 bearing=-0.1230 rad`, a nose-down command with a small right roll - which is
consistent with an aircraft left descending.

So this is **not a regression in the dive**. It is the hand-over gap that
`docs/DIVE_BOMB_TASK.md` already records, now reachable because the task can finally complete. The
torpedo side has the same gap and its own handoff, `docs/HANDOFF_TORPEDO_DONE_STATE.md`; the
integrator is staffing a done-state packet covering both. Do not try to fix it inside the dive-bomb
task, and do not read the ditch as evidence against the release binding.

---

## 4. Runs, rules, and the logs this packet left

Run ONLY through the launcher; never start `bsp_game.exe` directly (an untagged run takes the game's
own mutex and blocks the user's copy):

```
./tools/run_game.ps1 -Log local\<unique>.log -WaitSeconds 2400 -- --frames 5000 \
    --press-start-frame 30 --menu-select USN04 --mission-frames 4800 --mission-frame-seconds 0.05
```

* Every run needs its own `-Log`. **Never** pipe the launcher through `Select-Object -First N` or any
  short-circuiting filter: it kills the run silently, exit 0, no log. Use `-Last 5` or a redirect.
* A 4800-frame USN04 run takes 10 to 15 minutes, outlasts the 600 s foreground cap and is
  backgrounded. Nothing wakes you. Note the expected end, do read-only work, then read the log.
  Never re-launch. A run holds the tree's `bsp_game.exe`, so you cannot link while one is in flight.
* Judge a run by its log, not its exit code. If a run dies with
  `_FMOD_EventSystem_Init@20 result=61`, run `query session`: a disconnected remote-desktop session
  means no run can start at all.
* 4800 frames is the right length. At 3000, `movieval` never leaves `attackrun` (it needs about 1527
  ticks there) and the late-spawning Vals never dive - that is the mission, not a defect.
* USN04 carries **twelve** dive bombers since `SpawnNew` works: three `movieval`, and `D3A Val`
  `#1.1`, `#3.1`, `#5.1`, `#7.1`, three each. `#5.1` and `#7.1` spawn late and reach only
  `flyabove`/`attackrun` within 4800 frames. Read all twelve, not just `movieval`; the Vals are a
  second, different test of the chain and they are what proved the altitude gate.

Logs left in this tree (git-ignored, both to clean shutdown):

| log | binary | what it is |
| --- | --- | --- |
| `local\usn04_rel_before.log` | instrumentation only | the baseline: `releases=0`, aim error 209.30 m |
| `local\usn04_rel_impact.log` | + the impact-point binding | `releases=2`, aim error 8.06 m, and the Vals |

A before/after is valid only on the same binary apart from the change under test. That pair is; do
not reuse either against a binary that has moved.

Also in the tree: `local\t.ps1 <from> <to>` (the scripted CFG walk that prints `base [ESP+nn]` beside
every ESP-relative operand, seeded for `009C58D0`), `local\x87trace.py` and `local\x87_walk.py` (two
independent x87-depth walks).
