// cc13_segmix: undone game bodies from segments 51, 58, 30, 41, 42, 50, 61, 54
// and 46 with no cc9 host (packet cc13-seg40F segmix, docs/CC13_SEGMIX.md). Names
// are hypotheses; see the header for the hooks. Each body was transcribed from
// the disk-byte disassembly (bsp.py disasm-raw).

#include "bsp/cc13_segmix.hpp"

#include <cmath>
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

// ---------------------------------------------------------------------------
// 007A84D0 BSP_MovCam_CycleTarget (cc13-segmixE; ledger name was
// BSP_Recon_PublishedListCursor). __thiscall(ECX = camera; byte backward), RET 4.
// Bytes 007A84D0..007A879F.
//
// The player is [game + [game+18ECh]*4 + 18CCh], and its lists object is
// L = [player+30h]. L holds three lists: A {+DD8h count, +DDCh head}, B {+DE4h,
// +DE8h} and C {+DF0h, +DF4h}. Each node has +4h next and +8h payload, and the
// payload's +4h is the unit.
//
// - With no current target ([camera+38Ch] = 0), it takes the head unit of the
//   first non-empty list. When that unit passes slot +5Ch(5), 007A6910 targets
//   it and the call returns.
// - Otherwise it finds the unit's position p across the joined lists: in A
//   (00793610), else in B (00793670) + nA, else in C (007936D0) + nA + nB.
// - It then steps p by +1, or by -1 when `backward` and byte camera+518h are both
//   set, and wraps p in [0, total). It takes the unit at p, and repeats while the
//   unit fails +5Ch(5), at most total + 1 times.
// - At the end it calls 007A6910 with the unit when +5Ch(5) holds, asked again,
//   or with null.
//
// Deviation: when p falls outside a list's walk, the image reads [0+4] and
// faults. The model stops there and returns without retargeting (no 007A6910).
// cc7's earlier evidence, under the old name: docs/RECON_SLOT_OBJECT.md.
// ---------------------------------------------------------------------------
void cc13_segmix_movcam_cycle_target_007a84d0(void* camera, std::uint8_t backward, Cc13SegmixHost& host) {
    const auto lists = [&]() -> void* {
        void* const game = host.game_global_00e188a8();
        const std::int32_t slot = at<std::int32_t>(game, 0x18EC);
        void* const player = at<void*>(game, 0x18CC + static_cast<std::size_t>(slot) * 4);
        return at<void*>(player, 0x30);
    };
    // The node at `index` of the list {+0 count, +4 head}, or null.
    const auto node_at = [&](void* list, std::int32_t index) -> void* {
        if (index < 0 || index >= at<std::int32_t>(list, 0x0)) return nullptr;
        std::int32_t i = 0;
        for (void* node = at<void*>(list, 0x4); node != nullptr; node = at<void*>(node, 0x4), ++i) {
            if (index == i) return node;
        }
        return nullptr;
    };
    const auto unit_of = [&](void* node) { return at<void*>(at<void*>(node, 0x8), 0x4); };

    void* unit = at<void*>(camera, 0x38C);                                              // 007A84D8
    if (unit == nullptr) {
        void* const l = lists();
        void* head = nullptr;
        if (at<std::int32_t>(l, 0xDD8) > 0) head = at<void*>(l, 0xDDC);
        else if (at<std::int32_t>(l, 0xDE4) > 0) head = at<void*>(l, 0xDE8);
        else if (at<std::int32_t>(l, 0xDF0) > 0) head = at<void*>(l, 0xDF4);
        if (head != nullptr) unit = unit_of(head);
        if (unit != nullptr && host.unit_vcall_5c_is_kind(unit, 5)) {                  // 007A855F
            host.movcam_set_target_007a6910(camera, unit);
            return;
        }
    }
    void* l = lists();
    const std::int32_t count_a = at<std::int32_t>(l, 0xDD8);
    const std::int32_t count_ab = count_a + at<std::int32_t>(l, 0xDE4);
    const std::int32_t total = count_ab + at<std::int32_t>(l, 0xDF0);
    if (total > 0) {
        std::int32_t p = host.player_lists_index_a_00793610(l, unit);                  // 007A85CC
        if (p == -1) p = host.player_lists_index_b_00793670(lists(), unit) + count_a;
        if (p == count_a - 1) p = host.player_lists_index_c_007936d0(lists(), unit) + count_ab;
        std::int32_t tries = 0;
        for (;;) {
            const std::int32_t step = backward != 0 && at<std::uint8_t>(camera, 0x518) != 0 ? -1 : 1;
            p += step;
            if (p >= total) p = 0;
            else if (p < 0) p = total - 1;
            l = lists();
            void* node = nullptr;
            if (p < count_a) node = node_at(static_cast<std::uint8_t*>(l) + 0xDD8, p);
            else if (p < count_ab) node = node_at(static_cast<std::uint8_t*>(l) + 0xDE4, p - count_a);
            else node = node_at(static_cast<std::uint8_t*>(l) + 0xDF0, p - count_ab);
            if (node == nullptr) return;                                               // the image faults here
            unit = unit_of(node);
            if (host.unit_vcall_5c_is_kind(unit, 5)) break;                            // 007A86C0
            if (tries++ >= total) break;
        }
    }
    if (unit != nullptr && host.unit_vcall_5c_is_kind(unit, 5)) {                      // 007A86ED
        host.movcam_set_target_007a6910(camera, unit);
        return;
    }
    host.movcam_set_target_007a6910(camera, nullptr);                                  // 007A86FB
}

