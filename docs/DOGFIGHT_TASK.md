# The dogfight task: its state map, and a skeleton binding

Addresses: 009A9810, 009A94E0, 009AB1C0 (no Ghidra function), 009AAFA0, 009AAC70 (not read),
009A76E0, 009A7E80, 009A80E0, 009A84E0, 009A70E0, 009C2CA0.

Packet `cc9_dogfight_task`. USN04's script gives six fighters (`Lexington-class01_sqn01` ×3,
`Yorktown-class01_sqn02` ×3) the `dogfight` command. In the baseline they have the task but no
state, and all six are among the sixteen water contacts, at |v| 82 m/s.

## 1. The task object

* **Constructor `009A9810`** (`BSP_BotTaskDogfight_Construct`). This is the generic nine-step
  derived constructor (`docs/BOT_TASKS.md`), except that step 9 (`009A9874`, `009F9980`) runs
  before step 6 (`009A987F`, `007B8AD0`). Then `task+310h` is `task+510h` (moveto) for the
  flight leader, else `task+54Ch` (follow), at `009A9886`/`009A988E`. Why step 9 comes first
  was not established.
* **States**, built by `009A94E0` with `ESI = task+3F8h`, named by
  `009A6A60 BSP_BotTaskDogfight_RegisterStateNames`:

| state | task offset | built at | vtable / tick | status |
| --- | --- | --- | --- | --- |
| moveto | `+510h` | `009C2CA0` | `00D20B24`: tick `009C18C0` (the generic moveto tick), speed slot `+1Ch` = `009C1BC0` (not `009C1850`) | tick read in earlier packets; the dogfight speed slot and cruise profile (`009AAF30`) are **unread** |
| follow | `+54Ch` | `009C2980` `BSP_BotStateFollow_Construct` | the generic follow, tick `009C1FD0` | **read** (the follow-law packets) |
| prepare | `+5E4h` | `009C2980` | a second follow object | read as follow; the entry effects are unread |
| aim | `+67Ch` | vtable stored at `009A95B4` | `00D1F8D8`, tick `009A76E0` | **partial**: calls `009F9ED0` at `009A78BB` (`docs/PITCH_COMMAND_CALLERS.md`) |
| maneuver | `+6A4h` | `009A84E0` | not read | **unread** |
| attackrun | `+6E4h` | `009A70E0` | not read | **unread** |
| avoid_roll | `+708h` | vtable `00D1F918` | tick `009A7E80` | **unread** |
| avoid_turn | `+730h` | vtable `00D1F938` | tick `009A80E0` | **unread** |

* **The per-tick arm** is primary vtable `00D1F9B0` slot `+64h` = `009AB1C0`, which has no
  Ghidra function (`ghidra proto` finds none; INT3 follows at `009AB214`). It does five things:
  1. `task+4F8h = FFh`.
  2. `009AAC70(approach = task+3F8h, dt)`, which reads `+644h` AttackDist (**not read**).
  3. `009AAFA0(task, dt)`, the transitions.
  4. The state tick through `task+310h`'s vtable slot `+0Ch`.
  5. `task+2E4h = task+4F8h`.

## 2. The transitions, 009AAFA0 (read from the decompilation)

`ENG = task+4C8h != 0 || (unit+370h == 2 && task+4C4h != 0)`

* **From moveto or follow:** `ENG` calls `009A9D90`, the engage entry, which was not read.
  Otherwise the task goes to moveto if `007B8AD0` (flight leader), else follow.
* **From any other state, when not `ENG`:** moveto or follow by the same test.
* **From any other state, when `ENG`:**
  * `unit+370h == 0` switches to prepare (`009A9D50` is the state switch).
  * Otherwise, when neither the current state is aim nor `009AAA80` says no, the task switches
    to aim.
  * Otherwise it goes by state:
    * **attackrun:** to maneuver when `+4C8h` is set, clearing `+6BCh` and `+6D8h`.
    * **aim:** to `009A9970()`'s choice when `+6A0h` is set, else to maneuver when `009A98F0()`
      is true (calling `009A8560`).
    * **maneuver:** to aim when `009A9BD0()` is true.
    * **avoid_roll / avoid_turn:** to maneuver when their timer (`+720h` / `+748h`) goes
      negative, calling `009A86F0`.

`009AAA80`, `009A9970`, `009A98F0`, `009A9BD0`, `009A8560`, `009A86F0` and `009A9D90` were
identified by address only.

## 3. Target selection

It lives in `009AAC70` (the approach update, AttackDist 2000). **Not read** this packet. The
host's per-unit command target (the script's `dogfight` token's target, a `D3A Val` in USN04)
is what the stand-in heads at.

## 4. The binding (kDogfightTaskBound = true)

* `include/bsp/dogfight_task.hpp` / `src/dogfight_task.cpp`: the state enum,
  `dogfight_engaged_009aafa0`, `dogfight_unengaged_state_009aafa0`, and
  `dogfight_moveto_standin`.
* In the host, `run_dogfight_task_arm_009ab1c0` installs on class `00E08F58` **or** on the
  director's resolved command token `dogfight`. The token is a labelled SUBSTITUTION for
  the image's task build through `0099A170`, because a scene-issued order never sets the
  class in this host (run G1). It then applies
  `009AAFA0`'s unengaged arm (the leader in moveto, wing members in follow). Follow runs the
  generic follow tick: station placement, then the fly-to law with its pitch through
  `009F9ED0`. Moveto is a **labelled stand-in**: head at the command target (`009F9E40`) and
  pitch toward CruisingAlt (1400) through `009F9ED0` over at least 250 m, mode 2 on both axes.
  No speed command, because `009C1BC0` is unread.
