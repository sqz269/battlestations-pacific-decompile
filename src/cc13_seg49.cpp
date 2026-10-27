// Segment 49, reconstruction pass (packet cc13-seg49). See include/bsp/cc13_seg49.hpp and
// docs/CC13_SEG49.md. Every routine names its address, its original ABI and what was
// simplified.

#include "bsp/cc13_seg49.hpp"
#include "bsp/cc13_x87_model.hpp"

namespace bsp::cc13_seg49 {
std::uint32_t entity_path_type_code_0080d780() noexcept { return 0x12u; }
}

#include "bsp/band_0080_cc13.hpp"  // turn_path_wrap_angle_0080b190

#include <cmath>
#include <cstring>
#include <utility>

namespace bsp::cc13_seg49 {

// 00801200, ABI: ECX = value (no xref found; a vtable slot or dead), RET.
// Builds a 14h-byte record on the stack: +0h = ECX, +10h = 1, +4h..+0Ch left uninitialised
// (zeroed here), and passes its address to [00F8D394] vtable +40h.
void renderer_send_flagged_0x40_00801200(Seg49Host& host, std::uint32_t value)
{
    std::uint8_t bytes[0x14] = {};
    Raw message{bytes};
    message.at<std::uint32_t>(0x00) = value;
    message.at<std::uint32_t>(0x10) = 1;
    host.renderer_virtual_40(host.renderer_f8d394(), message);
}

// 00803330, ABI: __fastcall(ECX = gun) RET. MMultipleBombPlatform vtable 00CFE308 slot +A0h.
// gun+74h is the gun's local matrix (rows of four floats: +94h row 2, +A4h translation).
void multiple_bomb_platform_setup_00803330(Seg49Host& host, Raw gun)
{
    host.gun_setup_from_descriptor_0072e6d0(gun.p);
    Raw cls{static_cast<std::uint8_t*>(gun.ptr(0x3F0))};
    Raw platforms{static_cast<std::uint8_t*>(cls.ptr(0x538)) + 0x94};  // {data, count}
    const int slot = gun.at<int>(0x38C);
    if (slot >= platforms.at<int>(0x4)) {  // CMP / JL at 00803356
        host.ref_counted_pointer_array_resize_005471b0(platforms.p, slot + 1);
    }
    Raw platform{static_cast<std::uint8_t**>(platforms.ptr(0x0))[slot]};
    float* local = &gun.at<float>(0x74);
    host.matrix_copy_4x4_004134f0(local, &platform.at<float>(0x4C));
    const float tx = gun.at<float>(0xA4);
    const float ty = gun.at<float>(0xA8);
    const float tz = gun.at<float>(0xAC);
    const float forward_aim = platform.at<float>(0x48);
    // 008033A2..008033C2: (0 - x, 0 - y, ForwardAim - z), each stored as float.
    float aim[3] = {0.0f - tx, 0.0f - ty, forward_aim - tz};
    host.normalize_vector_with_floor_0042b260(aim);  // ECX = &aim, in place
    gun.at<float>(0x94) = aim[0];
    gun.at<float>(0x98) = aim[1];
    gun.at<float>(0x9C) = aim[2];
    host.orthonormalize_basis_rows_0085dc80(local);
    gun.at<float>(0xA4) = tx;
    gun.at<float>(0xA8) = ty;
    gun.at<float>(0xAC) = tz;
    gun.at<std::uint8_t>(0xC8) = 0;
    gun.at<std::uint8_t>(0x10C) = 0;
    for (void* child = gun.ptr(0x48); child != nullptr;
         child = Raw{static_cast<std::uint8_t*>(child)}.ptr(0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);
    }
    host.gun_attachment_virtual_38(gun.ptr(0x3CC), local);
}

// 00803480, ABI: __thiscall(ECX = gun, message) RET 4 -> AL. MRTGun vtable 00CFE548 slot +164h.
bool mrt_gun_handle_network_message_00803480(Seg49Host& host, Raw gun, Raw message)
{
    if (message.at<std::uint8_t>(0x10) != 0xAF) {
        return host.gun_handle_network_message_0072d830(gun.p, message);  // JMP, tail call
    }
    host.gun_set_trigger_held_1e8(gun.p, message.at<std::uint8_t>(0x20));
    return true;
}

// 008034C0, ABI: __fastcall(ECX = object) RET; no xref found. object+8h =
// [object+4h]+1E4h vtable[1]() (ECX = the embedded object).
void cache_sensor_class_index_008034c0(Seg49Host& host, Raw object)
{
    auto* unit = static_cast<std::uint8_t*>(object.ptr(0x4));
    object.at<int>(0x8) = host.sensor_class_index_1e4_4(unit + 0x1E4);
}

// 008034E0, ABI: void() RET. The scalar deleting destructor (flags 1) of each non-null pointer
// in 00F874BC..00F874C8 (three slots); the pointers are not cleared.
void recon_destroy_all_slots_008034e0(Seg49Host& host)
{
    for (std::size_t i = 0; i < (kReconSlotsEnd - kReconSlotsBegin) / 4; ++i) {
        if (void* slot = host.recon_slot_global(i)) {
            host.scalar_deleting_destructor_0(slot, 1);
        }
    }
}

// 00803860, ABI: __fastcall(ECX = scope) RET. Reinstalls vtable 00D08E5C and tail-jumps to
// 00A673E0 lua_settop with the register ABI ECX = [scope+4h]+4h (the lua_State), EDX = -2:
// pops the one table the scope pushed.
void recon_table_scope_destruct_00803860(Seg49Host& host, Raw scope)
{
    scope.at<std::uint32_t>(0x0) = kReconTableScopeVtable;
    Raw owner{static_cast<std::uint8_t*>(scope.ptr(0x4))};
    host.lua_settop_00a673e0(owner.ptr(0x4), -2);
}

// 008039B0, ABI: __fastcall(ECX = triple) RET. Element constructor handed as data to the
// array helpers of 00805125..008051E6 and 00805DB4: zeroes three dwords.
void zero_triple_008039b0(Raw triple) noexcept
{
    triple.at<std::uint32_t>(0x0) = 0;
    triple.at<std::uint32_t>(0x4) = 0;
    triple.at<std::uint32_t>(0x8) = 0;
}

// 008039C0, ABI: __fastcall(ECX = singleton) RET. Clears 00F874C8 and reinstalls the
// singleton-base vtable 00CE3818.
void recon_scan_class_ids_reset_singleton_base_008039c0(Seg49Host& host, Raw singleton)
{
    host.recon_slot_global(3) = nullptr;
    singleton.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// The read 00803B50, 00803D40, 00803DC0 and 00803E40 share: +8h when the force byte +10h is
// set, else +4h.
int detection_effective_level(Raw record) noexcept
{
    return record.at<std::uint8_t>(kDetectionForced) != 0 ? record.at<int>(kDetectionForcedLevel)
                                                         : record.at<int>(kDetectionLevel);
}

// 00803B50, ABI: __fastcall(ECX = detection record) RET. Sole caller 008AAB90.
void recon_detection_clear_forced_level_00803b50(Seg49Host& host, Raw record)
{
    const int old_effective = detection_effective_level(record);
    const int level = record.at<int>(kDetectionLevel);
    record.at<std::uint8_t>(kDetectionForced) = 0;
    if (level != old_effective) {
        host.detection_delegate_0(record.ptr(kDetectionDelegate),
                                  record.at<std::uint32_t>(kDetectionDelegateArg), old_effective, level);
    }
}

// 00803BE0, ABI: __fastcall(ECX = party) RET -> AL. The local player is
// [game + 18CCh + [game+18ECh] * 4]; its party is +28h.
std::uint8_t party_relative_to_local_player_00803be0(Seg49Host& host, int party)
{
    Raw game = host.game_e188a8();
    const int local_index = game.at<int>(0x18EC);
    Raw player{static_cast<std::uint8_t*>(game.ptr(0x18CC + local_index * 4))};
    const int reference = player.at<int>(0x28);
    if (reference == 2) {
        return party != 2 ? 2 : 0;  // NEG/SBB/AND 2
    }
    if (reference == party) {
        return 0;
    }
    return static_cast<std::uint8_t>((party == 2 ? 1 : 0) + 1);
}

// 00803D40, ABI: __fastcall(ECX = party, EDX = unit) RET -> EAX. Callers 00526180, 0071EDA0,
// 00835840. Only units for which vtable +5Ch(2) holds are filtered.
void* recon_filter_undetected_unit_00803d40(Seg49Host& host, int party, void* unit)
{
    if (unit == nullptr) {
        return nullptr;
    }
    if (host.is_kind_of_5c(unit, 2)) {
        Raw record{static_cast<std::uint8_t*>(unit) + kUnitDetectionBase + party * kDetectionStride};
        if (detection_effective_level(record) < 1) {
            return nullptr;
        }
    }
    return unit;
}

// 00803DC0, ABI: __fastcall(ECX = party, EDX = unit) RET -> EAX. No null check on the unit, as
// the image. Level >= 2 tail-jumps to unit vtable +14h.
const char* recon_display_name_for_party_00803dc0(Seg49Host& host, int party, void* unit)
{
    Raw record{static_cast<std::uint8_t*>(unit) + kUnitDetectionBase + party * kDetectionStride};
    if (detection_effective_level(record) < 2) {
        return kGlobalsUnknown;
    }
    return host.unit_name_14(unit);
}

// 00803E40, ABI: __thiscall(ECX = slot, peer index) RET 4. No caller found. In a session of
// kind 1 (game+1FE4h) each node of the list slot+E0Ch ({+4h next, +8h payload}, payload+4h =
// unit) sends a SessionMessage 61h (28h bytes, vtable 00D02D1C) to [game+18CCh + peer*4]. The
// message is built on the stack once per node and never destroyed on the normal path.
void recon_slot_send_detection_levels_to_peer_00803e40(Seg49Host& host, Raw slot, int peer_index)
{
    if (host.game_e188a8().at<int>(0x1FE4) != 1) {
        return;
    }
    for (void* node = slot.ptr(0xE0C); node != nullptr;
         node = Raw{static_cast<std::uint8_t*>(node)}.ptr(0x4)) {
        const int party = slot.at<int>(0x28);
        Raw payload{static_cast<std::uint8_t*>(Raw{static_cast<std::uint8_t*>(node)}.ptr(0x8))};
        auto* unit = static_cast<std::uint8_t*>(payload.ptr(0x4));
        std::uint8_t bytes[0x28];
        std::memset(bytes, 0, sizeof bytes);
        Raw message{bytes};
        host.session_message_construct_base_0075b430(message, 0x61);
        Raw record{unit + kUnitDetectionBase + party * kDetectionStride};
        message.at<std::uint16_t>(0x18) = 0;
        message.at<std::uint8_t>(0x1A) = 0;
        message.at<std::uint32_t>(0x04) = 1;
        message.at<std::uint8_t>(0x1C) = 0;
        message.at<std::uint32_t>(0x00) = kSessionMessage97Vtable;
        message.at<int>(0x20) = detection_effective_level(record);
        message.at<int>(0x24) = party;
        Raw game = host.game_e188a8();
        void* peer = game.ptr(0x18CC + peer_index * 4);
        // 00803EEE..00803EF6: ECX re-read as [[node+8h]+4h], the unit.
        host.entity_send_message_to_peer_00779fc0(unit, peer, message);
    }
}

// 00803F30, ABI: __fastcall(ECX = unit, EDX = target) RET -> ST0. No caller found. Table
// [unit+538h]+B4h; row (targetIndex + unitIndex * 8) at +8h + row * 0Ch is {entries, count} of
// 1Ch-byte entries; the answer is the largest first float, starting from 0.0 (FCOMIP / JBE:
// an unordered compare keeps the running value). Target's index is read first.
float unit_max_sensor_range_against_00803f30(Seg49Host& host, Raw unit, Raw target)
{
    Raw cls{static_cast<std::uint8_t*>(unit.ptr(0x538))};
    auto* table = static_cast<std::uint8_t*>(cls.ptr(0xB4));
    float best = 0.0f;
    if (table == nullptr) {  // NEG/SBB/TEST 00E1952Ch: zero only for a null table
        return 0.0f;
    }
    const int target_index = host.sensor_class_index_1e4_4(target.p + 0x1E4);
    const int unit_index = host.sensor_class_index_1e4_4(unit.p + 0x1E4);
    Raw row{table + 8 + (target_index + unit_index * 8) * 0xC};
    auto* entry = static_cast<std::uint8_t*>(row.ptr(0x0));
    auto* end = entry + row.at<int>(0x4) * 0x1C;
    for (; entry != end; entry += 0x1C) {
        const float value = *reinterpret_cast<float*>(entry);
        if (value > best) {
            best = value;
        }
    }
    return best;
}

// 008040E0, ABI: __thiscall(ECX = destination list, source list, node) RET 8. Unlinks node from
// source and appends it to this; no allocation.
void recon_list_splice_node_008040e0(ReconList& self, ReconList& source, ReconNode* node) noexcept
{
    if (node->prev == nullptr) {
        source.head = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node->next == nullptr) {
        source.tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    --source.count;
    node->prev = self.tail;
    if (self.count != 0) {
        self.tail->next = node;
    } else {
        self.head = node;
    }
    self.tail = node;
    node->next = nullptr;
    ++self.count;
}

// 00804150, ABI: __thiscall(ECX = destination list, source list) RET 4. Moves every node of
// source, head first, onto the tail of this.
void recon_list_splice_all_00804150(ReconList& self, ReconList& source) noexcept
{
    while (source.count != 0) {
        ReconNode* node = source.head;
        if (node->prev == nullptr) {
            source.head = node->next;
        } else {
            node->prev->next = node->next;
        }
        if (node->next == nullptr) {
            source.tail = node->prev;
        } else {
            node->next->prev = node->prev;
        }
        --source.count;
        node->prev = self.tail;
        if (self.count == 0) {
            self.head = node;
        } else {
            self.tail->next = node;
        }
        self.tail = node;
        node->next = nullptr;
        ++self.count;
    }
}

// 008042B0, ABI: __fastcall(ECX = list) RET. Pops the head and frees the node (00BF65AC)
// until the count is zero; payloads are not touched. The back edge is at 008042EC..008042F3.
void recon_list_clear_008042b0(Seg49Host& host, ReconList& self)
{
    while (self.count != 0) {
        ReconNode* node = self.head;
        if (node->prev == nullptr) {
            self.head = node->next;
        } else {
            node->prev->next = node->next;
        }
        if (node->next == nullptr) {
            self.tail = node->prev;
        } else {
            node->next->prev = node->prev;
        }
        --self.count;
        host.free_00bf65ac(node);
    }
}

// 00804300, ABI: __fastcall(ECX = record) RET via JMP. The detection record's element
// destructor (run by the eh vector destructor iterator at 0077E46C over unit+1E8h): vtable
// 00D08E74, then 004B7EF0 on record+14h.
void recon_detection_record_destruct_00804300(Seg49Host& host, Raw record)
{
    record.at<std::uint32_t>(0x0) = kReconDetectionRecordVtable;
    host.observer_link_destroy_base_004b7ef0(record.p + 0x14);
}

namespace {

// The node allocation 00804C30 and 00804E10 inline: operator new(0Ch), zeroed when non-null.
// A null result is dereferenced afterwards, as in the image.
ReconNode* new_recon_node(Seg49Host& host)
{
    auto* node = static_cast<ReconNode*>(host.operator_new_00bf681b(sizeof(ReconNode)));
    if (node != nullptr) {
        node->prev = nullptr;
        node->next = nullptr;
        node->payload = nullptr;
    }
    return node;
}

void append_node(ReconList& self, ReconNode* node) noexcept
{
    node->prev = self.tail;
    if (self.count != 0) {
        self.tail->next = node;
    } else {
        self.head = node;
    }
    self.tail = node;
    node->next = nullptr;
    ++self.count;
}

}  // namespace

// 00804C30, ABI: __thiscall(ECX = list, payload) RET 4.
void recon_list_push_back_00804c30(Seg49Host& host, ReconList& self, void* payload)
{
    ReconNode* node = new_recon_node(host);
    node->payload = payload;
    append_node(self, node);
}

// 00804D20, ABI: __fastcall(ECX = list) RET. Pops every node; a non-null payload gets its
// scalar deleting destructor (vtable[0](1)) and the node's payload is cleared before the
// unlink and free. The back edge is at 00804D7A..00804D82.
void recon_list_destroy_payloads_00804d20(Seg49Host& host, ReconList& self)
{
    while (self.count != 0) {
        ReconNode* node = self.head;
        if (node->payload != nullptr) {
            host.scalar_deleting_destructor_0(node->payload, 1);
            node->payload = nullptr;
        }
        if (node->prev == nullptr) {
            self.head = node->next;
        } else {
            node->prev->next = node->next;
        }
        if (node->next == nullptr) {
            self.tail = node->prev;
        } else {
            node->next->prev = node->prev;
        }
        --self.count;
        host.free_00bf65ac(node);
    }
}

// 00804E10, ABI: __thiscall(ECX = list, source list) RET 4. A fresh node per source node,
// same payload; the source is untouched.
void recon_list_copy_append_00804e10(Seg49Host& host, ReconList& self, const ReconList& source)
{
    for (ReconNode* it = source.head; it != nullptr; it = it->next) {
        void* payload = it->payload;
        ReconNode* node = new_recon_node(host);
        node->payload = payload;
        append_node(self, node);
    }
}

// 00804E70, ABI: __thiscall(ECX = entry, unit, level) RET 8 -> EAX = entry. The 1Ch recon
// entry: vtable 00D08E78, an empty {count, head, tail} member list at +10h, level +0Ch, unit
// +4h, and +8h = unit+1E4h vtable[1]() (the sensor class index).
void* recon_entry_construct_00804e70(Seg49Host& host, Raw entry, void* unit, int level)
{
    entry.at<std::uint32_t>(0x00) = kReconEntryVtable;
    entry.at<std::uint32_t>(0x10) = 0;
    entry.at<std::uint32_t>(0x14) = 0;
    entry.at<std::uint32_t>(0x18) = 0;
    entry.at<int>(0x0C) = level;
    entry.ptr(0x04) = unit;
    entry.at<int>(0x08) = host.sensor_class_index_1e4_4(static_cast<std::uint8_t*>(unit) + 0x1E4);
    return entry.p;
}

namespace {

ArchiveArg arg_int(std::int32_t value) noexcept { return {1, static_cast<std::uintptr_t>(static_cast<std::uint32_t>(value))}; }
ArchiveArg arg_key(const char* key) noexcept { return {0, reinterpret_cast<std::uintptr_t>(key)}; }

}  // namespace

// 00804F10, ABI: __thiscall(ECX = set, archive) RET 4. set+4h holds the three party records
// (34h apart; called with the unit's +1E4h sub-object). For child i present (+14h({1, i})):
// enter, read "value" into rec+0Ch ({2, float*}), recompute the level rec+4h from it (COMISS:
// 0 below 0.25, 1 below 0.5, else 2; NaN gives 2), and when "forcedLevel" is present set the
// force byte rec+10h and read it into rec+8h ({1, int*} into a stack slot); leave.
void recon_detection_set_read_archive_00804f10(Seg49Host& host, Raw set, void* archive)
{
    for (int i = 0; i < 3; ++i) {
        Raw record{set.p + 4 + i * kDetectionStride};
        if (!host.archive_has_14(archive, arg_int(i))) {
            continue;
        }
        host.archive_enter_04(archive, arg_int(i));
        host.archive_read_10(archive, arg_key(kArchiveValueKey),
                             {2, reinterpret_cast<std::uintptr_t>(&record.at<float>(0x0C))});
        const float value = record.at<float>(0x0C);
        int level = 2;
        if (kDetectionLevel1Threshold > value) {
            level = 0;
        } else if (kDetectionLevel2Threshold > value) {
            level = 1;
        }
        record.at<int>(kDetectionLevel) = level;
        if (host.archive_has_14(archive, arg_key(kArchiveForcedLevelKey))) {
            record.at<std::uint8_t>(kDetectionForced) = 1;
            // 00804FDA: the out slot is the routine's own archive argument ([ESP+1Ch]); ESI keeps
            // the pointer, so the slot starts as the archive pointer.
            auto forced = static_cast<std::int32_t>(reinterpret_cast<std::uintptr_t>(archive));
            host.archive_read_10(archive, arg_key(kArchiveForcedLevelKey),
                                 {1, reinterpret_cast<std::uintptr_t>(&forced)});
            record.at<int>(kDetectionForcedLevel) = forced;
        }
        host.archive_leave_08(archive);
    }
}

// 00805020, ABI: __thiscall(ECX = set, archive) RET 4. Writes child i when the record's value
// is not 0.0 (UCOMISS/LAHF/JP: NaN writes) or it is forced: enter {1, i}, +0Ch("value", {2,
// float}), +0Ch("forcedLevel", {1, rec+8h}) when forced, leave.
void recon_detection_set_write_archive_00805020(Seg49Host& host, Raw set, void* archive)
{
    for (int i = 0; i < 3; ++i) {
        Raw record{set.p + 4 + i * kDetectionStride};
        const bool forced = record.at<std::uint8_t>(kDetectionForced) != 0;
        if (!(record.at<float>(0x0C) != kDetectionDefaultValue) && !forced) {
            continue;
        }
        host.archive_enter_04(archive, arg_int(i));
        std::uint32_t bits = 0;
        std::memcpy(&bits, &record.at<float>(0x0C), sizeof bits);
        host.archive_write_0c(archive, arg_key(kArchiveValueKey), {2, bits});
        if (record.at<std::uint8_t>(kDetectionForced) != 0) {
            host.archive_write_0c(archive, arg_key(kArchiveForcedLevelKey),
                                  arg_int(record.at<int>(kDetectionForcedLevel)));
        }
        host.archive_leave_08(archive);
    }
}

// 00805430, ABI: __thiscall(ECX = slot, class id) RET 4. The carry-over list of the class is
// slot + E14h + id * 0Ch. For each node, unit = [payload+4h]; 00694AF0(ECX = unit, EDX = slot)
// gates 006952A0(unit, slot). Then 00804D20 destroys the list and its entries.
void recon_retire_carry_over_class_00805430(Seg49Host& host, Raw slot, int class_id)
{
    auto& list = slot.at<ReconList>(0xE14 + class_id * 0xC);
    for (ReconNode* node = list.head; node != nullptr; node = node->next) {
        void* unit = Raw{static_cast<std::uint8_t*>(node->payload)}.ptr(0x4);
        if (host.observer_is_pair_registered_00694af0(unit, slot.p)) {
            host.observer_unregister_pair_006952a0(unit, slot.p);
        }
    }
    recon_list_destroy_payloads_00804d20(host, list);
}

// 00805A70, ABI: __fastcall(ECX = record) RET -> EAX = record. The 34h detection record
// (vector-constructor path): vtable 00D08E74; the observer endpoint at +14h (vtable 00CE75CC,
// +18h/+1Ch/+20h/+28h zero, byte +24h = 1); level, forced level, value (0.0f) and force byte
// cleared. The endpoint's pair (+28h) is then unregistered when set, which it never is here
// (006952A0 with ECX = [+28h], EDX = record+14h).
void* recon_detection_construct_00805a70(Seg49Host& host, Raw record)
{
    Raw endpoint{record.p + 0x14};
    record.at<std::uint32_t>(0x00) = kReconDetectionRecordVtable;
    endpoint.at<std::uint32_t>(0x04) = 0;
    endpoint.at<std::uint32_t>(0x08) = 0;
    endpoint.at<std::uint32_t>(0x0C) = 0;
    endpoint.at<std::uint32_t>(0x14) = 0;
    endpoint.at<std::uint8_t>(0x10) = 1;
    endpoint.at<std::uint32_t>(0x00) = kObserverEndpointVtable;
    record.at<int>(kDetectionLevel) = 0;
    record.at<int>(kDetectionForcedLevel) = 0;
    record.at<float>(0x0C) = 0.0f;
    record.at<std::uint8_t>(kDetectionForced) = 0;
    if (void* partner = endpoint.ptr(0x14)) {
        host.observer_unregister_pair_006952a0(partner, endpoint.p);
        endpoint.ptr(0x14) = nullptr;
    }
    return record.p;
}

// 00805C60, ABI: __thiscall(ECX = record, level, detector) RET 8. Caller
// BSP_Session_DispatchQueuedMessage (message 61h). Stores the level; when the detector (+28h)
// changes, unregisters the old pair (006952A0, ECX = old detector, EDX = record+14h) and
// registers the new one (00694A60) when non-null; then the change delegate fires
// (old effective, new effective) when the effective level moved.
void recon_detection_set_level_and_detector_00805c60(Seg49Host& host, Raw record, int level, void* detector)
{
    const int old_effective = detection_effective_level(record);
    void* endpoint = record.p + 0x14;
    record.at<int>(kDetectionLevel) = level;
    if (record.ptr(0x28) != detector) {
        if (record.ptr(0x28) != nullptr) {
            host.observer_unregister_pair_006952a0(record.ptr(0x28), endpoint);
        }
        record.ptr(0x28) = detector;
        if (detector != nullptr) {
            host.observer_register_pair_00694a60(detector, endpoint);
        }
    }
    const int new_effective = detection_effective_level(record);
    if (new_effective != old_effective) {
        host.detection_delegate_0(record.ptr(kDetectionDelegate),
                                  record.at<std::uint32_t>(kDetectionDelegateArg), old_effective, new_effective);
    }
}

namespace {

// The inlined push_back of 00805F60: grow to max(capacity * 2, 1) through 00804030 when full,
// store unless the slot address is null, count the element either way.
void int_array_push(Seg49Host& host, Raw array, std::int32_t value)
{
    if (array.at<int>(0x4) == array.at<int>(0x8)) {
        int capacity = array.at<int>(0x8) * 2;
        if (capacity <= 1) {
            capacity = 1;
        }
        host.int_array_reserve_00804030(array.p, capacity);
    }
    auto* slot = static_cast<std::int32_t*>(array.ptr(0x0)) + array.at<int>(0x4);
    if (slot != nullptr) {
        *slot = value;
    }
    array.at<int>(0x4) += 1;
}

}  // namespace

// 00805F60, ABI: __fastcall(ECX = singleton) RET -> EAX. vtable 00D08EAC; both arrays
// zeroed; the 22 scan ids pushed into +4h; then 008058C0 assigns +4h into +10h (00806387,
// ECX = +10h) and 18h, 1Ah are pushed after them, so +10h holds 24 ids. The singleton pointer
// 00F874C8 is not written here.
void* recon_construct_scan_class_ids_00805f60(Seg49Host& host, Raw singleton)
{
    Raw scan{singleton.p + 0x4};
    Raw all{singleton.p + 0x10};
    singleton.at<std::uint32_t>(0x0) = kReconScanClassIdsVtable;
    scan.at<std::uint32_t>(0x0) = 0;
    scan.at<std::uint32_t>(0x4) = 0;
    scan.at<std::uint32_t>(0x8) = 0;
    all.at<std::uint32_t>(0x0) = 0;
    all.at<std::uint32_t>(0x4) = 0;
    all.at<std::uint32_t>(0x8) = 0;
    for (const std::int32_t id : kReconScanPushOrder) {
        int_array_push(host, scan, id);
    }
    host.int_array_assign_008058c0(all.p, scan.p);
    int_array_push(host, all, 0x18);
    int_array_push(host, all, 0x1A);
    return singleton.p;
}

// 00806400, ABI: __fastcall(ECX = singleton) RET. Called from 00806ED0. Each array is
// resized to 0 (00804090) and its data freed (00BF6989), +10h first; then 00F874C8 is
// cleared and the singleton-base vtable reinstalled.
void recon_scan_class_ids_destruct_00806400(Seg49Host& host, Raw singleton)
{
    host.int_array_resize_00804090(singleton.p + 0x10, 0);
    host.free_00bf6989(singleton.ptr(0x10));
    host.int_array_resize_00804090(singleton.p + 0x4, 0);
    host.free_00bf6989(singleton.ptr(0x4));
    host.recon_slot_global(3) = nullptr;
    singleton.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

namespace {

// The publish step 008069A0 and 00806A60 share: the group unit's record for the slot's party
// (unit + 1E8h + [slot+28h] * 34h) takes `level` at +4h, drops its detector pair (006952A0,
// ECX = [+28h], EDX = record+14h, then +28h = 0), and fires the change delegate when the
// effective level moved.
void publish_group_level(Seg49Host& host, Raw slot, void* group_unit, int level)
{
    Raw record{static_cast<std::uint8_t*>(group_unit) + kUnitDetectionBase + slot.at<int>(0x28) * kDetectionStride};
    const int old_effective = detection_effective_level(record);
    record.at<int>(kDetectionLevel) = level;
    if (void* detector = record.ptr(0x28)) {
        host.observer_unregister_pair_006952a0(detector, record.p + 0x14);
        record.ptr(0x28) = nullptr;
    }
    const int new_effective = detection_effective_level(record);
    if (new_effective != old_effective) {
        host.detection_delegate_0(record.ptr(kDetectionDelegate),
                                  record.at<std::uint32_t>(kDetectionDelegateArg), old_effective, new_effective);
    }
}

}  // namespace

// 008069A0, ABI: __thiscall(ECX = slot, group list) RET 4. Per group entry: the maximum +0Ch
// over its member list (+10h; CMOVLE from 0) is published on the group unit (entry+4h).
void recon_publish_group_level_from_members_008069a0(Seg49Host& host, Raw slot, const ReconList& groups)
{
    for (ReconNode* node = groups.head; node != nullptr; node = node->next) {
        Raw entry{static_cast<std::uint8_t*>(node->payload)};
        int best = 0;
        for (ReconNode* member = entry.at<ReconList>(0x10).head; member != nullptr; member = member->next) {
            const int level = Raw{static_cast<std::uint8_t*>(member->payload)}.at<int>(0x0C);
            if (best <= level) {
                best = level;
            }
        }
        publish_group_level(host, slot, entry.ptr(0x4), best);
    }
}

// 00806A60, ABI: __thiscall(ECX = slot, group list) RET 4. As 008069A0 with the entry's own
// +0Ch as the level (the convoy aggregate).
void recon_publish_group_level_stored_00806a60(Seg49Host& host, Raw slot, const ReconList& groups)
{
    for (ReconNode* node = groups.head; node != nullptr; node = node->next) {
        Raw entry{static_cast<std::uint8_t*>(node->payload)};
        publish_group_level(host, slot, entry.ptr(0x4), entry.at<int>(0x0C));
    }
}

namespace {

// Build the stack temporary (0041E870), append it to the label's string (00425E10), and
// return its storage to the pool when it has data.
void append_temporary(Seg49Host& host, void* string, const char* text)
{
    NativeStringRaw temporary{};
    host.native_string_construct_0041e870(temporary, text);
    host.native_string_append_00425e10(string, temporary);
    if (temporary.data != nullptr) {
        host.pool_return_00bd1510(temporary.data, temporary.length + 1);
    }
}

}  // namespace

// 00806CD0, ABI: __fastcall(ECX = label) RET. No-op without a unit. The level is re-read
// (effective level of the unit's record for the label's party) and a change sets the dirty
// byte. When dirty: level >= 2 first assigns unit vtable +10h's name to +10h (overwritten at
// once by the next assign, as in the image), then the relation word (00803BE0's rule against
// the label's party; "unknown" below level 2), then " " + unit vtable +28h, or " object" below
// level 2; the dirty byte is cleared. The relation table is built in the same stack slots the
// temporary string later occupies (00806D4A..00806DA1).
void recon_unit_label_refresh_00806cd0(Seg49Host& host, Raw label)
{
    void* unit = label.ptr(0x4);
    if (unit == nullptr) {
        return;
    }
    const int party = label.at<int>(0x0);
    const int previous = label.at<int>(0xC);
    Raw record{static_cast<std::uint8_t*>(unit) + kUnitDetectionBase + party * kDetectionStride};
    const int level = detection_effective_level(record);
    label.at<int>(0xC) = level;
    if (level != previous) {
        label.at<std::uint8_t>(0x8) = 1;
    }
    if (label.at<std::uint8_t>(0x8) == 0) {
        return;
    }
    const bool identified = level >= 2;
    void* string = label.p + 0x10;
    if (identified) {
        host.native_string_assign_cstring_0041e350(string, host.unit_name_10(unit));
    }
    int word = 3;
    if (identified) {
        const int unit_party = Raw{static_cast<std::uint8_t*>(unit)}.at<int>(0x54);
        if (party == 2) {
            word = unit_party != 2 ? 2 : 0;
        } else if (party == unit_party) {
            word = 0;
        } else {
            word = (unit_party == 2 ? 1 : 0) + 1;
        }
    }
    host.native_string_assign_cstring_0041e350(string, kLabelRelationWords[word]);
    if (identified) {
        append_temporary(host, string, kLabelSpace);
        append_temporary(host, string, host.unit_display_name_28(unit));
    } else {
        append_temporary(host, string, kLabelObject);
    }
    label.at<std::uint8_t>(0x8) = 0;
}

// 00806EA0, ABI: __fastcall(ECX = label) RET -> EAX. Refresh, then the string data or the
// empty string at 00F874E0.
const char* recon_unit_label_get_cstring_00806ea0(Seg49Host& host, Raw label)
{
    recon_unit_label_refresh_00806cd0(host, label);
    const char* data = static_cast<const char*>(label.ptr(0x14));
    return data != nullptr ? data : host.empty_string_f874e0();
}

// 00806EC0, ABI: __fastcall(ECX = label) RET -> EAX = label+10h (the NativeString).
void* recon_unit_label_get_string_00806ec0(Seg49Host& host, Raw label)
{
    recon_unit_label_refresh_00806cd0(host, label);
    return label.p + 0x10;
}

namespace {

// Unlink a node from a list without freeing it (the in-place unlink the image inlines).
void unlink_node(ReconList& list, ReconNode* node) noexcept
{
    if (node->prev == nullptr) {
        list.head = node->next;
    } else {
        node->prev->next = node->next;
    }
    if (node->next == nullptr) {
        list.tail = node->prev;
    } else {
        node->next->prev = node->prev;
    }
    --list.count;
}

// The first node of `list` carrying `payload` is unlinked and freed (00BF65AC); none: no-op.
void remove_first_payload(Seg49Host& host, ReconList& list, void* payload)
{
    for (ReconNode* node = list.head; node != nullptr; node = node->next) {
        if (node->payload == payload) {
            unlink_node(list, node);
            host.free_00bf65ac(node);
            return;
        }
    }
}

ReconNode* find_unit(ReconList& list, void* unit) noexcept
{
    for (ReconNode* node = list.head; node != nullptr; node = node->next) {
        if (Raw{static_cast<std::uint8_t*>(node->payload)}.ptr(0x4) == unit) {
            return node;
        }
    }
    return nullptr;
}

}  // namespace

// 00806F00, ABI: __thiscall(ECX = slot, notification) RET 4. Recon slot vtable 00D08E94 slot 1.
// Gated on the byte [game+19CCh]+4ACh. The unit is notification vtable +4h. For each id of the
// scan singleton's 24-id array (+10h data, +14h count; 00806480 re-read every step) the unit is
// IsKindOf: its entry is looked up in the own (+34h), enemy (+4C0h), then neutral (+94Ch)
// per-class list (+id*0Ch), first hit wins. The entry's payload is removed from the matching
// flat list (own +DD8h, enemy +DE4h, neutral +DF0h), from unknown +DFCh (enemy and neutral
// only), and from all +E08h; the class node is unlinked, the payload gets vtable[0](1) when
// non-null, the node is freed, and when the local player index is in [0, 8) and the slot's
// party (+28h) is that player's party (+28h) the byte game+193Ch is cleared.
void recon_slot_on_unit_removed_00806f00(Seg49Host& host, Raw slot, void* notification)
{
    Raw game = host.game_e188a8();
    if (Raw{static_cast<std::uint8_t*>(game.ptr(0x19CC))}.at<std::uint8_t>(0x4AC) == 0) {
        return;
    }
    void* unit = host.notification_unit_4(notification);
    for (int k = 0; k < host.recon_scan_class_ids_00806480().at<int>(0x14); ++k) {
        const int id = static_cast<const std::int32_t*>(host.recon_scan_class_ids_00806480().ptr(0x10))[k];
        if (!host.is_kind_of_5c(unit, id)) {
            continue;
        }
        auto& own = slot.at<ReconList>(0x34 + id * 0xC);
        auto& enemy = slot.at<ReconList>(0x4C0 + id * 0xC);
        auto& neutral = slot.at<ReconList>(0x94C + id * 0xC);
        ReconList* class_list = &own;
        ReconNode* node = find_unit(own, unit);
        if (node != nullptr) {
            remove_first_payload(host, slot.at<ReconList>(0xDD8), node->payload);
        } else if ((node = find_unit(enemy, unit)) != nullptr) {
            class_list = &enemy;
            remove_first_payload(host, slot.at<ReconList>(0xDE4), node->payload);
            remove_first_payload(host, slot.at<ReconList>(0xDFC), node->payload);
        } else if ((node = find_unit(neutral, unit)) != nullptr) {
            class_list = &neutral;
            remove_first_payload(host, slot.at<ReconList>(0xDF0), node->payload);
            remove_first_payload(host, slot.at<ReconList>(0xDFC), node->payload);
        } else {
            continue;
        }
        remove_first_payload(host, slot.at<ReconList>(0xE08), node->payload);
        unlink_node(*class_list, node);
        if (node->payload != nullptr) {
            host.scalar_deleting_destructor_0(node->payload, 1);
        }
        host.free_00bf65ac(node);
        const int local_index = game.at<int>(0x18EC);
        if (local_index > -1 && local_index < 8 &&
            slot.at<int>(0x28) == Raw{static_cast<std::uint8_t*>(game.ptr(0x18CC + local_index * 4))}.at<int>(0x28)) {
            game.at<std::uint8_t>(0x193C) = 0;
        }
    }
}

// 00807B40, ABI: __fastcall(ECX = registry) RET. Unwind funclet target: clears 00F874E4 and
// reinstalls the singleton-base vtable.
void sensor_class_registry_reset_singleton_base_00807b40(Seg49Host& host, Raw singleton)
{
    host.sensor_class_registry_f874e4() = nullptr;
    singleton.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00807B60, ABI: __fastcall(ECX = list) RET. The element constructor 00808200 hands to the
// vector-constructor iterator.
void sensor_entry_list_construct_00807b60(RawArray& list) noexcept
{
    list.data = nullptr;
    list.count = 0;
    list.capacity = 0;
}

namespace {

// 00807BD0 / 00807C60: capacity clamped to >= 1; when it exceeds the current capacity, a block
// of capacity * stride (00BF55BE) takes a dword copy of every element (skipped for a null
// block, as the image), the old block is freed (00BF6989, also when null) and the capacity set.
void reserve_raw(Seg49Host& host, RawArray& list, int capacity, std::size_t stride)
{
    if (capacity < 1) {
        capacity = 1;
    }
    if (list.capacity >= capacity) {
        return;
    }
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * stride));
    for (int i = 0; i < list.count; ++i) {
        if (block + i * stride != nullptr) {
            std::memcpy(block + i * stride, list.data + i * stride, stride);
        }
    }
    host.free_00bf6989(list.data);
    list.data = block;
    list.capacity = capacity;
}

// The push_back shape of 00807CF0 / 00807D40 / 00807F10 / 00807F60.
template <void (*Reserve)(Seg49Host&, RawArray&, int)>
void push_back_raw(Seg49Host& host, RawArray& list, const void* element, std::size_t stride)
{
    if (list.count == list.capacity) {
        int capacity = list.capacity * 2;
        if (capacity < 2) {
            capacity = 1;
        }
        Reserve(host, list, capacity);
    }
    std::uint8_t* slot = list.data + list.count * stride;
    if (slot != nullptr) {
        std::memcpy(slot, element, stride);
    }
    ++list.count;
}

}  // namespace

// 00807BD0, ABI: __thiscall(ECX = list, capacity) RET 4. 1Ch entries.
void sensor_entry_list_reserve_00807bd0(Seg49Host& host, RawArray& list, int capacity)
{
    reserve_raw(host, list, capacity, 0x1C);
}

// 00807C60, ABI: __thiscall(ECX = list, capacity) RET 4. 20h rings.
void sensor_gui_range_list_reserve_00807c60(Seg49Host& host, RawArray& list, int capacity)
{
    reserve_raw(host, list, capacity, 0x20);
}

// 00807CF0, ABI: __thiscall(ECX = list, entry*) RET 4. Grows to max(2 * capacity, 1) through
// 00807BD0 and copies seven dwords.
void sensor_entry_list_push_back_00807cf0(Seg49Host& host, RawArray& list, const void* entry)
{
    push_back_raw<sensor_entry_list_reserve_00807bd0>(host, list, entry, 0x1C);
}

// 00807D40, ABI: __thiscall(ECX = list, ring*) RET 4. Twin of 00807F60 through 00807C60.
void sensor_gui_range_list_push_back_00807d40(Seg49Host& host, RawArray& list, const void* ring)
{
    push_back_raw<sensor_gui_range_list_reserve_00807c60>(host, list, ring, 0x20);
}

// 00807D80, ABI: __thiscall(ECX = list, count) RET 4. Reserves (00807BD0) when count exceeds the
// capacity; each new entry gets +4h = FLT_MAX (00D7A248) and byte +18h = 0, its other fields
// untouched (the image unrolls this by four); a shrink only lowers the count.
void sensor_entry_list_resize_00807d80(Seg49Host& host, RawArray& list, int count)
{
    if (list.capacity < count) {
        sensor_entry_list_reserve_00807bd0(host, list, count);
    }
    for (int i = list.count; i < count; ++i) {
        std::uint8_t* entry = list.data + i * 0x1C;
        if (entry != nullptr) {
            Raw{entry}.at<float>(0x4) = kSensorEntryDefaultRange;
            Raw{entry}.at<std::uint8_t>(0x18) = 0;
        }
    }
    while (count < list.count) {
        --list.count;
    }
    list.count = count;
}

// 00807E60, ABI: __thiscall(ECX = list, count) RET 4. As 00807D80 with zeroed 20h rings
// (00807C60).
void sensor_gui_range_list_resize_00807e60(Seg49Host& host, RawArray& list, int count)
{
    if (list.capacity < count) {
        sensor_gui_range_list_reserve_00807c60(host, list, count);
    }
    for (int i = list.count; i < count; ++i) {
        std::uint8_t* ring = list.data + i * 0x20;
        if (ring != nullptr) {
            std::memset(ring, 0, 0x20);
        }
    }
    while (count < list.count) {
        --list.count;
    }
    list.count = count;
}

// 00807F10, ABI: __thiscall(ECX = list, entry*) RET 4. Byte twin of 00807CF0.
void sensor_entry_list_push_back_00807f10(Seg49Host& host, RawArray& list, const void* entry)
{
    push_back_raw<sensor_entry_list_reserve_00807bd0>(host, list, entry, 0x1C);
}

// 00807F60, ABI: __thiscall(ECX = list, ring*) RET 4. The GUIRange destination of 008082A0.
void sensor_gui_range_list_push_back_00807f60(Seg49Host& host, RawArray& list, const void* ring)
{
    push_back_raw<sensor_gui_range_list_reserve_00807c60>(host, list, ring, 0x20);
}

// 00808080, ABI: __fastcall(ECX = list) RET. Resize to 0 (00807D80), free the block.
void sensor_entry_list_destroy_00808080(Seg49Host& host, RawArray& list)
{
    sensor_entry_list_resize_00807d80(host, list, 0);
    host.free_00bf6989(list.data);
}

// 008080A0, ABI: __fastcall(ECX = list) RET. Resize to 0 (00807E60), free the block.
void sensor_gui_range_list_destroy_008080a0(Seg49Host& host, RawArray& list)
{
    sensor_gui_range_list_resize_00807e60(host, list, 0);
    host.free_00bf6989(list.data);
}

// 00808200, ABI: __fastcall(ECX = row) RET -> EAX. The CRT vector-constructor iterator
// (00BF7CD1) runs 00807B60 over seven 0Ch lists (00808080 is its unwind destructor; nothing
// here can throw), then the list at +54h is zeroed.
void* sensor_table_row_construct_00808200(Raw row) noexcept
{
    for (int i = 0; i < 7; ++i) {
        sensor_entry_list_construct_00807b60(row.at<RawArray>(i * 0xC));
    }
    row.at<std::uint32_t>(0x54) = 0;
    row.at<std::uint32_t>(0x58) = 0;
    row.at<std::uint32_t>(0x5C) = 0;
    return row.p;
}

// 00808E40, ABI: __fastcall(ECX = registry) RET. From 00808F60: resize the record array at
// +4h to 0 (00808C20), free its data, clear 00F874E4, reinstall the singleton-base vtable.
void sensor_class_registry_destruct_00808e40(Seg49Host& host, Raw registry)
{
    host.ref_ptr_array_resize_00808c20(registry.p + 0x4, 0);
    host.free_00bf6989(registry.ptr(0x4));
    host.sensor_class_registry_f874e4() = nullptr;
    registry.at<std::uint32_t>(0x0) = kSingletonBaseVtable;
}

// 00808EA0, ABI: __cdecl() -> EAX. Double-checked creation of the 10h registry {vtable
// 00D08FF4, data, capacity, size} under the lifetime manager's lock; registered (00BD0C30)
// even when operator new (00BF681B) returned null, as the image.
void* sensor_class_registry_get_singleton_00808ea0(Seg49Host& host)
{
    if (host.sensor_class_registry_f874e4() == nullptr) {
        host.singleton_lifetime_lock_00415350();
        if (host.sensor_class_registry_f874e4() == nullptr) {
            void* block = host.operator_new_00bf681b(0x10);
            if (block != nullptr) {
                Raw registry{static_cast<std::uint8_t*>(block)};
                registry.at<std::uint32_t>(0x0) = kSensorClassRegistryVtable;
                registry.at<std::uint32_t>(0x4) = 0;
                registry.at<std::uint32_t>(0x8) = 0;
                registry.at<std::uint32_t>(0xC) = 0;
            }
            host.sensor_class_registry_f874e4() = block;
            host.singleton_lifetime_register_00bd0c30(block);
        }
        host.singleton_lifetime_unlock_00415350();
    }
    return host.sensor_class_registry_f874e4();
}

// 008091D0, ABI: __thiscall(ECX = vehicle class, argument) RET 4 -> EAX. The ReconPlane class's
// factory slot: E94h bytes from 00BF55BE, memset to zero before the null test (as the image),
// 0074E0F0(argument) on a non-null block, then 009553D0(class) and +354h = class on the result,
// which a failed allocation leaves null.
void* recon_plane_class_create_unit_instance_008091d0(Seg49Host& host, void* vehicle_class, std::uint32_t argument)
{
    void* block = host.operator_new_00bf55be(0xE94);
    host.memset_00bf79f0(block, 0, 0xE94);
    void* unit = block != nullptr ? host.recon_plane_unit_instance_construct_0074e0f0(block, argument) : nullptr;
    host.unit_set_vehicle_class_009553d0(unit, vehicle_class);
    Raw{static_cast<std::uint8_t*>(unit)}.ptr(0x354) = vehicle_class;
    return unit;
}

// 00809290, ABI: __thiscall(ECX = part, source) RET 4. Called by the SHIP_CREATE (00813AE8) and
// LANDFORT_CREATE (00743E0C) message constructors: part+4h = (source+8h != null); when set,
// part+6h = the word [[source+8h]+18h] vtable +28h returns.
void create_message_store_carrier_type_00809290(Seg49Host& host, Raw message_part, Raw source)
{
    const bool present = source.ptr(0x8) != nullptr;
    message_part.at<std::uint8_t>(0x4) = present ? 1 : 0;
    if (present) {
        void* inner = Raw{static_cast<std::uint8_t*>(source.ptr(0x8))}.ptr(0x18);
        message_part.at<std::uint16_t>(0x6) = host.object_virtual_28_u16(inner);
    }
}

namespace {

// The length schedule 00809380 repeats three times: the components are float differences, the
// square sum is ((x*x + y*y) + z*z) (0080944D..0080945B) stored as float, compared against the
// double 1e-10 (FCOMI/JBE: unordered gives 0), and the root is _CIsqrt stored as float.
float stored_length(Seg49Host& host, float x, float y, float z)
{
    const float sum = static_cast<float>((static_cast<double>(x) * x + static_cast<double>(y) * y) +
                                         static_cast<double>(z) * z);
    if (!(static_cast<double>(sum) > kRepairZoneLengthEpsilon)) {
        return 0.0f;
    }
    return static_cast<float>(host.ci_sqrt_00bf7030(sum));
}

float diff(float a, float b) noexcept
{
    return static_cast<float>(static_cast<double>(a) - b);
}

}  // namespace

// 00809380, ABI: __thiscall(ECX = area, registry, bag) RET 8. Sole caller 00809880. Mode 1
// (bag+4h): the entity named by the bag's "RepairZoneArea" string (008F2260 / 008F0DF0 /
// 00925A90 with ECX = registry); mode 2: the archive object's (bag+8h vtable +18h) word +6h is an
// entity handle resolved through the two 10h-stride tables (below [00F89A10]: [00F89A54] +
// (id - [00F89A0C]) * 10h + 0Ch, else [00F89AA8] + (id - [00F89A60]) * 10h + 0Ch); either way
// +18h = 007AC9D0(entity). Any other mode keeps +18h. Then from path points 0, 1, 2: +34h = |p2 -
// p1|, +1Ch = p0, +28h = p1, +3Ch = 0.5 * (p1 + p0) (sums stored as float first), +38h = |p1 - p0|
// * 0.5, +48h = (+34h + |centre - p0|)^2.
void repair_zone_area_resolve_path_00809380(Seg49Host& host, Raw area, void* registry, Raw bag)
{
    const int mode = bag.at<int>(0x4);
    if (mode == 1 || mode == 2) {
        void* entity = nullptr;
        if (mode == 1) {
            void* property = host.property_bag_find_008f2260(bag.ptr(0x8), kRepairZoneAreaKey);
            entity = host.entity_registry_find_by_name_00925a90(registry,
                                                                host.property_string_value_008f0df0(property));
        } else {
            Raw object{static_cast<std::uint8_t*>(host.archive_object_18(bag.ptr(0x8)))};
            const std::uint32_t id = object.at<std::uint16_t>(0x6);
            std::uint32_t slot = 0;
            if (static_cast<std::int32_t>(id) < static_cast<std::int32_t>(host.read_global_u32(0x00F89A10))) {
                slot = host.read_global_u32(0x00F89A54) + ((id - host.read_global_u32(0x00F89A0C)) << 4) + 0xC;
            } else {
                slot = host.read_global_u32(0x00F89AA8) + ((id - host.read_global_u32(0x00F89A60)) << 4) + 0xC;
            }
            entity = *reinterpret_cast<void**>(static_cast<std::uintptr_t>(slot));
        }
        area.ptr(0x18) = host.entity_path_interface_007ac9d0(entity);
    }
    float a[3];
    float b[3];
    const float* p1 = host.path_transform_point_to_world_007af800(area.ptr(0x18), a, 1);
    const float* p2 = host.path_transform_point_to_world_007af800(area.ptr(0x18), b, 2);
    area.at<float>(0x34) = stored_length(host, diff(p2[0], p1[0]), diff(p2[1], p1[1]), diff(p2[2], p1[2]));
    const float* q = host.path_transform_point_to_world_007af800(area.ptr(0x18), b, 0);
    area.at<float>(0x1C) = q[0];
    area.at<float>(0x20) = q[1];
    area.at<float>(0x24) = q[2];
    q = host.path_transform_point_to_world_007af800(area.ptr(0x18), b, 1);
    area.at<float>(0x28) = q[0];
    area.at<float>(0x2C) = q[1];
    area.at<float>(0x30) = q[2];
    const float sx = static_cast<float>(static_cast<double>(area.at<float>(0x28)) + area.at<float>(0x1C));
    const float sy = static_cast<float>(static_cast<double>(area.at<float>(0x2C)) + area.at<float>(0x20));
    const float sz = static_cast<float>(static_cast<double>(area.at<float>(0x30)) + area.at<float>(0x24));
    area.at<float>(0x3C) = static_cast<float>(sx * kRepairZoneHalf);
    area.at<float>(0x40) = static_cast<float>(sy * kRepairZoneHalf);
    area.at<float>(0x44) = static_cast<float>(sz * kRepairZoneHalf);
    const float* e0 = host.path_transform_point_to_world_007af800(area.ptr(0x18), b, 0);
    const float* e1 = host.path_transform_point_to_world_007af800(area.ptr(0x18), a, 1);
    const float half_length = stored_length(host, diff(e1[0], e0[0]), diff(e1[1], e0[1]), diff(e1[2], e0[2]));
    area.at<float>(0x38) = static_cast<float>(half_length * kRepairZoneHalf);
    const float reach = stored_length(host, diff(area.at<float>(0x3C), area.at<float>(0x1C)),
                                      diff(area.at<float>(0x40), area.at<float>(0x20)),
                                      diff(area.at<float>(0x44), area.at<float>(0x24)));
    const float radius = static_cast<float>(static_cast<double>(area.at<float>(0x34)) + reach);
    area.at<float>(0x48) = static_cast<float>(static_cast<double>(radius) * radius);
}

// 00809740, ABI: __thiscall(ECX = area, owner) RET 4 -> EAX. Sole caller 00809880. Appends the
// area to 00F874F0; a full vector grows to capacity * 2 + 2 through 00BF55BE with a per-slot
// copy (skipped for a null block) and frees the old data (00BF6989) when non-null.
void* repair_zone_area_construct_00809740(Seg49Host& host, Raw area, void* owner)
{
    area.at<std::uint32_t>(0x4) = 0;
    area.at<std::uint32_t>(0x8) = 0;
    area.at<std::uint32_t>(0xC) = 0;
    area.at<std::uint8_t>(0x10) = 0;
    area.at<std::uint32_t>(0x0) = kRepairZoneAreaVtable;
    area.ptr(0x14) = owner;
    RepairZoneVector& areas = host.repair_zone_areas_f874f0();
    void** data = areas.data;
    if (areas.count == areas.capacity) {
        areas.capacity = areas.capacity * 2 + 2;
        data = static_cast<void**>(host.operator_new_00bf55be(areas.capacity * 4));
        for (std::uint32_t i = 0; i < areas.count; ++i) {
            if (data + i != nullptr) {
                data[i] = areas.data[i];
            }
        }
        if (areas.data != nullptr) {
            host.free_00bf6989(areas.data);
        }
    }
    areas.data = data;
    if (areas.data + areas.count != nullptr) {
        areas.data[areas.count] = area.p;
    }
    ++areas.count;
    return area.p;
}

// 00809820, ABI: __fastcall(ECX = area) RET via JMP 00695870. The vector's only remover: the
// first slot holding the area is closed up by shifting the tail down; then the base destructor.
void repair_zone_area_destruct_00809820(Seg49Host& host, Raw area)
{
    area.at<std::uint32_t>(0x0) = kRepairZoneAreaVtable;
    RepairZoneVector& areas = host.repair_zone_areas_f874f0();
    void** end = areas.data + areas.count;
    for (void** it = areas.data; it != end; ++it) {
        if (*it == area.p) {
            for (; it != areas.data + areas.count - 1; ++it) {
                it[0] = it[1];
            }
            --areas.count;
            break;
        }
    }
    host.callback_owner_destroy_00695870(area.p);
}

// 00809880, ABI: __fastcall(ECX = owner, EDX = registry, bag) RET 4 -> EAX. Sole caller
// 00809BC0. A mode-1 bag needs a "RepairZoneArea" property with a non-empty string; any other
// mode builds. operator new(4Ch) (00BF681B), 00809740(owner), then 00809380(registry, bag) on
// the result even when the allocation failed.
void* repair_zone_area_create_from_properties_00809880(Seg49Host& host, void* owner, void* registry, Raw bag)
{
    if (bag.at<int>(0x4) == 1) {
        void* property = host.property_bag_find_008f2260(bag.ptr(0x8), kRepairZoneAreaKey);
        if (property == nullptr) {
            return nullptr;
        }
        const char* text = host.property_string_value_008f0df0(property);
        if (std::strlen(text) == 0) {
            return nullptr;
        }
    }
    void* block = host.operator_new_00bf681b(0x4C);
    void* area = block != nullptr ? repair_zone_area_construct_00809740(host, Raw{static_cast<std::uint8_t*>(block)}, owner)
                                  : nullptr;
    repair_zone_area_resolve_path_00809380(host, Raw{static_cast<std::uint8_t*>(area)}, registry, bag);
    return area;
}

// 00809940, ABI: __thiscall(ECX = area, unused) RET 4. Sole caller 00809C10. When the owner is
// kind 6 (vtable +5Ch(6)): a = +38h * the owner's matrix row at +ECh (each product stored as
// float), centre +3Ch = the owner position (+FCh), +1Ch = position + a, +28h = position - a.
// The owner pose is refreshed (00414DB0) before each of the four reads when +C8h is clear.
void repair_zone_area_update_from_owner_pose_00809940(Seg49Host& host, Raw area, std::uint32_t unused)
{
    static_cast<void>(unused);
    if (!host.is_kind_of_5c(area.ptr(0x14), 6)) {
        return;
    }
    const auto refreshed = [&host, &area]() {
        Raw owner{static_cast<std::uint8_t*>(area.ptr(0x14))};
        if (owner.at<std::uint8_t>(0xC8) == 0) {
            host.entity_pose_refresh_world_00414db0(owner.p);
        }
        return owner;
    };
    Raw owner = refreshed();
    const float half = area.at<float>(0x38);
    const float ax = half * owner.at<float>(0xEC);
    const float ay = owner.at<float>(0xF0) * half;
    const float az = half * owner.at<float>(0xF4);
    owner = refreshed();
    area.at<float>(0x3C) = owner.at<float>(0xFC);
    area.at<float>(0x40) = owner.at<float>(0x100);
    area.at<float>(0x44) = owner.at<float>(0x104);
    owner = refreshed();
    area.at<float>(0x1C) = static_cast<float>(static_cast<double>(owner.at<float>(0xFC)) + ax);
    area.at<float>(0x20) = static_cast<float>(static_cast<double>(owner.at<float>(0x100)) + ay);
    area.at<float>(0x24) = static_cast<float>(static_cast<double>(owner.at<float>(0x104)) + az);
    owner = refreshed();
    area.at<float>(0x28) = diff(owner.at<float>(0xFC), ax);
    area.at<float>(0x2C) = diff(owner.at<float>(0x100), ay);
    area.at<float>(0x30) = diff(owner.at<float>(0x104), az);
}

// 00809A80, ABI: __thiscall(ECX = area, unit, party, position) RET 0Ch -> AL. False without an
// owner, when the owner is the unit, when the owner's +5Ch is clear or +5Dh / +60h / +5Eh set,
// or when 00803510(ECX = party, EDX = owner+54h) is non-zero. Then the horizontal squared
// distance to the centre (held unrounded) must not exceed +48h (JA), and 00419AB0 from {+1Ch,
// +24h} to {+28h, +30h} at {x, z} must not exceed the width +34h (JA; unordered admits).
bool repair_zone_area_contains_00809a80(Seg49Host& host, Raw area, void* unit, int party, const float* position)
{
    Raw owner{static_cast<std::uint8_t*>(area.ptr(0x14))};
    if (owner.p == nullptr || owner.p == unit) {
        return false;
    }
    if (owner.at<std::uint8_t>(0x5C) == 0 || owner.at<std::uint8_t>(0x5D) != 0 || owner.at<std::uint8_t>(0x60) != 0 ||
        owner.at<std::uint8_t>(0x5E) != 0) {
        return false;
    }
    if (host.party_relative_to_00803510(party, owner.at<int>(0x54)) != 0) {
        return false;
    }
    const float px = position[0];
    const float pz = position[2];
    const float dx = diff(px, area.at<float>(0x3C));
    const float dz = diff(pz, area.at<float>(0x44));
    const double d2 = static_cast<double>(dz) * dz + static_cast<double>(dx) * dx;
    if (d2 > static_cast<double>(area.at<float>(0x48))) {
        return false;
    }
    const float point[2] = {px, pz};
    const float end[2] = {area.at<float>(0x28), area.at<float>(0x30)};
    const float start[2] = {area.at<float>(0x1C), area.at<float>(0x24)};
    const float distance = host.segment_distance_00419ab0(start, end, point);
    return !(distance > area.at<float>(0x34));
}

// 00809BC0, ABI: __thiscall(ECX = slot, owner, registry, bag) RET 0Ch. slot+4h = owner; a mode-1
// bag, or a mode-2 bag whose archive object (vtable +18h) is present with byte +4h set, builds
// the area into slot+8h through 00809880(ECX = owner, EDX = registry, bag).
void unit_repair_zone_slot_load_00809bc0(Seg49Host& host, Raw slot, void* owner, void* registry, Raw bag)
{
    slot.ptr(0x4) = owner;
    const int mode = bag.at<int>(0x4);
    if (mode != 1) {
        if (mode != 2) {
            return;
        }
        auto* object = static_cast<std::uint8_t*>(host.archive_object_18(bag.ptr(0x8)));
        if (object == nullptr || object[4] == 0) {
            return;
        }
    }
    slot.ptr(0x8) = repair_zone_area_create_from_properties_00809880(host, owner, registry, bag);
}

// 00809C10, ABI: __thiscall(ECX = slot, argument) RET 4. 00809940 on the area +8h when present.
void unit_repair_zone_slot_update_00809c10(Seg49Host& host, Raw slot, std::uint32_t argument)
{
    if (slot.ptr(0x8) != nullptr) {
        repair_zone_area_update_from_owner_pose_00809940(host, Raw{static_cast<std::uint8_t*>(slot.ptr(0x8))}, argument);
    }
}

// 00809C50, ABI: __fastcall(ECX = unit, EDX = party, position) RET 4 -> EAX. The first area of
// 00F874F0 that 00809A80 accepts, else null.
void* repair_zone_area_find_containing_00809c50(Seg49Host& host, void* unit, int party, const float* position)
{
    RepairZoneVector& areas = host.repair_zone_areas_f874f0();
    for (void** it = areas.data; it != areas.data + areas.count; ++it) {
        if (repair_zone_area_contains_00809a80(host, Raw{static_cast<std::uint8_t*>(*it)}, unit, party, position)) {
            return *it;
        }
    }
    return nullptr;
}

// 00809F20, ABI: __fastcall(ECX = object) RET. Slot +3Ch of vtable 00D09078 (the rocket's +310h
// sub-object): with +184h set, 006E1CD0 then, when +28h is non-null, a tail jump to 00866B70
// (ECX = +28h).
void rocket_projectile_on_motor_ignition_00809f20(Seg49Host& host, Raw object)
{
    if (object.at<std::uint8_t>(0x184) == 0) {
        return;
    }
    host.projectile_shot_ignite_motor_006e1cd0(object.p);
    if (object.ptr(0x28) != nullptr) {
        host.effect_handle_request_stop_00866b70(object.ptr(0x28));
    }
}

// 00809F50, ABI: __thiscall(ECX = rocket, source) RET 4. MRocket vtable slot +190h:
// +318h..+320h = source vtable +34h(&local).
void rocket_projectile_store_vector_190_00809f50(Seg49Host& host, Raw rocket, void* source)
{
    float local[3];
    const float* v = host.object_vector_34(source, local);
    rocket.at<float>(0x318) = v[0];
    rocket.at<float>(0x31C) = v[1];
    rocket.at<float>(0x320) = v[2];
}

// 00809F90, ABI: __fastcall(ECX = object) RET -> EAX = object. 00470470 on object+1Ch.
void* hit_record_holder_reset_00809f90(Seg49Host& host, Raw object)
{
    host.hit_record_reset_00470470(object.p + 0x1C);
    return object.p;
}

// 00809FB0, ABI: __fastcall(ECX = homing block, rocket+468h) RET. The clear arm of 0080AA20:
// drop the target pair (006952A0, ECX = +14h, EDX = block), point +18h = 00F87574..7Ch, angle
// +24h = 0.
void rocket_clear_homing_target_00809fb0(Seg49Host& host, Raw block)
{
    if (block.ptr(0x14) != nullptr) {
        host.observer_unregister_pair_006952a0(block.ptr(0x14), block.p);
        block.ptr(0x14) = nullptr;
    }
    const float* zero = host.zero_vector_f87574();
    block.at<float>(0x18) = zero[0];
    block.at<float>(0x1C) = zero[1];
    block.at<float>(0x20) = zero[2];
    block.at<float>(0x24) = 0.0f;
}

// 0080AAD0, ABI: __fastcall(ECX = homing block) RET. Observer endpoint (vtable 00CE75CC,
// +4h/+8h/+0Ch/+14h zero, byte +10h = 1), point = 00F87574..7Ch, angle 0.
void rocket_homing_target_construct_0080aad0(Seg49Host& host, Raw block)
{
    block.at<std::uint32_t>(0x04) = 0;
    block.at<std::uint32_t>(0x08) = 0;
    block.at<std::uint32_t>(0x0C) = 0;
    block.at<std::uint32_t>(0x14) = 0;
    block.at<std::uint8_t>(0x10) = 1;
    block.at<std::uint32_t>(0x00) = kObserverEndpointVtable;
    const float* zero = host.zero_vector_f87574();
    block.at<float>(0x18) = zero[0];
    block.at<float>(0x1C) = zero[1];
    block.at<float>(0x20) = zero[2];
    block.at<float>(0x24) = 0.0f;
}

// 0080AB40, ABI: __thiscall(ECX = rocket, argument) RET 4 -> EAX. 006E2670(argument), the
// seven MRocket vtables, the homing block at +468h (0080AAD0's layout inline), +490h = 0.0f,
// byte +494h = 0, class id +C4h = 33h.
void* rocket_projectile_construct_0080ab40(Seg49Host& host, Raw rocket, std::uint32_t argument)
{
    host.bomb_projectile_construct_006e2670(rocket.p, argument);
    rocket.at<std::uint32_t>(0x000) = kRocketVtable;
    rocket.at<std::uint32_t>(0x010) = kRocketVtable10;
    rocket.at<std::uint32_t>(0x024) = kRocketVtable24;
    rocket.at<std::uint32_t>(0x170) = kRocketVtable170;
    rocket.at<std::uint32_t>(0x1E4) = kRocketVtable1E4;
    rocket.at<std::uint32_t>(0x310) = kRocketVtable310;
    rocket.at<std::uint32_t>(0x3E4) = kRocketVtable3E4;
    rocket_homing_target_construct_0080aad0(host, Raw{rocket.p + 0x468});
    rocket.at<std::uint8_t>(0x494) = 0;
    rocket.at<int>(0xC4) = kRocketClassId;
    rocket.at<float>(0x490) = 0.0f;
    return rocket.p;
}

// 0080AC60, ABI: RET. MRocket vtable slot +18Ch: does nothing.
void rocket_projectile_noop_18c_0080ac60() noexcept {}

// 0080ACF0, ABI: __fastcall(ECX = rocket) RET. From 0080ADC0: 004B7EF0 on the homing block
// (+468h), then the bomb base destructor 006E2A20.
void rocket_projectile_destruct_0080acf0(Seg49Host& host, Raw rocket)
{
    host.observer_link_destroy_base_004b7ef0(rocket.p + 0x468);
    host.bomb_projectile_destruct_006e2a20(rocket.p);
}

// 0080AD40, ABI: __fastcall(ECX = rocket) RET. Slot +130h: 00928560, then the world's
// ([+30h]) lists +30h, +210h and +27Ch take the rocket (00484540).
void rocket_projectile_register_in_world_lists_0080ad40(Seg49Host& host, Raw rocket)
{
    host.game_entity_register_in_parent_list_00928560(rocket.p);
    auto* world = static_cast<std::uint8_t*>(rocket.ptr(0x30));
    host.unit_list_push_back_00484540(world + 0x30, rocket.p);
    world = static_cast<std::uint8_t*>(rocket.ptr(0x30));
    host.unit_list_push_back_00484540(world + 0x210, rocket.p);
    world = static_cast<std::uint8_t*>(rocket.ptr(0x30));
    host.unit_list_push_back_00484540(world + 0x27C, rocket.p);
}

// 0080AD80, ABI: __fastcall(ECX = rocket) RET. Slot +134h: 006E29C0, then the first node of
// the world list +27Ch (head +280h, {+4h next, +8h entity}) carrying the rocket is erased
// (004837D0).
void rocket_projectile_unlink_from_world_lists_0080ad80(Seg49Host& host, Raw rocket)
{
    host.bomb_family_detach_from_world_lists_006e29c0(rocket.p);
    auto* world = static_cast<std::uint8_t*>(rocket.ptr(0x30));
    for (void* node = Raw{world}.ptr(0x280); node != nullptr; node = Raw{static_cast<std::uint8_t*>(node)}.ptr(0x4)) {
        if (Raw{static_cast<std::uint8_t*>(node)}.ptr(0x8) == rocket.p) {
            host.unit_list_erase_node_004837d0(world + 0x27C, node);
            return;
        }
    }
}

// 0080ADE0, ABI: __thiscall(ECX = rocket class, argument, party, position*, direction*, three
// unused) RET 1Ch -> EAX = rocket+310h. 498h bytes from 00BF55BE memset to zero before the null
// test; 0080AB40(argument). +54h = party, +314h = class; vtable +98h(0, [game+19CCh], identity
// 4x4). Velocity +318h = direction * [class+50h] (each product stored as float); matrix row 2
// (+94h) = direction, orthonormalised (0085DC80 on +74h); translation +A4h, +370h and +37Ch =
// position; +C8h/+10Ch cleared and the child poses invalidated (0042ED50 over +48h / +44h).
// A failed allocation is dereferenced, as in the image.
void* rocket_class_create_projectile_0080ade0(Seg49Host& host, void* rocket_class, std::uint32_t argument, int party,
                                              const float* position, const float* direction)
{
    void* block = host.operator_new_00bf55be(0x498);
    host.memset_00bf79f0(block, 0, 0x498);
    Raw rocket{static_cast<std::uint8_t*>(
        block != nullptr ? rocket_projectile_construct_0080ab40(host, Raw{static_cast<std::uint8_t*>(block)}, argument)
                         : nullptr)};
    rocket.at<int>(0x54) = party;
    rocket.ptr(0x314) = rocket_class;
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    host.rocket_virtual_98(rocket.p, 0, host.game_e188a8().ptr(0x19CC), identity);
    const float speed = Raw{static_cast<std::uint8_t*>(rocket.ptr(0x314))}.at<float>(0x50);
    rocket.at<float>(0x318) = direction[0] * speed;
    rocket.at<float>(0x31C) = direction[1] * speed;
    rocket.at<float>(0x320) = speed * direction[2];
    float* local = &rocket.at<float>(0x74);
    local[8] = direction[0];
    local[9] = direction[1];
    local[10] = direction[2];
    host.orthonormalize_basis_rows_0085dc80(local);
    rocket.at<float>(0xA4) = position[0];
    rocket.at<float>(0xA8) = position[1];
    rocket.at<float>(0xAC) = position[2];
    rocket.at<float>(0x370) = position[0];
    rocket.at<float>(0x374) = position[1];
    rocket.at<float>(0x378) = position[2];
    rocket.at<float>(0x37C) = position[0];
    rocket.at<float>(0x380) = position[1];
    rocket.at<float>(0x384) = position[2];
    rocket.at<std::uint8_t>(0xC8) = 0;
    rocket.at<std::uint8_t>(0x10C) = 0;
    for (void* child = rocket.ptr(0x48); child != nullptr; child = Raw{static_cast<std::uint8_t*>(child)}.ptr(0x44)) {
        host.scene_node_invalidate_subtree_pose_0042ed50(child);
    }
    return rocket.p + 0x310;
}

// ---------------------------------------------------------------------------
// Turn-path planner. The image runs x87 at 53-bit precision: every FSTP to a float slot is a
// float cast here and intermediates are doubles. FSIN/FCOS are std::sin/std::cos on the float
// argument (the x87 instructions agree with them to within the final float rounding for the
// small arguments the planner produces; not bit-proven).
// ---------------------------------------------------------------------------

// 0080B1F0, ABI: __thiscall(ECX = segment, float* out) RET 4. Kinds 0 and 1: the end point
// +14h. Kind 2: centre + radius * (cos a1, -, sin a1) with y = +24h (each sine / cosine stored
// as float, then float(r * c + centre)). Anything else: the zero vector 00F87574.
void turn_path_segment_end_point_0080b1f0(Seg49Host& host, Raw segment, float* out)
{
    const int kind = segment.at<int>(0x0);
    if (kind >= 0 && kind <= 1) {
        out[0] = segment.at<float>(0x14);
        out[1] = segment.at<float>(0x18);
        out[2] = segment.at<float>(0x1C);
        return;
    }
    if (kind == 2) {
        const float cx = segment.at<float>(0x20);
        const float cz = segment.at<float>(0x28);
        const float angle = segment.at<float>(0x34);
        const auto c = static_cast<float>(std::cos(static_cast<double>(angle)));
        const float x = static_cast<float>(static_cast<double>(segment.at<float>(0x2C)) * c + cx);
        const auto s = static_cast<float>(std::sin(static_cast<double>(angle)));
        const float z = static_cast<float>(static_cast<double>(segment.at<float>(0x2C)) * s + cz);
        out[1] = segment.at<float>(0x24);
        out[0] = x;
        out[2] = z;
        return;
    }
    const float* zero = host.zero_vector_f87574();
    out[0] = zero[0];
    out[1] = zero[1];
    out[2] = zero[2];
}

namespace {

// The 1e-10-gated root 0080B310 / 0080B530 share (the 00809380 schedule, two or three terms).
float gated_root(Seg49Host& host, double square_sum)
{
    const auto stored = static_cast<float>(square_sum);
    if (!(static_cast<double>(stored) > kRepairZoneLengthEpsilon)) {
        return 0.0f;
    }
    return static_cast<float>(host.ci_sqrt_00bf7030(stored));
}

}  // namespace

// 0080B310, ABI: __fastcall(ECX = path) RET -> ST0. Each segment's +3Ch takes the running length;
// a line adds |(end - start).xz| (dz*dz + dx*dx, 1e-10 gate), an arc adds radius * the wrapped
// sweep (0080B190 on a1 - a0, or a0 - a1 when the direction byte is clear), stored as float; the
// sum is written to +40h. Kind 0 adds nothing.
float turn_path_length_0080b310(Seg49Host& host, Raw path)
{
    float sum = 0.0f;
    auto* segment = static_cast<std::uint8_t*>(path.ptr(0x0));
    auto* end = segment + path.at<int>(0x4) * 0x44;
    for (; segment != end; segment += 0x44) {
        Raw s{segment};
        s.at<float>(0x3C) = sum;
        const int kind = s.at<int>(0x0);
        if (kind == 1) {
            const float dx = diff(s.at<float>(0x14), s.at<float>(0x08));
            const float dz = diff(s.at<float>(0x1C), s.at<float>(0x10));
            const float length = gated_root(host, static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
            sum = static_cast<float>(static_cast<double>(length) + sum);
        } else if (kind == 2) {
            float sweep = s.at<std::uint8_t>(0x38) != 0 ? diff(s.at<float>(0x34), s.at<float>(0x30))
                                                       : diff(s.at<float>(0x30), s.at<float>(0x34));
            bsp::cc13_band0080::turn_path_wrap_angle_0080b190(sweep);
            sum = static_cast<float>(static_cast<double>(s.at<float>(0x2C)) * sweep + sum);
        }
        s.at<float>(0x40) = sum;
    }
    return sum;
}

// 0080B530, ABI: __fastcall(ECX = circle {x, z, radius, direction byte +0Ch}, EDX = float*
// out angle) RET -> AL. The distance d from the circle centre to the goal (dy = gy - gy, so 0
// for a finite goal; sum (dy^2 + dx^2) + dz^2, 1e-10 gate) must satisfy radius - 300 <= d <=
// radius + 500 (both bounds stored as float). Without the heading constraint (byte +20h) that
// is the answer; with it, a = atan2(dz, dx) is stored to out, turned by pi/2 (00438AA0 add when
// the direction byte is set, 00438B10 subtract otherwise) and the answer is 006DEE40(turned,
// goal heading +24h, 20 degrees).
bool turn_path_circle_reaches_goal_0080b530(Seg49Host& host, Raw circle, float* out_angle)
{
    Raw state = host.global_object(kTurnPathState);
    const float cx = circle.at<float>(0x0);
    const float cz = circle.at<float>(0x4);
    const float gy = state.at<float>(0x18);
    const float dx = diff(state.at<float>(0x14), cx);
    const float dy = diff(gy, gy);
    const float dz = diff(state.at<float>(0x1C), cz);
    const float distance =
        gated_root(host, (static_cast<double>(dy) * dy + static_cast<double>(dx) * dx) + static_cast<double>(dz) * dz);
    const float radius = circle.at<float>(0x8);
    const auto low = static_cast<float>(radius - kTurnPathMinReach);
    if (low > distance) {
        return false;
    }
    const auto high = static_cast<float>(radius + kTurnPathMaxReach);
    if (distance > high) {
        return false;
    }
    if (state.at<std::uint8_t>(0x20) == 0) {
        return true;
    }
    const auto angle = static_cast<float>(host.ci_atan2_00bf701a(dz, dx));
    *out_angle = angle;
    const float turned = circle.at<std::uint8_t>(0xC) != 0
                             ? host.wrapped_angle_add_00438aa0(angle, kTurnPathHalfPi)
                             : host.wrapped_angle_subtract_00438b10(angle, kTurnPathHalfPi);
    return host.gun_bot_angle_within_tolerance_006dee40(turned, state.at<float>(0x24), kTurnPathHeadingTolerance);
}

// 0080B690, ABI: __fastcall(ECX = path) RET. +0Ch = the path length (0080B310); slope = (goal
// height - start height) / length stored as float; byte +10h = atan(slope) (00BF8490, stored as
// float) outside [-10, +10] degrees (an unordered angle counts as inside). Inside, each line's
// start and end heights (+0Ch, +18h) and each arc's height (+24h) become
// float(running length * slope + start height) from +3Ch / +40h (arcs: +40h).
void turn_path_apply_climb_0080b690(Seg49Host& host, Raw path)
{
    const float length = turn_path_length_0080b310(host, path);
    path.at<float>(0xC) = length;
    Raw state = host.global_object(kTurnPathState);
    // 0080B6A6..0080B6B4: the difference stays in the register (not stored) before FDIVRP.
    const double rise = static_cast<double>(state.at<float>(0x18)) - state.at<float>(0x04);
    const auto slope = static_cast<float>(rise / length);
    const auto climb = static_cast<float>(host.ci_atan_00bf8490(slope));
    const bool outside = kTurnPathMinClimb > climb || climb > kTurnPathMaxClimb;
    path.at<std::uint8_t>(0x10) = outside ? 1 : 0;
    if (outside) {
        return;
    }
    auto* segment = static_cast<std::uint8_t*>(path.ptr(0x0));
    auto* end = segment + path.at<int>(0x4) * 0x44;
    for (; segment != end; segment += 0x44) {
        Raw s{segment};
        const double base = state.at<float>(0x04);
        if (s.at<int>(0x0) == 1) {
            s.at<float>(0x0C) = static_cast<float>(static_cast<double>(s.at<float>(0x3C)) * slope + base);
            s.at<float>(0x18) = static_cast<float>(static_cast<double>(s.at<float>(0x40)) * slope + base);
        } else if (s.at<int>(0x0) == 2) {
            s.at<float>(0x24) = static_cast<float>(static_cast<double>(s.at<float>(0x40)) * slope + base);
        }
    }
}

// 0080B7D0, ABI: __thiscall(ECX = holder, capacity) RET 4. holder+0h points at a header {data
// (16-aligned, inside the block), count +4h, capacity +8h} that heads its own storage. A larger
// capacity allocates capacity * 4 + 18h (00BF55BE), copies the OLD CAPACITY's worth of
// elements (memcpy, only when there is a header with a non-zero capacity), keeps the count and
// frees the old block (00BF6989); otherwise only the capacity field is overwritten (no null
// check, as the image).
void aligned_buffer_reserve_0080b7d0(Seg49Host& host, Raw holder, int capacity)
{
    auto* header = static_cast<std::uint8_t*>(holder.ptr(0x0));
    const int current = header != nullptr ? Raw{header}.at<int>(0x8) : 0;
    if (capacity <= current) {
        Raw{header}.at<int>(0x8) = capacity;
        return;
    }
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf55be(capacity * 4 + 0x18));
    auto* data = reinterpret_cast<std::uint8_t*>((reinterpret_cast<std::uintptr_t>(block) + 0x18) & ~std::uintptr_t{0xF});
    header = static_cast<std::uint8_t*>(holder.ptr(0x0));
    if (header != nullptr && Raw{header}.at<int>(0x8) != 0) {
        host.memcpy_00bf7680(data, Raw{header}.ptr(0x0), Raw{header}.at<int>(0x8) * 4);
    }
    Raw fresh{block};
    fresh.ptr(0x0) = data;
    header = static_cast<std::uint8_t*>(holder.ptr(0x0));
    fresh.at<int>(0x4) = header != nullptr ? Raw{header}.at<int>(0x4) : 0;
    fresh.at<int>(0x8) = capacity;
    host.free_00bf6989(holder.ptr(0x0));
    holder.ptr(0x0) = block;
}

// 0080B890, ABI: __thiscall(ECX = destination, source) RET 4. The 44h segment copy, field by
// field: dwords +00h and +04h by MOV, floats +08h..+34h, +3Ch and +40h by FLD/FSTP DWORD (a
// signalling NaN is quieted; cc13-fixture10), the byte +38h alone.
void turn_path_segment_copy_0080b890(Raw destination, Raw source) noexcept
{
    const auto fld_fstp = [&](std::size_t off) {
        destination.at<std::uint32_t>(off) = cc13x87::x87_fld_fstp_bits(source.at<std::uint32_t>(off));
    };
    destination.at<std::uint32_t>(0x0) = source.at<std::uint32_t>(0x0);
    destination.at<std::uint32_t>(0x4) = source.at<std::uint32_t>(0x4);
    for (std::size_t off = 0x8; off < 0x38; off += 4) fld_fstp(off);
    destination.at<std::uint8_t>(0x38) = source.at<std::uint8_t>(0x38);
    fld_fstp(0x3C);
    fld_fstp(0x40);
}

// 0080BB60, ABI: __fastcall(ECX = path) RET. No caller. 0080B1F0 on every segment into one
// discarded stack vector (a debug-draw loop with its drawing compiled out).
void turn_path_visit_end_points_0080bb60(Seg49Host& host, Raw path)
{
    float point[3];
    auto* segments = static_cast<std::uint8_t*>(path.ptr(0x0));
    turn_path_segment_end_point_0080b1f0(host, Raw{segments}, point);
    for (int i = 1; i < path.at<int>(0x4); ++i) {
        turn_path_segment_end_point_0080b1f0(host, Raw{static_cast<std::uint8_t*>(path.ptr(0x0)) + i * 0x44}, point);
    }
}

// 0080BCB0, ABI: __thiscall(ECX = holder, const dword*) RET 4. When count == capacity (both 0
// without a header), 0080B7D0(max(capacity + capacity / 4, 32)) (signed division; 32 for any
// result up to 32); then data[count] = value and count + 1 (no null check: a missing header
// faults, as the image).
void aligned_buffer_push_0080bcb0(Seg49Host& host, Raw holder, const std::uint32_t* value)
{
    auto* header = static_cast<std::uint8_t*>(holder.ptr(0x0));
    const int count = header != nullptr ? Raw{header}.at<int>(0x4) : 0;
    const int capacity = header != nullptr ? Raw{header}.at<int>(0x8) : 0;
    if (count == capacity) {
        int grown = capacity / 4 + capacity;
        if (grown < 0x21) {
            grown = 0x20;
        }
        aligned_buffer_reserve_0080b7d0(host, holder, grown);
    }
    Raw h{static_cast<std::uint8_t*>(holder.ptr(0x0))};
    static_cast<std::uint32_t*>(h.ptr(0x0))[h.at<int>(0x4)] = *value;
    h.at<int>(0x4) += 1;
}

// 0080BD90, ABI: __stdcall(six floats) RET 18h -> EAX. A 20h node (00BF681B), +8h..+1Ch zeroed
// when allocated, then the six values stored at +8h..+1Ch (a failed allocation faults).
void* turn_path_new_candidate_0080bd90(Seg49Host& host, const float* values)
{
    auto* node = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x20));
    if (node != nullptr) {
        std::memset(node + 8, 0, 0x18);
    }
    for (int i = 0; i < 6; ++i) {
        Raw{node}.at<float>(8 + i * 4) = values[i];
    }
    return node;
}

// 0080BEA0, ABI: __thiscall(ECX = list {count, head, tail}, six dwords) RET 18h. The same 20h
// node ({prev, next, six dwords}) appended at the tail.
void turn_path_candidate_list_push_0080bea0(Seg49Host& host, ReconList& list, const std::uint32_t* values)
{
    auto* node = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x20));
    if (node != nullptr) {
        std::memset(node + 8, 0, 0x18);
    }
    for (int i = 0; i < 6; ++i) {
        Raw{node}.at<std::uint32_t>(8 + i * 4) = values[i];
    }
    auto* as_node = reinterpret_cast<ReconNode*>(node);
    as_node->prev = list.tail;
    if (list.count != 0) {
        list.tail->next = as_node;
    } else {
        list.head = as_node;
    }
    list.tail = as_node;
    as_node->next = nullptr;
    ++list.count;
}

