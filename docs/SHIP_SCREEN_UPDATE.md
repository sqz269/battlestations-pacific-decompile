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
