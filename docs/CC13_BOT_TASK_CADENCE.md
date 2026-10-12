# CC13 bot task think cadence: 0099B740 and task+308h (packet cc13-cadence1)

Addresses: 0099B740, 0099B710, 009BB6C0, 007ED3F0, 009998A0, 00999912-009999C1, 0099993C,
0099ACD0, 0099AD0F-0099AD7E, 0099AEF2-0099AF1C, 0099AE2B, 0099AEB3, 0099C6F0, 0099C765, 0099C8B0,
0099C925, 00BD2F10, 009BB640, 009B9420, 009BC3A0, 009A2810, 008A4B10, 007F0068

## Verdict

**The host is faithful enough here, and there is no diff.** Calling 0099B740 on every think instead
of on the image's 1 Hz cadence does not change behaviour in the current host, so
`kBotTaskThinkCadenceBound` is not proposed. The evidence:

1. 0099B740 only ever does one thing: it assigns `squadron+370h = 1`, and it does that only for the
   flight leader. Every bot task class in the image reaches it with the same constant-true predicate
   except one, and that one also only assigns 1 (section 2). On its own, running it more often
   changes nothing.
2. Running it more often could only matter if something wrote `+370h` to another value between two
   runs and something read that value before the next run. Within the host, every write of a value
   other than 1 happens at task install. The image runs 0099B740 on a task's first think whatever
   the phase (section 3), and so does the host, so the value is overwritten in both before the task
   arm reads it.
3. The image does have two later writers, which the host does not model: the `closetoship` chain,
   which starts with the Lua binding `PilotCloseToShipDrop`, and the pilot-control message `BCh`. No
   loose mission script in this installation calls `PilotCloseToShipDrop` (section 5).
4. The phase draw `-U(0,1)` at 0099C765/0099C925 has no effect in the host. Adding it would only
   add one draw per task construction to the unkeyed units-host sequence
   (`BSP_GUNNERY_RNG_STREAMS` unset) and shift every later draw there. Leave it out.

Status: exported (listing reads from disk bytes and the live Ghidra listing). No C++ was written,
so nothing was built or applied. The call-site rows were checked with
`tools/verify_report_calls.py` (17 rows, 0 failed). Nothing was pair-tested or validated in the game.

**Side finding for cc9 (section 6):** slot `+54h`, the "cruise profile" that the host's comments
treat as per-tick, is dispatched at only two sites in the image, both in
`BSP_PilotBot_Tick`, and both on task **activation**. It is not called per tick. That is a cadence
question for the cruise profiles. It is not the subject of this packet.

## 1. The think chain, read from the listing

`0099ACD0 BSP_PilotBot_Tick(bot, dt)`, with the per-frame dt:

```
0099AD0F  acc = bot+70h + dt; bot+70h = acc
0099AD21  if (0.09 [00D1F39C] > acc) { light path 0099A9E0; return }      ; FCOMIP / JBE
0099AD68  bot+70h = 0; bot+80h += acc; ... the planner, with dt := acc     ; the 0.09 s think
0099ADAA  (bot+7Ch) 0099A170 install the command task; bot+74h = -1.0 [00D7A260]
0099AE14  (list non-empty, unit+184h, 007B8AD0, head->vt[38h]()) 009BBFC0 builds the type-F
          leader task, 0099AE1C prepends it, 0099AE2B CALL head->vt[54h]
0099AE7E  0099A4C0 retire; if the head changed: 0099AEB3 CALL newhead->vt[54h]
0099AEF2  bot+74h -= acc; if (bot+74h <= 0) 0099AF1C CALL 009998A0(head, acc)
0099AF3D  (unit+9C2h[F876B8]) bot+74h = 3.0 [00CE3854]
```

`bot+74h` is -1 everywhere except on the `9C2h` path, so 009998A0 runs on every 0.09 s think.
It has one caller: a rel32 scan of `.text` finds only 0099AF1C, and an absolute-dword scan of every
section finds none.

`009998A0 BSP_PilotBot_Update(task, dt)`, `RET 4`, ECX = the head task:

```
009998A4  if (task+270h == 2 && unit+9C2h[F876B8]) return
009998D9  if (0099C270(task))  { task+2E4h = 0;  task->vt[64h](dt); 0099D300(task+4, dt); return }
00999912  t = task+308h                                                 ; FLD/FSTP/FLD
00999920  FLD dt; FCOMI ST1; JB 009999B5                                ; dt < t -> not due
0099992C  task+308h = t + (task+304h - dt)                              ; FSUBR [+304h]; FADDP
0099993C  CALL 0099B740
00999945  task+2E4h = FFh; task->vt[64h](dt); ... 0099D300(task+4, dt)
009999B5  FLD ST0; FSUBP ST2,ST0; FXCH; FSTP task+308h  -> task+308h = t - dt; JMP 00999945
```

