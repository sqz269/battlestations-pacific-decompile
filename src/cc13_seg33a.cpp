// Segment 33 lower half reconstructions (packet cc13-seg33a, marker cc13_seg33a). See
// include/bsp/cc13_seg33a.hpp for the model and docs/CC13_SEG33A.md for evidence. Names are
// hypotheses, not recovered symbols.

#include "bsp/cc13_seg33a.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <cstring>
#include <vector>

namespace bsp::cc13_seg33a {

namespace {

ArchiveArg value_arg(std::uint32_t kind, void* address) {
    return ArchiveArg{kind, reinterpret_cast<std::uintptr_t>(address)};
}
ArchiveArg float_arg(float value) {
    std::uint32_t bits;
    static_assert(sizeof bits == sizeof value);
    std::memcpy(&bits, &value, sizeof bits);
    return ArchiveArg{2u, bits};
}

}  // namespace

// ---------------------------------------------------------------------------
// Gun bots.
// ---------------------------------------------------------------------------

// 006DEEC0, __thiscall(bot, index), RET 4. The party slot [bot+50h]+1ACh+index*4: 8 counts as
// controlled without the call, otherwise 00927F10 decides. Returns 1/0 in the whole EAX.
bool pilot_bot_party_slot_ai_controlled_006deec0(Seg33aHost& host, void* bot, std::int32_t index) {
    const std::int32_t slot = at<std::int32_t>(ptr(bot, 0x50), 0x1AC + static_cast<std::size_t>(index) * 4);
    if (slot == 8) return true;
    return host.party_slot_is_ai_held_00927f10(slot);
}

// 006DF170, __thiscall(bot, target), RET 4; primary slot +38h of the gun-bot base. The target
// record lives at bot+38h (kind byte +0, id word +2, object +4, vec3 +8, float +14h); its observer
// member is bot+1Ch. Unchanged target: nothing. Otherwise slot +30h clears, and a non-null target
// is bound: vec3 from 00F87574, kind word 0 then kind byte 1, object, id = target+174h (word),
// float 0, and 00694A60(ECX target, EDX bot+1Ch).
void gun_bot_set_fire_target_006df170(Seg33aHost& host, void* bot, void* target) {
    void* const record = add(bot, 0x38);
    if (host.command_target_resolve_00521ea0(record) == target) return;
    host.gun_bot_clear_target_vt30(bot);
    if (target == nullptr) return;
    const float* zero = host.zero_vec3_00f87574();
    at<float>(record, 0x08) = zero[0];
    at<float>(record, 0x0C) = zero[1];
    at<float>(record, 0x10) = zero[2];
    at<std::uint16_t>(record, 0x00) = 0;
    at<void*>(record, 0x04) = target;
    at<std::uint8_t>(record, 0x00) = 1;
    at<std::uint16_t>(record, 0x02) = at<std::uint16_t>(target, 0x174);
    at<float>(record, 0x14) = 0.0f;
    host.observer_register_pair_00694a60(target, add(bot, 0x1C));
}

// 006DF1F0, __thiscall(bot, unit), RET 4; the artillery bot's attach. Base 008FBC80, then
// bot +58h = +50h, ErrorOffset +84h..+8Ch from 00F87574, calcErrTick +74h = 0, +78h = 1.0f
// (00D7A24C), then 006DEFF0(bot).
void gun_bot_attach_muzzle_class_006df1f0(Seg33aHost& host, void* bot, void* unit) {
    host.gun_bot_attach_base_008fbc80(bot, unit);
    at<void*>(bot, 0x58) = ptr(bot, 0x50);
    const float* zero = host.zero_vec3_00f87574();
    at<float>(bot, 0x84) = zero[0];
    at<float>(bot, 0x88) = zero[1];
    at<float>(bot, 0x8C) = zero[2];
    at<float>(bot, 0x74) = 0.0f;
    at<float>(bot, 0x78) = kOne_00d7a24c;
    host.artillery_bot_refresh_error_006deff0(bot);
}

// 006DF260, __thiscall(bot, archive), RET 4; slot +48h of 00CFE000. Section "_bot", base reader
// 008FE660, close, then the five fields by address through archive slot +10h; finally +58h = +50h.
void gun_bot_read_properties_006df260(Seg33aHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    host.gun_bot_read_base_properties_008fe660(bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_read_field_vt10(archive, key(kErrorKey), value_arg(6, add(bot, 0x5C)));
    host.archive_read_field_vt10(archive, key(kCalcErrTickKey), value_arg(2, add(bot, 0x74)));
    host.archive_read_field_vt10(archive, key(kDelayedFireKey), value_arg(3, add(bot, 0x7C)));
    host.archive_read_field_vt10(archive, key(kFireDelayTimeKey), value_arg(2, add(bot, 0x80)));
    host.archive_read_field_vt10(archive, key(kErrorOffsetKey), value_arg(5, add(bot, 0x84)));
    at<void*>(bot, 0x58) = ptr(bot, 0x50);
}

// 006DF390, __thiscall(bot, archive), RET 4; slot +4Ch. The writer twin through slot +0Ch: Error
// (kind 6) and ErrorOffset (kind 5) by address, the floats by value, delayedFire as kind 3 with
// only the low byte of the value written (the upper three bytes are stale stack in the image;
// here they are zero).
void gun_bot_write_properties_006df390(Seg33aHost& host, void* bot, void* archive) {
    host.archive_open_section_vt04(archive, key(kBotSectionKey));
    host.gun_bot_write_base_properties_008fe6d0(bot, archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kErrorKey), value_arg(6, add(bot, 0x5C)));
    host.archive_write_field_vt0c(archive, key(kCalcErrTickKey), float_arg(at<float>(bot, 0x74)));
    host.archive_write_field_vt0c(archive, key(kDelayedFireKey), ArchiveArg{3u, at<std::uint8_t>(bot, 0x7C)});
    host.archive_write_field_vt0c(archive, key(kFireDelayTimeKey), float_arg(at<float>(bot, 0x80)));
    host.archive_write_field_vt0c(archive, key(kErrorOffsetKey), value_arg(5, add(bot, 0x84)));
}

// 006DF4C0, __thiscall(bot, target), RET 4; the artillery bot's slot +38h. The base 006DF170, then
// with [bot+50h]+1ACh == 8 or 00927F10 passing, 0072D5B0(ECX [bot+58h], 0, 0); then when slot +44h
// answers null it tail-jumps gun->vtable[1E8h](0) (the zero is the null it just tested).
void artillery_gunner_bot_set_fire_target_006df4c0(Seg33aHost& host, void* bot, void* target) {
    gun_bot_set_fire_target_006df170(host, bot, target);
    const std::int32_t slot = at<std::int32_t>(ptr(bot, 0x50), 0x1AC);
    if (slot == 8 || host.party_slot_is_ai_held_00927f10(slot)) {
        host.gun_select_fire_mode_0072d5b0(ptr(bot, 0x58), 0, 0);
    }
    if (host.gun_bot_current_target_vt44(bot) == nullptr) {
        host.gun_release_fire_request_vt1e8(ptr(bot, 0x58), 0);
    }
}

// ---------------------------------------------------------------------------
// Battleship unit (vtable 00CF90B0) and the world-list bookkeeping of the ship family.
// ---------------------------------------------------------------------------

// 006DFC90, __thiscall(unit, arg), RET 4. 0081ED40(unit, arg), then the eight vtables of the
// battleship instance and the class id +C4h = 0Dh. Returns unit.
void* battleship_construct_006dfc90(Seg33aHost& host, void* unit, void* arg) {
    host.unit_vehicle_base_construct_0081ed40(unit, arg);
    at<std::uint32_t>(unit, 0x000) = 0x00CF90B0;
    at<std::uint32_t>(unit, 0x010) = 0x00CF9094;
    at<std::uint32_t>(unit, 0x024) = 0x00CF908C;
    at<std::uint32_t>(unit, 0x170) = 0x00CF9088;
    at<std::uint32_t>(unit, 0x1E4) = 0x00CF9080;
    at<std::uint32_t>(unit, 0x310) = 0x00CF9068;
    at<std::uint32_t>(unit, 0x38C) = 0x00CF9064;
    at<std::uint32_t>(unit, 0x72C) = 0x00CF9060;
    at<std::uint32_t>(unit, 0x0C4) = 0x0D;
    return unit;
}

// 006DFD10, __fastcall(unit /*ECX*/), RET; slot +30h of 00CF90B0, 00CFA778 and 00CFB738: the
// null-preserving sub-object pointer unit+72Ch.
void* ship_unit_sub_object_72c_006dfd10(void* unit) {
    return unit != nullptr ? add(unit, 0x72C) : nullptr;
}

// 006DFEF0, __thiscall(cls, arg), RET 4, SEH frame (state 0 around the constructor). Allocates
// 118Ch bytes (00BF55BE), clears them (00BF79F0) before the null test, constructs with 006DFC90
// when non-null, then 009553D0(ECX unit, cls) and unit+354h = cls on whatever the allocation gave
// (a null allocation faults there in the image; no guard is added).
void* battleship_create_006dfef0(Seg33aHost& host, void* cls, void* arg) {
    void* const memory = host.operator_new_00bf55be(0x118C);
    host.memset_00bf79f0(memory, 0, 0x118C);
    void* unit = memory != nullptr ? battleship_construct_006dfc90(host, memory, arg) : nullptr;
    host.unit_set_vehicle_class_009553d0(unit, cls);
    at<void*>(unit, 0x354) = cls;
    return unit;
}

namespace {

// The inlined find-and-erase of the ship family's detach routines: the list object's first node
// is at list+4, each node is {?, next +4, value +8}; the first node naming the unit is erased with
// 004837D0(ECX list, node) and the walk stops.
void detach_from_unit_list(Seg33aHost& host, void* list, void* unit) {
    for (void* node = ptr(list, 4); node != nullptr; node = ptr(node, 4)) {
        if (ptr(node, 8) == unit) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}

}  // namespace

// 006DFF80, __fastcall(unit /*ECX*/), RET. 00928560(unit), then 00484540 push_back of the unit on
// the four lists [unit+30h]+30h, +48h, +54h and +60h, in that order.
void ship_unit_register_in_world_lists_006dff80(Seg33aHost& host, void* unit) {
    host.game_entity_register_in_parent_list_00928560(unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x30), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x48), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x54), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x60), unit);
}

// 006DFFC0, __fastcall(unit /*ECX*/), RET. 006D3620(unit), then the unit leaves [unit+30h]+54h and
// then +60h (the world pointer is re-read before each list).
void unit_instance_detach_from_mid_world_lists_006dffc0(Seg33aHost& host, void* unit) {
    host.unit_instance_detach_early_lists_006d3620(unit);
    detach_from_unit_list(host, add(ptr(unit, 0x30), 0x54), unit);
    detach_from_unit_list(host, add(ptr(unit, 0x30), 0x60), unit);
}

// ---------------------------------------------------------------------------
// BOMB_CREATE session message, bomb platforms, the 00CF9348 observer member.
// ---------------------------------------------------------------------------

// 006E05F0, __thiscall(object, const float* v), RET 4: +60h..+68h and +6Ch..+74h both = v (x87
// FLD/FSTP float copies, each element reloaded from v).
void object_set_vec3_pair_60_006e05f0(void* object, const float* v) {
    at<float>(object, 0x60) = v[0];
    at<float>(object, 0x64) = v[1];
    at<float>(object, 0x68) = v[2];
    at<float>(object, 0x6C) = v[0];
    at<float>(object, 0x70) = v[1];
    at<float>(object, 0x74) = v[2];
}

// 006E0690, __thiscall(message, cursor), RET 4. The base 00765000(message, cursor), then the vec3
// +110h through 00429790(+110h, 3, 0, 1, FLT_MAX (00D7A248, float), 20h), the byte +11Ch as a
// boolean (004290B0, zero-extended), and the word +11Eh in 0Dh bits (00429120, zero-extended).
void bomb_create_message_write_006e0690(Seg33aHost& host, void* message, void* cursor) {
    host.net_entity_create_message_write_00765000(message, cursor);
    host.bit_cursor_write_float_array_00429790(cursor, static_cast<float*>(add(message, 0x110)), 3, 0, 1,
                                               kFltMax_00d7a248, 0x20);
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x11C));
    host.bit_cursor_write_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x11E), 0x0D);
}

// 006E06F0, __thiscall(message, stream), RET 4. The reading twin: 00767E30(message, stream), then
// with the cursor stream+4: 004294F0(+110h, 3, 0, 1, FLT_MAX, 20h), 00428D70(&+11Ch),
// 00428E30(&+11Eh, 0Dh).
void bomb_create_message_read_006e06f0(Seg33aHost& host, void* message, void* stream) {
    host.net_entity_create_message_read_00767e30(message, stream);
    void* const cursor = add(stream, 4);
    host.bit_cursor_read_float_array_004294f0(cursor, static_cast<float*>(add(message, 0x110)), 3, 0, 1,
                                              kFltMax_00d7a248, 0x20);
    host.bit_cursor_read_boolean_00428d70(cursor, static_cast<std::uint8_t*>(add(message, 0x11C)));
    host.bit_cursor_read_word_bits_00428e30(cursor, static_cast<std::uint16_t*>(add(message, 0x11E)), 0x0D);
}

// 006E0750, __thiscall(message, arg), RET 4: slot +20h of the BOMB_CREATE message, empty.
void bomb_create_message_vslot20_noop_006e0750(void*, void*) {}

// 006E0860, __thiscall(member), RET, SEH frame. Re-stamps vtable 00CF9348, unregisters from the
// subject +14h when set (006952A0, ECX subject, EDX member), then 00695870(member).
void observer_member_9348_destruct_006e0860(Seg33aHost& host, void* member) {
    at<std::uint32_t>(member, 0) = 0x00CF9348;
    if (ptr(member, 0x14) != nullptr) host.observer_unregister_pair_006952a0(ptr(member, 0x14), member);
    host.callback_owner_destroy_00695870(member);
}

namespace {

// The shared factory shape of 006E08F0 / 006E0970: allocate, clear before the null test,
// construct when non-null, then platform +3F4h = +354h = cls on the result (null faults there).
void* create_bomb_platform(Seg33aHost& host, void* cls, void* arg, std::size_t size, bool multiple) {
    void* const memory = host.operator_new_00bf55be(size);
    host.memset_00bf79f0(memory, 0, size);
    void* platform = nullptr;
    if (memory != nullptr) {
        platform = multiple ? host.multi_bomb_platform_construct_006e42d0(memory, arg)
                            : host.bomb_platform_construct_006e3c00(memory, arg);
    }
    at<void*>(platform, 0x3F4) = cls;
    at<void*>(platform, 0x354) = cls;
    return platform;
}

}  // namespace

// 006E08F0, __thiscall(cls, arg), RET 4, SEH frame: 514h bytes, 006E3C00.
void* bomb_platform_create_006e08f0(Seg33aHost& host, void* cls, void* arg) {
    return create_bomb_platform(host, cls, arg, 0x514, false);
}

// 006E0970, __thiscall(cls, arg), RET 4, SEH frame: 688h bytes, 006E42D0.
void* multiple_bomb_platform_create_006e0970(Seg33aHost& host, void* cls, void* arg) {
    return create_bomb_platform(host, cls, arg, 0x688, true);
}

// 006E09F0, __thiscall(projectile, message), RET 4 -> AL. Only message type BAh (+10h byte) is
// handled: 0084B8C0(ECX [msg+1Ch], EDX [projectile+314h], [msg+20h], &(msg+24h..2Ch),
// &(msg+30h..38h)) on stack copies of the two vec3s, then 1; other types 0.
bool bomb_projectile_handle_message_006e09f0(Seg33aHost& host, void* projectile, void* message) {
    if (at<std::uint8_t>(message, 0x10) != 0xBA) return false;
    float second[3] = {at<float>(message, 0x30), at<float>(message, 0x34), at<float>(message, 0x38)};
    float first[3] = {at<float>(message, 0x24), at<float>(message, 0x28), at<float>(message, 0x2C)};
    host.projectile_dispatch_impact_effect_0084b8c0(at<std::int32_t>(message, 0x1C), ptr(projectile, 0x314),
                                                    at<std::int32_t>(message, 0x20), first, second);
    return true;
}

// 006E0B40, __thiscall(projectile, char visible), RET 4. +459h = visible; with a scene node at
// +464h, 00B6DA70(ECX [node+0Ch], visible ? 1.0f (00D7A24C) : 0.0f, 0).
void bomb_projectile_set_visible_006e0b40(Seg33aHost& host, void* projectile, std::uint8_t visible) {
    at<std::uint8_t>(projectile, 0x459) = visible;
    void* const node = ptr(projectile, 0x464);
    if (node == nullptr) return;
    host.scene_node_set_visibility_factor_00b6da70(ptr(node, 0x0C), visible != 0 ? kOne_00d7a24c : 0.0f, 0);
}

// 006E0BC0, __thiscall(projectile, unused), RET 4. With a node at +464h,
// 00B6DA70(ECX [node+0Ch], 0.0f, 0); the flag +459h is not touched.
void bomb_projectile_hide_node_006e0bc0(Seg33aHost& host, void* projectile, void*) {
    void* const node = ptr(projectile, 0x464);
    if (node == nullptr) return;
    host.scene_node_set_visibility_factor_00b6da70(ptr(node, 0x0C), 0.0f, 0);
}

// 006E0CD0, __thiscall(object, float* out), RET 4. The point node +110h: unless its +5Ch bit 1 is
// set, 00B6DB70(node) refreshes the world matrix; out = node +120h..+128h.
void bomb_projectile_trail_last_point_006e0cd0(Seg33aHost& host, void* object, float* out) {
    void* const node = ptr(object, 0x110);
    if ((at<std::uint8_t>(node, 0x5C) & 0x2) == 0) host.transform_refresh_world_matrix_00b6db70(node);
    out[0] = at<float>(node, 0x120);
    out[1] = at<float>(node, 0x124);
    out[2] = at<float>(node, 0x128);
}

// 006E0D20, __thiscall(message, type, a2, entity), RET 0Ch. 00766FC0(message, type, a2, entity, 0),
// vtable 00CF9324, +11Ch = entity+458h (byte), +110h..+118h = entity +318h..+320h (the velocity),
// +11Eh = the id word +174h of entity+3D8h, or 0 without one. Returns message.
void* bomb_create_message_construct_006e0d20(Seg33aHost& host, void* message, std::uint32_t type,
                                            std::uint32_t a2, void* entity) {
    host.net_entity_create_message_construct_00766fc0(message, type, a2, entity, 0);
    at<std::uint32_t>(message, 0) = 0x00CF9324;
    at<std::uint8_t>(message, 0x11C) = at<std::uint8_t>(entity, 0x458);
    at<float>(message, 0x110) = at<float>(entity, 0x318);
    at<float>(message, 0x114) = at<float>(entity, 0x31C);
    at<float>(message, 0x118) = at<float>(entity, 0x320);
    void* const owner = ptr(entity, 0x3D8);
    at<std::uint16_t>(message, 0x11E) = owner != nullptr ? at<std::uint16_t>(owner, 0x174) : std::uint16_t{0};
    return message;
}

namespace {

// The observer-member retarget shared by 006E0DB0 / 006E0E60 / 006E1820: unchanged target does
// nothing; otherwise unregister from the old subject (006952A0), store, register with the new one
// (00694A60), both __fastcall(subject, member).
void observer_member_retarget(Seg33aHost& host, void* member, void* target) {
    void* const current = ptr(member, 0x14);
    if (current == target) return;
    if (current != nullptr) host.observer_unregister_pair_006952a0(current, member);
    at<void*>(member, 0x14) = target;
    if (target != nullptr) host.observer_register_pair_00694a60(target, member);
}

}  // namespace

// 006E0DB0, __thiscall(member, target), RET 4, no return value (EAX is left as the last call's).
void observer_member_set_target_006e0db0(Seg33aHost& host, void* member, void* target) {
    observer_member_retarget(host, member, target);
}

// 006E0E00, __thiscall(member, target), RET 4, SEH frame. +4h, +8h, +Ch = 0, vtable 00CF9348,
// +14h = target, +10h = 1, then 00694A60(target, member) for a non-null target. Returns member.
void* observer_member_9348_construct_006e0e00(Seg33aHost& host, void* member, void* target) {
    at<std::uint32_t>(member, 0x4) = 0;
    at<std::uint32_t>(member, 0x8) = 0;
    at<std::uint32_t>(member, 0xC) = 0;
    at<std::uint32_t>(member, 0) = 0x00CF9348;
    at<void*>(member, 0x14) = target;
    at<std::uint8_t>(member, 0x10) = 1;
    if (target != nullptr) host.observer_register_pair_00694a60(target, member);
    return member;
}

// 006E0E60, __thiscall(member, target), RET 4: the retarget, returning member.
void* observer_member_set_target_006e0e60(Seg33aHost& host, void* member, void* target) {
    observer_member_retarget(host, member, target);
    return member;
}

