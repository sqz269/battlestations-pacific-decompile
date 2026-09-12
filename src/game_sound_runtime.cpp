#include "bsp/game_sound_runtime.hpp"
#include "bsp/sound_channel_runtime.hpp"
#include "bsp/sound_sample_runtime.hpp"
#include "bsp/sound_sample_cache_shutdown.hpp"
#include "bsp/memory_stream.hpp"

#include <atomic>
#include <exception>
#include <filesystem>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <unordered_set>
#include <utility>

namespace bsp::game {

struct GameSoundRuntime::Impl final : SoundSystemUpdateHost, SoundShutdownFmodHost {
    static std::atomic<Impl*> callback_owner;
    GameSoundRuntimeServices services;
    GameSoundRuntimeWords words;
    void* volatile own_alternate{};
    void* volatile& alternate;
    SoundSystemOwner* volatile current_owner{};
    SoundAuxiliaryTreeOwner* volatile current_cache{};
    SoundEventQueryLock* volatile current_query_lock{};
    SoundOwnerLifetimeBindings lifetime;
    SoundEventQueryLockBindings query_lifetime;
    SoundSystemState system;
    SoundConfigurationState configuration;
    SoundManagerLevels levels;
    SoundClassOwnership classes;
    std::unique_ptr<SoundSystemOwner> manager;
    std::unique_ptr<SoundAuxiliaryTreeOwner> cache;
    // Declared before its users and destroyed after all explicit FMOD cleanup.
    FmodConfigurationLibrary library;
    SoundResourceRuntime resources;
    SoundSampleRuntime samples;
    BankSoundChannelVirtuals bank_virtuals;
    SoundInstanceContext instance;
    SoundSpatialChannelContext spatial;
    SpatialSoundEventVirtuals event_virtuals;
    SoundEventInstanceContext event;
    SoundChannelRuntime channels;
    SoundSystemUpdateContext update;
    SoundRetainedShutdownRuntime retained;
    SoundSystemShutdownContext shutdown_context;
    FrameClockSoundStartupHost clock;
    VfsSoundConfigurationLuaOwner lua;
    NativeSoundFileContext files;
    NativeSoundFileContext* previous_files{};
    bool bound{}, attempted{}, running{}, manager_destroyed{}, cache_destroyed{}, cleanup_failed{};
    std::atomic<std::size_t> opens{}, closes{}, reads{}, seeks{};
    std::atomic<std::size_t> reclaimed_files{};
    mutable std::mutex file_handles_mutex;
    std::unordered_set<void*> open_file_handles;
    std::optional<FmodResult> event_release_result;
    std::mutex file_error_mutex;
    std::exception_ptr file_error;

    Impl(GameSoundRuntimeServices input, const std::wstring& core, const std::wstring& events)
        : services(std::move(input)), alternate(services.alternate_word ? *services.alternate_word : own_alternate),
          lifetime{services.lifetime, current_owner, current_cache},
          query_lifetime{services.lifetime, current_query_lock}, configuration(services.strings), classes(levels),
          manager(std::make_unique<SoundSystemOwner>(system, configuration, levels, classes)),
          library(core, events.empty() ? (std::filesystem::path(core).parent_path() / L"fmod_event.dll").wstring() : events,
              {open_callback, close_callback, read_callback, seek_callback}),
          resources(system, current_owner, words.resource_bytes_00f8bbe4, services.mounts,
              services.registrations, services.load_events, library, services.strings),
          samples(current_owner, current_cache, words.resource_bytes_00f8bbe4, services.mounts,
              services.registrations, resources, library, services.strings,
              &words.null_pattern_00e17bf0, words.null_data_00f8bbec.data()),
          instance{current_owner, words.next_id_00f8bbd4, services.strings, samples, library, bank_virtuals},
          spatial{instance, library, library, services.crt},
          event{instance, library, library, services.crt, words.null_data_00f8bbec.data() + 3, event_virtuals},
          channels(spatial, event), update{*this, library, channels},
          retained(channels, samples, services.alternate_shutdown),
          shutdown_context{lifetime, retained, update, resources, *this, alternate, services.strings},
          clock(services.clock), lua(services.script_files, services.script_runtime, services.script_globals),
          files{services.strings, this, resolve_file, open_file} {
        if (!services.crt.dispatch_bypass_0109dd78 || !services.crt.except_00c27489)
            throw std::invalid_argument("Game sound spatial services require the actual CRT bindings");
    }

