#include "bsp/game_title_sound.hpp"
#include "bsp/game_sound_dialog_runtime.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/fmod_configuration_library.hpp"
#include "bsp/movie_player.hpp"
#include "bsp/native_physical_file_date.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/sound_sample_runtime.hpp"
#include "bsp/sound_stream_owner.hpp"
#include "bsp/sound_stream_runtime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstring>
#include <stdexcept>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Title stream playback requires MSVC Win32.
#endif

namespace bsp {
namespace {
template<class T> T read(const void* p, std::size_t offset) noexcept {
    T value; std::memcpy(&value, static_cast<const unsigned char*>(p) + offset, sizeof value); return value;
}
template<class T> void write(void* p, std::size_t offset, T value) noexcept {
    std::memcpy(static_cast<unsigned char*>(p) + offset, &value, sizeof value);
}
struct ConstructedStringCleanup {
    NativeString& value;
    NativeStringStorage& strings;
    ~ConstructedStringCleanup() { destroy_native_string_header_0041dd20(&value, strings); }
};
float music_volume(const volatile float& input) noexcept {
    const volatile float* source = &input;
    float result;
    __asm { mov eax, source }
    __asm { fld dword ptr [eax] }
    __asm { fstp result }
    return result;
}
}
bool movie_named_clip_is_playing_004f8ba0(const MoviePlayer& player,
    const NativeString& name, MoviePlayerHost& host) {
    void* const selected = player.selected_movie;
    if (!selected) return false;
    auto& widget = host.movie_widget_state(selected);
    if (!equal_native_string_headers_00435c40(&name, &widget.filename)) return false;
    return movie_widget_is_playing_00aac8e0(widget, host);
}
bool intro_movie_is_active_00584a30(MoviePlayer* volatile& publication,
    NativeStringStorage& strings, MoviePlayerHost& host) {
    NativeString name;
    name.resize_0041dd40(strings, 0x16, true);
    if (name.data()) std::memcpy(name.data(), "movies/midwaytheme.bik", name.length() + 1u);
    // D9ACBC state0 is armed after construction. Failed initial allocation
    // has no native string cleanup state.
    ConstructedStringCleanup cleanup{name, strings};
    auto* const player = publication;
    if (!player) throw std::logic_error("Title music requires the current movie-player publication");
    return movie_named_clip_is_playing_004f8ba0(*player, name, host);
}
void request_sound_stream_fade_in_00a85c20(void* stream) noexcept {
    if (read<std::uint32_t>(stream, 0x20) <= 2u && read<std::uint8_t>(stream, 0xa) == 0) {
        write<std::uint8_t>(stream, 0xb, 1);
        write<std::uint32_t>(stream, 0xc, 0); // XORPS/MOVSS positive zero.
    }
}
}

namespace bsp::game {
struct GameTitleSound::Impl final : SoundStreamOwnerHost {
    GameSoundRuntime& core;
    GameTitleSoundApplication application;
    SoundDialogTableContext table;
    SoundStreamRuntimeContext runtime;
    SoundStreamOwnerContext owner;

    Impl(GameSoundRuntime& sound, const GameSoundDialogRuntimeGlobals& globals,
        GameTitleSoundApplication input)
        : core(sound), application(input),
          table{sound.samples().sample_context().strings, sound.samples(), globals.format_counts_00e12ef0, 0},
          runtime{sound.lifetime_bindings().global_00f8bbd8, sound.fmod(), table.strings,
              globals.one_00d7a24c, globals.fade_rate_00ce3dc8,
              sound.words().null_data_00f8bbec.data() + 2, globals.null_integer_format_01090ab4},
          owner{table, sound.samples(), *this} {
        if (!application.selected_track_00e19504 || !globals.null_integer_format_01090ab4)
            throw std::invalid_argument("Title sound requires its borrowed live C-string publications");
    }
    void resolve_stream_name_00bdf4c0(void* name) override {
        (void)core.samples().resolve_name_00bdf4c0(*static_cast<NativeString*>(name));
    }
    void stop_stream_00a86bf0(void* stream) override {
        stop_sound_stream_00a86bf0(stream, runtime);
    }
    GameTitleSoundMenuView current_menu() noexcept { return application.menus.current_menu_00e198ac(); }
    void start() {
        if (!core.started()) throw std::logic_error("Title music requires a started core sound runtime");
        if (intro_movie_is_active_00584a30(application.movie_player_00e18d48,
                table.strings, application.movie_widgets)) return;
        if (current_menu().title_stream_50) return;
        void* const allocation = singleton_lifetime_allocate({SingletonAllocationKind::object, 0x54, 0x54});
        void* constructed = nullptr;
        if (allocation) {
            NativeString owned_name;
            try {
                // Native null table argument is already an owned-null slot.
                // The path is captured only AFTER the allocation callback.
                copy_construct_native_string_header_00426060(&owned_name,
                    &current_menu().title_path_40, table.strings);
                // A877D0 consumes both argument owners on every exit. Do not
                // add caller-side name destruction if its body throws.
                constructed = construct_sound_stream_00a877d0(allocation, owned_name, nullptr, owner);
            } catch (...) { singleton_lifetime_free(allocation); throw; }
        }
        current_menu().title_stream_50 = constructed;
        NativeString selected;
        construct_native_string_cstring_0041e870(&selected, application.selected_track_00e19504, table.strings);
        {
            ConstructedStringCleanup cleanup{selected, table.strings};
            start_sound_stream_00a867b0(current_menu().title_stream_50, selected, runtime);
        }
        const float gain = music_volume(application.music_volume_00f889a8);
        set_sound_stream_gain_00a864f0(current_menu().title_stream_50, gain);
        request_sound_stream_fade_in_00a85c20(current_menu().title_stream_50);
    }
};
GameTitleSound::GameTitleSound(GameSoundRuntime& core, const GameSoundDialogRuntimeGlobals& globals,
    GameTitleSoundApplication application) : impl_(std::make_unique<Impl>(core, globals, application)) {}
GameTitleSound::~GameTitleSound() = default;
void GameTitleSound::start_005884a0() { impl_->start(); }
void GameTitleSound::update_stream(void* stream, float dt) {
    update_sound_stream_00a874d0(stream, dt, impl_->runtime);
}
void GameTitleSound::stop_stream(void* stream) { stop_sound_stream_00a86bf0(stream, impl_->runtime); }
void GameTitleSound::release_stream_reference(void* stream) {
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(static_cast<unsigned char*>(stream) + 4)) == 0) {
        if (read<std::uint32_t>(stream, 0) != 0x00d5b360)
            throw std::logic_error("Title stream final reference requires its current D5B360 profile");
        scalar_delete_sound_stream_00a87b30(stream, 1, impl_->owner);
    }
}
}
