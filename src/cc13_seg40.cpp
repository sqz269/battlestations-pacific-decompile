// cc13_seg40: segment 40 leftovers (packet cc13-seg40, docs/CC13_SEG40.md).
// Names are hypotheses; see the header for the hooks. Evidence per function:
// Capstone disasm-raw over the disk bytes plus the Ghidra pseudocode.

#include "bsp/cc13_seg40.hpp"

#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg48_at<T>(base, offset);
}

float f32(std::uint32_t bits) {
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The owned-object delete of 007294C0 / 007359D0 / 007359F0: when [holder] is
// non-null, its vtable slot 0 with `push 1`, then [holder] = 0.
void delete_owned(void* holder, Cc13Seg40Host& host) {
    void* object = at<void*>(holder, 0);
    if (object != nullptr) {
        host.vcall_00_delete(object, 1);
        at<void*>(holder, 0) = nullptr;
    }
}

void* entity_as_kind(void* entity, std::int32_t kind, Cc13Seg40Host& host) {
    if (entity != nullptr && host.vcall_5c_has_ordnance_kind(entity, kind)) {
        return entity;
    }
    return nullptr;
}

}  // namespace

// 007293D0 BSP_TickSubnodeOwner_DestructBase, __thiscall(ECX = owner), EH
// frame 00C85508. Bytes 007293D0-00729414: 00695870(ECX = owner ? owner+1Ch
// : null), then 00875B30(ECX = owner).
void cc13_seg40_tick_subnode_owner_destruct_base_007293d0(void* owner, Cc13Seg40Host& host) {
    host.callback_owner_destroy_00695870(owner != nullptr ? static_cast<std::uint8_t*>(owner) + 0x1C : nullptr);
    host.tick_subnode_base_cleanup_00875b30(owner);
}

// 007294A0 BSP_FreeOwnedBlock_007294a0, __thiscall(ECX = holder): _free
// (00BF6989) of [holder] when non-null, then [holder] = 0.
void cc13_seg40_free_owned_block_007294a0(void* holder, Cc13Seg40Host& host) {
    if (at<void*>(holder, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(holder, 0));
        at<void*>(holder, 0) = nullptr;
    }
}

// 007294C0 BSP_OwnedPtr_Delete_007294c0, __thiscall(ECX = holder).
void cc13_seg40_owned_ptr_delete_007294c0(void* holder, Cc13Seg40Host& host) {
    delete_owned(holder, host);
}

// 007359D0 BSP_OwnedPtr_Delete_007359d0: the same body.
void cc13_seg40_owned_ptr_delete_007359d0(void* holder, Cc13Seg40Host& host) {
    delete_owned(holder, host);
}

// 007359F0 BSP_OwnedPtr_Delete_007359f0: the same body.
void cc13_seg40_owned_ptr_delete_007359f0(void* holder, Cc13Seg40Host& host) {
    delete_owned(holder, host);
}

// 0072B990 BSP_Entity_AsKind08, __thiscall(ECX = entity), EAX: entity when it
// is non-null and its slot +5Ch(8) answers true, else null.
void* cc13_seg40_entity_as_kind08_0072b990(void* entity, Cc13Seg40Host& host) {
    return entity_as_kind(entity, 8, host);
}

// 0072B9B0 BSP_Entity_AsKind06_0072b9b0: the same with kind 6.
void* cc13_seg40_entity_as_kind06_0072b9b0(void* entity, Cc13Seg40Host& host) {
    return entity_as_kind(entity, 6, host);
}

// 0072BCE0 BSP_GunBot_OnTargetDestroyed, __thiscall(ECX = the bot's
// sub-object at bot+1Ch, destroyed entity), RET 4. Bytes 0072BCE0-0072BCFD:
// when 00521EA0(ECX = this+1Ch) returns the destroyed entity, the bot (this -
// 1Ch) gets its slot +30h with ECX = this - 1Ch.
void cc13_seg40_gun_bot_on_target_destroyed_0072bce0(void* target_member, void* destroyed, Cc13Seg40Host& host) {
    auto* self = static_cast<std::uint8_t*>(target_member);
    if (host.command_target_resolve_object_00521ea0(self + 0x1C) == destroyed) {
        host.vcall_30(self - 0x1C);
    }
}

// 00728BE0 BSP_SessionMessage_GUN_CREATE_Write, __thiscall(ECX = message,
// cursor), RET 4. Bytes 00728BE0-00728C8D: 00765000(ECX = message, cursor);
// then on the cursor: byte +111h (8 bits); the two words +112h (00429140, 2,
// 10h); byte +110h (8); the boolean +116h; when it is set, +118h and +11Ch as
// 004295C0(value, 0, 1, pi [00D7A264], 10h); the boolean +120h.
void cc13_seg40_session_message_gun_create_write_00728be0(void* message, void* cursor, Cc13Seg40Host& host) {
    auto* base = static_cast<std::uint8_t*>(message);
    host.net_entity_create_message_write_00765000(message, cursor);
    host.bit_cursor_write_byte_bits_00428ff0(cursor, at<std::uint8_t>(message, 0x111), 8);
    host.bit_cursor_write_word_array_00429140(cursor, base + 0x112, 2, 0x10);
    host.bit_cursor_write_byte_bits_00428ff0(cursor, at<std::uint8_t>(message, 0x110), 8);
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x116));
    if (at<std::uint8_t>(message, 0x116) != 0) {
        host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x118), 0, 1, f32(kCc13Seg40PiBits), 0x10);
        host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x11C), 0, 1, f32(kCc13Seg40PiBits), 0x10);
    }
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x120));
}

