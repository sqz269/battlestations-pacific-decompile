#pragma once
#include "bsp/native_legacy_sbo_string.hpp"
#include <cstdint>
#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/cc13_seg48U.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/native_damageable_class_binding.hpp"
#include "bsp/net_entity_create_cc13.hpp"
namespace bsp::cc13_big53 { struct Big53Host; }
namespace bsp::cc13_left8b { class Left8bHost; }

namespace bsp { struct NativeStringRawPoolContext; struct NativeLuaObjectStorage; }

// oct18-tA: image layout, new source interfaces. See docs/OCT18_TAIL_A.md.
// Names describe behavior; they are not recovered symbols or binary replacements.
namespace bsp::oct18_tail_a {
std::uint32_t air_ops_block_kind_006ca5b0() noexcept;
void air_ops_site_platform_tick_006cebe0(void* self, std::uint32_t argument) noexcept;
void air_ops_site_slot50_006cec10(void* self, std::uint32_t a, std::uint32_t b,
                                std::uint32_t c) noexcept;
std::uint8_t airfield_site_slot30_006cf3f0() noexcept;
void mothership_site_slot14_006cfe30(void* self, std::uint32_t argument) noexcept;
void* air_ops_block_member_006d0d20(void* self) noexcept;
void* airfield_weapon_director_006d0d30(const void* self) noexcept;
void unit_slot1a8_006d1640() noexcept;
std::uint32_t airfield_class_recon_category_006d1d20() noexcept;
std::uint8_t airfield_slotb4_006d1d40() noexcept;
void airfield_apply_sync_message_006d1d60(void* self, std::uint32_t argument) noexcept;
std::uint32_t air_ops_block_kind_006d1d70() noexcept;
std::uint32_t unit_slot190_006d1de0() noexcept;
std::uint32_t unit_slot1d4_006d1ee0() noexcept;
std::uint8_t unit_slot1e4_006d1f40(const volatile std::uint8_t& global_00e17bf2) noexcept;
void unit_slot1ec_006d1f90(void* self, std::uint32_t argument) noexcept;
const char* command_class_base_name_006f7f30() noexcept;

void* plane_chase_camera_target_0042abd0(const void* self) noexcept;
void* bomb_view_camera_target_00435390(const void* self) noexcept;
void* ship_camera_target_00519340(const void* self) noexcept;
void* construct_borrowed_key_00438f80(void* self, const char* text) noexcept;
float weapon_scale_006e2850(const void* self) noexcept;
void* hud_controlled_unit_00644230(void* const volatile& global_00e188d8) noexcept;
void trace_message_004c9c90(const char* message) noexcept;
void* checkpoint_record_construct_00436690(void* self, NativeStringRawPoolContext& strings);
void* checkpoint_record_copy_construct_00437490(void* self, const void* source,
                                             NativeStringRawPoolContext& strings);
void* traffic_string_pair_construct_0048e1f0(void* self, const void* first,
                                          const void* second, NativeStringRawPoolContext& strings);
void* traffic_string_pair_copy_construct_0048e500(void* self, const void* source,
                                               NativeStringRawPoolContext& strings);
void entity_init_thunk_004a6b90(cc13_seg7::Seg7Host& host, cc13_seg7::Object self);
void entity_init_thunk_004ab030(cc13_seg7::Seg7Host& host, cc13_seg7::Object self);
// Same explicit screen-id boundary as the existing typed 004F71D0 target.
void frontend_register_thunk_0054ed00(FrontEndScreenTable& table, FrontEndScreen& screen,
                                    int screen_id) noexcept;
void inherit_race_thunk_0060c610(void* self) noexcept;
void catapult_init_thunk_006eb600(void* self, Cc13Seg48UHost& host);

struct WorldListServices {
    virtual ~WorldListServices() = default;
    virtual void register_parent_00928560(void* self) = 0;
    virtual void unregister_parent_00928570(void* self) = 0;
    virtual void push_back_00484540(void* list, void* entity) = 0;
    virtual void* erase_004837d0(void* list, void* node) = 0;
};
void register_world_lists_004f1870(void* self, WorldListServices& services);
void unregister_world_lists_004f1890(void* self, WorldListServices& services);

struct DialogIterator { void* owner; void* node; };
struct DialogServices {
    virtual ~DialogServices() = default;
    // Required CRT/STL bindings; no fallback implementations.
    virtual void invalid_parameter_00bf6713() = 0; // May return; no no-return assumption.
    virtual void append_native_string_00450540(void* vector, const void* key) = 0;
    virtual void increment_tree_iterator_00448c10(DialogIterator& iterator) = 0;
};
void collect_active_dialog_ids_00450750(void* self, void* output, DialogServices& services);
// The remaining routines use borrowed original-layout storage and explicit dependencies.
struct GridConstants {
    const volatile float& extent_00ce3968;
    const volatile double& diameter_00ce3960;
    const volatile float& multiplier_00ce3958;
    const volatile float& default_width_00ce396c;
};
struct GridServices {
    virtual ~GridServices() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void resize_bytes_004219a0(void* vector, std::uint32_t size, std::uint8_t fill) = 0;
};
void terrain_layer_set_dimension_00423960(void* self, std::int32_t dimension,
                                        const GridConstants& constants, GridServices& services);
void reset_avoid_zone_layers_00423af0(void* self, const GridConstants& constants,
                                   GridServices& services);
struct PendingClassState {
    void* vector_00f8a09c;
    void* volatile& head_00f8a0b0;
    std::uint32_t& count_00f8a0b4;
};
void clear_pending_class_ids_004c8aa0(PendingClassState& state,
                                    const SingletonLifetimeCallbacks& callbacks);
struct AirOpsServices {
    virtual ~AirOpsServices() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void remove_ready_front_006c4a70(void* container) = 0;
    virtual void unregister_observer_006952a0(void* observed, void* observer) = 0;
    virtual void register_observer_00694a60(void* observed, void* observer) = 0;
    virtual void scene_enable_00922f30(void* plane, std::uint32_t enabled) = 0;
};
void pull_ready_plane_006c6540(void* self, std::uint32_t unused_stack_word,
                             AirOpsServices& services);
struct HudServices {
    virtual ~HudServices() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void delete_slot4(void* object, std::uint32_t flags) = 0;
    virtual void build_marker_00642040(void* screen, void* entity, std::uint32_t a,
        std::uint32_t b, std::uint32_t c, std::uint8_t nonzero_kind, std::uint32_t kind) = 0;
};
void hud_root_exit_00646c50(void* self, NativeStringRawPoolContext& strings,
                          HudServices& services);
void add_subunit_markers_00642b80(void* screen, void* unit, std::uint32_t a,
                                std::uint32_t b, std::uint32_t c,
                                std::int32_t flag, HudServices& services);
void place_in_world_thunk_006dfe40(entity_mission_seg60_argon::Seg60Host& host,
    void* self, void* hierarchy_parent, void* world_node, const void* matrix);
void damageable_class_bind_thunk_006eb7c0(void* self,
    const NativeDamageableClassBindingContext& context);
void write_entity_create_thunk_00767d70(const net_cc13::EntityCreateMessage& message,
    NativeBitCursor* cursor, const NativeBitNumericContext& numeric);
void mothership_bind_model_00759120(void* self, cc13_big53::Big53Host& base,
                                  const SingletonLifetimeCallbacks& callbacks);
struct MovieState {
    void* const volatile& player_00e18d48;
    void* const volatile& sound_owner_00e198ac;
    const char* const volatile* names_00e084b8;
    const volatile float& value_00ceb364;
};
struct MovieServices {
    virtual ~MovieServices() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual std::uint8_t is_playing_004f89a0(void* player) = 0;
    virtual void play_004f8a20(void* player, const void* path, std::uint32_t mode,
                              float value, std::uint32_t flag) = 0;
    virtual void completion_004f8970(void* player, std::uint32_t image_callback) = 0;
    virtual void request_fade_00a85c00(void* stream) = 0;
};
void play_library_movie_004ff580(void* self, MovieState& state,
                               NativeStringRawPoolContext& strings, MovieServices& services);

// Deferred packet: complete ordinary functions, explicit required target contracts.
struct DeferredTargetServices {
    virtual ~DeferredTargetServices() = default;
    // Required WHOLE targets. Do not bind the known partial health/damage host projections.
    virtual void health_changed_00958a30(void* self) = 0;
    virtual void gun_setup_0072e6d0(void* self) = 0;
};
void health_changed_thunk_006d0d40(void* self, DeferredTargetServices&);
void gun_setup_thunk_006e35a0(void* self, DeferredTargetServices&);
struct TrafficRecordRange {
    void* const volatile& begin_00f8a088;
    void* const volatile& end_00f8a08c;
};
struct TrafficBuildServices {
    virtual ~TrafficBuildServices() = default;
    virtual void invalid_parameter_00bf6713() = 0;
    virtual void** map_subscript_00499320(void* map, void*& record_key) = 0;
    virtual void* allocate_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void*) noexcept = 0;
};
void build_traffic_runtime_004a5620(void* self, TrafficRecordRange&,
    TrafficBuildServices&, cc13_left8b::Left8bHost& constructor);
