// Packet cc13-left55, segment 56 part a. See include/bsp/cc13_left55_seg56a.hpp for the shared
// shape and the host ABI notes, local/left55_doc_seg56a.md for the per-function evidence. Comments
// cite the instruction each statement stands for.
#include "bsp/cc13_left55_seg56a.hpp"

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace bsp::cc13_left55::seg56a {
namespace {

void* game(ImageAccess& image)
{
    return image.global_ptr(kGameGlobal);
}

void* offset(void* base, std::uint32_t off)
{
    return static_cast<std::uint8_t*>(base) + off;
}

// [block+4Ch] + slot * 58h, with the signed slot of SUB ESI,1 / IMUL EBP,ESI,58h.
void* slot_record(void* slots, std::int32_t slot)
{
    return static_cast<std::uint8_t*>(slots) + static_cast<std::ptrdiff_t>(slot) * 0x58;
}

float global_f32(ImageAccess& image, std::uint32_t address)
{
    const std::int32_t bits = image.global_i32(address);
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The squadron member walk every squadron binding inlines: slots 0..4 come from +3D0h, a slot
// past the fifth yields a null plane (CMP reg,4 / JA / XOR reg,reg).
void* squadron_member(ImageAccess& image, void* squadron, std::int32_t slot)
{
    return static_cast<std::uint32_t>(slot) <= 4u
        ? image.read_ptr(squadron, 0x3D0u + 4u * static_cast<std::uint32_t>(slot))
        : nullptr;
}

// The entity-table tail shared by FindEntityByID, GetPlaneSquadron and GetSquadronLandedBase:
// a null entity pushes nil (00B66430), otherwise the uint16 at +174h goes through the
// thisTable push.
int push_entity_or_nil(void* entity, LuaResults& results, ImageAccess& image, Left55Seg56aHost& host)
{
    if (entity == nullptr) {
        results.push_nil();
    } else {
        host.push_this_table_entry_004260b0_00b663d0(
            static_cast<std::uint16_t>(image.read_i32(entity, 0x174u) & 0xFFFF)); // MOVZX word
    }
    return 1;
}

// The zero-vector command target 0089A7C5..0089A80B / 008A7896..008A78E3: kind 0,
// position_valid 0, object_id 0, object null, position = the floats at 00F87574, trailing 0.
SceneCommandTarget zero_target(ImageAccess& image)
{
    SceneCommandTarget target;
    target.kind = 0;
    target.position_valid = 0;
    target.object_id = 0;
    target.object = nullptr;
    target.position[0] = global_f32(image, kZeroVectorGlobal);
    target.position[1] = global_f32(image, kZeroVectorGlobal + 4u);
    target.position[2] = global_f32(image, kZeroVectorGlobal + 8u);
    target.trailing = 0.0f;
    return target;
}

int issue_fixed_command(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host,
    std::uint32_t command)
{
    void* entity = args.entity_at(0);
    const SceneCommandTarget target = zero_target(image);
    host.entity_issue_command_0077d600(entity,
        reinterpret_cast<void*>(static_cast<std::uintptr_t>(command)), target, 1); // PUSH 1
    return 0;
}

// The inlined world-position read: refresh when byte +C8h is clear, then +FCh/+100h/+104h.
void world_position(ImageAccess& image, Left55Seg56aHost& host, void* entity, float out[3])
{
    if (image.read_u8(entity, 0xC8u) == 0) {
        host.entity_pose_refresh_world_00414db0(entity);
    }
    out[0] = image.read_f32(entity, 0xFCu);
    out[1] = image.read_f32(entity, 0x100u);
    out[2] = image.read_f32(entity, 0x104u);
}

} // namespace

// 00892F70 HitTarget. Reads arg 0 entity (ESI), arg 1 number truncated by __ftol 00BF7420 (EDI),
// arg 2 entity (EBP), arg 3 and arg 4 vectors (00888760), arg 5 number stored as float
// (0089317B), arg 6 boolean; then 0072C170 at 008931DE. Nothing is pushed.
int lua_hit_target(LuaArgs& args, Left55Seg56aHost& host)
{
    void* gun = args.entity_at(0);
    // 008930AA get_number, 008930AF __ftol: truncation toward zero, low dword kept.
    const auto value = static_cast<std::int32_t>(static_cast<std::int64_t>(args.get_number(1)));
    void* target = args.entity_at(2);
    const Vec3 point_a = args.read_vector3(3);
    const Vec3 point_b = args.read_vector3(4);
    const float number = static_cast<float>(args.get_number(5)); // FSTP float 0089317B
    const bool flag = args.get_boolean(6);
    host.gun_compute_aim_for_target_0072c170(gun, value, target, point_a, point_b, number, flag);
    return 0;
}

// 00893380 AddWaterLoad.
int lua_add_water_load(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* unit = args.entity_at(0);                // EDI
    const std::int32_t side = args.get_integer(1); // ESI
    // 008934D2..008934E6: odd -> 1, even -> -1 (AND 80000001h, NEG/SBB, AND EBX(2), ADD -1).
    const std::int32_t x_sign = (side % 2 != 0) ? 1 : -1;
    // 008934E9..00893506: side < 2 -> 1, side < 4 -> 0, otherwise -1.
    const std::int32_t z_sign = side < 2 ? 1 : (side < 4 ? 0 : -1);
    void* hull = image.read_ptr(unit, 0x538u); // 0089350A
    // 00893510..00893557: FIMUL by the int and FMUL by double 0.5 (DC-form, per the decompiler);
    // multiplying a float by +-1/0 and by 0.5 is exact, so double arithmetic gives the same float.
    float point[3];
    point[0] = static_cast<float>(static_cast<double>(image.read_f32(hull, 0xA4u)) * x_sign * kWaterLoadHalf);
    point[1] = 0.0f; // XORPS / MOVSS 00893531
    point[2] = static_cast<float>(static_cast<double>(image.read_f32(hull, 0xA0u)) * z_sign * kWaterLoadHalf);
    const float amount = static_cast<float>(args.get_number(2)); // FSTP float 0089356F
    // 00893583..0089358D: FCOMIP, JBE skips unless amount > 0 (a NaN skips too).
    if (amount > 0.0f && !host.entity_vcall_5c(unit, 8)) { // 00893598 PUSH 8, JNZ skips
        float scaled = static_cast<float>(amount / kWaterLoadDivisor); // FDIV double, FSTP float
        if (0.0f > scaled) {                                           // 008935B8..008935BC
            scaled = 0.0f;
        } else if (static_cast<double>(scaled) > kWaterLoadClampTest) { // 008935C5..008935D1
            scaled = kWaterLoadClampValue;
        }
        // 008935ED..00893611: FISTP qword under a truncating control word; JBE on the low dword.
        const auto count = static_cast<std::int32_t>(static_cast<std::int64_t>(scaled));
        if (count != 0) {
            host.route_hull_impact_message_0080ff80_0077c2a0(unit, count, point,
                kWaterLoadRouteFlags, 0);
        }
    }
    return 0;
}

// 008945F0 SetWeaponDirectorTargetEnable. Arg 0 is read through 00888BE0 ("Ptr" userdata, no id
// lookup), not 00888AA0.
int lua_set_weapon_director_target_enable(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* owner = host.argument_ptr_field_00888be0(0); // EDI, 008946F3
    const std::int32_t column = args.get_integer(1);  // EBP
    std::int32_t row = -1;                              // OR ESI,FFFFFFFFh 0089473F
    bool value;
    if (args.count() == 4) {                            // 00894747
        row = args.get_integer(2);
        value = args.get_boolean(3);
    } else {
        value = args.get_boolean(2);
    }
    host.flag_grid_set_cell_00861c20(image.read_ptr(owner, 0x6DCu), column, row, value); // 008947C7
    return 0;
}

// 00894C00 IsAreaEmpty. Arg 1 (radius) is read before arg 0 (the centre).
int lua_is_area_empty(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56aHost& host)
{
    const float r = static_cast<float>(args.get_number(1)); // FSTP float
    const Vec3 v = args.read_vector3(0);
    float centre[3] = {v.x, 0.0f, v.z};                       // MOVSS [ESP+30h],XMM0(0) 00894D58
    bool empty = true;                                        // MOV byte [ESP+1Ch],1 00894D60
    // 00894D54..00894D99: min = centre - r, max = r + centre, per component (y = 0).
    const float box_min[3] = {centre[0] - r, 0.0f - r, centre[2] - r};
    const float box_max[3] = {r + centre[0], r + 0.0f, r + centre[2]};
    void* index = host.spatial_index_singleton_0042e630();
    if (host.spatial_index_query_box_0098b450(index, box_min, box_max)) { // 00894DAE, JNZ
        empty = false;
    } else {
        void* world = image.read_ptr(game(image), kGameWorldOffset);
        float height = 0.0f;
        // Three probes: the centre, centre + r, centre - r (all three components, y from 0).
        // Each blocks when it hits and the height is above -10 (COMISS / JA 00894DE7, 00894E55,
        // JBE 00894EBF).
        const float plus[3] = {centre[0] + r, centre[1] + r, r + centre[2]};   // 00894DED..
        const float minus[3] = {centre[0] - r, centre[1] - r, centre[2] - r};  // 00894E57..
        if (host.world_ground_height_at_00903860(world, centre, &height) && height > kAreaGroundThreshold) {
            empty = false;
        } else if (host.world_ground_height_at_00903860(world, plus, &height) && height > kAreaGroundThreshold) {
            empty = false;
        } else if (host.world_ground_height_at_00903860(world, minus, &height) && height > kAreaGroundThreshold) {
            empty = false;
        }
    }
    results.push_boolean(empty); // 00894ECF
    return 1;
}

// 00895ED0 SetAirBaseSlot.
int lua_set_air_base_slot(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false); // XOR DL,DL 00895FF4
    const std::int32_t slot = args.get_integer(1) - 1;                   // SUB ESI,1 00896024
    const std::int32_t count = args.get_integer(3);                      // EBP, arg 3 before arg 2
    void* vehicle_class = nullptr;                                        // 0089609F
    if (count != 0) {
        vehicle_class = host.vehicle_class_007b8a80(args.get_integer(2)); // 00896094
    }
    std::int32_t equipment = -1; // MOV [ESP+10h],FFFFFFFFh 008960C2
    if (args.count() > 4) {
        equipment = args.get_integer(4);
    }
    host.air_ops_update_slot_006cd350(block, slot, vehicle_class, count, equipment); // 00896115
    // 0089611A..00896127: the slot record is [block+4Ch] + slot*58h, state dword at +2Ch.
    void* slots = image.read_ptr(block, 0x4Cu);
    if (image.read_i32(slot_record(slots, slot), 0x2Cu) == 5) {
        host.air_ops_slot_command_006ccda0(block, slot, 2, 0, 9, 1); // PUSH 1,9,0,EBX(2),ESI
    }
    host.air_ops_fill_slot_006c4780(block, slot, vehicle_class, count); // 00896145
    if (args.count() > 4) {
        const std::int32_t value = args.get_integer(4); // read a second time, 00896172
        void* record = slot_record(image.read_ptr(block, 0x4Cu), slot);
        host.air_slot_set_10_006ba4b0(record, value); // 00896196
    }
    host.route_airbase_setslot_message_006bd800_0077c2a0(image.read_ptr(block, 0x7Cu), block, slot,
        0, 0); // MOV EDI,[EDI+7Ch] 008961A6, PUSH 0, PUSH 0
    return 0;
}

// 008970B0 RemoveWrecks. Walks the list at [[game+19CCh]+43Ch] (next at +4h, entity at +8h) and
// kills each with cause 1 (EBX); the next link is read after the kill (008971B9).
int lua_remove_wrecks(ImageAccess& image, Left55Seg56aHost& host)
{
    void* world = image.read_ptr(game(image), kGameWorldOffset);
    for (void* node = image.read_ptr(world, 0x43Cu); node != nullptr; node = image.read_ptr(node, 0x4u)) {
        host.mission_entity_kill_00926d90(image.read_ptr(node, 0x8u), 1);
    }
    return 0;
}

// 008973C0 RemoveAirBasePlanes.
int lua_remove_air_base_planes(LuaArgs& args, Left55Seg56aHost& host)
{
    void* block = host.air_ops_block_006bcd20(args.entity_at(0), false);      // XOR DL,DL 008974DC
    void* vehicle_class = host.vehicle_class_00964790(args.get_integer(1), true); // MOV DL,1 00897506
    const std::int32_t count = args.get_integer(2);
    host.air_ops_remove_stock_006c7b10(block, vehicle_class, count); // 00897543
    return 0;
}

// 008990B0 FindEntityByID.
int lua_find_entity_by_id(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56aHost& host)
{
    // MOVZX EAX,SI 008991C6, then a signed compare with [00F89A10] (JGE).
    const std::int32_t id = args.get_integer(0) & 0xFFFF;
    void* entity;
    if (id < image.global_i32(kIdSplitGlobal)) {
        entity = image.read_ptr(image.global_ptr(kIdLowTableGlobal),
            static_cast<std::uint32_t>((id - image.global_i32(kIdLowBaseGlobal)) << 4) + 0xCu);
    } else {
        entity = image.read_ptr(image.global_ptr(kIdHighTableGlobal),
            static_cast<std::uint32_t>((id - image.global_i32(kIdHighBaseGlobal)) << 4) + 0xCu);
    }
    return push_entity_or_nil(entity, results, image, host);
}

// 0089A660 Order.
int lua_order(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);                                             // ESI
    void* command = host.command_registry_find_by_name_00467170(args.get_string(1)); // EDI
    if (args.count() == 2) {                                                      // CMP EAX,EBX(2)
        const SceneCommandTarget target = zero_target(image);
        host.entity_issue_command_0077d600(entity, command, target, 1);
    } else {
        const SceneCommandTarget target = host.read_command_target_0088a810(2); // 0089A831
        host.entity_issue_command_0077d600(entity, command, target, 1);
    }
    return 0;
}