namespace {

// 0080BA23..0080BA6A: fmod by the double 2pi (_CIfmod 00BF857A), stored as float, then one
// wrap into (-pi, pi]: not above -pi adds 2pi, above pi subtracts it (each stored as float; an
// unordered value is kept).
float wrap_turn_angle(float angle)
{
    const auto reduced = static_cast<float>(std::fmod(static_cast<double>(angle), kTurnPathTwoPi));
    if (!(-kTurnPathPi < reduced)) {
        if (!std::isnan(reduced)) {
            return static_cast<float>(reduced + kTurnPathTwoPi);
        }
        return reduced;
    }
    if (static_cast<double>(reduced) > kTurnPathPi) {
        return static_cast<float>(reduced - kTurnPathTwoPi);
    }
    return reduced;
}

}  // namespace

// 0080BA00, ABI: __thiscall(ECX = out[6], ten floats) RET 28h -> EAX = out. out is zeroed, then
// = {x0 + r0 cos a0, z0 + r0 sin a0, a0, x1 + r1 cos a1, z1 + r1 sin a1, a1} with both angles
// reduced by wrap_turn_angle first and every sine / cosine stored as float. Arguments 3 and 8
// are not read.
float* turn_path_tangent_points_0080ba00(float* out, const float* args)
{
    for (int i = 0; i < 6; ++i) {
        out[i] = 0.0f;
    }
    const float a0 = wrap_turn_angle(args[4]);
    const float a1 = wrap_turn_angle(args[9]);
    const auto s0 = static_cast<float>(std::sin(static_cast<double>(a0)));
    const auto c0 = static_cast<float>(std::cos(static_cast<double>(a0)));
    out[0] = static_cast<float>(static_cast<double>(c0) * args[2] + args[0]);
    out[1] = static_cast<float>(static_cast<double>(args[2]) * s0 + args[1]);
    const auto s1 = static_cast<float>(std::sin(static_cast<double>(a1)));
    const auto c1 = static_cast<float>(std::cos(static_cast<double>(a1)));
    out[3] = args[5];
    out[4] = args[6];
    out[2] = a0;
    out[5] = a1;
    out[3] = static_cast<float>(static_cast<double>(c1) * args[7] + out[3]);
    out[4] = static_cast<float>(static_cast<double>(args[7]) * s1 + out[4]);
    return out;
}