// 006E0EA0, __thiscall(object, float* out), RET 4. out = identity (1.0f from 00D7A24C), then
// 00467050(ECX out, +30h, +34h, +38h) sets the rotation from the three angles, then out row 3
// (+30h..+38h) = object +24h..+2Ch. Returns out.
float* transform_from_object_006e0ea0(Seg33aHost& host, void* object, float* out) {
    for (int i = 0; i < 16; ++i) out[i] = (i % 5 == 0) ? kOne_00d7a24c : 0.0f;
    host.matrix_set_rotation_from_euler_00467050(out, at<float>(object, 0x30), at<float>(object, 0x34),
                                                 at<float>(object, 0x38));
    out[12] = at<float>(object, 0x24);
    out[13] = at<float>(object, 0x28);
    out[14] = at<float>(object, 0x2C);
    return out;
}

// ---------------------------------------------------------------------------
// Bomb projectile (record vtable family 00CF93F0): state writer and the per-step pose.
// ---------------------------------------------------------------------------

// 006E0F40, __thiscall(projectile, archive), RET 4. 0077C160(projectile, archive); field
// "bulletclass" = {kind 1, [[projectile+314h]+0Ch]} (the class index); section "_bomb"; section
// "_bulletBase" with 006E6E80(ECX projectile+310h, archive), closed; "flying" = {3, byte +458h};
// "_bomb" closed.
void bomb_projectile_write_state_006e0f40(Seg33aHost& host, void* projectile, void* archive) {
    host.net_entity_write_replication_state_0077c160(projectile, archive);
    host.archive_write_field_vt0c(archive, key(kBulletClassKey),
                                  ArchiveArg{1u, at<std::uint32_t>(ptr(projectile, 0x314), 0x0C)});
    host.archive_open_section_vt04(archive, key(kBombSectionKey));
    host.archive_open_section_vt04(archive, key(kBulletBaseSectionKey));
    bullet_base_write_state_006e6e80(host, add(projectile, 0x310), archive);
    host.archive_close_section_vt08(archive);
    host.archive_write_field_vt0c(archive, key(kFlyingKey), ArchiveArg{3u, at<std::uint8_t>(projectile, 0x458)});
    host.archive_close_section_vt08(archive);
}

namespace {

void refresh_pose_if_stale(Seg33aHost& host, void* entity) {
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
}

}  // namespace

// 006E1060, __thiscall(self = record+3E4h, float step), RET 4; slot +4h of the bomb family's tick
// element tables. Offsets below are from the record.
//  - scaled = (float)([[record+314h]+5Ch] x step), x87 FMUL then FSTP float.
//  - When record+410h >= 0.0f (00D7A218; COMISS/JC, so a NaN also skips): with flying (+458h),
//    tick element (record+310h) vtable[2Ch]() picks record vtable[194h] (zero) or [198h]
//    (non-zero) called with scaled. Then vtable[2Ch]() is asked again:
//      zero: with the air trail +338h, out = 006E0CD0(trail); 00414DB0 when +C8h is clear;
//        004842C0(ECX trail, record+FCh); d = record+FCh - out (each component rounded to float);
//        when (float)(dx*dx + dy*dy + dz*dz) > 1.0e-4 (00D7A268, double), M = world +CCh with row 2
//        = d, 0085DC80(M), row 3 = record+FCh, 0053D9C0(ECX trail, M); the pose is refreshed
//        before every read of +FCh/+CCh, as in the image.
//      non-zero: with the water trail +334h: 00414DB0 on [record+3DCh] when stale,
//        004142E0(ECX record+33Ch, &tmp, [record+3DCh]+CCh), 004842C0(ECX water trail, result).
//  - Always (also after the +410h gate fails): with flying and a node at +464h, M = world +CCh
//    with M[3] = M[7] = M[11] = 0 and M[15] = 1.0f, then [node+0Ch]->vtable[34h](M).
void bomb_projectile_place_step_pose_006e1060(Seg33aHost& host, void* self, float step) {
    void* const record = sub(self, 0x3E4);
    const float scaled = static_cast<float>(at<float>(ptr(record, 0x314), 0x5C) * step);
    if (at<float>(record, 0x410) >= 0.0f) {
        void* const element = add(record, 0x310);
        if (at<std::uint8_t>(record, 0x458) != 0) {
            if (host.tick_element_in_water_vt2c(element) == 0) host.bomb_record_step_air_vt194(record, scaled);
            else host.bomb_record_step_water_vt198(record, scaled);
        }
        if (host.tick_element_in_water_vt2c(element) == 0) {
            void* const trail = ptr(record, 0x338);
            if (trail != nullptr) {
                float last[3];
                bomb_projectile_trail_last_point_006e0cd0(host, trail, last);
                refresh_pose_if_stale(host, record);
                host.point_effect_set_world_point_004842c0(trail, static_cast<const float*>(add(record, 0xFC)));
                refresh_pose_if_stale(host, record);
                const float d[3] = {at<float>(record, 0xFC) - last[0], at<float>(record, 0x100) - last[1],
                                    at<float>(record, 0x104) - last[2]};
                const double x = d[0], y = d[1], z = d[2];
                const float length_squared = static_cast<float>((x * x + y * y) + z * z);
                if (length_squared > kTrailMinSegmentSquared_00d7a268) {
                    refresh_pose_if_stale(host, record);
                    float m[16];
                    for (int i = 0; i < 16; ++i) m[i] = at<float>(record, 0xCC + 4 * static_cast<std::size_t>(i));
                    m[8] = d[0];
                    m[9] = d[1];
                    m[10] = d[2];
                    host.matrix_orthonormalize_basis_rows_0085dc80(m);
                    refresh_pose_if_stale(host, record);
                    m[12] = at<float>(record, 0xFC);
                    m[13] = at<float>(record, 0x100);
                    m[14] = at<float>(record, 0x104);
                    host.point_effect_set_world_matrix_0053d9c0(trail, m);
                }
            }
        } else {
            void* const water_trail = ptr(record, 0x334);
            if (water_trail != nullptr) {
                void* const anchor = ptr(record, 0x3DC);
                refresh_pose_if_stale(host, anchor);
                float tmp[3];
                const float* point = host.vector3_transform_affine_point_004142e0(
                    static_cast<const float*>(add(record, 0x33C)), tmp, static_cast<const float*>(add(anchor, 0xCC)));
                host.point_effect_set_world_point_004842c0(water_trail, point);
            }
        }
    }
    void* const node = ptr(record, 0x464);
    if (at<std::uint8_t>(record, 0x458) != 0 && node != nullptr) {
        refresh_pose_if_stale(host, record);
        float m[16];
        for (int i = 0; i < 16; ++i) m[i] = at<float>(record, 0xCC + 4 * static_cast<std::size_t>(i));
        m[3] = 0.0f;
        m[7] = 0.0f;
        m[11] = 0.0f;
        m[15] = kOne_00d7a24c;
        host.scene_object_set_matrix_vt34(ptr(node, 0x0C), m);
    }
}

// 006E1640, __thiscall(projectile, peer), RET 4, SEH frame. Builds a BOMB_CREATE message (type
// B6h) on the stack with 006E0D20(msg, B6h, [[projectile+314h]+0Ch], projectile), sends it with
// 00779FC0(ECX projectile, peer, msg), then re-stamps vtable 00CF9324 and runs the base destructor
// 007668E0(msg).
void bomb_projectile_send_create_message_006e1640(Seg33aHost& host, void* projectile, void* peer) {
    alignas(4) std::uint8_t message[0x120];
    void* const msg = bomb_create_message_construct_006e0d20(
        host, message, 0xB6, at<std::uint32_t>(ptr(projectile, 0x314), 0x0C), projectile);
    host.entity_send_message_to_peer_00779fc0(projectile, peer, msg);
    at<std::uint32_t>(message, 0) = 0x00CF9324;
    host.net_entity_create_message_destruct_007668e0(message);
}

namespace {

// The fixed-step catch-up shared by 006E16D0 and 006E1930: lag = frame - stamp ticks; the tick
// element at record+3E4h runs 008746A0(ECX element, lag) and then its slot +4h with the leftover
// accumulator [00F876AC].
void catch_up_fixed_steps(Seg33aHost& host, void* element, std::int32_t negative_lag) {
    host.sim_object_run_fixed_steps_008746a0(element, -negative_lag);
    host.tick_element_place_vt04(element, host.fixed_step_remainder_00f876ac());
}

}  // namespace

// 006E16D0, __thiscall(projectile, state), RET 4. 006E0EA0(ECX state, &M) builds the transform,
// 004134F0(ECX projectile+418h, M) copies it; +318h..+320h = state +3Ch..+44h, +348h..+350h =
// state +48h..+50h; then with ([state+0Ch] - [00F876B0]) negative (JNS on the subtraction) the
// catch-up runs on projectile+3E4h.
void bomb_projectile_apply_launch_state_006e16d0(Seg33aHost& host, void* projectile, void* state) {
    float m[16];
    host.matrix_copy_4x4_x87_004134f0(add(projectile, 0x418), transform_from_object_006e0ea0(host, state, m));
    at<float>(projectile, 0x318) = at<float>(state, 0x3C);
    at<float>(projectile, 0x31C) = at<float>(state, 0x40);
    at<float>(projectile, 0x320) = at<float>(state, 0x44);
    at<float>(projectile, 0x348) = at<float>(state, 0x48);
    at<float>(projectile, 0x34C) = at<float>(state, 0x4C);
    at<float>(projectile, 0x350) = at<float>(state, 0x50);
    const std::int32_t lag = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(at<std::int32_t>(state, 0x0C)) - static_cast<std::uint32_t>(host.frame_counter_00f876b0()));
    if (lag < 0) catch_up_fixed_steps(host, add(projectile, 0x3E4), lag);
}

// 006E1780 and 006E17D0 (identical bodies, two vtables), __thiscall(member, subject_ref), RET 4.
// subject = subject_ref->vtable[4](); when it is the member's +14h, +14h = 0 and
// 006952A0(ECX subject, EDX member).
void observer_member_on_target_gone_006e1780(Seg33aHost& host, void* member, void* subject_ref) {
    void* const subject = host.subject_ref_resolve_vt04(subject_ref);
    if (subject == ptr(member, 0x14)) {
        at<void*>(member, 0x14) = nullptr;
        host.observer_unregister_pair_006952a0(subject, member);
    }
}
void observer_member_938c_on_notify_006e17d0(Seg33aHost& host, void* member, void* subject_ref) {
    observer_member_on_target_gone_006e1780(host, member, subject_ref);
}

// 006E1820, __thiscall(member, target), RET 4: the retarget returning member.
void* observer_member_set_target_006e1820(Seg33aHost& host, void* member, void* target) {
    observer_member_retarget(host, member, target);
    return member;
}

namespace {

// The effect release of 006E1860: 00867B10(ECX effect), effect+9 = 1, then the refcount at
// effect+4 drops through [00CE2220] (InterlockedDecrement) and a zero count calls the effect's
// vtable[0]; the record slot is cleared (twice in the image).
void release_point_effect(Seg33aHost& host, void* record, std::size_t slot) {
    if (ptr(record, slot) == nullptr) return;
    host.point_effect_stop_children_00867b10(ptr(record, slot));
    at<std::uint8_t>(ptr(record, slot), 0x9) = 1;
    void* const effect = ptr(record, slot);
    if (effect != nullptr) {
        if (host.interlocked_decrement_00ce2220(static_cast<std::int32_t*>(add(effect, 4))) == 0) {
            host.ref_counted_destroy_vt00(effect);
        }
        at<void*>(record, slot) = nullptr;
    }
    at<void*>(record, slot) = nullptr;
}

}  // namespace

// 006E1860, __thiscall(record), RET, tail-jumps to 009277F0. The scene node +464h gets
// vtable[4](1) (deleting destructor) and +464h = 0; the water trail +334h and the air trail +338h
// are stopped and released; then 009277F0(record).
void bomb_projectile_release_sub_objects_006e1860(Seg33aHost& host, void* record) {
    if (ptr(record, 0x464) != nullptr) {
        host.object_deleting_destructor_vt04(ptr(record, 0x464), 1);
        at<void*>(record, 0x464) = nullptr;
    }
    release_point_effect(host, record, 0x334);
    release_point_effect(host, record, 0x338);
    host.release_sub_objects_tail_009277f0(record);
}

namespace {

// The inlined handle lookup (as in 00521EA0): id 0 -> null; id < [00F89A10] -> table
// [00F89A54] at (id - [00F89A0C]) << 4, else [00F89AA8] at (id - [00F89A60]) << 4; object at +0Ch.
void* resolve_entity_handle(Seg33aHost& host, std::uint16_t id) {
    if (id == 0) return nullptr;
    const HandleTables tables = host.entity_handle_tables_00f89a0c();
    const std::int32_t value = id;
    if (value < tables.split) {
        return at<void*>(tables.low_entries, static_cast<std::size_t>((value - tables.low_base) << 4) + 0xC);
    }
    return at<void*>(tables.high_entries, static_cast<std::size_t>((value - tables.high_base) << 4) + 0xC);
}

}  // namespace

// 006E1930, __thiscall(message), RET, SEH frame. The BOMB_CREATE receive path:
//  M = identity via 0075B500(ECX message, M); class = 006EAFE0(ECX &ref, EDX word +1Eh);
//  entity = class->vtable[20h](word +1Ch, [+B0h], &(+24h..+2Ch), &M[8], 0, 0, 0), whose sub-object
//  answer is rebased by -310h (null stays null); the class reference is released (InterlockedDecrement
//  at ref+4, vtable[0] on zero); entity+58h = byte +C4h; 004134F0(entity+74h, M); +318h..+320h =
//  +110h..+118h; +458h = +11Ch; 006E6BB0(ECX entity+310h, owner of handle word +11Eh);
//  message->vtable[20h](entity); pose refresh; 004134F0(entity+418h, entity+CCh); element
//  entity+3E4h vtable[0Ch](); then lag = [+0Ch] - [00F876B0]: negative runs the catch-up,
//  positive gives +410h = (float)(+410h - lag x 0.05f) (00D0DE84; FILD/FMUL/FSUBP DE E9).
//  Returns the entity. A null entity faults at the +58h store in the image; no guard is added.
void* bomb_create_message_create_entity_006e1930(Seg33aHost& host, void* message) {
    float m[16];
    host.transform_set_identity_0075b500(message, m);
    void* class_ref = nullptr;
    void* const* holder = bullet_class_acquire_and_finalise_006eafe0(host, &class_ref, at<std::uint16_t>(message, 0x1E));
    void* const cls = *holder;
    const float position[3] = {at<float>(message, 0x24), at<float>(message, 0x28), at<float>(message, 0x2C)};
    void* const sub_object = host.bullet_class_create_projectile_vt20(cls, at<std::uint16_t>(message, 0x1C),
                                                                      ptr(message, 0xB0), position, &m[8], 0, 0, 0);
    void* const entity = sub_object != nullptr ? sub(sub_object, 0x310) : nullptr;
    if (class_ref != nullptr) {
        if (host.interlocked_decrement_00ce2220(static_cast<std::int32_t*>(add(class_ref, 4))) == 0) {
            host.ref_counted_destroy_vt00(class_ref);
        }
        class_ref = nullptr;
    }
    at<std::uint32_t>(entity, 0x58) = at<std::uint8_t>(message, 0xC4);
    host.matrix_copy_4x4_x87_004134f0(add(entity, 0x74), m);
    at<float>(entity, 0x318) = at<float>(message, 0x110);
    at<float>(entity, 0x31C) = at<float>(message, 0x114);
    at<float>(entity, 0x320) = at<float>(message, 0x118);
    at<std::uint8_t>(entity, 0x458) = at<std::uint8_t>(message, 0x11C);
    host.projectile_bind_owner_006e6bb0(add(entity, 0x310),
                                        resolve_entity_handle(host, at<std::uint16_t>(message, 0x11E)));
    host.message_apply_to_entity_vt20(message, entity);
    refresh_pose_if_stale(host, entity);
    host.matrix_copy_4x4_x87_004134f0(add(entity, 0x418), add(entity, 0xCC));
    void* const element = add(entity, 0x3E4);
    host.tick_element_activate_vt0c(element);
    const std::int32_t lag = static_cast<std::int32_t>(
        static_cast<std::uint32_t>(at<std::int32_t>(message, 0x0C)) - static_cast<std::uint32_t>(host.frame_counter_00f876b0()));
    if (lag < 0) {
        catch_up_fixed_steps(host, element, lag);
    } else if (lag > 0) {
        const double scaled = static_cast<double>(lag) * static_cast<double>(kLagSeconds_00d0de84);
        at<float>(entity, 0x410) = static_cast<float>(static_cast<double>(at<float>(entity, 0x410)) - scaled);
    }
    return entity;
}

// 006E1B40, __thiscall(member), RET, SEH frame. +4h, +8h, +Ch, +14h = 0, word +18h = 0, +10h = 1,
// vtable 00CF938C. Returns member.
void* observer_member_938c_construct_006e1b40(void* member) {
    at<std::uint32_t>(member, 0x4) = 0;
    at<std::uint32_t>(member, 0x8) = 0;
    at<std::uint32_t>(member, 0xC) = 0;
    at<std::uint32_t>(member, 0x14) = 0;
    at<std::uint16_t>(member, 0x18) = 0;
    at<std::uint8_t>(member, 0x10) = 1;
    at<std::uint32_t>(member, 0) = 0x00CF938C;
    return member;
}

// 006E1B90, __thiscall(member, target), RET 4. The retarget, then the id word +18h = target+174h
// (zero-extended through EAX), or 0 without a target. Returns member.
void* attacker_handle_repoint_006e1b90(Seg33aHost& host, void* member, void* target) {
    observer_member_retarget(host, member, target);
    at<std::uint16_t>(member, 0x18) = target != nullptr ? at<std::uint16_t>(target, 0x174) : std::uint16_t{0};
    return member;
}

// ---------------------------------------------------------------------------
// Projectile shot (the +310h sub-object of a projectile record, vtable 00CF93B0 at its base).
// ---------------------------------------------------------------------------

namespace {

// Intrusive reference counting through the two imports: [00CE221C] InterlockedIncrement and
// [00CE2220] InterlockedDecrement on object+4; a zero count calls the object's vtable[0].
void ref_add(Seg33aHost& host, void* object) {
    host.interlocked_increment_00ce221c(static_cast<std::int32_t*>(add(object, 4)));
}
void ref_release(Seg33aHost& host, void* object) {
    if (host.interlocked_decrement_00ce2220(static_cast<std::int32_t*>(add(object, 4))) == 0) {
        host.ref_counted_destroy_vt00(object);
    }
}

// One effect creation of 006E1CD0: the descriptor reference is passed by value (incremented by
// the caller, consumed by 008685E0(ECX &out, EDX [[00E188A8]+19ECh], descriptor, record+FCh, 1, 0));
// the result is assigned to the shot slot with the usual add-new / release-old order, and the
// temporary is released.
void create_motor_effect(Seg33aHost& host, void* shot, void* record, void* descriptor, std::size_t slot) {
    refresh_pose_if_stale(host, record);
    void* const manager = host.effect_manager_00e188a8_19ec();
    if (descriptor != nullptr) ref_add(host, descriptor);
    void* created = nullptr;
    void* const* result = host.point_effect_create_from_position_008685e0(
        &created, manager, descriptor, static_cast<const float*>(add(record, 0xFC)), 1, 0);
    void* const effect = *result;
    void* const old = ptr(shot, slot);
    if (old != effect) {
        at<void*>(shot, slot) = effect;
        if (effect != nullptr) ref_add(host, effect);
        if (old != nullptr) ref_release(host, old);
    }
    if (created != nullptr) ref_release(host, created);
}

}  // namespace

// 006E1CD0, __thiscall(shot), RET, SEH frame. Record = shot-310h, class = [shot+4].
//  - With a motor-flame descriptor [class+40h]: the effect at record+FCh into shot+28h.
//  - With a trail descriptor [class+48h]: the effect into shot+24h; then with the new effect
//    non-null, its +50h = (float)(1 / class->vtable[1Ch]()) for class kind [class+8] == 0Ah
//    (FLD1; FDIVRP DE F1), else 0.0333333f (00CE7628).
// The pose refresh (00414DB0 on a stale record) runs before each creation.
void projectile_shot_ignite_motor_006e1cd0(Seg33aHost& host, void* shot) {
    void* const record = sub(shot, 0x310);
    if (ptr(ptr(shot, 4), 0x40) != nullptr) {
        create_motor_effect(host, shot, record, ptr(ptr(shot, 4), 0x40), 0x28);
    }
    if (ptr(ptr(shot, 4), 0x48) == nullptr) return;
    create_motor_effect(host, shot, record, ptr(ptr(shot, 4), 0x48), 0x24);
    void* const trail = ptr(shot, 0x24);
    if (trail == nullptr) return;
    void* const cls = ptr(shot, 4);
    if (at<std::int32_t>(cls, 0x8) == 0x0A) {
        const double rate = host.projectile_class_rate_vt1c(cls);
        at<float>(trail, 0x50) = static_cast<float>(1.0 / rate);
    } else {
        at<float>(trail, 0x50) = kTrailInterval_00ce7628;
    }
}

