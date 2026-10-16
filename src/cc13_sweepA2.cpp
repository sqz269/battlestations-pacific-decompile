// Packet cc13-sweepA2: segments 0..14 leftovers continued (cc13_sweepA2 marker).
// See include/bsp/cc13_sweepA2.hpp and docs/CC13_SWEEPA2.md. Each body carries
// its address, the original ABI and every simplification next to it.

#include "bsp/cc13_sweepA2.hpp"

#include <cstdlib>
#include <cstring>

namespace bsp::cc13_sweepA2 {

namespace {

constexpr std::uint32_t kNodePool = 0x00e175b0;     // ECX of every 0043B0A0 call
constexpr std::uint32_t kGame = 0x00e188a8;         // [00E188A8] the game object
constexpr std::uint32_t kPlayer = 0x00e188d8;       // [00E188D8] the local player / commander
constexpr std::uint32_t kSoundManager = 0x00f8bbd8; // [00F8BBD8]

// A pointer passed where SweepAHost takes the image's dword argument.
std::uint32_t as_arg(void* pointer) {
    return static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(pointer));
}

// 1e10f, the dword at 00CE4970 (the "no pick yet" distance).
float no_pick_distance() {
    const std::uint32_t bits = 0x501502f9u;
    float value;
    std::memcpy(&value, &bits, sizeof value);
    return value;
}

// The sensor owner the pick loops test with 00804350: the player [00E188D8]
// when its +5Ch(5) holds, else its +3D0h when +5Ch(18h) holds, else null.
void* sensor_owner(SweepA2Host& host) {
    void* const player = host.global_pointer(kPlayer);
    if (host.vcall_entity_is_kind_5c(player, 5)) return player;
    if (host.vcall_entity_is_kind_5c(player, 0x18)) return Raw{static_cast<std::uint8_t*>(player)}.ptr(0x3d0);
    return nullptr;
}

// The count the pick loops require to be at least 2 (signed): with game =
// [00E188A8] and k = [[game + 18CCh + [game+18ECh]*4] + 28h], the dword at
// unit + k*34h + 1E8h + (the byte at +1F8h of that block set ? 8 : 4).
std::int32_t side_count(void* unit, SweepA2Host& host) {
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    const std::int32_t slot = game.at<std::int32_t>(0x18ec);
    const Raw side{static_cast<std::uint8_t*>(game.ptr(0x18cc + 4 * slot))};
    const Raw block{static_cast<std::uint8_t*>(unit) + side.at<std::int32_t>(0x28) * 0x34 + 0x1e8};
    return block.at<std::uint8_t>(0x10) != 0 ? block.at<std::int32_t>(8) : block.at<std::int32_t>(4);
}

}  // namespace

// ============================ segment 11 ===================================

// 004EE300, __thiscall(ECX = scene property bag), RET; the LandConvoy (1Ah)
// class callback, registered by the immediate 004EE300 pushed at 004F29E3
// (004EE250 registration in 004F2700). For i = 1..4: a pooled string sized by
// 0041DD40(4, 1) gets "Type" (00CE4780) copied with memcpy(length + 1) when
// its data is set; 004263B0 forms "Type<i>"; the bag is searched for it
// (008F2260, an empty string 00E18B5D when the data is null); a property of
// kind 6 (+4) yields its sub-bag (+0Ch), anything else null. The numbered
// string's data, then the key's (length and data as they stood after the
// resize), go back to the sized pool. The sub-bag (possibly null, as in the
// image) is searched for "Type"; that value is copy-constructed into a local
// 10h record (004E5B20) and, when its +0Ch is above 0 (signed), the vehicle
// class is created or fetched (00964790(id, 1)); the record is destroyed
// (008F0DE0). SEH states 0..2 cover the temporaries; no unwind is modelled.
void land_convoy_preload_type_classes_004ee300(void* bag, SweepA2Host& host) {
    const char* const type_key = static_cast<const char*>(host.global_address(0x00ce4780));
    for (std::int32_t i = 1; i <= 4; ++i) {
        alignas(4) std::uint8_t key_storage[8] = {};
        const Raw key{key_storage};
        host.native_string_resize_0041dd40(key, 4, 1);
        void* const key_data = key.ptr(4);
        const std::uint32_t key_length = key.at<std::uint32_t>(0);
        if (key_data != nullptr) std::memcpy(key_data, type_key, key_length + 1);
        alignas(4) std::uint8_t numbered_storage[8];
        const Raw numbered{numbered_storage};
        const Raw formed = host.native_string_concat_int_004263b0(key, numbered, i);
        const char* name = static_cast<const char*>(formed.ptr(4));
        if (name == nullptr) name = static_cast<const char*>(host.global_address(0x00e18b5d));
        void* sub_bag = nullptr;
        if (void* const property = host.scene_property_bag_find_008f2260(bag, name); property != nullptr) {
            const Raw p{static_cast<std::uint8_t*>(property)};
            if (p.at<std::int32_t>(4) == 6) sub_bag = p.ptr(0xc);
        }
        if (void* const data = numbered.ptr(4); data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          numbered.at<std::uint32_t>(0) + 1, 1);
        }
        if (key_data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(),
                                                          key_data, key_length + 1, 1);
        }
        alignas(4) std::uint8_t record_storage[0x10];
        const Raw record{record_storage};
        host.scene_property_value_copy_construct_004e5b20(record,
                                                          host.scene_property_bag_find_008f2260(sub_bag, type_key));
        if (record.at<std::int32_t>(0xc) > 0) host.vehicle_class_get_or_create_00964790(record.at<std::int32_t>(0xc), 1);
        host.scene_property_value_destroy_008f0de0(record);
    }
}

// 004F01F0, __thiscall(ECX = bag {+4 count, +8 64 bucket heads}, record),
// RET 4, AL. Walks the 64 buckets in order; the first node (+0 key length,
// +4 key data, +8 owned record, +0Ch next) whose +8 equals `record` is
// unlinked (from the bucket head or the previous node's +0Ch), its record is
// deleted through vtable slot 0 with flags 1 and +8 cleared, the key data is
// returned to the sized pool (00BD1510(00419CC0(), data, length + 1, 1)) when
// set, the node goes back to the 00E175B0 pool (0043B0A0), the count drops by
// one and AL = 1. No match: AL = 0. Callers 004F03C0, 008F4290, 008F4420.
bool scene_property_bag_remove_record_004f01f0(Raw bag, void* record, SweepA2Host& host) {
    for (std::uint32_t bucket = 0; bucket < 0x40; ++bucket) {
        Raw previous{nullptr};
        for (Raw node{static_cast<std::uint8_t*>(bag.ptr(8 + 4 * bucket))}; node.p != nullptr;
             node = Raw{static_cast<std::uint8_t*>(node.ptr(0xc))}) {
            if (node.ptr(8) != record) {
                previous = node;
                continue;
            }
            if (previous.p == nullptr) {
                bag.ptr(8 + 4 * bucket) = node.ptr(0xc);
            } else {
                previous.ptr(0xc) = node.ptr(0xc);
            }
            if (void* const value = node.ptr(8); value != nullptr) {
                host.vcall_scalar_deleting_destructor_slot0(value, 1);
                node.ptr(8) = nullptr;
            }
            if (void* const data = node.ptr(4); data != nullptr) {
                const std::uint32_t size = node.at<std::uint32_t>(0) + 1;
                host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(),
                                                              data, size, 1);
            }
            host.node_pool_return_0043b0a0(kNodePool, node.p);
            bag.at<std::int32_t>(4) -= 1;
            return true;
        }
    }
    return false;
}

// ============================ segment 3 ====================================

namespace {

// The intrusive release the bodies inline: InterlockedDecrement(object+4)
// and, on zero, vtable slot 0 with no stack argument.
void release_counted(void* object, SweepA2Host& host) {
    if (host.interlocked_decrement_ce2220(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(object) + 4)) ==
        0) {
        host.vcall_release_slot0(object);
    }
}

}  // namespace

// 0043ABA0 BSP_HudDigitIcon_RollUv, __fastcall(ECX = digit index, EDX =
// widget, float magnitude), RET 4; one caller, 0043B2F0. A 24-byte frame
// buffer filled with '0' gets _itoa(__ftol(magnitude * 100.0), buf + 8, 10)
// (00BF7420 truncates; the product of a float and 100.0 is exact in double).
// With L = strlen(buf): f = buf[L - 3 - index] - '0' (signed char, CVTSI2SS).
// When index <= 0, or every buf[L - 2 - index .. L - 3] is '9' (the digits
// below this one roll over), f = (float)(atol(buf[L-2..L-1]) / 100.0 + f).
// Then f = (float)(10.0 - f) (FSUBR double 00CE3DC0). The icon record
// (vtable +9Ch(0)) gives u0, v0, u1, v1 and h = (float)(v1 - v0); the stream
// (vtable +A0h, read before the record) is locked (+10h(0, 0, 0)) and its
// four vertices (base +28h, offset +8, stride +0Ch) get
//   v0': (u0, a), v1': (u1, a), v3': (u1, b), v2': (u0, b), in that order,
// a = (float)(v0 + h*f*(1/12)) and b = (float)(v0 + h*(f + 1)*(1/12)) with
// 1/12 the double at 00CE42C8; the product order is (h*f)*k then + v0, and
// (h*(f+1))*k + v0, where the DC CA FMUL ST2,ST0 at 0043ACF5 scales the h*f
// slot (checked with tools/x87_dc_form_scan.py). The stream is unlocked
// (+14h). The x87 extended intermediates are modelled in double; only the
// k products and the 1/100 division can round differently in the last bit.
void hud_digit_icon_roll_uv_0043aba0(std::int32_t digit_index, void* widget, float magnitude, SweepA2Host& host) {
    char buffer[25];
    std::memset(buffer, '0', 24);
    buffer[24] = '\0';
    const double scaled = static_cast<double>(magnitude) * 100.0;
    host.crt_itoa_00bf9010(static_cast<std::int32_t>(static_cast<std::int64_t>(scaled)), buffer + 8, 10);
    const std::int32_t length = static_cast<std::int32_t>(std::strlen(buffer));
    const char* const tail = buffer + length - 8;  // ECX at 0043ABFB
    float f = static_cast<float>(static_cast<std::int32_t>(static_cast<signed char>(tail[5 - digit_index])) - 0x30);
    bool roll = true;
    for (std::int32_t j = 6 - digit_index; j < 6; ++j) {
        if (tail[j] != '9') roll = false;
    }
    if (roll) {
        char fraction[3] = {tail[6], tail[7], '\0'};
        const std::int32_t hundredths = static_cast<std::int32_t>(std::atol(fraction));
        f = static_cast<float>(static_cast<double>(hundredths) / 100.0 + static_cast<double>(f));
    }
    f = static_cast<float>(10.0 - static_cast<double>(f));
    void* const stream_object = host.vcall_widget_vertex_stream_a0(widget);
    const Raw record{static_cast<std::uint8_t*>(host.vcall_widget_icon_record_9c(widget, 0))};
    const float u0 = record.at<float>(0xc);
    const float v0 = record.at<float>(0x10);
    const float u1 = record.at<float>(0x14);
    const float v1 = record.at<float>(0x18);
    const float h = static_cast<float>(static_cast<double>(v1) - static_cast<double>(v0));
    host.vcall_vertex_stream_lock_10(stream_object, 0, 0, 0);
    const Raw stream{static_cast<std::uint8_t*>(stream_object)};
    double k;
    {
        const std::uint64_t bits = 0x3fb5555555555555ull;  // 00CE42C8
        std::memcpy(&k, &bits, sizeof k);
    }
    auto vertex = [&](std::int32_t index) {
        return reinterpret_cast<float*>(static_cast<std::uint8_t*>(stream.ptr(0x28)) + stream.at<std::int32_t>(8) +
                                        index * stream.at<std::int32_t>(0xc));
    };
    const float a = static_cast<float>(static_cast<double>(h) * static_cast<double>(f) * k + static_cast<double>(v0));
    vertex(0)[0] = u0;
    vertex(0)[1] = a;
    vertex(1)[1] = a;
    vertex(1)[0] = u1;
    const float b =
        static_cast<float>(static_cast<double>(h) * (static_cast<double>(f) + 1.0) * k + static_cast<double>(v0));
    vertex(3)[0] = u1;
    vertex(3)[1] = b;
    vertex(2)[0] = u0;
    vertex(2)[1] = b;
    host.vcall_vertex_stream_unlock_14(stream_object);
}

