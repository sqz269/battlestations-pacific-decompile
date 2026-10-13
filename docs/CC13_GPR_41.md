# CC13 GPR 41: segment 41 reference models (land convoy, land fort, landing ship, triggers)

Packet `cc13-gpr41` (cc13 lineage, Claude). Segment 41 is `00735F30`-`00758EB0`. cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg41_cc13.cpp` and `include/bsp/gameplay_seg41_cc13.hpp`,
  registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

The names come from `docs/CC13_GP_0071_0078.md` (cc13-gp71) and `docs/LAND_AND_STRUCTURES.md`. Every
comparison, constant and float rounding below was re-read from the disk bytes (Capstone `disasm-raw`; the
strings and constants were read from the PE). The x87 was traced with `sc2_x87.py` where it does more than
one operation. Everything here is **reconstructed and build-tested**. None of it is ABI-compatible, wired or
game-validated.

Segment 41 is mostly not gameplay: about 120 of its 333 unrecorded functions are application, VFS, decal and
telemetry code (`00735F30`-`00741100` and `00750800`-`00757FFF`). The gameplay part is `007410B0`-`00750790`
and the mothership tail `00758090`-`00758E80`.

## 1. Models

### 1.1 LandConvoy damage pooling (class 1Ah, vtable `00CEA570`)

A convoy member (MLandVehicle, class 19h, vtable `00CFFDE0`) holds its convoy at `+738h`. The convoy's
roster is the vector `+398h`; `InitialSize` (`+3B8h`), `CollectedDamage` (`+3C0h`) and `LastHit` (`+3C4h`)
are the Lua property names that `007416F0` reads.

| address | model | what it does |
| --- | --- | --- |
| `0074CD30` | `add_damage_parent_aware_0074cd30` | The member's vtable[1ACh]. The base AddDamage `0095DA00` runs only with no convoy, with the convoy's reentry byte `+390h` set, or with convoy `+364h` exactly `0.0` (UCOMISS/LAHF/JP at `0074CD4E`; a NaN fails). Then, unless reentering, `00742330`. |
| `00742330` | `on_member_damaged_00742330` | With `+364h > 0` (COMISS/JBE) it sets `+390h`, sends the amount to the **lead member's** vtable[1ACh] (which takes the base because `+390h` is set), clears `+390h`, and kills the roster (`00742210(0)`) when the lead's `+370h` equals `0.0`. It always adds the amount to `+3C0h` (FADD, FSTP float) and stores the clock `[00F876A4]` in `+3C4h`. |
| `00742940` | `health_00742940` | Convoy vtable[110h]. With `+364h > 0` it returns the lead's `00923BE0`. Otherwise it sums `00923BE0` over the non-null members (each sum stored as a float) and returns `float(sum / InitialSize)` (FIDIV `+3B8h`). |
| `00741F20` | `set_invincible_00741f20` | Convoy vtable[0F4h]: `0042ED80`, then each non-null member's vtable[0F4h]. |
| `00741140`, `00741160` | `set_speed_00741140`, `set_field_344_00741160` | Store `+368h` (Lua `Speed`) and `+344h`. |

So `+364h` selects the mode. Positive: every member's hit goes to the lead member, and the convoy dies with
the lead. Zero: each member takes its own hits. Negative: no member takes damage at all; only `+3C0h`
counts it.

`+364h` is the convoy's scene property `HP` (cc13-gpr41b): `00743450` reads it from the property bag at
`00743552`..`00743573` (key `00CE6750` "HP"; an integer value goes through CVTSI2SS). The template
`universe/library/landconvoy.props` defaults it to `F 0.0`. In this installation's scenes 31 LandConvoy
entities exist. 29 do not author HP, so they use 0 (own damage). The two `Train` convoys of
`usn_12_indochina.scn` author `HP = F 1.0`, so they pool. The host already loads the value as
`SceneContentsRecord::convoy_hp` (`src/game_hosts_scene_contents.cpp:424`); it has no consumer.

### 1.2 Land fort and command building (MLandFort 1Bh `00CFF3F8`, MCommandBuilding 1Ch `00CFB028`)

`+738h` is the group owner ("master", `land_and_structures.hpp` `kLandGroupOwnerOffset`). `00744A20` is
called by the airfield and shipyard property readers (`006D5220`, `00849F70`), so hangars and markers are
sections whose master is the airfield or the shipyard.

| address | model | what it does |
| --- | --- | --- |
| `00743F50` | `supports_command_00743f50` | vtable[168h] (both classes). True for `settarget`, `clearorders`, `cleartarget` (`00CF8B0C`, `00CF8AF4`, `00CF8B00`; `00438E10`, case-insensitive). |
| `00744B30` | `may_follow_target_00744b30` | vtable[16Ch] (both classes). It requires `00743F50`. With no target the answer is `!command->vtable[8h]()` (`00467170` finds the command). With a target, it must be kind 5, 18h or 1Ah; `+5Dh` (dead) must be clear; and `00803510(self+54h, target+54h)` must be nonzero. A kind 8 (submarine) target must also pass `00852820`. |
| `00744A20` | `set_master_00744a20` | Stores `+738h` and `+73Ch`. With a master: when this party is at least 2 and the master's is below 2, it adopts the master's party (vtable[2Ch]). The master's `+36Ch` is set to this unit's, and the master's health to this unit's `+370h` (`00877B90`). |
| `00747400` | `die_00747400` | MLandFort vtable[1A8h]. Already dead: nothing. `+71Ch` set: `vtable[194h]("InferiorFailure", 0)`, and the unit does not die. `+50h` clear: nothing. **Any live element of the `+48h`/`+44h` chain: nothing** (`007474A0 CMP [EAX+5Dh],0; JE 007474CC`). Otherwise it sets its last damage level (vtable[1B4h]) and destroys itself (vtable[70h](1) = `0077D1A0`). |
| `00747170` | `on_destroyed_00747170` | vtable[7Ch] (both classes). See 1.2.1. |
| `007465E0` | `start_death_fire_007465e0` | See 1.2.2. |
| `00745F1E` (fragment) | `smoke_frame_00745f1e` | The `+744h` arm of the frame update `00745BE0`. `+748h -= dt` and `+750h += dt` (each stored as a float). When `0 >= +748h` (FLDZ/FCOMIP/JB, ordered), the smoke stops (`00867B10`, `00484620(0)`). |

The `LAND_AND_STRUCTURES.md` text for `00747400` says "no already-dead ancestor in the `+48h`/`+44h` chain".
The bytes say the opposite. One element with `+5Dh` clear (alive) ends the rule, so the fort dies only when
every chain element is already dead.

#### 1.2.1 `00747170` OnDestroyed

The body runs only when state `+70h == 1` and the clock is above 1.0 (`[00D7A24C]`, COMISS/JBE). Then:
1. Its own damage level is set to the last one (`0072AB10`, vtable[1B4h]).
2. The part render list is rebuilt when `+360h` has a list at `+19Ch` (scene work).
3. **The master takes `1e10` damage** through its vtable[1ACh] (`[00CE4970]`). When the master's `+5Ch` is
   set and `+5Dh`, `+60h` and `+5Eh` are clear, its vtable[1A8h] (the master's destruction rule) runs.
4. The death effect (`ExplosionEfx`, class `+13Ch`, at the first class `+1Ch` record's point) is created.
5. When class `+164h` (SmokeFireChanceMul) is above 0, `007465E0` runs.
6. Each `+48h`/`+44h` chain element is set to its own last damage level.
7. The base `00959450` always runs, including when the gate in the first line fails.

#### 1.2.2 `007465E0` StartDeathFire

1. A burning effect `+740h` (the fire `00746320` starts) is stopped and released.
2. If a smoke `+744h` already exists, it draws `d = U(class+168h, class+16Ch)` on **stream 0**
   (`0074679D`, ECX = 0). `+748h` keeps its value when that value is larger, otherwise it takes `d`
   (FCOMIP/JBE, so a tie takes `d`).
3. Otherwise, with no `SmokeFireEfx` (class `+160h`, one handle stored at `00749379` by `00749210`) it
   returns without drawing.
4. Otherwise it creates the smoke into `+744h` and draws `d` on stream 0 (`0074672B`) into `+748h`.
   **The draw happens even when the effect is not created.** With an effect, it sends the
   LANDFORT_EFFECT message `007448B0(7, d)` through `0077C2A0(m, 4, 0)`.

`00745BE0`'s `+740h` arm (hosted by cc9) falls through into the `+744h` arm at `00745F16`. When both effects
exist, `+748h` and `+750h` move twice per frame.

#### 1.2.3 The section-to-master forward in `007470B0` (not modelled; for cc9)

`007470B0` (MLandFort vtable[1ACh]) does more than cc9's host `landfort_fire_roll_007470b0` covers. After
the roll and the base AddDamage `0095DA00` (`00747136`), it forwards the same amount to the master:
`0074713B CMP [ESI+738h],0` ... `0074715A CALL [EDX+1ACh]`. So every hit on a hangar (a section) also
damages its airfield. When the section dies, `00747170` adds `1e10` on top.

### 1.3 MLandingShip (vtable `00CFFA30`; tick element at unit+310h)

| address | model | what it does |
| --- | --- | --- |
| `0074A420` | `lower_ramp_0074a420` | Sets `+1188h` and `+1189h`. `+11B4h` gets the world position of the first `+1198h` record's node (4Ch stride). `+11C0h` gets a copy of the pose `+74h`. |
| `0074AC30` | `place_step_pose_0074ac30` | Tick slot +4h. With `+1189h` clear it runs the motion controller `00811AB0(dt)`. With it set it **holds the pose**: `+74h = +11C0h`, `+C8h = +10Ch = 0`, and the `+48h`/`+44h` chain is invalidated. Only InitAll (`0074BEFB`) and the save load (`0074C347`) clear `+1189h`, so once the ramp is down the ship stays where it lowered it. |
| `0074BCA0` | `get_ramp_point_0074bca0` | vtable[24Ch]: `+1214h` with the flag, else `+1220h`. |
| `0074BBD0` | `small_and_moving_0074bbd0` | vtable[244h]: class `Length` (`+A0h`) below 80.0 and speed (vtable[38h]) above 3.0, both ordered. |
| `00749810` | `slot240_00749810` | vtable[240h]: false while on a pad (`+1200h`), else the base `008128E0(0)`. |

### 1.3.1 Landing ship names, message handler and the rest of the update (cc13-gpr41b)

The save section `"_landingship"` (`0074A1F0`) names the landing ship's own fields in Hungarian:

| offset | key | meaning |
| --- | --- | --- |
| `+1188h` | `nyitva` | open (ramp down) |
| `+1189h` | `horgony` | anchor; the pose hold of 1.3 |
| `+118Ah` | `partraszallas` | landing |
| `+118Bh` | `gyorsPartraszallas` | fast landing |
| `+118Ch` | `partraszalltunk` | we have landed |
| `+11A4h` | `rampaElfordulas` | ramp turn (0..1) |
| `+11A8h` | `lastTalaj` | last ground (time) |
| `+11ACh` | `nyitzarTimer` | open/close timer |
| `+11B4h` | `horgonyPont` | anchor point |
| `+11C0h` | `horgonyMatrix` | anchor matrix |

This confirms the reading of `+1189h` as an anchor.

| address | model | what it does |
| --- | --- | --- |
| `0074B570` | `handle_message_0074b570` | vtable[164h]. On `0A5h` (LandAtPad, built by `00749D90`) it calls `vtable[148h](1FFh, 8)` (`0077F360`, SetRoleAvailable), then `0074A990(pad handle +1Ch, building handle +1Eh)`, then shows the HUD prompt when the ship is the selected unit. On `0A6h` it runs `0074A420`. Any other kind goes to the ship base `00821E80`. |
| `0074B0B0` (fragment) | `capture_ship_warning_gate_0074b0b0` | Inside `0074AF50`, every frame for a live ship: with the ramp up, no pad, vtable[240h] true, and a current weapon command (`0080E1B0`) whose vtable[8h] is false, it reports the HUD `captureship` warning (`00977DA0` on `[00F8A0C4]`). Presentation. |
| `0074B329` (fragment) | `ramp_node_step_0074b44b` | When `rampaElfordulas` changed this frame (FUCOMIP/JNP, so a NaN also runs), each `+1198h` record sets its own `+44h = f((+48h - +44h) * r + +44h)`. **This writes into the lerp's own start**, so the turn compounds over frames instead of interpolating from a fixed start. The node then gets `RotationX(-0.0 - turn)` times its rest matrix `+4h`. Presentation. |

With cc9's two fragments (`0074AFCC`, `0074B109`), this covers `0074AF50`, apart from the base call `008255B0` and the
debug tail `0051F460`.

### 1.4 Lua mission triggers (base `00D00554`)

The natives `luaMW_AddProximityTrigger` (`008C9730`), `AddLineOfSightTrigger` (`008C9460`) and
`AddSquadOrdnanceDroppedTrigger` (`008C9A10`) create these triggers. Each trigger's check is its
vtable[0Ch](dt). A hit calls `0074FDD0`, the Lua callback. Whether a trigger fires once or repeatedly is
decided there and is not modelled.

| address | model | what it does |
| --- | --- | --- |
| `0074FEE0`, `007500E0` (partial) | `trigger_radius_squared` | The constructors store `r * r` as a float (`+5Ch` proximity, `+48h` line of sight). The line-of-sight timer `+4Ch` starts at 0. |
| `00750000` | `proximity_check_00750000` | Planar test. `dx` and `dz` are stored as floats, `d2 = (dx*dx + 0*0) + dz*dz` is stored as a float, and the trigger fires when `+5Ch > d2`. The target is the descriptor `+44h`, resolved by `006F7DD0`. |
| `00750230` | `line_of_sight_check_00750230` | 3-D `d2 = (dx*dx + dy*dy) + dz*dz`. While `+48h > d2`, `+4Ch -= dt`. When it drops below 0 it adds 0.5 (qword `00D7A280`) and casts `0084B6B0` from the watched entity toward `+44h`; for a squadron (kind 18h) the cast starts from its `+3D0h`. It fires when the first hit is `+44h`. So the cast runs at most every 0.5 s while in range, and at once on entry. |
| `007504F0` | `squad_ordnance_check_007504f0` | `+30h` null: vtable[10h] (target gone). Otherwise it fires when `007ED790(squadron, 0)` answers false (nothing left to drop). |

### 1.5 Ship leak manager (unit+10D4h; cc13-gpr41b)

The save section names the leak manager's fields: `"leak"` (`+18h`, one float per zone), `"water"` (`+1Ch`),
`"isWater"` (`+24h`, a byte), `"sumForces"` (`+28h`) and `"sumLeaks"` (`+2Ch`). These are the segment's
"sumforces" and "sumleaks" keywords. The count is `+14h` (6, from `0074F490`).

| address | model | what it does |
| --- | --- | --- |
| `0074E830` | `clear_all_zones_0074e830` | Zeroes water (`+1Ch`), then leak (`+18h`). The arm of `91h` MT_SHIP_LEAKCHEATWATER (`008221ED`). |
| `0074E860` | `load_zone_array_0074e860` | Copies `count` (msg+1Ch) floats from msg+20h into `+18h`, four per pass and then singly. It does not bound `count` by `+14h`; the model refuses such a count. This is the arm of `92h` MT_SHIP_SINK (`0082220D`), which cc9's `ship_wreck_sink_block_00824fe5` applies at once. |
| `0074EDC0`, `0074EF20` | `kLeakSaveKeys_0074edc0` | The read and write visitors. Both use the same keys in the same order. |

`0074F090` (AddLeak) and `0074EC50` (the wreck redistribution, with one stream-1 `U(0,1)` per leak at
`0074ECA5`) are already transcribed in `src/game_hosts_units.cpp`; they have pointer records.

### 1.6 Fort message router and the unit type name table (cc13-gpr41b)

| address | model | what it does |
| --- | --- | --- |
| `00744BE0` | `handle_message_00744be0` | vtable[164h] for both fort classes. `4Bh` (MT_ROLEOWNER): the base, with the answer forced true. `D1h` (MT_LANDFORT_EFFECT): `msg+28h` set -> vtable[208h] `007460E0(kind, float duration)`, the replica of `007465E0`'s message (kind 7, SmokeFireEfx); else vtable[20Ch] `00745FA0(kind, msg+20h)`. Both effect slots are presentation. Any other kind: the base answer. |
| `00746B90` | `unit_type_from_name_00746b90` | Case-insensitive (`00425850`), in image order: Destroyer 7, Cruiser 0Ah, LandingShip 0Ch, Cargo 0Bh, BattleShip 0Dh, Submarine 8, TorpedoBoat 0Eh, MotherShip 9, ReconPlane 14h, Fighter 13h, DiveBomber 12h, TorpedoBomber 11h, Kamikaze 17h, Bomber 10h, AirField 45h, Shipyard 46h, LandVehicle 19h, LandFort 1Bh. A nil value or no match returns the default. The only caller is `00749210`'s `FakedType` (class `+178h`), which no row in this installation authors (`vehicle_class_fields.cpp:487`). |

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `add_damage_parent_aware_0074cd30`, `on_member_damaged_00742330`, `health_00742940`, `set_invincible_00741f20` | the single AddDamage in `ShipHit::add_damage`, `src/game_hosts_gunnery.cpp:9797`, applied to every victim. The host keeps the convoy only as a name (`game_hosts_units.cpp:11436`, `row.land_convoy_738`). | the member's convoy (the units host's `land_convoy_738`); the convoy's `+364h` = scene `HP`, already loaded as `convoy_hp` (`game_hosts_scene_contents.cpp:424`); the roster order (`build_land_convoy_roster_00743450`, `game_hosts_units.cpp:11402`); member health `+370h`; the clock | none | OFF: none. ON: in this installation only USN12's two `Train` convoys pool (`HP = 1.0`); every other convoy keeps its own damage. With `+364h > 0`: hits on any member land on the lead, and killing the lead kills the whole convoy. With `+364h == 0`: the same as now, plus `CollectedDamage`/`LastHit` for Lua. With `+364h < 0`: convoy members become unkillable by hits. Scripts that read `CollectedDamage` or `LastHit` (`007416F0`) see values. |
| `die_00747400` | `GameGunneryHost::Impl::kill_unit`, `src/game_hosts_gunnery.cpp:11318`, which kills every unit at health 0. The command-building exception (`006F3270`) is already there; MLandFort's vtable[1A8h] is `00747400`. | `+71Ch` (`slot.parent_link_71c`, `game_hosts_units.cpp:1824`); `+50h` (not in the host; 3.2); the `+48h`/`+44h` chain's dead flags (the scene tree; 3.2) | none | OFF: none. ON: a land fort with a CommandBuilding parent (`+71Ch`) never dies; it raises InferiorFailure (message 7Dh, `+720h`) instead, as the airfield rule (`kAirfieldDestructionRuleBound`) already does. A fort whose chain holds a live element survives at 0 health. Kill counts and objective deaths for forts can drop. |
| `on_destroyed_00747170` | the same `kill_unit` (`:11318`): its death effects and the master hand-off | master `+738h` (`00744A20` callers: the airfield and shipyard readers `006D5220`, `00849F70`); class `+164h` and the effect rows (`vehicle_class_fields.cpp`, `00749210`); the state `+70h`; the clock | none directly. The master's AddDamage(1e10) reaches the master's vtable[1ACh] (`007470B0` for a fort: one stream-1 roll and possibly stream-0 fire draws, which cc9 already hosts; `006F1F20` for a command building) | OFF: none. ON: a destroyed hangar or marker hands its airfield or shipyard 1e10 damage. The airfield's destruction rule (`006D40F0`, hosted) then decides whether the airfield dies. When the master is a land fort, the stream-1 roll and any fire draws shift later stream-1 and stream-0 consumers. |
| `start_death_fire_007465e0`, `smoke_frame_00745f1e` | none. cc9's `src/game_hosts_ship_ai.cpp:12559` says "The +744h arm (00745F1E..) has no producer here"; `:12526` notes that `+744h` is set only by `007465E0` | class `+160h` (`SmokeFireEfx`), `+168h`/`+16Ch` (shared with the `+740h` fire, already in `CaptureBuilding`); the fire state `fire_740` / `fire_time_748` / `secondary_clock_750` | one **stream-0** `U(+168h, +16Ch)` per call when a smoke exists or the definition is present (`0074672B` / `0074679D`, ECX = 0). Gameplay: it sets `+748h`, which the frame update consumes. It goes through `LandFortHost::uniform` in image order. | OFF: none. ON: each land fort destroyed with SmokeFireChanceMul > 0 and a death-smoke definition takes one stream-0 draw at death, so every later stream-0 consumer shifts. A burning fort's fire (`+740h`) stops at death, so cc9's `+740h` arm and its secondary-explosion roll stop for that fort; the smoke then counts `+748h` down and `+750h` up until it ends. |
| `may_follow_target_00744b30`, `supports_command_00743f50` | the generic follow test `bsp::entity_may_follow_target_00779d50` at `src/game_hosts_script_orders.cpp:3046` (and `game_hosts_ai.cpp:2964`), asked for every follower; MLandFort and MCommandBuilding install `00744B30` at vtable[16Ch] | the order token; the target's kind (5, 18h, 1Ah, 8), `+5Dh` and `+54h`; `00803510`; `00852820` for submarines; the command registry's slot 8 for a token with no target | none | OFF: none. ON: script orders to forts and command buildings other than settarget, clearorders and cleartarget are refused. `settarget` to a submerged submarine is refused. |
| `set_master_00744a20` | none found (no host calls `00744A20`) | the section's and the master's `+54h`, `+36Ch`, `+370h` | none | ON: the master starts at the section's health and max health. A neutral section (party 2 or more) joins its master's side. |
| `lower_ramp_0074a420`, `place_step_pose_0074ac30` | the ramp latch `landing_ship_ramp_latch_0074afcc` (`include/bsp/building_pads.hpp:207`; true means `0074A420` runs), which sets only `ramp_down_1188` | the pose at the lowering; the first `+1198h` record's node position | none | OFF: none. ON: a landing ship whose ramp is down holds the captured pose for the rest of the mission. The motion controller `00811AB0` no longer runs for it, so drift or turning after the landing stops. |
| `small_and_moving_0074bbd0`, `slot240_00749810`, `get_ramp_point_0074bca0` | none found (the callers of vtable[240h]/[244h]/[24Ch] were not traced) | class `Length`, speed, `+1200h`, `+1214h`/`+1220h` | none | none known until a caller is bound |
| `handle_message_0074b570` | the LandAtPad delivery at `src/game_hosts_ship_ai.cpp:6020` and `:13093` (delivered at once to `begin_landing_0074a990`; the `vtable[148h]` call is a record there) | the two handles in the message; the role bookkeeping (`0077F360`) | none | ON: a landing ship that starts landing runs the role bookkeeping `0077F360(1FFh, 8)`, which the host does not run today (`docs/SCRIPTED_HELM.md` section 6 has the role mask). The player-role effect during a landing was not measured. |
| `ramp_node_step_0074b44b`, `capture_ship_warning_gate_0074b0b0` | none (presentation) | the ramp records and `rampaElfordulas`; the weapon command | none | ON: ramp meshes turn and the capture-ship HUD warning shows; no gameplay change |
| `clear_all_zones_0074e830`, `load_zone_array_0074e860`, `kLeakSaveKeys_0074edc0` | the 92h substitution in `ship_wreck_sink_block_00824fe5` (`src/game_hosts_units.cpp:3240`), which applies the rates directly; the 91h cheat has no host | the message count and payload; the save archive | none | OFF: none. ON: the same leak state, delivered through the message; the cheat clears water and leaks; saves carry `sumForces`/`sumLeaks` |
| `handle_message_00744be0`, `unit_type_from_name_00746b90` | none | the message kind and flag; the class row's FakedType | none | none in this installation (the effect slots are presentation; no row authors FakedType) |
| `proximity_check_00750000`, `line_of_sight_check_00750230`, `squad_ordnance_check_007504f0` | none; `src/mission_lua_host.cpp:516`-`518` lists the three natives unbound (binding the natives is cc9's Lua lane) | the watched unit's pose; the target descriptor; the radius; `0084B6B0` (spatial cast); `007ED790` | none | OFF: none. ON (after the natives are bound): missions that register these triggers get their callbacks. Which missions use them was not counted. |

## 3. Open questions

1. **Convoy `+364h`**: resolved in cc13-gpr41b. It is the scene property `HP` (1.1).
2. **`00747400`'s `+50h` and the chain.** `+50h` has no host meaning yet. The `+48h`/`+44h` chain is a scene
   link: the same walk in `0074AC30` invalidates poses. Whether it lists sections or children was not
   settled. The doc text in `LAND_AND_STRUCTURES.md` reads the condition the other way (1.2).
3. **`007470B0`'s master forward** (1.2.3) is not in cc9's host.
4. **`00744B30` with no target** depends on the command object's vtable[8h], which was not read.
5. **`sumForces` (`+28h`) and `sumLeaks` (`+2Ch`)** have names now, but no writer was traced in this segment.
6. The mothership tail (`00758090`-`00758E80`, runway/hangar failure routing and the elevator) is air ops
   and was left alone. Its failure flags are partly hosted (`air_operations.cpp:372`).
7. The land vehicle's replication (`0074D030`, `0074D100`, `0074D230`), creation messages and
   serializers were not modelled; they matter only in multiplayer or save/load.

## 4. Ledger

Function records (`config/reconstruction`), models: `00741140`, `00741160`, `0074CD30`, `00742330`,
`00742940`, `00741F20`, `00743F50`, `00744B30`, `00744A20`, `00747400`, `007465E0`, `00747170`, `0074A420`,
`0074AC30`, `0074BCA0`, `0074BBD0`, `00749810`, `00750000`, `00750230`, `007504F0`, `0074FEE0` (partial),
`007500E0` (partial). Fragment: `00745F1E`.

Host pointers, each checked against the listing at the cited instructions: `007470B0` (partial, see 1.2.3),
`00745BE0` (partial), `0074F490`, `0074F440`, `0074E9C0`, `00743450`, `007410B0`, `00741E90`, `0074CDA0`,
`0074A990`, `0074AD90`, `0074A4C0`, `00749B20`, `0074AF50` (partial).

cc13-gpr41b added models `0074B570`, `0074E830`, `0074E860`, `0074EDC0`, `0074EF20`, `0074A1F0` (names only),
`00744BE0`, `00746B90`; fragments `0074B0B0`, `0074B329`; host pointers `0074F090`, `0074EC50` (partial),
`0074BC10`.

## 5. Verification

- `./scripts/build.ps1` passed in this tree (bsp_game built with `gameplay_seg41_cc13.cpp`; ctest 2 of 2, `reconstructed_math` and `tool_tests`). No test was added; the models are host-callback reference code and cc9 binds them OFF.
- `tools/x87_dc_form_scan.py --range 00741100-00751000 --include-st0` finds five DC-form sites in the
  modelled functions: `0074FF4E`, `00750064`, `00750068`, `0075014C`, `007502FE`. All five are
  `fmul st(0), st(0)` squares and agree with the models. The other DC sites in the range are in hosted
  functions (`00742400`, `0074AF50`, the leak manager), not in these models.
- `sc2_x87.py 007502C2 0075033C` gives `(dy*dy + dx*dx) + dz*dz`, one float store, matching the model.
  `00745F38 00745F6F` gives `+748h = f(+748h - dt)` and `+750h = f(dt + +750h)`.
- `tools/verify_report_calls.py local/g41_calls.json` checked 35 call-site rows with 0 failures, and
  `local/g41_calls2.json` (cc13-gpr41b) checked 12 with 0 failures. Neither file is committed.
- cc13-gpr41b rebuilt after its changes: `./scripts/build.ps1` passed (ctest 2 of 2).

## 6. Handoff

Done: the convoy damage pooling, the land fort destruction rule, death handler and death smoke, the fort
command filter, the landing ship ramp anchor and three small overrides, and the three trigger checks. There
are 23 model records and 14 host-pointer records.

Left in segment 41, in order of gameplay reach:
- The convoy `+364h` question is resolved (scene `HP`; only USN12's trains pool).
- **The fort `+50h` flag and the meaning of the `+48h`/`+44h` chain** (3.2).
- **The mothership tail** `00758090`-`00758E80` (air ops lane).
- **The rest of the landing ship**: `0074BEC0` (InitAll) and `0074C280` (the save load). cc13-gpr41b covered
  `0074B570`, the rest of `0074AF50` and `0074BC10`.
- **The leak manager**: done in cc13-gpr41b (1.5). The writers of `sumForces` and `sumLeaks` are still open.
- About 120 non-gameplay functions: application, VFS, decals and telemetry.