// 006E22D0, __thiscall(shot), RET, SEH frame: the projectile-shot base constructor. vtable
// 00CF93B0; +24h = +28h = 0; the observer member at +98h built inline (938C); the member at +B4h
// built inline (938C, no id word); +CCh = 0; +45h = 1; the +98h member's inline set-target(0)
// (unregister when +ACh is set, which it is not at this point) and word +B0h = 0; +18h = +1Ch =
// -1; +20h = +21h = 0; +14h = 13h; +44h = 0; the +28h and +24h references released (both null
// here) and cleared; +D0h = 1.0f; +2Ch..+34h = 0; +5Ch = +90h = 0. Returns shot.
void* projectile_shot_base_construct_006e22d0(Seg33aHost& host, void* shot) {
    at<std::uint32_t>(shot, 0) = 0x00CF93B0;
    at<void*>(shot, 0x24) = nullptr;
    at<void*>(shot, 0x28) = nullptr;
    void* const member = add(shot, 0x98);
    at<std::uint32_t>(member, 0x4) = 0;
    at<std::uint32_t>(member, 0x8) = 0;
    at<std::uint32_t>(member, 0xC) = 0;
    at<std::uint32_t>(member, 0x14) = 0;
    at<std::uint8_t>(member, 0x10) = 1;
    at<std::uint32_t>(member, 0) = 0x00CF938C;
    at<std::uint16_t>(member, 0x18) = 0;
    at<std::uint32_t>(shot, 0xB8) = 0;
    at<std::uint32_t>(shot, 0xBC) = 0;
    at<std::uint32_t>(shot, 0xC0) = 0;
    at<std::uint32_t>(shot, 0xC8) = 0;
    at<std::uint8_t>(shot, 0xC4) = 1;
    at<std::uint32_t>(shot, 0xB4) = 0x00CF938C;
    at<std::uint32_t>(shot, 0xCC) = 0;
    at<std::uint8_t>(shot, 0x45) = 1;
    if (ptr(member, 0x14) != nullptr) {
        host.observer_unregister_pair_006952a0(ptr(member, 0x14), member);
        at<void*>(member, 0x14) = nullptr;
    }
    at<std::uint16_t>(member, 0x18) = 0;
    at<std::int32_t>(shot, 0x18) = -1;
    at<std::int32_t>(shot, 0x1C) = -1;
    at<std::uint8_t>(shot, 0x20) = 0;
    at<std::uint8_t>(shot, 0x21) = 0;
    at<std::uint32_t>(shot, 0x14) = 0x13;
    at<std::uint8_t>(shot, 0x44) = 0;
    if (void* const old = ptr(shot, 0x28); old != nullptr) {
        ref_release(host, old);
        at<void*>(shot, 0x28) = nullptr;
    }
    at<void*>(shot, 0x28) = nullptr;
    if (void* const old = ptr(shot, 0x24); old != nullptr) {
        at<void*>(shot, 0x24) = nullptr;
        ref_release(host, old);
    }
    at<float>(shot, 0xD0) = kOne_00d7a24c;
    at<float>(shot, 0x2C) = 0.0f;
    at<float>(shot, 0x30) = 0.0f;
    at<float>(shot, 0x34) = 0.0f;
    at<std::uint8_t>(shot, 0x5C) = 0;
    at<std::uint8_t>(shot, 0x90) = 0;
    return shot;
}

// 006E2410, __thiscall(object) -> ST0, RET. 00BD2F10(ECX 1, lo = +ACh, hi = +B0h): a draw from the
// shared generator.
float random_uniform_between_ac_and_b0_006e2410(Seg33aHost& host, void* object) {
    return host.random_uniform_float_range_00bd2f10(1, at<float>(object, 0xAC), at<float>(object, 0xB0));
}

// 006E2470, __thiscall(shot, float* out), RET 4: out = +8h..+10h.
void projectile_shot_get_position_006e2470(void* shot, float* out) {
    out[0] = at<float>(shot, 0x8);
    out[1] = at<float>(shot, 0xC);
    out[2] = at<float>(shot, 0x10);
}

// 006E24E0, __thiscall(shot) -> ST0, RET. The class's range: 00BD2F10(ECX 1, [shot+4]+ACh,
// [shot+4]+B0h), a draw from the shared generator.
float projectile_shot_random_class_range_006e24e0(Seg33aHost& host, void* shot) {
    void* const cls = ptr(shot, 4);
    return host.random_uniform_float_range_00bd2f10(1, at<float>(cls, 0xAC), at<float>(cls, 0xB0));
}

// 006E2550, __thiscall(shot, float scale), RET 4: +D0h = scale.
void projectile_shot_set_weapon_scale_006e2550(void* shot, float scale) {
    at<float>(shot, 0xD0) = scale;
}

// 006E2580, __thiscall(shot) -> AL, RET: the class byte [shot+4]+74h (the upper EAX bytes keep the
// class pointer's).
std::uint8_t projectile_shot_class_flag_74_006e2580(void* shot) {
    return at<std::uint8_t>(ptr(shot, 4), 0x74);
}

// 006E25A0, __thiscall(shot), RET, SEH frame. vtable 00CF93B0; 006E0860 on the members +B4h then
// +98h; the +28h and +24h references released and cleared.
void projectile_shot_destruct_006e25a0(Seg33aHost& host, void* shot) {
    at<std::uint32_t>(shot, 0) = 0x00CF93B0;
    observer_member_9348_destruct_006e0860(host, add(shot, 0xB4));
    observer_member_9348_destruct_006e0860(host, add(shot, 0x98));
    if (void* const old = ptr(shot, 0x28); old != nullptr) {
        ref_release(host, old);
        at<void*>(shot, 0x28) = nullptr;
    }
    if (void* const old = ptr(shot, 0x24); old != nullptr) {
        ref_release(host, old);
        at<void*>(shot, 0x24) = nullptr;
    }
}

// ---------------------------------------------------------------------------
// Bomb projectile record (vtable 00CF9438): construction, accessors, world lists, destruction.
// ---------------------------------------------------------------------------

namespace {

void stamp_bomb_projectile_vtables(void* record) {
    at<std::uint32_t>(record, 0x000) = 0x00CF9438;
    at<std::uint32_t>(record, 0x010) = 0x00CF941C;
    at<std::uint32_t>(record, 0x024) = 0x00CF9414;
    at<std::uint32_t>(record, 0x170) = 0x00CF9410;
    at<std::uint32_t>(record, 0x1E4) = 0x00CF9408;
}

}  // namespace

// 006E2670, __thiscall(record, arg), RET 4, SEH frame. 0077EED0(record, arg); 006E22D0(record+310h);
// 00875890(ECX record+3E4h, record, 0); then shot vtable +310h = 00CF95E0, the five record vtables,
// +458h = 0, +459h = 1, +45Ch = -1.0f (00D7A260), +460h = -1, class id +C4h = 2Ah, +3DCh = record,
// tick element vtable +3E4h = 00CF93F0. Returns record.
void* bomb_projectile_construct_006e2670(Seg33aHost& host, void* record, void* arg) {
    host.unit_owner_entity_construct_0077eed0(record, arg);
    projectile_shot_base_construct_006e22d0(host, add(record, 0x310));
    host.tick_registration_construct_00875890(add(record, 0x3E4), record, 0);
    at<std::uint32_t>(record, 0x310) = 0x00CF95E0;
    stamp_bomb_projectile_vtables(record);
    at<std::uint8_t>(record, 0x458) = 0;
    at<std::uint8_t>(record, 0x459) = 1;
    at<float>(record, 0x45C) = kMinusOne_00d7a260;
    at<std::int32_t>(record, 0x460) = -1;
    at<std::uint32_t>(record, 0x0C4) = 0x2A;
    at<void*>(record, 0x3DC) = record;
    at<std::uint32_t>(record, 0x3E4) = 0x00CF93F0;
    return record;
}

// 006E27C0 / 006E27D0, __fastcall(record /*ECX*/), RET: the null-preserving sub-objects +310h
// (the shot, the bullet block) and +3E4h (the tick element).
void* bomb_projectile_bullet_block_006e27c0(void* record) {
    return record != nullptr ? add(record, 0x310) : nullptr;
}
void* bomb_projectile_tick_element_006e27d0(void* record) {
    return record != nullptr ? add(record, 0x3E4) : nullptr;
}

// 006E2820, __thiscall(record) -> ST0, RET. The class [record+314h] range (+ACh, +B0h) drawn
// through 00BD2F10(ECX 1, lo, hi): the shared generator.
float weapon_entity_hull_damage_base_006e2820(Seg33aHost& host, void* record) {
    void* const cls = ptr(record, 0x314);
    return host.random_uniform_float_range_00bd2f10(1, at<float>(cls, 0xAC), at<float>(cls, 0xB0));
}

// 006E2860, __thiscall(record, float* out), RET 4; slot 34h: out = velocity +318h..+320h.
void bomb_projectile_get_velocity_006e2860(void* record, float* out) {
    out[0] = at<float>(record, 0x318);
    out[1] = at<float>(record, 0x31C);
    out[2] = at<float>(record, 0x320);
}

// 006E2940, __thiscall(record, arg), RET 4: slot 18Ch, empty.
void bomb_projectile_vslot18c_noop_006e2940(void*, void*) {}

// 006E2990, __fastcall(record /*ECX*/), RET; slot 130h of the bomb family. 00928560(record), then
// push_back on [record+30h]+30h and +210h.
void bomb_register_in_world_lists_006e2990(Seg33aHost& host, void* record) {
    host.game_entity_register_in_parent_list_00928560(record);
    host.unit_list_push_back_00484540(add(ptr(record, 0x30), 0x30), record);
    host.unit_list_push_back_00484540(add(ptr(record, 0x30), 0x210), record);
}

// 006E29C0, __fastcall(record /*ECX*/), RET. 00928570(record), then the record leaves
// [record+30h]+30h and +210h (first matching node, 004837D0).
void bomb_family_detach_from_world_lists_006e29c0(Seg33aHost& host, void* record) {
    host.game_entity_detach_from_parent_list_00928570(record);
    detach_from_unit_list(host, add(ptr(record, 0x30), 0x30), record);
    detach_from_unit_list(host, add(ptr(record, 0x30), 0x210), record);
}

// 006E2A20, __thiscall(record), RET, SEH frame. Re-stamps the five record vtables, the shot
// 00CF95E0 and the element 00CF93F0; 00875490(ECX record+3E4h); 006E25A0(record+310h);
// 0077E380(record).
void bomb_projectile_destruct_006e2a20(Seg33aHost& host, void* record) {
    stamp_bomb_projectile_vtables(record);
    at<std::uint32_t>(record, 0x310) = 0x00CF95E0;
    at<std::uint32_t>(record, 0x3E4) = 0x00CF93F0;
    host.unit_tick_element_destruct_00875490(add(record, 0x3E4));
    projectile_shot_destruct_006e25a0(host, add(record, 0x310));
    host.unit_owner_entity_destruct_0077e380(record);
}

// 006E2AC0, __thiscall(record, lua, key), RET 8, SEH frame; the Lua property getter (slot 138h).
// 00779BB0(record, lua, key) first. Then for the key name [key+4] equal to "owner" (00CF9620,
// strcmp 00BF7FBF) with an owner [record+3BCh] whose vtable[5Ch](1) answers true: the owner (re-read)
// is pushed as thisTable[tostring(owner id +174h)] (004260B0, 00B67910 "thisTable" 00CE7494,
// 00B678E0, 00B663D0, the two LuaObject temporaries destroyed with 00B67700, and the string buffer
// returned with 00BD1510(ECX 00419CC0(), buffer, size + 1, 1)); with the owner gone after the
// slot call, 00B66430 pushes nil. Nothing is pushed when the slot answers false.
void projectile_get_property_owner_006e2ac0(Seg33aHost& host, void* record, void* lua, void* key_object) {
    host.unit_get_property_recon_level_00779bb0(record, lua, key_object);
    const char* const name = static_cast<const char*>(ptr(key_object, 4));
    if (name == nullptr || host.strcmp_00bf7fbf(name, kOwnerKey) != 0) return;
    if (ptr(record, 0x3BC) == nullptr || !host.unit_is_alive_vt5c(ptr(record, 0x3BC), 1)) return;
    void* const owner = ptr(record, 0x3BC);
    if (owner == nullptr) {
        host.lua_object_push_nil_00b66430(lua);
        return;
    }
    NativeStr id_text{};
    NativeStr* const text = host.native_string_from_int_004260b0(&id_text, at<std::uint16_t>(owner, 0x174));
    alignas(4) std::uint8_t table[0x14];
    alignas(4) std::uint8_t value[0x14];
    void* const table_object = host.lua_object_get_by_literal_00b67910(lua, table, kThisTableKey);
    void* const value_object = host.lua_object_get_by_native_string_00b678e0(table_object, value, text);
    host.lua_object_push_value_00b663d0(value_object);
    host.lua_object_destruct_00b67700(value);
    host.lua_object_destruct_00b67700(table);
    if (id_text.data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), id_text.data,
                                                      id_text.size + 1, 1);
    }
}

// 006E2C00, __thiscall(cls, a1, a2, const float* position, const float* direction, a5, a6, a7),
// RET 1Ch, SEH frame; the bomb class's slot 20h (create projectile). 468h bytes from 00BF55BE,
// cleared by 00BF79F0 before the null test, constructed by 006E2670(record, a1) when non-null;
// +54h = a2, +314h = cls; record->vtable[98h](0, [[00E188A8]+19CCh], identity); speed = [cls+50h]
// rounded to float, velocity +318h..+320h = (float)(direction[i] x speed); the local matrix
// +74h row 2 = direction then 0085DC80(+74h); +A4h..+ACh, shot +60h..+68h and shot +6Ch..+74h =
// position; +C8h = +10Ch = 0; 0042ED50 on every child of the chain [+48h] / +44h. Returns the
// shot sub-object record+310h. a5..a7 are not read. A null allocation faults in the image.
void* bomb_class_create_projectile_006e2c00(Seg33aHost& host, void* cls, std::uint32_t a1, void* a2,
                                            const float* position, const float* direction, std::int32_t,
                                            std::int32_t, std::int32_t) {
    void* const memory = host.operator_new_00bf55be(0x468);
    host.memset_00bf79f0(memory, 0, 0x468);
    void* const record = memory != nullptr
                             ? bomb_projectile_construct_006e2670(host, memory, reinterpret_cast<void*>(static_cast<std::uintptr_t>(a1)))
                             : nullptr;
    at<void*>(record, 0x54) = a2;
    at<void*>(record, 0x314) = cls;
    float identity[16];
    for (int i = 0; i < 16; ++i) identity[i] = (i % 5 == 0) ? kOne_00d7a24c : 0.0f;
    host.entity_attach_scene_vt98(record, 0, host.scene_root_00e188a8_19cc(), identity);
    const float speed = at<float>(ptr(record, 0x314), 0x50);
    at<float>(record, 0x318) = direction[0] * speed;
    at<float>(record, 0x31C) = direction[1] * speed;
    at<float>(record, 0x320) = direction[2] * speed;
    at<float>(record, 0x74 + 0x20) = direction[0];
    at<float>(record, 0x74 + 0x24) = direction[1];
    at<float>(record, 0x74 + 0x28) = direction[2];
    host.matrix_orthonormalize_basis_rows_0085dc80(static_cast<float*>(add(record, 0x74)));
    at<float>(record, 0xA4) = position[0];
    at<float>(record, 0xA8) = position[1];
    at<float>(record, 0xAC) = position[2];
    void* const shot = add(record, 0x310);
    at<float>(shot, 0x60) = position[0];
    at<float>(shot, 0x64) = position[1];
    at<float>(shot, 0x68) = position[2];
    at<float>(shot, 0x6C) = position[0];
    at<float>(shot, 0x70) = position[1];
    at<float>(shot, 0x74) = position[2];
    at<std::uint8_t>(record, 0xC8) = 0;
    at<std::uint8_t>(record, 0x10C) = 0;
    for (void* child = ptr(record, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);
    }
    return shot;
}

// 006E2E10, __thiscall(record), RET, SEH frame: the bomb projectile's activate.
//  1. Pose refresh; +354h = !(+100h > 0078CF20([[00E188A8]+19F0h], +FCh, +100h)) (FCOMIP/JBE, so
//     an unordered compare also sets it); 0077F0E0(record).
//  2. +464h = [cls+A8h]->vtable[8](0, 1.0f) with a model, else 0. node = [[+464h]+0Ch] (no null
//     test). With a parent +3Ch: 00B6E680(ECX node, parent->vtable[18h]()), then
//     [[+464h]+0Ch]->vtable[38h](record+74h); without: pose refresh, node->vtable[34h](+CCh),
//     00B6D890(ECX [[+464h]+0Ch], [[00E188A8]+19ECh]).
//  3. With a parent, not flying (+458h) and record->vtable[5Ch](31h): +459h = 0. Then
//     00B6DA70(ECX [[+464h]+0Ch], +459h ? 1.0f : 0.0f, 0).
//  4. record->vtable[10h]() == 0: 0048D6C0(record, [cls+14h] or "" (00E19982)).
//  5. Flying and session mode [[00E188A8]+1FE4h] == 2: shot->vtable[34h]((1,1,1), (0,0,0)).
//  6. Pose refresh; 004134F0(+418h, +CCh); element +3E4h vtable[0Ch]().
//  7. Saved state ([+C0h] with kind [[+C0h]+4] == 3): 009238A0(record, &tmp) and the reader
//     004425C0(&reader, tmp by value); enter "_bomb", enter "_bulletBase", 006E7850(+310h, reader),
//     leave, "flying" read as {3, &+458h} (00BD6830), leave, 00441A20(&reader).
//  8. The Lua class link: name = "Class" (0041DD40(&name, 5, 1), then memcpy of size + 1 bytes
//     from 00CE44C8 when the buffer exists); self = 00927B40(record); self[name] =
//     globals.Bullets[[cls+0Ch]] (00B67980 on [00E188A8]+1A0Ch, 00B67800 "Bullets",
//     00B67720, 00B675D0); the four LuaObjects destroyed (00B67700) newest first; the name
//     buffer returned to the pool (00BD1510(00419CC0(), data, size + 1, 1)).
void bomb_projectile_activate_006e2e10(Seg33aHost& host, void* record) {
    refresh_pose_if_stale(host, record);
    const float water = host.game_world_sample_water_height_0078cf20(host.game_world_00e188a8_19f0(),
                                                                       at<float>(record, 0xFC), at<float>(record, 0x100));
    const bool above = at<float>(record, 0x100) > water;
    at<std::uint8_t>(record, 0x354) = above ? 0 : 1;
    host.unit_owner_entity_activate_and_register_0077f0e0(record);

    void* const model = ptr(ptr(record, 0x314), 0xA8);
    at<void*>(record, 0x464) = model != nullptr ? host.model_create_instance_vt08(model, 0, kOne_00d7a24c) : nullptr;
    void* const node = ptr(ptr(record, 0x464), 0x0C);
    if (ptr(record, 0x3C) != nullptr) {
        host.node_set_parent_00b6e680(node, host.entity_scene_node_vt18(ptr(record, 0x3C)));
        host.scene_object_set_local_matrix_vt38(ptr(ptr(record, 0x464), 0x0C), add(record, 0x74));
    } else {
        refresh_pose_if_stale(host, record);
        host.scene_object_set_matrix_vt34(node, static_cast<const float*>(add(record, 0xCC)));
        host.node_propagate_root_registration_00b6d890(ptr(ptr(record, 0x464), 0x0C), host.effect_manager_00e188a8_19ec());
    }
    if (ptr(record, 0x3C) != nullptr && at<std::uint8_t>(record, 0x458) == 0 && host.unit_is_alive_vt5c(record, 0x31)) {
        at<std::uint8_t>(record, 0x459) = 0;
    }
    host.scene_node_set_visibility_factor_00b6da70(ptr(ptr(record, 0x464), 0x0C),
                                                   at<std::uint8_t>(record, 0x459) != 0 ? kOne_00d7a24c : 0.0f, 0);
    if (host.entity_has_name_vt10(record) == 0) {
        const char* name = static_cast<const char*>(ptr(ptr(record, 0x314), 0x14));
        if (name == nullptr) name = host.empty_string_00e19982();
        host.scene_entity_set_name_0048d6c0(record, name);
    }
    void* const flying = add(record, 0x458);
    if (at<std::uint8_t>(flying, 0) != 0 && host.session_mode_00e188a8_1fe4() == 2) {
        const float zero[3] = {0.0f, 0.0f, 0.0f};
        const float ones[3] = {kOne_00d7a24c, kOne_00d7a24c, kOne_00d7a24c};
        host.shot_set_vectors_vt34(add(record, 0x310), ones, zero);
    }
    refresh_pose_if_stale(host, record);
    host.matrix_copy_4x4_x87_004134f0(add(record, 0x418), add(record, 0xCC));
    host.tick_element_activate_vt0c(add(record, 0x3E4));

    void* const saved = ptr(record, 0xC0);
    if (saved != nullptr && at<std::int32_t>(saved, 4) == 3) {
        alignas(4) std::uint8_t lua_data[0x14];
        alignas(4) std::uint8_t reader[0x14];
        host.entity_saved_lua_data_009238a0(record, lua_data);
        host.lua_reader_construct_004425c0(reader, lua_data);
        host.lua_reader_enter_key_00bd8e20(reader, key(kBombSectionKey));
        host.lua_reader_enter_key_00bd8e20(reader, key(kBulletBaseSectionKey));
        bullet_base_read_state_006e7850(host, add(record, 0x310), reader);
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_read_field_00bd6830(reader, key(kFlyingKey), value_arg(3, flying));
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_destroy_00441a20(reader);
    }

    NativeStr name{0u, nullptr};
    host.native_string_resize_0041dd40(&name, 5, 1);
    if (name.data != nullptr) host.memcpy_00bf7680(name.data, kClassKey, name.size + 1);
    alignas(4) std::uint8_t globals[0x14];
    alignas(4) std::uint8_t bullets[0x14];
    alignas(4) std::uint8_t entry[0x14];
    alignas(4) std::uint8_t self_table[0x14];
    void* const globals_object = host.lua_state_owner_get_globals_00b67980(host.lua_state_owner_00e188a8_1a0c(), globals);
    void* const bullets_object = host.lua_object_get_by_name_00b67800(globals_object, bullets, kBulletsKey);
    void* const entry_object = host.lua_object_get_by_index_00b67720(
        bullets_object, entry, at<std::int32_t>(ptr(record, 0x314), 0x0C));
    void* const self_object = host.mission_entity_lua_self_object_00927b40(record, self_table);
    host.lua_object_set_object_00b675d0(self_object, &name, entry_object);
    host.lua_object_destruct_00b67700(self_table);
    host.lua_object_destruct_00b67700(entry);
    host.lua_object_destruct_00b67700(bullets);
    host.lua_object_destruct_00b67700(globals);
    if (name.data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), name.data, name.size + 1, 1);
    }
}

