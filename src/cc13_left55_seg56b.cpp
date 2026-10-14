// Packet cc13-left55, part b: script bindings of segment 56. See include/bsp/cc13_left55_seg56b.hpp
// for the shared shape and local/left55_doc_seg56b.md for the per-function evidence. Comments cite
// the instruction each statement stands for. x87 arithmetic is written in double where the
// native keeps the 80-bit stack; every FSTP float is a static_cast<float> at the same point.
#include "bsp/cc13_left55_seg56b.hpp"

#include <cmath>
#include <cstdint>
#include <cstring>

namespace bsp::cc13_left55::seg56b {
namespace {

void* game(ImageAccess& image)
{
    return image.global_ptr(kGameGlobal);
}

void* scoring(ImageAccess& image)
{
    return image.read_ptr(game(image), kScoringManagerOffset);
}

// base + offset as an address, with the native's wrap-around (null bases included).
void* at(void* base, std::uint32_t offset)
{
    return reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(base) + offset);
}

Vec3 read_vec3(ImageAccess& image, void* base, std::uint32_t offset)
{
    return Vec3{image.read_f32(base, offset), image.read_f32(base, offset + 4u),
        image.read_f32(base, offset + 8u)};
}

// The first entity of the world list [[game+19CCh]+4h] and its +38h successor chain.
void* first_world_entity(ImageAccess& image)
{
    void* list = image.read_ptr(game(image), 0x19CCu);    // MOV EDX,[ECX+19CCh]
    return image.read_ptr(image.read_ptr(list, 4u), 0u);  // MOV EAX,[EDX+4] / MOV ESI,[EAX]
}

// The inline net-id lookup of 008A8C4E..008A8C7A: the id is the low word of the integer.
void* entity_from_net_id(ImageAccess& image, std::int32_t integer)
{
    const std::uint32_t id = static_cast<std::uint32_t>(integer) & 0xFFFFu; // MOVZX EAX,AX
    if (static_cast<std::int32_t>(id) < image.global_i32(kIdSplit)) {      // CMP / JGE (signed)
        const std::uint32_t row = (id - static_cast<std::uint32_t>(image.global_i32(kIdLowBase))) << 4;
        return image.read_ptr(image.global_ptr(kIdLowTable), row + 0xCu);
    }
    const std::uint32_t row = (id - static_cast<std::uint32_t>(image.global_i32(kIdHighBase))) << 4;
    return image.read_ptr(image.global_ptr(kIdHighTable), row + 0xCu);
}

int fill_nine_integers(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host,
    std::uint32_t first)
{
    void* entity = args.entity_at(0);
    LuaRef table = host.frame_new_table_00b67930(frame);
    for (int key = 1; key <= 9; ++key) { // LEA ESI,[EAX+1] ... CMP ESI,9 / JL
        host.table_set_integer_at_index_00b665d0(table, key,
            image.read_i32(entity, first + 4u * static_cast<std::uint32_t>(key - 1)));
    }
    return frame.result_count();
}

// The four Scoring_Add*Score bindings share this shape (008B9F6E..008BA06E for the first).
template <typename AddScore>
int add_score(LuaArgs& args, ImageAccess& image, AddScore add)
{
    std::int32_t slot = image.read_i32(game(image), kLocalSlotOffset); // MOV EDI,[EAX+18ECh]
    int index = 0;                                                     // XOR ESI,ESI
    if (image.read_i32(game(image), kSessionModeOffset) != 0) {        // CMP [EAX+1FE4h],EBP
        slot = args.get_integer(0);
        index = 1;
    } else if (args.count() == 3) {                                    // CMP EAX,3 / JNZ
        (void)args.get_integer(0); // read and discarded: no MOV EDI,EAX on this path
        index = 1;
    }
    const std::string key = args.get_string(index); // copied by 0041E870
    const int value = args.get_integer(index + 1);  // ADD ESI,1
    add(scoring(image), slot, key, value);
    return 0;
}

} // namespace

