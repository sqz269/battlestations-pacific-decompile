// Packet cc13-left8: segment 8 leftovers. See include/bsp/cc13_left8_seg8.hpp and
// docs/CC13_LEFT8.md. Each body carries its address, the original ABI and every
// simplification next to it.

#include "bsp/cc13_left8_seg8.hpp"

#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace bsp::cc13_left8 {
namespace {

constexpr std::uint32_t kGame = 0x00e188a8;
constexpr std::uint32_t kGameTime = 0x00f876a4;  // float, read by MOVSS
constexpr float kOne = 1.0f;                    // 00D7A24C
constexpr double kIdleActorRange = 300.0;       // 00CE3CA8 (qword)
constexpr std::uint32_t kNegativeHuge = 0x00ce4adc;  // float -1.0e10 (D01502F9)
constexpr std::uint32_t kListNodeVtable = 0x00ce6600;

Object game(Left8Host& host) { return obj(host.global_u32(kGame)); }

// A std::vector<T> {proxy +0, first +4, last +8, end +0Ch} at `vector`: the
// range test the image makes before an indexed access; `stride_shift` is the
// SAR of the byte difference. A failure calls 00BF6713 (no return in the image).
void check_index(Left8Host& host, Object vector, std::uint32_t index, unsigned stride_shift) {
    const std::uint32_t first = at<std::uint32_t>(vector, 0x4);
    if (first == 0 ||
        index >= (static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(vector, 0x8) - first) >>
                                              stride_shift))) {
        host.invalid_parameter_00bf6713();
    }
}

// The checks the image makes before using a vector<bool> reference returned
// by 0048F930: container and word set, first <= last, offset < bit count.
void check_bit_ref(Left8Host& host, const BitRef& ref) {
    if (ref.container == nullptr || ref.word == nullptr) host.invalid_parameter_00bf6713();
    const std::uint32_t first = at<std::uint32_t>(ref.container, 0x8);
    if (at<std::uint32_t>(ref.container, 0xc) < first) host.invalid_parameter_00bf6713();
    const std::uint32_t offset =
        static_cast<std::uint32_t>((static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(ref.word) - first) >> 2) *
                                   0x20) +
        ref.bit;
    if (at<std::uint32_t>(ref.container, 0x0) <= offset) host.invalid_parameter_00bf6713();
}

// begin() + distance on a vector<bool>, as the image builds it: the first <=
// last test, the container tests, then 0048D3B0, then the dereference checks.
BitRef bit_at_through_iterator(Left8Host& host, Object bits, std::int32_t distance) {
    const std::uint32_t first = at<std::uint32_t>(bits, 0x8);
    if (at<std::uint32_t>(bits, 0xc) < first) host.invalid_parameter_00bf6713();
    if (bits == nullptr) host.invalid_parameter_00bf6713();
    BitRef it{bits, reinterpret_cast<std::uint32_t*>(static_cast<std::uintptr_t>(first)), 0};
    if (bits == nullptr) host.invalid_parameter_00bf6713();
    host.bit_iterator_advance_0048d3b0(&it, distance);
    if (it.container == nullptr) {
        host.invalid_parameter_00bf6713();
    } else if (it.word == nullptr) {
        host.invalid_parameter_00bf6713();
    }
    const std::uint32_t first2 = at<std::uint32_t>(it.container, 0x8);
    if (at<std::uint32_t>(it.container, 0xc) < first2) host.invalid_parameter_00bf6713();
    const std::uint32_t offset =
        static_cast<std::uint32_t>((static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(it.word) - first2) >> 2) *
                                   0x20) +
        it.bit;
    if (at<std::uint32_t>(it.container, 0x0) <= offset) host.invalid_parameter_00bf6713();
    return it;
}

}  // namespace

// 00491860 __thiscall(group, member), RET 4 (vtable 00CE69D4 slot 2). When the
// member is set: group+18h = the game time float [00F876A4] and member vtable
// +114h(0). Then the first i < [+24h] whose slot in the +74h vector equals the
// member (each access checked) is cleared and bit i of the vector<bool> at
// +80h is set (0048F930 plus the reference checks).
void idle_group_release_member_00491860(Left8Host& host, Object group, Object member) {
    if (member != nullptr) {
        at<float>(group, 0x18) = host.global_f32(kGameTime);
        host.vcall_set_group_114(member, nullptr);
    }
    for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < at<std::int32_t>(group, 0x24); ++i) {
        check_index(host, group + 0x70, i, 2);
        if (at<Object*>(group, 0x74)[i] != member) continue;
        check_index(host, group + 0x70, i, 2);
        at<Object*>(group, 0x74)[i] = nullptr;
        BitRef scratch{};
        BitRef* ref = host.bit_vector_at_0048f930(group + 0x80, &scratch, i);
        check_bit_ref(host, *ref);
        *ref->word |= 1u << (ref->bit & 0x1f);
        return;
    }
}

// 00491950 __thiscall(group, index), RET 4. Sole caller 00494F30. The bit
// `alternate` is element index of the vector<bool> at +28h (0048F930, then
// the checked read 0048F230). When the +74h slot is empty, the class pointer is
// element index of +54h (alternate) or +44h, its vtable +8 creates the
// instance, stored in the slot; then instance vtable +98h(owner [+20h], owner
// node [owner+30h], &pose[index]) with pose the 40h-stride vector at
// [owner+538h]+6ECh (checked); +114h(group); +A0h(); when alternate,
// 004B0F80(instance, &anim[index], 1, 0) with anim the 8-byte vector at +60h
// (checked); +11Ch(). Finally +D8h() on the slot's instance in every case.
void ship_idle_actor_instantiate_00491950(Left8Host& host, Object group, std::uint32_t index) {
    check_index(host, group + 0x70, index, 2);
    Object* slot = at<Object*>(group, 0x74) + index;
    BitRef scratch{};
    const bool alternate = host.bit_ref_get_0048f230(host.bit_vector_at_0048f930(group + 0x28, &scratch, index));
    if (*slot == nullptr) {
        Object classes = group + (alternate ? 0x50 : 0x40);
        check_index(host, classes, index, 2);
        *slot = host.vcall_create_instance_08(at<Object*>(classes, 0x4)[index]);
        Object owner = at<Object>(group, 0x20);
        Object poses = at<Object>(owner, 0x538) + 0x6ec;
        check_index(host, poses, index, 6);
        host.vcall_attach_98(*slot, owner, at<Object>(owner, 0x30), at<Object>(poses, 0x4) + index * 0x40);
        host.vcall_set_group_114(*slot, group);
        host.vcall_a0(*slot);
        if (alternate) {
            check_index(host, group + 0x60, index, 3);
            soldier_start_animation_004b0f80(host, *slot, at<Object>(group, 0x64) + index * 8, true, false);
        }
        host.vcall_11c(*slot);
    }
    host.vcall_d8(*slot);
}

// 00491AC0 __fastcall(entity), RET (vtable 00CE67AC slot 1; no direct caller).
// Four layers L: box +6Ch+L*18h, member count [+17Ch+L*4], visible byte
// +20Ch+L, cell boxes (18h each) in the vector at +1CCh+L*10h, cell bits in
// the vector<bool> at +210h+L*14h; [+38h] cells per layer.
//   Layer box out of view (004B48E0 on [00E188A8]): when the visible byte is
//   set, 00481430(entity, L, i) for every member (count re-read each time),
//   then the byte is cleared.
//   In view: for each cell c (box checked): out of view and bit c set ->
//   00481430 for members [count*c/cells, (c+1)*count/cells - 1] (signed IDIV)
//   and bit c cleared; in view -> for the same member range, member =
//   0048C6A0(entity, L, i), member+A8h = water height 0078CF20([game+19F0],
//   member+A4h, member+ACh), member vtable +D8h(); then bit c set. Afterwards
//   the visible byte = 1. Each bit access goes through 0048D3B0 as begin()+c.
void layered_water_entity_update_00491ac0(Left8Host& host, Object entity) {
    for (std::int32_t layer = 0; layer <= 3; ++layer) {
        Object box = entity + 0x6c + layer * 0x18;
        std::int32_t* count = reinterpret_cast<std::int32_t*>(entity + 0x17c + layer * 4);
        std::uint8_t* visible = entity + 0x20c + layer;
        Object cells = entity + 0x1cc + layer * 0x10;
        Object bits = entity + 0x210 + layer * 0x14;
        if (!host.game_is_aabb_in_view_004b48e0(game(host), reinterpret_cast<const float*>(box))) {
            if (*visible != 0) {
                for (std::int32_t i = 0; i < *count; ++i) host.scene_entity_layer_reset_child_00481430(entity, layer, i);
                *visible = 0;
            }
            continue;
        }
        for (std::int32_t c = 0; c < at<std::int32_t>(entity, 0x38); ++c) {
            const std::uint32_t first = at<std::uint32_t>(cells, 0x4);
            if (first == 0 || static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(cells, 0x8) - first) /
                                                         0x18) <= static_cast<std::uint32_t>(c)) {
                host.invalid_parameter_00bf6713();
            }
            if (!host.game_is_aabb_in_view_004b48e0(
                    game(host), reinterpret_cast<const float*>(at<Object>(cells, 0x4) + c * 0x18))) {
                BitRef it = bit_at_through_iterator(host, bits, c);
                if ((*it.word & (1u << (it.bit & 0x1f))) != 0) {
                    const std::int32_t n = *count;
                    const std::int32_t per = at<std::int32_t>(entity, 0x38);
                    for (std::int32_t i = n * c / per; i <= (c + 1) * n / per - 1; ++i) {
                        host.scene_entity_layer_reset_child_00481430(entity, layer, i);
                    }
                    BitRef clear = bit_at_through_iterator(host, bits, c);
                    *clear.word &= ~(1u << (clear.bit & 0x1f));
                }
            } else {
                const std::int32_t n = *count;
                const std::int32_t per = at<std::int32_t>(entity, 0x38);
                for (std::int32_t i = n * c / per; i <= (c + 1) * n / per - 1; ++i) {
                    Object member = host.scene_entity_layer_member_0048c6a0(entity, layer, i);
                    at<float>(member, 0xa8) = host.game_world_sample_water_height_0078cf20(
                        at<Object>(game(host), 0x19f0), at<float>(member, 0xa4), at<float>(member, 0xac));
                    host.vcall_d8(member);
                }
                BitRef set = bit_at_through_iterator(host, bits, c);
                *set.word |= 1u << (set.bit & 0x1f);
            }
        }
        *visible = 1;
    }
}

