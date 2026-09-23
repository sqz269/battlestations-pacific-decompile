# Ship AI traffic records: what the approach steers away from

Packet cc9_ship_traffic, 2026-09-23. Base: main b70bb65db. Switch `kShipAiTrafficBound` in
`src/game_hosts_ship_ai.cpp`. Status: reconstructed, build-tested and run-compared on USN04 and
USN01. Not ABI-compatible and not game-validated. Descriptive names are hypotheses.

Addresses: 009E9190 BSP_ShipAi_ApproachRefreshAvoidance, 009E8360
BSP_ShipAi_TrafficRecord_Construct, 009E6170 BSP_ShipAi_TrafficRecord_IsActive, 009E6240
BSP_ShipAi_ApproachRefreshTargetFirepower, 0095EB40 BSP_Unit_RateExpectedWeaponDamage,
008053C0 BSP_Recon_EnsureSlot, 00720180 BSP_CommandControllerBase_Construct.

## 1. What a traffic record is, and who makes it

There is no separate producer. 009E9190 makes the records itself, on its own timer.

1. **Cadence.** Every call subtracts the frame seconds from nested+11F4h (009E91DC). When the
   timer goes negative (009E91E4, FCOMIP of 0.0 then JBE), it is re-armed from
   00BD2F10(1, 2.0, 3.0) (009E9209; floats 00CE3958 and 00CE3854) and the candidate walk runs.
2. **Candidates.** The walk takes 008053C0(ECX = [unit+54h]), the side's recon slot
   (009E921D..009E9220), and follows the intrusive list at slot+0DE8h (next at node+4h, value
   at [node+8h]+4h). That is the side's enemy contact list: every enemy unit with a recon level
   other than none.
3. **Filters, in order.**
   - The entity is nonzero (009E9243).
   - It answers vtable[5Ch](5) (009E9253).
   - It is not the brain's target [brain+0B20h] (009E9263).
   - Its planar distance squared, dx*dx + 0*0 + dz*dz as x87 and stored to float at 009E92CB,
     is below ([entity+494h] + 200.0)^2. The 200.0 is a double at 00CE4D70, added at 009E92D5,
     and the compare is FCOMIP then JBE at 009E92E5. Entity+494h is the entity's longest weapon
     range (00956C20, docs/GUNNERY_TABLES.md), not a speed.
4. **Duplicates.** The insert walks the list at nested+14A0h first and skips an entity that some
   record already holds at +14h (009E92EF..009E933B).
5. **The record.** operator new(124h) at 009E9342, then 009E8360(entity) at 009E935D, spliced at
   the end of the list (009E9366..009E9397, before the sentinel). The constructor stores:
   - vtable 00CEDDA0
   - +4h..+0Ch = 0 and +10h = 1
   - +14h = the entity, registered as an observer pair through 00694A60
   - the 60-sample range curve at +18h, cleared by 00954940
   - +108h = -1.0f (00D7A260), so the first advance refreshes at once
   - +120h = 1.0f (00D7A24C)

A traffic record is therefore a detected enemy, other than the current target, that stands
within its own weapon reach plus 200 m of this ship.

## 2. The pass over the records

Before the walk, 009E942B stores nested+1279h = 1 and 009E9432 stores nested+1254h = 30.0f
(00CE38C8). Those are byte +41h (use ready rounds) and word 7 (ready horizon) of the
nested+1238h block, the block that describes this ship as a target. The earlier header called
the 30.0 a "seed", which was a misreading. It is now kApproachAvoidReadyHorizon.

The accumulator starts from the zero-on-disk global at 00F87574..7C. The list is walked from the
head forward (009E943A..009E9441). For each record:

1. **Live test.** 009E6170(unit x, y, z, 300.0f), with 300.0 at 00CE3AE8 loaded at 009E9492.
   False when the entity is not live (+5Ch set, +5Dh/+60h/+5Eh clear) or has moved beyond
   ([entity+494h] + 300)^2. A false record is erased (009E9579..009E95B0) and the walk continues
   with the next node. The 200 to 300 gap is hysteresis.
