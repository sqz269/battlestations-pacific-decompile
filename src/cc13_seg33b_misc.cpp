// Segment 33 upper half (packet cc13-seg33b; marker cc13_seg33b): the MCatapult device, the
// LuaVariant map/vector template instances, the debug side swap and the fog-cloud model pool.
// See include/bsp/cc13_seg33b.hpp and docs/CC13_SEG33B.md. Every routine names its address,
// its original ABI and what was simplified.

#include "bsp/cc13_seg33b.hpp"

#include <initializer_list>

#include <cstring>

namespace bsp::cc13_seg33b {
namespace {

TaggedValue key_string(const char* text) { return TaggedValue{0, addr(text)}; }
TaggedValue int_value(std::int32_t value) { return TaggedValue{1, static_cast<std::uint32_t>(value)}; }
TaggedValue int_cell(Object cell) { return TaggedValue{1, addr(cell)}; }
TaggedValue bool_cell(Object cell) { return TaggedValue{3, addr(cell)}; }

// The observer member's target swap the image inlines (and 006F30A0 / 006F3320 / 006F34F0
// spell out): target at member+14h; unregister the old pair, store, register the new one.
void observer_set_target(Seg33bHost& host, Object member, Object target) {
    Object old = obj(at<std::uint32_t>(member, 0x14));
    if (old == target) return;
    if (old != nullptr) host.observer_unregister_006952a0(old, member);
    at<std::uint32_t>(member, 0x14) = addr(target);
    if (target != nullptr) host.observer_register_00694a60(target, member);
}

// The two-bank entity id table (inlined at 006ECB56..006ECB8D): ids below [00F89A10] index
// bank 0, the rest bank 1; 10h-byte records with the entity at +0Ch. Signed compare (JGE).
Object entity_by_id(Seg33bHost& host, std::uint32_t id) {
    if (static_cast<std::int32_t>(id) < static_cast<std::int32_t>(host.global_u32(kIdSplit))) {
        const std::uint32_t slot = (id - host.global_u32(kIdBank0Base)) << 4;
        return obj(at<std::uint32_t>(obj(host.global_u32(kIdBank0Records)), slot + 0xC));
    }
    const std::uint32_t slot = (id - host.global_u32(kIdBank1Base)) << 4;
    return obj(at<std::uint32_t>(obj(host.global_u32(kIdBank1Records)), slot + 0xC));
}

}  // namespace

// ===========================================================================================
// MCatapult
// ===========================================================================================

// 006EC140: the gun's fields, then a "_catapult" section with the four catapult fields. The
// value of startLaunch is passed as {3, byte}; the image writes only the low byte of the value
// dword (006EC1D2), the upper three bytes are stack contents (modelled as 0).
void catapult_debug_dump_006ec140(Seg33bHost& host, Object catapult, Object visitor) {
    host.gun_debug_dump_fields_0072adc0(catapult, visitor);
    host.visitor_begin_section_4(visitor, key_string("_catapult"));  // 00CFAA64
    host.visitor_write_field_c(visitor, key_string("ammo"), int_value(at<std::int32_t>(catapult, kCatapultAmmo)));
    host.visitor_write_field_c(visitor, key_string("orgAmmo"),
                               int_value(at<std::int32_t>(catapult, kCatapultOrgAmmo)));
    host.visitor_write_field_c(visitor, key_string("startLaunch"),
                               TaggedValue{3, at<std::uint8_t>(catapult, kCatapultStartLaunch)});
    host.visitor_write_field_c(visitor, key_string("launchTick"),
                               int_value(at<std::int32_t>(catapult, kCatapultLaunchTick)));
    host.visitor_end_section_8(visitor);
}

// 006EC240: angles are -0.0f (00D7A208) minus the yaw +4B4h / pitch +4B8h (SUBSS, so a zero
// gives -0.0f).
//  * With the base node +3BCh and a barrel node +3C8h: the base keeps its local position
//    (00B6DB60 row 3) and takes the yaw rotation (00B646E0, copied by 004134F0) through its
//    vtable +38h, then the position through +2Ch; the barrel likewise with the pitch (00B64640).
//  * With the base node only: the base takes pitch * yaw (00413920(ECX = pitch)(out, yaw)) and
//    its own position.
//  * Then when the launched squadron (+4A8h) has a plane (+3D0h) in state 3 (+900h): P is the
//    identity with row 3 (0, [owner+E8h], +4CCh); P = (P * pitch) * yaw (two 00413920 calls);
//    row 3 += the catapult's +A4h..+ACh; the plane's vtable +88h(P).
void catapult_pose_006ec240(Seg33bHost& host, Object catapult) {
    constexpr float kNegZero = -0.0f;  // 00D7A208
    Object base = obj(at<std::uint32_t>(catapult, 0x3BC));
    if (base != nullptr) {
        float position[3];
        float angle;
        float temp[16];
        float matrix[16];
        if (at<std::uint32_t>(catapult, 0x3C8) != 0) {
            const float* local = host.node_local_matrix_00b6db60(base);
            position[0] = local[12];
            position[1] = local[13];
            position[2] = local[14];
            angle = kNegZero - at<float>(catapult, 0x4B4);
            host.matrix_copy_004134f0(matrix, host.matrix_rotation_y_00b646e0(temp, &angle));
            host.node_set_local_matrix_38(obj(at<std::uint32_t>(catapult, 0x3BC)), matrix);
            host.node_set_position_2c(obj(at<std::uint32_t>(catapult, 0x3BC)), position);
            local = host.node_local_matrix_00b6db60(obj(at<std::uint32_t>(catapult, 0x3C8)));
            position[0] = local[12];
            position[1] = local[13];
            position[2] = local[14];
            angle = kNegZero - at<float>(catapult, 0x4B8);
            host.matrix_copy_004134f0(matrix, host.matrix_rotation_x_00b64640(temp, &angle));
            host.node_set_local_matrix_38(obj(at<std::uint32_t>(catapult, 0x3C8)), matrix);
            host.node_set_position_2c(obj(at<std::uint32_t>(catapult, 0x3C8)), position);
        } else {
            const float* local = host.node_local_matrix_00b6db60(base);
            position[0] = local[12];
            position[1] = local[13];
            position[2] = local[14];
            angle = kNegZero - at<float>(catapult, 0x4B4);
            host.matrix_copy_004134f0(matrix, host.matrix_rotation_y_00b646e0(temp, &angle));
            float pitch_angle = kNegZero - at<float>(catapult, 0x4B8);
            float temp2[16];
            float pitch[16];
            host.matrix_copy_004134f0(pitch, host.matrix_rotation_x_00b64640(temp2, &pitch_angle));
            float product[16];
            const float* combined = host.matrix_multiply_00413920(pitch, product, matrix);
            host.node_set_local_matrix_38(obj(at<std::uint32_t>(catapult, 0x3BC)), combined);
            host.node_set_position_2c(obj(at<std::uint32_t>(catapult, 0x3BC)), position);
        }
    }
    Object squadron = obj(at<std::uint32_t>(catapult, kCatapultLaunched));
    if (squadron == nullptr) return;
    Object plane = obj(at<std::uint32_t>(squadron, 0x3D0));
    if (plane == nullptr || at<std::int32_t>(plane, 0x900) != 3) return;
    const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
    float pose[16];
    host.matrix_copy_004134f0(pose, identity);
    pose[12] = 0.0f;
    pose[13] = at<float>(obj(at<std::uint32_t>(catapult, 0x3F4)), 0xE8);
    pose[14] = at<float>(catapult, 0x4CC);
    float angle = kNegZero - at<float>(catapult, 0x4B4);
    float temp[16];
    float yaw[16];
    host.matrix_copy_004134f0(yaw, host.matrix_rotation_y_00b646e0(temp, &angle));
    angle = kNegZero - at<float>(catapult, 0x4B8);
    float temp2[16];
    float pitch[16];
    host.matrix_copy_004134f0(pitch, host.matrix_rotation_x_00b64640(temp2, &angle));
    float first[16];
    float second[16];
    const float* step = host.matrix_multiply_00413920(pose, first, pitch);
    host.matrix_copy_004134f0(pose, host.matrix_multiply_00413920(step, second, yaw));
    const float x = at<float>(catapult, 0xA4) + pose[12];
    const float y = at<float>(catapult, 0xA8) + pose[13];
    const float z = at<float>(catapult, 0xAC) + pose[14];
    pose[12] = x;
    pose[13] = y;
    pose[14] = z;
    host.plane_set_pose_88(obj(at<std::uint32_t>(obj(at<std::uint32_t>(catapult, kCatapultLaunched)), 0x3D0)), pose);
}

// 006EC640: the gun base (0072E510 with the caller's argument), then the catapult vtables, the
// observer member at +494h (vtable 00CF6514, target +4A8h null, flag +4A4h = 1) and zeroed
// state; class id +C4h = 28h; launchTick = -1. Store order follows the image.
Object catapult_construct_006ec640(Seg33bHost& host, Object memory, std::uint32_t argument) {
    host.gun_construct_0072e510(memory, argument);
    at<std::uint8_t>(memory, 0x488) = 0;
    at<std::uint8_t>(memory, 0x489) = 0;
    at<std::uint32_t>(memory, 0x0) = kCatapultVtable;
    at<std::uint32_t>(memory, 0x10) = kCatapultVtable10;
    at<std::uint32_t>(memory, 0x24) = kCatapultVtable24;
    at<std::uint32_t>(memory, 0x170) = kCatapultVtable170;
    at<std::uint32_t>(memory, 0x1E4) = kCatapultVtable1E4;
    at<std::uint32_t>(memory, 0x310) = kCatapultVtable310;
    at<std::int32_t>(memory, 0x490) = -1;
    at<std::uint32_t>(memory, 0x498) = 0;
    at<std::uint32_t>(memory, 0x49C) = 0;
    at<std::uint32_t>(memory, 0x4A0) = 0;
    at<std::uint32_t>(memory, 0x4A8) = 0;
    at<std::uint8_t>(memory, 0x4A4) = 1;
    at<std::uint32_t>(memory, 0x494) = kObserverMemberVtable;
    at<std::uint32_t>(memory, 0x480) = 0;
    at<std::uint32_t>(memory, 0x484) = 0;
    at<std::uint32_t>(memory, 0xC4) = 0x28;
    for (std::size_t off : {0x48Cu, 0x4C8u, 0x4C4u, 0x4C0u, 0x4BCu, 0x4B8u, 0x4B4u, 0x4B0u, 0x4ACu, 0x4D4u,
                            0x4D0u, 0x4CCu}) {
        at<float>(memory, off) = 0.0f;  // MOVSS of a zeroed XMM0
    }
    return memory;
}

// 006EC7B0: true for 28h MCatapult, 20h, 1Eh, 4, 2, 1, 0 and the instance's own id at +C4h.
bool catapult_is_kind_of_006ec7b0(Object catapult, std::int32_t kind) {
    return kind == 0x28 || kind == 0x20 || kind == 0x1E || kind == 4 || kind == 2 || kind == 1 || kind == 0 ||
           kind == at<std::int32_t>(catapult, 0xC4);
}

// 006EC7F0: clean up the observer member (00653390, ECX = +494h), then the gun base (0072D950).
// The SEH frame only orders the base destructor after a throwing member cleanup.
void catapult_destruct_body_006ec7f0(Seg33bHost& host, Object catapult) {
    host.observer_member_cleanup_00653390(catapult + kCatapultObserver);
    host.gun_destruct_0072d950(catapult);
}

// 006EC860: operator new(4D8h) (00BF55BE), memset 0, construct (006EC640, null block -> null),
// then owner into +3F4h and +354h. The image writes those two fields through the result even
// when the allocation failed (no null test after the constructor), reproduced here. The SEH
// unwind that frees the block when the constructor throws is not modelled.
Object catapult_create_006ec860(Seg33bHost& host, Object owner, std::uint32_t argument) {
    Object memory = host.operator_new_00bf55be(kCatapultSize);
    host.memset_00bf79f0(memory, 0, kCatapultSize);
    Object catapult = memory == nullptr ? nullptr : catapult_construct_006ec640(host, memory, argument);
    at<std::uint32_t>(catapult, 0x3F4) = addr(owner);
    at<std::uint32_t>(catapult, 0x354) = addr(owner);
    return catapult;
}

// 006ECB50: the message's u16 entity id (+20h) becomes the observed squadron (+494h member,
// target +4A8h), its float +24h goes to +48Ch, then 006EC070 with the message's +0Ch.
void catapult_apply_launch_message_006ecb50(Seg33bHost& host, Object catapult, Object message) {
    const std::uint32_t id = at<std::uint16_t>(message, 0x20);
    Object squadron = entity_by_id(host, id);
    observer_set_target(host, catapult + kCatapultObserver, squadron);
    at<float>(catapult, 0x48C) = at<float>(message, 0x24);
    host.catapult_after_launch_006ec070(catapult, at<std::uint32_t>(message, 0xC));
}

// 006ECBE0: the device's health and parts (0043F7B0); one barrel: +448h = 1, +44Ch = 0,
// +414h = new[](4) (00BF55BE) holding -1.0f (00D7A260), +450h = 0.0f, +4CCh = +4D0h = owner
// [+3F4h]+E4h; AI bots (0072C6A0) in game mode 0 or 1. With a device owner (+3F0h): its
// arc record (as 006ED000 finds it, growing the array through 005471B0) gives +4ACh, +4BCh,
// +4B4h, +4C4h = record+94h and +4B0h, +4C0h, +4B8h, +4C8h = record+90h, each 0.0f when it
// equals FLT_MAX (double 00D7A278, FUCOMIP). When the class flag [+30h]+4A4h is set: the
// launch node +3BCh is the owner's existing node ([owner+38h] set: [[+360h]+160h]+0Ch, model
// null when +360h is) or a new 174h-byte node (00B6ED70 / 00B6F5A0 named after [owner+54h]);
// it gets the mount frame (0072DD20) as its local matrix (vtable +38h) and the parent
// [+3Ch]->vtable[18h]() (00B6E680); on the existing-node path "base" and "barrel" note nodes
// (0071AD50) go to +3C4h/+3C8h (both zeroed first). Then the mount frame is copied into +74h
// (004134F0), +C8h and +10Ch cleared, every child (+48h, next +44h) invalidated (0042ED50),
// vtable +1BCh([owner+538h]+C8h), and with +358h set vtable +194h("destroyed", 0). Then, on
// both paths, +3CCh = +3C8h, else +3C4h, else +3BCh; every barrel's reload timer is set to
// 0.0f (0072CF00(i, 0, 0)) and 00729700 runs. Unwinds are not modelled.
void catapult_init_device_006ecbe0(Seg33bHost& host, Object catapult) {
    host.device_init_health_and_parts_0043f7b0(catapult);
    at<std::int32_t>(catapult, 0x448) = 1;
    at<std::uint32_t>(catapult, 0x44C) = 0;
    Object timers = host.operator_new_00bf55be(4);
    at<std::uint32_t>(catapult, 0x414) = addr(timers);
    at<float>(timers, 0x0) = -1.0f;  // 00D7A260
    at<float>(catapult, 0x450) = 0.0f;
    const float e4 = at<float>(obj(at<std::uint32_t>(catapult, 0x3F4)), 0xE4);
    at<float>(catapult, 0x4CC) = e4;
    at<float>(catapult, 0x4D0) = e4;
    const std::int32_t mode = at<std::int32_t>(obj(host.global_u32(kGameGlobals)), 0x1FE4);
    if (mode == 0 || mode == 1) host.gun_create_ai_bots_0072c6a0(catapult);
    Object device = obj(at<std::uint32_t>(catapult, 0x3F0));
    if (device != nullptr) {
        Object arrays = obj(at<std::uint32_t>(device, 0x538)) + 0x94;
        const std::int32_t index = at<std::int32_t>(catapult, 0x38C);
        if (index >= at<std::int32_t>(arrays, 0x4)) host.pointer_array_resize_005471b0(arrays, index + 1);
        Object* cell = reinterpret_cast<Object*>(obj(at<std::uint32_t>(arrays, 0x0)) + index * 4);
        const double sentinel = 3.4028234663852886e+38;  // double 00D7A278
        const float a = static_cast<double>(at<float>(*cell, 0x94)) == sentinel ? 0.0f : at<float>(*cell, 0x94);
        for (std::size_t off : {0x4ACu, 0x4BCu, 0x4B4u, 0x4C4u}) at<float>(catapult, off) = a;
        const float b = static_cast<double>(at<float>(*cell, 0x90)) == sentinel ? 0.0f : at<float>(*cell, 0x90);
        for (std::size_t off : {0x4B0u, 0x4C0u, 0x4B8u, 0x4C8u}) at<float>(catapult, off) = b;
    }
    if (at<std::uint8_t>(obj(at<std::uint32_t>(catapult, 0x30)), 0x4A4) != 0) {
        Object owner = obj(at<std::uint32_t>(catapult, 0x3F4));
        at<std::uint32_t>(catapult, 0x3BC) = 0;
        at<std::uint32_t>(catapult, 0x3C8) = 0;
        at<std::uint32_t>(catapult, 0x3C4) = 0;
        float frame[16];
        auto model_of = [&]() -> Object {
            Object scene = obj(at<std::uint32_t>(catapult, 0x360));
            return scene != nullptr ? obj(at<std::uint32_t>(scene, 0x160)) : nullptr;
        };
        if (at<std::uint32_t>(owner, 0x38) != 0) {
            Object node = obj(at<std::uint32_t>(model_of(), 0xC));
            at<std::uint32_t>(catapult, 0x3BC) = addr(node);
            host.node_set_local_matrix_38(node, host.gun_platform_mount_frame_0072dd20(catapult, frame));
            Object parent = host.object_virtual_18(obj(at<std::uint32_t>(catapult, 0x3C)));
            host.node_set_parent_00b6e680(obj(at<std::uint32_t>(catapult, 0x3BC)), parent);
            at<std::uint32_t>(catapult, 0x3C4) = 0;
            at<std::uint32_t>(catapult, 0x3C8) = 0;
            at<std::uint32_t>(catapult, 0x3C4) = addr(host.model_find_note_node_0071ad50(model_of(), "base"));
            at<std::uint32_t>(catapult, 0x3C8) = addr(host.model_find_note_node_0071ad50(model_of(), "barrel"));
        } else {
            Object memory = host.node_allocate_00b6ed70(0x174);
            Object node = nullptr;
            bool named = false;
            alignas(4) std::uint8_t name[8] = {};  // {length, data}
            if (memory != nullptr) {
                host.native_string_assign_0041e870(
                    name, reinterpret_cast<const char*>(obj(at<std::uint32_t>(owner, 0x54))));
                named = true;
                node = host.node_construct_00b6f5a0(memory, name);
            }
            at<std::uint32_t>(catapult, 0x3BC) = addr(node);
            if (named && obj(at<std::uint32_t>(name, 0x4)) != nullptr) {
                host.pool_release_00419cc0_00bd1510(obj(at<std::uint32_t>(name, 0x4)), at<std::uint32_t>(name, 0x0) + 1);
            }
            node = obj(at<std::uint32_t>(catapult, 0x3BC));
            host.node_set_local_matrix_38(node, host.gun_platform_mount_frame_0072dd20(catapult, frame));
            Object parent = host.object_virtual_18(obj(at<std::uint32_t>(catapult, 0x3C)));
            host.node_set_parent_00b6e680(obj(at<std::uint32_t>(catapult, 0x3BC)), parent);
        }
        host.matrix_copy_004134f0(&at<float>(catapult, 0x74), host.gun_platform_mount_frame_0072dd20(catapult, frame));
        at<std::uint8_t>(catapult, 0xC8) = 0;
        at<std::uint8_t>(catapult, 0x10C) = 0;
        for (Object child = obj(at<std::uint32_t>(catapult, 0x48)); child != nullptr;
             child = obj(at<std::uint32_t>(child, 0x44))) {
            host.scene_node_invalidate_0042ed50(child);
        }
        host.unit_virtual_1bc(catapult,
                              at<std::uint32_t>(obj(at<std::uint32_t>(obj(at<std::uint32_t>(catapult, 0x3F0)), 0x538)), 0xC8));
        if (at<std::uint32_t>(catapult, 0x358) != 0) {
            alignas(4) std::uint8_t destroyed[8] = {};
            host.native_string_assign_0041e870(destroyed, "destroyed");  // 00CE44D0
            host.unit_virtual_194(catapult, destroyed, 0);
            if (obj(at<std::uint32_t>(destroyed, 0x4)) != nullptr) {
                host.pool_release_00419cc0_00bd1510(obj(at<std::uint32_t>(destroyed, 0x4)),
                                                    at<std::uint32_t>(destroyed, 0x0) + 1);
            }
        }
    }
    std::uint32_t launch = at<std::uint32_t>(catapult, 0x3C8);
    if (launch == 0) {
        launch = at<std::uint32_t>(catapult, 0x3C4);
        if (launch == 0) launch = at<std::uint32_t>(catapult, 0x3BC);
    }
    at<std::uint32_t>(catapult, 0x3CC) = launch;
    for (std::int32_t i = 0; i < at<std::int32_t>(catapult, 0x448); ++i) {
        host.gun_set_barrel_reload_timer_0072cf00(catapult, i, 0.0f, 0);
    }
    host.gun_apply_descriptor_node_flag_00729700(catapult);
}

// 006ED000: wrap the heading into (-pi, pi] (fmod by 2pi, then +2pi when <= -pi, -2pi when
// > pi; x87, stored as float), then over the device's arc table (owner [+3F0h]+538h, a pointer
// array at +94h grown by 005471B0 to index [+38Ch]; arcs {flags, edge +4h, edge +8h} 14h
// apart at record+3Ch, count +40h) with flag bit 1: a heading inside an arc's window
// (007F5960) returns at once; otherwise keep the nearer edge (|00438B10(heading, edge)|) when
// it beats the best so far (initially 10.0f, 00CE38B8). The fallback answer is +4ACh.
// Uncertainty: the image adds/subtracts 2pi in x87 extended precision; this uses double.
float catapult_launch_heading_006ed000(Seg33bHost& host, Object catapult, float heading) {
    float result = at<float>(catapult, kCatapultDefaultHeading);
    float best = kLaunchHeadingWindow;
    Object arrays = obj(at<std::uint32_t>(obj(at<std::uint32_t>(catapult, 0x3F0)), 0x538)) + 0x94;
    const std::int32_t index = at<std::int32_t>(catapult, 0x38C);
    if (index >= at<std::int32_t>(arrays, 0x4)) {
        host.pointer_array_resize_005471b0(arrays, index + 1);
    }
    Object* cell = reinterpret_cast<Object*>(obj(at<std::uint32_t>(arrays, 0x0)) + index * 4);
    heading = static_cast<float>(host.fmod_00bf857a(heading, kTwoPi));
    if (!(kMinusPi < heading)) {
        heading = static_cast<float>(heading + kTwoPi);
    } else if (heading > kPi) {
        heading = static_cast<float>(heading - kTwoPi);
    }
    Object arc = obj(at<std::uint32_t>(*cell, 0x3C));
    while (arc != obj(at<std::uint32_t>(*cell, 0x3C)) + at<std::int32_t>(*cell, 0x40) * 0x14) {
        if (((at<std::uint8_t>(arc, 0x0) >> 1) & 1) != 0) {
            if (host.gun_arc_angle_in_window_007f5960(arc, heading)) {
                return heading;
            }
            float d1 = host.subtract_wrapped_angle_00438b10(heading, at<float>(arc, 0x4));
            d1 = d1 < 0.0f ? -d1 : d1;  // AND 7FFFFFFFh
            float d2 = host.subtract_wrapped_angle_00438b10(heading, at<float>(arc, 0x8));
            d2 = d2 < 0.0f ? -d2 : d2;
            if (d2 < d1) {
                if (d2 < best) {
                    best = d2;
                    result = at<float>(arc, 0x8);
                }
            } else if (d1 < best) {
                best = d1;
                result = at<float>(arc, 0x4);
            }
        }
        arc += 0x14;
    }
    return result;
}

// 006ED690: the gun's saved state (0072F060); for a saved-game creation record (+C0h with kind
// 3 at +4h) read the "_catapult" table: ammo, orgAmmo, startLaunch, launchTick into
// +480h/+484h/+488h/+490h. Then an empty magazine (+480h == 0) sets barrel 0's reload timer
// to FLT_MAX (00CFA9D8) with set_full 0. The reader lives in the frame at ESP+8h (14h bytes
// up to the LuaObject argument); its size here is that frame span.
void catapult_load_saved_state_006ed690(Seg33bHost& host, Object catapult) {
    host.gun_load_saved_state_0072f060(catapult);
    Object record = obj(at<std::uint32_t>(catapult, 0xC0));
    if (record != nullptr && at<std::int32_t>(record, 0x4) == 3) {
        alignas(4) std::uint8_t lua_object[0x14] = {};
        alignas(4) std::uint8_t reader[0x14] = {};
        host.saved_entity_lua_data_009238a0(catapult, lua_object);
        host.lua_reader_construct_004425c0(reader, lua_object);
        host.lua_reader_enter_00bd8e20(reader, key_string("_catapult"));                   // 00CFAA64
        host.lua_reader_read_00bd6830(reader, key_string("ammo"), int_cell(catapult + 0x480));  // 00CF9B7C
        host.lua_reader_read_00bd6830(reader, key_string("orgAmmo"), int_cell(catapult + 0x484));
        host.lua_reader_read_00bd6830(reader, key_string("startLaunch"), bool_cell(catapult + 0x488));
        host.lua_reader_read_00bd6830(reader, key_string("launchTick"), int_cell(catapult + 0x490));
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_destruct_00441a20(reader);
    }
    if (at<std::int32_t>(catapult, kCatapultAmmo) == 0) {
        float flt_max;
        const std::uint32_t bits = kFltMaxBits;
        static_assert(sizeof(flt_max) == sizeof(bits), "float width");
        std::memcpy(&flt_max, &bits, sizeof(bits));
        host.gun_set_barrel_reload_timer_0072cf00(catapult, 0, flt_max, 0);
    }
}

// ===========================================================================================
// std::map<key, LuaVariant> and std::vector<LuaVariant> instances
// ===========================================================================================

// 006ED890: the comparator's operator(); ECX (the functor) is not used.
bool lua_map_key_less_006ed890(Seg33bHost& host, Object a, Object b) {
    return host.lua_key_less_00884460(a, b);
}

// 006EDE40 / 006EDEF0: ++it through 006EDA20, return it.
TreeIterator* lua_map_iterator_increment_006ede40(Seg33bHost& host, TreeIterator* it) {
    host.tree_iterator_increment_006eda20(it);
    return it;
}
TreeIterator* lua_map_iterator_increment_006edef0(Seg33bHost& host, TreeIterator* it) {
    host.tree_iterator_increment_006eda20(it);
    return it;
}

// 006EDFB0: --it through 006EDD80, return it.
TreeIterator* lua_map_iterator_decrement_006edfb0(Seg33bHost& host, TreeIterator* it) {
    host.tree_iterator_decrement_006edd80(it);
    return it;
}

// 006EDF00: when size == capacity reserve max(2 * capacity, 1) (006B87F0); copy-construct the
// variant at data + size * 14h unless that address is null; ++size. Layout {data, size, cap}.
void lua_variant_vector_push_back_006edf00(Seg33bHost& host, Object vector, Object variant) {
    if (at<std::int32_t>(vector, 0x4) == at<std::int32_t>(vector, 0x8)) {
        std::int32_t count = at<std::int32_t>(vector, 0x8) * 2;
        if (!(count > 1)) count = 1;
        host.lua_variant_vector_reserve_006b87f0(vector, count);
    }
    Object slot = obj(at<std::uint32_t>(vector, 0x0)) + at<std::int32_t>(vector, 0x4) * 0x14;
    if (slot != nullptr) {
        host.lua_variant_copy_construct_00886da0(slot, variant);
    }
    at<std::int32_t>(vector, 0x4) += 1;
}

namespace {
void init_head(Seg33bHost& host, Object map) {
    Object head = host.tree_allocate_head_006edea0(map);
    at<std::uint32_t>(map, 0x4) = addr(head);
    at<std::uint8_t>(head, 0x31) = 1;
    Object h = obj(at<std::uint32_t>(map, 0x4));
    at<std::uint32_t>(h, 0x4) = addr(h);
    at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x0) = at<std::uint32_t>(map, 0x4);
    at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x8) = at<std::uint32_t>(map, 0x4);
    at<std::uint32_t>(map, 0x8) = 0;
}
}  // namespace

