#pragma once

// cc13_seg62L: the multiplayer lobby layout bodies left by the seg 62/65
// large-body packet (packet cc13-seg62L, docs/CC13_SEG62L.md). Names are
// hypotheses, not recovered symbols. New C++ interfaces over the native
// records, not drop-in replacements of the original ABI. Nothing is wired.
//
// Same conventions as cc13_sweepB..B11: native objects are raw byte blocks
// touched only at the offsets the image reads or writes, and every call the
// image makes goes through a named host method at the same point with the
// same arguments. Cc13Seg62LHost extends Cc13SweepB11Host.

#include "bsp/cc13_sweepB11.hpp"

#include <cstdint>

namespace bsp {

class Cc13Seg62LHost : public Cc13SweepB11Host {
public:
    // 004C8950 BSP_Game_SetCommandLineText, __thiscall(ECX = [00E188A8],
    // const char* text), RET 4.
    virtual void game_set_command_line_text_004c8950(void* game, const char* text) = 0;
};

// Lobby layout (screen +2E4h first / +2E8h last of a vector of 24h-byte rows:
// +0h map id, +4h vector of NativeStrings (first +8h, last +0Ch), +14h vector
// of NativeStrings (first +18h, last +1Ch)); selection +2F0h row / +2F4h
// entry, first selection +2F8h / +2FCh; the selected strings are the image
// globals 00E195BC (from the +14h vector) and 00E195C4 (from the +4h vector);
// the game's layout string is [00E188A8]+600h.

// 005D8B80 (lobby layout selection refresh), __thiscall(ECX = screen), RET;
// EAX = 00E195BC. With a selection (+2F0h and +2F4h not -1): 00E195BC = the
// 005D5160 record's +14h vector [+2F4h] and, when [00E188A8]+1FE4h == 2,
// 004C8950 with its +4h vector [+2F4h] text. Otherwise both globals are
// emptied, the selection becomes -1/-1, and every row whose id is
// [[00E188A8]+5FCh]+1098h is scanned for the first +4h entry equal (length,
// then _stricmp) to the game's +600h string; a match selects (row, entry)
// and copies both strings, and the scan goes on to the next row.
std::uint32_t cc13_seg62L_lobby_refresh_selection_005d8b80(void* screen, Cc13Seg62LHost& host);

// 005D8FC0 (lobby layout select default), __thiscall(ECX = screen), RET.
// When [00E188A8]+5FCh is set: every row whose id is its +1098h and whose +14h
// vector is non-empty selects (row, 0), copies entry 0 of both vectors into
// 00E195BC / 00E195C4, records the first selection in +2F8h / +2FCh when both
// are -1, and copies the +4h entry 0 text (00E19605 when null) into the
// game's +600h string.
void cc13_seg62L_lobby_select_default_005d8fc0(void* screen, Cc13Seg62LHost& host);

}  // namespace bsp