// 0080C060, ABI: __thiscall(ECX = candidate list, x1, z1, r1, d1, x2, z2, r2, d2) RET 20h (the
// direction bytes are dword slots). The list is cleared (0080BD40). d = |c1 - c2| (dx*dx + dz*dz,
// 1e-10 gate). Opposite directions with r1 + r2 > d produce nothing. The circle with the larger
// radius becomes the second (swap when r1 > r2). h = atan2(z2 - z1, x2 - x1), hb = float(pi + h).
// beta: same direction: k = float(r2 / r1 - 1); k <= 0 gives pi/2, else pi/2 - asin(r1 / (d / k));
// opposite: pi/2 - asin(r1 / (d / float(r2 / r1 + 1))) (each quotient stored as float, asin by
// 0042CF10). The two tangent angles by (d1, d2, swapped):
//   d1 set, d2 set: both h - beta (unswapped) or h + beta (swapped);
//   d1 set, d2 clear: (h - beta, hb - beta) or (h + beta, hb + beta);
//   d1 clear, d2 clear: both h + beta or h - beta;
//   d1 clear, d2 set: (h + beta, hb + beta) or (h - beta, hb - beta);
// and 0080BA00 gets the circles in their original order with those angles; its six floats are
// appended as one candidate (0080BEA0). Each angle is stored as float.
void turn_path_circle_to_circle_candidates_0080c060(Seg49Host& host, ReconList& candidates, TurnCircle first,
                                                    TurnCircle second)
{
    host.dlist_clear_0080bd40(&candidates);
    const float dx = diff(first.x, second.x);
    const float dz = diff(first.z, second.z);
    const float d = gated_root(host, static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
    if (static_cast<double>(first.radius) + second.radius > d && first.direction != second.direction) {
        return;
    }
    bool swapped = false;
    if (first.radius > second.radius) {
        std::swap(first, second);
        swapped = true;
    }
    const auto h = static_cast<float>(host.ci_atan2_00bf701a(diff(second.z, first.z), diff(second.x, first.x)));
    const double q = static_cast<double>(second.radius) / first.radius;
    float beta = 0.0f;
    bool have_ratio = true;
    float u = 0.0f;
    if (first.direction == second.direction) {
        const auto k = static_cast<float>(q - 1.0);
        if (!(k > 0.0f)) {
            beta = kTurnPathHalfPi;
            have_ratio = false;
        } else {
            const auto t = static_cast<float>(static_cast<double>(d) / k);
            u = static_cast<float>(static_cast<double>(first.radius) / t);
        }
    } else {
        const auto k = static_cast<float>(q + 1.0);
        const auto t = static_cast<float>(static_cast<double>(d) / k);
        u = static_cast<float>(static_cast<double>(first.radius) / t);
    }
    if (have_ratio) {
        const float a = host.asin_clamped_0042cf10(u);
        beta = static_cast<float>(kTurnPathHalfPiDouble - a);
    }
    const auto hb = static_cast<float>(kTurnPathPi + h);
    const auto plus = [](float base, float offset) { return static_cast<float>(static_cast<double>(base) + offset); };
    const auto minus = [](float base, float offset) { return static_cast<float>(static_cast<double>(base) - offset); };
    float angle_first = 0.0f;   // the (post-swap) first circle's angle
    float angle_second = 0.0f;  // the (post-swap) second circle's angle
    if (first.direction != 0) {
        if (second.direction != 0) {
            angle_first = angle_second = swapped ? plus(h, beta) : minus(h, beta);
        } else if (!swapped) {
            angle_first = minus(h, beta);
            angle_second = minus(hb, beta);
        } else {
            angle_first = plus(h, beta);
            angle_second = plus(hb, beta);
        }
    } else if (second.direction == 0) {
        angle_first = angle_second = swapped ? minus(h, beta) : plus(h, beta);
    } else if (!swapped) {
        angle_first = plus(h, beta);
        angle_second = plus(hb, beta);
    } else {
        angle_first = minus(h, beta);
        angle_second = minus(hb, beta);
    }
    // The argument order restores the caller's order: the post-swap second circle goes first when
    // the circles were swapped.
    const TurnCircle& a = swapped ? second : first;
    const TurnCircle& b = swapped ? first : second;
    const float angle_a = swapped ? angle_second : angle_first;
    const float angle_b = swapped ? angle_first : angle_second;
    float args[10] = {a.x, a.z, a.radius, 0.0f, angle_a, b.x, b.z, b.radius, 0.0f, angle_b};
    float out[6];
    turn_path_tangent_points_0080ba00(out, args);
    std::uint32_t values[6];
    std::memcpy(values, out, sizeof values);
    turn_path_candidate_list_push_0080bea0(host, candidates, values);
}

// 0080C590, ABI: __thiscall(ECX = candidate list, x, z, r, direction, px, pz) RET 18h. No caller.
// d = |c - p| (dz*dz + dx*dx, 1e-10 gate); h = atan2(pz - z, px - x); angle = float(h +
// float(pi/2 - 0042CF10(float(r / d)))). 0080BA00(circle with angle, {px, pz, 0, -, angle}) and
// the six floats appended (0080BEA0). The list is not cleared and the direction is not used in
// the arithmetic.
void turn_path_circle_to_point_candidate_0080c590(Seg49Host& host, ReconList& candidates, TurnCircle circle,
                                                  float point_x, float point_z)
{
    const float dx = diff(circle.x, point_x);
    const float dz = diff(circle.z, point_z);
    const float d = gated_root(host, static_cast<double>(dz) * dz + static_cast<double>(dx) * dx);
    const auto h = static_cast<float>(host.ci_atan2_00bf701a(diff(point_z, circle.z), diff(point_x, circle.x)));
    const auto ratio = static_cast<float>(static_cast<double>(circle.radius) / d);
    const auto beta = static_cast<float>(kTurnPathHalfPiDouble - host.asin_clamped_0042cf10(ratio));
    const auto angle = static_cast<float>(static_cast<double>(beta) + h);
    float args[10] = {circle.x, circle.z, circle.radius, 0.0f, angle, point_x, point_z, 0.0f, 0.0f, angle};
    float out[6];
    turn_path_tangent_points_0080ba00(out, args);
    std::uint32_t values[6];
    std::memcpy(values, out, sizeof values);
    turn_path_candidate_list_push_0080bea0(host, candidates, values);
}

namespace {

// The 44h-array clear 0080C710 / 0080C750 / 0080C7F0 / 0080CF60 inline: a negative capacity
// reserves 0 (0080BC20), the count is stepped down to zero, then set to zero.
void clear_segments(Seg49Host& host, RawArray& segments)
{
    if (segments.capacity < 0) {
        host.array44_reserve_0080bc20(&segments, 0);
    }
    while (segments.count > 0) {
        --segments.count;
    }
    segments.count = 0;
}

}  // namespace

// 0080C710, ABI: __fastcall(ECX = segments) RET. Clear, then free the data (00BF6989).
void segment_array_destroy_0080c710(Seg49Host& host, RawArray& segments)
{
    clear_segments(host, segments);
    host.free_00bf6989(segments.data);
}

// 0080C750, ABI: __thiscall(ECX = self, source) RET 4 -> EAX = self. Clear, reserve the source
// count (0080BC20), then per source segment: grow to max(2 * capacity, 1) when full, copy it
// (0080B890) unless the slot address is null, count + 1.
RawArray* segment_array_assign_0080c750(Seg49Host& host, RawArray& self, const RawArray& source)
{
    clear_segments(host, self);
    host.array44_reserve_0080bc20(&self, source.count);
    for (int i = 0; i < source.count; ++i) {
        const std::uint8_t* from = source.data + i * 0x44;
        if (self.count == self.capacity) {
            int capacity = self.capacity * 2;
            if (capacity < 2) {
                capacity = 1;
            }
            host.array44_reserve_0080bc20(&self, capacity);
        }
        std::uint8_t* slot = self.data + self.count * 0x44;
        if (slot != nullptr) {
            turn_path_segment_copy_0080b890(Raw{slot}, Raw{const_cast<std::uint8_t*>(from)});
        }
        ++self.count;
    }
    return &self;
}

// 0080C7F0, ABI: __fastcall(ECX = segments) RET. Byte twin of 0080C710.
void segment_array_destroy_0080c7f0(Seg49Host& host, RawArray& segments)
{
    clear_segments(host, segments);
    host.free_00bf6989(segments.data);
}

// 0080C830, ABI: __thiscall(ECX = self, source) RET 4 -> EAX. A 3Ch path: the segment array
// (0080C750), then +0Ch, byte +10h, +14h..+1Ch, byte +20h, +24h..+30h, byte +34h, +38h.
void* turn_path_assign_0080c830(Seg49Host& host, Raw self, Raw source)
{
    segment_array_assign_0080c750(host, self.at<RawArray>(0x0), source.at<RawArray>(0x0));
    self.at<std::uint32_t>(0x0C) = source.at<std::uint32_t>(0x0C);
    self.at<std::uint8_t>(0x10) = source.at<std::uint8_t>(0x10);
    self.at<std::uint32_t>(0x14) = source.at<std::uint32_t>(0x14);
    self.at<std::uint32_t>(0x18) = source.at<std::uint32_t>(0x18);
    self.at<std::uint32_t>(0x1C) = source.at<std::uint32_t>(0x1C);
    self.at<std::uint8_t>(0x20) = source.at<std::uint8_t>(0x20);
    self.at<std::uint32_t>(0x24) = source.at<std::uint32_t>(0x24);
    self.at<std::uint32_t>(0x28) = source.at<std::uint32_t>(0x28);
    self.at<std::uint32_t>(0x2C) = source.at<std::uint32_t>(0x2C);
    self.at<std::uint32_t>(0x30) = source.at<std::uint32_t>(0x30);
    self.at<std::uint8_t>(0x34) = source.at<std::uint8_t>(0x34);
    self.at<std::uint32_t>(0x38) = source.at<std::uint32_t>(0x38);
    return self.p;
}

// 0080CF60, ABI: __fastcall(ECX = owner of a heap path) RET. When set: clear the path's segment
// array, free its data (00BF6989), free the path (00BF65AC) and null the owner.
void turn_path_owner_release_0080cf60(Seg49Host& host, void*& path)
{
    if (path == nullptr) {
        return;
    }
    auto& segments = *static_cast<RawArray*>(path);
    clear_segments(host, segments);
    host.free_00bf6989(segments.data);
    host.free_00bf65ac(path);
    path = nullptr;
}

namespace {

// operator new(3Ch) path of 0080C8C0 / 0080CA60: the words +0h..+8h and +14h..+38h zeroed when
// allocated (+0Ch and +10h are not); a failed allocation is used anyway, as the image.
Raw new_turn_path(Seg49Host& host)
{
    auto* block = static_cast<std::uint8_t*>(host.operator_new_00bf681b(0x3C));
    if (block != nullptr) {
        Raw p{block};
        p.at<std::uint32_t>(0x0) = 0;
        p.at<std::uint32_t>(0x4) = 0;
        p.at<std::uint32_t>(0x8) = 0;
        for (std::size_t off = 0x14; off <= 0x38; off += 4) {
            p.at<std::uint32_t>(off) = 0;
        }
    }
    return Raw{block};
}

// The segment push both candidate builders inline: grow to max(2 * capacity, 1) (0080BC20) when
// full, copy (0080B890) unless the slot address is null, count + 1.
void push_segment(Seg49Host& host, Raw path, Raw segment)
{
    auto& segments = path.at<RawArray>(0x0);
    if (segments.count == segments.capacity) {
        int capacity = segments.capacity * 2;
        if (capacity <= 1) {
            capacity = 1;
        }
        host.array44_reserve_0080bc20(&segments, capacity);
    }
    std::uint8_t* slot = segments.data + segments.count * 0x44;
    if (slot != nullptr) {
        turn_path_segment_copy_0080b890(Raw{slot}, segment);
    }
    ++segments.count;
}

}  // namespace

// 0080C8C0, ABI: __fastcall(ECX = circle {x, z, r, direction byte +0Ch}, EDX = path holder)
// RET. Called twice by 0080CFB0. When 0080B530 accepts the circle (its angle out-parameter is
// the end angle), one arc segment {kind 2, +4h 0, centre (x, goal height [00F87518], z), r,
// start = the current heading [00F87510] turned by pi/2 (00438AA0 add for a clear direction,
// 00438B10 subtract for a set one), end = that angle, direction} goes into a new path, the
// climb is applied (0080B690) and the path pointer is pushed onto the holder (0080BCB0). The
// segment's other fields are uninitialised stack in the image (zero here).
void turn_path_add_circle_to_point_candidate_0080c8c0(Seg49Host& host, Raw circle, Raw path_holder)
{
    float end_angle = 0.0f;
    if (!turn_path_circle_reaches_goal_0080b530(host, circle, &end_angle)) {
        return;
    }
    Raw state = host.global_object(kTurnPathState);
    std::uint8_t bytes[0x44] = {};
    Raw segment{bytes};
    segment.at<int>(0x0) = 2;
    segment.at<int>(0x4) = 0;
    const float heading = state.at<float>(0x10);
    const float start = circle.at<std::uint8_t>(0xC) != 0 ? host.wrapped_angle_subtract_00438b10(heading, kTurnPathHalfPi)
                                                          : host.wrapped_angle_add_00438aa0(heading, kTurnPathHalfPi);
    segment.at<float>(0x34) = end_angle;
    segment.at<float>(0x30) = start;
    segment.at<float>(0x20) = circle.at<float>(0x0);
    segment.at<float>(0x24) = state.at<float>(0x18);
    segment.at<std::uint8_t>(0x38) = circle.at<std::uint8_t>(0xC);
    segment.at<float>(0x28) = circle.at<float>(0x4);
    segment.at<float>(0x2C) = circle.at<float>(0x8);
    Raw path = new_turn_path(host);
    push_segment(host, path, segment);
    turn_path_apply_climb_0080b690(host, path);
    const auto pointer = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(path.p));
    aligned_buffer_push_0080bcb0(host, path_holder, &pointer);
}

