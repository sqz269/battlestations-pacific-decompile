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
