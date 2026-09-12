// Mission lobby settings: the values behind the `LobbySettings` global table
// (005E2F00) and the mission Lua teardown (004DC5C0 and the host destructor).
//
// docs/MISSION_LOBBY_SETTINGS.md and docs/MISSION_LUA_TEARDOWN.md carry the
// evidence. The field names, the slot count and the game-owned-slot rule live in
// bsp/mission_lua_machine.hpp and are not restated here; this header adds the
// value derivation, which docs/MISSION_LUA_MACHINE.md left open because the
// values come from game state the script probe does not own.
//
// Every descriptive name is a hypothesis, not a recovered symbol. Nothing here
// is binary-compatible with the native routine: the rules are pure functions
// over an injected view of the game state, and every native call site is one
// virtual method on a host.
#pragma once
#include <cstddef>

#include "bsp/mission_lua_machine.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The stored option indices, 00E1955C.
//
// Fourteen ints, one per name slot, mirroring the array at 00E08908. Each is an
// index into that slot's option list in MultiLobbySettings, not a value.
// 005D49E0 copies all fourteen from a source block, 0076C2C0 copies them into
// the 88h-byte network state record at +30h, and 005E3158 reads
// [slot*4 + 00E1955C] here.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kLobbySettingsStoredOptionCount = 14;

// Slot indices this header needs by name. The full name table is
// kLobbySettingsFields in bsp/mission_lua_machine.hpp.
inline constexpr int kLobbySettingsSlotPlayerCount = 0;
inline constexpr int kLobbySettingsSlotMap = 1;
inline constexpr int kLobbySettingsSlotGameMode = 2;
inline constexpr int kLobbySettingsSlotMapSize = 3;
inline constexpr int kLobbySettingsSlotResourceLimit = 5;
inline constexpr int kLobbySettingsSlotEnablePowerups = 10;
inline constexpr int kLobbySettingsSlotEnableMap = 11;
inline constexpr int kLobbySettingsSlotReloadPayload = 12;

// Option index 0 of the enable/disable and on/off slots, which is the "on"
// option in all three (MultiLobbySettings[10], [11] and [12]).
inline constexpr int kLobbySettingsOptionEnabled = 0;

// The mode the three Island Capture map sizes share. 005E2FDD copies the mode
// into the MapSize slot exactly when it is below this bound, because modes 0..3
// are the four map sizes of one game mode.
inline constexpr int kLobbySettingsIslandCaptureModeBound = 4;

// 00CE396C, stored in 00E0CFB4 when the effective mode is 4 or above.
inline constexpr float kLobbySettingsDefaultCommandPoints = 2400.0f;

// ---------------------------------------------------------------------------
// The injected game-state view. One field per native global or game-object
// offset 005E2F00 reads; nothing here is a native layout.
// ---------------------------------------------------------------------------
struct LobbySettingsGameState {
    bool network_session{false};   // game+1FE4h != 0
    bool game_mode_forced{false};  // game+61Ch != 0
    int effective_game_mode{0};    // 004BCA50 BSP_Game_GetEffectiveGameMode
    int stored_option[kLobbySettingsStoredOptionCount]{}; // 00E1955C
};

// The option registry loaded from Scripts/datatables/MultiGlobals.lua, table
// MultiLobbySettings, by 008D2F50. Both maps are std::map::operator[] in the
// native, so a missing key reads as zero / null rather than failing.
struct LobbySettingsOptionSource {
    virtual ~LobbySettingsOptionSource() = default;
    // Integer payload of (slot, option), registry+1Ch. Zero when the option's
    // datatable key is a string or the entry is absent.
    virtual int option_number(int slot, int option) = 0;
};

enum class LobbySettingsValueKind {
    Nil,    // 005E3104
    Number, // 005E31A0
    String, // 005E31D4
};

// ---------------------------------------------------------------------------
// The rules.
// ---------------------------------------------------------------------------

// 005D5500. The lobby menu row a slot occupies in the given effective mode, or
// -1 when the mode does not offer it. Slot 1 is row 0 in every mode.
int lobby_settings_menu_row(int effective_game_mode, int slot) noexcept;

// 005E30D7..005E30E3. Slots 0 and 2 are always published; any other slot whose
// menu row is -1 is written as nil.
bool lobby_settings_slot_is_published(int effective_game_mode, int slot) noexcept;

// 005E3140/005E3147 then 005E314C: the routine probes option 0 of the slot in
// the integer map. A non-zero payload means the slot's options carry numbers,
// zero means they carry labels. Slot 0Dh and any slot past the table are Nil.
LobbySettingsValueKind lobby_settings_value_kind(LobbySettingsOptionSource& options,
                                                 const LobbySettingsGameState& state,
                                                 int slot);

// 005E31A0. Only meaningful when the kind is Number: the payload of the stored
// option index.
int lobby_settings_number_value(LobbySettingsOptionSource& options,
                                const LobbySettingsGameState& state, int slot);

// 005E2F93..005E2FCC. False for the single-player early-out, which leaves the
// global nil and skips the whole table.
bool lobby_settings_publishes_table(const LobbySettingsGameState& state) noexcept;

// 005E2FCE..005E2FDD. Applies the forced-mode write-back to the GameMode slot
// and, below the Island Capture bound, to the MapSize slot. A no-op unless the
// mode is forced.
void lobby_settings_apply_mode_writeback(LobbySettingsGameState& state) noexcept;