// 0089B260 SetCommandBuildingLevel.
int lua_set_command_building_level(LuaArgs& args, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    CommandBuildingLevelMessage message;
    message.level = args.get_integer(1); // EDI
    if (args.count() == 3) {
        message.argument_2 = args.get_integer(2); // EBP, read before 0075B430 (0089B3E9)
    } else {
        message.argument_2 = 0; // EBP is 0 on this path (0089B461)
    }
    host.route_message_0077c2a0(entity, message, 7, 0);
    return 0;
}

// 0089B9D0 ForceFireDepthCharge. Walks the child list at +48h (next at +44h); a child of class 24h
// whose [+3F4h]+80h is 8 gets vtable +1F0h.
int lua_force_fire_depth_charge(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* unit = args.entity_at(0);
    for (void* child = image.read_ptr(unit, 0x48u); child != nullptr; child = image.read_ptr(child, 0x44u)) {
        if (host.entity_vcall_5c(child, 0x24) && image.read_i32(image.read_ptr(child, 0x3F4u), 0x80u) == 8) {
            host.entity_vcall_1f0(child);
        }
    }
    return 0;
}

// 0089BB70 KillTorpedoesFromUnit. Walks the world's entity list from [[[game+19CCh]+8h]] (next at
// +44h, read before the class test at 0089BCA5) and kills (cause 2, EBX) every class-2Bh entity
// whose +3BCh is the unit.
int lua_kill_torpedoes_from_unit(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* unit = args.entity_at(0); // EBP
    void* world = image.read_ptr(game(image), kGameWorldOffset);
    void* node = image.read_ptr(image.read_ptr(world, 0x8u), 0x0u);
    while (node != nullptr) {
        void* next = image.read_ptr(node, 0x44u);
        if (host.entity_vcall_5c(node, 0x2B) && image.read_ptr(node, 0x3BCu) == unit) {
            host.mission_entity_kill_00926d90(node, 2);
        }
        node = next;
    }
    return 0;
}

