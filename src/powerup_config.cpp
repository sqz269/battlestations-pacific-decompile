#include "bsp/powerup_config.hpp"

#include "bsp/lua_numeric.hpp"
#include "bsp/native_pooled_string_substring.hpp"
#include "bsp/singleton_lifetime.hpp"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <cstring>
#include <iterator>
#include <new>
#include <stdexcept>
#include <tuple>
#include <utility>

#if !defined(_MSC_VER) || !defined(_M_IX86)
#error Powerup configuration requires MSVC Win32.
#endif

namespace bsp {
namespace {
static_assert(sizeof(PowerupOwnerNativeStorage) == 0x1c4);
static_assert(offsetof(PowerupOwnerNativeStorage, maps_14c) == 0x14c);
static_assert(offsetof(PowerupOwnerNativeStorage, random_names_1b8) == 0x1b8);
static_assert(sizeof(PowerupFactionConfig) == 0x38);
static_assert(offsetof(PowerupFactionConfig, texture_24) == 0x24);
static_assert(sizeof(PowerupClassConfig) == 0xe4);
static_assert(offsetof(PowerupClassConfig, duration_7c) == 0x7c);
static_assert(offsetof(PowerupClassConfig, random_cc) == 0xcc);
static_assert(offsetof(PowerupClassConfig, texture_d0) == 0xd0);

void*& pointer(void* storage, std::size_t offset) {
    return *reinterpret_cast<void**>(static_cast<unsigned char*>(storage) + offset);
}
std::uint8_t& byte(void* storage, std::size_t offset) {
    return *(static_cast<unsigned char*>(storage) + offset);
}
void* tree_node(std::size_t size, std::size_t color) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
    pointer(node, 0) = nullptr;
    pointer(node, 4) = nullptr;
    pointer(node, 8) = nullptr;
    byte(node, color) = 1;
    byte(node, color + 1) = 0;
    return node;
}
void* list_head(std::size_t size) {
    void* const node = singleton_lifetime_allocate({SingletonAllocationKind::object, size, size});
    pointer(node, 0) = node;
    pointer(node, 4) = node;
    return node;
}
void finish_tree(PowerupNativeContainer& tree, std::size_t nil_offset) {
    byte(tree.head_04, nil_offset) = 1;
    pointer(tree.head_04, 4) = tree.head_04;
    pointer(tree.head_04, 0) = tree.head_04;
    pointer(tree.head_04, 8) = tree.head_04;
    tree.count_08 = 0;
}
void float_copy(float& output, const float& input) noexcept {
    auto* destination = &output;
    const auto* source = &input;
    __asm {
        mov eax, source
        fld dword ptr [eax]
        mov eax, destination
        fstp dword ptr [eax]
    }
}
void release_texture(void*& field, PowerupConfigHost& host) {
    auto* const texture = field;
    if (!texture) return;
    if (InterlockedDecrement(reinterpret_cast<volatile LONG*>(
            static_cast<unsigned char*>(texture) + 4)) == 0)
        host.texture_zero_references(texture);
    field = nullptr; // native nulling follows the callback
}
struct OwnedString {
    NativeStringStorage& storage;
    NativeString value;
    ~OwnedString() { destroy_native_string_header_0041dd20(&value, storage); }
};
struct CapturedString {
    NativeStringStorage& storage;
    NativeString value;
    char* data{};
    std::uint32_t size{};
    void capture() { data = value.data(); size = value.length() + 1u; }
    ~CapturedString() { if (data) storage.release(data, size); }
};
struct CategoryString {
    NativeStringStorage& storage;
    NativeString value;
    char* captured_data{};
    ~CategoryString() {
        if (captured_data) storage.release(captured_data, value.length() + 1u);
    }
};
struct GroupStrings {
    NativeStringStorage& storage;
    std::array<NativeString, 4> values;
    ~GroupStrings() {
        for (auto it = values.rbegin(); it != values.rend(); ++it)
            destroy_native_string_header_0041dd20(&*it, storage);
    }
};
struct OwnedRef {
    GuiLua51Host& host;
    GuiLuaRef value;
    ~OwnedRef() { host.release(value); }
};
// NativeString destination adaptation of existing00BD63B0/00BD61C0. Keep the
// looked-up Lua object alive THROUGH actual pooled resize and memcpy.
void read_string(NativeString& destination, GuiLua51Host& host, GuiLuaRef table,
    const char* key, bool has_default, PowerupConfigContext& context) {
    OwnedRef object{host, host.get_by_name(table, key)};
    const bool use_default = has_default && gui_lua_is_nil_00b65fb0(host, object.value);
    const char* text = use_default ? "" : host.to_string(object.value);
    const auto length = text ? static_cast<std::uint32_t>(std::strlen(text)) : 0u;
    destination.resize_0041dd40(context.strings, length, false);
    if (destination.data() && destination.length())
        std::memcpy(destination.data(), text, destination.length());
}
template<class T>
void read_scalar(T& destination, GuiLuaFieldType type, GuiLua51Host& host,
    GuiLuaRef table, const char* key, PowerupConfigContext& context,
    const GuiLuaVariant* fallback = nullptr) {
    OwnedRef object{host, host.get_by_name(table, key)};
    const auto field = gui_lua_field(type, &destination);
    if (fallback && gui_lua_is_nil_00b65fb0(host, object.value))
        (void)gui_lua_store_default_00bd61c0(field, *fallback);
    else
        (void)gui_lua_store_ref_00bd63b0(host, object.value, field, nullptr,
            context.crt_sse2_conversion);
}
GuiLuaVariant integer_default(std::int32_t value) {
    GuiLuaVariant result; result.tag = 1; result.value.integer = value; return result;
}
GuiLuaVariant float_default(float value) {
    GuiLuaVariant result; result.tag = 2; result.value.number = value; return result;
}
void enumerate_keys(GuiLuaKeyArray& keys, GuiLua51Host& host, GuiLuaRef table,
    const bool& mode) {
    for (auto& key : keys.keys) key.tag = -1;
    keys.count = 0;
    GuiLuaReader enumerator(host, host.copy_ref_00b66fa0(table), mode);
    enumerator.enumerate_keys_00bd5f50(keys);
}
const char* string_key(const GuiLuaVariant& key) {
    // Native008ECEC0 unconditionally treats the enumerated word as char*.
    // Reject malformed authoring instead of dereferencing an integer as a pointer.
    if (key.tag != 0 || !key.value.text)
        throw std::invalid_argument("PowerupClasses group and class keys require strings");
    return key.value.text;
}
void append_name(std::vector<NativeString>& values, const NativeString& source,
    NativeStringStorage& strings) {
    //00450540's owned-value operation through the standard vector projection;
    // native checked iterators/capacity policy and relocation timing are not ported.
    OwnedString copy{strings, {}};
    copy_construct_native_string_header_00426060(&copy.value, &source, strings);
    values.push_back(std::move(copy.value));
}
}

