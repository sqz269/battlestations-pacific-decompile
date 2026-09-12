// bsp_game.exe milestone 2e: from the main menu to the mission load request.
// See include/bsp/game_hosts_mission.hpp for the address list and the evidence.
#include "bsp/game_hosts_mission.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_mission_frame.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/lua_object.hpp"
#include "bsp/lua_state_owner.hpp"
#include "bsp/main_menu_mission_detail.hpp"
#include "bsp/main_menu_screen.hpp"
#include "bsp/main_menu_screens.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/mission_briefing_start.hpp"
#include "bsp/mission_load_path.hpp"
#include "bsp/mission_lua_machine.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/mission_tree_data.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/vfs_mounts.hpp"

#include <algorithm>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// The locale text of a menu label id, folded to ASCII for the log exactly as
// milestone 2d folds a resolved Text run.
std::string ascii_fold(const std::u16string& text) {
    std::string out;
    out.reserve(text.size());
    for (const char16_t unit : text) {
        out.push_back(unit >= 0x20 && unit < 0x7F ? static_cast<char>(unit) : '?');
    }
    return out;
}

// The per-class tally of one parsed scene, in the shape src/mission_scene_probe.cpp
// reports it. Nothing of this reaches the scene record; it is the entity walk
// passes 2 and 3 would perform, run here only to say what the file holds.
void count_entities(const std::vector<SceneEntity>& entities,
    std::map<std::string, std::size_t>& out, std::size_t& total) {
    for (const SceneEntity& entity : entities) {
        ++out[entity.class_name];
        ++total;
        count_entities(entity.children, out, total);
    }
}

// ---------------------------------------------------------------------------
// MissionTreeLuaView over a live Lua 5.1.1 state
// ---------------------------------------------------------------------------

// The six reader virtuals 005C6A70 and its siblings drive (00BD6830, 00BD68D0,
// 00BD5EB0, 00BD5F50) reach the interpreter through the same thin accessors the
// GUI reader uses, so this view is the same shape as GuiLuaReader's: a stack of
// references with the entered table on top. Nothing here is a reconstruction;
// it is the bridge between the reconstructed readers and stock Lua.
class LiveMissionTreeView final : public MissionTreeLuaView {
public:
    LiveMissionTreeView(GuiLuaHost& host, GuiLuaRef root) : host_(host) {
        stack_.push_back(root);
    }
    ~LiveMissionTreeView() override {
        // The root belongs to the caller; every reference this view pushed is
        // released here, which is the native reader's own leave discipline.
        while (stack_.size() > 1) {
            release(stack_.back());
            stack_.pop_back();
        }
    }
    LiveMissionTreeView(const LiveMissionTreeView&) = delete;
    LiveMissionTreeView& operator=(const LiveMissionTreeView&) = delete;

    void enter_by_name(std::string_view key) override {
        stack_.push_back(child(std::string(key).c_str()));
    }
    void enter_by_index(std::int32_t index) override {
        stack_.push_back(host_.get_by_index(stack_.back(), index));
    }
    void leave() override {
        if (stack_.size() <= 1) return;
        release(stack_.back());
        stack_.pop_back();
    }

    bool has_name(std::string_view key) override {
        const GuiLuaRef value = child(std::string(key).c_str());
        const bool present = is_value(host_.type_of(value));
        release(value);
        return present;
    }
    bool has_index(std::int32_t index) override {
        if (host_.type_of(stack_.back()) != GuiLuaType::Table) return false;
        const GuiLuaRef value = host_.get_by_index(stack_.back(), index);
        const bool present = is_value(host_.type_of(value));
        release(value);
        return present;
    }

    std::string read_string(std::string_view key, std::string_view fallback) override {
        const GuiLuaRef value = child(std::string(key).c_str());
        std::string out(fallback);
        if (host_.type_of(value) == GuiLuaType::String) {
            const char* text = host_.to_string(value);
            if (text != nullptr) out.assign(text);
        }
        release(value);
        return out;
    }
    std::int32_t read_int(std::string_view key, std::int32_t fallback) override {
        const GuiLuaRef value = child(std::string(key).c_str());
        std::int32_t out = fallback;
        if (host_.type_of(value) == GuiLuaType::Number) {
            out = static_cast<std::int32_t>(host_.to_number(value));
        }
        release(value);
        return out;
    }
    bool read_bool(std::string_view key, bool fallback) override {
        const GuiLuaRef value = child(std::string(key).c_str());
        bool out = fallback;
        const GuiLuaType type = host_.type_of(value);
        if (type == GuiLuaType::Boolean) {
            out = host_.to_boolean(value);
        } else if (type == GuiLuaType::Number) {
            out = host_.to_number(value) != 0.0;
        }
        release(value);
        return out;
    }
    std::array<float, 3> read_vec3(std::string_view key,
        const std::array<float, 3>& fallback) override {
        const GuiLuaRef value = child(std::string(key).c_str());
        std::array<float, 3> out = fallback;
        if (host_.type_of(value) == GuiLuaType::Table) {
            for (std::int32_t component = 0; component < 3; ++component) {
                const GuiLuaRef element = host_.get_by_index(value, component + 1);
                if (host_.type_of(element) == GuiLuaType::Number) {
                    out[static_cast<std::size_t>(component)] =
                        static_cast<float>(host_.to_number(element));
                }
                release(element);
            }
        }
        release(value);
        return out;
    }

    std::vector<std::string> read_string_array(std::string_view key) override {
        std::vector<std::string> out;
        const GuiLuaRef table = child(std::string(key).c_str());
        if (host_.type_of(table) == GuiLuaType::Table) {
            for (std::int32_t index = 1;; ++index) {
                const GuiLuaRef element = host_.get_by_index(table, index);
                const GuiLuaType type = host_.type_of(element);
                if (type == GuiLuaType::Nil) {
                    release(element);
                    break;
                }
                if (type == GuiLuaType::String) {
                    const char* text = host_.to_string(element);
                    out.emplace_back(text != nullptr ? text : "");
                } else {
                    out.emplace_back();
                }
                release(element);
            }
        }
        release(table);
        return out;
    }
    std::vector<std::int32_t> read_int_array(std::string_view key) override {
        std::vector<std::int32_t> out;
        const GuiLuaRef table = child(std::string(key).c_str());
        if (host_.type_of(table) == GuiLuaType::Table) {
            for (std::int32_t index = 1;; ++index) {
                const GuiLuaRef element = host_.get_by_index(table, index);
                const GuiLuaType type = host_.type_of(element);
                if (type == GuiLuaType::Nil) {
                    release(element);
                    break;
                }
                out.push_back(type == GuiLuaType::Number
                        ? static_cast<std::int32_t>(host_.to_number(element))
                        : 0);
                release(element);
            }
        }
        release(table);
        return out;
    }

    std::vector<std::string> string_keys() override {
        // 00BD5F50 walks the table with lua_next and keeps the string keys, at
        // most kGuiLuaKeyArrayCapacity of them.
        std::vector<std::string> out;
        if (host_.type_of(stack_.back()) != GuiLuaType::Table) return out;
        GuiLuaRef key{};
        GuiLuaRef value{};
        bool restart = true;
        while (host_.next(stack_.back(), key, value, restart)) {
            restart = false;
            if (host_.type_of(key) == GuiLuaType::String) {
                const char* text = host_.to_string(key);
                if (text != nullptr) out.emplace_back(text);
            }
            if (out.size() >= kGuiLuaKeyArrayCapacity) break;
        }
        return out;
    }

private:
    // 00BD5EB0 reports presence from lua_type: an unbound reference and a nil
    // are both absent, which is what the guarded reads test.
    static bool is_value(GuiLuaType type) noexcept {
        return type != GuiLuaType::Nil && type != GuiLuaType::None;
    }
    GuiLuaRef child(const char* key) {
        if (host_.type_of(stack_.back()) != GuiLuaType::Table) return GuiLuaRef{};
        return host_.get_by_name(stack_.back(), key);
    }
    void release(const GuiLuaRef& reference) {
        if (reference.valid()) host_.release(reference);
    }

    GuiLuaHost& host_;
    std::vector<GuiLuaRef> stack_;
};

}  // namespace

const char* game_mission_step_name(GameMissionStep step) noexcept {
    switch (step) {
    case GameMissionStep::Idle: return "Idle";
    case GameMissionStep::PublishList: return "PublishList";
    case GameMissionStep::MissionList: return "MissionList";
    case GameMissionStep::DetailPage: return "DetailPage";
    case GameMissionStep::BriefingStart: return "BriefingStart";
    case GameMissionStep::LoadRequested: return "LoadRequested";
    case GameMissionStep::SceneRecord: return "SceneRecord";
    case GameMissionStep::SceneLoaded: return "SceneLoaded";
    case GameMissionStep::InMission: return "InMission";
    case GameMissionStep::MissionFrames: return "MissionFrames";
    case GameMissionStep::Stopped: return "Stopped";
    }
    return "Idle";
}

// ---------------------------------------------------------------------------
// GameMissionHost::Impl
// ---------------------------------------------------------------------------

struct GameMissionHost::Impl {
    GameHostLog& log;
    GameVfsHost& vfs;
    GameScriptHost& scripts;
    GameFrontendHost& frontend;
    LocaleTables& locale;
    std::string requested_id;

    // 005CAAF0's tables, held for the run exactly as the mission-tree screen
    // holds them at +14h..+30h.
    MissionTreeTables tables;