// 006E3350, __thiscall(rack), RET (tail-jumps slot 224h). 0072D7E0(rack); with an owner
// [rack+3F0h] whose vtable[5Ch](0Fh) answers true and a loadout count [owner+C54h] > 0, the
// list 009552E0(ECX [owner+538h], count) is walked (node +4 next, +8 entry): an entry with
// rounds [entry+8] > 0 for this rack's class id ([[rack+3F4h]+6Ch] == [entry+4]) calls
// rack->vtable[1BCh](rounds) then rack->vtable[1C0h]([entry+0Ch] float), the entry re-read from
// the node. Then unless [rack+C0h] is a kind-3 record, rack->vtable[224h]().
void mbomb_platform_bind_owner_loadout_006e3350(Seg33aHost& host, void* rack) {
    host.gun_base_init_device_0072d7e0(rack);
    void* const owner = ptr(rack, 0x3F0);
    if (owner != nullptr && host.unit_is_alive_vt5c(owner, 0x0F)) {
        void* const unit = ptr(rack, 0x3F0);
        const std::int32_t count = at<std::int32_t>(unit, 0xC54);
        if (count > 0) {
            void* const list = host.class_loadout_record_009552e0(ptr(unit, 0x538), count);
            for (void* node = ptr(list, 4); node != nullptr; node = ptr(node, 4)) {
                void* const entry = ptr(node, 8);
                const std::int32_t rounds = at<std::int32_t>(entry, 8);
                if (rounds > 0 && at<std::int32_t>(ptr(rack, 0x3F4), 0x6C) == at<std::int32_t>(entry, 4)) {
                    host.rack_set_ammo_vt1bc(rack, rounds);
                    host.rack_set_reload_time_vt1c0(rack, at<float>(ptr(node, 8), 0x0C));
                }
            }
        }
    }
    void* const saved = ptr(rack, 0xC0);
    if (saved != nullptr && at<std::int32_t>(saved, 4) == 3) return;
    host.rack_attach_next_round_vt224(rack);
}

// 006E3460, __thiscall(rack, flag), RET 4 -> AL. 00729A80(rack, flag) false: false; true: the
// answer of rack->vtable[210h](2Ah, 0) in tail position. That slot answers a bullet-class pointer
// and the callers test AL, so only its low byte decides (a pointer with a zero low byte reads as
// false in the image too).
bool mbomb_platform_can_fire_006e3460(Seg33aHost& host, void* rack, std::int32_t flag) {
    if (!host.gun_can_fire_00729a80(rack, flag)) return false;
    return (reinterpret_cast<std::uintptr_t>(host.rack_round_of_kind_vt210(rack, 0x2A, 0)) & 0xFFu) != 0;
}

