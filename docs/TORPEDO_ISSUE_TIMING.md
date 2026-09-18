# When the release-order budget is issued (packet `cc8_torpedo_issue_timing`)

Addresses: `007CE040`, `007CE848`, `007CE865`, `007CE96F`, `007CE9FD`, `007CEA02`, `007CEA0F`,
`007CEA1C`, `007CEA29`, `007CEA33`, `007CEA51`, `007CEA66`, `007CEA78`, `007CEA82`, `007CEA8D`,
`007CEA92`, `007CEAB3`, `007CEAB8`, `007CEABE`, `007CEAC6`, `007CEACF`, `007CEADE`, `007CEAEE`,
`007CEB00`, `007CEB22`, `007CEB29`, `007CEB31`, `007C0D90`, `007C0F01`, `007EEF30`, `007EEF3B`,
`007EEF7F`, `007EE7F0`, `007BCBE0`, `007B8AD0`, `007BBBA0`, `007BBBAB`, `007BBC00`, `007D5D20`,
`007D619A`, `007D61B4`, `007D625B`, `007F2C60`, `007F2CDB`, `007F2DC1`, `004F0AD0`, `009D4850`,
`009D4956`, `00CE3860`, `00CE6448`, `00D05EA4`, `00CE69D0`, `00E188A8`.

## The answer, and the correction it forces

The packet asked when the native hands a torpedo bomber its release-order budget `unit+C58h`,
on the assumption that the plane's fixed step issues it every step from mission start, so that a
task installed later always finds 999 waiting. **That assumption is wrong.** The issue path is
gated by a pending-release-request counter that only an ordnance-release request raises, so the
flight-wide budget is issued **after** an aircraft has asked to drop, not before.

`007CEA8D` is the only call site of `007C0D90 BSP_Plane_TickReleaseOrderIssue` in the image
(`python tools/callsite_census.py 007c0d90`: one `CALL`, rel32 and absolute-dword forms, image
wide). It sits in a stage at `007CE9FD`-`007CEB31`, the tail of `007CE040
BSP_PlaneTickElement_FixedStep`, behind five guards, a randomised countdown and that counter.

## Register provenance for the stage

`ESI = ECX = this` at `007CE065`. Filtering the whole listing of `007CE040` for every write to
`EDI` (`local/output/disasm-raw-007ce040-*.txt`, 1092 lines) gives seven sites plus the `POP`:
`007CE0ED`, `007CE166`, `007CE1FC`, `007CE273`, `007CE2CF`, `007CE32D`, `007CE435`, `007CE461`,
`007CE4B0`, `007CE684`, `007CE6B9`, `007CE6F7`, `007CE6FC`, `007CE848`. The last one before the
stage is `007CE848 LEA EDI,[ESI-310h]`; `007CEAFE MOV EDI,EDI` is alignment padding inside the
device loop, not a write. So **`EDI` is the unit and `ESI` the element embedded at `unit+310h`**.

That mapping is confirmed independently: `007C0EFA` inside `007C0D90` reads the pilot control
block at `+9D4h` of its `ECX`, and `ECX` is `EDI` (`007CEA85`), while `007CE877` reads the same
block at `ESI+6C4h`; `9D4h - 6C4h = 310h`.

The stage's fields are therefore `ESI+910h/915h/918h/91Ch` = `unit+C20h/C25h/C28h/C2Ch`, and
`EDI+900h` is the plane control mode `unit+900h`.

## The stage, instruction by instruction

