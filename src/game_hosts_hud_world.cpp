// bsp_game.exe milestone 2k: the two in-mission HUD screens that show the world.
// See include/bsp/game_hosts_hud_world.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud_world.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"

#include "bsp/camera_projection.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/hud_markers_runtime.hpp"
#include "bsp/hud_minimap.hpp"
#include "bsp/hud_screens.hpp"
#include "bsp/hud_updates.hpp"
#include "bsp/pose_refresh.hpp"

#include <cmath>
#include <cstdio>
#include <map>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// The registry slots of the two screens, from docs/IN_GAME_INTERFACE_SCREEN_SETS.md.
constexpr int kMinimapScreenSlot = 0x35;
constexpr int kMarkersScreenSlot = 0x4D;

// 00639990 BSP_InGameHudMarkers_ResolveMarkerColourIndex, the non-objective
// part of the rule docs/HUD_MARKERS_RUNTIME.md states: 1 when [unit+54h] is 0,
// 0 when it is 1, 5 otherwise. The objective arm (2, 3 or 4 by 008DD240(unit)
// [+18h]) needs an objective container this process does not build.
int marker_colour_index_00639990(int team_id) noexcept {
    if (team_id == 0) return 1;
    if (team_id == 1) return 0;
    return 5;
}

}  // namespace

// ===========================================================================
// Slot 35h, the minimap
// ===========================================================================

struct GameHudMinimapHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in) : log(log_in), menu(menu_in) {}

    GameHostLog& log;
    GameMenuHost& menu;
    GameUnitsHost* units{nullptr};
    GameMissionLuaHost* lua{nullptr};
    GameHudMinimapSummary summary;

    GuiLayoutPage* page{nullptr};
    // The six `item_ship_Icon` templates at screen +6Ch..+7Ch, indexed by the
    // colour index 00639990 returns, and the parent at +F0h.
    GuiLayoutWidget* colour_templates[6]{};
    GuiLayoutWidget* marker_group{nullptr};   // unit_marker_Group, +F0h
    GuiLayoutWidget* compass{nullptr};        // +4Ch
    GuiLayoutWidget* island_map{nullptr};     // +50h
    GuiLayoutWidget* direction{nullptr};      // +48h

    // The per-unit icon map at screen +F8h, as this process holds it: one
    // run-time clone of the colour template per unit index.
    std::map<std::size_t, GuiLayoutWidget*> icons;
    // Stable handles, so the reconstruction's `void*` unit is an index rather
    // than a pointer into the units host.
    struct UnitHandle { std::size_t index{0}; };
    std::vector<UnitHandle> handles;

    float icon_heading{0.0f};
    bool bound_attempted{false};
    bool range_attempted{false};
    bool logged_camera{false};
    bool logged_entry{false};

    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }

    std::size_t index_of(void* handle) const noexcept {
        auto* typed = static_cast<UnitHandle*>(handle);
        return typed != nullptr ? typed->index : 0;
    }
    void* handle_for(std::size_t index) noexcept {
        if (index >= handles.size()) return nullptr;
        return &handles[index];
    }

    void bind_page_005bec50();
    GuiLayoutWidget* create_icon(std::size_t index);
};

namespace {

// bsp::HudMinimapHost, the boundary of src/hud_minimap.cpp. One native call site
// per method, in the order 005C0F20 reaches them.
class MinimapBinding final : public bsp::HudMinimapHost {
public:
    explicit MinimapBinding(GameHudMinimapHost::Impl& owner) : owner_(owner) {}

    void prepare_frame() override {
        // 005C0F3F -> 005BD420, the screen's own transform refresh. It belongs
        // to the slot 35h class, which is one of the 42 this process records.
        owner_.record("HudMinimap::prepare_frame", 0x005bd420u);
    }

    void* camera_unit() override {
        // 005C154E -> 004B4B00. There is no in-mission camera at game+19FCh in
        // this process, so the executable hands the pass the controlled unit and
        // records the call. This is the executable's decision, not a value the
        // routine returns.
        owner_.record("HudMinimap::camera_unit", 0x004b4b00u);
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return nullptr;
        return owner_.handle_for(owner_.units->controlled_index());
    }

    float minimap_range() override {
        // 005C157E -> 00432650, then +6Ch. The global config singleton is not
        // built here; the value is the one 0087D7B0 would have written.
        owner_.record("HudMinimap::global_config", 0x00432650u);
        return owner_.summary.minimap_range;
    }
    float visibility_range() override {
        // 005C158A -> 00432650, then +70h.
        owner_.record("HudMinimap::global_config", 0x00432650u);
        return owner_.summary.visibility_range;
    }

    void refresh_pose(void* unit) override {
        // 005C15BB -> 00414DB0 when [unit+C8h] is clear. Every created instance
        // publishes its world pose on the motion step, so +C8h is set and the
        // refresh is the no-op branch.
        static_cast<void>(unit);
        owner_.done("HudMinimap::refresh_pose", 0x00414db0u);
    }

    void* displayed_self_unit(void* controlled_unit) override {
        // 005C15F2 -> 00927880 on 00E188D8. The routine has no reconstruction,
        // so it is a record; the walk uses its result only to skip one cull.
        static_cast<void>(controlled_unit);
        owner_.record("HudMinimap::displayed_self_unit", 0x00927880u);
        return nullptr;
    }

    bsp::HudMinimapWorldPoint world_position(void* unit) override {
        // 005C1687 -> 00427EB0, which returns unit+FCh, the world-matrix
        // translation row (docs/HUD_MINIMAP.md correction 3).
        owner_.done("HudMinimap::world_position", 0x00427eb0u);
        bsp::HudMinimapWorldPoint point{};
        if (owner_.units == nullptr) return point;
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!owner_.units->unit_pose(owner_.index_of(unit), right, up, forward, translation)) {
            return point;
        }
        point.x = translation[0];
        point.y = translation[1];
        point.z = translation[2];
        return point;
    }

