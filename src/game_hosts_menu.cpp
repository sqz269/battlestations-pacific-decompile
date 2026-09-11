// bsp_game.exe milestone 2c: the frame's game state and profiler, the title
// pump and the main menu. See include/bsp/game_hosts_menu.hpp for the address
// list and the evidence.
#include "bsp/game_hosts_menu.hpp"

#include "bsp/frontend_entry.hpp"
#include "bsp/frontend_managers.hpp"
#include "bsp/frontend_screen_sets.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/frontend_states.hpp"
#include "bsp/game_frame_control.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_frontend.hpp"
#include "bsp/gui_layout_loader.hpp"
#include "bsp/input_tick.hpp"
#include "bsp/main_menu_screens.hpp"
#include "bsp/press_start_screen.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/title_init.hpp"

#include <algorithm>
#include <map>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

// 004f8710 publishes screen ids; the level-4 set the main-menu manager raises
// has exactly one element (docs/MAIN_MENU_PATH.md).
constexpr int kNoScreenSlot = -1;

std::string path_step_text(MainMenuPathStep step) {
    const std::string_view name = main_menu_path_step_name(step);
    return std::string(name.data(), name.size());
}

}  // namespace

// ---------------------------------------------------------------------------
// GameFrameProfiler
// ---------------------------------------------------------------------------

GameFrameProfiler::GameFrameProfiler(GameHostLog& log, int slot_count)
    : log_(log), slot_count_(slot_count < 2 ? 2 : slot_count) {
    // 00be3820 makes six allocations from one capacity and fills the colour
    // array with FFFFFF0Fh. The capacity is 00e15118 + 32h; 00e15118 is the
    // static counter count, which the image does not carry, so the executable
    // chooses one and records the substitution.
    const std::size_t slots = static_cast<std::size_t>(slot_count_);
    records_.resize(slots);
    history_.resize(slots * static_cast<std::size_t>(kProfilerHistoryFrames));
    start_offsets_.resize(slots * static_cast<std::size_t>(kProfilerHistoryFrames));
    current_.resize(slots);
    colors_.assign(slots, kProfilerDefaultSlotColor);
    display_.resize(slots);
    counters_.records = records_.data();
    counters_.history = history_.data();
    counters_.start_offsets = start_offsets_.data();
    counters_.current = current_.data();
    counters_.colors = colors_.data();
    counters_.display = display_.data();
    counters_.slot_count = slot_count_;

    LARGE_INTEGER frequency{};
    tick_scale_ = QueryPerformanceFrequency(&frequency) && frequency.QuadPart != 0
        ? 1.0 / static_cast<double>(frequency.QuadPart)
        : 0.0;
    log_.notef("profiler slots=%d history=%d spare=%d tick_scale=%.9f "
        "(0109db48 is zero in the image; seconds per QPC tick substituted)",
        slot_count_, kProfilerHistoryFrames, kProfilerSpareSlots, tick_scale_);
}

std::int64_t GameFrameProfiler::query_performance_counter() {
    ++samples_;
    LARGE_INTEGER counter{};
    QueryPerformanceCounter(&counter);
    return static_cast<std::int64_t>(counter.QuadPart);
}

const ProfilerCounterRecord& GameFrameProfiler::frame_record() const noexcept {
    return records_[static_cast<std::size_t>(app_update_slot())];
}

// ---------------------------------------------------------------------------
// GameMenuHost::Impl
// ---------------------------------------------------------------------------

struct GameMenuHost::Impl {
    // One registry slot. The flag bytes live in whichever object owns them
    // (004f7180 clears +4h and +5h in the base constructor), so this record
    // points at them rather than holding a second copy.
    struct MenuScreen {
        int slot{kNoScreenSlot};
        std::string name;
        // Owned when the leaf type has no reconstruction of its own; otherwise
        // `flags` points at the object that carries the two bytes.
        FrontEndScreen owned_flags{};
        FrontEndScreen* flags{nullptr};
        // The layouts the screen's virtual +24h reports to 004f83b0.
        std::vector<GuiLayoutPage*> pages;
        std::uint32_t enter_virtual{0};
        std::uint32_t exit_virtual{0};
        std::uint32_t update_virtual{0};
        bool press_start{false};
    };

    GameHostLog& log;
    GameFrontendHost& frontend;
    GameStateSlot& state;
    long press_start_frame{-1};
    GameMenuSummary summary;

    // --- the registry, 00e18b60 --------------------------------------------
    FrontEndWorld world{};
    GameFrontEndFrameState frame_state{};
    std::map<int, MenuScreen> screens;
    // Layout handles the press-start host hands back. Index 0 is reserved
    // because the native treats a null layout handle as absent.
    std::vector<GuiLayoutPage*> layout_handles{nullptr};
    std::vector<GuiLayoutWidget*> element_handles{nullptr};

    // --- the title object, 00e198c8 ----------------------------------------
    TitleInitState title_init{};
    TitleHandoverState handover{};
    TitleScreen title_object{};
    FrontEndFrameLayouts frame_layouts{};

    // --- the press-start screen, registry slot 5Ch --------------------------
    PressStartScreen press_start{};
    PressStartGlobals press_globals{};
    PromptPulse prompt_pulse{};

    // --- the injected input edge, 004c43c0 ----------------------------------
    InputActionRecord press_action{};
    bool press_start_handoff_requested{false};

    // --- the state-request ring, game+5D8h ---------------------------------
    GameStateRequestQueue requests{};

    // --- the screen sets and the input contexts ----------------------------
    FrontEndScreenSetStack screen_sets{};
    GameInputContextSetStack input_contexts{};

    // --- the front-end managers --------------------------------------------
    FrontEndManagerRegistry manager_registry{};
    FrontEndInterfaceLock interface_lock{};
    FrontEndManagerSet managers{};
    MainMenuManager main_menu{};
    MainMenuMusicState title_music{};
    FrontEndShellState shell_state{};
    LoadingScreenConfig loading_globals{};
    GameplayEffectManager* volatile effect_singleton{nullptr};
    GameplayEffectManagerAllocationWords effect_allocation_words{0};
    std::unique_ptr<SingletonLifetimeDomain> lifetime;
    std::unique_ptr<GameplayEffectManagerContext> effect_context;

    // --- the main-menu path -------------------------------------------------
    MainMenuPathState path{};
    MainMenuPathStep path_step{MainMenuPathStep::PressStartPoll};
    bool pumped_this_frame{false};

    Impl(GameHostLog& log_in, GameFrontendHost& frontend_in, GameStateSlot& state_in,
        long press_start_frame_in);

    MenuScreen* screen_at(int slot);
    // The executable's record of one registry slot. `external` is the object
    // that owns the two flag bytes, or null to let the record own them.
    MenuScreen& add_screen(int slot, std::string name, FrontEndScreen* external,
        std::uint32_t enter_virtual, std::uint32_t exit_virtual,
        std::uint32_t update_virtual, bool is_press_start);
    // 00aa5840 for a screen's own layout, with the byte 004f83b0 would publish.
    void attach_page(MenuScreen& screen, const std::string& layout_name);
    void pump(float raw_delta);
    void advance_path(float raw_delta);
};

namespace {

// ---------------------------------------------------------------------------
// 004f83b0, the per-screen visibility commit
// ---------------------------------------------------------------------------

class MenuCommitHost final : public FrontEndScreenCommitHost {
public:
    explicit MenuCommitHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void collect_screen_children(int slot, std::vector<void*>& children) override {
        // Screen vtable +24h. The base 004f75d0 is a bare RET 4, so a screen
        // with no layouts leaves the list alone; the press-start override
        // 0067d860 appends its one layout handle through 004d6790.
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        if (screen == nullptr) return;
        for (GuiLayoutPage* page : screen->pages) {
            if (page != nullptr) children.push_back(page);
        }
        owner_.log.implemented("FrontEndScreen::collect_children", "004f75d0");
    }

