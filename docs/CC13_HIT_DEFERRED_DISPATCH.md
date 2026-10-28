# CC13 hit deferred dispatch (kHitDeferredDispatchCc13Bound)

Addresses: 00926E80, 00926700, 00926750, 009239A0, 00470350, 00819BA7, 0084BBF9, 0084BE20,
00875E44, 00875EC4, 00874DB9, 00F899C0

Packet cc13-hitq (cc9 lead approval 2026-10-19), from the rev-convoy review of
`docs/CC13_FRONTIER_MERGE.md`. Switch: `bsp::kHitDeferredDispatchCc13Bound` in
`include/bsp/cc13_hitq_switches.hpp`, **OFF**. Status: reconstructed and build-tested; not
game-validated. Every descriptive name is a hypothesis, not a recovered symbol.

## What the image does

Evidence is the existing analysis in `docs/ENTITY_EVENT_QUEUES.md` and
`docs/PROJECTILE_IMPACT.md`, re-read for this packet against the listing.

| step | address | what |
| --- | --- | --- |
| record | `00470350` SetShot, before the push | `record+14h = shot->vtable[54h]()` = `006E7C60`, a draw through `00BD2F10` (callers `0084BF00`, `0084BAD0`, `00819A20`, `0080A000`, `00904470`) |
| push | `00926E80`, `__fastcall(ECX = record, EDX = direction)`, `RET` | under the `00924990` lock: `operator new(0x68)`, link at `head[+4h]` (`00926F12`), size `[00F899C8] += 1`, copy the record into `node+8h` with `00925050` (part array included), direction to `node+5Ch..+64h` (`00926F48`); `00E18684 = 1` (write-only); `shot->vtable[5Ch](29h)` at `00926F65` with the result discarded |
| callers | `0084BE20` | the impact `0084BC60`, step 6: the direct hit, then the blast at step 7 |
| | `0084BBF9` | the blast `0084BAD0`, one push per gathered entity (shot 0 for a kamikaze or a crash blast) |
| | `00819BA7` | the kamikaze direct arm of `00819A20` (SetShot at `00819B9C`, `+24h = unit+518h`, `+28h = unit+514h`) |
| drain | `00926700`, `__cdecl void(void)`, no lock | while `[00F899C8] != 0`: `node = head[+4h]` (`00926714`, the newest), `subject = record+0h`; when `subject != 0 && !subject[5Eh] && !subject[5Fh]` (`0092673F`, `00926745`) call `009239A0(ECX = subject, node+8h, node+5Ch)` at `00926750`; then re-read `head[+4h]` and pop it (`0092675B`..`009267A6`); `00E18684 = 0` |
| dispatch | `009239A0`, `__thiscall(entity, record, direction*)` | `+5Eh`/`+5Fh` again; the scoring hook `vtable[24h]`; `while (!entity->vtable[ECh](record)) entity = entity[+3Ch]` (`007BBCF0` OnHit -> `008777D0` damage and death); `0077CE60` attribution |
| drain sites | `00875E44` (row 6), `00875EC4` (row 14), `00874DB9` (`00874D00` with `CL = 0`) | the only callers of `00926700` |

Order is **LIFO**: push and pop both use `head[+4h]`. The fixed step `00875BB0` runs the job
waves (`00875CC0`) and then the fan-out (`00875E0C`), so a hit made in a wave is dispatched at
row 6 of the **same** step, after the rest of the waves and fan-out rows 1 to 5; a hit made after
row 14 waits for row 6 of the next step.

