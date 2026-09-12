# The unit group: its member records, their three producers, and the follow station

Addresses: 0070DAB0, 0070D7B0, 0070DB20, 0070EF30, 0070ED30, 0070EFD0, 0070E620, 0070ECF0, 0070E4C0, 0070ECA0, 0070DA00, 0070D030, 0070D060, 0070D070, 0070D080, 0070D0C0, 0070D0F0, 0070D100, 0070D140, 0070D1B0, 0070E450, 0070D290, 00811150, 00811180, 00810630, 009DACD0, 009DF2D0, 0077F940, 0077FAD0, 0088FFD0, 00E08FE0, 00E090A8, 00E09170

Packet `cc_ai_formation`. Read-only against `C:/Users/sqz269/bsp.gpr`,
`/battlestationspacific.exe`: no renames, comments, prototypes or saves through the bridge.
Reconstruction in `include/bsp/ship_ai_formation.hpp` and `src/ship_ai_formation.cpp`; call-site
evidence in `reports/ship_ai_formation.json`. Every descriptive name here is a hypothesis, not a
recovered symbol.

`docs/SHIP_AI_FOLLOW_LAND.md` and `docs/SHIP_AI_PATH_CORRIDOR.md` both reached the member record
from a consumer and had to leave its two float columns provisional. This packet read the three
routines that **write** them, so the meanings are settled and two statements in those documents
are corrected below.

## The object

`operator_new(0x508)` at `0070DB29` and `0070ECF9`, vtable `00CFD6F8`
(`[0] = 0070D260` scalar deleting destructor, `[4] = 0070ECA0`). Reached from an entity at
`entity+284h`; `0070EF30` and `0070E620` are the only two routines that write that pointer and
`0070E4C0` is the only one that clears it (`0070E54x`, `entity+284h = 0`).

| offset | field | producer |
| --- | --- | --- |
| `+0h` | vtable | `0070DAB0` |
| `+4h`..`+10h` | four dwords the constructor zeroes; not read by anything this packet read | `0070DAB0` |
| `+14h` | the leader entity | `0070D7B3` (create), `0070E65B` (Lua `leader`), `0070E579` (re-election), `0070D0C0` (setter) |
| `+18h` | 24 member records, `34h` bytes each | below |
| `+4F8h` | member count | `0070D7B0`, `0070EF30`, `0070E4C0`, `0070E620` |
| `+4FCh` | formation "type" | `0070D842` writes 6 for a ship-led runtime group; Lua field `type` at `0070E679`; `0077FB3F` from a session message byte |
| `+500h` | the **column index** every reader uses | Lua field `shape` at `0070E69A`; zeroed by the constructor at `0070DB14`. Nothing else in the image writes it |
| `+504h` | the group speed ceiling | Lua field `maxSpeed` at `0070E6C5`, but `0070DA00` recomputes it on every join and leave |

`(4F8h - 18h) / 34h = 24`, and the constructor's record loop runs exactly 24 times
(`0070DAF3` seeds `17h`, the loop body runs while the post-decrement value is `>= 0`). That is the
same 24 that `shipglobals.lua` gives as `FormationMaxCount`.

### The member record, `34h` bytes

| offset | field | evidence |
| --- | --- | --- |
| `+0h` | the member entity | `0070D030`, `0070D060`, `0070D080` compare it; `0070E771` and `0070EF6x` write it |
| `+4h`..`+0Fh` | `relativePosition`, the member's position in the leader's local frame, clamped to `FollowerMaxDist` | written by `0070EDB9`/`0070EDC3`/`0070EDCE`; Lua field `relativePosition` (`00CFD724`, type code 5) at `0070E78D`; session message `0077FB9A` |
| `+10h` + `4*k` | `dist[k].x`, the **across-track** offset, signed | `0070EEB6` stores `00811180`'s second out-parameter; Lua column `x` (`00CEB488`) at `0070E803`; `0070EEE1`/`0070EEF9`/`0070EF11` store the three table `x` values |
| `+20h` + `4*k` | `dist[k].z`, the **along-track** distance back along the leader's wake | `0070EEB1` stores `00811180`'s third out-parameter; Lua column `z` (`00CFD718`) at `0070E82C`; `0070D290` hands it straight to `00810630` as the distance to walk back |
| `+30h` | the speed this member publishes to the group | written by `0070D100` (from `009DF6AA`), reduced by `0070D140` (from `009F4E1F`), seeded to 999.0f (`00CF4888`) at `0070DAF9`, `0070D7EC`, `0070EF7A` and `0070DA1D` |