// 006EDFC0: head node from 006EDEA0, marked nil, parent/left/right = itself, size 0.
void lua_map_init_head_006edfc0(Seg33bHost& host, Object map) { init_head(host, map); }

// 006EE120: the same as the map constructor (two by-value arguments, unused), returns map.
Object lua_map_construct_006ee120(Seg33bHost& host, Object map) {
    init_head(host, map);
    return map;
}

// 006EE040: the value's destructor (00884AF0 on pair+10h), then the key's string storage
// {length +4h, data +8h} back to the sized pool when data is set.
void lua_map_pair_destruct_006ee040(Seg33bHost& host, Object pair) {
    host.lua_variant_destruct_00884af0(pair + 0x10);
    void* data = obj(at<std::uint32_t>(pair, 0x8));
    if (data != nullptr) {
        host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(pair, 0x4) + 1);
    }
}

// 006EE0B0: resize(0) (006B88E0), then free the buffer (00BF6989).
void lua_variant_vector_tidy_006ee0b0(Seg33bHost& host, Object vector) {
    host.lua_variant_vector_resize_006b88e0(vector, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(vector, 0x0)));
}

// 006EE0D0: key (00884EF0) then value at +10h (00886DA0). The unwind that destroys the key when
// the value copy throws is not modelled.
Object lua_map_pair_construct_006ee0d0(Seg33bHost& host, Object pair, Object key, Object value) {
    host.lua_key_copy_construct_00884ef0(pair, key);
    host.lua_variant_copy_construct_00886da0(pair + 0x10, value);
    return pair;
}

