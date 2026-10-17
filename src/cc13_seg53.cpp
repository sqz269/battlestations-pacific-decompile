// cc13_seg53: segment 53 reconstructions (docs/CC13_SEG53.md). See include/bsp/cc13_seg53.hpp.
// x87 model: the image runs with 53-bit precision, so an FSTP to a float slot is a float cast.

#include "bsp/cc13_seg53.hpp"

#include <cmath>
#include <cstring>

namespace bsp::cc13_seg53 {

namespace {
constexpr std::uintptr_t kEmptyNativeString_00f8756c = 0x00F8756C;  // shared "" for a null ptr
constexpr std::uint32_t kGunTurnSyncVtable_00d032b4 = 0x00D032B4;
constexpr std::uint32_t kGunTurnSyncType = 0xAE;
// [00D7A208] = -0.0f: the "else" arm of the abs below is -0.0f - d.
constexpr float kNegZero_00d7a208 = -0.0f;
// [00D7A238] = 0.01f: a channel is resent when |wrapped delta| reaches it.
constexpr float kResendThreshold_00d7a238 = 0.0099999998f;
// [00D7A23C] = 0.001f: the drift added to an unsent channel's last-sent value.
constexpr float kUnsentDrift_00d7a23c = 0.0010000000f;
// [00D7A264] = pi (float): quantization range of every angle channel.
constexpr float kAngleRange_00d7a264 = 3.14159274f;
}  // namespace

// ---------------------------------------------------------------------------
// 00851EE0 BSP_FixedStringBuilder_AppendBytes_Unreferenced, __thiscall(ECX builder; src, n),
// RET 8. Unsigned compare (JA): appends only when length+n <= capacity. memcpy is the CRT
// 00BF7680. No PE reference.
// ---------------------------------------------------------------------------
void fixed_string_builder_append_bytes_00851ee0(Seg53Host&, Object builder, const void* src,
                                                std::size_t n) {
    std::uint32_t length = field<std::uint32_t>(builder, 0xC);
    if (length + n <= field<std::uint32_t>(builder, 0x8)) {
        std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(builder, 0x4) + length), src, n);
        field<std::uint32_t>(builder, 0xC) += static_cast<std::uint32_t>(n);
        reinterpret_cast<std::uint8_t*>(field<std::uintptr_t>(builder, 0x4))
            [field<std::uint32_t>(builder, 0xC)] = 0;
    }
}

// ---------------------------------------------------------------------------
// 00852140 BSP_FixedStringBuilder_AppendNativeString, __thiscall(ECX builder; NativeString*
// {size, ptr}), RET 4 -> EAX = builder. A null ptr reads as the shared empty string 00F8756C.
// ---------------------------------------------------------------------------
Object fixed_string_builder_append_native_string_00852140(Seg53Host&, Object builder,
                                                          const std::uint32_t* native_string) {
    std::uint32_t size = native_string[0];
    std::uintptr_t text = native_string[1];
    if (text == 0) {
        text = kEmptyNativeString_00f8756c;
    }
    std::uint32_t length = field<std::uint32_t>(builder, 0xC);
    if (length + size <= field<std::uint32_t>(builder, 0x8)) {
        std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(builder, 0x4) + length),
                    reinterpret_cast<const void*>(text), size);
        field<std::uint32_t>(builder, 0xC) += size;
        reinterpret_cast<std::uint8_t*>(field<std::uintptr_t>(builder, 0x4))
            [field<std::uint32_t>(builder, 0xC)] = 0;
    }
    return builder;
}

// ---------------------------------------------------------------------------
// 00855FE0 BSP_ObserverLink14_Assign_Unreferenced, __thiscall(ECX link; target), RET 4 -> link.
// When +14h changes: unregister the old pair (ECX = old, EDX = link) if old was set, store, and
// register the new pair (ECX = target, EDX = link) if target is set.
// ---------------------------------------------------------------------------
Object observer_link14_assign_00855fe0(Seg53Host& host, Object link, Object target) {
    Object old = field<Object>(link, 0x14);
    if (old != target) {
        if (old != nullptr) {
            host.observer_unregister_pair_006952a0(old, link);
        }
        field<Object>(link, 0x14) = target;
        if (target != nullptr) {
            host.observer_register_pair_00694a60(target, link);
        }
    }
    return link;
}

// ---------------------------------------------------------------------------
// 008580C0 BSP_NodeHandle_UnlinkAndClear_Unreferenced, __thiscall(ECX = Node**), RET 0.
// ---------------------------------------------------------------------------
void node_handle_unlink_and_clear_008580c0(Seg53Host& host, Object* handle) {
    if (*handle != nullptr) {
        host.node_unlink_and_release_00b6dfa0(*handle);
        *handle = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 00858330 BSP_NativeTraceline_ScalarDeleteToModelPool, __thiscall(ECX traceline; flags), RET 4
// -> traceline. Vtable 00D0C8C8 slot 1: destroy, then on flags bit 0 return the slot to the
// model pool (ECX = traceline) instead of freeing.
// ---------------------------------------------------------------------------
Object native_traceline_scalar_delete_to_pool_00858330(Seg53Host& host, Object traceline,
                                                       std::uint8_t flags) {
    host.native_traceline_destroy_00af1980(traceline);
    if ((flags & 1) != 0) {
        host.native_model_pool_return_slot_00b748c0(traceline);
    }
    return traceline;
}

// 00858180 RET 4, 008581B0 RET 8, 00858200 RET, 008581E0 XOR AL,AL; RET (shared vtable leaves).
void noop_ret4_00858180(std::uint32_t) {}
void noop_ret8_008581b0(std::uint32_t, std::uint32_t) {}
void noop_00858200() {}
bool return_false_008581e0() { return false; }

// ---------------------------------------------------------------------------
// 00859410 BSP_SessionMessage_T_GUN_TURN_SYNC_ConstructFromGun, __thiscall(ECX message; record,
// byte mode, float a28, float a2C, float a20, float a24, int a30), RET 1Ch -> message.
// Base 0075B430(0AEh); +18h word 0, +1Ah byte 0, +4 = 0, +1Ch = +1Dh = mode, vtable 00D032B4,
// +40h = record; then the four angle slots are wrapped in place in the order +20h, +24h, +28h,
// +2Ch. Floats are MOVSS copies (no rounding).
// ---------------------------------------------------------------------------
Object gun_turn_sync_construct_00859410(Seg53Host& host, Object message, Object record,
                                        std::uint8_t mode, float a28, float a2c, float a20,
                                        float a24, std::int32_t a30) {
    host.session_message_construct_base_0075b430(message, kGunTurnSyncType);
    field<float>(message, 0x20) = a20;
    field<std::uint16_t>(message, 0x18) = 0;
    field<std::uint8_t>(message, 0x1A) = 0;
    field<std::uint32_t>(message, 0x04) = 0;
    field<float>(message, 0x24) = a24;
    field<std::uint8_t>(message, 0x1C) = mode;
    field<std::uint8_t>(message, 0x1D) = mode;
    field<float>(message, 0x28) = a28;
    field<std::uint32_t>(message, 0x00) = kGunTurnSyncVtable_00d032b4;
    field<float>(message, 0x2C) = a2c;
    field<std::int32_t>(message, 0x30) = a30;
    field<Object>(message, 0x40) = record;
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x20));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x24));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x28));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x2C));
    return message;
}

// ---------------------------------------------------------------------------
// 00859C80 BSP_SessionMessage_T_GUN_TURN_SYNC_Write, __thiscall(ECX message; stream), RET 4.
// Header, the two flag bytes +1Ch/+1Dh, then [message]->vtable[14h](ECX = message, stream)\n// (00859CC0 for this vtable).
// ---------------------------------------------------------------------------
void gun_turn_sync_write_00859c80(Seg53Host& host, Object message, Object stream) {
    host.session_message_write_header_0075b480(message, stream);
    host.bit_write_bool_004290b0(stream, field<std::uint8_t>(message, 0x1C));
    host.bit_write_bool_004290b0(stream, field<std::uint8_t>(message, 0x1D));
    host.message_vslot_14(message, stream);
}

namespace {
// One channel of 00859CC0: d = (float)SubtractWrapped(value, sent); |d| via the -0.0 - d arm
// (FCOMIP/JBE: d <= 0 or unordered takes it); COMISS/JBE: when threshold <= |d| or unordered,
// write 1 and the quantized value and record it as sent; otherwise write 0 and drift the
// sent value by +0.001 through AddWrapped (FSTP to float).
void write_angle_channel(Seg53Host& host, Object stream, float value, float* sent,
                         std::uint32_t bits) {
    float d = host.math_subtract_wrapped_angle_00438b10(value, *sent);
    float magnitude = d > 0.0f ? d : kNegZero_00d7a208 - d;
    if (!(kResendThreshold_00d7a238 > magnitude)) {
        host.bit_write_bool_004290b0(stream, 1);
        host.bit_write_float_004295c0(stream, value, 0, 1, kAngleRange_00d7a264, bits);
        *sent = value;
    } else {
        host.bit_write_bool_004290b0(stream, 0);
        *sent = host.math_add_wrapped_angle_00438aa0(*sent, kUnsentDrift_00d7a23c);
    }
}
}  // namespace

