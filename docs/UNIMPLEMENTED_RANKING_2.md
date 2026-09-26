# Unimplemented host records, ranking 2

cc9-platform2, 2026-09-25, packet `cc9_unimplemented_ranking_2`. The input is two runs' host
tables (name, address, status, calls), UNIMPLEMENTED rows only:
- **E2**: `local\ab_on_e2.log`, USN04 9200/9000. This is the latest E2 ON log: main ef25f15c8
  plus the AABB contract hunk, applied locally.
- **USN04**: `local\rank_usn04.log`, USN04 4700/4500 on main b12a7ede7.

Both used `BSP_GUNNERY_RNG_STREAMS=1` and a 1600x900 window. The script is `local\rank.py`.

| | E2 9000 | USN04 4500 |
| --- | ---: | ---: |
| unimplemented calls | 4,538,628 | 2,399,229 |
| rows (name + address) | 556 (both runs combined) | |
| distinct native sites | 463 | |

**Classes.**
- **mirror**: the record stands for an image call whose value is already bound elsewhere, or whose
  effect is render-side or unread. Binding it moves only record counts.
- **gap**: unread behaviour that may write state.
- **contract**: third-party library code (physics, render).
- **excluded**: owned by the gunnery worker (`src/game_hosts_gunnery.cpp`) or the planner worker
  (the pilot/planner hunks of `src/game_hosts_units.cpp`).

The owning file is where the record's literal sits.

## Top 30 rows by calls

| # | row | address | E2 | USN04 | owning file | class |
| ---: | --- | --- | ---: | ---: | --- | --- |
| 1 | `Plane::device_busy_1fc` | 007ceb00 | 391703 | 192571 | game_hosts_units.cpp | gap: 007CEB00's device walk unread (planner owner, units.cpp plane tick) |
| 2 | `UnitInstance::smooth_intensity` | 008227e0 | 189000 | 94500 | game_hosts_units.cpp | mirror: a visual intensity smoother, no gameplay reader |
| 3 | `ShipMotion::unit_post_motion` | 00826b84 | 163434 | 81000 | game_hosts_units.cpp | gap: the tail of 00825F20 after the controller step (00826B84..), unread |
| 4 | `ShipMotion::rigid_body_substep_schedule` | 00c5bb30 | 163434 | 81000 | game_hosts_units.cpp | contract: Dyn_World_Substep, physics library |
| 5 | `ShipAiOrder::slot_to_order_ring` | 00825f7c | 162000 | 81000 | game_hosts_ship_ai.cpp | mirror: publishes slot+40h..+48h, which steering does not read |
| 6 | `ShipAiOrder::tail_009ef910` | 009ef910 | 162000 | 81000 | game_hosts_ship_ai.cpp | gap: BSP_ShipAi_RefreshTurnClearance (2 KB), unread |
| 7 | `ShipAi::unit_weapon_director` | 00cfc3d0+vtable114 | 162000 | 81000 | game_hosts_ship_ai.cpp | mirror: vtable dispatch; the value is bound elsewhere |
| 8 | `ShipAi::tail_009da8d0` | 009da8d0 | 162000 | 81000 | game_hosts_ship_ai.cpp | gap (small): 009DA8D0, a periodic timer (+B3C/+B40) raising the torpedo warning and 00977820 |
| 9 | `ShipAi::drive_heading_vtable50` | 00cfc3d0+vtable50 | 162000 | 81000 | game_hosts_ship_ai.cpp | mirror: vtable dispatch; the heading is bound |
| 10 | `ShipAi::navigation_arm_tail` | 009eeaab | 137641 | 54629 | game_hosts_ship_ai.cpp | gap: 009EEAAB, the navigation arm's tail |
| 11 | `ShipAiPath::refresh_plan_head` | 009ed3e0 | 136509 | 54482 | game_hosts_ship_ai.cpp | partial: the head of 009ED3E0; the widths are literals |
| 12 | `ShipAiArmTail::after_arm` | 009de5b0 | 136509 | 54482 | game_hosts_ship_ai.cpp | gap: 009DE5B0 |
| 13 | `ShipAi::hold` | 009da0d0 | 129193 | 64715 | game_hosts_ship_ai.cpp | gap (small): 009DA0D0 copies the AI's next block into its current block |
| 14 | `AvoidZoneLayer::sample_0041bc20` | 0041bc20 | 102515 | 102515 | game_hosts_units.cpp | gap: the avoid-zone layer sample (units.cpp; the sea stands in) |
| 15 | `PilotBot::plan_controls` | 0099d300 | 89441 | 60977 | game_hosts_units.cpp | planner-owned (excluded) |
| 16 | `MissionCamera::collision_ray` | 0098b370 | 44995 | 22495 | game_hosts_hud.cpp | mirror: the camera's collision probe, five per frame |
| 17 | `BotTaskGun::tick` | 009fc7c0 | 43680 | 18153 | game_hosts_units.cpp | planner/dogfight-owned (excluded) |
| 18 | `HudShipScreen::gauge_digit_uv` | 0043aba0 | 36316 | 18316 | game_hosts_hud.cpp | mirror: GUI vertex UV rewrite, render-side |
| 19 | `BotStateFollow::steer_point` | 009bfee0 | 33046 | 17420 | game_hosts_units.cpp | planner-owned (excluded) |
| 20 | `BotStateFollow::command_step` | 009bee30 | 33046 | 17420 | game_hosts_units.cpp | planner-owned (excluded) |
| 21 | `BotStateFollow::steer_to_point` | 009f9e40 | 33046 | 17420 | game_hosts_units.cpp | planner-owned (excluded) |
| 22 | `ShipAi::replan_finish` | 009ddbc0 | 32807 | 16285 | game_hosts_ship_ai.cpp | gap: 009DDBC0 |
| 23 | `Gunnery::recon_slot_contacts_008053c0` | 008053c0 | 28486 | 16322 | game_hosts_gunnery.cpp | gunnery-owned (excluded) |
| 24 | `BotTaskDogfight::follow` | 009c1fd0 | 26963 | 11499 | game_hosts_units.cpp | planner/dogfight-owned (excluded) |
| 25 | `UnitInstance::wake_setting` | 00424c40 | 19581 | 5220 | game_hosts_units.cpp | mirror: the wake GameSettings read, render-side |
| 26 | `UnitMotion::unreconstructed_override_remainder_00758270` | 00758270 | 18478 | 9000 | game_hosts_units.cpp | gap: the 00758270 override remainder (units.cpp motion) |
| 27 | `GuiText::create_glyph_buffers` | 00ab8400 | 18328 | 9328 | game_hosts_text.cpp | mirror: text render resources |
| 28 | `GuiText::ensure_draw_sections` | 00ab8530 | 18328 | 9328 | game_hosts_text.cpp | mirror: text render resources |
| 29 | `UnitPickScreen::segment_query` | 009043a0 | 18160 | 9160 | game_hosts_hud.cpp | gap (HUD): 29h's spatial segment query 009043A0 |
| 30 | `HudMinimap::camera_heading_virtual` | 004b4b00 | 18160 | 9160 | game_hosts_hud_world.cpp | gap (HUD): the minimap camera heading getter |