PowerupConfigOwner::PowerupConfigOwner(const PowerupOwnerAllocationWords& words) noexcept {
    native.flag_word_10 = words.flag_word_10;
    std::size_t index = 0;
    native.descriptors_14.allocator_00 = words.allocator_words[index++];
    for (auto& cell : native.lists_20) cell.allocator_00 = words.allocator_words[index++];
    for (auto& cell : native.lists_80) cell.allocator_00 = words.allocator_words[index++];
    for (auto& cell : native.maps_14c) cell.allocator_00 = words.allocator_words[index++];
    native.map_1ac.allocator_00 = words.allocator_words[index++];
    native.random_names_1b8.allocator_00 = words.allocator_words[index];
}
void* allocate_powerup_class_node_008e5c50() { return tree_node(0xfc, 0xf8); }
void* allocate_powerup_runtime_map_node_008e5ca0() { return tree_node(0x2c, 0x28); }
void* allocate_name_vector_map_node_008e5cf0() { return tree_node(0x28, 0x24); }
void* allocate_powerup_list_head_008e5960() { return list_head(0x24); }
void* allocate_powerup_large_list_head_008e5980() { return list_head(0x30); }
PowerupNativeContainer& construct_powerup_list_008e65a0(PowerupNativeContainer& cell) {
    cell.head_04 = allocate_powerup_list_head_008e5960(); cell.count_08 = 0; return cell;
}
PowerupNativeContainer& construct_powerup_large_list_008e65c0(PowerupNativeContainer& cell) {
    cell.head_04 = allocate_powerup_large_list_head_008e5980(); cell.count_08 = 0; return cell;
}
PowerupNativeContainer& construct_powerup_runtime_map_008ea750(PowerupNativeContainer& cell) {
    cell.head_04 = allocate_powerup_runtime_map_node_008e5ca0();
    finish_tree(cell, 0x29); return cell;
}
PowerupConfigOwner& construct_powerup_config_008edc60(PowerupConfigOwner& owner,
    PowerupConfigOwner*& publication) {
    auto& native = owner.native;
    native.base_vector_04 = {nullptr, nullptr, nullptr};
    native.flag_word_10 &= 0xffffff00u;
    native.vtable_00 = 0x00d1629c;
    // Host exception cleanup tracks only successfully constructed heads.
    // It is not a reconstruction of the compiler's SEH unwind tables.
    std::array<void*, 35> created{};
    std::size_t count = 0;
    try {
        native.descriptors_14.head_04 = allocate_powerup_class_node_008e5c50();
        created[count++] = native.descriptors_14.head_04;
        finish_tree(native.descriptors_14, 0xf9);
        owner.descriptors_14.emplace();
        for (auto& cell : native.lists_20) {
            construct_powerup_list_008e65a0(cell); created[count++] = cell.head_04;
        }
        for (auto& cell : native.lists_80) {
            construct_powerup_large_list_008e65c0(cell); created[count++] = cell.head_04;
        }
        native.vector_140 = {nullptr, nullptr, nullptr};
        for (auto& cell : native.maps_14c) {
            construct_powerup_runtime_map_008ea750(cell); created[count++] = cell.head_04;
        }
        native.map_1ac.head_04 = allocate_name_vector_map_node_008e5cf0();
        created[count++] = native.map_1ac.head_04;
        finish_tree(native.map_1ac, 0x25);
        native.random_names_1b8.head_04 = allocate_name_vector_map_node_008e5cf0();
        created[count++] = native.random_names_1b8.head_04;
        finish_tree(native.random_names_1b8, 0x25);
        owner.random_names_1b8.emplace();
    } catch (...) {
        while (count) singleton_lifetime_free(created[--count]);
        owner.random_names_1b8.reset(); owner.descriptors_14.reset();
        throw;
    }
    publication = &owner;
    return owner;
}