// ---------------------------------------------------------------------------
// 007A87B0 BSP_MovCam_UpdateHandMode (cc13-segmixE). __thiscall(ECX = camera),
// RET. Bytes 007A87B0..007A8C51. It moves between the target's vehicle cameras
// with the stick.
//
// Inputs:
// - The cameras are the class array V = [[[camera+38Ch]+538h]+BCh] {+0h data,
//   +4h count}, with 5Ch-byte entries: +38h x, +40h z, byte +58h disabled. The
//   current entry is [camera+388h], i0.
// - The column half-width is R = class+A4h * 2^-6 ([00D04FC8]) as a float.
// - dx = -1 / 0 / 1 from camera+504h against -0.25f / 0.25f ([00CF00A8],
//   [00CE3868]). dy = +1 / 0 / -1 from camera+508h the same way (inverted).
// - The current column is s = -1 when -R > x0, 1 when x0 > R, else 0, and
//   n = s + dx.
//
// The search:
// 1. It returns when V is null or has fewer than 2 entries, or when n is outside
//    [-1, 1] and dy is 0.
// 2. Pass 1 checks each other enabled entry j. It must be in column n (x < -R for
//    n < 0; -R <= x <= R for 0; x > R for n > 0) and, for dy = -1 / +1, below /
//    above z0. Among those it keeps the first with the smallest |z0 - z|
//    (strictly smaller replaces).
// 3. When pass 1 finds nothing, and n != 0, dx = 0 and dy != 0, pass 2 runs.
//    It is the same over the middle column only, with entries read through
//    005CD820.
// 4. For a found entry: 007A6710(camera; j). When [camera+390h] is set, it calls
//    0079F710(camera; pose, [camera+390h], j, &failed), and 00797070(camera;
//    0, 0) when that did not fail.
//
// Bytes: D8 E9 at 007A8A22 is FSUBR ST(0),ST(1), and DE E2 at 007A8B7D is
// FSUBRP ST(2),ST(0). Both give z0 - z, and only the magnitude is used. All
// terms are float-exact under 24-bit precision. Every x87 compare that branches
// on CF rejects an unordered pair, as the image does.
// ---------------------------------------------------------------------------
void cc13_segmix_movcam_update_hand_mode_007a87b0(void* camera, Cc13SegmixHost& host) {
    const auto vec = [&]() -> void* {
        return at<void*>(at<void*>(at<void*>(camera, 0x38C), 0x538), 0xBC);
    };
    const auto unordered = [](float a, float b) { return std::isnan(a) || std::isnan(b); };
    void* v = vec();
    if (v == nullptr || at<std::int32_t>(v, 0x4) < 2) return;                       // 007A87CC..007A87D8
    std::int32_t dx = 0;
    std::int32_t dy = 0;
    const float stick_x = at<float>(camera, 0x504);
    if (-0.25f > stick_x) dx = -1;
    else if (stick_x > 0.25f) dx = 1;
    const float stick_y = at<float>(camera, 0x508);
    if (-0.25f > stick_y) dy = 1;
    else if (stick_y > 0.25f) dy = -1;

    const std::int32_t current = at<std::int32_t>(camera, 0x388);
    if (current >= at<std::int32_t>(v, 0x4)) host.vehicle_camera_array_resize_005cd640(v, current + 1);
    const auto entry = [&](void* array, std::int32_t j) {
        return at<std::uint8_t*>(array, 0x0) + static_cast<std::size_t>(j) * 0x5C;
    };
    const float x0 = x87_float_round_trip(at<float>(entry(v, current), 0x38));
    const float z0 = at<float>(entry(v, current), 0x40);
    void* const ship_class = at<void*>(at<void*>(camera, 0x38C), 0x538);
    const float r = static_cast<float>(static_cast<double>(at<float>(ship_class, 0xA4)) * 0.015625);
    const float minus_r = -r;
    std::int32_t column = 0;
    if (minus_r > x0) column = -1;
    else if (x0 > r) column = 1;
    const std::int32_t target = column + dx;                                           // 007A88C4
    if ((target < -1 || target > 1) && dy == 0) return;                                // 007A88D6

    std::int32_t best = -1;
    float best_z = 0.0f;                                                               // [esp+2Ch]
    const auto consider = [&](std::int32_t j, float z) {
        if (best != -1) {
            const float d1 = std::fabs(z0 - z);
            const float d2 = std::fabs(z0 - best_z);
            if (!(d2 > d1)) return;                                                    // FCOMIP, JBE
        }
        best = j;
        best_z = z;
    };
    const auto z_passes = [&](float z) {
        if (dy == 0) return true;
        if (dy == -1) return z0 > z;
        if (dy == 1) return z > z0;
        return false;
    };
    v = vec();
    for (std::int32_t j = 0; j < at<std::int32_t>(v, 0x4); ++j, v = vec()) {           // pass 1, 007A88FD
        if (j == current) continue;
        if (j >= at<std::int32_t>(v, 0x4)) host.vehicle_camera_array_resize_005cd640(v, j + 1);
        if (at<std::uint8_t>(entry(v, j), 0x58) != 0) continue;
        v = vec();
        if (j >= at<std::int32_t>(v, 0x4)) host.vehicle_camera_array_resize_005cd640(v, j + 1);
        const float x = at<float>(entry(v, j), 0x38);
        const float z = at<float>(entry(v, j), 0x40);
        bool in_column = false;
        if (target < 0) {
            in_column = minus_r > x;
        } else if (target == 0) {
            in_column = !(unordered(x, minus_r) || x < minus_r) && !unordered(r, x) && r >= x;
        } else {
            in_column = x > r;
        }
        if (!in_column || !z_passes(z)) continue;
        consider(j, z);
    }
    if (best == -1) {
        if (target == 0 || dx != 0 || dy == 0) return;                                 // 007A8A82..007A8A9A
        v = vec();
        for (std::int32_t j = 0; j < at<std::int32_t>(v, 0x4); ++j, v = vec()) {       // pass 2, 007A8AD0
            if (j == current) continue;
            if (j >= at<std::int32_t>(v, 0x4)) host.vehicle_camera_array_resize_005cd640(v, j + 1);
            if (at<std::uint8_t>(entry(v, j), 0x58) != 0) continue;
            auto* const e = static_cast<std::uint8_t*>(host.vehicle_camera_array_at_005cd820(vec(), j));
            const float x = x87_float_round_trip(at<float>(e, 0x38));
            const float z = at<float>(e, 0x40);
            if (unordered(x, minus_r) || x < minus_r) continue;                        // 007A8B32 JB
            if (unordered(r, x) || r < x) continue;                                    // 007A8B40 JB
            if (dy != -1 && dy != 1) continue;
            if (!z_passes(z)) continue;
            consider(j, z);
        }
        if (best == -1) return;
    }
    host.movcam_enter_vehicle_camera_007a6710(camera, best);                          // 007A8C08
    void* const object = at<void*>(camera, 0x390);
    if (object == nullptr) return;
    alignas(4) std::uint8_t pose[0x40];
    std::uint8_t failed = 0;
    host.movcam_get_vehicle_camera_pose_0079f710(camera, pose, object, best, &failed);  // 007A8C29
    if (failed == 0) host.movcam_set_look_target_00797070(camera, 0, 0);
}

