#include "bsp/sound_stream_runtime.hpp"
#include "bsp/native_vfs_open_logging.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <array>
#include <cstdio>
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Sound stream runtime requires MSVC Win32 x87 support.
#endif

namespace bsp {
// Independent reconstruction in sound_dialog_logical.hpp/cpp. This declaration
// allows the two source packets to build independently; final callers link the
// real implementation, not an emulated lookup or a success stub.
std::int32_t find_sound_dialog_record_00a865f0(void*, const void*) noexcept;

namespace {
template<class T> T read(const void* object, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const unsigned char*>(object) + offset, sizeof value); return value;
}
template<class T> void write(void* object, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(object) + offset, &value, sizeof value);
}
void* at(void* object, std::size_t offset) noexcept { return static_cast<unsigned char*>(object) + offset; }
template<class T> T* field(void* object, std::size_t offset) noexcept { return static_cast<T*>(at(object, offset)); }
SoundSystemOwner& current(SoundStreamRuntimeContext& context) {
    auto* owner = context.current_owner_00f8bbd8;
    if (!owner) throw std::logic_error("Sound stream operation requires the current sound manager");
    return *owner;
}
void memory_error(FmodResult result, SoundStreamRuntimeContext& context) {
    if (result == FmodResult::err_memory) {
        std::int32_t current_bytes, maximum_bytes; // A7A460 supplies unwritten, discarded outputs.
        context.fmod.memory_get_stats(&current_bytes, &maximum_bytes);
    }
}
struct Text {
    NativeString value;
    NativeStringStorage& strings;
    ~Text() { value.release_to(strings); }
};
struct Builder {
    alignas(4) std::byte bytes[0x18];
    SoundStreamRuntimeContext& context;
    explicit Builder(SoundStreamRuntimeContext& input) : context(input) {
        construct_native_log_builder_00426500(bytes, context.strings);
    }
    ~Builder() { destroy_native_log_builder_00425f80(bytes, context.strings); }
    Builder& text(const char* value) { append_native_log_cstring_00bd1a60(bytes, value, context.strings); return *this; }
    Builder& name(const void* value) { append_native_log_header_00bd1a20(bytes, value, context.strings); return *this; }
    Builder& integer(std::int32_t value) {
        append_native_log_int_00bd1bb0(bytes, value, context.strings, context.null_integer_format_01090ab4); return *this;
    }
    Builder& position(std::uint32_t value) {
        append_native_log_uint_00bd1ca0(bytes, value, context.strings, context.null_integer_format_01090ab4); return *this;
    }
};
void* append_integer(void* builder, std::uint32_t bits, NativeStringStorage& strings, const char* fallback) {
    const auto* format = read<const char*>(builder, 0xc);
    if (!format) format = fallback;
    std::int32_t argument; std::memcpy(&argument, &bits, sizeof argument);
    char buffer[64];
    const int formatted_length = std::snprintf(buffer, sizeof buffer, format, argument);
    if (formatted_length < 0 || formatted_length >= static_cast<int>(sizeof buffer))
        throw std::length_error("Native integer builder format exceeds its 64-byte temporary domain");
    const auto length = static_cast<std::uint32_t>(std::strlen(buffer));
    Text temporary{{}, strings};
    temporary.value.resize_0041dd40(strings, static_cast<std::uint32_t>(length), true);
    auto* const captured_data = temporary.value.data();
    const auto captured_length = temporary.value.length();
    if (captured_data) std::memcpy(captured_data, buffer, captured_length + 1u);
    if (captured_length) {
        const auto previous_length = read<std::uint32_t>(builder, 0);
        resize_native_string_header_0041dd40(builder, strings, previous_length + captured_length, true);
        std::memcpy(read<char*>(builder, 4) + previous_length, captured_data, captured_length);
    }
    return builder;
}