struct MissionAssignServices {
    virtual ~MissionAssignServices() = default;
    virtual void string_vector_assign_00506c80(void* destination, const void* source) = 0;
    virtual void pointer_vector_assign_004f60e0(void* destination, const void* source) = 0;
    virtual void picture_destroy_slot0(void* picture) = 0;
    virtual void header_assign_005c5470(void* destination, const void* source) = 0;
    virtual void erase_tree_00576740(void* tree, DialogIterator& output,
                                   const DialogIterator& first, const DialogIterator& last) = 0;
    virtual void copy_tree_005c6790(void* destination, const void* source) = 0;
};
void* assign_mission_record_005c7b80(void* self, const void* source,
                                  NativeStringRawPoolContext&, MissionAssignServices&);
struct InitOnceState {
    void* settings_00f88980;
    void* const volatile& backend_00f8bbf4;
    std::uint32_t& input_callback_00f8bbfc;
    const volatile std::uint8_t& compatibility_00f88a30;
    const volatile std::uint8_t& safe_zone_00f88a31;
};
struct InitOnceServices {
    virtual ~InitOnceServices() = default;
    virtual void file_block_construct_00be0a30(void* block, const void* name, std::uint32_t mode) = 0;
    virtual void file_block_destroy_00bdcb30(void* block) = 0;
    virtual void mission_lua_initialize_00884be0(void* mission_host) = 0;
    virtual void borrowed_lua_construct_00b65e20(void* owner, void* state) = 0;
    virtual void lua_script_00b69d40(void* owner, const void* path, std::uint32_t flags) = 0;
    virtual void compatibility_008d44c0(void* settings, std::uint32_t enabled) = 0;
    virtual void configure_input_00a917e0(void* backend, std::uint32_t kind,
                                      std::uint32_t enabled, void* argument) = 0;
    virtual void pointer_reader_00aa6be0(std::uint32_t image_callback) = 0;
    virtual void keyboard_reader_00a9ac60(std::uint32_t image_callback) = 0;
    virtual void* sound_queue_004c1b90() = 0;
    virtual void sound_set_initialize_00940ec0(void* queue) = 0;
    virtual void selection_reader_00a9ac80(std::uint32_t image_callback) = 0;
    virtual void button_selection_reader_00aab5e0(std::uint32_t image_callback) = 0;
    virtual void* gui_manager_004c12b0() = 0;
    virtual void safe_zone_00aa2110(void* manager, std::uint32_t visible) = 0;
    virtual void input_configuration_00698a10(void* settings) = 0;
    virtual void* menu_command_00425d10() = 0;
    virtual void* frontend_frame_004c1ac0() = 0;
    virtual void register_slot10(void* object) = 0;
    virtual void game_singleton_004c1c50() = 0;
    virtual void init_title_004c9a70(void* game) = 0;
    virtual void* resource_cache_004dc200() = 0;
    virtual void preload_cache_008fafc0(void* cache) = 0;
};
void game_on_init_once_004dd5b0(void* self, std::uint8_t initialize,
                              InitOnceState&, NativeStringRawPoolContext&, InitOnceServices&);

