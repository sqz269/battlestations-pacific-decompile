#include "bsp/movie_player.hpp"

namespace bsp {
static_assert(sizeof(void*) == 4, "MoviePlayer requires the supported Win32 target");
static_assert(sizeof(FrontEndScreen) == 2);
static_assert(offsetof(MoviePlayer, visibility) == 0x04);
static_assert(offsetof(MoviePlayer, callback_owner_vtable) == 0x08);
static_assert(offsetof(MoviePlayer, page) == 0x1c);
static_assert(offsetof(MoviePlayer, selected_movie) == 0x20);
static_assert(offsetof(MoviePlayer, shrink_wide_movie) == 0x24);
static_assert(offsetof(MoviePlayer, normal_movie) == 0x28);
static_assert(offsetof(MoviePlayer, black_backdrop) == 0x2c);
static_assert(offsetof(MoviePlayer, allow_skip) == 0x30);
static_assert(sizeof(MoviePlayer) == 0x34);

void set_movie_widget_source_00aae320(MovieWidgetState& widget,
    NativeStringStorage& storage, const NativeString& filename, std::uint8_t loop) {
    widget.filename.copy_from_00be0a30_fragment(storage, filename);
    widget.loop = loop;
}

void set_movie_widget_completion_00aafee0(MovieWidgetState& widget,
    MovieCompletionCallback callback) noexcept {
    widget.completion = callback;
}

void start_movie_widget_00ab0eb0(MovieWidgetState& widget, MovieWidgetHost& host) {
    if (widget.filename.length() == 0) {
        return;
    }
    if (widget.decoder == nullptr) {
        host.ensure_movie_resources(widget);
    }
    host.prepare_movie_subtitles(widget, widget.filename);
    while (!host.decoder_open(widget.decoder, widget.filename, true, 0)) {
        if (host.missing_movie_retry_dialog() == 2) {
            if (widget.completion != nullptr) {
                widget.completion();
            }
            return;
        }
    }
    host.decoder_set_volume_immediate(widget.decoder, 1.0f);
    host.decoder_set_loop(widget.decoder, widget.loop);
    host.decoder_prepare_frame(widget.decoder);
    host.bind_movie_textures(widget);
    host.decoder_set_running(widget.decoder, true);
}

void stop_movie_widget_00aac870(MovieWidgetState& widget, MovieWidgetHost& host) {
    if (widget.decoder != nullptr) {
        host.decoder_set_running(widget.decoder, false);
        host.decoder_close(widget.decoder);
    }
}

void movie_widget_visibility_changed_00ab0f90(MovieWidgetState& widget,
    MovieWidgetHost& host, bool visible) {
    if (visible) {
        start_movie_widget_00ab0eb0(widget, host);
    } else {
        stop_movie_widget_00aac870(widget, host);
    }
}

bool movie_widget_is_playing_00aac8e0(MovieWidgetState& widget, MovieWidgetHost& host) {
    return widget.decoder != nullptr && !host.decoder_completed(widget.decoder);
}

void skip_movie_widget_00ab04a0(MovieWidgetState& widget, MovieWidgetHost& host) {
    host.decoder_mark_completed(widget.decoder, true);
    host.clear_movie_subtitles(widget);
}

void update_movie_widget_00aadf40(MovieWidgetState& widget, MovieWidgetHost& host, float delta) {
    host.update_gui_widget(widget, delta);
    host.bind_movie_textures(widget);
    host.decoder_update(widget.decoder);
    if (!host.decoder_has_open_handle(widget.decoder)) {
        return;
    }
    if (!host.decoder_completed(widget.decoder)) {
        host.update_movie_subtitles(widget);
        return;
    }
    if (widget.completion != nullptr) {
        host.decoder_close(widget.decoder);
        // Native reloads +F8h after close. Do not cache it or add another guard.
        widget.completion();
    }
}

MoviePlayer& construct_movie_player_004f8af0(MoviePlayer& player) noexcept {
    player.screen_vtable = 0x00ceae54;
    player.visibility.wanted = false;
    player.visibility.active = false;
    player.callback_owner_vtable = 0x00ce3cd4;
    player.callback_owner_0c = nullptr;
    player.callback_owner_10 = nullptr;
    player.callback_owner_14 = 0;
    player.callback_owner_18 = false;
    player.page = nullptr;
    player.selected_movie = nullptr;
    player.allow_skip = false;
    player.screen_vtable = 0x00ceae94;
    player.callback_owner_vtable = 0x00ceae7c;
    return player;
}

void initialize_movie_player_004f8cf0(MoviePlayer& player,
    FrontEndScreenTable& screens, MoviePlayerHost& host) {
    screens.slots[kMoviePlayerScreenSlot] = &player.visibility;
    player.page = host.load_movie_page("_FullScreenMovies", 1, 0);
    host.set_widget_visible(player.page, false);
    player.shrink_wide_movie = host.find_movie_page_child(player.page, "ShrinkWideScreen_Movie", true);
    player.normal_movie = host.find_movie_page_child(player.page, "MoviePlay_Movie", true);
    player.black_backdrop = host.find_movie_page_child(player.page, "BlackMovie_Icon", true);
    host.set_widget_visible(player.black_backdrop, false);
}

void set_movie_completion_004f8970(MoviePlayer& player, MoviePlayerHost& host,
    MovieCompletionCallback callback) {
    set_movie_widget_completion_00aafee0(host.movie_widget_state(player.shrink_wide_movie), callback);
    set_movie_widget_completion_00aafee0(host.movie_widget_state(player.normal_movie), callback);
}

void play_movie_004f8a20(MoviePlayer& player, MoviePlayerHost& host,
    const NativeString& filename, std::uint8_t prefer_shrink_wide, float local_z,
    std::uint8_t loop) {
    player.allow_skip = false;
    host.set_widget_visible(player.shrink_wide_movie, false);
    host.set_widget_visible(player.normal_movie, false);
    player.selected_movie = prefer_shrink_wide != 0 && !host.platform_flag_0d()
        ? player.shrink_wide_movie : player.normal_movie;
    set_movie_widget_source_00aae320(host.movie_widget_state(player.selected_movie),
        host.movie_string_storage(), filename, loop);
    host.set_widget_local_z(player.selected_movie, local_z);
    host.set_widget_visible(player.selected_movie, true);
    host.set_widget_visible(player.black_backdrop, true);
    host.set_widget_local_z(player.black_backdrop,
        static_cast<float>(static_cast<double>(local_z) + 1.0));
    player.visibility.wanted = true;
}

void stop_movie_004f8ac0(MoviePlayer& player, MoviePlayerHost& host) {
    if (player.selected_movie != nullptr) {
        stop_movie_widget_00aac870(host.movie_widget_state(player.selected_movie), host);
    }
    host.set_widget_visible(player.black_backdrop, false);
    player.visibility.wanted = false;
}

bool movie_is_playing_004f89a0(MoviePlayer& player, MoviePlayerHost& host) {
    return player.selected_movie != nullptr &&
        movie_widget_is_playing_00aac8e0(host.movie_widget_state(player.selected_movie), host);
}

void update_movie_player_004f8c60(MoviePlayer& player, MoviePlayerHost& host, float delta) {
    (void)delta; // Native RET4 consumes but never reads its frame argument.
    if (player.selected_movie == nullptr) {
        return;
    }
    if ((host.mission_context_present() || player.allow_skip) &&
        (host.movie_input_action_pressed(3) || host.movie_input_action_pressed(1))) {
        host.movie_input_action_pressed(1);
        host.movie_input_action_pressed(3);
        skip_movie_widget_00ab04a0(host.movie_widget_state(player.selected_movie), host);
    }
    host.notify_movie_frame(0x0d);
}

void collect_movie_player_children_004f8f00(const MoviePlayer& player,
    std::vector<void*>& children) {
    children.push_back(player.page);
}

namespace {
class MovieCommitHost final : public FrontEndScreenCommitHost {
public:
    MovieCommitHost(MoviePlayer& player, MoviePlayerHost& host) : player_(player), host_(host) {}
    void collect_screen_children(int, std::vector<void*>& children) override {
        collect_movie_player_children_004f8f00(player_, children);
    }
    void set_child_visible(void* child, bool visible) override {
        host_.set_widget_visible(child, visible);
    }
private:
    MoviePlayer& player_;
    MoviePlayerHost& host_;
};
}

void enter_movie_player(MoviePlayer& player, MoviePlayerHost& host) {
    player.visibility.wanted = true;
    player.visibility.active = true;
    MovieCommitHost commit(player, host);
    commit_front_end_screen_visibility_004f83b0(player.visibility, kMoviePlayerScreenSlot, commit);
    // Native +18h is the empty 004F8940.
}

void destroy_movie_player_004f8be0(MoviePlayer& player,
    FrontEndScreenTable& screens, MoviePlayerHost& host) {
    player.screen_vtable = 0x00ceae94;
    player.callback_owner_vtable = 0x00ceae7c;
    host.release_movie_page(player.page);
    host.destroy_movie_callback_owner(player);
    // Base destructor 004F71A0 clears every alias in the 95-slot registry.
    player.screen_vtable = 0x00ceae54;
    for (auto& screen : screens.slots) {
        if (screen == &player.visibility) {
            screen = nullptr;
        }
    }
}

void finish_game_movie_004c7ed0(GameMovieCompletionState& state, MoviePlayer& player,
    MoviePlayerHost& player_host, GameMovieCompletionHost& game_host) {
    state.movie_request_active = false;
    game_host.set_movie_input_context(0x10, false);
    stop_movie_004f8ac0(player, player_host);
    player.visibility.wanted = false;
    state.requests_held = false;
}

void default_movie_completion_004f89d0(GameMovieCompletionState& state,
    MoviePlayer& player, MoviePlayerHost& player_host, GameMovieCompletionHost& game_host) {
    const int initial_state = game_host.current_game_state();
    if (initial_state == 1 || initial_state == 2 || initial_state == 4) {
        return;
    }
    finish_game_movie_004c7ed0(state, player, player_host, game_host);
    const int current_state = game_host.current_game_state();
    if (current_state == 0x0d || current_state == 0x0f) {
        game_host.set_movie_cinematic_mode(false, 0, true);
    }
}

} // namespace bsp
