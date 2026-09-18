# The AI command object on a group

Addresses: 00A13340, 00A37A00, 00A2BD00, 00A2C790, 00A2C8D0, 00A2C600, 009FE0B0, 00A11F80,
00A12450, 00A10D50, 00A10C60, 00A0FC50, 00A0FC80, 00A0FC90, 00A2C5A0, 00A2CBD0, 00A2DB80,
00A10710, 00A10890, 00A109B0, 00A10AE0, 00A102D0, 00A10370, 00A11F70, 00A12430, 00A10EC0,
00A10DC0, 00A11070, 00A179E0, 00A17880, 00BD2F10, 00D22968-00D22C68, 00E0E308, 00CE3958,
00CE3D34, 00CFD964, 00D22A88, 00D22C78, 00A2C6C0, 00A2C660, 009FE120, 009FE0F0.

Packet `cc8_ai_command_object`, read-only analysis. Every descriptive name here is a hypothesis,
not a recovered symbol. The three genuine string literals are the Lua keys `commandType`
(00D22A88), `targetPos` (00D22C78) and `target` (00CFD964).

Coverage: complete as rules for the factory arms, the install, the merge predicates, the
notification census and the member pass. Partial for the per-class tick at `vt+0Ch`: the three
shared helpers `00A10EC0`, `00A10DC0` and `00A11070` were not read, and neither were the eleven
class-specific `+0Ch` bodies.

## The ten vtable slots, settled

Dumping `00D22968`-`00D22C68` as sixteen ten-dword rows fixes what each slot is. `+10h` and `+24h`
are the same body in every class.

| Slot | Body | What it is |
| --- | --- | --- |
| `+0h` | per class | scalar deleting destructor; `00A2BD00` calls it with flag 1 |
| `+4h` | per class | `GetType`, the class id |
| `+8h` | per class | `IsType(id)` |
| `+0Ch` | per class | the tick `00A2C790` runs on the group's own schedule |
| `+10h` | `00A0FC50` in all sixteen | writes the member-pass interval, 2.0f and 4.0f |
| `+14h` | `00A0FC80` except two | the merge decision; `00A11F80` and `00A12450` override |
| `+18h` | `00A0FF60`/`00A13960`/`00A13A60` | the `"commandType"` describe writer |
| `+1Ch`, `+20h` | per class | not read |
| `+24h` | `00A0FC90` in all sixteen | the member notification; `RET 8`, discarded |

`00A0FC90` at `+24h` in every row closes `docs/AI_GROUP_THINK.md`'s open item, "whether any class
in the block overrides the slot with a real body was not settled". **No class does.** Both callers
of that slot, `00A2BD90` and `00A2C790`, therefore report into a sink.

## `00A13340`, the Lua command factory

`__thiscall(AiGroup* group)(five stack arguments)`, `RET 0x14`, body `00A13340`-`00A137C9`.
Exhaustive call-site census (`tools/callsite_census.py 00a13340`): **one** caller, `CALL` at
`00A37AD6` inside `00A37A00 BSP_LuaBinding_AISetCommand`. The result goes straight to
`00A2BD00(group)(command)` at `00A37ADE`, so a mission script's `SetCommand` is the only producer.

The body reads the Lua key `"commandType"` (`00D22A88`, pushed at `00A13364`) and compares the
string case-insensitively against the name table at `00E0E308` through `00425850`. Each arm's
`operator new` size is the `PUSH` immediately before `CALL 0x00BF681B`.