struct ProfileLuaServices {
    virtual ~ProfileLuaServices() = default;
    virtual std::int32_t type_00b65f80(const NativeLuaObjectStorage&) = 0;
    virtual double floor_00bf85b0(double value) = 0; // Bound CRT floor, not a game-body reconstruction.
    virtual void boolean_index_00b66530(NativeLuaObjectStorage&, std::int32_t key, std::uint32_t value) = 0;
    virtual void integer_index_00b665d0(NativeLuaObjectStorage&, std::int32_t key, std::int32_t value) = 0;
    virtual void string_index_00b66740(NativeLuaObjectStorage&, std::int32_t key, const char* value) = 0;
    virtual void table_index_00b66670(NativeLuaObjectStorage&, std::int32_t key) = 0;
};
struct ProfileLuaContext {
    NativeStringRawPoolContext& strings;
    const bool& crt_sse2_conversion;
    const char* empty_0108ff2c;
    ProfileLuaServices& services;
};
void copy_profile_lua_value_00436c50(void* manager, NativeLuaObjectStorage& destination,
    NativeLuaObjectStorage& key, NativeLuaObjectStorage& value, ProfileLuaContext&);

struct SessionFactoryRoute {
    std::uint32_t tag, allocation_bytes, callee, stack_argument, stack_argument_count;
    std::int32_t unwind_state;
    std::uint32_t result_register; // 0 EAX, 1 EDX; provider returns that captured pointer.
    std::uint32_t vtable_stamp, native_case_entry, native_call_site;
};
struct SessionFactoryServices {
    virtual ~SessionFactoryServices() = default;
    virtual void read_byte_00428c70(void* cursor, std::uint8_t& output, std::uint32_t bits) = 0;
    virtual void rewind_00428b80(void* cursor, std::uint32_t bits) = 0;
    virtual void* allocate_00bf681b(std::uint32_t bytes) = 0;
    virtual void free_00bf65ac(void* allocation) noexcept = 0;
    // Required constructor binding: ECX allocation, route's original stacked
    // argument (if any), original side effects, and selected EAX/EDX result.
    // This does not claim to implement the 203 constructor targets themselves.
    virtual void* construct(const SessionFactoryRoute& route, void* allocation) = 0;
    virtual void* direct_getter_00733720() = 0;
    virtual void* direct_getter_008e1530() = 0;
    // Target is read from the actual current message vtable+8; bind that override.
    virtual void read_slot8(void* message, std::uint32_t target, void* stream) = 0;
};
// Unsupported tags select the native uninitialized [entryESP-10h] word. Its
// source preimage must be supplied explicitly; zero is not a recovered default.
void* session_message_from_bitstream_00768530(void* stream, void* default_message_preimage,
                                           SessionFactoryServices&);

