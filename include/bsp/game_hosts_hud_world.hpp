#pragma once
// bsp_game.exe milestone 2k: the two in-mission HUD screens that show the world,
// as process bindings.
//
// Addresses: 005c0f20 (BSP_HudMinimapScreen_Update, slot 35h) with 005bd420,
// 004b4b00, 00432650, 0087d7b0, 00414db0, 00927880, 00427eb0, 00427e30,
// 005be110 / 00bf681b / 005bd590 / 005c0700 / 00694a60 (the per-unit icon
// entry), 00b6db70, 00bf701a, 00bf7030 and 00aa7dc0; 006435d0
// (BSP_InGameHudMarkersScreen_Update, slot 4Dh) with 00aa1fe0, 00640620,
// 0063bcd0, 006430c0, 0080e490, 0063abd0, 004323d0, 008ddf90, 0043f080,
// 00642040, 00639990, 0063a6c0, 00638e50, 0043a660, 00b70490, 00b62d10,
// 0063d1e0, 0043a290, 00b6fde0, 00640d70 and 00643360; and 004cc460 with
// 0068aca0 for the interface request that raises both screens.
//
// Nothing in this file is a reconstruction of native code. Every method is one
// call site of bsp::HudMinimapHost, bsp::HudMarkersUpdateHost or
// bsp::HudMarkersRuntimeHost, satisfied either by a reconstruction already on
// main (src/hud_minimap.cpp, src/hud_markers_runtime.cpp, src/hud_updates.cpp)
// or by the explicit unimplemented policy in GameHostLog.
//
// Three things are the executable's own and are labelled as such wherever they
// appear:
//
//   004b4b00  the camera's unit. This process builds no in-mission camera
//             (game+19FCh), so both screens are handed the controlled unit, and
//             the minimap's renderer basis is that unit's own forward row rather
//             than [renderer+110h] / [renderer+118h].
//   the icon  the native per-unit minimap icon entry (005bd590, 005c0700,
//             00694a60) and the per-marker widget writer 0063d1e0 have no
//             reconstruction, so the executable clones the page's own authored
//             template through the sprite bridge and drives the clone. The
//             bridge was never a reconstruction; see docs/GAME_EXECUTABLE.md.
//   the camera for the markers. 0043a660 needs a view-projection matrix and this
//             process has none, so the projection host is a **fixed top-down
//             orthographic camera fitted to the mission's own unit bounds**. It
//             is a stand-in, and every native camera call site stays a record.
//
// Evidence: docs/HUD_MINIMAP.md, docs/HUD_MARKERS_RUNTIME.md,
// docs/HUD_CENTRAL_UPDATES.md, docs/IN_GAME_INTERFACE_SCREEN_SETS.md,
// docs/GAME_EXECUTABLE.md.

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/ship_ai_seg67_switches_cc13.hpp"  // kCruiseLatchedPlayerArmCc13Bound