// ---------------------------------------------------------------------------
// 00859CC0 BSP_SessionMessage_T_GUN_TURN_SYNC_WriteAngleDeltas, __thiscall(ECX message; stream),
// RET 4. Only when +1Dh: bits = +1Ch ? 8 : 12; channels (+28h vs rec+8), (+2Ch vs rec+0Ch),
// (+20h vs rec+0), (+24h vs rec+4) where rec = [+40h]; then 00429090(+30h, 2). The value
// re-read after each "sent" store is the message slot itself (FLD [ESI+n]; FSTP [rec+m]).
// ---------------------------------------------------------------------------
void gun_turn_sync_write_angle_deltas_00859cc0(Seg53Host& host, Object message, Object stream) {
    if (field<std::uint8_t>(message, 0x1D) == 0) {
        return;
    }
    std::uint32_t bits = field<std::uint8_t>(message, 0x1C) == 0 ? 12u : 8u;
    float* rec = field<float*>(message, 0x40);
    write_angle_channel(host, stream, field<float>(message, 0x28), &rec[2], bits);
    write_angle_channel(host, stream, field<float>(message, 0x2C), &rec[3], bits);
    write_angle_channel(host, stream, field<float>(message, 0x20), &rec[0], bits);
    write_angle_channel(host, stream, field<float>(message, 0x24), &rec[1], bits);
    host.bit_write_signed_00429090(stream, field<std::int32_t>(message, 0x30), 2);
}

// ---------------------------------------------------------------------------
// 00859FA0 BSP_SessionMessage_T_GUN_TURN_SYNC_Read, __thiscall(ECX message; stream), RET 4.
// Header, flags into +1Ch/+1Dh through the cursor at stream+4, then [message]->vtable[18h](stream)\n// (00859FE0 for this vtable).
// ---------------------------------------------------------------------------
void gun_turn_sync_read_00859fa0(Seg53Host& host, Object message, Object stream) {
    host.session_message_read_header_0075b4c0(message, stream);
    host.bit_read_bool_00428d70(stream + 4, &field<std::uint8_t>(message, 0x1C));
    host.bit_read_bool_00428d70(stream + 4, &field<std::uint8_t>(message, 0x1D));
    host.message_vslot_18(message, stream);
}

// ---------------------------------------------------------------------------
// 00859FE0 BSP_SessionMessage_T_GUN_TURN_SYNC_ReadAngleDeltas, __thiscall(ECX message; stream),
// RET 4. Mirror of 00859CC0: per channel a present flag (+36h for +28h, +37h for +2Ch, +34h for
// +20h, +35h for +24h) then the quantized float when set; finally +30h with 2 bits.
// ---------------------------------------------------------------------------
void gun_turn_sync_read_angle_deltas_00859fe0(Seg53Host& host, Object message, Object stream) {
    if (field<std::uint8_t>(message, 0x1D) == 0) {
        return;
    }
    std::uint32_t bits = field<std::uint8_t>(message, 0x1C) == 0 ? 12u : 8u;
    Object cursor = stream + 4;
    static constexpr std::size_t kFlag[4] = {0x36, 0x37, 0x34, 0x35};
    static constexpr std::size_t kValue[4] = {0x28, 0x2C, 0x20, 0x24};
    for (int i = 0; i < 4; ++i) {
        host.bit_read_bool_00428d70(cursor, &field<std::uint8_t>(message, kFlag[i]));
        if (field<std::uint8_t>(message, kFlag[i]) != 0) {
            host.bit_read_float_004293f0(cursor, &field<float>(message, kValue[i]), 0, 1,
                                         kAngleRange_00d7a264, bits);
        }
    }
    host.bit_read_signed_00428d30(cursor, &field<std::int32_t>(message, 0x30), 2);
}

// ---------------------------------------------------------------------------
// 00859830 BSP_Unit_SetGunAimRecord, __thiscall(ECX unit; float a1, a2, a3, a4, a5), RET 14h.
// +4A0h = [00F876A8] (the mission clock), +4A4h..+4ACh = a1..a3 (MOVSS copies), then
// 004B4D80(out, a5, a4) with out reusing the dead a1..a3 argument slots; +4B0h..+4B8h = out.
// ---------------------------------------------------------------------------
void unit_set_gun_aim_record_00859830(Seg53Host& host, Object unit, float a4a4, float a4a8,
                                      float a4ac, float angle_b, float angle_a) {
    field<float>(unit, 0x4A0) = host.mission_clock_00f876a8();
    field<float>(unit, 0x4A4) = a4a4;
    field<float>(unit, 0x4A8) = a4a8;
    field<float>(unit, 0x4AC) = a4ac;
    float out[3];
    const float* direction = host.lighting_direction_from_angles_004b4d80(out, angle_a, angle_b);
    field<float>(unit, 0x4B0) = direction[0];
    field<float>(unit, 0x4B4) = direction[1];
    field<float>(unit, 0x4B8) = direction[2];
}

// ---------------------------------------------------------------------------
// 00859920 BSP_TurningGun_CommitStepAngles, __thiscall(ECX gun), RET 0. +178h = +170h,
// +17Ch = +174h (FLD/FSTP float), then wrap +178h and +17Ch in place (the second as a tail JMP).
// ---------------------------------------------------------------------------
void turning_gun_commit_step_angles_00859920(Seg53Host& host, Object gun) {
    field<float>(gun, 0x178) = field<float>(gun, 0x170);
    field<float>(gun, 0x17C) = field<float>(gun, 0x174);
    host.math_wrap_angle_in_place_00605070(&field<float>(gun, 0x178));
    host.math_wrap_angle_in_place_00605070(&field<float>(gun, 0x17C));
}

// ---------------------------------------------------------------------------
// 0085BA30 BSP_TurningGun_ApplySnapshotFields, __thiscall(ECX gun; T_GUN_TURN_SYNC* message),
// RET 4. Accepts when +4CCh == -1 or message+0Ch >= +4CCh (signed JL skips); stores the
// sequence, then returns if the gun is the local player's (roles 0 and 1) or message+1Dh is 0.
// Otherwise +49Ch = message+30h and each present channel (flags +36h/+34h/+37h/+35h) replaces
// +4C4h/+4BCh/+4C8h/+4C0h; then 0085B0F0(+4C4h, +4C8h, +4BCh, +4C0h).
// ---------------------------------------------------------------------------
void turning_gun_apply_snapshot_fields_0085ba30(Seg53Host& host, Object gun, Object message) {
    std::int32_t last = field<std::int32_t>(gun, 0x4CC);
    std::int32_t sequence = field<std::int32_t>(message, 0xC);
    if (last != -1 && sequence < last) {
        return;
    }
    field<std::int32_t>(gun, 0x4CC) = sequence;
    if (host.unit_is_local_player_role_00927f30(gun, 0)) {
        return;
    }
    if (host.unit_is_local_player_role_00927f30(gun, 1)) {
        return;
    }
    if (field<std::uint8_t>(message, 0x1D) == 0) {
        return;
    }
    field<std::int32_t>(gun, 0x49C) = field<std::int32_t>(message, 0x30);
    if (field<std::uint8_t>(message, 0x36) != 0) {
        field<float>(gun, 0x4C4) = field<float>(message, 0x28);
    }
    float a = field<float>(gun, 0x4C4);
    if (field<std::uint8_t>(message, 0x34) != 0) {
        field<float>(gun, 0x4BC) = field<float>(message, 0x20);
    }
    float c = field<float>(gun, 0x4BC);
    if (field<std::uint8_t>(message, 0x37) != 0) {
        field<float>(gun, 0x4C8) = field<float>(message, 0x2C);
    }
    float b = field<float>(gun, 0x4C8);
    if (field<std::uint8_t>(message, 0x35) != 0) {
        field<float>(gun, 0x4C0) = field<float>(message, 0x24);
    }
    float d = field<float>(gun, 0x4C0);
    host.turning_gun_set_angles_immediate_0085b0f0(gun, a, b, c, d);
}

// ---------------------------------------------------------------------------
// 0085C0F0 BSP_Matrix_SetLookAtCopy, __thiscall(ECX matrix; EDX eye; target*, up*), RET 8.
// EDX is a register input passed through untouched to 00B63F10; up is copied onto the stack
// by value (FLD/FSTP float). The 40h-byte temporary is then copied into the matrix (004134F0).
// ---------------------------------------------------------------------------
void matrix_set_look_at_copy_0085c0f0(Seg53Host& host, Object matrix, const float* eye,
                                      const float* target, const float* up) {
    alignas(4) std::uint8_t temporary[0x40];
    Object built = host.matrix_build_look_at_00b63f10(temporary, eye, target, up[0], up[1], up[2]);
    host.matrix_copy_4x4_004134f0(matrix, built);
}

// ---------------------------------------------------------------------------
// 0085C220 BSP_Matrix_SetFromEulerYXZCopy, __thiscall(ECX matrix; EDX angles), RET 0 -> matrix.
// EDX passes through to 0047C2A0 (ECX = temporary); then 004134F0 into the matrix.
// ---------------------------------------------------------------------------
Object matrix_set_from_euler_yxz_copy_0085c220(Seg53Host& host, Object matrix,
                                               const float* angles) {
    alignas(4) std::uint8_t temporary[0x40];
    Object built = host.matrix_build_from_euler_yxz_0047c2a0(temporary, angles);
    host.matrix_copy_4x4_004134f0(matrix, built);
    return matrix;
}

// ---------------------------------------------------------------------------
// 00860AD0 BSP_DebugDraw_ArcAroundAxis, __fastcall(ECX = centre float[3], EDX = axis float[3];
// float radius, float start, float sweep, rgba, segments), RET 14h. Builds a 4x4 frame with
// rows (1,0,0,0), (ay,az,ax,0), (ax,ay,az,0), (0,0,0,1) (MOVSS copies; [00D7A24C] = 1.0f),
// orthonormalizes it (0085DC80), puts centre in the translation row, and draws the arc
// (00860140 with ECX = frame, EDX = rgba).
// ---------------------------------------------------------------------------
void debug_draw_arc_around_axis_00860ad0(Seg53Host& host, const float* centre, const float* axis,
                                         float radius, float start, float sweep,
                                         std::uint32_t rgba, std::uint32_t segments) {
    float frame[16] = {1.0f,    0.0f,    0.0f,    0.0f,  //
                       axis[1], axis[2], axis[0], 0.0f,  //
                       axis[0], axis[1], axis[2], 0.0f,  //
                       0.0f,    0.0f,    0.0f,    1.0f};
    host.matrix_orthonormalize_basis_rows_0085dc80(frame);
    frame[12] = centre[0];
    frame[13] = centre[1];
    frame[14] = centre[2];
    host.debug_draw_arc_in_frame_00860140(frame, rgba, radius, start, sweep, segments);
}

