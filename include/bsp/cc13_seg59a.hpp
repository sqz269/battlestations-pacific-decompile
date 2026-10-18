// Segment 59 scoring and awards band (packet cc13-seg59a, docs/CC13_SEG59A.md).
//
// The mission scoring object *(game+21A0h) keeps one 284h-byte record per player slot (eight
// slots) plus a block of point-formula tunables at +1424h..+14B0h. This file reconstructs the
// band's award grant (0090EDE0) and its badge filter (0090C000), the award triggers that feed
// it (GA_AE, GA_DN, GA_AI, SA_IM, SA_OC, RUA_MU, GA_SL), the scoring maps the kill, damage,
// bomb-drop and party-loss events write (among them the slot record's saved unit_usage map at
// record+114h that 0090ED40 adds into, merged by 0091A080 and archived as section
// "unit_usage"), the commit-record message setters, the
// rewards point formulas, and the new-mission reset.
//
// Model as in cc13_seg33a: every routine works on the original objects as raw bytes (Win32
// layout, offsets as in the image) and reaches the engine through Seg59aHost, whose methods
// name the image routine, global or virtual slot they stand for, invoked at the binary's point
// with its arguments. The STL containers inside the records (case-insensitive string->int maps,
// int->int and int->float maps, a ushort vector) are BOUND, not reconstructed: their member
// instantiations are host methods named by address. A new C++ interface, not a drop-in binary
// replacement; nothing here is wired into bsp_game. Names are hypotheses, not recovered
// symbols. Marker: cc13_seg59a.

#ifndef BSP_CC13_SEG59A_HPP
#define BSP_CC13_SEG59A_HPP

#include <cstddef>
#include <cstdint>

namespace bsp::cc13_seg59a {

// The engine's pooled string, 8 bytes: +0 length, +4 data (null when empty and unallocated).
// Passed by value it occupies two stack dwords and the callee releases it.
struct NativeStr {
    std::uint32_t length{0};
    char* data{nullptr};
};
static_assert(sizeof(std::uint32_t) == 4, "Win32 layout");

// A checked STL iterator as the image passes it: +0 owning container, +4 node or element.
struct CheckedIter {
    void* container{nullptr};
    void* node{nullptr};
};

// Record geometry, from the IMUL 284h sites.
inline constexpr std::size_t kSlotRecordStride = 0x284;
inline constexpr std::uint32_t kSlotCount = 8;
inline constexpr std::size_t kAwardMapOffset = 0x70;            // record: string->int award/counter map
inline constexpr std::size_t kAwardMapHeadOffset = 0x74;        // that map's head (end) node
inline constexpr std::size_t kSessionMessageSize = 0x18C;       // [ESP+24h]..[ESP+1B0h) in 0090EDE0

struct Seg59aHost {
    virtual ~Seg59aHost() = default;

    // --- globals ---
    virtual void* game_00e188a8() = 0;                  // *(00E188A8)
    virtual void* hud_owner_00e198c4() = 0;             // *(00E198C4)
    virtual void* award_registry_00e19900() = 0;        // *(00E19900)
    virtual void* online_manager_00f8abe8() = 0;        // *(00F8ABE8)
    virtual void* stats_owner_00f8a2fc() = 0;           // *(00F8A2FC)
    virtual const char* empty_text_00f899a0() = 0;      // 00F899A0, the shared "" literal
    virtual const char* empty_text_00ce3a0c() = 0;      // 00CE3A0C, source of 0091C560's memcpy