// 0043D8F0 BSP_LuaTable_CopyIntoPropertyBag, __fastcall(ECX = LuaObject*
// table, EDX = bag), RET; SEH handler 00C5F63B. Callers 00944210, 0049D4C0
// and itself. Key and value LuaObjects are nil-constructed (00B65F50); the
// table is walked with 00B67080 / 00B67190 until 00B66420 reports the key
// unbound. Per entry: name = key's GetString (00B662B0); the bag's entry of
// that name is erased (0043D870, reconstructed by sweepA); then by the
// value's type, tested in this order: table -> a 114h block from operator new
// (00BF681B) is constructed as a bag (008F41A0(0), null when the allocation
// failed), added (008F3B60) and filled by recursion; integer -> 008F3710;
// boolean -> 008F3940 with GetBoolean's EAX; number -> 008F3770 with
// GetNumber rounded to float; string -> 008F38A0; anything else is skipped.
// Both objects are destroyed (00B67700) at the end. SEH states 1/2 cover the
// objects and the allocation; no unwind is modelled.
void lua_table_copy_into_property_bag_0043d8f0(LuaObjectStorage* table, void* bag, SweepA2Host& host) {
    LuaObjectStorage key;
    host.lua_object_construct_00b65f50(&key);
    LuaObjectStorage value;
    host.lua_object_construct_00b65f50(&value);
    host.lua_object_iterate_first_00b67080(table, &key, &value);
    while (!host.lua_object_is_unbound_00b66420(table, &key)) {
        const char* const name = host.lua_object_get_string_00b662b0(&key);
        cc13_sweepA::property_map_erase_by_name_0043d870(Raw{static_cast<std::uint8_t*>(bag)}, name, host);
        if (host.lua_object_is_table_00b661b0(&value)) {
            void* const block = host.operator_new_00bf681b(0x114);
            void* const sub_bag = block != nullptr ? host.scene_property_bag_construct_008f41a0(block, 0) : nullptr;
            host.scene_property_bag_add_sub_bag_008f3b60(bag, name, sub_bag);
            lua_table_copy_into_property_bag_0043d8f0(&value, sub_bag, host);
        } else if (host.lua_object_is_integer_00b66a60(&value)) {
            host.scene_property_bag_add_int_008f3710(bag, name, host.lua_object_get_integer_00b66290(&value));
        } else if (host.lua_object_is_boolean_00b66000(&value)) {
            host.scene_property_bag_add_bool_008f3940(bag, name, host.lua_object_get_boolean_00b66250(&value));
        } else if (host.lua_object_is_number_00b66050(&value)) {
            host.scene_property_bag_add_float_008f3770(bag, name,
                                                       static_cast<float>(host.lua_object_get_number_00b66270(&value)));
        } else if (host.lua_object_is_string_00b660a0(&value)) {
            host.scene_property_bag_add_string_008f38a0(bag, name, host.lua_object_get_string_00b662b0(&value));
        }
        host.lua_object_iterate_next_00b67190(table, &key, &value);
    }
    host.lua_object_destruct_00b67700(&value);
    host.lua_object_destruct_00b67700(&key);
}

// 00440AC0, __thiscall(ECX = class, LuaObject* table), RET 4; the vtable slot
// at 00CE4488 plus one direct CALL from 007327B0 (BSP_GunClass_ReadLuaFields,
// the derived reader calling its base). Runs the base reader 0087CA80(table), reads
// "ExplosionEfx" (00CE44AC) with 00B67800. Nil: the argument slot is reused as
// a null handle (state 1). Otherwise 00870CD0(&temp, 00B66290(field), 1)
// (state 2). The counted pointer at +70h takes the chosen handle when it
// differs: the new one is retained (InterlockedIncrement +4) before the old one
// is released. Then the temp (state bit 2) and the null slot (bit 1; it can
// never be non-null) are released and the field is destroyed (00B67700).
void gun_class_read_lua_explosion_efx_00440ac0(Raw cls, LuaObjectStorage* table, SweepA2Host& host) {
    host.damageable_class_read_lua_fields_0087ca80(cls, table);
    LuaObjectStorage field;
    host.lua_object_get_by_name_00b67800(table, &field, static_cast<const char*>(host.global_address(0x00ce44ac)));
    std::uint32_t state = 0;
    void* slot = nullptr;  // the reused argument slot [ESP+3Ch]
    void* temp = nullptr;  // [ESP+14h]
    void** source;
    if (host.lua_object_is_nil_00b65fb0(&field)) {
        slot = nullptr;
        source = &slot;
        state = 1;
    } else {
        const std::int32_t id = host.lua_object_get_integer_00b66290(&field);
        source = host.effect_handle_acquire_00870cd0(&temp, id, 1);
        state = 2;
    }
    void* const incoming = *source;
    void* const old = cls.ptr(0x70);
    if (old != incoming) {
        cls.ptr(0x70) = incoming;
        if (incoming != nullptr) {
            host.interlocked_increment_ce221c(
                reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(incoming) + 4));
        }
        if (old != nullptr) release_counted(old, host);
    }
    if ((state & 2) != 0) {
        state &= ~2u;
        if (temp != nullptr) {
            release_counted(temp, host);
            temp = nullptr;
        }
    }
    if ((state & 1) != 0 && slot != nullptr) release_counted(slot, host);
    host.lua_object_destruct_00b67700(&field);
}

// 00440F50 BSP_DeviceEntity_OnKilledLua, __thiscall(ECX = device entity),
// RET; SEH handler 00C5F840; the class-1Eh vtable slot (one direct caller,
// 00729FA0). When +70h is 1: the Lua event "destroyed" (00CE44D0, a
// NativeString released after) goes through vtable +194h(&event, 0); the
// owner unit +3Ch, when kind 1Bh with byte [owner+538h]+175h clear, runs
// vtable +1A8h; then, when the owner's bytes +5Ch is set and +5Dh, +60h,
// +5Eh are clear, the class effect handle [[+354h]+70h] is retained into a
// local; when it is set: the point is [element 0 of the vector +344h] + 0Ch
// (0043F8E0, when the vector has a first and a non-zero size) or else the
// global 00F87574; the pose is refreshed when +C8h is clear (00414DB0); the
// point goes through 00439820(world +CCh, null) and then 004142E0 with the
// inverse world matrix (00B6E0D0) of the node [[[+360h]+160h]+0Ch]; a
// by-value copy of the handle (retained) and that point create a point
// effect under the same node (008689C0(..., 1, 0, 0)); the result's held
// effect gets byte +9 = 1 after the result handle is released; the local
// handle is released. Always, last: 00878990 (the peer OnKilled). The
// "TEST EAX,00E17BF8" at 0044104D is the handle's bool conversion
// (-(handle != 0) & 00E17BF8). SEH states 0/1 are not modelled.
void device_entity_on_killed_lua_00440f50(Raw entity, SweepA2Host& host) {
    if (entity.at<std::int32_t>(0x70) == 1) {
        alignas(4) std::uint8_t event_storage[8];
        const Raw event{event_storage};
        host.native_string_construct_0041e870(event, static_cast<const char*>(host.global_address(0x00ce44d0)));
        host.vcall_entity_lua_event_194(entity.p, event, 0);
        if (void* const data = event.ptr(4); data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          event.at<std::uint32_t>(0) + 1, 1);
        }
        void* const owner = entity.ptr(0x3c);
        if (host.vcall_entity_is_kind_5c(owner, 0x1b)) {
            const Raw params{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(owner)}.ptr(0x538))};
            if (params.at<std::uint8_t>(0x175) == 0) host.vcall_unit_part_destroyed_1a8(owner);
        }
        const Raw unit{static_cast<std::uint8_t*>(entity.ptr(0x3c))};
        if (unit.at<std::uint8_t>(0x5c) != 0 && unit.at<std::uint8_t>(0x5d) == 0 && unit.at<std::uint8_t>(0x60) == 0 &&
            unit.at<std::uint8_t>(0x5e) == 0) {
            void* const handle = Raw{static_cast<std::uint8_t*>(entity.ptr(0x354))}.ptr(0x70);
            if (handle != nullptr) {
                host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(handle) + 4));
                const Raw points{entity.field(0x344)};
                const float* point;
                const auto* const first = static_cast<void* const*>(points.ptr(4));
                if (first != nullptr && (static_cast<void* const*>(points.ptr(8)) - first) != 0) {
                    point = reinterpret_cast<const float*>(
                        static_cast<std::uint8_t*>(*host.pointer_vector_at_0043f8e0(points, 0)) + 0xc);
                } else {
                    point = static_cast<const float*>(host.global_address(0x00f87574));
                }
                if (entity.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(entity.p);
                float world[6];  // frame ESP+18h..ESP+2Fh: world point at +0, local result at +0Ch
                host.vector3_transform_project_point_00439820(world, point,
                                                              reinterpret_cast<const float*>(entity.field(0xcc)),
                                                              nullptr);
                auto node = [&] {
                    const Raw holder{static_cast<std::uint8_t*>(entity.ptr(0x360))};
                    return Raw{static_cast<std::uint8_t*>(holder.ptr(0x160))}.ptr(0xc);
                };
                const float* const inverse = host.node_get_inverse_world_matrix_00b6e0d0(node());
                float* const local = host.vector3_transform_affine_point_004142e0(world, world + 3, inverse);
                void* const parent = node();
                host.interlocked_increment_ce221c(reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(handle) + 4));
                void* created = nullptr;
                void* const effect = *host.point_effect_create_008689c0(&created, parent, handle, local, 1, 0, 0);
                if (created != nullptr) release_counted(created, host);
                Raw{static_cast<std::uint8_t*>(effect)}.at<std::uint8_t>(9) = 1;
                release_counted(handle, host);
            }
        }
    }
    host.unit_tickable_entity_on_killed_00878990(entity.p);
}

