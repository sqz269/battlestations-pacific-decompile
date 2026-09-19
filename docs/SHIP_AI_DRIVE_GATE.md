# The ship drive gate: what actually stopped the USN04 carriers

Addresses: `0071D780`, `0071E6C0`, `0071E550`, `0071F600`, `00835C70`, `009E1170`, `00836BF0`,
`009DE050`, `00928F50`, `00928A00`

Packet `cc8_ship_drive`, on top of `docs/SHIP_AI_PATH_CURSOR.md` and
`docs/LUA_BINDING_MOVE_ON_PATH.md`. Three USN04 runs, same binary apart from the change under
test: `local/drive_before_usn04.log`, `local/drive_after_usn04.log`, `local/drive_keep_usn04.log`.

## 1. Two retractions, both of them from the previous packet's own run

`docs/SHIP_AI_PATH_CURSOR.md` section 8 says **"The ships still do not drive"** and rests it on
two numbers from the cursor table, `advances 0` in 900 tries and `travelled 48.30`. Both numbers
are artefacts of this host's own path build, not measurements of the hull.

**`travelled` and `advances` were reset every 3.06 s.** `begin_path_command_0071f600` was called
from the 5Bh message arm, i.e. once per `NavigatorMoveOnPath` call, and its body zeroes
`path_travelled`, `path_advances` and `path_visited` and re-seeds the cursor's join index. USN04's
script sends 49 of those per carrier in 150 s, one every 3.06 s. So `travelled 48.30` is not
"the Yorktown moved 48 m in the whole mission", it is **the Yorktown's hull path over the last
3.06 s: 15.8 m/s**, which is the commanded speed `008A3600` stored (16.719 m/s) less the turn.
`advances 0` is the same artefact: a cursor that is re-seeded every 61 steps cannot walk.

The ship AI rows in the same log say it independently. `ship ai step 70 .. 3000` for
`Yorktown-class01` is `state=moveonpath mode=navigate dir=ahead throttle=1.000`, and its
distance-to-waypoint `d32c` falls **5850.94 -> 3584.72**, a steady 0.83 m per 0.05 s step. The
hull covers 2.3 km; it does not arrive because its nearest authored `CarrierPath4` point is
5.8 km away when the order lands and the mission window is 150 s.

**The command row's `latch`, `steer` and `thrust` say nothing about a `moveonpath` row.** They
are written only by `finish_issue`'s `cruise` arm (`src/game_hosts_commands.cpp`, the
`row.current && current == kCruiseCommandObjectAddress` branch that runs
`cruise_command_begin_00835c70`, `00835E0E..00835E5D`). A row for any other command class prints
`latch -` and `0.000 / 0.000` by construction. Reading "both carriers' command rows read `latch -`,
`steer 0.000`, `thrust 0.000`" as evidence of a gate is reading the absence of a `cruise` latch.

What *is* true of the Lexington is in section 4: it is the player's unit and it stands still on
purpose.

## 2. `0071D780`'s weighted count is the queue-FULL test and nothing else

`0071D780`, body `0071D780-0071D807`, read whole. The walk is `LEA EAX,[ESI+3]` /
`LEA ECX,[EAX*8]` / `SUB ECX,EAX` at `0071D790`, i.e. `director + 1Ch*i + 54h`, stopping at the
first slot whose command pointer is null. `MOV EBX,0x1` at `0071D7A8` is the step, and only a
`moveonpath` slot (`CMP EAX,0xe08f80`) with a path object at `director+1A4h+i*4` whose
`vtable[4h]` answers true replaces it: `0071D7E0..0071D7F3` takes `(end - begin)` off the
vector `vtable[8h]` returns and divides by `0Ch` (the magic `0x2AAAAAAB` with `SAR EDX,1`), and
`CMP EAX,EBX / JL` keeps the larger of that and 1.

**The one thing that reads the answer is `CMP EAX,0xa`.** An exhaustive scan finds a single
rel32 call, `0071E6C3` in `0071E6C0 BSP_WeaponDirector_PushCommandSlot`, and no absolute
reference (`tools/callsite_census.py 0071d780`, plus `scan-bytes '80 d7 71 00'` with
`scan-bytes 'c0 e6 71 00'` as the control that shows the scanner does find `.rdata` vtable
slots). `0071E6C8 CMP EAX,0xa / JL` refuses the push and returns false; every site that wants a
slot INDEX runs its own unweighted walk instead - `0071E6D6..0071E6EE` in the same routine, four
instructions later, and `0071E56B..0071E57E` in `0071E550`, the make-room arm.

