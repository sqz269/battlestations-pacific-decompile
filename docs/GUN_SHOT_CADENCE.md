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

## Integration result: the settle gate was holding the count down after all

Divergence 2 is fixed. `src/game_hosts_gunnery.cpp` used `gun_aim_settled_0085ae4a`, whose
`kGunAimDeadBand = 0.00017453` rad is the **stepper's** dead band from `0085AD80`, as the gate
deciding whether a gun may fire. The native fire gate is `006DF520` step 12 through `006DEE40`
against `*00CF9054 = 0.0017453` rad. Both constants verified from the image: `00CFAA48` is
`d4 02 37 39` = 0.01 degree and `00CF9054` is `89 c3 e4 3a` = 0.1 degree, a clean factor of ten.

Measured on the 3200-frame USN02 line:

| | shots | entity impacts | hit rate | total damage | first shot |
| --- | --- | --- | --- | --- | --- |
| before | 273 | 167 | 61.2% | 16472.1 | 35.60 s |
| after | **729** | **181** | **24.8%** | **18490.1** | **1.40 s** |

**This packet's integrator predicted the change would not move the count materially**, on the
grounds that `no_settle` was 13% of targeted refusals and the count is held by the authored 17.5 s
reload. That prediction was wrong: the count went up 2.7x. The reload does bound how often a *given*
barrel fires, but the over-strict settle gate was delaying the first shot of every engagement and
costing whole reload cycles - `first_shot` moves from 35.60 s to 1.40 s, so each gun gains most of a
minute of firing time. The magnitude also agrees with this packet's own IJN01 estimate, which put
HEAVYARTILLERY at 60-90 against an observed 29, a 2-3x factor.

**The hit rate falls from 61.2% to 24.8%, and that is the expected direction.** A gun that fires
when it is within 0.1 degree of its commanded angle is aiming less precisely than one that waits for
0.01 degree. The native gate is the looser one, so the lower per-shot accuracy at higher volume is
faithful rather than a regression; total damage still rises from 16472.1 to 18490.1.

**A process note.** The first attempt at this fix replaced the whole predicate with a plain
subtraction instead of changing only the constant, dropping `wrapped_angle_subtract_00438b10` and
the strict comparison. That version gave 697 shots and a `queued_hits = 181` against `hull = 180`
mismatch that no other run has shown. It was caught because the result contradicted the prediction
above and prompted a re-read of the original rather than banking the number. Only the constant
differs now.

## 10. The immediate-fire slot `vtable[1F0h]` (packet `cc9_mrtgun_immediate_fire`, switch `kGunImmediateFireSlotBound`)

This section answers section 8's `gun_immediate_fire_006fdf60` and divergence 3. Ghidra has no
function at the slot bodies, so they were read from the disk bytes (`bsp.py disasm-raw`). The
`+310h` sub-object vtables were read from the PE on disk.

### 10.1 What the slot does

**MRTGun** (`Rapid_Turning_Gun`, class `24h`, built by `00731E20`, primary vtable `00CFBF58`,
`+310h` vtable `00CFBF10`):

```
006fdf60: mov byte ptr [ecx + 0x4d4], 1        ; slot 1F0h: raise gun+4D4h
006fdf67: ret
```

- **The reader is `0084C5B0`**, slot 8 of the `+310h` vtable. It is the class's per-step tick,
  with `ESI = gun+310h`:

```
0084c5d4: CALL 0x0072d130                      ; the base tick (latch -> 0ADh)
0084c5db: CMP byte ptr [ESI + 0x1c4],BL        ; gun+4D4h
0084c5e3: PUSH 0xad                            ; build message 0ADh ...
0084c625: CALL 0x0077c2a0                      ; ... and route it on the gun (ESI-310h)
```

  The `0ADh` arm (`0072D860`) is `vtable[1DCh]()`, which is FireIfReady and ignores the payload.
  So while `+4D4h` is set, the gun asks to fire every step. That request does **not** go through
  `0072D130`'s three gates, and it does not need the latch `+454h`.
- **The only clear is `006FDC90`**, MRTGun's `vtable[1E8h]`. It calls `0072D2C0(want)` and then
  zeroes `+4D4h` when `+454h` came back clear.
