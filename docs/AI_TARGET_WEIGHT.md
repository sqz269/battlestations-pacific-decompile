# The candidate target weight `00A0F810`, and the stack slots it reuses

Addresses: `00A0F810`, `00A0F821`, `00A0F843`, `00A0F848`, `00A0F84C`, `00A0F859`, `00A0F864`,
`00A0F86A`, `00A0F872`, `00A0F87E`, `00A0F889`, `00A0F891`, `00A0F897`, `00A0F89B`, `00A0F89E`,
`00A0F8A4`, `00A0F8AC`, `00A0F8B5`, `00A0F8C6`, `00A0F8CE`, `00A0F8E6`, `00A0F8EE`, `00A0F8F4`,
`00A0F903`, `00A0F912`, `00A0F929`, `00A0F92F`, `00A0F93D`, `00A0F943`, `00A0F961`, `00A08460`,
`00A04560`, `008DDF90`, `00923BE0`, `00CE38B8`, `00D7A2F0`, `00D228A0`, `00D7A24C`, `00D7A308`.

Packet `cc8_ai_target_weight`, read-only Ghidra analysis. Every descriptive name is a hypothesis,
not a recovered symbol. Reconstruction: `include/bsp/ai_close_attack_tick.hpp`,
`src/ai_close_attack_tick.cpp`. Host: `src/game_hosts_ai.cpp`. Report:
`reports/ai_target_weight.json`.

`00A13B60`'s candidate loop scores every candidate with this routine, and
`ai_close_attack_candidate_admitted` rejects a candidate outright when the weight is not positive
and it is not in the target group. Until this packet the host returned the candidate's class weight
from `009FDF30` and called it a labelled substitution.

## 1. The body, `00A0F810`-`00A0F961`, read in full

