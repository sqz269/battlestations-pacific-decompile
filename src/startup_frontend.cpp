#include "bsp/startup_frontend.hpp"

#include <cstddef>

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

namespace {
MovieDecoderState& decoder_state(void* pointer)
{
    // GUI movie.decoder is the factory's shared-base pointer at allocation+10h.
    return *static_cast<MovieDecoderState*>(pointer);
}
NativeMovieDecoder& complete_decoder(void* pointer)
{
    return *reinterpret_cast<NativeMovieDecoder*>(static_cast<std::byte*>(pointer)
        - offsetof(NativeMovieDecoder, state));
}
}

NativeStringStorage& DecodedMoviePlayerHost::movie_string_storage()
{
    return decoder_host_.movie_string_storage();
}
bool DecodedMoviePlayerHost::decoder_open(void* pointer, const NativeString& name,
    bool argument_2, int argument_3)
{
    return open_movie_decoder_00a4ca40(complete_decoder(pointer), decoder_host_,
        name, argument_2, argument_3);
}
void DecodedMoviePlayerHost::decoder_set_volume_immediate(void* pointer, float value)
{
    set_movie_decoder_volume_immediate_00a4cbd0(decoder_state(pointer), decoder_host_, value);
}
void DecodedMoviePlayerHost::decoder_set_loop(void* pointer, std::uint8_t value)
{
    set_movie_decoder_loop_00a4c720(decoder_state(pointer), value);
}
void DecodedMoviePlayerHost::decoder_prepare_frame(void* pointer)
{
    prepare_movie_decoder_frame_00a4d140(complete_decoder(pointer), decoder_host_);
}
void DecodedMoviePlayerHost::decoder_set_running(void* pointer, bool value)
{
    set_movie_decoder_running_00a4cb90(decoder_state(pointer), decoder_host_, value);
}
void DecodedMoviePlayerHost::decoder_close(void* pointer)
{
    close_movie_decoder_00a4cd30(decoder_state(pointer), decoder_host_);
}
bool DecodedMoviePlayerHost::decoder_completed(void* pointer)
{
    return movie_decoder_completed_00a4c700(decoder_state(pointer)) != 0;
}
void DecodedMoviePlayerHost::decoder_mark_completed(void* pointer, bool value)
{
    mark_movie_decoder_completed_00a4c710(decoder_state(pointer), value);
}
void DecodedMoviePlayerHost::decoder_update(void* pointer)
{
    update_movie_decoder_00a4cd70(complete_decoder(pointer), decoder_host_);
}
bool DecodedMoviePlayerHost::decoder_has_open_handle(void* pointer)
{
    return movie_decoder_handle_00a4cb50(decoder_state(pointer)) != nullptr;
}

void ProfileArchiveIoHost::deserialize_profile_007fdf00(ProfileResetState& profile)
{
    read_profile_archive_007fdf00(profile, *reader_slot_, archive_host_);
}

void LaidOutPromptHost::enter_screen()
{
    enter_prompt_screen_00531380(screen_, widgets_, enter_host_);
}
void LaidOutPromptHost::prepare_button(PromptWidget button)
{
    prepare_prompt_button_00532110(scratch_, button, button_host_);
}
void LaidOutPromptHost::layout_buttons_00530a60()
{
    layout_prompt_buttons_00530a60(screen_, layout_host_);
}

} // namespace bsp
