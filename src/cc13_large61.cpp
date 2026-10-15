// cc13_large61: large segment 61 functions. See include/bsp/cc13_large61.hpp and
// docs/CC13_LARGE61.md. Descriptive names are hypotheses, not recovered symbols.
#include "bsp/cc13_large61.hpp"

#include <cstring>

namespace bsp::cc13_large61 {

namespace {

constexpr std::uint32_t kVectorSize00415450 = 0x00415450;
constexpr std::uint32_t kVectorSize009413B0 = 0x009413B0;

std::uint32_t* first_of(Raw vector) { return static_cast<std::uint32_t*>(vector.ptr(kVecFirst)); }

// The MSVC9 checked `vector[i]` test the native inlines: first == null or i >= size
// reports and carries on.
void check_index(Raw vector, std::uint32_t i, Large61Host& host) {
    const auto first = reinterpret_cast<std::uintptr_t>(vector.ptr(kVecFirst));
    if (first == 0 ||
        i >= static_cast<std::uint32_t>(
                 static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(vector.ptr(kVecLast)) - first) >> 2)) {
        host.checked_iterator_report_00bf6713();
    }
}

// The `first != null && (last - first) >> 2 != 0` guard the native inlines.
std::uint32_t inline_size(Raw vector) {
    const auto first = reinterpret_cast<std::uintptr_t>(vector.ptr(kVecFirst));
    if (first == 0) return 0;
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(vector.ptr(kVecLast)) - first) >> 2);
}

}  // namespace

// 00947BC0. ESP+24h..+3Ch hold a0..a6. The EH state (3 after the vector members are
// zeroed) that would destroy the three vectors on a throw from 00492210 is not modelled.
Raw spawn_message_construct_00947bc0(Raw message, std::uint32_t a0, Raw a1, std::uint32_t a2,
                                     void* a3, Raw a4, Raw a5, void* a6, Large61Host& host) {
    host.session_message_construct_base_0075b430(message, kSpawnMessageType);  // 00947BE2
    message.at<std::uint32_t>(0x04) = 1;
    message.at<std::uint32_t>(0x00) = kSpawnMessageVtable;
    constexpr std::size_t kVectorWords[] = {0x28, 0x2c, 0x30, 0x38, 0x3c, 0x40, 0x48, 0x4c, 0x50};
    for (std::size_t off : kVectorWords) {
        message.at<std::uint32_t>(off) = 0;  // 00947BFB..00947C16: first/last/end of the vectors
    }
    message.at<std::uint32_t>(0x18) = a0;
    const std::uint16_t a3_id = a3 == nullptr ? 0 : Raw{static_cast<std::uint8_t*>(a3)}.at<std::uint16_t>(0x174);
    message.at<std::uint32_t>(0x1c) = a2;
    message.at<std::uint16_t>(0x20) = a3_id;
    message.at<std::uint32_t>(0x58) = 0;
    message.at<std::uint8_t>(0x56) = a6 != nullptr ? 1 : 0;
    if (a6 != nullptr) {
        message.at<std::uint16_t>(0x54) = Raw{static_cast<std::uint8_t*>(a6)}.at<std::uint16_t>(0x174);
    }

    // 00947C61: a4 -> +34h, dword copy.
    Raw copy4{message.field(0x34)};
    if (const std::uint32_t n = inline_size(a4); n != 0) {
        host.dword_vector_resize_00492210(copy4, n, 0);  // 00947C7E
        for (std::uint32_t i = 0; i < host.vector_size(kVectorSize00415450, a4); ++i) {
            check_index(a4, i, host);
            check_index(copy4, i, host);
            first_of(copy4)[i] = first_of(a4)[i];
        }
    }

    // 00947CEF: a1 -> +24h, each element mapped through 0095B9C0.
    Raw ids{message.field(0x24)};
    if (const std::uint32_t n = inline_size(a1); n != 0) {
        host.dword_vector_resize_00492210(ids, n, 0);  // 00947D11
        for (std::uint32_t i = 0; i < host.vector_size(kVectorSize009413B0, a1); ++i) {
            check_index(a1, i, host);
            check_index(ids, i, host);
            std::uint32_t* slot = first_of(ids) + i;  // 00947D64..00947D71, before the call
            *slot = host.vehicle_class_registry_id_0095b9c0(
                reinterpret_cast<void*>(static_cast<std::uintptr_t>(first_of(a1)[i])));
        }
    }

    // 00947D8E: a5 -> +44h. The guard tests a4, not a5 (00947D8E reloads [ESP+34h]); the
    // resize count is a5's size, 0 when a5 has no storage.
    Raw copy5{message.field(0x44)};
    if (inline_size(a4) != 0) {
        host.dword_vector_resize_00492210(copy5, inline_size(a5), 0);  // 00947DC7
        for (std::uint32_t i = 0; i < host.vector_size(kVectorSize00415450, a5); ++i) {
            check_index(a5, i, host);
            check_index(copy5, i, host);
            first_of(copy5)[i] = first_of(a5)[i];
        }
    }
    return message;
}

// 00945450 SpawnLight. Frame: position at ESP+34h, rotation copy at ESP+10h, the
// translation matrix at ESP+40h, Rx/Ry/Rz copies at ESP+A8h/+128h/+E8h, products at
// ESP+268h, +1E8h and +168h (the last is what vtable+88h receives).
std::int32_t lua_spawn_light_00945450(void* lua_state, const Large61Globals& globals,
                                      Large61Host& host) {
    // 00945474: the one-time function-static block. Nothing reads 00F89B54 afterwards here.
    if ((*globals.luakod_guard_00f89b58 & 1) == 0) {
        *globals.luakod_guard_00f89b58 |= 1;
        std::uint32_t temp[2] = {0, 0};  // NativeString {size, data}
        host.native_string_assign_0041e870(temp, "luakod");  // 00945496, string 00D11294
        *globals.luakod_static_00f89b54 = 0;
        if (temp[1] != 0) {
            host.sized_pool_return_00419cc0_00bd1510(reinterpret_cast<void*>(static_cast<std::uintptr_t>(temp[1])),
                                                     temp[0] + 1, 1);  // 009454B4, 009454BB
        }
    }

    void* owner = host.lua_state_owner_construct_borrowed_00b66c00(lua_state);  // 009454D3
    void* frame = host.lua_open_call_frame_00b679b0(owner);                    // 009454EF

    float position[3];
    void* arg1 = host.lua_argument_at_00b677e0(frame, 1);  // 0094550E
    host.lua_object_read_vector3_00888760(position, arg1);   // 00945521
    host.lua_object_destroy_00b67700(arg1);                  // 00945534

    const float* rotation_source = globals.zero_vector_00f87574;  // 00945579
    void* arg2 = nullptr;
    float rotation_read[3];
    if (host.lua_frame_argument_count_00b663f0(frame) >= 3) {  // 0094553D, 00945545 JL
        arg2 = host.lua_argument_at_00b677e0(frame, 2);       // 00945554
        rotation_source = host.lua_object_read_vector3_00888760(rotation_read, arg2);  // 00945572
    }
    float rotation[3] = {rotation_source[0], rotation_source[1], rotation_source[2]};
    if (arg2 != nullptr) host.lua_object_destroy_00b67700(arg2);  // 009455B3 (flag bit at ESP+0Ch)

    // 009455BC..009455D3: the second name is argument 3 when there are at least four, else 0.
    const std::int32_t second_index = host.lua_frame_argument_count_00b663f0(frame) < 4 ? 0 : 3;
    void* second_arg = host.lua_argument_at_00b677e0(frame, second_index);  // 009455DC
    void* name_arg = host.lua_argument_at_00b677e0(frame, 0);               // 009455F8
    const char* second_name = host.lua_object_get_string_00b662b0(second_arg);  // 0094560B
    const char* name = host.lua_object_get_string_00b662b0(name_arg);          // 00945613
    void* entity = host.game_spawn_entity_by_name_004c6ba0(globals.game_00e188a8, name,
                                                             second_name, 0);  // 0094561F
    host.lua_object_destroy_00b67700(name_arg);    // 00945635
    host.lua_object_destroy_00b67700(second_arg);  // 00945648

    // 0094564D..009456DD: identity with the position as the translation row.
    float translation[16] = {1.0f, 0.0f, 0.0f, 0.0f,  // 00D7A24C 1.0f
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             position[0], position[1], position[2], 1.0f};
    constexpr float kNegativeZero = -0.0f;  // 00D7A208, so each angle is the negated component
    float scratch_x[16], scratch_y[16], scratch_z[16];
    float rx[16], ry[16], rz[16];
    float angle_x = kNegativeZero - rotation[0];
    host.matrix_copy_004134f0(rx, host.matrix_rotation_x_00b64640(scratch_x, &angle_x));  // 009456E9, 009456F6
    float angle_y = kNegativeZero - rotation[1];
    host.matrix_copy_004134f0(ry, host.matrix_rotation_y_00b646e0(scratch_y, &angle_y));  // 0094571A, 00945727
    float angle_z = kNegativeZero - rotation[2];
    host.matrix_copy_004134f0(rz, host.matrix_rotation_z_00b64780(scratch_z, &angle_z));  // 0094574B, 00945758

    // 00945791..0094579F: ((Rz * Ry) * Rx) * T.
    float zy[16], zyx[16], world[16];
    float* p = host.matrix_multiply_00413920(rz, zy, ry);
    p = host.matrix_multiply_00413920(p, zyx, rx);
    host.matrix_multiply_00413920(p, world, translation);

    // 009457A4..009457C2: no null test of the spawned entity.
    host.entity_vcall_88_set_matrix(entity, world);
    host.entity_vcall_d8(entity);
    host.scene_resolve_deferred_references_0046aab0(globals.scene_database_00e18680);  // 009457CA

    const std::int32_t results = host.lua_frame_result_count_00b66400(frame);  // 009457D3
    host.lua_object_destroy_00b67700(frame);              // 009457E6
    host.lua_state_owner_close_00b669a0(owner);           // 009457FD
    return results;
}