// 008A8480 GetETA. Both entity reads use argument 0 (PUSH EBP with EBP = 0 at 008A855E and
// 008A8597), so A and B are the same entity unless the frame changes between the reads.
int lua_get_eta(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    void* a = args.entity_at(0); // ESI
    void* b = args.entity_at(0); // EDI
    if (image.read_u8(a, 0x10Cu) == 0) {                      // 008A85C7
        host.entity_pose_refresh_world_00414db0(a);
        image.write_u8(a, 0x10Cu, 1);                          // 008A85E3
        host.matrix_build_orthogonal_scaled_affine_inverse_00b63d50(at(a, 0x110u), at(a, 0xCCu));
    }
    const Vec3 b_vector = host.entity_vcall_34(b);                                  // 008A8603
    const Vec3 local = host.vector_transform_direction_0042d0d0(b_vector, at(a, 0x110u), false); // PUSH EBP(0)
    // 008A8617..008A861D: FADD float out.z, FSTP float.
    const float closing = static_cast<float>(static_cast<double>(host.entity_vcall_38(a)) + local.z);
    // 008A8621..008A862F: FCOMIP against double 0.001, JBE (also taken when unordered) -> nil.
    if (static_cast<double>(closing) > kEtaMinClosingSpeed) {
        if (image.read_u8(a, 0xC8u) == 0) {
            host.entity_pose_refresh_world_00414db0(a);
        }
        if (image.read_u8(b, 0xC8u) == 0) {
            host.entity_pose_refresh_world_00414db0(b);
        }
        Vec3 delta;
        delta.x = image.read_f32(b, 0xFCu) - image.read_f32(a, 0xFCu);   // FSUB float, FSTP float
        delta.y = image.read_f32(b, 0x100u) - image.read_f32(a, 0x100u);
        delta.z = image.read_f32(b, 0x104u) - image.read_f32(a, 0x104u);
        const float length = host.vector_length_0042b2f0(delta);         // FSTP float [ESP+10h]
        results.push_float(length / closing);                            // FDIV float, FSTP float
    } else {
        results.push_nil();
    }
    return 1;
}

// 008A8B40 SetPartyID. Argument 0 is a net id resolved inline, argument 1 the party.
int lua_set_party_id(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    void* entity = entity_from_net_id(image, args.get_integer(0));
    const std::int32_t party = args.get_integer(1);
    if (host.entity_vcall_5c(entity, 2)) {     // PUSH EBX(2), TEST AL,AL / JZ 008A8D16
        PartyIdMessage message;
        message.party = party;
        host.route_message_0077c2a0(entity, message, kRouteFlags, 0);
    } else {
        int zero = 0;                          // MOV [ESP+24h],EBP
        host.entity_vcall_2c(entity, party, image.read_i32(entity, 0x58u), &zero);
    }
    results.push_number(party);                // 008A8D2D PUSH EDI
    return 1;
}

// 008A8D90 SetCommandBuildingOwnerPlayer.
int lua_set_command_building_owner_player(LuaArgs& args, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t player = args.get_integer(1);
    if (host.entity_vcall_5c(entity, 0x1C)) {
        CommandBuildingOwnerMessage message;
        message.player = player;
        host.route_message_0077c2a0(entity, message, kRouteFlags, 0); // ECX = ESI at 008A8F20
    }
    return 0;
}

// 008A91B0 SetAiControl: clears byte +290h whatever the script passes; no other argument is read.
int lua_set_ai_control(LuaArgs& args, ImageAccess& image)
{
    image.write_u8(args.entity_at(0), 0x290u, 0);
    return 0;
}

// 008A9460 GetPrimaryTarget.
int lua_get_primary_target(LuaArgs& args, LuaFrame& frame, Left55Seg56bImage& image, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    // 008A9565..008A9578: null or not class 2 -> ESI = 0; the vcall below then reads a null
    // vtable in the native. The reconstruction hands the null to the host unchanged.
    if (entity == nullptr || !host.entity_vcall_5c(entity, 2)) {
        entity = nullptr;
    }
    CommandTargetBuffer target;
    host.entity_vcall_178(entity, target);                        // 008A959A
    if (target.bytes[0] != 0) {                                   // CMP [ESP+44h],0
        if (host.command_target_resolve_object_00521ea0(target) != nullptr) {
            void* object = host.command_target_resolve_object_00521ea0(target); // called again
            const std::string id = host.native_string_from_int_004260b0(image.read_u16(object, 0x174u));
            LuaRef table = host.frame_get_by_literal_00b67910(frame, "thisTable"); // 00CE7494
            LuaRef value = host.lua_get_by_native_string_00b678e0(table, id);
            host.lua_push_value_00b663d0(value);
            return frame.result_count();
        }
    } else if (target.bytes[1] != 0) {                            // CMP [ESP+45h],0
        LuaRef table = host.frame_new_table_00b67930(frame);
        Vec3 position;
        if (target.bytes[1] != 0) {                               // re-tested at 008A967A
            std::memcpy(&position, target.bytes + 8, sizeof position);
        } else {
            position = read_vec3(image, nullptr, kZeroVector3); // MOV EAX,00F87574; never taken
        }
        host.table_set_vector3_fields_0088ba30(table, position);
        return frame.result_count();
    }
    frame.push_nil();                                             // 008A96D6
    return frame.result_count();
}

