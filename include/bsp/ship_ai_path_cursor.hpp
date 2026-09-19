// The path cursor a `moveonpath` command carries, and the three routines that
// read and move it. Packet `cc8_ship_moveonpath`, read from the LISTING.
//
// Where the cursor lives. 0071BFF0 (`__thiscall(director)(int index)`, RET 4,
// body 0071BFF0-0071C00F, read whole) answers
// `(director->slots[index] ? director->slots[index] : director->slots[0]) + 10h`
// with the slot array at director+1A4h and the bound `index <= 9` at 0071BFF4.
// So every routine below takes the sub-object at command+10h as `this`, and the
// fields it reads are:
//
//   command+08h  follow mode   PATH_FM_*   written by 0071C1B0 at 0071C1D2
//   command+0Ch  start mode    PATH_SM_*   written by 0071C1B0 at 0071C1D8
//   command+10h  the cursor's own vtable
//   command+14h  cursor+04h    the path source object (0Ch bytes, 007B2250 /
//                              007B22A0: vtable, refcount 1, path interface)
//   command+18h  cursor+08h    the current point index
//   command+1Ch  cursor+0Ch    the follow mode the cursor runs on
//   command+20h  cursor+10h    the direction byte, 1 forward and 0 backwards
//
// The two mode spaces are the shipped script's own, and this installation's
// scripts/global/luamw_init.lua lines 224-232 name every value. 007ADC60 and
// 007ADCC0 branch on exactly 1, 2 and 3 and 007B1C50 on exactly 5, 6, 7 and 8,
// so the binding of name to number is established by the two sides agreeing,
// not assumed.

#ifndef BSP_SHIP_AI_PATH_CURSOR_HPP
#define BSP_SHIP_AI_PATH_CURSOR_HPP

#include <cstdint>

