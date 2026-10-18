// cc13_segmix: undone game bodies from segments 51, 58, 30, 41, 42, 50, 61, 54
// and 46 with no cc9 host (packet cc13-seg40F segmix, docs/CC13_SEGMIX.md). Names
// are hypotheses; see the header for the hooks. Each body was transcribed from
// the disk-byte disassembly (bsp.py disasm-raw).

#include "bsp/cc13_segmix.hpp"

#include <cstring>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(base) + offset);
}

// FLD float / FSTP float: exact for every value except a signalling NaN, which the
// x87 load quiets (sets mantissa bit 22).
float x87_float_round_trip(float value) {
    std::uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7F800000u) == 0x7F800000u && (bits & 0x007FFFFFu) != 0) bits |= 0x00400000u;
    std::memcpy(&value, &bits, sizeof(bits));
    return value;
}

// The stock snapshot's zeroed fields, shared by 007592A0 and 00766EF0: vtable
// 00CF8EB8 at +12Ch, then the two vectors +138h..+140h and +148h..+150h.
void mothership_create_init_snapshot(void* message) {
    at<std::uint32_t>(message, 0x12C) = 0x00CF8EB8u;
    at<std::uint32_t>(message, 0x138) = 0;
    at<std::uint32_t>(message, 0x13C) = 0;
    at<std::uint32_t>(message, 0x140) = 0;
    at<std::uint32_t>(message, 0x148) = 0;
    at<std::uint32_t>(message, 0x14C) = 0;
    at<std::uint32_t>(message, 0x150) = 0;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00759020 BSP_SessionMessage_MOTHERSHIP_CREATE_Destruct. __thiscall(ECX =
// message). Bytes 00759020..00759084: vtable 00D0189C, +12Ch = 00CF8EB8; the
// +148h vector's block is freed when non-null and +148h/+14Ch/+150h are zeroed,
// then the same for +138h; then JMP 0080E380 (the SHIP_CREATE destructor).
// ---------------------------------------------------------------------------
void cc13_segmix_mothership_create_destruct_00759020(void* message, Cc13SegmixHost& host) {
    at<std::uint32_t>(message, 0x0) = 0x00D0189Cu;
    at<std::uint32_t>(message, 0x12C) = 0x00CF8EB8u;
    void* const second = at<void*>(message, 0x148);
    if (second != nullptr) host.crt_free_00bf65ac(second);
    at<std::uint32_t>(message, 0x148) = 0;
    at<std::uint32_t>(message, 0x14C) = 0;
    at<std::uint32_t>(message, 0x150) = 0;
    void* const first = at<void*>(message, 0x138);
    if (first != nullptr) host.crt_free_00bf65ac(first);
    at<std::uint32_t>(message, 0x138) = 0;
    at<std::uint32_t>(message, 0x13C) = 0;
    at<std::uint32_t>(message, 0x140) = 0;
    host.ship_create_destruct_0080e380(message);                                       // 00759084 JMP
}

// ---------------------------------------------------------------------------
// 007590E0 BSP_SessionMessage_MOTHERSHIP_CREATE_Write. __thiscall(ECX = message;
// writer), RET 4. 00813B10(message, writer), then 006D44F0(ECX = message+12Ch,
// writer).
// ---------------------------------------------------------------------------
void cc13_segmix_mothership_create_write_007590e0(void* message, void* writer, Cc13SegmixHost& host) {
    host.ship_create_write_00813b10(message, writer);
    host.air_ops_stock_snapshot_write_006d44f0(static_cast<std::uint8_t*>(message) + 0x12C, writer);
}

// ---------------------------------------------------------------------------
// 007592A0 BSP_SessionMessage_MOTHERSHIP_CREATE_Construct. __thiscall(ECX =
// message; word, ship), RET 8; EAX = message. 00813A20(word, ship, 8Bh), vtable
// 00D0189C, the snapshot fields, then 006CC800(ECX = message+12Ch, ship ?
// ship+1188h : 0), the ship's air-ops block.
// ---------------------------------------------------------------------------
void* cc13_segmix_mothership_create_construct_007592a0(void* message, std::uint32_t word, void* ship,
                                                       Cc13SegmixHost& host) {
    host.ship_create_construct_00813a20(message, word, ship, 0x8B);                    // 007592CF
    at<std::uint32_t>(message, 0x0) = 0x00D0189Cu;
    mothership_create_init_snapshot(message);
    void* const block = ship != nullptr ? static_cast<std::uint8_t*>(ship) + 0x1188 : nullptr;
    host.air_ops_stock_snapshot_capture_006cc800(static_cast<std::uint8_t*>(message) + 0x12C, block);  // 0075930E
    return message;
}

// ---------------------------------------------------------------------------
// 00759330 BSP_MotherShip_SendCreateMessage. __thiscall(ECX = ship; peer), RET 4.
// A 154h-byte MOTHERSHIP_CREATE on the stack: 007592A0(word [[ship+538h]+70h],
// ship), 00779FC0(ECX = ship; peer, message), then 00759020.
// ---------------------------------------------------------------------------
void cc13_segmix_mothership_send_create_message_00759330(void* ship, std::uint32_t peer, Cc13SegmixHost& host) {
    alignas(4) std::uint8_t message[0x154];
    const std::uint32_t word = at<std::uint32_t>(at<void*>(ship, 0x538), 0x70);
    cc13_segmix_mothership_create_construct_007592a0(message, word, ship, host);
    host.entity_send_message_to_peer_00779fc0(ship, peer, message);                    // 00759378
    cc13_segmix_mothership_create_destruct_00759020(message, host);                    // 0075938C
}

// ---------------------------------------------------------------------------
// 007593D0 BSP_MotherShip_InitAll. __thiscall(ECX = ship), RET. Bytes
// 007593D0..00759583. The class is ship+538h, and the air-ops block is ship+1188h.
//   00822C20(ship), the unit init;
//   ship+1284h = 0071AD50(model, "elevator_2" 00D01890), model = [[ship+360h]+160h]
//     or null; then 004134F0(ECX = ship+1288h, 00B6DB60(ECX = node));
//   006C0D20(ECX = block; &class+814h, class+820h, class+824h) (FLD/FSTP copies);
//   006CADD0(ECX = block; [ship+C0h]);
//   block+3Ch = 006D0A10(ship) (006BA690 inlined: MOV [ECX+3Ch], arg);
//   when [ship+C0h] is set and its +4h is 3: a LuaReader on the stack built from
//     009238A0's saved entity data (004425C0), "_airBase" (00CF88BC) entered with
//     kind 0 (00BD8E20), 006BEF30(block, reader), left (00BD7A20); "_motherShip"
//     (00D01884) entered and left with nothing read; then 00441A20;
//   006BA670(ECX = block; 0.0f); ship+11A8h = 1.
// The node is not null-checked before 00B6DB60, as in the image.
// ---------------------------------------------------------------------------
void cc13_segmix_mothership_init_all_007593d0(void* ship, Cc13SegmixHost& host) {
    host.unit_instance_sentity_init_00822c20(ship);                                    // 007593EC
    void* const ship_class = at<void*>(ship, 0x538);
    float point[3] = {at<float>(ship_class, 0x814), at<float>(ship_class, 0x818), at<float>(ship_class, 0x81C)};
    const float value_820 = at<float>(ship_class, 0x820);
    const float value_824 = at<float>(ship_class, 0x824);
    void* const holder = at<void*>(ship, 0x360);
    void* const model = holder != nullptr ? at<void*>(holder, 0x160) : nullptr;
    void* const node = host.model_find_note_node_0071ad50(model, "elevator_2");         // 00759456
    at<void*>(ship, 0x1284) = node;
    host.matrix_copy_4x4_004134f0(static_cast<std::uint8_t*>(ship) + 0x1288, host.node_get_local_matrix_00b6db60(node));
    void* const block = static_cast<std::uint8_t*>(ship) + 0x1188;
    host.air_ops_block_create_holder_006c0d20(block, point, x87_float_round_trip(value_820),
                                              x87_float_round_trip(value_824));          // 00759493
    host.air_ops_load_from_scene_006cadd0(block, at<void*>(ship, 0xC0));               // 007594A1
    at<void*>(block, 0x3C) = host.mothership_site_create_006d0a10(ship);              // 007594A8 / 007594B0
    void* const properties = at<void*>(ship, 0xC0);
    if (properties != nullptr && at<std::int32_t>(properties, 0x4) == 3) {
        alignas(4) std::uint8_t saved[0x14];                                           // the by-value argument
        alignas(4) std::uint8_t reader[0x14];                                          // [esp+1Ch]
        host.saved_entity_lua_data_009238a0(ship, saved);                              // 007594D9
        host.lua_reader_construct_004425c0(reader, saved);                             // 007594E2
        host.lua_reader_enter_key_00bd8e20(reader, 0, "_airBase");                     // 00759509
        host.air_ops_block_read_traffic_table_006bef30(block, reader);
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_enter_key_00bd8e20(reader, 0, "_motherShip");                  // 0075953D
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_destroy_00441a20(reader);                                      // 00759557
    }
    host.air_ops_block_tick_006ba670(block, 0.0f);                                     // 00759564
    at<std::uint32_t>(ship, 0x11A8) = 1;
}

// ---------------------------------------------------------------------------
// 00759920 BSP_SessionMessage_LANDINGSHIP_CREATE_ConstructForRead. __fastcall(ECX
// = message); EAX = message. 0080E2C0, words +12Ch = +12Eh = 0, vtable 00CFF8E8.
// +130h is not written.
// ---------------------------------------------------------------------------
void* cc13_segmix_landingship_create_construct_for_read_00759920(void* message, Cc13SegmixHost& host) {
    host.ship_create_construct_for_read_0080e2c0(message);
    at<std::uint16_t>(message, 0x12C) = 0;
    at<std::uint16_t>(message, 0x12E) = 0;
    at<std::uint32_t>(message, 0x0) = 0x00CFF8E8u;
    return message;
}

// ---------------------------------------------------------------------------
// 00766EF0 BSP_SessionMessage_MOTHERSHIP_CREATE_ConstructForRead. __fastcall(ECX
// = message); EAX = message. 0080E2C0, vtable 00D0189C, then the snapshot fields
// as 007592A0 sets them. No capture.
// ---------------------------------------------------------------------------
void* cc13_segmix_mothership_create_construct_for_read_00766ef0(void* message, Cc13SegmixHost& host) {
    host.ship_create_construct_for_read_0080e2c0(message);
    at<std::uint32_t>(message, 0x0) = 0x00D0189Cu;
    mothership_create_init_snapshot(message);
    return message;
}

// ---------------------------------------------------------------------------
// 0093A2C0 BSP_SessionMessage9B_Construct (cc13-segmixC). __thiscall(ECX =
// message; dword a, dword b, byte c), RET 0Ch; EAX = message. Bytes
// 0093A2C0..0093A2FE: 0075B430(9Bh); +18h word = 0, +1Ah = +1Ch bytes = 0;
// +20h = a; +4h = 1 (delivery); vtable 00D03400; +24h = b; byte +28h = c.
// cc9 counts the send as "Failure::section_state_message_0093a2c0" and
// unit_message_arms lists 9Bh as a script/AI producer.
// ---------------------------------------------------------------------------
void* cc13_segmix_failure_message9b_construct_0093a2c0(void* message, std::uint32_t value_20, std::uint32_t value_24,
                                                       std::uint8_t flag_28, Cc13SegmixHost& host) {
    host.session_message_construct_0075b430(message, 0x9B);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x20) = value_20;
    at<std::uint32_t>(message, 0x4) = 1;
    at<std::uint32_t>(message, 0x0) = 0x00D03400u;
    at<std::uint32_t>(message, 0x24) = value_24;
    at<std::uint8_t>(message, 0x28) = flag_28;
    return message;
}

// ---------------------------------------------------------------------------
// 0093B920 BSP_RepairTask_Destruct (cc13-segmixD). __thiscall(ECX = task), RET.
// Bytes 0093B920..0093BA77. The unit's +A20h task, which 0081F72F destroys
// (unit_destructor_levels.hpp `l5_destroy_repair_task_a20`).
// - For i in 0..13h it reads slot i of the effect vector {+8h first, +0Ch last}.
//   Every read is checked (00BF6713 when the vector is null or i >= size).
//   When the slot is set, it runs 00867B10(effect), sets byte effect+9h = 1,
//   releases the reference (+4h count, slot 0 at zero) and zeroes the slot.
//   The image zeroes the slot twice.
// - The +14h vector {+18h, +1Ch, +20h}, when its first is set: 0093B300(first,
//   last; +14h, task), free, then zeroed.
// - The +4h vector {+8h, +0Ch, +10h} the same way through 0081C940.
// ---------------------------------------------------------------------------
void cc13_segmix_repair_task_destruct_0093b920(void* task, Cc13SegmixHost& host) {
    auto* const base = static_cast<std::uint8_t*>(task);
    const auto check = [&](std::int32_t i) {
        void** const first = at<void**>(task, 0x8);
        if (first == nullptr ||
            static_cast<std::uint32_t>(i) >= static_cast<std::uint32_t>(at<void**>(task, 0xC) - first)) {
            host.crt_invalid_parameter_noinfo_00bf6713();
        }
    };
    for (std::int32_t i = 0; i < 0x14; ++i) {
        check(i);                                                                       // 0093B950
        if (at<void**>(task, 0x8)[i] == nullptr) continue;
        check(i);
        void* const effect = at<void**>(task, 0x8)[i];
        host.point_effect_stop_children_00867b10(effect);                              // 0093B9A0
        at<std::uint8_t>(effect, 0x9) = 1;
        check(i);
        void** const slot = at<void**>(task, 0x8) + i;
        void* const reference = *slot;
        if (reference != nullptr) {
            if (host.interlocked_decrement(&at<std::int32_t>(reference, 4)) == 0) host.vcall_00_release(reference);
            *slot = nullptr;
        }
        *slot = nullptr;
    }
    if (at<void*>(task, 0x18) != nullptr) {
        host.stl_destroy_range_0093b300(at<void*>(task, 0x18), at<void*>(task, 0x1C), base + 0x14, task);
        host.crt_free_00bf65ac(at<void*>(task, 0x18));
    }
    at<std::uint32_t>(task, 0x18) = 0;
    at<std::uint32_t>(task, 0x1C) = 0;
    at<std::uint32_t>(task, 0x20) = 0;
    if (at<void*>(task, 0x8) != nullptr) {
        host.stl_destroy_range_0081c940(at<void*>(task, 0x8), at<void*>(task, 0xC), base + 0x4, task);
        host.crt_free_00bf65ac(at<void*>(task, 0x8));
    }
    at<std::uint32_t>(task, 0x8) = 0;
    at<std::uint32_t>(task, 0xC) = 0;
    at<std::uint32_t>(task, 0x10) = 0;
}

// ---------------------------------------------------------------------------
// 0092F930 BSP_UnitController_UpdatePoseFromBody (cc13-segmixD).
// __thiscall(ECX = controller (unit+1018h); float seconds), RET 4. Bytes
// 0092F930..0092FAE6. unit = [controller+1Ch], body = [controller+2Ch].
// - Byte controller+14h set: M = the unit's world matrix +CCh (00414DB0 first
//   when byte unit+C8h is 0).
// - Otherwise: M = 00C33650(00C43EA0(body; temp, seconds / 0.05f)), where 0.05f
//   is [00D0DE84] and the quotient is rounded to float.
// - Byte unit+61h set: M's angles (0042D2E0) are kept, except the middle one,
//   which becomes unit+FACh. M is rebuilt from identity through 00467050 with
//   x = unit+FB0h, z = unit+FB8h and the old y.
// - Finally +74h = M (004134F0), bytes unit+C8h and +10Ch = 0, and every child
//   from [unit+48h] along +44h is invalidated (0042ED50).
// unit_rudder.cpp reaches it through the unbound hook `controller_update_0092f930`.
// ---------------------------------------------------------------------------
void cc13_segmix_unit_controller_update_pose_0092f930(void* controller, float seconds, Cc13SegmixHost& host) {
    float matrix[16];                                                                  // [esp+54h]
    if (at<std::uint8_t>(controller, 0x14) != 0) {
        void* const unit = at<void*>(controller, 0x1C);
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        host.matrix_copy_4x4_004134f0(matrix, static_cast<std::uint8_t*>(unit) + 0xCC);
    } else {
        const float t = x87_float_round_trip(seconds / 0.05f);                        // 0092F971 FDIV, FSTP
        alignas(4) std::uint8_t transform[0x40];                                       // [esp+14h]
        void* const out = host.dyn_body_transform_at_00c43ea0(at<void*>(controller, 0x2C), transform, t);
        host.dyn_transform_expand_00c33650(out, matrix);                               // 0092F993
    }
    void* unit = at<void*>(controller, 0x1C);
    if (at<std::uint8_t>(unit, 0x61) != 0) {
        float c = 0.0f;
        float b = 0.0f;
        float a = 0.0f;
        host.matrix_extract_basis_angles_0042d2e0(matrix, &c, &b, &a);                // 0092F9B7
        b = at<float>(unit, 0xFAC);
        const float y = matrix[13];
        static const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                           0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        host.matrix_copy_4x4_004134f0(matrix, identity);                               // 0092FA50
        host.matrix_set_rotation_from_euler_00467050(matrix, x87_float_round_trip(c), x87_float_round_trip(b),
                                                     x87_float_round_trip(a));          // 0092FA73
        unit = at<void*>(controller, 0x1C);
        matrix[12] = at<float>(unit, 0xFB0);
        matrix[13] = y;
        matrix[14] = at<float>(unit, 0xFB8);
    }
    host.matrix_copy_4x4_004134f0(static_cast<std::uint8_t*>(unit) + 0x74, matrix);   // 0092FAB4
    unit = at<void*>(controller, 0x1C);
    at<std::uint8_t>(unit, 0xC8) = 0;
    at<std::uint8_t>(unit, 0x10C) = 0;
    for (void* child = at<void*>(unit, 0x48); child != nullptr; child = at<void*>(child, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);                       // 0092FAD3
    }
}

}  // namespace bsp