// 006EE150: the same from one pair (key at +0h, value at +10h). Unwind not modelled.
Object lua_map_pair_copy_construct_006ee150(Seg33bHost& host, Object pair, Object from) {
    host.lua_key_copy_construct_00884ef0(pair, from);
    host.lua_variant_copy_construct_00886da0(pair + 0x10, from + 0x10);
    return pair;
}

// 006EE230: operator new(34h) (00BF681B); when non-null: links, the pair copied at +0Ch
// (006EE150), color +30h, is-nil +31h = 0. The unwind that frees the node when the pair copy
// throws is not modelled.
Object lua_map_buy_node_006ee230(Seg33bHost& host, Object left, Object parent, Object right, Object value,
                                 std::uint8_t color) {
    Object node = host.operator_new_00bf681b(0x34);
    if (node != nullptr) {
        at<std::uint32_t>(node, 0x0) = addr(left);
        at<std::uint32_t>(node, 0x4) = addr(parent);
        at<std::uint32_t>(node, 0x8) = addr(right);
        lua_map_pair_copy_construct_006ee150(host, node + 0xC, value);
        at<std::uint8_t>(node, 0x30) = color;
        at<std::uint8_t>(node, 0x31) = 0;
    }
    return node;
}

// 006EE4E0: std::map::insert(value). Walk from the root: go left while key < node key
// (00884460(ECX = new key)(node key)). When the last step went left: at begin() insert left
// at once; otherwise step the candidate back (006EDD80). If candidate key < new key
// (00884460(ECX = candidate key)(new key)) insert at the walk's parent with the last
// direction (006EE2E0); otherwise return {candidate, false}.
TreeInsertResult* lua_map_insert_unique_006ee4e0(Seg33bHost& host, Object map, TreeInsertResult* out,
                                                 Object value) {
    Object parent = obj(at<std::uint32_t>(map, 0x4));  // the head
    Object node = obj(at<std::uint32_t>(parent, 0x4));  // the root
    bool add_left = true;
    while (at<std::uint8_t>(node, 0x31) == 0) {
        parent = node;
        add_left = host.lua_key_less_00884460(value, node + 0xC);
        node = obj(at<std::uint32_t>(node, add_left ? 0x0 : 0x8));
    }
    TreeIterator where{map, parent};
    if (add_left) {
        if (parent == obj(at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x0))) {
            TreeIterator* inserted = host.tree_insert_006ee2e0(map, &where, true, parent, value);
            out->where = *inserted;
            out->inserted = 1;
            return out;
        }
        host.tree_iterator_decrement_006edd80(&where);
    }
    if (host.lua_key_less_00884460(where.node + 0xC, value)) {
        TreeIterator* inserted = host.tree_insert_006ee2e0(map, &where, add_left, parent, value);
        out->where = *inserted;
        out->inserted = 1;
        return out;
    }
    out->where = TreeIterator{where.container, where.node};
    out->inserted = 0;
    return out;
}

