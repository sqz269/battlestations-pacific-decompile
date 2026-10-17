# CC13 moveto1: the kind-7 moveto task for a structure target

Packet `cc13-moveto1` (cc9 grant 2026-10-17: `009C3000`, the `0099A170` moveto arm, `007EEC50`).
Code: `include/bsp/cc13_moveto1.hpp`, `src/cc13_moveto1.cpp`, namespace `bsp::cc13_moveto1`.
The code is build-tested only. It is not wired, and no game was run. Every name is a hypothesis.
Every claim below comes from the Ghidra listing and the disk bytes of `/battlestationspacific.exe`.

## The question

A squadron ordered onto a building installs a kind-7 moveto with range 0 and freezes in the host.
This doc covers what the image does:

- where the range comes from;
- whether the image ever produces range 0 for a building;
- what ends the moveto in the image.

## The answer

1. **The image does produce range 0 for a building.** `007F15F0` BSP_PlaneSquadron_IssueInitialAttackCommand
   is the spawn-time initial target.
   - It calls `007EEC50(target, 1, 1)`.
   - When that returns null, it issues moveto `00E08F68` through `0071ECF0`.
   - The descriptor it sends is {kind = (target != 0), id = target `+174h`, object = target,
     point = `00F87574` (zero), `+14h` = 0}.
   - The other two callers do not fall back to moveto:
     - `007F1940`, at `007F1B2D`, issues nothing on null.
     - `008A4C90` PilotSetTarget calls `007EEC50` twice with the same arguments (`008A4E6B`,
       `008A4E99`). On null it jumps to `008A4EB1` and issues nothing.
2. **Range 0 is harmless in the image.** The image reads the range in only one place: the circle
   radius r = max(approach `+6Ch`, TurnCircleRadius `class+268h`) (`009C27A6`).
   - The activation `009C3650` zeroes approach `+6Ch` (task `+464h`) at `009C36AE`/`009C36B7`.
   - The approach update copies the director's `+6Ch` into it while the director's class is moveto
     (`009C35A1`-`009C35CB`, every update, not only on refresh ticks).
   - No constructor writes `+6Ch`: `009F9CE0`, `009C1C30`, `009C2DF0` and `009C3000` were all read.
   - **The arrival test does not read the range.** `009C35D4`-`009C3636` arrive in two cases:
     - the planar distance `+64h` < ClosingDist `tuning+370h` × `+24h`;
     - for the leader, the `+60h` dwell drops below 0 inside the band [inner, inner + TurnCircleRadius).
3. **The image ends the moveto at arrival.** Once `+5Ch` is set, approach `vtable[8]` `009C3100`
   runs on every update.
   - It calls task `vtable[40h]` `009C31B0`.
   - On a nonzero answer it calls `0071E430(block, 00E08F68, 0)`, EndCommand.
   - At the next 0.09 s think, `0099A4C0` retires the lone head task, because `vtable[40h]` is no
     longer 1. Then `0099A170` installs the task for the next command.
   - A building target ends like any other moveto target, because `009C31B0` compares the
     approach's own target at `+44h` with the command's target.
4. **attackmove with no attack class.** If the director holds attackmove, `0099A170` calls
   `007EEC50` (`0099A1E2`). A null answer takes `009C3C40` (`0099A219`) and builds the attackmove
   variant `009C3B10`.
   - Its keep test is the same `009C31B0`, which answers 0 because the class is attackmove, not
     moveto.
   - So at every think, `0099A4C0` retires the variant. It then tail-jumps (`0099A5EB`) to
     `0099A170`, which re-runs `007EEC50` and builds a fresh variant. `0099AEB3` then runs the
     activation `009C3650` on the new task.
   - The arrival never ends the attackmove, because `009C3100` sees 0.
   - Each think's task starts in moveto for a leader, or follow for a wingman. A leader within the
     inner radius switches to circle (r = TurnCircleRadius) on its first update. `+54Ch` starts
     negative, so `009C3310` picks a state at once.
   - The squadron takes the first attack class `007EEC50` returns.
5. **attackmove with a null target** installs nothing. `EDI` stays `00E08F78` (`0099A1D2`), no arm
   matches, and the routine exits at `0099A495`.

**Shared RNG `00BD2F10`, ECX = 1.** Every kind-7 construction draws in this order:
- `009C1C30`: U(0, `tuning+540h`), then U(0, 0.5);
- `009C2980` follow: -U(0, 0.6);
- `009C25D0` circle: U(0, 1);
- `009C3000`: `+54Ch` = -U(0, 1.0) at `009C3084`;
- the activation `009C3650`: `+460h` = U(`tuning+368h`, `+36Ch`) at `009C367C`, and `+424h` =
  U(0, `tuning+364h`) at `009C36A3`.