| address | test | effect |
| --- | --- | --- |
| `007CEA02` | `CMP dword [[00E188A8]+1FE4h], 2` | equal jumps to `007CEB31`, the stage exit |
| `007CEA0F` | `CMP byte [ESI+6D0h], 0` = `unit+9E0h` | non-zero exits |
| `007CEA1C` | `CMP byte [EDI+C3Ah], 0` | non-zero exits |
| `007CEA29` | `CMP byte [EDI+5Dh], 0` | non-zero exits |
| `007CEA33` | `MOV EAX,[EDI+900h]`, then `7`/`6`/`4`/`5` | any other value exits |
| `007CEA51` | `FLD unit+C28h`, `FSUB [ESP+104h]`, `FST` back | the countdown, stored with `FST` so the value stays on the x87 stack |
| `007CEA6C` | `FLDZ`, `FCOMPI ST(1)`, `JBE` | continues only on a strictly negative timer |
| `007CEA78` | `MOV EAX,[ESI+910h]`, `TEST`, `JLE` | zero or less takes the cleanup arm at `007CEAC6` |
| `007CEA82` | `ADD EAX,-1`, store, `CALL 007C0D90` | spend one request and issue |
| `007CEAB3` | `BSP_Random_UniformFloatRange(1, 00CE3860, 00CE6448)` | `0.9`..`1.1`, low pushed first |
| `007CEAB8` | `FMUL dword [EBX+1F4h]`, `EBX = [ESI+228h] = unit+538h` | scaled by the class descriptor |
| `007CEAC6` | `CMP byte [ESI+915h], 0` = `unit+C25h` | nothing pending, exit |
| `007CEACF` | `COMISS [00D05EA4] (-1.2), timer` | continues only while `-1.2 > timer` |
| `007CEAEE` | `MOVSS unit+C28h, [00CE69D0] (-0.5)` | parked **before** the walk and before the count test, so a blocked cleanup retries every 0.7 s |
| `007CEB00` | device array `ESI+664h`, count `ESI+684h`, vtable `+1FCh` | any true answer abandons the clear |
| `007CEB22` | `unit+C25h = 0`, `unit+C2Ch = 0` | the clear |

The four constants were read from the image: `00CE3860` = `3F666666` = 0.9, `00CE6448` =
`3F8CCCCD` = 1.1, `00D05EA4` = `BF99999A` = -1.2, `00CE69D0` = `BF000000` = -0.5.

## The counter's producer, and why a store census missed it

`python tools/store_census.py 0xc20` reports three sites, of which only `007D625B` is on the
unit, and it writes `EBX` with `EBX = 0` from the `XOR` at `007D619A` (no branch lands between
the two; the adjacent `007D61BA` and `007D6261` write the same zero to `unit+C50h` and
`unit+BF4h`). `python tools/store_census.py 0x918` reports nothing outside `007CE040`. Taken
alone that would make `007C0D90` dead code, which it plainly is not.

An exhaustive scan of `.text` for **every** instruction whose memory operand carries a
displacement of `0xC20` with a register base, reads included, sixteen alignments with resync
(`local/scan_disp.py`), returns twelve sites:

```
007bbc00: add dword ptr [ecx + 0xc20], ebx      <- the raiser
007c6aad: cmp dword ptr [ebp + 0xc20], 0
007ca070: mov ecx, dword ptr [edi + 0xc20]
007d625b: mov dword ptr [esi + 0xc20], ebx      <- the zero seed
007d6f9b: lea ecx, [esi + 0xc20]
007ece49 / 007effef / 009b7cc8 / 009c2006: cmp dword ptr [ecx + 0xc20], 0
004da5cb / 004df7e1 / 004eb9da: byte fields on other objects
```

`007BBC00` is the last instruction of `007BBBA0 BSP_Unit_RequestOrdnanceRelease` (body
`007BBBA0`-`007BBC07`, `INT3` padding from `007BBC08`) before its `POP EBX; RET`, and `EBX = 1`
from `007BBBAB`. It is past all three of that function's early exits (`007BBBB0`, `007BBBC6`,
`007BBBDC`), so **the counter rises on every release request, accepted or not**. The store
census missed it because it is an `ADD` with a register source, not a `MOV`.

`007BBBA0` reads its device from `ECX+DECh` at its first instruction, which names `unit+DECh`
and confirms `ECX` is the unit.

## The native order

1. **`004F0AD0 BSP_SceneUnit_CreatePlaneSquadronGen`** (body `004F0AD0`-`004F0BDF`) builds the
   squadron block through **`007F2C60 BSP_PlaneSquadronTickableEntity_Construct`** (body
   `007F2C60`-`007F2E18`), which writes the avoid-zone layer `ctl+34Ch` at `007F2CDB` and the
   attack mode `ctl+370h` at `007F2DC1`. Its three callers are `006C5050`, `006EC8E0
   BSP_MCatapult_Fire` and `00844FC0 BSP_Shipyard_CreateLaunchedUnit`: all launch-time, none of
   them an order. `007F3500 BSP_PlaneSquadron_CloneFrom` and `009483D0` also call the
   constructor. **So the brief was right that the block exists from launch.** Which of the three
   spawns USN01's flight, and where `ctl+3D0h`/`ctl+3CCh` are filled, is **partial**: not read.