// 00728C90 BSP_SessionMessage_GUN_CREATE_Read, __thiscall(ECX = message,
// stream), RET 4. Bytes 00728C90-00728D34: 00767E30(ECX = message, stream);
// the cursor is stream+4h; the mirror of 00728BE0 through 00428C70, 00428E50,
// 00428D70 and 004293F0 (the +116h test re-reads the byte just read).
void cc13_seg40_session_message_gun_create_read_00728c90(void* message, void* stream, Cc13Seg40Host& host) {
    auto* base = static_cast<std::uint8_t*>(message);
    host.net_entity_create_message_read_00767e30(message, stream);
    void* cursor = static_cast<std::uint8_t*>(stream) + 4;
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, base + 0x111, 8);
    host.bit_cursor_read_word_array_00428e50(cursor, base + 0x112, 2, 0x10);
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, base + 0x110, 8);
    host.bit_cursor_read_boolean_00428d70(cursor, base + 0x116);
    if (at<std::uint8_t>(message, 0x116) != 0) {
        host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x118), 0, 1, f32(kCc13Seg40PiBits), 0x10);
        host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x11C), 0, 1, f32(kCc13Seg40PiBits), 0x10);
    }
    host.bit_cursor_read_boolean_00428d70(cursor, base + 0x120);
}

// 007290D0 BSP_SessionMessage_S_T_GUN_FIRE_Construct, __thiscall(ECX =
// message, gun, value_28, byte flag_2c, float value_30, float value_34, byte
// flag_3a, float value_3c), RET 1Ch, EAX = message. Bytes 007290D0-00729167:
// 0075B430(ECX = message, B0h); word +18h = 0, bytes +1Ah, +1Ch = 0; +4h = 1;
// vtable 00CFDE8C; +20h = gun+488h and +24h = gun+48Ch (FLD/FSTP float); +28h;
// byte +2Ch; +30h; +34h; word +38h = [[gun+3B4h]+4h]; byte +3Ah; +3Ch; then
// 00605070 on +20h and on +24h.
void* cc13_seg40_session_message_s_t_gun_fire_construct_007290d0(void* message, void* gun, std::uint32_t value_28,
                                                                 std::uint8_t flag_2c, float value_30,
                                                                 float value_34, std::uint8_t flag_3a,
                                                                 float value_3c, Cc13Seg40Host& host) {
    host.session_message_construct_base_0075b430(message, 0xB0);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint32_t>(message, 0) = 0x00CFDE8Cu;
    at<float>(message, 0x20) = at<float>(gun, 0x488);
    at<std::uint32_t>(message, 0x28) = value_28;
    at<float>(message, 0x24) = at<float>(gun, 0x48C);
    at<float>(message, 0x30) = value_30;
    at<float>(message, 0x34) = value_34;
    at<std::uint8_t>(message, 0x2C) = flag_2c;
    at<std::uint16_t>(message, 0x38) = at<std::uint16_t>(at<void*>(gun, 0x3B4), 4);
    at<std::uint8_t>(message, 0x3A) = flag_3a;
    at<float>(message, 0x3C) = value_3c;
    host.math_wrap_angle_in_place_00605070(&at<float>(message, 0x20));
    host.math_wrap_angle_in_place_00605070(&at<float>(message, 0x24));
    return message;
}

// 007291C0 BSP_SessionMessage_S_T_GUN_FIRE_Write, __thiscall(ECX = message,
// cursor), RET 4. Bytes 007291C0-007292B2: 0075B480(ECX = message, cursor);
// the boolean +1Ch; +20h and +24h as 004295C0(v, 0, 1, pi, 0Ah); +28h through
// 00429070(v, 5); word +38h through 00429030(v, 10h); the boolean +2Ch and,
// when set, +30h and +34h as 004295C0(v, 0, 1, FLT_MAX [00CFDBF8], 20h); the
// boolean +3Ah and, when set, +3Ch the same way.
void cc13_seg40_session_message_s_t_gun_fire_write_007291c0(void* message, void* cursor, Cc13Seg40Host& host) {
    const float pi = f32(kCc13Seg40PiBits);
    const float big = f32(kCc13Seg40FltMaxBits);
    host.session_message_write_extended_header_0075b480(message, cursor);
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x1C));
    host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x20), 0, 1, pi, 0x0A);
    host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x24), 0, 1, pi, 0x0A);
    host.bit_cursor_write_unsigned_dword_bits_00429070(cursor, at<std::uint32_t>(message, 0x28), 5);
    host.bit_cursor_write_signed_word_bits_00429030(cursor, at<std::uint16_t>(message, 0x38), 0x10);
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x2C));
    if (at<std::uint8_t>(message, 0x2C) != 0) {
        host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x30), 0, 1, big, 0x20);
        host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x34), 0, 1, big, 0x20);
    }
    host.bit_cursor_write_boolean_004290b0(cursor, at<std::uint8_t>(message, 0x3A));
    if (at<std::uint8_t>(message, 0x3A) != 0) {
        host.bit_cursor_write_numeric_float_004295c0(cursor, at<float>(message, 0x3C), 0, 1, big, 0x20);
    }
}

