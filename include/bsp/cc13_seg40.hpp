#pragma once

// cc13_seg40: segment 40 (00728A90-00735EC0, weapon classes: barrels, torpedo,
// load time) leftovers (packet cc13-seg40, docs/CC13_SEG40.md). Same
// conventions as cc13_seg48: every name is a hypothesis, not a recovered
// symbol; native objects are raw byte blocks touched only at the offsets the
// image uses; every call goes through a named host method at the same point
// with the same arguments. Cc13Seg40Host extends Cc13Seg48Host, whose hooks
// (CRT, references, bit cursors, session-message base) are reused. Nothing is
// wired into the game. Build-tested only.

#include "bsp/cc13_seg48.hpp"

#include <cstdint>

namespace bsp {

class Cc13Seg40Host : public Cc13Seg48Host {
public:
    // ---- entity kinds and gun bots ----
    // 00521EA0 BSP_CommandTarget_ResolveObject: __thiscall(ECX = target), EAX.
    virtual void* command_target_resolve_object_00521ea0(void* target) = 0;
    // Gun bot slot +30h, no arguments.
    virtual void vcall_30(void* object) = 0;

    // ---- net messages ----
    // 00765000 / 00767E30 BSP_NetEntityCreateMessage_Write / _Read:
    // __thiscall(ECX = message, cursor or stream), RET 4.
    virtual void net_entity_create_message_write_00765000(void* message, void* cursor) = 0;
    virtual void net_entity_create_message_read_00767e30(void* message, void* stream) = 0;
    // 0075B480 / 0075B4C0 BSP_SessionMessage_Write/ReadExtendedHeader:
    // __thiscall(ECX = message, cursor or stream), RET 4.
    virtual void session_message_write_extended_header_0075b480(void* message, void* cursor) = 0;
    virtual void session_message_read_extended_header_0075b4c0(void* message, void* stream) = 0;
    // Bit-cursor writers, __thiscall(ECX = cursor, ...):
    // 00429140 WriteWordArray(const word*, count, bits), RET 0Ch;
    // 004290B0 WriteBoolean(zero-extended byte), RET 4;
    // 004295C0 WriteNumericFloat(float value, int, int, float max, int bits), RET 14h;
    // 00429070 WriteUnsignedDwordBits(dword, bits), RET 8;
    // 00429030 WriteSignedWordBits(zero-extended word, bits), RET 8.
    virtual void bit_cursor_write_word_array_00429140(void* cursor, const void* words, std::int32_t count,
                                                      std::int32_t bits) = 0;
    virtual void bit_cursor_write_boolean_004290b0(void* cursor, std::uint32_t value) = 0;
    virtual void bit_cursor_write_numeric_float_004295c0(void* cursor, float value, std::int32_t a, std::int32_t b,
                                                         float max, std::int32_t bits) = 0;
    virtual void bit_cursor_write_unsigned_dword_bits_00429070(void* cursor, std::uint32_t value,
                                                               std::int32_t bits) = 0;
    virtual void bit_cursor_write_signed_word_bits_00429030(void* cursor, std::uint32_t value, std::int32_t bits) = 0;
    // Bit-cursor readers, __thiscall(ECX = cursor, out*, ...):
    // 00428E50 ReadWordArray(word*, count, bits), RET 0Ch; 00428D70
    // ReadBoolean(byte*), RET 4; 004293F0 ReadNumericFloat(float*, int, int,
    // float max, int bits), RET 14h; 00428D10 ReadUnsignedDword(dword*, bits),
    // RET 8; 00428CB0 ReadUnsignedWord(word*, bits), RET 8.
    virtual void bit_cursor_read_word_array_00428e50(void* cursor, void* words, std::int32_t count,
                                                     std::int32_t bits) = 0;
    virtual void bit_cursor_read_boolean_00428d70(void* cursor, std::uint8_t* out) = 0;
    virtual void bit_cursor_read_numeric_float_004293f0(void* cursor, float* out, std::int32_t a, std::int32_t b,
                                                        float max, std::int32_t bits) = 0;
    virtual void bit_cursor_read_unsigned_dword_00428d10(void* cursor, std::uint32_t* out, std::int32_t bits) = 0;
    virtual void bit_cursor_read_unsigned_word_00428cb0(void* cursor, std::uint16_t* out, std::int32_t bits) = 0;
    // 00605070 BSP_Math_WrapAngleInPlace_Provisional: __thiscall(ECX = float*).
    virtual void math_wrap_angle_in_place_00605070(float* angle) = 0;

