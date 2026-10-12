# Camera-mover constructions and the events that trigger them (packet cc13-camevents1)

Worker cc13-camevents1, 2026-10-12, base origin/main 5a4509b2c. This is a read-only analysis for cc9
(item h). No C++ was written. Ten Ghidra renames were applied with the locked tool; the prior values
are in the worktree's `local/ghidra-annotations-20261012T073820Z.json`.

Status labels: everything here is **exported and listing-read**. Nothing is reconstructed or
game-validated. The call rows are in `reports/cc13_camevents1_calls.json`; verify_report_calls
reports 67 rows checked and 0 failed.

## 1. Short answer

- **Every mover construction makes three stream-0 draws.** The base `00432750` draws U(0, 2pi)
  three times through `00BD2F10`. Each draw sets `XOR ECX,ECX` first (at 00432919, 0043294E and
  0043296B), and the calls are at 00432938, 00432955 and 00432972. That is stream 0.
- **Stream 0 is not the gunnery stream.** `docs/RANDOM_STREAMS.md` section 1 censuses ECX at all
  625 call sites. Gunnery and most bot tasks pass ECX = 1. The 214 stream-0 sites sit in 54
  functions. Most of them are presentation code:
  - 0049A760, 0049B1F0, 0049C000 and 004A0D20 hold 63 sites;
  - post effects, waterdrops, lightning and clouds;
  - particles;
  - the camera wander `0042C1B0` and this constructor.

  A few stream-0 consumers are gameplay code (list in `local/ce1_stream0.txt`):
  - `BSP_BotTask_ConstructBase` 0099C6F0;
  - `BSP_BotStateLevelBombMoveTo_Construct` 009B6C10;
  - `BSP_ShipAi_NavigationBlockConstruct` 009E4330;
  - `BSP_LandFort_FrameUpdate` 00745BE0;
  - `BSP_Plane_OnDestroyed` 007BCAA0;
  - `BSP_Plane_StartNamedEffect` 007BBFA0;
  - `BSP_ProjectileTickableEntity_Construct` 006E7B00.

  So a camera construction shifts those consumers, and the gunnery stream is untouched.
- **Movies reseed stream 0.** `005CD1A0`, the first MovCamNew engage of the mission, sets stream 0 to
  54321. The new movie camera's first step with dt > 0, `00798C80`, sets it again, once per camera
  instance (`docs/RANDOM_STREAMS.md`). Draws taken before the last movie camera's first step are
  therefore erased. The offset that survives is 3 times the number of constructions after that point.
- **Every in-mission constructor reuses its mover** (section 3), except the movie-keyframe modifier,
  which builds one mover per modifier. A creator builds a mover only when its cached pointer is null.
  The pointer goes null in two ways:
  - **Never built.** This is the first use of that HUD view in the mission.
  - **Destroyed.** Another view installed its own mover through `004BC410`, which kills the outgoing
    one (00926D90). The view's observer then clears the pointer.
  So a construction happens when the player **changes camera family**. Re-selecting within the same
  family costs nothing.

## 2. Constructor census

**Method.** An E8/E9 rel32 and absolute-dword scan of the PE (`local/ce1_scan.py`) covered the base,
every subclass constructor, every creator and the class vtables.
- **Positive control.** The scan finds the known `0079CF8B -> 0079CBD0` and `0068B163 -> 0067B4D0`
  sites.
- **Cross-check.** `ghidra callers 00432750` lists the same 14 functions as the 14 rel32 sites.
- **Indirect calls.** No constructor has an absolute-dword (vtable or table) reference.
- **Inlined constructors.** The vtable scan shows that the ten class vtables are written only by
  their constructors, by the inlined copies listed below, and by destructors.