// A86150 has one FSTP32 after the manager product, then five factors with
// one FSTP32 before the speaker arrays. SSE float expressions would introduce
// extra rounding. Each entry below returns an already-rounded float in ST0.
__declspec(naked) float __fastcall rounded_product(const float*, const float*) {
    __asm {
        sub esp, 4
        fld dword ptr [ecx]
        fmul dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        add esp, 4
        ret
    }
}
__declspec(naked) float __fastcall rounded_mix(const float*) {
    __asm {
        sub esp, 4
        fld dword ptr [ecx]
        fmul dword ptr [ecx + 4]
        fmul dword ptr [ecx + 8]
        fmul dword ptr [ecx + 12]
        fmul dword ptr [ecx + 16]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        add esp, 4
        ret
    }
}
__declspec(naked) float __fastcall rounded_load(const float*) {
    __asm {
        sub esp, 4
        fld dword ptr [ecx]
        fstp dword ptr [esp]
        fld dword ptr [esp]
        add esp, 4
        ret
    }
}
// A87508..A875A1, including the incoming dt word reused by isPlaying later.
// ECX stream, EDX dt word, stack one pointer/rate pointer. No native ABI claim
// for this extracted internal helper. SSE ordered equality and x87 spill points
// follow the listing; the embedded minimum is existing00415510's body behavior.
__declspec(naked) void __fastcall advance_fade(void*, float*,
    const volatile std::uint32_t*, const volatile double*) {
    __asm {
        sub esp, 8
        cmp byte ptr [ecx + 0xa], 0
        je fade_in
        movss xmm1, dword ptr [ecx + 0xc]
        xorps xmm0, xmm0
        ucomiss xmm1, xmm0
        lahf
        test ah, 0x44
        movss dword ptr [esp], xmm1
        jp subtract_fade
        mov byte ptr [ecx + 0xa], 0
        mov byte ptr [ecx + 9], 1
        jmp fade_done
    subtract_fade:
        fld dword ptr [esp]
        fsub dword ptr [edx]
        fstp dword ptr [edx]
        fldz
        fld dword ptr [edx]
        fcomip st(0), st(1)
        fstp st(0)
        jbe store_fade_out
        movss xmm0, dword ptr [edx]
    store_fade_out:
        movss dword ptr [ecx + 0xc], xmm0
        jmp fade_done
    fade_in:
        cmp byte ptr [ecx + 0xb], 0
        je fade_done
        movss xmm0, dword ptr [ecx + 0xc]
        mov eax, dword ptr [esp + 12]
        movss xmm1, dword ptr [eax]
        ucomiss xmm0, xmm1
        lahf
        test ah, 0x44
        movss dword ptr [esp], xmm0
        jp add_fade
        mov byte ptr [ecx + 0xb], 0
        jmp fade_done
    add_fade:
        fld dword ptr [edx]
        mov eax, dword ptr [esp + 16]
        fmul qword ptr [eax]
        movss dword ptr [esp + 4], xmm1
        fadd dword ptr [esp]
        fstp dword ptr [edx]
        fld dword ptr [edx]
        fstp dword ptr [esp]
        fld dword ptr [esp + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp]
        fld dword ptr [esp + 4]
        fcomip st(0), st(1)
        fstp st(0)
        jbe choose_one
        fld dword ptr [esp]
        jmp store_fade_in
    choose_one:
        fld dword ptr [esp + 4]
    store_fade_in:
        fstp dword ptr [ecx + 0xc]
    fade_done:
        add esp, 8
        ret 8
    }
}
} // namespace

