# Screen 45h's update, and the in-mission screens 004F75C0 dispatches to

Packet `cc9_ship_screen_update` (worker cc9-platform, 2026-09-23). **Read only; binding is stopped
under the brief's rule**, because two blocks write gameplay state (section 3).
Addresses: 004F75C0 0064DD30 0064BB90 00CF5E30 004C43C0 004C5090 00535EE0 0077C2A0 00464710
0064A770 00927F30 009539E0 00690FD0 004F8670 004B6E50 004DC940 00852300 00815850 0043B370

## 1. The screens 004F75C0 dispatches to in USN04

**Source.** `local\hp_on_usn04.log` (USN04 4500, main de0780502), using the pump counts
`HudMinimap::update` (9,160) and `HudMarkers::update` (9,158).

**The pump.** It runs twice per mission frame. The interface is 20h (screens 29h 49h 44h 35h) for
the first pumps, then 25h for the rest of the run.

**The 25h level-1 set** is `29h 49h 44h 27h 4Dh 45h 46h 26h 2Eh 35h 50h`. 35h and 4Dh have bound
updates. **Each of the other nine is one `FrontEndScreen::update` record per pump:** 9 x 9,158 =
82,422, plus 13 from the early 20h pumps, which gives the logged 82,435.

| screen | registry slot | enter virtual | calls per run |
| --- | --- | --- | ---: |
| 26h | 38 | 0051ED60 | about 9,158 |
| 27h | 39 | 0067B3F0 | about 9,158 |
| 29h | 41 | 00521670 | about 9,160 |
| 2Eh | 46 | 005468B0 | about 9,158 |
| 44h | 68 | 00645F10 | about 9,160 |
| **45h** | 69 | 0064BB90 | about 9,158 |
| 46h | 70 | 0064A360 | about 9,158 |
| 49h | 73 | 0067BE80 | about 9,160 |
| 50h | 80 | 00682740 | about 9,158 |

The per-screen split is derived from the set lists and the two bound screens' counts; the log
counts the family as one row.

## 2. Screen 45h's update, `0064DD30`

**Identity.** `__thiscall(screen, float dt)`, SEH frame, body 0064DD30..006500E2, 2,258
instructions. It is vtable `00CF5E30` +20h, next to the enter virtual 0064BB90 at +10h.
- The screen object is built in `BSP_InGameInterface_Init` (0068CC70) through 0064B780, and sits at
  interface+78h.
- The 25h arm calls 0064D590 on it.
- Its unit is +184h.
- Its strings are the ship repair HUD's: `ingame.repair_armament`, `_water`, `_periscope`,
  `_engine`, `_fire` and `_body` (00CF619C..00CF6130), `EngineJam` (00CF6124), the hint ids
  `REPAIR1STUSE` and `REPPERMANENT` (00CF61C4, 00CF61B4), and
  `VehicleClass[..].NoRepairGUI` (00CE5880, 00CF5C88).

**It is the ship status and repair-crew screen.** Its blocks in listing order:

| block | what it does | reads | calls | writes |
| --- | --- | --- | --- | --- |
| 0064DD5F..0064DE90 | when +119h is set: a countdown at +30h (dt * 3.0), then positions a widget by the unit's pose | +30h, +FCh, unit+184h | 00AA8240 (x2) | HUD widgets |
| 0064DE92..0064E2F3 | the pipe-sight block (`docs/MISSION_CAMERA.md` section 9): time since the last shot, the zoom and blur springs, the effect at unit+4A4h, the fov | settings+44h..+7Ch, 00E197E8..00E197F4, 00F876A4 | 004BEC00, 00440490, 008687C0, 00440A30, 00B0CF80, 004DC940 | the fov and a point effect (presentation) |
| 0064E30C..0064E41E | layout and clamping of the screen's gauges | +5h, +15Ch, [00E198C4]+4Ch | 0064A960, 0064ABD0, 00415620, 0064A9F0 | HUD |
| **0064E42D..0064E5C9** | **turn-to-camera order.** With the controlled unit a local-player ship, not a submarine (vtable +5Ch(8)), on input action 98h pressed (004C43C0), or held (004C5090) with the timer +160h not negative: it reads the camera mover's yaw ([[00E198C4]+4Ch]+40h +384h) and the unit's heading (vtable +50h, 00438B10), builds a message through 00464710, and routes it with `0077C2A0(msg, 2, 0)`. It also writes unit+630h and 009539E0 on the unit | input, camera mover, unit | 004C43C0, 004C5090, 00927F30, 00438B10, 00464710, **0077C2A0**, 009539E0 | **gameplay: a unit order and unit+630h** |
| 0064E5CF..0064E9A2 | tutorial hints: award-tracker hint state, `VehicleClass[..].NoRepairGUI` through the Lua globals, and the screen closes itself when the hint shows | Lua, award tracker, [00F8898C] | 004C5090, 005FC620, 004E1CA0, 0068E600, 0068E5B0, 00690FD0, 00B67xxx, 004B6E50 | hint state, screen close |
| 0064E9A2..0064EA7A | switches the level-3 screen set on a condition | [00E198C4] | 00AA85B0, 004F8670, 004D8B70 | UI mode |
| 0064EAxx..0064F30C | the repair-crew rows: per-system state (armament, water, periscope, engine, fire, body) and their labels | unit repair state (00939F80/00939F70 read +34h/+38h), 0092D730 | 00ABBE50, 00ABE6E0, 00AA8B00, 00939F70/80, 0093A3F0 | HUD widgets |
| **0064F311..0064F35C** | **repair order.** On input action EFh (00535EE0) with a pending pick (+156h): builds a message from +14Ch through 0064A770 and routes it with `0077C2A0(msg, 7, 0)` to the controlled unit | input, +14Ch | 00535EE0, 0064A770, **0077C2A0** | **gameplay: a unit order** |
| 0064F35C..0064FFDE | the remaining gauges and repair timers | unit fields | 00ABBE50, 00ABE6E0, 0043B370, 00AA8B00 | HUD widgets |
| 0065005C..006500C1 | two gauges driven by unit+184h's 00852300 (the device list at +48h) and 00815850 (+104Ch plus 00810E90) | unit | 00852300, 00815850, 0043B370 | HUD widgets |

This is a block-level read. The 46 `CALL EAX` and 19 `CALL EDX` virtual calls still need their
receivers resolved block by block.

## 3. Why binding stopped

Two blocks write gameplay state:
- 0064E42D..0064E5C9 routes a unit order and writes unit+630h.
- 0064F311..0064F35C routes a repair order.

Both are gated on player input: actions 98h and EFh, through 004C43C0, 004C5090 and 00535EE0.
In this harness no in-mission action is ever driven, since only press-start 4Eh is, so in a run
neither order would be sent. Under the brief's rule the binding waits for the lead's decision.

**The binding this read supports.** Bind every presentation block. Keep the two order blocks as
the image's control flow with their input queries answered through the menu host's action
records, as the HUD already does for 004C43C0. The branch is then never taken here. The two
routes are kept as labelled records, and nothing sends a message.

## 4. State

No code changed. The `FrontEndScreen::update` record is unchanged. No run: the desktop session is
disconnected.

## 5. What runs with no player input (read 2026-09-23, second pass)

This pass reads Ghidra's decompile of 0064DD30, checked against the listing where x87 matters,
together with the screen's enter (0064BB90) and its layout loader (0064C0F0).

**The screen object's widgets**, from 0064C0F0's name lookups:

| offset | widget |
| --- | --- |
| +34h, +38h, +3Ch, +40h | pages GUI_ship, GUI_repair, GUI_ship_effects, GUI_ship_damage |
| +48h | ship_stick_Icon |
| +50h | ship_dir_Icon |
| +54h, +58h | ship_recon_Icon, ship_torpedo_Icon |
| +60h..+74h | the speed, recon and torpedo digit pairs |
| +90h..+A0h | repair_hl west, east, north, south, middle |
| +A4h, +A8h | repair_ikons_periscope_Icon, repair_ikons_engine_Icon |
| +ACh..+B8h | repair_warning west, east, north, south |
| +C0h | repair_Text |
| +C4h..+D4h | Icon_1..5 |
| +D8h..+E4h | Hl_1..4 |
| +E8h..+F4h | circle_1..4 |
| +F8h | ship_relation_Icon |
| +18Ch..+198h | the four Villanas flash icons, hidden at load |

**No gameplay write runs with no input.** The enter virtual stores **+160h = -1.0** (0064BE44).
- The turn-to-camera block writes unit+630h and routes the order only after action 98h is pressed,
  or held with +160h at or above zero. The action is never driven, so +160h stays -1 and its else
  arm, which also writes unit+630h, never runs.
- The repair-order route needs action EFh (00535EE0) with a pick pending at +156h.
- Repair mode (+157h) is entered only through the held-input path (004C5090).

**What does run every frame:**
- The relation-icon slide, while +119h is set.
- The pipe-sight block. Its zoom and blur adds are gated on the fire input (0064DED7: 004BEC00's
  record +1CB0h, held with time above zero). The blur spring (+15Ch, 00E197F0) and the zoom decay
  run, and so do 00B0CF80 on the renderer (a record here) and 004DC940.
- 0064A960, the relation icon. It is shown for a ship in a formation, with its state set by
  whether the unit leads (007788D0).
- 0064ABD0, the Villanas flashes: each alpha decays by `1 - dt*[00D7A328]`, and an icon is hidden
  below [00D7A270].
- The throttle stick. +44h eases toward `clamp((throttle + 0.5) / [00CE3D78], 0, 1)` (00415620),
  in thirds (00D7A2B0), or snaps within [00D7A23C]. Then 0064A9F0 turns ship_stick_Icon through
  its virtual +44h from a throttle map (00CE69D0, 00E08CAC, 00CF5C68, 00CF5C60).
- The repair-status flags +BCh..+BFh:
  - the fire or flood byte, unit+9E5h (0x48B == 2 for a submarine);
  - any device with +5Dh set on the unit's device list +48h;
  - 00939F80 and 00939F70 above [00D7A390].
- Then 00545AC0 and 00644240, and the four repair warnings are hidden.
- The analog repair selector reads six axes through 004C5070. These are input and answer zero
  here.
- The gauge tail: 00852300 and 00815850 on the unit through 0043B370.

## 6. Plan

The update is about 9 KB and calls about 20 helpers that have not been read: 00545AC0, 00644240,
00545360, 0054E610, 0064A2F0, 0064A770, 0064AAD0, 007788D0, 0043B370, 004C5070, 004C5090,
00535EE0, 005FC620, 00939F70/80, 0093A3F0, 00852300, 00815850 and the widget virtuals +34h, +44h,
+4Ch and +88h. It is bound in two stages behind `kHudShipScreenUpdateBound`.

- **Stage A:** the top-level flow and every block listed above as running each frame. The input
  queries are answered through the menu host's action records. The two 0077C2A0 routes, 00B0CF80
  and the award tracker are labelled records.
- **Stage B:** the repair-mode radial selector, the hint and Lua NoRepairGUI path, and the
  level-3 set switch. All are control flow the input gates never open here.

Each stage gets predictions and a USN04 pair.

## 7. Part 1: the relation icon, the flashes and the throttle stick

**The code.** `bsp::ship_screen_update_0064dd30` in `src/hud_ship_screen.cpp` runs the top-level
order of 0064DD30:
1. The +119h slide gate. The enter clears +119h (0064BE70), and only 0064A902 sets it.
2. The pipe-sight block, as a record.
3. `0064A960`, the relation icon: shown for a non-submarine in a formation. Its state select
   (+88h) is 1 unless the unit leads.
4. The flash enable +1ACh, then `0064ABD0`: each intensity decays by `1 - dt*4.0`, below 0.05 its
   icon is hidden, otherwise alpha is `min(v, 1)` and the icon is shown.
5. The +5h and +184h gates.
6. +44h eased toward `clamp((throttle + 0.5)/1.5, 0, 1)`: one third of the gap, or a snap within
   0.001. The enter seeds it with -1.0 (0064BD7C).
7. `0064A9F0(0.1)` turns ship_stick_Icon through +44h. The throttle maps to an angle: -1.178 below
   -0.5, 2.356 above 1, otherwise 2.356*t for positive t and 2.356*t for negative t (as 1.178*t
   doubled). The angle is eased by 0.1 from the widget's current +48h rotation.
8. The rest of the update, from 0064E415, as a record.

**The host.** The pump calls it for slot 45h when `kHudShipScreenUpdateBound` is on. The unit comes
from the 25h arm's hand-off on interface+78h (0064D590 stores +184h). The widgets are found by the
names 0064C0F0 looks up. Visibility, rotation and alpha go through the frontend.

**Records and substitutions:**
- **The relation icon's state select** (00AB1710) is a record: the bridge draws an icon's first
  authored state only.
- **The flash enable** `[[00E198C4]+4Ch]+30h` sits on an interface object this process does not
  build. It reads clear and is recorded, so the four flashes are zeroed and hidden each frame.
- **The pipe-sight block and the remainder** are records. The fov that the pipe-sight block sets
  is already applied by the mission camera tick.

## 8. Part 1 predictions, written before the pair

**Setup.** One tree on main 536f11bee plus this branch, built with `kHudShipScreenUpdateBound` off
and then on. All camera switches are on. USN04 4500, `BSP_GUNNERY_RNG_STREAMS=1`, back buffer
2560x1440.

- **Row that leaves:** `FrontEndScreen::update` falls by the screen-45h pumps, about 9,158.
- **Rows added:**
  - `HudShipScreen::update`: done, about 9,158.
  - `HudShipScreen::pipe_sight_block` and `HudShipScreen::flash_view_mode`: about 9,158
    UNIMPLEMENTED each.
  - `HudShipScreen::update_remainder`: about 9,158 UNIMPLEMENTED, once the screen is applied and
    has its unit.
  - `HudShipScreen::relation_select_state`: about 9,158 UNIMPLEMENTED if the player's ship sits in a
    formation, otherwise absent.
- **Total.** The unimplemented total **rises**, by about +18,316, or about +27,474 with the
  formation row. One family record becomes three or four block records: the update itself is now
  done, and what it still lacks is named.
- **Summary lines.** All identical. No summary line reads these widgets.

## 9. The part 1 pair

`local\ss_off_usn04.log` against `local\ss_on_usn04.log`, back buffer 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,176,379 | 2,203,853 (+27,474; predicted +27,474 with the formation row) |
| FrontEndScreen::update | 82,435 | 73,277 (-9,158) |
| HudShipScreen::update 0064DD30 | | 9,158 done |
| pipe_sight_block, flash_view_mode, update_remainder, relation_select_state | | 9,158 UNIMPLEMENTED each |

- **Summary lines.** 156 of 157 are identical, and every gameplay line is among them. The one that
  moved is the sprite-bridge line: `quads` goes from 189 to 185.
- **Why the bridge moved.** These are the four flash icons. The image hides them in its layout
  load (0064D543..0064D576) and 0064ABD0 keeps them hidden each frame. The host's page had drawn
  them as authored. This is the one prediction miss: I had said no summary line reads these
  widgets, and the bridge line counts them.
- **Result.** Every row prediction holds. **`kHudShipScreenUpdateBound` flips ON.**

## 10. Part 2: the controls, 0064E415..0064F665