namespace bsp::game {

// Packet cc9_hud_presentation_top (docs/HUD_PRESENTATION_TOP.md). ON binds three
// of the eight largest HUD rows: 004C43C0's edge test in the in-game interface
// update, 00B62D10 in the markers' clip-space projection (on the stand-in
// camera matrix), and 00432650's two global-config reads in the minimap. OFF
// keeps the records.
inline constexpr bool kHudPresentationTopBound = true;

// Packet cc9_mission_camera (docs/MISSION_CAMERA.md). ON creates the ShipCaptain
// camera mover when the 25h arm hands its ship view the unit (0064DA40), ticks
// it (00432ED0) and publishes its pose into the Operator node, which the
// markers' 00B70490 and the minimap's 00B6DB70 then read in place of the
// top-down stand-in. OFF keeps the stand-in and the two records.
inline constexpr bool kMissionCameraBound = true;

// Packet cc9_mission_camera, part 3 (docs/MISSION_CAMERA.md section 10). ON
// holds the marker widgets as the markers screen's pool A (entries taken in
// order through the frame cursor +7Ch) and runs 00640620 over the four pools:
// entries past a cursor are hidden, pool D's tail is released, the A/B/C
// cursors are zeroed. OFF keeps the per-unit widget map and the two records.
inline constexpr bool kHudMarkerPoolsBound = true;

// Packet cc9_mission_camera, part 3 (docs/MISSION_CAMERA.md section 9). ON,
// together with kMissionCameraBound, sets the Operator fov each camera tick the
// way screen 45h's pipe-sight block does through 004DC940 (Globals.FOVs.Ship x
// [00F889B4] x the zoom scale). OFF leaves the constructor's 40 degrees, so the
// camera pair and the FOV pair can be measured separately.
inline constexpr bool kMissionFovBound = true;

// Packet cc9_ship_screen_update (docs/SHIP_SCREEN_UPDATE.md). ON runs screen
// 45h's update 0064DD30 through bsp::ship_screen_update_0064dd30 in place of
// the FrontEndScreen::update record: part 1 binds the relation icon
// (0064A960), the flash icons (0064ABD0) and the throttle stick (+44h,
// 0064A9F0); the other blocks are records. OFF keeps the one record.
inline constexpr bool kHudShipScreenUpdateBound = true;

// Part 2 of the same packet (docs/SHIP_SCREEN_UPDATE.md section 10): the
// control flow from 0064E415 to 0064F665, the turn-to-camera and repair-menu
// gates over the menu host's action records. Their bodies are records never
// reached without input. OFF keeps the part-1 tail record.
inline constexpr bool kHudShipScreenControlsBound = true;

// Packet cc9_ship_screen_parts34, part 3 (docs/SHIP_SCREEN_UPDATE.md section
// 12): the damage panel 0064F665..0064FD24, Icon_3/Icon_5, the Hl_1..4 fade
// and the four circles. The repair task, the settings failure descriptors and
// the device list are records that answer "no data". OFF keeps the part-2
// tail record at 0064F665.
inline constexpr bool kHudShipScreenDamageBound = true;

// Part 4 of the same packet (section 13): 0064FD24..006500C1, ship_dir_Icon
// through 0064AAD0, the +128h/+12Ch spring and the digit gauges 0043B370.
// The digit texture rolls (0043ABA0) and the clock-gated step are records.
// OFF keeps one record at 0064FD24.
// Packet cc9_screen_26h_2eh (docs/SHIP_SCREEN_UPDATE.md section 14). Slots
// 26h, 2Eh and 3Eh keep FrontEndScreen's base update in their vtables (+20h
// holds 004F75C0 at 00CEC9D0, 00CEDF54 and 00CF4564), a bare RET 4. ON logs
// the pump's call on those slots as the done base update; OFF keeps them under
// the FrontEndScreen::update record.
inline constexpr bool kHudBaseUpdateScreensBound = true;

// Packet cc9_screen_50h (docs/SHIP_SCREEN_UPDATE.md section 16). ON runs
// screen 50h's update 00683020 (the four warnings: stall, oxygen, shallow
// water, exit zone) through bsp::warning_screen_update_00683020 in place of
// the FrontEndScreen::update record. The alert inputs the host lacks are
// records that answer "no alert". OFF keeps the record.
inline constexpr bool kHudWarningScreenBound = true;

// Packet cc9_screen_49h (docs/SHIP_SCREEN_UPDATE.md section 17). ON runs
// screen 49h's update 0067BF00, which picks the unit the screen follows
// (+8h), in place of the FrontEndScreen::update record. Screen 29h's unit
// and the controlled unit's target are records answering none.
inline constexpr bool kHudFollowScreenBound = true;

// Packet cc9_screen_46h, part 1 (docs/SHIP_SCREEN_UPDATE.md section 18). ON
// runs screen 46h's update 0064D610 in place of the FrontEndScreen::update
// record: the gates and the order input, with 0064A400, 0064B870 and screen
// 2Eh's 005484F0 as records. OFF keeps the one record.
inline constexpr bool kHudShipViewScreenBound = true;

// Part 2 of the same packet (section 19): 0064B870, the integrated throttle
// and rudder controls. The input manager fields and the player-role test are
// records; the role transfer and the order send are records never reached
// without input. OFF keeps the part-1 record.
inline constexpr bool kHudShipViewControlsBound = true;

// Section 21: 0064B870's 00927F30 role test reads the units host's role table
// (unit+1ACh, the table the 27h role take and the BSP_PLAYER_HELM option
// write) in place of the fixed "role 0 held, role 1 not" substitution. OFF
// keeps the substitution and its record.
inline constexpr bool kHudShipViewRoleTableBound = true;

// Part 3 of packet cc9_screen_46h (section 22): 0064A400, the binoculars
// screen's 0051EF00 and 0051F050 on the ShipCaptain mover. The input axes and
// GlobalConfig+4 are records; with no input the mover's yaw and pitch are
// rewritten with a zero step. OFF keeps the part-1 record.
inline constexpr bool kHudShipViewInputBound = true;

inline constexpr bool kHudShipScreenGaugesBound = true;

// Packet cc9_screen_44h (docs/SHIP_SCREEN_UPDATE.md section 23). ON runs
// screen 44h's update 00649860, the HUD root, through
// bsp::hud_root_screen_update in place of the FrontEndScreen::update record:
// the pause gate, the every-other-call power-up clone pass, the award ticker,
// the pending-unit commit, ClosedUnitHUD_Group's show, the closed-HUD toggle
// and the group-change tail. The power-up manager, the award queue, the pause
// bytes, the selection tuple (00644CC0), the selection poll (00644DB0) and the
// unit rows (00648C20) are records. OFF keeps the record.
inline constexpr bool kHudRootScreenBound = true;

// Packet cc9_screen_29h (docs/SHIP_SCREEN_UPDATE.md section 24). ON runs
// screen 29h's update 00527260 and its pick 00526A40 in place of the
// FrontEndScreen::update record, and answers screen 49h's read of 29h's +4Ch
// from the pick. The spatial query, the unit lists' producers, the squadron
// members, the grey-arrow set and the input device bytes are records; the
// lock branches (orders, role moves, take control) are records never reached
// without input. OFF keeps both records.
inline constexpr bool kHudUnitPickScreenBound = true;

// Packet cc9_screen_2eh_group (docs/SHIP_SCREEN_UPDATE.md section 26). ON
// routes the pump's slot 27h (vtable 00CF7A38 slot 20h, 0067BB50, the role-0
// take on the controlled unit) to GameUnitsHost::role_screen_update_0067bb50
// in place of the FrontEndScreen::update record, at the pump's cadence. The
// units host's GameUnitsHost::Impl::kRoleScreenFixedStepCall must be the
// opposite of this switch, or the take runs twice or never.
inline constexpr bool kHudRoleScreenPumpBound = true;

// Packet cc9_screen_2eh_group (docs/SHIP_SCREEN_UPDATE.md sections 27 and
// 28). ON runs screen 2Eh's release 005470C0 and bind 00549260 inside
// 0064DA40, and its body 005484F0 from screen 46h's update in place of the
// HudShipView::screen_2eh_005484f0 record. The group test 009542B0 answers
// "not available" as a record, so no group is selected and no gunner role is
// taken; the widgets (2Eh's layout 00546A20 is not run), the HUD target, the
// input axes and every input-gated branch are records. OFF keeps the record.
inline constexpr bool kHudWeaponGroupScreenBound = true;

// Packet cc9_screen_2eh_group, part 3 (docs/SHIP_SCREEN_UPDATE.md section
// 29). ON answers screen 29h's 008DDF90 (BSP_SzurkeNyil_ContainsUnit on the
// local slot's grey-arrow set, game+21A4h + game+18ECh*4) from
// bsp::game::game_objective_sets(), which the mission Lua's Objectives_Add
// and Objectives_AddUnit fill, in place of the "not a member" record. OFF
// keeps the record.
inline constexpr bool kHudGreyArrowSetBound = true;

// Packet cc9_gunner_role_take (docs/SHIP_SCREEN_UPDATE.md sections 31-33).
// ON answers screen 2Eh's 009542B0 from the units host's permission words
// and the gunnery host's gun functions, so 0064DA40's bind selects the first
// available weapon group and takes its gunner roles through 0077C470, as the
// image does (the Lexington: group 2, roles 2 and 3). The fire message 79h
// and its route stay records: its effect, handing the in-window guns of the
// group to the player seat, is the gunnery host's (section 32's contract).
// OFF keeps the "not available" record.
inline constexpr bool kHudGunnerRoleTakeBound = true;

// Same packet, section 34. ON answers 005484F0's target at 00548856,
// [[00E198C4]+CCh]+4Ch, from screen 29h's pick (the manager's +CCh is slot
// 29h, docs/IN_MISSION_INTERFACE_MANAGER.md), which 00526A40 stores at +4Ch.
// OFF keeps the record answering none.
inline constexpr bool kHudWeaponGroupTargetBound = true;

// Packet cc9_screen_2eh_layout (docs/SHIP_SCREEN_UPDATE.md section 37). ON
// runs screen 2Eh's layout 00546A20 over GUI_cross_ship / GUI_cross_gunstate
// (once, before the first use) and binds 005484F0's widget calls to it: the
// hide-all 005452F0, the row-widget presence and show (argument +D4h) and
// widget +C0h's show. Screen 45h's 00545360 then sets +D4h. The F2 placement
// (GUI geometry), the group sight 00548300 and the other widget helpers stay
// records. ON by its USN04 pair (section 40).
inline constexpr bool kHudWeaponGroupLayoutBound = true;

// Packet cc9_gui_extent_inputs (docs/GUI_EXTENT_INPUTS.md). ON answers the
// GUI extent 00AA1FE0 that the markers screen and screen 29h's projection
// ask for from the platform object's widescreen byte (+0Dh) and active
// aspect (+10h), through bsp::gui_aspect_extent_00aa1fe0, in place of the
// (1, 1) substitution and its record. ON by its USN04 pair
// (docs/GUI_EXTENT_INPUTS.md section 5).
inline constexpr bool kHudGuiExtentBound = true;

// Packet cc9_role_retake_4bh (docs/SHIP_SCREEN_UPDATE.md section 39). ON runs
// screen 2Eh's enter 005494C0 and exit 005470A0 from the pump in place of the
// FrontEndScreen::enter/exit records. The enter's 00549260 retakes the held
// group's roles, which the 4Bh arm answers as a no-op. ON by its USN04 pair
// (section 40).
inline constexpr bool kHudWeaponGroupEnterExitBound = true;

// Packet cc13-releasegun (docs/CC13_IJN18LOSS.md section 7). ON lets a plan's
// `<frame> releasegun <unit>` line open the map the way the player's C1h
// (IC_MAP_TOGGLE) does: 0068C1F0 -> 0068B470 -> 0068AC30 publishes the level-2
// set {4Ch 39h 44h 29h 49h}, 4Ch's +8h (005A8EF0) occludes level 1, and screen
// 46h's exit 0064A490 releases the weapon group (005470C0) and roles 1..8
// (0077C470(unit, 1FEh, 0)). Everything here waits for the first such line,
// so a plan without one runs as OFF. OFF: the line is refused.
inline constexpr bool kHarnessReleaseGunMapBound = true;  // ON (cc9-lua70, 2026-10-19): IJN18 l70_i18_r trial, ref EndMission 1685.52 s (Zao 163 cat-6 shots / 111 hits vs 0); see WINS_TABLE

// Packet cc13-helmmapgate (docs/CC13_IJN18LOSS.md section 7). 0064B870, the helm
// transfer and lever issue, has one caller: 0064D656, inside screen 46h's update
// 0064D610. 46h is a level-1 screen. 4Ch's +8h (005A8EF0) occludes level 1, and
// 004F8906 updates only the screens that are requested, so 0064B870 does not run
// while the map is up. ON: while the host's map is up (GameHudHost's
// release_gun_map_up), the units host skips player_helm_prepare_0064b870 and
// player_helm_issue_0064b870. OFF: both run every fixed step, as before. Either
// way the skipped calls are counted in the summary.
inline constexpr bool kHelmPrepareMapUpGateCc13Bound = true;  // ON (cc9-lua70, 2026-10-19): IJN18 l70_i18_r trial, ref EndMission 1685.52 s (Zao 163 cat-6 shots / 111 hits vs 0); see WINS_TABLE
static_assert(!kHelmPrepareMapUpGateCc13Bound || kHarnessReleaseGunMapBound,
    "kHelmPrepareMapUpGateCc13Bound needs kHarnessReleaseGunMapBound: only that path opens the map");
static_assert(!kHelmPrepareMapUpGateCc13Bound || bsp::kCruiseLatchedPlayerArmCc13Bound,
    "kHelmPrepareMapUpGateCc13Bound needs kCruiseLatchedPlayerArmCc13Bound: without the latch "
    "009F3F80 drives the ring from a stale control block");

// Packet cc13-relgaps, gap 2 (docs/CC13_IJN18LOSS.md section 7). Screen 4Ch's
// update 005B45C0, while the map is up, releases the helm on every call:
// 005B4673..005B46B3 loads [00E188D8], takes its vtable[13Ch]() unit (the ship
// itself), and when that unit's role-1 holder +1B0h is not 8 (005B468D) and
// 00927F10 does not call the holder AI-held (005B4695), sends
// 0077C470(unit, 2, 0). ON: the menu pump's slot-4Ch update runs that release on
// the controlled unit read live each call. The rest of 005B45C0 stays unhosted.
// OFF: the slot-4Ch update falls through to the pump's default record.
// The only plan that opens the map (ijn18_l70_i18_r) never closes it, so ON
// releases the helm on every update for the rest of that run.
inline constexpr bool kMapScreenHelmReleaseCc13Bound = false;
static_assert(!kMapScreenHelmReleaseCc13Bound || kHarnessReleaseGunMapBound,
    "kMapScreenHelmReleaseCc13Bound needs kHarnessReleaseGunMapBound: only that path opens the map");
static_assert(!kMapScreenHelmReleaseCc13Bound || kHelmPrepareMapUpGateCc13Bound,
    "kMapScreenHelmReleaseCc13Bound needs kHelmPrepareMapUpGateCc13Bound: without the gate the map "
    "release and player_helm_prepare_0064b870's per-step re-take fight every frame, which the "
    "image never does because screen 46h is occluded while the map is up");

// Packet cc13-relgaps, gap 3. 0064DA40, the ship view's unit bind on the 25h
// arm, releases the previous unit's roles 1..8 before it stores the new one:
// 0064DA64 005470C0, then with +1Ch set 0064DA70..0064DA77
// 0077C470(+1Ch, 1FEh, 0), then the +1Ch observer is unregistered and +1Ch takes
// the new unit (0064DAA3). The image does this even when the new unit is the
// same one. ON: the host's bind sends that release to the previously stored
// ship-view unit, after the 005470C0 group release and before the 00549260
// re-take. OFF: no release, as before.
inline constexpr bool kShipViewPreviousUnitReleaseCc13Bound = false;
static_assert(!kShipViewPreviousUnitReleaseCc13Bound || kMissionCameraBound,
    "kShipViewPreviousUnitReleaseCc13Bound needs kMissionCameraBound: the release sits in the "
    "host's 0064DA40 bind");

// Packet cc13-relgaps, gap 1 (docs/INTERFACE_RUNTIME_TAIL.md section 2). 005CD1A0,
// the movie interface's once-per-instance engage, ends 005CD202..005CD224: with
// [00E188D8] set and 00927F30(unit, 0) true, 0077C470([00E188D8], 1FFh, 0)
// releases roles 0..8. Role 0 clears +184h and, for a ship, runs 0080E290
// (0078021C..00780244). ON: the movie binding sends that release through the
// units host's 4Bh message. OFF: recorded only, as before.
inline constexpr bool kMovieEngageRoleReleaseCc13Bound = false;

// Packet cc9_hud_camera_team_lists (docs/HUD_CAMERA_TEAM_LISTS.md). ON answers
// 004B4B00 exactly for the minimap (005C154E) and the markers screen: the
// controlled unit [00E188D8] when it is kind 5, its +3D0h when kind 18h (a
// record here: the leader is not exposed), else none. The routine reads no
// camera. ON by its USN04 pair (docs/HUD_CAMERA_TEAM_LISTS.md section 4).
inline constexpr bool kHudCameraUnitBound = true;

// Packet cc9_minimap_camera_getters (docs/HUD_MINIMAP_CAMERA.md). ON gives the
// minimap's direction wedge 005C1851..005C187F its first term: the camera
// unit's (004B4B00's) vtable +C8h, 0042BA40 = -atan2(unit+ECh, unit+F4h), the
// hull's world yaw, so the wedge turns with the ship: C8h - icon + pi/2
// (00CF1438 double). ON by its USN04 pair (docs/HUD_MINIMAP_CAMERA.md section 4).
inline constexpr bool kHudMinimapDirectionWedgeBound = true;

// Packet cc9_hud_gauge_text_mirrors (docs/UNIMPLEMENTED_RANKING_2.md section
// "HUD mirrors"). ON answers the in-mission interface update's 00E188D8 reads
// (has a controlled unit; 0068CC04 IsKindOf(8); 0068CC1x its +100h) from the
// units host's controlled unit, which BSP_Game_SetControlledUnit's host
// counterpart (004C0890) sets. ON by its USN04 pair.
inline constexpr bool kHudInterfaceControlledUnitBound = true;

// Packet cc13-fix24 (docs/CC13_FIX24.md). ON answers the marker host's
// unit_health_colour_0077a2f0 hook with the reconstructed 0077A2F0 body
// (src/cc13_front6.cpp) over host-owned copies of its function-local statics
// 00F8715C/00F8716C/00F8717C and guard 00F8718C, scaling through 00BEE140.
// OFF records the hook as unimplemented and zero-fills the RGBA. No runtime
// caller reaches the hook yet, so the switch has no observable effect today.
inline constexpr bool kHudUnitHealthColourBound = false;

class GameHostLog;
class GameFrontendHost;
class GameMenuHost;
class GameUnitsHost;
class GameMissionLuaHost;

// ---------------------------------------------------------------------------
// Slot 35h, the minimap
// ---------------------------------------------------------------------------

struct GameHudMinimapSummary {
    bool page_bound{false};
    bool groups_bound{false};
    std::size_t colour_templates{0};   // of the six minimap_units_*_Group items
    bool marker_group_bound{false};    // unit_marker_Group, screen +F0h
    // The two radii, from Globals["Minimap"] in the installed globals.lua.
    float minimap_range{0.0f};
    float visibility_range{0.0f};
    bool range_from_data{false};
    std::size_t icons_created{0};      // one per unit the walk ever placed
    std::size_t icons_placed{0};       // the last frame's count
    std::size_t units_culled{0};       // the last frame's VisibilityRange drops
    unsigned long long frames{0};
    float camera_heading{0.0f};        // radians, the stand-in basis
};

class GameHudMinimapHost {
public:
    GameHudMinimapHost(GameHostLog& log, GameMenuHost& menu);
    ~GameHudMinimapHost();
    GameHudMinimapHost(const GameHudMinimapHost&) = delete;
    GameHudMinimapHost& operator=(const GameHudMinimapHost&) = delete;

