#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "bsp/frontend_screen_sets.hpp"
#include "bsp/native_string.hpp"

namespace bsp {

using MovieCompletionCallback = void (*)();
inline constexpr int kMoviePlayerScreenSlot = 0x56;

// Native Win32 34h layout, not a C++ virtual class. The two recorded vtable
// addresses are evidence identifiers, never callable pointers in this program.
// Construction deliberately leaves +24h/+28h/+2Ch and padding untouched;
// initialize_movie_player fills those three widget handles before use.
struct MoviePlayer {
    std::uint32_t screen_vtable;             // +00h, 00CEAE94
    FrontEndScreen visibility;              // +04h wanted, +05h applied
    std::uint8_t padding_06[2];
    std::uint32_t callback_owner_vtable;     // +08h, 00CEAE7C
    void* callback_owner_0c;
    void* callback_owner_10;
    std::uint32_t callback_owner_14;
    bool callback_owner_18;
    std::uint8_t padding_19[3];
    void* page;                             // +1Ch _FullScreenMovies
    void* selected_movie;                   // +20h, retained by stop
    void* shrink_wide_movie;                // +24h ShrinkWideScreen_Movie
    void* normal_movie;                     // +28h MoviePlay_Movie
    void* black_backdrop;                   // +2Ch BlackMovie_Icon
    bool allow_skip;                        // +30h, cleared on each play
    std::uint8_t padding_31[3];
};

// Projection of cGuiMovie fields used by the recovered playback bodies. The
// native GUI/scene fields and subtitle collections remain owned by the host.
// This projection is NOT a replacement native cGuiMovie object.
struct MovieWidgetState {
    void* decoder{nullptr};                 // native +F0h
    MovieCompletionCallback completion{nullptr}; // +F8h, no stack arguments
    NativeString filename{};               // +FCh/+100h
    std::uint8_t loop{0};                   // +105h, original byte preserved
};

// Renderer/codec/VFS boundaries. No default implementations or fake codecs.
// A native adapter must preserve the widget ECX when calling native callbacks;
// the reconstructed callbacks 004F89D0 and 00685060 do not consume that register.
struct MovieWidgetHost {
    virtual ~MovieWidgetHost() = default;
    virtual NativeStringStorage& movie_string_storage() = 0;
    // Widget virtual +78h (00AAE770): creates decoder and render geometry.
    virtual void ensure_movie_resources(MovieWidgetState& widget) = 0;
    virtual void prepare_movie_subtitles(MovieWidgetState& widget,
        const NativeString& filename) = 0; // 00AB0520
    virtual bool decoder_open(void* decoder, const NativeString& filename,
        bool argument_2, int argument_3) = 0; // decoder virtual +0Ch
    // MessageBoxA(null, original missing-movie text, "Error", 15h).
    virtual int missing_movie_retry_dialog() = 0;
    virtual void decoder_set_volume_immediate(void* decoder, float value) = 0; // 00A4CBD0
    virtual void decoder_set_loop(void* decoder, std::uint8_t loop) = 0; // virtual +28h
    virtual void decoder_prepare_frame(void* decoder) = 0; // virtual +34h
    virtual void bind_movie_textures(MovieWidgetState& widget) = 0; // 00AAC820, ensures resources
    virtual void decoder_set_running(void* decoder, bool running) = 0; // virtual +1Ch
    virtual void decoder_close(void* decoder) = 0; // virtual +10h
    virtual bool decoder_completed(void* decoder) = 0; // virtual +20h
    virtual void decoder_mark_completed(void* decoder, bool completed) = 0; // virtual +24h
    virtual void decoder_update(void* decoder) = 0; // virtual +2Ch
    virtual bool decoder_has_open_handle(void* decoder) = 0; // decoder+4h
    virtual void update_gui_widget(MovieWidgetState& widget, float delta) = 0; // 00AA87B0
    virtual void update_movie_subtitles(MovieWidgetState& widget) = 0; // 00AADBA0
    virtual void clear_movie_subtitles(MovieWidgetState& widget) = 0; // 00AB00C0
};

// 00AAE320, __thiscall(widget, NativeString const*, byte loop), RET 8.
// Only stores the source and loop byte; becoming visible starts the decoder.
void set_movie_widget_source_00aae320(MovieWidgetState& widget,
    NativeStringStorage& storage, const NativeString& filename, std::uint8_t loop);
// 00AAFEE0, __thiscall(widget, void(*)()), RET 4.
void set_movie_widget_completion_00aafee0(MovieWidgetState& widget,
    MovieCompletionCallback callback) noexcept;
// 00AB0EB0, __thiscall(widget), RET. Retry/cancel and full successful start.
void start_movie_widget_00ab0eb0(MovieWidgetState& widget, MovieWidgetHost& host);
// 00AAC870, __thiscall(widget), RET: running=false then close, iff decoder exists.
void stop_movie_widget_00aac870(MovieWidgetState& widget, MovieWidgetHost& host);
// 00AB0F90, __thiscall(widget, byte visible), RET 4; GUI activation hook.
void movie_widget_visibility_changed_00ab0f90(MovieWidgetState& widget,
    MovieWidgetHost& host, bool visible);
// 00AAC8E0, __thiscall(widget), RET: decoder exists and !completed.
bool movie_widget_is_playing_00aac8e0(MovieWidgetState& widget, MovieWidgetHost& host);
// 00AB04A0, __thiscall(widget), RET: mark complete, then clear subtitle state.
void skip_movie_widget_00ab04a0(MovieWidgetState& widget, MovieWidgetHost& host);
// 00AADF40, __thiscall(widget, float), RET 4. Completion closes first, then
// rereads/invokes the callback; nothing accesses the widget after that call.
void update_movie_widget_00aadf40(MovieWidgetState& widget, MovieWidgetHost& host, float delta);

struct MoviePlayerHost : MovieWidgetHost {
    virtual void* load_movie_page(const char* name, int cache, int mode) = 0; // 00AA5840
    virtual void* find_movie_page_child(void* page, const char* name, bool recurse) = 0; // 00AA7E00
    // Generic GUI visibility (00AA8530) propagates to the widget activation
    // hook above. A real host must route cGuiMovie transitions through it.
    virtual void set_widget_visible(void* widget, bool visible) = 0;
    virtual void set_widget_local_z(void* widget, float local_z) = 0; // 00AA7910
    virtual MovieWidgetState& movie_widget_state(void* widget) = 0;
    virtual bool platform_flag_0d() = 0; // *(0109CF04)+0Dh
    virtual bool mission_context_present() = 0; // 00E198C4 != 0
    virtual bool movie_input_action_pressed(int action) = 0; // 004C43C0
    virtual void notify_movie_frame(int event) = 0; // 004C1E90 then 00427190(0Dh)
    virtual void release_movie_page(void* page) = 0; // 00AA31F0
    virtual void destroy_movie_callback_owner(MoviePlayer& player) = 0; // 00695870, this+8
};

MoviePlayer& construct_movie_player_004f8af0(MoviePlayer& player) noexcept;
void initialize_movie_player_004f8cf0(MoviePlayer& player,
    FrontEndScreenTable& screens, MoviePlayerHost& host);
void set_movie_completion_004f8970(MoviePlayer& player, MoviePlayerHost& host,
    MovieCompletionCallback callback);
void play_movie_004f8a20(MoviePlayer& player, MoviePlayerHost& host,
    const NativeString& filename, std::uint8_t prefer_shrink_wide, float local_z,
    std::uint8_t loop);
void stop_movie_004f8ac0(MoviePlayer& player, MoviePlayerHost& host);
bool movie_is_playing_004f89a0(MoviePlayer& player, MoviePlayerHost& host);
void update_movie_player_004f8c60(MoviePlayer& player, MoviePlayerHost& host, float delta);
void collect_movie_player_children_004f8f00(const MoviePlayer& player,
    std::vector<void*>& children);
// Native logo caller's bytes +4h/+5h=1, commit visibility, then +18h no-op.
void enter_movie_player(MoviePlayer& player, MoviePlayerHost& host);
void destroy_movie_player_004f8be0(MoviePlayer& player,
    FrontEndScreenTable& screens, MoviePlayerHost& host);

struct GameMovieCompletionState {
    bool movie_request_active{false}; // game+7184h
    bool requests_held{false}; // game+5ECh
};
struct GameMovieCompletionHost {
    virtual ~GameMovieCompletionHost() = default;
    virtual int current_game_state() = 0; // reread after movie stop
    virtual void set_movie_input_context(int context, bool enabled) = 0; // 00A933F0
    virtual void set_movie_cinematic_mode(bool hide, std::uint8_t allow_simulation,
        bool argument_3) = 0; // reconstructed 004CD0F0, host can forward
};
// 004C7ED0 body: clear game+7184, input context(10h,0), stop, clear wanted,
// clear game+5EC. No active-state write and no callback invocation.
void finish_game_movie_004c7ed0(GameMovieCompletionState& state, MoviePlayer& player,
    MoviePlayerHost& player_host, GameMovieCompletionHost& game_host);
// 004F89D0 default callback. No-op in states 1/2/4; finish elsewhere, reread
// state, and leave cinematic mode only for states 0Dh/0Fh.
void default_movie_completion_004f89d0(GameMovieCompletionState& state,
    MoviePlayer& player, MoviePlayerHost& player_host, GameMovieCompletionHost& game_host);

} // namespace bsp
