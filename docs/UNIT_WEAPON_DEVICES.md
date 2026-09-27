# Unit weapon devices: guns, barrels, stance and firing

Addresses: 008BE440, 008BE600, 008BE7E0, 008A6950, 008A6AC0, 008A6490, 0089C590, 0089C8F0,
0089EEE0, 008BED80, 0089A8B0, 0089C360, 0088DC10, 008CF350, 00730160, 0072CF00, 0072D520,
0072E6D0, 0072ADC0, 0071BE80, 0071BED0, 0071BF20, 0071BF70, 0071DFD0, 0071E0D0, 0081F8B0,
0080E150, 007298D0.

Reconstruction: `include/bsp/unit_weapons.hpp`, `src/unit_weapons.cpp`. Report:
`reports/unit_weapons.json`. Every descriptive name below is a hypothesis, not a recovered
symbol; the field names in the gun record are the exception, because the image carries them as
literal strings.

## Summary

A unit does not hold a flat array of weapons. Guns are scene-graph nodes parented under the unit
and identified by a class test, and the unit holds two separate things next to them: a five-slot
array of walk roots at `+3CCh`/`+3D0h`, and a single **weapon director** pointer at `+738h`. The
director owns stance, fire permission and the fire target; the gun nodes own barrels, reload
timers and the act of firing. The Lua bindings split cleanly along that line: `GunForceFire`,
`GunForceFireWithAngle` and `GetGun` take a gun, everything else takes a unit and goes through
the director or through the unit's own torpedo counters.

## How a gun is found

`008CF350` (`GetGun(unit, n)`, one-based) is the producer of the device list:

| Step | Site | What happens |
| --- | --- | --- |
| 1 | `008CF3C1`-ish | `unit->vtable[5Ch](18h)`; when false the seed list stays empty |
| 2 | `008CF3D6` | seed loop over `unit[+3CCh]` entries, reading `unit[+3D0h + i*4]` while `i < 5` and pushing null beyond that |
| 3 | `008CF6xx` | pop a node, append every child (`node[+48h]`, chained through `node[+44h]`) |
| 4 | `008CF74x` | `node->vtable[5Ch](20h)`; on a hit decrement the requested index, and when it reaches zero return that node as an entity |

The `i < 5` clamp is in the listing, so the root array is a fixed five slots even though the
count at `+3CCh` is a plain int. Argument 2, when present, is a boolean read with `00B66250`
that changes which match is taken; this packet did not read that branch — `contract: unread`.

`0081F8B0` uses a shorter walk: only the unit's **direct** children, no worklist and no
recursion. Both agree on the class id `20h` and on the link offsets.

## The gun device record

Offsets and names come from `0072ADC0`, the class's own debug-dump routine: it pushes a
`{type, value}` pair and then a `{0, name-pointer}` pair for each field and calls a formatter
virtual, so the literal string that follows a field read names that field. `EDI` is the gun
throughout.

| Offset | Name in the image | Type | Read at | Meaning established by |
| --- | --- | --- | --- | --- |
| `+3D4h` | (array base) | ptr | `0072AED0`, `0072AF42`, `0072B00C` | per-barrel records, stride `14h` |
| `+3E4h` | — | ptr | `0072CF3x` | per-barrel attached objects; `(end-begin)>>2` is its count |
| `+3F0h` | — | ptr | `0072D523` | ammo provider; its `vtable[1F4h]`/`[1F8h]` test and consume a round |
| `+3F4h` | — | ptr | `0072D52B`, `0081F91x` | weapon class descriptor; `+80h` is the weapon type id |
| `+3F8h` | — | ptr | `0072D55C` | fire parameters; `+28h`/`+2Ch` reload min/max, `+30h` barrel delay, `+4h` spread |
| `+400h` | `throwA` | float | `0072B062` | aim value A, passed to Fire |
| `+404h` | `throwB` | float | `0072B0A8`-ish | aim value B, passed to Fire |
| `+414h` | — | ptr | `0072B0F0`, `0072CF2x` | `float[barrelNum]` reload timers |
| `+418h` | — | ptr | `0072CF33` | `float[barrelNum]`, the full-scale copy of the same |
| `+43Ch` | `delayGroupSize` | int | `0072B1B7` | divisor at `0072E78D` |
| `+440h` | `delayGroupIndex` | int | `0072B1E6` | — |
| `+444h` | `delayGroupCnt` | int | `0072B215` | — |
| `+448h` | `barrelNum` | int | `0072B283` | barrel count; bounds every per-barrel loop |
| `+44Ch` | `nextFireBarrel` | int | `0072B273` | round-robin cursor, advanced at `007309ED` |
| `+450h` | `barrelDelayTime` | float | `0072B136` | reloaded from `[+3F8h][+30h]` at `00730A05` |
| `+454h` | `contFiring` | — | `0072B244` | continuous-fire flag |

The per-barrel record at `+3D4h` has stride `14h` (`add ebp,14h` at `0072B051`):

| Record offset | Name | Read at |
| --- | --- | --- |
| `+4h` | `dist` (`00CFD71C`) | `0072AF48` |
| `+8h` | `speed` (`00CE6748`) | `0072AFAD` |
| `+Ch` | `DT` (`00CFDF48`) | `0072B012` |

`+0h` and `+10h` are inside the stride but were not named by the dump pass: `contract: unread`.