The x87 was checked against the bytes (`009999B5: D9 C0 DE EA D9 C9 D9 9E 08 03 00 00`).
`DE EA` is `FSUBP ST(2),ST(0)`, so `ST2 = t - dt`. The `not due` branch leaves `dt` in ST0 and
jumps past 00999941's `FLD dt` to 00999945. There the `FSTP [ESP]` at 0099994D consumes it as the
vt[64h] argument, so the x87 stack balances.

`0099C270` returns true when `unit+9C2h[F876B8]` is set. That is the same flag 009998A4 tests. On
that path neither the countdown nor 0099B740 runs.

**The cadence, exactly:**

- `+304h` is the period. Both constructors write it: `0099C75D` and `0099C91D` store
  `[00D7A24C] = 1.0`. A disp32 scan of segment 67 and 007B finds no other writer.
- `+308h` is the phase, drawn by the two constructors (section 4). Its only writers are those two
  (`0099C76C`, `0099C92C`) and the countdown (`00999936`, `009999BB`).
- Due test: `dt >= t`, with equality counted as due (FCOMI then JB). When due, `t += 1.0 - dt`.
  The carry is kept, so the cadence does not drift. Otherwise `t -= dt`.
- `t` starts at or below 0, so **the first think of every new task is due**. After that the task
  is due about once a second. With `dt` near 0.09 s, that is every 11th or 12th think, at an offset
  set by the phase.
- `dt` is the accumulated think time (`acc` above), not the frame dt.

## 2. What 0099B740 does for each task class

`0099B740` (`0099B740`-`0099B77A`, read whole, disk bytes):

```
if (task+2FCh && task+2F4h && task+2F4h == [task+2FCh + 3D0h])     ; 0099B757: the flight leader
    if (task->vt[38h]())                                            ; 0099B766
        007ED3F0(ECX = task+2FCh, 1)                                ; 0099B772 PUSH 1, 0099B774
007ED3F0: MOV EAX,[ESP+4]; MOV [ECX+370h],EAX; RET 4                ; a plain assignment
```

To get each class's slot `+38h`, every vtable stored by the callers of both base constructors
(0099C6F0, 22 callers; 0099C8B0, 13 callers) and by their callers was read from the PE
(`local/cad1_vtables.py`). The primary task vtables are:

| vtable | class | `+38h` | `+54h` | `+54h` tails into 0099B740 |
| --- | --- | --- | --- | --- |
| `00D1F4E8` | closetoship | `0099B710` | `009A2D60` | no |
| `00D1F738` | depthcharge | `0099B710` | `009A6500` | `009A65E1` |
| `00D1F9B0` | dogfight | `0099B710` | `009AAF30` | `009AAF92` |
| `00D1FFA0` | land | `0099B710` | `009B3C60` | no (see note) |
| `00D20210` | levelbomb | `0099B710` | `009B8C90` | `009B8D71` |
| `00D1FC70` | dropkamikaze | `0099B710` | `009AE9D0` | `009AEACF` |
| `00D20E18` | divebomb | `0099B710` | `009C8920` | `009C8A87` |
| `00D20F60` | retreat | `0099B710` | `0099B660` (RET) | no |
| `00D210E0` | strafe | `0099B710` | `009CD020` | `009CD0C3` |
| `00D05918` | rocket | `0099B710` | `007B7AF0` | `007B7B93` |
| `00D1FD40` | kamikaze | `0099B710` | `009AF480` | `009AF501` |
| `00D213C8` | torpedo | `0099B710` | `009D4A70` | `009D4C08` |
| `00D20500` | stop | `0099B710` | `0099B660` | no |
| `00D20388`, `00D20400`, `00D20480`, `00D20568`, `00D205E0`, `00D206D0`, `00D20748`, `00D207C0`, `00D20910`, `00D20B68` (moveto), `00D20BE0`, `00D21228` | non-attack classes | `0099B710` | `0099B660`, `009BD6E0` or `009C3650` | no |
| `00D20658` | type-F flight-leader task (ctor `009BB640`, factory `009BBFC0`) | **`009BB6C0`** | `009B9420` | no |

