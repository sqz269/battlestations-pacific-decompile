# What `007F16D0`'s answer drives, and whether the host matches it (packet `cc13-sqrtb`)

Worker cc13-reltimer, branch `agent/cc13-sqrtb`, 2026-10-19. Read-only analysis of STUB census row 17
(`docs/CC13_STUB_CENSUS.md`). No game was run, and no ledger record, lease or reconstruction touches
`007EE000-007F2FFF` (cc9's squadron-director range). The bytes come from the exports of
`/battlestationspacific.exe`. The logs are the census's six missions with a resolution, section 3.

**Verdict:** the squadron arm is not record-only. In the image, every caller of `007F16D0` turns its
answer into the squadron's next command. In the host, the same answer, cached per squadron in
`rtb_census`, chooses the land or retreat install of every member plane. In the six missions, each
non-null resolution was followed by that install for every placed member at the same simulated
second. The `RECORD ONLY` text in the `returntobase 007F16D0` log line and the `record(...)` status are
stale labels. The one divergence found is a compensating pair on kamikaze squadrons (section 4). It
is not live today, but it is a trap for whoever binds either half.

## 1. `007F16D0` writes only its out record

`void* __thiscall(ECX = squadron)(CommandRecord* out)`, `RET 4`. Every store goes through the `out`
pointer (`[ESP+38h]` after the prologue) or through `007F1000` / `007EF8B0`, which take `ECX = out`:
- `007F176E`: `007F1000(out, 00E08FA0, [sq+404h])` is the home arm, `land` at the home base.
- `007F17D2`: `007EF8B0(out, 00E08FA0, 00465080([site+4]+7Ch, 0.0))` is the site arm, `land` at the
  nearest landing site.
- `007F1896`-`007F18E6`: `out+0` = `00E08F90` (`retreat`), `+4`, `+5`, and `+0Ch`/`+14h` = the
  quarter-sum of the zone's corners (`00D7A348`).
- `007F18EB`-`007F192D`: the null arm, with `out+0` = 0 and the position `[00F87574..7C]`.

No squadron or unit field is written, so whatever the answer drives, it drives through the callers.

## 2. The callers and what each does with the answer

The callers were found by rel32 + absolute scans and agree with `bsp.py lookup 007F16D0`:

| caller | site | use of the answer |
|---|---|---|
| `007F1940` BSP_PlaneSquadron_ApplyEntityCommand | `007F1AFB` (the `returntobase` arm, `007F1AED CMP EAX,00E08F98`) | `EDI` = `out+0`. Null: `007F1B2D JZ 007F1B5D`, nothing is queued. Otherwise the record is copied (`0071DB50`). Unless the force flag `BL` is set, the director's `vtable[+34h]` must accept it. It is then queued on the squadron's director at `+348h` (`0071D880` first when `BL`; `0071ECF0`). The guard at `007F1AE4` needs `+3D0h`, a head plane. |
| `0084E010` BSP_PlaneSquadronCommandBlock_Step | `0084E552` (`B5` and the step tail) | Null: `0084E55B JZ 0084E595`, the moveto-self test. Otherwise `0071D880` clears the block, `0071ECF0(out+0, &out+4)` issues the command, and `+228h` = 1. |
| `009FFEB0` BSP_AiCommand_SendEmptySquadronHome | `009FFF09` | A non-null descriptor is issued through `0077D600(desc, &record, 1)`; null answers 0. |
| `0099A170` BSP_Bot_InstallCommandTask | `0099A201` | Only `out+0` is read: null goes to `009C3C40`, and `00E08F68` / `00E08F80` / `land` / `retreat` go to their factories. The target comes from the original command (`0071EB60`), not from the record. |
| `005F9CF0` BSP_HudOrder_ResolveReturnToBase | `005F9D15` | A pass-through: the record is returned to the HUD caller. |

So the answer becomes the squadron's current command: `land` (`00E08FA0`) at the home base or at a
site, `retreat` (`00E08F90`), or nothing. The member planes read it from there:
- **Bot intake.** Each member's bot command intake takes the squadron's `land` through the land arm
  `0099A3DD`-`0099A41A` (`docs/SQUADRON_LAND_TASK.md`), and `retreat` through `0099A45B` (`009CA2B0` ->
  `009C9D00`).
- **Land-task validity, `009B34D0`.** Every tick, the land task's check keeps the task only while the
  squadron's current command (`vtable[174h]`) is `land` with its target equal to approach `+30h`.
- **Re-issue, `009B3915`-`009B39AD`.** When that target moves, the land task re-issues `land` at the
  new target.
- **Spent-ordnance latch, `009F8160`.** With the current command `land`, `009F8295`-`009F82D5` latches
  `+3Ch` when the site is gone. `0084E010`'s B5 then resolves again through `007F16D0`, which closes
  the loop.

## 3. The host against the image, in the six missions

The host's path:
- `record_return_to_base_007f16d0` (`src/game_hosts_units.cpp`) resolves at the flight leader, once
  per delivered order. Its callers are the fan-out, `issue_return_to_base_007f16d0` (`0084E010 B5`,
  `0084E552 step tail resolve`), and `009FFF09`'s exclusion arm.
- The answer is cached per squadron in `rtb_census` (`last_arm`, `last_site`).
- `install_land_task_0099a3dd` reads that cache for each member. Arm 3 installs `retreat`
  (`install_retreat_task_009c9d00`, `kPlaneRetreatTaskBound`). Arm 2, and arm 1 with
  `kReturnToBaseHomeArmBound`, install `land` at the site or the home base
  (`install_land_task_core_009b41c0`). Any other arm is refused and nothing is installed, as with the
  image's null.
- The same cache stands in for the squadron's current command in `009B34D0` (a labelled
  substitution, `docs/SQUADRON_LAND_TASK.md`).

