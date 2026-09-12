#pragma once
#include "bsp/sound_shutdown.hpp"
#include <array>
#include <cstdint>
#include <memory>

namespace bsp {
struct NativeSoundAlternateOwnerStorage;
class SoundStreamFmodHost;
}
namespace bsp::game {
class GameSoundRuntime;

// Required only when the actual owner+230 callback word is nonnull. The token
// is the CURRENT native callback word; name is the actual caller temporary in
// ECX. Mission integration supplies the concrete callable translation.
class GameSoundDialogCallbackHost {
public:
    virtual ~GameSoundDialogCallbackHost() = default;
    virtual void invoke_callback230_ecx(void* token, NativeString& name) = 0;
};

// Borrow actual application publication/constant storage. None of these words
// is allocated, initialized or copied into an independent state model here.
// Live PE evidence: E12EF0 counts {0,1,2,6}; D7A24C bits3F800000;
// CE3DC8 bytes000000403333D33F; 1090AB4 initially an empty C string.
// The F8BBEE fallback aliases GameSoundRuntime::words().null_data_00f8bbec+2.
struct GameSoundDialogRuntimeGlobals {
    void* volatile& alternate_00f8bbcc;
    const std::array<std::uint32_t, 4>& format_counts_00e12ef0;
    const volatile std::uint32_t& one_00d7a24c;
    const volatile double& fade_rate_00ce3dc8;
    const char* null_integer_format_01090ab4;
    GameSoundDialogCallbackHost* callbacks = nullptr;
};

// Application composition over recovered actual234h/5Ch/54h/20h storage;
// this class has no native function address or original object ABI.
// Construct this facade before GameSoundRuntime, pass it as alternate_shutdown,
// bind update_alternate to update(pointer,dt), and pass the same alternate_word.
// Then attach the constructed core and its real FMOD stream adapter. Startup
// requires a started core. Caller services remain alive through core shutdown.
// A core destructor/drain may delete the dialog owner through this facade;
// its later destructor does not repeat that native teardown.
class GameSoundDialogRuntime final : public SoundAlternateShutdownHost {
public:
    explicit GameSoundDialogRuntime(GameSoundDialogRuntimeGlobals);
    ~GameSoundDialogRuntime();
    GameSoundDialogRuntime(const GameSoundDialogRuntime&) = delete;
    GameSoundDialogRuntime& operator=(const GameSoundDialogRuntime&) = delete;

    // One attachment. FMOD must route to this core's live library/system;
    // no default/null-success implementation is supplied by the facade.
    void attach(GameSoundRuntime&, SoundStreamFmodHost&);
    void startup();
    void shutdown();
    bool started() const noexcept;
    bool owns_owner(const void*) const noexcept;
    NativeSoundAlternateOwnerStorage& owner();
    void update(void* actual_owner, float dt);
    void delete_alternate_slot00(void* actual_owner, std::uint32_t flags) override;

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};
} // namespace bsp::game
