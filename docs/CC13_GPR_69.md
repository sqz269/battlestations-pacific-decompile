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
`008EADA0(ECX = [00F88C30], entry, party, target)` at `00A180C9`.

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

(in progress)