// 006E3490, __thiscall(platform, kind), RET 4. The first child on the chain [+48h] / +44h whose
// vtable[5Ch](kind) answers true, or null.
void* bomb_platform_find_child_of_kind_006e3490(Seg33aHost& host, void* platform, std::int32_t kind) {
    for (void* child = ptr(platform, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        if (host.unit_is_alive_vt5c(child, kind)) return child;
    }
    return nullptr;
}

// 006E34D0, __thiscall(platform, kind), RET 4: how many children on that chain answer true.
std::int32_t bomb_platform_count_children_of_kind_006e34d0(Seg33aHost& host, void* platform, std::int32_t kind) {
    std::int32_t count = 0;
    for (void* child = ptr(platform, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        if (host.unit_is_alive_vt5c(child, kind)) count += 1;
    }
    return count;
}

// 006E38E0, __thiscall(session), RET: 1 when [+F4h] is 0 or 1, else 0 (whole EAX).
bool session_mode_field_is_zero_or_one_006e38e0(void* session) {
    const std::int32_t mode = at<std::int32_t>(session, 0xF4);
    return mode == 0 || mode == 1;
}

// ---------------------------------------------------------------------------
// Bomb platform (rack, vtable 00CF96A8) messages and ordnance queries.
// ---------------------------------------------------------------------------

// 006E3940, __thiscall(message, std::uint16_t value), RET 4. 0075B430(message, ADh); word +18h = 0,
// +1Ah = +1Ch = 0 (bytes), word +20h = value, +4h = 1, vtable 00CF9628. Returns message.
void* gun_fire_message_construct_006e3940(Seg33aHost& host, void* message, std::uint16_t value) {
    host.session_message_construct_base_0075b430(message, 0xAD);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint16_t>(message, 0x20) = value;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CF9628;
    return message;
}

// 006E3A60, __thiscall(message, std::int32_t value), RET 4. The same shape for type B2h, with the
// dword +20h = value and vtable 00CF963C.
void* bomb_platform_reload_message_construct_006e3a60(Seg33aHost& host, void* message, std::int32_t value) {
    host.session_message_construct_base_0075b430(message, 0xB2);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::int32_t>(message, 0x20) = value;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CF963C;
    return message;
}

// 006E3DC0, __thiscall(gun, float* out, const float* in), RET 8; gun slot 1E0h of the rack: out = in
// (the muzzle origin needs no transform here). ECX is not read.
void slot_copy_vec3_identity_006e3dc0(void*, float* out, const float* in) {
    out[0] = in[0];
    out[1] = in[1];
    out[2] = in[2];
}

// 006E3DE0, __thiscall(gun, arg), RET 4: gun slot 1E4h of the rack, empty.
void stub_ret4_006e3de0(void*, void*) {}

namespace {

// The rack's loadout answer shared by 006E3FE0 / 006E4060 / 006E40F0: the fire block
// [rack+3F8h] and its descriptor [block+34h], asked through descriptor->vtable[8](kind).
bool rack_descriptor_is_kind(Seg33aHost& host, void* rack, std::int32_t kind) {
    void* const block = ptr(rack, 0x3F8);
    if (block == nullptr || ptr(block, 0x34) == nullptr) return false;
    return host.bullet_descriptor_is_kind_vt08(ptr(block, 0x34), kind);
}

}  // namespace

// 006E3FE0, __thiscall(rack, kind, char loadout), RET 8 -> AL. True when an attached child round
// (chain [+48h] / +44h) answers vtable[5Ch](kind); otherwise, with loadout set and the owner
// [+3F0h]->vtable[1E4h]() true, when the rack's descriptor answers vtable[8](kind).
bool mbomb_platform_has_round_of_kind_006e3fe0(Seg33aHost& host, void* rack, std::int32_t kind, std::uint8_t loadout) {
    for (void* child = ptr(rack, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        if (host.unit_is_alive_vt5c(child, kind)) return true;
    }
    if (loadout == 0 || !host.unit_loadout_answers_vt1e4(ptr(rack, 0x3F0))) return false;
    return rack_descriptor_is_kind(host, rack, kind);
}

// 006E4060, __thiscall(rack, char loadout), RET 4 -> the loaded bullet class. The first child that
// is kind 2Ah answers its [child+314h]; otherwise, under the same loadout gate, the descriptor
// [[rack+3F8h]+34h] when it answers vtable[8](2Ah); else null.
void* mbomb_platform_loaded_ordnance_006e4060(Seg33aHost& host, void* rack, std::uint8_t loadout) {
    for (void* child = ptr(rack, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        if (host.unit_is_alive_vt5c(child, 0x2A)) return ptr(child, 0x314);
    }
    if (loadout == 0 || !host.unit_loadout_answers_vt1e4(ptr(rack, 0x3F0))) return nullptr;
    if (!rack_descriptor_is_kind(host, rack, 0x2A)) return nullptr;
    return ptr(ptr(rack, 0x3F8), 0x34);
}

// 006E40F0, __fastcall(rack /*ECX*/), RET: the descriptor [[rack+3F8h]+34h] when it answers
// vtable[8](2Ah), else null.
void* unit_bomb_rack_loaded_device_006e40f0(Seg33aHost& host, void* rack) {
    if (!rack_descriptor_is_kind(host, rack, 0x2A)) return nullptr;
    return ptr(ptr(rack, 0x3F8), 0x34);
}

// 006E4290, __thiscall(rack, message), RET 4 -> AL = 1; the rack's slot 164h. Type +10h: B1h ->
// rack->vtable[228h](message); B2h -> rack->vtable[200h]() (nothing pushed); else the gun's
// 0072D830(rack, message).
bool mbomb_platform_handle_message_006e4290(Seg33aHost& host, void* rack, void* message) {
    const std::uint8_t type = at<std::uint8_t>(message, 0x10);
    if (type == 0xB1) {
        host.rack_apply_fire_message_vt228(rack, message);
    } else if (type == 0xB2) {
        host.rack_rearm_vt200(rack);
    } else {
        host.gun_handle_network_message_0072d830(rack, message);
    }
    return true;
}

// 006E4360, __thiscall(rack) -> AL, RET: the multiple rack's busy slot answers false.
bool mmultiple_bomb_platform_is_busy_006e4360(void*) { return false; }

// 006E4640, __thiscall(rack, char loadout), RET 4. 006E4060(rack, loadout) first; null: the first
// non-null display slot of the forty at +514h answers 00472520(ECX [slot+188h]); none: null.
void* mmultiple_bomb_platform_loaded_ordnance_006e4640(Seg33aHost& host, void* rack, std::uint8_t loadout) {
    void* const loaded = mbomb_platform_loaded_ordnance_006e4060(host, rack, loadout);
    if (loaded != nullptr) return loaded;
    for (int i = 0; i < 0x28; ++i) {
        void* const slot = ptr(rack, 0x514 + 4 * static_cast<std::size_t>(i));
        if (slot != nullptr) return host.trivial_getter_00472520(ptr(slot, 0x188));
    }
    return nullptr;
}

// 006E47E0, __thiscall(message, unused, std::uint16_t id, object, const float* a, const float* b),
// RET 14h. 0075B430(message, B1h); word +18h = 0, +1Ah = +1Ch = 0, word +20h = id, +4h = 1, vtable
// 00CF9B44; +24h..+2Ch = object +30h..+38h; 0042D2E0(ECX object, EDX &+30h, &+34h, &+38h) writes the
// three basis angles; +3Ch..+44h = a; +48h..+50h = b. The first stack argument is not read.
// Returns message.
void* bomb_platform_fire_message_construct_006e47e0(Seg33aHost& host, void* message, std::uint32_t,
                                                     std::uint16_t id, void* object, const float* a,
                                                     const float* b) {
    host.session_message_construct_base_0075b430(message, 0xB1);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint16_t>(message, 0x20) = id;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CF9B44;
    at<float>(message, 0x24) = at<float>(object, 0x30);
    at<float>(message, 0x28) = at<float>(object, 0x34);
    at<float>(message, 0x2C) = at<float>(object, 0x38);
    host.matrix_extract_normalized_basis_angles_0042d2e0(object, static_cast<float*>(add(message, 0x30)),
                                                         static_cast<float*>(add(message, 0x34)),
                                                         static_cast<float*>(add(message, 0x38)));
    at<float>(message, 0x3C) = a[0];
    at<float>(message, 0x40) = a[1];
    at<float>(message, 0x44) = a[2];
    at<float>(message, 0x48) = b[0];
    at<float>(message, 0x4C) = b[1];
    at<float>(message, 0x50) = b[2];
    return message;
}

// 006E4A60, __thiscall(rack, archive), RET 4. 0072ADC0(rack, archive); section "_bombPlatform";
// "ammo" {1, +484h}, "orgAmmo" {1, +488h}, "toRepeatTime" {2, float +494h}, "dropBombs" {3, byte
// +498h}; close.
void mbomb_platform_serialize_006e4a60(Seg33aHost& host, void* rack, void* archive) {
    host.gun_debug_dump_fields_0072adc0(rack, archive);
    host.archive_open_section_vt04(archive, key(kBombPlatformSectionKey));
    host.archive_write_field_vt0c(archive, key(kAmmoKey), ArchiveArg{1u, at<std::uint32_t>(rack, 0x484)});
    host.archive_write_field_vt0c(archive, key(kOrgAmmoKey), ArchiveArg{1u, at<std::uint32_t>(rack, 0x488)});
    host.archive_write_field_vt0c(archive, key(kToRepeatTimeKey), float_arg(at<float>(rack, 0x494)));
    host.archive_write_field_vt0c(archive, key(kDropBombsKey), ArchiveArg{3u, at<std::uint8_t>(rack, 0x498)});
    host.archive_close_section_vt08(archive);
}

// 006E4B60, __thiscall(rack, message), RET 4. 004254B0("MBombPlatform - fire msg received");
// rack->vtable[1D8h](0, 0.0f, 0.0f) (the format slot is reused for the second float); the round is
// the entity of handle word +20h through the inlined table lookup (no zero-id test here); found:
// 004254B0(".. bomb founded: %s", [[round+314h]+14h] or "" (00E19984)), then 006E16D0(round,
// message); not found: 004254B0(".. BUT bomb NOT founded!!!").
void mbomb_platform_apply_fire_message_006e4b60(Seg33aHost& host, void* rack, void* message) {
    host.diagnostic_004254b0(kFireMsgReceived, nullptr);
    host.rack_fire_vt1d8(rack, 0, 0.0f, 0.0f);
    const HandleTables tables = host.entity_handle_tables_00f89a0c();
    const std::int32_t id = at<std::uint16_t>(message, 0x20);
    void* const round = id < tables.split
                            ? at<void*>(tables.low_entries, static_cast<std::size_t>((id - tables.low_base) << 4) + 0xC)
                            : at<void*>(tables.high_entries, static_cast<std::size_t>((id - tables.high_base) << 4) + 0xC);
    if (round == nullptr) {
        host.diagnostic_004254b0(kFireMsgBombNotFound, nullptr);
        return;
    }
    const char* name = static_cast<const char*>(ptr(ptr(round, 0x314), 0x14));
    if (name == nullptr) name = host.empty_string_00e19984();
    host.diagnostic_004254b0(kFireMsgBombFound, name);
    bomb_projectile_apply_launch_state_006e16d0(host, round, message);
}

// 006E4C10, __fastcall(rack /*ECX*/) -> AL, RET, SEH frame; the rack's slot 1DCh. fired =
// 00727E30(rack). When fired and the session mode [[00E188A8]+1FE4h] is 0 or 1: a stack
// BOMBPLATFORM_FIRE message 006E47E0(msg, rack, word +49Ah, rack+4A0h, rack+4E0h, rack+4ECh) is
// broadcast with 0077C7B0(rack, msg, 0) and dropped (vtable re-stamped to 00CE4974, no call);
// rack->vtable[1F4h]([+49Ch]); then 004254B0("MBombPlatform::Fire: fire msg sent, (%s)", the
// round's class name, "" (00E19984) without one, or "unnamed" (00CE391C) when the handle +49Ah
// resolves to nothing). Returns fired.
bool mbomb_platform_fire_if_ready_006e4c10(Seg33aHost& host, void* rack) {
    const bool fired = host.gun_fire_if_ready_00727e30(rack);
    if (!fired) return fired;
    const std::int32_t mode = host.session_mode_00e188a8_1fe4();
    if (mode != 0 && mode != 1) return fired;
    alignas(4) std::uint8_t message[0x54];
    void* const msg = bomb_platform_fire_message_construct_006e47e0(
        host, message, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(rack)),
        at<std::uint16_t>(rack, 0x49A), add(rack, 0x4A0), static_cast<const float*>(add(rack, 0x4E0)),
        static_cast<const float*>(add(rack, 0x4EC)));
    host.session_owner_broadcast_message_0077c7b0(rack, msg, 0);
    at<std::uint32_t>(message, 0) = 0x00CE4974;
    host.rack_fire_slot_vt1f4(rack, at<std::int32_t>(rack, 0x49C));
    const HandleTables tables = host.entity_handle_tables_00f89a0c();
    const std::int32_t id = at<std::uint16_t>(rack, 0x49A);
    void* const round = id < tables.split
                            ? at<void*>(tables.low_entries, static_cast<std::size_t>((id - tables.low_base) << 4) + 0xC)
                            : at<void*>(tables.high_entries, static_cast<std::size_t>((id - tables.high_base) << 4) + 0xC);
    const char* name = kUnnamed_00ce391c;
    if (round != nullptr) {
        name = static_cast<const char*>(ptr(ptr(round, 0x314), 0x14));
        if (name == nullptr) name = host.empty_string_00e19984();
    }
    host.diagnostic_004254b0(kFireMsgSent, name);
    return fired;
}

// 006E4D30, __fastcall(entity /*ECX*/), RET: entity when non-null and vtable[5Ch](0Fh) answers
// true, else null.
void* entity_as_kind_0f_006e4d30(Seg33aHost& host, void* entity) {
    if (entity != nullptr && host.unit_is_alive_vt5c(entity, 0x0F)) return entity;
    return nullptr;
}

// 006E5A80, __thiscall(entity, index), RET 4. The pointer array at +94h ({data, count +98h, ...}):
// an index at or past the count first grows it with 005471B0(ECX entity+94h, index + 1); returns
// data[index] + 4Ch.
void* entity_child_local_frame_006e5a80(Seg33aHost& host, void* entity, std::int32_t index) {
    void* const array = add(entity, 0x94);
    if (index >= at<std::int32_t>(entity, 0x98)) host.ref_counted_pointer_array_resize_005471b0(array, index + 1);
    return add(at<void*>(ptr(array, 0), static_cast<std::size_t>(index) * 4), 0x4C);
}

// 006E6010, __thiscall(rack), RET, SEH frame; the rack's load-state slot. 0072F060(rack); with a
// kind-3 saved record [+C0h]: the reader (009238A0 / 004425C0), enter "_bombPlatform", read "ammo"
// {1, &+484h}, "orgAmmo" {1, &+488h}, "toRepeatTime" {2, &+494h}, "dropBombs" {3, &+498h}, leave,
// 00441A20. Then every attached child (+48h / +44h) takes +58h = [[rack+3F0h]+58h]; then
// 00729700(rack).
void bomb_platform_load_saved_state_006e6010(Seg33aHost& host, void* rack) {
    host.gun_load_saved_state_0072f060(rack);
    void* const saved = ptr(rack, 0xC0);
    if (saved != nullptr && at<std::int32_t>(saved, 4) == 3) {
        alignas(4) std::uint8_t lua_data[0x14];
        alignas(4) std::uint8_t reader[0x14];
        host.entity_saved_lua_data_009238a0(rack, lua_data);
        host.lua_reader_construct_004425c0(reader, lua_data);
        host.lua_reader_enter_key_00bd8e20(reader, key(kBombPlatformSectionKey));
        host.lua_reader_read_field_00bd6830(reader, key(kAmmoKey), value_arg(1, add(rack, 0x484)));
        host.lua_reader_read_field_00bd6830(reader, key(kOrgAmmoKey), value_arg(1, add(rack, 0x488)));
        host.lua_reader_read_field_00bd6830(reader, key(kToRepeatTimeKey), value_arg(2, add(rack, 0x494)));
        host.lua_reader_read_field_00bd6830(reader, key(kDropBombsKey), value_arg(3, add(rack, 0x498)));
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_destroy_00441a20(reader);
    }
    for (void* child = ptr(rack, 0x48); child != nullptr; child = ptr(child, 0x44)) {
        at<std::uint32_t>(child, 0x58) = at<std::uint32_t>(ptr(rack, 0x3F0), 0x58);
    }
    host.gun_apply_descriptor_node_flag_00729700(rack);
}

// 006E6210, __fastcall(crew /*ECX*/), RET -> EAX: crew level to skill index, with mp = session mode
// [[00E188A8]+1FE4h] != 0: 0 -> mp; 1 -> 1 + 2*mp; 2 -> 2 + 2*mp; 3 -> 5; anything else -> 1.
std::int32_t skill_from_crew_level_006e6210(Seg33aHost& host, std::int32_t crew) {
    if (crew == 0) return host.session_mode_00e188a8_1fe4() != 0 ? 1 : 0;
    if (crew == 1) return host.session_mode_00e188a8_1fe4() != 0 ? 3 : 1;
    if (crew == 2) return host.session_mode_00e188a8_1fe4() != 0 ? 4 : 2;
    return crew == 3 ? 5 : 1;
}

// 006E6270, __fastcall(skill /*ECX*/), RET -> EAX: skill index to crew level, mp as above: 0 -> 0;
// 1 -> !mp; 2 -> 2 - mp; 3 -> 2 - mp; 4 -> 3 - mp; 5 -> 3; anything else -> 1.
std::int32_t crew_level_from_skill_006e6270(Seg33aHost& host, std::int32_t skill) {
    if (skill == 0) return 0;
    if (skill == 1) return host.session_mode_00e188a8_1fe4() == 0 ? 1 : 0;
    if (skill == 2) return host.session_mode_00e188a8_1fe4() != 0 ? 1 : 2;
    if (skill == 3) return host.session_mode_00e188a8_1fe4() != 0 ? 1 : 2;
    if (skill == 4) return host.session_mode_00e188a8_1fe4() != 0 ? 2 : 3;
    return skill == 5 ? 3 : 1;
}

// 006E6360, __thiscall(skill, reader), RET 4. reader->vtable[10h]({0, "skilllevel"}, {1, &level})
// then skill->vtable[0](level). The local is the PUSH ECX slot, so an unread level is the skill
// object's own address, as in the image.
void skill_level_read_006e6360(Seg33aHost& host, void* skill, void* reader) {
    std::int32_t level = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(skill));
    host.archive_read_field_vt10(reader, key(kSkillLevelKey), value_arg(1, &level));
    host.skill_set_level_vt00(skill, level);
}

// 006E63A0, __thiscall(skill, archive), RET 4: archive->vtable[0Ch]({0, "skilllevel"}, {1, [skill+4]}).
void skill_level_write_006e63a0(Seg33aHost& host, void* skill, void* archive) {
    host.archive_write_field_vt0c(archive, key(kSkillLevelKey), ArchiveArg{1u, at<std::uint32_t>(skill, 4)});
}

// 006E6410, __thiscall(shot, unused), RET 4: leave water. With +44h set: +44h = 0, then the motor
// effect +28h gets 00866B70 (request stop) and the trail +24h gets 00867B10 (stop children), each
// when present.
void projectile_shot_set_air_mode_006e6410(Seg33aHost& host, void* shot, void*) {
    if (at<std::uint8_t>(shot, 0x44) == 0) return;
    at<std::uint8_t>(shot, 0x44) = 0;
    if (ptr(shot, 0x28) != nullptr) host.effect_handle_request_stop_00866b70(ptr(shot, 0x28));
    if (ptr(shot, 0x24) != nullptr) host.point_effect_stop_children_00867b10(ptr(shot, 0x24));
}

// 006E6450, __thiscall(shot, unused), RET 4: enter water. With +44h clear: +44h = 1, then +28h gets
// 00867B10 and +24h gets 00866B70 (the reverse pairing), each when present.
void projectile_shot_set_water_mode_006e6450(Seg33aHost& host, void* shot, void*) {
    if (at<std::uint8_t>(shot, 0x44) != 0) return;
    at<std::uint8_t>(shot, 0x44) = 1;
    if (ptr(shot, 0x28) != nullptr) host.point_effect_stop_children_00867b10(ptr(shot, 0x28));
    if (ptr(shot, 0x24) != nullptr) host.effect_handle_request_stop_00866b70(ptr(shot, 0x24));
}

// 006E6670, __thiscall(member, const other), RET 4, SEH frame: copy construction. +4h..+Ch = 0,
// +10h = 0, vtable 00CF9348, +14h = other+14h, +10h = 1, 00694A60(subject, member) when set.
void* observer_member_9348_copy_construct_006e6670(Seg33aHost& host, void* member, const void* other) {
    at<std::uint32_t>(member, 0x4) = 0;
    at<std::uint32_t>(member, 0x8) = 0;
    at<std::uint32_t>(member, 0xC) = 0;
    at<std::uint8_t>(member, 0x10) = 0;
    at<std::uint32_t>(member, 0) = 0x00CF9348;
    void* const subject = *reinterpret_cast<void* const*>(static_cast<const std::uint8_t*>(other) + 0x14);
    at<void*>(member, 0x14) = subject;
    at<std::uint8_t>(member, 0x10) = 1;
    if (subject != nullptr) host.observer_register_pair_00694a60(subject, member);
    return member;
}

// 006E6810, __thiscall(archive, ArchiveArg name, member by value (18h bytes)), RET 20h, SEH frame.
// archive->vtable[0Ch](name, {4, member.+14h}), then the by-value copy is destroyed with 006E0860.
void save_writer_write_observer_ref_9348_006e6810(Seg33aHost& host, void* archive, ArchiveArg name,
                                                  void* member_copy) {
    host.archive_write_field_vt0c(archive, name, ArchiveArg{4u, at<std::uintptr_t>(member_copy, 0x14)});
    observer_member_9348_destruct_006e0860(host, member_copy);
}

// 006E6900, __thiscall(shot, float dt), RET 4. +94h = (float)(+94h - dt) (FSUB then FSTP/FLD
// float); when 0 > the new value (FLDZ; FCOMIP; JBE), the member at +B4h drops its subject +C8h
// (006952A0, then +C8h = 0).
void projectile_shot_advance_owner_grace_006e6900(Seg33aHost& host, void* shot, float dt) {
    const float grace = static_cast<float>(static_cast<double>(at<float>(shot, 0x94)) - static_cast<double>(dt));
    at<float>(shot, 0x94) = grace;
    if (0.0f > grace) {
        void* const member = add(shot, 0xB4);
        if (ptr(member, 0x14) != nullptr) {
            host.observer_unregister_pair_006952a0(ptr(member, 0x14), member);
            at<void*>(member, 0x14) = nullptr;
        }
    }
}

// 006E6940, __thiscall(object), RET, tail-jumps 00923060. The effect +194h is stopped (00867B10),
// marked +9 = 1, released and cleared; the effect +198h is moved to [+23Ch]+FCh (pose refresh,
// 004842C0), finished with 00867790(0.0f, [[00E188A8]+19FCh]), stopped, marked, released and
// cleared; the node +280h gets vtable[4](1) and is cleared; then 00923060(object).
void bullet_stop_trail_effect_006e6940(Seg33aHost& host, void* object) {
    release_point_effect(host, object, 0x194);
    if (ptr(object, 0x198) != nullptr) {
        void* const anchor = ptr(object, 0x23C);
        refresh_pose_if_stale(host, anchor);
        host.point_effect_set_world_point_004842c0(ptr(object, 0x198), static_cast<const float*>(add(anchor, 0xFC)));
        host.effect_update_children_00867790(ptr(object, 0x198), 0.0f, host.effect_reference_node_00e188a8_19fc());
        release_point_effect(host, object, 0x198);
    }
    if (ptr(object, 0x280) != nullptr) {
        host.object_deleting_destructor_vt04(ptr(object, 0x280), 1);
        at<void*>(object, 0x280) = nullptr;
    }
    host.release_sub_objects_body_00923060(object);
}

// 006E6A60, (ECX reader, EDX member, ArchiveArg name), RET 8. reader->vtable[10h](name,
// {4, &subject}) (the local starts as the PUSH ECX slot, the reader's address), then the member is
// retargeted to subject (006952A0 / 00694A60).
void save_reader_read_observer_ref_006e6a60(Seg33aHost& host, void* reader, void* member, ArchiveArg name) {
    void* subject = reader;
    host.archive_read_field_vt10(reader, name, value_arg(4, &subject));
    observer_member_retarget(host, member, subject);
}

// 006E6B10, __thiscall(member, reader), RET 4. reader->vtable[10h]({0, "id"}, {1, &id}) (the local
// starts as the member's address); word +18h = id; a non-zero id resolves through the inlined
// handle lookup and the member is retargeted to it; id 0 leaves the subject as it was.
void save_reader_read_unit_handle_006e6b10(Seg33aHost& host, void* member, void* reader) {
    std::uint32_t id = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(member));
    host.archive_read_field_vt10(reader, key(kIdKey), value_arg(1, &id));
    at<std::uint16_t>(member, 0x18) = static_cast<std::uint16_t>(id);
    if (static_cast<std::uint16_t>(id) == 0) return;
    observer_member_retarget(host, member, resolve_entity_handle(host, static_cast<std::uint16_t>(id)));
}

namespace {

// One attached-effect creation of 006E6C20: 00868420(ECX &out, EDX [[00E188A8]+19ECh], descriptor
// (by-value reference, incremented here and consumed by the callee), [shot+CCh]+74h, flag, 0).
void* const* create_attached_effect(Seg33aHost& host, void* shot, void** out, void* descriptor, std::uint32_t flag) {
    void* const manager = host.effect_manager_00e188a8_19ec();
    if (descriptor != nullptr) ref_add(host, descriptor);
    return host.point_effect_create_from_matrix_00868420(out, manager, descriptor, add(ptr(shot, 0xCC), 0x74), flag, 0);
}

}  // namespace

// 006E6C20, __thiscall(shot), RET, SEH frame; shot slot 3Ch (spawn). Class = [shot+4].
//  - With [class+40h]: the descriptor is [class+44h] when +5Ch is set and that exists, else
//    [class+40h]; the effect is created with flag = byte +44h and assigned to +28h through
//    004845D0(&+28h, &out); the temporary is released; then with +28h set,
//    00867790(+28h, 0.0f, [[00E188A8]+19FCh]).
//  - With [class+48h]: created with flag = (+44h == 0) (SETZ into AL; the image pushes EAX with
//    stale upper bytes, here zero) and assigned inline to +24h (add new, release old), the
//    temporary released; then with +24h set, +24h+50h = (float)(1 / class->vtable[1Ch]())
//    (FLD1; FDIVRP DE F1) and 00867790(+24h, 0.0f, [[00E188A8]+19FCh]).
void projectile_shot_on_spawn_006e6c20(Seg33aHost& host, void* shot) {
    void* const cls = ptr(shot, 4);
    if (ptr(cls, 0x40) != nullptr) {
        void* const descriptor =
            (at<std::uint8_t>(shot, 0x5C) != 0 && ptr(cls, 0x44) != nullptr) ? ptr(cls, 0x44) : ptr(cls, 0x40);
        void* created = nullptr;
        void* const* result = create_attached_effect(host, shot, &created, descriptor, at<std::uint8_t>(shot, 0x44));
        host.retained_reference_assign_004845d0(add(shot, 0x28), result);
        if (created != nullptr) ref_release(host, created);
        if (ptr(shot, 0x28) != nullptr) {
            host.effect_update_children_00867790(ptr(shot, 0x28), 0.0f, host.effect_reference_node_00e188a8_19fc());
        }
    }
    void* const cls2 = ptr(shot, 4);
    if (ptr(cls2, 0x48) == nullptr) return;
    void* created = nullptr;
    void* const* result = create_attached_effect(host, shot, &created, ptr(cls2, 0x48),
                                                 at<std::uint8_t>(shot, 0x44) == 0 ? 1u : 0u);
    void* const effect = *result;
    void* const old = ptr(shot, 0x24);
    if (old != effect) {
        at<void*>(shot, 0x24) = effect;
        if (effect != nullptr) ref_add(host, effect);
        if (old != nullptr) ref_release(host, old);
    }
    if (created != nullptr) {
        ref_release(host, created);
        created = nullptr;
    }
    void* const trail = ptr(shot, 0x24);
    if (trail == nullptr) return;
    const double rate = host.projectile_class_rate_vt1c(ptr(shot, 4));
    at<float>(trail, 0x50) = static_cast<float>(1.0 / rate);
    host.effect_update_children_00867790(ptr(shot, 0x24), 0.0f, host.effect_reference_node_00e188a8_19fc());
}

// 006E6E80, __thiscall(bullet_base, archive), RET 4: the bullet-base writer (fields through slot
// 0Ch). "bbv" {5, &+8h}, "bberror" {5, &+38h}; section "owner" with "id" {1, word +B0h}, closed;
// "collOwner" through 006E6810 with a by-value copy of the member +B4h (006E6670, then its vtable
// set to 00CF938C); "ownerTime" {2, +94h}, "ownerType" {1, +14h}, "partyIndex" {1, +18h},
// "playerIndex" {1, +1Ch}, "hitAnything" {3, +20h}, "underWater" {3, +44h}, "flyTime" {2, +54h},
// "penetration" {2, +58h}, "hitDamageMul" {2, +D0h}, "tracers" {3, +28h or +24h set}.
void bullet_base_write_state_006e6e80(Seg33aHost& host, void* base, void* archive) {
    host.archive_write_field_vt0c(archive, key(kBbvKey), value_arg(5, add(base, 0x8)));
    host.archive_write_field_vt0c(archive, key(kBbErrorKey), value_arg(5, add(base, 0x38)));
    host.archive_open_section_vt04(archive, key(kOwnerKey));
    host.archive_write_field_vt0c(archive, key(kIdKey), ArchiveArg{1u, at<std::uint16_t>(base, 0xB0)});
    host.archive_close_section_vt08(archive);
    alignas(4) std::uint8_t member_copy[0x18];
    observer_member_9348_copy_construct_006e6670(host, member_copy, add(base, 0xB4));
    at<std::uint32_t>(member_copy, 0) = 0x00CF938C;
    save_writer_write_observer_ref_9348_006e6810(host, archive, key(kCollOwnerKey), member_copy);
    host.archive_write_field_vt0c(archive, key(kOwnerTimeKey), float_arg(at<float>(base, 0x94)));
    host.archive_write_field_vt0c(archive, key(kOwnerTypeKey), ArchiveArg{1u, at<std::uint32_t>(base, 0x14)});
    host.archive_write_field_vt0c(archive, key(kPartyIndexKey), ArchiveArg{1u, at<std::uint32_t>(base, 0x18)});
    host.archive_write_field_vt0c(archive, key(kPlayerIndexKey), ArchiveArg{1u, at<std::uint32_t>(base, 0x1C)});
    host.archive_write_field_vt0c(archive, key(kHitAnythingKey), ArchiveArg{3u, at<std::uint8_t>(base, 0x20)});
    host.archive_write_field_vt0c(archive, key(kUnderWaterKey), ArchiveArg{3u, at<std::uint8_t>(base, 0x44)});
    host.archive_write_field_vt0c(archive, key(kFlyTimeKey), float_arg(at<float>(base, 0x54)));
    host.archive_write_field_vt0c(archive, key(kPenetrationKey), float_arg(at<float>(base, 0x58)));
    host.archive_write_field_vt0c(archive, key(kHitDamageMulKey), float_arg(at<float>(base, 0xD0)));
    const bool tracers = ptr(base, 0x28) != nullptr || ptr(base, 0x24) != nullptr;
    host.archive_write_field_vt0c(archive, key(kTracersKey), ArchiveArg{3u, tracers ? 1u : 0u});
}

// ---------------------------------------------------------------------------
// Bullet (record vtable 00CF9DF0; the shot sub-object at +170h, the tick element at +244h).
// ---------------------------------------------------------------------------

// 006E71B0, __thiscall(bullet, archive), RET 4. 00924D90(bullet, archive); "bulletclass" {1,
// [[bullet+174h]+0Ch]}; section "_bullet"; section "_bulletBase" with 006E6E80(bullet+170h,
// archive); both closed.
void bullet_write_state_006e71b0(Seg33aHost& host, void* bullet, void* archive) {
    host.entity_write_archive_00924d90(bullet, archive);
    host.archive_write_field_vt0c(archive, key(kBulletClassKey),
                                  ArchiveArg{1u, at<std::uint32_t>(ptr(bullet, 0x174), 0x0C)});
    host.archive_open_section_vt04(archive, key(kBulletSectionKey));
    host.archive_open_section_vt04(archive, key(kBulletBaseSectionKey));
    bullet_base_write_state_006e6e80(host, add(bullet, 0x170), archive);
    host.archive_close_section_vt08(archive);
    host.archive_close_section_vt08(archive);
}

// 006E7850, __thiscall(bullet_base, reader), RET 4: the reading twin of 006E6E80 through slot 10h.
// "bbv" goes through a stack vec3 then into +8h..+10h; "bberror" {5, &+38h}; section "owner" with
// 006E6B10(+98h, reader), closed; 006E6A60(reader, +B4h, "collOwner"); "ownerTime" {2, &+94h};
// "ownerType" read into the reader-argument stack slot and then stored to +14h (an unread value
// leaves the reader's address there, as in the image); "partyIndex", "playerIndex" {1}, "hitAnything",
// "underWater" {3}, "flyTime", "penetration", "hitDamageMul" {2}; "tracers" into a stack byte and,
// when set, bullet_base->vtable[3Ch]() (006E6C20, the effects). The vec3 and the tracers byte are
// uninitialised in the image; here they start at zero.
void bullet_base_read_state_006e7850(Seg33aHost& host, void* base, void* reader) {
    float bbv[3] = {0.0f, 0.0f, 0.0f};
    host.archive_read_field_vt10(reader, key(kBbvKey), value_arg(5, bbv));
    at<float>(base, 0x8) = bbv[0];
    at<float>(base, 0xC) = bbv[1];
    at<float>(base, 0x10) = bbv[2];
    host.archive_read_field_vt10(reader, key(kBbErrorKey), value_arg(5, add(base, 0x38)));
    host.archive_open_section_vt04(reader, key(kOwnerKey));
    save_reader_read_unit_handle_006e6b10(host, add(base, 0x98), reader);
    host.archive_close_section_vt08(reader);
    save_reader_read_observer_ref_006e6a60(host, reader, add(base, 0xB4), key(kCollOwnerKey));
    host.archive_read_field_vt10(reader, key(kOwnerTimeKey), value_arg(2, add(base, 0x94)));
    std::uint32_t owner_type = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(reader));
    host.archive_read_field_vt10(reader, key(kOwnerTypeKey), value_arg(1, &owner_type));
    at<std::uint32_t>(base, 0x14) = owner_type;
    host.archive_read_field_vt10(reader, key(kPartyIndexKey), value_arg(1, add(base, 0x18)));
    host.archive_read_field_vt10(reader, key(kPlayerIndexKey), value_arg(1, add(base, 0x1C)));
    host.archive_read_field_vt10(reader, key(kHitAnythingKey), value_arg(3, add(base, 0x20)));
    host.archive_read_field_vt10(reader, key(kUnderWaterKey), value_arg(3, add(base, 0x44)));
    host.archive_read_field_vt10(reader, key(kFlyTimeKey), value_arg(2, add(base, 0x54)));
    host.archive_read_field_vt10(reader, key(kPenetrationKey), value_arg(2, add(base, 0x58)));
    host.archive_read_field_vt10(reader, key(kHitDamageMulKey), value_arg(2, add(base, 0xD0)));
    std::uint8_t tracers = 0;
    host.archive_read_field_vt10(reader, key(kTracersKey), value_arg(3, &tracers));
    if (tracers != 0) host.shot_on_spawn_vt3c(base);
}

// 006E7B00, __fastcall(bullet /*ECX*/), RET, SEH frame. 00925CE0(bullet); 006E22D0(+170h);
// 00875890(ECX +244h, bullet, 0); vtables +0h 00CF9DF0, +10h 00CF9DD8, +24h 00CF9DD0, +170h
// 00CF9D90, +244h 00CF9D78; class id +C4h = 29h; +23Ch = bullet; +27Ch = 00BD2F10(ECX 0, 0.0f,
// 0.1f (00D7A2F0)): one draw from the shared generator per constructed bullet. Returns bullet.
void* projectile_tickable_entity_construct_006e7b00(Seg33aHost& host, void* bullet) {
    host.scene_node_construct_00925ce0(bullet);
    projectile_shot_base_construct_006e22d0(host, add(bullet, 0x170));
    host.tick_registration_construct_00875890(add(bullet, 0x244), bullet, 0);
    at<std::uint32_t>(bullet, 0x000) = 0x00CF9DF0;
    at<std::uint32_t>(bullet, 0x010) = 0x00CF9DD8;
    at<std::uint32_t>(bullet, 0x024) = 0x00CF9DD0;
    at<std::uint32_t>(bullet, 0x170) = 0x00CF9D90;
    at<std::uint32_t>(bullet, 0x244) = 0x00CF9D78;
    at<std::uint32_t>(bullet, 0x0C4) = 0x29;
    at<void*>(bullet, 0x23C) = bullet;
    at<float>(bullet, 0x27C) = host.random_uniform_float_range_00bd2f10(0, 0.0f, kBulletPhaseMax_00d7a2f0);
    return bullet;
}

// 006E7C30 / 006E7C40, __fastcall(bullet /*ECX*/), RET: the null-preserving sub-objects +244h and
// +170h.
void* bullet_tick_element_006e7c30(void* bullet) {
    return bullet != nullptr ? add(bullet, 0x244) : nullptr;
}
void* bullet_bullet_block_006e7c40(void* bullet) {
    return bullet != nullptr ? add(bullet, 0x170) : nullptr;
}