**The flow with no input.** The whole repair menu (0064E62C..0064F30C) sits behind **004C5090(EFh)
held** at 0064E61F. That covers the hints, NoRepairGUI, the warning flags +BCh..+BFh, the analog
selector over axes 104h/105h/11Ch/11Dh/126h/127h, and the highlight text. When the action is not
held the code goes to 0064F311, where the repair route needs **00535EE0(EFh) released**. The
warning pulse at 0064F39A runs only while a flag is set, and the flags are written only inside the
menu. Repair mode (+157h) is entered only from the menu.

`ship_screen_controls_0064e415` transcribes this flow:
1. **The ship gate:** the controlled unit is present, is a ship (kind 6) and is the local player.
2. **Turn-to-camera, for a non-submarine:**
   - action 98h pressed, or held with +160h at or above zero: at +160h below zero it stores dt
     and routes the order; otherwise +160h grows by dt and resets to -1 past 1.0 when 009539E0
     agrees;
   - otherwise, with +160h at or above zero, +160h = -1 and unit+630h = -1.
3. **The repair gate:** a ship whose class `Repair` byte (+D0h) is set, and +157h clear. Then EFh
   held opens the menu; else EFh released with a pick pending routes the order; then the pulse.
4. **The tail:** with +157h set, the repair-mode panel. Otherwise, when the gate at 0064F633
   passes, 00545360 on the screen at [00E198C4]+50h. That routine sets that screen's +D4h to 1 and
   its +1Ch to 0.

The input queries go through the menu host's action records (004C43C0, 004C5090, 00535EE0). Only
press-start 4Eh is ever driven, so every body that sends an order, writes the unit or opens the
menu is a named record that performs nothing and is never reached.

**Substitutions:**
- **The class `Repair` byte** (VehicleClass key, 00962E16) is not loaded. It reads clear and is
  recorded. Both answers reach 0064F496 when nothing is pressed.
- **The local-player test 00927F30** answers true for the controlled unit.
- **The gate at 0064F633** reads [[00E198C4]+64h]+81h and +82h, which the host does not build.
  They read clear, so 00545360 is reached and recorded.

**Switch.** `kHudShipScreenControlsBound`. OFF keeps part 1's single tail record.

**Predictions, written before the pair.** One tree with part 1 on, `kHudShipScreenControlsBound`
off then on, USN04 4500, 2560x1440.
- **Row that leaves:** `HudShipScreen::update_remainder` at 0064E415, 9,158.
- **Rows added:**
  - `HudShipScreen::update_remainder` at 0064F665: 9,158.
  - `HudShipScreen::class_repair_flag`: 9,158.
  - `HudShipScreen::other_screen_00545360`: 9,158.
- **Rows not added:** none of turn_to_camera_order, turn_to_camera_release, turn_timer_expired,
  repair_menu, repair_order_route, warning_pulse or repair_mode_panel appears.
- **Total.** The unimplemented total rises by 18,316.
- **Summary lines.** All identical, since no widget changes.

**The part 2 pair.** `local\p2_off_usn04.log` against `local\p2_on_usn04.log`.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,203,853 | 2,222,169 (+18,316; predicted +18,316) |
| update_remainder at 0064E415 | 9,158 | none |
| update_remainder at 0064F665 | none | 9,158 |
| class_repair_flag | none | 9,158 |
| other_screen_00545360 | none | 9,158 |

- **No gated body ran.** None of the order, unit-write or menu records appears.
- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudShipScreenControlsBound` flips ON.**

## 11. Handoff notes for parts 3 and 4 (read, not bound)

**Where the work stands.** The tail from 0064F665 is the record `HudShipScreen::update_remainder`
(0064F665), about 9,158 calls per USN04 run. Read from Ghidra's decompile; check the listing
before binding.

**Part 3, the damage panel.** It runs whenever the controlled unit exists, input or not.
- **The Icon row.**
  - Icon_5 (+D4h) is shown or hidden by IsKindOf(8), the submarine test.
  - Icon_3 (+C4h) gets the opposite answer.
- **The selected damage index** (+150h) comes from unit+A44h (dword 0x291):

  | unit+A44h | 0 | 4 | 3 | 5 | 1 | 2 |
  | --- | --- | --- | --- | --- | --- | --- |
  | +150h | -1 | 0 | 1 | 2 | 2 | 3 |

- **Hl_1..4** (+D8h..+E4h) fade: the widget's color is read through +54h and written through
  +50h. The selected one's alpha grows by 2*dt up to 1; the others shrink by 2*dt down to 0.
- **The circles** are shown only while a ratio is above 0.01 (00D7A238); a shown circle's
  progress goes to a GUI timed entry (00AA8B00(2)) or, when +188h is set, to 00ABE6E0:
  - **circle_3 (+F0h), engine jam, surface ship:** 0093A3F0 over the `EngineJam` entry of the
    failure-descriptor vector at settings+3E8h (20h stride, name at +8h, seconds at +Ch).
  - **circle_3, submarine:** (settings+4C4h - unit+125Ch) / settings+4C4h, with an upper bound at
    00CED5D0.
  - **circle_1 (+E8h), water:** 00939F80 (repair task +34h) over unit+A60h.
  - **circle_2 (+ECh), fire:** 00939F70 (task +38h) over unit+A5Ch.
  - **circle_4 (+F4h), worst device:** the largest (+36Ch - +370h)/+36Ch over the unit's
    device list (+48h, next +44h). A device counts when it is kind 4, is not kind 0Fh, is kind
    20h, has [+3F4h]+80h not equal to 1, and has byte +378h set.
- **Host gaps.** The host has no repair task (fire and water seconds), device damage fractions or
  failure descriptors, so the circles read zero and hide. unit+A44h also needs a producer read.

**Part 4, the gauge tail** (0065005C..006500C1).
- With +104h set: 0043B370 on +7Ch, with 00852300(unit, dt), the device count over +48h.
- With +107h set: 0043B370 on +80h, with 00815850, which is +104Ch plus 00810E90.
- Then +188h is cleared.
- Still to read between 0064FB44 and 0065005C: the speed, recon and torpedo digit widgets
  (+60h..+74h) and ship_dir_Icon (+50h).

## 12. Part 3: the damage panel, 0064F665..0064FD24

Packet `cc9_ship_screen_parts34` (worker cc9-platform2, 2026-09-23). Read from the listing of
0064DD30; the decompile loses the stack arguments of every virtual call in this range.

**Corrections to section 11.**
- The panel reads the **controlled unit** (00E188D8, reloaded at 0064F62B and 0064F65F), not +184h.
- The settings descriptor vector at GameSettings+3E4h has a **14h** stride (the `/14h` idiom
  `IMUL 66666667h; SAR EDX,3` at 0064F904), not 20h. Its begin is +3E8h and its end +3ECh.
- The EngineJam numerator 0093A3F0 reads the **task's** own active-failure vector (task+18h..+1Ch,
  10h stride), and the panel divides it by the settings record's seconds.
- Circle_1 divides task+34h by unit+A60h and circle_2 divides task+38h by unit+A5Ch. Both divisors
  are loaded as floats (`FLD float ptr`), so docs/UNIT_FIRE_AND_REPAIR.md's "u32 cleared when the
  timer expires" for task+3Ch/+40h is a float duration here.

**The flow** (`bsp::ship_screen_damage_panel_0064f665`):
1. 0064F665..0064F678: return unless the controlled unit exists and answers IsKindOf(6).
2. 0064F67E..0064F6B5: Icon_5 (+D4h) visible for a submarine (IsKindOf(8)), Icon_3 (+C4h) otherwise.
3. 0064F6B7..0064F71C: +150h from unit+A44h: 0 gives -1, 4 gives 0, 3 gives 1, 5 and 1 give 2,
   2 gives 3. Any other value leaves +150h unchanged.
4. 0064F726..0064F7C3: Hl_1..4 (+D8h..+E4h). The colour comes back through virtual +54h into a
   stack quad and goes out through +50h. The selected highlight's alpha grows by `(dt+dt)` while
   below 1.0 and is set to 1.0 otherwise, with no clamp after the add. The others shrink by
   `(dt+dt)` while above zero and are set to zero otherwise.
5. Circle_3 (+F0h). A submarine shows `(settings+4C4h - unit+125Ch) / settings+4C4h` when it is
   above 0.01f and below 0.99 (00CED5D0). A surface ship shows `0093A3F0(unit+A20h) / seconds`,
   where seconds belongs to the last settings descriptor named `EngineJam`, when above 0.01f.
6. Circle_1 (+E8h): `00939F80(unit+A20h) / unit+A60h` above 0.01f.
7. Circle_2 (+ECh): `00939F70(unit+A20h) / unit+A5Ch` above 0.01f.
8. Circle_4 (+F4h): the largest `(+36Ch - +370h) / +36Ch` over the device list (+48h, next
   +44h), counting a device that answers IsKindOf(4), not IsKindOf(0Fh), IsKindOf(20h), has
   `[+3F4h]+80h != 1` and has byte +378h set. It is shown above 0.01f.
9. A shown circle calls virtual +34h(1). With +188h set it calls 00ABE6E0(ratio, 0, 0, 1.0),
   otherwise it gets 00AA8B00(widget, 2)'s timed entry and writes +0Ch = clamp(ratio, 0, 1) and
   +10h = 1.0. A hidden circle calls +34h(0). Every compare is `JBE`, so a 0/0 ratio hides.

**+188h.** 0064D590 sets it when the unit changes, and the update clears it at 006500C1. The host's
hand-off now sets it the same way, and the tail clears it whenever the panel is bound.

**Substitutions and records** (each answers the image's own "no data" value):

| record | address | stands for |
| --- | --- | --- |
| `HudShipScreen::repair_task` | 0064F6BD | the repair task at unit+A20h: priority +24h, the timers +34h/+38h, the divisors unit+A5Ch/+A60h and 0093A3F0. Its constructor is unread and nothing in the host produces it. All read zero, so +150h = -1 and circles 1 to 3 hide. |
| `HudShipScreen::settings_failure_descriptors` | 0064F8E7 | GameSettings+3E4h, not loaded; the loop finds no EngineJam and answers 0 |
| `HudShipScreen::device_list` | 0064FBEC | the device list with +36Ch/+370h/+378h, not modelled; it reads empty |
| `HudShipScreen::submarine_circle_terms` | 0064F7EB | unit+125Ch and GameSettings+4C4h (submarines only) |
| `HudShipScreen::circle_quad`, `circle_progress` | 00ABE6E0, 00AA8B00 | a shown circle's progress; the bridge draws neither |

**Switch.** `kHudShipScreenDamageBound`. OFF keeps part 2's tail record at 0064F665. ON runs the
panel, then either part 4 or the record `HudShipScreen::update_remainder` at 0064FD24.

**Predictions, written before the pair.** One tree, main 528a673ea plus this branch, with parts 1
and 2 on, `kHudShipScreenGaugesBound` off, and `kHudShipScreenDamageBound` off then on. USN04 4500,
`BSP_GUNNERY_RNG_STREAMS=1`, back buffer 2560x1440. The controlled unit is Lexington-class01, a
surface ship.
- **Row that leaves:** `HudShipScreen::update_remainder` at 0064F665, 9,158.
- **Rows added:** `update_remainder` at 0064FD24, `repair_task`, `settings_failure_descriptors` and
  `device_list`, 9,158 each.
- **Rows not added:** `submarine_circle_terms`, `circle_quad` and `circle_progress`.
- **Total.** The unimplemented total rises by 27,474 (three new records; the tail record moves).
- **Summary lines.** Every gameplay line is identical. The text line is identical. The sprite
  line's `quads` may move by one or two: Icon_5 is now hidden and Icon_3 shown each frame. The
  circles are Sections with no texture, and the Hl fade to alpha 0 changes no quad count, because
  the bridge does not cull by alpha.

**The part 3 pair.** `local\p3_off_usn04.log` against `local\p3_on_usn04.log` (worker tree), built
from one tree on main 528a673ea plus this branch, back buffer 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,221,102 | 2,248,576 (+27,474; predicted +27,474) |
| update_remainder at 0064F665 | 9,158 | none |
| update_remainder at 0064FD24 | none | 9,158 |
| repair_task, settings_failure_descriptors, device_list | none | 9,158 each |

- **Rows not added:** none of `submarine_circle_terms`, `circle_quad` or `circle_progress` appears.
- **Summary lines.** 156 of 157 are identical, and every gameplay line is among them. The sprite
  line's `quads` goes from 185 to 184, inside the predicted one or two.
- `PlatformLoopCallbacks::pretranslate` moves (18 to 23), as it did in the part 2 pair; it counts
  window messages.
- **Result.** Every prediction holds. **`kHudShipScreenDamageBound` flips ON.**

## 13. Part 4: the direction spring and the digit gauges, 0064FD24..006500C1

**Corrections to section 11.** The +7Ch gauge has three sources, tested in order: with +105h the
int unit+638h; else with +106h zero when unit+1124h equals 0.0f, otherwise the int
`[[unit+538h]+790h]`; else with +104h `00852300(unit)`. 00852300 is `int __fastcall(unit)`: it
counts the devices on +48h that answer IsKindOf(25h) and virtual +210h(2Dh, 0). The dt it
appears to take is 0043B370's second argument, pushed early. 00815850 is `int __fastcall(unit)`:
`unit+104Ch`, plus `00810E90()` when that is not negative.

**The flow** (`bsp::ship_screen_gauges_0064fd24`):
1. 0064FD24..0064FD50: 00939F70 and 00939F80 on +184h's task; both results are popped, so they
   have no effect.
2. 0064FD52..0064FD6B: r = 0064AAD0(screen) with +184h set, else 0.
3. 0064FD71..0064FDB5: when +128h differs from r (FUCOMIP; unordered counts as different),
   `+12Ch += (+128h - r) * 4.0` and +128h = r.
4. 0064FDBB..0064FF9D: when +12Ch is not 0.0f, a step gated on the platform clock: the clock
   (01090AB0 vtable +14h, a counter/frequency pair scaled by 00CE47A0) must pass the static
   00E19808 + 21h. The step then stores the clock in 00E19808, integrates +124h and decays +12Ch.
5. 0064FFA3: stop without +184h.
6. 0064FFB1..0064FFDE: the speed gauge +78h gets `0092D730([unit+1018h]) * 1.944`.
7. 0064FFE3..00650088: the +7Ch gauge from the source above.
8. 0065008D..006500BC: with +107h, the +80h gauge gets `00815850(unit)`.
9. 006500C1: +188h = 0.

**0064AAD0** is `float __thiscall(screen)`, RET. Ghidra's decompile gives the wrong return. The
routine returns the float at [ESP] (0064ABC5), which is the normalised rudder r, not the angle.
- `x = -unit+984h`, the ordered rudder.
- With x > 0.05, `r = min((x - 0.05)/0.95, 1)`. With x < -0.05, `r = max((x + 0.05)/0.95, -1)`.
  Otherwise r = 0.
- With r nonzero, `s = (r > 0 ? 10 : -10) * pi / 180 + r * 0.29670598` (00CF5C70), else s = 0.
- It sets ship_dir_Icon (+50h) to rotation `2*pi - s` through virtual +44h.

**The gauge 0043B370** is `__thiscall(gauge, float value, float dt)`, RET 8. The gauge is
0043DF90's 1Ch object: a vector of two digit icons, then +10h value, +14h rate and +18h primed.
- q = 004396F0(value): the floor, plus 1 when the fraction's magnitude exceeds 0.5 (00CE3800 is
  the float 0.5).
- On the first call: +10h = q, 0043B2F0(q), +14h = 0, +18h = 1.
- Afterwards: `k = clamp(dt * 4, 0, 1)`, `+14h = (k * ((q - +10h) - +14h) + +14h) * 0.875`,
  `+10h += k * +14h`, 0043B2F0(+10h), +18h = 1.
- 0043B2F0 calls 0043ABA0(i, digit_i, |v|) on each digit, then stores +10h = v and +18h = 0.
- 0043ABA0 turns the value into decimal digits and rewrites the digit icon's vertex UVs through
  the widget's vertex stream (+A0h, lock +10h, unlock +14h).

**Substitutions and records:**

| record | address | stands for |
| --- | --- | --- |
| `HudShipScreen::gauge_digit_uv` | 0043ABA0 | the digit UV roll; the bridge draws a widget's first authored state only |
| `HudShipScreen::dir_clock_step` | 0064FDD4 | the clock-gated step. The HUD host has no platform clock. +124h and +12Ch have no other reader: a disp32 scan of 124h finds only the enter (0064BD9A) and this block. The step is recorded and not applied. |
| `HudShipScreen::apply_unit_0064ae20` | 0064AE20 | the apply routine that sets +104h..+107h, the recon and torpedo icons and NoRepairGUI. It is not bound, so the flags keep the enter's clear values and only the speed gauge runs. Recorded once per hand-off. |
| `gauge_source_638`, `gauge_source_class_790`, `gauge_source_00852300`, `gauge_source_00815850` | | the three +7Ch sources and the torpedo count; never reached while the flags are clear |

The speed is `GameUnitsHost::unit_forward_speed_0092d730`, the reconstructed 0092D730, on the
screen's unit. The host returns it as a float where the image leaves it on the x87 stack.

**Switch.** `kHudShipScreenGaugesBound`. OFF keeps one record at 0064FD24.

**Predictions, written before the pair.** One tree with parts 1 to 3 on, `kHudShipScreenGaugesBound`
off then on, and the same run parameters as the part 3 pair.
- **Row that leaves:** `HudShipScreen::update_remainder` at 0064FD24, 9,158.
- **Rows added:** `gauge_digit_uv`, 18,316 (two digits of the speed gauge per frame), and
  `apply_unit_0064ae20`, once per 45h hand-off (1 expected).
- **Rows not added:** the four `gauge_source_*` rows. `dir_clock_step` is also expected to be
  absent: the run's order is rudder 0.000, so r stays 0 and +12Ch stays 0.
- **Total.** The unimplemented total rises by 9,159.
- **Summary lines.** All identical. The ship_dir_Icon rotation (2*pi at r = 0) changes no quad
  count, and the digit icons keep their authored UVs.

**Coverage** (parts 3 and 4):

| routine | C++ | coverage |
| --- | --- | --- |
| 0064DD30 block 0064F665..0064FD24 | `ship_screen_damage_panel_0064f665` | complete; the repair task, descriptors and device list are host records |
| 0064DD30 block 0064FD24..006500C1 | `ship_screen_gauges_0064fd24` | partial: 0064FDD4..0064FF9D (the clock-gated step) is the record `dir_clock_step` |
| 0064AAD0 | `ship_screen_dir_0064aad0` | complete |
| 0043B370 with 0043B2F0 | `ship_screen_gauge_0043b370` | complete; 0043ABA0 is the record `gauge_digit_uv` |
| 004396F0 | `ship_screen_gauge_round_004396f0` | complete |
| 0064AE20, 0064B370, 0043ABA0 | none | read only |

**The part 4 pair.** `local\p3_on_usn04.log` against `local\p4_on_usn04.log`, same tree.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,248,576 | 2,257,735 (+9,159; predicted +9,159) |
| update_remainder at 0064FD24 | 9,158 | none |
| gauge_digit_uv | none | 18,316 |
| apply_unit_0064ae20 | none | 1 |

- **Rows not added:** no `gauge_source_*` row and no `dir_clock_step` row.
- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudShipScreenGaugesBound` flips ON.**