// 0094BFF0. Frame (after the four register pushes): +10h NativeString {size, data} for the
// enqueue, +18h ECX, +1Ch member vector, +34h member (10h bytes: +0Ch the property bag), +44h
// seven limits, +60h the start matrix, +A0h the 004C7AA0 result, +C8h the flight matrix, +108h
// the reference frame. No null test of a1, a5 or the 008F2260 result.
void powerup_spawn_request_0094bff0(Raw manager, std::int32_t player_slot, void* vehicle_class, void* unit,
                                    std::uint32_t equipment, std::uint32_t reload_enabled, void* attack_target,
                                    std::uint32_t a6, std::uint32_t a7, std::uint32_t a8,
                                    const Large61Globals& globals, Large61Host& host) {
    Raw game{static_cast<std::uint8_t*>(globals.game_00e188a8)};
    // 0094C020..0094C02D: the party of the player's record.
    Raw player{static_cast<std::uint8_t*>(game.ptr(0x18cc + static_cast<std::size_t>(player_slot) * 4))};
    const std::uint32_t party = player.at<std::uint32_t>(0x28);
    Raw unit_raw{static_cast<std::uint8_t*>(unit)};
    if (unit_raw.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);  // 0094C03D

    alignas(4) std::uint8_t query[0x28];
    host.game_query_004c7aa0(globals.game_00e188a8, query, reinterpret_cast<const float*>(unit_raw.field(0xfc)),
                             500.0f /* 00CE397C */, -1);  // 0094C063
    const float* query_f = reinterpret_cast<const float*>(query);

    alignas(4) std::uint8_t members_storage[0x10] = {};  // +1Ch: allocator byte, first, last, end
    Raw members{members_storage};

    // 0094C07E..0094C09A: the NativeString argument is built in its own stack slot and the
    // member constructor takes it by value.
    Raw vehicle{static_cast<std::uint8_t*>(vehicle_class)};
    std::uint32_t name_arg[2] = {0, 0};
    host.native_string_assign_0041e870(name_arg, static_cast<const char*>(vehicle.ptr(0x54)));  // 0094C08F
    alignas(4) std::uint8_t member_storage[0x10];
    Raw member{member_storage};
    host.spawn_member_construct_00944130(member_storage, party, vehicle_class, name_arg[0], name_arg[1]);  // 0094C09A
    void* bag = member.ptr(0x0c);

    host.property_set_field0c_008ef7c0(host.property_bag_find_008f2260(bag, "OwnerPlayer"), 8);  // 0094C0B4, 0094C0BB
    if (host.vehicle_class_vcall_18(vehicle_class, 0x0f)) {  // 0094C0C9: class 0Fh
        host.property_bag_add_int_008f3710(bag, "WingCount", 3);                                  // 0094C0D8
        host.property_bag_add_float_008f3770(bag, "VelocitySI", vehicle.at<float>(0x18c));        // 0094C0EE
        host.property_bag_add_int_008f3710(bag, "Equipment", static_cast<std::int32_t>(equipment));  // 0094C102
        host.property_bag_add_bool_008f3940(bag, "ReloadEnabled", reload_enabled);                // 0094C116
    }
    host.property_bag_add_int_008f3710(
        bag, "AutoAttackTarget",
        Raw{static_cast<std::uint8_t*>(attack_target)}.at<std::uint16_t>(0x174));  // 0094C134
    host.spawn_member_vector_push_back_00949610(members, member_storage);         // 0094C142

    // 0094C147..0094C1E8: identity with the query's first point (out+10h) as the translation.
    float start[16] = {1.0f, 0.0f, 0.0f, 0.0f,
                       0.0f, 1.0f, 0.0f, 0.0f,
                       0.0f, 0.0f, 1.0f, 0.0f,
                       query_f[4], query_f[5], query_f[6], 1.0f};
    alignas(4) std::uint8_t ref_frame[0x58];  // +108h..+15Fh, the room the frame leaves
    float flight[16];
    const float* copied = host.spawn_ref_frame_get_matrix_008f8680(
        host.spawn_ref_frame_construct_008f84d0(ref_frame, start));  // 0094C1F1, 0094C200
    for (int i = 0; i < 16; ++i) flight[i] = copied[i];               // 0094C213 REP MOVSD
    host.callback_owner_destroy_00695870(ref_frame);                  // 0094C224

    // 0094C229..0094C291: forward row = the query's second point (out+1Ch) - translation; the
    // translation's y becomes 1000 (00CE3804) after it is read.
    const float dx = query_f[7] - flight[12];
    const float dy = query_f[8] - flight[13];
    flight[13] = 1000.0f;
    const float dz = query_f[9] - flight[14];
    flight[8] = dx;
    flight[9] = dy;
    flight[10] = dz;
    host.matrix_orthonormalize_rows_0085dc80(flight);  // 0094C298

    std::uint32_t name[2] = {0, 0};                    // +10h
    host.native_string_resize_0041dd40(name, 0, 1);    // 0094C2AC
    const std::uint32_t name_size = name[0];
    void* const name_data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(name[1]));
    if (name_data != nullptr) host.memcpy_00bf7680(name_data, "", name_size + 1);  // 0094C2C7, 00CE3A0C

    void* frame_handle = host.spawn_ref_frame_construct_008f84d0(ref_frame, flight);  // 0094C2E6
    const float limits[7] = {100.0f,     // 00CE3D08
                             2000.0f,    // 00CFFD60
                             100.0f,     // 00CE3D08
                             500.0f,     // 00CE397C
                             0.0f,
                             10000.0f,   // 00CE3D64
                             4000000.0f  // 00D19914
    };
    std::uint32_t id = *globals.spawn_request_counter_00e0cf74;  // 0094C2F3
    if (id > 16000) id = 1;                                       // 0094C2F9 JBE (unsigned)
    *globals.spawn_request_counter_00e0cf74 = id + 1;
    host.spawn_manager_enqueue_00949530(manager, members, id, frame_handle,
                                        -3.14159274f /* 00CE684C */, 3.14159274f /* 00D7A264 */,
                                        200.0f /* 00D19908 */, 2500.0f /* 00D1990C */, party, player_slot,
                                        a6, limits, name, a7, a8);  // 0094C3C6

    host.callback_owner_destroy_00695870(ref_frame);  // 0094C3DA
    if (name_data != nullptr) host.sized_pool_return_00419cc0_00bd1510(name_data, name_size + 1, 1);  // 0094C3F9
    if (member.at<std::uint32_t>(0x08) != 0) {  // 0094C3FE: the member's NativeString {+4 size, +8 data}
        host.sized_pool_return_00419cc0_00bd1510(member.ptr(0x08), member.at<std::uint32_t>(0x04) + 1, 1);  // 0094C41F
    }
    if (members.ptr(kVecFirst) != nullptr) {  // 0094C424
        host.member_destroy_range_00946d70(members.ptr(kVecFirst), members.ptr(kVecLast), members.p,
                                           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(manager.p)));  // 0094C445, tag = [+18h]
        host.operator_delete_00bf65ac(members.ptr(kVecFirst));  // 0094C44F
    }
}