// 0080CA60, ABI: __thiscall(ECX = path holder, x1, z1, r1, d1, x2, z2, r2, d2) RET 20h. Called
// four times by 0080CFB0. The tangent candidates of the two circles go into a local list
// (0080C060); each becomes a new three-segment path: an arc on circle 1 (centre height 0) from
// the current heading [00F87510] turned by pi/2 (00438B10 for d1 set, else 00438AA0) to the
// candidate's first angle; a line from the first to the second tangent point (heights 0); an arc
// on circle 2 from the candidate's second angle to the goal heading [00F87524] turned by pi/2
// (by d2). 0080B690 applies the climb and the path is pushed onto the holder (0080BCB0). The
// local list is cleared at the end (0080BD40). The segment fields a kind does not use are copied
// from three stack segments the image never initialises (persistent zeroed buffers here).
void turn_path_add_circle_to_circle_candidates_0080ca60(Seg49Host& host, Raw path_holder, TurnCircle first,
                                                         TurnCircle second)
{
    ReconList candidates{0, nullptr, nullptr};
    turn_path_circle_to_circle_candidates_0080c060(host, candidates, first, second);
    std::uint8_t first_arc[0x44] = {};
    std::uint8_t line[0x44] = {};
    std::uint8_t last_arc[0x44] = {};
    Raw state = host.global_object(kTurnPathState);
    for (ReconNode* node = candidates.head; node != nullptr; node = node->next) {
        Raw candidate{reinterpret_cast<std::uint8_t*>(node)};
        const float start = first.direction != 0
                                ? host.wrapped_angle_subtract_00438b10(state.at<float>(0x10), kTurnPathHalfPi)
                                : host.wrapped_angle_add_00438aa0(state.at<float>(0x10), kTurnPathHalfPi);
        Raw a{first_arc};
        a.at<float>(0x30) = start;
        a.at<float>(0x34) = candidate.at<float>(0x10);
        const float p1x = candidate.at<float>(0x08);
        const float p1z = candidate.at<float>(0x0C);
        const float p2x = candidate.at<float>(0x14);
        const float p2z = candidate.at<float>(0x18);
        Raw c{last_arc};
        c.at<float>(0x30) = candidate.at<float>(0x1C);
        c.at<float>(0x34) = second.direction != 0
                                ? host.wrapped_angle_subtract_00438b10(state.at<float>(0x24), kTurnPathHalfPi)
                                : host.wrapped_angle_add_00438aa0(state.at<float>(0x24), kTurnPathHalfPi);
        Raw path = new_turn_path(host);
        // Arc on circle 1.
        a.at<int>(0x00) = 2;
        a.at<int>(0x04) = 0;
        a.at<float>(0x20) = first.x;
        a.at<float>(0x24) = 0.0f;
        a.at<float>(0x28) = first.z;
        a.at<float>(0x2C) = first.radius;
        a.at<std::uint8_t>(0x38) = first.direction;
        push_segment(host, path, a);
        // Line between the tangent points.
        Raw l{line};
        l.at<int>(0x00) = 1;
        l.at<int>(0x04) = 0;
        l.at<float>(0x08) = p1x;
        l.at<float>(0x0C) = 0.0f;
        l.at<float>(0x10) = p1z;
        l.at<float>(0x14) = p2x;
        l.at<float>(0x18) = 0.0f;
        l.at<float>(0x1C) = p2z;
        push_segment(host, path, l);
        // Arc on circle 2.
        c.at<int>(0x00) = 2;
        c.at<int>(0x04) = 0;
        c.at<float>(0x20) = second.x;
        c.at<float>(0x24) = 0.0f;
        c.at<float>(0x28) = second.z;
        c.at<float>(0x2C) = second.radius;
        c.at<std::uint8_t>(0x38) = second.direction;
        push_segment(host, path, c);
        turn_path_apply_climb_0080b690(host, path);
        const auto pointer = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(path.p));
        aligned_buffer_push_0080bcb0(host, path_holder, &pointer);
    }
    host.dlist_clear_0080bd40(&candidates);
}

