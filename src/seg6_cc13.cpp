#include "bsp/seg6_cc13.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cmath>
#include <initializer_list>
#include <limits>

// Segment 6 reconstructions. See include/bsp/seg6_cc13.hpp for the layout
// conventions and docs/CC13_SEG6.md for the per-function evidence.

namespace bsp::cc13_seg6 {
namespace {

std::uint32_t u32(Object base, std::uint32_t offset) { return native_at<std::uint32_t>(base, offset); }
void set_u32(Object base, std::uint32_t offset, std::uint32_t value) { native_at<std::uint32_t>(base, offset) = value; }
void set_ptr(Object base, std::uint32_t offset, const void* value) {
    native_at<std::uintptr_t>(base, offset) = reinterpret_cast<std::uintptr_t>(value);
}
Object as_object(std::uint32_t value) { return reinterpret_cast<Object>(static_cast<std::uintptr_t>(value)); }

// The inline NativeString release every destructor here carries: when data is
// non-null, 00419CC0(data, size + 1, 1) then 00BD1510 on the returned pool
// with the same three stack arguments.
void release_string_block(Seg6Host& host, void* data, std::uint32_t size) {
    if (data != nullptr) {
        Object pool = host.sized_pool_singleton_00419cc0(data, size + 1, 1);
        host.sized_pool_return_block_00bd1510(pool, data, size + 1, 1);
    }
}

// A NativeString at `string` (+0 size, +4 data).
void release_native_string(Seg6Host& host, Object string) {
    release_string_block(host, native_ptr(string, 4), u32(string, 0));
}

// The inline "assign a NativeString from another" sequence at 00465AF7,
// 004692BB and 00469438: resize to the source size with flag 1, then copy
// the source bytes when the source size is non-zero. The caller has already
// done the self-assignment test the image makes.
void assign_native_string(Seg6Host& host, Object dest, std::uint32_t source_size, const void* source_data) {
    host.native_string_resize_0041dd40(dest, source_size, 1);
    if (source_size != 0) host.crt_memcpy_00bf7680(native_ptr(dest, 4), source_data, u32(dest, 0));
}

// The inline "assign the empty literal 00CE3A0C" sequence at 004658FB and
// 004659B4: resize to 0 with flag 0, then copy size bytes when data is set.
void assign_empty_literal(Seg6Host& host, Object string) {
    host.native_string_resize_0041dd40(string, 0, 0);
    if (native_ptr(string, 4) != nullptr)
        host.crt_memcpy_00bf7680(native_ptr(string, 4), host.image_address(0x00CE3A0Cu), u32(string, 0));
}

// The shared body of 004689D0 and 00468A90 (two COMDAT copies).
std::uint32_t name_bucket(Seg6Host& host, Object key) {
    std::uint32_t h = u32(key, 0);
    const char* p = reinterpret_cast<const char*>(native_ptr(key, 4));
    const std::uint32_t step = (h | 0x20u) >> 5;
    if (p == nullptr) p = reinterpret_cast<const char*>(host.image_address(0x00E18560u));
    std::uint32_t remaining = h;
    while (remaining >= step) {
        const std::uint8_t upper = static_cast<std::uint8_t>(host.crt_toupper_00bf924e(static_cast<signed char>(*p)));
        h ^= upper + (h << 5) + (h >> 2);
        remaining -= step;
        ++p;
    }
    return h & 0x3Fu;
}

// The shared body of 00468ED0 and 00468FB0.
template <class Bucket>
Object find_node(Seg6Host& host, Object table, Object key, std::uint32_t* bucket_out, Bucket bucket_fn) {
    const std::uint32_t bucket = bucket_fn(host, table, key);
    *bucket_out = bucket;
    Object node = native_ptr(table, 8 + bucket * 4);
    if (node == nullptr) return node;
    const std::uint32_t key_size = u32(key, 0);
    for (;;) {
        if (u32(node, 0) == key_size) {
            const char* name = reinterpret_cast<const char*>(native_ptr(node, 4));
            if (name == nullptr) name = reinterpret_cast<const char*>(host.image_address(0x00E18560u));
            const char* key_text = host.native_string_c_str_00419ca0(key);
            if (host.string_compare_00438e10(key_text, name) == 0) return node;
        }
        node = native_ptr(node, 0x0C);
        if (node == nullptr) return node;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Engine-movie items.
// ---------------------------------------------------------------------------

// 00465610, __fastcall(ECX action), RET: the movie action base destructor
// (vtable 00CE5498 restored), then the three NativeStrings at +2Ch, +18h and
// +08h released in that order.
void movie_action_destruct_base_00465610(Seg6Host& host, Object action) {
    set_u32(action, 0, 0x00CE5498u);
    release_native_string(host, action + 0x2C);  // 00465635
    release_native_string(host, action + 0x18);  // 0046565A
    release_native_string(host, action + 0x08);  // 0046567C
}

// 004658D0, __thiscall(item, float time), RET 4, returns item.
Object movie_timed_flag_construct_004658d0(Seg6Host& host, Object item, float time) {
    host.movie_timed_item_construct_00465850(item, time);  // 004658F6
    set_u32(item, 0x34, 1);                                 // 0046590C
    assign_empty_literal(host, item + 4);                   // 00465913..0046592D
    native_at<float>(item, 0x18) = 0.0f;                    // 00465937 (XORPS)
    return item;
}

// 00465950, __thiscall(item, float x, float y, float z, float time), RET 10h.
// x - 50.0 and z - 50.0 use the double at 00CE3938 on the x87 stack and are
// rounded to float once (FSTP); the double subtraction here can differ from
// the 80-bit one only by a double rounding.
Object movie_timed_box_construct_00465950(Seg6Host& host, Object item, float x, float y, float z, float time) {
    host.movie_timed_item_construct_00465850(item, time);  // 00465978
    const float x_lo = static_cast<float>(static_cast<double>(x) - 50.0);
    const float z_lo = static_cast<float>(static_cast<double>(z) - 50.0);
    native_at<float>(item, 0x0C) = x;
    native_at<float>(item, 0x10) = y;
    native_at<float>(item, 0x14) = z;
    native_at<float>(item, 0x1C) = x_lo;
    native_at<float>(item, 0x20) = y;
    native_at<float>(item, 0x24) = z_lo;
    assign_empty_literal(host, item + 4);
    native_at<float>(item, 0x28) = x_lo;
    native_at<float>(item, 0x2C) = y;
    native_at<float>(item, 0x30) = z_lo;
    set_u32(item, 0x34, 1);
    native_at<float>(item, 0x18) = 0.0f;
    return item;
}

// 00465A40, __thiscall(item, a0, a1, a2, float time, b0..b5, int v34, float
// v18, NativeString text by value), 14 stack dwords, RET 38h. The self-assignment test at
// 00465AE6 compares item+4 with the address of the by-value argument, which
// can never match; it is kept as the image has it.
Object movie_timed_record_construct_00465a40(Seg6Host& host, Object item, float a0, float a1, float a2,
                                             float time, const float b[6], std::int32_t value_34,
                                             float value_18, std::uint32_t text_size, char* text_data) {
    host.movie_timed_item_construct_00465850(item, time);  // 00465A70
    native_at<float>(item, 0x0C) = a0;
    native_at<float>(item, 0x10) = a1;
    native_at<float>(item, 0x14) = a2;
    for (int i = 0; i < 6; ++i) native_at<float>(item, 0x1C + 4 * i) = b[i];
    native_at<std::int32_t>(item, 0x34) = value_34;
    std::uint32_t by_value[2] = {text_size, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(text_data))};
    if (item + 4 != reinterpret_cast<Object>(by_value)) {  // 00465AE6
        host.native_string_resize_0041dd40(item + 4, text_size, 1);
        if (text_size != 0) host.crt_memcpy_00bf7680(native_ptr(item, 8), text_data, u32(item, 4));
    }
    native_at<float>(item, 0x18) = value_18;      // 00465B1D
    release_string_block(host, text_data, text_size);  // 00465B15..00465B3A, the argument's destructor
    return item;
}

// ---------------------------------------------------------------------------
// Matrix and value helpers.
// ---------------------------------------------------------------------------

// 00467050, __thiscall(float* matrix, a, b, c), RET 0Ch. Each FSIN/FCOS result
// is stored to a float local first; the products then run on the x87 stack and
// are rounded to float at each FSTP. Element [9] is SUBSS of sin(a) from the
// float -0.0 at 00D7A208. cc13-fixture2 follow-up: the callers (HUD free camera
// 00540390, scene database 0046DC10, plane control init 007BDF50 and others) run
// on the main thread at 24-bit precision control, so each operation rounds to a
// 24-bit significand (x87_*24), in listing order: sc*sa first (00467090..9E),
// then times sb (004670BF) or cb (004670D8). FSIN/FCOS: see cc13_x87_model.hpp.
void matrix_set_rotation_from_euler_00467050(float* m, float a, float b, float c) {
    using namespace cc13x87;
    const float sa = static_cast<float>(x87_sin(a));
    const float ca = static_cast<float>(x87_cos(a));
    const float sb = static_cast<float>(x87_sin(b));
    const float cb = static_cast<float>(x87_cos(b));
    const float sc = static_cast<float>(x87_sin(c));
    const float cc = static_cast<float>(x87_cos(c));
    m[9] = -0.0f - sa;                                               // 00467096..004670AA
    const double sc_sa = x87_mul24(sc, sa);                          // 0046709E
    m[0] = static_cast<float>(x87_add24(x87_mul24(cc, cb), x87_mul24(sb, sc_sa)));
    m[1] = static_cast<float>(x87_mul24(sc, ca));
    m[2] = static_cast<float>(x87_sub24(x87_mul24(sc_sa, cb), x87_mul24(cc, sb)));
    const double cc_sa = x87_mul24(cc, sa);                          // 004670EA FST double
    m[4] = static_cast<float>(x87_sub24(x87_mul24(cc_sa, sb), x87_mul24(sc, cb)));
    m[5] = static_cast<float>(x87_mul24(cc, ca));
    m[6] = static_cast<float>(x87_add24(x87_mul24(sc, sb), x87_mul24(cb, cc_sa)));
    m[8] = static_cast<float>(x87_mul24(sb, ca));
    m[10] = static_cast<float>(x87_mul24(ca, cb));
}

// 00468560, __thiscall(descriptor, const float pos[3], float extra), RET 8,
// returns descriptor (MOV EAX,ECX at 00468566; the callers pass EAX on). The
// stores in image order: word +0 = 0, byte +1 = 1, the position at +8..+10h
// (FLD/FSTP), +4 = 0, byte +0 = 0, word +2 = 0, +14h = extra (MOVSS).
Object command_target_init_position_00468560(Object d, const float* position, float extra) {
    native_at<std::uint16_t>(d, 0) = 0;
    native_at<std::uint8_t>(d, 1) = 1;
    for (int i = 0; i < 3; ++i) native_at<float>(d, 8 + 4 * i) = position[i];
    set_u32(d, 4, 0);
    native_at<std::uint8_t>(d, 0) = 0;
    native_at<std::uint16_t>(d, 2) = 0;
    native_at<float>(d, 0x14) = extra;
    return d;
}

// ---------------------------------------------------------------------------
// Scene database.
// ---------------------------------------------------------------------------

// 00467370, __fastcall(ECX flag byte), RET: 00925F20 with CL = 0 once.
void scene_database_init_entities_once_00467370(Seg6Host& host, Object flag) {
    if (*flag == 0) {
        host.sentity_init_all_00925f20(0);
        *flag = 1;
    }
}

// 004674F0, __thiscall({data, size, capacity}*, n), RET 4. The per-element
// null test is the placement-new test the image makes.
void pod_pointer_array_reserve_004674f0(Seg6Host& host, Object array, std::int32_t count) {
    if (count < 1) count = 1;
    if (native_at<std::int32_t>(array, 8) < count) {
        Object block = host.operator_new_00bf55be(static_cast<std::uint32_t>(count) * 4);
        Object slot = block;
        for (std::int32_t i = 0; i < native_at<std::int32_t>(array, 4); ++i, slot += 4) {
            if (slot != nullptr) set_u32(slot, 0, u32(native_ptr(array, 0), 4 * i));
        }
        host.free_00bf6989(native_ptr(array, 0));
        set_ptr(array, 0, block);
        native_at<std::int32_t>(array, 8) = count;
    }
}

// 00467EE0, __thiscall(database, int id), RET 4: iterates the class map at
// +34h and returns the row (*(node+8)) whose first dword equals id, else 0.
Object scene_database_find_class_row_by_id_00467ee0(Seg6Host& host, Object database, std::int32_t id) {
    std::uint32_t iterator[3] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(database + 0x34)), 0, 0};
    Object it = reinterpret_cast<Object>(iterator);
    host.scene_name_hash_iterator_begin_00466bb0(it);
    for (Object node = native_ptr(it, 4); node != nullptr; node = native_ptr(it, 4)) {
        Object row = native_ptr(node, 8);
        if (native_at<std::int32_t>(row, 0) == id) return row;
        host.scene_name_hash_iterator_next_00466c00(it);
    }
    return nullptr;
}

// 00468350, __thiscall(list {count, head, tail}, record), RET 4. The node is
// {prev +0, next +4, record +8}; a null operator new result is used as is,
// as in the image.
void scene_command_list_push_back_00468350(Seg6Host& host, Object list, Object record) {
    Object node = host.operator_new_00bf681b(0x0C);
    if (node != nullptr) {
        set_u32(node, 0, 0);
        set_u32(node, 4, 0);
        set_u32(node, 8, 0);
    }
    set_ptr(node, 8, record);
    set_u32(node, 0, u32(list, 8));
    if (u32(list, 0) != 0) {
        set_ptr(native_ptr(list, 8), 4, node);
        set_ptr(list, 8, node);
    } else {
        set_ptr(list, 4, node);
        set_ptr(list, 8, node);
    }
    set_u32(node, 4, 0);
    set_u32(list, 0, u32(list, 0) + 1);
}

// 00468600, __fastcall(ECX property, EDX array {data, size, capacity}), RET.
// The element count (008EF7F0, ECX = property) is re-read after each append.
void scene_property_collect_array_elements_00468600(Seg6Host& host, Object property, Object array) {
    Object element = native_ptr(property, 0x20);
    std::uint32_t i = 0;
    if (host.scene_property_array_element_count_008ef7f0(property) == 0) return;
    do {
        if (native_at<std::int32_t>(array, 4) == native_at<std::int32_t>(array, 8)) {
            std::int32_t grown = native_at<std::int32_t>(array, 8) * 2;
            if (grown <= 1) grown = 1;
            pod_pointer_array_reserve_004674f0(host, array, grown);  // 0046862D
        }
        Object slot = native_ptr(array, 0) + 4 * native_at<std::int32_t>(array, 4);
        if (slot != nullptr) set_u32(slot, 0, u32(element, 0));
        native_at<std::int32_t>(array, 4) += 1;
        ++i;
        element += 4;
    } while (i < host.scene_property_array_element_count_008ef7f0(property));
}

// 00468660, __thiscall(database, int id), RET 4: the class name (node+4) of
// the class-map row whose first dword is id; the literal 00E18560 when that
// name is null; 0 when no row matches.
Object scene_database_class_id_to_name_00468660(Seg6Host& host, Object database, std::int32_t id) {
    std::uint32_t iterator[3] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(database + 0x34)), 0, 0};
    Object it = reinterpret_cast<Object>(iterator);
    host.scene_name_hash_iterator_begin_00466bb0(it);
    for (Object node = native_ptr(it, 4); node != nullptr; node = native_ptr(it, 4)) {
        if (native_at<std::int32_t>(native_ptr(node, 8), 0) == id) {
            Object name = native_ptr(node, 4);
            return name != nullptr ? name : host.image_address(0x00E18560u);
        }
        host.scene_name_hash_iterator_next_00466c00(it);
    }
    return nullptr;
}

