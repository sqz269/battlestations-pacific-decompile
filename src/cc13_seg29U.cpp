// cc13 segment 29 cc9-lane leftovers (packet cc13-seg29U). See include/bsp/cc13_seg29U.hpp for
// the model and docs/CC13_SEG29U.md for the evidence. Names are hypotheses, not recovered symbols.

#include "bsp/cc13_seg29U.hpp"

#include <cmath>
#include <cstddef>

namespace bsp::cc13_seg29U {

namespace {

// The inlined NativeString destructor: when data is set, 00419CC0 then 00BD1510(data,
// length + 1, 1).
void release_string(Seg29UHost& host, char* data, std::uint32_t length) {
    if (data != nullptr) {
        void* const pool = host.sized_pool_00419cc0();
        host.sized_pool_return_00bd1510(pool, data, length + 1, 1);
    }
}

// The inlined std::list clear on the list object at `list` (head node pointer +4h, size +8h):
// unlink the head, zero the size, then free every former node with _free.
void list_clear(Seg29UHost& host, void* list) {
    void* head = ptr(list, 0x04);
    void* node = ptr(head, 0x00);
    at<void*>(head, 0x00) = head;
    head = ptr(list, 0x04);
    at<void*>(head, 0x04) = head;
    const bool empty = node == ptr(list, 0x04);
    at<std::uint32_t>(list, 0x08) = 0;
    if (!empty) {
        do {
            void* const next = ptr(node, 0x00);
            host.free_00bf65ac(node);
            node = next;
        } while (node != ptr(list, 0x04));
    }
}

// The four optional entity lookups of 006ACDC0 (006ACF78.., 006ACFAF.., 006ACFEA..): the
// property's string names an entity; anything that is missing or not IsKindOf(47h) is null.
void* find_path_entity(Seg29UHost& host, void* property) {
    const char* const name = host.property_string_value_008f0df0(property);
    void* const registry = ptr(host.global_pointer(kGameObjectGlobal), 0x19CC);
    void* const entity = host.entity_registry_find_00925a90(registry, name);
    if (entity == nullptr || !host.entity_is_kind_of_5c(entity, kPathEntityKind)) return nullptr;
    return entity;
}

}  // namespace

// ---------------------------------------------------------------------------
// 006AC370, __thiscall(ECX = landing point; ship point) RET 4. The list object is at +1FCh
// (head +200h). Each iteration re-reads the head; the checked-iterator tests (node == head ->
// 00BF6713) at 006AC39C, 006AC447 and 006AC468 are kept (the container test at 006AC388
// compares the list with itself and never fires). Node +8h holds the 18h-byte record whose
// +0h is the path entity.
// ---------------------------------------------------------------------------
void landing_point_update_paths_006ac370(void* point, void* ship_point, Seg29UHost& host) {
    void* const list = add(point, 0x1FC);
    void* node = ptr(ptr(point, 0x200), 0x00);
    alignas(4) std::uint8_t local_vec[12];        // ESP+10h
    alignas(4) std::uint8_t local_inverse[64];    // ESP+1Ch
    alignas(4) std::uint8_t local_product[64];    // ESP+5Ch
    while (node != ptr(list, 0x04)) {
        if (node == ptr(list, 0x04)) host.invalid_parameter_00bf6713();
        void* const entity = ptr(ptr(node, 0x08), 0x00);
        void* world;
        if (at<std::uint8_t>(entity, 0xC8) != 0) {
            world = add(entity, 0xCC);
        } else {
            if (ptr(entity, 0x3C) != nullptr) host.entity_pose_refresh_00414db0(ptr(entity, 0x3C));
            void* source;
            void* const parent = ptr(entity, 0x3C);
            if (parent != nullptr) {
                source = host.matrix_multiply_00413920(add(entity, 0x74), local_product,
                                                       add(parent, 0xCC));     // 006AC3DD
            } else {
                source = add(entity, 0x74);
            }
            host.matrix_copy_004134f0(add(entity, 0xCC), source);              // 006AC3F0
            at<std::uint8_t>(entity, 0xC8) = 1;
            at<std::uint8_t>(entity, 0x10C) = 0;
            world = add(entity, 0xCC);
        }
        host.matrix_inverse_00b63d50(local_inverse, world);                    // 006AC409
        void* const path = add(entity, 0x1E4);
        void* const local = host.transform_point_004142e0(ship_point, local_vec,
                                                          local_inverse);      // 006AC42B
        host.object_vslot_04(path, 0, local);                                  // 006AC438
        if (at<std::uint8_t>(path, 0x18) == 0) host.path_derive_knots_007af150(path);
        if (node == ptr(list, 0x04)) host.invalid_parameter_00bf6713();
        void* const owner = ptr(host.global_pointer(kGameObjectGlobal), 0x21D0);
        host.game_21d0_register_0048ca60(owner, ptr(node, 0x08), 1);           // 006AC463
        if (node == ptr(list, 0x04)) host.invalid_parameter_00bf6713();
        node = ptr(node, 0x00);
    }
}

// ---------------------------------------------------------------------------
// 006ACB40 BSP_LandingPoint_Destruct, __fastcall(ECX = landing point) RET. Vtables first, then
// the record list at +1FCh: every node's +8h record is _free'd and zeroed (checked-iterator
// tests kept), the list is cleared twice (the clear and the destructor's own clear are both
// inlined), its head is _free'd and +200h zeroed. Then the callback owner at +1E4h (vtable
// 00CE74FC; 006952A0 when +1F8h is set, 00695870) and 009287B0 on the entity.
// ---------------------------------------------------------------------------
void landing_point_destruct_006acb40(void* point, Seg29UHost& host) {
    at<std::uint32_t>(point, 0x000) = kLandingPointVtable;
    at<std::uint32_t>(point, 0x010) = kLandingPointVtable10;
    at<std::uint32_t>(point, 0x024) = kLandingPointVtable24;
    at<std::uint32_t>(point, 0x170) = kLandingPointVtable170;
    void* const list = add(point, 0x1FC);
    void* node = ptr(ptr(point, 0x200), 0x00);
    while (node != ptr(list, 0x04)) {
        if (node == ptr(list, 0x04)) host.invalid_parameter_00bf6713();       // 006ACBAC
        if (ptr(node, 0x08) != nullptr) {
            host.free_00bf65ac(ptr(node, 0x08));
            at<void*>(node, 0x08) = nullptr;
        }
        if (node == ptr(list, 0x04)) host.invalid_parameter_00bf6713();       // 006ACBCD
        node = ptr(node, 0x00);
    }
    list_clear(host, list);                                                    // 006ACBD6
    list_clear(host, list);                                                    // 006ACC02
    host.free_00bf65ac(ptr(list, 0x04));                                       // 006ACC36
    at<void*>(list, 0x04) = nullptr;
    void* const owner = add(point, 0x1E4);
    at<std::uint32_t>(owner, 0x00) = kCallbackOwnerVtable;
    if (ptr(owner, 0x14) != nullptr) host.observer_unregister_pair_006952a0(ptr(owner, 0x14), owner);
    host.callback_owner_destroy_00695870(owner);
    host.game_entity_destruct_009287b0(point);
}

// ---------------------------------------------------------------------------
// 006ACDC0, LandingPoint slot A4h, __thiscall(ECX = landing point) RET. 009295B0 first, then for
// i = 1..3 (006AD05D: JLE 3) the key "Paths" (00CF8228) + i. A property of type 6 (+4h) in the
// entity's bag ([[this+C0h]+8h]) holds a sub-bag (+0Ch) that must have "Path" (00CEA738); its
// entity, when IsKindOf(47h), gets an 18h-byte record (operator new, +10h/+14h zeroed, no null
// test on the stores) holding Path / PathEndZ (00CF821C) / PathEnemy (00CF8210) /
// PathEnemyEndZ (00CF8200) entities at +0h/+4h/+8h/+0Ch, pushed onto the list at +1FCh. A
// missing optional key leaves its dword as operator new returned it. IsKindOf is called twice
// on the Path entity (006ACF3B and 006ACF6A); the second answer selects entity or null.
// ---------------------------------------------------------------------------
void landing_point_read_paths_006acdc0(void* point, Seg29UHost& host) {
    host.entity_default_slot_a4_009295b0(point);
    static const char kPaths[] = "Paths";                                      // 00CF8228
    for (std::int32_t i = 1; i <= 3; ++i) {
        NativeStr number{};                                                    // ESP+30h
        NativeStr* const number_ptr = host.native_string_from_int_004260b0(&number, i);
        NativeStr paths{0, nullptr};                                           // ESP+20h
        host.native_string_resize_0041dd40(&paths, 5, 1);
        char* const paths_data = paths.data;
        const std::uint32_t paths_length = paths.length;
        if (paths_data != nullptr) host.memcpy_00bf7680(paths_data, kPaths, paths_length + 1);
        NativeStr key{};                                                       // ESP+28h
        host.native_string_concat_004261a0(&paths, &key, number_ptr);          // 006ACE58
        release_string(host, paths_data, paths_length);
        release_string(host, number.data, number.length);
        const char* const key_text = key.data != nullptr
            ? key.data : static_cast<const char*>(host.global_address(kEmptyCString));
        void* const bag = ptr(ptr(point, 0xC0), 0x08);
        void* const group = host.property_bag_find_008f2260(bag, key_text);   // 006ACEB4
        if (group != nullptr && at<std::int32_t>(group, 0x04) == 6) {
            void* const sub = ptr(group, 0x0C);
            void* const path = host.property_bag_find_008f2260(sub, "Path");
            if (path != nullptr) {
                void* const end_z = host.property_bag_find_008f2260(sub, "PathEndZ");
                void* const enemy = host.property_bag_find_008f2260(sub, "PathEnemy");
                void* const enemy_end_z = host.property_bag_find_008f2260(sub, "PathEnemyEndZ");
                const char* const name = host.property_string_value_008f0df0(path);
                void* const registry = ptr(host.global_pointer(kGameObjectGlobal), 0x19CC);
                void* const entity = host.entity_registry_find_00925a90(registry, name);
                if (entity != nullptr && host.entity_is_kind_of_5c(entity, kPathEntityKind)) {
                    void* record = host.operator_new_00bf681b(0x18);
                    if (record != nullptr) {
                        at<std::uint32_t>(record, 0x14) = 0;
                        at<std::uint32_t>(record, 0x10) = 0;
                    }
                    const bool kind = host.entity_is_kind_of_5c(entity, kPathEntityKind);
                    at<void*>(record, 0x00) = kind ? entity : nullptr;
                    if (end_z != nullptr) at<void*>(record, 0x04) = find_path_entity(host, end_z);
                    if (enemy != nullptr) at<void*>(record, 0x08) = find_path_entity(host, enemy);
                    if (enemy_end_z != nullptr) {
                        at<void*>(record, 0x0C) = find_path_entity(host, enemy_end_z);
                    }
                    host.list_push_back_006acd80(add(point, 0x1FC), &record);   // 006AD02A
                }
            }
        }
        release_string(host, key.data, key.length);
    }
}

// ===========================================================================
// Air ops, 006BA620 and up.
// ===========================================================================

namespace {

// The block's tracked lock at block+8h: EnterCriticalSection then +18h += 1, and the reverse.
// A null lock is skipped, as every inlined copy does.
void lock_enter(Seg29UHost& host, void* section) {
    if (section != nullptr) {
        host.enter_critical_section(section);
        at<std::int32_t>(section, 0x18) += 1;
    }
}
void lock_leave(Seg29UHost& host, void* section) {
    if (section != nullptr) {
        at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section(section);
    }
}

// The landing-record vector at block+A4h (begin +A8h, end +ACh, 14h-byte records). The
// element count is the signed (end - begin) / 14h (IMUL 66666667h, SAR 3, sign fix-up); the
// loops compare it unsigned (JAE / JB).
std::uint32_t landing_record_count(void* block) {
    auto* const begin = static_cast<std::uint8_t*>(ptr(block, 0xA8));
    auto* const end = static_cast<std::uint8_t*>(ptr(block, 0xAC));
    return static_cast<std::uint32_t>(static_cast<std::int32_t>(end - begin) /
                                      static_cast<std::int32_t>(kLandingRecordStride));
}
bool landing_index_valid(void* block, std::uint32_t index) {
    return ptr(block, 0xA8) != nullptr && index < landing_record_count(block);
}
void* landing_record(void* block, std::uint32_t index) {
    return add(ptr(block, 0xA8), index * kLandingRecordStride);
}

// The 14h-byte answer 006BCFD0 and 006BD080 fill: +0h word = plane+174h, +4h/+8h the record's
// floats, +0Ch = record+0Dh and +0Dh = record+0Ch (the two bytes cross), +10h the record dword.
void copy_landing_answer(void* out, void* record, void* plane) {
    at<std::uint16_t>(out, 0x00) = at<std::uint16_t>(plane, 0x174);
    at<float>(out, 0x04) = at<float>(record, 0x04);
    at<float>(out, 0x08) = at<float>(record, 0x08);
    at<std::uint8_t>(out, 0x0C) = at<std::uint8_t>(record, 0x0D);
    at<std::uint8_t>(out, 0x0D) = at<std::uint8_t>(record, 0x0C);
    at<std::uint32_t>(out, 0x10) = at<std::uint32_t>(record, 0x10);
}

// 006BCD80 / 006BCE60: every record whose +0h is the plane gets +0Ch = value. Each access
// repeats the checked-index test (00BF6713 when the vector is gone or the index is past it).
void set_landing_records_held(void* block, void* plane, std::uint8_t value, Seg29UHost& host) {
    void* const section = ptr(block, 0x08);
    lock_enter(host, section);
    for (std::uint32_t i = 0; landing_index_valid(block, i); ++i) {
        if (!landing_index_valid(block, i)) host.invalid_parameter_00bf6713();
        if (ptr(landing_record(block, i), 0x00) == plane) {
            if (!landing_index_valid(block, i)) host.invalid_parameter_00bf6713();
            at<std::uint8_t>(landing_record(block, i), 0x0C) = value;
        }
    }
    lock_leave(host, section);
}

// The squadron entity handle registry (006BD62F.., 006BDBF7..): signed compare against the
// split, then row (id - base) * 10h, entity at row +0Ch.
void* resolve_entity_handle(Seg29UHost& host, std::uint16_t id) {
    const auto dword = [&host](std::uint32_t address) {
        return *static_cast<std::int32_t*>(host.global_address(address));
    };
    const std::int32_t value = id;
    std::int32_t index;
    std::uint8_t* rows;
    if (value < dword(kHandleSplit)) {
        index = value - dword(kHandleBase0);
        rows = static_cast<std::uint8_t*>(host.global_pointer(kHandleRows0));
    } else {
        index = value - dword(kHandleBase1);
        rows = static_cast<std::uint8_t*>(host.global_pointer(kHandleRows1));
    }
    return ptr(rows + static_cast<std::ptrdiff_t>(index) * 0x10, 0x0C);
}

// The inlined tail of every AIRBASE message reader constructor after 0075B430: +18h word,
// +1Ah byte and +1Ch byte cleared, +4h = 1, then the message's own vtable.
void airbase_message_base(void* message, std::uint32_t type, Seg29UHost& host) {
    host.session_message_construct_base_0075b430(message, type);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
}

void* slot_record(void* block, std::uint32_t slot) {
    return add(ptr(block, 0x4C), slot * kAirOpsSlotStride);
}

}  // namespace

// 006BA670: when [block+3Ch] is set, its vtable +08h with dt (FLD/FSTP copies the float).
void air_ops_block_tick_006ba670(void* block, float dt, Seg29UHost& host) {
    if (ptr(block, 0x3C) != nullptr) host.object_vslot_08_float(ptr(block, 0x3C), dt);
}

// 006BC030: vtable +0Ch is 1 or 2 -> 1, else 0.
std::int32_t entity_kind_is_1_or_2_006bc030(void* entity, Seg29UHost& host) {
    const std::int32_t kind = host.object_vslot_0c(entity);
    return kind == 1 || kind == 2 ? 1 : 0;
}

// 006BC120: 1 when the owner [block+7Ch] is null or its torn-down byte +5Dh is set.
std::int32_t air_ops_block_owner_missing_006bc120(void* block) noexcept {
    void* const owner = ptr(block, 0x7C);
    if (owner == nullptr || at<std::uint8_t>(owner, 0x5D) != 0) return 1;
    return 0;
}

// 006BC3E0: 0075B430(89h), the common clears, vtable 00CF85F0.
void* do_launch_slot_message_construct_006bc3e0(void* message, Seg29UHost& host) {
    host.session_message_construct_base_0075b430(message, 0x89);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = kDoLaunchSlotVtable;
    return message;
}

// ---------------------------------------------------------------------------
// 006BC5E0, __thiscall(ECX = squadron) -> AL. Null squadron -> false. The command list
// (vtable +114h) must be on moveto (0071BE40 == 00E08F68). Refresh the pose when +C8h is clear
// (00414DB0), then d = squadron+FCh..+104h - the target position (0071EB60 -> 006F7DD0), each
// difference stored as a float. True when float(class+268h x 1.2 [00CEC160 double]) > |d|
// (FCOMI ST1; JBE takes <= and unordered to false). The product is modelled at the main
// thread's 24-bit precision (one rounding to float); at 53 bits it would be compared as a
// double, which can only differ when |d| falls between the two roundings.
// ---------------------------------------------------------------------------
bool squadron_target_within_range_006bc5e0(void* squadron, Seg29UHost& host) {
    if (squadron == nullptr) return false;
    void* const commands = host.object_vslot_114(squadron);
    if (host.current_command_0071be40(commands) != kMoveToCommandClass) return false;
    if (at<std::uint8_t>(squadron, 0xC8) == 0) host.entity_pose_refresh_00414db0(squadron);
    alignas(4) std::uint8_t descriptor_buffer[12];    // ESP+14h
    void* const descriptor = host.active_target_descriptor_0071eb60(commands, descriptor_buffer);
    const float* const target = host.target_descriptor_position_006f7dd0(descriptor);
    const float d[3] = {at<float>(squadron, 0xFC) - target[0],
                        at<float>(squadron, 0x100) - target[1],
                        at<float>(squadron, 0x104) - target[2]};
    const float length = host.vector3_length_0042b2f0(d);
    const float range = at<float>(ptr(squadron, 0x35C), 0x268);
    const float reach = static_cast<float>(static_cast<double>(range) * kHoldRangeScale);
    return reach > length;
}

// 006BC690: vtable 00CF6564, then 00653400 on +38h and 00653390 on +14h.
void air_ops_slot_record_destruct_006bc690(void* record, Seg29UHost& host) {
    at<std::uint32_t>(record, 0x00) = kAirOpsSlotRecordVtable;
    host.observer_ref_destruct_00653400(add(record, 0x38));
    host.observer_member_cleanup_00653390(add(record, 0x14));
}

// 006BC6F0: a new class sets +4h and +10h = class+134h (0 for a null class); +8h = count on
// every path, also when the class is unchanged.
void air_ops_assign_slot_class_006bc6f0(void* slot, void* vehicle_class, std::int32_t count) noexcept {
    if (ptr(slot, 0x04) != vehicle_class) {
        at<void*>(slot, 0x04) = vehicle_class;
        at<std::uint32_t>(slot, 0x10) =
            vehicle_class != nullptr ? at<std::uint32_t>(vehicle_class, 0x134) : 0u;
    }
    at<std::int32_t>(slot, 0x08) = count;
}

// 006BC730: +8h = 007EE5C0(+28h squadron), 0 without one; a zero count sets state +2Ch = 1.
void air_ops_slot_refresh_airborne_006bc730(void* slot, Seg29UHost& host) {
    void* const squadron = ptr(slot, 0x28);
    at<std::int32_t>(slot, 0x08) =
        squadron != nullptr ? host.squadron_airborne_count_007ee5c0(squadron) : 0;
    if (at<std::int32_t>(slot, 0x08) == 0) at<std::int32_t>(slot, 0x2C) = 1;
}

// 006BC800: the slot's squadron is on moveto and 006BC5E0 says it is not within range.
bool air_ops_slot_squadron_moving_out_of_range_006bc800(void* slot, Seg29UHost& host) {
    void* const squadron = ptr(slot, 0x28);
    if (squadron == nullptr) return false;
    void* const commands = host.object_vslot_114(squadron);
    if (host.current_command_0071be40(commands) != kMoveToCommandClass) return false;
    return !squadron_target_within_range_006bc5e0(ptr(slot, 0x28), host);
}

// 006BC870: _free (00BF6989) the +14h buffer when set (the field is not cleared), then the
// tail jump to 00695870.
void buffered_callback_owner_destruct_006bc870(void* owner, Seg29UHost& host) {
    if (ptr(owner, 0x14) != nullptr) host.free_00bf6989(ptr(owner, 0x14));
    host.callback_owner_destroy_00695870(owner);
}

// 006BC890: the traffic object's vtable +44h(out), else the three floats at 00F87574.
float* air_ops_block_runway_end_point_006bc890(void* block, float* out, Seg29UHost& host) {
    if (ptr(block, 0x3C) != nullptr) {
        host.object_vslot_44(ptr(block, 0x3C), out);
        return out;
    }
    const float* const fallback = static_cast<const float*>(host.global_address(kRunwayEndFallback));
    out[0] = fallback[0];
    out[1] = fallback[1];
    out[2] = fallback[2];
    return out;
}

// 006BC8E0: a 24h-byte DOLAUNCHSLOT (89h) message on the stack, +20h = slot, routed through
// 0077C2A0(ECX = [block+7Ch]; message, 1, 0). No destructor runs on it afterwards.
void air_ops_route_do_launch_slot_006bc8e0(void* block, std::uint32_t slot, Seg29UHost& host) {
    alignas(4) std::uint8_t message[0x24];            // ESP+4h
    host.session_message_construct_base_0075b430(message, 0x89);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x00) = kDoLaunchSlotVtable;
    at<std::uint32_t>(message, 0x20) = slot;
    host.session_route_message_0077c2a0(ptr(block, 0x7C), message, 1, 0);
}