// 004423A0 BSP_DeviceClassRegistry_ClearLuaClassTables, no register or stack
// input, RET; SEH handler 00C5F9B9. Callers 0045F600 (BSP_EngineMovie_Stop)
// and 004DFB70 (BSP_Game_LoadMissionScene). The globals of the game's Lua
// state ([00E188A8]+1A0Ch, 00B67980) give the table "DeviceClass" (00CE44BC);
// the globals temporary is destroyed at once. The table is walked (00B67080 /
// 00B67190 until 00B66420): for each entry the value's field "Got" (00CE452C)
// is read into a temporary; when it is NOT a boolean (00B66000), the key's
// integer (00B66290) is appended to a local vector<int> (00442190). Then for
// each collected index (00415450 size re-read each pass; checked-index report
// 00BF6713 on a null first or an index past the end) "DeviceClass[%d]=nil"
// (00CE4518) is formatted into a 2Ah-byte frame buffer (0043EE60) and run by
// the Lua machine [[00E188A8]+1A08h]+4 (006B8AD0(buffer, 0, 0, 2)). Value,
// key and table are destroyed; the vector's storage is freed (_free) when
// set. SEH states 1..6 are not modelled.
void device_class_registry_clear_lua_class_tables_004423a0(SweepA2Host& host) {
    alignas(4) std::uint8_t vector_storage[0x10];
    const Raw indices{vector_storage};
    indices.ptr(4) = nullptr;
    indices.ptr(8) = nullptr;
    indices.ptr(0xc) = nullptr;
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    LuaObjectStorage globals;
    LuaObjectStorage* const globals_object = host.lua_state_owner_get_globals_00b67980(game.field(0x1a0c), &globals);
    LuaObjectStorage table;
    host.lua_object_get_by_name_00b67800(globals_object, &table,
                                         static_cast<const char*>(host.global_address(0x00ce44bc)));
    host.lua_object_destruct_00b67700(&globals);
    LuaObjectStorage key;
    host.lua_object_construct_00b65f50(&key);
    LuaObjectStorage value;
    host.lua_object_construct_00b65f50(&value);
    host.lua_object_iterate_first_00b67080(&table, &key, &value);
    while (!host.lua_object_is_unbound_00b66420(&table, &key)) {
        LuaObjectStorage field;
        host.lua_object_get_by_name_00b67800(&value, &field, static_cast<const char*>(host.global_address(0x00ce452c)));
        const bool not_boolean = !host.lua_object_is_boolean_00b66000(&field);
        host.lua_object_destruct_00b67700(&field);
        if (not_boolean) {
            const std::int32_t index = host.lua_object_get_integer_00b66290(&key);
            host.int_vector_push_back_00442190(indices, &index);
        }
        host.lua_object_iterate_next_00b67190(&table, &key, &value);
    }
    char buffer[0x2c];
    for (std::uint32_t i = 0; i < host.vector_size_00415450(indices); ++i) {
        const auto* first = static_cast<const std::int32_t*>(indices.ptr(4));
        if (first == nullptr ||
            i >= static_cast<std::uint32_t>(static_cast<const std::int32_t*>(indices.ptr(8)) - first)) {
            host.invalid_parameter_noinfo_00bf6713();
            first = static_cast<const std::int32_t*>(indices.ptr(4));
        }
        host.string_format_buffer2a_0043ee60(buffer, static_cast<const char*>(host.global_address(0x00ce4518)),
                                             first[i]);
        const Raw current{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
        void* const machine = Raw{static_cast<std::uint8_t*>(current.ptr(0x1a08))}.ptr(4);
        host.lua_machine_run_string_006b8ad0(machine, buffer, nullptr, nullptr, 2);
    }
    host.lua_object_destruct_00b67700(&value);
    host.lua_object_destruct_00b67700(&key);
    host.lua_object_destruct_00b67700(&table);
    if (void* const storage = indices.ptr(4); storage != nullptr) host.free_00bf65ac(storage);
}

// ============================ segment 5 ====================================

// 0045A210, __thiscall(ECX = track, action), RET 4; one caller, 0045E5E0
// (BSP_MovieAnimation_ReadAction). The track keeps its actions in a std::list
// at +108h (sentinel at +10Ch), sorted by the float at action+4. An empty list
// gets the action appended (node bought before the sentinel); an action whose
// time is strictly below the first element's (FCOMIP + JBE: NaN goes on) is
// inserted at the front. Both of those link the node (next->prev = node,
// node->prev->next = node) after the STL size increment and return without a
// virtual call. Otherwise 00455E30(track, &it, time) finds the position, the
// action is inserted after the found node (00459FE0 at {container, node->next})
// and the track's vtable +8 runs. Checked-iterator reports (00BF6713): the
// dereference of an empty first element (cannot fire after the emptiness
// test), a null container or the end node from 00455E30. The container
// identity test (CMP ESI,ESI) is constant and not modelled.
void movie_track_insert_action_sorted_0045a210(Raw track, void* action, SweepA2Host& host) {
    const Raw list{track.field(0x108)};
    void* const value = action;
    const Raw sentinel{static_cast<std::uint8_t*>(track.ptr(0x10c))};
    void* const first = sentinel.ptr(0);
    if (first == sentinel.p) {
        const Raw next{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(list.ptr(4))}.ptr(0))};
        void* const node = host.list_buy_node_00455680(list, next.p, next.ptr(4), &value);
        host.list_grow_count_004599a0(list, 1);
        next.ptr(4) = node;
        Raw{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(node)}.ptr(4))}.ptr(0) = node;
        return;
    }
    const Raw head{static_cast<std::uint8_t*>(list.ptr(4))};
    const Raw front{static_cast<std::uint8_t*>(head.ptr(0))};
    if (front.p == head.p) host.invalid_parameter_noinfo_00bf6713();
    const float time = Raw{static_cast<std::uint8_t*>(action)}.at<float>(4);
    const float front_time = Raw{static_cast<std::uint8_t*>(front.ptr(8))}.at<float>(4);
    if (front_time > time) {
        const Raw next{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(list.ptr(4))}.ptr(0))};
        void* const node = host.list_buy_node_00455680(list, next.p, next.ptr(4), &value);
        host.list_grow_count_004599a0(list, 1);
        next.ptr(4) = node;
        Raw{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(node)}.ptr(4))}.ptr(0) = node;
        return;
    }
    void* found[2] = {nullptr, nullptr};
    host.entity_track_find_action_before_00455e30(track, found, time);
    const Raw container{static_cast<std::uint8_t*>(found[0])};
    if (container.p == nullptr) host.invalid_parameter_noinfo_00bf6713();
    const Raw node{static_cast<std::uint8_t*>(found[1])};
    if (node.p == container.ptr(4)) host.invalid_parameter_noinfo_00bf6713();
    // The result iterator is written over the same frame slots (ESP+10h).
    host.list_insert_00459fe0(list, found, container.p, node.ptr(0), &value);
    host.vcall_track_changed_8(track.p);
}

// ============================ segment 7 ====================================

// 0048C6A0, __thiscall(ECX = layered entity, layer L, index i), RET 8, EAX =
// the member; one caller, 00491AC0 (reconstructed in src/cc13_left8_seg8.cpp,
// which calls this through a host hook). The member vector of layer L is at
// entity + 18Ch + 10h*L (first +4, last +8); a checked-index report (00BF6713)
// precedes each access. An existing member is returned. Otherwise, with
// t = (float)(entity[16Ch + 4L] * i) (FIMUL), d = entity+FCh+0Ch*L, base =
// entity+CCh+0Ch*L, o = entity+12Ch+0Ch*L (all D8-form x87, no DC forms in
// the body): off = (float)(d * t) per axis, p = (float)(base + off), q =
// (float)(o + p). The record 00489C40(game, &q) gives a side (+4); the
// factory [entity + 20h + 4*00803BE0(side)] creates the member (vtable +8);
// the member gets +98h(0, [game+19CCh], identity), +114h(entity), +A0h(),
// its local matrix +74h = identity (004134F0), +A4h..+ACh = p, and is stored
// in the slot. The products and sums are exact in double for these operands
// (float * float, float + float), so the model rounds once like the image.
void* scene_entity_layer_member_0048c6a0(Raw entity, std::int32_t layer, std::int32_t index, SweepA2Host& host) {
    const Raw members{entity.field(0x18c + 0x10 * layer)};
    auto check = [&] {
        const auto* const first = static_cast<void* const*>(members.ptr(4));
        if (first == nullptr ||
            static_cast<std::uint32_t>(index) >=
                static_cast<std::uint32_t>(static_cast<void* const*>(members.ptr(8)) - first)) {
            host.invalid_parameter_noinfo_00bf6713();
        }
    };
    check();
    if (void* const existing = static_cast<void**>(members.ptr(4))[index]; existing != nullptr) return existing;
    const float t = static_cast<float>(static_cast<double>(entity.at<float>(0x16c + 4 * layer)) * index);
    const float* const d = reinterpret_cast<const float*>(entity.field(0xfc + 0xc * layer));
    const float* const base = reinterpret_cast<const float*>(entity.field(0xcc + 0xc * layer));
    const float* const o = reinterpret_cast<const float*>(entity.field(0x12c + 0xc * layer));
    float off[3];
    off[0] = static_cast<float>(static_cast<double>(d[0]) * t);
    off[1] = static_cast<float>(static_cast<double>(d[1]) * t);
    off[2] = static_cast<float>(static_cast<double>(t) * d[2]);
    float p[3];
    float q[3];
    for (int axis = 0; axis < 3; ++axis) p[axis] = static_cast<float>(static_cast<double>(base[axis]) + off[axis]);
    for (int axis = 0; axis < 3; ++axis) q[axis] = static_cast<float>(static_cast<double>(o[axis]) + p[axis]);
    const Raw record{static_cast<std::uint8_t*>(host.game_lookup_at_point_00489c40(host.global_pointer(kGame), q))};
    const std::int32_t side = host.party_relative_to_local_player_00803be0(record.at<std::int32_t>(4));
    void* const member = host.vcall_factory_create_8(entity.ptr(0x20 + 4 * side));
    float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                          0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    host.vcall_object_attach_98(member, 0, game.ptr(0x19cc), identity);
    host.vcall_object_set_owner_114(member, entity.p);
    host.vcall_object_activate_a0(member);
    host.matrix_copy_004134f0(reinterpret_cast<float*>(static_cast<std::uint8_t*>(member) + 0x74), identity);
    const Raw m{static_cast<std::uint8_t*>(member)};
    m.at<float>(0xa4) = p[0];
    m.at<float>(0xa8) = p[1];
    m.at<float>(0xac) = p[2];
    check();
    static_cast<void**>(members.ptr(4))[index] = member;
    return member;
}

// 0048F670 NamedClassBase_ReadLua, __thiscall(ECX = class, LuaObject* table),
// RET 4; SEH handler 00C62CB0; the base reader of many class vtables (callers
// 00470820, 004A7D30, 004ACAB0 and others). "Mesh" (00CE5FD0) is read as a
// string into a NativeString (0041E870; the field temporary destroyed); its
// data (or the empty 00E186ED) is copied into a second NativeString whose
// resource 007188A0 goes to +10h; that string's data returns to the sized
// pool. "FakeFireEfx" (00CE67A0) is read with 00B66380(default 0) (field
// destroyed); a non-zero id is acquired into the reused argument slot
// (00870CD0(&slot, id, 1)) and the counted pointer +14h takes it when it
// differs (retain new, then release old), then the slot is released. A
// std::string "fire" (00CE6798, 4 chars, SSO) names the point group
// 00718870(+10h resource, "fire", 1); its heap buffer (capacity >= 10h) is
// freed and the string reset. With a group: +18h..+20h = 00484270(group, &out,
// 0) and +24h = 1; without: +24h = 0. The mesh string's data returns to the
// sized pool. SEH states 2..6 are not modelled.
void named_class_read_lua_0048f670(Raw cls, LuaObjectStorage* table, SweepA2Host& host) {
    LuaObjectStorage field;
    host.lua_object_get_by_name_00b67800(table, &field, static_cast<const char*>(host.global_address(0x00ce5fd0)));
    alignas(4) std::uint8_t mesh_storage[8];
    const Raw mesh{mesh_storage};
    host.native_string_construct_0041e870(mesh, host.lua_object_get_string_00b662b0(&field));
    host.lua_object_destruct_00b67700(&field);
    const char* data = static_cast<const char*>(mesh.ptr(4));
    if (data == nullptr) data = static_cast<const char*>(host.global_address(0x00e186ed));
    alignas(4) std::uint8_t name_storage[8];
    const Raw name{name_storage};
    host.native_string_construct_0041e870(name, data);
    cls.ptr(0x10) = host.resource_load_with_game_factory_007188a0(name);
    if (void* const bytes = name.ptr(4); bytes != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), bytes,
                                                      name.at<std::uint32_t>(0) + 1, 1);
    }
    LuaObjectStorage efx_field;
    host.lua_object_get_by_name_00b67800(table, &efx_field, static_cast<const char*>(host.global_address(0x00ce67a0)));
    const std::int32_t efx = host.lua_object_get_integer_or_default_00b66380(&efx_field, 0);
    host.lua_object_destruct_00b67700(&efx_field);
    if (efx != 0) {
        void* slot = nullptr;  // the reused table-argument slot
        void* const incoming = *host.effect_handle_acquire_00870cd0(&slot, efx, 1);
        void* const old = cls.ptr(0x14);
        if (old != incoming) {
            cls.ptr(0x14) = incoming;
            if (incoming != nullptr) {
                host.interlocked_increment_ce221c(
                    reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(incoming) + 4));
            }
            if (old != nullptr) release_counted(old, host);
        }
        if (slot != nullptr) release_counted(slot, host);
    }
    alignas(4) std::uint8_t text_storage[0x1c];
    const Raw text{text_storage};
    text.at<std::uint32_t>(0x18) = 0xf;
    text.at<std::uint32_t>(0x14) = 0;
    text.at<std::uint8_t>(4) = 0;
    host.std_string_assign_counted_00408720(text, static_cast<const char*>(host.global_address(0x00ce6798)), 4);
    void* const group = host.resource_lookup_named_point_group_00718870(cls.ptr(0x10), text, 1);
    if (text.at<std::uint32_t>(0x18) >= 0x10) host.free_00bf65ac(text.ptr(4));
    text.at<std::uint32_t>(0x18) = 0xf;
    text.at<std::uint32_t>(0x14) = 0;
    text.at<std::uint8_t>(4) = 0;
    if (group != nullptr) {
        float out[3];
        const float* const point = host.point_group_point_00484270(group, out, 0);
        cls.at<float>(0x18) = point[0];
        cls.at<float>(0x1c) = point[1];
        cls.at<float>(0x20) = point[2];
        cls.at<std::uint8_t>(0x24) = 1;
    } else {
        cls.at<std::uint8_t>(0x24) = 0;
    }
    if (void* const bytes = mesh.ptr(4); bytes != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), bytes,
                                                      mesh.at<std::uint32_t>(0) + 1, 1);
    }
}

