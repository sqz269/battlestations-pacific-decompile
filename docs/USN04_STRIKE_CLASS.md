# Why every USN04 launch drew class 101, and what actually blocks the Japanese strike

Packet `cc8_usn04_strike_class`. The packet asked why every `LaunchSquadron` call passes class 101,
on the hypothesis that a chooser is broken or a native returns a neutral value that always selects
the first option. **Neither is true.** The answer to the question as asked is mundane; finding it
turned up two real defects, one of them mine, and both are fixed here.

## 1. The script

| | |
| --- | --- |
| path | `I:\SteamLibrary\steamapps\common\Battlestations Pacific\scripts\missions\usn\usn_19_coralus.lua` |
| size | 99030 bytes |
| mtime | **2024-08-26 16:10:32**, untouched bulk |

Worth stating because this installation is modded: the mission script is **not** one of the modified
files. `scripts\datatables\autoload\vehicleclasses.lua`, which the class ids below resolve against,
**is** — 3349364 bytes, mtime **2026-05-09 21:52:13**. So the launch blocks are the shipped ones and
the class table is this installation's.

## 2. Why every call passed 101

There are 26 `LaunchSquadron` sites. Only two of them ran, and their class table has one value in it
twice:

```lua
535: if Mission.MissionPhase == 1 then
537:   if Mission.Lex and not Mission.Lex.Dead then
538:     local stloPlaneNum, stloPlaneTable = luaGetSlotsAndSquads(Mission.Lex)
539:     if stloPlaneNum < 2 and IsReadyToSendPlanes(Mission.Lex) then
540:       local planeTypes = {
541:         101,
542:         101,
543:       }
544:       local slotIndex = LaunchSquadron(Mission.Lex, luaPickRnd(planeTypes), 3)
```

and the same shape for `Mission.Town` at `:551`-`:558`. `luaPickRnd` cannot return anything but 101,
whatever the chooser does. **There is no defect at these sites**, and `docs/DIVE_BOMB_MISSION_SURVEY.md`'s
"picks at random between a dive bomber and a Kate" is about the *striker* blocks, `:1404`-`:1408`,
which never ran.

**And the launch count of exactly four is authored too.** Line 539's `stloPlaneNum < 2` is the gate:
`luaGetSlotsAndSquads` counts the squadrons a carrier has out, and the block launches only while
fewer than two are. Two per American carrier, two carriers, four launches.

That corrects `docs/AIROPS_LAUNCH_TICK.md`'s reading of the same run, which said the script "launched
while `slot.squadron` was nil and stopped when it was not". The stopping is this explicit `< 2`. The
`squadron` key is still what makes the count real — before the tick filled slot+28h, `stloPlaneNum`
was always 0 and the script relaunched for ever, which is the 82 launches of the earlier run — but
the gate is the count, not the nil.

## 3. The chooser is sound

For completeness, because the packet asked whether the producer returns a neutral value:

```lua
995: function luaPickRnd(anytable, tablekey)
1001:  for key, value in pairs(anytable) do
1002:    rnd = luaRnd()
1003:    if rnd > rndmax then
1004:      rndmax = rnd
1005:      rndSelected = value
1011:  if tablekey == nil then return rndSelected
```

`luaRnd()` with no arguments reaches `rnd = random()` (`commandhelpers.lua:2553`), the native
`0088C160`. Note the failure mode a neutral value would actually produce: the comparison is a strict
`>` and `rndmax` starts at 0, so a producer that always returned 0 would leave `rndSelected` **nil**
and `luaPickRnd` would return nil — the first option would *not* be selected, the class argument
would be nil. That is not what happens.

