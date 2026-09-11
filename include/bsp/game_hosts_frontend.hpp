#pragma once
// bsp_game.exe milestone 2b: the fonts/GUI half of application initialize and the GUI
// pages the title bring-up loads, as process bindings.
//
// Addresses: 0073bae0 (the fonts + GUI bring-up 0073e13c calls), 007371d0 (the font
// registry singleton), 00ac3910 (the descriptor load), 0053bc00 / 00be9620 / 00be9760
// (the fingerprint payload preload), 004c12b0 (the GUI manager singleton), 00aa5d70 /
// 00aa5e20 (its construction and its fixed resource list), 00aa5840 (the page loader),
// 00aa7e00 (the direct-child lookup), 00ac6600 (the per-page Lua evaluation) and
// 00aaa710 (the property binding and the child walk).
//
// Nothing in this file is a reconstruction of native code. Every type here is an
// integration binding that satisfies one of the host interfaces in bsp/gui_startup.hpp
// and bsp/gui_layout_loader.hpp with either a concrete implementation over an already
// reconstructed routine or the explicit unimplemented policy in GameHostLog.
//
// Evidence: docs/APP_INIT_FONTS_GUI.md, docs/GUI_LAYOUT_LOADER.md, docs/GUI_LUA_READER.md,
// docs/GUI_PAGE_SCRIPT.md, docs/GAME_TITLE_INIT.md, docs/GAME_EXECUTABLE.md.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <d3d9.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {

class GameHostLog;
class GameVfsHost;
class GameScriptHost;
class GameFontHost;

// One entry of the fixed resource list 00aa5e20 walks, as this run observed it.
struct GameGuiResourceRecord {
    std::string name;
    std::string kind;         // texture | group | child, the 00aa5e20 call that obtains it
    bool acquired{false};     // a non-null texture, page or child came back
    std::uint16_t offset{0};  // the manager field it is stored in, 0xFFFF when not stored
    std::string detail;
};

// One GUI page loaded through 00aa5840.
struct GamePageRecord {
    std::string name;
    std::int32_t priority{0};
    std::size_t widgets{0};
    bool loaded{false};
    bool model_backed{false};
    std::string error;
};

// One widget of a loaded page's tree. The rectangle is in the authored 0..1 GUI space:
// `x`/`y`/`z` is resolved_position (00aa6750, the parent chain), `width`/`height` is the
// widget's own size pair at +20h/+24h.
struct GameWidgetRecord {
    std::string page;
    std::string key;
    std::string type;
    std::string parent;
    std::string texture;   // the first authored state's Texture, "" when the page has none
    std::string material;  // ShaderName for an Icon or a FrameBox, Font for a Text
    int depth{0};
    float x{0.0f};
    float y{0.0f};
    float z{0.0f};
    float width{0.0f};
    float height{0.0f};
    float pivot_x{0.0f};
    float pivot_y{0.0f};
    // The projected +E4h value. 00aaa710 reads "Visible" only when the key is present and
    // never for a page root, so a widget with no key keeps the constructor's false.
    bool visible{false};
    bool visible_authored{false};  // the page table carried a "Visible" key
    bool drawn{false};             // the sprite bridge drew a quad for it
};

// What the milestone-2b phase produced, for the run summary and the report.
struct GameFrontendSummary {
    bool gui_startup_ran{false};
    bool font_descriptors_loaded{false};
    bool gui_manager_created{false};
    std::size_t gui_resources_acquired{0};
    std::size_t gui_resource_stores{0};
    std::size_t pages_loaded{0};
    std::size_t pages_requested{0};
    std::size_t widgets{0};
    std::size_t widgets_with_texture{0};
    // The sprite bridge. Not part of the reconstruction; see the class comment below.
    bool bridge_open{false};
    std::string bridge_atlas;
    std::size_t bridge_atlas_items{0};
    std::size_t bridge_textures{0};
    std::size_t bridge_quads{0};
    unsigned long long bridge_frames{0};
};

// Phase 7 of 0073d410 plus the title pages, owned for the whole run.
//
// run_font_and_gui_startup_0073bae0 drives bsp::run_gui_startup, the reconstruction of
// 0073bae0, over this binding, so the order the run executes is the recovered order
// rather than one this file invents. load_title_pages then loads the GUI pages that
// docs/GAME_TITLE_INIT.md names, through the reconstructed page loader 00aa5840 and the
// per-page Lua evaluation 00ac6600.
//
// draw_bridge is NOT a reconstruction. The native GUI draw path is another owner's, so
// the widgets are drawn by an executable-side Direct3D 9 sprite bridge: two textured
// triangles per widget, textures created with D3DXCreateTextureFromFileInMemoryEx from
// bytes the mounted VFS read, sub-rectangles from the reconstructed atlas parser
// 00aeeaf0. Every value it uses is authored data; none of its drawing is recovered.
class GameFrontendHost {
public:
    GameFrontendHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
        GameFontHost& fonts, IDirect3DDevice9& device, bool widescreen);
    ~GameFrontendHost();
    GameFrontendHost(const GameFrontendHost&) = delete;
    GameFrontendHost& operator=(const GameFrontendHost&) = delete;

    // 0073bae0 at 0073e13c, through bsp::run_gui_startup.
    void run_font_and_gui_startup_0073bae0(const std::string& language_font_path);
    // The pages GGame::OnInitTitle brings up: the two frame layouts 00518250 selects for
    // sets 0..2 and the title screen's own FE_initial. The front-end state machine that
    // would request them is packet cc_frontend_states and is not reconstructed here.
    void load_title_pages(const std::vector<std::string>& names);
    // Opens the sprite bridge against the back buffer size. Safe to call once.
    void open_sprite_bridge(unsigned back_buffer_width, unsigned back_buffer_height);
    // Draws the loaded widget trees. Call between BeginScene and EndScene.
    void draw_bridge(IDirect3DDevice9& device);

    const std::vector<GameGuiResourceRecord>& gui_resources() const noexcept;
    const std::vector<GamePageRecord>& pages() const noexcept;
    const std::vector<GameWidgetRecord>& widgets() const noexcept;
    const GameFrontendSummary& summary() const noexcept;

    // Defined in src/game_hosts_frontend.cpp. Public only so the host bindings in that
    // file can name it; nothing outside it uses the type.
    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
