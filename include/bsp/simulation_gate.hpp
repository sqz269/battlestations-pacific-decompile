#pragma once
// The in-mission simulation gate of the game update BSP_Game_OnMove (004E4A40),
// 004E50B0..004E525E, plus the routines it calls.
// Addresses: 004C40A0, 004CCE50, 004F7740, 0068A140, 004BFE50, 004DB030,
//            0068C1F0, 004CD0F0, 004BCA50, 006881F0, 006882A0, 00689030.
// Every name below is a hypothesis, not a recovered symbol. Evidence and the
// call-by-call recovery are in docs/GAME_SIMULATION_GATE.md. Nothing here is
// binary compatible with the original: the vtables, the pooled strings and the
// checked-iterator containers are not reproduced.
#include <array>
#include <cstddef>
#include <cstdint>
#include <string_view>

#include "bsp/game_frame_control.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The two cinematic flags, game+634h and game+635h
// ---------------------------------------------------------------------------

// 004CD0F0 is the only writer of both. +634h is the "HUD hidden" flag the
// cinematic and pause paths raise; +635h is `hidden && allow_simulation` and is
// what lets the simulation keep running behind a hidden HUD. The gate at
// 004E50B0 tests exactly `+634h == 0 || +635h != 0`, so raising +634h with
// allow_simulation false is what actually stops the simulation.
struct CinematicModeFlags {
    bool hud_hidden{false}; // game+634h
    bool simulate_while_hidden{false}; // game+635h
};

// The four GUI layer groups 004CD0F0 drives, in the order the native body
// assigns them. Only "Sonar" was read back from the image, at 00CE7840; the
// other three are string immediates in the body that Ghidra already resolved.
inline constexpr std::string_view kLayerGroup3dEffect = "3DEffect";
inline constexpr std::string_view kLayerGroupInGameGui = "InGameGUI";
inline constexpr std::string_view kLayerGroupWarnings = "Warnings";
inline constexpr std::string_view kLayerGroupSonar = "Sonar";

// The byte 00A7F890 receives for each group. InGameGUI is handed game+634h
// itself, so the byte reads as "hidden", not "visible"; the other three share
// one value that stays raised while the game is in, or heading into,
// GameStateId::kMissionTeardown.
struct HudLayerVisibility {
    bool in_game_gui_hidden{false};
    bool effects_hidden{false}; // 3DEffect, Warnings and Sonar share this byte
};

// 004CD0F0's inputs to that decision, 004CD142..004CD165. `pending_teardown`
// is `game+5E8h != 0 && front(game+5D8h) == 10h`, the queue peek at 004CD158.
HudLayerVisibility hud_layer_visibility_004cd0f0(
    bool hud_hidden, int game_state, bool pending_teardown) noexcept;

// 004CD26C, the last write of the routine: `game+635h = (game+634h != 0) & p3`.
CinematicModeFlags cinematic_flags_004cd0f0(bool hud_hidden, bool allow_simulation) noexcept;

// ---------------------------------------------------------------------------
// The entry predicate, 004E50B0 / 004E5118 / 004E5124
// ---------------------------------------------------------------------------

struct SimulationGateEntry {
    CinematicModeFlags cinematic{}; // game+634h and game+635h
    int game_state{}; // game+5D4h
    bool simulation_suspended{}; // game+7184h
};

// True when OnMove runs the in-mission block. All three failures land on the
// same fallback at 004E53B4, which calls the interface-only update instead.
bool simulation_gate_open(const SimulationGateEntry& entry) noexcept;

// ---------------------------------------------------------------------------
// The pause decision, 004E5153..004E5211
// ---------------------------------------------------------------------------

// The two action indices the gate queries through 004C43C0. Index 1 is the one
// OnMove phase 15 resets every frame the timed-action set is non-empty
// (docs/GAME_INPUT_TICK.md), so a script can suppress it; 4Bh is not reset
// there and 0068C1F0 reads the same edge for its own handling.
inline constexpr int kPauseAction = 1; // 004E51DD, pushed from EDI = 1
inline constexpr int kAlternatePauseAction = 0x4B; // 004E5180

// Interface ids from the name table at 00E08CD8, which 006881F0 and 00689030
// index by the id at menu+20h to print `INTF_*` in their trace strings.
inline constexpr int kInterfaceMultiInGame = 0x17; // INTF_MULTIINGAME
inline constexpr int kInterfaceFreeCamera = 0x29; // INTF_FREECAMERA
inline constexpr int kInterfaceEngineMovie = 0x2D; // INTF_ENGINEMOVIECAMERA in the 00e08cd8 table
inline constexpr int kInterfaceShipyard = 0x31; // INTF_SHIPYARD
inline constexpr int kInterfaceAirbase = 0x32; // INTF_AIRBASE

// 0068A140, __thiscall, ECX = DAT_00E198C4, RET. `id == 32h || id == 31h`.
bool is_base_interface_0068a140(int interface_id) noexcept;

