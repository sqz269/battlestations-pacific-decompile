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

}  // namespace bsp