namespace {

// One turn circle centre of 0080CFB0: angle = float(heading -/+ pi/2 as double), centre =
// float(r * float(cos angle) + x), float(r * float(sin angle) + z).
void turn_circle_centre(double heading, double offset, float radius, float x, float z, float& cx, float& cz)
{
    const auto angle = static_cast<float>(heading + offset);
    const auto c = static_cast<float>(std::cos(static_cast<double>(angle)));
    cx = static_cast<float>(static_cast<double>(radius) * c + x);
    const auto s = static_cast<float>(std::sin(static_cast<double>(angle)));
    cz = static_cast<float>(static_cast<double>(radius) * s + z);
}

}  // namespace

// 0080CFB0, ABI: __thiscall(ECX = result path, thirteen dwords) RET 34h. No caller: the turn-path
// planner's entry point. The result's segments are cleared (count 0; 0080BC20(0) only for a
// negative capacity), +0Ch = 0.0, byte +10h = 0, and the request is stored into 00F87500..24
// and 00E0B510..18. Start circles left (heading - pi/2, direction 0) and right (heading + pi/2,
// direction 1) each try the straight-to-goal arc (0080C8C0 into a path holder). With the
// heading constraint, goal circles left / right (goal heading -/+ pi/2) feed 0080CA60 four
// times: (right, goal right dir 1), (left, goal right dir 1), (right, goal left dir 0), (left,
// goal left dir 0). The best path prefers one inside the climb limits, then the strictly shorter
// length (+0Ch; first wins ties). Inside the limits it is copied into the result (0080C830);
// otherwise the result gets one kind-0 segment {+4h 1, start, (start x, goal height, start z)}
// and byte +10h = 1. Every candidate path is cleared and freed (00BF6989 data, 00BF65AC path),
// the result's segment end points are visited (0080B1F0, a discarded debug loop that reads the
// first slot even for an empty array), the planner state is copied to result +14h..+38h and
// the holder's block is freed (00BF6989). Fields the image leaves as uninitialised stack in the
// fallback segment are zero here.
void turn_path_plan_0080cfb0(Seg49Host& host, Raw result, const TurnPathRequest& request)
{
    auto& segments = result.at<RawArray>(0x0);
    if (segments.capacity < 0) {
        host.array44_reserve_0080bc20(&segments, 0);
    }
    while (segments.count > 0) {
        --segments.count;
    }
    result.at<float>(0x0C) = 0.0f;
    segments.count = 0;
    result.at<std::uint8_t>(0x10) = 0;
    Raw state = host.global_object(kTurnPathState);
    state.at<float>(0x00) = request.start[0];
    state.at<float>(0x04) = request.start[1];
    state.at<float>(0x08) = request.start[2];
    state.at<float>(0x10) = request.heading;
    state.at<float>(0x14) = request.goal[0];
    state.at<float>(0x18) = request.goal[1];
    state.at<float>(0x1C) = request.goal[2];
    state.at<std::uint8_t>(0x0C) = static_cast<std::uint8_t>(request.start_flag);
    state.at<std::uint8_t>(0x20) = static_cast<std::uint8_t>(request.goal_flag);
    state.at<float>(0x24) = request.goal_heading;
    Raw limits = host.global_object(0x00E0B510);
    limits.at<float>(0x0) = request.radius;
    limits.at<float>(0x4) = request.max_climb;
    limits.at<float>(0x8) = request.min_climb;

    std::uint8_t holder_bytes[4] = {};
    Raw holder{holder_bytes};
    // Start circles.
    float left_x = 0.0f;
    float left_z = 0.0f;
    turn_circle_centre(request.heading, -kTurnPathHalfPiDouble, request.radius, request.start[0], request.start[2],
                       left_x, left_z);
    const float radius_global = limits.at<float>(0x0);
    float right_x = 0.0f;
    float right_z = 0.0f;
    turn_circle_centre(state.at<float>(0x10), kTurnPathHalfPiDouble, radius_global, request.start[0],
                       request.start[2], right_x, right_z);
    std::uint8_t left_bytes[0x10] = {};
    Raw left{left_bytes};
    left.at<float>(0x0) = left_x;
    left.at<float>(0x4) = left_z;
    left.at<float>(0x8) = request.radius;
    left.at<std::uint8_t>(0xC) = 0;
    std::uint8_t right_bytes[0x10] = {};
    Raw right{right_bytes};
    right.at<float>(0x0) = right_x;
    right.at<float>(0x4) = right_z;
    right.at<float>(0x8) = radius_global;
    right.at<std::uint8_t>(0xC) = 1;
    turn_path_add_circle_to_point_candidate_0080c8c0(host, left, holder);
    turn_path_add_circle_to_point_candidate_0080c8c0(host, right, holder);
    if (state.at<std::uint8_t>(0x20) != 0) {
        const float gx = state.at<float>(0x14);
        const float gz = state.at<float>(0x1C);
        float goal_left_x = 0.0f;
        float goal_left_z = 0.0f;
        turn_circle_centre(state.at<float>(0x24), -kTurnPathHalfPiDouble, limits.at<float>(0x0), gx, gz, goal_left_x,
                           goal_left_z);
        float goal_right_x = 0.0f;
        float goal_right_z = 0.0f;
        turn_circle_centre(state.at<float>(0x24), kTurnPathHalfPiDouble, limits.at<float>(0x0), gx, gz, goal_right_x,
                           goal_right_z);
        const float r = limits.at<float>(0x0);
        const TurnCircle start_right{right_x, right_z, radius_global, 1};
        const TurnCircle start_left{left_x, left_z, request.radius, 0};
        turn_path_add_circle_to_circle_candidates_0080ca60(host, holder, start_right, {goal_right_x, goal_right_z, r, 1});
        turn_path_add_circle_to_circle_candidates_0080ca60(host, holder, start_left, {goal_right_x, goal_right_z, r, 1});
        turn_path_add_circle_to_circle_candidates_0080ca60(host, holder, start_right, {goal_left_x, goal_left_z, r, 0});
        turn_path_add_circle_to_circle_candidates_0080ca60(host, holder, start_left, {goal_left_x, goal_left_z, r, 0});
    }
    auto* header = static_cast<std::uint8_t*>(holder.ptr(0x0));
    if (header != nullptr) {
        auto** paths = static_cast<void**>(Raw{header}.ptr(0x0));
        const int count = Raw{header}.at<int>(0x4);
        Raw best{nullptr};
        std::uint8_t best_flag = 1;
        for (int i = 0; i < count; ++i) {
            Raw p{static_cast<std::uint8_t*>(paths[i])};
            bool take = false;
            if (best.p == nullptr) {
                take = true;
            } else {
                const std::uint8_t pf = p.at<std::uint8_t>(0x10);
                const std::uint8_t bf = best.at<std::uint8_t>(0x10);
                if (pf == 0 && bf != 0) {
                    take = true;
                } else if (pf == bf || (pf != 0 && bf != 0)) {
                    take = best.at<float>(0x0C) > p.at<float>(0x0C);
                }
            }
            if (take) {
                best = p;
                best_flag = p.at<std::uint8_t>(0x10);
            }
        }
        if (best.p != nullptr) {
            if (best_flag != 0) {
                std::uint8_t bytes[0x44] = {};
                Raw climb{bytes};
                climb.at<int>(0x00) = 0;
                climb.at<int>(0x04) = 1;
                climb.at<float>(0x08) = state.at<float>(0x00);
                climb.at<float>(0x0C) = state.at<float>(0x04);
                climb.at<float>(0x10) = state.at<float>(0x08);
                climb.at<float>(0x14) = state.at<float>(0x00);
                climb.at<float>(0x18) = state.at<float>(0x18);
                climb.at<float>(0x1C) = state.at<float>(0x08);
                push_segment(host, result, climb);
                result.at<std::uint8_t>(0x10) = 1;
            } else {
                turn_path_assign_0080c830(host, result, best);
            }
        }
        for (int i = 0; i < Raw{header}.at<int>(0x4); ++i) {
            void*& slot = static_cast<void**>(Raw{header}.ptr(0x0))[i];
            if (slot == nullptr) {
                continue;
            }
            auto& path_segments = *static_cast<RawArray*>(slot);
            clear_segments(host, path_segments);
            host.free_00bf6989(path_segments.data);
            host.free_00bf65ac(slot);
            slot = nullptr;
        }
    }
    float point[3];
    turn_path_segment_end_point_0080b1f0(host, Raw{segments.data}, point);
    for (int i = 1; i < segments.count; ++i) {
        turn_path_segment_end_point_0080b1f0(host, Raw{segments.data + i * 0x44}, point);
    }
    result.at<float>(0x14) = state.at<float>(0x00);
    result.at<float>(0x18) = state.at<float>(0x04);
    result.at<float>(0x1C) = state.at<float>(0x08);
    result.at<std::uint8_t>(0x20) = state.at<std::uint8_t>(0x0C);
    result.at<float>(0x24) = state.at<float>(0x10);
    result.at<float>(0x28) = state.at<float>(0x14);
    result.at<float>(0x2C) = state.at<float>(0x18);
    result.at<float>(0x30) = state.at<float>(0x1C);
    result.at<std::uint8_t>(0x34) = state.at<std::uint8_t>(0x20);
    result.at<float>(0x38) = state.at<float>(0x24);
    host.free_00bf6989(holder.ptr(0x0));
}