namespace {

// The NativeString assignment 0045D230 inlines for each member: skipped when
// both are the same object; else 0041DD40(destination, source length, 1) and,
// when the source length is non-zero, memcpy of the destination's new length
// (no terminator byte; the resize writes it).
void assign_native_string(Raw destination, Raw source, SweepA2Host& host) {
    if (destination.p == source.p) return;
    host.native_string_resize_0041dd40(destination, source.at<std::uint32_t>(0), 1);
    if (source.at<std::uint32_t>(0) != 0) {
        std::memcpy(destination.ptr(4), source.ptr(4), destination.at<std::uint32_t>(0));
    }
}

}  // namespace

// 0045D230, __thiscall(ECX = destination, const source*), RET 4, EAX =
// destination; callers 0045F3A0 and 0045FAF0 (STL fill/copy helpers of the
// movie track lists). The implicit copy assignment of a 58h-byte record:
// NativeStrings at +0, +8, +14h, +20h, +2Ch, +38h, +40h, +4Ch and bytes at
// +10h, +1Ch, +28h, +34h, +48h, +54h, copied in address order.
Raw movie_record_assign_0045d230(Raw destination, Raw source, SweepA2Host& host) {
    assign_native_string(destination, source, host);
    assign_native_string(Raw{destination.field(8)}, Raw{source.field(8)}, host);
    destination.at<std::uint8_t>(0x10) = source.at<std::uint8_t>(0x10);
    assign_native_string(Raw{destination.field(0x14)}, Raw{source.field(0x14)}, host);
    destination.at<std::uint8_t>(0x1c) = source.at<std::uint8_t>(0x1c);
    assign_native_string(Raw{destination.field(0x20)}, Raw{source.field(0x20)}, host);
    destination.at<std::uint8_t>(0x28) = source.at<std::uint8_t>(0x28);
    assign_native_string(Raw{destination.field(0x2c)}, Raw{source.field(0x2c)}, host);
    destination.at<std::uint8_t>(0x34) = source.at<std::uint8_t>(0x34);
    assign_native_string(Raw{destination.field(0x38)}, Raw{source.field(0x38)}, host);
    assign_native_string(Raw{destination.field(0x40)}, Raw{source.field(0x40)}, host);
    destination.at<std::uint8_t>(0x48) = source.at<std::uint8_t>(0x48);
    assign_native_string(Raw{destination.field(0x4c)}, Raw{source.field(0x4c)}, host);
    destination.at<std::uint8_t>(0x54) = source.at<std::uint8_t>(0x54);
    return destination;
}

// 0045F110, __thiscall(ECX = destination, const source*), RET 4, EAX =
// destination; SEH handler 00C60ECA; callers 0045F3F0, 0045FB40, 0045FBA0
// and others (STL construct/uninitialized-copy helpers). The implicit copy
// constructor of the 58h-byte record 0045D230 assigns: each NativeString is
// zeroed ({0, 0}) and then assigned as in 0045D230, each byte copied, in
// address order. The SEH states (one per constructed member) are not
// modelled.
Raw movie_record_copy_construct_0045f110(Raw destination, Raw source, SweepA2Host& host) {
    auto string = [&](std::size_t offset) {
        const Raw d{destination.field(offset)};
        d.at<std::uint32_t>(0) = 0;
        d.ptr(4) = nullptr;
        assign_native_string(d, Raw{source.field(offset)}, host);
    };
    string(0);
    string(8);
    destination.at<std::uint8_t>(0x10) = source.at<std::uint8_t>(0x10);
    string(0x14);
    destination.at<std::uint8_t>(0x1c) = source.at<std::uint8_t>(0x1c);
    string(0x20);
    destination.at<std::uint8_t>(0x28) = source.at<std::uint8_t>(0x28);
    string(0x2c);
    destination.at<std::uint8_t>(0x34) = source.at<std::uint8_t>(0x34);
    string(0x38);
    string(0x40);
    destination.at<std::uint8_t>(0x48) = source.at<std::uint8_t>(0x48);
    string(0x4c);
    destination.at<std::uint8_t>(0x54) = source.at<std::uint8_t>(0x54);
    return destination;
}

// 0045D930, __thiscall(ECX = cutscene player), RET; SEH handler 00C60C5E;
// callers 004B9290, 004BEA40, 004CCE50 and others (the player's destructor
// body). For each track in the vector +64h (first +68h, last +6Ch), with the
// checked-index report (00BF6713) before each of its three element accesses:
// vtable slot 0 (called without a null test) decides the kind; a non-null track is destroyed as a camera track
// (0045BE40) when it answers true, else as an entity track (0045B220), and
// freed (_free); the slot is cleared. The vector is then erased whole (two
// first > last reports; the memmove_s of the empty tail never runs because
// the tail length is last - last = 0) so last = first. A NativeString sized
// 0 (preserve 1) gets the empty text 00CE3A0C (memcpy length + 1) and the
// voice manager [[00E198C4]+A4h] starts a fade 005B9760(1.0f, 1.0f, &text);
// the text's data returns to the sized pool. The vector's storage is freed
// and +68h..+70h cleared; the NativeString at +58h has its data returned to
// the sized pool. SEH states 1/2 are not modelled.
void cutscene_player_destruct_0045d930(Raw player, SweepA2Host& host) {
    auto first = [&] { return static_cast<void**>(player.ptr(0x68)); };
    auto size = [&] { return static_cast<std::uint32_t>(static_cast<void**>(player.ptr(0x6c)) - first()); };
    auto check = [&](std::uint32_t i) {
        if (first() == nullptr || i >= size()) host.invalid_parameter_noinfo_00bf6713();
    };
    for (std::uint32_t i = 0; first() != nullptr && i < size(); ++i) {
        check(i);
        const bool camera = host.vcall_track_is_camera_track_0(first()[i]);
        check(i);
        if (void* const track = first()[i]; track != nullptr) {
            if (camera) {
                host.movie_camera_track_destroy_0045be40(track);
            } else {
                host.movie_entity_track_destroy_0045b220(track);
            }
            host.free_00bf65ac(track);
        }
        check(i);
        first()[i] = nullptr;
    }
    void** const last = static_cast<void**>(player.ptr(0x6c));
    if (first() > last) host.invalid_parameter_noinfo_00bf6713();
    void** const begin = first();
    if (begin > static_cast<void**>(player.ptr(0x6c))) host.invalid_parameter_noinfo_00bf6713();
    if (begin != last) {
        const std::int32_t tail = static_cast<std::int32_t>(static_cast<void**>(player.ptr(0x6c)) - last);
        if (tail > 0) std::memmove(begin, last, static_cast<std::size_t>(tail) * 4);  // 00BF67A7 memmove_s
        player.ptr(0x6c) = begin + tail;
    }
    alignas(4) std::uint8_t text_storage[8] = {};
    const Raw text{text_storage};
    host.native_string_resize_0041dd40(text, 0, 1);
    if (void* const data = text.ptr(4); data != nullptr) {
        std::memcpy(data, host.global_address(0x00ce3a0c), text.at<std::uint32_t>(0) + 1);
    }
    const Raw hud{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    host.voice_manager_begin_sound_fade_005b9760(hud.ptr(0xa4), 1.0f, 1.0f, text);
    if (void* const data = text.ptr(4); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      text.at<std::uint32_t>(0) + 1, 1);
    }
    if (void* const storage = player.ptr(0x68); storage != nullptr) host.free_00bf65ac(storage);
    player.ptr(0x68) = nullptr;
    player.ptr(0x6c) = nullptr;
    player.ptr(0x70) = nullptr;
    if (void* const data = player.ptr(0x5c); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      player.at<std::uint32_t>(0x58) + 1, 1);
    }
}

// ============================ segment 10 ===================================

// 004D4720 BSP_Scene_BuildClassPreloadAliases, __thiscall(ECX = alias list
// 010904E4, a std::list of pooled strings: +4 the sentinel), RET; one caller,
// 004D54EE in 004D4DF0. Clears the list (004D05E0), then walks the checked
// vector<int> of collected class ids at 00F8A09C (first 00F8A0A0, last
// 00F8A0A4, both re-read from the image each step): for each id a temporary
// pooled string gets the "Class_" alias (0095C1C0), a copy node is allocated
// before the sentinel (004CE6F0(sentinel, sentinel->prev, alias)), the size is
// bumped (004CE780(1)), and the node is linked (sentinel+4 = node,
// node->prev->next = node); the temporary's data then goes back to the sized
// pool (00BD1510(00419CC0(), data, length + 1, 1)) when set. Checked-iterator
// reports (00BF6713) fire where the image tests: first > last at the start and
// at each loop test, and the iterator at or past last before the dereference
// and before the increment. The image's container-identity tests compare the
// constant 00F8A09C with itself and can never fire; they are not modelled. SEH
// state 0 covers the temporary; no unwind is modelled.
void scene_build_class_preload_aliases_004d4720(Raw list, SweepA2Host& host) {
    host.pooled_string_list_clear_004d05e0(list);
    const Raw ids{static_cast<std::uint8_t*>(host.global_address(0x00f8a09c))};
    auto first = [&] { return ids.at<std::int32_t*>(4); };
    auto last = [&] { return ids.at<std::int32_t*>(8); };
    std::int32_t* it = first();
    if (it > last()) host.invalid_parameter_noinfo_00bf6713();
    for (;;) {
        std::int32_t* const end = last();
        if (first() > end) host.invalid_parameter_noinfo_00bf6713();
        if (it == end) return;
        if (it >= last()) host.invalid_parameter_noinfo_00bf6713();
        alignas(4) std::uint8_t temporary[8];
        const Raw alias = host.scene_class_preload_alias_0095c1c0(Raw{temporary}, *it);
        const Raw sentinel{static_cast<std::uint8_t*>(list.ptr(4))};
        void* const node = host.alias_node_allocate_copy_004ce6f0(list, sentinel.p, sentinel.ptr(4), alias);
        host.alias_list_grow_count_004ce780(list, 1);
        sentinel.ptr(4) = node;
        const Raw linked{static_cast<std::uint8_t*>(node)};
        Raw{static_cast<std::uint8_t*>(linked.ptr(4))}.ptr(0) = node;
        const Raw temp{temporary};
        if (void* const data = temp.ptr(4); data != nullptr) {
            const std::uint32_t size = temp.at<std::uint32_t>(0) + 1;
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          size, 1);
        }
        if (it >= last()) host.invalid_parameter_noinfo_00bf6713();
        ++it;
    }
}

