#pragma once

// Packet cc13-mp3 (MP3c): the multiplayer lobby's option state for a --skirmish run under
// kSkirmishSessionCc13Bound. Header-only. Descriptive names are hypotheses.
//
// - 008D2F50 fills the 00576B10 registry from Scripts\datatables\MultiGlobals.lua,
//   table MultiLobbySettings, and appends one slot-1 option per numbered map record.
// - 005E3290 (the lobby screen constructor) walks each slot's options against an
//   availability map and leaves the default option index in 00E1955C[slot].
// - 005E51D0 (lobby enter) writes the mode into slots 2/3 and the content mask into
//   game+2060h/+2064h; 005DA530 (start) re-applies the mask and 005D62B0 copies the
//   fourteen indices into game+2020h.
// - 005E2F00 publishes them to Lua each scene load and sets 00E0C978 (EnablePowerups).

#include <array>
#include <cstddef>
#include <cstdint>
#include <map>

#include "bsp/mission_lobby_settings.hpp"

namespace bsp {

// 00576B10's C4h-byte registry, the members 005E3290 and 005E2F00 read.
struct LobbyOptionRegistry {
    bool loaded{false};
    // +1Ch, std::map<int, std::map<int, int>>: the integer payload of (slot, option).
    // 008D2F50 enters every (slot, option) it labels into +10h, +1Ch and +34h together
    // (008D378A / 008D387F / 008D3B16), so this map's key set is also +10h's, whose
    // inner size (+8h) is the option count 005E3290 reads.
    std::map<std::int32_t, std::map<std::int32_t, std::int32_t>> numbers_1c;
    // +28h, std::map<int, std::map<int, byte>>: 1 unless the option has a MenuDIS key.
    std::map<std::int32_t, std::map<std::int32_t, std::uint8_t>> enabled_28;
    // +40h..+8Ch, zeroed by 008D2F50 before the walk; +40h + 4*slot is the slot's
    // "Default" option index (the last one written in lua_next order).
    // LABELLED: slot 5 (ResourceLimit) has two Default options, 0 and 4, and its value 4
    // relies on the host's Lua 5.1 node order matching the image's. lua_Number is a
    // double (00A67797 FLD QWORD); the eight [k] = fields of a constructor make eight hash
    // nodes (mod 7) and integer keys hash by their double bits, so lua_next walks
    // 5,1,7,2,0,3,4,6 (5.1.1) or 0,2,7,3,1,4,5,6 (5.1.2 and later). Option 4 is walked
    // after option 0 either way; with another node layout slot 5 could read 0.
    std::array<std::int32_t, 20> defaults_40{};

    // 00575E80(+10h, &slot) then +8h: the inner map's size (operator[], so an absent
    // slot is an empty map).
    std::uint32_t option_count(std::int32_t slot) const noexcept {
        const auto found = numbers_1c.find(slot);
        return found == numbers_1c.end() ? 0u : static_cast<std::uint32_t>(found->second.size());
    }
    // 00575FD0 then 00573410 (operator[], an absent key reads 0).
    std::int32_t option_number(std::int32_t slot, std::int32_t option) const noexcept {
        const auto row = numbers_1c.find(slot);
        if (row == numbers_1c.end()) return 0;
        const auto cell = row->second.find(option);
        return cell == row->second.end() ? 0 : cell->second;
    }
};

// 005C6850 then 005C49C0 on an availability map (registry+28h, or a map record's +428h
// when its +424h is set). operator[], so an absent (slot, option) reads 0.
// The image's map reads (00575E80, 005C6850, 005C49C0) are std::map::operator[] calls:
// they insert an absent (slot, option) cell as 0 into registry+10h and the availability
// map. This C++ uses const find, which reads the same 0 without inserting. The two would
// diverge only if a later reader iterated those maps or read their sizes (+8h).
template <typename Map>
std::uint8_t lobby_option_flag(const Map& map, std::int32_t slot, std::int32_t option) noexcept {
    const auto row = map.find(slot);
    if (row == map.end()) return 0;
    const auto cell = row->second.find(option);
    return cell == row->second.end() ? std::uint8_t{0} : static_cast<std::uint8_t>(cell->second);
}

// 005E3290's walk, 005E3373..005E3548 (CMP slot, 0Eh). For each slot 0..13:
//   slot 8: option values count up from 0 while value < count (unsigned); each value
//     whose flag is 1 is stored to 00E1957C (slot 8, 005E3437), and the walk stops at
//     registry+60h (005E3442);
//   others: values count down from count - 1 while >= 0 (signed); each value whose
//     flag is 1 is stored to 00E1955C[slot] (005E351F), and the walk stops at
//     registry+40h+4*slot (005E352B).
// A slot with no enabled option keeps its old value.
template <typename Map>
void lobby_option_defaults_005e3290(const LobbyOptionRegistry& registry, const Map& availability,
                                    std::array<std::int32_t, kLobbySettingsStoredOptionCount>& stored) {
    for (std::int32_t key = 0; key < static_cast<std::int32_t>(kLobbySettingsStoredOptionCount); ++key) {
        const std::uint32_t count = registry.option_count(key);
        const std::int32_t stop = registry.defaults_40[static_cast<std::size_t>(key)];
        if (key == 8) {
            for (std::uint32_t value = 0; value < count; ++value) {
                if (lobby_option_flag(availability, key, static_cast<std::int32_t>(value)) == 1) {
                    stored[8] = static_cast<std::int32_t>(value);
                    if (static_cast<std::uint32_t>(stop) == value) break;
                }
            }
        } else {
            for (std::int32_t value = static_cast<std::int32_t>(count) - 1; value >= 0; --value) {
                if (lobby_option_flag(availability, key, value) == 1) {
                    stored[static_cast<std::size_t>(key)] = value;
                    if (stop == value) break;
                }
            }
        }
    }
}

// The lobby state the image holds in globals and game fields. Only a --skirmish run under
// kSkirmishSessionCc13Bound writes it, apart from 00E0C978 (below).
struct LobbySessionState {
    LobbyOptionRegistry registry;
    // 00E1955C..00E19590, BSS: zero until a lobby writes it.
    std::array<std::int32_t, kLobbySettingsStoredOptionCount> stored_options_00e1955c{};
    std::int32_t selected_map_00e19594{0};  // 005E57DD: game+60Ch
    // game+2020h..+2054h, 005D62B0's copy of the fourteen indices.
    std::array<std::int32_t, kLobbySettingsStoredOptionCount> game_settings_2020{};
    std::uint64_t content_mask_2060{0};     // game+2060h (low) / +2064h (high)
    // 00E0C978, written by 005E2F00 on every scene load (005E2FAB forces 1 in single
    // player). LABELLED: its value before the first load is not read; true here, which
    // is what every campaign load then writes.
    bool powerups_enabled_00e0c978{true};
};

inline LobbySessionState& lobby_session_state() noexcept {
    static LobbySessionState state;
    return state;
}

// 005D9C50: the map record's +10h contentID text after its last '_', through atol, as
// bit (n - 1) of a qword (__allshl on CL; 64 and above give 0). An empty name gives 0.
inline std::uint64_t lobby_content_mask_005d9c50(std::int32_t number) noexcept {
    const std::uint32_t shift = static_cast<std::uint32_t>(number - 1) & 0xFFu;
    return shift >= 64 ? 0 : (std::uint64_t{1} << shift);
}

}  // namespace bsp