// 00491E70 __thiscall(manager, ship key, bool destroy), RET 8. Callers 004A2FC0
// and 006AC4D0. 00488630 finds the key in the map at +28h (head [+2Ch]); the
// returned container must be +28h (checked). On a hit:
//   destroy: EnterCriticalSection([+4]), [cs+18h] += 1, node checks, the
//   value [node+10h] (when set) gets vtable +0(1), 0048F9A0 erases the node,
//   [cs+18h] -= 1, LeaveCriticalSection.
//   otherwise: node checks, then 0048C190 on the value (the troop's ship left).
void landing_traffic_remove_ship_00491e70(Left8Host& host, Object manager, std::uint32_t ship, bool destroy) {
    Object map = manager + 0x28;
    std::uint32_t iterator[2] = {0, 0};  // {container, node}
    host.tree_find_00488630(map, reinterpret_cast<Object>(iterator), &ship);
    Object container = obj(iterator[0]);
    Object head = at<Object>(map, 0x4);
    if (container == nullptr || container != map) host.invalid_parameter_00bf6713();
    Object node = obj(iterator[1]);
    if (node == head) return;
    if (destroy) {
        Object section = at<Object>(manager, 0x4);
        host.enter_critical_section(section);
        at<std::int32_t>(section, 0x18) += 1;
        if (container == nullptr) host.invalid_parameter_00bf6713();
        if (node == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
        if (Object value = at<Object>(node, 0x10); value != nullptr) host.vcall_deleting_dtor_00(value, 1);
        std::uint32_t scratch[2] = {0, 0};
        host.tree_erase_0048f9a0(map, reinterpret_cast<Object>(scratch), container, node);
        Object section2 = at<Object>(manager, 0x4);
        at<std::int32_t>(section2, 0x18) -= 1;
        host.leave_critical_section(section2);
        return;
    }
    if (container == nullptr) host.invalid_parameter_00bf6713();
    if (node == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
    host.landing_troop_on_ship_left_0048c190(at<Object>(node, 0x10));
}

// 00494F30 __fastcall(group), RET. Sole caller 00497B80. Sets +94h = 1; when
// the camera [[00E188A8]+1ED4h] has +C8h clear, 00414DB0 refreshes its world
// pose. For each i < [+24h] whose bit in the vector<bool> at +80h is clear
// (0048F930 plus reference checks): p = translation row (+30h) of pose i of the
// 40h-stride vector at [owner+538h]+6ECh (checked) with w = 1.0;
// 00B62D10(point, out, owner+74h) transforms it; the divide by w, the camera
// delta (camera+FCh..+104h) and the squared range (300.0 qword /
// [[game+19FCh]+1C4h])^2 are rounded to float at the image's stores. Range^2 >
// distance^2 -> 00491950(i); otherwise (also unordered) the +74h slot (checked)
// when set gets vtable +114h(0), 00926D90(2) and is cleared.
void ship_idle_actors_update_00494f30(Left8Host& host, Object group) {
    at<std::uint8_t>(group, 0x94) = 1;
    Object poses = at<Object>(at<Object>(group, 0x20), 0x538) + 0x6ec;
    Object camera = at<Object>(game(host), 0x1ed4);
    if (at<std::uint8_t>(camera, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(camera);
    for (std::uint32_t i = 0; static_cast<std::int32_t>(i) < at<std::int32_t>(group, 0x24); ++i) {
        BitRef scratch{};
        BitRef* ref = host.bit_vector_at_0048f930(group + 0x80, &scratch, i);
        check_bit_ref(host, *ref);
        if ((*ref->word & (1u << (ref->bit & 0x1f))) != 0) continue;
        Object owner = at<Object>(group, 0x20);
        check_index(host, poses, i, 6);
        const float* row = reinterpret_cast<const float*>(at<Object>(poses, 0x4) + i * 0x40 + 0x30);
        const float point[4] = {row[0], row[1], row[2], kOne};
        float out[4];
        const float* t = host.vector4_transform_00b62d10(out, point, reinterpret_cast<const float*>(owner + 0x74));
        const float p[4] = {t[0], t[1], t[2], t[3]};
        const float w = static_cast<float>(1.0 / static_cast<double>(p[3]));
        const float x = static_cast<float>(static_cast<double>(p[0]) * w);
        const float y = static_cast<float>(static_cast<double>(p[1]) * w);
        const float z = static_cast<float>(static_cast<double>(w) * p[2]);
        const float dx = static_cast<float>(static_cast<double>(x) - at<float>(camera, 0xfc));
        const float dy = static_cast<float>(static_cast<double>(y) - at<float>(camera, 0x100));
        const float dz = static_cast<float>(static_cast<double>(z) - at<float>(camera, 0x104));
        const float range =
            static_cast<float>(kIdleActorRange / static_cast<double>(at<float>(at<Object>(game(host), 0x19fc), 0x1c4)));
        const double ddx = dx, ddy = dy, ddz = dz;
        const float distance_sq = static_cast<float>(ddz * ddz + (ddy * ddy + ddx * ddx));
        const float range_sq = static_cast<float>(static_cast<double>(range) * range);
        if (range_sq > distance_sq) {
            ship_idle_actor_instantiate_00491950(host, group, i);
            continue;
        }
        check_index(host, group + 0x70, i, 2);
        Object* slot = at<Object*>(group, 0x74) + i;
        if (*slot != nullptr) {
            host.vcall_set_group_114(*slot, nullptr);
            host.mission_entity_kill_00926d90(*slot, 2);
            *slot = nullptr;
        }
    }
}

// 00497B80 __fastcall(group), RET (vtable 00CE69D4 slot 1): vtable +0Ch(); true
// -> 00494F30, false -> 00487940 (flush the members).
void ship_idle_actors_tick_00497b80(Left8Host& host, Object group) {
    if (host.vcall_group_is_visible_0c(group)) {
        ship_idle_actors_update_00494f30(host, group);
        return;
    }
    host.traffic_group_flush_pending_visibility_00487940(group);
}

// 00496920 __thiscall(node, parent), RET 4, EAX = node. Callers 00498D60,
// 00498E60, 0049C000, 004A0410, 004A3610, 004A41C0, 004A4B70. vtable 00CE6600;
// the child list at +0Ch gets its sentinel from 00485270 (+10h) and size +14h =
// 0; +8 = parent, +4 = 0. With a parent, the node is appended to the parent's
// list (+0Ch, head [+10h]): 00485880(list, head, head->prev, &node), 0048FD20
// (list, 1) grows the size, then the two links. +18h = [00CE4ADC] (-1.0e10f,
// bits D01502F9), +1Ch = 1.0f.
Object list_node_construct_00496920(Left8Host& host, Object node, Object parent) {
    at<std::uint32_t>(node, 0x0) = kListNodeVtable;
    at<Object>(node, 0x10) = host.list_allocate_sentinel_00485270(node + 0xc);
    at<std::uint32_t>(node, 0x14) = 0;
    at<Object>(node, 0x8) = parent;
    at<std::uint32_t>(node, 0x4) = 0;
    if (parent != nullptr) {
        Object head = at<Object>(parent, 0x10);
        Object value = node;
        Object entry = host.list_buy_node_00485880(parent + 0xc, head, at<Object>(head, 0x4), &value);
        host.list_incsize_0048fd20(parent + 0xc, 1);
        at<Object>(head, 0x4) = entry;
        at<Object>(at<Object>(entry, 0x4), 0x0) = entry;
    }
    at<float>(node, 0x18) = host.global_f32(kNegativeHuge);
    at<float>(node, 0x1c) = kOne;
    return node;
}

// 004969D0 __thiscall(group, member), RET 4: slot 2 of the vtables 00CE68D4,
// 00CE68E4, 00CE6A08 and 00CE6A68 (no direct caller). Member list A at +6Ch
// (head [+70h], size +74h) holds the live members in slot order; list B at
// +78h (head [+7Ch], size +80h) holds the free slot numbers in ascending order;
// [+88h] is the first slot number. When the member is set: +18h = game time,
// member vtable +114h(0). Then the front of B is popped (unlink, 00BF65AC,
// size - 1) while B is non-empty and its value < [+88h]. Walk: n = [+88h], b =
// first of B, a = first of A; until a is A's head: while b is not B's head and
// *b == n, b advances and n += 1; then when a's value is the member, n is
// inserted before b (004857F0(B, b, b->prev, &n), 0048FC80(B, 1), links), b
// advances when it is not the head, a is erased (unlink, 00BF65AC, A size - 1)
// and a = its successor; otherwise a advances; n += 1 in both cases. Every
// matching node is removed, not only the first. Iterator checks that compare a
// container with itself (CMP EBP,EBP) cannot fail and are not modelled.
void list_group_release_member_004969d0(Left8Host& host, Object group, Object member) {
    if (member != nullptr) {
        at<float>(group, 0x18) = host.global_f32(kGameTime);
        host.vcall_set_group_114(member, nullptr);
    }
    while (at<std::uint32_t>(group, 0x80) != 0) {
        Object head = at<Object>(group, 0x7c);
        Object front = at<Object>(head, 0x0);
        if (front == head) host.invalid_parameter_00bf6713();
        if (at<std::int32_t>(front, 0x8) >= at<std::int32_t>(group, 0x88)) break;
        head = at<Object>(group, 0x7c);
        front = at<Object>(head, 0x0);
        if (front == head) host.invalid_parameter_00bf6713();
        if (front != at<Object>(group, 0x7c)) {
            at<Object>(at<Object>(front, 0x4), 0x0) = at<Object>(front, 0x0);
            at<Object>(at<Object>(front, 0x0), 0x4) = at<Object>(front, 0x4);
            host.crt_free_00bf65ac(front);
            at<std::int32_t>(group, 0x80) -= 1;
        }
    }
    std::int32_t n = at<std::int32_t>(group, 0x88);
    Object list_a = group + 0x6c;
    Object list_b = group + 0x78;
    Object b = at<Object>(at<Object>(group, 0x7c), 0x0);
    Object a = at<Object>(at<Object>(list_a, 0x4), 0x0);
    for (;;) {
        if (a == at<Object>(list_a, 0x4)) return;
        if (b != at<Object>(list_b, 0x4)) {
            if (b == at<Object>(list_b, 0x4)) host.invalid_parameter_00bf6713();
            if (at<std::int32_t>(b, 0x8) == n) {
                if (b == at<Object>(list_b, 0x4)) host.invalid_parameter_00bf6713();
                b = at<Object>(b, 0x0);
                n += 1;
                continue;
            }
        }
        if (a == at<Object>(list_a, 0x4)) host.invalid_parameter_00bf6713();
        if (at<Object>(a, 0x8) == member) {
            Object entry = host.list_buy_node_004857f0(list_b, b, at<Object>(b, 0x4), &n);
            host.list_incsize_0048fc80(list_b, 1);
            at<Object>(b, 0x4) = entry;
            at<Object>(at<Object>(entry, 0x4), 0x0) = entry;
            if (at<Object>(b, 0x4) == at<Object>(list_b, 0x4)) host.invalid_parameter_00bf6713();
            if (b != at<Object>(list_b, 0x4)) {
                if (b == at<Object>(list_b, 0x4)) host.invalid_parameter_00bf6713();
                b = at<Object>(b, 0x0);
            }
            if (a == at<Object>(list_a, 0x4)) host.invalid_parameter_00bf6713();
            Object next = at<Object>(a, 0x0);
            if (a != at<Object>(list_a, 0x4)) {
                at<Object>(at<Object>(a, 0x4), 0x0) = next;
                at<Object>(at<Object>(a, 0x0), 0x4) = at<Object>(a, 0x4);
                host.crt_free_00bf65ac(a);
                at<std::int32_t>(list_a, 0x8) -= 1;
            }
            a = next;
        } else {
            if (a == at<Object>(list_a, 0x4)) host.invalid_parameter_00bf6713();
            a = at<Object>(a, 0x0);
        }
        n += 1;
    }
}

namespace {
// The checks the image makes before writing the last element of a local
// std::vector<int> {proxy +0, first +4, last +8, end +0Ch}: first <= last,
// last-4 does not wrap and is >= first, and last-4 < last.
std::int32_t* checked_back(Left8Host& host, Object vector) {
    const std::uint32_t first = at<std::uint32_t>(vector, 0x4);
    const std::uint32_t last = at<std::uint32_t>(vector, 0x8);
    if (first > last) host.invalid_parameter_00bf6713();
    const std::uint32_t back = last - 4;
    if (back > last || back < first) host.invalid_parameter_00bf6713();
    if (back >= at<std::uint32_t>(vector, 0x8)) host.invalid_parameter_00bf6713();
    return reinterpret_cast<std::int32_t*>(static_cast<std::uintptr_t>(back));
}

// (h * 7B913h + 67933h) mod 5A1C9h through FILD, divided by the qword
// 369097.0 (00CE6428) and rounded to float at the image's store.
float hash_unit(std::uint32_t hash) {
    const std::uint32_t r = (hash * 0x7b913u + 0x67933u) % 0x5a1c9u;
    return static_cast<float>(static_cast<double>(static_cast<std::int32_t>(r)) / 369097.0);
}
}  // namespace

// 00498790 __thiscall(tree, i, j), RET 8, EAX = member. The cell table is the
// vector of 10h-byte rows at +48h (first +4Ch); row i's vector holds member
// pointers (checked twice each). An empty cell gets a member built from the
// deterministic hash 00484660 of a local vector<int> {i, j, 0} (pushed with
// 00442190), whose last element is set to 1, 2, 3 (checked) before the next
// three hashes:
//   h1: o1 = (float)((u(h1) - 0.5) * [+44h]);
//   h2: o2 likewise; x = (float)((i * [+40h] + [+58h]) + o1), z =
//       (float)(([+40h] * j + [+60h]) + o2) (x87 order of the image);
//   h3: index = 00480A90(ECX = h3, EDX = weights +30h); class = element index
//       of the vector at +20h (checked); member = class vtable +8();
//       member +98h(0, [[00E188A8]+19CCh], &identity), +114h(tree), +A0h();
//       004134F0 copies the identity into member+74h;
//   h4: yaw = (float)(u(h4) * 6.2831854820251465 (qword 00CE3828));
//       00467050(member+74h, 0, yaw, 0).
// member+A4h = x, +A8h = 0, +ACh = z; the cell (re-checked) gets the member and
// the local vector's storage is freed (00BF65AC). The hash is deterministic, not
// an RNG stream.
Object traffic_cell_member_00498790(Left8Host& host, Object tree, std::uint32_t i, std::uint32_t j) {
    auto row_at = [&]() {
        const std::uint32_t first = at<std::uint32_t>(tree, 0x4c);
        if (first == 0 || (static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(tree, 0x50) - first) >>
                                                       4)) <= i) {
            host.invalid_parameter_00bf6713();
        }
        return at<Object>(tree, 0x4c) + i * 0x10;
    };
    Object row = row_at();
    check_index(host, row, j, 2);
    Object member = at<Object*>(row, 0x4)[j];
    if (member != nullptr) return member;

    std::uint32_t vector[4] = {0, 0, 0, 0};
    Object v = reinterpret_cast<Object>(vector);
    std::int32_t pushed = static_cast<std::int32_t>(i);
    host.int_vector_push_back_00442190(v, &pushed);
    pushed = static_cast<std::int32_t>(j);
    host.int_vector_push_back_00442190(v, &pushed);
    pushed = 0;  // the float 0.0 local, pushed as its bit pattern
    host.int_vector_push_back_00442190(v, &pushed);
    const float u1 = hash_unit(host.int_vector_hash_00484660(v));
    const float o1 = static_cast<float>((static_cast<double>(u1) - 0.5) * at<float>(tree, 0x44));
    *checked_back(host, v) = 1;
    const std::uint32_t h2 = host.int_vector_hash_00484660(v);
    const double spacing = at<float>(tree, 0x40);
    const float x = static_cast<float>((static_cast<double>(static_cast<std::int32_t>(i)) * spacing +
                                        at<float>(tree, 0x58)) +
                                       o1);
    const float u2 = hash_unit(h2);
    const float o2 = static_cast<float>((static_cast<double>(u2) - 0.5) * at<float>(tree, 0x44));
    const float z = static_cast<float>((spacing * static_cast<std::int32_t>(j) + at<float>(tree, 0x60)) + o2);
    *checked_back(host, v) = 2;
    const std::uint32_t index = host.weighted_pick_00480a90(host.int_vector_hash_00484660(v), tree + 0x30);
    check_index(host, tree + 0x20, index, 2);
    member = host.vcall_create_instance_08(at<Object*>(tree, 0x24)[index]);
    float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
    host.vcall_attach_98(member, nullptr, at<Object>(game(host), 0x19cc), reinterpret_cast<Object>(identity));
    host.vcall_set_group_114(member, tree);
    host.vcall_a0(member);
    float identity2[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
    host.matrix_copy_004134f0(member + 0x74, identity2);
    *checked_back(host, v) = 3;
    const float u4 = hash_unit(host.int_vector_hash_00484660(v));
    const float yaw = static_cast<float>(static_cast<double>(u4) * 6.2831854820251465);
    host.matrix_set_rotation_from_euler_00467050(member + 0x74, 0.0f, yaw, 0.0f);
    at<float>(member, 0xa4) = x;
    at<float>(member, 0xa8) = 0.0f;
    at<float>(member, 0xac) = z;
    row = row_at();
    check_index(host, row, j, 2);
    at<Object*>(row, 0x4)[j] = member;
    if (vector[1] != 0) host.crt_free_00bf65ac(obj(vector[1]));
    return member;
}

// 00498C40 __thiscall(tree, Node** slot, const float* box), RET 8 (called by
// 004879B0). When the minimum cell size +7Ch is above the box width (box[3] -
// box[0], FCOMIP; unordered takes the other branch): 0047BA40(tree, box, &i0,
// &i1, &j0, &j1), and for i0..i1, j0..j1 (signed, inclusive) member =
// 00498790(tree, i, j), member+A8h = 0078CF20([game+19F0h], +A4h, +ACh),
// vtable +D8h(). Otherwise, an empty *slot gets operator new(70h) (00BF681B)
// and 00481510(node, box) (0 when the allocation fails), then 004879B0(tree,
// node+60h/+64h/+68h/+6Ch, node+0/+18h/+30h/+48h) with *slot re-read each time.
void traffic_visible_cells_00498c40(Left8Host& host, Object tree, Object slot, const float* box) {
    const double width = static_cast<double>(box[3]) - static_cast<double>(box[0]);
    if (static_cast<double>(at<float>(tree, 0x7c)) > width) {
        std::int32_t i0 = 0, i1 = 0, j0 = 0, j1 = 0;
        host.quad_cell_range_0047ba40(tree, box, &i0, &i1, &j0, &j1);
        for (std::int32_t i = i0; i <= i1; ++i) {
            for (std::int32_t j = j0; j <= j1; ++j) {
                Object member =
                    traffic_cell_member_00498790(host, tree, static_cast<std::uint32_t>(i), static_cast<std::uint32_t>(j));
                at<float>(member, 0xa8) = host.game_world_sample_water_height_0078cf20(
                    at<Object>(game(host), 0x19f0), at<float>(member, 0xa4), at<float>(member, 0xac));
                host.vcall_d8(member);
            }
        }
        return;
    }
    if (at<Object>(slot, 0) == nullptr) {
        Object memory = host.operator_new_00bf681b(0x70);
        at<Object>(slot, 0) = memory != nullptr ? host.quad_node_construct_00481510(memory, box) : nullptr;
    }
    host.traffic_bounds_update_004879b0(tree, at<Object>(slot, 0) + 0x60, at<Object>(slot, 0) + 0x00);
    host.traffic_bounds_update_004879b0(tree, at<Object>(slot, 0) + 0x64, at<Object>(slot, 0) + 0x18);
    host.traffic_bounds_update_004879b0(tree, at<Object>(slot, 0) + 0x68, at<Object>(slot, 0) + 0x30);
    host.traffic_bounds_update_004879b0(tree, at<Object>(slot, 0) + 0x6c, at<Object>(slot, 0) + 0x48);
}

// 00498D60 __thiscall(object, parent, class name, const matrix*), RET 0Ch, EAX =
// object: the temp-added animation object. 00496920(object, parent); vtable
// 00CE6660; +20h = 004722D0(ECX = class name) (the anim class); 004134F0
// copies the matrix to +28h; +24h = 0; r = [class+28h]; +68h..+70h = matrix
// translation (+30h..+38h) - r, then overwritten with translation + r (the
// image stores both; x87 sums rounded at the float stores).
Object temp_anim_construct_00498d60(Left8Host& host, Object object, Object parent, const void* class_name,
                                    const float* matrix) {
    list_node_construct_00496920(host, object, parent);
    at<std::uint32_t>(object, 0x0) = 0x00ce6660;
    at<Object>(object, 0x20) = host.anim_class_get_or_create_004722d0(class_name);
    host.matrix_copy_004134f0(object + 0x28, matrix);
    at<std::uint32_t>(object, 0x24) = 0;
    const float r = at<float>(at<Object>(object, 0x20), 0x28);
    at<float>(object, 0x68) = static_cast<float>(static_cast<double>(matrix[12]) - r);
    at<float>(object, 0x6c) = static_cast<float>(static_cast<double>(matrix[13]) - r);
    at<float>(object, 0x70) = static_cast<float>(static_cast<double>(matrix[14]) - r);
    at<float>(object, 0x68) = static_cast<float>(static_cast<double>(r) + matrix[12]);
    at<float>(object, 0x6c) = static_cast<float>(static_cast<double>(r) + matrix[13]);
    at<float>(object, 0x70) = static_cast<float>(static_cast<double>(r) + matrix[14]);
    return object;
}

// The plane-debris spawner (vtable 00CE6830: slot 0 0049D610 the scalar
// deleting destructor over 0049A580, slot 1 0049B1F0 the debris tick, slot 2
// 00480B70). Its only creator is 007D0B80: plane+C4Ch = 00498F80([[00E188A8]
// +21D0h], plane), then 0049A760(plane+C4Ch) fills the debris records; the
// observed object is the plane. Named "marker" by cc13-left8 until cc13-m498.
// 00498E60 __thiscall(spawner, parent, observed), RET 8, EAX = spawner.
// 00496920(spawner, parent); vtable 00CE6830; +24h..+2Ch = 0; the observer
// sub-object at +30h: vtable 00CE65E8, +34h..+3Ch = 0, +40h byte = 1, +44h = 0;
// +4 = 1. When +44h differs from observed: an old target is released with
// 006952A0(ECX = old, EDX = observer), +44h = observed, and a non-null target
// is registered with 00694A60(ECX = observed, EDX = observer). +48h = 1, +49h =
// 0, +4Ch = U(0, (float)([00E186F4] - [00E186F8])) through 00BD2F10 on stream 0
// (ECX = 0).
Object plane_debris_spawner_construct_00498e60(Left8Host& host, Object spawner, Object parent, Object observed) {
    list_node_construct_00496920(host, spawner, parent);
    at<std::uint32_t>(spawner, 0x0) = 0x00ce6830;
    at<std::uint32_t>(spawner, 0x24) = 0;
    at<std::uint32_t>(spawner, 0x28) = 0;
    at<std::uint32_t>(spawner, 0x2c) = 0;
    Object observer = spawner + 0x30;
    at<std::uint32_t>(observer, 0x4) = 0;
    at<std::uint32_t>(observer, 0x8) = 0;
    at<std::uint32_t>(observer, 0xc) = 0;
    at<Object>(observer, 0x14) = nullptr;
    at<std::uint8_t>(observer, 0x10) = 1;
    at<std::uint32_t>(observer, 0x0) = 0x00ce65e8;
    at<std::uint32_t>(spawner, 0x4) = 1;
    if (at<Object>(observer, 0x14) != observed) {
        if (at<Object>(observer, 0x14) != nullptr) {
            host.observer_unregister_pair_006952a0(at<Object>(observer, 0x14), observer);
        }
        at<Object>(observer, 0x14) = observed;
        if (observed != nullptr) host.observer_register_pair_00694a60(observed, observer);
    }
    at<std::uint8_t>(spawner, 0x48) = 1;
    at<std::uint8_t>(spawner, 0x49) = 0;
    const float span = static_cast<float>(static_cast<double>(host.global_f32(0x00e186f4)) - host.global_f32(0x00e186f8));
    at<float>(spawner, 0x4c) = host.random_uniform_00bd2f10(0, 0.0f, span);
    return spawner;
}

// 00498F80 __thiscall(owner, observed), RET 4, EAX: operator new(50h); on
// success 00498E60(memory, [owner+8], observed), else 0.
Object plane_debris_spawner_create_00498f80(Left8Host& host, Object owner, Object observed) {
    Object memory = host.operator_new_00bf681b(0x50);
    if (memory == nullptr) return nullptr;
    return plane_debris_spawner_construct_00498e60(host, memory, at<Object>(owner, 0x8), observed);
}

namespace {
// The path-runner speed step shared by 0049A360 and 0049A410: when the runtime
// was touched within the last 42 s (+18h > [00F876A4] - 42.0 qword, FCOMIP;
// unordered counts as not touched) the speed +130h rises by dt to at most 3.0
// (qword compare, float 00CE3854 stored), otherwise it falls by dt to at least
// 1.0; each step value is rounded to float. Then +84h = (float)(dt * speed +
// +84h) in one x87 sequence.
void path_runner_step_speed(Left8Host& host, Object runtime, float dt) {
    const double touched_before = static_cast<double>(host.global_f32(kGameTime)) - 42.0;
    float speed;
    if (static_cast<double>(at<float>(runtime, 0x18)) > touched_before) {
        const float v = static_cast<float>(static_cast<double>(at<float>(runtime, 0x130)) + dt);
        speed = static_cast<double>(v) > 3.0 ? 3.0f : v;
    } else {
        const float v = static_cast<float>(static_cast<double>(at<float>(runtime, 0x130)) - dt);
        speed = 1.0 > static_cast<double>(v) ? kOne : v;
    }
    at<float>(runtime, 0x130) = speed;
    at<float>(runtime, 0x84) =
        static_cast<float>(static_cast<double>(dt) * at<float>(runtime, 0x130) + at<float>(runtime, 0x84));
}
}  // namespace

// 0049A360 __thiscall(runtime, float dt), RET 4: the speed step above, then
// vtable +0Ch() (visible): true -> 00496BD0(runtime), false -> 0048A5A0(runtime).
void path_runner_tick_0049a360(Left8Host& host, Object runtime, float dt) {
    path_runner_step_speed(host, runtime, dt);
    if (host.vcall_group_is_visible_0c(runtime)) {
        host.path_runner_update_members_00496bd0(runtime);
        return;
    }
    host.traffic_runtime_kill_all_members_0048a5a0(runtime);
}

// 0049A410 __thiscall(lane, float dt), RET 4: the landing lane tick. When +180h
// is clear and the ship [+14Ch] has +118Bh set, 0048C230(lane) rebuilds the
// path; if +180h is still clear the speed +130h = 0 and the step is skipped,
// otherwise the speed step runs. t = min((float)([+18Ch] * 0.5 + [+84h]),
// [+190h]) (FCOMIP, unordered picks +190h). t == +190h (FUCOMIP, ordered equal)
// -> 0074AD90(ECX = ship), the landing finished. Otherwise vtable +0Ch():
// true -> 00497BB0(lane, t, 0); false -> 0048A5A0(lane), and when t then equals
// a re-read +190h, vtable +0(1) deletes the lane.
void landing_lane_tick_0049a410(Left8Host& host, Object lane, float dt) {
    bool step = true;
    if (at<std::uint8_t>(lane, 0x180) == 0) {
        if (at<std::uint8_t>(at<Object>(lane, 0x14c), 0x118b) != 0) host.landing_lane_rebuild_path_0048c230(lane);
        if (at<std::uint8_t>(lane, 0x180) == 0) {
            at<float>(lane, 0x130) = 0.0f;
            step = false;
        }
    }
    if (step) path_runner_step_speed(host, lane, dt);
    const float p = static_cast<float>(static_cast<double>(at<float>(lane, 0x18c)) * 0.5 + at<float>(lane, 0x84));
    const float end = at<float>(lane, 0x190);
    const float t = end > p ? p : end;
    if (t == at<float>(lane, 0x190)) {
        host.landing_ship_finish_landing_0074ad90(at<Object>(lane, 0x14c));
        return;
    }
    if (host.vcall_group_is_visible_0c(lane)) {
        host.path_runner_update_formation_00497bb0(lane, t, false);
        return;
    }
    host.traffic_runtime_kill_all_members_0048a5a0(lane);
    if (t == at<float>(lane, 0x190)) host.vcall_deleting_dtor_00(lane, 1);
}

namespace {
// The member release loop shared by 0049A580 (54h records) and 0049C6E0 (4Ch
// records): the vector at +20h (first +24h, last +28h) holds records whose
// first dword is a member entity; each set member gets vtable +114h(0) and
// 00926D90(2), and the slot is cleared (every access checked against the
// signed record count).
void release_record_members(Left8Host& host, Object owner, std::int32_t stride) {
    Object v = owner + 0x20;
    auto count = [&]() {
        return static_cast<std::uint32_t>(
            static_cast<std::int32_t>(at<std::uint32_t>(v, 0x8) - at<std::uint32_t>(v, 0x4)) / stride);
    };
    auto check = [&](std::uint32_t i) {
        if (at<std::uint32_t>(v, 0x4) == 0 || count() <= i) host.invalid_parameter_00bf6713();
    };
    for (std::uint32_t i = 0; at<std::uint32_t>(v, 0x4) != 0 && i < count(); ++i) {
        check(i);
        Object member = at<Object>(at<Object>(v, 0x4) + i * stride, 0);
        if (member != nullptr) {
            host.vcall_set_group_114(member, nullptr);
            host.mission_entity_kill_00926d90(member, 2);
        }
        check(i);
        at<Object>(at<Object>(v, 0x4) + i * stride, 0) = nullptr;
    }
}

// erase(begin(), end()) as the image inlines it: the first <= last checks, then
// the tail [last, end) (empty here, since last is the vector's end) is moved
// down with `move_one` and the new last is first + tail count * stride.
template <class MoveOne>
void erase_all(Left8Host& host, Object v, std::int32_t stride, MoveOne move_one) {
    const std::uint32_t last = at<std::uint32_t>(v, 0x8);
    if (at<std::uint32_t>(v, 0x4) > last) host.invalid_parameter_00bf6713();
    const std::uint32_t first = at<std::uint32_t>(v, 0x4);
    if (first > at<std::uint32_t>(v, 0x8)) host.invalid_parameter_00bf6713();
    if (first == last) return;
    const std::uint32_t end = at<std::uint32_t>(v, 0x8);
    const std::int32_t tail = static_cast<std::int32_t>(end - last) / stride;
    std::uint32_t dst = first;
    for (std::uint32_t src = last; src != end; src += static_cast<std::uint32_t>(stride), dst += stride) {
        move_one(obj(dst), obj(src));
    }
    at<std::uint32_t>(v, 0x8) = first + static_cast<std::uint32_t>(tail * stride);
}

void free_vector_storage(Left8Host& host, Object v) {
    if (at<Object>(v, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v, 0x4));
    at<std::uint32_t>(v, 0x4) = 0;
    at<std::uint32_t>(v, 0x8) = 0;
    at<std::uint32_t>(v, 0xc) = 0;
}
}  // namespace

// 0049A580 __fastcall(spawner), RET: the 00498E60 spawner destructor (vtable
// 00CE6830). When +49h is set the dword [00E186E8] is decremented. The member
// release loop over the 54h records at +20h; erase(begin, end) with the
// copy-assign 0047F930 for the (empty) tail; 00495D40(vector, 0) (the image
// also reserves a 54h by-value record argument it does not initialise; it is
// not modelled). The observer at +30h gets vtable 00CE6430, its target +44h
// (when set) is released with 006952A0(ECX = target, EDX = observer), then
// 00695870(observer). The record storage is freed and zeroed and 00489DD0
// runs the list-node base destructor.
void plane_debris_spawner_destruct_0049a580(Left8Host& host, Object spawner) {
    at<std::uint32_t>(spawner, 0x0) = 0x00ce6830;
    if (at<std::uint8_t>(spawner, 0x49) != 0) host.global_add_i32(0x00e186e8, -1);
    release_record_members(host, spawner, 0x54);
    Object v = spawner + 0x20;
    erase_all(host, v, 0x54, [&](Object dst, Object src) { host.record54_copy_assign_0047f930(dst, src); });
    host.record54_vector_resize_00495d40(v, 0);
    Object observer = spawner + 0x30;
    at<std::uint32_t>(observer, 0x0) = 0x00ce6430;
    if (at<Object>(observer, 0x14) != nullptr) host.observer_unregister_pair_006952a0(at<Object>(observer, 0x14), observer);
    host.callback_owner_destroy_00695870(observer);
    free_vector_storage(host, v);
    host.list_node_destruct_00489dd0(spawner);
}

// 0049C6E0 __fastcall(owner), RET: destructor of the vtable 00CE6850 class.
// The member release loop over the 4Ch records at +20h; erase(begin, end) with
// the 4Ch record copy inlined (field by field) for the empty tail;
// 00495E20(vector, 0) (with an uninitialised 4Ch by-value argument, not
// modelled); the storage is freed and zeroed; 00489DD0.
void record4c_owner_destruct_0049c6e0(Left8Host& host, Object owner) {
    at<std::uint32_t>(owner, 0x0) = 0x00ce6850;
    release_record_members(host, owner, 0x4c);
    Object v = owner + 0x20;
    erase_all(host, v, 0x4c, [](Object dst, Object src) { std::memcpy(dst, src, 0x4c); });
    host.record4c_vector_resize_00495e20(v, 0);
    free_vector_storage(host, v);
    host.list_node_destruct_00489dd0(owner);
}

// 0049D540 __fastcall(tree), RET: destructor of the scene traffic cell tree
// (vtable 00CE6860). 00484AE0(tree, tree+80h, tree+58h) prunes the root; when
// the row vector (+48h, first +4Ch) has storage, 004914A0(ECX = first, EDX =
// last, &rows, tree) destroys the rows and the storage is freed; +4Ch..+54h,
// then the vectors at +30h and +20h are freed and zeroed; 00489DD0.
void traffic_tree_destruct_0049d540(Left8Host& host, Object tree) {
    at<std::uint32_t>(tree, 0x0) = 0x00ce6860;
    host.quad_tree_prune_00484ae0(tree, tree + 0x80, reinterpret_cast<const float*>(tree + 0x58));
    Object rows = tree + 0x48;
    if (at<Object>(rows, 0x4) != nullptr) {
        host.row_vectors_destroy_004914a0(at<Object>(rows, 0x4), at<Object>(rows, 0x8), rows, tree);
        host.crt_free_00bf65ac(at<Object>(rows, 0x4));
    }
    at<std::uint32_t>(rows, 0x4) = 0;
    at<std::uint32_t>(rows, 0x8) = 0;
    at<std::uint32_t>(rows, 0xc) = 0;
    free_vector_storage(host, tree + 0x30);
    free_vector_storage(host, tree + 0x20);
    host.list_node_destruct_00489dd0(tree);
}

namespace {
// A native string {length +0, data +4}: its inlined destructor returns data to
// the sized pool (00419CC0, then 00BD1510(ECX = pool, data, length + 1, 1)).
struct NativeString {
    std::uint32_t length{0};
    char* data{nullptr};
};
void release_native_string(Left8Host& host, NativeString& s) {
    if (s.data != nullptr) host.string_pool_return_00bd1510(host.string_pool_00419cc0(), s.data, s.length + 1, 1);
}
}  // namespace

// 0049DC00 __thiscall(manager, LuaObject table by value), RET 14h: TempAddAnim.
// name = table.AnimClassName (00B67800 into a temporary, 00B662B0, 0041E870
// assign, 00B67700); position = 00888760(ECX = &position, EDX = table.Position);
// operator new(80h) -> 00498D60(memory, [manager+8], &name, &matrix) with the
// identity matrix translated to the position (0 when the allocation fails).
// When table.tempID is not nil (00B65FB0), its string keys the map at
// manager+34h (0049CB60 returns the slot) and the slot takes the object. The
// strings are released through the pool and the by-value table is destructed.
void temp_add_anim_0049dc00(Left8Host& host, Object manager, LuaObject* table) {
    LuaObject temp{};
    NativeString name;
    host.native_string_assign_0041e870(&name, host.lua_get_string_00b662b0(
                                                   host.lua_get_by_name_00b67800(table, &temp, "AnimClassName")));
    host.lua_object_destruct_00b67700(&temp);
    float position[3] = {0.0f, 0.0f, 0.0f};
    host.lua_read_vector3_00888760(position, host.lua_get_by_name_00b67800(table, &temp, "Position"));
    host.lua_object_destruct_00b67700(&temp);
    Object memory = host.operator_new_00bf681b(0x80);
    Object created = nullptr;
    if (memory != nullptr) {
        const float matrix[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, position[0], position[1], position[2], kOne};
        created = temp_anim_construct_00498d60(host, memory, at<Object>(manager, 0x8), &name, matrix);
    }
    const bool has_id = !host.lua_is_nil_00b65fb0(host.lua_get_by_name_00b67800(table, &temp, "tempID"));
    host.lua_object_destruct_00b67700(&temp);
    if (has_id) {
        NativeString id;
        host.native_string_assign_0041e870(&id, host.lua_get_string_00b662b0(
                                                    host.lua_get_by_name_00b67800(table, &temp, "tempID")));
        *host.temp_map_subscript_0049cb60(manager + 0x34, &id) = created;
        release_native_string(host, id);
        host.lua_object_destruct_00b67700(&temp);
    }
    release_native_string(host, name);
    host.lua_object_destruct_00b67700(table);
}

// 0049DE90 __thiscall(manager, const NativeString* id), RET 4:
// RemoveTempAddedStuff. slot = 0049CB60(manager+34h, id); a set slot gets
// vtable +0(1) (deleting destructor) and is cleared.
void remove_temp_added_0049de90(Left8Host& host, Object manager, const void* id) {
    Object* slot = host.temp_map_subscript_0049cb60(manager + 0x34, id);
    if (*slot != nullptr) {
        host.vcall_deleting_dtor_00(*slot, 1);
        *slot = nullptr;
    }
}

namespace {
// Map teardown as the image inlines it: erase(begin, end) through the typed
// range eraser (ECX = map; &result, map, head->left, map, head), then the head
// node is freed and the head/size words are cleared.
template <class Erase>
void destroy_map(Left8Host& host, Object map, Erase erase) {
    Object head = at<Object>(map, 0x4);
    std::uint32_t result[2] = {0, 0};
    erase(map, reinterpret_cast<Object>(result), map, at<Object>(head, 0x0), map, head);
    host.crt_free_00bf65ac(at<Object>(map, 0x4));
    at<std::uint32_t>(map, 0x4) = 0;
    at<std::uint32_t>(map, 0x8) = 0;
}
}  // namespace

// 0049D4C0 __thiscall(record, source), RET 4 (from 00832F54): a property block
// on the stack (114h-byte frame) is constructed by 008F41A0(&block, 0),
// 0043D8F0(ECX = source, EDX = &block) fills it, 0049CF80(record, &block)
// applies it and 008F5410(&block) destroys it.
void traffic_record_apply_from_lua_0049d4c0(Left8Host& host, Object record, Object source) {
    PropertyBlock block{};
    host.property_block_construct_008f41a0(&block, 0);
    host.property_block_from_lua_0043d8f0(source, &block);
    host.traffic_record_apply_properties_0049cf80(record, &block);
    host.property_block_destruct_008f5410(&block);
}

// 0049F0E0 __fastcall(manager), RET: destructor of the vtable 00CE68D0
// manager. 0041CC80(ECX = manager+4) destroys the owned critical section; the
// owned object [+8] (when set) gets vtable +0(1) and is cleared; then the six
// maps at +4Ch, +40h, +34h, +28h, +1Ch, +10h are emptied with the erasers
// 0049A230, 0049A230, 00499B90, 004906D0, 00496390, 00493D50 and their head
// nodes freed.
void traffic_manager_destruct_0049f0e0(Left8Host& host, Object manager) {
    at<std::uint32_t>(manager, 0x0) = 0x00ce68d0;
    host.critical_section_destroy_owned_0041cc80(manager + 0x4);
    if (Object owned = at<Object>(manager, 0x8); owned != nullptr) {
        host.vcall_deleting_dtor_00(owned, 1);
        at<Object>(manager, 0x8) = nullptr;
    }
    destroy_map(host, manager + 0x4c, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_0049a230(m, r, c0, f, c1, l);
    });
    destroy_map(host, manager + 0x40, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_0049a230(m, r, c0, f, c1, l);
    });
    destroy_map(host, manager + 0x34, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_00499b90(m, r, c0, f, c1, l);
    });
    destroy_map(host, manager + 0x28, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_004906d0(m, r, c0, f, c1, l);
    });
    destroy_map(host, manager + 0x1c, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_00496390(m, r, c0, f, c1, l);
    });
    destroy_map(host, manager + 0x10, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_00493d50(m, r, c0, f, c1, l);
    });
}

