# CC13 GPR 69: segments 69 and 70 reference models (AI party brain, commands, planners)

Packet `cc13-gpr69` (cc13 lineage, Claude). Segment 69 is `009FE130`-`00A16030` (command types, the
vs-table, enemy and neutral weights, vehicle classes, targets). Segment 70 is `00A16050`-`00A371A0`
(the coordinator, capture, strategic gain, recon ratio, objectives). cc9 approved this
reference-model pass on these terms:
- new `*_cc13` files only: `src/gameplay_seg69_70_cc13.cpp` and
  `include/bsp/gameplay_seg69_70_cc13.hpp`, registered by one line in `cmake/startup.cmake`;
- ledger records;
- no host edit, no wiring, no game run.

The naming pass for this band is docs/CC13_GP_009F_00A3.md (cc13-gp9f). This packet read the bodies.

Method. Each comparison, constant and float rounding below was read from the disk bytes with
Capstone `disasm-raw`, the x87 register forms from the opcode bytes (`local/gpr69_pe.py`, not
committed). The models are **reconstructed and build-tested**. None is ABI-compatible, wired or
game-validated. Gameplay runs at 24-bit x87 precision (docs/CC13_FPU_PRECISION.md), so the models
compute in float where the image rounds to float.

## 1. Models

### 1.1 The AI party's power-up use (`00A179E0`, with `00A046C0`, `00A04860`, `00A04910`, `00A0F680`)

docs/SHIP_AI_OPEN_ITEMS.md section 5 established the arm structure of `00A179E0` and left the
operands and the six callees unread. This pass reads them all. Ghidra's body stops at `00A17D55`
(the checked-iterator call `00BF6713` is marked non-returning); the model is read from
`disasm-raw 00A179E0 --length 1974`.

**The pairs (`00A17A34..00A17D1B`).** For each group of the own team list (`00F8AA48 + team*0Ch`,
team = brain+24h), then each group of the enemy list (`(team == 0)*0Ch`), in list order:
- both populations `+5644h` are non-zero, and the own group's party `+5634h` equals brain+20h;
- the planar squared distance between the first members (`+FCh`, `+104h`) is at most 9,000,000
  (`[00D22CB8]`, 3000 units; FCOMI/JA, so NaN passes);
- the own strength (`00A2C4C0`, the sum of leader weights) is at least 15.0 (`[00CF3F20]`);
- neither strength is below half the other (`[00D7A280]` = 0.5; bytes `DC C9` then `D8 CA`);
- the pair's weight is `enemy strength / max(250000 [00D22CB4], d2)` (`00415550`, then `FDIVR`).

With no pair, nothing is used, even a party-wide or team-wide power-up.

**The choice (`00A17D5A..00A180A8`).** For each ready entry (`008EA0C0`, empty unless the byte
`00E0C978` is set), with the class `[entry+4]`:

| class `+8h` | scope `+84h` | candidates | score |
| --- | --- | --- | --- |
| 1 (air support) | 1 | the first member of each pair's own group, when `008E35F0` accepts it | `00A0F680(enemy group, class) * weight` |
| 1 | 2 | the first accepted member of each pair's enemy group | the same |
| 2 (multipliers) | 1 or 2 | every accepted member of each pair's own (1) or enemy (2) group | `00A046C0(member, class) * weight` |
| 2 | 4 | none (target 0) | `00A04860(party, class)` |
| 2 | 5 | none (target 0) | `00A04910(team, class)` |
| other | | none | 0.0 |

Every comparison is a strict `>` (`FCOMI` then `JBE`), so the first of equal scores wins and a NaN
never does. The overall seed is 0.0. The winner is used once per party think:
`008EADA0(ECX = [00F88C30], entry, party, target)` at `00A180C9`. The C++ pass makes that call
itself, through `PowerupUseHost::use_item(entry, party, target)` after the scoring loop, and
returns void like the image (cc13-fix14; it used to return the decision for the caller to fire).
Nothing in src calls the pass yet, so no switch guards it.

**`00A0F680`, the air-support score.** The class carries two flight blocks; the group's `+5638h`
picks `class+0Ch` (non-zero) or `class+44h` (zero). One 20h-byte value record is built:
`{[block+20h], 0.0, 0.0, 0, [block+1Ch], 009FDF30(class id), U(tuning+50h, tuning+54h), 0}`. The
spread is drawn on **stream 1** (`00A0F737`), where `00A04560`'s entity records use the deterministic
`00419010(..., entity mod 79)` (src/game_hosts_ai.cpp `group_value_spread`). The score is
`00A0C650(that record, 00A07E40(group), 0, -1.0, 0, 0, 1.0)`, the host's
`compose_attack_value_00a0c650`.

**`00A046C0`, a multiplier power-up's value on one unit.** Over the sixteen multipliers `m` at
`class+8Ch`, skipping `m == 1.0f` exactly: the term is `1 - m` for scope 2 and `m - 1` otherwise,
then by index (jump table `00A04820`):

| index | rule |
| --- | --- |
| 1 | doubled |
| 2 | 0 when health (`00923BE0`) > 0.9999 (`[00D04680]`) |
| 3 | `(1 - health) * term` (`DE E1`) |
| 4, 5 | 0 for a class `00828EC0` accepts (torpedo-proof small craft) |
| 7 | negated (`-0.0 - term`) |
| 10 | 0 unless a ship (kind 6) with class `+804h > 0` |
| 11 | 0 unless kind `1Ch` |
| 0, 6, 8, 9, 12..15 | the term as is |

The value is `float(00A04240(unit) * 009FDF30([unit+C4h])) * sum`.

**`00A04860` / `00A04910`.** Both walk the world list `[[00E188A8]+19CCh]+34h` and sum `00A046C0`
over the units `008E35F0` accepts. The side version keeps units whose derived party equals the
argument (side `+180h`: 8 gives -1; 0..7 is the party; anything else gives -1 in a multiplayer session
(`game+1FE4h`) and otherwise 4 or 0 by whether `+54h` differs from the local slot's team), then scales
by 0.25. The team version keeps `+54h == team` and scales by 0.125.

### 1.2 The planner spawn (`00A25A30` -> `00A236F0`, `00A23980` -> `00A0D1D0`, `00A03900`, `00A02180`)

docs/SHIP_AI_OPEN_ITEMS.md sections 47 and 51-52 established the route and closed it for the reference
rows (no authored stock). Section 52.3 left `00A0D1D0` and `00A236F0` unread. This pass reads them.

**`00A25A30`** returns 0 when `weight < 0.5` (`COMISS`/`JBE`: a NaN weight continues), when the threat
records are empty, or when `0066E590` returns no site. Otherwise it sorts the sites
(`00A236F0`, from `pos`, or from `00F87574` when `pos` is null) and returns `00A23980`'s answer.
Arguments, from the pushes: `(pos, weight, &records, tag, flag)`; `00A23980` receives
`(pos, &sorted, weight, &records, tag, flag)`.

**`00A236F0`** flattens the list of lists, keys each site by `(site+FCh - x)^2 + (site+104h - z)^2`
(float) and runs MSVC `std::sort` (`00A1FE30`; insertion sort `00A1C790` at 32 entries or fewer,
comparing `+4h` with a strict `>`). The order is ascending distance.

**`00A23980`** (the effective game mode `<= 3` arm). For each sorted site:
1. the classes are `0084D560(site)(&list, brain+24h, 0)`, each name resolved by
   `00964790 VehicleClass_GetOrCreate(name, 1)`;
2. the budget is `00942130(brain+20h)`;
3. `00A0D1D0(classes, budget, weight, site pos, 0, flag, -1.0)` composes the force (below);
4. a non-empty composition is weighted 1.0 when there is one site; otherwise
   `00419010(t+100h, t+10Ch, t+104h, t+108h, dist / speed) * score`, where speed is
   `00A07C10` over the composition, dist is `sqrt(d2)` only when `d2 > 1e-10` (`[00CE3820]`), and a
   null `pos` uses `t+100h` in place of the ratio;
5. the best weight above -1e10 (`[00CE4ADC]`, strict) keeps its composition (`00A21D90`).

With a best site it builds the class list, the record `+10h` values (class `+134h`) and
`0084CF80(site, class)` squad sizes, and calls
`0094C830([00F89B3C])(party, &classes, site, [slot[party]]+2Ch, &values, &squads, 0, tag)`.
The `vt[18h](0Fh)` / `(14h)` 1-or-3 value written first into the squad slot is dead: `0084CF80`
overwrites it. Note that the last argument is the **tag** (`00A23980` arg 5), not the flag; the
flag only reaches `00A0D1D0`, where it allows mixing planes and surface units.

**`00A0D1D0`, the greedy composition.** `__fastcall(ECX = composition, EDX = threat records;
classes, budget, target, pos, byte5, flag, arg7)`, `RET 1Ch`. Rounds run while `target > spent`
(gate `target > 0`):
1. Each class is tried unless (once restricted) `cost + spent > budget`, or (unless `flag`) its
   plane-ness differs from the first pick's. Cost is the int `class+124h` (100.0 `[00CE3D08]` in
   modes 4 and up).
