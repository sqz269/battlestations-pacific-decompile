// Packet cc13-front4: see include/bsp/cc13_front4.hpp and docs/CC13_FRONT4.md.
// Each function was read from the listing; x87 operand direction from the bytes.
#include "bsp/cc13_front4.hpp"

#include <cmath>
#include <cstddef>
#include <cstdint>

namespace bsp::cc13_front4 {

namespace {

constexpr std::uint32_t kInterfaceNameTable = 0x00E08CD8;  // char*[] indexed by interface id
constexpr std::uint32_t kLookAtEye = 0x00F87574;           // float[3], BSS (the zero vector)
constexpr std::int32_t kIntfScene3d = 0x20;
constexpr char kPopRequestFormat[] = "GVMultiMenu::PopRequestInterface() req:%s";  // 00CF77F4

constexpr double kLengthSquaredFloor_00ce3820 = 1e-10;  // double at 00CE3820
constexpr double kLengthFloor_00d7a350 = 1e-08;         // double at 00D7A350
constexpr float kNegZero_00d7a208 = -0.0f;              // float at 00D7A208
constexpr float kOne_00d7a24c = 1.0f;                   // float at 00D7A24C
constexpr float kTwoPi_00ce3d9c = 6.2831854820251465f;  // float at 00CE3D9C (0x40C90FDB)

template <class T>
T& at(void* object, std::size_t off) noexcept
{
    return *reinterpret_cast<T*>(static_cast<std::uint8_t*>(object) + off);
}

// The checked `*(--end())` of the vector whose first/last words are menu+5Ch/+60h,
// as 006881FB..0068822B and 0068823F..00688272 inline it.
std::int32_t checked_back(Front4Host& host, Raw menu)
{
    std::uint8_t* it = menu.at<std::uint8_t*>(0x60);
    if (menu.at<std::uint8_t*>(0x5C) > it) host.invalid_parameter_00bf6713();  // CMP/JBE
    std::uint8_t* prev = it - 4;
    if (prev > menu.at<std::uint8_t*>(0x60) || prev < menu.at<std::uint8_t*>(0x5C))  // JA / JNC
        host.invalid_parameter_00bf6713();
    it = prev;
    if (!(it < menu.at<std::uint8_t*>(0x60))) host.invalid_parameter_00bf6713();  // JC
    return *reinterpret_cast<std::int32_t*>(it);
}

// One x87 store: the value reaches the float slot it is read back from.
inline float f32(double v) noexcept { return static_cast<float>(v); }

} // namespace

// 0068AA40. Each early-out jumps to the RET 4 at 0068AA84.
void ingame_interface_request_scene_for_unit_0068aa40(Front4Host& host, Raw manager, void* unit)
{
    if (unit == manager.ptr(0x38)) return;  // 0068AA44 the pending payload already
    if (unit == nullptr) return;
    if (at<std::uint8_t>(unit, 0x5C) == 0) return;
    if (at<std::uint8_t>(unit, 0x5D) != 0) return;
    if (at<std::uint8_t>(unit, 0x60) != 0) return;
    if (at<std::uint8_t>(unit, 0x5E) != 0) return;
    const std::int32_t pending = manager.at<std::int32_t>(0x20);
    if (pending == 0x2B || pending == 0x2C || pending == 0x2D || pending == 0x29) return;
    host.frontend_push_interface_request_004cc460(manager, kIntfScene3d, unit);  // 0068AA7F, ECX unchanged
}

// 006881F0. The back element is read twice, once for the log line and once for
// the request; the vector is popped only after 004CC460 returns, re-reading the
// first/last words.
void multi_menu_pop_request_interface_006881f0(Front4Host& host, Raw menu)
{
    const std::int32_t logged = checked_back(host, menu);
    const char* const* names = static_cast<const char* const*>(host.global_address(kInterfaceNameTable));
    host.debug_printf_004254b0(kPopRequestFormat, names[logged]);  // 0068823A

    const std::int32_t id = checked_back(host, menu);
    host.frontend_push_interface_request_004cc460(menu, id, nullptr);  // 00688279

    // 0068827E: pop_back unless the vector is unallocated or empty (SAR 2 of the span).
    std::uint8_t* const first = menu.at<std::uint8_t*>(0x5C);
    if (first != nullptr) {
        std::uint8_t* const last = menu.at<std::uint8_t*>(0x60);
        if (((last - first) >> 2) != 0) menu.at<std::uint8_t*>(0x60) = last - 4;
    }
}

// 00928570. EDX = entity; the list is parent+24h, nodes {+4 next, +8 value} from
// the first node at parent+28h. Only the first node holding the entity is erased.
void game_entity_unlink_from_parent_list_00928570(Front4Host& host, Raw entity)
{
    std::uint8_t* const parent = entity.at<std::uint8_t*>(0x30);
    for (void* node = at<void*>(parent, 0x28); node != nullptr; node = at<void*>(node, 4)) {
        if (at<void*>(node, 8) == entity.p) {
            host.unit_list_erase_node_004837d0(Raw{parent + 0x24}, node);  // 0092858E
            return;
        }
    }
}

// 009258F0. Gated once by the byte +BCh, set at the end.
void scene_node_attach_to_parents_009258f0(Front4Host& host, Raw instance, void* hierarchy_parent,
                                           void* world_node, const float* matrix)
{
    if (instance.at<std::uint8_t>(0xBC) != 0) return;  // 009258FC

    // 00925906: world node, then append to its list [world+4] ({head, tail, count}, links +34h prev / +38h next).
    instance.ptr(0x30) = world_node;
    std::uint8_t* list = at<std::uint8_t*>(world_node, 4);
    if (at<std::uint32_t>(list, 8) != 0)
        at<void*>(at<void*>(list, 4), 0x38) = instance.p;
    else
        at<void*>(list, 0) = instance.p;
    instance.ptr(0x34) = at<void*>(list, 4);
    at<void*>(list, 4) = instance.p;
    instance.ptr(0x38) = nullptr;
    at<std::uint32_t>(list, 8) += 1;

    // 0092592F: hierarchy parent (children +48h head, +4Ch tail, +50h count; links +40h prev / +44h next),
    // or with no parent the world node's list [[+30h]+8].
    instance.ptr(0x3C) = hierarchy_parent;
    if (hierarchy_parent != nullptr) {
        if (at<std::uint32_t>(hierarchy_parent, 0x50) != 0)
            at<void*>(at<void*>(hierarchy_parent, 0x4C), 0x44) = instance.p;
        else
            at<void*>(hierarchy_parent, 0x48) = instance.p;
        instance.ptr(0x40) = at<void*>(hierarchy_parent, 0x4C);
        at<void*>(hierarchy_parent, 0x4C) = instance.p;
        instance.ptr(0x44) = nullptr;
        at<std::uint32_t>(hierarchy_parent, 0x50) += 1;
    } else {
        list = at<std::uint8_t*>(instance.ptr(0x30), 8);
        if (at<std::uint32_t>(list, 8) != 0)
            at<void*>(at<void*>(list, 4), 0x44) = instance.p;
        else
            at<void*>(list, 0) = instance.p;
        instance.ptr(0x40) = at<void*>(list, 4);
        at<void*>(list, 4) = instance.p;
        instance.ptr(0x44) = nullptr;
        at<std::uint32_t>(list, 8) += 1;
    }

    // 0092598E: hand the pending counters +B8h/+B4h on, cleared before the call.
    const std::uint32_t b8 = instance.at<std::uint32_t>(0xB8);
    if (b8 != 0 || instance.at<std::uint32_t>(0xB4) != 0) {
        const std::uint32_t b4 = instance.at<std::uint32_t>(0xB4);
        instance.at<std::uint32_t>(0xB4) = 0;
        instance.at<std::uint32_t>(0xB8) = 0;
        host.entity_adjust_world_list_counters_009245a0(instance, b8, b4);  // 009259B6
    }

    host.matrix_copy_004134f0(reinterpret_cast<float*>(instance.field(0x74)), matrix);  // 009259C4
    instance.at<std::uint8_t>(0xC8) = 0;
    instance.at<std::uint8_t>(0x10C) = 0;
    for (void* child = instance.ptr(0x48); child != nullptr; child = at<void*>(child, 0x44))
        host.scene_node_invalidate_subtree_pose_0042ed50(child);  // 009259E2
    instance.at<std::uint8_t>(0xBC) = 1;
}

std::int32_t material_texture_count_00b179c0(Raw material)
{
    return material.at<std::int16_t>(0x34);  // MOVSX EAX,word ptr [ECX+34h]
}

std::uint32_t object_word_0c_00b17370(Raw object)
{
    return object.at<std::uint32_t>(0x0C);
}

std::uint32_t render_service_word_1cc_00b0d1c0(Raw service)
{
    return service.at<std::uint32_t>(0x1CC);
}

std::uint32_t render_service_word_1d4_00b0d0d0(Raw service)
{
    return service.at<std::uint32_t>(0x1D4);
}

// 0085E4D0. Frame offsets are from ESP after SUB ESP,1CCh.
bool matrix_rotate_by_angular_velocity_0085e4d0(Front4Host& host, float* out, const float* in, float wx,
                                                float wy, float wz, float dt)
{
    // 0085E4D6..0085E506: (wx*wx + wy*wy) + wz*wz on the x87 stack, stored as a float.
    // Products of floats are exact in double; the sums are rounded to double here
    // instead of the 64-bit x87 mantissa before the final float store.
    const float length_sq = f32((static_cast<double>(wx) * wx + static_cast<double>(wy) * wy) +
                                static_cast<double>(wz) * wz);
    // 0085E514 FCOMI / JBE: an unordered or small square takes the zero arm.
    float length = 0.0f;
    if (static_cast<double>(length_sq) > kLengthSquaredFloor_00ce3820)
        length = f32(std::sqrt(static_cast<double>(length_sq)));  // 0085E51A 00BF7030 _CIsqrt
    // 0085E542 FCOMIP / JA: leave when 1e-8 > length (ordered); NaN continues.
    if (kLengthFloor_00d7a350 > static_cast<double>(length)) return false;

    // 0085E54A..0085E588: the axis, divided in place in the argument slots (FDIV, FDIV, FDIVR).
    float axis[3] = {wx / length, wy / length, wz / length};

    // 0085E58F..0085E6C7: |c| is c when c > 0 (ordered), else -0.0f - c (00D7A208).
    const auto magnitude = [](float c) { return c > 0.0f ? c : kNegZero_00d7a208 - c; };
    float up_x = 0.0f, up_y = 0.0f, up_z = 0.0f;  // XMM0, XMM6, XMM5
    if (magnitude(axis[0]) > magnitude(axis[1]) && magnitude(axis[0]) > magnitude(axis[2]))
        up_y = kOne_00d7a24c;  // 0085E62F, x dominant
    else if (magnitude(axis[1]) > magnitude(axis[0]) && magnitude(axis[1]) > magnitude(axis[2]))
        up_z = kOne_00d7a24c;  // 0085E6BD, y dominant
    else
        up_x = kOne_00d7a24c;  // 0085E6C7

    float look_at[16];      // ESP+CCh, reused for the rotation copy
    float look_at_copy[16]; // ESP+8Ch
    float inverse[16];      // ESP+0Ch, transposed in place
    float scratch[16];      // ESP+4Ch: the rotation, then the copy of `in`
    float product_a[16];    // ESP+18Ch
    float product_b[16];    // ESP+14Ch
    float product_c[16];    // ESP+10Ch

    float* const built = host.matrix_build_look_at_00b63f10(
        look_at, static_cast<const float*>(host.global_address(kLookAtEye)), axis, up_x, up_y, up_z);
    host.matrix_copy_004134f0(look_at_copy, built);    // 0085E703
    host.matrix_copy_004134f0(inverse, look_at_copy);  // 0085E714

    // 0085E719..0085E758: angle = -(float)(length * dt).
    const float angle = -f32(static_cast<double>(length) * dt);
    // 0085E71D..0085E7C4: transpose `inverse` in place.
    for (int r = 0; r < 4; ++r)
        for (int c = r + 1; c < 4; ++c) {
            const float t = inverse[r * 4 + c];
            inverse[r * 4 + c] = inverse[c * 4 + r];
            inverse[c * 4 + r] = t;
        }

    float* const rotation = host.matrix_rotation_z_00b64780(scratch, &angle);  // 0085E7CA, EDX = &angle
    host.matrix_copy_004134f0(look_at, rotation);                              // 0085E7D7

    for (int i = 0; i < 16; ++i) scratch[i] = in[i];  // 0085E7EC REP MOVSD

    // 0085E816..0085E826: ((in * look_at) * rotation) * transpose(look_at).
    float* m = host.matrix_multiply_00413920(in, product_a, look_at_copy);
    m = host.matrix_multiply_00413920(m, product_b, look_at);
    m = host.matrix_multiply_00413920(m, product_c, inverse);
    host.matrix_copy_004134f0(out, m);  // 0085E82E

    // 0085E833..0085E85A: the translation row from the copy of `in`.
    out[12] = scratch[12];
    out[13] = scratch[13];
    out[14] = scratch[14];
    host.matrix_orthonormalize_0085d3d0(out);  // 0085E85F
    return true;
}

// 00860BE0. h = radius + radius (FADD ST0,ST0, stored as a float). Each axis line
// runs from centre - h to centre + h on its axis; the other two coordinates are
// written as c - 0 and c + 0, as the binary computes them. Both line colours are
// the 16 bytes at EDX.
void debug_draw_wire_sphere_marker_00860be0(Front4Host& host, const float* centre, const float* rgba,
                                            float radius, std::uint32_t arc_rgba, std::uint8_t detailed)
{
    const float h = radius + radius;
    const float cx = centre[0], cy = centre[1], cz = centre[2];
    {
        const float p0[3] = {cx - 0.0f, cy - h, cz - 0.0f};  // ESP+14h
        const float p1[3] = {cx + 0.0f, cy + h, cz + 0.0f};  // ESP+8
        host.debug_draw_line_rgba_floats_0085f990(p0, p1, rgba, rgba);  // 00860C98
    }
    {
        const float p0[3] = {cx - h, cy - 0.0f, cz - 0.0f};
        const float p1[3] = {cx + h, cy + 0.0f, cz + 0.0f};
        host.debug_draw_line_rgba_floats_0085f990(p0, p1, rgba, rgba);  // 00860D3F
    }
    {
        const float p0[3] = {cx - 0.0f, cy - 0.0f, cz - h};
        const float p1[3] = {cx + 0.0f, cy + 0.0f, cz + h};
        host.debug_draw_line_rgba_floats_0085f990(p0, p1, rgba, rgba);  // 00860DE4
    }

    // 00860DFF NEG/SBB/AND 3Ch/ADD 4.
    const std::uint32_t segments = detailed != 0 ? 0x40u : 4u;
    const float x_axis[3] = {kOne_00d7a24c, 0.0f, 0.0f};
    host.debug_draw_arc_around_axis_00860ad0(centre, x_axis, radius, 0.0f, kTwoPi_00ce3d9c, arc_rgba,
                                             segments);  // 00860E3C
    const float y_axis[3] = {0.0f, kOne_00d7a24c, 0.0f};
    host.debug_draw_arc_around_axis_00860ad0(centre, y_axis, radius, 0.0f, kTwoPi_00ce3d9c, arc_rgba,
                                             segments);  // 00860E80
    const float z_axis[3] = {0.0f, 0.0f, kOne_00d7a24c};
    host.debug_draw_arc_around_axis_00860ad0(centre, z_axis, radius, 0.0f, kTwoPi_00ce3d9c, arc_rgba,
                                             segments);  // 00860EC4
}

// 005CD160. When the byte +4 is clear: request interface 2Bh for the current unit
// [00E188D8] on the manager [00E198C4], build the MovCam if needed, and target it
// at that unit (the unit read once, before the request).
void* movcam_ensure_target_005cd160(Front4Host& host, Raw screen)
{
    if (screen.at<std::uint8_t>(4) == 0) {
        void* const unit = host.global_pointer(0x00E188D8);
        host.frontend_push_interface_request_004cc460(Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00E198C4))},
                                                      0x2B, unit);  // 005CD17A
        host.hud_movie_screen_ensure_movcam_005cc000(screen);       // 005CD181
        host.movcam_set_target_007a6910(screen.ptr(0x30), unit);   // 005CD18A
    }
    return screen.ptr(0x30);
}