// 0049F260 ECX = outer object, RET: load ECX = [outer+0Ch] once, then
// conditionally tail-call 0049E560 with that CURRENT INNER receiver.
void traffic_refresh_if_set_0049f260(Left8Host& host, Object object) {
    const Object current = obj(at<std::uint32_t>(object, 0xc));
    if (current != nullptr) host.traffic_layers_rebuild_0049e560(current);
}

// 0049F9B0 __fastcall(record), RET, EAX = record: the 4Ch traffic record
// constructor (appended by 0082FD20). Map +34h: head from 004858D0(ECX = map),
// head+15h = 1, the three links to itself, size +3Ch = 0. Map +40h: head from
// 00443E20(ECX = map), head+19h = 1, links, size +48h = 0. +0, +4, +8 = 0, byte
// +0Ch = 0, +14h = 0, floats +18h..+30h = 0 (+10h is not written).
Object traffic_record_construct_0049f9b0(Left8Host& host, Object record) {
    Object map_a = record + 0x34;
    Object head = host.tree_buy_head_node_004858d0(map_a);
    at<Object>(map_a, 0x4) = head;
    at<std::uint8_t>(head, 0x15) = 1;
    at<Object>(at<Object>(map_a, 0x4), 0x4) = at<Object>(map_a, 0x4);
    at<Object>(at<Object>(map_a, 0x4), 0x0) = at<Object>(map_a, 0x4);
    at<Object>(at<Object>(map_a, 0x4), 0x8) = at<Object>(map_a, 0x4);
    at<std::uint32_t>(map_a, 0x8) = 0;
    Object map_b = record + 0x40;
    head = host.string_float_map_blank_node_00443e20(map_b);
    at<Object>(map_b, 0x4) = head;
    at<std::uint8_t>(head, 0x19) = 1;
    at<Object>(at<Object>(map_b, 0x4), 0x4) = at<Object>(map_b, 0x4);
    at<Object>(at<Object>(map_b, 0x4), 0x0) = at<Object>(map_b, 0x4);
    at<Object>(at<Object>(map_b, 0x4), 0x8) = at<Object>(map_b, 0x4);
    at<std::uint32_t>(map_b, 0x8) = 0;
    at<std::uint32_t>(record, 0x0) = 0;
    at<std::uint32_t>(record, 0x8) = 0;
    at<std::uint32_t>(record, 0x4) = 0;
    at<std::uint8_t>(record, 0xc) = 0;
    at<std::uint32_t>(record, 0x14) = 0;
    at<float>(record, 0x30) = 0.0f;
    at<float>(record, 0x2c) = 0.0f;
    at<float>(record, 0x28) = 0.0f;
    at<float>(record, 0x24) = 0.0f;
    at<float>(record, 0x1c) = 0.0f;
    at<float>(record, 0x18) = 0.0f;
    at<float>(record, 0x20) = 0.0f;
    return record;
}

