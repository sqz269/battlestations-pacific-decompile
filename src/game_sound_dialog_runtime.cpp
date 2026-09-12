#include "bsp/game_sound_dialog_runtime.hpp"
#include "bsp/game_sound_runtime.hpp"
#include "bsp/sound_alternate_owner.hpp"
#include "bsp/sound_dialog_logical.hpp"
#include "bsp/sound_dialog_table.hpp"
#include "bsp/sound_sample_runtime.hpp"
#include "bsp/sound_stream_owner.hpp"
#include "bsp/sound_stream_runtime.hpp"
#include "bsp/singleton_lifetime.hpp"
#include <cstddef>
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
void GameSoundDialogRuntime::delete_alternate_slot00(void* object, std::uint32_t flags) {
    impl_->destroy(object, flags);
}
} // namespace bsp::game