namespace {

// The inline two-bank handle lookup (0094B68C..0094B6CB, 0094BB4F..0094BB7B).
void* entity_from_id(std::uint16_t id, const Large61Globals& g) {
    const std::int32_t value = id;
    if (value < *g.handle_split_00f89a10) {
        return *reinterpret_cast<void* const*>(*g.bank0_records_00f89a54 +
                                               static_cast<std::uint32_t>(value - *g.bank0_base_00f89a0c) * 0x10 + 0x0c);
    }
    return *reinterpret_cast<void* const*>(*g.bank1_records_00f89aa8 +
                                           static_cast<std::uint32_t>(value - *g.bank1_base_00f89a60) * 0x10 + 0x0c);
}

// The by-value NativeString each member constructor receives, built inline: {0, 0},
// 0041DD40(strlen, 1), then memcpy(data, text, size + 1) when data is non-null.
void build_name_argument(std::uint32_t (&string)[2], const char* text, Large61Host& host) {
    string[0] = 0;
    string[1] = 0;
    host.native_string_resize_0041dd40(string, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (string[1] != 0) {
        host.memcpy_00bf7680(reinterpret_cast<void*>(static_cast<std::uintptr_t>(string[1])), text, string[0] + 1);
    }
}

// (float)entity+33Ch over (double)114.6f and -114.6f (00D199E0, 00D199D8), each through an
// x87 divide stored to float (0094B8BE..0094B942, 0094BF1F..0094BF41).
constexpr double kArcDivisor = static_cast<double>(114.6f);

}  // namespace

// 0094B600. Frame offsets are after the four register pushes. Not modelled: the EH states
// (0..7) that destroy the member vector, the members and the strings on a throw.
void spawn_manager_handle_message_0094b600(Raw manager, Raw message, std::uint32_t a1,
                                           const Large61Globals& globals, Large61Host& host) {
    Raw game{static_cast<std::uint8_t*>(globals.game_00e188a8)};
    Raw ids{message.field(0x24)};
    Raw equipment{message.field(0x34)};
    Raw wing_counts{message.field(0x44)};
    // 0094B631..0094B646: a loop that only counts up to the size of +24h; no effect.
    const std::uint32_t slot = message.at<std::uint32_t>(0x18);  // +20h
    alignas(4) std::uint8_t member_storage[0x10];
    Raw member{member_storage};
    alignas(4) std::uint8_t ref_frame[0x58];

    if (game.at<std::int32_t>(0x1fe4) == 2) {
        if (inline_size(ids) == 0) {
            host.spawn_manager_remove_request_00943e40(manager, message.at<std::uint32_t>(0x58));  // 0094B678
            return;
        }
        Raw player{static_cast<std::uint8_t*>(game.ptr(0x18cc + slot * 4))};
        const std::uint32_t party = player.at<std::uint32_t>(0x28);                  // +24h
        void* entity = entity_from_id(message.at<std::uint16_t>(0x20), globals);   // +14h
        alignas(4) std::uint8_t members_storage[0x10] = {};                           // +3Ch
        Raw members{members_storage};
        for (std::uint32_t i = 0; i < inline_size(ids); ++i) {  // 0094B6F0
            check_index(ids, i, host);
            void* cls = host.vehicle_class_get_or_create_00964790(first_of(ids)[i], true);  // 0094B731
            std::uint32_t name[2];
            build_name_argument(name, static_cast<const char*>(Raw{static_cast<std::uint8_t*>(cls)}.ptr(0x54)), host);
            host.spawn_member_construct_00944130(member_storage, party, cls, name[0], name[1]);  // 0094B78C, member at +28h
            check_index(equipment, i, host);
            host.property_bag_add_int_008f3710(member.ptr(0x0c), "Equipment",
                                               static_cast<std::int32_t>(first_of(equipment)[i]));  // 0094B7CB
            host.spawn_member_vector_push_back_00949610(members, member_storage);                 // 0094B7D9
            if (member.ptr(0x08) != nullptr) {
                host.sized_pool_return_00419cc0_00bd1510(member.ptr(0x08), member.at<std::uint32_t>(0x04) + 1, 1);  // 0094B800
            }
        }
        std::uint32_t empty[2] = {0, 0};                    // +28h
        host.native_string_resize_0041dd40(empty, 0, 1);    // 0094B824
        const std::uint32_t empty_size = empty[0];
        void* const empty_data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(empty[1]));
        if (empty_data != nullptr) host.memcpy_00bf7680(empty_data, "", empty_size + 1);  // 0094B83F, 00CE3A0C
        void* frame_handle = host.spawn_ref_frame_construct_from_entity_008f8530(ref_frame, entity);  // 0094B85B, +84h
        const float arc = static_cast<float>(Raw{static_cast<std::uint8_t*>(entity)}.at<std::int32_t>(0x33c));
        const float limits[7] = {100.0f, 2000.0f, 100.0f, 500.0f, 0.0f, 10000.0f, 4000000.0f};  // +68h, as in 0094BFF0
        const float half_arc_hi = static_cast<float>(static_cast<double>(arc) / kArcDivisor);
        const float half_arc_lo = static_cast<float>(static_cast<double>(arc) / -kArcDivisor);
        host.spawn_manager_enqueue_00949530(manager, members, message.at<std::uint32_t>(0x58), frame_handle,
                                            half_arc_lo, half_arc_hi, 200.0f /* 00D19908 */, 2500.0f /* 00D1990C */,
                                            party, static_cast<std::int32_t>(slot), a1, limits, empty, 0, 0);  // 0094B94C
        host.callback_owner_destroy_00695870(ref_frame);  // 0094B960
        if (empty_data != nullptr) host.sized_pool_return_00419cc0_00bd1510(empty_data, empty_size + 1, 1);  // 0094B97F
        if (members.ptr(kVecFirst) != nullptr) {
            // 0094B99B: the tag is the stale dword at +18h (the by-value string slot address of the
            // last iteration); 00946D70 does not interpret it, so 0 stands in for it here.
            host.member_destroy_range_00946d70(members.ptr(kVecFirst), members.ptr(kVecLast), members.p, 0);  // 0094B9A9
            host.operator_delete_00bf65ac(members.ptr(kVecFirst));  // 0094BFCB
        }
        return;
    }