std::uint8_t overlay_busy_006529e0(Raw overlay) { return overlay.at<std::uint8_t>(4); }
std::int32_t vehicle_class_equipment_count_00951f10(Raw descriptor) { return descriptor.at<std::int32_t>(0x128); }
bool device_iterator_at_end_00952140(Raw, std::uint32_t, Raw iterator)
{
    return iterator.at<std::uint32_t>(4) == 0;  // XOR EAX,EAX; CMP [ECX+4],EAX; SETZ AL
}
void* landing_pad_occupant_006ac220(Raw pad) { return pad.ptr(0x1F8); }
std::uint8_t storage_gate_00a3e540(Raw owner) { return owner.at<std::uint8_t>(0x120); }
std::uint8_t plane_byte_94_007d7a80(Raw plane) { return plane.at<std::uint8_t>(0x94); }
float water_amount_00939f80(Raw object) { return object.at<float>(0x34); }
float fire_amount_00939f70(Raw object) { return object.at<float>(0x38); }
void* unit_parts_object_0080e490(Raw unit) { return unit.ptr(0x1018); }
void* atmosphere_cloud_manager_00bbddf0(Raw atmosphere) { return atmosphere.ptr(0x3C); }

void* unit_weapon_director_0080e160(Raw unit) { return unit.ptr(0x738); }

