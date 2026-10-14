// Packet cc13-seg8: segment 8 class data and loaders. See
// include/bsp/classes_seg8_cc13.hpp and docs/CC13_SEG8.md. Each body carries
// its address, the original ABI and every simplification next to it.

#include "bsp/classes_seg8_cc13.hpp"

#include <cstring>

#include "bsp/engine_seg4_6_9_cc13.hpp"

namespace bsp::classes_seg8_cc13 {
namespace {

// 00E188A8: the game object; its +1A08h holds the Lua owner whose +4 is the machine.
constexpr std::uint32_t kGame = 0x00e188a8;
constexpr std::uint32_t kSingletonBaseVtable = 0x00ce3818;
// 00D7A24C 1.0f, 00CE3D08 100.0f, 00CE3800 0.5f, 00CE3814 1.2f.
constexpr float kOne = 1.0f;
constexpr float kHundred = 100.0f;
constexpr float kHalf = 0.5f;
constexpr float kOnePointTwo = 1.2f;

Raw as_raw(void* p) noexcept { return Raw{static_cast<std::uint8_t*>(p)}; }
Raw raw_from_u32(std::uint32_t value) noexcept {
    return Raw{reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(value))};
}

// The three "<Class>=nil\n" teardowns: 006B8AD0(text, 0, 0, 2) on the machine
// at [[00E188A8]+1A08h]+4.
void clear_lua_global(const char* text, Seg8Host& host) {
    const Raw game = raw_from_u32(host.global_u32(kGame));
    void* const machine = as_raw(game.ptr(0x1a08)).ptr(0x4);
    host.lua_run_string_006b8ad0(machine, text, 0, 0, 2);
}

// The intrusive reference release: InterlockedDecrement(+4) and slot 0 (no
// arguments) at zero; the field is cleared whenever it was set.
void release_ref(Raw owner, std::size_t field, Seg8Host& host) {
    void* const object = owner.ptr(field);
    if (object != nullptr) {
        auto* const count = reinterpret_cast<long*>(static_cast<std::uint8_t*>(object) + 4);
        if (host.interlocked_decrement_ce2220(count) == 0) host.object_slot_00(object);
        owner.ptr(field) = nullptr;
    }
}

// The point-effect stop: when set, 00867B10 on it and its byte +9 = 1.
void stop_effect(void* effect, Seg8Host& host) {
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        as_raw(effect).at<std::uint8_t>(0x9) = 1;
    }
}

void store_entity_vtables(Raw entity, std::uint32_t primary, std::uint32_t at_10, std::uint32_t at_24) {
    entity.at<std::uint32_t>(0x0) = primary;
    entity.at<std::uint32_t>(0x10) = at_10;
    entity.at<std::uint32_t>(0x24) = at_24;
}

// 004A7650 / 004AC0B0: node = [[part+178h]+160h]+0Ch; its vtable +38h(part+74h),
// then 00B6DA70(node, float +18Ch, 1). The node is re-read for the second call.
void refresh_visibility(Raw part, Seg8Host& host) {
    void* node = as_raw(as_raw(part.ptr(0x178)).ptr(0x160)).ptr(0xc);
    host.scene_node_slot_38(node, part.p + 0x74);
    node = as_raw(as_raw(part.ptr(0x178)).ptr(0x160)).ptr(0xc);
    host.scene_node_set_visibility_factor_00b6da70(node, part.at<float>(0x18c), 1);
}

// 004A7B50 / 004AC870, __thiscall(part, source), RET 4, EAX = part.
Raw construct_part(Raw part, std::uint32_t source, std::uint32_t primary, std::uint32_t at_10,
                   std::uint32_t at_24, std::uint32_t class_id, Seg8Host& host) {
    host.class_entity_construct_base_00489f30(part);
    store_entity_vtables(part, primary, at_10, at_24);
    part.at<std::uint32_t>(0x188) = 0;
    part.at<std::uint32_t>(0x170) = source;
    part.at<std::uint32_t>(0xc4) = class_id;
    part.at<float>(0x18c) = kOne;
    return part;
}

// 004A82C0 / 004ACFA0: vtables, stop the effect at +188h, release it, base destructor.
void destruct_part(Raw part, std::uint32_t primary, std::uint32_t at_10, std::uint32_t at_24, Seg8Host& host) {
    store_entity_vtables(part, primary, at_10, at_24);
    stop_effect(part.ptr(0x188), host);
    release_ref(part, 0x188, host);
    host.class_entity_destruct_base_00487380(part);
}