// 0080D690, ABI: __fastcall(ECX = entity) RET -> EAX. Sole caller BSP_UnitInstance_SEntityInit.
// 00925CE0, 007B28C0(ECX = +170h, entity), the four EntityPath vtables, class id +C4h = 48h.
void* entity_path_construct_0080d690(Seg49Host& host, Raw entity)
{
    host.scene_node_construct_00925ce0(entity.p);
    host.path_component_construct_007b28c0(entity.p + 0x170, entity.p);
    entity.at<std::uint32_t>(0x170) = kEntityPathVtable170;
    entity.at<std::uint32_t>(0x000) = kEntityPathVtable;
    entity.at<std::uint32_t>(0x010) = kEntityPathVtable10;
    entity.at<std::uint32_t>(0x024) = kEntityPathVtable24;
    entity.at<int>(0xC4) = kEntityPathClassId;
    return entity.p;
}

// 0080D700, ABI: __fastcall(ECX = entity) RET. From 0080F9D0: the four vtables, 007B1BD0 on
// +170h, then 00925780.
void entity_path_destruct_0080d700(Seg49Host& host, Raw entity)
{
    entity.at<std::uint32_t>(0x000) = kEntityPathVtable;
    entity.at<std::uint32_t>(0x010) = kEntityPathVtable10;
    entity.at<std::uint32_t>(0x024) = kEntityPathVtable24;
    entity.at<std::uint32_t>(0x170) = kEntityPathVtable170;
    host.path_component_destruct_007b1bd0(entity.p + 0x170);
    host.entity_destruct_base_00925780(entity.p);
}

