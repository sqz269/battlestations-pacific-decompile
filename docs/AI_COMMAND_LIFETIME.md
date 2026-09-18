# Why one attack order lands, and what actually multiplies groups

Addresses: `009FE080` BSP_Entity_IsGroupableCombatant, `007EDA90` (its squadron arm),
`00A2C5A0` BSP_AiGroup_HasGroupableCombatant, `00A2E260` (the split), `00A2CBD0` (the attack
order), `00A13340` (the command factory, still unread), `00A10890` / `00A109B0` / `00A10AE0`
(the three attack-command constructors).

Packet `cc8_ai_command_lifetime`, the follow-up `cc8_ai_coordinator_tick` named.

## The premise was wrong

That packet reported "one attack order lands per mission and never a second, because the
already-on-target test returns early and nothing clears the command without an AI command
object". The second half is wrong, and this packet's own doc said so without noticing:
`00A2CBD0` **deletes the old command through its vtable slot 0 with flag 1 and installs the new
one**. A different target lands a different order. Nothing has to clear anything first, and the
missing command factory `00A13340` is not what was blocking it.

Three things actually cap the order count, in the order they bite:

1. **The mode planner orders its first owned group only.** `00A26510` and `00A265F0`, read in
   full in `docs/AI_PLANNERS.md`, call `00A1CB80(planner)(firstOwnedGroup, 1.0f, ...)`. A
   planner holding two groups orders one of them. This is the hard cap: at most one order per
   planner per think, whatever the target set looks like.
2. **The target set had one element.** `00A2CBD0`'s second test skips when the chosen target is
   the one the group is already on, and with a single enemy group the scoring loop returns that
   same group every think. This is the one the coordinator host caused, and it is fixed below.
3. **Not the command lifetime.** See above.

## `009FE080`, the predicate that does two jobs

`__thiscall bool(ECX = entity)`, body `009FE080`-`009FE0AA`, read in full.

```
009fe085  MOV EDX,[EAX+5Ch]          ; vtable[+5Ch], IsKindOf
009fe088  PUSH 18h                   ; plane squadron
009fe08a  CALL EDX
009fe08e  MOV ECX,ESI
009fe090  JE  009fe0a0               ; not a squadron -> the ship tail
009fe092  CALL 007EDA90
009fe097  NEG AL / SBB EAX,EAX / ADD EAX,1    ; logical NOT
009fe09f  RET
009fe0a0  MOV EDX,[EAX+5Ch]
009fe0a5  PUSH 6                     ; ship base
009fe0a7  CALL EDX
009fe0aa  RET
```

`007EDA90` is true only when `[squadron+3D0h]` is non-null (`007EDA99`), `IsKindOf(17h)` holds on
that object (`007EDAA0`, `007EDAA8`), and the byte at `object+C24h` is clear (`007EDAAA`,
`007EDAB1`). A squadron is groupable exactly when that is **false**.

**It does not admit the plane base `0Fh`.** An individual aircraft is never a groupable
combatant; only a ship base, or a squadron whose carrier link fails `007EDA90`.

`00A2C5A0` walks the member list at `group+563Ch` and calls this same predicate on each member's
`+8h` (`00A2C5CE`), so the group-level combatant test and the split subset are **one rule**, not
two. `docs/AI_GROUP_THINK.md` lists them as separate host methods, which is right for the
interface and misleading about the rule.

## The rules

`include/bsp/ai_command_lifetime.hpp` and `src/ai_command_lifetime.cpp`, pure, no host:

* `ai_squadron_excluded_007eda90`, `ai_entity_is_groupable_combatant_009fe080`,
  `ai_group_has_groupable_combatant_00a2c5a0`.
* `ai_group_split_runs_00a2e260`: the subset is split out only when it is non-empty **and
  strictly smaller** than the population (`00A2E334` JBE, `00A2E342` JNC, both unsigned). A group
  whose members all pass, or none of which pass, is left alone.
* `ai_attack_order_outcome_00a2cbd0` and `ai_attack_arm_00a2cbd0`: the two head tests and the
  cautious-arm draw. A higher aggressive ratio makes `CAUTIOUSATTACK` less likely.

No name here collides with `bsp/ai_group_think.hpp` or `bsp/ai_planners.hpp`; `AiCommandType` and
`ai_command_is_type` stay where they are.

## The host correction

`src/game_hosts_ai.cpp`, this packet's own file. Two methods were wrong.

`split_detached_members` was a no-op on the reasoning that "nothing in this process detaches a
member". That was a misreading: `00A2E260` does not split on detachment, it splits on
`009FE080`, and it is the **group multiplier**. Leaving it a no-op is why the previous packet
measured one group per team and therefore one target for the planner to pick.

`group_has_groupable_combatant` admitted the plane base `0Fh`, which `009FE080` does not.

### Measured on IJN01, against the previous packet's run

| | coordinator packet | with the split |
| --- | --- | --- |
| groups created | 2 | 4 |
| splits taken | 0 | 2 |
| members added | 141 | 190 |
| planner claims | 1 | 2 |
| commands issued | 79 | 34 |
| attack orders | 1 | 1 |
| plane yaw_plans | 5904 | 5904 |
| plane heading_change | 8.881 rad | 8.881 rad |
| plane distance moved | 1270266.90 m | 1270266.90 m |
| pilot attack ordered | 4 | 4 |
| gunnery hull / deaths / damage | 4 / 2 / 500.0 | 4 / 2 / 500.0 |

