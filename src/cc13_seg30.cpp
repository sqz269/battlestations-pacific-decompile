// Segment 30 reconstructions (packet cc13-seg30). See include/bsp/cc13_seg30.hpp for the model and
// docs/CC13_SEG30.md for evidence. Names are hypotheses, not recovered symbols.

#include "bsp/cc13_seg30.hpp"
#include "bsp/airfield_marker_seg31_32_argon.hpp"
#include <cstring>

#include <utility>

namespace bsp::cc13_seg30 {
void* air_ops_holder_at_006bf1b0(Seg30Host& host,void* block,std::uint32_t index) {
    void* node=ptr(ptr(block,0x44),0);
    void* const list=reinterpret_cast<void*>(reinterpret_cast<std::uintptr_t>(block)+0x40u);
    std::uint32_t i=0;
    for (;;) {
        void* const head=ptr(list,4);
        if (!list) host.invalid_iterator_00bf6713();
        if (node==head || index==i) break;
        if (!list) host.invalid_iterator_00bf6713();
        if (node==ptr(list,4)) host.invalid_iterator_00bf6713();
        node=ptr(node,0); ++i;
    }
    void* const head=ptr(list,4);
    if (!list) host.invalid_iterator_00bf6713();
    if (node==head) return nullptr;
    if (!list) host.invalid_iterator_00bf6713();
    if (node==ptr(list,4)) host.invalid_iterator_00bf6713();
    return ptr(node,8);
}
void archive_write_object_006c0020(Seg30Host& host,void* archive,ArchiveArg key,void* object) {
    host.archive_write_0c(archive,key,{4,reinterpret_cast<std::uintptr_t>(object)});
}
void* observer_scalar_delete_006ce290(Seg30Host& host,void* self,std::uint32_t flags) {
    at<std::uint32_t>(self,0)=0x00cf88f8;
    if (flags&1) host.free_00bf65ac(self);
    return self;
}
void* mothership_site_air_ops_006cfe10(void* self) noexcept {
    void* const owner=ptr(self,0xb8);
    return owner ? add(owner,0x1188) : nullptr;
}
void air_field_reset_slots_006d0c60(AirFieldLeafHost& host,void* self) {
    host.air_ops_reset_slots_006bd360(add(self,0x72c));
}
void air_field_ignore_006d1d50(std::uint32_t) noexcept {}
void unit_empty_hook_006d1e90() noexcept {}
bool unit_false_hook_006d1ed0(std::uint32_t) noexcept { return false; }
std::uint32_t unit_class_006d1fb0(void* self) noexcept { return at<std::uint32_t>(self,0xc4); }
void air_field_party_race_006d2070(AirFieldLeafHost& host,void* self,
    std::uint32_t a,std::uint32_t b,std::uint32_t* kind) {
    *kind=0xf;
    host.unit_party_race_00951f30(self,a,b,kind);
    auto* item=static_cast<std::uint8_t*>(ptr(self,0x830));
    auto* end=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(item)+
        static_cast<std::uint32_t>(at<std::uint32_t>(self,0x834)*12u));
    while (item!=end) {
        host.child_party_race_2c(ptr(item,0),a,b,kind);
        const auto count=at<std::uint32_t>(self,0x834);
        void* const base=ptr(self,0x830);
        item=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(item)+12u);
        end=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(base)+count*12u);
    }
}
namespace {
bool runway_failure_name(void* name) {
    const char* const text=at<const char*>(name,4);
    return text && _stricmp(text,"RunwayFailure")==0;
}
bool route_air_field_failure(AirFieldLeafHost& host,void* self,void* name,bool clear) {
    alignas(4) std::uint8_t message[0x24];
    if (runway_failure_name(name)) {
        if (clear) airbase_clear_runway_failure_construct_006d1720(host,message);
        else airbase_set_runway_failure_construct_006d16a0(host,message);
    } else if (host.native_string_equals_00425850(name,"HangarFailure")) {
        if (clear) airbase_clear_hangar_failure_construct_006d1820(host,message);
        else airbase_set_hangar_failure_construct_006d17a0(host,message);
    } else return false;
    host.route_message_0077c2a0(self,message,7,0);
    return true;
}
}
bool air_field_set_failure_006d2210(AirFieldLeafHost& host,void* self,void* name,std::uint32_t value) {
    if (route_air_field_failure(host,self,name,false)) return true;
    return host.unit_set_failure_00953da0(self,name,value);
}
bool air_field_clear_failure_006d23d0(AirFieldLeafHost& host,void* self,void* name,std::uint32_t value) {
    if (route_air_field_failure(host,self,name,true)) return true;
    return host.unit_clear_failure_00953ea0(self,name,value);
}
bool air_field_query_failure_006d2350(AirFieldLeafHost& host,void* self,void* name,
    std::uint32_t value,std::uint8_t* out) {
    if (runway_failure_name(name)) { *out=at<std::uint8_t>(self,0x748); return true; }
    if (host.native_string_equals_00425850(name,"HangarFailure")) {
        *out=at<std::uint8_t>(self,0x749); return true;
    }
    return host.unit_query_failure_00953e60(self,name,value,out);
}
void air_field_target_order_006d2850(AirFieldLeafHost& host,void* self,void* message,
    const NativeCommandTargetHandleTables& tables) {
    host.command_registry_find_0077a020(message);
    alignas(4) std::uint8_t target[0x18];
    std::memcpy(target,add(message,0x24),sizeof target);
    if (host.command_from_ordinal_0077a050(message)==host.image_address(0x00e08ef8)) {
        const auto object=resolve_native_command_target_00521ea0(target,tables);
        host.director_set_target_00835860(ptr(self,0x8c8),object,1);
        return;
    }
    if (host.command_from_ordinal_0077a050(message)==host.image_address(0x00e08f00) ||
        host.command_from_ordinal_0077a050(message)==host.image_address(0x00e08f08))
        host.director_set_target_00835860(ptr(self,0x8c8),0,1);
}
void air_field_forward_float_006d3eb0(AirFieldLeafHost& host,void* self,float value) {
    host.entity_float_0042ed80(self,value);
    auto* item=static_cast<std::uint8_t*>(ptr(self,0x830));
    auto* end=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(item)+
        static_cast<std::uint32_t>(at<std::uint32_t>(self,0x834)*12u));
    while (item!=end) {
        host.child_float_f4(ptr(item,0),value);
        const auto count=at<std::uint32_t>(self,0x834);
        void* const base=ptr(self,0x830);
        item=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(item)+12u);
        end=reinterpret_cast<std::uint8_t*>(reinterpret_cast<std::uintptr_t>(base)+count*12u);
    }
}
void air_field_send_create_006d60b0(Seg30Host& host,
    airfield_marker_seg31_32_argon::Seg31_32Host& message_host,void* self,std::uint32_t peer) {
    void* const source=ptr(self,0x7ac);
    const float b=source ? at<float>(source,0xb4) : 0.0f;
    const float a=source ? at<float>(source,0xb0) : 0.0f;
    alignas(4) std::uint8_t storage[0x148];
    using namespace airfield_marker_seg31_32_argon;
    void* const message=session_message_airfield_create_construct_006d4c70(message_host,storage,self,a,b);
    try { host.entity_send_message_to_peer_00779fc0(self,peer,message); }
    catch (...) { session_message_airfield_create_destruct_006d4420(message_host,storage); throw; }
    session_message_airfield_create_destruct_006d4420(message_host,storage);
}

namespace {

// The engine's tracked lock: EnterCriticalSection, then the +18h recursion counter.
void lock_enter(Seg30Host& host, void* section) {
    if (section != nullptr) {
        host.enter_critical_section(section);
        at<std::int32_t>(section, 0x18) += 1;
    }
}
void lock_leave(Seg30Host& host, void* section) {
    if (section != nullptr) {
        at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section(section);
    }
}

void* slot_at(void* block, int slot) {
    return add(ptr(block, kBlockSlots), static_cast<std::size_t>(slot) * kSlotStride);
}

void* observer_member_set_target(Seg30Host& host, void* member, void* target) {
    void* const current = ptr(member, 0x14);
    if (current != target) {
        if (current != nullptr) host.observer_unregister_pair_006952a0(current, member);
        at<void*>(member, 0x14) = target;
        if (target != nullptr) host.observer_register_pair_00694a60(target, member);
    }
    return member;
}

// The inlined world-pose refresh: when entity+C8h is clear, refresh the parent, compose the local
// matrix +74h with the parent's world +CCh (or take +74h alone), copy into +CCh, set +C8h, clear
// +10Ch.
void refresh_entity_pose(Seg30Host& host, void* entity) {
    if (at<std::uint8_t>(entity, 0xC8) != 0) return;
    if (ptr(entity, 0x3C) != nullptr) host.entity_pose_refresh_world_00414db0(ptr(entity, 0x3C));
    alignas(4) std::uint8_t composed[0x40];
    const void* source = add(entity, 0x74);
    if (ptr(entity, 0x3C) != nullptr) {
        source = host.matrix_multiply_4x4_00413920(add(entity, 0x74), composed, add(ptr(entity, 0x3C), 0xCC));
    }
    host.matrix_copy_4x4_x87_004134f0(add(entity, 0xCC), source);
    at<std::uint8_t>(entity, 0xC8) = 1;
    at<std::uint8_t>(entity, 0x10C) = 0;
}

}  // namespace

// 006BEEC0, __thiscall(ECX = self), RET; tail-jumps to 00695870. Frees the +14h buffer.
void buffered_callback_owner_destruct_006beec0(Seg30Host& host, void* self) {
    if (ptr(self, 0x14) != nullptr) host.free_00bf6989(ptr(self, 0x14));
    host.callback_owner_destroy_00695870(self);
}

// 006BEF30, __thiscall(ECX = block, archive), RET 4. Opens the section {0, "traffic"}, lets the +3Ch
// traffic object visit it through its vtable +0Ch, closes the section.
void air_ops_block_read_traffic_table_006bef30(Seg30Host& host, void* block, void* archive) {
    void* const traffic = ptr(block, 0x3C);
    host.archive_open_04(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kTrafficKey)});
    host.object_read_lua_0c(traffic, archive);
    host.archive_close_08(archive);
}