// 0049FA70 __thiscall(record, const record* source), RET 4, EAX = record: the
// copy constructor. Dwords +0..+8, byte +0Ch, dwords +10h..+30h copied; then
// 004990B0(ECX = record+34h, source+34h) and 0049DEC0(ECX = record+40h,
// source+40h) copy the two maps.
Object traffic_record_copy_0049fa70(Left8Host& host, Object record, Object source) {
    at<std::uint32_t>(record, 0x0) = at<std::uint32_t>(source, 0x0);
    at<std::uint32_t>(record, 0x4) = at<std::uint32_t>(source, 0x4);
    at<std::uint32_t>(record, 0x8) = at<std::uint32_t>(source, 0x8);
    at<std::uint8_t>(record, 0xc) = at<std::uint8_t>(source, 0xc);
    for (std::size_t off = 0x10; off <= 0x30; off += 4) at<std::uint32_t>(record, off) = at<std::uint32_t>(source, off);
    host.tree_copy_construct_004990b0(record + 0x34, source + 0x34);
    host.string_float_map_copy_construct_0049dec0(record + 0x40, source + 0x40);
    return record;
}

// 0049F910 __fastcall(record), RET: the 4Ch traffic record destructor. The map
// at +40h is emptied with 00444B10 and the one at +34h with 00492A00, each
// head node freed and its head/size cleared.
void traffic_record_destruct_0049f910(Left8Host& host, Object record) {
    destroy_map(host, record + 0x40, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_00444b10(m, r, c0, f, c1, l);
    });
    destroy_map(host, record + 0x34, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_00492a00(m, r, c0, f, c1, l);
    });
}

// 0049FB20 __fastcall(runtime), RET: destructor of the path-runner traffic
// runtime (vtable 00CE68D4). 0048A5A0(runtime) kills the members; the vector
// at +120h is freed and zeroed; the vector at +110h (first +114h), when it has
// storage, gets 00432050(ECX = first, EDX = last, &vector, runtime) and is
// freed; then +114h..+11Ch, and the vectors at +100h, +F0h and +E0h are freed
// and zeroed; 0049F910(+8Ch) (the embedded traffic record), 00487110(+78h),
// 00485340(+6Ch), 0049F910(+20h), and 00489DD0(runtime).
void path_runtime_destruct_0049fb20(Left8Host& host, Object runtime) {
    at<std::uint32_t>(runtime, 0x0) = 0x00ce68d4;
    host.traffic_runtime_kill_all_members_0048a5a0(runtime);
    free_vector_storage(host, runtime + 0x120);
    Object v = runtime + 0x110;
    if (at<Object>(v, 0x4) != nullptr) {
        host.element_range_destroy_00432050(at<Object>(v, 0x4), at<Object>(v, 0x8), v, runtime);
        host.crt_free_00bf65ac(at<Object>(v, 0x4));
    }
    at<std::uint32_t>(v, 0x4) = 0;
    at<std::uint32_t>(v, 0x8) = 0;
    at<std::uint32_t>(v, 0xc) = 0;
    free_vector_storage(host, runtime + 0x100);
    free_vector_storage(host, runtime + 0xf0);
    free_vector_storage(host, runtime + 0xe0);
    traffic_record_destruct_0049f910(host, runtime + 0x8c);
    host.list_destruct_00487110(runtime + 0x78);
    host.list_tidy_00485340(runtime + 0x6c);
    traffic_record_destruct_0049f910(host, runtime + 0x20);
    host.list_node_destruct_00489dd0(runtime);
}

namespace {
// A vector whose elements need destruction: when it has storage, 00432050(ECX
// = first, EDX = last, &vector, owner) then the storage is freed; the three
// words are cleared either way.
void destroy_element_vector(Left8Host& host, Object v, Object owner) {
    if (at<Object>(v, 0x4) != nullptr) {
        host.element_range_destroy_00432050(at<Object>(v, 0x4), at<Object>(v, 0x8), v, owner);
        host.crt_free_00bf65ac(at<Object>(v, 0x4));
    }
    at<std::uint32_t>(v, 0x4) = 0;
    at<std::uint32_t>(v, 0x8) = 0;
    at<std::uint32_t>(v, 0xc) = 0;
}
}  // namespace

// 004A02E0 __fastcall(object), RET: destroys the element vectors at +6Ch, +5Ch,
// +4Ch and +3Ch (00432050 + free each), then 0048EE90(object) for the base.
void soldier_type_destruct_004a02e0(Left8Host& host, Object object) {
    destroy_element_vector(host, object + 0x6c, object);
    destroy_element_vector(host, object + 0x5c, object);
    destroy_element_vector(host, object + 0x4c, object);
    destroy_element_vector(host, object + 0x3c, object);
    host.traffic_path_set_destruct_0048ee90(object);
}

// 004A6510 __fastcall(runtime), RET: destructor of the vtable 00CE6A08 path
// runtime. 004A5B30(ECX = [[00E188A8]+21D0h], [runtime+14Ch], runtime)
// unregisters it, then 0049FB20(runtime).
void path_runtime_owned_destruct_004a6510(Left8Host& host, Object runtime) {
    at<std::uint32_t>(runtime, 0x0) = 0x00ce6a08;
    host.traffic_registry_remove_004a5b30(at<Object>(game(host), 0x21d0), at<Object>(runtime, 0x14c), runtime);
    path_runtime_destruct_0049fb20(host, runtime);
}

// 004A6580 __fastcall(group), RET: destructor of the ship idle actor group
// (vtable 00CE69D4). 00487940(group); 004A5B30([[00E188A8]+21D0h], [group+20h],
// group); the vector<bool> at +80h: size = 0, words +84h freed and zeroed; the
// vector at +70h freed; +60h (8-byte anim names, 00432050) destroyed and freed;
// +50h and +40h freed; the vector<bool> at +28h: size = 0, words +2Ch freed;
// 00489DD0(group).
void ship_idle_group_destruct_004a6580(Left8Host& host, Object group) {
    at<std::uint32_t>(group, 0x0) = 0x00ce69d4;
    host.traffic_group_flush_pending_visibility_00487940(group);
    host.traffic_registry_remove_004a5b30(at<Object>(game(host), 0x21d0), at<Object>(group, 0x20), group);
    at<std::uint32_t>(group, 0x80) = 0;
    free_vector_storage(host, group + 0x84);
    free_vector_storage(host, group + 0x70);
    destroy_element_vector(host, group + 0x60, group);
    free_vector_storage(host, group + 0x50);
    free_vector_storage(host, group + 0x40);
    at<std::uint32_t>(group, 0x28) = 0;
    free_vector_storage(host, group + 0x2c);
    host.list_node_destruct_00489dd0(group);
}

// 004A67C0 __fastcall(lane), RET: destructor of the landing lane (vtable
// 00CE6A68). When +180h is set, 00926D90(007ACA30(ECX = [lane+20h]), 2) kills
// the lane's path entity; then 004A6510(lane).
void landing_lane_destruct_004a67c0(Left8Host& host, Object lane) {
    at<std::uint32_t>(lane, 0x0) = 0x00ce6a68;
    if (at<std::uint8_t>(lane, 0x180) != 0) {
        host.mission_entity_kill_00926d90(host.path_object_get_entity_007aca30(at<Object>(lane, 0x20)), 2);
    }
    path_runtime_owned_destruct_004a6510(host, lane);
}

namespace {
// The landing pad key: the pad record's first entity [+0], else [+8]; key =
// entity + 1E4h, or 0 without one.
std::uint32_t pad_key(Object pad) {
    Object entity = at<Object>(pad, 0x0);
    if (entity == nullptr) entity = at<Object>(pad, 0x8);
    return entity != nullptr ? static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(entity + 0x1e4)) : 0u;
}
}  // namespace