| constructor (bytes) | creator, cached at | creator's caller | trigger |
|---|---|---|---|
| `0064B650` ShipCaptain (404h) | `0064DA40` BSP_HudShipView_BindUnitCamera, view `+20h` | 0068ACA0 25h arm (0068B140) | INTF_CAPTAIN: a surface ship becomes the controlled unit (20h classifier, type 6) |
| `0064B650` ShipCaptain (404h) | `0067C1A0` **BSP_HudTorpedoBoatView_BindUnitCamera**, `+2Ch` | 25h arm's sibling 27h (0068B0C1) | INTF_TBOATHELMSMAN (type 0Eh) |
| `00650830` submarine (410h), through the inlined copy in `00651370` | `00651370` BSP_HudUnitView_EnsureSubmarineCamera, `+2Ch` | `00651760` <- 28h arm (0068B102) | INTF_SUBMARINE (type 8) |
| `00519290` ship-camera base (3F4h) | `005213D0` **BSP_HudCommandBuildingView_BindUnitCamera**, `+20h` | 2Fh arm (0068B1E0) | INTF_COMMANDBUILDING (type 1Ch) |
| bomber view, inlined `00519290` + vtable 00CEC5F0 (3F4h) | `005198F0` BSP_BomberView_EnsureCamera, `+28h` | `00519B00` **BSP_HudBomberView_BindUnit** <- 23h arm (0068AFF5) | INTF_PLANEBOMBER |
| `00605130` plane chase (3FCh) | `00605E50` **BSP_PlaneHudScreen_EnsureChaseCamera**, `+94h` | `00608280`, `00608B60`, `00608C30`, `00608D50` | INTF_PLANE (22h, 0068B042), the plane screen's enter (slot 00CF43C4), the view toggle |
| `006065D0` plane cockpit (518h) | `00607F60` **BSP_PlaneHudScreen_EnsureCockpitCamera**, `+98h` | the same four | cockpit view selected (`+FCh` = 1) |
| plane spawn, inlined `00432750` + vtable 00CF4930 (3CCh) | `0060CF90` BSP_HudPlaneSpawnScreen_AttachPlane, `+1Ch` | 24h arm (0068B089) | INTF_PLANESPAWN (a plane that is not yet airborne, per 007BB9A0) |
| `00435280` bomb view (3F8h) / `00435120` torpedo view (3CCh) | `0067B4D0` **BSP_HudBombView_EnsureCamera**, `+4Ch` / `+50h` | 26h arm (0068B163) | INTF_BOMBVIEW: torpedo view when the target is null or kind 2Bh, else bomb view |
| `0079D020` new movie camera (570h) | `005CC170` BSP_HudMovieScreen_EnsureCamera, screen 37h `+1Ch` | `005CD1A0` <- 37h enter 005CD9C0, 005CD240 (MovCamNew rows 008B7850/008B79F0/008B7BA0, session 0076D760), 0068A1F0 (input 8Ah) | a MovCamNew scripted movie (2Ch) |
| `0079F480` MovCam (540h) | `005CC000` **BSP_HudMovieScreen_EnsureMovCam**, `+30h` | `005CD160` (the luaMW_MovCam bindings 008B2CC0..008BFA70, session MovCam messages, 0068A160 on input 89h), `005CD0F0`, `005CCA80` (slot 00CF1980) | an old-style MovCam movie (2Bh) |
| free camera, inlined `00432750` (900 bytes) | `0053DD90` BSP_HudFreeCameraScreen_Enter, once | screen enter, vtable 00CEDCA8 | INTF_FREECAMERA (29h) |
| idle camera, inlined `00432750` | `0054F900` BSP_HudIdleCameraScreen_Enter, once | screen enter, vtable 00CEE408 | INTF_IDLECAMERA (2Ah): a null-payload 20h with `game+1FE4h` set, which is multiplayer |
| 00650830 / 0064B650 / 006065D0 / 00605130 / 0078E830 / 005176F0 | `0079CBD0` BSP_GameCameraModifier_EnsureCamera, modifier `+8h` | `0079CF80` Begin (vtable 00D04CC0 slot 18h), run by keyframe begin 00791020 | a movie keyframe with a `gamecamera` / `fpscamera` modifier, chosen by the parent entity's kind |