// 006BF350, __thiscall(ECX = block), RET. Sum of the +44h stock list node counts (+0Ch), minus
// the count of every slot in state 1 or 5. The slot count +50h is re-read under the +0Ch lock on
// every iteration. The debug-iterator check inside the list walk cannot fire (the loop test
// already excludes the head) but is kept.
int air_ops_total_plane_count_006bf350(Seg30Host& host, void* block) {
    void* const head = ptr(block, 0x44);
    std::int32_t total = 0;
    for (void* node = ptr(head, 0); node != ptr(block, 0x44); node = ptr(node, 0)) {
        total += at<std::int32_t>(node, 0x0C);
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
    }
    for (std::int32_t i = 0;; ++i) {
        void* const section = ptr(block, 0x0C);
        lock_enter(host, section);
        const std::int32_t count = at<std::int32_t>(block, kBlockSlotCount);
        lock_leave(host, section);
        if (i >= count) break;
        void* const slot = add(ptr(block, kBlockSlots), static_cast<std::size_t>(i) * kSlotStride);
        const std::int32_t state = at<std::int32_t>(slot, kSlotState);
        if (state == 1 || state == 5) total -= at<std::int32_t>(slot, kSlotCount);
    }
    return total;
}

// 006BF880, __thiscall(ECX = list {first, last, count}, holder), RET 4. Unlinks holder through its
// +B8h prev / +BCh next links; a holder with both links null is only unlinked when it is the
// sole element (count < 2).
void air_ops_holder_list_unlink_006bf880(void* list, void* holder) {
    void* const prev = ptr(holder, 0xB8);
    if (prev == nullptr && ptr(holder, 0xBC) == nullptr && at<std::int32_t>(list, 8) >= 2) return;
    if (prev == nullptr) {
        at<void*>(list, 0) = ptr(holder, 0xBC);
    } else {
        at<void*>(prev, 0xBC) = ptr(holder, 0xBC);
    }
    if (ptr(holder, 0xBC) == nullptr) {
        at<void*>(list, 4) = ptr(holder, 0xB8);
    } else {
        at<void*>(ptr(holder, 0xBC), 0xB8) = ptr(holder, 0xB8);
    }
    at<void*>(holder, 0xBC) = nullptr;
    at<void*>(holder, 0xB8) = nullptr;
    at<std::int32_t>(list, 8) -= 1;
}

// 006C0E70, __thiscall(ECX = block, slot, class, count), RET 0Ch, returns AL.
bool air_ops_can_fill_slot_006c0e70(Seg30Host& host, void* block, int slot, void* vehicle_class, int count) {
    const std::int32_t state = at<std::int32_t>(slot_at(block, slot), kSlotState);
    if (state != 1 && state != 5) return false;
    if (vehicle_class == nullptr) return true;
    if (count > host.class_plane_limit_006bf3e0(block, vehicle_class)) return false;
    StockPair stock{};
    if (count > host.stock_available_006bf230(block, &stock, vehicle_class)->available) return false;
    return count <= host.slot_capacity_006bd460(block, slot);
}

// 006C10D0, __thiscall(ECX = block, slot, class, count), RET 0Ch, returns AL. No xref. The stock
// test counts the slot's own planes when the class is unchanged, and the limit is the block's
// plane limit +58h against the committed planes with this slot's count replaced.
bool air_ops_can_fill_slot_006c10d0(Seg30Host& host, void* block, int slot, void* vehicle_class, int count) {
    void* const record = slot_at(block, slot);
    const std::int32_t state = at<std::int32_t>(record, kSlotState);
    if (state != 1 && state != 5) return false;
    if (vehicle_class == nullptr) return true;
    void* const old_class = ptr(record, kSlotClass);
    StockPair stock{};
    std::int32_t free = host.stock_available_006bf230(block, &stock, vehicle_class)->free;
    if (old_class == vehicle_class) free += at<std::int32_t>(record, kSlotCount);
    if (count > host.class_plane_limit_006bf3e0(block, vehicle_class) || count > free) return false;
    const std::int32_t in_slot = at<std::int32_t>(record, kSlotCount);
    return host.committed_planes_006bd3f0(block) + (count - in_slot) <=
           at<std::int32_t>(block, kBlockPlaneLimit);
}

// 006C1170, __thiscall(ECX = block, slot, class, count), RET 0Ch. No xref. 006BF230 is called and
// its result discarded. On a class change the slot takes class+134h into +10h and is published.
void air_ops_set_slot_class_and_count_006c1170(Seg30Host& host, void* block, int slot, void* vehicle_class,
                                               std::int32_t count) {
    void* const record = slot_at(block, slot);
    void* const old_class = ptr(record, kSlotClass);
    StockPair stock{};
    host.stock_available_006bf230(block, &stock, vehicle_class);
    if (ptr(record, kSlotClass) != vehicle_class) {
        at<void*>(record, kSlotClass) = vehicle_class;
        at<std::uint32_t>(record, kSlotClassField134) =
            vehicle_class == nullptr ? 0u : at<std::uint32_t>(vehicle_class, 0x134);
    }
    at<std::int32_t>(record, kSlotCount) = count;
    if (old_class != vehicle_class) host.publish_slot_006bf150(block, slot);
}

// 006C1310 / 006C1350 / 006C4990 / 006C49D0, __thiscall(ECX = member, target), RET 4, return the
// member. Byte-identical template copies; no xrefs.
void* observer_member_set_target_006c1310(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}
void* observer_member_set_target_006c1350(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}
void* observer_member_set_target_006c4990(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}
void* observer_member_set_target_006c49d0(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}

// 006C1960, __thiscall(ECX = self, arg), RET 4. A 58h-byte item with three vtables (+0h, +14h,
// +38h); caller 00675C40 (support-manager update).
void support_manager_item_construct_006c1960(void* self, std::uint32_t arg) {
    at<std::uint32_t>(self, 0x04) = 0;
    at<std::uint32_t>(self, 0x08) = 0;
    at<std::uint32_t>(self, 0x10) = 0;
    at<std::uint32_t>(self, 0x00) = kSupportItemVtable0;
    at<std::uint32_t>(self, 0x0C) = 3;
    at<std::uint32_t>(self, 0x18) = 0;
    at<std::uint32_t>(self, 0x1C) = 0;
    at<std::uint32_t>(self, 0x20) = 0;
    at<std::uint32_t>(self, 0x28) = 0;
    at<std::uint32_t>(self, 0x14) = kSupportItemVtable14;
    at<std::uint8_t>(self, 0x24) = 1;
    at<std::uint8_t>(self, 0x34) = 0;
    at<std::uint32_t>(self, 0x2C) = 1;
    at<std::uint32_t>(self, 0x30) = 0;
    at<std::uint32_t>(self, 0x3C) = 0;
    at<std::uint32_t>(self, 0x40) = 0;
    at<std::uint32_t>(self, 0x44) = 0;
    at<std::uint32_t>(self, 0x4C) = 0;
    at<std::uint8_t>(self, 0x48) = 1;
    at<std::uint32_t>(self, 0x38) = kSupportItemVtable38;
    at<std::uint32_t>(self, 0x50) = 9;
    at<std::uint32_t>(self, 0x54) = arg;
}

// 006C47F0, __thiscall(ECX = block, message), RET 4. Message fields: +14h peer, +20h slot,
// +24h class id (resolved by 00964790 with DL = 1 when non-zero), +28h count, +2Ch slot +10h
// value. Accepted: fill, store +10h, re-broadcast and notify observer slot 4. Refused: build an
// AIRBASE_SETSLOT reply (a stack message of at most 30h bytes; the image runs no destructor on
// it) and send it to the peer only when the session mode is 1.
void air_ops_apply_slot_setup_006c47f0(Seg30Host& host, void* block, void* message) {
    const std::int32_t count = at<std::int32_t>(message, 0x28);
    const std::uint32_t field10 = at<std::uint32_t>(message, 0x2C);
    const std::int32_t slot = at<std::int32_t>(message, 0x20);
    void* vehicle_class = nullptr;
    if (at<std::uint32_t>(message, 0x24) != 0) {
        vehicle_class = host.vehicle_class_get_or_create_00964790(at<std::uint32_t>(message, 0x24), true);
    }
    if (air_ops_can_fill_slot_006c0e70(host, block, slot, vehicle_class, count)) {
        host.fill_slot_006c0f00(block, slot, vehicle_class, count);
        at<std::uint32_t>(slot_at(block, slot), 0x10) = field10;
        host.session_owner_broadcast_0077c7b0(ptr(block, kBlockOwner), message,
                                              at<std::uint32_t>(message, 0x14));
        host.observer_notify_slot_00696350(ptr(block, kBlockOwner), 4, 0);
        return;
    }
    alignas(4) std::uint8_t reply[0x30] = {};
    host.airbase_setslot_message_construct_006bd800(reply, block, slot);
    const std::int32_t mode = at<std::int32_t>(host.game_e188a8(), kGameSessionMode);
    if (mode != 0 && mode == 1) {
        host.entity_send_message_to_peer_00779fc0(ptr(block, kBlockOwner), at<std::uint32_t>(message, 0x14),
                                                  reply);
    }
}

// 006C4D80, __fastcall(ECX = vector {data, size, capacity}), RET. capacity = 2 * capacity + 2;
// each element is copy-constructed (0065CAF0) into the new block and the old one destroyed through
// vtable +0 (flags 0); the old block is freed.
void native_vector58_grow_006c4d80(Seg30Host& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 8) * 2 + 2;
    at<std::uint32_t>(vector, 8) = capacity;
    void* const fresh = host.operator_new_00bf55be(capacity * kSlotStride);
    for (std::uint32_t i = 0; i < at<std::uint32_t>(vector, 4); ++i) {
        void* const dst = add(fresh, i * kSlotStride);
        void* const src = add(ptr(vector, 0), i * kSlotStride);
        if (dst != nullptr) host.slot_record_copy_construct_0065caf0(dst, src);
        host.scalar_deleting_destructor_0(src, 0);
    }
    if (ptr(vector, 0) != nullptr) host.free_00bf6989(ptr(vector, 0));
    at<void*>(vector, 0) = fresh;
}

// 006C4FA0, __fastcall(ECX = self), RET, returns self. vtable 00CF87D4; the embedded list at +14h
// gets its head node from 006BFA20 (+18h) and size +1Ch = 0.
void* air_ops_callback_list_construct_006c4fa0(Seg30Host& host, void* self) {
    at<std::uint32_t>(self, 0x04) = 0;
    at<std::uint32_t>(self, 0x08) = 0;
    at<std::uint32_t>(self, 0x0C) = 0;
    at<std::uint8_t>(self, 0x10) = 0;
    at<std::uint32_t>(self, 0x00) = kCallbackListVtable;
    at<void*>(self, 0x18) = host.list_buy_head_node_006bfa20(add(self, 0x14));
    at<std::uint32_t>(self, 0x1C) = 0;
    return self;
}