// 004A4520 __thiscall(manager, config, a3, ship), RET 0Ch: launch the landing
// troops of a ship (caller 0074A990 BSP_LandingShip_BeginLandingAtPad). Under
// EnterCriticalSection([+4]) with [cs+18h] += 1, for each pad record in the list
// at config+1FCh (head [+200h]; node value +8; every node access checked):
// 00488630 looks the pad key up in the multimap at +28h (container checked).
//   Not found: pad+10h = 0, pad+14h = 1; group = operator new(E4h) ->
//   004A0410(group, [manager+8], pad, a3, ship) (0 on failure); 004926A0
//   inserts {key, group}.
//   Found with [[ship+538h]+814h] == [group+44h]: group+DCh = ship.
//   Found otherwise: the key is looked up again and, when present, 0048C190 on
//   that group (its ship left); the first group's +20h = 1; then as not found
//   (pad+10h = 0, pad+14h = 1, a new group inserted).
// Then [cs+18h] -= 1 and LeaveCriticalSection.
void landing_troops_launch_004a4520(Left8Host& host, Object manager, Object config, std::uint32_t a3, Object ship) {
    Object section = at<Object>(manager, 0x4);
    host.enter_critical_section(section);
    at<std::int32_t>(section, 0x18) += 1;
    Object list = config + 0x1fc;
    Object map = manager + 0x28;
    auto check_node = [&](Object node) {
        if (node == at<Object>(list, 0x4)) host.invalid_parameter_00bf6713();
    };
    auto create_and_insert = [&](Object node) {
        check_node(node);
        at<std::uint32_t>(at<Object>(node, 0x8), 0x10) = 0;
        check_node(node);
        at<std::uint32_t>(at<Object>(node, 0x8), 0x14) = 1;
        Object memory = host.operator_new_00bf681b(0xe4);
        Object group = nullptr;
        if (memory != nullptr) {
            check_node(node);
            group = host.landing_troop_group_construct_004a0410(memory, at<Object>(manager, 0x8), at<Object>(node, 0x8),
                                                                a3, ship);
        }
        check_node(node);
        std::uint32_t pair[2] = {pad_key(at<Object>(node, 0x8)),
                                 static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(group))};
        std::uint32_t result[3] = {0, 0, 0};
        host.tree_insert_equal_004926a0(map, reinterpret_cast<Object>(result), pair);
    };
    for (Object node = at<Object>(at<Object>(list, 0x4), 0x0); node != at<Object>(list, 0x4);) {
        check_node(node);
        std::uint32_t key = pad_key(at<Object>(node, 0x8));
        std::uint32_t found[2] = {0, 0};
        host.tree_find_00488630(map, reinterpret_cast<Object>(found), &key);
        Object container = obj(found[0]);
        Object hit = obj(found[1]);
        Object head = at<Object>(map, 0x4);
        if (container == nullptr || container != map) host.invalid_parameter_00bf6713();
        if (hit == head) {
            create_and_insert(node);
        } else {
            Object ship_class = at<Object>(ship, 0x538);
            if (container == nullptr) host.invalid_parameter_00bf6713();
            if (hit == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
            if (at<std::uint32_t>(ship_class, 0x814) == at<std::uint32_t>(at<Object>(hit, 0x10), 0x44)) {
                if (hit == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
                at<Object>(at<Object>(hit, 0x10), 0xdc) = ship;
            } else {
                check_node(node);
                std::uint32_t key2 = pad_key(at<Object>(node, 0x8));
                std::uint32_t again[2] = {0, 0};
                host.tree_find_00488630(map, reinterpret_cast<Object>(again), &key2);
                Object head2 = at<Object>(map, 0x4);
                Object container2 = obj(again[0]);
                if (container2 == nullptr || container2 != map) host.invalid_parameter_00bf6713();
                if (obj(again[1]) != head2) {
                    if (container2 == nullptr) host.invalid_parameter_00bf6713();
                    if (obj(again[1]) == at<Object>(container2, 0x4)) host.invalid_parameter_00bf6713();
                    host.landing_troop_on_ship_left_0048c190(at<Object>(obj(again[1]), 0x10));
                }
                if (hit == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
                at<std::uint8_t>(at<Object>(hit, 0x10), 0x20) = 1;
                create_and_insert(node);
            }
        }
        check_node(node);
        node = at<Object>(node, 0x0);
    }
    Object section2 = at<Object>(manager, 0x4);
    at<std::int32_t>(section2, 0x18) -= 1;
    host.leave_critical_section(section2);
}

// 004A5B90 __thiscall(registry, key, index), RET 8. groups = 004A5750(registry
// + 1Ch, &key) (the vector of traffic groups registered for the key). The first
// group whose type +4 is 3 (each access checked) is used: when index < its
// member count +24h, member = element index of its +74h vector (checked), the
// group's vtable +8(member) releases it, and a non-null member gets vtable
// +114h(0) and 00926D90(2).
void traffic_registry_kill_member_004a5b90(Left8Host& host, Object registry, std::uint32_t key, std::int32_t index) {
    Object groups = host.traffic_registry_groups_004a5750(registry + 0x1c, &key);
    const std::int32_t count =
        at<std::uint32_t>(groups, 0x4) == 0
            ? 0
            : static_cast<std::int32_t>(at<std::uint32_t>(groups, 0x8) - at<std::uint32_t>(groups, 0x4)) >> 2;
    for (std::int32_t i = 0; i < count; ++i) {
        check_index(host, groups, static_cast<std::uint32_t>(i), 2);
        if (at<std::int32_t>(at<Object*>(groups, 0x4)[i], 0x4) != 3) continue;
        check_index(host, groups, static_cast<std::uint32_t>(i), 2);
        Object group = at<Object*>(groups, 0x4)[i];
        if (index >= at<std::int32_t>(group, 0x24)) return;
        check_index(host, group + 0x70, static_cast<std::uint32_t>(index), 2);
        Object member = at<Object*>(group, 0x74)[index];
        host.vcall_group_release_member_08(group, member);
        if (member != nullptr) {
            host.vcall_set_group_114(member, nullptr);
            host.mission_entity_kill_00926d90(member, 2);
        }
        return;
    }
}

// 004AB9A0 __fastcall(table), RET, EAX: the weighted entry pick of
// 0049C000. With an empty entry vector (+18h, last +1Ch) returns 0. Otherwise r
// = _ftol(00BD2F10(stream 0, 0, (float)(int)[+10h])) (CVTSI2SS, truncating
// conversion); for i < count: when entry i's +8 < r (signed) stop, else r -=
// entry +8. An index that ran off the end becomes count - 1. Returns entry i
// (each access checked). The comparison direction is the image's.
Object weighted_entry_pick_004ab9a0(Left8Host& host, Object table) {
    if (at<std::uint32_t>(table, 0x18) == 0) return nullptr;
    const std::int32_t count =
        static_cast<std::int32_t>(at<std::uint32_t>(table, 0x1c) - at<std::uint32_t>(table, 0x18)) >> 2;
    if (count == 0) return nullptr;
    const float total = static_cast<float>(at<std::int32_t>(table, 0x10));
    std::int32_t r = static_cast<std::int32_t>(host.random_uniform_00bd2f10(0, 0.0f, total));
    std::int32_t i = 0;
    for (; i < count; ++i) {
        check_index(host, table + 0x14, static_cast<std::uint32_t>(i), 2);
        if (at<std::int32_t>(at<Object*>(table, 0x18)[i], 0x8) < r) break;
        check_index(host, table + 0x14, static_cast<std::uint32_t>(i), 2);
        r -= at<std::int32_t>(at<Object*>(table, 0x18)[i], 0x8);
    }
    if (i == count) i = count - 1;
    check_index(host, table + 0x14, static_cast<std::uint32_t>(i), 2);
    return at<Object*>(table, 0x18)[i];
}

// ---------------------------------------------------------------------------
// Soldier and wreck class helpers.

// 004AF890 __fastcall(class), RET, EAX = soldier (soldier class vtable slot):
// operator new(1B0h) (00BF55BE) and _memset(p, 0, 1B0h) before the null test;
// on success 00489F30(p) builds the class-entity base, vtables 00CE7008 /
// 00CE6FEC (+10h) / 00CE6FE4 (+24h), class id +C4h = 3Ah. Then (also with a
// null p) 0047B9E0(ECX = p, class) and p+188h = class.
Object soldier_class_create_instance_004af890(Left8Host& host, Object soldier_class) {
    Object p = host.operator_new_00bf55be(0x1b0);
    std::memset(p, 0, 0x1b0);
    if (p != nullptr) {
        host.class_entity_construct_base_00489f30(p);
        at<std::uint32_t>(p, 0x0) = 0x00ce7008;
        at<std::uint32_t>(p, 0x10) = 0x00ce6fec;
        at<std::uint32_t>(p, 0x24) = 0x00ce6fe4;
        at<std::uint32_t>(p, 0xc4) = 0x3a;
    }
    host.class_entity_set_class_0047b9e0(p, soldier_class);
    at<Object>(p, 0x188) = soldier_class;
    return p;
}

// 004AFFE0 __fastcall(soldier), RET (soldier vtable 00CE7008 slot): +190h = -1;
// +18Ch = ([[+188h]+30h] != 0) (SETA); 0048DBD0(soldier) runs the base
// SEntityInit. node = [[+178h]+160h]+0Ch (re-read after each call): when
// 00B6D980(node) is non-zero, 00B6DCA0(ECX = [node+34h], [globals+4]), else
// 00B6DCA0(ECX = node, [globals+4]), globals = 00485520() (float argument).
void soldier_sentity_init_004affe0(Left8Host& host, Object soldier) {
    at<std::int32_t>(soldier, 0x190) = -1;
    at<std::uint8_t>(soldier, 0x18c) = at<std::uint32_t>(at<Object>(soldier, 0x188), 0x30) != 0 ? 1 : 0;
    host.class_entity37_sentity_init_base_0048dbd0(soldier);
    auto node = [&]() { return at<Object>(at<Object>(at<Object>(soldier, 0x178), 0x160), 0xc); };
    if (host.node_query_00b6d980(node()) != 0) {
        const float value = at<float>(host.soldier_globals_00485520(), 0x4);
        host.node_set_word50_00b6dca0(at<Object>(node(), 0x34), value);
        return;
    }
    const float value = at<float>(host.soldier_globals_00485520(), 0x4);
    host.node_set_word50_00b6dca0(node(), value);
}

// 004B1880 __thiscall(matrix), RET, EAX = matrix: 0085DAD0(ECX = matrix).
Object matrix_orthonormalize_return_004b1880(Left8Host& host, Object matrix) {
    host.matrix_orthonormalize_0085dad0(matrix);
    return matrix;
}

// 004B3F10, the register ABI of 004B3D20 passed through unchanged, EAX: class =
// 004B3D20(...); when non-null with +28h clear, vtable +0Ch(class) activates
// it. Returns the class in every case.
Object wreck_class_get_or_create_activated_004b3f10(Left8Host& host, const void* name) {
    Object wreck_class = wreck_class_get_or_create_004b3d20(host, name);
    if (wreck_class != nullptr && at<std::uint8_t>(wreck_class, 0x28) == 0) host.vcall_class_activate_0c(wreck_class);
    return wreck_class;
}

namespace {
// The inlined NativeString-from-C-string construction: {0, 0}, 0041DD40(&s,
// strlen, 1), then memcpy(data, text, length + 1) when data is set.
void native_string_from_text(Left8Host& host, NativeString& s, const char* text) {
    s = NativeString{};
    host.native_string_resize_0041dd40(&s, static_cast<std::uint32_t>(std::strlen(text)), 1);
    if (s.data != nullptr) std::memcpy(s.data, text, s.length + 1);
}
}  // namespace

namespace {
// The class-registry get-or-load shared by 004B1400 (SoldierClass) and 004B3D20
// (WreckClass): slot = lookup(registry()+4, name); when empty, globals
// (00B67980 on [00E188A8]+1A0Ch), table = globals.<table_name>, row =
// 00B68100(&table, &row, name); class = create() (operator new + the class
// constructor); unless class+4 is the name itself, 0041DD40 resizes class+4 to
// the name length and a non-empty name is memcpy'd ([class+4] bytes); class
// vtable +4(&row); the slot (looked up again) takes the class; row, table and
// globals are destructed.
template <class Registry, class Lookup, class Create>
Object class_registry_load(Left8Host& host, const void* name, const char* table_name, Registry registry, Lookup lookup,
                           Create create) {
    LuaObject globals{};
    LuaObject table{};
    LuaObject row{};
    host.lua_get_globals_00b67980(game(host) + 0x1a0c, &globals);
    host.lua_get_by_name_00b67800(&globals, &table, table_name);
    host.lua_get_by_native_string_00b68100(&table, &row, name);
    Object entity_class = create();
    Object field = entity_class + 0x4;
    const NativeString* source = static_cast<const NativeString*>(name);
    if (field != reinterpret_cast<Object>(const_cast<NativeString*>(source))) {
        host.native_string_resize_0041dd40(field, source->length, 1);
        if (source->length != 0) std::memcpy(at<char*>(field, 0x4), source->data, at<std::uint32_t>(field, 0x0));
    }
    host.vcall_class_read_lua_04(entity_class, &row);
    *lookup(registry() + 0x4, name) = entity_class;
    host.lua_object_destruct_00b67700(&row);
    host.lua_object_destruct_00b67700(&table);
    host.lua_object_destruct_00b67700(&globals);
    return entity_class;
}

// The class-registry teardown shared by 004B1540 and 004B3E60: every node's
// class [node+14h] gets vtable +0(1) and is cleared (container and node checks
// against a freshly fetched registry; `increment` advances the {container,
// node} iterator), then `destroy_subtree(map, root)` and the head reset.
template <class Registry, class Increment, class DestroySubtree>
void class_registry_destroy(Left8Host& host, Registry registry, Increment increment, DestroySubtree destroy_subtree) {
    Object map = registry() + 0x4;
    std::uint32_t iterator[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(map)),
                                 static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(at<Object>(at<Object>(map, 0x4), 0x0)))};
    for (;;) {
        Object current_map = registry() + 0x4;
        Object head = at<Object>(current_map, 0x4);
        Object container = obj(iterator[0]);
        Object node = obj(iterator[1]);
        if (container == nullptr || container != current_map) host.invalid_parameter_00bf6713();
        if (node == head) break;
        if (container == nullptr) host.invalid_parameter_00bf6713();
        if (node == at<Object>(container, 0x4)) host.invalid_parameter_00bf6713();
        if (Object value = at<Object>(node, 0x14); value != nullptr) {
            host.vcall_deleting_dtor_00(value, 1);
            at<Object>(node, 0x14) = nullptr;
        }
        increment(reinterpret_cast<Object>(iterator));
    }
    Object registry_map = registry() + 0x4;
    destroy_subtree(registry_map, at<Object>(at<Object>(registry_map, 0x4), 0x4));
    at<Object>(at<Object>(registry_map, 0x4), 0x4) = at<Object>(registry_map, 0x4);
    at<std::uint32_t>(registry_map, 0x8) = 0;
    at<Object>(at<Object>(registry_map, 0x4), 0x0) = at<Object>(registry_map, 0x4);
    at<Object>(at<Object>(registry_map, 0x4), 0x8) = at<Object>(registry_map, 0x4);
}
}  // namespace

// 004B3D20 __fastcall(ECX = const NativeString* name), RET, EAX = class: the
// WreckClass get-or-load (registry 004B3C50, lookup 004B39B0, table
// "WreckClass" at 00CE7350). The class: operator new(54h) -> 00489E60 (named
// class base), vtable 00CE72CC, byte +28h = 0, +34h = +38h = 0 (0 on failure).
// No activation call here (004B3F10 adds it).
Object wreck_class_get_or_create_004b3d20(Left8Host& host, const void* name) {
    Object* slot = host.wreck_registry_lookup_004b39b0(host.wreck_registry_004b3c50() + 0x4, name);
    if (*slot != nullptr) return *slot;
    return class_registry_load(
        host, name, "WreckClass", [&]() { return host.wreck_registry_004b3c50(); },
        [&](Object map, const void* key) { return host.wreck_registry_lookup_004b39b0(map, key); },
        [&]() {
            Object memory = host.operator_new_00bf681b(0x54);
            if (memory == nullptr) return Object{nullptr};
            host.named_class_construct_base_00489e60(memory);
            at<std::uint32_t>(memory, 0x0) = 0x00ce72cc;
            at<std::uint8_t>(memory, 0x28) = 0;
            at<std::uint32_t>(memory, 0x34) = 0;
            at<std::uint32_t>(memory, 0x38) = 0;
            return memory;
        });
}

// 004B3E60 cdecl(), RET: the wreck class registry teardown (registry 004B3C50,
// iterator increment 004B1B70, subtree free 004B2DD0), the shape of 004B1540.
void wreck_class_registry_clear_004b3e60(Left8Host& host) {
    class_registry_destroy(
        host, [&]() { return host.wreck_registry_004b3c50(); },
        [&](Object iterator) { host.wreck_registry_iterator_increment_004b1b70(iterator); },
        [&](Object map, Object root) { host.wreck_registry_destroy_subtree_004b2dd0(map, root); });
}