- **The scan:** `D4 04 00 00` occurs in 70 places, and only these three are in gun code
  (`006FDCA8`, `006FDCF4`, `006FDF62`, all writes). The reader goes through the sub-object
  (`C4 01 00 00` at `0084C5DD`).

**MSTGun** (`Single_Turning_Gun`, class `27h`, built by `006FE390`, primary vtable `00CFC190`,
`+310h` vtable `00CFC14C`):

```
006fdc50: mov eax, [ecx]; mov edx, [eax+1E8h]; push 1; call edx; ret   ; slot 1F0h = vtable[1E8h](1)
```

- **Its tick is `006FE0D0`** (`+310h` slot 8). It runs `0084C5B0` first. Then, when the latch
  `gun+454h` is set, it takes the muzzle count `M = (desc+A0h - desc+9Ch) / 0Ch`:

```
006fe117: mov ecx, [esi+13Ch]                  ; the barrel cursor gun+44Ch
006fe11d: add eax, -1
006fe120: cmp ecx, eax                         ; cursor == M - 1 ?
006fe136: push 0 ; call [vtable+1E8h]          ;   yes: request off
006fe13a: mov byte ptr [esi+1C8h], 0           ;        gun+4D8h = 0
006fe149: mov byte ptr [esi+1C8h], 1           ;   no, and cursor != 0: gun+4D8h = 1
```

- **`006FDCD0`, MSTGun's `vtable[1E8h]`, skips a false request while `+4D8h` is set.** Otherwise
  it behaves like `006FDC90`.

**Who calls slot 1F0h:**
- **The ArtilleryGunnerBot** `006DF520` (GUN_BOT_TICKS). It never calls `vtable[1E8h](1)`. Step 12
  arms `delayedFire` when the solver and `0085ABA0` succeeded and both angles are within 0.1 degree.
  Step 13 draws `fireDelayTime = U(0, 0.1)`. Step 15 counts it down in the same tick
  (`006DFC37..006DFC61`) and calls `vtable[1F0h]` on `t < 0` (`fldz; fcompi; jbe`). It lowers the
  request only through `006DF4C0` when the target clears.
- **The TorpedoBot** `008FFF20`. It re-decides every 0.2 s and sends `vtable[1E8h](0)` on every
  abort, so the host's per-step request already matches it. It is not changed here.

### 10.2 The divergence

With the switch OFF, the host feeds `want_fire = target && accepted && settled && window` into the
latch every step, for every gun. In the image:
- **An artillery-bot gun raises its request after settling plus a `U(0, 0.1)` delay, and holds it
  while unsettled.** Only the MSTGun salvo test or a target clear lowers it. The window is still
  enforced, by CanFire (`0085A830`).
- **Every MSTGun, whichever bot drives it, drops its latched request once the cursor reaches the
  last muzzle.** A guarded false request is ignored while `+4D8h` is set.

### 10.3 The binding (committed OFF)

`kGunImmediateFireSlotBound` in `src/game_hosts_gunnery.cpp`:
- **The turning class** comes from the device row's `Type`: `Rapid_Turning_Gun` is 24h and
  `Single_Turning_Gun` is 27h. It is read into `gun_turning_class`.
- **For those two classes,** the request goes through the class `vtable[1E8h]`. Guns in an
  artillery-bot category (2, 3, 4, 6, 9, against a non-plane) run steps 12, 13 and 15 in place of
  `want_fire`. The new stream `Draw::artillery_fire_delay` (18) supplies the draw.
- **MRTGun's extra `0ADh`** turns the step's fire attempt on while `+4D4h` is set.
- **MSTGun's salvo test** runs at the top of the gun's next step, before its bot. The bot is the
  next reader of the latch, so the order is the image's.
- **Census line:** `summary mission gunnery immediate fire ...`.
- **Labelled:**
  - `0ADh` is taken as delivered in the same step, as the host already does for `0072D130`.
  - One fire attempt stands for two sends in a step. A second FireIfReady after a shot is refused
    by the barrel delay.
  - The MSTGun count uses the host's `barrel_num` (at least 1) where the image would compare
    against `-1` for a model with no muzzle node.