// 006EE890: std::map::_Erase: recurse into the right subtree, keep the left child, destroy the
// pair's value (00884AF0 on node+1Ch) and its key string storage (node+10h length, +14h data,
// pool free), free the node (00BF65AC), continue with the left child until a nil node.
void lua_map_erase_subtree_006ee890(Seg33bHost& host, Object map, Object node) {
    while (at<std::uint8_t>(node, 0x31) == 0) {
        lua_map_erase_subtree_006ee890(host, map, obj(at<std::uint32_t>(node, 0x8)));
        Object left = obj(at<std::uint32_t>(node, 0x0));
        Object pair = node + 0xC;
        host.lua_variant_destruct_00884af0(pair + 0x10);
        void* data = obj(at<std::uint32_t>(pair, 0x8));
        if (data != nullptr) {
            host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(pair, 0x4) + 1);
        }
        host.free_00bf65ac(node);
        node = left;
    }
}

// ===========================================================================================
// 006EEB20: publish an input event to the mission script (no static caller; reached by
// pointer)
// ===========================================================================================
// The globals of the game's Lua state (00B67980, ECX = [00E188A8]+1A0Ch), their "Mission"
// table; when it is not nil: Mission.Dead (looked up, never used) and Mission.Ptr's userdata
// (00B662D0); an empty argument vector and an empty std::map<key, LuaVariant>; the pairs
// F = value and Message = 4 (kept until after the call), Shift / Ctrl / Alt = (flag != 0)
// (each destroyed right after its insert); the map wrapped as one variant (00886840) and
// pushed (006EDF00); 009291D0(ECX = userdata)(&vector); then the two kept pairs destroyed,
// the map erased (006EE960 over [begin, end)), its head freed (00BF65AC), the vector emptied
// (006B88E0(0)) and freed (00BF6989), and the four LuaObjects destroyed in reverse. Key
// strings are freed as soon as their pair is built. Every unwind state is not modelled.
void mission_publish_input_event_006eeb20(Seg33bHost& host, std::uint32_t value, std::uint8_t shift,
                                          std::uint8_t ctrl, std::uint8_t alt) {
    alignas(4) std::uint8_t globals[0x14] = {};
    alignas(4) std::uint8_t mission[0x14] = {};
    Object game = obj(host.global_u32(kGameGlobals));
    Object g = host.lua_state_globals_00b67980(game + 0x1A0C, globals);
    host.lua_get_by_name_00b67800(g, mission, "Mission");  // 00CF5148
    if (!host.lua_is_nil_00b65fb0(mission)) {
        alignas(4) std::uint8_t dead[0x14] = {};
        alignas(4) std::uint8_t ptr[0x14] = {};
        host.lua_get_by_name_00b67800(mission, dead, "Dead");  // 00CF829C
        host.lua_get_by_name_00b67800(mission, ptr, "Ptr");    // 00CFAD08
        Object userdata = host.lua_to_userdata_00b662d0(ptr);
        alignas(4) std::uint8_t arguments[0xC] = {};  // std::vector<LuaVariant> {data, size, cap}
        alignas(4) std::uint8_t map[0xC] = {};        // std::map {allocator, head, size}
        Object head = host.tree_allocate_head_006edea0(map);
        at<std::uint32_t>(map, 0x4) = addr(head);
        at<std::uint8_t>(head, 0x31) = 1;
        at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x4) = at<std::uint32_t>(map, 0x4);
        at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x0) = at<std::uint32_t>(map, 0x4);
        at<std::uint32_t>(obj(at<std::uint32_t>(map, 0x4)), 0x8) = at<std::uint32_t>(map, 0x4);
        at<std::uint32_t>(map, 0x8) = 0;

        alignas(4) std::uint8_t key[0x10] = {};
        alignas(4) std::uint8_t variant[0x14] = {};
        alignas(4) std::uint8_t kept_f[0x24] = {};
        alignas(4) std::uint8_t kept_message[0x24] = {};
        alignas(4) std::uint8_t pair[0x24] = {};
        TreeInsertResult result{};
        auto release_key = [&]() {
            void* data = obj(at<std::uint32_t>(key, 0x8));
            if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(key, 0x4) + 1);
        };
        // F = value and Message = 4: pair built, variant destroyed, key freed, then inserted.
        const struct {
            const char* name;
            std::uint32_t value;
            Object pair;
        } kept[] = {{"F", value, kept_f}, {"Message", 4, kept_message}};  // 00CFAD04, 00CFACFC
        for (const auto& k : kept) {
            host.lua_key_from_text_00884d90(key, k.name);
            Object v = host.lua_variant_from_dword_00884a00(variant, k.value);
            lua_map_pair_construct_006ee0d0(host, k.pair, key, v);
            host.lua_variant_destruct_00884af0(variant);
            release_key();
            lua_map_insert_unique_006ee4e0(host, map, &result, k.pair);
        }
        // Shift / Ctrl / Alt: inserted, then the pair, the variant and the key destroyed.
        const struct {
            const char* name;
            std::uint8_t flag;
        } flags[] = {{"Shift", shift}, {"Ctrl", ctrl}, {"Alt", alt}};  // 00CFACF4, 00CFACEC, 00CFACE8
        for (const auto& f : flags) {
            host.lua_key_from_text_00884d90(key, f.name);
            Object v = host.lua_variant_from_dword_00884a00(variant, f.flag != 0 ? 1u : 0u);
            Object p = lua_map_pair_construct_006ee0d0(host, pair, key, v);
            lua_map_insert_unique_006ee4e0(host, map, &result, p);
            lua_map_pair_destruct_006ee040(host, pair);
            host.lua_variant_destruct_00884af0(variant);
            release_key();
        }
        alignas(4) std::uint8_t table[0x14] = {};
        host.lua_variant_from_map_00886840(table, map);
        lua_variant_vector_push_back_006edf00(host, arguments, table);
        host.lua_variant_destruct_00884af0(table);
        host.mission_message_009291d0(userdata, arguments);
        lua_map_pair_destruct_006ee040(host, kept_message);
        lua_map_pair_destruct_006ee040(host, kept_f);
        TreeIterator erased{};
        Object end = obj(at<std::uint32_t>(map, 0x4));
        host.lua_map_erase_range_006ee960(map, &erased, TreeIterator{map, obj(at<std::uint32_t>(end, 0x0))},
                                          TreeIterator{map, end});
        host.free_00bf65ac(obj(at<std::uint32_t>(map, 0x4)));
        at<std::uint32_t>(map, 0x4) = 0;
        at<std::uint32_t>(map, 0x8) = 0;
        host.lua_variant_vector_resize_006b88e0(arguments, 0);
        host.free_00bf6989(obj(at<std::uint32_t>(arguments, 0x0)));
        host.lua_object_destruct_00b67700(ptr);
        host.lua_object_destruct_00b67700(dead);
    }
    host.lua_object_destruct_00b67700(mission);
    host.lua_object_destruct_00b67700(globals);
}

