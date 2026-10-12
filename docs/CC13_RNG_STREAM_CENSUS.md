# Random-stream census: every draw and seed of the thread streams (packet cc13-rng1)

Worker cc13-rng1, 2026-10-12, base origin/main fa36636cd. Read-only analysis for cc9. No C++, no
Ghidra writes, no ledger records. Everything here is **exported and listing-read** (disk bytes, the
index and the live Ghidra function table); nothing is reconstructed or game-validated.

Addresses: `00BD2F10`, `00BD2F40`, `00BD2F90`, `00BD2FC0`, `00BD2FD0`, `00BD2ED0`, `00BD2FE0`,
`00BD3050`, `00BD2E00`, `00BD2E20`, `00BF0CF0`, `00BF0DF0`, `00CD8A10`, `00CD8A30`, `00BE2EA0`,
`00BE3150`, `00875BB0`, `006E7B00`, `006E71B0`, `0099C6F0`, `0099C8B0`, `0099EF3A`, `009A188C`,
`005CD1A0`, `00798C80`, `0079D020`, `0079A260`.

Data: `reports/cc13_rng_stream_census.json`, one row per site (759): site, entry, stream, ECX
evidence, function, class with confidence and reason, and host status with the generator. Scripts:
`local/rn1_census.py` (sites and ECX), `local/rn1_host.py` (host grep), `local/rn1_class.py` (hand
classification), `local/rn1_build.py` (report). `tools/verify_report_calls.py` checks 759 call rows;
20 fail, all on code Ghidra has no function for (section 1.3), none on a wrong callee.

## 0. Short answer

- **759 draw/seed sites take a stream id in ECX**, all resolved: 333 on stream 0, 426 on stream 1.
  Every site passes 0 or 1; three register-derived ones were settled by reading their guard.
  This supersedes `docs/RANDOM_STREAMS.md` section 1's 214 / 327 / 84-unresolved count for `00BD2F10`
  (now 226 / 399 / 0) and adds the four other entries.
- **Stream 0 is 313 presentation sites, 16 gameplay, 1 neutral, 3 seed.** The gameplay ones are:
  the bot-task base's avoid-gunfire timer, the level-bomb periods, the plane death timers, the land-fort
  fire rolls, two bomb-drop offsets and the ship-AI navigation block. The host models 8 of the 16:
  6 on its own stream-0 generator and 2 on the units generator.
- **Every shell or bullet construction draws stream 0.** `006E7B00` stores U(0, 0.1) at
  `projectile+27Ch`, and its only reader is the bullet serializer. So gunfire volume moves stream 0
  in the image as much as particles do. The host does not take this draw.
- **Stream 1 also carries presentation: 50 sites.** They are movie camera shake (11), unit
  break-up and wreck debris (23), damage smoke (3), the HUD plane effects, rain, clouds, lightning and
  the movie camera's per-step draw. The host feeds two of these into its gunnery generator: damage
  smoke and the movie step draw.
- **Two facts bound how far any same-sequence model can go:**
  1. Each registered thread has its own pair of streams. The fixed-step entity waves run on
     frame-job workers by default (`00E0B6CE = 1`), so a tick's draws land on whichever thread
     claims the job.
  2. The movie camera's destructor reseeds the main thread's stream 1 from the clock.

  The image's draw sequence is therefore not reproducible in the image itself. The host's
  separation of generators is the right shape, and the remaining work is keeping presentation off
  them.

## 1. Method

### 1.1 Entry points (read from the listing)

| entry | ECX | what it does |
| --- | --- | --- |
| `00BD2F10` | stream | `00BD2ED0` then `00BD2E60`: U(lo, hi) |
| `00BD2F40` | stream | `00BD2ED0`, guard bytes, `00BA2C20`: one word as an unsigned fraction in [0, 1) (`00D63B80` = 2^-32) |
| `00BD2F90` | stream | `00BD2ED0` then `00BF0DF0(a, b)`. `00BF0DF0` is a rejection loop: it draws two words per pass, branches back to `00BF0E20` while s >= 1 (`00BF0F50 FCOMI`, `00BF0F52 JAE`), then calls `00BFA910` (log) and `00BF7030`. This is a polar-method normal, and it takes **a variable number of words per call** |
| `00BD2FC0` | stream | `00BD2ED0`, then a tail JMP to `00BA2C20`: a raw word (callers take `% count`) |
| `00BD2FD0` | stream | seed: `00BD2ED0`, then `00BF0CF0(EDX)` |
| `00BD2ED0` | stream | state lookup; no direct caller outside the five above |