## 14. Screens 26h, 2Eh and 3Eh: the base update

Packet `cc9_screen_26h_2eh` (worker cc9-platform2, 2026-09-23).

**Evidence.** Each registry slot's vtable is found through its enter virtual at +10h, and +20h is
read from the image:

| slot | vtable | +20h |
| --- | --- | --- |
| 26h | 00CEC9B0 | 004F75C0 |
| 2Eh | 00CEDF34 | 004F75C0 |
| 3Eh | 00CF4544 | 004F75C0 |

004F75C0 (`BSP_FrontEndScreen_BaseUpdate`) is a bare `RET 4`, followed by INT3 padding. Every
other slot of the 42 has its own update. So the pump's call on these three slots does nothing,
and the host's `FrontEndScreen::update` record for them stands for no missing behaviour.

**Binding.** `kHudBaseUpdateScreensBound`. ON logs the pump's call on slots 26h, 2Eh and 3Eh as
the done row `FrontEndScreen::base_update` (004F75C0). OFF keeps them under the record.

**Predictions, written before the pair.** One tree with parts 1 to 4 on, the switch off then on,
USN04 4500 as before. 26h and 2Eh are in the 25h level-1 set; 3Eh is not.
- `FrontEndScreen::update` falls by 18,316 (two slots x 9,158).
- `FrontEndScreen::base_update` appears as done with 18,316 calls.
- The unimplemented total falls by 18,316.
- Every summary line is identical.

**The pair.** `local\p5_off_usn04.log` against `local\p5_on_usn04.log`, one tree with parts 1 to 4
on, back buffer 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,257,735 | 2,239,419 (-18,316; predicted -18,316) |
| FrontEndScreen::update | 73,277 | 54,961 |
| FrontEndScreen::base_update | none | 18,316 done |

- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudBaseUpdateScreensBound` flips ON.** The OFF total
  equals the part 4 ON total, so the switch-off build is neutral.

## 15. Screen 27h's update writes gameplay state

27h's update 0067BB50..0067BC59 (vtable 00CF7A38) was read and **not bound**. With no input gate
it calls `0077C470` (`BSP_UnitInstance_SendRoleTransfer`, a session message 4Bh) on +30h's unit:
- when +30h changes away from the controlled unit;
- when the local player slot game+18ECh is 0..7, depending on the screen's +4h byte and 0059BBD0;
- when no controlled unit exists.

It also moves an observer pair through 006952A0 and 00694A60. Under the brief's rule this screen
waits for the lead's decision.

## 16. Screen 50h, the warning screen: 00683020

Packet `cc9_screen_50h` (worker cc9-platform2, 2026-09-23). Ghidra has no function at 00683020.
The start and exclusive end are **00683020..006832E9**: the `RET 4` at 006832E6 is followed by
padding, and the jump table sits at 006832EC. The routine was read from the disk listing
(`disasm-raw`). The screen is `BSP_HudWarningScreen_Register`'s (00682740, GUI_Warning). Its layout
006823C0 binds first_Group (+68h), warning_text (+50h), warning_1_Icon (+54h) and
warning_2_Icon (+58h).

**The four alerts.** Each has a flag at +28h+i, a hold at +8h+4i and a pulse phase at +18h+4i.

| i | test (controlled unit, alive: byte +5Dh clear) | show routine | text |
| --- | --- | --- | --- |
| 0 | IsKindOf(18h) and `007C6E10([unit+3D0h]) != 0` | 00682ED0 | `ingame.warning_stall` |
| 1 | IsKindOf(8), `0.2 > unit+127Ch` (00CE3D10) and `00852860(unit)` | 00682CB0 | `ingame.warning_o2` |
| 2 | IsKindOf(6) and byte unit+1011h | 00682D60 | `ingame.warning_shallowwater` |
| 3 | `00681F40(game, &unit+FCh, 0.0)`, after 00414DB0 when byte +C8h is clear | 00682E10 | `ingame.warning_exitezone` |

A test that holds sets the flag and hold = 1.0. If the flag was clear, it also zeroes the phase.

**The rest of the update:**
- 00683181: when +74h (last frame's unit) is set and differs from the controlled unit, every
  alert is cleared and its sound gets virtual +8(1).
- 006831C2: with +2Ch clear (the register clears it), first_Group is hidden and the three
  warning widgets get alpha 1.0.
- 00683205..006832D3, per alert:
  - A tracked sound whose +Ch reports finished is released.
  - An active alert's hold drops by dt.
  - The first alert, in index order, whose hold is still non-negative is shown through the jump
    table at 006832EC. Every other active alert stops its sound (+8(0)) and clears its flag.
- 006832DF: +74h = the controlled unit.

Each show routine advances its phase by +78h (the stored dt) and shows first_Group. It sets the
text and pulses warning_text and warning_2_Icon at `(sin(phase * 2pi) + 1) * 0.5 * [00CEFFA0] +
[00CE3DC8]`. It starts the alert's sound through 00682800, and the stall alert also triggers an
award-tracker hint. The show routines are records here. They are reached only when an alert
fires, so this update binds no gameplay write.

**Substitutions and records:**

| record | address | stands for |
| --- | --- | --- |
| `HudWarningScreen::contact_latch_1011` | 006830A5 | unit+1011h, the kind-8 contact latch that 008255B0 rotates from +1010h (docs/UNIT_INSTANCE_UPDATE.md). The host has no terrain contacts, so it reads clear. |
| `HudWarningScreen::world_edge` | 00681F40 | GGame+711Ch..+7130h, the world bounds, which are unmodelled; it reads "not near" |
| `plane_stall`, `submarine_oxygen`, `submarine_below` | 007C6E10, 006830E6, 00852860 | plane and submarine terms |
| `show_alert`, `sound_*` | the show routines, 006831B5, 0068321D, 0068322F | reached only after an alert fires |

The host's pose is always current, so the +C8h test answers "current" and 00414DB0 is not called.

**Switch.** `kHudWarningScreenBound`. OFF keeps the `FrontEndScreen::update` record for slot 50h.

**Predictions, written before the pair.** One tree with every earlier switch on, the switch off
then on, USN04 4500 as before. The controlled unit is the Lexington, a ship.
- `FrontEndScreen::update` falls by 9,158, from 54,961 to 45,803.
- `HudWarningScreen::update` appears as done with 9,158 calls.
- `contact_latch_1011` and `world_edge` appear with 9,158 calls each. No plane, submarine, sound or
  `show_alert` row appears.
- The unimplemented total rises by 9,158.
- Every gameplay line is identical. The sprite `quads` may fall if the host drew first_Group,
  which the update now hides each frame. The alpha writes change no count.

**The pair.** `local\p6_off_usn04.log` against `local\p6_on_usn04.log`, one tree, back buffer
2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,239,419 | 2,248,577 (+9,158; predicted +9,158) |
| FrontEndScreen::update | 54,961 | 45,803 |
| HudWarningScreen::update | none | 9,158 done |
| contact_latch_1011, world_edge | none | 9,158 each |

- **Rows not added:** no plane, submarine, sound or `show_alert` row.
- **Summary lines.** 156 of 157 are identical, and every gameplay line is among them. The sprite
  `quads` did not move.
- **The miss.** The text bridge line's `widgets` goes from 9,339 to 9,338. warning_text sits under
  first_Group, which the update now hides each frame as the image does, so the host no longer draws
  it as authored. I had not predicted a text line would move.
- **Result.** Every row prediction holds. **`kHudWarningScreenBound` flips ON.**

## 17. Screen 49h, the follow-unit pick: 0067BF00

Packet `cc9_screen_49h` (worker cc9-platform2, 2026-09-23). Ghidra has no function at 0067BF00.
The start and exclusive end are **0067BF00..0067BFCA**: `RET 4` at 0067BFC7, then INT3 padding.
The routine was read from the disk listing. vtable 00CF6D68 +20h, `__thiscall(screen, float dt)`
with dt unused.

**What it does.** It writes one field, +8h, and draws nothing:
1. +8h = 0. Stop unless screen 29h ([00E198C4]+CCh) is applied (+5h).
2. The target is `00927880(controlled)`, which goes through the unit's vtable +114h and that
   object's +18h. It is kept only when it answers IsKindOf(2).
3. The pick is screen 29h's unit (+4Ch) when it is alive and visible (the 0043F080 bytes) and is
   not the controlled unit; otherwise the target.
4. +8h = the pick, then 0 when it equals the controlled unit.
5. A non-null +8h must be alive and visible. A plane (IsKindOf(18h)) also needs a live
   `[unit+3D0h]` (0043F080 on it).

**Substitutions and records:**

| record | address | stands for |
| --- | --- | --- |
| `HudFollowScreen::screen_29h_unit` | 0067BF44 | screen 29h's +4Ch, stored by 29h's own update (005272E5, from 00526A40), which is not bound; it reads null |
| `HudFollowScreen::controlled_target` | 00927880 | the two unread virtuals; it answers none |
| `HudFollowScreen::leader_3d0` | 0067BFAF | a plane pick's +3D0h |

Screen 29h's applied byte comes from the registry through the new
`GameMenuHost::in_game_screen_applied`.

**Switch.** `kHudFollowScreenBound`. OFF keeps the `FrontEndScreen::update` record for slot 49h.

**Predictions, written before the pair.** One tree with every earlier switch on, the switch off
then on, USN04 4500.
- 49h is also in the early 20h pumps, so it runs about 9,160 times. `FrontEndScreen::update`
  falls by that count, and `HudFollowScreen::update` appears as done with it.
- `screen_29h_unit` appears once per pump in which 29h is applied (up to 9,160).
  `controlled_target` appears once per pump that has a controlled unit (about 9,158).
  `leader_3d0` does not appear.
- The unimplemented total rises by about 9,158, the two records less the moved update.
- Every summary line is identical, because +8h is not drawn by this update.

**The pair.** `local\p7_off_usn04.log` against `local\p7_on_usn04.log`, one tree, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,248,577 | 2,257,737 (+9,160; predicted about +9,158) |
| FrontEndScreen::update | 45,803 | 36,643 |
| HudFollowScreen::update | none | 9,160 done |
| screen_29h_unit, controlled_target | none | 9,160 each |

- A controlled unit exists in every 49h pump, the early 20h ones included, so `controlled_target`
  counts 9,160, not the 9,158 I estimated. `leader_3d0` does not appear.
- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudFollowScreenBound` flips ON.**

## 18. Screen 46h, the ship view: 0064D610 (part 1)

Packet `cc9_screen_46h` (worker cc9-platform2, 2026-09-23). Ghidra has no function at 0064D610.
The start and exclusive end are **0064D610..0064D731** (two `RET 4` exits, the last at 0064D72E).
The routine was read from the disk listing. vtable 00CF7978 +20h, `__thiscall(screen, float dt)`,
SEH frame. The 25h arm's hand-off on interface+7Ch, 0064DA40, stores the unit at +1Ch.

**The flow:**
1. 0064D62B..0064D639: stop unless +4h (the wanted byte) is set and +1Ch is non-null.
2. 0064D647: 0064A400(dt). It calls screen 26h's 0051F330: 0051EF00 and 0051F050 read the camera
   axes and the view and fire actions, and move the camera mover. With 46h's +20h set, it also
   calls screen 2Eh's 005454B0, which stores three floats and forwards two to screen 4Dh's
   00637620.
3. 0064D656: 0064B870(dt), `BSP_HudUnitOrder_UpdateIntegratedControls`.
4. 0064D65B..0064D66D: screen 2Eh's 005484F0 (3.3 KB: input actions, interface requests and a
   session route), when [00E198C4]+50h exists.
