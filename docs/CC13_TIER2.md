# CC13 Tier 2: the minor_deviation LIVE rows

Packet: cc9's adoption plan item 3, Tier 2 of docs/CC13_ADOPTION_TRIAGE.md. For each row, I
re-read the bytes (disk bytes through `bsp.py disasm-raw`, or the export listing) and the current
host, including the host's ON switches and callers. Liveness comes from the current-main logs
p20e_off_jm08 (JM08), p20d_off_usn04 (USN04) and p23f_off_u21f (USN21), all in cc9-physics1's
local\.

Out of scope: rows other cc13 workers hold, which are 007C0D90, the 007C11E0 land chain,
009B7C90 / 009A76E0 / 009B5C80 / 009C3100, 009FC7C0 and the 009C1FD0 follow seams. The struck
ranges are also excluded.

## Switches

| Switch (include/bsp/cc13_tier2_switches.hpp) | Address | Host site | Reach | Effect ON |
| --- | --- | --- | --- | --- |
| `kLineOfFireLandFromMuzzleBound` (OFF) | 0072CDD0 | game_hosts_gunnery.cpp `line_of_fire_blocked_0072cdd0` | land queries for the line of fire: JM08 4721, USN04 1080, USN21 11892; 0 land hits on all three | The land segment starts at the unraised muzzle |
| `kDiveBombGoawayFloatStoresBound` (OFF) | 009C7F00 | dive_bomb_task.cpp `dive_bomb_goaway_complete_009c7f00`, called from game_hosts_units.cpp (dive-bomb goaway state) | dive-bomb task: JM08 39 aircraft, 23 releases; USN21 219 aircraft, 13 releases; USN04 29 aircraft, 0 releases | The term and ceiling - 100 are rounded to float where the image stores them |

### 0072CDD0: the land query starts at the muzzle

- **Bytes.**
  - 0072CDF1 loads EBX with the caller's muzzle pointer.
  - 0072CDF5..0072CE0F copy it to [ESP+18h].
  - 0072CE42..0072CE52 add the qword 5.0 at 00D7A370 to the copy's y only.
  - The end point is the target's +FCh..+104h. Its y is raised by the same 5.0 and floored at
    5.0f (00CE3850, 0072CE62..0072CE76).
  - 0072CE8E PUSH EBX passes the original, unraised muzzle as the start of 0098ADD0 (kind 44h,
    0072CE91).
  - The unit walk 0098B130 (0072CECE) takes the raised copy.
- **Host.** `line_of_fire_blocked_0072cdd0` (game_hosts_gunnery.cpp) starts both queries at
  `from`, the muzzle raised by 5.0. The land query runs under kLandscapeSpatialAttachBound, which
  is ON.
- **Live.** The line-of-fire land queries are counted in the `landscape attach` census line:
  - JM08: 4721 queries;
  - USN04: 1080 queries;
  - USN21: 11892 queries.

  None of them met land. ON lowers the start by 5 m, so it can only add land hits. A gun whose
  muzzle is at or below the terrain surface may be refused where OFF clears it:
  - a land platform whose gun takes the identity frame, which puts it at the unit origin
    (kLandPlatformAttachmentBound);
  - a ship hard against an island.

  0072F6E0 caches a refusal per (gun, target) for the gun's life, so an ON refusal persists.
- **ON-only lookups.** None. The ON arm passes the caller's `muzzle` array, which is alive for the
  whole call, to the same `landscape_segment_hit`. That function reads the same landscape entries
  as the OFF arm.
- **Shared stream.** No draw. The 0072F76D expiry draw happens on every cache miss under either
  value of the switch.

## Recorded only

- **00864680, the line-of-sight eye point** (game_hosts_gunnery.cpp `line_of_sight_00864680`).
  - Stale half: the 0.5 s eye cache is modelled. It is `kGunneryLosEyeCacheBound`, ON
    (008646B5 COMISS/JC, 008647A4 reload 0.5f from 00CE3800).
  - The raise is cfg+94h (5.0) when the owner fails IsKindOf(5). IsKindOf(5) is the unit class,
    so every LOS participant passes it.
  - With [unit+538h] set and no model at class+50h, the raise is class+A8h Height + cfg+94h
    (00864793, 00864799). The host takes this arm.
  - The model arm (00862C00 answers, 0086474B..00864791) uses top + max(cfg+94h,
    cfg+9Ch * (top - bottom)). It stays labelled: this host builds no model at class+50h, so the
    branch condition cannot be decided (docs/GUNNERY_OPEN_ITEMS.md, G2).
  - No switch: a switch would need host data that does not exist.