// ---------------------------------------------------------------------------
// 007A74F0 BSP_MovCam_SetPathWalkPointProperty (cc13-segmixF). This is
// __thiscall(ECX = camera; LuaObject table by value (14h), float a, byte reverse,
// float c, name, variants), RET 28h. Bytes 007A74F0..007A7C50. The sole caller is
// the Lua native 008B4DA0. cc13_left55_seg56b reaches it through the hook
// movcam_set_path_walk_point_property_007a74f0.
//
// It clears the path records (0079C830), then walks the table (00B67080 /
// 00B67190, ended by 00B66420). For each key k, the slot is k - 1 when reverse is
// set and k + 1 otherwise. The camera+478h pointer vector grows to the slot + 1
// (unsigned compare), and a new 4Ch path record goes into the slot through the
// checked index.
//
// The record starts with vtable 00D043D8, zeroes, and +3Ch = +40h = 1/3f
// ([00D04678]). It is then filled from the value's fields. Each field is read into
// one LuaObject, T, that lives across the whole iteration:
//
// | Field | Effect on the record |
// |---|---|
// | parent | entity +14h, through the observer swap below |
// | pos | +18h/+1Ch/+20h = x, y, z (read in the order z, y, x; numbers taken x, y, z) |
// | rotRefEntity | entity +24h through the swap; mode +48h = 1 (+48h is 0 first) |
// | rotRefPosW | +28h..+30h via 00888760; mode 2 |
// | rotRefPosL | the same; mode 3 |
// | rotRefRotW | +34h = x, +38h = y; mode 4 |
// | rotRefRotL | the same; mode 5 |
// | sustainBefore, sustainAfter, speed | +3Ch, +40h, +44h |
//
// The observer swap for slot S (+14h or +24h, with the other slot O) is:
// 1. When the old [S] is set, differs from the new entity, and differs from [O],
//    unregister (rec) from the old (006952A0).
// 2. Store the new entity in S.
// 3. When the new entity is set and differs from [O], register (rec) with it
//    (00694A60).
//
// The tail calls 007A5A00(camera; a, reverse, c, name, variants) and destroys the
// value, the key and the by-value table.
//
// Notes:
// - A null record from operator new is stored as is, and the field stores fault
//   through it as in the image (not guarded).
// - The EH frame 00C8BBF4 is not modelled.
// ---------------------------------------------------------------------------
void cc13_segmix_movcam_set_path_walk_point_property_007a74f0(void* camera, void* table, float a,
                                                              std::uint8_t reverse, float c, void* name,
                                                              std::uint32_t variants, Cc13SegmixHost& host) {
    host.movcam_clear_path_records_0079c830(camera);                                  // 007A751C
    alignas(4) std::uint8_t key[0x14];                                                 // [esp+54h]
    alignas(4) std::uint8_t value[0x14];                                               // [esp+3Ch]
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(value);
    host.lua_object_iterate_first_00b67080(table, key, value);
    const auto number = [&](void* object) {
        return static_cast<float>(host.lua_object_get_number_00b66270(object));
    };
    while (!host.lua_object_is_unbound_00b66420(table, key)) {
        const std::int32_t index = host.lua_object_get_integer_00b66290(key);
        const std::uint32_t slot = static_cast<std::uint32_t>(reverse != 0 ? index - 1 : index + 1);
        void* const vector = static_cast<std::uint8_t*>(camera) + 0x478;
        const auto size = [&]() -> std::uint32_t {
            void** const begin = at<void**>(vector, 0x4);
            return begin == nullptr ? 0u : static_cast<std::uint32_t>(at<void**>(vector, 0x8) - begin);
        };
        if (size() < slot + 1) host.pointer_vector_resize_0079eb10(vector, slot + 1, nullptr);
        auto* rec = static_cast<std::uint8_t*>(host.crt_operator_new_00bf681b(0x4C));
        if (rec != nullptr) {
            at<std::uint32_t>(rec, 0x4) = 0;
            at<std::uint32_t>(rec, 0x8) = 0;
            at<std::uint32_t>(rec, 0xC) = 0;
            at<std::uint32_t>(rec, 0x14) = 0;
            at<std::uint8_t>(rec, 0x10) = 0;
            at<std::uint32_t>(rec, 0x0) = 0x00D043D8;
            at<float>(rec, 0x18) = 0.0f;
            at<float>(rec, 0x1C) = 0.0f;
            at<float>(rec, 0x20) = 0.0f;
            at<std::uint32_t>(rec, 0x24) = 0;
            for (std::size_t off = 0x28; off <= 0x38; off += 4) at<float>(rec, off) = 0.0f;
            at<std::uint32_t>(rec, 0x48) = 0;
            float third;
            const std::uint32_t third_bits = 0x3EAAAAABu;                             // [00D04678]
            std::memcpy(&third, &third_bits, sizeof third);
            at<float>(rec, 0x40) = third;
            at<float>(rec, 0x3C) = third;
            at<float>(rec, 0x44) = 0.0f;
        }
        if (at<void**>(vector, 0x4) == nullptr || slot >= size()) host.crt_invalid_parameter_noinfo_00bf6713();
        at<void**>(vector, 0x4)[slot] = rec;                                           // 007A7660

        const auto swap_entity = [&](std::size_t slot_offset, std::size_t other_offset, void* entity) {
            void* const old = at<void*>(rec, slot_offset);
            if (old != nullptr && old != entity && old != at<void*>(rec, other_offset)) {
                host.observer_unregister_pair_006952a0(old, rec);
            }
            at<void*>(rec, slot_offset) = entity;
            if (entity != nullptr && entity != at<void*>(rec, other_offset)) {
                host.observer_register_pair_00694a60(entity, rec);
            }
        };
        alignas(4) std::uint8_t field[0x14];                                           // T, [esp+10h]
        alignas(4) std::uint8_t temp[0x14];
        const auto read_field = [&](const char* name_key) {                           // T = value[key]
            host.lua_object_assign_00b67690(field, host.lua_object_get_by_name_00b67800(value, temp, name_key));
            host.lua_object_destroy_00b67700(temp);
        };
        host.lua_object_get_by_name_00b67800(value, field, "parent");                 // T built in place
        if (!host.lua_object_is_nil_00b65fb0(field)) {
            swap_entity(0x14, 0x24, host.object_handle_from_lua_table_00888aa0(field));
        }
        read_field("pos");
        {
            alignas(4) std::uint8_t z_object[0x14];
            alignas(4) std::uint8_t y_object[0x14];
            alignas(4) std::uint8_t x_object[0x14];
            void* const z_field = host.lua_object_get_by_name_00b67800(field, z_object, "z");
            void* const y_field = host.lua_object_get_by_name_00b67800(field, y_object, "y");
            void* const x_field = host.lua_object_get_by_name_00b67800(field, x_object, "x");
            const float x = number(x_field);
            const float y = number(y_field);
            const float z = number(z_field);
            at<float>(rec, 0x18) = x;
            at<float>(rec, 0x1C) = y;
            at<float>(rec, 0x20) = z;
            host.lua_object_destroy_00b67700(x_object);
            host.lua_object_destroy_00b67700(y_object);
            host.lua_object_destroy_00b67700(z_object);
        }
        at<std::uint32_t>(rec, 0x48) = 0;                                              // 007A77BF
        read_field("rotRefEntity");
        if (!host.lua_object_is_nil_00b65fb0(field)) {
            swap_entity(0x24, 0x14, host.object_handle_from_lua_table_00888aa0(field));
            at<std::uint32_t>(rec, 0x48) = 1;
        }
        const auto read_position = [&](const char* name_key, std::uint32_t mode) {
            read_field(name_key);
            if (host.lua_object_is_nil_00b65fb0(field)) return;
            float out[3];
            const float* const p = host.lua_object_read_vector3_00888760(out, field);
            at<float>(rec, 0x28) = p[0];
            at<float>(rec, 0x2C) = p[1];
            at<float>(rec, 0x30) = p[2];
            at<std::uint32_t>(rec, 0x48) = mode;
        };
        read_position("rotRefPosW", 2);
        read_position("rotRefPosL", 3);
        const auto read_rotation = [&](const char* name_key, std::uint32_t mode) {
            read_field(name_key);
            if (host.lua_object_is_nil_00b65fb0(field)) return;
            alignas(4) std::uint8_t y_object[0x14];
            alignas(4) std::uint8_t x_object[0x14];
            void* const y_field = host.lua_object_get_by_name_00b67800(field, y_object, "y");
            void* const x_field = host.lua_object_get_by_name_00b67800(field, x_object, "x");
            const float x = number(x_field);
            const float y = number(y_field);
            at<float>(rec, 0x34) = x;
            at<float>(rec, 0x38) = y;
            host.lua_object_destroy_00b67700(x_object);
            host.lua_object_destroy_00b67700(y_object);
            at<std::uint32_t>(rec, 0x48) = mode;
        };
        read_rotation("rotRefRotW", 4);
        read_rotation("rotRefRotL", 5);
        const auto read_number = [&](const char* name_key, std::size_t offset) {
            read_field(name_key);
            if (!host.lua_object_is_nil_00b65fb0(field)) at<float>(rec, offset) = number(field);
        };
        read_number("sustainBefore", 0x3C);
        read_number("sustainAfter", 0x40);
        read_number("speed", 0x44);
        host.lua_object_destroy_00b67700(field);                                       // 007A7B92
        host.lua_object_iterate_next_00b67190(table, key, value);
    }
    host.movcam_begin_path_walk_007a5a00(camera, a, reverse, c, name, variants);      // 007A7BFC
    host.lua_object_destroy_00b67700(value);
    host.lua_object_destroy_00b67700(key);
    host.lua_object_destroy_00b67700(table);
}

