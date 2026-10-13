# CC13 GPR 40: segments 40 and 59 reference models (gun gates, re-arm, gun bots, TorpedoBot gates)

Packet `cc13-gpr40` (cc13 lineage, Claude). Segment 40 is `00728A90`-`00735EC0` (destroyed, barreldelaytime,
torpedo, titlelist, throwb). Segment 59 is `008EE670`-`00922240` (bulletthrowmul, vertangleerror, torpedobot,
thinktimeleft). cc9 approved this reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg40_59_cc13.cpp` and `include/bsp/gameplay_seg40_59_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

Excluded:
- `gun_can_fire_00729a80`, which is hosted;
- the `005459B0` artillery-kind gate, which is hosted;
- segment 68;
- `0072C970`, which cc9 had leased (`cc9_g40_torpedo_fall`) when this pass started.

The lease covered the rest of both segments.

**Method.** Every branch, store and RNG call site was read from the disk bytes with Capstone `disasm-raw`.
The three x87 sequences that are modelled (`0072F76D`, `008FDC4D`..`008FDC7F`, and `009001C7`..`009002BF`)
were read from the opcodes. `tools/x87_dc_form_scan.py` finds one DC-form site in segment 40 (`0072CE4E`, in
`0072CDD0`) and eight in segment 59 (`008FF812`, `009004AD`, `00900543`, `0090066B`, `00901EC0`, `0090245E`,
`00902E8F`, `00902F11`). None of them is in a modelled stretch. Section 5 lists them for cc9, because four are in
hosted bots.

The models are **reconstructed and build-tested**. None of them is ABI-compatible, wired or game-validated. They
round at every float store, as 24-bit gameplay precision does. Two lengths (`0042B2F0`) are taken in double and
rounded once, where the image rounds each square and sum to 24 bits. That costs at most a few ulp.

Segment 40 had 136 functions with no record and no tag. Segment 59 had 330, of which about 215 are not EH funclets
or STL. Most of segment 40's gun code is already hosted or documented by cc9: GUN_SHOT_CADENCE, BULLET_THROW,
AA_TARGETING, GUN_BASE_TICK and SHIP_PLATFORM_ATTACHMENT. Segment 59's first half (`008EE670`-`008F8000`) is the
scene property bag and CPropTreeLibrary, and its tail (`00904000`-`00922240`) is mission scoring. Neither is gunnery.
This pass took the gunnery: the gun gates, the re-arm, the ammunition mode, the line-of-fire cache and the bots in
`008FB4C0`-`00903600`.

## 1. Findings with a host consequence

### 1.1 `00728A90` is the anti-air kind test: submerged AA guns fire in the host

`00728A90` (`00728A9C`..`00728AB3`) answers `[[gun+3F4h]+80h]` in {1, 5, 6}. `00729A80`'s test 7 asks it at
`00729B3F` after the muzzle's world y fails `COMISS 1.0 (00D7A24C)` at `00729B2F`. It then asks `005459B0` (kinds
2, 3, 4, 6). Either one refuses the shot. GUNNERY_OPEN_ITEMS 206 bound the second predicate and left `00728A90`
"false (not read here)".

The host still feeds `gate.muzzle_submerged = false` at two places:
- `src/game_hosts_gunnery.cpp:7854`, the 0ADh FireIfReady path;
- `src/game_hosts_gunnery.cpp:11776`.

So a kind-1 (AA gun) or kind-5 (flak) mount fires with its muzzle under `y = 1.0`. Kind 6 is already refused
through the artillery set.

The rows this reaches:
- a submerged submarine's AA mounts (USN06 and JM06 carry submarines; the 206 pair silenced their deck guns);
- the AA mounts of a sinking hull.

The model is `gun_submerged_fire_blocked_00729b2a`. A NaN y falls through to the two kind tests, as `JBE` does.

### 1.2 The TorpedoBot tick's think timer, two range gates and no-solution exit (`008FFF20`, `009000DD`..`0090031E`)

The host says (`src/game_hosts_gunnery.cpp:837`): "Steps 5 and 7 (the two range gates) and the 0.2 s think timer
stay unmodelled". Read from the bytes, after the tick's target and role tests:

1. `009000DD`..`009000F6`: `thinkTimeLeft` (`bot+5Ch`, named by the property reader `008FF360`) `-= dt`, stored
   to float. If it is still `> 0`, the tick returns (`FCOMI`/`JA`, so NaN proceeds). Otherwise it is reset to
   `0.2f` (`00CE54A0`) at `00900101`.
2. `009000FC`..`00900117`: if `bot+64h` is set and `008527E0(bot+64h)` answers false, the tick returns and the
   trigger is untouched.
   - The attach `008FF310` sets `bot+64h` to the payload only when it answers `IsKindOf(8)`.
   - `bot+58h`, the same payload, is used as the gun.
   - So for a gun payload this test is expected to be dead. That is uncertain.
3. `0090016C`..`009001F3` (gate A): `d = |target - gun|` in 3D (`0042B2F0`). The limit is
   `00415510(&(bot+68h + 100.0), &(unit+44Ch * 1.5))`, that is `min(unit range * 1.5, attack range + 100)`. The
   doubles are `00D7A220` and `00CE3D78`. If `d > limit`, the trigger drops (`vtable[1E8h](0)`, `009003B5`).
4. `0090025E`..`0090026B`: when `008FBB00` has no intercept, the trigger drops (`0090097B`).
5. `00900280`..`0090031E` (gate B): `P = intercept + unit+6D4h * target forward (x, z)`. Then
   `d2 = |P - gun|` over x and z with y = 0. The limit is `min(unit+44Ch, bot+68h)`. If `d2 > limit`, the trigger
   drops (`00900974`).

`00415510` answers `b > a ? a : b`, with `a = *ECX` and `b = *EDX`, so a tie or a NaN `b` gives `b`.

`bot+68h` is the attack range. `00903420` computes it whenever the target changes (`009035C3`, from the
TorpedoBot SetFireTarget `009035A0`), as `00901BA0(size, bot+6Ch)`.
- `00901BA0` is `008387B0(settings, 7, size, 00731020(gun class), accuracy)`. The accuracy is FireTargetAccuracy
  when `bot+6Ch` is set and AnyTargetAccuracy when it is clear.
- `size` is 100.0 with no target and no bounds.
- For a unit target, `size` is its class `+0A0h`.
- For any other target, `size` is the largest component of its bounds extent.
- The constructor `008FF260` seeds `bot+68h = 10000.0f` (`00CE3D64`).

The host differs on all four counts:
- no think period: the host evaluates the tube every fixed step;
- no gate A and no gate B: the gunnery pass's own range test is the only one;
- on no solution, the host aims at the target's present position (`:6950`, `lead = at`) and can fire, where the
  image drops the trigger.

Models:
- `torpedo_bot_tick_gates_008fff20`;
- `torpedo_bot_attack_range_00901ba0`;
- `torpedo_bot_target_size_00903420`;
- `min_float_by_ref_00415510`.

The ship-AI host already runs `00901BA0`'s flag-1 arm (`src/game_hosts_ship_ai.cpp:9586`, for `009E9BC0`).

### 1.3 The TorpedoBot's slot accept `008FC010` is more than a range test

The TorpedoBot's vtable `00D182B0` has slot `1Ch` = `008FC010` (dword `00D182CC`). `00729BC0` dispatches a
category-7 gun's accept to it. It asks, in order:
1. the target answers `IsKindOf(6)`;
2. `00828EC0([t+538h])` (the torpedo-proof small craft test) answers false;
3. `[t+5Dh]` is clear;
4. the gun's `vtable[1D4h]`, the line-of-fire cache below, answers true.

The host gives category 7 only the range test (`src/game_hosts_gunnery.cpp:5594`-`5609`). Its `bind_aa_acceptance`
runs the AA bots alone. `kGunneryClassArmsBound` (`00862820`) already refuses `IsKindOf(0Eh)`.

What this adds:
- the landing-ship arm of `00828EC0`: class kind `0Ch`, byte `+808h` clear and `+500h >= 8.938947`;
- the dead byte;
- the line-of-fire test, for torpedoes.

### 1.4 The AAFlakBot rolls its error at every target set, not only after a shot (`008FF200`)