// ---------------------------------------------------------------------------
// 006BC960, __thiscall(ECX = holder) RET. h = float(-holder+B4h x 0.5 [00D7A280 double]).
// Owner [[holder+4]+7Ch] IsKindOf(9) (a mother ship): x = 0, z = float(h + 10.0 [00CE3DC0]).
// Otherwise x = float((double(holder+B0h) - [holder+4]->vtable[10h]()) x 0.5), clamped to 0
// when 0 > x (FCOMI; JBE keeps x, so NaN stays), z = float(h + 35.0 [00CF8608]). Then +A4h = x,
// +ACh = z, +A8h = 0.5f (00CE3800). 24-bit model: the difference rounds to float before the
// exact halving.
// ---------------------------------------------------------------------------
void air_ops_holder_refresh_touchdown_006bc960(void* holder, Seg29UHost& host) {
    const float half_length = static_cast<float>(-static_cast<double>(at<float>(holder, 0xB4)) * kHalf);
    void* const block = ptr(holder, 0x04);
    float x;
    float z;
    if (host.entity_is_kind_of_5c(ptr(block, 0x7C), 9)) {
        x = 0.0f;
        z = static_cast<float>(static_cast<double>(half_length) + kMotherShipTouchdownOffset);
    } else {
        const double width = at<float>(holder, 0xB0);
        const float diff = static_cast<float>(width - host.object_vslot_10_x87(block));
        const float t = static_cast<float>(static_cast<double>(diff) * kHalf);
        x = 0.0f > t ? 0.0f : t;
        z = static_cast<float>(static_cast<double>(half_length) + kAirfieldTouchdownOffset);
    }
    at<float>(holder, 0xA4) = x;
    at<float>(holder, 0xAC) = z;
    at<float>(holder, 0xA8) = kTouchdownHeight;
}

