#pragma once
#include "bsp/sound_lifetime_access.hpp"

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
    FrameClock& clock;
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