// ---------------------------------------------------------------------------
// 00860BB0 BSP_DebugDraw_CircleAroundAxis, __fastcall(ECX = centre, EDX = axis passed through;
// float radius, rgba, segments), RET 0Ch: 00860AD0(radius, 0.0f, [00CE3D9C] = 2*pi (float),
// rgba, segments).
// ---------------------------------------------------------------------------
void debug_draw_circle_around_axis_00860bb0(Seg53Host& host, const float* centre,
                                            const float* axis, float radius, std::uint32_t rgba,
                                            std::uint32_t segments) {
    constexpr float kTwoPi_00ce3d9c = 6.28318548f;
    debug_draw_arc_around_axis_00860ad0(host, centre, axis, radius, 0.0f, kTwoPi_00ce3d9c, rgba,
                                        segments);
}

// ---------------------------------------------------------------------------
// 00861970..008619C0: __fastcall(ECX bridge), RET 0: AL = byte [[bridge+4]+offset] (the upper
// EAX bytes are the director pointer's). No rel32 or absolute reference in the PE (dead
// accessors, cc13-gp85).
// ---------------------------------------------------------------------------
namespace {
std::uint8_t director_byte(Object bridge, std::size_t offset) {
    return field<std::uint8_t>(field<Object>(bridge, 0x4), offset);
}
float artillery_row_float(Seg53Host& host, Object bot, std::size_t offset) {
    Object table = host.artillery_bot_config_00e19994();
    return field<float>(table, static_cast<std::size_t>(field<std::int32_t>(bot, 0x14)) * 0x28 +
                                   offset);
}
}  // namespace

std::uint8_t director_bridge_get_allow_fire_00861970(Object bridge) {
    return director_byte(bridge, 0x3C);
}
std::uint8_t director_bridge_get_torpedo_enabled_00861980(Object bridge) {
    return director_byte(bridge, 0x222);
}
std::uint8_t director_bridge_get_artillery_enabled_00861990(Object bridge) {
    return director_byte(bridge, 0x220);
}
std::uint8_t director_bridge_get_aa_enabled_008619a0(Object bridge) {
    return director_byte(bridge, 0x221);
}
std::uint8_t director_bridge_get_depth_charge_enabled_008619b0(Object bridge) {
    return director_byte(bridge, 0x223);
}
std::uint8_t director_bridge_get_allow_move_008619c0(Object bridge) {
    return director_byte(bridge, 0x3D);
}

// ---------------------------------------------------------------------------
// 00861A10..00861B10: __fastcall(ECX bot), RET 0, ST0 = float [[00E19994] + 28h*[bot+14h] + n]
// (LEA EAX,[EAX+EAX*4]; FLD [ECX+EAX*8+n]). Row fields per include/bsp/robot_config.hpp.
// ---------------------------------------------------------------------------
float artillery_bot_get_max_error_radius_00861a10(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x0C);
}
float artillery_bot_get_error_range_mul_00861a30(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x10);
}
float artillery_bot_get_min_error_mul_00861a50(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x14);
}
float artillery_bot_get_approach_mul_min_00861a70(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x18);
}
float artillery_bot_get_approach_mul_max_00861a90(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x1C);
}
float artillery_bot_get_deviation_mul_00861ab0(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x20);
}
float artillery_bot_get_angle_change_00861ad0(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x24);
}
float artillery_bot_get_error_dist_inc_start_time_00861af0(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x28);  // (index + 1) * 28h
}
float artillery_bot_get_error_dist_inc_full_time_00861b10(Seg53Host& host, Object bot) {
    return artillery_row_float(host, bot, 0x2C);
}

// ---------------------------------------------------------------------------
// 00861B30 BSP_GunneryTargetRecord_Destruct, __fastcall(ECX record): [record] = 00D0D2FC, then
// JMP 00695870 (the callback-owner base destructor on the same ECX).
// ---------------------------------------------------------------------------
void gunnery_target_record_destruct_00861b30(Seg53Host& host, Object record) {
    field<std::uint32_t>(record, 0x0) = 0x00D0D2FC;
    host.callback_owner_destroy_00695870(record);
}