// 0080D770, ABI: RET -> EAX. Vtable 00D092B8 slot +28h: the class name.
const char* entity_path_class_name_0080d770() noexcept
{
    return kEntityPathClassName;
}

namespace {

inline constexpr float kOne_00d7a24c = 1.0f;  // 00D7A24C

// The shared tail of 00804350 / 00804620 (00804469..008045FF and 008046E7..0080487C): every
// cell of the 7x7 sensor table [class+B4h] (row i, column j at +8h + i*60h + j*0Ch), whatever
// the classes of the two units. Row 4 divides by the squared [game+21C4h]+78h, the others by
// 1.0. The cell quotient is held as a double (FSTP double at 0080453E); an entry admits when
// the quotient is not above its +4h (JA: unordered admits), its +10h bit is in mask, and,
// when the arc byte +18h is set, |00438B10(atan2(dx, dz), heading)| is not above +14h.
bool sensor_table_admits(Seg49Host& host, Raw unit, Raw cls, float dx, float dz, float denominator,
                         std::uint32_t mask)
{
    // 00804497..008044CB: dz*dz and dx*dx each stored as float, the sum stored as float.
    const float dz2 = dz * dz;
    const float dx2 = dx * dx;
    const float sum = static_cast<float>(static_cast<double>(dz2) + dx2);
    const float ratio = static_cast<float>(static_cast<double>(sum) / denominator);
    for (int i = 0; i < 7; ++i) {
        for (int j = 0; j < 7; ++j) {
            Raw row{static_cast<std::uint8_t*>(cls.ptr(0xB4)) + 8 + i * 0x60 + j * 0xC};
            float divisor = kOne_00d7a24c;
            if (i == 4) {
                const float weather_78 = Raw{static_cast<std::uint8_t*>(host.game_e188a8().ptr(0x21C4))}.at<float>(0x78);
                divisor = weather_78 * weather_78;
            }
            auto* entry = static_cast<std::uint8_t*>(row.ptr(0x0));
            if (entry == entry + row.at<int>(0x4) * 0x1C) {
                continue;
            }
            const double quotient = static_cast<double>(ratio) / static_cast<double>(divisor);
            for (; entry != static_cast<std::uint8_t*>(row.ptr(0x0)) + row.at<int>(0x4) * 0x1C; entry += 0x1C) {
                Raw e{entry};
                if (quotient > static_cast<double>(e.at<float>(0x4))) {
                    continue;
                }
                if ((mask & (1u << (e.at<std::uint32_t>(0x10) & 31u))) == 0) {
                    continue;
                }
                if (e.at<std::uint8_t>(0x18) == 0) {
                    return true;
                }
                const float heading = host.unit_heading_50(unit.p);
                const float bearing = static_cast<float>(host.ci_atan2_00bf701a(dx, dz));
                const float off = host.wrapped_angle_subtract_00438b10(bearing, heading);
                const float limit = e.at<float>(0x14);
                if (!(std::fabs(off) > limit)) {  // JBE: unordered returns true
                    return true;
                }
            }
        }
    }
    return false;
}

}  // namespace