Positive control: the `00BD2F10` count, 625, equals `docs/RANDOM_STREAMS.md`'s.

### 1.2 Sites and stream ids

- **Sites.** A rel32 `CALL`/`JMP` scan of `.text` finds the sites. An absolute-dword scan of every
  section for the eleven entry and state routines finds **0** references; the positive control,
  `00CE223C`, gives 16. So no entry is reached through a pointer.
- **ECX.** ECX comes from a backward reaching-definition walk over the enclosing function's linear
  decode. The walk follows fallthrough and branch predecessors, chains through `MOV ECX,reg` and
  `LEA ECX,[reg+N]`, and treats any `CALL` as clobbering ECX.
- **Three sites resolved by hand**, where the register value depends on a guard:

| site | evidence | stream |
| --- | --- | --- |
| `007B1CE2` | `LEA ECX,[EAX-6]` on the arm where `007B1CCC CMP EAX,7 / JNE` fell through | 1 |
| `007BC0FC` | `LEA ECX,[EDI+2]` after `007BC0F4 CMP EDI,-1 / JNE` | 1 |
| `00933F02` | `LEA ECX,[EDX+1]` after `00933EC3 TEST EDX,EDX / JNE` | 1 |

Spot check: `009C579E` resolves through `MOV ECX,EBX`, `009C53E2 LEA EBX,[EBP-1]` and `009C53DD MOV
EBP,2`, giving stream 1.

| entry | stream 0 | stream 1 |
| --- | --- | --- |
| `00BD2F10` | 226 | 399 |
| `00BD2F40` | 11 | 3 |
| `00BD2F90` | 0 | 2 (`00902D77`, `00902D95`, `BSP_GunBot_LeadAimTick`) |
| `00BD2FC0` | 94 | 18 |
| `00BD2FD0` (seed) | 2 | 4 |

### 1.3 Sites outside any Ghidra function

18 sites lie after INT3 padding that ends the preceding indexed function's body. Ghidra and the
index have no function there. The report gives each its real start (`reach` field):
- 6 are referenced: vtable slots (`006E24E0`, `007EBF10`, `009BA2F0`, `009BD6E0`) or a call
  (`006E7C60` from `006E7DC6` in `BSP_Projectile_CommitStepPose`).
- 12 have no CALL/JMP/Jcc (rel32 or rel8) and no absolute dword reference: apparently unreached.

Their class is the preceding function's, marked `guess`. Two more sites, `006FCA6B` and `00AF7F92`,
are inside their function by bytes, but Ghidra's body for that function stops short of them.

## 2. Seeds and state sets

| site | stream | value | trigger |
| --- | --- | --- | --- |
| `00CD8A30` | all 20 thread states (10 slots x 2) | 4357 (`1105h`), by vector-construct with `00BD2E00` | static initialisation |
| `00CD8A10` | fallback `00E14748` (unregistered threads, **both stream ids share it**) | 4357 | static initialisation |
| `005CD1B0` / `005CD1BC` | 1 / 0 | 12345 / 54321 | `BSP_HudMovieScreen_EngageMovieInterface`, the first MovCamNew engage |
| `00798CAD` / `00798CB9` | 1 / 0 | 12345 / 54321 | `00798C80`, each movie camera's first step with dt > 0 |
| `0079D1AB` | 1 | 123 | `0079D020`, movie camera constructor |
| `0079A2FB` | 1 | wall-clock ms | `0079A260`, movie camera destructor |

Every reseed runs on the main thread, so it sets **slot 0 only**. Worker threads' streams keep their
4357 sequence for the whole process.

Separate generators that take no stream id, listed so nobody mistakes them for consumers:
- the ocean beams state `00E130C0` (`00BA3275`, `00BA32C3`, `00BA3472`);
- the ocean water-particle state `00E13A88`, seeded 4357 at `00CD88BA`;
- `BSP_OceanSpectrum_BuildInitial`'s stack state (`00B9579B` seed, `00B957D9`/`00B957F4` normals);
- `00B94B0C` in `BSP_ParseNativeMeshItem_00b94900`.

None of these touches the thread streams.

## 3. Threads: whose stream a draw uses

- **Registration.** `00BD2FE0` has three callers: `BSP_WinMain` (slot 0), `BSP_FrameJobPool_WorkerMain
  00BE2EA0` (one slot per worker), and the unreferenced wrapper `008F7CF0`. The Dyn task threads
  never register, so a draw from them would hit the shared fallback state.