**SUBSTITUTION, the host's step order (correction after review rev3).** The host does not run
the gunnery pass inside the job waves before the fan-out. `InMissionTickBinding::run_step_subsystems`
(`src/game_hosts_mission_frame.cpp`) calls `run_subsystems_00875e0c` (the fan-out, rows 1 to 16)
**first** and `motion_step_00825f20` (the units' step, the gunnery pass inside it) **after** it.
Every ON push is therefore made after this step's rows 6 and 14 and drains at **row 6 of the next
fixed step**: one step (0.05 s) later than the image's same-step row 6. The hits pushed on the
mission's last step are never dispatched; the summary's `left=` counts them. No drain point was
added for this (lead decision 2026-10-19); the delay is labelled here instead. Packet cc13-hitq2
adds that drain point under its own sub-switch; see the next section. With the sub-switch OFF,
everything in this paragraph and in the sections below still holds.

## The same-step drain (kHitDeferredSameStepDrainCc13Bound, packet cc13-hitq2)

Requested by cc9 on 2026-10-19. The sub-switch `bsp::kHitDeferredSameStepDrainCc13Bound` is in
`include/bsp/cc13_hitq_switches.hpp` and is **OFF**. A `static_assert` requires
`kHitDeferredDispatchCc13Bound` whenever it is ON. Status: build-tested, not game-validated.

**What runs between a push and the image's row 6.** Per fixed step, `00875BB0` runs the job waves
inline (the group walk from `00875CAA`, the host's `run_job_waves_00875cc0`), then the fan-out
rows inline from `00875E0C` (row 1 is the physics-world call there). The gunnery pass and
the wave-2 trigger walk (`00875B90` -> `008759B0`) are wave sub-nodes. So after a push the image
runs:
- the rest of the waves;
- fan-out rows 1 to 5: `00C5C540` physics world, `004462D0` buoyancy, `0098BDB0` spatial refresh,
  `00874DE0` callbacks;
- row 6, the drain.

Rows 7 to 16 follow in the same step: the Lua calls, the think list, the session pump, InitAll,
row 14 (the second drain, empty of wave pushes), `009273A0` and `00903610`.

**The host's step** (`in_mission_subsystem_tick.cpp` driver):
1. `run_step_job_waves` (the job waves, land convoys, deck tick);
2. `run_step_subsystems`, which runs, in order:
   - the fan-out rows 1 to 16;
   - the Lua target-gone drain;
   - `motion_step_00825f20` (ship AI, the AI pass, `GameGunneryHost::fixed_step` with every push,
     the per-slot motion loop);
   - the proximity/squad-ordnance trigger walk;
   - read-only diagnostics.

**Candidate (a), built.** With the sub-switch ON, `run_step_subsystems` calls
`GameStepSubsystemsHost::drain_deferred_entity_events_00926700` once more. The call goes right
after the trigger walk and before the diagnostics: the first point after every push of the step,
as the image's row 6 is. It is the same drain and binding the rows use, and only the call site is
new. The fan-out's own row 6 and row 14 calls stay as they are (they now usually find the queue
empty), and so does `00874DB9`. Candidate (b), moving the fan-out after the motion step, was not
built: it changes the OFF order and needs cc9's sign-off.

**Order (a) gives, versus the image.**

| | image | host with (a) |
| --- | --- | --- |
| push | wave 2 of step N | `motion_step` of step N |
| between push and drain | rest of the waves, then rows 1 to 5 of step N | rest of the motion step, the trigger walk |
| drain | row 6 of step N | the new call, end of step N |
| after the drain, same step | rows 7 to 16 of step N | the read-only diagnostics |
| next | step N+1 waves | step N+1 waves, then rows 1 to 16 |

Residue, labelled:
- **Rows 1 to 5.** They ran before the pushes in step N, not between the pushes and the drain.
  That is the host's existing fan-out-before-motion order, unchanged here.
- **Rows 7 to 16 after a death.** A death at the drain reaches them in step N+1, not step N. That
  covers the destroy-list delivery and `kSunkShipFlushBound` at row 15, the expiry counter at row
  16, InitAll and the Lua target-gone drain. The image runs them later in the same step.
- **Pushes from the fan-out itself** (rows 7 to 13; none are known in this host) would still drain
  at row 14 as before.
- `left=` is 0 at the end of a run, except for pushes made after the last step's motion step.

**RNG (00BD2F10).** The drained handlers' draws (`Draw::hit_effect`, `Draw::ship_breakup`,
`Draw::ship_break_in_two`) land at the end of step N:
- after the rest of the gunnery pass, the per-slot motion loop and the trigger walk of step N;
- before step N+1's waves and its rows 1 to 5.

That is where the image puts them relative to step N+1. Draws in step N's rows 7 to 13 now come
before the drained draws instead of after them. In this host those rows draw only through the Lua
calls, the think list and the session pump. With the sub-switch OFF the drained draws come after
step N+1's rows 1 to 5, as the section below says.

**Expected movers, sub-switch ON (against the hitq ON baseline):**
- death and damage land one step earlier, in the same step as the hit;
- `left=` drops to 0 or near it;
- `GameStepSubsystemsSummary::deferred_event_passes` gains one per step;
- RNG-coupled differences from the first hit onward.