    void* team_unit_list_head(int team_index) override {
        // [[[00E188A8 + 18ECh*4 + 18CCh] + 30h] + E0Ch]. Nothing in this process
        // fills the local player's unit registry, which is the same stand-in
        // milestone 2i records for walk 0 of 004C3CB0: the executable hands the
        // walk the created units so the recovered filters run over real units.
        static_cast<void>(team_index);
        owner_.record("HudMinimap::team_unit_list", 0x004c3cb0u);
        if (owner_.handles.empty()) return nullptr;
        return &owner_.handles[0];
    }
    void* team_unit_list_next(void* node) override {
        const std::size_t next = owner_.index_of(node) + 1;
        return owner_.handle_for(next);
    }
    void* team_unit_list_unit(void* node) override { return node; }

    bool unit_is_alive_and_visible(void* unit) override {
        // 005C1628..005C164A, the same four bytes 0043F080 tests.
        owner_.done("HudMinimap::unit_alive_and_visible", 0x0043f080u);
        return owner_.units != nullptr
            && owner_.units->unit_alive_and_visible(owner_.index_of(unit));
    }

    bool is_kind_of(void* unit, int class_id) override {
        // 005C165D, the unit vtable +5Ch, over the recovered class chain.
        owner_.done("HudMinimap::is_kind_of", 0x006fe530u);
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.index_of(unit), class_id);
    }

    bool unit_shows_on_minimap(void* unit) override {
        // 005C1675, the unit vtable +B8h with no arguments. reports/hud_minimap.json
        // resolves the slot to 0043F080; the four-byte gate above has already
        // passed for every unit that reaches here, so the answer is true. The
        // slot's identity is the packet's reading and is marked uncertain.
        static_cast<void>(unit);
        owner_.record("HudMinimap::unit_shows_on_minimap", 0x0043f080u);
        return true;
    }

    void* find_icon_entry(void* unit) override {
        // 005C170A -> 005BE110, the lookup in the map at screen +F8h.
        owner_.record("HudMinimap::find_icon_entry", 0x005be110u);
        auto found = owner_.icons.find(owner_.index_of(unit));
        return found != owner_.icons.end() ? found->second : nullptr;
    }
    void* create_icon_entry(void* unit) override {
        // 005C171C..005C1767: 00BF681B allocates 0Ch bytes, 005BD590 constructs
        // the entry, 005C0700 inserts it and 00694A60 attaches it. None of the
        // four is reconstructed (packet `hud_minimap_icon_entries`), so all four
        // are records and what the executable creates instead is a clone of the
        // page's own `item_ship_Icon` template.
        owner_.record("HudMinimap::allocate_icon_entry", 0x00bf681bu);
        owner_.record("HudMinimap::construct_icon_entry", 0x005bd590u);
        owner_.record("HudMinimap::insert_icon_entry", 0x005c0700u);
        owner_.record("HudMinimap::attach_icon_entry", 0x00694a60u);
        return owner_.create_icon(owner_.index_of(unit));
    }

    void renderer_basis(float& y_component, float& x_component) override {
        // 005C17A7 -> 00B6DB70 then [renderer+110h] and [renderer+118h]. The
        // renderer at game+19FCh is the renderer owner's and this process has
        // none, so the executable supplies the controlled unit's own forward row
        // and records the refresh. The heading is then atan2(fwd.x, fwd.z),
        // which is the convention the trajectory dump already prints.
        owner_.record("HudMinimap::refresh_renderer_basis", 0x00b6db70u);
        owner_.done("HudMinimap::atan2", 0x00bf701au);
        y_component = 0.0f;
        x_component = 1.0f;
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return;
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!owner_.units->unit_pose(owner_.units->controlled_index(), right, up, forward,
                translation)) {
            return;
        }
        y_component = forward[0];
        x_component = forward[2];
    }

    void set_icon_position(void* icon, const bsp::HudGuiPoint& position) override {
        // 005C1C99 -> 00AA7DC0 BSP_GuiWidget_SetLocalPositionAndBounds.
        owner_.done("HudMinimap::set_icon_position", 0x00aa7dc0u);
        auto* widget = static_cast<GuiLayoutWidget*>(icon);
        if (widget == nullptr) return;
        owner_.menu.frontend().set_widget_local_position(*widget, position.x, position.y,
            position.z);
    }

    void set_icon_rotation(void* icon, float radians) override {
        // 005C1CD0, the widget rotation virtual +44h.
        owner_.done("HudMinimap::set_icon_rotation", 0x00aa7d00u);
        auto* widget = static_cast<GuiLayoutWidget*>(icon);
        if (widget == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*widget, radians);
    }

    void set_map_layer_rotation(void* widget, float radians) override {
        // 005C1825 and 005C184F, the same virtual on the compass and the map.
        owner_.done("HudMinimap::set_map_layer_rotation", 0x00aa7d00u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_rotation(*typed, radians);
    }

    void* map_layer(std::size_t slot) override {
        if (slot == bsp::kHudMinimapCompassSlot) return owner_.compass;
        if (slot == bsp::kHudMinimapIslandMapSlot) return owner_.island_map;
        if (slot == bsp::kHudMinimapDirIconSlot) return owner_.direction;
        return nullptr;
    }

private:
    GameHudMinimapHost::Impl& owner_;
};

}  // namespace

