# How an AI command reaches its member units

Addresses: 00A02020, 00A10C20, 00A10DC0, 00A124E0, 00A152B0, 00A12A90, 00A15490, 00A15500,
00A152E0, 00A11F70, 00A12430, 00A11070, 00A11690, 00A11AF0, 00A11B80, 00A13B60, 00A14DD0,
00A126C0, 00A15570, 00A156E0, 00A11FF0, 00A10EC0, 0077D600, 0077C8D0, 00417B10, 00414DB0,
009FE080, 007EDA90, 009FFEB0, 009FFC10, 00A371A0, 00A2BD00, 00E08F68, 00D21530, 00E17BF2,
00CE380C, 00F87574.

Packet `cc8_ai_command_tick`, read-only analysis. Every descriptive name here is a hypothesis, not
a recovered symbol.

Coverage. Complete as rules: the order bridge `00A02020` (`00A02020`-`00A02175`), the leader point
`00A10C20`, the follower pass `00A10DC0` (`00A10DC0`-`00A10EB6`), and the `vt+0Ch` bodies of
`MOVETO` (`00A124E0`-`00A1253F`), `CAUTIOUSMOVE` (`00A152B0`-`00A152D1`), `MOVETOATTACK`
(`00A12A90`-`00A12C53`), `CLOSEATTACK` (`00A15490`-`00A154FB`) and `DEFENDPOSITION`
(`00A15500`-`00A1556A`). **Partial, by address range**: `00A13B60`-`00A14D9F`,
`00A14DD0`-`00A152A7`, `00A11070`-`00A113C1` below its two named calls, `00A11690`-`00A11AEC`,
`00A11AF0`-`00A11B73`, `00A11B80`-`00A11F63`, `00A10EC0`-`00A1106A`, and the `REGROUPINGMOVE`
(`00A126C0`-`00A12A4C`), `PATROLTO` (`00A15570`-`00A156D3`), `RETREAT` (`00A156E0`-`00A158FF`),
`SELLING` (`00A11FF0`-`00A1242D`) and `CAUTIOUSATTACK` (`00A152E0` onward) bodies, read only to
their first dispatch.

## `00A02020`, the one bridge to a scene command

`__fastcall(Entity* member, const float* point)`, body `00A02020`-`00A02175`, read in full.
`docs/AI_COMMAND_OBJECT.md` left open how a command could reach a unit; this is the answer, and it
is the only one. Ten routines call it, seven of them `vt+0Ch` ticks or their helpers.

```
if (member->vtable[+5Ch](18h))  ok = !007EDA90(member)     ; 00A02026, 00A0203A
else                            ok = member->vtable[+5Ch](6)
if (!ok) return
if (member+C8h == 0) 00414DB0(member)                      ; 00A02052, refresh the pose
dx = member+FCh - point[0];  dz = member+104h - point[2]   ; 00A0205C-00A0206A
if (dx*dx + dz*dz < [00D21530]) return                     ; 00A0206E
if (member->vtable[+5Ch](6)) {                             ; 00A02089, a ship
    0082ADA0(0)
    p = 00417B10(&descriptor, &stack, [00CE38C8], 1)       ; 00A020E8
    out = { p[0], 0.0f, p[1] }                             ; 00A020F5-00A02103
} else out = point
descriptor = { kind 0, position_valid 1, object_id 0, object null, out, trailing 0 }
0077D600(member)(00E08F68, &descriptor, 1)                 ; 00A0214B-00A0216A
```

Three facts settle the packet's question.

- **The command descriptor is always `00E08F68`.** That is `moveto`, ordinal 15, category 3 of
  `docs/SCENE_COMMAND_TYPES.md` and `docs/COMMAND_CLASSES.md`. No AI command class issues any other
  scene command anywhere in this chain.
- **The path is the scene-command path**, `0077D600 BSP_Entity_IssueCommand` with the two-message
  hop of `docs/ENTITY_LUA_ORDER_PATH.md`. Nothing writes an order slot or a bot task directly.
- **The descriptor carries a point, never an object.** `kind` is 0 and `object` is null, so the
  target is resolved as a position. A group command therefore never names its enemy to the unit; it
  names where to go.

The dword at `00D21530` reads `00 00 00 00`, so `d2 >= 0.0f` always holds and the distance test
never blocks an order. Ghidra reports an overlapping symbol at that address, so the value is
recorded as read rather than as a designed threshold.

## `00A10C20` and `00A10DC0`, the shared halves

`00A10C20 BSP_AiGroup_LeaderPoint`, `__thiscall(group)`: the first member's `+FCh` pose, or the
zero vector `00F87574` when the group is empty (`00A10C29`).

`00A10DC0 BSP_AiCommand_FollowerPass`, `__fastcall(command)`, body `00A10DC0`-`00A10EB6`, read in
full. It returns unless the population is above one (`00A10DCB CMP [group+5644h],1 / JBE`) and its
walk starts at the node after the head, so the leader is skipped. Per follower:

| Test | Action | Evidence |
| --- | --- | --- |
| `vtable[+5Ch](6)`, a ship base | `0077C8D0 BSP_Entity_RequestJoinFormation(follower)(leader)` | `00A10E3E`, `00A10E61`-`00A10E67` |
| `vtable[+5Ch](18h)` and not `009FFEB0` | `00A02020(follower, 00A10C20(group))`, a `moveto` at the leader | `00A10E77`, `00A10E83`, `00A10E8C`-`00A10E98` |
| anything else | nothing | falls through to `00A10E9D` |

So a group moves as **one ordered leader plus ships joining its formation and squadrons sent to its
point**. Six of the nine concrete tick bodies call it.

`009FFEB0` is a second squadron exclusion, distinct from `007EDA90`: it returns false outright when
the byte at `00E17BF2` is set, and that byte is `00` in the image, so the carrier arm from
`009FFEC6` is what runs. That arm was not read.

## The class ticks

| Class | `vt+0Ch` | What it does |
| --- | --- | --- |
| `NONCONTROL` | `00A11F70` | `JMP 00A10EC0`, unread |
| `IDLE` | `00A12430` | `00A10EC0`, then **`00A10DC0`**, then `JMP 00A11070` |
| `MOVETO` | `00A124E0` | leader must pass `009FE080`, then `00A02020` at the command's own `+8h` destination, then `00A10DC0` and `00A11070` |
| `CAUTIOUSMOVE` | `00A152B0` | `00A14DD0` on the `+14h` base with the group and `+8h`, then `00A10DC0` and `00A11690` |
| `MOVETOATTACK` | `00A12A90` | close or promote, below |
| `CAUTIOUSATTACK` | `00A152E0` | `00A14DD0` on the `+20h` base with the target's leader point, then `00A10DC0`, `00A11690`, then the same `+1F4h` comparison |
| `CLOSEATTACK` | `00A15490` | `00A13B60(1.5f, target leader point, target group, 1)`, then `00A11B80` and `00A11AF0` |
| `DEFENDPOSITION` | `00A15500` | `00A10DC0` and `00A11690` first, then `00A13B60(1.0f, own leader point, null, 0)` and `00A11B80(0)` |
| `REGROUPINGMOVE`, `PATROLTO`, `RETREAT`, `SELLING` | `00A126C0`, `00A15570`, `00A156E0`, `00A11FF0` | all reach `00A02020`; bodies read only to their first dispatch |

### `MOVETOATTACK` is a two-state machine