// 004A7C10 / 004AC990 / 004B1F90: own vtable, release the second then the
// first reference, named-class base destructor.
void destruct_class(Raw cls, std::uint32_t vtable, std::size_t first, std::size_t second, Seg8Host& host) {
    cls.at<std::uint32_t>(0) = vtable;
    release_ref(cls, second, host);
    release_ref(cls, first, host);
    host.named_class_destruct_base_00489e80(cls);
}

// 004A7CB0 / 004ACA30: operator new 190h (00BF55BE), memset 0 before the
// null test (as the native), construct with the class as source, or null.
void* create_part(Raw cls, std::uint32_t primary, std::uint32_t at_10, std::uint32_t at_24, std::uint32_t class_id,
                  Seg8Host& host) {
    void* const memory = host.operator_new_00bf55be(0x190);
    std::memset(memory, 0, 0x190);
    if (memory == nullptr) return nullptr;
    const auto source = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(cls.p));
    return construct_part(as_raw(memory), source, primary, at_10, at_24, class_id, host).p;
}

// The shared string release: data +4 set -> (data, length + 1, 1) to the pool.
void release_pooled_string(Raw string, Seg8Host& host) {
    void* const data = string.ptr(0x4);
    if (data != nullptr) {
        const std::uint32_t bytes = string.at<std::uint32_t>(0x0) + 1;
        void* const pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, data, bytes, 1);
    }
}

void singleton_base_destruct(Raw object, std::uint32_t global, Seg8Host& host) {
    host.global_u32(global) = 0;
    object.at<std::uint32_t>(0) = kSingletonBaseVtable;
}

// 004A9960 / 004AEC00, __thiscall(ECX = name string {length, data}), RET.
// registry[name]; when empty: G = globals (00B67980 on game+1A0Ch), T =
// G[class table], E = T[name data or the empty string], a new class
// (operator new 00BF681B, null on failure), its name string (+4) assigned
// from `name` (0041DD40 resize then memcpy, skipped when the same object),
// vtable +4(E), registry[name] = it, destroy E, T, G. Then vtable +0Ch on the
// class either way (also when the allocation failed; the native does the same).
template <class Construct, class Registry, class Subscript>
void* load_from_globals(Raw name, const char* table, std::uint32_t empty_string, std::uint32_t size,
                        Construct construct, Registry registry, Subscript subscript, Seg8Host& host) {
    void* cls = *subscript(Raw{registry().p + 0x4}, name);
    if (cls == nullptr) {
        LuaObject globals{};
        LuaObject class_table{};
        LuaObject entry{};
        const Raw game = raw_from_u32(host.global_u32(kGame));
        host.lua_get_globals_00b67980(game.p + 0x1a0c, &globals);
        host.lua_get_by_name_00b67800(&globals, &class_table, table);
        const char* key = static_cast<const char*>(name.ptr(0x4));
        if (key == nullptr) key = static_cast<const char*>(host.image_address(empty_string));
        host.lua_get_by_name_00b67800(&class_table, &entry, key);
        void* const memory = host.operator_new_00bf681b(size);
        cls = memory != nullptr ? construct(as_raw(memory)).p : nullptr;
        const Raw cls_name{static_cast<std::uint8_t*>(cls) + 0x4};
        if (cls_name.p != name.p) {
            host.native_string_resize_0041dd40(cls_name, name.at<std::uint32_t>(0x0), 1);
            if (name.at<std::uint32_t>(0x0) != 0) {
                std::memcpy(cls_name.ptr(0x4), name.ptr(0x4), cls_name.at<std::uint32_t>(0x0));
            }
        }
        host.class_slot_04(cls, &entry);
        *subscript(Raw{registry().p + 0x4}, name) = cls;
        host.lua_object_destruct_00b67700(&entry);
        host.lua_object_destruct_00b67700(&class_table);
        host.lua_object_destruct_00b67700(&globals);
    }
    host.class_slot_0c(cls);
    return cls;
}

}  // namespace

// 004A7120 / 004AF930 / 004B1890, __cdecl, RET: "PlanePartClass=nil\n"
// (00CE6A78), "SoldierClass=nil\n" (00CE712C), "WreckClass=nil\n" (00CE7174).
void plane_part_class_clear_lua_global_004a7120(Seg8Host& host) { clear_lua_global("PlanePartClass=nil\n", host); }
void soldier_class_clear_lua_global_004af930(Seg8Host& host) { clear_lua_global("SoldierClass=nil\n", host); }
void wreck_class_clear_lua_global_004b1890(Seg8Host& host) { clear_lua_global("WreckClass=nil\n", host); }

