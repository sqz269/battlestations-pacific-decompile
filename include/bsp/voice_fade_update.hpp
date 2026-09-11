#pragma once

namespace bsp {

struct VoicePlaybackManager;
struct SoundSystemOwner;
struct MissionLuaHostServices;
class NativeStringStorage;

// Remaining services at the original call/load sites. The fade algorithm,
// string lifetime, sound application and thread-safe Lua dispatch are concrete.
class VoiceFadeHost {
public:
    virtual ~VoiceFadeHost() = default;
    virtual void log_004254b0(const char* message) = 0; // CALL 005B8CDD
    // Resolve CURRENT [00E188A8]+1A08 after the callback string was cleared.
    // Must return a valid service; no cached/null/fake-success fallback.
    virtual MissionLuaHostServices& mission_lua_005b8d14() = 0;
};

struct VoiceFadeBindings {
    NativeStringStorage& strings;
    SoundSystemOwner* volatile& sound_singleton_00f8bbd8;
    VoiceFadeHost& host;
};

// Complete bounded 005B8C30; native ECX=manager, stack float delta, RET4.
// Steps D4 toward D8 by float(DC*delta), consumes a nonempty callback on
// ordered equality, then rereads D4 and the current sound singleton. Applies
// the sound manager's existing +4C after changing +6C, to dirty its entries.
// Callback/logging/storage may reenter and replace fields/singletons. The
// manager and every sound owner loaded must remain valid through their use.
// Numeric domain excludes signaling-NaN/x87 trap/status equivalence; no game
// ABI compatibility is claimed. Evidence: docs/VOICE_FADE_UPDATE.md.
void update_voice_sound_fade_005b8c30(VoicePlaybackManager& manager,
    float delta, VoiceFadeBindings& bindings);

} // namespace bsp
