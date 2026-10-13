# CC13 VER 5: verification of the ship kill and wreck path

Packet cc13-ver5 (Claude, cc13 lineage), 2026-10-13. This is a check of the host code that decides
when a ship counts as dead, against the disk bytes. It follows docs/CC13_VER_4.md, which covered
damage, flooding and the KillDepth kill. The sources were `bsp.py disasm-raw` (Capstone over the PE
on disk), `sc2_x87.py`, and a hand decode of every x87 register form quoted here. Line numbers are
from origin/main 161e9f45a.

There were no game runs. The run evidence in section 5 comes from existing cc9 logs. Every name is a
hypothesis, not a recovered symbol.

## 1. The chain, image against host

| step | image | host | verdict |
|---|---|---|---|
| health write | `00877B90`: unchanged returns (FUCOMIP/JNP); clamp to [0, max]; `+2E8h = -1` when health > max - 1.0 (`00D7A210`); then `vtable[1B0h]` on **every** changed write unless lobby mode is 2 | `set_health_00877b90` (unit_damage.cpp:263) | match |
| death test | ship `1B0h` = `00827A90`, which calls `00958A30` first; the no-parent arm `00958DA7`: `COMISS 0,[+370h]` / `JB`, then `vtable[70h](1)` | `unit_is_dead` then `kill_unit` (game_hosts_gunnery.cpp:10376, 10647, 11519, 11614, 13331) | match, apart from a NaN health (D5) and writes outside the host's damage paths (D3) |
| destroy | `0077D1A0`: message `4Eh` (session-gated), `00926C80` (`+60h`; a zero cause `+70h` becomes the destroyed parent's cause, or 1), then **`00986480` entityKilled** with no session gate | `kill_unit` with `kDeathRouteDestroyBound` (`+60h`) | **D1: entityKilled is never fired** |
| death message | `00827AB0..00827B77`: health <= 0, then Mass `[desc+B0h]` < 100.0 (`00D7A220`) sends `9Ah`; otherwise the controller tests `0092BEA0`/`0092BE90` and a stream-1 `U(0, 1)` against `WreckChance` (settings `+65Ch`, 0.3 in this installation) send `98h` (breakup), or nothing | not modelled | D4 (no outcome change for ordinary ships) |
| flush, row 15 | `00926390`: `+5Dh = +60h = 1`, `vtable[7Ch]` = `00824B60` OnWrecked | `kill_unit` sets `dead` / `sunk` at once | timing only (one frame, docs/ENTITY_DEAD_FLAG.md section 2) |
| OnWrecked | `00824B60`: leak `0074EC50`, inertia x2, damping 2.5 / 0.5, `sinkTime = 0`, effects, the cause-1 timer and scatter, breakup registration, throttle cut, `+740h` release; **no early exit** (the only branch out, `00824BEB`, rejoins at `00824BF9`); tail `00959450` | `ship_wreck_sink_block_00824fe5` (game_hosts_units.cpp:3246) | match (the inertia note is in section 4) |
| loss and credit | `00959450`: `00878990` first (`0095945B`, which reaches `00929800`: `Dead`, `KillReason`), then, with clock > 1.0, cause 1 and `world+4ACh`, `009813A0` (the `kill` channel) unless the controller flags veto it, then `0091BDA0` | `unit_death_00959450` (mission_result.cpp:235) | gate match; **D2: the host fires `kill` before it publishes `Dead`** |
| wreck sinking | `009329C0` each step; `+5Dh` gates only the planing torque | `ship_hydro_apply_forces_009329c0` | match (section 4) |
| final kill | `00825F20`: `sinkTime` and the terrain mask, then KillDepth `00826628` -> `00926D90(1)` -> kill list -> `00928C80` | game_hosts_units.cpp:34182 (`kSunkShipKillDepthBound`, ON) | match (docs/CC13_VER_4.md); the host wrecks do reach it now (section 5) |

## 2. Divergences

Each one lists the instruction address, the host site, image against host, and the predicted effect.

**D1. entityKilled is never fired.** Fixed behind `kLuaEntityKilledListenersBound` (OFF), 977bbd242.
- Image: `0077D248..0077D253` calls `00986480` on `[00F8A0C4]` for every destroy. Section 3 has the
  detail.
- Host: game_hosts_lua.cpp stores the `entityKilled` subscriptions and never evaluates them. The only
  channels it fires are `kill`, `hit` and `recon`.
- Effect: ten scripts in this installation listen on this channel (all mtime 2024-07-13).
  - Three copies of JM01: the Radio station hint, and the Oklahoma's death. That death removes her
    from secondary objective 3, increments `TorpedoTargetsHit` (completion at 5), posts the
    narrative and starts the capsizing movie.
  - `usn_airtraining`, `ijn_airtraining`, and `ijn/usn_seatraining`, `ijn_seatraining2/3`,
    `usn_seatraining2`: their stages end on the target's or the player's death.
- None of this runs in the host today. It does not change when `Dead` flips.

**D2. The host fires `kill` before the victim's `Dead`.** Fixed behind `kKillListenerDeadFirstBound`
(OFF), b570cbeae.
- Image: `0095945B` (`00878990` -> `0077D270` -> `00929B60` -> `00929800`) writes `Dead` and
  `KillReason` before `00959507` reaches `009813A0`, in the same flush.
- Host:
  - `dispatch_kill_listeners_009813a0` runs from the spawn drain (game_hosts_lua.cpp
    `run_spawn_queue_0094c490`). That is inside the world tick, which runs after the fixed step
    (in_mission_subsystem_tick.cpp:42).
  - `publish_unit_deaths_00929800` runs later, in `run_script_timers` (game_hosts_mission_frame.cpp:3601).
- Effect:
  - A kill callback that reads the victim's `.Dead`, or runs `luaRemoveDeadsFromTable` over a table
    that holds the victim, sees it alive unless the callback sets `pEntity.Dead = true` itself.
  - `local/v5_killcb2.py` counts **55 such callbacks** (285 kill callbacks in 105 files). Examples:
    - USN02 `luaUS02ConvoyDead`: hidden objective 1 never completes on the last convoy ship.
    - ESMP01 `luaJPCVSunk`: `luaMissionFailed` on the last carrier.
    - USN03 `luaUS03ZuihoKilled`, USN10 `luaUSN10_IowaWasDestroyed`, LOMP05 `luaVanguardBBSunk`.
    - The multiplayer target-kill callbacks.
  - Each such callback fires once per death, so on the last member of a set the image takes a branch
    the host never takes. This changes outcomes. It does not change when `Dead` itself flips.
  - With the switch on, `hit` is evaluated before `kill`. The image fires `hit` inside the hit
    (`0077CE60` -> `00988510`), ahead of the flush, so a hit callback on the lethal hit still sees the
    victim alive.

**D3. Health writes outside the host's damage paths never kill.** Adoption note: game_hosts_gunnery.cpp
is leased by cc9-gunnery36.
- Image: any changed write through `00877B90` runs `1B0h` -> `00958A30`. So `SetHP` (`008C1790`,
  unbound in the host) or `OverrideHP` (`008C1930`) to a value at or below 0 destroys the unit.
- Host: `override_hp_008c1930` (game_hosts_gunnery.cpp, after :13334) stores the health and has no
  health-zero test.
- Effect: none found. Of the 184 `SetHP` / `OverrideHP` calls in this installation's scripts, none
  passes a literal value at or below 0.

**D4. The death message and the breakup roll are not modelled.**
- Image (`00827ACA..00827B77`):
  - **Mass < 100.0:** the `9Ah` message goes to `00814560`. It detaches the live parts. For a class
    with `KamikazeDamage` or `KamikazeBlastDamage` above 0, it also calls `Kill(1)` at once, which
    puts the unit on the kill list (`TrulyDead` true, `LastPosition` set, world-list unlink) with no
    wreck descent.
  - **Mass >= 100.0:** when both controller tests pass, one stream-1 `U(0, 1)` (`00827B4D`, ECX = 1)
    against `WreckChance` 0.3 sends `98h` (breakup).
- Host: no `9Ah` / `98h`, and no stream-1 draw at death. Every host death sinks as a wreck and is
  killed at KillDepth.
- Effect:
  - The two kamikaze classes in vehicleclasses.lua (the Kaiten entry at line 2073 and the Kamikaze
    Boat at 22945; the file's mtime is 2026-05-09, so it is locally modified) stay in the world as
    wrecks until KillDepth, where the image removes them at once. Their `Dead` flips at the same time
    in both.
  - The breakup is presentation (`004A5AA0`, `00935D30` were not opened).
  - The stream-1 draw is already a labelled host gap (docs/CC13_RNG_STREAM_CENSUS.md, `00827B4D`).
- Library: `ship_on_health_changed_00827a90` tested the selector as `>= 0.0f`. It is corrected to
  100.0 in d915b5d0c. No host calls that function.

**D5. A NaN health.** `00958DAA` COMISS, followed by JB, does not destroy on a NaN. `bsp::unit_is_dead`
(unit_damage.cpp:312) answers true. Modelled as `unit_health_kills_00958da7`. No reach is known.

**D6. Stale statements about KillDepth.**
- The comment on `kSunkShipKillDepthBound` (game_hosts_units.cpp:6266..6278, leased by cc9-lua52)
  says no host wreck descends.
- docs/CC13_WIN_PATH_NATIVES.md section 2 says the same.
- Both are out of date: see section 5.
- `00928C80` nulls the self table's `Ptr`, and with it `TrulyDead` and `LastPosition`. The host still
  only records that step. Every `TrulyDead` call site in this installation's scripts is a camera or
  fail-entity choice, so this stays presentation-only.

## 3. entityKilled, `00986480`

Read from the bytes and from the existing export.
- `0077D1A0` (unit `vtable[70h]`):
  - Under `[00E0AF20]` and lobby mode 0 or 1 it sends message `4Eh` (`0077D1DE..0077D236`).
  - Then, unconditionally, `00926C80(recurse)` (`0077D243`).
  - Then `00986480(this)` when `[00F8A0C4]` is non-null (`0077D248..0077D253`).
- `00986480`:
  - Takes the manager lock.
  - Returns when `00923810(entity, 1)` (the `+3Ch` scene parent; the function walks the chain
    `n - 1` more levels) is non-null and does not answer `IsKindOf(44h)`, the Landscape
    (`009864D2..009864F1`).
  - Otherwise it boxes the entity id word `+174h` and the string `00E0CF04[entity+70h]`. The table
    holds `none harm soft sell exitzone landed editor`, then a null, read from the PE.
  - It clears `+104h`/`+105h` and evaluates the channel's subscriptions through `0097B8C0`.
    `00968FF0` tests `entity` (+0Ch) and then `killReason` (+1Ch); an empty set matches all.
  - It calls each matched callback through `00887E50`.
- In the image this fires inside the hit, **before** the flush that writes `Dead`. A callback
  therefore sees the victim with `Dead` unset. The host order (entityKilled in the spawn drain,
  `Dead` after the frame) keeps that.
- The binding's substitutions, all labelled in `include/bsp/game_hosts_lua.hpp`:
  - The 44h gate always passes.
  - The death is the host's destroyed-unit row, evaluated on the next drain.
  - A squadron slot waits for its last plane, as the `kill` dispatcher does.
  - The reason is `GameUnitsHost::kill_cause_00926d90`, the same source the `Dead` publisher uses.

## 4. `009329C0`, the buoyancy integral, against the bytes

Everything outside the flooding block that docs/CC13_VER_4.md checked was decoded here:
- **Depth.**
  - `00932DD5..00932E12`: clamped = min(height, span) (FCOMIP/JBE), then 0 if negative (COMISS),
    and depth = span - clamped.
  - The span is `[el+8] - [el+0Ch]`, and the shape span is `[el+4] - [el+0Ch]`.
  - The loop index is never negative, so the `[00CE3854]` defaults are dead.
- **Buoyancy** (`00932E44..00932EE9`): `[el+0] * (d * (k * (d / s) + (1 - k)))`, with
  `k = IsKindOf(8) ? 0 : 0.5`. The FSUBRP at `00932E67` gives `1 - k`. The settings-based coefficient
  at `00932E73..00932EE1` is reached only for a negative index, so it is dead too.
- **Gates.** depth > 0 (COMISS/JBE), and speed squared != 0 (UCOMISS, LAHF/JNP: a NaN proceeds). The
  speed squared is `(x*x + y*y) + z*z`.
- **Exponent ladder** (`00932FB5..00933018`): 1.0, 2.0 and 0.5 by equality. Any other value keeps the
  previous slot. The drag scale is `(Mass / count) * factor * 10.0`, where count is the element
  count (stride 24h) taken as unsigned.
- **Drag record roles.**
  - `+00h`/`+04h` (linear / quadratic) are taken when `dir.y < 0` (`00933161` COMISS 0, `[ESP+7Ch]`,
    then JBE). That makes them the downward pair, as the host names them.
  - `+10h`/`+14h` are lateral and `+18h` forward. The forward term is linear only, multiplied by the
    scale.
  - The lateral and vertical projections are negated (`0093322A` FLDZ / FSUBRP).
- **Impulse clamp** (`0093341B..0093375E`):
  - dv = force * dt / total mass.
  - The rows are taken in the order 2, 0, 1. The limit is `-1.0 / count` (DC /7 FDIVR on the double
    at `00D7A250`).
  - Each axis is clamped when limit > dv / v (FCOMI, JBE).
  - The rebuild is (forward + lateral) + up.
- **Buoyancy on world Y** (`00933780`); `+0` on x and z.
- **Planing torque** (`009337BC..00933951`):
  - It needs `+5Dh` clear (the host's `suppress_planing` = `state->simulate`, game_hosts_units.cpp:9861),
    the last element, material 1, and forward speed > 0. The forward speed is row 2 times the body's
    linear velocity.
  - Magnitude: `((W * L) * depth / span) * speed`. The lift is row 1 times that magnitude, and the arm
    is row 2 times `f(L * 0.5)`.
  - The torque is `cross(lift, arm)`, subtracted.
- **Cross products.** `004F9B30`'s ABI is ECX = out, EDX = a, with b on the stack; its first output is
  `a.y*b.z - a.z*b.y`. With that ABI the element torque is `cross(arm, force)` and the point velocity
  is `lin + cross(omega, arm)`.
- **Tail** (`009339ED..00933B38`):
  - The leak tick runs after the loop.
  - The scratch at `+68h..+7Ch` is zeroed, and the flooding weight is `0 - f(water * 10.0)` from the
    pre-tick water (`[ESP+B0h]`, S-1BC).
  - The heel torque `0074F2E0` is added. Force and torque go out through `00C35360` / `00C35330`.

**Verdict:** match. The host differs only in last-bit rounding, because it does each step in float
where the image rounds at 24 bits with a wider exponent.

The wreck's inertia (`00825001..00825044`, modelled as `wreck_inertia_00825006`):
- `00C37F10` returns `f(1 / inverse)` with no zero test, and the image doubles that.
- `00C37E70` maps a zero inertia to a zero inverse, and 1/I otherwise.
- A zero inverse therefore comes back as zero in both the image (through +inf) and the host (which
  maps it to 0). They match.

## 5. Do host wrecks reach KillDepth? Yes

The `summary sunk ship kill depth` line in recent cc9 logs (`bound=1`, `kill_depth=-200.0`):

| log | wrecks | kills | lowest hull end |
|---|---|---|---|
| cc9-gunnery36 `g40_tf0_u15.log` | 51 | 50 | -353.36 |
| cc9-ships44 `s46_b_a4.log` | 85 | 85 | -428.73 |
| cc9-lua52 `l56_M_j8.log` | 26 | 26 | -376.89 |
| cc9-physics1 `p6e8_Br1.log` | 10 | 10 | -381.59 |
| cc9-ships44 `s46_g_i13idle.log` | 2 | 1 | -202.35 |

So the kill-depth drain runs in current builds. The wrecks that are not killed are the ones still
descending when the run ends. The USN02 figure in the switch comment (lowest end -47.04 m) predates
this.

## 6. Do hits ever carry a non-negative `+34h`? Yes

- `00723E90`, behind the unit-part shape's slot 0 `00724510` (vtable `00CFD768`), copies the element
  it hit: `00723F6C MOV [EAX+34h],EDX` with EDX = `element+8h`, and `+30h` = `element+4h`
  (docs/HIT_HULL_SEGMENT.md). Every element this installation's ship models author has an index of
  0 or more: `body 0`, `fizika 0..10` and so on (docs/GEOM_MESH_RESOURCE.md section 5).
- So a shell that hits a ship's mesh takes the hull pass at `008777DD`, and the host's ungated hull
  pass matches the image there.
- `-1` comes only from `00470470` (no hit) and the three hard-coded shapes `0087FEC0`, `0087FF80` and
  `00929B80`, the only immediate stores of kind `0Ah` in the image (byte scan
  `C7 ?? 30 0A 00 00 00`).
- What remains open is which hits the image resolves through those three shapes, because there the
  host applies hull damage that the image skips. The owner of the subobject shape at `entity+344h` was
  not traced.

## 7. Fixes, models and records

| sha | what | switch |
|---|---|---|
| 977bbd242 | D1, the entityKilled channel | `kLuaEntityKilledListenersBound = false` |
| d915b5d0c | D4, the library selector threshold 100.0 | none (library only) |
| b570cbeae | D2, `Dead` before the `kill` callbacks, and `hit` before `kill` | `kKillListenerDeadFirstBound = false` |

Models, in `src/ship_kill_ver_cc13.cpp` and `include/bsp/ship_kill_ver_cc13.hpp` (reconstructed and
build-tested, not ABI-compatible):
- `unit_health_kills_00958da7`
- `entity_event_parent_gate_009864d2`
- `kill_reason_name_00e0cf04`
- `wreck_inertia_00825006`

Ledger records:
- `009329C0`: the byte check appended with `--replace`.
- `00986480`: function record for the host binding.
- Fragments: `00958DA7`, `009864D2`, `00825001`.

Adoption notes for cc9, in leased files:
- D3 (game_hosts_gunnery.cpp).
- D6's comment (game_hosts_units.cpp).
- D4's kamikaze-class Kill at the death message, if the two kamikaze classes ever matter in a mission.

## 8. Open

- Which units carry a `+3Ch` scene parent other than the Landscape. That parent would suppress
  `entityKilled` for them; the binding assumes there is none.
- The order of `entityKilled` against `hit` inside one hit. Both run inside `009239A0`'s dispatch;
  their relative order was not read.
- The image's `9Ah` / `98h` receivers past their tail calls (`00935D30`, `004A5AA0`).
- The owner of the subobject shape at `entity+344h` (section 6).
- The `+71Ch` subobject arm of `00958A30` (`00958A58..00958DA6`).