`00A12A90` reads `00A371A0`'s `+1F4h`, which
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` names **`CloseAttack_CollectDist`**, shipped 5000
(`00D1AF84`). It refreshes both leaders' poses, takes the horizontal separation through `009FFC10`,
and at `00A12B93`/`00A12BA8` requires the own leader to pass `009FE080` **and** the distance to
exceed the field. That arm orders the leader at the target group's leader point
(`00A12BC9 00A10C20` on `command+1Ch`, then `00A12BD2 00A02020`) and runs the follower and formation
passes. Otherwise `00A12C1B` allocates `20h`, runs the `ATTACK` base constructor, installs the
`CLOSEATTACK` vtable `00D22C44` with the secondary `00D22C2C`, and hands it to `00A2BD00`.

So an attack is: close to 5000 units as a formation issuing `moveto`, then hand over to
`CLOSEATTACK`, whose `00A13B60` was not read. `CAUTIOUSATTACK` makes the same comparison at
`00A1535A`.

## Host methods

| Host method | Native | Note |
| --- | --- | --- |
| `command_tick` | `vt+0Ch` | runs `ai_command_tick_vt000c` for the classes read |
| `tick_issue_moveto` | `00A02020` tail | `issue_script_command` with `00E08F68` and a position descriptor |
| `tick_leader_point` | `00A10C20` | the group's first member's pose |
| `tick_request_join_formation` | `0077C8D0` | recorded; this process has no formation ring |
| `tick_avoid_zone_point` | `00417B10` | recorded; a ship is sent at the requested point |
| `tick_squadron_excluded_009ffeb0` | `009FFEB0` | recorded; answers what `007EDA90` answers here |
| `tick_replace_command` | `00A2BD00` | the `CLOSEATTACK` promotion |

## Corrections

Appended to `docs/AI_COMMAND_OBJECT.md` and `docs/AI_COORDINATOR_TICK.md`, never rewriting them.
`docs/AI_COMMAND_OBJECT.md` said the only path from a command to a unit "could only be the class's
own `vt+0Ch` tick, which this packet did not read". That is now read for five classes, and the path
is `00A02020` to `0077D600` with the `moveto` descriptor `00E08F68`.

## no_ghidra_function

| Address | Inclusive end | What it is |
| --- | --- | --- |
| `00A152E0` | not established | `CAUTIOUSATTACK`'s `vt+0Ch`; read to `00A15372` only, coverage partial |

## Validation

Three campaign missions, 3000 mission frames at 0.05 s. Before is this tree at `b1432783a`, the
commit this packet builds on, whose own after-runs are reused unchanged as the baseline. After is
this change.

| Mission | Run | `moveto` issued by ticks | Followers walked | Formation requests | Promotions | Scene commands total | Gunnery hits | Damage |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | n/a | n/a | n/a | n/a | 34 | 4 | 500.0 |
| IJN01 | after | 0 | 2386 | 2291 | 1 | 34 | 4 | 500.0 |
| USN01 | before | n/a | n/a | n/a | n/a | 17 | 25 | 229.5 |
| USN01 | after | 0 | 357 | 325 | 1 | 17 | 24 | 229.5 |
| USN02 | before | n/a | n/a | n/a | n/a | 14 | 160 | 18661.8 |
| USN02 | after | 49 | 681 | 681 | 0 | 63 | 161 | 19061.0 |

Attribution.

- **USN02 is the case where the tick runs end to end.** Its `MOVETOATTACK` group never promotes,
  so the closing arm orders its leader every 2 to 4 seconds: 49 `moveto` scene commands, which is
  the whole of the rise from 14 to 63 commands. The gunnery census moves with it, 160 hits to 161
  and 18661.8 damage to 19061.0, because the ordered units end up in different places.
- **IJN01 and USN01 issue nothing, for one named reason each time.** Both promote on the first
  tick with `groupable=0`: `009FE080` rejects the group's first member. The distances at the moment
  of promotion were 4756.1 on IJN01 and 75.7 on USN01, both already below the shipped
  `CloseAttack_CollectDist` of 5000, so the native would also have promoted there. Once the command
  is `CLOSEATTACK` its tick is `00A13B60`, which was not read, so nothing further is issued.
- **The follower pass is what actually moves.** 2386, 357 and 681 followers walked, of which 2291,
  325 and 681 are ships answering `vtable[+5Ch](6)` and receiving a formation request. No plane
  squadron follower appeared on any of the three, which is why the follower arm of `00A02020` never
  fires and `tick_orders` comes only from the `MOVETOATTACK` leader arm.
- **The pilot-attack table is unchanged on all three.** `moveto` is not an attack order, so the yaw
  arm of `docs/PILOT_BOT_PLAN_CONTROLS.md` sees nothing new. USN02's table still reports no unit
  ordered at a plannable target.

### Two named missing inputs

- **`00A371A0 + 1F4h`, `CloseAttack_CollectDist`, is `0.0` in this process; the shipped value is
  5000 at `00D1AF84`.** `ai_tuning_load_00a335d0` reads six keys and this is not one of them, so
  `tuning.at(0x1F4)` answers the zero an unloaded slot holds. The measurement prints it as
  `collect_dist=0.0`. With the field loaded the comparison would still promote on all three
  missions, because the measured distances are below 5000, but the rule is running on a zero.
  Loading it needs `include/bsp/ai_tuning_globals.hpp` and `src/ai_tuning_globals.cpp`, which this
  packet does not own.
- **`009FFD70`'s leader key is unread**, so this process orders the member list by unit index while
  the native orders it by that key. The first member is therefore not the native's leader, and on
  IJN01 and USN01 it is a unit `009FE080` rejects. That, not the tuning field, is what stops the
  `MOVETOATTACK` closing arm on those two missions.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_command_engage_pass` | `00A13B60`-`00A14D9F`, `00A11B80`, `00A11AF0` | what `CLOSEATTACK` and `DEFENDPOSITION` do with the point, the target group and the 1.5f/1.0f radius; the largest unread body in the chain |
| `ai_command_cautious_pass` | `00A14DD0`-`00A152A7` | the `CAUTIOUSMOVE` and `CAUTIOUSATTACK` third base, and whether `CautionMove_Dist` (`+1F0h`, 8000) enters there |
| `ai_group_formation_shape` | `00A11070`-`00A113C1`, `0070EFD0`, `0070D080`, `0077C880` | the formation pass every move tick ends with, and what `0077C8D0` does with a join request |
| `ai_group_leader_key` | `009FFD70`, `009FDF30` | the member ordering key, so the host's first member is the native's leader |

## Correction from docs/AI_CLOSE_ATTACK_TICK.md and docs/AI_COMMAND_INPUTS.md (2026-09-18)

Three claims above stop at `00A02020` and were wrongly generalised. The close-attack tick
`00A13B60` calls `0077D600` directly with a kind-1 descriptor that names the target entity, its id
and its object pointer, choosing the class `attackmove` when the served member is a ship base and
`settarget` otherwise; the cautious approach pass issues a third class, `clearorders`. So the
descriptor does carry an object on that path, `moveto` is not the only scene command an AI command
issues, and `00A02020` is the only bridge for the move family, not for every class. The candidate
pass, its scoring and the served-member gate are in docs/AI_CLOSE_ATTACK_TICK.md; on USN02 it takes
scene commands from 14 to 635.

The member ordering key is `009FFD80`, not `009FFD70`: `009FFD70` is a two-instruction thunk onto
`009FDF30` with `entity+C4h`, consumed by the merge strength test, the total-leader-weight sum and
the compose pass, while the sorted insert calls `009FFD80`, the per-class tuning weight scaled by
class (docs/AI_COMMAND_INPUTS.md). The "Two named missing inputs" section and the
`ai_group_leader_key` follow-up row above are superseded by that packet, which also loaded
`CloseAttack_CollectDist` (shipped 3000, not the image's 5000) with the rest of its block.
