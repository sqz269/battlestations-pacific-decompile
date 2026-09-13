// bsp_game.exe milestone 2h: the in-mission HUD screens and their GUI pages.
// See include/bsp/game_hosts_hud.hpp for the address list and the evidence.
#include "bsp/game_hosts_hud.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_hud_world.hpp"
#include "bsp/game_hosts_menu.hpp"
#include "bsp/game_hosts_units.hpp"

#include "bsp/hud_screens.hpp"
#include "bsp/in_mission_interface_runtime.hpp"
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
    InMissionInterfaceUpdateState update_state{};
    // 004bca50's answer during the mission. The load resolves a single-player
    // campaign mission to 8, the same value the scene contents pass uses.
    int effective_game_mode{8};
    // Milestone 2j: 004c9ca0's loading element at [00e198c4]+D4h. The 1 arm
    // allocates 34h bytes at 004c9cd7 and constructs with 00636d90, which has no
    // reconstruction, so what this process holds is the object's identity and
    // the two bytes the routine itself reads and writes: +4h at 004c9d1f /
    // 004c9d82 and +5h at 004c9d72 / 004c9d85.
    bool loading_element_present{false};
    bool loading_element_4{false};
    bool loading_element_5{false};
    bool in_game_interface_applied{false};

    // Milestone 2k. The two screens whose update virtual is reconstructed, and
    // the controlled-unit payload the second interface request carries.
    std::unique_ptr<GameHudMinimapHost> minimap;
    std::unique_ptr<GameHudMarkersHost> markers;
    GameUnitsHost* units{nullptr};
    bool unit_request_pending{false};
    bool unit_request_applied{false};
    int unit_interface_id{0};

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

// The 20h arm's unit probe, 0068ae0b. Milestone 2h reached it only with a null
// payload, so none of these was called and all three recorded. Milestone 2k
// makes the second request carry the controlled unit, and the probe is then the
// recovered class test over the recovered chain: a ship answers IsKindOf(6), so
// the arm re-enters its own virtual +10h with 25h INTF_CAPTAIN.
class HudUnitQuery final : public InGameInterfaceUnitQuery {
public:
    explicit HudUnitQuery(GameHudHost::Impl& owner) : owner_(owner) {}
    bool unit_is_kind_of(int type_code) override {
        if (owner_.units != nullptr && owner_.units->controlled_bound()) {
            owner_.done("InGameInterface::unit_is_kind_of", 0x0068ae1cu);
            return owner_.units->unit_is_kind_of(owner_.units->controlled_index(),
                type_code);
        }
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
        // 0068AFB4, `this->vtable[10h](newId, payload)`, which is 0068ACA0
        // itself. Milestone 2h recorded it because the null-payload arm chose no
        // id; with the controlled unit as the payload the 20h arm is a unit-kind
        // classifier and this re-entry is what publishes the ship HUD.
        owner_.done("InGameInterface::redispatch", 0x0068afb4u);
        if (depth_ >= 4) return;  // the native has no guard; a cycle would hang
        ++depth_;
        apply_in_game_interface_0068aca0(owner_.manager, *this, interface_id, has_payload);
        --depth_;
    }

private:
    GameHudHost::Impl& owner_;
    HudUnitQuery& query_;
    int depth_{0};
};

