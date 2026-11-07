#pragma once
#include "bsp/sound_lifetime_access.hpp"
#include "bsp/sound_resource_runtime.hpp"

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace bsp {
struct VfsMountContext;
struct VfsCandidateRegistrations;
class VfsLuaScriptFiles;
class LuaScriptRuntime;
struct LuaRuntimeGlobals;
struct FrameClock;
class NativeStringStorage;
struct ResourceLoadEventHost;
class SingletonLifetimeDomain;
struct CameraAxesCrtAccess;
class SoundAlternateShutdownHost;
struct SoundSystemOwner;
struct SoundOwnerLifetimeBindings;
struct SoundEventQueryLockBindings;
struct SoundInstanceContext;
struct SoundSystemUpdateContext;
class FmodConfigurationLibrary;
class SoundResourceRuntime;
class SoundSampleRuntime;
class SoundChannelRuntime;
struct SoundRequestQueue;
struct SoundRetainedSourceRecord;
}

namespace bsp::game {

// Borrowed application services, all alive through explicit shutdown and the
// destruction of the runtime. Bind this runtime in the raw manager's concrete
// deletion bindings, or route the semantic fixture's deleting callback through
// owns_registered()/delete_registered(). No private VFS, clock,
// Lua runtime, singleton domain or alternate-engine implementation is supplied.
struct GameSoundRuntimeServices {
    VfsMountContext& mounts;
    const VfsCandidateRegistrations& registrations;
    VfsLuaScriptFiles& script_files;
    LuaScriptRuntime& script_runtime;
    const LuaRuntimeGlobals& script_globals;
    // Additive semantic/raw borrowed binding. Field order and existing aggregate
    // initializers remain valid; this changes source aggregate size, not a game ABI.
    SoundClockBinding clock;
    NativeStringStorage& strings;
    ResourceLoadEventHost& load_events;
    SoundLifetimeAccess lifetime;
    const CameraAxesCrtAccess& crt;
    SoundAlternateShutdownHost& alternate_shutdown;
    // Required when an alternate pointer is published. An empty function is
    // allowed only while that word is null; a live unbound call is an error.
    std::function<void(void*, float)> update_alternate;
    void* volatile* alternate_word = nullptr;
};

// Projections of writable PE globals; zero initialization matches loader state,
// not a claim about later native-process contents. EF aliases EC+3.
struct GameSoundRuntimeWords {
    std::uint32_t next_id_00f8bbd4{};
    std::uint32_t resource_bytes_00f8bbe4{};
    char null_pattern_00e17bf0{};
    std::array<char, 4> null_data_00f8bbec{};
};

struct GameSoundRuntimeSummary {
    bool started{};
    bool sound_enabled{};
    std::size_t classes{}, listeners{}, configured_types{}, configured_groups{};
    std::size_t resources{}, samples{}, active_entries{}, fmod_calls{}, fmod_errors{};
    std::uint32_t resource_bytes{};
    std::size_t file_opens{}, file_closes{}, file_reads{}, file_seeks{};
    // Host-owned VFS adapters reclaimed after successful SDK shutdown when
    // the installed library omitted Close (observed for native EOF state3).
    // Separate from actual FMOD callback closes; pending must end at zero.
    std::size_t file_reclaims{}, file_handles_pending{};
};

// Application composition of recovered routines, not a native object/ABI.
// Calls and owner mutations are serialized by the game thread. Exactly one
// runtime may hold the process-global FMOD file binding. External sample and
// channel references must be released before shutdown. Startup is one-shot.
class GameSoundRuntime final {
public:
    GameSoundRuntime(GameSoundRuntimeServices, const std::wstring& fmod_dll,
        const std::wstring& event_dll = {});
    ~GameSoundRuntime();
    GameSoundRuntime(const GameSoundRuntime&) = delete;
    GameSoundRuntime& operator=(const GameSoundRuntime&) = delete;

    // Pass !settings.audio.enabled_24. True selects the recovered no-sound
    // branch; false permits the actual installed FMOD hardware-device path.
    void startup(bool sound_disabled);
    // OnInitOnce 004DD6F6: owns the five GUI sample references until teardown,
    // using this runtime's actual class table, cache and sample final release.
    void initialize_gui_sound_set();
    // 004C1B90's queue: the request bytes (+18h..+1Dh) and forced slot (+20h) of
    // the published [00F89B34] owner initialize_gui_sound_set built. Null when
    // that owner is not published (before OnInitOnce, after its destruction).
    SoundRequestQueue* gui_sound_requests() noexcept;
    // 004E5496: 00941140 on the published owner, then 00941070 for the slot it
    // picks; `select_device_active` is 00941140's slot-1 input-device test.
    // False, having done nothing, when no owner is published: nothing can have
    // written a request then, since every writer goes through the same queue.
    bool apply_gui_sound_requests_00941140(const std::function<bool()>& select_device_active);
    // The scene record's three 14h environment sound slots at record+D5Ch +
    // slot*14h (slot 0 Ocean, 1 Air, 2 Underwater), held here in place of the
    // record at [00E188A8]+5FCh. 004ED530 (through 004ED690) writes the name
    // at +4 and the floats at +0Ch/+10h; 004ED910 acquires the sample at +0
    // through 00A83FD0 and passes the record to 00A7F2F0, whose retained
    // channel copies it. Slot 0..2; anything else throws.
    SoundRetainedSourceRecord& scene_environment_record(std::size_t slot);
    // 004BFD70's release of the three sample references (+D5Ch, +D70h,
    // +D84h: InterlockedDecrement of sample+4, vtable slot 0 at zero, the
    // slot cleared), with each record's name returned to the pool, so the
    // slots read as a newly constructed record (004DA2A0) again. This
    // process has no session teardown, so a new scene header releases the
    // previous scene's slots, and the manager/cache teardown releases them
    // before FMOD closes. Idempotent.
    void release_scene_environment_records() noexcept;
    void shutdown();
    bool started() const noexcept;
    GameSoundRuntimeSummary summary() const noexcept;
    SoundSystemOwner& owner();
    FmodConfigurationLibrary& fmod() noexcept;
    SoundResourceRuntime& resources() noexcept;
    SoundSampleRuntime& samples() noexcept;
    SoundChannelRuntime& channels() noexcept;
    SoundInstanceContext& instance_context() noexcept;
    SoundSystemUpdateContext& update_context() noexcept;
    SoundOwnerLifetimeBindings& lifetime_bindings() noexcept;
    SoundEventQueryLockBindings& event_query_lifetime() noexcept;
    GameSoundRuntimeWords& words() noexcept;
    void* volatile& current_alternate() noexcept;

    // Manager/cache dispatch uses allocation identity, independently of the
    // current publication used by native unregister. Query/alternate owners
    // remain externally created: their publication must retain their identity
    // until their actual deleting routine clears it.
    bool owns_registered(void*) const noexcept;
    // Receives native deleting flags. Bit0 releases the separately allocated
    // projection after its recovered destructor. Never shuts the shared domain.
    void delete_registered(void*, std::uint32_t flags) noexcept;
    // FMOD C callbacks catch host exceptions at the DLL boundary. Call this on
    // the game thread to report the original error; startup also checks it.
    void rethrow_file_error();

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
}
