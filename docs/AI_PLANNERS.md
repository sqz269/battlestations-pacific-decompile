# AI planners and AI command classes

Addresses: 00A15A70, 00A1EE50, 00A1EFC0, 00A1F0F0, 00A1F220, 00A1F390, 00A1F500, 00A1F630,
00A1F760, 00A1F890, 00A28A60, 00A1CF90, 00A22800, 00A29FD0, 00A25F70, 00A26210, 00A26510,
00A265F0, 00A18480, 00A1D110, 00A1D140, 00A1D170, 00A1D1A0, 00A1CB80, 00A1C8B0, 00A2CB10,
00A2CBD0, 00A179E0, 00A10710, 00A10890, 00A109B0, 00A10AE0, 00A0FF60, 00A11F80, 00A12450,
00E0E308, 00F8AA48

This packet reads the half of `docs/AI_GROUP_THINK.md` that doc left analysed only: the eight
planner classes behind `brain+0h`..`+1Ch`, the sixteen AI command classes, the brain's planning
tail `00A179E0` and the filler of the `00F8AA60` list. Ghidra was read-only for this packet.
Descriptive names below are hypotheses. Two families of names are **genuine string literals** in
the image and therefore recovered: the eight planner names and the fifteen command type names.

## Headline: the eight planners are named in the image

Each planner constructor stores its own name in a native string at `planner+14h`, and a second
literal `"<name> Coordinator"` sits beside each vtable. The brain has eight slots but only ever
fills four of them: `00A15A70` branches on `BSP_Game_GetEffectiveGameMode` (`004BCA50`) and for
modes 4 through 7 constructs **exactly one** mode planner, leaving `brain+0h`..`+0Ch` null; for
every other mode it constructs the first four and leaves `brain+10h`..`+1Ch` null.

| Slot | Name | Mode | Vtable | Ctor | Think (vt +20h) | +30h | Size | Issues |
| --- | --- | --- | --- | --- | --- | --- | --- | --- |
| `+0h` | `Defend` | 0..3 | `00D22D44` | `00A1EFC0` | `00A28A60` | `00A18480` | `38h` | `DEFENDPOSITION`, `PATROLTO` |
| `+4h` | `Attack` | 0..3 | `00D22D94` | `00A1F0F0` | `00A1CF90` | `00A18480` | `38h` | via `00A2CBD0` |
| `+8h` | `Sell` | 0..3 | `00D22DE4` | `00A1F220` | `00A22800` | `00A18480` | `38h` | `SELLING` |
| `+0Ch` | `Capture` | 0..3 | `00D22E34` | `00A1F390` | `00A29FD0` | `00A18480` | `44h` | not read |
| `+10h` | `Duel` | 4 | `00D22E7C` | `00A1F500` | `00A25F70` | `00A1D110` | `38h` | `MOVETO`, via `00A2CBD0` |
| `+14h` | `Escort` | 5 | `00D22EC4` | `00A1F630` | `00A26210` | `00A1D140` | `38h` | `PATROLTO` x2, via `00A2CBD0` |
| `+18h` | `Siege` | 6 | `00D22F0C` | `00A1F760` | `00A26510` | `00A1D170` | `38h` | via `00A2CBD0` |
| `+1Ch` | `Competitive` | 7 | `00D22F54` | `00A1F890` | `00A265F0` | `00A1D1A0` | `38h` | via `00A2CBD0` |

