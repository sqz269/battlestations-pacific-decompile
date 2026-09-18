# The CLOSEATTACK tick, and how a group member gets an enemy

Addresses: 00A13B60, 00A0F810, 00A2C720, 00A14DD0, 00A08460, 00419010, 00A371A0, 0077D600,
00A02020, 0071EB60, 00521EA0, 00414DB0, 009FFEB0, 007EDA90, 00E08F78, 00E08EF8, 00E08F08,
00E08F68, 00E188A8, 00CE380C, 00CE4ADC, 00D7A24C.

Packet `cc8_ai_close_attack_tick`, read-only analysis. Every descriptive name here is a
hypothesis, not a recovered symbol.

Coverage. Complete as rules: the collect radius, the per-member gate, the candidate admission
test, the four-factor score with its four tuning fields, the winner test, and the order class and
descriptor the winner receives. **Partial, by address range**: `00A14A78`-`00A14D4C`, the
no-candidate fallback, whose offset point is assembled from six stack slots not traced to their
producers; `00A13BFF`-`00A14380`, the collection walk, read for its bounds and filter but not
instruction by instruction; `00A0F810`, whose callee set is enumerated but whose body is unread;
and `00A14DD0`-`00A152A7`, read only to its first dispatch.

## The correction this packet forces

`docs/AI_COMMAND_TICK.md` concluded that "the command descriptor is always `00E08F68`", that "no
AI command class issues any other scene command", and that "the descriptor carries a point, never
an object". **All three are wrong**, and the reason they looked right is that the packet that wrote
them read `00A02020` and stopped at `00A13B60`.

| Class descriptor | Ordinal | Name | Issued by | Descriptor |
| --- | --- | --- | --- | --- |
| `00E08F68` | 15 | `moveto` | `00A02020` at `00A0216A` | a position |
| `00E08F78` | 17 | `attackmove` | `00A13B60` at `00A14A6E`, ship member | an **object** |
| `00E08EF8` | 1 | `settarget` | `00A13B60` at `00A14A6E`, other member | an **object** |
| `00E08F08` | 3 | `clearorders` | `00A14DD0` at `00A14EA7` | no target |

The object descriptor is built at `00A14A33`-`00A14A65`: `kind` is set to 1 at `00A14A4C`,
`object` takes the chosen entity at `00A14A46`, `object_id` its `+174h` at `00A14A51`/`00A14A5D`,
the position triple is the zero vector `00F87574`, and `trailing` is zeroed at `00A14A65`.

## `00A13B60`, the pass

`__thiscall(AiCommand* command)(float radius, const float* centre, AiGroup* targetGroup,
int flag)`, body `00A13B60`-`00A14D9F`. `CLOSEATTACK`'s `vt+0Ch` calls it at `00A154E3` with the
target group's leader point and the `1.5f` at `00CE380C`; `DEFENDPOSITION`'s calls it at
`00A1555B` with its own group's leader point and `FLD1`.

### The collect radius

`00A13B8A` reads tuning `+1F4h` `CloseAttack_CollectDist` and `00A13BA1` multiplies it by the
caller's radius; `00A13BBD` squares the product. So `CLOSEATTACK` collects within 4500 and
`DEFENDPOSITION` within 3000 on the shipped table. `00A13BE6` walks the world collection
`[00E188A8] + 19CCh + 64h`, the same one the group seed phase scans, into a local list.

### Which members are served

`00A143A0` walks the own group's `+563Ch` list. Per member:

| Arm | Test | Evidence |
| --- | --- | --- |
| plane squadron | `vtable[+5Ch](18h)`, then served unless the carrier link passes `007EDA90`'s shape inline | `00A143ED`, `00A14402` `PUSH 17h`, `00A1440C` `+C24h`, `00A14413` `JE` |
| anything else | served only when `vtable[+5Ch](6)` and the `+538h` object answers false to its `vtable[+2Ch]` | `00A14427`, `00A1443D` |

**An individual plane is neither**, so a group of loose aircraft is served by nothing here. That is
what the measurement below shows on two of the three missions.

### The score

For each candidate in the collected list:

```
weight = 00A0F810(member, candidate)                    ; 00A146C4
inGroup = targetGroup ? 00A2C720(targetGroup)(candidate) : false   ; 00A146B5
if (weight <= 0 and !inGroup) skip                      ; 00A146CD-00A146DB
range = 00419010(NearDist, 1.0f, FarDist, 0.0f, dist)   ; 00A1493D
score = range * weight
      * (inGroup       ? tuning[+204h] : 1.0f)          ; 00A1494E / 00A1495D
      * (existing hit  ? tuning[+200h] : 1.0f)          ; 00A1497D / 00A1496D
if (score > best) { best = score; chosen = candidate }  ; 00A149A8, JBE at 00A149AC
```

`00419010 BSP_Math_InterpolateClamped(x0, y0, x1, y1, x)`, `RET 14h`, returns `y0` when the two
x-endpoints are equal (`00419026`). With `x0 = CloseAttack_NearDist` and `y1 = 0.0f`, the factor is
1.0 at or inside 3000 and falls linearly to 0.0 at 6000. That is exactly what the shipped script's
own comments on those two keys say, which is the producer-side confirmation of the model.

The four tuning fields, on the shipped IslandCaptureRookie record:

| Field | Key | Value |
| --- | --- | --- |
| `+1F4h` | `CloseAttack_CollectDist` | 3000 |
| `+1F8h` | `CloseAttack_NearDist` | 3000 |
| `+1FCh` | `CloseAttack_FarDist` | 6000 |
| `+200h` | `CloseAttack_ExistingTargetMul` | 1.8 |
| `+204h` | `CloseAttack_TargetGroupMemberMul` | 10 |

A candidate with a non-positive weight survives only by belonging to the target group, and a
target-group member is worth ten times an equal outsider, so the command's own target group
dominates the choice without being the only thing that can be attacked.

### `00A14DD0` does not share the body

`00A14DD0`-`00A152A7`, the third base's pass that `CAUTIOUSMOVE` and `CAUTIOUSATTACK` reach, calls
neither `00A0F810` nor `00A2C720`. It issues `clearorders` at `00A14EA7` and also calls
`00A02020`. Read only to its first dispatch.

## Host methods

| Host method | Native | Note |
| --- | --- | --- |
| `command_tick`, CloseAttack and DefendPosition arms | `00A13B60` | runs `ai_close_attack_tick_00a13b60` |
| `close_issue_order` | `0077D600` | the registry path with `attackmove` or `settarget` and the target's name |
| `close_target_weight` | `00A0F810` | substitution: the candidate's own class weight from `009FDF30` |
| `close_in_target_group` | `00A2C720` | the member list search |
| `close_member_current_target` | `0071EB60` then `00521EA0` | for the sticky multiplier |
| `close_member_controller_busy` | `+538h` `vtable[+2Ch]` | recorded; contract unread |
| `close_issue_moveto` | `00A02020` | the fallback, at the centre rather than the untraced offset |

Three labelled substitutions. The target weight stands in for `00A08460`'s model, which this
process cannot run; the class weight preserves the per-class ordering the score needs but is not
the native number. The order carries the target's name through the registry rather than the
pointer and `+174h` id the native descriptor holds. The fallback `moveto` goes to the collect
centre instead of the offset point `00A14A78`-`00A14D4C` builds.

## Corrections

Appended to `docs/AI_COMMAND_TICK.md`, never rewriting it: the three scene-command claims above,
and the `009FFD70`-to-`009FFD80` ordering-key correction that `docs/AI_COMMAND_INPUTS.md` already
records.

## no_ghidra_function

None. Every routine read here has a Ghidra function.

## Validation

Three campaign missions, 3000 mission frames at 0.05 s. Before is this tree at `f603d17b5`, whose
own after-runs are reused unchanged. After is this change.