// 006C5000, __thiscall(ECX = self), RET; clears the +14h list then tail-jumps to 00695870.
void air_ops_callback_list_destruct_006c5000(Seg30Host& host, void* self) {
    host.list_clear_006c1430(add(self, 0x14));
    host.callback_owner_destroy_00695870(self);
}

// 006C5640, __thiscall(ECX = block, squadron), RET 4, returns AL. The squadron's +3D0h leader
// unit: a small-turn-roll unit is accepted when block flag +20h bit 0 is set; otherwise the
// leader must be kind 10h or 16h and bit 1 must be set.
bool air_ops_block_accepts_squadron_for_landing_006c5640(Seg30Host& host, void* block, void* squadron) {
    if (squadron == nullptr || !host.squadron_not_excluded_006c4790(block, squadron)) return false;
    void* const leader = ptr(squadron, 0x3D0);
    if (host.unit_uses_small_turn_roll_limit_0047b880(leader) && (at<std::uint32_t>(block, 0x20) & 1u) != 0) {
        return true;
    }
    if (!host.is_kind_of_5c(leader, 0x10) && !host.is_kind_of_5c(leader, 0x16)) return false;
    return ((at<std::uint32_t>(block, 0x20) >> 1) & 1u) != 0;
}

// 006C56B0, __thiscall(ECX = block, plane), RET 4, returns AL. Ghidra shows a cdecl body; the
// listing replaces the stack argument with plane+9D4h (its squadron) and tail-jumps to 006C5640
// with ECX unchanged. A null plane answers false.
bool air_ops_block_accepts_plane_for_landing_006c56b0(Seg30Host& host, void* block, void* plane) {
    if (plane == nullptr) return false;
    return air_ops_block_accepts_squadron_for_landing_006c5640(host, block, ptr(plane, 0x9D4));
}

// 006C6C60, __thiscall(ECX = vector, record), RET 4. push_back of a 58h slot record: when full the
// same growth as 006C4D80 runs inline; the new element is copy-constructed (0065CAF0) unless its
// address is null.
void native_vector58_push_back_006c6c60(Seg30Host& host, void* vector, const void* record) {
    if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) native_vector58_grow_006c4d80(host, vector);
    void* const dst = add(ptr(vector, 0), at<std::uint32_t>(vector, 4) * kSlotStride);
    if (dst != nullptr) host.slot_record_copy_construct_0065caf0(dst, record);
    at<std::uint32_t>(vector, 4) += 1;
}

// 006C6E30, __thiscall(ECX = self), RET, returns self. 006C4FA0, then vtable 00CF8888.
void* air_ops_callback_list8888_construct_006c6e30(Seg30Host& host, void* self) {
    air_ops_callback_list_construct_006c4fa0(host, self);
    at<std::uint32_t>(self, 0) = kCallbackList8888Vtable;
    return self;
}

// 006C71C0, __thiscall(ECX = self), RET. Byte-identical to 006C5000.
void air_ops_callback_list_destruct_006c71c0(Seg30Host& host, void* self) {
    host.list_clear_006c1430(add(self, 0x14));
    host.callback_owner_destroy_00695870(self);
}

// 006C88D0, __thiscall(ECX = self, source), RET 4. vtable 00CF8888 slot +4h. The source's vtable
// +4h answers the observed target; the pair (target, self) is unregistered and every node of the
// +14h list holding that target is removed: erase(remove(begin, end, target), end).
void air_ops_callback_list8888_on_notify_006c88d0(Seg30Host& host, void* self, void* source) {
    void* const target = host.object_virtual_04(source);
    host.observer_unregister_pair_006952a0(target, self);
    void* const list = add(self, 0x14);
    void* const head = ptr(self, 0x18);
    CheckedIter removed{};
    host.list_remove_006c8750(&removed, &target, CheckedIter{list, ptr(head, 0)}, CheckedIter{list, head});
    CheckedIter erased{};
    host.list_erase_range_006c7030(list, &erased, removed, CheckedIter{list, ptr(list, 4)});
}

// 006CA9C0, __thiscall(ECX = self, target), RET 4. Registers the pair (target, self), then
// push_back(target) on the +14h list: 006BFBD0 buys the node (next = head, prev = head->prev),
// 006C89D0 grows the size, and the node is linked in.
void air_ops_callback_list_add_006ca9c0(Seg30Host& host, void* self, void* target) {
    host.observer_register_pair_00694a60(target, self);
    void* const list = add(self, 0x14);
    void* const head = ptr(list, 4);
    void* const node = host.list_buy_node4_006bfbd0(list, head, ptr(head, 4), &target);
    host.list_increment_size_006c89d0(list, 1);
    at<void*>(head, 4) = node;
    at<void*>(ptr(node, 4), 0) = node;
}

// 006CDE40, __thiscall(ECX = self), RET. free(*self) when non-null; the pointer is not cleared.
void free_owned_block_006cde40(Seg30Host& host, void* self) {
    if (ptr(self, 0) != nullptr) host.free_00bf6989(ptr(self, 0));
}

// 006CDEA0, __fastcall(ECX = vector {data, size, capacity}), RET. capacity = 2 * capacity + 2,
// dwords copied (a null destination skips the store), old block freed.
void native_dword_vector_grow_006cdea0(Seg30Host& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 8) * 2 + 2;
    at<std::uint32_t>(vector, 8) = capacity;
    auto* const fresh = static_cast<std::uint32_t*>(host.operator_new_00bf55be(capacity * 4));
    std::uint32_t* dst = fresh;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(vector, 4); ++i, ++dst) {
        if (dst != nullptr) *dst = static_cast<std::uint32_t*>(ptr(vector, 0))[i];
    }
    if (ptr(vector, 0) != nullptr) host.free_00bf6989(ptr(vector, 0));
    at<void*>(vector, 0) = fresh;
}

// 006CDF40, __thiscall(ECX = self), RET. Byte-identical to 006BEEC0.
void buffered_callback_owner_destruct_006cdf40(Seg30Host& host, void* self) {
    buffered_callback_owner_destruct_006beec0(host, self);
}

// 006CE260, __thiscall(ECX = self, float dt), RET 4. Slot +4h of vtable 00CF8928: forwards dt to
// 007586D0 with ECX = [self+4h] (the mothership).
void mothership_step_forwarder_006ce260(Seg30Host& host, void* self, float dt) {
    host.mothership_step_elevator_frame_007586d0(ptr(self, 4), dt);
}

// 006CE300, __thiscall(ECX = member, target), RET 4. The observer setter without the
// `MOV EAX, ESI`: EAX is left as whatever 00694A60/006952A0 or the compare left, so no result.
void observer_member_set_target_006ce300(Seg30Host& host, void* member, void* target) {
    observer_member_set_target(host, member, target);
}

// 006CE330, __thiscall(ECX = vector, const dword* value), RET 4. push_back with the 006CDEA0
// growth inline; a null destination skips the store but the size still grows.
void native_dword_vector_push_back_006ce330(Seg30Host& host, void* vector, const std::uint32_t* value) {
    if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) native_dword_vector_grow_006cdea0(host, vector);
    auto* const dst = static_cast<std::uint32_t*>(ptr(vector, 0)) + at<std::uint32_t>(vector, 4);
    if (dst != nullptr) *dst = *value;
    at<std::uint32_t>(vector, 4) += 1;
}

// 006CE400, __thiscall(ECX = holder), RET, returns holder. Intrusive release: InterlockedDecrement
// on object+4h; at zero the object's vtable +0 runs with no arguments. The holder is cleared.
void* ref_ptr_release_006ce400(Seg30Host& host, void* self) {
    void* const object = ptr(self, 0);
    if (object != nullptr) {
        if (host.interlocked_decrement(&at<std::int32_t>(object, 4)) == 0) host.object_virtual_0_noargs(object);
        at<void*>(self, 0) = nullptr;
    }
    return self;
}

// 006CE430, __thiscall(ECX = self), RET. Byte-identical to 006BEEC0.
void buffered_callback_owner_destruct_006ce430(Seg30Host& host, void* self) {
    buffered_callback_owner_destruct_006beec0(host, self);
}

// 006CEA10, __thiscall(ECX = self, target), RET 4, returns self. vtable 00CF8900, +10h = 1,
// +14h = target, registered with 00694A60 when non-null.
void* follow_observer_member_construct_006cea10(Seg30Host& host, void* self, void* target) {
    at<std::uint32_t>(self, 0x04) = 0;
    at<std::uint32_t>(self, 0x08) = 0;
    at<std::uint32_t>(self, 0x0C) = 0;
    at<std::uint32_t>(self, 0x00) = kFollowObserverVtable;
    at<void*>(self, 0x14) = target;
    at<std::uint8_t>(self, 0x10) = 1;
    if (target != nullptr) host.observer_register_pair_00694a60(target, self);
    return self;
}

// 006CEA70 / 006CEF40, __thiscall(ECX = member, target), RET 4, return the member (same body as
// 006C1310).
void* observer_member_set_target_006cea70(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}
void* observer_member_set_target_006cef40(Seg30Host& host, void* member, void* target) {
    return observer_member_set_target(host, member, target);
}

// 006CEB40, __thiscall(ECX = holder), RET. 006CE400 without the return value.
void ref_ptr_release_006ceb40(Seg30Host& host, void* self) {
    ref_ptr_release_006ce400(host, self);
}

// 006CE4F0, __thiscall(ECX = site, const float* point, float radius), RET 8. No xref. First plane of
// the occupant vector (+34h data, +38h size) whose refreshed world position (+FCh..+104h) lies
// strictly closer than radius; null when none. Squared length summed as (dx*dx + dy*dy) + dz*dz
// and stored to float; distance 0 when it is not above 1e-10 (double), else sqrt stored to float.
// The loop end is recomputed from +34h/+38h each step.
void* air_ops_site_scan_occupants_near_006ce4f0(Seg30Host& host, void* site, const float* point, float radius) {
    auto* it = static_cast<void**>(ptr(site, 0x34));
    if (it == it + at<std::int32_t>(site, 0x38)) return nullptr;
    do {
        void* const plane = *it;
        refresh_entity_pose(host, plane);
        const float dx = at<float>(plane, 0xFC) - point[0];
        const float dy = at<float>(plane, 0x100) - point[1];
        const float dz = at<float>(plane, 0x104) - point[2];
        // x87 keeps the products and sums in extended precision; double holds the float products
        // exactly.
        const auto squared = static_cast<float>(
            (static_cast<double>(dx) * dx + static_cast<double>(dy) * dy) + static_cast<double>(dz) * dz);
        float distance;
        if (static_cast<double>(squared) > kDistanceEpsilon) {
            distance = host.crt_sqrt_00bf7030(squared);
        } else {
            distance = 0.0f;
        }
        if (radius > distance) return plane;
        ++it;
    } while (it != static_cast<void**>(ptr(site, 0x34)) + at<std::int32_t>(site, 0x38));
    return nullptr;
}