The mode numbers are `004BCA50`'s space. `009FFC80 BSP_Ai_EffectiveGameModeIndex`, which selects
the tuning record, uses 3, 4, 5, 6 for the same four modes, so the tuning sub-tables
`DuelParams`, `EscortParams`, `SiegeParams`, `CompetitiveParams` of
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md` line up with `Duel`, `Escort`, `Siege`, `Competitive`
one index lower.

`Sell` is not a misread: `"Sell"` sits at `00D22DDC`, `"Sell Coordinator"` at `00D22E18`, and the
planner's think constructs a `SELLING` command. Skirmish has resource points (`00D227A8`,
`"QQ--- Skirmish AI try compose attacker group from %d resource point against %d unit: \n"`), so
the AI can scrap units back.

### Rate

Every one of these ticks on the party-think clock of `docs/AI_GROUP_THINK.md`: `00A182C0` ->
`00A181A0`, once per party every 3 to 5 s. For modes 0..3 all four of `brain+0h`..`+0Ch` tick in
slot order on every think. For modes 4..7 the single mode planner's tick is the whole think, and
`00A15970`'s immediate-think override can additionally fire it before the timer through `vt +30h`.

## The planner object (`00A1EE50`, complete)

`__thiscall(planner)(brain)`, `RET 4`. Every derived constructor calls it first and then
overwrites `+0h`.

| Offset | Meaning |
| --- | --- |
| `+0h` | vtable; the base installs `00D22D08`, the derived class its own |
| `+4h`, `+8h`, `+0Ch` | zeroed; no producer or consumer found in this packet |
| `+10h` | byte, zeroed |
| `+14h`, `+18h` | native string: length, data. The planner's name |
| `+1Ch` | the party brain |
| `+20h` | `std::list<AiGroup*>` object (12 bytes). `00A168A0` builds the sentinel |
| `+24h` | the list's root node, the sentinel `00A168A0` allocated |
| `+28h` | the list's size, the owned-group count |
| `+2Ch` | byte, the replan flag. Set by the claim, read-and-cleared by `vt +30h` |
| `+30h` | `brain+24h`, the own team index |
| `+34h` | `(brain+24h == 0)`, the enemy team index |

`Capture` (`44h`) adds `+38h = 0`, `+3Ch = 0`, `+40h = DAT_00D7A260` (`-1.0f`).

`00A22750 BSP_AiPlanner_ClaimGroup` appends into the list object at `planner+20h` (`LEA EDI,
[ESI+20h]` at `00A22773`), not at `planner+20h` as a bare pointer; the root it splices against is
`planner+24h`.

`00A1C8B0` is **not** an accept test. Body read in full (`00A1C8B0-00A1C8FE`): it walks
`planner+24h` and returns 1 when the group is already a member, 0 otherwise, i.e. a `contains`.
The claim therefore skips groups it already owns.

## The thirteen planner virtuals

Slots `+4h` (`00A18570`, a forward to `vtable[+24h]`), `+8h`/`+0Ch`/`+10h` (`0042B120`,
`0042B130`, `0042B140`) and `+14h` (`006935C0`) are shared by all eight classes and the base;
`+18h`, `+1Ch` and `+2Ch` are per class and pure in the base. `+20h` is the think and `+30h` the
replan query.

`00A18480`, the base `+30h` worn by the first four planners, is three instructions:
`MOV AL,[ECX+2Ch]; MOV byte [ECX+2Ch],0; RET`. A read-and-clear of the replan flag.

The four mode planners override it with four identical bodies (`00A1D110`, `00A1D140`,
`00A1D170`, `00A1D1A0`, `30h` bytes each). Each saves and clears the flag, and when the planner
owns at least one group asks `00A2CB10` about the **first** owned group: if that group is still
engaged the saved flag is returned, and if it is not, the answer is forced to true. So a mode
planner replans the moment its group finishes what it was doing.

`00A2CB10` (`__fastcall(group)`, body `00A2CB10-00A2CBC4`, read in full) asks the group's command
at `group+564Ch` three `IsType` questions in order, `6`, `10`, `2`:

- any `ATTACK` or any `DEFEND`: engaged;
- any `MOVE`: engaged only while the squared planar distance from the command's target
  (`command+8h`, `command+10h`) to the group position exceeds `00CE3D64` = `10000.0f`, i.e. 100
  units;
- `NONCONTROL`, `IDLE`, `SELLING`: not engaged.

## The AI command classes

Sixteen vtables, ten slots each (`+0h`..`+24h`), terminating at `00D2298C`, `00D229B4`,
`00D229DC`, `00D22A04`, `00D22A34`, `00D22A5C`, `00D22A84`, `00D22AB8`, `00D22AE0`, `00D22B10`,
`00D22B38`, `00D22B60`, `00D22BA0`, `00D22BE0`, `00D22C28`, `00D22C68` — the addresses
`docs/AI_GROUP_THINK.md` cited are these terminators, so each vtable **starts** `24h` lower.
Four are abstract (pure `+4h`): the root and the three family bases.

`vt +4h` returns the class id and `vt +8h` answers `IsType(id)`. `vt +18h` (`00A0FF60`) writes
the pair `"commandType" = g_aiCommandTypeNames[GetType()]` into a sink through `00B66790`, and the
name table is fifteen `char*` at `00E0E308`. That table settles the id space:

| Id | Name | Vtable | GetType | IsType also accepts | Ctor seen | Size |
| --- | --- | --- | --- | --- | --- | --- |
| — | (root, abstract) | `00D22968` | pure | — | — | — |
| 0 | `NONCONTROL` | `00D22990` | `00A0FCE0` | — | `BSP_AiGroup_Construct` | — |
| 1 | `IDLE` | `00D229E0` | `00A0FD80` | — | `BSP_AiGroup_Construct` | — |
| 2 | `MOVE` (abstract) | `00D22A94` | pure | `2` | — | — |
| 3 | `MOVETO` | `00D22ABC` | `00A10270` | `2` | `00A2CCF0` | — |
| 4 | `CAUTIOUSMOVE` | `00D22AEC` | `00A10310` | `2` | `00A102D0` | — |
| 5 | `REGROUPINGMOVE` | `00D22B14` | `00A103A0` | `2` | `00A10370` | — |
| 6 | `ATTACK` (abstract) | `00D22B7C` | pure | `6` | `00A10710` | — |
| 7 | `MOVETOATTACK` | `00D22BBC` | `00A108C0` | `6` | `00A10890` | `20h` |
| 8 | `CAUTIOUSATTACK` | `00D22C04` | `00A109F0` | `6` | `00A109B0` | `2Ch` |
| 9 | `CLOSEATTACK` | `00D22C44` | `00A10B10` | `6` | `00A10AE0` | `20h` |
| 10 | `DEFEND` (abstract) | `00D22A10` | pure | `10` | — | — |
| 11 | `DEFENDPOSITION` | `00D22A38` | `00A0FE90` | `10` | `00A2BE20` | — |
| 12 | `PATROLTO` | `00D22B3C` | `00A10470` | `10` | `00A2C310` | — |
| 13 | `RETREAT` | `00D22A60` | `00A0FEF0` | `10` | `00A2BE70` | — |
| 14 | `SELLING` | `00D229B8` | `00A0FD30` | — | inline at `00A228AD` | — |

`00A13340` is a factory that installs nine of these vtables; it was not read.

### Layout, from the `ATTACK` base constructor `00A10710`

`__thiscall(command)(group, target)`. `+0h` primary vtable, `+4h` the owning group, `+8h` the
secondary (observer) vtable, `+0Ch`..`+14h` zero, `+18h` a byte, `+1Ch` the target group, then
`BSP_Observer_RegisterPair`. `00A1CB80` reads the current target back from `command+1Ch` and
`00A2CBD0` compares against it, which is the producer-side confirmation of `+1Ch`.

The `MOVE` and `ATTACK` families carry a secondary base at `+8h` whose vtable shares the
`00A10040, 0042B120, 0042B130, 0042B140, 006935C0` tail with the planner base's slots `+4h`..
`+14h`. Three classes (`DEFEND` base, `CAUTIOUSMOVE`, `CAUTIOUSATTACK`) carry a third, two-slot
base (`00A14DD0`, `00A12580`) installed at `+20h` with fields `+24h` (byte) and `+28h = 4`.

### The merge decision at `vt +14h`

The default is `00A0FC80`, `XOR AL,AL; RET 4` — **false**. Only two classes override it, so a
group merges only while it is uncommanded:

| Class | Body | Rule |
| --- | --- | --- |
| `NONCONTROL` | `00A11F80` | other non-null, `other+5644h != 0`, `owner+5644h != 0`, `HasGroupableCombatant(owner) == HasGroupableCombatant(other)`, then `00A10D50(other)` and `00A10C60(other)` |
| `IDLE` | `00A12450` | the same, plus `00A2C600(owner) == 00A2C600(other)` |

Both take one stack argument and `RET 4`. `00A10D50` and `00A10C60` were not read; their contract
is unread.

## The planner choice, per group

`docs/AI_GROUP_THINK.md`'s table stands, with the two predicates now named:
`00A2C5A0 BSP_AiGroup_HasGroupableCombatant(group)` and
`00A2C450 BSP_AiGroup_HasMemberInWorldSet(group, brain+24h)`. No groupable combatant, or one that
is in the brain's own world set, picks `brain+0h` `Defend`; a groupable combatant outside the set
picks `brain+0Ch` `Capture`. `include/bsp/ai_group_think.hpp`'s `ai_planner_for_group` already
carries the rule.

So the commander itself only ever hands a group to `Defend` or `Capture`. The other two group
planners acquire groups elsewhere:

- `Sell` claims in its own think (`00A22750` at `00A2284D`);
- `Defend` and `Capture` claim extra groups inside their thinks, with the claim body inlined
  (`00A1C8B0` at `00A28F49`, `00A297E2`, `00A2B1E3`, `00A2B2C3`, each followed by
  `BSP_Observer_RegisterPair` at `00A28F88`, `00A29825`, `00A2B226`, `00A2B306`);
- `Attack` (`brain+4h`) never claims. Its think only walks the list it already owns, so its groups
  must arrive through `00A16EF0`, which calls `00A22750` six times and was not read.

## `00A1CB80`, the shared target pass

`__thiscall(planner)(AiGroup* group, float aggressive, bool resetTarget)`, body
`00A1CB80-00A1CF0C`, read in full. The argument order is from the `Attack` think's pushes
(`00A1CFEE`, `00A1CFF0`, `00A1CFF4`) and the `Siege` think's (`00A265C8`, `00A265CA`,
`00A265D0`); `EBX = [ESP+40h]` at `00A1CB89` is the group, and every `EBX+56xxh` access in the
body is a group field.

Four of the eight planners funnel into it: `Attack` (`00A1CFF7`, once per owned group, aggressive
= the party's aggressive ratio from `00F8A8D0 + party*1Ch`), `Escort` (`00A264F5`), `Siege`
(`00A265D3`) and `Competitive` (`00A266A2`), the last three with aggressive = `1.0f` and
resetTarget = `([00F8A9E0] == 3)`.

```
if (resetTarget) { group+5624h = 0; group+5628h = -1; }
current = group->command->IsType(ATTACK) ? command+1Ch : 0
enemy   = g_aiGroupsByTeam[planner+34h]
if (enemy.size == 0) return
best = -999999.0f (00D22CC4); chosen = 0
for (cand in enemy) {
    if (cand+5644h == 0) continue
    base   = 00A0F970(0, -1.0f, 0, 0, 1.0f)                     ; 00A1CC65
    d2     = squared planar distance of the two leaders' poses   ; entity+0FCh, +104h
    dist   = d2 <= 1.0e-11 ? 0 : sqrt(d2)                        ; 00CE3820, a double
    range  = InterpolateClamped(tuning+1D0h, 1.0f, tuning+1D4h, 00D7A2F0, dist)
    ownSet = HasMemberInWorldSet(cand, brain+24h) ? tuning+1CCh : 1.0f
    sticky = (cand == current)                    ? tuning+1D8h : 1.0f
    score  = sticky * ownSet * base * range
    if (score > best) { best = score; chosen = cand }
}
if (chosen) 00A2CBD0(group)(chosen, aggressive)
```

`tuning` is `00A371A0 BSP_Ai_ActiveModeTuning`, the `23Ch` record of
`docs/AI_GLOBALS_AND_TARGET_WEIGHTS.md`; the four fields `+1CCh`, `+1D0h`, `+1D4h`, `+1D8h` are
read there for the first time and their Lua key names were not resolved. The debug arm
`00A2B950(cand, 5, score, ...)` prints all four factors.

## `00A2CBD0`, the attack order

`__thiscall(AiGroup* group)(AiGroup* target, float aggressive)`, body `00A2CBD0-00A2CCE8`, read in
full. `ECX` is the group, established from `MOV ECX,EBX` at `00A1CEED` with `EBX` the group.

```
if (group+5644h == 0) return
if (command->IsType(ATTACK) && command+1Ch == target) return   ; already on it
member = first member of group+5640h
if (member->vtable[+5Ch](6) == 0 && 00A2C9F0(00A10C20()) && 00BD2F40() > aggressive)
     cmd = new(2Ch); 00A109B0(group, target)     ; CAUTIOUSATTACK