| `commandType` | Name table | Id | `operator new` | How it is built | Lua argument |
| --- | --- | --- | --- | --- | --- |
| absent or `NONCONTROL` | `00E0E308` | 0 | `8` | vtable `00D22990` inline at `00A13430` | none |
| `IDLE` | `00E0E30C` | 1 | `8` | vtable `00D229E0` inline at `00A133F6` | none |
| `MOVETO` | `00E0E314` | 3 | `14h` | vtable `00D22ABC` inline at `00A134D0` | `targetPos` |
| `CAUTIOUSMOVE` | `00E0E318` | 4 | `20h` | ctor `00A102D0` at `00A13560` | `targetPos` |
| `REGROUPINGMOVE` | `00E0E31C` | 5 | `14h` | ctor `00A10370` at `00A135D2` | `targetPos` |
| `MOVETOATTACK` | `00E0E324` | 7 | `20h` | ctor `00A10890` at `00A13651` | `target` |
| `CAUTIOUSATTACK` | `00E0E328` | 8 | `2Ch` | ctor `00A109B0` at `00A136B2` | `target` |
| `CLOSEATTACK` | `00E0E32C` | 9 | `20h` | ctor `00A10AE0` at `00A13712` | `target` |
| `DEFENDPOSITION` | `00E0E334` | 11 | `8` | vtable `00D22A38` inline at `00A13740` | none |
| `RETREAT` | `00E0E33C` | 13 | `8` | vtable `00D22A60` inline at `00A1376E` | none |
| anything else | — | 1 | `8` | vtable `00D229E0` inline at `00A13787` | none |

`MOVE` (2), `ATTACK` (6) and `DEFEND` (10) are abstract; `PATROLTO` (12) and `SELLING` (14) have no
arm, so a script asking for either gets `IDLE`.

`DEFENDPOSITION` and `RETREAT` are allocated eight bytes and receive only the vtable and the owner
back-pointer, although their family carries a third base at `+20h`. A script command of either
class therefore has no position and no target. Stated as read, not explained.

`include/bsp/ai_planner_tails.hpp`'s `ai_tail_command_recipe` already carries this table from packet
`cc2_ai_planner_tails`; the listing read above confirms every row, every size and both argument
keys independently.

### Layout

| Offset | Field | Evidence |
| --- | --- | --- |
| `+0h` | primary vtable | every arm writes it |
| `+4h` | the owning group | `MOV [EAX+4],EDI` in every inline arm, `EDI` the `ECX` group |
| `+8h`..`+10h` | the `MOVE` family's destination, three floats | `00A134C5`-`00A134DC` from `00888760` |
| `+8h` | the `ATTACK` family's observer sub-object vtable | `00A10710`, `docs/AI_PLANNERS.md` |
| `+1Ch` | the `ATTACK` family's target group | `00A10710`; `00A2CBD0` and `00A2DB80` compare it |
| `+20h`..`+28h` | `CAUTIOUSATTACK`'s third base | size `2Ch`, `docs/AI_PLANNERS.md` |

The `MOVE` family's `+8h` is a float and the `ATTACK` family's `+8h` a vtable pointer, so the two
families share no field above `+4h`.

### Where the group keeps it, and what replaces it