`0072E6D0` (body `0072E6D0`-`0072F053`, `__fastcall(gun)`, `RET`) is the **producer** of this
layout: it writes `+448h` at `0072E71A`, `+44Ch` at `0072E72A`, `+43Ch` and `+450h`, sizing the
gun from its descriptor. Field meanings above are taken from it and from the dump routine, not
from consumers.

The gun's own vtable base is `00CFE0A8`, stored by the constructors `0072D950` (`0072D972`) and
`0072E510` (`0072E535`); `0072E6D0` sits at `+A0h` of it.

## Reload timers

`0072CF00` is `__thiscall(gun, int barrel, float time, int setFull)`, `RET 0Ch` (epilogues at
`0072D01D` and `0072D127`). It writes `gun[+414h][barrel] = time`, and when `setFull` is
non-zero also `gun[+418h][barrel] = time`. Before the write, and only when `time` is **below**
the threshold float at `00D7A278`, it divides by a rate: `1.0f` (`00D7A24C`) normally, or
`008E6430(8, gun[+3F0h])` when the gameplay-modifier byte `00E0C978` is set and
`(00F88C30)+E8h` is non-zero. The tail, which attaches a per-barrel object through
`gun[+3F4h][+C8h]` under a critical section and `00B6E010 BSP_Node_PrependChild`, is an effects
contract: `contract: unread`.

`0072D520` is `__thiscall(gun, int barrel, char full)`, `RET 8`:

| Order | Site | Step |
| --- | --- | --- |
| 1 | `0072D531` | `gun[+3F0h]->vtable[1F4h](gun[+3F4h])` — is a round available? |
| 2 | `0072D592` | if not: `0072CF00(gun, barrel, *00CFDBF8, 0)` — the sentinel, which never decays |
| 3 | `0072D54C` | if so: `gun[+3F0h]->vtable[1F8h](gun[+3F4h])` — consume the round |
| 4 | `0072D579` | if `full`: `t = 00BD2F10(gun[+3F8h][+28h], gun[+3F8h][+2Ch])`, a random in the reload range |
| 5 | `0072D589` | `0072CF00(gun, barrel, t, 1)` |

`00CFDBF8` and `00D7A278` are the sentinel and the "this barrel is empty" threshold:
`0081F8B0` re-arms exactly the barrels whose timer is at or above `00D7A278`.

## Firing: `00730160`

Reached only as gun `vtable[1D8h]` (`00CFE280`). `__thiscall(gun, int useExplicitThrow,
float throwA, float throwB)`, body `00730160`-`00730A1B`, single epilogue `00730A14` `RET 0Ch`.
Ghidra has no function object there — `FUN_0072F830`'s body stops at `0073015C` — so it was read
from the raw listing; `reports/unit_weapons.json` records it as `no_ghidra_function`.

The three-argument shape is proved from the caller, not assumed: at `008BE5AD` `GunForceFire`
pushes `1`, `gun[+400h]` and `gun[+404h]`, and the SEH state slot is at `[ESP+514h]` both before
the sequence (`008BE54B`) and after the call (`008BE5BE`), so the callee pops twelve bytes.

| Order | Site | Step |
| --- | --- | --- |
| 1 | `00730169` | input manager `004BEC00`; abort when `mgr[4][+540h]` has `byte +28h` set and `float +24h > 0` |
| 2 | `0073018E` | abort when `gun[+5Dh] != 0` |
| 3 | `00730198` | abort when `gun[+358h] > 0` and `(00E188A8)[+1FE4h] != 2` |
| 4 | `007301BC` | build the firing frame from node `gun[+3CCh]`, refreshing its world matrix (`00B6DB70`) and folding in the aim controller `gun[+3Ch]` when its `vtable[8Ch]` says so |
| 5 | `00730259` | `gun->vtable[5Ch](24h, ...)` muzzle query, giving a bool and an out-flag |
| 6 | `007302BB` | pick the barrel: normally `007298D0(gun, nextFireBarrel)`, a forward scan modulo `barrelNum` over `+414h` for the first timer at or below zero; on the query hit instead clamp `gun[+4D0h]` against `006FDD70(gun[+3F4h])` |
| 7 | `0073031D` | spread from `gun[+3F8h][+4h]`, zeroed for weapon types 1, 5 and 6 under a settings flag, then scaled by the ammo provider through `00521E70` |
| 8 | `0073051F` | when `useExplicitThrow == 0` both throws are randomised (`00BD2F10`); otherwise the two float arguments are used. Either way they are written to `+400h`/`+404h`, scaled by `00470440(7, provider)`, and used to perturb the muzzle direction |
| 9 | `00730762` | muzzle position: per-barrel offset from `gun[+3F4h][+98h]` (stride `18h`) if present, else node `gun[+3BCh]`'s world translation |
| 10 | `00730840`, `007308FE`, `00730978` | `0072F830(gun, &pos, &dir, &up, flag)` spawns the projectile; the delay-group counter at `gun[+3B4h][+Ch][barrel]` decides whether `flag` is 1 or 0 and is reloaded from `[+3B4h][+8h]` |
| 11 | `007309E8` | `0072D520(gun, barrel, 1)` — consume a round and re-arm that barrel |
| 12 | `007309ED` | `nextFireBarrel = (barrel + 1) % barrelNum` |
| 13 | `00730A05` | `barrelDelayTime = gun[+3F8h][+30h]` |

