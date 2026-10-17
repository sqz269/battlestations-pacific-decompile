# CC13 FIX1: the command-building moveto in 009F7C90's +3Dh arm

Packet cc13-fix1 implements finding R3 of `docs/CC13_VERIFY11.md` in `src/game_hosts_units.cpp`.
The cc9 grant of 2026-10-17 covers the edit. The behaviour sits behind a new switch,
`GameUnitsHost::Impl::kSquadronCommandBuildingMoveToBound`, which is **OFF**. With the switch OFF,
the host does exactly what it did before.

## The image (009F7D01-009F7D8A)

`009F7C90` is `__thiscall(ECX = the squadron auto-target object)`, `RET 0`. `[esi+4]` is the
director and `[esi+8]` is the squadron. The director is the squadron's command block:
`0084D8BD-0084D8C1` pushes the squadron and ESI (the block), then calls `009F7590`.

The +3Dh arm runs when `+3Dh` is set and `007EE720(squadron)` is true (`009F7D01-009F7D15`). In
order, it does this:

1. `d = 00521EA0([esi+4]+58h)` (`009F7D17-009F7D1D`) resolves slot 0's command target. A null `d`
   jumps to `009F7D7B`.
2. `d->vtable[5Ch](1Ch)` (`009F7D28-009F7D35`) asks IsKindOf MCommandBuilding. False jumps to
   `009F7D7B`.
3. `0071BE40()->vtable[0Ch]()` (`009F7D37-009F7D50`) reads the current command's category. It must
   be 1 or 2, or the arm jumps to `009F7D7B`.
4. `0071BE60()` (`009F7D52-009F7D5D`) counts the filled slots. It must equal 1, or the arm jumps
   to `009F7D7B`.
5. `0071ECF0(00E08F68, 004684F0(&local, d))` (`009F7D5F-009F7D75`) issues a moveto on the same
   director. `004684F0` builds the object target from `d`:
   - kind 1, byte +1 = 0, id = `d+174h`, object = `d`;
   - the position is `00F87574`, which holds 0, 0, 0;
   - +14h is 0.0.
6. `009F7D7B-009F7D8A` runs whichever way the tests went: `0071E430(current, 1)`, where
   `current` is re-read after the issue through `0071BE40`.

So the moveto is issued **before** the command ends, and the end runs either way.

## The host

`command_building_moveto_009f7d17` is a lambda in `run_spent_ordnance_rtb_0084e010`. Both host
models of this arm call it immediately before their `end_command_0071e430(leader, command, true)`:

- the fighter arm (`kFighterSpentEndBound`);
- the dead-target retest arm (`kSquadronDeadTargetRetestBound`).

Its reads map to the host like this:

| Image | Host |
|---|---|
| `00521EA0([sq+4]+58h)` | the leader's `command_target_plus_one`; this is the same resolved target that the retest arm already reads |
| `vtable[5Ch](1Ch)` | `unit_is_kind_of(d, 0x1C)` |
| `0071BE40` category | `bsp::entity_order_command_class_by_address(commands.current_command_0071be40(leader))->category` (the registry's vt+0Ch) |
| `0071BE60` | `commands.director_leading_slot_categories_0071df83(leader, ...)`, which walks the same slots: from +54h in steps of 1Ch, stopping at the first null, at most 10 slots |
| `0071ECF0` | `issue_script_command(sq.squadron_unit, 0x00E08F68, target, 1, ...)`, which builds the same target that `0084E595`'s moveto-self (`kSquadronMoveToSelfBound`) builds |

What happens to the members' rows does not change. The end then retires each live member's rows
and clears their targets, as before. The moveto row lives on the squadron unit.

### Uncertain (LABELLED)

- **Two different directors.** The host puts the issue on the squadron unit because `0084E595`
  issues into the same command block, and cc9-lua59 placed that issue there. The four reads use the
  leader's director and target, because the host's end path already uses the leader as its proxy for
  that director.
- **The flags argument.** `0071ECF0` has no flags argument. The value 1 comes from the moveto-self
  precedent.
- **A null current command.** The image would call through a null current command. The host skips
  the issue instead.
- **A dead squadron unit.** As in the moveto-self issue, a dead squadron unit is not issued to.
- **A dead target.** On the retest arm the target is dead. The image's `00521EA0` would still have
  to resolve it from the handle table; the host does not model that and tests only the slot's
  class. If a dead building's handle no longer resolves in the image, this path over-issues.
- **Interaction with moveto-self.** The moveto-self tail later reads the leader's class, not the
  squadron unit's rows, so it does not see this moveto.

## What a pair run should show

Compare the switch ON against OFF on a mission where a squadron's ordnance-class order ends on a
CommandBuilding target. JM08 has HQ buildings, so try it first.

- **ON:** the log gains the line `command-building moveto: squadron "..." -> "<building>" ...
  (009F7D75, packet cc13-fix1)` immediately before the existing `fighter spent:` line or
  `squadron retest:` line. The squadron unit gains a moveto row over the building.
- **Everywhere else:** no change. A squadron whose target is not a CommandBuilding, or whose queue
  holds more than one command, behaves byte for byte as it did with the switch OFF.