**Exactly four columns exist.** The Lua `dist` loop runs four times (`0070E84B CMP EDI,4`,
`0070E84E JL 0070E7D0`) and `0070ED30` writes exactly four `(x, z)` pairs. `0070D290` selects one
with the index at `group+500h` and never bounds it.

## The three producers of the columns

### `0070ED30`, on join, per member

`__thiscall(group)(int index)`, `RET 4`, body `0070ED30-0070EF27`, read complete. One call site,
`0070EF85` in `0070EF30`, with the index of the record just appended and **before** the count is
bumped.

```
if (record.entity == 0) return                                  ; 0070ED4A
leader = group+14h
refresh leader pose, build leader+110h = inverse(leader+0CCh)    ; 0070ED60, 0070ED7C, latch +10Ch
refresh member pose                                              ; 0070ED90, latch +0C8h
record+4h = leader_inverse * member_world_position(member+0FCh)   ; 0070EDAB, stores 0070EDB9..0070EDCE
if |record+4h|^2 > FollowerMaxDist^2:                            ; 0070EDD8 reads settings+424h
    normalise (0042B260) and scale to FollowerMaxDist            ; 0070EE20..0070EE3A
world = leader_world * record+4h                                 ; 0070EE6B
(across, along) = 00811180(leader, world)                        ; 0070EEA6
record+10h = across ; record+20h = along                         ; 0070EEBC, 0070EEB1
d = settings+42Ch (FormationShipDist)                            ; 0070EEC6
record+14h/+24h = LINE[index]    * d                             ; 0070EED0, 0070EEE4
record+18h/+28h = COLUMN[index]  * d                             ; 0070EEF0, 0070EEFC
record+1Ch/+2Ch = DIAMOND[index] * d                             ; 0070EF08, 0070EF14
```

Column 0 is therefore "hold the relative position you had when you joined", expressed in the
leader's wake frame; columns 1, 2 and 3 are the three canned patterns at the member's **join
index**. `0070ED30` scales all three tables by `FormationShipDist` alone.

### `0070EFD0`, the reshape, whole group

`__thiscall(group)(int shape)`, `RET 4`, body `0070EFD0-0070F08C`, read complete. It rewrites
**column 0 only**: the leader's own record gets `(0, 0)` (`0070F026`, `0070F02C`) and every other
record with a live entity gets the next entry of the shape's table, from a counter that starts at
1 (`0070F00B`) and skips the leader. The table address is
`00E08F18 + shape*0C8h + slot*8` (`0070F04x`), so `0C8h` is the table stride, which makes each
table 25 entries. The along column carries an extra `1.5` (`00CE3D78`, `0070F057`) that
`0070ED30` does not apply.

Two call sites: `0077FB53` in the session-message handler `0077FAD0`, with a byte from the
message; and `00890116` in `0088FFD0`, the Lua binding whose failure string is
`luaMW_SetFormationShape`, with the binding's second argument. Shape 0 would index `00E08F18`,
which holds pointers, not floats; no call site was found that passes 0, and the projection
refuses it rather than reproducing the out-of-range read.

### `0070E620`, the scene, whole group

`__thiscall(group)(reader*)`, `RET 4`, body `0070E620-0070E88C`, read complete. A visitor over the
reader's vtable (`+4` open, `+8` close, `+10h` field, `+14h` next element); each field is two
8-byte structs pushed by value, `{0, name}` then `{type, destination}`, and the call cleans both
(`RET 10h`). Fields in order:

| Lua key | string | type code | destination |
| --- | --- | --- | --- |
| `leader` | `00CFD760` | 4 | `group+14h` (via a temporary, `0070E657`) |
| `type` | `00CE4AD0` | 1 | `group+4FCh` |
| `shape` | `00CFD758` | 1 | `group+500h` |
| `maxSpeed` | `00CFD74C` | 2 | `group+504h` |
| `unitList` | `00CFD740` | array | one element per member record |
| &nbsp;&nbsp;`unitID` | `00CFD738` | 4 | `record+0h`, and **`entity+284h = group`** at `0070E777` |
| &nbsp;&nbsp;`relativePosition` | `00CFD724` | 5 | `record+4h` |
| &nbsp;&nbsp;`dist` | `00CFD71C` | array of 4 | |
| &nbsp;&nbsp;&nbsp;&nbsp;`x` | `00CEB488` | 2 | `record+10h + 4*k` |
| &nbsp;&nbsp;&nbsp;&nbsp;`z` | `00CFD718` | 2 | `record+20h + 4*k` |

`0070ECF0` is `operator_new(0x508)` + `0070DAB0` + `0070E620`, and its one call site is
`0077FD04` inside `0077FAD0`, under the Lua key `formation` (`00D03DFC`, tested by
`BSP_LuaReader_HasKey` at `0077FCDC`); the result is stored into `entity+284h`. So a scene that
authors a `formation` table on an entity gets a whole group object with its own `shape` and its
own four `dist` columns per member, and that is the only path that can put a non-zero value into
`group+500h`.

## The three patterns

Three tables of 25 `(x, z)` float pairs. The HUD menu `00536A90` carries four strings,
`ingame.formation_line`, `ingame.formation_column`, `ingame.formation_diamond` and
`ingame.formation_disband`; the first three match the table contents in order.

| shape | table | contents |
| --- | --- | --- |
| 1 | `00E08FE0` | `(0,0), (1,0), (-1,0), (2,0), (-2,0) ... (12,0), (-12,0)` - line abreast |
| 2 | `00E090A8` | `(0,0), (0,1), (0,-1), (0,2), (0,-2) ... (0,12), (0,-12)` - line ahead and astern |
| 3 | `00E09170` | `(0,0), (0,-1), (-1,-0.5), (1,-0.5), (0,1), (0,-2), (-1,0.5), (1,0.5), (-1,-1.5), (1,-1.5), (-2,-0.5), (2,-0.5), (-1,-0.5), (0,-3) ... (0,-14)` - a diamond around the leader that degenerates into a line ahead past entry 12 |

Entry 12 of the diamond repeats entry 2; that is in the image. Only 24 members can exist, so entry
24 of each table is unreachable.

The units are `FormationShipDist` (`settings+42Ch`), which `shipglobals.lua` sets to 250.0. The
other two settings this chain reads are `FollowerMaxDist` (`settings+424h`, 4000.0, the clamp on
`relativePosition` at `0070EDD8`) and `FormationMaxCount` (`settings+420h`, 24, the join cap at
`0077FA6x`). The three keys are loaded by `BSP_GameSettings_LoadFromLuaGlobals` `0083B5E0` at
`0083F02E`, `0083F070` and `0083F0F4`, with the key strings at `00D0AB0C`, `00D0AAF8` and
`00D0AAD4`.

## The wake trail the station is measured along

`00811180` (`this` = the leader entity) and `00810630` (`this` = `leader+0BD0h`;
`00811150` adds `0BD0h` at `00811164` and tail calls it) read the same ring buffer: 40 samples of
`18h` bytes at `entity+0BD8h`, head index at `entity+0F98h`.

| sample offset | field |
| --- | --- |
| `+0h`..`+8h` | world position |
| `+0Ch` | world heading, radians |
| `+10h` | arc length to the next sample |
| `+14h` | the yaw rate recorded at that sample |