// 006CEB70, __thiscall(ECX = site), RET. vtable 00CF8948; frees the occupant buffer +34h, destroys
// the callback owner at +20h (00695870) and the follow member at +4h (006CDD70).
void air_ops_site_destruct_006ceb70(Seg30Host& host, void* site) {
    at<std::uint32_t>(site, 0) = kAirOpsSiteVtable;
    if (ptr(site, 0x34) != nullptr) host.free_00bf6989(ptr(site, 0x34));
    host.callback_owner_destroy_00695870(add(site, 0x20));
    host.follow_observer_member_destroy_006cdd70(add(site, 0x04));
}

// 006CEE80, __thiscall(ECX = member, source), RET 4. When the source's vtable +4h target is the
// member's +14h target: clear +14h first, then unregister (target, member).
void air_ops_site_on_ready_plane_gone_006cee80(Seg30Host& host, void* member, void* source) {
    void* const target = host.object_virtual_04(source);
    if (target == ptr(member, 0x14)) {
        at<void*>(member, 0x14) = nullptr;
        host.observer_unregister_pair_006952a0(target, member);
    }
}

// 006CEFF0, __thiscall(ECX = self, source), RET 4. Unregisters (target, self) and removes every
// copy of target from the dword vector +14h/+18h: new_end = remove(begin, end, target); the tail
// move from end to end is empty; size -= (end - new_end) / 4 (SAR).
void air_ops_site_occupants_on_notify_006ceff0(Seg30Host& host, void* self, void* source) {
    void* const target = host.object_virtual_04(source);
    host.observer_unregister_pair_006952a0(target, self);
    auto* const begin = static_cast<std::uint32_t*>(ptr(self, 0x14));
    const auto value = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target));
    std::uint32_t* const new_end = host.dword_remove_006ced40(begin, begin + at<std::int32_t>(self, 0x18), &value);
    std::uint32_t* const end = static_cast<std::uint32_t*>(ptr(self, 0x14)) + at<std::int32_t>(self, 0x18);
    std::uint32_t* src = end;
    std::uint32_t* const dst_base = new_end;
    while (src != static_cast<std::uint32_t*>(ptr(self, 0x14)) + at<std::int32_t>(self, 0x18)) {
        dst_base[src - end] = *src;
        ++src;
    }
    at<std::int32_t>(self, 0x18) -= static_cast<std::int32_t>(end - new_end);
}

// 006CFA80, __thiscall(ECX = airfield), RET, returns the site. operator new(48h); when it succeeds
// 006CF100 constructs it, the vtable becomes 00CF89F8 and +44h = the airfield.
void* air_field_site_create_006cfa80(Seg30Host& host, void* airfield) {
    void* const site = host.operator_new_00bf681b(0x48);
    if (site == nullptr) return nullptr;
    host.air_ops_site_construct_006cf100(site);
    at<std::uint32_t>(site, 0) = kAirFieldSiteVtable;
    at<void*>(site, 0x44) = airfield;
    return site;
}

// 006CF1C0, __thiscall(ECX = site, archive), RET 4. Site vtable +0Ch. Reads the +4h ready-plane
// member ("readyPlane"), the float +40h ("lastLandingTime"), then the "planes" section: while
// has({1, i}) read({1, i}, {4, &plane}) and push the plane on the +20h occupant list.
void air_ops_site_read_state_006cf1c0(Seg30Host& host, void* site, void* archive) {
    host.save_reader_read_observer_ref_006cf0a0(archive, add(site, 0x04),
                                                ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kReadyPlaneKey)});
    host.archive_read_10(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kLastLandingTimeKey)},
                         ArchiveArg{2, reinterpret_cast<std::uintptr_t>(add(site, 0x40))});
    host.archive_open_04(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kPlanesKey)});
    for (std::uint32_t i = 0; host.archive_has_14(archive, ArchiveArg{1, i}); ++i) {
        void* plane = nullptr;  // the image leaves this stack slot uninitialised
        host.archive_read_10(archive, ArchiveArg{1, i}, ArchiveArg{4, reinterpret_cast<std::uintptr_t>(&plane)});
        host.air_ops_site_push_occupant_006ce910(add(site, 0x20), plane);
    }
    host.archive_close_08(archive);
}

// 006CF2A0, __thiscall(ECX = site, archive), RET 4. Site vtable +10h. Writes a by-value copy of the
// +4h member (006CE8A0 into the argument area, vtable 00CF89B4) as "readyPlane" through 006CEAD0,
// the float +40h as "lastLandingTime" (kind 2, value bits), then every occupant as {1, i} ->
// {4, plane} inside "planes". The occupant end is recomputed each step.
void air_ops_site_write_state_006cf2a0(Seg30Host& host, void* site, void* archive) {
    alignas(4) std::uint8_t member_copy[0x18];
    host.follow_observer_member_copy_construct_006ce8a0(member_copy, add(site, 0x04));
    at<std::uint32_t>(member_copy, 0) = kFollowObserverByValueVtable;
    host.save_writer_write_observer_ref_006cead0(archive, member_copy,
                                                 ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kReadyPlaneKey)});
    host.archive_write_0c(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kLastLandingTimeKey)},
                          ArchiveArg{2, at<std::uint32_t>(site, 0x40)});
    host.archive_open_04(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kPlanesKey)});
    std::uint32_t index = 0;
    for (auto* it = static_cast<std::uint32_t*>(ptr(site, 0x34));
         it != static_cast<std::uint32_t*>(ptr(site, 0x34)) + at<std::int32_t>(site, 0x38); ++it, ++index) {
        host.archive_write_0c(archive, ArchiveArg{1, index}, ArchiveArg{4, *it});
    }
    host.archive_close_08(archive);
}

// 006D0470, __thiscall(ECX = site), RET. vtable 00CF8A58. The +BCh effect (refcounted at +4h):
// when present, 00867B10 stops its children and +9h is set; then it is released (InterlockedDecrement,
// vtable +0 with no arguments at zero) and cleared. Then the +A0h follow member (006CDD70), the
// +44h elevator platform (006FC2E0) and the base site (006CEB70).
void mothership_site_destruct_006d0470(Seg30Host& host, void* site) {
    at<std::uint32_t>(site, 0) = kMotherShipSiteVtable;
    void* effect = ptr(site, 0xBC);
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 0x9) = 1;
    }
    effect = ptr(site, 0xBC);
    if (effect != nullptr) {
        if (host.interlocked_decrement(&at<std::int32_t>(effect, 4)) == 0) host.object_virtual_0_noargs(effect);
        at<void*>(site, 0xBC) = nullptr;
    }
    host.follow_observer_member_destroy_006cdd70(add(site, 0xA0));
    host.elevator_platform_destruct_006fc2e0(add(site, 0x44));
    air_ops_site_destruct_006ceb70(host, site);
}

// 006D0530, __thiscall(ECX = site), RET, returns AL. For every occupant (end recomputed per step):
// refresh its world pose, take the third component of 006BCC90(holder = [site+B8h]+1208h, out,
// plane+FCh) and fail when the tuning singleton's +520h is greater than it. True when none fails.
bool mothership_site_has_free_runway_006d0530(Seg30Host& host, void* site) {
    auto* it = static_cast<void**>(ptr(site, 0x34));
    if (it == it + at<std::int32_t>(site, 0x38)) return true;
    do {
        void* const plane = *it;
        refresh_entity_pose(host, plane);
        float local[3];
        const float along = host.air_ops_holder_local_point_006bcc90(ptr(ptr(site, 0xB8), 0x1208), local,
                                                                     static_cast<float*>(add(plane, 0xFC)))[2];
        if (at<float>(host.game_tuning_singleton_0042e740(), 0x520) > along) return false;
        ++it;
    } while (it != static_cast<void**>(ptr(site, 0x34)) + at<std::int32_t>(site, 0x38));
    return true;
}

// 006D07B0, __thiscall(ECX = site, archive), RET 4. Base 006CF1C0, then "loadDelay" (float +9Ch),
// the +A0h member as "justLiftedUpID" (006CF0A0), and the "elevator" section: 00BD4FE0 builds a
// 0Ch-byte reader reference in the argument area, which 006FBF00 (ECX = site+44h) takes by value.
void mothership_site_read_state_006d07b0(Seg30Host& host, void* site, void* archive) {
    air_ops_site_read_state_006cf1c0(host, site, archive);
    host.archive_read_10(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kLoadDelayKey)},
                         ArchiveArg{2, reinterpret_cast<std::uintptr_t>(add(site, 0x9C))});
    host.save_reader_read_observer_ref_006cf0a0(archive, add(site, 0xA0),
                                                ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kJustLiftedUpIdKey)});
    alignas(4) std::uint8_t section[0x0C];
    host.archive_reader_ref_open_00bd4fe0(archive, section, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kElevatorKey)});
    host.elevator_platform_read_state_006fbf00(add(site, 0x44), section);
}

// 006D0850, __thiscall(ECX = site, archive), RET 4. The writer twin of 006D07B0: base 006CF2A0,
// "loadDelay" (kind 2, the float bits of +9Ch), a by-value copy of +A0h (006CE8A0, vtable
// 00CF89B4) as "justLiftedUpID" through 006CEAD0, then 00BD5010 / 006FBFD0 for "elevator".
void mothership_site_write_state_006d0850(Seg30Host& host, void* site, void* archive) {
    air_ops_site_write_state_006cf2a0(host, site, archive);
    host.archive_write_0c(archive, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kLoadDelayKey)},
                          ArchiveArg{2, at<std::uint32_t>(site, 0x9C)});
    alignas(4) std::uint8_t member_copy[0x18];
    host.follow_observer_member_copy_construct_006ce8a0(member_copy, add(site, 0xA0));
    at<std::uint32_t>(member_copy, 0) = kFollowObserverByValueVtable;
    host.save_writer_write_observer_ref_006cead0(archive, member_copy,
                                                 ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kJustLiftedUpIdKey)});
    alignas(4) std::uint8_t section[0x0C];
    host.archive_writer_ref_open_00bd5010(archive, section, ArchiveArg{0, reinterpret_cast<std::uintptr_t>(kElevatorKey)});
    host.elevator_platform_write_state_006fbfd0(add(site, 0x44), section);
}

// 006D0A10, __thiscall(ECX = mothership), RET, returns the site. operator new(C0h) then 006CFAF0
// (ECX = site, mothership); null when the allocation fails.
void* mothership_site_create_006d0a10(Seg30Host& host, void* mothership) {
    void* const site = host.operator_new_00bf681b(0xC0);
    if (site == nullptr) return nullptr;
    return host.mothership_site_construct_006cfaf0(site, mothership);
}

