# The pitch-command callers: 009F9ED0's other three, and the host's seven 009FB800 seams

Addresses: 009A76E0, 009CA870, 007B4980 (the three other callers of 009F9ED0), 009FB800 (its
three exits), 009C18C0 (009C1B17), 009C4220 (009C4401), 009D07B0 (009D0A92), 009D0F10
(009D109C, 009D10FF, 009D1194), 009C62B0 (009C6F7D), 009BEE30 (009BFC21).

Packet `cc9_pitch_callers`. Analysis plus one small binding (section 3).

## 1. The three other callers of 009F9ED0

Each is slot `+0Ch` (the tick) of a bot-state vtable with the follow state's layout
(`00D20AB8`). Each vtable has exactly one `.rdata` reference and one constructor store. The
constructors place their state sub-objects 98h apart from `ESI = task+3F8h`, which maps them
onto `docs/BOT_TASKS.md`'s whole-object state offsets:

| routine | vtable (slot +0Ch at) | stored by | object | state | ledger name |
| --- | --- | --- | --- | --- | --- |
| `009CA870` | `00D20FE8` (`00D20FF4`) | strafe task `009CC020` at `009CC10B`, `EDI = ESI+254h` | task `+64Ch` | strafe `gotowards` | `BSP_BotStateStrafeGoTowards_Tick_Provisional` |
| `007B4980` | `00D057B8` (`00D057C4`) | rocket task `007B6830` at `007B691B`, `EDI = ESI+258h` | task `+650h` | rocket `gotowards` | `BSP_BotStateRocketGoTowards_Tick_Provisional` |
| `009A76E0` | `00D1F8D8` (`00D1F8E4`) | dogfight task `009A94E0` at `009A95B4` and `009A9670` at `009A9746`, `[ESI+284h]` | task `+67Ch` | dogfight `aim` | `BSP_BotStateDogfightAim_Tick_Provisional` |

The mapping check: in `009CC020`, `ESI+124h/1BCh/254h` fall on strafe `follow +51Ch`,
`prepare +5B4h` and `gotowards +64Ch`. In `007B6830`, `ESI+128h/1C0h/258h` fall on rocket
`+520h/+5B8h/+650h`. In `009A94E0`, `ESI+154h/1ECh/284h` fall on dogfight `+54Ch/+5E4h/+67Ch`.

**Their arguments, PARTIAL.**
* `009CA870` and `007B4980` are byte-for-byte twins. At `009CAC2D`/`007B4D3D`,
  `a = XMM0 - [ESP+18h]` with `XMM0` = dword `[00D7A208]` (-0.0) on both paths, so
  `a = -[ESP+18h]`. `d` is the x87 top. `[ESP+18h]` is built at `009CABB7`-`009CAC0F` from a
  negated `[ESP+28h]` and `[EBX+1Ch] * [[this]+30h]`, halved by double `[00D7A280]` (0.5) and
  compared against double `[00CE3D78]` times it. Its inputs were not traced.