struct BlastServices {
    virtual ~BlastServices() = default;
    virtual void invalid_00bf6713() = 0; // May return; callers then reload original fields.
    virtual bool class_slot5c(void* entity, std::uint32_t class_id) = 0;
    virtual void pose_00414db0(void* entity) = 0;
    virtual const void* multiply_00413920(void* left, void* output, const void* right) = 0;
    virtual void stop_00480bf0(void* entity) = 0;
    virtual std::uint32_t random_00bd2fc0() = 0;
    virtual void play_004b0f80(void* entity, const void* effect, std::uint32_t, std::uint32_t) = 0;
    virtual void cluster_00480c20(void* cluster, float value) = 0;
    virtual void tree_next_00427fe0(DialogIterator&) = 0;
};
struct BlastContext {
    void* const& game_00e188a8;
    const float& scalar_00ce3930;
    // Required raw binding: ECX point, EDX minimum, stacked maximum, RET4, ST0.
    void (__cdecl* distance_0085bf90)();
    BlastServices& services;
};
void blast_sites_00428800(const float* center, float radius, std::uint32_t unused, BlastContext&);

struct PointAnnouncementServices {
    virtual ~PointAnnouncementServices() = default;
    virtual void base_construct_00496920(void* self, std::uint32_t argument) = 0;
    virtual void* parts_004aed50(void* definition, std::uint8_t enabled) = 0;
    virtual void resize_00495e20(void* vector, std::uint32_t count, const void* stack_preimage_76_bytes) = 0;
    virtual void* pick_004ab9a0(void* table) = 0;
    virtual void invalid_00bf6713() = 0;
    virtual void vector_destroy_0048e160(void* vector) noexcept = 0;
    virtual void base_destroy_00489dd0(void* self) noexcept = 0;
};
struct PointAnnouncementContext {
    // Raw random ABI: ECX zero stream, stacked minimum/maximum, RET8, ST0 result.
    void (__cdecl* random_00bd2f10)();
    std::int32_t (__cdecl* truncate_st0_00bf7420)();
    const float &angle_min_00ce684c, &angle_max_00d7a264;
    const float &sign_min_00ce6848, &sign_max_00ce38b8;
    PointAnnouncementServices& services;
};
void* construct_point_announcement_0049c000(void* self, std::uint32_t base_argument,
    const float* center, const float* velocity, void* definition,
    const void* stack_preimage_76_bytes, PointAnnouncementContext&);