std::uint32_t sound_stream_speaker_layout_00a87b60(const SoundSystemOwner& owner) noexcept {
    return static_cast<std::uint32_t>(owner.system.speaker_layout);
}
void set_sound_stream_gain_00a864f0(void* stream, float gain) noexcept { write(stream, 0x14, gain); }
void set_sound_stream_row_gain_00a86670(void* stream, std::int32_t index, float gain) noexcept {
    auto* row = static_cast<unsigned char*>(read<void*>(stream, 0x48)) + static_cast<std::uint32_t>(index) * 12u;
    write(row, 0, gain); write(row, 4, gain);
}
__declspec(naked) std::uint8_t __fastcall advance_sound_stream_row_00a85b50(void*, void*, float) {
    __asm {
        sub esp, 8
        movss xmm1, dword ptr [ecx + 8]
        xorps xmm0, xmm0
        ucomiss xmm1, xmm0
        lahf
        test ah, 0x44
        movss dword ptr [esp], xmm1
        jnp unchanged
        fld dword ptr [esp]
        mov dl, 1
        fmul dword ptr [esp + 12]
        fstp dword ptr [esp]
        fld dword ptr [ecx]
        fstp dword ptr [esp + 12]
        fld dword ptr [ecx + 4]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fld dword ptr [esp + 12]
        fcomi st(0), st(1)
        jbe falling
        fld dword ptr [esp]
        faddp st(2), st(0)
        fxch st(1)
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fst dword ptr [ecx + 4]
    compare_target:
        fcomip st(0), st(1)
        fstp st(0)
        jc changed
        movss xmm1, dword ptr [esp + 12]
        movss dword ptr [ecx + 4], xmm1
        movss dword ptr [ecx + 8], xmm0
    changed:
        mov al, dl
        add esp, 8
        ret 4
    unchanged:
        xor dl, dl
        mov al, dl
        add esp, 8
        ret 4
    falling:
        fxch st(1)
        fcomi st(0), st(1)
        jbe equal_or_unordered
        fsub dword ptr [esp]
        fstp dword ptr [esp + 4]
        fld dword ptr [esp + 4]
        fst dword ptr [ecx + 4]
        fxch st(1)
        jmp compare_target
    equal_or_unordered:
        fstp st(1)
        movss dword ptr [ecx + 8], xmm0
        fstp st(0)
        xor al, al
        add esp, 8
        ret 4
    }
}
void* append_native_log_int_00bd1bb0(void* builder, std::int32_t value,
    NativeStringStorage& strings, const char* fallback) {
    return append_integer(builder, static_cast<std::uint32_t>(value), strings, fallback);
}
void* append_native_log_uint_00bd1ca0(void* builder, std::uint32_t value,
    NativeStringStorage& strings, const char* fallback) {
    return append_integer(builder, value, strings, fallback);
}

void update_sound_stream_levels_00a86150(void* stream, float dt, SoundStreamRuntimeContext& context) {
    auto* owner = &current(context);
    const float global = owner->flag_50 ? 0.0f : rounded_product(&owner->level_6c, &owner->levels.global_4c);
    std::array<float, 64> left{}, right{}, combined{};
    auto* table = read<void*>(stream, 0x3c);
    auto* descriptor = static_cast<unsigned char*>(read<void*>(table, 8));
    auto* row = static_cast<unsigned char*>(read<void*>(stream, 0x48));
    auto* const end = reinterpret_cast<unsigned char*>(reinterpret_cast<std::uintptr_t>(row)
        + static_cast<std::uint32_t>(read<std::int32_t>(stream, 0x4c)) * 12u);
    const bool had_rows = row != end;
    while (row != end) {
        advance_sound_stream_row_00a85b50(row, nullptr, rounded_load(&dt));
        const auto offset = read<std::int32_t>(descriptor, 0xc);
        const auto format = read<std::int32_t>(descriptor, 0x10);
        const auto mix = [&]() {
            const float operands[]{read<float>(row, 4), read<float>(stream, 0xc),
                read<float>(stream, 0x14), read<float>(read<void*>(stream, 0x3c), 0x14), global};
            return rounded_mix(operands);
        };
        if (format == 1) left[offset] = right[offset] = combined[offset] = mix();
        else if (format == 2) {
            right[offset] = 0.0f; left[offset + 1] = 0.0f;
            const float gain = mix();
            right[offset + 1] = left[offset] = combined[offset + 1] = combined[offset] = gain;
        } else {
            for (std::int32_t channel = 0; channel < read<std::int32_t>(descriptor, 8); ++channel)
                right[offset + channel] = left[offset + channel] = combined[offset + channel] = mix();
        }
        row += 12; descriptor += 20;
    }
    if (had_rows) owner = &current(context);
    if (read<std::int32_t>(stream, 0x20) != 2 || !read<void*>(stream, 0x44)) return;
    if (owner->native_vtable_00 != 0x00d5b44cu)
        throw std::logic_error("Stream routing requires the recovered D5B44C speaker-layout virtual");
    const auto layout = sound_stream_speaker_layout_00a87b60(*owner);
    const auto route = [&](std::int32_t speaker, const float* levels) {
        context.fmod.channel_set_speaker_levels(read<void*>(stream, 0x44), speaker, levels,
            read<std::int32_t>(read<void*>(stream, 0x3c), 0x18));
    };
    if (layout == 1) { route(0, combined.data()); return; }
    if (layout == 4) { route(2, combined.data()); return; }
    if (layout != 2) {
        const std::array<float, 64> silence{};
        for (std::int32_t speaker = 2; speaker != 6; ++speaker)
            context.fmod.channel_set_speaker_levels(read<void*>(stream, 0x44), speaker, silence.data(), 6);
    }
    route(0, left.data()); route(1, right.data());
}