// 0089BD20 ForceEquippedOrdnanceType. Writes the integer to +96Ch of every squadron member.
int lua_force_equipped_ordnance_type(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    const std::int32_t type = args.get_integer(1);
    for (std::int32_t slot = 0; slot < image.read_i32(squadron, 0x3CCu); ++slot) {
        image.write_i32(squadron_member(image, squadron, slot), 0x96Cu, type); // 0089BE86
    }
    return 0;
}

// 0089BEF0 ReplaceDeviceWithEntity.
int lua_replace_device_with_entity(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* unit = args.entity_at(0);                 // EBX
    const std::int32_t id = args.get_integer(1);    // EBP
    // 0089C03F..0089C066: the first class-1Eh child whose +38Ch is the id, else null (ESI = 0).
    void* device = nullptr;
    for (void* child = image.read_ptr(unit, 0x48u); child != nullptr; child = image.read_ptr(child, 0x44u)) {
        if (host.entity_vcall_5c(child, 0x1E) && image.read_i32(child, 0x38Cu) == id) {
            device = child;
            break;
        }
    }
    void* replacement = host.argument_ptr_field_00b67800_00b662d0(2); // EBP, 0089C08C/0089C09B
    // 0089C0C4..0089C118: the inlined world-matrix cache of the device.
    void* world_matrix = offset(device, 0xCCu);
    if (image.read_u8(device, 0xC8u) == 0) {
        void* parent = image.read_ptr(device, 0x3Cu);
        if (parent != nullptr) {
            host.entity_pose_refresh_world_00414db0(parent); // 0089C0DC
        }
        const void* source;
        float product[16];
        parent = image.read_ptr(device, 0x3Cu); // re-read at 0089C0E1
        if (parent != nullptr) {
            source = host.matrix_multiply_00413920(offset(device, 0x74u), product, offset(parent, 0xCCu));
        } else {
            source = offset(device, 0x74u);
        }
        host.matrix_copy_004134f0(world_matrix, source); // 0089C10C
        image.write_u8(device, 0xC8u, 1);
        image.write_u8(device, 0x10Cu, 0);
    }
    host.entity_set_world_pose_matrix_00741e90(replacement, world_matrix); // 0089C122
    // 0089C127..0089C140: replacement->vtable[40h](unit->vtable[34h]()); the +34h call is made
    // first (0089C139), the +40h slot is loaded from the replacement's vtable before it.
    const Vec3 value = host.entity_vcall_34(unit);
    host.entity_vcall_40(replacement, value);
    host.entity_sub310_vcall_0c(offset(replacement, 0x310u)); // 0089C151
    host.mission_entity_kill_00926d90(device, 2);             // 0089C157
    return 0;
}