struct FrontendShellState {
    void* const& renderer_00f8d394;
    void* const& sound_00f8bbd8;
    void*& title_00e198c8;
    void*& options_00e198b8;
    void*& main_00e198ac;
    void*& multiplayer_00e198b4;
    void* const& game_00e188a8;
    void* const& achievements_00e19900;
    void* const& live_00f8abe8;
    const float& progress_00d7a2f0;
};
struct FrontendShellServices {
    virtual ~FrontendShellServices() = default;
    virtual void renderer_slot70(void*,const char*) = 0;
    virtual void sound_stats_00a7a460(void*,const char*) = 0;
    virtual void* effects_004c1650() = 0;
    virtual void effects_stats_0086b0b0(void*,const char*) = 0;
    virtual void delete_slot0(void*,std::uint32_t) = 0;
    virtual void file_construct_00be0a30(void*,const void*,std::uint32_t) = 0;
    virtual void file_destroy_00bdcb30(void*) = 0;
    virtual void* default_config_0057d0d0(void*) = 0;
    virtual void publish_config_0057cff0(void*) = 0;
    virtual void config_destroy_004d2a80(void*) = 0;
    virtual void loading_begin_0057cb60(std::uint32_t) = 0;
    virtual void progress_0057bec0(float) = 0;
    virtual void on_init_004e3aa0(void*) = 0;
    virtual void platform_events_004db290(void*) = 0;
    virtual void loading_end_0057c250() = 0;
    virtual void* allocate_00bf681b(std::uint32_t) = 0;
    virtual void free_00bf65ac(void*) noexcept = 0;
    virtual void* menu_construct(std::uint32_t target,void*) = 0; // 689800/686170/6887E0, ECX allocation.
    virtual void menu_slot4(void*) = 0;
    virtual void lua_run_006b8ad0(void*,const char*,std::uint32_t,std::uint32_t,std::uint32_t) = 0;
    virtual void interface_request_004cc460(void*,std::uint32_t,std::uint32_t) = 0;
    virtual void menu_slot8(void*) = 0;
    virtual void title_music_005884a0(void*) = 0;
    virtual bool pending_invite_0067d6e0() = 0;
    virtual void raise_multiplayer_004bfc70(void*) = 0;
    virtual DialogIterator* achievement_find_004c8b80(void*,DialogIterator&,const void*) = 0;
    virtual void invalid_00bf6713() = 0;
    virtual bool midway_folder_0090c5d0(void*) = 0;
    virtual std::uint32_t achievement_id_006b8da0(void*,const void*) = 0;
    virtual bool live_enabled_00a3e520(void*) = 0;
    virtual bool signed_in_004b44f0(void*) = 0;
    virtual void grant_00a410a0(void*,std::uint32_t) = 0;
    virtual void record_award_007fbe20(void*,const void*,std::uint32_t) = 0;
    virtual void session_mode_0076fad0(void*,std::uint32_t) = 0;
};
void enter_frontend_shell_004e4000(void* self,const void* config_stack_preimage_28_bytes,
    FrontendShellState&,NativeStringRawPoolContext&,FrontendShellServices&);

struct PlayerDropStackPreimage {
    std::uint8_t active[8]; float weights[8]; std::uint32_t list_allocator_word;
    std::uint8_t message[36];
};
struct PlayerDropState {
    void* const& game_00e188a8; void* const& unit_head_00f87198;
    const float &unassigned_00d7a218, &minimum_00d7a244;
};
struct PlayerDropServices {
    virtual ~PlayerDropServices() = default;
    virtual void diagnostic_start_004254b0() = 0;
    virtual float player_weight_00946f00(std::uint32_t player_in_ecx) = 0;
    virtual void* list_head_004c1630(void* list) = 0;
    virtual void* list_node_004c2220(void* list,void* next,void* previous,const void* payload) = 0;
    virtual void grow_004cee30(void* list,std::uint32_t count) = 0;
    virtual void invalid_00bf6713() = 0;
    virtual std::uint32_t unit_name_slot10(void*) = 0;
    virtual void diagnostic_value_004254b0(std::uint32_t name,double value) = 0;
    virtual void diagnostic_move_004254b0(std::uint32_t name,std::uint32_t party,
        std::uint32_t old_player,std::uint32_t new_player) = 0;
    virtual void unit_slot148(void*,std::uint32_t value,std::uint32_t reason) = 0;
    virtual void message_base_0075b430(void*,std::uint32_t type) = 0;
    virtual void message_unwind_004b5ce0(void*) noexcept = 0;
    virtual void unit_message_0077c2a0(void* unit,const void* message,std::uint32_t,std::uint32_t) = 0;
    virtual void assign_slots_004c3840(void* game,std::uint32_t notify) = 0;
    virtual void list_unwind_004c8070(void*) noexcept = 0;
    virtual void free_00bf65ac(void*) noexcept = 0;
};
void reassign_units_on_drop_004d56e0(void* self,const PlayerDropStackPreimage&,
    PlayerDropState&,PlayerDropServices&);