2. A temporary record `{class, pos.x, pos.z, 1, class+134h, 009FDF30(id), U(+50h, +54h) stream 1, 0}`
   is appended (with `arg7 >= 0` the position is zero and the byte 0).
3. `total = 00A0C650(comp, threats, 1, arg7, buf, byte5, f) + sum_i 00A03900(comp[i], candidate)`.
4. With `total > 0`: `c = t+234h * total - t+238h * 00A0C650(threats, comp, 0, 100.0, 0, 0, 1.0)`.
   In modes 0..3 with `spent > 0.1`, `c` is blended:
   `c*(1-k) + (k*c) * spent/(cost+spent)`, `k = clamp(t+230h, 0, 1)`.
5. The best candidate must beat the round threshold strictly. The threshold starts at 0.001
   (modes 0..3) or -1e8, and after each accepted pick it is that pick's **total**, not its final
   weight.
6. A pick that would overrun the budget (modes 0..3) ends the composition when
   `spent > 0.75 * target`, else turns the budget filter on for later rounds; an affordable pick is
   bought: `spent += cost`, and its record is rebuilt with a **second** stream-1 draw.
The first pick fixes the plane-ness filter even when it was not bought. The return value is the last
bought pick's total.

**`00A03900`.** In modes 0..2 (`009FFC80`), the pair bonus is the first 0Ch row of the tuning
singleton's table (`+FA8h + mode*10h`) whose ids equal (earlier class, candidate class); else 0.

**`00A02180` and `00A07C10` return the minimum speed.** The ledger names said maximum
(`BSP_UnitList_MaxSpeed`, `BSP_AiGroupRecords_MaxSpeed`), but the seed is 1e10 (`[00CE4970]`) and a
value replaces it unless it is greater. So the spawn weight divides by the slowest class's speed. They
are renamed (section 4).

**The two planner arms that call `00A25A30`** (the host records both: src/game_hosts_ai.cpp
`capture_spawn_arm_00a2b400` and `defend_spawn_tail_00a29b8e`).

| | Capture `00A2B477..00A2B7A6` (in `00A29FD0`) | Defend `00A29BE7..00A29E2A` (in `00A28A60`) |
| --- | --- | --- |
| queue gate | `00946970(brain+20h) > 0` skips (`FCOMI`/`JA`; NaN passes) | the same |
| ramp | the byte `[ESP+4Fh]` (host `ai_tail_capture_spawn_due`) | none |
| budget w | `float(float((1 - pct) * 00942130) - 00A1C900)` (`DE E1`) | `float(float(pct * 00942130) - 00A1C900)` |
| budget gate | `1.0 > w` skips (NaN passes) | the same |
| choice | the strict best `+14h` above -1e10 among targets with `+2Ch > 0` | the first record with `+30h > 0` |
| amount | `+1Ch`, or w when `+1Ch > w` | `+30h`, or w when `+30h > w` |
| threats | `00A24870(entity)` | the record's own list `+20h` |
| call | `00A25A30(entity+FCh, amount, &threats, "[capture]"+id, 0)` at `00A2B758` | `00A25A30(entity+FCh, amount, &rec+20h, "[defend]"+id, 1)` at `00A29DAA` |
| after | a true answer stores planner+3Ch = 0.0 | nothing |

`pct` is `[00F8A8BC + 4*009FFC80()]`, the Defend resource percent. `00A1C900`, tagged
`STL_inst_00a1c900`, is game code: the sum of `00A2C530` over the planner's owned groups (renamed
`BSP_AiPlanner_OwnedResourceSum`). So the Defend spawn is the only one that may mix planes and
surface units in its composition (flag 1).
### 1.3 The party think schedule (`00A163D0`)

The controller constructor (`00A31730`, from `00A32350 BSP_AiController_Create`) initialises the
eight party slots: `+0h` = 0, attack and aggressive ratios = 0.5, the brain pointer = 0, and the
next think time = `[00F876A4] + U(3.0, 5.0)` on **stream 1**, one draw per slot in slot order.
The host's `GameAiCoordinatorHost::create_00a32350` (src/game_hosts_ai.cpp:5865) fills the next think
times with 0.0 and draws nothing.

### 1.4 The RETREAT command's tick (`00A156E0`, `00A11530`, `00A01230`)