- **Dispatch.** `docs/CC13_FPU_PRECISION.md` section 3, from the listing:
  - `00875BB0` queues every fixed-step element into the frame-job pool and dispatches with the byte
    `00E0B6CE`. That byte is `1` in `.data`, and only the Lua `SetSingleThread` changes it.
  - With a non-zero byte, the workers and the main thread claim jobs by a LIFO race.
  - `BSP_Game_Render` dispatches with 0, so render-time work stays on the main thread.
- **Consequences:**
  - Presentation draws made during render all hit slot 0 stream 0, on the main thread. These are
    the particles, post effects, cameras and weather.
  - A gameplay draw made inside a fixed-step wave virtual hits the stream of whichever thread
    ran that element, with slot 0 or a worker's slot chosen per job.
  - Main-thread sequencing (message handlers, the sixteen-call fan-out, Lua) stays on slot 0.

  Which gameplay consumers sit inside a wave virtual was not traced per site; that is open item 2.
  **In the image, then, neither stream's per-draw order is reproducible:**
  1. stream 1 for the clock reseed and the job race;
  2. stream 0 for the job race and for frame-rate-dependent render draws.

## 4. Stream 0

### 4.1 Gameplay and neutral consumers (all 17 sites)

| site | entry | function | what the draw sets | host |
| --- | --- | --- | --- | --- |
| `0099C7BA` | F10 | `BSP_BotTask_ConstructBase 0099C6F0` | `task+3E8h`, the avoid-gunfire timer (read at `0099EF3A` `BSP_PilotBot_AvoidGunfire`, `009A188C` `BSP_PilotBot_UpdateAvoidance`) | not modelled |
| `0099C97A` | F10 | `0099C8B0` (the base variant) | same `+3E8h` | not modelled |
| `009B4BF3` | F10 | `BSP_BotStateLevelBombAttackRun_Enter` | state `+18h` period | modelled on the **units** generator (`lb_draw "ar18"`, `game_hosts_units.cpp:21532`) |
| `009B6D03` | F10 | `BSP_BotStateLevelBombMoveTo_Enter` | state `+44h` period | modelled on the units generator (`lb_draw "mv44"`, `:21527`) |
| `007BC3C7` | F10 | `BSP_Plane_StartNamedEffect` | `plane+C10h` ExplosionExplosionDelay | modelled, host stream-0 LCG (`game_hosts_units.cpp:16615`) |
| `007BCABB` | F10 | `BSP_Plane_OnDestroyed` | `plane+C14h` engine-fire delay | not modelled (`game_hosts_units.cpp:13512` says so) |
| `00745D47` | F10 | `BSP_LandFort_FrameUpdate` | secondary-explosion chance roll | modelled, host stream-0 LCG (`game_hosts_ship_ai.cpp:12585`) |
| `00745E5B` | F10 | same | back-off timer U(5, 15) | modelled, host stream 0 (`:12593`) |
| `00745EBB` | F10 | same | class+2Ch point draw | not taken (`:12591`, labelled "count 0") |
| `007464E6`, `00746596` | F10 | `00746320` (fort fire start) | fire durations `+748h` | modelled, host stream 0 (`:12531`, `:12543`) |
| `0074672B`, `0074679D` | F10 | `007465E0` (fort death-arm fire) | fire durations | not located |
| `006E5479`, `006E549E` | F10 | `BSP_MBombPlatform_Drop` | two offsets added into the spawn position | not modelled (host `bomb_scatter` takes the four stream-1 draws only) |
| `009E465F` | F10 | `BSP_ShipAi_NavigationBlockConstruct` | `-U(0,1)` into `blk+148h` (no traced reader) | modelled, host stream 0 (`game_hosts_ship_ai.cpp:11692`, `kShipAiTimerDrawsBound`) |
| `006E7B97` | F10 | `BSP_ProjectileTickableEntity_Construct` | `projectile+27Ch` = U(0, 0.1). **Neutral:** the only readers, `006E73FA` and `006E74A4`, are in `006E71B0`, the bullet serializer (`"bulletclass"`, `"_bullet"`, `"_bulletBase"`) | not modelled |

The host's stream-0 generator, "host stream-0 LCG" above, is the static `stream0` in
`GameGunneryHost::death_mode_draw_00bd2f10` (`game_hosts_gunnery.cpp:12285`, seed `2545F491h`).
It is never reseeded.

### 4.2 Presentation consumers (313 sites)

