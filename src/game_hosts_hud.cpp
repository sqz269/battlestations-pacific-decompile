// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages.
// See include/bsp/game_hosts_hud.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_menu.hpp"

#include "bsp/hud_screens.hpp"
#include "bsp/ingame_interface.hpp"

#include <cstdio>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameHudHost::Impl {
    Impl(GameHostLog& log_in, GameMenuHost& menu_in) : log(log_in), menu(menu_in) {}

    GameHostLog& log;
    GameMenuHost& menu;
    GameHudSummary summary;
    InGameInterfaceManager manager{};

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
};

// ---------------------------------------------------------------------------
// 0068aca0's host
// ---------------------------------------------------------------------------

namespace {

// The 20h arm's unit probe. 0068ae0b is reached only with a payload; this
// process has no local player unit, so the arm takes its null-payload path and
// none of these is called. They record rather than answer.
class HudUnitQuery final : public InGameInterfaceUnitQuery {
public:
    explicit HudUnitQuery(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_is_kind_of(int type_code) override {
        static_cast<void>(type_code);
        owner_.record("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
        return false;
    }
    bool plane_is_in_flight() override {
        owner_.record("InGameInterface::plane_is_in_flight", 0x007bb9a0u);
        return false;
    }
    bool has_delegate_unit() override {
        owner_.record("InGameInterface::has_delegate_unit", 0x0068af18u);
        return false;
    }

private:
    GameHudHost::Impl& owner_;
};

class HudInterfaceBinding final : public InGameInterfaceHost {
public:
    HudInterfaceBinding(GameHudHost::Impl& owner, HudUnitQuery& query)
        : owner_(owner), query_(query) {}

    bool apply_base_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 00684600. The lock at 00e19894 is the front-end owner's and is clear
        // in this process, so the base accepts the request; the record keeps the
        // base body itself out of the concrete count.
        owner_.record("InGameInterface::apply_base_request", 0x00684600u);
        owner_.summary.applied_interface_id = interface_id;
        return true;
    }
    bool payload_unit_is_dead() override { return false; }
    void limbo_screen_take_unit() override {
        owner_.record("InGameInterface::limbo_screen_take_unit", 0x00565fb0u);
    }
    void hud_root_screen_set_interface(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        // 00646040 BSP_InGameHudRootScreen_SetInterface, the per-id hook the
        // manager calls on registry slot 44h before the switch.
        owner_.record("InGameInterface::hud_root_set_interface", 0x00646040u);
    }
    bool mission_overlay_present() override { return false; }
    void tick_mission_overlay() override {
        owner_.record("InGameInterface::tick_mission_overlay", 0x0042a930u);
    }
    void set_level1_screen_set(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_screen_set_004f8530(ids, count);
        owner_.done("InGameInterface::set_level1_screen_set", 0x004f8530u);
        owner_.summary.level1_screens = 0;
        owner_.summary.level1_screen_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;  // the varargs terminator
            owner_.summary.level1_screen_ids.push_back(ids[i]);
            ++owner_.summary.level1_screens;
        }
    }
    void set_level1_input_contexts(const int* ids, std::size_t count) override {
        owner_.menu.publish_level1_input_contexts_004d8a50(ids, count);
        owner_.done("InGameInterface::set_level1_input_contexts", 0x004d8a50u);
        owner_.summary.level1_contexts = 0;
        owner_.summary.level1_context_ids.clear();
        for (std::size_t i = 0; i < count && ids != nullptr; ++i) {
            if (ids[i] == 0) break;
            owner_.summary.level1_context_ids.push_back(ids[i]);
            ++owner_.summary.level1_contexts;
        }
    }
    void screen_receive_unit(std::uint16_t manager_offset) override {
        static_cast<void>(manager_offset);
        owner_.record("InGameInterface::screen_receive_unit", 0x0068ad07u);
    }
    int hud_root_screen_query() override {
        owner_.record("InGameInterface::hud_root_screen_query", 0x00644230u);
        return 0;
    }
    void bomb_view_screen_bind(int value) override { static_cast<void>(value); }
    void bomb_view_screen_store(int value) override { static_cast<void>(value); }
    std::uint32_t unit_ambient_sound_source() override { return 0; }
    void stop_ambient_sound() override {
        owner_.record("InGameInterface::stop_ambient_sound", 0x0054d510u);
    }
    std::uint32_t start_ambient_sound(std::uint32_t source) override {
        static_cast<void>(source);
        owner_.record("InGameInterface::start_ambient_sound", 0x00a7acf0u);
        return 0;
    }
    void collapse_overlays(bool clear_level2, bool run_extra_hook) override {
        static_cast<void>(clear_level2);
        static_cast<void>(run_extra_hook);
        // 0068ab80 collapses the level-2 and level-3 sets. Both are empty here.
        owner_.record("InGameInterface::collapse_overlays", 0x0068ab80u);
    }
    InGameInterfaceUnitQuery& unit_query() override { return query_; }
    bool is_multiplayer() override { return false; }
    void redispatch(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        owner_.record("InGameInterface::redispatch", 0x0068afb4u);
    }

private:
    GameHudHost::Impl& owner_;
    HudUnitQuery& query_;
};

}  // namespace