// 004A7650 / 004AC0B0, __thiscall(part), RET.
void plane_part_refresh_visibility_004a7650(Raw part, Seg8Host& host) { refresh_visibility(part, host); }
void ship_part_refresh_visibility_004ac0b0(Raw part, Seg8Host& host) { refresh_visibility(part, host); }

// 004A7B50: vtables 00CE6AB0 / 00CE6A98 / 00CE6A90, class id 3Fh.
Raw plane_part_construct_004a7b50(Raw part, std::uint32_t source, Seg8Host& host) {
    return construct_part(part, source, 0x00ce6ab0, 0x00ce6a98, 0x00ce6a90, 0x3f, host);
}
// 004AC870: vtables 00CE6E10 / 00CE6DF8 / 00CE6DF0, class id 40h.
Raw ship_part_construct_004ac870(Raw part, std::uint32_t source, Seg8Host& host) {
    return construct_part(part, source, 0x00ce6e10, 0x00ce6df8, 0x00ce6df0, 0x40, host);
}

// 004AF7E0, __thiscall(soldier), RET, EAX = soldier: base 00489F30, vtables
// 00CE7008 / 00CE6FEC / 00CE6FE4, class id 3Ah.
Raw soldier_construct_004af7e0(Raw soldier, Seg8Host& host) {
    host.class_entity_construct_base_00489f30(soldier);
    store_entity_vtables(soldier, 0x00ce7008, 0x00ce6fec, 0x00ce6fe4);
    soldier.at<std::uint32_t>(0xc4) = 0x3a;
    return soldier;
}

// 004B1EE0, __thiscall(wreck), RET, EAX = wreck: base 00489F30, vtables
// 00CE71A8 / 00CE7190 / 00CE7188, effects +18Ch/+190h = 0, class id 3Bh.
Raw wreck_construct_004b1ee0(Raw wreck, Seg8Host& host) {
    host.class_entity_construct_base_00489f30(wreck);
    store_entity_vtables(wreck, 0x00ce71a8, 0x00ce7190, 0x00ce7188);
    wreck.at<std::uint32_t>(0x18c) = 0;
    wreck.at<std::uint32_t>(0x190) = 0;
    wreck.at<std::uint32_t>(0xc4) = 0x3b;
    return wreck;
}

// 004A82C0 / 004ACFA0, __thiscall(part), RET (SEH frame only for unwinding).
void plane_part_destruct_004a82c0(Raw part, Seg8Host& host) {
    destruct_part(part, 0x00ce6ab0, 0x00ce6a98, 0x00ce6a90, host);
}
void ship_part_destruct_004acfa0(Raw part, Seg8Host& host) {
    destruct_part(part, 0x00ce6e10, 0x00ce6df8, 0x00ce6df0, host);
}

// 004B2590, __thiscall(wreck), RET: vtables; stop the effects at +18Ch then
// +190h; release +190h then +18Ch; base destructor 00487380.
void wreck_destruct_004b2590(Raw wreck, Seg8Host& host) {
    store_entity_vtables(wreck, 0x00ce71a8, 0x00ce7190, 0x00ce7188);
    stop_effect(wreck.ptr(0x18c), host);
    stop_effect(wreck.ptr(0x190), host);
    release_ref(wreck, 0x190, host);
    release_ref(wreck, 0x18c, host);
    host.class_entity_destruct_base_00487380(wreck);
}

// 004B2680, __thiscall(wreck, archive), RET 4: 00924D90(archive) with ECX =
// wreck; then write("wreckclass", class name or the empty string at
// 00E187FC), begin("_wreck"), write("state", int +194h), write("timeLeft",
// float +198h), end. Strings are tag 0, ints tag 1, floats tag 2.
void wreck_save_archive_004b2680(Raw wreck, void* archive, Seg8Host& host) {
    host.entity_save_base_00924d90(wreck, archive);
    const Raw cls{static_cast<std::uint8_t*>(wreck.ptr(0x188))};
    void* class_name = cls.ptr(0x8);
    if (class_name == nullptr) class_name = host.image_address(0x00e187fc);
    const auto u32 = [](const void* p) { return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(p)); };
    host.archive_write_slot_0c(archive, TaggedValue{0, u32("wreckclass")}, TaggedValue{0, u32(class_name)});
    host.archive_begin_slot_04(archive, TaggedValue{0, u32("_wreck")});
    host.archive_write_slot_0c(archive, TaggedValue{0, u32("state")}, TaggedValue{1, wreck.at<std::uint32_t>(0x194)});
    host.archive_write_slot_0c(archive, TaggedValue{0, u32("timeLeft")}, TaggedValue{2, wreck.at<std::uint32_t>(0x198)});
    host.archive_end_slot_08(archive);
}