    const ClockTimestamp* current_timestamp_slot14() override {
        return sound_frame_clock_current_00bee050(services.clock);
    }
    void* current_alternate_00f8bbcc() override { return alternate; }
    void update_alternate_slot04(void* object, float dt) override {
        if (!services.update_alternate)
            throw std::logic_error("Published alternate sound owner has no update service");
        services.update_alternate(object, dt);
    }
    FmodResult dsp_remove(void* dsp) override { return library.dsp_remove(dsp); }
    FmodResult dsp_release(void* dsp) override { return library.dsp_release(dsp); }
    FmodResult channel_group_release(void* group) override { return library.channel_group_release(group); }
    void memory_get_stats(std::int32_t* current, std::int32_t* maximum) override {
        library.memory_get_stats(current, maximum);
    }
    FmodResult release_event_system(void* event_handle) override {
        const auto result = library.release_event_system(event_handle);
        event_release_result = result;
        return result;
    }
    void reclaim_closed_system_files() {
        // Host-only ownership, after successful SDK shutdown and its final
        // callbacks. Native EOF can destroy its stream without Sound::release;
        // this installed SDK then drops that sound without invoking file Close.
        // These pointers are our callable VFS adapters, NEVER FMOD Sound handles.
        // Even with no open files, a failed release cannot establish that the
        // SDK has stopped using its library or callbacks. Caller retains the
        // binding and takes the existing failed-teardown exception path.
        if (event_release_result && *event_release_result != FmodResult::ok)
            throw std::logic_error("FMOD shutdown failed; its library and callback lifetime must be retained");
        for (;;) {
            void* handle;
            {
                std::lock_guard<std::mutex> lock(file_handles_mutex);
                if (open_file_handles.empty()) return;
                if (!event_release_result)
                    throw std::logic_error("Cannot reclaim VFS adapters before successful FMOD shutdown");
                const auto entry = open_file_handles.begin();
                handle = *entry;
                open_file_handles.erase(entry);
            }
            sound_file_close_00a7b750(handle, nullptr);
            ++reclaimed_files;
        }
    }
    static bool resolve_file(void* raw, NativeString& name) {
        return static_cast<Impl*>(raw)->samples.resolve_name_00bdf4c0(name);
    }
    static void* open_file(void* raw, const NativeString& name, std::uint32_t flags) {
        auto& self = *static_cast<Impl*>(raw);
        auto opened = open_resource_memory_00bdf310_fragment(self.services.mounts,
            name.data() ? std::string(name.data(), name.length()) : std::string{}, flags);
        if (!opened.provider_opened) return nullptr;
        if (!opened.stream || !opened.stream->has_backing() || !opened.stream->fully_initialized())
            throw std::runtime_error("FMOD VFS file did not provide a complete stream: " + opened.error);
        return create_sound_memory_stream_adapter(std::move(opened.stream));
    }
    void remember_file_error() noexcept {
        std::lock_guard<std::mutex> lock(file_error_mutex);
        if (!file_error) file_error = std::current_exception();
    }
    static std::int32_t __stdcall open_callback(const char* name, std::int32_t unicode,
        std::uint32_t* size, void** handle, void** userdata) noexcept {
        auto* self = callback_owner.load();
        if (!self) { *handle = nullptr; return 0x17; }
        try {
            const auto result = sound_file_open_00a7d410(name, unicode, size, handle, userdata);
            if (!result) {
                try {
                    std::lock_guard<std::mutex> lock(self->file_handles_mutex);
                    self->open_file_handles.insert(*handle);
                } catch (...) {
                    // The source callback itself cannot accept ownership if
                    // its bookkeeping allocation fails. SDK sees failed open.
                    sound_file_close_00a7b750(*handle, nullptr); // Native userdata is unwritten and ignored.
                    *handle = nullptr;
                    throw;
                }
                ++self->opens;
            }
            return result;
        } catch (...) { self->remember_file_error(); *handle = nullptr; return 0x17; }
    }
    static std::int32_t __stdcall close_callback(void* handle, void* userdata) noexcept {
        auto* self = callback_owner.load();
        try {
            if (self && handle) {
                // Detach before native close frees the adapter. A concurrent
                // Open may reuse that address as soon as it has been freed.
                std::lock_guard<std::mutex> lock(self->file_handles_mutex);
                self->open_file_handles.erase(handle);
            }
            const auto result = sound_file_close_00a7b750(handle, userdata);
            if (self && handle && !result) {
                ++self->closes;
            }
            return result;
        } catch (...) { if (self) self->remember_file_error(); return 0x17; }
    }
    static std::int32_t __stdcall read_callback(void* handle, void* destination,
        std::uint32_t requested, std::uint32_t* actual, void* userdata) noexcept {
        auto* self = callback_owner.load();
        try {
            if (self) ++self->reads;
            return sound_file_read_00a79930(handle, destination, requested, actual, userdata);
        } catch (...) { if (self) self->remember_file_error(); if (actual) *actual = 0; return 0x16; }
    }
    static std::int32_t __stdcall seek_callback(void* handle, std::uint32_t position, void* userdata) noexcept {
        auto* self = callback_owner.load();
        try {
            if (self) ++self->seeks;
            return sound_file_seek_00a79970(handle, position, userdata);
        } catch (...) { if (self) self->remember_file_error(); return 0x17; }
    }
    void bind_files() {
        Impl* expected{};
        if (!callback_owner.compare_exchange_strong(expected, this))
            throw std::logic_error("Another game sound runtime owns the FMOD file callbacks");
        previous_files = bind_sound_file_context(&files);
        if (previous_files) {
            bind_sound_file_context(previous_files);
            callback_owner = nullptr;
            throw std::logic_error("An existing FMOD file binding must finish before game sound startup");
        }
        bound = true;
    }
    void unbind_files() noexcept {
        if (!bound) return;
        bind_sound_file_context(previous_files);
        callback_owner = nullptr;
        bound = false;
    }
    void rethrow_file_error() {
        std::exception_ptr error;
        { std::lock_guard<std::mutex> lock(file_error_mutex); error = file_error; }
        if (error) std::rethrow_exception(error);
    }
    void destroy_manager(std::uint32_t flags) {
        if (!manager || manager_destroyed) return;
        auto* object = manager.get();
        // The destructor owns native unregister and leaves dead +44/+48/+54
        // words. Do not dereference them or run it twice after this point.
        manager_destroyed = true;
        running = false;
        if (flags & 1u) manager.release();
        try {
            scalar_delete_sound_system_00a883b0(object, static_cast<std::uint8_t>(flags), shutdown_context);
            reclaim_closed_system_files();
        } catch (...) { cleanup_failed = true; throw; }
        unbind_files();
    }
    void destroy_cache(std::uint32_t flags) {
        if (!cache || cache_destroyed) return;
        auto* object = cache.get();
        cache_destroyed = true;
        if (flags & 1u) cache.release();
        scalar_delete_sound_sample_cache_00a88750(object, flags, samples.sample_cache_context(), lifetime);
    }
    void destroy_query(std::uint32_t flags) {
        if (auto* object = current_query_lock) {
            // A89B40 clears publication and frees storage, but does not remove
            // its shared-domain entry. Explicit host teardown must remove that
            // pointer before freeing it; it is safe after a drain popped it too.
            services.lifetime.get_manager_00415350()->unregister_object(object);
            scalar_delete_sound_event_query_lock_00a89b40(object, flags, query_lifetime);
        }
    }
    void recover_failed_constructor() {
        // A88770's recovered EH destroys only the completed base. This is an
        // additional application RAII policy for its still-live external assets,
        // not a claim that native constructor unwind releases FMOD. The shared
        // domain remains live, and the completed auxiliary singleton remains
        // published until resource/sample cleanup has finished.
        manager_destroyed = true;
        lua.close();
        if (current_owner) {
            // The base did not finish: registration is its only throwing stage.
            // Remove any inserted entry and clear publication without running
            // the derived destructor or destroying unconstructed base members.
            unregister_sound_system_owner_00a7b230(*manager, lifetime);
        }
        if (current_cache && !cache) {
            // Auxiliary registration can throw before its unique_ptr reaches
            // A88770. Its factory already freed that allocation, while native
            // publication survives. Remove only the pointer value; never read
            // a vtable or call a destructor on the dead object.
            services.lifetime.get_manager_00415350()->unregister_object(current_cache);
            current_cache = nullptr;
        }
        // Resource final-release lookup requires the same owner+54 even though
        // native base unwind already removed its singleton registration.
        current_owner = manager.get();
        try {
            if (manager->resource_owner_54) {
                resources.destroy_owner(*manager->resource_owner_54);
                manager->resource_owner_54.destroy_storage_preserving_word();
            }
            if (system.event_system) release_event_system(system.event_system);
            reclaim_closed_system_files();
        } catch (...) { current_owner = nullptr; throw; }
        current_owner = nullptr;
        destroy_cache(0);
        destroy_query(1);
        unbind_files();
    }
    void shutdown() {
        if (cleanup_failed)
            throw std::logic_error("Sound teardown failed; live FMOD ownership cannot be discarded");
        if (running) destroy_manager(0);
        destroy_cache(0);
        destroy_query(1);
        unbind_files();
    }
};
std::atomic<GameSoundRuntime::Impl*> GameSoundRuntime::Impl::callback_owner{};

GameSoundRuntime::GameSoundRuntime(GameSoundRuntimeServices services,
    const std::wstring& core, const std::wstring& events)
    : impl_(std::make_unique<Impl>(std::move(services), core, events)) {}
GameSoundRuntime::~GameSoundRuntime() {
    try { impl_->shutdown(); } catch (...) { std::terminate(); }
}
void GameSoundRuntime::startup(bool disabled) {
    auto& self = *impl_;
    if (self.attempted) throw std::logic_error("Game sound startup is one-shot");
    if (self.alternate && !self.services.update_alternate)
        throw std::logic_error("Alternate sound update service is required for a published owner");
    self.bind_files();
    self.attempted = true;
    try {
        construct_sound_system_00a88770(*self.manager, self.lifetime, self.cache,
            disabled, 0, 0, self.clock, self.resources, self.library, self.library,
            self.lua, self.shutdown_context);
        self.running = true;
        self.rethrow_file_error();
    } catch (...) {
        // A second cleanup exception cannot safely unload a DLL whose live
        // objects or file callbacks survived; match the host destructor policy.
        try {
            if (self.running) self.shutdown();
            else self.recover_failed_constructor();
        } catch (...) { std::terminate(); }
        throw;
    }
}
void GameSoundRuntime::shutdown() { impl_->shutdown(); }
bool GameSoundRuntime::started() const noexcept { return impl_->running; }
GameSoundRuntimeSummary GameSoundRuntime::summary() const noexcept {
    const auto& self = *impl_;
    GameSoundRuntimeSummary value;
    value.started = self.running;
    value.sound_enabled = self.system.sound_enabled;
    value.classes = self.levels.classes_98.size();
    value.listeners = self.configuration.listeners_ac.size();
    value.configured_types = self.configuration.types_38.size();
    value.configured_groups = self.configuration.channel_groups_128.size();
    value.active_entries = self.levels.entries_8c.size();
    if (self.running && self.manager->resource_owner_54)
        value.resources = self.manager->resource_owner_54->records_04.size();
    if (self.current_cache) value.samples = self.current_cache->count_0c;
    value.resource_bytes = self.words.resource_bytes_00f8bbe4;
    value.fmod_calls = self.library.calls().size();
    for (const auto& call : self.library.calls()) if (call.result != FmodResult::ok) ++value.fmod_errors;
    value.file_opens = self.opens; value.file_closes = self.closes;
    value.file_reads = self.reads; value.file_seeks = self.seeks;
    value.file_reclaims = self.reclaimed_files;
    { std::lock_guard<std::mutex> lock(self.file_handles_mutex); value.file_handles_pending = self.open_file_handles.size(); }
    return value;
}
SoundSystemOwner& GameSoundRuntime::owner() {
    if (!impl_->running) throw std::logic_error("Game sound owner is not running");
    return *impl_->manager;
}
FmodConfigurationLibrary& GameSoundRuntime::fmod() noexcept { return impl_->library; }
SoundResourceRuntime& GameSoundRuntime::resources() noexcept { return impl_->resources; }
SoundSampleRuntime& GameSoundRuntime::samples() noexcept { return impl_->samples; }
SoundChannelRuntime& GameSoundRuntime::channels() noexcept { return impl_->channels; }
SoundInstanceContext& GameSoundRuntime::instance_context() noexcept { return impl_->instance; }
SoundSystemUpdateContext& GameSoundRuntime::update_context() noexcept { return impl_->update; }
SoundOwnerLifetimeBindings& GameSoundRuntime::lifetime_bindings() noexcept { return impl_->lifetime; }
SoundEventQueryLockBindings& GameSoundRuntime::event_query_lifetime() noexcept { return impl_->query_lifetime; }
GameSoundRuntimeWords& GameSoundRuntime::words() noexcept { return impl_->words; }
void* volatile& GameSoundRuntime::current_alternate() noexcept { return impl_->alternate; }
bool GameSoundRuntime::owns_registered(void* object) const noexcept {
    return object && ((!impl_->manager_destroyed && object == impl_->manager.get()) ||
        (!impl_->cache_destroyed && object == impl_->cache.get()) ||
        object == impl_->current_query_lock || object == impl_->alternate);
}
void GameSoundRuntime::delete_registered(void* object, std::uint32_t flags) noexcept {
    try {
        if (object && !impl_->manager_destroyed && object == impl_->manager.get()) impl_->destroy_manager(flags);
        else if (object && !impl_->cache_destroyed && object == impl_->cache.get()) impl_->destroy_cache(flags);
        else if (object && object == impl_->current_query_lock) impl_->destroy_query(flags);
        else if (object && object == impl_->alternate) impl_->services.alternate_shutdown.delete_alternate_slot00(object, flags);
        else throw std::logic_error("Foreign singleton passed to game sound deleting dispatcher");
    } catch (...) { std::terminate(); }
}
void GameSoundRuntime::rethrow_file_error() { impl_->rethrow_file_error(); }
}