`00811180(point)` walks the whole ring for the nearest sample, then answers the signed
perpendicular distance from the trail (`*param_3`, sign from the cross product) and the
accumulated arc length back to it (`*param_4`). `00810630(along)` is the inverse: it walks `along`
metres back from the head and answers the interpolated position (`param_3`), the direction
`(cos, 0, sin)` of `wrap_2pi(pi/2 - heading)` (`param_4`) and the interpolated `+14h` yaw rate
(`param_5`).

**`00810630` never writes `param_5` on the `along <= 0` branch** (`00810645-00810718`; the branch
even reuses `[ESP+0x4C]`, the incoming `param_5` slot, as an angle scratch at `00810658`). So
`0070D290`'s `out[4]` is left at whatever the caller's stack held whenever a member's station is
abreast of or ahead of the leader, and `009DF2D0` reads it at `009DF3F9` as a yaw rate. The
projection carries a validity flag instead and substitutes 0, which makes `009DACD0` answer 1.

## `0070D290`, the station point

`__thiscall(group)(entity unit, float out[7], float across_scale, float along_scale)`, `RET 10h`,
body `0070D290-0070D3FE`, read complete.

```
record = 0070D080(group, unit)                                   ; 0070D2A1
if (unit != group+14h && record != 0):                           ; 0070D2A6, 0070D2AF
    k = group+500h                                               ; 0070D2B7
    out[5] = record[10h + 4k] * across_scale                     ; 0070D2BD, stored 0070D2C9
    out[6] = record[20h + 4k] * along_scale                      ; 0070D2CC, stored 0070D2EA
    wake = 00811150(group+14h, out[6], &pos, &dir, &out[4])      ; 0070D2EE sets ECX, 0070D2F4
    out[0] = pos.x - out[5] * dir.z                              ; 0070D342
    out[1] = pos.z + out[5] * dir.x                              ; 0070D348
    out[2] = dir.x ; out[3] = dir.z                              ; 0070D34B, 0070D356
else:                                                            ; 0070D362
    out[0] = unit+0FCh ; out[1] = unit+104h                      ; 0070D387, 0070D38B
    a = wrap_2pi(pi/2 - unit->vtable[50h]())                     ; 0070D399, 0070D3A5
    out[2] = cos(a) ; out[3] = sin(a)                            ; 0070D3C5, 0070D3D9
    out[4] = out[5] = out[6] = 0                                 ; 0070D3E1, 0070D3E9, 0070D3F1
```

The stack slots behind the FPU sequence at `0070D2F9-0070D356` were resolved by tracking `ESP`
from the four pushes at `0070D2D7`-`0070D2ED` through `00811150`'s `RET 10h`:
`[ESP+0x20]`/`[ESP+0x28]` are `pos.x`/`pos.z` and `[ESP+0x14]`/`[ESP+0x1C]` are `dir.x`/`dir.z`.

## `009DF2D0`, whole

`__fastcall(state)`, `RET 0`, body `009DF2D0-009DF6B4`, read complete. One caller, `009E1689` in
`009E1610`. `docs/SHIP_AI_FOLLOW_LAND.md` covers `009DF2D0-009DF5DF`; this packet adds the speed
match and corrects two statements about the first half.

The speed match, `009DF5E4-009DF6B4`:

```
speed_now = 0092D730([leader+1018h])                             ; 009DF5EA, the LEADER's controller
yaw_now   = 00811940(leader, speed_now, out[5])                  ; 009DF607, RET 0
ratio_now = 009DACD0(yaw_now, speed_now, out[5])                 ; 009DF612, RET 0Ch
blend     = 00419010(0, ratio_now, 400.0f, ratio_at_wake, out[6]); 009DF645, RET 14h
state+28h = blend * state+28h                                    ; 009DF658, 009DF65B
d         = max(blend, 0.25f)                                    ; 009DF664..009DF684, 00CE3868
published = 0080FC30(unit) * 1.25 / d                            ; 009DF68A, 009DF68F, 009DF69A
0070D100(group, unit, published)                                 ; 009DF6AA
```