* **Not bound:** the engaged arm (it needs `009AAC70`'s `+4C8h` latch), and prepare, aim,
  maneuver, attackrun and both avoid states. A per-unit census counts ticks within AttackDist
  of the target, which is where the image would start engaging.

## 5. Predictions, written before run G1

Control G0 (main `0899b3bb2`) was already running when this was written.

1. **None of the six fighters drowns.** The leaders climb toward 1400 m and hold heading on
   their targets. The wing members are placed on the leaders' stations. Water contacts go from
   16 to 10: the six fighters leave the list, and every other unit's contact is unchanged.
2. **State sequences:** one `none -> moveto` for each leader and one `none -> follow` for each
   wing member, with no further transitions.
3. **Engagement:** the census reports how close each leader comes to its target. Nothing
   engages, because the engaged arm is unbound.
4. **Strikers and dive-bomb rows unchanged**, unless the fighters' guns hit a Val through the
   gunnery host. Any change in `gunnery damage`, deaths or the dive-bomb rows is attributed by
   unit before landing.

## 6. Runs

All USN04 runs use the E2 parameters (`--frames 9200 --press-start-frame 30 --menu-select USN04
--mission-frames 9000 --mission-frame-seconds 0.05`), each from its own copied binary. USN01's
script issues no `dogfight` (its only token is `artillery`), so there is no USN01 pair.

| run | configuration | binary | log | fighters drowned | water contacts | dive-bomb rows | gunnery kills |
| --- | --- | --- | --- | --- | --- | --- | --- |
| G0 | main `0899b3bb2` | `local\binG0` | `local\G0_usn04.log` | 4 of 6 | 11 | - | 11 |
| G1 | install gated on class `00E08F58` only | `local\binG1` | `local\G1_usn04.log` | 4 of 6 | 11 | identical | 11 |
| G2 | install on the class **or** the command token `dogfight` | `local\binG2` | `local\G2_usn04.log` | **0 of 6** | **7** | identical | 9 |

**The baseline had moved.** Section 5 was written against the old baseline of 16 contacts with all
six fighters among them. On main `0899b3bb2` (G0) the count is 11, and four of the six fighters
are among them: `Yorktown-class01_sqn02` ×3 and `Lexington-class01_sqn01|.-2`, all at |v| 82.

**G1 is a null.** The six fighters never get `attack_command_class`: their order is scene-issued,
and the per-unit table's `dogfight` is the director's command token (`row.command`). So the class
gate never fired, and G1 is identical to G0 in all 84 summary rows. G2 adds the token as a
labelled substitute trigger (section 4).

**G2 against G0, term by term:**
* **Dogfight rows.** Six `none -> moveto|follow` transitions, as predicted, one per aircraft.
  The two leaders hold moveto for 4214 ticks and the four wing members follow.
  `Yorktown-class01_sqn02` comes within 104.7 m of its target and spends 366 ticks inside
  AttackDist, where the image would engage. `Lexington-class01_sqn01` never comes closer than
  18190.5 m.
* **Water contacts 11 -> 7.** The four fighter drownings are gone. The remaining seven are G0's
  seven Val rows, at identical speeds.
* **Dive-bomb rows** (`divebomb`, `release census`, `db aim exit`, `gunnery: bomb drop`):
  identical.
* **Gunnery 11 -> 9 kills.** The per-unit `killed_by` table shows every kill in both runs is ship
  anti-aircraft fire on Japanese aircraft; no fighter scores or is killed. Two Kates that died in
  G0, `B5N Kate #6.1|.-2` (Lexington-class01, 230.41 s) and `#6.1|.-3` (Fletcher-class02,
  422.02 s), survive in G2. Four other kills move by at most 0.35 s and the damage columns
  change. The mechanism shows in the recon row: `observers` 7774 -> 7856. The six fighters now
  live the whole mission and observe, which changes recon identification (`identified` 5822 ->
  5560, `none` 1838 -> 2212) and so the ships' anti-aircraft picture. The contact counts
  (`dead` 113375 -> 102333) fall for the same reason: four fewer dead aircraft linger in
  contact lists.
* **Plane-motion totals and the `pilot attack` ordered row** change because the six fighters now
  fly all mission. `worst_closed` -8957.7 m is `Lexington-class01_sqn01`, whose target moves away
  faster than the stand-in closes.

## 7. Decision

**The skeleton lands** (`kDogfightTaskBound = true`, token trigger included). The measurable goal
is met: no fighter drowns. Every other change is attributed above to the fighters now being
alive, and nothing a fighter does in its own right changes another unit.

What remains owed, in order:
1. `009AAC70`: the approach update. It holds target selection and the `+4C8h` in-range latch
   the engaged arm needs.
2. `009A9D90` and the engaged states: prepare, aim (`009A76E0`, partial), maneuver, attackrun,
   avoid_roll and avoid_turn.
3. The dogfight moveto's speed slot `009C1BC0` and cruise profile `009AAF30`, which would retire
   the moveto stand-in.
4. Where the image turns a scene-issued `dogfight` order into the kind-2 task, which would retire
   the token trigger.