// 006BCA40: T (+A4h) through the holder frame (+8h), 004142E0, copied to out.
float* air_ops_holder_touchdown_world_006bca40(void* holder, float* out, Seg29UHost& host) {
    float local[3];
    host.transform_point_004142e0(add(holder, 0xA4), local, add(holder, 0x08));
    out[0] = local[0];
    out[1] = local[1];
    out[2] = local[2];
    return out;
}

// ---------------------------------------------------------------------------
// 006BCA80 BSP_AirOpsHolder_PathPoint, __thiscall(ECX = holder; float out[4], float t) RET 8.
// a = float(holder+8Ch x t). x, z = holder+A4h, +ACh, copied before the clamp holder+ACh =
// min(+ACh, -20.0f) (COMISS; JBE: no store for <= or unordered). Unless a == 0 exactly (UCOMISS,
// LAHF/TEST 44h/JNP: NaN rotates), T is turned in polar form: r2 = float(float(x*x) +
// float(z*z)), r = r2 > 1e-10 (FCOMI, JBE -> 0) ? float(_CIsqrt r2) : 0; phi = float(_CIatan2(z,
// x)); b = float(pi/2 - phi) (+2pi when 0 > b); c = float(pi/2 - 00438AA0(b, a)) (+2pi when
// 0 > c); x = float(r cos c), z = float(r sin c) (FCOS/FSIN at extended precision). Then
// (x, holder+A8h, z) through the frame +8h (004142E0); out[i] = float(p[i] + float(v[i] x t))
// with v = [[holder+4]+7Ch]->vtable[34h](scratch); out[3] = 00438AA0(holder+88h, a). The x87
// chain is modelled at the main thread's 24-bit precision (each operation rounded to float).
// ---------------------------------------------------------------------------
void air_ops_holder_path_point_006bca80(void* holder, float* out, float t, Seg29UHost& host) {
    float x = at<float>(holder, 0xA4);
    const float a = at<float>(holder, 0x8C) * t;
    float z = at<float>(holder, 0xAC);
    if (z > kTouchdownZCeiling) at<float>(holder, 0xAC) = kTouchdownZCeiling;
    if (!(a == 0.0f)) {
        const float r2 = x * x + z * z;
        float r;
        if (static_cast<double>(r2) > kPolarEpsilon) {
            r = static_cast<float>(host.sqrt_00bf7030(r2));
        } else {
            r = 0.0f;
        }
        const float phi = static_cast<float>(host.atan2_00bf701a(z, x));
        float b = static_cast<float>(kHalfPi - static_cast<double>(phi));
        if (0.0f > b) b = static_cast<float>(static_cast<double>(b) + kTwoPi);
        float c = static_cast<float>(kHalfPi - static_cast<double>(host.add_wrapped_angle_00438aa0(b, a)));
        if (0.0f > c) c = static_cast<float>(static_cast<double>(c) + kTwoPi);
        const float cos_c = static_cast<float>(std::cos(static_cast<double>(c)));
        const float sin_c = static_cast<float>(std::sin(static_cast<double>(c)));
        x = r * cos_c;
        z = r * sin_c;
    }
    const float local[3] = {x, at<float>(holder, 0xA8), z};
    float world[3];
    host.transform_point_004142e0(const_cast<float*>(local), world, add(holder, 0x08));
    float scratch[3];
    void* const owner = ptr(ptr(holder, 0x04), 0x7C);
    const float* const v = host.object_vslot_34(owner, scratch);
    const float lead[3] = {v[0] * t, v[1] * t, v[2] * t};
    out[0] = lead[0] + world[0];
    out[1] = lead[1] + world[1];
    out[2] = lead[2] + world[2];
    out[3] = host.add_wrapped_angle_00438aa0(at<float>(holder, 0x88), a);
}

