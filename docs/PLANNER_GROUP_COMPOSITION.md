# Group composition and the CLOSEATTACK fallback (packet cc9_group_composition)

Addresses: 00A2E720, 00A2C8D0, 00A2DB80, 00A2D8E0, 00A2DDE0 (composition, cited); 00A12A90,
00A15490, 00A13B60, 00A14A78-00A14D4C (the promotion and its no-candidate arm, read here);
00A02020 (cited). Every name is a hypothesis, not a recovered symbol. Ghidra was not written.

## 1. The composition is the image's

The brief located the merge pass at 00A0C650's caller. That is not where it lives.
00A0C650 is `BSP_AiGroup_ComposeAttackValue`, which scores one group against another for the
planner (00A0F970, docs/PLANNER_KATE_TARGETING.md). Groups are composed by the coordinator's
composition pass 00A2E720, whose read is `docs/AI_GROUP_THINK.md` sections 1-2:
- **Evict (00A2DDE0):** members whose gate bytes fail, or whose party or team no longer matches,
  leave.
- **Split (00A2E260):** the 009FE080 subset moves to a new group.
- **Seed:** one new group per ungrouped live team-0/1 entity.
- **Auto-merge (00A2C8D0, then 00A2DB80):** a ship group and an air group never merge; otherwise
  the absorbed group's command decides.
- **Proximity merge:** of two groups' leaders.
- **Members sorted by the 009FFD80 class weight (00A2D8E0):** the head is the leader.

The host binds all of these (`src/game_hosts_ai.cpp`: `evict_invalid_members`,
`split_detached_members`, `can_auto_merge`, `merge_group`, the sorted `attach`). The fighter
flights merging, and later six more aircraft joining the group under Lexington-class01_sqn01, are
therefore those rules. No composition divergence was found. The one host defect that bears on it
is the dead member that 00A2DDE0 does not evict, because the host's plane death never raises
+5Dh/+60h (`docs/PLANNER_KATE_TARGETING.md`, routed to the plane-death hunk).

## 2. The promotion, and who gets Yorktown's order

**00A12A90 (MOVETOATTACK).** Its two states are read in `docs/AI_COMMAND_TICK.md`:
- Above CollectDist (+1F4h, 3000 in this run) it orders the LEADER at the target's leader point
  and runs the follower pass 00A10DC0. Ship followers get 0077C8D0 join-formation; squadrons get a
  moveto at the leader point.
- At or below CollectDist it installs CLOSEATTACK (vtable 00D22C44) through 00A2BD00.
- The promotion itself orders no member. It is the class that changes.

**00A15490 (CLOSEATTACK)** calls 00A13B60(1.5, target leader point, target group, 1) every tick.
00A13B60 serves each ship member of the group (`docs/AI_CLOSE_ATTACK_TICK.md`):
- It scores candidates collected within 1.5 x CollectDist of the centre, with a range factor
  that runs from 1.0 at NearDist 3000 to 0.0 at FarDist 6000 over the member-to-candidate
  distance.
- The best candidate gets `attackmove` (ship) or `settarget`.
- A member with no candidate falls to 00A14A78-00A14D4C, read here in full:

```
r   = FILD [[command+4]+5644h] (own population) * double [00D22C90] = 160.0   00A14A78-00A14A8B
if (double [00CE3D90] = 400.0 > r) r = float [00CFD710] = 400.0              00A14A93-00A14AA9
d2  = (member+FCh - centre.x)^2 + (member+104h - centre.z)^2                 00A14B97-00A14BD3
if (d2 > r*r)                                                                00A14BE3
    point = 0.4 * member+FCh..104h + 0.6 * centre                            00A14CC1-00A14D41
            (double [00CE65D0] = 0.4, double [00CEFF98] = 0.6)
    00A02020(member, point)                                                  00A14D48
```

**So yes: the image does order a far member, Yorktown included, when its group is in
CLOSEATTACK and the member has no candidate.** But it is sent 60% of the way to the centre,
re-issued every tick, and a member within max(160 x population, 400) of the centre is not ordered
at all. The host sent every candidate-less member AT the centre, whatever the distance.

In D0 (`local\D0_9000.log`, range factor OFF) the served members are the two carriers:
- 333 ticks serve two members, giving 333 attackmoves and 333 fallbacks.
- Of the fallback movetos logged as issued, 34 go to the Lexington and 19 to Yorktown.

The binding is `kCloseAttackFallbackOffsetBound` (`include/bsp/ai_close_attack_tick.hpp`). The
OFF path is the old centre moveto.

## 3. Predictions, written before the pair

The pair is G0 (switch off) against G1 (on), both with the range factor OFF, E2 9000, stream
option on. G0 should reproduce D0: 1 torpedo, 0 bombs, goal replans 212, promotion at 121.80 s.

| row | G0 | G1 prediction |
| --- | --- | --- |
| promotion time | 121.80 s | unchanged: the promotion precedes the fallback |
| composition (group memberships, fighter first order Val #3.1) | as D0 | unchanged |
| Lexington fallback movetos | 34 | fewer (0-20) while the centre is within 2880 m of it (18 members x 160) |
| Yorktown fallback movetos | 19 | a similar count, each to a point 60% of the way, so its track bends by a smaller first leg |
| ship goal replans | 212 | 150-260 |
| torpedo drops / Kate deaths | 1 / 16 | 0-3 / 16 ± 2 |
| Lexington | alive | alive |

A caveat that holds on both sides: from 289.85 s D0's centre is the dead Kate #4.1, frozen 23.9 km
away. Both carriers then chase a wreck's point until the plane-death flags land.

## 4. The pair, measured (G0 against G1)

G0 (`local\G0_9000.log`) reproduces D0: 1 torpedo, 0 bombs, 212 goal replans, and the promotion
at 121.80 s with dist 2974.5 m.

| row | G0 (off) | G1 (on) | prediction | verdict |
| --- | --- | --- | --- | --- |
| promotion | 121.80 s, 2974.5 m | 121.80 s, 2974.5 m | unchanged | held |
| served / attackmove / fallback ticks | 666 / 333 / 333 | 666 / 333 / 333 | - | - |
| Lexington fallback movetos issued | 34 | 34 | 0-20 | **missed** |
| Yorktown fallback movetos issued | 19 | 34 | similar | more issue events: the 60% point moves every tick |
| Yorktown goal distance (d32c) at steps 3000 / 5000 / 7000 / 8900 | 24133 / 21897 / 20231 / 18648 m | 14467 / 13019 / 11353 / 9770 m | a shorter leg | held |
| ship goal replans | 212 | 230 | 150-260 | held |
| torpedo drops / Kate deaths | 1 / 16 | 1 / 16 | 0-3 / 16 ± 2 | held |
| damage | 7700.0 | 7700.0 | - | - |
| Lexington | alive | alive | alive | held |

**Why the Lexington row did not drop.** It never falls inside r (18 x 160 = 2880 m) of the centre.
Every one of the 333 fallback ticks has d2 > r2, because the centre is the target group's leader
point, and from early on that point is a far or frozen aircraft (section 3's caveat, and
`docs/PLANNER_KATE_TARGETING.md`).

**Switch state landed: `kCloseAttackFallbackOffsetBound` ON.** The Yorktown's order is the
image's in kind, since 00A13B60's no-candidate arm does order a far member. Its point is now the
image's 0.4/0.6 blend rather than the centre itself. The range-factor retry still waits on the
plane-death flags.