Steps 7 and 8 are transcribed from the listing but their sub-type table (`gun[+3F8h][+34h][+8h]`
selecting 2, 3, 4, 5 or 7) is not decoded here: `contract: unread`. The projectile class itself
is `docs/TICK_ELEMENT_OVERRIDES.md`; `0072F830` is a spawn contract.

`GunForceFire` differs from `GunForceFireWithAngle` in one way only: before the call it walks
`barrelNum` and sets every reload timer to `0.0f` with `setFull = 0` (`008BE561`-`008BE579`),
so the barrel scan at step 6 always finds barrel zero ready. The angle variant fires whatever
barrel is already ready and supplies the two throws from Lua.

## The weapon director

`unit->vtable[114h]` is `0080E150`: `mov eax,[ecx+738h]; ret`. The director is therefore a
plain pointer field at unit `+738h`, and every stance, target and enable binding goes through it.

`0071BE80` is `__thiscall(director, int stance)`, `RET 4` (epilogue `0071BEC2`):

| Order | Site | Step |
| --- | --- | --- |
| 1 | `0071BE8F` | `allowFire = director->vtable[24h](stance)` |
| 2 | `0071BE9D` | `allowMove = director->vtable[28h](stance)` |
| 3 | `0071BEAF` | `director->vtable[40h](allowFire)` |
| 4 | `0071BEBx` | `director->vtable[44h](allowMove)` |

Both questions are asked before either answer is applied. Three siblings call the same four
virtuals with a literal stance: `0071BED0` passes 0 (`RET` at `0071BF12`), `0071BF20` passes 1
(`RET` at `0071BF62`), `0071BF70` passes 2. Those literals line up exactly with the constants
the shipped scripts define in `scripts/global/luamw_init.lua`, whose duplicate table in
`scripts/global/commandhelpers.lua` annotates each with its fire/move pair:

| Stance | Value | Fire | Move |
| --- | --- | --- | --- |
| `STANCE_HOLD_FIRE` | 0 | hold | hold |
| `STANCE_FREE_FIRE` / `STANCE_GUARD` | 1 | free | hold |
| `STANCE_FREE_ATTACK` | 2 | free | free |
| `STANCE_MOVE_ONLY` | 3 | hold | free |

The bodies of `vtable[24h]` and `vtable[28h]` were not read, so the two predicates in
`unit_weapons.hpp` are the script table's decoding, not a recovered body: `contract: unread`.

## Artillery and torpedo enable

`ArtilleryEnable` and `TorpedoEnable` read the director only as a null gate and then never use
it. Both callees build a session message with base kind `5Ah` (`BSP_SessionMessage_ConstructBase`),
set the boolean into the message and route it with `BSP_Session_RouteMessage(msg, 7, 0)`. The
only difference is one dword: `0071DFD0` writes 3, `0071E0D0` writes 5. So weapon enabling is a
networked command, not a local field write; the receiving handler is not read here.

## Torpedo stock

`0081F8B0` is `__thiscall(unit, int stock)`, reached straight from `ShipSetTorpedoStock` with no
class test and no director.

| Order | Step |
| --- | --- |
| 1 | `live = 00810E90(unit)` |
| 2 | if `stock < live`: `unit[+104Ch] = 0`, clamp `stock` up to 0, then call `0081DCB0(unit)` while `stock < 00810E90(unit)` |
| 3 | else: `unit[+104Ch] = stock - live` |
| 4 | for each direct child that is a gun with descriptor `+80h == 7`: re-arm every barrel whose `+414h` timer is at or above `00D7A278`, with `0072D520(gun, barrel, 1)` |

So `+104Ch` is the spare stock held back, and the torpedo weapon type id is 7. The native loop
in step 2 has no progress guard; the reconstruction adds one and says so in a comment.

`GetShipTorpedoes` is unrelated to the devices: it walks the live-projectile registry at
`(00E188A8)[+19CCh]`, count `+21Ch` and list head `+224h`, each node's `+8h` being the entity,
and returns a Lua table of those whose `+5Eh` is zero and whose `+3BCh` owner is the unit.

## Host table

One row per native call site in the reconstruction. `this`/args are the register and stack shape
at the site.

