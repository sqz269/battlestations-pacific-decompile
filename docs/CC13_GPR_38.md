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

### 1.6 The two follow-up answers (cc13-gpr38b)

These answers are static, with no run made.

- **What `007F16D0` answers for class 32: land at site, Airfield3 on this map.**
  - The null arm does not apply:
    - class 32's `MinWaterSpd` is 22.222221 (`vehicleclasses.lua:18874`, this installation, mtime
      2026-05-09);
    - the Betty head is kind 10h, not 17h. The `007F1709` kamikaze null arm catches only the Ohka
      spawns (`Betty*_kz*` resolve to null in g39_bb0).
  - Squadron `+369h` is the scene property `ReloadEnabled`:
    - `007F1FE0` writes it from the bag (`ReloadEnabled`, type 3) or from the spawn record +124h;
    - the constructor `007F2D09` defaults it to 1 (`C6 86 69 03 00 00 01`);
    - `us_osumi.scn`'s Betty rows (`PlaneSquadronGen`, template `Japan\AIR\Betty`) carry no
      `ReloadEnabled`, so it stays 1, and the land arms are tried.
  - `[00E17BF2]` stays 0, because `us_osumi.lua` never calls `SetDeviceReloadEnabled`. So the
    latch and B5 gates pass.
  - The home arm fails, because `HomeBase = ""`.
  - The site arm `006C0840` keeps same-party airfields with a live owner (a kind-10h head needs
    approach bit 20h) and takes the nearest by key. In g39_bb0, Japanese squadrons resolve
    `land at site Airfield3` (line 37065 and following), and Airfield3 is still a live Japanese home
    at 1644.08 s (line 365970).
  - The answer would be `retreat` (border zone, then Kill(4) `exitzone`) only if no Japanese
    airfield qualified.
  - Host note: `issue_return_to_base_007f16d0` pins `in.has_deck_369 = true`
    (`src/game_hosts_units.cpp:11837`). That is right for USNOS, but it should read
    `ReloadEnabled`.
- **A landed Betty counts as dead.**
  - A plane that lands is stock-returned and killed with cause 5, `landed` (docs/SHIP_AI_OPEN_ITEMS.md
    200.2). USNOS already shows three `KillReason=landed` at Airfield3.
  - The squadron entity, which is what `Mission.FinalTrgs` holds, is killed by `007F3970` when its
    member count reaches zero (`007F3A16` / `007F3A2A`, docs/ENTITY_DEAD_FLAG.md section 8). Its
    cause is the last plane's cause. `luaRemoveDeadsFromTable` (`commandhelpers.lua:1593`) reads
    only `.Dead`, so a fully landed squadron, or a mix of landed and shot-down planes, leaves the
    table.
  - Host caveat (cc9): the squadron-Dead switch does not model removal by landing (`007F3970` with
    argument 1).
## 2. Segment 38 reference models

Code: `include/bsp/gameplay_seg38_cc13.hpp`, `src/gameplay_seg38_cc13.cpp` (namespace
`bsp::gameplay_seg38_cc13`). These are semantic models taking values and host interfaces:
- engine queries (vtable answers, target resolution, the handle tables) are inputs;
- the bit cursor is the `BitSink` / `BitSource` interface, named after the native helpers;
- the session router's messages are returned as `RoutedDirectorMessage` records.

They are not native layouts and nothing is wired. Status of each: **reconstructed, build-tested**.

### 2.1 Adoption table

| model | address | host or stub it would replace | inputs | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- | --- |
| `on_entity_removed_0071ddb0` | `0071DDB0` | `release_observed_target_0071ddb0` (src/game_hosts_commands.cpp) answers one slot | per-slot "targets the removed entity", vt70/vt74 answers, `[GGame+5D4h]`, session mode | none | OFF: identical. ON: a removed target also clears every later queued slot whose class vt74 allows it (`0071D900`), on top of ending slot 0 |
| `retarget_to_member_or_point_0071edd0` | `0071EDD0` | the attack-target conversion inside the commands host (none separate) | the target, squadron member list and live bytes, positions | none | ON: a squadron target with two or more live members resolves to the **last** live member, because the distance gate is `d^2 < FLT_MAX` (`00D7A278` is `47EFFFFFE0000000`); a non-squadron entity becomes a fixed point |
| `controller_update_0071f290` | `0071F290` | the commands host's per-step director entry | unit live bytes, dt, slot and override presence, the two vt78 answers, session mode | none | ON: the `+40h` override-delay countdown, and a command ends at stage 2 when its vt78 validity test fails while `+44h` is clear |
| queue predicates (`0071BDE0`, `0071C0C0`, `0071D780`, `0071DEE0`, `0071DF70`, `0071DC80`, `0071D6D0`, `0071E200`) | | the host's per-unit command rows answer these implicitly | slot classes, categories, path facts | none | no change until a host keeps a ten-slot queue |
| messages (`0071C730`, `0071CDD0`, `0071D3E0`, `0071D900`, `0071D980`, `0071DAD0`/`0071DD30`/`0071E050`/`0071E150`, `0071E2E0`) | | the session messages are not routed by the single-player host | payload values | none | none |
| descriptor and snapshot codec (`00721070`..`007219C0`) | | none (network/save path) | descriptor fields | none | none |
| geometry (`0071CEA0`, `0071CF30`, `0071CE60`, `0071DB90`, `0071D600`, `00722CA0`) | | none | rect, points | none | none |