### 10.4 Census (OFF runs, `local\IF_OFF_<m>.log`)

| mission | MRTGun, artillery-bot categories / other | MSTGun, artillery-bot categories / other |
| --- | --- | --- |
| USN02 | 0 / 204 | 134 / 120 |
| USN04 | 0 / 259 | 106 / 64 |
| USN01 | 0 / 129 | 41 / 46 |
| USN13 | 0 / 1068 | 261 / 194 |

**No MRTGun on these missions has an artillery-bot category,** so `006FDF60`'s sticky flag is never
raised here. The switch acts through the MSTGun half: the artillery bot's delayed raise, and the
salvo test on every MSTGun (artillery, torpedo, single AA and depth-charge mounts).

### 10.5 Predictions, written before any ON run

| row | OFF (`IF_OFF_<m>`) | prediction ON |
| --- | --- | --- |
| all four: `extra_sends`, `flag_drops` | - | **0** (no MRTGun is armed by the artillery bot) |
| all four: `slot_calls`, `arms`, `salvo_drops` | - | each above 0 |
| USN02 9200/9000 | 26 deaths, 847 hit records, 1117 shots, first hit 41.05 s, failed 212.91 s | shots within -15%..+5% (the salvo test re-raises through fresh staggers, and torpedo tubes lose the last tube of a spread); first hit **same or later**, by at most 0.15 s; deaths 26 +- 4; pair_diff exit 3 |
| USN04 4700/4500 | 40 / 644 / 5333, first hit 92.50 s | shots down 0..10% (64 non-artillery MSTGuns re-stagger); first hit same or later by at most 0.15 s; deaths 40 +- 3; exit 3 |
| USN01 3200/3000 | 5 / 178 / 623, first hit 53.75 s | shots within -10%..+5%; deaths 5 +- 1; exit 3 |
| USN13 3200/3000 | 23 / 572 / 4265, first hit 67.90 s | shots down 0..10%; deaths 23 +- 3; exit 3 |

**The direction of the first shot is the one firm prediction.** A gun's first request now waits
for the `U(0, 0.1)` delay after its first settle. The first stagger draw per gun is unchanged,
because the stream is keyed per gun, so no gun fires earlier than OFF.

### 10.6 The pairs, and the flip

OFF is `local\IF_OFF_<m>.log`: this tree at `5532a16cd`, copied to `local\if_off_bin`. ON is
`local\IF_ON_<m>.log`: `pair_export --commit 5532a16cd --flip kGunImmediateFireSlotBound=true`
into `local\if_on`. RNG streams and the death table were on, with lockstep 0.05. The OFF build
equals the head without the switch: `pair_diff` against `local\g4_dr_usn04.log` exits 1.

| mission | OFF deaths / hit records / shots, first hit | ON | census (ON) | prediction | verdict |
| --- | --- | --- | --- | --- | --- |
| USN02 9200/9000 | 26 / 847 / 1117, 41.05 s, failed 212.91 s | **9 / 4597 / 3666, 33.35 s, failed 34.70 s**; pair_diff 3 | arms 111400, slot calls 111391, salvo drops 92516, ignored false 5829, extra sends 0 | shots -15..+5%, first hit same or later, deaths 26 +- 4 | **failed** on shots, first hit and deaths |
| USN04 4700/4500 | 40 / 644 / 5333, 92.50 s | 41 / 633 / 5345, 92.50 s; pair_diff 3 | arms 0, salvo drops 16219 | shots down 0..10%, deaths 40 +- 3 | deaths and first hit held; shots **+0.2%**, a marginal fail |
| USN01 3200/3000 | 5 / 178 / 623, 53.75 s | 5 / 177 / 623, 53.75 s; pair_diff 3 | arms 0, salvo drops 2368 | shots -10..+5%, deaths 5 +- 1 | held |
| USN13 3200/3000 | 23 / 572 / 4265, 67.90 s | 23 / 552 / 4222, 67.90 s; pair_diff 3 | arms 0, salvo drops 6494 | shots down 0..10%, deaths 23 +- 3 | held (-1.0%) |
| all four | - | extra sends 0, flag drops 0 | - | 0 | held |

**What moved USN02.** The artillery half, in categories 2 and 6, per the gun rows:

| category | shots OFF -> ON | latch rises OFF -> ON |
| --- | --- | --- |
| 2 | 209 -> 646 | 299 -> 34246 |
| 3 | 263 -> 270 | 124 -> 8087 |
| 6 | 433 -> 2523 | 289 -> 47277 |
| 7 (torpedo) | 144 -> 227 | 126 -> 3132 |

- **The Fubuki and Dutch 5-inch dual-purpose mounts (device 299) go from single figures to about
  100..155 shots each.** Their reload is 2.7 s over 2 barrels, so 450 s allows about 330; no gun
  exceeds its reload bound.
- **With the switch OFF, a gun had to be settled on every step until the stagger (`U(0, 0.12)`,
  drawn at each rise) ran out.** Its request flickered with the settle test while the aim-error
  envelope moved the commanded angles, and each rise redrew the stagger.
- **In the image, the settle test only arms the request.** The latch then holds until the salvo
  test drops it, so the stagger runs out and the ready barrel fires.
- **So the first-shot prediction was wrong in its premise.** OFF needs a settle on the step of
  the shot, while ON needs one only at the arm, so ON guns can fire earlier.
- **The torpedo rise is downstream.** Minegumo's first spread launches at the same times on both
  sides (1.45..4.95 s). The later spreads, at 208, 328 and 448 s, follow a different battle.
  Torpedoes are not armed by this binding.
- **The USN02 outcome.** Houston is sunk at 33.80 s by Minegumo's opening spread, from 2719 m.
  Houston's path changes in the heavier opening exchange (controlled moved 3581 -> 444 m), and
  the mission fails at 34.70 s. Reference g's USN02 row and GENERATED_SHIP_AI 5's phase-2 failure
  no longer describe the head once this is ON.

**Decision: `kGunImmediateFireSlotBound` is ON.**
- The request path is the image's, read from the listings above: slot 1F0h on both classes,
  `0084C5B0`, `006FE0D0`, and the artillery bot's steps 12, 13 and 15.
- The failed rows are consequences of the settle test no longer gating every step. The mispredicted
  first-shot direction came from reasoning that assumed it did.
- The census holds: no MRTGun is armed on these missions, so `006FDF60`'s sticky flag is bound but
  unexercised here.
- **Labelled, and worth checking first if USN02 looks wrong:**
  - the same-step `0ADh` delivery;
  - the host evaluates settle after stepping the gun, where `006DF520` compares the angles before
    the gun's own step.

### 10.7 The two timing labels (packet `cc9_fire_request_timing`, sub-switch `kGunWaveOrderBound`)

Section 10.6 left two labels that set the fire rate directly. Both were read against the fixed-step
structure.

**(1) 0ADh delivery: same step in the image, as in the host. No change.**
- Both sends go through `0077C2A0` `BSP_Session_RouteMessage`. The latch path's is `0072D290`
  inside `0072D130`. MRTGun's extra one is `0084C625 CALL 0x0077c2a0`.
- **In a local session the only destination is the loopback queue `0076E520`.** It is drained by
  `00778450` `BSP_Session_PumpStep`, fan-out row 9 (docs/UNIT_STATE_MESSAGE.md "cc2-session-dispatch",
  docs/SHIP_NEIGHBOUR_AVOIDANCE.md "route" / "drain").
- **The gun's tick runs before that pump.** It is the gun node's `+8h` slot (`0084C5B0` /
  `006FE0D0`, the `+310h` vtables `00CFBF10` / `00CFC14C` at index 2). Wave 3 of the job waves calls
  it (`00874FE0`: `element->vtable[+8h](0.05f)`), and "waves 1-3 run inside the step loop, before
  the subsystem fan-out" (docs/FIXED_STEP_JOB_WAVES.md).
- **So a 0ADh posted by either path is delivered to FireIfReady (`0072D860`) at row 9 of the same
  step.** The two paths do not differ. The host's same-step FireIfReady is the image's.

**(2) Settle order: the host was one aim step ahead of the image. Corrected under the sub-switch.**
- Wave 1 calls the element's `+4h` slot, which for a turning gun is `0085AD80` (index 1 of the same
  `+310h` vtables). It steps the angles toward the command the bot set on the previous step.