Against OFF, the hitq movers below hold with "next step" read as "end of the same step":
- in-step readers after the pushes (the rest of the gunnery pass and the per-slot motion loop)
  still see pre-hit health;
- kill attribution and draw order are still LIFO.

## What the host did (OFF, unchanged)

`GameGunneryHost::Impl::apply_hit` (the `0084BE20` direct hit, called from `run_projectiles`, and
each `0084BBF9` blast record, called from `apply_impact_blast`) and
`GameGunneryHost::Impl::apply_gunless_blast_hit` (the shot-0 `0084BBF9` records of
`kamikaze_detonate_00819a20` and `plane_crash_impact_0084bc60`) dispatch each hit where it
happens, in arrival order, inside the gunnery pass of wave 2. The existing drain
`bsp::drain_deferred_entity_events_00926700` already ran at all three drain points through
`GameFixedStepHost` -> `GameStepSubsystemsHost` -> `DeferredEventBinding`, over a queue that was
always empty. That drain and those three sites are reused; no second drain was written.

## What ON does

| image | host |
| --- | --- |
| `00470350` draw, then `00926E80` | `apply_hit` with `deferred_replay` null: the wreck/index gate, the `Draw::hull_damage` draw with the same arguments the OFF body uses, then `queue_deferred_hit`; `apply_gunless_blast_hit`: its dead gate, then `queue_deferred_hit` (shot 0, no draw) |
| node `+8h` record copy, `+5Ch` direction | `DeferredHit`: the record and a copy of its part array, the direction, the drawn `record+14h`, and the funnel's own state (`round_bullet_class`, `impact_shape_kind`, `impact_hull_segment`, team id) |
| `head[+4h]` | `std::deque::back()`; the deque is created on the first push |
| `00926700` loop and gate | unchanged `bsp::drain_deferred_entity_events_00926700`; `DeferredEventBinding` answers the size and `back()` from the gunnery host, the subject from `GameUnitsHost::unit_identity(victim)` and its `+5Eh`/`+5Fh` from `unit_scene_node_flags` |
| `00926750` -> `009239A0` | `GameGunneryHost::dispatch_deferred_hit_009239a0` -> `Impl::dispatch_deferred_hit`: restore the funnel state, set `deferred_replay`, call the same funnel; its body is the dispatch, with the drawn value in place of the draw |
| `0092675B` pop of the re-read `head[+4h]` | `pop_deferred_hit_back`, the current back |

A hit raised from inside a dispatch is queued, not run: the funnel takes `deferred_replay` and
clears it on entry. With the drain's two `back()` calls kept, that reproduces the image's
re-entrancy hazard (the new node is popped undispatched and the dispatched node runs again). The
existing analysis found no `vtable[ECh]` path to `00926E80`, and no hit funnel is called from the
host's death path, so the hazard is not expected to fire.

ON adds one summary line, `summary mission gunnery deferred hits queued= dispatched=
refused_at_queue= left=`. `left=` is the queue at the report: the last step's pushes, which no
later row 6 drains (the step-order substitution above), so it is expected to be non-zero in any
run with hits on its final step.

## OFF identity

* `kHitDeferredDispatchCc13Bound = false`. Every new statement on a hit path is inside
  `if constexpr (kHitDeferredDispatchCc13Bound)`, or replaces a statement with an
  `if constexpr (!kHitDeferredDispatchCc13Bound)` / `else` arm holding the same statement
  (`++summary.queued_hits`, `done("Projectile::queue_hit_00926e80")`, the `Draw::hull_damage`
  draw).
* `apply_hit` gains `const DeferredHit* replay = nullptr;` and a `static_cast<void>`; OFF it is
  never read.
* The binding: OFF both pointers are null, `deferred_queue_size()` returns 0 as before, so the
  drain makes the same calls (`set_deferred_pending_flag_00e18684(false)` alone) and the same
  log entries. `drain_deferred_entity_events_00926700` makes no new call OFF: the
  `units_->gunnery()` lookup is inside the `if constexpr`.
* New members of `Impl`: a null `std::unique_ptr<std::deque<...>>`, a null pointer and three
  zero counters. Nothing allocates OFF. The summary line is inside `if constexpr`.

## ON data lifetime