// The game modes 004BFE50 accepts. 0068C1F0 opens with the same four-way
// compare on the same value, so this is one mode set, not two.
inline constexpr std::array<int, 4> kSpectatorGameModes{4, 5, 6, 7};

// 004BCA50, __thiscall, ECX = game, RET. Returns game+614h unless the mode is
// neither forced (game+61Ch) nor part of a network session (game+1FE4h) and is
// not already 8 or 9, in which case it reports 8. 004BFE50 inlines this.
inline constexpr int kDefaultSinglePlayerGameMode = 8;
int effective_game_mode_004bca50(
    int raw_mode, bool mode_forced, bool multiplayer_session) noexcept;

// 004BFE50, no arguments, RET. Reads the game through DAT_00E188A8 and ignores
// the ECX the call site sets. The two accessors both run on the object at
// DAT_00E198C4+BCh: 006529E0 returns its byte +4h and 006529F0 returns
// `int +5Ch == 1`.
bool multiplayer_spectator_active_004bfe50(
    int effective_mode, bool spectator_enabled, int spectator_state) noexcept;

// 004F7740, no arguments, RET. Five checked vectors of 4-byte elements sit at
// 00E18CFC, 00E18D0C, 00E18D1C, 00E18D2C and 00E18D3C. The walk starts at the
// last one with the counter at 5, steps back 10h a time and returns the counter
// at the first non-empty vector, or 0 when it walks past the first. 004F7620
// uses the same five levels with the same 1..5 numbering, which is what fixes
// the base and the direction.
inline constexpr std::size_t kPriorityLevelCount = 5;
int top_populated_level_004f7740(
    const std::array<std::size_t, kPriorityLevelCount>& level_sizes) noexcept;

// The three branch targets of 004E5153..004E5211, named after the code they
// reach. The player-eliminated arm jumps over both branches to 004E525E, so
// neither the pause menu nor the interface update runs on that frame, and the
// rest of the simulation still does.
enum class SimulationGateBranch {
    kGateClosed, // 004E53B4, the simulation did not run at all
    kSimulationOnly, // 004E517A, the jump to 004E525E
    kPauseMenuOpened, // 004E5213
    kInterfaceOnly, // 004E5242
};

struct PauseDecisionInputs {
    // 004E5153..004E517A. The slot array is game+18CCh and the selector is
    // game+18ECh; the test only runs inside a network session.
    bool multiplayer_session{}; // game+1FE4h != 0
    bool active_slot_present{}; // the selected slot pointer is non-null
    std::int16_t active_slot_counter{}; // slot+10h, read as a signed word

    // 004E5180..004E51DB, the 4Bh chain.
    bool alternate_pause_pressed{}; // 004C43C0(4Bh)
    bool hud_hidden{}; // game+634h, re-read at 004E518D
    int top_populated_level{}; // 004F7740, compared against EDI = 1 with JG
    bool base_interface_active{}; // 0068A140
    bool spectator_gate_enabled{}; // game+19C4h
    bool spectator_flag{}; // 006529E0 on DAT_00E198C4+BCh
    bool multiplayer_spectator_active{}; // 004BFE50

    // 004E51DD..004E5211, the shared pause test.
    bool pause_pressed{}; // 004C43C0(1)
    bool scene_console_open{}; // 00425D10()+25Ch
    bool scene_modal_188h{}; // 00425D10()+188h
    bool scene_modal_218h{}; // 00425D10()+218h
};

// 004E5153..004E5211 as one pure function. The 4Bh chain does **not** open the
// pause menu on its own: every one of its failures falls to the same 004E51DD
// test that the not-pressed case reaches, and the chain's only effect is that
// when it holds completely and 004BFE50 reports no spectator, control jumps
// straight to the interface-only branch and the pause test is skipped. So the
// chain is a suppression of pause, not a second way to reach it.
SimulationGateBranch decide_simulation_gate_branch(const PauseDecisionInputs& inputs) noexcept;

// ---------------------------------------------------------------------------
// The engine-movie latch, 004E5138..004E5153
// ---------------------------------------------------------------------------

// 004CCE50 builds `<mission base name>.ema` from the mission path at
// [game+5FCh]+90Ch, truncated at its first '.' (00CE3A70), with the extension
// at 00CE7838 appended, and hands it to the 74h-byte player it allocates and
// stores in game+38h. On success it switches the interface to
// kInterfaceEngineMovie; when the player reports state 5 it tears the player
// down again and notifies the listener list at DAT_00E188B4.
inline constexpr std::string_view kEngineMovieExtension = ".ema";
inline constexpr std::size_t kEngineMoviePlayerSizeBytes = 0x74;
inline constexpr int kEngineMoviePlayerFailedState = 5; // [game+38h]+60h

struct SimulationGateState {
    // game+1EE0h. The gate calls 004CCE50 while it is set and clears it right
    // after (004E514C), so it is a one-shot request, not a mode.
    bool engine_movie_requested{false};
};

// ---------------------------------------------------------------------------
// The audio environment, the tail of 0068C1F0 (0068CB79..0068CC55)
// ---------------------------------------------------------------------------

