#include "bsp/title_init.hpp"

#include "bsp/frame_clock.hpp"

// GGame::OnInitTitle (004c9a70), the front-end frame layout selector (00518250)
// and the logo sequence advance (00685070) and skip poll (00685170).
// docs/GAME_TITLE_INIT.md holds the evidence for every statement order below.
namespace bsp {
namespace {

// 00E08500, 00E08604 and 00E0862C read out of the image. Set 4 is empty in all
// three tables and every backdrop row except set 2 is empty as well.
constexpr FrontEndFrameLayoutNames kLayoutNames = {
    // backdrop, 00E08500, 13 slots per set
    {
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, "FE_options_bg", nullptr, nullptr,
            nullptr, nullptr, nullptr, "FE_main"},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr},
        {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            nullptr, nullptr, nullptr},
    },
    // panel, 00E08604, 2 slots per set
    {
        {"FE_frame", nullptr},
        {"FE_frame", nullptr},
        {"FE_frame", nullptr},
        {"GUI_pause", nullptr},
        {nullptr, nullptr},
    },
    // title, 00E0862C, 2 slots per set
    {
        {"FE_frame_title", nullptr},
        {"FE_frame_title", nullptr},
        {"FE_frame_title", nullptr},
        {"GUI_pause_title", nullptr},
        {nullptr, nullptr},
    },
};

// The release decision at 00518286..005182bc, applied to one handle slot. The
// refcount word is read before the branch, so a handle whose word is exactly 1
// goes back to the GUI manager and every other handle is only dereferenced.
void release_layout_slot(void*& handle, FrontEndFrameLayoutHost& host)
{
    if (handle == nullptr) {
        return;
    }
    if (host.gui_layout_refcount(handle) == 1) {
        host.gui_manager_release_layout(handle);
    } else {
        host.gui_layout_release_ref(handle);
    }
    handle = nullptr;
}

// One release pass: the native code walks slots on the outside and sets on the
// inside, skipping the set it is about to load. Order is preserved because the
// GUI manager sees the releases in it.
template <int Slots>
void release_other_sets(void* (&table)[kFrontEndFrameSetCount][Slots],
    FrontEndFrameLayoutHost& host, int keep_set)
{
    for (int slot = 0; slot < Slots; ++slot) {
        for (int set = 0; set < kFrontEndFrameSetCount; ++set) {
            if (set != keep_set) {
                release_layout_slot(table[set][slot], host);
            }
        }
    }
}

// One load pass: only a null handle is filled, and only from a non-null name.
template <int Slots>
void acquire_set(void* (&table)[kFrontEndFrameSetCount][Slots],
    const char* const (&names)[kFrontEndFrameSetCount][Slots], FrontEndFrameLayoutHost& host,
    int set)
{
    for (int slot = 0; slot < Slots; ++slot) {
        if (table[set][slot] != nullptr) {
            continue;
        }
        const char* name = names[set][slot];
        table[set][slot] = (name != nullptr) ? host.gui_layout_acquire(name) : nullptr;
    }
}

} // namespace

const FrontEndFrameLayoutNames& front_end_frame_layout_names() noexcept
{
    return kLayoutNames;
}

void select_front_end_frame_set(FrontEndFrameLayouts& layouts, FrontEndFrameLayoutHost& host,
    int set, bool commit)
{
    // 00518272: the whole body is skipped when the set is already current.
    if (set == layouts.active_set) {
        return;
    }
    if (set < 0 || set >= kFrontEndFrameSetCount) {
        // The native code indexes the tables without a bound check; a caller
        // outside 0..4 would read past them. Refuse instead of reproducing that.
        return;
    }

    if (commit) {
        release_other_sets(layouts.backdrop, host, set);
        release_other_sets(layouts.panel, host, set);
        release_other_sets(layouts.title, host, set);
    }

    acquire_set(layouts.backdrop, kLayoutNames.backdrop, host, set);
    acquire_set(layouts.panel, kLayoutNames.panel, host, set);
    acquire_set(layouts.title, kLayoutNames.title, host, set);

    // 0051864a: the current set is only recorded on a committing call, so a
    // non-committing call loads without making the set current.
    if (commit) {
        layouts.active_set = set;
    }
}