// 008AA560 PutRelTo: place entity A at offset (argument 2) in entity B's frame, heading
// (optional argument 3, degrees) and drag A's group members along.
int lua_put_rel_to(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    void* a = args.entity_at(0);              // EBP
    void* b = args.entity_at(1);              // EBX
    const Vec3 offset = args.read_vector3(2); // 00888760 into ESP+40h
    if (image.read_u8(b, 0xC8u) == 0) {
        host.entity_pose_refresh_world_00414db0(b);
    }
    float matrix[16];                         // REP MOVSD of B+CCh, 10h dwords, into ESP+74h
    for (std::uint32_t i = 0; i < 16u; ++i) {
        matrix[i] = image.read_f32(b, 0xCCu + 4u * i);
    }
    if (image.read_u8(b, 0xC8u) == 0) {       // 008AA718, tested again after the copy
        host.entity_pose_refresh_world_00414db0(b);
    }
    const Vec3 world = host.vector_transform_affine_point_004142e0(offset, at(b, 0xCCu));
    matrix[12] = world.x;                     // ESP+A4h..ACh: the copy's translation row
    matrix[13] = world.y;
    matrix[14] = world.z;
    if (host.entity_vcall_5c(a, 6) && !host.entity_vcall_5c(a, 8)) {
        matrix[13] = 0.0f;                    // 008AA784 XORPS / MOVSS [ESP+A8h]
    }
    host.entity_vcall_114(a);
    if (host.entity_vcall_5c(a, 2)) {
        (void)host.unit_is_formation_follower_007788b0(a); // result not used
    }
    host.entity_vcall_120(a, matrix);
    if (args.count() == 4) {
        const double degrees = args.get_number(3);
        host.entity_vcall_11c(a, static_cast<float>(degrees * kPiAsDouble / kDegreesPerHalfTurn));
    }
    host.entity_vcall_d8(a);
    if (host.entity_vcall_5c(a, 2) && host.unit_is_group_leader_00778890(a)) {
        const std::int32_t members = image.read_i32(image.read_ptr(a, 0x284u), 0x4F8u); // EBX
        for (std::int32_t i = 0; i < members; ++i) {
            void* member = host.unit_group_member_at_0070d060(image.read_ptr(a, 0x284u), i); // reloaded
            if (member != a) {
                // PUSH 00F87574: the address of the global zero vector.
                host.entity_vcall_118(member, reinterpret_cast<const void*>(static_cast<std::uintptr_t>(kZeroVector3)));
                host.entity_vcall_d8(member);
            }
        }
    }
    return 0;
}

// 008AAB90 ClearForcedReconLevel: entity, level.
int lua_clear_forced_recon_level(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    const std::uint32_t level_offset = static_cast<std::uint32_t>(args.get_integer(1)) * 0x34u + 0x1E8u; // IMUL EDI,EDI,34h
    host.recon_clear_forced_level_00803b50(at(entity, level_offset));
    if (host.entity_vcall_5c(entity, 0x18)) {
        // 008AACF6..008AAD27: the squadron's planes; slots past the fifth use a null base.
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            void* plane = static_cast<std::uint32_t>(slot) <= 4u
                ? image.read_ptr(entity, 0x3D0u + 4u * static_cast<std::uint32_t>(slot))
                : nullptr;
            host.recon_clear_forced_level_00803b50(at(plane, level_offset));
        }
    } else if (host.entity_vcall_5c(entity, 0x1A)) {
        // 008AAD83..008AADE4: a checked-iterator walk of the vector at +394h (begin +398h, end +39Ch).
        auto address = [](void* p) { return reinterpret_cast<std::uintptr_t>(p); };
        void* it = image.read_ptr(entity, 0x398u);
        if (address(it) > address(image.read_ptr(entity, 0x39Cu))) {     // JBE skips
            host.invalid_parameter_00bf6713();
        }
        for (;;) {
            void* end = image.read_ptr(entity, 0x39Cu);
            if (address(image.read_ptr(entity, 0x398u)) > address(end)) { // CMP [ESI+4],EBX / JBE
                host.invalid_parameter_00bf6713();
            }
            if (it == end) {
                break;
            }
            if (address(it) >= address(image.read_ptr(entity, 0x39Cu))) { // JC skips
                host.invalid_parameter_00bf6713();
            }
            host.recon_clear_forced_level_00803b50(at(image.read_ptr(it, 0u), level_offset));
            if (address(it) >= address(image.read_ptr(entity, 0x39Cu))) {
                host.invalid_parameter_00bf6713();
            }
            it = at(it, 4u);
        }
    }
    return 0;
}