    // ---- guns, scene nodes, messages (batch 2) ----
    // 004B7EF0 BSP_ObserverLink_DestroyBase: __thiscall(ECX = link).
    virtual void observer_link_destroy_base_004b7ef0(void* link) = 0;
    // 006FDDA0 BSP_TurningGunBase_Construct: __thiscall(ECX = gun, arg), RET 4.
    virtual void turning_gun_base_construct_006fdda0(void* gun, std::uint32_t arg) = 0;
    // 00B6DA70 BSP_SceneNode_SetVisibilityFactor: __thiscall(ECX = node, float, int), RET 8.
    virtual void scene_node_set_visibility_factor_00b6da70(void* node, float factor, std::int32_t flag) = 0;
    // 00B6D800 BSP_Node_GetNameHeader: __thiscall(ECX = node), EAX (name text at +4h).
    virtual void* node_get_name_header_00b6d800(void* node) = 0;
    // 00BF7FBF _stricmp (cdecl, CRT).
    virtual std::int32_t crt_stricmp_00bf7fbf(const char* a, const char* b) = 0;
    // 0077C2A0 BSP_Session_RouteMessage: __thiscall(ECX = entity, message*, int, int), RET 0Ch.
    virtual void session_route_message_0077c2a0(void* entity, void* message, std::int32_t a, std::int32_t b) = 0;
    // 00443490 BSP_DeviceClass_ResolveAndActivate: __fastcall(ECX = zero-extended word id, DL), EAX.
    virtual void* device_class_resolve_and_activate_00443490(std::uint32_t id, std::uint8_t flag) = 0;
    // Device class slot +28h(zero-extended word), EAX = the new entity.
    virtual void* vcall_28_create(void* device_class, std::uint32_t id) = 0;
    // 00766FC0 BSP_NetEntityCreateMessage_Construct: __thiscall(ECX = message, int type, a, entity, int), RET 10h.
    virtual void net_entity_create_message_construct_00766fc0(void* message, std::int32_t type, std::uint32_t a,
                                                              void* entity, std::int32_t b) = 0;
    // 00BF857A LIBCRT_fmod (x87: st1 fmod st0), CRT.
    virtual double crt_fmod_00bf857a(double x, double y) = 0;
    // Effect slot +20h(float), RET 4.
    virtual void vcall_20_mix(void* element, float mix) = 0;
    // Message slot +0Ch(int), AL (the kind query on the request).
    virtual bool vcall_0c_is(void* object, std::int32_t kind) = 0;
    // Gun slot +1B4h(int), RET 4.
    virtual void vcall_1b4(void* gun, std::int32_t index) = 0;
    // 0072B4C0 BSP_Gun_StopFiringEffects: __thiscall(ECX = gun).
    virtual void gun_stop_firing_effects_0072b4c0(void* gun) = 0;
    // 00956C20 BSP_Unit_RebuildWeaponCategoryIndex: __thiscall(ECX = unit).
    virtual void unit_rebuild_weapon_category_index_00956c20(void* unit) = 0;
    // 00779FC0 BSP_Entity_SendMessageToPeer: __thiscall(ECX = entity, peer, message*), RET 8.
    virtual void entity_send_message_to_peer_00779fc0(void* entity, std::uint32_t peer, void* message) = 0;
    // 00730B80 BSP_MultipleBombPlatform_Construct: __thiscall(ECX = block, arg), RET 4, EAX.
    virtual void* multiple_bomb_platform_construct_00730b80(void* block, std::uint32_t arg) = 0;