In case 4, that is seven draws per plane per 0.09 s think for as long as the attackmove stays
unresolved. It happens in a normal mission wherever an attackmove lands on a target that
`007EEC50` cannot classify.

## When `007EEC50` answers null for a building

- **Side gate** (`007EEC70`-`007EEC92`):
  - A CommandBuilding (1Ch) goes on when its side differs from the unit's, so a side-2 (neutral)
    command building can be attacked.
  - Any other target needs a different side and a side other than 2.
- **Classifiers.** `00922DC0` = `00922C80` with DL = 1 (docs/ATTACK_CAPABILITY_INPUTS.md):
  - It answers true for airfield 45h, shipyard 46h, command building 1Ch, and members of the
    `008DDF90` set.
  - A plain LandFort 1Bh counts as surface only when `class+178h` (FakedType) == 1Bh or when it is
    in that set.
  - `00922B10` (airborne) is false for every building.
- **Null for a building therefore means one of these:**
  - every surface-gated arm fails (a LandFort that is not surface);
  - strafe fails (`+C24h` PilotFires clear on the slot-0 plane, or `0047B850` true);
  - the bomber has no matching ordnance.
- **Moveto for any target.** `007EEC50` returns moveto whenever `[unit+3D0h]` is 0, meaning there
  is no plane in slot 0 (`007EEC54`).

## Functions

| address | C++ | ABI | status |
| --- | --- | --- | --- |
| `009C2DF0` | `moveto_approach_block_construct_009c2df0` | thiscall(approach)(bot, target), RET 8 | reconstructed, build-tested |
| `009C3000` | `moveto_task_construct_009c3000` | thiscall(task)(bot, target), RET 8 | reconstructed, build-tested |
| `009C3B10` | `moveto_task_attackmove_construct_009c3b10` | thiscall(task)(bot, target), RET 8 | reconstructed, build-tested |
| `009BE310` | `moveto_target_create_from_command_009be310` | ECX = descriptor, RET | reconstructed, build-tested |
| `009C3BE0` | `make_moveto_009c3be0` | ECX = bot, EDX = descriptor, RET | reconstructed, build-tested |
| `009C3C40` | `make_moveto_for_attackmove_009c3c40` | ECX = bot, EDX = descriptor, RET | reconstructed, build-tested |
| `009C33A0` | `moveto_task_point_009c33a0` | thiscall(task)(out), RET 4; no Ghidra function | reconstructed from bytes, build-tested |
| `009C31B0` | `moveto_task_command_still_current_009c31b0` | thiscall(task), RET, 0/1/2 | reconstructed, build-tested |
| `007EEC00` | `unit_attack_command_applies_with_flag_007eec00` | thiscall(unit)(class, target, surface, out), RET 10h | reconstructed, build-tested |
| `007EEC50` | `unit_choose_attack_command_007eec50` | thiscall(unit)(target, prefer, guns), RET 0Ch | reconstructed, build-tested |
| `0099A170` | (existing) `install_native_pilot_bot_command_task_0099a170` | fastcall(ECX = bot) | existing; moveto arms re-verified, ledger only |

Notes per function:
- **`009C2DF0`** calls `009C1C30(approach, [bot+50h], target)`, then the registry `00411E20`
  (`+70h`).
  - It then stores the vtables `00D20B48`/`00D20B44`.
  - The inline moveto state at `+84h` is stored in the order `+88h` = approach, `+90h`, `+94h`,
    `+98h`, `+8Ch` = 0, then vtable `00D20A80`.
  - Then follow `009C2980(+9Ch)(approach, 100.0f [00CE3D08])` and circle `009C25D0(+134h)(approach)`.
  - It registers "moveto (moveto)" `00D20998`, "follow (moveto)" `00D20988` and "circle (moveto)"
    `00D20978`, in that order.
- **`009C3000`** runs these steps in order:
  1. `0099C6F0(bot, 7)`, then `009C2DF0(task+3F8h, bot, target)`.
  2. The vtables `00D20B68`/`00D20B58`/`+468h` `00D20B54`, then `+548h` = 1.0f (`00D7A24C`).
  3. `+54Ch` = -U(0, 1), followed by the start state: a leader takes circle `+52Ch` if `+454h` is
     set, otherwise moveto `+47Ch`; a non-leader takes follow `+494h`. The state is stored at
     `+310h` and entered through `vtable[4]`.
  4. `009F9980(approach, task)`, then `_free(target)`. That frees the `009BE310` object without a
     destructor call.
  - It reads no range.