2. **`007D5D20 BSP_Plane_ReadPropertyBag`** (body `007D5D20`-`007D771E`) builds the device at
   `unit+DECh` at `007D61B4` and zeroes `unit+C20h` at `007D625B`. `unit+C28h` has no writer
   outside `007CE040`, so both the counter and the countdown start at zero.
3. Every fixed step of `007CE040`: the pilot-command commit `007BB920` at `007CE865`, the
   control-block block at `007CE877` (skipped to `007CE947` when `unit+9D4h` is null), the latch
   `007B9770` at `007CE96F`, then the stage. `007CE99D`'s `JE` lands on `007CE9FD` itself, so
   nothing branches around the stage and it runs for **every** motion arm, not only free flight.
4. The stage fires only with `unit+C20h > 0`, which only `007BBC00` raises.
5. `007C0D90` walks `unit+48h` for a class-`25h` device holding ordnance `2Ah`, sets `unit+C25h`
   at `007C0EE2` and calls `007EEF30` at `007C0F01`, which calls `007EE7F0` at `007EEF3B` and
   then hands 999 to the flight at `007EEF7F`.

**Answering the packet's question (1b) directly:** for a freshly spawned loaded bomber with no
order, `007C0D90`'s device walk answers *yes* (it is a class/ordnance test, nothing to do with a
task) and `007EEF40`'s armed-fraction gate answers *yes* (the flight is fully loaded, so
`ctl+374h` is below the threshold `ctl+390h`). Neither is the blocker. The blocker is upstream of
both, in the stage: `unit+C20h` is zero until somebody requests a drop.

**Answering (1c):** the stage is the last thing in the plane's fixed step. The pilot bot tick
`0099ACD0` is a separate tickable that deposits its command through `007B8C90` into
`unit+9FCh`..`+A14h`; `007CE040` then consumes it at `007CE865`
(`docs/PLANE_BOT_CONTROL_WRITEBACK.md` links 4, 10, 11, 12, 14). Whether the bot's tickable runs
before or after the plane element's in the same frame is **not read here**: the ordering inside
`007CE040` is, and that is what the stage depends on.

## The consequence: the budget is an effect of a drop, not its precondition

`unit+C58h` is spent by `0099AF53` in `BSP_PilotBot_Tick`, which offers each task its vtable
`+24h` arm. So the AI release loop is:

```
007BBBA0 request  ->  unit+C20h += 1  ->  next expiry of unit+C28h  ->  007C0D90
    ->  007EEF30  ->  unit+C58h = 999 for the flight  ->  0099AF53 offers  ->  009D49A0 arms
    ->  a drop  ->  007BBBA0 again
```

It is a loop with no entry from the budget side. `unit+C58h` is how one aircraft's drop
authorises the rest of the flight, and `007EEF30`'s second loop at `007EEF9F` says the same
thing more plainly: when the calling unit *lacks* a follow target (it is the leader), every other
controlled unit is handed a count of **1**.

The entry has to be a `007BBBA0` call that is not budget-gated. `python
tools/callsite_census.py 007bbba0` returns 34 sites. The torpedo task's own are `009D4956`
(`009D4850 BSP_BotTaskTorpedo_TickArm`, the manual passthrough `task+424h`), `009D3EB6`
(`BSP_BotTaskTorpedo_DumpRemainingRounds`), and `009D258A`, `009D25B3`, `009D26F8`, `009D2938`,
`009D29CB` in the `BotStateTorpedoDone` trio, all of which run *after* a drop. **Judgement on
the second release path:** `task+424h` is the only one of those that can fire first, the
previous packet's store census found no raiser for it in the image, and the message dispatcher
`007F0030` with its jump tables at `007F01E8`/`007F0204` is where a raiser would live. That
makes `task+424h` a *player/commander* authority rather than an AI one, and the AI entry is more
likely one of the four unexamined `007BBBA0` callers (`009C7FF0`, `009C83E0`, `009FA3A0`,
`009FD0E0`) or the force flag `ctl+378h`, which `007EEF62` uses to bypass the follow-target test.
None of those was read here: **partial**.

## ABI