void prepare_powerup_descriptor_allocation(PowerupClassConfig& value,
    const PowerupDescriptorAllocationWords& words) noexcept {
    std::size_t index = 0;
    auto store = [&](std::size_t offset) {
        std::memcpy(reinterpret_cast<unsigned char*>(&value) + offset,
            &words.scalar_words[index++], 4);
    };
    store(8);
    for (std::size_t faction : {std::size_t{0x0c}, std::size_t{0x44}})
        for (std::size_t offset = 0x10; offset <= 0x20; offset += 4) store(faction + offset);
    for (std::size_t offset = 0x7c; offset <= 0xcc; offset += 4) store(offset);
}
PowerupFactionConfig& construct_powerup_faction_008e6130(PowerupFactionConfig& value) {
    new (&value.name_00) NativeString;
    new (&value.description_08) NativeString;
    value.uv_28 = {{0, 0, 1, 1}};
    value.texture_24 = nullptr;
    return value;
}
PowerupClassConfig& construct_powerup_class_008e6720(PowerupClassConfig& value) {
    new (&value.name_00) NativeString;
    for (auto& faction : value.factions_0c) construct_powerup_faction_008e6130(faction);
    value.uv_d4 = {{0, 0, 1, 1}};
    value.texture_d0 = nullptr;
    return value;
}
PowerupFactionConfig& copy_powerup_faction_008e7120(PowerupFactionConfig& out,
    const PowerupFactionConfig& in, NativeStringStorage& strings) {
    copy_construct_native_string_header_00426060(&out.name_00, &in.name_00, strings);
    copy_construct_native_string_header_00426060(&out.description_08, &in.description_08, strings);
    out.word_10 = in.word_10; out.word_14 = in.word_14;
    out.unit_class_index_18 = in.unit_class_index_18;
    out.equipment_index_1c = in.equipment_index_1c;
    out.word_20 = in.word_20;
    out.texture_24 = in.texture_24; // no retain in008E71CF..71D5
    std::memcpy(out.uv_28.data(), in.uv_28.data(), 16);
    return out;
}
PowerupClassConfig& copy_powerup_class_008e79d0(PowerupClassConfig& out,
    const PowerupClassConfig& in, NativeStringStorage& strings) {
    copy_construct_native_string_header_00426060(&out.name_00, &in.name_00, strings);
    out.group_08 = in.group_08;
    for (std::size_t i = 0; i != 2; ++i)
        copy_powerup_faction_008e7120(out.factions_0c[i], in.factions_0c[i], strings);
    float_copy(out.duration_7c, in.duration_7c);
    float_copy(out.cooldown_80, in.cooldown_80);
    out.target_type_84 = in.target_type_84; out.target_filter_88 = in.target_filter_88;
    std::memcpy(out.multipliers_8c.data(), in.multipliers_8c.data(), 64);
    out.random_cc = in.random_cc; // adjacent padding remains destination storage
    out.texture_d0 = in.texture_d0; // no retain in008E7A90..7A9C
    std::memcpy(out.uv_d4.data(), in.uv_d4.data(), 16);
    return out;
}
void destroy_powerup_faction_008e6170(PowerupFactionConfig& value,
    PowerupConfigContext& context) {
    release_texture(value.texture_24, context.host);
    destroy_native_string_header_0041dd20(&value.description_08, context.strings);
    destroy_native_string_header_0041dd20(&value.name_00, context.strings);
}
void destroy_powerup_class_008e67b0(PowerupClassConfig& value,
    PowerupConfigContext& context) {
    release_texture(value.texture_d0, context.host);
    destroy_powerup_faction_008e6170(value.factions_0c[1], context);
    destroy_powerup_faction_008e6170(value.factions_0c[0], context);
    destroy_native_string_header_0041dd20(&value.name_00, context.strings);
}