// The three names 00A7B710 receives. "Air" is the pointer at 00CE9B1C; the
// other two are inline immediates.
inline constexpr std::string_view kAudioEnvironmentAir = "Air";
inline constexpr std::string_view kAudioEnvironmentUnderwater = "Underwater";
inline constexpr std::string_view kAudioEnvironmentCockpit = "Cockpit";

struct AudioEnvironmentInputs {
    // 0068CB79..0068CB9B: the cockpit interface at menu+6Ch is up and 00604F30
    // returns 1, or the interface at menu+84h is up and its +74h is 2.
    bool cockpit_interface_active{};
    // 0068CBD8..0068CC46. The camera transform is game+19FCh; +124h is its
    // world height and 0078CF20 returns the water height under +120h. The byte
    // flag is the device-capability path at 0068CC0A, which forces "Air" when
    // no device reports capability 8 or its float +100h is below 00CF1430.
    float camera_height{};
    float water_height{};
    bool device_allows_underwater{};
};

// Faithful to the native operand order: the result is "Air" only when the
// camera is strictly above the water surface **and** the device flag is clear;
// every other combination selects "Underwater".
std::string_view audio_environment_0068c1f0(const AudioEnvironmentInputs& inputs) noexcept;

// ---------------------------------------------------------------------------
// The sequence, 004E50B0..004E525E
// ---------------------------------------------------------------------------

// One method per native call site of the gate, in call order. There are no
// default implementations: nothing here stands in for unrecovered behaviour.
struct SimulationGateHost {
    virtual ~SimulationGateHost() = default;

    // 004E5131, 004C40A0, __thiscall, ECX = game, RET. Four calls, each with
    // the scaled delta at game+21F0h and each with its own ECX that the
    // decompiler dropped: 00875BB0 with ECX = game, 004C3CB0 with ECX = game
    // and no argument, [[game+19CCh]]->vtable[+0Ch], and 00447B80 with
    // ECX = game+30h read at 004C40D7.
    virtual void update_in_mission_subsystems() = 0;

    // 004E5147, 004CCE50, __thiscall. The call site loads ECX from
    // DAT_00E188A8 rather than reusing ESI, even though the two hold the same
    // game pointer.
    virtual void begin_engine_movie() = 0;

    // game+1FE4h, the network session pointer, re-read at 004E5153.
    virtual bool multiplayer_session() = 0;
    // The selected slot: game+[18CCh + game+18ECh * 4]. Returns false when the
    // pointer is null, in which case the counter is not read.
    virtual bool active_local_player_slot(std::int16_t& counter_10h) = 0;

    // 004C43C0, the rising-edge test. ECX = game is set at both sites and
    // ignored by the callee, which reads the input singleton itself.
    virtual bool input_action_pressed(int action) = 0;

    // game+634h, read again at 004E518D after the action test.
    virtual bool hud_hidden() = 0;
    // 004F7740.
    virtual int top_populated_level() = 0;
    // 0068A140 with ECX = DAT_00E198C4.
    virtual bool base_interface_active() = 0;
    // DAT_00E188A8+19C4h, the byte that decides whether the spectator flag is
    // consulted at all.
    virtual bool spectator_gate_enabled() = 0;
    // 006529E0 with ECX = [DAT_00E198C4+BCh].
    virtual bool spectator_flag() = 0;
    // 004BFE50.
    virtual bool multiplayer_spectator_active() = 0;

    // 00425D10 runs three times, once per field, at 004E51E9, 004E51F7 and
    // 004E5205. They are kept apart because the native code re-fetches the
    // object between them.
    virtual bool scene_console_open() = 0; // +25Ch, a byte
    virtual bool scene_modal_188h() = 0; // +188h, a dword
    virtual bool scene_modal_218h() = 0; // +218h, a dword

    // The pause branch, 004E5213..004E5240. The extra pair only runs while the
    // HUD is already hidden and the free-camera object reports active:
    // [DAT_00E198C4+A8h]+8h.
    virtual bool free_camera_active() = 0;
    virtual void build_pause_unit_list() = 0; // 0054E440, owned elsewhere
    virtual void toggle_pause_menu() = 0; // 004DB030, ECX = game

    // The interface-only branch, 004E5242..004E525C.
    virtual bool in_mission_interface_ready() = 0; // DAT_00E198C4 && +3Ch
    virtual void update_in_mission_interface() = 0; // 0068C1F0, ECX = DAT_00E198C4
    virtual void update_interface_only() = 0; // 004C40F0, ECX = game, owned elsewhere
};

// 004E50B0..004E525E and the 004E53B4 fallback, as one routine. Returns which
// branch ran. The profiler bracket around the block (004E509D / 004E54A2) and
// the one-shot "GGame::OnMove::game" label at 004E50C6 are frame-control work
// and are not modelled here.
SimulationGateBranch run_simulation_gate(
    SimulationGateState& state, SimulationGateHost& host, const SimulationGateEntry& entry);
}