- Wave 2 (`00875B90` -> `008759B0`) walks the node's sub-list. The bots are linked into it at attach
  (`008FBC8B`: `00876020(gun+310h, bot)`, GUN_BOT_TICKS). `006DF520` there sets the new command
  (`006DFB54`, `0085ABA0`) and compares the **already stepped** `gun+480h` / `+484h` with it (`006DFB83
  FLD [ECX+480h]`, `006DFBAD FLD [EDX+484h]`, `006DEE40`).
- **The host** called `0085ABA0` first, stepped toward the new command, and only then tested settle.
  So it tested one aim step later, against a gun that had already moved toward the new command.
- The same order puts MSTGun's salvo test (`006FE0D0`, wave 3) **after** the bot and after the
  step's send. The host ran it before the bot.
- **Per settle event:** the host's test passed as soon as the gun could reach the new command within
  one step. The image's passes only when the command moved less than 0.1 degree since the last
  step. The shot-count effect is measured by the pair below.
- `kGunWaveOrderBound` (OFF in `src/game_hosts_gunnery.cpp`) moves the aim step ahead of `0085ABA0`,
  for every gun as wave 1 does, and runs the salvo test after the send and before FireIfReady. The
  census line is `summary mission gunnery wave order ... pre_steps=`.

**OFF, this tree (`local\WO_OFF_<m>.log`, main `eec19cbf1` synced, with the torpedo swim ON):**

| mission | deaths / hit records / shots | first hit | end |
| --- | --- | --- | --- |
| USN02 9200/9000 | 12 / 5166 / 3826 | 19.20 s | failed 29.75 s; Houston sunk 22.75 s (Minegumo Long Lance) |
| USN04 4700/4500 | 28 / 491 / 3656 | 100.85 s | none |
| USN13 3200/3000 | 16 / 312 / 2069 | 97.00 s | none |

The twenty 5-inch dual-purpose mounts (device 299) on USN02 average 77 shots each (maximum 167).

**Predictions, written before the ON runs:**

| row | prediction |
| --- | --- |
| USN02 shots | down, 0..15% (fewer arms and later re-arms; the aim lags one step) |
| USN02 shots per device-299 mount | mean 77 -> 60..77 |
| USN02 first hit | 19.20 s +- 0.2 s (a torpedo; the tube trains one step later) |
| USN02 Houston | still sunk by an opening-spread Minegumo Long Lance, 22.75 s +- 1.5 s |
| USN02 failure | 29.75 s +- 1.5 s |
| USN04 | hit records down 0..10% (AA aim one step behind); deaths 28 +- 3; exit 3 |
| USN13 | hit records down 0..10%; deaths 16 +- 3; exit 3 |
| census | pre_steps > 0 on each ON run |

### 10.8 The wave-order pair, and the verdict: OFF until the AA fire tests are bound

OFF is `local\WO_OFF_<m>.log`: this tree at `1446b3f62`, copied to `local\wo_off_bin`. ON is
`local\WO_ON_<m>.log`: `pair_export --commit 1446b3f62 --flip kGunWaveOrderBound=true`
(`local\wo_on`). Streams and the death table were on.

| row | OFF | ON | prediction | verdict |
| --- | --- | --- | --- | --- |
| USN02 shots | 3826 | 2621 (-31%) | down 0..15% | **failed** (larger) |
| USN02 device-299 mean shots per mount | 77.0 (max 167) | 66.7 (max 159) | 60..77 | held |
| USN02 first hit | 19.20 s | 19.20 s | +- 0.2 s | held |
| USN02 Houston | 22.75 s, Minegumo Long Lance | 20.85 s, **Yamakaze** Long Lance, 2439 m | Minegumo, 22.75 +- 1.5 s | **failed** on the shooter and by 0.4 s |
| USN02 failure | 29.75 s | 29.75 s | +- 1.5 s | held |
| USN02 deaths / hit records | 12 / 5166 | 10 / 4226 | - | - |
| USN04 | 28 / 491 / 3656, first hit 100.85 s | 22 / 361 / 1571, first hit 117.55 s | hit records -0..10%, deaths 28 +- 3 | **failed** |
| USN13 | 16 / 312 / 2069 | 14 / 238 / 899 | hit records -0..10%, deaths 16 +- 3 | deaths held, hit records **failed** (-24%) |
| census | - | pre_steps > 0 on all three | > 0 | held |