// 006E7DE0, __fastcall(bullet /*ECX*/), RET, SEH frame. 00875490 on +244h (null-preserving),
// 006E25A0 on +170h (null-preserving), then 00925780(bullet). No vtable is re-stamped here.
void bullet_destruct_006e7de0(Seg33aHost& host, void* bullet) {
    host.unit_tick_element_destruct_00875490(bullet != nullptr ? add(bullet, 0x244) : nullptr);
    projectile_shot_destruct_006e25a0(host, bullet != nullptr ? add(bullet, 0x170) : nullptr);
    host.entity_destruct_base_00925780(bullet);
}

// 006E7E80, __fastcall(bullet /*ECX*/), RET, SEH frame: the bullet's activate. 00923840(bullet);
// with a model [[+174h]+A8h]: +280h = model->vtable[8](0, 1.0f), pose refresh,
// [[+280h]+0Ch]->vtable[34h](+CCh), 00B6D890(ECX [[+280h]+0Ch], [[00E188A8]+19ECh]); without, +280h
// = 0. Then a kind-3 saved record is read: enter "_bullet", enter "_bulletBase",
// 006E7850(+170h, reader), leave twice, 00441A20.
void bullet_activate_006e7e80(Seg33aHost& host, void* bullet) {
    host.entity_activate_base_00923840(bullet);
    void* const model = ptr(ptr(bullet, 0x174), 0xA8);
    if (model != nullptr) {
        void* const instance = host.model_create_instance_vt08(model, 0, kOne_00d7a24c);
        at<void*>(bullet, 0x280) = instance;
        void* const node = ptr(instance, 0x0C);
        refresh_pose_if_stale(host, bullet);
        host.scene_object_set_matrix_vt34(node, static_cast<const float*>(add(bullet, 0xCC)));
        host.node_propagate_root_registration_00b6d890(ptr(ptr(bullet, 0x280), 0x0C), host.effect_manager_00e188a8_19ec());
    } else {
        at<void*>(bullet, 0x280) = nullptr;
    }
    void* const saved = ptr(bullet, 0xC0);
    if (saved == nullptr || at<std::int32_t>(saved, 4) != 3) return;
    alignas(4) std::uint8_t lua_data[0x14];
    alignas(4) std::uint8_t reader[0x14];
    host.entity_saved_lua_data_009238a0(bullet, lua_data);
    host.lua_reader_construct_004425c0(reader, lua_data);
    host.lua_reader_enter_key_00bd8e20(reader, key(kBulletSectionKey));
    host.lua_reader_enter_key_00bd8e20(reader, key(kBulletBaseSectionKey));
    bullet_base_read_state_006e7850(host, add(bullet, 0x170), reader);
    host.lua_reader_leave_00bd7a20(reader);
    host.lua_reader_leave_00bd7a20(reader);
    host.lua_reader_destroy_00441a20(reader);
}

// ---------------------------------------------------------------------------
// Small helpers between the bullet and the bullet-class cache.
// ---------------------------------------------------------------------------

// 006E7FD0, __fastcall(object /*ECX*/), RET: [00E199A8] = 0, then object vtable = 00CE3818.
void singleton_e199a8_reset_on_unwind_006e7fd0(Seg33aHost& host, void* object) {
    *host.singleton_slot_00e199a8() = nullptr;
    at<std::uint32_t>(object, 0) = 0x00CE3818;
}

// 006E8010, (ECX void** ref, EDX cause), RET: with an entity in *ref, 00926D90(ECX entity, cause)
// (the EDX input passes straight through as the pushed argument), then *ref = null.
void entity_ref_kill_and_clear_006e8010(Seg33aHost& host, void** ref, std::int32_t cause) {
    if (*ref == nullptr) return;
    host.mission_entity_kill_00926d90(*ref, cause);
    *ref = nullptr;
}

// 006E8040, __thiscall(entity, const float* position), RET 4: +C8h = 1, +10Ch = 0, world position
// +FCh..+104h = position.
void entity_set_world_position_direct_006e8040(void* entity, const float* position) {
    at<std::uint8_t>(entity, 0xC8) = 1;
    at<std::uint8_t>(entity, 0x10C) = 0;
    at<float>(entity, 0xFC) = position[0];
    at<float>(entity, 0x100) = position[1];
    at<float>(entity, 0x104) = position[2];
}

// 006E80D0, __thiscall(void** slot, void* value), RET 4: releases the old reference (InterlockedDecrement
// at +4, vtable[0] at zero), clears, then stores value without adding a reference. Returns slot.
void** ref_ptr_assign_006e80d0(Seg33aHost& host, void** slot, void* value) {
    if (*slot != nullptr) {
        ref_release(host, *slot);
        *slot = nullptr;
    }
    *slot = value;
    return slot;
}

// 006E8150, __thiscall(vector {data, count +4h, capacity +8h}, n), RET 4, SEH frame. n < 1 becomes
// 1; with capacity < n: a new block of n*4 bytes (00BF55BE), each live element copied with a
// reference added (the element slot is zeroed first; the null test on the slot pointer is the
// compiler's placement-new guard), every old element released and cleared, the old block freed
// (00BF6989), data = new block, capacity = n.
void pointer_vector_reserve_006e8150(Seg33aHost& host, void* vector, std::int32_t n) {
    if (n < 1) n = 1;
    if (at<std::int32_t>(vector, 8) >= n) return;
    void** const block = static_cast<void**>(host.operator_new_00bf55be(static_cast<std::size_t>(n) * 4));
    void** slot = block;
    for (std::int32_t i = 0; i < at<std::int32_t>(vector, 4); ++i, ++slot) {
        if (slot == nullptr) continue;
        *slot = nullptr;
        void* const value = static_cast<void**>(ptr(vector, 0))[i];
        if (value != nullptr) {
            *slot = value;
            ref_add(host, value);
        }
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(vector, 4); ++i) {
        void** const element = static_cast<void**>(ptr(vector, 0)) + i;
        if (*element != nullptr) {
            ref_release(host, *element);
            *element = nullptr;
        }
    }
    host.free_00bf6989(ptr(vector, 0));
    at<void**>(vector, 0) = block;
    at<std::int32_t>(vector, 8) = n;
}

// 006E8250, (NativeStr name by value, consumed), RET 8; ECX is not read. The index of the name in
// the twenty-entry table of C strings at 00E08E58: a null name matches a null or empty entry; a
// non-null name matches a null entry only when its size is 0, otherwise by _stricmp (00BF7FBF,
// case-insensitive).
// The string buffer is returned to the pool (00BD1510(00419CC0(), data, size + 1, 1)) on both
// exits. No match answers 0, the same as a match at index 0.
std::int32_t weapon_name_index_006e8250(Seg33aHost& host, NativeStr name) {
    const char* const* const table = host.weapon_name_table_00e08e58();
    std::int32_t found = -1;
    for (std::int32_t i = 0; i < 0x14 && found < 0; ++i) {
        const char* const entry = table[i];
        if (name.data == nullptr) {
            if (entry == nullptr || entry[0] == '\0') found = i;
        } else if (entry == nullptr) {
            if (name.size == 0) found = i;
        } else if (host.strcmp_00bf7fbf(name.data, entry) == 0) {
            found = i;
        }
    }
    if (name.data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), name.data, name.size + 1, 1);
    }
    return found < 0 ? 0 : found;
}

// ---------------------------------------------------------------------------
// The bullet-class cache (singleton [00E199A8], vtable 00CFA42C; a reference-counted pointer
// vector at +4h).
// ---------------------------------------------------------------------------

// 006E86F0, __thiscall(vector, n), RET 4. n above the capacity reserves first (006E8150); new
// slots from count to n are zeroed (with the placement-new null guard); surplus elements are
// released from the back (count decremented before each release); count = n.
void bullet_class_cache_resize_006e86f0(Seg33aHost& host, void* vector, std::int32_t n) {
    if (n > at<std::int32_t>(vector, 8)) pointer_vector_reserve_006e8150(host, vector, n);
    for (std::int32_t i = at<std::int32_t>(vector, 4); i < n; ++i) {
        void** const slot = static_cast<void**>(ptr(vector, 0)) + i;
        if (slot != nullptr) *slot = nullptr;
    }
    while (n < at<std::int32_t>(vector, 4)) {
        at<std::int32_t>(vector, 4) -= 1;
        void** const slot = static_cast<void**>(ptr(vector, 0)) + at<std::int32_t>(vector, 4);
        if (*slot != nullptr) {
            ref_release(host, *slot);
            *slot = nullptr;
        }
    }
    at<std::int32_t>(vector, 4) = n;
}

// 006E97F0, __fastcall(cls /*ECX*/), RET, SEH frame. With no linked class +A8h and a link flag
// +A0h: the name [+A4h] (or "" at 00E199AC) is wrapped in a NativeStr (0041E870), +A8h =
// 007188A0(&name) (load through the game factory), and the buffer is returned to the pool. +C8h = 1
// in every case.
void bullet_class_resolve_linked_class_006e97f0(Seg33aHost& host, void* cls) {
    if (ptr(cls, 0xA8) == nullptr && at<std::uint32_t>(cls, 0xA0) != 0) {
        const char* name = static_cast<const char*>(ptr(cls, 0xA4));
        if (name == nullptr) name = host.empty_string_00e199ac();
        NativeStr text{0u, nullptr};
        host.native_string_assign_0041e870(&text, name);
        at<void*>(cls, 0xA8) = host.resource_load_with_game_factory_007188a0(&text);
        if (text.data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), text.data, text.size + 1, 1);
        }
    }
    at<std::uint8_t>(cls, 0xC8) = 1;
}

// 006E9AA0 and 006E9AD0 (identical), __fastcall(vector /*ECX*/), RET: 006E86F0(vector, 0), then
// free(data).
void bullet_class_cache_destruct_006e9aa0(Seg33aHost& host, void* vector) {
    bullet_class_cache_resize_006e86f0(host, vector, 0);
    host.free_00bf6989(ptr(vector, 0));
}
void bullet_class_cache_destruct_006e9ad0(Seg33aHost& host, void* vector) {
    bullet_class_cache_resize_006e86f0(host, vector, 0);
    host.free_00bf6989(ptr(vector, 0));
}

// 006E9B10, __fastcall(cache /*ECX*/), RET, SEH frame: 006E86F0(cache+4, 0), free([cache+4]),
// [00E199A8] = 0, vtable 00CE3818.
void bullet_class_cache_singleton_destruct_006e9b10(Seg33aHost& host, void* cache) {
    bullet_class_cache_resize_006e86f0(host, add(cache, 4), 0);
    host.free_00bf6989(ptr(cache, 4));
    *host.singleton_slot_00e199a8() = nullptr;
    at<std::uint32_t>(cache, 0) = 0x00CE3818;
}

// 006E9B70, __cdecl(), RET, SEH frame -> the cache. With [00E199A8] null: the lifetime manager's lock
// ([00415350()+10h], when set) is entered ([00CE2218] EnterCriticalSection, +18h count up); still
// null after the lock: 10h bytes from 00BF681B, initialised to vtable 00CFA42C and three zero
// dwords (null stays null), published to [00E199A8] and registered with
// 00BD0C30(ECX 00415350(), cache); the lock is left (+18h count down, [00CE2210]). Returns
// [00E199A8].
void* bullet_class_cache_get_006e9b70(Seg33aHost& host) {
    void** const slot = host.singleton_slot_00e199a8();
    if (*slot != nullptr) return *slot;
    void* const section = ptr(host.singleton_lifetime_get_manager_00415350(), 0x10);
    if (section != nullptr) {
        host.enter_critical_section_00ce2218(section);
        at<std::int32_t>(section, 0x18) += 1;
    }
    if (*slot == nullptr) {
        void* const memory = host.operator_new_00bf681b(0x10);
        if (memory != nullptr) {
            at<std::uint32_t>(memory, 0x0) = 0x00CFA42C;
            at<std::uint32_t>(memory, 0x4) = 0;
            at<std::uint32_t>(memory, 0x8) = 0;
            at<std::uint32_t>(memory, 0xC) = 0;
        }
        *slot = memory;
        host.singleton_lifetime_register_00bd0c30(host.singleton_lifetime_get_manager_00415350(), *slot);
    }
    if (section != nullptr) {
        at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section_00ce2210(section);
    }
    return *slot;
}

// 006E9C50, __cdecl(), RET, SEH frame: drop the bullet classes the scripts did not get. Over the Lua table
// globals.Bullets ([00E188A8]+1A0Ch, 00B67980, 00B67800 "Bullets"), iterated with 00B67080 /
// 00B67190 until 00B66420 reports the end, every entry whose "Got" (00CE452C) is not exactly
// boolean true (00B66000) has its key (00B66290) appended to a local std::vector<int> (00442190).
// Then for each collected index (00415450 size; the checked index never fails here)
// "Bullets[%d]=nil" (00CFA430) is formatted into a 2Ah-byte buffer (0043EE60) and run with
// 006B8AD0(ECX [[00E188A8]+1A08h]+4, buffer, 0, 0, 2). The LuaObjects are destroyed (00B67700)
// value, key, table, and the vector buffer is freed (00BF65AC).
void bullet_classes_reload_from_lua_006e9c50(Seg33aHost& host) {
    std::vector<std::int32_t> missing;
    alignas(4) std::uint8_t globals[0x14];
    alignas(4) std::uint8_t bullets[0x14];
    alignas(4) std::uint8_t key_object[0x14];
    alignas(4) std::uint8_t value_object[0x14];
    alignas(4) std::uint8_t got[0x14];
    void* const globals_object = host.lua_state_owner_get_globals_00b67980(host.lua_state_owner_00e188a8_1a0c(), globals);
    host.lua_object_get_by_name_00b67800(globals_object, bullets, kBulletsKey);
    host.lua_object_destruct_00b67700(globals);
    host.lua_object_construct_00b65f50(key_object);
    host.lua_object_construct_00b65f50(value_object);
    host.lua_object_iterate_first_00b67080(bullets, key_object, value_object);
    while (!host.lua_object_is_unbound_00b66420(bullets, key_object)) {
        void* const got_object = host.lua_object_get_by_name_00b67800(value_object, got, kGotKey);
        const bool taken = host.lua_object_is_boolean_true_00b66000(got_object);
        host.lua_object_destruct_00b67700(got);
        if (!taken) missing.push_back(host.lua_object_get_integer_00b66290(key_object));
        host.lua_object_iterate_next_00b67190(bullets, key_object, value_object);
    }
    for (std::size_t i = 0; i < missing.size(); ++i) {
        char buffer[0x2C];
        host.string_format_buffer_0043ee60(buffer, kBulletsNilFormat, missing[i]);
        host.lua_machine_run_string_006b8ad0(host.lua_machine_00e188a8_1a08(), buffer, 0, 0, 2);
    }
    host.lua_object_destruct_00b67700(value_object);
    host.lua_object_destruct_00b67700(key_object);
    host.lua_object_destruct_00b67700(bullets);
}

// 006E9E80, __fastcall(cls /*ECX*/), RET, SEH frame: the bullet class destructor. vtable 00CFA138;
// the cache (006E9B70) slot [cls+0Ch] (grown with 006E86F0 when past the count) is released and
// cleared; the linked class +A8h is released. With a game and a scene [[00E188A8]+19CCh], every
// entity on the chain [[scene+4]] / +38h (the next link read before the tests) that is a bullet
// (vtable[5Ch](29h)) of this class ([+174h]) or else a bomb (vtable[5Ch](2Ah)) of this class
// ([+314h]) is killed with 00926D90(entity, 0). Then the references +78h, +7Ch, the name +A0h/+A4h,
// the references +48h, +44h, +40h, +3Ch, +38h, +34h, +30h, +2Ch, +28h, +24h, the names +18h/+1Ch
// and +10h/+14h are released in that order, then 00BD30F0(cls).
void bullet_class_destruct_006e9e80(Seg33aHost& host, void* cls) {
    at<std::uint32_t>(cls, 0) = 0x00CFA138;
    void* const cache = bullet_class_cache_get_006e9b70(host);
    const std::int32_t index = at<std::int32_t>(cls, 0x0C);
    void* const vector = add(cache, 4);
    if (index >= at<std::int32_t>(cache, 8)) bullet_class_cache_resize_006e86f0(host, vector, index + 1);
    void** const slot = static_cast<void**>(ptr(vector, 0)) + index;
    if (*slot != nullptr) {
        ref_release(host, *slot);
        *slot = nullptr;
    }
    *slot = nullptr;
    auto release_member = [&](std::size_t offset) {
        if (ptr(cls, offset) != nullptr) {
            ref_release(host, ptr(cls, offset));
            at<void*>(cls, offset) = nullptr;
        }
    };
    auto release_name = [&](std::size_t size_offset) {
        char* const data = at<char*>(cls, size_offset + 4);
        if (data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), data,
                                                          at<std::uint32_t>(cls, size_offset) + 1, 1);
        }
    };
    release_member(0xA8);
    void* const game = host.game_00e188a8();
    if (game != nullptr && ptr(game, 0x19CC) != nullptr) {
        for (void* entity = ptr(ptr(ptr(game, 0x19CC), 4), 0); entity != nullptr;) {
            void* const next = ptr(entity, 0x38);
            bool kill = host.unit_is_alive_vt5c(entity, 0x29) && ptr(entity, 0x174) == cls;
            if (!kill) kill = host.unit_is_alive_vt5c(entity, 0x2A) && ptr(entity, 0x314) == cls;
            if (kill) host.mission_entity_kill_00926d90(entity, 0);
            entity = next;
        }
    }
    release_member(0x78);
    release_member(0x7C);
    release_name(0xA0);
    for (std::size_t offset : {0x48u, 0x44u, 0x40u, 0x3Cu, 0x38u, 0x34u, 0x30u, 0x2Cu, 0x28u, 0x24u}) {
        release_member(offset);
    }
    release_name(0x18);
    release_name(0x10);
    host.bullet_class_base_destruct_00bd30f0(cls);
}

// 006EA1A0, __cdecl(), RET: 006E86F0(006E9B70()+4, 0), the cache emptied.
void bullet_class_cache_clear_006ea1a0(Seg33aHost& host) {
    bullet_class_cache_resize_006e86f0(host, add(bullet_class_cache_get_006e9b70(host), 4), 0);
}

// ---------------------------------------------------------------------------
// The bullet-class family: name getters, constructors (006E8320 then the vtable and the sub-type
// +8h) and destructors (release the kind's own reference, then 006EA2C0 -> 006E9E80).
// ---------------------------------------------------------------------------

// 006EA1E0, __cdecl() -> EAX, RET: the name "MArtilleryBullet" (00CFA464).
const char* artillery_bullet_class_get_name_006ea1e0() { return kArtilleryBulletName; }
// 006EA220, __cdecl() -> EAX, RET: the name "MKamikazePlane" (00CFA4A0).
const char* kamikaze_plane_class_get_name_006ea220() { return kKamikazePlaneName; }
// 006EA4A0, __cdecl() -> EAX, RET: the name "MFlakBullet" (00CFA560).
const char* flak_bullet_class_get_name_006ea4a0() { return kFlakBulletName; }

namespace {

void* construct_bullet_class(Seg33aHost& host, void* cls, std::uint32_t vtable, std::uint32_t sub_type) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0) = vtable;
    at<std::uint32_t>(cls, 8) = sub_type;
    return cls;
}

void release_class_reference(Seg33aHost& host, void* cls, std::size_t offset) {
    if (ptr(cls, offset) != nullptr) {
        ref_release(host, ptr(cls, offset));
        at<void*>(cls, offset) = nullptr;
    }
}

}  // namespace

// 006EA200, __fastcall(cls /*ECX*/), RET: 006E8320, vtable 00CFA478, sub-type 11h.
void* kamikaze_plane_class_construct_006ea200(Seg33aHost& host, void* cls) {
    return construct_bullet_class(host, cls, 0x00CFA478, 0x11);
}

// 006EA2C0, __fastcall(cls /*ECX*/), RET, SEH frame: release +D4h, then 006E9E80(cls).
void bomb_class_destruct_body_006ea2c0(Seg33aHost& host, void* cls) {
    release_class_reference(host, cls, 0xD4);
    bullet_class_destruct_006e9e80(host, cls);
}

// 006EA330, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA4DC, sub-type 12h (rocket).
void* rocket_class_construct_006ea330(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA4DC;
    at<std::uint32_t>(cls, 8) = 0x12;
    return cls;
}

// 006EA3A0, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA508, +E8h = 0, sub-type 0Bh (depth charge).
void* depth_charge_class_construct_006ea3a0(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA508;
    at<std::uint32_t>(cls, 0xE8) = 0;
    at<std::uint32_t>(cls, 8) = 0x0B;
    return cls;
}

// 006EA400, __fastcall(cls /*ECX*/), RET, SEH frame: release +E8h, then 006EA2C0.
void depth_charge_class_destruct_006ea400(Seg33aHost& host, void* cls) {
    release_class_reference(host, cls, 0xE8);
    bomb_class_destruct_body_006ea2c0(host, cls);
}

// 006EA470, __fastcall(cls /*ECX*/), RET: 006E8320, vtable 00CFA53C, sub-type 10h, +D4h = -1 (flak).
void* flak_bullet_class_construct_006ea470(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0) = 0x00CFA53C;
    at<std::uint32_t>(cls, 8) = 0x10;
    at<std::int32_t>(cls, 0xD4) = -1;
    return cls;
}

