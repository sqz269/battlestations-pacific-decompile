#pragma once

#include <cstdint>
#include <optional>
#include <string>

#include "bsp/game_settings.hpp"

struct lua_State;

namespace bsp {

// Mission checkpoints: the Lua global BSP_Chk_Save (00CE4124) in the mission
// state (game+1A0Ch), keyed by mission id (game+2198h), each entry holding
// __difficulty, __unlockFrom, __unlockTo, __unlockName, __playTime and a
// copy of the table the script passed to MW_SaveCheckpoint (008ACB70); and
// its text-archive image written to the game storage file (kind 2) named at
// game+684h. Host interfaces over stock Lua 5.1 states; not ABI compatible.
// The read side (004374F0) is in include/bsp/profile_manager.hpp.
// Evidence: docs/CC13_PROFILE_SAVE.md.

// The four record fields 00438020 fills before writing (00436690 defaults
// 0, -1, -1, ""). Native: __difficulty = [game+6ACh]; __unlockFrom /
// __unlockTo = index and value of the first nonzero dword in the vehicle-class
// registry array (00437F50, +10h, count at +8); __unlockName = the localized
// name of that class (+54h key through 00A9FAD0, "globals.unknown" when the
// class is missing), narrowed from UTF-16 by 00436630.
struct CheckpointRecordFields {
    std::int32_t difficulty{0};
    std::int32_t unlock_from{-1};
    std::int32_t unlock_to{-1};
    std::string unlock_name;
};

// Address: 00435D30. Original ABI: __thiscall (ECX only forwarded),
// [ESP+4] = writer*, [ESP+8] = key LuaObject*, [ESP+0Ch] = value LuaObject*,
// RET 0Ch. The key becomes an integer variant for a number (float32
// truncation), a string variant for a string, and integer 0 otherwise. A
// boolean is written as type 3, a number as type 1 when its float32 equals its
// floor and as type 2 otherwise, a string as type 0, a table as a section of
// its slots in lua_next order; other values are skipped.
void write_lua_slot_archive_00435d30(lua_State* L, int key, int value, SettingsWriter& writer);

// Address: 00436730. Original ABI: __thiscall (ECX forwarded), [ESP+4] =
// destination table LuaObject*, [ESP+8] = key, [ESP+0Ch] = value, RET 0Ch.
// Copies booleans, numbers (integer setter when float32 equals its floor,
// else float32), strings and tables (child created and assigned before it is
// filled) from `source` into the table at `table` in `destination`. String
// keys keep their C-string prefix; any other key goes through float32
// truncation. Same rules as 00436C50; tables are copied without alias
// preservation and a cycle recurses without bound natively (the host throws).
// Under kWireGoal4Chain299Bound the copy runs over native 14h LuaObjects
// (00B66530/00B665D0/00B66580/00B66740/00B66670 for number keys, 00B673A0/
// 00B67460/00B67400/00B66790/00B67580 for string keys) and, unlike 00436C50,
// copies nothing for a key that is neither a string nor a number
// (00436A0B..00436A13). A value nested deeper than the owner's 50 tracking
// slots allow is refused up front (std::length_error, host admission).
void copy_lua_slot_00436730(lua_State* source, int key, int value,
                            lua_State* destination, int table);

// Address: 00438020 (body 00438020..004389E7), reached from MW_SaveCheckpoint
// through 00438A00 (directly, or as the storage continuation 004389F0).
// Original ABI: __thiscall, ECX = profile manager (data LuaObject at +4,
// "write other missions" byte at +18h), RET.
// Replaces BSP_Chk_Save[mission_id] with a new table holding the five fields
// and a copy of the data table, then, when `writer` is non-null (native: the
// 00A3E540 flag and storage state != 1), writes section BSP_Chk_Save, the
// mission section with the five fields and the data slots, and, when
// `include_other_missions`, every other table entry of BSP_Chk_Save whose key
// differs case-insensitively from mission_id. The profile record upsert
// (007FC2C0) and the storage continuation (00BD53C0) stay with the caller.
// Under kWireGoal4Chain299Bound the data-table copy is the native iteration
// 0043844D..004384DB calling 00436C50 (src/oct18_tail_a.cpp), whose string
// setter needs the live 0108FF2C empty-string authority in `empty_0108ff2c`
// (bsp_game: game_native_lua_globals_process()); null throws std::logic_error.
void save_checkpoint_00438020(lua_State* mission, const std::string& mission_id,
                              const CheckpointRecordFields& record, float play_time,
                              lua_State* script, int data_table, SettingsWriter* writer,
                              bool include_other_missions,
                              const char* empty_0108ff2c = nullptr);

// Address: 004379C0, reached from MW_LoadCheckpoint (008ACA30).
// Original ABI: __thiscall, ECX = profile manager, [ESP+4] = destination
// script context, RET 4. When BSP_Chk_Save[mission_id] is a table, returns its
// __playTime (float32, 0.0 when not a number; native stores it at
// [[game+21A0h]+14A0h]) and pushes a 00436730 copy of the entry onto
// `destination`; otherwise pushes nil and returns nothing.
std::optional<float> load_checkpoint_004379c0(lua_State* mission, const std::string& mission_id,
                                              lua_State* destination);

// Address: 00437C70 (Lua part). Sets BSP_Chk_Save[mission_id] = nil. Native
// then erases the profile record (007FBE10), and when the save name at
// game+684h is not empty, the storage is idle and its file exists, requests a
// write whose continuation 00437C60 (no Ghidra function) tail-jumps to
// 00437250.
void clear_checkpoint_00437c70(lua_State* mission, const std::string& mission_id);

// Address: 00437250. Original ABI: __thiscall, ECX = profile manager, RET.
// Returns when storage state is 1; otherwise writes section BSP_Chk_Save
// with every table entry whose key differs from mission_id.
void write_other_checkpoints_00437250(lua_State* mission, const std::string& mission_id,
                                      SettingsWriter& writer);

// Address: 0043CC20 (body 0043CC20..0043CF9B). Original ABI: ECX = out
// NativeString*, [ESP+4] = float seconds, RET 4; caller 0062C2C0 (the
// scoring screen). Truncates to whole seconds (CVTTSS2SI) and builds, in a
// 80h-byte fixed builder: ".<hours>|.:|" when seconds >= 3600, then
// ".<minutes % 60>|.:|" (two digits only when hours were printed), then
// ".<seconds % 60>" (always two digits). "." and "|.:|" are GUI text markup.
std::string format_play_time_0043cc20(float seconds);

} // namespace bsp