void GameHudMinimapHost::Impl::bind_page_005bec50() {
    if (bound_attempted) return;
    bound_attempted = true;
    // 005BEC50 loads GUI_minimap and stores the page root at +1Ch; it and the
    // layout binder 005BE240 then take every widget by name through 00AA7E00.
    // The screen's own register virtual is one of the 42 records, so what runs
    // here is the widget bind, over the page the HUD manager's Init loaded.
    record("HudMinimap::screen_register", 0x005bec50u);
    page = menu.in_game_page(kMinimapScreenSlot, bsp::kHudMinimapPageName);
    if (page == nullptr || !page->root) {
        log.note("minimap: GUI_minimap is not loaded on registry slot 35h, so the unit "
            "icon pass has no page to place icons in");
        return;
    }
    summary.page_bound = true;
    GuiLayoutWidget& root = *page->root;
    compass = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapCompassIcon);
    island_map = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapIslandMapIcon);
    direction = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapDirIcon);
    marker_group = bsp::find_child_by_name_00aa7e00(root, bsp::kHudMinimapMarkerGroup);
    done("HudMinimap::find_child", 0x00aa7e00u);
    summary.marker_group_bound = marker_group != nullptr;
    for (std::size_t slot = 0; slot < 6; ++slot) {
        GuiLayoutWidget* group = bsp::find_child_by_name_00aa7e00(root,
            bsp::kHudMinimapUnitGroups[slot]);
        if (group == nullptr) continue;
        colour_templates[slot] = bsp::find_child_by_name_00aa7e00(*group,
            bsp::kHudMinimapUnitItemIcon);
        if (colour_templates[slot] != nullptr) ++summary.colour_templates;
    }
    summary.groups_bound = summary.colour_templates > 0;
    // The island map's own content. minimap_terrain.mshd is the two-sampler GUI
    // shader effect; `RadarMap` at texture register 0 is a render target the
    // renderer owner produces, and 005BED21 is where 005BEC50 takes the widget
    // that receives it. Nothing in this process can produce that target.
    record("HudMinimap::radar_map_target", 0x005bed21u);
    log.notef("minimap page bound: %zu of 6 colour templates, unit_marker_Group %s, "
        "compass %s, island map %s, direction wedge %s; the RadarMap render target "
        "minimap_terrain.mshd samples is the renderer owner's and stays a record, so the "
        "island map keeps the page's authored error.tga",
        summary.colour_templates, marker_group != nullptr ? "bound" : "missing",
        compass != nullptr ? "bound" : "missing",
        island_map != nullptr ? "bound" : "missing",
        direction != nullptr ? "bound" : "missing");
}

GuiLayoutWidget* GameHudMinimapHost::Impl::create_icon(std::size_t index) {
    if (page == nullptr || marker_group == nullptr || units == nullptr) return nullptr;
    const GameUnitRow* row = units->unit_row(index);
    if (row == nullptr) return nullptr;
    const int colour = marker_colour_index_00639990(row->party);
    record("HudMinimap::marker_colour_index", 0x00639990u);
    GuiLayoutWidget* templ = colour_templates[colour < 0 || colour > 5 ? 0 : colour];
    if (templ == nullptr) {
        for (GuiLayoutWidget* candidate : colour_templates) {
            if (candidate != nullptr) { templ = candidate; break; }
        }
    }
    if (templ == nullptr) return nullptr;
    char key[64];
    std::snprintf(key, sizeof(key), "item_ship_Icon_%zu", index);
    // The clone is parented to `unit_marker_Group`, screen +F0h, which
    // docs/HUD_MINIMAP.md calls the per-unit icon map's parent: its authored
    // position (0.865167, 0.164028) is the minimap centre, and the six colour
    // groups' positions are not, so an icon whose local position is the small
    // offset 005C1C61 computes only lands on the map under this parent. Which
    // group the native entry is parented to is the open question of packet
    // `hud_minimap_icon_entries`.
    GuiLayoutWidget* icon = menu.frontend().clone_runtime_widget(page->name, *templ,
        *marker_group, key);
    if (icon == nullptr) return nullptr;
    icons.emplace(index, icon);
    ++summary.icons_created;
    if (!logged_entry) {
        logged_entry = true;
        log.notef("minimap icons: the 0Ch-byte per-unit entry of 005bd590 / 005c0700 / "
            "00694a60 has no reconstruction, so the executable clones the page's own "
            "`item_ship_Icon` template (colour index %d of six, from 00639990's rule on "
            "party %d) under unit_marker_Group and drives the clone through the sprite "
            "bridge; the clone is an executable-side stand-in", colour, row->party);
    }
    return icon;
}

GameHudMinimapHost::GameHudMinimapHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudMinimapHost::~GameHudMinimapHost() = default;

const GameHudMinimapSummary& GameHudMinimapHost::summary() const noexcept {
    return impl_->summary;
}

void GameHudMinimapHost::attach_world(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& host = *impl_;
    host.units = &units;
    host.lua = &lua;
    host.handles.clear();
    host.handles.resize(units.count());
    for (std::size_t index = 0; index < host.handles.size(); ++index) {
        host.handles[index].index = index;
    }
    if (host.range_attempted) return;
    host.range_attempted = true;
    // 0087D7B0 writes the two radii into the global config object at +6Ch and
    // +70h from Globals["Minimap"]; the loader is not reconstructed, so only its
    // two reads are performed, against the installed globals.lua.
    float range = 0.0f;
    float visibility = 0.0f;
    if (lua.read_minimap_globals_0087d7b0(range, visibility)) {
        host.summary.minimap_range = range;
        host.summary.visibility_range = visibility;
        host.summary.range_from_data = true;
    } else {
        host.log.note("minimap range: Globals[\"Minimap\"] did not load, so the pass has no "
            "radius and places no icon; 005c1799 would divide 80.0 by zero");
    }
}

