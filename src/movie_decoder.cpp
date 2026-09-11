#include "bsp/movie_decoder.hpp"
#include "bsp/unit_motion.hpp"

#include <cstddef>
#include <cmath>
#include <limits>
#include <new>

namespace bsp {
static_assert(sizeof(MovieDecoderState) == 0x28);
static_assert(offsetof(MovieDecoderState, handle) == 4);
static_assert(offsetof(MovieDecoderState, running) == 8);
static_assert(offsetof(MovieDecoderState, completed) == 9);
static_assert(offsetof(MovieDecoderState, loop) == 10);
static_assert(offsetof(MovieDecoderState, volume) == 0x0c);
static_assert(offsetof(MovieDecoderState, target_volume) == 0x10);
static_assert(offsetof(MovieDecoderState, fade_delay) == 0x14);
static_assert(offsetof(MovieDecoderState, time) == 0x18);
static_assert(offsetof(MovieDecoderState, frame_seconds) == 0x1c);
static_assert(offsetof(MovieDecoderState, filename) == 0x20);
static_assert(sizeof(NativeMovieDecoder) == 0x38);
static_assert(offsetof(NativeMovieDecoder, texture) == 8);
static_assert(offsetof(NativeMovieDecoder, vtordisp) == 0x0c);
static_assert(offsetof(NativeMovieDecoder, state) == 0x10);
static_assert(sizeof(MovieSurfaceLock) == 8);
static_assert(sizeof(BinkRealtime) == 0x38);

namespace {
const char* filename_text(const MovieDecoderState& state) noexcept {
    return state.filename.data() ? state.filename.data() : "";
}
}

void construct_movie_decoder_state_00a4cee0(MovieDecoderState& state) noexcept {
    state.vtable = 0x00d24e78;
    state.handle = nullptr;
    state.running = 0;
    state.completed = 0;
    state.loop = 0;
    state.field_0b = 0;
    state.volume = 1.0f;
    state.target_volume = 0.0f;
    state.fade_delay = 0.0f;
    state.time = 0.0f;
    state.frame_seconds = 0.0f;
    ::new (&state.filename) NativeString{};
}

NativeMovieDecoder& construct_movie_decoder_00a4c660(NativeMovieDecoder& decoder) noexcept {
    decoder.file_vbtable = 0x00d24de8;
    decoder.texture_vbtable = 0x00d24de0;
    construct_movie_decoder_state_00a4cee0(decoder.state);
    // 00A4C8B0(0), followed by 00A4D040(0), then the most-derived table.
    decoder.state.vtable = 0x00d24df0;
    decoder.vtordisp = 8;
    decoder.state.vtable = 0x00d24ed8;
    decoder.vtordisp = 0;
    decoder.texture = nullptr;
    decoder.state.vtable = 0x00d24d98;
    decoder.vtordisp = 0;
    return decoder;
}

void close_movie_decoder_00a4cd30(MovieDecoderState& state, MovieDecoderHost& host) {
    host.movie_log("Closing movie %s", filename_text(state));
    state.completed = 1;
    if (state.handle) {
        host.bink().close(state.handle);
        state.handle = nullptr;
    }
}

void destroy_movie_decoder_state_00a4cf30(MovieDecoderState& state, MovieDecoderHost& host) {
    state.vtable = 0x00d24e78;
    close_movie_decoder_00a4cd30(state, host);
    destroy_native_string_header_0041dd20(&state.filename, host.movie_string_storage());
}

void destroy_movie_decoder_00a4c860(NativeMovieDecoder& decoder, MovieDecoderHost& host) {
    // 00A4C770 -> 00A4D0F0 -> 00A4C920, then 00A4CF30.
    decoder.state.vtable = 0x00d24d98;
    decoder.vtordisp = 0;
    decoder.state.vtable = 0x00d24ed8;
    if (decoder.texture) {
        host.release_movie_resource(decoder.texture);
        decoder.texture = nullptr;
    }
    decoder.state.vtable = 0x00d24df0;
    decoder.vtordisp = 8;
    destroy_movie_decoder_state_00a4cf30(decoder.state, host);
}

bool begin_movie_decoder_open_00a4cfb0(MovieDecoderState& state, MovieDecoderHost& host,
    const NativeString& filename, bool, int) {
    state.time = 0.0f;
    state.completed = 0;
    if (state.handle) close_movie_decoder_00a4cd30(state, host);
    state.filename.copy_from_00be0a30_fragment(host.movie_string_storage(), filename);
    host.movie_log("Opening movie %s", filename_text(state));
    return false;
}

bool open_movie_decoder_00a4ca40(NativeMovieDecoder& decoder, MovieDecoderHost& host,
    const NativeString& filename, bool, int) {
    begin_movie_decoder_open_00a4cfb0(decoder.state, host, filename, true, 0);
    NativeString path;
    auto& storage = host.movie_string_storage();
    path.copy_from_00be0a30_fragment(storage, filename);
    try {
        host.resolve_movie_path(path);
        const std::uint32_t track = 0;
        host.bink().set_sound_track(1, &track);
        decoder.state.handle = host.bink().open(path.data() ? path.data() : "", 0x4000);
        const bool opened = decoder.state.handle != nullptr;
        path.release_to(storage);
        return opened;
    } catch (...) {
        path.release_to(storage);
        throw;
    }
}

BinkHandle* movie_decoder_handle_00a4cb50(const MovieDecoderState& state) noexcept { return state.handle; }
std::uint8_t movie_decoder_completed_00a4c700(const MovieDecoderState& state) noexcept { return state.completed; }
void mark_movie_decoder_completed_00a4c710(MovieDecoderState& state, std::uint8_t value) noexcept { state.completed = value; }
void set_movie_decoder_loop_00a4c720(MovieDecoderState& state, std::uint8_t value) noexcept { state.loop = value; }
void set_movie_decoder_target_volume_00a4cbc0(MovieDecoderState& state, float value) noexcept { state.target_volume = value; }
float movie_decoder_volume_00a4cc00(const MovieDecoderState& state) noexcept { return state.volume; }

void pause_movie_decoder_00a4cb60(MovieDecoderState& state, MovieDecoderHost& host, std::uint8_t paused) {
    if (state.handle) {
        if (paused == 0) state.fade_delay = 0.5f;
        host.bink().pause(state.handle, paused);
    }
}

void ensure_movie_decoder_frame_seconds_00a4cc10(MovieDecoderState& state, MovieDecoderHost& host) {
    if (state.frame_seconds == 0.0f) {
        BinkRealtime realtime;
        host.bink().get_realtime(state.handle, &realtime, 1);
        // Native FILD unsigned fixup and x87 reciprocal/multiply, one final float store.
        state.frame_seconds = static_cast<float>(static_cast<double>(realtime.frame_rate_divisor)
            * (1.0 / static_cast<double>(realtime.frame_rate)));
    }
}

void seek_movie_decoder_00a4cc70(MovieDecoderState& state, MovieDecoderHost& host, std::uint32_t frame) {
    state.completed = 0;
    if (state.handle) {
        host.bink().go_to(state.handle, frame, 0);
        ensure_movie_decoder_frame_seconds_00a4cc10(state, host);
        state.time = static_cast<float>(static_cast<double>(frame) * state.frame_seconds);
    }
}

void set_movie_decoder_running_00a4cb90(MovieDecoderState& state, MovieDecoderHost& host, std::uint8_t running) {
    if (running != 0 && state.running == 0) {
        seek_movie_decoder_00a4cc70(state, host, 0);
        state.time = 0.0f;
    }
    state.running = running;
}

void apply_movie_decoder_volume_00a4c980(MovieDecoderState& state, MovieDecoderHost& host, float value) {
    float clamped = value;
    if (0.0f > value) clamped = 0.0f;
    else if (value > 1.0f) clamped = 1.0f;
    // Two original doubles: 00D24E48 is the exact double of float sqrt(0.5),
    // followed by 32768. Original field stores the unclamped input after Bink.
    const double scaled = static_cast<double>(clamped) * 0.707106769084930419921875 * 32768.0;
    const std::int32_t volume = std::isnan(scaled)
        ? (host.sse2_truncation_enabled() ? (std::numeric_limits<std::int32_t>::min)() : 0)
        : static_cast<std::int32_t>(scaled);
    host.bink().set_volume(state.handle, 0, volume);
    state.volume = value;
}

void set_movie_decoder_volume_immediate_00a4cbd0(MovieDecoderState& state, MovieDecoderHost& host, float value) {
    state.volume = value;
    state.target_volume = value;
    apply_movie_decoder_volume_00a4c980(state, host, value);
}

void* prepare_movie_decoder_frame_00a4d140(NativeMovieDecoder& decoder, MovieDecoderHost& host) {
    if (decoder.texture) {
        host.release_movie_resource(decoder.texture);
        decoder.texture = nullptr;
    }
    if (decoder.state.handle) {
        const auto width = decoder.state.handle->width;
        const auto height = decoder.state.handle->height;
        decoder.texture = host.create_movie_texture(width, height, 1, 0x16, 1);
        return decoder.texture;
    }
    return nullptr;
}

void decode_movie_decoder_frame_00a4d1b0(NativeMovieDecoder& decoder, MovieDecoderHost& host) {
    host.bink().do_frame(decoder.state.handle);
    if (decoder.texture) {
        MovieSurfaceLock lock{};
        void* surface = host.acquire_movie_surface(decoder.texture, 0, 0);
        host.lock_movie_surface(surface, lock, 0, 0);
        host.bink().copy_to_buffer(decoder.state.handle, lock.pixels, lock.pitch,
            decoder.state.handle->height, 0, 0, 3);
        host.finish_movie_texture(decoder.texture, 0);
        host.release_movie_resource(surface);
    }
    if (decoder.state.handle->frame_number != decoder.state.handle->frame_count)
        host.bink().next_frame(decoder.state.handle);
}

bool update_movie_decoder_00a4cd70(NativeMovieDecoder& decoder, MovieDecoderHost& host) {
    auto& state = decoder.state;
    if (!state.handle) return false;
    ensure_movie_decoder_frame_seconds_00a4cc10(state, host);
    if (host.bink().wait(state.handle) == 0 && state.running != 0) {
        BinkRealtime realtime;
        host.bink().get_realtime(state.handle, &realtime, 1);
        const auto frame = realtime.frame_number == 0xffffffffu ? 0u : realtime.frame_number;
        const float now = static_cast<float>(static_cast<double>(frame) * state.frame_seconds);
        const float delta = now - state.time;
        state.time = now;
        if (frame % 30 == 0) host.movie_log("Playing movie %s", filename_text(state));
        if (state.volume != state.target_volume) {
            // Native COMISS/JBE also takes the step arm for an unordered delay.
            if (state.fade_delay > 0.0f) state.fade_delay -= delta;
            else {
                state.fade_delay = 0.0f;
                state.volume = unit_step_towards_0042ac60(state.volume, state.target_volume, delta + delta);
            }
            apply_movie_decoder_volume_00a4c980(state, host, state.volume);
        }
        if (state.handle->frame_number == state.handle->frame_count) {
            if (state.loop == 0) {
                state.completed = 1;
                return false;
            }
            host.bink().go_to(state.handle, 0, 0);
        }
        decode_movie_decoder_frame_00a4d1b0(decoder, host);
    }
    return true;
}
} // namespace bsp