// 004689D0 / 00468A90, [ESP+4] = const NativeString*, RET 4; ECX is not read.
// h = size; step = (size | 20h) >> 5; for each of size/step characters taken
// one after another from the start, h ^= toupper(c) + (h << 5) + (h >> 2);
// the bucket is h & 3Fh. A null data pointer reads the literal 00E18560.
std::uint32_t scene_name_hash_bucket_004689d0(Seg6Host& host, Object, Object key) { return name_bucket(host, key); }
std::uint32_t scene_class_map_bucket_00468a90(Seg6Host& host, Object, Object key) { return name_bucket(host, key); }

// 00468ED0 / 00468FB0, __thiscall(table, const NativeString* key, uint* bucket
// out), RET 8: the chain at table+8+bucket*4, matching on size then on
// 00438E10(c_str(key), node name) == 0.
Object scene_name_hash_find_node_00468ed0(Seg6Host& host, Object table, Object key, std::uint32_t* bucket) {
    return find_node(host, table, key, bucket, scene_name_hash_bucket_004689d0);
}
Object scene_class_map_find_node_00468fb0(Seg6Host& host, Object table, Object key, std::uint32_t* bucket) {
    return find_node(host, table, key, bucket, scene_class_map_bucket_00468a90);
}

// 00469010, __fastcall(ECX record), RET: releases the NativeStrings at +0Ch
// then +04h.
void scene_record_destruct_fields_00469010(Seg6Host& host, Object record) {
    release_native_string(host, record + 0x0C);
    release_native_string(host, record + 0x04);
}