    // The created units the walk reads, and the Lua machine the two radii come
    // out of. Called once, after the instantiate pass created the units.
    void attach_world(GameUnitsHost& units, GameMissionLuaHost& lua);

    // One call of the slot 35h update virtual, from the recovered pump 004f8830.
    void update_005c0f20(float seconds);

    void report();
    const GameHudMinimapSummary& summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

// ---------------------------------------------------------------------------
// Slot 4Dh, the world markers
// ---------------------------------------------------------------------------

struct GameHudMarkersSummary {
    bool page_bound{false};
    bool template_bound{false};        // sidemarker_Group
    std::size_t markers_created{0};
    std::size_t markers_added{0};      // the last frame's 006430c0 acceptances
    std::size_t markers_rejected{0};   // the last frame's gate rejections
    std::size_t markers_on_screen{0};  // the last frame's visible bounds
    std::size_t markers_collapsed{0};  // 0063ab75's collapse, the zero-extent case
    unsigned long long frames{0};
    // The stand-in camera: the mission's own unit bounds and the half extent the
    // orthographic projection divides by.
    float bounds_min[3]{};
    float bounds_max[3]{};
    float camera_half_extent{0.0f};
    bool camera_fitted{false};
};

class GameHudMarkersHost {
public:
    GameHudMarkersHost(GameHostLog& log, GameMenuHost& menu);
    ~GameHudMarkersHost();
    GameHudMarkersHost(const GameHudMarkersHost&) = delete;
    GameHudMarkersHost& operator=(const GameHudMarkersHost&) = delete;

    void attach_world(GameUnitsHost& units);

    // One call of the slot 4Dh update virtual, from the recovered pump 004f8830.
    void update_006435d0(float seconds);

    void report();
    const GameHudMarkersSummary& summary() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