`ratio_at_wake` is the earlier `009DACD0` call at `009DF400`, whose first argument is `out[4]`
(`009DF3F9 FLD [ESP+0x4C]`, `ESP+0x4C` is `out[4]` in this frame) - the yaw rate recorded at the
wake point this ship is sitting on. So the blend is: alongside the leader, match the leader's
**current** turn; 400 m or more astern, match the turn the leader was making **where you are**.
`00811940` is `RET 0`, which is why its two stack arguments become arguments two and three of the
`009DACD0` call four instructions later; that was checked by tracking `ESP` from `009DF5F7`
through `009DF60C`.

`009DACD0` (`__cdecl(yaw_rate, speed, across)`, `RET 0Ch`, body `009DACD0-009DAD9F`, read
complete) is the station-keeping speed ratio: 1.0 unless `|yaw_rate| > 1e-4` (`00D7A268`, the
double widening of the float `1e-4f`) and `|speed| > 1.0` (`00D7A24C`), in which case
`R = |speed / yaw_rate|` and the answer is `(R - across) / R` for a positive yaw rate and
`(R + across) / R` for a negative one. A follower on the inside of the turn is told to slow down
and one on the outside to speed up. Which side of the ship a positive `across` is was not settled:
it follows from the heading basis at `006BC0C0` and no run-time evidence was taken.

The published speed goes into this unit's own `record+30h`, and `0070D140` (called at `009F4E1F`
inside `009F4DA0 BSP_ShipAi_ThrottleCeilingStep`) reduces those over the group to a minimum, which
the same step compares with `group+504h` through the getter `0070D0F0` (`009F4E34`). So a ship
that has to slow down for its station drags the whole formation's ceiling down with it.

## How a unit joins, leads and leaves

* **Create.** `0070DB20` = `operator_new(0x508)` + `0070DAB0` + `0070D7B0(leader)`. `0070D7B0`
  writes `group+14h = leader`, `leader+284h = group`, makes the leader member 0 with an all-zero
  relative position and all eight columns zero, sets the count to 1, and for a ship leader
  (`vtable[5Ch](6)` at `0070D82F`) sets `type = 6` and `maxSpeed` from the leader's class.
  **The constructor leaves `shape` at 0**, so for every runtime group the live column is column 0,
  which is exactly the column the reshape writes.
* **Join.** `0070EF30(group, entity)`: `entity+284h = group` first (`0070EF38`, before any test);
  if the entity is already a member only the observer pair is added (`0070EFAA`, `0070EFB7`);
  otherwise the record is appended, `record+30h = 999`, `0070ED30` fills the four columns with the
  **join index** (`0070EF85`), the count is bumped, the observer pair is registered
  (`0070EF95`) and `0070DA00` re-reduces the ceiling (`0070EF9C`).
* **The order that causes a join** is `0077F940(this = the ordered unit, other = the unit to
  follow)`, body `0077F940-0077FACA`. It refuses when the two are already in one group, caps the
  merged size at `FormationMaxCount`, creates a group around `other` when it has none, otherwise
  redirects to `other`'s leader, detaches the ordered unit's own followers when it was leading,
  and calls `0070EF30` for each unit it brought. Its one call site is `0077FB74` in `0077FAD0`,
  the session message handler, which resolves the target from a 16-bit id at `0077FB5D`.
* **Leave.** `0070E4C0` clears `entity+284h` (`0070E54x`), re-elects a leader when the leaving
  unit was one (`0070E4DD` through `0070E579`), compacts the record array by copying the 13 dwords
  of each following record down (`0070E591`-`0070E58x`), destroys the group through
  `vtable[0]` when the count reaches zero and otherwise calls `0070DA00`. `0070ECA0` is
  `vtable[4]`: find a member by entity and hand it to `0070E4C0`.
* **The ceiling.** `0070DA00` sets `group+504h` to the minimum over live members of that member's
  own maximum speed: `[[unit+538h]+500h]` for a ship (`vtable[5Ch](6)`) and
  `[[[unit+3D0h]+538h]+188h]` otherwise. `0070D1B0` is the same expression as a standalone
  function with no callers, so `0070DA00` has it inlined.