// 004691B0, __thiscall(database, const char* name), RET 4: find in the hidden
// record map at +18h; 0 at end, else the record at node+14h.
Object scene_database_find_hidden_entity_record_004691b0(Seg6Host& host, Object database, const char* name) {
    std::uint32_t key[2] = {0, 0};
    std::uint32_t found[2] = {0, 0};  // {container, node}
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, name);                       // 004691C0
    Object map = database + 0x18;
    host.hidden_record_map_find_00468cd0(map, reinterpret_cast<Object>(found), key_obj);  // 004691D4
    release_native_string(host, key_obj);                                       // 004691DD..004691F3
    Object container = as_object(found[0]);
    Object node = as_object(found[1]);
    Object head = native_ptr(map, 4);
    if (container == nullptr || container != map) host.checked_iterator_report_00bf6713();
    if (node == head) return nullptr;
    if (container == nullptr) host.checked_iterator_report_00bf6713();
    if (node == native_ptr(container, 4)) host.checked_iterator_report_00bf6713();
    return native_ptr(node, 0x14);
}

// 00469240, __thiscall(record, const record* source), RET 4.
Object scene_hidden_entity_record_copy_construct_00469240(Seg6Host& host, Object record, Object source) {
    set_u32(record, 0, 0x00CE5640u);
    set_u32(record, 0x54, 0);
    set_u32(record, 0x58, 0);
    set_ptr(record, 0x04, host.property_bag_clone_008f41f0(native_ptr(source, 0x04)));
    set_ptr(record, 0x08, host.string_duplicate_00438e40(reinterpret_cast<const char*>(native_ptr(source, 0x08))));
    set_ptr(record, 0x0C, host.string_duplicate_00438e40(reinterpret_cast<const char*>(native_ptr(source, 0x0C))));
    set_u32(record, 0x10, u32(source, 0x10));
    host.matrix_copy_4x4_004134f0(record + 0x14, source + 0x14);
    if (record + 0x54 != source + 0x54) assign_native_string(host, record + 0x54, u32(source, 0x54), native_ptr(source, 0x58));
    return record;
}

// 00469300, __fastcall(ECX record), RET: the bag's deleting destructor, CRT
// free (00BF6989) of the two strdup'd names, then the NativeString at +54h.
void scene_hidden_entity_record_destruct_00469300(Seg6Host& host, Object record) {
    set_u32(record, 0, 0x00CE5640u);
    if (native_ptr(record, 4) != nullptr) {
        host.object_deleting_destructor_vcall_00(native_ptr(record, 4), 1);
        set_u32(record, 4, 0);
    }
    if (native_ptr(record, 8) != nullptr) {
        host.free_00bf6989(native_ptr(record, 8));
        set_u32(record, 8, 0);
    }
    if (native_ptr(record, 0x0C) != nullptr) {
        host.free_00bf6989(native_ptr(record, 0x0C));
        set_u32(record, 0x0C, 0);
    }
    release_native_string(host, record + 0x54);
}

// 004693A0, __thiscall(record, flags), RET 4: vtable 00CE5640 slot 0.
Object scene_hidden_entity_record_deleting_destructor_004693a0(Seg6Host& host, Object record, std::uint8_t flags) {
    scene_hidden_entity_record_destruct_00469300(host, record);
    if (flags & 1) host.free_00bf65ac(record);
    return record;
}

// 004693C0, __thiscall(record, bag, className, name, party, frame, const
// NativeString* parent), RET 18h: +8 = strdup(name), +0Ch = strdup(className).
Object scene_hidden_entity_record_construct_004693c0(Seg6Host& host, Object record, Object bag,
                                                     const char* class_name, const char* name,
                                                     std::int32_t party, Object frame, Object parent_name) {
    set_u32(record, 0, 0x00CE5640u);
    set_u32(record, 0x54, 0);
    set_u32(record, 0x58, 0);
    set_ptr(record, 0x04, bag);
    set_ptr(record, 0x08, host.string_duplicate_00438e40(name));        // 00469408
    set_ptr(record, 0x0C, host.string_duplicate_00438e40(class_name));  // 00469414
    native_at<std::int32_t>(record, 0x10) = party;
    host.matrix_copy_4x4_004134f0(record + 0x14, frame);                // 0046942B
    if (record + 0x54 != parent_name) assign_native_string(host, record + 0x54, u32(parent_name, 0), native_ptr(parent_name, 4));
    return record;
}

// 00469480, __thiscall(database, const char* name), RET 4. The head is read
// before the find (00469496); the key is released after the compare.
bool scene_database_has_hidden_entity_record_00469480(Seg6Host& host, Object database, const char* name) {
    std::uint32_t key[2] = {0, 0};
    std::uint32_t found[2] = {0, 0};
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, name);
    Object map = database + 0x18;
    Object head = native_ptr(map, 4);
    Object it = host.hidden_record_map_find_00468cd0(map, reinterpret_cast<Object>(found), key_obj);
    Object container = native_ptr(it, 0);
    if (container == nullptr || container != map) host.checked_iterator_report_00bf6713();
    const bool present = native_ptr(it, 4) != head;
    release_native_string(host, key_obj);
    return present;
}

// 00469610, __thiscall(database, a, b, c), RET 0Ch: a 14h-byte record built
// by 004690D0(a, b, c) (0 when operator new fails) pushed on the list at +14Ch.
void scene_database_queue_entity_command_00469610(Seg6Host& host, Object database, const char* a,
                                                  const char* b, const char* c) {
    Object block = host.operator_new_00bf681b(0x14);
    Object record = block != nullptr ? host.scene_command_record_construct_004690d0(block, a, b, c) : nullptr;
    scene_command_list_push_back_00468350(host, database + 0x14C, record);  // 0046966C
}

// 00469690, __thiscall(database, const char* className), RET 4. The iterator
// starts before the key is built (004696B6, 004696C4). The comparison is the
// inline case-insensitive NativeString-to-C-string test: a null key matches an
// empty name; a non-null key matches through __stricmp, or a null name when
// the key is empty (the name is never null after the 00E18560 substitution).
std::int32_t scene_database_class_name_to_id_00469690(Seg6Host& host, Object database, const char* class_name) {
    std::uint32_t iterator[3] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(database + 0x34)), 0, 0};
    Object it = reinterpret_cast<Object>(iterator);
    host.scene_name_hash_iterator_begin_00466bb0(it);
    std::uint32_t key[2] = {0, 0};
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, class_name);
    const char* key_data = reinterpret_cast<const char*>(native_ptr(key_obj, 4));
    const std::uint32_t key_size = u32(key_obj, 0);
    for (Object node = native_ptr(it, 4); node != nullptr; node = native_ptr(it, 4)) {
        const char* name = reinterpret_cast<const char*>(native_ptr(node, 4));
        if (name == nullptr) name = reinterpret_cast<const char*>(host.image_address(0x00E18560u));
        bool match;
        if (key_data == nullptr) {
            match = name == nullptr || name[0] == '\0';
        } else if (name == nullptr) {
            match = key_size == 0;
        } else {
            match = host.crt_stricmp_00bf7fbf(key_data, name) == 0;
        }
        if (match) {
            const std::int32_t id = native_at<std::int32_t>(native_ptr(node, 8), 0);
            release_native_string(host, key_obj);
            return id;
        }
        host.scene_name_hash_iterator_next_00466c00(it);
    }
    release_native_string(host, key_obj);
    return 0;
}

// 004697B0, __fastcall(ECX database), RET: deleting destructor (slot 0, 1) on
// every non-null element of the vector at +28h..+2Ch, then the inline
// erase(begin, end).
void scene_database_destroy_pending_objects_004697b0(Seg6Host& host, Object database) {
    std::uint32_t i = 0;
    Object first;
    for (;;) {
        first = native_ptr(database, 0x28);
        if (first == nullptr) break;
        if (i >= static_cast<std::uint32_t>((native_ptr(database, 0x2C) - first) >> 2)) break;
        if (first == nullptr || i >= static_cast<std::uint32_t>((native_ptr(database, 0x2C) - first) >> 2))
            host.checked_iterator_report_00bf6713();  // 004697DA
        Object element = native_ptr(native_ptr(database, 0x28), 4 * i);
        if (element != nullptr) host.object_deleting_destructor_vcall_00(element, 1);
        ++i;
    }
    Object last = native_ptr(database, 0x2C);
    if (first > last) host.checked_iterator_report_00bf6713();  // 004697FD
    Object begin = native_ptr(database, 0x28);
    if (begin > native_ptr(database, 0x2C)) host.checked_iterator_report_00bf6713();  // 0046980A
    if (begin != last) {
        const std::int32_t count = static_cast<std::int32_t>((native_ptr(database, 0x2C) - last) >> 2);
        const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4;
        if (count > 0) host.crt_memmove_s_00bf67a7(begin, bytes, last, bytes);
        set_ptr(database, 0x2C, begin + bytes);
    }
}