// 0089C1B0 ReplaceSceneBorder. The frame slots are fetched for arg 1 then arg 0, and the vectors
// read in the same order (0089C2D0 arg 1, 0089C2DC arg 0); 004CA760 gets (arg 0, arg 1).
int lua_replace_scene_border(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    const Vec3 second = args.read_vector3(1);
    const Vec3 first = args.read_vector3(0);
    host.game_replace_scene_border_004ca760(game(image), first, second); // 0089C2E8
    return 0;
}

// 0089CAA0 DCEnable. The director comes from vtable +114h before the boolean is read; the send is
// skipped when it is null (CMP ESI,EBP / JZ 0089CBF9).
int lua_dc_enable(LuaArgs& args, Left55Seg56aHost& host)
{
    void* director = host.entity_vcall_114(args.entity_at(0));
    const bool enable = args.get_boolean(1);
    if (director != nullptr) {
        host.weapon_director_send_subkind6_0071e150(director, enable);
    }
    return 0;
}

// 0089D250 GetPlaneSquadron: the entity at plane+9D4h, or nil.
int lua_get_plane_squadron(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56aHost& host)
{
    void* plane = args.entity_at(0);
    return push_entity_or_nil(image.read_ptr(plane, 0x9D4u), results, image, host);
}

// 0089D470 GetSquadronLandedBase: the entity at squadron+3B4h, or nil.
int lua_get_squadron_landed_base(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host)
{
    void* squadron = args.entity_at(0);
    return push_entity_or_nil(image.read_ptr(squadron, 0x3B4u), results, image, host);
}