The Lua bindings `luaMW_IsInFormation` (`008996A0`), `luaMW_IsFormationLeader` (`00899810`),
`luaMW_IsFormationFollower` (`00899980`), `luaMW_GetFormationShape` (`008901C0`) and
`luaMW_SetFormationShape` (`0088FFD0`) are the script-visible surface; only the last was read, at
its `0070EFD0` call site.

## What the executable must implement, in call order

`include/bsp/ship_ai_formation.hpp` declares one pure virtual per native call site. In the order a
formation is built and then stepped:

1. `ShipAiFormationJoinHost` - `0070EF30`: `set_entity_group_284`,
   `observer_pair_registered` / `observer_register_pair` (`00694AF0` / `00694A60`),
   `produce_member_slots_0070ed30`, `refresh_speed_ceiling_0070da00`.
2. `ShipAiFormationSlotHost` - `0070ED30`: `refresh_world_pose_00414db0`,
   `build_world_inverse_00b63d50`, `world_position_0fc`,
   `transform_by_leader_inverse_004142e0`, `game_settings_follower_max_dist_00424c40`,
   `normalize_in_place_0042b260`, `transform_by_leader_world_004142e0`,
   `wake_decompose_00811180`, `game_settings_formation_ship_dist_00424c40`.
3. `ShipAiFormationReshapeHost` - `0070EFD0`: `game_settings_formation_ship_dist_00424c40`.
4. `ShipAiFormationStationHost` - `0070D290`: `wake_point_00811150`,
   `refresh_world_pose_00414db0`, `world_position_xz_0fc`, `unit_heading_vtable_50`.
5. `ShipAiFollowFormationPointHost` - `009DF2D0`: `station_point_0070d290`,
   `unit_hull_radius_9c8`, `leader_body_axis_speed_0092d730`, `zone_set_vtable_218`,
   `push_out_of_zones_00417b10`, `ship_class_turn_radius_0082e850`,
   `leader_command_yaw_rate_00811940`, `unit_reference_speed_0080fc30`,
   `publish_member_speed_0070d100`.

For the game executable's 13 `stop` ships: creating the group they would expect needs
`0070DB20`-equivalent state (one object, leader set, leader as member 0, count 1, `shape` 0,
`type` 6), then one `ship_ai_formation_add_member` per follower, then
`ship_ai_formation_reshape` with the chosen shape. Nothing in that path needs Lua; the Lua path
(`0070ECF0`) only matters for a scene that authors a `formation` table.

## Coverage