void GameHudMinimapHost::update_005c0f20(float seconds) {
    Impl& host = *impl_;
    static_cast<void>(seconds);
    host.bind_page_005bec50();
    if (!host.summary.page_bound || host.units == nullptr) {
        host.record("HudMinimap::update", 0x005c0f20u);
        return;
    }
    MinimapBinding binding(host);
    binding.prepare_frame();

    // The heading the whole frame shares, 005C1788..005C1812. The pass computes
    // it again internally; it is recomputed here because the icon rotation at
    // 005C1CA1 is outside what src/hud_minimap.cpp covers.
    float basis_y = 0.0f;
    float basis_x = 0.0f;
    binding.renderer_basis(basis_y, basis_x);
    const bsp::HudMinimapHeadings headings = bsp::hud_minimap_headings_005c1788(
        std::atan2(basis_y, basis_x), host.summary.minimap_range);
    host.icon_heading = headings.icon;
    host.summary.camera_heading = headings.map;

    const std::size_t before = host.icons.size();
    // The team index is [00E188A8+18ECh]; this process's local player is the
    // controlled unit's own party, which is 0 for the Allied side of USN02.
    int team = 0;
    if (host.units->controlled_bound()) {
        const GameUnitRow* row = host.units->unit_row(host.units->controlled_index());
        if (row != nullptr && row->party >= 0 && row->party <= 7) team = row->party;
    }
    const int placed = bsp::hud_minimap_place_unit_icons_005c154e(team, binding,
        bsp::kHudMinimapDepthSelf);
    host.done("HudMinimap::update", 0x005c0f20u);
    host.summary.icons_placed = static_cast<std::size_t>(placed < 0 ? 0 : placed);
    const std::size_t reachable = host.units->count() > 0 ? host.units->count() - 1 : 0;
    host.summary.units_culled = reachable > host.summary.icons_placed
        ? reachable - host.summary.icons_placed : 0;
    ++host.summary.frames;

    // 005C1CA1..005C1CD0: pi/2 minus the sum of the unit's own heading and the
    // icon heading, handed to the widget rotation virtual +44h. The pass in
    // src/hud_minimap.cpp places positions only, so the rotation runs here.
    for (const auto& entry : host.icons) {
        const GameUnitRow* row = host.units->unit_row(entry.first);
        if (row == nullptr || entry.second == nullptr) continue;
        const float unit_heading = row->heading_degrees * 3.1415927f / 180.0f;
        binding.set_icon_rotation(entry.second,
            bsp::hud_minimap_icon_rotation_005c1ca1(unit_heading, headings.icon));
    }
    // 005C1872, the direction wedge: SetRotation(camera->vtable[C8h]() -
    // icon_heading + pi/2). The camera object is the renderer owner's, so the
    // heading it would return is recorded and the controlled unit's own is used.
    if (host.direction != nullptr) {
        host.record("HudMinimap::camera_heading_virtual", 0x004b4b00u);
        binding.set_map_layer_rotation(host.direction,
            bsp::kHudMinimapHeadingBias - headings.icon);
    }
    if (host.icons.size() != before && !host.logged_camera) {
        host.logged_camera = true;
        host.log.notef("minimap unit icons: %zu icon(s) created out of %zu created unit(s); "
            "the camera unit is the controlled unit (004b4b00 is a record, there is no "
            "camera at game+19FCh), so the player's own ship carries no dot and is the "
            "centre direction wedge, exactly as 005c1650 skips it",
            host.icons.size(), host.units->count());
    }
    // Where the icons actually land, once per run and again every 100 frames, so
    // the picture can be checked against the transform rather than trusted.
    if (host.summary.frames == 1 || host.summary.frames % 100 == 0) {
        const auto first = host.icons.begin();
        if (first != host.icons.end() && first->second != nullptr) {
            const GameUnitRow* row = host.units->unit_row(first->first);
            host.log.notef("  minimap frame %-4llu placed=%zu drawn=%zu  first=%s "
                "local=(%.5f, %.5f, %.1f) parent=%s", host.summary.frames,
                host.summary.icons_placed, host.menu.frontend().runtime_widgets_drawn(),
                row != nullptr ? row->name.c_str() : "?",
                static_cast<double>(first->second->transform.position.x),
                static_cast<double>(first->second->transform.position.y),
                static_cast<double>(first->second->transform.position.z),
                host.marker_group != nullptr ? host.marker_group->key.c_str() : "-");
        }
    }
}

void GameHudMinimapHost::report() {
    Impl& host = *impl_;
    if (!host.summary.page_bound) return;
    host.log.notef("summary mission minimap range=%.0f visibility=%.0f from_data=%d "
        "icons=%zu placed=%zu culled=%zu frames=%llu heading=%.4f rad",
        static_cast<double>(host.summary.minimap_range),
        static_cast<double>(host.summary.visibility_range),
        host.summary.range_from_data ? 1 : 0, host.summary.icons_created,
        host.summary.icons_placed, host.summary.units_culled, host.summary.frames,
        static_cast<double>(host.summary.camera_heading));
}

// ===========================================================================
// Slot 4Dh, the world markers
// ===========================================================================

struct GameHudMarkersHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in)
        : log(log_in), menu(menu_in),
          fallback_pose{bsp::PoseRefreshParentSlot(fallback_parent), fallback_local,
              fallback_valid, fallback_world, fallback_derived} {}

    GameHostLog& log;
    GameMenuHost& menu;
    GameUnitsHost* units{nullptr};
    GameHudMarkersSummary summary;

    GuiLayoutPage* page{nullptr};
    GuiLayoutWidget* sidemarker{nullptr};       // the authored template
    GuiLayoutWidget* type_icon{nullptr};        // its `type_Icon` child
    std::map<std::size_t, GuiLayoutWidget*> markers;

    bsp::HudMarkersUpdateState state{};
    bsp::HudMarkerClipRect clip{};
    bool wide_aspect{false};
    bool bound_attempted{false};
    bool logged_selection{false};
    bool logged_camera{false};
    // The marked set at screen +34h, as an index set for one frame.
    std::vector<std::size_t> marked;

    struct UnitHandle { std::size_t index{0}; };
    std::vector<UnitHandle> handles;

    // The pose the squad arm of 006435D0 would read. That arm never runs here:
    // its list is the world object's and the radius at unit+7C4h has no field.
    bsp::CameraMatrix fallback_local{};
    bsp::CameraMatrix fallback_world{};
    std::uint8_t fallback_valid{1};
    std::uint8_t fallback_derived{1};
    bsp::PoseRefreshView* fallback_parent{nullptr};
    bsp::PoseRefreshView fallback_pose;

    void record(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.unimplemented(method, text);
    }
    void done(const char* method, std::uint32_t address) {
        char text[16];
        format_address(address, text);
        log.implemented(method, text);
    }

    std::size_t index_of(void* handle) const noexcept {
        auto* typed = static_cast<UnitHandle*>(handle);
        return typed != nullptr ? typed->index : 0;
    }

    void bind_page_0063e280();
    void fit_stand_in_camera();
    void place_marker(std::size_t index, const bsp::HudMarkerScreenBounds& bounds);
};

