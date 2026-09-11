#pragma once
#include <cstddef>
#include <cstdint>

#include "bsp/game_frame_control.hpp"
#include "bsp/mission_scene_load.hpp"

// Mission state entry: the transition from the loaded scene (game+5D4h = 0Ch,
// left behind by BSP_Game_LoadMissionScene 004dfb70) into the running mission
// (game+5D4h = 0Dh). Two native routines make the transition:
//
//   004db920 BSP_Game_UpdateDeviceWaitScreen - the state 0Ch frame handler,
//            reached from BSP_Game_OnMove at 004e5046. It decides whether the
//            frame may enter, and both of its arms tail into 004da6c0.
//   004da6c0 BSP_Game_EnterMissionState - the entry itself. It is the only
//            writer of state 0Dh outside the request 12h arm of the drain.
//
// Every name below is a hypothesis, not a recovered symbol.
// docs/MISSION_STATE_ENTRY.md.

namespace bsp {

struct AudioSettings;

// ---------------------------------------------------------------------------
// State 0Ch, the device wait screen (004db920)
// ---------------------------------------------------------------------------

// 004db93c/004db94c. The handler splits on the local view mode game+1FE4h and
// on the byte the native listing addresses as [game+1EF0h + 29Ch] (EDI is
// loaded with LEA at 004db944, so the literal reading is session+29Ch; the
// same byte is read as game+218Ch in docs/FRONTEND_MANAGERS.md). The network
// player-count arm runs only when the mode is non-zero AND that byte is set;
// everything else takes the input-device arm.
bool mission_entry_uses_player_count_arm(
    std::int32_t local_view_mode, bool session_flag_29c) noexcept;

// The input-device arm, 004db955..004db9b9. The device table at 00f8bbf4 keeps
// its own previous-sample byte at +DDh, so the arm fires on a rising edge and
// the byte is stored back on every pass, edge or not.
struct DeviceWaitLatch {
    bool any_button_down{false}; // table+DDh, the previous sample
};

// Returns true on a rising edge; always updates the latch, which is what
// 004db977 does before the branch.
bool device_wait_button_edge(DeviceWaitLatch& latch, bool any_button_down) noexcept;

// 004db988/004db99a. Both busy bytes suppress the edge for this frame: the menu
// command screen singleton (00425d10) at +25Ch and the GUI root 00f8abe8 at
// +3E8h. The latch has already been updated, so the edge is consumed, not
// deferred.
bool device_wait_screens_idle(bool menu_screen_busy, bool gui_root_busy) noexcept;

// The player-count arm, 004db9be..004dba3a. It walks the eight local player
// slots at game+18CCh (two unrolled passes of four, stride 4) and counts the
// slots that are neither excluded (+9h) nor bound to a device (+0Eh). The slot
// pointers come from the global game object 00e188a8, not from ECX.
struct MissionEntryPlayerSlot {
    bool excluded{false}; // slot+9h, the same byte LocalPlayerSlot::inactive names
    bool device_bound{false}; // slot+0Eh, set to 1 for local play by 004dfb70 step 23
};

std::size_t count_unbound_player_slots(
    const MissionEntryPlayerSlot (&slots)[kLocalPlayerSlotCount]) noexcept;

// 004dba2f..004dba3a. The arm waits while any slot is still unbound, unless
// game+624h overrides it; game+624h is unidentified (docs/GAME_WORLD_OCEAN.md
// lists it as open), so it is carried as a raw field.
bool player_count_arm_may_enter(
    std::size_t unbound_slots, std::int32_t game_field_624) noexcept;

// ---------------------------------------------------------------------------
// The one-shot bytes 004da6c0 arms (game+1EE1h..+1EE5h)
// ---------------------------------------------------------------------------

// Five adjacent bytes in the game object, all initialised by the constructor
// 004ddb90 and all rewritten here. The names come from their consumers, which
// are listed per field.
struct MissionOneShots {
    // +1EE1h. Written by GGame::EndScene 004d7970 from its own argument and
    // read by the mission-teardown arm of the drain (004e45e8, 004e47b0,
    // 004e4918) and by the state 11h bookkeeping, which publishes
    // 00e198b0 = (byte == 0). Clear means the mission ended on its own terms.
    bool scene_ended_by_abort{false};
    // +1EE2h. The re-entry latch inside 004d7970: while it is set the end of
    // scene body (metrics, BSP_MissionPlayerRecords_Reset, 00920a20) is
    // skipped. 004d7970 sets it at 004d7a49 after the first pass.
    bool end_scene_body_done{false};
    // +1EE3h. A cached copy of "this session is networked", also written the
    // same way by 0076d030 and 00772610. The drain reads it at 004e45d2 and
    // 004e4717 to choose the front-end shell over the multiplayer menu, after
    // game+1FE4h itself has been torn down.
    bool session_was_networked{false};
    // +1EE4h. Session end notification, including normal EndScene. 0076d0e0
    // and 007728b0 raise it only for a zero message payload; 007727a0 and
    // 00772990 raise it unconditionally. Legacy name retained. The drain
    // reads it at 004e4932 and pushes multiplayer notice interface 4.
    bool session_dropped{false};
    // +1EE5h. Raised by 004d87b0 with the ingame.multi_notenoughplayer
    // warning, cleared by the session restart paths 0076fad0 and 00772610,
    // and read by the debrief 00920a20 at 00920aa2 and 00920b31.
    bool not_enough_players{false};
};

// 004da6e6..004da70f. Four bytes are cleared unconditionally and +1EE3h takes
// game+1FE4h != 0. The native order is +1EE2h, +1EE4h, +1EE1h, +1EE5h, then
// +1EE3h; nothing observes the order, so this is a whole-struct projection.
MissionOneShots arm_mission_one_shots(std::int32_t local_view_mode) noexcept;

// ---------------------------------------------------------------------------
// The fields 004da6c0 writes
// ---------------------------------------------------------------------------

// The subset of the game object this routine touches. Everything else it
// reaches goes through the host below.
struct MissionStateEntryState {
    std::uint32_t game_state{kGameStateSceneReady}; // +5D4h, 0Ch on entry
    float scaled_frame_delta{0.0f}; // +21F0h, the simulation clock of docs/GAME_FRAME_CONTROL.md
    std::int32_t local_view_mode{0}; // +1FE4h, zero for single player
    std::size_t local_slot_index{0}; // +18ECh, the index into the slot array at +18CCh
    MissionOneShots one_shots{}; // +1EE1h..+1EE5h
    bool hud_suppressed{false}; // +608h, the byte docs/GAME_SIMULATION_GATE.md reads with game+1FE4h
};

// The mask 00a7a440 substitutes for its stack argument before tail-jumping to
// 00a7a3f0, which marks every bus whose class bit is in the mask dirty.
inline constexpr std::uint32_t kAudioEnvironmentBusMask = 0xFFFF; // 00a7a44b

// 004cd0f0 arguments at 004da763..004da769, pushed in reverse: hide = 0,
// allow_simulation = 0, and 1 for the third byte. Because 004cd0f0 computes
// game+635h as -(game+634h != 0) & allow_simulation, clearing the hide flag
// clears both cinematic flags and opens the simulation gate.
inline constexpr bool kMissionEntryCinematicHide = false;
inline constexpr bool kMissionEntryCinematicAllowSimulation = false;
inline constexpr bool kMissionEntryCinematicThirdArgument = true;

// The scope literal at 00ce7ab8 pushed to 004254b0, whose body is a no-op in
// this build.
inline constexpr const char* kMissionEntryScopeLabel = "GGame::SceneInit()";

// The 16-bit value 004da71e stores at [game+18CCh + game+18ECh*4] + 10h. The
// load 004dfb70 leaves FFFDh in the same field for every slot (step 23 of
// docs/MISSION_SCENE_LOAD.md), so this promotes the one slot the local player
// drives. The field's meaning is not established.
inline constexpr std::uint16_t kLocalSlotReadyValue = 1;

// ---------------------------------------------------------------------------
// Integration boundary
// ---------------------------------------------------------------------------

// One method per native call site of 004da6c0, in body order. Nothing here has
// a default implementation; none of it stands in for unrecovered behaviour.
struct MissionStateEntryHost {
    virtual ~MissionStateEntryHost() = default;
    // 004da6c9, 004254b0 with the scope literal. The body is trivial in this
    // build, so this exists only to keep the call site visible.
    virtual void enter_scope(const char* label) = 0;
    // 004da6df, 00447060 with ECX = game+30h, the same sub-object 00447b80
    // takes in docs/GAME_SIMULATION_GATE.md. It releases every element of the
    // 20h-stride vector at +14h/+18h through the dynamics owner
    // [00e188a8]+18h, empties it, then compacts the second range at +4h/+8h.
    virtual void release_deferred_dynamics() = 0;
    // 004da71e, only when the session is not networked.
    virtual void mark_local_slot_ready(std::size_t slot_index, std::uint16_t value) = 0;
    // 004da734, 00a7a440 with ECX = the sound manager 00f8bbd8.
    virtual void set_audio_environment_level(float level, std::uint32_t bus_mask) = 0;
    // 004da746, 004c9ca0 with 0. The same interface switch 004dfb70 calls with
    // 1 at 004e1873; it loads interface/textures/allbutingame.ats.
    virtual void apply_in_game_interface(bool loading) = 0;
    // 004da755, 004d87b0, only when the session is networked. It can end the
    // scene through 004d7970 and so change game+5D4h.
    virtual void check_multiplayer_player_count() = 0;
    // 004da75a re-reads game+5D4h after that call.
    virtual std::uint32_t game_state() = 0;
    // 004da769, 004cd0f0.
    virtual void set_cinematic_mode(bool hide, bool allow_simulation, bool third) = 0;
};

// 004da6c0 BSP_Game_EnterMissionState. Native __fastcall void(GGame*), ECX is
// the only input, RET with no immediate. Returns true when the routine ran to
// the end, false when the multiplayer check moved the state away from 0Dh and
// the native code took the early return at 004da761 - in that case the
// cinematic reset and the +608h store do not happen. Audio settings project
// the mutable static settings object at 00f88980; its master_20 is read at
// the native 004da724 point, after the earlier host calls, not at entry.
bool run_mission_state_entry(MissionStateEntryState& state,
    const AudioSettings& audio_settings, MissionStateEntryHost& host);

// One method per native call site of 004db920 that is not already a predicate
// above.
struct MissionDeviceWaitHost {
    virtual ~MissionDeviceWaitHost() = default;
    // 004db95d, 00a91020 with ECX = the device table 00f8bbf4.
    virtual bool any_dynamic_device_button_down() = 0;
    virtual bool menu_command_screen_busy() = 0; // 00425d10, +25Ch
    virtual bool gui_root_busy() = 0; // 00f8abe8, +3E8h
    // 004db9ad..004db9b4. The FLDZ at 004db9a7 pushes the argument of
    // 00a92c40, not of the singleton getter 004bec00; a zero delta refreshes
    // device state without advancing hold timers.
    virtual void update_input_manager(float delta_seconds) = 0;
    // 004dba42..004dba8f. Builds the tag 0Ch event through 0075b430, dispatches
    // it into the session at game+1EF0h through 0076a9f0, then runs 007848f0 on
    // session+188h, or on session+18Ch when +188h is null.
    virtual void dispatch_session_event(int event_tag) = 0;
    // 004dba96, ECX = game.
    virtual void enter_mission_state() = 0;
};

// The tag 004db920 passes to 0075b430. 004dfb70 step 23 builds the same tag.
inline constexpr int kMissionEntrySessionEventTag = 0x0C;

// The delta 004db920 hands to the input manager.
inline constexpr float kDeviceWaitInputDelta = 0.0f;

// Inputs the state 0Ch handler reads that are not part of MissionStateEntryState.
struct MissionDeviceWaitInputs {
    std::int32_t local_view_mode{0}; // game+1FE4h
    bool session_flag_29c{false}; // [game+1EF0h + 29Ch]
    std::int32_t game_field_624{0}; // game+624h, unidentified
    std::size_t unbound_player_slots{0}; // the count of count_unbound_player_slots
};

// 004db920 BSP_Game_UpdateDeviceWaitScreen. Native __thiscall void(GGame*),
// ECX only, RET with no immediate, wrapped in an SEH scope (handler 00c66908)
// that is not modelled. Returns true when the frame entered the mission state.
bool run_mission_device_wait(const MissionDeviceWaitInputs& inputs, DeviceWaitLatch& latch,
    MissionDeviceWaitHost& host);

}