// 008ABAC0 GetRoleAvailable: {[1..9] = entity+188h dwords}.
int lua_get_role_available(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    return fill_nine_integers(args, frame, image, host, 0x188u);
}

// 008ABC70 GetPlayerIndexBySlot.
int lua_get_player_index_by_slot(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    const std::int32_t slot = args.get_integer(0);
    std::int32_t index = 0;
    void* g = game(image);
    if (g != nullptr) {
        index = image.read_i32(image.read_ptr(g, kPlayerSlotTable + 4u * static_cast<std::uint32_t>(slot)), 0x1Cu);
    }
    results.push_number(index);
    return 1;
}

// 008ABDF0 GetValidPlayerInSlot: game && slot+8h && !slot+9h.
int lua_get_valid_player_in_slot(LuaArgs& args, LuaResults& results, ImageAccess& image)
{
    const std::int32_t slot = args.get_integer(0);
    bool valid = false;                                   // 008ABF0D
    void* g = game(image);
    if (g != nullptr) {
        void* record = image.read_ptr(g, kPlayerSlotTable + 4u * static_cast<std::uint32_t>(slot));
        if (image.read_u8(record, 8u) != 0) {
            valid = image.read_u8(record, 9u) == 0;       // 008ABF21..008ABF2C
        }
    }
    results.push_boolean(valid);
    return 1;
}

// 008ABF90 GetRoleOwner: {[1..9] = entity+1ACh dwords}.
int lua_get_role_owner(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    return fill_nine_integers(args, frame, image, host, 0x1ACu);
}

// 008AC9D0 Save: only the dead prologue string; no frame, XOR EAX,EAX at 008ACA1B.
int lua_save()
{
    return 0;
}

// 008ACD10 AddFire: reads its four arguments and does nothing with them.
int lua_add_fire(LuaArgs& args)
{
    (void)args.entity_at(0);
    (void)args.get_integer(1);
    (void)args.get_integer(2);
    (void)args.get_number(3);
    return 0;
}

// 008ACF00 GetFire: pushes 0.0 when the fire list at entity+344h holds a record whose +4h and
// +8h match arguments 1 and 2, nothing otherwise.
int lua_get_fire(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t first = args.get_integer(1);  // EBP
    const std::int32_t second = args.get_integer(2); // [ESP+10h]
    void* fires = at(entity, 0x344u);                // ADD ESI,344h
    auto in_range = [&](std::uint32_t i) {
        void* begin = image.read_ptr(fires, 4u);
        if (begin == nullptr) {
            return false;
        }
        const auto bytes = reinterpret_cast<std::intptr_t>(image.read_ptr(fires, 8u))
            - reinterpret_cast<std::intptr_t>(begin);
        return i < static_cast<std::uint32_t>(bytes >> 2); // SAR EAX,2 / CMP EDI,EAX / JC
    };
    int pushed = 0;
    for (std::uint32_t i = 0; i < host.vector_size_0043ec50(fires); ++i) {
        if (!in_range(i)) {
            host.invalid_parameter_00bf6713();
        }
        void* record = image.read_ptr(image.read_ptr(fires, 4u), 4u * i);
        if (image.read_i32(record, 4u) != first) {
            continue;
        }
        if (!in_range(i)) {
            host.invalid_parameter_00bf6713();
        }
        record = image.read_ptr(image.read_ptr(fires, 4u), 4u * i);
        if (image.read_i32(record, 8u) == second) {
            results.push_float(0.0f);                // FLDZ / FSTP float
            pushed = 1;
            break;
        }
    }
    return pushed;
}

// 008AD150 SetRepairLevel(entity, category name, level): the level is argument 2 and is read
// before the name (argument 1), PUSH EBX(2) at 008AD269.
int lua_set_repair_level(LuaArgs& args, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    const std::int32_t level = args.get_integer(2);
    const std::string name = args.get_string(1); // copied into the by-value argument (0041E870)
    const int category = host.repair_category_from_name_0081ad40(name);
    host.unit_set_repair_level_00827a40(entity, category, level);
    return 0;
}