The groups are now `team=0 members=34` (commanded), `team=0 members=45`, `team=1 members=29`,
`team=1 members=2`. The commanded group is 30 `artillery` tokens and 4 `dogfight`, against 75
and 4 before.

**The 45 orders that went away were inert.** Plane motion, the pilot attack table and the
gunnery outcome are identical to the digit. The previous packet was ordering 45 units that did
nothing with the order, and the split removes them from the group without changing any
behaviour. That is the right kind of null result: the grouping is now faithful and nothing that
was working stopped working.

**The order count did not move**, which isolates cause 1 above. Two claims produced two owned
groups but still one order, because the mode planner orders its first owned group only.

## Uncertainty that has to travel with this

The squadron arm's three inputs have **no producer in this process**: nothing builds the
`+3D0h` carrier link, its `IsKindOf(17h)` answer or the `+C24h` byte. `combatant_facts` leaves
all three false, so `ai_squadron_excluded_007eda90` answers false and the host admits **every**
squadron as groupable. Four of IJN01's units take that path, the four that draw a `dogfight`
token. Thirty of the thirty-four are ship bases and do not depend on it.

If `007EDA90`'s inputs ever get a producer and any of those four squadrons has a live carrier
link with a clear `+C24h`, that squadron leaves the combatant set and the group splits
differently. The 30 ship bases are unaffected.

## no_ghidra_function

none. `009FE080`, `007EDA90`, `00A2C5A0`, `00A2E260` and `00A2CBD0` all have Ghidra functions.
`00A13340` has one and is still unread; this packet did not need it.

## Contract

The fixed-step hook in `src/game_hosts_units.cpp` is still not committed. It was applied locally
to take the measurement above and reverted. `docs/AI_COORDINATOR_TICK.md` carries it verbatim.

## Corrections

Appended to `docs/AI_PLANNERS.md` and to `docs/AI_COORDINATOR_TICK.md`, never rewriting them.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_planner_owned_group_walk` | `00A1CB80`, `00A26510`, `00A265F0` | whether a planner that owns several groups orders only the first natively too, or whether `00A179E0`'s engagement pass orders the rest. This is now the only cap left on the order count |
| `ai_squadron_carrier_link` | `007EDA90`, `+3D0h`, `+C24h` | the squadron's carrier link, so the four squadron members stop being admitted by default |
| `ai_command_object` | `00A13340`, `00A2C8D0` | still worth doing, but for the merge predicate at `vtable[+14h]`, not for the order count |

## Correction from docs/PLANE_SQUADRON_ENTITY.md

Appended by packet `cc8_plane_squadron_entity`. The text above is left as written; this section
records what a later read of the listing settled. **The rule this document states is correct; the
naming of `007EDA90`'s three inputs is not.**

`[squadron+3D0h]` is **not a carrier link**. It is `members[0]` of the squadron's five-slot member
plane array, which is to say the **flight leader**. Three witnesses:

| Witness | Site | What it shows |
| --- | --- | --- |
| the constructor | `007F2DA3..007F2DBB` | zeroes exactly five dwords, `+3D0h`, `+3D4h`, `+3D8h`, `+3DCh`, `+3E0h` |
| the spawn tail | `007F4B55`, `007F4B60` | `MOV [ESI+EDI*4+3D0h],EAX` with `EAX` the plane just created, then `+3CCh += 1` |
| the leader rotation | `007ED618`, `007ED621` | `MOV ESI,[ECX+EDX*4+3D0h]` indexed by `EDX`, bounded by `CMP [ECX+3CCh],EDX` |

A carrier link cannot be indexed by a member counter, and `007ED610`
`BSP_PlaneSquadron_PromoteFlightLeader` exists to move a chosen member into slot `0`.

So the three reads at `007EDAA0`, `007EDAA8` and `007EDAAA` are all taken on the flight leader:

* `IsKindOf(17h)` is `MPlaneKamikaze` (`docs/ENTITY_CLASS_IDS.md` row 17, class test `009534A0`,
  chain `{17h, 0Fh, 05, 04, 02, 01, 0}`), not a carrier class.
* `leader+C24h` is the authored `PilotFires` byte (`docs/ATTACK_GATE_TAILS.md` section "Gate 2":
  two write sites, both in `FUN_007CD930`, whose single caller is `007D673E` in
  `BSP_Plane_ReadPropertyBag`, so it is a load-time constant).

`007EDA90` is therefore the same shape as the `kamikaze` attack gate `00604A50`, applied to a
flight's leader: **"is this a kamikaze flight whose pilot does not fire"**. `009FE080` negates it,
so a plane squadron is a groupable combatant unless it is an uncommitted kamikaze flight.

The three field names in `include/bsp/ai_command_lifetime.hpp` were renamed accordingly by packet
`cc8_ai_squadron_served`: `squadron_has_carrier` is now `squadron_has_flight_leader`,
`squadron_carrier_is_kind_17` is now `leader_is_kamikaze_kind_17`, and
`squadron_carrier_flag_0c24` is now `leader_pilot_fires_0c24`. The follow-up row
`ai_squadron_carrier_link` in the table above is superseded: there is no carrier link to produce,
and the input that has no producer in this process is the authored `PilotFires` byte.