| family | sites | examples | host |
| --- | --- | --- | --- |
| scene props, debris visuals, point effects (`0042xxxx`..`004Bxxxx`) | 102 | `0049A760`/`0049B1F0` (19 each, plane break-up visuals), `0049C000` point announcement (17), `004A0D20` (15), the blast-prop picks `00428800`/`00428420` | not run |
| particles (`00AFxxxx`..`00B0xxxx`, `008742A0`, `00873AE0`) | 83 | the tracer, sprite, floating and axial inits (19, 12, 11, 9), emitters | reconstructed as libraries, never called by the game host (`game_hosts.cpp` reaches only the manager constructor `00AF0B10`) |
| post effect old film `00B529A0` | 46 | per rendered frame while active | not run |
| weather, water, foliage | 53 | waterdrops (16), lightning (20), clouds (6), rain `008023B0` (7) | not run |
| unit effects | 19 | damage-smoke effect pick `00822A89`, fort effect picks `00745D79` and `007463C1`, plane break-up | **the three picks are modelled on the host stream-0 LCG**, interleaved with the gameplay draws as in the image |
| camera, HUD, movie | 10 | camera mover constructor (3 per construction, `docs/CC13_CAMERA_CONSTRUCTION_EVENTS.md`), wander retarget (4), MovCam picks | not taken (`mission_camera.cpp:177`) |

## 5. Stream 1

- **372 gameplay sites in 220 functions**, plus 4 seed sites. These are the pilot-bot tasks and
  states, ship AI, gunnery and gun bots, projectiles, unit damage and ship systems, air ops, the AI
  planners, and the Lua `BSP_LuaBinding_Random 0088C160`.
- **Host status from the address grep**:

  | host status | sites |
  | --- | --- |
  | cited on a host draw line | 45 |
  | cited in comments only | 62 |
  | function referenced, site not cited | 160 |
  | no reference at all | 105 (68 functions) |

  "Function referenced" does not mean the draw is modelled. It needs a per-function read.
- **The host splits stream 1 into several generators.** In the image they are all one stream:

  | host generator | users |
  | --- | --- |
  | **H-G**, `GameGunneryHost::Impl::rng` (LCG, seed `9E3779B9h`) | gunnery; ship AI through `ship_ai_draw`; plane death mode; the movie reseeds (`reseed_shared_stream_00bd2fd0`) |
  | **H-U**, `GameUnitsHost::Impl::db_release_rng` | squadron release, level-bomb, `st_draw` / `lb_draw` |
  | **H-AI**, `game_hosts_ai.cpp:1550` | AI group think intervals |
  | **H-SO**, `GameScriptOrdersHost::random_state_` | script orders |

  Only H-G receives the movie reseed.
- **Presentation on stream 1 (50 sites):**
  - movie camera shake `00454A50`/`00454EF0`/`00439A00` (12);
  - debris from unit break-up, sinking and part detach, `00935D30`/`00935540`/`00934150`/`0092A240`
    (23);
  - damage smoke `008227E0` (3);
  - HUD plane effects `0060ABD0`/`00609BD0` (3);
  - the movie camera per-step draw `00798D07`;
  - the named-effect pick `007BC0FC`;
  - rain `00801400`;
  - scene-load clouds `004BA870` (5);
  - lightning `00BC05A9`.

  The host draws two of these from H-G:
  - damage smoke `00822981`/`00822A17`/`00822A43` (`game_hosts_gunnery.cpp:12254-12256`);
  - the movie step `00798D07` (`game_hosts_hud.cpp:1917`, `kMovieStepDrawBound`).

  The clouds are a stub that returns `low` (`game_hosts_scene_contents.cpp:2877`). The rest are
  not run.

## 6. Couplings that matter for same-binary pairs

1. **Host H-S0.**
   - **Who shares it.** The plane explosion delay, the land-fort rolls and timers, the ship-AI
     navigation block, and three presentation effect picks (damage smoke, two fort effects).
   - **What breaks a pair.** H-S0 is never reseeded, and nothing that is presentation-only in the
     host draws from it. A pair stays clean as long as no new presentation code is wired to
     `death_mode_draw_00bd2f10(0, ...)`. In the image, the same consumers sit behind hundreds of
     render-time stream-0 draws, so the host cannot and should not reproduce their values; only
     the separation can be reproduced.
2. **Unmodelled stream-0 gameplay that would join H-S0 if wired:**
   - the bot-task avoid-gunfire timer (`0099C7BA`/`0099C97A`): one draw per bot-task construction,
     which is frequent;
   - the engine-fire delay `007BCABB`;
   - the bomb-drop offsets `006E5479`/`006E549E`;
   - the fort death-arm durations.

   Wiring any of these to H-S0 offsets every later H-S0 draw. In the image they are interleaved
   with presentation anyway, so a keyed stream (unit, consumer) is as faithful as the shared one
   and is pair-stable.