`0099B710` is `MOV AL,1 / RET`. The single exception is `009BB6C0`:
`XOR EAX,EAX; CMP word [ECX+414h],0Ah; SETG AL`, so it returns true only after `+414h > 10`.
`+414h` is a per-think counter. `009BC52B ADD word [ESI+414h],BX` in its vt[64h] `009BC3A0`
increments it, and `009BC53D`, the activation hook `009B942C` and `009B9480` reset it. The class's
`+54h` (`009B9420`) calls `007ED3C0 BSP_PilotControl_ClearReleaseOrders`, which writes `+378h` and
not `+370h`.

So **for every class the effect is either nothing or the assignment `+370h = 1`.** That assignment
is idempotent. The type-F predicate only delays the first assignment until the next due think after
`+414h` passes 10. Neither the type-F task nor the non-attack classes have a host.

Census of 0099B740's callers: a rel32 scan of `.text` finds 10 sites, and an absolute-dword search
of all sections finds none. They are one `CALL` (`0099993C`) and the nine `JMP` tails in the table.
`docs/BOT_TASKS.md`'s cruise-profile table lists `0099B740` for land `009B3C60`. The census has no
site in `009B3C60`, so that row is wrong.

## 3. The host's four sites against the image

Image call sites: `0099993C` (on the 1 Hz cadence, for every class) and the nine `+54h` tails
(on activation only, section 6). On a new task the image runs 0099B740 at once by both routes: the
first `009998A0` after install is due, because `t <= 0`, and `bot+74h = -1` lets it run on that same
think.

| host site (`src/game_hosts_units.cpp`, origin/main) | stands for | runs | changes behaviour? |
| --- | --- | --- | --- |
| `run_dive_bomb_attack_mode_tick_0099b740` (18970; call 22581, before the arm) | `0099993C` | every think | **no**. `db_attack_mode_370` is written only here and by its seed, so after the first think it holds 1, and the image's 1 Hz re-run would write the same 1 |
| `lb_cruise_profile_009b8c90` tail (21612-21628; call 22033) | `009B8D71` tail / `0099993C` | every think | **no**. Besides this site, `lb_attack_mode_370` is written only at install (21500, kForced), and the image overwrites that on the first think too |
| strafe block (22280-22297) | `0099993C` | every think | **no**. Install (20764, kForced) is the same case as levelbomb |
| `run_attack_mode_tick_0099b740` (25397; call 24617) | `009D4C08` tail | every think | **no**. The only other writers are `run_attack_mode_lowering_paths` (25359), and both of its arms are dead in the host: `torpedo_closetoship_task_installed` is never set, and `torpedo_pending_mode_message_bc` has no producer |

The host and the image agree on order: the image runs 0099B740 before vt[64h], so the arm never sees
the install seed for a leader. The dive-bomb and strafe sites are also placed before their arms.

## 4. The phase draw

Two constructors draw it: `0099C6F0` and its sibling `0099C8B0`, which has 13 callers, one per class.
Each does `FLD1` / `FLDZ` into `[ESP+4]` / `[ESP]`, then `MOV ECX,1`, then
`CALL 00BD2F10` (`0099C765` and `0099C925`), then `FCHS`, then
`FSTP [ESI+308h]` (`0099C76C` and `0099C92C`). That makes `+308h = -U(0.0, 1.0)` on **stream 1**, the
same stream as the other units-host draws in `docs/CC13_UNLABELLED_DRAWS_BATCH.md`. The draw
happens once per task construction and never on re-arm. Activation does not redraw it.

The host does not need it. Its only reader is the countdown, and the first think after construction
is due whatever the phase. After that the phase decides only which think in each second runs the
assignment, and section 3 shows the result is the same 1.

## 5. When a diff would become necessary

A value other than 1 has to survive between due thinks. Three image paths can make that happen, and
none is hosted:

1. **closetoship drop.** `008A4B10` is the Lua name `PilotCloseToShipDrop`
   (`src/mission_lua_host.cpp` binding table, row `{"PilotCloseToShipDrop", 0x008A4B10U}`). The
   ledger name `BSP_LuaBinding_PilotStopCloseToShip` is a hypothesis. It calls `007ED430(2)` only
   when slot 0's command is closetoship (`00E08FA8`), and it builds no new task. The closetoship
   arm `009A2810` (vt[64h] `009A2B00`) arms its two-second timer only when it reads `+370h == 2`
   (`009A2869`). Because 0099B740 runs before vt[64h] on a due think, **the cadence decides whether
   the drop is armed.** If the next leader think after the Lua call is the due one, the 2 is
   overwritten with 1 before `009A2869` reads it. With a 1 Hz cadence that happens about one think
   in eleven. On a per-think host it would happen every time, and the drop would never arm. A host
   for this chain must model the countdown. The loose scripts in this installation (621 `.lua`/`.luc`
   files under the game root) never call `PilotCloseToShipDrop`. Only
   `scripts/missions/multi/competitive07.lua` (lines 893 and 1147) calls `PilotCloseToShip`.