// 004A7BF0, __thiscall(class), RET, EAX = class: 00489E60, vtable 00CE6BD4,
// +5Ch and +60h = 0.
Raw plane_part_class_construct_004a7bf0(Raw cls, Seg8Host& host) {
    host.named_class_construct_base_00489e60(cls);
    cls.at<std::uint32_t>(0) = 0x00ce6bd4;
    cls.at<std::uint32_t>(0x5c) = 0;
    cls.at<std::uint32_t>(0x60) = 0;
    return cls;
}

// 004AC910, __thiscall(class), RET, EAX = class: 00489E60, vtable 00CE6F34,
// +54h/+58h = 0, +2Ch = +28h = 100.0f, +30h = 0.5f, +34h = 1.2f, +38h..+4Ch
// = 0.0f, +50h = 1.0f (stores in the listing order).
Raw ship_part_class_construct_004ac910(Raw cls, Seg8Host& host) {
    host.named_class_construct_base_00489e60(cls);
    cls.at<std::uint32_t>(0) = 0x00ce6f34;
    cls.at<std::uint32_t>(0x54) = 0;
    cls.at<std::uint32_t>(0x58) = 0;
    cls.at<float>(0x2c) = kHundred;
    cls.at<float>(0x28) = kHundred;
    cls.at<float>(0x30) = kHalf;
    cls.at<float>(0x34) = kOnePointTwo;
    cls.at<float>(0x3c) = 0.0f;
    cls.at<float>(0x38) = 0.0f;
    cls.at<float>(0x44) = 0.0f;
    cls.at<float>(0x40) = 0.0f;
    cls.at<float>(0x4c) = 0.0f;
    cls.at<float>(0x48) = 0.0f;
    cls.at<float>(0x50) = kOne;
    return cls;
}

// 004B1F70, __thiscall(class), RET, EAX = class: 00489E60, byte +28h = 0,
// vtable 00CE72CC, +34h/+38h = 0.
Raw wreck_class_construct_004b1f70(Raw cls, Seg8Host& host) {
    host.named_class_construct_base_00489e60(cls);
    cls.at<std::uint8_t>(0x28) = 0;
    cls.at<std::uint32_t>(0) = 0x00ce72cc;
    cls.at<std::uint32_t>(0x34) = 0;
    cls.at<std::uint32_t>(0x38) = 0;
    return cls;
}

// 004A7C10 (+60h, +5Ch), 004AC990 (+58h, +54h), 004B1F90 (+38h, +34h),
// __thiscall(class), RET.
void plane_part_class_destruct_004a7c10(Raw cls, Seg8Host& host) { destruct_class(cls, 0x00ce6bd4, 0x5c, 0x60, host); }
void ship_part_class_destruct_004ac990(Raw cls, Seg8Host& host) { destruct_class(cls, 0x00ce6f34, 0x54, 0x58, host); }
void wreck_class_destruct_004b1f90(Raw cls, Seg8Host& host) { destruct_class(cls, 0x00ce72cc, 0x34, 0x38, host); }

// 004A7CB0 / 004ACA30, __thiscall(class), RET, EAX = part or null.
void* plane_part_class_create_instance_004a7cb0(Raw cls, Seg8Host& host) {
    return create_part(cls, 0x00ce6ab0, 0x00ce6a98, 0x00ce6a90, 0x3f, host);
}
void* ship_part_class_create_instance_004aca30(Raw cls, Seg8Host& host) {
    return create_part(cls, 0x00ce6e10, 0x00ce6df8, 0x00ce6df0, 0x40, host);
}

// 004A9960: "PlanePartClass" (00CE6C6C), class size 64h, empty name 00E187C0,
// registry 004A9890 / map 004A95F0. The native inlines 004A7BF0's body.
void* plane_part_class_load_from_globals_004a9960(Raw name, Seg8Host& host) {
    return load_from_globals(
        name, "PlanePartClass", 0x00e187c0, 0x64, [&](Raw c) { return plane_part_class_construct_004a7bf0(c, host); },
        [&] { return host.plane_part_registry_004a9890(); },
        [&](Raw map, Raw key) { return host.plane_part_map_subscript_004a95f0(map, key); }, host);
}