// 004B2030 __thiscall(class, source), RET 4, EAX = wreck: WreckClass
// CreateInstance. operator new(19Ch) (00BF55BE) and _memset before the null
// test; on success 00489F30, vtables 00CE71A8 / 00CE7190 (+10h) / 00CE7188
// (+24h), +18Ch = +190h = 0, class id +C4h = 3Bh. 0047B9E0(wreck, class),
// +188h = class, 00926BE0(wreck). With a source entity: m = a copy of its
// matrix +74h; owner = [source+3Ch]. Without an owner: the source pose is
// refreshed (00414DB0) when +C8h is clear; the world position +FCh..+104h is
// handed to 009039D0(ECX = [source+30h], &pos), which returns the containing
// object; when its +10Ch is clear it is refreshed (00414DB0), 00B63D50(ECX =
// obj+110h, EDX = obj+CCh) builds its inverse and +10Ch = 1; 00439820(ECX =
// &local, EDX = &pos, obj+110h, 0) gives the local position written into m's
// translation, and owner = that object. Then wreck vtable +98h(owner,
// [source+30h], &m) and +C0h = 0.
Object wreck_class_create_instance_004b2030(Left8Host& host, Object wreck_class, Object source) {
    Object p = host.operator_new_00bf55be(0x19c);
    std::memset(p, 0, 0x19c);
    if (p != nullptr) {
        host.class_entity_construct_base_00489f30(p);
        at<std::uint32_t>(p, 0x0) = 0x00ce71a8;
        at<std::uint32_t>(p, 0x10) = 0x00ce7190;
        at<std::uint32_t>(p, 0x24) = 0x00ce7188;
        at<std::uint32_t>(p, 0x18c) = 0;
        at<std::uint32_t>(p, 0x190) = 0;
        at<std::uint32_t>(p, 0xc4) = 0x3b;
    }
    host.class_entity_set_class_0047b9e0(p, wreck_class);
    at<Object>(p, 0x188) = wreck_class;
    host.mission_entity_register_00926be0(p);
    if (source == nullptr) return p;
    float m[16];
    std::memcpy(m, source + 0x74, sizeof(m));
    Object owner = at<Object>(source, 0x3c);
    if (owner == nullptr) {
        if (at<std::uint8_t>(source, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(source);
        const float pos[3] = {at<float>(source, 0xfc), at<float>(source, 0x100), at<float>(source, 0x104)};
        Object container = host.world_find_container_009039d0(at<Object>(source, 0x30), pos);
        if (at<std::uint8_t>(container, 0x10c) == 0) {
            host.entity_pose_refresh_world_00414db0(container);
            host.matrix_inverse_00b63d50(container + 0x110, container + 0xcc);
            at<std::uint8_t>(container, 0x10c) = 1;
        }
        float local[3];
        const float* l = host.matrix_transform_point_00439820(local, pos, container + 0x110, 0);
        m[12] = l[0];
        m[13] = l[1];
        m[14] = l[2];
        owner = container;
    }
    host.vcall_attach_98(p, owner, at<Object>(source, 0x30), reinterpret_cast<Object>(m));
    at<std::uint32_t>(p, 0xc0) = 0;
    return p;
}

namespace {
// The intrusive handle release: InterlockedDecrement([h+4]) ([00CE2220]) and
// vtable slot 0 with no arguments when it reaches zero.
void release_handle(Left8Host& host, Object handle) {
    if (handle != nullptr && host.interlocked_decrement_ce2220(reinterpret_cast<long*>(handle + 4)) == 0) {
        host.object_slot_00(handle);
    }
}

// The handle assignment: when the field differs, it takes the new handle
// (InterlockedIncrement [00CE221C] when non-null) and the old one is released.
void assign_handle(Left8Host& host, Object& field, Object handle) {
    Object old = field;
    if (old == handle) return;
    field = handle;
    if (handle != nullptr) host.interlocked_increment_ce221c(reinterpret_cast<long*>(handle + 4));
    release_handle(host, old);
}

float lua_number_field(Left8Host& host, LuaObject* table, const char* name) {
    LuaObject temp{};
    const float value = static_cast<float>(host.lua_get_number_00b66270(host.lua_get_by_name_00b67800(table, &temp, name)));
    host.lua_object_destruct_00b67700(&temp);
    return value;
}

// One wreck emitter section (Fire / Smoke): Chance -> chance, EmitterEfx ->
// the handle 00870CD0(&temp, 00B66290(obj), 1) assigned to the field and the
// temporary released, TimeMin / TimeMax -> the two times.
void wreck_emitter_section(Left8Host& host, LuaObject* section, float& chance, Object& effect, float& time_min,
                           float& time_max) {
    chance = lua_number_field(host, section, "Chance");
    LuaObject temp{};
    LuaObject* efx = host.lua_get_by_name_00b67800(section, &temp, "EmitterEfx");
    Object handle = nullptr;
    const std::int32_t id = host.lua_get_integer_00b66290(efx);
    assign_handle(host, effect, *host.effect_handle_acquire_00870cd0(&handle, id, 1));
    release_handle(host, handle);
    host.lua_object_destruct_00b67700(&temp);
    time_min = lua_number_field(host, section, "TimeMin");
    time_max = lua_number_field(host, section, "TimeMax");
}
}  // namespace

// 004B21E0 __thiscall(class, LuaObject* table), RET 4: WreckClass
// ReadLuaFields. 0048F670(class, table); table.Fire, when not nil: +2Ch =
// Chance, +34h = the EmitterEfx handle, +3Ch / +40h = TimeMin / TimeMax;
// otherwise +2Ch = 0. table.Smoke likewise into +30h, +38h, +44h, +48h (+30h =
// 0 when nil). table.Remain: +4Ch / +50h = TimeMin / TimeMax. Numbers are
// 00B66270 stored as float. The image keeps the temporary handle in the
// table argument's stack slot; Remain, Smoke and Fire are destructed last.
void wreck_class_read_lua_fields_004b21e0(Left8Host& host, Object wreck_class, LuaObject* table) {
    host.named_class_read_lua_0048f670(wreck_class, table);
    LuaObject fire{};
    host.lua_get_by_name_00b67800(table, &fire, "Fire");
    if (!host.lua_is_nil_00b65fb0(&fire)) {
        wreck_emitter_section(host, &fire, at<float>(wreck_class, 0x2c), at<Object>(wreck_class, 0x34),
                              at<float>(wreck_class, 0x3c), at<float>(wreck_class, 0x40));
    } else {
        at<std::uint32_t>(wreck_class, 0x2c) = 0;
    }
    LuaObject smoke{};
    host.lua_get_by_name_00b67800(table, &smoke, "Smoke");
    if (!host.lua_is_nil_00b65fb0(&smoke)) {
        wreck_emitter_section(host, &smoke, at<float>(wreck_class, 0x30), at<Object>(wreck_class, 0x38),
                              at<float>(wreck_class, 0x44), at<float>(wreck_class, 0x48));
    } else {
        at<std::uint32_t>(wreck_class, 0x30) = 0;
    }
    LuaObject remain{};
    host.lua_get_by_name_00b67800(table, &remain, "Remain");
    at<float>(wreck_class, 0x4c) = lua_number_field(host, &remain, "TimeMin");
    at<float>(wreck_class, 0x50) = lua_number_field(host, &remain, "TimeMax");
    host.lua_object_destruct_00b67700(&remain);
    host.lua_object_destruct_00b67700(&smoke);
    host.lua_object_destruct_00b67700(&fire);
}

namespace {
TaggedValue tagged(std::uint32_t tag, const void* p) {
    return TaggedValue{tag, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p))};
}

// One wreck effect: the handle [class+handle_offset] is passed by value with a
// reference taken (InterlockedIncrement) to 008687C0(ECX = &temp, EDX = node,
// handle, &identity, 1, 0, 0); the result is assigned to the field and the
// temporary released.
void wreck_create_effect(Left8Host& host, Object wreck, std::size_t handle_offset, std::size_t field_offset) {
    const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
    Object node = at<Object>(at<Object>(at<Object>(wreck, 0x178), 0x160), 0xc);
    Object handle = at<Object>(at<Object>(wreck, 0x188), handle_offset);
    if (handle != nullptr) host.interlocked_increment_ce221c(reinterpret_cast<long*>(handle + 4));
    Object temp = nullptr;
    Object* created = host.point_effect_create_with_parent_matrix_008687c0(&temp, node, handle, identity, 1, 0, 0);
    assign_handle(host, at<Object>(wreck, field_offset), *created);
    release_handle(host, temp);
}
}  // namespace

// 004B3480 __fastcall(wreck), RET: Wreck SEntityInit. 0048DBD0(wreck); then
// by the spawn record [+C0h]:
//   none: r = U(stream 0, 0, 1); phase +194h = 2 when Fire.Chance [class+2Ch]
//     > r, else 1 when Chance + Smoke.Chance [+30h] > r, else 0 (FCOMI, an
//     unordered compare falls through to the lower phase); +198h = U(stream 0,
//     v, v) with v = the phase's TimeMax (+40h / +48h / +50h; both bounds v).
//   type 1 (scene properties, bag [+C0h]+8 searched with 008F2260): phase = 2
//     when "Fire" is set, else ("Smoke" set); +198h = "Duration" (int or float
//     by its tag), replaced by 1.0e10f (00CE4970) when 0 >= it (COMISS).
//   type 3 (save data): 009238A0(wreck, &data) and the reader 004425C0 over it,
//     key "_wreck" (00BD8E20), "state" -> int +194h and "timeLeft" -> float
//     +198h (00BD6830), 00BD7A20 leaves the key, 00441A20 destroys the reader.
// 004B2760 places it, vtable +D8h(); the effects +190h and +18Ch are released
// and cleared; phase >= 2 creates the Fire effect into +18Ch (handle
// [class+34h]) and phase > 0 the Smoke effect into +190h (handle [class+38h]).
void wreck_sentity_init_004b3480(Left8Host& host, Object wreck) {
    host.class_entity37_sentity_init_base_0048dbd0(wreck);
    Object record = at<Object>(wreck, 0xc0);
    if (record == nullptr) {
        const float r = host.random_uniform_00bd2f10(0, 0.0f, kOne);
        Object wreck_class = at<Object>(wreck, 0x188);
        const float fire = at<float>(wreck_class, 0x2c);
        float v;
        if (fire > r) {
            at<std::int32_t>(wreck, 0x194) = 2;
            v = at<float>(wreck_class, 0x40);
        } else if (static_cast<double>(fire) + at<float>(wreck_class, 0x30) > r) {
            at<std::int32_t>(wreck, 0x194) = 1;
            v = at<float>(wreck_class, 0x48);
        } else {
            at<std::int32_t>(wreck, 0x194) = 0;
            v = at<float>(wreck_class, 0x50);
        }
        at<float>(wreck, 0x198) = host.random_uniform_00bd2f10(0, v, v);
    } else if (at<std::int32_t>(record, 0x4) == 1) {
        if (at<std::uint8_t>(host.scene_property_bag_find_008f2260(at<Object>(record, 0x8), "Fire"), 0xc) != 0) {
            at<std::int32_t>(wreck, 0x194) = 2;
        } else {
            Object smoke = host.scene_property_bag_find_008f2260(at<Object>(at<Object>(wreck, 0xc0), 0x8), "Smoke");
            at<std::int32_t>(wreck, 0x194) = at<std::uint8_t>(smoke, 0xc) != 0 ? 1 : 0;
        }
        Object duration = host.scene_property_bag_find_008f2260(at<Object>(at<Object>(wreck, 0xc0), 0x8), "Duration");
        const float value = at<std::uint32_t>(duration, 0x4) == 0 ? static_cast<float>(at<std::int32_t>(duration, 0xc))
                                                                  : at<float>(duration, 0xc);
        at<float>(wreck, 0x198) = value;
        if (!(0.0f < value)) at<float>(wreck, 0x198) = host.global_f32(0x00ce4970);
    } else if (at<std::int32_t>(record, 0x4) == 3) {
        LuaObject data{};
        host.sentity_get_saved_lua_data_009238a0(wreck, &data);
        LuaReader reader_storage{};
        LuaReader* reader = &reader_storage;
        host.lua_reader_construct_004425c0(reader, &data);
        host.lua_reader_enter_key_00bd8e20(reader, tagged(0, "_wreck"));
        host.lua_reader_read_field_00bd6830(reader, tagged(0, "state"), tagged(1, wreck + 0x194));
        host.lua_reader_read_field_00bd6830(reader, tagged(0, "timeLeft"), tagged(2, wreck + 0x198));
        host.lua_reader_leave_key_00bd7a20(reader);
        host.lua_reader_destroy_00441a20(reader);
    }
    wreck_place_on_surface_004b2760(host, wreck);
    host.vcall_d8(wreck);
    if (Object smoke = at<Object>(wreck, 0x190); smoke != nullptr) {
        release_handle(host, smoke);
        at<Object>(wreck, 0x190) = nullptr;
    }
    at<Object>(wreck, 0x190) = nullptr;
    if (Object fire = at<Object>(wreck, 0x18c); fire != nullptr) {
        at<Object>(wreck, 0x18c) = nullptr;
        release_handle(host, fire);
    }
    if (at<std::int32_t>(wreck, 0x194) >= 2) wreck_create_effect(host, wreck, 0x34, 0x18c);
    if (at<std::int32_t>(wreck, 0x194) > 0) wreck_create_effect(host, wreck, 0x38, 0x190);
}

// 004AEA00 __fastcall(registry), RET: the ship part class registry destructor
// (vtable 00CE6F6C). The class teardown of 004B1540 over the map at +4 (with
// the registry itself as the container; increment 004ABDA0, subtree free
// 004ADB80, head reset), then 004AD2A0() deletes the objects of the global
// vector at 00E187DC, 004AE3E0 erases [head->left, head) (now empty), the head
// node is freed and +8 / +0Ch cleared, [00E187E8] = 0 and the vtable becomes the
// singleton base 00CE3818.
void ship_part_class_registry_destruct_004aea00(Left8Host& host, Object registry) {
    at<std::uint32_t>(registry, 0x0) = 0x00ce6f6c;
    class_registry_destroy(
        host, [&]() { return registry; },
        [&](Object iterator) { host.ship_part_registry_iterator_increment_004abda0(iterator); },
        [&](Object map, Object root) { host.ship_part_registry_destroy_subtree_004adb80(map, root); });
    host.ship_part_objects_delete_all_004ad2a0();
    Object map = registry + 0x4;
    destroy_map(host, map, [&](Object m, Object r, Object c0, Object f, Object c1, Object l) {
        host.tree_erase_range_004ae3e0(m, r, c0, f, c1, l);
    });
    host.global_set_u32(0x00e187e8, 0);
    at<std::uint32_t>(registry, 0x0) = 0x00ce3818;
}

namespace {
// globals[name][index] as a float, each step a fresh globals object
// (00B67980 on [00E188A8]+1A0Ch), 00B67800, 00B67720(ECX = table, out, index),
// 00B66270; the three temporaries are destructed innermost first.
float lua_global_array_number(Left8Host& host, const char* name, std::int32_t index) {
    LuaObject globals{};
    LuaObject table{};
    LuaObject element{};
    host.lua_get_globals_00b67980(game(host) + 0x1a0c, &globals);
    host.lua_get_by_name_00b67800(&globals, &table, name);
    const float value = static_cast<float>(
        host.lua_get_number_00b66270(host.lua_get_by_index_00b67720(&table, &element, index)));
    host.lua_object_destruct_00b67700(&element);
    host.lua_object_destruct_00b67700(&table);
    host.lua_object_destruct_00b67700(&globals);
    return value;
}
}  // namespace

namespace {
// The std::vector<Object> push_back as the image inlines it: with room, store
// and advance; otherwise the first <= last check and the typed insert
// `insert(vector, &result, vector, last, &value)`.
template <class Insert>
void push_back_pointer(Left8Host& host, Object vector, Object value, Insert insert) {
    const std::uint32_t first = at<std::uint32_t>(vector, 0x4);
    const std::uint32_t size = first == 0 ? 0 : (at<std::uint32_t>(vector, 0x8) - first) >> 2;
    if (first != 0 && size < ((at<std::uint32_t>(vector, 0xc) - first) >> 2)) {
        at<Object>(obj(at<std::uint32_t>(vector, 0x8)), 0) = value;
        at<std::uint32_t>(vector, 0x8) += 4;
        return;
    }
    const std::uint32_t last = at<std::uint32_t>(vector, 0x8);
    if (first > last) host.invalid_parameter_00bf6713();
    std::uint32_t result[2] = {0, 0};
    insert(vector, reinterpret_cast<Object>(result), vector, obj(last), &value);
}
}  // namespace

