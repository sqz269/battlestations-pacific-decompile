# The ring winner: the NaN, and where the AI's order really goes

Addresses: `009E46F0` (the arc centre), `009E6870` (the arc score), `009F1BC0` (the store at
`009F27CB`), `009E76D0`, `009E3C00`, `0071EB60`, `00414EB0`, `00825F7C`. Constants `00D7A3A0`
(0.1), `00CE3830` (pi/2), `00CE3828` (2*pi).

## Half 1, the NaN

### The answer in one line

The reconstruction diverged, and at a producer, not in the arithmetic. `nested+11DCh`, the centre
of the standoff arc, had **no writer** in this process, so it was a permanent 0. Both arc edges
then collapsed onto bearing 0, ring slot 0 sits exactly on bearing 0, its distance to the nearest
edge was exactly 0, and with a zero span the divide at `009E6935` computed `0 / 0`. The native
does not do this, because `009F27CB` gives that field a live bearing every frame.

### Why the NaN reached the winner

`009E7BE0` sums five words per slot and `009E79CA` seeds the winner with slot 0. `009E7BFA` is a
strict `FCOMIP` / `JBE`, so an unordered comparison keeps the incumbent. Slot 0's total was the
NaN, every later slot compared unordered against it, and the seed survived all sixty.

The measured words before the fix, identical on all fourteen ships:

```
words Haguro  raw_18=6700.0000 norm_2c=10.0000 pen_30=-nan(ind) bear_34=0.0733 evade_38=0.2500 avoid_3c=0.0000
```

Only `+30h` is NaN. It is written by `ship_ai_ring_scan_arc_score_009e6870`, which is faithful:
`009E6931`'s `FCOMIP` takes the divide arm when `span >= nearest` **or the pair is unordered**,
and `009E6935` is `nearest / span * scaled_span`. With `nearest` and `span` both 0 that is `0/0`
in the image too. The input is what was wrong.

### `009E46F0`, the missing producer

`__thiscall float(blk /* ECX = brain+8h */)(float* unitXZ, std::uint8_t* out11D4,
std::uint8_t* out11D5)`, `RET 0Ch` at `009E47D0`, `009E4862` and `009E4899`, body
`009E46F0`-`009E489B`. Called once per frame at `009F27C6` and stored to `nested+11DCh` at
`009F27CB`. Two arms:

* **Path arm**, taken when `[blk+2F4h]+1Ch` is non-zero (`009E46FC`). `009D5AE0` and `009D5B90`
  fill the two override bytes `nested+11D4h` / `nested+11D5h`, whose addresses are pushed by the
  caller at `009F27A4` / `009F27AB` and which the mode-2 block later tests at `009F283A` /
  `009F283F`. `009E4813` calls `009E3C00` for the next path point with the unit's own x and z as
  the query, then `009E481C` / `009E4826` take `point - unit` in that order.
* **Command arm**, `009E4707`. `blk+3FCh` is the unit entity; `vtable[114h]()` on it yields the
  object `0071EB60` turns into the active command's target descriptor. `009E4759` tests
  `descriptor+1h` and takes `descriptor+8h`, its position, or else the global vector at
  `00F87574`, which the image ships as **twelve zero bytes**, so that fallback is the world
  origin and not an unknown.

Both arms end the same way: `009E4843` stores the squared planar distance, `009E4851` compares it
against `00D7A3A0` (`0.1f` widened) and only the reached side runs `atan2` at `009E4865`, the
`FSUBR` of pi/2 at `009E4872` and the wrap by 2\*pi at `009E4882`. That tail is exactly
`ship_ai_approach_heading_from_delta`, which was already on main. Everything else returns `0.0f`
from the `FLDZ` at `009E485B`.

At the `atan2` call `ST0` is `dx` and `ST1` is `dz`, established by walking the stack from
`009E482D`: `FLD dz`, `FLD ST0`, `FLD dx`, `FLD ST0`, `FMUL ST0`, `FLD ST2`, `FMULP ST3`,
`FADDP ST2`, `FXCH`, `FSTP`. The two pops at `009E4857` and `009E4859` before the zero return
confirm two live registers.

### Host methods

| method | native | note |
| --- | --- | --- |
| `ShipAiApproachPoint::arc_centre_009e46f0` | `009E46F0` | new; 8400 calls on USN02 |
| `ShipAiApproachPoint::arc_centre_command_arm` | `009E4707` | the arm USN02 actually takes, because these ships carry no path plan |
| `ShipAiApproachPoint::arc_centre_no_command` | `009E4726` | recorded when the unit has no active command descriptor |

New rule `ship_ai_approach_arc_centre_009e46f0(have_point, dx, dz)` in
`src/ship_ai_approach_update.cpp` holds the shared tail. The two arms are resolved in the host,
because both of their sources are host state.

**Divergence, deliberate.** When the path walk returns no point the image subtracts whatever
`009E3C00` left in the record; this host refuses instead, because the alternative is a bearing
computed from a zeroed output field.