// 006BCCE0: point through the inverse frame at holder+48h (004142E0), copied to out.
float* air_ops_holder_to_local_006bcce0(void* holder, float* out, void* point, Seg29UHost& host) {
    float local[3];
    host.transform_point_004142e0(point, local, add(holder, 0x48));
    out[0] = local[0];
    out[1] = local[1];
    out[2] = local[2];
    return out;
}

void air_ops_block_hold_landing_record_006bcd80(void* block, void* plane, Seg29UHost& host) {
    set_landing_records_held(block, plane, 1, host);
}

void air_ops_block_release_landing_record_006bce60(void* block, void* plane, Seg29UHost& host) {
    set_landing_records_held(block, plane, 0, host);
}

// ---------------------------------------------------------------------------
// 006BCF40, __thiscall(ECX = scene property bag) RET. party = ["Party" (00CE5804)]+0Ch (no null
// test). For i = 1..12 (JLE 0Ch): "PlaneStock %d" (00CE5718) into a 20h-byte buffer
// (00467310); a type-6 property with a sub-bag names a class in its "Type" (00CE4780) +0Ch
// (no null test), which 0095BA60 marks for the party and 0046BE90 registers.
// ---------------------------------------------------------------------------
void air_ops_register_party_plane_stock_006bcf40(void* bag, Seg29UHost& host) {
    void* const party_property = host.property_bag_find_008f2260(bag, "Party");
    const std::uint32_t party = at<std::uint32_t>(party_property, 0x0C);
    for (std::int32_t i = 1; i <= 12; ++i) {
        char key[0x20];                                   // ESP+10h
        host.format_to_buffer32_00467310(key, "PlaneStock %d", i);
        void* const stock = host.property_bag_find_008f2260(bag, key);
        if (stock == nullptr || at<std::int32_t>(stock, 0x04) != 6) continue;
        void* const sub = ptr(stock, 0x0C);
        if (sub == nullptr) continue;
        void* const type = host.property_bag_find_008f2260(sub, "Type");
        void* const vehicle_class = ptr(type, 0x0C);
        host.vehicle_class_mark_party_required_0095ba60(vehicle_class, party);
        host.register_hidden_stock_unit_0046be90(host.global_pointer(kHiddenStockRegistry),
                                                 vehicle_class);
    }
}

