// cc13 segment 53 / 47 leftovers (packet cc13-seg53U). See include/bsp/cc13_seg53U.hpp and
// docs/CC13_SEG53U.md for the evidence. Instruction addresses in the comments are the image's;
// names are hypotheses, not recovered symbols.

#include "bsp/cc13_seg53U.hpp"

#include <cstring>

namespace bsp::cc13_seg53U {

namespace {

float* global_floats(Seg53UHost& host, std::uint32_t address) {
    return static_cast<float*>(host.global(address));
}

// A function-local static colour: the guard word's bit 0, then four floats.
void init_static_rgba(Seg53UHost& host, std::uint32_t guard_address, std::uint32_t address,
                      float r, float g, float b, float a) {
    auto& guard = *static_cast<std::uint32_t*>(host.global(guard_address));
    if ((guard & 1u) == 0) {
        guard |= 1u;
        float* const c = global_floats(host, address);
        c[0] = r;
        c[1] = g;
        c[2] = b;
        c[3] = a;
    }
}

// [task+3FCh], refreshed through 00414DB0 when its byte +C8h is clear.
void* refreshed_owner(void* task, Seg53UHost& host) {
    void* const owner = ptr(task, 0x3FC);
    if (at<std::uint8_t>(owner, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(owner);
    return owner;
}

// 007B7517..007B75D7 / 007B76A4..007B775B: tip + 0.5 * len * scale * axis, every product
// rounded to float as the image's FSTP float stores do (the 0.5 is the exact double 00D7A280;
// the DC C9 / D8 C9 encodings at 007B7585 / 007B7591 were read as bytes).
void half_axis_end(void* owner, std::size_t scale_off, std::size_t axis_off, float len,
                   const float tip[3], float end[3]) {
    const float s = at<float>(owner, scale_off);
    const float a0 = at<float>(owner, axis_off) * s;
    const float a1 = at<float>(owner, axis_off + 4) * s;
    const float a2 = s * at<float>(owner, axis_off + 8);
    const float b0 = a0 * len;
    const float b1 = a1 * len;
    const float b2 = len * a2;
    const float h0 = static_cast<float>(b0 * kHalfDouble);
    const float h1 = static_cast<float>(b1 * kHalfDouble);
    const float h2 = static_cast<float>(kHalfDouble * b2);
    end[0] = h0 + tip[0];
    end[1] = h1 + tip[1];
    end[2] = h2 + tip[2];
}

}  // namespace

// 00859950.
void gun_build_sync_delta_00859950(void* gun, void* record, Seg53UHost& host) {
    void* const owner = ptr(gun, 0x3C);
    if (owner != nullptr && host.object_is_kind_of_vslot_5c(owner, kGunSyncOwnerSkipKind)) {
        return;                                                            // 0085996A
    }
    void* const peer = at<void*>(record, 0);                               // 00859976
    const std::int32_t value = peer == nullptr ? kGunSyncNoPeerValue : at<std::int32_t>(peer, 0x20);
    // 0085998C: ECX is [gun+3Ch] again, not null-tested on this path.
    const std::uint8_t role = host.unit_has_role_slot_value_00927850(ptr(gun, 0x3C), value);
    // 00859999: AL overwrites the low byte of the record argument's slot, and that whole
    // dword is what 008599EC loads and pushes.
    const std::uint32_t role_dword =
        (static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(record)) & 0xFFFFFF00u) | role;
    host.replication_build_sync_delta_0077b280(gun, record, role_dword, 0.0f, 1, kGunSyncTolA, 3,
                                               kGunSyncTolB, 10, kGunSyncTolC, 20, kGunSyncTolD,
                                               200);                       // 00859A0E
}

// 00864380.
void unit_gunnery_ai_destruct_00864380(void* ai, Seg53UHost& host) {
    at<std::uint32_t>(ai, 0x00) = kUnitGunneryAiVtable;                    // 008643A8
    at<std::uint32_t>(ai, 0x1C) = kUnitGunneryAiVtable1C;                  // 008643AF
    if (void* const p = ptr(ai, 0x5C)) host.object_delete_vslot_0(p, 1);   // 008643CB
    if (void* const p = ptr(ai, 0x60)) host.object_delete_vslot_0(p, 1);   // 008643DA
    if (void* const holder = ptr(ai, 0x68)) {                              // 008643DC
        if (void* const buffer = ptr(holder, 0x4)) host.crt_free_00bf6989(buffer);
        host.operator_delete_00bf65ac(holder);                             // 008643F4
    }
    host.operator_delete_00bf65ac(ptr(ai, 0x64));                          // 00864400, no test

    // 00864405..00864483: delete every mapped object of the tree at +B0h, erasing as it goes.
    void* const tree = add(ai, 0xB0);
    void* container = tree;
    auto* node = at<std::uint8_t*>(ptr(ai, 0xB4), 0);                      // head->left
    for (;;) {
        void* const end = ptr(tree, 0x4);                                  // 00864422, re-read
        if (container == nullptr || container != tree) host.invalid_parameter_00bf6713();
        if (node == end) break;                                            // 00864438
        if (container == nullptr) host.invalid_parameter_00bf6713();       // 0086443E
        if (node == ptr(container, 0x4)) host.invalid_parameter_00bf6713();  // 00864448
        if (void* const mapped = ptr(node, 0x10)) host.object_delete_vslot_0(mapped, 1);
        void* iterator[2] = {container, node};                             // 00864460
        host.tree_iterator_increment_00862110(iterator);                   // 00864468
        void* out[2] = {};
        host.tree_erase_00863bf0(tree, out, container, node);              // 00864476
        container = iterator[0];                                           // 0086447B
        node = static_cast<std::uint8_t*>(iterator[1]);
    }

    // 00864485: the vector at +C0h.
    if (void* const data = ptr(ai, 0xC0)) host.operator_delete_00bf65ac(data);
    at<std::uint32_t>(ai, 0xC0) = 0;
    at<std::uint32_t>(ai, 0xC4) = 0;
    at<std::uint32_t>(ai, 0xC8) = 0;

    // 008644B0..008644D8: clear the tree, free its head.
    void* const head = ptr(tree, 0x4);
    void* out[2] = {};
    host.tree_erase_range_00864090(tree, out, tree, ptr(head, 0), tree, head);
    host.operator_delete_00bf65ac(ptr(tree, 0x4));                         // 008644C9
    at<std::uint32_t>(tree, 0x4) = 0;
    at<std::uint32_t>(tree, 0x8) = 0;
    host.tick_subnode_owner_destruct_base_007293d0(ai);                    // 008644E3
}

// 007B7290. Slot +04h of the rocket bot task; draws only when byte +35Dh is set.
void bot_task_rocket_debug_draw_007b7290(void* task, Seg53UHost& host) {
    host.trivial_0099b6e0(task);                                           // 007B7298
    if (at<std::uint8_t>(task, 0x35D) == 0) return;                        // 007B72A4

    init_static_rgba(host, kStaticRedGuard, kStaticRed, kOne, 0.0f, 0.0f, kOne);
    init_static_rgba(host, kStaticBlueGuard, kStaticBlue, 0.0f, 0.0f, kOne, kOne);

    // 007B7315..007B7359: a marker at the task's point +370h.
    const float center[3] = {at<float>(task, 0x370), at<float>(task, 0x374), at<float>(task, 0x378)};
    host.debug_wire_sphere_marker_00860be0(center, global_floats(host, kStaticBlue),
                                           kMarkerRadius, global_floats(host, kStaticRed), 1);

    // 007B735E..007B73D7: len = |owner position (+FCh) - point|.
    const float point[3] = {at<float>(task, 0x370), at<float>(task, 0x374), at<float>(task, 0x378)};
    void* owner = refreshed_owner(task, host);
    const float delta[3] = {at<float>(owner, 0xFC) - point[0], at<float>(owner, 0x100) - point[1],
                            at<float>(owner, 0x104) - point[2]};
    const float len = static_cast<float>(host.vector3_length_0042b2f0(delta));

    // 007B73D1..007B7488: tip = owner position + len * (ECh, F0h, F4h).
    owner = refreshed_owner(task, host);
    const float v1 = at<float>(owner, 0xF0) * len;                         // 007B7407
    const float v2 = at<float>(owner, 0xF4) * len;                         // 007B7410
    const float v0 = len * at<float>(owner, 0xEC);                         // 007B7417
    owner = refreshed_owner(task, host);                                   // 007B73FB / 007B7436
    const float tip[3] = {v0 + at<float>(owner, 0xFC), at<float>(owner, 0x100) + v1,
                          at<float>(owner, 0x104) + v2};

    init_static_rgba(host, kStaticWhiteGuard, kStaticWhite, kOne, kOne, kOne, kOne);
    host.debug_wire_sphere_marker_00860be0(tip, global_floats(host, kStaticWhite), kMarkerRadius,
                                           global_floats(host, kStaticWhite), 1);  // 007B74FC

    // 007B7501..007B7689: a line from the tip along half the scaled (DCh, E0h, E4h) axis.
    float end[3];
    owner = refreshed_owner(task, host);
    half_axis_end(owner, 0x9E8, 0xDC, len, tip, end);
    init_static_rgba(host, kStaticGreenGuard, kStaticGreen, 0.0f, kOne, kHalf, kOne);
    float rgba0[4];
    float rgba1[4];
    std::memcpy(rgba0, global_floats(host, kStaticGreen), sizeof rgba0);
    std::memcpy(rgba1, rgba0, sizeof rgba1);
    host.debug_line_rgba_0085f990(tip, end, rgba0, rgba1);

    // 007B768E..007B781D: the same along (CCh, D0h, D4h) scaled by +9E4h.
    owner = refreshed_owner(task, host);
    half_axis_end(owner, 0x9E4, 0xCC, len, tip, end);
    init_static_rgba(host, kStaticCyanGuard, kStaticCyan, 0.0f, kHalf, kOne, kOne);
    std::memcpy(rgba0, global_floats(host, kStaticCyan), sizeof rgba0);
    std::memcpy(rgba1, rgba0, sizeof rgba1);
    host.debug_line_rgba_0085f990(tip, end, rgba0, rgba1);
}

// 007D06A0. For i = 0, 1, ...: name = prefix + _itoa(i); stop at the first name the model has
// no note node for; otherwise push the node onto the caller's vector.
void model_collect_numbered_note_nodes_007d06a0(void* model, const char* prefix, void* vector,
                                                Seg53UHost& host) {
    for (std::int32_t i = 0;; ++i) {
        // The 1Ch-byte std::string at image ESP+20h: +4h buffer / pointer, +14h size,
        // +18h capacity.
        alignas(4) std::uint8_t str[0x1C] = {};
        at<std::uint32_t>(str, 0x18) = 0xF;                                // 007D06E2
        at<std::uint32_t>(str, 0x14) = 0;                                  // 007D06EA
        at<std::uint8_t>(str, 0x4) = 0;                                    // 007D06EE
        host.string_assign_counted_00408720(str, prefix, std::strlen(prefix));  // 007D0706
        char digits[0x70];
        host.itoa_00bf9010(i, digits, 10);                                 // 007D071E
        host.string_append_007b7cc0(str, digits, std::strlen(digits));     // 007D0745
        const char* const name = at<std::uint32_t>(str, 0x18) >= 0x10
                                     ? at<const char*>(str, 0x4)
                                     : reinterpret_cast<const char*>(add(str, 0x4));
        void* node = host.model_find_note_node_0071ad50(model, name);      // 007D0762
        if (node == nullptr) {
            if (at<std::uint32_t>(str, 0x18) >= 0x10) host.operator_delete_00bf65ac(ptr(str, 0x4));
            return;                                                        // 007D07EB
        }
        // 007D076F..007D07B9: the inlined push_back.
        auto* const first = at<std::uint8_t*>(vector, 0x4);
        const std::uint32_t size = first == nullptr
            ? 0u
            : static_cast<std::uint32_t>((at<std::uint8_t*>(vector, 0x8) - first) >> 2);
        if (first != nullptr &&
            size < static_cast<std::uint32_t>((at<std::uint8_t*>(vector, 0xC) - first) >> 2)) {
            auto*& last = at<std::uint8_t*>(vector, 0x8);
            *reinterpret_cast<void**>(last) = node;                        // 007D0795
            last += 4;
        } else {
            auto* const last = at<std::uint8_t*>(vector, 0x8);
            if (first > last) host.invalid_parameter_00bf6713();           // 007D07A6
            void* out[2] = {};
            host.dword_vector_insert_one_00505d50(vector, out, vector, last, &node);
        }
        if (at<std::uint32_t>(str, 0x18) >= 0x10) {
            host.operator_delete_00bf65ac(ptr(str, 0x4));                  // 007D07D9
        }
    }
}

}  // namespace bsp::cc13_seg53U
