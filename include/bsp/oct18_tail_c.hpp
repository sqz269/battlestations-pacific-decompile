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

namespace bsp::oct18_tail_c {
void section_loaded_00abe630(void*, RawThis base_00aa7170);
void widget_describe_alias_00ac6fe0(void*, void*, void (__thiscall* base_00aaaed0)(void*, void*));
void icon_geometry_alias_00ad2df0(void*, RawThis base_00ab2540);
void render_release_alias_00b0fc00(void*, RawThis base_00b0f6e0);
struct SectionBindings {
    void* (__thiscall* base_construct_00aa9390)(void*, std::uint32_t);
    const std::uint32_t* scalar_d7a24c;
    const std::uint32_t* scalar_ce3800;
    void* (__fastcall* mesh_allocate_00b73b60)(std::uint32_t);
    void* (__thiscall* mesh_construct_00b73d70)(void*);
    const float* mesh_scalar_d7a260;
    void (__thiscall* set_geometry_00b75170)(void*, std::uint32_t, void*, float, float);
    void (__thiscall* position_00aa7dc0)(void*, const void*);
    void (__cdecl* release_string_00419cc0_00bd1510)(void*, std::uint32_t, std::uint32_t);
    RawThis base_destroy_00aa9730;
};
void* section_construct_00abf420(void*, const SectionBindings&);
void section_constructed_00abeda0(void*, const SectionBindings&);
void section_destroy_00abf4f0(void*, const SectionBindings&);
// ECX raw BF857A _CIfmod target; EDX three actual double-literal pointers:
// CE3828, CE3D18, CE3D28. Stack angle; ST0 result; RET4. No CRT substitution.
float __fastcall normalize_angle_00abe960(void*, const double* const*, float);
struct FontRegistryBindings {
    void* (__thiscall* sentinel_00ac3400)(void*);
    RawThis font_destroy_00ad53a0;
    void (__cdecl* free_bf65ac)(void*);
    CheckedIterator checked_iterator_bf6713;
};
void* font_registry_construct_00ac3690(void*, const FontRegistryBindings&);
void font_registry_destroy_00ac36f0(void*, std::uint32_t& image_f8bf44, const FontRegistryBindings&);
struct SoundCacheBindings {
    void (__thiscall* reserve_00a84450)(void*, std::int32_t);
    void* (__thiscall* sentinel_004c3020)(void*);
    RawThis destroy_record_00a842e0;
};
void sound_cache_resize_00a845a0(void*, std::int32_t, const SoundCacheBindings&);
struct GuiListVisibilityBindings {
    void (__thiscall* base_visible_00aa8530)(void*, std::uint32_t);
    void* (__cdecl* manager_004c12b0)();
    void* (__thiscall* select_widget_00aa0f50)(void*, std::uint32_t);
};
void gui_list_visible_00a9cce0(void*, std::uint32_t, const GuiListVisibilityBindings&);
void* lua_vector4_or_00b67d40(void*, void*, const void*);
struct QueryGuardScratch { void* owner; std::uint8_t taken; };
struct QueryBindings {
    std::uint8_t (__thiscall* enter_00b33ad0)(void*);
    void (__thiscall* leave_00b33b00)(void*, std::uint8_t);
    void* (__cdecl* allocate_00bf681b)(std::uint32_t);
    void* (__thiscall* construct_00b5fe90)(void*);
    void (__thiscall* reserve_00b22530)(void*, std::int32_t);
};
void* renderer_query_create_00b27c20(void*, const volatile std::uint8_t& image_108d6dc,
    QueryGuardScratch&, const QueryBindings&);
struct WarningIteratorPair { void* owner; void* node; };
struct WarningRemoveBindings {
    void* (__thiscall* channel_00980150)(void*, const void*);
    WarningIteratorPair* (__thiscall* find_0096d6f0)(void*, WarningIteratorPair*, std::uint32_t);
    WarningIteratorPair* (__thiscall* erase_009713d0)(void*, WarningIteratorPair*, void*, void*);
    CheckedIterator checked_iterator_bf6713;
};
void warning_remove_listener_00980d30(void*, const void*, std::uint32_t, const WarningRemoveBindings&);
struct WholeFileBindings {
    void (__thiscall* string_resize_0041dd40)(void*, std::uint32_t, std::uint32_t);
    void (__cdecl* release_string_00419cc0_00bd1510)(void*, std::uint32_t, std::uint32_t);
    void* (__cdecl* allocate_00bf55be)(std::uint32_t);
};
void* vfs_load_whole_file_00bdc8b0(void*, const void*, std::uint32_t*, const WholeFileBindings&);
} // namespace bsp::oct18_tail_c

