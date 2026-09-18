# Why the close-attack pass served nobody on IJN01 and USN01

Addresses: `00A143ED`, `00A14402`, `00A1440C`, `00A14413`, `00A14427`, `00A1443D`, `00A13B60`,
`00A2C450`, `008DDF90`, `00A181A0`, `00A18253`, `00A18258`, `00A18262`, `00A18269`, `00A1826B`,
`00A18273`, `00A22750`, `00A2C5A0`, `00A2E260`, `00A2E2E7`, `00A2E3A2`, `00A2E412`, `00A2E42A`,
`00A1CB80`, `00A179E0`, `00A2E835`.

Packet `cc8_ai_squadron_served`, read-only Ghidra analysis. Every descriptive name is a hypothesis,
not a recovered symbol. Host: `src/game_hosts_ai.cpp`. Report: `reports/ai_squadron_served.json`.
Predecessor: `docs/PLANE_SQUADRON_ENTITY.md`, which built the squadrons this packet expected the
pass to serve.

**The answer is not the member gate and not the squadron.** The gate admits squadrons exactly as it
should; the group the gate was asked about held no squadron and no ship. One line of
`src/game_hosts_ai.cpp` decided that, and it was a labelled substitution for `00A2C450`.

## 1. The member gate is innocent

`00A143ED`-`00A14413` is the squadron arm of `00A13B60`'s member test: `PUSH 18h` through the
member's `vtable[+5Ch]`, then `007EDA90`'s shape inline (`00A14402 PUSH 17h`, `00A1440C` the byte
at `+C24h`, `00A14413 JE` skipping the member). The other arm is `00A14427 PUSH 6`, the ship base,
and `00A1443D`, the controller-busy test through `vtable[+2Ch]`.

Instrumented per member on IJN01, one line per member with every input's value
(`GameAiCoordinatorHost::Impl::diag_close_member`, bounded to 48 lines):

```
ai diag close member=AirField 01      squadron_18h=0 excluded_007eda90=0 ship_base_6=0 busy=0 served=0
ai diag close member=Airfield1Hangar  squadron_18h=0 excluded_007eda90=0 ship_base_6=0 busy=0 served=0
ai diag close member=Static warhawk 06 squadron_18h=0 excluded_007eda90=0 ship_base_6=0 busy=0 served=0
```

Every member of the group the pass was given is a land structure or a static prop: not a squadron,
not a ship base. The gate is correct and refuses all thirty. `007EDA90` never even runs, because
the `18h` test that guards it is false. **The refusing input is `squadron_18h=0` together with
`ship_base_6=0`, and its producer is not the gate: it is whichever group the planner ordered.**

## 2. The group the planner ordered

`00A181A0 BSP_AiPartyBrain_Think`, the per-party walk, `00A18230`-`00A18284`, read in full:

```
00a18238  if (group->+5644h == 0) continue          ; empty group
00a18243  CALL 00A2E260                             ; the split, unconditional
00a18248  if (group->+5654h != 0) continue          ; already claimed
00a18253  CALL 00A2C5A0                             ; has a groupable combatant?
00a1825a  JZ  00a18270      -> ECX = brain+0h       ; no  -> the FIRST planner
00a1825c  ECX = brain+24h                           ; the brain's world-set index
00a18262  CALL 00A2C450                             ; a member in that world set?
00a18269  JNZ 00a18270      -> ECX = brain+0h       ; yes -> the FIRST planner
00a1826b                       ECX = brain+0Ch      ; no  -> the FOURTH planner
00a18273  CALL 00A22750                             ; claim the group with it
```

`00A22750` appends to the planner's owned list (`00A2276C`-`00A22793` insert at the tail
`[planner+24h]`, no ordering key), and `00A1CB80`'s mode tick orders only the **first** owned
group. So when one planner owns two groups, the second is never ordered.

`00A2E260`, the split, `00A2E2E7`-`00A2E42A`, read in full: the members for which `009FE080` holds
are collected (`00A2E2EE JZ` skips the rest), each collected member is then **removed** from the
original group (`00A2E3A2 CALL 0077BEA0` on `group+563Ch`, `00A2E3AF` clearing its back pointer at
`+16Ch`), and they are given a **new** group (`00A2E40D PUSH 5660h`, `00A2E42A CALL 00A2DFA0` for
the first, `00A2E447 CALL 00A2D8E0` for the rest). The original keeps the **non-groupable**
remainder, and it keeps its claim and its command object.

That is the trap. On IJN01 the party's single seeded group splits into a 49-member combatant group
and a 30-member remainder of airfields, hangars and static props. The remainder is the original, so
it is the one already in the team list ahead of the new group, and it is claimed first.

## 3. The line that decided it

