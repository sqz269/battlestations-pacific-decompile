#include "bsp/cc13_sweepC2.hpp"
#include "bsp/game_sound_dialog_runtime.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/sound_alternate_owner.hpp"
#include "bsp/sound_dialog_logical.hpp"
#include "bsp/sound_dialog_table.hpp"
#include "bsp/sound_sample_runtime.hpp"
#include "bsp/sound_stream_owner.hpp"
#include "bsp/sound_stream_runtime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include "bsp/sound_streaming_cc13.hpp"
#include "bsp/native_physical_file_date.hpp"
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
#include <exception>
#include <stdexcept>

namespace bsp::game {
namespace {
std::uint32_t profile(const void* p) noexcept {
    std::uint32_t value; std::memcpy(&value, p, sizeof value); return value;
}
} // namespace

struct GameSoundDialogRuntime::Impl final : SoundAlternateOwnerDependencies,
    SoundDialogLogicalDependencies, SoundStreamOwnerHost, GameplayEffectComponentLifetime {
    struct Contexts {
        SoundDialogTableContext table;
        SoundStreamRuntimeContext stream_runtime;
        SoundStreamOwnerContext stream_owner;
        SoundDialogLogicalBindings logical;
        SoundAlternateOwnerBindings alternate;

        Contexts(Impl& self, GameSoundRuntime& core, SoundStreamFmodHost& fmod)
            : table{core.samples().sample_context().strings, core.samples(),
                  self.globals.format_counts_00e12ef0, 0},
              stream_runtime{core.lifetime_bindings().global_00f8bbd8, fmod,
                  table.strings, self.globals.one_00d7a24c, self.globals.fade_rate_00ce3dc8,
                  core.words().null_data_00f8bbec.data() + 2,
                  self.globals.null_integer_format_01090ab4},
              stream_owner{table, self, self},
              logical{table.strings, self, self.globals.alternate_00f8bbcc, self},
              alternate{core.lifetime_bindings().domain, self.globals.alternate_00f8bbcc,
                  table.strings, self, self} {
            // The table loader copies temporary+C only after its last string
            // allocation and overwrites the appended row+C before any external
            // call. Its scratch0 does not escape or stand in for a PE global.
        }
    };

    GameSoundDialogRuntimeGlobals globals;
    GameSoundRuntime* core{};
    std::unique_ptr<Contexts> contexts;
    NativeSoundAlternateOwnerStorage* allocation{};
    bool attempted{}, constructed{}, teardown_failed{};

    explicit Impl(GameSoundDialogRuntimeGlobals input) : globals(input) {
        if (!globals.null_integer_format_01090ab4)
            throw std::invalid_argument("Dialog logging requires the borrowed1090AB4 fallback string");
    }
    Contexts& bound() {
        if (!contexts) throw std::logic_error("Dialog runtime is not attached to core sound");
        return *contexts;
    }
    // Stack-only adapter, constructed only by the ON service. It borrows the
    // A79230 owner/channels, its actual string allocator, and its live refs.
    struct MusicDirectorAdapter final : cc13_sweepC2::DialogMusicHost,
        cc13_sweepC::MusicRowHost {
        Impl& runtime;
        explicit MusicDirectorAdapter(Impl& value) : runtime(value) {}
        void* image_address(std::uint32_t address) override {
            // A786F0 uses F8BBD0 only as the log's verified empty C string.
            if (address == cc13_sweepC2::kEmptyNativeText)
                return const_cast<char*>(cc13_sweepC2::kEmptyText);
            throw std::logic_error("Music adapter received an unrelated image address");
        }
        std::int32_t interlocked_increment_00ce221c(std::int32_t* p) override {
            return InterlockedIncrement(reinterpret_cast<volatile LONG*>(p));
        }
        std::int32_t interlocked_decrement_00ce2220(std::int32_t* p) override {
            return InterlockedDecrement(reinterpret_cast<volatile LONG*>(p));
        }
        void refcounted_destroy_vslot_00(void* object) override {
            runtime.zero_references_slot_00(object);
        }
        void string_pool_release_00419cc0_00bd1510(void* data, std::uint32_t size) override {
            runtime.bound().table.strings.release(static_cast<char*>(data), size);
        }
        void* sound_stream_copy_name_00a77ff0(void* stream, void* out) override {
            return &copy_sound_stream_name_00a77ff0(stream, *static_cast<NativeString*>(out),
                runtime.bound().table.strings);
        }
        bool native_string_equals_insensitive_00435c40(void* text, const void* other) override {
            return equal_native_string_headers_00435c40(text, other);
        }
        void log_printf_text_int_004254b0(const char* format, const char* text,
            std::int32_t value) override { std::fprintf(stderr, format, text, value); }
        void log_printf_int_int_004254b0(const char* format, std::int32_t first,
            std::int32_t second) override { std::fprintf(stderr, format, first, second); }
        void logical_dialog_record_assign_00a78230(void* record, const void* source) override {
            copy_sound_dialog_record_00a78230(record, source,
                runtime.bound().table.strings, runtime);
        }
        void logical_dialog_configuration_clear_00a77bb0(void* configuration) override {
            clear_sound_dialog_configuration_references_00a77bb0(configuration, runtime);
        }
        void music_director_select_row_00a782e0(void* director, void* field) override {
            cc13_sweepC::music_director_select_row_and_store_name_00a782e0(*this, director, field);
        }
        void sound_stream_select_row_00a86710(void* stream, const void* name) override {
            cc13_audio::select_sound_stream_row_00a86710(stream, name);
        }
        void native_string_resize_0041dd40(void* string, std::uint32_t length,
            std::uint32_t keep) override {
            resize_native_string_header_0041dd40(string, runtime.bound().table.strings,
                length, keep != 0);
        }
    };
    void check_owner(void* object) const {
        if (!constructed || object != allocation || !object || profile(object) != 0x00d58f78)
            throw std::logic_error("Dialog service requires its constructed actual234h owner");
    }
    void load_stream_table_00a87060(void* table, const NativeString& path) override {
        load_sound_dialog_table_00a87060(table, path, bound().table);
    }
    void update_logical_00a78820(void* logical, float dt, float gain) override {
        update_sound_dialog_logical_00a78820(logical, dt, gain, bound().logical);
    }
    void start_logical_00a783f0(void* logical) override {
        start_sound_dialog_logical_00a783f0(logical, bound().logical);
    }
    void invoke_callback230_ecx(void* callback, NativeString& temporary) override {
        if (!globals.callbacks)
            throw std::logic_error("Published dialog callback230 requires its mission binding");
        globals.callbacks->invoke_callback230_ecx(callback, temporary);
    }
    void* construct_stream_00a877d0(void* receiver, NativeString& owned_name,
        void* owned_table) override {
        // Direct consuming call: no second argument copy, retain or destructor.
        return construct_sound_stream_00a877d0(receiver, owned_name, owned_table, bound().stream_owner);
    }
    void start_stream_00a867b0(void* stream, const void* name_header) override {
        start_sound_stream_00a867b0(stream, *static_cast<const NativeString*>(name_header),
            bound().stream_runtime);
    }
    void set_stream_row_gain_00a86670(void* stream, std::int32_t row, float gain) override {
        set_sound_stream_row_gain_00a86670(stream, row, gain);
    }
    void set_stream_gain_00a864f0(void* stream, float gain) override {
        set_sound_stream_gain_00a864f0(stream, gain);
    }
    void update_stream_00a874d0(void* stream, float dt) override {
        update_sound_stream_00a874d0(stream, dt, bound().stream_runtime);
    }
    void resolve_stream_name_00bdf4c0(void* name_header) override {
        // The established VFS resolver normalizes and updates this actual
        // header even on false. Native A877D0 deliberately ignores AL.
        (void)core->samples().resolve_name_00bdf4c0(*static_cast<NativeString*>(name_header));
    }
    void stop_stream_00a86bf0(void* stream) override {
        stop_sound_stream_00a86bf0(stream, bound().stream_runtime);
    }
    void zero_references_slot_00(void* object) override {
        // Caller already performed InterlockedDecrement(actual+4). These are
        // current native profiles, never cached at construction or ref capture.
        switch (profile(object)) {
        case 0x00d5b360:
            scalar_delete_sound_stream_00a87b30(object, 1, bound().stream_owner);
            return;
        case 0x00d58f80:
            scalar_delete_sound_alternate_table_00a796f0(object, 1, bound().table.strings);
            return;
        case 0x00d5b074:
            core->samples().zero_references_slot_00(object);
            return;
        default:
            throw std::logic_error("Dialog reference has an unreconstructed current profile");
        }
    }
    void destroy(void* object, std::uint32_t flags) {
        check_owner(object);
        constructed = false; // Native teardown may not be entered twice.
        try {
            scalar_delete_sound_alternate_owner_00a790d0(
                allocation, static_cast<std::uint8_t>(flags), bound().alternate);
        } catch (...) { teardown_failed = true; throw; }
        if (flags & 1u) allocation = nullptr;
    }
    void shutdown() {
        if (teardown_failed)
            throw std::logic_error("Dialog native teardown failed; its remaining storage is not reusable");
        if (!allocation) return;
        if (constructed) {
            // Explicit host shutdown follows the A882C0 consumer schedule:
            // unregister captured owner before entering its deleting body.
            bound().alternate.domain.get_manager_00415350()->unregister_object(allocation);
            destroy(allocation, 1);
        } else {
            // A prior flags0 deleting call ran the native body but retained
            // allocation. Free it once, without another member destruction.
            singleton_lifetime_free(allocation);
            allocation = nullptr;
        }
    }
};

GameSoundDialogRuntime::GameSoundDialogRuntime(GameSoundDialogRuntimeGlobals globals)
    : impl_(std::make_unique<Impl>(globals)) {}
GameSoundDialogRuntime::~GameSoundDialogRuntime() {
    try { impl_->shutdown(); } catch (...) { std::terminate(); }
}
void GameSoundDialogRuntime::attach(GameSoundRuntime& core, SoundStreamFmodHost& fmod) {
    auto& self = *impl_;
    if (self.contexts || self.attempted)
        throw std::logic_error("Dialog runtime attachment is one-shot");
    if (&core.current_alternate() != &self.globals.alternate_00f8bbcc)
        throw std::invalid_argument("Core sound and dialog require the same alternate publication word");
    self.contexts = std::make_unique<Impl::Contexts>(self, core, fmod);
    self.core = &core;
}
void GameSoundDialogRuntime::startup() {
    auto& self = *impl_;
    auto& context = self.bound();
    if (!self.core->started()) throw std::logic_error("Dialog startup requires a started core sound runtime");
    if (self.attempted) throw std::logic_error("Dialog startup is one-shot");
    if (self.globals.alternate_00f8bbcc)
        throw std::logic_error("Dialog startup cannot replace another published owner");
    self.attempted = true;
    self.allocation = static_cast<NativeSoundAlternateOwnerStorage*>(singleton_lifetime_allocate(
        {SingletonAllocationKind::object, 0x234, sizeof(NativeSoundAlternateOwnerStorage)}));
    try {
        construct_sound_alternate_owner_00a79230(*self.allocation, context.alternate);
        self.constructed = true;
    } catch (...) {
        // Native constructor cleans its completed members/base, but failed
        // base registration can leave this pointer published/registered. This
        // additional host cleanup removes only our allocation identity before
        // freeing it; it never invokes the incomplete derived destructor.
        try {
            context.alternate.domain.get_manager_00415350()->unregister_object(self.allocation);
            if (self.globals.alternate_00f8bbcc == self.allocation)
                self.globals.alternate_00f8bbcc = nullptr;
        } catch (...) { self.teardown_failed = true; throw; }
        singleton_lifetime_free(self.allocation);
        self.allocation = nullptr;
        throw;
    }
}
void GameSoundDialogRuntime::shutdown() { impl_->shutdown(); }
bool GameSoundDialogRuntime::started() const noexcept { return impl_->constructed; }
bool GameSoundDialogRuntime::owns_owner(const void* object) const noexcept {
    return object && object == impl_->allocation;
}
NativeSoundAlternateOwnerStorage& GameSoundDialogRuntime::owner() {
    impl_->check_owner(impl_->allocation);
    return *impl_->allocation;
}
void GameSoundDialogRuntime::update(void* object, float dt) {
    impl_->check_owner(object);
    update_sound_alternate_owner_00a789c0(*impl_->allocation, dt, impl_->bound().alternate);
}
void GameSoundDialogRuntime::set_music_level(std::int32_t level) {
    auto& self = *impl_;
    auto* const current = self.globals.alternate_00f8bbcc;
    self.check_owner(current);
    Impl::MusicDirectorAdapter calls(self);
    // 00A788A0 calls the existing level/row selection body; OFF keeps no service.
    cc13_sweepC2::music_director_set_level_00a788a0(calls,
        *static_cast<NativeSoundAlternateOwnerStorage*>(current), level);
}
void GameSoundDialogRuntime::set_music_target_level(float level) {
    auto& self = *impl_;
    void* const current = self.globals.alternate_00f8bbcc;
    self.check_owner(current);
    // A79230 produces these float words as 1,1,0; A789C0 updates +220h.
    cc13_sweepC::StreamedDialogLevelView view{
        cc13_sweepC::at<float>(current, 0x220), cc13_sweepC::at<float>(current, 0x224),
        cc13_sweepC::at<float>(current, 0x228)};
    // 00A77860 calls the existing fade target/rate body; OFF keeps no service.
    cc13_sweepC::streamed_dialog_manager_set_target_level_00a77860(view, level);
}
void GameSoundDialogRuntime::clear_channels() {
    auto& self = *impl_;
    // 004DA857 MOV ECX,[00F8BBCC]; 004DA85D CALL 00A77C10: the current
    // publication, which must be this runtime's constructed owner.
    void* const current = self.globals.alternate_00f8bbcc;
    self.check_owner(current);
    clear_sound_alternate_channels_00a77c10(
        *static_cast<NativeSoundAlternateOwnerStorage*>(current), self.bound().alternate);
}
void GameSoundDialogRuntime::set_all_paused(std::uint8_t paused) {
    auto& self = *impl_;
    void* const current = self.globals.alternate_00f8bbcc;
    self.check_owner(current);
    // Reuse the existing body: stores owner+215h, visits owner+8/+Ch and
    // logical+4h, then changes stream+8h only in its real STARTING/PLAY state.
    // The running stream updater applies that byte to the live FMOD channel.
    cc13_sweepC::streamed_dialog_manager_set_all_paused_00a77760(current, paused);
}
GameSoundRuntime* GameSoundDialogRuntime::attached_core() const noexcept { return impl_->core; }
void GameSoundDialogRuntime::delete_alternate_slot00(void* object, std::uint32_t flags) {
    impl_->destroy(object, flags);
}
} // namespace bsp::game