// ---------------------------------------------------------------------------
// GameHudHost
// ---------------------------------------------------------------------------

GameHudHost::GameHudHost(GameHostLog& log, GameMenuHost& menu)
    : impl_(std::make_unique<Impl>(log, menu)) {}

GameHudHost::~GameHudHost() = default;

const GameHudSummary& GameHudHost::summary() const noexcept { return impl_->summary; }

void GameHudHost::build_manager_0068a990() {
    Impl& impl = *impl_;
    if (impl.summary.manager_built) return;
    // 004e0452 allocates 108h bytes and 004e046c runs the constructor; 0068cc70
    // is the Init the caller runs next. Neither is reconstructed.
    impl.record("InGameInterface::construct_manager", 0x0068a990u);
    impl.record("InGameInterface::manager_init", 0x0068cc70u);
    // 0068cc9a: the atlas `interface/Textures/game.ats`. The sprite bridge
    // already merges every atlas the installation ships, so the load is the
    // bridge's, not this one's, and the native call stays a record.
    impl.record("InGameInterface::load_atlas", 0x00aefa30u);

    for (std::size_t i = 0; i < kInGameHudScreenCount; ++i) {
        const InGameHudScreenSlot& slot = kInGameHudScreens[i];
        const HudScreenLayout* layout = hud_screen_layout_for_slot(slot.registry_slot);
        GameHudScreenRecord record;
        record.registry_slot = slot.registry_slot;
        record.manager_offset = slot.offset;
        record.register_virtual = layout != nullptr ? layout->register_virtual : 0;
        record.layout_virtual = layout != nullptr ? layout->layout_virtual : 0;

        // Init builds each screen with operator new(size) and its constructor,
        // then calls its virtual +10h. The 42 classes are not reconstructed, so
        // the allocation and the constructor are one record and the registry
        // record is the executable's own, the same substitution milestone 2c
        // makes for the seven main-menu screen classes.
        // One record for the pair. The 42 leaf constructors have 42 distinct
        // addresses, which kInGameHudScreens carries; recording them under one
        // method name would put 42 different call sites on one row.
        impl.record("InGameInterface::create_screen", 0x00bf681bu);
        char name[32];
        std::snprintf(name, sizeof(name), "HUD%02X", static_cast<unsigned>(slot.registry_slot));
        if (!impl.menu.register_in_game_screen(slot.registry_slot, name,
                record.register_virtual, record.layout_virtual)) {
            impl.log.notef("HUD screen slot %02X could not be registered",
                static_cast<unsigned>(slot.registry_slot));
            continue;
        }
        // 004f71d0, which every one of the 42 +10h overrides calls or jumps to.
        impl.done("InGameInterface::screen_register", 0x004f71d0u);
        ++impl.summary.screens_built;

        if (layout != nullptr) {
            for (std::size_t p = 0; p < layout->page_count; ++p) {
                const std::string page = layout->pages[p];
                record.pages.push_back(page);
                ++impl.summary.pages_requested;
                if (impl.menu.attach_in_game_page(slot.registry_slot, page)) {
                    ++record.pages_loaded;
                    ++impl.summary.pages_loaded;
                }
            }
            for (std::size_t w = 0; w < layout->widget_count; ++w) {
                const std::string widget = layout->widgets[w].name;
                ++record.widgets_requested;
                ++impl.summary.widgets_requested;
                if (impl.menu.in_game_page_has_child(slot.registry_slot, widget)) {
                    ++record.widgets_bound;
                    ++impl.summary.widgets_bound;
                }
            }
        }
        impl.summary.screens.push_back(std::move(record));
    }

    // 0068d73a inside Init: 004cc460(this, 20h, 0), the first interface request.
    impl.summary.pushed_interface_id = kInterfaceScene3d;
    impl.record("InGameInterface::push_interface_request", 0x004cc460u);
    impl.summary.manager_built = true;
    impl.log.notef("in-mission HUD manager: %zu of %zu screens registered, %zu of %zu pages "
        "loaded, %zu of %zu named widgets bound; INTF_SCENE3D (%02Xh) pushed by Init at "
        "0068d73a", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.pages_loaded,
        impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.pushed_interface_id));
}