### 2.2 Findings worth knowing

- **`0071EDD0` picks the last live member, not the nearest.** The ledger name was
  `TargetStillValid`. The member loop overwrites its choice for every member that passes
  `d^2 < (double)FLT_MAX`, so only a NaN or infinite distance is rejected. A member index of 5 or
  more reads a null member (`+5Dh` at address 5Dh), so the image relies on at most five members.
- **`0071CF30` has an operand slip.** Its fourth "inside" test compares the b-axis coordinate
  with `-half_a` (`0071CFD0 FCOMPI ST(3)`) where the other three tests pair each axis with its own
  half extent. The model keeps the image's test.
- **`0071D600` takes the maximum of twenty floats** (`+1D0h..+21Ch`, four unrolled groups of five),
  not eight as its ledger name says. The seed is `-1e10f` (`00CE4ADC`).
- **`0071F570` drops only the last point.** It is `0071F3E0(count - 1)`, an erase-at-index, and
  only runs when there are two or more points. It does not clear the list.
- **`0071C1B0` writes the path of the last filled slot.** It indexes `+1A0h + first_empty*4`, so
  with an empty queue it writes through `director+1A0h`, which is not a path pointer.
- **`0071E4C0` with a null slot-0 path** calls `007B1350` with ECX = 10h (null + 10h). The model
  does not cover it (see section 3).
- **Defaults of the director path object** (`0071FB90`): followMode 1, startMode 5, `+2Ch` = -1,
  `+3Ch` = -1.0f.

### 2.3 Existing reconstructions checked and recorded, not duplicated

| address | existing model | check |
| --- | --- | --- |
| `00722400`, `007225F0` | `radix_sort_records16_00722210` (src/band_0067_cc13.cpp) with key index 1 / 2 | the decompiles differ from `00722210` only in the key dword (+4h / +8h) |

## 3. Records, status and what is still open

**Records.**
- 71 `ledger add-function` records with source `src/gameplay_seg38_cc13.cpp`, status
  `semantic_build_tested`, `abi_compatible: false`. The four 5Ah senders share one model, and the
  two thunks `007213B0` and `007213C0` point at the routines they jump to.
- Two records point at the existing `radix_sort_records16_00722210`, as listed in section 2.3.
- No Ghidra renames or comments were applied in this packet.

**Build.** `./scripts/build.ps1` passes in the worktree, and both CTest suites pass.

**Not modelled (open), with the reason:**
- **Existing models elsewhere, not re-checked here.** I did not add records for these:
  - `0071BA20` (`model_collect_children_by_prefix_0071ba20`, src/frontend_seg12_17_cc13.cpp);
  - `0071BE60`, `0071D810`, `0071D9E0` (src/unit_commanded_speed.cpp);
  - `0071BFF0` (src/ship_ai_state_steps.cpp);
  - `0071D2A0`, `0071D340`, `0071FC40` (src/ai_command_tick.cpp);
  - `0071D880` (src/entity_command_arms.cpp);
  - `0071EBF0` (src/unit_gunnery_pass.cpp);
  - `00722210` (src/band_0067_cc13.cpp).
  
  A follow-up should check each against the bytes and record it.
- **Closed in cc13-gpr38b:** `0071F600` (`begin_command_base_0071f600`), `0071E4C0`
  (`current_path_point_0071e4c0`) and `00720450` (`copy_state_from_00720450`, including the
  land-to-attackmove rewrite for kind-6 owners).
- **`0071FDE0` is modelled with coverage `partial`** (`director_path_add_point_0071fde0`). The gate,
  the list edits and the push order are the image's. The world-bounds helpers `007B1EC0`,
  `0071C4F0`, `004BBDD0` and `BSP_Point3Vector_PruneFromIndex` are supplied callbacks. The argument
  roles of `004BBDD0` are not settled.- **`0071F9F0`, `0071FD10`, `0071FC70`.** Vector assign, path copy and path destructor: STL and
  ownership plumbing.
- **Reference-count plumbing:** `0071EA60`, `0071F0E0`, `0071F110`, `0071F3B0`, `0071F460`,
  `0071F530`.
- **Thin forwards and accessors:** `0071BFC0`, `0071C150`, `0071C1A0`, `0071C280` (noop),
  `0071C470`, `0071F150`, `00721030`, `00721CB0`, `00721CC0`.
- **The USNOS question (section 1).** It is static. No game run was made, by the packet's terms.
  Still open:
  - which arm `007F16D0` answers for class 32;
  - whether a landed Betty leaves `Mission.FinalTrgs`.