// 00861B40 BSP_GunneryTargetRecord_OnObserveeDestroyed, __thiscall(ECX record; observee), RET 4:
// clears +1Ch when it is the destroyed observee.
void gunnery_target_record_on_observee_destroyed_00861b40(Object record, Object observee) {
    if (observee == field<Object>(record, 0x1C)) {
        field<Object>(record, 0x1C) = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 008626D0 BSP_GunneryTargetRecord_ResetAimOffset_Provisional, __fastcall(ECX record), RET 0.
// The listing (the decompiler misreads it): v = record+74h becomes (0, 0, [+70h]) (MOVSS),
// then 0085C3F0(ECX = v; axis = local (0, 1.0f, 0), angle = float [+6Ch]) rotates it in place.
// ---------------------------------------------------------------------------
void gunnery_target_record_reset_aim_offset_008626d0(Seg53Host& host, Object record) {
    float* v = &field<float>(record, 0x74);
    v[2] = field<float>(record, 0x70);
    v[0] = 0.0f;
    v[1] = 0.0f;
    const float axis[3] = {0.0f, 1.0f, 0.0f};
    host.vector3_rotate_about_axis_0085c3f0(v, axis, field<float>(record, 0x6C));
}

// 00863620 BSP_Entity_AsKind02OrNull, __fastcall(ECX entity) -> entity when non-null and
// vtable[5Ch](2) is true, else 0.
Object entity_as_kind02_or_null_00863620(Seg53Host& host, Object entity) {
    if (entity != nullptr && host.entity_is_kind_vslot_5c(entity, 2)) {
        return entity;
    }
    return nullptr;
}

// 00864510 BSP_UnitGunneryAi_VSlot60_NoOp: RET 4.
void unit_gunnery_ai_vslot60_noop_00864510(std::uint32_t) {}

// ---------------------------------------------------------------------------
// 008658C0 BSP_NodeHandle_Release, __fastcall(ECX = Node**): if set, 00B6DFA0 then clear
// (byte-identical to 008580C0).
// ---------------------------------------------------------------------------
void node_handle_release_008658c0(Seg53Host& host, Object* handle) {
    if (*handle != nullptr) {
        host.node_unlink_and_release_00b6dfa0(*handle);
        *handle = nullptr;
    }
}

// 00865E30 BSP_OwnedObject_DeleteIfSet_00865E30, __fastcall(ECX = Object**): if set,
// [object]->vtable[4](1) (delete) and clear.
void owned_object_delete_if_set_00865e30(Seg53Host& host, Object* owner) {
    if (*owner != nullptr) {
        host.object_delete_vslot_04(*owner, 1);
        *owner = nullptr;
    }
}

// ---------------------------------------------------------------------------
// 00866760 / 008669A0 the effect deletion lock / effect manager singleton holders' destructors,
// __fastcall(ECX holder): vtable 00D0D3D0 / 00D0D3CC, 0041CC80(ECX = holder+4), clear the
// singleton pointer [00F87654] / [00F87650], then the SingletonBase vtable 00CE3818.
// ---------------------------------------------------------------------------
void effect_deletion_lock_holder_dtor_00866760(Seg53Host& host, Object holder) {
    field<std::uint32_t>(holder, 0x0) = 0x00D0D3D0;
    host.critical_section_destroy_owned_0041cc80(holder + 4);
    host.set_global_dword(0x00F87654, 0);
    field<std::uint32_t>(holder, 0x0) = 0x00CE3818;
}

void effect_manager_holder_dtor_008669a0(Seg53Host& host, Object holder) {
    field<std::uint32_t>(holder, 0x0) = 0x00D0D3CC;
    host.critical_section_destroy_owned_0041cc80(holder + 4);
    host.set_global_dword(0x00F87650, 0);
    field<std::uint32_t>(holder, 0x0) = 0x00CE3818;
}

// ---------------------------------------------------------------------------
// 00869D70 BSP_EffectParticle_AdmitAtPosition, __thiscall(ECX system; float* position, radius),
// RET 8 -> AL. Walks the emitter pointers [+28h] x [+2Ch]: an emitter with byte +66h set
// admits at once; otherwise fade = (float)00AF4130(emitter; position by value, radius, scratch
// = the caller's position argument slot) and, when fade > 0 (FCOMIP/JBE: unordered rejects),
// 00AF3F80(emitter; position, radius) true admits. 0 when no emitter admits. The radius is
// passed through as the raw argument dword.
// ---------------------------------------------------------------------------
bool particle_admit_at_position_00869d70(Seg53Host& host, Object system, const float* position,
                                         std::uint32_t radius_bits) {
    Object* it = field<Object*>(system, 0x28);
    Object* end = it + field<std::int32_t>(system, 0x2C);
    for (; it != end; ++it) {
        Object emitter = *it;
        if (field<std::uint8_t>(emitter, 0x66) != 0) {
            return true;
        }
        const float* scratch_slot = position;  // [ESP+14h], overwritten by the callee
        float fade = host.particle_compute_lod_fade_00af4130(
            emitter, position[0], position[1], position[2], radius_bits, &scratch_slot);
        if (fade > 0.0f && host.particle_is_sphere_visible_00af3f80(
                               *it, position[0], position[1], position[2], radius_bits)) {
            return true;
        }
    }
    return false;
}

// 0086B7C0 BSP_EffectComponent_Slot0CFalse: XOR AL,AL; RET.
bool effect_component_slot0c_false_0086b7c0() { return false; }

// 0086B9A0 adjustor thunk: SUB ECX,80h; JMP 0086CF70 (stack flags passed through).
Object effect_tracer_adjustor_thunk_0086b9a0(Seg53Host& host, Object sub_object,
                                             std::uint32_t flags) {
    return host.scalar_deleting_dtor_0086cf70(sub_object - 0x80, flags);
}

// ---------------------------------------------------------------------------
// 0086B9B0 BSP_EffectTracer_Destruct, __fastcall(ECX tracer) under an SEH frame (state 0 ->
// -1): the component base at +80h (0 when the tracer is null: the compiler's base-conversion
// test) is destroyed by 0086B7E0, then the traceline payload by 0086AD50(ECX = tracer).
// ---------------------------------------------------------------------------
void effect_tracer_destruct_0086b9b0(Seg53Host& host, Object tracer) {
    Object component = tracer != nullptr ? tracer + 0x80 : nullptr;
    host.effect_component_destruct_0086b7e0(component);
    host.native_traceline_payload_destroy_0086ad50(tracer);
}

// ---------------------------------------------------------------------------
// 0086FC90 BSP_PooledStringListOwner_Construct_0086FC90, __fastcall(ECX owner) -> owner, under
// an SEH frame: +0 = +4 = 0; the list at +8 gets its sentinel (004C3020, ECX = owner+8) at +0Ch
// and size +10h = 0; then +24h, +20h, +1Ch, +18h, +14h = 0 in that order.
// ---------------------------------------------------------------------------
Object pooled_string_list_owner_construct_0086fc90(Seg53Host& host, Object owner) {
    field<std::uint32_t>(owner, 0x0) = 0;
    field<std::uint32_t>(owner, 0x4) = 0;
    Object sentinel = host.pooled_string_list_allocate_sentinel_004c3020(owner + 0x8);
    field<Object>(owner, 0xC) = sentinel;
    field<std::uint32_t>(owner, 0x10) = 0;
    field<std::uint32_t>(owner, 0x24) = 0;
    field<std::uint32_t>(owner, 0x20) = 0;
    field<std::uint32_t>(owner, 0x1C) = 0;
    field<std::uint32_t>(owner, 0x18) = 0;
    field<std::uint32_t>(owner, 0x14) = 0;
    return owner;
}

// 0086FD90 BSP_ParticleResourceRecord_DestroyIfSet, __fastcall(ECX records {data, count}):
// when count != 0, 0086FCF0(ECX = data + count*2Ch - 2Ch) (the last record) and count -= 1.
void particle_resource_record_destroy_if_set_0086fd90(Seg53Host& host, Object records) {
    std::int32_t count = field<std::int32_t>(records, 0x4);
    if (count != 0) {
        host.particle_resource_record_destroy_0086fcf0(field<Object>(records, 0x0) +
                                                       count * 0x2C - 0x2C);
        field<std::int32_t>(records, 0x4) += -1;
    }
}

// 00871A90 BSP_GameplayEffect_FindIdByName, __fastcall(ECX name) -> EAX:
// 00871750(ECX = 004C1650(), name).
std::uint32_t gameplay_effect_find_id_by_name_00871a90(Seg53Host& host, Object name) {
    Object manager = host.gameplay_effect_manager_get_singleton_004c1650();
    return host.gameplay_effect_name_index_find_id_00871750(manager, name);
}

// 00872080 BSP_RegisteredType4Effect_SetAlpha_Provisional, __thiscall(ECX effect; float), RET 4:
// with a tracer at +34h, 00BA9960(ECX = tracer, alpha) (FLD/FSTP float copy).
void registered_type4_effect_set_alpha_00872080(Seg53Host& host, Object effect, float alpha) {
    Object tracer = field<Object>(effect, 0x34);
    if (tracer != nullptr) {
        host.skinned_water_tracer_set_alpha_00ba9960(tracer, alpha);
    }
}

// 008721A0 BSP_EffectNode_IsNonpositive20, __fastcall(ECX node) -> EAX: COMISS 0, [+20h]; JC
// (0 < x or unordered) returns 0, else 1; so NaN returns 0, matching x <= 0.
std::uint32_t effect_node_is_nonpositive20_008721a0(Object node) {
    return field<float>(node, 0x20) <= 0.0f ? 1u : 0u;
}

// 00872770 BSP_RegisteredModelEffect_SetValue1DC, __thiscall(ECX effect; float), RET 4:
// [[effect+1Ch]+1DCh] = value (MOVSS).
void registered_model_effect_set_value1dc_00872770(Object effect, float value) {
    field<float>(field<Object>(effect, 0x1C), 0x1DC) = value;
}

// 00872E80 BSP_LightningStormOwner_Reset, __fastcall(ECX = Storm**): if set, 00BC04C0(ECX =
// storm), CRT free(storm), clear.
void lightning_storm_owner_reset_00872e80(Seg53Host& host, Object* owner) {
    Object storm = *owner;
    if (storm != nullptr) {
        host.lightning_storm_destroy_00bc04c0(storm);
        host.crt_free_00bf65ac(storm);
        *owner = nullptr;
    }
}

// 00857D60 BSP_TorpedoBoatClass_ReconCategory2: MOV EAX,2; RET (class vtable slot).
std::uint32_t torpedo_boat_class_recon_category_00857d60() { return 2; }

// ---------------------------------------------------------------------------
// 00857E20 BSP_TorpedoBoatClass_CreateEntity, __thiscall(ECX class; flag), RET 4 -> boat, under
// an SEH frame (state 0 around the constructor). operator new(1190h) (00BF55BE), CRT memset of
// the raw return with no null test (00BF79F0), 00857CD0(ECX = memory, flag) when non-null else
// 0, then 009553D0(ECX = boat, class) and boat+354h = class, neither null-tested.
// ---------------------------------------------------------------------------
Object torpedo_boat_class_create_entity_00857e20(Seg53Host& host, Object vehicle_class,
                                                 std::uint32_t flag) {
    Object memory = host.operator_new_00bf55be(0x1190);
    std::memset(memory, 0, 0x1190);
    Object boat = memory != nullptr ? host.torpedo_boat_construct_00857cd0(memory, flag) : nullptr;
    host.unit_set_vehicle_class_009553d0(boat, vehicle_class);
    field<Object>(boat, 0x354) = vehicle_class;
    return boat;
}

// 00855B90 BSP_TorpedoProjectile_ShowNode, __thiscall(ECX torpedo; unused), RET 4 (vtable
// 00D0C3E8 slot 68h): with a model at +464h, 00B6DA70(ECX = [model+0Ch], 1.0f, 0).
void torpedo_projectile_show_node_00855b90(Seg53Host& host, Object torpedo, std::uint32_t) {
    Object model = field<Object>(torpedo, 0x464);
    if (model != nullptr) {
        host.scene_node_set_visibility_00b6da70(field<Object>(model, 0xC), 1.0f, 0);
    }
}

// ---------------------------------------------------------------------------
// 00855BD0 BSP_TorpedoCreateMessage_ResolveTarget_Unreferenced, __fastcall(ECX message) -> EAX.
// Net id = word +128h; 0 for id 0; id < [00F89A10] (signed JGE) reads bank 0
// [[00F89A54] + (id - [00F89A0C])*10h + 0Ch], else bank 1 [[00F89AA8] + (id - [00F89A60])*10h
// + 0Ch]. No PE reference.
// ---------------------------------------------------------------------------
Object torpedo_create_message_resolve_target_00855bd0(Seg53Host& host, Object message) {
    std::int32_t id = field<std::uint16_t>(message, 0x128);
    if (id == 0) {
        return nullptr;
    }
    if (id < host.net_handle_split_00f89a10()) {
        std::int32_t row = id - host.net_bank0_base_00f89a0c();
        return field<Object>(host.net_bank0_records_00f89a54(), static_cast<std::size_t>(row << 4) + 0xC);
    }
    std::int32_t row = id - host.net_bank1_base_00f89a60();
    return field<Object>(host.net_bank1_records_00f89aa8(), static_cast<std::size_t>(row << 4) + 0xC);
}

// ---------------------------------------------------------------------------
// 008598B0 BSP_TurningGun_ReadRecentShot, __thiscall(ECX gun; out), RET 4. With an out and
// [00F876A8] - gun+4A0h (x87, unrounded) below double [00D7A348] = 0.25 (FCOMIP/JBE: unordered
// skips), copies the six floats +4A4h..+4B8h to out+78h..+8Ch and sets out+90h = 1.
// ---------------------------------------------------------------------------
void turning_gun_read_recent_shot_008598b0(Seg53Host& host, Object gun, Object out) {
    constexpr double kRecentShotWindow_00d7a348 = 0.25;
    if (out == nullptr) {
        return;
    }
    double age = static_cast<double>(host.mission_clock_00f876a8()) -
                 static_cast<double>(field<float>(gun, 0x4A0));
    if (kRecentShotWindow_00d7a348 > age) {
        for (std::size_t i = 0; i < 6; ++i) {
            field<float>(out, 0x78 + i * 4) = field<float>(gun, 0x4A4 + i * 4);
        }
        field<std::uint8_t>(out, 0x90) = 1;
    }
}

// ---------------------------------------------------------------------------
// 00859A20 BSP_TurningGun_WriteAimSnapshotFields, __thiscall(ECX gun; holder*, int count),
// RET 8, under an SEH frame. Returns when gun+3Ch is set and its vtable[5Ch](6) is true, or when
// count >= 5 (signed). Otherwise builds a stack T_GUN_TURN_SYNC (00859410) with record
// gun+4BCh, mode = (gun+1B0h != peer id) where peer id = [holder] ? [[holder]+20h] : 8, and
// (+494h, +498h, +488h, +48Ch, +49Ch); sends it to [holder] (00779FC0, ECX = gun), stamps the
// holder (00779F80), and the message's inlined destructor stores the base vtable 00CE4974.
// ---------------------------------------------------------------------------
void turning_gun_write_aim_snapshot_fields_00859a20(Seg53Host& host, Object gun, Object* holder,
                                                    std::int32_t count) {
    Object owner = field<Object>(gun, 0x3C);
    if (owner != nullptr && host.entity_is_kind_vslot_5c(owner, 6)) {
        return;
    }
    if (count >= 5) {
        return;
    }
    std::int32_t peer_id = *holder == nullptr ? 8 : field<std::int32_t>(*holder, 0x20);
    std::uint8_t mode = field<std::int32_t>(gun, 0x1B0) != peer_id ? 1 : 0;
    alignas(4) std::uint8_t message[0x44];
    Object built = gun_turn_sync_construct_00859410(
        host, message, gun + 0x4BC, mode, field<float>(gun, 0x494), field<float>(gun, 0x498),
        field<float>(gun, 0x488), field<float>(gun, 0x48C), field<std::int32_t>(gun, 0x49C));
    host.entity_send_message_to_peer_00779fc0(gun, *holder, built);
    host.net_entity_stamp_sync_tick_00779f80(gun, reinterpret_cast<Object>(holder));
    field<std::uint32_t>(message, 0x0) = 0x00CE4974;
}

// ---------------------------------------------------------------------------
// 00859B10 BSP_TurningGun_WriteSnapshotFields, __thiscall(ECX gun; message*, writer), RET 8.
// Fills an existing T_GUN_TURN_SYNC: +40h = gun+4BCh, +28h/+2Ch/+20h/+24h = +494h/+498h/
// +488h/+48Ch (MOVSS), +30h = +49Ch, wraps +20h, +24h, +28h, +2Ch in place; then +38h/+3Ch =
// (gun+370h, 0.0f) when the vector at +348h..+34Ch is non-null and non-empty (SAR count), else
// (-1.0f, -1.0f) ([00D7A260]); finally [message]->vtable[14h](writer).
// ---------------------------------------------------------------------------
void turning_gun_write_snapshot_fields_00859b10(Seg53Host& host, Object gun, Object message,
                                                Object writer) {
    constexpr float kNoTarget_00d7a260 = -1.0f;
    std::uint32_t begin = field<std::uint32_t>(gun, 0x348);
    bool has_targets = begin != 0 &&
                       ((static_cast<std::int32_t>(field<std::uint32_t>(gun, 0x34C) - begin)) >> 2) != 0;
    float time = has_targets ? field<float>(gun, 0x370) : 0.0f;
    field<Object>(message, 0x40) = gun + 0x4BC;
    field<float>(message, 0x28) = field<float>(gun, 0x494);
    field<float>(message, 0x2C) = field<float>(gun, 0x498);
    field<float>(message, 0x20) = field<float>(gun, 0x488);
    field<float>(message, 0x24) = field<float>(gun, 0x48C);
    field<std::int32_t>(message, 0x30) = field<std::int32_t>(gun, 0x49C);
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x20));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x24));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x28));
    host.math_wrap_angle_in_place_00605070(&field<float>(message, 0x2C));
    if (has_targets) {
        field<float>(message, 0x38) = time;
        field<float>(message, 0x3C) = 0.0f;
    } else {
        field<float>(message, 0x38) = kNoTarget_00d7a260;
        field<float>(message, 0x3C) = kNoTarget_00d7a260;
    }
    host.message_vslot_14(message, writer);
}

