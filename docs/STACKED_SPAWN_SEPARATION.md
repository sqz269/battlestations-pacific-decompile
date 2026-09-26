# What separates a squadron spawned at one point (packet cc9_stacked_spawn_separation)

2026-09-25. Ghidra read-only. Names are hypotheses. There are three reads, following
docs/SQUADRON_SPAWN_SEATS.md. With the host's station teleport off, two sets stay exactly coincident
for at least 20 s: the eight task-less Zero pairs, and two carrier flights (Lexington sqn03 and
Yorktown sqn04).

## 1. Vehicle avoidance does not separate coincident aircraft, in the image or the host

`007DF4F0`'s aircraft arm tests each listed candidate at `007DFAB6` and keeps it only while closing:
`dot(position in own frame, relative velocity in own frame) < 0`.
- Two members at one point with one velocity have both vectors at 0. The dot is 0, and the candidate
  is skipped before any band.
- The list itself (`007E11D0`: kinds 6 and 0Fh within 1080 m) does include squadron mates.
- So neither a fixed push nor the slot vector nor a random draw comes from this arm. The host's
  `vehicle_avoidance_007df4f0` matches at the same test.
- Coincident twins in the image separate only if something gives them different controls: a task
  and its follow state, or different random draws.

## 2. The Zeros are ordered in the image, and the host drops the order

This installation's `scripts/missions/usn/usn_19_coralus.lua` spawns each bomber wave with
`SpawnNew`: a Val or Kate group with `WingCount` 4, and an `A6M Zero` group with `WingCount` 2.
Its callback `luaBombersSpawnedLex(unit1, unit2)` does the following:
- `PilotSetTarget(unit1, Mission.Lex)` for the bombers;
- when `Mission.Difficulty == 1 or 2`, for the Zeros: `EntityTurnToEntity(unit2, Lex)`,
  `UnitSetFireStance(unit2, 2)` and **`PilotMoveToRange(unit2, Mission.Lex)`**.

The host runs E2 at effective difficulty 1 (`MissionStart::set_effective_difficulty game+6ACh=1`). Both
natives log `UNIMPLEMENTED`:
- `MissionLuaNative::PilotMoveToRange [008a4590]`;
- `MissionLuaNative::UnitSetFireStance [008a6490]`.

So the Zeros get no order, and their AI group logs `command=NONCONTROL`.

What the image does with the order:
- **The native.** `008A4590` issues command class `00E08F68` (`moveto`) through `0077D600`
  `BSP_Entity_IssueCommand` (`008A471C`-`008A472A`). The descriptor's `+14h` is the optional range
  (`008A4708`). The rule is already reconstructed as `pilot_move_to_range_008a4590`
  (`src/pilot_order_bindings.cpp`), but no host wires it.
- **The task.** `0099A170` `BSP_Bot_InstallCommandTask` compares the class with `00E08F68` at
  `0099A223`, with no target precondition, and calls the factory **`009C3BE0`** (`0099A236`).
- **The factory.** `009C3BE0` allocates **0x550 bytes** and constructs them with **`009C3000`**
  `(bot, 009BE310(target))`. The constructor:
  - calls `BSP_BotTask_ConstructBase` (`0099C6F0`) with kind **7**;
  - builds an approach sub-object at `+3F8h` (`009C2DF0`, target in);
  - sets the task vtable **`00D20B68`**, the approach vtable `00D20B58` and `+468h` = `00D20B54`;
  - sets `+548h` = 1.0 and `+54Ch` = -U(0, 1).
- **The start state.** It is chosen through `007B8AD0`, the unit's `+9D8h` leader test
  (`009C3097`-`009C30BF`):
  - a leader enters `+52Ch` when `task+454h` is set, otherwise `+47Ch`;
  - a wingman enters `+494h`.
  The state is stored at `+310h` and its `vtable[4]` enter is called. `BSP_BotApproach_BindToTask`
  (`009F9980`) follows.

So in the image each Zero wingman starts its moveto task in a separate state from its leader, the
follow-type state at `+494h`. That state flies it to its formation station, which is what separates
the pair.

## 3. The carrier flights are task-less until their first order, too

Lexington sqn03 is created by the air-ops route at 30.0 s, in the air at 150 m beside the carrier
(`air ops squadron: ... at (-12629.4 150.0 -12694.3)`). Its formation indices are assigned at 30.05
s. It holds no task until the AI planner's order at 116.8 s (`none -> moveto`). Yorktown sqn04 is the
same.
- No task means no follow state, so the wingmen fly the same reseeded plan as their leader and stay
  coincident: pairwise 0.0 m at 20 s and at 40 s.
- What the image gives an air-ops squadron between launch and its first order is not read here.

## 4. Verdict, and the scope question

- **No single term separates coincident members.** The image separates them through the task each
  wingman runs: for the Zeros, the kind-7 moveto task the script orders.
- **Binding it is a new task for the units host, not one switch:**
  - wire `PilotMoveToRange` and `UnitSetFireStance` into the mission host, the moveto command into
    `0099A170`'s moveto arm, and a kind-7 task in the units host;
  - read and bind the task's three states (`+47Ch`, `+494h`, `+52Ch`), its tick (`00D20B68` slots)
    and its transitions;
  - the state bodies may reuse the host's existing moveto (`009C18C0`) and follow (`009C1FD0`) ticks,
    which the torpedo and dive-bomb tasks already share.
- **Proposed next packet, `cc9_pilot_moveto_task`:** read `009C3000`'s states and the kind-7 tick,
  bind the task and the two natives under one switch, and pair with the teleport OFF. The carrier
  flights' pre-order behaviour is a separate read.
- **Nothing is bound in this packet**, and `kPlaneFormationPlacementEnabled` stays ON.