`008FF200` is the AAFlakBot's SetFireTarget. It runs:
1. `006DF170`, which stores the target;
2. if there is a target and the role slot `[[bot+50h]+1ACh]` is AI-held (8, or `00927F10`):
   - a kind-6 gun switches to fire mode 1 with `keep_timers = 0` (`0072D5B0(1, 0)`, `008FF240`);
   - then `008FDBE0` rolls the error (`008FF247`).

The host rolls only after a shot (`src/game_hosts_gunnery.cpp:7249`, `fe.fired`). It starts every gun at
`{0, 0, 0}` (`:2936`). So the first flak salvo on each new target has no error in the host.

Each roll is seven stream-1 draws: `U(0, 100)`, then `U` and one raw MT output for each of h, v and d. The host
already reproduces their order and branch; its `sign()` is a `U(0, 1) < 0.5` draw in place of the MT bit, and the
two are equally likely. The `GoodRatio` split, the `[Min, Max]`/`[Max, Bad]` ranges and the double `pi/180` match
`:7252`-`7264`.

### 1.5 Smaller items

- **TailGunnerBot skill set `008FBEF0`** (vtable `00D18140` slot 18h) draws `aimTime = U(stream 1, AimPeriodMin,
  AimPeriodMax)` at `008FBF5C`. The class's SetFireTarget `008FEA40` then overwrites it with -1.0, so the only
  lasting effect is one stream-1 draw per tail-gunner bot at attach. The host draws nothing there.
- **Re-arm draw `0072D579`.** It is already labelled NOT-MODELLED at `src/game_hosts_gunnery.cpp:3088` and in
  CC13_RNG_STREAM_CENSUS. The draw is on stream 1 (`ECX = 1` at `0072D56E`). `00BD2E60` draws unconditionally, so
  every shot that finds a round advances stream 1 once, after that shot's two throw draws.
- **Throw cone axes.** The cone at `0073075E` adds `r*cos(theta)` times row 1 and `r*sin(theta)` times row 0 of the
  barrel node's world matrix (`00730654`..`0073075E`, as `src/gun_dispersion.cpp`'s `gun_throw_apply_00730654`
  models). The host instead builds `a = normalize(up x direction)` and `b = direction x a`
  (`src/game_hosts_gunnery.cpp:8015`-`8027`). With the same two draws, each shot lands at a different point; the
  distribution is the same while both pairs are orthonormal.
- **The throw modifier.** `00470440(7)` scales theta as well as the radius (`0073062E`). This is neutral at 1.0.
- **Line-of-fire cache expiry.** `0072F6E0` (this = `gun+424h`, `00730A20`) draws `U(0.8, 1.2) * [00432650()+8Ch]`
  per cache miss, after the predicate. The host draws it (`:5737`) and calls it unread. Two checks agree:
  - `0072AD40`'s aging runs on `gun+114h` (`0072D183`), not on `gun+424h`;
  - an `LEA`/`ADD ECX,424h` byte scan finds only the thunk, `0072E6D0`'s setup and Unwind funclets.

  No reader of these expiries was found.
- **`0072D5B0` SelectFireMode** is the ammunition switch that cadence item 8 describes. On a switch without
  `keep_timers`, each barrel consumes a round and keeps its timer, or with no round is pinned at FLT_MAX. The host
  never switches. Its dual-purpose second ammunition is a projectile substitution at fire time
  (`kDualPurposeSecondAmmoBound`), so its reload and barrel delay stay at record 0.
- **`00731020`, `00731040` and `00731050` are gun-CLASS methods on fire record 0**, not on the selected record.
  The names `CurrentBullet` and `CurrentModeRecord` are therefore misleading; the evidence is appended to the
  records.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG (stream 1, image order) | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `gun_submerged_fire_blocked_00729b2a` (`00728A90`) | `gate.muzzle_submerged = false`, `src/game_hosts_gunnery.cpp:7854` and `:11776` | `gun.category` (the weapon kind), `muzzle[1]` | none | OFF: none. ON: AA guns of kinds 1 and 5 with the muzzle below y = 1.0 stop firing: a submerged submarine's AA mounts and a sinking hull's. `can_fire_refusals` rises on USN06/JM06 submarines. A run without submarines or sinkings is identical. |