`group+564Ch`, size `5660h`. The group constructor installs an eight-byte instance before the first
member is added: vtable `00D22990` (`NONCONTROL`) when the group's AI party slot `+5634h` is
negative, `00D229E0` (`IDLE`) when `009FFE50` admits that slot, `00D22990` otherwise
(`docs/AI_GROUP_THINK.md`'s constructor table). Both of those classes override the merge predicate,
so a group can auto-merge from birth and stops being able to as soon as any order lands.

`00A2BD00 BSP_AiGroup_SetCommand`, `__thiscall(group)(command)`, `RET 4`, body
`00A2BD00`-`00A2BD2E`, read in full: when `group+564Ch` is non-null, `MOV EAX,[ECX]` /
`MOV EDX,[EAX]` / `PUSH 1` / `CALL EDX` at `00A2BD0D`-`00A2BD13` runs slot `+0h` with flag 1, then
`[ESI+564Ch]` takes the new pointer. The null arm at `00A2BD23` only stores. Three producers reach
it: the Lua `SetCommand` at `00A37ADE`, the attack order `00A2CBD0`, and phase A of the merge
`00A2DB80`.

## The merge decision

`00A2C8D0 can_auto_merge`, `__thiscall(into)(from)`, `RET 4`. Its own gates are in
`docs/AI_GROUP_THINK.md`; the delegated half is `from+564Ch->vtable[+14h](into)`, and only two
classes answer anything but false.

`00A11F80`, `NONCONTROL`, body `00A11F80`-`00A11FE8`, read in full:

| Test | Evidence |
| --- | --- |
| the argument is non-null | `00A11F85 TEST ESI,ESI` |
| `other+5644h != 0` | `00A11F8C` |
| the owner `command+4h` is non-null and `owner+5644h != 0` | `00A11F95`, `00A11F9C` |
| `00A2C5A0(owner) == 00A2C5A0(other)` | `00A11FA6` with `ECX` the owner, `00A11FAF` with `ECX = ESI` |
| `00A10D50(command)(other)` | `00A11FBC` |
| `00A10C60(command)(other)` | `00A11FC8` |

`00A12450`, `IDLE`, body `00A12450`-`00A124CF`, read in full: the same six tests with
`00A2C600(owner) == 00A2C600(other)` inserted at `00A12490`-`00A124A3`, between the grouping
equality and the two leader tests.

`00A10D50 BSP_AiCommand_MergeLeaderStrengthOk` and `00A10C60 BSP_AiCommand_MergeLeaderDistanceOk`
were read by packet `cc2_ai_planner_tails` and are unchanged here: merge only into an equal or
stronger lead unit, and only while the squared planar leader distance is under
`AutoMerge_MergeDist`² (tuning `+208h`, shipped 650).

### `00A2C600`, the extra equality `IDLE` asks

`__thiscall(group)`, `RET 0`, body `00A2C600`-`00A2C650`, read in full. It walks the `+563Ch` member
list from `[group+5640h]` and leaves with `AL = 1` at the first member `009FE0B0` accepts
(`00A2C62B`-`00A2C635`), `AL = 0` off the end.

`009FE0B0`, `__thiscall(entity)`, `RET 0`, body `009FE0B0`-`009FE0E9`: three calls to
`entity->vtable[+5Ch]` with `0x1B` (`009FE0B8`), `0x45` (`009FE0C5`) and `0x46` (`009FE0D4`), first
hit wins. Which three entity classes those ids name was not resolved, so the predicate is carried
as "the member is one of the three `009FE0B0` classes". `00A2D8E0`, the sorted member insert, calls
the same routine, which is the producer-side confirmation that it is an entity-class test.

### What would merge under the shipped tuning

`AutoMerge_MergeDist` is 650 (`docs/AI_TUNING_GLOBALS.md`), and phase 4 of the composition pass
walks the eight **party** lists `00F8A9EC + p*12`, not the team lists. Two groups of the same party
merge when both are populated, both answer `00A2C5A0` alike, the absorbed group still carries its
birth `NONCONTROL` or `IDLE` command, the absorber's lead unit is at least as strong, and the two
leaders are within 650 units. On IJN01, USN01 and USN02 the composition pass files every group under
its own team, and the run measurements below report how many pairs clear all six tests.

Phase 5, the proximity merge, walks the list at `00F8AA64`. That address is `00F8AA48 + 2*12 + 4`,
the head pointer of `g_aiGroupsByTeam[2]`, which answers `docs/AI_GROUP_THINK.md`'s open question
about `+54h`: a group lands there only when its seed entity's `+54h` is 2. The seed phase at
`00A2E835`-`00A2EA5A` admits an entity only while `+54h < 2` (`CMP [ESI+54h],EBP` with `EBP = 2`),
so **no seeded group can ever be in the team-2 list**; only the Lua `AICreateGroup` binding could
put one there. On the three campaign missions measured the proximity phase walks an empty list, and
that, not the merge distance, is why it takes nothing.

## The per-member dispatch

The brief called `vtable[+114h]` a slot of the command. It is not: `00A2C790` loads it from the
**member entity's** own vtable (`MOV ESI,[EDI+8]` then `MOV EDX,[ESI]` / `MOV EAX,[EDX+114h]` /
`MOV ECX,ESI` at `00A2C7F0`-`00A2C7FD`). The command is only the recipient.

