# Gun shot cadence: why HEAVYARTILLERY lands tens of shots, not hundreds

Addresses: `00729A80`, `00727E30`, `00730160`, `0072D130`, `0072D2C0`, `0072D520`, `0072CF00`,
`007298D0`, `0072AB80`, `0072E6D0`, `007313E0`, `0072D5D1`, `006D1E50`, `00728A90`, `005459B0`,
`006DEE40`, `00427EB0`, `008E6430`, `006DF520`. Data `00D7A218`, `00D7A24C`, `00D7A278`,
`00CFDBF8`, `00CE81A8`, `00CF9054`, `00CFAA48`.

`docs/AA_VERTICAL_WINDOW.md` closed by handing over `no_settle = 1828` as "the thread for whoever
asks why HEAVYARTILLERY lands 29 shots rather than hundreds". This packet pulls it.

**The answer is that 29 is very nearly right.** The authored reload of the two mounts that fire on
IJN01 is `17.5 s` and the instrumented window is `25 s`, so every barrel gets two firing
opportunities and no more. Hundreds was never the number. The cadence has one real error — the
host's barrel count comes from the wrong field — and that error is worth a factor of two or three,
in **both** directions depending on the mount.

## 1. `CanFire`, every conjunct

`00729A80` `BSP_Gun_CanFire`, `__thiscall(gun)(bool checkReload) -> bool`, body
`00729A80`-`00729B88`, two `XOR AL,AL` failure epilogues and one `MOV AL,1` at `00729B83`, all
`RET 4`. `docs/GUN_AIMING.md` already listed the eight tests; this reading confirms that list from
the listing and settles the three it left open (`gun+358h`, `006D1E50`, the pair at test 7).

| # | Site | Must hold | What the host models |
| --- | --- | --- | --- |
| 1 | `00729A83`-`00729A96` | `[[gun+3F8h]+34h] != 0` — the selected fire-parameter record carries a bullet class | `fire_params_armed = true`, a faithful constant: every gun the host builds came from a `Bullet[1]` row |
| 2 | `00729A9E` | `gun+3B8h == 0` | `disabled = false` |
| 3 | `00729AA7` | `gun+358h <= 0`, signed | `damage_counter = 0`; the host has no gun damage state |
| 4 | `00729ABC`-`00729AD4` | `!checkReload \|\| (gun+450h <= 0 && gun+478h <= 0)` | both live: `barrel_delay_time`, `secondary_delay` |
| 5 | `00729ADD`-`00729B0A` | `!006D1E50(kind) \|\| [[gun+3F0h]+6F8h] <= 0` | **not modelled**: `unit_cooldown_applies = false` |
| 6 | `00729B0C`-`00729B26` | `kind != 7 \|\| [[gun+3F0h]+6FCh] <= 0` | structurally present; the cooldown input is always `0` |
| 7 | `00729B2A`-`00729B51` | `muzzle.y >= 1.0` **or** (`!00728A90(gun)` **and** `!005459B0(gun)`) | `muzzle_world_y` is live; both predicates hard-coded `false` |
| 8 | `00729B53`-`00729B7D` | `gun+448h > 0` **and** some barrel `i` has `!checkReload \|\| gun[+414h][i] <= 0` | live, and it is the only path that returns true |

Notes the listing settles:

- The `NEG`/`SBB`/`TEST ECX,0E19983h` at `00729A8C`-`00729A96` is the compiler's `!= 0`: `SBB` makes
  `ECX` all-ones or zero, and any non-zero mask then decides the branch. Test 1 is a plain
  null check, not a flag test.
- `kind` is `[[gun+3F4h]+80h]`, the device row's `Function` id (`docs/GUNNERY_TABLES.md`).
  `006D1E50` takes it in `ECX` and answers true for `{2,3,4,6}` — `LIGHTARTILLERY`,
  `MEDIUMARTILLERY`, `HEAVYARTILLERY`, `LIGHTARTILLERYFLAK`, i.e. the artillery set.
  `005459B0` is the same predicate reading the kind off the gun itself; `00728A90` is the
  anti-air set `{1,5,6}`. Their union is `{1..6}`, so test 7 reads: **a gun of any kind from
  1 to 6 whose muzzle is below `y = 1.0` (`00D7A24C`) cannot fire.** Kind `0` (`PLANEGUN`)
  and kinds `>= 7` are exempt.
- `00427EB0` `BSP_EntityPose_GetWorldPositionRefreshed` refreshes when `gun+0C8h == 0` and
  returns `gun+0FCh`; `[EAX+4]` is the `y` component.