| container | filled by | read at | cleared, moved or destroyed before the drain? |
| --- | --- | --- | --- |
| the queue (`Impl::deferred_hits`) | the three funnels, inside `GameGunneryHost::fixed_step`, which the host runs in `motion_step_00825f20` after the step's fan-out | the next step's row 6 (then row 14), or `00874DB9` | nothing clears it: it lives in the gunnery host and is destroyed with it, and the image never clears `00F899C0` either (references: push, drain and the static init/atexit pair `00CD3970` / `00CDF4D0`). But because of the step-order substitution, a push waits through the rest of its step and the next step's rows 1 to 5, and when `00874D00` runs first (GenerateObject from Lua) its InitAll at `00874D79` (`run_sentity_init_all_00925f20`) and its session rows run **before** its own drain at `00874DB9`. Nothing between push and drain touches the queue; what can change is the victim and shooter state the dispatch reads (rows below). The last step's pushes are never drained (`left=`) |
| the victim (`unit_state[victim]`, `unit_identity`) | the unit host | dispatch | indexes are never freed; the funnel bounds check runs again at the dispatch. In the image the subject is a raw pointer freed only on the second `00903610` pass after `00922FD0` marks it (`docs/ENTITY_EVENT_QUEUES.md`, the grace period), so it outlives any push-to-drain interval |
| a victim killed between push and drain | an earlier node of the same drain (LIFO), or an explicit Kill | dispatch | the image refuses only `+5Eh`/`+5Fh` (`0092673F`, `009239A6`), set by the kill pass `009274CE` and `00922FD0`; a damage death (`0077D1A0` -> destroy list -> `00926390`) sets `+5Dh`/`+60h` only, so the image still dispatches. The host keeps its own gate in the funnel body: `apply_hit` refuses a dead unit outside the wreck window (`kWreckHitDeliveryBound`), `apply_gunless_blast_hit` refuses any dead unit, both as OFF |
| the shooter (`unit_state[shooter]`, attribution) | the unit host | dispatch | stable index. The image's shot was killed at `0084BE00` before the push and is read by `009239A0`; it stays allocated through the same grace period |
| the part array | `apply_impact_blast`'s locals `element_entries` / `entry`, the kamikaze and crash blast locals | dispatch | copied into the node at the push, as `00925050` copies it; the dispatch points the record at the copy |
| the funnel state (`round_bullet_class`, `impact_shape_kind`, `impact_hull_segment`) | `run_projectiles` around the call, reset after it | dispatch | stored in the node and restored for the dispatch, then put back |

SUBSTITUTION: a unit whose scene flags the unit host does not model (`scene_flags_available`
false) reads `+5Eh`/`+5Fh` clear, so `0092673F` dispatches and the funnel's gate decides.

## Shared RNG `00BD2F10`

The drained handlers draw. `ShipHitBinding::random_unit_float` (`Draw::hit_effect`, the fire and
leak chances of `00826F10`) and the death path's `ship_death_message_00827ab0`
(`Draw::ship_breakup` at `00827B4D`, `Draw::ship_break_in_two`) move from the gunnery pass to the
**next** step's row 6: after every later draw of their own step (the rest of the pass, other
units' aim, reload and gunner draws, the rest of the motion step) and after the next step's rows 1
to 5 too, and they run newest hit first. The image would put them at the same step's row 6; the
extra step is the step-order substitution. The `Draw::hull_damage` draw stays at the hit, in the OFF
position. Any pair diff on a run with a hit is therefore RNG-coupled from the first hit onward.

## Expected movers when ON

* **Death timing: one step later.** A unit killed by gunfire dies at row 6 of the **next** fixed
  step (SUBSTITUTION, the host's fan-out-before-motion order; the image's row 6 is in the same
  step). Readers in between see it alive at its pre-hit health: the rest of `run_projectiles`
  (later rounds of the same step reach it as a live target, not a wreck),
  `run_plane_crash_sweeps_007c0910`, `run_kamikaze_contacts`, `run_damage_control`, the inferior
  failure timers, `publish_ai_weapon_facts`, the per-slot motion loop after the gunnery pass, the
  Lua proximity triggers after the motion step, the frame work between steps, and the next step's
  fan-out rows 1 to 5. A hit on the mission's last step never lands.
* **Kill attribution with several hits in one step**: dispatched newest first, so the hit that
  takes health to zero, `last_attacker`, `lua_kill_attacker` / `lua_kill_slot` and the
  `0077CE60` block end on the oldest hit instead of the newest. A direct hit and its own blast
  (`0084BE20` then `0084BBF9`) swap: the blast records land first, the direct hit last.