namespace {
// The platform array of 0085AB50 / 0085AD00: [[gun+3F0h]+538h]+94h = {data, count, ...};
// grown to index+1 (005471B0) when gun+38Ch >= count (signed JL), then data re-read.
Object gun_platform_slot(Seg53Host& host, Object gun) {
    Object array = field<Object>(field<Object>(gun, 0x3F0), 0x538) + 0x94;
    std::int32_t index = field<std::int32_t>(gun, 0x38C);
    if (index >= field<std::int32_t>(array, 0x4)) {
        host.ref_counted_pointer_array_resize_005471b0(array, index + 1);
    }
    return field<Object*>(array, 0x0)[index];
}
}  // namespace

// ---------------------------------------------------------------------------
// 0085AB50 BSP_TurningGun_FilterHeadingToTraverse, __thiscall(ECX gun; float heading, float
// limit), RET 8 -> ST0: 007F6190(ECX = this gun's platform slot, heading, limit) returned as is
// (FLT_MAX when no traverse window accepts it, per gun_bot_ticks.hpp).
// ---------------------------------------------------------------------------
double turning_gun_filter_heading_to_traverse_0085ab50(Seg53Host& host, Object gun, float heading,
                                                       float limit) {
    Object platform = gun_platform_slot(host, gun);
    return host.gun_platform_snap_heading_007f6190(platform, heading, limit);
}

// ---------------------------------------------------------------------------
// 0085AD00 BSP_TurningGun_AimToRestAngles, __fastcall(ECX gun). v = float [slot+94h]; FUCOMIP
// against double [00D7A278] = FLT_MAX with LAHF/TEST AH,44h/JNP: only an exact FLT_MAX skips
// (NaN calls); otherwise 0085ABA0(ECX = gun; v, float [slot+90h]).
// ---------------------------------------------------------------------------
void turning_gun_aim_to_rest_angles_0085ad00(Seg53Host& host, Object gun) {
    constexpr double kNoRestAngle_00d7a278 = 3.4028234663852886e+38;
    Object slot = gun_platform_slot(host, gun);
    float v = field<float>(slot, 0x94);
    if (static_cast<double>(v) != kNoRestAngle_00d7a278) {
        host.turning_gun_set_target_angles_0085aba0(gun, v, field<float>(slot, 0x90));
    }
}

// ---------------------------------------------------------------------------
// 0085B980 BSP_TurningGun_AimAtWorldDirection, __thiscall(ECX gun; const float* direction),
// RET 4. pose = gun+3Ch: when pose+10Ch is 0, 00414DB0(pose), pose+10Ch = 1, 00B63D50(ECX =
// pose+110h, EDX = pose+CCh). Local v = 0042D0D0(EDX = direction; pose+110h, 0), normalized in
// place (0042B260). s = clamp(v.y): double [00D7A250] = -1.0 when -1 > y (FCOMI/JA, ordered),
// else 1.0 when y > 1 (JBE keeps y, so NaN stays), else y; FSTP float. Then
// a = (float)0042CF10(s), h = (float)_CIatan2(v.x, v.z) (00BF701A; ST1 = x, ST0 = z), and
// 0085ABA0(ECX = gun; -h, a).
// ---------------------------------------------------------------------------
void turning_gun_aim_at_world_direction_0085b980(Seg53Host& host, Object gun,
                                                 const float* direction) {
    constexpr double kLower_00d7a250 = -1.0;
    Object pose = field<Object>(gun, 0x3C);
    if (field<std::uint8_t>(pose, 0x10C) == 0) {
        host.entity_pose_refresh_world_00414db0(pose);
        field<std::uint8_t>(pose, 0x10C) = 1;
        host.matrix_build_affine_inverse_00b63d50(pose + 0x110, pose + 0xCC);
    }
    float v[3];
    host.vector3_transform_direction_0042d0d0(v, direction, pose + 0x110, 0);
    host.geometry_normalize_with_floor_0042b260(v);
    double y = v[1];
    double clamped = kLower_00d7a250 > y ? kLower_00d7a250 : (y > 1.0 ? 1.0 : y);
    float s = static_cast<float>(clamped);
    float a = host.geometry_asin_clamped_0042cf10(s);
    float h = static_cast<float>(std::atan2(static_cast<double>(v[0]), static_cast<double>(v[2])));
    host.turning_gun_set_target_angles_0085aba0(gun, -h, a);
}

namespace {
TaggedValue tagged(std::uint32_t tag, const void* value) {
    return TaggedValue{tag, reinterpret_cast<std::uintptr_t>(value)};
}
}  // namespace

// ---------------------------------------------------------------------------
// 0085BB50 BSP_TurningGun_LoadInstanceState, __fastcall(ECX gun), SEH frame. 0072F060 first;
// then, when the entity record gun+C0h is set and record+4 == 3 (a saved game), the saved Lua
// data (009238A0) feeds a stack LuaReader (004425C0) that enters "_turningGun" (00D0CB94) and
// reads horzAngle (00D0CB88) -> +480h, vertAngle -> +484h, tHorzAngle -> +494h, tVertAngle
// -> +498h (tag 0Ah) and horzRotDir (00D0CB58) -> +49Ch (tag 1); leave (00BD7A20) and destroy
// (00441A20).
// ---------------------------------------------------------------------------
void turning_gun_load_instance_state_0085bb50(Seg53Host& host, Object gun) {
    host.gun_load_saved_state_0072f060(gun);
    Object record = field<Object>(gun, 0xC0);
    if (record == nullptr || field<std::int32_t>(record, 0x4) != 3) {
        return;
    }
    LuaObject data{};
    host.entity_get_saved_lua_data_009238a0(gun, &data);
    alignas(4) std::uint8_t reader[0x14]{};
    host.lua_reader_construct_004425c0(reader, &data);
    host.lua_reader_enter_key_00bd8e20(reader, tagged(0, "_turningGun"));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "horzAngle"), tagged(10, gun + 0x480));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "vertAngle"), tagged(10, gun + 0x484));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "tHorzAngle"), tagged(10, gun + 0x494));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "tVertAngle"), tagged(10, gun + 0x498));
    host.lua_reader_read_field_00bd6830(reader, tagged(0, "horzRotDir"), tagged(1, gun + 0x49C));
    host.lua_reader_leave_key_00bd7a20(reader);
    host.lua_reader_destroy_00441a20(reader);
}