    // 005861B0's three page roots and the widget handles the detail page drives.
    GuiLayoutPage* main_page{nullptr};
    GuiLayoutPage* worldmap_page{nullptr};
    GuiLayoutPage* briefing_grid_page{nullptr};
    GuiLayoutPage* briefing_page{nullptr};
    std::map<std::uint16_t, GuiLayoutWidget*> widgets;  // screen field -> widget
    GuiLayoutWidget* active_group{nullptr};             // +110h
    GuiLayoutWidget* selected_map_point{nullptr};       // +330h
    std::array<float, 3> backdrop{0.0f, 0.0f, 0.0f};    // +124h
    // +134h + index*10h, the five point vectors.
    std::array<std::vector<std::array<float, 3>>, 5> point_lists{};

    // The main-menu screen's own mutable state and the page word at 00E08874.
    MainMenuScreenState screen_state{};
    MainMenuPage page{MainMenuPage::TopLevel};
    std::uint32_t published_group{0};    // 00E194D8
    std::uint32_t published_mission{0};  // 00E194DC
    std::int32_t sub_selection{-1};      // 00E08878

    // The scripted accept edge: the frame the page machine is allowed to see
    // one fire of 004D92B0 on.
    int pending_action{0};

    // The load path's own state, game+5F0h and game+5D8h.
    MissionLoadPathState load{};

    GameMissionStep step{GameMissionStep::Idle};
    GameMissionSummary summary;

    // Milestone 2f: the mission Lua machine 004dd627 builds once per process
    // and the host that carries the load past the renderer owners, enters the
    // mission state and runs the headless frames.
    long mission_frames{0};
    // Milestone 2g, --mission-complete-frame N: the in-mission frame on which
    // the executable makes the call a script's end-movie binding makes.
    long mission_complete_frame{-1};
    // Milestone 2i: the player order --order issues on --order-frame, and the
    // fixed in-mission frame delta --mission-frame-seconds asks for.
    long order_frame{-1};
    float order_throttle{0.0f};
    float order_rudder{0.0f};
    float mission_frame_seconds{0.0f};
    GameFrameProfiler* profiler{nullptr};
    std::string language;
    std::unique_ptr<GameMissionLuaHost> lua;
    std::unique_ptr<GameMissionFrameHost> frame_host;
    GameHudHost* hud{nullptr};  // milestone 2h, owned by GameMenuHost

    Impl(GameHostLog& log_in, GameVfsHost& vfs_in, GameScriptHost& scripts_in,
        GameFrontendHost& frontend_in, LocaleTables& locale_in, std::string requested,
        long mission_frames_in, GameFrameProfiler* profiler_in, std::string language_in)
        : log(log_in), vfs(vfs_in), scripts(scripts_in), frontend(frontend_in),
          locale(locale_in), requested_id(std::move(requested)),
          mission_frames(mission_frames_in), profiler(profiler_in),
          language(std::move(language_in)) {
        summary.requested_id = requested_id;
        summary.mission_frames_requested = mission_frames;
    }

    const MissionRecordData* selected_record() const {
        return selected_mission_005806a0(tables, published_group, published_mission);
    }
    GuiLayoutWidget* widget(std::uint16_t field) {
        const auto found = widgets.find(field);
        return found != widgets.end() ? found->second : nullptr;
    }
    void show(std::uint16_t field, bool visible) {
        GuiLayoutWidget* node = widget(field);
        if (node == nullptr) return;
        frontend.set_widget_visible(*node, visible);
    }

    void publish_selection_00580940();
    void run_page_machine(float seconds);
    void build_detail_page_0058c010();
    void start_briefing_005922f0();
    void consume_load_request();
    void read_scene_file(SceneRecord& record, const std::string& scene_path);
    void finish_scene_load();
    void publish_frame_summary();
};

namespace {

// ---------------------------------------------------------------------------
// 005CAAF0's MissionTreeScriptHost
// ---------------------------------------------------------------------------

class MissionTreeScriptBinding final : public MissionTreeScriptHost {
public:
    explicit MissionTreeScriptBinding(GameMissionHost::Impl& owner) : owner_(owner) {}
    ~MissionTreeScriptBinding() override { close_reader(); }
    MissionTreeScriptBinding(const MissionTreeScriptBinding&) = delete;
    MissionTreeScriptBinding& operator=(const MissionTreeScriptBinding&) = delete;

    // 00B66BD0 then 00B6A020(mask). The repository's stock Lua 5.1.1 stands in
    // for the native interpreter, as the decal table already does.
    void open_state(std::uint32_t library_mask) override {
        owner_.log.implemented("MissionTree::construct_lua_state_owner", "00b66bd0");
        owner_ptr_ = std::make_unique<PcStorageLuaOwner>(
            owner_.scripts.files().owner_environment(owner_.scripts.runtime(),
                owner_.scripts.globals()));
        owner_ptr_->open_storage_archive_00b6a020(library_mask);
        owner_.log.implemented("MissionTree::open_lua_libraries", "00b6a020");
    }

    // 00B69D40(path, 0): the file and every VFS override of it, unprotected.
    void run_script(std::string_view path) override {
        script_path_.assign(path);
        owner_.scripts.runtime().run_file(owner_ptr_->storage_lua_38(), script_path_);
        owner_.summary.tree_script_ran = true;
        owner_.log.implemented("MissionTree::run_script", "00b69d40");
    }

    // 00B67980 then 00B67800, and the reader 004425C0 over the result.
    MissionTreeLuaView& open_table(std::string_view global_name) override {
        reader_ = std::make_unique<GuiLua51Host>(*owner_ptr_->storage_lua_38());
        root_ = lua_global_by_name_00b67980(*reader_, std::string(global_name).c_str());
        if (reader_->type_of(root_) != GuiLuaType::Table) {
            throw std::runtime_error("the mission tree global is not a table");
        }
        view_ = std::make_unique<LiveMissionTreeView>(*reader_, root_);
        owner_.log.implemented("MissionTree::open_table", "00b67800");
        return *view_;
    }

    // 0057BEC0, once per group entry. The loading screen is another owner's.
    void report_progress(float fraction) override {
        static_cast<void>(fraction);
        owner_.log.unimplemented("MissionTree::report_progress", "0057bec0");
    }

    // 00586150, the mission id the shell wants selected. --menu-select is the
    // only writer this process has.
    std::string requested_mission_id() override {
        owner_.log.implemented("MissionTree::requested_mission_id", "00586150");
        return owner_.requested_id;
    }

    void close_state() override {
        close_reader();
        owner_ptr_.reset();
        owner_.log.implemented("MissionTree::close_lua_state", "00b669a0");
    }

private:
    void close_reader() {
        // The view holds references into the reader and the reader holds
        // registry references into the state, so they retire in that order.
        view_.reset();
        if (root_.valid() && reader_) reader_->release(root_);
        root_ = GuiLuaRef{};
        reader_.reset();
    }

    GameMissionHost::Impl& owner_;
    std::unique_ptr<PcStorageLuaOwner> owner_ptr_;
    std::unique_ptr<GuiLua51Host> reader_;
    std::unique_ptr<LiveMissionTreeView> view_;
    GuiLuaRef root_{};
    std::string script_path_;
};

// ---------------------------------------------------------------------------
// 0058C010's MissionDetailHost
// ---------------------------------------------------------------------------

class MissionDetailBinding final : public MissionDetailHost {
public:
    explicit MissionDetailBinding(GameMissionHost::Impl& owner) : owner_(owner) {}