// ---------------------------------------------------------------------------
// 0068c1f0's host, the per-frame update
// ---------------------------------------------------------------------------
//
// Step 17 of the in-mission branch of 004e4a40 calls this at 004e5252. Almost
// everything it reads belongs to one of the 42 screen classes, to the camera at
// game+19FCh or to the controlled unit at 00e188d8, and this process owns none
// of those: every one of them is a record with its own address and a neutral
// answer, and the recovered control flow runs over them. The two exceptions are
// the level-3 vector and the input contexts, which are the same stacks the
// level-1 publish uses.
class HudUpdateBinding final : public InMissionInterfaceUpdateHost {
public:
    explicit HudUpdateBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    int effective_game_mode() override {
        owner_.done("InGameInterfaceUpdate::effective_game_mode", 0x004bca50u);
        return owner_.effective_game_mode;
    }
    int local_player_slot() override { return 0; }
    bool local_slot_flag_19() override { return false; }
    int local_slot_unit_id() override { return -1; }
    bool has_controlled_unit() override {
        // 00e188d8, written only by BSP_Game_SetControlledUnit 004c0880. Nothing
        // in this process sets a controlled unit, so the HUD has no unit.
        owner_.record("InGameInterfaceUpdate::controlled_unit", 0x00e188d8u);
        return false;
    }
    bool input_action_pressed(int action) override {
        static_cast<void>(action);
        owner_.record("InGameInterfaceUpdate::input_action_pressed", 0x004c43c0u);
        return false;
    }
    bool modifier_key_allows_back_out() override { return false; }
    bool session_flag_19c4() override { return false; }
    bool overlay_c_extra_gate() override { return false; }