// ---------------------------------------------------------------------------
// 0085A130 BSP_TurningGun_DebugDumpFields, __thiscall(ECX gun; writer), RET 4: 0072ADC0(gun,
// writer), then writer[4]({0, "_turningGun"}), writer[0Ch] for horzAngle/vertAngle/tHorzAngle/
// tVertAngle with {0Ah, the float's raw bits} (MOVSS copies of +480h/+484h/+494h/+498h) and
// horzRotDir {1, +49Ch}, then writer[8]().
// ---------------------------------------------------------------------------
void turning_gun_debug_dump_fields_0085a130(Seg53Host& host, Object gun, Object writer) {
    host.gun_debug_dump_fields_0072adc0(gun, writer);
    host.writer_open_vslot_04(writer, tagged(0, "_turningGun"));
    host.writer_field_vslot_0c(writer, tagged(0, "horzAngle"),
                               TaggedValue{10, field<std::uint32_t>(gun, 0x480)});
    host.writer_field_vslot_0c(writer, tagged(0, "vertAngle"),
                               TaggedValue{10, field<std::uint32_t>(gun, 0x484)});
    host.writer_field_vslot_0c(writer, tagged(0, "tHorzAngle"),
                               TaggedValue{10, field<std::uint32_t>(gun, 0x494)});
    host.writer_field_vslot_0c(writer, tagged(0, "tVertAngle"),
                               TaggedValue{10, field<std::uint32_t>(gun, 0x498)});
    host.writer_field_vslot_0c(writer, tagged(0, "horzRotDir"),
                               TaggedValue{1, field<std::uint32_t>(gun, 0x49C)});
    host.writer_close_vslot_08(writer);
}

// 008658E0 BSP_WorldAtmosphere_ReleaseNode, __fastcall(ECX descriptor): +38h is tested twice
// (CMP then TEST of the loaded copy), then 00B6DFA0(ECX = node) and +38h = 0.
void world_atmosphere_release_node_008658e0(Seg53Host& host, Object descriptor) {
    if (field<Object>(descriptor, 0x38) != nullptr) {
        Object node = field<Object>(descriptor, 0x38);
        if (node != nullptr) {
            host.node_unlink_and_release_00b6dfa0(node);
            field<Object>(descriptor, 0x38) = nullptr;
        }
    }
}

// ---------------------------------------------------------------------------
// 00865B80 BSP_RainDescriptor_Construct, __thiscall(ECX descriptor) -> descriptor, SEH frame.
// vtable 00D0D3AC; the NativeString at +4 {size, ptr} = {0, 0}; +34h = 80D0D0FFh (bytes FF D0
// D0 80); +38h = 0; resize the string to 0Ch (0041DD40(0Ch, 0)) and, with a buffer, memcpy
// "raindrop.tga" (00D0D3B0, size bytes) into it; then floats +30h = 15.0 [00CE5380], +2Ch =
// 20.0 [00CE3930], +18h = 0.95 [00CED9E0], +1Ch = 0.9 [00CE3860], +10h = 0.1 [00D7A2F0],
// +14h = 0.02 [00CE9BAC], +0Ch = 1400.0 [00CED560], +20h = 3.0 [00CE3854], +24h = -12.0
// [00CE781C], +28h = 3.0, and +34h = 80D0D0FFh again.
// ---------------------------------------------------------------------------
Object rain_descriptor_construct_00865b80(Seg53Host& host, Object descriptor) {
    constexpr std::uint32_t kRainColour = 0x80D0D0FFu;
    static const char kTexture_00d0d3b0[] = "raindrop.tga";
    field<std::uint32_t>(descriptor, 0x0) = 0x00D0D3AC;
    Object text = descriptor + 0x4;
    field<std::uint32_t>(text, 0x0) = 0;
    field<std::uint32_t>(text, 0x4) = 0;
    field<std::uint32_t>(text, 0x30) = kRainColour;
    field<std::uint32_t>(descriptor, 0x38) = 0;
    host.native_string_resize_0041dd40(text, 0xC, 0);
    if (field<std::uintptr_t>(text, 0x4) != 0) {
        std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(text, 0x4)), kTexture_00d0d3b0,
                    field<std::uint32_t>(text, 0x0));
    }
    field<float>(descriptor, 0x30) = 15.0f;
    field<float>(descriptor, 0x2C) = 20.0f;
    field<float>(descriptor, 0x18) = 0.949999988f;
    field<float>(descriptor, 0x1C) = 0.899999976f;
    field<float>(descriptor, 0x10) = 0.100000001f;
    field<float>(descriptor, 0x14) = 0.0199999996f;
    field<float>(descriptor, 0x0C) = 1400.0f;
    field<float>(descriptor, 0x20) = 3.0f;
    field<float>(descriptor, 0x24) = -12.0f;
    field<float>(descriptor, 0x28) = 3.0f;
    field<std::uint32_t>(descriptor, 0x34) = kRainColour;
    return descriptor;
}

// ---------------------------------------------------------------------------
// 00865CF0 BSP_RainDescriptor_GetSingleton() -> 00F875C0, SEH frame. Function-static guard
// bit 0 of [00F875FC]: on first use set it, construct (00865B80, ECX = 00F875C0) and register
// the destructor thunk 00CDEA50 with atexit (00BF6FF5).
// ---------------------------------------------------------------------------
Object rain_descriptor_get_singleton_00865cf0(Seg53Host& host) {
    std::uint32_t* guard = host.global_dword_ptr(0x00F875FC);
    if ((*guard & 1) == 0) {
        *guard |= 1;
        rain_descriptor_construct_00865b80(host, host.global_object(0x00F875C0));
        host.crt_atexit_00bf6ff5(0x00CDEA50);
    }
    return host.global_object(0x00F875C0);
}

// 008674A0 BSP_EffectManager_ClearLiveReferences, __fastcall(ECX manager): 004C9550(ECX =
// manager+10h, 0), then JMP 008671A0(ECX = manager).
void effect_manager_clear_live_references_008674a0(Seg53Host& host, Object manager) {
    host.live_effect_references_resize_004c9550(manager + 0x10, 0);
    host.effect_manager_flush_pending_deletes_008671a0(manager);
}

// 00869D20 BSP_Effects_ClearLuaTable(): 006B8AD0(ECX = [[[00E188A8]+1A08h]+4];
// "Effects=nil\n" (00D0D4AC), 0, 0, 2).
void effects_clear_lua_table_00869d20(Seg53Host& host) {
    host.lua_machine_run_string_006b8ad0(host.lua_machine_00e188a8_1a08_4(), "Effects=nil\n", 0, 0,
                                         2);
}

// 00861C20 BSP_FlagGridCC_SetCell, __thiscall(ECX grid; column, row, byte), RET 0Ch. Cells are
// bytes at grid + CCh + row*61h + column; a negative row (JL) writes the column in all 12 rows.
void flag_grid_set_cell_00861c20(Object grid, std::int32_t column, std::int32_t row,
                                 std::uint8_t value) {
    if (row >= 0) {
        grid[row * 0x61 + column + 0xCC] = value;
        return;
    }
    std::uint8_t* cell = grid + column + 0xCC;
    for (int i = 0; i < 12; ++i, cell += 0x61) {
        *cell = value;
    }
}

// ---------------------------------------------------------------------------
// 00861CD0 / 00861D20 / 00861D70 / 00861DC0, __thiscall(ECX ai; byte allowed), RET 4. Each walks
// a category list in .data while the entry is < 0Ch (signed; the first entry is tested before
// the loop): artillery [00E0A4F8] = {1,2,3,4,6} sets/clears bit 2 of word +80h+4*c; AA
// [00E0A510] = {1,5,6} bit 1; torpedo [00E0A520] = {7} and depth charge [00E0A528] = {8,9}
// store the whole word 3 or 0 and then the byte flag +77h / +78h = allowed. The lists are read
// through the image at call time.
// ---------------------------------------------------------------------------
namespace {
const std::int32_t* category_list(Seg53Host& host, std::uint32_t address) {
    return reinterpret_cast<const std::int32_t*>(host.global_dword_ptr(address));
}
void set_category_bit(Seg53Host& host, Object ai, std::uint32_t list_address, std::uint32_t bit,
                      bool allowed) {
    const std::int32_t* list = category_list(host, list_address);
    for (int i = 0; list[i] < 12; ++i) {
        std::uint32_t& word = field<std::uint32_t>(ai, 0x80 + list[i] * 4);
        if (allowed) {
            word |= bit;
        } else {
            word &= ~bit;
        }
    }
}
void store_category_words(Seg53Host& host, Object ai, std::uint32_t list_address, bool allowed) {
    const std::int32_t* list = category_list(host, list_address);
    for (int i = 0; list[i] < 12; ++i) {
        field<std::uint32_t>(ai, 0x80 + list[i] * 4) = allowed ? 3u : 0u;
    }
}
}  // namespace

void unit_gunnery_ai_set_artillery_group_allowed_00861cd0(Seg53Host& host, Object ai, std::uint8_t allowed) {
    set_category_bit(host, ai, 0x00E0A4F8, 2, allowed != 0);
}
void unit_gunnery_ai_set_aa_group_allowed_00861d20(Seg53Host& host, Object ai, std::uint8_t allowed) {
    set_category_bit(host, ai, 0x00E0A510, 1, allowed != 0);
}
void unit_gunnery_ai_set_torpedo_group_allowed_00861d70(Seg53Host& host, Object ai, std::uint8_t allowed) {
    store_category_words(host, ai, 0x00E0A520, allowed != 0);
    field<std::uint8_t>(ai, 0x77) = allowed;
}
void unit_gunnery_ai_set_depth_charge_group_allowed_00861dc0(Seg53Host& host, Object ai,
                                                             std::uint8_t allowed) {
    store_category_words(host, ai, 0x00E0A528, allowed != 0);
    field<std::uint8_t>(ai, 0x78) = allowed;
}