Two calls to `00A04560` at `00A0F821` and `00A0F830` build a record for the target (`EDI`) and for
the attacker (`ESI`). `00A0F843` then calls `00A08460 BSP_Ai_TargetWeight` with the four inputs
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` already models as `AiTargetWeightKey`: the attacker entity
from `[ESI]`, the attacker class from `[ESI+10h]`, the target entity from `[EDI]` and the target's
neutral flag from `[EDI+1Ch]`.

Everything after that is four multipliers.

### The stack slots shift, and reading them naively inverts the routine

The two class tests push their class id before an indirect `vtable` call, so `[ESP+n]` means a
different address on either side of them. `tools/stack_frame_walk.py 00a0f810 --indirect-pops 4`
normalises it; without that assumption the walker leaves the frame unknown after `00A0F85B` and
every later offset is four bytes out.

| Site | As written | Depth | Normalised slot | What it holds |
| --- | --- | --- | --- | --- |
| `00A0F848` | `FSTP [ESP+10h]` | 104 | **A** | the raw `00A08460` weight |
| `00A0F864` | `MOVSS [ESP+10h]` | 104 | **A** | zeroed when the attacker is a command building |
| `00A0F889` | `FSTP [ESP+14h]` | 104 | **B** | `attacker+18h * target+18h` |
| `00A0F891` | `MOVSS [ESP+14h]` | 104 | **B** | reused, seeded `1.0f` |
| `00A0F897` | `FMUL [ESP+10h]` | 104 | **A** | the product takes the weight |
| `00A0F89E` | `FSTP [ESP+18h]` | 104 | **C** | the scaled product |
| `00A0F8C6` | `MOVSS [ESP+14h]` | 104 | **B** | `10.0f`, the objective multiplier |
| `00A0F8E6` | `FSTP [ESP+1Ch]` | 104 | **D** | `2.0 - 00923BE0(target)` |
| `00A0F8EE` | `MOVSS [ESP+14h]` | **108** | **A** | reused, seeded `1.0f` |
| `00A0F929` | `MOVSS [ESP+14h]` | **108** | **A** | `0.1f` for the `009FE0B0` trio |
| `00A0F93D` | `MOVSS [ESP+10h]` | 104 | **A** | `0.01f` when that trio is not `1Ch` |
| `00A0F943` | `FLD [ESP+14h]` | 104 | **B** | the epilogue begins |
| `00A0F948` | `FMUL [ESP+14h]` | 100 | **C** | after `POP EDI` |
| `00A0F94F` | `FMUL [ESP+0Ch]` | 88 | **D** | after three more pops |
| `00A0F953` | `FMUL [ESP]` | 88 | **A** | the class multiplier |

**`00A0F8EE` is the trap.** Written as `[ESP+14h]` it looks like the same slot `00A0F8C6` had just
set to `10.0f`, which would make the objective multiplier dead code. It is not: it runs four bytes
deeper, after `00A0F8EA PUSH 1Ch`, and lands on slot **A**, which is free because `00A0F897`
already consumed the raw weight into the product. Slot A is then reused as the class multiplier.

### The four multipliers

* **Slot B, the objective multiplier.** `00A0F87E` compares the local player's party
  (`[00E188A8]+18CCh`, slot 0, `+28h`) with the attacker's `+54h` and picks objective set `0`
  (`game+21A4h`) when they match and set `4` (`game+21B4h`) when they do not. `00A0F8B5` then asks
  `008DDF90 BSP_SzurkeNyil_ContainsUnit` whether the target is in it. A hit raises the multiplier
  from `1.0f` to **`10.0f`** (`00CE38B8`). Note that this caller indexes the eight sets by a party
  comparison, not by the brain's slot as `00A2C450` does.
* **Slot C, the scaled product.** `attacker+18h * target+18h * rawWeight * target+14h`.
* **Slot D, the health term.** `2.0` (the double at `00D7A308`) minus `00923BE0(target)`, which
  the ledger already names `BSP_UnitInstance_GetHealth`. A target at full health contributes
  `1.0` and a destroyed one `2.0`, so the model prefers a damaged target by up to a factor of two.
  `contract: unread` for `00923BE0`'s body.
* **Slot A, the class multiplier.** `1.0f`, or **`0.1f`** (`00D7A2F0`) when the target answers the
  `009FE0B0` trio `IsKindOf(1Bh)`, `IsKindOf(45h)` or `IsKindOf(46h)`, and then **`0.01f`**
  (`00D228A0`) instead when that trio target is **not** also `IsKindOf(1Ch)`, a command building.
  The `0.01f` overwrites the `0.1f`, same slot.

The epilogue multiplies **B × C × D × A**. `coverage: complete` for `00A0F810`.

### The constants

| Address | Bytes | Value | Role |
| --- | --- | --- | --- |
| `00D7A24C` | `00 00 80 3F` | `1.0f` | the seed of both reused slots |
| `00CE38B8` | `00 00 20 41` | `10.0f` | an objective target |
| `00D7A2F0` | `CD CC CC 3D` | `0.1f` | the `009FE0B0` trio |
| `00D228A0` | `0B D7 23 3C` | `0.01f` | that trio, not a command building |
| `00D7A308` | `…00 40` (double) | `2.0` | the base of the health term |

So the model ranks an objective ten times up, and a land fort or its two siblings a tenth down, or
a hundredth down unless it is a command building. That last pair is what changes target choice on a
mission with land structures.

## 2. Host methods

| Site | In | Callee | Host method | this / args | ret |
| --- | --- | --- | --- | --- | --- |
| `00A0F843` | `00A0F810` | `00A08460` | `close_target_weight`, base term only | attacker; class, target, neutral | float |
| `00A0F8B5` | `00A0F810` | `008DDF90` | `close_target_weight`, objective arm | set; target | bool |
| `00A0F8F4` | `00A0F810` | target `vtable[+5Ch]` | `ai_entity_class_matches_009fe0b0` | target; `1Bh` | bool |
| `00A0F903` | `00A0F810` | target `vtable[+5Ch]` | the same | target; `45h` | bool |
| `00A0F912` | `00A0F810` | target `vtable[+5Ch]` | the same | target; `46h` | bool |
| `00A0F92F` | `00A0F810` | target `vtable[+5Ch]` | `units.unit_is_kind_of(target, 0x1C)` | target; `1Ch` | bool |
| `00A0F859` | `00A0F810` | attacker `vtable[+18h]` | (not modelled) | attacker; `1Ch` | bool |
| `00A0F8CE` | `00A0F810` | `00923BE0` `BSP_UnitInstance_GetHealth` | (not modelled) | target | float |

### Substitutions, each labelled

* **The base term.** `00A08460`'s own weight needs the per-barrel reload, accuracy and shot count
  that live in the gunnery host, and the AI coordinator is constructed with the log and the units
  host only. The candidate's class weight from `009FDF30` still stands in for it. The shape and the
  three multipliers around it are the native's; this innermost term is not.
* **The two record factors and `target+14h`.** The AI's per-entity records at `00A04560` are not
  built here, so all three keep the identity value `1.0f` and the product is the base weight alone.
* **The attacker's command-building zeroing** (`00A0F84C`, `00A0F859`) needs the attacker record's
  `+1Ch`, which this process has no producer for, so that arm never runs.
* **`00923BE0 BSP_UnitInstance_GetHealth`** is not read through here, so the health term stays
  `0` and slot D is the constant `2.0`, which is the value a destroyed target would give. The
  model's damage preference is therefore inert in this process, and wiring the units host's
  health would switch it on; that is the cheapest of the four follow-ups.
* **The objective arm is real.** It reads the same eight sets `docs/MISSION_OBJECTIVES.md` fills,
  through the party comparison the listing shows, and answers false on these missions only because
  those sets hold no units.

## 3. Validation

**Superseded on 2026-09-18 by "Correction: the census is measured" at the end of this file. The
three runs completed once the session was reconnected; the paragraphs below record why they were
blocked, and the expected column they set up is scored against the measurement there.**

**Not measured. The census this packet owes is blocked by the environment, not by the packet, and
the cause is known.**

Every run on this host fails before the window. Two failure texts were seen, both from FMOD and
both before the renderer starts:

```
FMOD EventSystem init ... result 61                              (16:01, 16:02)
startup failed: FMOD bank raw-length output unavailable:
  path=sound/gui/error.fsb bytes=2688 mode=2634
  create_result=78 length_result=37 bank_returned=0              (later attempts)