* **In-frame hp/alive readers inside the funnels' callers**: `apply_impact_blast`'s
  `be_damage_element` and its per-record `took=` / `health=` log,
  `kamikaze_detonate_00819a20`'s `kz_blast_damage`, and `plane_crash_impact_0084bc60`'s `dealt`,
  `pc_blast_damage` and the `plane crash impact: ... dealt= victim_health=` log read health right
  after the call and see no change ON (diagnostics only).
* **Fires and leaks** start at the next step's row 6, so `run_damage_control` first sees them in the
  next gunnery pass.
* **Lua hit events** (`hit_events`) and the death table are filled at the next step's row 6, in
  LIFO order.
* **Draw order**, above.
* Counters: `wreck_hits_reaching` and its two siblings count at the dispatch; a hit the queue
  gate refuses counts in `refused_at_queue` instead.

Pairs: gunnery53's successor runs USN04, JM08, USN21 and BSM09, both envs. cc13 made no runs.

## Uncertainty and residuals

* `plane_spin_side_800` (`007BBCF0`, `unit+800h`) is still written by `run_projectiles` before
  the funnel, at the hit; the image writes it inside OnHit at the dispatch, so with several hits
  on one plane between drains the image keeps the oldest hit's kind and the host the newest.
  The write site in `run_projectiles` carries an ON note saying so; it was not moved.
  `notify_plane_hit_007bbcf0` is in the funnel body and does move to the dispatch.
* **`0084BBCF` is untraced.** In `0084BAD0` the blast skips the push when the gathered record's
  entity equals `ECX` (`0084BBCF CMP [ESI],ECX` / `JE 0084BBFE`), the source-entity exclusion.
  Where `ECX` is loaded and what it holds per caller was not traced in this packet; the host's
  blast gathers keep their own exclusions, unchanged by this switch.
* The queue gate in `apply_hit` (dead and outside the wreck window: not queued, not drawn) stands
  for the narrowphase's hit-index membership, as the OFF gate does; it is the host's model, not
  a read of the index.
* The scoring hook `vtable[24h]` of `009239A0` is not in the host OFF or ON.
* **Adoption note (not changed here)**: the kamikaze direct arm's push at `00819BA7` is the
  delivery of its damage record. The host's `kamikaze_detonate_00819a20` counts the arm and
  records `00915F20`, which is `BSP_MissionScoring_RecordHitDamage` (`00819BBE`, after the push),
  and never delivers the record, OFF or ON. Taken up separately as packet cc13-kzdirect
  (`kKamikazeDirectHitCc13Bound`, branch `agent/cc13-kzdirect`, `docs/CC13_KAMIKAZE_DIRECT_HIT.md`),
  which delivers through the immediate funnel; routing it through this queue is its follow-up.
* The host has no lock (one thread); the image's `00924990` section is not modelled.

## Unobservable gaps left open by the gun packets B, C1 and C2 (cc13-bflush, 2026-10-28)

Read-only audit (cc13-cgbuildA, packet cc13-bflush). These are two gaps between the drain and the flush that
the gun packets B (`kGunDestroyedFailureCc13Bound`), C1 (`kGunDeviceHitDamageCc13Bound`) and C2
(`kGunDeviceBlastDamageCc13Bound`) leave open on purpose. Both are labelled in
`src/game_hosts_gunnery.cpp`. Neither has a switch; the names below are reserved for the
sketches. No runs were made; the exposure counts come from existing u12 logs.

### kGunDestroyAtFlushCc13Bound (sketch, not built): the gun is destroyed at the flush

- **Image.** The killing hit is applied in drain #1 (`00926700` at `00875E44`). The chain is
  `00728080` -> `0043F7D0` -> vtable[70h] `0077D1A0` -> `00926C80`, which sets `+60h` and queues
  the gun on `00F899A8`. The gun is destroyed only at FlushPending (`009273A0` at `00875EC9`):
  `00926390` sets `+5Dh`, then `00729FA0` -> `0072B5A0` sets `+3B8h` and re-runs `00956C20`.
  Between the two, the image runs the queued Lua calls (`00888230`, `00875E55`), the think
  list (`00929460`, `00875E64`), the session pump, creates and InitAll, and drain #2
  (`00875EC4`).
- **Host.** `gun_destroyed_failure_0072b5a0` sets `+5Dh`, `+3B8h` and the level at once, inside
  the drain.