void GameHudHost::apply_pending_interface_0068aca0() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built || impl.summary.interface_applied) return;
    // 006840f0 services the pending record and 00684600 hands the id to the
    // manager's own virtual +10h.
    impl.record("InGameInterface::service_pending_request", 0x006840f0u);
    HudUnitQuery query(impl);
    HudInterfaceBinding binding(impl, query);
    const bool accepted = apply_in_game_interface_0068aca0(impl.manager, binding,
        kInterfaceScene3d, false);
    impl.done("InGameInterface::apply_pending_interface", 0x0068aca0u);
    impl.summary.interface_applied = accepted;

    // Which of the level-1 screens hold a page, for the run's own report.
    impl.summary.level1_pages.clear();
    for (const int id : impl.summary.level1_screen_ids) {
        for (GameHudScreenRecord& record : impl.summary.screens) {
            if (record.registry_slot != id) continue;
            record.in_level1_set = true;
            for (const std::string& page : record.pages) {
                impl.summary.level1_pages.push_back(page);
            }
        }
    }

    std::string screens;
    for (const int id : impl.summary.level1_screen_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        screens += text;
    }
    std::string contexts;
    for (const int id : impl.summary.level1_context_ids) {
        char text[8];
        std::snprintf(text, sizeof(text), "%02Xh ", static_cast<unsigned>(id));
        contexts += text;
    }
    std::string pages;
    for (const std::string& page : impl.summary.level1_pages) {
        if (!pages.empty()) pages += ' ';
        pages += page;
    }
    impl.log.notef("in-mission level-1 set for INTF_SCENE3D (null payload, single player): "
        "screens %s| contexts %s| pages %s", screens.c_str(), contexts.c_str(),
        pages.c_str());
}

void GameHudHost::release_main_menu_manager_00686c90() {
    impl_->menu.destroy_main_menu_manager_00686c90();
    impl_->log.note("the main-menu manager was destroyed: 00687300 -> 00686c90 -> 00683aa0, "
        "whose tail publishes the empty level-4 screen set (004f8710) and the empty level-4 "
        "input contexts (004d8c00), so the front-end pages come down");
}

void GameHudHost::update_interface_only_004c40f0(float raw_delta) {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.menu.pump_interface_only_004c40f0(raw_delta);
    ++impl.summary.pump_frames;
}

}  // namespace bsp::game