    // --- NativeString and its pool ---
    virtual void native_string_resize_0041dd40(NativeStr* s, std::uint32_t length, std::int32_t flag) = 0;
    virtual void native_string_construct_0041e870(NativeStr* s, const char* text) = 0;
    virtual void native_string_copy_construct_00426060(NativeStr* dst, const NativeStr* src) = 0;
    // 00553C80, __thiscall(ECX = string, const char* prefix, offset), RET 8, AL.
    virtual bool native_string_has_prefix_at_00553c80(const NativeStr* s, const char* prefix,
                                                      std::uint32_t offset) = 0;
    // 00419CC0 is pushed (data, length+1, 1) and its result becomes ECX of 00BD1510, which
    // consumes the same three pushes (RET 0Ch). The singleton ignores its pushes.
    virtual void* pool_singleton_00419cc0() = 0;
    virtual void pool_return_block_00bd1510(void* pool, void* data, std::uint32_t size, std::int32_t flag) = 0;
    virtual void memcpy_00bf7680(void* dst, const void* src, std::uint32_t size) = 0;

    // --- checked-STL failure (_invalid_parameter_noinfo) ---
    virtual void invalid_parameter_00bf6713() = 0;

    // --- bound STL member instantiations ---
    virtual CheckedIter string_int_map_find_004c8b80(void* map, const NativeStr* key) = 0;
    virtual std::int32_t* string_int_map_index_005070c0(void* map, const NativeStr* key) = 0;
    virtual std::int32_t* int_int_map_index_00625900(void* map, const std::int32_t* key) = 0;
    virtual float* int_float_map_index_0090e740(void* map, const std::int32_t* key) = 0;
    virtual void* int_map_map_index_009119e0(void* map, const std::int32_t* key) = 0;
    virtual std::int32_t* ushort_int_map_index_00667eb0(void* map, const std::int32_t* key) = 0;
    virtual CheckedIter ushort_int_map_find_0065cd40(void* map, const std::int32_t* key) = 0;
    virtual void ushort_int_map_erase_0058a3a0(void* map, CheckedIter* out, void* where_container,
                                               void* where_node) = 0;
    virtual CheckedIter ushort_vector_begin_00725020(void* vec) = 0;
    virtual CheckedIter ushort_vector_end_00725050(void* vec) = 0;
    virtual bool checked_iter_equal_00722180(const CheckedIter* lhs, const CheckedIter* rhs) = 0;
    virtual void ushort_vector_push_back_007265e0(void* vec, const std::int32_t* value) = 0;
    // 0090D9D0, __thiscall(vector of 8-byte {ushort, float}, iterator* out, const entry*), RET 8.
    virtual void drop_record_vector_append_0090d9d0(void* vec, CheckedIter* out, const void* entry) = 0;

    // --- engine callees ---
    virtual void session_message21_construct_00764340(void* message) = 0;
    // 00772AF0 with ECX = game+1EF0h, (message*, peer record = [game+18CCh+slot*4]), callee pops 8.
    virtual void session_send_to_remote_player_00772af0(void* session, void* message, void* peer) = 0;
    // 00648AB0, ECX = [[00E198C4]+40h], NativeString by value (the callee releases it).
    virtual void hud_root_queue_award_ticker_00648ab0(void* hud_root, NativeStr token) = 0;
    virtual std::int32_t award_registry_id_for_name_006b8da0(void* registry, const NativeStr* name) = 0;
    virtual void online_awards_grant_00a410a0(void* online_manager, std::int32_t award_id) = 0;
    virtual void award_tracker_record_at_least_007fbe20(void* tracker, const NativeStr* key, std::int32_t value) = 0;
    virtual void* award_table_find_record_0050fc30(void* table, const NativeStr* key) = 0;
    virtual void* global_config_00432650() = 0;
    virtual void mission_player_records_reset_00916980(void* scoring) = 0;
    virtual void stats_owner_reset_00994310(void* stats_owner) = 0;
    virtual void session_message_construct_base_0075b430(void* message, std::int32_t type) = 0;
    // 005097D0, __thiscall(profile = game+650h, const NativeString* name), RET 4, EAX.
    virtual std::int32_t profile_get_named_counter_005097d0(void* profile, const NativeStr* name) = 0;
    // 00770AF0, __thiscall(session = game+1EF0h, message*), RET 4.
    virtual void session_send_to_secondary_first_peer_00770af0(void* session, void* message) = 0;