* `009A76E0` at `009A78BB`: `a = [ESP+18h] - unit+100h` (the member's world Y; `EDI =
  [[ESI]+4]`) and `d` = the float at `[ESP+28h]` before the `SUB ESP,8`. Their producers were not
  read.

**None of the three states is modelled by the host** (no strafe, rocket or dogfight tick in
`src/game_hosts_units.cpp`). So their exact arguments bind nothing today.

## 2. The seven host seams

`009FB800` takes two stack floats (a desired altitude and a dimensionless reference), `RET 8`,
and writes `cmd+2BCh` and **`cmd+2D0h = 2` on all three exits** (`009FB93F`, `009FB95F`,
`009FBA41`). `009FBA50` clamps a glide-slope altitude and hands it, with its own fourth argument
as the reference, to `009FB800` (`docs/TORPEDO_DESCENT_LAW.md`). The image call sites below are
from each state's listing.

| host line | seam | image site | image routine | host arguments | verdict |
| --- | --- | --- | --- | --- | --- |
| 2913 | `run_follow_law_009bfee0_009bee30` (else branch) | `009BFC21` | `009F9ED0` | cmdAlt as `reference` (the old stand-in) | wrong routine, **already bound** to `009F9ED0` behind `kPlaneFollowFlyToPitch = true` (packet `cc9_follow_pitch`); the branch is dead |
| 5704 | torpedo attack run `command_altitude_and_throttle` | `009D0A92` in `009D07B0` | `009FBA50` -> `009FB800` | `009FBA50`'s clamped altitude and its fourth argument (`pitch_reference`) | arguments faithful; **mode store missing**: no `pitch_mode_2d0 = 2` |
| 6996 | dive-bomb `run_dive_bomb_move_to_tick_009c18c0` | `009C1B17` in `009C18C0` | `009FBA50` -> `009FB800` | same | arguments faithful; **mode store missing** |
| 7212 | dive-bomb `run_dive_bomb_attackrun_tick_009c4220` | `009C4401` in `009C4220` | `009FBA50` -> `009FB800` | same, mode written at 7218 | faithful |
| 7678 | dive-bomb `run_dive_bomb_flyabove_tick_009c62b0` | `009C6F7D` in `009C62B0` | `009FB800` | target altitude and `a.reference`; the image clamps its reference at dword `[00CE74F8]` 0.8 (`009C6F53`); mode written | faithful as far as read; **owned by cc9_flyover_speed**, not edited |
| 8872 | torpedo `run_move_to_tick_009c18c0` | `009C1B17` (the same moveto state) | `009FBA50` -> `009FB800` | same as 6996 | arguments faithful; **mode store missing** |
| 9020 | torpedo `run_goaway_tick_009d0f10` | `009D109C` / `009D10FF` / `009D1194` in `009D0F10` | `009FB800` | `[EDI+1Ch]` above the float `[00CE3930]` 20 m, else the float `[00CE3804]` 1000; reference `FLD1` 1.0; mode written | faithful (`include/bsp/torpedo_goaway_tick.hpp` carries both constants) |

So the defect of `docs/PLANE_FOLLOW_PITCH.md` (an altitude fed as the reference) occurs only in
the follow seam, and that is already fixed. The only other mismatch is the mode word, missing at
three `009FBA50` seams.

## 3. The binding, and why it is predicted to be neutral

`kPitchCommandCallersBound = true` writes `plan_state.pitch_mode_2d0 = 2` beside the pitch
target at host lines 5704, 6996 and 8872, as `009FB800` does in the image. The old form (no write)
is the `false` branch.

**Prediction: identical runs on USN01 and USN04.**
* The host's plan state starts at mode 2: `include/bsp/pilot_plan_slots.hpp`, `pitch_mode_2d0 =
  2`, from the reset at `0099B54E`.
* The only host writers of any other value are the dive-bomb aimdive (`0`), turndown (`0`),
  goaway (the rule's value) and fly-over (the rule's value).
* None of those runs before a dive-bomb or torpedo `moveto`, or before the torpedo attack run.
  A dive bomber leaves goaway for `attackrun`, which writes 2 itself, or for `done`, where the
  follow law writes 2.
* So the mode is already 2 whenever these three seams run, and the stores are faithful no-ops in
  every flow the host reaches today.

Any difference in the pair means a flow that reaches these seams with the mode not at 2, which
would itself be a finding.

## 4. Runs and decision

Control P0 is main at `128b845f2`; treatment P1 is the same tree with
`kPitchCommandCallersBound = true`. Each runs from its own copied binary. The two `.text`
sections differ, so the stores are compiled in.

| run | mission, parameters | binary | log | `summary mission` rows (refills excluded) | per-unit rows |
| --- | --- | --- | --- | --- | --- |
| P0 | USN01, `--frames 3200 --mission-frames 3000` | `local\binP0` | `local\P0_usn01.log` | 76 | - |
| P1 | USN01, same | `local\binP1` | `local\P1_usn01.log` | 76, identical to P0 | identical |
| P0 | USN04, E2 parameters (`--frames 9200 --mission-frames 9000`) | `local\binP0` | `local\P0_usn04.log` | 84 | - |
| P1 | USN04, same | `local\binP1` | `local\P1_usn04.log` | 84, identical to P0 | identical |

The per-unit rows compared were every `torpedo`, `divebomb`, `water contact`,
`release census`, `glide census`, `db aim exit` and `follow law` row: no difference. The seams
were reached. The torpedo attack-run seam (`BotApproach::command_altitude`, `009FBA50`) ran 1423
times in USN01 and 1182 in USN04, identically in both runs of each pair. The torpedo moveto's
glide census prints in both USN01 runs.

**Decision: the three mode stores land** (`kPitchCommandCallersBound = true`). They are the
image's own stores, and the pair is identical, as section 3 predicted from the mode already
being 2.

## 5. For cc9_flyover_speed / cc9_goaway_reattack

* The fly-over's `009FB800` seam (`009C6F7D`) matches the image as far as this packet read it:
  the altitude, a reference clamped at 0.8 (dword `[00CE74F8]`), and mode 2. No mismatch to hand
  over.
* The dive-bomb goaway (`009C4A40`) does not call `009FB800` or `009F9ED0` directly (its host
  seam takes `r.pitch_mode_2d0` from its own rule). This packet did not audit it.