    bool camera_screen_spectating() override { return record_false("006529e0", 0x006529e0u); }
    bool camera_screen_flag_09() override { return record_false("00652a20", 0x00652a20u); }
    bool camera_screen_flag_08() override { return record_false("00652a30", 0x00652a30u); }
    void camera_screen_set_flag_08(bool value) override {
        static_cast<void>(value);
        owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_08", 0x00652a50u);
    }
    bool camera_screen_flag_0a() override { return false; }
    void camera_screen_set_flag_0a(bool value) override { static_cast<void>(value); }
    bool camera_screen_flag_30() override { return false; }
    bool camera_screen_flag_66() override { return record_false("006529c0", 0x006529c0u); }
    void camera_screen_set_flag_66(bool value) override {
        static_cast<void>(value);
        owner_.record("InGameInterfaceUpdate::camera_screen_set_flag_66", 0x006529b0u);
    }
    void camera_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::camera_screen_frame_step", 0x00673130u);
    }

    std::size_t level3_screen_count() override { return 0; }
    int level3_screen_id(std::size_t index) override { static_cast<void>(index); return 0; }
    void set_level3_screen_set(const int* ids, std::size_t count) override {
        static_cast<void>(ids);
        static_cast<void>(count);
        owner_.record("InGameInterfaceUpdate::set_level3_screen_set", 0x004f8670u);
    }
    void set_level3_input_contexts(const int* ids, std::size_t count) override {
        static_cast<void>(ids);
        static_cast<void>(count);
        owner_.record("InGameInterfaceUpdate::set_level3_input_contexts", 0x004d8b70u);
    }

    // The spectate walk over [[game+19cch]+58h]. construct_world 004de610 is a
    // load record, so the world object that list hangs off does not exist.
    std::size_t spectate_unit_count() override {
        owner_.record("InGameInterfaceUpdate::spectate_unit_list", 0x004de610u);
        return 0;
    }
    bool unit_is_kind_of(std::size_t, int) override { return false; }
    bool unit_has_pilot(std::size_t) override { return false; }
    int unit_owner_id(std::size_t) override { return -1; }
    int unit_pilot_state(std::size_t) override { return 0; }
    bool unit_dead(std::size_t) override { return false; }
    int unit_player_slot(std::size_t) override { return -1; }
    int unit_team(std::size_t) override { return 0; }
    void pilot_detach(std::size_t) override {
        owner_.record("InGameInterfaceUpdate::pilot_detach", 0x007ee4e0u);
    }
    void unit_bind_player(std::size_t, int) override {
        owner_.record("InGameInterfaceUpdate::unit_bind_player", 0x00927cc0u);
    }
    void hud_root_set_spectated_unit(std::size_t, bool) override {
        owner_.record("InGameInterfaceUpdate::hud_root_set_spectated_unit", 0x00647300u);
    }

    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(interface_id);
        static_cast<void>(has_payload);
        owner_.record("InGameInterfaceUpdate::push_interface_request", 0x004cc460u);
    }
    void push_scene_request_for_unit() override {
        owner_.record("InGameInterfaceUpdate::push_scene_request_for_unit", 0x004b4b00u);
    }
    void toggle_tactical_overlay(std::uint32_t) override {
        owner_.record("InGameInterfaceUpdate::toggle_tactical_overlay", 0x0068c0b0u);
    }
    void exit_free_camera_overlay() override {
        owner_.record("InGameInterfaceUpdate::exit_free_camera_overlay", 0x0068b3f0u);
    }
    void back_out_one_overlay_level() override {
        owner_.record("InGameInterfaceUpdate::back_out_one_overlay_level", 0x0068b470u);
    }
    void collapse_overlays(bool, bool) override {
        owner_.record("InGameInterfaceUpdate::collapse_overlays", 0x0068ab80u);
    }
    void toggle_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_movie_camera", 0x0068a160u);
    }
    void toggle_new_movie_camera() override {
        owner_.record("InGameInterfaceUpdate::toggle_new_movie_camera", 0x0068a1f0u);
    }
    bool query_scene_unit(SceneRequestUnitFlags& flags, bool& is_pending_payload) override {
        flags = SceneRequestUnitFlags{};
        is_pending_payload = false;
        owner_.record("InGameInterfaceUpdate::query_scene_unit", 0x004b4b00u);
        return false;
    }

    void hud_root_set_field_1c(std::uint32_t) override {
        owner_.record("InGameInterfaceUpdate::hud_root_set_field_1c", 0x00644220u);
    }
    bool screen_2b_wanted() override { return false; }
    void screen_2b_set_mode(bool) override {
        owner_.record("InGameInterfaceUpdate::screen_2b_set_mode", 0x0053d290u);
    }
    bool screen_34_query() override { return record_false("0054d3c0", 0x0054d3c0u); }
    bool screen_2a_wanted() override { return false; }
    void screen_2a_clear_wanted() override {}
    bool screen_3b_block_a() override { return false; }
    bool screen_3b_block_b() override { return false; }
    void screen_3b_idle() override {
        owner_.record("InGameInterfaceUpdate::screen_3b_idle", 0x005fb080u);
    }
    bool screen_45_block_a() override { return false; }
    bool screen_45_block_b() override { return false; }
    bool map_screen_wanted() override { return false; }
    void map_screen_frame_step() override {
        owner_.record("InGameInterfaceUpdate::map_screen_frame_step", 0x00612da0u);
    }
    bool map_screen_busy() override { return record_false("00611750", 0x00611750u); }
    std::uint32_t map_screen_overlay_argument() override { return 0; }
    bool pause_screen_flag_05() override { return false; }
    bool pause_screen_flag_08() override { return false; }
    bool pause_screen_child_query() override { return false; }
    bool support_request_pending() override { return record_false("008ed9c0", 0x008ed9c0u); }

    void input_manager_update(float seconds) override {
        static_cast<void>(seconds);
        // 004bec00 then 00a92c40. The mission frame already ticks the input
        // manager through its own step; this call is the manager's own.
        owner_.record("InGameInterfaceUpdate::input_manager_update", 0x004bec00u);
    }
    bool device_a_flag_0b() override { return false; }
    bool device_a_flag_10() override { return false; }
    bool device_b_flag_0b() override { return false; }
    bool device_b_flag_10() override { return false; }
    std::uint32_t controlled_unit_overlay_argument() override { return 0; }

    bool has_ambient_sound_instance() override { return false; }
    bool camera_present() override {
        // game+19fch, the in-mission camera. Nothing in this process builds one.
        owner_.record("InGameInterfaceUpdate::camera", 0x004e4a40u);
        return false;
    }
    void refresh_camera_transform() override {
        owner_.record("InGameInterfaceUpdate::refresh_camera_transform", 0x00b6db70u);
    }
    float camera_height() override { return 0.0f; }
    float camera_ground_x() override { return 0.0f; }
    void set_ambient_volume(float volume) override {
        static_cast<void>(volume);
        owner_.record("InGameInterfaceUpdate::set_ambient_volume", 0x00a79880u);
    }
    float water_height(float x, float y) override {
        static_cast<void>(x);
        static_cast<void>(y);
        owner_.record("InGameInterfaceUpdate::water_height", 0x0078cf20u);
        return 0.0f;
    }
    bool cockpit_screen_wanted() override { return false; }
    int cockpit_screen_mode() override { return record_false("00604f30", 0x00604f30u) ? 1 : 0; }
    bool second_cockpit_screen_wanted() override { return false; }
    int second_cockpit_screen_mode() override { return 0; }
    bool controlled_unit_is_submarine() override { return false; }
    void refresh_controlled_unit_pose() override {}
    float controlled_unit_height() override { return 0.0f; }
    void set_audio_environment(std::string_view name) override {
        owner_.summary.audio_environment.assign(name.data(), name.size());
        owner_.record("InGameInterfaceUpdate::set_audio_environment", 0x00a7b710u);
    }
    void tick_profile_hints(int slot) override {
        static_cast<void>(slot);
        owner_.record("InGameInterfaceUpdate::tick_profile_hints", 0x004c1e90u);
    }

