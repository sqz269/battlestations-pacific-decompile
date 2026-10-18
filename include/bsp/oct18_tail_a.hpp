#pragma once
#include <cstdint>
#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/cc13_seg48U.hpp"
#include "bsp/frontend_state_machine.hpp"
#include "bsp/entity_mission_seg60_argon.hpp"
#include "bsp/native_damageable_class_binding.hpp"
#include "bsp/net_entity_create_cc13.hpp"
namespace bsp::cc13_big53 { struct Big53Host; }

namespace bsp { struct NativeStringRawPoolContext; }

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

} // namespace bsp::oct18_tail_a