| address | ABI | evidence |
| --- | --- | --- |
| `007CE040` | `void __thiscall(element, float step)` | `007CE065 MOV ESI,ECX`; the step is `[ESP+104h]` after `SUB ESP,0E4h` and four pushes |
| `007BBBA0` | `void __fastcall(unit)` | `ECX+DECh` at the first instruction; `007BBC07 RET` with no immediate |
| `007C0D90` | `void __fastcall(unit)` | `007CEA85 MOV ECX,EDI`; `RET 0` after `ADD ESP,20h` |
| `007EEF30` | `void __thiscall(ctl, unit)` | `ECX` from `unit+9D4h` at `007C0EFA` |
| `007B8AD0` | `bool __fastcall(unit)` | three instructions, `unit+9D8h == 0` |

## Host methods

- `bsp::plane_release_issue_stage_007ce9fd` (`src/torpedo_issue_timing.cpp`): the stage as a pure
  rule over explicit inputs, returning the arm, the two field write-backs and whether to call
  `007C0D90`.
- `bsp::issue_stage_mode_allows_007cea33`: the four-value control-mode test.
- `bsp::release_request_raise_007bbc00`: the counter raise.
- `GameUnitsHost` (`src/game_hosts_units.cpp`): the stage now runs from
  `PlaneBinding::latch_control_input_007b9770`, the sequence position of `007CE96F`, instead of
  from inside the free-flight arm. Three further host changes:
  - the task gate on the issue path is gone, because `007CE9FD` has none;
  - `request_ordnance_release` raises `unit+C20h` through the rule;
  - the release-order binding's controlled-unit array is the torpedo-armed aircraft rather than
    the aircraft with an installed task, because `ctl+3D0h` belongs to the squadron and the
    squadron exists at launch. **SUBSTITUTION**: this host has no squadron object.

Contracts logged through the unimplemented-host mechanism: `App::state_1fe4` (`007CEA02`),
`Plane::issue_block_c3a` (`007CEA1C`), `Unit::issue_block_5d` (`007CEA29`),
`Plane::device_busy_1fc` (`007CEB00`), `PlaneClass::issue_interval_1f4` (`007CEAB8`).

## Corrections

### To `docs/TORPEDO_RELEASE_ORDERS.md`

That doc's ledger note on `007C0D90` calls it "the one path from the plane tick to the
release-order budget", which holds, and its note on `007EEF30` says the path is "reached only
from `BSP_PlaneTickElement_FixedStep` through `007C0D90`, so the player's fire button does not
raise `unit+C58h`", which also holds. What it does not say, and what this packet adds, is that
the call at `007CEA8D` is behind the `007CE9FD` stage: the budget is not issued on every step,
and it is not issued at all until `007BBC00` has raised `unit+C20h`. Its `TorpedoReleaseOrderHost`
contract list should gain `007BBC00` as the producer of the field that decides whether the host
is called.

### To `docs/BOT_TASK_STATES.md`

Its ledger record for `007BBBA0` already ends "then increments `unit+C20h`". That observation was
right and is not corrected here; what was missing is the consumer. Nothing had connected that
increment to `007CEA78`, so the field read as bookkeeping rather than as the gate on the only
call site of the whole release-order path. The exhaustive scan above is what makes the link
exclusive: `007BBC00` is the field's only raiser image wide.

### To this repository's host

The reconstruction note on `torpedo_release_orders_c58` reads "Its raiser was not found:
contract: unread, so it stays at zero here." The raiser is `007BCBE0` at `007EEF7F`, reached
through the chain above; what was not found, and now is, is the raiser of the counter that lets
that chain run.

## no_ghidra_function

None. Every address cited here falls inside a function Ghidra has defined: `007CE040`
(`007CE040`-`007CF172`), `007BBBA0` (`007BBBA0`-`007BBC07`, `INT3` from `007BBC08`), `007C0D90`
(`007C0D90`-`007C0F0D`), `007EEF30` (`007EEF30`-`007EEFF6`), `007EE7F0` (`007EE7F0`-`007EE8A0`),
`007BCBE0` (`007BCBE0`-`007BCC16`), `007D5D20` (`007D5D20`-`007D771E`), `007F2C60`
(`007F2C60`-`007F2E18`), `004F0AD0` (`004F0AD0`-`004F0BDF`).

## Validation

Both baselines are this tree's own before-runs at `f4a96ed28`, because the standing censuses have
moved twice today.

### USN01, five ordered torpedo aircraft