5. 0064D675: 00815850(unit), whose result is discarded.
6. 0064D680..0064D71A: on input action 95h pressed, with the unit's vtable +234h(0) and the
   local-player test 00927F30, an order is routed. A kind-0Ch unit goes through 00812960, 00465080
   and 0077D600; any other through 0064A820 and `0077C2A0(msg, 2, 0)`. Input-gated, so these are
   records never reached.

**Part 1 binds the flow above.** Steps 2 to 4 are the records `HudShipView::view_input`,
`integrated_controls` and `screen_2eh_005484f0`. Each is a later part. The +4h byte comes from the
registry, and the +1Ch presence comes from the host's 0x7C hand-off.

**Switch.** `kHudShipViewScreenBound`. OFF keeps the `FrontEndScreen::update` record for slot 46h.

**Predictions, written before the pair.** One tree with every earlier switch on, the switch off
then on, USN04 4500.
- `FrontEndScreen::update` falls by about 9,158. `HudShipView::update` appears as done with the
  same count.
- `view_input`, `integrated_controls` and `screen_2eh_005484f0` appear with that count each, in
  every pump where +4h is set and the unit is bound. No `unit_virtual_234`, `unit_00812960` or
  `order_route_*` row appears.
- The unimplemented total rises by about 2 x 9,158 = 18,316.
- Every summary line is identical.

**The part 1 pair.** `local\p8_off_usn04.log` against `local\p8_on_usn04.log`, one tree, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,257,737 | 2,276,053 (+18,316; predicted about +18,316) |
| FrontEndScreen::update | 36,643 | 27,485 |
| HudShipView::update | none | 9,158 done |
| view_input, integrated_controls, screen_2eh_005484f0 | none | 9,158 each |

