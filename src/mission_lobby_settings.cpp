// Mission lobby settings and the mission Lua teardown.
//
// Evidence: docs/MISSION_LOBBY_SETTINGS.md, docs/MISSION_LUA_TEARDOWN.md and
// reports/mission_lobby_settings.json. Ghidra was read-only for this packet.
// These are rules over an injected view, not a binary-compatible rebuild: the
// native works on Lua objects, a std::map registry and raw globals, all of
// which stay behind the two host interfaces.
#include "bsp/mission_lobby_settings.hpp"

namespace bsp {
namespace {

// 005D5500's per-mode tables. Each pair is {slot, menu row}; a slot that is
// absent from its mode's table has no row, which is what -1 reports.
struct MenuRow {
    int slot;
    int row;
};

// Modes 0..3, the four Island Capture map sizes (005D5528 switch).
constexpr MenuRow kIslandCaptureRows[] = {
    {3, 1}, {5, 2}, {7, 3}, {8, 4}, {10, 5}, {11, 6}, {12, 7}, {13, 8},
};

// Mode 4. Publishes the non-IC halves of the paired settings.
constexpr MenuRow kModeFourRows[] = {
    {4, 1}, {6, 2}, {9, 3}, {11, 4}, {12, 5}, {13, 6},
};

// Modes 5 and 6, which share one arm of the switch.
constexpr MenuRow kModeFiveSixRows[] = {
    {11, 1}, {12, 2}, {13, 3},
};

// Mode 7.
constexpr MenuRow kModeSevenRows[] = {
    {6, 1}, {8, 2}, {11, 3}, {12, 4}, {13, 5},
};

template <std::size_t N>
int row_in(const MenuRow (&rows)[N], int slot) noexcept
{
    for (const MenuRow& row : rows) {
        if (row.slot == slot) return row.row;
    }
    return -1;
}

// 005E2FE2..005E3017: each flag is "the stored option index is 0", which is the
// enable / on option in all three of MultiLobbySettings[10], [11] and [12].
bool option_is_enabled(const LobbySettingsGameState& state, int slot) noexcept
{
    if (slot < 0 || static_cast<std::size_t>(slot) >= kLobbySettingsStoredOptionCount) return false;
    return state.stored_option[slot] == kLobbySettingsOptionEnabled;
}

} // namespace

int lobby_settings_menu_row(int effective_game_mode, int slot) noexcept
{
    // 005D5504: slot 1 answers before the mode is read at all.
    if (slot == kLobbySettingsSlotMap) return 0;

    // 005D5520 compares the mode unsigned, so a negative mode falls through to
    // the -1 tail rather than into the Island Capture arm.
    const unsigned mode = static_cast<unsigned>(effective_game_mode);
    if (mode < static_cast<unsigned>(kLobbySettingsIslandCaptureModeBound)) {
        return row_in(kIslandCaptureRows, slot);
    }
    if (mode == 4u) return row_in(kModeFourRows, slot);
    if (mode == 5u || mode == 6u) return row_in(kModeFiveSixRows, slot);
    if (mode == 7u) return row_in(kModeSevenRows, slot);
    return -1;
}

bool lobby_settings_slot_is_published(int effective_game_mode, int slot) noexcept
{
    // 005E30DC and 005E30E3: PlayerCount and GameMode skip the row test.
    if (slot == kLobbySettingsSlotPlayerCount || slot == kLobbySettingsSlotGameMode) return true;
    return lobby_settings_menu_row(effective_game_mode, slot) != -1;
}

LobbySettingsValueKind lobby_settings_value_kind(LobbySettingsOptionSource& options,
                                                 const LobbySettingsGameState& state, int slot)
{
    if (slot < 0 || static_cast<std::size_t>(slot) >= kLobbySettingsStoredOptionCount) {
        return LobbySettingsValueKind::Nil;
    }
    // Slot 0Dh has a null name pointer and is skipped by number at 005E30C7.
    if (lobby_settings_field_name(static_cast<std::size_t>(slot)) == nullptr) {
        return LobbySettingsValueKind::Nil;
    }
    if (!lobby_settings_slot_is_published(state.effective_game_mode, slot)) {
        return LobbySettingsValueKind::Nil;
    }
    // 005E314C: the probe is option 0 of this slot in the integer map.
    return options.option_number(slot, 0) != 0 ? LobbySettingsValueKind::Number
                                               : LobbySettingsValueKind::String;
}

int lobby_settings_number_value(LobbySettingsOptionSource& options,
                                const LobbySettingsGameState& state, int slot)
{
    if (slot < 0 || static_cast<std::size_t>(slot) >= kLobbySettingsStoredOptionCount) return 0;
    return options.option_number(slot, state.stored_option[slot]);
}

bool lobby_settings_publishes_table(const LobbySettingsGameState& state) noexcept
{
    // 005E2F99 / 005E2FA2: the early-out needs both to be clear.
    return state.network_session || state.game_mode_forced;
}

void lobby_settings_apply_mode_writeback(LobbySettingsGameState& state) noexcept
{
    // 005E2FA9 and 005E2FCC both reach 005E2FCE only when the forced byte is set.
    if (!state.game_mode_forced) return;
    state.stored_option[kLobbySettingsSlotGameMode] = state.effective_game_mode;
    if (state.effective_game_mode < kLobbySettingsIslandCaptureModeBound) {
        // 005E2FDD: the four Island Capture modes are the four map sizes.
        state.stored_option[kLobbySettingsSlotMapSize] = state.effective_game_mode;
    }
}

LobbySettingsModeFlags lobby_settings_mode_flags(const LobbySettingsGameState& state) noexcept
{
    LobbySettingsModeFlags flags;
    if (!lobby_settings_publishes_table(state)) {
        // 005E2FAB: single player forces powerups on, reload payload off, map on.
        flags.powerups_enabled = true;
        flags.reload_payload_on = false;
        flags.map_enabled = true;
        return flags;
    }
    flags.powerups_enabled = option_is_enabled(state, kLobbySettingsSlotEnablePowerups);
    flags.reload_payload_on = option_is_enabled(state, kLobbySettingsSlotReloadPayload);
    flags.map_enabled = option_is_enabled(state, kLobbySettingsSlotEnableMap);
    return flags;
}

float lobby_settings_command_points(LobbySettingsOptionSource& options,
                                    const LobbySettingsGameState& state)
{
    if (state.effective_game_mode >= kLobbySettingsIslandCaptureModeBound) {
        return kLobbySettingsDefaultCommandPoints;
    }
    // 005E325A: the ResourceLimit payload, doubled, converted with CVTSI2SS.
    const int payload = lobby_settings_number_value(options, state, kLobbySettingsSlotResourceLimit);
    return static_cast<float>(payload * 2);
}

void lobby_settings_sync_005e2f00(LobbySettingsSyncHost& host, LobbySettingsOptionSource& options,
                                  LobbySettingsGameState& state)
{
    // 005E2F59, before the gate: the global is dropped whatever happens next.
    host.set_global_nil(kLobbySettingsTable);

    if (!lobby_settings_publishes_table(state)) {
        // 005E2FAB then the 005E2FC0 jump straight to the command-point tail.
        host.publish_mode_flags(lobby_settings_mode_flags(state));
        host.publish_command_points(lobby_settings_command_points(options, state));
        return;
    }

    lobby_settings_apply_mode_writeback(state);                 // 005E2FCE
    host.publish_mode_flags(lobby_settings_mode_flags(state));  // 005E2FE2
    host.set_global_new_table(kLobbySettingsTable);             // 005E304B
    host.open_global_table(kLobbySettingsTable);                // 005E30B1

    for (std::size_t slot = 0; slot < kLobbySettingsSlotCount; ++slot) {
        const char* field = lobby_settings_field_name(slot);
        if (field == nullptr) continue; // 005E30C7 skips 0Dh by number
        const int index = static_cast<int>(slot);
        switch (lobby_settings_value_kind(options, state, index)) {
        case LobbySettingsValueKind::Nil:
            host.table_set_nil(field); // 005E3104
            break;
        case LobbySettingsValueKind::Number:
            host.table_set_number(field, lobby_settings_number_value(options, state, index));
            break;
        case LobbySettingsValueKind::String:
            host.table_set_string(field, host.option_label(index)); // 005E31BD, 005E31D4
            break;
        }
    }

    host.close_table();                                          // 005E3214
    host.publish_command_points(lobby_settings_command_points(options, state)); // 005E326F
}

void mission_lua_teardown_004dc5c0(MissionLuaTeardownHost& host, MissionLuaTeardownState& state)
{
    // 004DC60C. The inline owner at game+1A0Ch borrows the machine's state, so
    // its owns byte is clear and 00B65E80 only drops the pointer.
    host.close_borrowed_state_owner();
    state.borrowed_owner_cleared = true;

    // 004DC617. Unconditional in the native: 008844F0 is called with the host
    // pointer before 004DC622 tests it for null. The machine's own vtable slot
    // 0 (006B87C0) closes the state when the owns byte at machine+8h is set.
    host.release_lua_machine();
    state.machine_released = true;
    state.state_closed = state.machine_owns_state;

    // 004DC62C, guarded. The host destructor clears the deferred-call list at
    // host+8h (00887D90) and frees the sentinel node, then the host itself.
    if (state.host_present) {
        host.destroy_host();
        state.queued_calls_cleared = true;
        state.host_destroyed = true;
    }
}

bool mission_lua_self_table_needs_rebuild(bool self_table_is_nil) noexcept
{
    // 004E0225 returns "is nil" and 004E0249 skips the rebuild when it is false.
    return self_table_is_nil;
}

} // namespace bsp