namespace {

// bsp::HudMarkersRuntimeHost, the boundary of src/hud_markers_runtime.cpp.
class MarkerRuntimeBinding final : public bsp::HudMarkersRuntimeHost {
public:
    explicit MarkerRuntimeBinding(GameHudMarkersHost::Impl& owner) : owner_(owner) {}

    void gui_extent(float& width, float& height) override {
        // 00AA1FE0. The GUI space the page tree composes in is the unit square,
        // which is what the sprite bridge multiplies by the back buffer, so the
        // extent the executable supplies is (1, 1) and the native getter is a
        // record.
        owner_.record("HudMarkers::gui_extent", 0x00aa1fe0u);
        width = 1.0f;
        height = 1.0f;
    }
    void* camera_unit() override {
        owner_.record("HudMarkers::camera_unit", 0x004b4b00u);
        return nullptr;
    }
    void* displayed_self_unit(void* controlled_unit) override {
        static_cast<void>(controlled_unit);
        owner_.record("HudMarkers::displayed_self_unit", 0x00927880u);
        return nullptr;
    }
    bool parts_object_flag_is_one(void* unit) override {
        // 006430E4 -> 0080E490 then the byte at +14h. The parts object belongs to
        // packet `unit_parts`; a created instance of this process has none, so
        // the gate answers the value that lets the unit through and says so.
        static_cast<void>(unit);
        owner_.record("HudMarkers::parts_object", 0x0080e490u);
        return false;
    }
    bool is_kind_of(void* unit, int class_id) override {
        owner_.done("HudMarkers::is_kind_of", 0x006fe530u);
        return owner_.units != nullptr
            && owner_.units->unit_is_kind_of(owner_.index_of(unit), class_id);
    }
    bool is_alive_and_visible(void* unit) override {
        owner_.done("HudMarkers::is_alive_and_visible", 0x0043f080u);
        return owner_.units != nullptr
            && owner_.units->unit_alive_and_visible(owner_.index_of(unit));
    }
    bool already_marked(void* unit) override {
        // 0064311C -> 0063ABD0, the linear scan of the frame's vector at +34h.
        owner_.done("HudMarkers::already_marked", 0x0063abd0u);
        const std::size_t index = owner_.index_of(unit);
        for (std::size_t marked : owner_.marked) {
            if (marked == index) return true;
        }
        return false;
    }
    void mark_unit(void* unit) override {
        // 00643131 -> 004323D0, the push_back into the same vector.
        owner_.done("HudMarkers::mark_unit", 0x004323d0u);
        owner_.marked.push_back(owner_.index_of(unit));
    }
    bool objective_container_contains(void* unit) override {
        // 00643183 -> 008DDF90 against [00E188A8+21A4h+team*4]. The objective
        // containers belong to the mission objective owner; none exists here.
        static_cast<void>(unit);
        owner_.record("HudMarkers::objective_container", 0x008ddf90u);
        return false;
    }
    int marker_colour_index(void* unit) override {
        owner_.record("HudMarkers::marker_colour_index", 0x00639990u);
        if (owner_.units == nullptr) return 5;
        const GameUnitRow* row = owner_.units->unit_row(owner_.index_of(unit));
        return marker_colour_index_00639990(row != nullptr ? row->party : -1);
    }
    void refresh_pose(void* unit) override {
        static_cast<void>(unit);
        owner_.done("HudMarkers::refresh_pose", 0x00414db0u);
    }
    void world_matrix_rows(void* unit, float right[3], float up[3], float forward[3],
        float translation[3]) override {
        owner_.done("HudMarkers::world_matrix_rows", 0x00427eb0u);
        for (int lane = 0; lane < 3; ++lane) {
            right[lane] = 0.0f;
            up[lane] = 0.0f;
            forward[lane] = 0.0f;
            translation[lane] = 0.0f;
        }
        if (owner_.units == nullptr) return;
        owner_.units->unit_pose(owner_.index_of(unit), right, up, forward, translation);
    }
    void class_record_extents(void* unit, float& forward, float& right, float& up) override {
        // [unit+538h] +A0h, +A4h and +A8h. Only +A0h and +A8h have a recovered
        // Lua key and both are zero on this installation's ships, so the eight
        // corners collapse onto the anchor and 0063AB75's collapse path runs.
        owner_.record("HudMarkers::class_record_extents", 0x0063a7c3u);
        forward = 0.0f;
        right = 0.0f;
        up = 0.0f;
        if (owner_.units == nullptr) return;
        owner_.units->unit_class_extents(owner_.index_of(unit), forward, right, up);
    }
    void project_clip_space(const float world[3], float out_clip[4]) override {
        // 0043A6A0 -> 00B70490 for the view-projection matrix and 0043A6AF ->
        // 00B62D10 for the transform. Both are the renderer's, and this process
        // has no camera, so **the projection is an executable-side stand-in**: a
        // fixed top-down orthographic camera over the mission's own unit bounds,
        // with +x to the right and +z up the screen.
        owner_.record("HudMarkers::view_projection_matrix", 0x00b70490u);
        owner_.record("HudMarkers::transform_vec4", 0x00b62d10u);
        const float half = owner_.summary.camera_half_extent;
        const float cx = 0.5f * (owner_.summary.bounds_min[0] + owner_.summary.bounds_max[0]);
        const float cz = 0.5f * (owner_.summary.bounds_min[2] + owner_.summary.bounds_max[2]);
        out_clip[0] = half > 0.0f ? (world[0] - cx) / half : 0.0f;
        out_clip[1] = half > 0.0f ? (world[2] - cz) / half : 0.0f;
        out_clip[2] = 0.5f;
        out_clip[3] = 1.0f;
    }
    const char* unit_class_name(void* unit) override {
        owner_.record("HudMarkers::unit_class_name", 0x00803dc0u);
        if (owner_.units == nullptr) return "";
        const GameUnitRow* row = owner_.units->unit_row(owner_.index_of(unit));
        return row != nullptr ? row->type_symbol.c_str() : "";
    }
    float unit_health(void* unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::unit_health", 0x0077a2f0u);
        return 0.0f;
    }
    void* find_child(void* widget, const char* name) override {
        owner_.done("HudMarkers::find_child", 0x00aa7e00u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr || name == nullptr) return nullptr;
        return bsp::find_child_by_name_00aa7e00(*typed, name);
    }
    void set_visible(void* widget, bool visible) override {
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_visible(*typed, visible);
    }
    void set_local_position(void* widget, const bsp::HudGuiPoint& position) override {
        owner_.done("HudMarkers::set_local_position", 0x00aa7dc0u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr) return;
        owner_.menu.frontend().set_widget_local_position(*typed, position.x, position.y,
            position.z);
    }
    void set_resolved_position(void* widget, const bsp::HudGuiPoint& position) override {
        static_cast<void>(widget);
        static_cast<void>(position);
        owner_.record("HudMarkers::set_resolved_position", 0x00aa8240u);
    }
    void set_local_xy(void* widget, float x, float y) override {
        static_cast<void>(widget);
        static_cast<void>(x);
        static_cast<void>(y);
        owner_.record("HudMarkers::set_local_xy", 0x00aa7d00u);
    }
    void set_localised_text(void* widget, const char* key) override {
        owner_.record("HudMarkers::set_localised_text", 0x00abaed0u);
        auto* typed = static_cast<GuiLayoutWidget*>(widget);
        if (typed == nullptr || key == nullptr) return;
        owner_.menu.frontend().set_widget_text_source(*typed, key);
    }
    void widget_size(void* widget, float& width, float& height) override {
        owner_.record("HudMarkers::widget_size", 0x00aa6740u);
        width = 0.0f;
        height = 0.0f;
        static_cast<void>(widget);
    }
    void release_pool_tail(int pool, std::uint32_t cursor) override {
        static_cast<void>(pool);
        static_cast<void>(cursor);
        owner_.record("HudMarkers::release_pool_tail", 0x00640620u);
    }

private:
    GameHudMarkersHost::Impl& owner_;
};

// bsp::HudMarkersUpdateHost, the boundary of 006435D0 itself.
class MarkersUpdateBinding final : public bsp::HudMarkersUpdateHost {
public:
    MarkersUpdateBinding(GameHudMarkersHost::Impl& owner, MarkerRuntimeBinding& runtime)
        : owner_(owner), runtime_(runtime) {}