// ---------------------------------------------------------------------------
// 00824B60 BSP_UnitInstance_OnWrecked (cc13-segmixG). __thiscall(ECX = unit), RET.
// Bytes 00824B60..008252B0. This is the ship override of vtable slot 7Ch, which
// 00926390 tail-dispatches at 009263AE, and 007497F0 jumps to it. Only parts of
// it were in the repository: cc9's partial fragments native_unit_wreck_tail
// (00824F39..00824FE4) and unit_death_sink (the sink block, the scatter and the
// throttle cut), and game_hosts_ready logs the whole handler as unimplemented.
// This is the whole body. Write-ups: docs/UNIT_DEATH_MESSAGE_AND_SINK.md and
// docs/CC13_SEGMIX.md.
//
// Its six steps, in image order:
// 1. Wreck smoke. It runs when [desc+55Ch] and [game+19FCh] are both set:
//    - L = float(length A0h * 0.2), H = float(height A8h * 0.6) and
//      n = trunc(clamp(L / 10, 1, 2)).
//    - For each of the n puffs, it draws z = U(-0.0f - L, L) and appends
//      (0, H, z) to the float3 vector unit+74Ch.
//    - It then creates the [desc+55Ch] effect on [unit+4A4h] at the translation
//      (0, H, z), and appends that reference to the vector unit+758h.
// 2. In game mode 4 ([[game+38h]+60h]), it stops and drops the effect handles
//    +9ECh, +9F0h and +9F4h.
// 3. The sink block: the leak reset, the inertia doubled, the dampings 2.5 / 0.5,
//    unit+828h = +82Ch = 0, and 00818970.
// 4. When the destroy cause [unit+70h] is 1, it draws unit+BC8h from the settings
//    range +64Ch..+650h. When [desc+6A8h] is set, it also fills five float3 slots
//    at unit+B68h:
//    - z = U(-A0h/2.5, A0h/2.5) is drawn first;
//    - then x = U(-A4h/2.5, A4h/2.5);
//    - then y = U(0, A8h/3).
// 5. Breakup: when the class has 50h-byte pieces (+6E0h..+6E4h) or 1Ch-byte
//    pieces (+700h..+704h), and [game+21D0h] is set, it calls 004A5AA0.
// 6. The throttle cut, 0.0f at the order ring slot [unit+97Ch] (unit+838h +
//    cursor * 20h). Then it releases [unit+740h] through vtable +10h, clears the
//    slot, and calls 00959450.
//
// Byte checks: DE F1 at 00825131 / 00825179 is FDIVRP ST(1),ST(0), giving
// -a / 2.5. DC F9 at 00825161 is FDIV ST(1),ST(0), giving b / 2.5. The fistp at
// 00824C19 runs under RC = chop (OR 0C00h), so n is truncated, not rounded as
// UNIT_DEATH_MESSAGE_AND_SINK.md wrote. Within the clamp to [1, 2] that gives
// n = 1 below L = 20 and n = 2 from L = 20, where L is 0.2 times the class
// length.
//
// The 0.2 and 0.6 products are double constants under 24-bit precision. They
// are modelled as float(double * double), and a double-rounding difference is
// possible in halfway cases. The divisions are exact as float(double / double).
// The EH frame 00C9184D is not modelled.
// ---------------------------------------------------------------------------
namespace {

// The intrusive reference release the handler inlines: decrement +4h, slot 0 at zero.
void release_reference(void* object, Cc13SegmixHost& host) {
    if (object == nullptr) return;
    if (host.interlocked_decrement(&at<std::int32_t>(object, 0x4)) == 0) host.vcall_00_release(object);
}

void* retain_reference(void* object, Cc13SegmixHost& host) {
    if (object != nullptr) host.interlocked_increment(&at<std::int32_t>(object, 0x4));
    return object;
}

}  // namespace