**Never or unreachably called:**
- **`00519490`** (the standalone bomber-view constructor) and **`0060C930`**
  (BSP_PlaneSpawnCamera_Construct) have no rel32 or absolute reference. Their creators inline the
  same stores.
- **`005178F0`** inlines the airfield camera (vtable 00CEC1F8 at 0051799E) and installs it at
  00517A80. It also has no reference. INTF_AIRFIELD (2Eh) jumps straight to the 0068ACA0 tail.
- So the **airfield (`005176F0`) and shipyard (`0078E830`) cameras are built only by
  EnsureCamera**, for a movie modifier whose parent is an airfield or a shipyard.

**The modifier camera is never installed.** It is a private mover. EnsureCamera gives it the
Operator node (`0042A920`) but never calls `004BC410`, so it neither kills nor replaces the game's
mover.

**No other in-mission interface constructs a mover:**
- 2Dh, the engine movie;
- 34h, limbo;
- 21h, the map;
- 2Eh, 30h, 31h, 32h and 35h.

The engine-movie screen 38h's enter, `005CDC50`, does call `004BC410(0)`. That **kills** the current
mover without building one, so the next unit interface rebuilds its mover.

## 3. Reuse and re-construction

All the creators share one shape, read from the listing for 0064DA40, 00605E50, 00607F60,
005198F0, 0060CF90, 0067B4D0, 0067C1A0, 005213D0, 00651370, 005CC000 and 005CC170.

```
if (view->mover == null) { new; ctor (3 stream-0 draws); 00923870 place; 00694A60 observe }
retarget / seed (only when the adopted countdown +3E8h is 0)
004BC410(view->mover)      ; no-op when it is already game+1ED4h
```

**Two ways the pointer goes null:**
1. **Kill and observe.** `004BC410` (BSP_Game_SetCameraMover) first calls the new mover's slot 124h
   with the old one (the adopt-from hand-over), then `00926D90(old, 2)`. The kill is deferred: the
   on-killed drain `009273A0` delivers the destroy notice to each registered observer, and the
   view's observer clears its pointer. Read for two views:
   - `0064A4C0` HudShipViewScreen_OnObservedDestroyed clears screen `+1Ch` (unit) or `+20h`
     (ShipCaptain).
   - `00604E70` **BSP_PlaneHudScreen_OnObservedDestroyed** clears `+B8h` (plane), `+94h` (chase) or
     `+98h` (cockpit), at 00604E7C, 00604E91 and 00604EA6.

   The other views register the same 00694A60 pair, so the same rule is **inferred** for them.
2. **The bomber view.** `005198F0` also drops `+28h` when the mover's `+60h` or `+5Dh` byte is set.

**When a mover is reused:**
- **Same view, mover alive.** This covers re-selecting another ship of the same family: ship to ship
  under 25h, plane to plane under 22h, sub to sub under 28h. The creator skips construction and only
  retargets. 0064DA40 re-seeds the orbit only when the target changes and `+3E8h` is 0.
- **The controlled unit dies.** The ship view's observer clears the unit (`+1Ch`), not the mover.
  The mover survives, so the next ship selected under 25h reuses it.
- **Adopt-from (slot 124h)** never avoids a construction. It copies orbit state into a mover that
  was just built (or reused), and only for the same target.
- **The modifier camera.** EnsureCamera builds only while modifier `+8h` is null. A keyframe that
  begins again reuses its camera, and each new modifier builds one.

**When a mover is rebuilt:**
- **Any change of camera family.** Ship, torpedo boat, sub, command building, bomber, plane chase,
  plane cockpit, plane spawn, bomb view, torpedo view, movie, MovCam and free camera are separate
  families. Installing one kills the previous family's mover. Returning to a family therefore costs
  a new construction, unless its mover was never replaced.