**Where the shots went (gun rows by category):**

| mission | category | shots OFF -> ON |
| --- | --- | --- |
| USN04 | 1 (AA) | 3173 -> 1267 |
| USN04 | 5 | 150 -> 101 |
| USN04 | 6 | 333 -> 203 |
| USN02 | 2 | 700 -> 411 |
| USN02 | 3 | 269 -> 161 |
| USN02 | 6 | 2647 -> 1861 |
| USN02 | 7 | 210 -> 188 |

**The answers to the two labels:**
1. **0ADh delivery does not make the host fire earlier.** The image delivers it in the same step
   on both paths (10.7), as the host does. Multiplier 1.
2. **The settle order does make the host fire earlier.** It tested one aim step later than the
   image, so a moving command could pass the host's test and fail the image's. On USN02, whose
   guns are all artillery-bot guns, the image's order multiplies the shots by **0.69** and the
   device-299 mounts' by **0.87**. Neither the first hit nor the 29.75 s failure moves, because
   the failure is torpedo-driven (Houston goes to an opening-spread Long Lance either way). **The
   USN02 early failure stands as the image's own for an idle player.**

**Why the sub-switch stays OFF.**
- The drop on USN04 and USN13 is the AA guns, and it exposes a separate host divergence. The
  host gates **every** gun on `006DF520`'s 0.1-degree settle.
- The image's AA bots use their own tests:
  - `008FFA20` uses BOT_FIRE_TARGET section 3's hysteresis through the `008FEF40` debounce.
  - `009030C0` fires within **one degree** (`0090335E` / `0090339A`, `00CE3984`) (GUN_BOT_TICKS 6.1,
    6.2).
- Against a one-degree window, the one-step aim lag of the image's wave order costs almost nothing.
  Against the host's 0.1 degree, it halves the AA fire. Flipping now would add a second error to
  the AA path.
- **The next step is to bind the AA bots' own fire tests, then flip `kGunWaveOrderBound`.** The
  structure is established: wave 1 before wave 2, and the salvo test after the send.

### 10.9 The AA bots' own fire tests (packet `cc9_aa_bot_fire_tests`, switch `kAaBotFireTestsBound`)

Section 10.8 found the host gating every gun on `006DF520`'s 0.1-degree settle. The three AA bots
were read for the tests they actually apply.

**Which bot drives which gun.**
- The bot slots are fixed by sub-type (GUN_BOT_TICKS). The sub-type 1 slot takes `00902920` unless
  the gun sits under an owner of kind `0Fh` (a plane), in which case it takes `008FFA20`.
- robots.lua names them. `008FFA20`'s row reader is `TailGunnerBot`'s (`ShootRange` at `+18h`,
  `008FCAF8`), and `00902920` is `AAGunnerBot`.
- Sub-types 5 and 6 against a plane take `009030C0`, `AAFlakBot`.

**`00902920` AAGunnerBot, ship category-1 guns.** The request goes straight to `vtable[1E8h]`, with
no debounce:

```
00902fb0: call 0x85aba0 ; test al,al ; je 0x903078          ; accepted, else fire = 0
00902fcd: fld [eax+60h] ; fmul qword [00D7A390] (0.9)       ; 0.9 x the class range
00902fde: fcompi ; jbe 0x90306f                             ; fire only if distance < it
00902ffc: call 0x438b10 (gun+480h, h) ; and 7fffffffh       ; |dh|
0090302a: call 0x438b10 (gun+484h, v) ; and 7fffffffh       ; |dv|
00903044: fadd ; fld qword [00CF0098] (0.0872665, 5 deg) ; fcompi ; jbe 0x90306f   ; |dh|+|dv| < 5 deg
00903066: test byte [eax+634h],1 ; je 0x903078              ; inhibit bit 0
009030a8: jmp [edx+1E8h]                                    ; vtable[1E8h](fire)
```

