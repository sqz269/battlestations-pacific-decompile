# The plane's device walk, 007CEB00 (packet cc9_plane_device_walk)

2026-09-26. Ghidra read-only; names are ledger hypotheses. Switch `kPlaneDeviceWalkBound` in
`GameUnitsHost::Impl` (`src/game_hosts_units.cpp`), landed OFF first. OFF keeps the record
`Plane::device_busy_1fc` (`007ceb00`) and answers "not busy".

## What 007CEB00 is

**Where it sits.** `007CEB00` is not a function. It is the loop head of the cleanup arm of the
release-issue stage inside `BSP_PlaneTickElement_FixedStep` (body `007CE040`-`007CF172`), with
`ESI` = unit+310h.
1. The arm is reached only when:
   - the five guards pass;
   - the interval timer has expired;
   - no issue request is left (unit+C20h <= 0);
   - unit+C25h is set;
   - the timer is below -1.2 (`00D05EA4`).
2. It parks the timer at -0.5 (`00CE69D0`, `007CEAEE`).
3. It walks `unit+974h[0 .. unit+994h)` (`ESI+664h`, `ESI+684h`) and calls `device->vtable[1FCh]()`
   on each entry (`007CEB00`-`007CEB1D`).
4. The first true answer jumps to `007CEB31` and abandons the clear. With no busy device it
   clears unit+C25h (`007CEB22`) and zeroes unit+C2Ch (`007CEB29`).

The host already had the rest of the stage (`bsp::plane_release_issue_stage_007ce9fd`,
`docs/TORPEDO_ISSUE_TIMING.md`). Only the walk's answer was a stand-in.

**The list.** Two routines fill it, both with the plane's children that answer `IsKindOf(25h)`
through `vtable[5Ch]`:
- `007CDF20` at `007CE01F`-`007CE02C`, at creation;
- `007BCB30` at `007BCB90`-`007BCBC2`, the re-equip. Its only caller is `007ED690`. It kills the
  old devices (`00926D90`), creates the new ones (`0095A880`) and rebuilds the list, skipping
  children with +5Fh set.

Class 25h is `MBombPlatform`, and its only subclass is `MMultipleBombPlatform` 26h
(`docs/ENTITY_CLASS_IDS.md`). The list therefore holds the plane's bomb and torpedo racks, and
never its guns.

**The answer, vtable slot +1FCh:**

| class | vtable | slot `+1FCh` | body |
| --- | --- | --- | --- |
| `MBombPlatform` 25h | `00CF96A8` (entry `00CF98A4`) | `006E3CD0` | `MOV AL,[ECX+498h]; RET` |
| `MMultipleBombPlatform` 26h | `00CF9918` (entry `00CF9B14`) | `006E4360` | `XOR AL,AL; RET`: never busy |

**The busy byte +498h.**
- **Writers, from a byte scan of every store form (`C6`, `88`, `C7`, `89`, `66 89` with disp32
  498h):**
  - the `MBombPlatform` constructor `006E3C00` writes 0 (`006E3C75`);
  - `006E3550`, both classes' vtable slot `+1F0h` (entries `00CF9898` and `00CF9B08`), writes 1
    (`006E3556`). It also adds its float argument to +494h: `+494h += arg; +498h = 1`.
  - `BSP_MCatapult_Construct` stores a dword at +498h in a different class.
- **No other writer.** The scan cannot see a write through a sub-object base, so the claim is
  "no literal-displacement writer", not "no writer".
- **Where it is raised.** `BSP_Plane_TickReleaseOrderIssue` `007C0D90` (the stage's issue call,
  `007CEA8D`) walks the same children. For the first rack that holds ordnance 2Ah
  (`vtable[210h](2Ah, 0)`) and is not busy (`vtable[1FCh]`), it fires it through
  `vtable[1F0h](delay)` at `007C0E17`.
- **When the walk stops.** A level bomber (`IsKindOf(10h)`) dropping plain bombs keeps walking,
  firing every rack with a staggered delay (`007C0E67`-`007C0EC9`). Anything else stops after
  the first rack: torpedo 2Bh, depth charge 2Ch or rocket 33h, or a plane that is not a level
  bomber. Either way it sets unit+C25h (`007C0EE2`) and, unless the ordnance is a rocket, calls
  `007EEF30`.