    // 0094B9B8: the request id.
    std::uint32_t id = *globals.spawn_request_counter_00e0cf74;
    if (id > 16000) id = 1;
    *globals.spawn_request_counter_00e0cf74 = id + 1;
    message.at<std::uint32_t>(0x58) = id;

    if (game.at<std::int32_t>(0x1fe4) != 0 && host.game_effective_mode_004bca50() > 3) {  // 0094B9EE
        if (slot <= 7) {  // 0094B9FC JA (unsigned)
            if (host.spawn_manager_player_usage_00946970(manager, slot) > 0.0) return;  // 0094BA0D..0094BA1A
            Raw world{static_cast<std::uint8_t*>(game.ptr(0x19cc))};
            for (Raw node{static_cast<std::uint8_t*>(world.ptr(0x34))}; node.p != nullptr;
                 node = Raw{static_cast<std::uint8_t*>(node.ptr(0x04))}) {  // 0094BA2B
                void* unit = node.ptr(0x08);
                Raw u{static_cast<std::uint8_t*>(unit)};
                if (u.at<std::uint8_t>(0x5c) == 0 || u.at<std::uint8_t>(0x5d) != 0 || u.at<std::uint8_t>(0x60) != 0 ||
                    u.at<std::uint8_t>(0x5e) != 0) {
                    continue;
                }
                if (slot != host.unit_loss_counting_slot_009ffd20(unit)) continue;  // 0094BA53
                if (host.entity_vcall_5c(unit, 0x18) &&
                    Raw{static_cast<std::uint8_t*>(u.ptr(0x3d0))}.at<std::int32_t>(0x900) != 7) {
                    continue;  // 0094BA67..0094BA79
                }
                if (!host.entity_vcall_5c(unit, 0x0f)) return;  // 0094BA84..0094BA88
            }
        }
    } else {
        // 0094BA97: the summed class cost (+124h, int) against the player's support points.
        float total = 0.0f;
        for (std::uint32_t i = 0; i < inline_size(ids); ++i) {
            check_index(ids, i, host);
            void* cls = host.vehicle_class_get_or_create_00964790(first_of(ids)[i], true);  // 0094BAD4
            total = static_cast<float>(
                static_cast<double>(Raw{static_cast<std::uint8_t*>(cls)}.at<std::int32_t>(0x124)) + total);  // FILD, FADD, FSTP
        }
        const double cost = total;  // +28h as a double
        if (cost > host.player_support_points_00946f00(slot)) return;  // 0094BAF8..0094BB05
    }

    // 0094BB0B
    Raw player{static_cast<std::uint8_t*>(game.ptr(0x18cc + slot * 4))};
    const std::uint32_t party = player.at<std::uint32_t>(0x28);  // +1Ch
    Raw session{game.field(0x1ef0)};
    if (session.at<std::int32_t>(0xf4) == 1 && session.at<std::uint8_t>(0x29c) == 0 && player.ptr(0x50) != nullptr) {
        host.session_send_to_nonlocal_peer_00770b50(session.p, player.ptr(0x50), message);  // 0094BB46
    }
    void* entity = entity_from_id(message.at<std::uint16_t>(0x20), globals);  // +24h
    alignas(4) std::uint8_t members_storage[0x10] = {};                         // +58h
    Raw members{members_storage};
    float widest = 0.0f;                                                        // +10h
    for (std::uint32_t i = 0; i < inline_size(ids); ++i) {                     // 0094BBB0
        check_index(ids, i, host);
        void* cls = host.vehicle_class_get_or_create_00964790(first_of(ids)[i], true);  // 0094BBF1
        Raw c{static_cast<std::uint8_t*>(cls)};
        std::uint32_t name[2];
        build_name_argument(name, static_cast<const char*>(c.ptr(0x54)), host);
        host.spawn_member_construct_00944130(member_storage, party, cls, name[0], name[1]);  // 0094BC4C, member at +3Ch
        void* bag = member.ptr(0x0c);
        host.property_set_field0c_008ef7c0(host.property_bag_find_008f2260(bag, "OwnerPlayer"), slot);  // 0094BC69
        if (message.at<std::int32_t>(0x1c) >= 0) {  // 0094BC75 JL
            void* skill = host.property_bag_find_008f2260(bag, "Skill");  // 0094BC82, 00CF8838
            host.property_set_field0c_008ef7c0(skill, message.at<std::uint32_t>(0x1c));
        }
        if (host.vehicle_class_vcall_18(cls, 0x0f)) {  // 0094BC9C
            check_index(wing_counts, i, host);
            host.property_bag_add_int_008f3710(bag, "WingCount", static_cast<std::int32_t>(first_of(wing_counts)[i]));  // 0094BCDF
            host.property_bag_add_float_008f3770(bag, "VelocitySI", c.at<float>(0x18c));  // 0094BCF5
            if (inline_size(equipment) != 0) {
                check_index(equipment, i, host);
                host.property_bag_add_int_008f3710(bag, "Equipment", static_cast<std::int32_t>(first_of(equipment)[i]));  // 0094BD71
            } else {
                const std::int32_t count = host.vehicle_class_equipment_count_00951f10(cls);  // 0094BD36
                host.property_bag_add_int_008f3710(bag, "Equipment", count > 0 ? 1 : 0);    // SETG, 0094BD71
            }
        } else if (host.vehicle_class_vcall_18(cls, 6)) {  // 0094BD54
            host.property_bag_add_bool_008f3940(bag, "TorpedoDirector", 1);  // 0094BD63, 00D09934
            host.property_bag_add_int_008f3710(bag, "CamoColor", 1);        // 0094BD71, 00D09980
        }
        host.property_bag_add_float_008f3770(bag, "ResourceUsage",
                                             static_cast<float>(c.at<std::int32_t>(0x124)));  // 0094BD8B, CVTSI2SS
        if (message.at<std::uint8_t>(0x56) != 0) {
            host.property_bag_add_int_008f3710(bag, "AutoAttackTarget", message.at<std::uint16_t>(0x54));  // 0094BDA2
        }
        host.spawn_member_vector_push_back_00949610(members, member_storage);  // 0094BDB0
        if (c.at<float>(0xa0) > widest) widest = c.at<float>(0xa0);           // 0094BDC5 FCOMIP/JA
        if (member.ptr(0x08) != nullptr) {
            host.sized_pool_return_00419cc0_00bd1510(member.ptr(0x08), member.at<std::uint32_t>(0x04) + 1, 1);  // 0094BDFD
        }
    }

    // 0094BE12..0094BE78: the area limits at +3Ch.
    const float radius = static_cast<float>(static_cast<double>(widest) * 1.5);  // 00CE3D78
    const float radius_sq = static_cast<float>(static_cast<double>(radius) * radius);
    const float floor_radius = 50.0 > radius ? 50.0f : radius;  // 00CE3938 double, 00CEB4D4 float
    const float limits[7] = {radius, radius, floor_radius, floor_radius, 0.0f, radius_sq, radius_sq};

