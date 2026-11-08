#pragma once
#include <cstdint>
#include <vector>
#include "bsp/mission_lua_class_table_resets.hpp"

namespace bsp::entity_mission_seg60_argon { struct Seg60Host; }
namespace bsp { struct ActiveFailure; }

namespace bsp::oct18_tail_b {
// Address-suffixed, source-level interfaces over genuine Win32 object storage.
// Original receivers/stack cleanup are recorded in docs/OCT18_TAIL_B.md. These
// interfaces are not binary replacements. Every dependency below is required;
// this module supplies no fallback host and performs no game-host wiring.

// The only two calls the four-key subscription matchers 00968470 / 00968AF0 make,
// split out of Host so a listener dispatcher can run them without the rest.
class SubscriptionFilterHost {
public:
    virtual ~SubscriptionFilterHost() = default;
    // [filter]->vtable[0](ECX = filter, boxed parameter) -> AL; `filter` is the
    // filter object embedded in the subscription (LEA ECX,[EDI+0Ch] at 00968498).
    virtual bool filter_vslot_00(void* filter, std::uint32_t value) = 0;
    // The original uses the canonical CRT invalid-iterator/range handler.
    // Providers must not return from this call.
    virtual void invalid_iterator_00bf6713() = 0;
};

// 007F8890 only needs the CRT comparison and invalid-iterator boundary. Keep
// those dependencies usable by the live profile owner without unrelated hosts.
class ProfileContentHost {
public:
    virtual ~ProfileContentHost() = default;
    virtual int compare_case_insensitive_00bf7fbf(const char*, const char*) = 0;
    virtual void invalid_iterator_00bf6713() = 0;
};

class Host : public SubscriptionFilterHost, public ProfileContentHost {
public:
    ~Host() override = default;
    virtual void run_lua_006b8ad0(const char*, int, int, int) = 0;
    virtual bool is_kind_vslot_5c(void*, int) = 0;
    virtual void movie_reset_vslot_2c(void*) = 0;
    virtual void copy_controller_00720450(void* destination, void* source) = 0;
    virtual bool category_vslot_48(void*, int) = 0;
    virtual void* fire_target_vslot_2c(void*) = 0;
    virtual void store_fire_target_00836240(void*, void*, std::uint8_t) = 0;
    virtual void visit_enables_00836510(void*, void*) = 0;
    virtual void visit_reference_vslot_10(void* visitor, const char* name,
                                         int type, void* value) = 0;
    virtual void deliver_entity_vslot_18c(void*, void*) = 0;
    virtual void erase_peer_holder_0077d980(void* list, void* node) = 0;
    virtual void free_00bf65ac(void*) = 0;
    virtual void scalar_delete_vslot_00(void*, unsigned flags) = 0;
    virtual void enter_critical_section(void*) = 0;
    virtual void leave_critical_section(void*) = 0;
    virtual void delete_critical_section(void*) = 0;
    virtual void* buy_peer_node_0076ea10(void* list, void* next, void* previous, void* peer) = 0;
    virtual void grow_list_00773600(void* list, unsigned amount) = 0;
    virtual bool host_address_equal_vslot_34(void* client, void*, void*) = 0;
    virtual void insert_host_vector_00785700(void* vector, void* end, void* const* value) = 0;
    virtual void construct_controller_00720180(void*, void* endpoint) = 0;
    virtual void visitor_open_vslot_04(void*, const char* name) = 0;
    virtual void visitor_close_vslot_08(void*) = 0;
    virtual void visitor_value_vslot_0c(void*, const char*, int type, std::uint32_t bits) = 0;
    virtual bool class_is_kind_vslot_18(void*, int) = 0;
    virtual void* allocate_00bf681b(unsigned bytes) = 0;
    virtual void* construct_auto_target_009f7590(void* memory, void* block, void* squadron) = 0;
    virtual void stationary_dead_vslot_7c(void*) = 0;
    virtual void stationary_live_vslot_70(void*, unsigned) = 0;
    virtual void release_body_00c34f70(void* world, void* body) = 0;
    virtual void resize_body_vector_00882620(void* vector, unsigned, unsigned) = 0;
    virtual void remove_floating_object_00b9fe00(void* wave_field, void*) = 0;
    virtual void unlink_release_node_00b6dfa0(void*) = 0;
    virtual void remove_wave_object_vslot_1c(void* wave_field, void*) = 0;
    virtual void terrain_delete_vslot_04(void*, unsigned) = 0;
    virtual void release_reference(void*) = 0; // InterlockedDecrement(+4h); slot 0() iff zero
    struct Iterator { void* owner; void* node; };
    virtual Iterator find_target_00863b10(void* tree, void* target) = 0;
    virtual void* construct_target_00864880(void* memory, void* unit, void* target) = 0;
    virtual void insert_target_00864160(void* tree, void* target, void* record) = 0;
    virtual void register_gun_00862660(void* record, void* gun) = 0;
    virtual void* vehicle_class_00964790(std::uint32_t class_id, bool read_race) = 0;
    virtual void* instantiate_class_vslot_28(void* descriptor, std::uint16_t entity_id) = 0;
    virtual void native_string_resize_0041dd40(void* header, std::uint32_t length, bool terminate) = 0;
    virtual void construct_score_record_0091ce90(void* receiver) = 0;
    virtual void select_host_00783f20(void* client, std::uint32_t index) = 0;
    virtual void* game_global_00e188a8() = 0;
    virtual void scene_records_destroy_all_004bf930(void* list) = 0;
    virtual void apply_director_command_0071c1e0(void* director, void* message) = 0;
    virtual void set_gunbot_target_006df170(void* bot, void* target) = 0;
    virtual bool derive_ship_turn_fields_00828f20(void* descriptor) = 0;
    virtual void insert_census_00441f30(void* vector, void* end, const std::int32_t* value) = 0;
    virtual void read_vehicle_fields_00960230(void* descriptor, void* lua_row) = 0;
    virtual void insert_string_set_004d0640(void* set, const void* native_key) = 0;
    virtual void increment_string_tree_004be730(Iterator&) = 0;
    virtual void erase_string_subtree_004cec60(void* set, void* root) = 0;
    virtual Iterator find_counter_004c8b80(void* map, const void* native_key) = 0;
    virtual std::int32_t* index_counter_005070c0(void* map, const void* native_key) = 0;
    void invalid_iterator_00bf6713() override = 0;
};

bool class60_is_kind_007810f0(const void*, int);
bool class54_is_kind_0079a380(const void*, int);
bool class55_is_kind_0079d840(const void*, int);
void plane_clear_turbo_007b8a90(void*);
// Typed view of 007B8A90 over the caller's storage: the turbo byte +9F9h = 0.
// The raw entry above wraps it.
template <class Byte>
void plane_clear_turbo_007b8a90(Byte& turbo_9f9) noexcept {
    turbo_9f9 = static_cast<Byte>(0);
}
bool director_allows_category_008364b0(int) noexcept;
bool ship_director_allows_category_00836790(int) noexcept;
float gun_class_value_for_target_00836eb0(const void*, void*, Host&);
// Typed view of 00836EB0's receiver: the two floats it chooses between, block
// +764h (target answers kind 08h, MSubmarine) and +768h (any other target).
struct GunClassTargetValues {
    float value_764;
    float value_768;
};
float gun_class_value_for_target_00836eb0(const GunClassTargetValues& values,
                                          bool target_is_kind_8) noexcept;
void* ship_tuning_block_00837de0(void* settings, const void* game);
void reset_stationary_class_table_0084eb60(MissionLuaRunString&);
void movie_camera_track_set_time_00797da0(void* track, float time, Host&);
void director_copy_state_00836680(void* destination, void* source, Host&);
void director_visit_state_008367f0(void* director, void* visitor, Host&);
void* stationary_part_scene_0084efe0(const void* stationary);
bool consume_single_peer_flag_00784cd0(void* transport, Host&);
bool recon_subscription_matches_00968470(const void* subscription,
                                         const void* parameters, SubscriptionFilterHost&);

void deliver_entity_sync_0077c710(void* entity, void* message, Host& host);
void remove_peer_holder_0077da20(void* entity, void* record, Host& host);
void network_destruct_007844e0(void* network, Host& host);
void clear_host_lists_00785170(void* client, Host& host);
void queue_peer_removal_007857f0(void* network, void* peer, std::uint32_t timer_bits, Host& host);
void merge_pending_hosts_00785cc0(void* client, Host& host);
void* construct_director_base_008363e0(void* director, void* endpoint, std::uint8_t flag, Host& host);
void director_visit_properties_008360c0(void* director, void* visitor, Host& host);
void director_dump_properties_008362a0(void* director, void* visitor, Host& host);
void* construct_squadron_block_0084d810(void* block, void* squadron, Host& host);
void stationary_apply_state_0084ec10(void* message, void* const* first_global,
                                    void* const* end_global, Host& host);
void landscape_release_terrain_008826f0(void* landscape, const void* game, Host& host);
void add_gun_to_target_00864ca0(void* ai, void* target, void* gun, Host& host);
void* instantiate_ship_message_0081bfe0(void* message, Host& host);
// 00759090 is an unconditional JMP to 0081BFE0, same ECX/RET ABI.
void* instantiate_ship_message_thunk_00759090(void* message, Host& host);
void* construct_score_record_tail_0091d620(void* receiver, Host& host);
void select_listed_server_007701f0(void* session, std::uint32_t index,
                                  std::uint32_t& scene_flag, const void* empty_bytes, Host& host);
void* vehicle_class_get_or_create_007b8a80(std::uint32_t type_id, Host& host);
void director_apply_command_thunk_00835610(void* director, void* message, Host& host);
void gunbot_set_target_thunk_008fe790(void* bot, void* target, Host& host);
bool ship_derive_turn_fields_thunk_00852200(void* descriptor, Host& host);

float engine_jam_seconds_0093a3f0(const void* task, Host& host);
// 0093A3F0's typed view: the same body over the ActiveFailure vector a host
// keeps as its copy of the repair task's +18h failure records.
float engine_jam_seconds_0093a3f0(const std::vector<ActiveFailure>& failures);
void append_vehicle_census_0095c550(void* vector, std::int32_t class_id, Host& host);
// 0095C550's typed view: the same body over a std::vector a host keeps as its
// copy of the census at 00F8A09C (dedupe scan, then append at last).
void append_vehicle_census_0095c550(std::vector<std::int32_t>& census, std::int32_t class_id);
void inherit_race_activate_thunk_009277e0(void* entity);
std::uint8_t surface_target_thunk_00922dc0(void* entity,
                                          entity_mission_seg60_argon::Seg60Host& host);
void read_vehicle_fields_thunk_008429c0(void* descriptor, void* lua_row, Host& host);
bool event_subscription_matches_00968af0(const void* subscription, const void* parameters,
                                         SubscriptionFilterHost& host);
char* decode_scene_fragment_008ee670(const char* input, char* scratch);

bool profile_owns_content_007f8890(const void* profile, const void* id, ProfileContentHost& host);
void flush_pending_unlocks_007fb990(void* profile, Host& host);
void record_award_at_least_007fbe20(void* profile, const void* key, std::int32_t value, Host& host);
} // namespace bsp::oct18_tail_b