3. **The projectile draw `006E7B97`** is neutral for the simulation and must stay unmodelled, or be
   drawn and discarded on a stream of its own. If it were put on H-S0, gunfire volume would move the
   fort and plane-delay draws.
4. **Presentation on H-G.** The damage-smoke respawn draws (3 per respawn) and the movie per-step
   draw advance the gunnery generator, the same coupling shape as the image (both are stream 1).
   - **Effect on a pair.** A change that moves smoke respawns or movie length shifts every later
     gunnery and ship-AI draw on the default path. `BSP_GUNNERY_RNG_STREAMS=1` keys them apart.
   - **cc9's choice.** If cc9 wants presentation off the gameplay stream, these two are the only
     cases in the host.
5. **The level-bomb periods** (image stream 0) are drawn from H-U, a stream-1 stand-in. That is
   labelled "modelled, other generator" in the report. Under H-U's per-key option they are
   isolated; on its default path they share H-U with the squadron release draws.
6. **The movie reseeds.**
   - The host applies the stream-1 reseeds to H-G only. H-U, H-AI and H-SO are not reseeded,
     although in the image they are the same stream.
   - The host never applies the stream-0 reseed, which matches its separation.
   - In the image, the slot-0 stream-0 reseed erases the presentation offset accumulated before
     the last movie camera's first step. Draws made after that point still interleave with
     per-frame render draws.

## 7. Corrections (for the files' owners)

- `docs/RANDOM_STREAMS.md` section 1:
  - The counts are superseded by section 1.2 above.
  - "every one of the 327-plus stream-1 sites draws from it" is true for slot 0 only: fixed-step
    wave draws can run on a frame-job worker's own streams (section 3).
- `src/game_hosts_gunnery.cpp:14040` ("stream 0: no generator in this host") and
  `src/game_hosts_hud.cpp:1843` ("stream 0 has no host generator") are stale. The static `stream0`
  in `death_mode_draw_00bd2f10` is a stream-0 generator, and it is not reseeded. These are cc9's
  files and were not edited.
- `src/mission_camera.cpp:178-179` says the camera mover's draws "would advance the shared random
  stream the ship AI draws from". They are stream 0 (`XOR ECX,ECX`). The ship AI's draws are
  stream 1, except `009E465F`, which is on H-S0.

## 8. Uncertainty and open items

1. **Classification confidence.** Each row is marked `read` (store target or reader checked),
   `name`, `context` (callers, roots, strings), `band` (stream-1 gameplay by address band) or
   `guess`.
   - The `context` presentation rows in `0042xxxx`..`004Bxxxx` are scene and visual objects by
     caller and string evidence; their fields were not traced to a renderer.
   - The 349 `band` rows are gameplay by subsystem, not per-draw readings.
2. **Thread placement.** Which gameplay consumers run inside fixed-step wave virtuals, and so on
   a worker's stream, was not traced per site.
3. **Host status for stream 1.** It is an address grep. A per-function read of the 160 "function
   referenced" sites would turn each into modelled or not and name its host generator.
4. **The 12 apparently unreached functions** (section 1.3) could be reached through a computed
   address that neither scan finds.
5. **`00BF0DF0`'s variable word count.** The two gun-bot normals (`00902D77`, `00902D95`) consume a
   data-dependent number of words. The host's `draw_normal` does not need to match this, but it
   is one more reason exact sequences are out of reach.

## 9. Handoff

The census is complete for the five stream entries. The next packets, in value order:
1. A per-function host read of stream 1's "function referenced" rows. Use
   `local/rn1_hostlines.py <fn>` and set `HOST` overrides in `local/rn1_build.py`.
