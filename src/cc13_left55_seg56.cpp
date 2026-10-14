// Packet cc13-left55, segment 56 helpers and the first ten bindings. See
// include/bsp/cc13_left55.hpp for the ABI and docs/CC13_LEFT55.md for the evidence.
#include "bsp/cc13_left55.hpp"

#include <cstring>

namespace bsp::cc13_left55 {
namespace {

void copy_into(Left55Host& host, NativeStr* dst, const NativeStr* src)
{
    if (dst != src) {
        host.native_string_resize_0041dd40(dst, src->size, true);
        if (src->size != 0) {
            std::memcpy(dst->data, src->data, dst->size);
        }
    }
}

void release(Left55Host& host, const NativeStr& s)
{
    if (s.data != nullptr) {
        host.pool_release_00419cc0_00bd1510(s.data, s.size + 1);
    }
}

char* at(void* base, std::uint32_t offset)
{
    return static_cast<char*>(base) + offset;
}

// 008D3E00's nested walk over a map<K, map<K2, T*>> at `outer` (proxy +0, head +4): every inner
// node's pointer at +10h is freed and cleared. Iterators are {container, node}.
void free_inner_values(Left56Host& host, char* outer)
{
    auto head_of = [](char* map) { return *reinterpret_cast<char**>(map + 4); };
    void* it[2] = {outer, *reinterpret_cast<char**>(head_of(outer))}; // begin = head->left
    for (;;) {
        char* end = head_of(outer);
        if (it[0] == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it[1] == end) {
            break;
        }
        if (it[0] == nullptr) {
            host.invalid_parameter_00bf6713();
        }
        if (it[1] == head_of(outer)) {
            host.invalid_parameter_00bf6713();
        }
        char* node = static_cast<char*>(it[1]);
        char* inner = node + 0x10;                                   // the node's map value
        void* in[2] = {inner, *reinterpret_cast<char**>(head_of(inner))};
        for (;;) {
            if (it[1] == head_of(outer)) {
                host.invalid_parameter_00bf6713();
            }
            char* inner_end = head_of(inner);
            if (inner == nullptr) {                                  // CMP node,-10h
                host.invalid_parameter_00bf6713();
            }
            if (in[1] == inner_end) {
                break;
            }
            if (inner == nullptr) {
                host.invalid_parameter_00bf6713();
            }
            if (in[1] == head_of(inner)) {
                host.invalid_parameter_00bf6713();
            }
            void** value = reinterpret_cast<void**>(static_cast<char*>(in[1]) + 0x10);
            if (*value != nullptr) {
                host.free_00bf65ac(*value);
                *value = nullptr;
            }
            host.map_iterator_increment_005672d0(in);
        }
        host.map_iterator_increment_00569b00(it);
    }
}

using EraseRange = void (Left56Host::*)(void*, void*, void*, void*, void*, void*);

// The inlined map destructor: erase(begin, end), free the head, clear head and size.
void destroy_map(Left56Host& host, char* map, EraseRange erase)
{
    std::uint8_t out[8];
    char* head = *reinterpret_cast<char**>(map + 4);
    (host.*erase)(map, out, map, *reinterpret_cast<char**>(head), map, head);
    host.free_00bf65ac(*reinterpret_cast<void**>(map + 4));
    *reinterpret_cast<void**>(map + 4) = nullptr;
    *reinterpret_cast<std::uint32_t*>(map + 8) = 0;
}

} // namespace

NamedPairRecord* named_pair_assign_0088b410(Left56Host& host, NamedPairRecord* record,
    const NamedPairRecord* source)
{
    record->first = source->first;
    record->second = source->second;
    copy_into(host, &record->name, &source->name);
    record->flag = source->flag;
    return record;
}

void entity_start_anim_0088b4b0(Left56Host& host, void* entity, const NativeStr* name,
    float offset)
{
    *reinterpret_cast<std::uint8_t*>(at(entity, 0x70C)) = 1;
    copy_into(host, reinterpret_cast<NativeStr*>(at(entity, 0x714)), name);
    *reinterpret_cast<float*>(at(entity, 0x710)) = host.now_00f876a4() - offset; // 0088B4EE
}

void* tree_construct_0088b520(Left56Host& host, void* tree)
{
    tree_construct_0088aaf0(host, tree); // same body as 0088AAF0, returning the tree
    return tree;
}

NamedValue* named_value_copy_0088b550(Left56Host& host, NamedValue* value, const NamedValue* source)
{
    value->name = {};
    copy_into(host, &value->name, &source->name);
    value->value = source->value;
    return value;
}

NamedPairRecord* named_pair_copy_0088b5a0(Left56Host& host, NamedPairRecord* record,
    const NamedPairRecord* source)
{
    record->first = source->first;
    record->second = source->second;
    record->name = {};
    copy_into(host, &record->name, &source->name);
    record->flag = source->flag;
    return record;
}

void scored_sort_heap_0088b650(ScoredEntry* first, ScoredEntry* last)
{
    for (; last - first > 1; --last) {
        const ScoredEntry value = *(last - 1);
        *(last - 1) = *first;
        scored_adjust_heap_0088a130(first, 0, static_cast<std::int32_t>(last - 1 - first), value);
    }
}

NamedSessionMessage* named_message_e2_construct_0088b6d0(Left56Host& host,
    NamedSessionMessage* message, const NativeStr* name, std::uint32_t value_20,
    std::uint32_t value_24)
{
    host.session_message_base_0075b430(message, 0xE2);
    *reinterpret_cast<std::int32_t*>(message->base + 4) = 1;
    *reinterpret_cast<std::uint32_t*>(message->base) = kNamedMessageE2Vtable;
    message->name = {};
    copy_into(host, &message->name, name);
    message->value_20 = value_20;
    message->value_24 = value_24;
    return message;
}

NamedSessionMessage* named_message_e7_construct_0088b7b0(Left56Host& host,
    NamedSessionMessage* message, const NativeStr* name)
{
    host.session_message_base_0075b430(message, 0xE7);
    *reinterpret_cast<std::int32_t*>(message->base + 4) = 1;
    *reinterpret_cast<std::uint32_t*>(message->base) = kNamedMessageE7Vtable;
    message->name = {};
    copy_into(host, &message->name, name);
    return message;
}

bool lua_object_is_vector3_table_0088b840(Left56Host& host, void* object)
{
    if (!host.lua_object_is_table_00b661b0(object)) {
        return false;
    }
    std::int32_t matched = 0;
    alignas(4) std::uint8_t key[0x14];
    alignas(4) std::uint8_t value[0x14];
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(value);
    host.lua_object_iterate_first_00b67080(object, key, value);
    bool result = false;
    if (!host.lua_object_is_unbound_00b66420(object, key)) {
        for (;;) {
            if (!host.lua_object_is_string_00b660a0(key)) {
                break;
            }
            if (!host.lua_object_is_number_00b66050(value)) {
                break;
            }
            NativeStr name{};
            host.native_string_assign_0041e870(&name, host.lua_object_get_string_00b662b0(key));
            const bool is_x = name.data != nullptr && host.stricmp_00bf7fbf(name.data, "x") == 0;
            if (!is_x && !host.native_string_equals_ci_00425850(&name, "y") &&
                !host.native_string_equals_ci_00425850(&name, "z")) {
                release(host, name);
                break;
            }
            ++matched;
            release(host, name);
            host.lua_object_iterate_next_00b67190(object, key, value);
            if (host.lua_object_is_unbound_00b66420(object, key)) {
                result = matched == 3;
                break;
            }
        }
    }
    host.lua_object_destruct_00b67700(value);
    host.lua_object_destruct_00b67700(key);
    return result;
}

bool lua_object_is_entity_or_vector3_0088ba00(Left56Host& host, void* object)
{
    return host.lua_value_is_entity_handle_008889c0(object) ||
        lua_object_is_vector3_table_0088b840(host, object);
}

void variant_array_walk_0088bbc0(Left56Host& host, VariantArray* array)
{
    LuaVariant* end = array->data + array->count;
    for (LuaVariant* v = array->data; v != end; ++v) {
        if (v->tag != 0 && v->tag != 1 && v->tag == 5) {
            variant_map_walk_0088b120(host, v->payload.map); // ECX = [ESI+8]
        }
    }
}

void multi_lobby_settings_destroy_008d3e00(Left56Host& host, void* settings)
{
    char* self = static_cast<char*>(settings);
    *reinterpret_cast<std::uint32_t*>(self) = kMultiLobbySettingsVtable;
    free_inner_values(host, self + 0x10);
    free_inner_values(host, self + 0x34);
    destroy_map(host, self + 0x34, &Left56Host::map_erase_range_00576120); // 008D4033
    destroy_map(host, self + 0x28, &Left56Host::map_erase_range_00576740); // 008D4062
    destroy_map(host, self + 0x1C, &Left56Host::map_erase_range_005761f0); // 008D4091
    destroy_map(host, self + 0x10, &Left56Host::map_erase_range_00576120); // 008D40BD
    destroy_map(host, self + 0x04, &Left56Host::map_erase_range_005724d0); // 008D40EB
    host.write_global_00f8894c(0);
    *reinterpret_cast<std::uint32_t*>(self) = kSingletonBaseVtable;
}

// ---------------------------------------------------------------------------
// Bindings
// ---------------------------------------------------------------------------

int lua_log_to_file_0088bc00(Left56Host& host, void* lua_state)
{
    // Every argument is collected as variants and dropped: nothing is written anywhere.
    alignas(4) std::uint8_t holder[0x08];
    ResultSink sink{};
    host.lua_state_holder_construct_006b7e30(holder, lua_state);     // 0088BCBB
    host.result_sink_construct_008875f0(&sink, holder);              // 0088BCCD
    result_sink_collect_00887120(host, &sink, -1, 10);               // 0088BCDE
    variant_array_walk_0088bbc0(host, reinterpret_cast<VariantArray*>(sink.variants)); // 0088BCE7
    host.result_sink_teardown_00887670(&sink);                       // 0088BCF4
    host.lua_state_holder_destruct_006b7e50(holder);                 // 0088BD01
    return 0;                                                        // literal, not 00B66400
}

int lua_do_file_outside_mpak_0088bd20(cc13_seg56::LuaArgs& args, Left56Host& host)
{
    host.set_script_nesting_00e1ae84(host.script_nesting_00e1ae84() + 1);
    NativeStr name{};
    const std::string path = args.get_string(0);
    host.native_string_assign_0041e870(&name, path.c_str());
    host.run_script_with_overrides_00b69d40(&name, 0);               // 0088BE42
    release(host, name);
    host.set_script_nesting_00e1ae84(host.script_nesting_00e1ae84() - 1);
    return 0;
}

int lua_log_0088bec0()
{
    return 0; // only the dead prologue: no frame is opened
}

int lua_dump_0088bf20()
{
    return 0; // only the dead prologue
}

int lua_assert_0088c3e0(cc13_seg56::LuaArgs& args)
{
    // Type tests whose answers are never used: Assert asserts nothing.
    if (args.count() == 1 && args.is_boolean(0)) {                   // 0088C4D2 / 0088C4FE
        return 0;
    }
    if (args.count() == 2 && args.is_boolean(0)) {                   // 0088C53C / 0088C564
        args.is_string(1);                                           // 0088C591
    }
    return 0;
}

int lua_add_watch_0088d130(cc13_seg56::LuaArgs& args, Left56Host& host)
{
    NativeStr name{};
    const std::string text = args.get_string(0);
    host.native_string_assign_0041e870(&name, text.c_str());        // built in the argument slot
    void* watches = *reinterpret_cast<void**>(at(host.game_global_00e188a8(), 0x21CC));
    host.watch_add_00922240(watches, name);                          // 0088D24F, callee owns it
    return 0;
}

int lua_log_to_console_0088d420(cc13_seg56::LuaArgs& args)
{
    args.count();                                                    // read and discarded
    return 0;
}

int lua_remove_watch_0088d560(cc13_seg56::LuaArgs& args, Left56Host& host)
{
    NativeStr name{};
    const std::string text = args.get_string(0);
    host.native_string_assign_0041e870(&name, text.c_str());
    void* watches = *reinterpret_cast<void**>(at(host.game_global_00e188a8(), 0x21CC));
    host.watch_remove_009222e0(watches, name);                       // 0088D67F
    return 0;
}

int lua_network_log_0088d6e0(cc13_seg56::LuaArgs& args, cc13_seg56::LuaResults& results,
    Left56Host& host)
{
    if (args.count() == 1) {
        const std::string text = args.get_string(0);
        NativeStr name{};
        host.native_string_assign_0041e870(&name, text.c_str());
        auto* target = reinterpret_cast<NativeStr*>(at(host.packet_statistics_00f871b4(), 0x7649AC));
        copy_into(host, target, &name);
        release(host, name);
    }
    const bool started =
        host.packet_statistics_start_csv_00789560(host.packet_statistics_00f871b4()); // 0088D87B
    results.push_boolean(started);                                   // 0088D88D
    return 1;
}

int lua_get_gui_name_0088daa0(cc13_seg56::LuaArgs& args, Left56Host& host)
{
    void* entity = args.entity_at(0);
    host.push_c_string_00b66710(host.entity_gui_name_vcall_14(entity)); // 0088DBBD / 0088DBC4
    return 1;
}

} // namespace bsp::cc13_left55