// 007292C0 BSP_SessionMessage_S_T_GUN_FIRE_Read, __thiscall(ECX = message,
// stream), RET 4. Bytes 007292C0-007293A1: 0075B4C0(ECX = message, stream);
// cursor = stream+4h; the mirror of 007291C0 through 00428D70, 004293F0,
// 00428D10(&+28h, 5), 00428CB0(&+38h, 10h).
void cc13_seg40_session_message_s_t_gun_fire_read_007292c0(void* message, void* stream, Cc13Seg40Host& host) {
    auto* base = static_cast<std::uint8_t*>(message);
    const float pi = f32(kCc13Seg40PiBits);
    const float big = f32(kCc13Seg40FltMaxBits);
    host.session_message_read_extended_header_0075b4c0(message, stream);
    void* cursor = static_cast<std::uint8_t*>(stream) + 4;
    host.bit_cursor_read_boolean_00428d70(cursor, base + 0x1C);
    host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x20), 0, 1, pi, 0x0A);
    host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x24), 0, 1, pi, 0x0A);
    host.bit_cursor_read_unsigned_dword_00428d10(cursor, &at<std::uint32_t>(message, 0x28), 5);
    host.bit_cursor_read_unsigned_word_00428cb0(cursor, &at<std::uint16_t>(message, 0x38), 0x10);
    host.bit_cursor_read_boolean_00428d70(cursor, base + 0x2C);
    if (at<std::uint8_t>(message, 0x2C) != 0) {
        host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x30), 0, 1, big, 0x20);
        host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x34), 0, 1, big, 0x20);
    }
    host.bit_cursor_read_boolean_00428d70(cursor, base + 0x3A);
    if (at<std::uint8_t>(message, 0x3A) != 0) {
        host.bit_cursor_read_numeric_float_004293f0(cursor, &at<float>(message, 0x3C), 0, 1, big, 0x20);
    }
}