`00A2C790`, `__thiscall(group)`, `RET 0` (`ADD ESP,0x10` cleans its own frame), body
`00A2C790`-`00A2C8C6`, read in full. No Ghidra name; the decompiler drops every register argument,
so the chain below is from the listing.

```
if (group+5644h == 0) return
for each node in the group+563Ch list:
    member  = node+8h
    dir     = member->vtable[+114h]()          ; 00A2C7FD, the gate
    if (dir == 0) continue
    desc    = 0071EB60(member->vtable[+114h]()) ; 00A2C811
    current = 0071BE40(member->vtable[+114h]()) ; 00A2C826
    if (group+564Ch != 0)
        group+564Ch->vtable[+24h](current, desc) ; 00A2C837 PUSH EDI, PUSH EAX
if (group+5650h <= [00F876A4]) {
    group+564Ch->vtable[+0Ch]()                 ; 00A2C879
    group+564Ch->vtable[+10h](&hi, &lo)         ; 00A2C890
    group+5650h = [00F876A4] + 00BD2F10(lo, hi) ; 00A2C8B3, FADD at 00A2C8B8
}
```

`0071EB60` and `0071BE40` both take the `vt+114h` result as `ECX`, and `0071ECF0
BSP_WeaponDirector_IssueCommand` is the other caller of the `+24h` slot with the same
`(command, SceneCommandTarget*)` pair, so `vt+114h` yields the member's own weapon director and the
notification says "this member currently holds that scene command against that target". The push
order at `00A2C837`-`00A2C838` puts the `0071BE40` result first.

`00A0FC50`, the `vt+10h` body every class shares, has no Ghidra function. Six instructions:
`*[ESP+4] = [00CE3958]`, `*[ESP+8] = [00CE3D34]`, `RET 8`. The two constants are `0x40000000` and
`0x40800000`, so **2.0f and 4.0f**. Following the stack through the two `LEA`/`PUSH` pairs at
`00A2C886`-`00A2C88F` and the two `FLD`/`FSTP` pairs at `00A2C89F`-`00A2C8B0`, the low argument of
`00BD2F10(lo, hi)` is the `00CE3958` slot. **Every group's command ticks on a uniform 2 to 4 second
schedule, the same for all sixteen classes.**

### So a MOVETO does not reach a unit here

Nothing in `00A2C790` issues a command. The per-member chain only reads and reports, and every
class discards the report at `+24h`. The only path that could turn a `MOVETO`, `CAUTIOUSMOVE` or
`DEFENDPOSITION` into a scene command is the class's own `vt+0Ch` tick, which this packet did not
read. `NONCONTROL`'s `+0Ch` (`00A11F70`) is a bare `JMP 00A10EC0`; `IDLE`'s (`00A12430`) runs
`00A10EC0`, `00A10DC0` then tail-calls `00A11070`. Those three helpers, `00A10EC0`-`00A1106A`,
`00A10DC0`-`00A10EB6` and `00A11070`-`00A113C1`, are the follow-up. Until they are read, the
statement in `docs/AI_COORDINATOR_TICK.md` that this process substitutes `0077D600` for the
dispatch stands, with the correction that the substitution replaces `vt+0Ch`, not `vt+114h`.

## The planner's owned-group walk

`docs/AI_COMMAND_LIFETIME.md`'s `ai_planner_owned_group_walk` asked whether `00A179E0`'s engagement
pass orders a planner's groups beyond the first. It does not. `00A179E0 BSP_AiPartyBrain_EngagementPass`
pairs populated groups within 3000 units whose strengths are within a factor of two and passes each
pair to `00A17880`, which the ledger records as `std::vector<T>::push_back` for a 28-byte element,
its only caller being `00A179E0`. The pass records; it issues nothing. So the native really does
order only the planner's first owned group per think, through
`00A1CB80(planner)(firstOwnedGroup, ...)`.