private:
    bool record_false(const char* suffix, std::uint32_t address) {
        char method[80];
        std::snprintf(method, sizeof(method), "InGameInterfaceUpdate::query_%s", suffix);
        owner_.record(method, address);
        return false;
    }

    GameHudHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0068cc70's host, the manager's Init
// ---------------------------------------------------------------------------
//
// bsp::init_in_game_interface_0068cc70 drives this, so the order the 42 screens
// are built and registered in is the recovered one. The 42 leaf classes are not
// reconstructed: the allocation and the constructor are records and the
// registry record is the executable's own, the substitution milestone 2c makes
// for the seven main-menu screen classes. What register_screen then does is the
// recovered part of each +10h override, 004f71d0 plus the page the screen loads
// through 00aa5840 and the widget names it binds through 00aa7e00.
class HudInitBinding final : public InMissionInterfaceInitHost {
public:
    explicit HudInitBinding(GameHudHost::Impl& owner) : owner_(owner) {}

    void load_texture_atlas(std::string_view stem) override {
        static_cast<void>(stem);
        // 00af0060 on the atlas manager at 00f8c26c. The sprite bridge already
        // merges every atlas the installation ships, so the load is the
        // bridge's and the native call stays a record.
        owner_.record("InGameInterface::load_atlas", 0x00af0060u);
    }
    void publish_manager_global() override {
        // 0068ccfe, the second write of 00e198c4. The object is this process's
        // own InGameInterfaceManager, so the publish is real.
        owner_.summary.manager_global_published = true;
        owner_.done("InGameInterface::publish_manager_global", 0x0068ccfeu);
    }
    void base_init() override {
        // 0068cd04, the base Init 00683a90, a bare RET in this build.
        owner_.done("InGameInterface::base_init", 0x00683a90u);
    }
    bool allocate_screen(std::size_t index, std::size_t size_bytes) override {
        static_cast<void>(index);
        static_cast<void>(size_bytes);
        owner_.record("InGameInterface::allocate_screen", 0x00bf681bu);
        return true;
    }
    void construct_screen(std::size_t index, std::uint32_t constructor) override {
        static_cast<void>(index);
        // The 42 leaf constructors have 42 distinct addresses, all of them call
        // sites inside 0068cc70; kInGameHudScreens carries them. One record
        // stands for the set, because recording them under one method name
        // would put 42 call sites on one row.
        if (constructor != 0) ++owner_.summary.screen_constructors;
        owner_.record("InGameInterface::screen_constructor", 0x0068cc70u);
    }
    void register_screen(std::size_t index) override {
        if (index >= kInGameHudScreenCount) return;
        const InGameHudScreenSlot& slot = kInGameHudScreens[index];
        const HudScreenLayout* layout = hud_screen_layout_for_slot(slot.registry_slot);
        GameHudScreenRecord record;
        record.registry_slot = slot.registry_slot;
        record.manager_offset = slot.offset;
        record.register_virtual = layout != nullptr ? layout->register_virtual : 0;
        record.layout_virtual = layout != nullptr ? layout->layout_virtual : 0;

        char name[32];
        std::snprintf(name, sizeof(name), "HUD%02X",
            static_cast<unsigned>(slot.registry_slot));
        if (!owner_.menu.register_in_game_screen(slot.registry_slot, name,
                record.register_virtual, record.layout_virtual)) {
            owner_.log.notef("HUD screen slot %02X could not be registered",
                static_cast<unsigned>(slot.registry_slot));
            return;
        }
        owner_.done("InGameInterface::screen_register", 0x004f71d0u);
        ++owner_.summary.screens_built;

        if (layout != nullptr) {
            for (std::size_t p = 0; p < layout->page_count; ++p) {
                const std::string page = layout->pages[p];
                record.pages.push_back(page);
                ++owner_.summary.pages_requested;
                if (owner_.menu.attach_in_game_page(slot.registry_slot, page)) {
                    ++record.pages_loaded;
                    ++owner_.summary.pages_loaded;
                }
            }
            for (std::size_t w = 0; w < layout->widget_count; ++w) {
                const std::string widget = layout->widgets[w].name;
                ++record.widgets_requested;
                ++owner_.summary.widgets_requested;
                if (owner_.menu.in_game_page_has_child(slot.registry_slot, widget)) {
                    ++record.widgets_bound;
                    ++owner_.summary.widgets_bound;
                }
            }
        }
        owner_.summary.screens.push_back(std::move(record));
    }
    void push_interface_request(int interface_id, bool has_payload) override {
        static_cast<void>(has_payload);
        // 0068d73a, 004cc460(20h, 0). The pending record it writes is the
        // front-end owner's; the id is what the executable applies.
        owner_.summary.pushed_interface_id = interface_id;
        owner_.record("InGameInterface::push_interface_request", 0x004cc460u);
    }
    void hud_root_set_field_1c(std::uint32_t value) override {
        static_cast<void>(value);
        owner_.record("InGameInterface::hud_root_set_field_1c", 0x00644220u);
    }

private:
    GameHudHost::Impl& owner_;
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
    // is the Init the caller runs next. Packet cc_hud_updates reconstructed
    // both, so the executable runs them rather than recording them: the object
    // is bsp::InGameInterfaceManager and the order the 42 screens are built and
    // registered in is 0068cc70's own.
    construct_in_game_interface_0068a990(impl.manager);
    impl.done("InGameInterface::construct_manager", 0x0068a990u);