    void request_page_audio(std::string_view mission_name) override {
        static_cast<void>(mission_name);
        owner_.log.unimplemented("MissionDetail::request_page_audio", "00518d60");
    }
    void set_background_icon_state(int state, int reserved, float blend) override {
        static_cast<void>(state);
        static_cast<void>(reserved);
        static_cast<void>(blend);
        owner_.log.unimplemented("MissionDetail::set_background_icon_state", "0058c04c");
    }
    int read_tree_selected_group() override {
        owner_.log.implemented("MissionDetail::read_tree_selected_group", "0058c066");
        return static_cast<int>(owner_.tables.selection.group);
    }
    int read_tree_selected_mission_index() override {
        owner_.log.implemented("MissionDetail::read_tree_selected_mission_index", "005c3850");
        return static_cast<int>(owner_.tables.selection.mission);
    }
    void publish_selection(int group, int mission_index) override {
        owner_.published_group = static_cast<std::uint32_t>(group);
        owner_.published_mission = static_cast<std::uint32_t>(mission_index);
        owner_.log.implemented("MissionDetail::publish_selection", "0058c071");
    }
    void set_active_group_widget(MissionGroup group) override {
        const MissionGroupBinding& binding = mission_group_binding(group);
        owner_.active_group = owner_.widget(binding.handle_offset);
        owner_.log.implemented("MissionDetail::set_active_group_widget", "0058c0aa");
    }
    void set_campaign_flags(MissionDetailCampaignFlags flags) override {
        owner_.screen_state.us_campaign = flags.us_campaign;
        owner_.screen_state.dlc_campaign = flags.dlc_campaign;
        owner_.log.implemented("MissionDetail::set_campaign_flags", "0058c0b4");
    }
    void bind_selected_map_point(std::string_view widget_name) override {
        owner_.selected_map_point = find_in_group(widget_name);
        owner_.log.implemented("MissionDetail::bind_selected_map_point", "0058c18b");
    }
    std::size_t map_point_list_size(MissionGroup group) override {
        return owner_.point_lists[index_of(group)].size();
    }
    void append_map_point(MissionGroup group, std::array<float, 3> point) override {
        owner_.point_lists[index_of(group)].push_back(point);
        owner_.log.implemented("MissionDetail::append_map_point", "004215d0");
    }
    std::string_view selected_mission_name() override {
        const MissionRecordData* record = owner_.selected_record();
        return record != nullptr ? std::string_view(record->screen.title) : std::string_view{};
    }
    std::string_view selected_background_key() override {
        const MissionRecordData* record = owner_.selected_record();
        return record != nullptr ? std::string_view(record->extra.background)
                                 : std::string_view{};
    }
    std::string_view selected_background_movie_key() override {
        const MissionRecordData* record = owner_.selected_record();
        return record != nullptr ? std::string_view(record->extra.background_movie)
                                 : std::string_view{};
    }
    std::string_view selected_background_voice_key() override {
        const MissionRecordData* record = owner_.selected_record();
        return record != nullptr ? std::string_view(record->extra.background_voice)
                                 : std::string_view{};
    }
    std::size_t group_mission_count() override {
        const MissionGroupData* group = group_record();
        return group != nullptr ? group->missions.size() : 0;
    }
    bool mission_is_side_mission(std::size_t mission) override {
        const MissionGroupData* group = group_record();
        if (group == nullptr || mission >= group->missions.size()) return false;
        return group->missions[mission].extra.side_mission;
    }
    bool map_flag_visible(std::size_t mission) override {
        // 005C2F70 on the record. Its body is not reconstructed, so the flag is
        // shown for every mission of the group and the decision is recorded.
        static_cast<void>(mission);
        owner_.log.unimplemented("MissionDetail::map_flag_visible", "005c2f70");
        return true;
    }
    void set_map_flag(std::string_view widget_name, bool visible, float level) override {
        static_cast<void>(level);
        GuiLayoutWidget* node = find_in_group(widget_name);
        if (node == nullptr) {
            ++missing_;
            return;
        }
        owner_.frontend.set_widget_visible(*node, visible);
        ++flags_;
        // The flag's virtual +4Ch takes the 1.0/0.4 level, which is a renderer
        // blend the executable cannot apply; the icon is shown or hidden only.
        owner_.log.unimplemented("MissionDetail::set_map_flag_level", "0058c482");
    }
    void set_map_point(std::string_view widget_name, int state, int reserved,
        float blend) override {
        static_cast<void>(state);
        static_cast<void>(reserved);
        static_cast<void>(blend);
        GuiLayoutWidget* node = find_in_group(widget_name);
        if (node == nullptr) {
            ++missing_;
            return;
        }
        owner_.frontend.set_widget_visible(*node, true);
        ++points_;
        owner_.log.unimplemented("MissionDetail::set_map_point_state", "0058c62e");
    }
    std::array<float, 3> map_point_position(std::string_view widget_name) override {
        GuiLayoutWidget* node = find_in_group(widget_name);
        if (node == nullptr) return {0.0f, 0.0f, 0.0f};
        const GuiWidgetPoint point = resolved_position(node->transform);
        owner_.log.implemented("MissionDetail::map_point_position", "00aa6750");
        return {point.x, point.y, point.z};
    }
    std::array<float, 3> backdrop_position() override { return owner_.backdrop; }
    void set_widget_visible(std::uint16_t field, bool visible) override {
        owner_.show(field, visible);
        owner_.log.implemented("MissionDetail::set_widget_visible", "0058c6e0");
    }
    void commit_page_state() override {
        owner_.log.unimplemented("MissionDetail::commit_page_state", "00583e50");
    }
    void set_briefing_text(std::string_view background_key) override {
        GuiLayoutWidget* node = owner_.widget(kMissionDetailBriefingTextField);
        briefing_key_.assign(background_key);
        if (node == nullptr) {
            owner_.log.unimplemented("MissionDetail::set_briefing_text", "00abaed0");
            return;
        }
        // 00ABAED0 with the localise flag, exactly as a page-authored
        // DefaultText reaches the same setter. The bridge's text path then runs
        // 00A9FAD0 over the key the record carried.
        owner_.frontend.set_widget_text_source(*node, briefing_key_);
        owner_.log.implemented("MissionDetail::set_briefing_text", "00abaed0");
    }
    void reset_scroller() override {
        owner_.log.unimplemented("MissionDetail::reset_scroller", "00683790");
    }
    double briefing_text_height() override {
        owner_.log.unimplemented("MissionDetail::briefing_text_height", "00ab6bd0");
        return 0.0;
    }
    float text_clip_height() override {
        GuiLayoutWidget* node = owner_.widget(kMissionDetailTextClipField);
        owner_.log.implemented("MissionDetail::text_clip_height", "00aa6740");
        return node != nullptr ? node->transform.size.height : 0.0f;
    }
    void set_scroll_range(double range) override {
        static_cast<void>(range);
        owner_.log.unimplemented("MissionDetail::set_scroll_range", "006834a0");
    }
    void start_preview_movie(std::string_view movie_key) override {
        static_cast<void>(movie_key);
        owner_.log.unimplemented("MissionDetail::start_preview_movie", "0058c8a0");
    }
    bool page_audio_suppressed() override { return false; }  // screen+570h
    std::string_view audio_language_folder() override {
        owner_.log.unimplemented("MissionDetail::audio_language_folder", "008d57a0");
        return std::string_view{};
    }
    void request_streamed_dialog(std::string_view path, std::uint32_t callback) override {
        static_cast<void>(path);
        static_cast<void>(callback);
        owner_.log.unimplemented("MissionDetail::request_streamed_dialog", "0058c9be");
    }
    void set_page(MainMenuPage page) override {
        owner_.page = page;
        owner_.log.implemented("MissionDetail::set_page", "0058cac3");
    }
    void set_footer_commands(const std::array<MissionDetailCommand, 2>& commands) override {
        static_cast<void>(commands);
        owner_.log.unimplemented("MissionDetail::set_footer_commands", "0054b530");
    }
    void clear_help_line() override {
        owner_.log.unimplemented("MissionDetail::clear_help_line", "0054a0c0");
    }
    void clear_list_box() override {
        owner_.log.unimplemented("MissionDetail::clear_list_box", "00a9bec0");
    }
    void set_list_box_flags(bool a, bool b) override {
        static_cast<void>(a);
        static_cast<void>(b);
        owner_.log.unimplemented("MissionDetail::set_list_box_flags", "0058cd9e");
    }
    std::array<float, 3> list_box_position() override { return {0.0f, 0.0f, 0.0f}; }
    void move_list_box(std::array<float, 3> position) override {
        static_cast<void>(position);
        owner_.log.unimplemented("MissionDetail::move_list_box", "00aa8240");
    }
    void finish_list_box() override {
        owner_.log.unimplemented("MissionDetail::finish_list_box", "00aa6bc0");
    }

    std::size_t flags() const noexcept { return flags_; }
    std::size_t points() const noexcept { return points_; }
    std::size_t missing() const noexcept { return missing_; }
    const std::string& briefing_key() const noexcept { return briefing_key_; }

private:
    static std::size_t index_of(MissionGroup group) {
        const std::size_t index = static_cast<std::size_t>(group);
        return index < 5 ? index : 0;
    }
    const MissionGroupData* group_record() const {
        // 00580650 returns the mission-tree group record for the published
        // group, which is the same vector 005CAAF0 filled.
        if (owner_.published_group >= owner_.tables.groups.size()) return nullptr;
        return &owner_.tables.groups[owner_.published_group];
    }
    GuiLayoutWidget* find_in_group(std::string_view name) {
        if (owner_.active_group == nullptr) return nullptr;
        // 00AA7E00 walks direct children only; the map icons sit one level
        // under the group, so the direct lookup is what the native performs.
        const std::string key(name);
        GuiLayoutWidget* found = find_child_by_name_00aa7e00(*owner_.active_group, key);
        if (found == nullptr) found = find_descendant_by_name(*owner_.active_group, key);
        return found;
    }

    GameMissionHost::Impl& owner_;
    std::size_t flags_{0};
    std::size_t points_{0};
    std::size_t missing_{0};
    std::string briefing_key_;
};

// ---------------------------------------------------------------------------
// 00599DB0's MainMenuScreenUpdateHost
// ---------------------------------------------------------------------------

class MenuUpdateBinding final : public MainMenuScreenUpdateHost {
public:
    explicit MenuUpdateBinding(GameMissionHost::Impl& owner) : owner_(owner) {}

    bool text_owner_has_queue() override { return false; }        // screen+10h
    void dispatch_text_events(float seconds) override {
        static_cast<void>(seconds);
        owner_.log.unimplemented("MainMenuUpdate::dispatch_text_events", "00a96f40");
    }
    bool content_set_dirty() override { return false; }           // 00e0887c
    void clear_content_set_dirty() override {}
    bool content_manager_present() override { return false; }     // 00f8a304
    int content_entry_count() override { return 0; }
    bool content_entry_damaged(int index) override {
        static_cast<void>(index);
        return false;
    }
    void show_message(std::string_view key) override {
        static_cast<void>(key);
        owner_.log.unimplemented("MainMenuUpdate::show_message", "00531b00");
    }
    bool content_download_ready() override { return false; }      // 00f8abe8+30h
    void clear_content_download_ready() override {}
    bool menu_command_screen_idle() override { return true; }
    bool has_selected_user() override { return false; }
    int selected_sign_in_state() override { return 0; }
    bool content_manager_ready() override { return false; }
    void content_manager_commit() override {}
    // 00F8A2FC is null in this process, so the arm the async-text manager owns
    // is the one 0059A028 selects; page 0Ah is never the page here.
    bool async_text_manager_present() override { return false; }
    bool async_text_pending() override { return false; }
    void set_async_text_pending(bool pending) override { static_cast<void>(pending); }
    bool async_text_busy() override { return false; }
    int async_text_status() override { return 0; }
    void async_text_read_title(std::uint16_t dst_offset) override {
        static_cast<void>(dst_offset);
    }
    void async_text_submit(std::uint16_t a, std::uint16_t b) override {
        static_cast<void>(a);
        static_cast<void>(b);
    }
    void rebuild_top_level_after_text(bool flag) override { static_cast<void>(flag); }
    void request_game_state(int request) override {
        static_cast<void>(request);
        owner_.log.unimplemented("MainMenuUpdate::request_game_state", "004d7920");
    }
    void set_page(MainMenuPage page) override { owner_.page = page; }