- No order-route or unit-virtual row appears.
- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudShipViewScreenBound` flips ON.**

## 19. Screen 46h part 2: the integrated controls 0064B870

`bsp::integrated_controls_0064b870`, read from the listing. `__thiscall(screen 46h, float dt)`,
RET 4, body 0064B870..0064BB48. The existing fragment `issue_hud_order_fragment_0064b870`
(0064BA97..0064BB16) covers the order it sends.

**The flow:**
1. 0064B879: with byte [+1Ch]+6C8h set, it reads the input manager's turn axis +1BE4h, thrust
   axis +1BB4h, the two binding queries 00A92050/00A92090 on +1B90h, and later the byte +1B91h.
2. "Thrust moved":
   - With 00A92050, it is "+30h clear". When +30h is set and +34h differs from the thrust axis,
     +30h is cleared instead.
   - Without it, it is |thrust| > 0.1 (00D7A3A0). The magnitude is `-0.0 - x` for x <= 0.
3. 0064B96C..0064B9BD: when |turn| > 0.1 or the thrust moved, with [+1Ch]+1130h zero and the
   player holding role 0 but not role 1, it calls `0077C470(unit, 2, 1)` for the transfer. It then
   seeds +28h from unit+984h and +24h from unit+980h.
4. 0064B9C5..0064BB12, only while the player holds role 1:
   - +24h integrates the thrust axis: `+24h - thrust*dt` without a device query. With one, and
     +1B91h set, it is `-thrust*0.5` for positive thrust, else `-0 - thrust`.
   - It is clamped to [-0.5, 1] by 00415690.
   - +28h = `+28h - turn*dt`, clamped to [-1, 1].
   - The quantised order goes out through 00816A40.
5. 0064BB19..0064BB3C: role 1 is given back (`0077C470(2, 0)`) while game+19C4h is set.

**Substitutions and records:**
- The +6C8h gate reads set. The unit constructor stores 1 at 0095CE29, and a disp32 scan finds no
  other byte writer. No record.
- `HudShipView::control_inputs` (004BEC00): the input manager fields read zero and clear.
- `HudShipView::local_player_role` (00927F30): the host has no role table. It answers role 0 held
  and role 1 not held (docs/CONTROLLED_UNIT_HELM.md section 6).
- `role_transfer`, `issue_order`, `unit_1130` and `game_19c4` are reached only after input.

**Switch.** `kHudShipViewControlsBound`. OFF keeps part 1's `HudShipView::integrated_controls`
record.

**Predictions, written before the pair.** Part 1 on, the switch off then on, USN04 4500.
- **Row that leaves:** `HudShipView::integrated_controls` at 0064B870, 9,158.
- **Rows added:**
  - `HudShipView::integrated_controls_0064b870`: done, 9,158.
  - `control_inputs`: 9,158.
  - `local_player_role`: 18,316, from the role-1 tests at 0064B9C5 and 0064BB1E. The role-0 test
    is short-circuited because no axis moves.
- **Rows not added:** no `role_transfer`, `issue_order`, `unit_1130` or `game_19c4` row.
- **Total.** The unimplemented total rises by 18,316.
- **Summary lines.** All identical, and the controlled unit's motion is unchanged.

**The part 2 pair.** `local\p9_off_usn04.log` against `local\p9_on_usn04.log`, one tree, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,276,053 | 2,294,369 (+18,316; predicted +18,316) |
| integrated_controls (record) | 9,158 | none |
| integrated_controls_0064b870 | none | 9,158 done |
| control_inputs | none | 9,158 |
| local_player_role | none | 18,316 |

- No `role_transfer`, `issue_order`, `unit_1130` or `game_19c4` row appears.
- **Summary lines.** All 157 are identical.
- **Result.** Every prediction holds. **`kHudShipViewControlsBound` flips ON.**

## 20. Handoff (cc9-platform2, 2026-09-23)

**Landed on agent/cc9-platform2**, each switch ON by its own USN04 pair:

| switch | code | what it runs |
| --- | --- | --- |
| `kHudShipScreenDamageBound` | 0064F665..0064FD24 | 45h's damage panel |
| `kHudShipScreenGaugesBound` | 0064FD24..006500C1 | 45h's direction icon and digit gauges |
| `kHudBaseUpdateScreensBound` | 004F75C0 | slots 26h, 2Eh and 3Eh, a bare `RET 4` |
| `kHudWarningScreenBound` | 00683020 | 50h's four warnings |
| `kHudFollowScreenBound` | 0067BF00 | 49h's follow pick |
| `kHudShipViewScreenBound` | 0064D610 | 46h's top-level flow |
| `kHudShipViewControlsBound` | 0064B870 | 46h's integrated controls |

The screen code for 49h, 50h and 46h is in `src/hud_warning_screen.cpp`.

**What is left of the in-mission screens:**
- **27h, 0067BB50..0067BC59.** Already reconstructed as `GameUnitsHost::Impl::role_screen_update_0067bb50`
  (kPlayerRoleBookkeepingBound, docs/SCRIPTED_HELM.md section 6.1). The pump keeps its record
  (section 21).
- **46h part 3** is bound (section 22).
- **46h part 4, `HudShipView::screen_2eh_005484f0`.** 005484F0 is 3.3 KB of screen 2Eh: seven
  input actions, 004C5090 holds, two interface requests (`BSP_FrontEndManager_PushInterfaceRequest`)
  and a session route. Read it whole; expect input-gated order and UI-mode writes. Its first gates:
  - 2Eh's +40h, +24h and [+20h]+14h must all be non-null. The host builds none of them, because
    2Eh's layout 00546A20 is not run.
  - A controlled unit must exist, and it must hold role 0 (00927F30).

  Then the one-shot bytes +108h and +109h (0051E7E0, 006502C0, the mover's +388h/+384h from
  +10Ch/+110h). Read the gate producers first: if the gates fail in the image as well, the whole
  routine is a no-op here.
- **29h, 00527260** (continues past 0052735C) with 00526A40, its large worker. 29h's +4Ch is what
  49h's `screen_29h_unit` record stands for.
- **44h, 00649860..0064A24C**, the HudRoot update. `src/hud_root_rows.cpp` reconstructs part of it,
  but its host is not bound.

**Method notes:**
- Build each pair from one tree: build OFF, copy `bsp_game.exe` and `xlive_stub.dll` to
  `local\bin\<name>`, flip the switch, build ON, copy again. Pass `-Exe local\bin\<name>\bsp_game.exe`
  to `tools/run_game.ps1`, so both runs can queue at once while the tree moves on.
- `local\w\unimpl.py <off.log> <on.log>` (worker tree, not committed) prints the unimplemented
  total, every row that changed and every summary line that moved.
  `PlatformLoopCallbacks::pretranslate` moves between any two runs; it counts window messages.
- Ghidra has no function at 00683020, 0067BF00 or 0064D610. Read those from `disasm-raw`, and
  take their ends from the RET and padding.

## 21. Screen 27h's pump call, and 0064B870's role test (cc9-platform2, 2026-09-23)

**27h stays a record in the pump.** Its update 0067BB50 is already reconstructed as
`GameUnitsHost::Impl::role_screen_update_0067bb50`, under `kPlayerRoleBookkeepingBound`. The units
host calls it once per fixed step, before the unit loop. That member is private to
`GameUnitsHost::Impl`, so the HUD pump cannot reach it. Calling it from the pump as well would run
the role take a second time per step, and the pump runs twice per mission frame.

What the plumbing needs is the owner's change in `src/game_hosts_units.cpp`:
- a public `GameUnitsHost` entry point for the 27h update;
- the fixed-step call removed, or gated off when the pump drives it.

Then the pump's slot 27h can call it once per pump, as the image does. Until then
`FrontEndScreen::update` keeps counting slot 27h, and the role take keeps its fixed-step cadence.

**0064B870's role test.** `GameUnitsHost::unit_current_role_slot` reads `current_roles_01ac`. That
is the table the 27h take and the BSP_PLAYER_HELM transfer write. So `00927F30(unit, role)`
becomes "the holder is slot 0", the local player's game+18ECh in this single-player host. The
switch is `kHudShipViewRoleTableBound`.
- **With the helm option on,** role 1 is held. 0064B870's role-1 branch then runs the lever
  integration from zero inputs and reaches `issue_order`, which is a record. The option's own
  00816A40 issue stays the only one, and so does its transfer: the HUD's transfer needs an axis.
- **The raw bodies**, start and exclusive end, each with INT3 before the start:
  - 00683020..006832E9: the RET 4 is at 006832E6; alignment and the jump table at 006832EC
    follow.
  - 0067BF00..0067BFCA: the RET 4 is at 0067BFC7, and INT3 follows at 0067BFCA. So 0067BFCA
    is exclusive.
  - 0064D610..0064D731: the RET 4 is at 0064D72E, and INT3 follows.

**Predictions, written before the pair.** One tree on main da8398163 plus this branch, every earlier
switch on, `kHudShipViewRoleTableBound` off then on, USN04 4500, no BSP_PLAYER_HELM.
- **Row that leaves:** `HudShipView::local_player_role`, 18,316.
- **Rows added:** none.
- **Why nothing else moves.** The 27h take gives the player role 0, and nothing gives role 1. The
  role-1 test at 0064B9C5 and 0064BB1E therefore still answers false, and no branch changes.
- **Total.** The unimplemented total falls by 18,316.
- **Summary lines.** All identical.

**The pair.** `local\p10_off_usn04.log` against `local\p10_on_usn04.log`, one tree, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,371,778 | 2,353,462 (-18,316; predicted -18,316) |
| HudShipView::local_player_role | 18,316 | none |

- No other row moves.
- **Summary lines.** All 160 are identical.
- **Result.** Every prediction holds. **`kHudShipViewRoleTableBound` flips ON.**

## 22. Screen 46h part 3: the view input 0064A400 (cc9-platform2, 2026-09-23)

Read from the listings. Bodies, start and exclusive end (Ghidra functions exist for all four):

| routine | body | what it is |
| --- | --- | --- |
| 0064A400 | 0064A400..0064A449 | calls 26h's 0051F330, then 2Eh's 005454B0 when 46h+20h is set |
| 0051F330 | 0051F330..0051F360 | calls 0051EF00(dt), then `0051F050(26h+40h, &26h+24h, dt, 26h+38h)` |
| 0051EF00 | 0051EF00..0051F035 | the binoculars raise, the lowered arm and the raised arm |
| 0051F050 | 0051F050..0051F321 | the view axes into the mover, then action 75h and the view arm |

**The mover is the mission camera's.** Screen 26h is the binoculars screen (0051ED60). 0064DA40
hands it the new ShipCaptain mover through 0051E730, which sets 26h+40h. So 0051F050 writes the
same mover `src/mission_camera.cpp` ticks:
- **Yaw.** `+384h = 00438AA0(+384h, min(dt,0.5) * -1 * 26h+38h * axis(+1584h) * GlobalConfig+4)`.
- **Pitch.** `0051E650(min(dt,0.5) * 0.5 * 26h+38h * axis(+15B4h) * GlobalConfig+4)`. It adds with
  wrap into +388h, then clamps into [+3ECh, +3F0h].
- **The axes.** Each is clamped to [-50, 50] (00CE4938/00CE3938).

These are the image's only writes of yaw and pitch after the seed, on the pump's frame path. The
mission camera tick reads them and writes the pose, so the view input is not a second pose
writer. With the axes at zero, both steps are zero. The yaw is rewritten unchanged. The pitch
also stays unchanged, because the seed (-10 degrees, 00CECA08) is inside this installation's
-89..89 limits.

**0051EF00 with no input.** +30h is 0 (the register), the raise axis is 0 and action E0h is not
pressed. So it clears +34h/+35h, hides the Tavcso_Model widget (+20h), and calls 00452B80(0) on
00E081A0. That routine stores the byte, calls 00B0D020 on the renderer object [00F8D39C], and
sets +4h.

**Records and substitutions:**
- `HudShipView::view_input_terms` (0051F061): the input axes read zero. GlobalConfig+4 is unread
  and reads 1.0; it only scales a zero axis.
- `HudShipView::binoculars_lens_off` (00452B80): the renderer call is not modelled.
- `HudShipView::screen_2eh_005454b0`: 2Eh+48h..+50h and 4Dh+44h/+48h have no reader in this host.
- Never reached here: `binoculars_raise_toggle` (0051E7E0), `binoculars_raised_view` and
  `binoculars_view_arm` (after action 75h).

**Switch.** `kHudShipViewInputBound`. OFF keeps part 1's `HudShipView::view_input` record.

**Predictions, written before the pair.** Every earlier switch on, the switch off then on, USN04 4500.
- **Row that leaves:** `HudShipView::view_input`, 9,158.
- **Rows added:** `view_input_0064a400` done, 9,158; `view_input_terms`, `binoculars_lens_off` and
  `screen_2eh_005454b0`, 9,158 each. None of the toggle, raised-view or view-arm rows appears.
- **Total.** The unimplemented total rises by 18,316.
- **Summary lines.** All identical. The camera pose does not move, so the markers' and minimap's
  projections are unchanged. Tavcso_Model is a Model widget, which the sprite bridge does not draw.

**Who else writes +384h/+388h, and in what order.** `src/mission_camera.cpp` writes them in two
places:
- The bind seed, 0064DA40: yaw from the heading, pitch = -10 degrees.
- The update's death re-base, 00432FC8..00433028: `yaw = 00438AA0(yaw, -0 - angle)`, in mode 0
  only, when unit+5Dh is set. 0064DA40 binds a ship's mover in mode 1 (00432E60(unit, 1)), so that
  write does not run for this camera.

Everything else in the tick reads yaw and pitch. The host runs the tick once per interface frame,
from the first of the 4Dh (markers) or 35h (minimap) updates. In the 25h set's order
(`29h 49h 44h 27h 4Dh 45h 46h 26h 2Eh 35h 50h`) that is 4Dh, before 46h. So the view input's write
reaches the pose on the next tick.

In the image the mover ticks as a world entity. Whether that is before or after the interface pump
is not established here, and with zero input the order changes nothing.

**Request to cc9-platform (the camera's owner, on hold).** No change to the tick is needed for
this binding. If the tick is ever moved to the world update, check it against the pump order above:
a view-input step taken after the tick in the same frame would show one frame later. If the mode-0
re-base is ever reached for a ship, it and 0051F050 both write +384h, the tick first.

**The part 3 pair.** `local\p11_off_usn04.log` against `local\p11_on_usn04.log`, one tree on main
plus this branch, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,355,466 | 2,373,782 (+18,316; predicted +18,316) |
| view_input (record) | 9,158 | none |
| view_input_0064a400 | none | 9,158 done |
| view_input_terms, binoculars_lens_off, screen_2eh_005454b0 | none | 9,158 each |

- No toggle, raised-view or view-arm row appears.
- **Summary lines.** All 160 are identical.
- **Result.** Every prediction holds. **`kHudShipViewInputBound` flips ON.**

## 23. Screen 44h, the HUD root: 00649860 (cc9-platform3, 2026-09-24)

Packet `cc9_screen_44h`. `bsp::hud_root_screen_update` (`src/hud_updates.cpp`, from packet
`hud_central_updates`, docs/HUD_CENTRAL_UPDATES.md) was checked against the listing
00649860..0064A24C: the gate, the cadence, the clone pass, the tail and the toggle's kind-18h arm
all match. Ghidra has a function here. What was missing is the host, and three producers.

**Correction.** Field +40h is **ClosedUnitHUD_Group**, not a selector widget. 006463E0 stores it at
00646AAE, from a lookup under Units_Group (+30h, 00646A3E) on the GUI_selector page (+28h,
006469C3). The installed `interface/gui_selector.lua` authors it visible, so 0064A108's show every
frame changes nothing drawn.

**The producers.**
- **+78h, the pending unit handle.** Its only literal-address writer is 0076CFD0 at 0076D021 (a
  scan of every `[00E198C4]` load followed by a `+40h` load and a `+78h` store). 0076CFD0 receives
  session message 4Dh, which 0076CF30 builds, and 0076CF30's one caller is 008AB3D0, Lua
  `MW_MultiSelectUnit`. The receiver stores the handle only for the local player's slot. The
  commit that follows, 00645600 (`BSP_InGameHudRoot_SetControlledUnit`), writes gameplay state,
  but it is gated by a mission-script call, not by input. The host keeps that Lua native
  unimplemented, and USN04 never calls it: `MissionLuaNative::MultiSelectUnit` has no row in
  `p11_on_usn04.log`. So +78h stays 0, in the image as here, and the commit, the camera notify
  and both interface requests are records never reached.
- **+B0h..+B4h, the award ticker queue.** 00648AB0 pushes it. Its one caller is
  `BSP_MissionScoring_GrantAward` (0090EDE0, at 0090EF93). The host's award grants do not reach
  it, so the queue reads empty. That is a substitution, and it is recorded.
- **game+61Fh and +620h, the suppression bytes.** They are the pause bytes: 004D95F0 (the pause
  menu's "set paused") writes +61Fh, and 004D94F0 (tutorial hints) writes +620h. The host never
  pauses. Both read clear, and that is recorded.

**What runs with no input,** per call:
1. The pause gate, then `+F4h` counts down. The body runs on every second call, starting with the
   first, because the enter 006488D0 zeroes `+F4h` and 44h enters once per run.
2. On a body call: both clone vectors are emptied. They are always empty here. Then, with a
   controlled unit, 008E9AF0 fills the power-up vectors. The power-up manager is not built, so it
   answers none and nothing is cloned.
3. The award ticker: the timer is 0 and the queue is empty.
4. +78h is 0.
5. ClosedUnitHUD_Group is shown.
6. The selection tuple 00644CC0/00644C20 is stored at +C2h. The selection poll 00644DB0 and the
   unit rows 00648C20 run. All three are records in this part.
7. The closed-HUD toggle, for a controlled unit that is not kind 9, 45h or 46h. First game+19C4h,
   then action DFh through the menu host's action records. A ship never takes the kind-18h
   `+379h` arm, so 00647080 is not reached.
8. The group tail. The screen's +20h is never set here, and game+1FE4h is zero in single player.

**Records and substitutions:**

| record | address | stands for |
| --- | --- | --- |
| `HudRootScreen::game_pause_bytes` | 00649863 | game+61Fh/+620h, read clear |
| `HudRootScreen::platform_row_inset` | 006499D9 | platform+0Dh, the widescreen byte; it only places the first power-up row |
| `HudRootScreen::collect_powerups` | 008E9AF0 | the power-up manager [00F88C30], five empty vectors |
| `HudRootScreen::award_ticker_queue` | 00649FE0 | the award queue, empty |
| `HudRootScreen::selection_tuple` | 0064A114 | 00644CC0/00644C20; +C2h has no reader in this host |
| `HudRootScreen::selection_poll` | 00644DB0 | the unit selection poll, actions 8Dh/8Ch/8Eh/8Fh |
| `HudRootScreen::unit_rows` | 00648C20 | the unit rows: name, flag, payload, health, command icon |
| `HudRootScreen::game_19c4` | 0064A179 | game+19C4h, read clear |
| never reached | 008E62A0, 00AAB4C0, 00648060, 0064A018, 00645600, 004CC460, 0064A1E0, 00647080, 006485A0 | the clone bodies, the ticker step, the pending commit, the plane arm, the toggle, the group rebuild |

**Switch.** `kHudRootScreenBound`. OFF keeps the `FrontEndScreen::update` record for slot 44h.
00644DB0 and 00648C20 have bounded reconstructions (`src/hud_root_rows.cpp`), and they are left
for their own sub-switches: 00648C20 writes the unit rows and can move the text and sprite lines.

**Predictions, written before the pair.** One tree on main abfe67af8 plus this branch, with every
earlier switch on. The switch is off, then on. USN04 4500, `BSP_GUNNERY_RNG_STREAMS=1`, back
buffer 2560x1440. 44h is pumped 9,160 times, as often as 49h. Its body runs on 4,580 of them.
- **Row that falls:** `FrontEndScreen::update`, from 27,485 to 18,325 (-9,160).
- **Rows added:**
  - `HudRootScreen::update`: done, 9,160.
  - `game_pause_bytes`, `award_ticker_queue`, `selection_tuple`, `selection_poll` and
    `unit_rows`: 9,160 each.
  - `platform_row_inset`: 4,580.
  - `collect_powerups`: 4,580, or up to 2 fewer if the two 20h-interface pumps have no
    controlled unit.
  - `game_19c4`: 9,160, or up to 2 fewer for the same reason.
- **Rows not added:** none of the never-reached records above.
- **Total.** The unimplemented total rises by about 54,960, with 6 fewer at most.
- **Summary lines.** All are identical. One exception is possible: the frontend's show call marks
  ClosedUnitHUD_Group as visibility-applied and invalidates the quads. If the bridge had not drawn
  the GUI_selector page's children as authored, the sprite or text line could move.

**The pair.** `local\h1_off_usn04.log` against `local\h1_on_usn04.log`, one tree on main abfe67af8
plus this branch, back buffer 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,374,478 | 2,429,438 (+54,960; predicted about +54,960) |
| FrontEndScreen::update | 27,485 | 18,325 |
| HudRootScreen::update | none | 9,160 done |
| game_pause_bytes, award_ticker_queue, selection_tuple, selection_poll, unit_rows, game_19c4 | none | 9,160 each |
| platform_row_inset, collect_powerups | none | 4,580 each |

- **Rows not added.** No never-reached record appears.
- **The two ranges.** `collect_powerups` and `game_19c4` took the top of their ranges, so a
  controlled unit exists on every 44h pump, the two 20h-interface pumps included.
- **Summary lines.** All 161 are identical. The show call on ClosedUnitHUD_Group moved no bridge
  line.
- **Result.** Every prediction holds. **`kHudRootScreenBound` flips ON.**

## 24. Screen 29h, the unit pick: 00527260 and 00526A40 (cc9-platform3, 2026-09-24)

Packet `cc9_screen_29h`. Read from the listings. Ghidra has no function at 00527260: its body
follows 00526A40's padding.

| routine | start, exclusive end | ABI |
| --- | --- | --- |
| 00527260 | 00527260..00527BE0: INT3 before the start; the RET 4 at 00527BDD is followed directly by FUN_00527BE0 | `__thiscall(screen, float dt)`, vtable 00CECCF8 +20h |
| 00526A40 | 00526A40..00527256: RET 0Ch at 00527253, INT3 after | `__thiscall(screen, unit* exclude, bool* by_ray, float zoom)` |
| 00522050 | 00522050..005220BD | `__thiscall(screen, const float3*)`, RET 4 |

**What 29h is.** It is the unit-pick screen: the unit near the aim point that the player can
order, take over or follow. Its strings are the ship section names `ingame.sections_magazine`,
`_engine` and `_fuel`, and `Stearring`. The lock logic in its middle sends orders (0077C2A0,
0077D600), moves roles (0077C470), and takes control of the pick (00647300, the HUD root's
SetSpectatedUnit). It also plays a lock sound.

**The pick, 00526A40:**
1. +50h = 0, `*by_ray` = 0.
2. Below full zoom, the zoom is scaled through 00419010 and GlobalConfig+5Ch
   (LockRadiusZoomModifier).
3. With game+19C4h set, the pick is 005A1310(0) on the screen at [00E198C4]+54h.
4. Otherwise it takes the firing unit, 004B4B00: the controlled unit if kind 5, its +3D0h if
   kind 18h. The plane-bot flag is the firing unit being kind 0Fh with slot 8 or an AI-held slot
   (00927F10).
5. A segment is cast from the camera node's position (game+19FCh, world row 3), 10000 along its
   forward (row 2), through 009043A0.
6. A hit with no plane-bot flag runs the hit branch, which starts from the hit unit.
7. Without a pick from the ray, the lock radius is `r = LockRadiusMultipliers[difficulty] *
   (0.03 / zoom)`. The index is 2 in a session (game+1FE4h), else game+6ACh. The radius is
   squared as a float.
8. It walks game+1974h, the local team's live units, then game+19BCh, the kind-35h and grey-arrow
   units. A kind-18h entry contributes up to five members from +3D0h. A candidate must not be the
   excluded unit, be alive (+5Dh clear), be kind 5, and be a grey-arrow member or neither kind
   1Ah nor kind 19h. Its point is its position (+FCh), or 00901C20's intercept for a plane bot.
   00522050 projects the point (0043A660, mode 1), and scores `(0.5-x)^2 + (0.5-y)^2` when all
   three clip bits are set, else 1e10. The smallest score strictly under r^2 wins.
9. After a ray hit, a kind-44h hit keeps a different pick only when 005220C0 accepts it.
10. The pick is dropped unless 0043F080's four bytes pass.

**The update, 00527260, with no input:**
1. +D8h = 0 and [00E18DB7] = 1. The only other reader of that byte is
   BSP_Session_RouteMessage, from the lock branches.
2. +4Ch = 00526A40(controlled, &+B4h, 26h+38h). The observer pair on +18h follows it.
3. +B0h is the pick, or its vtable +140h owner when it is not kind 19h.
4. The local player record's +19h byte ends the update.
5. +ECh counts down by dt to 0.
6. It exits with no controlled unit, or while +ECh is not zero. With game+19C4h set and
   [00E0E350] set, it returns without the tail. Interfaces 30h, 31h and 32h exit.
7. The input flags come from the action records' device bytes, +0Bh and +10h of
   `[[input+4]+30h*action+2Ch]` for actions CEh, CFh, D1h, D3h, D4h and D5h, plus 004C43C0(D0h).
   Without game+19C4h, +B8h = 0.
8. Each lock branch runs only under its flag. With none set, only 004B4B00 and 00523580 run,
   and both only read.
9. The tail: [00E18DB7] = 0, and the sound at 00A7E490 only if +D8h was set.

So with no input, 29h writes only its own fields. It never writes gameplay state.

**Records and substitutions:**

| record | address | stands for |
| --- | --- | --- |
| `UnitPickScreen::input_record_bytes` | 00527419 | the direct device-byte reads, once per update; they answer clear |
| `game_19c4_pick`, `game_19c4` | 00526B05, 005273C5 | game+19C4h, read clear |
| `segment_query` | 009043A0 | the spatial index is not built; no hit |
| `team_unit_list` | 004C3CB0 | game+1974h stand-in: the created units of the controlled unit's party, alive and visible, not kind 2Ah |
| `kind35_list` | 004C3E99 | game+19BCh, empty |
| `squadron_members` | 00526E58 | +3D0h members are not exposed |
| `grey_arrow_set` | 008DDF90 | no set at game+21A4h |
| `gui_extent` | 00AA1FE0 | mode 1's y scale; 1.0, as the markers host's 4/3 law gives |
| `team_record_19` | 0052733C | the local player record's +19h, read clear |
| `owner_140` | 0052731C | vtable +140h of a pick that is not kind 19h; answers none |
| never reached | the ray-hit branch, the plane-bot arm, the lock branches, the zoom arm, the tail sound | |

LockRadiusMultipliers is read from this installation's `scripts/datatables/globals.lua`
(2024-07-13). The file gives 2.0, 1.8 and 1.5.

**Switch.** `kHudUnitPickScreenBound`. ON also answers 49h's `screen_29h_unit` from +4Ch, in place
of that record. OFF keeps both records.

**Predictions, written before the pair.** One tree on 2c2496df2 with every earlier switch on,
`kHudUnitPickScreenBound` off then on, USN04 4500, 2560x1440. 29h is pumped 9,160 times, as often
as 49h.
- **Rows that leave or fall:**
  - `FrontEndScreen::update` falls from 18,325 to 9,165, leaving only slot 27h.
  - `HudFollowScreen::screen_29h_unit` leaves (9,160).
- **Rows added, one per call:**
  - `UnitPickScreen::update`: done, 9,160.
  - `input_record_bytes`, `game_19c4_pick`, `segment_query`, `team_unit_list`, `kind35_list`,
    `team_record_19` and `game_19c4`: 9,160 each. 44h showed a controlled unit on every pump, so
    no exit comes before the `game_19c4` query.
- **Rows added per candidate.** Their counts depend on the team's size, so only their shape is
  predicted:
  - `grey_arrow_set`: once per team unit per call. It is at least 9,160, because the controlled
    unit is walked too, and it falls as units die.
  - `gui_extent`: once per candidate that passes the kind and alive filters.
  - `squadron_members`: once per kind-18h team unit per call, if the stand-in list has any.
  - `owner_140`: once per call with a pick. It is absent if nothing near the screen centre is in
    range.
- **Rows that may appear.** `camera_basis` or `project` can appear a few times, on pumps before
  the mission camera first publishes.
- **Rows not added.** None of these appears: `lock_branch`, `ray_hit_branch`, `tail_sound`,
  `reset_c0`, `lock_radius_multiplier`, `gunbot_intercept`, `spectated_unit`,
  `lock_zoom_modifier`, `byte_e0e350`.
- **Log note.** The lock radius read logs 3 values.
- **Summary lines.** All identical. 29h and 49h write only their own fields, and nothing in this
  host reads 49h's +8h.

**The pair.** `local\h2_off_usn04.log` against `local\h2_on_usn04.log`, one tree on 2c2496df2 plus
this change, 2560x1440.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,429,438 | 2,979,312 (+549,874) |
| FrontEndScreen::update | 18,325 | 9,165 (predicted) |
| HudFollowScreen::screen_29h_unit | 9,160 | none (predicted) |
| UnitPickScreen::update | none | 9,160 done |
| input_record_bytes, game_19c4_pick, game_19c4, segment_query, team_unit_list, kind35_list, team_record_19 | none | 9,160 each (predicted) |
| grey_arrow_set | none | 256,262, about 28 team units per call |
| gui_extent | none | 247,051, the candidates that pass the filters |
| owner_140 | none | 707, the updates with a pick |
| project, camera_basis | none | 51 and 3, before the camera first publishes |

- **Rows not added.** No `squadron_members` row appears, so the stand-in team list holds no
  kind-18h unit. None of the never-reached records appears.
- **Summary lines.** All 161 are identical.
- **The total.** It rises by 549,874. Nearly all of the rise is the two per-candidate records,
  which follow the image's own per-candidate calls (008DDF90 and 00AA1FE0). That rise is the
  price of naming them; the image makes the same calls. A grey-arrow set producer (game+21A4h),
  and the GUI extent handed to the HUD host, would retire both rows.
- **Result.** Every row prediction holds, and the per-candidate rows have the predicted shape.
  **`kHudUnitPickScreenBound` flips ON.**

## 25. Screen 2Eh's 005484F0: its gates pass in the image (cc9-platform3, 2026-09-24)

Section 20 asked whether 005484F0's first gates fail in the image as they do here. **They pass.**
005484F0 is therefore not a no-op, and it was not bound.

**The body.** 005484F0..005491F4: the RET at 005491F3 is followed by INT3. Ghidra has a function
here (FUN_005484F0).

**The gate fields.** The screen's layout 00546A20 does not write them. The constructor 005472F0
zeroes them:
- +20h is an array of 18h-byte weapon-group entries, whose vtable is 00CEDDA0.
- +24h is the array's count.
- +40h is a unit, held through an observer pair at +2Ch.

**Their writer, 00549260.** It is `__thiscall(screen 2Eh, unit group_unit, unit bound)`, RET 8.
- It stores `bound` at +40h through the observer.
- It resizes +20h through 005467B0, then appends the entry 005460A0 builds from `group_unit`
  through 00546730. A kind-1Ch unit adds one entry for each 64h-byte record at +778h (count
  +77Ch).
- When the first entry's unit changed, it resets through 00548410 and runs the per-kind setup
  (kind 6 onward).

Its callers: `BSP_HudShipView_BindUnitCamera` 0064DA40, 00519B00, 005213D0, 00549430, 005494C0,
00650210 and 0067C1A0. Every call passes ECX = [00E198C4]+50h, the 2Eh screen.

**Why the gates pass.** 0064DA40 binds the controlled ship, which is the Lexington in USN04. That
gives:
- +40h non-null;
- +24h at 1 or more;
- [+20h]+14h, the ship.

The role test 00927F30(ship, 0) then answers "held", as section 21 established for the 27h take.

**What binding needs:**
1. 00549260's group build. It belongs with 0064DA40, which `src/mission_camera.cpp` models;
   cc9-platform owns that file and is on hold.
2. The 2Eh group entry type, 005460A0 and 00546730.
3. The 3.3 KB body itself, which makes these calls:
   - 7 calls to 004C43C0 and 7 to 004C5090;
   - two interface requests (004CC460);
   - one order route (0077C2A0);
   - three calls to 00803CE0 and four to 00927F30;
   - 0051E7E0 and 006502C0, the one-shot bytes +108h/+109h.

Expect input-gated orders and UI-mode writes. It is a packet of its own.

## 26. Screen 27h runs from the pump (cc9-platform2, 2026-09-25)

**The routing.** 0067BB50 is slot 20h of vtable 00CF7A38, HUD page 27h (the vtable dword sits at
00CF7A58). The recovered pump 004F8830 calls slot 20h of every screen that is both wanted (+4h)
and applied (+5h), so the image runs the 27h take at the pump's cadence. That is twice per
mission frame here, from `run_front_end_state_frame`.
- `bsp::kHudRoleScreenPumpBound` routes the pump's slot 27h to
  `GameHudHost::update_role_screen_0067bb50`, which forwards to the public
  `GameUnitsHost::role_screen_update_0067bb50`.
- `GameUnitsHost::Impl::kRoleScreenFixedStepCall` goes false in the same change, so the take runs
  once per pump and never from the fixed step. The two switches must stay opposite.
- The routine's test of the page byte +4h at 0067BBE0 was labelled "taken as set". On the pump
  path it is exact: the pump skips the screen when +4h is clear.

**Predictions, written before the pair.** One tree, `local\bin\r27_off` (pump switch off, fixed-step
call on) against `local\bin\r27_on`, USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1` on both.
- **Row that leaves:** `FrontEndScreen::update` 004f75c0, 9,165 (27h is the last slot the pump
  still records).