// 006BCFD0: under the lock, a non-negative index inside the vector (JL / JAE) fills out from
// 006BA7B0's record (the plane is the record's +0h) and answers 1.
bool air_ops_block_read_queue_record_006bcfd0(void* block, std::int32_t index, void* out,
                                             Seg29UHost& host) {
    void* const section = ptr(block, 0x08);
    lock_enter(host, section);
    bool found = false;
    if (index >= 0 && landing_index_valid(block, static_cast<std::uint32_t>(index))) {
        void* const record = host.landing_vector_at_006ba7b0(add(block, 0xA4), index);
        copy_landing_answer(out, record, ptr(record, 0x00));
        found = true;
    }
    lock_leave(host, section);
    return found;
}

// 006BD080 BSP_AirOps_FindLandingAssignment: under the lock, the first record whose +0h is the
// plane fills out (checked index tests kept) and answers 1; a miss leaves out untouched.
bool air_ops_find_landing_assignment_006bd080(void* block, void* plane, void* out,
                                             Seg29UHost& host) {
    void* const section = ptr(block, 0x08);
    lock_enter(host, section);
    bool found = false;
    for (std::uint32_t i = 0; landing_index_valid(block, i); ++i) {
        if (!landing_index_valid(block, i)) host.invalid_parameter_00bf6713();
        if (ptr(landing_record(block, i), 0x00) != plane) continue;
        found = true;
        if (!landing_index_valid(block, i)) host.invalid_parameter_00bf6713();
        copy_landing_answer(out, landing_record(block, i), plane);
        break;
    }
    lock_leave(host, section);
    return found;
}