// 006D0C30, __thiscall(ECX = record), RET, returns EAX (0/1). Ghidra: vtable 00D1A9A0 slot +14h.
// True when float +138h > 10.0f (SSE COMISS) and float +13Ch > 100.0 (double, x87 FCOMIP).
std::uint32_t class_record_positive_thresholds_006d0c30(void* record) {
    if (at<float>(record, 0x138) > kThresholdA &&
        static_cast<double>(at<float>(record, 0x13C)) > kThresholdB) {
        return 1;
    }
    return 0;
}

// 006D0C80, __thiscall(ECX = airfield), RET. 0077E830 (Lua self), then a 240h-byte weapon director
// (008363E0 with airfield+310h, or null for a null airfield, and 0) into +8C8h (null when the
// allocation fails); when [airfield+C0h]+4h == 1, 0095A880(0, 1, 0) instantiates the class devices.
void air_field_init_lua_and_weapon_director_006d0c80(Seg30Host& host, void* airfield) {
    host.mission_entity_attach_lua_self_0077e830(airfield);
    void* director = host.operator_new_00bf681b(0x240);
    if (director != nullptr) {
        director = host.weapon_director_base_construct_008363e0(
            director, airfield != nullptr ? add(airfield, 0x310) : nullptr, 0);
    }
    at<void*>(airfield, 0x8C8) = director;
    if (at<std::int32_t>(ptr(airfield, 0xC0), 4) == 1) {
        host.unit_instantiate_class_devices_0095a880(airfield, 0, 1, 0);
    }
}

// 006D0D50, __thiscall(ECX = airfield, message), RET 4. No direct xref (a vtable slot). Message
// type 72h (AIRBASE_SET_RUNWAYFAILURE) clears +748h (the RunwayFailure byte of the Lua getter),
// else type 74h (SET_HANGARFAILURE) clears +749h. The SET types clear: the routine's role (an
// undo or rejected-request path) is not established.
void air_field_clear_failure_flags_on_set_006d0d50(Seg30Host& host, void* airfield, void* message) {
    if (host.message_is_type_0c(message, 0x72)) {
        at<std::uint8_t>(airfield, kAirFieldRunwayFailure) = 0;
        return;
    }
    if (host.message_is_type_0c(message, 0x74)) at<std::uint8_t>(airfield, kAirFieldHangarFailure) = 0;
}

// 006D0D90, __thiscall(ECX = airfield, message), RET 4. No direct xref. Type 73h
// (CLEAR_RUNWAYFAILURE) clears +748h, else type 75h (CLEAR_HANGARFAILURE) clears +749h.
void air_field_clear_failure_flags_on_clear_006d0d90(Seg30Host& host, void* airfield, void* message) {
    if (host.message_is_type_0c(message, 0x73)) {
        at<std::uint8_t>(airfield, kAirFieldRunwayFailure) = 0;
        return;
    }
    if (host.message_is_type_0c(message, 0x75)) at<std::uint8_t>(airfield, kAirFieldHangarFailure) = 0;
}

// 006D0DF0, __thiscall(ECX = airfield, token, target), RET 8, returns AL. True when the +72Ch
// sub-object's 006BA6A0 or the entity's 00779D50 accept, or when the token equals (case-
// insensitively, 00438E10 == 0) "settarget", "cleartarget" or "clearorders".
bool air_field_may_follow_target_006d0df0(Seg30Host& host, void* airfield, const char* token, void* target) {
    if (host.may_follow_006ba6a0(add(airfield, 0x72C), token, target)) return true;
    if (host.entity_may_follow_target_00779d50(airfield, token, target)) return true;
    if (host.cstring_compare_insensitive_00438e10(token, kSetTargetToken) == 0) return true;
    if (host.cstring_compare_insensitive_00438e10(token, kClearTargetToken) == 0) return true;
    return host.cstring_compare_insensitive_00438e10(token, kClearOrdersToken) == 0;
}

// 006D0E90, __thiscall(ECX = singleton), RET. Clears the published pointer 00E19954 and restores
// the singleton base vtable 00CE3818.
void air_field_lock_singleton_reset_006d0e90(Seg30Host& host, void* singleton) {
    host.global_u32(kAirFieldLockSingletonGlobal) = 0;
    at<std::uint32_t>(singleton, 0) = kSingletonBaseVtable;
}

// 006D1070, __thiscall(ECX = holder), RET. free(*holder) and clear it when non-null.
void free_and_null_006d1070(Seg30Host& host, void* holder) {
    if (ptr(holder, 0) != nullptr) {
        host.free_00bf6989(ptr(holder, 0));
        at<void*>(holder, 0) = nullptr;
    }
}

namespace {
// The POD element move shared by the 0Ch- and 10h-element vectors: dword-wise copies into a fresh
// operator-new block (a null destination skips the copy), then the old block is freed.
void move_pod_vector(Seg30Host& host, void* vector, std::uint32_t capacity, std::uint32_t element) {
    auto* const fresh = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * element));
    std::uint8_t* dst = fresh;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(vector, 4); ++i, dst += element) {
        if (dst != nullptr) {
            void* const src = add(ptr(vector, 0), i * element);
            for (std::uint32_t off = 0; off < element; off += 4) {
                at<std::uint32_t>(dst, off) = at<std::uint32_t>(src, off);
            }
        }
    }
    if (ptr(vector, 0) != nullptr) host.free_00bf6989(ptr(vector, 0));
    at<void*>(vector, 0) = fresh;
}
void move_vector12(Seg30Host& host, void* vector, std::uint32_t capacity) {
    move_pod_vector(host, vector, capacity, 0x0C);
}
}  // namespace

// 006D1170, __thiscall(ECX = vector {data, size, capacity}, capacity), RET 4. Sets the capacity
// (no comparison with the old one) and moves the 0Ch-byte elements.
void native_vector12_reserve_006d1170(Seg30Host& host, void* vector, std::uint32_t capacity) {
    at<std::uint32_t>(vector, 8) = capacity;
    move_vector12(host, vector, capacity);
}

// 006D11E0, __fastcall(ECX = vector), RET. capacity = 2 * capacity + 2, then the same move.
void native_vector12_grow_006d11e0(Seg30Host& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 8) * 2 + 2;
    at<std::uint32_t>(vector, 8) = capacity;
    move_vector12(host, vector, capacity);
}

// 006D1430, __thiscall(ECX = node, owner), RET 4, returns node. Base vtable 00CE89E8, 004E6480
// field init, +4Ch = owner, vtable 00CF8B18.
void* air_field_collision_node_construct_006d1430(Seg30Host& host, void* node, std::uint32_t owner) {
    at<std::uint32_t>(node, 0) = kCollisionNodeBaseVtable;
    host.collision_node_init_fields_004e6480(node);
    at<std::uint32_t>(node, 0x4C) = owner;
    at<std::uint32_t>(node, 0) = kAirFieldCollisionNodeVtable;
    return node;
}

// 006D1460, __thiscall(ECX = node), RET. Restores the base vtable and frees the +FCh shape buffer
// (not cleared).
void air_field_collision_node_destruct_006d1460(Seg30Host& host, void* node) {
    at<std::uint32_t>(node, 0) = kCollisionNodeBaseVtable;
    if (ptr(node, 0xFC) != nullptr) host.free_00bf6989(ptr(node, 0xFC));
}

// 006D1530, __thiscall(ECX unused, kind), RET 4, returns EAX. True for kinds 62h, 49h, 46h.
bool air_field_collision_node_is_kind_006d1530(std::uint32_t kind) {
    return kind == 0x62 || kind == 0x49 || kind == 0x46;
}

namespace {
void* construct_runway_message(Seg30Host& host, void* message, std::uint32_t type, std::uint32_t vtable) {
    host.session_message_construct_base_0075b430(message, type);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<float>(message, 0x20) = kRunwayMessageField20;
    at<std::uint32_t>(message, 0) = vtable;
    return message;
}
}  // namespace

// 006D16A0 / 006D1720 / 006D17A0 / 006D1820, __thiscall(ECX = message), RET, return the message.
// 0075B430(type), +18h word / +1Ah / +1Ch bytes = 0, +4h = 1, float +20h = 256.0, own vtable.
void* airbase_set_runway_failure_construct_006d16a0(Seg30Host& host, void* message) {
    return construct_runway_message(host, message, 0x72, kSetRunwayFailureVtable);
}
void* airbase_clear_runway_failure_construct_006d1720(Seg30Host& host, void* message) {
    return construct_runway_message(host, message, 0x73, kClearRunwayFailureVtable);
}
void* airbase_set_hangar_failure_construct_006d17a0(Seg30Host& host, void* message) {
    return construct_runway_message(host, message, 0x74, kSetHangarFailureVtable);
}
void* airbase_clear_hangar_failure_construct_006d1820(Seg30Host& host, void* message) {
    return construct_runway_message(host, message, 0x75, kClearHangarFailureVtable);
}

// 006D1990, __thiscall(ECX = shape, owner, float width, float length, kind), RET 10h, returns shape.
// Base vtable 00CE89DC, +1Ch = kind, +10h..+18h = the vec3 at 00F87574 (bit copies) and +4h..+0Ch
// the same through x87, +20h = owner, vtable 00CF8B94, +24h = hw = (float)(width * 0.5),
// +28h = hl = (float)(length * 0.5) (0.5 is the double at 00D7A280; FMUL at 006D19F7 is the DC
// form). Bounds via 0098AAB0(min, max): min = {-hw + 0, -0.5, 1 - hl}, max = {hw + 0, -0.5, hl + 1}
// (both y components are -0.5, as computed by 0 - 0.5).
void* air_field_runway_shape_construct_006d1990(Seg30Host& host, void* shape, std::uint32_t owner, float width,
                                                float length, std::uint32_t kind) {
    at<std::uint32_t>(shape, 0) = kShapeBaseVtable;
    at<std::uint32_t>(shape, 0x1C) = kind;
    at<std::uint32_t>(shape, 0x10) = host.global_u32(kShapeOriginGlobal);
    at<std::uint32_t>(shape, 0x14) = host.global_u32(kShapeOriginGlobal + 4);
    at<std::uint32_t>(shape, 0x18) = host.global_u32(kShapeOriginGlobal + 8);
    at<float>(shape, 0x04) = at<float>(shape, 0x10);
    at<float>(shape, 0x08) = at<float>(shape, 0x14);
    at<float>(shape, 0x0C) = at<float>(shape, 0x18);
    at<std::uint32_t>(shape, 0x20) = owner;
    at<std::uint32_t>(shape, 0) = kRunwayShapeVtable;
    const auto hw = static_cast<float>(static_cast<double>(width) * kHalf);
    at<float>(shape, 0x24) = hw;
    const auto hl = static_cast<float>(static_cast<double>(length) * kHalf);
    at<float>(shape, 0x28) = hl;
    const auto y = static_cast<float>(0.0 - kHalf);
    float max_corner[3] = {hw + 0.0f, y, hl + 1.0f};
    float min_corner[3] = {-hw + 0.0f, y, 1.0f + -hl};
    host.collision_shape_set_bounds_0098aab0(shape, min_corner, max_corner);
    return shape;
}