2. **message `BCh`** (`007F0068`): it sets 0 or 2. Its producer is `contract: unread`
   (`docs/TORPEDO_ATTACK_MODE.md` section 2).
3. **the type-F leader task** (`00D20658`): its predicate opens on the first due think after
   `+414h > 10`. It is not hosted.

What such a diff would contain if it is ever needed. Per task host, keep `float think_phase_308`
and `think_period_304 = 1.0f`. Seed `think_phase_308 = -draw("<unit>#task308", 0, 1)` at install,
using the units-host keyed draw, never stream 0. On each think, `if (dt >= t) { t = t + (1.0f - dt);
run 0099B740 } else t -= dt;`, before the arm. The countdown must use the think's accumulated dt.

## 6. Side finding: slot +54h is an activation hook, not a tick

`docs/BOT_TASKS.md` (Open questions) left open which slot runs per tick. Section 1 answers it: the
per-think slot is **vt[64h]**, which 009998A0 calls at `009998FB` and `0099995A`. **vt[54h]** is
called only at:

- `0099AE2B`, on the type-F leader task that was just prepended; and
- `0099AEB3`, on the new head when `0099A4C0 BSP_PilotBot_RetireFinishedTasks` changed the head.

A linear sweep of segment 67 (`00996120`-`009F6060`) for `MOV r,[r+54h]` followed by `CALL r`
(`local/cad1_slotsweep.py`) finds only those two task dispatches; the third hit, `009F0DBB`, is in
the ship AI. The same sweep finds the two known `+64h` calls, so it is not vacuous. MSVC emits no
`CALL [r+54h]` form here, so a byte scan for that form proves nothing and is not cited. Image-wide
the sweep finds 105 `+54h` dispatches. Only `0099AE2B` loads its object straight from a `+58h` list
(`local/cad1_ctx.py`), and the rest were not adjudicated, so this claim is `coverage: partial`
outside segment 67 and `007B0000`-`00800000`.

A task installed at `0099ADAA` (`0099A170`, the command task) gets no vt[54h] on that think. If the
task is already the head when `0099A4C0` runs, the head does not change, so `0099AEB3` does not fire
either. This was not traced further: whether every attack task reaches a vt[54h] dispatch is
`contract: unread`.

The consequence for cc9 does not belong to this packet. The host comments call the cruise profiles
`009C8920`, `009B8C90`, `009D4A70` and `009CD020` per-tick (`game_hosts_units.cpp` 3900, 21581-21586
"the profile's call cadence (0099AE28, unread)", 24308, 25352). That includes the dive profile's
stream-1 draw at `009C897C` (`kDiveProfileDrawBound`). In the image they run on activation. For
0099B740 this makes no difference (section 3). For the profile bodies it is a separate packet.

## 7. Missions likely to move

None: there is no diff. A future closetoship-drop host (section 5.1) would move no mission in this
installation's loose scripts, because none calls `PilotCloseToShipDrop`.

## 8. Notes for cc9 (labelled, not diffs)

- `game_hosts_units.cpp` 1086, 20762 and 21500 attribute the install seed `kForced` to
  "008A4C41 -> 007ED430(2) at the order". `008A4B10` is `PilotCloseToShipDrop`, and it raises the
  mode only under a running closetoship command, so the attack order is not it. In the image the
  squadron's `+370h` is already 1 from the leader's first think of any task, because every class's
  predicate is true (section 2). The seed is overwritten before the leader's arm reads it, so it is
  harmless for leaders. A member that thinks before its leader in the install frame could read the
  seed for one think. That frame order was not traced.
- `BOT_TASKS.md`: land `009B3C60` has no 0099B740 tail (section 2). The Open-questions entry
  "which slot the scheduler calls per tick" is answered: vt[64h] from `009998A0`, and vt[54h] on
  activation (section 6).

## Scripts

All scripts are in this tree's `local/` (not committed): `cad1_vtables.py` (vtable slots of the
constructor callers), `cad1_slotsweep.py` / `cad1_ctx.py` (`+54h` dispatch sweep),
`cad1_dispscan.py` (`+304h`/`+308h` disp32 census) and `cad1_calls.json` (the verify_report_calls
rows).