// 004E1A50 BSP_AwardTracker_DestroyMembers, __fastcall(ECX = tracker), RET;
// SEH handler 00C6764F; the body of the scalar deleting destructor 004E1C80
// (vtable 00CE7F08). In this order: the trees at +94h, +88h, +7Ch are erased
// whole (004DE3E0(&it, tree, [head], tree, head)), their head node freed
// (_free) and +4/+8 cleared; the deque +68h is drained (004D0140); the
// NativeString +60h releases its data; the pooled-string lists +54h and +48h
// are cleared (004D05E0), their head freed and +4 cleared; the trees +3Ch
// (00444B10) and +30h (004DC4E0) as the first three; the NativeStrings +28h,
// +1Ch, +10h, +8 release their data; then [00E198D0] = 0 and the vptr = the
// base vtable 00CE3818. The descending SEH states (0Ch..0) are not modelled.
void award_tracker_destroy_members_004e1a50(Raw tracker, SweepA2Host& host) {
    using Erase = void (SweepA2Host::*)(Raw, void**, void*, void*, void*, void*);
    auto tree = [&](std::size_t offset, Erase erase) {
        const Raw t{tracker.field(offset)};
        void* const head = t.ptr(4);
        void* result;
        (host.*erase)(t, &result, t.p, Raw{static_cast<std::uint8_t*>(head)}.ptr(0), t.p, head);
        host.free_00bf65ac(t.ptr(4));
        t.ptr(4) = nullptr;
        t.ptr(8) = nullptr;
    };
    auto string = [&](std::size_t offset) {
        const Raw s{tracker.field(offset)};
        if (void* const data = s.ptr(4); data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          s.at<std::uint32_t>(0) + 1, 1);
        }
    };
    auto pooled_list = [&](std::size_t offset) {
        const Raw l{tracker.field(offset)};
        host.pooled_string_list_clear_004d05e0(l);
        host.free_00bf65ac(l.ptr(4));
        l.ptr(4) = nullptr;
    };
    tree(0x94, &SweepA2Host::tree_erase_range_004de3e0);
    tree(0x88, &SweepA2Host::tree_erase_range_004de3e0);
    tree(0x7c, &SweepA2Host::tree_erase_range_004de3e0);
    host.award_tracker_drain_hint_deque_004d0140(Raw{tracker.field(0x68)});
    string(0x60);
    pooled_list(0x54);
    pooled_list(0x48);
    tree(0x3c, &SweepA2Host::tree_erase_range_00444b10);
    tree(0x30, &SweepA2Host::tree_erase_range_004dc4e0);
    string(0x28);
    string(0x1c);
    string(0x10);
    string(0x8);
    *static_cast<std::int32_t*>(host.global_address(0x00e198d0)) = 0;
    tracker.at<std::uint32_t>(0) = 0x00ce3818u;
}

// 004D7BA0 BSP_Game_ShowSessionLostMessage, __thiscall(ECX = game, char
// kicked), RET 4; SEH handler 00C66408; caller 004D80D0. When the session
// [00F8A2FC] answers vtable +14h false: 006881F0 runs while [[00E198B4]+20h]
// is 1Bh (tested before each call), then the text is sized to 17h by 0041DD40
// (preserve 0) and "FE.multi_cablelost_live" (00CE7990) copied with memcpy of
// the length (no terminator byte; the resize writes it). True: 0041E350
// assigns "ingame.multi_kicked" (00CE797C) when `kicked`, else
// "FE.multi_conlost" (00CE7968). Then game+608h = 1, 004CD0F0(1, 0, 1),
// [00E19698]+4 = 0 when that pointer is set, [00F1B038] = 1; a second string
// is sized 0 (preserve 1) and given the empty text 00CE3A0C (memcpy length +
// 1); a by-value NativeString of 00CE3A0C is built (0041E870) and the menu
// command screen (00425D10) raises the prompt 00531B00(1, &text, 2, callback
// 004D7B10, 0, &second, 0.0f, 0, by-value string, 1). Both strings' data go
// back to the sized pool (second first). SEH states 1/2 are not modelled.
void game_show_session_lost_message_004d7ba0(Raw game, std::uint8_t kicked, SweepA2Host& host) {
    alignas(4) std::uint8_t text_storage[8] = {};
    const Raw text{text_storage};
    if (!host.vcall_session_query_14(host.global_pointer(0x00f8a2fc))) {
        while (Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198b4))}.at<std::int32_t>(0x20) == 0x1b) {
            host.multi_menu_pop_request_interface_006881f0();
        }
        host.native_string_resize_0041dd40(text, 0x17, 0);
        if (void* const data = text.ptr(4); data != nullptr) {
            std::memcpy(data, host.global_address(0x00ce7990), text.at<std::uint32_t>(0));
        }
    } else {
        host.native_string_assign_cstring_0041e350(
            text, static_cast<const char*>(host.global_address(kicked != 0 ? 0x00ce797c : 0x00ce7968)));
    }
    game.at<std::uint8_t>(0x608) = 1;
    host.game_set_cinematic_mode_004cd0f0(game, 1, 0, 1);
    if (void* const overlay = host.global_pointer(0x00e19698); overlay != nullptr) {
        Raw{static_cast<std::uint8_t*>(overlay)}.at<std::uint8_t>(4) = 0;
    }
    *static_cast<std::int32_t*>(host.global_address(0x00f1b038)) = 1;
    alignas(4) std::uint8_t second_storage[8] = {};
    const Raw second{second_storage};
    host.native_string_resize_0041dd40(second, 0, 1);
    if (void* const data = second.ptr(4); data != nullptr) {
        std::memcpy(data, host.global_address(0x00ce3a0c), second.at<std::uint32_t>(0) + 1);
    }
    alignas(4) std::uint8_t by_value_storage[8];
    const Raw by_value{by_value_storage};
    host.native_string_construct_0041e870(by_value, static_cast<const char*>(host.global_address(0x00ce3a0c)));
    void* const menu = host.menu_command_screen_get_singleton_00425d10();
    host.menu_prompt_screen_raise_00531b00(menu, 1, text, 2, 0x004d7b10u, 0, second, 0.0f, 0, by_value, 1);
    if (void* const data = second.ptr(4); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      second.at<std::uint32_t>(0) + 1, 1);
    }
    if (void* const data = text.ptr(4); data != nullptr) {
        host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                      text.at<std::uint32_t>(0) + 1, 1);
    }
}

// 004D8620 BSP_Frontend_UpdateActiveScreens, __fastcall(ECX = game), RET;
// SEH handler 00C664C8; one caller, 004E549D in BSP_Game_OnMove. Fast path,
// when the byte 00E188AE, the menu command screen's +4 (00425D10) and
// [00E19698]+4 are all clear: GUI manager (004C12B0) update 00AA4F80(dt =
// game+21ECh, 0). Otherwise a local vector<page*> (VC8 layout, 10h bytes,
// first at +4) collects, when 00E188AE is clear, the menu screen's visible
// pages (005329C0, only when its +4 is set) and [00E19698]+8 (only when its
// +4 is set); then always [[00E18D48]+1Ch]; then, when [00E198C4] is 0 and
// [00E198AC] is not, the page 00AA5840("FE_main" 00CE7A70 as a NativeString,
// 1, 0) (the string's data then goes back to the sized pool). The vector is
// copied by value (004D35D0) into 00AA3F70(pages, dt = game+21ECh), which
// destroys the copy; the local's storage is freed with _free (00BF65AC) when
// set. SEH states 1/2 cover the string and the copy; no unwind is modelled.
void frontend_update_active_screens_004d8620(Raw game, SweepA2Host& host) {
    const Raw flags{static_cast<std::uint8_t*>(host.global_address(0x00e188ae))};
    if (flags.at<std::uint8_t>(0) == 0 &&
        Raw{static_cast<std::uint8_t*>(host.menu_command_screen_get_singleton_00425d10())}.at<std::uint8_t>(4) == 0 &&
        Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e19698))}.at<std::uint8_t>(4) == 0) {
        void* const gui = host.gui_manager_get_or_create_004c12b0();
        host.gui_manager_update_00aa4f80(gui, game.at<float>(0x21ec), 0);
        return;
    }
    alignas(4) std::uint8_t vector_storage[0x10];
    const Raw pages{vector_storage};
    pages.ptr(4) = nullptr;
    pages.ptr(8) = nullptr;
    pages.ptr(0xc) = nullptr;
    void* value = nullptr;
    if (flags.at<std::uint8_t>(0) == 0) {
        if (Raw{static_cast<std::uint8_t*>(host.menu_command_screen_get_singleton_00425d10())}.at<std::uint8_t>(4) != 0) {
            host.front_end_collect_visible_pages_005329c0(host.menu_command_screen_get_singleton_00425d10(), pages);
        }
        const Raw overlay{static_cast<std::uint8_t*>(host.global_pointer(0x00e19698))};
        if (overlay.at<std::uint8_t>(4) != 0) {
            value = overlay.ptr(8);
            host.page_vector_push_back_004d6790(pages, &value);
        }
    }
    value = Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e18d48))}.ptr(0x1c);
    host.page_vector_push_back_004d6790(pages, &value);
    if (*static_cast<std::int32_t*>(host.global_address(0x00e198c4)) == 0 &&
        *static_cast<std::int32_t*>(host.global_address(0x00e198ac)) != 0) {
        alignas(4) std::uint8_t name_storage[8];
        const Raw name{name_storage};
        host.native_string_construct_0041e870(name, static_cast<const char*>(host.global_address(0x00ce7a70)));
        void* const gui = host.gui_manager_get_or_create_004c12b0();
        value = host.gui_manager_load_page_00aa5840(gui, name, 1, 0);
        host.page_vector_push_back_004d6790(pages, &value);
        if (void* const data = name.ptr(4); data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          name.at<std::uint32_t>(0) + 1, 1);
        }
    }
    const float dt = game.at<float>(0x21ec);
    alignas(4) std::uint8_t copy_storage[0x10];
    const Raw copy{copy_storage};
    host.page_vector_copy_construct_004d35d0(copy, pages);
    void* const gui = host.gui_manager_get_or_create_004c12b0();
    host.gui_manager_update_page_list_00aa3f70(gui, copy, dt);
    if (void* const first = pages.ptr(4); first != nullptr) host.free_00bf65ac(first);
}

// ============================ segment 13 ===================================

// 0050B730, __thiscall(ECX = tactical library screen), RET; SEH handler
// 00C69B88; one caller, 0050D900 (view kind 5). The shown-entry vector at
// +444h (first +448h, last +44Ch) is emptied by 00505DE0 after two
// first > last reports (00BF6713). The selected item of the list widget
// +B8h (00A9CA00, called twice) gives an id: atol of its text (vtable +28h,
// data +4, the empty 00E18D78 when null); 99999 returns at once; no
// selection leaves id 0. With the group vector +45Ch (2Ch-byte entries,
// first +460h) non-empty and the id below its count (unsigned), entry =
// 004F9360(id): a local vector<int> receives the entry's +0 (00442190), and
// the shown vector receives 004FB090(entry +0) (0050AD00); then for each
// child id of the entry's vector +1Ch (first) / +20h (last), re-read every
// pass, with the checked-index reports before each of the two reads: the
// local vector receives the child slot, the shown vector 004FB090(child).
// The local vector is never read; its storage is freed (_free) when set.
void tactical_library_show_selected_group_0050b730(Raw screen, SweepA2Host& host) {
    const Raw shown{screen.field(0x444)};
    void* const last = shown.ptr(8);
    if (shown.ptr(4) > last) host.invalid_parameter_noinfo_00bf6713();
    void* const first = shown.ptr(4);
    if (first > shown.ptr(8)) host.invalid_parameter_noinfo_00bf6713();
    void* erased[2];
    host.entry_vector_erase_00505de0(shown, erased, shown.p, first, shown.p, last);
    std::int32_t id = 0;
    if (host.list_widget_selected_item_00a9ca00(screen.ptr(0xb8)) != nullptr) {
        void* const item = host.list_widget_selected_item_00a9ca00(screen.ptr(0xb8));
        const char* text = static_cast<const char*>(host.vcall_item_text_28(item).ptr(4));
        if (text == nullptr) text = static_cast<const char*>(host.global_address(0x00e18d78));
        id = static_cast<std::int32_t>(std::atol(text));
        if (id == 99999) return;
    }
    const Raw groups{screen.field(0x45c)};
    const auto* const groups_first = static_cast<const std::uint8_t*>(groups.ptr(4));
    if (groups_first == nullptr) return;
    const std::int32_t count = static_cast<std::int32_t>(static_cast<const std::uint8_t*>(groups.ptr(8)) - groups_first) / 0x2c;
    if (static_cast<std::uint32_t>(count) <= static_cast<std::uint32_t>(id)) return;
    const Raw entry = host.library_entry_at_004f9360(groups, static_cast<std::uint32_t>(id));
    alignas(4) std::uint8_t local_storage[0x10];
    const Raw local{local_storage};
    local.ptr(4) = nullptr;
    local.ptr(8) = nullptr;
    local.ptr(0xc) = nullptr;
    host.int_vector_push_back_00442190(local, reinterpret_cast<const std::int32_t*>(entry.p));
    host.entry_vector_push_back_0050ad00(shown, host.tactical_library_find_entry_by_id_004fb090(screen, entry.at<std::int32_t>(0)));
    for (std::uint32_t i = 0;; ++i) {
        auto size = [&] {
            return static_cast<std::uint32_t>(static_cast<const std::int32_t*>(entry.ptr(0x20)) -
                                              static_cast<const std::int32_t*>(entry.ptr(0x1c)));
        };
        if (entry.ptr(0x1c) == nullptr || i >= size()) break;
        if (entry.ptr(0x1c) == nullptr || i >= size()) host.invalid_parameter_noinfo_00bf6713();
        host.int_vector_push_back_00442190(local, static_cast<const std::int32_t*>(entry.ptr(0x1c)) + i);
        if (entry.ptr(0x1c) == nullptr || i >= size()) host.invalid_parameter_noinfo_00bf6713();
        const std::int32_t child = static_cast<const std::int32_t*>(entry.ptr(0x1c))[i];
        host.entry_vector_push_back_0050ad00(shown, host.tactical_library_find_entry_by_id_004fb090(screen, child));
    }
    if (void* const storage = local.ptr(4); storage != nullptr) host.free_00bf65ac(storage);
}