// ===========================================================================================
// 006EDB40: debug side swap (reached from the game-state 0Dh input handler, 006EF12C)
// ===========================================================================================
// Toggle the local player record's +28h (0 <-> 1), refresh its recon slot (+30h = 008053C0
// (side)), rebuild the slot lists (008073C0), then for every unit of the list at
// [[game+19CCh]+34h] (next +4h, unit +8h) with +5Ch set and +5Dh, +60h, +5Eh clear, call slot 0
// of unit+1E4h with (side, v, v), v = the side's 34h-byte record at unit+1E8h: its +8h when
// the flag at +10h is set, else +4h. Both arguments read the same flag byte (unit+1F8h is
// record+10h). Then clear game+193Ch, rebuild the local player's unit lists (004C3CB0) and
// tail-jump to 006485A0 with ECX = [[00E198C4]+40h].
void debug_swap_local_side_006edb40(Seg33bHost& host) {
    Object game = obj(host.global_u32(kGameGlobals));
    Object record = obj(at<std::uint32_t>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4));
    const std::uint32_t side = at<std::int32_t>(record, 0x28) == 0 ? 1u : 0u;
    at<std::uint32_t>(record, 0x28) = side;
    game = obj(host.global_u32(kGameGlobals));
    record = obj(at<std::uint32_t>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4));
    at<std::uint32_t>(record, 0x30) = host.recon_ensure_slot_008053c0(side);
    game = obj(host.global_u32(kGameGlobals));
    record = obj(at<std::uint32_t>(game, 0x18CC + at<std::int32_t>(game, 0x18EC) * 4));
    host.recon_rebuild_slot_lists_008073c0(at<std::uint32_t>(record, 0x30));
    game = obj(host.global_u32(kGameGlobals));
    for (Object link = obj(at<std::uint32_t>(obj(at<std::uint32_t>(game, 0x19CC)), 0x34)); link != nullptr;
         link = obj(at<std::uint32_t>(link, 0x4))) {
        Object unit = obj(at<std::uint32_t>(link, 0x8));
        if (at<std::uint8_t>(unit, 0x5C) == 0 || at<std::uint8_t>(unit, 0x5D) != 0 ||
            at<std::uint8_t>(unit, 0x60) != 0 || at<std::uint8_t>(unit, 0x5E) != 0) {
            continue;
        }
        Object side_record = unit + side * 0x34 + 0x1E8;
        const std::uint32_t first = at<std::uint8_t>(side_record, 0x10) != 0 ? at<std::uint32_t>(side_record, 0x8)
                                                                             : at<std::uint32_t>(side_record, 0x4);
        const std::uint32_t second = at<std::uint8_t>(side_record, 0x10) != 0 ? at<std::uint32_t>(side_record, 0x8)
                                                                              : at<std::uint32_t>(side_record, 0x4);
        host.unit_sub1e4_slot0(unit + 0x1E4, side, first, second);
    }
    game = obj(host.global_u32(kGameGlobals));
    at<std::uint8_t>(game, 0x193C) = 0;
    host.game_build_local_player_unit_lists_004c3cb0(game);
    host.hud_force_select_unit_006485a0(obj(at<std::uint32_t>(obj(host.global_u32(kHudRoot)), 0x40)));
}

// ===========================================================================================
// Fog-cloud model pool
// ===========================================================================================

namespace {
Object fog_block(Object pool, std::uint32_t index) {
    return obj(at<std::uint32_t>(obj(at<std::uint32_t>(pool, 0x28)), index * 4));
}
std::uint16_t& fog_free_count(Object block) { return at<std::uint16_t>(block, kFogFreeCount); }

// The reallocation the pool inlines at 006F0782 and 006F09BF: new[] (00BF55BE) the new
// capacity, copy count dwords (placement-null test per element), free the old array (00BF6989).
void fog_reallocate_blocks(Seg33bHost& host, Object pool, std::uint32_t capacity) {
    Object fresh = host.operator_new_00bf55be(capacity * 4);
    for (std::uint32_t i = 0; i < at<std::uint32_t>(pool, 0x2C); ++i) {
        if (fresh + i * 4 != nullptr) {
            at<std::uint32_t>(fresh, i * 4) = at<std::uint32_t>(obj(at<std::uint32_t>(pool, 0x28)), i * 4);
        }
    }
    if (obj(at<std::uint32_t>(pool, 0x28)) != nullptr) {
        host.free_00bf6989(obj(at<std::uint32_t>(pool, 0x28)));
    }
    at<std::uint32_t>(pool, 0x28) = addr(fresh);
}
}  // namespace

// 006EF2A0: forwards to 00B748E0(a1, a2, (float)a4, 0). The fourth argument is converted as
// unsigned (FILD then +2^32 (00CE3978) when negative); the third is not read.
void fog_cloud_render_006ef2a0(Seg33bHost& host, Object model, std::uint32_t a1, float a2, std::uint32_t a3,
                               std::uint32_t a4) {
    (void)a3;
    float visibility = static_cast<float>(static_cast<std::int32_t>(a4));
    if (static_cast<std::int32_t>(a4) < 0) {
        visibility = static_cast<float>(static_cast<double>(static_cast<std::int32_t>(a4)) + kTwoTo32);
    }
    host.generated_model_render_00b748e0(model, a1, a2, visibility, 0);
}

// 006EF3F0: free count 2, free stack {1, 0}, each slot's index dword = the pool index.
Object fog_pool_init_block_006ef3f0(Object block, std::uint32_t pool_index) {
    at<std::uint16_t>(block, kFogFreeCount) = kFogSlotsPerBlock;
    for (std::uint32_t i = 0; i < 2; ++i) {
        at<std::uint16_t>(block, kFogFreeStack + i * 2) = static_cast<std::uint16_t>(1 - i);
        at<std::uint32_t>(block, kFogSlotIndex + i * kFogSlotBytes) = pool_index;
    }
    return block;
}

// 006F03A0: under the section: push the slot's index in its block ((slot - block) / 194h,
// signed) on the block's free stack, ++count, lower first-free (unsigned compare).
void fog_pool_free_slot_006f03a0(Seg33bHost& host, Object pool, Object slot) {
    host.enter_critical_section(pool + 0xC);
    at<std::int32_t>(pool, 0x24) += 1;
    const std::uint32_t index = at<std::uint32_t>(slot, kFogSlotIndex);
    Object block = fog_block(pool, index);
    const std::int32_t offset = static_cast<std::int32_t>(slot - block);
    at<std::uint16_t>(block, kFogFreeStack + fog_free_count(block) * 2) =
        static_cast<std::uint16_t>(offset / static_cast<std::int32_t>(kFogSlotBytes));
    fog_free_count(block) += 1;
    if (index < at<std::uint32_t>(pool, 0x34)) {
        at<std::uint32_t>(pool, 0x34) = index;
    }
    at<std::int32_t>(pool, 0x24) -= 1;
    host.leave_critical_section(pool + 0xC);
}

// 006F06F0: the slot back to the static pool 00E199B0.
void fog_cloud_operator_delete_006f06f0(Seg33bHost& host, Object slot) {
    fog_pool_free_slot_006f03a0(host, obj(kFogCloudPool), slot);
}