// 0089E050 PlaneSetGunfire. Null-tests the entity (0089E166) and the squadron's first plane
// (0089E190). A squadron (class 18h) is replaced by [+3D0h]; otherwise it must be a plane (0Fh).
int lua_plane_set_gunfire(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* plane = args.entity_at(0);
    if (plane == nullptr) {
        return 0;
    }
    if (host.entity_vcall_5c(plane, 0x18)) {
        plane = image.read_ptr(plane, 0x3D0u); // 0089E179
    } else if (!host.entity_vcall_5c(plane, 0x0F)) {
        return 0;
    }
    if (plane != nullptr) {
        const bool on = args.get_boolean(1);
        // SETNZ DL / LEA EDX,[EDX+EDX-1]: 1 when on, -1 (FFh) when off.
        image.write_u8(plane, 0x9B0u, static_cast<std::uint8_t>(on ? 1 : 0xFF));
    }
    return 0;
}

// 0089E850 SquadronForceRelease. Members whose byte at +9C0h + 8 * word[00F876B8] is set get the
// release request.
int lua_squadron_force_release(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* squadron = args.entity_at(0);
    for (std::int32_t slot = 0; slot < image.read_i32(squadron, 0x3CCu); ++slot) {
        void* plane = squadron_member(image, squadron, slot);
        const std::uint32_t player = image.global_u16(kLocalPlayerSlotGlobal); // MOVZX 0089E983
        if (image.read_u8(plane, 0x9C0u + player * 8u) != 0) {
            host.unit_request_ordnance_release_007bbba0(squadron_member(image, squadron, slot)); // re-selected 0089E994
        }
    }
    return 0;
}