// 006D1A90, __fastcall(ECX = vector {data, size, capacity}), RET. 10h-byte POD elements,
// capacity = 2 * capacity + 2.
void native_vector16_grow_006d1a90(Seg30Host& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 8) * 2 + 2;
    at<std::uint32_t>(vector, 8) = capacity;
    move_pod_vector(host, vector, capacity, 0x10);
}

// 006D1B60, __thiscall(ECX = vector, const element*), RET 4. push_back with the 006D1A90 growth
// inline; a null destination skips the copy but the size grows.
void native_vector16_push_back_006d1b60(Seg30Host& host, void* vector, const void* element) {
    if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) native_vector16_grow_006d1a90(host, vector);
    void* const dst = add(ptr(vector, 0), at<std::uint32_t>(vector, 4) * 0x10);
    if (dst != nullptr) {
        for (std::uint32_t off = 0; off < 0x10; off += 4) {
            at<std::uint32_t>(dst, off) = at<std::uint32_t>(const_cast<void*>(element), off);
        }
    }
    at<std::uint32_t>(vector, 4) += 1;
}

// 006D1EC0, __thiscall(ECX = unit), RET. Returns unit+390h.
void* unit_get_390_006d1ec0(void* unit) { return ptr(unit, 0x390); }

// 006D1F30, __thiscall(ECX = unit), RET. Returns [[unit+538h]+54h] (the class type name).
void* unit_get_class_type_name_006d1f30(void* unit) { return ptr(ptr(unit, 0x538), 0x54); }

// 006D1F50, __thiscall(ECX unused, float out[2]), RET 4. Writes 0.0 to out[0] and out[1].
void unit_get_default_extent_zero_006d1f50(float* out) {
    out[0] = 0.0f;
    out[1] = 0.0f;
}

// 006D1FF0, __thiscall(ECX = unit, attacker, unused), RET 8. +700h = the float at 00F876A4 (the
// fixed-step clock, bit copy); when the attacker's [+354h]+80h kind is 2, 3, 4 or 6, +708h = +700h too.
void unit_stamp_attack_times_006d1ff0(Seg30Host& host, void* unit, void* attacker, std::uint32_t /*unused*/) {
    at<std::uint32_t>(unit, 0x700) = host.global_u32(kGameTimeGlobal);
    if (attacker == nullptr) return;
    const std::int32_t kind = at<std::int32_t>(ptr(attacker, 0x354), 0x80);
    if (kind == 2 || kind == 3 || kind == 4 || kind == 6) at<float>(unit, 0x708) = at<float>(unit, 0x700);
}

// 006D2190, __fastcall(ECX = out vec3, EDX = property bag, name, const float* fallback), RET 8,
// returns out. out = fallback (x87 copies); when the bag has the property (008F2260) its string
// value (008F0DF0) names an entity in the registry [00E188A8]+19CCh (00925A90); a found entity's
// world position +FCh..+104h replaces out, refreshing the pose (00414DB0) when +C8h is clear.
float* scene_position_from_named_entity_006d2190(Seg30Host& host, float* out, void* bag, const char* name,
                                                 const float* fallback) {
    out[0] = fallback[0];
    out[1] = fallback[1];
    out[2] = fallback[2];
    void* const property = host.scene_property_bag_find_008f2260(bag, name);
    if (property == nullptr) return out;
    const char* const value = host.scene_property_get_string_008f0df0(property);
    void* const entity = host.entity_registry_find_by_name_00925a90(ptr(host.game_e188a8(), 0x19CC), value);
    if (entity != nullptr) {
        if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        out[0] = at<float>(entity, 0xFC);
        out[1] = at<float>(entity, 0x100);
        out[2] = at<float>(entity, 0x104);
    }
    return out;
}

// 006D25F0, __thiscall(ECX unused, token), RET 4, returns AL. True when the token equals
// (00438E10 == 0) "settarget", "sendfighters", "cleartarget" or "clearorders", in that order.
bool air_field_supports_command_006d25f0(Seg30Host& host, const char* token) {
    if (host.cstring_compare_insensitive_00438e10(token, kSetTargetToken) == 0) return true;
    if (host.cstring_compare_insensitive_00438e10(token, kSendFightersToken) == 0) return true;
    if (host.cstring_compare_insensitive_00438e10(token, kClearTargetToken) == 0) return true;
    return host.cstring_compare_insensitive_00438e10(token, kClearOrdersToken) == 0;
}

// 006D2910, __thiscall(ECX = airfield, message), RET 4, returns AL. Switch on message byte +10h:
// 72h / 74h (SET runway / hangar failure) -> own vtable +204h(message), 73h / 75h (CLEAR) -> own
// vtable +208h(message), both answering true (jump table 006D296C). Any other type goes to the
// air-ops block at +72Ch (006CD6C0), and when that declines, to the unit handler 0095ABE0.
bool air_field_handle_message_006d2910(Seg30Host& host, void* airfield, void* message) {
    switch (at<std::uint8_t>(message, 0x10)) {
    case 0x72:
    case 0x74:
        host.air_field_virtual_204(airfield, message);
        return true;
    case 0x73:
    case 0x75:
        host.air_field_virtual_208(airfield, message);
        return true;
    default:
        break;
    }
    if (host.air_ops_handle_session_message_006cd6c0(add(airfield, 0x72C), message)) return true;
    return host.unit_handle_message_0095abe0(airfield, message);
}

// 006D2A90, __thiscall(ECX = airfield, float* out), RET 4, returns out. A usable hangar object
// (006D2730) answers through its vtable +C4h(out); otherwise out is the airfield's own world
// position (refreshing the pose when +C8h is clear).
float* air_field_get_world_position_006d2a90(Seg30Host& host, void* airfield, float* out) {
    void* const hangar = host.air_field_find_usable_hangar_object_006d2730(airfield);
    if (hangar != nullptr) {
        host.object_virtual_c4(hangar, out);
        return out;
    }
    if (at<std::uint8_t>(airfield, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(airfield);
    out[0] = at<float>(airfield, 0xFC);
    out[1] = at<float>(airfield, 0x100);
    out[2] = at<float>(airfield, 0x104);
    return out;
}

// 006D2C10, __thiscall(ECX = vector, const element*), RET 4. push_back of a 0Ch-byte POD with the
// 006D11E0 growth inline; a null destination skips the copy but the size grows.
void native_vector12_push_back_006d2c10(Seg30Host& host, void* vector, const void* element) {
    if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) native_vector12_grow_006d11e0(host, vector);
    void* const dst = add(ptr(vector, 0), at<std::uint32_t>(vector, 4) * 0x0C);
    if (dst != nullptr) {
        for (std::uint32_t off = 0; off < 0x0C; off += 4) {
            at<std::uint32_t>(dst, off) = at<std::uint32_t>(const_cast<void*>(element), off);
        }
    }
    at<std::uint32_t>(vector, 4) += 1;
}

// 006D2CB0, __thiscall(ECX = vector, count), RET 4. No xref. Shrinking only stores the size. Growing
// past the capacity sets capacity = count exactly and moves the elements; the new elements are
// zeroed (a null address skips the stores).
void native_vector12_resize_006d2cb0(Seg30Host& host, void* vector, std::uint32_t count) {
    if (count <= at<std::uint32_t>(vector, 4)) {
        at<std::uint32_t>(vector, 4) = count;
        return;
    }
    if (at<std::uint32_t>(vector, 8) < count) {
        at<std::uint32_t>(vector, 8) = count;
        move_vector12(host, vector, count);
    }
    for (std::uint32_t i = at<std::uint32_t>(vector, 4); i < count; ++i) {
        void* const element = add(ptr(vector, 0), i * 0x0C);
        if (element != nullptr) {
            at<std::uint32_t>(element, 0) = 0;
            at<std::uint32_t>(element, 4) = 0;
            at<std::uint32_t>(element, 8) = 0;
        }
    }
    at<std::uint32_t>(vector, 4) = count;
}

// 006D2FA0, __thiscall(ECX = unit), RET. [[unit+6F4h]+160h]+0Ch, or 0 when +6F4h is null.
std::uint32_t unit_get_6f4_field_006d2fa0(void* unit) {
    void* const object = ptr(unit, 0x6F4);
    if (object == nullptr) return 0;
    return at<std::uint32_t>(ptr(object, 0x160), 0x0C);
}

// 006D2FC0, __thiscall(ECX = airfield), RET. With +15Ch set: the +160h name, or the empty string
// at 00E1996C when +160h is null; otherwise the class type name [[+538h]+54h].
const char* air_field_get_name_006d2fc0(Seg30Host& host, void* airfield) {
    if (at<std::uint32_t>(airfield, 0x15C) != 0) {
        void* const name = ptr(airfield, 0x160);
        if (name != nullptr) return static_cast<const char*>(name);
        return host.empty_string_00e1996c();
    }
    return static_cast<const char*>(ptr(ptr(airfield, 0x538), 0x54));
}

// 006D2FF0, __thiscall(ECX = self), RET, returns self. vtable 00CF8E48; +4h = 00BD1860() (a new
// critical section).
void* air_field_lock_singleton_construct_006d2ff0(Seg30Host& host, void* self) {
    at<std::uint32_t>(self, 0) = kAirFieldLockSingletonVtable;
    at<void*>(self, 4) = host.critical_section_create_00bd1860();
    return self;
}

// 006D3040, __thiscall(ECX = node, hit record, const float* local point), RET 8. The node's +20h
// entity (pose refreshed when +C8h is clear) transforms the point by its world matrix +CCh
// (004142E0); the record takes the result at +8h..+10h, kind +30h = 0Bh, +34h = 0.
void air_field_collision_fill_hit_record_006d3040(Seg30Host& host, void* node, void* record, const float* point) {
    void* const entity = ptr(node, 0x20);
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    float world[3];
    host.vector3_transform_affine_point_004142e0(point, world, add(entity, 0xCC));
    at<float>(record, 0x08) = world[0];
    at<float>(record, 0x0C) = world[1];
    at<float>(record, 0x10) = world[2];
    at<std::uint32_t>(record, 0x30) = 0x0B;
    at<std::uint32_t>(record, 0x34) = 0;
}

// 006D30B0, __thiscall(ECX unused, hits, float distance), RET 8. Appends the part hit
// {0Bh, 0, 0, distance} through 006D2E30 (ECX = hits).
void air_field_collision_append_part_hit_006d30b0(Seg30Host& host, void* hits, float distance) {
    alignas(4) std::uint8_t hit[0x10];
    at<std::uint32_t>(hit, 0x0) = 0x0B;
    at<std::uint32_t>(hit, 0x4) = 0;
    at<std::uint32_t>(hit, 0x8) = 0;
    at<float>(hit, 0xC) = distance;
    host.hit_record_append_part_hit_006d2e30(hits, hit);
}

// 006D32F0, __cdecl(), RET, returns the singleton. Double-checked creation under the singleton
// manager's tracked lock (00415350()+10h): operator new(8) + 006D2FF0 (null on failure) published
// at 00E19954 and registered with 00BD0C30 (ECX = 00415350() again).
void* air_field_lock_singleton_get_006d32f0(Seg30Host& host) {
    if (host.global_u32(kAirFieldLockSingletonGlobal) != 0) {
        return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kAirFieldLockSingletonGlobal)));
    }
    void* const section = ptr(host.singleton_lifetime_manager_00415350(), 0x10);
    lock_enter(host, section);
    if (host.global_u32(kAirFieldLockSingletonGlobal) == 0) {
        void* object = host.operator_new_00bf681b(8);
        if (object != nullptr) object = air_field_lock_singleton_construct_006d2ff0(host, object);
        host.global_u32(kAirFieldLockSingletonGlobal) =
            static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
        host.singleton_lifetime_register_00bd0c30(host.singleton_lifetime_manager_00415350(), object);
    }
    lock_leave(host, section);
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(host.global_u32(kAirFieldLockSingletonGlobal)));
}

