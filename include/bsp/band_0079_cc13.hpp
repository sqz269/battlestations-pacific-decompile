#pragma once
// Band 00790000-007FFFFF, non-gameplay pieces (packet cc13-sweep79,
// docs/CC13_BAND_0079_007F.md): the movie-camera track's keyframe list
// queries, the recon contact lists, the player-record leaves in 007F7xxx and
// the difficulty name lookup. New C++ interfaces over the recovered rules;
// none of these is a drop-in binary replacement, and nothing here is wired
// into the game host. Names are hypotheses, not recovered symbols.
#include <cstddef>
#include <cstdint>
#include <list>

#include "bsp/multiplayer_session_cc13.hpp"

namespace bsp::band0079_cc13 {

// ---------------------------------------------------------------------------
// Movie-camera track keyframe list (track+6Ch, a std::list<Keyframe*>)
// ---------------------------------------------------------------------------

// The keyframe fields the list queries read. Offsets are the image's; the
// keyframe class itself (vtable 00D046E0, 0079C910) is larger.
struct MovieKeyframeView {
    const void* entity_1c = nullptr;  // +1Ch, watched entity (MovieCameraKeyframe_SetEntity)
    std::int16_t id_da = -1;          // +0DAh, key id (0079C910 sets FFFFh)
    float start_f0 = 0;               // +0F0h, start time
    float duration_f4 = 0;            // +0F4h
    std::int32_t state_f8 = 0;        // +0F8h, 0 and 3 are skipped by every query
};

// A key takes part in the queries unless its state is 0 or 3.
inline bool keyframe_is_active(const MovieKeyframeView& key) {
    return key.state_f8 != 0 && key.state_f8 != 3;
}

using KeyframeList = std::list<const MovieKeyframeView*>;

// 00797980 MovieCameraTrack_HasActiveKeyForEntity, __thiscall(track, entity)
// RET 4. A null entity answers false before the walk.
bool track_has_active_key_for_entity_00797980(const KeyframeList& keys, const void* entity);

// 00797C60 MovieCameraTrack_FindKeyById, __thiscall(track, id) RET 4. The
// image compares the short at +0DAh with the full int argument, so an id
// outside the short range never matches. Returns null when nothing matches.
const MovieKeyframeView* track_find_key_by_id_00797c60(const KeyframeList& keys, std::int32_t id);

// 00797D20 MovieCameraTrack_FirstActiveKeyEntity, __fastcall(track). The first
// non-null entity among active keys, else null.
const void* track_first_active_key_entity_00797d20(const KeyframeList& keys);

// ---------------------------------------------------------------------------
// Recon contact lists (ReconSlot triples, docs/FIXED_STEP_COUNTDOWN.md)
// ---------------------------------------------------------------------------

struct ReconContactPayload {
    std::uint32_t word_00 = 0;
    const void* unit_04 = nullptr;    // +04h, the unit the IndexOf helpers compare
};

struct ReconContactNode {
    std::uint32_t word_00 = 0;
    const ReconContactNode* next_04 = nullptr;      // +04h
    const ReconContactPayload* payload_08 = nullptr; // +08h
};

// One {count, head} pair of the slot: own +0DD8h, enemy +0DE4h, neutral +0DF0h.
struct ReconContactTriple {
    std::int32_t count_00 = 0;
    const ReconContactNode* head_04 = nullptr;
};
inline constexpr std::size_t kReconOwnTripleOffset = 0xDD8;      // 00793610
inline constexpr std::size_t kReconEnemyTripleOffset = 0xDE4;    // 00793670 (cc7 name)
inline constexpr std::size_t kReconNeutralTripleOffset = 0xDF0;  // 007936D0

// 00793610 BSP_Recon_OwnContactIndexOf / 007936D0 BSP_Recon_NeutralContactIndexOf
// (and the enemy twin 00793670), __thiscall(slot, unit) RET 4: index of the
// first node whose payload +4h is the unit, else -1. Each step re-walks the
// list from the head. The image dereferences a null payload when the list is
// shorter than count; this interface requires count <= list length.
std::int32_t recon_contact_index_of_00793610(const ReconContactTriple& triple, const void* unit);

// 00793380 BSP_ReconList_PayloadAt, __thiscall(list, index) RET 4: walks index
// next links from list+4h and returns the payload. Quirk kept: when a node has
// no next before index is used up, the walk stops on that node and returns its
// payload (the last element), not null. A null head gives null.
const ReconContactPayload* recon_list_payload_at_00793380(const ReconContactNode* head, std::int32_t index);

// ---------------------------------------------------------------------------
// Player-record leaves (118h records, bsp::mp_cc13::PlayerRecord)
// ---------------------------------------------------------------------------

// 007F7470 / 007F7480, __fastcall(record): +14h == 1 / == 0 (the MT_CHECKSCENELOAD
// word). Caller 0076F400.
bool player_record_scene_check_set_007f7470(const mp_cc13::PlayerRecord& record);
bool player_record_scene_check_clear_007f7480(const mp_cc13::PlayerRecord& record);

// 007F7490 BSP_PlayerRecord_StampVoiceTime, __thiscall(record, bool) RET 4:
// with the flag, +8Ch = game+64Ch (passed in here).
void player_record_stamp_voice_time_007f7490(mp_cc13::PlayerRecord& record, bool stamp,
                                             std::uint32_t game_time_64c);
// 0076C420: reads signed game+18C8h, and when >=0 tail-calls 007F7490
// on game+748h + index*118h. Both branches consume one stack word (RET4).
void stamp_local_voice_time_0076c420(mp_cc13::PlayerRecord* records, std::int32_t local_index,
                                   bool stamp, std::uint32_t game_time_64c);

// The MT_CAMERA (55h) state 007F7C60 stores in the record (+0A8h..+0C8h).
struct PlayerCameraState {
    std::int32_t tick_a8 = 0;          // +0A8h, the message tick
    std::uint32_t vec_ac[3]{};         // +0ACh..+0B4h
    std::uint32_t vec_b8[3]{};         // +0B8h..+0C0h
    std::uint32_t word_c4 = 0;         // +0C4h
    std::uint8_t byte_c8 = 0;          // +0C8h
};

// 007F7C60 BSP_PlayerRecord_SetCameraState, __thiscall(record, tick, a*, b*,
// word, byte) RET 14h: plain stores, the vectors copied as dwords.
void player_record_set_camera_state_007f7c60(PlayerCameraState& state, std::int32_t tick,
                                             const std::uint32_t a[3], const std::uint32_t b[3],
                                             std::uint32_t word, std::uint8_t byte);

// 007F7CD0 BSP_PlayerRecord_HasRecentCameraState, __fastcall(record): signed
// (tick_00f876b0 - record+0A8h) < 1Eh. The image returns the comparison in AL
// with the high bytes of the difference left in EAX.
inline constexpr std::int32_t kCameraStateFreshTicks = 0x1E;
bool player_record_has_recent_camera_state_007f7cd0(const PlayerCameraState& state,
                                                    std::int32_t tick_00f876b0);

// ---------------------------------------------------------------------------
// Difficulty names (00E0B074 -> 00D08BE8, 00D08BD0, 00D08BB8)
// ---------------------------------------------------------------------------

inline constexpr const char* kDifficultyNames[3] = {
    "globals.diff_rookie",   // 00D08BE8
    "globals.diff_regular",  // 00D08BD0
    "globals.diff_veteran",  // 00D08BB8
};

// The engine's counted string as 007F7BF0 reads it: length +0h, text +4h.
struct CountedStringView {
    std::uint32_t length = 0;
    const char* text = nullptr;
};

// 007F7BF0 BSP_Difficulty_IndexOfName, __fastcall(const string*): index of the
// name among kDifficultyNames compared with __stricmp; a null text matches an
// empty or null table entry. No match returns 0, the same as "rookie".
std::int32_t difficulty_index_of_name_007f7bf0(const CountedStringView& name);

}  // namespace bsp::band0079_cc13