struct CommandLineStackPreimage {
    NativeLegacySboStringStorage strings[8];
    std::uint32_t token_vector[4], scratch;
    char directory[260];
};
struct CommandLineState {
    void* const& game_00e188a8; void* const& storage_0109cecc;
    const char* empty_00e18b1c; const void* clear_bytes_00ce3a0c;
    std::uint8_t &singlethread_00e0b6ce,&flaktest_00e19b4d,&vehiclelog_00e0cfc0;
    std::uint32_t& hints_00f8898c;
};
struct CommandLineServices {
    virtual ~CommandLineServices() = default;
    virtual void split_0094ec70(void* output,const NativeLegacySboStringStorage&,const char*) = 0;
    virtual void vector_unwind_004d6800(void*) noexcept = 0;
    virtual void range_destroy_004ce520(void* first,void* end,void* owner,std::uint32_t scratch) = 0;
    virtual void free_00bf65ac(void*) noexcept = 0;
    virtual void invalid_00bf6713() = 0;
    virtual std::int32_t compare_bytes_004b3fc0(const char*,const char*,std::uint32_t) = 0;
    virtual std::int32_t compare_004beb60(const NativeLegacySboStringStorage&,std::uint32_t,
        std::uint32_t,const char*,std::uint32_t) = 0;
    virtual bool matches_004c2df0(const NativeLegacySboStringStorage&,const char*) = 0;
    virtual char* character_004bcb10(NativeLegacySboStringStorage&,std::uint32_t) = 0;
    virtual std::int32_t atol_00bf8417(const char*) = 0;
    virtual void* class_registry_00437f50() = 0;
    virtual std::uint32_t difficulty_007f7bf0(const void* native_string) = 0;
    virtual void iterator_add_004bcec0(DialogIterator&,std::uint32_t) = 0;
    virtual DialogIterator* postincrement_004bce80(DialogIterator&,DialogIterator&,std::uint32_t) = 0;
    virtual DialogIterator* end_004c44d0(void* vector,DialogIterator&) = 0;
    virtual bool iterator_not_equal_004bcf00(DialogIterator&,const DialogIterator&) = 0;
    virtual NativeLegacySboStringStorage* iterator_get_004bcdf0(DialogIterator&) = 0;
    virtual void push_command_004d6f30(void* game,const NativeLegacySboStringStorage&) = 0;
    virtual void lowercase_004bcc00(void* native_string) = 0;
    virtual const char* strstr_00bf9440(const char*,const char*) = 0;
    virtual NativeLegacySboStringStorage* concatenate_004d0860(NativeLegacySboStringStorage&,
        const NativeLegacySboStringStorage&,const NativeLegacySboStringStorage&) = 0;
    virtual std::uint32_t current_directory(std::uint32_t,char*) = 0; // IAT00CE2264
    virtual std::int32_t tolower_00bf9611(std::int32_t) = 0;
    virtual void console_label_004c89a0(void* game,const void* native_string) = 0;
    virtual void pending_scene_004e2770(void* game,const char*,const char*) = 0;
    virtual void command_text_004c8950(void* game,const char*) = 0;
    virtual void newest_slot_004c5e00(void* storage,void* output_native_string) = 0;
    virtual void* empty_save_00436710(void* output_native_string) = 0;
    virtual bool string_not_equal_00449af0(const void*,const void*) = 0;
    virtual void request_read_007ff100(void* profile,const void* name,std::uint32_t callback) = 0;
    virtual void enqueue_004d3ed0(void* queue,const std::uint32_t*) = 0;
    virtual void build_record_004e1d70(void* game,const char*,std::uint32_t,std::uint32_t,const char*) = 0;
    virtual void select_record_004c6890(void* game,std::uint32_t) = 0;
    virtual void reset_participants_004bb160(void* game) = 0;
    virtual void game_mode_004bc890(void* game,std::uint32_t,std::uint32_t) = 0;
    virtual void duel_005d4a00(std::uint32_t ecx) = 0;
};
void apply_command_line_004e27e0(void* self,const CommandLineStackPreimage&,
    CommandLineState&,NativeStringRawPoolContext&,CommandLineServices&);