    std::uint32_t empty[2] = {0, 0};                    // +28h
    host.native_string_resize_0041dd40(empty, 0, 1);    // 0094BE86
    const std::uint32_t empty_size = empty[0];
    void* const empty_data = reinterpret_cast<void*>(static_cast<std::uintptr_t>(empty[1]));
    if (empty_data != nullptr) host.memcpy_00bf7680(empty_data, "", empty_size + 1);  // 0094BEA1
    void* frame_handle = host.spawn_ref_frame_construct_from_entity_008f8530(ref_frame, entity);  // 0094BEBD, +84h
    const float arc = static_cast<float>(Raw{static_cast<std::uint8_t*>(entity)}.at<std::int32_t>(0x33c));
    const float half_arc_hi = static_cast<float>(static_cast<double>(arc) / kArcDivisor);
    const float half_arc_lo = static_cast<float>(static_cast<double>(arc) / -kArcDivisor);
    host.spawn_manager_enqueue_00949530(manager, members, message.at<std::uint32_t>(0x58), frame_handle,
                                        half_arc_lo, half_arc_hi, 200.0f, 2500.0f, party,
                                        static_cast<std::int32_t>(slot), a1, limits, empty, 0, 0);  // 0094BF47
    host.callback_owner_destroy_00695870(ref_frame);  // 0094BF5B
    if (empty_data != nullptr) host.sized_pool_return_00419cc0_00bd1510(empty_data, empty_size + 1, 1);  // 0094BF7A
    // 0094BF7F..0094BFCB: the members' strings released inline, then the storage.
    auto* first = static_cast<std::uint8_t*>(members.ptr(kVecFirst));
    if (first == nullptr) return;
    auto* last = static_cast<std::uint8_t*>(members.ptr(kVecLast));
    for (std::uint8_t* element = first; element != last; element += 0x10) {
        Raw e{element};
        if (e.ptr(0x08) != nullptr) {
            host.sized_pool_return_00419cc0_00bd1510(e.ptr(0x08), e.at<std::uint32_t>(0x04) + 1, 1);  // 0094BFB7
        }
    }
    host.operator_delete_00bf65ac(first);  // 0094BFCB
}

namespace {

// A NativeString temporary built inline: {0, 0}, 0041DD40(length, 1), then
// memcpy(data, text, size + 1) when data is non-null. `length` is the image's constant.
struct TempString {
    std::uint32_t s[2] = {0, 0};  // {size, data}
    void* data() const { return reinterpret_cast<void*>(static_cast<std::uintptr_t>(s[1])); }
};

void temp_build(TempString& t, std::uint32_t length, const char* text, Large61Host& host) {
    t.s[0] = 0;
    t.s[1] = 0;
    host.native_string_resize_0041dd40(t.s, length, 1);
    if (t.s[1] != 0) host.memcpy_00bf7680(t.data(), text, t.s[0] + 1);
}

void temp_release(TempString& t, Large61Host& host) {
    if (t.s[1] != 0) host.sized_pool_return_00419cc0_00bd1510(t.data(), t.s[0] + 1, 1);
}

// The inline NativeString inequality of 0093D3B6..0093D3ED (and its two copies): an empty
// name equals only an empty text; an empty text never equals a non-empty name; otherwise
// _stricmp. The effect name is fetched afresh for each comparison.
bool effect_name_differs(void* material, const TempString& text, Large61Host& host) {
    const std::uint32_t* name = host.effect_name_00b172d0(Raw{static_cast<std::uint8_t*>(material)}.ptr(0x7c));
    if (name[0] == 0) return text.s[0] != 0;  // NEG/SBB
    if (text.s[0] == 0) return true;
    return host.stricmp_00bf7fbf(reinterpret_cast<const char*>(static_cast<std::uintptr_t>(name[1])),
                                 static_cast<const char*>(text.data())) != 0;
}

// Register one named parameter: build the name, call the registration, release the name.
template <class Register>
void register_named(std::uint32_t length, const char* text, Large61Host& host, Register reg) {
    TempString name;
    temp_build(name, length, text, host);
    reg(name.s);
    temp_release(name, host);
}

// 0093D6BA..0093D745 and 0093DCAA..0093DD35: the ocean bounds (game +711Ch, +7124h, +7128h,
// +7130h) as {width + 6000, depth + 6000, centre x, centre z}. Each difference or sum is
// stored to float first; the 6000 (00CF0A60) and 0.5 (00D7A280) are doubles applied on the
// x87 stack (DC C1 FADD ST1,ST0 and DC C9 FMUL ST1,ST0, read from the bytes).
void ocean_size(Raw game, float* out) {
    const float width = game.at<float>(0x7128) - game.at<float>(0x711c);
    out[0] = static_cast<float>(static_cast<double>(width) + 6000.0);
    const float depth = game.at<float>(0x7124) - game.at<float>(0x7130);
    out[1] = static_cast<float>(6000.0 + static_cast<double>(depth));
    const float sum_x = game.at<float>(0x7128) + game.at<float>(0x711c);
    out[2] = static_cast<float>(static_cast<double>(sum_x) * 0.5);
    const float sum_z = game.at<float>(0x7130) + game.at<float>(0x7124);
    out[3] = static_cast<float>(0.5 * static_cast<double>(sum_z));
}

}  // namespace