- **Rows added:** none. `HudRoleScreen::update` is an implemented row.
- **Take bookkeeping.** The take fires on the first call that sees a controlled kind-2 unit
  without role 0, and afterwards only when the controlled unit changes. The pump reaches page 27h
  once the in-mission interface publishes it, before the mission's first fixed step moves any unit.
  So `Session::entity_role_message_4b` and the role counts stay identical.
- **Summary lines.** All identical, the Lexington's path and death time included. The ignored
  counters are `ship avoidance search refills` and `pretranslate`.

**The pair.** `local\r27_off_usn04.log` against `local\r27_on_usn04.log`, one tree, USN04 4700/4500.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,991,192 | 2,982,034 (-9,158) |
| FrontEndScreen::update 004f75c0 | 9,165 | 7 |
| HudRoleScreen::update 0067bb50 (concrete) | none | 9,158 |

- **Summary lines.** All 163 are identical, the role bookkeeping and the Lexington's path included.
- **Prediction miss.** `FrontEndScreen::update` did not reach 0. Slot 27h accounted for 9,158 of
  its 9,165 calls. The other 7 come from a slot that is not 27h (see the end of section 28).
- **Result.** The take is unchanged and runs once per pump. **`kHudRoleScreenPumpBound` is ON and
  `kRoleScreenFixedStepCall` is false.**

## 27. Screen 2Eh's weapon groups: the bind 00549260 (cc9-platform2, 2026-09-25)

**Correction to section 25.** 0064DA40 pushes its +20h and +1Ch before calling 00549260
(0064DCB3..0064DCC4). Its +20h is the ShipCaptain mover it constructed at 0064DAC0..0064DB02, not a
unit. So 2Eh's +40h, the object 00549260 observes through the pair at +2Ch, is the camera mover.
The argument order is `(group_unit, bound)`: the frame puts `[ESP+38h]` in ESI and `[ESP+3Ch]` in EDI.

**The array.** +20h data, +24h count, +28h capacity; 18h-byte entries.
- 005460A0 builds an entry: base vtable 00CEDD88, +4h..+Ch zeroed, +10h = 1, +14h = the unit, and
  an observer registration on it. 00545600 is the matching destructor.
- 00546730 appends a copy (00545F50), growing through 00546370 to twice the capacity. The copy gets
  the derived vtable 00CEDDA0.
- 005467B0 resizes, destroying from the tail through vtable slot 0.

**The groups.** +44h selects one of six weapon groups. The table below is read from 009542B0,
00545370, 00545E50, 00545410 and 00548300:

| group | 009542B0 on the unit | player roles tested | 0077C470 mask | 00548300 |
| --- | --- | --- | --- | --- |
| 0 | yes, no unit test | none | none | 005464E0 |
| 1 | no (the switch default) | 2 | 04h | nothing |
| 2 | permission +194h, an anti-air gun (BSP_Gun_IsAntiAirKind) | 2 or 3 | 0Ch | 00547A20(5) |
| 3 | permission +198h, an artillery gun; a kind-8 unit must be surfaced | 4 | 10h | 00547A20(2) |
| 4 | permission +19Ch, a gun with +3F4h+80h = 7 | 5 | 20h | 00547A20(7) |
| 5 | permission +1A4h, a gun with +3F4h+80h = 8 or 9 | 7 | 80h | 00547A20(9) |

The permission words pass when they hold 9 (PLAYER_ANY) or the local slot game+18ECh. Group 2
also passes when 0059BBD0(2, slot) does. The guns are the kind-20h children on unit+48h, chained
through +44h.

**Selecting a group takes roles.** 00545BD0(take) calls 00545410(unit, take) on every entry.
00545410 asks 009542B0 first, then sends 0077C470(mask, take) when the group is available or the
call releases. 00548410 and 005484B0 release the old group's roles, store +44h, take the new
group's roles, then run 00548360.
- **00548410(forward, unused)**, RET 8: it steps +44h by 1 (forward) or 5 (back) modulo 6 until it
  lands on an available group among 1..5. If none is available, the result is group 0.
- **005484B0(group)** selects `group` only when it is available.
- **00548360** gives the "LVLAA" award hint on group 2, when the screen is applied and the player
  unit is a kind-18h unit whose +3D0h is kind 10h.

**00549260(group_unit, bound)**, RET 8:
1. It stores `bound` at +40h and clears +54h.
2. With no unit it empties the array and returns.
3. It notes whether the first entry changed: the array was empty, or entry 0's unit differs.
4. It rebuilds the array with the unit. A kind-1Ch unit (MCommandBuilding) adds the unit of each
   64h-byte record at +778h (count +77Ch) whose +14h is set.
5. **Changed:** +44h = 0, 00548410(forward). Then a kind-6 unit that is not kind 8 or 0Eh (a
   surface ship other than a torpedo boat) tries group 3 through 005484B0.
6. **Unchanged:** it retakes the current group's roles if the group is still available.
   Otherwise it steps forward.
7. It runs 00548360, then sets +D6h when the player unit's vtable +10h name is "LST - Rocket"
   (00CEDF90, 0Dh bytes with the terminator).

**005470C0, 0064DA40's first call.** It clears +490h on every kind-22h child of every entry unit.
Then it releases the current group's mask on each unit. It calls 009542B0 there but discards the
answer. Finally it resets the widgets (005464E0) and clears +100h and +D6h. +44h is kept.

**What the image does in USN04.** The bind's cycle asks 009542B0 for groups 1 to 5 on the
Lexington. The first group that answers yes gets its gunner roles taken by the local player
through the 4Bh role message. From then on 005484F0 builds fire message 79h each call and routes
it to the Lexington (section 28). This host cannot answer 009542B0: the permission words
+188h..+1A8h are private to the units host, and the gun list is the gunnery host's. So the
reconstruction answers "not available" as a record. **No group is selected and no gunner role is
taken.** Binding 009542B0 is a gameplay change and needs its own pair: it would move the role
table and every routine that reads it.

**Code.** `bsp::weapon_group_screen_release_005470c0` and `bsp::weapon_group_screen_bind_00549260`
in `src/mission_camera.cpp`. The helpers 00545B30, 00545B80, 00545370, 00545E50, 00545BD0, 00548360,
00548410 and 005484B0 are in `src/hud_weapon_group_screen.cpp`.

## 28. Screen 2Eh's 005484F0 (cc9-platform2, 2026-09-25)

**The body.** `__thiscall(screen 2Eh)`, plain RET at 005491F3, 005484F0..005491F4 (FUN_005484F0).
The call census matches section 25: seven 004C43C0, seven 004C5090, two 004CC460, one 0077C2A0,
three 00803CE0 and four 00927F30 calls. The fourth 00927F30 call, at 00548F69, has its answer
discarded.

**The flow.**
1. **Gates** (00548512..0054854E): +40h, +24h, entry 0's unit, [00E188D8], and 00927F30(entry 0, 0).
2. **One-shot bytes.** Each of +108h and +109h is cleared before its block runs.
   - +108h: the binocular screen [00E198C4]+4Ch is reset through 0051E7E0, or on a kind-8 unit the
     periscope +84h through 006502C0.
   - +109h: the periscope mover +2Ch gets +10Ch/+110h in +388h/+384h.
   - Only the torpedo branch sets them (00548E20, 00548E2A).
