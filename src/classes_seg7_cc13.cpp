#include "bsp/classes_seg7_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cstring>

namespace bsp::cc13_seg7 {

namespace {

// One field-copy step of the record copies: 'd' dword, 'f' float (FLD/FSTP DWORD, which
// quiets a signalling NaN; cc13-fixture10), 'b' byte.
struct CopyStep {
    char kind;
    std::uint32_t offset;
};

template <std::size_t N>
Object copy_record(Object dest, Object source, const CopyStep (&steps)[N]) {
    for (const CopyStep& step : steps) {
        switch (step.kind) {
        case 'd': native_at<std::uint32_t>(dest, step.offset) = native_at<std::uint32_t>(source, step.offset); break;
        case 'f':
            native_at<std::uint32_t>(dest, step.offset) =
                cc13x87::x87_fld_fstp_bits(native_at<std::uint32_t>(source, step.offset));
            break;
        default: native_at<std::uint8_t>(dest, step.offset) = native_at<std::uint8_t>(source, step.offset); break;
        }
    }
    return dest;
}

constexpr CopyStep kRecord54[] = {
    {'d', 0x00}, {'d', 0x04}, {'f', 0x08}, {'f', 0x0C}, {'f', 0x10}, {'f', 0x14}, {'f', 0x18},
    {'f', 0x1C}, {'f', 0x20}, {'f', 0x24}, {'f', 0x28}, {'f', 0x2C}, {'f', 0x30}, {'f', 0x34},
    {'f', 0x38}, {'f', 0x3C}, {'f', 0x40}, {'f', 0x44}, {'f', 0x48}, {'b', 0x4C}, {'d', 0x50}};
constexpr CopyStep kRecord4C[] = {
    {'d', 0x00}, {'d', 0x04}, {'f', 0x08}, {'f', 0x0C}, {'f', 0x10}, {'f', 0x14}, {'f', 0x18},
    {'f', 0x1C}, {'f', 0x20}, {'f', 0x24}, {'f', 0x28}, {'f', 0x2C}, {'f', 0x30}, {'f', 0x34},
    {'f', 0x38}, {'f', 0x3C}, {'f', 0x40}, {'f', 0x44}, {'b', 0x48}};
constexpr CopyStep kRecord40[] = {
    {'d', 0x00}, {'d', 0x04}, {'f', 0x08}, {'f', 0x0C}, {'d', 0x10}, {'f', 0x14}, {'f', 0x18},
    {'f', 0x1C}, {'f', 0x20}, {'f', 0x24}, {'f', 0x28}, {'d', 0x2C}, {'f', 0x30}, {'f', 0x34},
    {'f', 0x38}, {'f', 0x3C}};

constexpr float kOne = 1.0f;          // 00D7A24C
constexpr double kHalf = 0.5;         // 00D7A280 (double)

// x87 float arithmetic at 53-bit precision, stored as float: equal to the
// float operation (double rounding through 53 bits is innocuous for + - * /).
float f_add(float a, float b) { return static_cast<float>(static_cast<double>(a) + static_cast<double>(b)); }
float f_sub(float a, float b) { return static_cast<float>(static_cast<double>(a) - static_cast<double>(b)); }

// The class AABB of an entity, shared by 00481090 and 004812F0: refresh the
// world pose when entity+C8h is clear, then min = pose - extent and max =
// pose + extent with extents (class +538h) +A4h (x), +ACh (y), +A0h (z).
// Box layout passed to 004B48E0: min x, y, z, max x, y, z.
bool entity_aabb_in_view(Seg7Host& host, Object entity_holder, std::uint32_t entity_offset) {
    Object entity = native_at<Object>(entity_holder, entity_offset);
    if (native_at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float px = native_at<float>(entity, 0xFC);
    const float py = native_at<float>(entity, 0x100);
    const float pz = native_at<float>(entity, 0x104);
    Object klass = native_at<Object>(native_at<Object>(entity_holder, entity_offset), 0x538);
    const float ex = native_at<float>(klass, 0xA4);
    const float ey = native_at<float>(klass, 0xAC);
    const float ez = native_at<float>(klass, 0xA0);
    const float box[6] = {f_sub(px, ex), f_sub(py, ey), f_sub(pz, ez),
                          f_add(px, ex), f_add(py, ey), f_add(pz, ez)};
    return host.game_is_aabb_in_view_004b48e0(host.game_00e188a8(), box);
}

void kill_entity(Seg7Host& host, Object entity) {
    host.attach_vcall_114(entity, nullptr);
    host.mission_entity_kill_00926d90(entity, 2);
}

void release_ref(Seg7Host& host, Object object) {
    if (host.interlocked_decrement_import_00ce2220(object + 4) == 0) host.object_vcall_00(object);
}

} // namespace

// ---------------------------------------------------------------------------
// Record copies.
// ---------------------------------------------------------------------------
Object record54_copy_0047d250(Object dest, Object source) { return copy_record(dest, source, kRecord54); }
Object record4c_copy_0047d2e0(Object dest, Object source) { return copy_record(dest, source, kRecord4C); }
Object record40_copy_0047f0c0(Object dest, Object source) { return copy_record(dest, source, kRecord40); }
Object record54_copy_assign_0047f930(Object dest, Object source) { return copy_record(dest, source, kRecord54); }
Object record4c_copy_assign_0047f9c0(Object dest, Object source) { return copy_record(dest, source, kRecord4C); }

// 0047E540.
Object matrix_init_translation_0047e540(Object matrix, const float* translation) {
    native_at<float>(matrix, 0x30) = translation[0];
    native_at<float>(matrix, 0x00) = kOne;
    native_at<float>(matrix, 0x34) = translation[1];
    native_at<float>(matrix, 0x04) = 0.0f;
    native_at<float>(matrix, 0x08) = 0.0f;
    native_at<float>(matrix, 0x38) = translation[2];
    native_at<float>(matrix, 0x0C) = 0.0f;
    native_at<float>(matrix, 0x10) = 0.0f;
    native_at<float>(matrix, 0x14) = kOne;
    for (std::uint32_t offset = 0x18; offset <= 0x24; offset += 4) native_at<float>(matrix, offset) = 0.0f;
    native_at<float>(matrix, 0x28) = kOne;
    native_at<float>(matrix, 0x2C) = 0.0f;
    native_at<float>(matrix, 0x3C) = kOne;
    return matrix;
}

// 00481510. The first input float is latched (FLD/FSTP into the argument
// slot) and reused as min x.
void box_corners_from_bounds_00481510(Object box, const float* p) {
    native_at<std::uint32_t>(box, 0x6C) = 0;
    native_at<std::uint32_t>(box, 0x68) = 0;
    native_at<std::uint32_t>(box, 0x64) = 0;
    native_at<std::uint32_t>(box, 0x60) = 0;
    const float x0 = p[0];
    const float s0 = f_add(p[3], x0);
    const float s1 = f_add(p[4], p[1]);
    const float s2 = f_add(p[5], p[2]);
    native_at<float>(box, 0x00) = x0;
    const float c0 = static_cast<float>(static_cast<double>(s0) * kHalf);
    const float c1 = static_cast<float>(static_cast<double>(s1) * kHalf);
    const float c2 = static_cast<float>(kHalf * static_cast<double>(s2));
    native_at<float>(box, 0x04) = p[1];
    native_at<float>(box, 0x08) = p[2];
    native_at<float>(box, 0x0C) = c0;
    native_at<float>(box, 0x10) = c1;
    native_at<float>(box, 0x14) = c2;
    native_at<float>(box, 0x18) = p[0];
    native_at<float>(box, 0x20) = c2;
    native_at<float>(box, 0x1C) = 0.0f;
    native_at<float>(box, 0x24) = c0;
    native_at<float>(box, 0x28) = 0.0f;
    native_at<float>(box, 0x2C) = p[5];
    native_at<float>(box, 0x30) = c0;
    native_at<float>(box, 0x34) = 0.0f;
    native_at<float>(box, 0x38) = p[2];
    native_at<float>(box, 0x40) = 0.0f;
    native_at<float>(box, 0x3C) = p[3];
    native_at<float>(box, 0x44) = c2;
    native_at<float>(box, 0x48) = c0;
    native_at<float>(box, 0x4C) = c1;
    native_at<float>(box, 0x50) = c2;
    native_at<float>(box, 0x54) = p[3];
    native_at<float>(box, 0x58) = p[4];
    native_at<float>(box, 0x5C) = p[5];
}

// ---------------------------------------------------------------------------
// Property bag iterator.
// ---------------------------------------------------------------------------

// 0047E480. The chain link is node+0Ch; on its end the bucket index is
// advanced to the next non-empty bucket below 40h.
bool property_bag_iterator_next_0047e480(Object it) {
    Object node = native_at<Object>(it, 0x04);
    if (node == nullptr) return native_at<Object>(it, 0x04) != nullptr;
    Object next = native_at<Object>(node, 0x0C);
    if (next == nullptr) {
        native_at<std::uint32_t>(it, 0x08) += 1;
        std::uint32_t bucket = native_at<std::uint32_t>(it, 0x08);
        if (bucket < 0x40) {
            Object bag = native_at<Object>(it, 0x00);
            while (native_at<Object>(bag, 0x08 + bucket * 4) == nullptr) {
                bucket += 1;
                native_at<std::uint32_t>(it, 0x08) = bucket;
                if (bucket >= 0x40) break;
            }
        }
        if (native_at<std::uint32_t>(it, 0x08) >= 0x40) {
            native_at<Object>(it, 0x04) = nullptr;
            return native_at<Object>(it, 0x04) != nullptr;
        }
        next = native_at<Object>(native_at<Object>(it, 0x00), 0x08 + native_at<std::uint32_t>(it, 0x08) * 4);
    }
    native_at<Object>(it, 0x04) = next;
    return native_at<Object>(it, 0x04) != nullptr;
}

// 00480690. With a non-empty bag (+4 count): bucket = 0, then while the
// bucket is empty and the index is below 40h, step and test the next bucket.
// The test reads bucket[40h] (bag+108h) when every bucket is empty, as the
// image does, and the node is then that dword.
void property_bag_iterator_begin_00480690(Object it) {
    Object bag = native_at<Object>(it, 0x00);
    if (native_at<std::uint32_t>(bag, 0x04) == 0) {
        native_at<Object>(it, 0x04) = nullptr;
        return;
    }
    native_at<std::uint32_t>(it, 0x08) = 0;
    if (native_at<Object>(bag, 0x08) == nullptr) {
        while (native_at<std::uint32_t>(it, 0x08) < 0x40) {
            const std::uint32_t bucket = native_at<std::uint32_t>(it, 0x08) + 1;
            native_at<std::uint32_t>(it, 0x08) = bucket;
            if (native_at<Object>(bag, 0x08 + bucket * 4) != nullptr) break;
        }
    }
    native_at<Object>(it, 0x04) = native_at<Object>(bag, 0x08 + native_at<std::uint32_t>(it, 0x08) * 4);
}

// 00484D20.
const char* property_bag_iterator_key_00484d20(Object it) {
    const char* key = native_at<const char*>(native_at<Object>(it, 0x04), 0x04);
    return key != nullptr ? key : "";  // 00E186ED, a .bss byte (zero at load)
}

// 00487040 __fastcall(ECX owner), RET: new(0Ch); {owner+4, ...} positioned by
// 00480690; null when the allocation fails.
Object property_bag_create_iterator_00487040(Seg7Host& host, Object owner) {
    Object it = host.operator_new_00bf681b(0x0C);
    if (it == nullptr) return nullptr;
    native_at<Object>(it, 0x00) = owner + 4;
    property_bag_iterator_begin_00480690(it);
    return it;
}

// 0048F5D0.
float property_bag_get_float_or_default_0048f5d0(Seg7Host& host, Object bag, const char* name,
                                                 float fallback) {
    if (!property_bag_contains_0048e9f0(host, bag, name)) return fallback;
    Object record = host.property_bag_find_008f2260(bag, name);
    if (native_at<std::int32_t>(record, 0x04) == 1) return native_at<float>(record, 0x0C);
    return static_cast<float>(native_at<std::int32_t>(record, 0x0C));
}

// ---------------------------------------------------------------------------
// Scene entities, points and proxies.
// ---------------------------------------------------------------------------

// 004802F0 __fastcall(ECX effect): 00867B10, then +9 = 1.
void point_effect_stop_and_mark_004802f0(Seg7Host& host, Object effect) {
    host.point_effect_stop_children_00867b10(effect);
    native_at<std::uint8_t>(effect, 0x09) = 1;
}

// 00480810 __thiscall(array, index, const float* value), RET 8: the element
// from 007AD030(index) takes three FLD/FSTP floats.
void point3_array_set_at_00480810(Seg7Host& host, Object array, std::uint32_t index, const float* value) {
    float* element = host.point3_array_at_007ad030(array, index);
    element[0] = value[0];
    element[1] = value[1];
    element[2] = value[2];
}

// 00480860: as 00480810 after array+18h = 0 (the byte is cleared before the
// call).
void point3_array_set_at_dirty_00480860(Seg7Host& host, Object array, std::uint32_t index,
                                        const float* value) {
    native_at<std::uint8_t>(array, 0x18) = 0;
    float* element = host.point3_array_at_007ad030(array, index);
    element[0] = value[0];
    element[1] = value[1];
    element[2] = value[2];
}

// 004808A0 / 004808B0: +18h = 0, then a tail jump into 007B2850 (RET 8) /
// 007B1B40 (RET 4) with the caller's arguments.
void point3ptr_array_insert_at_dirty_004808a0(Seg7Host& host, Object array, std::int32_t index, Object value) {
    native_at<std::uint8_t>(array, 0x18) = 0;
    host.point3ptr_array_insert_at_007b2850(array, index, value);
}
void point3ptr_array_erase_at_dirty_004808b0(Seg7Host& host, Object array, std::uint32_t index) {
    native_at<std::uint8_t>(array, 0x18) = 0;
    host.point3ptr_array_erase_at_007b1b40(array, index);
}

// 004808F0 (no ECX use), RET: new(20h) and eight 0.0f stores, or null.
Object scene_path_record_create_004808f0(Seg7Host& host) {
    Object record = host.operator_new_00bf681b(0x20);
    if (record == nullptr) return nullptr;
    for (std::uint32_t offset = 0; offset < 0x20; offset += 4) native_at<float>(record, offset) = 0.0f;
    return record;
}

// 00480960 __fastcall: AX = word [[source+14h]+174h] (the upper half of EAX
// keeps the pointer's upper half; callers use the word).
std::uint16_t scene_path_source_word174_00480960(Object source) {
    return native_at<std::uint16_t>(native_at<Object>(source, 0x14), 0x174);
}

// 00480BA0 __fastcall(entity): +174h vtable+8(entity) when set; +178h
// vtable+4(1) and +178h = 0 when set; tail jump to 00923040.
void scene_entity_on_destroyed_release_links_00480ba0(Seg7Host& host, Object entity) {
    if (native_at<Object>(entity, 0x174) != nullptr) host.notify_vcall_08(native_at<Object>(entity, 0x174), entity);
    Object owned = native_at<Object>(entity, 0x178);
    if (owned != nullptr) {
        host.delete_vcall_04(owned, 1);
        native_at<Object>(entity, 0x178) = nullptr;
    }
    host.scene_entity_destroyed_base_00923040(entity);
}

// 00480BF0: the effect +180h (NEG/SBB/TEST against the non-zero 00E186EC, so
// a plain null test) gets 00867B10 and +9 = 1.
void scene_entity_stop_blast_effect_00480bf0(Seg7Host& host, Object entity) {
    Object effect = native_at<Object>(entity, 0x180);
    if (effect == nullptr) return;
    host.point_effect_stop_children_00867b10(effect);
    native_at<std::uint8_t>(effect, 0x09) = 1;
}

// 00480C20 __thiscall(entity, float), RET 4: unless [00E188A8]+1FE4h is 2,
// +DCh vtable+1ACh(value) when +DCh is set.
void scene_entity_forward_blast_notify_00480c20(Seg7Host& host, Object entity, float value) {
    if (native_at<std::int32_t>(host.game_00e188a8(), 0x1FE4) == 2) return;
    if (native_at<Object>(entity, 0xDC) == nullptr) return;
    host.blast_vcall_1ac(native_at<Object>(entity, 0xDC), value);
}

// 00481090 __fastcall(check), RET. Fade +1Ch and the AABB test of +20h.
//   d = watched - camera ([00E188A8]+1ED4h), poses refreshed first;
//   total = float((dx*dx + dy*dy) + dz*dz) on the x87 stack;
//   r = float(300.0 (double 00CE3CA8) / [[00E188A8]+19FCh]+1C4h); r2 = float(r*r);
//   total <= r2 (or unordered): alpha = float(dt*0.5 + alpha), capped at 1.0f;
//   otherwise alpha = float(alpha - dt*0.5), floored at 0.0f and stored; at
//   zero, a set +3Ch returns false, else alpha = 0.0001f (00CE3C68).
// dt is [00E188A8]+21ECh and 0.5 the double 00D7A280.
bool entity_view_check_with_fade_00481090(Seg7Host& host, Object check) {
    Object game = host.game_00e188a8();
    Object camera = native_at<Object>(game, 0x1ED4);
    if (camera == nullptr) return false;
    if (native_at<std::uint8_t>(camera, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(camera);
        game = host.game_00e188a8();
    }
    Object watched = native_at<Object>(check, 0x20);
    if (native_at<std::uint8_t>(watched, 0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(watched);
        game = host.game_00e188a8();
    }
    const float dx = f_sub(native_at<float>(watched, 0xFC), native_at<float>(camera, 0xFC));
    const float dy = f_sub(native_at<float>(watched, 0x100), native_at<float>(camera, 0x100));
    const float dz = f_sub(native_at<float>(watched, 0x104), native_at<float>(camera, 0x104));
    const float r = static_cast<float>(300.0 / static_cast<double>(native_at<float>(native_at<Object>(game, 0x19FC), 0x1C4)));
    const double sum_xy = static_cast<double>(dy) * dy + static_cast<double>(dx) * dx;
    const float total = static_cast<float>(sum_xy + static_cast<double>(dz) * dz);
    const float r2 = static_cast<float>(static_cast<double>(r) * r);
    const double dt = static_cast<double>(native_at<float>(game, 0x21EC));
    if (!(total > r2)) {
        float alpha = static_cast<float>(dt * kHalf + static_cast<double>(native_at<float>(check, 0x1C)));
        if (alpha > 1.0f) alpha = kOne;
        native_at<float>(check, 0x1C) = alpha;
    } else {
        float alpha = static_cast<float>(static_cast<double>(native_at<float>(check, 0x1C)) - dt * kHalf);
        if (0.0f > alpha) alpha = 0.0f;
        native_at<float>(check, 0x1C) = alpha;
        // COMISS 0, alpha; JC: a positive or unordered alpha goes on to the
        // AABB test.
        if (alpha <= 0.0f) {
            if (native_at<std::uint8_t>(check, 0x3C) != 0) return false;
            native_at<float>(check, 0x1C) = 9.99999974737875e-05f;  // 00CE3C68
        }
    }
    return entity_aabb_in_view(host, check, 0x20);
}

// 004812F0 __fastcall(check), RET: unless +180h is set, the class AABB of the
// entity +14Ch; otherwise the stored box at +19Ch. Returns 1 when in view.
bool entity_view_check_004812f0(Seg7Host& host, Object check) {
    if (native_at<std::uint8_t>(check, 0x180) == 0) return entity_aabb_in_view(host, check, 0x14C);
    return host.game_is_aabb_in_view_004b48e0(host.game_00e188a8(), reinterpret_cast<const float*>(check + 0x19C));
}

namespace {
// A std::vector<Object> {proxy +0, first +4, last +8, end +0Ch}: the range
// test the image makes before an indexed access (a failure calls 00BF6713,
// which does not return in the image; the access then proceeds as written).
void check_index(Seg7Host& host, Object vector, std::uint32_t index, std::uint32_t shift) {
    Object first = native_at<Object>(vector, 0x04);
    if (first == nullptr ||
        index >= (static_cast<std::uint32_t>(native_at<Object>(vector, 0x08) - first) >> shift)) {
        host.invalid_parameter_00bf6713();
    }
}

// The kill-and-clear of one child pointer, shared by 00481430 and 00484A10.
void reset_child(Seg7Host& host, Object vector, std::uint32_t index) {
    check_index(host, vector, index, 2);
    Object child = native_at<Object*>(vector, 0x04)[index];
    if (child != nullptr) kill_entity(host, child);
    check_index(host, vector, index, 2);
    native_at<Object*>(vector, 0x04)[index] = nullptr;
}
} // namespace

// 00481430 __thiscall(entity, layer, index), RET 8: the layer vector is
// entity+18Ch+layer*10h; child vtable+114h(0) and 00926D90(2), then the slot
// is cleared. Each indexed access is range-checked (00BF6713).
void scene_entity_layer_reset_child_00481430(Seg7Host& host, Object entity, std::int32_t layer,
                                             std::uint32_t index) {
    reset_child(host, entity + 0x18C + static_cast<std::uint32_t>(layer) * 0x10, index);
}

// 004814C0 __fastcall(entity), RET: 00481430 for every child of each of the
// four layer vectors.
void scene_entity_layers_reset_all_004814c0(Seg7Host& host, Object entity) {
    for (std::int32_t layer = 0; layer < 4; ++layer) {
        Object vector = entity + 0x18C + static_cast<std::uint32_t>(layer) * 0x10;
        for (std::uint32_t i = 0;; ++i) {
            Object first = native_at<Object>(vector, 0x04);
            if (first == nullptr || i >= (static_cast<std::uint32_t>(native_at<Object>(vector, 0x08) - first) >> 2)) break;
            scene_entity_layer_reset_child_00481430(host, entity, layer, i);
        }
    }
}

// 00484A10 __thiscall(tree, i, j), RET 8: the cell vector is element i of
// the 10h-byte vector at tree+4Ch/+50h (checked), then as 00481430.
void quad_cell_reset_00484a10(Seg7Host& host, Object tree, std::uint32_t i, std::uint32_t j) {
    auto row = [&]() {
        Object first = native_at<Object>(tree, 0x4C);
        if (first == nullptr || i >= (static_cast<std::uint32_t>(native_at<Object>(tree, 0x50) - first) >> 4)) {
            host.invalid_parameter_00bf6713();
        }
        return native_at<Object>(tree, 0x4C) + i * 0x10;
    };
    Object cells = row();
    check_index(host, cells, j, 2);
    Object child = native_at<Object*>(cells, 0x04)[j];
    if (child != nullptr) kill_entity(host, child);
    cells = row();
    check_index(host, cells, j, 2);
    native_at<Object*>(cells, 0x04)[j] = nullptr;
}

// 00484AE0 __thiscall(tree, Node** slot, const float* box), RET 8. When the
// minimum cell size tree+7Ch is above the box width (box[3] - box[0] on the
// x87 stack, FCOMIP), every cell in the 0047BA40 range is reset with 00484A10
// (i outer, j inner, signed and inclusive). Otherwise (including unordered)
// the node *slot recurses into its four quadrants (child slots +60h..+6Ch
// with boxes +0, +18h, +30h, +48h) and is then freed (00BF65AC) and cleared.
void quad_tree_prune_00484ae0(Seg7Host& host, Object tree, Object slot, const float* box) {
    const double width = static_cast<double>(box[3]) - static_cast<double>(box[0]);
    if (static_cast<double>(native_at<float>(tree, 0x7C)) > width) {
        std::int32_t i0 = 0, i1 = 0, j0 = 0, j1 = 0;
        host.quad_cell_range_0047ba40(tree, box, &i0, &i1, &j0, &j1);
        for (std::int32_t i = i0; i <= i1; ++i) {
            for (std::int32_t j = j0; j <= j1; ++j) {
                quad_cell_reset_00484a10(host, tree, static_cast<std::uint32_t>(i), static_cast<std::uint32_t>(j));
            }
        }
        return;
    }
    Object node = native_at<Object>(slot, 0);
    if (node == nullptr) return;
    quad_tree_prune_00484ae0(host, tree, node + 0x60, reinterpret_cast<const float*>(node + 0x00));
    node = native_at<Object>(slot, 0);
    quad_tree_prune_00484ae0(host, tree, node + 0x64, reinterpret_cast<const float*>(node + 0x18));
    node = native_at<Object>(slot, 0);
    quad_tree_prune_00484ae0(host, tree, node + 0x68, reinterpret_cast<const float*>(node + 0x30));
    node = native_at<Object>(slot, 0);
    quad_tree_prune_00484ae0(host, tree, node + 0x6C, reinterpret_cast<const float*>(node + 0x48));
    node = native_at<Object>(slot, 0);
    if (node == nullptr) return;
    host.crt_free_00bf65ac(node);
    native_at<Object>(slot, 0) = nullptr;
}

// 004879B0 __thiscall(tree, Node** slot, const float* extent), RET 8: box =
// {extent[0..2] - centre, extent[3..5] + centre} with the centre at
// tree+70h..+78h; in view -> 00498C40(tree, slot, extent), else 00484AE0.
void traffic_bounds_update_004879b0(Seg7Host& host, Object tree, Object slot, const float* p) {
    const float box[6] = {f_sub(p[0], native_at<float>(tree, 0x70)), f_sub(p[1], native_at<float>(tree, 0x74)),
                          f_sub(p[2], native_at<float>(tree, 0x78)), f_add(p[3], native_at<float>(tree, 0x70)),
                          f_add(native_at<float>(tree, 0x74), p[4]), f_add(native_at<float>(tree, 0x78), p[5])};
    if (host.game_is_aabb_in_view_004b48e0(host.game_00e188a8(), box)) {
        host.traffic_visible_cells_00498c40(tree, slot, p);
        return;
    }
    quad_tree_prune_00484ae0(host, tree, slot, p);
}

// 0048C900 __thiscall(proxy, unused), RET 4: 004879B0(proxy, proxy+80h,
// proxy+58h).
void traffic_proxy_update_0048c900(Seg7Host& host, Object proxy, std::uint32_t) {
    traffic_bounds_update_004879b0(host, proxy, proxy + 0x80, reinterpret_cast<const float*>(proxy + 0x58));
}

// 004847A0 __fastcall(entity): 00B6DA70(ECX [[+178h]+160h]+0Ch, 0.0f, 0),
// then 00922F80(entity, 0) when +5Ch is set.
void scene_node_hide_and_maybe_disable_004847a0(Seg7Host& host, Object entity) {
    Object node = native_at<Object>(native_at<Object>(native_at<Object>(entity, 0x178), 0x160), 0x0C);
    host.scene_node_set_visibility_factor_00b6da70(node, 0.0f, 0);
    if (native_at<std::uint8_t>(entity, 0x5C) != 0) host.scene_node_disable_00922f80(entity, 0);
}

// 004847D0 __fastcall(entity), RET. new(1ACh); with memory:
// 007135C0(memory, entity, [[entity+170h]+10h] vtable+8(0, 1.0f)); +178h =
// the result (or null). Then the visual's node is parented:
//   +3Ch answers IsKindOf(44h): parent = [+3Ch]+3D0h vtable+40h();
//   else +3Ch answers IsKindOf(4): walk +3Ch while the holder's
//     [+360h]+160h is unset and its +3Ch answers IsKindOf(4); parent = that
//     holder's [[+360h]+160h]+0Ch (a null +360h reads +0Ch of null);
//   else 00B6D890(root [00E188A8]+19ECh).
void anim_class_create_instance_004847d0(Seg7Host& host, Object entity) {
    Object memory = host.operator_new_00bf681b(0x1AC);
    Object instance = nullptr;
    if (memory != nullptr) {
        Object visual = host.class_instantiate_vcall_08(native_at<Object>(native_at<Object>(entity, 0x170), 0x10), 0, kOne);
        instance = host.unit_part_instance_construct_007135c0(memory, entity, visual);
    }
    native_at<Object>(entity, 0x178) = instance;
    auto node = [&]() {
        return native_at<Object>(native_at<Object>(native_at<Object>(entity, 0x178), 0x160), 0x0C);
    };
    Object owner = native_at<Object>(entity, 0x3C);
    if (owner != nullptr && host.is_kind_vcall_5c(owner, 0x44)) {
        Object node_now = node();
        Object parent = host.parent_node_vcall_40(native_at<Object>(native_at<Object>(entity, 0x3C), 0x3D0));
        host.node_set_parent_00b6e680(node_now, parent);
        return;
    }
    owner = native_at<Object>(entity, 0x3C);
    if (owner != nullptr && host.is_kind_vcall_5c(owner, 4)) {
        Object holder = native_at<Object>(entity, 0x3C);
        for (;;) {
            Object part = native_at<Object>(holder, 0x360);
            if (part != nullptr && native_at<Object>(part, 0x160) != nullptr) break;
            Object up = native_at<Object>(holder, 0x3C);
            if (up == nullptr || !host.is_kind_vcall_5c(up, 4)) break;
            holder = native_at<Object>(holder, 0x3C);
        }
        Object part = native_at<Object>(holder, 0x360);
        Object visual = part != nullptr ? native_at<Object>(part, 0x160) : nullptr;
        host.node_set_parent_00b6e680(node(), native_at<Object>(visual, 0x0C));
        return;
    }
    host.node_propagate_root_registration_00b6d890(node(), native_at<Object>(host.game_00e188a8(), 0x19EC));
}

// 00484940 __fastcall(entity), RET: with +178h and its node [[+178h]+160h]
// +0Ch: node vtable+38h(entity+74h); then 00B6DA70(node, 1.0f, 1) when +174h
// is null, 00B6DA70(node, [+174h]+1Ch, 1) when +184h is clear, else nothing.
void scene_entity_refresh_visibility_factor_00484940(Seg7Host& host, Object entity) {
    Object instance = native_at<Object>(entity, 0x178);
    if (instance == nullptr) return;
    Object node = native_at<Object>(native_at<Object>(instance, 0x160), 0x0C);
    if (node == nullptr) return;
    host.node_update_vcall_38(node, entity + 0x74);
    Object fader = native_at<Object>(entity, 0x174);
    auto current_node = [&]() {
        return native_at<Object>(native_at<Object>(native_at<Object>(entity, 0x178), 0x160), 0x0C);
    };
    if (fader == nullptr) {
        host.scene_node_set_visibility_factor_00b6da70(current_node(), kOne, 1);
        return;
    }
    if (native_at<std::uint8_t>(entity, 0x184) != 0) return;
    host.scene_node_set_visibility_factor_00b6da70(current_node(), native_at<float>(fader, 0x1C), 1);
}

// 00484BC0 __thiscall(proxy, unused), RET 4. Box proxy+68h in view: create
// the object on first use (+20h vtable+8 -> +24h; vtable+98h(0,
// [00E188A8]+19CCh, &identity), vtable+114h(proxy), vtable+A0h(),
// 004134F0(object+74h, proxy+28h)), then vtable+D8h(). Out of view: kill
// +24h (vtable+114h(0), 00926D90(2)) and clear it.
void scene_proxy_update_when_visible_00484bc0(Seg7Host& host, Object proxy, std::uint32_t) {
    if (host.game_is_aabb_in_view_004b48e0(host.game_00e188a8(), reinterpret_cast<const float*>(proxy + 0x68))) {
        if (native_at<Object>(proxy, 0x24) == nullptr) {
            Object object = host.create_vcall_08(native_at<Object>(proxy, 0x20));
            native_at<Object>(proxy, 0x24) = object;
            const float identity[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne};
            host.set_pose_vcall_98(object, 0, native_at<Object>(host.game_00e188a8(), 0x19CC), identity);
            host.attach_vcall_114(native_at<Object>(proxy, 0x24), proxy);
            host.vcall_a0(native_at<Object>(proxy, 0x24));
            host.matrix_copy_004134f0(native_at<Object>(proxy, 0x24) + 0x74, proxy + 0x28);
        }
        host.vcall_d8(native_at<Object>(proxy, 0x24));
        return;
    }
    Object object = native_at<Object>(proxy, 0x24);
    if (object != nullptr) kill_entity(host, object);
    native_at<Object>(proxy, 0x24) = nullptr;
}

// 0048C920 __fastcall(proxy), RET: vtable 00CE6660; kill +24h when set;
// +24h = 0; 00489DD0 (the base destructor).
void scene_proxy_destruct_0048c920(Seg7Host& host, Object proxy) {
    native_at<std::uint32_t>(proxy, 0x00) = 0x00CE6660u;
    Object object = native_at<Object>(proxy, 0x24);
    if (object != nullptr) kill_entity(host, object);
    native_at<Object>(proxy, 0x24) = nullptr;
    host.scene_proxy_base_destroy_00489dd0(proxy);
}

// 00487230 __fastcall(path), RET: 00928570, then the first node of the list
// [path+30h]+36Ch (head pointer +370h, next +4) whose +8 is the path is
// erased with 004837D0(list, node).
void path_unregister_from_world_lists_00487230(Seg7Host& host, Object path) {
    host.path_unregister_base_00928570(path);
    Object world = native_at<Object>(path, 0x30);
    for (Object node = native_at<Object>(world, 0x370); node != nullptr; node = native_at<Object>(node, 0x04)) {
        if (native_at<Object>(node, 0x08) == path) {
            host.unit_list_erase_node_004837d0(world + 0x36C, node);
            return;
        }
    }
}

// 00487940 __fastcall(group), RET: when +94h is set, clear it and kill and
// clear each member i < [+24h] of the vector +74h (range-checked).
void traffic_group_flush_pending_visibility_00487940(Seg7Host& host, Object group) {
    if (native_at<std::uint8_t>(group, 0x94) == 0) return;
    native_at<std::uint8_t>(group, 0x94) = 0;
    for (std::int32_t i = 0; i < native_at<std::int32_t>(group, 0x24); ++i) {
        check_index(host, group + 0x70, static_cast<std::uint32_t>(i), 2);
        Object* slot = native_at<Object*>(group, 0x74) + i;
        if (*slot != nullptr) {
            host.attach_vcall_114(*slot, nullptr);
            host.mission_entity_kill_00926d90(*slot, 2);
            *slot = nullptr;
        }
    }
}

// 004897B0 __thiscall(node, matrix), RET 4: 004134F0(node+74h, matrix);
// +C8h = 0, +10Ch = 0; 0042ED50 on each child (+48h, next +44h).
void scene_node_set_matrix_and_invalidate_004897b0(Seg7Host& host, Object node, Object matrix) {
    host.matrix_copy_004134f0(node + 0x74, matrix);
    native_at<std::uint8_t>(node, 0xC8) = 0;
    native_at<std::uint8_t>(node, 0x10C) = 0;
    for (Object child = native_at<Object>(node, 0x48); child != nullptr; child = native_at<Object>(child, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);
    }
}

// 0048CC60 __fastcall(traffic), RET (AL): flips the byte 00E081C0; when it
// turns off, walks the std::map at traffic+28h (head +2Ch) with a stack
// iterator {container, node} and calls 0048C090 on each node's value (+10h),
// advancing with 00427FE0. Returns the new byte.
std::uint8_t traffic_toggle_debug_and_refresh_0048cc60(Seg7Host& host, Object traffic) {
    const std::uint8_t on = host.traffic_debug_00e081c0() == 0 ? 1 : 0;
    host.set_traffic_debug_00e081c0(on);
    if (on != 0) return on;
    alignas(4) std::uint8_t iterator[8];
    native_at<Object>(iterator, 0) = traffic + 0x28;
    native_at<Object>(iterator, 4) = native_at<Object>(native_at<Object>(traffic, 0x2C), 0);
    while (native_at<Object>(iterator, 4) != native_at<Object>(traffic, 0x2C)) {
        host.landing_troop_kill_all_member_entities_0048c090(native_at<Object>(native_at<Object>(iterator, 4), 0x10));
        host.tree_iterator_increment_00427fe0(iterator);
    }
    return host.traffic_debug_00e081c0();
}

// 0048D6C0 __thiscall(entity, const char* name), RET 4: 0041DD40(entity+154h,
// strlen(name) or 0 for null, flag 0), then memcpy(data, name, size) when
// the data (+158h) is non-null.
void scene_entity_set_name_0048d6c0(Seg7Host& host, Object entity, const char* name) {
    const std::uint32_t length = name != nullptr ? static_cast<std::uint32_t>(std::strlen(name)) : 0;
    host.native_string_resize_0041dd40(entity + 0x154, length, 0);
    void* data = native_at<void*>(entity, 0x158);
    if (data != nullptr) host.crt_memcpy_00bf7680(data, name, native_at<std::uint32_t>(entity, 0x154));
}

// ---------------------------------------------------------------------------
// Pointer arrays and observer slots.
// ---------------------------------------------------------------------------

namespace {
using ReserveFn = void (*)(Seg7Host&, Object, std::int32_t);

// 00482510 / 00482570 (byte-identical): capacity at least 1; when it grows,
// new(capacity*4) through 00BF55BE, copy count dwords (each store skipped
// when the new block is null), free the old block (00BF6989).
void reserve(Seg7Host& host, Object array, std::int32_t capacity) {
    if (capacity < 1) capacity = 1;
    if (native_at<std::int32_t>(array, 0x08) >= capacity) return;
    Object block = host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity) * 4);
    Object out = block;
    for (std::int32_t i = 0; i < native_at<std::int32_t>(array, 0x04); ++i, out += 4) {
        if (out != nullptr) native_at<std::uint32_t>(out, 0) = native_at<std::uint32_t*>(array, 0x00)[i];
    }
    host.crt_free_00bf6989(native_at<void*>(array, 0x00));
    native_at<Object>(array, 0x00) = block;
    native_at<std::int32_t>(array, 0x08) = capacity;
}

// The push-backs (byte-identical apart from which reserve they call): grow
// to max(2*capacity, 1) when full, store unless the slot address is null,
// count + 1.
void push_back(Seg7Host& host, Object array, const std::uint32_t* value, ReserveFn grow) {
    if (native_at<std::int32_t>(array, 0x04) == native_at<std::int32_t>(array, 0x08)) {
        std::int32_t capacity = native_at<std::int32_t>(array, 0x08) * 2;
        if (capacity <= 1) capacity = 1;
        grow(host, array, capacity);
    }
    std::uint32_t* slot = native_at<std::uint32_t*>(array, 0x00) + native_at<std::int32_t>(array, 0x04);
    if (slot != nullptr) *slot = *value;
    native_at<std::int32_t>(array, 0x04) += 1;
}

// The resizes: reserve when count exceeds the capacity, zero-fill new slots
// (null slot addresses skipped), step the count down one at a time while it
// is above the target, then store the target.
void resize(Seg7Host& host, Object array, std::int32_t count, ReserveFn grow) {
    if (count > native_at<std::int32_t>(array, 0x08)) grow(host, array, count);
    for (std::int32_t i = native_at<std::int32_t>(array, 0x04); i < count; ++i) {
        std::uint32_t* slot = native_at<std::uint32_t*>(array, 0x00) + i;
        if (slot != nullptr) *slot = 0;
    }
    while (count < native_at<std::int32_t>(array, 0x04)) native_at<std::int32_t>(array, 0x04) -= 1;
    native_at<std::int32_t>(array, 0x04) = count;
}

void observer_set(Seg7Host& host, Object slot, Object subject) {
    Object current = native_at<Object>(slot, 0x14);
    if (current == subject) return;
    if (current != nullptr) host.observer_unregister_pair_006952a0(current, slot);
    native_at<Object>(slot, 0x14) = subject;
    if (subject != nullptr) host.observer_register_pair_00694a60(subject, slot);
}

void observer_release(Seg7Host& host, Object slot, Object subject) {
    const std::uint32_t id = host.subject_id_vcall_04(subject);
    if (id != native_at<std::uint32_t>(slot, 0x14)) return;
    native_at<std::uint32_t>(slot, 0x14) = 0;
    host.observer_unregister_pair_006952a0(reinterpret_cast<Object>(static_cast<std::uintptr_t>(id)), slot);
}

Object copy_range(Seg7Host& host, Object first, Object last, Object dest) {
    const std::int32_t count = static_cast<std::int32_t>(last - first) >> 2;
    const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4;
    if (count != 0) host.crt_memmove_s_00bf67a7(dest, bytes, first, bytes);
    return dest + bytes;
}
} // namespace

void pod_pointer_array_reserve_00482510(Seg7Host& host, Object array, std::int32_t capacity) { reserve(host, array, capacity); }
void pod_pointer_array_reserve_00482570(Seg7Host& host, Object array, std::int32_t capacity) { reserve(host, array, capacity); }
void pod_pointer_array_push_back_00485390(Seg7Host& host, Object array, const std::uint32_t* value) {
    push_back(host, array, value, pod_pointer_array_reserve_00482510);
}
void pod_pointer_array_push_back_004853d0(Seg7Host& host, Object array, const std::uint32_t* value) {
    push_back(host, array, value, pod_pointer_array_reserve_00482570);
}
void pod_pointer_array_push_back_004883b0(Seg7Host& host, Object array, const std::uint32_t* value) {
    push_back(host, array, value, pod_pointer_array_reserve_00482510);
}
void pod_pointer_array_push_back_004883f0(Seg7Host& host, Object array, const std::uint32_t* value) {
    push_back(host, array, value, pod_pointer_array_reserve_00482570);
}
void pod_pointer_array_resize_00485990(Seg7Host& host, Object array, std::int32_t count) {
    resize(host, array, count, pod_pointer_array_reserve_00482510);
}
void pod_pointer_array_resize_004859e0(Seg7Host& host, Object array, std::int32_t count) {
    resize(host, array, count, pod_pointer_array_reserve_00482570);
}
// 0048CE30 / 0048CE50: resize to 0, then free the block (00BF6989).
void pod_pointer_array_tidy_0048ce30(Seg7Host& host, Object array) {
    pod_pointer_array_resize_00485990(host, array, 0);
    host.crt_free_00bf6989(native_at<void*>(array, 0x00));
}
void pod_pointer_array_tidy_0048ce50(Seg7Host& host, Object array) {
    pod_pointer_array_resize_004859e0(host, array, 0);
    host.crt_free_00bf6989(native_at<void*>(array, 0x00));
}

// 00483910 (no return value) / 00486B10 / 004893F0 (return the slot):
// unregister the old subject (006952A0, ECX subject, EDX slot), store,
// register the new one (00694A60). RET 4.
void observer_slot_set_00483910(Seg7Host& host, Object slot, Object subject) { observer_set(host, slot, subject); }
Object observer_slot_set_00486b10(Seg7Host& host, Object slot, Object subject) {
    observer_set(host, slot, subject);
    return slot;
}
Object observer_slot_set_004893f0(Seg7Host& host, Object slot, Object subject) {
    observer_set(host, slot, subject);
    return slot;
}

// 004884F0 / 00488520: id = subject vtable+4(); when it equals +14h, clear
// +14h and 006952A0(ECX id, EDX slot). RET 4.
void observer_slot_release_if_subject_004884f0(Seg7Host& host, Object slot, Object subject) {
    observer_release(host, slot, subject);
}
void observer_slot_release_if_subject_00488520(Seg7Host& host, Object slot, Object subject) {
    observer_release(host, slot, subject);
}

Object pointer_vector_copy_range_00486f80(Seg7Host& host, Object first, Object last, Object dest) {
    return copy_range(host, first, last, dest);
}
Object pointer_vector_copy_range_0048d740(Seg7Host& host, Object first, Object last, Object dest) {
    return copy_range(host, first, last, dest);
}
Object pointer_vector_copy_range_0048d830(Seg7Host& host, Object first, Object last, Object dest) {
    return copy_range(host, first, last, dest);
}

// ---------------------------------------------------------------------------
// Strings, buffers, holders and bit flags.
// ---------------------------------------------------------------------------

// 0048CFC0 __fastcall(string): a non-null data (+4) goes back to the pool
// (00419CC0, then 00BD1510 with size + 1, 1).
void native_string_destroy_storage_0048cfc0(Seg7Host& host, Object string) {
    void* data = native_at<void*>(string, 0x04);
    if (data == nullptr) return;
    const std::uint32_t bytes = native_at<std::uint32_t>(string, 0x00) + 1;
    Object pool = host.sized_pool_singleton_00419cc0(data, bytes, 1);
    host.sized_pool_return_block_00bd1510(pool, data, bytes, 1);
}

// 0048E0A0 __fastcall(buffer): +0 = 0; free +8 (00BF65AC) when set; +8, +0Ch
// and +10h = 0.
void named_buffer_clear_0048e0a0(Seg7Host& host, Object buffer) {
    native_at<std::uint32_t>(buffer, 0x00) = 0;
    if (native_at<void*>(buffer, 0x08) != nullptr) host.crt_free_00bf65ac(native_at<void*>(buffer, 0x08));
    native_at<std::uint32_t>(buffer, 0x08) = 0;
    native_at<std::uint32_t>(buffer, 0x0C) = 0;
    native_at<std::uint32_t>(buffer, 0x10) = 0;
}

// 0048E3B0 __thiscall(dest, NativeString* source, const dword* third),
// RET 8: dest = {0, 0}; unless dest is source, 0041DD40(source size, 1) and
// memcpy of the new size for a non-empty source; dest+8 = *third. Returns dest.
Object native_string_pair_copy_construct_0048e3b0(Seg7Host& host, Object dest, Object source,
                                                  const std::uint32_t* third) {
    native_at<std::uint32_t>(dest, 0x00) = 0;
    native_at<void*>(dest, 0x04) = nullptr;
    if (dest != source) {
        host.native_string_resize_0041dd40(dest, native_at<std::uint32_t>(source, 0x00), 1);
        if (native_at<std::uint32_t>(source, 0x00) != 0) {
            host.crt_memcpy_00bf7680(native_at<void*>(dest, 0x04), native_at<void*>(source, 0x04),
                                     native_at<std::uint32_t>(dest, 0x00));
        }
    }
    native_at<std::uint32_t>(dest, 0x08) = *third;
    return dest;
}

// 0048E630 __fastcall(holder): the RefPtr box *holder: its object loses a
// reference (InterlockedDecrement import 00CE2220 of object+4; vtable slot 0
// at zero) and the box's pointer is cleared; the box is freed and *holder = 0.
void ref_counted_ptr_holder_delete_0048e630(Seg7Host& host, Object holder) {
    Object box = native_at<Object>(holder, 0x00);
    if (box == nullptr) return;
    Object object = native_at<Object>(box, 0x00);
    if (object != nullptr) {
        release_ref(host, object);
        native_at<Object>(box, 0x00) = nullptr;
    }
    host.crt_free_00bf65ac(box);
    native_at<Object>(holder, 0x00) = nullptr;
}

// 0048F200 __thiscall(owner, char on), RET 4: word = 0048E330(owner); bit
// 1 << (owner+8 & 1Fh) set or cleared. Returns owner.
Object bit_flag_set_flag_0048f200(Seg7Host& host, Object owner, std::uint8_t on) {
    std::uint32_t* word = host.bit_word_0048e330(owner);
    const std::uint32_t bit = 1u << (native_at<std::uint32_t>(owner, 0x08) & 0x1F);
    if (on != 0) *word |= bit;
    else *word &= ~bit;
    return owner;
}

// 0048F550 __thiscall(owner, source), RET 4: 0048F230(ECX source) decides
// between setting and clearing the same bit.
Object bit_flag_sync_flag_0048f550(Seg7Host& host, Object owner, Object source) {
    if (host.bit_source_0048f230(source)) {
        std::uint32_t* word = host.bit_word_0048e330(owner);
        *word |= 1u << (native_at<std::uint32_t>(owner, 0x08) & 0x1F);
        return owner;
    }
    std::uint32_t* word = host.bit_word_0048e330(owner);
    *word &= ~(1u << (native_at<std::uint32_t>(owner, 0x08) & 0x1F));
    return owner;
}

// ---------------------------------------------------------------------------
// Scene-node and class-entity bodies (batch 2).
// ---------------------------------------------------------------------------

// 00480930.
bool path_is_kind_of_00480930(Object object, std::int32_t id) {
    return id == 0x47 || id == 1 || id == 0 || id == native_at<std::int32_t>(object, 0xC4);
}

// 004845D0.
Object retained_reference_assign_004845d0(Seg7Host& host, Object dest, Object source) {
    Object incoming = native_at<Object>(source, 0);
    Object old = native_at<Object>(dest, 0);
    if (old == incoming) return dest;
    native_at<Object>(dest, 0) = incoming;
    if (incoming != nullptr) host.interlocked_increment_import_00ce221c(incoming + 4);
    if (old != nullptr) release_ref(host, old);
    return dest;
}

// 00487210.
void scene_node_attach_to_kind41_lists_00487210(Seg7Host& host, Object entity) {
    host.game_entity_register_in_parent_00928560(entity);
    host.unit_list_push_back_00484540(native_at<Object>(entity, 0x30) + 0x36C, entity);
}

// 00487270. The checked-iterator tests only repeat the loop condition.
void entity_list_update_all_00487270(Seg7Host& host, Object manager, float delta) {
    Object node = native_at<Object>(native_at<Object>(manager, 0x10), 0);
    while (node != native_at<Object>(manager, 0x10)) {
        Object current = node;
        node = native_at<Object>(node, 0);
        host.entity_update_vcall_04(native_at<Object>(current, 0x08), delta);
    }
}

// 00489760.
void scene_node_set_position_and_invalidate_00489760(Seg7Host& host, Object node, const float* position) {
    native_at<float>(node, 0xA4) = position[0];
    native_at<float>(node, 0xA8) = position[1];
    native_at<float>(node, 0xAC) = position[2];
    native_at<std::uint8_t>(node, 0xC8) = 0;
    native_at<std::uint8_t>(node, 0x10C) = 0;
    for (Object child = native_at<Object>(node, 0x48); child != nullptr; child = native_at<Object>(child, 0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);
    }
}

// 0048E9F0. The out slot handed to 0043B8B0 is the name argument's own stack
// slot.
bool property_bag_contains_0048e9f0(Seg7Host& host, Object bag, const char* name) {
    alignas(4) std::uint8_t key[8];
    host.native_string_assign_0041e870(key, name);
    alignas(4) std::uint8_t slot[4];
    std::memcpy(slot, &name, 4);
    const bool found = host.property_map_find_borrowed_key_node_0043b8b0(bag + 4, key, slot) != nullptr;
    native_string_destroy_storage_0048cfc0(host, key);
    return found;
}

// 0048DBD0.
void class_entity37_sentity_init_base_0048dbd0(Seg7Host& host, Object entity) {
    host.entity_inherit_race_and_activate_00923840(entity);
    const char* name = native_at<const char*>(native_at<Object>(entity, 0x170), 0x08);
    if (name == nullptr) name = "";  // 00E186ED, a .bss byte (zero at load)
    const std::uint32_t length = static_cast<std::uint32_t>(std::strlen(name));
    host.native_string_resize_0041dd40(entity + 0x154, length, 0);
    void* data = native_at<void*>(entity, 0x158);
    if (data != nullptr) host.crt_memcpy_00bf7680(data, name, native_at<std::uint32_t>(entity, 0x154));
    host.sentity_init_vcall_120(entity);
}

// 00487380. The null test of +180h is the NEG/SBB/TEST 00E186EC idiom.
void class_entity37_destruct_base_00487380(Seg7Host& host, Object entity) {
    native_at<std::uint32_t>(entity, 0x00) = 0x00CE6490u;
    native_at<std::uint32_t>(entity, 0x10) = 0x00CE6474u;
    native_at<std::uint32_t>(entity, 0x24) = 0x00CE646Cu;
    Object effect = native_at<Object>(entity, 0x180);
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        native_at<std::uint8_t>(effect, 0x09) = 1;
    }
    Object owned = native_at<Object>(entity, 0x178);
    if (owned != nullptr) {
        host.delete_vcall_04(owned, 1);
        native_at<Object>(entity, 0x178) = nullptr;
    }
    effect = native_at<Object>(entity, 0x180);
    if (effect != nullptr) {
        release_ref(host, effect);
        native_at<Object>(entity, 0x180) = nullptr;
    }
    host.entity_destruct_base_00925780(entity);
}

// 00489F30. hi uses FMUL ST(1),ST(0) (DC form, bytes checked) on the first
// singleton's clip; lo = FCHS(second clip) * 0.25 (FMULP). 0.25 is the double
// 00D7A348. The x87 result is stored with FSTP float.
Object class_entity37_construct_base_00489f30(Seg7Host& host, Object entity) {
    host.scene_node_construct_00925ce0(entity);
    native_at<std::uint32_t>(entity, 0x00) = 0x00CE6490u;
    native_at<std::uint32_t>(entity, 0x10) = 0x00CE6474u;
    native_at<std::uint32_t>(entity, 0x24) = 0x00CE646Cu;
    native_at<Object>(entity, 0x180) = nullptr;
    native_at<std::int32_t>(entity, 0xC4) = 0x37;
    Object first = soldier_globals_get_singleton_00485520(host);
    Object second = soldier_globals_get_singleton_00485520(host);
    constexpr double kQuarter = 0.25;  // 00D7A348
    const float hi = static_cast<float>(static_cast<double>(native_at<float>(first, 0x0C)) * kQuarter);
    const float lo = static_cast<float>(kQuarter * -static_cast<double>(native_at<float>(second, 0x0C)));
    native_at<float>(entity, 0x17C) = static_cast<float>(host.random_uniform_float_range_00bd2f10(nullptr, lo, hi));
    native_at<std::uint8_t>(entity, 0x184) = 0;
    return entity;
}

// 0047F200. The stack matrix is an identity with row 3 = node+A4h..+ACh;
// the angle is SUBSS -0.0f (00D7A208) - heading.
void scene_node_set_heading_0047f200(Seg7Host& host, Object node, float heading) {
    alignas(4) std::uint8_t translation[0x40];
    const float values[16] = {kOne, 0, 0, 0, 0, kOne, 0, 0, 0, 0, kOne, 0,
                              native_at<float>(node, 0xA4), native_at<float>(node, 0xA8),
                              native_at<float>(node, 0xAC), kOne};
    std::memcpy(translation, values, sizeof(values));
    const float angle = -0.0f - heading;
    alignas(4) std::uint8_t rotation_out[0x40];
    alignas(4) std::uint8_t rotation[0x40];
    alignas(4) std::uint8_t product[0x40];
    Object built = host.matrix_build_rotation_y_00b646e0(rotation_out, &angle);
    host.matrix_copy_004134f0(rotation, built);
    Object result = host.matrix_multiply_00413920(rotation, product, translation);
    host.matrix_copy_004134f0(node + 0x74, result);
}

// ---------------------------------------------------------------------------
// Soldier globals and soldier types.
// ---------------------------------------------------------------------------

// 00484700 __fastcall(globals), RET: vtable 00CE6458, +1Ch = 0, 0084CB70.
Object soldier_globals_construct_00484700(Seg7Host& host, Object globals) {
    native_at<std::uint32_t>(globals, 0x00) = 0x00CE6458u;
    native_at<std::uint8_t>(globals, 0x1C) = 0;
    host.soldier_globals_load_lua_0084cb70(globals);
    return globals;
}

// 00485520 (no ECX use), RET. Double-checked creation under the lifetime
// manager's lock: with [00F87544] null, section = [00415350()]+10h; a
// non-null section is entered (import 00CE2218) and its counter +18h
// incremented; if [00F87544] is still null, new(20h) + 00484700 (null on a
// failed allocation) is stored there and registered with
// 00BD0C30(ECX 00415350(), instance); then the counter is decremented and
// the section left (00CE2210). Returns [00F87544].
Object soldier_globals_get_singleton_00485520(Seg7Host& host) {
    if (host.soldier_globals_00f87544() != nullptr) return host.soldier_globals_00f87544();
    Object section = native_at<Object>(host.singleton_manager_00415350(), 0x10);
    if (section != nullptr) {
        host.enter_critical_section_import_00ce2218(section);
        native_at<std::int32_t>(section, 0x18) += 1;
    }
    if (host.soldier_globals_00f87544() == nullptr) {
        Object memory = host.operator_new_00bf681b(0x20);
        host.set_soldier_globals_00f87544(memory != nullptr ? soldier_globals_construct_00484700(host, memory) : nullptr);
        host.singleton_register_00bd0c30(host.singleton_manager_00415350(), host.soldier_globals_00f87544());
    }
    if (section != nullptr) {
        native_at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section_import_00ce2210(section);
    }
    return host.soldier_globals_00f87544();
}

namespace {
// GetByName into a stack LuaObject, one reader, then the LuaObject's
// destructor: the pattern every key of 00487770 and 0048DC40 follows.
float lua_float_key(Seg7Host& host, Object lua, const char* key, float fallback) {
    alignas(4) std::uint8_t ref[0x14];
    Object found = host.lua_get_by_name_00b67800(lua, ref, key);
    const float value = static_cast<float>(host.lua_get_float_or_default_00b66330(found, fallback));
    host.lua_destruct_00b67700(ref);
    return value;
}

std::int32_t lua_int_key(Seg7Host& host, Object lua, const char* key, std::int32_t fallback) {
    alignas(4) std::uint8_t ref[0x14];
    Object found = host.lua_get_by_name_00b67800(lua, ref, key);
    const std::int32_t value = host.lua_get_integer_or_default_00b66380(found, fallback);
    host.lua_destruct_00b67700(ref);
    return value;
}
} // namespace

// 00487770. Defaults: MaxTime 10.0f (00CE38B8), MinTime 2.0f (00CE3958),
// Radius 10.0f, Weight 10.0f; each x87 result is stored with FSTP float.
// A non-zero ExplosionEfx id gets a handle from 00870CD0(ECX = the argument
// slot reused as a RefPtr temporary, EDX = id, 1); the RefPtr at +0 takes the
// handle's object (InterlockedIncrement 00CE221C on the new one, release of
// the old one at refcount zero via vtable slot 0) and the temporary is then
// released the same way. Returns the row.
Object soldier_fake_effect_construct_00487770(Seg7Host& host, Object row, Object lua) {
    native_at<Object>(row, 0x00) = nullptr;
    native_at<float>(row, 0x04) = lua_float_key(host, lua, "MaxTime", 10.0f);
    native_at<float>(row, 0x08) = lua_float_key(host, lua, "MinTime", 2.0f);
    native_at<float>(row, 0x0C) = lua_float_key(host, lua, "Radius", 10.0f);
    native_at<float>(row, 0x10) = lua_float_key(host, lua, "Weight", 10.0f);
    const std::int32_t id = lua_int_key(host, lua, "ExplosionEfx", 0);
    if (id == 0) return row;
    alignas(4) std::uint8_t temporary[4];
    Object handle_slot = host.effect_handle_acquire_00870cd0(temporary, id, 1);
    Object incoming = native_at<Object>(handle_slot, 0);
    Object old = native_at<Object>(row, 0x00);
    if (old != incoming) {
        native_at<Object>(row, 0x00) = incoming;
        if (incoming != nullptr) host.interlocked_increment_import_00ce221c(incoming + 4);
        if (old != nullptr) release_ref(host, old);
    }
    Object held = native_at<Object>(temporary, 0);
    if (held != nullptr) release_ref(host, held);
    return row;
}

// 0048DC40. vtable 00CE66DC, +4 = class, the FakeEffects array +8..+10h = 0;
// Weight via 00B66270 (no default); FireChance 0.0f; Damage 5.0f (00CE3850);
// MinDistance 5.0f; MaxDistance 10.0f (00CE38B8); MaxEntity 14h; MininRow 1;
// MaxinRow 2; Defence via 00B662F0(0) into the byte +39h; OutingDelay 1.5f
// (00CE380C) into +28h. A FakeEffects value that is neither nil nor a
// non-table is iterated (key and value LuaObjects constructed, 00B67080 /
// 00B66420 / 00B67190): each value builds a 14h-byte row (null on a failed
// allocation) appended to +8 with the inlined 00485390 push-back. The value,
// key and table LuaObjects are destroyed in that order. Byte +38h is not
// written here. Returns the type.
Object soldier_type_construct_0048dc40(Seg7Host& host, Object type, Object klass, Object lua) {
    native_at<std::uint32_t>(type, 0x00) = 0x00CE66DCu;
    native_at<Object>(type, 0x04) = klass;
    Object effects = type + 0x08;
    native_at<std::uint32_t>(effects, 0x00) = 0;
    native_at<std::uint32_t>(effects, 0x04) = 0;
    native_at<std::uint32_t>(effects, 0x08) = 0;
    {
        alignas(4) std::uint8_t ref[0x14];
        Object found = host.lua_get_by_name_00b67800(lua, ref, "Weight");
        native_at<float>(type, 0x14) = static_cast<float>(host.lua_get_number_00b66270(found));
        host.lua_destruct_00b67700(ref);
    }
    native_at<float>(type, 0x18) = lua_float_key(host, lua, "FireChance", 0.0f);
    native_at<float>(type, 0x1C) = lua_float_key(host, lua, "Damage", 5.0f);
    native_at<float>(type, 0x20) = lua_float_key(host, lua, "MinDistance", 5.0f);
    native_at<float>(type, 0x24) = lua_float_key(host, lua, "MaxDistance", 10.0f);
    native_at<std::int32_t>(type, 0x2C) = lua_int_key(host, lua, "MaxEntity", 0x14);
    native_at<std::int32_t>(type, 0x30) = lua_int_key(host, lua, "MininRow", 1);
    native_at<std::int32_t>(type, 0x34) = lua_int_key(host, lua, "MaxinRow", 2);
    {
        alignas(4) std::uint8_t ref[0x14];
        Object found = host.lua_get_by_name_00b67800(lua, ref, "Defence");
        native_at<std::uint8_t>(type, 0x39) = host.lua_get_boolean_or_default_00b662f0(found, 0);
        host.lua_destruct_00b67700(ref);
    }
    native_at<float>(type, 0x28) = lua_float_key(host, lua, "OutingDelay", 1.5f);

    alignas(4) std::uint8_t table[0x14];
    host.lua_get_by_name_00b67800(lua, table, "FakeEffects");
    if (!host.lua_is_nil_00b65fb0(table) && host.lua_is_table_00b661b0(table)) {
        alignas(4) std::uint8_t key[0x14];
        alignas(4) std::uint8_t value[0x14];
        host.lua_construct_00b65f50(key);
        host.lua_construct_00b65f50(value);
        host.lua_iterate_first_00b67080(table, key, value);
        while (!host.lua_iterate_done_00b66420(table, value)) {
            Object memory = host.operator_new_00bf681b(0x14);
            Object row = memory != nullptr ? soldier_fake_effect_construct_00487770(host, memory, value) : nullptr;
            const std::uint32_t row_value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(row));
            push_back(host, effects, &row_value, pod_pointer_array_reserve_00482510);
            host.lua_iterate_next_00b67190(table, key, value);
        }
        host.lua_destruct_00b67700(value);
        host.lua_destruct_00b67700(key);
    }
    host.lua_destruct_00b67700(table);
    return type;
}

} // namespace bsp::cc13_seg7

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::cc13_seg7 {
void scene_runtime_detach_00480b70(Object runtime, Object entity, SceneDetachHost& host) {
    if (entity) {
        native_at<float>(runtime, 0x18) = host.time_00f876a4();
        host.attach_114(entity, nullptr);
    }
}
void scene_disable_if_byte5c_clear_00480be0(Object entity, SceneDisableHost& host) {
    if (!native_at<std::uint8_t>(entity, 0x5c)) host.enable_00922f30(entity, 0);
}
const char* path_reference_name_004871e0(Object reference) noexcept {
    Object entity = native_at<Object>(reference, 0x14);
    if (!entity) return "<null entity>";
    if (!native_at<std::uint32_t>(entity, 0x154)) return "<null name>";
    const char* text = native_at<const char*>(entity, 0x158);
    return text ? text : "";
}
Object soldier_globals_scalar_delete_00484770(Object object, std::uint32_t flags,
                                               SoldierGlobalsDeleteHost& host) {
    host.set_global_00f87544(nullptr);
    native_at<std::uint32_t>(object, 0) = 0x00ce3818;
    if (flags & 1u) host.free_00bf65ac(object);
    return object;
}
namespace {
// All three differences and the final sum are rounded at FSTP float. The
// intermediate products/addition retain the x87 precision (modelled in double).
float view_distance_squared(const float a[3], const float b[3]) {
    const float x = static_cast<float>(static_cast<double>(a[0]) - b[0]);
    const float y = static_cast<float>(static_cast<double>(a[1]) - b[1]);
    const float z = static_cast<float>(static_cast<double>(a[2]) - b[2]);
    return static_cast<float>((static_cast<double>(x) * x + static_cast<double>(y) * y) +
                              static_cast<double>(z) * z);
}
}
bool unit_path_in_view_00480c60(Object runtime, UnitPathViewHost& host) {
    Object actor = native_at<Object>(host.game_00e188a8(), 0x1ed4);
    if (!actor) return false;
    if (!native_at<std::uint8_t>(actor, 0xc8)) host.refresh_pose_00414db0(actor);
    Object unit = native_at<Object>(runtime, 0x14c);
    bool near = false;
    Object game;
    if (!host.unit_predicate_0080e3d0(unit)) {
        unit = native_at<Object>(runtime, 0x14c);
        if (!native_at<std::uint8_t>(unit, 0xc8)) host.refresh_pose_00414db0(unit);
        game = host.game_00e188a8();
        Object camera = native_at<Object>(game, 0x19fc);
        const float radius = static_cast<float>(300.0 / native_at<float>(camera, 0x1c4));
        const float square = static_cast<float>(static_cast<double>(radius) * radius);
        const float distance = view_distance_squared(reinterpret_cast<float*>(unit + 0xfc),
                                                      reinterpret_cast<float*>(actor + 0xfc));
        // JA takes only ordered distance > square; unordered is near.
        near = !(distance > square);
    } else {
        game = host.game_00e188a8();
    }
    if (near) {
        const float fade = static_cast<float>(static_cast<double>(native_at<float>(game, 0x21ec)) * 0.5 +
                                               native_at<float>(runtime, 0x1c));
        native_at<float>(runtime, 0x1c) = fade > 1.0f ? 1.0f : fade;
    } else {
        float fade = static_cast<float>(static_cast<double>(native_at<float>(runtime, 0x1c)) -
                                         static_cast<double>(native_at<float>(game, 0x21ec)) * 0.5);
        if (fade < 0.0f) fade = 0.0f;
        native_at<float>(runtime, 0x1c) = fade;
        if (fade <= 0.0f) return false;
    }
    unit = native_at<Object>(runtime, 0x14c);
    if (!native_at<std::uint8_t>(unit, 0xc8)) host.refresh_pose_00414db0(unit);
    Object extent = native_at<Object>(native_at<Object>(runtime, 0x14c), 0x538);
    const float size[3] = {native_at<float>(extent, 0xa4), native_at<float>(extent, 0xac),
                           native_at<float>(extent, 0xa0)};
    float bounds[6];
    for (unsigned i = 0; i != 3; ++i) {
        const float p = native_at<float>(unit, 0xfc + i * 4);
        bounds[i] = static_cast<float>(static_cast<double>(p) - size[i]);
        bounds[i + 3] = static_cast<float>(static_cast<double>(p) + size[i]);
    }
    if (!host.aabb_in_view_004b48e0(host.game_00e188a8(), bounds)) return false;
    float points[4][3];
    for (unsigned i = 0; i != 4; ++i) {
        float input[4] = {native_at<float>(runtime, 0x150 + i * 12),
                           native_at<float>(runtime, 0x154 + i * 12),
                           native_at<float>(runtime, 0x158 + i * 12), 1.0f};
        float out[4];
        float* p = host.transform_00b62d10(input, out,
            reinterpret_cast<const float*>(native_at<Object>(runtime, 0x14c) + 0x74));
        const float inv = static_cast<float>(1.0 / p[3]);
        for (unsigned j = 0; j != 3; ++j) points[i][j] = static_cast<float>(static_cast<double>(p[j]) * inv);
    }
    Object camera = native_at<Object>(host.game_00e188a8(), 0x19fc);
    const float radius = static_cast<float>(300.0 / native_at<float>(camera, 0x1c4));
    const float square = static_cast<float>(static_cast<double>(radius) * radius);
    for (const auto& point : points)
        if (square > view_distance_squared(point, reinterpret_cast<float*>(actor + 0xfc))) return true;
    return false;
}
}