Before this packet, `GameAiCoordinatorHost::Impl::group_has_member_in_world_set` answered

```cpp
return g != nullptr && g->team == set_index;   // the old stand-in for 00A2C450
```

and `brain_world_set_index` returns the brain's party slot, so for every group a brain walks the
two are equal and the answer is **always true**. `00A18269 JNZ` then sends every group of the party
to the first planner. Measured (`ai diag planner`, bounded to 24 lines):

```
ai diag planner kind=1 owned=2 first_group_members=30 first_leader=AirField 01 ...
ai diag planner kind=0 owned=0 ...
ai diag planner kind=3 owned=0 ...
ai diag planner kind=4 owned=0 ...
```

One planner owned both groups, its first was the 30-member remainder, and `00A1CB80` ordered that
one alone. The 49-member group holding every ship and every squadron stayed `IDLE` for the whole
mission.

### What `00A2C450` actually does, and what it answers here

`00A2C450 BSP_AiGroup_HasMemberInWorldSet`, `__thiscall(group, int setIndex) -> bool`, `RET 4`,
body `00A2C450`-`00A2C4B7`, read in full. It walks the member list at `+563Ch`/`+5640h` and for
each member calls `008DDF90 BSP_SzurkeNyil_ContainsUnit` against the entity set
`[00E188A8] + setIndex*4 + 21A4h` (`00A2C489 MOV ECX,[ECX+EBP*4+21A4h]`), returning `1` at
`00A2C4AB` on the first member found in it and `0` at `00A2C4B4` when the walk ends.

**This process builds no entity set at `world+21A4h`.** Running the native routine against an
empty set finds no member, so its answer here is `false` — and `false` is the one answer the old
stand-in could never give. The fix is therefore not a guess: it is what `00A2C450` computes when
the table it reads is empty.

```cpp
return false;   // 00A2C4B4, the walk-ended arm, against an empty set
```

`coverage: complete` for `00A2C450`; `contract: unread` for `008DDF90` beyond
`docs/ATTACK_CAPABILITY_INPUTS.md`'s note, and for the producer of the sets at `world+21A4h`.

## 4. What this leaves standing

Three things this packet checked and did **not** change, because they are native:

* **`00A1CB80` orders only the first owned group.** That is what the routine does. With the fix
  each planner owns one group, so the cap costs nothing on these three missions; it remains the
  open question `ai_planner_owned_group_walk` in `docs/AI_COMMAND_LIFETIME.md`.
* **`00A2E260` leaves the non-groupable remainder in the original group.** Confirmed from the
  listing, and the host already matched it.
* **`00A179E0 BSP_AiPartyBrain_EngagementPass`** (body `00A179E0`-`00A18195`) was the other
  candidate producer. Its callee list is `BSP_AiGroup_TotalLeaderWeight`,
  `BSP_EntityPose_RefreshWorld`, `BSP_Math_MaxFloatByRef`, `008E35F0`, `008EA0C0`, `008EADA0`,
  `00A046C0`, `00A04860`, `00A04910`, `00A0F680`, `00A16B60` and list/vector helpers. **It calls
  neither `00A2CBD0` nor `00A22750`**, so it issues no order and claims no group, and it is not
  the producer. It stays `contract: unread`.

One substitution is named and not fixed: `00A2E835`, the five world collections at `world+19CCh`,
is still stood in for by one flat collection, which is why a party's structures and combatants are
seeded into the same group at all. With `00A2C450` answering correctly the split and the two-planner
split-out absorb that, but the seed substitution remains the reason the remainder group exists.

## 5. Host methods

| Site | In | Callee | Host method | this / args | ret |
| --- | --- | --- | --- | --- | --- |
| `00A18262` | `00A181A0` | `00A2C450` | `group_has_member_in_world_set` | group; brain `+24h` | bool |
| `00A2C491` | `00A2C450` | `008DDF90` | (no host call: the set is empty) | set; member | bool |
| `00A18253` | `00A181A0` | `00A2C5A0` | `group_has_groupable_combatant` | group | bool |
| `00A18273` | `00A181A0` | `00A22750` | `planner_claim_group` | planner; group | void |
| `00A18243` | `00A181A0` | `00A2E260` | `split_detached_members` | group | void |
| `00A143ED` | `00A13B60` | member `vtable[+5Ch]` | `close_member_is_plane_squadron` | member; `18h` | bool |
| `00A14427` | `00A13B60` | member `vtable[+5Ch]` | `close_member_is_ship_base` | member; `6` | bool |
| `00A1443D` | `00A13B60` | member `vtable[+2Ch]` | `close_member_controller_busy` | member | bool |

## 6. Validation

