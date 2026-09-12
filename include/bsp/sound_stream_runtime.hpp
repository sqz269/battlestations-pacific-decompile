#pragma once

#include "bsp/sound_system_owner.hpp"
#include <cstdint>

namespace bsp {

// Exact library operations used by the actual54h D5B360 stream. Identically
// named existing FmodConfigurationLibrary methods can fulfill this interface.
// Native-byte output methods must preserve the actual C++ export behavior.
// The installed C FMOD_BOOL wrappers can overwrite outputs after native errors;
// their normalization is not interchangeable with these byte-pointer calls.
class SoundStreamFmodHost {
public:
    virtual ~SoundStreamFmodHost() = default;
    virtual FmodResult create_stream(void*, const char*, std::uint32_t, void*, void**) = 0;
    virtual FmodResult system_play_sound(void*, std::int32_t, void*, bool, void**) = 0;
    virtual FmodResult sound_get_length(void*, std::uint32_t*, std::uint32_t) = 0;
    virtual FmodResult sound_get_open_state(void*, std::int32_t* state,
        std::uint32_t* percent, std::uint8_t* starving) = 0;
    virtual FmodResult sound_get_format(void*, std::int32_t* type,
        std::int32_t* format, std::int32_t* channels, std::int32_t* bits) = 0;
    virtual FmodResult channel_set_speaker_levels(void*, std::int32_t speaker,
        const float* levels, std::int32_t count) = 0;
    virtual FmodResult channel_set_priority(void*, std::int32_t) = 0;
    virtual FmodResult channel_set_position(void*, std::uint32_t, std::uint32_t) = 0;
    virtual FmodResult channel_get_position(void*, std::uint32_t*, std::uint32_t) = 0;
    virtual FmodResult channel_set_paused(void*, std::uint8_t) = 0;
    virtual FmodResult stream_channel_is_playing(void*, std::uint8_t& inout) = 0;
    virtual FmodResult channel_stop(void*) = 0;
    virtual FmodResult release_sound(void*) = 0;
    virtual void memory_get_stats(std::int32_t*, std::int32_t*) = 0;
};

struct SoundStreamRuntimeContext {
    SoundSystemOwner* volatile& current_owner_00f8bbd8;
    SoundStreamFmodHost& fmod;
    NativeStringStorage& strings;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile double& fade_rate_00ce3dc8;
    const char* null_filename_00f8bbee;
    const char* null_integer_format_01090ab4;
};

// Body-read A87B60: ECX manager, EAX +170, RET (7 bytes). Ghidra currently
// lacks this function start; D5B44C+8 points here. Typed projection, not ABI.
std::uint32_t sound_stream_speaker_layout_00a87b60(const SoundSystemOwner&) noexcept;
// ECX actual54h receiver. Gain takes float/RET4; row takes index,float/RET8.
// Both copy raw float bits. Row indexing is unchecked native pointer arithmetic.
void set_sound_stream_gain_00a864f0(void*, float) noexcept;
void set_sound_stream_row_gain_00a86670(void*, std::int32_t index, float) noexcept;
// Full native arithmetic/ABI: ECX actual12h {target,current,rate}, stack dt,
// AL changed, RET4. EDX is ignored on entry; the extra argument binds fastcall.
std::uint8_t __fastcall advance_sound_stream_row_00a85b50(void*, void*, float);

// ECX actual54h receiver; stack float/RET4 for levels/update, native name/RET4
// for start; no arguments/RET for query/stop/ready. These callable C++ entries
// add explicit services and preserve native stores, calls and ignored results.
void update_sound_stream_levels_00a86150(void*, float, SoundStreamRuntimeContext&);
std::int32_t query_sound_stream_open_state_00a86b40(void*, SoundStreamRuntimeContext&);
void stop_sound_stream_00a86bf0(void*, SoundStreamRuntimeContext&);
void ready_sound_stream_00a86de0(void*, SoundStreamRuntimeContext&);
void start_sound_stream_00a867b0(void*, const NativeString&, SoundStreamRuntimeContext&);
void update_sound_stream_00a874d0(void*, float, SoundStreamRuntimeContext&);

// Full two numeric StringBuilder bodies. ECX actual18h builder, stack DWORD,
// EAX builder, RET4. Both use the current builder+8 integer format (default %d)
// and the same DWORD bits, despite one caller passing an unsigned position.
// Valid native formatting must fit the original64-byte temporary buffer.
void* append_native_log_int_00bd1bb0(void*, std::int32_t, NativeStringStorage&,
    const char* null_format_01090ab4);
void* append_native_log_uint_00bd1ca0(void*, std::uint32_t, NativeStringStorage&,
    const char* null_format_01090ab4);

} // namespace bsp