struct HudRowsState {
    void* const& controlled_00e188d8;void* const& screens_00e198c4;
    void* const& modifiers_00f88c30;
    const float &one_00d7a24c,&ellipsis_00d7a260;
    // Actual descriptor identities corresponding to E08F10,18,...,90,A0.
    const void* commands[18];
};
struct HudRowsServices {
    virtual ~HudRowsServices() = default;
    virtual void rebuild_00648290(void* root) = 0;
    virtual void* selected_00644a60(void* root) = 0;
    virtual bool class_slot5c(void* unit,std::uint32_t) = 0;
    virtual void assign_text_0041e350(void* header,const char*) = 0;
    virtual bool ordnance(void* unit,std::uint32_t target,std::uint32_t argument) = 0;
    virtual std::uint32_t count(void* unit,std::uint32_t target) = 0;
    virtual void* concat_int_004263b0(const void* left,void* output,std::uint32_t value) = 0;
    virtual void* concat_004261a0(const void* left,void* output,const void* right) = 0;
    virtual bool not_equal_00449af0(const void* left,const void* right) = 0;
    virtual void widget_slot4c(void* widget,float) = 0;
    virtual void* timed_00aa8b00(void* widget,std::uint32_t key) = 0;
    virtual void ticker_00ac2f20(void*,float,float) = 0;
    virtual void delete_timed_00aa8b80(void*,std::uint32_t key) = 0;
    virtual void localized_00abaed0(void*,const void* text,std::uint32_t) = 0;
    virtual bool local_role_00927f30(void*,std::uint32_t) = 0;
    virtual void* director_slot114(void* unit) = 0;
    virtual const void* command_0071be40(void* director) = 0;
    virtual void widget_slot88(void*,std::uint32_t frame,std::uint32_t,float) = 0;
    virtual const char* unit_name_slot14(void*) = 0;
    virtual void ellipsized_00abb000(void*,const void* text,float,std::uint32_t) = 0;
    virtual void widget_slot34(void*,std::uint32_t) = 0;
    virtual std::uint32_t icon_00653200(void* selector,std::uint32_t payload) = 0;
    virtual void widget_slot48(void*,const float* pair) = 0;
    virtual float airfield_health_006d2560(void*) = 0;
    virtual void* settings_00424c40() = 0;
};
struct HudRowsContext {
    // Required raw bindings: health ECX unit, RET/ST0; modifier ECX manager,
    // stack(kind,unit), RET8/ST0. Kernels consume ST0 before any source spill.
    void (__cdecl* health_00923be0)();
    void (__cdecl* modifier_00470440)();
    HudRowsState& state;NativeStringRawPoolContext& strings;HudRowsServices& services;
};
void update_hud_rows_00648c20(void* self,HudRowsContext&);

struct WorldConstructServices {
    virtual ~WorldConstructServices() = default;
    // Bind actual original primitive targets. Stack words are in native entry
    // order (ESP+4 first), float words keep their bits, EAX is returned. Restore
    // the target's original ECX/EDX/stack convention in the provider; this new
    // interface is not a binary-ABI adapter and has no fallback implementation.
    virtual std::uint32_t invoke(std::uint32_t target,std::uint32_t ecx,std::uint32_t edx,
                                const std::uint32_t* stack,std::uint32_t words) = 0;
    // Actual borrowed image cells/data, not a synthesized global map or heap.
    virtual std::uint32_t& word(std::uint32_t address) = 0;
    virtual std::uint8_t& byte(std::uint32_t address) = 0;
    virtual const void* image(std::uint32_t address) = 0;
};
void construct_world_004de610(void* self,NativeStringRawPoolContext&,WorldConstructServices&);

} // namespace bsp::oct18_tail_a