std::int32_t query_sound_stream_open_state_00a86b40(void* stream, SoundStreamRuntimeContext& context) {
    std::int32_t state = 2;
    if (auto* sound = read<void*>(stream, 0x40)) {
        std::uint32_t percent; std::uint8_t starving; // Native outputs are unwritten, then never read.
        context.fmod.sound_get_open_state(sound, &state, &percent, &starving);
        if (state == 2) {
            context.fmod.release_sound(read<void*>(stream, 0x40));
            write<void*>(stream, 0x40, nullptr);
            { Builder log(context); log.text("Stream open error:").name(at(stream, 0x2c)); }
            stop_sound_stream_00a86bf0(stream, context);
        }
    }
    return state;
}
void stop_sound_stream_00a86bf0(void* stream, SoundStreamRuntimeContext& context) {
    if (read<std::int32_t>(stream, 0x20) == 1) {
        auto state = query_sound_stream_open_state_00a86b40(stream, context);
        while (state != 0 && state != 2) { Sleep(0); state = query_sound_stream_open_state_00a86b40(stream, context); }
        write<std::uint32_t>(stream, 0x20, state == 2 ? 3 : 2);
    }
    if (read<std::int32_t>(stream, 0x20) == 2) {
        { Builder log(context); log.text("Stream ").name(at(stream, 0x24)).text(" stops FMOD streaming"); }
        if (auto* channel = read<void*>(stream, 0x44)) {
            context.fmod.channel_get_position(channel, field<std::uint32_t>(stream, 0x18), 1);
            Builder log(context); log.text("Raw savedpos ").position(read<std::uint32_t>(stream, 0x18));
        } else write<std::uint32_t>(stream, 0x18, 0);
        const auto position = read<std::uint32_t>(stream, 0x18);
        write<std::uint32_t>(stream, 0x18, position < 1000 || position > read<std::uint32_t>(stream, 0x1c) ? 0 : position - 1000);
        { Builder log(context); log.text("New savedpos ").position(read<std::uint32_t>(stream, 0x18)); }
        if (auto* channel = read<void*>(stream, 0x44)) context.fmod.channel_stop(channel);
        write<void*>(stream, 0x44, nullptr);
        if (auto* sound = read<void*>(stream, 0x40)) memory_error(context.fmod.release_sound(sound), context);
        write<void*>(stream, 0x40, nullptr);
    } else { Builder log(context); log.text("Stream '").name(at(stream, 0x24)).text("' already stopped"); }
    write<std::uint32_t>(stream, 0x20, 0);
    write<std::uint8_t>(stream, 0xa, 0); write<std::uint8_t>(stream, 0xb, 0);
    write<std::uint8_t>(stream, 8, 0); write<std::uint8_t>(stream, 9, 0);
    write<std::uint32_t>(stream, 0xc, context.one_00d7a24c);
}
void ready_sound_stream_00a86de0(void* stream, SoundStreamRuntimeContext& context) {
    { Builder log(context); log.text("Stream OnReady ").name(at(stream, 0x24)); }
    memory_error(context.fmod.system_play_sound(current(context).system.system, -1,
        read<void*>(stream, 0x40), true, field<void*>(stream, 0x44)), context);
    context.fmod.channel_set_priority(read<void*>(stream, 0x44), 0);
    std::int32_t type, format, channels, bits; // Required calls, native outputs never subsequently read.
    memory_error(context.fmod.sound_get_format(read<void*>(stream, 0x40), &type, &format, &channels, &bits), context);
    if (read<std::uint32_t>(stream, 0x18)) {
        { Builder log(context); log.text("Stream '").name(at(stream, 0x24)).text("' restarted at pos:").position(read<std::uint32_t>(stream, 0x18)); }
        memory_error(context.fmod.channel_set_position(read<void*>(stream, 0x44), read<std::uint32_t>(stream, 0x18), 1), context);
        write<std::uint32_t>(stream, 0x18, 0);
    }
}
void start_sound_stream_00a867b0(void* stream, const NativeString& name, SoundStreamRuntimeContext& context) {
    auto one = context.one_00d7a24c;
    auto other = one;
    std::int32_t selected = -1;
    if (name.length()) {
        selected = find_sound_dialog_record_00a865f0(read<void*>(stream, 0x3c), &name);
        one = context.one_00d7a24c; other = 0;
    }
    for (std::int32_t index = 0; index < read<std::int32_t>(stream, 0x4c); ++index) {
        auto* row = static_cast<unsigned char*>(read<void*>(stream, 0x48)) + static_cast<std::uint32_t>(index) * 12u;
        const auto gain = index == selected ? one : other;
        write(row, 0, gain); write(row, 4, gain);
    }
    const auto state = read<std::int32_t>(stream, 0x20);
    if (state == 1 || state == 2) return;
    const bool resume = read<std::uint32_t>(stream, 0x18) != 0;
    write<std::uint32_t>(stream, 0x20, 1);
    write<std::uint8_t>(stream, 8, 0); write<std::uint8_t>(stream, 9, 0);
    write<std::uint8_t>(stream, 0xa, 0); write<std::uint8_t>(stream, 0xb, 0);
    write(stream, 0xc, one);
    if (resume) { write<std::uint8_t>(stream, 0xb, 1); write<std::uint32_t>(stream, 0xc, 0); }
    auto* filename = read<const char*>(stream, 0x30);
    const bool loop = read<std::uint8_t>(read<void*>(stream, 0x3c), 0x1c) != 0;
    if (!filename) filename = context.null_filename_00f8bbee;
    memory_error(context.fmod.create_stream(current(context).system.system, filename,
        0x100c0u | (loop ? 2u : 0u), nullptr, field<void*>(stream, 0x40)), context);
    write<std::uint32_t>(stream, 0x1c, 0);
}
void update_sound_stream_00a874d0(void* stream, float dt, SoundStreamRuntimeContext& context) {
    if (current(context).flag_69) return;
    update_sound_stream_levels_00a86150(stream, rounded_load(&dt), context);
    advance_fade(stream, &dt, &context.one_00d7a24c, &context.fade_rate_00ce3dc8);
    const auto state = read<std::int32_t>(stream, 0x20);
    if (state == 2) {
        query_sound_stream_open_state_00a86b40(stream, context); // Return deliberately ignored.
        context.fmod.channel_set_paused(read<void*>(stream, 0x44), read<std::uint8_t>(stream, 8));
        auto playing = read<std::uint8_t>(&dt, 0);
        context.fmod.stream_channel_is_playing(read<void*>(stream, 0x44), playing);
        if (!read<std::uint8_t>(read<void*>(stream, 0x3c), 0x1c) && !playing) {
            write<std::uint32_t>(stream, 0x20, 3);
            Builder log(context); log.text("Stream '").name(at(stream, 0x24)).text("' Over");
            return;
        }
        if (read<std::uint8_t>(stream, 9)) {
            { Builder log(context); log.text("cStreamAudio::Update() NORMAL state accepts stop request ").name(at(stream, 0x24)); }
            stop_sound_stream_00a86bf0(stream, context);
        }
    } else if (state == 1 && query_sound_stream_open_state_00a86b40(stream, context) == 0) {
        {
            Builder log(context);
            const auto pause = read<std::uint8_t>(stream, 8), stop = read<std::uint8_t>(stream, 9);
            log.text("Stream ").name(at(stream, 0x24)).text(" Starting tostop=").integer(stop).text(" topause=").integer(pause);
        }
        write<std::uint32_t>(stream, 0x20, 2);
        context.fmod.sound_get_length(read<void*>(stream, 0x40), field<std::uint32_t>(stream, 0x1c), 1);
        if (!read<std::uint8_t>(stream, 9)) {
            ready_sound_stream_00a86de0(stream, context);
            update_sound_stream_levels_00a86150(stream, 0.0f, context);
            context.fmod.channel_set_paused(read<void*>(stream, 0x44), read<std::uint8_t>(stream, 8));
        } else {
            { Builder log(context); log.text("cStreamAudio::Update() STARTING state accepts stop request ").name(at(stream, 0x24)); }
            stop_sound_stream_00a86bf0(stream, context);
        }
    }
}
} // namespace bsp