namespace {

double f64(std::uint64_t bits) {
    double value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The angle wrap inlined twice in 0072A1D0 (0072A286-0072A2E5 and
// 0072A2E7-0072A37E). D = [00CE3828] (2pi from float, bits 401921FB60000000),
// lo = [00CE3D18] (-pi, C00921FB60000000), hi = [00CE3D28] (+pi,
// 400921FB60000000). slot = float source (FLD/FSTP); r = float(00BF857A(slot,
// D)) stored to the slot; FCOMIP lo against r with JB: when lo >= r (ordered)
// slot = r + D; otherwise FCOMI r against hi with JBE: when r > hi (ordered)
// slot = r - D; a NaN keeps r. The FADD/FSUB of the double constant rounds
// once at the game's 24-bit precision; here the double sum is rounded to float
// (a double rounding that can differ only in the last float bit).
void wrap_angle_slot(float* slot, float source, Cc13Seg40Host& host) {
    const double d = f64(0x401921FB60000000ull);
    const double lo = f64(0xC00921FB60000000ull);
    const double hi = f64(0x400921FB60000000ull);
    *slot = source;
    const float r = static_cast<float>(host.crt_fmod_00bf857a(static_cast<double>(*slot), d));
    *slot = r;
    if (lo >= static_cast<double>(r)) {
        *slot = static_cast<float>(static_cast<double>(r) + d);
    } else if (static_cast<double>(r) > hi) {
        *slot = static_cast<float>(static_cast<double>(r) - d);
    }
}

// The MeshHolder walk shared by 007297B0 / 00729870: the model node M =
// [gun+3C0h] gets 00B6DA70(model_factor, 0); then for every child of
// [M+30h] (first [+34h], next [+3Ch]) whose name (00B6D800 header +4h) is
// non-null and _stricmp-equal to "MeshHolder" (00CFDEA4), 00B6DA70(child,
// holder_factor, 0).
void mesh_holder_visibility(void* gun, float model_factor, float holder_factor, Cc13Seg40Host& host) {
    void* model = at<void*>(gun, 0x3C0);
    void* node = at<void*>(model, 0x30);
    host.scene_node_set_visibility_factor_00b6da70(model, model_factor, 0);
    for (void* child = at<void*>(node, 0x34); child != nullptr; child = at<void*>(child, 0x3C)) {
        const char* name = at<const char*>(host.node_get_name_header_00b6d800(child), 4);
        if (name != nullptr && host.crt_stricmp_00bf7fbf(name, "MeshHolder") == 0) {
            host.scene_node_set_visibility_factor_00b6da70(child, holder_factor, 0);
        }
    }
}

// The "destroyed" event body of 00729FD0 / 0072A0D0: when the event name's
// text (+4h) is non-null and _stricmp-equal to "destroyed" (00CE44D0), a
// 24h-byte message on the stack is built: 0075B430(&msg, type); word +18h =
// 0, byte +1Ah = 0, +4h = 1, byte +1Ch = 0, +20h = 256.0f ([00CF8B3C]); the
// vtable; then 0077C2A0(ECX = entity, &msg, 7, 0) and AL = 1. No destructor
// runs on the local message. Otherwise AL = 0. The local is zeroed here only
// because C++ needs a value.
bool on_event_destroyed(void* entity, const void* event_name, std::int32_t type, std::uint32_t vtable,
                        Cc13Seg40Host& host) {
    const char* name = at<const char*>(const_cast<void*>(event_name), 4);
    if (name == nullptr || host.crt_stricmp_00bf7fbf(name, "destroyed") != 0) {
        return false;
    }
    alignas(4) std::uint8_t message[0x24] = {};
    host.session_message_construct_base_0075b430(message, type);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<float>(message, 0x20) = 256.0f;
    at<std::uint32_t>(message, 0) = vtable;
    host.session_route_message_0077c2a0(entity, message, 7, 0);
    return true;
}

}  // namespace

// 0072A1D0 BSP_SessionMessage_GUN_CREATE_Construct, __thiscall(ECX = message,
// a, gun, byte value_111), RET 0Ch, EAX = message, EH frame 00C855A8. Bytes
// 0072A1D0-0072A37E: 00766FC0(ECX = message, ABh, a, gun, 0); vtable
// 00CFDE1C; byte +111h = value_111; byte +120h = byte gun+3B8h; dword +112h
// = 0, then for i < [gun+3ACh] word +112h+2i = word [[gun+3A8h]+14h*i+4h];
// byte +110h = byte gun+3FCh; byte +116h = gun's slot +5Ch(22h); when set,
// +118h and +11Ch are gun+488h and gun+48Ch wrapped into [-pi, pi] (see
// wrap_angle_slot).
void* cc13_seg40_session_message_gun_create_construct_0072a1d0(void* message, std::uint32_t a, void* gun,
                                                               std::uint8_t value_111, Cc13Seg40Host& host) {
    host.net_entity_create_message_construct_00766fc0(message, 0xAB, a, gun, 0);
    at<std::uint32_t>(message, 0) = 0x00CFDE1Cu;
    at<std::uint8_t>(message, 0x111) = value_111;
    at<std::uint8_t>(message, 0x120) = at<std::uint8_t>(gun, 0x3B8);
    at<std::uint32_t>(message, 0x112) = 0;
    for (std::int32_t i = 0; i < at<std::int32_t>(gun, 0x3AC); ++i) {
        at<std::uint16_t>(message, 0x112 + 2 * i) = at<std::uint16_t>(at<std::uint8_t*>(gun, 0x3A8) + 0x14 * i, 4);
    }
    at<std::uint8_t>(message, 0x110) = at<std::uint8_t>(gun, 0x3FC);
    const bool turning = host.vcall_5c_has_ordnance_kind(gun, 0x22);
    at<std::uint8_t>(message, 0x116) = turning ? 1 : 0;
    if (turning) {
        wrap_angle_slot(&at<float>(message, 0x118), at<float>(gun, 0x488), host);
        wrap_angle_slot(&at<float>(message, 0x11C), at<float>(gun, 0x48C), host);
    }
    return message;
}

// 0072A190 BSP_SessionMessage_GUN_CREATE_CreateEntity, __thiscall(ECX =
// message). Bytes 0072A190-0072A1C6: class = 00443490(ECX = word +1Eh, DL =
// 0); entity = class slot +28h(word +1Ch); entity+38Ch = byte +111h
// (zero-extended); byte entity+3B8h = byte +120h.
void cc13_seg40_session_message_gun_create_create_entity_0072a190(void* message, Cc13Seg40Host& host) {
    void* device_class = host.device_class_resolve_and_activate_00443490(at<std::uint16_t>(message, 0x1E), 0);
    void* entity = host.vcall_28_create(device_class, at<std::uint16_t>(message, 0x1C));
    at<std::uint32_t>(entity, 0x38C) = at<std::uint8_t>(message, 0x111);
    at<std::uint8_t>(entity, 0x3B8) = at<std::uint8_t>(message, 0x120);
}

// 0072B640 BSP_Gun_SendCreateMessage, __thiscall(ECX = gun, peer), RET 4, EH
// frame 00C855EB. Bytes 0072B640-0072B6B7: a 124h-byte stack message built by
// 0072A1D0(&msg, [[gun+3F4h]+6Ch], gun, the low byte of [gun+38Ch]);
// 00779FC0(ECX = gun, peer, &msg); then the destructor inline: vtable 00CFDE1C
// and 007668E0(&msg).
void cc13_seg40_gun_send_create_message_0072b640(void* gun, std::uint32_t peer, Cc13Seg40Host& host) {
    alignas(4) std::uint8_t message[0x124] = {};
    void* built = cc13_seg40_session_message_gun_create_construct_0072a1d0(
        message, at<std::uint32_t>(at<void*>(gun, 0x3F4), 0x6C), gun, at<std::uint8_t>(gun, 0x38C), host);
    host.entity_send_message_to_peer_00779fc0(gun, peer, built);
    at<std::uint32_t>(message, 0) = 0x00CFDE1Cu;
    host.net_entity_create_message_destruct_007668e0(message);
}

// 007297B0 BSP_Gun_ApplyWreckVisibility, __thiscall(ECX = gun). Bytes
// 007297B0-0072986B: when [gun+358h] is 0 the model is shown (1.0f) and every
// MeshHolder child hidden (0.0f); otherwise the model is hidden and the
// MeshHolder children shown.
void cc13_seg40_gun_apply_wreck_visibility_007297b0(void* gun, Cc13Seg40Host& host) {
    if (at<std::uint32_t>(gun, 0x358) == 0) {
        mesh_holder_visibility(gun, 1.0f, 0.0f, host);
    } else {
        mesh_holder_visibility(gun, 0.0f, 1.0f, host);
    }
}

// 00729870 BSP_Gun_ShowOnlyMeshHolder, __thiscall(ECX = gun): model 0.0f,
// MeshHolder children 1.0f (bytes 00729870-007298CB).
void cc13_seg40_gun_show_only_mesh_holder_00729870(void* gun, Cc13Seg40Host& host) {
    mesh_holder_visibility(gun, 0.0f, 1.0f, host);
}

// 00729FD0 FUN_00729fd0, __thiscall(ECX = entity, const event name*, unused),
// RET 8, AL, EH frame 00C85568: the "destroyed" event sends message type 68h
// (vtable 00CFDE64). Bytes 00729FD0-0072A080.
bool cc13_seg40_gun_on_event_destroyed_00729fd0(void* entity, const void* event_name, std::uint32_t unused,
                                                Cc13Seg40Host& host) {
    (void)unused;
    return on_event_destroyed(entity, event_name, 0x68, 0x00CFDE64u, host);
}

// 0072A0D0 FUN_0072a0d0: the same body with type 69h and vtable 00CFDE78.
bool cc13_seg40_gun_on_event_destroyed_0072a0d0(void* entity, const void* event_name, std::uint32_t unused,
                                                Cc13Seg40Host& host) {
    (void)unused;
    return on_event_destroyed(entity, event_name, 0x69, 0x00CFDE78u, host);
}

// 0072AA10 BSP_GunEffectSet_SetMix, __thiscall(ECX = set, float mix), RET 4.
// Bytes 0072AA10-0072AA7B: slot +20h(mix) on every non-null element of {data
// +0Ch, count +10h} (the 00E19AB0 mask test), then on every element of {data
// +18h, count +1Ch} without a test.
void cc13_seg40_gun_effect_set_set_mix_0072aa10(void* set, float mix, Cc13Seg40Host& host) {
    void** first = at<void**>(set, 0xC);
    void** last = first + at<std::int32_t>(set, 0x10);
    for (void** it = first; it != last; ++it) {
        if (*it != nullptr) {
            host.vcall_20_mix(*it, mix);
        }
    }
    first = at<void**>(set, 0x18);
    last = first + at<std::int32_t>(set, 0x1C);
    for (void** it = first; it != last; ++it) {
        host.vcall_20_mix(*it, mix);
    }
}

// 0072ABE0 BSP_GunBarrelTable_Destruct, __thiscall(ECX = table): vtable
// 00CFDC0C; [+0Ch] then [+10h] _free (00BF6989) and cleared when non-null.
void cc13_seg40_gun_barrel_table_destruct_0072abe0(void* table, Cc13Seg40Host& host) {
    at<std::uint32_t>(table, 0) = 0x00CFDC0Cu;
    if (at<void*>(table, 0xC) != nullptr) {
        host.crt_free_00bf6989(at<void*>(table, 0xC));
        at<void*>(table, 0xC) = nullptr;
    }
    if (at<void*>(table, 0x10) != nullptr) {
        host.crt_free_00bf6989(at<void*>(table, 0x10));
        at<void*>(table, 0x10) = nullptr;
    }
}

// 0072B5A0 BSP_Gun_ApplySetDestroyedFailure, __thiscall(ECX = gun, request),
// RET 4. Bytes 0072B5A0-0072B637: when the request's slot +0Ch(68h) is true:
// byte gun+3B8h = 1; index = 0 when [gun+360h] is null, its +16Ch is null, or
// ([+170h] - [+16Ch]) >> 4 is 0; else that count - 1 (the image's dead -1
// arm for a null +16Ch cannot be reached after the first test); gun slot
// +1B4h(index). Then when byte [[gun+3F0h]+720h], byte gun+3B8h or byte
// gun+5Dh is set: 0072B4C0(gun) and 00956C20(ECX = [gun+3F0h]).
void cc13_seg40_gun_apply_set_destroyed_failure_0072b5a0(void* gun, void* request, Cc13Seg40Host& host) {
    if (host.vcall_0c_is(request, 0x68)) {
        void* table = at<void*>(gun, 0x360);
        at<std::uint8_t>(gun, 0x3B8) = 1;
        std::int32_t index = 0;
        if (table != nullptr && at<std::uint32_t>(table, 0x16C) != 0) {
            const std::int32_t count = static_cast<std::int32_t>(at<std::uint32_t>(table, 0x170) -
                                                                 at<std::uint32_t>(table, 0x16C)) >> 4;
            if (count != 0) {
                index = count - 1;
            }
        }
        host.vcall_1b4(gun, index);
    }
    if (at<std::uint8_t>(at<void*>(gun, 0x3F0), 0x720) != 0 || at<std::uint8_t>(gun, 0x3B8) != 0 ||
        at<std::uint8_t>(gun, 0x5D) != 0) {
        host.gun_stop_firing_effects_0072b4c0(gun);
        host.unit_rebuild_weapon_category_index_00956c20(at<void*>(gun, 0x3F0));
    }
}

// 00730E80 BSP_RapidFixedSlaveGun_Construct, __thiscall(ECX = gun, arg), RET 4,
// EAX = gun: 006FDDA0(ECX = gun, arg); vtables 00CFE548 (+0h), 00CFE530 (+10h),
// 00CFE528 (+24h), 00CFE524 (+170h), 00CFE51C (+1E4h), 00CFE504 (+310h); kind
// +C4h = 23h.
void* cc13_seg40_rapid_fixed_slave_gun_construct_00730e80(void* gun, std::uint32_t arg, Cc13Seg40Host& host) {
    host.turning_gun_base_construct_006fdda0(gun, arg);
    at<std::uint32_t>(gun, 0) = 0x00CFE548u;
    at<std::uint32_t>(gun, 0x10) = 0x00CFE530u;
    at<std::uint32_t>(gun, 0x24) = 0x00CFE528u;
    at<std::uint32_t>(gun, 0x170) = 0x00CFE524u;
    at<std::uint32_t>(gun, 0x1E4) = 0x00CFE51Cu;
    at<std::uint32_t>(gun, 0x310) = 0x00CFE504u;
    at<std::uint32_t>(gun, 0xC4) = 0x23;
    return gun;
}

// 00730FB0 BSP_GunAimError_Write, __thiscall(ECX = error, writer*), RET 4.
// Bytes 00730FB0-00731017: writer slot +0Ch(0, "type" 00CE4AD0, type 0, the
// text "flak" 00CFE738), then (0, "distErr" 00CFD51C, type 2, float +4h).
void cc13_seg40_gun_aim_error_write_00730fb0(void* error, void* writer, Cc13Seg40Host& host) {
    static const char kFlak[] = "flak";
    host.archive_vcall_0c_put(writer, 0, "type", 0,
                              static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(kFlak)));
    host.archive_vcall_0c_put(writer, 0, "distErr", 2, at<std::uint32_t>(error, 4));
}

// 00731220 BSP_TorpedoShotCommand_Destruct, __thiscall(ECX = command), EH frame
// 00C858B8: vtable 00CFE740; 004B7EF0(ECX = command+0Ch); base vtable 00CFE2B8.
void cc13_seg40_torpedo_shot_command_destruct_00731220(void* command, Cc13Seg40Host& host) {
    at<std::uint32_t>(command, 0) = 0x00CFE740u;
    host.observer_link_destroy_base_004b7ef0(static_cast<std::uint8_t*>(command) + 0xC);
    at<std::uint32_t>(command, 0) = 0x00CFE2B8u;
}

// 0072BDD0 BSP_GunBot_SetFireTargetPoint, __thiscall(ECX = bot, x, y, z),
// RET 0Ch: the bot's slot +30h() first, then word +38h = 0, byte +39h = 1,
// +40h/+44h/+48h = x/y/z, +3Ch = 0, byte +38h = 0, word +3Ah = 0, +4Ch = 0
// (the pseudocode's store order).
void cc13_seg40_gun_bot_set_fire_target_point_0072bdd0(void* bot, std::uint32_t x, std::uint32_t y, std::uint32_t z,
                                                        Cc13Seg40Host& host) {
    host.vcall_30(bot);
    at<std::uint16_t>(bot, 0x38) = 0;
    at<std::uint8_t>(bot, 0x39) = 1;
    at<std::uint32_t>(bot, 0x40) = x;
    at<std::uint32_t>(bot, 0x44) = y;
    at<std::uint32_t>(bot, 0x48) = z;
    at<std::uint32_t>(bot, 0x3C) = 0;
    at<std::uint8_t>(bot, 0x38) = 0;
    at<std::uint16_t>(bot, 0x3A) = 0;
    at<std::uint32_t>(bot, 0x4C) = 0;
}

namespace {

// The node-factory body of 00731C80 / 00731D50: operator new (00BF55BE)
// `size` bytes, memset (00BF79F0) 0 before the null test, construct when
// non-null (a null block leaves the gun null and the image writes through it
// anyway); gun+3F4h = gun+354h = class; 0041DD40(ECX = gun+154h,
// strlen([class+54h]) or 0, 0); memcpy (00BF7680) of the resized length
// [gun+154h] from [class+54h] when [gun+158h] is non-null.
void* create_gun_node(void* gun_class, std::uint32_t arg, std::uint32_t size,
                      void* (*construct)(void*, std::uint32_t, Cc13Seg40Host&), Cc13Seg40Host& host) {
    void* block = host.operator_new_00bf55be(size);
    host.crt_memset_00bf79f0(block, 0, size);
    void* gun = block != nullptr ? construct(block, arg, host) : nullptr;
    at<void*>(gun, 0x3F4) = gun_class;
    at<void*>(gun, 0x354) = gun_class;
    const char* name = at<const char*>(gun_class, 0x54);
    std::uint32_t length = 0;
    if (name != nullptr) {
        while (name[length] != '\0') {
            ++length;
        }
    }
    auto* string = static_cast<std::uint8_t*>(gun) + 0x154;
    host.native_string_resize_0041dd40(string, length, 0);
    if (at<void*>(string, 4) != nullptr) {
        host.crt_memcpy_00bf7680(at<void*>(string, 4), name, at<std::uint32_t>(string, 0));
    }
    return gun;
}

void* construct_multiple_bomb_platform(void* block, std::uint32_t arg, Cc13Seg40Host& host) {
    return host.multiple_bomb_platform_construct_00730b80(block, arg);
}

}  // namespace

// 00731C80 BSP_NodeFactory_CreateMultipleBombPlatform, __thiscall(ECX = gun
// class, arg), RET 4, EAX, EH frame 00C859C8: 480h bytes, 00730B80.
void* cc13_seg40_node_factory_create_multiple_bomb_platform_00731c80(void* gun_class, std::uint32_t arg,
                                                                     Cc13Seg40Host& host) {
    return create_gun_node(gun_class, arg, 0x480, &construct_multiple_bomb_platform, host);
}

// 00731D50 BSP_NodeFactory_CreateRapidFixedSlaveGun: the same body with 4D0h
// bytes and 00730E80.
void* cc13_seg40_node_factory_create_rapid_fixed_slave_gun_00731d50(void* gun_class, std::uint32_t arg,
                                                                    Cc13Seg40Host& host) {
    return create_gun_node(gun_class, arg, 0x4D0, &cc13_seg40_rapid_fixed_slave_gun_construct_00730e80, host);
}

// 00733B10 BSP_SessionMessage_MISSIONINFO_Construct, __thiscall(ECX =
// message, const native string* text), RET 4, EH frame 00C85C53. Bytes
// 00733B10-00733B91: 0075B430(ECX = message, 35h); +4h = 1; vtable 00CFE96C;
// the string at +18h = {0, null}; unless it is `text` itself: 0041DD40(ECX =
// +18h, text size, 1) and, when the text size is non-zero, memcpy (00BF7680)
// of the resized size [+18h] from the text data into [+1Ch].
void* cc13_seg40_session_message_missioninfo_construct_00733b10(void* message, const void* text,
                                                                Cc13Seg40Host& host) {
    host.session_message_construct_base_0075b430(message, 0x35);
    at<std::uint32_t>(message, 4) = 1;
    auto* string = static_cast<std::uint8_t*>(message) + 0x18;
    at<std::uint32_t>(message, 0) = 0x00CFE96Cu;
    at<std::uint32_t>(string, 0) = 0;
    at<std::uint32_t>(string, 4) = 0;
    if (string != text) {
        void* source = const_cast<void*>(text);
        host.native_string_resize_0041dd40(string, at<std::uint32_t>(source, 0), 1);
        if (at<std::uint32_t>(source, 0) != 0) {
            host.crt_memcpy_00bf7680(at<void*>(string, 4), at<void*>(source, 4), at<std::uint32_t>(string, 0));
        }
    }
    return message;
}

// 00731A50 BSP_GunClass_ActivateModelAndBulletClasses, __thiscall(ECX = gun
// class, arg), RET 4. Bytes 00731A50-00731AD2: only when byte +44h is 0:
// 0043EBD0(ECX = class, arg); then over the 48h-byte bullet records {data
// +74h, count +78h} (end taken once): for a non-null record whose +34h is
// non-null (the 00E19983 mask test), 006EAFE0(ECX = &slot, EDX =
// [[record+34h]+0Ch]) builds a reference in the argument's stack slot, which
// is then released and cleared. The slot starts as the argument; 006EAFE0
// constructs it without reading it, so it is a local here.
void cc13_seg40_gun_class_activate_model_and_bullet_classes_00731a50(void* gun_class, std::uint32_t arg,
                                                                     Cc13Seg40Host& host) {
    if (at<std::uint8_t>(gun_class, 0x44) != 0) {
        return;
    }
    host.damageable_class_ensure_model_resource_0043ebd0(gun_class, arg);
    auto* record = at<std::uint8_t*>(gun_class, 0x74);
    auto* end = record + 0x48 * at<std::int32_t>(gun_class, 0x78);
    for (; record != end; record += 0x48) {
        if (record != nullptr && at<void*>(record, 0x34) != nullptr) {
            void* slot = nullptr;
            host.bullet_class_acquire_and_finalise_006eafe0(&slot, at<std::uint32_t>(at<void*>(record, 0x34), 0xC));
            if (slot != nullptr) {
                if (host.interlocked_decrement(&at<std::int32_t>(slot, 4)) == 0) {
                    host.vcall_00_release(slot);
                }
                slot = nullptr;
            }
        }
    }
}

// 0072CAE0 BSP_NativeVector14_Destruct_0072cae0, __thiscall(ECX = vector):
// 0072B6D0(ECX = vector, 0), then _free (00BF6989) of [vector].
void cc13_seg40_native_vector14_destruct_0072cae0(void* vector, Cc13Seg40Host& host) {
    host.native_vector14_resize_0072b6d0(vector, 0);
    host.crt_free_00bf6989(at<void*>(vector, 0));
}

// 0072CC60 BSP_NativeParticlePopulationLock_Destruct, __thiscall(ECX = lock):
// vtable 00CFDEB4; 0041CC80(ECX = lock+4h); the singleton [0108FF50] = 0;
// base vtable 00CE3818.
void cc13_seg40_native_particle_population_lock_destruct_0072cc60(void* lock, std::uint32_t* global_0108ff50,
                                                                  Cc13Seg40Host& host) {
    at<std::uint32_t>(lock, 0) = 0x00CFDEB4u;
    host.critical_section_destroy_owned_0041cc80(static_cast<std::uint8_t*>(lock) + 4);
    *global_0108ff50 = 0;
    at<std::uint32_t>(lock, 0) = 0x00CE3818u;
}

namespace {

// The callback-holder destructor body of 00733E70 / 00733F00: vtable; the Lua
// argument vector V = [+args]: when non-null, 006B88E0(ECX = V, 0), _free
// (00BF6989) of [V], _free (00BF65AC) of V, [+args] = 0; then the native
// string {size, data} at +name: when data is non-null, the sized pool takes it
// back (00419CC0, 00BD1510(data, size + 1, 1)).
void destroy_callback_holder(void* holder, std::uint32_t vtable, std::size_t args, std::size_t name,
                             Cc13Seg40Host& host) {
    at<std::uint32_t>(holder, 0) = vtable;
    void* vector = at<void*>(holder, args);
    if (vector != nullptr) {
        host.lua_variant_vector_resize_006b88e0(vector, 0);
        host.crt_free_00bf6989(at<void*>(vector, 0));
        host.crt_free_00bf65ac(vector);
        at<void*>(holder, args) = nullptr;
    }
    void* data = at<void*>(holder, name + 4);
    if (data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data,
                                              at<std::uint32_t>(holder, name) + 1, 1);
    }
}

}  // namespace