```

**The cause is that the agents run in a remote-desktop session that has been disconnected since
about 15:55, so that session has no audio endpoint.** `Win32_SoundDevice` reporting all eight
devices `OK` is not a contradiction: those exist at the machine level, while what FMOD needs is the
endpoint of session 1. The `device_hr=0x80004005` line in the run summary is the renderer never
starting, downstream of this, and not a Direct3D fault; this packet chased Direct3D first and that
was wrong.

It is also not the launcher race fixed in `d8dfc77be`: there is no access violation, no mutex, the
exit is a clean 1, and it does not clear on retry or after clearing stray processes and the lock
file. This worktree is suspect-free for the earlier crash bisect, which is what the two 11 KB logs
were used for.

No run can start until the session is reconnected, so **every run in this packet is blocked**, and
the census below is a plan rather than a measurement. IJN01 runs first when they work again,
because it is the mission where admission should move.

What **is** established without a run: the build is clean at `/W4 /WX` and both existing ctest
cases pass, so the reconstruction compiles and nothing regressed in them.

### The movement to expect, when the machine runs again

Six runs, the three missions before and after, on the `summary mission ai target weight` line this
packet adds and on the existing coordinator line:

* **IJN01 is the mission that should move.** It carries 239 land forts and 2450 served members,
  each scoring 321 candidates, so `scored` (465500) keeps its count while the ranking under it
  changes: every land-fort candidate drops to a hundredth of its class weight unless it is a
  command building, which should move the `attackmove`/`settarget` split away from its current
  2250/150 and change which units appear in the pilot-attack table.
* **USN02 is the control for ships.** Its fourteen party-0 members face no land forts, so the trio
  never fires and its 616 served and 559 attackmove should be unchanged.
* **USN01 is the null check.** It stays at zero served for the native distance gate established in
  `docs/AI_SQUADRON_SERVED.md`, so nothing here can move it.
* **`objective_hits` should be zero on all three**, because the eight sets hold no units
  (`docs/MISSION_OBJECTIVES.md`), and a non-zero value would mean the objective arm found something
  those measurements say is not there.

## 4. Corrections

* `docs/AI_CLOSE_ATTACK_TICK.md`'s follow-up row `ai_candidate_target_weight` ("the real target
  weight, so the choice stops running on a class-weight stand-in") is **partly answered**: the
  wrapper `00A0F810` is bound with its four multipliers, and only `00A08460`'s innermost term is
  still stood in for. Appended there.

## 5. Follow-up packets

* `00A08460`'s own inputs: the per-barrel reload, accuracy and shot count, and a route from the
  gunnery host to the AI coordinator that does not need a wiring line in `src/game_hosts.cpp`.
* `00A04560`, the per-entity AI record whose `+10h`, `+14h`, `+18h` and `+1Ch` this routine reads.
* `00923BE0 BSP_UnitInstance_GetHealth` into slot D, which turns on the damage preference.
* The attacker's `vtable[+18h]`, which `00A0F859` asks with `1Ch` and which is not the `+5Ch` class
  test the other three use.

## Correction: the census is measured (packet `cc8_ai_target_weight_census`, 2026-09-18)

Section 3's "blocked, not measured" is retracted. The remote-desktop session was reconnected and
all three missions ran. The build is `4baa3f3f0` for every row below
(`build/win32/Release/bsp_game.exe`, 16:33), so the three measured rows share one executable.

### The measured column

`summary mission ai coordinator`, the four order counters and `scored`:

| Mission | Run | served | attackmove | settarget | fallback | scored |
| --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before, `local/obj2_ijn01.log` 15:37 | 2450 | 2250 | 150 | 50 | 465500 |
| IJN01 | after, `local/tw_ijn01.log` 16:54 | 2450 | 2250 | **141** | **59** | 465500 |
| USN02 | before, `local/obj_usn02_after.log` 15:04 | 616 | 559 | 0 | 57 | 4004 |
| USN02 | after, `local/tw_usn02.log` 17:08 | 616 | 559 | 0 | 57 | 4004 |
| USN01 | before, `local/obj2_usn01.log` 15:50 | 0 | 0 | 0 | 0 | 0 |
| USN01 | after, `local/tw_usn01.log` | 0 | 0 | 0 | 0 | 0 |

The three `summary mission ai target weight` lines this packet added, which exist only in the
after runs:

| Mission | queries | objective_hits | fort_targets | non_command | torn_down | model_runs | stand_ins | weapon_rows |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | 465500 | 0 | 460600 | 460600 | 0 | 0 | 465500 | 321 |
| USN02 | 4004 | 0 | 0 | 0 | 0 | 0 | 4004 | 32 |
| USN01 | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 77 |

### Expected against measured

| Expectation (section 3) | Measured | Verdict |
| --- | --- | --- |
| IJN01 `scored` keeps 465500 while the ranking under it changes | 465500 both runs; every candidate re-weighted | **held** |
| IJN01 `attackmove`/`settarget` moves off 2250/150 | 2250 unchanged; 150 to 141, 50 to 59 | **held in the letter, wrong in the cause** — see below |
| USN02 unchanged at 616 served / 559 attackmove | 616 / 559, and every other counter identical | **held** |
| USN01 stays at zero served | 0 across the board, `queries` 0 | **held** |
| `objective_hits` zero on all three | 0, 0, 0 | **held** |

### What moved, and what the movement is not

**The weight itself moved, on IJN01 only, and completely.** `queries` 465500 with `fort_targets`
460600 means 188 of every 190 candidates a served member scores answer the `009FE0B0` trio. The
sharp measured fact the expected column did not predict is that **`non_command` equals
`fort_targets` exactly**: not one trio target in IJN01 is also `IsKindOf(1Ch)`, so **every one of
the 460600 takes the `0.01f` arm and the `0.1f` arm at `00D7A2F0` never fires in this mission**.
The mission's land structures are uniformly a hundredth, not a tenth. USN02 confirms the other
side: `fort_targets` 0, so the trio never fires and the control is exact.

**The 9 order-ticks that moved from `settarget` to `fallback` are not this binding, and attributing
them to it would be wrong.** `ai_close_attack_score` is
`range_factor x target_weight x group_mul x sticky_mul`, and a member takes the fallback moveto
only when `chosen == nullptr`, which needs every admitted candidate to score at or below the
`0.0f` seed. The binding multiplies each candidate's weight by `10.0`, `1.0`, `0.1` or `0.01` —
strictly positive every one, with `target_term` measured at a uniform `1.0` — so a candidate that
scored positive before scores positive after. `best > 0` before therefore implies `best > 0`
after, and the binding **cannot** turn a chosen target into a fallback. It changes the argmax, not
its existence. `scored` holding at 465500 confirms admission did not move either.

The interval `obj2_ijn01` (15:37) to `tw_ijn01` (16:54) carries about thirty commits from four
streams, not just this one: `7610d07ed` the health term, `627f2866e` the binding, `d4d381de8` the
inner weight, `b0ec8d97b`/`b527286ee`/`9d06efce5` the gunnery rows and the muzzle-count divide, and
the dive-bomb and torpedo-run-in work. A plane whose candidates all cross `far_dist` scores zero
and falls back, so any commit that moved aircraft moves this counter. **The 9 ticks belong to that
interval, not to `00A0F810`, and the IJN01 pair is not a clean A/B for this packet.** The counters
in the second table above are, because they measure the binding's own arithmetic directly.

**Second-order lines moved and are likewise unattributable.** IJN01 gunnery went from
`shots=39143 entity_impacts=17 hull=16 deaths=3 total_damage=793.2 first_hit=4.65 s` to
`shots=38349 entity_impacts=104 hull=102 deaths=4 total_damage=1571.8 first_hit=2.45 s`, and
pilot attack from `heading_error_last_mean=0.265` to `0.055` at an unchanged `ordered=33`. Six
times the impacts on slightly fewer shots is the shape of `b527286ee`'s muzzle-count divide
correction, which landed in the same interval. Named here as movement, not as this packet's
result.

**What did not move, and why.** `objective_hits` is 0 on all three because the eight objective
sets hold no units (`docs/MISSION_OBJECTIVES.md`), so slot B is `1.0` everywhere and the `10.0f`
at `00CE38B8` is unexercised. `torn_down_targets` is 0 on all three because `00A13B60`'s candidate
loop drops anything failing `close_candidate_alive` before scoring, so every candidate is live,
takes the full-health `1.0` and leaves slot D at a uniform `1.0`. **Slot D moved nothing in any
mission**, and the earlier "halve every candidate weight" reading (`7610d07ed`, 16:17) was already
retracted in the tree by the comment at `src/game_hosts_ai.cpp:1005` before these runs: the term is
`2.0 - 1.0 = 1.0` for a live target, not `0.5`. Any brief still carrying the halving is stale.

**`model_runs` is 0 on all three with rows published on all three**, which is the gap and not a
plumbing failure. IJN01 publishes `weapon_rows=321`, one per entity in the mission
(`summary world walk entities=321`), so the lookup finds a row for both attacker and target. The
gate at `src/game_hosts_ai.cpp:931` also requires `inputs_complete`, and
`src/game_hosts_gunnery.cpp:2704` holds `barrel.accuracy = 0.0f` with the flag forced false at
`2711`, for the single reason that `009FE270` at `00A094E6` has no producer. **Rows are complete
but for one field.**

### What this census cannot see

The coordinator counters record chosen-versus-none and ship-versus-plane, never **which** target
was chosen. The re-ranking of 460600 candidates to a hundredth is therefore established in the
weight and **not observed in the choice**: the expectation "change which units appear in the
pilot-attack table" is untested, because no line names the picked target. Instrumenting
`close_issue_order` with the chosen target's class is the smallest thing that would close it.