`random()` is reconstructed and routed. `bsp::lua_binding_random` takes the zero-argument range
`[0, 32767)` from `random_binding_range_0088c160` (`008AE360`'s `FLD [00D11318]`) and samples it
through `GameScriptOrdersHost::random_uniform_00bd2f10`, a 32-bit LCG with a fixed seed. Draws are
well spread over 32767 values, so ties are negligible and `luaPickRnd({158, 162})` is a real coin
flip. The earlier run's `random 0088c160 concrete calls=8` is exactly two draws for each of the four
launches, which corroborates the whole chain.

## 4. The real blocker: `GetDifficulty` was never routed

All six striker sites sit in one `if / elseif / elseif` on the difficulty:

| line | guard | sites | count per launch |
| --- | --- | --- | --- |
| `:1382` | `if Mission.Difficulty == 0 then` | `:1393`, `:1408`, `:1428`, `:1443` | 3 |
| `:1454` | `elseif Mission.Difficulty == 1 then` | `:1465`, `:1480`, `:1500`, `:1515` | 4 |
| `:1526` | `elseif Mission.Difficulty == 2 then` | `:1538`, `:1554`, `:1575`, `:1591` | 5 |

and `Mission.Difficulty = GetDifficulty()` at `:68`.

`GetDifficulty` reported `UNIMPLEMENTED` in every run. An unimplemented binding in this host falls
through `mission_binding_returns_entity` to a bare `return 0` — it **pushes nothing**. A binding that
pushes nothing is not a neutral value to a Lua script: `Mission.Difficulty` was **nil**, `nil == 0`
is false, and so were the other two. **Every one of the twelve Japanese striker launches was
unreachable at every difficulty**, and no length of run would have reached them.

The body was not missing. `bsp::lua_binding_get_difficulty` has been in `src/lua_binding_core.cpp`
since `docs/LUA_BINDING_CORE.md`, and it is faithful:

```
008ae059: XOR EBP,EBP
008ae10e: MOV EAX,[0x00e188a8]
008ae113: CMP dword ptr [EAX + 0x1fe4],EBP
008ae121: JZ  0x008ae12a          ; equal (so zero) -> the campaign arm
008ae123: MOV EAX,0x2             ; non-campaign: the literal 2
008ae12a: MOV EAX,dword ptr [EAX + 0x6ac]   ; campaign: the effective difficulty
008ae130: PUSH EAX
008ae135: CALL 0x00b664b0         ; lua_pushnumber
```

What was missing was one row in `kScriptOrderBindings`, so `GameScriptOrdersHost::handles` answered
false and the dispatch never reached the body. The fix is that row and a dispatch arm.

The two host readers it needs were already overridden on the adapter, and both now carry their
source rather than a convenient zero:

* `game_non_campaign_flag()` → 0. This process asserts a campaign session in two other places,
  `non_campaign_session()` returning false and `inputs.non_campaign_session = false`, both in
  `src/game_hosts_mission.cpp`, and `006CDC70` gates its sub-updates on the same word.
* `game_effective_difficulty()` → 0, game+6ACh. Nothing in this process writes it —
  `MissionStart::set_effective_difficulty` is a record at `0058BF58` — so the field holds its
  constructor zero, which is what `008AE12A` would read. That it is also the arm the strike wants is
  a consequence, not the reason.

## 5. A regression of mine, retracted

Packet `cc8_airops_launch_tick` changed the Lua `squadron` key from the raw id to the entity's
`thisTable` slot, reasoning that the script hands `slot.squadron` straight to `PilotSetTarget`, which
needs a table. **The script does no such thing.** All four readers go through `thisTable` themselves:

```lua
545:  local launchedWildcat = thisTable[tostring(GetProperty(Mission.Lex, "slots")[slotIndex].squadron)]
560:  Mission.scoutingWildcat = thisTable[tostring(GetProperty(Mission.Town, "slots")[slotIndex].squadron)]
1394: Mission.ZuikakuZero  = thisTable[tostring(...)]
1409: local launchedStriker = thisTable[tostring(GetProperty(Mission.Zuikaku, "slots")[slotIndex].squadron)]
1411: PilotSetTarget(launchedStriker, bombertrg)
```

`kMissionLuaEntityKeyFormat` is `"%d"`, so a `thisTable` key is the entity id as text and
`tostring` of the number is exactly that key. Pushing a table makes `tostring` yield
`"table: 0x..."`, every one of those lookups returns nil, and the mission then orders nil —
including `PilotSetTarget(launchedStriker, ...)` at `:1411`, the one call this whole stream exists
to reach. The original integer was right; the reasoning that replaced it was wrong. Restored.

Both defects had to be fixed together. Either one alone still leaves no torpedo task: without the
difficulty the striker never launches, and with it but without the integer key the striker launches
and is then ordered as nil.

## 6. The classes, by id

Read from this installation's table with `bsp_mission_script_probe --vehicle-class <index>`, which
runs the same autoload folder `00886900` does:

| id | `Name` | `Type` | where it is launched |
| --- | --- | --- | --- |
| 101 | `globals.unitclass_wildcat` | `Fighter` | `Mission.Lex` `:544`, `Mission.Town` `:558` |
| 150 | `globals.unitclass_zero` | (a fighter; launched first at `:1393` to set `Mission.ZeroOverZuikaku`) | Zuikaku, Shokaku |
| 158 | `globals.unitclass_val` | `DiveBomber` | the striker coin flip |
| 162 | `globals.unitclass_kate` | `TorpedoBomber` | the striker coin flip |

### Which ids carry ordnance kind 2Bh

`Type` above is the authored class type and is not the ordnance. The 2Bh bit comes from a gun's
*bullet* class: `game_hosts_gunnery.cpp:538` reads the bullet class's `Type` string and
`ordnance_kinds_for_bullet_type` maps it, and only `{"Torpedo", bit(0x2b) | bit(0x2a) | bit(0x29)}`
sets 2Bh. That is three tables away from the vehicle class, along the chain the gunnery host's own
Lua chunk walks:

```
VehicleClass[N] -> a platform's ["Gun"][1] -> Devices[id] -> dev.Bullet[i].Bullet
                -> Bullets[id]["Type"] == "Torpedo"
```

Walking those three tables in this installation (`classtables/arcade/bulletclasses.lua`,
`classtables/arcade/deviceclasses.lua`, `autoload/vehicleclasses.lua`) gives:

* 12 bullet classes have `Type = "Torpedo"`: 4, 27, 29, 61-67, 69, 70.
* 36 device classes fire one of those.
* **89 vehicle classes carry ordnance 2Bh**, and of the four ids this mission launches exactly one
  of them does:

| id | class | carries 2Bh |
| --- | --- | --- |
| 101 | Wildcat, `Fighter` | no |
| 150 | Zero | no |
| 158 | Val, `DiveBomber` | no |
| **162** | **Kate, `TorpedoBomber`** | **yes**, through device 85 |

So the striker coin flip `{158, 162}` has one torpedo-capable face, and **162 is the id the whole
stream needs**. The run corroborates it independently and by id: a launched 162 that raises
`units_with torpedo` proves it, and the earlier run already proved the converse for 101, which
raised `general_bomb` and left `torpedo` at 34.

## 7. The deck brake is a non-campaign mechanism, so it is not what paces this

Recorded here because it bears directly on the launch counts above.
`docs/AIROPS_LAUNCH_TICK.md` section 5 proposed implementing the deck's readiness brake — block+38h,
written by `006C6540` out of the spotting queue — to replace this process's squadron ceiling. The
branch bytes in `007F1C00` say that cannot work:

```
007f1c4b: CMP dword ptr [ECX + 0x1fe4],0x0
007f1c55: JZ  0x007f1c69          ; ZERO -> 006CC7B0, the block+74h queue
007f1c57: CALL 0x006cc760         ; non-zero -> the block+C0h spotting queue
```

The spotting queue is the **non-campaign** arm. A campaign session takes `006CC7B0` into block+74h,
which `006C58A0` drains straight into a slot through `006C56D0`, and nothing on that path writes
block+38h. This process asserts a campaign session in three places, so **in a campaign the deck has
no readiness brake** — which is consistent with what the runs show, and with why the mission script
carries its own `stloPlaneNum` gates.

So the pacing this packet was looking for is the script's gate, section 2's `stloPlaneNum < 2`, and
it already works because the tick keeps slot+28h filled. The campaign arm is now reconstructed
(`air_ops_push_assign_queue_006cc7b0`, `air_ops_arrive_squadron_006c56d0`,
`air_ops_drain_assign_queue_006c58a0`, drained first in the deck update as `006CDC70` has it); the
spotting arm is deliberately not, because it cannot execute here. The ceiling stays as a safety net,
raised to 64 so it sits above the bound the script's gates impose rather than at it.

## Uncertainty

* The `Zero` at `:1393` must launch first and set `Mission.ZeroOverZuikaku` before the `else` arm
  reaches the strikers, and the striker arm additionally needs `stloPlaneNum >= 1`. So the first
  launch on each Japanese carrier is a fighter and only later ones are strikers. Untested until the
  run.
* `luaAddZuikakuZeroListener` at `:1400` and the listener bindings were not read.
* Whether the modded `vehicleclasses.lua` changed 158 or 162's weapons from the shipped ones was not
  checked; the class table is this installation's throughout.

## Host methods

| method | address | coverage |
| --- | --- | --- |
| `bsp::lua_binding_get_difficulty` | 008AE030 | complete; this packet routed it, it did not write it |
| `bsp::lua_binding_random` | 0088C160 | already complete and already routed |

## no_ghidra_function

None.

## Validation

Build and the measured run are below.