## 4. Frequency per event

| event | constructions (x3 stream-0 draws) | reasoning |
|---|---|---|
| mission start, ship as first controlled unit | 1 (ShipCaptain) | scene load step 19 -> 004C0890 -> 004CC460(20h) -> 0068ACA0 20h classifier -> 25h -> 0064DA40 with view `+20h` null |
| mission start, other first unit | 1 of its family | plane: 22h -> chase; sub: 28h. USN01 first applies 2Eh (airfield), which builds nothing, and builds at its first 25h |
| unit switch, same family, mover alive | 0 | retarget only |
| unit switch, different family | 1 | the new view builds; the old family's mover is killed |
| controlled ship destroyed | 0 | unless the next selection changes family |
| plane chase / cockpit toggle | 1 per toggle | input DFh (`IC_ZOOM_ROLE_SWITCH` = 223 in this installation's Inputs.lua, if 004C43C0 indexes IC ids) flips `+FCh` at 0060AD9F -> 00608280 |
| bomb release in cockpit | 1, plus 1 when the view returns | 006082D0 forces chase (`+FCh` = 0, 0060840E); it returns to cockpit at 006086B6 when `+104h` is set |
| bomb or torpedo view | 1 in, 1 out | 26h builds; returning to 22h rebuilds the chase or cockpit mover |
| sub periscope, dive, surface | 0 | `0042CB70` SelectMode and `0042F6C0` Setup change modes on the same mover |
| MovCamNew movie (2Ch, or later movies under the current interface) | 1 movie camera, then 1 ShipCaptain when SetSelectedUnit returns | the movie camera's draws come after the 005CD1A0 reseed and are erased by its own first-step reseed at 00798C80; the ShipCaptain's 3 draws after the movie survive |
| old MovCam movie (2Bh) | 1, then 1 for the unit's family on return | `005CC000` builds when `+30h` is null; no stream-0 reseed on this path |
| engine movie (2Dh) | 0 in, 1 on return | screen 38h kills the mover |
| movie keyframe with `gamecamera` / `fpscamera` | 1 per modifier | private mover. In this installation the key occurs in `commandhelpers.lua` luaCamOnTargetNew (lines 7459, 7480) and 1-5 times in each COTP, PRCP, ESMP and LOMP mission script; no base `missions\usn\*.lua` file uses it |
| plane launch, landing, AI kills, death of other units | 0 | no creator is on those paths |

**A typical single-player ship mission with an intro movie** (the USN04 shape, from the interface
sequence 20h, 25h, 2Ch, 25h in `docs/HUD_PICK_SEGMENT_QUERY.md` section 8.7):
1. ShipCaptain at mission start: 3 draws, erased at the movie.
2. Movie camera: 3 draws, erased by its first step.
3. ShipCaptain at `SetSelectedUnit`: **3 surviving draws**.

After that, add 3 for every change of camera family the player makes. A ship-only player usually
makes none.

Every additional MovCamNew movie that ends in `SetSelectedUnit` resets stream 0 and then adds 3.
A player who flies planes adds 3 per chase/cockpit toggle and 6 per bomb-view round trip. A typical
carrier mission therefore adds tens of constructions.

## 5. The harness lines (src/game_hosts_mission_frame.cpp, read only)

| harness line | image call it emulates | does that image path construct a camera? |
|---|---|---|
| `<frame> select <unit>` (3150..3179) | `00647300` body: 00645060, 00645600, then 00647040's `004CC460(20h, unit)` push; the next service runs 0068ACA0 | **Yes, at the next interface service, but only when the selected unit's family has no live mover.** Static reach from 00647300 / 00645600 / 004C0890 (direct calls, depth 6) finds no constructor, because the construction is the deferred 20h -> 25h/27h/28h/22h/... arm. A ship selected while the ShipCaptain is alive: 0 constructions. A ship selected after a movie, or after a sub, plane or bomb view: 1 |
| `<frame> takehelm <unit> ...` (3474..3497) | the ShipCaptain screen's helm, 0064B870: SetRoleAvailable 0077F360 -> 00927D20, transfer 0077C470(unit, 2, 1), helm message 00816A40 | **No.** Direct-call reach at depth 6 from 0064B870, 0077C470, 00816A40, 0077F360 and 00927D20 hits no constructor and no 004BC410. The positive control from 0068ACA0 at depth 4 hits 10. The image's helm runs inside a 25h screen whose mover already exists |

**What the host does today (game_hosts_hud.cpp, read only):**
- **`bind_mission_camera_0064da40`** records three `MissionCamera::phase_draw` 00BD2F10 rows when
  `camera_bound` is false (1314..1319).
- **`camera_bound` is cleared only by `build_movie_camera_005cc170`** (1870) and by
  `detach_world_2k` (1448). So the host reproduces the movie-then-ShipCaptain rebuild. It has no other
  family, so a `select` of a non-ship unit records no draws.
- **`build_movie_camera_005cc170` records no phase draws for 0079D020's own base constructor.** In
  the image those three draws come after 005CD1A0's reseed and before 00798C80's, so they are
  erased. Omitting them is harmless only while the first-step reseed is modelled.

## 6. Names applied (ledger and Ghidra, locked tool)

- 00605E50 BSP_PlaneHudScreen_EnsureChaseCamera
- 00607F60 BSP_PlaneHudScreen_EnsureCockpitCamera
- 00608280 BSP_PlaneHudScreen_ApplyViewMode
- 00608D50 BSP_PlaneHudScreen_BindPlane (RET 4)
- 00604E70 BSP_PlaneHudScreen_OnObservedDestroyed
- 00519B00 BSP_HudBomberView_BindUnit (RET 4)
- 0067C1A0 BSP_HudTorpedoBoatView_BindUnitCamera (RET 4)
- 005213D0 BSP_HudCommandBuildingView_BindUnitCamera (RET 4)
- 0067B4D0 BSP_HudBombView_EnsureCamera (RET 8)
- 005CC000 BSP_HudMovieScreen_EnsureMovCam

All are hypotheses, not recovered symbols.

## 7. Uncertainty and open items

- **The observer clear is read only for the ship view and the plane screen.** For the bomb view,
  torpedo boat, sub, command building, plane spawn and movie screens it is inferred from the same
  00694A60 registration. A view whose observer did not clear its pointer would reuse a dead mover;
  nothing seen suggests that.
- **Input DFh.** Mapping DFh to `IC_ZOOM_ROLE_SWITCH` assumes 004C43C0's index is the Inputs.lua
  IC id. The name does not obviously mean "cockpit toggle". What is read is that it flips `+FCh`.
- **The plane screen's slot 00CF43C4 is taken to be its `+18h` enter.** That assumes the vtable
  starts at 00CF43AC, with 00CF43A8 belonging to the preceding table.
- **Ordering inside one frame.** The killed mover's observer clear waits for the on-killed drain
  009273A0. Two family switches inside one frame, before the drain, could reuse a dying mover. This
  was not checked.
- **The stream-0 census is cc9's** (`local/ecx_census_00bd2f10.txt` in the cc9-aa-targeting tree,
  read through `local/ce1_stream0.py`). 84 sites with undecoded ECX may include more stream-0
  consumers.
- **Not read:** 0053DD90 and 0054F900 beyond their ledger records ("creates once"), and what pushes
  26h, 29h and 23h.

## 8. Handoff

The packet is complete. Leases are released. Scripts are in the worktree's `local\ce1_*`:
- `ce1_scan` (rel32 + absolute-dword census);
- `ce1_reach` (direct-call reach to constructors);
- `ce1_stream0` (stream-0 consumers);
- `ce1_dw` (dword dump).