include/bsp/ai_command_tick.hpp:194 records that the RETREAT tick is not bound in the host; the host
read it only to its first dispatch. After the follower pass `00A10DC0`, the formation wedge
`00A11690` and `00A13B60(0.75 [00CEE07C], leader, 0, 0)`:
1. **`00A11530`** samples the same ring as `00A113D0` (750 units around the leader, pi/6 steps
   accumulated as float while `2pi > a`: 13 samples; `DC C9` makes x = 750 sin a, z = 750 cos a).
   Each sample is scored by **`00A01230`**, `float(00A010F0(p, t) - 00A010F0(p, t == 0))`, with
   t = `(group+5638h == 0)`. The strict best above -1e10 gives the direction d. When no sample
   beats the seed the image leaves its out vector unwritten (the caller's stack); the model returns
   none and the tick does nothing.
2. `ahead = leader + d`, `behind = leader - d`. When
   `float(00A010F0(ahead, t) - 8.0 [00CE3DB0]) > 00A010F0(behind, t)` (`FCOMI`/`JBE`), the group's
   first member is ordered to `behind` through `00A02020` (the host's `issue_to_member` path).

So the retreat heads away from the direction where `00A010F0(t)` most exceeds `00A010F0(!t)`, and
only when that side is worse than the opposite one by more than 8.

**`00A2C9F0`**, the other user of `00A01230`, is the gate of the cautious arm of `00A2CBD0` (the host's
`ai_attack_arm_00a2cbd0` takes it as an input, src/ai_command_lifetime.cpp:64). It answers false for a
group with `+5638h >= 2` and when the target is within `tuning+1F0h` of the leader; otherwise it
scores the midpoint `(leader + target) * 0.5` with `00A01230` and answers `0.0 < v` (a NaN answers
true).

### 1.5 Team entity collection (`00A07D40`, `00A24870`)

`00A07D40(team, &records; filter, exclude_static)` walks the world list `+34h` and keeps an entity
whose `+5Dh` is clear and `+54h` is the team, which the filter does not exclude (vtable slot 0 on
the entity's position), which is kind 6, 18h or 1Bh, and (with `exclude_static`) not kind 1Bh, 45h
or 46h. `00A24870` uses the sphere filter of vtable `00D22CC0` (slot 0 = `00A1A660`) centred on the
target with `r^2 = float(tuning+1A4h squared)`, team = planner+34h, and falls back to the target's own
record. The host's labelled stand-in `capture_defenders_00a24870` (src/game_hosts_ai.cpp:4027)
matches these rules except that it reads `+5Dh` through `unit_active` and walks its world list 2.

### 1.6 Small group and planner helpers

- **`00A2C530`**, the group resource: the float sum of `member+304h`. The host substitutes the
  constant 0 (src/game_hosts_ai.cpp:3814, `kCaptureGroupResource`), on the stated ground that the
  field is 0 on this installation; the model is the image's rule for when it is not.
- `00A18480` and `00A1D110`/`00A1D140`/`00A1D170`/`00A1D1A0` (the planners' needs-replan slot) were
  first modelled here and then found already reconstructed in src/ai_planners.cpp
  (`ai_planner_needs_replan_base`/`_mode`); the duplicate was deleted and the records point there.
- **`00A2E1E0`** counts unit types for the Lua group info: `map[member+170h key] += 009FE060(member)`.
- **`00A1A6D0`**, from the script group creator `00A16EF0`: a group with a groupable combatant
  (`00A2C5A0`) patrols to the entity, any other defends its position.
- **`00A04530`**: `float(00A04240(unit) * 009FDF30([unit+C4h]))`, the factor `00A046C0` also applies.

### 1.7 Which parties the AI runs (`00A32DF0`)

`00A32DF0` is slot 0 of the controller entity's vtable `00D23240`, its activation. Besides naming the
entity "HighLvlAI", it decides which party records are enabled:
1. In effective game modes 0..3, each half of the eight player slots (0..3, 4..7) elects at most one
   AI slot. When any slot of the half has `+8h` set and `+9h` clear, no slot of the half is elected
   (`+0Ah` cleared on all four); otherwise the half's first slot is elected (`+0Ah` = 1, the others 0).
2. Party record `p` is enabled when its slot has `+8h`, `+9h` and `+0Ah`; an enabled record's two
   ratios are reset to 0.5.
3. `[00F8AB6A]` is cleared, and when `[game+61Ch]` is set, record 0 is enabled regardless.

The host (src/game_hosts_ai.cpp:5866) enables a party slot when a created unit carries that side, and
its comment attributes the records to the mission's `SetParty` binding. In the image they come from the
player slot flags here (the Lua binding at `00A374C4` can still write them later). Whether `+8h`/`+9h`
are "present"/"AI-controlled" was not established beyond this use; the field names are hypotheses.

## 2. Adoption table (cc9 terms)

| model | replaces (host file:line) | inputs and where the host gets them | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `ai_party_brain_engagement_pass_00a179e0` (+ `ai_powerup_*`, `ai_candidate_attack_value_00a0f680`) | `AiParties::party_brain_plan_tail` (src/game_hosts_ai.cpp:5078), a record | the party's ready power-up entries (no manager in this process: `PowerUps::pre_pass`/`post_pass` are records); the team group lists, populations, parties, first-member poses, `00A2C4C0` strengths; the class `+8h`/`+84h`/`+8Ch`; unit health, kinds, class `+804h`, `00A04240`, `009FDF30`; `008E35F0` | **stream 1**: one `U(t+50h, t+54h)` per `00A0F680` call (`00A0F737`), per entry, per pair, only after `008E35F0` accepts | OFF: none. ON with a power-up manager: an AI party with an engagement pair fires its best ready power-up each think. Air-support entries shift stream 1 by one draw per accepted pair, every think, even when nothing is fired |
| `ai_try_spawn_group_gate_00a25a30`, `ai_sort_spawn_sites_00a236f0`, `ai_choose_spawn_composition_00a23980`, `ai_match_final_weight_00a0d1d0`, `ai_class_pair_value_00a03900`, `ai_unit_list_min_speed_00a02180` | the Capture spawn arm, counted only (src/game_hosts_ai.cpp:4383 `spawn_due`, :4423) and the Defend arm; `quick_spawn_named_group` (src/game_hosts_ai.cpp:5465) for the mode planners | `0066E590`'s sites with their stock (`0084D560`), `00942130`, mode tuning `+50h..+54h`, `+100h..+10Ch`, `+230h..+238h`, the class-pair table, `00A0C650` both ways (host `compose_attack_value_00a0c650`), `00419010`, `0084CF80`; the request goes to `0094C830`, which ends at the SpawnNew queue the host drains | **stream 1**: one draw per candidate tried per round and one more per bought pick (`00A0D604`, `00A0DAB8`/`00A0DB21`) | OFF: none. ON: nothing changes on the reference rows (no authored stock, docs/SHIP_AI_OPEN_ITEMS.md 52.2). On `ijn_11_operation_to` and the three `ijn_02_force_z` copies, Capture and Defend thinks that pass the gates buy units and shift stream 1 |
| `ai_init_party_think_schedule_00a163d0` | the `next_think.fill(0.0f)` in `create_00a32350` (src/game_hosts_ai.cpp:5865) | the float clock at creation | **stream 1**: 8 draws at controller creation | OFF: none. ON: every party's first think moves from 0 s to 3..5 s, and every later stream-1 consumer shifts by 8 draws |
| `ai_retreat_tick_00a156e0`, `ai_retreat_pick_direction_00a11530`, `ai_net_danger_at_point_00a01230` | none: the RETREAT tick `00A156E0` is not bound (include/bsp/ai_command_tick.hpp:194) | the host's `tick_danger_cost_00a010f0` (src/game_hosts_ai.cpp:3078), the leader point, the prelude (`00A10DC0`, `ai_formation_wedge_00a11690`, `00A13B60`), `issue_to_member` | none | OFF: none. ON: a group under a RETREAT command (Lua `00A13340`, or `00A2BE70`, which is unreferenced) moves its leader 750 units away from the worst direction when that side's cost exceeds the opposite side's by 8 |
| `ai_should_route_cautiously_00a2c9f0` (with the existing `ai_attack_arm_00a2cbd0`) | the unconditional draw in `order_attack` (src/game_hosts_ai.cpp:5772) | group `+5638h`, tuning `+1F0h`, the own leader, the target group's leader (`00A10C20(target)`), `00A010F0` | the arm's draw `00BD2F40` is **stream 1** (`00A2CC51 MOV ECX,1`) and happens only after both gates pass; the host draws its private LCG on every order | OFF: none. ON: a group led by a ship never takes CAUTIOUSATTACK; other groups take it only when the target is farther than `+1F0h` and the midpoint scores above 0, then with probability `1 - aggressive`; stream 1 shifts by one draw per such order |
| `ai_collect_team_entities_00a07d40`, `ai_collect_threats_near_00a24870` | the labelled stand-in `capture_defenders_00a24870` (src/game_hosts_ai.cpp:4027) | the world list `+34h`, `+5Dh`, `+54h`, the kinds, tuning `+1A4h` | none | OFF: none. ON: the same set when the host's world list 2 is the image's `+34h` list and `unit_active` is `!+5Dh`; any difference shows as different capture group values |
| `ai_capture_spawn_arm_00a2b477`, `ai_defend_spawn_arm_00a29be7`, `ai_planner_owned_resource_00a1c900` | the records `capture_spawn_arm_00a2b400` (src/game_hosts_ai.cpp:4424) and `defend_spawn_tail_00a29b8e` | `00946970` (queued requests), the Defend percent, `00942130`, the owned groups' `00A2C530`, the plan targets and defend records | none | OFF: none. ON: the same decision the host counts, plus the amount and the threats handed to `00A25A30`; nothing spawns without the chain above and authored stock |
| `ai_group_resource_sum_00a2c530` | the constant `kCaptureGroupResource` (src/game_hosts_ai.cpp:3814) | `member+304h` | none | OFF: none. ON: none on this installation, where `+304h` is 0 for every member (the host's own finding); otherwise capture prices drop by the group resource |
| `ai_group_count_unit_types_00a2e1e0`, `ai_order_defend_group_00a1a6d0`, `ai_hinted_class_weight_00a04530` | none (the Lua group info, the script creator's order) | member `+170h`, `009FE060`; `00A2C5A0`; `00A04240`, `009FDF30` | none | OFF: none. ON: neither the Lua info nor the script order is bound |
| `ai_controller_init_party_slots_00a32df0` | the side scan in `create_00a32350` (src/game_hosts_ai.cpp:5866..5875) | the eight player slot records `[game+18CCh+4p]` `+8h..+0Ah`, `004BCA50`, `[game+61Ch]` | none | OFF: none. ON: the AI runs exactly the parties the slot flags elect, one per half at most in modes 0..3; a side with units but no elected slot stops thinking, and an elected slot without units thinks with no groups |

No model draws from stream 0.

## 3. For cc9 (no host edits made)

- **`00A02180` / `00A07C10` / `00A07FB0` are minimums,** renamed (section 4). The only other reader,
  the speed bonus of `00A0C650`, is multiplied by tuning `+22Ch`, which every table authors 0
  (src/game_hosts_ai.cpp:5397), so no host behaviour depends on it today.
- **`00A23980`'s request carries the tag, not the flag** (section 1.2).
- **The host never seeds the think schedule** (section 1.3): 8 stream-1 draws and a 3..5 s first-think
  delay per party are missing.
- **`009FFE50`** answers false in the host for the whole multiplayer arm; the image tail-calls
  `004B5510` on the slot record. **`009FFC80`**: with no active party slot the image answers 1, not 0.
- **`order_attack` (src/game_hosts_ai.cpp:5755) skips both gates of `00A2CBD0`.** The image
  (`00A2CC2E..00A2CC65`) takes MOVETOATTACK without a draw when the group's first member is a ship
  (vtable `+5Ch`(6)) or when `00A2C9F0(group)(00A10C20(target))` is false, and only then draws
  `00BD2F40` on stream 1. The host draws its private LCG for every order and goes CAUTIOUSATTACK with
  probability `1 - aggressive` even for ship groups. The pure arm `ai_attack_arm_00a2cbd0`
  (src/ai_command_lifetime.cpp:62) already encodes the image's order; it is not called.
- **The party records come from the slot flags** (`00A32DF0`, section 1.7), not from units' sides.
- **The power-up use** is complete as a model; its binding still waits on a power-up manager
  (docs/SHIP_AI_OPEN_ITEMS.md section 5).

## 4. Ledger

Function records (`config/reconstruction`), models in `src/gameplay_seg69_70_cc13.cpp`:
- the power-up use: `00A179E0`, `00A046C0`, `00A04860`, `00A04910`, `00A0F680`;
- the think schedule: `00A163D0`;
- the spawn chain: `00A02180`, `00A07C10`, `00A03900`, `00A0D1D0`, `00A236F0` (partial: tie order above
  32 sites), `00A23980` (partial: the mode 4+ arm), `00A25A30` (partial: the gate), `00A1C900`;
- the retreat: `00A01230`, `00A11530`, `00A156E0` (partial: the prelude), and `00A2C9F0`;
- the collection: `00A07D40`, `00A24870`;
- helpers: `00A2C530`, `00A2E1E0`, `00A1A6D0`, `00A04530`;
- the party slots: `00A32DF0` (partial: the entity name and `00923840`).

Fragments: `00A2B477` (in `00A29FD0`, to `00A2B7A6`) and `00A29BE7` (in `00A28A60`, to `00A29E2A`).

Renames (`config/names`, applied with `tools/ghidra_annotate.py --apply`; the prior annotations are in
that tool's local JSON, and each new evidence names the old value):

| address | was | now |
| --- | --- | --- |
| `00A02180` | `BSP_UnitList_MaxSpeed` | `BSP_UnitList_MinSpeed` |
| `00A07C10` | `BSP_AiGroupRecords_MaxSpeed` | `BSP_AiGroupRecords_MinSpeed` |
| `00A07FB0` | `BSP_AiGroup_MaxSpeed` | `BSP_AiGroup_MinSpeed` |
| `00A1C900` | `STL_inst_00a1c900` (tag only) | `BSP_AiPlanner_OwnedResourceSum` |

Records for existing reconstructions, each checked against the listing in this pass
(`local/gpr69_dis.py`, `local/gpr69_clonecheck.py`, not committed):
- the AI command GetType slots (12, to the `AiCommandType` enum in include/bsp/ai_planners.hpp) and
  IsType slots (15, to `ai_command_is_type` in src/ai_planners.cpp);
- in src/band_009f_cc13.cpp (cc13-gp9f's models): the twelve CloneForGroup slots (partial: the type
  only), the planner IsType and GetName slots (16), `00A2C0A0`, `00A2C0D0`, `00A2C100`, `00A16450`,
  `00A15950`, `00A1A660`, `00A2C150`, `00A2C1C0`, `00A2C230`, `009FEC40`, `00A1C960`, `00A1CA60` (the
  `D8 C9`, `DE FA`, `D8 F1`, `D8 7F` forms read from the bytes), `00A2B8F0`, `00A2B950`, `00A2BCC0`,
  `00A2C3D0` (the model divides by the list size where the image divides by `+5644h`);
- in the host and its rule files: `00A113D0` (src/ai_command_tick.cpp:526), `00A18510`, `00A18520`,
  `009FFE50` (partial: the multiplayer arm), `00A10040` (partial: src/game_hosts_ai.cpp:1620 models the
  target arm; the other arm installs a null command), `009FE200`, `009FFD80`, `009FFC80` (partial: the
  no-active-slot answer is the caller's), `00A11F80` and `00A12450` (`ai_command_can_merge`), `00A18480`,
  `00A1D110`, `00A1D140`, `00A1D170`, `00A1D1A0` (src/ai_planners.cpp `ai_planner_needs_replan_*`),
  `00A2CB10` (`ai_group_command_is_engaged`; the radius squared is 10000.0 `[00CE3D64]`),
  `00A2BE20` and `00A2C310` (src/game_hosts_ai.cpp:4817 and :4837, the DEFENDPOSITION and PATROLTO
  installs).

A duplicate was caught: `00A18480` and the mode planners' needs-replan slot were first modelled in
this packet, then found in src/ai_planners.cpp; the duplicate code was deleted before commit of the
records that point at the existing functions.

## 5. Verification

- `./scripts/build.ps1` passed in this tree: the build and ctest (2 of 2). No test was added.
- `tools/verify_report_calls.py local/gpr69_calls.json` checked 72 call-site rows cited here (the power-up pass, the scorers, the spawn chain, the retreat tick, the collection), with 0 failures. The row file is not committed.
- `tools/x87_dc_form_scan.py --range 009FE130-00A371A0 --by-function` finds 12 DC-form sites in 10 functions. Two are in modelled functions, `00A115A0` (`00A11530`) and `00A17C82` (`00A179E0`); both were read from the bytes (`DC C9`). The `DE` pop forms of `00A0D1D0` and the `D8` forms of `00A1CA60` were read from the bytes as well.

## 6. Open and handoff

Counts. The band held 301 functions with a name but no record and no tag at the start (index rebuilt
after `git merge origin/main`). This packet added 117 records citing this doc: 27 models in
`src/gameplay_seg69_70_cc13.cpp` (two of them fragments) and 90 listing-checked records for existing
reconstructions. 187 of the 301 remain without a record:

| kind | count | note |
| --- | --- | --- |
| EH funclets (`Catch_All@`, `Unwind@`) | 21 | no record needed |
| debug writers and describers (`*Debug*`, `*Describe*`) | 31 | unreferenced or presentation; docs/CC13_GP_009F_00A3.md lists them |
| reconstructed in the hosts but not checked here | 96 | the big ones first: `00A0C650`, `00A13B60`, `00A14DD0`, `00A28A60`, `00A29FD0`, `00A1E250`, `00A243D0`, `00A250A0`, `00A1A7A0`, `00A2E4C0`, `00A2DB80`, `00A02020`, `00A04560` |
| no caller in the index and no reconstruction | 26 | unreferenced predicates (`009FE130`, `009FE170`, `009FE1B0`, `009FFE00`, `009FFF20`), destructor bodies, `00A2BE70`, `00A2C2A0`, `00A185E0`, `00A22920`, controller arrays |
| others | 13 | `00A08020`/`00A08420` (the mode planners' quick-spawn records, stream-1 jitter), `00A31C20`/`00A31C90` (multiplayer party messages), `00A12580`, `00A13080`, `00A2BD50`, constructors |

Next, by value:
1. **cc9 decisions** (section 3): the `order_attack` gates and stream-1 draw, the party-slot election,
   the think-schedule seed. Each is one host call into an existing model.
2. **Check the large host reconstructions** listed above against the listing and record them; they are
   cc9's code, so any divergence goes to cc9 as a note, not an edit.
3. **`00A08020`/`00A08420`**, only if a mode planner (modes 4..7) is ever run.

Not done: no Ghidra comment beyond the four renames; no game run (cc9's terms).

## 7. gpr69b: the host reconstructions checked against the listing

Packet `cc13-gpr69b` (cc13 lineage, Claude) continues this one on the same terms: the cc13 files
only, ledger records, no host edit, no wiring, no game run. Each body below was read whole from the
disk bytes with Capstone `disasm-raw`; the x87 forms of the arithmetic tails were read from the bytes
(`local/tools_scene2/sc2_x87.py` in the cc13 tree). Constants were read from the PE
(`local/g69b_pe.py`, not committed). New models are in `src/gameplay_seg69_70_cc13.cpp` and are
**reconstructed and build-tested**; none is ABI-compatible, wired or game-validated.

### 7.1 `00A0C650` with every argument (`00A0C330`, `00A0C3C0`)

The host has `compose_attack_value_00a0c650` and `group_value_pair_00a0c3c0`
(src/game_hosts_ai.cpp:5375 and :5229) for the one argument set `00A0F970` and `00A250A0` pass,
`(0, -1.0, no debug, 0, 1.0)`, and they match the listing on that set. `00A23980` (the spawn
composition, section 1.2) also calls `(1, arg7, debug, byte5, factor)` and `(0, 100.0, 0, 0, 1.0)`,
so this pass models the general body:

- `00A0C3C0` uses its distance argument as given unless it is negative (`COMISS`/`JBE`); only then
  does it take `00414C60` over the pose delta (both `+0Ch` bytes set) or 0. `00A23980`'s fixed 100.0
  therefore puts every threat pair inside the arrive distance. `arg1` is not read.
- `00A0C650`: with `byte5` set, every attacker but the last scores with the rnd flag clear (no
  `+18h` spreads) and is multiplied by `factor`, or by 0.001 (`[00D7A23C]`) when 0.001 (`[00D7A318]`)
  is greater. Both the per-attacker and the per-target best are kept (from 0.0). With `arg1` set, three
  penalties apply: `+21Ch` x attackers whose best is <= 0, `+220h` x targets whose best is <= 0, and
  `+224h` x pairs of attackers sharing `record+0h`. The tail, from the bytes:
  `result = f(speed + base) - ((unattacked + idle) + repeat)`, then 0 when `0 > result`.

### 7.2 `00A13B60`, the CLOSEATTACK/DEFENDPOSITION pass: ten divergences

`ai_close_attack_pass_00a13b60` is the whole body as a model. The host's reconstruction
(src/ai_close_attack_tick.cpp:71) says its collection walk was "read for its bounds and its filter
but not instruction by instruction". Read whole, the image differs from the host in these points
(cc9's code; no edit made):

1. **The candidate set is five world-registry lists, not every unit.** `[[00E188A8]+19CCh] + 1Ch +
   id*0Ch` for ids 6 (ships), 24 (plane squadrons), 45h, 46h and 1Bh (the `009FE0B0` trio), in that
   order, each entity with `+5Ch` set and `+5Dh`, `+60h`, `+5Eh` clear.
2. **The team rule depends on the list.** Ships and squadrons must be on the enemy team,
   `+54h == (group+5638h == 0)`; a third-party ship (`+54h >= 2`) is never a candidate. The trio lists
   take any entity not on the own team, so neutral buildings are. The host takes `team != own` for all.
3. **Ties go to list order** (ships, squadrons, 45h, 46h, 1Bh, each in list order); the host's order
   is its unit index.
4. **The collect test is `f(dz^2 + dx^2) < r^2`, strict and without a square root;** the host takes
   `sqrt` and keeps `d^2 <= r^2`.
5. **The squadron gate calls `009FFEB0`.** After the inline `007EDA90` shape (leader kind 17h with
   `+C24h` clear skips), the image calls `009FFEB0 BSP_AiCommand_SendEmptySquadronHome` and skips the
   squadron when it answers true; that call may itself issue the squadron's return order. The host's
   `close_member_squadron_excluded` answers `007EDA90` only, although the host already has
   `tick_squadron_excluded_009ffeb0` (src/game_hosts_ai.cpp:2723).
6. **The member's current target joins the list** when it lies outside the collect radius but inside
   `2 r^2` (`00A14604`-`00A1464C`); it stays in the list for the members after it in the same pass.
7. **The current target only counts when it answers `vtable+5Ch(2)` and is on the enemy team**
   (`00A144C8`-`00A144F1`); otherwise no sticky multiplier applies.
8. **The best seed is -1e10 (`[00CE4ADC]`), not 0.** An admitted candidate scoring 0 (beyond
   `+1FCh` FarDist, or a zero-weight target-group member) or less still wins over "none", so the member
   gets an order where the host sends it to the fallback.
9. **Product order:** `(sticky * group) * (range * weight)` (`00A14990`-`00A1499C`); the host multiplies
   `((range * weight) * group) * sticky`. A last-bit difference when both multipliers apply.
10. **Fallback rounding:** each of `0.4 * pos` and `0.6 * centre` is stored as a float before the add
    (`00A14CC1`-`00A14D41`); the host forms the sum in double.

The host's `kCloseAttackFallbackOffsetBound` arm matches otherwise (radius `max(pop * 160, 400)`,
strict `>`).

### 7.3 `00A28A60`, the Defend think

The host (src/game_hosts_ai.cpp:4629) matches passes B and C, the hand-off, the anchor search, the
resource subtraction, the orders and the merge pass. Two divergences and one model:

1. **Pass A misses half of `00A2DEF0`.** A member is a defend candidate when it answers
   `vtable+5Ch(1Ch)` **or** `008DDF90` finds it in the team's set `[game + 21A4h + team*4]`
   (`ai_defend_member_is_candidate_00a2def0`). The host collects only kind 1Ch.
2. **The records are sorted.** `00A28300` is the STL introsort; with 32 or fewer records only the
   insertion pass `00A25750` runs, whose predicate puts `x` before `y` when `0 < x.T` and
   `y.weight < x.weight` (record `+4h`, `+8h`). `ai_defend_sort_records_00a28300` returns that order
   (nullopt above 32, whose partition path was not read). The host keeps build order, so with two or
   more records the nearest-record search and the spawn tail can pick differently.
3. The anchor lists are a map keyed by the entity pointer (labelled in the host).

`00A243D0` (the record scorer, src/game_hosts_ai.cpp:4530) matches except: the requirement is
`+1E8h * (+19Ch * b)` in the image and `(1.5 * point) * b` in the host; the 1Ch skip is `IsKindOf`
in the image and a class-id test in the host; a NaN `00A03510` weight is not skipped
(`FCOMPI`/`JAE`); the Lua `StrategicGain` is fixed at 0 by the host.

### 7.4 `00A29FD0`, the Capture think, and `00A1E250`

The host's target path (src/game_hosts_ai.cpp:4099, :4371) matches the records, the price, the group
weights, the assignment loop's order and tests, the price decrement, the merges and the hand-off.
Divergences:

1. **The no-target path hands off per group, not per side.** With no enemy list-28 entity the image
   still builds the group records; each unassigned group goes to `brain+8h` when its record found a
   nearest own list-28 entity **not in that group** (`node+1Ch`), else to `brain+4h`. The host
   (src/game_hosts_ai.cpp:3755) sends every group to slot 2 whenever any own CommandBuilding exists.
2. **The target test has no activity filter in the image:** a list-28 entity with `+54h` other than
   the planner's side is a target; the host requires `unit_active` and kind 1Ch over all units.
3. **The clock prologue runs on every think** (`00A29FF7`-`00A2A0C1`): `+40h`, `+38h` and `+3Ch` advance
   even with no target. The host advances them only on the target path, so the planner age that drives
   the spawn ramp starts at the first think with a target instead of the first think. Model:
   `ai_capture_planner_clock_00a29fd0`.
4. **The length epsilon is 1e-10.** `[00CE3820]` holds the double 1e-10; the host's
   `kAiTailLengthEpsilon` and the literal in `capture_plan_00a29fd0` use 1e-11.
5. **Score rounding:** `f(+1BCh * s + d((1 - +1BCh) * w))` with each step at 24-bit precision
   (`ai_capture_assignment_score_00a2ac11`); the host forms it in double.
6. `00A1E250`: the image divides by `+19Ch` without the host's substitute of 35 for 0, and forms
   `res * ((1 / pv) * factor)`; the host forms `res * factor / pv`.

The `[00F8A9E0] == 1` arm (group `+5624h`/`+5628h` resets and the `00A2B950`/`00A2BCC0` calls) is not
modelled; nothing in this process writes `[00F8A9E0]`.

### 7.5 `00A14DD0`

The CAUTIOUSMOVE route pass (src/ai_command_tick.cpp:415) matches the listing, including the x87
forms of the three candidate points (`base`, `base + off`, `base - off`).

### 7.6 `009FE270`, the bullet-type accuracy: two arms are conditional

`ai_bullet_type_accuracy_offset_009fe270` (src/ai_tuning_globals.cpp:219) reproduces the dispatch
(the byte table `009FE6EC` and the jump table `009FE6C4` were read from the PE) and every four-way arm,
but the function first calls the target class's `vtable+1Ch` (its class id) and two arms test it:

- DepthCharge (sub-type 0Bh, `009FE44A CMP EAX,8 / JNE 009FE6BB`) answers `+148h` only against
  class id 8; any other target gets 0.0.
- Paratroopers (0Fh, `009FE465 CMP EAX,1Ch / JNE`) answers `+14Ch` only against class id 1Ch.

The host answers both unconditionally. Any caller that weights a depth charge against a surface
target, or a paratrooper drop against anything but a CommandBuilding, gets a non-zero accuracy the
image does not give.

### 7.7 The command ticks: `00A11070` runs after almost every tick and is not in the host

`00A11070 BSP_AiCommand_FormationShapePass` is the tail of the MOVETO, MOVETOATTACK (closing arm),
PATROLTO, IDLE, REGROUPINGMOVE and SELLING (ground arm) ticks. For a group led by a ship with a
formation object (`leader+284h`) it applies shape 0 (`0070EFD0`), then gives every later ship member
that has a formation record the next of 24 fixed slots, `record+10h/+20h = -(x, z) * tuning+210h`
(the table at `00F8A7B0`, filled once), and broadcasts `FORMATION_SET`. The host records it on the IDLE
arm (src/game_hosts_ai.cpp:2185) and omits it elsewhere. Model: `ai_formation_shape_00a11070`.

Also modelled:
- `ai_regrouping_move_tick_00a126c0`: REGROUPINGMOVE (`00A126C0`, not in the host). With a follower
  beyond `AutoMerge_LeaveDist` from the leader, the leader goes to `(dest + 3 * centroid) / 4` of all
  members (each step stored as a float); otherwise to the destination.
- `ai_order_cautious_move_00a2ccf0`: the move twin of `00A2CBD0`, called only by the Duel think
  `00A25F70`; the stream-1 draw happens only for a non-ship leader that `00A2C9F0` routes cautiously.

Checked and matching: `00A02020` (the order bridge), `00A10EC0` (the auto-merge leave pass),
`00A11690` (the wedge, spot-checked from the bytes), `00A0F810` (the candidate weight, epilogue
`((B*C)*D)*A`), `00A12A90` and `00A15570` (apart from `00A11070`). `00A11FF0` (SELLING) differs in
three labelled ways: the host issues `returntobase` per plane where the image orders the squadron, it
omits `00A11070`, and it records the per-member session message 51h. `00A04560` matches apart from the
host's labelled spread and class query.

### 7.8 The mode planners: Duel and Escort have their own bodies

The host's `ai_mode_planner_tick` (src/ai_planners.cpp) runs `00A1CB80` on the first owned group for
all four mode planners. That is right for Siege (`00A26510`) and Competitive (`00A265F0`, whose reset
flag is a constant 0), but the Duel and Escort thinks do something else:

- **Duel (`00A25F70`)**, model `ai_duel_think_00a25f70`: the nearest enemy group (`00A1C960`) is attacked
  (`00A2CBD0`, aggressive 1.0) when it is within 4000 (`16e6 > d^2`) or the own team has one group;
  otherwise the group moves (`00A2CCF0`, ratio 1.0) to a blend of the two team centroids,
  `own * t + enemy * (1 - t)` with `t` rising from 0 at 500 to 1 at 2000 units between the leader and
  the own centroid. With no enemy group it moves to the own centroid. `00A2CCF0` with ratio 1.0 always
  installs MOVETO but still draws stream 1 when its gates pass.
- **Escort (`00A26210`)**, model `ai_escort_think_00a26210`: the nearest entity of the team's objective
  set (`[game + team*4 + 21A4h] + 18h`) decides: an own entity is escorted (PATROLTO to the midpoint of
  it and the enemy centroid), an enemy entity is attacked through its AI group or patrolled to; only an
  empty set falls back to `00A1CB80`.

`00A1CB80` itself matches but for the product order (`((range * base) * own) * sticky` in the image),
the host's near radius (9e6 in `near_radius_squared()`, 1e-10 in the image; no effect while
`FreeAttack_NearDist` is 5000 or 9000) and the per-candidate target-list records it writes when reset.

Also checked and matching: `00A22800` (the Sell think), `00A2E4C0` (the air split), `00A11B80` (the
transport moves, spot-checked).

**The constant at `00CE3820` is the double 1e-10** (`BB BD D7 D9 DF 7C DB 3D`). `kAiPlannerNearRadiusSquared`
(include/bsp/ai_planners.hpp:218), `kAiTailLengthEpsilon` (include/bsp/ai_planner_tails.hpp:68) and the
literal in `capture_plan_00a29fd0` (src/game_hosts_ai.cpp:4180) say 1e-11; include/bsp/gameplay_seg33_cc13.hpp
already notes the same.

### 7.9 `00A03760`: the arrival value's speed guard is inverted in the host

`00A03760 BSP_AiCapture_UnitArrivalValue` feeds the `a`/`b` terms of every Capture target score
(`00A1E250`). Outside the CommandBuilding's capture range the bytes at `00A0382C` are

```
DD 05 A0 A3 D7 00   FLD  qword [00D7A3A0]      ; 0.1
F3 0F 11 44 24 08   MOVSS [esp+8], xmm0         ; the unit's class speed
D9 44 24 08         FLD  dword [esp+8]
DB F1               FCOMI ST0, ST1              ; speed vs 0.1
DD D9               FSTP ST1
77 54               JA   00A03896               ; speed > 0.1 -> return 0.0
```

so any unit faster than 0.1 contributes **nothing** outside the range, and only a unit at or below
0.1 (or NaN) reaches the linear `(1 - time / limit) * w` arm. The host's
`ai_tail_unit_arrival_value` (src/ai_planner_tails.cpp:92) has the guard the other way round: it
returns 0 for speed <= 0.1 and gives every moving ship the approach value. On the image's rule the
Capture terms count only units inside the capture range. Model: `ai_unit_arrival_value_00a03760`.
Whether the image's rule is an original bug does not matter for parity; the host should follow the
bytes.

Also checked: `00A03510` (the capture weights; the host fixes the CommandBuilding's Lua
`CaptureWeight` at 1.0), `00A0F970`, `00A250A0`, `00A1A7A0`, `00A1D010` (matching), `00A1A720` (the
image passes the party slot's aggressive ratio `[00F8A8D0 + party*1Ch]`, the host the constant 0.5)
and `00A22750` (the host's labelled refusal of a group another planner holds).

### 7.10 Smaller checks

- `00A2DB80` (the merge): every ATTACK-family command aimed at the absorbed group is replaced by a
  **new** command of the same class (`00A10890`, `00A109B0`, `00A10AE0`, installed by `00A2BD00`); the
  host rebinds the existing command's target in place, so state the constructors reset (the
  CAUTIOUSATTACK route counter `+28h = 4`, its flag `+24h = 0`, the member-pass schedule) survives a
  merge in the host.
- `00A010F0` (the danger cost): the image walks list 5 (`world+58h`) by team with no liveness test;
  the host walks its active units.
- `00A15970`: matches for modes 0..3; for modes 4..7 the image asks the mode planner's `vt+30h`, the
  host answers false.
- `00A32350` / `00A31730`: the controller creation also clears the forced-target-weight tree
  (`00A03A60`); the think-schedule seed and the party-slot election are sections 1.3 and 1.7.
- `00A1CF90`, `00A152B0`, `00A11F70`, `00A31DB0`, `00A13340` (the name-to-recipe step) and `00A16EF0`
  (the routing) match. `00A12430` (IDLE) matches but for `00A11070`.

### 7.11 Adoption notes for the gpr69b models (cc9 terms)

| model | replaces (host file:line) | inputs | RNG | OFF-switch prediction |
| --- | --- | --- | --- | --- |
| `ai_compose_attack_value_00a0c650` (+ `ai_pair_attack_value_00a0c3c0`, `ai_pair_base_value_00a0c330`) | `compose_attack_value_00a0c650` / `group_value_pair_00a0c3c0` (src/game_hosts_ai.cpp:5375, :5229) for the general argument sets | the `AiValueRecord`s, `00A08460`, the class kind and speed, tuning `+D8h..+FCh`, `+214h..+22Ch`, `00A07C10` | none | OFF: none. ON with the host's `(0, -1.0, 0, 0, 1.0)`: identical numbers. ON for `00A23980`'s two calls: the spawn composition gains the byte5 factor, the penalties and the fixed distance 100.0 |
| `ai_close_attack_pass_00a13b60` | `ai_close_attack_tick_00a13b60` (src/ai_close_attack_tick.cpp:71) | the five world lists in order with `+5Ch/+5Dh/+5Eh/+60h/+54h`, member gate inputs, current target (kind 2), `009FFEB0`, `00A0F810`, `00A2C720`, tuning `+1F4h..+204h` | none in the pass; `009FFEB0` may issue a squadron's return order | OFF: none. ON: neutral ships stop being targets and neutral buildings start; empty bombers go home instead of being ordered; a member's current target up to sqrt(2) x the collect radius stays a candidate; zero-score admitted candidates are ordered instead of falling back |
| `ai_defend_member_is_candidate_00a2def0`, `ai_defend_sort_records_00a28300` | pass A and the record order of `defend_think_00a28a60` (src/game_hosts_ai.cpp:4680, :4733) | member kind 1Ch, `008DDF90` on `[game+21A4h+team*4]`; record T and weight | none | OFF: none. ON: members of the team's objective set become defend records; with two or more records the anchors and the spawn tail follow the sorted order |
| `ai_capture_planner_clock_00a29fd0`, `ai_capture_assignment_score_00a2ac11` | the dt block of `capture_target_path_00a29fd0` (src/game_hosts_ai.cpp:4376) and the score at :4205 | the float clock, `+38h/+3Ch/+40h`, tuning `+1C0h..+1C8h`, `+1BCh` | none | OFF: none. ON: the Capture planner age counts from the first think, so the spawn ramp is further along when a target appears; last-bit score changes |
| `ai_formation_shape_00a11070` | nothing (recorded on IDLE, src/game_hosts_ai.cpp:2185) | leader kind 6 and `+284h`, `0070D080` records, tuning `+210h` | none | OFF: none. ON: ship groups under MOVETO/MOVETOATTACK/PATROLTO/IDLE/REGROUPINGMOVE/SELLING get the 24-slot box formation offsets every tick (`FORMATION_SET` broadcast) |
| `ai_regrouping_move_tick_00a126c0` | nothing (REGROUPINGMOVE has no host tick) | member poses, `+8h`, tuning `+20Ch`, `[00F87574]` | none | OFF: none. ON: a Lua `REGROUPINGMOVE` group with stragglers steers to `(dest + 3 * centroid) / 4` |
| `ai_order_cautious_move_00a2ccf0`, `ai_duel_think_00a25f70`, `ai_escort_think_00a26210` | `ai_mode_planner_tick` (src/ai_planners.cpp) for Duel and Escort | the nearest enemy group and its d^2, team centroids `00A1CA60`, the objective set, `00A2C150`, `00A2C9F0` | **stream 1**: one `00BD2F40` per `00A2CCF0` call that passes the ship and `00A2C9F0` gates | OFF: none. ON: only in effective modes 4 (Duel) and 5 (Escort); Duel groups hold near their centroid until within 4000 of an enemy, Escort groups follow the objective set |
| `ai_unit_arrival_value_00a03760` | `ai_tail_unit_arrival_value` (src/ai_planner_tails.cpp:92) via `CaptureScoreHost::unit_arrival_value` (src/game_hosts_ai.cpp:3912) | weight, distance, `+7A0h`, class speed, tuning `+198h` | none | OFF: none. ON: moving units outside a CommandBuilding's capture range stop counting in the Capture `a`/`b` terms; target totals and the assignment change wherever such units were approaching |

None of the new models draws from stream 0.

### 7.12 Divergences for cc9, by expected effect

1. `00A03760`: the arrival value's speed guard is inverted in the host (7.9).
2. `00A13B60`: the candidate set, the team rule per list, the `009FFEB0` squadron call, the current
   target's extended radius and the -1e10 best seed (7.2, items 1, 2, 5, 6, 8).
3. `00A11070` is not run after the MOVETO, MOVETOATTACK, PATROLTO, IDLE, REGROUPINGMOVE and SELLING
   ticks (7.7).
4. `00A28A60`: pass A misses objective-set members, and the records are not sorted (7.3).
5. `00A29FD0`: the no-target hand-off and the clock prologue (7.4, items 1 and 3).
6. `009FE270`: DepthCharge and Paratroopers are conditional on the target class id (7.6).
7. Duel and Escort run `00A1CB80` in the host (7.8).
8. `00A2DB80`: rebinding instead of new commands (7.10).
9. `00A1A720` and `00A1CF90` use the constant 0.5 for the party's aggressive ratio (labelled).
10. The constant `00CE3820` is 1e-10 (7.8), and last-bit product/sum orders in `00A0C650` users,
    `00A13B60`, `00A1CB80`, `00A243D0`, `00A1E250` and the capture score.

### 7.13 Ledger (gpr69b)

Function records added (`config/reconstruction`, all citing this doc), 52 in all:
- models in `src/gameplay_seg69_70_cc13.cpp` (12): `00A0C330`, `00A0C3C0`, `00A0C650`, `00A13B60`,
  `00A2DEF0`, `00A28300` (partial: more than 32 records), `00A11070`, `00A126C0`, `00A2CCF0`, `00A25F70`,
  `00A26210`, `00A03760`; the planner clock and the assignment score are named in `00A29FD0`'s record;
- listing-checked host records (40): `00A14DD0`, `00A28A60`*, `00A243D0`*, `00A29FD0`*, `00A1E250`*,
  `00A0F810`, `00A04560`*, `009FE270`*, `00A02020`, `00A12A90`*, `00A15570`*, `00A10EC0`, `00A11690`,
  `00A11FF0`*, `00A1CB80`*, `00A26510`, `00A265F0`, `00A22800`, `00A2E4C0`, `00A11B80`, `00A03510`*,
  `00A0F970`, `00A250A0`, `00A1A7A0`, `00A1A720`*, `00A1D010`, `00A22750`*, `00A010F0`*, `00A15970`*,
  `00A32350`*, `00A31730`*, `00A13340`*, `00A16EF0`*, `00A2DB80`*, `00A31DB0`*, `009FFC10`*, `00A152B0`,
  `00A12430`*, `00A11F70`, `00A1CF90` (* partial, the divergence in the record's evidence).

No Ghidra annotation was made in this pass (no rename was needed; the names stand).

### 7.14 Verification

- `./scripts/build.ps1` passed in this tree after the final `git merge origin/main`: the build and
  ctest (2 of 2). No test was added.
- `tools/verify_report_calls.py local/g69b_calls.json local/g69b_calls2.json`: 82 call-site rows cited
  in this section, 0 failures (row files not committed).
- `tools/x87_dc_form_scan.py --range 009FE130-00A371A0`: the DC-form sites in the bodies modelled here
  (`00A126C0` x2, `00A13B60` x2, `00A26210`) were read with `--context 3`; each is the
  `FLD K / FMUL ST1,ST0 / FXCH` scale idiom and the models scale the loaded component.

### 7.15 Open and handoff

114 of the 165 functions of the band without a record (EH funclets excluded, index rebuilt after
`git merge origin/main`) at the start of this pass remain without one: 31 debug writers and
describers, 33 constructors, destructors and container helpers, and these real bodies:
- host reconstructions not yet checked: `00A2C790`, `00A2D9D0`, `00A2DDE0`, `00A2E260`, `00A2EEE0`,
  `00A2DFA0`, `00A15A70`, `00A32500`, `00A07EB0`, `00A2C8D0`, `00A2BD00`, `00A2BD50`, `00A2BD90`,
  `00A2BDB0`, `00A2BE10`, `00A2DB50`, `00A1E210`, `00A1F310`, `009FFEB0` (the host's
  `rtb_exclusion_arm`), `00A07FB0`, `00A184F0`, `00A0FC50`, `00A0FC90`, `00A317C0`;
- no host and no model: `00A152E0` is a vtable slot the host does model inline (record pending),
  `00A12580`, `00A25B90` with `00A08420`/`00A08020` (the mode planners' quick spawn), `00A31C90`/
  `00A329F0` (multiplayer party messages, unreferenced broadcast);
- unreferenced (no caller or data reference): `00A13280`, `00A13080`, `00A22920`, `00A2BE70`,
  `00A2C2A0`, `00A185E0`, `009FE130`, `009FE170`, `009FE1B0`, `009FFE00`, `009FFF20`;
- small helpers and Lua setters: `009FF3A0`, `009FFD60`, `00A038C0`, `00A03A60`, `00A07F60`,
  `00A07F80`, `00A16990`, `00A16B20`, `00A31800`, `00A31C60`.

Next, by value: cc9's decisions on 7.12 items 1-5; then the membership operations (`00A2D9D0`,
`00A2DDE0`, `00A2E260`, `00A2C790`) against the host, which matter for every group think.