void cc13_segmix_ship_on_wrecked_00824b60(void* unit, Cc13SegmixHost& host) {
    void* const desc = at<void*>(unit, 0x538);
    void* game = host.game_global_00e188a8();
    if (at<void*>(desc, 0x55C) != nullptr && at<std::uint32_t>(game, 0x19FC) != 0) {   // 00824B9A, 00824BAD
        const float length = static_cast<float>(static_cast<double>(at<float>(desc, 0xA0)) * 0.2);
        const float height = static_cast<float>(static_cast<double>(at<float>(desc, 0xA8)) * 0.6);
        float scale = static_cast<float>(static_cast<double>(length) / 10.0);
        if (!(std::isnan(scale) || 1.0f <= scale)) {
            scale = 1.0f;                                                              // 00824BF1
        } else if (!std::isnan(scale) && scale > 2.0f) {
            scale = 2.0f;                                                              // 008252A6
        }
        std::uint32_t count = std::isnan(scale) ? 0u : static_cast<std::uint32_t>(scale);   // FISTP, chop
        const float low = -0.0f - length;                                              // [00D7A208] - L
        for (; count != 0; --count) {
            const float z = host.random_uniform_float_range_00bd2f10(1, low, length); // 00824C5C
            // The float3 vector {+74Ch data, +750h size, +754h capacity}.
            if (at<std::uint32_t>(unit, 0x750) == at<std::uint32_t>(unit, 0x754)) {
                const std::uint32_t capacity = at<std::uint32_t>(unit, 0x754) * 2 + 2;
                at<std::uint32_t>(unit, 0x754) = capacity;
                auto* const fresh = static_cast<float*>(host.crt_operator_new_00bf55be(capacity * 12));
                for (std::uint32_t i = 0; i < at<std::uint32_t>(unit, 0x750); ++i) {
                    if (fresh == nullptr) continue;
                    const float* const source = at<float*>(unit, 0x74C) + i * 3;
                    fresh[i * 3 + 0] = source[0];
                    fresh[i * 3 + 1] = source[1];
                    fresh[i * 3 + 2] = source[2];
                }
                if (at<void*>(unit, 0x74C) != nullptr) host.crt_free_00bf6989(at<void*>(unit, 0x74C));
                at<float*>(unit, 0x74C) = fresh;
            }
            float* const point = at<float*>(unit, 0x74C) + at<std::uint32_t>(unit, 0x750) * 3;
            if (point != nullptr) {
                point[0] = 0.0f;
                point[1] = height;
                point[2] = z;
            }
            at<std::uint32_t>(unit, 0x750) += 1;                                       // 00824D17

            float matrix[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, height, z, 1.0f};
            void* const definition = retain_reference(at<void*>(desc, 0x55C), host);   // the by-value slot
            void* effect = nullptr;                                                    // [esp+20h]
            void** const created = static_cast<void**>(host.point_effect_create_with_parent_matrix_008687c0(
                &effect, at<void*>(unit, 0x4A4), definition, matrix, 1, 0, 0));        // 00824DE4
            // The reference vector {+758h data, +75Ch size, +760h capacity}.
            if (at<std::uint32_t>(unit, 0x75C) == at<std::uint32_t>(unit, 0x760)) {
                const std::uint32_t capacity = at<std::uint32_t>(unit, 0x760) * 2 + 2;
                at<std::uint32_t>(unit, 0x760) = capacity;
                auto* const fresh = static_cast<void**>(host.crt_operator_new_00bf55be(capacity * 4));
                for (std::uint32_t i = 0; i < at<std::uint32_t>(unit, 0x75C); ++i) {
                    void** const old_slot = at<void**>(unit, 0x758) + i;
                    if (fresh != nullptr) fresh[i] = retain_reference(*old_slot, host);
                    void* const old = *old_slot;
                    if (old != nullptr) {
                        release_reference(old, host);
                        *old_slot = nullptr;
                    }
                }
                if (at<void*>(unit, 0x758) != nullptr) host.crt_free_00bf6989(at<void*>(unit, 0x758));
                at<void**>(unit, 0x758) = fresh;
            }
            void** const slot = at<void**>(unit, 0x758) + at<std::uint32_t>(unit, 0x75C);
            if (slot != nullptr) *slot = retain_reference(*created, host);
            at<std::uint32_t>(unit, 0x75C) += 1;                                       // 00824EFA
            release_reference(effect, host);
            effect = nullptr;
        }
    }

    game = host.game_global_00e188a8();                                                // 00824F39
    if (at<void*>(game, 0x38) != nullptr && at<std::int32_t>(at<void*>(game, 0x38), 0x60) == 4) {
        for (std::size_t offset = 0x9EC; offset <= 0x9F4; offset += 4) {
            void* const handle = at<void*>(unit, offset);
            if (handle == nullptr) continue;
            host.live_effect_manager_stop_matching_008674c0(host.effect_manager_singleton_004d1100(), handle);
            host.effect_reference_adopt_00484620(static_cast<std::uint8_t*>(unit) + offset, nullptr);
        }
    }

    host.leak_manager_reset_for_wreck_0074ec50(static_cast<std::uint8_t*>(unit) + 0x10D4);   // 00824FEB
    float buffer[3];
    const float* const inertia =
        host.dyn_body_get_inertia_00c37f10(at<void*>(at<void*>(unit, 0x1018), 0x2C), buffer);
    const float doubled[3] = {static_cast<float>(2.0 * inertia[0]), static_cast<float>(2.0 * inertia[1]),
                              static_cast<float>(2.0 * inertia[2])};
    host.dyn_body_set_inertia_00c37e70(at<void*>(at<void*>(unit, 0x1018), 0x2C), doubled);
    host.dyn_body_set_angular_damping_00c37de0(at<void*>(at<void*>(unit, 0x1018), 0x2C), 2.5f);   // [00CF87C8]
    host.dyn_body_set_linear_damping_00c37e00(at<void*>(at<void*>(unit, 0x1018), 0x2C), 0.5f);    // [00CE3800]
    at<float>(unit, 0x828) = 0.0f;
    at<float>(unit, 0x82C) = 0.0f;
    host.ship_stop_wreck_effects_00818970(unit);                                       // 0082508E

    if (at<std::int32_t>(unit, 0x70) == 1) {                                           // 00825093
        void* const settings = host.game_settings_00424c40();
        at<float>(unit, 0xBC8) =
            host.random_uniform_float_range_00bd2f10(1, at<float>(settings, 0x64C), at<float>(settings, 0x650));
        if (at<void*>(at<void*>(unit, 0x538), 0x6A8) != nullptr) {
            for (std::size_t i = 0; i < 5; ++i) {                                     // 008250F0
                float* const slot = &at<float>(unit, 0xB68 + i * 0xC);
                slot[0] = 0.0f;
                slot[1] = 0.0f;
                slot[2] = 0.0f;
                const float hull_length = at<float>(at<void*>(unit, 0x538), 0xA0);
                const float z_high = static_cast<float>(static_cast<double>(hull_length) / 2.5);
                const float z_low = static_cast<float>(-static_cast<double>(hull_length) / 2.5);
                slot[2] = slot[2] + host.random_uniform_float_range_00bd2f10(1, z_low, z_high);
                const float hull_width = at<float>(at<void*>(unit, 0x538), 0xA4);
                const float x_high = static_cast<float>(static_cast<double>(hull_width) / 2.5);
                const float x_low = static_cast<float>(-static_cast<double>(hull_width) / 2.5);
                slot[0] = slot[0] + host.random_uniform_float_range_00bd2f10(1, x_low, x_high);
                const float height = at<float>(at<void*>(unit, 0x538), 0xA8);
                const float y_high = static_cast<float>(static_cast<double>(height) / 3.0);
                slot[1] = slot[1] + host.random_uniform_float_range_00bd2f10(1, 0.0f, y_high);
            }
        }
    }

    void* const ship_class = at<void*>(unit, 0x538);                                   // 008251D3
    const auto count_of = [&](std::size_t begin, std::size_t end, std::int32_t stride) {
        const auto first = at<std::int32_t>(ship_class, begin);
        if (first == 0) return 0;
        return (at<std::int32_t>(ship_class, end) - first) / stride;
    };
    if (count_of(0x6E0, 0x6E4, 0x50) != 0 || count_of(0x700, 0x704, 0x1C) != 0) {
        void* const manager = at<void*>(host.game_global_00e188a8(), 0x21D0);
        if (manager != nullptr) host.breakup_register_wreck_004a5aa0(manager, unit);  // 0082523A
    }

    const std::uint32_t cursor = at<std::uint32_t>(unit, 0x97C);                       // 0082523F
    at<float>(unit, 0x838 + static_cast<std::size_t>(cursor << 5)) = 0.0f;
    void* const holder = at<void*>(unit, 0x740);
    if (holder != nullptr) {
        host.vcall_10(holder);
        at<void*>(unit, 0x740) = nullptr;
    }
    host.unit_on_destroyed_00959450(unit);                                             // 00825271
}

