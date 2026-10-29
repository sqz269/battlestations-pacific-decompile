// bsp_game.exe milestone 2e: from the main menu to the mission load request.
// See include/bsp/game_hosts_mission.hpp for the address list and the evidence.
#include "bsp/game_hosts_mission.hpp"
#include "bsp/game_hosts_ai.hpp"

#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_mission_frame.hpp"
#include "bsp/game_hosts_mission_result.hpp"
#include "bsp/game_hosts_script_orders.hpp"
#include "bsp/game_profile_load.hpp"
#include "bsp/game_profile_owner.hpp"
#include "bsp/mission_map_flag_policy.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/engine_seg1_cc13.hpp"
#include "bsp/engine_seg43_45_cc13.hpp"
#include "bsp/game_observer_runtime.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/gui_lua_runtime.hpp"
#include "bsp/gui_widget.hpp"
#include "bsp/locale_tables.hpp"
#include "bsp/lobby_option_defaults_cc13.hpp"
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
#include "bsp/game_native_renderer_application.hpp"
#include "bsp/gui_texture.hpp"
#include "bsp/mission_picture_owner.hpp"
#include "bsp/native_game_grid.hpp"
#include "bsp/native_string.hpp"
#include "bsp/native_string_pool_storage.hpp"
#include "bsp/native_system_constant_gather.hpp"
#include "bsp/native_texture_loading_cache.hpp"
#include "bsp/ocean_spectrum.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_record_side_blocks.hpp"
#include "bsp/session_participant_pools.hpp"
#include "bsp/session_context_cc13.hpp"
#include "bsp/vfs_mounts.hpp"
#include "bsp/sw1b_switch_reach.hpp"

#include <algorithm>
#include <array>
#include <atomic>
#include <cctype>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <map>
#include <new>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// Packet cc15-fe2: the mission-detail page's 00414130 call (0059A36F..0059A39C)
// runs engine_seg1_cc13.cpp's lerp instead of returning the offset unchanged.
constexpr bool kMissionDetailOffsetLerpBound = true;  // ON by bundle pair (cc9-sweep1b)

// Packet cc9_frame_delta_jitter. A process-private splitmix64 stream, so the
// shared generator 00BD2F10 (gunnery, planner) never sees a draw from it.
struct MissionFrameJitter {
    bool armed{false};
    double fraction{0.0};         // pct / 100
    std::uint64_t state{0};
    float next_factor() noexcept {
        state += 0x9E3779B97F4A7C15ull;
        std::uint64_t z = state;
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBull;
        z ^= z >> 31;
        const double u = static_cast<double>(z >> 11) * (1.0 / 9007199254740992.0);
        return static_cast<float>(1.0 + fraction * (2.0 * u - 1.0));
    }
};
MissionFrameJitter g_mission_frame_jitter;

// Harness only (packet cc9_tooling_1): the in-mission frame the crash record in
// src/game_main.cpp reads (-1 before the first mission frame), and the --crash-test
// trigger (-1 disarmed). Neither is read by the simulation.
volatile long g_harness_mission_frame = -1;
long g_harness_crash_test_frame = -1;
// Harness only (--stop-after-end S): the in-mission frame on which the script-orders host
// first saw the mission end (Mission.MissionStatus or Mission.EndMission), and how many
// further frames to run before stopping (-1: never, the default). Not read by the simulation.
long g_harness_mission_end_frame = -1;
long g_harness_stop_after_end_frames = -1;
bool g_harness_stopped_after_end = false;