    int local_team_index() override {
        if (owner_.units == nullptr || !owner_.units->controlled_bound()) return 0;
        const GameUnitRow* row = owner_.units->unit_row(owner_.units->controlled_index());
        return row != nullptr && row->party >= 0 && row->party <= 7 ? row->party : 0;
    }
    bool game_hud_suppressed() override { return false; }
    void gui_extent(float& width, float& height) override {
        runtime_.gui_extent(width, height);
    }
    void publish_clip_rect(const bsp::HudMarkerClipRect& rect) override {
        // 00E197C4..00E197D8, one rectangle whose x bounds are stored twice
        // (docs/HUD_MARKERS_RUNTIME.md's correction).
        owner_.clip = rect;
        owner_.done("HudMarkers::publish_clip_rect", 0x0064361bu);
    }
    std::uint32_t camera_unit() override {
        owner_.record("HudMarkers::camera_unit", 0x004b4b00u);
        // There is no camera; the controlled unit stands in, which is what makes
        // the target, group and reinforcement gates reachable at all.
        return owner_.units != nullptr && owner_.units->controlled_bound() ? 1u : 0u;
    }
    void reset_marker_pool() override {
        // 00640620, the four pools. This process owns no pool entry, so the
        // release walks nothing and the routine is a record.
        owner_.record("HudMarkers::reset_marker_pool", 0x00640620u);
    }
    void clear_marker_set() override {
        owner_.done("HudMarkers::clear_marker_set", 0x0063bcd0u);
        owner_.marked.clear();
    }
    bool controlled_unit_present() override {
        return owner_.units != nullptr && owner_.units->controlled_bound();
    }
    std::uint32_t controlled_unit() override { return 1u; }
    std::uint32_t self_marker_unit() override {
        owner_.record("HudMarkers::self_marker_unit", 0x00927880u);
        return 1u;
    }
    void refresh_screen_state() override {
        owner_.record("HudMarkers::refresh_screen_state", 0x006394b0u);
    }
    void refresh_marker_layout() override {
        owner_.record("HudMarkers::refresh_marker_layout", 0x0063b5e0u);
    }

    void add_marker(std::uint32_t unit, int a, int b, int kind) override {
        if (unit == 0 || owner_.units == nullptr || !owner_.units->controlled_bound()) return;
        const std::size_t index = owner_.units->controlled_index();
        if (index >= owner_.handles.size()) return;
        // 006430C0 through its reconstruction. `a` is the force-objective-colour
        // byte, `b` and `kind` the two kind arguments the caller supplies.
        bsp::HudMarkerFlags flags = bsp::hud_marker_flags_006430c0(
            &owner_.handles[index], &owner_.handles[index], nullptr, 0, 0, false, a != 0);
        bsp::HudMarkerScreenBounds bounds{};
        const bsp::HudMarkerBuilder builder = bsp::hud_markers_add_unit_marker_006430c0(
            &owner_.handles[index], flags, a != 0, b, kind, false, 0, 0, owner_.clip,
            owner_.wide_aspect, bsp::kHudMarkersEnterField88, runtime_, bounds);
        owner_.done("HudMarkers::add_unit_marker", 0x006430c0u);
        if (builder == bsp::HudMarkerBuilder::None) {
            ++owner_.summary.markers_rejected;
            return;
        }
        ++owner_.summary.markers_added;
        if (bounds.collapsed) ++owner_.summary.markers_collapsed;
        owner_.place_marker(index, bounds);
    }
    void note_marker(std::uint32_t unit) override { static_cast<void>(unit); }

    std::uint32_t interface_target_unit() override {
        // [[00E198C4 + CCh] + 4Ch]. The interface manager's target is set by the
        // selector screen, which is one of the 42 records.
        owner_.record("HudMarkers::interface_target_unit", 0x0068aca0u);
        return 0;
    }
    int unit_team_id(std::uint32_t unit) override { static_cast<void>(unit); return 0; }
    int local_team_record_id() override {
        owner_.record("HudMarkers::local_team_record", 0x004c3cb0u);
        return 0;
    }
    bool unit_passes_target_filter(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::target_filter", 0x00804350u);
        return false;
    }
    bool target_is_selectable(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::target_selectable", 0x005220c0u);
        return false;
    }