// 006F0700: link at the head of the allocator list (00E188B4) under the allocator_list_elem
// vtable 00D7A0C0, then the pool vtable 00CFAD6C, InitializeCriticalSection(+0Ch), lock depth
// 0, empty block array, first-free -1, and reserve 20h block pointers (inline 006F0770..).
Object fog_pool_construct_006f0700(Seg33bHost& host, Object pool) {
    at<std::uint32_t>(pool, 0x0) = kAllocatorListElemVtable;
    at<std::uint32_t>(pool, 0x4) = 0;
    at<std::uint32_t>(pool, 0x8) = host.global_u32(kAllocatorListHead);
    if (host.global_u32(kAllocatorListHead) != 0) {
        at<std::uint32_t>(obj(host.global_u32(kAllocatorListHead)), 0x4) = addr(pool);
    }
    host.global_u32(kAllocatorListHead) = addr(pool);
    at<std::uint32_t>(pool, 0x0) = kFogCloudPoolVtable;
    host.initialize_critical_section(pool + 0xC);
    at<std::uint32_t>(pool, 0x24) = 0;
    at<std::uint32_t>(pool, 0x28) = 0;
    at<std::uint32_t>(pool, 0x2C) = 0;
    at<std::uint32_t>(pool, 0x30) = 0;
    at<std::uint32_t>(pool, 0x34) = 0xFFFFFFFFu;
    if (at<std::uint32_t>(pool, 0x30) < 0x20) {
        at<std::uint32_t>(pool, 0x30) = 0x20;
        fog_reallocate_blocks(host, pool, 0x20);
    }
    return pool;
}

// 006F0830: free every block (00BF65AC) and the array (00BF6989), leave the section once per
// remaining lock depth, delete it, restore the allocator_list_elem vtable and unlink.
void fog_pool_destruct_006f0830(Seg33bHost& host, Object pool) {
    at<std::uint32_t>(pool, 0x0) = kFogCloudPoolVtable;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(pool, 0x2C); ++i) {
        host.free_00bf65ac(fog_block(pool, i));
    }
    if (obj(at<std::uint32_t>(pool, 0x28)) != nullptr) {
        host.free_00bf6989(obj(at<std::uint32_t>(pool, 0x28)));
    }
    while (at<std::int32_t>(pool, 0x24) > 0) {
        at<std::int32_t>(pool, 0x24) -= 1;
        host.leave_critical_section(pool + 0xC);
    }
    host.delete_critical_section(pool + 0xC);
    at<std::uint32_t>(pool, 0x0) = kAllocatorListElemVtable;
    Object prev = obj(at<std::uint32_t>(pool, 0x4));
    if (prev != nullptr) {
        at<std::uint32_t>(prev, 0x8) = at<std::uint32_t>(pool, 0x8);
    } else {
        host.global_u32(kAllocatorListHead) = at<std::uint32_t>(pool, 0x8);
    }
    Object next = obj(at<std::uint32_t>(pool, 0x8));
    if (next != nullptr) {
        at<std::uint32_t>(next, 0x4) = at<std::uint32_t>(pool, 0x4);
    }
}

// 006F08C0: free every block whose free count is 2 (00BF65AC), moving the last block into the
// hole and renumbering its two slots; then first-free = the first block with a free slot, or
// -1. No lock is taken.
void fog_pool_trim_006f08c0(Seg33bHost& host, Object pool) {
    std::uint32_t i = 0;
    if (at<std::uint32_t>(pool, 0x2C) != 0) {
        do {
            Object block = fog_block(pool, i);
            if (fog_free_count(block) == 2) {
                host.free_00bf65ac(block);
                Object blocks = obj(at<std::uint32_t>(pool, 0x28));
                at<std::uint32_t>(blocks, i * 4) = at<std::uint32_t>(blocks, (at<std::uint32_t>(pool, 0x2C) - 1) * 4);
                at<std::uint32_t>(pool, 0x2C) -= 1;
                if (i < at<std::uint32_t>(pool, 0x2C)) {
                    Object moved = fog_block(pool, i);
                    for (std::uint32_t s = 0; s < 2; ++s) {
                        at<std::uint32_t>(moved, kFogSlotIndex + s * kFogSlotBytes) = i;
                    }
                }
                --i;  // unsigned wrap at 0 is undone by the increment
            }
            ++i;
        } while (i < at<std::uint32_t>(pool, 0x2C));
    }
    at<std::uint32_t>(pool, 0x34) = 0xFFFFFFFFu;
    for (std::uint32_t b = 0; b < at<std::uint32_t>(pool, 0x2C); ++b) {
        if (fog_free_count(fog_block(pool, b)) != 0) {
            at<std::uint32_t>(pool, 0x34) = b;
            return;
        }
    }
}

// 006F0960: under the section; with no block holding a free slot, first-free = count, a new
// 330h-byte block (00BF681B; a null result is stored as a null block) initialised by 006EF3F0,
// the array grown to 2 * capacity + 2 when full (inline, 00BF55BE / 00BF6989), the block
// appended (placement-null test). Pop the slot, and when the block is now full advance
// first-free past full blocks (-1 when none).
Object fog_pool_allocate_slot_006f0960(Seg33bHost& host, Object pool) {
    host.enter_critical_section(pool + 0xC);
    at<std::int32_t>(pool, 0x24) += 1;
    if (at<std::int32_t>(pool, 0x34) == -1) {
        at<std::uint32_t>(pool, 0x34) = at<std::uint32_t>(pool, 0x2C);
        Object memory = host.operator_new_00bf681b(kFogBlockBytes);
        Object block = memory == nullptr ? nullptr : fog_pool_init_block_006ef3f0(memory, at<std::uint32_t>(pool, 0x34));
        if (at<std::uint32_t>(pool, 0x2C) == at<std::uint32_t>(pool, 0x30)) {
            const std::uint32_t capacity = at<std::uint32_t>(pool, 0x30) * 2 + 2;
            at<std::uint32_t>(pool, 0x30) = capacity;
            fog_reallocate_blocks(host, pool, capacity);
        }
        Object cell = obj(at<std::uint32_t>(pool, 0x28)) + at<std::uint32_t>(pool, 0x2C) * 4;
        if (cell != nullptr) {
            at<std::uint32_t>(cell, 0) = addr(block);
        }
        at<std::uint32_t>(pool, 0x2C) += 1;
    }
    Object block = fog_block(pool, at<std::uint32_t>(pool, 0x34));
    fog_free_count(block) -= 1;
    Object slot = block + at<std::uint16_t>(block, kFogFreeStack + fog_free_count(block) * 2) * kFogSlotBytes;
    if (fog_free_count(block) == 0) {
        std::uint32_t next = at<std::uint32_t>(pool, 0x34) + 1;
        at<std::uint32_t>(pool, 0x34) = 0xFFFFFFFFu;
        for (; next < at<std::uint32_t>(pool, 0x2C); ++next) {
            if (fog_free_count(fog_block(pool, next)) != 0) {
                at<std::uint32_t>(pool, 0x34) = next;
                break;
            }
        }
    }
    at<std::int32_t>(pool, 0x24) -= 1;
    host.leave_critical_section(pool + 0xC);
    return slot;
}

// 006F0C30: 00B750C0 (model teardown), then with flag bit 0 the slot back to 00E199B0.
Object fog_cloud_deleting_destructor_006f0c30(Seg33bHost& host, Object model, std::uint8_t flags) {
    host.model_destroy_00b750c0(model);
    if ((flags & 1) != 0) {
        fog_pool_free_slot_006f03a0(host, obj(kFogCloudPool), model);
    }
    return model;
}

namespace {
// The pooled engine string the image builds for a literal: {0, 0}, 0041DD40(length, 1), then
// memcpy of length + 1 bytes when storage was allocated.
void literal_string(Seg33bHost& host, std::uint8_t* string, const char* text, std::uint32_t length) {
    at<std::uint32_t>(string, 0x0) = 0;
    at<std::uint32_t>(string, 0x4) = 0;
    host.native_string_resize_0041dd40(string, length, 1);
    void* data = obj(at<std::uint32_t>(string, 0x4));
    if (data != nullptr) host.memcpy_00bf7680(data, text, at<std::uint32_t>(string, 0x0) + 1);
}
void release_string(Seg33bHost& host, std::uint8_t* string) {
    void* data = obj(at<std::uint32_t>(string, 0x4));
    if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(string, 0x0) + 1);
}
}  // namespace