// ---------------------------------------------------------------------------
// 00935D30, the ship breakup (cc13-segmixI). __thiscall(ECX = controller, the
// unit+1018h controller), RET. Bytes 00935D30..00936DBE, with an aligned frame and
// EH frame 00CA771E (not modelled). 00814520 BSP_UnitInstance_OnBreakupMessage
// tail-jumps here after its 004A5AA0 registration. The unit is [controller+1Ch].
// Liveness/RNG: each breakup makes 11 stream-1 draws (6 front, 5 back). cc9's
// unit_death_sink.cpp calls host.breakup_parts, which nothing implements, so host
// breakups skip them.
// The two hull halves are the nodes [controller+370h] (front, named ELELLYE) and
// [controller+374h] (back, named HATULLYA). Steps, in image order:
//
// 1. Hide the interior. Each child of the model instance named "hajobelso*"
//    (0071BA20) gets 00B6DA70(1.0f, 0), then moves to its local position minus
//    100000 on every axis (slot +2Ch). The bytes are DC E9 (FSUB ST(1),ST(0)),
//    D8 E1 and D8 /5, all p - 1e5.
// 2. It copies both halves' world matrices (refreshed through 00B6DB70 when
//    +5Ch bit 2 is clear) and renames +374h HATULLYA and +370h ELELLYE.
// 3. Every gun child (kind 20h) with [gun+3C0h] gets 007297B0
//    BSP_Gun_ApplyWreckVisibility.
// 4. The root node R = model+10h(unit+190h(0)) gets 00B6DA70(1.0f, 0) and the
//    automatic world sphere when it is the current group type.
// 5. Two 398h TickableGameEntity pieces. Each is zeroed, constructed (00929E50),
//    given slot +98h(0, [game+19CCh], identity), posed at its half's world
//    matrix M, and installed (0092AAE0) with a source {unit, half node, p, side}.
//    The offset comes from the shared stream (ECX = 1):
//    - Front: sx, sy, sz = sign(U(-1, 1)) in that order. a = U(0, 1.5) * sy,
//      b = U(0, 1.5) * sz and d = U(10, 20). p = ((M.t + d * row2) + b * row0) +
//      a * row1, and side = float(sx).
//    - Back: sy, sz, a, b and d drawn the same way (5 draws, no sx).
//      p = ((M.t - d * row2) + b * row0) + a * row1. side is left at the front's
//      value, because the image never rewrites source+14h.
//    Each sign is -1 when 0 > r, 1 when r > 0, else 0, with an unordered r
//    giving 0.
// 6. The pieces take the R children ELELLYE / HATULLYA (00B6E750, recursive)
//    through 0092A240, and point at each other at +33Ch.
// 7. Each gun with a model instance re-parents its named node (found under R)
//    to ELELLYE when the node's position in the inverse instance frame has
//    z > [class+580h], else to HATULLYA. The automatic world sphere follows.
// 8. Clears +370h/+374h and kills the unit (00926D90(1)). Every shape of the
//    body [controller+2Ch] gets collide mask 0 and group 0.
// 9. Creates [settings+3A8h] at the unit's world position on [game+19ECh]
//    (008685E0), sets effect byte +9h, and frees the name vector.
//
// Every float op is float-exact under 24-bit precision: products and sums of
// floats, rounded at each FSTP, as written here.
// ---------------------------------------------------------------------------
namespace {

struct SegmixNativeString {
    std::uint32_t length;
    char* data;
};

// The inlined temporary: resize, copy the text with its NUL, use, return to the sized pool.
template <typename Use>
void with_native_string(const char* text, std::uint32_t length, Cc13SegmixHost& host, Use use) {
    SegmixNativeString string = {0, nullptr};
    host.native_string_resize_0041dd40(&string, length, 1);
    if (string.data != nullptr) host.crt_memcpy_00bf7680(string.data, text, string.length + 1);
    use(&string);
    if (string.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), string.data, string.length + 1, 1);
    }
}