Logs: the census logs (`docs/CC13_STUB_CENSUS.md` section 1), joined per squadron by
`local/sqrtb_join.py`, using the `returntobase 007F16D0:` lines and the
`summary land task squadron` lines, plus the retreat installs `retreat <plane> -> moveto|follow`.

| mission | resolutions (squadrons) | arms | member outcome |
|---|---|---|---|
| USN21 | 30 | home 14, null 16 | 14 squadrons: `land` at the home carrier for every placed member, first install = the resolution second (e.g. Lexington_sqn04 545.84 s, Intrepid_sqn31 779.23 s, 3/3). 16 null: JakeTmpl (116.50 s, 1 refusal) and Kamikaze1-15 (2767.57 s, 5 refusals each); nothing installed. |
| USN01 | 5 | site 5 | `land` at Enterprise; 12/12 placements installed at the resolution second. |
| BSM09 | 6 | site 4, home 2 | D3A Val 1-4 `land` at Zuikaku (22.40-27.00 s), Yorktown_sqn01/02 at Yorktown; 16/16 installed. |
| BSM01 | 68 | retreat 68 | `retreat` on every member: 118 retreat installs (`rtb_installs=118`), first at the resolution second (Jap #2.1 150.00 s, 3/3). |
| IJN08 | 3 | site 1, home 2 | 8/8 installed at the resolution second. |
| USNOS | 15 (13 squadrons) | site 7, home 8 | 33/33 installed at the resolution second. plane #4.6 and #4.4 re-resolve at 972.09 s (`land target gone` at Airfield3 -> B5 -> `land at site Airfield1`) and re-install at that second, the image's `009F82D5` -> `0084E552` loop. |

There were no refusals other than the null arms: `refused unread=0 ground=0 dead=0` throughout,
and there were no retreat refusals. The timing difference is the one already labelled: the host
installs at the order's delivery, while the image's member bots take the squadron's command on their
next tick (`docs/SENTITY_INIT_ATTACH_ORDER.md` 22.7). The planes are the same: the host's members are
the squadron's live registry members, which are the image's `+3D0h` array.

## 4. The kamikaze pair: two deviations that cancel

USN21's Kamikaze1-15 (class 45, "Kamikaze - A6M Zero", `IsKindOf(17h)`, 5 planes each) spawn at
2767.57 s with the `kamikaze` command `00E08F50` at Intrepid. In the host:
1. **The spent latch fires at spawn.** The log reads
   `spent ordnance: squadron "Kamikaze1" latched (command 00E08F50, 5 live) at 2767.57 s`. The host's
   `009F8160` ordnance arm (`game_hosts_units.cpp`, the loop calling
   `plane_holds_rack_round_007b9140`) asks only the racks. The image's arm calls `007ED790(sq, 0)`
   (`009F81CD`), which walks `+3D0h` calling `007B9140(member, 0)`, and `007B914A` answers 1 for any
   `IsKindOf(17h)` plane before looking at a rack. So the image never latches a kamikaze squadron, and
   `009F81E9 JNZ 009F820E` skips the latch. The other host callers of the stand-in add the 17h arm
   themselves (`game_hosts_ai.cpp` `record_loadout_00a04560`, `game_hosts_script_orders.cpp` `0089ECBD`,
   `game_hosts_lua.cpp` `007ED7B3`); the latch does not.
2. **`007F16D0` answers null.** `record_return_to_base_007f16d0` passes `in.head_c24 = false` ("not
   carried"), so `007F1709`-`007F1720` (a kamikaze head with `+C24h` clear) takes the null arm. The
   host does carry `+C24h`: `plane_pilot_fires_c24` (`kPilotFiresBound` ON, `007CD930`). For class 45
   it is 1, because all four platforms author `PilotFires = true` (`vehicleclasses.lua` lines
   23261-23334). With the real byte, the image would go on to the home arm, the site arm, or the
   retreat arm. `004C7730` finds a side-1 zone on every map, since `004E7142`'s table gives side 1 to
   six of the twelve zones, so the answer would be non-null.

Today the two cancel. The null answer installs nothing, so the kamikaze tasks installed at 2767.57 s
(`kamikaze task 009AF720 kind 0Ch installed Kamikaze1 -> Intrepid`) keep running, which is the image's
outcome because it never latches. Residual effects are counters and log lines only:
- `spent_rtb_latches`, `spent_rtb_issues` and the B5 line;
- 15 `land task REFUSED` lines;
- these squadrons are held by the spent path, so the general step tail (`kSquadronGeneralStepTailBound`)
  skips them.

**The trap.** Carrying `+C24h` into `007F16D0` alone sends all 75 kamikazes into `retreat` (or `land`)
at the moment they spawn. Fixing the latch alone is safe on its own. The adoption order is therefore:
1. Add `007B914A`'s kind-17h arm to the spent latch (`009F81C8`-`009F8205`) and to the other latch
   loops that stand for `007ED790`.
2. Only then pass `slots[head]->plane_pilot_fires_c24` as `in.head_c24`.

The expected reach of step 1 is USN21 only: across the eleven census logs, the only
`latched (command 00E08F50` lines are USN21's fifteen. It removes 15 latches, 15 B5 issues and 75 refusals and changes no plane's
task. Step 2 then changes nothing in the six missions, because no kamikaze squadron resolves any more.
The latch lives in the units host outside the struck range, but it is cc9's squadron spent-ordnance
packet (`cc9_squadron_spent_ordnance_rtb`), so it is routed to cc9, not built here.

## 5. Census row 17

The row's question was whether the squadron arm itself drives anything. It does, in both the image
and the host. The host matches the image's land and retreat outcome for every resolution in the six
missions, with the labelled one-tick-early install. Proposed status: `matched (stale RECORD ONLY
label)`, with the kamikaze pair of section 4 as the one open item.