So the count that the host's `Impl::command_count` models (first empty slot) is right where it is
used, at `command_accepted`, `make_room_for_command` and the queue-advance site, and the
weighting belongs only on `DirectorBinding::command_count`, which feeds
`director_push_command_slot_0071e6c0`'s capacity test. That is where this packet wired it.

**Measured: it changes the arithmetic and refuses nothing.** The after run prints

```
0071d780's `moveonpath` weighting is live: the queue-full test at 0071e6c8 sees 8 where the
unweighted walk every index site runs (0071e6d6, 0071e56b) sees 1
```

and never prints the refusal line, because 8 is below 10. **The weighting is not the displacement
bug**, and it could not have been: the value never reaches an index site in the image.

## 3. `0071F600` runs when the command BEGINS, not when the message arrives

`0071F600`'s callers: `00835D33`, inside `00835C70 BSP_WeaponDirector_BeginCurrentCommand`, plus
`0084DDDC` and two `.rdata` vtable slots. The argument decides which command it begins, and
`0071F618 CMP byte ptr [ESP+2Ch],0x0` / `0071F62F MOV EBP,[ESI+54h]` takes **slot 0**, the queue
head; the zero arm at `0071F738` is the override command at `+188h`. So the path object is built
for the command that is current, once, when the director begins it.

`00835C70`'s own head matters twice. `00835D02..00835D23`: when `[director+24Ch]+184h` is set -
the unit byte section 4 is about - and the head command is `cruise` (`00E08F70`) or `stop`
(`00E08F88`), it returns true **without** calling `0071F600` at all.

The 49 repeats a carrier's script sends do not begin 49 commands: `0071E6C0` refuses a push whose
descriptor and command match the slot below (`0071E70C` `0071E200`, `0071E721`), so they queue
nothing. This host was building on every message.

**Measured.** With the build moved behind `begin_current_command_00835c70`, called from the
director step before the `00836BF0` arm and offered again at the message:

| row | before | after |
| --- | --- | --- |
| `WeaponDirector::path_build_0071f600` | concrete 98 | concrete 6 |
| `Yorktown-class01` cursor `travelled` | 48.30 | 645.86 |
| `Lexington-class01` cursor row | present | gone: its `moveonpath` never becomes the head |
| `moved` / `total_path` | 100.51 / 36963.45 | 100.51 / 36963.45 |

The whole host-call census, diffed in both directions (1241 rows each way,
`local/drive_calldiff.txt`), moves exactly two rows: the path build above and
`PlatformLoopCallbacks::pretranslate` 19 -> 18, which is the Windows message pump and is not
deterministic. Nothing was lost, nothing was gained, and the world summary is identical to the
centimetre - the change is bookkeeping, and the 645.86 m of Yorktown hull path was already
happening in the before run without being counted.

## 4. The `cruise` state has a player-controlled arm, and the Lexington is in it

`009E1170`, the `cruise` state step. `009E11C2 CMP byte ptr [ECX+0x184],BL` with `BL = 0` and
`ECX = [ai+0AA8h]`, the unit: **clear** jumps to `009E1265`, the AI arm this host projects;
**set** falls into `009E11CE..009E1262`, which is a different routine:

```
009E11D6  [ai+3F4h] = 0 ; [ai+3F8h] = -1 ; [ai+3FCh] = 0 ; [ai+0B38h] = 1
009E11F9  COMISS XMM0,[state+8h] / JBE 009E1240   ; XMM0 = 00CE6848 = -10.0f, so the
                                                  ; capture below runs only while
                                                  ; [state+8h] is under the -10 sentinel
009E1207  [state+8h]  = [unit+994h]
009E1212  unit->vtable[5Ch](0Eh) ? [state+0Ch] = 0.0f : [state+0Ch] = [unit+998h]
009E1249  009DFFB0(state+4, [state+0Ch])       ; the same rudder setter the AI arm uses
009E1257  009DBF90(state+4, [state+8h])        ; the same throttle setter
009E1262  RET 4
```

So for that unit the cruise step **latches the unit's own pair at `+994h` / `+998h` once and
re-applies it every step**, instead of computing a desired throttle and rudder. It is the same
shape `docs/CRUISE_COMMAND.md` recovered for the latch on the AI side, over the player's live
pair. With no player input that pair is zero, and the ship stands still. `local/drive_before_usn04.log` is exactly that:
`Lexington-class01 state=cruise mode=rudder dir=stopped throttle=0.000` from the first sample, and
`moved=100.51` for the whole mission.

