# Unimplemented host calls in USN04, and the minimap unit gate

Packets `cc9_unimplemented_audit` and `cc9_hud_minimap_bind` (worker cc9-platform, 2026-09-23).
Addresses: 0043F080 005BE110 005C1675 006D1FA0 006F57D0 007001E0 0074DDF0 008DDF00 005BCC00
008BD900 008B19A0 008B15B0 008B8AD0 0088C750

## 1. Where the unimplemented calls are

**Source.** The UNIMPLEMENTED table the host prints at exit, from the USN04 4500-frame reference
run `fb_trt_usn04.log` (the difficulty worker's tree, main `42dffdde5`). It has 517 UNIMPLEMENTED
rows and **3,084,000 calls**. This packet's own control on main `14010ab95` is in section 5. A row
counts calls to one host record, not one native: a native the host splits into several records
appears once per record.

**Share by subsystem.** Rows are bucketed by their record prefix. The bucket is a reading, not a
recovered attribution; the ship-AI weapon-director row is counted under ship AI here.

| subsystem | calls | share |
| --- | ---: | ---: |
| HUD and presentation (minimap, markers, front end, in-game interface) | 931,037 | 30.2% |
| ship AI (orders, hold, paths, avoid zones, follow) | 792,025 | 25.7% |
| world, physics and unit instance (ship motion, unit sub-updates) | 742,760 | 24.1% |
| plane AI (plane devices, pilot bot, bot states) | 440,892 | 14.3% |
| mission frame, events and other frame hosts | about 125,000 | 4.1% |
| gunnery (recon contacts, auto target, weapon director) | 49,497 | 1.6% |
| Lua mission natives | 181 | 0.0% |

**The top 25 rows.**

| # | calls | record | address | subsystem | owner | what binding it takes |
| ---: | ---: | --- | --- | --- | --- | --- |
| 1 | 200,774 | Plane::device_busy_1fc | 007CEB00 | plane AI | cc9-dogfight-engaged | The plane device's busy query at +1FCh; needs the device objects the plane host does not build. |
| 2 | 183,200 | HudMinimap::unit_shows_on_minimap | 0043F080 (mis-addressed; the call is vtable +B8h) | HUD | this packet | Bound here: the class's +B8h body (section 3). |
| 3 | 94,500 | UnitInstance::update_unit_timers | 00956600 | world/unit | none | Reconstructed (`unit_update_timers_00956600`); needs the descriptor damage table at +354h and a pose view per slot. |
| 4 | 94,500 | UnitInstance::update_propellers | 00834E90 | world/unit (rendering) | none | Reconstructed; its tail sub-updates 00834820, 00834CC0 and 00834A70 are not, and no scene nodes are built. |
| 5 | 94,500 | UnitInstance::update_engine_audio | 008252C0 | sound | none | Reconstructed. SEntityInit sets the +9C4h gate (00822CBE), so the RPM smoother runs; the emitters at +BB4h..+BBCh are not built. |
| 6 | 94,500 | UnitInstance::sub_update_0081c050 | 0081C050 | world/unit | none | Not read. |
| 7 | 94,500 | UnitInstance::smooth_intensity | 008227E0 | rendering/effects | none | Not reconstructed. |
| 8 | 94,500 | UnitInstance::publish_effect_intensity | 00815AA0 | rendering/effects | none | Reconstructed; the effect groups at +FFCh are never filled. |
| 9 | 85,484 | HudMinimap::find_icon_entry | 005BE110 | HUD | this packet | Bound here: MSVC `std::map::find` (section 3). |
| 10 | 82,435 | FrontEndScreen::update | 004F75C0 | HUD/presentation | none | The front-end screen's per-frame virtual. |
| 11 | 82,422 | HudMarkers::gui_extent | 00AA1FE0 | HUD | none | Widget extent; needs the widget layout sizes. |
| 12 | 81,000 | ShipMotion::unit_post_motion | 00826B84 | world/physics | none named; cc9-hull-axis works in ship motion | Post-motion tail of the unit tick. |
| 13 | 81,000 | ShipMotion::rigid_body_substep_schedule | 00C5BB30 | world/physics | as row 12 | Physics substep scheduler. |
| 14 | 81,000 | ShipAiOrder::tail_009ef910 | 009EF910 | ship AI | cc9-difficulty / cc9-neighbour | Order-ring tail. |
| 15 | 81,000 | ShipAiOrder::slot_to_order_ring | 00825F7C | ship AI | cc9-difficulty / cc9-neighbour | Slot to order-ring copy. |
| 16 | 81,000 | ShipAi::unit_weapon_director | 00CFC3D0+114h | gunnery | cc9-gunnery-host | The ship's weapon-director virtual. |
| 17 | 81,000 | ShipAi::tail_009da8d0 | 009DA8D0 | ship AI | cc9-difficulty / cc9-neighbour | AI tick tail. |
| 18 | 81,000 | ShipAi::drive_heading_vtable50 | 00CFC3D0+50h | ship AI | cc9-difficulty / cc9-neighbour | Heading drive virtual. |
| 19 | 73,264 | HudMarkers::view_projection_matrix | 00B70490 | rendering | none | Needs the renderer's camera; this process has none. |
| 20 | 73,264 | HudMarkers::transform_vec4 | 00B62D10 | rendering | none | Follows row 19. |
| 21 | 63,415 | ShipAi::hold | 009DA0D0 | ship AI | cc9-difficulty / cc9-neighbour | Hold state. |
| 22 | 61,610 | AvoidZoneLayer::sample_0041bc20 | 0041BC20 | ship AI | cc9-neighbour | Avoid-zone sample. |
| 23 | 59,242 | PilotBot::plan_controls | 0099D300 | plane AI | cc9-dogfight-engaged | Pilot control planner. |
| 24 | 49,440 | ShipAi::navigation_arm_tail | 009EEAAB | ship AI | cc9-difficulty / cc9-neighbour | Navigation arm tail. |
| 25 | 44,879 | ShipAiPath::refresh_plan_head | 009ED3E0 | ship AI | cc9-difficulty / cc9-neighbour | Path plan refresh; tied with ShipAiArmTail::after_arm 009DE5B0. |

**Headline.** The owned areas (ship AI, plane AI and gunnery) hold 11 of the top 25 rows. The
other fourteen are HUD, rendering and the unit instance's step-11 presentation sub-updates. In this
process nothing a mission's outcome reads depends on them.

## 2. The mission-end natives

USN04 4500 does not end (`summary mission end: none`), so none of these appear in its table. They
run when the mission fails (`docs/TURNDOWN_EXIT.md` section 4, `docs/MISSION_END.md` section 3),
where each is logged UNIMPLEMENTED with a neutral return.

| native | address | caller in the script | what it does on the end path |
| --- | --- | --- | --- |
| Objectives_Failed | 008BD900 | `luaObj_FailedAll`, argc 5 | Marks objectives failed on the objectives screen; the script has already decided. |
| CountdownCancel | 008B19A0 | `luaInitMissionEnd` | Cancels the HUD countdown. |
| MissionNarrativeClear | 008B15B0 | `luaInitMissionEnd` | Clears queued narrative. |
| Scoring_SetMissionCompleted | 008B8AD0 | `luaMissionFailedNew`, argument false | Leaves the commit slot's scoring record clear. `EndScene` (008B01B0) reads that record to raise the restart prompt. |
| BannSupportmanager | 0088C750 | `luaMissionFailedNew` | Bans the support manager. |

Also on that path and unbound: SetInvincible (00897A50), EnableInput (008AFBD0) and
MissionNarrative (008B0C10). The narrative panel fires `luaMissionEnd_CamOnEnt`, which is what
eventually reaches `EndScene`. Of the five, **Scoring_SetMissionCompleted is the only one
`EndScene` reads**. The narrative's completion is the gate on reaching `EndScene` at all.

## 3. The binding: the minimap unit gate and the icon lookup

**The walk.** `005C154E` (`hud_minimap_place_unit_icons_005c154e`) skips a unit unless all three
tests pass:
1. the four bytes 0043F080 tests, inlined at 005C1628;
2. `vtable[5Ch](5)` at 005C165D;
3. `vtable[B8h]()` at 005C1675.

It then looks the unit up in the icon map at screen+F8h (005C170A calls 005BE110) and creates an
entry on a miss.

**The record was mis-addressed.** The host recorded test 3 under 0043F080, the __fastcall byte
gate. The listing at 005C166B..005C1675 is `MOV EAX,[EDX+0B8h] / CALL EAX`, a virtual.

**The law.** Every class whose kind test accepts 5 was read at its own vtable +5Ch, to confirm the
`unit_kind_query` table row, and at +B8h, from the image on disk. Four bodies result:

| body | code | classes |
| --- | --- | --- |
| 006D1FA0 | `MOV AL,1 / RET` | 05h..17h, 45h, 46h: all ships, submarines and planes, the airfield and the shipyard |
| 006F57D0 | `XOR AL,AL / RET` | 1Bh MLandFort, 1Ch MCommandBuilding |
| 007001E0 | `XOR AL,AL / RET` | 35h |
| 0074DDF0 | `008DDF00(this)` on `[[00E188A8]+[..+18ECh]*4+21A4h]` | 19h MLandVehicle |

`bsp::hud_minimap_unit_shows_body` (`src/hud_minimap.cpp`) maps the most-derived class id at +C4h
to its body. The host answers true or false from it. For a land vehicle it records 008DDF00 and
keeps the old true. That is a **labelled substitution**: 008DDF00 is not reconstructed.

**The lookup.** 005BE110 is MSVC's `std::map::find`. It walks down from the root at `[map+4]+4`,
reads the key at node+0Ch and the nil byte at node+15h, and returns `end()` unless the key is not
below the found node's. 005BCC00 then compares the result with `end()`. The key is the unit
pointer (EDI, stored at 005C1702). The host's icon map, keyed by unit index, stands in for the
container at screen+F8h. The index is injective over the same units, so the same find is the law
and the row becomes `done`.

**Switch.** `kHudMinimapUnitGateBound` in `src/game_hosts_hud_world.cpp`. OFF keeps both records
and the constant true.

**No Ghidra function.** `ghidra proto` answers `?` at 006D1FA0, and the three short bodies sit in
INT3 padding after the previous function:

| start | exclusive end | body |
| --- | --- | --- |
| 006D1FA0 | 006D1FA3 | `MOV AL,1 / RET` |
| 006F57D0 | 006F57D3 | `XOR AL,AL / RET` |
| 007001E0 | 007001E3 | `XOR AL,AL / RET` |

**State.** Reconstructed, build-tested. Not ABI-compatible and not game-validated.

## 4. Predictions, written before the pair

The pair is one tree, `agent/cc9-platform` on main `14010ab95`, built twice differing only in
`kHudMinimapUnitGateBound`, with `BSP_GUNNERY_RNG_STREAMS=1` on both sides.

* **Unimplemented total.** OFF equals the control's table total. ON is lower by exactly the two
  rows' counts, `unit_shows_on_minimap` plus `find_icon_entry` (183,200 + 85,484 = 268,684 in the
  reference table), provided every unit reaching test 3 is a ship or a plane.