// 007D1E50. Base constructor first, then the two vptrs and the zero stores in the
// binary's order (dwords unless noted). The SEH frame has no state store; nothing
// here can throw after the base constructor.
void* plane_class_construct_base_007d1e50(Front4Host& host, Raw object)
{
    host.vehicle_class_construct_base_00749050(object);  // 007D1E69, EAX ignored
    object.at<std::uint32_t>(0x00) = 0x00D05EAC;
    object.at<std::uint32_t>(0x6C) = 0x00D05EA8;
    static constexpr std::uint16_t kDwords1[] = {0x138, 0x13C, 0x14C, 0x150, 0x154};
    for (const std::uint16_t off : kDwords1) object.at<std::uint32_t>(off) = 0;
    object.at<std::uint8_t>(0x1F8) = 0;
    static constexpr std::uint16_t kDwords2[] = {0x210, 0x214, 0x218, 0x220, 0x228, 0x22C, 0x230, 0x234,
                                                 0x238, 0x23C, 0x240, 0x250, 0x254, 0x258, 0x544, 0x548,
                                                 0x54C, 0x554, 0x558, 0x55C, 0x564, 0x568, 0x56C, 0x5A4,
                                                 0x5BC, 0x5C0, 0x5C4};
    for (const std::uint16_t off : kDwords2) object.at<std::uint32_t>(off) = 0;
    object.at<std::uint8_t>(0x5D4) = 0;
    object.at<std::uint8_t>(0x5E4) = 0;
    object.at<std::uint8_t>(0x5F4) = 0;
    return object.p;
}

