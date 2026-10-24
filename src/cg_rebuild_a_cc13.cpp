// Packet cc13-cgbuildA: see include/bsp/cg_rebuild_a_cc13.hpp. Build-tested only;
// nothing here is wired into a game host.
#include "bsp/cg_rebuild_a_cc13.hpp"

namespace bsp::cg_rebuild_a_cc13 {
namespace {

constexpr std::uint32_t kLuaOwner = 0x00e188a8;    // [00E188A8] + 1A0Ch = the Lua state owner
constexpr std::uint32_t kGroupMargin = 0x00ce38b8; // 10.0f
constexpr float kBuoyLayerRange = 100.0f;          // [00CE3D08]

// Element constructors / destructors the EH vector iterators receive.
constexpr std::uint32_t kListConstruct_0048ce70 = 0x0048ce70;   // STL_List_Construct
constexpr std::uint32_t kListTidyThunk_00488430 = 0x00488430;   // thunk to STL_List_Tidy 00485430
constexpr std::uint32_t kCtor_00495420 = 0x00495420;
constexpr std::uint32_t kCtor_00495430 = 0x00495430;
constexpr std::uint32_t kCtor_00499280 = 0x00499280;
constexpr std::uint32_t kVectorTidy_0048cec0 = 0x0048cec0;
constexpr std::uint32_t kVectorTidy_0048f1d0 = 0x0048f1d0;
constexpr std::uint32_t kNamedBufferClear_0048e0a0 = 0x0048e0a0;
constexpr std::uint32_t kVectorFreeBuffer_0040ab90 = 0x0040ab90;
constexpr std::uint32_t kPathPlanDestruct_009dbdb0 = 0x009dbdb0;
constexpr std::uint32_t kElement_009560f0 = 0x009560f0;

// The inlined native-string release: if data, return size+1 bytes to the pool
// (00419CC0 then 00BD1510(data, size + 1, 1)).
void release_string(CgRebuildAHost& host, const NativeString& string) {
    if (string.data != nullptr) {
        Object pool = host.string_pool_00419cc0();
        host.string_pool_return_00bd1510(pool, string.data, string.size + 1, 1);
    }
}

// The inlined NativeString(const char*): size/data zeroed, 0041DD40(length, 1),
// then memcpy(data, text, size + 1) when data is set.
void assign_string(CgRebuildAHost& host, NativeString& string, const char* text, std::uint32_t length) {
    string.size = 0;
    string.data = nullptr;
    host.native_string_resize_0041dd40(&string, length, 1);
    if (string.data != nullptr) {
        host.crt_memcpy_00bf7680(string.data, text, string.size + 1);
    }
}

std::uint32_t text_length(const char* text) {
    const char* end = text;
    while (*end != '\0') ++end;
    return static_cast<std::uint32_t>(end - text);
}

// The inlined pointer-array push_back on {data +0, size +4, capacity +8}: when
// size == capacity, reserve max(capacity * 2, 1) (00482570); store when the slot
// address is non-null; size + 1.
void push_pointer(CgRebuildAHost& host, Object array, Object value) {
    const std::int32_t capacity = at<std::int32_t>(array, 8);
    if (at<std::int32_t>(array, 4) == capacity) {
        std::int32_t grown = capacity + capacity;
        if (grown <= 1) grown = 1;
        host.pod_pointer_array_reserve_00482570(array, grown);
    }
    Object* slot = at<Object*>(array, 0) + at<std::int32_t>(array, 4);
    if (slot != nullptr) *slot = value;
    at<std::int32_t>(array, 4) += 1;
}

// The inlined std::vector tidy with an element destroyer, on a 10h-byte vector
// {proxy +0, first +4, last +8, end +0Ch}: when first is set, destroy
// [first, last) (ECX/EDX, stack vector, owner), free first (00BF65AC); then the
// three pointers are zeroed.
template <class Destroy>
void tidy_vector(CgRebuildAHost& host, Object vector, Object owner, Destroy destroy) {
    if (at<Object>(vector, 4) != nullptr) {
        destroy(at<Object>(vector, 4), at<Object>(vector, 8), vector, owner);
        host.crt_free_00bf65ac(at<Object>(vector, 4));
    }
    at<Object>(vector, 4) = nullptr;
    at<Object>(vector, 8) = nullptr;
    at<Object>(vector, 0x0C) = nullptr;
}

// The inlined POD std::vector tidy at first/last/end: free first when set
// (00BF65AC), zero the three.
void tidy_pod_vector(CgRebuildAHost& host, Object first_field) {
    if (at<Object>(first_field, 0) != nullptr) host.crt_free_00bf65ac(at<Object>(first_field, 0));
    at<Object>(first_field, 0) = nullptr;
    at<Object>(first_field, 4) = nullptr;
    at<Object>(first_field, 8) = nullptr;
}

// The inlined intrusive-reference release of the pointer at field: when set,
// InterlockedDecrement(&p->count +4h); at 0 call [[p]+0]() (ECX = p, no
// argument); then the field is zeroed. A null field is left untouched.
void release_ref(CgRebuildAHost& host, Object owner, std::size_t field) {
    Object p = at<Object>(owner, field);
    if (p == nullptr) return;
    if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(p, 4)) == 0) host.vslot0_release(p);
    at<Object>(owner, field) = nullptr;
}