    MainMenuPage page() override { return owner_.page; }
    bool action_fired(int action) override {
        // 004D92B0 over the front-end action record table. This process has no
        // input device bound to the menu, so the scripted edge stands in: the
        // action the step machine armed fires once and is consumed here.
        owner_.log.implemented("MainMenuUpdate::action_fired", "004d92b0");
        if (owner_.pending_action == 0 || owner_.pending_action != action) return false;
        owner_.pending_action = 0;
        return true;
    }
    void build_top_level_page(bool select_first) override {
        static_cast<void>(select_first);
        owner_.log.unimplemented("MainMenuUpdate::build_top_level_page", "00584ae0");
    }
    void build_single_player_page() override {
        owner_.log.unimplemented("MainMenuUpdate::build_single_player_page", "00584f50");
    }
    void build_mission_detail_page() override {
        owner_.log.unimplemented("MainMenuUpdate::build_mission_detail_page", "0058c010");
    }
    void mission_detail_accept() override {
        owner_.log.unimplemented("MainMenuUpdate::mission_detail_accept", "00594bf0");
    }
    void mission_detail_back() override {
        owner_.log.unimplemented("MainMenuUpdate::mission_detail_back", "00599340");
    }
    void open_tactical_library_with_selection() override {
        owner_.log.unimplemented("MainMenuUpdate::open_tactical_library", "005885d0");
    }
    void open_tactical_library_mode2() override {
        owner_.log.unimplemented("MainMenuUpdate::open_tactical_library_mode2", "005886c0");
    }
    void set_active_mission_group(MissionGroup group) override {
        const MissionGroupBinding& binding = mission_group_binding(group);
        owner_.active_group = owner_.widget(binding.handle_offset);
        owner_.log.implemented("MainMenuUpdate::set_active_mission_group", "0059a4b4");
    }
    void position_backdrop() override {
        owner_.log.unimplemented("MainMenuUpdate::position_backdrop", "00aa8240");
    }
    void drive_map(float seconds, MissionGroup group, float zoom_delta, bool moved) override {
        static_cast<void>(seconds);
        static_cast<void>(group);
        static_cast<void>(zoom_delta);
        static_cast<void>(moved);
        // 00588C70's own body is the map anchor and the point-list walk, which
        // needs the world-map camera this process does not build.
        owner_.log.unimplemented("MainMenuUpdate::drive_map", "00588c70");
    }
    MapZoomInput map_zoom_input() override {
        // Records 121h and 122h have no entry in this process's action table.
        owner_.log.implemented("MainMenuUpdate::map_zoom_input", "0059a517");
        return MapZoomInput{};
    }
    void animate_detail_group() override {
        owner_.log.unimplemented("MainMenuUpdate::animate_detail_group", "00683820");
    }
    void animate_page_group(bool pad_axis_override_off) override {
        static_cast<void>(pad_axis_override_off);
        owner_.log.unimplemented("MainMenuUpdate::animate_page_group", "00683820");
    }
    void update_scene(int scene_id) override {
        static_cast<void>(scene_id);
        owner_.log.unimplemented("MainMenuUpdate::update_scene", "004c1e90");
    }
    float blend_map_offset(float current, float low, float high) override {
        static_cast<void>(low);
        static_cast<void>(high);
        owner_.log.unimplemented("MainMenuUpdate::blend_map_offset", "00414130");
        return current;
    }

private:
    GameMissionHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 00626930, 005922F0 and 0058BDF0
// ---------------------------------------------------------------------------

class MissionStatsBinding final : public MissionStatsResetHost {
public:
    explicit MissionStatsBinding(GameMissionHost::Impl& owner) : owner_(owner) {}
    void set_stats_mission_name(std::string_view name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("MissionStats::set_mission_name", "00e19798");
    }
    void set_stats_debriefing_text(std::string_view text) override {
        static_cast<void>(text);
        owner_.log.unimplemented("MissionStats::set_debriefing_text", "00e197a0");
    }
    void clear_stats_container(const MissionStatsContainer& container) override {
        static_cast<void>(container);
        owner_.log.unimplemented("MissionStats::clear_container", "00626930");
    }

private:
    GameMissionHost::Impl& owner_;
};

// 004E2770 / 004E1D70's SetPendingSceneHost. The header pass is concrete: it
// reads the installed `.scn` through the mounted VFS and runs the reconstructed
// reader over it.
class PendingSceneBinding final : public SetPendingSceneHost {
public:
    explicit PendingSceneBinding(GameMissionHost::Impl& owner) : owner_(owner) {}

    void destroy_scene_record(SceneRecord& record) override {
        static_cast<void>(record);
        owner_.log.implemented("SetPendingScene::destroy_scene_record", "004bf930");
    }
    void enter_file_block(const std::string& name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("SetPendingScene::enter_file_block", "00be0a30");
    }
    void leave_file_block() override {
        owner_.log.unimplemented("SetPendingScene::leave_file_block", "00bdcb30");
    }
    bool scene_file_exists(const std::string& scene_path) override {
        owner_.log.implemented("SetPendingScene::scene_file_exists", "0109ceec+8");
        return owner_.vfs.exists(scene_path);
    }
    void load_scene_header_pass(const std::string& scene_path,
        const std::string& weather_override, SceneRecord& record) override {
        static_cast<void>(weather_override);
        owner_.read_scene_file(record, scene_path);
    }
    void notify_award_tracker() override {
        owner_.log.unimplemented("SetPendingScene::notify_award_tracker", "0068ea00");
    }

private:
    GameMissionHost::Impl& owner_;
};

class MissionStartBinding final : public MissionStartHost {
public:
    MissionStartBinding(GameMissionHost::Impl& owner, PendingSceneBinding& scene,
        MissionStatsBinding& stats)
        : owner_(owner), scene_(scene), stats_(stats) {}

    void set_current_mission_key(std::string_view mission_id) override {
        static_cast<void>(mission_id);
        owner_.log.unimplemented("MissionStart::set_current_mission_key", "0058be40");
    }
    void set_pending_scene(std::string_view scene, std::string_view weather_override) override {
        // 004E2770, which clears the record list, builds the record through
        // 004E1D70's Create arm and selects index 0 with 004C6890.
        SelectSceneRecordInputs inputs{};
        inputs.session_mode = 0;   // game+1FE4h, single player
        inputs.script_slot = 0;    // game+614h
        inputs.script_slot_forced = false;
        const SetPendingSceneResult result = run_set_pending_scene_004e2770(
            owner_.load.records, std::string(scene), std::string(weather_override), inputs,
            scene_);
        owner_.load.selection = result.selection;
        owner_.load.have_pending_scene = result.selection.have_record;
        owner_.log.implemented("MissionStart::set_pending_scene", "004e2770");
        owner_.log.implemented("MissionStart::select_scene_record", "004c6890");
    }
    void publish_loading_config(const std::vector<std::string>& text,
        std::string_view mission_name, bool second_side_enabled) override {
        static_cast<void>(text);
        static_cast<void>(mission_name);
        static_cast<void>(second_side_enabled);
        owner_.log.unimplemented("MissionStart::publish_loading_config", "0057d060");
    }
    void set_mission_key_mirror(std::string_view mission_id) override {
        static_cast<void>(mission_id);
        owner_.log.unimplemented("MissionStart::set_mission_key_mirror", "0058befa");
    }
    void reset_mission_stats(const MissionRecordData& record) override {
        run_mission_stats_reset_00626930(record, stats_);
        owner_.log.implemented("MissionStart::reset_mission_stats", "00626930");
    }
    bool main_menu_flag_5c() override { return false; }        // screen+5Ch
    std::int32_t chosen_difficulty() override { return 0; }    // game+6B0h
    void set_effective_difficulty(std::int32_t value) override {
        static_cast<void>(value);
        owner_.log.unimplemented("MissionStart::set_effective_difficulty", "0058bf58");
    }
    bool checkpoint_differs(const MissionRecordData& record) override {
        static_cast<void>(record);
        owner_.log.unimplemented("MissionStart::checkpoint_differs", "007f8d60");
        return false;
    }
    void write_checkpoint() override {
        owner_.log.unimplemented("MissionStart::write_checkpoint", "00437c70");
    }
    void request_mission_start() override {
        // 00439020: 004D7920(game, 6) then 004D7920(game, 0Ah).
        std::size_t count = 0;
        const std::uint32_t* requests = mission_start_state_requests_00439020(count);
        for (std::size_t i = 0; i < count; ++i) owner_.load.requests.push_back(requests[i]);
        owner_.log.implemented("MissionStart::request_mission_start", "00439020");
    }
    void reset_front_end_timer() override {
        owner_.log.unimplemented("MissionStart::reset_front_end_timer", "00a92c40");
    }
    bool metrics_enabled() override { return false; }          // 00e1aed4
    bool non_campaign_session() override { return false; }     // game+1FE4h
    MainMenuPage current_page() override { return owner_.page; }
    void report_mission_start_metrics(int code, bool flag) override {
        static_cast<void>(code);
        static_cast<void>(flag);
        owner_.log.unimplemented("MissionStart::report_metrics", "00753810");
    }
    void finish_start() override {
        owner_.log.unimplemented("MissionStart::finish_start", "004d2a80");
    }

private:
    GameMissionHost::Impl& owner_;
    PendingSceneBinding& scene_;
    MissionStatsBinding& stats_;
};

class MissionPlayBinding final : public MissionBriefingPlayHost {
public:
    MissionPlayBinding(GameMissionHost::Impl& owner, MissionStartBinding& start,
        const MissionRecordData& record)
        : owner_(owner), start_(start), record_(record) {}