| routine | body | coverage |
| --- | --- | --- |
| `0070DAB0` | `0070DAB0-0070DB1B` | complete |
| `0070D7B0` | `0070D7B0-0070D868` | complete |
| `0070DB20` | `0070DB20-0070DB59` | complete |
| `0070EF30` | `0070EF30-0070EFC0` | complete |
| `0070ED30` | `0070ED30-0070EF27` | complete |
| `0070EFD0` | `0070EFD0-0070F08C` | complete |
| `0070E620` | `0070E620-0070E88C` | complete |
| `0070ECF0` | `0070ECF0-0070ED29` | complete |
| `0070ECA0` | `0070ECA0-0070ECE6` | complete |
| `0070DA00` | `0070DA00-0070DA9D` | complete |
| `0070D030` | `0070D030-0070D058` | complete |
| `0070D070` | `0070D070-0070D07D` | complete |
| `0070D080` | `0070D080-0070D0B5` | complete |
| `0070D0C0` | `0070D0C0-0070D0E3` | complete |
| `0070D0F0` | `0070D0F0-0070D0F6` | complete |
| `0070D100` | `0070D100-0070D132` | complete |
| `0070D140` | `0070D140-0070D19F` | complete |
| `0070D1B0` | `0070D1B0-0070D1E1` | complete |
| `0070D290` | `0070D290-0070D3FE` | complete (both branches) |
| `009DACD0` | `009DACD0-009DAD9F` | complete |
| `009DF2D0` | `009DF2D0-009DF6B4` | complete |
| `00811150` | `00811150-00811174` | complete |
| `00810630` | `00810630-008108F4` | partial: the `along <= 0` branch `00810645-00810718` read complete; the trail walk `0081071B-008108F2` read for its out-parameters and the `+10h`/`+14h` sample fields, not for its interpolation arithmetic |
| `00811180` | `00811180-00811812` | partial: the unrolled 40-sample nearest search `008111xx-008117xx` read only far enough to establish that it is a nearest-sample search; the tail that forms the two out-parameters read complete |
| `0070E450` | `0070E450-0070E4B2` | partial: the `+18h`/`+4F8h` walk and the `vtable[5Ch](6)` / `vtable[214h]` maximum only |
| `0070E4C0` | `0070E4C0-0070E619` | partial: the leader re-election, the `entity+284h` clear, the compaction and the two tail calls; the observer bookkeeping between them was not read |
| `0077F940` | `0077F940-0077FACA` | partial: the size cap, the group creation and the join loop; the `vtable[114h]`/`vtable[58h]` follow-up on each joined unit was not read |
| `0077FAD0` | `0077FAD0-0077FE7E` | partial: the `0070EFD0`, `0070EF30`-via-`0077F940`, `0070D080` and `0070ECF0` arms only; the rest of the message handler was not read |
| `0088FFD0` | `0088FFD0-008901BC` | partial: only the `0070EFD0` call site `00890116` and its two arguments |
| `0083B5E0` | `0083B5E0-00842951` | partial: only the three `settings+420h`/`+424h`/`+42Ch` stores and their key strings |
| `00536A90` | `00536A90-00537AED` | not read; cited only for the four `ingame.formation_*` strings it references |

## Corrections

| was | is | evidence |
| --- | --- | --- |
| `docs/SHIP_AI_FOLLOW_LAND.md`: "a slot is (along, across) from a per-member table" and "`out[0..1] = base + along * (-dir.z, dir.x)`" | the order is (across, along): `record+10h + 4k` is the across-track offset and is what multiplies the perpendicular; `record+20h + 4k` is the along-track distance and is what `00810630` walks back | `0070D2BD` reads `+10h + 4k` into `out[5]`, and `out[5]` is the factor on `-dir.z` / `dir.x` at `0070D316`/`0070D31E`. `0070D2CC` reads `+20h + 4k` into `out[6]`, and `out[6]` is `00811150`'s first argument at `0070D2F1`. The producer agrees: `0070ED30` stores `00811180`'s perpendicular out-parameter into `+10h` (`0070EEBC`) and its arc-length out-parameter into `+20h` (`0070EEB1`), and the Lua names are `x` and `z` |
| `docs/SHIP_AI_FOLLOW_LAND.md`: the station point is the slot "minus while making way and plus while not" | it is **plus** while the latch at `state+2Ch` is set and **minus** while it is clear | `009DF361 CMP byte [ESI+2Ch],0` then `009DF36F JZ 009DF399`; the fall-through at `009DF371` reaches the `FADD` pair at `009DF388`/`009DF393`, the jump target at `009DF399` reaches the `FSUB` pair at `009DF3B4`/`009DF3BF`. The same convention holds for the probe offset (`009DF455`) and for the direction subtraction (`009DF4E4`), where only "set means plus" keeps the re-derived direction pointing forward |
| `docs/SHIP_AI_PATH_CORRIDOR.md`: "The record is `34h` bytes so at most four fit in each array, which is this packet's hypothesis from the record size alone" | four is the image's own bound | `0070E84B CMP EDI,4` closes the Lua `dist` loop after four `(x, z)` pairs, and `0070ED30` writes exactly four pairs (`0070EEB1`-`0070EF1B`) |
| `docs/SHIP_AI_PATH_CORRIDOR.md`: "nothing read for this packet writes either column, so the two meanings stay provisional" | settled; `0070ED30`, `0070EFD0` and `0070E620` are the three writers | as above. The header constants `kShipAiUnitGroupMemberLateral` and `kShipAiUnitGroupMemberAxial` were already the right way round |
| `include/bsp/ship_ai_path_corridor.hpp`: `ShipAiUnitGroupMember::field_04` "unread here" and `field_30` "unread here" | `+4h` is `relativePosition` and `+30h` is the published member speed | `0070EDB9`, `0070E78D`, `0077FB9A` for the first; `0070D100`, `0070D140`, `0070DAF9` for the second. The struct is **not** redefined; `bsp/ship_ai_formation.hpp` adds named offsets and two accessors that round-trip `field_30` as a float |
| `009DF2D0`'s ledger evidence: "It publishes a per-member speed back into the formation record's `+30h`" (unqualified) | it publishes `reference_speed * 1.25 / max(blend, 0.25)`, where `blend` is a clamped interpolation between two turn-radius ratios over the along-track offset | `009DF645`, `009DF664`-`009DF69A`. Appended to the ledger record, the old evidence kept |