// 004F1B60, __fastcall(ECX = const char* list, EDX = owner), RET; SEH handler
// 00C68291; one caller, 004F1D70 (BSP_SceneRecord_ApplyHeaderProperties). The
// list goes into a std::string (00408720, strlen count) that 0094EC70 splits
// at "," (00CE4BFC) into a vector<std::string> (1Ch elements); the string's
// heap buffer is freed and the string reset. Each element (checked-iterator
// reports on first > last at each loop test and on dereferencing or stepping
// at last) is copied with an unbounded strcpy into a 104h-byte frame buffer;
// trailing '"' are cut (from the end, while the index is >= 0), then leading
// '"' skipped while the index stays below the remaining last index (only when
// that index is > 0); the rest becomes a NativeString (0041DD40(strlen, 1) +
// memcpy of length + 1) appended to owner+C6Ch (004CDC20), and its data goes
// back to the sized pool. The vector's strings are destroyed (004CE520) and
// its storage freed when it has one. SEH states 2/3 are not modelled.
void scene_header_append_unquoted_names_004f1b60(const char* list, void* owner, SweepA2Host& host) {
    alignas(4) std::uint8_t text_storage[0x1c];
    const Raw text{text_storage};
    text.at<std::uint32_t>(0x18) = 0xf;
    text.at<std::uint32_t>(0x14) = 0;
    text.at<std::uint8_t>(4) = 0;
    host.std_string_assign_counted_00408720(text, list, static_cast<std::uint32_t>(std::strlen(list)));
    alignas(4) std::uint8_t parts_storage[0x10] = {};
    const Raw parts{parts_storage};
    host.split_string_0094ec70(parts, text, static_cast<const char*>(host.global_address(0x00ce4bfc)));
    if (text.at<std::uint32_t>(0x18) >= 0x10) host.free_00bf65ac(text.ptr(4));
    text.at<std::uint32_t>(0x18) = 0xf;
    text.at<std::uint32_t>(0x14) = 0;
    text.at<std::uint8_t>(4) = 0;
    auto first = [&] { return static_cast<std::uint8_t*>(parts.ptr(4)); };
    auto last = [&] { return static_cast<std::uint8_t*>(parts.ptr(8)); };
    if (first() > last()) host.invalid_parameter_noinfo_00bf6713();
    std::uint8_t* it = first();
    for (;;) {
        if (first() > last()) host.invalid_parameter_noinfo_00bf6713();
        if (it == last()) break;
        if (it >= last()) host.invalid_parameter_noinfo_00bf6713();
        const Raw element{it};
        const char* source = element.at<std::uint32_t>(0x18) >= 0x10 ? static_cast<const char*>(element.ptr(4))
                                                                       : reinterpret_cast<const char*>(element.field(4));
        char buffer[0x104];
        {
            char* out = buffer;
            char c;
            do {
                c = *source++;
                *out++ = c;
            } while (c != '\0');
        }
        std::int32_t index = static_cast<std::int32_t>(std::strlen(buffer)) - 1;
        while (index >= 0 && buffer[index] == '"') {
            --index;
            buffer[index + 1] = '\0';
        }
        std::int32_t start = 0;
        if (index > 0) {
            while (buffer[start] == '"') {
                ++start;
                if (start >= index) break;
            }
        }
        const char* const name = buffer + start;
        alignas(4) std::uint8_t name_storage[8] = {};
        const Raw native{name_storage};
        host.native_string_resize_0041dd40(native, static_cast<std::uint32_t>(std::strlen(name)), 1);
        const std::uint32_t length = native.at<std::uint32_t>(0);
        void* const data = native.ptr(4);
        if (data != nullptr) std::memcpy(data, name, length + 1);
        host.native_string_vector_append_004cdc20(static_cast<std::uint8_t*>(owner) + 0xc6c, native);
        if (data != nullptr) {
            host.sized_storage_pool_return_block_00bd1510(host.sized_storage_pool_get_singleton_00419cc0(), data,
                                                          length + 1, 1);
        }
        if (it >= last()) host.invalid_parameter_noinfo_00bf6713();
        it += 0x1c;
    }
    if (void* const storage = parts.ptr(4); storage != nullptr) {
        host.string_range_destroy_004ce520(storage, parts.ptr(8), parts.p, owner);
        host.free_00bf65ac(parts.ptr(4));
    }
}

// ============================ segment 14 ===================================

// 005213D0 BSP_HudCommandBuildingView_BindUnitCamera, __thiscall(ECX = view,
// unit), RET 4; SEH handler 00C6B2F8; one caller, 0068ACA0 (interface 2Fh on
// this+B0h). With hud = [00E198C4]: 005470C0(hud+50h) first; a previous unit
// (+1Ch) gets message 1FEh (0077C470(1FEh, 0)); the new unit's observer pair
// with view+8 is unregistered when registered (00694AF0 / 006952A0); +1Ch =
// unit (dereferenced: params = unit+538h). An existing camera (+20h) resets
// the binoculars (0051E8E0 on hud+4Ch). With no camera (re-tested after the
// reset): a 3F4h block from operator new, memset 0, constructed by 00519290
// (null on a failed allocation), stored in +20h, attached by 00923870(0,
// [game+19CCh], identity), +3ECh/+3F0h = params+19Ch/+1A0h, +39Ch = 0.4f
// (00CE7804), and registered with view+8 (00694A60). Then 0042F650(camera,
// unit); when the camera's previous target (+3ACh, read before) differs: the
// unit's pose is refreshed if needed (00414DB0) and 00521370 (sweepA) turns
// unit+ECh into pitch/yaw; when the camera's +3E8h is 0: +384h = -0.0f - yaw,
// +388h = -0.17453294f (00CECA08), +38Ch/+398h/+394h/+390h = params+198h.
// Then 004BC410(game, camera), 0051E730(hud+4Ch, camera), 00549260(hud+50h,
// unit, camera), and the unit's pair with view+8 is registered unless it
// already is. The SEH state around the allocation is not modelled.
void command_building_view_bind_unit_camera_005213d0(Raw view, void* unit, SweepA2Host& host) {
    const Raw hud{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    host.hud_weapon_group_screen_release_005470c0(hud.ptr(0x50));
    if (void* const previous = view.ptr(0x1c); previous != nullptr) host.entity_send_message_0077c470(previous, 0x1fe, 0);
    void* const observer = view.field(8);
    if (unit != nullptr && host.observer_is_pair_registered_00694af0(unit, observer)) {
        host.observer_unregister_pair_006952a0(unit, observer);
    }
    view.ptr(0x1c) = unit;
    const Raw params{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(unit)}.ptr(0x538))};
    bool create = view.ptr(0x20) == nullptr;
    if (!create) {
        host.hud_binoculars_reset_0051e8e0(Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))}.ptr(0x4c));
        create = view.ptr(0x20) == nullptr;
    }
    if (create) {
        void* const block = host.operator_new_00bf55be(0x3f4);
        std::memset(block, 0, 0x3f4);
        void* const camera = block != nullptr ? host.ship_camera_construct_base_00519290(block) : nullptr;
        view.ptr(0x20) = camera;
        const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
        const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
        host.camera_attach_00923870(camera, 0, game.ptr(0x19cc), identity);
        const Raw cam{static_cast<std::uint8_t*>(view.ptr(0x20))};
        cam.at<float>(0x3ec) = params.at<float>(0x19c);
        cam.at<float>(0x3f0) = params.at<float>(0x1a0);
        Raw{static_cast<std::uint8_t*>(view.ptr(0x20))}.at<float>(0x39c) = 0.4f;  // 00CE7804 = 3ECCCCCDh
        host.observer_register_pair_00694a60(view.ptr(0x20), observer);
    }
    void* const target = view.ptr(0x1c);
    const Raw camera{static_cast<std::uint8_t*>(view.ptr(0x20))};
    const bool retarget = camera.ptr(0x3ac) != target;
    host.camera_mover_set_target_0042f650(camera.p, target);
    if (retarget) {
        const Raw u{static_cast<std::uint8_t*>(view.ptr(0x1c))};
        if (u.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(u.p);
        float pitch;
        float yaw = 0.0f;  // the image writes the reused argument slot
        cc13_sweepA::direction_to_pitch_yaw_00521370(reinterpret_cast<const float*>(u.field(0xec)), &pitch, &yaw, host);
        const Raw cam{static_cast<std::uint8_t*>(view.ptr(0x20))};
        const float p198 = params.at<float>(0x198);
        if (cam.at<std::int32_t>(0x3e8) == 0) {
            cam.at<float>(0x384) = -0.0f - yaw;
            std::uint32_t bits = 0xbe32b8c3u;  // 00CECA08
            float tilt;
            std::memcpy(&tilt, &bits, sizeof tilt);
            cam.at<float>(0x388) = tilt;
            cam.at<float>(0x38c) = p198;
            cam.at<float>(0x398) = p198;
            cam.at<float>(0x394) = p198;
            cam.at<float>(0x390) = p198;
        }
    }
    host.game_set_camera_mover_004bc410(host.global_pointer(kGame), view.ptr(0x20));
    host.hud_binoculars_bind_0051e730(Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))}.ptr(0x4c),
                                      view.ptr(0x20));
    host.hud_weapon_group_screen_bind_00549260(Raw{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))}.ptr(0x50),
                                               view.ptr(0x1c), view.ptr(0x20));
    if (void* const bound = view.ptr(0x1c); bound != nullptr) {
        if (!host.observer_is_pair_registered_00694af0(bound, observer)) {
            host.observer_register_pair_00694a60(view.ptr(0x1c), observer);
        }
    }
}