- **Ammunition is not a `CanFire` conjunct.** It is enforced after the shot, in `0072D520`
  (section 2). A gun whose provider is empty fires once more and then holds a barrel timer of
  `FLT_MAX` forever.
- `gun+358h` is the destroyed-state level `docs/GUN_PLATFORM_ARC.md` established, not a counter.
  `BSP_Gun_Fire` lets a damaged gun through (`00730198`-`007301B6`) when `[00E188A8]+1FE4h == 2`;
  `CanFire` refuses it unconditionally. So a damaged gun fires only on a path that reaches
  `00730160` without `CanFire` — `GunForceFire`, or the `0B0h` message.

`00727E30` `BSP_Gun_FireIfReady` is `vtable[1D0h](1)` then `vtable[1D8h](0, 0.0f, 0.0f)`:
the automatic path always asks with `checkReload = 1`.

## 2. The native cadence

### Per shot, in `BSP_Gun_Fire`'s tail

| Site | Rule |
| --- | --- |
| `007302C4` | `007298D0(gun, gun+44Ch)` picks the barrel: a forward scan modulo `gun+448h` for the first timer `<= 0`, returning the **unmodulated start index** when none is found |
| `007309E8` | `0072D520(gun, gun+44Ch, 1)` |
| `007309ED`-`00730A0E` | `gun+44Ch = (gun+44Ch + 1) % gun+448h` |
| `00730A05` | `gun+450h = [gun+3F8h]+30h`, the authored `BarrelDelayTime` |

One `Fire` call spawns **one** projectile. The salvo fields `class+0CCh`
`OneTimeBulletAmount` and `class+0D0h` `MultiBulletConeAngle` exist, but
`OneTimeBulletAmount` appears **zero** times in this installation's arcade device table and
`MultiBulletConeAngle` once, so in this data a salvo is one shot. That is a data property.

### `0072D520` `BSP_Gun_RearmBarrel`, `__thiscall(gun)(int barrel, bool applyReload)`, `RET 8`

| Site | Rule |
| --- | --- |
| `0072D531` | `[gun+3F0h]->vtable[1F4h](gun+3F4h)` — is a round available? |
| `0072D592`-`0072D5A5` | if not: `0072CF00(barrel, *00CFDBF8 = FLT_MAX, 0)`. The barrel never comes back |
| `0072D54C` | if so: `vtable[1F8h](gun+3F4h)` consumes the round |
| `0072D555` | `applyReload == 0` returns here with no timer written |
| `0072D579` | `t = 00BD2F10([gun+3F8h]+28h, [gun+3F8h]+2Ch)` |
| `0072D589` | `0072CF00(barrel, t, 1)` |

`00BD2F10` is `__fastcall(int kind)(float lo, float hi)`, `RET 8` — proved from `0072D3B3`,
where only eight bytes are pushed and the epilogue two instructions later pops the two
callee-saved registers with no `ADD ESP`. The `PUSH 1` at `0072D565` is therefore `0072CF00`'s
third argument pushed early, not a third argument to the random helper.

### `0072CF00` `BSP_Gun_SetBarrelReloadTimer`, `(int barrel, float t, int setFull)`

`0072CF19`-`0072CF6F`: when `t < *00D7A278` (a double holding `FLT_MAX`, so the sentinel is
excluded) and the two mode guards `[00E0C978]` and `[[00F88C30]+0E8h]` are both set,
`t /= 008E6430(8, [gun+3F0h])`, a per-unit modifier product that starts at `1.0`.
`0072CF88` stores `gun[+414h][barrel] = t`; `0072CF95` mirrors it into `gun[+418h][barrel]`
when `setFull`.

### Per fixed step, `0072D130`

`0072D1A7`/`0072D1B5` decrement `gun+478h` and `gun+450h` unconditionally. After the three-way
gate, the loop `0072D1ED`-`0072D22D` advances **only timers that are not already negative**
(`COMISS`/`JC` at `0072D1F8`/`0072D205`) through `0072D21B` `0072CF00(i, t - dt, 0)`. A set
`gun+454h` then emits one `0ADh` message per step, and the handler turns each into
`FireIfReady`. `docs/GUN_PLATFORM_ARC.md` already put it exactly right: the rate limiting is
entirely in `CanFire`'s timers, not in the sender.

### The two authored numbers, and where they come from

`FUN_007313E0` `BSP_GunClass_ReadBulletLuaFields` builds the `48h`-byte fire-parameter record:

