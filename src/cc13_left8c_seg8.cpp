// Segment 8 hard x87 leftovers (cc13-left8c). See include/bsp/cc13_left8c_seg8.hpp
// and docs/CC13_LEFT8C.md. Offsets are the image's; comments give each
// function's address, original ABI and the simplifications made.

#include "bsp/cc13_left8c_seg8.hpp"

#include <cmath>

namespace bsp::cc13_left8c {

namespace {

constexpr std::uint32_t kGame = 0x00e188a8;          // [00E188A8], the game object
constexpr std::uint32_t kUpX = 0x00f8758c;           // float3 global up vector
constexpr std::uint32_t kUpY = 0x00f87590;
constexpr std::uint32_t kUpZ = 0x00f87594;
constexpr float kOne = 1.0f;                         // 00D7A24C
constexpr float kZero = 0.0f;                        // 00D7A218 (COMISS m32)
constexpr double kOneD = 1.0;                        // 00D7A210 (qword)
constexpr double kHalf = 0.5;                        // 00D7A280 (qword)
constexpr double kMinusOne = -1.0;                   // 00D7A250 (qword)
constexpr double kEndPad = 9.999999747378752e-05;    // 00D7A268 (qword, 1e-4f widened)
constexpr double kHashDivisor = 369097.0;            // 00CE6428 (qword) = 5A1C9h
// Sway angle = cursor * k + index * c (qwords; k are float values widened).
constexpr double kSwayT1 = 0.11999999731779099;      // 00CE67E8, with index * 2Ah
constexpr double kSwayT2 = 0.10999999940395355;      // 00CE67E0, with index * 61h
constexpr double kSwayT3 = 0.12999999523162842;      // 00CE67D8, with index * 36h
constexpr double kSwayT4 = 0.14000000059604645;      // 00CE67D0, with index * 53h

std::uint32_t vector_size(Object vector, std::uint32_t stride) {
    Object first = at<Object>(vector, 0x4);
    if (first == nullptr) return 0;
    return static_cast<std::uint32_t>(at<Object>(vector, 0x8) - first) / stride;
}

void check_index(Left8cHost& host, Object vector, std::uint32_t index, std::uint32_t stride) {
    if (at<Object>(vector, 0x4) == nullptr || index >= vector_size(vector, stride)) host.invalid_parameter_00bf6713();
}

// The image's hash-to-unit step: float((h * 7B913h + 67933h) mod 5A1C9h / 369097.0).
float hash_unit(std::uint32_t h) {
    const std::uint32_t r = (h * 0x7b913u + 0x67933u) % 0x5a1c9u;
    return static_cast<float>(static_cast<double>(static_cast<std::int32_t>(r)) / kHashDivisor);
}

// v[1] = value on the local {proxy, first, last, end} int vector, after the
// image's "first set and size > 1" check.
void set_second(Left8cHost& host, Object v, std::int32_t value) {
    if (at<Object>(v, 0x4) == nullptr || vector_size(v, 4) <= 1) host.invalid_parameter_00bf6713();
    at<std::int32_t>(at<Object>(v, 0x4), 0x4) = value;
}

}  // namespace

// 00496BD0 __fastcall(ECX runtime), RET (EBP-aligned frame, 17Ch locals). The
// scene-traffic member update, the tail of the path-runtime slot 1 0049A360.
// Fields: +20h path, +24h/+28h knot indices, byte +2Ch cyclic path, +34h
// columns, +38h row spacing, +3Ch column gap, +44h speed, +48h sway scale,
// +4Ch/+50h row/column deviation, list +6Ch members (head +70h, size +74h),
// list<int> +78h dead indices (head +7Ch, size +80h), +84h time, +88h first
// live index, +D8h landscape, byte +DCh soldiers, +E0h/+F0h class vectors,
// +100h class weights, +110h animation names, +120h their weights, +130h time
// scale, +138h..+140h half extents, byte +144h reverse, +148h path length.
// Steps, in image order:
//   amplitude = float(+48h * +44h * +130h); L = float(+38h / +44h);
//   target = _ftol2(+84h / L) * [+34h] (00BF7420 truncates), 0 when cyclic.
//   While [+88h] < target: the dead indices below [+88h] are erased from the
//   front of +78h (inlined unlink, 00BF65AC, size--); then per step, when the
//   front of +78h is [+88h] it is erased (004887A0), else the front member of
//   +6Ch gets vtable +114h(0), 00926D90(member, 2) and is unlinked and freed;
//   [+88h] += 1.
//   dir = +144h ? -1 : 1. Non-cyclic: start = float(knot(+28h) + (float(fmod(
//   float(+84h / L), 1.0)) - 1.0) * dir * +38h), end = knot(+24h) (007B05E0
//   twice, +28h first). Cyclic: start = float(P + +84h * dir * +44h), end =
//   float(start - dir * (P - 1e-4)), P = +148h.
//   Rows run from cursor = start while cursor >= end (cursor <= end when
//   reversed; FCOMIP with the conditional FXCH, unordered exits), each:
//   v = {index, 42} (00441F30 at null, inlined push_back); row position
//   pos = float((hash_unit(00484660(v)) - 0.5) * +4Ch + cursor), wrapped by
//   fmod(pos, P) (+P when negative) on a cyclic path, else clamped to
//   [0, float(P - 1e-4)]. 007B04C0(path, pos, &p, 0, !cyclic); the view box
//   p -/+ (+138h..+140h) goes to 004B48E0. Visible: 007B03C0(path, pos,
//   &M.row3, &M.row2, !cyclic), row2 *= -1.0 when reversed, row1 = the
//   global up vector, 0085DAD0(M).
//   Per column j < [+34h] (index advances with j, also for skips): a dead
//   index (front of the +78h iterator) advances that iterator and is skipped.
//   Visible: W = M; when [+34h] > 1 or +50h > 0.0f: v[1] = j and off =
//   float((hash_unit(00484660(v)) - 0.5) * +50h + (j - ([+34h] - 1.0) * 0.5)
//   * +3Ch), W.row3 += off * W.row0. Sway: s1..s4 = sin(float(cursor * k +
//   index * c)), W.row3.x += s1 * amplitude * s2, W.row3.z += s3 * amplitude
//   * s4. With a landscape: W.row3.y = 0087FA20(x, z) - [landscape+A8h], and
//   for vehicles row1 = 0087FB90 normal, 0085DAD0(W).
//   The member is the next node of +6Ch, or at the list end a new one: class
//   = 00480A90(index, +100h) from +F0h (soldiers) or +E0h, member = class
//   vtable +8(); vtable +98h([pathentity+3Ch], [pathentity+30h], &identity)
//   (007ACA30 twice); appended to +6Ch (00485920, 0048FFB0, links); vtable
//   +114h(runtime), +A0h(); soldiers with animation names: v[1] = 54h and
//   004B0F80(member, &names[00480A90(00484660(v), +120h)], 1, 0); the member
//   iterator is the list end again. Visible members get W as in 00497BB0
//   (a new member copies it, then like an existing member with byte +5Ch
//   clear calls vtable +11Ch() and copies it again; an existing member with
//   +5Ch set copies it with 004134F0), clear bytes +C8h/+10Ch, invalidate
//   their children (0042ED50) and call vtable +D8h(); hidden members with
//   byte +5Ch set call vtable +118h().
//   cursor = float(cursor - +38h * dir); v's storage is freed (00BF65AC).
// x87 operand order is from the bytes (tools/x87_dc_form_scan.py: DE E9 at
// 00496E34, 00497B46; DE EB at 00497129; DC C9 at 004971BE; D8 C9 at
// 004971D0, 0049745D; DC E9 at 0049742A; DE E1 at 00497440). No RNG: the
// picks and offsets hash the member index.
// Simplifications: the list/vector iterator self-checks that cannot fail in a
// consistent container (front node == head with a nonzero size, the cached
// container pointer != +6Ch, a node == head when it was just tested unequal)
// are not modelled; the x87 expressions are evaluated in double, not 80-bit
// extended, before each float store; sin is the CRT's in double rounded to
// float where the image uses FSIN on the float angle. The dword flag passed to
// 007B04C0 carries leftover EDX bits above the SETZ byte; the callee reads only
// the low byte, so 0/1 is passed. The local vector's EH unwind is not modelled.
void scene_traffic_update_members_00496bd0(Left8cHost& host, Object runtime) {
    const auto f = [&](std::size_t offset) { return static_cast<double>(at<float>(runtime, offset)); };
    const float amplitude = static_cast<float>(f(0x48) * at<float>(runtime, 0x44) * f(0x130));
    const float spacing_per_speed = static_cast<float>(f(0x38) / at<float>(runtime, 0x44));
    std::int32_t target = static_cast<std::int32_t>(f(0x84) / spacing_per_speed) * at<std::int32_t>(runtime, 0x34);
    const bool cyclic = at<std::uint8_t>(runtime, 0x2c) != 0;
    if (cyclic) target = 0;

    // Member-count reconciliation.
    if (at<std::int32_t>(runtime, 0x88) < target) {
        while (at<std::int32_t>(runtime, 0x80) != 0) {
            Object head = at<Object>(runtime, 0x7c);
            Object first = at<Object>(head, 0x0);
            if (at<std::int32_t>(first, 0x8) >= at<std::int32_t>(runtime, 0x88)) break;
            at<Object>(at<Object>(first, 0x4), 0x0) = at<Object>(first, 0x0);
            at<Object>(at<Object>(first, 0x0), 0x4) = at<Object>(first, 0x4);
            host.crt_free_00bf65ac(first);
            at<std::int32_t>(runtime, 0x80) -= 1;
        }
        while (at<std::int32_t>(runtime, 0x88) < target) {
            Object dead = at<std::int32_t>(runtime, 0x80) != 0 ? at<Object>(at<Object>(runtime, 0x7c), 0x0) : nullptr;
            if (dead != nullptr && at<std::int32_t>(dead, 0x8) == at<std::int32_t>(runtime, 0x88)) {
                Object result[2] = {nullptr, nullptr};
                host.int_list_erase_004887a0(runtime + 0x78, reinterpret_cast<Object>(result), runtime + 0x78, dead);
            } else if (at<std::int32_t>(runtime, 0x74) != 0) {
                host.vcall_set_group_114(at<Object>(at<Object>(at<Object>(runtime, 0x70), 0x0), 0x8), nullptr);
                host.mission_entity_kill_00926d90(at<Object>(at<Object>(at<Object>(runtime, 0x70), 0x0), 0x8), 2);
                Object head = at<Object>(runtime, 0x70);
                Object first = at<Object>(head, 0x0);
                if (first != head) {
                    at<Object>(at<Object>(first, 0x4), 0x0) = at<Object>(first, 0x0);
                    at<Object>(at<Object>(first, 0x0), 0x4) = at<Object>(first, 0x4);
                    host.crt_free_00bf65ac(first);
                    at<std::int32_t>(runtime, 0x74) -= 1;
                }
            }
            at<std::int32_t>(runtime, 0x88) += 1;
        }
    }

    std::int32_t index = at<std::int32_t>(runtime, 0x88);
    const bool reversed = at<std::uint8_t>(runtime, 0x144) != 0;
    const std::int32_t dir = reversed ? -1 : 1;
    const float dir_f = static_cast<float>(dir);
    Object path = at<Object>(runtime, 0x20);
    float start;
    float end;
    if (!cyclic) {
        const float phase = static_cast<float>(f(0x84) / spacing_per_speed);
        const float frac = static_cast<float>(host.crt_fmod_00bf857a(phase, 1.0));
        const double first_knot = host.path_knot_distance_007b05e0(path, at<std::uint32_t>(runtime, 0x28));
        start = static_cast<float>(first_knot + (static_cast<double>(frac) - kOneD) * dir_f * f(0x38));
        end = host.path_knot_distance_007b05e0(at<Object>(runtime, 0x20), at<std::uint32_t>(runtime, 0x24));
    } else {
        const float length = at<float>(runtime, 0x148);
        start = static_cast<float>(f(0x84) * dir_f * at<float>(runtime, 0x44) + length);
        end = static_cast<float>(static_cast<double>(start) - dir_f * (static_cast<double>(length) - kEndPad));
    }
    Object dead_it = at<Object>(at<Object>(runtime, 0x7c), 0x0);
    Object member_it = at<Object>(at<Object>(runtime, 0x70), 0x0);
    float cursor = start;
    const auto mul = [](float a, float b) { return static_cast<float>(static_cast<double>(a) * b); };
    const auto add = [](float a, float b) { return static_cast<float>(static_cast<double>(a) + b); };
    for (;;) {
        // 00496E68: FCOMIP with the FXCH when reversed; CF (less or unordered) exits.
        if (!reversed ? !(cursor >= end) : !(end >= cursor)) return;
        std::uint32_t vector[4] = {0, 0, 0, 0};  // {proxy, first, last, end}
        Object v = reinterpret_cast<Object>(vector);
        Object result[2] = {nullptr, nullptr};
        host.int_vector_insert_00441f30(v, reinterpret_cast<Object>(result), v, nullptr, &index);
        {
            const std::int32_t value = 0x2a;
            Object first = at<Object>(v, 0x4);
            Object last = at<Object>(v, 0x8);
            if (first != nullptr && vector_size(v, 4) < static_cast<std::uint32_t>(at<Object>(v, 0xc) - first) / 4) {
                at<std::int32_t>(last, 0x0) = value;
                at<Object>(v, 0x8) = last + 4;
            } else {
                if (first > last) host.invalid_parameter_00bf6713();
                host.int_vector_insert_00441f30(v, reinterpret_cast<Object>(result), v, last, &value);
            }
        }
        float pos = static_cast<float>((static_cast<double>(hash_unit(host.int_vector_hash_00484660(v))) - kHalf) *
                                           at<float>(runtime, 0x4c) +
                                       cursor);
        if (cyclic) {
            const float length = at<float>(runtime, 0x148);
            pos = static_cast<float>(host.crt_fmod_00bf857a(pos, length));
            if (kZero > pos) pos = add(pos, length);
        } else {
            const float limit = static_cast<float>(static_cast<double>(at<float>(runtime, 0x148)) - kEndPad);
            if (kZero > pos) {
                pos = kZero;
            } else if (pos > limit) {
                pos = limit;
            }
        }
        float m[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
        float p[3];
        host.camera_path_sample_point_007b04c0(at<Object>(runtime, 0x20), pos, p, 0, cyclic ? 0u : 1u);
        const float box[6] = {
            static_cast<float>(static_cast<double>(p[0]) - at<float>(runtime, 0x138)),
            static_cast<float>(static_cast<double>(p[1]) - at<float>(runtime, 0x13c)),
            static_cast<float>(static_cast<double>(p[2]) - at<float>(runtime, 0x140)),
            add(p[0], at<float>(runtime, 0x138)),
            add(at<float>(runtime, 0x13c), p[1]),
            add(at<float>(runtime, 0x140), p[2]),
        };
        const bool visible = cc13_left8::game_is_aabb_in_view_004b48e0(host, obj(host.global_u32(kGame)), box);
        if (visible) {
            host.camera_path_sample_world_007b03c0(at<Object>(runtime, 0x20), pos, &m[12], &m[8],
                                                   at<std::uint8_t>(runtime, 0x2c) == 0 ? 1u : 0u);
            if (at<std::uint8_t>(runtime, 0x144) != 0) {
                m[8] = static_cast<float>(static_cast<double>(m[8]) * kMinusOne);
                m[9] = static_cast<float>(static_cast<double>(m[9]) * kMinusOne);
                m[10] = static_cast<float>(kMinusOne * m[10]);
            }
            m[4] = host.global_f32(kUpX);
            m[5] = host.global_f32(kUpY);
            m[6] = host.global_f32(kUpZ);
            host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(m));
        }
        for (std::int32_t j = 0; j < at<std::int32_t>(runtime, 0x34); ++j, ++index) {
            if (dead_it != at<Object>(runtime, 0x7c) && at<std::int32_t>(dead_it, 0x8) == index) {
                dead_it = at<Object>(dead_it, 0x0);
                continue;
            }
            float w[16];
            if (visible) {
                for (std::size_t i = 0; i < 16; ++i) w[i] = m[i];
                if (at<std::int32_t>(runtime, 0x34) > 1 || at<float>(runtime, 0x50) > kZero) {
                    set_second(host, v, j);
                    const double jitter = (static_cast<double>(hash_unit(host.int_vector_hash_00484660(v))) - kHalf) *
                                          at<float>(runtime, 0x50);
                    const double column =
                        (static_cast<double>(j) - (static_cast<double>(at<std::int32_t>(runtime, 0x34)) - kOneD) * kHalf) *
                        at<float>(runtime, 0x3c);
                    const float off = static_cast<float>(jitter + column);
                    const float d[3] = {mul(off, w[0]), mul(w[1], off), mul(off, w[2])};
                    w[12] = add(w[12], d[0]);
                    w[13] = add(w[13], d[1]);
                    w[14] = add(w[14], d[2]);
                }
                const auto sway = [&](double k, std::int32_t c) {
                    const float angle = static_cast<float>(static_cast<double>(cursor) * k +
                                                           static_cast<std::int32_t>(static_cast<std::uint32_t>(index) *
                                                                                     static_cast<std::uint32_t>(c)));
                    return static_cast<float>(std::sin(static_cast<double>(angle)));
                };
                const float s1 = sway(kSwayT1, 0x2a);
                const float s2 = sway(kSwayT2, 0x61);
                w[12] = static_cast<float>(static_cast<double>(s1) * amplitude * s2 + w[12]);
                const float s3 = sway(kSwayT3, 0x36);
                const float s4 = sway(kSwayT4, 0x53);
                Object landscape = at<Object>(runtime, 0xd8);
                w[14] = static_cast<float>(static_cast<double>(s3) * amplitude * s4 + w[14]);
                if (landscape != nullptr) {
                    w[13] = static_cast<float>(
                        static_cast<double>(host.landscape_forward_local_height_0087fa20(landscape, w[12], w[14])) -
                        at<float>(landscape, 0xa8));
                    if (at<std::uint8_t>(runtime, 0xdc) == 0) {
                        float normal[3];
                        const float* n =
                            host.landscape_forward_local_normal_0087fb90(at<Object>(runtime, 0xd8), normal, w[12], w[14]);
                        w[4] = n[0];
                        w[5] = n[1];
                        w[6] = n[2];
                        host.matrix_orthonormalize_0085dad0(reinterpret_cast<Object>(w));
                    }
                }
            }
            Object member = nullptr;
            bool created = false;
            if (member_it == at<Object>(runtime, 0x70)) {
                Object class_vector = at<std::uint8_t>(runtime, 0xdc) != 0 ? runtime + 0xf0 : runtime + 0xe0;
                const std::uint32_t pick = host.weighted_pick_00480a90(static_cast<std::uint32_t>(index), runtime + 0x100);
                check_index(host, class_vector, pick, 4);
                member = host.vcall_create_instance_08(at<Object>(at<Object>(class_vector, 0x4), 4 * pick));
                const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
                Object node_owner = at<Object>(host.path_object_get_entity_007aca30(at<Object>(runtime, 0x20)), 0x30);
                Object owner = at<Object>(host.path_object_get_entity_007aca30(at<Object>(runtime, 0x20)), 0x3c);
                host.vcall_attach_98(member, owner, node_owner, reinterpret_cast<Object>(const_cast<float*>(identity)));
                Object head = at<Object>(runtime, 0x70);
                Object node = host.list_buy_node_00485920(runtime + 0x6c, head, at<Object>(head, 0x4), &member);
                host.list_incsize_0048ffb0(runtime + 0x6c, 1);
                at<Object>(head, 0x4) = node;
                at<Object>(at<Object>(node, 0x4), 0x0) = node;
                host.vcall_set_group_114(member, runtime);
                host.vcall_a0(member);
                if (at<std::uint8_t>(runtime, 0xdc) != 0 && at<Object>(runtime, 0x114) != nullptr &&
                    vector_size(runtime + 0x110, 8) != 0) {
                    set_second(host, v, 0x54);
                    const std::uint32_t anim =
                        host.weighted_pick_00480a90(host.int_vector_hash_00484660(v), runtime + 0x120);
                    check_index(host, runtime + 0x110, anim, 8);
                    cc13_left8::soldier_start_animation_004b0f80(host, member, at<Object>(runtime, 0x114) + 8 * anim,
                                                                 true, false);
                }
                member_it = at<Object>(runtime, 0x70);
                created = true;
            } else {
                member = at<Object>(member_it, 0x8);
                member_it = at<Object>(member_it, 0x0);
            }
            if (!visible) {
                if (at<std::uint8_t>(member, 0x5c) != 0) host.vcall_118(member);
                continue;
            }
            const auto store = [&]() {
                for (std::size_t i = 0; i < 16; ++i) at<float>(member, 0x74 + 4 * i) = w[i];
            };
            if (created) store();
            if (!created && at<std::uint8_t>(member, 0x5c) != 0) {
                host.matrix_copy_004134f0(member + 0x74, w);
            } else if (at<std::uint8_t>(member, 0x5c) == 0) {
                host.vcall_11c(member);
                store();
            }
            Object child = at<Object>(member, 0x48);
            at<std::uint8_t>(member, 0xc8) = 0;
            at<std::uint8_t>(member, 0x10c) = 0;
            for (; child != nullptr; child = at<Object>(child, 0x44)) host.scene_node_invalidate_subtree_pose_0042ed50(child);
            host.vcall_d8(member);
        }
        cursor = static_cast<float>(static_cast<double>(cursor) - f(0x38) * dir_f);
        if (at<Object>(v, 0x4) != nullptr) host.crt_free_00bf65ac(at<Object>(v, 0x4));
    }
}

namespace {

constexpr std::uint32_t kDebrisSpawnCount = 0x00e186e8;   // dword counter
constexpr std::uint32_t kDebrisDelayHi = 0x00e186f4;      // float
constexpr std::uint32_t kDebrisDelayLo = 0x00e186f8;      // float
constexpr std::uint32_t kDebrisSpread = 0x00e18744;       // float
constexpr std::uint32_t kDebrisSpreadSmall = 0x00e18748;  // float
constexpr std::uint32_t kDebrisClasses = 0x00e1875c;      // vector<class*> (first 00E18760)
constexpr std::uint32_t kDebrisTickWeights = 0x00e1877c;  // vector<float>
constexpr double kGravity = -9.8100004196167;             // 00CE65E0 (qword, -9.81f widened)
constexpr float kGroundDefault = -100.0f;                 // 00CE65D8
constexpr double kLift = 0.10000000149011612;             // 00D7A3A0 (qword, 0.1f widened)
constexpr float kFadeEnd = 2.0f;                          // 00CE3958
constexpr float kPi = 3.14159274f;                        // 00D7A264 (00CE684C holds -kPi)
constexpr float kTen = 10.0f;                             // 00CE38B8 (00CE6848 holds -10)

// 0047BE80 __thiscall(vector, index), RET 4, EAX: the checked operator[] of
// the 54h-byte record vector (00BF6713 when first is null or index >= size).
Object record54_at_0047be80(Left8cHost& host, Object vector, std::uint32_t index) {
    if (at<Object>(vector, 0x4) == nullptr || index >= vector_size(vector, 0x54)) host.invalid_parameter_00bf6713();
    return at<Object>(vector, 0x4) + 0x54 * index;
}

// The inlined counted-handle copy into a by-value argument slot: the slot gets
// the template and, when it is set, InterlockedIncrement([00CE221C]) on +4.
Object copy_template(Left8cHost& host, Object handle) {
    if (handle == nullptr) return nullptr;
    host.interlocked_increment_ce221c(&at<long>(handle, 0x4));
    return handle;
}

// The inlined counted-handle release: InterlockedDecrement([00CE2220]) on +4
// and slot 0 () when it reaches zero; the handle is cleared.
void release_handle(Left8cHost& host, Object& handle) {
    if (handle == nullptr) return;
    if (host.interlocked_decrement_ce2220(&at<long>(handle, 0x4)) == 0) host.object_slot_00(handle);
    handle = nullptr;
}

std::int32_t sign_of(float r) { return 0.0f > r ? -1 : (r > 0.0f ? 1 : 0); }

}  // namespace

// 0049B1F0 __thiscall(ECX spawner, float dt), RET 4: slot 1 of the vtable
// 00CE6830 that 00498E60 installs (plane_debris_spawner_construct_00498e60,
// named marker_construct until cc13-m498; the object is the plane-debris
// spawner whose records 0049A760 fills). No direct caller. Spawner fields: +20h vector of 54h-byte
// records, +44h plane entity, byte +48h, byte +49h spawned latch, +4Ch spawn
// timer. Record fields: +0 entity, +4 class, +8 position, +14h velocity, +20h
// angles, +2Ch turn rates, +38h their decay, +44h life, +48h drag, byte +4Ch
// alive, +50h class index. Steps, in image order:
//   When byte +49h is clear and the entity is gone: +49h = 1, [00E186E8] += 1.
//   keep = byte +48h set or +49h clear. +4Ch = float(+4Ch - dt).
//   With the entity, +49h clear and 0 > +4Ch, one record is added:
//     +4Ch = 00BD2F10(0, [00E186F8], [00E186F4]); 00414DB0 when entity byte
//     +C8h is 0; vtable +34h(&origin); small = 0047B880(entity); the record
//     vector grows by one (00495D40) and 0047BE80 gives the new record; +50h =
//     00480A90(spawner + 0Dh, 00E1877C) clamped to the class range; +4 =
//     class, +0 = 0, byte +4Ch = 1; then the body of 0049A760's per-record
//     setup with the draws in this order: r = draw(0, small ? [00E18748] :
//     [00E18744]), 00439950 direction, position = R * {r*d.x, float(d.y*r) *
//     0.1, r*d.z} + row3 of the entity world matrix (+CCh); a1..a3 =
//     draw(-pi, pi), +20h = a3, +24h = a2, +28h = a1; +44h = draw(class+28h,
//     +2Ch); r1..r3 = draw(-10, 10) give signs; +34h = draw(class+48h, +4Ch)
//     * sign(r1), +30h = next * sign(r2), +2Ch = next * sign(r3); c1..c3 =
//     draw(class+50h, +54h), +38h = c3, +3Ch = c2, +40h = c1; +48h =
//     draw(class+40h, +44h); dist = draw(class+30h, +34h), yaw =
//     draw(class+38h, +3Ch), angle = draw(-pi, pi); w = sin(angle)*row0 +
//     cos(angle)*row1; dir = sin(yaw)*w + cos(yaw)*row2; +14h = origin +
//     dist*dir.
//   For each alive record (byte +4Ch): life = float(+44h - dt). Below 0: +4Ch
//   = 0, the entity gets vtable +114h(0) and 00926D90(entity, 2), +0 = 0.
//   Otherwise keep = 1; velocity.y += float(class+58h * -9.81 * dt) (x and z
//   get + 0.0); with drag > 0: len = 0042B2F0(&velocity), velocity *=
//   float((len - drag*dt) / len); position += velocity * dt. water =
//   0078CF20([game+19F0h], x, z); ground = -100, 00903860([game+19CCh],
//   &position, &ground). When water > y or ground > y: with a splash template
//   (class+5Ch), water >= ground and !0043B9C0(&position), 008685E0 creates
//   the template at {x, water, z} in [game+19ECh] and the result's byte +9 =
//   1 (the out handle is released); life = 0 either way. Each turn rate moves
//   toward 0 by its decay * dt (x87: rate > 0 ? rate - decay*dt : decay*dt +
//   rate); angles += rates * dt. Without an entity, class vtable +8() creates
//   one (0 without a class), vtable +98h(0, [game+19CCh], &identity),
//   +114h(spawner), +A0h(); with a trail template (class+60h), 008685E0 at the
//   position and 004845D0 assigns it to entity+188h. With an entity: when
//   +188h is set, 004842C0(+188h, &position); entity+74h = 00413920(
//   0047C2A0(angles), {identity, row3 = position}) through 004134F0; +18Ch =
//   00419010(0, 0, 2, 1, life); vtable +D8h().
//   Finally byte +48h = 0 and, when keep is clear, vtable slot 0(1) deletes
//   the spawner.
// RNG: 00BD2F10 stream 0 draws 1 (timer) + 18 per spawned record, plus
// 00439950's direction; no draws in the per-record tick.
// x87 operand order is from the bytes (tools/x87_dc_form_scan.py: DE EA at
// 0049B253, D8 E1 at 0049B9AA, DC C1 at 0049BA1E, D8 EA / DE F2 at
// 0049BA6A/0049BA6C, D8 CA + DE E9 at 0049BC69/0049BC92); dt is carried in
// ST0 across the loop and reloaded after calls.
// Simplifications: sin/cos come from the CRT in double rounded to float where
// the image uses FSIN/FCOS on the float input; products and sums are
// evaluated in double and rounded at each image store; the record fill value
// passed by value to 00495D40 is uninitialised stack in the image and is not
// modelled (the new record's fields are all written); the loop's repeated
// size check before the record access cannot fail; the EH state of the trail
// handle is not modelled.
void plane_debris_tick_0049b1f0(Left8cHost& host, Object spawner, float dt) {
    const auto mul = [](float a, float b) { return static_cast<float>(static_cast<double>(a) * b); };
    const auto add = [](float a, float b) { return static_cast<float>(static_cast<double>(a) + b); };
    bool keep = false;
    if (at<std::uint8_t>(spawner, 0x49) == 0 && at<Object>(spawner, 0x44) == nullptr) {
        at<std::uint8_t>(spawner, 0x49) = 1;
        host.global_add_i32(kDebrisSpawnCount, 1);
    }
    if (at<std::uint8_t>(spawner, 0x48) != 0 || at<std::uint8_t>(spawner, 0x49) == 0) keep = true;
    const float timer = static_cast<float>(static_cast<double>(at<float>(spawner, 0x4c)) - dt);
    at<float>(spawner, 0x4c) = timer;
    if (at<Object>(spawner, 0x44) != nullptr && at<std::uint8_t>(spawner, 0x49) == 0 && 0.0f > timer) {
        at<float>(spawner, 0x4c) =
            host.random_uniform_00bd2f10(0, host.global_f32(kDebrisDelayLo), host.global_f32(kDebrisDelayHi));
        Object entity = at<Object>(spawner, 0x44);
        if (at<std::uint8_t>(entity, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        float origin[3];
        host.vcall_34(entity, origin);
        const float* m = &at<float>(entity, 0xcc);
        const bool small = host.unit_uses_small_turn_roll_limit_0047b880(entity);
        Object classes = host.image_address(kDebrisClasses);
        const std::int32_t types = static_cast<std::int32_t>(vector_size(classes, 4));
        Object records = spawner + 0x20;
        const std::uint32_t count = vector_size(records, 0x54);
        host.record54_vector_resize_00495d40(records, count + 1);
        Object record = record54_at_0047be80(host, records, count);
        std::int32_t pick = static_cast<std::int32_t>(host.weighted_pick_00480a90(
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(spawner + 0xd)),
            host.image_address(kDebrisTickWeights)));
        at<std::int32_t>(record, 0x50) = pick;
        if (pick < 0) {
            pick = 0;
        } else if (pick > types - 1) {
            pick = types - 1;
        }
        at<std::int32_t>(record, 0x50) = pick;
        if (at<Object>(classes, 0x4) == nullptr || static_cast<std::uint32_t>(pick) >= vector_size(classes, 4)) {
            host.invalid_parameter_00bf6713();
        }
        at<Object>(record, 0x4) = at<Object>(at<Object>(classes, 0x4), 4 * pick);
        at<Object>(record, 0x0) = nullptr;
        at<std::uint8_t>(record, 0x4c) = 1;

        const float r = host.random_uniform_00bd2f10(0, 0.0f, host.global_f32(small ? kDebrisSpreadSmall : kDebrisSpread));
        float direction[3];
        const float* d = host.random_unit_direction_00439950(direction);
        at<float>(record, 0x8) = mul(d[0], r);
        at<float>(record, 0xc) = mul(d[1], r);
        at<float>(record, 0x10) = mul(r, d[2]);
        at<float>(record, 0xc) = static_cast<float>(static_cast<double>(at<float>(record, 0xc)) * kLift);
        const float vx = at<float>(record, 0x8);
        const float vy = at<float>(record, 0xc);
        const float vz = at<float>(record, 0x10);
        const float a[3] = {mul(m[0], vx), mul(m[1], vx), mul(vx, m[2])};
        const float b[3] = {mul(vy, m[4]), mul(m[5], vy), mul(vy, m[6])};
        const float c[3] = {mul(m[8], vz), mul(m[9], vz), mul(vz, m[10])};
        float sum[3];
        for (std::size_t i = 0; i < 3; ++i) sum[i] = add(add(c[i], b[i]), a[i]);
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x8 + 4 * i) = sum[i];
        at<float>(record, 0x8) = add(at<float>(record, 0x8), m[12]);
        at<float>(record, 0xc) = add(m[13], at<float>(record, 0xc));
        at<float>(record, 0x10) = add(m[14], at<float>(record, 0x10));
        const float a1 = host.random_uniform_00bd2f10(0, -kPi, kPi);
        const float a2 = host.random_uniform_00bd2f10(0, -kPi, kPi);
        const float a3 = host.random_uniform_00bd2f10(0, -kPi, kPi);
        at<float>(record, 0x20) = a3;
        at<float>(record, 0x24) = a2;
        at<float>(record, 0x28) = a1;
        at<float>(record, 0x44) = host.random_uniform_00bd2f10(0, at<float>(at<Object>(record, 0x4), 0x28),
                                                                at<float>(at<Object>(record, 0x4), 0x2c));
        const float r1 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const float r2 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const float r3 = host.random_uniform_00bd2f10(0, -kTen, kTen);
        const std::int32_t s1 = sign_of(r1);
        const std::int32_t s2 = sign_of(r2);
        const std::int32_t s3 = sign_of(r3);
        const auto turn = [&](std::int32_t sign) {
            Object debris_class = at<Object>(record, 0x4);
            return static_cast<float>(static_cast<double>(host.random_uniform_00bd2f10(
                                          0, at<float>(debris_class, 0x48), at<float>(debris_class, 0x4c))) *
                                      sign);
        };
        const float t1 = turn(s1);
        const float t2 = turn(s2);
        const float t3 = turn(s3);
        at<float>(record, 0x34) = t1;
        at<float>(record, 0x2c) = t3;
        at<float>(record, 0x30) = t2;
        const auto draw = [&](std::size_t lo) {
            Object debris_class = at<Object>(record, 0x4);
            return host.random_uniform_00bd2f10(0, at<float>(debris_class, lo), at<float>(debris_class, lo + 4));
        };
        const float c1 = draw(0x50);
        const float c2 = draw(0x50);
        const float c3 = draw(0x50);
        at<float>(record, 0x38) = c3;
        at<float>(record, 0x3c) = c2;
        at<float>(record, 0x40) = c1;
        at<float>(record, 0x48) = draw(0x40);
        const float dist = draw(0x30);
        const float yaw = draw(0x38);
        const float angle = host.random_uniform_00bd2f10(0, -kPi, kPi);
        const float cos_angle = static_cast<float>(std::cos(static_cast<double>(angle)));
        const float u[3] = {mul(m[4], cos_angle), mul(m[5], cos_angle), mul(cos_angle, m[6])};
        const float sin_angle = static_cast<float>(std::sin(static_cast<double>(angle)));
        const float v[3] = {mul(m[0], sin_angle), mul(m[1], sin_angle), mul(sin_angle, m[2])};
        const float w[3] = {add(v[0], u[0]), add(v[1], u[1]), add(v[2], u[2])};
        const float cos_yaw = static_cast<float>(std::cos(static_cast<double>(yaw)));
        const float p[3] = {mul(cos_yaw, m[8]), mul(m[9], cos_yaw), mul(cos_yaw, m[10])};
        const float sin_yaw = static_cast<float>(std::sin(static_cast<double>(yaw)));
        const float q[3] = {mul(sin_yaw, w[0]), mul(sin_yaw, w[1]), mul(sin_yaw, w[2])};
        const float dir[3] = {add(q[0], p[0]), add(q[1], p[1]), add(q[2], p[2])};
        const float offset[3] = {mul(dir[0], dist), mul(dir[1], dist), mul(dist, dir[2])};
        for (std::size_t i = 0; i < 3; ++i) at<float>(record, 0x14 + 4 * i) = add(origin[i], offset[i]);
    }

    Object records = spawner + 0x20;
    for (std::uint32_t i = 0; at<Object>(records, 0x4) != nullptr && i < vector_size(records, 0x54); ++i) {
        Object record = at<Object>(records, 0x4) + 0x54 * i;
        if (at<std::uint8_t>(record, 0x4c) == 0) continue;
        const float life = static_cast<float>(static_cast<double>(at<float>(record, 0x44)) - dt);
        at<float>(record, 0x44) = life;
        if (0.0f > life) {
            Object entity = at<Object>(record, 0x0);
            at<std::uint8_t>(record, 0x4c) = 0;
            if (entity != nullptr) {
                host.vcall_set_group_114(entity, nullptr);
                host.mission_entity_kill_00926d90(entity, 2);
            }
            at<Object>(record, 0x0) = nullptr;
            continue;
        }
        keep = true;
        const float fall = static_cast<float>(static_cast<double>(at<float>(at<Object>(record, 0x4), 0x58)) * kGravity * dt);
        float* velocity = &at<float>(record, 0x14);
        velocity[0] = static_cast<float>(static_cast<double>(velocity[0]) + 0.0);
        velocity[1] = add(fall, velocity[1]);
        velocity[2] = static_cast<float>(0.0 + static_cast<double>(velocity[2]));
        const float drag = at<float>(record, 0x48);
        if (drag > 0.0f) {
            const float length = host.vector3_length_0042b2f0(velocity);
            const float ratio = static_cast<float>((static_cast<double>(length) - static_cast<double>(drag) * dt) /
                                                   static_cast<double>(length));
            velocity[0] = mul(ratio, velocity[0]);
            velocity[1] = mul(ratio, velocity[1]);
            velocity[2] = mul(ratio, velocity[2]);
        }
        const float step[3] = {mul(velocity[0], dt), mul(velocity[1], dt), mul(dt, velocity[2])};
        float* position = &at<float>(record, 0x8);
        for (std::size_t k = 0; k < 3; ++k) position[k] = add(step[k], position[k]);
        Object game = obj(host.global_u32(kGame));
        const float water = host.game_world_sample_water_height_0078cf20(at<Object>(game, 0x19f0), position[0], position[2]);
        float ground = kGroundDefault;
        host.world_ground_height_at_00903860(at<Object>(obj(host.global_u32(kGame)), 0x19cc), position, &ground);
        const float y = position[1];
        if (water > y || ground > y) {
            Object splash = at<Object>(at<Object>(record, 0x4), 0x5c);
            if (splash != nullptr && water >= ground && !host.surface_probe_0043b9c0(position)) {
                const float at_surface[3] = {position[0], water, position[2]};
                Object scene_root = at<Object>(obj(host.global_u32(kGame)), 0x19ec);
                Object slot = copy_template(host, at<Object>(at<Object>(record, 0x4), 0x5c));
                Object out = nullptr;
                Object effect = *host.point_effect_create_at_position_008685e0(&out, scene_root, slot, at_surface, 0, 0);
                release_handle(host, out);
                if (effect != nullptr) at<std::uint8_t>(effect, 0x9) = 1;
            }
            at<float>(record, 0x44) = 0.0f;
        }
        const auto decay = [&](std::size_t rate, std::size_t step_offset) {
            const float value = at<float>(record, rate);
            const double change = static_cast<double>(at<float>(record, step_offset)) * dt;
            at<float>(record, rate) = static_cast<float>(value > 0.0f ? static_cast<double>(value) - change
                                                                      : change + static_cast<double>(value));
        };
        decay(0x2c, 0x38);
        decay(0x30, 0x3c);
        decay(0x34, 0x40);
        const float turn[3] = {mul(at<float>(record, 0x2c), dt), mul(at<float>(record, 0x30), dt),
                               mul(dt, at<float>(record, 0x34))};
        for (std::size_t k = 0; k < 3; ++k) at<float>(record, 0x20 + 4 * k) = add(turn[k], at<float>(record, 0x20 + 4 * k));
        Object entity = at<Object>(record, 0x0);
        if (entity == nullptr) {
            Object debris_class = at<Object>(record, 0x4);
            entity = debris_class != nullptr ? host.vcall_create_instance_08(debris_class) : nullptr;
            at<Object>(record, 0x0) = entity;
            const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
            host.vcall_attach_98(entity, nullptr, at<Object>(obj(host.global_u32(kGame)), 0x19cc),
                                 reinterpret_cast<Object>(const_cast<float*>(identity)));
            host.vcall_set_group_114(at<Object>(record, 0x0), spawner);
            host.vcall_a0(at<Object>(record, 0x0));
            if (at<Object>(at<Object>(record, 0x4), 0x60) != nullptr) {
                Object scene_root = at<Object>(obj(host.global_u32(kGame)), 0x19ec);
                Object slot = copy_template(host, at<Object>(at<Object>(record, 0x4), 0x60));
                Object out = nullptr;
                Object* made = host.point_effect_create_at_position_008685e0(&out, scene_root, slot, position, 0, 0);
                host.retained_assign_004845d0(at<Object>(record, 0x0) + 0x188, made);
                release_handle(host, out);
            }
            entity = at<Object>(record, 0x0);
            if (entity == nullptr) continue;
        }
        if (at<Object>(entity, 0x188) != nullptr) host.point_effect_set_world_point_004842c0(at<Object>(entity, 0x188), position);
        float translate[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
        translate[12] = position[0];
        translate[13] = position[1];
        translate[14] = position[2];
        const float angles[3] = {at<float>(record, 0x20), at<float>(record, 0x24), at<float>(record, 0x28)};
        Object target = at<Object>(record, 0x0) + 0x74;
        float rotation[16];
        float product[16];
        float* r = host.matrix_build_from_euler_yxz_0047c2a0(rotation, angles);
        const float* moved = host.matrix_multiply_00413920(reinterpret_cast<Object>(r), product, translate);
        host.matrix_copy_004134f0(target, moved);
        at<float>(at<Object>(record, 0x0), 0x18c) =
            host.interpolate_clamped_00419010(0.0f, 0.0f, kFadeEnd, kOne, at<float>(record, 0x44));
        host.vcall_d8(at<Object>(record, 0x0));
    }
    at<std::uint8_t>(spawner, 0x48) = 0;
    if (!keep) host.vcall_deleting_dtor_00(spawner, 1);
}

}  // namespace bsp::cc13_left8c