// 008AD540 FailureRepairEnable: the class-6 test result is discarded (no TEST after 008AD65F).
int lua_failure_repair_enable(LuaArgs& args, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    (void)host.entity_vcall_5c(entity, 6);
    const bool enabled = args.get_boolean(1);
    host.repair_task_set_failure_repair_enabled_00939fe0(at(entity, 0xA20u), enabled);
    return 0;
}

// 008ADC40 GetRepairLevel(entity, category name).
int lua_get_repair_level(LuaArgs& args, LuaResults& results, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    const std::string name = args.get_string(1);
    const int category = host.repair_category_from_name_0081ad40(name);
    results.push_number(host.unit_get_repair_level_0080e130(entity, category));
    return 1;
}

// 008AE180 KillStationaries: kill (cause 0) every world entity of class 36h. The successor is
// read after the kill (008AE297).
int lua_kill_stationaries(ImageAccess& image, Left55Seg56bHost& host)
{
    for (void* entity = first_world_entity(image); entity != nullptr; entity = image.read_ptr(entity, 0x38u)) {
        if (host.entity_vcall_5c(entity, 0x36)) {
            host.mission_entity_kill_00926d90(entity, 0);
        }
    }
    return 0;
}

// 008AE2F0 KillBullets: the same walk for classes 29h, 2Ah, 2Bh (short-circuit order).
int lua_kill_bullets(ImageAccess& image, Left55Seg56bHost& host)
{
    for (void* entity = first_world_entity(image); entity != nullptr; entity = image.read_ptr(entity, 0x38u)) {
        if (host.entity_vcall_5c(entity, 0x29) || host.entity_vcall_5c(entity, 0x2A)
            || host.entity_vcall_5c(entity, 0x2B)) {
            host.mission_entity_kill_00926d90(entity, 0);
        }
    }
    return 0;
}

// 008AE760 GetAirbaseOrders: block +E0h, +E1h (booleans), +E4h (integer). No null test.
int lua_get_airbase_orders(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false); // XOR DL,DL
    results.push_boolean(image.read_u8(block, 0xE0u) != 0);
    results.push_boolean(image.read_u8(block, 0xE1u) != 0);
    results.push_number(image.read_i32(block, 0xE4u));
    return 3;
}

// 008AE900 SetAirbaseOrders(entity, a, b, n): the readers run n first (argument 3, ECX = EDI),
// then argument 2, then argument 1; 006BF650 receives (arg1, arg2, arg3).
int lua_set_airbase_orders(LuaArgs& args, Left55Seg56bHost& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false);
    const std::int32_t third = args.get_integer(3);
    const bool second = args.get_boolean(2);
    const bool first = args.get_boolean(1);
    host.air_ops_block_set_orders_006bf650(block, first, second, third);
    return 0;
}

// 008AEB20 SetBorderZoneParty(position, party): the zone record keyed by the position gets
// +4h = party.
int lua_set_border_zone_party(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    const Vec3 key = args.read_vector3(0);
    const std::int32_t party = args.get_integer(1);
    void* zone = host.game_find_border_zone_00489c40(game(image), key);
    image.write_i32(zone, 4u, party); // 008AEC7D
    return 0;
}

// 008AF260 GetBorderZone: two tables {x,y,z} from game+711Ch..7124h and game+7128h..7130h,
// all six floats read before the first table (008AF342..008AF38C).
int lua_get_border_zone(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    void* g = game(image);
    const Vec3 low = read_vec3(image, g, 0x711Cu);
    const Vec3 high = read_vec3(image, g, 0x7128u);
    LuaRef table = host.frame_new_table_00b67930(frame);
    host.table_set_number_field_00b67400(table, "x", low.x); // key strings 00CEB488/00D045F8/00CFD718
    host.table_set_number_field_00b67400(table, "y", low.y);
    host.table_set_number_field_00b67400(table, "z", low.z);
    table = host.frame_new_table_00b67930(frame);            // the first LuaObject is destroyed first
    host.table_set_number_field_00b67400(table, "x", high.x);
    host.table_set_number_field_00b67400(table, "y", high.y);
    host.table_set_number_field_00b67400(table, "z", high.z);
    return frame.result_count();
}