| Site | Callee | Host method | this / args / ret | Gate |
| --- | --- | --- | --- | --- |
| `008BE540` | `00888D20` | `entity_from_lua_ptr_field` | ECX = LuaObject; ret gun | — |
| `008BE56B` | `0072CF00` | `gun_set_reload_timer` | ECX = gun; (barrel, 0.0f, 0) | `barrel < barrelNum` |
| `008BE5AD` | gun `vtable[1D8h]` = `00730160` | `gun_fire` | ECX = gun; (1, throwA, throwB); RET 0Ch | — |
| `008BE76F` | gun `vtable[1D8h]` | `gun_fire` | ECX = gun; (1, arg1, arg2) | — |
| `008BE7xx` | unit `vtable[114h]` = `0080E150` | `weapon_director` | ECX = unit; ret director | `vtable[5Ch]` true |
| `008BE7xx` | `006E8250` | `director_has_fired` | ECX = director; (name, window); ret bool | director non-null |
| `008A6950` | `0071BF20` | `director_set_stance_0071be80` with 1 | ECX = director | — |
| `008A6AC0` | `0071BED0` | `director_set_stance_0071be80` with 0 | ECX = director | — |
| `008A6490` | `0071BE80` | `director_set_stance_0071be80` | ECX = director; (stance); RET 4 | — |
| `0071BE8F` | director `vtable[24h]` | `director_stance_allows_fire` | ECX = director; (stance); ret bool | — |
| `0071BE9D` | director `vtable[28h]` | `director_stance_allows_move` | ECX = director; (stance); ret bool | — |
| `0071BEAF` | director `vtable[40h]` | `director_apply_fire_permission` | ECX = director; (bool) | — |
| `0071BEBx` | director `vtable[44h]` | `director_apply_move_permission` | ECX = director; (bool) | — |
| `0089C5xx` | `0071DFD0` | `route_weapon_enable` (sub-kind 3) | (bool) | director non-null |
| `0089C8xx` | `0071E0D0` | `route_weapon_enable` (sub-kind 5) | (bool) | director non-null |
| `0089EExx` | `0081F8B0` | `set_torpedo_stock_0081f8b0` | ECX = unit; (stock) | — |
| `0081F8B4` | `00810E90` | `torpedo_count` | ECX = unit; ret int | — |
| `0081F8Dx` | `0081DCB0` | `torpedo_spawn_one` | ECX = unit | `stock < live` |
| `0081F95A` | `0072D520` | `gun_rearm_barrel` | ECX = gun; (barrel, 1); RET 8 | timer >= `00D7A278` |
| `0089C3xx` | director `vtable[2Ch]` | `director_fire_target` | ECX = director; ret entity | director non-null |
| `0089A8xx` | `008889C0` | `lua_argument_is_entity` | ret bool | argument not nil |
| `0089A8xx` | `00B66xxx` ReadVector3 | `lua_argument_vector3` | out float[3] | not nil, not entity |
| `008CF74x` | node `vtable[5Ch]` | `class_test` | ECX = node; (20h); ret bool | — |
| `008BEDxx` | `00B666C0` | `lua_result_table_set` | (key, entity) | owner and alive |
| `0088DCxx` | unread | `unit_firepower` | ECX = unit; ret float | — |

## Coverage

| Routine | Coverage |
| --- | --- |
| `008BE440`, `008BE600` | complete |
| `008CF350` | partial: the argument-2 boolean branch is unread |
| `00730160` | partial: steps 1-13 ordered from the listing; the spread sub-type table at `0073031D`-`0073049C` and the spawn callee `0072F830` are unread |
| `0072CF00` | partial: the effect-attach tail after the timer write is unread |
| `0072D520`, `0072E6D0` (writes only), `0071BE80`, `0071BED0`, `0071BF20` | complete |
| `0071DFD0`, `0071E0D0` | complete as message builders; the receiving handler is unread |
| `0081F8B0`, `008BED80`, `0089C360`, `0080E150`, `007298D0` | complete |
| `008BE7E0` | partial: the `+9D4h` fallback gate was read from pseudocode, not assembly; `006E8250` is unread |
| `0089A8B0` | partial: three branches identified, all three setter bodies unread |
| `0088DC10` | entry only: the firepower computation after the entity fetch is unread |

## Open questions

- Which classes answer `vtable[5Ch](20h)`. The gun vtable `00CFE0A8` has siblings at `00CFE308`,
  `00CFE548`, `00CFBD20`, `00CFBF58` and `00CFC190`; whether those are AA, artillery and torpedo
  subclasses is untested.
- The two unnamed dwords in the `14h` per-barrel record.
- `gun[+3B4h]`, the delay-group block Fire decrements, has no reader outside `00730160` here.
- Planes' bombs and torpedoes: `ShipSetTorpedoStock` and `GetShipTorpedoes` are ship-only by
  name, and no plane path was followed. Contract, unread.

## Corrections from docs/GUN_AIMING.md and docs/WEAPON_DIRECTOR.md (packets cc2_gun_aiming, cc2_weapon_director)

Four readings above are corrected by the packets that read the gun's update and the director:
the per-barrel record's `dist`/`speed` are a recoil spring, not a projectile speed, and no lead
computation reads them; `gun+3F0h` is the owning unit, not an ammo provider; `0071DFD0` and
`0071E0D0` are director methods routing through the endpoint at director+34h, not free functions
that null-gate the director; and there is one fire-target setter (`00835860`), whose three
branches are argument shapes, not three setters.

## Correction from docs/WEAPON_CLASS_DESCRIPTOR.md (packet cc2_projectile_kinds)

The "sub-type table selecting 2, 3, 4, 5 or 7" that Fire consults is the gun class descriptor's
weapon type at `+80h`, not the projectile descriptor's sub-type; and the `+88h`/`+8Ch` rotation
rates docs/GUN_AIMING.md reads belong to the gun class descriptor, not to the projectile class.

## Correction from docs/GUN_DISPERSION.md (packet cc7_gun_dispersion)

- **Was:** the "Firing: `00730160`" section states "Ghidra has no function object there ... so it was
  read from the raw listing; `reports/unit_weapons.json` records it as `no_ghidra_function`".
  **Is:** `00730160` is now a defined Ghidra function, `BSP_Gun_Fire`, body `00730160 - 00730A1D`.
  It was created through `tools/ghidra_define_function.py` during a later integration, so the
  listing, the decompiler and `bsp.py ghidra proto|disasm|decompile` all answer for it now. The
  original raw-listing reading stands; only the "no function object" caveat is stale.
  **Evidence:** `bsp.py ghidra proto 00730160 --brief` reports `body 00730160 - 00730a1d`, and the
  epilogue is `POP EBP` at `00730A14`, `ADD ESP,0FCh` at `00730A15`, `RET 0Ch` at `00730A1B`, with
  `CC` padding from `00730A1E`.

