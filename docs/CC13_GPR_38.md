# CC13 GPR 38: segment 38 reference models, and the USNOS Betty idle path

Packet `cc13-gpr38` (Claude, cc13 lineage). Segment 38 is `0071BA20-00722FB0`: the weapon
director's command slots, the path follower and user-path messages, the command-target
descriptors and the controller snapshot messages. This pass was approved by cc9 on these terms:
- new `*_cc13` files only (`src/gameplay_seg38_cc13.cpp`, `include/bsp/gameplay_seg38_cc13.hpp`);
- ledger records;
- no host edits, no wiring, no game runs.

Evidence levels follow AGENTS.md:
- every model here is **reconstructed** from the disk bytes and the Ghidra pseudocode, and **build-tested**;
- nothing is wired or game-validated;
- the USNOS answer (section 1) is static analysis plus existing cc9 logs. No run was made.

## 1. USNOS: what the image does with a Betty squadron after the Ohka drop

**Question (cc9).** In USNOS (`COTP-USN/us_osumi.lua`), after the Ohka drop the Betties fly
straight off the map and never come back. What does the image do with a plane whose squadron
director goes idle after a task ends?

**The script.** `luaFuntime` (`us_osumi.lua:1795`, this installation, mtime 2024-10-29):
- spawns `Betty1`..`Betty6` (class 32 `BettyOhka`, four planes each);
- adds them to `Mission.FinalTrgs`;
- calls `PilotSetTarget(unit, random TilmanGrp ship)`.

The primary objective at `:1181` needs `luaRemoveDeadsFromTable(Mission.FinalTrgs)` to be empty, so
every Betty squadron has to die or leave.

### 1.1 Short answer

- **The image sends a spent squadron to return-to-base.** It does not use a follow or user-path
  default.
  1. The squadron's spent-ordnance latch fires (`009F8160` / `009F7C90`).
  2. Every member's drop-kamikaze task then answers break-off (`009AE1B0`).
  3. `EndCommand` raises the `dropkamikaze` command to stage 2.
  4. The squadron's own step `0084E010` calls `007F16D0` and issues its answer: land at site, land
     at home, or `retreat`.
- **If `007F16D0` answers null,** the squadron is ordered to `moveto` itself (`0084E595-0084E5C6`).
- **The out-of-map Kill(4)** applies only to a squadron whose command is `retreat`.
- **The host never reaches step 2.** It installs a second, phantom torpedo task on every Betty. Its
  version of the 009F7C90 end test reads that task's break-off and never asks the drop-kamikaze
  task.

### 1.2 Which director runs for a squadron

| director vtable | slot +7Ch (the step) | evidence |
| --- | --- | --- |
| `00D09F58` (ship, derived) | `00836920` | dword `00D09FD4` = `20 69 83 00` |
| `00D0BD98` (plane squadron command block) | `0084E010` | dword `00D0BE14` = `10 E0 84 00` |
| `00D09EC0` (base) | `00835F60` | dword `00D09F3C` |
| `00CFDA40` | `00BF698E` | dword `00CFDABC` |

The cruise / stop / follow idle tail at `00836E02-00836E92` (docs/AI_RETASK.md,
docs/COMMAND_COMPLETION.md) belongs to the ship director only, and a squadron never runs it.

The pilot bot could not use cruise or follow anyway. `0099A170`'s class chain ends with
`retreat -> MakeRetreat`, then `stop -> MakeStop`, else `return`, so `cruise` (`00E08F70`) and
`follow` (`00E08F60`) build no bot task. None of this segment's start-mode, follow-mode or user-path
defaults takes part.

### 1.3 The image path, step by step

1. **The task stays while the command stands.**
   - `0099A4C0` retires a lone head task only when its vtable `+40h` is not 1.
   - For drop-kamikaze that slot is `009AE140`. It returns 1 while the director's current class
     (`0071BE40`) is `00E08F30` or `00E08F78` and the resolved target is still `task+440h`.
   - Vtable `+34h` is `0099B700` (`XOR AL,AL`) and `+38h` is `0099B710` (`MOV AL,1`), so neither of
     them retires it.
2. **In goAway the leader orbits; it does not leave.**
   - `009AD480` steers by `009FD570` with standoff = startDist and range = targetDist.
   - The solver's offset ramps 0 -> pi over e = range - standoff in -200 .. +300 m, which puts an
     equilibrium tangent near e = +50 m (plane_fly_to_solver.cpp:145-177).
   - Within 500 m of a world edge the solver also pulls toward the map centre (`009FDD62`).
3. **The latch.** The squadron's +3Ch latch has two writers, both gated on ctl+369h clear or
   `[00E17BF2]` clear:
   - `009F8160`'s ordnance arm sets it when no member holds a rack round (cc9's model,
     docs/SQUADRON_LAND_TASK.md 5eb);
   - `009F7C90`'s own arm sets it when the `007ED920` flag at +28h falls, outside the
     `IsKindOf(13h)` classes.
