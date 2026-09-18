# Why a torpedo gun is never given a target

Addresses: 008651B4, 008651F5, 00865442, 00863990, 008633D0, 00862820, 008657C0, 00865809,
00865833, 00729BC0, 00727F10, 00E09D64, 00E0A1F0, 00864FE0.

Packet `cc8_torpedo_gun_assignment`, owner `agent/cc8-torpedo-drop`.

## The answer

`assigns = 0` over every torpedo-carrying gun in USN01 is **not a missing binding**. It is two
different behaviours of the image, one per category, and neither is a host defect. Nothing was
bound, because binding either would contradict the executable.

The packet was dispatched to fix a defect. There is no defect here, so this document is the
deliverable and the host change is the measurement that proves it.

## A plane's torpedo is not a TORPEDO-category gun

This is the distinction the previous packet's reading missed. Counted per run rather than assumed
from the category name, USN01 has 39 guns whose bullet class derived a swim speed, and they sit in
two different categories:

| category | function | swim-capable guns | owners |
| --- | --- | --- | --- |
| 7 | `TORPEDO` | 15 | 5 units |
| 0Ah | `BOMBPLATFORM` | 24 | the torpedo bombers |

So "why do torpedo guns get no target" is two questions with two different answers.

## Category 0Ah, BOMBPLATFORM: the preference list is empty

`score_candidate_00863990` refuses a candidate whose class ranks zero for the category. The rank
comes from the per-category preference row, and the BOMBPLATFORM row at `00E0A1F0` is **all zeros
in the image**:

```
00e0a1f0  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
00e0a200  00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

An empty list ranks every class zero, so `00863990` refuses **every** candidate for category 0Ah,
for every target, on every tick. A bomb platform is therefore never assigned a gunnery target by
design. Its 234327 aim ticks in USN01 are the platform tracking its rest angles, not a gun failing
to shoot.

That is consistent with how aerial ordnance is actually released: not by the gunnery director but
by the bot-task chain `007BBBA0` -> `unit+C20h` -> `007CEA82` -> `007C0D90` -> `007EEF30` ->
`007BCBE0` -> `unit+C58h`, spent at `0099AFB6`. See `docs/TORPEDO_RELEASE_SPAWN.md`.

For contrast, the TORPEDO row at `00E09D64` is populated, eight entries, mothership first:

```
00e09d64  09 00 00 00 0d 00 00 00 0a 00 00 00 07 00 00 00
00e09d74  0c 00 00 00 0b 00 00 00 08 00 00 00 41 00 00 00
```

## Category 7, TORPEDO: no recon sweep, so only a director target

The image cuts the torpedo category out of the recon sweep. In the pass at `00864FE0`:

```
008651b4: cmp dword ptr [esp + 0x14], 7    ; the category index
008651b9: jne 0x8651c0
008651bb: mov eax, ecx
008651bd: mov bl, byte ptr [eax + 0x7c]    ; the torpedo category's own enable
...
008651e7: cmp byte ptr [ebx + 0x59], 0
008651eb: jne 0x865442
008651f1: mov esi, dword ptr [esp + 0x14]
008651f5: cmp esi, 7
008651f8: je 0x865442                      ; category 7 jumps PAST the sweep
008651fe: mov ecx, dword ptr [edi + 0x54]
00865201: call 0x8053c0                    ; the recon contact list
```

`00865442` is step 8.6, where the director's fire target and command target are read. So a torpedo
mount's **only** candidate source is a director-assigned target. It never picks its own from the
recon list the way a machine gun does, which is why the comparison with AAMACHINEGUN in the
dispatch was never going to hold: the two categories do not gather candidates the same way.

USN01 then simply never gives those units a target. Over 15000 pass ticks on units that own a
TORPEDO-category gun, the command target is set on 0 and the fire target on 0, so
`score_candidate_00863990` is called **0 times** for category 7 and every downstream guard is
vacuous. Of the 5 units owning a torpedo gun, 0 have a command target, while 14 other units in the
mission do.

USN02 is the control. There the same code assigns 54869 times and fires 44 torpedoes, all 44 of
which enter the swim. The machinery works; USN01's ship torpedo carriers are simply never ordered
onto anything.

## The guards past the candidate list, read but not reached

None of these is exercised in USN01 because the candidate list is empty. They are recorded so the
path is read to its end.

| address | guard |
| --- | --- |
| `008657C0` | a torpedo-class launcher skips an air target nearer than the ammunition's minimum range |
| `008657F7` | the gun's own slot test, `00729BC0`'s answer |
| `00865809` | the torpedo category refuses the director's own fire target unless `this+7Dh` says otherwise |
| `00865833` | the one call to `00727F10` |

`00727F10` `BSP_Gun_SetBotFireTarget`, `void __thiscall(gun, target)`, `RET 8`. It writes the
target into the gun's six bot slots at `+390h`, `+394h`, `+398h`, `+39Ch`, `+3A0h` and `+3A4h`,
each behind a null test, so a gun whose slot object does not exist silently takes no target. Two
slots are special: `+398h` calls `vtable[30h]` instead when the target answers kind `0Fh`, and
`+394h` calls `vtable[30h]` unless the target answers `0Fh` or `0Eh`. `+39Ch` also gets
`vtable[3Ch]`. The host models the assignment as a single row write and does not reproduce the six
slots, which is a **divergence** and is recorded as one: it cannot matter while the call is never
reached, and it would matter the moment it is.

## Host methods

Instrumentation only. No native address produces these counters and no behaviour changed.

| host method | file | what it measures |
| --- | --- | --- |
| torpedo candidate funnel | `src/game_hosts_gunnery.cpp` | pass ticks, command target, fire target, then one counter per `return false` inside the `00863990` binding |
| swim-capable guns per category | `src/game_hosts_gunnery.cpp` | which category a swim-capable round is actually mounted in |
| TORPEDO-category owners | `src/game_hosts_gunnery.cpp` | how many of those units ever hold a command target |

## Corrections

Appended, not rewritten.

* `docs/TORPEDO_RELEASE_SPAWN.md`, "Why `swims_started` is zero": that section reads `assigns = 0`
  as "the next gate is target assignment", which overstates it. It is a gate, but not a fault:
  category 0Ah cannot be assigned at all because `00E0A1F0` is empty, and category 7 can only be
  assigned from a director target that USN01 never issues. The same section's grouping of TORPEDO
  and BOMBPLATFORM as "failing differently" was right, and this document gives both reasons.
* `docs/BOT_TASK_STATES.md`, "The ordnance release": the door-command reading is reinforced here.
  A bomb platform is never a gunnery target holder, so the release cannot come from the aiming
  path; the bot-task chain is the only producer.

## no_ghidra_function

None. Every routine read in this packet has a Ghidra function.

## Validation

USN01 and USN02, 3200 frames, `--mission-frames 3000` at `0.05` s, through `tools/run_game.ps1` on
this worktree. This packet changed no behaviour, so the before and after runs differ only in the
counters present; the simulation numbers are identical and are not presented as a delta.

### USN01

| | value |
| --- | --- |
| swim-capable guns, category 7 TORPEDO | 15 |
| swim-capable guns, category 0Ah BOMBPLATFORM | 24 |
| TORPEDO-category owning units | 5 |
| of those, with a command target | 0 |
| pass ticks on a TORPEDO-category owner | 15000 |
| those ticks with a command target | 0 |
| those ticks with a fire target | 0 |
| `00863990` calls for category 7 | 0 |
| TORPEDO assigns / shots | 0 / 0 |
| BOMBPLATFORM assigns / aim ticks | 0 / 234327 |
| units with a command target, whole mission | 14 |
| torpedo drops, water crossings, breakups, `swims_started` | 1, 1, 1, 0 |

### USN02, the control

| | USN01 | USN02 |
| --- | --- | --- |
| swim-capable guns, category 7 | 15 | 71 |
| swim-capable guns, category 0Ah | 24 | 0 |
| TORPEDO-category owning units | 5 | 29 |
| of those, with a command target | 0 | 14 |
| pass ticks on such a unit | 15000 | 87000 |
| those ticks with a command target | 0 | 42000 |
| those ticks with a fire target | 0 | 69493 |
| `00863990` calls for category 7 | 0 | 2523 |
| accepted | 0 | 1568 |
| rejected: liveness `00862820` | 0 | 366 |
| rejected: rank, empty-list test | 0 | 0 |
| rejected: mask `008633D0` | 0 | 0 |
| rejected: range | 0 | 589 |
| TORPEDO assigns / shots | 0 / 0 | 55960 / 46 |
| `swims_started` | 0 | 46 |

Every guard in the category-7 path passes at a healthy rate in USN02, and 46 of 46 torpedo shots
enter the swim. The rank and mask tests reject nothing there, which is the positive control for the
BOMBPLATFORM finding: an empty preference row is what makes the rank test bite, and category 7's
row is not empty.

USN02 also has **zero** swim-capable guns in category 0Ah, matching its own run log line that no
ordered aircraft in that mission carries torpedo ordnance. So the two missions between them
separate the two categories cleanly.

One caveat on precision: an earlier USN02 run on this tree reported 44 swims where this one
reports 46. The runs are not bit-identical, so single-digit differences in these counts are run
variation, not signal.

## Follow-up packets

1. **The six bot slots of `00727F10`.** The host writes one row where the image writes up to six
   slot objects with two kind tests. Harmless today, wrong the moment a torpedo gun is assigned.
2. **Ship torpedo orders in USN01.** Whether the mission script ever intends its 5 torpedo carriers
   to engage, and if so which command row should reach them. This is a mission-content question,
   not a gunnery one.
3. **The air-drop gate that actually matters.** Release altitude, from
   `docs/TORPEDO_RELEASE_SPAWN.md`: 700 m gives a 117.2 m/s entry against a 100.0 m/s
   `MaxWaterHitVel`. That, not target assignment, is what keeps an aerial torpedo out of the swim.
