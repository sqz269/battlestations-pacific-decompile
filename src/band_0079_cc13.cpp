// Band 00790000-007FFFFF non-gameplay leaves (packet cc13-sweep79,
// docs/CC13_BAND_0079_007F.md). Evidence: live Ghidra decompiles saved by the
// sweep and the listing where noted. Reconstructed and build-tested only;
// nothing here is wired into the game host or game-validated.
#include "bsp/band_0079_cc13.hpp"

namespace bsp::band0079_cc13 {

namespace {

// ASCII case-insensitive equality, the "C"-locale behaviour of the CRT
// __stricmp the image calls (only the zero/non-zero result is used).
bool ascii_iequal(const char* a, const char* b) {
    for (;; ++a, ++b) {
        unsigned char ca = static_cast<unsigned char>(*a);
        unsigned char cb = static_cast<unsigned char>(*b);
        if (ca >= 'A' && ca <= 'Z') ca = static_cast<unsigned char>(ca - 'A' + 'a');
        if (cb >= 'A' && cb <= 'Z') cb = static_cast<unsigned char>(cb - 'A' + 'a');
        if (ca != cb) return false;
        if (ca == 0) return true;
    }
}

}  // namespace

// 00797980: if (entity == 0) return 0; then for each node of the list at
// +6Ch: state +0F8h not 0 and not 3, and key+1Ch == entity -> return 1.
bool track_has_active_key_for_entity_00797980(const KeyframeList& keys, const void* entity) {
    if (entity == nullptr) return false;
    for (const MovieKeyframeView* key : keys) {
        if (keyframe_is_active(*key) && key->entity_1c == entity) return true;
    }
    return false;
}

// 00797C60: first node with *(short*)(key+0DAh) == id; returns node->value
// (the key), 0 at the end of the list. No state test here.
const MovieKeyframeView* track_find_key_by_id_00797c60(const KeyframeList& keys, std::int32_t id) {
    for (const MovieKeyframeView* key : keys) {
        if (static_cast<std::int32_t>(key->id_da) == id) return key;
    }
    return nullptr;
}

// 00797D20: first active key whose +1Ch is non-null; returns that entity.
const void* track_first_active_key_entity_00797d20(const KeyframeList& keys) {
    for (const MovieKeyframeView* key : keys) {
        if (keyframe_is_active(*key) && key->entity_1c != nullptr) return key->entity_1c;
    }
    return nullptr;
}

// 00793610 / 007936D0 (and 00793670): for i in [0, count) re-walk i links
// from the head, take node+8h, compare payload+4h with the unit.
std::int32_t recon_contact_index_of_00793610(const ReconContactTriple& triple, const void* unit) {
    for (std::int32_t i = 0; i < triple.count_00; ++i) {
        const ReconContactNode* node = triple.head_04;
        for (std::int32_t j = 0; node != nullptr && j < i; ++j) node = node->next_04;
        // The image reads [0+4h] here when the list is short; see the header.
        if (node != nullptr && node->payload_08 != nullptr && node->payload_08->unit_04 == unit) return i;
    }
    return -1;
}

// 00793380: for (; index > 0; --index) { if (!node) return 0; if (!node->next)
// break; node = node->next; } return node ? node->payload : 0.
const ReconContactPayload* recon_list_payload_at_00793380(const ReconContactNode* head, std::int32_t index) {
    const ReconContactNode* node = head;
    for (; index > 0; --index) {
        if (node == nullptr) return nullptr;
        if (node->next_04 == nullptr) break;
        node = node->next_04;
    }
    return node != nullptr ? node->payload_08 : nullptr;
}

bool player_record_scene_check_set_007f7470(const mp_cc13::PlayerRecord& record) {
    return record.scene_check_14 == 1;  // CMP [ECX+14h],1; SETE AL
}

bool player_record_scene_check_clear_007f7480(const mp_cc13::PlayerRecord& record) {
    return record.scene_check_14 == 0;
}

void player_record_stamp_voice_time_007f7490(mp_cc13::PlayerRecord& record, bool stamp,
                                             std::uint32_t game_time_64c) {
    if (stamp) record.last_voice_time_8c = game_time_64c;
}
void stamp_local_voice_time_0076c420(mp_cc13::PlayerRecord* records, std::int32_t local_index,
                                   bool stamp, std::uint32_t game_time_64c) {
    if (local_index >= 0) player_record_stamp_voice_time_007f7490(records[local_index], stamp, game_time_64c);
}

void player_record_set_camera_state_007f7c60(PlayerCameraState& state, std::int32_t tick,
                                             const std::uint32_t a[3], const std::uint32_t b[3],
                                             std::uint32_t word, std::uint8_t byte) {
    state.tick_a8 = tick;
    for (int i = 0; i < 3; ++i) state.vec_ac[i] = a[i];
    for (int i = 0; i < 3; ++i) state.vec_b8[i] = b[i];
    state.word_c4 = word;
    state.byte_c8 = byte;
}

bool player_record_has_recent_camera_state_007f7cd0(const PlayerCameraState& state,
                                                    std::int32_t tick_00f876b0) {
    // 32-bit wrapping subtraction, then a signed compare (the decompile's int).
    const auto diff = static_cast<std::int32_t>(static_cast<std::uint32_t>(tick_00f876b0) -
                                                static_cast<std::uint32_t>(state.tick_a8));
    return diff < kCameraStateFreshTicks;
}

// 007F7BF0: i = 0..2 over 00E0B074; returns i on a match, 0 after the third.
std::int32_t difficulty_index_of_name_007f7bf0(const CountedStringView& name) {
    for (std::int32_t i = 0; i < 3; ++i) {
        const char* entry = kDifficultyNames[i];
        if (name.text == nullptr) {
            if (entry == nullptr || entry[0] == '\0') return i;
        } else if (entry == nullptr) {
            if (name.length == 0) return i;
        } else if (ascii_iequal(name.text, entry)) {
            return i;
        }
    }
    return 0;
}

}  // namespace bsp::band0079_cc13