// 006F1440: a slot of the fog pool 00E199B0 (006F0960) built as a generated model named "Tej"
// (00B75030, vtable 00CFAD10; the name string is freed only when the slot was non-null); a
// mesh (00B73B60(BCh) / 00B73D70, null when the allocation fails) set as its geometry
// (00B75170(0, mesh, -1.0f, -1.0f), called even on a null model). The renderer [00F8D394]
// makes the vertex format "simplecolor.mvfm" (+38h), a 4-vertex buffer (+5Ch(4, 1, format)),
// after which the format's reference is dropped (InterlockedDecrement on +4h, vtable slot 0 at
// zero), and a 6-index buffer (+60h(6, 1, 65h)). Indices {0, 1, 2, 0, 2, 3}; vertices 24h
// apart {x, y, z, colour +18h, u +1Ch, v +20h}: (-10, -10), (10, -10), (10, 10), (-10, 10)
// at height z (00CE6848 = -10.0f, 00CE38B8 = 10.0f) with uv (0,0), (1,0), (1,1), (0,1). The
// effect "alphachannel.mshd" (+48h) makes a material (00B18780(110h) / 00B18900) whose slot 0
// takes the texture "cloud.dds" (+64h(name, 0); 00B189F0); a draw section (00B85EE0(60h) /
// 00B857F0) gets the material (00B864C0) and is appended (00B73C60); element 0 (00B732C0) gets
// {+8h 4, +0Ch 0, +14h 0, +10h 4, +18h 2}, the section owners are cleared (00B73C10) and the
// element appended; finally the model registers under [[00E188A8]+19ECh] (00B6D890).
// Unwinds are not modelled.
Object fog_cloud_create_006f1440(Seg33bHost& host, float z, std::uint32_t colour) {
    Object slot = fog_pool_allocate_slot_006f0960(host, obj(kFogCloudPool));
    Object model = nullptr;
    bool named = false;
    alignas(4) std::uint8_t name[8] = {};
    if (slot != nullptr) {
        host.native_string_assign_0041e870(name, "Tej");  // 00CFADB8
        named = true;
        host.generated_model_construct_00b75030(slot, name);
        at<std::uint32_t>(slot, 0x0) = kFogCloudModelVtable;
        model = slot;
    }
    if (named) release_string(host, name);
    Object memory = host.mesh_allocate_00b73b60(0xBC);
    Object mesh = memory != nullptr ? host.mesh_construct_00b73d70(memory) : nullptr;
    host.generated_model_set_geometry_00b75170(model, 0, mesh, -1.0f, -1.0f);  // 00D7A260

    Object renderer = obj(host.global_u32(kRenderer));
    alignas(4) std::uint8_t text[8] = {};
    literal_string(host, text, "simplecolor.mvfm", 0x10);  // 00CFADA4
    Object format = obj(host.render_virtual_1(renderer, 0x38, addr(text)));
    release_string(host, text);
    renderer = obj(host.global_u32(kRenderer));
    Object vertices = obj(host.render_virtual_3(renderer, 0x5C, 4, 1, addr(format)));
    if (host.interlocked_decrement(&at<std::int32_t>(format, 0x4)) == 0) {
        host.render_virtual_0(format, 0x0);
    }
    host.mesh_set_vertex_stream_00b73bb0(mesh, 0, vertices);
    renderer = obj(host.global_u32(kRenderer));
    Object indices = obj(host.render_virtual_3(renderer, 0x60, 6, 1, 0x65));
    host.mesh_set_index_stream_00b73b70(mesh, indices);
    Object index = obj(host.render_virtual_3(indices, 0xC, 0, 0, 0));
    const std::uint16_t quad[6] = {0, 1, 2, 0, 2, 3};
    for (int i = 0; i < 6; ++i) at<std::uint16_t>(index, i * 2) = quad[i];
    host.render_virtual_0(indices, 0x10);
    Object vertex = obj(host.render_virtual_3(vertices, 0x10, 0, 0, 0));
    const float corner[4][4] = {{-10.0f, -10.0f, 0.0f, 0.0f}, {10.0f, -10.0f, 1.0f, 0.0f},
                                {10.0f, 10.0f, 1.0f, 1.0f}, {-10.0f, 10.0f, 0.0f, 1.0f}};
    for (int i = 0; i < 4; ++i) {
        Object v = vertex + i * 0x24;
        at<float>(v, 0x0) = corner[i][0];
        at<float>(v, 0x4) = corner[i][1];
        at<float>(v, 0x8) = z;
        at<std::uint32_t>(v, 0x18) = colour;
        at<float>(v, 0x1C) = corner[i][2];
        at<float>(v, 0x20) = corner[i][3];
    }
    host.render_virtual_0(vertices, 0x14);

    literal_string(host, text, "alphachannel.mshd", 0x11);  // 00CFAD90
    renderer = obj(host.global_u32(kRenderer));
    Object effect = obj(host.render_virtual_1(renderer, 0x48, addr(text)));
    release_string(host, text);
    memory = host.material_allocate_00b18780(0x110);
    Object material = memory != nullptr ? host.material_construct_00b18900(memory, effect) : nullptr;
    literal_string(host, text, "cloud.dds", 9);  // 00CFAD84
    renderer = obj(host.global_u32(kRenderer));
    Object texture = obj(host.render_virtual_2(renderer, 0x64, addr(text), 0));
    release_string(host, text);
    host.material_set_texture_00b189f0(material, 0, texture);
    memory = host.draw_section_allocate_00b85ee0(0x60);
    Object section = memory != nullptr ? host.draw_section_construct_00b857f0(memory) : nullptr;
    host.draw_section_set_material_00b864c0(section, material);
    host.mesh_append_section_00b73c60(mesh, section);
    Object element = host.mesh_element_00b732c0(mesh, 0);
    at<std::uint32_t>(element, 0x8) = 4;
    at<std::uint32_t>(element, 0xC) = 0;
    at<std::uint32_t>(element, 0x14) = 0;
    at<std::uint32_t>(element, 0x10) = 4;
    at<std::uint32_t>(element, 0x18) = 2;
    host.mesh_clear_section_owners_00b73c10(mesh);
    host.mesh_append_section_00b73c60(mesh, element);
    host.node_propagate_root_00b6d890(model, at<std::uint32_t>(obj(host.global_u32(kGameGlobals)), 0x19EC));
    return model;
}

// 006F13E0: unless self-assignment, resize the string (0041DD40(length, 1)) and memcpy the
// new length's bytes when the source length is non-zero; then copy the six fields (floats
// through x87 FLD/FSTP, dwords by MOV) even on self-assignment.
Object fog_cloud_record_assign_006f13e0(Seg33bHost& host, Object to, Object from) {
    if (to != from) {
        host.native_string_resize_0041dd40(to, at<std::uint32_t>(from, 0x0), 1);
        if (at<std::uint32_t>(from, 0x0) != 0) {
            host.memcpy_00bf7680(obj(at<std::uint32_t>(to, 0x4)), obj(at<std::uint32_t>(from, 0x4)),
                                 at<std::uint32_t>(to, 0x0));
        }
    }
    at<float>(to, 0x8) = at<float>(from, 0x8);
    at<float>(to, 0xC) = at<float>(from, 0xC);
    at<std::uint32_t>(to, 0x10) = at<std::uint32_t>(from, 0x10);
    at<float>(to, 0x14) = at<float>(from, 0x14);
    at<float>(to, 0x18) = at<float>(from, 0x18);
    at<std::uint32_t>(to, 0x1C) = at<std::uint32_t>(from, 0x1C);
    return to;
}

// ===========================================================================================
// Command list, target descriptor, command stream, convex object
// ===========================================================================================

// 006F7D10: operator new(0Ch) (00BF681B) -> {0, 0, value}. On a null result the image still
// stores the value at address 8h (006F7D35), reproduced through the null pointer.
Object command_list_new_node_006f7d10(Seg33bHost& host, std::uint32_t value) {
    Object node = host.operator_new_00bf681b(0xC);
    if (node != nullptr) {
        at<std::uint32_t>(node, 0x0) = 0;
        at<std::uint32_t>(node, 0x4) = 0;
        at<std::uint32_t>(node, 0x8) = value;
        return node;
    }
    at<std::uint32_t>(static_cast<Object>(nullptr), 0x8) = value;
    return nullptr;
}

// 006F7D40: a zeroed node (null when new fails; the stores go through it regardless), +8h =
// value, +0h = old tail; non-empty: old tail +4h = node; empty: head = node; tail = node,
// node +4h = 0, ++count. List {count +0h, head +4h, tail +8h}.
void command_list_append_006f7d40(Seg33bHost& host, Object list, std::uint32_t value) {
    Object node = host.operator_new_00bf681b(0xC);
    if (node != nullptr) {
        at<std::uint32_t>(node, 0x0) = 0;
        at<std::uint32_t>(node, 0x4) = 0;
        at<std::uint32_t>(node, 0x8) = 0;
    }
    at<std::uint32_t>(node, 0x8) = value;
    at<std::uint32_t>(node, 0x0) = at<std::uint32_t>(list, 0x8);
    if (at<std::uint32_t>(list, 0x0) != 0) {
        at<std::uint32_t>(obj(at<std::uint32_t>(list, 0x8)), 0x4) = addr(node);
    } else {
        at<std::uint32_t>(list, 0x4) = addr(node);
    }
    at<std::uint32_t>(list, 0x8) = addr(node);
    at<std::uint32_t>(node, 0x4) = 0;
    at<std::uint32_t>(list, 0x0) += 1;
}

// 006F7DD0: with an entity (+0h): refresh its pose when +C8h is clear (00414DB0); with a point
// (+1h) the point transformed by the entity's world matrix (+CCh, 004142E0), else the
// entity's world position (+FCh). Without an entity: the point, or the zero vector 00F87574.
void target_descriptor_resolve_position_006f7dd0(Seg33bHost& host, Object descriptor, float* out) {
    if (at<std::uint8_t>(descriptor, 0x0) == 0) {
        if (at<std::uint8_t>(descriptor, 0x1) != 0) {
            out[0] = at<float>(descriptor, 0x8);
            out[1] = at<float>(descriptor, 0xC);
            out[2] = at<float>(descriptor, 0x10);
        } else {
            out[0] = *reinterpret_cast<const float*>(&host.global_u32(kZeroVector));
            out[1] = *reinterpret_cast<const float*>(&host.global_u32(kZeroVector + 4));
            out[2] = *reinterpret_cast<const float*>(&host.global_u32(kZeroVector + 8));
        }
        return;
    }
    Object entity = obj(at<std::uint32_t>(descriptor, 0x4));
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_00414db0(entity);
    if (at<std::uint8_t>(descriptor, 0x1) != 0) {
        float local[3];
        const float* p = host.transform_affine_point_004142e0(&at<float>(descriptor, 0x8), local,
                                                              &at<float>(entity, 0xCC));
        out[0] = p[0];
        out[1] = p[1];
        out[2] = p[2];
        return;
    }
    out[0] = at<float>(entity, 0xFC);
    out[1] = at<float>(entity, 0x100);
    out[2] = at<float>(entity, 0x104);
}