// 004AEC00: "ShipPartClass" (00CE6F70), class size 5Ch, empty name 00E187EC,
// registry 004AEB30 / map 004AE7F0, constructor 004AC910.
void* ship_part_class_load_from_globals_004aec00(Raw name, Seg8Host& host) {
    return load_from_globals(
        name, "ShipPartClass", 0x00e187ec, 0x5c, [&](Raw c) { return ship_part_class_construct_004ac910(c, host); },
        [&] { return host.ship_part_registry_004aeb30(); },
        [&](Raw map, Raw key) { return host.ship_part_map_subscript_004ae7f0(map, key); }, host);
}

// 004AE990 / 004B3B50, __thiscall(registry), RET, EAX = registry: vtable
// (00CE6F6C / 00CE734C), head = buy-head-node(registry+4) stored at +8, its
// byte +19h = 1 and its three links point at itself, size +0Ch = 0.
namespace {
Raw construct_registry(Raw registry, void* head) {
    registry.ptr(0x8) = head;
    Raw h{static_cast<std::uint8_t*>(head)};
    h.at<std::uint8_t>(0x19) = 1;
    as_raw(registry.ptr(0x8)).ptr(0x4) = registry.ptr(0x8);
    as_raw(registry.ptr(0x8)).ptr(0x0) = registry.ptr(0x8);
    as_raw(registry.ptr(0x8)).ptr(0x8) = registry.ptr(0x8);
    registry.at<std::uint32_t>(0xc) = 0;
    return registry;
}
}  // namespace

Raw ship_part_class_registry_construct_004ae990(Raw registry, Seg8Host& host) {
    registry.at<std::uint32_t>(0) = 0x00ce6f6c;
    void* const head = host.tree_buy_head_node_004ac2f0(Raw{registry.p + 0x4});
    return construct_registry(registry, head);
}

Raw wreck_class_registry_construct_004b3b50(Raw registry, Seg8Host& host) {
    registry.at<std::uint32_t>(0) = 0x00ce734c;
    void* const head = host.tree_buy_head_node_004b1dd0(Raw{registry.p + 0x4});
    return construct_registry(registry, head);
}

// 004B3BC0, __thiscall(registry), RET: 004B3170 erases [leftmost, head) of
// the tree at +4 (iterators {tree, node}), operator delete (00BF65AC) of the
// head, +8 and +0Ch = 0, [00E187F8] = 0, vtable 00CE3818. The mapped class
// objects are not deleted by this body.
void wreck_class_registry_destruct_004b3bc0(Raw registry, Seg8Host& host) {
    const Raw tree{registry.p + 0x4};
    std::uint32_t result[2] = {0, 0};
    void* const head = registry.ptr(0x8);
    host.tree_erase_range_004b3170(tree, result, as_raw(head).ptr(0x0), tree, head);
    host.operator_delete_00bf65ac(tree.ptr(0x4));
    tree.ptr(0x4) = nullptr;
    tree.at<std::uint32_t>(0x8) = 0;
    singleton_base_destruct(registry, 0x00e187f8, host);
}

// 004AEB30 / 004B3C50, __cdecl, RET: lazy getters (operator new 10h,
// constructors 004AE990 / 004B3B50, 00BD0C30 registers the global itself).
void* ship_part_class_registry_get_004aeb30(void*& global_00e187e8, cc13_sys::LazySingletonHost& host) {
    static const cc13_sys::LazySingletonGetterRow row{0x004aeb30, 0x00e187e8, 0x10, 0x004ae990, 0, 0, 0, 0};
    return cc13_sys::lazy_singleton_get(row, global_00e187e8, host);
}
void* wreck_class_registry_get_004b3c50(void*& global_00e187f8, cc13_sys::LazySingletonHost& host) {
    static const cc13_sys::LazySingletonGetterRow row{0x004b3c50, 0x00e187f8, 0x10, 0x004b3b50, 0, 0, 0, 0};
    return cc13_sys::lazy_singleton_get(row, global_00e187f8, host);
}