// 00469B60, __thiscall(owner, const char* name), RET 4: the value (node+8) of
// the name-hash node at owner+4. The node is read without a null test.
std::uint32_t scene_name_table_lookup_value_00469b60(Seg6Host& host, Object owner, const char* name) {
    std::uint32_t key[2] = {0, 0};
    std::uint32_t bucket = 0;  // the image reuses the argument slot
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, name);
    Object node = scene_name_hash_find_node_00468ed0(host, owner + 4, key_obj, &bucket);
    const std::uint32_t value = u32(node, 8);
    release_native_string(host, key_obj);
    return value;
}

// 00469EB0, __thiscall(database, const char* className), RET 4: node+8 of the
// class-map node at +34h, or 0.
std::uint32_t scene_database_find_class_value_by_name_00469eb0(Seg6Host& host, Object database,
                                                               const char* class_name) {
    std::uint32_t key[2] = {0, 0};
    std::uint32_t bucket = 0;
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, class_name);
    Object node = scene_class_map_find_node_00468fb0(host, database + 0x34, key_obj, &bucket);
    const std::uint32_t value = node != nullptr ? u32(node, 8) : 0;
    release_native_string(host, key_obj);
    return value;
}

// 0046A090, __fastcall(ECX record**), RET.
void scene_record_ptr_delete_0046a090(Seg6Host& host, Object slot) {
    Object record = native_ptr(slot, 0);
    if (record != nullptr) {
        scene_record_destruct_fields_00469010(host, record);
        host.free_00bf65ac(record);
        set_u32(slot, 0, 0);
    }
}

// 0046A9F0, __fastcall(ECX list {count, head, tail}), RET: pops from the head
// until empty, destroying each node's record (+8) and the node.
void scene_command_list_clear_0046a9f0(Seg6Host& host, Object list) {
    while (u32(list, 0) != 0) {
        Object node = native_ptr(list, 4);
        Object record = native_ptr(node, 8);
        if (record != nullptr) {
            scene_record_destruct_fields_00469010(host, record);
            host.free_00bf65ac(record);
            set_u32(node, 8, 0);
        }
        Object prev = native_ptr(node, 0);
        if (prev != nullptr) set_u32(prev, 4, u32(node, 4));
        else set_u32(list, 4, u32(node, 4));
        if (native_ptr(node, 4) != nullptr) set_u32(native_ptr(node, 4), 0, u32(node, 0));
        else set_u32(list, 8, u32(node, 0));
        set_u32(list, 0, u32(list, 0) - 1);
        host.free_00bf65ac(node);
    }
}

// 0046AA60, [ESP+4] object, RET 4: frees the block at +8, then (the image
// tests the pointer only here) releases the NativeString at +0 and returns the
// object to the 14h-byte node pool at 00E18514.
void named_vector_destruct_0046aa60(Seg6Host& host, Object object) {
    if (native_ptr(object, 8) != nullptr) {
        host.free_00bf65ac(native_ptr(object, 8));
        set_u32(object, 8, 0);
    }
    if (object != nullptr) {
        release_native_string(host, object);
        host.enum_node_pool_return_0043b0a0(host.image_address(0x00E18514u), object);  // 0046AAA2
    }
}

// 0046B240, __fastcall(ECX map), RET: for each of the 64 buckets at map+8,
// frees each node's value (+8, CRT free), releases its key, and returns the
// node to the pool at 00E18514; then clears the bucket. +4 = 0 at the end.
// The pool return is 0043B0A0's body inlined (0046B2A4..0046B302: the same
// critical section at pool+0Ch, counter at +24h, chunk table at +28h, lowest
// free chunk at +34h); it is issued here as that call.
void scene_class_map_destruct_0046b240(Seg6Host& host, Object map) {
    Object bucket = map + 8;
    for (int remaining = 0x40; remaining != 0; --remaining, bucket += 4) {
        Object node = native_ptr(bucket, 0);
        while (node != nullptr) {
            Object next = native_ptr(node, 0x0C);
            if (native_ptr(node, 8) != nullptr) {
                host.free_00bf65ac(native_ptr(node, 8));
                set_u32(node, 8, 0);
            }
            release_native_string(host, node);
            host.enum_node_pool_return_0043b0a0(host.image_address(0x00E18514u), node);
            node = next;
        }
        set_u32(bucket, 0, 0);
    }
    set_u32(map, 4, 0);
}

namespace {

// The checked walk 0046B6A3..0046B6FE and 0046BD30..0046BD77 share: the
// iterator {container, node} starts at head->left; each record at node+14h
// gets its deleting destructor (slot 0, 1). `clear_slot` is 0046B680's
// extra store of 0 into node+14h before the call.
void delete_hidden_records(Seg6Host& host, Object database, bool clear_slot) {
    Object map = database + 0x18;
    std::uint32_t iterator[2] = {static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(map)),
                                 u32(native_ptr(map, 4), 0)};
    for (;;) {
        Object container = as_object(iterator[0]);
        Object node = as_object(iterator[1]);
        Object head = native_ptr(map, 4);
        if (container == nullptr || container != map) host.checked_iterator_report_00bf6713();
        if (node == head) break;
        if (container == nullptr) host.checked_iterator_report_00bf6713();
        if (node == native_ptr(container, 4)) host.checked_iterator_report_00bf6713();
        Object record;
        if (clear_slot) {
            if (node == native_ptr(container, 4)) host.checked_iterator_report_00bf6713();
            record = native_ptr(node, 0x14);
            set_u32(node, 0x14, 0);
        } else {
            record = native_ptr(node, 0x14);
        }
        if (record != nullptr) host.object_deleting_destructor_vcall_00(record, 1);
        host.tree_iterator_increment_00467980(reinterpret_cast<Object>(iterator));
    }
}

// The inline map clear after the walk: _Erase(root), then head->parent =
// head, size = 0, head->left = head, head->right = head.
void clear_hidden_record_tree(Seg6Host& host, Object map) {
    host.hidden_record_tree_erase_0046ad10(map, native_ptr(native_ptr(map, 4), 4));
    set_ptr(native_ptr(map, 4), 4, native_ptr(map, 4));
    set_u32(map, 8, 0);
    set_ptr(native_ptr(map, 4), 0, native_ptr(map, 4));
    set_ptr(native_ptr(map, 4), 8, native_ptr(map, 4));
}

}  // namespace

// 0046B680, __fastcall(ECX database), RET: when the hidden-record map is not
// empty, deletes every record (nulling node+14h first) and clears the map.
void scene_database_clear_hidden_records_0046b680(Seg6Host& host, Object database) {
    if (u32(database, 0x20) == 0) return;
    delete_hidden_records(host, database, true);
    clear_hidden_record_tree(host, database + 0x18);
}

// 0046BD10, __fastcall(ECX database), RET: deletes every hidden record, then
// every pending object in the vector at +24h {+28h first, +2Ch last}, clears
// the map, and erases the vector (inline erase(begin, end)). The image's
// container self-test at 0046BD9D compares ESI with itself and never fires.
void scene_database_release_records_and_pending_0046bd10(Seg6Host& host, Object database) {
    delete_hidden_records(host, database, false);
    Object vector = database + 0x24;
    Object cursor = native_ptr(vector, 4);
    if (cursor > native_ptr(vector, 8)) host.checked_iterator_report_00bf6713();  // 0046BD88
    for (;;) {
        Object last = native_ptr(vector, 8);
        if (native_ptr(vector, 4) > last) host.checked_iterator_report_00bf6713();  // 0046BD98
        if (cursor == last) break;
        if (cursor >= native_ptr(vector, 8)) host.checked_iterator_report_00bf6713();  // 0046BDAF
        if (native_ptr(cursor, 0) != nullptr) host.object_deleting_destructor_vcall_00(native_ptr(cursor, 0), 1);
        if (cursor >= native_ptr(vector, 8)) host.checked_iterator_report_00bf6713();  // 0046BDC7
        cursor += 4;
    }
    clear_hidden_record_tree(host, database + 0x18);
    Object last = native_ptr(vector, 8);
    if (native_ptr(vector, 4) > last) host.checked_iterator_report_00bf6713();  // 0046BDFF
    Object begin = native_ptr(vector, 4);
    if (begin > native_ptr(vector, 8)) host.checked_iterator_report_00bf6713();  // 0046BE0C
    if (begin != last) {
        const std::int32_t count = static_cast<std::int32_t>((native_ptr(vector, 8) - last) >> 2);
        const std::uint32_t bytes = static_cast<std::uint32_t>(count) * 4;
        if (count > 0) host.crt_memmove_s_00bf67a7(begin, bytes, last, bytes);
        set_ptr(vector, 8, begin + bytes);
    }
}

