# The yaw-rate accessor's forward speed (packet `cc9_unit_yaw_rate_forward_speed`)

Addresses: 00811940 0082ECB0 0092D730

Worker cc9-lua8, 2026-09-28, from a defect cc9-ships8 found. The switch is
`kUnitYawRateForwardSpeedBound` in `include/bsp/game_hosts_units.hpp`.

## The defect

`GameUnitsHost::unit_current_yaw_rate_00811940` (`src/game_hosts_units.cpp`) builds a fresh
`UnitRudderBinding` and calls `bsp::unit_current_yaw_rate_00811940`. That reaches `00811890`,
which reads the body-axis speed through the binding's `forward_speed_0092d730()`. The accessor
never set the binding's public `forward_speed`, so it answered 0.0f, and `0082ECB0` returned 0 for
every unit. The binding's other two users set it first. **The fix** sets `rudder.forward_speed =
unit_forward_speed_0092d730(index)` before the call. A summary line counts the calls and the
non-zero answers.

## The callers (all in `src/game_hosts_ship_ai.cpp`)

| binding | image site | what the rate drives |
| --- | --- | --- |
| `ShipAiControls::unit_current_yaw_rate_00811940` | `009ED9C1` (`src/ship_ai_states.cpp`) | in Rudder steering mode, the heading target `+324h` = heading ± the yaw rate (`+364h` picks the sign) |
| `NeighbourAvoidBoxBinding::observed_command_yaw_rate_00811940` | `src/ship_ai_neighbour_box.cpp` 450 | the neighbour's predicted turn in the avoid box `009EAE20` / `009EAFC0` |
| `SubTargetLeadBinding::yaw_rate_from_rudder_0082ecb0` | `009E2A97` (`src/ship_ai_attackmove_substates.cpp`) | the attackmove lead pursuit's predicted target turn. The binding is still UNIMPLEMENTED in the native table, but it returns this accessor's value |

## OFF census (this tree's build of `a31730f54`, 3200/3000, the reference parameters)

| row | accessor calls | non-zero | Controls | Neighbour | Lead |
| --- | --- | --- | --- | --- | --- |
| JM06 | 53010 | 0 | 16 | 50592 | 2402 |
| USN02 | 31870 | 0 | 1 | 31869 | 0 |
| USN12 | 8976 | 0 | 3 | 8973 | 0 |
| USN04 | 44737 | 0 | 16 | 44721 | 0 |
| USN01 | 19426 | 0 | 16708 | 2718 | 0 |

The OFF logs are `local\l8_yoff_<row>.log` in worktree cc9-lua8.

## Predictions, written before the ON runs

ON is `pair_export --commit a31730f54 --flip kUnitYawRateForwardSpeedBound=true` (`local/l8_y_on`).
- **Every row:** the same call counts to within the drift the change itself causes, and `nonzero`
  rises from 0 to most of the calls. A unit with no rudder or no speed still answers 0.
- **JM06, USN02, USN12, USN04: exit 3.** The neighbour box now predicts each neighbour's turn,
  so the avoid boxes of turning ships move. Ship tracks move and, with them, gunnery (shots,
  hits, first hit). JM06 also moves through its 2402 lead-pursuit calls. The size is not
  predicted: the rate is small at low rudder, so the moves may be small.
- **USN01: exit 3.** It is not an identity row. Its 16708 Controls calls put the yaw rate into
  the Rudder-mode heading target, so the ships steering in that mode hold a heading offset by
  their turn rate.
- **Mechanism check:** no row may show a zero `nonzero` while the unit table shows a ship
  turning under way.