    void flush_award_tracker() override {
        owner_.log.unimplemented("MissionPlay::flush_award_tracker", "00690cd0");
    }
    void suspend_page_for_movie() override {
        owner_.log.unimplemented("MissionPlay::suspend_page_for_movie", "005830a0");
    }
    void stop_front_end_audio() override {
        owner_.log.unimplemented("MissionPlay::stop_front_end_audio", "00a85c00");
    }
    void apply_pending_interface() override {
        owner_.log.unimplemented("MissionPlay::apply_pending_interface", "0059234c");
    }
    void arm_movie_surface() override {
        owner_.log.unimplemented("MissionPlay::arm_movie_surface", "0059235b");
    }
    void commit_movie_visibility() override {
        owner_.log.unimplemented("MissionPlay::commit_movie_visibility", "004f83b0");
    }
    void enter_movie_surface() override {
        owner_.log.unimplemented("MissionPlay::enter_movie_surface", "0059236d");
    }
    void play_mission_movie(std::string_view movie_name) override {
        static_cast<void>(movie_name);
        owner_.log.unimplemented("MissionPlay::play_mission_movie", "004f8a20");
    }
    void mark_movie_active() override {
        owner_.log.unimplemented("MissionPlay::mark_movie_active", "0059238e");
    }
    void set_movie_completion(std::uint32_t completion_address) override {
        static_cast<void>(completion_address);
        owner_.log.unimplemented("MissionPlay::set_movie_completion", "004f8970");
    }
    void start_selected_mission() override {
        run_start_selected_mission_0058bdf0(record_, start_);
        owner_.log.implemented("MissionPlay::start_selected_mission", "0058bdf0");
    }
    void clear_command_bar() override {
        owner_.log.unimplemented("MissionPlay::clear_command_bar", "0054b530");
    }

private:
    GameMissionHost::Impl& owner_;
    MissionStartBinding& start_;
    const MissionRecordData& record_;
};

}  // namespace

// ---------------------------------------------------------------------------
// The steps
// ---------------------------------------------------------------------------

void GameMissionHost::Impl::publish_selection_00580940() {
    // 00580940, __cdecl void(). The inputs are the mission-tree screen's own
    // +10h/+0Ch (which 005CAAF0's tail resolved from --menu-select), the side
    // index 005C27E0 reads off the record, and the group-completed test
    // 005C3BE0. Nothing in this process completes a mission, so the progress
    // side of 005C3BE0 is the "not completed" answer for every mission.
    MainMenuSelectionInputs inputs{};
    inputs.non_campaign_session = false;  // game+1FE4h
    inputs.selected_group = tables.selection.group;
    inputs.mission_index = tables.selection.mission;
    const MissionRecordData* record = selected_mission_005806a0(tables,
        tables.selection.group, tables.selection.mission);
    inputs.side_index = record != nullptr ? mission_side_index_005c27e0(*record) : 0;
    if (tables.selection.group < tables.groups.size()) {
        const std::vector<bool> completed;
        inputs.group_completed = group_completed_005c3be0(
            tables.groups[tables.selection.group], completed);
        log.implemented("MainMenuSelect::group_completed", "005c3be0");
    }
    log.implemented("MainMenuSelect::side_index", "005c27e0");

    const MainMenuMissionSelection published =
        publish_main_menu_mission_selection_00580940(inputs);
    log.implemented("MainMenuSelect::publish_selection", "00580940");
    published_group = published.group_index;
    published_mission = published.mission_index;
    sub_selection = published.sub_selection;
    page = published.page;

    summary.published_group = static_cast<int>(published_group);
    summary.published_mission = static_cast<int>(published_mission);
    summary.list_page = static_cast<int>(page);
    log.notef("mission selection published: group=%u mission=%u side=%zu page=%d "
        "sub_selection=%d", published_group, published_mission, inputs.side_index,
        summary.list_page, sub_selection);
}

void GameMissionHost::Impl::run_page_machine(float seconds) {
    MenuUpdateBinding host(*this);
    const MainMenuUpdateResult result =
        run_main_menu_screen_update_00599db0(screen_state, host, seconds);
    log.implemented("MainMenuScreen::update", "00599db0");
    static const char* const arms[] = {"None", "TopLevel", "TacticalLibrary",
        "MissionDetail", "Objectives", "MissionList"};
    log.notef("main-menu update arm=%s page=%d scene=%d epilogue=%d",
        arms[static_cast<std::size_t>(result.arm)], static_cast<int>(page), result.scene_id,
        result.reached_epilogue ? 1 : 0);
}

void GameMissionHost::Impl::build_detail_page_0058c010() {
    MissionDetailBinding host(*this);
    const MissionDetailOutcome outcome = build_mission_detail_page_0058c010(host);
    log.implemented("MainMenuScreen::build_mission_detail_page", "0058c010");
    summary.detail_built = outcome.built;
    summary.detail_page = static_cast<int>(outcome.page);
    summary.detail_missions_visited = outcome.missions_visited;
    summary.detail_map_flags = host.flags();
    summary.detail_map_points = host.points();
    summary.detail_briefing_text = host.briefing_key();
    log.notef("mission detail page built=%d page=%d group=%d missions=%zu flags=%zu "
        "points=%zu missing_icons=%zu briefing_key=%s", outcome.built ? 1 : 0,
        summary.detail_page, outcome.published_group, outcome.missions_visited,
        host.flags(), host.points(), host.missing(),
        host.briefing_key().empty() ? "(none)" : host.briefing_key().c_str());
    frontend.invalidate_bridge();
}

void GameMissionHost::Impl::start_briefing_005922f0() {
    const MissionRecordData* record = selected_record();
    if (record == nullptr) {
        log.notef("no selected mission record; the briefing start has nothing to run");
        return;
    }
    MissionStatsBinding stats(*this);
    PendingSceneBinding scene(*this);
    MissionStartBinding start(*this, scene, stats);
    MissionPlayBinding play(*this, start, *record);
    const bool movie_arm = run_briefing_play_005922f0(*record, play);
    log.implemented("MainMenuScreen::play_selected_mission", "005922f0");
    summary.briefing_started = true;
    summary.movie_arm = movie_arm;
    summary.movie_name = record->extra.movie_name;
    summary.requests_queued = load.requests.size();
    summary.load_requested = !load.requests.empty();
    if (movie_arm) {
        // 005923A2 leaves the start to 0058D9D0, which the movie player calls
        // when the clip finishes. The movie player is another owner's, so the
        // executable runs the completion itself and says so.
        log.notef("briefing took the movie arm (%s); running the completion 0058d9d0 "
            "directly because the movie player is not bound",
            record->extra.movie_name.c_str());
        MissionStartBinding start_after(*this, scene, stats);
        run_briefing_movie_finished_0058d9d0(*record, start_after);
        log.implemented("MainMenuScreen::briefing_movie_finished", "0058d9d0");
        summary.requests_queued = load.requests.size();
        summary.load_requested = !load.requests.empty();
    }
}

void GameMissionHost::Impl::read_scene_file(SceneRecord& record,
    const std::string& scene_path) {
    // 0046DF00's header pass. 008D9CF0 opens the path through the VFS provider
    // table at 0109CEEC and reads the whole stream; the parser then runs over
    // the text. The scene-graph owner is needed by passes 2 and 3 (004D4DF0),
    // not by this one, which is a correction to the owner the load inventory
    // records for this step.
    summary.scene_path = scene_path;
    summary.scene_short_name = derive_scene_short_name(scene_path);
    const SceneDatabasePath split = split_scene_path_0046df00(scene_path);
    summary.scene_stem = split.stem;

    // 00BDF4C0 resolves the virtual name through the mounts and the search
    // groups, then 00BDF310 opens it into memory: the pair every other asset in
    // this process takes.
    std::string text;
    {
        VfsProviderManager* manager = vfs.manager();
        std::string resolved = scene_path;
        if (manager == nullptr
            || !resolve_existing_resource_00bdf4c0_fragment(manager->context(),
                vfs.search_registrations(), resolved)) {
            log.notef("scene %s not resolved through the mounts", scene_path.c_str());
            log.unimplemented("SetPendingScene::load_scene_header_pass", "0046df00");
            return;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(manager->context(),
            resolved, 2);
        log.implemented("SceneFileReader::read_scene_file", "008d9cf0");
        if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
            log.notef("scene %s not read: %s", scene_path.c_str(),
                opened.error.empty() ? "no provider returned a stream" : opened.error.c_str());
            log.unimplemented("SetPendingScene::load_scene_header_pass", "0046df00");
            return;
        }
        text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
            static_cast<std::size_t>(opened.stream->size_00bef600()));
    }
    summary.scene_file_read = true;
    summary.scene_bytes = text.size();