3. **Selection** (0054867A): if +44h is 0 or no longer available, step forward (00548410).
4. **Widgets**: hide all (005452F0), then the group's sight (00548300). A row is chosen: group 3
   gives 1 (the rocket gauges, from the player unit's +63Ch), or 4 on an "LST - Rocket". Group 4
   gives 2, and group 5 gives 3 (widget +98h).
5. **Keys.**
   - Action 99h pressed stores the clock [00F876A4] in +104h.
   - 9Ch, 9Dh, 9Eh and 9Fh select groups 2, 3, 4 and 5.
   - 9Bh pressed with 99h held is the lock branch. On group 3 it walks the input manager's +19CCh
     list and ends in 004CC460(26h, unit).
   - Otherwise the 9Bh axis (004D9480, 004C5070) steps the group.
   - Otherwise 99h held on group 4 is the torpedo branch. It ends in 004CC460(26h, unit) and sets
     +108h/+109h.
6. **Target** (00548839): +54h = [[00E198C4]+CCh]+4Ch. The mover's aim (0042D7E0, 00521370) is
   computed here, but only the fire message reads it.
7. **Role-held block**, only when +44h is not 0 and 00545E50 finds the group's role held on some
   entry:
   - the target marks (00548E4A's nearest gunner within unit+49Ch, three 00803CE0 relations);
   - fire message 79h (00954A10: group, aim, 99h pressed, 99h held, has-target, the target's
     +174h id);
   - `0077C2A0(unit, message, 3, 0)` for each entry whose 00545370 role test passes;
   - group 5's hold latch +100h.
8. **Tail**: the row's widgets and +D0h. With +44h at 0 only widget +C0h is shown.

**Coverage.** Every branch is reconstructed as control flow. What the host does not hold is
recorded, each under its own name: the widgets, the target, the clock, the axis, the input set
00547250, the lock and torpedo branches, the target marks' unit queries, the fire message and its
route. `HudWeaponGroupScreen::lock_branch` and `::torpedo_branch` return "not taken", and they are
reached only on input.

**Predictions, written before the pair.** One tree with every section 26 change,
`local\bin\g2e_off` (`kHudWeaponGroupScreenBound` off) against `local\bin\g2e_on`, USN04
4700/4500, `BSP_GUNNERY_RNG_STREAMS=1` on both.
- **The bind.** It runs once, with an empty array, so the release does only `reset_widgets`. The
  cycle asks four groups (2..5; group 1 needs no query), and 005484B0(3) asks one more because the
  Lexington is a kind-6 ship. Then the name test.
- **Every call** of the 9,158 passes the gates: the 27h take runs earlier in the same pump. Group 0
  is always available, and +44h = 0 makes the cycle ask groups 2..5 again, four records. Then come
  `hide_widgets`, `group_sight`, `input_axis_active`, `hud_target` and `widget_c0`.
  With no input, no key branch is reached.
- **Row that leaves:** `HudShipView::screen_2eh_005484f0`, 9,158.
- **Rows added, unimplemented:**

| row | calls |
| --- | ---: |
| HudWeaponGroupScreen::group_available 009542b0 | 36,637 (4 x 9,158 + 5) |
| HudWeaponGroupScreen::hide_widgets | 9,158 |
| HudWeaponGroupScreen::group_sight | 9,158 |
| HudWeaponGroupScreen::input_axis_active | 9,158 |
| HudWeaponGroupScreen::hud_target | 9,158 |
| HudWeaponGroupScreen::widget_c0 | 9,158 |
| HudWeaponGroupScreen::reset_widgets | 1 |
| HudWeaponGroupScreen::player_unit_name | 1 |

- **Rows added, concrete:** `HudWeaponGroupScreen::bind` 1 and `HudWeaponGroupScreen::update` 9,158.
- **Total.** The unimplemented total rises by 73,271 (82,429 added, 9,158 removed). The records
  mirror the image's own per-call calls.
- **Summary lines.** All identical: nothing here writes gameplay state while no group is selected.

## 29. The grey-arrow set and the GUI extent (cc9-platform2, 2026-09-25)

Screen 29h's pick (section 24) makes two calls per candidate that stood as records:
`UnitPickScreen::grey_arrow_set` (008DDF90) and `UnitPickScreen::gui_extent` (00AA1FE0).

**The grey-arrow set: bound.** 008DDF90 is `BSP_SzurkeNyil_ContainsUnit` on
`[game+21A4h + game+18ECh*4]`, the local slot's set (docs/LOCAL_PLAYER_UNIT_LISTS.md).
- **Its producer.** `BSP_Game_ConstructWorld` builds the eight sets (008DF900). Only the mission
  Lua fills them: 008CD440 `Objectives_Add` and 008CDD60 `Objectives_AddUnit`. Both are
  reconstructed now and fill `bsp::game::game_objective_sets()` (`src/game_hosts_lua.cpp`). The AI
  host already reads that table for 00A2C450.
- **What the test reads.** The set's `_Tree` at +18h is keyed by the unit: 008DDF00 finds the
  argument itself (008DD310). The emptiness test at 008DDF93 reads the tree's size +20h, so it counts
  units, not objectives. USN04 adds one objective, "Bombers", to slot 0 with no units, so the set
  stays empty.
- **The binding.** `kHudGreyArrowSetBound` answers from `units_in_slot(0)`. An empty set answers
  "no" (008DDF99). Otherwise the subject is the unit when it is kind 5, the vehicle root. A kind-18h
  unit's +3D0h is not exposed, so that case is a record, `UnitPickScreen::grey_arrow_squadron`.
- **Counted per call.** The image makes the call once per candidate, so the implemented row keeps
  that count.

**The GUI extent: stays a record, per call.** 00AA1FE0 `BSP_Gui_GetAspectExtent` is a pure getter.
It computes `a = 16/9` when the platform's widescreen byte `[[0109CF04]+0Dh]` is set, else `4/3`.
It returns `(a / (4/3), a / [[0109CF04]+10h])`. Both inputs are platform state that this process
substitutes: the markers host and the pick both use `(1, 1)`, the 4/3 law. The call stays a
per-candidate record, like the image's own call, until the platform publishes the two fields.
Binding it then is a projection change (0043A660 mode 1), with its own pair.

**Predictions, written before the pair.** One tree with every section 26 to 28 change,
`local\bin\ga_off` (`kHudGreyArrowSetBound` off) against `local\bin\ga_on`, USN04 4700/4500,
`BSP_GUNNERY_RNG_STREAMS=1` on both.
- **Row that changes:** `UnitPickScreen::grey_arrow_set` goes from UNIMPLEMENTED to concrete with
  the same count (261,108 in the section 26 runs).
- **Rows added:** none. The set is empty, so `grey_arrow_squadron` is never reached.
- **Total.** The unimplemented total falls by the row's count.
- **Summary lines.** All identical: the answer is "no" on both sides.

## 30. The pairs of sections 28 and 29, and the handoff (cc9-platform2, 2026-09-25)

**Section 28's pair.** `local\g2e_off_usn04.log` against `local\g2e_on_usn04.log`, one tree,
USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1`.

| row | OFF | ON (predicted) |
| --- | ---: | ---: |
| unimplemented total | 2,982,034 | 3,055,305 (+73,271; +73,271) |
| HudShipView::screen_2eh_005484f0 | 9,158 | none (none) |
| HudWeaponGroupScreen::group_available | none | 36,637 (36,637) |
| ::hide_widgets, ::group_sight, ::input_axis_active, ::hud_target, ::widget_c0 | none | 9,158 each (9,158) |
| ::reset_widgets, ::player_unit_name | none | 1 each (1) |
| ::bind / ::update (concrete) | none | 1 / 9,158 |

- **Summary lines.** All 163 are identical.
- **Result.** Every prediction holds. **`kHudWeaponGroupScreenBound` is ON.**

**Section 29's pair.** `local\ga_off_usn04.log` against `local\ga_on_usn04.log`, same settings.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 3,055,305 | 2,794,197 (-261,108; predicted -261,108) |
| UnitPickScreen::grey_arrow_set | 261,108 unimplemented | 261,108 concrete |

- **Summary lines.** All 163 are identical.
- **Result.** Every prediction holds. **`kHudGreyArrowSetBound` is ON.**

**The 7 leftover `FrontEndScreen::update` calls (section 26).** A diagnostic build logged the slot
of every call that reaches the record. All 7 are slot 1, `FE_main`, the main menu, in the frames
between the press-start exit and the mission load. They are front-end calls, not HUD ones: FE_main's
own update virtual is not bound.

**Handoff.**
- **009542B0 is the next gameplay packet.** In the image, the 0064DA40 bind takes the gunner roles
  of the first available weapon group for the local player. It needs:
  - the units host's permission words +188h..+1A8h;
  - 0059BBD0(2, slot);
  - the gunnery host's per-unit gun kinds (anti-air, artillery, +3F4h+80h = 7, 8 or 9);
  - the kind-8 depth tests.

  Once 009542B0 answers, the role take (0077C470 through `role_request`) and the per-call fire
  message 79h route (0077C2A0) become live. Both are records now. They change the role table that
  the 27h take, 0064B870 and the gunnery read, so land them only on a measured pair.
- **The HUD target** `[[00E198C4]+CCh]+4Ch` is slot 33h's +4Ch. Its producer is not identified.
- **The 2Eh widgets** need 2Eh's layout 00546A20 bound (the 25-slot row grid +5Ch..+BCh and
  +C0h..+CCh), like screens 45h and 50h.
- **The GUI extent** 00AA1FE0 waits on the platform publishing its widescreen byte and active
  aspect (section 29).

## 31. The gunner-role take: 009542B0 and the first cycle (cc9-platform2, 2026-09-25)

Packet `cc9_gunner_role_take`. Section 27 left 009542B0 as a "not available" record. This section
reads its inputs and binds it under `kHudGunnerRoleTakeBound`.

**009542B0(unit, group)**, `__thiscall`, RET 4:

| group | permission test (0059BBD0: the word is 9 or the local slot game+18ECh) | kind-8 unit | gun test on the kind-20h children of unit+48h |
| --- | --- | --- | --- |
| 0 | none | - | none, answers yes |
| 2 | +194h (role 3), else 0059BBD0(2) (role 2) | pose y above 00CE3D50 | Function 1, 5 or 6 (00728A90, anti-air) |
| 3 | +198h (role 4) | pose y above 00CE3D50 | Function 2, 3, 4 or 6 (005459B0, artillery) |
| 4 | +19Ch (role 5) | BSP_Submarine_IsShallowEnoughToEngage | Function 7 |
| 5 | +1A4h (role 7) | none | Function 8 or 9 |

- The Function is `[gun+3F4h]+80h`. The gunnery host keeps it as `GameGunRow::category`
  (`include/bsp/gunnery_tables.hpp`, written by 007327B0).
- **The binding.**
  - The permission words come from the new `GameUnitsHost::unit_role_permission`.
  - The gun test walks `GameUnitsHost::gunnery()->guns()` for the unit's rows.
  - A kind-8 unit's depth tests are not on this host. They are a record,
    `HudWeaponGroupScreen::submarine_depth`, answering no.

**0077C470(mask, take).** The new `GameUnitsHost::role_request_0077c470` delivers the 4Bh message
00780162 from the local slot at once, as the 27h take does. The routine's gate is game+5D4h above 0Ch
(or game+216Ch) with session mode 0. `BSP_Game_EnterMissionState` stores 0Dh at 004DA73C before
004C9CA0 at 004DA746, so the gate is open when 0064DA40 binds.

**The Lexington in USN04.** The mission script's `SetRoleAvailable` runs at the Lua init stage, before
the bind. The Lexington's permission words are `989988888`, so roles 0, 2 and 3 are open to any
player. The bind's cycle therefore goes as follows:
- Group 1 is never available. **Group 2 is**: +194h is 9 and the carrier has AA guns.
- +44h becomes 2, and 00545410 sends mask 0Ch: **the local player takes roles 2 and 3**. Both
  holders were 8, and both words are 9.
- The ship is kind 6, so 005484B0(3) tries group 3. +198h is 8, so it fails and the group stays 2.
- 00548360 stops at its kind-18h test: the player unit is a ship.

From then on 005484F0 finds group 2 available and its roles held on every call. So it runs the
role-held block, builds fire message 79h and routes it to the Lexington.

## 32. What fire message 79h does to the guns: a contract for the gunnery host

The route 0077C2A0 delivers the message to `BSP_Unit_HandleMessage` 0095ABE0, which sends it to
`BSP_Unit_ApplyGunAimMessage` 00959C20 (RET 4 at 0095A43E).
- **Message layout** (00954A10): +1Ch group, +20h..+28h the aim vector, +2Ch and +30h (+30h clamped),
  +34h action 99h pressed, +35h 99h held, +36h has-target, +38h the target id.
- **Dispatch.** The routine jumps on group - 1 through the table at 0095A5C0. Groups 1 and 2 both go
  to 00959C91. Group 3 goes to 00959F72 (role 4, +1BCh), 4 to 0095A1CC and 5 to 0095A441.

**The group 1/2 arm, 00959C91..00959F6D**, for each kind-20h gun on unit+48h that 00954210(group,
gun) keeps:
1. It aims: 00957BD0 and 00955830, then the fire window 007F60A0 on the resulting angles.
2. **Out of the window:** if the gun's own seat is AI-held (00521E70(gun, 0), `[gun+1ACh]` is 8 or
   an AI slot), nothing happens. Otherwise 00729F70 gives it back: `vtable[154h](0, 8)`, trigger
   `vtable[1E8h](0)`, `BSP_Gun_ClearBotFireTarget`.
3. **In the window:** if the seat is AI-held, `gun->vtable[154h](0, [unit+1B4h])` hands the gun to
   the unit's role-2 holder, the player. Kind 6 also calls 0084C500(0).
4. It turns the gun (0085ABA0 at 00959E01, 00859830).
5. **The trigger** `vtable[1E8h]` gets 1 only when +34h (99h pressed) is set and the gun is inside
   the 00D1A8A0 three-degree window (or is Function 1). Otherwise it gets 0.

**The answer to "does the idle player's held group keep firing".** No, for the guns that bear on
the camera's aim.
- Every call, each AA gun whose fire window contains the camera direction becomes a player seat:
  `[gun+1ACh]` becomes 0. It is turned to the camera's aim with its trigger at 0.
- The gun bots' side gate (docs/GUN_BOT_TICKS.md step 4, 008FFA99) runs a bot only when
  `[gun+1ACh]` is 8 or an AI slot. So those mounts stop firing at aircraft.
- Mounts that cannot bear on the camera stay with, or return to, the AI and keep firing.
- With 99h pressed, the player's mounts fire along the camera.

**Contract for `src/game_hosts_gunnery.cpp`** (not changed by this packet):
- A per-gun seat `[gun+1ACh]` that `side_enabled_00927f10` reads, default 8.
- A message entry for 79h: group, aim vector, pressed/held and the target id. For the group's guns
  it applies the arm above.
- 00954210's group filter needs reading first. It decides which mounts belong to group 2.

Until that lands, `HudWeaponGroupScreen::fire_message` and `::route_fire_message` stay records.
Every AA mount keeps its bot, so this host's Lexington fires more AA than the image does while
the player is idle.

**Other readers of roles 2 and 3.** They were searched in the host code (`current_roles_01ac`,
`unit_current_role_slot`), and none reads them yet:
- the ship AI reads roles 0 and 1;
- the generic tick reads roles 0 and 4;
- the image's dispersion seat test 00521E70(unit, 2 or 3) at 00730471/00730482
  (docs/GUN_DISPERSION.md) is not modelled. `gun_throw_magnitude_0073031d` takes the seat's
  multiplier as an input, and it assumes an AI seat.

## 33. Predictions for the gunner-role take, written before the pairs

One tree, `local\bin\grt_off` (`kHudGunnerRoleTakeBound` off) against `local\bin\grt_on`,
`BSP_GUNNERY_RNG_STREAMS=1` on both. Two forms: USN04 4700/4500, and E2 9200/9000. `N` is the number
of `HudWeaponGroupScreen::update` calls, 9,158 in the 4500 form.

- **The bind.** It makes six group queries, all concrete: the cycle's groups 2..5, the take's
  group-2 test in 00545410, and 005484B0(3). It makes one role request (mask 0Ch), also concrete.
  No `lvlaa_hint` record.
- **Every call.**
  - Group 2 is available (one query), so there is no cycle.
  - The role-held block runs. `hud_target` answers none, so the target marks show nothing and
    there is no target.
  - Then `fire_message`, one `route_fire_message` (the Lexington), then `input_in_set`,
    `row_widget` (row 0, column 0) and `input_mode`.
  - `widget_c0` is no longer reached.
- **Rows.**

| row | OFF | ON |
| --- | ---: | ---: |
| HudWeaponGroupScreen::group_available | 4N+5 unimplemented | N+6 concrete |
| HudWeaponGroupScreen::role_request | none | 1 concrete |
| HudWeaponGroupScreen::widget_c0 | N | none |
| ::fire_message, ::route_fire_message, ::input_in_set, ::row_widget, ::input_mode | none | N each |
| Session::entity_role_message_4b (concrete) | k | k+1 |

- **Unimplemented total.** It falls by exactly 5: 4N+5+N removed, 5N added.
- **Summary lines.** Two lines move, and every other line is identical in both forms:
  - `summary mission player roles`: takes 1 -> 3.
  - The Lexington's `held` string: `088888888` -> `080088888`.
- **The anti-aircraft rows are unchanged.** The AA acceptance line, the gunnery damage line
  (hit_records, deaths) and the plane death-mode line do not move. The Kate death band is
  unchanged in both forms: no host routine reads roles 2 and 3 (section 32).
- **In the image**, by contrast, the camera-bearing AA mounts go quiet while the player is idle.
  The host's AA hit records should drop once the section 32 contract lands. That is the gunnery
  host's pair, not this one.

## 34. 005484F0's target is screen 29h's pick

**Correction to sections 28 and 30.** `[00E198C4]+CCh` is not slot 33h: the manager's offsets are
not slot times four. docs/IN_MISSION_INTERFACE_MANAGER.md line 120 maps +CCh to **screen 29h**
(constructor 00525A90, vtable 00CECCF8). So `[[00E198C4]+CCh]+4Ch` is 29h's +4Ch, the unit that
00526A40 picks. It is also the unit 49h follows, and the one the interface update hands to
0068C0B0 (docs/IN_MISSION_INTERFACE_RUNTIME.md). Its producer is reconstructed:
`bsp::unit_pick_screen_update_00527260` stores it in `UnitPickScreenState::pick_4c`.
`kHudWeaponGroupTargetBound` returns it at 00548856.

**Group 2's gun filter, for section 32's contract.** 00954210 (`__cdecl(kind, gun)`, RET 8) keeps
a gun when:
- it is operational (00729F10);
- for kinds 1 and 2, its Function is 1, 5 or 6;
- kind 2 also accepts `BSP_Gun_IsTorpedoClassLauncher` 005459E0;
- kind 3 accepts 005459B0, kind 4 Function 7, and kind 5 0080F750.

**Predictions, written before the pair.** One tree with sections 31 to 33 on, `local\bin\tgt_off`
against `local\bin\tgt_on`, USN04 4700/4500, `BSP_GUNNERY_RNG_STREAMS=1`.
- Pump slot 29h runs before 46h, so on each call the target is that pump's pick. Let `P` be the
  number of 005484F0 calls whose pick is set. `P` is at least 662, the section 29 runs'
  `UnitPickScreen::owner_140` count, which counts the non-payload picks.
- With a target and group 2 held, the gunner branch (00548E4A) runs:
  - For the Lexington, `gunner_distance` and `gunner_range` answer 0 as records. The range test
    fails, so there is no nearest gunner.
  - `target_virtual_10c` answers no.
  - +54h is cleared (0054900D), so there is no fire-message target.
- **Rows:**

| row | OFF | ON |
| --- | ---: | ---: |
| HudWeaponGroupScreen::hud_target | 9,158 unimplemented | 9,158 concrete |
| ::gunner_distance, ::gunner_range, ::target_virtual_10c | none | P each |

- **Unimplemented total.** It changes by `3P - 9,158`.
- **Summary lines.** All identical: nothing on this path writes gameplay state.

## 35. The pairs of sections 33 and 34, and the handoff (cc9-platform2, 2026-09-25)

**Section 33's pair, E2 form (9200/9000).** `local\grt_off_e2.log` against `local\grt_on_e2.log`.
N = 18,158.

| row | OFF | ON (predicted) |
| --- | ---: | ---: |
| unimplemented total | 5,308,902 | 5,308,897 (-5; -5) |
| HudWeaponGroupScreen::group_available | 72,637 unimplemented (4N+5) | 18,164 concrete (N+6) |
| HudWeaponGroupScreen::role_request | none | 1 concrete |
| HudWeaponGroupScreen::widget_c0 | 18,158 | none |
| ::fire_message, ::route_fire_message, ::input_in_set, ::row_widget, ::input_mode | none | 18,158 each |
| Session::entity_role_message_4b (concrete) | 1 | 2 |

- **Summary lines.** Only `summary mission player roles` moves, takes 1 -> 3. The Lexington's line
  reads `held=088888888` -> `held=080088888` with `open=989988888`. The gunnery damage line (588 hit
  records, 37 deaths), the AA acceptance line and the plane death-mode line are identical.
- **One unrelated line moved.** The OFF run's `summary window_created` line reports
  `frames_presented=1493 exit_code=1`, against 9199 and 0 for ON. Its mission still simulated all
  9,000 frames (`mission frame 9000`, `pump_frames=9000`). This is a presentation-side anomaly of
  that run, not of the switch.
- **Result.** Every prediction holds. **`kHudGunnerRoleTakeBound` is ON.**

**Section 33's pair, USN04 4500.** Only the OFF side exists: `local\grt_off_usn04.log`, with the
predicted OFF rows (group_available 36,637, widget_c0 9,158).
- The ON run (`local\grt_on_usn04.log`) died with 0xC0000005 at mission frame 4361. The same binary
  ran the E2 form past that frame, and runs are deterministic.
- The retry (`local\grt_on2_usn04.log`) failed at startup on `sound/gui/error.fsb`
  (create_result 78). `query session` then showed session 1 disconnected: no audio endpoint, so
  every run fails until the session is reconnected.
- **Pending:** rerun `local\bin\grt_on` on USN04 4500 after a reconnect. The E2 pair covers the same
  frames.

**Section 34's pair.** `local\tgt_off_usn04.log` against `local\tgt_on_usn04.log`, USN04 4500.

| row | OFF | ON |
| --- | ---: | ---: |
| unimplemented total | 2,794,667 | 2,785,509 (-9,158) |
| HudWeaponGroupScreen::hud_target | 9,158 unimplemented | 9,158 concrete |
| ::gunner_distance, ::gunner_range, ::target_virtual_10c | none | none (predicted P >= 662) |

- **Summary lines.** All 180 are identical.
- **Prediction miss: P is 0.** The 662 came from the section 29 runs, which were built before this
  branch took main at 44e93b370. On the current tree, `UnitPickScreen::owner_140` never fires in any
  of the four runs (grt E2 OFF/ON, tgt OFF/ON). So screen 29h picks no unit, and the target stays
  empty. The formula `3P - 9,158` holds with P = 0. Why main stopped picking was not investigated.
- **Result.** **`kHudWeaponGroupTargetBound` is ON.** The answer is exact; it is empty whenever
  29h's pick is.

**Handoff.**
- **For the gunnery worker: section 32's contract.** The 79h message arm hands the camera-bearing
  group-2 mounts to the player's seat `[gun+1ACh]`, and the gun bots' side gate then stops them.
  Until that lands, this host's Lexington fires AA from every mount while the player is idle, and
  the image does not. Expect the AA hit records and the Kate death band to fall when it lands.
  00954210's filter (section 34) is the membership rule.
- **Dispersion.** The role seat test 00521E70(unit, 2 or 3) in 0073031D would now answer "player"
  for the Lexington's default-arm guns. The host's `bot_throw_multiplier` input assumes an AI seat.
- **Screen 29h.** Why it picks nothing on current main is open.

## 36. Why screen 29h stopped picking in USN04 (cc9-platform2, 2026-09-25, read only)

Packet `cc9_unit_pick_regression`. Section 34 found `UnitPickScreen::owner_140` at 0 on current main,
against 662 in the section 29 runs. Those runs were built on 88e1a3062 plus 3f79fea6b.

**The pick's inputs did not change.** Every other `UnitPickScreen::` row is identical between
`local\ga_on_usn04.log` (picks) and `local\tgt_on_usn04.log` (no picks):
- the same 9,160 updates, with `camera_basis` 3 and `project` 51;
- 261,108 grey-arrow tests, the same candidate walk;
- 251,897 `gui_extent` calls, one per projected candidate.

So the lists, the lock radius (`lock radius: scripts/datatables/globals.lua` in both) and the
projection path are the same. Only the outcome changed: no candidate projects inside the lock circle.

**What moved is the camera, because the Lexington's path moved.**
- `controlled unit frame` lines match through frame 2870 and differ from 2880.
- At 2880 both runs switch the Lexington from `moveonpath` to an AI `moveto`
  (`cmdlife 143.75s ... issue ai_command_tick/moveto`, command target "D3A Val #1.1|.-4"). The
  target differs: heading 0.7103 at 6,251 m on the picking tree, -0.2479 at 6,448 m on main.
- By frame 3110 the ship steers 19.1° instead of 37.2°, with rudder 0.853 against -0.213. The first
  pick came at frame 3112 on the picking tree. The ShipCaptain camera follows the ship, so the
  friendly units the pick finds near screen centre are no longer there.
- That Val's own trace diverges at the anti-aircraft blasts it takes: 12.7 m instead of 15.0 m, then
  more damage. That is a downstream effect of changed plane flights, not a pick input.

**Which landing changed it: f7de926f4** (packet cc9_planner_heading_writes,
`kPilotStateHeadingWritesBound`, in `src/game_hosts_units.cpp`). It is isolated by trees:

| tree | contains | owner_140 | Lexington heading at 3110 |
| --- | --- | ---: | ---: |
| section 29 runs (88e1a3062 + 3f79fea6b) | 3f79fea6b | 662 | 37.220 |
| cc9-aa-targeting `mzO`/`mzT` (muzzle pair, 9176f4b42's tree) | 7dd40497c, muzzle offsets on/off | 662 / 662 | 37.220 / 37.220 |
| 44e93b370 (sections 33 and 34 runs, role take on or off) | f7de926f4, 7dd40497c, 3f79fea6b | 0 | 19.075 |

- The only code commit that 44e93b370 has and both picking trees lack is f7de926f4.
- 6824b2817 is documentation only.
- 7dd40497c (the torpedo launch gate) and 3f79fea6b (sections 26-28) each sit in a picking tree.
- The muzzle pair shows that moving the AA shot origins alone does not move the path.
- f7de926f4's own pair already records a path change: docs/PLANNER_HEADING_WRITES.md line 137,
  "Lexington moved 6750.78 m -> 5929.06 m", judged by band.

The torpedo moveto and attack-run heading writes change the Kates' flights. That moves the AA
engagement and the Val the Lexington's moveto is aimed at, and so the ship's heading and the camera.

**Not a host bug.** Screen 29h's pick is a function of where the camera looks. On USN04 the camera
looks where the Lexington steers, and that is a knife-edge (the memory note on the Lexington applies
to its path as well as its death). Neither the pick nor the 2Eh target needs a change. What this
means for measurements: the 29h and 2Eh target rows depend on the ship's path, and they should be
judged from same-tree pairs only.

**Confirming run, pending the reconnect.** Rebuild current main with
`kPilotStateHeadingWritesBound = false` and run USN04 4700/4500 with `BSP_GUNNERY_RNG_STREAMS=1`.
Expect:
- `UnitPickScreen::owner_140` back at 662;
- the Lexington's `controlled unit frame 3110` heading at 37.220;
- a non-zero `HudWeaponGroupScreen::gunner_distance` row, since the 2Eh target now follows the pick.

The switch belongs to the planner packet, so the build is a local diagnostic only.

## 37. Screen 2Eh's layout 00546A20 (cc9-platform2, 2026-09-25)

Packet `cc9_screen_2eh_layout`. The switch `kHudWeaponGroupLayoutBound` is **committed OFF**. Its pair
waits for the session reconnect.

**The screen's vtable, 00CEDF34.** It is read from the dwords at 00CEDF30..00CEDF5C:

| slot | routine | role |
| --- | --- | --- |
| +10h | 005468B0 | register: `BSP_GuiManager_LoadPage("GUI_cross_gunstate")` into +DCh, `"GUI_cross_ship"` into +58h |
| +14h | 00546A20 | layout, this section |
| +18h | 005494C0 | enter: 00549260(entry 0's unit or none, +40h), page +58h vtable +34h(1), +D4h = 1 |
| +1Ch | 005470A0 | exit: 005464E0, page +58h vtable +34h(0), +D4h = 0 (no Ghidra function; 005470A0..005470BD, see the report) |
| +20h | 004F75C0 | update, the base `RET 4` (section 14) |

**00546A20**, `__fastcall(screen)`, plain RET at 00547076. Every lookup is
`BSP_GuiWidget_FindChildByName` (00AA7E00) with its recursive flag set.
1. +D0h = 0.
2. If the `GUI_cross_ship` page +58h exists, for each row 0..4 it finds the group on the page:
   `ship_AA_Group`, `ship_art_Group`, `ship_torpedo_Group`, `ship_DC_Group`, `ship_rocket_Group`.
   - Row 1 also finds `cross_botton_Icon`, `cross_left_Icon` and `cross_right_Icon` under its group,
     into +C4h, +C8h and +CCh.
   - Each column 0..4 is cleared, then found under the group (jump table 0054708C):

| column | name | rows that skip it |
| --- | --- | --- |
| 0 | `cross__Icon` | none |
| 1 | `cross_F_Icon` | 3 |
| 2 | `cross_H_Icon` | none |
| 3 | `cross_HF_Icon` | 2, 3 |
| 4 | `cross_L_Icon` | 1, 2, 3 |

   - The cell is stored at +5Ch + row*14h + column*4h.
   - When the row's F cell exists, `cross_F2_Icon` is found under it and placed:
     - its local bounds are half the F cell's size (00AA6740, 00AA7DC0);
     - rows 1 and 2 then get a resolved position (00AA6750, 00AA8240) whose y is 00CE3800 or
       00CEDE2C;
     - the F2 handle is not stored.
3. On the `GUI_cross_gunstate` page +DCh, with no null test, it finds three widgets:
   - `CrosshairDisable_Icon` into +C0h;
   - `GunState_Icon` into +FCh, which is then hidden (vtable +34h(0));
   - `circle_Section` into +D8h.

This installation's `interface/gui_cross_ship.lua` defines exactly these names. The occurrence counts
match the skip rules: `cross__Icon` 5, `cross_F_Icon` 4, `cross_H_Icon` 5, `cross_HF_Icon` 3,
`cross_L_Icon` 2 and `cross_F2_Icon` 4. `gui_cross_gunstate.lua` defines the three gunstate names.

**+D4h, the show argument.** Every row-widget call in 005484F0 passes the byte +D4h. Its writers:
- the enter 005494C0 (1) and the exit 005470A0 (0);
- 00545360 (1, and +1Ch = 0) and 00545AC0 (0, +1Ch = 1). Both are called from screen 45h's
  0064DD30. 45h calls 00545360 at 0064F65A on every update outside repair mode: 9,158 calls in
  USN04 4500.

**The binding** (`bsp::weapon_group_layout_00546a20`, `src/hud_weapon_group_screen.cpp`; host
`WeaponGroupLayoutBinding`, `src/game_hosts_hud.cpp`):
- Pages come from `GameMenuHost::in_game_page(0x2E, ...)`. Lookups use
  `bsp::find_descendant_by_name`, and visibility uses `set_widget_visible`. The GUI resource
  code behind them is the front end's (Codex's), used as a contract.
- The layout runs once, before the first widget use. The image runs it at registration; the
  difference is labelled.
- 005484F0's `hide_widgets` (005452F0), `row_widget_present`, `show_row_widget` (argument +D4h)
  and `widget_c0` act on the found widgets.
- 45h's 00545360 sets +D4h.
- The F2 placement is a record, `HudWeaponGroupScreen::place_f2`: it is GUI geometry.
- 00548300's group sight, 005464E0, the group-3 rocket widgets and the group-5 widget +98h stay
  records.
- The 2Eh enter and exit are the generic `FrontEndScreen::enter`/`exit` records. Binding the enter
  would re-run 00549260, whose role retake 4Bh message would be refused: a summary-line change.
  So it is left for its own packet.

**Predictions, written before the pair.** One tree (main bd3d6996b plus this),
`local\bin\lay_off` against `local\bin\lay_on`, `BSP_GUNNERY_RNG_STREAMS=1`, USN04 4700/4500 and E2
9200/9000. N is the number of 005484F0 calls: 9,158 and 18,158. On this tree group 2 is held and the
target is empty (section 36), so every call takes the role-held path with row 0.

| row | OFF | ON |
| --- | ---: | ---: |
| HudWeaponGroupScreen::hide_widgets | N unimplemented | N concrete |
| HudWeaponGroupScreen::row_widget | N unimplemented | N concrete |
| HudWeaponGroupScreen::show_row_widget | none | N concrete (row 0, column 0, `ship_AA_Group/cross__Icon`) |
| HudShipScreen::other_screen_00545360 | N unimplemented | N concrete |
| HudWeaponGroupScreen::layout | none | 1 concrete |
| HudWeaponGroupScreen::place_f2 | none | 4 unimplemented (rows 0, 1, 2, 4) |
| HudWeaponGroupScreen::input_mode | N | N (unchanged) |

- **Unimplemented total.** It falls by 3N - 4: 27,470 in USN04 and 54,470 in E2.
- **Summary lines.** All identical. No summary line counts image-widget visibility, and nothing here
  writes gameplay state.
- **If a page did not load**, the cells stay empty: `show_row_widget` and `place_f2` do not appear,
  and the total falls by 3N instead. That would be a front-end page-loading finding, not a change
  to this switch.
