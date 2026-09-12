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
