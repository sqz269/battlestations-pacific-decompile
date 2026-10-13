# CC13 reference models, segment 33: bomb platforms and their device reload (cc13-gpr33)

Worker cc13-gpr33, 2026-10-13, branch `agent/cc13-gpr33`. A reference-model pass over segment 33
(`006DECA0`-`006FA320`, the bullet and bomb classes), on cc9's terms: new `*_cc13` files only, ledger
records, no host edits, no wiring, no game runs.

Status words follow AGENTS.md. **Reconstructed, build-tested** applies to
`src/gameplay_seg33_cc13.cpp` / `include/bsp/gameplay_seg33_cc13.hpp` (registered for `bsp_game` by
one appended line in `cmake/startup.cmake`). Nothing is fixture-tested, ABI-compatible or
game-validated. **Pointer record** means a ledger record whose `source` is an existing file that
already reconstructs the function; no code was added for it. Names are hypotheses.

Lease: range `006DECA0-006FA320` and the three owned files. Skipped as instructed: the rocket path
(`0080A000`, `0080AA20`, `009FD0E0`), the plane crash sweep `007C0910`, gun bots, command classes,
command buildings and air ops. No Ghidra mutation was made (every function here already had a
reviewed name from cc13-gp6b, docs/CC13_GP_006B_0070.md).

## Counts

| | count |
|---|---|
| functions in the segment with a name but no record or tag at the start (`bsp.py range`) | 425 |
| of those, mentioned by address anywhere in `src/` or `include/` | 141 |
| records added, own reconstructions (`source` = `src/gameplay_seg33_cc13.cpp`) | 36 |
| records added, pointer records to existing sources | 18 |
| **records added** | **54** |

## Why the bomb platforms

The host's rack tick `run_rack_tick_006e56f0` (`src/game_hosts_units.cpp:26935`) models the
`toRepeatTime` countdown and the fire window of `006E56F0`, but not the slice between them,
`006E57A4`-`006E57D2`:

```
006E57A4  MOV ECX,[ESI+0E0h]      ; owner
006E57AC  MOV EDX,[EAX+1E4h]      ; owner vtable[1E4h]: the plane answers 006D1F40 = byte [00E17BF2]
006E57B6  JZ  006E57D4
006E57C2  MOV EDX,[EAX+204h]      ; rack vtable[204h] = 006E41C0 (single) / 006E44C0 (multi)
006E57D2  CALL EDX                ; (dt)
```

`[00E17BF2]` is the device-reload flag the Lua native `SetDeviceReloadEnabled` (`008C1350`) writes
(the host already keeps it: `lua_device_reload_enabled_00e17bf2`, `src/game_hosts_lua.cpp:6045`). In
this installation's base campaigns it is set by `scripts/missions/ijn/JM/jm01`, `jm02`, `jm04`,
`jm06`, `jm08`, `jm09`, `jm12`, `jm13`, `jm14` and `usn/usn_19_coralus`; `jm01` also calls
`SetDeviceReloadTimeMul(3)` and `jm03` `SetDeviceReloadTimeMul(0)` (`008C14B0` stores the number
straight into `[00E08178]`, `008C15B4`, which both reload bodies multiply dt by). So with the flag set,
an empty rack counts its ReloadTime down in flight and refills.