`./tools/run_game.ps1`, 3200 frames, `--mission-frames 3000 --mission-frame-seconds 0.05`.
"Before" is this branch at `7c1036d5f`, the commit `cc8_plane_squadron_entity` produced, whose logs
are `local/<mission>_after.log`; "after" is `local/<mission>_served.log`.

### The close-attack pass

| Mission | | `served` | `attackmove` | `settarget` | `scored` | attack orders | claims | promotions |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 0 | 0 | 0 | 0 | 1 | 2 | 1 |
| IJN01 | after | 2450 | 2250 | 57 | 465500 | 2 | 2 | 2 |
| USN01 | before | 0 | 0 | 0 | 0 | 1 | 2 | 1 |
| USN01 | after | 0 | 0 | 0 | 0 | 2 | 2 | 1 |
| USN02 | before | 616 | 559 | 0 | 4004 | 1 | 1 | 1 |
| USN02 | after | 616 | 559 | 0 | 4004 | 1 | 1 | 1 |

### The squadrons, and what reached the aircraft

| Mission | | squadrons | in groups | `007EDA90` true | squadron commands | member orders | aircraft ordered | mean distance closed |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 33 | 33 | 0 | 141 | 141 | 33 | 0.0 |
| IJN01 | after | 33 | 33 | 0 | 54 | 54 | 33 | 0.0 |
| USN01 | before | 20 | 20 | 0 | 477 | 477 | 14 | 1435.0 |
| USN01 | after | 20 | 20 | 0 | 459 | 459 | 14 | 1186.3 |
| USN02 | before | 0 | 0 | 0 | 0 | 0 | 0 | None |
| USN02 | after | 0 | 0 | 0 | 0 | 0 | 0 | None |

### Gunnery

| Mission | | candidates | gun evaluations | assigns |
| --- | --- | --- | --- | --- |
| IJN01 | before | 1841 | 2776 | 2766 |
| IJN01 | after | 1957 | 3044 | 3018 |
| USN01 | before | 1059 | 1082 | 1082 |
| USN01 | after | 1363 | 1205 | 1205 |
| USN02 | before | 3135 | 5146 | 5146 |
| USN02 | after | 3135 | 5146 | 5146 |


### USN01 still serves nobody, and that one is native

With `00A2C450` answering correctly, USN01's combatant group **is** claimed by the fourth planner
and **is** attack-ordered:

```
ai diag order_attack group_members=16 group_leader=Enterprise target_members=18 target_leader=Katori cautious=0
```

Its command stays `MOVETOATTACK` for the whole mission, and `00A13B60` runs only under
`CLOSEATTACK`. `00A12A90` promotes only when the leader stops being a groupable combatant or the
two leader points close to within `CloseAttack_CollectDist`. Instrumented every tick:

| | first tick | last tick | gate |
| --- | --- | --- | --- |
| leader-point separation | 16889.3 m | 16044.8 m | `collect_dist` 3000.0 m |
| leader groupable | 1 | 1 | must be 0 to force the promotion |
| members | 16 | 16 | |

The group closes about 845 m in the 150 s of mission time the run simulates, from a start almost
five and a half times the collect distance. **This is a native gate the mission fails, not a
substitution**: nothing here is stood in for, and the packet stops at the value rather than
changing the rule.