// 00733E70 BSP_CountdownCallbackHolder_Destruct, __thiscall(ECX = holder), EH
// frame 00C85CCB: vtable 00CFE964, vector +24h, string {+1Ch, +20h}.
void cc13_seg40_countdown_callback_holder_destruct_00733e70(void* holder, Cc13Seg40Host& host) {
    destroy_callback_holder(holder, 0x00CFE964u, 0x24, 0x1C, host);
}

// 00733F00 BSP_NarrativeCallbackHolder_Destruct, __thiscall(ECX = holder), EH
// frame 00C85CEB: vtable 00CFE968, vector +1Ch, string {+14h, +18h}.
void cc13_seg40_narrative_callback_holder_destruct_00733f00(void* holder, Cc13Seg40Host& host) {
    destroy_callback_holder(holder, 0x00CFE968u, 0x1C, 0x14, host);
}

// 00733DC0 BSP_NarrativeEntry_Destruct, __thiscall(ECX = entry), EH frame
// 00C85CB6. Bytes 00733DC0-00733E60: the holder body with vtable 00CFE960,
// vector +14h and string {+0Ch, +10h}; then the second string {+4h, +8h} goes
// back to the pool the same way.
void cc13_seg40_narrative_entry_destruct_00733dc0(void* entry, Cc13Seg40Host& host) {
    destroy_callback_holder(entry, 0x00CFE960u, 0x14, 0x0C, host);
    void* data = at<void*>(entry, 8);
    if (data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), data, at<std::uint32_t>(entry, 4) + 1, 1);
    }
}