## Families (top 30 by calls)

| family | rows | sites | E2 | USN04 | owning file(s) |
| --- | ---: | ---: | ---: | ---: | --- |
| `ShipAi` | 6 | 6 | 785,641 | 378,629 | game_hosts_ship_ai.cpp |
| `Plane` | 4 | 4 | 391,766 | 192,624 | game_hosts_units.cpp |
| `ShipMotion` | 2 | 2 | 326,868 | 162,000 | game_hosts_units.cpp |
| `ShipAiOrder` | 2 | 2 | 324,000 | 162,000 | game_hosts_ship_ai.cpp |
| `HudMarkers` | 18 | 18 | 290,530 | 146,530 | game_hosts_hud.cpp,game_hosts_hud_world.cpp |
| `UnitInstance` | 2 | 2 | 208,581 | 99,720 | game_hosts_script_orders.cpp,game_hosts_units.cpp |
| `HudShipScreen` | 10 | 10 | 176,345 | 91,245 | game_hosts_hud.cpp |
| `ShipAiArmTail` | 3 | 3 | 152,893 | 68,582 | game_hosts_ship_ai.cpp |
| `ShipAiPath` | 2 | 2 | 136,529 | 54,503 | game_hosts_ship_ai.cpp |
| `UnitPickScreen` | 9 | 9 | 127,174 | 64,174 | game_hosts_hud.cpp |
| `HudRootScreen` | 8 | 8 | 127,120 | 64,120 | game_hosts_hud.cpp |
| `MissionFrame` | 15 | 15 | 126,001 | 63,001 | game_hosts_mission_frame.cpp |
| `PilotBot` | 2 | 2 | 107,102 | 78,430 | game_hosts_units.cpp |
| `BotStateFollow` | 4 | 4 | 103,735 | 57,001 | game_hosts_units.cpp |
| `AvoidZoneLayer` | 1 | 1 | 102,515 | 102,515 | game_hosts_units.cpp |
| `HudMinimap` | 15 | 14 | 90,863 | 45,863 | game_hosts_hud.cpp,game_hosts_hud_world.cpp,game_hosts_lua.cpp |
| `InGameInterfaceUpdate` | 8 | 8 | 81,000 | 40,500 | game_hosts_hud.cpp |
| `HudWeaponGroupScreen` | 7 | 7 | 72,639 | 36,639 | game_hosts_hud.cpp |
| `HudShipView` | 4 | 4 | 72,632 | 36,632 | game_hosts_hud.cpp |
| `ShipAiRingScan` | 3 | 3 | 50,400 | 34,560 | game_hosts_ship_ai.cpp |
| `AutoTarget` | 5 | 5 | 47,355 | 23,728 | game_hosts_ship_ai.cpp |
| `ShipAiFollow` | 4 | 4 | 42,720 | 45,280 | game_hosts_ship_ai.cpp |
| `MissionCamera` | 2 | 2 | 44,998 | 22,498 | game_hosts_hud.cpp |
| `BotTaskGun` | 1 | 1 | 43,680 | 18,153 | game_hosts_units.cpp |
| `Gunnery` | 5 | 5 | 42,123 | 24,556 | game_hosts_gunnery.cpp |
| `BotTaskDogfight` | 3 | 3 | 38,337 | 18,503 | game_hosts_units.cpp |
| `GuiText` | 2 | 2 | 36,656 | 18,656 | game_hosts_text.cpp |
| `HudWarningScreen` | 2 | 2 | 36,316 | 18,316 | game_hosts_hud.cpp |
| `ShipAiApproach` | 15 | 15 | 30,229 | 21,098 | game_hosts_ship_ai.cpp |
| `MissionEvents` | 3 | 3 | 27,000 | 13,500 | game_hosts_commands.cpp,game_hosts_mission_frame.cpp |