else cmd = new(20h); 00A10890(group, target)     ; MOVETOATTACK
delete the old command through its vtable slot 0 with flag 1; group+564Ch = cmd
```

`00BD2F40` is compared against the ratio, so a **higher** aggressive ratio makes
`CAUTIOUSATTACK` less likely. This is the one place the per-party aggressive ratio of
`docs/AI_GROUP_THINK.md` changes behaviour.

## The mode planners' think

`00A26510` (`Siege`, `00A26510-00A265E8`) and `00A265F0` (`Competitive`,
`00A265F0-00A266B7`) are read in full and share one shape:

```
if (planner+28h == 0) {                       ; owns nothing yet
    name = "[siege]" / "[competitive]"        ; 00D23014 / 00D2301C
    00A25B90(planner)(&name, 0, 1, -1)        ; quick spawn
    00A16450([00E0E344])(0.5f)                ; 00CE3800
    return
}
00A1CB80(planner)(firstOwnedGroup, 1.0f, [00F8A9E0] == 3)
```

`Duel` (`00A25F70-00A26202`) and `Escort` (`00A26210-00A2650C`) open with the same spawn arm
(`"[duel]"` `00D23000`, `"[escort]"` `00D23008`) and then add their own orders: `Duel` calls
`00A2CBD0` at `00A261BA` and builds a `MOVETO` at `00A261EC`; `Escort` calls `00A2CBD0` at
`00A2649C` and builds `PATROLTO` at `00A2646A` and `00A264D3`. The `Capture` think spawns
`"[capture]"` (`00D22CA8`, used at `00A2B6CE`) the same way. `"[defend]"` (`00D22C98`) is used by
`00A15570`, not by the `Defend` think.

## `00A179E0`, the brain's engagement pass

`__fastcall(brain)`, body `00A179E0-00A18195`. Decompiled with 66 unreachable blocks removed, so
the arms below are what the decompiler kept; the removed blocks are the EH funclet tails.

```
008EA0C0(brain+20h, &vec)                   ; fill a vector from the party
if (vec.begin == 0) return
if ((vec.end - vec.begin) >> 2 != 0) {
    own   = 00A17960(&g_aiGroupsByTeam[brain+24h])
    enemy = 00A17960(&g_aiGroupsByTeam[brain+24h == 0])
    for (a in own) for (b in enemy) {
        if (a+5644h == 0 || b+5644h == 0) continue
        if (a+5634h != brain+20h) continue                  ; a belongs to this party
        d2 = squared planar distance of the two leaders
        if (d2 > 9000000.0f) continue                        ; 00D22CB8, radius 3000
        sa = 00A2C4C0(a); if (sa < 15.0) continue            ; 00CF3F20, a double
        sb = 00A2C4C0(b)
        if (sa*0.5 <= sb && 0.5*sb <= sa)                    ; 00D7A280, a double
            00A17880(&pair)                                  ; record the engagement
    }
    00A16B60(); 00A16B60()                                   ; free the two snapshots
}
free(vec.begin)
```

So the pass pairs every populated group of this brain's party against every populated enemy-team
group within 3000 units whose strength is within a factor of two, and records each pair. The
`(team == 0)` flip is the same one every planner uses, so it distinguishes teams 0 and 1 only and
maps team 2 onto team 0. `00A17880`, `00A17960` and `00A2C4C0` were not read; their contracts are
unread.

## The `00F8AA60` list: filler found

`00F8AA60`/`00F8AA64` is element **2** of a three-element array. `CG_static_init_00CE04B0`
constructs it with `00BF7C6E(0xF8AA48, elementSize 0xC, count 3, ctor 0xA16E00)`, so the array is
`std::list<AiGroup*> g_aiGroupsByTeam[3]` based at `00F8AA48` with stride `0Ch`: object
`00F8AA48`/`00F8AA54`/`00F8AA60`, root `+4h`, size `+8h`.

The filler is `BSP_AiGroup_Construct`: every group appends itself to
`g_aiGroupsByTeam[group+5638h]` unconditionally (`00A2E086`-`00A2E0BD`), where `group+5638h` is
`kAiGroupTeamId`, copied from the constructor's second argument `+54h`.
`include/bsp/ai_group_think.hpp` already declares this array as `kAiGroupPerTeamListBase`; the
open question in `docs/AI_GROUP_THINK.md` is the one that is stale. `BSP_AiGroups_ComposePass`
reads element 2's root at `00F8AA64` (`00A2EB11`, `00A2EE13`, `00A2EE25`, `00A2EE43`, `00A2EE5A`),
and the planners read elements 0 and 1 through `planner+34h`.

## Reconstruction

`include/bsp/ai_planners.hpp` and `src/ai_planners.cpp`. Complete as rules: the class table and
the constructor's mode branch, the planner layout, the command id space and its `IsType`
hierarchy, the `+14h` merge predicate, `00A2CB10`, the `+30h` replan virtuals, `00A2CBD0`'s
order choice, `00A1CB80`'s scoring loop and `00A179E0`'s pair filter. The two sequence routines
`ai_planner_choose_attack_target` and `ai_mode_planner_tick` run over `AiPlannerHost`, one method
per native call site. Nothing here is ABI-compatible.

## Open questions

- The `Defend` think `00A28A60` (`00A28A60-00A29E2A`) and the `Capture` think `00A29FD0`
  (`00A29FD0-00A2B7EB`) are read only for what they claim and what they issue. Their scoring,
  their use of `00A371A0` at `00A298E2`/`00A29AB0`/`00A2B4A6` and the `"capture: %.2f\n=(%.2f-
  %.2f/2)+(%.2f-%.2f/2)+%.2f"` formula at `00D22CC4` are unread.
- `00A16EF0`, which claims six times and is the only candidate producer for the `Attack` planner's
  group list.
- The Lua key names of tuning fields `+1CCh`, `+1D0h`, `+1D4h`, `+1D8h`, `+208h`'s neighbours.
  The loader `00A335D0` has the store sites; they were not searched.
- `00A10C60` and `00A10D50`, the two tests both merge overrides end with.
- `00A13340`, the command factory, and the meaning of the two-slot base at command `+20h`.
- Planner `+4h`, `+8h`, `+0Ch` have no producer beyond the zeroing and no consumer this packet
  found.
- What value `+54h` takes for a group to land in `g_aiGroupsByTeam[2]`, the list the compose pass
  walks.
