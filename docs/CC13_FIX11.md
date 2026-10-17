# CC13 FIX11: the Lua native SetDeadMeat (008AC7B0)

Packet cc13-fix11, granted by the cc9 gameplay lead on 2026-10-17. Switch:
`bsp::game::kLuaSetDeadMeatBound` in `include/bsp/game_hosts_lua.hpp`, committed **OFF**.

## Status

Reconstructed and build-tested in the Lua host (`GameMissionLuaHost::run_set_dead_meat_008ac7b0`,
`src/game_hosts_lua.cpp`). Not game-validated: no run was made. With the switch OFF the
dispatch row is not handled, so the native records `UNIMPLEMENTED` exactly as before and no
draw, kill or log line is added (the summary line prints only when the switch is ON).

## The image

`008AC7B0-008AC9C2`, a binding with no stack arguments (`RET`). Argument 0 is resolved by
`00888AA0` at `008AC8AF` with no null test, then:

| arm | test | sites | effect |
| --- | --- | --- | --- |
| plane | `vtable[5Ch](0Fh)` at `008AC8CF` | `008AC8D5..008AC8FB` | `vtable[194h]("spinning" 00D059EC, 0)`, the plane's `007BBFA0` |
| ship | `vtable[5Ch](6)` at `008AC92E` | `008AC934..008AC96A` | `00BD2F10(ECX = 1, -1.0f, 1.0f)` into a pivot `{draw, 0, 0}`, then `008110F0(&pivot)` |
| else | - | `008AC971..008AC97A` | `vtable[70h](1)` |

Then `008AC980` `00B66400` returns the result count; nothing is pushed, so Lua gets no value.

The draw: `FLD1` is stored at `[ESP+4]` and `[00D7A260]` (bytes `00 00 80 BF`, `-1.0f`) at `[ESP]`,
so 00BD2F10's `(lo, hi)` is `(-1.0f, 1.0f)`, on stream `ECX = 1`, the shared stream. The `FSTP`
at `008AC950` stores it as the pivot's x and `008AC95E`/`008AC964` zero y and z. `008110F0`
(`__thiscall(this, const float pivot[3])`, `RET 4`, body `008110F0-00811144`) never reads its
argument (docs/UNIT_DAMAGE_AND_DEATH.md "Sink"), so the draw's only effect is to advance the
shared stream by one value. `008110F0` returns at `008110F7` when `+5Dh` is set and at `00811108`
when `[+150h] > 0.0f` (`COMISS`/`JA`: a NaN does not refuse); otherwise `00811111`
`vtable[70h](1)` (`0077D1A0` for a unit), then it zeroes `+828h`/`+82Ch` and releases `+740h`.
The draw is taken before the gate, so a refused sink still consumes it.

## The host body

`run_set_dead_meat_008ac7b0` keeps the image's order. The standalone body
`bsp::lua_set_dead_meat` (`src/options_seg56_cc13.cpp`) is faithful but runs on `Seg56Host`,
an interface of 86 pure methods this host does not implement, so the three arms are carried
in the Lua host directly, line for line with it. Substitutions, all labelled in the comment:

- class tests: the units host's `unit_is_kind_of(index, 0x0F)` and `(index, 6)`. A squadron's
  slot is fused with its wing-0 plane, so a squadron answers as that plane instead of taking
  the image's else arm.
- an entity with no units-host slot is counted (`unresolved`) and nothing runs.
- **plane arm: counted, not applied.** The host runs 007BBFA0's "spinning" only inside the
  units host's death-mode block (`007CA8A0 -> 007BBFA0 -> 007D0B80`, `src/game_hosts_units.cpp`),
  for a plane the gunnery host has already killed. Nothing public applies it to a live plane,
  and `GameMissionLuaHost::run_set_failure_0088eff0` has no plane arm either. So the plane
  arm's own stream-1 `U(0, 1)` draw at `007BC1F2` is also not taken. Building this needs a
  units-host entry, which is outside this packet.
- RNG: `GameGunneryHost::death_mode_draw_00bd2f10(1, index, -1.0f, 1.0f)`. By default this is the
  shared stream-1 generator in call order, as in the image. Under the measurement option
  `BSP_GUNNERY_RNG_STREAMS=1` it is keyed `(death_mode, unit)`; no other ship draw uses that key.
  No new draw path was added.
- `+5Dh` is `GameGunneryHost::unit_dead()` (dead, or health <= 0). `+150h` is
  `unit_invincibility()`, read whatever the invincibility floor switch says, because the image
  always gates. The gate is `bsp::sink_is_refused_008110f0` (`src/unit_damage.cpp`).
- `vtable[70h](1)`, both for the sink and for the else arm, is
  `GameGunneryHost::destroy_unit_0077d1a0(index, 1)`, the host's death funnel. The gunnery
  summary's "airfield destroy kills" counter also counts these calls, so subtract
  `sunk + destroyed` from it when reading an ON run. The `0077D1E4` destroy message,
  `+828h`/`+82Ch` and the `+740h` release are not held by the host. `src/game_hosts_gunnery.cpp`
  was not edited: cc9-gunnery36 holds its lease until 23:17, and the existing public API was
  enough.

Counters (`summary mission script dead meat ...`, ON only): `calls`, `sunk`, `sink_refused`,
`destroyed`, `plane_unapplied`, `unresolved`.

## Where it is live

The installation's `scripts/missions/usn/usn_19_coralus.lua` (mtime 2024-08-26) is USN04 Coral Sea:

- `luaRetreatShohoFleet` (`:1678`, called from `luaZuikakuMovieEnd` `:1943`): for every live
  member of `Mission.IJNShohoFleet`, `SetInvincible(unit, 0.0)`, `PutTo` 40000 m in +z,
  then `SetDeadMeat(unit)` (`:1689`).
- `luaLexDeadMovie` (`:1223`): the same sequence over `Mission.IJNShohoFleet`,
  `IJNTransportFleet`, `TownFleet` and `LexFleet` (`:1243`, `:1257`, `:1271`, `:1285`), then
  `luaKillAllPlanes()`. This is the losing branch.

All these targets are ships, and `SetInvincible(unit, 0.0)` runs first, so the `+150h` gate
passes. The plane arm is not reached in this mission.

## Pair prediction (USN04, OFF vs ON)

- OFF (today): `MissionLuaNative::SetDeadMeat 008ac7b0 UNIMPLEMENTED` with a call count. The
  Shoho fleet is moved 40000 m away and stays alive.
- ON: after phase 3's `luaZuikakuMovieEnd`, each surviving Shoho-fleet ship takes one shared
  stream-1 draw and dies through the death funnel. Expect `sunk` = the number of Shoho-fleet
  ships alive at that point, `sink_refused = 0`, `plane_unapplied = 0`, and the SetDeadMeat
  row moving from UNIMPLEMENTED to implemented. Expect the death table to gain those ships, and
  every later stream-1 draw to shift by one value per call (docs/RANDOM_STREAMS.md: the stream
  is shared). So gunnery and plane draws after that point are RNG-coupled and cannot be
  attributed per event.
- Host artefact to check first: the funnel (`GameGunneryHost::Impl::kill_unit`) credits the
  ship's last attacker (`killed_by`, `kill_credits`), as it does for every funnel death. Whether
  the image's Destroy route (`0077D1A0 -> 00926C80`) produces the same credit was not traced
  in this packet. So if the player last hit one of these ships, a new kill credit or score
  line in the ON log is unverified.
