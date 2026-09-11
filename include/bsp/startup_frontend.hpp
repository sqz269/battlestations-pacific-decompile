#pragma once

#include "bsp/movie_player.hpp"
#include "bsp/movie_decoder.hpp"
#include "bsp/frontend_prompt_layout.hpp"
#include "bsp/profile_archive.hpp"
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

// These additional composition adapters connect the recovered backend bodies.
// They require real object ownership and the remaining GUI/archive services;
// docs/STARTUP_BACKEND_INTEGRATION.md records the limits.
class DecodedMoviePlayerHost : public MoviePlayerHost {
public:
    explicit DecodedMoviePlayerHost(MovieDecoderHost& decoder_host) noexcept
        : decoder_host_(decoder_host) {}
    NativeStringStorage& movie_string_storage() final;
    bool decoder_open(void*, const NativeString&, bool, int) final;
    void decoder_set_volume_immediate(void*, float) final;
    void decoder_set_loop(void*, std::uint8_t) final;
    void decoder_prepare_frame(void*) final;
    void decoder_set_running(void*, bool) final;
    void decoder_close(void*) final;
    bool decoder_completed(void*) final;
    void decoder_mark_completed(void*, bool) final;
    void decoder_update(void*) final;
    bool decoder_has_open_handle(void*) final;
private:
    MovieDecoderHost& decoder_host_;
};

class ProfileArchiveIoHost : public ProfileIoHost {
public:
    ProfileArchiveIoHost(ProfileArchiveReader*& reader_slot,
        ProfileArchiveHost& archive_host) noexcept
        : reader_slot_(reader_slot), archive_host_(archive_host) {}
    void deserialize_profile_007fdf00(ProfileResetState& profile) final;
private:
    ProfileArchiveReader*& reader_slot_;
    ProfileArchiveHost& archive_host_;
};

class LaidOutPromptHost : public FrontEndPromptHost {
public:
    LaidOutPromptHost(FrontEndPromptScreen& screen, FrontEndPromptWidgets& widgets,
        FrontEndPromptLayoutHost& layout_host, FrontEndPromptEnterHost& enter_host,
        FrontEndPromptButtonHost& button_host, std::vector<NativeString>& scratch) noexcept
        : screen_(screen), widgets_(widgets), layout_host_(layout_host),
          enter_host_(enter_host), button_host_(button_host), scratch_(scratch) {}
    void enter_screen() final;
    void prepare_button(PromptWidget button) final;
    void layout_buttons_00530a60() final;
private:
    FrontEndPromptScreen& screen_;
    FrontEndPromptWidgets& widgets_;
    FrontEndPromptLayoutHost& layout_host_;
    FrontEndPromptEnterHost& enter_host_;
    FrontEndPromptButtonHost& button_host_;
    std::vector<NativeString>& scratch_;
};

} // namespace bsp
