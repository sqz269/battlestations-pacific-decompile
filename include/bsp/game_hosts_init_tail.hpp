#pragma once
// bsp_game.exe milestone 2c: the tail of cSkeletonAppMidway::Init that milestones
// 1, 2a and 2b recorded as unimplemented phases.
//
// Addresses: 0073c3b0 the phase-2 hardware probe with its registry accessors
// 0098d4e0 / 0098d470 / 0098d570 / 0098d590 / 0098d5c0 / 0098d5f0 and its
// detection object 009927d0; 00736b60 / 00be0660 / 00736c30 / 00bd9230 /
// 00bb40b0 the provider factory tail 0073d94f-0073d98d; 004c1400 / 00736dd0 /
// 00736ea0 / 00b80a50 the phase-6 parser registrations 0073db41-0073db69;
// 0073fa70 / 00af06a0 the shared startup-singleton publication and 00740840 the
// phase-9 decal definition table.
//
// Nothing in this file is a reconstruction of native code. Every type is an
// integration binding that satisfies one of the host interfaces declared in
// bsp/app_init_tail.hpp and bsp/app_bootstrap.hpp with either a concrete
// implementation over an already reconstructed routine or the explicit
// unimplemented policy in GameHostLog.
//
// Evidence: docs/APP_INIT_TAIL.md, docs/LUA_OBJECT_API.md,
// docs/APP_INIT_BOOTSTRAP.md, docs/GAME_EXECUTABLE.md.

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/app_bootstrap.hpp"
#include "bsp/app_init_tail.hpp"
#include "bsp/structured_resource_registry.hpp"
#include "bsp/world_effects_startup.hpp"

namespace bsp::game {

class GameHostLog;
class GameVfsHost;

// ---------------------------------------------------------------------------
// A. Phase 2 hardware probe 0073c3b0, called at 0073d610
// ---------------------------------------------------------------------------

// What one probe run observed. `result` is the tail's verdict, which is only
// reached when all four stored values were present and correctly typed.
struct GameHardwareProbeSummary {
    bool ran{false};
    int stored_values_read{0};
    bool profile_complete{false};
    HardwareProfile stored{};
    HardwareProbeResult result{HardwareProbeResult::profile_incomplete};
    std::size_t changes{0};
    bool commit_enabled{false};
};

// The registry half of the probe is concrete: 0098d4e0 is RegOpenKeyExA on
// HKLM\SOFTWARE\Eidos\BSM_HWD with 0x20019 followed by RegQueryValueExA, and
// 0098d470 is RegCreateKeyExA with 0x20006 followed by RegSetValueExA. The
// machine half is the detection object 009927d0, which has no reconstruction
// (packet `hardware_detection_object`), so its four accessors and the localized
// message table take the unimplemented-host policy.
//
// Two decisions here are the milestone's own and are not claims about the game:
// the message box at 0073c827 is not shown and the registry write-back at
// 0073c843 is not performed unless --hardware-probe-commit is given, because an
// unattended run must not block on a modal dialog or rewrite a machine profile.
// Both are logged with the values the native would have used.
class GameHardwareProbe final : public HardwareProbeTailHost {
public:
    GameHardwareProbe(GameHostLog& log, bool commit);

    // 0073c3b0 in full: the four stored reads and their all-four gate through
    // bsp::probe_hardware_0073c3b0, then bsp::run_hardware_probe_tail_0073c3b0.
    void run();
    const GameHardwareProbeSummary& summary() const noexcept { return summary_; }

    // --- HardwareProbeTailHost, one method per native call site -------------
    std::uint32_t current_gpu_device_id() override;            // 0098c870
    std::string gpu_device_name(std::uint32_t id) override;    // 0098c890
    std::string current_sound_device() override;               // 0098d3d0
    std::uint32_t current_cpu_speed() override;                // 0098c7f0
    std::uint32_t current_memory_size() override;              // 0098c800
    std::string localized_message(std::int32_t index, const std::string& first,
        const std::string& second) override;                   // 00996060 / 00735450
    bool ask_write_default_options(const std::string& text,
        const std::string& caption) override;                  // 0073c827
    void write_default_options_file(const std::string& path) override;  // 0098f430
    void write_hardware_dword(const std::string& value_name,
        std::uint32_t value) override;                         // 0098d570
    void write_hardware_string(const std::string& value_name,
        const std::string& value) override;                    // 0098d590

    // 0098d5c0 and 0098d5f0, both through 0098d4e0. Public so the read-side
    // adapter can forward to them without a second registry binding.
    bool read_stored_qword(const std::string& value_name, std::uint32_t& out) const;
    bool read_stored_string(const std::string& value_name, std::string& out) const;

private:
    GameHostLog& log_;
    bool commit_{};
    GameHardwareProbeSummary summary_;
};

// ---------------------------------------------------------------------------
// D. The startup singleton publication shared by 0073fa70 and 00af06a0
// ---------------------------------------------------------------------------

class GameStartupSingletonPublication final : public StartupSingletonPublicationHost {
public:
    GameStartupSingletonPublication(GameHostLog& log,
        const StartupSingletonPublication& record, const char* label);
    void enter_lifetime_lock() override;                 // 00415350 then +10h
    void register_lifetime(void* instance) override;     // 00bd0c30
    void leave_lifetime_lock() override;
    void publish(void* instance) override;               // the store to the global
    void* published() const noexcept { return published_; }

private:
    GameHostLog& log_;
    StartupSingletonPublication record_;
    const char* label_{};
    void* published_{};
    int depth_{};
};

// ---------------------------------------------------------------------------
// E. Phase 9, the decal definition table 00740840 at 0073de8c
// ---------------------------------------------------------------------------

struct GameDecalTableSummary {
    bool published{false};
    bool script_read{false};
    bool script_ran{false};
    std::size_t definitions{0};
    std::size_t textures_requested{0};
    std::size_t shaders_requested{0};
    std::string error;
};

// The 1Ch decal system of 0073de6d: 0073fa70 publishes it at 00e1aea0, then
// 00740872 installs the derived vtable and the loader builds its own Lua state
// owner (00b66bd0), opens it with mask 1 (00b6a020), runs
// scripts/datatables/decals.lua through the mounted VFS (00b69d40) and walks the
// `Decals` global. The parse is bsp::load_decal_definitions_00740840; the four
// calls around it are this binding's.
class GameDecalTable {
public:
    GameDecalTable(GameHostLog& log, GameVfsHost& vfs);
    ~GameDecalTable();
    GameDecalTable(const GameDecalTable&) = delete;
    GameDecalTable& operator=(const GameDecalTable&) = delete;

    void run();
    const GameDecalTableSummary& summary() const noexcept { return summary_; }
    const std::vector<DecalDefinition>& definitions() const noexcept { return definitions_; }

private:
    GameHostLog& log_;
    GameVfsHost& vfs_;
    GameDecalTableSummary summary_;
    std::vector<DecalDefinition> definitions_;
};

}  // namespace bsp::game