std::int32_t stream_sign(float r) {
    if (0.0f > r) return -1;
    if (r > 0.0f) return 1;
    return 0;
}

void refresh_and_copy_world(void* node, float* out, Cc13SegmixHost& host) {
    if ((at<std::uint8_t>(node, 0x5C) & 2) == 0) host.transform_refresh_world_matrix_00b6db70(node);
    std::memcpy(out, &at<float>(node, 0xF0), 16 * sizeof(float));
}

}  // namespace

void cc13_segmix_ship_break_in_two_00935d30(void* controller, Cc13SegmixHost& host) {
    const auto instance_of = [&](void* entity) -> void* {
        void* const visual = at<void*>(entity, 0x360);
        return visual != nullptr ? at<void*>(visual, 0x160) : nullptr;
    };
    alignas(4) std::uint8_t interior[0x10] = {};                                     // [F+68h]
    host.node_collect_children_by_name_prefix_0071ba20(instance_of(at<void*>(controller, 0x1C)), interior,
                                                       "hajobelso");
    float scratch[3];                                                                  // [F+13Ch]
    for (std::uint32_t i = 0;; ++i) {
        void** const first = at<void**>(interior, 0x4);
        if (first == nullptr || i >= static_cast<std::uint32_t>(at<void**>(interior, 0x8) - first)) break;
        host.scene_node_set_visibility_factor_00b6da70(first[i], 1.0f, 0);           // 00935DB7
        const auto checked = [&]() {
            void** const begin = at<void**>(interior, 0x4);
            if (begin == nullptr || i >= static_cast<std::uint32_t>(at<void**>(interior, 0x8) - begin)) {
                host.crt_invalid_parameter_noinfo_00bf6713();
            }
            return at<void**>(interior, 0x4)[i];
        };
        void* const node = checked();
        const float* const p = host.node_copy_local_position_00b6e0a0(checked(), scratch);
        const float moved[3] = {static_cast<float>(static_cast<double>(p[0]) - 100000.0),   // [00CF81F0]
                                static_cast<float>(static_cast<double>(p[1]) - 100000.0),
                                static_cast<float>(static_cast<double>(p[2]) - 100000.0)};
        host.node_vcall_2c_set_position(node, moved);                                  // 00935E45
    }

    float front_world[16];                                                             // [F+F8h]
    float back_world[16];                                                              // [F+B8h]
    refresh_and_copy_world(at<void*>(controller, 0x370), front_world, host);
    refresh_and_copy_world(at<void*>(controller, 0x374), back_world, host);
    with_native_string("HATULLYA", 8, host, [&](const void* name) {
        host.node_assign_name_00b6f960(at<void*>(controller, 0x374), name);           // 009360DB
    });
    with_native_string("ELELLYE", 7, host, [&](const void* name) {
        host.node_assign_name_00b6f960(at<void*>(controller, 0x370), name);           // 0093614D
    });
    for (void* child = at<void*>(at<void*>(controller, 0x1C), 0x48); child != nullptr; child = at<void*>(child, 0x44)) {
        if (host.unit_vcall_5c_is_kind(child, 0x20) && at<void*>(child, 0x3C0) != nullptr) {
            host.gun_apply_wreck_visibility_007297b0(child);                           // 0093619F
        }
    }
    void* const unit = at<void*>(controller, 0x1C);
    void* const key = host.unit_vcall_190(unit, 0);
    void* const root = host.model_vcall_10_root_node(at<void*>(unit, 0x4A4), key);    // [F+64h]
    host.scene_node_set_visibility_factor_00b6da70(root, 1.0f, 0);
    if (host.node_vcall_0c_is_type(root, host.group_current_type_00b8e600())) {
        host.group_enable_automatic_world_sphere_00b8f0f0(root);
    }

    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const auto make_piece = [&](const float* world) -> void* {
        void* const block = host.crt_operator_new_00bf55be(0x398);
        host.crt_memset_00bf79f0(block, 0, 0x398);
        void* const piece = block != nullptr ? host.tickable_game_entity_construct_00929e50(block) : nullptr;
        host.entity_vcall_98(piece, 0, at<std::uint32_t>(host.game_global_00e188a8(), 0x19CC), identity);
        host.entity_set_world_pose_matrix_00741e90(piece, world);
        return piece;
    };
    const auto draw = [&](float lo, float hi) { return host.random_uniform_float_range_00bd2f10(1, lo, hi); };
    const auto scaled = [](float value, std::int32_t sign) {
        return static_cast<float>(value * static_cast<float>(sign));                  // FIMUL, exact
    };
    // The 0092AAE0 source block [F+4Ch]: unit, node, x, y, z, side (float).
    alignas(4) std::uint8_t source[0x18] = {};

    void* const front = make_piece(front_world);                                       // 00936235
    const std::int32_t sx = stream_sign(draw(-1.0f, 1.0f));                           // 00936328
    at<void*>(source, 0x0) = unit;
    at<void*>(source, 0x4) = at<void*>(controller, 0x370);
    const float side = static_cast<float>(sx);                                         // CVTSI2SS
    const float ry = draw(-1.0f, 1.0f);
    const float rz = draw(-1.0f, 1.0f);
    {
        const float a = scaled(draw(0.0f, 1.5f), stream_sign(ry));                   // [00CE380C]
        const float u[3] = {front_world[4] * a, front_world[5] * a, front_world[6] * a};
        const float b = scaled(draw(0.0f, 1.5f), stream_sign(rz));
        const float v[3] = {b * front_world[0], b * front_world[1], b * front_world[2]};
        const float d = draw(10.0f, 20.0f);                                            // [00CE38B8], [00CE3930]
        const float w[3] = {front_world[8] * d, front_world[9] * d, front_world[10] * d};
        float p[3];
        for (int k = 0; k < 3; ++k) {
            p[k] = front_world[12 + k] + w[k];
            p[k] = p[k] + v[k];
            p[k] = p[k] + u[k];
        }
        at<float>(source, 0x8) = p[0];
        at<float>(source, 0xC) = p[1];
        at<float>(source, 0x10) = p[2];
        at<float>(source, 0x14) = side;
        host.tickable_install_collision_0092aae0(front, source);                      // 00936584
    }

    void* const back = make_piece(back_world);                                         // 009365B7
    at<void*>(source, 0x4) = at<void*>(controller, 0x374);
    {
        const float ry2 = draw(-1.0f, 1.0f);
        const float rz2 = draw(-1.0f, 1.0f);
        const float a = scaled(draw(0.0f, 1.5f), stream_sign(ry2));
        const float u[3] = {back_world[4] * a, back_world[5] * a, back_world[6] * a};
        const float b = scaled(draw(0.0f, 1.5f), stream_sign(rz2));
        const float v[3] = {b * back_world[0], b * back_world[1], b * back_world[2]};
        const float d = draw(10.0f, 20.0f);
        const float w[3] = {back_world[8] * d, back_world[9] * d, back_world[10] * d};
        float p[3];
        for (int k = 0; k < 3; ++k) {
            p[k] = back_world[12 + k] - w[k];
            p[k] = p[k] + v[k];
            p[k] = p[k] + u[k];
        }
        at<float>(source, 0x8) = p[0];
        at<float>(source, 0xC) = p[1];
        at<float>(source, 0x10) = p[2];
        host.tickable_install_collision_0092aae0(back, source);                       // 009368B2
    }

    with_native_string("ELELLYE", 7, host, [&](const void* name) {
        host.tickable_entity_attach_node_0092a240(front, host.node_find_child_by_name_00b6e750(root, name, 1));
    });
    with_native_string("HATULLYA", 8, host, [&](const void* name) {
        host.tickable_entity_attach_node_0092a240(back, host.node_find_child_by_name_00b6e750(root, name, 1));
    });
    at<void*>(front, 0x33C) = back;                                                    // 009369AF
    at<void*>(back, 0x33C) = front;

    for (void* gun = at<void*>(at<void*>(controller, 0x1C), 0x48); gun != nullptr; gun = at<void*>(gun, 0x44)) {
        if (!host.unit_vcall_5c_is_kind(gun, 0x20)) continue;
        float inverse[16];                                                             // [F+78h], REP MOVSD
        std::memcpy(inverse,
                    host.node_get_inverse_world_matrix_00b6e0d0(
                        at<void*>(instance_of(at<void*>(controller, 0x1C)), 0xC)),
                    sizeof inverse);
        void* const gun_instance = instance_of(gun);
        if (gun_instance == nullptr || at<void*>(gun_instance, 0xC) == nullptr) continue;
        const char* const gun_name = host.object_vcall_10_name(gun);
        void* node = nullptr;
        with_native_string(gun_name, static_cast<std::uint32_t>(std::strlen(gun_name)), host,
                           [&](const void* name) { node = host.node_find_child_by_name_00b6e750(root, name, 1); });
        float node_world[16];
        refresh_and_copy_world(node, node_world, host);
        float local[3];
        host.vector3_transform_affine_point_004142e0(&node_world[12], local, inverse);   // 00936AF8
        const float split = at<float>(at<void*>(at<void*>(controller, 0x1C), 0x538), 0x580);
        const bool to_front = local[2] > split;                                        // FCOMIP, JBE
        with_native_string(to_front ? "ELELLYE" : "HATULLYA", to_front ? 7u : 8u, host, [&](const void* name) {
            host.node_set_parent_00b6e680(node, host.node_find_child_by_name_00b6e750(root, name, 1));
        });
        if (host.node_vcall_0c_is_type(node, host.group_current_type_00b8e600())) {
            host.group_enable_automatic_world_sphere_00b8f0f0(node);
        }
    }

    at<void*>(controller, 0x374) = nullptr;                                            // 00936C39
    at<void*>(controller, 0x370) = nullptr;
    host.mission_entity_kill_00926d90(at<void*>(controller, 0x1C), 1);
    void* const body = at<void*>(controller, 0x2C);
    if (body != nullptr) {
        for (void* shape = host.body_first_shape_00c31dc0(body); shape != nullptr; shape = at<void*>(shape, 0x208)) {
            host.shape_collide_mask_assign_00c48020(shape, 0);
            host.shape_group_assign_00c48050(shape, 0);
        }
    }

    void* const owner = at<void*>(controller, 0x1C);                                   // 00936C7A
    float* const world = &at<float>(owner, 0xCC);
    if (at<std::uint8_t>(owner, 0xC8) == 0) {
        if (at<void*>(owner, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(at<void*>(owner, 0x3C));
        const float* source_matrix = &at<float>(owner, 0x74);
        float product[16];                                                             // [F+78h]
        if (at<void*>(owner, 0x3C) != nullptr) {
            source_matrix = host.matrix_multiply_4x4_00413920(&at<float>(owner, 0x74), product,
                                                              &at<float>(at<void*>(owner, 0x3C), 0xCC));
        }
        for (int k = 0; k < 16; ++k) world[k] = source_matrix[k];
        at<std::uint8_t>(owner, 0xC8) = 1;
        at<std::uint8_t>(owner, 0x10C) = 0;
    }
    void* const manager = at<void*>(host.game_global_00e188a8(), 0x19EC);
    void* const definition = retain_reference(at<void*>(host.game_settings_00424c40(), 0x3A8), host);
    void* effect = nullptr;                                                            // [F+14h]
    void** const created = static_cast<void**>(
        host.point_effect_create_from_position_008685e0(&effect, manager, definition, &world[12], 0, 0));
    auto* const spawned = static_cast<std::uint8_t*>(*created);                        // read before the release
    release_reference(effect, host);
    at<std::uint8_t>(spawned, 0x9) = 1;                                                // 00936D94
    if (at<void*>(interior, 0x4) != nullptr) host.crt_free_00bf65ac(at<void*>(interior, 0x4));
}

}  // namespace bsp