    HudInitBinding init(impl);
    init_in_game_interface_0068cc70(impl.manager, init);
    impl.done("InGameInterface::manager_init", 0x0068cc70u);

    impl.summary.manager_built = true;
    // manager+3Ch, the byte 00684700 sets and 006840FE and the frame's step 17
    // test. The manager's vtable slot +08h is 00684700 itself, so the native
    // raises it through the vtable; nothing in the reconstructed load reaches
    // that call, and Ghidra reports only three direct callers of 00684700, none
    // of them this manager. The executable sets the byte so the request Init
    // pushed can be serviced and step 17 can run, and says so. The other half
    // of 00684700, deactivating every other registered manager, has already
    // happened: the load destroyed the main-menu manager through 00686c90.
    impl.manager.base.active = true;
    impl.record("InGameInterface::activate_manager", 0x00684700u);
    impl.log.notef("in-mission HUD manager: %zu of %zu screens registered through "
        "bsp::init_in_game_interface_0068cc70 (%zu leaf constructors are records), %zu of "
        "%zu pages loaded, %zu of %zu named widgets bound; INTF_SCENE3D (%02Xh) pushed by "
        "Init at 0068d73a", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.screen_constructors,
        impl.summary.pages_loaded, impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.pushed_interface_id));
}

void GameHudHost::apply_pending_interface_0068aca0() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    bool has_payload = false;
    if (!impl.summary.interface_applied) {
        has_payload = false;
    } else if (impl.unit_request_pending && !impl.unit_request_applied) {
        // Milestone 2k: the second request, the one that carries the controlled
        // unit. Serviced through the same 006840f0 / 00684600 path.
        has_payload = true;
        impl.unit_request_applied = true;
    } else {
        return;
    }
    // 006840f0 services the pending record and 00684600 hands the id to the
    // manager's own virtual +10h.
    impl.record("InGameInterface::service_pending_request", 0x006840f0u);
    HudUnitQuery query(impl);
    HudInterfaceBinding binding(impl, query);
    const bool accepted = apply_in_game_interface_0068aca0(impl.manager, binding,
        kInterfaceScene3d, has_payload);
    impl.done("InGameInterface::apply_pending_interface", 0x0068aca0u);
    if (!has_payload) impl.summary.interface_applied = accepted;
    if (has_payload) impl.unit_interface_id = impl.summary.applied_interface_id;

    // Which of the level-1 screens hold a page, for the run's own report.
    impl.summary.level1_pages.clear();
    for (GameHudScreenRecord& record : impl.summary.screens) record.in_level1_set = false;
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
    impl.log.notef("in-mission level-1 set for INTF_SCENE3D (%s payload, single player) "
        "applied as %02Xh: screens %s| contexts %s| pages %s",
        has_payload ? "controlled unit" : "null",
        static_cast<unsigned>(impl.summary.applied_interface_id), screens.c_str(),
        contexts.c_str(), pages.c_str());
    if (has_payload) {
        impl.log.note("the 20h arm classified the controlled unit through the recovered "
            "IsKindOf chain and re-entered its own virtual +10h, which is what raises the "
            "world markers screen (4Dh) and keeps the minimap (35h): both have a "
            "reconstructed update virtual and the pump 004f8830 now calls them");
    }
}