| Mission | Run | Members served | `attackmove` | `settarget` | Fallback `moveto` | Candidates scored | Scene commands | Gunnery hits | Damage |
| --- | --- | --- | --- | --- | --- | --- | --- | --- | --- |
| IJN01 | before | n/a | n/a | n/a | n/a | n/a | 34 | 4 | 500.0 |
| IJN01 | after | 0 | 0 | 0 | 0 | 0 | 34 | 4 | 500.0 |
| USN01 | before | n/a | n/a | n/a | n/a | n/a | 17 | 23 | 220.0 |
| USN01 | after | 0 | 0 | 0 | 0 | 0 | 17 | 23 | 220.0 |
| USN02 | before | n/a | n/a | n/a | n/a | n/a | 14 | 161 | 19061.0 |
| USN02 | after | 616 | 559 | 0 | 57 | 4004 | 635 | 168 | 20721.4 |

Attribution.

- **USN02 is where the tick runs.** Its attack group is fourteen ships led by `DeRuyter`, it
  promotes to `CLOSEATTACK` early, and from then on every command tick serves its members: 616
  member-visits, 4004 candidate scorings, 559 `attackmove` orders and 57 fallback `moveto`s when no
  candidate was inside the 4500 collect radius. Scene commands rise from 14 to 635, which is the
  559 plus the 57 plus the 14 and the 5 the move-to-attack arm already issued.
- **The gunnery census moves with it**: 161 hits to 168 and 19061.0 damage to 20721.4, with deaths
  and kill credits unchanged at 3. The ships are now aimed at chosen enemies rather than drifting.
- **IJN01 and USN01 serve nobody, for a native reason.** Both attack groups are made of individual
  aircraft, led by `Dauntless1` and `ScoutDauntless`. The member gate admits a plane squadron or a
  ship base, and an individual plane is neither, so `served` is 0 and every census is unchanged.
  This is the same gate that already stopped their `MOVETOATTACK` closing arm, now confirmed one
  level deeper.
- **`settarget` is never issued on these three missions**, because it is the arm for a served
  member that is not a ship, and the only served members here are ships.
- **The pilot-attack table is unchanged on all three.** USN02's new orders go to ships, so the yaw
  arm of `docs/PILOT_BOT_PLAN_CONTROLS.md` sees nothing new, and USN02 still reports no unit
  ordered at a plannable target.

## Follow-up packets

| id | addresses | what it is |
| --- | --- | --- |
| `ai_close_attack_fallback_point` | `00A14A78`-`00A14D4C` | the offset point the no-candidate arm builds, the last unread piece of `00A13B60` |
| `ai_candidate_target_weight` | `00A0F810`, `00A08460` | the real target weight, so the choice stops running on a class-weight stand-in |
| `ai_cautious_approach_pass` | `00A14DD0`-`00A152A7` | the third base's pass and why it issues `clearorders` |
| `ai_group_plane_squadrons` | the seed phase, `009FE0B0` | why IJN01's and USN01's attack groups are loose aircraft rather than squadrons, which is what keeps both missions' ticks idle |

## Correction from docs/PLANE_SQUADRON_ENTITY.md

Appended by packet `cc8_plane_squadron_entity`. The text above is left as written.

The plane-squadron row of the member-gate table (the one reading "served unless the carrier link
passes `007EDA90`'s shape inline", with the sites `00A143ED`, `00A14402` `PUSH 17h`, `00A1440C`
`+C24h`, `00A14413` `JE`) describes the inline copy of `007EDA90` correctly but names its object
wrongly. `[squadron+3D0h]` is the squadron's **flight leader**, the first entry of its five-slot
member plane array, not a carrier; `17h` is `MPlaneKamikaze`; and the byte at `+C24h` is the
authored `PilotFires`. The evidence is in `docs/PLANE_SQUADRON_ENTITY.md` sections 1 and 2 and is
repeated in the correction appended to `docs/AI_COMMAND_LIFETIME.md`.

The follow-up row `ai_group_plane_squadrons` ("why IJN01's and USN01's attack groups are loose
aircraft rather than squadrons") was answered by packet `cc8_plane_squadron_entity`: this process
created no `PlaneSquadronGen` at all, because the scene loader makes one unit per scene entity and
an aircraft scene entity **is** a squadron. Both missions now build squadrons (33 on IJN01, 20 on
USN01) and both hold them as group members. That did **not** make `served` non-zero; packet
`cc8_ai_squadron_served` and `docs/AI_SQUADRON_SERVED.md` carry what does.