| measure | before | after |
| --- | --- | --- |
| budget `unit+C58h` on arm tick 1 | 0 | 0 |
| peak `unit+C58h` | 999 | 0 |
| issue ticks | 2597 | 0 |
| arm offers (`0099AF53` passed) | 1298 | 0 |
| arm ticks blocked by no order | 1 | 1299 |
| `prepare` entered on arm tick 1 | Mav2, Mav3 | Mav2, Mav3 |
| `007EEF40` gate `ctl+390h` / `ctl+374h` | 0.9500 / 0.8000, open | never evaluated, both 0 |
| release requests `007BBBA0` | 0 | 0 |
| releases | 0 | 0 |
| aim complete `009D15F0` | 5 of 5, first at aim tick 275 | unchanged |
| goaway enters | 98 | 98 |
| gunnery damage | hull 23, total 220.0, deaths 1 | unchanged |

The after-run's stage census reads `stage_ticks=3000 guard_blocked=0 waiting=3000 issues=0
cleanups=0 requests_007BBBA0=0 C20h_left=0 C28h=-150.005` for every aircraft. The countdown
free-falling to -150 over 3000 steps of 0.05 s is the native behaviour with nothing to spend:
`007CEAC6`'s `JE` exits before the cleanup arm can park it, because `unit+C25h` is never raised.

The `007EEF40` row reads zero after the change only because the gate is never reached: the stage
exits at `007CEA78` before `007C0D90` runs, so the binding that writes those two fields is never
built. The `prepare` window is unchanged: Mav2 and Mav3 entered `prepare` on arm tick 1 in **both**
runs, against a budget of 0 in both. That row was misread from Mav5's line alone on the first
pass and is corrected here.

The aim tick, the goaway state and the attack-mode history are bit-identical before and after,
so the change is confined to the release-order path: the state machine still runs every tick and
only the `0099AF53` offer is withheld, which is what the native gates.

This is a **deliberate regression in the peak-budget number and a correction in behaviour**. The
host previously handed out 999 from the second step of every ordered aircraft's life on no
native authority at all. It now issues nothing, which is what the native does until a drop is
requested.

### The next gate

`unit+C58h` at `0099AF53`, value 0 on all 6495 ticks. Upstream of it, `unit+C20h` at `007CEA78`,
value 0 for the whole mission. Upstream of that, a call to `007BBBA0` that does not depend on the
budget: for the torpedo task the only candidate is `009D4956`, gated on `task+424h`, for which no
raiser exists in the image.

### USN02

USN02 has no ordered aircraft carrying torpedo ordnance, so `0099A170` builds no kind-`Eh` task
and the torpedo census is empty in both runs. The stage now runs for every plane regardless of
ordnance, so the mission was re-run to attribute any change. **It is identical.**

| measure | before | after |
| --- | --- | --- |
| gunnery damage | queued 126, hull 125, deaths 3, total 13673.7, first hit 41.85 s | identical |
| ordnance census | `units_with torpedo=29` of 32 units with guns | identical |
| pilot attack | no unit ordered at a target the yaw arm could plan for | identical |
| host methods | 707 concrete, 489 unimplemented | identical |

Nothing to attribute. The host-method count is the tell: USN01 went 442 to 444 unimplemented,
which is the five new contracts of this stage (`007CEA02`, `007CEA1C`, `007CEA29`, `007CEB00`,
`007CEAB8`) minus the three that the issue path used to log and no longer reaches
(`Plane::droppable_device_walk`, `PilotControl::pre_issue_hook`,
`PlaneClass::issue_threshold_a0`). USN02's stayed at 489 because no unit in that mission runs the
reconstructed plane fixed step at all, so the stage never executes there.

## Follow-up packets

1. **`007BBBA0`'s four unexamined callers** `009C7FF0`, `009C83E0`, `009FA3A0` and `009FD0E0`.
   One of them is the most likely AI entry into the release loop; naming them closes the
   deadlock this packet documents.
2. **`ctl+378h`, the force flag** at `007EEF62`, which bypasses the follow-target test in the
   issue loop. Its writers were not censused here.
3. **`ctl+3D0h`/`ctl+3CCh` population**: which of `004F0AD0`'s three callers spawns USN01's
   flight, and where the unit array is filled. That would replace this packet's membership
   substitution with the real rule.
4. **`unit+900h`'s values 4 and 5.** Only 6 (`007C63F4`, spawn) and 7 (free flight) are named.
5. **The class descriptor field at `unit+538h`+`1F4h`**, the interval scale, and `+A0h`, the
   issue threshold `0079CD36` seeds `ctl+390h` from. Both stand in at 1.0 today.