```
GetByName("ReloadTime")
if (!IsNumber())  { +28h = ReloadTime[1];  +2Ch = ReloadTime[2]; }
else              { +2Ch = +28h = (float)value; }
GetByName("BarrelDelayTime");  +30h = value
```

**Every `ReloadTime` in this installation's arcade device table is a number**, so
`00BD2F10(R, R) = R` and the native reload is deterministic here. `docs/GUN_DISPERSION.md`'s
`ReloadTime[1]`/`ReloadTime[2]` labelling is right about the layout and describes a branch this
data never takes.

`gun+448h` is **not** a Lua field. `0072E713` in the gun constructor calls `0072AB80(gunClass)`
= `max(1, (class+0A0h - class+9Ch) / 0Ch)`, the size of the class's muzzle vector, which
`00732560` copies verbatim from the model's `"fire"` node group
(`docs/GUN_MOUNT_POSITIONS.md`). It is a **model** property.

`FUN_0072D5D1` `BSP_Gun_SelectBulletRecord` sets `gun+3F8h = [[gun+3F4h]+74h] + idx*48h` and
`gun+3B4h = [gun+3A8h] + idx*14h`, so a gun that switches ammunition type switches its
`ReloadTime` and `BarrelDelayTime` with it. `docs/GUNNERY_CANDIDATE_ORDER.md`'s
`[gun+3F4h]+74h + (kind == 6 ? 48h : 0)` is the same stride seen from the candidate test.

### The steady state

For a gun holding the latch, with `M = gun+448h` barrels, reload `R` and barrel delay `D`:

```
opening burst : M shots, spaced D apart, because every timer starts at 0
steady rate   : min(1/D, M/R) shots per second
```

`CanFire` test 8 asks whether *some* barrel is ready while `Fire` takes the *first ready barrel
from the cursor*, so the two agree and the long-run rate is `M/R` whenever `D <= R/M`, which
holds for every artillery row in this data.

Independent corroboration from the data side: the arcade device table this installation ships
opens with the mod author's own helper

```lua
function RPM_RT(RPM, BarrelNum)  return BarrelNum / (RPM/60)  end
```

which is `R = M / rate` — the same relation, written by someone tuning against the engine.

## 3. Divergences against `src/game_hosts_gunnery.cpp`

| # | What | Native | Host | Direction |
| --- | --- | --- | --- | --- |
| 1 | **barrel count** | muzzle-node count of the class (`0072AB80`) | number of `Bullet[i]` sub-tables in the device row (`game_hosts_gunnery.cpp:434`) | **both ways.** Iowa/South Dakota 16'' **3X** authors one `Bullet` row: host 1 vs native 3, host fires **1/3**. A Mahan 5''/38 single mount authors two `Bullet` rows (two ammunition types): host 2 vs native 1, host fires **2x** |
| 2 | **settle before the trigger** | `006DF520` step 12 (`006DFB8C`, `006DFBB6`) arms when `006DEE40` says both axes are inside `*00CF9054 = 0.0017453` rad, **0.1 degree** | `gun_aim_settled_0085ae4a`, `kGunAimDeadBand = 0.00017453` rad, **0.01 degree** — the dead band from inside the *stepper* `0085AD80`, not a fire gate | host **less**, by a factor of ten per axis |
| 3 | the artillery bot's fire path | `006DF520` arms `delayedFire`, counts `fireDelayTime = 00BD2F10(0, 0.1)` down and calls `gun->vtable[1F0h]` once | the `0072D2C0` latch, one `0ADh` per fixed step, `FireIfReady` each time | host asks `CanFire` far more often; the net is bounded by the timers either way. `006FDF60`, slot `1F0h`, is **contract: unread**, so the end of the native path is not established here |
| 4 | `CanFire` test 5 | `[[gun+3F0h]+6F8h]` gates every artillery kind | `unit_cooldown_applies = false` | host **more**, by whatever `00953CC0` seeds |
| 5 | `CanFire` test 7 | `00728A90` and `005459B0` | both `false` | host **more**, and only for a muzzle below `y = 1.0` |
| 6 | reload scaling | `t /= 008E6430(8, unit)` | not modelled | **neutral** while no modifier is active: the product starts at `1.0` and the two mode guards are clear in a single mission |
| 7 | ammunition | empty provider pins the fired barrel at `FLT_MAX` | not modelled | **neutral** here: every artillery and AA row in this data authors `Ammo = 9999` |
| 8 | ammunition-type selection | `0072D5D1` re-points `gun+3F8h`, so reload and barrel delay follow the selected record | always `Bullet[1]` | **neutral** for reload on IJN01 (both records author the same `ReloadTime`); `BarrelDelayTime` differs by `0.1 s` on some dual-purpose rows |
| 9 | `ReloadTime` authored as a pair | `+28h`/`+2Ch` = `ReloadTime[1]`/`[2]` | `num()` returns nil for a table, so `reload` becomes `0` (`game_hosts_gunnery.cpp:288`) | **latent**, not live: no row in this installation authors a pair. A gun with `reload = 0` would fire every fixed step |
| 10 | reload draw | `00BD2F10(+28h, +2Ch)` per shot | fixed `gun.reload_time` | **faithful here**, because the reader writes the same scalar to both slots |