IJN01's other group is the same rule on its other arm: `ai diag movetoattack leader=Storage, 05 01
dist=9792.3 collect=3000.0 groupable=0 members=8`. A non-groupable leader promotes at once, and
`00A13B60`'s member gate then correctly refuses all eight.

### Attribution

* **IJN01, `served` 0 to 2450**, `attackmove` 0 to 2250, `settarget` 0 to 57 and
  `candidates_scored` 0 to 465500: entirely the `00A2C450` correction. The combatant group moved
  from the first planner to the fourth, was ordered, closed to within `collect_dist` and was
  served. `attack_orders` went from 1 to 2 and `promotions` from 1 to 2, one per planner.
  `squadron_commands` FELL from 141 to 54 on the same mission, which is not a loss: before the fix
  those were follower `moveto` orders from the one commanded group, and after it the squadrons are
  being served attack orders by `00A13B60` instead, which the `attackmove` column counts.
* **USN01, `served` unchanged at 0**: the correction worked (`attack_orders` 1 to 2, the Enterprise
  group ordered), and the distance gate above is what stops it.
* **USN02, everything unchanged**: `served=616`, `attackmove=559`, `commands=635`,
  `groups_created=2`, `members_added=50` before and after. Its single party-0 group has a groupable
  combatant and never splits, so the world-set answer only moves it from the first planner to the
  fourth and the same planner ticks it either way. This is the regression evidence.
* **The squadrons themselves are served.** On IJN01 the per-member instrumentation shows
  `member=Warhawk1 squadron_18h=1 excluded_007eda90=0 ship_base_6=0 busy=0 served=1`, and the same
  for `Dauntless1`, `B-17 01` and `B-17 02`.

### USN01's gunnery drop is not the squadron work

Asked whether the squadron packets caused USN01's gunnery census to fall to zero. **It is not
zero in any configuration this worker can run, and the squadron work moves it the other way.**
Four builds of the same mission, same 3000 mission frames:

| Build | shots | projectiles | entity impacts | water | hits | damage | first hit |
| --- | --- | --- | --- | --- | --- | --- | --- |
| `f5400e43f`, before both packets | - | - | - | - | 23 | 220.0 | 4.70 s |
| `7c1036d5f`, the squadron packet | - | - | - | - | 36 | 241.0 | 4.70 s |
| `2eb86d8c4`, plus the served fix | 14676 | 14676 | 45 | 42 | 45 | 563.5 | 4.70 s |
| `8f3370237`, main alone | 13699 | 13699 | 10 | 42 | 10 | 313.4 | 58.60 s |
| `af098961a`, main merged into this branch | 13823 | 13828 | 10 | 166 | 10 | 346.0 | 58.60 s |

Read down the two packets' own column: 23 hits become 36 and then 45, and 220.0 damage becomes
563.5. Neither packet removed a hit from that mission.

The fall from 45 to 10 belongs to work merged into `main` between `f5400e43f` and `8f3370237`,
and it reproduces on **main alone**, without this branch's served fix. Its shape is not a
targeting change: the guns still fire almost as much (13699 shots against 14676, a 7% drop), but
of the shells that land, entity impacts fall from 45 to 10 while the shot count barely moves.
First hit slips from 4.70 s to 58.60 s and the mission's one kill disappears
(`deaths=1 kill_credits=1` becomes `deaths=0 kill_credits=0`), as does the contact census's
`dead=2840`, which becomes `dead=0` because nothing dies. That is projectiles missing, not guns
refusing to shoot.

Merging `main` into this branch changes only the damage total, 313.4 to 346.0, and moves the
aircraft's mean closure from 92.3 m to 1342.6 m: the served fix gives the combatant group an
attack order it did not have, which shifts the firing geometry (`arc_blocks` 0 to 1100, water
impacts 42 to 166) without changing the impact count.

**Verdict: a regression in what the shells can hit, introduced on `main` by something other than
these two packets.** It is not an artefact of aircraft flying attack tasks instead of strafing
paths, because the shot count is nearly unchanged and the losses are all on the impact side.
Locating it needs a bisect over `main` between those two commits, which this packet did not do.
Logs: `local/usn01_before.log`, `local/usn01_after.log`, `local/usn01_served.log`,
`local/usn01_mainonly.log`, `local/usn01_merged.log`.

## 7. Follow-up packets

* `00A2E835`, the five world collections at `world+19CCh`: the remaining reason a party's
  structures and combatants seed into one group.
* The producer of the entity sets at `world+21A4h`, and `008DDF90 BSP_SzurkeNyil_ContainsUnit`.
  With those built, `00A2C450` could answer for real rather than by the empty-set arm.
* `00A1CB80`'s owned-group walk: whether a planner that owns several groups orders only the first
  natively, still open.
* `00A179E0 BSP_AiPartyBrain_EngagementPass`, body `00A179E0`-`00A18195`, still unread.
* `00A1443D`, the controller-busy test through the member's `vtable[+2Ch]`, still a neutral value.

## Correction from docs/AI_WORLD_SETS.md

Appended by packet `cc8_ai_world_sets`. The text above is left as written, and the answer it
installed is unchanged and still correct.

Section 3 says of `00A2C450` that "this process builds no entity set at `world+21A4h`". **It does
build them.** `004DF90F`-`004DF959` in `BSP_Game_ConstructWorld` allocates exactly eight, `30h`
each, constructs each with `008DF900(set, i)` carrying the loop index, stores the pointer into
`game+21A4h + i*4` and calls `008DA160`; `004DE20A`-`004DE234` in `BSP_Game_ConstructActualStorage`
only nulls the same eight slots. An exhaustive census of every addressing form of the displacement
`21A4h`..`21C0h` in `.text` (50 sites, positive control `004DE20A`) finds no other writer.

They are the **per-player-slot objective sets**, class string `"SzurkeNyil"` at `00D16100`, vtable
`00D1610C`, holding `Objective` records whose own `+20h` unit lists carry the units. The right
reason the answer here is `false` is that their producers, the mission Lua bindings
`008CD440 Objectives_Add` and `008CDD60 Objectives_AddUnit`, are **unimplemented in this process**,
so every set is empty and `008DDF90` finds nothing. The host no longer hardcodes the `false`: it
walks a real eight-slot table that nothing fills yet.
