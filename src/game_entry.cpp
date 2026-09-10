#include "bsp/game_entry.hpp"

#include <cstring>

namespace bsp {
namespace {

// Literal addresses of the seven switch strings scanned at 004e55c9..004e5735.
// The order is the recovered order and the duplicate is present in the binary.
constexpr const char kSkipLogos[] = "skipLogos"; // 00ce7fe4
constexpr const char kNoSkipLogos[] = "noskipLogos"; // 00ce7fd8
constexpr const char kSkipTitle[] = "skipTitle"; // 00ce7fcc
constexpr const char kSkipBriefings[] = "skipBriefings"; // 00ce7fbc
constexpr const char kLockitMark[] = "lockitMark"; // 00ce7fb0
constexpr const char kLockitRaw[] = "lockitRaw"; // 00ce7fa4
constexpr const char kScenarioSuffix[] = ".scn"; // 00ce7888

} // namespace

bool startup_switch_present(const char* command_line, const char* token) noexcept {
    if (command_line == nullptr) {
        return false;
    }
    const char* const hit = std::strstr(command_line, token);
    if (hit == nullptr) {
        return false;
    }
    // 004e55e1: CMP EAX,-1 on the offset of the hit. The comparison cannot be
    // true for a non-null strstr result, so this always falls through; it is
    // kept because it is what the binary does.
    return (hit - command_line) != -1;
}

GameStartupState game_on_init(GameStartupSystems& systems, GameStartupFlags& flags) {
    systems.install_startup_callback();
    systems.create_startup_controller();
    systems.register_startup_handler();

    bool scenario_path = false;
    if (systems.logo_sequence_forced()) {
        // 004e57ac: the whole scan is skipped and the logos are forced on.
        flags.skip_logos = false;
    } else {
        const char* const command_line = systems.command_line();
        if (startup_switch_present(command_line, kSkipLogos)) {
            flags.skip_logos = true;
        }
        if (startup_switch_present(command_line, kNoSkipLogos)) {
            flags.skip_logos = false;
        }
        if (startup_switch_present(command_line, kSkipTitle)) {
            flags.skip_title = true;
        }
        if (startup_switch_present(command_line, kSkipBriefings)) {
            flags.skip_briefings = true;
        }
        if (startup_switch_present(command_line, kLockitMark)) {
            flags.lockit_mode = 1;
        }
        if (startup_switch_present(command_line, kLockitRaw)) {
            flags.lockit_mode = 2;
        }
        // 004e56d8 repeats the skipBriefings test with the same literal and
        // the same store. Removing it would change nothing, so it stays.
        if (startup_switch_present(command_line, kSkipBriefings)) {
            flags.skip_briefings = true;
        }
        if (startup_switch_present(command_line, kScenarioSuffix)) {
            flags.skip_title = true;
            flags.skip_logos = true;
            scenario_path = true;
        }
    }

    if (!scenario_path && !flags.skip_logos) {
        // 004e57bc: the default boot. The state is written before the logo
        // object exists, and a failed allocation is not checked afterwards.
        systems.create_logo_sequence();
        return GameStartupState::kLogoSequence;
    }

    // 004e5748. The three calls after OnInitOnce run on both branches even
    // though OnInitOnce(false) would repeat them; first_time is always true.
    systems.on_init_once(true);
    systems.on_init_title();
    systems.notify_title_ready();

    if (!scenario_path) {
        // 004e5815: OnInitTitle already wrote 2; this rewrites the same value.
        return GameStartupState::kTitleScreen;
    }

    systems.on_init_mission();
    systems.drain_state_requests();
    // 004e578c writes game+5D4h before the queue push at 004e5796, and both
    // carry the same value.
    systems.enqueue_state_request(GameStartupState::kScenarioLoad);
    return GameStartupState::kScenarioLoad;
}

FrameHook* frame_hook_construct(FrameHook& hook, SingletonLifetimeManager& manager,
    FrameHook** published) {
    hook.vtable = kFrameHookBaseVtable; // 007372c8

    manager.lock(); // 007372ce..007372ed, skipped when manager+10h is null
    *published = &hook; // 007372f6, the store to 00f88c20
    manager.register_object(*published); // 0073730a, the argument is re-read
    manager.unlock(); // 0073730f..0073731e

    // 0073e4ef, the derived constructor inlined by the caller and identical to
    // the standalone 00737830.
    hook.vtable = kFrameHookDerivedVtable;
    hook.flag = false;
    hook.value = 0.0F;
    return &hook;
}

} // namespace bsp