- **Was:** steps 7 and 8 of the Fire sequence carried `contract: unread`.
  **Is:** both are read. Step 7/8 are where the authored `Throw` cone is applied, and they are the
  per-shot dispersion producer for the whole game - not `0072F830`, whose per-pellet ring is
  deterministic. The draw is `throwA = U(0, 2*pi)` and `throwB = tan(Throw') * U(0,1)` from the
  MT19937 stream at `00BD2F10`, applied to the barrel matrix as
  `row2 + throwB*(cos(throwA)*row1 + sin(throwA)*row0)` and left **unnormalised**, so the realised
  half-angle is `atan(throwB)` and the distribution is uniform in radius rather than in area.
  See docs/GUN_DISPERSION.md for the derivation and for the `Throw'` magnitude chain.

## Torpedo stock, bound (packet `cc9_torpedo_stock`, `kTorpedoStockBound`, committed OFF)

Worker cc9-ships, 2026-09-27. Ghidra was read only. It takes the contract in
`docs/CONSTRUCT_WORLD.md` section 27.

### The read (V)

| routine | what it does | evidence |
| --- | --- | --- |
| pass B, 00822C20 | `unit+104Ch = -1` | 00823517 |
| pass C, 0081F980 | `0081F8B0([unit+538h]+7A0h)`, `MaxTorpedoStock`, reached on both arms of the local branch | 008201A9..008201B8 |
| 00810E90 | **loaded torpedo barrels**: walks the list at `unit+3ECh`, which is `+398h + 7*0Ch`, the head of category 7's gun list. It counts each gun's barrels (`gun+448h`) whose timer (`gun+414h`) is below the double at 00D7A278 (FLT_MAX). No weapon-type test is needed | 00810E91, 00810EB6..00810F3D |
| 0081DCB0 | **unloads one random loaded torpedo barrel**: collects every {gun, barrel} with a timer below FLT_MAX, picks one with 00BD2F10, and sets its timer to 00D7A248 (the float FLT_MAX) through 0072CF00 | 0081DD10..0081DDDD |
| 00810D80, no Ghidra function (00810D80..00810D9F) | the unit's `vtable[1F4h]`, *round available*: weapon kind `[desc+80h] != 7` answers 1; kind 7 answers `unit+104Ch != 0` | 00810D84..00810D9D |
| 00810DA0, no Ghidra function (00810DA0..00810DC2) | the unit's `vtable[1F8h]`, *consume*: kind 7 with `unit+104Ch > 0` decrements it; -1 and 0 are left alone | 00810DA4..00810DC0 |
| 0072D520 | after a shot: round available -> consume and reload; else `0072CF00(barrel, FLT_MAX, 0)`, and the barrel never fires again (`docs/GUN_SHOT_CADENCE.md`) | 0072D531..0072D5A5 |
| 00825450, the supply tick | from `UpdateShipMotion` (00826182), only when 00809C50 finds the ship in a supply area: every `settings+498h` it calls `0081F8B0(stock + 1)` while spare + loaded is below `MaxTorpedoStock` | the decompile |

- **Consequences.**
  - Pass B's -1 means unlimited.
  - After pass C a ship can fire exactly `MaxTorpedoStock` torpedoes, whatever its barrel count:
    the loaded barrels plus the spare.
  - A class with no `MaxTorpedoStock` (IntegerOrZero, 0) gets spare 0, and 0081DCB0 empties
    every loaded tube at pass C.
- **The table** 00810D80/00810DA0 sits in nine vtables (00CF92A4, 00CFA96C, 00CFB92C, 00CFC5C4,
  00CFFC24, 00D01824, 00D0986C, 00D0C174, 00D0C83C), so it is the unit base's provider.

**Corrections to `src/unit_weapons.cpp` / `include/bsp/unit_weapons.hpp` (names only, not
edited):**
- `torpedo_count` (00810E90) is the loaded-barrel count, not a count of torpedoes in the water.
- `torpedo_spawn_one` (0081DCB0) unloads one random loaded barrel; it spawns nothing.
- The reconstruction's control flow in `set_torpedo_stock_0081f8b0` matches the image.

### This installation's stocks on USN02

`scripts/datatables/autoload/vehicleclasses.lua` (2026-05-09, locally modified) against
usn_2_java.scn's `Type` rows:

| ships | class | `MaxTorpedoStock` |
| --- | --- | --- |
| Kortenaer, Electra, Encounter, Jupiter, Witte | Icarus (265) | 30 |
| Alden, John1..3 | Clemson (25) | 24 |
| Exeter | York (21) | 18 |
| Perth | Fiji (263) | 22 |
| Haguro, Nachi | Myoko (293) | 18 |
| Jintsu, Naka | Kuma (70) | 18 |
| Yudachi, Samidare, Murasame, Harusame, Yamakaze, Kawakaze | Shiratsuyu (289) | 24 |
| Minegumo, Asagumo, Yukikaze, Tokitsukaze, Amatsukaze, Hatsukaze | Kagero (276) | 28 |
| Sazanami, Ushio | Fubuki (73) | 27 |
| DeRuyter, Java, Houston | DeRuyter (20), Northampton (19) | absent; no tubes |