// ---------------------------------------------------------------------------
// 006BD230, __thiscall(ECX = block; slot, command) RET 8. The owner's (+7Ch) vtable +114h
// object, when non-null, gives the sender through its vtable +18h. The slot (block+4Ch +
// slot x 58h) is "ready" when ((state +2Ch is 1 or 5) and +8h > 0, signed) or state is 4.
// command 0 -> 3 ready / 2 not; command 1 -> 1 ready / 2 not; other commands -> 0. A 30h-byte
// 82h message on the stack: +20h slot, +24h answer, +28h word = sender+174h (0 without one),
// +2Ch = 9, vtable 00CF64C4; routed 0077C2A0(message, 0, 0).
// ---------------------------------------------------------------------------
void air_ops_slot_state_from_command_006bd230(void* block, std::uint32_t slot,
                                             std::int32_t command, Seg29UHost& host) {
    void* const holder = host.object_vslot_114(ptr(block, 0x7C));
    void* const sender = holder != nullptr ? host.object_vslot_18(holder) : nullptr;
    std::int32_t answer = 0;
    if (command == 0 || command == 1) {
        void* const record = slot_record(block, slot);
        const std::int32_t state = at<std::int32_t>(record, 0x2C);
        const bool ready = ((state == 1 || state == 5) && at<std::int32_t>(record, 0x08) > 0) ||
                           state == 4;
        answer = ready ? (command == 0 ? 3 : 1) : 2;
    }
    alignas(4) std::uint8_t message[0x30];            // ESP+0Ch after the frame
    host.session_message_construct_base_0075b430(message, 0x82);
    at<std::uint32_t>(message, 0x20) = slot;
    at<std::int32_t>(message, 0x24) = answer;
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x00) = kSlotStateMessageVtable;
    at<std::uint16_t>(message, 0x28) =
        sender != nullptr ? at<std::uint16_t>(sender, 0x174) : static_cast<std::uint16_t>(0);
    at<std::uint32_t>(message, 0x2C) = 9;
    host.session_route_message_0077c2a0(ptr(block, 0x7C), message, 0, 0);
}