### Validation

Build `scripts/build.ps1`, Win32 Release, `/W4 /WX`, clean. Tests 2 of 2. USN02, 3000 mission
frames.

| number | before | after |
| --- | --- | --- |
| `pen_30`, all ships | `-nan(ind)` | 3.51 to 3.94 |
| slot totals best / worst | `-nan` / `-nan` | 14.17 / 10.98 |
| ring winner first / last, Haguro | 0 / 0 | 37 / 56 |
| the same, Jintsu / Yudachi / Samidare | 0 / 0 | 23 / 0, 52 / 57, 8 / 0 |
| heading_changes, four ships | 532 / 572 / 524 / 578 | 347 / 290 / 480 / 389 |
| committed slot first / last | 30 / 28 | 30 / 28 |
| standoff | 1450 / 1550 / 200 | 1450 / 1550 / 200 |
| shots | 853 | 872 |
| hull | 119 | 125 |
| deaths | 3 | 3 |
| total_damage | 12463.2 | 13673.7 |

**Attribution.** One input moved and everything follows from it. With a live arc centre the arc
cost is finite, so all sixty totals are ordered and `009E7BFA` can actually take a new incumbent:
the winner stops being the seed and ranges over the ring. The published heading therefore settles
rather than flapping, which is why heading changes **fall** by a third to a half. That heading
feeds `009E6A90`'s throttle limiter, the ships' speeds differ, their tracks differ, and the
gunnery census moves with them: 19 more shots, 6 more hull hits, 1210 more damage, the same three
deaths. The committed slots and standoff ranges are sampled first and last only and are unchanged
at those two instants.

An intermediate run with the path arm alone changed nothing at all: `arc_centre_command_arm` was
recorded 8400 times, once per scan, because no ship on USN02 carries a path plan. The command arm
is what produces the bearing here.

USN01 for the record, same settings, clean: `hull=23 deaths=1 total_damage=220.0`, unchanged from
the previous packet. No ship on that mission runs the attackmove approach, so the arc centre never
reaches it.

## Half 2, where the AI's order actually goes

`ShipAiOrder::slot_to_order_ring` is recorded 96000 times per run at `00825F7C`, and **its name is
wrong**. `docs/UNIT_AI_ORDER_SLOT_READER.md` has since read `00825F7C`..`00826D6B` and it is not
an AI publisher. It carries the hop from the ring's own ordered rudder at `+984h` (`00826C61`)
through `00811890` (`00826C75`) to `unit->vtable[50h]` (`00826CDB`), and, gated on `unit+61h` at
`008266C1`, the inlined manual-autopilot ring setters at `008266CE`..`0082674C`.

That document also supersedes the negative result this record's comment was quoting from
`docs/UNIT_AUTOPILOT_PAIR.md`. Readers of `slot+40h`, `+44h` and `+48h` **do** exist, twelve of
them, and none is on the ring path. Its rel32 scan over `.text` for every call reaching `00816A40`
or `0080DAD0` finds the ring's write cursor filled from exactly three HUD order routines and
nothing else, so on the image's evidence **the AI never writes the order ring**.

The contract is therefore not "bind a missing publisher". It is: the AI's order lands at
`unit + 0A98h + 54h * index`, `00825F2C` flips the index and `00811D10` copies the slot across,
both already reconstructed and run here; what is unimplemented at `00825F7C` is the rudder-to-yaw
hop above, whose one unresolved question is named in that doc and not resolved here: the two call
sites of vtable slot `50h` disagree on the prototype, `0081196C` calling it with nothing pushed
and `00826CD5` setting up one outgoing float.

This packet corrects the record's comment and the run's own log line to cite the superseding
document; it does not rename the census entry, so the counts stay comparable across runs.

## Corrections

Appended to `docs/SHIP_AI_RING_SCAN.md` and `docs/SHIP_AI_APPROACH_UPDATE.md`. The comment at the
`ShipAiOrder::slot_to_order_ring` record in `src/game_hosts_ship_ai.cpp` is corrected in place,
with the old claim quoted in the new text.

## no_ghidra_function

none. `00825F7C` has no function of its own and was not read here; it lies inside
`00825F20 BSP_UnitInstance_UpdateShipMotion` and is cited from
`docs/UNIT_AI_ORDER_SLOT_READER.md`.

## Follow-up packets

- `ship_ai_vtable50_prototype`: the two call sites of unit vtable slot `50h` disagree, `0081196C`
  pushing nothing and `00826CD5` one float. Until that is settled the rudder-to-yaw hop cannot be
  bound.
- `ship_ai_arc_centre_path_arm`: `009D5AE0` and `009D5B90`, the two override bytes the path arm
  writes into `nested+11D4h` / `nested+11D5h`. No ship on USN02 takes that arm, so it is untested
  here.
- `recon_sensor_pass_rule_c`: `00806840` and `008048A0`, still open.