    // ---- gun classes, narratives (batch 3) ----
    // 0043EBD0 BSP_DamageableClass_EnsureModelResource: __thiscall(ECX = class, arg), RET 4.
    virtual void damageable_class_ensure_model_resource_0043ebd0(void* gun_class, std::uint32_t arg) = 0;
    // 006EAFE0 BSP_BulletClass_AcquireAndFinalise: __fastcall(ECX = out ref*, EDX = key), constructs *out.
    virtual void bullet_class_acquire_and_finalise_006eafe0(void** out, std::uint32_t key) = 0;
    // 0072B6D0 BSP_NativeVector14_Resize_0072b6d0: __thiscall(ECX = vector, int), RET 4.
    virtual void native_vector14_resize_0072b6d0(void* vector, std::int32_t size) = 0;
    // 0041CC80 BSP_CriticalSection_DestroyOwned: __thiscall(ECX = section).
    virtual void critical_section_destroy_owned_0041cc80(void* section) = 0;
    // 006B88E0 BSP_LuaVariantVector_Resize: __thiscall(ECX = vector, int), RET 4.
    virtual void lua_variant_vector_resize_006b88e0(void* vector, std::int32_t size) = 0;
    // 00734CE0 BSP_NarrativeList_Clear: __thiscall(ECX = the list at narrative+4h).
    virtual void narrative_list_clear_00734ce0(void* list) = 0;
};

// pi as float at 00D7A264 (bits 40490FDB) and FLT_MAX at 00CFDBF8 (bits 7F7FFFFF).
constexpr std::uint32_t kCc13Seg40PiBits = 0x40490FDBu;
constexpr std::uint32_t kCc13Seg40FltMaxBits = 0x7F7FFFFFu;

// ---- small owners ----
void cc13_seg40_tick_subnode_owner_destruct_base_007293d0(void* owner, Cc13Seg40Host& host);
void cc13_seg40_free_owned_block_007294a0(void* holder, Cc13Seg40Host& host);
void cc13_seg40_owned_ptr_delete_007294c0(void* holder, Cc13Seg40Host& host);
void cc13_seg40_owned_ptr_delete_007359d0(void* holder, Cc13Seg40Host& host);
void cc13_seg40_owned_ptr_delete_007359f0(void* holder, Cc13Seg40Host& host);
void* cc13_seg40_entity_as_kind08_0072b990(void* entity, Cc13Seg40Host& host);
void* cc13_seg40_entity_as_kind06_0072b9b0(void* entity, Cc13Seg40Host& host);
void cc13_seg40_gun_bot_on_target_destroyed_0072bce0(void* target_member, void* destroyed, Cc13Seg40Host& host);

// ---- gun session messages ----
void cc13_seg40_session_message_gun_create_write_00728be0(void* message, void* cursor, Cc13Seg40Host& host);
void cc13_seg40_session_message_gun_create_read_00728c90(void* message, void* stream, Cc13Seg40Host& host);
void* cc13_seg40_session_message_s_t_gun_fire_construct_007290d0(void* message, void* gun, std::uint32_t value_28,
                                                                 std::uint8_t flag_2c, float value_30,
                                                                 float value_34, std::uint8_t flag_3a,
                                                                 float value_3c, Cc13Seg40Host& host);
void cc13_seg40_session_message_s_t_gun_fire_write_007291c0(void* message, void* cursor, Cc13Seg40Host& host);
void cc13_seg40_session_message_s_t_gun_fire_read_007292c0(void* message, void* stream, Cc13Seg40Host& host);
void* cc13_seg40_session_message_gun_create_construct_0072a1d0(void* message, std::uint32_t a, void* gun,
                                                               std::uint8_t value_111, Cc13Seg40Host& host);
void cc13_seg40_session_message_gun_create_create_entity_0072a190(void* message, Cc13Seg40Host& host);
void cc13_seg40_gun_send_create_message_0072b640(void* gun, std::uint32_t peer, Cc13Seg40Host& host);

// ---- guns ----
void cc13_seg40_gun_apply_wreck_visibility_007297b0(void* gun, Cc13Seg40Host& host);
void cc13_seg40_gun_show_only_mesh_holder_00729870(void* gun, Cc13Seg40Host& host);
bool cc13_seg40_gun_on_event_destroyed_00729fd0(void* entity, const void* event_name, std::uint32_t unused,
                                                Cc13Seg40Host& host);
bool cc13_seg40_gun_on_event_destroyed_0072a0d0(void* entity, const void* event_name, std::uint32_t unused,
                                                Cc13Seg40Host& host);
void cc13_seg40_gun_effect_set_set_mix_0072aa10(void* set, float mix, Cc13Seg40Host& host);
void cc13_seg40_gun_barrel_table_destruct_0072abe0(void* table, Cc13Seg40Host& host);
void cc13_seg40_gun_apply_set_destroyed_failure_0072b5a0(void* gun, void* request, Cc13Seg40Host& host);
void* cc13_seg40_rapid_fixed_slave_gun_construct_00730e80(void* gun, std::uint32_t arg, Cc13Seg40Host& host);
void cc13_seg40_gun_aim_error_write_00730fb0(void* error, void* writer, Cc13Seg40Host& host);
void cc13_seg40_torpedo_shot_command_destruct_00731220(void* command, Cc13Seg40Host& host);
void cc13_seg40_torpedo_shot_command_read_00731270(void* command, void* reader, Cc13Seg40Host& host);
void cc13_seg40_gun_bot_set_fire_target_point_0072bdd0(void* bot, std::uint32_t x, std::uint32_t y, std::uint32_t z,
                                                        Cc13Seg40Host& host);
void* cc13_seg40_node_factory_create_multiple_bomb_platform_00731c80(void* gun_class, std::uint32_t arg,
                                                                     Cc13Seg40Host& host);
void* cc13_seg40_node_factory_create_rapid_fixed_slave_gun_00731d50(void* gun_class, std::uint32_t arg,
                                                                    Cc13Seg40Host& host);
void cc13_seg40_gun_class_activate_model_and_bullet_classes_00731a50(void* gun_class, std::uint32_t arg,
                                                                     Cc13Seg40Host& host);
void cc13_seg40_native_vector14_destruct_0072cae0(void* vector, Cc13Seg40Host& host);
void cc13_seg40_native_particle_population_lock_destruct_0072cc60(void* lock, std::uint32_t* global_0108ff50,
                                                                  Cc13Seg40Host& host);
void cc13_seg40_countdown_callback_holder_destruct_00733e70(void* holder, Cc13Seg40Host& host);
void cc13_seg40_narrative_callback_holder_destruct_00733f00(void* holder, Cc13Seg40Host& host);
void cc13_seg40_narrative_entry_destruct_00733dc0(void* entry, Cc13Seg40Host& host);
void cc13_seg40_mission_narrative_destruct_00735080(void* narrative, Cc13Seg40Host& host);
void* cc13_seg40_session_message_missioninfo_construct_00733b10(void* message, const void* text,
                                                                Cc13Seg40Host& host);

}  // namespace bsp