2. A thread-placement trace for the 16 stream-0 gameplay sites. Find whether each runs from a
   wave virtual (`00875BB0`'s `+4h`/`+8h`/`+0Ch`) or on the main thread.
3. cc9's decision on items 2 to 4 of section 6.

Re-run: `python local/rn1_census.py && python local/rn1_host.py && python local/rn1_build.py`, all
from the worktree root.

## 10. Stream-1 host read, the "function referenced" rows (packet cc13-rng2, 2026-10-12)

Worker cc13-rng2, 2026-10-12 08:28 UTC, base origin/main 8af4cee9e. Read-only: no C++, no Ghidra
writes, no ledger records. This closes open item 3 of section 8 for the 160 stream-1 GAMEPLAY sites
whose host status was "function referenced; site not cited". Each site's instructions were read
from the disk image (bounds resolved from the float constants). The host's handling was read from
`src/` and `include/` at the function and site addresses, in bounded windows. Nothing was built or
run. "Modelled" means the host source draws the same distribution; no run checked it.

Data: `reports/cc13_rng_stream1_host_status.json`, one row per site with the class, the image
distribution, the host behaviour, the host file:line and whether the host labels the difference.
Scripts in the worktree: `local/rn2_ctx.py` (image listing around a site), `local/rn2_grep.py`,
`local/rn2_results.txt` (the hand classification), `local/rn2_build.py` (report).

### 10.1 Counts

| class | sites | notes |
| --- | --- | --- |
| MODELLED | 88 | H-G 38, H-U 48, H-AI 2 |
| MODELLED-DIFFERENTLY | 32 | 30 labelled, 2 unlabelled |
| NOT-MODELLED | 27 | 13 labelled, 14 unlabelled |
| HOST-OMITS-FUNCTION | 13 | 4 are apparently unreached code in the image (section 1.3) |

The H-U generator is `GameUnitsHost::release_altitude_draw_00bd2f10`, with its `st_draw` / `lb_draw`
wrappers. It is keyed by the unit name plus a suffix (`name#t88`, `"mv_side"` and so on). So the
pilot-bot draws are separated per consumer, as section 6 item 2 suggests. They are not on H-G.

### 10.2 For cc9: the unlabelled differences

The two unlabelled MODELLED-DIFFERENTLY sites come first; they are what the lanes act on. The
unlabelled NOT-MODELLED sites follow, gameplay first.

| site | function | class | image | host | host file:line |
| --- | --- | --- | --- | --- | --- |
| `0070C247` | BSP_FlakProjectile_Detonate | MD | `U(BlastDamageMin, BlastDamageMax)` on **every** `0070C210` call. That includes the FlyTime expiry (`0070C3B7 PUSH 0` then `CALL 0070C210`), and `0084BAD0` follows the draw unconditionally (`0070C280`). An expiring flak round bursts with radial damage | draws and bursts on the proximity lock only. The FlyTime expiry is the "silent" life bound (no draw, no blast) | `src/game_hosts_gunnery.cpp:8960`, `:8856`; `docs/FLAK_PROXIMITY_BURST.md:17` |
| `009C7839` | BSP_BotStateDiveBombTurnDown_SetRollDirection | MD | roll magnitude `U(0.8, 1.0)` with the side sign | the dive-bomb binding's `uniform_between(low, high)` returns `low`: always 0.8 | `src/game_hosts_units.cpp:18914` via `src/dive_bomb_task.cpp:1255` |
| `0084C19F` | BSP_Projectile_TraceSegmentAndImpact | NM | `U(DamageMin, DamageMax)` handed to `00428420`, the blast-reacting scene props | `00428420` not run. The consumer is presentation (section 4.2 classes `00428420` so); reclassify | none |
| `00827B4D` | BSP_ShipInstance_OnHealthChanged | NM | at death, `U(0, 1)` against `Physics.WreckChance` (settings `+65Ch`) decides the breakup message | library reconstruction only (`ship_on_health_changed_00827a90`, `host.breakup_roll`); no game host calls it | `src/unit_damage.cpp:461` |
| `007BC1F2` | BSP_Plane_StartNamedEffect | NM | with `+C36h` clear, `U(0, 1) < 0.5` before the `007BA200` session message | not drawn; the host models this function's stream-0 death delay only | `src/game_hosts_units.cpp:16616` |
| `007D6247` | BSP_Plane_ReadPropertyBag | NM | `unit+C44h = U(2, 4)` | not drawn; no host field (its reader was not traced) | - |
| `007D7F90` | BSP_PlaneFlightController_Construct | NM | `U(DeadMeat RotationMin pair)` (`00F87330`/`00F87334`) | not drawn; the tuning pair is loaded and unused | `include/bsp/game_tuning_singleton.hpp:192` |
| `007F2D5B` | BSP_PlaneSquadronTickableEntity_Construct | NM | `+3C4h = -U(0, x)`, phase stagger | not drawn | - |
| `006CDAA5` | `006CD810` (callee of BSP_AirOps_Update) | NM | U01 roll | no host reference | - |
| `008AC94B` | `SetDeadMeat` Lua native (`008AC7B0`) | NM | `U(-1, 1)` | name-table entry only; no handler | `src/mission_lua_host.cpp:303` |
| `009FB45D`, `009FB5AF` | BSP_BotApproachTargetRef_ConstructFromStream | NM | `+60h = -U(0, x)` on the stream-construction (load) path | not hosted | - |
| `008540EA` | BSP_SubmarineUnit_Serialize | NM | `U(0, 1)` vs 0.5 | serialize path not hosted | - |
| `008546E4` | BSP_SubmarineUnit_Update | NM | `NextU32 % 15 == 0` calls `00933BB0` (the parts area section 5 calls debris presentation) | not drawn; only the periscope arm is hosted | `src/game_hosts_units.cpp:9164` |
| `0082571D` | BSP_UnitInstance_Update | NM | `+BC8h = U(Sounds.ShipDeadMeatSoundTimeMin, Max)` (`+64Ch`/`+650h`): a sound timer | not drawn. Presentation; reclassify | - |
| `006526DF` | BSP_HudUnitView_UpdateOrderAndView | NM | `U(0, 1) > 0.99` on the player's HUD order path | not drawn | `src/unit_order_record.cpp:102` |

Two of these are presentation, not gameplay: `0084C19F` (scene props) and `0082571D` (a sound timer).
`008546E4` probably is too. For the image this means that ship sinking sounds and gunfire hitting
scene props advance stream 1, as sections 4.2 and 5 describe for the others.

### 10.3 Labelled differences (the host says it substitutes)

- **The dogfight task draws nothing; every draw is at its midpoint.** Label:
  `kDogfightManeuverBodyBound`, `src/game_hosts_units.cpp:5476-5482`, "Draws at their midpoints, as
  every other dogfight draw in this host". 19 sites:
  - **The aim enter** (`009A75EE` U(0.8, 1.4), `009A7624` U(0.5, 0.8)).
  - **The avoid enters and their common tick** (`009A7D72`, `009A7E09`, `009A7E4E`, `009A7E6C`,
    `009A8049`, `009A807B`, `009A80B5`, `009A80D3`).
  - **The maneuver arm, enter and tick** (`009A8596`, `009A88BD`, `009A89A5`, `009A8AD5`, `009A8AFE`,
    `009A906B`).
  - **The target-select reselect timer and noise** (`009AA65A`, `009AA97F`).
  - **The neighbour finder noise** (`007E21AA`).

  Two of them change behaviour, not just spread:
  - `009A8712`: the maneuver expiry rolls `U(0, 1)`, doubled when threatened, against
    `TurnAfterChance`. A fixed 0.5 makes the chance a threshold. A pilot whose `TurnAfterChance` is
    above 0.5 always turns after; below it never does.
  - `009A99C6`: the avoid pick draws `U(0, roll + turn weight)` against the roll weight. The host
    picks turn exactly when the roll weight is under half the sum, so a weighted coin becomes a
    majority rule.

  The dogfight attackrun and maneuver constructor phase draws (`009A713A`, `009A853A`) are not
  taken at all (NM, the same label).
- **Plane neighbours**: the two clocks are seeded at their midpoints (`007E1F94`, `007E1FB9`;
  `src/game_hosts_units.cpp:987-992`, "U(0,P)+P at midpoints"). The third draw (`007E1FE1`,
  `U(0, +90h)`) has no host field.
- **Fighter gun burst clock** `009FCED1`: the image draws `U(robot +234h, +238h)` from the pilot's
  skill row. The host uses `burst_draw = 3.75` and `delay_draw = 1.15`, which
  `include/bsp/dogfight_task.hpp:328-329` describes as midpoints of `U(3.0, 4.5)` / `U(0.7, 1.6)`.
  Nothing sets them per skill level.
- **Torpedo attackrun constructor** `009D072A`: `-0.5` for `-U(0, 1)`. Label:
  `src/game_hosts_units.cpp:1551`, "the midpoint, as every other draw in this host".
- **Dive-bomb approach** `009C3FB0`: in-range `U(1.6, 1.8)`, pinned to 1.6. Label:
  `src/game_hosts_units.cpp:22477`, "Pinned at the low end of each draw, as this host pins every
  draw". The same approach's `U(0.6, 0.8)` (`009C3F81`) *is* drawn, under
  `kAttackDistDrawBound`. The turndown roll `009C7839` is the unlabelled pin in 10.2.
- **AutoTarget phase** `009F6A8F`: `-(index % 20) * 0.05` for `-U(0, 1)`
  (`src/game_hosts_ship_ai.cpp:11816`).
- **Command-building capture phase** `006F56FF`: 0 for `-U(0, 1)`
  (`src/game_hosts_ship_ai.cpp:12235`).
- **Air-ops squadron creation** `006C52BA`: the image passes `U(100, 300)` with the squadron's
  property bag. The host creates one plane at a fixed 150 m over its home base, and that
  substitution is labelled. Whether the image's draw is that height was not traced.
- **Labelled NOT-MODELLED:**
  - gun barrel re-arm `0072D579`: the value is equal, because every ReloadTime in this
    installation is a scalar; only the stream advance is skipped. Label at
    `src/game_hosts_gunnery.cpp:2927`;
  - the flak passing burst `0070C7F7`, argued unreachable in the image, `:8879`;
  - the shipyard hangar pick `00844AB4`, `:1021`;
  - the ordnance sub-object `009FD0E0` (2 sites, a contract), `src/game_hosts_units.cpp:29952`;
  - the cruise profile `009C3650` (2 sites), `:26400`;
  - strafe aim `+18h` `009CB116`, no reader, `:21031`;
  - path cursor random direction `007B1CE2`, start mode 7 only, `src/game_hosts_commands.cpp:3144`;
  - the Duel order `00A2CD47`, `include/bsp/ai_command_tick.hpp:185`;
  - the ship wreck handler `00824C5C`, logged unimplemented, `src/game_hosts_ready.cpp:595`.

### 10.4 HOST-OMITS-FUNCTION

- **The plane depth-charge bot task** (`009A3390` approach, 4 sites, and the `009A36A0` attackrun):
  no host runs it; the references are library headers.
- **Other functions no host reaches:**
  - the moveonpath task constructor `009BCE50`;
  - the manoeuvre constructor `009BBAD0` (factory `009BBF60`);
  - `007B5AF0`;
  - the vtable slot `007EBF10`;
  - the Lua `Spawn` placement search `00941FA0` (no host implements `find_spawn_position`).
- **Apparently unreached in the image** (section 1.3): `006E242B`, `006F2402`, `009DA9A9`.

### 10.5 What is modelled (88)

| area | generator | notes |
| --- | --- | --- |
| Gunnery | H-G, keyed `Draw::` | aim-error envelope, ranging, visibility TTL, AA flak error (13 sites), aim wander, blast damage on depth-charge burst and impact. The AA flak sign is `U(0, 1) < 0.5` against the image's `NextU32 & 1`: the same Bernoulli(0.5) |
| Land-fort fire roll `00747116` | H-G | run only on the land-fort and capture-building damage paths; other kinds with `SmokeFireChanceMul > 0` were not checked |
| Ship AI | H-G `ship_ai_draw` | sub-attack bands, torpedo track admission (3), brain-constructor timers (5, including two drawn and discarded for the order) |
| AI groups | H-AI | the party think `U(3, 5)` and the cautious-attack U01 |
| Pilot bots | H-U | avoid-gunfire timers; land, level-bomb, strafe (goaway, flikflak, tightturn), retreat-zone, moveto, takeoff and torpedo-approach draws; wreck leak weights; the wanderer |

The torpedo-approach reset draws its six values in a different order from the image: `#t88` and
`#t12c` first. H-U is keyed per suffix, so on its keyed path the order does not matter.

### 10.6 Uncertainty

- **Upper bounds.** Several were read from a stack slot rather than a constant; these rows give
  the host's bounds as stated in its comments. The magnitude and order of the draws were checked
  against the listing.
- **Unconfirmed consumers.** The consumers of `007D6247` (`+C44h`), `007BC1F2` (`007BA200`'s
  message) and `006CDAA5` were not traced. Their gameplay class is still the census band.
- **Flak expiry.** The image reads no stack argument before `0084BAD0` (`0070C24C`..`0070C280`).
  That fact makes `0070C247`'s FlyTime-expiry blast certain from the listing. Its game effect (time-fused AA
  bursts) was not run.
- **Sites outside this set.** The other host-status rows of section 5 were not re-read: "cited at
  a host draw" (45), "comments only" (62) and "no reference" (105). The known pins the lead listed
  are all there, not here:
  - the aimglide re-arm `U(0.2, 0.5)` pinned to 0.2 (`009C579E`) and the aimdive re-arm
    `U(0.5, 1.0)` fixed at 0.5 (`009C6114`) are "comments only";
  - the torpedo AngleErr, drawn per shot (`00900830`/`0090083E`), is "cited at a host draw".

### 10.7 Handoff

1. For cc9 (`0070C247`). Decide whether the flak FlyTime expiry should burst. That changes AA
   lethality, not just the RNG.
2. For cc9 (`009C7839`). The dive-bomb binding's `uniform_between` should draw from H-U, or say
   that it pins.
3. Reclassify `0084C19F` and `0082571D` (and probably `008546E4`) as PRESENTATION in the census
   report.
4. Run the same read for the 62 "comments only" and 105 "no reference" rows. The
   "comments only" rows hold the known pins. Use `local/rn2_ctx.py <fn>` for the image side.