// 005224C0, ECX unused, (squadron), RET 4, EAX = member index; one caller,
// 00642960 (BSP_InGameHudMarkers_AddSquadronMemberMarkers). Count +3CCh (signed;
// none: EAX = 0); member i is [+3D0h + 4i] for i <= 4, else null (dereferenced
// as the image does). A member whose byte +C8h is clear gets the inlined pose
// refresh: the parent +3Ch refreshed (00414DB0) when set; the world matrix
// +CCh = 00413920(local +74h, temp, parent +CCh) with a parent, else a copy of
// +74h (004134F0); +C8h = 1, +10Ch = 0. Then 00638E50 projects +FCh to the
// screen; dx = (float)(x - 0.5) (DC E9 FSUB ST1,ST0 with the double 0.5 at
// 00D7A280; operand order checked by tools/x87_dc_form_scan.py), dy =
// (float)(y - 0.5) (FSUBR float); |d| is d when d > 0 else -0.0f - d (SSE).
// The member wins when the best so far (float, start 20.0f from 00CE3930) is
// strictly above |dx| + |dy| (x87 FADD of two floats, compared unrounded;
// modelled in double, which is exact unless the two magnitudes differ by more
// than 2^29; x - 0.5 likewise); then best = (float)(|dy| + |dx|) and the index
// is kept. The projection result is ignored.
std::int32_t squadron_member_nearest_screen_centre_005224c0(Raw squadron, SweepA2Host& host) {
    float best = 20.0f;  // 00CE3930
    std::int32_t best_index = 0;
    if (squadron.at<std::int32_t>(0x3cc) <= 0) return 0;
    for (std::int32_t i = 0; i < squadron.at<std::int32_t>(0x3cc); ++i) {
        const Raw member{static_cast<std::uint8_t*>(static_cast<std::uint32_t>(i) <= 4 ? squadron.ptr(0x3d0 + 4 * i)
                                                                                       : nullptr)};
        if (member.at<std::uint8_t>(0xc8) == 0) {
            if (void* const parent = member.ptr(0x3c); parent != nullptr) host.entity_pose_refresh_world_00414db0(parent);
            const float* source;
            alignas(4) float product[16];
            if (void* const parent = member.ptr(0x3c); parent != nullptr) {
                source = host.matrix_multiply_00413920(reinterpret_cast<const float*>(member.field(0x74)), product,
                                                       reinterpret_cast<const float*>(Raw{static_cast<std::uint8_t*>(parent)}.field(0xcc)));
            } else {
                source = reinterpret_cast<const float*>(member.field(0x74));
            }
            host.matrix_copy_004134f0(reinterpret_cast<float*>(member.field(0xcc)), source);
            member.at<std::uint8_t>(0xc8) = 1;
            member.at<std::uint8_t>(0x10c) = 0;
        }
        float screen[3];
        host.hud_marker_project_and_clip_00638e50(reinterpret_cast<const float*>(member.field(0xfc)), screen);
        const float dx = static_cast<float>(static_cast<double>(screen[0]) - 0.5);
        const float dy = static_cast<float>(static_cast<double>(screen[1]) - 0.5);
        const float adx = dx > 0.0f ? dx : -0.0f - dx;
        const float ady = dy > 0.0f ? dy : -0.0f - dy;
        if (static_cast<double>(best) > static_cast<double>(ady) + static_cast<double>(adx)) {
            best_index = i;
            best = static_cast<float>(static_cast<double>(ady) + static_cast<double>(adx));
        }
    }
    return best_index;
}

// 005226D0, ECX unused, (unit), RET 4, EAX = 0 or 1 (AL = 0 for a null unit,
// the rest of EAX then the caller's). Callers 00525F80 and 00526180. True when
// the unit is in the current side's set [[00E188A8] + 21A4h + [..+18ECh]*4]
// (008DDF90), or when it is eligible: byte +5Ch set, +5Dh, +60h and +5Eh
// clear, one of the kinds 6, 18h, 1Ah, 5 (tested in that order through vtable
// +5Ch), and then kind 1Ch, or none of 1Bh, 0Fh, 19h (in that order). The
// set test runs first and unconditionally.
bool unit_pick_is_command_candidate_005226d0(void* unit, SweepA2Host& host) {
    if (unit == nullptr) return false;
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    void* const set = game.ptr(0x21a4 + 4 * game.at<std::int32_t>(0x18ec));
    const bool in_set = host.unit_set_contains_008ddf90(set, unit);
    const Raw u{static_cast<std::uint8_t*>(unit)};
    bool eligible = false;
    if (u.at<std::uint8_t>(0x5c) != 0 && u.at<std::uint8_t>(0x5d) == 0 && u.at<std::uint8_t>(0x60) == 0 &&
        u.at<std::uint8_t>(0x5e) == 0 &&
        (host.vcall_entity_is_kind_5c(unit, 6) || host.vcall_entity_is_kind_5c(unit, 0x18) ||
         host.vcall_entity_is_kind_5c(unit, 0x1a) || host.vcall_entity_is_kind_5c(unit, 5))) {
        if (host.vcall_entity_is_kind_5c(unit, 0x1c)) {
            eligible = true;
        } else {
            eligible = !host.vcall_entity_is_kind_5c(unit, 0x1b) && !host.vcall_entity_is_kind_5c(unit, 0xf) &&
                       !host.vcall_entity_is_kind_5c(unit, 0x19);
        }
    }
    return in_set || eligible;
}