// 004AED50 (ECX = index, DL = force), RET, EAX = record: ShipExplosion
// ReadPartsTable. The global record vector (first [00E187DC], last [00E187E0],
// cached) is searched for a record whose +4 equals the index; a non-null hit is
// returned. Otherwise t = globals.ShipExplosion[sprintf("%d", index)]; when t
// is nil and force is clear, 0 is returned. A record = operator new(24h)
// (vtable 00CE6F68, +18h..+20h = 0, +4 = -1, +10h = 0; null on failure gets
// written through anyway) with +4 = index, +8 = PartsNum[1] and +0Ch =
// PartsNum[2] (00B66290). For k = 0..98, when t["Part%d"] is a table
// (00B661B0), it is assigned to a local (00B67690) and a part = operator
// new(0Ch) {vtable 00CE6DD4, +4 = +8 = 0} gets +8 = max(Weight, 0) and +4 =
// 004AEC00(the Class string as a NativeString) (the ship part class); the part
// is pushed onto the record's vector at +14h (insert 004AE230) and +10h +=
// part+8. The record is pushed onto the global vector 00E187D8 (insert
// 004AE1A0) and returned. The sprintf buffers are 8 bytes in the image; this
// code bounds them (MINOR: only an index of 8+ digits differs).
Object ship_explosion_read_parts_table_004aed50(Left8Host& host, std::int32_t index, bool force) {
    const std::uint32_t list_first = host.global_u32(0x00e187dc);
    const std::uint32_t list_last = host.global_u32(0x00e187e0);
    for (std::uint32_t i = 0; list_first != 0 && i < ((list_last - list_first) >> 2); ++i) {
        if (at<std::int32_t>(at<Object*>(obj(list_first), 0)[i], 0x4) != index) continue;
        if (list_first == 0 || i >= ((list_last - list_first) >> 2)) host.invalid_parameter_00bf6713();
        Object hit = at<Object*>(obj(host.global_u32(0x00e187dc)), 0)[i];
        if (hit != nullptr) return hit;
        break;
    }
    LuaObject globals{};
    LuaObject explosion{};
    LuaObject table{};
    host.lua_get_globals_00b67980(game(host) + 0x1a0c, &globals);
    host.lua_get_by_name_00b67800(&globals, &explosion, "ShipExplosion");
    char key[8];
    std::snprintf(key, sizeof(key), "%d", index);
    host.lua_get_by_name_00b67800(&explosion, &table, key);
    if (host.lua_is_nil_00b65fb0(&table) && !force) {
        host.lua_object_destruct_00b67700(&table);
        host.lua_object_destruct_00b67700(&explosion);
        host.lua_object_destruct_00b67700(&globals);
        return nullptr;
    }
    Object record = host.operator_new_00bf681b(0x24);
    if (record != nullptr) {
        at<std::uint32_t>(record, 0x0) = 0x00ce6f68;
        at<std::uint32_t>(record, 0x18) = 0;
        at<std::uint32_t>(record, 0x1c) = 0;
        at<std::uint32_t>(record, 0x20) = 0;
        at<std::int32_t>(record, 0x4) = -1;
        at<std::uint32_t>(record, 0x10) = 0;
    }
    at<std::int32_t>(record, 0x4) = index;
    for (std::int32_t slot = 1; slot <= 2; ++slot) {
        LuaObject parts{};
        LuaObject element{};
        host.lua_get_by_name_00b67800(&table, &parts, "PartsNum");
        at<std::int32_t>(record, slot == 1 ? 0x8 : 0xc) =
            host.lua_get_integer_00b66290(host.lua_get_by_index_00b67720(&parts, &element, slot));
        host.lua_object_destruct_00b67700(&element);
        host.lua_object_destruct_00b67700(&parts);
    }
    LuaObject part_table{};
    host.lua_object_construct_00b65f50(&part_table);
    for (std::int32_t k = 0; k < 0x63; ++k) {
        char name[8];
        std::snprintf(name, sizeof(name), "Part%d", k);
        LuaObject temp{};
        const bool is_table = host.lua_is_table_00b661b0(host.lua_get_by_name_00b67800(&table, &temp, name));
        host.lua_object_destruct_00b67700(&temp);
        if (!is_table) continue;
        host.lua_object_assign_00b67690(&part_table, host.lua_get_by_name_00b67800(&table, &temp, name));
        host.lua_object_destruct_00b67700(&temp);
        Object part = host.operator_new_00bf681b(0xc);
        if (part != nullptr) {
            at<std::uint32_t>(part, 0x0) = 0x00ce6dd4;
            at<std::uint32_t>(part, 0x4) = 0;
            at<std::uint32_t>(part, 0x8) = 0;
        }
        const std::int32_t weight =
            host.lua_get_integer_00b66290(host.lua_get_by_name_00b67800(&part_table, &temp, "Weight"));
        at<std::int32_t>(part, 0x8) = weight < 0 ? 0 : weight;
        host.lua_object_destruct_00b67700(&temp);
        NativeString class_name;
        native_string_from_text(host, class_name,
                                host.lua_get_string_00b662b0(host.lua_get_by_name_00b67800(&part_table, &temp, "Class")));
        at<Object>(part, 0x4) = host.ship_part_class_get_or_load_004aec00(&class_name);
        release_native_string(host, class_name);
        host.lua_object_destruct_00b67700(&temp);
        push_back_pointer(host, record + 0x14, part, [&](Object v, Object r, Object c, Object where, const Object* value) {
            host.pointer_vector_insert_004ae230(v, r, c, where, value);
        });
        at<std::int32_t>(record, 0x10) += at<std::int32_t>(part, 0x8);
    }
    host.lua_object_destruct_00b67700(&part_table);
    push_back_pointer(host, host.image_address(0x00e187d8), record, [&](Object v, Object r, Object c, Object where, const Object* value) {
        host.pointer_vector_insert_004ae1a0(v, r, c, where, value);
    });
    host.lua_object_destruct_00b67700(&table);
    host.lua_object_destruct_00b67700(&explosion);
    host.lua_object_destruct_00b67700(&globals);
    return record;
}

namespace {
// table[name][index] as a float: 00B67800, 00B67720, 00B66270; the element is
// destructed before the named object.
float lua_field_index_number(Left8Host& host, LuaObject* table, const char* name, std::int32_t index) {
    LuaObject field{};
    LuaObject element{};
    host.lua_get_by_name_00b67800(table, &field, name);
    const float value =
        static_cast<float>(host.lua_get_number_00b66270(host.lua_get_by_index_00b67720(&field, &element, index)));
    host.lua_object_destruct_00b67700(&element);
    host.lua_object_destruct_00b67700(&field);
    return value;
}

// An effect handle field read with a default: id = 00B66380(table[name], 0),
// 00870CD0(&temp, id, 0), assigned to the field, the temporary released, then
// the named object destructed.
void lua_effect_field(Left8Host& host, LuaObject* table, const char* name, Object& field) {
    LuaObject object{};
    LuaObject* named = host.lua_get_by_name_00b67800(table, &object, name);
    const std::int32_t id = host.lua_get_integer_or_default_00b66380(named, 0);
    Object temp = nullptr;
    assign_handle(host, field, *host.effect_handle_acquire_00870cd0(&temp, id, 0));
    release_handle(host, temp);
    host.lua_object_destruct_00b67700(&object);
}
}  // namespace

// 004ACAB0 __thiscall(class, LuaObject* table), RET 4: ShipPartClass
// ReadLuaFields. 0048F670(class, table); +28h / +2Ch = StartVelocity[1] / [2],
// +30h / +34h = StartVelocityAngle[1] / [2], +38h / +3Ch = AirDrag[1] / [2],
// +50h = GravityMul (00B66330 with default 1.0f), +40h / +44h =
// RotationSpeed[1] / [2], +48h / +4Ch = RotationDecline[1] / [2]; the handles
// +54h = WaterSplashEfx and +58h = SmokeEfx (00B66380 default 0, 00870CD0 flag 0).
void ship_part_class_read_lua_fields_004acab0(Left8Host& host, Object part_class, LuaObject* table) {
    host.named_class_read_lua_0048f670(part_class, table);
    at<float>(part_class, 0x28) = lua_field_index_number(host, table, "StartVelocity", 1);
    at<float>(part_class, 0x2c) = lua_field_index_number(host, table, "StartVelocity", 2);
    at<float>(part_class, 0x30) = lua_field_index_number(host, table, "StartVelocityAngle", 1);
    at<float>(part_class, 0x34) = lua_field_index_number(host, table, "StartVelocityAngle", 2);
    at<float>(part_class, 0x38) = lua_field_index_number(host, table, "AirDrag", 1);
    at<float>(part_class, 0x3c) = lua_field_index_number(host, table, "AirDrag", 2);
    {
        LuaObject gravity{};
        at<float>(part_class, 0x50) = static_cast<float>(
            host.lua_get_float_or_default_00b66330(host.lua_get_by_name_00b67800(table, &gravity, "GravityMul"), kOne));
        host.lua_object_destruct_00b67700(&gravity);
    }
    at<float>(part_class, 0x40) = lua_field_index_number(host, table, "RotationSpeed", 1);
    at<float>(part_class, 0x44) = lua_field_index_number(host, table, "RotationSpeed", 2);
    at<float>(part_class, 0x48) = lua_field_index_number(host, table, "RotationDecline", 1);
    at<float>(part_class, 0x4c) = lua_field_index_number(host, table, "RotationDecline", 2);
    lua_effect_field(host, table, "WaterSplashEfx", at<Object>(part_class, 0x54));
    lua_effect_field(host, table, "SmokeEfx", at<Object>(part_class, 0x58));
}

// 004A7D30 __thiscall(class, LuaObject* table), RET 4: PlanePartClass
// ReadLuaFields. 0048F670(class, table); +28h / +2Ch = TimeToLive[1] / [2],
// +30h / +34h = StartVelocityAdd, +38h / +3Ch = StartVelocityDirAdd, +40h /
// +44h = AirDrag, +58h = GravityMul (00B66330, default 1.0 from FLD1), +48h /
// +4Ch = RotationSpeed, +50h / +54h = RotationDecline; the handles +5Ch =
// WaterSplashEfx and +60h = SmokeEfx (00B66380 default 0, 00870CD0 flag 0).
void plane_part_class_read_lua_fields_004a7d30(Left8Host& host, Object part_class, LuaObject* table) {
    host.named_class_read_lua_0048f670(part_class, table);
    at<float>(part_class, 0x28) = lua_field_index_number(host, table, "TimeToLive", 1);
    at<float>(part_class, 0x2c) = lua_field_index_number(host, table, "TimeToLive", 2);
    at<float>(part_class, 0x30) = lua_field_index_number(host, table, "StartVelocityAdd", 1);
    at<float>(part_class, 0x34) = lua_field_index_number(host, table, "StartVelocityAdd", 2);
    at<float>(part_class, 0x38) = lua_field_index_number(host, table, "StartVelocityDirAdd", 1);
    at<float>(part_class, 0x3c) = lua_field_index_number(host, table, "StartVelocityDirAdd", 2);
    at<float>(part_class, 0x40) = lua_field_index_number(host, table, "AirDrag", 1);
    at<float>(part_class, 0x44) = lua_field_index_number(host, table, "AirDrag", 2);
    {
        LuaObject gravity{};
        at<float>(part_class, 0x58) = static_cast<float>(
            host.lua_get_float_or_default_00b66330(host.lua_get_by_name_00b67800(table, &gravity, "GravityMul"), kOne));
        host.lua_object_destruct_00b67700(&gravity);
    }
    at<float>(part_class, 0x48) = lua_field_index_number(host, table, "RotationSpeed", 1);
    at<float>(part_class, 0x4c) = lua_field_index_number(host, table, "RotationSpeed", 2);
    at<float>(part_class, 0x50) = lua_field_index_number(host, table, "RotationDecline", 1);
    at<float>(part_class, 0x54) = lua_field_index_number(host, table, "RotationDecline", 2);
    lua_effect_field(host, table, "WaterSplashEfx", at<Object>(part_class, 0x5c));
    lua_effect_field(host, table, "SmokeEfx", at<Object>(part_class, 0x60));
}

// 004AF290 cdecl(), RET: ShipExplosion LoadGlobals. [00E187D4] =
// DeviceExplosionChanceDistRange[1], [00E187D0] = ...DistRange[2], [00E187CC] =
// DeviceExplosionChanceDistMul[1], [00E187C8] = ...DistMul[2] (floats); then
// 004AED50(ECX = i, DL = 0) for i = 0, 1, ... until it returns 0.
void ship_explosion_load_globals_004af290(Left8Host& host) {
    host.global_set_f32(0x00e187d4, lua_global_array_number(host, "DeviceExplosionChanceDistRange", 1));
    host.global_set_f32(0x00e187d0, lua_global_array_number(host, "DeviceExplosionChanceDistRange", 2));
    host.global_set_f32(0x00e187cc, lua_global_array_number(host, "DeviceExplosionChanceDistMul", 1));
    host.global_set_f32(0x00e187c8, lua_global_array_number(host, "DeviceExplosionChanceDistMul", 2));
    for (std::int32_t i = 0;; ++i) {
        if (ship_explosion_read_parts_table_004aed50(host, i, false) == nullptr) break;
    }
}

// 004B0DE0 __thiscall(class, LuaObject* table), RET 4 (soldier class vtable
// 00CE7150 slot +4). 0048F670(class, table) reads the named-class base; then
// for table.LoopLengths (00B67800, "LoopLengths" at 00CE7144) when not nil:
// key and value objects are constructed (00B65F50), 00B67080(ECX = &loop,
// &key, &value) starts the iteration and, until 00B66420(&loop, &key) reports
// the end, the key string becomes a NativeString (0041DD40 + memcpy), the
// class's string->float map at +28h gets slot = 00444BE0(map, &string) and
// *slot = (float)00B66270(&value); the string returns to the pool and
// 00B67190(&loop, &key, &value) advances. value, key and loop are destructed.
void soldier_class_read_lua_004b0de0(Left8Host& host, Object soldier_class, LuaObject* table) {
    host.named_class_read_lua_0048f670(soldier_class, table);
    LuaObject loop{};
    host.lua_get_by_name_00b67800(table, &loop, "LoopLengths");
    if (!host.lua_is_nil_00b65fb0(&loop)) {
        LuaObject key{};
        LuaObject value{};
        host.lua_object_construct_00b65f50(&key);
        host.lua_object_construct_00b65f50(&value);
        host.lua_iterate_first_00b67080(&loop, &key, &value);
        while (!host.lua_iterate_done_00b66420(&loop, &key)) {
            NativeString name;
            native_string_from_text(host, name, host.lua_get_string_00b662b0(&key));
            const NativeString captured = name;
            float* slot = host.string_float_map_lookup_00444be0(soldier_class + 0x28, &name);
            *slot = static_cast<float>(host.lua_get_number_00b66270(&value));
            if (captured.data != nullptr) {
                host.string_pool_return_00bd1510(host.string_pool_00419cc0(), captured.data, captured.length + 1, 1);
            }
            host.lua_iterate_next_00b67190(&loop, &key, &value);
        }
        host.lua_object_destruct_00b67700(&value);
        host.lua_object_destruct_00b67700(&key);
    }
    host.lua_object_destruct_00b67700(&loop);
}