// 00A1EEF0. vptr 00D22D08; while the group list at +20h (sentinel [+24h], count
// +28h) is non-empty, take its first group: when 00A1C8B0 says the planner owns it,
// unregister the observer pair, remove the group from the list and clear
// group+5654h. A group the planner does not own is not removed, so the loop takes
// it again (as the binary does). Then the list is cleared, the string block at
// +18h (capacity +14h) goes back to the sized pool, and the base destructor runs.
void ai_planner_destruct_base_00a1eef0(Front4Host& host, Raw planner)
{
    planner.at<std::uint32_t>(0) = 0x00D22D08;  // 00A1EF0F
    while (planner.at<std::uint32_t>(0x28) != 0) {
        std::uint8_t* const sentinel = planner.at<std::uint8_t*>(0x24);
        std::uint8_t* const first = at<std::uint8_t*>(sentinel, 0);
        if (first == sentinel) host.invalid_parameter_00bf6713();  // 00A1EF31
        void* const group = at<void*>(first, 8);
        void* value = group;  // the stack slot whose address 00A1D1D0 receives
        if (host.ai_planner_owns_group_00a1c8b0(planner, group)) {
            host.observer_unregister_pair_006952a0(group, planner.p);  // 00A1EF4D
            host.stl_list_remove_00a1d1d0(Raw{planner.field(0x20)}, &value);
            at<std::uint32_t>(group, 0x5654) = 0;
        }
    }
    host.stl_list_clear_00a16b60(Raw{planner.field(0x20)});  // 00A1EF73
    void* const block = planner.ptr(0x18);
    if (block != nullptr) {
        const std::uint32_t size = planner.at<std::uint32_t>(0x14) + 1;
        void* const pool = host.sized_storage_pool_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(pool, block, size, 1);  // 00A1EF95
    }
    host.callback_owner_destroy_00695870(planner);  // 00A1EFA4
}

