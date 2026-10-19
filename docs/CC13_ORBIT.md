# The LOMP10 Ashigara origin orbit (packet cc13-orbit, read-only)

cc9's nav-set pair: with the 3E0h cruise seed ON (`kShipAiCruiseDistanceSeedBound`, together with
`kShipAiNavigateAsternSkipBound`), LOMP10 Ashigara drives to about (0, 0), arriving at step 8480.
It then circles within 500 of the origin until 1800 s. OFF wins at 1303.70 s.

Logs (cc9-ships44, read only): `s61_n4_l10.log`, `s61_n3s_l10.log`, `s61_off_l10.log`.

## Verdict

The ship is steering at the world origin because nested+11DCh, the approach reference
bearing, is the bearing to (0, 0). The host reads it through the wrong arm of `009E46F0`:
- the image picks the path arm when the live plan's `+1Ch` (`search_state`) is non-zero;
- the host tests `node_count` (`+34h`) instead, which is the path follower's cursor, not a
  "plan exists" flag.

So with a live plan whose cursor sits at node 0, the host takes the command-descriptor arm. For
an entity target such as CB4 that arm yields the zero vector `00F87574`, the world origin.

The seed does not create this. In OFF the same 1D8h already points at the origin, but OFF steers
mode 3 through the navigation arm toward the goal, so nothing follows it. With the skip ON, mode 3
steers by 1D8h. With 32Ch = 0, the -0.625 astern flips (docs/CC13_MODE3STEER.md section 2) kept
the ship from following it. The seed removes the flips, so the ship follows 1D8h to the origin and
circles it, because the bearing to a point it is passing over keeps turning.

## 1. Which image path writes 1D8h in mode 3

1D8h has one writer on this path, the attackmove approach `009F3240`:
- `009F3314` reads sub+1214h (= nested+120Ch);
- `009F335C` stores it at brain+1E0h, which is blk+1D8h;
- `009F3360` wraps it (`00605070`);
- `009F3375` sets mode 3.

`009ED947` then copies 1D8h to 324h every step (`ship_ai_states.cpp`).

nested+120Ch is written by `009E5E90`, called at `009E7ECB` by the ring scan `009E76D0` with the
bearing of the winning ring slot, `ring[winner].angle_08`. The goal-override arm at
`009E7C72..009E7EA6` would replace that bearing with the unbinned bearing to brain+0B2Ch, but it
needs nested+1209h. The logged values are slot angles, so it did not run.

The ring has 60 slots (`kShipAiApproachSlotCount`, `009E7FCC`) at 6 degrees (`009E56F9`); these
are the "sector-bin headings" (-2.1991 = -21pi/30, -1.7802 = -17pi/30, ...).

The slot totals are anchored on nested+11DCh. It is read by:
- the class rating `009E5DA0` (query word 5, `009E8153`, minus the slot angle at `009E5DB4`);
- the standoff arc `009E6870` (its centre, `009E74B7`);
- the obstacle probe `009E6640` (the probe-length bearing).

So the winning slot is the one nearest 11DCh unless an obstacle or the arc penalty moves it.

## 2. The point behind 11DCh

