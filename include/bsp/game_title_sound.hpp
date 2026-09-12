#pragma once
#include "bsp/native_string.hpp"
#include <memory>

namespace bsp {
struct MoviePlayer;
struct MoviePlayerHost;

// 004F8BA0: ECX actual34h MoviePlayer, stack borrowed8h name, AL, RET4.
// Captures selected widget+20 once, compares requested name against its native
// filename first, then calls the existing AAC8E0 playback predicate. GUI state
// uses the existing MovieWidgetState projection, not a new native GUI layout.
bool movie_named_clip_is_playing_004f8ba0(const MoviePlayer&, const NativeString&,
    MoviePlayerHost&);
// 00584A30: no native arguments, AL, RET. Builds/releases the actual8h intro
// name and loads the current E18D48 publication after that allocation/copy.
bool intro_movie_is_active_00584a30(MoviePlayer* volatile& current_player,
    NativeStringStorage&, MoviePlayerHost&);
// A85C20: ECX actual54h stream, RET. If unsigned state20 <=2 and fade-out byteA
// is clear, set fade-in byteB and positive-zero fadeC. Does not start FMOD.
void request_sound_stream_fade_in_00a85c20(void* actual_stream) noexcept;
}

namespace bsp::game {
class GameSoundRuntime;
struct GameSoundDialogRuntimeGlobals;

// References to the CURRENT manager's canonical fields; no copied path or
// stream owner. A raw binding uses E198AC+40/+50. A projection binding must
// return its sole actual8h path and owning stream slot. Each call reloads the
// manager publication, including calls after allocation/string callbacks.
struct GameTitleSoundMenuView {
    const NativeString& title_path_40;
    void*& title_stream_50;
};
struct GameTitleSoundMenuHost {
    virtual ~GameTitleSoundMenuHost() = default;
    virtual GameTitleSoundMenuView current_menu_00e198ac() noexcept = 0;
};
struct GameTitleSoundApplication {
    GameTitleSoundMenuHost& menus;
    MoviePlayer* volatile& movie_player_00e18d48;
    MoviePlayerHost& movie_widgets;
    // E19504 is a live NUL-terminated C-string buffer, NOT an8h header.
    // The existing alleged producer xrefs are reads/fallbacks; no selection
    // policy or immutable empty-string default is invented here.
    const char* selected_track_00e19504;
    const volatile float& music_volume_00f889a8;
};

// Concrete application composition; not an original native object or vtable.
// Borrow the core and the SAME constant/string/VFS services used by its dialog
// facade. The current menu owns the stream's initial reference. This helper
// owns neither that slot nor the movie state; release menu stream references
// before core shutdown and keep all borrowed services alive through release.
class GameTitleSound final {
public:
    GameTitleSound(GameSoundRuntime&, const GameSoundDialogRuntimeGlobals&,
        GameTitleSoundApplication);
    ~GameTitleSound();
    GameTitleSound(const GameTitleSound&) = delete;
    GameTitleSound& operator=(const GameTitleSound&) = delete;

    // Complete native005884A0 order in the existing throwing allocator/string
    // domain: no stack arguments, ECX ignored, RET. Replaces the older semantic
    // TitleMusicHost wrapper so all temporary ownership and manager reloads
    // survive. A failed open leaves the published owning reference intact.
    void start_005884a0();
    // Application routes to existing recovered bodies; no new native claims.
    void update_stream(void* actual_stream, float dt);
    void stop_stream(void* actual_stream);
    void release_stream_reference(void* actual_stream);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