PowerupClassMap::iterator lower_bound_powerup_class_008e5b10(PowerupClassMap& values,
    const NativeString& key) { return values.lower_bound(key); }
PowerupNameVectorMap::iterator lower_bound_powerup_names_008e5bb0(PowerupNameVectorMap& values,
    const NativeString& key) { return values.lower_bound(key); }
PowerupClassConfig& lookup_powerup_class_008eb4a0(PowerupConfigOwner& owner,
    const NativeString& key, PowerupConfigContext& context) {
    auto& values = *owner.descriptors_14;
    auto found = lower_bound_powerup_class_008e5b10(values, key);
    if (found != values.end() && !native_string_less_case_insensitive_00443d00(key, found->first))
        return found->second;
    PowerupClassConfig initial;
    prepare_powerup_descriptor_allocation(initial, context.descriptor_scratch);
    construct_powerup_class_008e6720(initial);
    OwnedString pair_key{context.strings, {}};
    copy_construct_native_string_header_00426060(&pair_key.value, &key, context.strings);
    PowerupClassConfig pair;
    copy_powerup_class_008e79d0(pair, initial, context.strings);
    OwnedString node_key{context.strings, {}};
    copy_construct_native_string_header_00426060(&node_key.value, &pair_key.value, context.strings);
    PowerupClassConfig node;
    copy_powerup_class_008e79d0(node, pair, context.strings);
    found = values.emplace_hint(found, std::piecewise_construct,
        std::forward_as_tuple(std::move(node_key.value)), std::forward_as_tuple(std::move(node)));
    ++owner.native.descriptors_14.count_08;
    //008E70B0 destroys pair descriptor before its key, then008E67B0 initial.
    destroy_powerup_class_008e67b0(pair, context);
    destroy_native_string_header_0041dd20(&pair_key.value, context.strings);
    new (&pair_key.value) NativeString;
    destroy_powerup_class_008e67b0(initial, context);
    return found->second;
}
std::vector<NativeString>& lookup_powerup_names_008ec460(PowerupConfigOwner& owner,
    const NativeString& key, NativeStringStorage& strings) {
    auto& values = *owner.random_names_1b8;
    auto found = lower_bound_powerup_names_008e5bb0(values, key);
    if (found != values.end() && !native_string_less_case_insensitive_00443d00(key, found->first))
        return found->second;
    OwnedString pair_key{strings, {}};
    copy_construct_native_string_header_00426060(&pair_key.value, &key, strings);
    OwnedString node_key{strings, {}};
    copy_construct_native_string_header_00426060(&node_key.value, &pair_key.value, strings);
    found = values.emplace_hint(found, std::piecewise_construct,
        std::forward_as_tuple(std::move(node_key.value)), std::forward_as_tuple());
    ++owner.native.random_names_1b8.count_08;
    return found->second;
}