// 006F99C0: read the "name" (00CE49B8) field into an engine string through the stream's
// vtable +10h ({0, "name"}, {0, &string}), find the command class by that name (00467170;
// "" 00E19A78 when the string has no storage), free the string, return the class.
Object command_stream_read_class_006f99c0(Seg33bHost& host, Object stream) {
    alignas(4) std::uint8_t name[8] = {};  // {length, data}
    host.stream_read_field_10(stream, TaggedValue{0, addr("name")}, TaggedValue{0, addr(name)});
    const char* text = reinterpret_cast<const char*>(obj(at<std::uint32_t>(name, 0x4)));
    if (text == nullptr) text = reinterpret_cast<const char*>(obj(kEmptyCString));
    Object klass = host.command_registry_find_00467170(text);
    void* data = obj(at<std::uint32_t>(name, 0x4));
    if (data != nullptr) host.pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(name, 0x0) + 1);
    return klass;
}

// 006F9AC0: read the count (00BE9A80 into *count), new[] count * 2 (00BF55BE, saturated),
// store it at *out_data, then read count values (each into a dword) keeping the low word.
void convex_read_u16_array_006f9ac0(Seg33bHost& host, Object node, Object out_data, std::uint32_t* count) {
    host.node_read_u32_00be9a80(node, count);
    const std::uint64_t bytes = static_cast<std::uint64_t>(*count) * 2u;
    Object data = host.operator_new_00bf55be(bytes > 0xFFFFFFFFull ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes));
    at<std::uint32_t>(out_data, 0x0) = addr(data);
    for (std::int32_t i = 0; i < static_cast<std::int32_t>(*count); ++i) {
        std::uint32_t value = 0;
        host.node_read_u32_00be9a80(node, &value);
        at<std::uint16_t>(obj(at<std::uint32_t>(out_data, 0x0)), i * 2) = static_cast<std::uint16_t>(value);
    }
}

// 006F9B20: five u32 reads; the low words of the first four go to out[0..3], the fifth value
// is read and dropped.
Object convex_read_record_006f9b20(Seg33bHost& host, Object node, std::uint16_t* out) {
    std::uint32_t value = 0;
    for (int i = 0; i < 4; ++i) {
        host.node_read_u32_00be9a80(node, &value);
        out[i] = static_cast<std::uint16_t>(value);
    }
    host.node_read_u32_00be9a80(node, &value);
    return node;
}

// 006F9BF0: x, y, z through 00BE9A60, each call on the node the previous one returned.
Object node_read_vector3_006f9bf0(Seg33bHost& host, Object node, float* out) {
    Object next = host.node_read_float_00be9a60(node, &out[0]);
    next = host.node_read_float_00be9a60(next, &out[1]);
    host.node_read_float_00be9a60(next, &out[2]);
    return node;
}

// 006F9C20 / 006F9C40: vtable 00CFB69C, three buffers at +10h/+14h/+18h (freed when set).
Object convex_arrays_construct_006f9c20(Object holder) {
    at<std::uint32_t>(holder, 0x0) = kConvexArraysVtable;
    at<std::uint32_t>(holder, 0x10) = 0;
    at<std::uint32_t>(holder, 0x14) = 0;
    at<std::uint32_t>(holder, 0x18) = 0;
    return holder;
}
void convex_arrays_destruct_006f9c40(Seg33bHost& host, Object holder) {
    at<std::uint32_t>(holder, 0x0) = kConvexArraysVtable;
    for (std::size_t off : {0x10u, 0x14u, 0x18u}) {
        void* block = obj(at<std::uint32_t>(holder, off));
        if (block != nullptr) host.free_00bf6989(block);
    }
}

// 006F9C80 / 006F9CA0: vtable 00CFB6A0, two buffers at +14h/+18h.
Object convex_arrays2_construct_006f9c80(Object holder) {
    at<std::uint32_t>(holder, 0x0) = kConvexArrays2Vtable;
    at<std::uint32_t>(holder, 0x14) = 0;
    at<std::uint32_t>(holder, 0x18) = 0;
    return holder;
}
void convex_arrays2_destruct_006f9ca0(Seg33bHost& host, Object holder) {
    at<std::uint32_t>(holder, 0x0) = kConvexArrays2Vtable;
    for (std::size_t off : {0x14u, 0x18u}) {
        void* block = obj(at<std::uint32_t>(holder, off));
        if (block != nullptr) host.free_00bf6989(block);
    }
}

// 006F9CD0: the resource-item base (00B868B0), vtable 00CFB6A4, the hull at +0Ch (00C32D50).
Object convex_object_construct_006f9cd0(Seg33bHost& host, Object object) {
    host.resource_item_construct_00b868b0(object);
    at<std::uint32_t>(object, 0x0) = kConvexObjectVtable;
    host.convex_hull_construct_00c32d50(object + 0xC);
    return object;
}

// 006F9D70: vtable 00CFB6A4, the hull at +0Ch (00C37450), the base (00B86890).
void convex_object_destruct_006f9d70(Seg33bHost& host, Object object) {
    at<std::uint32_t>(object, 0x0) = kConvexObjectVtable;
    host.convex_hull_destroy_00c37450(object + 0xC);
    host.resource_item_destroy_00b86890(object);
}

// 006F9D20 / 006F9D30.
std::uint32_t return_zero_006f9d20(std::uint32_t a, std::uint32_t b) {
    (void)a;
    (void)b;
    return 0;
}
std::uint32_t return_zero_006f9d30() { return 0; }

// 006F9D40.
bool convex_object_is_kind_of_006f9d40(Seg33bHost& host, std::uint32_t token) {
    for (std::uint32_t cell = kConvexTokens; cell != kConvexTokensEnd; cell += 4) {
        if (host.global_u32(cell) == token) return true;
    }
    return false;
}

// 006FA320: the hull's vertices (00C356D0, 0Ch each) and its neighbour list (00C35630, uint16:
// per vertex a count then that many vertex indices, read through a running 16-bit cursor).
// For vertex i transformed by the matrix (004142E0), each neighbour j >= i (unsigned) is
// transformed and drawn as a line in white {1, 1, 1, 1} (00D7A24C) through 0085FB30. Both
// buffers are freed after (00BF6989). The unwind that frees them on a throw is not modelled.
void convex_hull_debug_draw_006fa320(Seg33bHost& host, Object hull, const float* matrix) {
    alignas(4) std::uint8_t vertices[0xC] = {};   // {data, size, capacity}
    alignas(4) std::uint8_t neighbours[0xC] = {};
    host.convex_hull_vertices_00c356d0(hull, vertices);
    host.convex_hull_neighbours_00c35630(hull, neighbours);
    std::uint32_t cursor = 0;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(vertices, 0x4); ++i) {
        const float* data = reinterpret_cast<const float*>(obj(at<std::uint32_t>(vertices, 0x0)));
        const std::uint16_t* links = reinterpret_cast<const std::uint16_t*>(obj(at<std::uint32_t>(neighbours, 0x0)));
        std::uint32_t count = links[cursor & 0xFFFF];
        const float point[3] = {data[i * 3], data[i * 3 + 1], data[i * 3 + 2]};
        ++cursor;
        float scratch[3];
        const float* t = host.transform_affine_point_004142e0(point, scratch, matrix);
        const float from[3] = {t[0], t[1], t[2]};
        if (static_cast<std::int32_t>(count) > 0) {
            do {
                const std::uint32_t j =
                    reinterpret_cast<const std::uint16_t*>(obj(at<std::uint32_t>(neighbours, 0x0)))[cursor & 0xFFFF];
                if (j >= i) {
                    const float* d = reinterpret_cast<const float*>(obj(at<std::uint32_t>(vertices, 0x0)));
                    const float other[3] = {d[j * 3], d[j * 3 + 1], d[j * 3 + 2]};
                    float scratch2[3];
                    const float* u = host.transform_affine_point_004142e0(other, scratch2, matrix);
                    const float to[3] = {u[0], u[1], u[2]};
                    const float colour[4] = {kOne, kOne, kOne, kOne};
                    host.debug_line_0085fb30(from, to, colour);
                }
                ++cursor;
            } while (--count != 0);
        }
    }
    void* links = obj(at<std::uint32_t>(neighbours, 0x0));
    if (links != nullptr) host.free_00bf6989(links);
    void* data = obj(at<std::uint32_t>(vertices, 0x0));
    if (data != nullptr) host.free_00bf6989(data);
}

// 006F0800, ECX fog-pool chunk, RET; EAX is the zero-extended equality.
// The allocator's kFogFreeCount is32Ch, kFogSlotsPerBlock is2. New source
// interface, build-only; this is the pool predicate also stored at00D64EAC/00D652AC.
bool fog_chunk_all_slots_free_006f0800(Object chunk) {
    return at<std::uint16_t>(chunk, kFogFreeCount) == kFogSlotsPerBlock;
}

}  // namespace bsp::cc13_seg33b
