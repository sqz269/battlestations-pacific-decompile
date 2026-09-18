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

**Not measured. The census this packet owes is blocked by a machine fault, not by the packet.**

Every run on this host now fails before the window with

```
startup failed: FMOD bank raw-length output unavailable: path=sound/gui/error.fsb
bytes=2688 mode=2634 create_result=78 length_result=37 bank_returned=0
```

and the `device_hr=0x80004005` in the run summary is the renderer never starting, not a Direct3D
fault. It began mid-session on an unchanged binary, after two 3000-frame runs had completed on the
same build, and it reproduces on every mission across four clean attempts with no `bsp_game`
process alive, no lock file, the bank file present and all eight audio endpoints reporting OK. It
is not the launcher race fixed in `d8dfc77be`: there is no access violation, no mutex and a clean
exit 1, and it does not clear on retry.

What **is** established: the build is clean at `/W4 /WX` and the existing tests pass, so the
reconstruction compiles and nothing regressed in them.

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