bool logo_skip_allowed(float elapsed_seconds, float entry_delay, bool skip_action_pressed) noexcept
{
    // FCOMIP then JBE at 006851aa: strictly greater, and NaN takes the JBE.
    return (elapsed_seconds > entry_delay) && skip_action_pressed;
}

LogoAdvanceOutcome logo_advance_or_finish(LogoSequenceState& state, LogoSequenceHost& host)
{
    const std::uint32_t index = state.next_index;
    // 00685077..0068508f: a null entry vector has count zero; the finish test
    // does not inspect the separate delay vector.
    const std::size_t entry_count = state.entries ? state.entry_count : 0;
    if (index >= entry_count) {
        // 00685091: virtual +0h with 1, then BSP_Game_OnInitOnce(game, 0).
        host.destroy_self();
        host.reenter_title();
        return LogoAdvanceOutcome::SequenceFinished;
    }

    state.next_index = index + 1;
    host.install_movie_completion_callback();
    // The callback registration can mutate state. Preserve native re-reads
    // and permit the CRT handler to repair storage before returning.
    if (state.entries == nullptr || index >= state.entry_count)
        host.invalid_parameter_noinfo_00bf6713();
    host.movie_play(state.entries[index], 1, 0.0F, 0);
    host.movie_screen_enter();
    state.entry_started = host.now();
    if (state.delays == nullptr || index >= state.delay_count)
        host.invalid_parameter_noinfo_00bf6713();
    state.entry_delay = state.delays[index];
    return LogoAdvanceOutcome::EntryStarted;
}

bool logo_poll_skip(LogoSequenceState& state, LogoSequenceHost& host, const ClockTimestamp& now,
    bool skip_action_pressed, LogoAdvanceOutcome* outcome)
{
    ClockTimestamp elapsed{};
    subtract_timestamp_00530890(elapsed, now, state.entry_started);
    if (!logo_skip_allowed(timestamp_seconds_x87(elapsed), state.entry_delay, skip_action_pressed)) {
        return false;
    }
    const LogoAdvanceOutcome result = logo_advance_or_finish(state, host);
    if (outcome != nullptr) {
        *outcome = result;
    }
    return true;
}

void run_title_init(TitleInitState& state, TitleInitHost& host)
{
    // 004c9a9f..004c9b00: the scope label is built as a pooled string, handed
    // to the file block, and freed before the first real step.
    host.open_named_block(kTitleInitScopeLabel);

    host.reset_player_profile(); // 004c9b00, ECX = game+650h
    state.game_state = GameStartupState::kTitleScreen; // 004c9b0d
    host.load_texture_atlas(kTitleInitAtlas); // 004c9b52
    host.select_front_end_frame_set(FrontEndFrameSet::Title, true); // 004c9b7e/004c9b85

    // 004c9b8a: the attract screen is built once and never rebuilt, and its
    // init virtual runs on the branch that built it only.
    if (state.attract_screen == nullptr) {
        state.attract_screen = host.create_attract_screen();
        host.attract_screen_init(state.attract_screen);
    }

    // 004c9bc7: skipTitle takes the direct enqueue instead of the screen.
    if (state.skip_title) {
        host.title_screen_skip();
    } else if (state.title_screen == nullptr) {
        state.title_screen = host.create_title_screen();
        host.title_screen_activate(state.title_screen);
    }

    // 004c9c13: only when the mission-side record block still exists.
    if (state.mission_player_records != nullptr) {
        host.reset_mission_player_records(state.mission_player_records);
    }
    state.front_end_slot = 0; // 004c9c2b

    // 004c9c31: an incoherent sign-in state costs the sign-in reset and a
    // rebind of the primary input device.
    if (!host.sign_in_state_valid()) {
        host.reset_sign_in_state();
        host.rebind_primary_input();
    }

    // 004c9c4a: close the movie screen. This is 004b6e50 inlined: run the exit
    // virtual only if it was active, clear both flag bytes either way, then
    // commit the cleared visibility to the children.
    if (state.movie_screen.active) {
        host.movie_screen_exit();
    }
    state.movie_screen.wanted = false;
    state.movie_screen.active = false;
    host.movie_screen_commit();

    host.close_named_block(); // 004c9c79
}

} // namespace bsp