- **009FC000 / 009FC1C0, LegAltitudeManager register and remove** (leg_altitude_manager_cc13.cpp).
  - Both are faithful.
  - The walk swaps the last record in on removal and re-tests the same index. The end is re-read
    each pass (009FC0AB, 009FC231).
  - The remove predicate is unit, null or +5Dh (009FC1F5..009FC201).
  - Register returns on a live match (009FC077) and appends only for a non-null unit (009FC050
    SETNZ, 009FC0B8).
  - The only omission is the 009FB010 critical section, which is single-threaded here.
- **007ECF80, the squadron broadcast** (plane_squadron_entity.cpp).
  - The reconstruction has no caller in src. The live fan-out is
    `GameUnitsHost::set_skill_level_007b8ae0` under kSkillLevelSquadronFanOutBound (ON), which
    skips kPlaneSquadronNoUnit members.
  - The image walks i < +3CCh over +3D0h with no null test.
  - The added guards only matter on an error path.
- **007C9540, set_unit_world_basis.**
  - The image copies the whole matrix to +74h and +674h through 004134F0, so the translation
    row is applied.
  - Both of its callers pass the unit's own position as the translation:
    - In the squadron arm, 008A0D9B..008A0DB7 load each member's +FCh..+104h into
      [ESP+64h]..[ESP+6Ch] before the vtable[88h] call.
    - In the single-entity arm, the host's caller sets m[12..14] to the unit's position
      (game_hosts_script_orders.cpp, EntityTurnToEntity).
  - Not applying the translation is therefore unobservable.
- **0045F520 / 0045F440, call_entry_point_if_defined.**
  - The image tests IsFunction (00B66200). The host's `global_is_defined` tests !nil.
  - They differ only for a global with an entry-point name that is bound to a non-function.
    JM08, USN04 and USN21 log no "attempt to call" line.
- **009E9EB0, the sub-attack fire step.** Stale. 009E4A60's range term is fixed under
  `kSubAttackOwnHalfLengthBound` (ON, ship_ai_sub_attack.hpp). The rest is precision only.
- **007BC7A0, the player release attitude gate.**
  - Bytes: 007BC7CA..007BC7D4 are XORPS / COMISS 0,[desc+E0h] / JAE. A rocket class (IsKindOf
    33h of 007B9510's weapon) skips the attitude gate when +E0h <= 0. An unordered compare sets
    CF, so NaN does not skip it.
  - The host comment at game_hosts_units.cpp `player_bomb_release_006082d0` reads "+E0h > 0".
    The sign is inverted. That is cc9's file, so it is not edited here.
  - Not live in a normal mission: the host's only caller is the scripted helm order in
    game_hosts_mission_frame.cpp. Its 007B9140 stand-in, `ordnance_has_general_bomb_2ah`,
    excludes 33h before the gate, so the rocket pass is never reached.
- **007B8AE0, set skill level.**
  - The image's pilot-bot store ([sub+A68h] vtable[18h], 007B8AF1..007B8AFD) is unconditional.
    Only 009565A0's unit+390h store skips a unit with +5Dh set.
  - The host keeps one index for both. They differ only on a unit with +5Dh set. Such a unit is
    out of action: the player unit lists require +5Dh clear, the ship throttle gate zeroes its
    throttle, and the leg manager drops it. Its unit+390h is not expected to be read again. That
    last point is a hypothesis; no reader census was run.
- **00827B90, the failure effects.** Stale. `failure_name_is` (game_hosts_gunnery.cpp) already
  compares case-insensitively (cc13 backfill G3, 771b6a3a0; 7a0386564 only added callers).
- **009C7F00, goaway complete.** This is an URGENT category, reported to the lead.
  - The logic matches.
  - The image stores the term (009C7F16, 009C7F7D) and ceiling - 100 (009C7FB8) as floats. The
    host keeps them in double.
  - Under the main thread's 24-bit precision control, that is a difference of at most 1 ULP at
    a compare.
  - The lead asked for a switch anyway, because cc9 is chasing goaway completion and the
    project models 24-bit precision. It is `kDiveBombGoawayFloatStoresBound` (OFF; see
    Switches). There is no ON-only lookup and no draw.
  - The host's double products are exact: each multiplies a float by the qword 00D7A390,
    whose value is float(0.9). So the switch's cast is the image's single rounding.
  - Recorded, no switch: the ceiling pick at 009C7F3C is a JBE with ST0 = ctl+398h and
    ST1 = the sum, so the image takes +398h when the compare is unordered. The host's
    `cruise_altitude_398 <= sum` takes the sum. They differ only on a NaN input.
- **009D0160 / 009D1360 / 009D1500, torpedo run times.** This is an URGENT category, reported to
  the lead.
  - The added zero-divide guards never fire on live data:
    - run_speed is 51.44 on every 009D1360 log line on JM08, USN04 and USN21;
    - the decel_distance guard's arm needs dd > leg >= 0;
    - 009D1500's divisor is a release distance, never 0 with the shipped tuning.
  - The x87 quotient note is moot under the 24-bit precision control.
  - The lead agreed: verified-equivalent on live data, no switch.