    // Virtual slot 0 of the unit's interface at unit+170h (the unit's category key).
    virtual std::int32_t unit_iface170_slot0(void* iface) = 0;
};

// --- badge filter and award grant ---
bool score_table_is_badge_earned_0090c000(Seg59aHost& host, void* scoring, NativeStr key);
void scoring_grant_award_0090ede0(Seg59aHost& host, void* scoring, std::int32_t slot, const NativeStr* token,
                                  std::int32_t value, std::uint8_t accumulate);
std::int32_t award_registry_score_for_name_00914270(Seg59aHost& host, void* scoring, NativeStr key);

// --- award triggers ---
void scoring_grant_ga_ae_once_0090f5a0(Seg59aHost& host, void* scoring, std::int32_t slot);
void scoring_plane_achievement_0090f6c0(Seg59aHost& host, void* scoring, void* plane, std::int32_t kind);
void scoring_capture_awards_0090f860(Seg59aHost& host, void* scoring, std::uint32_t slot, std::int32_t building_kind,
                                     std::int32_t building_type);
void scoring_spawn_awards_00913ab0(Seg59aHost& host, void* scoring, std::int32_t slot, std::int32_t kind);

// --- scoring map writers (e.g. the saved unit_usage map, slot record+114h) ---
CheckedIter* find_ushort_00908500(CheckedIter* out, const std::int32_t* value, CheckedIter first, CheckedIter last);
void scoring_record_unique_kill_0090ec40(Seg59aHost& host, void* scoring, void* unit, std::int32_t unused);
void scoring_add_unit_float_0090ed40(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit, float amount);
void scoring_add_party_loss_0090ed90(Seg59aHost& host, void* scoring, std::int32_t party, const NativeStr* name,
                                     std::int32_t amount);
void scoring_count_unit_event_0090eb80(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit,
                                       std::uint8_t second_map);
void scoring_count_owner_event_0090ebf0(Seg59aHost& host, void* scoring, void* unit);
void scoring_record_drop_interval_0090e670(Seg59aHost& host, void* scoring, std::uint32_t slot, void* unit,
                                           float seconds);
void scoring_set_slot_ranking_00905350(void* scoring, std::int32_t slot, std::int32_t ranking);

// --- commit-record message setters ---
void score_record_set_condition_message_0090bda0(Seg59aHost& host, void* record, NativeStr text);
void score_record_set_victory_message_0090be30(Seg59aHost& host, void* record, NativeStr text);
void score_record_set_message_264_0090bec0(Seg59aHost& host, void* record, NativeStr text);
void scoring_set_final_function_name_0090bfc0(Seg59aHost& host, void* scoring, const NativeStr* name);

// --- rewards point formulas (main thread, x87 24-bit precision control assumed) ---
std::int32_t scoring_map_usage_points_009064d0(void* scoring, float seconds);
std::int32_t scoring_remaining_unit_points_00910500(Seg59aHost& host, void* scoring, std::int32_t category,
                                                    std::uint32_t index);
std::int32_t scoring_suicide_points_00910570(Seg59aHost& host, void* scoring, std::int32_t category,
                                             std::uint32_t index, std::int32_t count);
std::int32_t scoring_kill_points_00911c00(Seg59aHost& host, void* scoring, std::int32_t selector, std::int32_t by,
                                          std::int32_t target, float kills, std::uint32_t index, std::uint8_t flag);
std::int32_t scoring_damage_points_00911ce0(Seg59aHost& host, void* scoring, std::int32_t by, std::int32_t target,
                                            std::int32_t selector, std::int32_t damage, std::uint32_t index,
                                            std::uint8_t flag);

// --- new-mission reset ---
void scoring_reset_for_new_mission_0091c560(Seg59aHost& host, void* scoring);
void scoring_sync_profile_counters_0090fa40(Seg59aHost& host, void* scoring);

}  // namespace bsp::cc13_seg59a

#endif  // BSP_CC13_SEG59A_HPP