// 0089ED20 IsInPosition. False without a group (+284h == 0); otherwise 0070EB00 on the group.
int lua_is_in_position(LuaArgs& args, LuaResults& results, ImageAccess& image, Left55Seg56aHost& host)
{
    void* unit = args.entity_at(0);
    if (image.read_ptr(unit, 0x284u) == nullptr) {
        results.push_boolean(false); // PUSH EBP(0) 0089EE42
    } else {
        void* group = image.read_ptr(unit, 0x284u); // re-read at 0089EE56
        const float range = static_cast<float>(args.get_number(1)); // FSTP float [ESP]
        results.push_boolean(host.observer_target_within_range_0070eb00(group, range));
    }
    return 1;
}

// 0089F360 PlaneSetMovieCollision. A plane (class 0Fh) gets byte +9E2h; a squadron (18h) sets it
// on every member.
int lua_plane_set_movie_collision(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const bool value = args.get_boolean(1); // BL
    if (host.entity_vcall_5c(entity, 0x0F)) {
        image.write_u8(entity, 0x9E2u, value ? 1 : 0);
    } else if (host.entity_vcall_5c(entity, 0x18)) {
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            image.write_u8(squadron_member(image, entity, slot), 0x9E2u, value ? 1 : 0);
        }
    }
    return 0;
}

// 0089FAF0 SquadronSetBaseUnsupported. The block is the inlined 006BCD20 without its null case:
// base+72Ch for class 45h, otherwise base+1188h (0089FC46..0089FC4E).
int lua_squadron_set_base_unsupported(LuaArgs& args, Left55Seg56aHost& host)
{
    void* squadron = args.entity_at(0); // EDI
    void* base = args.entity_at(1);     // ESI
    void* block = host.entity_vcall_5c(base, 0x45) ? offset(base, 0x72Cu) : offset(base, 0x1188u);
    host.air_ops_block_push_b8_006ca730(block, squadron); // 0089FC54
    return 0;
}

// 008A0630 SquadronSetTravelSpeed. The optional boolean defaults to false (008A07BA).
int lua_squadron_set_travel_speed(LuaArgs& args, ImageAccess& image)
{
    void* squadron = args.entity_at(0);
    const float speed = static_cast<float>(args.get_number(1)); // FSTP float 008A076C
    bool flag = false;
    if (args.count() == 3) {
        flag = args.get_boolean(2);
    }
    image.write_f32(squadron, 0x388u, kTravelSpeedField388); // 008A07E2
    image.write_u8(squadron, 0x38Fu, flag ? 1 : 0);          // 008A07F0
    image.write_f32(squadron, 0x3A0u, speed);                 // 008A07FA
    image.write_u8(squadron, 0x3ACu, 1);                      // 008A0802
    return 0;
}

// 008A0F10 EntityTurnToPosition.
int lua_entity_turn_to_position(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0); // EDI
    const Vec3 target = args.read_vector3(1);
    if (image.read_u8(entity, 0xC8u) == 0) {
        host.entity_pose_refresh_world_00414db0(entity); // 008A107C
    }
    // 008A1081..008A10C3: the offset from the entity's world position.
    Vec3 delta;
    delta.x = target.x - image.read_f32(entity, 0xFCu);
    delta.y = target.y - image.read_f32(entity, 0x100u);
    delta.z = target.z - image.read_f32(entity, 0x104u);
    bool keep_vertical = false; // 008A10FF
    if (args.count() == 3) {
        keep_vertical = args.get_boolean(2);
    }
    if (!keep_vertical) {
        delta.y = 0.0f; // 008A1128
    }
    // 008A112E..008A11A6: rows (1,0,0,0), (0,1,0,0), (dx,dy,dz,0), (0,0,0,1); then 0085DC80.
    float matrix[16] = {
        kOne, 0.0f, 0.0f, 0.0f,
        0.0f, kOne, 0.0f, 0.0f,
        delta.x, delta.y, delta.z, 0.0f,
        0.0f, 0.0f, 0.0f, kOne,
    };
    host.matrix_orthonormalize_basis_rows_0085dc80(matrix);
    if (image.read_i32(game(image), kSessionModeOffset) != 0 && host.entity_vcall_5c(entity, 2)) {
        if (!host.unit_is_formation_follower_007788b0(entity)) {
            // 008A11E0..008A11E8: FCHS, stored as float.
            const float heading = -host.vector3_compass_heading_007b4e90(delta);
            host.route_relocate_heading_message_007edeb0_0077c2a0(entity, heading, 0, 1, 0);
        }
    } else if (host.entity_vcall_5c(entity, 0x18)) {
        // 008A122D..008A12A2: each member's pose gets the rotation with its own position.
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            void* plane = squadron_member(image, entity, slot);
            world_position(image, host, plane, &matrix[12]);
            host.entity_vcall_88(plane, matrix);
        }
    } else if (host.entity_vcall_5c(entity, 6)) {
        world_position(image, host, entity, &matrix[12]); // 008A12B8..008A12EC
        host.entity_vcall_88(entity, matrix);             // 008A1301
        float transform[12];
        host.dyn_copy_matrix43_00c336c0(transform, matrix); // 008A130F
        void* parts = host.unit_parts_object_0080e490(entity); // 008A1316
        host.dyn_body_set_transform_00c56cf0(image.read_ptr(parts, 0x2Cu), transform); // 008A1328
    } else {
        world_position(image, host, entity, &matrix[12]); // 008A132F..008A1363
        host.entity_vcall_88(entity, matrix);             // 008A1378
    }
    return 0;
}