// 00A10DC0. The group is [command+4]; its member list proxy is group+563Ch
// (sentinel [+5640h], count [+5644h]; nodes +0 next, +8 member). Runs when the
// count is above 1 (unsigned), from the node after the first (the leader). The
// iterator's container is captured once; each pass re-reads [command+4] for the
// end and the container check.
void ai_command_follower_pass_00a10dc0(Front4Host& host, Raw command)
{
    std::uint8_t* group = command.at<std::uint8_t*>(4);
    if (at<std::uint32_t>(group, 0x5644) <= 1) return;  // 00A10DD4 JBE
    std::uint8_t* const container = group + 0x563C;
    std::uint8_t* const sentinel = at<std::uint8_t*>(group, 0x5640);
    std::uint8_t* const first = at<std::uint8_t*>(sentinel, 0);
    if (first == sentinel) host.invalid_parameter_00bf6713();  // 00A10DF3
    for (std::uint8_t* node = at<std::uint8_t*>(first, 0);; node = at<std::uint8_t*>(node, 0)) {
        group = command.at<std::uint8_t*>(4);  // 00A10E00
        std::uint8_t* const end = at<std::uint8_t*>(group, 0x5640);
        if (container == nullptr || container != group + 0x563C) host.invalid_parameter_00bf6713();
        if (node == end) return;  // 00A10E1D
        if (container == nullptr) host.invalid_parameter_00bf6713();
        if (node == at<std::uint8_t*>(container, 4)) host.invalid_parameter_00bf6713();  // 00A10E31
        void* const member = at<void*>(node, 8);
        if (host.vcall_entity_is_kind_5c(member, 6)) {
            // 00A10E48: join the formation of the list's first member.
            std::uint8_t* const g = command.at<std::uint8_t*>(4);
            std::uint8_t* const s = at<std::uint8_t*>(g, 0x5640);
            std::uint8_t* const head = at<std::uint8_t*>(s, 0);
            if (head == s) host.invalid_parameter_00bf6713();  // 00A10E5C
            host.entity_request_join_formation_0077c8d0(member, at<void*>(head, 8));  // 00A10E67
        } else if (host.vcall_entity_is_kind_5c(member, 0x18)) {
            if (!host.ai_command_send_empty_squadron_home_009ffeb0(member)) {  // 00A10E83
                const float* const point = host.ai_group_leader_point_00a10c20(command.ptr(4));  // 00A10E8F
                host.ai_command_issue_move_to_member_00a02020(member, point);              // 00A10E98
            }
        }
        if (node == at<std::uint8_t*>(container, 4)) host.invalid_parameter_00bf6713();  // 00A10EA2
    }
}

} // namespace bsp::cc13_front4