namespace bsp {

// PATH_FM_*, scripts/global/luamw_init.lua 224-226. The comments are the
// shipped Hungarian, translated.
enum : int {
    kShipAiPathFollowSimple = 1,    // PATH_FM_SIMPLE, runs to the end and stops
    kShipAiPathFollowPingPong = 2,  // PATH_FM_PINGPONG, back and forth, forever
    kShipAiPathFollowCircle = 3,    // PATH_FM_CIRCLE, round and round
};

// PATH_SM_*, scripts/global/luamw_init.lua 228-232.
enum : int {
    kShipAiPathStartJoin = 5,          // PATH_SM_JOIN / _JOIN_FORWARD, nearest point
    kShipAiPathStartBegin = 6,         // PATH_SM_BEGIN, the head of the path
    kShipAiPathStartJoinRandomDir = 7, // PATH_SM_JOIN_RANDOM_DIR, nearest, either way
    kShipAiPathStartJoinBackwards = 8, // PATH_SM_JOIN_BACKWARDS, nearest, reversed
};

// The cursor's three mutable fields. `point_count` is not a cursor field: it is
// what `path->vtable[0Ch]()` answers, called fresh at every site that needs it
// (007ADC60 at 007ADC78, 007ADCC0 at 007ADCE6, 007ADD70 at 007ADD84), so it is
// passed in rather than cached here.
struct ShipAiPathCursor {
    int index_08{0};          // cursor+08h
    int follow_mode_0c{0};    // cursor+0Ch
    bool forward_10{true};    // cursor+10h, the direction byte
};

// 007ADC30 `BSP_EntityCommand_SlotHasNoLegs` already has a projection in
// src/ship_ai_goal_vector.cpp; this header does not duplicate it.

// 007ADC60 `BSP_EntityCommand_IsOnFinalLeg`, `bool __thiscall(cursor)()`, RET 0,
// body 007ADC60-007ADCBD, read whole.
//
//   007ADC63  no path object      -> true
//   007ADC7A  count <= 0          -> true
//   007ADC87  mode == 1 (SIMPLE): forward and index == count-1 -> true
//                                 backwards and index == 0     -> true
//   007ADC8F  mode 2 or 3         -> false, always: PINGPONG and CIRCLE have no
//                                   final leg, which is why a carrier ordered
//                                   round a patrol path never finishes
//   otherwise (mode 0 or >= 4)    -> false
bool ship_ai_path_on_final_leg_007adc60(const ShipAiPathCursor& cursor,
                                        bool has_path, int point_count);

// 007ADCC0, `int __thiscall(cursor)()`, RET 0, body 007ADCC0-007ADD4A, read
// whole. Answers the index the cursor should move to and, for PINGPONG only,
// flips the direction byte in place (007ADD17 and 007ADD34). It does NOT store
// the index: its caller 007ADD70 does that through `cursor->vtable[2](next)` at
// 007ADFE9.
//
//   mode 1 SIMPLE    007ADD3D: index +1 forward, -1 backwards, no wrap. The
//                    caller's guard at 007ADFAC-007ADFD4 stops it at the ends.
//   mode 2 PINGPONG  007ADD0E: at index 0 turn forward and answer 1; at the last
//                    point turn backwards and answer index-1; otherwise the
//                    SIMPLE step.
//   mode 3 CIRCLE    007ADCD9: forward, the last point wraps to 0; backwards,
//                    index 0 wraps to count-1; otherwise the SIMPLE step.
//   other            007ADCD7 JNZ: the index, unchanged.
int ship_ai_path_next_index_007adcc0(ShipAiPathCursor& cursor, int point_count);

// 007ADFAC-007ADFD4, the guard 007ADD70 puts in front of every advance. False
// means the cursor has reached the terminal point of a SIMPLE path and 007ADD70
// leaves the loop with its `no advance happened` flag still set, which is the
// only way that routine answers true.
bool ship_ai_path_advance_allowed_007adfac(const ShipAiPathCursor& cursor,
                                           bool has_path, int point_count);

// 007B1C50, `__thiscall(cursor)(holder, pos, float, int follow_mode,
// int start_mode)`, RET 14h, body 007B1C50-007B1D2E, read whole. The path build
// 0071F600 reaches through 007B1D30 once, when the command is begun.
//
//   007B1C97  cursor+0Ch = follow mode
//   007B1C9A  cursor+10h = 1, forward
//   007B1C9E  start mode 6 BEGIN          -> index 0 and return
//   007B1CBB  start mode 8 JOIN_BACKWARDS -> direction 0
//   007B1CD1  start mode 7 JOIN_RANDOM_DIR-> 00BD2F10(0.0f, 1.0f) against the
//             0.5f at 00CE3800: strictly greater keeps forward
//   007B1D06  start mode 5 JOIN           -> forward
//   007B1D24  every start mode but 6 then takes the index from
//             `path->vtable[10h](pos, f, direction)` = 007B11F0.
//
// `random_forward` is the answer that comparison would give; a caller that does
// not draw a number passes true, which is what start mode 5 does anyway.
void ship_ai_path_cursor_start_007b1c50(ShipAiPathCursor& cursor, int follow_mode,
                                        int start_mode, int joined_index,
                                        bool random_forward);

// 007B11F0, `int __thiscall(path)(const float* pos, float f, int direction)`,
// body 007B11F0-007B12xx. HEAD READ ONLY, and the tail is labelled where it is
// used: 007B11FE takes the count, 007B120E calls `path->vtable[14h](pos)` for
// the nearest point index, and 007B121B-007B123A picks that point's neighbour
// in the travel direction as a second candidate. The projection test at
// 007B1263 onwards that chooses between the two was not read, and this function
// answers the nearest index alone.
//
// `path->vtable[14h]` (007B1100 on the authored-path source) is itself unread:
// the planar nearest-point search here is a HYPOTHESIS for it, not a reading.
int ship_ai_path_nearest_index_007b1100(const float* points_xyz, int point_count,
                                        float x, float z);

}  // namespace bsp

#endif  // BSP_SHIP_AI_PATH_CURSOR_HPP