## Host methods

`src/game_hosts_ai.cpp` now builds the object. `GameAiCoordinatorHost::Impl` also implements
`bsp::AiCommandMemberPassHost`, one method per native call of `00A2C790`.

| Host method | Native | Note |
| --- | --- | --- |
| `initial_command_for` | group ctor `+564Ch` | `ai_command_initial_type` on the party slot |
| `can_auto_merge` | `00A2C8D0` | the whole routine: the self, population and family gates, then the real `vt+14h` predicate |
| `group_has_ship` | `00A2C6C0` / `009FE120` | `unit_is_kind_of(6)` over the members |
| `group_has_air` | `00A2C660` / `009FE0F0` | `unit_is_kind_of(0Fh)` or `(18h)` over the members |
| `group_matches_009fe0b0` | `00A2C600` | `unit_is_kind_of` for `0x1B`, `0x45`, `0x46` |
| `group_member_pass` | `00A2C790` | runs `ai_command_member_pass_00a2c790` |
| `member_weapon_director` | `member->vtable[+114h]` | substitution: a live unit stands in for the director |
| `director_target_descriptor` | `0071EB60` | `active_command_descriptor_0071eb60` |
| `director_current_command` | `0071BE40` | `director_current_command_0071be40` |
| `command_notify_member` | `vt+24h` | `00A0FC90`, so it counts and discards |
| `command_tick` | `vt+0Ch` | counted only; the three helpers are unread |
| `command_pass_interval` | `vt+10h` `00A0FC50` | 2.0f and 4.0f |
| `order_attack` | `00A2CBD0` + `00A2BD00` | installs the replacement, counts the delete |
| `merge_group` | `00A2DB80` phase A | rebinds every ATTACK aimed at the absorbed group |

Three labelled substitutions. `member_weapon_director` has no director object to return, so a live
unit stands in for a non-null pointer. `release_group_reference` reverts a command whose target was
released to the group's birth class, because this process cannot hold the dangling `+1Ch` the
native leaves. And `00A10D50` compares `AiLeaderWeight` through the adjustor thunk `009FFD70`,
whose contract is unread, so `group_leader_order_key` substitutes the leader's unit index: the
merge counts below show that the predicate runs, not that it answers what the native would.

## Corrections

Appended to `docs/AI_COORDINATOR_TICK.md`, `docs/AI_GROUP_THINK.md` and `docs/AI_PLANNERS.md`,
never rewriting them.

## no_ghidra_function

| Address | Inclusive end | What it is |
| --- | --- | --- |
| `00A0FC50` | `00A0FC72` | `vt+10h` in all sixteen classes; writes 2.0f and 4.0f, `RET 8` |
| `00A0FC90` | `00A0FC92` | `vt+24h` in all sixteen classes; `RET 8` |
| `00A11F70` | `00A11F74` | `NONCONTROL`'s `vt+0Ch`; `JMP 00A10EC0` |

`00A0FC80`, the default `vt+14h` (`XOR AL,AL; RET 4`), was already recorded by
`docs/AI_PLANNERS.md`.

## Validation

Three campaign missions, 3000 mission frames at 0.05 s, from this worktree. Before is
`main` 97d639fd7 built here; after is this change.

| Mission | Run | Groups | Commands by class | Merges | Member passes | `vt+0Ch` ticks | Attack orders |
| --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | 4 | none built | 0 auto, 0 prox | 330000 | n/a | 1 MOVETOATTACK |
| IJN01 | after | 4 | 1 MOVETOATTACK, 1 IDLE, 2 NONCONTROL | 0 auto, 0 prox | 330000 | 204 | 1 MOVETOATTACK |
| USN01 | before | 4 | none built | 0 auto, 0 prox | 153000 | n/a | 1 MOVETOATTACK |
| USN01 | after | 4 | 1 MOVETOATTACK, 1 IDLE, 2 NONCONTROL | 0 auto, 0 prox | 153000 | 204 | 1 MOVETOATTACK |
| USN02 | before | 2 | none built | 0 auto, 0 prox | 96000 | n/a | 1 MOVETOATTACK |
| USN02 | after | 2 | 1 MOVETOATTACK, 1 NONCONTROL | 0 auto, 0 prox | 96000 | 103 | 1 MOVETOATTACK |