// 006D3440, __thiscall(ECX = entity), RET. The entity itself when non-null and IsKindOf(1Bh)
// (vtable +5Ch), else null.
void* entity_as_kind1b_006d3440(Seg30Host& host, void* entity) {
    if (entity != nullptr && host.is_kind_of_5c(entity, 0x1B)) return entity;
    return nullptr;
}

namespace {
// The world's unit list at world+offset: search for the node holding unit (+8h) from the first
// node (list+4h, linked through +4h) and erase the first match with 004837D0.
void erase_from_world_list(Seg30Host& host, void* unit, std::size_t offset) {
    void* const list = add(ptr(unit, 0x30), offset);
    for (void* node = ptr(list, 4); node != nullptr; node = ptr(node, 4)) {
        if (ptr(node, 8) == unit) {
            host.unit_list_erase_node_004837d0(list, node);
            return;
        }
    }
}
}  // namespace

// 006D35F0, __thiscall(ECX = unit), RET. 00928560, then push_back(unit) on the world lists
// [unit+30h]+30h and +48h (00484540).
void unit_register_in_world_lists_006d35f0(Seg30Host& host, void* unit) {
    host.game_entity_register_in_parent_list_00928560(unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x30), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x48), unit);
}

// 006D3670, __thiscall(ECX = unit), RET. 006D35F0's lists plus world+54h.
void unit_register_in_world_lists_006d3670(Seg30Host& host, void* unit) {
    host.game_entity_register_in_parent_list_00928560(unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x30), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x48), unit);
    host.unit_list_push_back_00484540(add(ptr(unit, 0x30), 0x54), unit);
}

// 006D36A0, __thiscall(ECX = unit), RET. 006D3620 (the early lists), then erase from world+54h.
void unit_game_object_detach_from_world_lists_006d36a0(Seg30Host& host, void* unit) {
    host.unit_detach_from_early_world_lists_006d3620(unit);
    erase_from_world_list(host, unit, 0x54);
}

// 006D3710, __thiscall(ECX = airfield), RET. 006D36A0's steps plus erase from world+354h.
void air_field_detach_from_world_lists_006d3710(Seg30Host& host, void* airfield) {
    host.unit_detach_from_early_world_lists_006d3620(airfield);
    erase_from_world_list(host, airfield, 0x54);
    erase_from_world_list(host, airfield, 0x354);
}

// 006D3770, __thiscall(ECX = singleton), RET. vtable 00CF8E48, 0041CC80 destroys the +4h section,
// clears 00E19954, base vtable 00CE3818.
void air_field_lock_singleton_destruct_006d3770(Seg30Host& host, void* singleton) {
    at<std::uint32_t>(singleton, 0) = kAirFieldLockSingletonVtable;
    host.critical_section_destroy_owned_0041cc80(add(singleton, 4));
    host.global_u32(kAirFieldLockSingletonGlobal) = 0;
    at<std::uint32_t>(singleton, 0) = kSingletonBaseVtable;
}

// 006CA410, __fastcall(ECX = block), RET. Air-ops block destructor body, members in reverse:
// vtable 00CF88A0; the three owned critical sections +4h/+8h/+0Ch (0041CC80); the +80h and +3Ch
// objects deleted through vtable +0 (flags 1); the +ECh tree erased whole (006C93F0) and its head
// freed (00BF65AC) with +F0h/+F4h cleared; the +C0h callback list (list clear +D4h, 00695870);
// the +B4h list; the +A8h buffer (00BF65AC) with +A8h..+B0h cleared; the +84h callback owner
// (buffer +98h, 00695870); the +5Ch callback list; every 58h slot record destroyed through
// vtable +0 (flags 0) then the slot buffer +4Ch freed; then 0065B590 (+40h), 00653390 (+24h) and
// 00487110 (+10h).
void air_ops_block_destruct_006ca410(Seg30Host& host, void* block) {
    at<std::uint32_t>(block, 0) = kAirOpsBlockVtable;
    host.critical_section_destroy_owned_0041cc80(add(block, 0x04));
    host.critical_section_destroy_owned_0041cc80(add(block, 0x08));
    host.critical_section_destroy_owned_0041cc80(add(block, 0x0C));
    if (ptr(block, 0x80) != nullptr) host.scalar_deleting_destructor_0(ptr(block, 0x80), 1);
    if (ptr(block, 0x3C) != nullptr) host.scalar_deleting_destructor_0(ptr(block, 0x3C), 1);
    void* const tree = add(block, 0xEC);
    void* const head = ptr(tree, 4);
    CheckedIter erased{};
    host.tree_erase_range_006c93f0(tree, &erased, CheckedIter{tree, ptr(head, 0)}, CheckedIter{tree, head});
    host.free_00bf65ac(ptr(tree, 4));
    at<void*>(tree, 4) = nullptr;
    at<std::uint32_t>(tree, 8) = 0;
    host.list_clear_006c1430(add(block, 0xD4));
    host.callback_owner_destroy_00695870(add(block, 0xC0));
    host.list_clear_006c1430(add(block, 0xB4));
    if (ptr(block, 0xA8) != nullptr) host.free_00bf65ac(ptr(block, 0xA8));
    at<std::uint32_t>(block, 0xA8) = 0;
    at<std::uint32_t>(block, 0xAC) = 0;
    at<std::uint32_t>(block, 0xB0) = 0;
    if (ptr(block, 0x98) != nullptr) host.free_00bf6989(ptr(block, 0x98));
    host.callback_owner_destroy_00695870(add(block, 0x84));
    host.list_clear_006c1430(add(block, 0x70));
    host.callback_owner_destroy_00695870(add(block, 0x5C));
    if (ptr(block, kBlockSlots) != nullptr) {
        for (std::uint32_t i = 0; i < at<std::uint32_t>(block, kBlockSlotCount); ++i) {
            host.scalar_deleting_destructor_0(add(ptr(block, kBlockSlots), i * kSlotStride), 0);
        }
        host.free_00bf6989(ptr(block, kBlockSlots));
    }
    host.list_tidy_0065b590(add(block, 0x40));
    host.observer_member_cleanup_00653390(add(block, 0x24));
    host.award_index_list_destroy_00487110(add(block, 0x10));
}

// 006D3B10, __thiscall(ECX = airfield), RET. MAirfield destructor body: the eight vtables
// (+0h 00CF8C08, +10h 00CF8BEC, +24h 00CF8BE4, +170h 00CF8BE0, +1E4h 00CF8BD8, +310h 00CF8BC0,
// +38Ch 00CF8BBC, +72Ch 00CF8BA0); with a +82Ch buffer, [+828h]+F8h = 0 and the buffer is freed
// (00BF65AC, not cleared); the spatial index singleton (0042E630) detaches the +828h node
// (0098A500, even when null) which is then deleted through vtable +4h (flags 1) when non-null;
// +830h freed (00BF6989); then 006CA410 on the +72Ch block and 00959940 on the unit.
void air_field_destruct_006d3b10(Seg30Host& host, void* airfield) {
    static constexpr std::pair<std::size_t, std::uint32_t> kVtables[] = {
        {0x000, 0x00CF8C08}, {0x010, 0x00CF8BEC}, {0x024, 0x00CF8BE4}, {0x170, 0x00CF8BE0},
        {0x1E4, 0x00CF8BD8}, {0x310, 0x00CF8BC0}, {0x38C, 0x00CF8BBC}, {0x72C, 0x00CF8BA0}};
    for (const auto& [offset, vtable] : kVtables) at<std::uint32_t>(airfield, offset) = vtable;
    if (ptr(airfield, 0x82C) != nullptr) {
        at<std::uint32_t>(ptr(airfield, 0x828), 0xF8) = 0;
        host.free_00bf65ac(ptr(airfield, 0x82C));
    }
    host.spatial_index_detach_node_0098a500(host.spatial_index_singleton_0042e630(), ptr(airfield, 0x828));
    if (ptr(airfield, 0x828) != nullptr) host.object_virtual_04_delete(ptr(airfield, 0x828), 1);
    if (ptr(airfield, 0x830) != nullptr) host.free_00bf6989(ptr(airfield, 0x830));
    air_ops_block_destruct_006ca410(host, add(airfield, 0x72C));
    host.unit_game_object_destruct_00959940(airfield);
}