// 005E2FAB and 005E2FE2..005E3017. Each flag is the "option index 0 is
// selected" test, and the single-player branch forces (1, 0, 1).
struct LobbySettingsModeFlags {
    bool powerups_enabled{true}; // 00E0C978
    bool reload_payload_on{false}; // 00E17BF2
    bool map_enabled{true};      // 00E08880
};
LobbySettingsModeFlags lobby_settings_mode_flags(const LobbySettingsGameState& state) noexcept;

// 005E321F..005E326F, which runs on both paths. Below the Island Capture bound
// the ResourceLimit payload is doubled; otherwise the constant.
float lobby_settings_command_points(LobbySettingsOptionSource& options,
                                    const LobbySettingsGameState& state);

// ---------------------------------------------------------------------------
// The sync, one virtual per native call site inside 005E2F00.
// ---------------------------------------------------------------------------
struct LobbySettingsSyncHost {
    virtual ~LobbySettingsSyncHost() = default;
    // 005E2F59, 00B67350 on the globals object.
    virtual void set_global_nil(const char* name) = 0;
    // 005E304B, 00B67580 on the globals object.
    virtual void set_global_new_table(const char* name) = 0;
    // 005E30B1, 00B67800; the table object every field write below targets.
    virtual void open_global_table(const char* name) = 0;
    // 005E3104, 00B67350 on the table object.
    virtual void table_set_nil(const char* field) = 0;
    // 005E31A0, 00B67460: lua_pushlstring then lua_pushnumber then lua_settable.
    virtual void table_set_number(const char* field, int value) = 0;
    // 005E31D4, 00B67630, with the label 005E2320 built.
    virtual void table_set_string(const char* field, const char* value) = 0;
    // 005E3214, 00B67700 on the table object.
    virtual void close_table() = 0;
    // 005E31BD, 005E2320: the display label for a slot. Needs the mission record
    // at 00E19594, the player-count bytes at game+2017h/+2018h and the label map,
    // none of which are in the state view, so it stays a host call.
    virtual const char* option_label(int slot) = 0;
    // 005E2FAB / 005E2FE2..005E3017, the three flag bytes.
    virtual void publish_mode_flags(const LobbySettingsModeFlags& flags) = 0;
    // 005E326F, the float at 00E0CFB4.
    virtual void publish_command_points(float value) = 0;
};

// 005E2F00 in full, in native order. Mutates the state's GameMode and MapSize
// slots through the write-back above, exactly as the native mutates 00E19564
// and 00E19568.
void lobby_settings_sync_005e2f00(LobbySettingsSyncHost& host,
                                  LobbySettingsOptionSource& options,
                                  LobbySettingsGameState& state);

// ---------------------------------------------------------------------------
// The mission Lua teardown, 004DC5C0 steps 7 to 9.
//
// The release order is: the borrowed state pointer at game+1A0Ch, then the
// LuaMachine at host+4h (which is where lua_close runs), then the host, whose
// destructor clears the queued named calls and frees the list sentinel. The
// binding globals, DoFile, the thisTable self tables and LobbySettings are
// values inside that one lua_State and are never released individually.
// ---------------------------------------------------------------------------
struct MissionLuaTeardownHost {
    virtual ~MissionLuaTeardownHost() = default;
    // 004DC60C, 00B65E80 on game+1A0Ch. Closes nothing: the inline owner
    // borrows the state, so this only clears its pointer.
    virtual void close_borrowed_state_owner() = 0;
    // 004DC617, 008844F0 on the host. Deletes the machine at host+4h through
    // its vtable slot 0 (006B87C0), which calls lua_close when the machine's
    // owns byte is set, and nulls host+4h. Not null-guarded in the native.
    virtual void release_lua_machine() = 0;
    // 004DC62C, the host vtable slot 0 (00888380 -> 00888300) with flag 1:
    // clears the deferred-call list at host+8h (00887D90, destroying each
    // queued named call), frees the sentinel node at host+0Ch, frees the host.
    virtual void destroy_host() = 0;
};

// What the teardown observes and leaves behind. `host_present` is the
// 004DC622 null test; `state_closed` records that lua_close ran.
struct MissionLuaTeardownState {
    bool host_present{true};        // game+1A08h != 0
    bool machine_owns_state{true};  // machine+8h, set to 1 at 006B87B0
    bool borrowed_owner_cleared{false};
    bool machine_released{false};
    bool state_closed{false};
    bool queued_calls_cleared{false};
    bool host_destroyed{false};
};

// 004DC5C0 steps 7 to 9, in order.
void mission_lua_teardown_004dc5c0(MissionLuaTeardownHost& host, MissionLuaTeardownState& state);

// ---------------------------------------------------------------------------
// What a scene reload leaves alone.
//
// Nothing on the 004DFB70 path deletes the host, the machine or the state:
// 008844F0's only caller is 004DC617, and the deferred-call clear 00887D90 is
// reached only from the host destructor and its unwind funclet. The per-load
// reset is two Lua globals, modelled by MissionSceneLoadHost in
// bsp/mission_scene_load.hpp; only its gate is restated here, because the
// native keeps an existing self table rather than rebuilding it.
// ---------------------------------------------------------------------------

// 004E0225 then 004E0249: 00B65FB0 answers "is nil", and JZ skips the rebuild
// when the answer is false. So the self-table root is created only when the
// global is nil, and survives every later scene load.
bool mission_lua_self_table_needs_rebuild(bool self_table_is_nil) noexcept;

// 004E0321 is unconditional: the recon global is set to nil on every load,
// before and after the rebuild gate alike.
inline constexpr bool kMissionLuaReconClearedEveryLoad = true;

} // namespace bsp