**The Lexington standing still with no player at the controls is faithful, and path following on
USN04 has to be judged on the Yorktown.** That was the integrator's hypothesis and this is the
listing behind it.

It is narrower than "the image suppresses the AI drive for the player's unit", and the difference
matters for the next packet. Neither `009E59C0` (`moveonpath`) nor `009E5770` (`movetopos`)
tests `+184h` at all - filtering both listings whole finds the byte in neither, and the only
`+0AA8h` loads in `009E59C0` are at `009E5A72`, `009E5BA0` and `009E5C20`, none of them a test.
A player-controlled ship under `moveonpath` still steers itself. What the player's unit loses is
the `cruise` state's computed pair and, through `00835D0C`, the begin of a `cruise` or `stop`
command.

### The whole census of `+184h` tests, which is what settles it

`docs/HANDOFF_SHIP_DRIVE_GATE.md` asks for the byte rather than the two distances: **does a
ship-AI DRIVE site test it?** Scanning the image for every `CMP`/`MOV` against `[reg+184h]`
(`38 99`, `80 b8`, `80 b9`, `80 ba`, `80 be`, `80 bf`, `8a 80`, `8a 81`, `8a 86`, `8a 87`, each
followed by `84 01 00 00`; `009E11C2` itself is in the result, which is the control that the
pattern occurs):

| site | function |
| --- | --- |
| `009E11C2` | `009E1170 BSP_ShipAi_CruiseStateStep` |
| `00835D0C` | `00835C70 BSP_WeaponDirector_BeginCurrentCommand` |
| `00836978`, `00836AAD`, `00836E45` | `00836920 BSP_WeaponDirector_Step` |
| `009F3DF3` | `009F3DD0 BSP_ShipAi_SyncStateToCurrentCommand` |
| `009F5E06` | `009F5DA0 BSP_WeaponDirector_AutoTargetTick` |
| `0099ADD0` | `0099ACD0 BSP_PilotBot_Tick` (the plane side) |

**`009F3F80 BSP_ShipAi_DriveOrderRing` is not in it, and neither is any state step.** So the
answer is: the gate is real and it is in the COMMAND layer, not the drive. The three sites in
the director step are the sharpest of them:

- `00836962..00836985`: with the primary stage running, `[unit+184h]` set falls into
  `PUSH 2 / 0071D810(2)` - the player's unit has its running primary command raised to the
  terminal stage every step. `0099C230` never appears on this path; this is the ship-side
  equivalent.
- `00836AAD` and `00836E45`: the set byte jumps over the `[unit+73Ch]+28h` commanded-speed test,
  in the stage arm and again in the idle tail's `00E08F60` branch.

This host already models the first of the three: `weapon_director_step_prepass_00836941` in
`src/unit_commanded_speed.cpp` raises the stage when `state.unit_player_controlled`. The other
two are not modelled and are named here rather than fixed.

> **Correction, packet `cc8_ship_command`.** The last sentence is wrong: **all three are
> modelled**, and had been since `f8abcd529` (2026-09-11), which is an ancestor of this doc's own
> commit. The claim was written without grepping the host.
>
> * `00836AAD` is `weapon_director_stop_arm_00836a8b`'s second test,
>   `if (!raise) raise = state.unit_player_controlled;`. The image raises when
>   `[unit+184h]` is set (`00836AB4 JNZ 00836ACE`), otherwise when the commanded speed is **not**
>   below `00D7A218` (`00836AC8 JC 00836D67` keeps the `stop` running). `00D7A218` is the float
>   0.0 and `navigator_commanded_speed_active` treats 0.0 as active, so the host's
>   `raise = count>1 || player || active(speed)` is the same predicate.
> * `00836E45` is `weapon_director_idle_reissue_00836dc9`'s
>   `const bool cruise = state.unit_player_controlled || navigator_commanded_speed_active(...)`.
>   The image jumps to the `00E08F70` `cruise` push when `[unit+184h]` is set
>   (`00836E4C JNZ 00836E7A`) and again when the speed is not below 0.0 (`00836E60 JNC`), and
>   pushes `00E08F88` `stop` only otherwise. Same predicate, same two pushes.
>
> Both branch senses were re-read from the listing to check the host's modelling is correct and
> not merely present. `docs/SHIP_COMMAND_LIFETIME.md` section 1 has the whole stage spine, and it
> also corrects what this section's `+184h` framing left out: `00836962`'s **other** arm,
> `0071BE60() > 1`, ends a running command for any unit, controlled or not, and that arm - not the
> `+184h` one - is what ended the Yorktown's `moveonpath`.