    const void* squad_first_node() override {
        // [[00E188A8 + 19CCh] + 16Ch], a list on the world object 004DE610 would
        // build. The squad radius at unit+7C4h has no field either, and
        // hud_marker_within_radius rejects every distance against a zero radius,
        // so the squad arm would add nothing even with a list.
        owner_.record("HudMarkers::squad_list", 0x004de610u);
        return nullptr;
    }
    std::uint32_t squad_node_unit(const void* node) override {
        static_cast<void>(node);
        return 0;
    }
    const void* squad_next_node(const void* node) override {
        static_cast<void>(node);
        return nullptr;
    }
    bsp::PoseRefreshView& unit_pose(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::unit_pose", 0x00414db0u);
        return owner_.fallback_pose;
    }
    int squad_marker_radius(std::uint32_t unit) override {
        static_cast<void>(unit);
        owner_.record("HudMarkers::squad_marker_radius", 0x006437dbu);
        return 0;
    }

    bool pick_world_point(float screen_x, float screen_y, float& wx, float& wy,
        float& wz) override {
        // 0043A290, the crosshair unproject onto the plane y == 0. It needs the
        // same camera 0043A660 needs, and the stand-in is orthographic, so the
        // pick is the inverse of the same projection.
        owner_.record("HudMarkers::pick_crosshair_world_point", 0x0043a290u);
        static_cast<void>(screen_x);
        static_cast<void>(screen_y);
        wx = 0.0f;
        wy = 0.0f;
        wz = 0.0f;
        return false;
    }
    void publish_crosshair_point(bool valid, float wx, float wy, float wz) override {
        static_cast<void>(valid);
        static_cast<void>(wx);
        static_cast<void>(wy);
        static_cast<void>(wz);
        owner_.record("HudMarkers::publish_crosshair_point", 0x006438f7u);
    }
    void viewport_size(int& width, int& height) override {
        owner_.record("HudMarkers::viewport_descriptor", 0x00b6fde0u);
        width = 0;
        height = 0;
    }

    void sweep_objectives() override {
        owner_.record("HudMarkers::sweep_objectives", 0x00643360u);
    }
    void sweep_command_units() override {
        owner_.record("HudMarkers::sweep_command_units", 0x00642c20u);
    }
    void add_group_marker(std::uint32_t unit, int flag) override {
        static_cast<void>(unit);
        static_cast<void>(flag);
        owner_.record("HudMarkers::add_group_marker", 0x006434e0u);
    }
    std::uint32_t query_target_unit() override {
        owner_.record("HudMarkers::query_target_unit", 0x00523020u);
        return 0;
    }
    bool unit_is_group_leader(std::uint32_t unit) override {
        static_cast<void>(unit);
        return false;
    }
    std::size_t group_member_count(std::uint32_t unit) override {
        static_cast<void>(unit);
        return 0;
    }
    std::uint32_t group_member(std::uint32_t unit, std::size_t index) override {
        static_cast<void>(unit);
        static_cast<void>(index);
        return 0;
    }
    void flush_markers() override {
        // 00640D70 then 00640620: the target section callout and the closing
        // pool reset. The callout needs a target and a parts object.
        owner_.record("HudMarkers::target_section_callout", 0x00640d70u);
        owner_.record("HudMarkers::reset_marker_pool", 0x00640620u);
    }
    float objective_detail_level() override {
        owner_.record("HudMarkers::objective_detail_level", 0x00f88a00u);
        return 0.0f;
    }
    void sweep_reinforcements() override {
        owner_.record("HudMarkers::sweep_reinforcements", 0x00643d1au);
    }

private:
    GameHudMarkersHost::Impl& owner_;
    MarkerRuntimeBinding& runtime_;
};

}  // namespace

void GameHudMarkersHost::Impl::bind_page_0063e280() {
    if (bound_attempted) return;
    bound_attempted = true;
    // 0063E280 BSP_HudMarkersScreen_Register loads GUI_markers and stores it at
    // the screen's +8Ch; the screen class is one of the 42 records, so what runs
    // here is the widget bind over the page the manager's Init loaded.
    record("HudMarkers::screen_register", 0x0063e280u);
    page = menu.in_game_page(kMarkersScreenSlot, "GUI_markers");
    if (page == nullptr || !page->root) {
        log.note("markers: GUI_markers is not loaded on registry slot 4Dh, so the marker "
            "pass has no page to place markers in");
        return;
    }
    summary.page_bound = true;
    sidemarker = bsp::find_child_by_name_00aa7e00(*page->root, "sidemarker_Group");
    done("HudMarkers::find_child", 0x00aa7e00u);
    if (sidemarker != nullptr) {
        type_icon = bsp::find_child_by_name_00aa7e00(*sidemarker, "type_Icon");
        summary.template_bound = type_icon != nullptr;
    }
    log.notef("markers page bound: sidemarker_Group %s, its type_Icon %s. The per-marker "
        "widget writer 0063d1e0 has no reconstruction, so the executable clones the "
        "authored sidemarker_Group and places the clone itself",
        sidemarker != nullptr ? "bound" : "missing",
        type_icon != nullptr ? "bound" : "missing");
}