void GameHudHost::attach_world_2k(GameUnitsHost& units, GameMissionLuaHost& lua) {
    Impl& impl = *impl_;
    impl.units = &units;
    if (!impl.minimap) impl.minimap = std::make_unique<GameHudMinimapHost>(impl.log, impl.menu);
    if (!impl.markers) impl.markers = std::make_unique<GameHudMarkersHost>(impl.log, impl.menu);
    impl.minimap->attach_world(units, lua);
    impl.markers->attach_world(units);
}

void GameHudHost::detach_world_2k() noexcept {
    Impl& impl = *impl_;
    impl.units = nullptr;
    impl.unit_request_pending = false;
    impl.unit_request_applied = false;
    impl.unit_interface_id = 0;
    // The child hosts borrow units and Lua. Their default destructors release
    // source caches without invoking their borrowed owners.
    impl.markers.reset();
    impl.minimap.reset();
}

void GameHudHost::request_scene_interface_for_unit_004cc460() {
    Impl& impl = *impl_;
    if (impl.units == nullptr || !impl.units->controlled_bound()) return;
    if (impl.unit_request_pending) return;
    // The load creates the units on its `load_scene_contents` row and builds the
    // HUD manager on a later row, so the request outlives the push: it is
    // serviced by the first 004c40f0 pass that finds the manager built, right
    // after the null-payload request Init pushed. In the game the pusher is the
    // HUD root, which by definition already has a manager; that ordering is the
    // executable's and is the only part of this step that is.
    // 004cc460(20h, unit). The native pushers are the HUD root's own 00649860,
    // 006485a0 and 00647300, all of which read a HUD root object this process
    // does not own, so the push is recorded and the executable makes it.
    impl.record("InGameInterface::push_interface_request", 0x004cc460u);
    impl.unit_request_pending = true;
    impl.log.note("interface request 20h pushed with the controlled unit as its payload: "
        "milestone 2h applied the null-payload request Init pushes at 0068d73a, whose arm "
        "publishes only 29h 49h 44h 35h; the same arm with a unit is a classifier");
}

void GameHudHost::update_minimap_screen_005c0f20(float seconds) {
    Impl& impl = *impl_;
    if (!impl.minimap) {
        impl.record("HudMinimap::update", 0x005c0f20u);
        return;
    }
    impl.minimap->update_005c0f20(seconds);
}

void GameHudHost::update_markers_screen_006435d0(float seconds) {
    Impl& impl = *impl_;
    if (!impl.markers) {
        impl.record("HudMarkers::update", 0x006435d0u);
        return;
    }
    impl.markers->update_006435d0(seconds);
}