    const SceneDocument document = parse_scene_document(text);
    log.implemented("SetPendingScene::load_scene_header_pass", "0046df00");

    record.scene_path = scene_path;
    record.file_present = true;
    record.mission_id = document.header.unique_id;  // record+1098h
    summary.scene_mission_id = document.header.unique_id;
    summary.scene_groups = document.groups.size();

    // The entity walk belongs to passes 2 and 3; it is run here only to report
    // what the selected mission's scene holds, exactly as src/mission_scene_probe.cpp
    // does, and nothing of it reaches the record.
    std::map<std::string, std::size_t> classes;
    std::size_t total = 0;
    count_entities(document.entities, classes, total);
    summary.scene_entities = total;
    summary.scene_distinct_classes = classes.size();
    for (const auto& row : classes) {
        GameSceneClassCount entry;
        entry.name = row.first;
        entry.count = row.second;
        entry.registered = scene_entity_class_is_registered(row.first);
        if (entry.registered) {
            const int class_id = scene_entity_class_id_from_name(row.first);
            entry.registration_pass = scene_registration_pass_handles_class(class_id)
                || scene_registration_fallback_class(class_id);
        } else {
            summary.scene_unregistered += row.second;
        }
        summary.scene_classes.push_back(entry);
    }

    // The five numbered VFS blocks 004DFB70 would open around the load phases.
    const MissionLoadPhase phases[] = {MissionLoadPhase::WorldConstruction,
        MissionLoadPhase::MissionScript, MissionLoadPhase::StageInit,
        MissionLoadPhase::EngineMovie, MissionLoadPhase::RendererHandoff};
    for (const MissionLoadPhase phase : phases) {
        summary.scene_block_names.push_back(
            mission_load_phase_block_name(phase, summary.scene_short_name));
    }
    // 008860B0 resolves the record's script-table entry at +928h + slot*8, and
    // for single player normalise_mission_script_slot always answers slot 8.
    // That table is filled by the native header pass from data this reader does
    // not model, so the path below is the one the scene file's own base name
    // produces, which is what src/mission_scene_probe.cpp reports and what the
    // installation actually ships.
    const std::size_t separator = split.stem.find_last_of('/');
    const std::string script_name = separator == std::string::npos
        ? split.stem
        : split.stem.substr(separator + 1);
    summary.mission_script_path = mission_script_path(script_name);
    log.notef("scene %s: %zu bytes, uniqueID=%d, %zu entities in %zu classes, short name "
        "\"%s\", mission script \"%s\"", scene_path.c_str(), summary.scene_bytes,
        summary.scene_mission_id, summary.scene_entities, summary.scene_distinct_classes,
        summary.scene_short_name.c_str(), summary.mission_script_path.c_str());
    for (const GameSceneClassCount& row : summary.scene_classes) {
        log.notef("  scene class %-24s x%-5zu %s%s", row.name.c_str(), row.count,
            row.registered ? "registered" : "UNREGISTERED",
            row.registration_pass ? " registration-pass" : " instantiate-only");
    }
    std::string blocks;
    for (const std::string& block : summary.scene_block_names) {
        if (!blocks.empty()) blocks += ' ';
        blocks += block;
    }
    log.notef("  scene vfs blocks: %s", blocks.c_str());
}

void GameMissionHost::Impl::consume_load_request() {
    // 004E4430's drain. Request 6 is the interface change the front-end owner
    // services; request 0Ah dispatches 004DFB70, whose first phase needs the
    // renderer and the world.
    std::size_t serviced = 0;
    while (!load.requests.empty()) {
        const std::uint32_t request = load.requests.front();
        load.requests.erase(load.requests.begin());
        ++serviced;
        if (request == kMissionStartInterfaceRequest) {
            log.unimplemented("MissionLoad::apply_pending_interface", "00684600");
            continue;
        }
        if (request == kMissionSceneLoadRequest) {
            log.notef("state request 0Ah dequeued; 004dfb70 is the handler");
            break;
        }
    }
    static_cast<void>(serviced);

    summary.scene_record_built = load.selection.have_record;
    if (!load.records.empty()) {
        const SceneRecord& record = load.records.front();
        log.notef("scene record: path=%s present=%d mission_id=%d side_blocks=%zu "
            "slots_filled=%zu selected_index=%d", record.scene_path.c_str(),
            record.file_present ? 1 : 0, record.mission_id, record.side_blocks.size(),
            load.selection.slots_filled, load.selection.selected_index);
    }

    // Every host method the load needs, with its owner area. The run stops at
    // the first one whose owner is the renderer or the scene graph.
    std::size_t count = 0;
    const MissionLoadHostStep* steps = mission_load_host_steps(count);
    summary.load_host_steps = count;
    summary.load_host_external = mission_load_external_step_count();
    char label[192];
    for (std::size_t i = 0; i < count; ++i) {
        const MissionLoadHostStep& step_row = steps[i];
        if (step_row.owner == MissionLoadOwner::PureLogic) continue;
        std::snprintf(label, sizeof(label), "MissionLoad(%s)::%s",
            mission_load_owner_name(step_row.owner), step_row.method);
        char address[16];
        std::snprintf(address, sizeof(address), "%08lx",
            static_cast<unsigned long>(step_row.address));
        const bool renderer_or_scene = step_row.owner == MissionLoadOwner::Renderer
            || step_row.owner == MissionLoadOwner::SceneGraph;
        // The header pass is the one scene-graph step this process performs: it
        // needs the reader and the VFS only, and it already ran above.
        const bool header_pass = step_row.address == 0x0046df00u
            && std::string(step_row.host) == "SetPendingSceneHost";
        if (header_pass) continue;
        log.unimplemented(label, address);
        if (renderer_or_scene && summary.load_stopped_at.empty()) {
            summary.load_stopped_at = std::string(step_row.method) + " ["
                + address + "] " + mission_load_owner_name(step_row.owner);
        }
    }
    log.notef("mission load host inventory: %zu steps, %zu need a subsystem owner; the "
        "first renderer or scene-graph owner is %s (milestone 2e stopped there; 2f walks "
        "the rest)", summary.load_host_steps, summary.load_host_external,
        summary.load_stopped_at.empty() ? "(nothing)" : summary.load_stopped_at.c_str());
}

// ---------------------------------------------------------------------------
// GameMissionHost
// ---------------------------------------------------------------------------

GameMissionHost::GameMissionHost(GameHostLog& log, GameVfsHost& vfs, GameScriptHost& scripts,
    GameFrontendHost& frontend, LocaleTables& locale, std::string requested_mission_id,
    long mission_frames, GameFrameProfiler* profiler, std::string language,
    long mission_complete_frame, GameHudHost* hud, long order_frame, float order_throttle,
    float order_rudder, float mission_frame_seconds)
    : impl_(std::make_unique<Impl>(log, vfs, scripts, frontend, locale,
          std::move(requested_mission_id), mission_frames, profiler,
          std::move(language))) {
    // Milestone 2g, --mission-complete-frame N.
    impl_->mission_complete_frame = mission_complete_frame;
    // Milestone 2h: the in-mission HUD, owned by the front-end host because the
    // registry its screens land in is that object's.
    impl_->hud = hud;
    // Milestone 2i, --order-frame / --order and --mission-frame-seconds.
    impl_->order_frame = order_frame;
    impl_->order_throttle = order_throttle;
    impl_->order_rudder = order_rudder;
    impl_->mission_frame_seconds = mission_frame_seconds;
}

GameMissionHost::~GameMissionHost() = default;

bool GameMissionHost::requested() const noexcept { return !impl_->requested_id.empty(); }

const GameMissionSummary& GameMissionHost::summary() const noexcept {
    impl_->summary.step = impl_->step;
    return impl_->summary;
}