## Reading

- **The ship AI dominates the gaps:** the `ShipAi*` families, 53 rows and 1,540,313 calls in E2. Most of it is
  per-step tails the host never ran (009DA8D0, 009DA0D0, 009EF910, 009DE5B0, 009DDBC0, 009EEAAB):
  162,000 calls in E2, which is 18 per mission frame. Two rows are vtable dispatches
  whose values are already bound (the weapon director and the heading getter), so they are mirrors.
- **Ship motion:** the 00825F20 tail after the controller step (00826B84) is a gap. The physics
  substep schedule (00C5BB30) is the dynamics library's.
- **The HUD families** (`HudMarkers`, `HudShipScreen`, `UnitPickScreen`, `HudRootScreen`,
  `HudMinimap`, about 800 K calls together) are mostly per-call mirrors of GUI or render calls.
  Their real gaps are 29h's spatial query and the minimap camera getters, where missing producers
  make the pick and the minimap answer none.
- **Planner-owned** rows (`PilotBot`, `BotStateFollow`, `BotTaskDogfight`, `BotTaskGun`,
  `Plane::device_busy_1fc`) and the gunnery-owned recon slot are the other workers'.

## The top real gap outside the excluded files

The ship AI tails in `src/game_hosts_ship_ai.cpp`: 162,000 calls each in E2. Among them:
- `ShipAi::tail_009da8d0` (113 bytes) is the smallest self-contained one. It is a periodic countdown
  (+B40h, period +B3Ch) that on expiry raises `BSP_WarningManager_ReportTorpedo` (+3F2h set) and
  calls 00977820 (+3F1h set).
- `ShipAi::hold` 009DA0D0 (255 bytes) copies the next block (+3A8h.., +1F4h..) into the current one
  (+38Ch.., +1C4h..).
- `ShipAiOrder::tail_009ef910` is the 2 KB turn-clearance refresh.

The binding packet takes the two small ones first: 009DA0D0's commit and 009DA8D0's timer. Their
readers decide whether they move anything.