// 006BD620: handle +34h; 0 -> null. The resolved entity is returned when it is non-null and
// neither +5Dh nor +5Eh is set; otherwise +24h = 5, the handle is cleared and null returned.
void* air_ops_slot_check_squadron_handle_006bd620(void* record, Seg29UHost& host) {
    const std::uint16_t id = at<std::uint16_t>(record, 0x34);
    if (id == 0) return nullptr;
    void* const entity = resolve_entity_handle(host, id);
    if (entity != nullptr && at<std::uint8_t>(entity, 0x5D) == 0 &&
        at<std::uint8_t>(entity, 0x5E) == 0) {
        return entity;
    }
    at<std::int32_t>(record, 0x24) = 5;
    at<std::uint16_t>(record, 0x34) = 0;
    return nullptr;
}

// 006BD800: 0075B430(81h) and the common clears; +20h = slot, vtable 00CF8624, then from the
// slot record: +24h = 0095B9C0(record+4h class), +28h = record+8h, +2Ch = record+10h.
void* set_slot_message_construct_006bd800(void* message, void* block, std::uint32_t slot,
                                          Seg29UHost& host) {
    airbase_message_base(message, 0x81, host);
    at<std::uint32_t>(message, 0x20) = slot;
    at<std::uint32_t>(message, 0x00) = kSetSlotVtable;
    void* const record = slot_record(block, slot);
    at<std::uint32_t>(message, 0x24) = host.vehicle_class_registry_id_0095b9c0(ptr(record, 0x04));
    at<std::uint32_t>(message, 0x28) = at<std::uint32_t>(record, 0x08);
    at<std::uint32_t>(message, 0x2C) = at<std::uint32_t>(record, 0x10);
    return message;
}