// 008AF700 GetBorderCross(entity or position, bearing degrees): the first world-box edge a
// 50000-unit probe along the bearing (relative to the entity's heading) crosses, or nil.
int lua_get_border_cross(LuaArgs& args, LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    void* entity = nullptr;                    // XOR ESI,ESI at 008AF7F6
    Vec3 start;
    if (args.is_entity_table(0)) {             // 008889C0 -> BL
        entity = args.entity_at(0);
        if (image.read_u8(entity, 0xC8u) == 0) {
            host.entity_pose_refresh_world_00414db0(entity);
        }
        start = read_vec3(image, entity, 0xFCu);
    } else {
        start = args.read_vector3(0);
    }
    float bearing = static_cast<float>(args.get_number(1) * kPiAsDouble / kDegreesPerHalfTurn); // FSTP [ESP+1Ch]
    if (entity != nullptr) {
        if (image.read_u8(entity, 0xC8u) == 0) {
            host.entity_pose_refresh_world_00414db0(entity);
        }
        if (image.read_u8(entity, 0xC8u) == 0) {
            host.entity_pose_refresh_world_00414db0(entity);
        }
        const float y = image.read_f32(entity, 0xCCu + 0x28u); // FLD [EDI+28h]: first operand
        const float x = image.read_f32(entity, 0xCCu + 0x20u); // FLD [EBX+20h]: second operand
        const float heading = static_cast<float>(std::atan2(static_cast<double>(y), static_cast<double>(x))); // 00BF701A
        bearing = static_cast<float>(static_cast<double>(heading) - kHalfPiAsDouble + bearing);
    }
    const float s = static_cast<float>(std::sin(static_cast<double>(bearing))); // FSIN, FSTP float
    const float c = static_cast<float>(std::cos(static_cast<double>(bearing))); // FCOS, FSTP float
    void* g = image.global_ptr(kGameGlobal);                                    // ESI = [00E188A8]
    Vec3 end;
    if (!host.game_point_outside_map_bounds_0071c4f0(g, start)) {
        // 008AF9B1..008AFA07 (decompiler order; the listing's FMUL ST1 is the DC form).
        const float dx = static_cast<float>(-static_cast<double>(s) * kBorderProbeLength);
        const float dz = static_cast<float>(kBorderProbeLength * static_cast<double>(c));
        end.x = static_cast<float>(static_cast<double>(dx) + start.x);
        end.y = static_cast<float>(static_cast<double>(start.y) + kBorderProbeLift);
        end.z = static_cast<float>(static_cast<double>(dz) + start.z);
    } else {
        end = Vec3{0.0f, start.y, 0.0f};                                        // 008AFA0D
    }
    BoxEdgeCrossing crossing;
    host.world_first_box_edge_crossing_004bc120(g, start, end, crossing);
    if (!crossing.crossed) {
        frame.push_nil();
        return frame.result_count();
    }
    LuaRef table = host.frame_new_table_00b67930(frame);
    host.table_set_number_field_00b67400(table, "x", crossing.cross.x);
    host.table_set_number_field_00b67400(table, "y", crossing.cross.y);
    host.table_set_number_field_00b67400(table, "z", crossing.cross.z);
    return frame.result_count();
}

// 008AFFC0 EndSCore: no frame. On the host, once (latch byte game+1EF4h), send message E4h and,
// when exactly one of the eight player records is live, call [00F8A2FC] vtable+184h if its
// vtable+198h answers true.
int lua_end_score(ImageAccess& image, Left55Seg56bHost& host)
{
    void* g = game(image);
    if (image.read_i32(g, kSessionModeOffset) != 1 || image.read_u8(g, 0x1EF4u) != 0) {
        return 0;
    }
    image.write_u8(g, 0x1EF4u, 1);                                         // 008B008C
    // 008B0093..008B00C0: 004C9780(&message, [game+18C8h], 1), then 0076A9F0(game+1EF0h, &message, EBX = 0).
    void* first = image.read_ptr(game(image), 0x18C8u);
    host.session_send_e4_004c9780_0076a9f0(at(game(image), 0x1EF0u), first, 1, 0);
    int live = 0;
    void* records = game(image);                                           // 008B00EA
    for (std::uint32_t i = 0; i < 8u; ++i) {                               // two passes of four, stride 118h
        const std::uint32_t base = 0x750u + 0x118u * i;
        if (image.read_u8(records, base) != 0
            && (image.read_u8(records, base + 1u) == 0 || image.read_u8(records, base + 2u) != 0)) {
            ++live;
        }
    }
    if (live == 1) {
        void* object = image.global_ptr(0x00F8A2FCu);
        if (object != nullptr && host.object_vcall_198(object)) {
            host.object_vcall_184(image.global_ptr(0x00F8A2FCu));          // reloaded at 008B018A
        }
    }
    return 0;
}