void load_powerup_faction_008e6850(PowerupFactionConfig& value, GuiLua51Host& lua,
    GuiLuaRef table, PowerupConfigContext& context) {
    read_string(value.name_00, lua, table, "name", false, context);
    read_string(value.description_08, lua, table, "description", false, context);
    const auto zero = integer_default(0);
    read_scalar(value.unit_class_index_18, GuiLuaFieldType::Int, lua, table,
        "unitClassIndex", context, &zero);
    value.word_20 = 0;
    read_scalar(value.equipment_index_1c, GuiLuaFieldType::Int, lua, table,
        "equipmentIndex", context, &zero);
    OwnedString icon{context.strings, {}};
    read_string(icon.value, lua, table, "mapIconPicture", true, context);
    // Comparison to a freshly constructed empty string has no allocation and
    // is exactly the native length-zero gate; no locale compare is reached.
    if (icon.value.length() != 0) {
        const auto& callbacks = context.host.current_gui_texture_callbacks_004c12b0();
        std::array<float, 2> size;
        std::memcpy(size.data(), context.faction_texture_size_words.data(), 8);
        value.texture_24 = resolve_gui_texture_00aa2660(icon.value.data(), value.uv_28,
            size, 1.0f, callbacks);
    } // Empty icon deliberately retains an existing faction texture.
}
void load_powerup_class_008e7350(PowerupClassConfig& value, GuiLua51Host& lua,
    GuiLuaRef table, PowerupConfigContext& context) {
    {
        OwnedRef allied{lua, lua.get_by_name(table, "allied")};
        load_powerup_faction_008e6850(value.factions_0c[0], lua, allied.value, context);
    }
    {
        OwnedRef japanese{lua, lua.get_by_name(table, "japanese")};
        load_powerup_faction_008e6850(value.factions_0c[1], lua, japanese.value, context);
    }
    const auto zero = float_default(0);
    read_scalar(value.duration_7c, GuiLuaFieldType::Float, lua, table, "duration", context, &zero);
    read_scalar(value.cooldown_80, GuiLuaFieldType::Float, lua, table, "cooldown", context, &zero);
    std::int32_t target;
    read_scalar(target, GuiLuaFieldType::Int, lua, table, "targetType", context);
    value.target_type_84 = target; //008E7471: after reader/temporary-ref cleanup
    const auto filter = integer_default(15);
    read_scalar(target, GuiLuaFieldType::Int, lua, table, "targetFilter", context, &filter);
    value.target_filter_88 = target; //008E74B1: same local scratch slot
    OwnedRef random{lua, lua.get_by_name(table, "random")};
    value.random_cc = gui_lua_is_nil_00b65fb0(lua, random.value) ? 1 : lua.to_boolean(random.value);
    lua.release(random.value); random.value = {};
    OwnedString icon{context.strings, {}};
    read_string(icon.value, lua, table, "PUMicon", true, context);
    if (icon.value.length() == 0) {
        const auto& faction = context.host.current_selected_player_faction_28();
        if (faction == 0) read_string(icon.value, lua, table, "USPUMicon", true, context);
        else if (faction == 1) read_string(icon.value, lua, table, "JapPUMicon", true, context);
    }
    if (icon.value.length() != 0) {
        const auto& callbacks = context.host.current_gui_texture_callbacks_004c12b0();
        std::array<float, 2> size{{0, 0}}; // reused fresh empty NativeString header
        value.texture_d0 = resolve_gui_texture_00aa2660(icon.value.data(), value.uv_d4,
            size, 1.0f, callbacks);
    } else value.texture_d0 = nullptr; // old pointer is not released
    value.multipliers_8c.fill(1.0f);
    bool has_multipliers;
    {
        OwnedRef test{lua, lua.get_by_name(table, "multipliers")};
        has_multipliers = !gui_lua_is_nil_00b65fb0(lua, test.value);
    }
    if (has_multipliers) {
        OwnedRef multipliers{lua, lua.get_by_name(table, "multipliers")};
        for (std::int32_t index = 0; index != 16; ++index) {
            OwnedRef number{lua, lua.get_by_index(multipliers.value, index)};
            value.multipliers_8c[index] = gui_lua_is_nil_00b65fb0(lua, number.value)
                ? 1.0f : lua_object_number_00b66270(lua, number.value);
        }
    }
}