`unit+184h` is named "the controlled-unit byte" on the strength of the `009F3DF3` note
`src/game_hosts_ship_ai.cpp` already carries, plus these two uses and
`009281C0`, which clears it when controller slot 0 takes state 8
(`009281D1 CMP EBX,0x8`, `009281D6 MOV byte ptr [ESI+184h],0x0`). The **branch** is proved; the
**name** is a hypothesis.

## 5. The displacement: this host destroyed every director whenever a unit was created

`GameCommandsHost::register_units` ended `GameUnitsHost::create_units`, and `create_units` is
called again for every batch the mission spawns (`src/game_hosts_script_orders.cpp` has two such
producers; USN04 fulfils 8 `SpawnNew` requests and ends at 57 units from 21). It ran

```cpp
host.directors.assign(host.units.size(), GameDirector{});
host.navigator_params.assign(host.units.size(), bsp::CruiseSpeedSetting{});
```

which rebuilt **every** director: the ten command slots at `director+54h`, the queue mode, the
stage, the cruise latch, the slot-0 path object, and every unit's commanded speed. In the
executable a director is per-unit state built with its unit (`00720180` through `008363E0`) and
destroyed with it; creating another unit does not touch it. The units are appended, so the
existing indices do not move and `resize` keeps them.

**Measured**, `local/drive_keep_usn04.log` against `local/drive_after_usn04.log`, same binary
apart from the `resize`:

```
re-registered the unit table: 21 director(s) kept, 3 new     (then 24, 27, 30, ... to 57)
summary mission director  idle_reissues 407 -> 35   stop 393 -> 32   cruise 14 -> 3
summary mission commands  pushed 791 -> 409   current 704 -> 332   latched 12 -> 3
                          commanded_speeds 13 -> 3
CruiseCommand::raise_command_stage 0071d810   concrete 405 -> 35
GameUnitMessage::apply_set_command 00721a40   concrete 879 -> 498
CruiseState::set_desired_throttle 009dbf90    concrete 240 -> 1096
summary mission world     total_path 36963.45 -> 39835.45   moved 100.51 unchanged
```

Twelve wipes a mission, one per three-plane squadron batch. The census diff has 266 changed rows
and **no row gained and none lost** (`local/keep_calldiff.txt`), which is the shape a change like
this should have: the same work, done for longer, because the orders survive. The fleet covers
2.87 km more. The controlled Lexington still reads `moved=100.51` - section 4 is why - and the
gunnery totals move with the extra motion, which USN04 does not reproduce run to run anyway.

This is what made the `moveonpath` order look displaced *for the fleet*: the queue was not losing
to another command, it was being emptied from under every unit several times a mission, after
which the idle tail re-issued `cruise` or `stop` into the empty slot 0.

**The Lexington's own displacement is a different thing and it survives the fix.** Its command
rows in the same run are 24 consecutive `moveto ai_command_tick` rows with `curr=1`, i.e. the AI
coordinator's `00A02020` order taking slot 0 and holding it (104 calls a run, unchanged across all
three runs). The script's `moveonpath` sits behind it and never begins, which is why the run has
no Lexington cursor row at all. Whether the coordinator should be ordering the unit the player
controls is the open question, and it belongs to `src/game_hosts_ai.cpp`: the image has a
precedent for excluding that unit in `0099C230`, the gate all five bomber break-off routines
share.

## 6. `Party` on the `thisTable` slot: the writer is `00928F50`, not `00928A00`

`docs/MISSION_LUA_SELF_TABLE.md` has `00928A00` seeding `ID`, `Dead` and `Ptr` only, and the
string table agrees (`thisTable`, `Dead`, `Ptr`). The other writer is
`00928F50 BSP_MissionEntity_SetPartyRaceLuaMirror`, root entity vtable slot 11 reached through the
adjustor thunk `00951F30`: after forwarding to the base `00923B80` it takes `00927B40`'s object
and sets `Race` (`00928FD9`) and `Party` (`00929046`) through `00B67460`, reading both off the
entity rather than off its own arguments. It is already reconstructed as
`bsp::mission_entity_set_party_race_00928f50` in `src/mission_entity_lua_attach.cpp`; nothing in
this process called it.

`src/game_hosts_lua.cpp` runs it as its own pass, `mirror_party_race_00928f50`, from
`attach_script_orders` - one hop after the attach, which is the first point where this host can
reach a unit's party, and the same ordering the image has, since `00928F50` is a different virtual
call from `00928A00`'s. The keys line up: `recon`'s three index tables are `0`, `1` and `2`
(`src/recon_values.cpp`, the `index != 3` loop), this installation's `luamw_init.lua` 73-75 give
`PARTY_ALLIED 0`, `PARTY_JAPANESE 1`, `PARTY_NEUTRAL 2`, and `GameUnitRow::party` is in that same
space (the controlled Lexington reports `party 0`). `Race` is left alone: this host has no race.

