#include "bsp/startup_frontend.hpp"

namespace bsp {

void StartupLogoMovieHost::install_movie_completion_callback()
{
    set_movie_completion_004f8970(player_, host_, completion_);
}

void StartupLogoMovieHost::movie_play(const char* name, int prefer_shrink_wide,
    float local_z, int loop)
{
    // LogoSequenceState projects each native 8-byte string as a C string.
    // Restore a temporary owning header at this interface boundary; this
    // conversion is host scaffolding, not an extra native startup allocation.
    NativeStringStorage& storage = host_.movie_string_storage();
    NativeString filename;
    struct ReleaseString {
        NativeString& value;
        NativeStringStorage& storage;
        ~ReleaseString() { value.release_to(storage); }
    } release{filename, storage};
    filename.assign_0041e870(storage, name);
    play_movie_004f8a20(player_, host_, filename,
        static_cast<std::uint8_t>(prefer_shrink_wide), local_z,
        static_cast<std::uint8_t>(loop));
}

void StartupLogoMovieHost::movie_screen_enter()
{
    enter_movie_player(player_, host_);
}

void TitleProfileResetHost::reset_player_profile()
{
    reset_profile_007fdb20(profile_, settings_, host_);
}

} // namespace bsp