| `torpedo_bot_tick_gates_008fff20` with `torpedo_bot_attack_range_00901ba0` and `torpedo_bot_target_size_00903420` | the torpedo path `src/game_hosts_gunnery.cpp:6933`-`6952` (no-solution fallback `:6950`), the `:837` note, and the per-step want_fire under `kTorpedoBotReadyGateBound` | `bot+5Ch` (new per-gun state, 0 at attach), `dt`; gun and target positions; `unit+44Ch` (category-7 range); `bot+68h` from `008387B0` (the ship-AI host's `weapon_hit_accuracy_scaled_008387b0`) with the target size; the `008FBB00` solution; `state.torpedo_spread` and the target forward | none in the gates. Fewer evaluations give fewer launches, so fewer `torpedo_gyro` draws (`00900830`/`0090083E`) follow. | OFF: none. ON: launches move onto 0.2 s think ticks, a delay of up to 0.2 s. A tube whose target is beyond `min(1.5 R, A + 100)` in 3D, or whose lead point is beyond `min(R, A)` horizontally, drops its trigger. No solution means no launch. Torpedo shots fall, and every later stream-1 consumer shifts: judge per tube, not by kill totals. |
| `torpedo_bot_accepts_008fc010` | the category-7 slot answer, range only: `src/game_hosts_gunnery.cpp:5594`-`5609` | IsKindOf(6); `torpedo_proof_small_craft_00828ec0` (band_0080_cc13) on the target class; the dead byte; the line-of-fire answer the AA path already computes | none of its own. The line-of-fire miss draws `0072F76D` once per (gun, target), as the AA path does. | OFF: none. ON: tubes refuse fast landing ships and line-of-fire-blocked targets. The extra line-of-fire misses add stream-1 draws. |
| `aa_flak_bot_set_fire_target_008ff200` with `aa_flak_bot_roll_error_008fdbe0` | the after-shot-only roll, `src/game_hosts_gunnery.cpp:7249` (state `:2936`) | target set events (the host's gunnery target change), role AI-held (`slot_ai_held_00927f10`), the gun kind, the AAFlakBot row at the bot's level | seven per target set: `U(0, 100)`, `U`, MT bit, `U`, MT bit, `U`, MT bit (`008FDC0D`..). For a kind-6 gun the mode switch comes first and draws nothing. | OFF: none. ON: a flak gun's first salvo on each target carries a rolled h/v/fuse error, so first-salvo AA kills fall at Stun..SPVeteran rows with non-zero ranges. Seven extra stream-1 draws per acquisition shift all later draws. |
| `tail_gunner_bot_set_skill_008fbef0` | nothing (no draw at bot attach) | the TailGunnerBot row at the bot's level | one `U(AimPeriodMin, AimPeriodMax)` per tail-gunner attach (`008FBF5C`) | OFF: none. ON: the value is overwritten before use, so only the stream position moves, by one draw per plane with a tail gun at spawn. |
| `gun_rearm_barrel_0072d520` | the labelled scalar re-arm, `src/game_hosts_gunnery.cpp:3088` and `:7878` | the provider answers, `ReloadTime[1]`/`[2]` (equal in this installation) | one `U(lo, hi)` per shot with a round, after the shot's throw draws | OFF: none. ON: reload values are identical (scalar ReloadTime); every shot advances stream 1 once, so all later draws shift. |
| `gun_line_of_fire_decision_0072f6e0` | `src/game_hosts_gunnery.cpp:5731`-`5741` (the host's cache) | the predicate answer, `WeaponDirectorThinkTime` (`think_time`) | one `U(0.8, 1.2)` per miss, after the predicate | already equivalent. The host draws at the same point. |
| `gun_select_fire_mode_0072d5b0` | nothing (cadence item 8) | mode, keep flag, per-barrel provider answers | none | OFF: none. ON (with `008FF200` or `006DF4C0` calling it): a dual-purpose gun takes record 1's ReloadTime/BarrelDelayTime against planes. Barrel timers are kept; a gun with no rounds pins every barrel. |
| `gun_least_barrel_timer_00729920`, `gun_is_operational_00729f10`, `gun_torpedo_bot_accuracy_or_one_00729f40`, `gun_min_air_range_record_00729b90`, `gun_class_max_weapon_range_00731020`, the bot getters, `aa_gunner_bot_accepts_008fbe00`, `turret_or_flak_bot_accepts_008fbf70`, `depth_charge_bot_accepts_008fb590` | the host copies `src/game_hosts_ship_ai.cpp:9571`, `:5691`, `src/game_hosts_gunnery.cpp:5666`-`5705`, `:880`-`888`, `:5673`-`5685` | as named | none | none. The host copies agree with the listing (section 3). |

## 3. Host reconstructions checked against the listing

| address | host | result |
| --- | --- | --- |
| `00730160` throw `0073031D`..`0073075E` | `src/gun_dispersion.cpp` (reference); `bullet_throw_magnitude_0073031d` `src/game_hosts_gunnery.cpp:3433` | The magnitude matches. For sub-type 0Ah the host uses `f == 7` where the image tests that `gun+39Ch` exists; for sub-type 10h it uses `f in {5, 6}` and for 4..7 `f in {2, 3, 4, 6}`, where the image tests that the bot slot exists. That is equivalent while `0072C6A0` creates the bots by kind. Both draws are stream 1, theta first. The axes differ (1.5). |
| `0072AD40` | `gun_age_pending_timers_0072ad40`, `src/gun_pending_timers.cpp` | Same. |
| `00729920` | inline at `src/game_hosts_ship_ai.cpp:9571` | Same seed, 3600.0. |
| `00729B90` | `bind_aa_acceptance`, `src/game_hosts_gunnery.cpp:5697`-`5705` | Kind 6 reads record 1, as the image does; the host reads it only against a plane (`dp_air_ammo`), where the image reads it for any target. The host's only consumer is the AA minimum-range skip; whether the image's caller `00865773` also reaches it for a ship target was not traced. |
| `00729F40` | `first_torpedo_bot_accuracy_00729f40`, `src/game_hosts_ship_ai.cpp:5691` | Same: TorpedoBot `vtable[20h]` = `008FB530`, FireTargetAccuracy. |
| `008FDBE0` | `src/game_hosts_gunnery.cpp:7252`-`7264` | Same order and branch. The sign is a `U(0, 1) < 0.5` draw in place of the MT bit. The host's pi is a float, where the image's is a double, so the two differ by at most 1 ulp. |
| `008FBE00`, `008FBF70`, `008FBFC0` | `bind_aa_acceptance` notes `:5673`-`5685` and `:5803` | Same terms. The armour test uses fire record 0 of `[gun+354h]`. |
| bot getters | `kBulletThrowMul` `:880`-`888` (robots.lua rows) | The offsets agree with `robot_config.hpp`'s readers. |
| `00901BA0` | `torpedo_bot_range_00901ba0`, `src/game_hosts_ship_ai.cpp:9586` | Same for the flag-1 arm. |

## 4. Ledger

Function records (`config/reconstruction`, status `semantic_build_tested`, source `src/gameplay_seg40_59_cc13.cpp`):
- segment 40:
  - `00728A90`, `00729920`, `00729B90` (partial), `00729F10`, `00729F40`, `00731020`;
  - `0072D520`, `0072D5B0` (partial: the 0ACh message is the caller's), `0072F6E0`.
- segment 59 getters: `008FB4C0`, `008FB4E0`, `008FB500`, `008FB530`, `008FB550`, `008FB570`.
- segment 59 accept tests: `008FB590`, `008FBE00`, `008FBF70`, `008FBFC0`, `008FC010`.
- segment 59 bots and targets:
  - `008FDBE0` (partial: the `bot+64h` stamp and the `gun+41Ch` hand-off are not modelled);
  - `008FF200` (partial: `006DF170` is the caller's);
  - `008FBEF0`, `00901BA0`, `00903420`.
- a fragment record `009000DD` `torpedo_bot_tick_gates_008fff20` (partial) under the existing `008FFF20` record.

Host checks (status `reconstructed_listing_checked`): `00730160` (partial, source `src/gun_dispersion.cpp`) and
`0072AD40` (source `src/gun_pending_timers.cpp`).

`00415510` is modelled (`min_float_by_ref_00415510`) but not recorded, because it lies outside this lease.

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build, with `gameplay_seg40_59_cc13.obj` compiled, and ctest
  (2 of 2). It passed again after the final `git merge origin/main` (see the handoff). No test was added.
- `tools/verify_report_calls.py local/gpr40_calls.json` checked 37 call-site rows with 0 failures. The rows cover:
  - the test 7 calls `00729B2A`, `00729B3F`, `00729B4A`;
  - the re-arm `0072D579`/`0072D589`/`0072D5A5`;
  - the mode switch `0072D672`;
  - the line-of-fire `0072F73A`/`0072F743`/`0072F76D`;
  - the throw draws `00730557`/`00730575` and `007309E8`;
  - `008FDC0D`/`008FDC48`;
  - `008FF209`/`008FF240`/`008FF247`;
  - `008FBF5C`, `008FC031`, `008FBE93`/`008FBF9C`/`008FBFEC`;
  - `00901BAD`/`00901C0B`, `009034B1`/`00903563`/`009035C3`;
  - the TorpedoBot gates `00900110`, `009001A8`, `009001E6`, `0090025E`, `009002EB`, `00900311`;
  - `008FF319`, `008FF2B3`, `0072D18C`.

  The row file is not committed.
- The `ECX = 1` stream reads: `0072D56E`, `0073054D`, `0073056B`, `0072F761`, `008FDC05`, `008FDC3F`, `008FBF2E`.
- DC-form sites for cc9:
  - `008FFF20`: `009004AD`, `00900543`, `0090066B`;
  - `00902920`: `00902E8F`, `00902F11`;
  - `00901C20`: `00901EC0`;
  - `00902290`: `0090245E`;
  - `008FF640`: `008FF812`;
  - `0072CDD0`: `0072CE4E`.

  These are in hosted or documented functions this pass did not model. Each is `ST(1) = ST(1) op ST0`.

## 6. Open, and named but not reconstructed

- `bot+64h` (TorpedoBot): a gun payload is not expected to answer `IsKindOf(8)`, so the `008527E0` gate looks dead.
  This is not proven.
- `bot+6Ch` (TorpedoBot fire-target flag): its setter was not traced. Only the clear `008FF2B0` was read.
- The line-of-fire expiry reader. The cache at `gun+424h` grows with every new target, and nothing found erases it.
- `0072C170` (the Lua shot `00892F70`) has register inputs and a stepped trajectory loop. It was not modelled.
- Segment 59's scene property bag (`008EE670`-`008F8000`, about 90 named functions) and mission scoring
  (`00904000`-`00922240`, about 100 `FUN_` entries) were not taken: they are not gunnery.
- Segment 40's mission narrative (`00733030`-`00735450`) and session messages were not taken.
- Field names from the property readers, which are not applied to Ghidra:
  - TorpedoBot `+5Ch` thinkTimeLeft, `+60h` targetAngle;
  - DepthChargeBot `+5Ch` thinkTimeLeft;
  - AAFlak `+58h`/`+5Ch`/`+60h` horz/vert/distError;
  - AAGunner `+58h` calcTick, `+60h` angleErrTick, `+64h` actAngleErr, `+6Ch` dAngleErr;
  - TailGunner `+58h`/`+59h` act/nextFireState, `+5Ch` fireDelay, `+60h`/`+64h` horz/vertAngleError, `+6Ch`
    aimTime, `+84h` angleError, `+88h`/`+8Ch` aimPeriodMin/Max, `+90h` shootRange.

## 7. Handoff

State: the models, the records and this doc are committed on `agent/cc13-gpr40`. Nothing is wired. The lease is
released.

The next packets, in order of host reach:
1. **Bind `00728A90`.** In cc9's lane: a switch around `gate.muzzle_submerged` at the two sites, fed from
   `gun_is_anti_air_kind_00728a90(gun.category)`. Pair on USN06 and JM06 against the 206 runs.
2. **Bind the TorpedoBot gates.** This needs per-gun `thinkTimeLeft` state, `bot+68h` recomputed on target change
   from `00903420`/`00901BA0`, and the trigger drop for gates A, B and no-solution.
3. **Bind the AAFlakBot acquisition roll.** This needs a target-change event in the gunnery host. Check how often
   the image calls SetFireTarget (`008FF200` rolls on every call with a target, not only on a change) before
   wiring it.
4. Take the scene property bag (`008EE670`-`008F8000`) and mission scoring (`00904000`-`00922240`) as their own
   packets. They are not gunnery.

Read first: sections 1.2 and 2 of this doc, then `include/bsp/gameplay_seg40_59_cc13.hpp`.
