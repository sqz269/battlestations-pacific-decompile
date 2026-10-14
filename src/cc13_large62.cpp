// Packet cc13-large62: the large segment 62/65 functions cc13-seg62 left. See
// include/bsp/cc13_large62.hpp and docs/CC13_LARGE62.md. Names are hypotheses;
// build-tested only.

#include "bsp/cc13_large62.hpp"

#include <cstring>

namespace bsp::cc13_large62 {

using cc13_seg62::addr;
using cc13_seg62::at;
using cc13_seg62::obj;

namespace {

const char* cstr(std::uint32_t address) noexcept {
    return reinterpret_cast<const char*>(obj(address));
}

// The inlined native string destructor: when the data pointer is set, return
// size + 1 bytes to the pool (00419CC0, 00BD1510).
void release_string(Large62Host& host, const std::uint32_t* string) {
    if (string[1] != 0) {
        const Object pool = host.pool_singleton_00419cc0();
        host.pool_return_block_00bd1510(pool, obj(string[1]), string[0] + 1, 1);
    }
}

// The inlined string assignment: unless dest is source, resize dest to the
// source size with fill 1 (0041DD40) and copy the bytes when that size is
// non-zero (memcpy count is dest's new size).
void assign_string(Large62Host& host, Object dest, Object source) {
    if (dest == source) {
        return;
    }
    const std::uint32_t size = at<std::uint32_t>(source, 0x0);
    host.string_resize_0041dd40(dest, size, 1);
    if (size != 0) {
        host.memcpy_00bf7680(obj(at<std::uint32_t>(dest, 0x4)), obj(at<std::uint32_t>(source, 0x4)),
                             at<std::uint32_t>(dest, 0x0));
    }
}

// The inlined find() tail both trees in 0097B5E0 share: lower_bound's node is
// the end unless it is not the head and the key does not order before the
// node's key (+0Ch size, +10h data). An empty key orders before any non-empty
// node key; an empty node key compares equal; otherwise _stricmp. The
// checked-iterator tests that follow (iterator container null or not the
// tree) compare the tree with itself and cannot fire, so they are not spelled
// out.
bool key_orders_before_node(Large62Host& host, Object key, Object node) {
    std::int32_t order;
    if (at<std::uint32_t>(key, 0x0) == 0) {
        order = at<std::uint32_t>(node, 0xc) != 0 ? -1 : 0;
    } else if (at<std::uint32_t>(node, 0xc) == 0) {
        order = 0;
    } else {
        order = host.stricmp_00bf7fbf(cstr(at<std::uint32_t>(key, 0x4)), cstr(at<std::uint32_t>(node, 0x10)));
    }
    return order < 0;
}

// The checked index test before each names[i]: begin null or i past the end
// reports through 00BF6713.
Object checked_name(Large62Host& host, Object names, std::uint32_t index) {
    const std::uint32_t first = at<std::uint32_t>(names, 0x4);
    if (first == 0 || index >= ((at<std::uint32_t>(names, 0x8) - first) >> 3)) {
        host.invalid_parameter_00bf6713();
    }
    return obj(at<std::uint32_t>(names, 0x4) + index * 8);
}

}  // namespace

// 0097B5E0 __thiscall(ECX = manager; out string*, entry by value, suffix by
// value, const vector<string>* names), RET 24h. entry is {string +0 (the
// separator set whose data 0097D650 passes to strcspn), map<string,string>
// +8 (head +0Ch, size +10h)}; the caller copies manager+11Ch into it.
// Keeps a copy of *out; for each name, in order, that the entry map holds:
// restores *out from the copy, then 0096EDD0(out, &suffix, map[name])
// (0049C9C0 lookup-or-insert, though the name is present), and stops at the
// first result the message map [[00E188A8]+21DCh]+4 holds (00702300 plus the
// inlined find). Without a hit *out keeps the last key built, or is untouched
// when no name matched. Then destroys the copy, the entry (0097A190) and the
// suffix. The null test on the message map pointer (+4 of a loaded pointer)
// is the image's. SEH 00CAB3F8 (states 1, 2) not modelled.
void resolve_message_key_0097b5e0(Large62Host& host, Object manager, Object out, Object entry, Object suffix,
                                  Object names) {
    std::uint32_t saved[2] = {0, 0};
    const Object saved_string = reinterpret_cast<Object>(saved);
    assign_string(host, saved_string, out);

    const Object tree = entry + 0x8;
    if (host.string_vector_size_00427ac0(names) != 0) {
        std::uint32_t index = 0;
        do {
            const std::uint32_t end_node = at<std::uint32_t>(entry, 0xc);
            const Object key = checked_name(host, names, index);
            Object node = host.tree_lower_bound_00485790(tree, key);
            if (addr(node) == at<std::uint32_t>(entry, 0xc) || key_orders_before_node(host, key, node)) {
                node = obj(at<std::uint32_t>(entry, 0xc));
            }
            if (addr(node) != end_node) {
                assign_string(host, out, saved_string);
                const Object mapped = host.string_map_lookup_or_insert_0049c9c0(tree, checked_name(host, names, index));
                host.build_message_key_0096edd0(manager, out, suffix, mapped);

                const Object messages = obj(at<std::uint32_t>(obj(host.global_u32(0x00e188a8)), 0x21dc));
                const std::uint32_t message_end = at<std::uint32_t>(messages, 0x8);
                const Object message_tree = messages + 0x4;
                Object hit = host.message_map_lower_bound_00702300(message_tree, out);
                if (message_tree == nullptr) {
                    host.invalid_parameter_00bf6713();
                }
                if (addr(hit) == at<std::uint32_t>(message_tree, 0x4) || key_orders_before_node(host, out, hit)) {
                    hit = obj(at<std::uint32_t>(message_tree, 0x4));
                }
                if (addr(hit) != message_end) {
                    break;
                }
            }
            ++index;
        } while (index < host.string_vector_size_00427ac0(names));
    }

    release_string(host, saved);
    host.key_entry_destroy_0097a190(entry);
    release_string(host, reinterpret_cast<const std::uint32_t*>(suffix));
}

// 0097D650 __thiscall(ECX = manager; text string*, entity), RET 8. Returns at
// once when text has no data, is empty, the manager's separator set (the data
// pointer +120h of the string +11Ch) is null, or strcspn(text, set) is the
// text size or -1. Otherwise:
//   tail = 00469840(text, pos, 1)  (by that function's ledger order: start pos,
//          count 1, the separator character itself)
//   prefix = copy of tail, then += the relation word: table 00E0B080 indexed
//          by 3 without an entity, else by 00803CE0(local party, entity)
//   keys = vector<string>; each candidate is prefix + name (004261A0) appended
//          by 00450540:
//     - vtable[10h]() name, when the entity's recon level for the local party
//       (+1E8h + party*34h: +8h if byte +10h is set, else +4h) is >= 2
//     - vtable[28h]() name, same test (re-read)
//     - table 00E0B590[(entity+170h)->vtable[0]()], same test (re-read)
//     - one default by kind: vtable[5Ch](6) "default_ship", (0Fh)
//       "default_plane", (18h) "default_planesquadron", else 00922A90
//       "default_landobject" (its two temporaries freed by 0041DD20), else none
//   0097B5E0(text, copy of manager+11Ch (0097BD10), copy of tail, &keys)
// then frees prefix, the keys (00432050 over [first, last) with the image's
// two junk stack words, here the manager and the vector, then _free(first))
// and tail. The entity is dereferenced without a null test after the
// relation step, as in the image. SEH 00CAB948 not modelled.
void resolve_entity_sound_keys_0097d650(Large62Host& host, Object manager, Object text, Object entity) {
    const std::uint32_t text_data = at<std::uint32_t>(text, 0x4);
    const std::uint32_t separators = at<std::uint32_t>(manager, 0x120);
    if (text_data == 0 || at<std::uint32_t>(text, 0x0) == 0 || separators == 0) {
        return;
    }
    const std::uint32_t pos = host.strcspn_00bf9a50(cstr(text_data), cstr(separators));
    if (pos == at<std::uint32_t>(text, 0x0) || pos == 0xffffffffu) {
        return;
    }

    std::uint32_t tail[2];
    host.substring_00469840(text, reinterpret_cast<Object>(tail), pos, 1);
    // keys: +0 allocator word (never written by the image), +4 first, +8 last, +0Ch end.
    std::uint32_t keys[4] = {0, 0, 0, 0};
    const Object key_vector = reinterpret_cast<Object>(keys);

    std::uint32_t prefix[2] = {0, 0};
    const Object prefix_string = reinterpret_cast<Object>(prefix);
    host.string_resize_0041dd40(prefix_string, tail[0], 1);
    if (tail[0] != 0) {
        host.memcpy_00bf7680(obj(prefix[1]), obj(tail[1]), prefix[0]);
    }

    std::int32_t relation;
    if (entity == nullptr) {
        relation = 3;
    } else {
        const Object game = obj(host.global_u32(0x00e188a8));
        const Object local = obj(at<std::uint32_t>(game, 0x18cc + at<std::uint32_t>(game, 0x18ec) * 4));
        relation = host.recon_relation_00803ce0(obj(at<std::uint32_t>(local, 0x28)), entity);
    }
    std::uint32_t side[2];
    host.string_construct_0041e870(reinterpret_cast<Object>(side),
                                   cstr(host.global_u32(0x00e0b080 + static_cast<std::uint32_t>(relation) * 4)));
    if (side[0] != 0) {
        const std::uint32_t old_size = prefix[0];
        host.string_resize_0041dd40(prefix_string, side[0] + old_size, 1);
        host.memcpy_00bf7680(obj(prefix[1] + old_size), obj(side[1]), side[0]);
    }
    release_string(host, side);

    const auto recon_level = [&]() {
        const Object game = obj(host.global_u32(0x00e188a8));
        const Object local = obj(at<std::uint32_t>(game, 0x18cc + at<std::uint32_t>(game, 0x18ec) * 4));
        const Object record = entity + at<std::uint32_t>(local, 0x28) * 0x34 + 0x1e8;
        return at<std::uint8_t>(record, 0x10) != 0 ? at<std::int32_t>(record, 0x8) : at<std::int32_t>(record, 0x4);
    };
    // prefix + name, appended; both temporaries freed inline.
    const auto append_key = [&](const char* name) {
        std::uint32_t suffix[2];
        std::uint32_t joined[2];
        host.string_construct_0041e870(reinterpret_cast<Object>(suffix), name);
        const Object result = host.string_concat_004261a0(prefix_string, reinterpret_cast<Object>(joined),
                                                          reinterpret_cast<Object>(suffix));
        host.string_vector_append_00450540(key_vector, result);
        release_string(host, joined);
        release_string(host, suffix);
    };

    if (recon_level() >= 2) {
        append_key(host.vt10_name(entity));
    }
    if (recon_level() >= 2) {
        append_key(host.vt28_name(entity));
    }
    if (recon_level() >= 2) {
        const std::uint32_t index = host.vt0(entity + 0x170);
        append_key(cstr(host.global_u32(0x00e0b590 + index * 4)));
    }
    if (host.vt5c_is_kind_of(entity, 6)) {
        append_key("default_ship");  // 00D1B82C
    } else if (host.vt5c_is_kind_of(entity, 0xf)) {
        append_key("default_plane");  // 00D1B81C
    } else if (host.vt5c_is_kind_of(entity, 0x18)) {
        append_key("default_planesquadron");  // 00D1B804
    } else if (host.class_id_is_land_family_00922a90(entity)) {
        std::uint32_t suffix[2];
        std::uint32_t joined[2];
        host.string_construct_0041e870(reinterpret_cast<Object>(suffix), "default_landobject");  // 00D1B7F0
        const Object result = host.string_concat_004261a0(prefix_string, reinterpret_cast<Object>(joined),
                                                          reinterpret_cast<Object>(suffix));
        host.string_vector_append_00450540(key_vector, result);
        host.string_destroy_0041dd20(reinterpret_cast<Object>(joined));
        host.string_destroy_0041dd20(reinterpret_cast<Object>(suffix));
    }

    // The by-value arguments of 0097B5E0, built in its argument area.
    std::uint32_t tail_copy[2] = {0, 0};
    const Object tail_argument = reinterpret_cast<Object>(tail_copy);
    host.string_resize_0041dd40(tail_argument, tail[0], 1);
    if (tail[0] != 0) {
        host.memcpy_00bf7680(obj(tail_copy[1]), obj(tail[1]), tail_copy[0]);
    }
    std::uint32_t entry[5];
    host.key_entry_copy_0097bd10(reinterpret_cast<Object>(entry), manager + 0x11c);
    resolve_message_key_0097b5e0(host, manager, text, reinterpret_cast<Object>(entry), tail_argument, key_vector);

    release_string(host, prefix);
    if (keys[1] != 0) {
        host.string_range_destroy_00432050(obj(keys[1]), obj(keys[2]), addr(key_vector), addr(manager));
        host.free_00bf65ac(obj(keys[1]));
    }
    keys[1] = 0;
    keys[2] = 0;
    keys[3] = 0;
    release_string(host, tail);
}

// 00982540 __thiscall(ECX = manager), RET. Caller 00987590 (its first call).
// The "input" channel: no lock and no +104h/+105h flags, unlike the shared
// dispatchers. Builds "input" (resize 5 + memcpy of size + 1 from 00D1B894),
// takes the channel's subscription tree from 00980150(manager+F8h) and frees
// the name. With a non-empty tree, walks every subscription (node+14h) and
// every entry of its input set (subscription+10h, action id at node+0Ch);
// when the action's 30h-byte record at [004BEC00()+4] has byte +28h set and
// byte +20h clear, appends the callback name (subscription+4, 00450540), the
// action id (00697F40) and the record's float +24h (inline push_back, else
// 0045A120 after its where <= last check) to three local vectors. The
// iterator tests (container null or not the tree, dereferencing the head)
// are the image's checked-iterator reports. Then for each collected name i:
// a new 0Ch-byte variant vector (zeroed when allocated), variant(int id[i])
// 00884A00 and variant(float value[i]) 008849D0 pushed (006EDF00) and
// destroyed (00884AF0), 00887E50(lua host [[00E188A8]+1A08h], 0, &name[i],
// args, 0, -1), and the args freed (006B88E0(0), 00BF6989, 00BF65AC). The
// float travels as its bits (FLD/FSTP of a float, so only an SNaN would be
// quieted). Frees values, ids, then the names (inline string destructors,
// then _free). SEH 00CABF70 not modelled.
void pump_input_channel_00982540(Large62Host& host, Object manager) {
    std::uint32_t name[2] = {0, 0};
    host.string_resize_0041dd40(reinterpret_cast<Object>(name), 5, 1);
    if (name[1] != 0) {
        host.memcpy_00bf7680(obj(name[1]), "input", name[0] + 1);  // 00D1B894
    }
    const Object tree = host.channel_index_00980150(manager + 0xf8, reinterpret_cast<Object>(name));
    release_string(host, name);
    if (at<std::uint32_t>(tree, 0x8) == 0) {
        return;
    }

    // Each vector: +0 allocator word, +4 first, +8 last, +0Ch end.
    std::uint32_t names[4] = {0, 0, 0, 0};
    std::uint32_t ids[4] = {0, 0, 0, 0};
    std::uint32_t values[4] = {0, 0, 0, 0};
    const Object names_vector = reinterpret_cast<Object>(names);
    const Object ids_vector = reinterpret_cast<Object>(ids);
    const Object values_vector = reinterpret_cast<Object>(values);

    std::uint32_t outer[2] = {addr(tree), at<std::uint32_t>(obj(at<std::uint32_t>(tree, 0x4)), 0x0)};
    for (;;) {
        const std::uint32_t head = at<std::uint32_t>(tree, 0x4);
        if (outer[0] == 0 || obj(outer[0]) != tree) {
            host.invalid_parameter_00bf6713();
        }
        if (outer[1] == head) {
            break;
        }
        if (outer[0] == 0) {
            host.invalid_parameter_00bf6713();
        }
        if (outer[1] == at<std::uint32_t>(obj(outer[0]), 0x4)) {
            host.invalid_parameter_00bf6713();
        }
        const Object subscription = obj(at<std::uint32_t>(obj(outer[1]), 0x14));
        const Object set = subscription + 0x10;
        std::uint32_t inner[2] = {addr(set), at<std::uint32_t>(obj(at<std::uint32_t>(subscription, 0x14)), 0x0)};
        for (;;) {
            const std::uint32_t set_head = at<std::uint32_t>(set, 0x4);
            if (inner[0] == 0 || obj(inner[0]) != set) {
                host.invalid_parameter_00bf6713();
            }
            if (inner[1] == set_head) {
                break;
            }
            if (inner[0] == 0) {
                host.invalid_parameter_00bf6713();
            }
            if (inner[1] == at<std::uint32_t>(obj(inner[0]), 0x4)) {
                host.invalid_parameter_00bf6713();
            }
            std::uint32_t action = at<std::uint32_t>(obj(inner[1]), 0xc);
            const std::uint32_t offset = action * 0x30;
            const Object record = obj(at<std::uint32_t>(host.input_manager_004bec00(), 0x4) + offset);
            if (at<std::uint8_t>(record, 0x28) != 0 && at<std::uint8_t>(record, 0x20) == 0) {
                host.string_vector_append_00450540(names_vector, subscription + 0x4);
                host.dword_vector_append_00697f40(ids_vector, &action);
                float value = at<float>(obj(at<std::uint32_t>(host.input_manager_004bec00(), 0x4) + offset), 0x24);
                if (values[1] != 0 && ((values[2] - values[1]) >> 2) < ((values[3] - values[1]) >> 2)) {
                    at<float>(obj(values[2]), 0x0) = value;
                    values[2] += 4;
                } else {
                    if (values[1] > values[2]) {
                        host.invalid_parameter_00bf6713();
                    }
                    std::uint32_t inserted[2];
                    host.float_vector_insert_0045a120(values_vector, reinterpret_cast<Object>(inserted), values_vector,
                                                      values[2], &value);
                }
            }
            host.tree_iterator_next_00969b50(reinterpret_cast<Object>(inner));
        }
        host.tree_iterator_next_00966960(reinterpret_cast<Object>(outer));
    }

    if (names[1] != 0 && ((names[2] - names[1]) >> 3) != 0) {
        for (std::uint32_t i = 0; names[1] != 0 && i < ((names[2] - names[1]) >> 3); ++i) {
            const Object args = host.operator_new_00bf681b(0xc);
            if (args != nullptr) {
                at<std::uint32_t>(args, 0x0) = 0;
                at<std::uint32_t>(args, 0x4) = 0;
                at<std::uint32_t>(args, 0x8) = 0;
            }
            std::uint8_t variant[0x18];  // the frame's 18h bytes at +6Ch
            if (ids[1] == 0 || i >= ((ids[2] - ids[1]) >> 2)) {
                host.invalid_parameter_00bf6713();
            }
            host.lua_variant_int_00884a00(variant, at<std::uint32_t>(obj(ids[1]), i * 4));
            host.lua_variant_vector_push_006edf00(args, variant);
            host.lua_variant_destruct_00884af0(variant);
            if (values[1] == 0 || i >= ((values[2] - values[1]) >> 2)) {
                host.invalid_parameter_00bf6713();
            }
            host.lua_variant_008849d0(variant, at<std::uint32_t>(obj(values[1]), i * 4));
            host.lua_variant_vector_push_006edf00(args, variant);
            host.lua_variant_destruct_00884af0(variant);
            if (names[1] == 0 || i >= ((names[2] - names[1]) >> 3)) {
                host.invalid_parameter_00bf6713();
            }
            const Object lua_host = obj(at<std::uint32_t>(obj(host.global_u32(0x00e188a8)), 0x1a08));
            host.lua_call_named_00887e50(lua_host, 0, obj(names[1] + i * 8), args, 0, -1);
            if (args != nullptr) {
                host.lua_variant_vector_resize_006b88e0(args, 0);
                host.free_00bf6989(obj(at<std::uint32_t>(args, 0x0)));
                host.free_00bf65ac(args);
            }
        }
    }

    if (values[1] != 0) {
        host.free_00bf65ac(obj(values[1]));
    }
    values[1] = 0;
    values[2] = 0;
    values[3] = 0;
    if (ids[1] != 0) {
        host.free_00bf65ac(obj(ids[1]));
    }
    ids[1] = 0;
    ids[2] = 0;
    ids[3] = 0;
    if (names[1] != 0) {
        for (std::uint32_t element = names[1]; element != names[2]; element += 8) {
            release_string(host, reinterpret_cast<const std::uint32_t*>(obj(element)));
        }
        host.free_00bf65ac(obj(names[1]));
    }
}

namespace {

using cc13_seg62::TaggedValue;

TaggedValue tagged(std::uint32_t tag, std::uint32_t value) noexcept {
    return TaggedValue{tag, value};
}
TaggedValue named(const char* key) noexcept {
    return TaggedValue{0, addr(key)};
}

// The per-key name string: cleared, resized to strlen with fill 1, and the
// bytes plus the terminator copied when the data pointer is set.
void string_from_key(Large62Host& host, std::uint32_t* string, const char* text) {
    string[0] = 0;
    string[1] = 0;
    std::uint32_t length = 0;
    while (text[length] != '\0') {
        ++length;
    }
    host.string_resize_0041dd40(reinterpret_cast<Object>(string), length, 1);
    if (string[1] != 0) {
        host.memcpy_00bf7680(obj(string[1]), text, string[0] + 1);
    }
}

// A list<warning*> push_back as the image inlines it: 0096B510 buys the node
// before the head, 0096F960 grows the size, then the node is linked in.
void load_warning_list(Large62Host& host, Object manager, std::size_t list_offset, Object reader) {
    for (std::uint32_t i = 0; host.reader_vt14_has(reader, tagged(1, i)); ++i) {
        host.reader_vt4_enter(reader, tagged(1, i));
        std::uint32_t value = host.warning_create_from_archive_009766d0(reader);
        const Object list = manager + list_offset;
        const Object head = obj(at<std::uint32_t>(list, 0x4));
        const Object node = host.list_buy_node_0096b510(list, head, obj(at<std::uint32_t>(head, 0x4)), &value);
        host.list_increase_size_0096f960(list, 1);
        at<std::uint32_t>(head, 0x4) = addr(node);
        at<std::uint32_t>(obj(at<std::uint32_t>(node, 0x4)), 0x0) = addr(node);
        host.reader_vt8_leave(reader);
    }
}

}  // namespace

// 00980380 __thiscall(ECX = manager; reader), RET 4. No xrefs in the saved
// analysis (reached through a vtable). The load side of 0096DD30:
//   vt10("disableAll", {3, +D0h})
//   "disableEntity": for i while vt14({1,i}): vt10({1,i}, {4, &entity}), then
//       *0077EDF0(+D4h, &entity) = 1
//   "waitingQueue" into the list +E0h and "disableList" into the list +ECh:
//       for i while vt14({1,i}): vt4({1,i}), 009766D0(reader) pushed back, vt8
//   "luaListeners": keys (vt18, tags preset to -1, count cleared); per channel
//       key: name string, vt4(key), its keys likewise; per subscription key:
//       name string, vt4(key2), *00978D60(00980150(+F8h, channel), sub name)
//       = 0097E360(channel, reader), vt8
//   "useCount": keys (vt18, nothing preset); vt10(key, {1, 005070C0(+144h, name)})
//   "lastUse": keys; vt10(key, {2, 00444BE0(+150h, name)})
//   "shipNoiseInfo": keys; each key value is an entity handle decoded through
//       the two-bank table (low word < [00F89A10]: ([00F89A54] + (low -
//       [00F89A0C]) * 10h) + 0Ch, else [00F89AA8], [00F89A60]); record =
//       00975D00(+184h, &entity); vt4({1, handle}); vt10("collisionSound
//       DisableTime", {2, record}); vtC("airRaidSoundExpires", {2, record+4},
//       {2, -1e10f}); when record+4 != -1e10f (or unordered: UCOMISS, LAHF,
//       TEST AH,44h, JNP) 0096C070(record, entity); vt8
// with a vt8 closing each section. The tag -1 preset and count clear happen
// only before the two luaListeners lists; the later sections reuse the first
// block as the image does. SEH 00CABD2C not modelled.
void warning_manager_load_archive_00980380(Large62Host& host, Object manager, Object reader) {
    host.reader_vt10_read(reader, named("disableAll"), tagged(3, addr(manager + 0xd0)));  // 00D1B07C

    host.reader_vt4_enter(reader, named("disableEntity"));  // 00D1B06C
    for (std::uint32_t i = 0; host.reader_vt14_has(reader, tagged(1, i)); ++i) {
        std::uint32_t entity;
        host.reader_vt10_read(reader, tagged(1, i), tagged(4, addr(&entity)));
        const Object key = obj(entity);
        *host.entity_suppress_slot_0077edf0(manager + 0xd4, &key) = 1;
    }
    host.reader_vt8_leave(reader);

    host.reader_vt4_enter(reader, named("waitingQueue"));  // 00D1B05C
    load_warning_list(host, manager, 0xe0, reader);
    host.reader_vt8_leave(reader);

    host.reader_vt4_enter(reader, named("disableList"));  // 00D1B050
    load_warning_list(host, manager, 0xec, reader);
    host.reader_vt8_leave(reader);

    ReaderKeyList keys;
    ReaderKeyList sub_keys;
    host.reader_vt4_enter(reader, named("luaListeners"));  // 00D1B040
    for (auto& key : keys.keys) {
        key.tag = 0xffffffffu;
    }
    keys.count = 0;
    host.reader_vt18_list_keys(reader, reinterpret_cast<Object>(&keys));
    for (std::int32_t j = 0; j < keys.count; ++j) {
        std::uint32_t channel[2];
        string_from_key(host, channel, cstr(keys.keys[j].value));
        host.reader_vt4_enter(reader, keys.keys[j]);
        for (auto& key : sub_keys.keys) {
            key.tag = 0xffffffffu;
        }
        sub_keys.count = 0;
        host.reader_vt18_list_keys(reader, reinterpret_cast<Object>(&sub_keys));
        for (std::int32_t k = 0; k < sub_keys.count; ++k) {
            std::uint32_t callback[2];
            string_from_key(host, callback, cstr(sub_keys.keys[k].value));
            host.reader_vt4_enter(reader, sub_keys.keys[k]);
            const Object channel_map = host.channel_index_00980150(manager + 0xf8, reinterpret_cast<Object>(channel));
            std::uint32_t* const slot = host.subscription_slot_00978d60(channel_map, reinterpret_cast<Object>(callback));
            *slot = host.parse_event_block_0097e360(reinterpret_cast<Object>(channel), reader);
            host.reader_vt8_leave(reader);
            release_string(host, callback);
        }
        host.reader_vt8_leave(reader);
        release_string(host, channel);
    }
    host.reader_vt8_leave(reader);

    host.reader_vt4_enter(reader, named("useCount"));  // 00D1B034
    host.reader_vt18_list_keys(reader, reinterpret_cast<Object>(&keys));
    for (std::int32_t j = 0; j < keys.count; ++j) {
        std::uint32_t key_name[2];
        string_from_key(host, key_name, cstr(keys.keys[j].value));
        const Object count = host.int_map_slot_005070c0(manager + 0x144, reinterpret_cast<Object>(key_name));
        host.reader_vt10_read(reader, keys.keys[j], tagged(1, addr(count)));
        release_string(host, key_name);
    }
    host.reader_vt8_leave(reader);

    host.reader_vt4_enter(reader, named("lastUse"));  // 00D1B02C
    host.reader_vt18_list_keys(reader, reinterpret_cast<Object>(&keys));
    for (std::int32_t j = 0; j < keys.count; ++j) {
        std::uint32_t key_name[2];
        string_from_key(host, key_name, cstr(keys.keys[j].value));
        const Object time = host.float_map_slot_00444be0(manager + 0x150, reinterpret_cast<Object>(key_name));
        host.reader_vt10_read(reader, keys.keys[j], tagged(2, addr(time)));
        release_string(host, key_name);
    }
    host.reader_vt8_leave(reader);

    host.reader_vt4_enter(reader, named("shipNoiseInfo"));  // 00D1B01C
    host.reader_vt18_list_keys(reader, reinterpret_cast<Object>(&keys));
    for (std::int32_t j = 0; j < keys.count; ++j) {
        const std::uint32_t handle = keys.keys[j].value;
        const std::int32_t low = static_cast<std::int32_t>(handle & 0xffffu);
        std::uint32_t entity;
        if (low < static_cast<std::int32_t>(host.global_u32(0x00f89a10))) {
            entity = at<std::uint32_t>(obj(host.global_u32(0x00f89a54)),
                                       (static_cast<std::uint32_t>(low) - host.global_u32(0x00f89a0c)) * 0x10 + 0xc);
        } else {
            entity = at<std::uint32_t>(obj(host.global_u32(0x00f89aa8)),
                                       (static_cast<std::uint32_t>(low) - host.global_u32(0x00f89a60)) * 0x10 + 0xc);
        }
        std::uint32_t key = entity;
        const Object record = host.ship_noise_slot_00975d00(manager + 0x184, &key);
        host.reader_vt4_enter(reader, tagged(1, handle));
        host.reader_vt10_read(reader, named("collisionSoundDisableTime"), tagged(2, addr(record)));  // 00D1B000
        const float fallback = host.global_f32(0x00ce4adc);  // -1e10f (bits D01502F9)
        std::uint32_t fallback_bits;
        static_assert(sizeof(fallback_bits) == sizeof(fallback), "float word");
        std::memcpy(&fallback_bits, &fallback, sizeof(fallback));
        host.reader_vtc_read_default(reader, named("airRaidSoundExpires"), tagged(2, addr(record + 0x4)),  // 00D1AFEC
                                     tagged(2, fallback_bits));
        if (!(at<float>(record, 0x4) == host.global_f32(0x00ce4adc))) {
            host.ship_noise_start_air_raid_0096c070(manager, record, entity);
        }
        host.reader_vt8_leave(reader);
    }
    host.reader_vt8_leave(reader);
}

namespace {

// The point group's float3 vector at +48h/+4Ch, counted with the image's
// signed division by 0Ch.
std::uint32_t point_count(Object group) {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(at<std::uint32_t>(group, 0x4c) - at<std::uint32_t>(group, 0x48)) / 0xc);
}

// The checked element access before each point read: front() when index is 0
// (begin null or empty), else begin null or count <= index (unsigned).
void check_point(Large62Host& host, Object group, std::uint32_t index) {
    if (at<std::uint32_t>(group, 0x48) == 0 || (index == 0 ? point_count(group) == 0 : point_count(group) <= index)) {
        host.invalid_parameter_00bf6713();
    }
}

// p[index] - p[0], each difference stored as a float (FSUB, FSTP float).
void point_delta(Large62Host& host, Object group, std::uint32_t index, float* out) {
    check_point(host, group, 0);
    check_point(host, group, index);
    const Object points = obj(at<std::uint32_t>(group, 0x48));
    for (std::size_t axis = 0; axis < 3; ++axis) {
        out[axis] = at<float>(points, index * 0xc + axis * 4) - at<float>(points, axis * 4);
    }
}

}  // namespace