void GameHudHost::apply_in_game_interface_004c9ca0(bool loading) {
    Impl& host = *impl_;
    // 004c9cc0 tests the one stack argument and branches on it, so this is two
    // routines sharing a frame. The load calls it with 1 at 004e1873 and the
    // mission-state entry with 0 at 004da746.
    if (loading) {
        // 004c9ccd..004c9d68, the arm that puts the loading element up. The
        // element is allocated at 004c9cd7 and constructed by 00636d90, neither
        // of which is reconstructed, so both are records; the two stores this
        // process can make are the slot at manager+D4h and the byte at +4h.
        if (!host.loading_element_present) {
            host.record("MissionLoad::allocate_loading_element", 0x00bf681bu);
            host.record("MissionLoad::construct_loading_element", 0x00636d90u);
            host.loading_element_present = true;
            // 004c9d11, the element's own vtable +10h right after the store.
            host.record("MissionLoad::loading_element_init", 0x004c9d11u);
        }
        host.loading_element_4 = true;   // 004c9d1f
        host.done("MissionLoad::apply_in_game_interface", 0x004c9ca0u);
        host.log.notef("in-game interface applied with 1: the loading element at "
            "[00e198c4]+D4h is up and its +4h byte is set; the single-player branch at "
            "004c9d23 ends the routine, because game+1FE4h is zero here");
        return;
    }

    // 004c9d6b..004c9ea2, the arm the mission-state entry takes.
    if (host.loading_element_present) {
        if (host.loading_element_5) {
            // 004c9d7e, the element's exit virtual +1Ch, only when +5h is set.
            host.record("MissionEntry::loading_element_exit", 0x004c9d7eu);
        }
        host.loading_element_4 = false;  // 004c9d82
        host.loading_element_5 = false;  // 004c9d85
        // 004c9d88, 004f83b0 on the element: the cleared bytes are committed
        // into the page tree. No page is attached to this element, so the walk
        // reaches nothing and the call is recorded rather than claimed.
        host.record("MissionEntry::commit_loading_element", 0x004f83b0u);
        // 004c9daa, the element's deleting destructor through vtable +0Ch with
        // 1, then the slot is nulled at 004c9dac.
        host.record("MissionEntry::destroy_loading_element", 0x004c9daau);
        host.loading_element_present = false;
    }
    // 004c9dae..004c9dcf: the name at 00ce765c is assigned into a pooled string
    // and handed to the atlas manager at 00f8c26c. The sprite bridge owns every
    // atlas in this process, so the release is a record.
    host.record("MissionEntry::release_front_end_atlas", 0x00aefa30u);
    // 004c9dfb: PUSH 1; PUSH 3; CALL 004c1ac0; MOV ECX,EAX; CALL 00518250. The
    // two pushes are 00518250's arguments (set 3, commit 1) and 004c1ac0 takes
    // none: it is the lazy getter for the 164h front-end frame object at
    // 00e18d80. This is the committing call, and it is what releases the front
    // end's own layouts.
    host.done("MissionEntry::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, true);
    // 004c9e1d: 007f8d60(game+650h, game+2198h) and the latch at game+1EE0h,
    // which is the engine-movie flag the frame reads. The comparison is over the
    // player profile's own record set, which this process does not own.
    host.record("MissionEntry::engine_movie_check", 0x007f8d60u);
    // 004c9e32: the single-player tail. game+1FE4h is zero in this process, so
    // the branch at 004c9e38 jumps over the input-context push 004bec00 /
    // 00a933f0, the controlled-unit registry walk 008053c0 / 008073c0, the unit
    // list rebuild 004c3cb0 and the interface refresh 006485a0. Recording them
    // would claim call sites this run does not reach, so it does not.
    host.in_game_interface_applied = true;
    host.done("MissionEntry::apply_in_game_interface", 0x004c9ca0u);
    host.log.notef("in-game interface applied with 0: the loading element is torn down and "
        "00518250(3, commit=1) at 004c9e06 releases every other set's layouts, so FE_frame "
        "and FE_frame_title come off the screen and set 3's GUI_pause pair is acquired "
        "hidden");
}

void GameHudHost::select_front_end_layout_00518250() {
    // The load's own row at 004e04e4: PUSH EBX (0); PUSH 3; CALL 004c1ac0; MOV
    // ECX,EAX; CALL 00518250. Milestone 2i read the two pushes as 004c1ac0's
    // arguments; they are 00518250's (set 3, commit 0). 004c1ac0 is
    // BSP_FrontEndFrame_GetOrCreate, the lazy getter for the 164h front-end
    // frame object at 00e18d80, and it takes none: it only supplies ECX. Set 3
    // is the pause pair GUI_pause / GUI_pause_title
    // (bsp/title_init.hpp, FrontEndFrameSet::Pause).
    impl_->done("MissionLoad::front_end_frame_get", 0x004c1ac0u);
    impl_->menu.select_front_end_frame_set_00518250(3, false);
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

void GameHudHost::report() {
    Impl& impl = *impl_;
    if (!impl.summary.manager_built) return;
    impl.log.notef("summary mission hud screens=%zu/%zu pages=%zu/%zu widgets=%zu/%zu "
        "interface=%02Xh level1_screens=%zu level1_contexts=%zu pump_frames=%llu "
        "update_frames=%llu audio_environment=%s", impl.summary.screens_built,
        static_cast<std::size_t>(kInGameHudScreenCount), impl.summary.pages_loaded,
        impl.summary.pages_requested, impl.summary.widgets_bound,
        impl.summary.widgets_requested,
        static_cast<unsigned>(impl.summary.applied_interface_id),
        impl.summary.level1_screens, impl.summary.level1_contexts,
        impl.summary.pump_frames, impl.summary.update_frames,
        impl.summary.audio_environment.empty() ? "(none)"
                                               : impl.summary.audio_environment.c_str());
    // Milestone 2j. `minimap_islandmap_Icon` of GUI_minimap names the texture
    // `error.tga` with the material `minimap_terrain.mshd`, and milestone 2h
    // read that as the page's own authored texture. It is, and the material is
    // not an ordinary single-texture one: the installed
    // shaderfx/gui/minimap_terrain.shfx declares two samplers, `RadarMap` at
    // texture register 0 with clamped addressing and `FadeBorder` at index 1,
    // and names no texture file at all. Sampler 0 is the widget's own texture
    // slot, which is why `error.tga` reaches it and why the icon already takes
    // exactly the texture path every other HUD texture takes (the bridge's VFS
    // loader standing in for the renderer's vtable +64h, 00aa5e60). What the
    // running game puts there instead is the mission's own radar map, which the
    // minimap screen 005bec50 binds after taking the widget by name at 005bed21
    // through 00aa7e00, and which the renderer owner produces. There is no
    // second texture for this process to load.
    impl.record("HudMinimap::radar_map_texture", 0x00aa5e60u);
    if (impl.minimap) impl.minimap->report();
    if (impl.markers) impl.markers->report();
    impl.log.note("minimap island-map icon: minimap_terrain.mshd is the two-sampler GUI "
        "shader effect shaderfx/gui/minimap_terrain.shfx (RadarMap at register 0, "
        "FadeBorder at index 1) and names no texture, so error.tga is sampler 0's authored "
        "value and the icon is already on the same texture path as every other HUD texture; "
        "what replaces it is the renderer owner's radar map");
}

bool GameHudHost::manager_active() const noexcept {
    return impl_->summary.manager_built && impl_->manager.base.active;
}

void GameHudHost::update_in_game_interface_0068c1f0() {
    Impl& impl = *impl_;
    if (!manager_active()) return;
    HudUpdateBinding binding(impl);
    bsp::update_in_game_interface_0068c1f0(impl.manager, impl.update_state, binding);
    impl.done("InGameInterface::update", 0x0068c1f0u);
    ++impl.summary.update_frames;
}

}  // namespace bsp::game