// 0093D2D0. The EH states (0..0Fh) that release the temporaries on a throw are not modelled.
void map_scene_setup_ocean_materials_0093d2d0(Raw scene, void* node, std::uint32_t flag,
                                              const OceanMaterialGlobals& globals, Large61Host& host) {
    Raw game{static_cast<std::uint8_t*>(globals.game_00e188a8)};
    const std::uint32_t type = host.generated_model_type_00b74310();  // 0093D306
    if (host.node_vcall_0c(node, type)) {                             // 0093D310
        const std::int32_t geometries = host.node_geometry_count_00b74650(node);  // 0093D31C
        for (std::int32_t g = 0; g < geometries; ++g) {
            void* geometry = host.node_geometry_00b74640(node, g);                 // 0093D34B
            const std::int32_t elements = host.geometry_element_count_00b72b40(geometry);  // 0093D359
            for (std::int32_t e = 0; e < elements; ++e) {
                void* material = Raw{static_cast<std::uint8_t*>(host.geometry_element_00b732c0(geometry, e))}.ptr(0x20);  // 0093D37B
                // 0093D383..0093D4DE: other = the effect is none of the three ocean shaders.
                TempString wave, color, shore;
                bool built_color = false, built_shore = false;
                temp_build(wave, 0x21, "shaderfx/ocean/oceanwavedamp.shfx", host);  // 00D197E0
                bool other = false;
                if (effect_name_differs(material, wave, host)) {
                    temp_build(color, 0x1e, "shaderfx/ocean/oceancolor.shfx", host);  // 00D197C0
                    built_color = true;
                    if (effect_name_differs(material, color, host)) {
                        temp_build(shore, 0x19, "shaderfx/ocean/shore.shfx", host);   // 00D197A4
                        built_shore = true;
                        other = effect_name_differs(material, shore, host);
                    }
                }
                if (built_shore) temp_release(shore, host);  // 0093D4E2
                if (built_color) temp_release(color, host);  // 0093D514
                temp_release(wave, host);                    // 0093D54B
                if (!other) continue;                        // 0093D57F

                if (static_cast<std::uint8_t>(flag) != 0) {  // 0093D589
                    TempString mesh;
                    temp_build(mesh, 0x1d, "bterrain_underwater_test.mshd", host);  // 00D19784
                    void* shader = host.shader_manager_vcall_48(globals.shader_manager_00f8d394, mesh.s);  // 0093D5E0
                    temp_release(mesh, host);
                    host.material_set_shader_00b19210(material, shader);  // 0093D611
                    register_named(0x13, "cSmoothMapZoomLevel", host, [&](const std::uint32_t* n) {
                        host.material_register_float_00b18b20(material, n, &scene.at<float>(0x14));  // 0093D675
                    });
                    if ((*globals.guard_00f89b20 & 1) == 0) *globals.guard_00f89b20 |= 1;  // 0093D6AA
                    ocean_size(game, globals.ocean_size_underwater_00f89b10);
                    register_named(10, "cOceanSize", host, [&](const std::uint32_t* n) {
                        host.material_register_float4_array_00b18ac0(material, n, globals.ocean_size_underwater_00f89b10, 1);  // 0093D787
                    });
                    if (shader != nullptr &&
                        host.interlocked_decrement_00ce2220(
                            reinterpret_cast<std::int32_t*>(static_cast<std::uint8_t*>(shader) + 4)) == 0) {  // 0093D7C2
                        host.object_vcall_00(shader);  // 0093D7D6
                    }
                    continue;
                }

                register_named(0x20, "cStaticShot_Size_OffsetX_OffsetY", host, [&](const std::uint32_t* n) {
                    host.material_register_float3_00b18ae0(material, n, globals.static_shot_00e1891c);  // 0093D825
                });
                register_named(0x2a, "cBasic___ShoreH_DirtH_SimpleDist_SimpleFog", host, [&](const std::uint32_t* n) {
                    host.material_register_float4_array_00b18ac0(material, n, globals.basic_00e18928, 1);  // 0093D89C
                });
                register_named(0x14, "cSnoise___Div_Max_UV", host, [&](const std::uint32_t* n) {
                    host.material_register_float3_00b18ae0(material, n, globals.snoise_00e18938);  // 0093D911
                });
                register_named(0x11, "cHnoise___Div_Max", host, [&](const std::uint32_t* n) {
                    host.material_register_float2_00b18b00(material, n, globals.hnoise_00e18944);  // 0093D986
                });
                if ((*globals.guard_00f89b20 & 2) == 0) {  // 0093D9B1
                    *globals.guard_00f89b20 |= 2;
                    globals.ground_00f89b04[0] = *globals.ground_seed_x_00e1894c;
                    globals.ground_00f89b04[2] = *globals.ground_seed_z_00e18954;
                }
                // 0093D9E1..0093DA03: +8 is scaled by 3.0 (00D7A2B0 double) on every pass; +4 = 1.
                globals.ground_00f89b04[2] = static_cast<float>(static_cast<double>(globals.ground_00f89b04[2]) * 3.0);
                globals.ground_00f89b04[1] = 1.0f;
                register_named(0x19, "cGround___slope_UV_maskUV", host, [&](const std::uint32_t* n) {
                    host.material_register_float3_00b18ae0(material, n, globals.ground_00f89b04);  // 0093DA4D
                });
                register_named(0x0e, "cUW___slope_UV", host, [&](const std::uint32_t* n) {
                    host.material_register_float2_00b18b00(material, n, globals.uw_slope_00e18958);  // 0093DAC5
                });
                register_named(9, "cUWColor0", host, [&](const std::uint32_t* n) {
                    host.material_register_float4_00b18aa0(material, n, globals.uw_color0_00e18960);  // 0093DB4C
                });
                register_named(9, "cUWColor1", host, [&](const std::uint32_t* n) {
                    host.material_register_float4_00b18aa0(material, n, globals.uw_color1_00e18970);  // 0093DBD9
                });
                register_named(0x0b, "cShadowFade", host, [&](const std::uint32_t* n) {
                    host.material_register_float_00b18b20(material, n, &scene.at<float>(0x18));  // 0093DC69
                });
                if ((*globals.guard_00f89b20 & 4) == 0) *globals.guard_00f89b20 |= 4;  // 0093DC9A
                ocean_size(game, globals.ocean_size_map_00f89af4);
                register_named(10, "cOceanSize", host, [&](const std::uint32_t* n) {
                    host.material_register_float4_array_00b18ac0(material, n, globals.ocean_size_map_00f89af4, 1);  // 0093DD77
                });
            }
        }
    }
    // 0093DDDB..0093DDF5: the children, through the sibling links.
    for (void* child = Raw{static_cast<std::uint8_t*>(node)}.ptr(0x34); child != nullptr;
         child = Raw{static_cast<std::uint8_t*>(child)}.ptr(0x3c)) {
        map_scene_setup_ocean_materials_0093d2d0(scene, child, flag, globals, host);
    }
}