    void set_child_visible(void* child, bool visible) override {
        // The child's vtable +34h, the GUI element visibility setter. 004F8434
        // reads the screen's applied byte +5h and forwards it here.
        auto* page = static_cast<GuiLayoutPage*>(child);
        if (page == nullptr) return;
        owner_.frontend.commit_page_visibility(*page, visible);
        ++owner_.summary.visibility_commits;
        owner_.log.implemented("FrontEndScreen::set_child_visible", "004f83b0+34h");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004f7620's two per-screen predicates
// ---------------------------------------------------------------------------

class MenuSetBindings final : public FrontEndScreenSetHostBindings {
public:
    explicit MenuSetBindings(GameMenuHost::Impl& owner) : owner_(owner) {}

    bool screen_manages_own_visibility(int slot) override {
        // Vtable +4h. The base 004f7570 is `xor al,al; ret` and no leaf this
        // milestone builds overrides it, so the screen sets own every request.
        static_cast<void>(slot);
        return front_end_screen_self_managed_004f7570();
    }
    bool screen_occludes_lower_levels(int slot) override {
        // Vtable +8h, base 004f7580, also false.
        static_cast<void>(slot);
        static_cast<void>(owner_);
        return front_end_screen_predicate_004f7580();
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004f8710 and 004d8c00, the level-4 setters every manager calls
// ---------------------------------------------------------------------------

class MenuScreenSetHost final : public FrontEndScreenSetHost {
public:
    MenuScreenSetHost(GameMenuHost::Impl& owner, MenuSetBindings& bindings)
        : owner_(owner), bindings_(bindings) {}

    void set_gui_interface_set(const int* ids, std::size_t count) override {
        set_front_end_screen_set_004f8710(owner_.screen_sets, owner_.world.screens,
            bindings_, ids, count);
        owner_.log.implemented("FrontEndManager::set_gui_interface_set", "004f8710");
        owner_.summary.published_screen_id = count != 0 && ids != nullptr ? ids[0] : 0;
    }
    void set_game_interface_set(const int* ids, std::size_t count) override {
        set_game_input_context_set_004d8c00(owner_.input_contexts, ids, count);
        owner_.log.implemented("FrontEndManager::set_game_interface_set", "004d8c00");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuSetBindings& bindings_;
};

// ---------------------------------------------------------------------------
// The two refcount helpers the interface records use
// ---------------------------------------------------------------------------

class MenuPayloadHost final : public FrontEndPayloadHost {
public:
    explicit MenuPayloadHost(GameMenuHost::Impl& owner) : owner_(owner) {}
    void retain_payload(void* payload) override {
        if (payload == nullptr) return;
        owner_.log.unimplemented("FrontEndManager::retain_payload", "00694a60");
    }
    void release_payload(void* payload) override {
        if (payload == nullptr) return;
        owner_.log.unimplemented("FrontEndManager::release_payload", "006952a0");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// The press-start screen's host, 0067cfb0 and its neighbours
// ---------------------------------------------------------------------------

class MenuPressStartHost final : public PressStartScreenHost {
public:
    explicit MenuPressStartHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    // --- GUI layout and element --------------------------------------------
    std::uint32_t load_layout(const char* name) override {
        // 0067ca80: 004c12b0 then 00aa5840(name, 1, 0).
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(kPressStartScreenId);
        GuiLayoutPage* page = owner_.frontend.load_screen_page(name);
        owner_.log.implemented("PressStartScreen::load_layout", "00aa5840");
        if (page == nullptr) return 0;
        owner_.layout_handles.push_back(page);
        if (screen != nullptr) {
            screen->pages.push_back(page);
            // A screen is born neither wanted nor active, so the byte 004f83b0
            // would publish to a freshly bound layout is false.
            owner_.frontend.commit_page_visibility(*page, screen->flags != nullptr
                && screen->flags->active);
        }
        return static_cast<std::uint32_t>(owner_.layout_handles.size() - 1);
    }
    std::uint32_t find_element(std::uint32_t layout, const char* name) override {
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        if (page == nullptr) return 0;
        GuiLayoutWidget* widget = owner_.frontend.find_page_child(*page, name);
        if (widget == nullptr) return 0;
        owner_.element_handles.push_back(widget);
        return static_cast<std::uint32_t>(owner_.element_handles.size() - 1);
    }
    void release_layout(std::uint32_t layout) override {
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        owner_.frontend.release_screen_page(page);
    }
    void register_screen(int screen_id) override {
        register_front_end_screen_004f71d0(owner_.world.screens, owner_.press_start.base,
            screen_id);
        owner_.log.implemented("FrontEndScreen::register", "004f71d0");
    }
    void clear_page_context() override {
        owner_.log.unimplemented("PressStartScreen::clear_page_context", "00518d60");
    }
    void publish_layout(void* sink, std::uint32_t layout) override {
        // 004d6790(sink, screen+8h): the layout joins the list 004f83b0 walks.
        auto* children = static_cast<std::vector<void*>*>(sink);
        GuiLayoutPage* page = layout < owner_.layout_handles.size()
            ? owner_.layout_handles[layout] : nullptr;
        if (children != nullptr && page != nullptr) children->push_back(page);
        owner_.log.implemented("PressStartScreen::publish_layout", "004d6790");
    }

    // --- the pulsing prompt element ----------------------------------------
    void set_element_color(std::uint32_t element, float r, float g, float b,
        float a) override {
        GuiLayoutWidget* widget = element < owner_.element_handles.size()
            ? owner_.element_handles[element] : nullptr;
        if (widget != nullptr) owner_.frontend.set_widget_color(*widget, r, g, b, a);
        owner_.log.implemented("PressStartScreen::set_element_color", "0067cfb0+vtable50");
    }
    void set_element_flag(std::uint32_t element, bool value) override {
        GuiLayoutWidget* widget = element < owner_.element_handles.size()
            ? owner_.element_handles[element] : nullptr;
        if (widget != nullptr) owner_.frontend.set_widget_visible(*widget, value);
        owner_.log.implemented("PressStartScreen::set_element_flag", "0067d081");
    }

    // --- XenonSystemManager 00F8ABE8, which the process does not build ------
    SignInPhase sign_in_phase() override {
        owner_.log.unimplemented("XenonSystemManager::sign_in_phase", "00a3e500");
        return SignInPhase::Idle;
    }
    bool profile_change_pending() override {
        owner_.log.unimplemented("XenonSystemManager::profile_change_pending", "00a3e3b0");
        return false;
    }
    bool has_selected_user() override {
        owner_.log.unimplemented("XenonSystemManager::has_selected_user", "00a3e510");
        return false;
    }
    bool user_operation_busy() override {
        owner_.log.unimplemented("XenonSystemManager::user_operation_busy", "00a3e540");
        return false;
    }
    bool sign_in_blocked() override {
        owner_.log.unimplemented("XenonSystemManager::sign_in_blocked", "00f8abe8+3e8");
        return false;
    }
    int invitee_slot() override {
        owner_.log.unimplemented("XenonSystemManager::invitee_slot", "00a3e470");
        return 0;
    }
    void request_sign_in(int pad_index) override {
        static_cast<void>(pad_index);
        owner_.log.unimplemented("XenonSystemManager::request_sign_in", "00a3f3d0");
    }
    void reset_sign_in_state() override {
        owner_.log.unimplemented("XenonSystemManager::reset_sign_in_state", "00a40020");
    }

    // --- input --------------------------------------------------------------
    bool action_pressed_this_frame(int action) override {
        return owner_.press_start_frame >= 0
            && action == kPressStartInputAction
            && action_pressed_this_frame_004c43c0(owner_.press_action);
    }
    void* primary_device() override {
        owner_.log.unimplemented("PressStartScreen::primary_device", "004ba6d0");
        return nullptr;
    }
    bool device_button_held(void* device) override {
        static_cast<void>(device);
        owner_.log.unimplemented("PressStartScreen::device_button_held", "0067c880");
        return false;
    }
    int device_pad_index(void* device) override {
        static_cast<void>(device);
        owner_.log.unimplemented("PressStartScreen::device_pad_index", "0067c880+34h");
        return 0;
    }
    void rebind_primary_input() override {
        owner_.log.unimplemented("PressStartScreen::rebind_primary_input", "0067c970");
    }
    bool input_edge_latch() override {
        owner_.log.unimplemented("PressStartScreen::input_edge_latch", "004b43b0");
        return false;
    }

    // --- the shared menu command screen 00425d10 ---------------------------
    bool menu_screen_active() override {
        // The 0x290 singleton is not built here, so it is never up.
        return owner_.world.menu.base.active;
    }
    bool menu_command_pending() override {
        return owner_.world.menu.pending_command_a != 0
            || owner_.world.menu.pending_command_b != 0;
    }
    void clear_prompt_slot(int slot) override {
        static_cast<void>(slot);
        owner_.log.unimplemented("MenuCommandScreen::clear_prompt_slot", "00532a20");
    }
    void clear_all_prompt_slots() override {
        owner_.log.unimplemented("MenuCommandScreen::clear_all_prompt_slots", "00530650");
    }
    void close_menu_screen() override {
        owner_.log.unimplemented("MenuCommandScreen::close", "004b6e50");
    }
    void raise_prompt(int slot, const char* message_key, int kind) override {
        static_cast<void>(slot);
        static_cast<void>(kind);
        owner_.log.unimplemented("MenuCommandScreen::raise_prompt", "00531b00");
        owner_.log.notef("press-start prompt %s requested", message_key);
    }

    // --- the profile block at game+650h -------------------------------------
    void reset_profile_007fdb20() override {
        owner_.log.unimplemented("PressStartScreen::reset_profile", "007fdb20");
    }
    void reset_award_tracker_004374f0() override {
        owner_.log.unimplemented("PressStartScreen::reset_award_tracker", "004374f0");
    }
    const char* signed_in_gamertag() override {
        owner_.log.unimplemented("PressStartScreen::signed_in_gamertag", "00a3eae0");
        return "";
    }
    void set_profile_name(const char* name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("PressStartScreen::set_profile_name", "007f9290");
    }
    void set_profile_display_name(const char* name) override {
        static_cast<void>(name);
        owner_.log.unimplemented("PressStartScreen::set_profile_display_name", "007f9340");
    }
    std::uint64_t signed_in_xuid() override {
        owner_.log.unimplemented("PressStartScreen::signed_in_xuid", "00a3eb00");
        return 0;
    }
    void set_profile_xuid(std::uint64_t xuid) override {
        static_cast<void>(xuid);
        owner_.log.unimplemented("PressStartScreen::set_profile_xuid", "007fdb20+48h");
    }
    std::uint64_t save_id() override {
        owner_.log.unimplemented("PressStartScreen::save_id", "00a3e5d0");
        return 0;
    }
    void reset_save_manager_00bd3450() override {
        owner_.log.unimplemented("PressStartScreen::reset_save_manager", "00bd3450");
    }
    void commit_profile_007fae70() override {
        owner_.log.unimplemented("PressStartScreen::commit_profile", "007fae70");
    }

    // --- the save/storage manager at 0109CECC -------------------------------
    bool storage_device_required() override {
        owner_.log.unimplemented("StorageManager::device_required", "0109cecc+21h");
        return false;
    }
    bool storage_busy() override {
        owner_.log.unimplemented("StorageManager::busy", "0109cecc+8h");
        return false;
    }
    bool storage_query_1c(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::query_1c", "0109cecc+vtable1c");
        return false;
    }
    bool storage_query_14(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::query_14", "0109cecc+vtable14");
        return false;
    }
    void request_read_007ff100(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::request_read", "007ff100");
    }
    void request_write_007fa710(const char* save_name) override {
        static_cast<void>(save_name);
        owner_.log.unimplemented("StorageManager::request_write", "007fa710");
    }

    // --- the offline bring-up in the phase-2 arm ---------------------------
    void reset_offline_profile() override {
        owner_.log.unimplemented("PressStartScreen::reset_offline_profile", "008d5b50");
    }
    void apply_input_settings() override {
        owner_.log.unimplemented("PressStartScreen::apply_input_settings", "006ac030");
    }
    void refresh_input_bindings() override {
        owner_.log.unimplemented("PressStartScreen::refresh_input_bindings", "008d44c0");
    }
    void commit_input_manager() override {
        owner_.log.unimplemented("PressStartScreen::commit_input_manager", "00698a10");
    }

    // --- frame tail ---------------------------------------------------------
    void frontend_tail_event(int code) override {
        static_cast<void>(code);
        owner_.log.unimplemented("PressStartScreen::frontend_tail_event", "00427190");
    }
    void request_main_menu_state() override {
        // 0068d8a0 BSP_TitleScreen_Skip. The main-menu path owns that step, so
        // the request is handed to it rather than enqueued twice; see
        // docs/GAME_EXECUTABLE.md, milestone 2c.
        owner_.log.implemented("PressStartScreen::request_main_menu_state", "0068d8a0");
        owner_.press_start_handoff_requested = true;
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 00518250, the front-end frame layout selector
// ---------------------------------------------------------------------------

class MenuFrameLayoutHost final : public FrontEndFrameLayoutHost {
public:
    explicit MenuFrameLayoutHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void* gui_layout_acquire(const char* name) override {
        // 004c12b0 then 00aa5840(&name, 1, 1). These two layouts belong to the
        // 0x164 singleton at 00e18d80, not to a registry screen, so no 004f83b0
        // ever reaches them and the sprite bridge's substitute rule still
        // decides whether they are drawn.
        owner_.log.implemented("FrontEndFrameLayouts::acquire", "00518380");
        GuiLayoutPage* page = owner_.frontend.load_screen_page(name);
        if (page != nullptr) {
            // Not screen owned in the native sense; publish the same "shown"
            // byte the title state leaves them at so the rule is explicit.
            owner_.frontend.commit_page_visibility(*page, true);
        }
        return page;
    }
    std::int32_t gui_layout_refcount(void* layout) override {
        auto* page = static_cast<GuiLayoutPage*>(layout);
        return page != nullptr ? page->reference_count : 0;
    }
    void gui_manager_release_layout(void* layout) override {
        owner_.frontend.release_screen_page(static_cast<GuiLayoutPage*>(layout));
    }
    void gui_layout_release_ref(void* layout) override {
        auto* page = static_cast<GuiLayoutPage*>(layout);
        if (page != nullptr && page->reference_count > 0) --page->reference_count;
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0068d8d0 and 0068d850, the title handover
// ---------------------------------------------------------------------------

class MenuTitleHandoverHost final : public TitleHandoverHost {
public:
    MenuTitleHandoverHost(GameMenuHost::Impl& owner, MenuPressStartHost& press,
        MenuCommitHost& commit) : owner_(owner), press_(press), commit_(commit) {}

    void reset_storage_availability() override {
        owner_.log.unimplemented("TitleScreen::reset_storage_availability", "00bd3450");
    }
    void request_game_state(int state) override {
        enqueue_state_request_004d3ed0(owner_.requests, static_cast<std::uint32_t>(state));
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        owner_.log.implemented("TitleScreen::request_game_state", "004d7920");
    }
    FrontEndScreen* create_press_start_screen() override {
        // operator new(18h) then 0067c840, which chains 004f7180.
        construct_press_start_screen_0067c840(owner_.press_start);
        owner_.add_screen(kPressStartScreenId, "press_start",
            &owner_.press_start.base, 0x0067cb40u, 0x0067c870u, 0x0067cfb0u, true);
        owner_.log.implemented("TitleScreen::create_press_start_screen", "0067c840");
        return &owner_.press_start.base;
    }
    void screen_register(FrontEndScreen& screen) override {
        static_cast<void>(screen);
        register_press_start_screen_0067ca80(owner_.press_start, press_);
        owner_.summary.press_start_registered =
            owner_.world.screens.slots[kPressStartScreenId] == &owner_.press_start.base;
        owner_.summary.press_start_slot = kPressStartScreenId;
    }
    void screen_commit(FrontEndScreen& screen) override {
        commit_front_end_screen_visibility_004f83b0(screen, kPressStartScreenId, commit_);
        owner_.log.implemented("FrontEndScreen::commit_visibility", "004f83b0");
    }
    void screen_enter(FrontEndScreen& screen) override {
        static_cast<void>(screen);
        enter_press_start_screen_0067cb40(owner_.press_start, press_);
        ++owner_.summary.screen_enters;
        owner_.log.implemented("PressStartScreen::enter", "0067cb40");
    }
    void update_title_owner(float raw_delta) override {
        static_cast<void>(raw_delta);
        owner_.log.unimplemented("TitleScreen::update_title_owner", "00f8bbf4+4h");
    }
    void update_press_start_screen(float raw_delta) override {
        // 004f71f0 on title+40h, which tail-calls the screen's vtable +20h with
        // the raw delta. The registry pump reaches the same body in pass C, so
        // the native runs 0067cfb0 twice per frame in state 2 and so does this.
        const PressStartOutcome outcome = update_press_start_screen_0067cfb0(
            owner_.press_start, owner_.press_globals, owner_.prompt_pulse, raw_delta,
            press_);
        owner_.log.implemented("PressStartScreen::update", "0067cfb0");
        if (outcome != PressStartOutcome::Idle) {
            owner_.log.notef("press-start outcome %d", static_cast<int>(outcome));
        }
    }

private:
    GameMenuHost::Impl& owner_;
    MenuPressStartHost& press_;
    MenuCommitHost& commit_;
};

// ---------------------------------------------------------------------------
// 004c9a70, GGame::OnInitTitle
// ---------------------------------------------------------------------------

class MenuTitleInitHost final : public TitleInitHost {
public:
    MenuTitleInitHost(GameMenuHost::Impl& owner, MenuFrameLayoutHost& layouts,
        MenuTitleHandoverHost& handover, MenuCommitHost& commit)
        : owner_(owner), layouts_(layouts), handover_(handover), commit_(commit) {}

    void open_named_block(const char* label) override {
        owner_.log.unimplemented("TitleInit::open_named_block", "00be0a30");
        owner_.log.notef("title init scope %s", label);
    }
    void close_named_block() override {
        owner_.log.unimplemented("TitleInit::close_named_block", "00bdcb30");
    }
    void reset_player_profile() override {
        owner_.log.unimplemented("TitleInit::reset_player_profile", "007fdb20");
    }
    void load_texture_atlas(const char* path) override {
        // 00af0060 on the atlas manager 00f8c26c. The sprite bridge parses the
        // installed descriptors itself; the native manager is another owner's.
        owner_.log.unimplemented("TitleInit::load_texture_atlas", "00af0060");
        owner_.log.notef("title atlas %s requested", path);
    }
    void select_front_end_frame_set(FrontEndFrameSet set, bool commit) override {
        select_front_end_frame_set_impl(set, commit);
    }
    void* create_attract_screen() override {
        // operator new(4Ch) then 00689d90. The attract screen plays FE_attract
        // and is not reconstructed.
        owner_.log.unimplemented("TitleInit::create_attract_screen", "00689d90");
        return nullptr;
    }
    void attract_screen_init(void* screen) override {
        static_cast<void>(screen);
        owner_.log.unimplemented("TitleInit::attract_screen_init", "00689d90+4h");
    }
    void* create_title_screen() override {
        // operator new(44h) then 0068d760. Only +40h is recovered.
        owner_.log.implemented("TitleInit::create_title_screen", "0068d760");
        return &owner_.title_object;
    }
    void title_screen_activate(void* screen) override {
        static_cast<void>(screen);
        activate_title_screen_0068d8d0(owner_.handover, owner_.world.screens, handover_);
        owner_.title_object.attached_screen = owner_.handover.press_start;
        owner_.world.title = &owner_.title_object;
        owner_.log.implemented("TitleInit::title_screen_activate", "0068d8d0");
    }
    void title_screen_skip() override {
        owner_.log.unimplemented("TitleInit::title_screen_skip", "0068d8a0");
    }
    void reset_mission_player_records(void* records) override {
        static_cast<void>(records);
        owner_.log.unimplemented("TitleInit::reset_mission_player_records", "00916980");
    }
    bool sign_in_state_valid() override {
        owner_.log.unimplemented("TitleInit::sign_in_state_valid", "0067c8f0");
        return false;
    }
    void reset_sign_in_state() override {
        owner_.log.unimplemented("TitleInit::reset_sign_in_state", "00a40020");
    }
    void rebind_primary_input() override {
        owner_.log.unimplemented("TitleInit::rebind_primary_input", "0067c970");
    }
    void movie_screen_exit() override {
        owner_.log.unimplemented("TitleInit::movie_screen_exit", "00e18d48+vtable1c");
    }
    void movie_screen_commit() override {
        // 004f83b0 on 00e18d48. No movie screen is registered here, so the
        // cleared byte reaches no child.
        static_cast<void>(commit_);
        owner_.log.unimplemented("TitleInit::movie_screen_commit", "004f83b0");
    }

private:
    void select_front_end_frame_set_impl(FrontEndFrameSet set, bool commit) {
        bsp::select_front_end_frame_set(owner_.frame_layouts, layouts_,
            static_cast<int>(set), commit);
        owner_.log.implemented("TitleInit::select_front_end_frame_set", "00518250");
    }

    GameMenuHost::Impl& owner_;
    MenuFrameLayoutHost& layouts_;
    MenuTitleHandoverHost& handover_;
    MenuCommitHost& commit_;
};

// ---------------------------------------------------------------------------
// 004f8830's per-screen virtuals
// ---------------------------------------------------------------------------

class MenuScreenHost final : public FrontEndScreenHost {
public:
    MenuScreenHost(GameMenuHost::Impl& owner, MenuPressStartHost& press,
        MenuCommitHost& commit) : owner_(owner), press_(press), commit_(commit) {}

    void screen_exit(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_exits;
        if (screen != nullptr && screen->press_start) {
            exit_press_start_screen_0067c870(owner_.press_start);
            owner_.log.implemented("PressStartScreen::exit", "0067c870");
            return;
        }
        // Every other leaf's +1Ch is its own routine; none is reconstructed.
        owner_.log.unimplemented("FrontEndScreen::exit", "004f75b0");
        owner_.log.notef("screen %d exit virtual %08lx", slot,
            screen != nullptr ? static_cast<unsigned long>(screen->exit_virtual) : 0ul);
    }

    void screen_enter(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_enters;
        if (screen != nullptr && screen->press_start) {
            enter_press_start_screen_0067cb40(owner_.press_start, press_);
            owner_.log.implemented("PressStartScreen::enter", "0067cb40");
            return;
        }
        owner_.log.unimplemented("FrontEndScreen::enter", "004f75a0");
        if (screen == nullptr) return;
        owner_.log.notef("screen %d (%s) enter virtual %08lx", slot, screen->name.c_str(),
            static_cast<unsigned long>(screen->enter_virtual));
    }

    void screen_update(int slot, float seconds) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        ++owner_.summary.screen_updates;
        if (screen != nullptr && screen->press_start) {
            const PressStartOutcome outcome = update_press_start_screen_0067cfb0(
                owner_.press_start, owner_.press_globals, owner_.prompt_pulse, seconds,
                press_);
            owner_.log.implemented("PressStartScreen::update", "0067cfb0");
            if (outcome != PressStartOutcome::Idle) {
                owner_.log.notef("press-start outcome %d", static_cast<int>(outcome));
            }
            return;
        }
        owner_.log.unimplemented("FrontEndScreen::update", "004f75c0");
    }

    void screen_commit(int slot) override {
        GameMenuHost::Impl::MenuScreen* screen = owner_.screen_at(slot);
        if (screen == nullptr || screen->flags == nullptr) return;
        commit_front_end_screen_visibility_004f83b0(*screen->flags, slot, commit_);
        owner_.log.implemented("FrontEndScreen::commit_visibility", "004f83b0");
        owner_.frontend.invalidate_bridge();
    }

    void menu_command_update(float seconds) override {
        static_cast<void>(seconds);
        owner_.log.unimplemented("MenuCommandScreen::update", "00425d10+vtable20");
    }
    void menu_command_exit() override {
        owner_.log.unimplemented("MenuCommandScreen::exit", "00425d10+vtable1c");
    }
    void menu_command_commit() override {
        owner_.log.unimplemented("MenuCommandScreen::commit", "004f83b0+menu");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuPressStartHost& press_;
    MenuCommitHost& commit_;
};

// ---------------------------------------------------------------------------
// 004e4b9d..004e4d2c, the front-end branch of GGame::OnMove
// ---------------------------------------------------------------------------

class MenuFrameHost final : public FrontEndFrameHost {
public:
    MenuFrameHost(GameMenuHost::Impl& owner, MenuTitleHandoverHost& handover)
        : owner_(owner), handover_(handover) {}

    int game_state() override { return owner_.state.value; }

    bool render_queue_retained() override {
        // 004c11f0 then 00b1bf90. No retention control exists here, so the
        // branch always opens its frame.
        owner_.log.unimplemented("FrontEndFrame::render_queue_retained", "00b1bf90");
        return false;
    }
    void renderer_begin_frame() override {
        // 00f8d394 vtable +0Ch. The milestone's own Clear/BeginScene stands in.
        owner_.log.unimplemented("FrontEndFrame::renderer_begin_frame", "00f8d394+0ch");
    }
    void update_logo_sequence() override {
        owner_.log.unimplemented("FrontEndFrame::update_logo_sequence", "00685170");
    }
    void reinitialize_title_screen() override {
        owner_.log.unimplemented("FrontEndFrame::reinitialize_title_screen", "004db220");
    }
    void update_title_screen(float raw_delta) override {
        update_title_screen_0068d850(owner_.handover, handover_, raw_delta);
        owner_.log.implemented("FrontEndFrame::update_title_screen", "0068d850");
    }
    void gui_clear_screens(bool flag) override {
        static_cast<void>(flag);
        owner_.log.unimplemented("FrontEndFrame::gui_clear_screens", "00aa0e50");
    }
    void gui_set_enabled(bool enabled) override {
        static_cast<void>(enabled);
        owner_.log.unimplemented("FrontEndFrame::gui_set_enabled", "00aa0e00");
    }
    void gui_update(float raw_delta, int mode) override {
        static_cast<void>(raw_delta);
        static_cast<void>(mode);
        owner_.log.unimplemented("FrontEndFrame::gui_update", "00aa4f80");
    }
    void game_render() override {
        owner_.log.unimplemented("FrontEndFrame::game_render", "004ca440");
    }
    void game_finish_render_frame() override {
        // 004ca1f0 clears game+34h. The milestone's own Present is the frame
        // boundary here, so the projected byte is cleared with it.
        owner_.frame_state.render_queue_open = false;
        owner_.log.unimplemented("FrontEndFrame::game_finish_render_frame", "004ca1f0");
    }
    bool state_requests_pending() override { return owner_.requests.count != 0; }
    void drain_state_requests() override {
        if (owner_.requests.count == 0) return;
        const std::uint32_t request = front_state_request(owner_.requests);
        pop_front_state_request(owner_.requests);
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        apply_drained_game_state(owner_.state, static_cast<int>(request));
        owner_.log.implemented("FrontEndFrame::drain_state_requests", "004e4430");
        owner_.log.notef("state request %lu drained by the front-end branch",
            static_cast<unsigned long>(request));
    }

private:
    GameMenuHost::Impl& owner_;
    MenuTitleHandoverHost& handover_;
};

// ---------------------------------------------------------------------------
// 005884a0, the title music the shell starts
// ---------------------------------------------------------------------------

class MenuTitleMusicHost final : public TitleMusicHost {
public:
    explicit MenuTitleMusicHost(GameMenuHost::Impl& owner) : owner_(owner) {}
    bool movie_clip_active(std::string_view clip) override {
        static_cast<void>(clip);
        owner_.log.unimplemented("TitleMusic::movie_clip_active", "004f8ba0");
        return false;
    }
    void* create_music_stream(std::string_view path) override {
        owner_.log.unimplemented("TitleMusic::create_music_stream", "00a877d0");
        owner_.log.notef("title music path %.*s", static_cast<int>(path.size()), path.data());
        return nullptr;
    }
    std::string_view selected_track_name() override {
        owner_.log.unimplemented("TitleMusic::selected_track_name", "00e19504");
        return {};
    }
    void open_stream(void* stream, std::string_view track) override {
        static_cast<void>(stream);
        static_cast<void>(track);
        owner_.log.unimplemented("TitleMusic::open_stream", "00a867b0");
    }
    float music_volume() override {
        owner_.log.unimplemented("TitleMusic::music_volume", "00f889a8");
        return 0.0f;
    }
    void set_stream_volume(void* stream, float volume) override {
        static_cast<void>(stream);
        static_cast<void>(volume);
        owner_.log.unimplemented("TitleMusic::set_stream_volume", "00a864f0");
    }
    void play_stream(void* stream) override {
        static_cast<void>(stream);
        owner_.log.unimplemented("TitleMusic::play_stream", "00a85c20");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 00686380 and its siblings, the three front-end managers
// ---------------------------------------------------------------------------

class MenuManagerHost final : public FrontEndManagerHost {
public:
    explicit MenuManagerHost(GameMenuHost::Impl& owner) : owner_(owner) {}

    void open_load_block(std::string_view name) override {
        owner_.log.unimplemented("FrontEndManager::open_load_block", "00be0ae0");
        owner_.log.notef("front-end load block %.*s", static_cast<int>(name.size()),
            name.data());
    }
    void close_load_block() override {
        owner_.log.unimplemented("FrontEndManager::close_load_block", "00bdcb30");
    }
    void report_loading_progress(float progress) override {
        static_cast<void>(progress);
        owner_.log.unimplemented("FrontEndManager::report_loading_progress", "0057bec0");
    }
    void* create_screen(std::size_t size_bytes, std::uint32_t constructor_address) override {
        // operator new(size) then the leaf constructor. None of the seven
        // main-menu screen classes is reconstructed (docs/MAIN_MENU_SCREENS.md
        // records their vtables, ids and layouts), so the executable builds its
        // own registry record from that recovered table and records the
        // constructor address it stands for.
        owner_.log.unimplemented("FrontEndManager::create_screen", "00bf681b");
        const MainMenuScreenClass* recovered = nullptr;
        for (const MainMenuScreenClass& entry : kMainMenuScreens) {
            if (entry.constructor == constructor_address) {
                recovered = &entry;
                break;
            }
        }
        if (recovered == nullptr) {
            owner_.log.notef("front-end screen %08lx (%zu bytes) has no recovered class",
                static_cast<unsigned long>(constructor_address), size_bytes);
            return nullptr;
        }
        GameMenuHost::Impl::MenuScreen& screen = owner_.add_screen(recovered->screen_id,
            std::string(recovered->primary_layout), nullptr, recovered->enter_virtual,
            recovered->exit_virtual, recovered->update_virtual, false);
        owner_.log.notef("front-end screen id=%d layout=%.*s constructor=%08lx size=%lu",
            recovered->screen_id, static_cast<int>(recovered->primary_layout.size()),
            recovered->primary_layout.data(),
            static_cast<unsigned long>(constructor_address),
            static_cast<unsigned long>(recovered->object_size));
        return &screen;
    }
    void register_screen(void* screen) override {
        // The leaf's virtual +10h, which chains 004f71d0 and, in five of the
        // seven, binds the GUI layout. Only the main menu's layout is loaded
        // here; the other six bind data tables or pages no screen set raises.
        auto* record = static_cast<GameMenuHost::Impl::MenuScreen*>(screen);
        if (record == nullptr) return;
        register_front_end_screen_004f71d0(owner_.world.screens, *record->flags,
            record->slot);
        ++owner_.summary.screens_registered;
        owner_.log.implemented("FrontEndScreen::register", "004f71d0");
        if (record->slot == kMainMenuPathScreenId) {
            owner_.attach_page(*record, "FE_main");
            owner_.summary.main_menu_pages_loaded = record->pages.size();
        } else {
            owner_.log.unimplemented("FrontEndScreen::bind_layout", "004f7590");
        }
    }
    void hide_screen(void* screen) override {
        static_cast<void>(screen);
        owner_.log.unimplemented("FrontEndManager::hide_screen", "004f75b0");
    }
    void destroy_screen(void* screen) override {
        static_cast<void>(screen);
        owner_.log.unimplemented("FrontEndManager::destroy_screen", "004f75e0");
    }
    void cache_music(std::string_view path) override {
        // The FileStore cache and remove pair GVMainMenu uses for its two .fsb
        // paths and their .def siblings (docs/FRONTEND_MANAGERS.md).
        owner_.log.unimplemented("FrontEndManager::cache_music", "004fc150");
        owner_.log.notef("front-end music cached %.*s", static_cast<int>(path.size()),
            path.data());
    }
    void remove_music(std::string_view path) override {
        static_cast<void>(path);
        owner_.log.unimplemented("FrontEndManager::remove_music", "004fc150+remove");
    }
    void register_locale_table(std::string_view name) override {
        owner_.log.unimplemented("FrontEndManager::register_locale_table", "00aa09d0");
        owner_.log.notef("front-end locale table %.*s", static_cast<int>(name.size()),
            name.data());
    }
    void reload_locale_tables() override {
        owner_.log.unimplemented("FrontEndManager::reload_locale_tables", "00aa06d0");
    }
    void set_gui_layer_enabled(int layer, bool enabled) override {
        static_cast<void>(layer);
        static_cast<void>(enabled);
        owner_.log.unimplemented("FrontEndManager::set_gui_layer_enabled", "00aa0e00");
    }
    bool returning_from_mission() override { return false; }   // 00e198b0
    bool mission_result_pending() override { return false; }   // game+1ee1h
    void clear_returning_from_mission() override {
        owner_.log.unimplemented("FrontEndManager::clear_returning_from_mission", "00e198b0");
    }
    bool script_gc_enabled() override { return false; }
    void run_script_gc() override {
        owner_.log.unimplemented("FrontEndManager::run_script_gc", "006b8ad0");
    }
    void set_game_state(int state) override { owner_.state.value = state; }
    OptionsMenuManager* create_options_manager() override {
        owner_.log.unimplemented("FrontEndManager::create_options_manager", "00689800");
        return nullptr;
    }
    MainMenuManager* create_main_menu_manager() override {
        // operator new(78h) then 00686170. Only the fields the recovered Init
        // and destructor touch are modelled.
        owner_.log.implemented("FrontEndManager::create_main_menu_manager", "00686170");
        return &owner_.main_menu;
    }
    MultiMenuManager* create_multi_menu_manager() override {
        owner_.log.unimplemented("FrontEndManager::create_multi_menu_manager", "006887e0");
        return nullptr;
    }
    void refresh_multiplayer_state() override {
        owner_.log.unimplemented("FrontEndManager::refresh_multiplayer_state", "00576b10");
    }
    bool downloadable_content_present() override { return false; }
    void refresh_downloadable_content() override {
        owner_.log.unimplemented("FrontEndManager::refresh_downloadable_content", "00687c00");
    }
    void refresh_online_services() override {
        owner_.log.unimplemented("FrontEndManager::refresh_online_services", "008d2f50");
    }
    bool profile_name_empty() override { return false; }
    void create_default_profile() override {
        owner_.log.unimplemented("FrontEndManager::create_default_profile", "0076f0e0");
    }
    bool online_signed_in() override { return false; }
    void clear_multiplayer_entry_reason() override {
        owner_.log.unimplemented("FrontEndManager::clear_multiplayer_entry_reason", "00e19610");
    }
    void reset_interface_stack() override {
        owner_.log.unimplemented("FrontEndManager::reset_interface_stack", "004d95f0");
    }
    void lower_in_mission_interface() override {
        owner_.log.unimplemented("FrontEndManager::lower_in_mission_interface", "004da780");
    }
    void raise_front_end_dirty_flag() override {
        owner_.log.unimplemented("FrontEndManager::raise_front_end_dirty_flag", "00e08874");
    }

private:
    GameMenuHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 004e4000, BSP_Game_EnterFrontEndShell
// ---------------------------------------------------------------------------

class MenuShellHost final : public FrontEndShellHost {
public:
    MenuShellHost(GameMenuHost::Impl& owner, MenuManagerHost& managers,
        MenuPayloadHost& payloads, MenuScreenSetHost& screens)
        : owner_(owner), managers_(managers), payloads_(payloads), screens_(screens) {}

    void renderer_set_budget(std::uint32_t value) override {
        static_cast<void>(value);
        owner_.log.unimplemented("FrontEndShell::renderer_set_budget", "00f8d394+18h");
    }
    void probe_texture_memory(const char* label) override {
        static_cast<void>(label);
        owner_.log.unimplemented("FrontEndShell::probe_texture_memory", "00f8d394+70h");
    }
    void probe_sound_memory(const char* label) override {
        static_cast<void>(label);
        owner_.log.unimplemented("FrontEndShell::probe_sound_memory", "00a7a460");
    }
    GameplayEffectManagerContext& effect_manager_context() override {
        owner_.log.implemented("FrontEndShell::effect_manager_context", "004c1650");
        return *owner_.effect_context;
    }
    bool title_screen_present() override { return owner_.world.title != nullptr; }
    void destroy_title_screen() override {
        // 004e4071: the 44h object's vtable +0h with 1, then the global is
        // nulled. Its press-start screen stays registered until the screen sets
        // drop it, which is what the pump's exit pass then does.
        owner_.world.title = nullptr;
        owner_.title_init.title_screen = nullptr;
        owner_.log.implemented("FrontEndShell::destroy_title_screen", "004e4071");
    }
    bool front_end_manager_b8_present() override { return owner_.managers.options != nullptr; }
    bool front_end_manager_ac_present() override { return owner_.managers.main_menu != nullptr; }
    bool front_end_manager_b4_present() override { return owner_.managers.multi_menu != nullptr; }
    void open_load_block(const char* label) override {
        owner_.log.unimplemented("FrontEndShell::open_load_block", "00be0a30");
        owner_.log.notef("front-end shell load block %s", label);
    }
    void close_load_block() override {
        owner_.log.unimplemented("FrontEndShell::close_load_block", "00bdcb30");
    }
    LoadingScreenConfig& loading_globals() override { return owner_.loading_globals; }
    void begin_loading(LoadingScreenMode mode) override {
        static_cast<void>(mode);
        // 0057cb60 builds the 58h loading screen and starts its render worker.
        // That screen is another packet; the shell loads synchronously here.
        owner_.log.unimplemented("FrontEndShell::begin_loading", "0057cb60");
    }
    void report_progress(float progress) override {
        static_cast<void>(progress);
        owner_.log.unimplemented("FrontEndShell::report_progress", "0057bec0");
    }
    void end_loading() override {
        owner_.log.unimplemented("FrontEndShell::end_loading", "0057c250");
    }
    void game_on_init() override {
        // 004e3aa0's first store is game+5D4h = 3.
        owner_.state.value = kGameStateFrontEndInit;
        owner_.log.implemented("FrontEndShell::game_on_init", "004e3ac2");
    }
    void poll_platform_session_events() override {
        owner_.log.unimplemented("FrontEndShell::poll_platform_session_events", "004db290");
    }
    std::int32_t game_state() override { return owner_.state.value; }
    void create_manager_b8() override {
        owner_.log.unimplemented("FrontEndShell::create_manager_b8", "00689800");
    }
    void create_manager_ac() override {
        MainMenuManager* manager = managers_.create_main_menu_manager();
        if (manager == nullptr) return;
        init_main_menu_00686380(*manager, owner_.managers, owner_.interface_lock, managers_,
            payloads_, screens_, owner_.manager_registry);
        owner_.managers.main_menu = manager;
        owner_.manager_registry.managers.push_back(&manager->base);
        owner_.log.implemented("FrontEndShell::create_manager_ac", "00686380");
    }
    void create_manager_b4() override {
        owner_.log.unimplemented("FrontEndShell::create_manager_b4", "006887e0");
    }
    void lua_collect_garbage() override {
        owner_.log.unimplemented("FrontEndShell::lua_collect_garbage", "006b8ad0");
    }
    int manager_ac_mode() override {
        return owner_.managers.main_menu != nullptr
            ? owner_.managers.main_menu->base.applied.interface_id : 0;
    }
    void reset_manager_ac_mode() override {
        // 004e4259. Step 9 of the main-menu path owns this site, so the shell
        // records it and leaves the push to the path; see docs/GAME_EXECUTABLE.md.
        owner_.log.implemented("FrontEndShell::reset_manager_ac_mode", "004e4259");
    }
    void manager_ac_enter() override {
        owner_.log.implemented("FrontEndShell::manager_ac_enter", "004e4269");
    }
    void manager_ac_start_sub() override {
        owner_.log.implemented("FrontEndShell::manager_ac_start_sub", "004e4274");
    }
    bool post_state_hook_wanted() override { return false; }
    void post_state_hook() override {
        owner_.log.unimplemented("FrontEndShell::post_state_hook", "004bfc70");
    }
    bool award_gate_open() override { return false; }
    int award_id(const char* key) override {
        static_cast<void>(key);
        owner_.log.unimplemented("FrontEndShell::award_id", "006b8da0");
        return 0;
    }
    bool award_system_ready() override { return false; }
    bool award_session_ready() override { return false; }
    void grant_award(int id) override {
        static_cast<void>(id);
        owner_.log.unimplemented("FrontEndShell::grant_award", "00a410a0");
    }
    void record_award(const char* key, int value) override {
        static_cast<void>(key);
        static_cast<void>(value);
        owner_.log.unimplemented("FrontEndShell::record_award", "007fbe20");
    }
    void send_network_quit() override {
        owner_.log.unimplemented("FrontEndShell::send_network_quit", "0076fad0");
    }

private:
    GameMenuHost::Impl& owner_;
    MenuManagerHost& managers_;
    MenuPayloadHost& payloads_;
    MenuScreenSetHost& screens_;
};

// ---------------------------------------------------------------------------
// The main-menu path itself
// ---------------------------------------------------------------------------

class MenuPathHost final : public MainMenuPathHost {
public:
    MenuPathHost(GameMenuHost::Impl& owner, MenuShellHost& shell, MenuPayloadHost& payloads,
        MenuScreenSetHost& screen_sets, MenuSetBindings& bindings, MenuScreenHost& screens,
        MenuCommitHost& commit, MenuTitleMusicHost& music)
        : owner_(owner), shell_(shell), payloads_(payloads), screen_sets_(screen_sets),
          bindings_(bindings), screens_(screens), commit_(commit), music_(music) {}

    bool input_action_pressed(int action) override {
        if (owner_.press_start_handoff_requested) return true;
        return action == kPressStartInputAction
            && action_pressed_this_frame_004c43c0(owner_.press_action);
    }

    void reset_storage_availability() override {
        owner_.log.unimplemented("TitleSkip::reset_storage_availability", "00bd3450");
    }
    void request_game_state(std::int32_t state) override {
        enqueue_state_request_004d3ed0(owner_.requests, static_cast<std::uint32_t>(state));
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        owner_.log.implemented("TitleSkip::request_game_state", "004d7920");
        owner_.log.notef("state request %d enqueued by the press-start skip", state);
    }
    void release_state_request_hold() override {
        owner_.state.requests_held = false;
        owner_.log.implemented("TitleSkip::release_state_request_hold", "0068d8c6");
    }

    std::int32_t drain_state_request() override {
        if (owner_.requests.count == 0) return 0;
        const std::uint32_t request = front_state_request(owner_.requests);
        pop_front_state_request(owner_.requests);
        owner_.state.pending_requests = static_cast<int>(owner_.requests.count);
        apply_drained_game_state(owner_.state, static_cast<int>(request));
        owner_.log.implemented("MainMenuPath::drain_state_request", "004e4430");
        return static_cast<std::int32_t>(request);
    }
    void enter_front_end_shell() override {
        const FrontEndShellOutcome outcome = bsp::enter_front_end_shell(owner_.shell_state,
            shell_);
        owner_.summary.shell_entered = outcome == FrontEndShellOutcome::ShellReady;
        owner_.log.implemented("MainMenuPath::enter_front_end_shell", "004e4000");
        owner_.log.notef("front-end shell entry outcome=%s",
            outcome == FrontEndShellOutcome::ShellReady ? "ShellReady"
                                                        : "AbortedByPlatformEvent");
    }

    int main_menu_manager_mode() override {
        return owner_.managers.main_menu != nullptr
            ? owner_.managers.main_menu->base.applied.interface_id : 0;
    }
    void push_interface_request(int interface_id, void* payload) override {
        if (owner_.managers.main_menu == nullptr) return;
        push_interface_request_004cc460(owner_.managers.main_menu->base,
            owner_.interface_lock, interface_id, payload, payloads_);
        owner_.log.implemented("MainMenuPath::push_interface_request", "004cc460");
    }
    void activate_main_menu_manager() override {
        if (owner_.managers.main_menu == nullptr) return;
        activate_front_end_manager_00684700(owner_.manager_registry,
            owner_.managers.main_menu->base, owner_.managers.in_mission,
            owner_.interface_lock, payloads_, screen_sets_);
        owner_.summary.main_menu_manager_active = owner_.managers.main_menu->base.active;
        owner_.log.implemented("MainMenuPath::activate_main_menu_manager", "00684700");
    }
    void start_main_menu_screen_object() override {
        start_title_music_005884a0(owner_.title_music, kMainMenuTitleMusic, music_);
        owner_.log.implemented("MainMenuPath::start_main_menu_screen_object", "005884a0");
    }
    void set_game_state(std::int32_t state) override {
        owner_.state.value = state;
        owner_.log.implemented("MainMenuPath::set_game_state", "004e4279");
        owner_.log.notef("game state %d (front-end shell ready)", state);
    }

    bool service_pending_interface_requests() override {
        const bool serviced = service_pending_interface_requests_006840f0(owner_.managers,
            owner_.interface_lock, payloads_, screen_sets_);
        owner_.log.implemented("MainMenuPath::service_pending_interface_requests", "006840f0");
        if (serviced) return true;
        // Run-time correction. On a cold boot 00686380 has already run 00684600
        // with INTF_MAINMENU at 006868AD, so manager+4h equals manager+20h
        // before the shell's push at 004E4259 and 006840F0 has nothing to
        // service. docs/MAIN_MENU_PATH.md's step 13 assumes a mismatch that this
        // path does not produce, so the channel is reported as already applied
        // and the path publishes the screen set itself.
        MainMenuManager* manager = owner_.managers.main_menu;
        return manager != nullptr && manager->base.active
            && manager->base.applied.interface_id == manager->base.pending.interface_id
            && manager->base.applied.interface_id == kMainMenuPathInterfaceId;
    }
    bool apply_interface_request(int interface_id, void* payload) override {
        if (owner_.managers.main_menu == nullptr) return false;
        const bool applied = apply_interface_request_00684600(owner_.managers.main_menu->base,
            owner_.interface_lock, interface_id, payload, payloads_);
        owner_.log.implemented("MainMenuPath::apply_interface_request", "00684600");
        return applied;
    }
    int map_interface_to_screen(int interface_id) override {
        // 00685848, the identity arm of the main menu's own +10h override.
        const int screen_id = main_menu_screen_id_00685820(interface_id);
        owner_.log.implemented("MainMenuPath::map_interface_to_screen", "00685820");
        return screen_id;
    }

    void publish_screen_set_level4(const int* ids, std::size_t count) override {
        set_front_end_screen_set_004f8710(owner_.screen_sets, owner_.world.screens,
            bindings_, ids, count);
        owner_.log.implemented("MainMenuPath::publish_screen_set_level4", "004f8710");
        owner_.summary.published_screen_id = count != 0 && ids != nullptr ? ids[0] : 0;
        owner_.log.notef("level-4 screen set published: %zu id(s), first=%d", count,
            owner_.summary.published_screen_id);
    }
    void publish_input_context_set_level4(const int* ids, std::size_t count) override {
        set_game_input_context_set_004d8c00(owner_.input_contexts, ids, count);
        owner_.log.implemented("MainMenuPath::publish_input_context_set_level4", "004d8c00");
    }

    FrontEndScreenPhase pump_front_end_screens(float raw_delta) override {
        // 004c4165 inside BSP_Game_UpdateInterfaceOnly 004c40f0, which OnMove
        // calls unconditionally at 004e53b6 once the state leaves {1, 2, 4}.
        const FrontEndScreen* screen = owner_.world.screens.slots[
            owner_.path.published_screen_id >= 0
                && owner_.path.published_screen_id < kFrontEndScreenSlotCount
                    ? owner_.path.published_screen_id : 0];
        const FrontEndScreenPhase phase = screen != nullptr
            ? front_end_screen_phase(*screen) : FrontEndScreenPhase::Hidden;
        owner_.pump(raw_delta);
        return phase;
    }
    void commit_screen_visibility(int screen_id, bool visible) override {
        // 004f88e8. The pump above already ran 004f83b0 for this screen in its
        // enter pass; re-applying the same byte is idempotent and keeps the
        // native call site on the record.
        static_cast<void>(visible);
        screens_.screen_commit(screen_id);
        static_cast<void>(commit_);
    }
    void enter_screen(int screen_id) override {
        // 004f88f4. The pump's enter pass already called the same virtual.
        static_cast<void>(screen_id);
        owner_.log.notef("main-menu screen %d entered by the pump at 004f88f4", screen_id);
    }
    void update_screen(int screen_id, float raw_delta) override {
        static_cast<void>(screen_id);
        static_cast<void>(raw_delta);
    }

private:
    GameMenuHost::Impl& owner_;
    MenuShellHost& shell_;
    MenuPayloadHost& payloads_;
    MenuScreenSetHost& screen_sets_;
    MenuSetBindings& bindings_;
    MenuScreenHost& screens_;
    MenuCommitHost& commit_;
    MenuTitleMusicHost& music_;
};

// The singleton lifetime domain the gameplay effect manager registers with.
// 004e4000 probes that registry at 004e4062 as stripped instrumentation, so the
// domain exists only to satisfy the probe.
void menu_lifetime_destroy(void* context, void* owner, std::uint32_t flags) noexcept {
    static_cast<void>(context);
    static_cast<void>(owner);
    static_cast<void>(flags);
}
void menu_lifetime_invalid_parameter(void* context) noexcept { static_cast<void>(context); }

}  // namespace

// ---------------------------------------------------------------------------
// GameMenuHost::Impl
// ---------------------------------------------------------------------------

GameMenuHost::Impl::Impl(GameHostLog& log_in, GameFrontendHost& frontend_in,
    GameStateSlot& state_in, long press_start_frame_in)
    : log(log_in), frontend(frontend_in), state(state_in),
      press_start_frame(press_start_frame_in) {
    summary.press_start_frame = press_start_frame_in;
    world.title = nullptr;
    lifetime = std::make_unique<SingletonLifetimeDomain>(SingletonLifetimeCallbacks{
        nullptr, &menu_lifetime_destroy, &menu_lifetime_invalid_parameter});
    effect_context = std::make_unique<GameplayEffectManagerContext>(
        GameplayEffectManagerContext{*lifetime, effect_singleton, effect_allocation_words});
    // The press-start action record is enabled so 004c43c0's own gate at
    // 00a92c88 does not skip it; every other action has no record at all.
    press_action.enabled = true;
    press_action.registered = true;
}

GameMenuHost::Impl::MenuScreen* GameMenuHost::Impl::screen_at(int slot) {
    const auto found = screens.find(slot);
    return found != screens.end() ? &found->second : nullptr;
}

GameMenuHost::Impl::MenuScreen& GameMenuHost::Impl::add_screen(int slot, std::string name,
    FrontEndScreen* external, std::uint32_t enter_virtual, std::uint32_t exit_virtual,
    std::uint32_t update_virtual, bool is_press_start) {
    MenuScreen& screen = screens[slot];
    screen.slot = slot;
    screen.name = std::move(name);
    screen.flags = external != nullptr ? external : &screen.owned_flags;
    screen.enter_virtual = enter_virtual;
    screen.exit_virtual = exit_virtual;
    screen.update_virtual = update_virtual;
    screen.press_start = is_press_start;
    return screen;
}

void GameMenuHost::Impl::attach_page(MenuScreen& screen, const std::string& layout_name) {
    GuiLayoutPage* page = frontend.load_screen_page(layout_name);
    log.implemented("FrontEndScreen::load_layout", "00aa5840");
    if (page == nullptr) {
        log.notef("screen %d layout %s did not load", screen.slot, layout_name.c_str());
        return;
    }
    screen.pages.push_back(page);
    // 004f7180 leaves a screen neither wanted nor active, so the byte 004f83b0
    // would publish for a freshly bound layout is that screen's current +5h.
    frontend.commit_page_visibility(*page, screen.flags != nullptr && screen.flags->active);
}

void GameMenuHost::Impl::pump(float raw_delta) {
    MenuCommitHost commit(*this);
    MenuPressStartHost press(*this);
    MenuScreenHost screens_host(*this, press, commit);
    run_front_end_screen_pump(world, screens_host, raw_delta);
    pumped_this_frame = true;
    ++summary.pump_frames;
    log.implemented("FrontEndScreens::pump", "004f8830");
}

void GameMenuHost::Impl::advance_path(float raw_delta) {
    MenuCommitHost commit(*this);
    MenuPressStartHost press(*this);
    MenuScreenHost screens_host(*this, press, commit);
    MenuSetBindings bindings(*this);
    MenuScreenSetHost screen_set_host(*this, bindings);
    MenuPayloadHost payloads(*this);
    MenuManagerHost managers_host(*this);
    MenuShellHost shell(*this, managers_host, payloads, screen_set_host);
    MenuTitleMusicHost music(*this);
    MenuPathHost path_host(*this, shell, payloads, screen_set_host, bindings, screens_host,
        commit, music);

    // The native takes the whole run from the press to state 5 inside one
    // OnMove call and only reaches the pump on the following frame, so the loop
    // stops as soon as it needs a pump this frame has already spent.
    for (int guard = 0; guard < 24; ++guard) {
        const MainMenuPathStep next = advance_main_menu_path(path_step, path, path_host,
            raw_delta);
        if (next == path_step) break;
        log.notef("main-menu path %s -> %s", path_step_text(path_step).c_str(),
            path_step_text(next).c_str());
        path_step = next;
        summary.path_step = path_step_text(path_step);
        if (path_step == MainMenuPathStep::ScreenVisible) break;
        if (path_step == MainMenuPathStep::EnterScreen && pumped_this_frame) break;
    }
    press_start_handoff_requested = false;
}

// ---------------------------------------------------------------------------
// GameMenuHost
// ---------------------------------------------------------------------------

GameMenuHost::GameMenuHost(GameHostLog& log, GameFrontendHost& frontend, GameStateSlot& state,
    long press_start_frame)
    : impl_(std::make_unique<Impl>(log, frontend, state, press_start_frame)) {}

GameMenuHost::~GameMenuHost() = default;

void GameMenuHost::run_title_init_004c9a70() {
    Impl& host = *impl_;
    MenuCommitHost commit(host);
    MenuPressStartHost press(host);
    MenuFrameLayoutHost layouts(host);
    MenuTitleHandoverHost handover(host, press, commit);
    MenuTitleInitHost title(host, layouts, handover, commit);

    host.title_init.skip_title = false;
    host.handover.skip_title = false;
    host.handover.requests_held = host.state.requests_held;
    run_title_init(host.title_init, title);
    // 004c9b0d writes game+5D4h = 2 through the same field the frame reads.
    host.state.value = static_cast<int>(host.title_init.game_state);
    host.summary.title_init_ran = true;
    host.summary.game_state = host.state.value;
    host.path.game_state = host.state.value;
    host.path.state_requests_held = host.state.requests_held;
    host.log.implemented("Title bring-up GGame::OnInitTitle", "004c9a70");
    host.log.notef("title init: game state %d, press-start slot %d, registry screens %zu",
        host.state.value, host.summary.press_start_slot, host.screens.size());
    host.frontend.invalidate_bridge();
}

bool GameMenuHost::input_action_pressed(int action) {
    Impl& host = *impl_;
    if (action != kPressStartInputAction) return false;
    return action_pressed_this_frame_004c43c0(host.press_action);
}

void GameMenuHost::frame(float raw_delta, unsigned long long frame_index) {
    Impl& host = *impl_;
    host.pumped_this_frame = false;

    // 00a92370's prologue: the current pair becomes the previous pair and the
    // current pair is zeroed, which is what makes the injected press one frame
    // long. 00a92aa0 then produces exactly the rising edge 004c43c0 reports.
    if (host.press_start_frame >= 0
        && frame_index == static_cast<unsigned long long>(host.press_start_frame)) {
        const InputEffectParam no_param{};
        start_action_00a92aa0(host.press_action, 1.0f, no_param, nullptr);
        host.summary.press_start_injected = true;
        host.log.notef("press-start input action %d injected on frame %llu",
            kPressStartInputAction, frame_index);
    } else {
        begin_action_frame_00a92370(host.press_action);
    }

    if (is_front_end_game_state(host.state.value)) {
        // 004e4b9d. The branch ends by pumping the registry at 004e4ca3.
        MenuCommitHost commit(host);
        MenuPressStartHost press(host);
        MenuScreenHost screens_host(host, press, commit);
        MenuTitleHandoverHost handover(host, press, commit);
        MenuFrameHost frame_host(host, handover);
        run_front_end_state_frame(host.frame_state, host.world, frame_host, screens_host,
            raw_delta);
        // 004e4ca3, the pump the branch's shared tail runs.
        host.log.implemented("FrontEndScreens::pump", "004f8830");
        host.pumped_this_frame = true;
        ++host.summary.pump_frames;
    } else {
        // 004e53b6, BSP_Game_UpdateInterfaceOnly, which OnMove calls on every
        // frame that does not take the front-end branch.
        host.log.implemented("FrontEndScreens::update_interface_only", "004c40f0");
        if (host.path_step == MainMenuPathStep::ScreenVisible) {
            // Once the path has settled the pump at 004c4165 is the only
            // front-end work left in the frame; before that the path's own
            // EnterScreen step performs it.
            host.pump(raw_delta);
        }
    }

    host.advance_path(raw_delta);
    host.summary.game_state = host.state.value;
    host.frame_state.render_queue_open = false;
}

const GameMenuSummary& GameMenuHost::summary() const noexcept { return impl_->summary; }

}  // namespace bsp::game