// 00804350, ABI: __fastcall(ECX = unit, EDX = target, mask) RET 4 -> AL. HUD calls it as
// (target, 5). The target's signature is [target+538h]+B8h when target vtable +5Ch(5) holds,
// else 1.0; the modifier is 008E6430(0Ch, unit) when 00E0C978 and [00F88C30]+118h are set,
// else 1.0. Denominator (00804400..00804430): float(float(m*m) * sig) * float(vis*vis), vis =
// [game+21C4h]+74h. Poses are refreshed (00414DB0) when +C8h is clear, target first.
bool unit_sensors_detect_unit_00804350(Seg49Host& host, Raw unit, Raw target, std::uint32_t mask)
{
    Raw cls{static_cast<std::uint8_t*>(unit.ptr(0x538))};
    if (cls.ptr(0xB4) == nullptr) {
        return false;
    }
    float signature = kOne_00d7a24c;
    if (host.is_kind_of_5c(target.p, 5)) {
        signature = Raw{static_cast<std::uint8_t*>(target.ptr(0x538))}.at<float>(0xB8);
    }
    float modifier = kOne_00d7a24c;
    if (host.powerups_flag_e0c978() && host.powerup_manager_f88c30().at<int>(0x118) != 0) {
        modifier = host.gameplay_modifiers_product_for_unit_008e6430(0xC, unit.p);
    }
    const float vis = Raw{static_cast<std::uint8_t*>(host.game_e188a8().ptr(0x21C4))}.at<float>(0x74);
    const float m2 = modifier * modifier;
    const float m2s = m2 * signature;
    const float vis2 = vis * vis;
    const float denominator = m2s * vis2;
    if (target.at<std::uint8_t>(0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(target.p);
    }
    const float tx = target.at<float>(0xFC);
    const float tz = target.at<float>(0x104);
    if (unit.at<std::uint8_t>(0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit.p);
    }
    const float dx = static_cast<float>(static_cast<double>(tx) - unit.at<float>(0xFC));
    const float dz = static_cast<float>(static_cast<double>(tz) - unit.at<float>(0x104));
    return sensor_table_admits(host, unit, cls, dx, dz, denominator, mask);
}

// 00804620, ABI: __fastcall(ECX = unit, EDX = float* point {x, y, z}, mask) RET 4 -> AL. Sole
// caller BSP_InGameHudMarkers_UpdateTargetSection. 00804350 without the target signature:
// denominator float(float(m*m) * float(vis*vis)); point x and z are read before the unit's
// pose refresh.
bool unit_sensors_detect_point_00804620(Seg49Host& host, Raw unit, const float* point, std::uint32_t mask)
{
    Raw cls{static_cast<std::uint8_t*>(unit.ptr(0x538))};
    if (cls.ptr(0xB4) == nullptr) {
        return false;
    }
    float modifier = kOne_00d7a24c;
    if (host.powerups_flag_e0c978() && host.powerup_manager_f88c30().at<int>(0x118) != 0) {
        modifier = host.gameplay_modifiers_product_for_unit_008e6430(0xC, unit.p);
    }
    const float vis = Raw{static_cast<std::uint8_t*>(host.game_e188a8().ptr(0x21C4))}.at<float>(0x74);
    const float px = point[0];
    const float pz = point[2];
    const float m2 = modifier * modifier;
    const float vis2 = vis * vis;
    const float denominator = m2 * vis2;
    if (unit.at<std::uint8_t>(0xC8) == 0) {
        host.entity_pose_refresh_world_00414db0(unit.p);
    }
    const float dx = static_cast<float>(static_cast<double>(px) - unit.at<float>(0xFC));
    const float dz = static_cast<float>(static_cast<double>(pz) - unit.at<float>(0x104));
    return sensor_table_admits(host, unit, cls, dx, dz, denominator, mask);
}

}  // namespace bsp::cc13_seg49