* **Rows that change.** Those two rows leave the UNIMPLEMENTED table and reappear as `done` rows
  at 006D1FA0 and 005BE110. A `(fort)` or `land_vehicle_player_query` row appears only if USN04
  carries a class 19h, 1Bh, 1Ch or 35h unit; none is expected.
* **Rows expected flat.** Everything else: damage, deaths, queued hits, releases, the minimap's
  placed-icon counts, and every other UNIMPLEMENTED row. Both answers are the ones the host gave
  before (true, and the same map find), so the walk and everything downstream are identical.

## 5. The pair

Logs `local\ua_off_usn04.log` (OFF, run from a copy of the OFF build under `build\off`) and
`local\ua_on_usn04.log` (ON). Both use `--frames 4700 --press-start-frame 30 --menu-select USN04
--mission-frames 4500 --mission-frame-seconds 0.05` with `BSP_GUNNERY_RNG_STREAMS=1`. The control
`local\ua_base_usn04.log`, built on main `14010ab95` before any edit, matches OFF on every table
row and every summary line. The only exception is one startup callback count (18 against 19).

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 3,061,823 | **2,793,139** (-268,684) |
| HudMinimap::unit_shows_on_minimap | 183,200 UNIMPLEMENTED at 0043F080 | 183,200 done at 006D1FA0 |
| HudMinimap::find_icon_entry | 85,484 UNIMPLEMENTED | 85,484 done |
| `(fort)` and `land_vehicle_player_query` rows | absent | absent |
| every other table row | | identical |
| all 120 `summary` lines (damage, deaths, releases, mission end) | | identical |

**Result.** Every prediction holds. All 20 units reaching test 3 are class 05h..17h, so the
answer stays true, and the walk and everything downstream are unchanged. **The switch lands ON.**
The control's total (3,061,823) is below the reference table's 3,084,000 because main moved
between `42dffdde5` and `14010ab95`, mostly in ship AI and plane AI rows.