Everything else in the fire chain checks out against the listing: `gun_can_fire_00729a80`
reproduces all eight conjuncts in order including the `!check_reload` early exit inside the
barrel loop, and `gun_fixed_step_tick_0072d130` reproduces the two unconditional decrements, the
three-way gate and the `>= 0` guard on the barrel advance.

## 4. The expected-shot bound

Run: `build/win32/Release/bsp_game.exe --frames 700 --press-start-frame 30 --menu-select IJN01
--mission-frames 500 --mission-frame-seconds 0.05 --log local/cadence_run.log --xlive-dll
build/win32/Release/xlive_stub.dll --game-root <install>`, exit 0. It reproduces
`docs/AA_VERTICAL_WINDOW.md`'s numbers exactly — `targeted refusals=12058 no_accept=12058
no_settle=1828 no_window=0`, `arc_blocks=1174`, cat 4 `45/418/29`, cat 6 `72/143/40` — and adds
`trigger_rises=52 fire_messages=7628 fire_if_ready=7628 can_fire_refusals=7559 shots=69`.

The HEAVYARTILLERY that fire on IJN01 are two device rows, resolved through the vehicle table:

| Unit | Vehicle | Device | `Bullet` rows | `ReloadTime` | `BarrelDelayTime` | Comment |
| --- | --- | --- | --- | --- | --- | --- |
| `Maryland`, `West Virginia` | 315 `Colorado 1941` | 205 | 1 | `17.5` | `0.6` | `PACK 3 Colorado 16'' Battery 2019` |
| `Massachusetts`, `Iowa` | 321 `South Dakota 1920` | 516 | 1 | `17.5` | `0.6` | `South Dakota 1920 16'' 3X` |

With `M = 1`, `R = 17.5` and a 25 s window, an engaged gun fires once when it settles and once
`17.5 s` later: **two shots**. The per-gun table prints exactly `2` for all thirteen
continuously engaged rows and `1` for the five that engage after `t = 7.5 s` or lose the target.
**The host's own arithmetic is reproduced to the shot.**

With the native `M` — 2 for a twin turret, 3 for the South Dakota triple — the same guns would
fire `4` and `6`. Scaled over the same engaged rows that is roughly **60 to 90 shots, not
hundreds**, and the same calculation for cat 6 runs the other way: `Downes`/`Cassin` platforms
4 and 5 carry single 5'' mounts with two ammunition rows and `R = 5.0`, so the host's `M = 2`
prints `10` shots each where a single-muzzle mount would fire `1 + floor(24.6/5) = 5`.

**29 is faithful in shape and low by the barrel-count factor.** There is no missing gate. The
reason HEAVYARTILLERY lands tens of shots is arithmetic: `17.5 s` of reload inside `25 s` of
mission admits two firing opportunities per barrel.

The 98-99% `can_fire_refusals` rate is required, not a symptom. A gun that holds the latch and
fires every `17.5 s` must refuse 349 of every 350 ticks at 20 Hz. `7628` fire messages over `52`
latch rises is an average held latch of `147` ticks = `7.3 s`, **shorter than one reload**, which
is why an engaged gun usually gets its opening burst and nothing more inside one episode.

`trigger_rises = 52` also closes the fire-start stagger: `gun+478h` is seeded 52 times at up to
`0.12 s` (`00CE81A8`) across a mission of `606 x 25 = 15150` gun-seconds. The stagger costs at
most `6.2` gun-seconds in total and is not why anything fails to fire.

## 5. The `no_settle = 1828` verdict

**Yes, the reconstruction's settle test is too strict — by exactly ten times per axis — and it is
taken from the wrong routine.**

`0085AE4A` is an early-out *inside* the angle stepper `0085AD80`: "the gun is already where it was
told to point, skip the platform call". `src/game_hosts_gunnery.cpp:1400` reuses it as a
`want_fire` conjunct, which inverts what it is for. HEAVYARTILLERY is kind `2,3,4,6`, which
`0072C6A0` maps to the `ArtilleryGunnerBot` slot `gun+398h` (`docs/GUN_BOT_TICKS.md` section 3),
whose tick is `006DF520`, and that bot's own arming gate is `006DEE40(gun+480h, h, *00CF9054)`
with `*00CF9054 = 0.0017453` rad. The host uses `0.00017453`.

Three qualifications, because the number is not the shot-count answer:

1. The native gate is on the **commanded** angles at the moment of arming, in the bot, not on the
   gun's rotation state in the fire path. `CanFire` has no angle test at all; `0085A830`'s test 3
   tests the *current* angles against the arc window, which is `arc_blocked`, a different counter.
2. `1828` lost gun-ticks out of `12058 + 1828` targeted refusals is `13%`. A gun that misses its
   window on one tick takes it on the next; the binding constraint is the `17.5 s` reload.
3. So correcting `kGunAimDeadBand`'s use here is a **faithfulness** fix. It is not proposed as a
   way to raise the shot count, and it would not raise it much.

## 6. Proven versus assumed

**Proven from the listing or the data**

- `CanFire`'s eight conjuncts and their order, from `00729A80`-`00729B88` read instruction by
  instruction; the three predicate bodies `006D1E50`, `00728A90`, `005459B0` in full.
- `Fire`'s tail: barrel pick, rearm, cursor advance, `gun+450h` reload; `0072D520`'s two arms and
  the `FLT_MAX` sentinel; `0072CF00`'s store, mirror and modifier divide; `0072D130`'s two
  decrements and guarded barrel advance.
- `00BD2F10`'s two-float `RET 8` shape, from the stack balance at `0072D3B3`.
- `007313E0`'s scalar-versus-pair branch for `ReloadTime`, and that this installation authors only
  scalars (554 `ReloadTime` occurrences, all numeric or `RPM_RT(...)` calls).
- `gun+448h`'s producer chain `0072E713` -> `0072AB80` -> the class muzzle vector.
- The run numbers in section 4, from a run this worktree made.

**Assumed, or read from a contract**

- That a `3X` mount's model carries three `"fire"` nodes. The **mechanism** is proven — `gun+448h`
  is the size of the muzzle vector and the vector is a verbatim copy of the `"fire"` group — but
  the group's contents are the model/resource packet's contract and were not read. The `2X`/`3X`/
  `4X` in the device `Comment` strings and the author's `RPM_RT(RPM, BarrelNum)` helper are
  corroboration, not a recovered source. **The factor of 2-3 in divergence 1 is therefore a
  well-supported estimate, not a measurement.**
- `006FDF60`, gun vtable slot `1F0h`, the immediate fire `006DF520` actually calls: **contract:
  unread**. Divergence 3's net direction is unresolved because of it.
- `[gun+3F0h]->vtable[1F4h]`/`[1F8h]`, the ammunition provider pair: call shape only.
- `008E6430`'s modifier list contents; only its `1.0` starting value and its two mode guards were
  read.
- `[00E188A8]+1FE4h == 2` is taken from `docs/GUN_PLATFORM_ARC.md`; this packet re-read the
  `00730198`-`007301B6` branch but not what writes the global.

**Coverage**

| Routine | Body | Coverage |
| --- | --- | --- |
| `00729A80` | `00729A80`-`00729B88` | complete |
| `00727E30` | `00727E30`-`00727E64` | complete |
| `0072D520` | `0072D520`-`0072D5AD` | complete |
| `007298D0` | `007298D0`-`00729913` | complete |
| `0072AB80` | `0072AB80`-`0072ABD1` | complete |
| `006D1E50` | `006D1E50`-`006D1E6C` | complete |
| `00728A90` | `00728A90`-`00728AB3` | complete |
| `005459B0` | `005459B0`-`005459D8` | complete |
| `006DEE40` | `006DEE40`-`006DEE80` | complete |
| `00427EB0` | `00427EB0`-`00427EC8` | complete |
| `0072D2C0` | `0072D2C0`-`0072D3E8` | complete |
| `0072D130` | `0072D130`-`0072D2BE` | complete for the two decrements and the barrel loop; the effect-reference arm at `0072D14D` was not re-read |
| `0072CF00` | `0072CF00`-`0072D129` | partial: `0072CF00`-`0072CF9A`, the scale and the two stores. `0072CF9A`-`0072D129` (the per-barrel attached objects at `gun+3E4h`) unread |
| `00730160` | `00730160`-`00730A1B` | partial: the head `00730160`-`007301BC` and the tail `007309C4`-`00730A1B`. The middle is `docs/UNIT_WEAPON_DEVICES.md`'s contract |
| `007313E0` | `007313E0`-`00731A00` | partial: `Throw`, `TracerRatio`, `ReloadTime`, `BarrelDelayTime` and the start of `FireEfx`. The rest of the `48h` record unread |
| `0072D5D1` | `0072D5D1`-`0072D6DB` | partial: `0072D5D1`-`0072D61C`, the two pointer re-points. The tail unread |
| `008E6430` | `008E6430`-`008E649D` | partial: the `1.0` seed and the loop head; the modifier combination unread |

## 7. What this packet did not publish

No header and no source. Every rule this reading touches already exists and is already faithful:
`gun_can_fire_00729a80` and `gun_can_fire_turning_0085a830` in `include/bsp/gun_aiming.hpp`,
`gun_fixed_step_tick_0072d130` and `gun_set_fire_request_0072d2c0` in
`include/bsp/gun_platform_arc.hpp`, `next_ready_barrel_007298d0` and
`barrel_reload_write_0072cf00` in `include/bsp/unit_weapons.hpp`. The findings are two input
errors in `src/game_hosts_gunnery.cpp`, which the integrator owns, and a bound. Publishing a
`gun_shot_cadence` module would have added a modelling artefact with no native routine behind it.

**For the integrator, in priority order:**

1. `gun.barrel_num` (`game_hosts_gunnery.cpp:434`) is the count of `Bullet[i]` sub-tables. The
   native `gun+448h` is the model's `"fire"` node count. These are different quantities and the
   host's is wrong in both directions. The Lua device table **cannot** supply it; either take it
   from the model (the model packet's contract) or label the field a stand-in in the header and in
   the run log, so no later reading treats a cat-4 or cat-6 shot count as a cadence result.
2. `settled` (`game_hosts_gunnery.cpp:1400`) should be the artillery bot's `0.0017453` rad
   (`00CF9054`), not the stepper's `0.00017453` (`00CFAA48`). Same test, different constant, and
   `0085AE4A`'s dead band belongs only inside `gun_step_aim_0085ad80`.
3. `f['reload']` (`game_hosts_gunnery.cpp:288`) silently becomes `0` if a mod authors
   `ReloadTime = {min, max}`, which `007313E0` handles. A `type(v) == 'table'` arm would close it.

## 8. Follow-up packets

- **`gun_immediate_fire_006fdf60`** — read gun vtable slot `1F0h` on `MRTGun` (`00CFBF58+1F0h`).
  It is the call `006DF520` and `008FFF20` actually make, and until it is read the host's use of
  the `0072D2C0`/`0ADh` latch path for artillery cannot be judged.
- **`gun_class_muzzle_group`** — the `"fire"` node group `00718870` builds and `00732560` copies.
  It is the only source for `gun+448h`, and it settles divergence 1 from a measurement rather than
  from a `Comment` string.
- **`unit_ammunition_provider`** — `[gun+3F0h]->vtable[1F4h]`/`[1F8h]` and `0081F8B0`, the routine
  that re-arms barrels sitting at the `FLT_MAX` sentinel. Dormant on this data (`Ammo = 9999`)
  but the whole ammunition model hangs off it.
- **`unit_fire_cooldown_6f8h`** — what `00953CC0` seeds into `unit+6F8h` and `unit+6FCh`.
  `CanFire` tests 5 and 6 are the two conjuncts the host does not model at all.

## 9. Installation

`I:/SteamLibrary/steamapps/common/Battlestations Pacific`. **This installation is modded**
(BSPRM/AlterBSP). `scripts/datatables/classtables/arcade/deviceclasses.lua` has mtime
`2026-05-09 22:37` and carries the author's `RPM_RT`/`RPM_DT` helpers at its head;
`classtables/realistic/deviceclasses.lua` and `autoload/deviceclasses.lua` are both
`2024-07-13 08:26`. `autoload/deviceclasses.lua` selects arcade unless `GameMode == 1`.
Every number in sections 2 and 4 comes from the arcade table, which is the modified one. The
installation was only read.