Attribution.

- **Commands by class.** Every group now carries one. Party 0 is admitted by `009FFE50`, so its
  groups are born `IDLE`; party 1 is not, so its groups are born `NONCONTROL`; the single attack
  order replaces one `IDLE` with `MOVETOATTACK` carrying a target. Before the change the host held
  a flag and an arm, and the report line could only print `command=0/1`.
- **The tick count is the schedule.** 204 ticks over four groups and 103 over two is 51 per group
  in 150 s of mission time, one every 2.9 s, inside the 2.0-4.0 s band `00A0FC50` supplies. This is
  the run-time evidence for the interval constants.
- **`can_auto_merge` went from `UNIMPLEMENTED` to `concrete`** at the same 23990 calls per mission,
  and `group_member_pass` likewise at 11996. The call counts are identical before and after, so the
  phases were always reached; only the answers changed.
- **Merges stay at zero, and the class table says why for most pairs.** A `MOVETOATTACK` group
  answers the default `00A0FC80`, false, so every pair containing one is rejected by class alone.
  The remaining same-party pairs are USN01's two team-1 `NONCONTROL` groups and IJN01's, which the
  predicate rejects on one of its six tests. Which test was not instrumented, so this is
  **partial**: the leading candidate is the ship/air family gate, because the seed phase builds one
  group per entity collection and the collections separate the two, but that was not measured.
- **Gunnery.** IJN01 is unchanged at 4 hits, 4 records, 2 deaths, 500.0 damage. USN01 moves from 25
  hits to 24 and USN02 from 168 to 160, with damage 229.5 unchanged on USN01 and 19723.2 to 18661.8
  on USN02; deaths and kill credits are unchanged on all three. The cause is the shared random
  stream: `random_interval` draws the command's 2-4 s schedule from the same stand-in generator as
  the party think, which shifts the think times by one draw. `first_command` moves 4.85 s to 4.20 s
  and `thought` 39 to 38 on all three missions for the same reason. No gunnery code changed.

The first attempt at the after-runs hung: `00A2C8D0`'s own gates are the host method's contract,
not the sequence's, and without the `from == into` rejection phase 4 merged a group with itself,
left it populated and restarted its scan forever. `src/ai_group_think.cpp` line 198 calls
`can_auto_merge(a, b)` with no identity guard, while phase 5 at line 219 has its own. The gate now
rejects a null or self argument, an empty population on either side, and the family pair through
`00A2C6C0`/`009FE120` and `00A2C660`/`009FE0F0`.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_command_tick_helpers` | `00A10EC0`, `00A10DC0`, `00A11070` | the three bodies `NONCONTROL` and `IDLE` share at `vt+0Ch`, and whether any of them issues a scene command |
| `ai_command_move_family_tick` | `00A124E0`, `00A152B0`, `00A126C0`, `00A15500` | the `MOVETO`, `CAUTIOUSMOVE`, `REGROUPINGMOVE` and `DEFENDPOSITION` ticks, the only remaining path from a command to a unit |
| `ai_entity_class_ids` | `009FE0B0`, `vtable+5Ch` | which entity classes `0x1B`, `0x45` and `0x46` name |

## Correction from the cc8 integration: the two listing-only routines are defined (2026-09-18)

The `no_ghidra_function` list above was true when the packet read them. At integration
`00A0FC50`-`00A0FC72` (RET 8) and the five-byte thunk `00A11F70`-`00A11F74` (JMP `00A10EC0`) were
defined in Ghidra (record `reports/ai_command_object_function_definitions.json`, commit
`e985ff1f3`) and their ledger names applied; `00A0FC90` was already `BSP_AiCommand_NotifyEntityCommand_NoOp`.