| field | image writer | host writer |
| --- | --- | --- |
| nested+11DCh, reference bearing | `009F27C6` -> `009E46F0` (every pass); `009F28AE` (approach mode 2 only, bearing to the target unit) | `ship_ai_approach_update.cpp` (`slot_scale_11dc`) from `ship_ai_arc_centre_next_point_009e46f0`; `game_hosts_ship_ai.cpp` mode-2 arm |
| `009E46F0` path arm point | `009E47D3..`: `009E3C00` on the live plan `[blk+2F4h]`, the next path point | the same call, but only when `node_count != 0`; it also refuses when `record.node_18 == 0` (labelled divergence) |
| `009E46F0` command arm point | `009E4714..009E4769`: unit `[blk+3FCh]` vtable[114h], then `0071EB60`. If descriptor+1h is set it uses descriptor+8h, otherwise the twelve zero bytes at `00F87574` | `GameCommandsHost::active_command_descriptor_0071eb60` (director slot 0 or the override descriptor) |
| the descriptor for an entity order | `00465080` (read from bytes): +0h byte = 1 for an object, **+1h = 0**, +8h..+10h copied from `00F87574` (zeros) | `kind = 1`, `position_valid = 0`, position zero (e.g. `game_hosts_units.cpp`'s home-base order) |

**The arm test, from the bytes.** `009E46F6 MOV ECX,[ESI+2F4h]`, `009E46FC CMP dword [ECX+1Ch],0`,
`009E4701 JNE 009E47D3`. The path arm is taken when the live plan's +1Ch is non-zero.

`ShipAiPathPlanBlock` (`ship_ai_path_planner.hpp`) maps +1Ch to `search_state`:
- 0 means Empty (`009D9D69`);
- a seeded, searched, extracting, ready or failed plan is non-zero (`009E381E`, `009EC6B0`,
  `009EC71A`, `009EC742`).

The host's `ship_ai_arc_centre_next_point_009e46f0` (`game_hosts_ship_ai.cpp`) instead tests
`live.node_count == 0`, which is +34h. +34h is a follower cursor:
- `009E3C3F` and `009EC721` zero it;
- `009E421F` increments it.

Ashigara's order is against CB4, a CommandBuilding: `command target 0071EBF0: unit=Ashigara
token="CB4"`, and `fire=CB4`. That is an entity descriptor, so the command arm returns
(0, 0) - (unit x, unit z), and 11DCh is the bearing to the world origin.

**The logs agree.**
- `ShipAiApproachPoint::arc_centre_command_arm` count equals the `arc_centre_009e46f0` count in
  both runs (OFF 6498/6498, ON 9334/9334). Every call took the command arm, although the planner
  allocated nodes (OFF 516, ON 4578 `allocate_path_node`).
- `desired_1d8` is the slot nearest the bearing from the ship to (0, 0), using heading =
  atan2(dx, dz), the convention that gives -2.3038 for the opening leg:
  - ON: 120 of 120 sampled rows up to step 8460 are within one slot;
  - OFF: within one slot until about step 16000, at (1379, -762) d1d8 = -1.0472 against -1.0075
    to the origin. Later rows differ, after OFF's approach state changes near the goal.
- OFF's 324h (3.8167, d330 = the falling goal distance) comes from the navigation arm, not from
  1D8h. That is why OFF reaches (-2861.6, -4039.0).

## 3. The 32Ch / 009DC313 side (cc9's suspect)

The suspect is real, but it acts on 324h, not on 1D8h:
- The arm final queries `009DC2E0` with range = min(32Ch, look-ahead share) (`009DE633`).
- `009DC313` refuses a range under 10 (`kFreeBearingMinRange`). With 32Ch = 0 it always refused;
  with 1000 it runs.
- An accepted answer rewrites 324h with an avoid-zone corner bearing (`009DF0FA..009DF109`).

That accounts for the 324h values that differ from 1D8h inside a frame (step 8500: tgt324 =
0.1047, d1d8 = -1.7802). The probe reads the unit's own position and the zone outline, both
populated, so no unpopulated point is involved.

The trap is 1D8h. The escape and separation terms only perturb the orbit.

## 4. Fix proposal (no build yet)

**Switch.** A new OFF switch, for example `kShipAiArcCentrePlanTestCc13Bound`. Under it,
`ship_ai_arc_centre_next_point_009e46f0` takes the path arm when `live.search_state != 0`
(`009E46FC`), not when `live.node_count != 0`. With a live plan, 11DCh then becomes the bearing
to `009E3C00`'s next path point, toward CB4.

**Keep the record-node refusal for now, with a counter.** The host returns false when
`record.node_18 == 0`. That gives 11DCh = 0, which is bearing 0, a fixed north and not the
origin. The image instead subtracts whatever `009E3C00` left in its stack record. Count how often
that happens under the switch before modelling it.

**Log-only first.** Count, per call, the arm the image would take: `search_state` non-zero while
`node_count` is 0. That confirms the reach on LOMP10 without moving anything.

**Pairing.** Flip the switch together with the seed and the skip. Pass criteria:
- LOMP10 Ashigara's d1d8 tracks the bearing to the next path point, not to (0, 0);
- the mission completes.

Re-check IJN18 and the s60 set, because every ship with an entity order whose plan cursor sits at
0 changes 11DCh.

**A faithful image result.** When no plan exists (`search_state == 0`), the image also aims an
entity-target attackmove at the origin. If Ashigara has no plan in the image either, the orbit
is faithful, and the difference from the game lies upstream in the planner.

## Uncertainty

- The image values of `search_state` for Ashigara are inferred, not observed. The host never logs
  it at `009E46F0`. The log-only counter above settles that.
- The ring-slot argument (section 1) relies on the host reconstructions of the four slot scorers.
  I did not re-read their bytes for this packet.
- The 1D8h-to-origin match uses the heading convention atan2(dx, dz), inferred from the opening
  leg, and the 6-degree slot width. It is a fit to the log, not a trace of the 009E5E90 arm
  choice.