// 008B0470 TraingingGroundEndScene: no frame; GGame::EndScene(aborted = 1).
int lua_training_ground_end_scene(ImageAccess& image, Left55Seg56bHost& host)
{
    host.game_end_scene_004d7970(game(image), true); // PUSH 1 at 008B0526
    return 0;
}

// 008B4DA0 MovCam_PathWalkPathPointPropertyPotlekokkal(value, point, [flag], [number], [name],
// [variants...]). The optional tail is the MovCam one (engine_seg56_cc13.hpp read_movcam_tail
// with first = 2), built here in the native order so the variant vector is collected before
// 005CD160.
int lua_movcam_path_walk_path_point_property_potlekokkal(LuaArgs& args, ImageAccess& image,
    Left55Seg56bHost& host)
{
    // Argument 0 is taken as a LuaObject (008B4E9F) with no reader; it is copied into 007A74F0's
    // by-value argument and passed by address to 0076D5B0.
    const float point = static_cast<float>(args.get_number(1)); // FSTP float [ESP+18h]
    bool flag = true;                                            // MOV byte [ESP+44h],1
    if (args.count() >= 3 && args.is_boolean(2)) {
        flag = args.get_boolean(2);
    }
    float number = 0.0f;                                         // XORPS / MOVSS [ESP+28h]
    if (args.count() >= 4 && args.is_number(3)) {
        number = static_cast<float>(args.get_number(3));
    }
    std::string name;                                            // "" from 00CE3A0C
    if (args.count() >= 5 && args.is_string(4)) {
        name = args.get_string(4);                               // strlen, null -> empty
    }
    void* variants = nullptr;                                    // XOR EBX,EBX at 008B5127
    if (args.count() >= 6) {
        variants = host.movcam_collect_variants_00887370(args, 6); // PUSH 4 / PUSH -1 / PUSH 6
    }
    void* target = host.movcam_ensure_target_005cd160(
        image.read_ptr(image.global_ptr(kInGameInterfaceGlobal), 0x98u));
    host.movcam_set_path_walk_point_property_007a74f0(target, args, 0, point, flag, number, name, variants);
    const std::int32_t mode = image.read_i32(game(image), kSessionModeOffset);
    if (mode != 0 && mode == 1) {                                // 008B523A / 008B523E
        host.session_broadcast_movcam_path_walk_point_property_0076d5b0(at(game(image), 0x1EF0u), args, 0,
            point, flag, number);
    }
    return 0;
}

// 008B8970 Scoring_GetRealPlayTime: float [[game+21A0h]+14A0h].
int lua_scoring_get_real_play_time(LuaResults& results, ImageAccess& image)
{
    results.push_float(image.read_f32(scoring(image), 0x14A0u));
    return 1;
}

// 008B8CA0 Scoring_SetRanking([slot], ranking): the slot argument exists only in a session.
int lua_scoring_set_ranking(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    std::int32_t slot = image.read_i32(game(image), kLocalSlotOffset); // ESI
    int index = 0;                                                     // XOR EAX,EAX
    if (image.read_i32(game(image), kSessionModeOffset) != 0) {
        slot = args.get_integer(0);
        index = 1;
    }
    const std::int32_t ranking = args.get_integer(index);
    host.scoring_set_slot_ranking_00905350(scoring(image), slot, ranking);
    return 0;
}

// 008B8E60 Scoring_ForceRefreshScoringTable([slot]).
int lua_scoring_force_refresh_scoring_table(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    std::int32_t slot = image.read_i32(game(image), kLocalSlotOffset);
    if (image.read_i32(game(image), kSessionModeOffset) != 0) {
        slot = args.get_integer(0);
    }
    host.scoring_force_refresh_009150a0(scoring(image), slot);
    return 0;
}

// 008B9E90 Scoring_AddActionScore([slot], key, value).
int lua_scoring_add_action_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    return add_score(args, image, [&](void* s, int slot, const std::string& key, int value) {
        host.scoring_add_action_score_0090f140(s, slot, key, value);
    });
}

// 008BA550 Scoring_AddShipScore([slot], key, value).
int lua_scoring_add_ship_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    return add_score(args, image, [&](void* s, int slot, const std::string& key, int value) {
        host.scoring_add_ship_score_0090f1a0(s, slot, key, value);
    });
}

// 008BAC10 Scoring_AddPlaneScore([slot], key, value).
int lua_scoring_add_plane_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    return add_score(args, image, [&](void* s, int slot, const std::string& key, int value) {
        host.scoring_add_plane_score_0090f200(s, slot, key, value);
    });
}