void GameHudMarkersHost::Impl::fit_stand_in_camera() {
    if (summary.camera_fitted || units == nullptr || units->count() == 0) return;
    summary.camera_fitted = true;
    bool first = true;
    for (std::size_t index = 0; index < units->count(); ++index) {
        float right[3] = {0.0f, 0.0f, 0.0f};
        float up[3] = {0.0f, 0.0f, 0.0f};
        float forward[3] = {0.0f, 0.0f, 0.0f};
        float translation[3] = {0.0f, 0.0f, 0.0f};
        if (!units->unit_pose(index, right, up, forward, translation)) continue;
        for (int lane = 0; lane < 3; ++lane) {
            if (first || translation[lane] < summary.bounds_min[lane]) {
                summary.bounds_min[lane] = translation[lane];
            }
            if (first || translation[lane] > summary.bounds_max[lane]) {
                summary.bounds_max[lane] = translation[lane];
            }
        }
        first = false;
    }
    const float half_x = 0.5f * (summary.bounds_max[0] - summary.bounds_min[0]);
    const float half_z = 0.5f * (summary.bounds_max[2] - summary.bounds_min[2]);
    const float half = half_x > half_z ? half_x : half_z;
    // 1.2 is the stand-in's own margin, so a ship that leaves the start box
    // still projects inside the clip rectangle for a while. It is not a
    // recovered value; nothing about this camera is.
    summary.camera_half_extent = half > 0.0f ? half * 1.2f : 1.0f;
    log.notef("markers camera: **an executable-side stand-in**, a fixed top-down "
        "orthographic camera over the mission's own unit bounds x[%.0f, %.0f] z[%.0f, %.0f], "
        "half extent %.0f. 00b70490 and 00b62d10, the view-projection matrix and the "
        "transform 0043a660 needs, are records: this process builds no camera at "
        "game+19FCh, so nothing here is a claim about what the game projects",
        static_cast<double>(summary.bounds_min[0]), static_cast<double>(summary.bounds_max[0]),
        static_cast<double>(summary.bounds_min[2]), static_cast<double>(summary.bounds_max[2]),
        static_cast<double>(summary.camera_half_extent));
}

void GameHudMarkersHost::Impl::place_marker(std::size_t index,
    const bsp::HudMarkerScreenBounds& bounds) {
    if (page == nullptr || sidemarker == nullptr || type_icon == nullptr) return;
    const float centre_x = 0.5f * (bounds.min_x + bounds.max_x);
    const float centre_y = 0.5f * (bounds.min_y + bounds.max_y);
    if (!(centre_x > clip.left && centre_x < clip.right && centre_y > clip.top
            && centre_y < clip.bottom)) {
        return;
    }
    ++summary.markers_on_screen;
    GuiLayoutWidget* marker = nullptr;
    auto found = markers.find(index);
    if (found != markers.end()) {
        marker = found->second;
    } else {
        char key[64];
        std::snprintf(key, sizeof(key), "sidemarker_Group_%zu", index);
        marker = menu.frontend().clone_runtime_widget(page->name, *sidemarker, *page->root,
            key);
        if (marker == nullptr) return;
        markers.emplace(index, marker);
        ++summary.markers_created;
        // 0063D1E0 binds `Unit_name_Text` by name and fills it at 0063D751
        // through 00ABBE50, the plain C-string setter, not the localisation id
        // setter 00ABAED0 at 0063D694. A unit's instance name is not a locale
        // id, and the executable's text path only carries ids, so the label
        // keeps the page's authored `Unit Name` and the C-string setter is a
        // record. Nothing invents a string here.
        record("HudMarkers::set_unit_name_text", 0x00abbe50u);
        if (bsp::find_child_by_name_00aa7e00(*marker, "Unit_name_Text") != nullptr) {
            done("HudMarkers::find_child", 0x00aa7e00u);
        }
    }
    // The authored template puts `type_Icon` at its own offset inside the group,
    // so the group is moved by the difference rather than to the point itself.
    const float offset_x = type_icon->transform.position.x
        - sidemarker->transform.pivot_x * sidemarker->transform.size.width;
    const float offset_y = type_icon->transform.position.y
        - sidemarker->transform.pivot_y * sidemarker->transform.size.height;
    menu.frontend().set_widget_local_position(*marker, centre_x - offset_x,
        centre_y - offset_y, sidemarker->transform.position.z);
}

GameHudMarkersHost::GameHudMarkersHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudMarkersHost::~GameHudMarkersHost() = default;

const GameHudMarkersSummary& GameHudMarkersHost::summary() const noexcept {
    return impl_->summary;
}

void GameHudMarkersHost::attach_world(GameUnitsHost& units) {
    Impl& host = *impl_;
    host.units = &units;
    host.handles.clear();
    host.handles.resize(units.count());
    for (std::size_t index = 0; index < host.handles.size(); ++index) {
        host.handles[index].index = index;
    }
    host.fit_stand_in_camera();
}

void GameHudMarkersHost::update_006435d0(float seconds) {
    Impl& host = *impl_;
    host.bind_page_0063e280();
    if (!host.summary.page_bound || host.units == nullptr) {
        host.record("HudMarkers::update", 0x006435d0u);
        return;
    }
    host.wide_aspect = false;
    host.summary.markers_added = 0;
    host.summary.markers_rejected = 0;
    host.summary.markers_on_screen = 0;
    host.summary.markers_collapsed = 0;
    MarkerRuntimeBinding runtime(host);
    MarkersUpdateBinding update(host, runtime);
    const bool ran = bsp::hud_markers_screen_update(host.state, update, seconds);
    host.done("HudMarkers::update", 0x006435d0u);
    if (!ran) return;
    ++host.summary.frames;
    if (!host.logged_selection) {
        host.logged_selection = true;
        host.log.notef("markers selection: 006435d0 marks the controlled unit, the interface "
            "manager's target, the squad members within unit+7C4h, the objectives, the "
            "command units and the target group. This process has only the controlled unit, "
            "so the pass added %zu marker(s) and the other five sources are records "
            "(0068aca0's target, 004de610's squad list, 00643360, 00642c20 and 006434e0). "
            "Marking every ship would be an invented selection, not this routine's",
            host.summary.markers_added);
    }
}

void GameHudMarkersHost::report() {
    Impl& host = *impl_;
    if (!host.summary.page_bound) return;
    host.log.notef("summary mission markers added=%zu rejected=%zu on_screen=%zu "
        "collapsed=%zu widgets=%zu frames=%llu camera=orthographic half=%.0f",
        host.summary.markers_added, host.summary.markers_rejected,
        host.summary.markers_on_screen, host.summary.markers_collapsed,
        host.summary.markers_created, host.summary.frames,
        static_cast<double>(host.summary.camera_half_extent));
}

}  // namespace bsp::game