// 006D4210, __thiscall(ECX = AIRFIELD_CREATE message), RET, returns the entity. The class for the
// word id +1Eh (00964790, DL = 1) creates the entity for the word +1Ch through its vtable +28h;
// the name +C0h (strlen, 0 when null) is copied into the entity's string +154h: 0041DD40(length,
// 0) then memcpy(data, name, string length) when the data pointer is non-null.
void* airfield_create_message_create_entity_006d4210(Seg30Host& host, void* message) {
    void* const vehicle_class = host.vehicle_class_get_or_create_00964790(at<std::uint16_t>(message, 0x1E), true);
    void* const entity = host.class_create_entity_28(vehicle_class, at<std::uint16_t>(message, 0x1C));
    const char* const name = static_cast<const char*>(ptr(message, 0xC0));
    void* const text = add(entity, 0x154);
    std::uint32_t length = 0;
    if (name != nullptr) {
        while (name[length] != '\0') ++length;
    }
    host.native_string_resize_0041dd40(text, length, 0);
    if (ptr(text, 4) != nullptr) host.memcpy_00bf7680(ptr(text, 4), name, at<std::uint32_t>(text, 0));
    return entity;
}

// 006CD550, __thiscall(ECX = block, slot, mode), RET 8, returns AL. No xref (twin of 006BD230).
// has = the owner's (+7Ch) vtable +114h object answering vtable +18h (0 without one). The wanted
// state w: mode 0 -> 3 when the slot is fillable (state 1/5 with planes, or 4) else 2; mode 1 ->
// 1 or 2 the same way; any other mode -> 0. False when the slot has no class or w == 3 without
// has. Then by slot state: 5 -> true; 1 and 4 -> w != 2; 2 -> true when w == 2, else when
// (slot+4Ch == 0) differs from (has == 0); 3 -> true when w == 2, else 006BC5E0(slot+28h squadron)
// true -> w == 3, false -> w == 1; any other state -> false.
bool air_ops_slot_state_from_squadron_command_006cd550(Seg30Host& host, void* block, int slot, int mode) {
    void* const command = host.object_virtual_114(ptr(block, kBlockOwner));
    const std::int32_t has = command == nullptr ? 0 : host.object_virtual_18_int(command);
    auto fillable = [&] {
        void* const record = slot_at(block, slot);
        const std::int32_t state = at<std::int32_t>(record, kSlotState);
        return ((state == 1 || state == 5) && at<std::int32_t>(record, kSlotCount) > 0) || state == 4;
    };
    std::int32_t wanted = 0;
    if (mode == 0) {
        wanted = fillable() ? 3 : 2;
    } else if (mode == 1) {
        wanted = fillable() ? 1 : 2;
    }
    void* const record = slot_at(block, slot);
    if (ptr(record, kSlotClass) == nullptr) return false;
    if (wanted == 3 && has == 0) return false;
    switch (at<std::int32_t>(record, kSlotState)) {
    case 5:
        return true;
    case 1:
    case 4:
        return wanted != 2;
    case 2:
        if (wanted == 2) return true;
        return (at<std::int32_t>(record, 0x4C) == 0) != (has == 0);
    case 3:
        if (wanted == 2) return true;
        if (host.air_ops_squadron_target_within_range_006bc5e0(ptr(record, 0x28))) return wanted == 3;
        return wanted == 1;
    default:
        return false;
    }
}

// 006C83B0, __thiscall(ECX = block, message), RET 4. Case 84h of 006CD6C0. Message fields: +20h
// slot index, +24h new state, +28h class id, +2Ch count, +30h slot +10h value.
// When the index is past the slot count (+50h): under the +0Ch lock, re-checked, append default
// slot records until the index fits. Each default is the 58h stack record 006C1960 builds (owner
// = block), pushed with the 006C6C60 growth inline and copy-constructed (0065CAF0), then the
// temporary is torn down inline: vtables 00CF6564 / 00CF64AC at +0h/+38h, the +38h member's
// target (+4Ch) unregistered, 00695870 on +38h, vtable 00CF6494 at +14h, the +14h member's target
// (+28h) unregistered, 00695870 on +14h.
// Then the slot: class from 007B8A80 (the thunk that sets DL = 1 for 00964790) or null; on a class
// change +4h = class and +10h = class+134h; +8h = count; +10h = message+30h (overwrites); when the
// old state is 2 and the new one 3, +0Ch = count; +2Ch = state; float +30h = 0 or, with byte +34h
// set, 5.0f (00CE3850) and +34h cleared; the +14h member retargets (+28h) to 006BD620(message);
// 006BF150 publishes the slot.
void air_ops_apply_slot_record_message84_006c83b0(Seg30Host& host, void* block, void* message) {
    const std::uint32_t index = at<std::uint32_t>(message, 0x20);
    if (index >= at<std::uint32_t>(block, kBlockSlotCount)) {
        void* const section = ptr(block, 0x0C);
        lock_enter(host, section);
        if (index >= at<std::uint32_t>(block, kBlockSlotCount)) {
            do {
                alignas(4) std::uint8_t record[kSlotStride];
                support_manager_item_construct_006c1960(record, static_cast<std::uint32_t>(
                                                                    reinterpret_cast<std::uintptr_t>(block)));
                at<float>(record, 0x30) = 0.0f;
                native_vector58_push_back_006c6c60(host, add(block, kBlockSlots), record);
                at<std::uint32_t>(record, 0x00) = kSupportItemVtable0;
                at<std::uint32_t>(record, 0x38) = kSlotMember38DestroyVtable;
                if (ptr(record, 0x4C) != nullptr) {
                    host.observer_unregister_pair_006952a0(ptr(record, 0x4C), add(record, 0x38));
                }
                host.callback_owner_destroy_00695870(add(record, 0x38));
                at<std::uint32_t>(record, 0x14) = kSlotMember14DestroyVtable;
                if (ptr(record, 0x28) != nullptr) {
                    host.observer_unregister_pair_006952a0(ptr(record, 0x28), add(record, 0x14));
                }
                host.callback_owner_destroy_00695870(add(record, 0x14));
            } while (index >= at<std::uint32_t>(block, kBlockSlotCount));
        }
        lock_leave(host, section);
    }
    const std::int32_t state = at<std::int32_t>(message, 0x24);
    void* const slot = slot_at(block, static_cast<int>(index));
    const std::int32_t count = at<std::int32_t>(message, 0x2C);
    void* vehicle_class = nullptr;
    if (at<std::uint32_t>(message, 0x28) != 0) {
        vehicle_class = host.vehicle_class_get_or_create_00964790(at<std::uint32_t>(message, 0x28), true);
    }
    if (ptr(slot, kSlotClass) != vehicle_class) {
        at<void*>(slot, kSlotClass) = vehicle_class;
        at<std::uint32_t>(slot, kSlotClassField134) =
            vehicle_class == nullptr ? 0u : at<std::uint32_t>(vehicle_class, 0x134);
    }
    const bool was_state2 = at<std::int32_t>(slot, kSlotState) == 2;
    at<std::int32_t>(slot, kSlotCount) = count;
    at<std::uint32_t>(slot, kSlotClassField134) = at<std::uint32_t>(message, 0x30);
    if (was_state2 && state == 3) at<std::int32_t>(slot, 0x0C) = count;
    at<std::int32_t>(slot, kSlotState) = state;
    at<float>(slot, 0x30) = 0.0f;
    if (at<std::uint8_t>(slot, 0x34) != 0) {
        at<float>(slot, 0x30) = kSlotRelaunchDelay;
        at<std::uint8_t>(slot, 0x34) = 0;
    }
    void* const squadron = host.air_ops_slot_squadron_handle_006bd620(message);
    void* const member = add(slot, 0x14);
    if (ptr(member, 0x14) != squadron) {
        if (ptr(member, 0x14) != nullptr) host.observer_unregister_pair_006952a0(ptr(member, 0x14), member);
        at<void*>(member, 0x14) = squadron;
        if (squadron != nullptr) host.observer_register_pair_00694a60(squadron, member);
    }
    host.publish_slot_006bf150(block, static_cast<int>(index));
}

// 006D37E0, __thiscall(ECX = airfield, archive), RET 4. Slot +A8h of vtable 00CF8C08. Base
// 0095BB10, then section "_airBase" visited by 006C19D0 (ECX = +72Ch block); section "_airField":
// "runwayWidth" / "runwayLength" (kind 2, the float bits of [+7ACh]+B0h / +B4h, 0.0 when +7ACh is
// null), section "hangars" holding per hangar i (0Ch records at +830h, count +834h) a section
// {1, i} with "entityID" {4, record+0h}, "entryPathID" {4, 007ACA30(record+4h)}, "exitPathID"
// {4, 007ACA30(record+8h)}; then section "gameUnit" visited by 008360C0 (ECX = the airfield);
// the "gameUnit" and "_airField" sections are closed last.
void air_field_serialize_006d37e0(Seg30Host& host, void* airfield, void* archive) {
    auto key = [](const char* text) { return ArchiveArg{0, reinterpret_cast<std::uintptr_t>(text)}; };
    host.vehicle_unit_load_archive_common_0095bb10(airfield, archive);
    host.archive_open_04(archive, key(kAirBaseSection));
    host.air_ops_block_declare_lua_properties_006c19d0(add(airfield, 0x72C), archive);
    host.archive_close_08(archive);
    host.archive_open_04(archive, key(kAirFieldSection));
    void* holder = ptr(airfield, 0x7AC);
    host.archive_write_0c(archive, key(kRunwayWidthKey),
                          ArchiveArg{2, holder != nullptr ? at<std::uint32_t>(holder, 0xB0) : 0u});
    holder = ptr(airfield, 0x7AC);
    host.archive_write_0c(archive, key(kRunwayLengthKey),
                          ArchiveArg{2, holder != nullptr ? at<std::uint32_t>(holder, 0xB4) : 0u});
    host.archive_open_04(archive, key(kHangarsKey));
    for (std::uint32_t i = 0; i < at<std::uint32_t>(airfield, 0x834); ++i) {
        void* const record = add(ptr(airfield, 0x830), i * 0x0C);
        host.archive_open_04(archive, ArchiveArg{1, i});
        host.archive_write_0c(archive, key(kEntityIdKey), ArchiveArg{4, at<std::uint32_t>(record, 0)});
        host.archive_write_0c(archive, key(kEntryPathIdKey),
                              ArchiveArg{4, reinterpret_cast<std::uintptr_t>(
                                                host.path_object_get_entity_007aca30(ptr(record, 4)))});
        host.archive_write_0c(archive, key(kExitPathIdKey),
                              ArchiveArg{4, reinterpret_cast<std::uintptr_t>(
                                                host.path_object_get_entity_007aca30(ptr(record, 8)))});
        host.archive_close_08(archive);
    }
    host.archive_close_08(archive);
    host.archive_open_04(archive, key(kGameUnitSection));
    host.weapon_director_visit_properties_008360c0(airfield, archive);
    host.archive_close_08(archive);
    host.archive_close_08(archive);
}

}  // namespace bsp::cc13_seg30
