#pragma once
#include <cstdint>

// Exact-address lane tC leaves. Names describe hypotheses, not recovered symbols.
// Raw getters borrow the original 32-bit storage. Callback setters receive the
// actual image cell explicitly; they do not create a replacement singleton.
// Evidence and original ABIs: docs/OCT18_TAIL_C.md. No application wiring.
namespace bsp::oct18_tail_c {
const void* __fastcall accepted_invite_info_00a3e460(const void* owner);
void __fastcall enumerate_input_noop_00a90ed0(void* owner);
std::uint8_t __fastcall gui_layer_visible_00aa38e0(const void* layer);
void set_pointer_activation_reader_00aa6be0(std::uint32_t& image_f8bc84,
                                          std::uint32_t reader);
void set_selection_callback_00aab5e0(std::uint32_t& image_f8bcfc,
                                    std::uint32_t callback);
std::uint32_t __fastcall terrain_field3c_00ad56d0(const void* owner);
float __fastcall foliage_scalar0c_00ad5720(const void* owner);
std::uint32_t __fastcall foliage_bush_texture_00ad57d0(const void* owner);
std::uint32_t __fastcall foliage_bush_normal_texture_00ad57e0(const void* owner);
float __fastcall atmosphere_time_of_day_00bbdd30(const void* owner);
float __fastcall atmosphere_sky_blend_00bbdd40(const void* owner);
const void* __fastcall atmosphere_wind_direction_00bbdd60(const void* owner);
float __fastcall atmosphere_wind_speed_00bbdd70(const void* owner);
std::uint32_t __fastcall atmosphere_skybox_00bbddb0(const void* owner);
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
// Mandatory bindings to the identified original callees. No default/no-op host.
using RawThis = void (__thiscall*)(void*);
using CheckedIterator = void (__cdecl*)(); // BF6713 can return.
struct AiBindings {
    CheckedIterator checked_iterator_bf6713;
    RawThis refresh_pose_00414db0;
    const float* zero_vector_f87574;
    void* (__cdecl* active_tuning_00a371a0)();
    double (__thiscall* class_weight_009ffd70)(void*); // unspilled ST0
    void (__thiscall* torpedo_enable_0071e0d0)(void*, std::uint32_t);
    void (__thiscall* target_pass_00a13b60)(void*, float, const void*, void*, int);
    void (__thiscall* landing_00a11b80)(void*, void*);
    RawThis followers_00a10dc0;
    RawThis formation_00a11690;
    const float* close_attack_scale_ce380c;
    void* (__fastcall* entity_record_00a04560)(void*, void*, std::uint32_t);
    void (__thiscall* append_record_00a078f0)(void*, const void*);
};
std::uint32_t __fastcall active_user_slot_00a3eac0(const void* owner);
// ECX node, unused EDX, original XYZ stack argument, replaced by node+B0h.
void __fastcall node_local_position_00b6dab0(void*, void*, const void* xyz);
bool ai_group_contains_entity_00a2c720(void*, void*, CheckedIterator);
void ai_enable_torpedoes_00a11af0(void*, const AiBindings&);
void ai_close_attack_tick_00a15490(void*, const AiBindings&);
void ai_defend_position_tick_00a15500(void*, const AiBindings&);
bool ai_merge_strength_00a10d50(void*, void*, const AiBindings&);
bool ai_merge_distance_00a10c60(void*, void*, const AiBindings&);
void ai_build_entity_records_00a07e40(void*, void*, std::uint32_t, const AiBindings&);
void sound_dsp_resize_00a7a980(void*, std::int32_t,
    void (__thiscall* reserve_00a7a8b0)(void*, std::int32_t));

struct GuiTaggedWord { std::uint32_t tag; std::uint32_t value; };
static_assert(sizeof(GuiTaggedWord) == 8, "Original GUI descriptor pair");
struct GuiLineBindings {
    CheckedIterator checked_iterator_bf6713;
    void (__thiscall* describe_base_00aaaed0)(void*, void*);
    const char* points_key_cfd91c;
    void* (__cdecl* copy_pod8_0043b920)(void*, void*, void*);
    void (__thiscall* ensure_capacity_00ac2690)(void*, int);
    void* (__thiscall* geometry_00b74640)(void*, std::uint32_t);
    void* (__thiscall* element_00b732c0)(void*, std::uint32_t);
};
void gui_line_describe_00ac2090(void*, void*, const GuiLineBindings&);
void gui_line_reset_00ac2950(void*, const GuiLineBindings&);
struct GroupSphereBindings {
    RawThis refresh_world_00b6db70;
    void* (__thiscall* transform_sphere_007c1180)(void*, void*, const void*);
    RawThis aggregate_world_sphere_00b8ebe0;
};
void* group_world_sphere_00b8f100(void*, const GroupSphereBindings&);
void release_whole_file_00bd91f0(void*, void (__cdecl* free_bf6989)(void*));
struct FileStoreBindings {
    void* (__thiscall* memory_view_00bef750)(void*);
    void (__thiscall* add_file_00be7760)(void*, const void*, void*);
};
void cache_file_from_vfs_00be7ab0(void*, void*, const void*, std::uint32_t,
    const FileStoreBindings&);
} // namespace bsp::oct18_tail_c
