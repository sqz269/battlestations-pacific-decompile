#pragma once
#include <cstdint>
#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/cc13_seg48U.hpp"
#include "bsp/frontend_state_machine.hpp"

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
} // namespace bsp::oct18_tail_a