// ---------------------------------------------------------------------------
// 00863A80 BSP_UnitGunneryAi_CreateDirectorBridge, __fastcall(ECX ai). With an owner at +50h
// whose vtable[5Ch](2) is true, director = [owner->vtable[140h]()]->vtable[114h](); with a
// director, operator new(14h) (00BF681B) gets {ai, director, 0, five zero bytes +0Ch..+10h},
// +64h = the bridge (0 when the allocation failed), and 008624C0(ECX = bridge, 1) runs even
// for a null bridge.
// ---------------------------------------------------------------------------
void unit_gunnery_ai_create_director_bridge_00863a80(Seg53Host& host, Object ai) {
    Object owner = field<Object>(ai, 0x50);
    if (owner == nullptr || !host.entity_is_kind_vslot_5c(owner, 2)) {
        return;
    }
    Object director = host.object_vslot_114(host.entity_vslot_140(owner));
    if (director == nullptr) {
        return;
    }
    Object bridge = host.operator_new_00bf681b(0x14);
    if (bridge != nullptr) {
        field<Object>(bridge, 0x0) = ai;
        field<Object>(bridge, 0x4) = director;
        field<std::uint32_t>(bridge, 0x8) = 0;
        for (std::size_t i = 0xC; i <= 0x10; ++i) {
            field<std::uint8_t>(bridge, i) = 0;
        }
    }
    field<Object>(ai, 0x64) = bridge;
    host.director_bridge_apply_008624c0(bridge, 1);
}

// 0086B990 / 0086BC50 / 0086BD60 / 0086CFC0 / 0086D010 / 0086D060: the effect components'
// category slot, MOV EAX,n; RET (tracer 3, particle 1, shake and the three rumbles 6).
std::uint32_t effect_tracer_get_category_0086b990() { return 3; }
std::uint32_t effect_particle_get_category_0086bc50() { return 1; }
std::uint32_t effect_shake_get_category_0086bd60() { return 6; }
std::uint32_t effect_const_rumble_get_category_0086cfc0() { return 6; }
std::uint32_t effect_slope_rumble_get_category_0086d010() { return 6; }
std::uint32_t effect_square_rumble_get_category_0086d060() { return 6; }

// ---------------------------------------------------------------------------
// 0086CDB0 BSP_EffectNamedRecord14_CopyConstruct, __thiscall(ECX record; const source*), RET 4
// -> record. +0/+4 FLD/FSTP float copies; the NativeString at +8 {size, ptr} starts {0, 0} and,
// unless it is the source's own string (address compare), is resized to the source size
// (0041DD40(size, 1)) and, for a non-zero source size, memcpy'd (00BF7680) for the new size;
// +10h copied.
// ---------------------------------------------------------------------------
Object effect_named_record14_copy_construct_0086cdb0(Seg53Host& host, Object record,
                                                     Object source) {
    field<float>(record, 0x0) = field<float>(source, 0x0);
    Object text = record + 0x8;
    Object source_text = source + 0x8;
    field<float>(record, 0x4) = field<float>(source, 0x4);
    field<std::uint32_t>(text, 0x0) = 0;
    field<std::uint32_t>(text, 0x4) = 0;
    if (text != source_text) {
        host.native_string_resize_0041dd40(text, field<std::uint32_t>(source_text, 0x0), 1);
        if (field<std::uint32_t>(source_text, 0x0) != 0) {
            std::memcpy(reinterpret_cast<void*>(field<std::uintptr_t>(text, 0x4)),
                        reinterpret_cast<const void*>(field<std::uintptr_t>(source_text, 0x4)),
                        field<std::uint32_t>(text, 0x0));
        }
    }
    field<std::uint32_t>(record, 0x10) = field<std::uint32_t>(source, 0x10);
    return record;
}

// ---------------------------------------------------------------------------
// 0086E670 BSP_EffectNamedRecord14Array_Append, __thiscall(ECX array {data, count, capacity};
// const record*), RET 4, SEH frame. When count == capacity, reserve capacity*2 (at least 1,
// JG) via 0086CE10; the slot data + count*14h is copy-constructed (0086CDB0) when non-null
// (the placement-new test), and count += 1.
// ---------------------------------------------------------------------------
void effect_named_record14_array_append_0086e670(Seg53Host& host, Object array, Object source) {
    std::int32_t capacity = field<std::int32_t>(array, 0x8);
    if (field<std::int32_t>(array, 0x4) == capacity) {
        std::int32_t grown = capacity + capacity;
        if (!(grown > 1)) {
            grown = 1;
        }
        host.named_record14_array_reserve_0086ce10(array, grown);
    }
    Object slot = field<Object>(array, 0x0) + field<std::int32_t>(array, 0x4) * 0x14;
    if (slot != nullptr) {
        effect_named_record14_copy_construct_0086cdb0(host, slot, source);
    }
    field<std::int32_t>(array, 0x4) += 1;
}

// 0086F900 BSP_EffectNamedRecord14Array_Destroy, __fastcall(ECX array): 0086E6F0(0), then
// free(data) (00BF6989).
void effect_named_record14_array_destroy_0086f900(Seg53Host& host, Object array) {
    host.named_record14_array_resize_0086e6f0(array, 0);
    host.crt_free_00bf6989(field<Object>(array, 0x0));
}

// ---------------------------------------------------------------------------
// 00871390 BSP_ParticleResourceRecords_EraseUnordered, __thiscall(ECX array {data, count};
// iterator*), RET 4. Unless *it is the last record (data + (count-1)*2Ch), the last record is
// assigned over it (00870C10, ECX = *it); then the last record (re-read) is destroyed (0086FCF0)
// and count -= 1.
// ---------------------------------------------------------------------------
void particle_resource_records_erase_unordered_00871390(Seg53Host& host, Object array,
                                                        const Object* it) {
    std::int32_t count = field<std::int32_t>(array, 0x4);
    Object last = field<Object>(array, 0x0) + (count - 1) * 0x2C;
    if (*it != last) {
        host.particle_resource_record_assign_00870c10(*it, field<Object>(array, 0x0) + count * 0x2C - 0x2C);
    }
    host.particle_resource_record_destroy_0086fcf0(field<Object>(array, 0x0) +
                                                   field<std::int32_t>(array, 0x4) * 0x2C - 0x2C);
    field<std::int32_t>(array, 0x4) += -1;
}

// 00871FF0 BSP_RegisteredModelEffect_SetEmitterValue, __thiscall(ECX effect; a, float), RET 8:
// 00AF6790(ECX = [effect+1Ch]; a, value) (FLD/FSTP float copy).
void registered_model_effect_set_emitter_value_00871ff0(Seg53Host& host, Object effect,
                                                        std::uint32_t a, float value) {
    host.particle_model_set_emitter_value_00af6790(field<Object>(effect, 0x1C), a, value);
}

// ---------------------------------------------------------------------------
// 0085BCE0 BSP_ReconCategory_IndexFromName, __fastcall(ECX = NativeString {size, ptr}) -> index
// 0..18, or 13h on no match. Per table entry: a null string ptr matches a null or empty entry;
// with a string, a null entry matches size 0 and otherwise CRT _stricmp (00BF7FBF) == 0.
// ---------------------------------------------------------------------------
std::int32_t recon_category_index_from_name_0085bce0(Seg53Host& host, Object name) {
    for (std::int32_t index = 0; index < 0x13; ++index) {
        const char* text = reinterpret_cast<const char*>(field<std::uintptr_t>(name, 0x4));
        const char* entry = host.recon_category_name_00e0b590(index);
        if (text == nullptr) {
            if (entry == nullptr || std::strlen(entry) == 0) {
                return index;
            }
        } else if (entry == nullptr) {
            if (field<std::uint32_t>(name, 0x0) == 0) {
                return index;
            }
        } else if (_stricmp(text, entry) == 0) {
            return index;
        }
    }
    return 0x13;
}

// ---------------------------------------------------------------------------
// 0085BD50 BSP_ReconCategory_FromClassTypeName_Unreferenced, __fastcall(ECX = NativeString).
// "Destroyer" is the inlined compare (only with a non-null ptr, CRT _stricmp); the rest go
// through 00425850 in this order: Cruiser 4, LandingShip 6, Cargo 5, BattleShip 3, Submarine
// 0Dh, TorpedoBoat 2, MotherShip 0, ReconPlane / SmallReconPlane / LargeReconPlane 0Bh, Fighter
// 0Ah, DiveBomber 8, TorpedoBomber 9, Kamikaze 0Ch, LevelBomber 7, AirField 10h, Shipyard 11h,
// LandVehicle 0Eh, LandFort 0Fh, else 13h. No PE reference.
// ---------------------------------------------------------------------------
std::int32_t recon_category_from_class_type_name_0085bd50(Seg53Host& host, Object name) {
    const char* text = reinterpret_cast<const char*>(field<std::uintptr_t>(name, 0x4));
    if (text != nullptr && _stricmp(text, "Destroyer") == 0) {
        return 1;
    }
    static constexpr struct {
        const char* name;
        std::int32_t category;
    } kChain[] = {
        {"Cruiser", 4},       {"LandingShip", 6},     {"Cargo", 5},           {"BattleShip", 3},
        {"Submarine", 0xD},   {"TorpedoBoat", 2},     {"MotherShip", 0},      {"ReconPlane", 0xB},
        {"SmallReconPlane", 0xB}, {"LargeReconPlane", 0xB}, {"Fighter", 0xA}, {"DiveBomber", 8},
        {"TorpedoBomber", 9}, {"Kamikaze", 0xC},      {"LevelBomber", 7},     {"AirField", 0x10},
        {"Shipyard", 0x11},   {"LandVehicle", 0xE},   {"LandFort", 0xF},
    };
    for (const auto& entry : kChain) {
        if (host.native_string_equals_insensitive_00425850(name, entry.name)) {
            return entry.category;
        }
    }
    return 0x13;
}