// 0093FF10. Frame offsets (Lxx) are ESP-relative after the three register pushes (the frame
// is EBP-based with ESP aligned to 8). Options copies are 48h bytes: +04h volume, +0Ch/+10h
// min/max distance, +20h the Lin/Log byte, +3Ch the parameter array (data first).
void sound_overlay_draw_0093ff10(Raw overlay, std::uint32_t flag, const SoundOverlayGlobals& globals,
                                 Large61Host& host) {
    const bool enabled = static_cast<std::uint8_t>(flag) != 0;  // [EBP+8] byte
    host.array12_resize_0093f840(overlay.field(0x04), 0);       // 0093FF3E
    Raw manager{static_cast<std::uint8_t*>(*globals.sound_manager_slot_00f8bbd8)};  // EDI, L34

    // 0093FF4D..0093FFC8: copy the manager's sound pointers (+8Ch data, +90h count) into a
    // local dword array {data L4C, count L50, capacity L54}, growing it through 0093F610.
    alignas(4) std::uint8_t sounds_storage[0x0c] = {};
    Raw sounds{sounds_storage};
    {
        auto* source = static_cast<void**>(manager.ptr(0x8c));
        void** const source_end = source + manager.at<std::int32_t>(0x90);
        for (; source != source_end; ++source) {
            void* const element = *source;  // L2C
            if (sounds.at<std::int32_t>(4) == sounds.at<std::int32_t>(8)) {
                std::int32_t capacity = sounds.at<std::int32_t>(8) * 2;
                if (capacity <= 1) capacity = 1;
                host.array4_reserve_0093f610(sounds.p, capacity);  // 0093FF9C
            }
            void** slot = static_cast<void**>(sounds.ptr(0)) + sounds.at<std::int32_t>(4);
            if (slot != nullptr) *slot = element;
            sounds.at<std::int32_t>(4) += 1;
        }
    }
    host.qsort_s_00bfbf10(sounds.ptr(0), static_cast<std::uint32_t>(sounds.at<std::int32_t>(4)), 4,
                          0x0093F4B0, nullptr);                                        // 0093FFD9
    host.streamed_dialog_draw_debug_00a777a0(*globals.dialog_manager_slot_00f8bbcc);         // 0093FFE7
    const float listener[3] = {manager.at<float>(0xf4), manager.at<float>(0xf8), manager.at<float>(0xfc)};  // L6C

    void** const first = static_cast<void**>(sounds.ptr(0));
    void** const last = first + sounds.at<std::int32_t>(4);  // L48
    void* closest = nullptr;                                 // L44
    float closest_distance = 0.0f;                           // L2C
    float screen[2];                                         // L38
    // 00940038: the positioned sound whose label lands nearest the screen centre (512, 384),
    // within 200 pixels.
    for (void** it = first; it != last; ++it) {
        const float level = host.sound_vcall_1c(*it);  // 0094003F
        if (level == 0.0f) continue;                    // FUCOMIP, LAHF/TEST AH,44h/JNP
        if (*host.sound_sample_options_00a81860(Raw{static_cast<std::uint8_t*>(*it)}.ptr(0x4c)) != 1) continue;
        float position[3];                              // LD8
        host.sound_vcall_38(*it, position);             // 0094007B
        if (host.camera_project_0043a870(position, screen, 2, 1) != 7) continue;  // 0094008C
        float placed[2];                                // LCC
        const float* label = host.overlay_place_label_0093fa70(overlay.p, placed, screen[0], screen[1], *it);  // 009400C7
        screen[0] = label[0];
        screen[1] = label[1];
        const float dx = static_cast<float>(screen[0] - 512.0);  // 00D19878
        const float dy = static_cast<float>(screen[1] - 384.0);  // 00D19870
        const float dx2 = static_cast<float>(static_cast<double>(dx) * dx);
        const float dy2 = static_cast<float>(static_cast<double>(dy) * dy);
        const float sum = static_cast<float>(static_cast<double>(dy2) + dx2);
        const float distance = static_cast<float>(host.sqrt_00bf7030(sum));  // 0094011D
        if ((closest == nullptr || closest_distance > distance) && 200.0 > distance) {  // 00CE4D70
            closest = *it;
            closest_distance = distance;
        }
    }

    for (void** it = first; it != last; ++it) {  // 00940182, iterator at L14
        void* const sound = *it;
        host.sound_vcall_14(sound);                     // 0094018B
        const float level = host.sound_vcall_1c(sound); // 00940194
        if (level == 0.0f || !enabled) continue;        // 009401A8, 009401AE
        Raw sample{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(sound)}.ptr(0x4c))};

        // 009401B8..009401F1: the sample's file name (+50h) copied into L24 {size, data}.
        TempString name;
        const std::uint32_t* file = &sample.at<std::uint32_t>(0x50);
        if (static_cast<const void*>(name.s) != file) {
            host.native_string_resize_0041dd40(name.s, file[0], 1);
            if (file[0] != 0) {
                host.memcpy_00bf7680(name.data(), reinterpret_cast<const void*>(static_cast<std::uintptr_t>(file[1])),
                                     name.s[0]);
            }
        }
        auto assign = [&](const std::uint32_t* from) {  // 00940280..009402B4, 0094036D..009403A3
            if (static_cast<const void*>(name.s) == from) return;
            host.native_string_resize_0041dd40(name.s, from[0], 1);
            if (from[0] != 0) {
                host.memcpy_00bf7680(name.data(), reinterpret_cast<const void*>(static_cast<std::uintptr_t>(from[1])),
                                     name.s[0]);
            }
        };
        {   // 009401F4..009402DF: keep what follows the last '/' (00CE7898).
            TempString slash;
            temp_build(slash, 1, "/", host);
            const std::int32_t at = host.native_string_rfind_00467cf0(name.s, slash.s, 0x7fffffff);  // 0094023D
            temp_release(slash, host);
            if (at != -1) {
                std::uint32_t tail[2];  // LF8
                assign(host.native_string_substr_00469840(name.s, tail, static_cast<std::uint32_t>(at) + 1, 0x7fffffff));  // 00940279
                if (tail[1] != 0) {
                    host.sized_pool_return_00419cc0_00bd1510(reinterpret_cast<void*>(static_cast<std::uintptr_t>(tail[1])),
                                                             tail[0] + 1, 1);  // 009402DF
                }
            }
        }
        {   // 009402E4..009403CE: drop from the last '.' (00CE3A70).
            TempString dot;
            temp_build(dot, 1, ".", host);
            const std::int32_t at = host.native_string_rfind_00467cf0(name.s, dot.s, 0x7fffffff);  // 00940330
            temp_release(dot, host);
            if (at != -1) {
                std::uint32_t head[2];  // LCC
                assign(host.native_string_substr_00469840(name.s, head, 0, static_cast<std::uint32_t>(at)));  // 00940366
                if (head[1] != 0) {
                    host.sized_pool_return_00419cc0_00bd1510(reinterpret_cast<void*>(static_cast<std::uintptr_t>(head[1])),
                                                             head[0] + 1, 1);  // 009403CE
                }
            }
        }

        alignas(4) std::uint8_t options_storage[0x48];  // L80
        Raw options{options_storage};
        host.sound_options_copy_construct_0093fdf0(options_storage, host.sound_sample_options_00a81860(sample.p));  // 009403E9
        float position[3] = {listener[0], listener[1], listener[2]};  // L58
        if (*host.sound_sample_options_00a81860(sample.p) == 1) {     // 0094041F
            float scratch[3];                                         // LE8
            const float* p = host.sound_vcall_38(sound, scratch);     // 00940438
            position[0] = p[0];
            position[1] = p[1];
            position[2] = p[2];
        }
        const char* scale = options.at<std::uint8_t>(0x20) != 0 ? "Lin" : "Log";  // 00D1986C, 00D11110
        const float dx = listener[0] - position[0];
        const float dy = listener[1] - position[1];
        const float dz = listener[2] - position[2];
        // 00940495..009404D9: dy*dy and dx*dx each stored to float, summed on the x87 stack with
        // dz*dz stored to float, the total stored to float, then sqrt.
        const float dy2 = static_cast<float>(static_cast<double>(dy) * dy);
        const float dx2 = static_cast<float>(static_cast<double>(dx) * dx);
        const float dz2 = static_cast<float>(static_cast<double>(dz) * dz);
        const float total = static_cast<float>((static_cast<double>(dy2) + dx2) + dz2);
        const float distance = static_cast<float>(host.sqrt_00bf7030(total));
        const char* text = name.s[1] != 0 ? static_cast<const char*>(name.data()) : globals.empty_string_00f89b30;
        // 009404F5..00940540: DC C9 FMUL ST1,ST0 (bytes), so _ftol takes level * 100 first and
        // 100 * volume second; min and max are CVTTSS2SI.
        const auto level_percent = static_cast<std::int32_t>(static_cast<double>(level) * 100.0);  // 00D7A220
        const auto volume_percent = static_cast<std::int32_t>(100.0 * static_cast<double>(options.at<float>(0x04)));
        char label[0x200];  // L190
        host.sprintf_sound_label_00bf7a6a(label, "%s(%d%%%%) %s:%d/%d/%dm:%d", text, volume_percent, scale,
                                          static_cast<std::int32_t>(options.at<float>(0x0c)),
                                          static_cast<std::int32_t>(options.at<float>(0x10)),
                                          static_cast<std::int32_t>(distance), level_percent);  // 00940540, 00D19850

        if (*host.sound_sample_options_00a81860(sample.p) == 1 &&
            host.camera_project_0043a870(position, screen, 2, 1) == 7) {  // 0094054D, 00940567
            float pair[2];  // L148
            const float* at = host.overlay_find_pair_0093f790(overlay.p, pair, sound);  // 00940584
            screen[0] = at[0];
            screen[1] = at[1];
            if (sound == closest) {
                if ((*globals.colour_guard_00f89b2c & 1) == 0) {  // 009405A6
                    *globals.colour_guard_00f89b2c |= 1;
                    globals.colour_00f89b28[2] = 0xff;
                    globals.colour_00f89b28[1] = 0x00;
                    globals.colour_00f89b28[0] = 0x00;
                    globals.colour_00f89b28[3] = 0xff;
                }
                std::uint32_t colour;
                std::memcpy(&colour, globals.colour_00f89b28, 4);
                host.diagnostic_text_coloured_004c70c0(globals.game_00e188a8, screen[0], screen[1], colour, label);  // 009405F9
            } else {
                host.diagnostic_text_004c7070(globals.game_00e188a8, screen[0], screen[1], label);  // 00940623
            }
        } else {
            host.diagnostic_sink_004c14c0();  // 0094062D
        }
        host.sound_parameters_resize_0093f950(options.field(0x3c), 0);  // 00940643
        host.free_00bf6989(options.ptr(0x3c));                         // 00940650
        temp_release(name, host);                                      // 0094067D
    }

    // 0094069F..0094071A: FMOD statistics read and dropped; the eight sink calls and the three
    // counters cleared between them.
    std::int32_t channels = 0;  // LC8
    void* system = manager.ptr(0x44);
    host.fmod_get_channels_playing_00a4c296(system, &channels);
    float dsp_usage, total_usage;  // L140, L144
    host.fmod_get_cpu_usage_00a4c290(system, &dsp_usage, nullptr, nullptr, &total_usage);
    for (int i = 0; i < 5; ++i) host.diagnostic_sink_004c14c0();
    Raw live_manager{static_cast<std::uint8_t*>(*globals.sound_manager_slot_00f8bbd8)};
    live_manager.at<std::uint32_t>(0x160) = 0;
    host.diagnostic_sink_004c14c0();
    live_manager = Raw{static_cast<std::uint8_t*>(*globals.sound_manager_slot_00f8bbd8)};
    live_manager.at<std::uint32_t>(0x164) = 0;
    host.diagnostic_sink_004c14c0();
    live_manager = Raw{static_cast<std::uint8_t*>(*globals.sound_manager_slot_00f8bbd8)};
    live_manager.at<std::uint32_t>(0x168) = 0;
    host.diagnostic_sink_004c14c0();

    if (closest != nullptr && enabled) {  // 0094071F..0094072F
        alignas(4) std::uint8_t options_storage[0x48];  // L80
        Raw options{options_storage};
        host.sound_options_copy_construct_0093fdf0(
            options_storage,
            host.sound_sample_options_00a81860(Raw{static_cast<std::uint8_t*>(closest)}.ptr(0x4c)));  // 00940745
        const bool up = host.frontend_action_edge_004d92b0(globals.game_00e188a8, 0x20);    // 0094075A, L1A
        const bool down = host.frontend_action_edge_004d92b0(globals.game_00e188a8, 0x21);  // 0094076B, L23
        Raw input{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(host.input_manager_004bec00())}.ptr(4))};
        const bool shift_a = input.at<std::uint8_t>(0x9e8) != 0 && input.at<float>(0x9e4) > 0.0f;  // L1B, 00D7A218
        input = Raw{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(host.input_manager_004bec00())}.ptr(4))};
        const bool shift_b = input.at<std::uint8_t>(0xa18) != 0 && input.at<float>(0xa14) > 0.0f;  // L13
        const bool changed = up || down;  // L1A after 009407D6
        float& volume = options.at<float>(0x04);
        float& min_distance = options.at<float>(0x0c);
        float& max_distance = options.at<float>(0x10);
        constexpr double kVolumeStep = static_cast<double>(0.05f);  // 00D7A270
        auto clamp_min = [&](float r) {  // 00940822..0094087B
            min_distance = r;
            if (0.0f > r) min_distance = 0.0f;
            else if (r > max_distance) min_distance = max_distance;
        };
        if (up) {
            if (!shift_a) {
                if (!shift_b) volume = static_cast<float>(volume + kVolumeStep);  // 009407F6
                else max_distance = host.sound_overlay_axis_step_0093f280(1, max_distance);  // 0094088A
            } else if (!shift_b) {
                clamp_min(host.sound_overlay_axis_step_0093f280(1, min_distance));  // 0094081D
            }
        }
        if (down) {
            if (!shift_a) {
                if (!shift_b) {
                    volume = static_cast<float>(volume - kVolumeStep);  // 009408BB
                } else {
                    const float r = host.sound_overlay_axis_step_0093f280(0, max_distance);  // 00940952
                    max_distance = min_distance > r ? min_distance : r;
                }
            } else if (!shift_b) {
                clamp_min(host.sound_overlay_axis_step_0093f280(0, min_distance));  // 009408E2
            }
        }
        // 00940991..00940A54: volume quantised to 0.05f steps (DC F9 FDIV ST1,ST0 and D8 C9 FMUL,
        // read from the bytes); below one step it becomes 0.05f (00CE7638), above 1 it is 1.
        const auto steps = static_cast<std::int32_t>(volume / kVolumeStep + 0.5);  // 00D7A280, _ftol
        const float quantised = static_cast<float>(steps * kVolumeStep);
        if (kVolumeStep > quantised) volume = 0.05f;
        else volume = quantised > 1.0f ? 1.0f : quantised;

        bool apply = changed;
        Raw controls{static_cast<std::uint8_t*>(*globals.control_manager_slot_00f8bbf4)};
        Raw list{controls.field(0x6c)};
        if (inline_size(list) != 0) {  // 009409DE..009409F0
            if (list.ptr(kVecFirst) == nullptr || inline_size(list) == 0) host.checked_iterator_report_00bf6713();
            Raw front{static_cast<std::uint8_t*>(*static_cast<void**>(list.ptr(kVecFirst)))};
            if (front.p != nullptr && front.at<std::uint8_t>(0x32) != 0 && front.at<std::uint8_t>(0x132) == 0 &&
                shift_b) {
                options.at<std::uint8_t>(0x20) = options.at<std::uint8_t>(0x20) == 0 ? 1 : 0;  // 00940A24 SETZ
                apply = true;
            }
        }
        if (apply) {  // 00940A5D
            alignas(4) std::uint8_t by_value[0x48];
            host.sound_options_copy_construct_0093fdf0(by_value, options_storage);  // 00940A6E
            host.sound_apply_settings_0093fe90(closest, by_value);                  // 00940A7A
            host.sound_manager_dirty_class_entries_00a7a3f0(*globals.sound_manager_slot_00f8bbd8, 0xffff);  // 00940A8A
        }
        host.sound_parameters_resize_0093f950(options.field(0x3c), 0);  // 00940AA0
        host.free_00bf6989(options.ptr(0x3c));                         // 00940AAD
    }

    // 00940AB7..00940CA2: a wire sphere at the camera target when the camera's world matrix
    // differs from the view's (game+1ED4h vtable+120h).
    void* game = globals.game_00e188a8;
    if (game != nullptr && Raw{static_cast<std::uint8_t*>(game)}.ptr(0x19fc) != nullptr) {
        Raw g{static_cast<std::uint8_t*>(game)};
        Raw camera{static_cast<std::uint8_t*>(g.ptr(0x19fc))};
        if ((camera.at<std::uint8_t>(0x5c) & 2) == 0) host.transform_refresh_world_00b6db70(camera.p);  // 00940ADF
        float world[16];  // L150
        std::memcpy(world, camera.field(0xf0), sizeof(world));
        float view[16];   // L100
        std::memcpy(view, world, sizeof(view));
        if (g.ptr(0x1ed4) != nullptr) {
            alignas(4) std::uint8_t scratch[0x40];  // L80
            const float* m = host.object_vcall_120(g.ptr(0x1ed4), scratch);  // 00940B37
            for (int i = 0; i < 16; ++i) view[i] = m[i];
        }
        if (host.matrix44_not_equal_0093efc0(world, view)) {  // 00940C27
            const float colour[4] = {1.0f, 1.0f, 1.0f, 1.0f};   // LD8
            const float colour2[4] = {1.0f, 1.0f, 1.0f, 1.0f};  // LE8
            host.debug_wire_sphere_00860be0(&view[12], colour, 3.0f /* 00CE3854 */, colour2, 1);  // 00940CA2
        }
    }

    // 00940CA7..00940D28: the local array's inline destructor. The reallocation and zero-fill
    // branches run only for a negative capacity or count.
    auto* data = static_cast<std::uint32_t*>(sounds.ptr(0));
    const std::int32_t count = sounds.at<std::int32_t>(4);
    const std::int32_t capacity = sounds.at<std::int32_t>(8);
    if (capacity < 0 && capacity < 1) {
        auto* fresh = static_cast<std::uint32_t*>(host.operator_new_00bf55be(4));  // 00940CC1
        for (std::int32_t i = 0; i < count; ++i) {
            if (fresh + i != nullptr) fresh[i] = data[i];
        }
        host.free_00bf6989(data);  // 00940CED
        data = fresh;
    }
    if (count < 0) {
        for (std::int32_t i = count; i < 0; ++i) {
            if (data + i != nullptr) data[i] = 0;  // 00940D0A
        }
    }
    host.free_00bf6989(data);  // 00940D28
}

}  // namespace bsp::cc13_large61