2. **Advance.** 009E6240(seconds, unit position, &nested+1238h) at 009E950F. It applies the same
   live test, then +108h -= seconds. When +108h drops below 0:
   - +10Ch..+114h = unit - entity, with y overwritten by 0
   - +11Ch = its length (0042B2F0), and the direction is divided by it
   - +118h = its compass heading (007B4E90)
   - block word 5 = the heading (009E634C), +40h = 1 (009E634F), word 0 = the distance (009E6359)
   - 0095EB40 is called with ECX = [record+14h], so the rating is the CONTACT's weapons against
     this ship
   - +120h = the result, or 1.0f (00D7A24C) when the result is below 1.0 (009E6367..009E639B)
   - +108h = 00BD2F10(1, 2.0, 3.0) (009E63A6)
3. **Sum.** accumulator += float(weight x direction), each product stored to float first
   (009E9514..009E9570).

After the walk, when |acc|^2 > 1.0 (009E95F4, FCOMIP then JBE), the avoid heading is the compass
heading of the accumulator. The strength is interp(0 -> 0, tune+0Ch -> tune+8h) of |acc|
(0042B2F0), where tune+8h = 3.0 (0081F22D) and tune+0Ch = 1000.0 (0081F254). Every slot gets
avoid_3c = interp(0 -> strength, pi -> 0) of the SIGNED wrapped difference between the slot
angle and the avoid heading (009E968D, 009E96B4). The term reaches the decision only through
009E7BE0's five-word slot total, which 009E76D0 compares. The standoff scan 009E71A5 never reads
it.

The image clears nested+1278h and +1279h on every frame fill (009F29CA/009F29D1 and
009F2F69/009F2F70), so the bytes the pass sets do not leak into the target curve.

## 3. Host against image

| Term | Image | Host | Status |
|---|---|---|---|
| contact list | [008053C0(unit+54h)+0DE8h] | the gunnery host's recon stand-in: enemy side, rule (b)'s four bytes, not sunk, a ship or plane base, recon level other than none | substitution, recorded earlier (include/bsp/game_hosts_gunnery.hpp) |
| kind test 009E9253 | vtable[5Ch](5) | `unit_is_kind_of(i, 5)` | faithful |
| target exclusion | [brain+0B20h] pointer | the same one-based handle | faithful |
| entity+494h | longest weapon range | gunnery row `any_weapon_max_range` | faithful |
| positions | +0FCh / +104h after 00414DB0 | `unit_position_00fc` | faithful |
| duplicate walk, append at end | 009E92EF..009E9397 | the same | faithful |
| live test | +5Ch/+5Dh/+60h/+5Eh | `unit_alive_and_visible`, and the unit not sunk | faithful |
| walk order | head forward | forward; the reconstruction walked backward before this packet | corrected |
| accumulation | each product stored to float, then added | the same | corrected |
| rating | 0095EB40 with ECX = contact over nested+1238h | `ship_ai_firepower_rating_0095eb40` with the contact's `FirepowerBinding` and this ship's block | faithful to the bound rating |
| both timers | 00BD2F10(1, 2.0, 3.0) | 2.0, the low bound | LABELLED substitution; the approach draws no random numbers |
| strength and tune | tune+8h 3.0, tune+0Ch 1000.0 | the same | faithful |

## 4. Predictions

These were written to local/tr_predictions.txt before any run.

1. Records would come only from enemy aircraft, since USN04 has no enemy ships. This held: the
   first record on each ship is a Val or a Kate.
2. Most weights would sit at the 1.0 floor, and the avoid term would be far below the 3.0 cap.
   This held. Every weight is exactly 1.0, and the largest avoid term on any ship is 0.0323.
3. Standoffs would be unchanged. This held on all eight ships.
4. Winner changes would move by at most 10 in aggregate. This held: 117 became 122.
5. Gunnery would be unchanged or moved only through positions. This held: nothing moved.
6. USN01 would be identical. This held.

## 5. Measurement

Runs used `BSP_GUNNERY_RNG_STREAMS=1` on both sides, at 4700/4500 frames for USN04 and
3200/3000 for USN01. The control is the same source with `kShipAiTrafficBound` false. Logs:

- local/tr_ctl_usn04.log and local/tr_trt_usn04.log
- local/tr_ctl_usn01.log and local/tr_trt_usn01.log
- the diagnostic pair local/tr_dgc_usn04.log and local/tr_dgt_usn04.log, compared by
  local/tr_cmp.py

The diagnostic pair added a temporary per-scan line, which was not committed. It gives the
winner, the best total with and without the avoid term, the gap to the second-best slot in the
control, and the record count.