- **Why it is unobservable today.**
  - More damage in the window does nothing. Health is already 0, and `00879070` refuses a
    positive amount on a dead entity (`008790C1`); `00877B90` clamps a negative request to 0.
    A second `0043F7D0` returns at `00926CBF` because `+60h` is already set. The host refuses
    the same damage at `+3B8h` instead. Health is the same; only the refused counter differs.
  - The gun can't be revived in the window: `0072A0D0` needs full health.
  - Shells already in flight don't depend on their gun's flags.
  - No host code reads the gun's flags in the window. `run_due_entity_think_00929460` runs an
    empty field list (`game_hosts_ready.cpp` ~:597). Ship AI and gun fire run outside the
    window. No Lua native reads gun `+5Dh` or `+3B8h`.
- **What would make it observable.** Hosting ship-AI brain thinks in `00929460`, or a Lua native
  that reads device state. One AI evaluation in the step a gun dies would then still count
  that gun in the image (category lists, `0095EC8B` `device_is_destroyed`) and not in the host.
- **Fix sketch.** Under the switch, `gun_destroyed_failure_0072b5a0` sets only `destroyed_60`
  and queues the gun. `GameStepSubsystemsHost::flush_pending_entity_queues_009273a0`
  (`game_hosts_ready.cpp` ~:619) calls a new gunnery hook that applies `+5Dh`, `+3B8h`, the level
  and `00956C20`. The OFF arm stays verbatim. Expected movers today: none.

### kDeviceHitAttributionCc13Bound (sketch, not built): device records reach `0077CE60`

- **Image.** When a handler's vtable[ECh] returns AL = 1, `009239A0` walks up the `[+3Ch]`
  parents (`00923A93..00923AC6`) to a plane, or to a node whose parent is kind 44h. If the top
  node is kind 2, it calls `0077CE60(top, record)` (`00923ADA`). For a device record (handlers
  `0043F7F0` and `008777D0`), the top node is the parent ship. `0077CE60` then:
  - gates on the ship's `+5Ch`, `+5Dh`, `+60h` and `+5Eh`;
  - computes the damage with `00470740` and armour 0, because the record's `+34h` is -1;
  - if the damage is above 0.0, writes the last-hit clock `+2E4h` and the attribution block
    (`+2D4h`, the `+2C4h` attacker via `0077CC80`, the sole-attacker tri-state and the slots);
  - always calls `00915F20` (MissionScoring_RecordHitDamage) and `00988510` (the "hit" warning
    channel, which also feeds Lua `AddListener("hit", ...)`);
  - calls `0090EC40` only when a plane shoots a non-plane.
- **Host.** Only the ship path attributes (`kill_attribution_0077ce60`, `game_hosts_gunnery.cpp`
  ~:13560-13603). Two device paths skip it: C1's non-relay early return (~:13252) and every C2
  mount record (`device_blast_dispatch_009239a0`).
- **Why it is unobservable today.**
  - C1 non-relay never ran in the post-flip u12 logs: every log shows device mesh
    `dispatched == relays`.
  - C2: 8708 mount records in `sw2br_u12_sw2e_on` and `sw2br_u12_sw2e_off`, and 0 of them lack
    their unit's own blast record in the same burst. That own record has the same shooter and
    clock, so:
    - the attribution writes the same attacker, and the sole-attacker state is unchanged;
    - the last-hit clock is the same step;
    - the Lua hit-channel event is throttled per (victim, shooter) by `00988949..00988969`
      (clock + 2.0, or + 1e-4 for the quick kinds), and on the host by `kLuaHitRateLimitBound`
      (`game_hosts_lua.cpp` ~:9715).
  - `00915F20` scoring is not modelled on the host ship path either, so that gap is not
    specific to devices.
- **What would make it observable.** A device record with no own-unit record in the same step.
  Examples: a C1 direct hit that doesn't relay; a blast that reaches a mount but not its hull
  (a gather change); or a new rate-limit window.
- **Fix sketch.** Move the ship path's attribution block into one helper keyed by (parent unit,
  shooter, record). Call it after `device_blast_dispatch_009239a0`, after the gunless dispatch,
  and at C1's non-relay return. Gate the attribution part on `00470740(record, 0) > 0`; post the
  hit event unconditionally. The OFF arm stays verbatim. Expected movers on shipped data: none.
  This is worth doing together with `00915F20` scoring.