// 008A1B70 SquadronSetWandererMul. A plane (class 0Fh) gets float +840h; anything else is
// treated as a squadron and every member gets it (no squadron class test).
int lua_squadron_set_wanderer_mul(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const float value = static_cast<float>(args.get_number(1)); // FSTP float 008A1CA5
    if (host.entity_vcall_5c(entity, 0x0F)) {
        image.write_f32(entity, 0x840u, value);
    } else {
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            image.write_f32(squadron_member(image, entity, slot), 0x840u, value);
        }
    }
    return 0;
}

// 008A1D60 SquadronSetWandererEnabled: the same shape with byte +844h.
int lua_squadron_set_wanderer_enabled(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const bool value = args.get_boolean(1); // BL
    if (host.entity_vcall_5c(entity, 0x0F)) {
        image.write_u8(entity, 0x844u, value ? 1 : 0);
    } else {
        for (std::int32_t slot = 0; slot < image.read_i32(entity, 0x3CCu); ++slot) {
            image.write_u8(squadron_member(image, entity, slot), 0x844u, value ? 1 : 0);
        }
    }
    return 0;
}

// 008A54C0 PlaneBombFallingTime: 007BCC80(plane, world y) pushed as a float.
int lua_plane_bomb_falling_time(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host)
{
    void* plane = args.entity_at(0);
    if (image.read_u8(plane, 0xC8u) == 0) {
        host.entity_pose_refresh_world_00414db0(plane);
    }
    const float time = host.weapon_drop_fall_time_007bcc80(plane, image.read_f32(plane, 0x100u));
    results.push_float(time); // FSTP float [ESP] 008A55FC
    return 1;
}

// 008A5870 PilotStartAttackRun.
int lua_pilot_start_attack_run(LuaArgs& args, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const StartAttackRunMessage message;
    host.route_message_0077c2a0(entity, message, 0, 0); // PUSH EBP, PUSH EBP
    return 0;
}

// 008A5A20 SetAirbaseSmallPlanes. block+20h = 1 when the flag is set, else 2; skipped when the
// entity has no block (CMP EAX,EBP / JZ 008A5B73).
int lua_set_airbase_small_planes(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const bool small_planes = args.get_boolean(1);
    void* block = host.air_ops_block_006bcd20(entity, false); // XOR DL,DL, after the read
    if (block != nullptr) {
        image.write_i32(block, 0x20u, small_planes ? 1 : 2); // NEG BL / SBB / ADD 2
    }
    return 0;
}

// 008A5D60 GetSquadFlyTime. Arg 0 through 00888D20 ("Ptr"); 007EE150 receives it only when it is
// non-null and of class 18h, otherwise null (008A5E76..008A5E8D).
int lua_get_squad_fly_time(LuaResults& results, Left55Seg56aHost& host)
{
    void* squadron = host.argument_ptr_field_00888d20(0);
    void* argument = (squadron != nullptr && host.entity_vcall_5c(squadron, 0x18)) ? squadron : nullptr;
    results.push_float(host.squadron_min_member_908_007ee150(argument));
    return 1;
}