The multiple rack (`MMultipleBombPlatform`, class 26h, every Corsair, Raiden, Shinden and Kikka rocket
set per the host's note at `src/game_hosts_units.cpp:575`) has no host model at all: the host
keeps a substitute count for it (`src/game_hosts_units.cpp:4060`, "a MultiBombPlatform keeps the
substitute, its count being unread"). Its round bookkeeping is modelled here.

## The single rack (`MBombPlatform`, 25h, vtable `00CF96A8`)

Fields (serializer names from `006E4A60` where they exist): `+484h` ammo, `+488h` orgAmmo, `+48Ch`
reload time, `+490h` reload countdown, `+494h` toRepeatTime, `+498h` dropBombs. The constructor
`006E3C00` sets `+48Ch = +490h = 30.0` (`00CE38C8`). The bag Equipment entry gives Ammo
(`vtable[1BCh]` = `006E3530`) and ReloadTime (`vtable[1C0h]` = `006E3A20`, below 1.0 becomes 1.0)
(docs/SQUADRON_LAND_TASK.md, `0095A880`).

The reload cycle:

1. `006E41C0` (slot `+204h`): step = `[00E08178]` x dt, stored as a float. Only while the rack is empty
   (`vtable[20Ch]` = `006E3570`: attached 2Ah rounds + ammo == 0), `+490h -= step`. Below zero, in
   session modes 0/1, the reload message B2h (`006E3A60`, argument -1) is routed (`0077C2A0(1,0)`) and
   `+490h = +48Ch`; in mode 2, `+490h = 0`.
2. The message reaches `006E4290`, whose B2h arm calls `vtable[200h]` = `006E3410` (Rearm):
   `+490h = +48Ch`; with a round still attached `ammo = orgAmmo - 1`, else `ammo = orgAmmo` and a tail
   jump to `vtable[224h]` = `006E3E70`, which builds the next round and takes one from ammo.

`006E41C0` has two callers: the rack tick (`006E57D2`) and `BSP_AirField_HandleMessage` (`006D292D`,
the airfield rearm).

## The multiple rack (`MMultipleBombPlatform`, 26h, vtable `00CF9918`)

It adds forty slots, one per platform node of its class ("Platforms", `006E02C0`, count at class
`+188h`): display rounds at `+514h` (entities the rack builds on the nodes, `006E5AB0`), slot timers at
`+5B4h` (-99 idle, `00CF5BFC`), slot flags at `+654h`, the display count `+67Ch`, a load timer `+680h`
and an ammo count `+684h`. Five of its vtable bodies have no Ghidra function (`006E45E0`, `006E3700`,
`006E5AB0`, `006E5F10`, `006E4A00`); they were read with `bsp.py disasm-raw`.

- **Fill** (`006E5AB0`, slot `+224h`): over the platforms while ammo `+484h` > 0, every empty slot gets
  a display round (ammo -1, `+67Ch` +1); then `+680h = 0.5` (`00CE3800`).
- **Load** (`006E5900`, the tick element's slot `+8h`, before the base tick): in modes 0/1 a load timer
  below 100 counts down; at expiry the first occupied slot gets a B3h load message; with none,
  `+67Ch = 0` and the timer parks at 999. B3h (and B2h) reach `006E5F10`, which calls `006E5D70`
  ("MultiBomb GenerateRealAmmo: %d"): the real round is created at the slot's frame, the display round
  is killed, `+680h = 1.2`. A fire message ADh with display rounds and at most three real rounds sets
  `+680h = -1` (load at the next tick). After the pre-tick, `toRepeatTime = -1`: the multi rack has no
  repeat pacing.
- **Drop** (B1h arm of `006E5F10`): `+684h -= 1` and the trim `006E4690`, then the dropped round's slot
  timer starts at ReloadTime (`vtable[1F4h]` = `006E37F0`).
- **Reload** (`006E44C0`, slot `+204h`, the same slice as above): each running slot timer counts down by
  `[00E08178]` x dt; at expiry the timer goes back to -99 and `+684h += 1`; modes 0/1 route B2h for that
  slot and stop (one slot per tick), mode 2 marks the slot loaded and goes on.

## Reconstructions (reconstructed, build-tested)

All in `src/gameplay_seg33_cc13.cpp`; ABI and offsets per declaration in the header.

| address | function | coverage |
|---|---|---|
| `006E3C00` | `rack_construct_006e3c00` | rack fields only |
| `006E3530` | `rack_set_ammo_006e3530` | complete |
| `006E3A20` | `rack_set_reload_time_006e3a20` | complete (FLD1/FCOMIP/JBE; NaN kept) |
| `006E3500`, `006E3570` | `rack_round_count_006e3500`, `rack_is_empty_006e3570` | over the attached-round count |
| `006E4130` | `rack_ammo_fraction_006e4130` | FILD/FIDIV, one float round |
| `006E4170` | `rack_remaining_drop_time_006e4170` | FIMUL, one float round |
| `006E3550`, `006E3CD0` | `rack_fire_006e3550`, `rack_is_busy_006e3cd0` | complete |
| `006E41C0` | `rack_advance_reload_006e41c0` | complete; the message is a returned event |
| `006E3410` | `rack_rearm_006e3410` | complete; the tail call is returned |
| `006E3E70` | `rack_attach_next_round_006e3e70` | ammo bookkeeping only |
| `006E56F0` | `rack_tick_reload_slice_006e57a4`, `rack_fire_window_open_006e57d4`, `rack_fire_window_step_006e5816` | the countdown, the reload slice, the window bookkeeping |
| `006E42D0` | `multi_rack_construct_006e42d0` | multi fields only |
| `006E3770` | `multi_rack_set_ammo_006e3770` | complete |
| `006E35C0`, `006E35F0` | `multi_rack_any_slot_reloading_006e35c0`, `multi_rack_min_reload_left_006e35f0` | complete |
| `006E3730`, `006E45E0` | `multi_rack_count_rounds_006e3730`, `multi_rack_has_round_006e45e0` | over the inputs |
| `006E3700` | `multi_rack_ammo_fraction_006e3700` | no zero guard, as the image |
| `006E3790` | `multi_rack_kill_display_round_006e3790` | the kill itself is the host's |
| `006E37F0` | `multi_rack_reset_slot_timer_006e37f0` | the slot<0 miss is not modelled |
| `006E3840`, `006E3870` | `multi_rack_display_round_died_006e3840` | per slot |
| `006E4410` | `multi_rack_request_loads_006e4410` | the bound is clamped to 40 |
| `006E44C0` | `multi_rack_advance_reload_006e44c0` | complete; the message is the returned slot |
| `006E4690` | `multi_rack_trim_006e4690` | the image's spin is stopped |
| `006E5AB0` | `multi_rack_fill_display_rounds_006e5ab0` | slot bookkeeping; create is a callback |
| `006E5D70` | `multi_rack_generate_real_round_006e5d70` | bookkeeping; the spawn is the host's |
| `006E5900` | `multi_rack_pre_tick_006e5900` | complete; the base tick is separate |
| `006E5F10` | `multi_rack_on_fire_message_006e5f54`, `multi_rack_on_drop_message_006e5f86` | the ADh and B1h arms |
| `006E01C0`, `006E02C0` | `BombPlatformClassFields` | field offsets and defaults only |
| `006E7CD0`, `006E2890` | `projectile_speed_006e7cd0` | complete (bullet `+178h`, bomb `+318h`) |
| `006E2220` | `projectile_extrapolate_position_006e2220` | the position only |

Every floating compare was read in the listing (COMISS/FCOMIP and its branch); the x87 blocks of
`006E44C0` and `006E7CD0` were also run through `sc2_x87.py`, and `tools/x87_dc_form_scan.py` finds no
DC-form register operation in `006E2200`-`006E2900`, `006E3400`-`006E5F90` or `006E7CD0`-`006E7D50`.

## Adoption

| model | replaces (host, file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
|---|---|---|---|---|
| `rack_tick_reload_slice_006e57a4` + `rack_advance_reload_006e41c0` + `rack_rearm_006e3410` + `rack_attach_next_round_006e3e70` | nothing replaced; the slice is absent between `src/game_hosts_units.cpp:26959` (countdown) and `:26960` (window) of `run_rack_tick_006e56f0` | ammo/orgAmmo: `rack_ammo_per_rack` / `rack_authored_per_rack`; ReloadTime: the same Equipment entry (`read_equipment_ammo`, `src/game_hosts_units.cpp:3742`, reads its Ammo; ReloadTime is not read yet); attached rounds: the host's per-rack round state; flag: `lua_device_reload_enabled_00e17bf2()`; multiplier: `[00E08178]`, not hosted (1.0; `SetDeviceReloadTimeMul` 008C14B0 is only named in `src/mission_lua_host.cpp:315`); session mode 0 | none | missions without the flag (USN01, USN04, USN13) unchanged. JM08 (flag set), as a prediction: a single-rack bomber that has dropped all its rounds gets them back ReloadTime (default 30 s, floor 1 s) after its last drop and can attack again; expect extra drops and kills late in JM08, and an `ammo` that climbs back in any per-rack census. jm03 (mul 0): no change |
| `MultiRackState` and the `multi_rack_*` functions | the substitute count in `count_remaining_rounds_007c1db0` (`src/game_hosts_units.cpp:4060`) and the bare `orx_rack_ammo` (`:579`) | platform count (class `+188h`, the "Platforms" list), Ammo/ReloadTime as above, real-round count, the session-message delivery (B1h/B2h/B3h/ADh) | none | rocket racks would show display rounds converting to real ones with a 0.5 s first and 1.2 s later load delay, and `+684h` would gate rounds rather than the host's count; with the reload flag set (JM08) rocket slots refill one per tick per expiry. Without it, the visible change is the load cadence of real rounds (at most three real rounds re-armed per fire message) |
| `projectile_speed_006e7cd0` | none (host readers of the shot's `vtable[38h]` were not searched) | the record velocity | none | none until a host calls it |
| `projectile_extrapolate_position_006e2220` | none | snapshot `+448h`, velocity `+318h`, t | none | none until a host calls it |

## Pointer records (no new code)

| address | source | coverage |
|---|---|---|
| `006E1510` | `src/bomb_torpedo_tick.cpp:93`, `:107` (`bomb_air_velocity`, `bomb_pitch_blend`) | partial |
| `006E2570` | `src/bomb_torpedo_tick.cpp:80` (`bomb_water_mode`) | the in-water byte `+44h` |
| `006E2800` | `src/bomb_torpedo_tick.cpp:265` (`bomb_sweep_flags`) | both bytes set, `RET 8` |
| `006E1010` | `src/bomb_torpedo_tick.cpp:256` (`bomb_commit_snapshot`) | partial |
| `006E65F0` | `src/projectile_impact.cpp:75` | the damped arm |
| `006E7760` | `src/projectile_impact.cpp:55` (gravity off) | partial |
| `006E8320`, `006E8770`, `006E1BE0` | `src/projectile_kinds.cpp:153`, `:163`, `:230` | per docs/WEAPON_CLASS_DESCRIPTOR.md |
| `006EA1C0`, `006EA260` | `src/projectile_kinds.cpp:24`, `:29` (class table rows) | vtable and sub-type |
| `006E2790`, `006E3E10`, `006E43B0`, `006E7C00` | `src/entity_class_ids.cpp:54`, `:49`, `:50`, `:53` | the id chains (bytes read; `006E7C00` also compares `[ECX+C4h]`) |
| `006E7C60` | `src/game_hosts_gunnery.cpp:9951` | the stream-1 damage draw |
| `006E0A70` | `src/game_hosts_gunnery.cpp:13422` | partial (velocity only) |
| `006E1F00` | `src/game_hosts_gunnery.cpp:13464` | partial, labelled in the host |

## Findings for others

- **`00CE3820` is 1e-10, not 1e-11.** The bytes `BB BD D7 D9 DF 7C DB 3D` decode to exactly `1e-10`
  (Python `struct.unpack('<d', ...)`). `include/bsp/ai_planners.hpp:218`
  (`kAiPlannerNearRadiusSquared = 1.0e-11`) and `include/bsp/ai_planner_tails.hpp:68`
  (`kAiTailLengthEpsilon = 1.0e-11`) carry 1e-11 for this address; other headers
  (`ai_command_tick.hpp:216`, `avoid_zone_geometry.hpp:150`) have 1e-10. Not edited here.
- **Two image hazards in the multi rack.** `006E4690` loops forever if `+67Ch` > 0 while every `+514h`
  slot is null (the miss index -1 reads `+510h`); `006E37F0` with a negative slot and no negative timer
  writes `+653h`/`+5B0h` (index -1). Both are kept consistent in the image by the observer
  (`006E3840`) and the kill; the models stop instead and say so.
- **Unread:** whether `BSP_MissionEntity_Kill(0)` in `006E3790` reaches the observer `006E3840` before
  the slot store (which would decrement `+67Ch` twice). The model assumes it does not.

## Open

- The single rack's attach `006E3E70` and the multi's `006E5D70` create rounds through the class's
  `vtable[20h]`; the host's spawn would stand in, as it does for `006E4D50`.
- The dropped slot index in the B1h arm comes from `entity(msg+20h)+460h`; `006E5D70` writes the slot to
  the created object's `+150h`. Whether those are the same field (the object returned by the create
  being the record at `-310h`) was not read.
- ReloadTime is not stored by the host yet; adopting the reload needs it from the Equipment entry
  (`+0Ch`).
- The rest of the segment's 425 unrecorded functions: session-message plumbing, serializers, the
  catapult (air ops), gun bots, command classes and command buildings (cc9's lanes), STL bodies. None
  was attempted.

## Handoff

Done: 54 records, the two rack models, the speed and extrapolation leaves; build passed in this tree.
Next, if wanted: the single rack's save state `006E6010` / serializer `006E4A60` field list (names for
the host's per-rack state), and the bomb record's `006E16D0` ApplyLaunchState (the fire-message
path). Scripts: this tree's `local/g33_*.py`. Call claims: `reports/cc13_gpr33_calls.json`
(`tools/verify_report_calls.py`: 9 rows, 0 failed).