// 0046BE50, __fastcall(ECX owner), RET: for the eight dwords at game+1910h..
// +192Ch (game re-read from 00E188A8 each time), pushes the address of each
// positive one onto the vector at owner+8.
void scene_stock_push_positive_counters_0046be50(Seg6Host& host, Object owner) {
    for (std::uint32_t offset = 0x1910; offset < 0x1930; offset += 4) {
        Object counter = host.game_00e188a8() + offset;
        if (native_at<std::int32_t>(counter, 0) > 0) host.pointer_vector_push_back_00442190(owner + 8, counter);
    }
}

// 0046BE90, __thiscall(owner, int unitClass), RET 4: unitClass 2ACh is
// ignored; otherwise unless the class name of unitClass (00468660 on
// [00E18680]) equals "SpawnPoint" (00CE56B8, through 00425850), the address
// of the argument is pushed onto the vector at owner+8.
void scene_stock_register_unit_class_0046be90(Seg6Host& host, Object owner, std::int32_t unit_class) {
    if (unit_class == 0x2AC) return;
    Object name = scene_database_class_id_to_name_00468660(host, host.scene_database_00e18680(), unit_class);
    bool spawn_point = false;
    if (name != nullptr) {
        std::uint32_t text[2] = {0, 0};
        Object text_obj = reinterpret_cast<Object>(text);
        Object built = host.native_string_construct_0041e870(text_obj, reinterpret_cast<const char*>(name));
        spawn_point = host.native_string_equals_00425850(built, reinterpret_cast<const char*>(host.image_address(0x00CE56B8u)));
        release_native_string(host, text_obj);
    }
    if (!spawn_point) host.pointer_vector_push_back_00442190(owner + 8, &unit_class);
}

// 0046BF20, __thiscall(owner, bag, const char* className), RET 8: when the bag
// has a "Type" property (00CE4780) and className's id (00469690 on owner) is
// not 47h, registers that property's value (+0Ch) through 0046BE90. The bag is
// searched again for the value.
void scene_stock_register_from_type_property_0046bf20(Seg6Host& host, Object owner, Object bag,
                                                      const char* class_name) {
    const char* type_key = reinterpret_cast<const char*>(host.image_address(0x00CE4780u));
    if (host.property_bag_find_008f2260(bag, type_key) == nullptr) return;
    if (scene_database_class_name_to_id_00469690(host, owner, class_name) == 0x47) return;
    Object property = host.property_bag_find_008f2260(bag, type_key);
    scene_stock_register_unit_class_0046be90(host, owner, native_at<std::int32_t>(property, 0x0C));
}

namespace {

const char* literal(Seg6Host& host, std::uint32_t va) { return reinterpret_cast<const char*>(host.image_address(va)); }

// The common body of the "PlaneStock %d" and "Slot %d" passes (0046C07A..
// 0046C0D8, 0046C11F..0046C180): a sub-bag property (type 6) whose "Count" is
// non-zero routes its "Type" value by the outer bag's "Hidden" byte: 0 ->
// 0095C4D0, otherwise 0046BE90. "Type" is read before "Count".
void route_counted_stock(Seg6Host& host, Object owner, Object bag, const char* key) {
    Object property = host.property_bag_find_008f2260(bag, key);
    if (property == nullptr || u32(property, 4) != 6) return;
    Object sub = native_ptr(property, 0x0C);
    if (sub == nullptr) return;
    const std::int32_t type = native_at<std::int32_t>(host.property_bag_find_008f2260(sub, literal(host, 0x00CE4780u)), 0x0C);
    if (u32(host.property_bag_find_008f2260(sub, literal(host, 0x00CE5710u)), 0x0C) == 0) return;
    if (native_at<std::uint8_t>(host.property_bag_find_008f2260(bag, literal(host, 0x00CE5708u)), 0x0C) == 0)
        host.vehicle_class_stock_queue_append_0095c4d0(type);
    else
        scene_stock_register_unit_class_0046be90(host, owner, type);
}

// The inline NativeString built from the class name at 0046C196..0046C1F5 and
// 0046C299..0046C2F8: resize(strlen, 1); when data is set, copy size+1 bytes
// (the terminator too), __stricmp against the literal, release. No data, no match.
bool class_name_is(Seg6Host& host, const char* class_name, std::uint32_t literal_va) {
    std::uint32_t text[2] = {0, 0};
    Object text_obj = reinterpret_cast<Object>(text);
    std::uint32_t length = 0;
    while (class_name[length] != '\0') ++length;
    host.native_string_resize_0041dd40(text_obj, length, 1);
    char* data = reinterpret_cast<char*>(native_ptr(text_obj, 4));
    if (data == nullptr) return false;
    const std::uint32_t size = u32(text_obj, 0);
    host.crt_memcpy_00bf7680(data, class_name, size + 1);
    const bool match = host.crt_stricmp_00bf7fbf(data, literal(host, literal_va)) == 0;
    release_string_block(host, data, size);
    return match;
}

// 0046C329..0046C3A4 and 0046C3C3..0046C439: "Stock %d" sub-bags of `list`
// (which may be null; the image calls 008F2260 with it regardless) while each
// is a non-null type-6 bag; when its "Count" is non-zero, its "Type" is queued
// through 0095C4D0 unless the effective game mode is 4. Returns the next index.
std::int32_t queue_list_stock(Seg6Host& host, Object list, std::int32_t index) {
    for (;; ++index) {
        char key[32];
        host.format_to_buffer32_00467310(key, literal(host, 0x00CE56DCu), index);
        Object property = host.property_bag_find_008f2260(list, key);
        if (property == nullptr || u32(property, 4) != 6) return index;
        Object sub = native_ptr(property, 0x0C);
        if (sub == nullptr) return index;
        if (u32(host.property_bag_find_008f2260(sub, literal(host, 0x00CE5710u)), 0x0C) == 0) continue;
        const std::int32_t type = native_at<std::int32_t>(host.property_bag_find_008f2260(sub, literal(host, 0x00CE4780u)), 0x0C);
        if (host.game_effective_game_mode_004bca50(host.game_00e188a8()) != 4)
            host.vehicle_class_stock_queue_append_0095c4d0(type);
    }
}

}  // namespace

// 0046BF70, __thiscall(owner, bag, const char* className), RET 8.
// 1. "AirField" (00CE5738; inline __stricmp on a NativeString copy, which an
//    empty copy never matches) or "MotherShipGen" (00CE5728, 00425850 on a
//    second copy): "PlaneStock %d" for 1..12, then "Slot %d" for 1..NumSlots.
// 2. "Shipyard" (00CE56E8): "Stock %d" for 1..20 whose "Type" value is > 0,
//    routed by the bag's "Hidden" byte as in 1 ("Count" is not consulted).
// 3. "SpawnPoint" (00CE56B8): "Stock %d" from 1 in the "JapanList" bag, then
//    on in the "AlliedList" bag with the index continuing where it stopped.
void scene_stock_register_class_stock_lists_0046bf70(Seg6Host& host, Object owner, Object bag,
                                                     const char* class_name) {
    bool generator;
    {
        std::uint32_t first[2] = {0, 0};
        Object first_obj = reinterpret_cast<Object>(first);
        host.native_string_construct_0041e870(first_obj, class_name);  // 0046BF9F
        const char* data = reinterpret_cast<const char*>(native_ptr(first_obj, 4));
        if (data != nullptr && host.crt_stricmp_00bf7fbf(data, literal(host, 0x00CE5738u)) == 0) {
            generator = true;
        } else {
            std::uint32_t second[2] = {0, 0};
            Object second_obj = reinterpret_cast<Object>(second);
            Object built = host.native_string_construct_0041e870(second_obj, class_name);  // 0046BFD4
            generator = host.native_string_equals_00425850(built, literal(host, 0x00CE5728u));
            release_native_string(host, second_obj);
        }
        release_native_string(host, first_obj);
    }
    if (generator) {
        for (std::int32_t i = 1; i <= 12; ++i) {
            char key[32];
            host.format_to_buffer32_00467310(key, literal(host, 0x00CE5718u), i);
            route_counted_stock(host, owner, bag, key);
        }
        const std::int32_t slots = native_at<std::int32_t>(host.property_bag_find_008f2260(bag, literal(host, 0x00CE56FCu)), 0x0C);
        for (std::int32_t i = 0; i < slots; ++i) {
            char key[32];
            host.format_to_buffer32_00467310(key, literal(host, 0x00CE56F4u), i + 1);
            route_counted_stock(host, owner, bag, key);
        }
    }
    if (class_name_is(host, class_name, 0x00CE56E8u)) {
        for (std::int32_t i = 1; i <= 20; ++i) {
            char key[32];
            host.format_to_buffer32_00467310(key, literal(host, 0x00CE56DCu), i);
            Object property = host.property_bag_find_008f2260(bag, key);
            if (property == nullptr || u32(property, 4) != 6) continue;
            Object sub = native_ptr(property, 0x0C);
            if (sub == nullptr) continue;
            if (native_at<std::int32_t>(host.property_bag_find_008f2260(sub, literal(host, 0x00CE4780u)), 0x0C) <= 0) continue;
            const std::int32_t type = native_at<std::int32_t>(host.property_bag_find_008f2260(sub, literal(host, 0x00CE4780u)), 0x0C);
            if (native_at<std::uint8_t>(host.property_bag_find_008f2260(bag, literal(host, 0x00CE5708u)), 0x0C) == 0)
                host.vehicle_class_stock_queue_append_0095c4d0(type);
            else
                scene_stock_register_unit_class_0046be90(host, owner, type);
        }
    }
    if (!class_name_is(host, class_name, 0x00CE56B8u)) return;
    Object japan = host.property_bag_find_008f2260(bag, literal(host, 0x00CE56D0u));
    Object japan_list = (japan != nullptr && u32(japan, 4) == 6) ? native_ptr(japan, 0x0C) : nullptr;
    const std::int32_t next = queue_list_stock(host, japan_list, 1);
    Object allied = host.property_bag_find_008f2260(bag, literal(host, 0x00CE56C4u));
    Object allied_list = (allied != nullptr && u32(allied, 4) == 6) ? native_ptr(allied, 0x0C) : nullptr;
    queue_list_stock(host, allied_list, next);
}