// 006EA4F0, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA56C, +D8h = 0, sub-type 0Ah (torpedo).
void* torpedo_class_construct_006ea4f0(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA56C;
    at<std::uint32_t>(cls, 0xD8) = 0;
    at<std::uint32_t>(cls, 8) = 0x0A;
    return cls;
}

// 006EA530, __fastcall(cls /*ECX*/) -> ST0, RET; the torpedo class's slot 1Ch: (float)([+E4h] x 1.5)
// (FMUL by the double 00CE3D78, then FSTP float / FLD).
float torpedo_class_scaled_e4_006ea530(void* cls) {
    return static_cast<float>(static_cast<double>(at<float>(cls, 0xE4)) * kOnePointFive_00ce3d78);
}

// 006EA570, __fastcall(cls /*ECX*/), RET, SEH frame: release +D8h, then 006EA2C0.
void torpedo_class_destruct_006ea570(Seg33aHost& host, void* cls) {
    release_class_reference(host, cls, 0xD8);
    bomb_class_destruct_body_006ea2c0(host, cls);
}

// 006EA5E0, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA59C, +E0h = 0, sub-type 13h (water mine).
void* water_mine_class_construct_006ea5e0(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA59C;
    at<std::uint32_t>(cls, 0xE0) = 0;
    at<std::uint32_t>(cls, 8) = 0x13;
    return cls;
}

// 006EA640, __fastcall(cls /*ECX*/), RET, SEH frame: release +E0h, then 006EA2C0.
void water_mine_class_destruct_006ea640(Seg33aHost& host, void* cls) {
    release_class_reference(host, cls, 0xE0);
    bomb_class_destruct_body_006ea2c0(host, cls);
}

// 006EA6B0, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA5CC, sub-type 0Ch (dummy target).
void* dummy_target_class_construct_006ea6b0(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA5CC;
    at<std::uint32_t>(cls, 8) = 0x0C;
    return cls;
}

// 006EA720, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA604, +F0h = +F4h = 0 (an empty name), sub-type 0Fh
// (paratrooper).
void* paratrooper_class_construct_006ea720(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA604;
    at<std::uint32_t>(cls, 0xF0) = 0;
    at<std::uint32_t>(cls, 0xF4) = 0;
    at<std::uint32_t>(cls, 8) = 0x0F;
    return cls;
}

// 006EA780, __fastcall(cls /*ECX*/), RET, SEH frame: the name +F0h/+F4h returned to the pool when it has a buffer, then
// 006EA2C0.
void paratrooper_class_destruct_006ea780(Seg33aHost& host, void* cls) {
    char* const data = at<char*>(cls, 0xF4);
    if (data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), data,
                                                      at<std::uint32_t>(cls, 0xF0) + 1, 1);
    }
    bomb_class_destruct_body_006ea2c0(host, cls);
}

// 006EA7F0, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA63C, sub-type 0Dh (dummy kamikaze plane).
void* dummy_kamikaze_plane_class_construct_006ea7f0(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA63C;
    at<std::uint32_t>(cls, 8) = 0x0D;
    return cls;
}

// 006EA870, __fastcall(cls /*ECX*/), RET: 006E8320, +D4h = 0, vtable 00CFA678, sub-type 0Eh (dummy submarine).
void* dummy_submarine_class_construct_006ea870(Seg33aHost& host, void* cls) {
    host.bullet_class_construct_006e8320(cls);
    at<std::uint32_t>(cls, 0xD4) = 0;
    at<std::uint32_t>(cls, 0) = 0x00CFA678;
    at<std::uint32_t>(cls, 8) = 0x0E;
    return cls;
}

// 006EAFE0, (ECX void** holder, EDX id), RET, SEH frame -> holder. *holder = 0 is not written here:
// 006EA910(ECX holder, EDX id) fills it (EDX passes through). With a class that is not yet
// finalised (+C8h clear): class->vtable[14h](), then (*holder)->vtable[10h]() (re-read).
void* const* bullet_class_acquire_and_finalise_006eafe0(Seg33aHost& host, void** holder, std::int32_t id) {
    bullet_class_get_or_create_006ea910(host, holder, id);
    void* const cls = *holder;
    if (cls != nullptr && at<std::uint8_t>(cls, 0xC8) == 0) {
        host.bullet_class_resolve_vt14(cls);
        host.bullet_class_finalise_vt10(*holder);
    }
    return holder;
}

// ---------------------------------------------------------------------------
// Cargo unit (vtable 00CFA778) and the catapult.
// ---------------------------------------------------------------------------

// 006EB160, __thiscall(unit, arg), RET 4. 0081ED40(unit, arg); the eight cargo vtables; class id
// +C4h = 0Bh; the arrival timer +1188h = 0.0f. Returns unit.
void* cargo_construct_006eb160(Seg33aHost& host, void* unit, void* arg) {
    host.unit_vehicle_base_construct_0081ed40(unit, arg);
    at<std::uint32_t>(unit, 0x000) = 0x00CFA778;
    at<std::uint32_t>(unit, 0x010) = 0x00CFA75C;
    at<std::uint32_t>(unit, 0x024) = 0x00CFA754;
    at<std::uint32_t>(unit, 0x170) = 0x00CFA750;
    at<std::uint32_t>(unit, 0x1E4) = 0x00CFA748;
    at<std::uint32_t>(unit, 0x310) = 0x00CFA730;
    at<std::uint32_t>(unit, 0x38C) = 0x00CFA72C;
    at<std::uint32_t>(unit, 0x72C) = 0x00CFA728;
    at<std::uint32_t>(unit, 0x0C4) = 0x0B;
    at<float>(unit, 0x1188) = 0.0f;
    return unit;
}

// 006EB290, __thiscall(cls, arg), RET 4, SEH frame: the battleship factory's shape with 006EB160.
void* vehicle_class_cargo_create_instance_006eb290(Seg33aHost& host, void* cls, void* arg) {
    void* const memory = host.operator_new_00bf55be(0x118C);
    host.memset_00bf79f0(memory, 0, 0x118C);
    void* unit = memory != nullptr ? cargo_construct_006eb160(host, memory, arg) : nullptr;
    host.unit_set_vehicle_class_009553d0(unit, cls);
    at<void*>(unit, 0x354) = cls;
    return unit;
}

// 006EB320, __thiscall(cargo, float dt), RET 4. 008255B0(cargo, dt); then while alive (+5Ch set,
// +5Dh, +60h and +5Eh clear): with the timer +1188h > 1.0f and an order (vtable[174h]() non-null)
// whose vtable[8]() answers false, a landing site 006F2C30(ECX &+FCh after the pose refresh, EDX
// [+54h], 2, &out) (the out slot starts as the PUSH ECX value) reports the capture
// (00977DA0(ECX [00F8A0C4], 0, cargo)) and zeroes the timer; every other path adds dt
// ((float)(timer + dt), FADD float). Then a timer above 2.0f (00CE3958) is reset to 0.
void cargo_update_006eb320(Seg33aHost& host, void* cargo, float dt) {
    host.unit_instance_update_008255b0(cargo, dt);
    if (at<std::uint8_t>(cargo, 0x5C) == 0 || at<std::uint8_t>(cargo, 0x5D) != 0 || at<std::uint8_t>(cargo, 0x60) != 0 ||
        at<std::uint8_t>(cargo, 0x5E) != 0) {
        return;
    }
    bool reported = false;
    if (at<float>(cargo, 0x1188) > kOne_00d7a24c && host.unit_current_order_vt174(cargo) != nullptr &&
        !host.order_is_done_vt08(host.unit_current_order_vt174(cargo))) {
        void* const owner = ptr(cargo, 0x54);
        refresh_pose_if_stale(host, cargo);
        void* out = cargo;
        if (host.find_nearest_landing_site_006f2c30(add(cargo, 0xFC), owner, 2, &out) != 0) {
            host.warning_manager_report_capture_ship_00977da0(host.warning_manager_00f8a0c4(), 0, cargo);
            at<float>(cargo, 0x1188) = 0.0f;
            reported = true;
        }
    }
    if (!reported) {
        at<float>(cargo, 0x1188) = static_cast<float>(static_cast<double>(at<float>(cargo, 0x1188)) + static_cast<double>(dt));
    }
    if (at<float>(cargo, 0x1188) > kTwo_00ce3958) at<float>(cargo, 0x1188) = 0.0f;
}

// 006EB5C0, __fastcall(plane /*ECX*/), RET -> EAX: [+900h] == 3.
bool plane_is_launching_006eb5c0(void* plane) { return at<std::int32_t>(plane, 0x900) == 3; }

// 006EB660, __thiscall(catapult, n), RET 4: +480h = +484h = n.
void mcatapult_set_ammo_006eb660(void* catapult, std::int32_t n) {
    at<std::int32_t>(catapult, 0x480) = n;
    at<std::int32_t>(catapult, 0x484) = n;
}

// 006EB680, __thiscall(catapult, float heading), RET 4: when catapult->vtable[1D0h](1) answers true,
// +488h = 1 and +48Ch = heading.
void mcatapult_queue_launch_006eb680(Seg33aHost& host, void* catapult, float heading) {
    if (!host.gun_can_fire_vt1d0(catapult, 1)) return;
    at<std::uint8_t>(catapult, 0x488) = 1;
    at<float>(catapult, 0x48C) = heading;
}

// 006EB6B0, __thiscall(cls, lua), RET 4, SEH frame. 007327B0(cls, lua); +E0h = 8.0f (00CE3918);
// +DCh = lua["VertRotAngle"] as float, default 0.0f; +E4h = lua["PlaneStartPosZ"], default 0.0f;
// +E8h = lua["PlaneStartPosY"], default 2.0f (00CE3958); each through 00B67800 and 00B66330, the
// temporary LuaObject destroyed with 00B67700.
void catapult_class_read_lua_fields_006eb6b0(Seg33aHost& host, void* cls, void* lua) {
    host.gun_class_read_lua_fields_007327b0(cls, lua);
    at<float>(cls, 0xE0) = kEight_00ce3918;
    alignas(4) std::uint8_t field[0x14];
    at<float>(cls, 0xDC) = host.lua_reference_get_float_or_default_00b66330(
        host.lua_object_get_by_name_00b67800(lua, field, kVertRotAngleKey), 0.0f);
    host.lua_object_destruct_00b67700(field);
    at<float>(cls, 0xE4) = host.lua_reference_get_float_or_default_00b66330(
        host.lua_object_get_by_name_00b67800(lua, field, kPlaneStartPosZKey), 0.0f);
    host.lua_object_destruct_00b67700(field);
    at<float>(cls, 0xE8) = host.lua_reference_get_float_or_default_00b66330(
        host.lua_object_get_by_name_00b67800(lua, field, kPlaneStartPosYKey), kTwo_00ce3958);
    host.lua_object_destruct_00b67700(field);
}

// 006EB870, __fastcall(cls /*ECX*/) -> ST0, RET: (float)(|[+DCh]| / [+8Ch]) when [+8Ch] > 0.0f
// (COMISS/JBE, so NaN gives 0), else 0.0f.
float ratio_abs_dc_over_8c_006eb870(void* cls) {
    const float divisor = at<float>(cls, 0x8C);
    if (!(divisor > 0.0f)) return 0.0f;
    const float magnitude = std::fabs(at<float>(cls, 0xDC));
    // cc13-fixture8: 006EB8A5 FDIV rounds to 24 bits on the main thread, then the FSTP rounds
    // again; the two differ from one float division only for a denormal quotient.
    return static_cast<float>(cc13x87::x87_div24(magnitude, divisor));
}

// 006EB9C0, __thiscall(message, entity), RET 4. 0075B430(message, 7Ch); word +18h = 0, +1Ah = +1Ch
// = 0, +4h = 1, vtable 00CFAA0C, word +20h = entity+174h. Returns message.
void* vehicle_add_launched_message_construct_006eb9c0(Seg33aHost& host, void* message, void* entity) {
    host.session_message_construct_base_0075b430(message, 0x7C);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CFAA0C;
    at<std::uint16_t>(message, 0x20) = at<std::uint16_t>(entity, 0x174);
    return message;
}

// 006EBA90, __thiscall(message, std::uint8_t value), RET 4: type C6h, byte +20h = value, vtable
// 00CFAA20.
void* plane_takeoff_message_construct_006eba90(Seg33aHost& host, void* message, std::uint8_t value) {
    host.session_message_construct_base_0075b430(message, 0xC6);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint8_t>(message, 0x20) = value;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CFAA20;
    return message;
}

// 006EBB90, __thiscall(message, entity, float heading), RET 8: type B4h, vtable 00CFAA34, +24h =
// heading, word +20h = entity+174h.
void* catapult_fire_message_construct_006ebb90(Seg33aHost& host, void* message, void* entity, float heading) {
    host.session_message_construct_base_0075b430(message, 0xB4);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CFAA34;
    at<float>(message, 0x24) = heading;
    at<std::uint16_t>(message, 0x20) = at<std::uint16_t>(entity, 0x174);
    return message;
}

// 006EBCC0, __fastcall(turn /*ECX*/), RET (tail call): +1ACh = +1A4h, +1B0h = +1A8h, +1C0h = +1BCh,
// then 00605070 wraps +1ACh and (tail) +1B0h in place.
void catapult_turn_commit_angles_006ebcc0(Seg33aHost& host, void* turn) {
    at<float>(turn, 0x1AC) = at<float>(turn, 0x1A4);
    at<float>(turn, 0x1B0) = at<float>(turn, 0x1A8);
    at<float>(turn, 0x1C0) = at<float>(turn, 0x1BC);
    host.math_wrap_angle_in_place_00605070(static_cast<float*>(add(turn, 0x1AC)));
    host.math_wrap_angle_in_place_00605070(static_cast<float*>(add(turn, 0x1B0)));
}

namespace {

// One axis of 006EBD90: d = 00438B10(target, current); positive d moves by min(d, rate x dt), the
// rest by max(d, -rate x dt) (the step rounded to float; an unordered compare takes the step);
// the result is 00438AA0(current, delta).
float catapult_turn_axis(Seg33aHost& host, float current, float target, float rate, float dt) {
    const float d = host.math_subtract_wrapped_angle_00438b10(target, current);
    float delta;
    if (d > 0.0f) {
        const float step = static_cast<float>(static_cast<double>(rate) * static_cast<double>(dt));
        delta = step > d ? d : step;
    } else {
        const float step = static_cast<float>(static_cast<double>(-rate) * static_cast<double>(dt));
        delta = d > step ? d : step;
    }
    return host.math_add_wrapped_angle_00438aa0(current, delta);
}

}  // namespace

// 006EBD90, __thiscall(turn, float dt), RET 4. +1A4h = +1ACh, +1A8h = +1B0h; the targets +1B4h and
// +1B8h are wrapped (00605070). With the class [+E4h]: a yaw rate +88h that is not positive (COMISS
// 0 / JC) snaps +1A4h to the target, likewise the pitch rate +8Ch for +1A8h. Then when both
// |00438B10(target, current)| are below 1.745329e-4 (00CFAA48) both snap; otherwise each axis
// steps (catapult_turn_axis). Finally +1BCh = max-of([class+E4h], (float)(+1C4h x dt + +1C0h)) in
// the image's compare order (the class value when it is greater, else the sum; NaN gives the sum).
void catapult_turn_step_006ebd90(Seg33aHost& host, void* turn, float dt) {
    at<float>(turn, 0x1A4) = at<float>(turn, 0x1AC);
    at<float>(turn, 0x1A8) = at<float>(turn, 0x1B0);
    float* const yaw_target = static_cast<float*>(add(turn, 0x1B4));
    float* const pitch_target = static_cast<float*>(add(turn, 0x1B8));
    host.math_wrap_angle_in_place_00605070(yaw_target);
    host.math_wrap_angle_in_place_00605070(pitch_target);
    void* const cls = ptr(turn, 0xE4);
    if (!(0.0f < at<float>(cls, 0x88))) at<float>(turn, 0x1A4) = *yaw_target;
    if (!(0.0f < at<float>(cls, 0x8C))) at<float>(turn, 0x1A8) = *pitch_target;
    const float yaw_gap = std::fabs(host.math_subtract_wrapped_angle_00438b10(*yaw_target, at<float>(turn, 0x1A4)));
    const float pitch_gap = std::fabs(host.math_subtract_wrapped_angle_00438b10(*pitch_target, at<float>(turn, 0x1A8)));
    if (kSnapAngle_00cfaa48 > yaw_gap && kSnapAngle_00cfaa48 > pitch_gap) {
        at<float>(turn, 0x1A4) = *yaw_target;
        at<float>(turn, 0x1A8) = *pitch_target;
    } else {
        at<float>(turn, 0x1A4) = catapult_turn_axis(host, at<float>(turn, 0x1A4), *yaw_target,
                                                    at<float>(ptr(turn, 0xE4), 0x88), dt);
        at<float>(turn, 0x1A8) = catapult_turn_axis(host, at<float>(turn, 0x1A8), *pitch_target,
                                                    at<float>(ptr(turn, 0xE4), 0x8C), dt);
    }
    const float sum = static_cast<float>(static_cast<double>(at<float>(turn, 0x1C4)) * static_cast<double>(dt) +
                                         static_cast<double>(at<float>(turn, 0x1C0)));
    const float floor_value = at<float>(ptr(turn, 0xE4), 0xE4);
    at<float>(turn, 0x1BC) = floor_value > sum ? floor_value : sum;
}

// 006EBFF0, __fastcall(catapult /*ECX*/) -> AL, RET, SEH frame. fired = 00727E30(catapult); fired in
// session mode 1: a stack CATAPULT_FIRE message 006EBB90(msg, [+4A8h], +48Ch) broadcast with
// 0077C7B0(catapult, msg, 0). Returns fired.
bool mcatapult_fire_if_ready_006ebff0(Seg33aHost& host, void* catapult) {
    const bool fired = host.gun_fire_if_ready_00727e30(catapult);
    if (!fired || host.session_mode_00e188a8_1fe4() != 1) return fired;
    alignas(4) std::uint8_t message[0x28];
    void* const msg = catapult_fire_message_construct_006ebb90(host, message, ptr(catapult, 0x4A8),
                                                               at<float>(catapult, 0x48C));
    host.session_owner_broadcast_message_0077c7b0(catapult, msg, 0);
    return fired;
}

