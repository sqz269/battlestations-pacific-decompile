#include "bsp/voice_fade_update.hpp"

#include "bsp/mission_lua_host.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/native_string.hpp"
#include "bsp/sound_system_owner.hpp"
#include "bsp/unit_motion.hpp"
#include "bsp/voice_playback.hpp"

#include <string>

namespace bsp {
namespace {

// Native state 0 is armed only AFTER 00426060 returns. The implicit
// NativeString destructor does not release; normal and C++ exceptional exits
// use the existing actual-header destruction body, preserving its callbacks.
struct FadeCallbackCleanup {
    NativeString& callback;
    NativeStringStorage& strings;
    ~FadeCallbackCleanup() noexcept
    {
        destroy_native_string_header_0041dd20(&callback, strings);
    }
};

} // namespace

void update_voice_sound_fade_005b8c30(VoicePlaybackManager& manager,
    float delta, VoiceFadeBindings& bindings)
{
    // 005B8C4B..73: x87 multiply with a float spill before the scalar helper.
    const float step = static_cast<float>(
        static_cast<double>(manager.fade_rate_dc) * static_cast<double>(delta));
    manager.fade_value_d4 = unit_step_towards_0042ac60(
        manager.fade_value_d4, manager.fade_target_d8, step);

    // FUCOMIP/LAHF/TEST 44h/JP: unordered skips completion; +/-0 compare equal.
    if (manager.fade_value_d4 == manager.fade_target_d8) {
        // 0041E870 constructs an empty temporary from 00CE3A0C (first byte0).
        // It has no allocation. 00449AF0 against that length-zero header is
        // exactly this length predicate, without entering CRT __stricmp.
        if (manager.fade_callback_e0.length() != 0) {
            bindings.host.log_004254b0("MoveSoundFade Callback"); // 005B8CDD
            NativeString callback;
            // Log and allocation may alter the original; copy current fields
            // through the existing native helper, not a pre-log snapshot.
            copy_construct_native_string_header_00426060(
                &callback, &manager.fade_callback_e0, bindings.strings); // 005B8CEA
            FadeCallbackCleanup cleanup{callback, bindings.strings};
            // 0041E350 with the known empty CString delegates to resize(0,0).
            // Clear BEFORE dispatch; a callback can safely install another fade.
            manager.fade_callback_e0.resize_0041dd40(bindings.strings, 0, false);
            auto& lua = bindings.host.mission_lua_005b8d14();
            const std::string name = callback.length() == 0
                ? std::string{}
                : std::string(callback.data(), callback.length());
            // Original args: self=null, name=&copy, arguments=null, first=0,
            // last=-1. The concrete existing wrapper retains lifecycle and
            // worker-thread queuing behavior. Its outcome is ignored natively.
            (void)call_named_entry_point_threadsafe(lua, name, {}); // 005B8D1C
        } // cleanup at 005B8D21..43, including its possible reentry
    }

    // Read AFTER callback and temporary cleanup. Do not publish the earlier
    // stepped value if either operation began another fade or changed D4.
    const float current = manager.fade_value_d4; // 005B8D48/50
    auto* sound = bindings.sound_singleton_00f8bbd8; // 005B8D4A
    // FUCOMIP/LAHF/TEST44/JNP skips only ordered equality, so NaN dirties.
    if (sound->level_6c != current) {
        sound->level_6c = current; // 005B8D6E
        sound = bindings.sound_singleton_00f8bbd8; // 005B8D73: reload, not cached
        const float global = sound->levels.global_4c; // 005B8D79
        apply_sound_global_level_00a7a440(sound->levels, global); // 005B8D7F
    }
}

} // namespace bsp