4. **The end test (`009F7C90` -> `007EE6B0`).**
   - `007EE6B0` calls each member's head task vtable `[1Ch]`.
   - For drop-kamikaze that is `009AE1B0`: break-off allowed (`0099C230`), and then any of
     - no target;
     - target `+5Dh` set;
     - reload off and (not an attack state, or no ordnance).
   - After the drop every member answers true, in goAway, done or formation.
   - Then `EndCommand(current, 1)` raises the command to stage 2.
5. **The squadron step's tail (`0084E010`, `0084E4B1-0084E5C6`, bytes read with disasm-raw).**
   - `bVar4` = (stage 2, queue < 2, latch +38h->+3Ch clear, command not retreat / returntobase /
     land), or an empty queue with a leader (`0084E010` head).
   - `bVar5` = the same with the latch set, or override stage 2 with the latch.
   - With `bVar5`, or with `bVar4` / no command / category not 1-2 when ctl+369h is clear:
     - if ctl+369h is clear or `[00E17BF2]` is clear, and ctl+368h is clear, it calls `007F16D0`
       (`0084E552`);
     - a non-null answer gives `0071D880` (clear), `0071ECF0` (issue) and +228h = 1.
   - Otherwise, when `bVar4` holds and the queue count is <= (stage == 2): `0071ECF0(00E08F68,
     00465080(squadron, 0.0))`, a `moveto` on the squadron entity itself.
6. **The map exit** (`007C6C30` -> `007EE410` -> `007CA790` -> `007F31A0` Kill(4)) is gated by
   `007EE2E0`: the squadron's command must be `retreat`. Only a `retreat` answer from `007F16D0`
   leads there.

### 1.4 The host divergence (cc9's files; not edited here)

- **A phantom torpedo task.**
  - `src/game_hosts_units.cpp:26130-26161` (`run_torpedo_task_arm_009d4850`) installs the
    torpedo task on any plane with kind-2Bh ordnance and a command target, whatever its
    `attack_command_class`.
  - It runs beside the drop-kamikaze arm (`:32183-32207`). BettyOhka's racks are `[2Bh 2Fh]`, so
    every Betty carries both tasks.
  - `local/g39_bb0_os.log` (cc9-gunnery36, 2026-10-13) shows it on Betty1:
    - `torpedo reset engage Betty1` at line 293765;
    - the `torpedo Betty1 glide census` rows;
    - `run-in at attackrun entry`.
  - The image's `0099A170` builds exactly one task from the director's class: `00E08F30 ->
    MakeDropKamikaze`. `MakeTorpedo` is built only for `00E08F18`.
- **The end test ignores drop-kamikaze.**
  - The host's 009F7C90 loop (`:15229-15238`) reads `torpedo_breakoff_now`, the dive-bomb
    break-off and the level-bomb break-off only. A Betty is answered by its phantom torpedo task.
  - `bsp::drop_kamikaze_should_break_off_009ae1b0` (src/kamikaze_task_cc13.cpp) is never asked
    there.
  - Measured in the same log, Betty1-4 latch and never reach B5:
    - Betty2 at 1602.04 s, Betty3 at 1616.25 s, Betty4 at 1631.27 s, Betty1 at 1743.88 s
      (`spent ordnance ... latched (command 00E08F30)`);
    - the summary reads `latches=35 ends=31 issues=31`.
  - The leaders then cycle goAway <-> moveto at 3.8-4.1 km from their targets to the end of the run.
- **No moveto-self fallback.** The host does not run `0084E595-0084E5C6`, so a null `007F16D0`
  answer leaves the squadron with no order.

### 1.5 Adoption note

All three steps sit behind one OFF switch:
1. Gate the torpedo arm on `attack_command_class == 00E08F18`
   (`src/game_hosts_units.cpp:26130`).
2. In the 009F7C90 loop (`:15229`), add `else if (u.drop_kamikaze_task_installed) all = all &&
   009AE1B0(u)`, which reuses the existing model.
3. Optionally, issue `0084E595`'s `moveto` on the squadron itself when `007F16D0` is null.

The inputs are the latch state, each member's dkz state, its ordnance and target, and reload
(`[00E17BF2]`, ctl+369h).

**RNG.**
- Steps 1 and 2 draw nothing themselves.
- Removing the phantom task removes its `009D0380` reset draws (stream 1).
- A B5 that resolves to `retreat` draws through the hosted `009C9D26` / `009C9BB0` / `009C91B0`
  sites.

**Prediction.**
- OFF: identical.
- ON:
  - the Betty rows lose their torpedo lines;
  - Betty1-4 (and Betty5/6 once their last holder drops) reach B5 within a step of the latch;
  - `007F16D0` resolves for them. On this map, Japanese squadrons resolve `land at site Airfield3`,
    9-11 km west (g39_bb0 lines 37065 and following). Class 32 may also answer retreat or null; I
    did not check which.

**Open.**
- A `retreat` ends in Kill(4) `exitzone`, which counts as dead for
  `luaRemoveDeadsFromTable(Mission.FinalTrgs)`.
- I have not traced whether a landed Betty counts as dead.