// 004A83C0 / 004AD150 / 004B2820, __thiscall(string), RET.
void native_string_release_storage_004a83c0(Raw string, Seg8Host& host) { release_pooled_string(string, host); }
void native_string_release_storage_004ad150(Raw string, Seg8Host& host) { release_pooled_string(string, host); }
void native_string_release_storage_004b2820(Raw string, Seg8Host& host) { release_pooled_string(string, host); }

// 004B4E00 / 004B4E20 / 004B4EB0 / 004B4ED0, __thiscall(object), RET: the
// singleton global (00E198D0 / 00F8A71C / 00E19A64 / 00F8998C) = 0 and vtable 00CE3818.
void hint_tracker_destruct_singleton_base_004b4e00(Raw object, Seg8Host& host) {
    singleton_base_destruct(object, 0x00e198d0, host);
}
void singleton_00f8a71c_destruct_base_004b4e20(Raw object, Seg8Host& host) {
    singleton_base_destruct(object, 0x00f8a71c, host);
}
void singleton_00e19a64_destruct_base_004b4eb0(Raw object, Seg8Host& host) {
    singleton_base_destruct(object, 0x00e19a64, host);
}
void singleton_00f8998c_destruct_base_004b4ed0(Raw object, Seg8Host& host) {
    singleton_base_destruct(object, 0x00f8998c, host);
}

// 004B4560, __thiscall(platform), RET: AL = byte +180h, then +180h = 0.
bool win32_platform_consume_close_request_004b4560(Raw platform) noexcept {
    const std::uint8_t requested = platform.at<std::uint8_t>(0x180);
    platform.at<std::uint8_t>(0x180) = 0;
    return requested != 0;
}

// 004B4F40, __thiscall(pair), RET: 00951560 on the pair, then a tail jump to
// 00951560 with ECX = pair+54h.
void entity_id_table_pair_reset_004b4f40(Raw pair, Seg8Host& host) {
    host.entity_id_table_reset_00951560(pair);
    host.entity_id_table_reset_00951560(Raw{pair.p + 0x54});
}

// 004B5670 / 004B5680, __thiscall(record, byte), RET 4: byte +8 / +9 = value.
void participant_record_set_claimed_004b5670(Raw record, std::uint8_t value) noexcept {
    record.at<std::uint8_t>(0x8) = value;
}
void participant_record_set_gate_byte9_004b5680(Raw record, std::uint8_t value) noexcept {
    record.at<std::uint8_t>(0x9) = value;
}

// 004B6210, __thiscall(session), RET: when byte +27Dh is set, 0076FAD0(0);
// byte +27Dh = 0 either way.
void session_clear_flag_27d_and_set_local_mode_004b6210(Raw session, Seg8Host& host) {
    if (session.at<std::uint8_t>(0x27d) != 0) host.session_set_mode_0076fad0(session, 0);
    session.at<std::uint8_t>(0x27d) = 0;
}

// 004B79F0, __thiscall(world), RET: EAX = world+13Ch.
void* world_class_list_head_004b79f0(Raw world) noexcept { return world.p + 0x13c; }

// 004B81C0 / 004B83B0, __thiscall(ref), RET: when *ref is set, 00B6DFA0 on it
// and *ref = 0.
void node_ref_unlink_and_clear_004b81c0(Raw ref, Seg8Host& host) {
    if (ref.ptr(0x0) != nullptr) {
        host.node_unlink_00b6dfa0(ref.ptr(0x0));
        ref.ptr(0x0) = nullptr;
    }
}
void node_ref_unlink_and_clear_004b83b0(Raw ref, Seg8Host& host) { node_ref_unlink_and_clear_004b81c0(ref, host); }

// 004B83D0, __thiscall(ref), RET: the same with 00B72250.
void native_scene_ref_release_and_clear_004b83d0(Raw ref, Seg8Host& host) {
    if (ref.ptr(0x0) != nullptr) {
        host.scene_ref_release_00b72250(ref.ptr(0x0));
        ref.ptr(0x0) = nullptr;
    }
}

// 004B8470, __thiscall(ref), RET, register input EDX: when *ref is set,
// 00926D90 (ECX = *ref) with the caller's EDX pushed as its stack argument,
// then *ref = 0. The EDX value is whatever the caller left there.
void mission_entity_ref_kill_and_clear_004b8470(Raw ref, std::uint32_t edx_in, Seg8Host& host) {
    if (ref.ptr(0x0) != nullptr) {
        host.mission_entity_kill_00926d90(ref.ptr(0x0), edx_in);
        ref.ptr(0x0) = nullptr;
    }
}

}  // namespace bsp::classes_seg8_cc13