// 008A5EF0 IsPlayerControlled. Arg 0 through 00888D20 ("Ptr"), no null test.
int lua_is_player_controlled(LuaResults& results, Left55Seg56aHost& host)
{
    void* unit = host.argument_ptr_field_00888d20(0);
    results.push_boolean(host.unit_is_local_players_role_slot_00779b60(unit));
    return 1;
}

// 008A62A0 SetCheatTurbo. A squadron (18h) writes float +340h on members 0..4 and stops at the
// first null slot, without reading the member count (008A6406..008A641F); otherwise a class-5
// entity gets +340h itself.
int lua_set_cheat_turbo(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    void* entity = args.entity_at(0);
    const float value = static_cast<float>(args.get_number(1)); // FSTP float 008A63D5
    if (host.entity_vcall_5c(entity, 0x18)) {
        for (std::uint32_t slot = 0; slot <= 4u; ++slot) {
            void* plane = image.read_ptr(entity, 0x3D0u + 4u * slot);
            if (plane == nullptr) {
                break;
            }
            image.write_f32(plane, 0x340u, value);
        }
    } else if (host.entity_vcall_5c(entity, 5)) {
        image.write_f32(entity, 0x340u, value);
    }
    return 0;
}

// 008A6640 UnitGetFireStance. From the director's bytes +3Ch/+3Dh: +3Ch set -> 1 + (+3Dh != 0);
// clear -> 3 when +3Dh is set, else 0.
int lua_unit_get_fire_stance(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host)
{
    void* director = host.entity_vcall_114(args.entity_at(0));
    const std::uint8_t b3d = image.read_u8(director, 0x3Du); // MOV CL,[EAX+3Dh] 008A6766
    std::int32_t stance;
    if (image.read_u8(director, 0x3Cu) != 0) {
        stance = (b3d != 0 ? 1 : 0) + 1; // SETNZ / ADD 1
    } else {
        stance = b3d != 0 ? 3 : 0;       // NEG / SBB / AND 3
    }
    results.push_number(stance);
    return 1;
}

// 008A6C30 UnitSetPlayerCommandsEnabled. The director is fetched before the integer is read.
int lua_unit_set_player_commands_enabled(LuaArgs& args, Left55Seg56aHost& host)
{
    void* director = host.entity_vcall_114(args.entity_at(0));
    const std::int32_t value = args.get_integer(1);
    host.weapon_director_send_subkind2_0071dd30(director, value);
    return 0;
}

// 008A7780 PilotStop: 0077D600 with the `stop` command object 00E08F88 and the zero target.
int lua_pilot_stop(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    return issue_fixed_command(args, image, host, kCommandObjectStop);
}

// 008A7940 PilotClearOrders: the same with `clearorders` 00E08F08.
int lua_pilot_clear_orders(LuaArgs& args, ImageAccess& image, Left55Seg56aHost& host)
{
    return issue_fixed_command(args, image, host, kCommandObjectClearOrders);
}

// 008A8280 GetRelativeSpeed. Both entity reads use argument 0 (PUSH EBP at 008A835E and 008A8397),
// so `observer` and `other` are the same entity in the native.
int lua_get_relative_speed(LuaArgs& args, LuaResults& results, ImageAccess& image,
    Left55Seg56aHost& host)
{
    void* observer = args.entity_at(0); // ESI
    void* other = args.entity_at(0);    // EDI
    if (image.read_u8(observer, 0x10Cu) == 0) {
        host.entity_pose_refresh_world_00414db0(observer);                        // 008A83D2
        image.write_u8(observer, 0x10Cu, 1);                                       // 008A83E3
        host.matrix_affine_inverse_00b63d50(offset(observer, 0x110u), offset(observer, 0xCCu)); // 008A83EA
    }
    // 008A83EF..008A840B: other->vtable[34h](&tmp) (RET 4), then 0042D0D0(out, tmp,
    // observer+110h, 0); the two 0042D0D0 stack arguments are pushed before the virtual call.
    const Vec3 velocity = host.entity_vcall_34(other);
    const Vec3 local = host.vector3_transform_direction_0042d0d0(velocity, offset(observer, 0x110u), 0);
    // 008A8410..008A842A: observer->vtable[38h]() + local.x, stored as float.
    const float speed = host.entity_vcall_38(observer) + local.x;
    results.push_float(speed);
    return 1;
}

} // namespace bsp::cc13_left55::seg56a