void load_powerup_config_008ecec0(PowerupConfigOwner& owner,
    LuaStateOwnerEnvironment environment, LuaScriptRuntime& scripts,
    PowerupConfigContext& context) {
    PcStorageLuaOwner state(std::move(environment));
    state.open_storage_archive_00b6a020(4);
    {
        OwnedString path{context.strings, {}};
        path.value.assign_0041e870(context.strings, "Scripts/datatables/PowerupClasses.lua");
        (void)scripts.run_file(state.storage_lua_38(), path.value.data(), false);
    }
    {
    GroupStrings group_names{context.strings, {}};
    const char* const group_texts[] = {"A*", "S*", "T*", "  "};
    for (std::size_t index = 0; index != 4; ++index)
        group_names.values[index].assign_0041e870(context.strings, group_texts[index]);
    {
        GuiLua51Host lua(*state.storage_lua_38());
        OwnedRef globals{lua, lua.globals()};
        OwnedRef table{lua, lua.get_by_name(globals.value, "PowerupClasses")};
        lua.release(globals.value); globals.value = {};
        // Native seeds/copies the reader then tidies the first vector. Host
        // references provide independent retained Lua identity, not STL layout.
        OwnedRef reader{lua, lua.copy_ref_00b66fa0(table.value)};
        for (std::int32_t group = 1; group <= 3; ++group) {
            OwnedRef group_table{lua, lua.get_by_index(reader.value, group)};
            GuiLuaKeyArray categories;
            enumerate_keys(categories, lua, group_table.value, context.crt_sse2_conversion);
            for (std::int32_t c = 0; c < categories.count; ++c) {
                CategoryString category{context.strings, {}};
                category.value.assign_0041e870(context.strings, string_key(categories.keys[c]));
                category.captured_data = category.value.data(); //008ED1CC EDI
                OwnedRef category_table{lua, lua.get_by_name(group_table.value,
                    category.value.data() ? category.value.data() : "")};
                GuiLuaKeyArray names;
                enumerate_keys(names, lua, category_table.value, context.crt_sse2_conversion);
                const bool entered_inner_loop = names.count > 0;
                for (std::int32_t n = 0; n < names.count; ++n) {
                    CapturedString name{context.strings, {}};
                    name.value.assign_0041e870(context.strings, string_key(names.keys[n]));
                    name.capture(); // EDI=data/EBP=length survive insertion and callbacks
                    auto& descriptor = lookup_powerup_class_008eb4a0(owner, name.value, context);
                    if (&descriptor.name_00 != &name.value) {
                        descriptor.name_00.resize_0041dd40(context.strings, name.size - 1u, true);
                        if (name.size != 1u)
                            std::memcpy(descriptor.name_00.data(), name.data, descriptor.name_00.length());
                    }
                    descriptor.group_08 = group;
                    {
                        OwnedRef item{lua, lua.get_by_name(category_table.value, string_key(names.keys[n]))};
                        load_powerup_class_008e7350(descriptor, lua, item.value, context);
                    } // reader leave before testing CURRENT random byte
                    if (descriptor.random_cc) {
                        auto& category_names = lookup_powerup_names_008ec460(owner, category.value, context.strings);
                        append_name(category_names, name.value, context.strings);
                        auto& type_names = lookup_powerup_names_008ec460(owner, group_names.values[group - 1], context.strings);
                        append_name(type_names, name.value, context.strings);
                    }
                }
                //008ED395 reloads data only after a nonempty inner loop. The
                // following reader leave precedes CURRENT length on pool return.
                if (entered_inner_loop) category.captured_data = category.value.data();
            } // category reader leaves before category string cleanup
        }
    } // reader refs then table ref, before four group strings and Lua close
    }
    state.close_storage_archive_00b65e80();
}