// A real write through a null pointer, so the record shows a genuine access violation at a
// known instruction in bsp_game.exe. noinline keeps the faulting instruction in this body.
__declspec(noinline) void harness_crash_test_fault() {
    volatile int* volatile target = nullptr;
    *target = 0x0BADF00D;
}


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

    std::vector<TableKey> table_keys() override {
        // 00BD5F50 keeps every string, integer and number key in lua_next
        // order (tags 0, 1, 2). The kGuiLuaKeyArrayCapacity stop is a host
        // guard: 00BD5F50 has no bounds check and would overrun the 250-pair
        // frame array. Only the record reader's switched UniqueMultiSettings
        // walk calls this.
        std::vector<TableKey> out;
        if (host_.type_of(stack_.back()) != GuiLuaType::Table) return out;
        GuiLuaRef key{};
        GuiLuaRef value{};
        bool restart = true;
        while (host_.next(stack_.back(), key, value, restart)) {
            restart = false;
            const GuiLuaType type = host_.type_of(key);
            if (type == GuiLuaType::String) {
                out.push_back({GuiLuaKeyKind::Name, 0});
            } else if (type == GuiLuaType::Number) {
                const double number = host_.to_number(key);
                const auto integer = static_cast<std::int32_t>(number);
                if (static_cast<double>(integer) == number) {
                    out.push_back({GuiLuaKeyKind::Index, integer});
                } else {
                    out.push_back({GuiLuaKeyKind::FloatIndex, 0});
                }
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
// Native services for 005C6A70's picture tail
// ---------------------------------------------------------------------------

namespace {

// The image reaches every one of these through globals that exist before the
// mission-tree screen registers: the GUI manager singleton DAT_00f8bc5c
// (004c12b0, built during 0073bae0), the current renderer 00F8D394 (renderer
// constructor at 0073DA88, device inside BECEE0 before 0073DC27) and the atlas
// manager 00F8C26C. 005C75A9 calls 004c12b0 and 005C75C7 calls 00AA2660 with
// that renderer behind its slot +64h, so this object only has to exist before
// 005CAAF0 runs; game_hosts.cpp binds it once the device is created.
struct NativeMissionPictureLoader {
    GameHostLog& log;
    GameNativeRendererApplication& renderer;
    GameFrontendHost& frontend;
    GuiTextureCallbacks callbacks;
    MissionPictureTextureServices services;
    // B31C20 needs persistent acquisition frames. A frame left running or
    // failed by a throwing load is retained to process exit (its destructor
    // would terminate), as the host retains other interrupted native graphs.
    std::vector<std::unique_ptr<NativeTextureCacheAcquired>> frames;
    std::size_t loads{};
    std::size_t null_results{};
    std::vector<std::string> first_names;

    NativeMissionPictureLoader(GameHostLog& l, GameNativeRendererApplication& r,
        GameFrontendHost& f)
        : log(l), renderer(r), frontend(f),
          services{callbacks, r.actual_owners(),
              [this] { return frontend.gui_manager_004c12b0(); },
              // 005C75B4 passes the frame float2 at [esp+48h] uninitialized.
              // 00AA2660 reads it only on an atlas hit, and then only to
              // decide whether to query width/height and write a size back
              // into that same temporary, which 005C6A70 never reads again.
              // No atlas hit is reachable here (find_atlas_item below), so
              // the value is unobservable; zero is a host choice, not a
              // recovered preimage.
              [] { return std::array<float, 2>{0.0f, 0.0f}; }} {
        // 00AA2660's atlas lookup, 00AEFB20 on the manager at 00F8C26C. The
        // host never constructs that manager (00AF0060 is unimplemented in
        // TitleInit::load_texture_atlas, game_hosts_menu.cpp), so the lookup
        // always misses. In the binary it holds allbutingame.ats; none of
        // this installation's twelve interface/textures/*.ats descriptors
        // names a missions/ item, and every picture in missiontree.lua is a
        // loose fe/missions/ texture, so the binary misses here too.
        callbacks.find_atlas_item = [](std::string_view) -> const TextureAtlasItem* {
            return nullptr;
        };
        // 00AA2660's miss arm: current renderer slot +64h with flags 0. The
        // observed renderer profile D5F0A8 holds B319B0 there, as the font
        // producer checks before the same call.
        callbacks.load_texture = [this](std::string_view name, std::uint32_t flags) -> void* {
            return load_texture(name, flags);
        };
        // Texture profile D61948 slots +3Ch/+40h are B3CE50/B3CE60 (image
        // bytes at 00D61984/00D61988). Reached only on an atlas hit.
        callbacks.width = [](void* texture) {
            require_texture_2d(texture);
            return get_raw_texture_width_00b3ce50(texture);
        };
        callbacks.height = [](void* texture) {
            require_texture_2d(texture);
            return get_raw_texture_height_00b3ce60(texture);
        };
        // 00AA2660's InterlockedIncrement(texture+4) on an atlas hit.
        callbacks.retain = [](void* texture) {
            auto* count = std::launder(reinterpret_cast<std::atomic<std::int32_t>*>(
                static_cast<std::byte*>(texture) + 4));
            count->fetch_add(1, std::memory_order_seq_cst);
        };
        // Existing ready/native-acquisition gate the font producer uses.
        (void)renderer.section_layout_services();
        validate_mission_picture_services(services);
    }
    NativeMissionPictureLoader(const NativeMissionPictureLoader&) = delete;
    NativeMissionPictureLoader& operator=(const NativeMissionPictureLoader&) = delete;
    ~NativeMissionPictureLoader() {
        for (auto& frame : frames) {
            if (frame && frame->phase != NativeTextureCacheAcquired::Phase::not_started &&
                frame->phase != NativeTextureCacheAcquired::Phase::complete)
                static_cast<void>(frame.release());
        }
    }

    static void require_texture_2d(void* texture) {
        std::uint32_t profile{};
        std::memcpy(&profile, texture, sizeof(profile));
        if (profile != 0x00d61948)
            throw std::logic_error("mission picture atlas texture is not the D61948 2D profile");
    }

    void* load_texture(std::string_view name, std::uint32_t flags) {
        NativeTextureCacheContext& cache = renderer.texture_cache();
        NativeGameGridContext& grid = renderer.game_grid_context();
        void* receiver = renderer.publication_00f8d394();
        std::uint32_t profile{};
        if (receiver) std::memcpy(&profile, receiver, sizeof(profile));
        const auto* table = grid.graphics.streams.vertices.actual_renderer_profile_00d5f0a8;
        if (!receiver || profile != 0x00d5f0a8 || !table || table[0x64 / 4] != 0x00b319b0)
            throw std::runtime_error("mission picture requires the observed current renderer64=B319B0 profile");
        frames.push_back(std::make_unique<NativeTextureCacheAcquired>());
        // 00AA2660 forwards 005C6A70's frame string; the host builds the same
        // actual header from the record's picture name and destroys it after.
        const std::string text(name);
        NativeString native_name;
        native_name.assign_0041e870(cache.strings, text.c_str());
        void* result = nullptr;
        try {
            result = load_native_renderer_texture_00b319b0(receiver, &native_name, flags, cache,
                frames.back().get());
        } catch (...) {
            destroy_native_string_header_0041dd20(&native_name, cache.strings);
            throw;
        }
        destroy_native_string_header_0041dd20(&native_name, cache.strings);
        ++loads;
        if (result == nullptr) ++null_results;
        if (first_names.size() < 3) first_names.push_back(text);
        return result;
    }
};

}  // namespace

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

    // Declared before `tables` so every picture reference the records hold is
    // released before these services and their acquisition frames go.
    std::unique_ptr<NativeMissionPictureLoader> native_pictures;
    // 005CAAF0's tables, held for the run exactly as the mission-tree screen
    // holds them at +14h..+30h.
    MissionTreeTables tables;
    const MissionPictureTextureServices* picture_services{};
    bool tree_load_started{};

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
    // Milestone 2l: --order may name one of the 26 command classes instead of a
    // throttle/rudder pair, with an optional target entity.
    std::string order_command;
    std::string order_command_target;
    std::string order_unit;   // milestone 2n, --order-unit <name>
    std::string helm_orders_path;   // packet cc9_scripted_helm_orders
    std::int32_t chosen_difficulty_override{-1};   // packet cc13-torpranges, --difficulty
    // Packet cc13-mp1, --skirmish: requested_id names a MissionTree.multi entry.
    bool skirmish{false};
    std::int32_t skirmish_mode{-1};     // game+618h
    std::int32_t skirmish_index{-1};    // into tables.multi
    // Milestone 2o, --ai-drive <name>=<throttle>,<rudder>.
    std::string ai_drive_unit;
    float ai_drive_throttle{0.0f};
    float ai_drive_rudder{0.0f};
    float order_speed{0.0f};
    bool order_speed_set{false};
    // Milestone 2j, --trajectory-csv <path>.
    std::string trajectory_csv;
    GameFrameProfiler* profiler{nullptr};
    std::string language;
    // Native game allocation is zeroed at 0073E163. These two participant
    // pools retain bytes across scene loads; frame_host and its ShipAI borrow
    // this owner and are destroyed first (reverse member destruction order).
    bsp::SessionParticipantPools participants{
        bsp::ParticipantPoolInitialization::ZeroedGameAllocation};
    // Only a successfully parsed header properties bag supplies native+988h.
    // Key by the actual scene path; the semantic side_blocks vector still
    // lacks resolved Party/Race and must not imply a zero participant count.
    std::map<std::string, std::int32_t> participant_scene_counts;
    // The last parsed scene's side-block Party ordinals (slot record +28h, 004BB160 /
    // 004C6890 from block +0h), -1 where a block has none. Packet cc13-mp3 (MP3b).
    std::array<int, 8> scene_slot_parties{-1, -1, -1, -1, -1, -1, -1, -1};
    std::unique_ptr<GameMissionLuaHost> lua;
    GameObserverRuntime* observer_runtime{nullptr};
    std::unique_ptr<GameMissionFrameHost> frame_host;
    GameHudHost* hud{nullptr};  // milestone 2h, owned by GameMenuHost
    // cc15 lane L2, owned by GameMenuHost: game+650h and [00E198D0].
    GameProfileOwner* profile{nullptr};
    GameHintTrackerSlot* hint_tracker{nullptr};

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
    void resolve_skirmish_entry();
    void start_skirmish();
    std::int32_t skirmish_slot_team(std::size_t slot) const;
    void join_local_player_007752d0();
    void lobby_reset_slots_005d5340();
    void lobby_multi_menu_init_00689540();
    std::size_t menu_record_for_scene_005d7070() const;
    std::uint64_t map_content_mask_005d9c50() const;
    void lobby_enter_settings_005e51d0();
    void lobby_apply_selected_map_005da530();
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
        if constexpr (kProfileOwnerCc15Bound) {
            // 005C2F70 on the record: 0090C560(ECX = [game+6B4h], &record+0).
            const MissionGroupData* group = group_record();
            MissionProgress* progress =
                owner_.profile != nullptr ? owner_.profile->mission_progress() : nullptr;
            if (group != nullptr && mission < group->missions.size() && progress != nullptr) {
                owner_.log.implemented("MissionDetail::map_flag_visible", "005c2f70");
                return mission_map_flag_visible_005c2f70(group->missions[mission], *progress);
            }
        }
        // Without the profile owner the flag is shown for every mission of the
        // group and the decision is recorded.
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
    // 00414130 is ECX = a, EDX = b, [ESP+4] = t, returning (b - a) * t + a. The
    // caller passes ECX = this+1A8h, EDX -> 00CEFF04 and t -> 00D7A2F0, so the
    // offset moves a tenth of the way to -0.21 per frame: the header's `low`
    // is the target and `high` the rate. This runs on the main thread (24-bit x87
    // precision); math_lerp_00414130 rounds each of the three operations to 24
    // bits as the image does (cc13-fixture8, bit-exact in the leaf fixture).
    float blend_map_offset(float current, float low, float high) override {
        bsp::sw1b_note(bsp::Sw1bReach::MissionDetailOffsetLerp,                // sw1b reach
            engine_seg1_cc13::math_lerp_00414130(&current, &low, &high), current);
        if (kMissionDetailOffsetLerpBound) {
            owner_.log.implemented("MainMenuUpdate::blend_map_offset", "00414130");
            return engine_seg1_cc13::math_lerp_00414130(&current, &low, &high);
        }
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
        if constexpr (kHintTrackerCc15Bound) {
            if (owner_.hint_tracker != nullptr) {
                owner_.hint_tracker->get_004e1ca0().clear_hint_queue_0068ea00();
                owner_.log.implemented("SetPendingScene::notify_award_tracker", "0068ea00");
                return;
            }
        }
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
        // Packet cc13-mp1 (MP2a): a published skirmish context carries the session
        // word and its mode. Campaign identity: the OFF arm leaves the three old
        // literals above (0, 0, false).
        if constexpr (bsp::kSkirmishSessionCc13Bound) {
            const bsp::SessionContext& session = bsp::session_context();
            if (session.session_word_1fe4 != 0) {
                inputs.session_mode = session.session_word_1fe4;
                inputs.script_slot = session.game_mode_614;
                inputs.script_slot_forced = session.forced_61c;
            }
        }
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
    // game+6B0h, read at 0058BF52 when the record's difficulty is 3.
    // SUBSTITUTION, labelled, packet cc9_difficulty: this host loads no player
    // profile (007FDB20 is unimplemented), so the value is the one that reset
    // leaves, 1 (007FDFE2 / 007FE01B, docs/GAME_PROFILE_RESET.md). A loaded
    // profile would put its own SelectedDifficulty (profile+60h) here instead,
    // and the briefing's play action 0051B7B0 would overwrite it with the
    // player's pick. Switch off: the old 0.
    // Packet cc13-torpranges: --difficulty N stands in for a loaded profile's
    // SelectedDifficulty (this installation's saved profile holds 0).
    // cc15-prof2, kProfileDifficultyCc15Bound: game+6B0h is the game+650h
    // profile's +60h, which 0058BF52 reads; --difficulty keeps precedence (a
    // harness rule; the image has no such override). docs/CC15_PROFILE_LOAD.md 7.
    std::int32_t chosen_difficulty() override {
        if (owner_.chosen_difficulty_override >= 0) return owner_.chosen_difficulty_override;
        if constexpr (kProfileDifficultyCc15Bound && kProfileOwnerCc15Bound) {
            if (kSkillLevelBound && owner_.profile != nullptr) {
                const std::int32_t value = owner_.profile->profile().selected_difficulty_60;
                owner_.log.notef("MissionStart::chosen_difficulty game+6B0h=profile+60h=%d", value);
                return value;
            }
        }
        return kSkillLevelBound ? 1 : 0;
    }
    // game+6ACh, stored at 0058BF37 (the record's own value) or 0058BF58 (the
    // player's). GetDifficulty's reader in the script host reads it back.
    void set_effective_difficulty(std::int32_t value) override {
        if (!kSkillLevelBound) {
            owner_.log.unimplemented("MissionStart::set_effective_difficulty", "0058bf58");
            return;
        }
        set_game_effective_difficulty_6ac(value);
        owner_.log.notef("MissionStart::set_effective_difficulty game+6ACh=%d", value);
        owner_.log.implemented("MissionStart::set_effective_difficulty", "0058bf58");
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
        if constexpr (kMissionStartTemporaryReleaseCc15Bound) {
            // 0058BFEF 004D2A80 on the [ESP+10h] loading-config block: the
            // binding built no copy of it (publish_loading_config took the
            // record's vector and name by reference), so nothing is freed.
            owner_.log.implemented("MissionStart::finish_start", "004d2a80");
        } else {
            owner_.log.unimplemented("MissionStart::finish_start", "004d2a80");
        }
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
        if constexpr (kHintTrackerCc15Bound) {
            if (owner_.hint_tracker != nullptr) {
                // 0059230D 004E1CA0, then 00592314 00690CD0 on it.
                owner_.hint_tracker->get_004e1ca0().rebuild_history_00690cd0();
                owner_.log.implemented("MissionPlay::flush_award_tracker", "00690cd0");
                return;
            }
        }
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

// Packet cc13-mp1 (MP1). The image enters a skirmish through the multiplayer front
// end: the main menu's skirmish item 005E6F70 sets game+218Ch and requests states 6
// and 7; the mode selector's 005EC990 calls 00770E40 (Session_SetMode(1)), whose
// 0076FAD0 builds one scene record per MissionTree.multi entry (0076FC7C
// 004E1D70(record+24h, 0, -1, 0), record+24h the sceneFile text) and selects
// game+60Ch (0076FCA6 004C6890); the lobby's 005DA530 selects the chosen map
// (005DA604 004C6890([00E19594])) and 004DA6C0 enters the mission once loaded. The
// image's AutoTest driver 00427190 automates exactly that (docs/CC13_SEG1.md, screens
// 8..0Bh: the mode-selector values 3,0,1,2,4,4,4, then 004C6890(+1Ch), 005E1CF0(0),
// 005DA530, then 004DA6C0).
// LABELLED harness entry: the entry is picked by its Lua `id` (case-blind, the
// compare 005C3470 uses) or by its 0-based list index, the image's game+60Ch. Only
// that entry's record is built (the image builds all of them and selects one), the
// session, lobby and mode state are not modelled (MP2a/MP3), and the load is asked
// for with 00439020's two requests, as the campaign start does.
void GameMissionHost::Impl::resolve_skirmish_entry() {
    skirmish_index = -1;
    const std::string& wanted = requested_id;
    // An all-digit argument is only ever an index; an entry whose id is all digits
    // cannot be picked by id (rev9 N2).
    bool numeric = !wanted.empty();
    for (const char c : wanted) numeric = numeric && c >= '0' && c <= '9';
    if (numeric) {
        const unsigned long index = std::strtoul(wanted.c_str(), nullptr, 10);
        if (index < tables.multi.size()) skirmish_index = static_cast<std::int32_t>(index);
    } else {
        for (std::size_t i = 0; i < tables.multi.size(); ++i) {
            const std::string& id = tables.multi[i].screen.name;
            if (_stricmp(id.c_str(), wanted.c_str()) == 0) {
                skirmish_index = static_cast<std::int32_t>(i);
                break;
            }
        }
    }
    summary.skirmish_index = skirmish_index;
    summary.requested_found = skirmish_index >= 0;
    if (skirmish_index < 0) {
        log.notef("--skirmish %s names no MissionTree.multi entry (%zu entries); nothing is "
            "loaded", wanted.c_str(), tables.multi.size());
        return;
    }
    const MissionRecordData& record = tables.multi[static_cast<std::size_t>(skirmish_index)];
    summary.selected_id = record.screen.name;
    summary.selected_title = record.screen.title;
    summary.selected_scene = record.screen.scene;
    log.notef("skirmish selection: multi[%d] id=%s name=%s scene=%s mode=%d (packet cc13-mp1)",
        skirmish_index, record.screen.name.c_str(), record.screen.title.c_str(),
        record.screen.scene.c_str(), skirmish_mode);
}

// Packet cc13-mp3 (MP3a): the host arm of 0076FAD0 Session_SetMode(1) for a --skirmish
// run, over the reconstructed session_set_mode_0076fad0 (engine_seg43_45_cc13.cpp).
// Built only inside start_skirmish's kSkirmishSessionCc13Bound arm.
// Concrete: the scene record (one entry, below), its selection, the 004BB160
// participant reset and SetGameMode(+618h, 0). Recorded only: the XLive and transport
// calls. LABELLED: the server 00A42170 is not created, so the transport has no
// entries after the first and every session broadcast reaches nobody.
// LABELLED (rev9 N3): the image builds one record per MissionTree.multi entry
// (0076FC7C) and selects game+60Ch; this host builds only the chosen entry, so its
// game+60Ch (SceneSelection::selected_index) is 0 where the image's is the entry's
// index. Its only reader is the scene-record log line in run_until_scene_load.
// LABELLED: 0076FAD0 builds and selects before it stores the mode (0076FE85), so the
// image's 0076FCA6 selection runs with +1FE4h still 0; the lobby's 005DA604 selects
// again with +1FE4h = 1, and the host's single selection matches that last one.
class SkirmishSessionModeBinding final : public bsp::cc13cam2::SessionModeHost {
public:
    SkirmishSessionModeBinding(GameMissionHost::Impl& owner, MissionStartBinding& start,
        const MissionRecordData& record)
        : owner_(owner), start_(start), record_(record) {}

    bool world_present_e198c4() override { return false; }  // the front end has no world
    void singleton_f8a2fc_slot154() override {
        owner_.log.unimplemented("Session::set_mode xlive slot 154h", "0076fb4c");
    }
    void clear_scene_records_004c8900() override {
        // 004E2770 below clears the list itself before it builds.
        owner_.log.implemented("Session::clear_scene_records", "004c8900");
    }
    void set_game_608() override { owner_.log.unimplemented("Session::set_game_608", "0076fb68"); }
    void reset_peer_rows_007868c0(bool) override {
        owner_.log.unimplemented("Session::reset_peer_rows", "007868c0");
    }
    void delete_virtual(std::uintptr_t) override {}  // no transport was created before
    void set_route_default_e0af1c(std::int32_t value) override {
        route_default_e0af1c = value;  // 0077C2A0's default route flags; no host reader
        owner_.log.implemented("Session::set_route_default", "00e0af1c");
    }
    void please_wait_frame_0060d570() override {}
    std::uintptr_t create_server_00a42170(bool) override {
        owner_.log.unimplemented("Session::create_server (no peers, labelled)", "00a42170");
        return kNoPeerServer;
    }
    std::uintptr_t create_client_00a41c70() override {
        throw std::logic_error("0076FAD0 client arm in a skirmish");
    }
    std::int32_t transport_state_9c(std::uintptr_t) override { return server_state_9c; }
    void set_transport_state_9c(std::uintptr_t, std::int32_t state) override {
        server_state_9c = state;
    }
    // 0076FC28..0076FC7C walk the MissionTree.multi records; only the chosen one here.
    std::vector<std::string> scene_record_names() override { return {record_.screen.scene}; }
    void set_scene_count_f1af2c(std::int32_t) override {}
    void build_scene_record_004e1d70(const std::string& name) override {
        // 004E1D70(name, 0, -1, 0) for the one entry. Host substitution: the image calls
        // 004E1D70 directly at 0076FC7C once per record; this goes through the host's
        // 004E2770 (clear, build, select index 0).
        start_.set_pending_scene(name, std::string_view{});
    }
    void select_scene_record_004c6890() override {
        // 0076FCA6 004C6890(game+60Ch): the 004E2770 above already selected index 0.
        owner_.log.implemented("Session::select_scene_record", "004c6890");
    }
    void reset_participants_004bb160() override {
        // 0076FCB1 004BB160 over the selected scene's +988h count.
        const bool have_scene = owner_.load.selection.have_record;
        std::int32_t count = 0;
        if (have_scene && !owner_.load.records.empty()) {
            const auto found =
                owner_.participant_scene_counts.find(owner_.load.records.front().scene_path);
            if (found == owner_.participant_scene_counts.end()) {
                throw std::logic_error("0076FCB1 004BB160 without the scene's participant count");
            }
            count = found->second;
        }
        if (!owner_.participants.reset_flags_004bb160(have_scene, count)) {
            throw std::logic_error("0076FCB1 004BB160 rejected the scene participant count");
        }
        owner_.log.notef("session set mode 1: 004bb160 scene_present=%d scene_slots=%d",
            have_scene ? 1 : 0, count);
    }
    void clear_game_1ee5() override { owner_.log.unimplemented("Session::clear_game_1ee5", "0076fcc1"); }
    void reset_peer_pairs_00778590() override {
        owner_.log.unimplemented("Session::reset_peer_pairs", "00778590");
    }
    std::uintptr_t client_peer_0076f8a0(std::uintptr_t) override { return 0; }
    void debug_print_004254b0(const char*) override {}
    void send_quit_00783dc0(std::uintptr_t, std::uintptr_t) override {}
    void flush_007848f0(std::uintptr_t) override {}
    void clear_e19698_4() override {}
    void mark_local_slot_left() override {}
    void refresh_from_online_session_0076f0e0() override {}
    void clear_e17bf2() override {}
    void set_e08178(float) override {}
    void set_game_mode_004bc890() override {
        // 0076FEA4 SetGameMode(+618h, 0). rev9 N4: through session setup, before the
        // scene load, every SetGameMode on the skirmish path stores (m, m, forced 0).
        // The lobby's 005D5660, 005D54E0 and 005D62B0 take m = [00E19564], or [00E19568]
        // when that is 0..3 (005D5660 tests with a signed JG, 005D54E0/005D62B0 with an
        // unsigned CMOVBE: they agree only for non-negative modes); 0076FEA4 re-applies
        // +618h and 004C6AF2 (004C6890 with +1FE4h set) re-applies +614h, both equal to
        // the last m. So the end state of setup is (M, M, 0), M from the last lobby call,
        // in any order. Later callers break the triple: 004D548E in LoadSceneContents
        // (9, 0 on its AL == 0 branch) and 0046B99B in KillPass (8, flag EBP).
        // LABELLED: bsp::set_game_mode_004bc890 stores only the triple. The image's
        // 004BC8B3.. arm (+1FE4h != 0 and +5FCh != 0) also makes mode-specific scene
        // edits (mode 7 at 004BC8C7); at 0076FEA4 +1FE4h has just become 1, and +5FCh is
        // presumed null at the front end, so that arm is not modelled.
        bsp::SessionContext& session = bsp::session_context();
        bsp::set_game_mode_004bc890(session, session.raw_game_mode_618, false);
        owner_.log.implemented("Session::set_game_mode", "004bc890");
    }

    std::int32_t route_default_e0af1c{0};
    std::int32_t server_state_9c{0};

private:
    static constexpr std::uintptr_t kNoPeerServer = 1;  // a handle, never dereferenced
    GameMissionHost::Impl& owner_;
    MissionStartBinding& start_;
    const MissionRecordData& record_;
};

void GameMissionHost::Impl::start_skirmish() {
    if (skirmish_index < 0
        || static_cast<std::size_t>(skirmish_index) >= tables.multi.size()) {
        log.notef("no skirmish entry was resolved; the run stops");
        return;
    }
    const MissionRecordData& record = tables.multi[static_cast<std::size_t>(skirmish_index)];
    // Packet cc13-mp1 (MP2a): 005E6F87 (+218Ch = 1), then 0076FAD0's session word 1
    // and SetGameMode(+618h, 0), published before the scene record is built.
    if constexpr (bsp::kSkirmishSessionCc13Bound) {
        bsp::session_context() = bsp::skirmish_session_context(skirmish_mode);
        log.notef("skirmish session context: +1FE4h=1 +218Ch=1 +618h=%d +614h=%d +61Ch=0 "
            "effective mode %d (004bca50)", skirmish_mode, skirmish_mode,
            bsp::effective_game_mode_004bca50(bsp::session_context()));
    } else {
        log.notef("skirmish session context not published (kSkirmishSessionCc13Bound off): "
            "the map loads as a single-player scene");
    }
    MissionStatsBinding stats(*this);
    PendingSceneBinding scene(*this);
    MissionStartBinding start(*this, scene, stats);
    if constexpr (bsp::kSkirmishSessionCc13Bound) {
        // Packet cc13-mp3 (MP3a): 00770E40 -> 0076FAD0 Session_SetMode(1).
        bsp::cc13cam2::SessionModeState session_mode{};
        SkirmishSessionModeBinding binding(*this, start, record);
        bsp::cc13cam2::session_set_mode_0076fad0(session_mode, 1, binding);
        log.notef("session set mode 1 (0076fad0): mode=%d route default=%d server state=%d",
            session_mode.mode_f4, binding.route_default_e0af1c, binding.server_state_9c);
        // Packet cc13-mp3 (MP3c): the lobby option registry and defaults (00689540).
        lobby_multi_menu_init_00689540();
        // Packet cc13-mp3 (MP3b): the host player joins its own lobby, then the lobby
        // screen enters (MP3c: its settings writes, then 005D5340).
        join_local_player_007752d0();
        lobby_enter_settings_005e51d0();
        lobby_reset_slots_005d5340();
        // Packet cc13-mp3 (MP3c): the host starts the game from the lobby.
        lobby_apply_selected_map_005da530();
    } else {
        // 0076FC7C / 0076FCA6 for the one entry: 004E1D70 then 004C6890.
        start.set_pending_scene(record.screen.scene, std::string_view{});
    }
    // LABELLED: the multiplayer start's own request path is not traced (MP3).
    start.request_mission_start();
    summary.requests_queued = load.requests.size();
    summary.load_requested = !load.requests.empty();
    if constexpr (!bsp::kSkirmishSessionCc13Bound) {
        log.unimplemented("Session::set_mode_host", "0076fad0");
    }
    if constexpr (!bsp::kSkirmishSessionCc13Bound) {
        log.unimplemented("MultiGameLobby::apply_selected_map", "005da530");
    }
}

// Packet cc13-mp3 (MP3b). current[slot]+28h after 0076FAD0's 004BB160: a populated
// mission record has its scene side block's Party (004BB244..004BB251, block +0h), and
// 004BB630 copies the slot's mission +28h into a bound player record (004BB64A..004BB650),
// so the team is the slot's either way. LABELLED: a trailing record (slot >= the scene's
// MaxPlayerNum) keeps its old +28h; the host takes the zeroed allocation's 0 (0073E163),
// the value on the first selection after start-up.
std::int32_t GameMissionHost::Impl::skirmish_slot_team(std::size_t slot) const {
    std::int32_t count = 0;
    if (load.selection.have_record && !load.records.empty()) {
        const auto found = participant_scene_counts.find(load.records.front().scene_path);
        if (found != participant_scene_counts.end()) count = found->second;
    }
    if (static_cast<std::int32_t>(slot) >= count) return 0;
    const int party = scene_slot_parties[slot];
    if (party < 0) {
        throw std::logic_error("skirmish slot record without a side-block Party");
    }
    return party;
}

// Packet cc13-mp3 (MP3b). 007752D0 BSP_MMultiPlayer_ServerAddPlayer for the server's
// first transport entry, the host's own join (the entry 00784790 skips), then its slot
// pick and 0076C840 BSP_Session_HostSelectPlayer (multiplayer_session_cc13.cpp).
// Concrete: the 004BB440 claim, the pick over current[] +9h/+28h and the 004BB630 bind.
// LABELLED: the profile name, tag, xuid, rank and platform words, the MT_MYPLAYER /
// MT_SELECTPLAYER / MT_SERVERINFO messages (no entries after the first), and 007F8690's
// placement are not represented; game+18C8h/+18ECh are only logged.
// LABELLED guard: the host reads local slot 0 throughout (for example
// PowerupManagerBinding::local_player_slot_18ec), so any other pick throws. Of this
// installation's multi scenes, duelOne.scn (Party Japanese, Allied x7) picks slot 1
// and scene175/902/908.scn (all Japanese) pick -1 (no team-0 AI slot) outside mode 7.
void GameMissionHost::Impl::join_local_player_007752d0() {
    bsp::ParticipantRecordId local;
    if (participants.claim_player_flags_004bb440(local)
        != bsp::ParticipantClaimResult::Claimed) {
        throw std::logic_error("007752D0 local join without a free player record");
    }
    // 00775D06..00775DC9: mode 7 takes the first AI-held current record; otherwise the
    // first AI-held record of the team (+28h 0 or 1) with fewer human (+9h clear)
    // current records, team 0 on a tie, or -1 when that team has none.
    const std::int32_t mode = bsp::effective_game_mode_004bca50(bsp::session_context());
    std::int32_t first_team0 = -1, first_team1 = -1, humans0 = 0, humans1 = 0, choice = 0;
    for (std::size_t j = 0; j < bsp::kSceneSlotRecordCount; ++j) {
        bsp::ParticipantRecordId current;
        bsp::ParticipantRecordBytes bytes;
        if (!participants.active_record(j, current)
            || !participants.read_record_bytes(current, bytes)
            || !bytes.ai_held_09.available) {
            throw std::logic_error("00775D06 slot pick over an unrepresented record");
        }
        const std::int32_t team = skirmish_slot_team(j);
        if (bytes.ai_held_09.value != 0) {
            if (mode == 7) {
                choice = static_cast<std::int32_t>(j);
                break;
            }
            if (team == 0) {
                if (first_team0 < 0) first_team0 = static_cast<std::int32_t>(j);
            } else if (team == 1 && first_team1 < 0) {
                first_team1 = static_cast<std::int32_t>(j);
            }
        } else if (team == 0) {
            ++humans0;
        } else {
            ++humans1;
        }
    }
    if (mode != 7) choice = humans1 < humans0 ? first_team1 : first_team0;
    log.notef("skirmish local join (007752d0): player record %zu, mode %d, slot pick %d "
        "(team0 first %d humans %d, team1 first %d humans %d)", local.index, mode, choice,
        first_team0, humans0, first_team1, humans1);
    if (choice < 0) {
        // In the image -1 is a real outcome: no AI-held slot on the chosen team, so
        // 0076C840 leaves the local player unbound. LABELLED: the host does not model an
        // unbound host player.
        throw std::logic_error("skirmish local join: no free slot (-1); an unbound host "
            "player is not modelled");
    }
    if (choice != 0) {
        throw std::logic_error("skirmish local slot " + std::to_string(choice)
            + " != 0 not modelled");
    }
    // 0076C840: the target must be free (-1 or AI-held), the record claimed and not
    // AI-held; its old slot (+20h, -1 after 004BB160) is not restored.
    bsp::ParticipantRecordId target;
    bsp::ParticipantRecordBytes target_bytes;
    bsp::ParticipantRecordBytes local_bytes;
    if (!participants.active_record(static_cast<std::size_t>(choice), target)
        || !participants.read_record_bytes(target, target_bytes)
        || !participants.read_record_bytes(local, local_bytes)) {
        throw std::logic_error("0076C840 over an unrepresented record");
    }
    if (target_bytes.ai_held_09.value == 0 || local_bytes.claimed_08.value == 0
        || local_bytes.ai_held_09.value != 0) {
        log.notef("skirmish local join: 0076c840 refuses slot %d", choice);
        return;
    }
    participants.install_record_004bb630(static_cast<std::size_t>(choice), local);  // 0076C92C
    log.implemented("Session::host_select_player", "0076c840");
    log.notef("skirmish local slot (0076c98e 004b4740): game+18ECh=%d, game+18C8h=%zu", choice,
        local.index);
}

// Packet cc13-mp3 (MP3b). 005E51D0 BSP_MultiGameLobbyScreen_Enter -> 005D5340
// (frontend_seg15_22_cc13.cpp multi_game_lobby_reset_slots_005d5340), the AI defaults
// through 00771E80 -> 0076CB90 (host arm). LABELLED: the Live-service arm
// (005D5340's slot 198h / [00F8A2FC]+4Dh test, all eight slots off) is not taken: there
// is no Live service; the MT_AIENABLE broadcasts reach no peer.
void GameMissionHost::Impl::lobby_reset_slots_005d5340() {
    const std::int32_t mode = bsp::effective_game_mode_004bca50(bsp::session_context());
    if (static_cast<std::uint32_t>(mode) > 3u) {
        log.notef("skirmish lobby enter (005d5340): mode %d keeps 004bb160's +0Ah", mode);
        return;
    }
    // 005D53C0..005D542D: a side is taken by a player record (game+748h pool) that is
    // claimed and not AI-held (+8h set, +9h clear), at its +28h. A bound record's +28h is
    // its slot's team (004BB630); LABELLED: an unbound one keeps the zeroed 0.
    std::array<std::uint8_t, 3> side_taken{};
    const auto side_index = [](std::int32_t team) {
        if (team < 0 || team > 2) throw std::logic_error("005D5340 side outside 0..2");
        return static_cast<std::size_t>(team);
    };
    for (std::size_t k = 0; k < bsp::kSceneSlotRecordCount; ++k) {
        const bsp::ParticipantRecordId player{bsp::ParticipantPool::Player, k};
        bsp::ParticipantRecordBytes bytes;
        if (!participants.read_record_bytes(player, bytes) || bytes.claimed_08.value == 0
            || bytes.ai_held_09.value != 0) {
            continue;
        }
        std::int32_t team = 0;
        for (std::size_t j = 0; j < bsp::kSceneSlotRecordCount; ++j) {
            bsp::ParticipantRecordId current;
            if (participants.active_record(j, current) && current.pool == player.pool
                && current.index == k) {
                team = skirmish_slot_team(j);
            }
        }
        side_taken[side_index(team)] = 1;
    }
    // 005D5433..005D547C: per current slot, 00771E80(j, 0) when its side is taken, else
    // 00771E80(j, 1) and the side becomes taken.
    std::string summary_text;
    for (std::size_t j = 0; j < bsp::kSceneSlotRecordCount; ++j) {
        const std::size_t side = side_index(skirmish_slot_team(j));
        std::uint8_t on = 0;
        if (side_taken[side] == 0) {
            on = 1;
            side_taken[side] = 1;
        }
        if (!participants.set_slot_ai_enabled_0076cb90(j, on)) {
            throw std::logic_error("0076CB90 over an unrepresented record");
        }
        summary_text += on != 0 ? '1' : '0';
    }
    log.notef("skirmish lobby enter (005d5340): mode %d, +0Ah per slot %s", mode,
        summary_text.c_str());
}

// Packet cc13-mp3 (MP3c). 00689540 GVMultiMenu::Init runs 008D2F50 (the 00576B10
// registry) and constructs the lobby screen, whose 005E3290 leaves the option defaults in
// 00E1955C. LABELLED: the image does both when the multiplayer menu opens, before
// Session_SetMode, with 00E19594 = the game+60Ch of that moment; the host does them after
// its Session_SetMode with 00E19594 = the chosen entry, because 005D7070 needs that entry's
// scene uniqueID. 00688E40 BSP_MultiMenu_OnSceneExit also reconstructs the lobby screen,
// so the image runs the 005E3290 walk again with that moment's game+60Ch; one skirmish per
// process never reaches it. The choice of record only matters through its +428h rows: this
// installation's map records override rows 2 and 3 alone (luaOverrideMultiLobbySettings),
// and the lobby enter rewrites slot 2, and slot 3 when the mode is 0..3.
void GameMissionHost::Impl::lobby_multi_menu_init_00689540() {
    bsp::LobbySessionState& lobby = bsp::lobby_session_state();
    bsp::LobbyOptionRegistry registry;
    {
        // 008D2F50: a fresh owner with library mask 1, then 00B69D40 on the datatable.
        auto lua_owner = std::make_unique<PcStorageLuaOwner>(
            scripts.files().owner_environment(scripts.runtime(), scripts.globals()));
        lua_owner->open_storage_archive_00b6a020(1);
        const std::string path = "Scripts\\datatables\\MultiGlobals.lua";
        scripts.runtime().run_file(lua_owner->storage_lua_38(), path);
        GuiLua51Host reader(*lua_owner->storage_lua_38());
        const GuiLuaRef root = lua_global_by_name_00b67980(reader, "MultiLobbySettings");
        const auto integer = [&](const GuiLuaRef& value) {
            return static_cast<std::int32_t>(reader.to_number(value));  // 00B66290
        };
        const auto present = [&](const GuiLuaRef& table, const char* name) {  // !00B65FB0
            const GuiLuaRef value = reader.get_by_name(table, name);
            const GuiLuaType type = reader.type_of(value);
            if (value.valid()) reader.release(value);
            return type != GuiLuaType::Nil && type != GuiLuaType::None;
        };
        // MultiLobbySettings: slot -> option -> the option's own table, each in lua_next order.
        GuiLuaRef slot{};
        GuiLuaRef options{};
        for (bool first = true; reader.type_of(root) == GuiLuaType::Table
                 && reader.next(root, slot, options, first); first = false) {
            if (reader.type_of(options) != GuiLuaType::Table) continue;
            GuiLuaRef option{};
            GuiLuaRef choices{};
            for (bool first_option = true; reader.next(options, option, choices, first_option);
                 first_option = false) {
                if (reader.type_of(choices) != GuiLuaType::Table) continue;
                GuiLuaRef key{};
                GuiLuaRef value{};
                for (bool first_key = true; reader.next(choices, key, value, first_key);
                     first_key = false) {
                    const std::int32_t outer = integer(slot);
                    const std::int32_t inner = integer(option);
                    registry.enabled_28[outer][inner] = present(choices, "MenuDIS") ? 0 : 1;
                    if (present(choices, "Default")) {
                        if (outer < 0 || outer >= static_cast<std::int32_t>(registry.defaults_40.size())) {
                            throw std::logic_error("008D2F50 Default outside +40h..+8Ch");
                        }
                        registry.defaults_40[static_cast<std::size_t>(outer)] = inner;
                    }
                    const GuiLuaType key_type = reader.type_of(key);
                    if (key_type == GuiLuaType::String && reader.type_of(value) != GuiLuaType::Boolean) {
                        registry.numbers_1c[outer][inner] = 0;  // a label option
                    }
                    if (key_type == GuiLuaType::Number
                        && static_cast<double>(integer(key)) == reader.to_number(key)) {  // 00B66A60
                        registry.numbers_1c[outer][inner] = integer(key);
                    }
                }
            }
        }
        if (root.valid()) reader.release(root);
    }
    // 008D2F50's tail: every map record (the multi records) whose id text is a non-zero
    // number becomes an enabled slot-1 option keyed by that number, payload its ordinal.
    std::int32_t ordinal = 0;
    for (const MissionRecordData& map : tables.multi) {
        const long number = std::atol(map.screen.name.c_str());
        if (number == 0) continue;
        registry.enabled_28[1][static_cast<std::int32_t>(number)] = 1;
        registry.numbers_1c[1][static_cast<std::int32_t>(number)] = ++ordinal;
    }
    registry.loaded = true;
    lobby.registry = registry;
    log.implemented("MultiMenu::load_lobby_option_registry", "008d2f50");

    // 005E3290 with 00E19594 = the chosen entry: the availability map is the map record's
    // +428h when its +424h is set, else registry+28h.
    lobby.selected_map_00e19594 = skirmish_index;
    const MissionRecordData& map = tables.multi[menu_record_for_scene_005d7070()];
    if (map.extra.has_unique_multi_settings) {
        bsp::lobby_option_defaults_005e3290(lobby.registry, map.extra.unique_multi_settings_by_index,
            lobby.stored_options_00e1955c);
    } else {
        bsp::lobby_option_defaults_005e3290(lobby.registry, lobby.registry.enabled_28,
            lobby.stored_options_00e1955c);
    }
    std::string text;
    for (std::size_t s = 0; s < lobby.stored_options_00e1955c.size(); ++s) {
        text += (s == 0 ? "" : ",") + std::to_string(lobby.stored_options_00e1955c[s]);
    }
    log.notef("skirmish lobby options (005e3290): map record %s +424h=%d, 00E1955C = {%s}",
        map.screen.name.c_str(), map.extra.has_unique_multi_settings ? 1 : 0, text.c_str());
}

// 005D7070 for 00E19594 = the chosen entry. The image takes scene record game+5F0h[index]
// (one per multi entry, built by 0076FAD0), reads its +1098h uniqueID (29h as 26h, 2Ah as
// 27h), and returns the first map record whose id text equals it (equal length, then
// _stricmp), else 0. The host builds only the chosen entry's scene record, so the uniqueID
// is its header pass's.
std::size_t GameMissionHost::Impl::menu_record_for_scene_005d7070() const {
    std::int32_t kind = summary.scene_mission_id;
    if (kind == 0x29) {
        kind = 0x26;
    } else if (kind == 0x2A) {
        kind = 0x27;
    }
    const std::string key = std::to_string(kind);  // 004260B0
    for (std::size_t i = 0; i < tables.multi.size(); ++i) {
        const std::string& id = tables.multi[i].screen.name;
        if (id.size() != key.size()) continue;
        bool match = true;
        for (std::size_t c = 0; c < id.size() && match; ++c) {
            match = std::tolower(static_cast<unsigned char>(id[c]))
                == std::tolower(static_cast<unsigned char>(key[c]));
        }
        if (match) return i;
    }
    return 0;
}

// 005D9C50 over 005D8650's record (005D7070(00E19594)): its +10h contentID.
std::uint64_t GameMissionHost::Impl::map_content_mask_005d9c50() const {
    const std::string& name = tables.multi[menu_record_for_scene_005d7070()].extra.content_id;
    if (name.empty()) return 0;
    const std::size_t underscore = name.rfind('_');
    const std::string suffix = name.substr(underscore == std::string::npos ? 0 : underscore + 1);
    return bsp::lobby_content_mask_005d9c50(static_cast<std::int32_t>(std::atol(suffix.c_str())));
}

// 005E51D0 (lobby enter), 005E57B0..005E58B7, as the host (+1FE4h == 1).
void GameMissionHost::Impl::lobby_enter_settings_005e51d0() {
    bsp::LobbySessionState& lobby = bsp::lobby_session_state();
    const std::int32_t mode = bsp::effective_game_mode_004bca50(bsp::session_context());
    lobby.stored_options_00e1955c[2] = mode;                                   // 00E19564
    if (static_cast<std::uint32_t>(mode) <= 3u) lobby.stored_options_00e1955c[3] = mode;  // 00E19568
    lobby.selected_map_00e19594 = skirmish_index;                              // 005E57DD, game+60Ch
    // 005E581C, 005E20A0: from game+60Ch % count, the first map record (cycling) whose
    // availability [2][00E19564] is 1 becomes the selection. LABELLED: a different pick
    // means the requested entry disables the mode, which the lobby UI never offers; the
    // host stops rather than load another map.
    const std::size_t count = tables.multi.size();  // 00F1AF2C, one scene record per entry
    std::size_t index = static_cast<std::size_t>(skirmish_index) % count;
    for (std::size_t tried = 0; tried < count; ++tried) {
        const MissionRecordData& entry = tables.multi[index];
        const std::uint8_t on = entry.extra.has_unique_multi_settings
            ? bsp::lobby_option_flag(entry.extra.unique_multi_settings_by_index, 2, lobby.stored_options_00e1955c[2])
            : bsp::lobby_option_flag(lobby.registry.enabled_28, 2, lobby.stored_options_00e1955c[2]);
        if (on == 1) break;
        index = (count + index + 1) % count;
    }
    if (index != static_cast<std::size_t>(skirmish_index)) {
        throw std::runtime_error("skirmish entry " + tables.multi[static_cast<std::size_t>(skirmish_index)].screen.name
            + " disables mode " + std::to_string(mode) + " (005E20A0 would select entry "
            + tables.multi[index].screen.name + ")");
    }
    // 005E585A..005E58B7: +2C8h:+2CCh = 005DF510(mode), +2C0h:+2C4h = 005D9C50(mode),
    // game+2060h/+2064h = their OR. LABELLED: 005DF510 (the VehicleClass unit mask over
    // every map record) is not modelled and reads 0.
    lobby.content_mask_2060 = map_content_mask_005d9c50();
    log.notef("skirmish lobby enter (005e51d0): 00E19564=%d 00E19568=%d 00E19594=%d "
        "game+2060h=%08x:%08x (005DF510 not modelled)", lobby.stored_options_00e1955c[2],
        lobby.stored_options_00e1955c[3], lobby.selected_map_00e19594,
        static_cast<unsigned>(lobby.content_mask_2060 >> 32),
        static_cast<unsigned>(lobby.content_mask_2060));
}

// 005DA530 (the lobby's start), as the host. LABELLED: the map list entry (+2F0h/+2F4h)
// to the command line and 00E195C4 is not represented, and 004C6890(00E19594) is the
// selection 0076FAD0's binding already made. Then the mask again (005DA61C..005DA64D) and
// 005D62B0.
void GameMissionHost::Impl::lobby_apply_selected_map_005da530() {
    bsp::LobbySessionState& lobby = bsp::lobby_session_state();
    lobby.content_mask_2060 = map_content_mask_005d9c50();  // +2C8h:+2CCh (005DF510) read as 0
    // 005D62B0: the mode from 00E19564, or 00E19568 when that is 0..3 (unsigned), set with
    // forced 0; the fourteen indices copied to game+2020h.
    std::int32_t mode = lobby.stored_options_00e1955c[2];
    if (static_cast<std::uint32_t>(mode) < 4u) mode = lobby.stored_options_00e1955c[3];
    bsp::set_game_mode_004bc890(bsp::session_context(), mode, false);
    lobby.game_settings_2020 = lobby.stored_options_00e1955c;
    // 0076C9C0, 00772390 and 00772430 broadcast to peers; a host-only session has none.
    log.notef("skirmish lobby start (005da530 / 005d62b0): SetGameMode(%d, 0), game+2020h "
        "takes 00E1955C, broadcasts reach no peer", mode);
}

void GameMissionHost::Impl::read_scene_file(SceneRecord& record,
    const std::string& scene_path) {
    participant_scene_counts.erase(scene_path);
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
        auto* manager = vfs.ready() ? &vfs.context() : nullptr;
        std::string resolved = scene_path;
        if (manager == nullptr
            || !resolve_existing_resource_00bdf4c0_fragment(*manager,
                vfs.search_registrations(), resolved)) {
            log.notef("scene %s not resolved through the mounts", scene_path.c_str());
            log.unimplemented("SetPendingScene::load_scene_header_pass", "0046df00");
            return;
        }
        VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(*manager,
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
    if (document.has_header && document.header.has_properties) {
        const auto table = bsp::read_scene_record_slot_table_004f1d70(document.header.properties);
        // Packet cc9_stage_init_chunk_errors: 004F1D70's `GameStageScript` /
        // `StageScript` arm into record+968h, slot 8 of the +928h table. The
        // other StageScript* keys are not needed by a single-player load.
        {
            const bsp::SceneProperty* stage =
                document.header.properties.find("GameStageScript");
            if (stage == nullptr) stage = document.header.properties.find("StageScript");
            if (stage != nullptr && !stage->values.empty()) {
                if (record.script_names.size() < 11) record.script_names.resize(11);
                record.script_names[8] = stage->values.front();
            }
        }
        participant_scene_counts.emplace(scene_path, table.max_player_num);
        // Packet cc9_ai_party_gate: 004C6890 copies each side block's +0h
        // (Party) into slot record +28h. The ordinal is this installation's
        // universe/library/global.enums `enum Party` (Allied 0, Japanese 1,
        // Neutral 2; lines 1705-1710, mtime 2024-10-29; SHIP_AI 71).
        {
            std::array<int, 8> parties{};
            for (std::size_t i = 0; i < parties.size(); ++i) {
                const bsp::SceneSlotEnumValue& party = table.blocks[i].party;
                int ordinal = -1;
                if (table.blocks[i].present && party.present) {
                    if (party.symbol == "Allied") ordinal = 0;
                    else if (party.symbol == "Japanese") ordinal = 1;
                    else if (party.symbol == "Neutral") ordinal = 2;
                }
                parties[i] = ordinal;
            }
            ai_publish_scene_slot_parties(parties);
            scene_slot_parties = parties;
        }
        // Packet cc9_physics1_sea: the g_Ocean/Waves keys 0078C9B0 (called at
        // 004DF87F) loads into the ocean object; a no-op while
        // kWaveFieldLiveAmplitudeBound is off.
        {
            const bsp::OceanSceneWaves waves =
                bsp::ocean_live_field_publish_scene(document.header.properties);
            if (bsp::kWaveFieldLiveAmplitudeBound) {
                log.notef("ocean waves: path=%s authored=%d simple=%d height_scale=%.3f "
                    "wave_height=%.3f wind_speed=%.3f choppy=%.3f time_scale=%.3f",
                    scene_path.c_str(), waves.authored ? 1 : 0, waves.simple_ocean ? 1 : 0,
                    waves.height_scale_5c, waves.wave_height_54, waves.wind_speed_50,
                    waves.choppy_58, waves.time_scale_34);
            }
        }
        log.notef("scene participant scalar: path=%s available=1 max_players=%d "
            "authored=%d", scene_path.c_str(), table.max_player_num,
            table.max_player_num_authored ? 1 : 0);
    } else {
        log.notef("scene participant scalar: path=%s available=0 (header properties missing)",
            scene_path.c_str());
    }

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
        // LABELLED (rev9 N3): a skirmish builds only the chosen record, so
        // selected_index (game+60Ch) is 0 here where the image's is the entry's index.
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
    float order_rudder, float mission_frame_seconds, std::string trajectory_csv,
    std::string order_command, std::string order_command_target, float order_speed,
    bool order_speed_set)
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
    // Milestone 2j, --trajectory-csv.
    impl_->trajectory_csv = std::move(trajectory_csv);
    // Milestone 2l, --order <command>[:<entity>].
    impl_->order_command = std::move(order_command);
    impl_->order_command_target = std::move(order_command_target);
    // Milestone 2m, --order speed=<m/s>.
    impl_->order_speed = order_speed;
    impl_->order_speed_set = order_speed_set;
}

GameMissionHost::~GameMissionHost() = default;

void GameMissionHost::bind_observer_runtime(GameObserverRuntime& runtime) {
    if (!runtime.has_live_dispatch_owner())
        throw std::logic_error("mission observer binding requires a live dispatch owner");
    if (impl_->observer_runtime != nullptr && impl_->observer_runtime != &runtime)
        throw std::logic_error("mission observer runtime cannot change while the host lives");
    if (impl_->frame_host != nullptr) impl_->frame_host->bind_observer_runtime(runtime);
    impl_->observer_runtime = &runtime;
}

void GameMissionHost::bind_profile(GameProfileOwner& profile, GameHintTrackerSlot& hint_tracker) {
    impl_->profile = &profile;
    impl_->hint_tracker = &hint_tracker;
}

void GameMissionHost::set_order_unit(std::string unit) {
    impl_->order_unit = std::move(unit);
}

void GameMissionHost::set_helm_orders(std::string path) {
    impl_->helm_orders_path = std::move(path);
}

void GameMissionHost::set_chosen_difficulty(std::int32_t value) {
    impl_->chosen_difficulty_override = value;
}

void GameMissionHost::set_skirmish(std::int32_t mode) {
    impl_->skirmish = true;
    impl_->skirmish_mode = mode;
    impl_->summary.skirmish = true;
    impl_->summary.skirmish_mode = mode;
}

void GameMissionHost::set_ai_drive(std::string unit, float throttle, float rudder) {
    impl_->ai_drive_unit = std::move(unit);
    impl_->ai_drive_throttle = throttle;
    impl_->ai_drive_rudder = rudder;
}

bool GameMissionHost::requested() const noexcept { return !impl_->requested_id.empty(); }

const GameMissionSummary& GameMissionHost::summary() const noexcept {
    impl_->summary.step = impl_->step;
    return impl_->summary;
}
float GameMissionHost::simulated_seconds() const noexcept {
    return impl_->frame_host ? impl_->frame_host->frame_summary().simulated_seconds : 0.0f;
}
GameNativeMissionUnitSource GameMissionHost::native_render_unit_source() const noexcept {
    return impl_->frame_host ? impl_->frame_host->native_render_unit_source()
                             : GameNativeMissionUnitSource{};
}

void GameMissionHost::bind_mission_picture_services(const MissionPictureTextureServices& services) {
    if (impl_->tree_load_started || impl_->picture_services)
        throw std::logic_error("Mission picture services must bind once before tree loading");
    validate_mission_picture_services(services);
    impl_->picture_services = &services;
}
void GameMissionHost::bind_native_mission_pictures(GameNativeRendererApplication& renderer) {
    if (impl_->native_pictures)
        throw std::logic_error("Native mission picture services bind once");
    auto loader = std::make_unique<NativeMissionPictureLoader>(impl_->log, renderer,
        impl_->frontend);
    bind_mission_picture_services(loader->services);
    impl_->native_pictures = std::move(loader);
}
void GameMissionHost::load_mission_tree_005caaf0() {
    Impl& host = *impl_;
    host.tree_load_started = true;
    MissionTreeScriptBinding binding(host);
    try {
        host.tables = host.picture_services
            ? bsp::load_mission_tree_005caaf0(binding, *host.picture_services)
            : bsp::load_mission_tree_metadata(binding);
        host.summary.tree_loaded = true;
        host.summary.tree_pictures_loaded = host.picture_services != nullptr;
    } catch (const std::exception& error) {
        host.summary.tree_error = error.what();
        host.log.notef("mission tree did not load: %s", error.what());
        host.log.unimplemented("MissionTreeScreen::load_tables", "005caaf0");
        return;
    }
    if (host.picture_services) {
        host.log.implemented("MissionTreeScreen::load_tables", "005caaf0");
        if (const NativeMissionPictureLoader* pictures = host.native_pictures.get()) {
            std::string names;
            for (const std::string& name : pictures->first_names) {
                if (!names.empty()) names += ' ';
                names += name;
            }
            host.log.notef("mission tree pictures: %zu renderer loads through 005c6a70, "
                "%zu null results, first: %s", pictures->loads, pictures->null_results,
                names.c_str());
        }
    } else {
        host.log.notef("mission-tree metadata loaded; native picture services are unbound");
        host.log.unimplemented("MissionTreeScreen::picture_resources", "005c6a70");
    }

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
    if (host.skirmish) {
        // Packet cc13-mp1: the id names a MissionTree.multi entry, not a campaign one.
        // Campaign identity: without --skirmish the else arm is the old code.
        host.resolve_skirmish_entry();
    } else if (!host.summary.requested_found) {
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
// Packet cc9_stage_init_chunk_errors (docs/MISSION_LUA_MACHINE.md, "The mission
// script name"). True: the mission script is the scene record's own name,
// record+928h slot 8 (+968h), which 004F1D70 fills from the header property
// `GameStageScript`, or `StageScript` when that is absent, and which a
// single-player load selects (004E087B normalises the slot to 8). 008860B0
// takes the name verbatim. False: the name is derived from the scene path, as
// before; that misses every script whose folder differs from the scene's
// (JM06's `COTP-IJN\PRCPIJN\JM06`, LOMP06's `USN\LOMP\06_crucial_cargo`).
inline constexpr bool kSceneStageScriptBound = true;

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
    if (kSceneStageScriptBound && record.script_names.size() > 8
        && !record.script_names[8].empty()) {
        const std::string authored = record.script_names[8];
        log.notef("mission script name from the scene header: %s (record+968h; the path "
            "derivation answered %s)", authored.c_str(), script_name.c_str());
        script_name = authored;
    }
    summary.lua_script_path = bsp::mission_script_path(script_name);
    // Correction to the line the scene summary printed: milestone 2e derived the
    // mission script path from the scene stem alone, which is the bare form no
    // installed script occupies. The record's own field is the authority and the
    // header pass does not fill it, so this derivation replaces it.
    summary.mission_script_path = summary.lua_script_path;
    // Packet cc9_scene_script_table: the line states which source won. With
    // kSceneStageScriptBound the record's +928h slot 8 (+968h, 004F1D70 from the
    // header's GameStageScript / StageScript) is used whenever the header names one.
    const bool from_header = kSceneStageScriptBound && record.script_names.size() > 8
        && !record.script_names[8].empty();
    log.notef("mission script name: %s -> %s (%s)", record.scene_path.c_str(),
        summary.lua_script_path.c_str(),
        from_header ? "record+928h slot 8, the header's GameStageScript/StageScript"
                    : "derived from the scene path; the header names no stage script");

    // The previous frame and its children borrow the previous Lua owner.
    // Destroy those borrowers before replacing that owner on a later load.
    frame_host.reset();
    lua = std::make_unique<GameMissionLuaHost>(log, vfs);
    // Forward the retained live table; keep the independent copied language input.
    frame_host = std::make_unique<GameMissionFrameHost>(log, vfs, *lua, participants,
        locale, profiler, language, hud);
    if (observer_runtime != nullptr) frame_host->bind_observer_runtime(*observer_runtime);
    // cc15 lane L2: the record commit 009205E0 writes [game+6B4h].
    if (GameMissionResultHost* result = lua->mission_result()) result->bind_profile(profile);
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
    // Milestone 2l: --order <command>[:<entity>] takes the same frame.
    frame_host->set_player_command(order_command, order_command_target, order_unit);
    // Packet cc9_scripted_helm_orders: the timed order file, if one was given.
    if (!helm_orders_path.empty()) frame_host->set_helm_orders(helm_orders_path);
    // Milestone 2o: --ai-drive engages on that frame too, so the order that
    // puts a unit into a state and the stand-in for that state's step start
    // together.
    if (!ai_drive_unit.empty()) {
        frame_host->set_ai_drive(ai_drive_unit, ai_drive_throttle, ai_drive_rudder);
    }
    // Milestone 2m: --order speed=<m/s> takes it too.
    if (order_speed_set) frame_host->set_player_commanded_speed(order_speed);
    frame_host->set_mission_frame_seconds(mission_frame_seconds);
    // Milestone 2j: where the per-step per-unit trace is written.
    frame_host->set_trajectory_csv(trajectory_csv);
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
    const auto participant_count = participant_scene_counts.find(record.scene_path);
    frame_host->run_scene_load_004dfb70(record.scene_path, script_name,
        record.locale_table_list, record.mission_id, &record,
        participant_count == participant_scene_counts.end() ? nullptr : &participant_count->second);

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

bool GameMissionHost::next_advance_runs_mission_frame() const noexcept {
    const Impl& host = *impl_;
    if (host.requested_id.empty() || !host.summary.tree_loaded) return false;
    if (host.step != GameMissionStep::InMission && host.step != GameMissionStep::MissionFrames) {
        return false;
    }
    return host.frame_host != nullptr && host.mission_frames > 0;
}

bool GameMissionHost::advance(float seconds) {
    Impl& host = *impl_;
    if (host.requested_id.empty() || !host.summary.tree_loaded) return false;

    switch (host.step) {
    case GameMissionStep::Idle:
        if (host.skirmish) {
            // Packet cc13-mp1: the multiplayer front end, not the campaign pages.
            // Campaign identity: skirmish is false unless --skirmish called set_skirmish.
            host.start_skirmish();
            host.step = host.summary.load_requested ? GameMissionStep::LoadRequested
                                                    : GameMissionStep::Stopped;
            return host.summary.load_requested;
        }
        host.publish_selection_00580940();
        host.step = GameMissionStep::PublishList;
        return true;
    case GameMissionStep::PublishList:
        // The mission-list arm of 00599DB0 with no action armed.
        host.run_page_machine(seconds);
        host.step = GameMissionStep::MissionList;
        return true;
    case GameMissionStep::MissionList:
        if constexpr (kChallengeGroupPageCc15Bound) {
            if (host.page == MainMenuPage::TrainingGrounds) {
                // Packet cc15-chg. 00598BCF: page 8 is 00598B60's training-list
                // arm. 00598BD4 reads the activated row's data off the list at
                // screen+1B8h (00A9C990), 00598BE2/00598BE7 store it into
                // 00E194DC and 00E08878, and 00598BEC calls 005922F0 on the
                // screen. The row is the published mission, the one
                // selected_record() reads. No mission-detail page is built, and
                // screen+5Ch is not cleared the way the campaign arm clears it
                // at 00599315.
                host.log.unimplemented("MainMenuScreen::page_dispatch", "00598b60");
                host.log.notef("training-list arm 00598bcf: page 8 row %u (group %u) goes "
                    "to 005922f0 with no mission-detail page", host.published_mission,
                    host.published_group);
                host.start_briefing_005922f0();
                host.step = GameMissionStep::BriefingStart;
                return true;
            }
        }
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
        // Packet cc9_frame_delta_jitter: one factor per in-mission frame. With
        // --mission-frame-seconds the fixed delta is what the frame host uses, so
        // the jittered value replaces it for this frame; the fixed step (0.05) and
        // 00875BB0's accumulator are untouched and absorb the difference.
        float frame_seconds = seconds;
        if (g_mission_frame_jitter.armed) {
            const float factor = g_mission_frame_jitter.next_factor();
            if (host.mission_frame_seconds > 0.0f) {
                host.frame_host->set_mission_frame_seconds(host.mission_frame_seconds * factor);
            } else {
                frame_seconds = seconds * factor;
            }
        }
        g_harness_mission_frame = static_cast<long>(host.summary.mission_frames_run) + 1;
        if (g_harness_mission_frame == g_harness_crash_test_frame) {
            host.log.notef("harness crash test: null write in harness_crash_test_fault "
                "(bsp_game.exe+%lx) before mission frame %ld",
                static_cast<unsigned long>(reinterpret_cast<std::uintptr_t>(&harness_crash_test_fault)
                    - reinterpret_cast<std::uintptr_t>(GetModuleHandleA(nullptr))),
                g_harness_mission_frame);
            harness_crash_test_fault();
        }
        const bool more = host.frame_host->run_mission_frame_004e4a40(frame_seconds);
        host.publish_frame_summary();
        host.step = GameMissionStep::MissionFrames;
        // The frame budget bounds the in-mission frames only. Once the mission
        // has left game state 0Dh the remaining frames belong to the exit path,
        // which runs to its own end (milestone 2g).
        const bool budget_spent = host.frame_host->in_mission_phase()
            && host.summary.mission_frames_run
                >= static_cast<unsigned long long>(host.mission_frames);
        // Harness only: --stop-after-end ends the run that many frames after the mission's end
        // was recorded, instead of simulating on to the frame budget.
        const bool end_spent = g_harness_stop_after_end_frames >= 0
            && g_harness_mission_end_frame >= 0
            && g_harness_mission_frame >= g_harness_mission_end_frame + g_harness_stop_after_end_frames;
        if (end_spent) {
            g_harness_stopped_after_end = true;
            host.log.notef("harness stop-after-end: mission end recorded at mission frame %ld, "
                "stopping at mission frame %ld", g_harness_mission_end_frame, g_harness_mission_frame);
        }
        if (!more || budget_spent || end_spent) {
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

// Harness only (packet cc9_tooling_1); declared in src/game_main.cpp, the only caller.
long harness_mission_frame() noexcept { return g_harness_mission_frame; }
void arm_harness_crash_test(long mission_frame) noexcept { g_harness_crash_test_frame = mission_frame; }
// Harness only: --stop-after-end (src/game_main.cpp) and the script-orders host's end record.
void arm_harness_stop_after_end(long frames) noexcept { g_harness_stop_after_end_frames = frames; }
bool harness_stopped_after_end() noexcept { return g_harness_stopped_after_end; }
void harness_note_mission_end() noexcept {
    if (g_harness_mission_end_frame < 0) g_harness_mission_end_frame = g_harness_mission_frame;
}

void set_mission_frame_jitter(float percent, std::uint32_t seed) noexcept {
    g_mission_frame_jitter.armed = percent > 0.0f;
    g_mission_frame_jitter.fraction = static_cast<double>(percent) / 100.0;
    g_mission_frame_jitter.state = seed;
}

}  // namespace bsp::game