## Findings worth carrying

* **`00810630` leaves its fifth out-parameter unwritten when `along <= 0`.** The branch
  `00810645-00810718` writes `param_3` and `param_4` and returns without touching `param_5`; it
  even reuses `param_5`'s incoming stack slot `[ESP+0x4C]` as an angle scratch at `00810658`. So
  `0070D290`'s `out[4]` is stale whenever a member's station is abreast of or ahead of the leader,
  and `009DF2D0` consumes it as a yaw rate at `009DF3F9`. Because `009DF2D0`'s frame is the same
  depth on every tick, the slot in practice holds the previous tick's value. No run-time evidence
  was taken; `bsp_game.exe` does not reach this path (no formation is created there yet).
* **`group+500h` has exactly two writers**: the constructor (`0070DB14`, zero) and the Lua reader
  (`0070E69A`). No searched routine that takes the group as `this` writes it. The player-facing
  reshape writes `group+4FCh` and column 0 instead, which is consistent only because every runtime
  group's `shape` is 0.
* **`0070ED30` indexes the tables by the join index; `0070EFD0` by a leader-skipping counter that
  starts at 1.** The two agree only when the leader is member 0, which `0070D7B0` guarantees for a
  runtime group but the Lua reader does not.
* `0070D0F0` is `group+504h`'s getter and the only reader of it outside `0070DA00`;
  `009F4DA0 BSP_ShipAi_ThrottleCeilingStep` calls it at `009F4E34` right after `0070D140` at
  `009F4E1F`.

## Follow-up packets

| packet | addresses | what is left |
| --- | --- | --- |
| `ship_ai_wake_trail` | `00811180` (`00811180-00811812`), `00810630` (`0081071B-008108F2`), the producer of `entity+0BD8h`..`+0F98h` | the trail's writer: who appends a sample, at what interval, and what `+14h` is exactly (a commanded yaw rate, a measured one, or a curvature). The two consumers here are read; the ring's producer is not. It decides whether the station point lags the leader by a fixed distance or a fixed time |
| `ship_ai_formation_orders` | `0077F940` tail, `0077FAD0`, `0077BD70`, `0077BDC5`/`0077BDD7`, `0070D8D0`, `0053ACD0`, `0053AFE7`, `00536A90` | the order path: the HUD formation menu, the `vtable[114h]`/`vtable[58h]` follow-up each joined unit gets, and the leave/disband arm. Closes which integer the menu sends for line, column and diamond |
| `ship_ai_formation_lua` | `008996A0`, `00899810`, `00899980`, `008901C0`, `0088FFD0` whole | the five `luaMW_*Formation*` bindings whole, so a mission script can build and query a formation |
| `ship_ai_group_observer` | `00694A60`, `00694AF0`, `0070E4C0`'s middle | what the observer pair does on a member's death, and whether it is what calls `0070ECA0` |

## no_ghidra_function

none. Every address named, documented or reconstructed here is inside an existing Ghidra function
whose body range is quoted in the Coverage table and in `reports/ship_ai_formation.json`.