**`009030C0` AAFlakBot, category 5 and category 6 against a plane.** `0085ABA0`'s answer is not
tested:

```
0090332c: fld [edx+58h] ; fld dist ; fcomi ; jbe 0x903412   ; min < distance, else fire = 0
0090333d: fld [edx+60h] ; fcompi ; jbe 0x9033db             ; distance < max
0090335e: call 0x438b10 (gun+480h, h) ; comiss [00CE3984] (1 deg) ; jbe 0x9033db
0090339a: call 0x438b10 (gun+484h, v) ; comiss [00CE3984] ; jbe 0x9033db
009033d2: test byte [eax+634h],1
00903410: jmp [edx+1E8h]
```

**`008FFA20` TailGunnerBot, a category-1 gun under a plane.** The range is `bot+90h` `shootRange`,
robots.lua `ShootRange`: 950 / 750 / 800 / 800 / 850 / 950 by skill index. The angles are the
cached request `bot+70h` / `+74h` against `gun+480h` / `+484h`:

```
008ffdbe: cmp byte [esi+58h],0 ... jnz 0x8ffe75             ; committed state picks the branch
008ffdec: fsub qword [00D7A378] (40.0)                      ; not firing: distance < range - 40
008ffe62: fld qword [00D18390] (0.1047, 6 deg) ; fcompi ; jbe  ;   and |dh|+|dv| < 6 deg -> request 1
008ffdd4: fld qword [00CE3D88] (20.0) ; fadd                ; firing: range + 20
008ffe8d: ja 0x8ffefe                                       ;   distance > range + 20 -> request 0
008ffef0: fld qword [00D18388] (0.1571, 9 deg) ; fcomip ; jbe  ;   |dh|+|dv| > 9 deg -> request 0
008fff12: call 0x008fef40                                   ; the debounce
```

**`008FEF40`, the debounce.**
- A changed request stores it at `+59h` and sets `+5Ch` to 0.1 s (`00D17D3C`) to open or 0.3 s
  (`00CE69C8`) to cease. It does not commit.
- A repeated request subtracts `dt` while `+5Ch >= 0` (`008FEF7B`). It commits `+58h = +59h` once
  `+5Ch <= 0` (`008FEF91`).
- Then it calls `vtable[1E8h](+58h)` every tick.

**None of the three tests the fire window.** CanFire (`0085A830`) enforces it.

**The binding (committed OFF).**
- `kAaBotFireTestsBound` replaces `want_fire` for those three gun kinds with the rules above. The
  distance is the muzzle to the target's aim point. The class range is `gun.max_range`, or the
  second ammunition's range for a category-6 gun firing flak. The minimum is `MinRange`.
- The census line is `summary mission gunnery aa bot fire tests ...`.
- **Labelled:** `+58h` / `+60h` taken as `MinRange` and the class range, the host's two fields for
  them.

**OFF (`local\AT_OFF_<m>.log`, this tree at the head with the wave order OFF):**

| mission | deaths / hit records / shots | first hit | plane deaths | category 1 shots / hits | category 5 | category 6 |
| --- | --- | --- | --- | --- | --- | --- |
| USN04 | 28 / 491 / 3656 | 100.85 s | 29 | 3173 / 93 | 150 / 98 | 333 / 278 |
| USN13 | 16 / 312 / 2069 | 97.00 s | 17 | 1815 / 115 | 47 / 31 | 207 / 166 |
| USN01 | 5 / 177 / 623 | 53.75 s | 6 | 583 / 123 | 16 / 24 | 24 / 30 |
| USN02 | 12 / 5166 / 3826 | 19.20 s | - | 0 | 0 | 2647 (no plane target) |

**Predictions, written before the ON runs:**

| row | prediction |
| --- | --- |
| USN04 category-1 shots | **up**, at least +30%: the 5-degree sum replaces the flickering 0.1-degree settle |
| USN04 category 5 and 6 (against planes) shots | up |
| USN04 AA hits and plane kills | up (category 1 hits above 93); deaths 28 -> at least 28 |
| USN04 first hit | same or earlier than 100.85 s |
| USN13 | category-1 shots up at least +30%, AA hits up |
| USN01 | **moves**, against the brief's identity: its convoy fires category-1 AA at the Dauntlesses (583 shots OFF). Category-1 shots up |
| USN02 | **identity**, pair_diff exit 0 or 1: no plane is ever a target |
| census | gunner and flak counts above 0 on USN04, USN13 and USN01; tail above 0 wherever a plane's rear gun has a target |