// ---------------------------------------------------------------------------
// The RefPtr<T> instantiations, __fastcall(ECX = T**): with a referent p, InterlockedDecrement
// (p+4) ([00CE2220]) and, at zero, [p]->vtable[0](ECX = p); then *ref = 0 (only when p was set:
// the JZ skips the store). Destruct (6 copies) returns ref in EAX; release (4 copies) does not.
// 0086AFC0, __thiscall(ECX ref; raw), RET 4 -> ref: the same release, then *ref = 0 and
// *ref = raw (both stores; with no referent only the second), no AddRef of raw.
// ---------------------------------------------------------------------------
namespace {
void ref_ptr_drop(Seg53Host& host, Object referent) {
    if (host.interlocked_decrement_00ce2220(&field<std::int32_t>(referent, 0x4)) == 0) {
        host.refcounted_vslot_00(referent);
    }
}
}  // namespace

Object ref_ptr_destruct(Seg53Host& host, Object* ref) {
    Object referent = *ref;
    if (referent != nullptr) {
        ref_ptr_drop(host, referent);
        *ref = nullptr;
    }
    return reinterpret_cast<Object>(ref);
}

void ref_ptr_release(Seg53Host& host, Object* ref) {
    Object referent = *ref;
    if (referent != nullptr) {
        ref_ptr_drop(host, referent);
        *ref = nullptr;
    }
}

Object* ref_ptr_attach_0086afc0(Seg53Host& host, Object* ref, Object raw) {
    Object referent = *ref;
    if (referent != nullptr) {
        ref_ptr_drop(host, referent);
        *ref = nullptr;
    }
    *ref = raw;
    return ref;
}

// 00872220 / 00872240 / 00872260 BSP_NodeRef_Release, __fastcall(ECX = Node**): byte-for-byte
// the 008658C0 body.
void node_ref_release(Seg53Host& host, Object* handle) {
    node_handle_release_008658c0(host, handle);
}

// ---------------------------------------------------------------------------
// 0085A9A0 BSP_Gun_TargetInFireWindow, __thiscall(ECX gun; target, float direction[3], float*
// yaw, float* pitch), RET 10h -> AL of 007F60A0.
//  * The platform slot address &data[gun+38Ch] is taken after the 005471B0 grow test; the
//    platform pointer itself is read from it only at the final call.
//  * mount = gun+3CCh: 00B6DB70 unless bit 1 of byte mount+5Ch; position = mount+120h..128h.
//  * target: 00414DB0 unless byte target+C8h; d = (float)(target+FCh.. - position) (x87 FSUB
//    float, FSTP float), copied to direction.
//  * pose = gun+3Ch: the 0085B980 refresh; 0042D0D0(out = the local d, EDX = direction;
//    pose+110h, 0) copied back into direction, normalized in place (0042B260).
//  * 00521370(ECX = direction, EDX = pitch; yaw).
//  * y = *yaw: FCOMI y, pi; JC (below or unordered) skips, so only an ordered y >= pi gets
//    *yaw = (float)(y - 2pi); p = *pitch: FCOMIP -pi, p; JC skips, so only an ordered
//    p <= -pi gets *pitch = (float)(p + 2pi). The constants are
//    the doubles [00CE3D28] = pi, [00CE3828] = 2pi, [00CE3D18] = -pi.
//  * 007F60A0(ECX = platform; horz = -0.0f - *yaw (SUBSS), vert = *pitch).
// ---------------------------------------------------------------------------
bool gun_target_in_fire_window_0085a9a0(Seg53Host& host, Object gun, Object target,
                                        float* direction, float* yaw, float* pitch) {
    constexpr double kPi_00ce3d28 = 3.1415927410125732;
    constexpr double kTwoPi_00ce3828 = 6.2831854820251465;
    constexpr double kNegPi_00ce3d18 = -3.1415927410125732;
    Object array = field<Object>(field<Object>(gun, 0x3F0), 0x538) + 0x94;
    std::int32_t index = field<std::int32_t>(gun, 0x38C);
    if (index >= field<std::int32_t>(array, 0x4)) {
        host.ref_counted_pointer_array_resize_005471b0(array, index + 1);
    }
    Object* platform_slot = field<Object*>(array, 0x0) + index;
    Object mount = field<Object>(gun, 0x3CC);
    if ((field<std::uint8_t>(mount, 0x5C) & 2) == 0) {
        host.transform_refresh_world_matrix_00b6db70(mount);
    }
    const float position[3] = {field<float>(mount, 0x120), field<float>(mount, 0x124),
                               field<float>(mount, 0x128)};
    if (field<std::uint8_t>(target, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(target);
    }
    float d[3] = {field<float>(target, 0xFC) - position[0], field<float>(target, 0x100) - position[1],
                  field<float>(target, 0x104) - position[2]};
    direction[0] = d[0];
    direction[1] = d[1];
    direction[2] = d[2];
    Object pose = field<Object>(gun, 0x3C);
    if (field<std::uint8_t>(pose, 0x10C) == 0) {
        host.entity_pose_refresh_world_00414db0(pose);
        field<std::uint8_t>(pose, 0x10C) = 1;
        host.matrix_build_affine_inverse_00b63d50(pose + 0x110, pose + 0xCC);
    }
    const float* local = host.vector3_transform_direction_0042d0d0(d, direction, pose + 0x110, 0);
    direction[0] = local[0];
    direction[1] = local[1];
    direction[2] = local[2];
    host.geometry_normalize_with_floor_0042b260(direction);
    host.direction_to_pitch_yaw_00521370(direction, pitch, yaw);
    double y = *yaw;
    if (y >= kPi_00ce3d28) {
        *yaw = static_cast<float>(y - kTwoPi_00ce3828);
    }
    double p = *pitch;
    if (p <= kNegPi_00ce3d18) {
        *pitch = static_cast<float>(p + kTwoPi_00ce3828);
    }
    float horz = kNegZero_00d7a208 - *yaw;
    return host.gun_platform_angles_in_fire_window_007f60a0(*platform_slot, horz, *pitch);
}

namespace {
// One channel: FLD float, FMUL double 255.0 [00CE4B48], FISTP under a chop control word (OR
// 0C00h), low byte kept. An out-of-range product is the integer indefinite 80000000h (byte 0).
std::uint32_t colour_byte(float channel) {
    double scaled = static_cast<double>(channel) * 255.0;
    std::int32_t value = (scaled >= -2147483648.0 && scaled < 2147483648.0)
                             ? static_cast<std::int32_t>(scaled)
                             : static_cast<std::int32_t>(0x80000000u);
    return static_cast<std::uint32_t>(value) & 0xFFu;
}
std::uint32_t argb(float r, float g, float b, float a) {
    return (colour_byte(a) << 24) | (colour_byte(r) << 16) | (colour_byte(g) << 8) | colour_byte(b);
}
}  // namespace

// ---------------------------------------------------------------------------
// 0085F990 BSP_DebugDraw_LineRgbaFloats, __fastcall(ECX = p0, EDX = p1; r0, g0, b0, a0, r1, g1,
// b1, a1 floats), RET 20h. Each channel is chopped to a byte (above) and packed ARGB (r -> byte
// 2, g -> 1, b -> 0, a -> 3); then [[00F8D394]]->vtable[B4h](p0, p1, c0, c1).
// ---------------------------------------------------------------------------
void debug_draw_line_rgba_floats_0085f990(Seg53Host& host, const float* p0, const float* p1,
                                          float r0, float g0, float b0, float a0, float r1,
                                          float g1, float b1, float a1) {
    std::uint32_t c0 = argb(r0, g0, b0, a0);
    std::uint32_t c1 = argb(r1, g1, b1, a1);
    host.debug_renderer_line_vslot_b4(p0, p1, c0, c1);
}

// 0085FB30 BSP_DebugDraw_LineRgbaFloatsOneColour, __fastcall(ECX = p0, EDX = p1 passed
// through; r, g, b, a), RET 10h: 0085F990 with the four dwords pushed twice.
void debug_draw_line_rgba_floats_one_colour_0085fb30(Seg53Host& host, const float* p0,
                                                     const float* p1, float r, float g, float b,
                                                     float a) {
    debug_draw_line_rgba_floats_0085f990(host, p0, p1, r, g, b, a, r, g, b, a);
}

// ---------------------------------------------------------------------------
// 00860030 BSP_DebugDraw_ScreenLineRgbaFloats, __thiscall(ECX = float rgba[4]; x0, y0, x1, y1),
// RET 10h. One ARGB colour from rgba (the 0085F990 conversion); the two screen points are
// copied to locals (MOVSS) and [[00F8D394]]->vtable[C0h](&(x0, y0), &(x1, y1), c, c).
// ---------------------------------------------------------------------------
void debug_draw_screen_line_rgba_floats_00860030(Seg53Host& host, const float* rgba, float x0,
                                                 float y0, float x1, float y1) {
    std::uint32_t colour = argb(rgba[0], rgba[1], rgba[2], rgba[3]);
    const float second[2] = {x1, y1};
    const float first[2] = {x0, y0};
    host.debug_renderer_screen_line_vslot_c0(first, second, colour, colour);
}

}  // namespace bsp::cc13_seg53