| Ship | Inserts | Erases | Max records | Scans with records | Avoid max | Scans the avoid term decides | Winner changes ctl / trt | Heading changes ctl / trt |
|---|---|---|---|---|---|---|---|---|
| Northampton-class01 | 18 | 5 | 14 | 402 | 0.0280 | 0 | 18 / 18 | 18 / 18 |
| Northampton-class02 | 18 | 5 | 14 | 402 | 0.0247 | 0 | 9 / 9 | 116 / 116 |
| Fletcher-class01 | 18 | 5 | 14 | 384 | 0.0276 | 20 | 11 / 14 | 28 / 31 |
| Fletcher-class02 | 18 | 5 | 14 | 402 | 0.0232 | 1 | 14 / 14 | 71 / 71 |
| Fletcher-class03 | 18 | 5 | 14 | 393 | 0.0323 | 9 | 16 / 18 | 63 / 65 |
| Fletcher-class04 | 18 | 5 | 14 | 402 | 0.0252 | 1 | 19 / 19 | 69 / 69 |
| York-class01 | 18 | 5 | 14 | 402 | 0.0254 | 0 | 11 / 11 | 33 / 33 |
| York-class02 | 18 | 5 | 14 | 402 | 0.0237 | 0 | 19 / 19 | 43 / 43 |

Judged term by term, per ship:

- **Only the avoid term moved.** On every ship, the set of scans where the winner differs from
  the control equals the set where the treatment's best slot changes when the avoid term is
  removed. The winner equals that best slot on every scan of both runs.
- **The decided scans are near-ties.** On each ship, the largest control gap between the top
  two slots over its flipped scans is below the smallest avoid term on the treatment's winner
  over the same scans:

  | Ship | Largest control gap | Smallest avoid term |
  |---|---|---|
  | Fletcher-class01 | 0.0067 | 0.0127 |
  | Fletcher-class02 | 0.0005 | 0.0177 |
  | Fletcher-class03 | 0.0130 | 0.0254 |
  | Fletcher-class04 | 0.00003 | 0.0186 |
- **Why the term is small.** Every refresh rated the contact's weapons below 1.0 against the
  ship, so each weight is the 1.0 floor. With up to 14 unit vectors, |acc| stays below about 11,
  and the strength is 3.0 x |acc| / 1000.
- **Unchanged.**
  - Standoffs first and last on all eight ships.
  - 103 queued hits, 15656.2 damage and 9 deaths.
  - Lexington at 141.9 HP.
  - Every aircraft and ship row in the gunnery table.
- **Position traces moved slightly after the flips,** from step 2900 on Fletcher-class02 and
  later on the other three Fletchers, with no gunnery consequence.
- **USN01 matches line for line,** apart from one platform message count. No ship there enters
  attackmove.

Decision: landed with `kShipAiTrafficBound` on. Every changed decision row is explained by the
avoid term deciding a near-tie. Nothing else moved.

## 6. Step 4, read-only

- **Scans 127 to 141 of docs/SHIP_AI_RING_QUERY.md.** The window starts at fixed step 3126, the
  first scan after the log records `entity dead: unit=21 "D3A Val #1.1" died=156.20 s`. It ends
  about step 3190, when the ships retarget. The held target had health 0. The output cap is
  damage_cap x 4, and damage_cap is that health, so the rating was 0. It was not armour.
- **The allow bytes [0080E160(unit)+220h..+223h].** 0080E160 returns unit+738h, the command
  controller. Its constructor 00720180 stores 1 into +220h, +221h, +222h and +223h
  (007202FD..00720312). The only other writer is the base message apply 0071C1E0, for sub-kinds
  3 to 6. Its senders are 0071DFD0 from the Lua binding 0089C590 ArtilleryEnable, 0071E050 from
  0089C740, 0071E0D0 from 0089C8F0 TorpedoEnable and 00A11AF0, and 0071E150 from 0089CAA0. The
  host's forced 1 is the constructor default; it is exact unless a mission script calls one of
  those bindings.

## 7. Open

- Why every aircraft rates below 1.0 against every ship, which makes all weights the floor.
  Plane categories 0, 1 and 10 go through 0095EB40's buckets, and it is not checked whether the
  image agrees.
- All eight ships insert the same 18 contacts and erase 5. The ships are close together, so this
  is plausible, but the plane positions were not checked against the 1100 m keep radius.
- Whether the image releases a dead target from [brain+0B20h] sooner than the host's retarget
  does.
- The two random timers use the low bound, as the approach's other stream-1 draws do.