**Measured**, `local/drive_party_usn04.log` against `local/drive_keep_usn04.log`:

```
thisTable: 00928f50's `Party` mirror written on 21 slot(s)
MissionLuaNative::GetSelectedUnit 008ab070   UNIMPLEMENTED 49 -> concrete 49
MissionEntity::set_party_race_lua_mirror 00928f50   new, concrete 1
MissionLuaNative::IsGUIActive 008ca010              new, UNIMPLEMENTED 49
MissionLuaNative::Music_Control_SetLevel 008c4d10   new, concrete 1
entity_resolves 132 -> 181       moved / total_path unchanged at 100.51 / 39835.45
```

No `script call Think failed` line anywhere in the run, and the census gains three rows and loses
none. The count on `GetSelectedUnit` does not move, which is the control
`docs/HANDOFF_USN04_LUA_NATIVES.md` asks for: the row resolves, it does not start answering rows
it should refuse. The two new rows are what `luaCheckMusic` reaches once `luaGetShipsAround`
returns instead of raising - it asks `IsGUIActive` 49 times and sets the music level once.

`SetSelectedUnit` `008AB260` at script line 1031 is reached once, `argc=1`, in `luaStageInit`, and
it is still a record: the script hands the entity table straight back and this host does nothing
with it. Binding it means driving `004C0890`, which is the next packet's.

Correction to `docs/SHIP_AI_PATH_CURSOR.md` section 9, which says the missing half is "`Party`
(and `Name`, which the same file's helpers read)": `Name` is **not** needed. Every `.Name` read in
`luaGetShipsAround` (this installation's `scripts/global/commandhelpers.lua` 322, 333) is inside a
`RELEASE_LOGOFF` comment. The live reads on that path are `target.ID`, `targetUnit.Party` and
`targetUnit.ID`, and `recon[party][allegiance]` is already a table for any of the three party
indices `00803A40` installs, so an empty recon map returns an empty list instead of raising.

## 7. Coverage

| routine | coverage |
| --- | --- |
| `0071D780` | complete, read whole, `0071D780-0071D807` |
| `0071E6C0` | the capacity test, the slot walk and the duplicate refusal, `0071E6C0-0071E70C`; the tail from `0071E72A` was already projected |
| `0071E550` | the head only, `0071E550-0071E582`, for the unweighted walk |
| `0071F600` | the argument test and the queue-head fetch, `0071F618-0071F62F`; the arm itself is `docs/SHIP_AI_PATH_CURSOR.md`'s |
| `00835C70` | `00835C70-00835D3A`, the player-controlled early return and the `0071F600` call |
| `009E1170` | the player-controlled arm `009E11C2-009E1262`, read whole; the AI arm from `009E1265` was already projected |
| `009281C0` | complete, read whole |
| `00928F50` | not re-read; `docs/MISSION_ENTITY_LUA_ATTACH.md`'s reading is cited |

## 8. What is still open

1. **No arrival.** All three runs end with `advances 0`. The Yorktown closes about 2.3 km of a
   5.8 km leg in the 150 s window - `d32c` still falls 8.33 m per ten steps at step 3000, with
   `throttle 1.000` and a live `rudder -0.124` - and the arrival radius is
   `min(unit+9C8h * 2.5, 0082E850 * 1.2)`, a few hundred metres. A run long enough to cover the
   leg is what decides whether the cursor walks; nothing in the chain is known to be missing.
   `travelled` is measured from the last BEGIN, so with two begins in the third run its 633.39 m
   is the 38 s since the second one, not the mission.
2. **One leg per director step.** `007ADD70`'s loop can take several; unchanged from
   `docs/SHIP_AI_PATH_CURSOR.md`.
3. **Two of the director step's three player-controlled arms are not modelled**, `00836AAD` and
   `00836E45`, both of which skip the `[unit+73Ch]+28h` commanded-speed test for the player's
   unit. `00836978`'s is already in `weapon_director_step_prepass_00836941`.
4. **The begin's place in the frame.** `00835C70` is reached only through the director vtable
   slots `00D09FD0` / `00D09FD4`, so where in a frame it runs is not established; this host begins
   at the director step and again at the message, which is the same choice milestone 2m made for
   the `cruise` begin.
