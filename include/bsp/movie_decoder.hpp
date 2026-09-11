#pragma once

#include <cstdint>
#include "bsp/native_string.hpp"

namespace bsp {

// Only the public prefix consumed by the game. Bink owns the complete handle;
// never allocate a decoder handle using sizeof(BinkHandle).
struct BinkHandle {
    std::uint32_t width;
    std::uint32_t height;
    std::uint32_t frame_count;
    std::uint32_t frame_number;
};
struct BinkRealtime {
    std::uint32_t frame_number;
    std::uint32_t frame_rate;
    std::uint32_t frame_rate_divisor;
    std::uint32_t remaining[11];
};

// Required imported entrypoints, with the shipped Win32 stdcall decorations.
// Bind these to the shipped binkw32.dll; no codec implementations are supplied.
struct BinkImports {
    BinkHandle* (__stdcall* open)(const char*, std::uint32_t); // _BinkOpen@8
    void (__stdcall* close)(BinkHandle*); // _BinkClose@4
    void (__stdcall* set_sound_track)(std::uint32_t, const std::uint32_t*); // @8
    void (__stdcall* set_volume)(BinkHandle*, std::uint32_t, std::int32_t); // @12
    std::int32_t (__stdcall* do_frame)(BinkHandle*); // _BinkDoFrame@4
    std::int32_t (__stdcall* copy_to_buffer)(BinkHandle*, void*, std::int32_t,
        std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t); // @28
    void (__stdcall* next_frame)(BinkHandle*); // _BinkNextFrame@4
    std::int32_t (__stdcall* wait)(BinkHandle*); // _BinkWait@4
    void (__stdcall* get_realtime)(BinkHandle*, BinkRealtime*, std::uint32_t); // @12
    void (__stdcall* go_to)(BinkHandle*, std::uint32_t, std::uint32_t); // _BinkGoto@12
    std::int32_t (__stdcall* pause)(BinkHandle*, std::int32_t); // _BinkPause@8
};

// Exact Win32 shared-base layout, reached at allocation+10h. Recorded vtable
// words are evidence identifiers, never callable native pointers in this build.
struct MovieDecoderState {
    std::uint32_t vtable;             // +00h
    BinkHandle* handle;                // +04h
    std::uint8_t running;              // +08h
    std::uint8_t completed;            // +09h
    std::uint8_t loop;                 // +0Ah
    std::uint8_t field_0b;             // +0Bh, zeroed, otherwise unresolved
    float volume;                     // +0Ch, original unclamped input
    float target_volume;              // +10h
    float fade_delay;                 // +14h
    float time;                       // +18h
    float frame_seconds;              // +1Ch, lazily initialized when zero
    NativeString filename;            // +20h/+24h
};
struct NativeMovieDecoder {
    std::uint32_t file_vbtable;        // +00h, 00D24DE8: shared base +10h
    std::uint32_t texture_vbtable;     // +04h, 00D24DE0: shared base +0Ch
    void* texture;                    // +08h, native reference-counted resource
    std::int32_t vtordisp;            // +0Ch
    MovieDecoderState state;          // +10h, factory returns this address
};
struct MovieSurfaceLock {
    std::int32_t pitch;
    void* pixels;
};

struct MovieDecoderHost {
    virtual ~MovieDecoderHost() = default;
    virtual const BinkImports& bink() = 0;
    virtual NativeStringStorage& movie_string_storage() = 0;
    // 00BDD600 on singleton0109CEEC; modifies a copied path. Return is ignored.
    virtual void resolve_movie_path(NativeString& path) = 0;
    virtual void movie_log(const char* format, const char* filename) = 0; // 004254B0
    // Native0109EEA4 selects the CRT float conversion path, relevant for NaN.
    virtual bool sse2_truncation_enabled() = 0;
    // singleton00F8D394 virtual+88h; arguments forwarded without substitutions.
    virtual void* create_movie_texture(std::uint32_t width, std::uint32_t height,
        int argument_3, int format, int argument_5) = 0;
    // InterlockedDecrement(resource+4), invoke virtual+0 only if result is zero.
    virtual void release_movie_resource(void* resource) = 0;
    virtual void* acquire_movie_surface(void* texture, int level, int argument_2) = 0; // +30h
    virtual void lock_movie_surface(void* surface, MovieSurfaceLock& output,
        int argument_2, int argument_3) = 0; // +2Ch, native result ignored
    virtual void finish_movie_texture(void* texture, int level) = 0; // +38h
};

// Complete 38h object path (original hidden construction flag=1), __thiscall,
// RET4/EAX=this. Arbitrary further-derived virtual-base layouts are not modeled.
NativeMovieDecoder& construct_movie_decoder_00a4c660(NativeMovieDecoder&) noexcept;
// 00A4CEE0 shared base construction, ECX=state, RET.
void construct_movie_decoder_state_00a4cee0(MovieDecoderState&) noexcept;
// 00A4C860 complete destruction chain, ECX=allocation, no operator delete.
void destroy_movie_decoder_00a4c860(NativeMovieDecoder&, MovieDecoderHost&);
// 00A4CF30 base destruction; closes and releases filename without clearing header.
void destroy_movie_decoder_state_00a4cf30(MovieDecoderState&, MovieDecoderHost&);

// Original concrete ECX=allocation+8h (shared state-8h), via thunk00A4C800; RET0Ch.
// The two trailing stack arguments are unused by this concrete file decoder.
bool open_movie_decoder_00a4ca40(NativeMovieDecoder&, MovieDecoderHost&,
    const NativeString&, bool argument_2, int argument_3);
// 00A4CFB0 base open returns false after reset/close/copy/log, RET0Ch.
bool begin_movie_decoder_open_00a4cfb0(MovieDecoderState&, MovieDecoderHost&,
    const NativeString&, bool argument_2, int argument_3);
void close_movie_decoder_00a4cd30(MovieDecoderState&, MovieDecoderHost&); // RET
BinkHandle* movie_decoder_handle_00a4cb50(const MovieDecoderState&) noexcept; // EAX
std::uint8_t movie_decoder_completed_00a4c700(const MovieDecoderState&) noexcept; // AL
void mark_movie_decoder_completed_00a4c710(MovieDecoderState&, std::uint8_t) noexcept; // RET4
void set_movie_decoder_loop_00a4c720(MovieDecoderState&, std::uint8_t) noexcept; // RET4
void pause_movie_decoder_00a4cb60(MovieDecoderState&, MovieDecoderHost&, std::uint8_t); // RET4
void set_movie_decoder_running_00a4cb90(MovieDecoderState&, MovieDecoderHost&, std::uint8_t); // RET4
void seek_movie_decoder_00a4cc70(MovieDecoderState&, MovieDecoderHost&, std::uint32_t); // RET4
void set_movie_decoder_target_volume_00a4cbc0(MovieDecoderState&, float) noexcept; // RET4
void set_movie_decoder_volume_immediate_00a4cbd0(MovieDecoderState&, MovieDecoderHost&, float); // RET4
void apply_movie_decoder_volume_00a4c980(MovieDecoderState&, MovieDecoderHost&, float); // RET4
float movie_decoder_volume_00a4cc00(const MovieDecoderState&) noexcept; // ST0, RET
void ensure_movie_decoder_frame_seconds_00a4cc10(MovieDecoderState&, MovieDecoderHost&); // RET
// virtual+34h releases old texture then creates(width,height,1,16h,1) if open.
void* prepare_movie_decoder_frame_00a4d140(NativeMovieDecoder&, MovieDecoderHost&);
// virtual+0: DoFrame, optional texture copy, NextFrame iff current != last.
void decode_movie_decoder_frame_00a4d1b0(NativeMovieDecoder&, MovieDecoderHost&);
// virtual+2Ch, RET/AL. Returns false for no handle or completed last frame.
bool update_movie_decoder_00a4cd70(NativeMovieDecoder&, MovieDecoderHost&);

} // namespace bsp