namespace bsp::oct18_tail_c {
struct ArchiveFieldBindings {
    void (__cdecl* indent_00bd6990)(std::uint32_t);
    void (__cdecl* key_00bd7b30)(std::uint8_t, GuiTaggedWord);
    void (__cdecl* value_00bd7f50)(GuiTaggedWord);
    void (__cdecl* terminator_00bd6ac0)(std::uint8_t);
};
void archive_write_field_00bd8cd0(void*, GuiTaggedWord, GuiTaggedWord, const ArchiveFieldBindings&);
struct MpkgBindings {
    void* (__cdecl* allocate_header_00bf55be)(std::uint32_t);
    void (__cdecl* free_header_00bf65ac)(void*);
    void* (__cdecl* allocate_owner_00bf681b)(std::uint32_t);
    void* (__thiscall* inflate_00bbc1d0)(void*, void*, const std::uint32_t*, std::uint32_t, std::uint32_t);
    void* (__thiscall* memory_00bef750)(void*);
    void* (__thiscall* copy_range_00bef840)(void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
    void* (__thiscall* adopted_00bf1130)(void*, void*, std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t);
    int (__cdecl* stricmp_00bf7fbf)(const char*, const char*);
};
std::uint32_t mpkg_entry_offset_00bb8a90(void*, void*, const MpkgBindings&);
void* mpkg_open_entry_00bb8be0(void*, void*, std::uint32_t, void* volatile& image_109ceec, const MpkgBindings&);
void* mpkg_open_named_00bb8d60(void*, const void*, std::uint32_t, void* volatile& image_109ceec, const MpkgBindings&);
struct VoiceBindings {
    std::uint8_t (__thiscall* accept_00a43f40)(void*, std::uint32_t, std::uint32_t);
    std::uint8_t (__thiscall* state_00687860)(void*, std::uint32_t, std::uint32_t);
    void (__thiscall* incoming_00a49040)(void*, const void*, std::uint32_t, const void*);
    void (__thiscall* mute_00a49180)(void*, std::uint32_t, std::uint32_t);
    CheckedIterator checked_iterator_bf6713;
};
void voice_packet_00a44000(void*, const void*, void* volatile& image_e198b4, const VoiceBindings&);
void voice_refresh_flags_00a44150(void*, std::uint8_t, const VoiceBindings&);
struct TextClipBindings {
    int (__thiscall* has_geometry_00b74650)(void*);
    void* (__thiscall* geometry_00b74640)(void*, std::uint32_t);
    std::uint32_t (__thiscall* count_00b72b40)(void*);
    void* (__thiscall* element_00b732c0)(void*, std::uint32_t);
    void (__thiscall* clip_00aa9f10)(void*, void*);
    CheckedIterator checked_iterator_bf6713;
};
void text_register_clip_00ab7a40(void*, const TextClipBindings&);
struct LayerReadBindings {
    const char* priority_d5cb00;
    const char* render_order_d5caf4;
    void (__thiscall* register_root_00b6d890)(void*, void*);
    void (__thiscall* position_00aa7dc0)(void*, const void*);
    void (__thiscall* read_base_00aaa710)(void*, void*);
};
void layer_read_properties_00ac4c50(void*, void*, std::uint32_t& priority_preimage, const LayerReadBindings&);
struct GuiDestroyBindings {
    CheckedIterator checked_iterator_bf6713;
    RawThis clear_strings_00aa51d0;
    void (__cdecl* free_00bf65ac)(void*);
    void* (__thiscall* erase_tree_00aa49e0)(void*, void*, void*, void*, void*, void*);
};
void gui_manager_destroy_00aa6340(void*, std::uint32_t& image_f8bc5c, const GuiDestroyBindings&);
struct PlatformBindings {
    void* volatile& sound_f8bbd8;
    void* volatile& renderer_f8d394;
    void* volatile& render_service_f8d39c;
    void (__stdcall* xinput_enable_00c2f172)(int);
    RawThis sound_pause_00a7a480;
    RawThis sound_resume_00a7a4a0;
    void* (__cdecl* media_get_004c1710)();
    void (__thiscall* media_pause_00a4c2d0)(void*, std::uint32_t);
    void* (__cdecl* gui_get_004c12b0)();
    void (__thiscall* gui_pause_00aa33a0)(void*, std::uint32_t);
    RawThis reload_renderer_00b24fb0;
    RawThis refresh_render_service_00b0d1e0;
    void (__thiscall* append_text_00bed370)(void*, const void*);
};
std::intptr_t platform_handle_message_00bed3b0(void*, std::uintptr_t, std::uint32_t,
    std::uint32_t, std::intptr_t, const PlatformBindings&);
} // namespace bsp::oct18_tail_c