// 006EA910, (ECX void** holder, EDX id), RET, SEH frame (EBP frame, ESP aligned to 8) -> holder.
//  The cache (006E9B70) vector is grown to id + 1 when id is at or past its count. A cached class
//  skips to the hand-out. Otherwise: entry = globals.Bullets[id] (00B67980, 00B67800 "Bullets",
//  00B67720); type = NativeStr(entry["Type"] as string) (00B67800 "Type", 00B662B0, 0041E870);
//  entry["Got"] = true (00B673A0 with a NativeStr "Got", whose buffer is then returned); the type
//  is compared case-insensitively (00425850) against, in order: "Bullet" (D4h bytes, 006E8320),
//  "Artillery" (D4h, 006E8320 then vtable 00CFA440 and sub-type 4, inline), "Kamikaze" (D4h,
//  006EA200), "Bomb" (D8h, 006EA260), "Torpedo" (FCh, 006EA4F0), "DepthCharge" (F8h, 006EA3A0),
//  "DummyTarget" (E4h, 006EA6B0), "DummyKamikazePlane" (E0h, 006EA7F0), "DummySubmarine" (E0h,
//  006EA870), "Paratrooper" (100h, 006EA720), "Flak" (DCh, 006EA470), "Rocket" (E8h, 006EA330),
//  "WaterMine" (ECh, 006EA5E0). The first three allocate with 00BF55BE + 00BF79F0 inline, the rest
//  through 00470B80. An unknown type: *holder = 0, 0041DD20(&type), the four LuaObjects destroyed,
//  return (no hand-out). A known type: cls->vtable[18h](&entry) (no null test; a failed allocation
//  faults there), cls+0Ch = id, the cache slot (grown again when needed) releases its old value
//  and takes cls without an added reference; the type buffer is returned to the pool; the
//  LuaObjects are destroyed (type field, entry, Bullets, globals).
//  Hand-out: the cache is grown when needed, *holder = 0, then *holder = slot value with a
//  reference added when non-null.
void* const* bullet_class_get_or_create_006ea910(Seg33aHost& host, void** holder, std::int32_t id) {
    auto cache_vector = [&]() {
        void* const vector = add(bullet_class_cache_get_006e9b70(host), 4);
        if (id >= at<std::int32_t>(vector, 4)) bullet_class_cache_resize_006e86f0(host, vector, id + 1);
        return vector;
    };
    void* vector = cache_vector();
    if (static_cast<void**>(ptr(vector, 0))[id] == nullptr) {
        alignas(4) std::uint8_t globals[0x14];
        alignas(4) std::uint8_t bullets[0x14];
        alignas(4) std::uint8_t entry[0x14];
        alignas(4) std::uint8_t type_field[0x14];
        void* const globals_object = host.lua_state_owner_get_globals_00b67980(host.lua_state_owner_00e188a8_1a0c(), globals);
        void* const bullets_object = host.lua_object_get_by_name_00b67800(globals_object, bullets, kBulletsKey);
        void* const entry_object = host.lua_object_get_by_index_00b67720(bullets_object, entry, id);
        void* const type_object = host.lua_object_get_by_name_00b67800(entry_object, type_field, kTypeKey);
        NativeStr type{0u, nullptr};
        host.native_string_assign_0041e870(&type, host.lua_object_get_string_00b662b0(type_object));
        NativeStr got{0u, nullptr};
        host.native_string_assign_0041e870(&got, kGotKey);
        host.lua_object_set_boolean_00b673a0(entry, &got, 1);
        if (got.data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), got.data, got.size + 1, 1);
        }
        auto is = [&](const char* name) { return host.native_string_equals_insensitive_00425850(&type, name); };
        auto new_cleared = [&](std::size_t size) {
            void* const memory = host.operator_new_00bf55be(size);
            host.memset_00bf79f0(memory, 0, size);
            return memory;
        };
        auto zeroed = [&](std::size_t size) { return host.memory_alloc_zeroed_00470b80(size); };
        void* cls = nullptr;
        bool known = true;
        if (is(kBulletTypeName)) {
            void* const memory = new_cleared(0xD4);
            if (memory != nullptr) {
                host.bullet_class_construct_006e8320(memory);
                cls = memory;
            }
        } else if (is(kArtilleryTypeName)) {
            void* const memory = new_cleared(0xD4);
            if (memory != nullptr) {
                host.bullet_class_construct_006e8320(memory);
                at<std::uint32_t>(memory, 0) = 0x00CFA440;
                at<std::uint32_t>(memory, 8) = 4;
                cls = memory;
            }
        } else if (is(kKamikazeTypeName)) {
            void* const memory = new_cleared(0xD4);
            if (memory != nullptr) cls = kamikaze_plane_class_construct_006ea200(host, memory);
        } else if (is(kBombTypeName)) {
            void* const memory = zeroed(0xD8);
            if (memory != nullptr) cls = host.bomb_class_construct_006ea260(memory);
        } else if (is(kTorpedoTypeName)) {
            void* const memory = zeroed(0xFC);
            if (memory != nullptr) cls = torpedo_class_construct_006ea4f0(host, memory);
        } else if (is(kDepthChargeTypeName)) {
            void* const memory = zeroed(0xF8);
            if (memory != nullptr) cls = depth_charge_class_construct_006ea3a0(host, memory);
        } else if (is(kDummyTargetTypeName)) {
            void* const memory = zeroed(0xE4);
            if (memory != nullptr) cls = dummy_target_class_construct_006ea6b0(host, memory);
        } else if (is(kDummyKamikazePlaneTypeName)) {
            void* const memory = zeroed(0xE0);
            if (memory != nullptr) cls = dummy_kamikaze_plane_class_construct_006ea7f0(host, memory);
        } else if (is(kDummySubmarineTypeName)) {
            void* const memory = zeroed(0xE0);
            if (memory != nullptr) cls = dummy_submarine_class_construct_006ea870(host, memory);
        } else if (is(kParatrooperTypeName)) {
            void* const memory = zeroed(0x100);
            if (memory != nullptr) cls = paratrooper_class_construct_006ea720(host, memory);
        } else if (is(kFlakTypeName)) {
            void* const memory = zeroed(0xDC);
            if (memory != nullptr) cls = flak_bullet_class_construct_006ea470(host, memory);
        } else if (is(kRocketTypeName)) {
            void* const memory = zeroed(0xE8);
            if (memory != nullptr) cls = rocket_class_construct_006ea330(host, memory);
        } else if (is(kWaterMineTypeName)) {
            void* const memory = zeroed(0xEC);
            if (memory != nullptr) cls = water_mine_class_construct_006ea5e0(host, memory);
        } else {
            known = false;
        }
        if (!known) {
            *holder = nullptr;
            host.native_string_destroy_storage_0041dd20(&type);
            host.lua_object_destruct_00b67700(type_field);
            host.lua_object_destruct_00b67700(entry);
            host.lua_object_destruct_00b67700(bullets);
            host.lua_object_destruct_00b67700(globals);
            return holder;
        }
        host.bullet_class_read_lua_vt18(cls, entry);
        at<std::int32_t>(cls, 0x0C) = id;
        vector = cache_vector();
        void** const slot = static_cast<void**>(ptr(vector, 0)) + id;
        if (*slot != nullptr) {
            ref_release(host, *slot);
            *slot = nullptr;
        }
        *slot = cls;
        if (type.data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_00419cc0(), type.data, type.size + 1, 1);
        }
        host.lua_object_destruct_00b67700(type_field);
        host.lua_object_destruct_00b67700(entry);
        host.lua_object_destruct_00b67700(bullets);
        host.lua_object_destruct_00b67700(globals);
    }
    vector = cache_vector();
    void* const value = static_cast<void**>(ptr(vector, 0))[id];
    *holder = nullptr;
    if (value != nullptr) {
        *holder = value;
        ref_add(host, value);
    }
    return holder;
}

// ---------------------------------------------------------------------------
// oct18b-nA3: newly defined game bodies, 006E0BE0..006F58D0.
// Evidence: saved Ghidra exports; 006E7551 JE 006E765A agrees with the disk
// bytes (Ghidra's 006E7250 body repaired in main 5764ca914).
// New C++ interfaces, not installed vtable entries. The host calls are real
// engine/virtual contracts; no game run or native ABI equivalence is claimed.
// ---------------------------------------------------------------------------
namespace {
constexpr float late_full_float_scale = 3.40282346638528859812e38f; // 00D7A248 / 00CFA9D8 = 7F7FFFFF

// 006E2490 stores the x87 sum to float before the double 1e-10 comparison.
// Keep its instruction order rather than rounding each square independently.
float late_shot_speed(const float* velocity) {
    float squared;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov eax, velocity
        fld dword ptr [eax + 4]
        fld dword ptr [eax]
        fld dword ptr [eax + 8]
        fld st(1)
        fmulp st(2), st(0)
        fld st(2)
        fmulp st(3), st(0)
        fxch st(1)
        faddp st(2), st(0)
        fmul st(0), st(0)
        faddp st(1), st(0)
        fstp squared
    }
#else
    const long double x = velocity[0], y = velocity[1], z = velocity[2];
    squared = static_cast<float>((y * y + x * x) + z * z);
#endif
    if (!(static_cast<double>(squared) > 1e-10)) return 0.0f; // 00CE3820
    float result;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        fld squared
        fsqrt
        fstp result
    }
#else
    result = static_cast<float>(std::sqrt(static_cast<double>(squared)));
#endif
    return result;
}

// The branch in 006E7394 compares the unspilled x87 x*x+z*z against the
// exact double at 00CF4868 (3F40624DE0000000), including unordered -> false.
bool late_horizontal_velocity_above_threshold(const float* velocity) {
    static const double threshold = 0.0005000000237487257;
#if defined(_MSC_VER) && defined(_M_IX86)
    std::uint8_t above;
    __asm {
        mov ecx, velocity
        fld dword ptr [ecx + 8]
        fld dword ptr [ecx]
        fmul st(0), st(0)
        fld st(1)
        fmulp st(2), st(0)
        faddp st(1), st(0)
        fld threshold
        fxch st(1)
        fcompp
        fnstsw ax
        sahf
        seta above
    }
    return above != 0;
#else
    const long double x = velocity[0], z = velocity[2];
    return x * x + z * z > threshold;
#endif
}

void late_write_bool_word(Seg33aHost& host, void* message, void* cursor) {
    host.session_message_write_header_0075b480(message, cursor);
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x1C));
    host.bit_cursor_write_word_bits_00429120(cursor, at<std::uint16_t>(message, 0x20), 12);
}
void late_read_bool_word(Seg33aHost& host, void* message, void* stream) {
    host.session_message_read_header_0075b4c0(message, stream);
    void* const cursor = add(stream, 4); // every reader advances from wrapper to raw cursor
    host.bit_cursor_read_boolean_00428d70(cursor, static_cast<std::uint8_t*>(add(message, 0x1C)));
    host.bit_cursor_read_word_bits_00428e30(cursor, static_cast<std::uint16_t*>(add(message, 0x20)), 12);
}
} // namespace

// 006E0BE0/006E0C50: ECX message, cursor/stream, RET 4. Header; 3,2 bits;
// vec3+24h signed 16 bits with scale 22000 (00CF9360), vec3+30h 10 bits/2.
void kill_bullet_message_write_006e0be0(Seg33aHost& host, void* message, void* cursor) {
    host.session_message_write_header_0075b480(message, cursor);
    host.bit_cursor_write_unsigned_00429070(cursor, at<std::uint32_t>(message, 0x1C), 3);
    host.bit_cursor_write_unsigned_00429070(cursor, at<std::uint32_t>(message, 0x20), 2);
    host.bit_cursor_write_float_array_00429790(cursor, static_cast<float*>(add(message, 0x24)), 3, 0, 1, 22000.0f, 16);
    host.bit_cursor_write_float_array_00429790(cursor, static_cast<float*>(add(message, 0x30)), 3, 0, 1, 2.0f, 10);
}
void kill_bullet_message_read_006e0c50(Seg33aHost& host, void* message, void* stream) {
    host.session_message_read_header_0075b4c0(message, stream);
    void* const cursor = add(stream, 4);
    host.bit_cursor_read_unsigned_00428d10(cursor, static_cast<std::uint32_t*>(add(message, 0x1C)), 3);
    host.bit_cursor_read_unsigned_00428d10(cursor, static_cast<std::uint32_t*>(add(message, 0x20)), 2);
    host.bit_cursor_read_float_array_004294f0(cursor, static_cast<float*>(add(message, 0x24)), 3, 0, 1, 22000.0f, 16);
    host.bit_cursor_read_float_array_004294f0(cursor, static_cast<float*>(add(message, 0x30)), 3, 0, 1, 2.0f, 10);
}

// 006E2490..006E2540: ECX shot, RET, ST0. Class floats' semantic names
// are unknown; exact BC/C0/C4 offsets are retained. D0 is the scale set by 006E2550.
float projectile_shot_speed_006e2490(void* shot) { return late_shot_speed(static_cast<const float*>(add(shot, 8))); }
float projectile_shot_class_bc_006e2510(void* shot) { return at<float>(ptr(shot, 4), 0xBC); }
float projectile_shot_class_c0_006e2520(void* shot) { return at<float>(ptr(shot, 4), 0xC0); }
float projectile_shot_class_c4_006e2530(void* shot) { return at<float>(ptr(shot, 4), 0xC4); }
float projectile_shot_weapon_scale_006e2540(void* shot) { return at<float>(shot, 0xD0); }

// 006E39B0/E0, 006E3AD0/3B00, 006E3BA0: ECX message, cursor/stream, RET 4.
// bool byte+1C; then word+20 unsigned12 or DWORD+20 signed6. 3BA0 is signed8+1C only.
void gun_fire_message_write_006e39b0(Seg33aHost& h, void* m, void* c) { late_write_bool_word(h, m, c); }
void gun_fire_message_read_006e39e0(Seg33aHost& h, void* m, void* s) { late_read_bool_word(h, m, s); }
void bomb_platform_reload_message_write_006e3ad0(Seg33aHost& h, void* m, void* c) {
    h.session_message_write_header_0075b480(m, c);
    h.bit_cursor_write_boolean_004290b0(c, at<std::uint8_t>(m, 0x1C));
    h.bit_cursor_write_signed_00429090(c, at<std::int32_t>(m, 0x20), 6);
}
void bomb_platform_reload_message_read_006e3b00(Seg33aHost& h, void* m, void* s) {
    h.session_message_read_header_0075b4c0(m, s);
    void* const c = add(s, 4);
    h.bit_cursor_read_boolean_00428d70(c, static_cast<std::uint8_t*>(add(m, 0x1C)));
    h.bit_cursor_read_signed_00428d30(c, static_cast<std::int32_t*>(add(m, 0x20)), 6);
}
void multi_bomb_generate_message_write_006e3ba0(Seg33aHost& h, void* m, void* c) {
    h.session_message_write_header_0075b480(m, c);
    h.bit_cursor_write_signed_00429090(c, at<std::int32_t>(m, 0x1C), 8);
}
// 006E3D90: ECX rack, skill on stack, tail JMP vslot1C4h. 006E3DB0: RET, EAX owner+3BCh.
void rack_set_bot_skill_forward_006e3d90(Seg33aHost& h, void* rack, std::int32_t skill) { h.gun_set_bot_skill_vt1c4(rack, skill); }
void* rack_owner_006e3db0(void* rack) { return ptr(rack, 0x3BC); }

// 006E48A0/006E4950: ECX message, cursor/stream, RET 4. bool+word12,
// followed by four vec3 arrays at24/30/3C/48, signed numeric32 with FLT_MAX scale.
void bomb_platform_fire_message_write_006e48a0(Seg33aHost& h, void* m, void* c) {
    late_write_bool_word(h, m, c);
    for (std::size_t off = 0x24; off <= 0x48; off += 12)
        h.bit_cursor_write_float_array_00429790(c, static_cast<float*>(add(m, off)), 3, 0, 1, late_full_float_scale, 32);
}
void bomb_platform_fire_message_read_006e4950(Seg33aHost& h, void* m, void* s) {
    late_read_bool_word(h, m, s);
    for (std::size_t off = 0x24; off <= 0x48; off += 12)
        h.bit_cursor_read_float_array_004294f0(add(s, 4), static_cast<float*>(add(m, off)), 3, 0, 1, late_full_float_scale, 32);
}

// 006E7250: ECX bullet, one unused DWORD on stack, RET4. Ghidra's int* return
// is incidental EAX; callers use the update as void. The matrix-copy and x87
// branch follow the listing. Ghidra's body is 006E7250-006E766E (RET 4 at
// 006E766C) since main 5764ca914; 006E7551 JE 006E765A decodes there as on
// disk, and the tail calls are in its callees.
// Intrusive references and the consumed by-value definition use the existing
// family contracts. Host exceptions after creation release the result temporary.
void bullet_update_effects_006e7250(Seg33aHost& h, void* bullet, std::uint32_t) {
    if (h.tick_element_in_water_vt2c(add(bullet, 0x170)) != 0) {
        if (ptr(bullet, 0x194) == nullptr) return;
        void* const node = ptr(bullet, 0x23C);
        refresh_pose_if_stale(h, node);
        float point[3];
        const float* const transformed = h.vector3_transform_affine_point_004142e0(
            static_cast<const float*>(add(bullet, 0x19C)), point, static_cast<const float*>(add(node, 0xCC)));
        h.point_effect_set_world_point_004842c0(ptr(bullet, 0x194), transformed);
        void* const effect = ptr(bullet, 0x194); // EDI is captured before the virtual scale call
        at<float>(effect, 0x54) = h.entity_pose_scale_vt38(node);
        refresh_pose_if_stale(h, node);
        void* const effect_after_refresh = ptr(bullet, 0x194);
        std::memcpy(add(effect_after_refresh, 0x68), add(node, 0xEC), 3 * sizeof(float));
        at<std::uint32_t>(effect_after_refresh, 0x2C) = 0;
        return;
    }
    if (ptr(bullet, 0x198) != nullptr) {
        const float velocity[3] = {at<float>(bullet, 0x178), at<float>(bullet, 0x17C), at<float>(bullet, 0x180)};
        const bool oriented = late_horizontal_velocity_above_threshold(velocity);
        float matrix[16];
        if (oriented) {
            void* const node = ptr(bullet, 0x23C);
            refresh_pose_if_stale(h, node);
            std::memcpy(matrix, add(node, 0xCC), sizeof matrix);
            std::memcpy(matrix + 8, velocity, sizeof velocity);
            h.matrix_orthonormalize_basis_rows_0085dc80(matrix);
        }
        // Re-read velocity after orthonormalization just as the image does.
        const float factor = at<float>(bullet, 0x27C);
        const float offset[3] = {factor * at<float>(bullet, 0x178), factor * at<float>(bullet, 0x17C), factor * at<float>(bullet, 0x180)};
        void* const node = ptr(bullet, 0x23C);
        refresh_pose_if_stale(h, node);
        const float point[3] = {at<float>(node, 0xFC) + offset[0], at<float>(node, 0x100) + offset[1], at<float>(node, 0x104) + offset[2]};
        if (oriented) {
            std::memcpy(matrix + 12, point, sizeof point);
            h.point_effect_set_world_matrix_0053d9c0(ptr(bullet, 0x198), matrix);
        } else {
            h.point_effect_set_world_point_004842c0(ptr(bullet, 0x198), point);
        }
        return;
    }
    if (at<std::uint8_t>(bullet, 0x278) == 0) return;
    void* const cls = ptr(bullet, 0x174);
    void* const descriptor = ptr(cls, at<std::uint8_t>(bullet, 0x1CC) != 0 ? 0x44 : 0x40);
    if (descriptor == nullptr) return;
    void* const manager = h.effect_manager_00e188a8_19ec();
    const std::uint32_t flag = at<std::uint8_t>(bullet, 0x1B4);
    const void* const local_matrix = add(ptr(bullet, 0x23C), 0x74);
    ref_add(h, descriptor); // 00440490 copies the by-value definition reference
    void* created = nullptr;
    void* const* const result = h.point_effect_create_from_matrix_00868420(&created, manager, descriptor, local_matrix, flag, 0);
    {
        struct Temporary {
            Seg33aHost& host; void*& value;
            ~Temporary() { if (value != nullptr) ref_release(host, value); }
        } temporary{h, created};
        h.retained_reference_assign_004845d0(add(bullet, 0x198), result);
    }
    if (ptr(bullet, 0x198) != nullptr)
        h.effect_update_children_00867790(ptr(bullet, 0x198), 0.0f, h.effect_reference_node_00e188a8_19fc());
}

// 006E7C50: AL=1, RET. 006E7C90: ECX bullet, ST0=[+240h], RET.
bool bullet_always_true_006e7c50() noexcept { return true; }
float bullet_weapon_scale_006e7c90(void* bullet) { return at<float>(bullet, 0x240); }
// 006E7CA0: ECX bullet, float* out, RET4; EAX retains out. Each lane is x87-copied.
float* bullet_velocity_006e7ca0(void* bullet, float* output) {
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {
        mov ecx, bullet
        mov eax, output
        fld dword ptr [ecx + 178h]
        fstp dword ptr [eax]
        fld dword ptr [ecx + 17ch]
        fstp dword ptr [eax + 4]
        fld dword ptr [ecx + 180h]
        fstp dword ptr [eax + 8]
    }
#else
    for (std::size_t i = 0; i != 3; ++i) output[i] = at<float>(bullet, 0x178 + 4 * i);
#endif
    return output;
}
// 006EA230/40: ECX unread, EAX zero. Actual RET1Ch / RET4 establish ignored stack sizes.
// Unknown argument meanings are deliberately retained as uninterpreted words.
void* bullet_class_unavailable_create_006ea230(void*, std::uint32_t, std::uint32_t, std::uint32_t,
                                             std::uint32_t, std::uint32_t, std::uint32_t, std::uint32_t) noexcept { return nullptr; }
void* bullet_class_unavailable_aux_006ea240(void*, std::uint32_t) noexcept { return nullptr; }

// 006EBA50..006EBC60: ECX message, cursor/stream, RET4. Header precedes all fields.
void vehicle_launched_message_read_006eba50(Seg33aHost& h, void* m, void* s) { late_read_bool_word(h, m, s); }
void plane_takeoff_message_write_006ebb10(Seg33aHost& h, void* m, void* c) {
    h.session_message_write_header_0075b480(m, c);
    h.bit_cursor_write_boolean_004290b0(c, at<std::uint8_t>(m, 0x1C));
    h.bit_cursor_write_boolean_004290b0(c, at<std::uint8_t>(m, 0x20));
}
void plane_takeoff_message_read_006ebb40(Seg33aHost& h, void* m, void* s) {
    h.session_message_read_header_0075b4c0(m, s);
    h.bit_cursor_read_boolean_00428d70(add(s, 4), static_cast<std::uint8_t*>(add(m, 0x1C)));
    h.bit_cursor_read_boolean_00428d70(add(s, 4), static_cast<std::uint8_t*>(add(m, 0x20)));
}
void catapult_fire_message_write_006ebc10(Seg33aHost& h, void* m, void* c) {
    late_write_bool_word(h, m, c);
    h.bit_cursor_write_float_004295c0(c, at<float>(m, 0x24), 0, 1, late_full_float_scale, 32);
}
void catapult_fire_message_read_006ebc60(Seg33aHost& h, void* m, void* s) {
    late_read_bool_word(h, m, s);
    h.bit_cursor_read_float_004293f0(add(s, 4), static_cast<float*>(add(m, 0x24)), 0, 1, late_full_float_scale, 32);
}
// 006EC0D0: ECX catapult, message, RET4; AL result, not the stale upper EAX.
bool catapult_handle_message_006ec0d0(Seg33aHost& h, void* catapult, void* message) {
    if (at<std::uint8_t>(message, 0x10) != 0xB4) return h.gun_handle_network_message_0072d830(catapult, message);
    h.catapult_apply_fire_message_vt1f0(catapult, message);
    return true;
}
// 006EC620: ECX catapult, float timestep, RET4. Recoil first, then plane pose.
void catapult_update_pose_006ec620(Seg33aHost& h, void* catapult, float timestep) {
    h.gun_update_barrel_recoil_0072b2d0(catapult, timestep);
    h.catapult_pose_launching_plane_006ec240(catapult);
}
}  // namespace bsp::cc13_seg33a