// 0046DC10, __thiscall(database, const char* name, second, const float pos[3],
// float heading), RET 10h. ESP-anchored from the listing: the frame is
// 0Ch (EH) + 94h + 10h (four saves), so the arguments sit at [S+B4h..C0h];
// locals: key {S+10h, S+14h}, find result {S+18h, S+1Ch}, database copy S+20h
// (reused for the new block), identity matrix S+24h..S+63h, placed frame
// S+64h..S+A3h. The 0046C550 RET 5Ch and the creator's 10h cleanup bring ESP
// back to S, which the epilogue's [ESP+A4h] read confirms.
Object scene_database_create_hidden_entity_at_0046dc10(Seg6Host& host, Object database, const char* name,
                                                       Object second, const float* position, float heading) {
    // 0046DC39..0046DCAA: find the hidden record by name, as in 004691B0.
    std::uint32_t key[2] = {0, 0};
    std::uint32_t found[2] = {0, 0};
    Object key_obj = reinterpret_cast<Object>(key);
    host.native_string_construct_0041e870(key_obj, name);
    Object map = database + 0x18;
    host.hidden_record_map_find_00468cd0(map, reinterpret_cast<Object>(found), key_obj);
    release_native_string(host, key_obj);
    Object container = as_object(found[0]);
    Object node = as_object(found[1]);
    Object head = native_ptr(map, 4);
    if (container == nullptr || container != map) host.checked_iterator_report_00bf6713();
    if (node == head) return nullptr;  // 0046DC95
    if (container == nullptr) host.checked_iterator_report_00bf6713();
    if (node == native_ptr(container, 4)) host.checked_iterator_report_00bf6713();
    Object record = native_ptr(node, 0x14);

    // 0046DCB2..0046DD04: the class row of record+0Ch through 00468FB0 on +34h;
    // the node's value is read without a null test.
    std::uint32_t class_key[2] = {0, 0};
    std::uint32_t bucket = 0;
    Object class_key_obj = reinterpret_cast<Object>(class_key);
    host.native_string_construct_0041e870(class_key_obj, reinterpret_cast<const char*>(native_ptr(record, 0x0C)));
    Object row = native_ptr(scene_class_map_find_node_00468fb0(host, database + 0x34, class_key_obj, &bucket), 8);
    release_native_string(host, class_key_obj);

    // 0046DD09..0046DD36: the parent entity named by the record's +54h string.
    Object parent = nullptr;
    if (u32(record, 0x54) != 0) {
        const char* parent_name = reinterpret_cast<const char*>(native_ptr(record, 0x58));
        if (parent_name == nullptr) parent_name = literal(host, 0x00E18560u);
        parent = host.entity_registry_find_by_name_00925a90(native_ptr(host.game_00e188a8(), 0x19CC), parent_name);
    }

    // 0046DD3A..0046DDA6: the record's frame (+14h, 16 dwords) with the
    // translation replaced by pos; the 3x3 part is rebuilt from Euler
    // (0, heading, 0) when the double 2*pi at 00CE3828 (0x401921FB60000000)
    // compares strictly above the heading (FCOMIP; an unordered heading skips).
    float frame[16];
    for (int i = 0; i < 16; ++i) native_at<std::uint32_t>(reinterpret_cast<Object>(frame), 4 * i) = u32(record, 0x14 + 4 * i);
    frame[12] = position[0];
    frame[13] = position[1];
    frame[14] = position[2];
    const double two_pi_00ce3828 = 6.283185482025146484375;  // exactly float(2*pi)
    if (two_pi_00ce3828 > static_cast<double>(heading)) matrix_set_rotation_from_euler_00467050(frame, 0.0f, heading, 0.0f);

    // 0046DDA8..0046DE4A: a literal identity (1.0f from 00D7A24C) passed by value.
    float identity[16];
    for (int i = 0; i < 16; ++i) identity[i] = (i % 5 == 0) ? 1.0f : 0.0f;

    // 0046DE4C..0046DE7C: the generation gate; a refusal returns 0.
    Object bag = native_ptr(record, 4);
    const std::int32_t class_id = native_at<std::int32_t>(row, 0);
    const char* class_name = reinterpret_cast<const char*>(
        scene_database_class_id_to_name_00468660(host, host.scene_database_00e18680(), class_id));
    if (!host.scene_entity_should_generate_0046c550(database, class_name, second, parent,
                                                    reinterpret_cast<Object>(frame), bag, 0, identity, 0))
        return nullptr;

    // 0046DE82..0046DEE2: the class creator at row+4, the 0Ch-byte bag holder
    // (0 when operator new fails) stored at entity+C0h, then 00925F20 with CL = 0.
    Object entity = host.class_row_create_entity_call(native_ptr(row, 4), native_at<std::int32_t>(row, 0), second,
                                                      parent, reinterpret_cast<Object>(frame), native_ptr(record, 4), 0);
    Object block = host.operator_new_00bf681b(0x0C);
    Object holder = block != nullptr ? host.property_bag_ref_construct_00922e20(block, native_ptr(record, 4)) : nullptr;
    set_ptr(entity, 0xC0, holder);
    host.sentity_init_all_00925f20(0);
    return entity;
}

// 0046EFB0, __fastcall(ECX database), RET: the destructor body of vtable
// 00CE59FC slot 0 (CG_scalar_deleting_dtor_0046F330).
void scene_database_destruct_0046efb0(Seg6Host& host, Object database) {
    set_u32(database, 0, 0x00CE59FCu);
    scene_database_release_records_and_pending_0046bd10(host, database);  // 0046EFDF
    std::uint32_t erased[2] = {0, 0};
    for (std::uint32_t set_offset : {0x164u, 0x158u}) {
        Object set = database + set_offset;
        Object head = native_ptr(set, 4);
        host.int_set_erase_range_0046b170(set, reinterpret_cast<Object>(erased), set, native_ptr(head, 0), set, head);
        host.free_00bf65ac(native_ptr(set, 4));
        set_u32(set, 4, 0);
        set_u32(set, 8, 0);
    }
    host.raw_list_clear_storage_004683b0(database + 0x14C);  // 0046F056
    release_native_string(host, database + 0x144);
    release_native_string(host, database + 0x13C);
    set_u32(database, 0x34, 0x00CE55B0u);
    scene_class_map_destruct_0046b240(host, database + 0x34);  // 0046F0B9
    if (native_ptr(database, 0x28) != nullptr) host.free_00bf65ac(native_ptr(database, 0x28));
    set_u32(database, 0x28, 0);
    set_u32(database, 0x2C, 0);
    set_u32(database, 0x30, 0);
    Object map = database + 0x18;
    Object head = native_ptr(map, 4);
    host.hidden_record_map_erase_range_0046b570(map, reinterpret_cast<Object>(erased), map, native_ptr(head, 0), map, head);
    host.free_00bf65ac(native_ptr(map, 4));
    set_u32(map, 4, 0);
    set_u32(map, 8, 0);
    if (native_ptr(database, 0x0C) != nullptr) host.free_00bf65ac(native_ptr(database, 0x0C));
    set_u32(database, 0x0C, 0);
    set_u32(database, 0x10, 0);
    set_u32(database, 0x14, 0);
}

