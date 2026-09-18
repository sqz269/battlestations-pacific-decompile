# Both bot speed setters now take the unit's own class row

Addresses: 007C47F0, 009C1850, 009C189A, 009C18A0, 009C18A7, 009BECD0, 009C44F0, 009C7A94,
007DB760, 007C4850.

Packet `cc8_bot_speed_class_rows`, owner `agent/cc8-torpedo-run-in`, on main `577934c0f` merged.

Two setters computed `007C47F0`'s product from defaults. Both now compute it from the unit's real
vehicle-class row, and **one of the two substitutions turns out to have been the wrong field rather
than a stand-in value.**

## 1. The product

`007C47F0(approach+8h)` is `tuning+24Ch Dynamics/SpdMultipliers/LevelFlight` times
`classDesc+184h StallSpd`. Both halves are available in this host and neither needs a default:

* the tuning row comes from the `PlaneGlobals` mirror, which `007E2A20` fills at load;
* `StallSpd` is on the slot as `plane_stall_spd`, loaded by `src/game_hosts_lua.cpp` from the
  vehicle-class row and already read by the free-flight arm at `007DB760` and by `007C4850`'s
  climb-angle solver.

A helper on the units host computes the product once and both setters call it.

## 2. The dive-bomb turndown: a stand-in value, retired

It used `kLevelFlightMultiplier * kStallSpeedDefault`, `1.8 * 17.5` = 31.5, on the note that *"the
class descriptor is not modelled here"*. It is modelled - on the slot - so the substitution is
retired and the turndown now sees its own aircraft's stall speed.

## 3. The move-to setter: the wrong field, corrected

`docs/TORPEDO_GLIDE_THROTTLE_WIRING.md` bound step 2 with the row's `TravelSpeed`, labelled a
substitution "because `007C47F0` and `009BECD0` are unread".

**`007C47F0` is not unread any more**, and it is not `TravelSpeed`. `009C1850` calls it at
`009C186A` and `009BECD0` at `009C1895`, so the desired speed is the `LevelFlight` and `StallSpd`
product, shaped by `009BECD0` against the distance. Substituting `TravelSpeed` was not a plausible
stand-in for the right quantity; it was a different class field.

For USN01's `Mav` row - `StallSpd` 19.4, `LevelFlight` 1.8 - the commanded speed moves from the
substituted **66.67 m/s** to **34.92 m/s**, a factor of 1.9.

**What remains substituted is `009BECD0`'s shaping**, not the speed, and the comment now says so.
That is a narrower and truer label than the one it replaces.

## 4. No other bot-task site takes a class field from a default

A census over the host's class-row consumers:

| site | field | source |
| --- | --- | --- |
| `src/game_hosts_units.cpp:2846` | `StallSpd` | the row |
| `:2935`, `:2947`, `:2955` | `MaxSpd` | the row |
| `:1120` `db_release_speed` | `TravelSpeed` | the row |
| `:902`-`905` (census) | `TravelSpeed`, `MaxSpd` | the row |

`kStallSpeedDefault` was the only class-field default in `include/bsp/dive_bomb_task.hpp`; every
other constant there is a tuning row or a geometry literal. **`009C7A94` is not a `MaxSpd` site**:
it is `dive_bomb_decay_dive_altitude_009c7a94`, a dive-altitude ceiling whose inputs are
`Pilot/DiveBomb` tuning rows, and its host call site passes `kCruisingAltitudeThird`, a tuning
constant rather than a class field.

## 5. The run, and why it cannot show this on USN01

**No run was taken, for two independent reasons, and neither is the startup crash alone.**

1. **The safe base is too old to carry the code.** `48F293197` predates
   `docs/TORPEDO_GLIDE_THROTTLE_WIRING.md`'s wiring, so `run_move_to_tick_009c18c0` does not exist
   there. Cherry-picking this change onto it conflicts, and resolving the conflict would mean
   running a function with no caller.
2. **The move-to branch never executes on USN01 anyway.**
   `docs/TORPEDO_ENGAGED_TEST.md` established it: the ordered bombers start 1490 m out against a
   4840 m engaged threshold, so they go to `attackrun` on the first arm tick and never occupy
   `moveto`. The glide census that prints the desired-speed pair fires only in that branch, so on
   this mission it prints nothing whatever the binding says.

So the census line was widened to print the stall speed beside the desired speed, and it will show
the per-class value the first time a mission puts a torpedo bomber outside the engage range. The
dive-bomb turndown's half **does** run, and its effect is visible in that worker's census rather
than mine.

## ABI

`007C47F0`, the product; `009C1850` `BSP_BotStateMoveTo_SetDesiredSpeed`, as recorded, calling
`007C47F0` at `009C186A` and `009BECD0` at `009C1895`.

## Uncertainty

* `009BECD0`, which shapes the product against the distance. Unread, and now the only substitution
  left in the move-to setter.
* `007C47F0`'s own body was not read; the product is taken from the dive-bomb worker's note on the
  turndown binding, which names it as `tuning+24Ch * classDesc+184h`, and it agrees with the
  turndown's two constants.

## Host methods

| host method | file | native | kind |
| --- | --- | --- | --- |
| `bot_desired_speed_007c47f0` | `src/game_hosts_units.cpp` | `007C47F0` | binding |
| the dive-bomb turndown's desired speed | `src/game_hosts_units.cpp` | `009C44F0`'s input | binding, **substitution retired** |
| the move-to setter's desired speed | `src/game_hosts_units.cpp` | `009C1850` | binding, **field corrected**; `009BECD0`'s shaping still substituted |

## Corrections

Appended to the doc it amends, and verified present there.

* `docs/TORPEDO_GLIDE_THROTTLE_WIRING.md` section 1: the move-to setter's substitution was the
  wrong field, not a stand-in value.

## no_ghidra_function

None.

## Validation

No run; section 5 gives both reasons. Nothing in this packet changes any code path USN01 executes
through the torpedo chain.

## Follow-up packets

1. **`009BECD0`**, the last substitution in the move-to setter.
2. **`007C47F0`'s body**, to confirm the product first-hand rather than from the turndown's binding.
