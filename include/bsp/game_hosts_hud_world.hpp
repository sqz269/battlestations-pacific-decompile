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