// 0046F160, __fastcall(ECX database), RET, returns database. The head-node
// constructors are 00468260 (isnil byte +19h) and 004682B0 (isnil +11h).
// The set at +158h receives 44h, 5Eh, 1Dh, 47h, 3Dh; the set at +164h
// receives 47h, 3Dh.
Object scene_database_construct_0046f160(Seg6Host& host, Object database) {
    set_u32(database, 0, 0x00CE59FCu);
    set_u32(database, 0x0C, 0);
    set_u32(database, 0x10, 0);
    set_u32(database, 0x14, 0);
    {
        Object map = database + 0x18;
        Object head = host.hidden_record_map_buy_head_node_00468260(map);
        set_ptr(map, 4, head);
        native_at<std::uint8_t>(head, 0x19) = 1;
        set_ptr(native_ptr(map, 4), 4, native_ptr(map, 4));
        set_ptr(native_ptr(map, 4), 0, native_ptr(map, 4));
        set_ptr(native_ptr(map, 4), 8, native_ptr(map, 4));
        set_u32(map, 8, 0);
    }
    set_u32(database, 0x28, 0);
    set_u32(database, 0x2C, 0);
    set_u32(database, 0x30, 0);
    set_u32(database, 0x34, 0x00CE55B0u);
    set_u32(database, 0x38, 0);
    for (std::uint32_t i = 0; i < 0x40; ++i) set_u32(database, 0x3C + 4 * i, 0);  // REP STOSD
    for (std::uint32_t offset = 0x13C; offset <= 0x154; offset += 4) set_u32(database, offset, 0);
    for (std::uint32_t set_offset : {0x158u, 0x164u}) {
        Object set = database + set_offset;
        Object head = host.int_set_buy_head_node_004682b0(set);
        set_ptr(set, 4, head);
        native_at<std::uint8_t>(head, 0x11) = 1;
        set_ptr(native_ptr(set, 4), 4, native_ptr(set, 4));
        set_ptr(native_ptr(set, 4), 0, native_ptr(set, 4));
        set_ptr(native_ptr(set, 4), 8, native_ptr(set, 4));
        set_u32(set, 8, 0);
    }
    native_at<std::uint8_t>(database, 4) = 1;  // 0046F268
    // 0Ch result (owner, node, inserted byte + 3 untouched): the image's locals
    // after SUB ESP,14h are the key at [ESP+10h] (0046F26C) and the result at
    // [ESP+18h] (0046F275 LEA ECX,[ESP+1Ch] after one push), 18h..23h, up to the
    // pushed SEH link at [ESP+24h]. ([ESP+14h] holds the database, 0046F186.)
    std::uint32_t inserted[3] = {0, 0, 0};
    for (std::int32_t key : {0x44, 0x5E, 0x1D, 0x47, 0x3D})
        host.int_set_insert_0046ac50(database + 0x158, reinterpret_cast<Object>(inserted), &key);
    for (std::int32_t key : {0x47, 0x3D})
        host.int_set_insert_0046ac50(database + 0x164, reinterpret_cast<Object>(inserted), &key);
    return database;
}

// 0046F350, cdecl(), RET: operator new(170h), construct, store the result
// (0 when the allocation failed) in 00E18680.
void scene_database_create_singleton_0046f350(Seg6Host& host) {
    Object block = host.operator_new_00bf681b(0x170);
    host.set_scene_database_00e18680(block != nullptr ? scene_database_construct_0046f160(host, block) : nullptr);
}

// 0046F3E0, cdecl(), RET: the deleting destructor (slot 0, 1) of [00E18680]
// when set, then 00E18680 = 0.
void scene_database_destroy_singleton_0046f3e0(Seg6Host& host) {
    Object database = host.scene_database_00e18680();
    if (database != nullptr) {
        host.object_deleting_destructor_vcall_00(database, 1);
        host.set_scene_database_00e18680(nullptr);
    }
}

// ---------------------------------------------------------------------------
// Visual class entities, cloud entity, path entity.
// ---------------------------------------------------------------------------

// 00472530 and 00473E30: each is a single JMP 0048DBD0 (the 37h entity base
// SEntityInit), __fastcall(ECX entity).
void class_entity37_sentity_init_00472530(Seg6Host& host, Object entity) { host.sentity_init_base_0048dbd0(entity); }
void class_entity37_sentity_init_00473e30(Seg6Host& host, Object entity) { host.sentity_init_base_0048dbd0(entity); }

// 00475990, __fastcall(ECX entity), RET: the cloud entity destructor body.
void cloud_entity_destruct_00475990(Seg6Host& host, Object entity) {
    set_u32(entity, 0x00, 0x00CE5E68u);
    set_u32(entity, 0x10, 0x00CE5E50u);
    set_u32(entity, 0x24, 0x00CE5E48u);
    Object instance = native_ptr(entity, 0x1A4);
    if (instance != nullptr) {
        Object game = host.game_00e188a8();
        if (native_ptr(game, 0x19E8) != nullptr) {
            Object manager = host.atmosphere_cloud_manager_00bbddf0(native_ptr(game, 0x19E8));
            host.cloud_manager_release_instance_00bbd910(manager, instance);
        }
    }
    host.class_entity37_destruct_base_00487380(entity);  // 004759FF
}

// 00476550, __thiscall({data, size, capacity}*, n), RET 4: 20h-byte elements
// (four dwords, three floats copied by FLD/FSTP, one dword).
void cloud_particle_array_reserve_00476550(Seg6Host& host, Object array, std::int32_t count) {
    if (count < 1) count = 1;
    if (native_at<std::int32_t>(array, 8) < count) {
        Object block = host.operator_new_00bf55be(static_cast<std::uint32_t>(count) << 5);
        for (std::int32_t i = 0; i < native_at<std::int32_t>(array, 4); ++i) {
            Object dest = block + 0x20 * i;
            if (dest != nullptr) {
                Object src = native_ptr(array, 0) + 0x20 * i;
                for (std::uint32_t k = 0; k < 4; ++k) set_u32(dest, 4 * k, u32(src, 4 * k));
                for (std::uint32_t k = 4; k < 7; ++k) native_at<float>(dest, 4 * k) = native_at<float>(src, 4 * k);
                set_u32(dest, 0x1C, u32(src, 0x1C));
            }
        }
        host.free_00bf6989(native_ptr(array, 0));
        set_ptr(array, 0, block);
        native_at<std::int32_t>(array, 8) = count;
    }
}

// 00476BD0, __fastcall(ECX entity), RET: cloud entity vtable slot 00CE5F10.
void cloud_entity_create_visual_00476bd0(Seg6Host& host, Object entity) {
    Object cloud_class = native_ptr(entity, 0x188);
    if (native_ptr(cloud_class, 0x34) == nullptr) {
        set_u32(entity, 0x178, 0);
        Object manager = host.atmosphere_cloud_manager_00bbddf0(native_ptr(host.game_00e188a8(), 0x19E8));
        set_ptr(entity, 0x1A4, host.cloud_manager_create_instance_00bbd770(manager, native_ptr(cloud_class, 0x28)));
        return;
    }
    Object block = host.operator_new_00bf681b(0x1AC);
    Object part = nullptr;
    if (block != nullptr) {
        Object model = host.model_source_vcall_08(native_ptr(native_ptr(entity, 0x170), 0x10), 6, 1.0f);
        part = host.unit_part_instance_construct_007135c0(block, entity, model);
    }
    set_ptr(entity, 0x178, part);
    Object parent = native_ptr(entity, 0x3C);
    if (parent != nullptr && host.entity_is_kind_vcall_5c(parent, 4)) {
        Object walk = native_ptr(entity, 0x3C);
        for (;;) {
            Object visual = native_ptr(walk, 0x360);
            if (visual != nullptr && native_ptr(visual, 0x160) != nullptr) break;
            if (native_ptr(walk, 0x3C) == nullptr) break;
            if (!host.entity_is_kind_vcall_5c(native_ptr(walk, 0x3C), 4)) break;
            walk = native_ptr(walk, 0x3C);
        }
        Object visual = native_ptr(walk, 0x360);
        Object instance = visual != nullptr ? native_ptr(visual, 0x160) : nullptr;
        Object parent_node = native_ptr(instance, 0x0C);  // no null test in the image (00476CE2)
        host.node_set_parent_00b6e680(native_ptr(native_ptr(native_ptr(entity, 0x178), 0x160), 0x0C), parent_node);
        return;
    }
    Object node = native_ptr(native_ptr(native_ptr(entity, 0x178), 0x160), 0x0C);
    Object child = native_ptr(node, 0x34);
    set_u32(node, 0x48, u32(node, 0x48) | 0x40000000u);
    for (; child != nullptr; child = native_ptr(child, 0x3C)) host.node_or_flags_recursive_00475e60(child, 0x40000000u);
    Object root = native_ptr(host.game_00e188a8(), 0x19EC);
    host.node_propagate_root_registration_00b6d890(native_ptr(native_ptr(native_ptr(entity, 0x178), 0x160), 0x0C), root);
}