Reference d's launches per shooter (`local\rb4_usn02.log`, 209 in all):
- Tokitsukaze and Minegumo launch 32 each, against a stock of 28.
- Amatsukaze launches 24, Yukikaze 17, Asagumo 16, John1 12, and every other ship 9 or fewer.

### The binding

- **The class value.** The gunnery host's class reader adds `torpstock` (`MaxTorpedoStock`, 0
  when absent) to each unit's state.
- **The provider.** At the shared fire site, a ship's category-7 barrel runs 0072D520's provider
  pair on the unit's spare:
  - the first torpedo shot sets the spare as 0081F8B0 does at pass C;
  - each later shot spends a spare if there is one;
  - with none, the fired barrel is pinned at FLT_MAX.
- **LABELLED SUBSTITUTIONS.**
  - The pass C set is made at the ship's first torpedo shot. Every barrel is still loaded then,
    so the count is pass C's.
  - The supply tick 00825450 is not bound. No area test (00809C50) is read, so whether a
    reference ship ever resupplies is unknown.
  - The unload 0081DCB0 (stock below the loaded count) is not bound. No USN02 ship has fewer
    stock than tubes.
- **Planes:** not affected; the rule is limited to ships.
- **The census:** `summary mission gunnery torpedo stock bound=.. sets=.. spent=..
  emptied_barrels=.. ships_dry=..`, and one `torpedo stock:` line per ship set or dry.

### Predictions (written before the pairs; the same tree, switch only, both variables set)

**USN02 9200/9000.**
- `sets=19`: the 19 ships that launch in reference d.
- `ships_dry=2`, Tokitsukaze and Minegumo, after their 28th launch.
- `emptied_barrels=16`: 8 per dry Kagero. Each fired barrel after the spare runs out is pinned,
  and the 8 loaded barrels are the last 8 shots.
- **Timing.** Everything is identical up to Tokitsukaze's 29th launch in reference d (364.18 s).
  Tokitsukaze then launches nothing, and Minegumo stops after its 28th (387.63 s).
- **Gyro launches** 209 -> about 201. Later dynamics may move it by a few.
- **Death rows** identical up to 364 s. Later deaths may move: Haguro at 384.03 s and Jintsu at
  405.77 s in reference d.
- pair_diff exit 3 if anything after 364 s moves, else exit 1.

**USN04 4700/4500.** No ship launches a torpedo (reference d has 0 launch lines). So `sets=0`,
and the pair is identical (exit 1).

### The pairs, measured, and the verdict

- **Builds.** `tools/pair_export.py` of `52a3c4e6f`: `local\ri_off` (SHA-256 prefix
  `8071D701F196`) and `local\ri_on` (`E14C7AFCF9C3`).
- **Logs.** `local\ts_{off,on}_{usn02,usn04}.log` in worktree cc9-ships. Each shows the 1600x900
  fit, the immediate present interval, its own module directory and the final COM release.

**USN04 4700/4500: pair_diff exit 1.** `sets=0`, no ship launches, identical.

**USN02 9200/9000: pair_diff exit 3.**

```
* deaths                                 22                                       21
* hit records                            603                                      623
* hull hits                              305                                      313
* damage                                 56330.1                                  55393.8
* shots                                  1115                                     1124
  first hit                              35.65 s                                  35.65 s
  mission end                            failed at 39.65 s (Mission.EndMission) text="Game Over" e... failed at 39.65 s (Mission.EndMission) text="Game Over" e...
DEATH ROWS: 22 -> 21 rows, 0 only ON, 1 only OFF, 2 changed
```

- **The census.** `sets=19 spent=181 emptied_barrels=22 ships_dry=1`; gyro launches 201 -> 203.
- **Launches per shooter, OFF -> ON:**
  - Minegumo 32 -> 28. It runs dry, with its last torpedo pinned.
  - Amatsukaze and Tokitsukaze 24 -> 24.
  - John2 6 -> 12.
  - Every other ship is unchanged.
- **The first diverging launch is #176 at 344.24 s.** John3 fires there ON; Alden fires at
  347.89 s OFF.
- **The deaths.** John2 survives ON. Haguro dies at 395.93 s to a torpedo (category 7) instead of
  at 382.18 s to a shell. Jintsu moves by 0.35 s. The mission still fails at 39.65 s.

**Predictions against the measurement:**

| row | predicted | measured | held |
| --- | --- | --- | --- |
| USN04 | identity | identity | yes |
| `sets` | 19 | 19 | yes |
| `ships_dry` | 2 (Tokitsukaze, Minegumo) | 1 (Minegumo) | **no** |
| `emptied_barrels` | 16 | 22 | **no** |
| gyro launches | about 201, down | 201 -> 203, up | **no** |
| identical until 364.18 s | yes | first divergence at 344.24 s | **no** |

**Why they failed.**
1. **The base moved since reference d.** On this tree Tokitsukaze launches 24, not 32, so it never
   runs dry.
2. **Pinning starts when the spare reaches 0, not at the last torpedo.**
   - A Kagero's spare is 20, so from its 21st launch every fired barrel is pinned. That is 4
     barrels each for Amatsukaze and Tokitsukaze, and the last 8 for Minegumo.
   - Those barrels no longer reload, which changes the later cadence before any ship is dry, so
     the battle moves from about 344 s.
   - The prediction confused "dry" with "no spare".

**Verdict: `kTorpedoStockBound` ON.**
- A ship now fires exactly its class's `MaxTorpedoStock`, as 0081F8B0 and 0072D520's provider pair
  allow.