void GameMissionHost::load_mission_tree_005caaf0() {
    Impl& host = *impl_;
    MissionTreeScriptBinding binding(host);
    try {
        host.tables = bsp::load_mission_tree_005caaf0(binding);
        host.summary.tree_loaded = true;
    } catch (const std::exception& error) {
        host.summary.tree_error = error.what();
        host.log.notef("mission tree did not load: %s", error.what());
        host.log.unimplemented("MissionTreeScreen::load_tables", "005caaf0");
        return;
    }
    host.log.implemented("MissionTreeScreen::load_tables", "005caaf0");

    // A defect in the shared reconstruction, found here and not fixed here
    // because src/mission_tree_data.cpp belongs to another packet.
    // read_mission_record_005c6a70 fills MissionRecordData::sides, which is the
    // MissionSideBlockData pair, but every screen-facing consumer
    // (mission_side_index, mission_side_block, mission_opens_briefing,
    // mission_side_index_005c27e0 and run_start_selected_mission_0058bdf0)
    // reads MissionRecord::sides through record.screen, which the reader never
    // writes. A record loaded from the real table therefore answers every side
    // query from a default-constructed block: the side index comes out 1 for
    // every mission and the page pairing sends a United States mission to the
    // Japanese page. The executable copies the reader's blocks into the
    // screen-facing projection so the recovered rules see real data; the fix
    // belongs in read_mission_record_005c6a70.
    std::size_t bridged = 0;
    for (MissionGroupData& group : host.tables.groups) {
        for (MissionRecordData& record : group.missions) {
            for (std::size_t side = 0; side < kMissionSideBlockCount; ++side) {
                record.screen.sides[side] = record.sides[side].screen;
            }
            ++bridged;
        }
    }
    for (MissionRecordData& record : host.tables.multi) {
        for (std::size_t side = 0; side < kMissionSideBlockCount; ++side) {
            record.screen.sides[side] = record.sides[side].screen;
        }
        ++bridged;
    }
    host.log.notef("side blocks projected into the screen-facing record for %zu missions "
        "(read_mission_record_005c6a70 fills only MissionRecordData::sides)", bridged);

    host.summary.tree_groups = host.tables.groups.size();
    host.summary.tree_multi = host.tables.multi.size();
    for (const MissionGroupData& group : host.tables.groups) {
        host.summary.tree_missions += group.missions.size();
    }
    host.summary.selected_group = host.tables.selection.group;
    host.summary.selected_mission = host.tables.selection.mission;
    // 005CAAF0's tail clamps a miss to (0, 0), so the lookup is repeated here
    // to tell a hit from the clamp. The key is the record's Lua `id`, which for
    // the campaign missions is a short code such as USN02, not the scene file's
    // stem.
    const MissionTreeIndex found =
        find_mission_by_id_005c3470(host.tables.groups, host.requested_id);
    host.summary.requested_found = static_cast<std::int32_t>(found.mission) >= 0;
    const MissionRecordData* record = selected_mission_005806a0(host.tables,
        host.tables.selection.group, host.tables.selection.mission);
    if (record != nullptr) {
        host.summary.selected_id = record->screen.name;
        host.summary.selected_title = record->screen.title;
        host.summary.selected_scene = record->screen.scene;
    }
    if (!host.summary.requested_found) {
        host.log.notef("mission id %s is not in the tree; 005caaf0's tail clamps the "
            "selection to the first mission of the first group", host.requested_id.c_str());
    }
    host.log.notef("mission tree: %zu groups, %zu missions, %zu multiplayer entries",
        host.summary.tree_groups, host.summary.tree_missions, host.summary.tree_multi);
    for (std::size_t index = 0; index < host.tables.groups.size(); ++index) {
        const MissionGroupData& group = host.tables.groups[index];
        std::string ids;
        for (std::size_t i = 0; i < group.missions.size() && i < 4; ++i) {
            if (!ids.empty()) ids += ' ';
            ids += group.missions[i].screen.name;
        }
        host.log.notef("  group %zu %-24s missions=%zu ids: %s%s", index,
            group.extra.group_name.c_str(), group.missions.size(), ids.c_str(),
            group.missions.size() > 4 ? " ..." : "");
    }
    host.log.notef("mission tree selection: group=%u mission=%u id=%s name=%s scene=%s",
        host.summary.selected_group, host.summary.selected_mission,
        host.summary.selected_id.c_str(), host.summary.selected_title.c_str(),
        host.summary.selected_scene.c_str());
    if (record != nullptr) {
        // The two 154h side blocks at record+0B8h and +20Ch. 005C27E0 reads the
        // low byte of the first one, so the side index the page pairing uses is
        // decided here.
        host.log.notef("  side blocks: allied enabled=%u briefing=\"%s\" hints=%zu; "
            "japanese enabled=%u briefing=\"%s\" hints=%zu; movie=\"%s\" difficulty=%u",
            record->screen.sides[0].enabled, record->screen.sides[0].briefing_key.c_str(),
            record->screen.sides[0].loading_text.size(), record->screen.sides[1].enabled,
            record->screen.sides[1].briefing_key.c_str(),
            record->screen.sides[1].loading_text.size(),
            record->extra.movie_name.c_str(), record->screen.difficulty);
    }
}

void GameMissionHost::bind_main_menu_layout_005861b0(GuiLayoutPage* main_page) {
    Impl& host = *impl_;
    host.main_page = main_page;

    // 0058629C, 0058641A and 005864A0: the three page roots the screen keeps.
    struct PageBind {
        const char* name;
        std::uint16_t field;
        GuiLayoutPage** slot;
    };
    const PageBind pages[] = {
        {"FE_worldmap_historical", 0x2F0, &host.worldmap_page},
        {"FE_briefing_grid", 0x244, &host.briefing_grid_page},
        {"FE_briefing", 0x248, &host.briefing_page},
    };
    for (const PageBind& entry : pages) {
        GuiLayoutPage* page = host.frontend.load_screen_page(entry.name);
        host.log.implemented("MainMenuScreen::load_layout", "00aa5840");
        if (page == nullptr || !page->root) {
            host.log.notef("main-menu layout %s did not load", entry.name);
            continue;
        }
        *entry.slot = page;
        host.widgets[entry.field] = page->root.get();
        ++host.summary.detail_pages_loaded;
        // The page builder that would show or hide a root on the top-level page
        // (00584AE0) is not reconstructed, so the executable leaves all three
        // hidden until 0058C010 shows +2F0h. That is this milestone's own
        // decision and not recovered behaviour.
        host.frontend.set_widget_visible(*page->root, false);
    }
    if (host.worldmap_page == nullptr) return;

    // The fourteen 00AA7E00 lookups 005861B0 runs against +2F0h. Four of them
    // reach widgets nested deeper than one level, which 00AA7E00 cannot do on
    // its own; the native chains lookups to get there, and the executable does
    // the same through find_descendant_by_name.
    struct WidgetBind {
        std::uint16_t field;
        const char* name;
    };
    const WidgetBind bindings[] = {
        {0x310, "missions_US_Group"},
        {0x314, "missions_JP_Group"},
        {0x318, "missions_US_DLC_Group"},
        {0x31C, "missions_JP_DLC_Group"},
        {0x320, "training_Group"},
        {0x324, "mission_pic_Group"},
        {0x328, "bg_01_Icon"},
        {0x32C, "background_Icon"},
        {0x344, "historical_Group"},
        {0x114, "mission_mappoint_template_Icon"},
        {0x118, "mission_mapflag_template_Icon"},
        {0x33C, "video_Movie"},
        {0x3B0, "test_Clipbox"},
        {0x3B8, "content_main_Text"},
    };
    GuiLayoutWidget& root = *host.worldmap_page->root;
    for (const WidgetBind& entry : bindings) {
        GuiLayoutWidget* node = find_child_by_name_00aa7e00(root, entry.name);
        bool direct = node != nullptr;
        if (node == nullptr) node = find_descendant_by_name(root, entry.name);
        host.log.implemented("MainMenuScreen::find_child", "00aa7e00");
        if (node == nullptr) {
            ++host.summary.detail_widgets_missing;
            host.log.notef("  widget +%03Xh %-32s NOT FOUND", entry.field, entry.name);
            continue;
        }
        host.widgets[entry.field] = node;
        ++host.summary.detail_widgets_bound;
        host.log.notef("  widget +%03Xh %-32s %s", entry.field, entry.name,
            direct ? "direct child" : "nested (chained lookup)");
    }

    // +124h: 005861B0 stores the three floats 00AA6750 returns, which 0059A68E
    // later passes to 00AA8240 on bg_01_Icon at +328h.
    GuiLayoutWidget* backdrop = host.widget(0x328);
    if (backdrop != nullptr) {
        const GuiWidgetPoint point = resolved_position(backdrop->transform);
        host.backdrop = {point.x, point.y, point.z};
    }
    host.log.notef("main-menu layout bound: %zu pages, %zu widgets, %zu missing",
        host.summary.detail_pages_loaded, host.summary.detail_widgets_bound,
        host.summary.detail_widgets_missing);
}

void GameMissionHost::build_top_level_page_00584ae0() {
    Impl& host = *impl_;
    // 00584AE0's seven-iteration loop at 00584BF0..00584C70 allocates each entry
    // through 00AAB4C0 and labels it with the string at 00E087B8[i]. The entries
    // are list-box items built in code, not layout widgets, so the sprite bridge
    // has nothing to draw for them: what the executable can do is resolve each
    // label id through the locale table the phase-6 load filled.
    host.log.unimplemented("MainMenuScreen::build_top_level_page", "00584ae0");
    host.log.unimplemented("MainMenuScreen::allocate_list_entry", "00aab4c0");
    for (const MainMenuItem& item : kMainMenuItems) {
        GameMenuItemLabel label;
        label.index = item.index;
        label.label_id.assign(item.label);
        label.enabled = item.enabled;
        const std::u16string* text = host.locale.find_00a9ec70(label.label_id);
        if (text != nullptr) {
            label.text = ascii_fold(*text);
            label.resolved = true;
            ++host.summary.menu_items_resolved;
        }
        host.log.implemented("MainMenuScreen::locale_lookup", "00a9ec70");
        host.log.notef("  menu item %d %-28s enabled=%d %s", label.index,
            label.label_id.c_str(), label.enabled ? 1 : 0,
            label.resolved ? label.text.c_str() : "(no table entry)");
        host.summary.menu_items.push_back(label);
    }
}


// ---------------------------------------------------------------------------
// Milestone 2f: past the renderer-owner hosts
// ---------------------------------------------------------------------------

