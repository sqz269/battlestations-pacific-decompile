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