**What it gates.**
- A plane whose fired rack is an `MBombPlatform` answers busy from then on, until a re-equip
  constructs new racks. Its cleanup never clears unit+C25h, and the arm re-parks the timer every
  0.7 s of step time.
- A plane with only multiple racks clears as before.
- In this host the only readers of unit+C25h are the kind-7 move-to states: the moveto and
  circle release arms, and the follow state's release arm. Torpedo and dive-bomb tasks do not
  read it here, so the flag's lifetime changes counters, not motion, for aircraft without a
  move-to task.

## The reconstruction

`plane_device_walk_007ceb00()` answers `rack_single_fired > 0`.
- **The census.** `plane_rack_census()` counts, once per unit, the plane's `BSPGun` platforms
  whose Function is `BOMBPLATFORM`, split by the device class's `Type`: `"BombPlatform"` or
  `"MultiBombPlatform"`. This installation's `classtables/realistic/deviceclasses.lua` has 28 and 6
  of them.
- **Firing.** The issue path `run_release_order_issue_007c0d90` marks one single rack fired
  each time its walk finds a device, while an unfired single rack is left.
- **The count.** The walk is counted, and marked `done`, only in the cleanup arms, where the
  image runs it.
- **The log.** A summary line `torpedo <unit> device walk 007CEB00: single= multi= fired= walks=
  busy= C25h=` is printed only when the switch is ON.

**Substitutions and assumptions**, labelled in the code:
- The mapping from `Type` to the constructed class (25h or 26h) is the device factory's, and is
  assumed from the names.
- The host keeps no child list, so the rack fired is taken to be a single one while any single
  rack is unfired. The list order is unread, and it matters only for a plane that mixes both kinds.
- A re-equip (`007BCB30`) is not modelled, so no rack is rebuilt.

**Bodies with no Ghidra function** (`ghidra proto --brief` shows none; INT3 padding before
each; each referenced only from the vtable entries named above):

| start | inclusive end | evidence |
| --- | --- | --- |
| `006E3CD0` | `006E3CD6` | `8A 81 98 04 00 00` `MOV AL,[ECX+498h]`, `C3` at 006E3CD6; INT3 006E3CC3-006E3CCF and from 006E3CD7 |
| `006E4360` | `006E4362` | `32 C0` `XOR AL,AL`, `C3` at 006E4362; INT3 006E435C-006E435F and from 006E4363 |
| `006E3550` | `006E3569` | `FLD [ECX+494h]`, `MOV byte [ECX+498h],1`, `FADD [ESP+4]`, `FSTP [ECX+494h]`, `RET 4` (`C2 04 00`) at 006E3567; INT3 006E3543-006E354F and from 006E356A |

## Predictions (written before the runs)

The pair is `local\dw_off` against `local\dw_on`, built from main `a4e3798c3` plus this packet.
Both sides have every switch as on main, streams on, and one run at a time.

| row | E2 9000 | USN04 4700/4500 |
| --- | --- | --- |
| `Plane::device_busy_1fc` | UNIMPLEMENTED (about 361 500 on E2, 187 200 on 4500) becomes concrete, with calls = the cleanup-arm entries: from tens to about 2 000, most of them blocked retries | same, fewer |
| rack census | every B5N Kate has one single rack (its torpedo), `fired=1` after its drop; D3A Vals carry racks but fire none through this path (0 `fired`); Zeros none | same |
| unit+C25h at the end | 1 for every Kate that dropped (5 on E2), against 0 OFF | same pattern |
| issue-stage lines | `cleanups` rises and `waiting` falls for the dropping Kates; every other torpedo line identical | same |
| plane deaths, hit records, torpedo / dive releases, the Lexington's movement | identical: no host reader of unit+C25h is on a Kate or Val path | identical |
| identical rows | everything except the rows above | same |

If a behaviour row moves, the unit+C25h lifetime reaches a reader not listed above, and that
reader is the finding.