// 00476D80, __fastcall(ECX entity), RET: vtable 00CE5E88 slot 46. The class
// field +34h is re-read after the instance call.
void cloud_entity_update_visibility_00476d80(Seg6Host& host, Object entity) {
    if (native_ptr(native_ptr(entity, 0x188), 0x34) == nullptr) {
        if (native_ptr(entity, 0x1A4) == nullptr) return;
        host.cloud_instance_vcall_34(native_ptr(entity, 0x1A4), entity + 0x74);
        if (native_ptr(native_ptr(entity, 0x188), 0x34) != nullptr) return;
        host.cloud_instance_invalidate_lighting_00bbe760(native_ptr(entity, 0x1A4), 0);
        return;
    }
    Object part = native_ptr(entity, 0x178);
    if (part == nullptr) return;
    Object node = native_ptr(native_ptr(part, 0x160), 0x0C);
    if (node == nullptr) return;
    host.scene_node_vcall_38(node, entity + 0x74);
    Object fade = native_ptr(entity, 0x174);
    if (fade == nullptr) return;
    host.scene_node_set_visibility_factor_00b6da70(native_ptr(native_ptr(native_ptr(entity, 0x178), 0x160), 0x0C),
                                                   native_at<float>(fade, 0x1C), 1);
}

// 00477180, __thiscall(entity, writer), RET 4: the base writer 00924D90, then
// writer vtable +0Ch with two by-value {0, text} pairs: "cloudclass"
// (00CE5FC4) and the class name (class+8, or the literal 00E186A4 when null).
void cloud_entity_write_scene_properties_00477180(Seg6Host& host, Object entity, Object writer) {
    host.sentity_write_scene_properties_00924d90(entity, writer);
    const char* name = reinterpret_cast<const char*>(native_ptr(native_ptr(entity, 0x188), 8));
    if (name == nullptr) name = reinterpret_cast<const char*>(host.image_address(0x00E186A4u));
    host.writer_vcall_0c(writer, 0, reinterpret_cast<const char*>(host.image_address(0x00CE5FC4u)), 0, name);
}

// 004772C0, __thiscall(bag, const char* name), RET 4. The NativeString copy is
// searched first; on a hit it is searched again, on a miss the original C
// string is searched; the second result is returned.
Object property_bag_find_by_cstring_004772c0(Seg6Host& host, Object bag, const char* name) {
    std::uint32_t text[2] = {0, 0};
    Object text_obj = reinterpret_cast<Object>(text);
    std::uint32_t length = 0;
    if (name != nullptr) while (name[length] != '\0') ++length;
    host.native_string_resize_0041dd40(text_obj, length, 0);
    char* data = reinterpret_cast<char*>(native_ptr(text_obj, 4));
    const std::uint32_t size = u32(text_obj, 0);
    const char* lookup = reinterpret_cast<const char*>(host.image_address(0x00E186A4u));
    if (data != nullptr) {
        host.crt_memcpy_00bf7680(data, name, size);
        lookup = data;
    }
    const char* second = name;
    if (host.property_bag_find_008f2260(bag, lookup) != nullptr)
        second = data != nullptr ? data : reinterpret_cast<const char*>(host.image_address(0x00E186A4u));
    Object result = host.property_bag_find_008f2260(bag, second);
    release_string_block(host, data, size);
    return result;
}

// 0047B660, __thiscall(entity, int id), RET 4, returns entity.
Object path_entity_construct_0047b660(Seg6Host& host, Object entity, std::int32_t id) {
    host.game_entity_construct_00928630(entity, 0, id);
    host.path_component_construct_007b28c0(entity + 0x1E4, entity);
    set_u32(entity, 0x1E4, 0x00CE63E8u);
    set_u32(entity, 0x000, 0x00CE6290u);
    set_u32(entity, 0x010, 0x00CE6274u);
    set_u32(entity, 0x024, 0x00CE626Cu);
    set_u32(entity, 0x170, 0x00CE6268u);
    set_u32(entity, 0x0C4, 0x47);
    return entity;
}

// 0047B700, __fastcall(ECX entity), RET: the path entity destructor body.
void path_entity_destruct_0047b700(Seg6Host& host, Object entity) {
    set_u32(entity, 0x000, 0x00CE6290u);
    set_u32(entity, 0x010, 0x00CE6274u);
    set_u32(entity, 0x024, 0x00CE626Cu);
    set_u32(entity, 0x170, 0x00CE6268u);
    set_u32(entity, 0x1E4, 0x00CE63E8u);
    host.path_component_destruct_007b1bd0(entity + 0x1E4);
    host.game_entity_destruct_009287b0(entity);
}

}  // namespace bsp::cc13_seg6

// oct18b-nA1: newly defined, assembly-reviewed leaves; build-only interfaces.

namespace bsp::cc13_seg6 {
void anim_entity_advance_time_004710d0(Object entity, AnimAdvanceHost& host) {
    Object game = host.game_00e188a8();
    const float sum = static_cast<float>(static_cast<double>(native_at<float>(game, 0x21f0)) +
                                         native_at<float>(entity, 0x190));
    const float time = static_cast<float>(host.cifmod_00bf857a(sum, native_at<float>(entity, 0x18c)));
    native_at<float>(entity, 0x190) = time;
    Object visual = native_ptr(entity, 0x178);
    Object resource = native_ptr(visual, 0x160);
    host.animation_set_time_24(native_ptr(resource, 0xc), 0, time, 1);
    host.scene_refresh_visibility_00484940(entity);
}
void cloud_mark_dirty_00475a20(Object entity) noexcept { native_at<std::uint8_t>(entity, 0x1a8) = 1; }
Object cloud_animation_00476bb0(Object entity) noexcept {
    Object visual = native_ptr(entity, 0x178);
    return visual ? native_ptr(native_ptr(visual, 0x160), 0xc) : nullptr;
}
bool vehicle_accumulate_hit_0047a660(Object entity, Object hit, VehicleHitHost& host) {
    // COMISS/JA accepts unordered values at +150h, unlike a C++ <= test.
    if (native_at<float>(entity, 0x150) > 0.0f) return true;
    Object definition = native_ptr(native_ptr(entity, 0x188), 0x28);
    const float armour = native_at<float>(definition, 0x4c);
    std::uint32_t part;
    const float damage = static_cast<float>(native_at<std::int32_t>(hit, 0x34) == -1
        ? host.worst_part_004706d0(hit, &part, armour) : host.hull_damage_00470510(hit, armour));
    // COMISS zero,damage / JNC: NaN continues too.
    if (damage <= 0.0f) return true;
    const float total = static_cast<float>(static_cast<double>(damage) + native_at<float>(entity, 0x18c));
    native_at<float>(entity, 0x18c) = total;
    // FCOMIP total,100 / JC: unordered exits here.
    if (!(static_cast<double>(total) >= 100.0)) return true;
    if (!native_at<std::uint8_t>(entity, 0xc8)) host.refresh_pose_00414db0(entity);
    Object game = host.game_00e188a8();
    definition = native_ptr(native_ptr(entity, 0x188), 0x28);
    Object effect_definition = native_ptr(definition, 0x138);
    if (effect_definition) host.add_reference_ce221c(effect_definition);
    Object effect_handle = nullptr;
    Object* returned = host.create_point_effect_008685e0(&effect_handle, native_ptr(game, 0x19ec),
        effect_definition, reinterpret_cast<const float*>(entity + 0xfc), 0, 0);
    Object effect = *returned;
    if (effect_handle && host.release_reference_ce2220(effect_handle) == 0)
        host.destroy_vcall_00(effect_handle);
    native_at<std::uint8_t>(effect, 9) = 1;
    definition = native_ptr(native_ptr(entity, 0x188), 0x28);
    Object secondary = native_ptr(definition, 0x13c);
    if (secondary) host.vehicle_effect_0047bb70(native_ptr(host.game_00e188a8(), 0x21d0), secondary, entity);
    host.mission_kill_00926d90(entity, 1);
    return true;
}
std::uint32_t path_class_kind_0047b790() noexcept { return 0x12; }
}