// 008860b0 concatenates "Scripts/missions/" + name + ".lua" with no directory
// walk, and Scripts/missions/ holds no loose .lua at all, so the name must
// carry a subdirectory (docs/MISSION_LUA_MACHINE.md, gap 2). The name the
// native uses comes from the scene record's script table at +928h, which the
// header pass does not fill (milestone 2e). **The executable therefore derives
// it from the scene path and checks the result against the mounted tree**: the
// scene file's own parent directory folded to lower case, then, if that does
// not resolve, each of the eight installed subdirectories in turn.
void GameMissionHost::Impl::finish_scene_load() {
    if (load.records.empty()) return;
    const SceneRecord& record = load.records.front();

    std::string stem = record.scene_path;
    std::string parent;
    const std::size_t slash = stem.find_last_of("/\\");
    if (slash != std::string::npos) {
        parent = stem.substr(0, slash);
        stem = stem.substr(slash + 1);
        const std::size_t parent_slash = parent.find_last_of("/\\");
        if (parent_slash != std::string::npos) parent = parent.substr(parent_slash + 1);
    }
    const std::size_t dot = stem.find_last_of('.');
    if (dot != std::string::npos) stem = stem.substr(0, dot);
    std::string folded;
    folded.reserve(parent.size());
    for (const char c : parent) {
        folded.push_back(static_cast<char>(c >= 'A' && c <= 'Z' ? c - 'A' + 'a' : c));
    }

    std::vector<std::string> candidates;
    if (!folded.empty()) candidates.push_back(folded + "/" + stem);
    for (const std::string& directory : bsp::installed_mission_subdirectories()) {
        std::string candidate = directory + "/" + stem;
        if (std::find(candidates.begin(), candidates.end(), candidate) == candidates.end()) {
            candidates.push_back(std::move(candidate));
        }
    }
    std::string script_name;
    for (const std::string& candidate : candidates) {
        if (!bsp::mission_script_name_carries_subdirectory(candidate)) continue;
        if (vfs.exists(bsp::mission_script_path(candidate))) {
            script_name = candidate;
            break;
        }
    }
    if (script_name.empty() && !candidates.empty()) script_name = candidates.front();
    summary.lua_script_path = bsp::mission_script_path(script_name);
    // Correction to the line the scene summary printed: milestone 2e derived the
    // mission script path from the scene stem alone, which is the bare form no
    // installed script occupies. The record's own field is the authority and the
    // header pass does not fill it, so this derivation replaces it.
    summary.mission_script_path = summary.lua_script_path;
    log.notef("mission script name derived from the scene path: %s -> %s (the record's "
        "+928h script table is not filled by the header pass)", record.scene_path.c_str(),
        summary.lua_script_path.c_str());

    lua = std::make_unique<GameMissionLuaHost>(log, vfs);
    frame_host = std::make_unique<GameMissionFrameHost>(log, vfs, *lua, profiler, language,
        hud);
    // 00884be0 at 004dd627 runs from BSP_Game_OnInitOnce, well before the load;
    // this process reaches its first mission here, so the machine is built now
    // and kept for the rest of the run.
    lua->start_machine_00884be0();
    // Milestone 2g: game+2198h, the key the record commit writes under, and the
    // frame the executable makes the script's end-movie call on.
    frame_host->set_mission_key(summary.selected_id);
    frame_host->set_mission_complete_frame(mission_complete_frame);
    // Milestone 2i: the player order and the deterministic frame delta.
    frame_host->set_player_order(order_frame, order_throttle, order_rudder);
    frame_host->set_mission_frame_seconds(mission_frame_seconds);
    // Milestone 2h. The load's release of the main-menu manager runs
    // BSP_MainMenu_Destroy 00686c90, which exits each of its seven screens,
    // clears both flag bytes and commits through 004f83b0 at 00686db9, then
    // destroys each screen through its vtable +0Ch at 00686e14. Destroying the
    // main-menu screen releases the three layouts it holds at +2F0h, +244h and
    // +248h, which is what takes the mission-detail page off the screen. The
    // executable owns those three roots directly rather than through the
    // screen's +24h collector, so it hides them here and says so.
    for (GuiLayoutPage* released : {worldmap_page, briefing_grid_page, briefing_page}) {
        if (released != nullptr && released->root) {
            frontend.set_widget_visible(*released->root, false);
        }
    }
    frontend.invalidate_bridge();
    log.note("the mission-detail page roots were hidden: 00686c90 destroys the main-menu "
        "screen, which releases the three layouts it holds at +2F0h, +244h and +248h");
    frame_host->run_scene_load_004dfb70(record.scene_path, script_name,
        record.locale_table_list, record.mission_id);

    const GameMissionLoadRunSummary& load_run = frame_host->load_summary();
    summary.mission_load_finished = load_run.state_after == bsp::kGameStateSceneReady;
    summary.mission_load_concrete = load_run.concrete;
    summary.mission_load_records = load_run.records;
    summary.mission_game_state = static_cast<int>(load_run.state_after);
    publish_frame_summary();
}

void GameMissionHost::Impl::publish_frame_summary() {
    if (lua != nullptr) {
        const GameMissionLuaSummary& machine = lua->summary();
        summary.lua_bindings = machine.bindings_registered;
        summary.lua_natives = machine.natives.size();
        summary.lua_native_calls = machine.native_calls;
    }
    if (frame_host != nullptr) {
        const GameMissionFrameRunSummary& frames = frame_host->frame_summary();
        summary.mission_frames_run = frames.frames;
        summary.mission_frames_simulated = frames.simulated;
        summary.mission_exit_note = frames.exit_note;
        summary.mission_exit_frames = frames.exit_frames;
        summary.mission_exit_completed = frames.exit_completed;
        summary.mission_complete_injected = frames.complete_injected;
        summary.mission_entered = frame_host->entry_summary().entered;
        if (frame_host->entry_summary().state != 0) {
            summary.mission_game_state = static_cast<int>(frame_host->entry_summary().state);
        }
    }
}

bool GameMissionHost::advance(float seconds) {
    Impl& host = *impl_;
    if (host.requested_id.empty() || !host.summary.tree_loaded) return false;

    switch (host.step) {
    case GameMissionStep::Idle:
        host.publish_selection_00580940();
        host.step = GameMissionStep::PublishList;
        return true;
    case GameMissionStep::PublishList:
        // The mission-list arm of 00599DB0 with no action armed.
        host.run_page_machine(seconds);
        host.step = GameMissionStep::MissionList;
        return true;
    case GameMissionStep::MissionList:
        // 00598B60's campaign arm calls the builder at 00599318. The dispatcher
        // itself is not reconstructed, so the executable reaches the builder the
        // way that arm does and records the dispatcher.
        host.log.unimplemented("MainMenuScreen::page_dispatch", "00598b60");
        host.build_detail_page_0058c010();
        host.run_page_machine(seconds);
        host.step = GameMissionStep::DetailPage;
        return true;
    case GameMissionStep::DetailPage:
        if (!host.summary.detail_built) {
            // 0058C092 rejects every published group but 1..4, so the training
            // grounds fall through to the epilogue without ever writing page 9.
            // There is no mission-detail page and therefore no footer command.
            host.log.notef("no mission-detail page was built for published group %d, so "
                "the page's footer command does not exist; the run stops here",
                host.summary.published_group);
            host.step = GameMissionStep::Stopped;
            return false;
        }
        // The page's `globals.continue` footer command, which 00592640 turns
        // into the play action 005922F0.
        host.log.unimplemented("MainMenuScreen::footer_command", "00592640");
        host.start_briefing_005922f0();
        host.step = GameMissionStep::BriefingStart;
        return true;
    case GameMissionStep::BriefingStart:
        host.step = GameMissionStep::LoadRequested;
        return true;
    case GameMissionStep::LoadRequested:
        host.consume_load_request();
        host.step = GameMissionStep::SceneRecord;
        return true;
    case GameMissionStep::SceneRecord:
        // Milestone 2e stopped here, in front of the first renderer-owner host.
        // Milestone 2f walks the rest of the recovered load inventory: every
        // renderer, scene-graph, world and sound step stays an explicit record
        // with the neutral value the reconstruction documents, so the load runs
        // to its end and leaves game state 0Ch behind.
        host.finish_scene_load();
        host.step = GameMissionStep::SceneLoaded;
        return true;
    case GameMissionStep::SceneLoaded:
        if (host.frame_host == nullptr || !host.summary.mission_load_finished) {
            host.step = GameMissionStep::Stopped;
            return false;
        }
        // 004db920 for game state 0Ch, which tails into 004da6c0.
        host.frame_host->enter_mission_state_004da6c0();
        host.publish_frame_summary();
        if (!host.summary.mission_entered) {
            host.log.notef("the mission state was not entered; the run stops at state 0x%02X",
                host.summary.mission_game_state);
            host.step = GameMissionStep::Stopped;
            return false;
        }
        host.step = GameMissionStep::InMission;
        return true;
    case GameMissionStep::InMission:
    case GameMissionStep::MissionFrames: {
        if (host.frame_host == nullptr || host.mission_frames <= 0) {
            if (host.frame_host != nullptr) host.frame_host->report(host.mission_frames);
            host.publish_frame_summary();
            host.step = GameMissionStep::Stopped;
            return false;
        }
        const bool more = host.frame_host->run_mission_frame_004e4a40(seconds);
        host.publish_frame_summary();
        host.step = GameMissionStep::MissionFrames;
        // The frame budget bounds the in-mission frames only. Once the mission
        // has left game state 0Dh the remaining frames belong to the exit path,
        // which runs to its own end (milestone 2g).
        const bool budget_spent = host.frame_host->in_mission_phase()
            && host.summary.mission_frames_run
                >= static_cast<unsigned long long>(host.mission_frames);
        if (!more || budget_spent) {
            host.frame_host->report(host.mission_frames);
            host.publish_frame_summary();
            host.step = GameMissionStep::Stopped;
            return false;
        }
        return true;
    }
    case GameMissionStep::Stopped:
        return false;
    }
    return false;
}

}  // namespace bsp::game