// 00522F10, __thiscall(ECX = screen, unit, void** command_out, float*
// best_distance, int mask, owner, void** best), RET 18h, no result. Caller
// 00523020. The unit must pass the side count (>= 2) and 00804350(owner, unit,
// mask). The command is 00522680(player [00E188D8], unit); unless the player
// is kind 18h with byte +379h set, the player must also allow that command's
// name (vtable +16Ch). A unit not flagged at +C8h has its pose refreshed
// (00414DB0); its screen distance 00522050(unit+FCh), stored as a float, is
// taken when *best_distance is strictly greater (NaN never wins) and 005220C0
// accepts the unit: *best = unit, *command_out = command, *best_distance = d.
void unit_pick_consider_unit_00522f10(Raw screen, void* unit, void** command_out, float* best_distance,
                                      std::int32_t mask, void* owner, void** best, SweepA2Host& host) {
    if (side_count(unit, host) < 2) return;
    if (!host.unit_sensors_detect_unit_00804350(owner, unit, mask)) return;
    bool controlled = false;
    if (host.vcall_entity_is_kind_5c(host.global_pointer(kPlayer), 0x18)) {
        const Raw player{static_cast<std::uint8_t*>(host.global_pointer(kPlayer))};
        if (player.at<std::uint8_t>(0x379) != 0) controlled = true;
    }
    void* const command = const_cast<void*>(
        cc13_sweepA::unit_pick_attackmove_color_00522680(host.global_pointer(kPlayer), as_arg(unit), host));
    if (!controlled) {
        const char* const name = host.vcall_command_name_4(command);
        if (!host.vcall_entity_accepts_command_16c(host.global_pointer(kPlayer), name, as_arg(unit))) return;
    }
    const Raw u{static_cast<std::uint8_t*>(unit)};
    if (u.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float distance = static_cast<float>(
        host.unit_pick_centre_distance_sq_00522050(screen, reinterpret_cast<const float*>(u.field(0xfc))));
    if (!(*best_distance > distance)) return;
    if (!host.unit_pick_accepts_unit_005220c0(screen, unit)) return;
    *best = unit;
    *command_out = command;
    *best_distance = distance;
}

// 00523020, __thiscall(ECX = screen, void** command_out), RET 4, EAX = the
// chosen unit or null; caller 006435D0. The sensor owner as in 00523130. Walks
// the game's unit list ([[00E188A8]+19BCh], next +4, unit +8): a unit of kind
// 18h offers its members instead (count +3CCh, signed, members at +3D0h; an
// index above 4 offers null, as the image's bounded read does), any other
// unit offers itself, each through 00522F10(screen, candidate, command_out,
// &best_distance, 5, owner, &best). Best distance starts at 1e10f (00CE4970).
// Empty list: EAX = 0.
void* unit_pick_find_member_target_00523020(Raw screen, void** command_out, SweepA2Host& host) {
    float best_distance = no_pick_distance();
    void* best = nullptr;
    void* const owner = sensor_owner(host);
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    Raw node{static_cast<std::uint8_t*>(game.ptr(0x19bc))};
    if (node.p == nullptr) return nullptr;
    do {
        void* const unit = node.ptr(8);
        if (host.vcall_entity_is_kind_5c(unit, 0x18)) {
            const Raw u{static_cast<std::uint8_t*>(unit)};
            for (std::int32_t i = 0; i < u.at<std::int32_t>(0x3cc); ++i) {
                void* const member = static_cast<std::uint32_t>(i) <= 4 ? u.ptr(0x3d0 + 4 * i) : nullptr;
                unit_pick_consider_unit_00522f10(screen, member, command_out, &best_distance, 5, owner, &best,
                                                 host);
            }
        } else {
            unit_pick_consider_unit_00522f10(screen, unit, command_out, &best_distance, 5, owner, &best, host);
        }
        node = Raw{static_cast<std::uint8_t*>(node.ptr(4))};
    } while (node.p != nullptr);
    return best;
}

// 00523130, __thiscall(ECX = unit pick screen, void** command_out), RET 4,
// EAX = the chosen unit or null. The sensor owner is the player [00E188D8]
// when its +5Ch query (5) holds, else the player's +3D0h when query (18h)
// holds, else null. Walks the game's unit list ([game+19BCh], next +4, unit
// +8): a unit whose count at unit + [[game + 18CCh + [game+18ECh]*4]+28h]*34h
// + 1E8h (+8 when the byte at +1F8h of that block is set, else +4) is below 2
// (signed) is skipped, as is one 00804350(owner, unit, 5) rejects. The command
// object is 00E08F78 when [game+19C4h] is set, player query (6) holds and the
// player allows "attackmove" for the unit, else 00E08EF8; the player must then
// allow that command's name (its vtable slot 1). A unit not flagged at +C8h
// has its pose refreshed (00414DB0); the screen distance 00522050(unit+FCh)
// is stored as a float (FSTP float) and the unit is taken when it is strictly
// below the best so far (start 1e10f from 00CE4970; FCOMIP + JBE, so NaN never
// wins) and 005220C0 accepts it; then *command_out = the command object.
// command_out is written only on a take. Empty list: EAX = 0.
void* unit_pick_find_command_target_00523130(Raw screen, void** command_out, SweepA2Host& host) {
    float best_distance = no_pick_distance();
    void* best = nullptr;
    void* const owner = sensor_owner(host);
    Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    Raw node{static_cast<std::uint8_t*>(game.ptr(0x19bc))};
    if (node.p == nullptr) return nullptr;
    do {
        void* const unit = node.ptr(8);
        if (side_count(unit, host) >= 2 && host.unit_sensors_detect_unit_00804350(owner, unit, 5)) {
            // 00522680's choice, inlined here (005231F2..00523244).
            void* player = host.global_pointer(kPlayer);
            void* command = host.global_address(0x00e08ef8);
            game = Raw{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
            if (game.at<std::uint8_t>(0x19c4) != 0 && host.vcall_entity_is_kind_5c(player, 6) &&
                host.vcall_entity_accepts_command_16c(player, "attackmove", as_arg(unit))) {
                command = host.global_address(0x00e08f78);
            }
            player = host.global_pointer(kPlayer);
            const char* const name = host.vcall_command_name_4(command);
            if (host.vcall_entity_accepts_command_16c(player, name, as_arg(unit))) {
                const Raw u{static_cast<std::uint8_t*>(unit)};
                if (u.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(unit);
                const float distance = static_cast<float>(
                    host.unit_pick_centre_distance_sq_00522050(screen, reinterpret_cast<const float*>(u.field(0xfc))));
                if (best_distance > distance && host.unit_pick_accepts_unit_005220c0(screen, unit)) {
                    best = unit;
                    *command_out = command;
                    best_distance = distance;
                }
            }
        }
        node = Raw{static_cast<std::uint8_t*>(node.ptr(4))};
    } while (node.p != nullptr);
    return best;
}

// 00525250, __thiscall(ECX = unit pick screen), RET, AL. With a target from
// 00523130 (command slot zero-initialised): 00525170(screen+C0h), then a 18h
// descriptor {byte kind 1, byte 0, uint16 unit+174h, unit, float3 from
// 00F87574..00F8757C, float 0} goes to 0077D600 on the player [00E188D8] with
// the chosen command and flags = player query +5Ch(18h); AL = 1. No target: a
// tracked sound 00A7E490(sample screen+DCh, class +E4h, type +E8h, 1) on
// [00F8BBD8] is created and released at once (InterlockedDecrement on +4, the
// vtable slot 0 release on zero); AL = 0.
bool unit_pick_issue_command_at_cursor_00525250(Raw screen, SweepA2Host& host) {
    void* command = nullptr;
    void* const target = unit_pick_find_command_target_00523130(screen, &command, host);
    if (target != nullptr) {
        host.stl_inst_00525170(Raw{screen.field(0xc0)});
        alignas(4) std::uint8_t descriptor[0x18];
        const Raw d{descriptor};
        const float* const cursor = static_cast<const float*>(host.global_address(0x00f87574));
        d.at<std::uint16_t>(0) = 0;
        d.at<float>(8) = cursor[0];
        d.at<float>(0xc) = cursor[1];
        d.at<float>(0x10) = cursor[2];
        d.ptr(4) = target;
        d.at<std::uint8_t>(0) = 1;
        d.at<std::uint16_t>(2) = Raw{static_cast<std::uint8_t*>(target)}.at<std::uint16_t>(0x174);
        d.at<float>(0x14) = 0.0f;
        void* const player = host.global_pointer(kPlayer);
        const std::uint32_t flags = host.vcall_entity_is_kind_5c_eax(player, 0x18);
        host.entity_issue_command_0077d600(host.global_pointer(kPlayer), command, descriptor, flags);
        return true;
    }
    void* sound = command;  // the image reuses the command slot for the result
    host.sound_create_tracked_00a7e490(host.global_pointer(kSoundManager), &sound, screen.ptr(0xdc),
                                       screen.at<std::int32_t>(0xe4), screen.at<std::int32_t>(0xe8), 1);
    if (sound != nullptr) {
        if (host.interlocked_decrement_ce2220(
                reinterpret_cast<volatile long*>(static_cast<std::uint8_t*>(sound) + 4)) == 0) {
            host.vcall_release_slot0(sound);
        }
    }
    return false;
}

// 00525F80, __thiscall(ECX = unit pick screen), RET; SEH handler 00C6B5E0;
// one caller, 00526720. The player [00E188D8] is asked kind 5 and, when that
// fails, kind 18h (both results unused). With the game's unit count
// [[00E188A8]+19B8h] above 0 (signed): every listed unit (node +8, next +4)
// that 005226D0 accepts gets a {unit, distance} record (00522860, sweepA)
// appended to a local vector (00525D90); after two first > last reports
// the vector is sorted (00524E90 over (last - first)/8 elements). In sorted
// order each unit is wrapped in an observer handle (vptr 00CE74FC, +14h unit,
// +10h byte 1, registered with 00694A60 when the unit is set), the handle is
// re-tagged 00CE75CC and appended to the screen's vector +C4h (00525ED0),
// re-tagged 00CE74FC, unregistered (006952A0, unit set) and destroyed
// (00695870). Then when the screen's vector (first +C8h, 18h elements) is
// non-empty (checked-index report on an empty or null one) and its first
// element's unit equals 00927880(player), +D4h = 0. The local vector's
// storage is freed (_free). SEH states 1..3 are not modelled.
void unit_pick_collect_candidates_00525f80(Raw screen, SweepA2Host& host) {
    void* const player = host.global_pointer(kPlayer);
    if (!host.vcall_entity_is_kind_5c(player, 5)) host.vcall_entity_is_kind_5c(player, 0x18);
    const Raw units{static_cast<std::uint8_t*>(host.global_pointer(kGame)) + 0x19b8};
    if (units.at<std::int32_t>(0) <= 0) return;
    alignas(4) std::uint8_t vector_storage[0x10];
    const Raw records{vector_storage};
    records.ptr(4) = nullptr;
    records.ptr(8) = nullptr;
    records.ptr(0xc) = nullptr;
    for (Raw node{static_cast<std::uint8_t*>(units.ptr(4))}; node.p != nullptr;
         node = Raw{static_cast<std::uint8_t*>(node.ptr(4))}) {
        void* const unit = node.ptr(8);
        if (!unit_pick_is_command_candidate_005226d0(unit, host)) continue;
        alignas(4) std::uint8_t record_storage[8];
        const Raw record = cc13_sweepA::unit_distance_record_set_00522860(Raw{record_storage}, unit, host);
        host.distance_vector_push_back_00525d90(records, record);
    }
    auto* const last = static_cast<std::uint8_t*>(records.ptr(8));
    if (static_cast<std::uint8_t*>(records.ptr(4)) > last) host.invalid_parameter_noinfo_00bf6713();
    auto* const first = static_cast<std::uint8_t*>(records.ptr(4));
    if (first > static_cast<std::uint8_t*>(records.ptr(8))) host.invalid_parameter_noinfo_00bf6713();
    host.ptr_float_intro_sort_00524e90(first, last, static_cast<std::int32_t>(last - first) >> 3);
    for (std::uint32_t i = 0;; ++i) {
        const auto* const begin = static_cast<const std::uint8_t*>(records.ptr(4));
        if (begin == nullptr ||
            i >= static_cast<std::uint32_t>((static_cast<const std::uint8_t*>(records.ptr(8)) - begin) >> 3)) {
            break;
        }
        void* const unit = Raw{const_cast<std::uint8_t*>(begin) + 8 * i}.ptr(0);
        alignas(4) std::uint8_t handle_storage[0x18];
        const Raw handle{handle_storage};
        handle.ptr(4) = nullptr;
        handle.ptr(8) = nullptr;
        handle.ptr(0xc) = nullptr;
        handle.at<std::uint32_t>(0) = 0x00ce74fcu;
        handle.ptr(0x14) = unit;
        handle.at<std::uint8_t>(0x10) = 1;
        if (unit != nullptr) host.observer_register_pair_00694a60(unit, handle.p);
        handle.at<std::uint32_t>(0) = 0x00ce75ccu;
        host.handle_vector_push_back_00525ed0(Raw{screen.field(0xc4)}, handle);
        handle.at<std::uint32_t>(0) = 0x00ce74fcu;
        if (void* const held = handle.ptr(0x14); held != nullptr) host.observer_unregister_pair_006952a0(held, handle.p);
        host.callback_owner_destroy_00695870(handle);
    }
    if (const auto* const front = static_cast<const std::uint8_t*>(screen.ptr(0xc8));
        front != nullptr && (static_cast<const std::uint8_t*>(screen.ptr(0xcc)) - front) / 0x18 != 0) {
        const auto* const again = static_cast<const std::uint8_t*>(screen.ptr(0xc8));
        if (again == nullptr || (static_cast<const std::uint8_t*>(screen.ptr(0xcc)) - again) / 0x18 == 0) {
            host.invalid_parameter_noinfo_00bf6713();
        }
        void* const front_unit = Raw{static_cast<std::uint8_t*>(screen.ptr(0xc8))}.ptr(0x14);
        if (front_unit == host.controlled_target_00927880(host.global_pointer(kPlayer))) screen.at<std::int32_t>(0xd4) = 0;
    }
    if (void* const storage = records.ptr(4); storage != nullptr) host.free_00bf65ac(storage);
}

// 005241D0 BSP_HudUnitPickScreen_IssueCommandToTarget, __thiscall(ECX =
// screen, command), RET 4, AL; SEH handler 00C6B398; one caller, 00527260.
// AL = 0 when the player [00E188D8] or the command is null. With
// [game+19C4h] set, a session message 57h (0075B430; +4 = 1, bytes +18h,
// +1Ah, +1Ch and word +18h cleared, vptr 00CECCB4) is routed through the
// player's +13Ch object (0077C2A0(&msg, 0, 0)); no destructor runs. With a
// target (+B0h): a descriptor from 00465080(target, 0.0f) goes to 0077D600(
// player, command, &desc, 0); then the controlled vehicle (004B4B00), when
// kind 0Fh, gets 007BB170([game+18ECh], float [00F876A4]). Without a target
// the descriptor is {kind 0, byte 1 = 1 when the HUD [00E198C4] byte +FCh is
// set else 0, id 0, null, point, 0.0f}: point = HUD +F0h/+F8h with y = the
// player's +100h (pose refreshed first when +C8h is clear), or tuning +210h -
// 300.0 (rounded to float) when the player is kind 18h; with +FCh clear the
// point is the cursor 00F87574..00F8757C. AL = 1 on every non-null path.
bool unit_pick_issue_command_to_target_005241d0(Raw screen, void* command, SweepA2Host& host) {
    void* player = host.global_pointer(kPlayer);
    if (player == nullptr || command == nullptr) return false;
    const Raw game{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
    alignas(4) std::uint8_t message_storage[0x40];  // frame ESP+18h..ESP+57h
    const Raw message{message_storage};
    if (game.at<std::uint8_t>(0x19c4) != 0) {
        host.session_message_construct_base_0075b430(message, 0x57);
        message.at<std::uint16_t>(0x18) = 0;
        message.at<std::uint8_t>(0x1a) = 0;
        message.at<std::int32_t>(4) = 1;
        message.at<std::uint8_t>(0x1c) = 0;
        message.at<std::uint32_t>(0) = 0x00ceccb4u;
        void* const session = host.vcall_player_session_13c(host.global_pointer(kPlayer));
        host.session_route_message_0077c2a0(session, message, 0, 0);
        player = host.global_pointer(kPlayer);
    }
    if (void* const target = screen.ptr(0xb0); target != nullptr) {
        const Raw descriptor = host.command_target_from_entity_00465080(message, target, 0.0f);
        host.entity_issue_command_0077d600(host.global_pointer(kPlayer), command, descriptor.p, 0);
        void* const vehicle = host.game_get_controlled_vehicle_004b4b00();
        if (vehicle != nullptr && host.vcall_entity_is_kind_5c(vehicle, 0xf)) {
            const Raw g{static_cast<std::uint8_t*>(host.global_pointer(kGame))};
            host.plane_set_pilot_command_last_time_007bb170(vehicle, g.at<std::int32_t>(0x18ec),
                                                            *static_cast<const float*>(host.global_address(0x00f876a4)));
        }
        return true;
    }
    const Raw hud{static_cast<std::uint8_t*>(host.global_pointer(0x00e198c4))};
    alignas(4) std::uint8_t descriptor_storage[0x18];
    const Raw descriptor{descriptor_storage};
    if (hud.at<std::uint8_t>(0xfc) != 0) {
        const float x = hud.at<float>(0xf0);
        const float z = hud.at<float>(0xf8);
        const Raw p{static_cast<std::uint8_t*>(player)};
        if (p.at<std::uint8_t>(0xc8) == 0) host.entity_pose_refresh_world_00414db0(player);
        float y = p.at<float>(0x100);
        if (host.vcall_entity_is_kind_5c(host.global_pointer(kPlayer), 0x18)) {
            const Raw tuning{static_cast<std::uint8_t*>(host.game_tuning_get_singleton_0042e740())};
            y = static_cast<float>(static_cast<double>(tuning.at<float>(0x210)) - 300.0);  // 00CE3CA8
        }
        descriptor.at<float>(8) = x;
        descriptor.at<float>(0xc) = y;
        descriptor.at<std::uint16_t>(0) = 0;
        descriptor.at<float>(0x10) = z;
        descriptor.at<std::uint8_t>(1) = 1;
        descriptor.ptr(4) = nullptr;
        descriptor.at<std::uint8_t>(0) = 0;
        descriptor.at<std::uint16_t>(2) = 0;
        descriptor.at<float>(0x14) = 0.0f;
    } else {
        const float* const cursor = static_cast<const float*>(host.global_address(0x00f87574));
        descriptor.at<float>(8) = cursor[0];
        descriptor.at<float>(0xc) = cursor[1];
        descriptor.at<float>(0x10) = cursor[2];
        descriptor.at<std::uint16_t>(0) = 0;
        descriptor.ptr(4) = nullptr;
        descriptor.at<std::uint8_t>(0) = 0;
        descriptor.at<std::uint16_t>(2) = 0;
        descriptor.at<float>(0x14) = 0.0f;
    }
    host.entity_issue_command_0077d600(host.global_pointer(kPlayer), command, descriptor.p, 0);
    return true;
}

}  // namespace bsp::cc13_sweepA2