**The pairs** (ON: `pair_export --commit 661f3ad89 --flip kAaBotFireTestsBound=true`, `local\at_on`;
logs `local\AT_{OFF,ON}_<m>.log`):

| mission | OFF deaths / hit records / shots | ON | category 1 shots | 5 | 6 | AA hits (1 / 5 / 6) | plane deaths | pair_diff |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| USN04 | 28 / 491 / 3656, 100.85 s | 44 / 803 / 10090, 100.95 s | 3173 -> 9174 | 150 -> 197 | 333 -> 719 | 93/98/278 -> 128/125/535 | 29 -> 45 | 3 |
| USN13 | 16 / 312 / 2069, 97.00 s | 19 / 400 / 4152, 96.65 s | 1815 -> 3667 | 47 -> 106 | 207 -> 379 | 115/31/166 -> 90/53/257 | 17 -> 20 | 3 |
| USN01 | 5 / 177 / 623, 53.75 s | 5 / 441 / 1510, 53.60 s | 583 -> 1465 | 16 -> 20 | 24 -> 25 | 123/24/30 -> 391/27/23 | 6 -> 6 | 3 |
| USN02 | 12 / 5166 / 3826 | identical | 0 | 0 | 2647 | - | - | **1** |

Census, ON:

| mission | gunner | flak | tail |
| --- | --- | --- | --- |
| USN04 | 1007689 | 173167 | 72815 |
| USN13 | 2592000 | 158223 | 505426 |
| USN01 | 321000 | 66019 | 30932 |

| prediction | verdict |
| --- | --- |
| USN04 category 1 shots up at least 30% | held (+189%) |
| USN04 category 5 and 6 up | held |
| USN04 AA hits and plane kills up | held (kills 29 -> 45) |
| USN04 first hit same or earlier | **failed**: 0.1 s later (100.95 s) |
| USN13 category 1 shots up at least 30%, AA hits up | shots held (+102%); AA hits up in total (312 -> 400), but category 1 hits **fell** (115 -> 90) |
| USN01 moves, category 1 shots up | held |
| USN02 identity | held (exit 1) |
| census above 0 | held |

**Decision: `kAaBotFireTestsBound` is ON.**
- The three requests are the listings above.
- The rise stays within CanFire's reload timers, which are unchanged.
- The two failed rows are small and downstream of a longer AA engagement.

### 10.10 The wave order re-paired with the AA tests ON (packet `cc9_wave_order_repair`)

Section 10.8 held `kGunWaveOrderBound` OFF because it halved AA fire under the host's shared
0.1-degree gate. With `kAaBotFireTestsBound` ON (10.9), the AA guns now ask on their own 5-degree,
1-degree and hysteresis tests. OFF is the AA-ON head: `local\AT_ON_<m>.log`, which equals this
tree's build (`pair_diff` exits 0 on USN04). ON is
`pair_export --flip kGunWaveOrderBound=true` of the commit that carries these predictions.

**Predictions, written before the ON runs:**

| row | OFF | prediction |
| --- | --- | --- |
| USN02 9200/9000 | 12 / 5166 / 3826, device-299 mean 77.0 | **the same as 10.8's ON run**: USN02 has no AA target, and its OFF equals 10.8's OFF (`pair_diff` against `WO_OFF_usn02` exits 1). So pair_diff against `local\WO_ON_usn02.log` exits 0 or 1: 10 / 4226 / 2621, device-299 mean 66.7 (x0.87), Houston 20.85 s (Yamakaze), failure 29.75 s |
| USN04 4700/4500 | 44 / 803 / 10090, category 1 shots 9174 | category-1 shots within -10%..+5% (the one-step lag is small against 5 degrees); deaths 44 +- 4; hit records within +-15% |
| USN13 3200/3000 | 19 / 400 / 4152, category 1 shots 3667 | category-1 shots within -10%..+5%; deaths 19 +- 3 |
