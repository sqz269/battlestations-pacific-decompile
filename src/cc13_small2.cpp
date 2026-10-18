// cc13 small-row packet 2 (packet cc13-small2). See include/bsp/cc13_small2.hpp and
// docs/CC13_SMALL2.md for the evidence and the adoption notes. Instruction addresses in the
// comments are the image's; names are hypotheses, not recovered symbols.

#include "bsp/cc13_small2.hpp"

#include <cmath>
#include <cstring>
#include <initializer_list>
#include <set>

namespace bsp::cc13_small2 {

namespace {

float global_float(Small2Host& host, std::uint32_t address) {
    return *static_cast<const float*>(host.global_address(address));
}

std::int32_t global_int(Small2Host& host, std::uint32_t address) {
    return *static_cast<const std::int32_t*>(host.global_address(address));
}

std::int32_t session_mode(Small2Host& host) {
    return at<std::int32_t>(host.global_ptr(kGame), 0x1FE4);
}

}  // namespace

// ---------------------------------------------------------------------------
// Segment 43.
// ---------------------------------------------------------------------------

// 00778820 __thiscall(holder, entity) RET 4: holder[0] = entity->vtable[114h]() (the weapon
// director); returns the holder. Called by 00780649 for entity message 59h.
void* entity_weapon_director_ref_construct_00778820(void* holder, void* entity,
                                                   Small2Host& host) {
    at<void*>(holder, 0) = host.entity_weapon_director_114(entity);   // 0077882F
    return holder;
}

// 00778860 __thiscall(unit) RET: the weapon director's path object 0, or null without one.
void* unit_director_path_object0_00778860(void* unit, Small2Host& host) {
    void* const director = host.entity_weapon_director_114(unit);     // 00778868
    if (director == nullptr) return nullptr;                           // 0077886C
    return host.weapon_director_slot_path_object_0071bfc0(director, 0);  // 00778872
}

// 00779EB0 __thiscall(entity, Record** record) RET 4, float in ST0. The record's point
// (+ACh) is copied before the pose refresh; the entity's world position is +FCh..+104h.
// At the main thread's 24-bit precision every x87 step is a float store or a float-rounded
// product, so float arithmetic reproduces it; a fixed-step job at 53 bits would round the
// products and the sum once, at the FSTP (00779F39), instead (uncertain which applies).
// Sum order from the listing: (dx*dx + dy*dy) + dz*dz.
float unit_distance_to_record_00779eb0(void* entity, void* const* record, Small2Host& host) {
    const float px = at<float>(*record, 0xAC);                         // 00779EB9
    const float py = at<float>(*record, 0xB0);                         // 00779ECB
    const float pz = at<float>(*record, 0xB4);                         // 00779EE0
    if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const float dx = px - at<float>(entity, 0xFC);                     // 00779EF6
    const float dy = py - at<float>(entity, 0x100);                    // 00779F04
    const float dz = pz - at<float>(entity, 0x104);                    // 00779F12
    const float sq = (dx * dx + dy * dy) + dz * dz;                    // 00779F29..00779F39
    // 00779F47 FCOMI / JBE: at or below 1e-10, or unordered, gives 0.
    if (!(static_cast<double>(sq) > kMinDistanceSquared)) return 0.0f;
    return static_cast<float>(std::sqrt(static_cast<double>(sq)));    // 00779F4D 00BF7030
}

// 0077A080 __thiscall(msg, group) RET 4, EAX = msg: MT_FORMATION_SET (78h). Header via
// 0075B430(78h), then vtable 00CED548, +20h = group+500h, +24h = n = group+4F8h, and two
// new[] arrays of n 16-byte rows (+28h from member record +10h..+1Ch, +2Ch from +20h..+2Ch).
// The new[] size is n*16 with the image's overflow guard (MUL / SETO / NEG / OR -> FFFFFFFFh).
void* session_message_formation_set_construct_0077a080(void* msg, void* group,
                                                       Small2Host& host) {
    host.session_message_construct_base_0075b430(msg, kMsgFormationSet);       // 0077A0A1
    at<std::uint16_t>(msg, 0x18) = 0;                                          // 0077A0A8
    at<std::uint8_t>(msg, 0x1A) = 0;                                           // 0077A0AC
    at<std::uint32_t>(msg, 0x4) = 1;                                           // 0077A0AF
    at<std::uint8_t>(msg, 0x1C) = 0;                                           // 0077A0B6
    at<std::uint32_t>(msg, 0x0) = kMsgFormationSetVtable;                      // 0077A0BD
    at<std::uint32_t>(msg, 0x20) = at<std::uint32_t>(group, 0x500);            // 0077A0C3
    at<std::uint32_t>(msg, 0x24) = at<std::uint32_t>(group, 0x4F8);            // 0077A0CC
    const auto array_bytes = [](std::uint32_t n) -> std::uint32_t {
        const std::uint64_t bytes = static_cast<std::uint64_t>(n * 4u) * 4u;  // ADD, ADD, MUL 4
        return (bytes >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
    };
    at<void*>(msg, 0x28) = host.operator_new_array_00bf55be(
        array_bytes(at<std::uint32_t>(msg, 0x24)));                            // 0077A0EE
    at<void*>(msg, 0x2C) = host.operator_new_array_00bf55be(
        array_bytes(at<std::uint32_t>(msg, 0x24)));                            // 0077A10E
    for (std::uint32_t i = 0; i < at<std::uint32_t>(msg, 0x24); ++i) {        // 0077A116 / 0077A17D JC
        void* const record = host.group_member_record_at_0070d070(group, i);  // 0077A124
        float* const a = static_cast<float*>(ptr(msg, 0x28)) + i * 4;
        float* const b = static_cast<float*>(ptr(msg, 0x2C)) + i * 4;
        for (int k = 0; k < 4; ++k) {                                          // 0077A129..0077A179
            a[k] = at<float>(record, 0x10 + 4 * k);
            b[k] = at<float>(record, 0x20 + 4 * k);
        }
    }
    return msg;
}

// 0077A650 __thiscall(unit) RET. Callers (00826D5A, 007F3D1C) run it only when unit+308h
// is not 0.0f; 0077F7B0 is the only writer of a non-zero value.
void unit_expire_at_scheduled_time_0077a650(void* unit, Small2Host& host) {
    const float due = at<float>(unit, 0x308);                          // 0077A653
    const float clock = global_float(host, kClock);                    // 0077A659
    // 0077A65F FCOMIP ST0(clock), ST1(due); JC: before the due time, or unordered.
    if (!(clock >= due)) return;
    void* const game = host.global_ptr(kGame);                         // 0077A665
    if (at<std::int32_t>(game, 0x1FE4) == kSessionModeClient) return;  // 0077A66A
    host.expiry_notice_0090ebf0(ptr(game, 0x21A0), unit);              // 0077A67A
    host.mission_entity_kill_00926d90(unit, kKillReasonExpired);       // 0077A683
}

// 0077B050 __thiscall(entity, TargetPoint* out) RET 4, entity vtable slot D4h. Two calls to
// vtable[178h] (the command target descriptor); the first call's scratch is reused as the
// out buffer of 006F7DD0.
void entity_update_command_target_position_0077b050(void* entity, TargetPoint* out,
                                                    Small2Host& host) {
    alignas(4) std::uint8_t first[0x18];
    alignas(4) std::uint8_t second[0x18];
    void* const target = host.entity_command_target_178(entity, first);   // 0077B065
    if (at<std::uint8_t>(target, 1) == 0 && at<std::uint8_t>(target, 0) == 0) {  // 0077B067
        out->valid = 0;                                                    // 0077B0B4
        return;
    }
    void* const again = host.entity_command_target_178(entity, second);   // 0077B086
    const float* const point =
        host.target_descriptor_resolve_position_006f7dd0(again, first);   // 0077B08A
    out->point[0] = point[0];                                              // 0077B097
    out->valid = 1;                                                        // 0077B099
    out->point[1] = point[1];                                              // 0077B0A1
    out->point[2] = point[2];                                              // 0077B0A7
}

// 0077C540 __cdecl, RET. Zero the 64 bucket counts, let every flagged net entity queue its
// kind-8 children (vtable[180h], which fills the buckets), drain the buckets from 63 down to
// 0 through receiver->vtable[188h](arg, byte), then store tick % 12 at 00F87150.
// The checked-iterator reports at 0077C587 / 0077C595 / 0077C5BC / 0077C5D1 sit behind
// tests the loop has already passed (a self-compare, or a node already known not to be the
// end), so they cannot fire and are left out.
void replication_flush_queued_rows_0077c540(Small2Host& host) {
    auto* const counts = static_cast<std::int32_t*>(host.global_address(kFlushCounts));
    for (int b = 0; b < kFlushBuckets; ++b) counts[b] = 0;                  // 0077C553
    for (void* node = ptr(host.global_ptr(kNetEntityListHead), 0);          // 0077C555
         node != host.global_ptr(kNetEntityListHead);                       // 0077C560
         node = ptr(node, 0)) {                                             // 0077C5D6
        void* const entity = ptr(node, 8);                                  // 0077C568
        if (at<std::uint8_t>(entity, 0x5C) == 0) continue;                  // 0077C56B
        // The child list: container entity+2A4h, head re-read each pass at +2A8h.
        for (void* child = ptr(ptr(entity, 0x2A8), 0);                      // 0077C571
             child != ptr(entity, 0x2A8);                                   // 0077C582
             child = ptr(child, 0)) {                                       // 0077C5C1
            void* const element = add(child, 8);                            // 0077C5A4
            if (at<std::int32_t>(ptr(ptr(element, 0), 0x50), 0xC) == kChildKindQueued)
                host.entity_queue_child_180(entity, element);               // 0077C5B5
        }
    }
    const std::int32_t tick = global_int(host, kTick);                      // 0077C5E0
    auto* const bytes = static_cast<const std::uint8_t*>(host.global_address(kFlushBytes));
    auto* const args = static_cast<const std::uint32_t*>(host.global_address(kFlushArgs));
    auto* const receivers = static_cast<void* const*>(host.global_address(kFlushReceivers));
    for (int b = kFlushBuckets - 1; b >= 0; --b) {                          // 0077C63F..0077C658
        for (std::int32_t i = 0; i < counts[b]; ++i) {                      // 0077C602 / 0077C632
            const std::size_t row = static_cast<std::size_t>(b) * 0x300 + i;
            host.net_flush_row_188(receivers[row], args[row], bytes[row]);  // 0077C62A
        }
    }
    // 0077C65A..0077C673: signed tick / 12 by multiply, then the low byte of the remainder.
    at<std::uint8_t>(host.global_address(kFlushPhase), 0) =
        static_cast<std::uint8_t>(tick % 12);
}

// 0077C880 __thiscall(owner, msg) RET 4. Outside client mode the message goes through
// 0077C7B0(owner; msg, 0); in client mode the owner's handle (+174h) is stamped at msg+18h
// and the message is sent to the secondary first peer (tail JMP 00770AF0).
void unit_group_broadcast_formation_0077c880(void* owner, void* msg, Small2Host& host) {
    if (session_mode(host) != kSessionModeClient) {                         // 0077C885
        host.session_owner_broadcast_0077c7b0(owner, msg, 0);               // 0077C895
        return;
    }
    at<std::uint16_t>(msg, 0x18) = at<std::uint16_t>(owner, 0x174);        // 0077C89D..0077C8A8
    host.session_send_secondary_first_peer_00770af0(add(host.global_ptr(kGame), 0x1EF0), msg);
}

// 0077CD80 __thiscall(unit, LuaObject* out) RET 4, returns out:
// out = globals.thisTable[tostring(word unit+18h)].
LuaObject* unit_lua_this_table_entry_0077cd80(void* unit, LuaObject* out, Small2Host& host) {
    NativeString key{0, nullptr};
    host.native_string_from_int_004260b0(&key, at<std::uint16_t>(unit, 0x18));   // 0077CDA9
    LuaObject globals{};
    LuaObject table{};
    void* const g = host.lua_globals_00b67980(add(host.global_ptr(kGame), 0x1A0C),
                                              &globals);                    // 0077CDC9
    void* const t = host.lua_get_by_name_00b67800(
        g, &table, static_cast<const char*>(host.global_address(kThisTableName)));  // 0077CDDF
    host.lua_get_by_native_string_00b68100(t, out, &key);                    // 0077CDF1
    host.lua_destroy_00b67700(&table);                                       // 0077CE07
    host.lua_destroy_00b67700(&globals);                                     // 0077CE15
    if (key.data != nullptr) host.native_string_free_storage(&key);          // 0077CE1E..0077CE39
    return out;
}

// 0077D270 __thiscall(unit) RET, the unit override of mission-entity slot 7Ch. Clears the
// game's +193Ch byte, marks the recon slot of each of the three +1F0h rows (34h apart) with a
// positive count (row+8 selects row+0 or row-4), and when the kill cause +70h is 1 ("harm";
// 00926C80 writes 1 at 00926CF0 or copies the parent's cause at 00926CEB; 0077A650 kills
// with cause 3) and there is a script handle (word +2C8h) sets self.LastBanto = thisTable
// entry of the +2B0h record under the mission-entity lock (critical section [lock+4], depth +18h). Then 00929B60.
void unit_instance_on_killed_lua_0077d270(void* unit, Small2Host& host) {
    at<std::uint8_t>(host.global_ptr(kGame), 0x193C) = 0;                      // 0077D295
    for (std::int32_t i = 0; i < 3; ++i) {                                     // 0077D29C..0077D2C4
        const std::size_t row = 0x1F0 + 0x34 * static_cast<std::size_t>(i);
        const std::int32_t count = at<std::uint8_t>(unit, row + 8) != 0
                                       ? at<std::int32_t>(unit, row)
                                       : at<std::int32_t>(unit, row - 4);
        if (count > 0) host.recon_mark_slot_dirty_00803ba0(i);                 // 0077D2B6
    }
    if (at<std::int32_t>(unit, 0x70) == 1 && at<std::uint16_t>(unit, 0x2C8) != 0) {  // 0077D2CB / 0077D2D4
        void* const section = ptr(host.mission_entity_lock_get_004c1570(), 4);  // 0077D2E1
        if (section != nullptr) {                                              // guard, vtable 00CE37FC
            host.enter_critical_section_iat_00ce2218(section);                // 0077D2FA
            at<std::int32_t>(section, 0x18) += 1;                              // 0077D300
        }
        NativeString name{0, nullptr};
        host.native_string_resize_0041dd40(&name, 9, 1);                       // 0077D316
        if (name.data != nullptr)
            std::memcpy(name.data, host.global_address(kLastBantoName), name.length + 1);  // 0077D331
        LuaObject entry{};
        LuaObject self{};
        LuaObject* const value =
            unit_lua_this_table_entry_0077cd80(add(unit, 0x2B0), &entry, host);   // 0077D349
        void* const self_obj = host.mission_entity_lua_self_object_00927b40(unit, &self);
        host.lua_set_object_00b675d0(self_obj, &name, value);                  // 0077D36F
        host.lua_destroy_00b67700(&self);                                      // 0077D37C
        host.lua_destroy_00b67700(&entry);                                     // 0077D38A
        if (name.data != nullptr) host.native_string_free_storage(&name);      // 0077D38F..0077D3AE
        if (section != nullptr) {
            at<std::int32_t>(section, 0x18) -= 1;                              // 0077D3BE
            host.leave_critical_section_iat_00ce2210(section);                // 0077D3C2
        }
    }
    host.mission_entity_on_killed_lua_00929b60(unit);                          // 0077D3CA
}

// 0077D3F0 __thiscall(entity, const NativeString* name, arg) RET 8. Copies the name into
// entity+15Ch, makes the (compiled-out) log call, and in server mode broadcasts a SETGUINAME
// message built from a by-value copy of the name (the constructor owns that copy).
// native_string_assign_0041dd40 folds two image calls: 0041DD40(dst; src.length, 1), then
// the 00BF7680 memcpy(dst.data, src.data, dst.length) when src.length is non-zero. The
// SETGUINAME constructor hook can forward to mp_seg43_cc13's
// message_setguiname_construct_0077ccc0 (the length/chars split of the by-value string).
void entity_set_gui_name_0077d3f0(void* entity, const NativeString* name, std::uint32_t arg,
                                  Small2Host& host) {
    auto* const own = static_cast<NativeString*>(add(entity, 0x15C));
    if (own != name) host.native_string_assign_0041dd40(own, *name);        // 0077D417..0077D437
    const char* text = own->data;                                            // 0077D43F
    if (text == nullptr) text = static_cast<const char*>(host.global_address(kEmptyText));
    host.log_noop_004254b0(static_cast<const char*>(host.global_address(kSetGuiNameFormat)),
                           host.entity_name_10(entity), text);               // 0077D453 / 0077D45B
    if (session_mode(host) != kSessionModeServer) return;                    // 0077D469
    NativeString copy{0, nullptr};                                           // 0077D484
    if (&copy != name) host.native_string_assign_0041dd40(&copy, *name);     // 0077D47E..0077D4AF
    alignas(4) std::uint8_t msg[0x2C];
    void* const built = host.msg_setguiname_construct_0077ccc0(msg, copy, arg);  // 0077D4BB
    host.session_owner_broadcast_0077c7b0(entity, built, 0);                 // 0077D4CB
    host.msg_setguiname_destroy_00765b70(msg);                               // 0077D4DC
}

// 0077D7F0 __cdecl, RET. With a camera target (game+19FCh) send a CAMERA message to the
// secondary first peer; then, when the controlled unit (kind 0Fh) or its +3D0h owner
// (kind 18h) holds local player role 1, pass its first local record (or null) to
// vtable[184h].
void replication_step_local_player_record_0077d7f0(Small2Host& host) {
    void* const game = host.global_ptr(kGame);                               // 0077D805
    if (game != nullptr && ptr(game, 0x19FC) != nullptr) {                   // 0077D80E / 0077D812
        void* const fe = host.global_ptr(kFrontEnd);                         // 0077D81B
        const bool flag = fe != nullptr && ptr(fe, 0x54) != nullptr &&
                          at<std::uint8_t>(ptr(fe, 0x54), 4) != 0;           // 0077D820..0077D835
        alignas(4) std::uint8_t msg[0x38];
        void* const built =
            host.msg_camera_construct_00764960(msg, at<std::uint32_t>(game, 0x19FC), flag);
        host.session_send_secondary_first_peer_00770af0(add(host.global_ptr(kGame), 0x1EF0),
                                                        built);              // 0077D85F
        host.msg_camera_destroy_0075b420(msg);                               // 0077D86F
    }
    if (host.global_ptr(kControlledUnit) == nullptr) return;                 // 0077D874
    void* unit = nullptr;
    if (host.entity_is_kind_5c(host.global_ptr(kControlledUnit), kUnitKind0F)) {   // 0077D886
        unit = host.global_ptr(kControlledUnit);                             // 0077D88C
    } else if (host.entity_is_kind_5c(host.global_ptr(kControlledUnit), kUnitKind18)) {
        unit = ptr(host.global_ptr(kControlledUnit), 0x3D0);                 // 0077D8AD
    } else {
        return;
    }
    if (unit == nullptr) return;                                             // 0077D8B3
    if (!host.unit_is_local_player_role_00927f30(unit, 1)) return;           // 0077D8BB
    void* record = nullptr;
    if (at<std::uint32_t>(unit, 0x2AC) != 0) {                               // 0077D8C4
        void* const head = ptr(unit, 0x2A8);
        void* const first = ptr(head, 0);
        if (first == head) host.invalid_parameter_00bf6713();                // 0077D8DE
        record = add(first, 8);                                              // 0077D8E3
    }
    host.entity_send_local_record_184(unit, record);                         // 0077D8F2
}

// 0077F5E0 __cdecl, RET. Server mode with the pending byte set: for every net entity, walk
// its child queue (container +298h, head +29Ch, size +2A0h), trace each child through
// vtable[17Ch] when 00E0AF20 is set, then empty the queue and free its nodes. The child
// keys (child+50h) go into a local std::set that nothing reads before it is destroyed.
// The checked-iterator reports (0077F678 / 0077F689 / 0077F693 / 0077F6BB / 0077F6C8 /
// 0077F6D2 / 0077F70F / 0077F750) sit behind container self-compares or a node already known
// not to be the end, so they cannot fire and are omitted.
void replication_drain_child_queues_0077f5e0(Small2Host& host) {
    if (session_mode(host) != kSessionModeServer) return;                    // 0077F603
    if (at<std::uint8_t>(host.global_address(kNetChildQueuePending), 0) == 0) return;
    std::set<std::uint32_t> keys;                                            // 0077F623 0077BC30
    for (void* node = ptr(host.global_ptr(kNetEntityListHead), 0);           // 0077F649
         node != host.global_ptr(kNetEntityListHead);                        // 0077F668 / 0077F67D
         node = ptr(node, 0)) {                                              // 0077F755
        void* const entity = ptr(node, 8);                                   // 0077F698
        for (void* child = ptr(ptr(entity, 0x29C), 0);                       // 0077F69B
             child != ptr(entity, 0x29C);                                    // 0077F6B2 / 0077F6C0
             child = ptr(child, 0)) {                                        // 0077F714
            void* const value = ptr(child, 8);                               // 0077F6DE
            if (at<std::uint8_t>(host.global_address(kNetTraceEnabled), 0) != 0)
                host.entity_trace_child_17c(entity, value);                  // 0077F6EE
            keys.insert(at<std::uint32_t>(value, 0x50));                     // 0077F705 0077E500
        }
        // 0077F718..0077F741: unlink every node from the head, zero the size, free the nodes.
        void* const head = ptr(entity, 0x29C);
        void* n = ptr(head, 0);
        at<void*>(head, 0) = head;
        at<void*>(ptr(entity, 0x29C), 4) = ptr(entity, 0x29C);
        at<std::uint32_t>(entity, 0x2A0) = 0;
        while (n != ptr(entity, 0x29C)) {
            void* const next = ptr(n, 0);
            host.crt_free_00bf65ac(n);                                       // 0077F734
            n = next;
        }
    }
    at<std::uint8_t>(host.global_address(kNetChildQueuePending), 0) = 0;     // 0077F770
}   // 0077F78A 0077E740 / 0077F794 _free: the set goes away here.

// 0077FE80 __thiscall(entity, msg, bool* out) RET 8, AL. The fall-through of 00780120:
// msg->vtable[10h]() false -> *out = 0 (when out) and 0; else the type byte msg+10h,
// 53h..78h, picks an arm through the byte table 0078005C and the targets at 00780044, and
// anything else (or a table entry of 5) returns entity->vtable[164h](msg). `out` is
// written only on the not-ready path. 00521E30 resolves the 16-bit handle in CX; at
// 0077FEC8 ECX's high half still holds the message pointer's (the routine reads CX only).
bool session_dispatch_entity_kind_message_0077fe80(void* entity, void* msg, bool* out,
                                                   Small2Host& host) {
    if (!host.message_is_ready_10(msg)) {                                       // 0077FE8F
        if (out != nullptr) *out = false;                                       // 0077FE9D
        return false;
    }
    switch (at<std::uint8_t>(msg, 0x10)) {                                      // 0077FEAA
    case 0x53: {  // MT_GIVEUNIT                                                // 0077FFAF
        const std::uint32_t player = at<std::uint32_t>(msg, 0x20);
        void* const slot = at<void*>(host.global_ptr(kGame),
                                     0x18CC + static_cast<std::size_t>(player) * 4);
        if (at<std::uint8_t>(slot, 9) == 0) {                                   // 0077FFBF
            host.entity_claim_player_144(entity, player);                       // 0077FFD1
            host.warning_manager_report_new_unit_00977cd0(
                host.global_ptr(kWarningManager), at<std::uint32_t>(msg, 0x20));  // 0077FFDD
            void* const game = host.global_ptr(kGame);
            if (game != nullptr && at<std::int32_t>(game, 0x1FE4) == kSessionModeServer) {
                host.session_owner_broadcast_0077c7b0(entity, msg, 0);          // 0077FFF9
                host.unit_group_release_member_0077c980(entity, nullptr);      // 00780002
            }
        }
        host.unit_apply_role_owner_slots_0077a5d0(entity);                     // 00780009
        return true;
    }
    case 0x54:  // MT_SETGUINAME, the receiving side of 0077D3F0                // 00780015
        host.entity_set_gui_name_0c(entity, add(msg, 0x20), at<std::uint8_t>(msg, 0x28));
        return true;
    case 0x76: {  // MT_FORMATION_JOIN                                          // 0077FEC8
        void* const target = host.entity_handle_resolve_00521e30(at<std::uint16_t>(msg, 0x20));
        if (target != nullptr) host.unit_group_join_or_merge_0077f940(entity, target);
        return true;
    }
    case 0x77: {  // MT_FORMATION_LEAVE                                         // 0077FEE4
        const std::uint16_t id = at<std::uint16_t>(msg, 0x20);
        void* const target = id != 0 ? host.entity_handle_resolve_00521e30(id) : nullptr;
        host.unit_group_hand_over_leadership_0077bd70(entity, target);         // 0077FEF7
        return true;
    }
    case 0x78: {  // MT_FORMATION_SET                                           // 0077FF03
        if (ptr(entity, 0x284) != nullptr) {
            host.unit_group_apply_formation_shape_0070efd0(ptr(entity, 0x284),
                                                           at<std::uint32_t>(msg, 0x20));
            const std::int32_t n = at<std::int32_t>(ptr(entity, 0x284), 0x4F8);   // 0077FF21
            for (std::int32_t i = 0; i < n; ++i) {                              // 0077FF2F / 0077FF98 JL
                void* const record = host.group_member_record_at_0070d070(
                    ptr(entity, 0x284), static_cast<std::uint32_t>(i));         // 0077FF3B
                const float* const a = static_cast<const float*>(ptr(msg, 0x28)) + i * 4;
                const float* const b = static_cast<const float*>(ptr(msg, 0x2C)) + i * 4;
                for (int k = 0; k < 4; ++k) {                                   // 0077FF43..0077FF95
                    at<float>(record, 0x10 + 4 * k) = a[k];
                    at<float>(record, 0x20 + 4 * k) = b[k];
                }
            }
        }
        host.session_owner_broadcast_0077c7b0(entity, msg,
                                              at<std::int32_t>(msg, 0x14));     // 0077FFA3
        return true;
    }
    default:                                                                    // 0078002F
        return host.entity_generic_message_164(entity, msg);
    }
}

// 0077F7B0 __thiscall(unit, msg) RET 4, entity message 51h (00780522). Schedules the
// unit's expiry at clock + mode delay + message age, transfers every role, retires the
// controlled unit's HUD, and outside client mode clears and stops its orders.
void unit_apply_expiry_message_0077f7b0(void* unit, const void* msg, Small2Host& host) {
    const std::int32_t age = at<std::int32_t>(const_cast<void*>(msg), 0xC) -
                             global_int(host, kTick);                        // 0077F7BB / 0077F7BE
    // 0077F7C8 FILD, 0077F7D2 FMUL float 0.05, 0077F7D8 FSTP float. FILD is exact; the
    // product rounds to float (24-bit precision) like the C++ below for |age| < 2^24. On a
    // 53-bit fixed-step thread the sums below would round only at the FSTP (uncertain).
    const float age_seconds = static_cast<float>(age) * kTickSeconds;
    const std::int32_t mode = host.game_effective_mode_004bca50(host.global_ptr(kGame));
    const float clock = global_float(host, kClock);                          // 0077F7E4
    const float delay = mode == kGameModeDelayAlt
                            ? at<float>(host.global_config_00432650(), 0x104)   // 0077F7F5
                            : at<float>(host.global_config_00432650(), 0x100);  // 0077F802
    at<float>(unit, 0x308) = (delay + clock) + age_seconds;                  // 0077F808..0077F81A
    host.unit_send_role_transfer_0077c470(unit, kRoleMaskAll, 0);            // 0077F820
    host.entity_clear_roles_148(unit, kRoleMaskAll, 8);                      // 0077F836
    if (host.global_ptr(kControlledUnit) == unit) {                          // 0077F838
        host.limbo_screen_set_state_00565fb0(ptr(host.global_ptr(kFrontEnd), 0xE8),
                                             kLimboStateExpired);            // 0077F84E
        void* const fe = host.global_ptr(kFrontEnd);                         // 0077F853
        if (at<std::uint32_t>(fe, 0x4) == at<std::uint32_t>(fe, 0x20) &&
            at<std::uint32_t>(fe, 0x1C) == at<std::uint32_t>(fe, 0x38))      // 0077F85C / 0077F864
            host.front_end_push_interface_004cc460(fe, kInterfaceAfterExpiry, 0);  // 0077F86C
    }
    if (session_mode(host) == kSessionModeClient) return;                    // 0077F877
    for (const std::uint32_t order : {kOrderClearOrders, kOrderStop}) {      // 0077F8D7 / 0077F92F
        const auto* const def = static_cast<const float*>(host.global_address(kDefaultCommandPosition));
        TargetDescriptor target{};
        target.kind = 0;
        target.has_entity = 0;
        target.pad2 = 0;
        target.entity = 0;
        target.point[0] = def[0];
        target.point[1] = def[1];
        target.point[2] = def[2];
        target.extra = 0.0f;
        host.entity_issue_command_0077d600(unit, order, &target, 1);
    }
}

// 00780DB0 __thiscall(object, id) RET 4, EAX = object: the MultiScore net entity. Base
// 0077EED0(id), five vtable words, +C4h = 60h, +54h = 2, +58h = 0, single device role 9
// (0077F2D0), byte +310h = 0. Then, on a borrowed Lua state owner over
// [[[game+1A08h]+4]+4], for the globals "MultiScore" and "MultiScore_save" in turn: create the
// table (00B67580), look it up again by name (00B67800: 00D04080 at 00780EFC, 00D04070 at
// 00780FF6) and give it eight empty subtables at 0..7 (00B66670). The owner is closed last.
void* multi_score_construct_00780db0(void* object, std::uint32_t id, Small2Host& host) {
    host.unit_owner_entity_construct_0077eed0(object, id);                    // 00780DDC
    at<std::uint32_t>(object, 0x000) = 0x00D040B8;                            // 00780DEE
    at<std::uint32_t>(object, 0x010) = 0x00D040A0;
    at<std::uint32_t>(object, 0x024) = 0x00D04098;
    at<std::uint32_t>(object, 0x170) = 0x00D04094;
    at<std::uint32_t>(object, 0x1E4) = 0x00D0408C;                            // 00780E0C
    at<std::uint32_t>(object, 0x0C4) = 0x60;                                  // 00780E16
    at<std::uint32_t>(object, 0x054) = 2;                                     // 00780E20
    at<std::uint32_t>(object, 0x058) = 0;                                     // 00780E27
    host.unit_assign_single_device_role_0077f2d0(object, 9);                  // 00780E2A
    at<std::uint8_t>(object, 0x310) = 0;                                      // 00780E2F
    alignas(4) std::uint8_t owner[0x54];
    void* const game = host.global_ptr(kGame);
    host.lua_state_owner_construct_borrowed_00b66c00(
        owner, ptr(ptr(ptr(game, 0x1A08), 4), 4));                            // 00780E4C
    for (const std::uint32_t name_address : {kMultiScoreName, kMultiScoreSaveName}) {
        const char* const text = static_cast<const char*>(host.global_address(name_address));
        NativeString name{0, nullptr};
        host.native_string_resize_0041dd40(&name, static_cast<std::uint32_t>(std::strlen(text)),
                                           1);                                // 00780E69 / 00780F63
        if (name.data != nullptr) std::memcpy(name.data, text, name.length + 1);
        LuaObject globals{};
        host.lua_set_new_table_00b67580(host.lua_globals_00b67980(owner, &globals), &name);
        host.lua_destroy_00b67700(&globals);                                  // 00780EC2 / 00780FBC
        if (name.data != nullptr) host.native_string_free_storage(&name);
        LuaObject table{};
        void* const g = host.lua_globals_00b67980(owner, &globals);           // 00780EF7 / 00780FF1
        host.lua_get_by_name_00b67800(g, &table, text);                       // 00780F10 / 0078100A
        host.lua_destroy_00b67700(&globals);                                  // 00780F21 / 0078101B
        for (std::int32_t i = 0; i < 8; ++i)
            host.lua_set_new_table_at_index_00b66670(&table, i);              // 00780F35 / 00781027
        host.lua_destroy_00b67700(&table);                                    // 00780F4E / 00781040
    }
    host.lua_state_owner_close_00b669a0(owner);                               // 00781050
    return object;
}

// ---------------------------------------------------------------------------
// Segment 26.
// ---------------------------------------------------------------------------

// 00641E30 __thiscall(markers, unit) RET 4. Skips a unit whose parent (00923810(1)) is a
// kind-6 entity with parts byte +14h == 1, and a unit that is not live (+5Ch set, +5Dh / +60h
// / +5Eh clear). Projects the unit's world position (y forced to 0 for a kind-8 unit under
// 00852860 while the [game+1ED4h] entity's y is > 0) through 006395E0 into a, b, c, d (a is
// written into the unit argument's own stack slot), then emits a box marker on the 006407A0(0)
// sink with the pkt642 contract of 0063C620: corners = (a, b), (a, d), (c, d), (c, b);
// group 3; show 0; text_mode 1; text = unit->vtable[10h](); unit null; distance "" by value;
// scale 0.0f; colour = {a, b, a, d} (the binary copies the projected values into the colour
// slot); flag 0. 14 stack dwords, RET 38h. Only four corner pairs are written here (frame
// +18h..+34h); the contract's fifth pair would be this frame's SEH record at +38h, which
// 0063C620 does not read here: unit null, so the special path is skipped (JZ 0063C815);
// corners+20h/+24h are read only at 0063C83D/0063C848.
void hud_markers_add_unit_part_markers_00641e30(void* markers, void* unit, Small2Host& host) {
    void* const parent = host.entity_ancestor_00923810(unit, 1);              // 00641E55
    if (parent != nullptr && host.entity_is_kind_5c(parent, kUnitKind06) &&
        at<std::uint8_t>(host.unit_parts_object_0080e490(parent), 0x14) == 1)
        return;                                                                // 00641E7A
    if (unit == nullptr || at<std::uint8_t>(unit, 0x5C) == 0 || at<std::uint8_t>(unit, 0x5D) != 0 ||
        at<std::uint8_t>(unit, 0x60) != 0 || at<std::uint8_t>(unit, 0x5E) != 0)
        return;                                                                // 00641E80..00641EAA
    void* const sink = host.hud_markers_sink_006407a0(markers, 0);            // 00641EB4
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    float point[3] = {at<float>(unit, 0xFC), at<float>(unit, 0x100), at<float>(unit, 0x104)};
    if (host.entity_is_kind_5c(unit, kUnitKind08) &&
        host.entity_is_below_altitude_third_00852860(unit)) {                  // 00641EFE / 00641F06
        const float* const ref = host.entity_world_position_refreshed_00427eb0(
            ptr(host.global_ptr(kGame), 0x1ED4));                              // 00641F1A
        if (ref[1] > 0.0f) point[1] = 0.0f;                                    // 00641F27 COMISS / JA
    }
    float a = 0.0f, b = 0.0f, c = 0.0f, d = 0.0f;
    host.hud_markers_project_part_bounds_006395e0(markers, point, unit, &a, &b, &c, &d);  // 00641F6C
    const float corners[8] = {a, b, a, d, c, d, c, b};                        // 00641F8C..00641FE9
    const float colour[4] = {a, b, a, d};                                      // 00641F96..00641FC5
    NativeString empty{0, nullptr};
    host.native_string_assign_cstr_0041e870(
        &empty, static_cast<const char*>(host.global_address(kEmptyCString)));  // 00641FEF
    const char* const name = host.entity_name_10(unit);                       // 00642005
    // The by-value string is owned by 0063C620 from here (EH state -1 at 00642015).
    host.hud_marker_emit_box_0063c620(sink, corners, 3, 0, 1, name, nullptr, empty, 0.0f, colour,
                                      0);
}

// 00645BB0 __thiscall(listener, unit, event, candidate) RET 0Ch, slot +0Ch of the HUD-root
// secondary vtable 00CF5A84 (the listener sits at root+8). Event 1 for the controlled unit:
// adopt `candidate` when 00645060 accepts it, else reset the cursor to (0, FFFFh).
// 00645060 is __fastcall(ECX = unit, EDX = team [game+18ECh], allow = 1).
void hud_root_listener_current_0c_00645bb0(void* listener, void* unit, std::int32_t event,
                                           void* candidate, Small2Host& host) {
    if (event != 1) return;                                                  // 00645BB4
    if (unit != host.global_ptr(kControlledUnit)) return;                    // 00645BC0
    const std::int32_t team = at<std::int32_t>(host.global_ptr(kGame), 0x18EC);  // 00645BCE
    if (host.unit_is_selectable_00645060(candidate, team, true)) {           // 00645BDD
        host.hud_root_set_controlled_unit_00645600(static_cast<std::uint8_t*>(listener) - 8,
                                                   candidate);                // 00645BEA
        return;
    }
    at<std::uint8_t>(listener, 0xBA) = 0;                                    // 00645BF4
    at<std::uint16_t>(listener, 0xBC) = 0xFFFF;                              // 00645BFB
}

// 00648930 __thiscall(hudRoot) RET. When the current player slot (game+18CCh[game+18ECh])
// exists and its +19h byte is clear, a unit is controlled, the session mode is not 0, and
// 00645060 rejects the controlled unit, force a new selection (tail JMP 006485A0).
void hud_root_ensure_selectable_controlled_unit_00648930(void* hud_root, Small2Host& host) {
    void* const game = host.global_ptr(kGame);                               // 00648930
    if (game == nullptr) return;
    const std::int32_t team = at<std::int32_t>(game, 0x18EC);                // 0064893C
    void* const slot = at<void*>(game, 0x18CC + static_cast<std::size_t>(team) * 4);
    if (slot == nullptr) return;                                             // 00648942
    if (at<std::uint8_t>(slot, 0x19) != 0) return;                           // 00648953
    void* const unit = host.global_ptr(kControlledUnit);                     // 00648959
    if (unit == nullptr) return;
    if (at<std::int32_t>(game, 0x1FE4) == 0) return;                         // 00648963
    if (host.unit_is_selectable_00645060(unit, team, true)) return;          // 0064896E
    host.hud_root_force_select_unit_006485a0(hud_root);                      // 0064897A
}

// 0064A270 __thiscall(task, priority) RET 4: task+24h = priority, then the warning manager's
// "repair" channel with (entity = task[0], priority).
void repair_task_set_repair_priority_0064a270(void* task, std::int32_t priority,
                                              Small2Host& host) {
    at<std::int32_t>(task, 0x24) = priority;                                 // 0064A274
    host.warning_manager_repair_channel_00983c10(host.global_ptr(kWarningManager),
                                                 at<std::uint32_t>(task, 0), priority);
}

// ---------------------------------------------------------------------------
// Segment 9.
// ---------------------------------------------------------------------------

// 004C3A80 __thiscall(game) RET, from BSP_Game_UpdateMultiplayerInterface (004D80D0). In
// game mode 0Dh with the local slot index game+18ECh in 0..7: refresh the HUD root's
// selection (00648930), set the local slot's +19h "no unit" byte from the first kind-5 or
// kind-18h net entity 00645160 accepts (spectating it when the byte was already set),
// report it to the [FE+B8h]+10h widget when game+2194h is set, then do the same test for
// the rotating slot [00E188C4] through 00645060(allow 0) and advance that index mod 8.
// The net-list iterator reports (004C3B08 / 004C3B15 / 004C3B1F / 004C3B57 and the second
// loop's) sit behind tests already passed and are left out.
void game_update_player_slot_unit_flags_004c3a80(void* game, Small2Host& host) {
    if (at<std::int32_t>(game, 0x5D4) != kGameModeSlotScan) return;          // 004C3A86
    const std::int32_t local = at<std::int32_t>(game, 0x18EC);               // 004C3A97
    if (local < 0 || local > 7) return;                                      // 004C3A9D / 004C3AA5
    void* fe = host.global_ptr(kFrontEnd);                                   // 004C3AAE
    if (fe != nullptr && ptr(fe, 0x40) != nullptr)
        hud_root_ensure_selectable_controlled_unit_00648930(ptr(fe, 0x40), host);  // 004C3AC0
    const auto slot_of = [game](std::int32_t i) {
        return at<void*>(game, 0x18CC + static_cast<std::size_t>(i) * 4);
    };
    const bool was_none = at<std::uint8_t>(slot_of(at<std::int32_t>(game, 0x18EC)), 0x19) != 0;
    bool none = true;                                                        // 004C3ADC BL = 1
    void* found = nullptr;                                                   // 004C3AE9
    for (void* node = ptr(host.global_ptr(kNetEntityListHead), 0);
         node != host.global_ptr(kNetEntityListHead); node = ptr(node, 0)) { // 004C3AF8 / 004C3B5C
        void* const entity = ptr(node, 8);                                   // 004C3B24
        if (!host.entity_is_kind_5c(entity, kUnitKind05) &&
            !host.entity_is_kind_5c(entity, kUnitKind18))
            continue;                                                        // 004C3B30 / 004C3B3F
        if (host.unit_is_selectable_for_current_player_00645160(entity, false)) {  // 004C3B49
            none = false;                                                    // 004C3B60
            found = entity;                                                  // 004C3B66
            break;
        }
    }
    if (was_none && !none) {                                                 // 004C3B6A / 004C3B71
        host.hud_root_set_spectated_unit_00647300(ptr(host.global_ptr(kFrontEnd), 0x40), found);
        if (host.hud_get_controlled_unit_00644230(ptr(host.global_ptr(kFrontEnd), 0x40)) ==
            nullptr)
            none = true;                                                     // 004C3B99
    }
    at<std::uint8_t>(slot_of(at<std::int32_t>(game, 0x18EC)), 0x19) = none ? 1 : 0;  // 004C3BB0
    if (at<std::uint8_t>(game, 0x2194) != 0) {                               // 004C3BB3
        fe = host.global_ptr(kFrontEnd);
        if (fe != nullptr && ptr(fe, 0xB8) != nullptr && ptr(ptr(fe, 0xB8), 0x10) != nullptr)
            host.widget_set_flag_34(ptr(ptr(fe, 0xB8), 0x10), none);         // 004C3BE9
    }
    bool rotating_none = true;                                               // 004C3BF2
    for (void* node = ptr(host.global_ptr(kNetEntityListHead), 0);
         node != host.global_ptr(kNetEntityListHead); node = ptr(node, 0)) { // 004C3C00 / 004C3C4C
        if (host.unit_is_selectable_00645060(ptr(node, 8),
                                             global_int(host, kRotatingSlot), false)) {  // 004C3C39
            rotating_none = false;                                           // 004C3C50
            break;
        }
    }
    at<std::uint8_t>(slot_of(global_int(host, kRotatingSlot)), 0x19) = rotating_none ? 1 : 0;
    // 004C3C66..004C3C7E: (index + 1) % 8 with C remainder semantics.
    at<std::int32_t>(host.global_address(kRotatingSlot), 0) =
        (global_int(host, kRotatingSlot) + 1) % 8;
}

// ---------------------------------------------------------------------------
// Segment 23.
// ---------------------------------------------------------------------------

// 005F98F0 __thiscall(unit) RET, bool in AL: a kind-6 unit that is not kind 9 is inside the
// capture radius (int +7A0h) of some same-team (+54h) entity on the list at
// [[game+19CCh]+16Ch] (singly linked: +4 next, +8 entity). Horizontal distance only.
// x87 from the listing: s = dx*dx + dz*dz stored to float, r = (float)int, r*r stored to
// float, then FCOMIP ST0(s), ST1(r*r) / JBE: s <= r*r, or unordered, is inside.
// Modelled at 24-bit precision; at 53 bits the two squares would be added before rounding.
bool unit_within_friendly_capture_radius_005f98f0(void* unit, Small2Host& host) {
    if (!host.entity_is_kind_5c(unit, kUnitKind06)) return false;            // 005F98FD
    if (host.entity_is_kind_5c(unit, kUnitKind09)) return false;             // 005F9910
    const std::uint32_t team = at<std::uint32_t>(unit, 0x54);                // 005F9926
    for (void* node = ptr(ptr(host.global_ptr(kGame), 0x19CC), 0x16C);       // 005F991A..005F992B
         node != nullptr; node = ptr(node, 4)) {                             // 005F99CE
        void* const entity = ptr(node, 8);                                   // 005F9940
        if (at<std::uint32_t>(entity, 0x54) != team) continue;               // 005F9943
        if (at<std::uint8_t>(entity, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
        const float dx = at<float>(unit, 0xFC) - at<float>(entity, 0xFC);    // 005F9972
        const float dz = at<float>(unit, 0x104) - at<float>(entity, 0x104);  // 005F9982
        const float r = static_cast<float>(at<std::int32_t>(entity, 0x7A0)); // 005F998C FILD
        const float s = dx * dx + dz * dz;                                   // 005F99A2..005F99B0
        const float r2 = r * r;                                              // 005F99B8..005F99BE
        if (!(s > r2)) return true;                                          // 005F99C8 / JBE
    }
    return false;
}

}  // namespace bsp::cc13_small2