// 004B1400 __fastcall(ECX = const NativeString* name), RET, EAX = class: the
// SoldierClass get-or-load. slot = 004B0CB0(ECX = 004B1330()+4, name). When
// empty: globals 00B67980(ECX = [00E188A8]+1A0Ch), table = globals.SoldierClass
// (00B67800), row = 00B68100(ECX = &table, &row, name); operator new(34h) ->
// 004B12A0(class) (0 on failure); unless class+4 is the name itself, 0041DD40
// resizes class+4 to the name length and a non-empty name is memcpy'd (length
// [class+4] bytes, no terminator copy); class vtable +4(&row) reads it; the
// slot (looked up again) takes it; row, table and globals are destructed.
// Every path ends with class vtable +0Ch() and returns the class.
Object soldier_class_get_or_load_004b1400(Left8Host& host, const void* name) {
    Object soldier_class =
        *host.soldier_registry_get_or_insert_004b0cb0(host.soldier_registry_004b1330() + 0x4, name);
    if (soldier_class == nullptr) {
        soldier_class = class_registry_load(
            host, name, "SoldierClass", [&]() { return host.soldier_registry_004b1330(); },
            [&](Object map, const void* key) { return host.soldier_registry_get_or_insert_004b0cb0(map, key); },
            [&]() {
                Object memory = host.operator_new_00bf681b(0x34);
                return memory != nullptr ? host.soldier_class_construct_004b12a0(memory) : Object{nullptr};
            });
    }
    host.vcall_class_activate_0c(soldier_class);
    return soldier_class;
}

// 004B1540 cdecl(), RET: the soldier registry teardown. For every node of the
// map at 004B1330()+4 (begin = head->left; the singleton is re-fetched for each
// end test; the iterator container is checked): the class [node+14h], when
// set, gets vtable +0(1) and the value is cleared; 004AFB50 advances. Then
// 004B0600(ECX = map, [head+4]) frees the tree and the head is reset (parent,
// left, right = head, size 0).
void soldier_registry_destroy_004b1540(Left8Host& host) {
    class_registry_destroy(
        host, [&]() { return host.soldier_registry_004b1330(); },
        [&](Object iterator) { host.soldier_registry_iterator_increment_004afb50(iterator); },
        [&](Object map, Object root) { host.soldier_registry_destroy_subtree_004b0600(map, root); });
}

// 004AFCA0 __fastcall(soldier), RET by a tail jump to 00484940(soldier). Only
// when +18Ch is set: s = +198h; s == 0 (UCOMISS, ordered equal) -> step =
// [[00E188A8]+21F0h] (the frame time); otherwise step = 0042B2F0(&d) / s with d
// the float deltas +A4h..+ACh minus +19Ch..+1A4h (the distance moved). +1A8h =
// (float)(step + +1A8h). Looping (+1ADh): +1A8h = (float)fmod(+1A8h, +194h)
// (00BF857A); otherwise +1A8h = min(+1A8h, (float)(+194h - 0.001 qword
// 00D7A318)) (FCOMIP; unordered keeps the limit). Node [[+178h]+160h]+0Ch
// vtable +24h(max(+190h, 0), +1A8h, 1); +19Ch..+1A4h = +A4h..+ACh.
void soldier_advance_animation_phase_004afca0(Left8Host& host, Object soldier) {
    if (at<std::uint8_t>(soldier, 0x18c) != 0) {
        const float s = at<float>(soldier, 0x198);
        double step;
        if (s == 0.0f) {
            step = at<float>(game(host), 0x21f0);
        } else {
            const float d[3] = {static_cast<float>(static_cast<double>(at<float>(soldier, 0xa4)) - at<float>(soldier, 0x19c)),
                                static_cast<float>(static_cast<double>(at<float>(soldier, 0xa8)) - at<float>(soldier, 0x1a0)),
                                static_cast<float>(static_cast<double>(at<float>(soldier, 0xac)) - at<float>(soldier, 0x1a4))};
            step = static_cast<double>(host.vector3_length_0042b2f0(d)) / s;
        }
        at<float>(soldier, 0x1a8) = static_cast<float>(step + at<float>(soldier, 0x1a8));
        if (at<std::uint8_t>(soldier, 0x1ad) != 0) {
            at<float>(soldier, 0x1a8) = static_cast<float>(
                host.crt_fmod_00bf857a(at<float>(soldier, 0x1a8), at<float>(soldier, 0x194)));
        } else {
            const float limit = static_cast<float>(static_cast<double>(at<float>(soldier, 0x194)) - 0.0010000000474974513);
            const float v = at<float>(soldier, 0x1a8);
            at<float>(soldier, 0x1a8) = limit > v ? v : limit;
        }
        const std::int32_t id = at<std::int32_t>(soldier, 0x190);
        Object node = at<Object>(at<Object>(at<Object>(soldier, 0x178), 0x160), 0xc);
        host.anim_node_play_24(node, static_cast<std::uint32_t>(id > 0 ? id : 0), at<float>(soldier, 0x1a8), 1);
        at<float>(soldier, 0x19c) = at<float>(soldier, 0xa4);
        at<float>(soldier, 0x1a0) = at<float>(soldier, 0xa8);
        at<float>(soldier, 0x1a4) = at<float>(soldier, 0xac);
    }
    host.scene_entity_refresh_visibility_factor_00484940(soldier);
}

// 004B18B0 __thiscall(wreck, float dt), RET 4. +198h = t = (float)(+198h -
// dt); only when 0 > t (FCOMIP; unordered returns): +194h -= 1; then phase 1
// -> 00867B10(ECX = [+18Ch]) stops that effect and +198h = 00BD2F10(stream 0,
// v, v) with v = [[+188h]+48h] (both bounds the same value); phase 0 ->
// 00867B10([+190h]) and +198h = U(stream 0, w, w), w = [[+188h]+50h]; any other
// phase -> 00926D90(wreck, 2).
void wreck_tick_phases_004b18b0(Left8Host& host, Object wreck, float dt) {
    const float t = static_cast<float>(static_cast<double>(at<float>(wreck, 0x198)) - dt);
    at<float>(wreck, 0x198) = t;
    if (!(0.0f > t)) return;
    at<std::int32_t>(wreck, 0x194) -= 1;
    const std::int32_t phase = at<std::int32_t>(wreck, 0x194);
    if (phase == 1) {
        host.point_effect_stop_children_00867b10(at<Object>(wreck, 0x18c));
        const float v = at<float>(at<Object>(wreck, 0x188), 0x48);
        at<float>(wreck, 0x198) = host.random_uniform_00bd2f10(0, v, v);
        return;
    }
    if (phase == 0) {
        host.point_effect_stop_children_00867b10(at<Object>(wreck, 0x190));
        const float w = at<float>(at<Object>(wreck, 0x188), 0x50);
        at<float>(wreck, 0x198) = host.random_uniform_00bd2f10(0, w, w);
        return;
    }
    host.mission_entity_kill_00926d90(wreck, 2);
}

// 004B2760 __fastcall(wreck), RET (a tail jump to 0042ED50 on the placed path).
// When the owner [+3Ch] is set and its vtable +5Ch(44h) (kind test) is true:
// +A8h = (float)(0087FA20(ECX = owner, x = +A4h, z = +ACh) - [owner+A8h]);
// 0087FB90(ECX = owner, &n, x, z) gives the surface normal copied into the
// matrix row +84h..+8Ch; 0085DAD0(ECX = wreck+74h) re-orthonormalises; then
// 0042ED50(wreck).
void wreck_place_on_surface_004b2760(Left8Host& host, Object wreck) {
    Object owner = at<Object>(wreck, 0x3c);
    if (owner == nullptr || !host.vcall_is_kind_of_5c(owner, 0x44)) return;
    owner = at<Object>(wreck, 0x3c);
    const float h = host.landscape_forward_local_height_0087fa20(owner, at<float>(wreck, 0xa4), at<float>(wreck, 0xac));
    at<float>(wreck, 0xa8) = static_cast<float>(static_cast<double>(h) - at<float>(owner, 0xa8));
    float n[3] = {0.0f, 0.0f, 0.0f};
    const float* normal =
        host.landscape_forward_local_normal_0087fb90(owner, n, at<float>(wreck, 0xa4), at<float>(wreck, 0xac));
    at<float>(wreck, 0x84) = normal[0];
    at<float>(wreck, 0x88) = normal[1];
    at<float>(wreck, 0x8c) = normal[2];
    host.matrix_orthonormalize_0085dad0(wreck + 0x74);
    host.scene_node_invalidate_subtree_pose_0042ed50(wreck);
}

// 004B0F80 __thiscall(soldier, const anim-name*, bool random_start, bool force),
// RET 0Ch. Only when +18Ch is set: id = 00B78670(anim set, name) with the set
// [[[[+178h]+160h]+0Ch]+130h]+2Ch. When id differs from +190h or force: +190h =
// id; +194h = 00B6DC90(node = [[+178h]+160h]+0Ch, id) (duration); +198h =
// *00444BE0([+188h]+28h, name); start = U(0, duration) through 00BD2F10 on
// stream 0 (ECX = 0) when random_start and duration > 0 (COMISS, so unordered
// gives 0), else 0; +1A8h = start; node vtable +24h(id, start, 1); +19Ch..+1A4h
// = +A4h..+ACh. Then (still under +18Ch) +1ADh = random_start.
void soldier_start_animation_004b0f80(Left8Host& host, Object soldier, const void* anim_name, bool random_start,
                                      bool force) {
    if (at<std::uint8_t>(soldier, 0x18c) == 0) return;
    Object anim_set = at<Object>(at<Object>(at<Object>(at<Object>(at<Object>(soldier, 0x178), 0x160), 0xc), 0x130), 0x2c);
    const std::uint32_t id = host.anim_set_find_00b78670(anim_set, anim_name);
    if (at<std::uint32_t>(soldier, 0x190) != id || force) {
        at<std::uint32_t>(soldier, 0x190) = id;
        at<float>(soldier, 0x194) =
            host.anim_node_duration_00b6dc90(at<Object>(at<Object>(at<Object>(soldier, 0x178), 0x160), 0xc), id);
        at<float>(soldier, 0x198) = *host.string_float_map_lookup_00444be0(at<Object>(soldier, 0x188) + 0x28, anim_name);
        float start = 0.0f;
        const float duration = at<float>(soldier, 0x194);
        if (random_start && duration > 0.0f) start = host.random_uniform_00bd2f10(0, 0.0f, duration);
        at<float>(soldier, 0x1a8) = start;
        Object node = at<Object>(at<Object>(at<Object>(soldier, 0x178), 0x160), 0xc);
        host.anim_node_play_24(node, at<std::uint32_t>(soldier, 0x190), start, 1);
        at<float>(soldier, 0x19c) = at<float>(soldier, 0xa4);
        at<float>(soldier, 0x1a0) = at<float>(soldier, 0xa8);
        at<float>(soldier, 0x1a4) = at<float>(soldier, 0xac);
    }
    at<std::uint8_t>(soldier, 0x1ad) = random_start ? 1 : 0;
}

// ---------------------------------------------------------------------------
// Small accessors at the end of the segment.

// 004B4750 __fastcall(game), RET: [game+21A0h] + [[game+21A0h]+1424h]*284h + 4,
// the score record of the commit slot.
Object mission_scoring_commit_record_004b4750(Object game_object) {
    Object scoring = at<Object>(game_object, 0x21a0);
    return scoring + at<std::int32_t>(scoring, 0x1424) * 0x284 + 4;
}

// 004B4820 __thiscall(job, unused), RET 4: ECX = [00E198C4], 0068A0D0().
void render_camera_update_job_run_004b4820(Left8Host& host, Object, std::uint32_t) {
    host.game_submit_main_scene_render_0068a0d0(obj(host.global_u32(0x00e198c4)));
}

// 004B48E0 __thiscall(game, const float* box), RET 4, AL. With the camera
// [game+1ED4h] and its +1BCh set: 00B71590(ECX = [camera+1BCh], box)
// != 0AAAh. Otherwise 0.
bool game_is_aabb_in_view_004b48e0(Left8Host& host, Object game_object, const float* box) {
    Object camera = at<Object>(game_object, 0x1ed4);
    if (camera == nullptr) return false;
    Object frustum = at<Object>(camera, 0x1bc);
    if (frustum == nullptr) return false;
    return host.camera_classify_aabb_00b71590(frustum, box) != 0xaaa;
}

// 004B4940 __thiscall(game, float near), RET 4: 00B6FBF0(ECX = [game+19FCh],
// near) when that camera is set.
void game_set_camera_near_plane_004b4940(Left8Host& host, Object game_object, float near_plane) {
    Object camera = at<Object>(game_object, 0x19fc);
    if (camera != nullptr) host.camera_set_near_plane_00b6fbf0(camera, near_plane);
}

// 004B6720 __fastcall(ECX a0, EDX a1, b0, b1, out), RET 0Ch: packs the x and z
// of the four float3 points into float2 locals and calls 004F3730(ECX = a0',
// EDX = a1', b0', b1', &cross) (its result is ignored); out = (cross.x, 0,
// cross.z). On a miss the image copies its uninitialised locals; this code
// starts them at 0 (MINOR).
void segment_crossing_2d_float3_004b6720(Left8Host& host, const float* a0, const float* a1, const float* b0,
                                         const float* b1, float* out) {
    const float pa0[2] = {a0[0], a0[2]};
    const float pa1[2] = {a1[0], a1[2]};
    const float pb0[2] = {b0[0], b0[2]};
    const float pb1[2] = {b1[0], b1[2]};
    float cross[2] = {0.0f, 0.0f};
    host.math_segment_crossing_2d_004f3730(pa0, pa1, pb0, pb1, cross);
    out[0] = cross[0];
    out[2] = cross[1];
    out[1] = 0.0f;
}

// 004B6980 __thiscall(colours), RET, EAX = colours: three RGBA rows (0.2, 0.2,
// 0.2, 0) from 00CE54A0, (0.8, 0.8, 0.8, 0) from 00CE74F8 and (0.1, 0.1, 0.3, 0)
// from 00D7A2F0 / 00CE69C8 (float constants, copied as dwords).
Object cloud_light_colours_construct_004b6980(Object colours) {
    constexpr float kAmbient = 0.2f;  // 3E4CCCCD
    constexpr float kSun = 0.8f;      // 3F4CCCCD
    constexpr float kBottom = 0.1f;   // 3DCCCCCD
    constexpr float kBottomB = 0.3f;  // 3E99999A
    float* c = reinterpret_cast<float*>(colours);
    c[0] = kAmbient; c[1] = kAmbient; c[2] = kAmbient; c[3] = 0.0f;
    c[4] = kSun; c[5] = kSun; c[6] = kSun; c[7] = 0.0f;
    c[8] = kBottom; c[9] = kBottom; c[10] = kBottomB; c[11] = 0.0f;
    return colours;
}

// 004B6E30 __fastcall(screen), RET: bytes +4 and +5 = 1, 004F83B0(ECX =
// screen), then a tail jump to vtable +18h on the screen.
void frontend_screen_mark_visible_004b6e30(Left8Host& host, Object screen) {
    at<std::uint8_t>(screen, 0x4) = 1;
    at<std::uint8_t>(screen, 0x5) = 1;
    host.frontend_screen_commit_visibility_004f83b0(screen);
    host.vcall_screen_refresh_18(screen);
}

// 004B7F70 / 004B8410 / 004B8450 / 004B8490, cdecl(dst, fmt, ...): _vsprintf_s
// (00BF7289) with the counts 200h, 0FFh, 0FFCh and 100h; EAX passes the
// _vsprintf_s result through.
int format_string_512_004b7f70(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vsprintf_s(dst, 0x200, format, args);
    va_end(args);
    return result;
}
int format_string_255_004b8410(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vsprintf_s(dst, 0xff, format, args);
    va_end(args);
    return result;
}
int format_string_4092_004b8450(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vsprintf_s(dst, 0xffc, format, args);
    va_end(args);
    return result;
}
int format_string_256_004b8490(char* dst, const char* format, ...) {
    va_list args;
    va_start(args, format);
    const int result = vsprintf_s(dst, 0x100, format, args);
    va_end(args);
    return result;
}

}  // namespace bsp::cc13_left8