- The moves are late in the run (after 344 s) and follow from the pinned barrels.
- **Open:**
  - the supply tick 00825450 and its area test 00809C50;
  - the unload 0081DCB0, which needs stock below the loaded count;
  - the Lua `TorpedoStock` property 00815870 and the HUD gauge 00815850, which read spare +
    loaded;
  - `ShipSetTorpedoStock` (`set_torpedo_stock_0081f8b0` in `src/unit_weapons.cpp`), which is
    still not wired to this spare.

## Handoff: cc9-ships retires after `cc9_torpedo_stock` (2026-09-27)

Worker cc9-ships, successor of cc9-scene-entities. Every switch below was measured by same-tree
pairs (`tools/pair_export.py` + `tools/pair_diff.py`).

**Landed or committed ON:**

| switch | file | doc |
| --- | --- | --- |
| `kShipDirectorEnablesBound` (+ TorpedoEnable, CLOSEATTACK 00A11AF0) | `include/bsp/game_hosts_gunnery.hpp` | `docs/SENTITY_INIT_PASSES.md` section 8 |
| `kSceneRaceAndScriptIdentityBound` | `include/bsp/game_hosts_scene_contents.hpp` | `docs/SENTITY_INIT_ATTACH_ORDER.md` section 17 |
| `kSceneHomeBaseContractBound` | same | `docs/CONSTRUCT_WORLD.md` section 31 |
| `kTorpedoStockBound` | `include/bsp/game_hosts_gunnery.hpp` | this file, "Torpedo stock, bound" |

Reference d is in `docs/GAME_EXECUTABLE.md`, "Mission reference baselines, 2026-09-27 d". The
USN01 drop flag is closed there.

**Committed OFF, owned by this line: the joint wing flip.** `kWingConstructionInPassABound`
(`include/bsp/game_hosts_units.hpp`, `docs/CONSTRUCT_WORLD.md` section 30) flips together with
cc9-movie-camera's Lua half (`docs/WING_CONSTRUCTION_LUA.md`, when it lands). The lead sends the
go.
- This half alone moved USN04's air battle within its bands: the wing planes now follow all
  leaders in index order.
- The joint pair should expect `squadron_ids` and `wing_member_tables` back at 40, and that small
  air move rather than identity.

**Open items, in the order I would take them:**
1. **The ship AI's torpedo standoff** (`docs/SENTITY_INIT_PASSES.md` 8, "Open").
   - 009F1BC0 caches `director+222h` at `nested+12BAh` and a value at `+12B4h` (009F2D46..009F2D8C,
     arithmetic unread).
   - 009E6E80 caps the standoff at `+12B4h` minus the turn radius when both are set.
   - The host never writes `clearance_12b4`, and it names `+12BAh` `clearance_valid_12ba`
     (`src/ship_ai_approach_update.cpp:465`, no image store).
   - This is the largest remaining torpedo-behaviour gap.
2. **Torpedo supply and wiring** (this file):
   - the supply tick 00825450 and its area test 00809C50;
   - `ShipSetTorpedoStock` (`set_torpedo_stock_0081f8b0`) is not yet wired to the gunnery host's
     spare;
   - the unload 0081DCB0;
   - `TorpedoStock` 00815870 and the HUD gauge 00815850;
   - rename `torpedo_count` / `torpedo_spawn_one` in `unit_weapons.*` to what they are.
3. **Bypass launches not reached on the reference missions:** `NavigatorForceTorpedo` 008A7200
   (`vtable[1D8h]` unread) and `SubmarineAttack` 00894440. Both are used by jm05/jm06.
4. **00A11B80**, CLOSEATTACK's and DEFENDPOSITION's middle call: unread and unbound.
5. **Identity:**
   - the Lua host's marker mirror should write Race itself (`src/game_hosts_lua.cpp`);
   - SetParty's `vtable[2Ch]` on units;
   - whether the image's SpawnNew bag carries a `HomeBase` (unread; IJN08 measures the scene side).
6. **From the scene-entities handoff, not started:** the quadtree segment walk 00ADA240 (00AEA2B0 /
   00AE9D80 / 00AECC40), and the island-rotation question
   (`docs/SCENE_CONTENTS_HOSTS.md` section 9).

**Tools kept in this worktree's `local\`** (not committed):
- `cc9-ships-disp.py` (displacement census);
- `cc9-ships-abs.py` (absolute-dword census);
- `cc9-ships-dw.py` / `cc9-ships-str.py` (PE dword and string readers);
- `cc9-ships-sect.py` (doc section grep);
- `cc9-ships-stock.py` / `cc9-ships-scnclass.py` (class `MaxTorpedoStock` against a scene's
  `Type` rows);
- the `cc9-ships-edit*.py` edit scripts.

## The supply tick (packet `cc9_torpedo_supply_tick`, `kTorpedoSupplyTickBound`, committed OFF)

Worker cc9-ships2, on main `088805f9a`. Ghidra was read-only. The switch is in
`include/bsp/game_hosts_gunnery.hpp`.

### The read

**00825450**, `__thiscall(unit)(float seconds)`, body 00825450..0082558F. It is called from
UpdateShipMotion 00825F20 at 00826182, so every ship runs it every motion step.
1. 00825479: 00809C10 on `unit+72Ch`, the unit's own area holder. It steps the area in `[+8h]`
   when the unit owns one.