void release_powerup_configuration_storage(PowerupConfigOwner& owner,
    PowerupConfigContext& context) {
    const auto empty = [](const auto& cells) {
        for (const auto& cell : cells) if (cell.count_08 != 0) return false;
        return true;
    };
    auto& native = owner.native;
    if (native.base_vector_04.begin_00 || native.vector_140.begin_00 ||
        !empty(native.lists_20) || !empty(native.lists_80) || !empty(native.maps_14c) ||
        native.map_1ac.count_08 != 0)
        throw std::logic_error("Powerup config cleanup requires unused runtime containers");
    if (owner.random_names_1b8) {
        auto& names = *owner.random_names_1b8;
        while (!names.empty()) {
            auto current = std::prev(names.end());
            for (auto& value : current->second)
                destroy_native_string_header_0041dd20(&value, context.strings);
            destroy_native_string_header_0041dd20(const_cast<NativeString*>(&current->first), context.strings);
            names.erase(current);
        }
        owner.random_names_1b8.reset();
    }
    if (owner.descriptors_14) {
        auto& classes = *owner.descriptors_14;
        while (!classes.empty()) {
            auto current = std::prev(classes.end());
            destroy_powerup_class_008e67b0(current->second, context);
            destroy_native_string_header_0041dd20(const_cast<NativeString*>(&current->first), context.strings);
            classes.erase(current);
        }
        owner.descriptors_14.reset();
    }
    const auto free_head = [](PowerupNativeContainer& cell) {
        singleton_lifetime_free(cell.head_04); cell.head_04 = nullptr; cell.count_08 = 0;
    };
    free_head(native.random_names_1b8); free_head(native.map_1ac);
    for (auto it = native.maps_14c.rbegin(); it != native.maps_14c.rend(); ++it) free_head(*it);
    for (auto it = native.lists_80.rbegin(); it != native.lists_80.rend(); ++it) free_head(*it);
    for (auto it = native.lists_20.rbegin(); it != native.lists_20.rend(); ++it) free_head(*it);
    free_head(native.descriptors_14);
}

} // namespace bsp