// 006BD9E0: 0075B430(85h), the clears, vtable 00CF8638, +20h = 0095B9C0(class), +24h = value.
void* stock_item_message_construct_006bd9e0(void* message, void* vehicle_class,
                                            std::uint32_t value, Seg29UHost& host) {
    airbase_message_base(message, 0x85, host);
    at<std::uint32_t>(message, 0x00) = kStockItemVtable;
    at<std::uint32_t>(message, 0x20) = host.vehicle_class_registry_id_0095b9c0(vehicle_class);
    at<std::uint32_t>(message, 0x24) = value;
    return message;
}

// 006BDAC0: 0075B430(86h), the clears, vtable 00CF864C, +20h word = plane+174h.
void* appear_ready_plane_message_construct_006bdac0(void* message, void* plane, Seg29UHost& host) {
    host.session_message_construct_base_0075b430(message, 0x86);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0x00) = kAppearReadyPlaneVtable;
    at<std::uint16_t>(message, 0x20) = at<std::uint16_t>(plane, 0x174);
    return message;
}

// 006BDBF0: the entity the +20h handle names (no null or liveness test; id 0 is looked up).
void* message_resolve_unit_006bdbf0(void* message, Seg29UHost& host) {
    return resolve_entity_handle(host, at<std::uint16_t>(message, 0x20));
}

}  // namespace bsp::cc13_seg29U