// 008BB2D0 Scoring_AddCommandScore([slot], key, value).
int lua_scoring_add_command_score(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    return add_score(args, image, [&](void* s, int slot, const std::string& key, int value) {
        host.scoring_add_command_score_0090f260(s, slot, key, value);
    });
}

// 008BBC40 ScoringIsUnlocked(expression).
int lua_scoring_is_unlocked(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    const std::string expression = args.get_string(0);
    results.push_boolean(host.profile_is_unlock_satisfied_007fc4c0(at(game(image), 0x650u), expression));
    return 1;
}

// 008BC740 SetAchievements([slot], token, [value = 1]): the slot is taken when argument 0 is an
// integer; GrantAward's flag is 0 (PUSH EBP).
int lua_set_achievements(LuaArgs& args, ImageAccess& image, Left55Seg56bHost& host)
{
    std::int32_t slot = image.read_i32(game(image), kLocalSlotOffset); // EDI
    const bool has_slot = args.is_integer(0);                          // 00B66A60
    if (has_slot) {
        slot = args.get_integer(0);
    }
    const int index = has_slot ? 1 : 0;
    const std::string token = args.get_string(index);
    std::int32_t value = 1;                                            // MOV EBX,1
    if (args.count() > index + 1) {                                    // JLE skips
        value = args.get_integer(index + 1);
    }
    host.scoring_grant_award_0090ede0(scoring(image), slot, token, value, 0);
    return 0;
}

// 008BCD80 Multi_GetPlayers: {[slot] = party} for the eight slot records whose byte +9h is 0;
// keys are the 0-based slot numbers (XOR EDI,EDI, incremented every pass).
int lua_multi_get_players(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    LuaRef table = host.frame_new_table_00b67930(frame);
    for (std::uint32_t i = 0; i < 8u; ++i) {                       // ESI 18CCh..18E8h
        void* record = image.read_ptr(game(image), kPlayerSlotTable + 4u * i);
        if (image.read_u8(record, 9u) == 0) {
            host.table_set_integer_at_index_00b665d0(table, static_cast<int>(i), image.read_i32(record, 0x28u));
        }
    }
    return frame.result_count();
}

// 008BCF10 GetPlayerDetails: {[slot] = {party, ai, playerindex, playerslot, playerName,
// ailevel}} for every slot record with +8h set and (+9h clear or +0Ah set); 0-based keys.
int lua_get_player_details(LuaFrame& frame, ImageAccess& image, Left55Seg56bHost& host)
{
    LuaRef table = host.frame_new_table_00b67930(frame);
    for (std::uint32_t i = 0; i < 8u; ++i) {
        void* record = image.read_ptr(game(image), kPlayerSlotTable + 4u * i); // MOV ESI,[EDI+EDX]
        if (image.read_u8(record, 8u) == 0) {
            continue;
        }
        if (image.read_u8(record, 9u) != 0 && image.read_u8(record, 0xAu) == 0) {
            continue;
        }
        const int slot = static_cast<int>(i);                                  // EBP
        host.table_set_new_table_at_index_00b66670(table, slot);
        LuaRef entry = host.table_get_by_index_00b67720(table, slot);
        host.table_set_string_integer_00b67460(entry, "party", image.read_i32(record, 0x28u));
        host.table_set_boolean_00b673a0(entry, "ai", image.read_u8(record, 9u) != 0); // MOV DL,[ESI+9]
        host.table_set_string_integer_00b67460(entry, "playerindex", image.read_i32(record, 0x1Cu));
        host.table_set_string_integer_00b67460(entry, "playerslot", slot);
        host.table_set_cstring_00b66790(entry, "playerName", static_cast<const char*>(at(record, 0x58u))); // 00D1472C
        host.table_set_string_integer_00b67460(entry, "ailevel", image.read_i32(record, 0x2Cu));
    }
    return frame.result_count();
}

// 008AC420 IsUnitUntouchable: vtable+140h answer non-null and its byte +1D4h set.
int lua_is_unit_untouchable(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56bHost& host)
{
    void* entity = args.entity_at(0);
    bool untouchable = false;
    if (host.entity_vcall_140(entity) != nullptr) {
        untouchable = image.read_u8(host.entity_vcall_140(entity), 0x1D4u) != 0; // called twice
    }
    results.push_boolean(untouchable);
    return 1;
}

} // namespace bsp::cc13_left55::seg56b