- **`009C3B10`** runs `009C3000`, stores `00D20BE0`/`00D20BD4`/`00D20BD0`, and binds a second time
  through `009F9980`.
  - The vtable bytes show that `00D20BE0` differs from `00D20B68` only in `+0` (`009C3BC0`) and
    `+3Ch` (`009C3B80`).
  - The two approach vtables differ only in `+4` (`009C3B90` and `009C3190`, the
    `this-3F8h` destructor thunks).
- **`009BE310`**: a descriptor with kind != 0 builds the 14h entity target `009BE0F0(new,
  00521EA0(desc))`.
  - Otherwise it builds the 10h point target: vtable `00D209C8`, with the point taken from
    descriptor `+8` if `+1` is set, else from `00F87574`.
  - A failed allocation returns 0.
- **`009C31B0`** answers:
  - 2 when `[task+404h]` (the squadron) is null;
  - 0 when the `vtable[114h]` block is null, or the class is not moveto;
  - for an entity target, 1 if `[task+43Ch]` is non-null and equal to the resolved target;
  - for a point target, 1 if `10000.0f` (`00CE3D64`) > dz² + dx².
  - The point target compares the task point (`vtable[0Ch]`) with the descriptor point
    (`006F7DD0`), all in float.
  - NaN takes the 0 arm (`FCOMIP`/`JA`).
- **`007EEC00`**: flag = (`unit+369h` && byte `00E17BF2`) ? 1 : 0, then `007EE8F0(class, target,
  surface, flag)`. It writes `*out` only on success.
- **`007EEC50`**:
  1. Returns moveto if `[unit+3D0h]` == 0, and 0 if the target is null.
  2. Applies the side gate above.
  3. Reads surface (`00922DC0`), then airborne (`00922B10`).
  4. If the target is not airborne, tries levelbomb, dropkamikaze, divebomb, torpedo, rocket,
     kamikaze and depthcharge in that order.
  5. If prefer is set, returns the ordnance class.
  6. Otherwise tries dogfight, then strafe, and returns the result only if guns are allowed.
  - Simplification: the surface argument is EBP, whose upper three bytes are stale (the unit
    pointer pushed at `007EEC50`). It is passed here as the low byte, which `007EE8F0` tests as a
    char.

## Host comparison (the host files are read-only for this packet)

- **The chooser.** `bsp::attack_command_choose` (`src/attack_commands.cpp`) has the image's order,
  side gate and ordnance/guns structure. Two differences:
  - **(minor)** Its `target_present` folds in the target's `+5Dh`. `007EEC50` and `007EE8F0` do
    not test `+5Dh`, so for a `+5Dh` target the image still runs the arms that do not need a
    classifier.
  - **(possible REAL)** `GameScriptOrdersHost::choose_attack_class_007eec50` stands
    `unit_is_kind_of(unit, 0Fh)` in for `[unit+3D0h] != 0`. If the ordered unit is a squadron
    object (18h) rather than a plane, the host returns moveto, while the image (with a plane in
    slot 0) runs the class chain.
- **The install.** `run_moveto_task_install_009c3000` (`src/game_hosts_units.cpp`) installs only when
  `attack_command_class` == moveto, and only once.
  - The image's attackmove variant (case 4: re-chosen and rebuilt at every think, seven draws per
    plane) is not modelled.
  - The host's `+54Ch` draw and start state match `009C3000`.
- **The arrival.** The host's `009C3100` (`moveto_arrival_end_command_009c3100`) tests
  `attack_command_class == moveto`.
  - The image's `009C31B0` also requires the command's target to be the approach's own target, or
    the point to lie within 100 m.
  - The image returns 2 (and still ends the command) when there is no squadron. The host returns
    early.
- **The range.** The host also reads `moveto_range` only for the circle radius
  (`game_hosts_units.cpp` around 33733). So in the host, too, range 0 cannot by itself stop a
  plane. A freeze has to come from the arrival or retirement path, or from the case-4 path, which
  the host does not model.

## Not done

- `009C3100` is hosted (`moveto_arrival_end_command_009c3100`); this packet records its reading in
  the ledger only.
- `009C3570`, `009C3310` and `009C3650` are hosted in `game_hosts_units.cpp`; they were read for the
  answer above and not reconstructed.
- The meaning of `vt[5Ch](2)` at the `0099A170` attack arms is still open. No VehicleClass
  IsKindOf row (for example `00963D80`, `00953680`) accepts 2, so it is an entity-level kind.