2. 0082549A: `unit+112Ch = 00809C50(unit, [unit+54h], &unit+0FCh)`.
3. When `[[00E188A8]+1FE4h] == 2`, nothing else happens.
4. With no area: `+1154h = 0` and `+1150h = 1.0` (00D7A24C).
5. With an area:
   - `+1150h` becomes `settings+494h` (Repair.RepairZoneMultiplier, 5.00 in shipglobals.lua:437).
   - `+1154h` becomes `+1154h - seconds`.
   - When that falls below 0: `s = +104Ch`, plus 00810E90's loaded barrels when `s >= 0`.
     If `s < [class+7A0h]` (MaxTorpedoStock), the tick calls `0081F8B0(s + 1)` and sends the
     session message 96h (0080FE10, 0077C7B0).
   - Then `+1154h += settings+498h` (Repair.TorpedoRestockTime, 1.00, shipglobals.lua:438).

`+1150h` is the repair-zone multiplier. The same tick resets it to 1.0 outside an area.

**00809C50**, the area test, walks the vector at 00F874F0 (count 00F874F4). 00809A80 accepts an
area when all of these hold:
- its owner `+14h` is live and is not the unit;
- the owner is of the same party (BSP_Party_RelativeTo answers 0);
- the unit's x/z lies inside the bounding circle (`+3Ch`, `+44h`, radius squared `+48h`);
- the unit is within `+34h` of the segment `+1Ch..+30h`.

**Who registers an area.**
- The vector's only writers are 00809740 (add) and 00809820 (remove), in an absolute-dword scan.
- 00809740's only caller is 00809880, and 00809880's only caller is 00809BC0.
- 00809BC0 is called from pass C (0081F980) and from 00748CC0.
- 00809BC0 builds an area for a mode-2 (savegame) bag. For a mode-1 (fresh scene) bag it builds
  one only when `RepairZoneArea` is found and non-empty.

**This installation has no supply area on any fresh mission.**
- `universe/library/ship.props:23` and `landfort.props:4` default `RepairZoneArea` to `R ""`.
- A recursive search of `universe/scenes` finds `RepairZoneArea` in every mission directory (bsm,
  chg, usn, ijn, multi, COTP). No row has a non-empty value.
- usn_2_java.scn has 42 rows and usn_19_coralus.scn 41, all `""`.

So 00809C50 answers 0 on every mission the harness can run, the tick never supplies, and **no
measuring pair exists**. Only a savegame could carry an area.

**0081DCB0** collects every loaded {gun, barrel} of the category-7 list `unit+3ECh`, in list and
barrel order (0081DD10..0081DD44). It picks one with `trunc(00BD2F10(stream 1, 0, n))`
(0081DD7B..0081DDA6, FISTP under RC = truncate) and pins it through
`0072CF00(gun, barrel, FLT_MAX, 0)` (0081DDDD).

### The binding (under `kTorpedoSupplyTickBound`)

- `bsp::torpedo_supply_tick_00825450` (`src/unit_weapons.cpp`) is steps 2..5. The gunnery host
  runs it for every ship once per fixed step.
- **LABELLED:** the area registry is empty. The tick runs after the gunnery passes rather than
  inside the motion update, which it does not read. A resupply's 0081F8B0 call and message 96h are
  recorded, not performed (unreachable while the registry is empty).
- The stock set of `kTorpedoStockBound` now runs 0081F8B0's unload loop with 0081DCB0 while the
  stock is below the loaded count.
  - The draw is `Draw::torpedo_unload`, keyed (unit, 0) under `BSP_GUNNERY_RNG_STREAMS=1`, and the
    shared generator otherwise.
  - **LABELLED:** it runs at the first shot, not at pass C, so the barrel about to fire can be one
    of those drawn.
- **Census:** `summary mission gunnery torpedo supply bound=.. ticks=.. in_area=.. sets=..
  unloaded=..`, and one `torpedo stock: <ship> unloads gun .. barrel ..` line per unload.

**Folded into the same commit** (identity by construction; the pairs below cover both):
- `src/unit_weapons.cpp` / `include/bsp/unit_weapons.hpp`:
  - `torpedo_count` is renamed `loaded_torpedo_barrels_00810e90`, and `torpedo_spawn_one`
    `unload_random_torpedo_barrel_0081dcb0`.
  - Their comments and the one caller, `set_torpedo_stock_0081f8b0`, are updated.
  - `UnitWeaponHost` has no implementer in the tree.
- The ship AI host's stance-push recomputation of 00863920 is removed; the standoff always asks
  the gunnery host's accessor. It was reached only with `kShipAiQueryGateBytesBound` off, which
  landed ON.

### Predictions (written before the pairs; same tree, switch only, streams and the death table on)

| row | prediction |
| --- | --- |
| USN02 `pair_diff` | 1, gameplay identical: no area, and no USN02 ship has stock below loaded (the stock lines show loaded 5..16 against stock 18..30) |
| USN02 `torpedo supply` | `in_area=0 sets=0 unloaded=0`; `ticks` = ship units times the steps they live (nonzero) |
| USN04 `pair_diff` | 1: no area, and no ship fires a torpedo, so no stock is ever set |
| USN04 `torpedo supply` | `in_area=0 sets=0 unloaded=0`, `ticks` nonzero |
| both | the torpedo stock lines (`sets`, `spent`, `emptied_barrels`, `ships_dry`) identical |
