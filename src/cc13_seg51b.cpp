// cc13_seg51b: the last three unreconstructed bodies of segment 51. See
// include/bsp/cc13_seg51b.hpp and docs/CC13_SEG51B.md. Not wired; build-tested only.

#include "bsp/cc13_seg51b.hpp"

namespace bsp::cc13_seg51b {

namespace {

// [00D7A24C], float 1.0 (the matrix diagonal in 0081C550).
constexpr float kOne = 1.0f;

// The inlined native string release: 00419CC0(data, size + 1, 1) then 00BD1510 on the pool.
void release_native_string(Seg51bHost& host, NativeString& string) {
    if (string.data != nullptr) {
        Object pool = host.sized_storage_pool_get_singleton_00419cc0();
        host.sized_storage_pool_return_block_00bd1510(
            pool, reinterpret_cast<Object>(string.data), string.size + 1u, 1u);
    }
}

// Resize to the literal's length, then memcpy(data, literal, size + 1) when data is non-null.
void set_native_string(Seg51bHost& host, NativeString& string, const char* text,
                       std::uint32_t length) {
    string.size = 0;
    string.data = nullptr;
    host.native_string_resize_0041dd40(&string, length, 1u);
    if (string.data != nullptr) {
        host.memcpy_00bf7680(string.data, text, string.size + 1u);
    }
}

// The refcounted release: InterlockedDecrement(+4), vtable[0] at zero.
void release_ref(Seg51bHost& host, Object object) {
    if (host.interlocked_decrement_00ce2220(&field<std::int32_t>(object, 4)) == 0) {
        host.refcounted_vslot_00(object);
    }
}

} // namespace

// 0081BA80-0081BEA0. Frame: Lua state owner at ESP+74h, LuaObjects at +4Ch (the guncamos table),
// +38h (the entry), +60h and +24h (temporaries); native strings at +10h and +18h.
void gun_camos_write_entry_0081ba80(Seg51bHost& host, std::int32_t index, const float* colour) {
    alignas(4) std::uint8_t owner[0x4C8] = {};
    alignas(4) std::uint8_t camos[0x14] = {};
    alignas(4) std::uint8_t entry[0x14] = {};
    alignas(4) std::uint8_t temp_a[0x14] = {};
    alignas(4) std::uint8_t temp_b[0x14] = {};

    host.lua_state_owner_construct_00b66bd0(owner);  // 0081BAA7
    host.lua_state_owner_open_00b6a020(owner, 1u);   // 0081BABB
    host.lua_object_construct_00b65f50(camos);       // 0081BAC4
    host.lua_object_construct_00b65f50(entry);       // 0081BAD5

    // 0081BADA-0081BB5D: exists = fs->vtable[8]("guncamos.lua").
    NativeString path{};
    set_native_string(host, path, "guncamos.lua", 0xCu);
    Object fs = host.file_system_0109ceec();
    const std::uint8_t exists = host.file_system_vslot_08_exists(fs, &path);
    release_native_string(host, path);

    NativeString name{};
    if (exists != 0) {
        // 0081BB6C-0081BC01: run guncamos.lua, then camos = globals["guncamos"].
        host.native_string_assign_0041e870(&name, "guncamos.lua");
        host.lua_state_owner_run_script_00b69d40(owner, &name, nullptr);
        release_native_string(host, name);
        Object globals = host.lua_state_owner_get_globals_00b67980(owner, temp_a);
        Object value = host.lua_object_get_by_name_00b67800(globals, temp_b, "guncamos");
        host.lua_object_assign_00b67690(camos, value);
        host.lua_object_destruct_00b67700(temp_b);  // 0081BBF8
        host.lua_object_destruct_00b67700(temp_a);  // 0081BCC6 (via 0081BCBF)
    } else {
        // 0081BC06-0081BCC6: globals.guncamos = {}, then camos = globals["guncamos"].
        host.native_string_assign_0041e870(&name, "guncamos");
        Object globals = host.lua_state_owner_get_globals_00b67980(owner, temp_a);
        host.lua_object_set_new_table_00b67580(globals, &name);
        host.lua_object_destruct_00b67700(temp_a);  // 0081BC46
        release_native_string(host, name);
        globals = host.lua_state_owner_get_globals_00b67980(owner, temp_b);
        Object value = host.lua_object_get_by_name_00b67800(globals, temp_a, "guncamos");
        host.lua_object_assign_00b67690(camos, value);
        host.lua_object_destruct_00b67700(temp_a);  // 0081BCB6
        host.lua_object_destruct_00b67700(temp_b);  // 0081BCC6
    }

    // 0081BCCB-0081BD47: camos[index] = {}; entry = camos[index]; entry[1..4] = colour[0..3].
    host.lua_object_set_new_table_at_index_00b66670(camos, index);
    Object slot = host.lua_object_get_by_index_00b67720(camos, temp_a, index);
    host.lua_object_assign_00b67690(entry, slot);
    host.lua_object_destruct_00b67700(temp_a);
    host.lua_object_set_numeric_element_00b66580(entry, 1, colour[0]);
    host.lua_object_set_numeric_element_00b66580(entry, 2, colour[1]);
    host.lua_object_set_numeric_element_00b66580(entry, 3, colour[2]);
    host.lua_object_set_numeric_element_00b66580(entry, 4, colour[3]);

    // 0081BD4C-0081BDC3: stream = fs->vtable[4]("guncamos.lua", 35h).
    set_native_string(host, path, "guncamos.lua", 0xCu);
    fs = host.file_system_0109ceec();
    Object stream = host.file_system_vslot_04_open(fs, &path, 0x35u);
    release_native_string(host, path);

    // 0081BDC3-0081BE38: camos written as "guncamos" source text (0, 0, 1, 0).
    set_native_string(host, name, "guncamos", 8u);
    host.lua_object_write_lua_source_text_00b6a390(camos, stream, &name, 0u, 0u, 1u, 0u);
    release_native_string(host, name);
    if (stream != nullptr) {
        release_ref(host, stream);  // 0081BE38-0081BE50
    }

    host.lua_object_destruct_00b67700(entry);     // 0081BE5E
    host.lua_object_destruct_00b67700(camos);     // 0081BE6F
    host.lua_state_owner_close_00b669a0(owner);   // 0081BE83
}

// 0081C1F0-0081C2FD.
void refptr_array_push_back_0081c1f0(Seg51bHost& host, Object array, const Object* ref) {
    Object*& data = field<Object*>(array, 0);
    std::uint32_t& count = field<std::uint32_t>(array, 4);
    std::uint32_t& capacity = field<std::uint32_t>(array, 8);

    if (count == capacity) {
        // 0081C217: the capacity is stored before the allocation.
        capacity = capacity * 2u + 2u;
        Object* grown = reinterpret_cast<Object*>(host.operator_new_00bf55be(capacity * 4u));
        for (std::uint32_t i = 0; i < count; ++i) {
            // 0081C241-0081C262: copy with a reference increment (skipped for a null slot
            // address, the placement-new test on grown + i).
            Object* dest = grown + i;
            if (dest != nullptr) {
                *dest = nullptr;
                Object element = data[i];
                if (element != nullptr) {
                    *dest = element;
                    host.interlocked_increment_00ce221c(&field<std::int32_t>(element, 4));
                }
            }
            // 0081C266-0081C294: release the old element.
            Object old = data[i];
            if (old != nullptr) {
                release_ref(host, old);
                data[i] = nullptr;
            }
        }
        if (data != nullptr) {
            host.operator_delete_00bf6989(reinterpret_cast<Object>(data));
        }
        data = grown;
    }

    // 0081C2C2-0081C2EA: append with a reference increment; count++ even for a null slot.
    Object* slot = data + count;
    if (slot != nullptr) {
        *slot = nullptr;
        Object element = *ref;
        if (element != nullptr) {
            *slot = element;
            host.interlocked_increment_00ce221c(&field<std::int32_t>(element, 4));
        }
    }
    count += 1u;
}

// 0081C550-0081C820.
void unit_instance_toggle_deck_visuals_0081c550(Seg51bHost& host, Object unit) {
    // 0081C56B-0081C57E: flip +749h.
    std::uint8_t& visible = field<std::uint8_t>(unit, 0x749);
    visible = visible == 0 ? 1 : 0;

    // 0081C584-0081C610: every 44h-byte record of the vector at +660h (begin +664h, end +668h)
    // with a node gets visibility factor 1.0 (visible) or 0.0, tail 0.
    Object vector = unit + 0x660;
    Object it = field<Object>(vector, 4);
    if (field<Object>(vector, 8) < it) {
        host.invalid_parameter_00bf6713();
    }
    for (;;) {
        Object end = field<Object>(vector, 8);
        if (field<Object>(vector, 4) > end) {
            host.invalid_parameter_00bf6713();
        }
        if (it == end) {
            break;
        }
        if (it >= field<Object>(vector, 8)) {
            host.invalid_parameter_00bf6713();
        }
        if (field<Object>(it, 0) != nullptr) {
            if (it >= field<Object>(vector, 8)) {
                host.invalid_parameter_00bf6713();
            }
            const float factor = visible != 0 ? 1.0f : 0.0f;  // FLD1 / FLDZ
            host.scene_node_set_visibility_factor_00b6da70(field<Object>(it, 0), factor, 0);
        }
        if (it >= field<Object>(vector, 8)) {
            host.invalid_parameter_00bf6713();
        }
        it += 0x44;
    }

    Object effects = unit + 0xA14;  // {data, count}
    if (visible == 0) {
        // 0081C61F-0081C697: stop and drop every live effect, then resize to 0.
        Object* cursor = field<Object*>(unit, 0xA14);
        Object* last = cursor + field<std::uint32_t>(unit, 0xA18);
        for (; cursor != last; ++cursor) {
            Object effect = *cursor;
            if (effect == nullptr) {
                continue;
            }
            host.point_effect_stop_children_00867b10(effect);
            field<std::uint8_t>(effect, 9) = 1;
            Object held = *cursor;
            if (held != nullptr) {
                release_ref(host, held);
                *cursor = nullptr;
            }
            *cursor = nullptr;
        }
        host.live_effect_references_resize_004c9550(effects, 0u);
    } else {
        // 0081C69C-0081C7EE: one point effect per 16-byte position of the class's list
        // (+678h, count +67Ch) when its definition +674h is set, parented to +4A4h.
        Object unit_class = field<Object>(unit, 0x538);
        if (field<Object>(unit_class, 0x674) != nullptr) {
            const std::uint8_t* position = field<std::uint8_t*>(unit_class, 0x678);
            const std::uint8_t* position_end =
                position + field<std::uint32_t>(unit_class, 0x67C) * 16u;
            for (; position != position_end; position += 16) {
                // Identity rotation with the position as the translation row (x, y, z, 1).
                float matrix[16] = {};
                matrix[0] = kOne;
                matrix[5] = kOne;
                matrix[10] = kOne;
                matrix[12] = reinterpret_cast<const float*>(position)[0];
                matrix[13] = reinterpret_cast<const float*>(position)[1];
                matrix[14] = reinterpret_cast<const float*>(position)[2];
                matrix[15] = kOne;
                Object parent = field<Object>(unit, 0x4A4);
                Object definition = field<Object>(field<Object>(unit, 0x538), 0x674);
                if (definition != nullptr) {
                    host.interlocked_increment_00ce221c(&field<std::int32_t>(definition, 4));
                }
                Object created = nullptr;
                host.point_effect_create_with_parent_matrix_008687c0(&created, parent,
                                                                     definition, matrix, 1u,
                                                                     0u, 0u);
                if (created != nullptr) {
                    host.live_effect_references_append_0074d780(effects, &created);
                }
                if (created != nullptr) {
                    release_ref(host, created);
                    created = nullptr;
                }
            }
        }
    }

    // 0081C7F4-0081C809: the kotel/rope nodes follow the same flag.
    host.scene_node_toggle_kotel_rope_nodes_00815d20(unit, field<Object>(unit, 0x4A4),
                                                     visible != 0 ? 1 : 0);
}

} // namespace bsp::cc13_seg51b