// The inlined counted-array teardown on {data +0, count +4, capacity +8}: when
// capacity < 0 the reserve routine runs with 0; count is decremented to 0 one
// step at a time (only while positive), then stored 0; data is freed through
// 00BF6989 (null or not; data itself is not cleared).
template <class Reserve>
void destroy_counted_array(CgRebuildAHost& host, Object array, Reserve reserve) {
    if (at<std::int32_t>(array, 8) < 0) reserve(array, 0);
    if (at<std::int32_t>(array, 4) > 0) {
        do {
            at<std::int32_t>(array, 4) += -1;
        } while (at<std::int32_t>(array, 4) > 0);
    }
    at<std::int32_t>(array, 4) = 0;
    host.crt_free_00bf6989(at<Object>(array, 0));
}

}  // namespace

// ---------------------------------------------------------------------------
// 004A0410 Unknown_VtCE66E0_Construct (PROVISIONAL): the landing-troop group
// constructor (E4h bytes from 004A4520 operator_new). 589 instructions.
// ABI: __thiscall(ECX = group; stack parent, pad, base, ship), RET 10h, EAX = group.
// Stack args land as +D8h = pad (2nd), +E0h = base (3rd), +DCh = ship (4th)
// (004A0493..004A04B4); left8 called the 3rd argument a3, soldier_traffic_cc13
// names +E0h the CommandBuilding.
// Order:
//   00496920(group, parent) scene node base; vtable 00CE66E0; pointer array
//   +48h {data, size, capacity} zeroed; +98h = [00CE38B8] (10.0f, AABB margin);
//   eh vector ctor(+B4h, 0Ch, 3, 0048CE70, 00488430) (three member lists);
//   +DCh/+E0h/+D8h; +44h = [[ship+538h]+814h] (class index).
//   LandingTroopClass[+44h] (globals [00E188A8]+1A0Ch -> "LandingTroopClass" ->
//   by index), the two temporaries destructed (table first, then globals);
//   eight numbers by name into +24h..+40h (Speed, MaxDeflection, Distance,
//   RunMinTime, RunMaxTime, FireMinTime, FireMaxTime, DieChance): each FSTP
//   float of 00B66270's ST0 result.
//   "Classes" table; iterate (00B67080 / 00B67190 with key and row, stop when
//   00B66420 says the row is unbound). Per row: "Type" string copied into a
//   native string. Empty data (nullptr) skips the row. _stricmp "Soldier":
//   operator_new(7Ch); if non-null: "ClassName" string -> native string,
//   004B1400(&name), 0049FC80(memory, class, row). _stricmp "Vehicle":
//   operator_new(3Ch); if non-null: "ClassId" integer, 00964790(id, DL = 1),
//   0047B3C0, 0048DC40(memory, visual, row), then vtable 00CE6794 and
//   byte +38h = 0. Either kind (null on allocation failure) is pushed to +48h.
//   Any other type is ignored. Temporaries are released in the image's order.
//   After the loop: entity = 007ACA30(pad[0] ? pad[0]+1E4h : 0); +54h = the
//   entity's pose [entity+3Ch]; refresh the pose world matrix when +C8h is clear
//   (00414DB0 on the parent, 00413920 local x parent world or the local +74h,
//   004134F0 into +CCh, +C8h = 1, +10Ch = 0); copy the 16 world floats into
//   +58h..+94h (FLD/FSTP float pairs: bit copies except that an SNaN would be
//   quieted). AABB max +A8h = 007AF800(pad[0]+1E4h, 0) (no null test here),
//   min +9Ch = max; for each of the four pad slots that is set, include points
//   1 .. count-1 (count = (+1F0h - +1ECh) >> 2, or 0 when +1ECh is null,
//   re-read each step) with 00427D10. min -= +98h, max += +98h (float x87
//   ops on float operands, correctly rounded at the FSTP), byte +20h = 0.
//   Destruct row, key, Classes, the class row; return group.
// EH: state 0 after the base, 1 after the array, 2 after the lists, 3..7 the
//   Lua temporaries, 8..0Fh each number read, 10h..12h Classes/key/row,
//   13h/15h the Type string, 16h..1Dh the new'd member, then 11h/10h/7/2 on
//   the way out. EBX bits 1/2/4 mark live ClassName temp / name string / ClassId
//   temp for the funclets. Unwinding is not modelled.
// Uncertainty: the role of the 3rd stack argument (+E0h); the iterator's
//   key/row naming; 00964790 receives DL = 1 with EDX's upper bytes left over
//   from an address (modelled as the byte flag).
// ---------------------------------------------------------------------------
Object landing_troop_group_construct_004a0410(CgRebuildAHost& host, Object group, Object parent, Object pad,
                                              Object base, Object ship) {
    host.scene_node_construct_00496920(group, parent);
    Object members = group + 0x48;
    at<std::uint32_t>(group, 0) = 0x00ce66e0;
    at<std::uint32_t>(members, 0) = 0;
    at<std::uint32_t>(members, 4) = 0;
    at<std::uint32_t>(members, 8) = 0;
    at<std::uint32_t>(group, 0x98) = host.global_u32(kGroupMargin);
    host.eh_vector_construct_00bf7cd1(group + 0xB4, 0x0C, 3, kListConstruct_0048ce70, kListTidyThunk_00488430);
    at<Object>(group, 0xDC) = ship;
    at<Object>(group, 0xE0) = base;
    at<Object>(group, 0xD8) = pad;
    at<std::uint32_t>(group, 0x44) = at<std::uint32_t>(at<Object>(ship, 0x538), 0x814);

    LuaObject globals;     // [ESP+B4h]
    LuaObject class_table; // [ESP+A0h]
    LuaObject troop_class; // [ESP+38h]
    LuaObject temp;        // [ESP+18h]
    LuaObject* g = host.lua_get_globals_00b67980(obj(host.global_u32(kLuaOwner)) + 0x1A0C, &globals);
    LuaObject* t = host.lua_get_by_name_00b67800(g, &class_table, "LandingTroopClass");
    host.lua_get_by_index_00b67720(t, &troop_class, at<std::int32_t>(group, 0x44));
    host.lua_object_destruct_00b67700(&class_table);
    host.lua_object_destruct_00b67700(&globals);

    static constexpr struct {
        const char* name;
        std::size_t offset;
    } kNumbers[] = {
        {"Speed", 0x24},       {"MaxDeflection", 0x28}, {"Distance", 0x2C},    {"RunMinTime", 0x30},
        {"RunMaxTime", 0x34},  {"FireMinTime", 0x38},   {"FireMaxTime", 0x3C}, {"DieChance", 0x40},
    };
    for (const auto& number : kNumbers) {
        LuaObject* value = host.lua_get_by_name_00b67800(&troop_class, &temp, number.name);
        at<float>(group, number.offset) = static_cast<float>(host.lua_get_number_00b66270(value));
        host.lua_object_destruct_00b67700(&temp);
    }

    LuaObject classes;   // [ESP+60h]
    LuaObject key;       // [ESP+78h]
    LuaObject row;       // [ESP+4Ch]
    host.lua_get_by_name_00b67800(&troop_class, &classes, "Classes");
    host.lua_object_construct_00b65f50(&key);
    host.lua_object_construct_00b65f50(&row);
    host.lua_iterate_first_00b67080(&classes, &key, &row);
    while (!host.lua_is_unbound_00b66420(&classes, &row)) {
        LuaObject type_temp;   // [ESP+C8h]
        NativeString type;     // [ESP+18h]
        const char* type_text = host.lua_get_string_00b662b0(host.lua_get_by_name_00b67800(&row, &type_temp, "Type"));
        assign_string(host, type, type_text, text_length(type_text));
        host.lua_object_destruct_00b67700(&type_temp);
        if (type.data != nullptr) {
            if (host.crt_stricmp_00bf7fbf(type.data, "Soldier") == 0) {
                Object memory = host.operator_new_00bf681b(0x7C);
                Object soldier = nullptr;
                bool name_temp_live = false;   // EBX bit 1
                bool name_live = false;        // EBX bit 2
                LuaObject name_temp;           // [ESP+8Ch]
                NativeString name;             // [ESP+30h]
                if (memory != nullptr) {
                    LuaObject* value = host.lua_get_by_name_00b67800(&row, &name_temp, "ClassName");
                    name_temp_live = true;
                    const char* name_text = host.lua_get_string_00b662b0(value);
                    assign_string(host, name, name_text, text_length(name_text));
                    name_live = true;
                    Object soldier_class = host.soldier_class_get_or_load_004b1400(&name);
                    soldier = host.soldier_type_construct_0049fc80(memory, soldier_class, &row);
                }
                push_pointer(host, members, soldier);
                if (name_live) release_string(host, name);
                if (name_temp_live) host.lua_object_destruct_00b67700(&name_temp);
            } else if (host.crt_stricmp_00bf7fbf(type.data, "Vehicle") == 0) {
                Object memory = host.operator_new_00bf681b(0x3C);
                bool id_temp_live = false;   // EBX bit 4
                LuaObject id_temp;           // [ESP+8Ch]
                if (memory != nullptr) {
                    LuaObject* value = host.lua_get_by_name_00b67800(&row, &id_temp, "ClassId");
                    id_temp_live = true;
                    const std::int32_t class_id = host.lua_get_integer_00b66290(value);
                    Object vehicle_class = host.vehicle_class_get_or_create_00964790(class_id, 1);
                    Object visual = host.vehicle_visual_class_get_or_create_0047b3c0(vehicle_class);
                    host.soldier_type_construct_from_lua_0048dc40(memory, visual, &row);
                    at<std::uint32_t>(memory, 0) = 0x00ce6794;
                    at<std::uint8_t>(memory, 0x38) = 0;
                }
                push_pointer(host, members, memory);
                if (id_temp_live) host.lua_object_destruct_00b67700(&id_temp);
            }
        }
        release_string(host, type);
        host.lua_iterate_next_00b67190(&classes, &key, &row);
    }

    Object first_slot = at<Object>(pad, 0);
    Object entity = host.path_object_get_entity_007aca30(first_slot != nullptr ? first_slot + 0x1E4 : nullptr);
    at<Object>(group, 0x54) = at<Object>(entity, 0x3C);
    Object pose = at<Object>(entity, 0x3C);
    const float* world = nullptr;
    if (at<std::uint8_t>(pose, 0xC8) != 0) {
        world = reinterpret_cast<const float*>(pose + 0xCC);
    } else {
        if (at<Object>(pose, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(at<Object>(pose, 0x3C));
        const float* source = nullptr;
        float product[16];   // [ESP+DCh]
        if (at<Object>(pose, 0x3C) != nullptr) {
            source = host.matrix_multiply_00413920(reinterpret_cast<const float*>(pose + 0x74), product,
                                                   reinterpret_cast<const float*>(at<Object>(pose, 0x3C) + 0xCC));
        } else {
            source = reinterpret_cast<const float*>(pose + 0x74);
        }
        host.matrix_copy_004134f0(reinterpret_cast<float*>(pose + 0xCC), source);
        at<std::uint8_t>(pose, 0xC8) = 1;
        at<std::uint8_t>(pose, 0x10C) = 0;
        world = reinterpret_cast<const float*>(pose + 0xCC);
    }
    for (std::size_t i = 0; i < 16; ++i) at<float>(group, 0x58 + i * 4) = world[i];

    float point[3];   // [ESP+18h]
    const float* first = host.scene_path_point_to_world_007af800(at<Object>(pad, 0) + 0x1E4, point, 0);
    at<float>(group, 0xA8) = first[0];
    at<float>(group, 0xAC) = first[1];
    at<float>(group, 0xB0) = first[2];
    float* aabb = &at<float>(group, 0x9C);
    aabb[0] = at<float>(group, 0xA8);
    aabb[1] = at<float>(group, 0xAC);
    aabb[2] = at<float>(group, 0xB0);
    for (std::size_t slot = 0; slot < 0x10; slot += 4) {
        if (at<Object>(pad, slot) == nullptr) continue;
        for (std::int32_t index = 1;; ++index) {
            Object path_owner = at<Object>(pad, slot);
            const std::uint32_t begin = at<std::uint32_t>(path_owner, 0x1EC);
            const std::int32_t count =
                begin == 0 ? 0 : static_cast<std::int32_t>(at<std::uint32_t>(path_owner, 0x1F0) - begin) >> 2;
            if (index >= count) break;
            const float* p = host.scene_path_point_to_world_007af800(path_owner + 0x1E4, point,
                                                                     static_cast<std::uint32_t>(index));
            host.aabb_include_point_00427d10(aabb, p);
        }
    }
    const float margin = at<float>(group, 0x98);
    aabb[0] = aabb[0] - margin;
    aabb[1] = aabb[1] - margin;
    aabb[2] = aabb[2] - margin;
    const float margin_again = at<float>(group, 0x98);
    at<float>(group, 0xA8) = margin_again + at<float>(group, 0xA8);
    at<float>(group, 0xAC) = margin_again + at<float>(group, 0xAC);
    at<float>(group, 0xB0) = margin_again + at<float>(group, 0xB0);
    at<std::uint8_t>(group, 0x20) = 0;

    host.lua_object_destruct_00b67700(&row);
    host.lua_object_destruct_00b67700(&key);
    host.lua_object_destruct_00b67700(&classes);
    host.lua_object_destruct_00b67700(&troop_class);
    return group;
}

// ---------------------------------------------------------------------------
// 0048DFD0 Unknown_VtCE66E0_Destruct (PROVISIONAL): the landing-troop group
// destructor body (scalar deleting wrapper 0048EFB0, vtable 00CE66E0 slot 0).
// 54 instructions. ABI: __thiscall(ECX = group), RET.
// Order: vtable 00CE66E0; for i < [+4Ch] (signed) each non-null member
// [[+48h]+i*4] gets [[m]+0](1); 0048C090(group) kills the member entities;
// eh vector dtor(+B4h, 0Ch, 3, 00488430) (lists, last to first inside the
// iterator); 004859E0(+48h, 0); free [+48h] (00BF6989; the pointer is not
// cleared); 00489DD0(group) base.
// EH: state 2 over the member loop/0048C090, 1 over the lists, 0 over the
// array, -1 before the base. Not modelled.
// ---------------------------------------------------------------------------
void landing_troop_group_destruct_0048dfd0(CgRebuildAHost& host, Object group) {
    at<std::uint32_t>(group, 0) = 0x00ce66e0;
    for (std::int32_t i = 0; i < at<std::int32_t>(group, 0x4C); ++i) {
        Object member = at<Object*>(group, 0x48)[i];
        if (member != nullptr) host.vslot0_delete(member, 1);
    }
    host.landing_troop_kill_all_members_0048c090(group);
    host.eh_vector_destruct_00bf7c6e(group + 0xB4, 0x0C, 3, kListTidyThunk_00488430);
    host.pod_pointer_array_resize_004859e0(group + 0x48, 0);
    host.crt_free_00bf6989(at<Object>(group, 0x48));
    host.list_node_destruct_00489dd0(group);
}

// ---------------------------------------------------------------------------
// 004A41C0 Unknown_VtCE67AC_Construct (PROVISIONAL): the buoy layer scene node
// constructor (vtable 00CE67AC, whose slot 1 is the layered water tick 00491AC0).
// 152 instructions; no direct caller in the image (reached through a pointer).
// ABI: __thiscall(ECX = node; stack parent), RET 4, EAX = node.
// Order: 00496920(node, parent); vtable 00CE67AC; eh vector ctors
// (+18Ch, 10h, 4, 00495420, 0048CEC0), (+1CCh, 10h, 4, 00495430, 0048F1D0),
// (+210h, 14h, 4, 00499280, 0048E0A0); +30h = 100.0f, +34h = 8; +20h, +24h,
// +28h = 00475710(&name) for "OwnBuoy", "EnemyBuoy", "NeutralBuoy" (each a
// temporary native string built with the literal's compile-time length and
// released after the call); +2Ch = 0; 0049E560(node).
// EH: state 0..3 after each part, 4/5/6 over each string temporary. Not modelled.
// Uncertainty: the role of +30h/+34h (a range and a count by their values only).
// ---------------------------------------------------------------------------
Object buoy_layer_node_construct_004a41c0(CgRebuildAHost& host, Object node, Object parent) {
    host.scene_node_construct_00496920(node, parent);
    at<std::uint32_t>(node, 0) = 0x00ce67ac;
    host.eh_vector_construct_00bf7cd1(node + 0x18C, 0x10, 4, kCtor_00495420, kVectorTidy_0048cec0);
    host.eh_vector_construct_00bf7cd1(node + 0x1CC, 0x10, 4, kCtor_00495430, kVectorTidy_0048f1d0);
    host.eh_vector_construct_00bf7cd1(node + 0x210, 0x14, 4, kCtor_00499280, kNamedBufferClear_0048e0a0);
    at<float>(node, 0x30) = kBuoyLayerRange;
    at<std::uint32_t>(node, 0x34) = 8;
    static constexpr struct {
        const char* text;
        std::uint32_t length;
        std::size_t offset;
    } kBuoys[] = {{"OwnBuoy", 7, 0x20}, {"EnemyBuoy", 9, 0x24}, {"NeutralBuoy", 11, 0x28}};
    for (const auto& buoy : kBuoys) {
        NativeString name;   // [ESP+10h]
        assign_string(host, name, buoy.text, buoy.length);
        at<std::uint32_t>(node, buoy.offset) = host.buoy_class_get_or_create_00475710(&name);
        release_string(host, name);
    }
    at<std::uint32_t>(node, 0x2C) = 0;
    host.traffic_layers_rebuild_0049e560(node);
    return node;
}

// ---------------------------------------------------------------------------
// 0048F890 Unknown_VtCE67AC_Destruct (PROVISIONAL): the buoy layer node
// destructor body (scalar deleting wrapper 00491AA0). 41 instructions.
// ABI: __thiscall(ECX = node), RET.
// Order: vtable 00CE67AC; 004814C0(node); eh vector dtors in reverse member
// order: (+210h, 14h, 4, 0048E0A0), (+1CCh, 10h, 4, 0048F1D0),
// (+18Ch, 10h, 4, 0048CEC0); 00489DD0(node) base.
// EH: state 3 over 004814C0, then 2/1/0 before each iterator, -1 before the base.
// ---------------------------------------------------------------------------
void buoy_layer_node_destruct_0048f890(CgRebuildAHost& host, Object node) {
    at<std::uint32_t>(node, 0) = 0x00ce67ac;
    host.scene_entity_layers_reset_all_004814c0(node);
    host.eh_vector_destruct_00bf7c6e(node + 0x210, 0x14, 4, kNamedBufferClear_0048e0a0);
    host.eh_vector_destruct_00bf7c6e(node + 0x1CC, 0x10, 4, kVectorTidy_0048f1d0);
    host.eh_vector_destruct_00bf7c6e(node + 0x18C, 0x10, 4, kVectorTidy_0048cec0);
    host.list_node_destruct_00489dd0(node);
}

// ---------------------------------------------------------------------------
// 00963600 BSP_ShipClass_DestructBase (PROVISIONAL name): the ship class
// destructor body. 387 instructions. ABI: __thiscall(ECX = class), RET.
// Callers: the scalar deleting wrappers 00963B00, 009652B0..00965390, the
// submarine class destructor 00963F90 (call and tail jumps) and five 5-byte
// tail-jump stubs 00963F40..00963F80 / 00964010 Ghidra never defined.
// Destruction order exactly as the image runs it (EH state before each step):
//   vtables: +0 = 00D1ACC4, +6Ch = 00D1ACC0 (second base);           state 1Ch
//   1  vector +7F4h  004CC760 + free + zero                           1Ch
//   2  vector +7E4h  004CC760 + free + zero                           1Bh
//   3  POD    +7D8h  free + zero
//   4  00956150(+7A4h)
//   5  vector +70Ch  0082B5A0 + free + zero                           18h
//   6  009632C0(+6FCh)                                                17h
//   7  POD    +6F0h  free + zero
//   8  00963320(+6DCh)                                                15h
//   9  vector +6CCh  0082C830 + free + zero
//   10 POD    +6C0h  free + zero
//   11 refs   +6B0h, +6ACh, +6A8h, +6A4h                              12h, 11h, 10h, 0Fh
//   12 array  +690h  00468050(0) + free(00BF6989)                     0Eh
//   13 array  +684h  0074D190 when capacity < 0, count to 0, free     0Dh
//   14 array  +678h  00829180 when capacity < 0, count to 0, free     0Ch
//   15 ref    +674h                                                   0Bh
//   16 vector +664h  004CC760 + free + zero                           0Ah
//   17 ref    +65Ch                                                   9
//   18 array  +650h  0074D190 when capacity < 0, count to 0, free     8
//   19 refs   +634h, +630h, +5ACh                                     7, 6, 5
//   20 POD    +588h  free + zero
//   21 ref    +55Ch                                                   3
//   22 POD    +52Ch  free + zero
//   23 ref    +51Ch                                                   1
//   24 eh vector dtor(+138h, 30h, 14h, 009560F0)                      0
//   25 0095F1C0(class) vehicle class base                             -1
// Refs: InterlockedDecrement([p+4]) through 00CE2220 (the first four calls go
// through EBP = [00CE2220], the rest call [00CE2220] directly; same target);
// at zero [[p]+0]() with no argument; the field is zeroed only when p was set.
// EH unwinding is not modelled.
// Uncertainty: member types are named by their helpers only.
// ---------------------------------------------------------------------------
void ship_class_destruct_00963600(CgRebuildAHost& host, Object ship_class) {
    auto range_004cc760 = [&host](Object first, Object last, Object vector, Object owner) {
        host.destroy_range_004cc760(first, last, vector, owner);
    };
    auto reserve_0074d190 = [&host](Object array, std::int32_t capacity) {
        host.points_array_reserve_0074d190(array, capacity);
    };
    at<std::uint32_t>(ship_class, 0) = 0x00d1acc4;
    at<std::uint32_t>(ship_class, 0x6C) = 0x00d1acc0;
    tidy_vector(host, ship_class + 0x7F4, ship_class, range_004cc760);
    tidy_vector(host, ship_class + 0x7E4, ship_class, range_004cc760);
    tidy_pod_vector(host, ship_class + 0x7D8);
    host.stl_tidy_00956150(ship_class + 0x7A4);
    tidy_vector(host, ship_class + 0x70C, ship_class, [&host](Object first, Object last, Object vector, Object owner) {
        host.destroy_range_0082b5a0(first, last, vector, owner);
    });
    host.stl_tidy_009632c0(ship_class + 0x6FC);
    tidy_pod_vector(host, ship_class + 0x6F0);
    host.stl_tidy_00963320(ship_class + 0x6DC);
    tidy_vector(host, ship_class + 0x6CC, ship_class, [&host](Object first, Object last, Object vector, Object owner) {
        host.destroy_range_0082c830(first, last, vector, owner);
    });
    tidy_pod_vector(host, ship_class + 0x6C0);
    release_ref(host, ship_class, 0x6B0);
    release_ref(host, ship_class, 0x6AC);
    release_ref(host, ship_class, 0x6A8);
    release_ref(host, ship_class, 0x6A4);
    host.pod_pointer_array_resize_00468050(ship_class + 0x690, 0);
    host.crt_free_00bf6989(at<Object>(ship_class, 0x690));
    destroy_counted_array(host, ship_class + 0x684, reserve_0074d190);
    destroy_counted_array(host, ship_class + 0x678, [&host](Object array, std::int32_t capacity) {
        host.indexed_points_reserve_00829180(array, capacity);
    });
    release_ref(host, ship_class, 0x674);
    tidy_vector(host, ship_class + 0x664, ship_class, range_004cc760);
    release_ref(host, ship_class, 0x65C);
    destroy_counted_array(host, ship_class + 0x650, reserve_0074d190);
    release_ref(host, ship_class, 0x634);
    release_ref(host, ship_class, 0x630);
    release_ref(host, ship_class, 0x5AC);
    tidy_pod_vector(host, ship_class + 0x588);
    release_ref(host, ship_class, 0x55C);
    tidy_pod_vector(host, ship_class + 0x52C);
    release_ref(host, ship_class, 0x51C);
    host.eh_vector_destruct_00bf7c6e(ship_class + 0x138, 0x30, 0x14, kElement_009560f0);
    host.vehicle_class_destruct_base_0095f1c0(ship_class);
}

// ---------------------------------------------------------------------------
// 004042C0 Unknown_Dtor (PROVISIONAL): the SAP broad-phase manager destructor
// body, reached from 004042A0 (Dyn_SAPBroadPhaseManager_vslot7, the class at
// vtable 00D7A13C). Sibling of the SAPBroadPhaseManager2 body 004043F0
// (native_dyn_sap_lifetime). 79 instructions.
// ABI: __stdcall(manager) (stack argument, ECX unused), RET 4.
// Order: free [+194h] when set (00BF6989; not cleared); for i < [+FCh]
// (unsigned, re-read) free [[+F8h]+i*4] (00BF9DC8), then free [+F8h] when set;
// the same for {+5Ch, +60h} and {+28h, +2Ch}; eh vector dtor(+4, 0Ch, 3,
// 0040AB90); finally the base vtable 00D7A0E4.
// EH: one state (0) over the whole body. Not modelled.
// Uncertainty: the three {data, count} pairs are page lists by shape only.
// ---------------------------------------------------------------------------
void sap_broad_phase_manager_destruct_004042c0(CgRebuildAHost& host, Object manager) {
    if (at<Object>(manager, 0x194) != nullptr) host.crt_free_00bf6989(at<Object>(manager, 0x194));
    static constexpr std::size_t kPageLists[] = {0xF8, 0x5C, 0x28};
    for (const std::size_t list : kPageLists) {
        for (std::uint32_t i = 0; i < at<std::uint32_t>(manager, list + 4); ++i) {
            host.crt_free_00bf9dc8(at<Object*>(manager, list)[i]);
        }
        if (at<Object>(manager, list) != nullptr) host.crt_free_00bf6989(at<Object>(manager, list));
    }
    host.eh_vector_destruct_00bf7c6e(manager + 4, 0x0C, 3, kVectorFreeBuffer_0040ab90);
    at<std::uint32_t>(manager, 0) = 0x00d7a0e4;
}

// ---------------------------------------------------------------------------
// 009E0130 Unknown_Dtor (PROVISIONAL): the ship-AI nav block destructor
// (block = brain+8h; constructor 009E4330, vtable 00D21854 whose 009E46D0 is
// the deleting destructor). 87 instructions.
// ABI: __thiscall(ECX = block), RET (callers 009E46D0, 009E5460, 009F1160).
// Order: vtable 00D21854; for i < [+400h] (signed) each set [+404h+i*4]:
// 0064A610(p+4Ch), 009D5EB0(p+34h), free p (00BF65AC), slot = 0. For i <
// [+604h] each set [+608h+i*4]: [p] = 00CF5C20, 006952A0(ECX = [p+14h],
// EDX = p) when [p+14h] is set, 00695870(p), free p, slot = 0. Then
// 004158A0 on +A7Ch, +A5Ch, +A3Ch; eh vector dtor(+224h, 68h, 2, 009DBDB0).
// EH: state 0 at entry, 4/3 and 5/3 inside the loops, 2/1/0 over the three
// lists, -1 before the iterator. Not modelled.
// ---------------------------------------------------------------------------
void ship_ai_nav_block_destruct_009e0130(CgRebuildAHost& host, Object block) {
    at<std::uint32_t>(block, 0) = 0x00d21854;
    for (std::int32_t i = 0; i < at<std::int32_t>(block, 0x400); ++i) {
        Object& slot = at<Object>(block, 0x404 + static_cast<std::size_t>(i) * 4);
        Object p = slot;
        if (p == nullptr) continue;
        host.observed_owner_destroy_0064a610(p + 0x4C);
        host.path_node_observer_destruct_009d5eb0(p + 0x34);
        host.crt_free_00bf65ac(p);
        slot = nullptr;
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(block, 0x604); ++i) {
        Object& slot = at<Object>(block, 0x608 + static_cast<std::size_t>(i) * 4);
        Object p = slot;
        if (p == nullptr) continue;
        at<std::uint32_t>(p, 0) = 0x00cf5c20;
        if (at<Object>(p, 0x14) != nullptr) host.observer_unregister_pair_006952a0(at<Object>(p, 0x14), p);
        host.callback_owner_destroy_00695870(p);
        host.crt_free_00bf65ac(p);
        slot = nullptr;
    }
    host.avoid_zone_segment_list_clear_004158a0(block + 0xA7C);
    host.avoid_zone_segment_list_clear_004158a0(block + 0xA5C);
    host.avoid_zone_segment_list_clear_004158a0(block + 0xA3C);
    host.eh_vector_destruct_00bf7c6e(block + 0x224, 0x68, 2, kPathPlanDestruct_009dbdb0);
}

}  // namespace bsp::cg_rebuild_a_cc13