// 00735080 BSP_MissionNarrative_Destruct, __thiscall(ECX = narrative), EH
// frame 00C85E56. Bytes 00735080-007350E9: vtable 00CFE9C0; 00733F00(ECX =
// +38h); 00733E70(ECX = +10h); 00734CE0(ECX = narrative); _free (00BF65AC) of
// [+4h] and [+4h] = 0 (the bytes read +4h; the pseudocode's param_1[2] is
// wrong).
void cc13_seg40_mission_narrative_destruct_00735080(void* narrative, Cc13Seg40Host& host) {
    auto* base = static_cast<std::uint8_t*>(narrative);
    at<std::uint32_t>(narrative, 0) = 0x00CFE9C0u;
    cc13_seg40_narrative_callback_holder_destruct_00733f00(base + 0x38, host);
    cc13_seg40_countdown_callback_holder_destruct_00733e70(base + 0x10, host);
    host.narrative_list_clear_00734ce0(narrative);
    host.crt_free_00bf65ac(at<void*>(narrative, 4));
    at<void*>(narrative, 4) = nullptr;
}

// 00731270 BSP_TorpedoShotCommand_Read, __thiscall(ECX = command, reader*),
// RET 4: reader slot +10h(0, "heading" 00CFE758, type 2, &+4h), then (0,
// "swimdepth" 00CFE74C, type 2, &+8h).
void cc13_seg40_torpedo_shot_command_read_00731270(void* command, void* reader, Cc13Seg40Host& host) {
    auto* base = static_cast<std::uint8_t*>(command);
    host.archive_vcall_10_get(reader, 0, "heading", 2, base + 4);
    host.archive_vcall_10_get(reader, 0, "swimdepth", 2, base + 8);
}

}  // namespace bsp