// 0095DE00 __fastcall(ECX = unit), RET. Callers 006F4D10 and 0095E5B0
// (Seg62Host::attach_flag_node_0095de00). Flag params (0070BBA0, then width
// 2.5f from 00CF87C8 and height 1.5f from 00CE380C, then the nation texture
// 009574D0) are built once. When the game settings byte +688h is set, for
// index = 1, 2, ...: the class resource's ([unit+538h]+50h) point group
// "zaszlo" (00D1A984, a std::string through 00408720, freed when its capacity
// reached 10h) number index is looked up (00718870); the loop ends without a
// group or with fewer than three points. Otherwise:
//   - the params are copied (009589D0); while the class vector +114h (14h-byte
//     {width, height, texture}) has entries left, the next one overrides width
//     and height and, when its texture is non-empty, the texture (00425F40)
//   - basis rows: row 2 = p2 - p0, row 1 = 004F9B30(p2 - p0, p1 - p0),
//     orthonormalized in place (0085DC80); w of rows 0..2 = 0, row 3 w = 1.0f
//   - the texture table by row 2's y: > 0.5f [00E19B08], < -0.5f [00E19B04],
//     else [00E19B0C]; s = height / table+48h, unless width / table+44h is
//     strictly smaller (FCOMIP, JBE keeps the height ratio, also unordered)
//   - basis = basis * diag(s, s, s, 1) (004134F0 copy of the scale, 00413920,
//     004134F0 back), then row 3 xyz = p0
//   - the model: 0070B210 pool block; when non-null, 0070BFC0(block, "MFlag"
//     (00D1A97C), params) with the name freed after; else 0
//   - the parent: for a vtable[5Ch](6) unit, the model is pushed (0095D210)
//     into the first part slot (parts 0080E490 +8Ch + k*20h, k < 14h) whose
//     class index list (+138h + k*30h) holds index; then when parts +370h and
//     +374h (0092BEA0, 0092BE90) are both set, p0.z > class +580h picks +370h,
//     else +374h; a zero result, or any other unit, uses unit +4A4h
//   - 00B6E680(model, parent), model vtable[38h](basis), 0095D160(unit+660h,
//     &model), and the params copy's texture freed
// then the params' texture is freed. No null test on the model before
// 00B6E680, as in the image. The image's basis rows 0 and 3 start as stale
// stack on the first pass; here they start at zero (row 0 goes to 0085DC80
// unread by this function and row 3 xyz is overwritten after the multiply,
// so only non-finite stale bits could differ). SEH 00CA94EE not modelled.
void attach_flag_node_0095de00(Large62Host& host, Object unit) {
    std::uint32_t params[4];  // {width, height, texture string {size, data}}
    const Object defaults = reinterpret_cast<Object>(params);
    host.flag_params_init_0070bba0(defaults);
    at<float>(defaults, 0x0) = host.global_f32(0x00cf87c8);
    at<float>(defaults, 0x4) = host.global_f32(0x00ce380c);
    cc13_seg62::flag_select_nation_texture_009574d0(host, unit, defaults);

    if (at<std::uint8_t>(host.game_settings_00424c40(), 0x688) != 0) {
        std::uint32_t index = 0;
        std::uint32_t override_index = 0;
        std::uint32_t override_offset = 0;
        float basis[16] = {};
        for (;;) {
            ++index;
            // std::string: +4 inline buffer (10h bytes), +14h size, +18h capacity.
            std::uint32_t name[7];
            const Object name_string = reinterpret_cast<Object>(name);
            name[6] = 0xf;
            name[5] = 0;
            at<std::uint8_t>(name_string, 0x4) = 0;
            host.std_string_assign_00408720(name_string, "zaszlo", 6);
            const Object resource = obj(at<std::uint32_t>(obj(at<std::uint32_t>(unit, 0x538)), 0x50));
            const Object group = host.point_group_lookup_00718870(resource, name_string, index);
            if (name[6] >= 0x10) {
                host.free_00bf65ac(obj(name[1]));
            }
            name[6] = 0xf;
            name[5] = 0;
            at<std::uint8_t>(name_string, 0x4) = 0;
            if (group == nullptr || at<std::uint32_t>(group, 0x48) == 0 ||
                static_cast<std::int32_t>(point_count(group)) <= 2) {
                break;
            }

            std::uint32_t copy[4];
            const Object params_copy = reinterpret_cast<Object>(copy);
            host.flag_params_copy_009589d0(params_copy, defaults);
            const Object vehicle_class = obj(at<std::uint32_t>(unit, 0x538));
            const auto override_count = [&]() {
                return static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(vehicle_class, 0x118) -
                                                                           at<std::uint32_t>(vehicle_class, 0x114)) /
                                                  0x14);
            };
            if (at<std::uint32_t>(vehicle_class, 0x114) != 0 && override_index < override_count()) {
                if (at<std::uint32_t>(vehicle_class, 0x114) == 0 || override_index >= override_count()) {
                    host.invalid_parameter_00bf6713();
                }
                const Object element = obj(at<std::uint32_t>(vehicle_class, 0x114) + override_offset);
                at<float>(params_copy, 0x0) = at<float>(element, 0x0);
                at<float>(params_copy, 0x4) = at<float>(element, 0x4);
                if (at<std::uint32_t>(element, 0x8) != 0) {
                    host.string_assign_header_00425f40(params_copy + 0x8, element + 0x8);
                }
                ++override_index;
                override_offset += 0x14;
            }
            std::uint32_t model = 0;

            point_delta(host, group, 2, &basis[8]);
            float side[3];
            float forward[3];
            point_delta(host, group, 1, side);
            point_delta(host, group, 2, forward);
            float cross[3];
            const float* const up = host.vector3_cross_004f9b30(cross, forward, side);
            basis[4] = up[0];
            basis[5] = up[1];
            basis[6] = up[2];
            host.orthonormalize_0085dc80(basis);
            basis[11] = 0.0f;
            basis[7] = 0.0f;
            basis[3] = 0.0f;
            const float one = host.global_f32(0x00d7a24c);  // 1.0f
            basis[15] = one;

            const float row2_y = basis[9];
            std::uint32_t table;
            if (row2_y > host.global_f32(0x00ce3800)) {  // 0.5f
                table = host.global_u32(0x00e19b08);
            } else if (host.global_f32(0x00ce69d0) > row2_y) {  // -0.5f
                table = host.global_u32(0x00e19b04);
            } else {
                table = host.global_u32(0x00e19b0c);
            }
            const float by_height = at<float>(params_copy, 0x4) / at<float>(obj(table), 0x48);
            const float by_width = at<float>(params_copy, 0x0) / at<float>(obj(table), 0x44);
            const float scale_factor = by_height > by_width ? by_width : by_height;
            float scale[16] = {};
            scale[0] = scale_factor;
            scale[5] = scale_factor;
            scale[10] = scale_factor;
            scale[15] = one;
            float scale_copy[16];
            host.matrix_copy_004134f0(reinterpret_cast<Object>(scale_copy), scale);
            const Object product = host.matrix_multiply_00413920(reinterpret_cast<Object>(basis),
                                                                 reinterpret_cast<Object>(scale),
                                                                 reinterpret_cast<Object>(scale_copy));
            host.matrix_copy_004134f0(reinterpret_cast<Object>(basis), reinterpret_cast<const float*>(product));
            check_point(host, group, 0);
            const Object points = obj(at<std::uint32_t>(group, 0x48));
            basis[12] = at<float>(points, 0x0);
            basis[13] = at<float>(points, 0x4);
            basis[14] = at<float>(points, 0x8);

            const Object block = host.flag_pool_allocate_0070b210();
            if (block != nullptr) {
                std::uint32_t model_name[2];
                host.string_construct_0041e870(reinterpret_cast<Object>(model_name), "MFlag");
                model = addr(host.flag_model_construct_0070bfc0(block, reinterpret_cast<Object>(model_name),
                                                                params_copy));
                release_string(host, model_name);
            } else {
                model = 0;
            }

            std::uint32_t parent = 0;
            if (host.vt5c_is_kind_of(unit, 6)) {
                const Object parts = host.unit_parts_0080e490(unit);
                Object slot = parts + 0x8c;
                Object list = obj(at<std::uint32_t>(unit, 0x538)) + 0x138;
                bool placed = false;
                for (std::int32_t k = 0; k < 0x14; ++k, slot += 0x20, list += 0x30) {
                    if (placed) {
                        break;
                    }
                    for (std::uint32_t j = 0;; ++j) {
                        const std::uint32_t first = at<std::uint32_t>(list, 0x4);
                        if (first == 0 ||
                            j >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(list, 0x8) - first) >> 2)) {
                            break;
                        }
                        if (at<std::uint32_t>(obj(first), j * 4) == index) {
                            host.vector_push_0095d210(slot, &model);
                            placed = true;
                            break;
                        }
                    }
                }
                if (host.parts_node_0092bea0(parts) != 0 && host.parts_node_0092be90(parts) != 0) {
                    const Object current_class = obj(at<std::uint32_t>(unit, 0x538));
                    check_point(host, group, 0);
                    const float z = at<float>(obj(at<std::uint32_t>(group, 0x48)), 0x8);
                    parent = z > at<float>(current_class, 0x580) ? host.parts_node_0092bea0(parts)
                                                                 : host.parts_node_0092be90(parts);
                }
            }
            if (parent == 0) {
                parent = at<std::uint32_t>(unit, 0x4a4);
            }
            host.node_set_parent_00b6e680(obj(model), parent);
            host.vt38_set_matrix(obj(model), basis);
            host.vector_push_0095d160(unit + 0x660, &model);
            release_string(host, &copy[2]);
        }
    }
    release_string(host, &params[2]);
}

}  // namespace bsp::cc13_large62
