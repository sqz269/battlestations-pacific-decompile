#pragma once

#include "bsp/movie_player.hpp"
#include "bsp/profile_reset.hpp"
#include "bsp/title_init.hpp"

namespace bsp {

// Host composition, not a recovered class or native object layout. These
// adapters bind recovered startup calls while keeping unrecovered services
// abstract. Evidence: docs/STARTUP_FRONTEND_INTEGRATION.md.
class StartupLogoMovieHost : public LogoSequenceHost {
public:
    StartupLogoMovieHost(MoviePlayer& player, MoviePlayerHost& host,
        MovieCompletionCallback completion) noexcept
        : player_(player), host_(host), completion_(completion) {}

    void install_movie_completion_callback() final;
    void movie_play(const char* name, int prefer_shrink_wide,
        float local_z, int loop) final;
    void movie_screen_enter() final;

private:
    MoviePlayer& player_;
    MoviePlayerHost& host_;
    MovieCompletionCallback completion_;
};

class TitleProfileResetHost : public TitleInitHost {
public:
    TitleProfileResetHost(ProfileResetState& profile, GameSettingsBlock& settings,
        ProfileResetHost& host) noexcept
        : profile_(profile), settings_(settings), host_(host) {}

    void reset_player_profile() final;

private:
    ProfileResetState& profile_;
    GameSettingsBlock& settings_;
    ProfileResetHost& host_;
};

} // namespace bsp
