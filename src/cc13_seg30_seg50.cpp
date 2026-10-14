// Segment 50 reconstructions (packet cc13-seg30). See include/bsp/cc13_seg30.hpp for the model and
// docs/CC13_SEG30.md for evidence. Names are hypotheses, not recovered symbols.

#include "bsp/cc13_seg30.hpp"

#include <cstring>

namespace bsp::cc13_seg30 {

namespace {

// The POD element move of the doubling arrays: dword copies into a fresh operator-new block (a
// null destination skips the copy), then the old block is freed.
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

}  // namespace

// 0080DDB0, __thiscall(ECX = self), RET, returns self. Base vtable 00CFDB90, the two command target
// descriptors +10h and +28h (00721070), vtable 00CFDB9C.
void* controller_snapshot_construct_0080ddb0(Seg50Host& host, void* self) {
    at<std::uint32_t>(self, 0) = kControllerSnapshotBaseVtable;
    host.command_target_descriptor_init_00721070(add(self, 0x10));
    host.command_target_descriptor_init_00721070(add(self, 0x28));
    at<std::uint32_t>(self, 0) = kControllerSnapshotVtable;
    return self;
}

// 0080DE00, __thiscall(ECX = self), RET, returns self. As 0080DDB0 with vtable 00CFDBA8, byte +47h
// = 0 and floats +48h / +4Ch = 0.0.
void* controller_snapshot_ex_construct_0080de00(Seg50Host& host, void* self) {
    at<std::uint32_t>(self, 0) = kControllerSnapshotBaseVtable;
    host.command_target_descriptor_init_00721070(add(self, 0x10));
    host.command_target_descriptor_init_00721070(add(self, 0x28));
    at<std::uint32_t>(self, 0) = kControllerSnapshotExVtable;
    at<std::uint8_t>(self, 0x47) = 0;
    at<float>(self, 0x48) = 0.0f;
    at<float>(self, 0x4C) = 0.0f;
    return self;
}

// 0080E240, __stdcall(ship) (ECX unused), RET 4. Null for a null ship. Otherwise the ship's +0Ch
// record, created on first use: operator new(1Ch) with +0Fh = 1, +14h = 0, +10h = 0, +19h = 1,
// +18h = 1 (the other bytes are left as allocated); a failed allocation stores and returns null.
void* ship_sync_state_get_or_create_0080e240(Seg50Host& host, void* ship) {
    if (ship == nullptr) return nullptr;
    if (ptr(ship, 0x0C) == nullptr) {
        void* const state = host.operator_new_00bf681b(0x1C);
        if (state != nullptr) {
            at<std::uint8_t>(state, 0x0F) = 1;
            at<std::uint32_t>(state, 0x14) = 0;
            at<std::uint32_t>(state, 0x10) = 0;
            at<std::uint8_t>(state, 0x19) = 1;
            at<std::uint8_t>(state, 0x18) = 1;
            at<void*>(ship, 0x0C) = state;
            return state;
        }
        at<void*>(ship, 0x0C) = nullptr;
    }
    return ptr(ship, 0x0C);
}

// 0080E2C0, __thiscall(ECX = message), RET, returns the message. SHIP_CREATE for reading: base
// 00766F60, vtable 00D09410, a 50h-byte controller snapshot (the 0080DE00 body inline, null when the
// allocation fails) at +110h, vtable 00CFF354 at +114h, +120h = -1, +124h = 0.
void* ship_create_message_construct_for_read_0080e2c0(Seg50Host& host, void* message) {
    host.net_entity_create_message_construct_00766f60(message);
    at<std::uint32_t>(message, 0) = kShipCreateMessageVtable;
    void* pair = host.operator_new_00bf681b(0x50);
    if (pair != nullptr) pair = controller_snapshot_ex_construct_0080de00(host, pair);
    at<void*>(message, 0x110) = pair;
    at<std::uint32_t>(message, 0x114) = kShipCreateMessage114Vtable;
    at<std::int32_t>(message, 0x120) = -1;
    at<std::uint32_t>(message, 0x124) = 0;
    return message;
}

// 0080E380, __thiscall(ECX = message), RET; tail-jumps to 007668E0. vtable 00D09410, the +110h snapshot
// freed with 00BF65AC (no destructor runs, null passed through), vtable 00CFF354 at +114h.
void ship_create_message_destruct_0080e380(Seg50Host& host, void* message) {
    void* const pair = ptr(message, 0x110);
    at<std::uint32_t>(message, 0) = kShipCreateMessageVtable;
    host.free_00bf65ac(pair);
    at<std::uint32_t>(message, 0x114) = kShipCreateMessage114Vtable;
    host.net_entity_create_message_destruct_007668e0(message);
}

// 0080EE90, __thiscall(ECX = holder), RET. free(*holder) when non-null (00BF6989), not cleared.
void buffer_free_0080ee90(Seg50Host& host, void* holder) {
    if (ptr(holder, 0) != nullptr) host.free_00bf6989(ptr(holder, 0));
}

// 0080EF70, __thiscall(ECX = holder), RET. free(*holder) through 00BF65AC and clear it.
void buffer_free_and_null_0080ef70(Seg50Host& host, void* holder) {
    if (ptr(holder, 0) != nullptr) {
        host.free_00bf65ac(ptr(holder, 0));
        at<void*>(holder, 0) = nullptr;
    }
}

// 0080EF90, __thiscall(ECX = holder), RET. The held object's vtable +10h (no arguments), then clear.
void object_ptr_release_virtual10_0080ef90(Seg50Host& host, void* holder) {
    if (ptr(holder, 0) != nullptr) {
        host.object_virtual_10(ptr(holder, 0));
        at<void*>(holder, 0) = nullptr;
    }
}

// 0080F0F0, __thiscall(ECX = holder), RET. 00B6DFA0 on the held node, then clear.
void node_ptr_unlink_and_release_0080f0f0(Seg50Host& host, void* holder) {
    if (ptr(holder, 0) != nullptr) {
        host.node_unlink_and_release_00b6dfa0(ptr(holder, 0));
        at<void*>(holder, 0) = nullptr;
    }
}

// 0080F2F0, __fastcall(ECX = vector {data, size, capacity}), RET. Vec3 (0Ch) elements,
// capacity = 2 * capacity + 2. The image copies through x87 FLD/FSTP float pairs, which equal the
// dword copies here for every value but a signalling NaN.
void vec3_array_grow_0080f2f0(Seg50Host& host, void* vector) {
    const std::uint32_t capacity = at<std::uint32_t>(vector, 8) * 2 + 2;
    at<std::uint32_t>(vector, 8) = capacity;
    move_pod_vector(host, vector, capacity, 0x0C);
}

// 0080FD90, __thiscall(ECX = message, const dword payload[8]), RET 4, returns the message.
// SHIP_HELMSMAN_CONTROL: 0075B430(8Eh), +18h word / +1Ah byte / +4h = 0, vtable 00D02DA8, the
// 20h-byte payload copied (REP MOVSD) to +1Ch.
void* ship_helmsman_control_message_construct_0080fd90(Seg50Host& host, void* message, const std::uint32_t* payload) {
    host.session_message_construct_base_0075b430(message, 0x8E);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 0;
    at<std::uint32_t>(message, 0) = kHelmsmanControlMessageVtable;
    for (std::size_t i = 0; i < 8; ++i) at<std::uint32_t>(message, 0x1C + i * 4) = payload[i];
    return message;
}

// 0080FDD0, __thiscall(ECX = message, float value), RET 4, returns the message.
// SHIP_LANDINGSHIPS_LAUNCHED: 0075B430(95h), +18h word / +1Ah byte = 0, +4h = 1, vtable 00D03388,
// float +1Ch = value.
void* ship_landing_ships_launched_message_construct_0080fdd0(Seg50Host& host, void* message, float value) {
    host.session_message_construct_base_0075b430(message, 0x95);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint32_t>(message, 0x04) = 1;
    at<std::uint32_t>(message, 0) = kLandingShipsLaunchedMessageVtable;
    at<float>(message, 0x1C) = value;
    return message;
}

// 00811820, __thiscall(ECX = unit), RET, returns ST0. The local x of the point {+FA0h, 0, +FA8h}
// after the unit's inverse world matrix +110h (004142E0). The inverse is rebuilt when +10Ch is
// clear: 00414DB0 on the unit, +10Ch = 1, then 00B63D50(ECX = +110h, EDX = +CCh).
float unit_local_x_of_fa0_point_00811820(Seg50Host& host, void* unit) {
    const float point[3] = {at<float>(unit, 0xFA0), 0.0f, at<float>(unit, 0xFA8)};
    if (at<std::uint8_t>(unit, 0x10C) == 0) {
        host.entity_pose_refresh_world_00414db0(unit);
        at<std::uint8_t>(unit, 0x10C) = 1;
        host.matrix_build_affine_inverse_00b63d50(add(unit, 0x110), add(unit, 0xCC));
    }
    float local[3];
    host.vector3_transform_affine_point_004142e0(point, local, add(unit, 0x110));
    return local[0];
}

// 008129E0, __stdcall(const target*) (ECX unused), RET 4, returns AL. True when target kind +0h
// is 1, its entity +4h is IsKindOf(29h) and [[entity+174h]+8h] is 4, 5, 6, 7, 9 or 0Ah.
bool target_is_kind29_with_class_in_set_008129e0(Seg50Host& host, const void* target) {
    auto* const t = const_cast<void*>(target);
    if (at<std::int32_t>(t, 0) != 1) return false;
    void* const entity = ptr(t, 4);
    if (!host.is_kind_of_5c(entity, 0x29)) return false;
    const std::int32_t kind = at<std::int32_t>(ptr(entity, 0x174), 8);
    return kind == 4 || kind == 5 || kind == 6 || kind == 7 || kind == 9 || kind == 0xA;
}

// 008138A0, __thiscall(ECX = ship, record), RET 4. No direct xref (a sync slot). Calls the cdecl
// 0077B280(ship, record, F, 0.0f, 1, 600.0f, 3, 1400.0f, 4, 4000.0f, 5, FLT_MAX, 6) where F is the
// stack byte (+BCCh > 0.5f, SSE COMISS); the image pushes that slot as a dword whose upper three
// bytes are never written (passed as zero here).
void ship_build_sync_delta_008138a0(Seg50Host& host, void* ship, void* record) {
    const std::uint32_t flag = at<float>(ship, 0xBCC) > kSyncSpeedThreshold ? 1u : 0u;
    host.replication_build_sync_delta_0077b280(ship, record, flag, 0.0f, 1, kSyncRange3, 3, kSyncRange4, 4,
                                               kSyncRange5, 5, kSyncRange6, 6);
}

// 00813A20, __thiscall(ECX = message, a1, ship, a3), RET 0Ch, returns the message. SHIP_CREATE for
// sending: 00766FC0(a3, a1, ship, 1), vtable 00D09410; +110h = a 50h-byte controller snapshot
// built by 007214C0 from [ship+738h] (null when the allocation fails); vtable 00CFF354 at +114h;
// +11Ch = ([ship+1130h] != 0); +120h = [ship+6B8h]; +124h = ship vtable +204h(); +128h =
// [ship+115Ch]; 00809290(ECX = +114h, ship+72Ch) stores the carrier part.
void* ship_create_message_construct_00813a20(Seg50Host& host, void* message, std::uint32_t a1, void* ship,
                                             std::uint32_t a3) {
    host.net_entity_create_message_construct_00766fc0(message, a3, a1, ship, 1);
    at<std::uint32_t>(message, 0) = kShipCreateMessageVtable;
    void* snapshot = host.operator_new_00bf681b(0x50);
    if (snapshot != nullptr) snapshot = host.controller_snapshot_build_007214c0(snapshot, ptr(ship, 0x738));
    at<void*>(message, 0x110) = snapshot;
    at<std::uint32_t>(message, 0x114) = kShipCreateMessage114Vtable;
    at<std::uint8_t>(message, 0x11C) = at<std::uint32_t>(ship, 0x1130) != 0 ? 1 : 0;
    at<std::uint32_t>(message, 0x120) = at<std::uint32_t>(ship, 0x6B8);
    at<std::int32_t>(message, 0x124) = host.unit_virtual_204_int(ship);
    at<std::uint32_t>(message, 0x128) = at<std::uint32_t>(ship, 0x115C);
    host.create_message_store_carrier_type_00809290(add(message, 0x114), add(ship, 0x72C));
    return message;
}

namespace {
// The writers' "abs": a positive value is kept, anything else negated (JG / NEG).
std::uint32_t positive_or_negated(std::int32_t value) {
    return static_cast<std::uint32_t>(value > 0 ? value : -value);
}
}  // namespace

// 00813B10, __thiscall(ECX = message, writer), RET 4. 00765000 base, the snapshot +110h
// (00721550), bool +118h, when set the word +11Ah in 13 bits, bool +11Ch, |+124h| in 2 bits
// (unsigned), +128h in 4 bits (signed), then +120h: a negative value writes bool 0 only, else
// bool 1 and |+120h| in 8 bits.
void ship_create_message_write_00813b10(Seg50Host& host, void* message, void* writer) {
    host.net_entity_create_message_write_00765000(message, writer);
    host.controller_snapshot_write_00721550(ptr(message, 0x110), writer);
    host.bit_write_bool_004290b0(writer, at<std::uint8_t>(message, 0x118));
    if (at<std::uint8_t>(message, 0x118) != 0) {
        host.bit_write_word_00429120(writer, at<std::uint16_t>(message, 0x11A), 13);
    }
    host.bit_write_bool_004290b0(writer, at<std::uint8_t>(message, 0x11C));
    host.bit_write_unsigned_00429070(writer, positive_or_negated(at<std::int32_t>(message, 0x124)), 2);
    host.bit_write_signed_00429090(writer, at<std::uint32_t>(message, 0x128), 4);
    if (at<std::int32_t>(message, 0x120) < 0) {
        host.bit_write_bool_004290b0(writer, 0);
        return;
    }
    host.bit_write_bool_004290b0(writer, 1);
    host.bit_write_unsigned_00429070(writer, positive_or_negated(at<std::int32_t>(message, 0x120)), 8);
}

// 00813BD0, __thiscall(ECX = message, stream), RET 4. 00767E30 base, snapshot read (00721610), then
// through the reader at stream+4h: bool +118h, the word +11Ah (13 bits) when set, bool +11Ch,
// +124h (2 bits unsigned), +128h (4 bits signed), then a bool: set -> +120h = 8 bits, clear ->
// +120h = -1.
void ship_create_message_read_00813bd0(Seg50Host& host, void* message, void* stream) {
    host.net_entity_create_message_read_00767e30(message, stream);
    host.controller_snapshot_read_00721610(ptr(message, 0x110), stream);
    void* const reader = add(stream, 4);
    host.bit_read_bool_00428d70(reader, &at<std::uint8_t>(message, 0x118));
    if (at<std::uint8_t>(message, 0x118) != 0) {
        host.bit_read_word_00428e30(reader, &at<std::uint16_t>(message, 0x11A), 13);
    }
    host.bit_read_bool_00428d70(reader, &at<std::uint8_t>(message, 0x11C));
    std::uint32_t value = 0;
    host.bit_read_unsigned_00428d10(reader, &value, 2);
    at<std::uint32_t>(message, 0x124) = value;
    host.bit_read_signed_00428d30(reader, &value, 4);
    at<std::uint32_t>(message, 0x128) = value;
    std::uint8_t has_peer = 0;
    host.bit_read_bool_00428d70(reader, &has_peer);
    if (has_peer != 0) {
        host.bit_read_unsigned_00428d10(reader, &value, 8);
        at<std::uint32_t>(message, 0x120) = value;
        return;
    }
    at<std::int32_t>(message, 0x120) = -1;
}

namespace {
std::int32_t bits_used(void* cursor) {
    return (at<std::int32_t>(cursor, 8) - at<std::int32_t>(cursor, 0)) * 8 + at<std::int32_t>(cursor, 0x0C);
}
}  // namespace

// 00814000, __thiscall(ECX = message, writer), RET 4. SHIP_GUNS_SYNC: 0075B480 header; +22h = the
// number of guns (the [[message+1Ch]+48h] list linked through +44h answering IsKindOf(22h)),
// written in 8 bits; bools +20h and +21h, copied to +40h / +41h; then each such gun writes its
// snapshot (00859B10, ECX = gun, (message+24h, writer)); with +20h set the walk stops once more than
// E10h (3600) bits were written since the bools.
void ship_guns_sync_message_write_00814000(Seg50Host& host, void* message, void* writer) {
    host.session_message_write_extended_header_0075b480(message, writer);
    at<std::uint8_t>(message, 0x22) = 0;
    for (void* gun = ptr(ptr(message, 0x1C), 0x48); gun != nullptr; gun = ptr(gun, 0x44)) {
        if (host.is_kind_of_5c(gun, 0x22)) at<std::uint8_t>(message, 0x22) += 1;
    }
    host.bit_write_byte_00428ff0(writer, at<std::uint8_t>(message, 0x22), 8);
    host.bit_write_bool_004290b0(writer, at<std::uint8_t>(message, 0x20));
    host.bit_write_bool_004290b0(writer, at<std::uint8_t>(message, 0x21));
    const std::int32_t start = bits_used(writer);
    at<std::uint8_t>(message, 0x40) = at<std::uint8_t>(message, 0x20);
    at<std::uint8_t>(message, 0x41) = at<std::uint8_t>(message, 0x21);
    for (void* gun = ptr(ptr(message, 0x1C), 0x48); gun != nullptr; gun = ptr(gun, 0x44)) {
        if (!host.is_kind_of_5c(gun, 0x22)) continue;
        host.turning_gun_write_snapshot_00859b10(gun, add(message, 0x24), writer);
        if (at<std::uint8_t>(message, 0x20) != 0 && bits_used(writer) - start > 0xE10) return;
    }
}

// 008140D0, __thiscall(ECX = message, stream), RET 4. SHIP_GUNS_SYNC read: 0075B4C0 header, then
// through the reader at stream+4h: count +22h (8 bits), bools +20h / +21h; for each of the count
// 44h-byte records at +24h: +1Ch / +1Dh = the two bools, record vtable +18h(stream); once more
// than ED8h (3800) bits were read since the bools, +22h becomes the number read so far and the
// loop ends.
void ship_guns_sync_message_read_008140d0(Seg50Host& host, void* message, void* stream) {
    host.session_message_read_extended_header_0075b4c0(message, stream);
    void* const reader = add(stream, 4);
    host.bit_read_byte_00428c70(reader, &at<std::uint8_t>(message, 0x22), 8);
    host.bit_read_bool_00428d70(reader, &at<std::uint8_t>(message, 0x20));
    host.bit_read_bool_00428d70(reader, &at<std::uint8_t>(message, 0x21));
    const std::int32_t start = bits_used(reader);
    if (at<std::uint8_t>(message, 0x22) == 0) return;
    std::int32_t i = 0;
    void* record = add(message, 0x24);
    do {
        at<std::uint8_t>(record, 0x1C) = at<std::uint8_t>(message, 0x20);
        at<std::uint8_t>(record, 0x1D) = at<std::uint8_t>(message, 0x21);
        host.object_virtual_18_stream(record, stream);
        if (bits_used(reader) - start > 0xED8) {
            at<std::uint8_t>(message, 0x22) = static_cast<std::uint8_t>(i + 1);
            return;
        }
        ++i;
        record = add(record, 0x44);
    } while (i < at<std::uint8_t>(message, 0x22));
}

// 00814F70, __thiscall(ECX = vector, const float vec3*), RET 4. push_back with the 0080F2F0 growth
// inline (x87 float copies, equal to the dword copies here but for a signalling NaN); a null
// destination skips the copy but the size grows.
void vec3_array_push_back_00814f70(Seg50Host& host, void* vector, const float* value) {
    if (at<std::uint32_t>(vector, 4) == at<std::uint32_t>(vector, 8)) vec3_array_grow_0080f2f0(host, vector);
    auto* const dst = static_cast<float*>(add(ptr(vector, 0), at<std::uint32_t>(vector, 4) * 0x0C));
    if (dst != nullptr) {
        dst[0] = value[0];
        dst[1] = value[1];
        dst[2] = value[2];
    }
    at<std::uint32_t>(vector, 4) += 1;
}

namespace {
// The intrusive release: InterlockedDecrement on object+4h, vtable +0 (no arguments) at zero,
// then the holder is cleared.
void release_ref(Seg50Host& host, void* holder) {
    void* const object = ptr(holder, 0);
    if (object == nullptr) return;
    if (host.interlocked_decrement(&at<std::int32_t>(object, 4)) == 0) host.object_virtual_0_noargs(object);
    at<void*>(holder, 0) = nullptr;
}
}  // namespace

// 00815060 / 00815200, __thiscall(ECX = holder), RET, return the holder. Intrusive release.
void* ref_ptr_release_00815060(Seg50Host& host, void* holder) {
    release_ref(host, holder);
    return holder;
}
void* ref_ptr_release_00815200(Seg50Host& host, void* holder) {
    release_ref(host, holder);
    return holder;
}

// 00817BD0 / 0081A590 / 0081A8A0, __thiscall(ECX = holder), RET. The same release, no result.
void ref_ptr_release_00817bd0(Seg50Host& host, void* holder) { release_ref(host, holder); }
void ref_ptr_release_0081a590(Seg50Host& host, void* holder) { release_ref(host, holder); }
void ref_ptr_release_0081a8a0(Seg50Host& host, void* holder) { release_ref(host, holder); }

// 00815EE0, __thiscall(ECX = unit), RET. The two 54h-byte AI order slots: i = +B40h; when the byte
// at unit + B38h - i * 54h is set it is cleared, +B40h = j = 1 - i, and 00811D10 copies
// (ECX = unit + AECh - j * 54h, source = unit + A98h + j * 54h).
void unit_flip_pending_ai_order_slot_00815ee0(Seg50Host& host, void* unit) {
    const std::int32_t i = at<std::int32_t>(unit, 0xB40);
    auto* const base = static_cast<std::uint8_t*>(unit);
    std::uint8_t* const pending = base - i * 0x54 + 0xB38;
    if (*pending == 0) return;
    *pending = 0;
    const std::int32_t j = 1 - at<std::int32_t>(unit, 0xB40);
    at<std::int32_t>(unit, 0xB40) = j;
    host.unit_ai_order_slot_copy_00811d10(base - j * 0x54 + 0xAEC, base + j * 0x54 + 0xA98);
}

// 00816990, __thiscall(ECX = ship, peer), RET 4. Builds a SHIP_CREATE message on the stack
// (00813A20 with a1 = [[ship+538h]+70h], a3 = 8Ah), sends it to the peer (00779FC0, ECX = ship),
// then destroys it inline exactly as 0080E380 does.
void ship_send_create_message_to_peer_00816990(Seg50Host& host, void* ship, std::uint32_t peer) {
    alignas(4) std::uint8_t message[0x12C];
    ship_create_message_construct_00813a20(host, message, at<std::uint32_t>(ptr(ship, 0x538), 0x70), ship, 0x8A);
    host.entity_send_message_to_peer_00779fc0(ship, peer, message);
    ship_create_message_destruct_0080e380(host, message);
}

namespace {
// The float push_back shared by 00817750 and 0081A4F0: full -> 008153E0(max(2 * capacity, 1)).
void float_push_back(Seg50Host& host, void* vector, const float* value) {
    if (at<std::int32_t>(vector, 4) == at<std::int32_t>(vector, 8)) {
        std::int32_t capacity = at<std::int32_t>(vector, 8) * 2;
        if (capacity <= 1) capacity = 1;
        host.float_array_reserve_008153e0(vector, capacity);
    }
    auto* const dst = static_cast<float*>(ptr(vector, 0)) + at<std::int32_t>(vector, 4);
    if (dst != nullptr) *dst = *value;
    at<std::int32_t>(vector, 4) += 1;
}
}  // namespace

// 00817750 / 0081A4F0, __thiscall(ECX = vector, const float*), RET 4. Float push_back; growth to
// max(2 * capacity, 1) through 008153E0.
void float_array_push_back_00817750(Seg50Host& host, void* vector, const float* value) {
    float_push_back(host, vector, value);
}
void float_array_push_back_0081a4f0(Seg50Host& host, void* vector, const float* value) {
    float_push_back(host, vector, value);
}

// 00818290, __thiscall(ECX = task), RET. Releases the +34h reference, then 00875B30 (base).
void ship_explosion_failure_task_destruct_00818290(Seg50Host& host, void* task) {
    release_ref(host, add(task, 0x34));
    host.tick_subnode_base_cleanup_00875b30(task);
}

// 00819840, __thiscall(ECX = unit, matrix), RET 4. 009585B0 sets the world matrix; the pose is
// refreshed when +C8h is clear; the +1018h motion proxy syncs to +CCh (0092D620); 00818EA0 resets
// the pose-dependent state.
void unit_set_world_matrix_and_sync_00819840(Seg50Host& host, void* unit, const void* matrix) {
    host.unit_instance_set_world_matrix_009585b0(unit, matrix);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    host.motion_proxy_sync_transform_0092d620(ptr(unit, 0x1018), add(unit, 0xCC));
    host.unit_reset_pose_dependent_state_00818ea0(unit);
}

// 0081A5C0, __thiscall(ECX = array), RET. A negative capacity +8h first goes through 0074D190(0);
// then the size +4h is counted down to 0 one step at a time and finally stored as 0.
void points_array_clear_0081a5c0(Seg50Host& host, void* array) {
    if (at<std::int32_t>(array, 8) < 0) host.points_array_reserve_capacity_0074d190(array, 0);
    while (at<std::int32_t>(array, 4) > 0) at<std::int32_t>(array, 4) += -1;
    at<std::int32_t>(array, 4) = 0;
}

// 008141E0, __thiscall(ECX = ship, float heading, int player), RET 8. Slot +208h of the class-06
// ship vtables (the 7Bh VEHICLE_UNIT_LAUNCH arm at 00821F12 pushes 0080F710(message) as `player`
// and the float message+20h as `heading`). Counts the catapults (device list +48h, next +44h,
// IsKindOf(28h)) and asks each vtable +1D0h(1); ready = (+638h > 0) and every answer true.
// Nothing happens without a ready catapult. In a session (game+1FE4h != 0) with an effective game
// mode <= 3 and either player >= 0 or game+18ECh >= 0, the player's points (00946F00, ECX = player
// or game+18ECh, stored to float) must not be below the class's +124h integer (class from
// 007B8A80 on [[ship+538h]+C0h]), else return. With one catapult it launches; with several, the one
// whose |00438B10(heading, 006ED000(heading))| is the smallest below 10.0f (00CE38B8) launches.
// Image quirk kept: with several catapults and none below 10.0f, the launch runs with ECX = the
// raw bits of `heading` (EDI is initialised from that stack slot).
void ship_launch_from_catapult_008141e0(Seg50Host& host, void* ship, float heading, std::int32_t player) {
    std::int32_t catapults = 0;
    void* last = nullptr;
    bool ready = at<std::int32_t>(ship, 0x638) > 0;
    void* device = ptr(ship, 0x48);
    if (device == nullptr) return;
    do {
        if (host.is_kind_of_5c(device, 0x28)) {
            ++catapults;
            last = device;
            if (!host.device_virtual_1d0(device, 1)) ready = false;
        }
        device = ptr(device, 0x44);
    } while (device != nullptr);
    if (catapults == 0 || !ready) return;
    void* const game = host.game_e188a8();
    if (at<std::int32_t>(game, kGameSessionMode) != 0 && host.game_effective_mode_004bca50() <= 3) {
        std::int32_t who = player;
        bool gated = player >= 0;
        if (!gated) {
            who = at<std::int32_t>(game, 0x18EC);
            gated = who >= 0;
        }
        if (gated) {
            const auto points = static_cast<float>(host.player_points_00946f00(who));
            void* const vehicle_class =
                host.vehicle_class_get_or_create_00964790(at<std::uint32_t>(ptr(ship, 0x538), 0xC0), true);
            if (static_cast<double>(points) < static_cast<double>(at<std::int32_t>(vehicle_class, 0x124))) return;
        }
    }
    void* chosen = last;
    if (catapults > 1) {
        std::uint32_t heading_bits = 0;
        static_assert(sizeof(heading_bits) == sizeof(heading));
        std::memcpy(&heading_bits, &heading, sizeof heading_bits);
        chosen = reinterpret_cast<void*>(static_cast<std::uintptr_t>(heading_bits));
        float best = kThresholdA;
        for (device = ptr(ship, 0x48); device != nullptr; device = ptr(device, 0x44)) {
            if (!host.is_kind_of_5c(device, 0x28)) continue;
            const float launch = host.catapult_launch_heading_006ed000(device, heading);
            float difference = host.math_subtract_wrapped_angle_00438b10(heading, launch);
            std::uint32_t bits = 0;
            std::memcpy(&bits, &difference, sizeof bits);
            bits &= 0x7FFFFFFFu;
            std::memcpy(&difference, &bits, sizeof bits);
            if (best > difference) {
                best = difference;
                chosen = device;
            }
        }
    }
    host.catapult_queue_launch_006eb680(chosen, heading);
}

namespace {
// The name test of 00815D20: the name header (00B6D800) text +4h is non-null, strstr finds the
// needle and the match offset is not negative.
bool node_name_contains(Seg50Host& host, void* node, const char* needle) {
    void* const header = host.node_get_name_header_00b6d800(node);
    const char* const name = static_cast<const char*>(ptr(header, 4));
    if (name == nullptr) return false;
    const char* const found = host.strstr_00bf9440(name, needle);
    if (found == nullptr) return false;
    return found - static_cast<const char*>(ptr(header, 4)) >= 0;
}
}  // namespace

// 00815D20, __thiscall(ECX = self, node, int hide), RET 8. Recursive (ECX = self passed through).
// A node whose name contains "kotel" (00D0949C) or else "rope" (00D09494) gets
// 00B6DA70((float)hide, 0) and its local position (00B6E0A0) shifted by off = (float)(hide * 200000
// - 100000) on all three axes through its vtable +2Ch(&shifted); then every child (+34h, next +3Ch).
void scene_node_toggle_kotel_rope_nodes_00815d20(Seg50Host& host, void* self, void* node, std::int32_t hide) {
    if (node_name_contains(host, node, kKotelNeedle) || node_name_contains(host, node, kRopeNeedle)) {
        host.scene_node_set_visibility_factor_00b6da70(node, static_cast<float>(hide), 0);
        const auto offset = static_cast<float>(hide * 200000 - 100000);
        float scratch[3];
        const float* const position = host.node_copy_local_position_00b6e0a0(node, scratch);
        const float shifted[3] = {
            static_cast<float>(static_cast<double>(offset) + position[0]),
            static_cast<float>(static_cast<double>(position[1]) + offset),
            static_cast<float>(static_cast<double>(position[2]) + offset)};
        host.node_virtual_2c_set_position(node, shifted);
    }
    for (void* child = ptr(node, 0x34); child != nullptr; child = ptr(child, 0x3C)) {
        scene_node_toggle_kotel_rope_nodes_00815d20(host, self, child, hide);
    }
}

// 008160F0, __thiscall(ECX = unit, float out[3], const float target[3]), RET 8. h = (float)(class
// +A4h + 20.0 (double 00CE3D88)); the two hull-axis points p0 = world({-0.0f - h, 0, 0}) and
// p1 = world({h, 0, 0}) (004142E0 against +CCh, each after a pose refresh when +C8h is clear);
// each point's horizontal distance to the target (dx = target.x - p.x and dz = target.z - p.z
// stored to float, (dx*dx + dz*dz) stored to float, 0 unless above 1e-10, else sqrt to float);
// out = p1 when d1 <= d0, else p0.
void unit_nearer_hull_end_point_008160f0(Seg50Host& host, void* unit, float* out, const float* target) {
    const auto h = static_cast<float>(static_cast<double>(at<float>(ptr(unit, 0x538), 0xA4)) + kHullEndMargin);
    float points[2][3];
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float bow[3] = {kNegativeZero - h, 0.0f, 0.0f};
    host.vector3_transform_affine_point_004142e0(bow, points[0], add(unit, 0xCC));
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float stern[3] = {h, 0.0f, 0.0f};
    host.vector3_transform_affine_point_004142e0(stern, points[1], add(unit, 0xCC));
    const float tx = target[0];
    const float tz = target[2];
    float distance[2];
    for (int i = 0; i < 2; ++i) {
        const float dx = tx - points[i][0];
        const float dz = tz - points[i][2];
        const auto squared =
            static_cast<float>(static_cast<double>(dx) * dx + static_cast<double>(dz) * dz);
        distance[i] = static_cast<double>(squared) > kDistanceEpsilon ? host.crt_sqrt_00bf7030(squared) : 0.0f;
    }
    const int pick = distance[1] <= distance[0] ? 1 : 0;
    out[0] = points[pick][0];
    out[1] = points[pick][1];
    out[2] = points[pick][2];
}

// 00810900, __thiscall(ECX = channel {+0h value, +4h min, +8h max, +0Ch rate, +10h rate limit,
// +14h acceleration}, float target, float dt), RET 8. Nothing for dt <= 0 (SSE COMISS; NaN goes
// on). d = dt, or 0.2f (00CE54A0) when 0.2 (double 00CE3D10, the float 0.2 widened) > dt;
// wanted = (float)((target - value) / d); the rate moves toward wanted by at most
// (float)(rate +- acceleration * dt) (FMUL at 00810950 is D8 CB, FSUBP at 00810970 is DE EA:
// rate - a*dt), is clamped to [-limit, +limit], value += dt * rate (stored to float), and
// 00415690 clamps the value into [min, max] (ECX = &value, EDX = &min, stack = &max).
// x87 keeps the intermediates in extended precision; double is used here (the products are exact,
// a sum can round twice).
void helm_channel_rate_limit_towards_00810900(Seg50Host& host, void* channel, float target, float dt) {
    if (!(0.0f < dt) && !(dt != dt)) return;
    float divisor = dt;
    if (kHelmMinStepWide > static_cast<double>(dt)) divisor = kHelmMinStep;
    const auto wanted =
        static_cast<float>((static_cast<double>(target) - at<float>(channel, 0)) / static_cast<double>(divisor));
    const float rate = at<float>(channel, 0x0C);
    const double step = static_cast<double>(at<float>(channel, 0x14)) * dt;
    float next;
    if (wanted < rate) {
        const auto lower = static_cast<float>(static_cast<double>(rate) - step);
        next = lower > wanted ? lower : wanted;
    } else {
        const auto upper = static_cast<float>(static_cast<double>(rate) + step);
        next = wanted <= upper ? wanted : upper;
    }
    at<float>(channel, 0x0C) = next;
    const float low = -at<float>(channel, 0x10);
    const float current = at<float>(channel, 0x0C);
    float clamped;
    if (low > current) {
        clamped = low;
    } else {
        const float high = at<float>(channel, 0x10);
        clamped = current <= high ? current : high;
    }
    at<float>(channel, 0x0C) = clamped;
    at<float>(channel, 0) =
        static_cast<float>(static_cast<double>(dt) * at<float>(channel, 0x0C) + at<float>(channel, 0));
    host.math_clamp_in_place_00415690(add(channel, 0), add(channel, 4), add(channel, 8));
}

namespace {
// The compass heading of a planar direction, as both 00810A90 and 00810CF0 compute it:
// a = (float)atan2(z, x); h = (float)(pi/2 - a) with pi/2 the widened float 00CE3830; a negative h
// gets 2pi (the widened float 00CE3828) added and is stored to float again.
float heading_from_direction(Seg50Host& host, float x, float z) {
    const auto angle = static_cast<float>(host.crt_atan2_00bf701a(z, x));
    auto heading = static_cast<float>(kHalfPiWide - angle);
    if (0.0f > heading) heading = static_cast<float>(static_cast<double>(heading) + kTwoPiWide);
    return heading;
}
}  // namespace

// 00810A90, __thiscall(ECX = estimator, x, y, z, heading, time), RET 14h. The helm estimator
// (unit+FACh): +0h heading, +4h..+0Ch position, +10h reset flag, +14h speed, +18h throttle channel,
// +30h rudder channel (both 00810900 channels), +48h last time, +4Ch unit.
// A time step backwards (last > 0 and last > time) sets last = -1.0f and the reset flag. With
// last < 0: last = time, speed = 0, reset flag set. Otherwise dt = (float)(time - last), last =
// time, and for dt > 0: v = (float)(delta position / dt) per axis; speed = 0042B2F0(&v) (stored to
// float, also +14h); s = min(speed / class+500h, 1.0f) (class = [[+4Ch]+538h]); s = -0.0f - s when
// |00438B10(heading_from(v.x, v.z), heading)| > pi/2; rudder = (double)(float)(00438B10(+0h,
// heading) / dt) / 0082ECB0(ECX = class, 1.0f, speed, 1.0f), stored to float. A reset seeds
// +30h = rudder and +18h = s and clears the flag; otherwise 00810900 moves +30h toward rudder and
// +18h toward s with dt. In every case the position and heading are stored last.
void helm_estimator_update_00810a90(Seg50Host& host, void* estimator, float x, float y, float z, float heading,
                                    float time) {
    const float previous = at<float>(estimator, 0x48);
    if (previous > 0.0f && previous > time) {
        at<float>(estimator, 0x48) = kMinusOne;
        at<std::uint8_t>(estimator, 0x10) = 1;
    }
    const float last = at<float>(estimator, 0x48);
    if (0.0f > last) {
        at<float>(estimator, 0x48) = time;
        at<float>(estimator, 0x14) = 0.0f;
        at<std::uint8_t>(estimator, 0x10) = 1;
    } else {
        const auto dt = static_cast<float>(static_cast<double>(time) - last);
        at<float>(estimator, 0x48) = time;
        if (dt > 0.0f) {
            const auto dx = static_cast<float>(static_cast<double>(x) - at<float>(estimator, 0x4));
            const auto dy = static_cast<float>(static_cast<double>(y) - at<float>(estimator, 0x8));
            const auto dz = static_cast<float>(static_cast<double>(z) - at<float>(estimator, 0xC));
            const float velocity[3] = {static_cast<float>(static_cast<double>(dx) / dt),
                                       static_cast<float>(static_cast<double>(dy) / dt),
                                       static_cast<float>(static_cast<double>(dz) / dt)};
            const auto speed = static_cast<float>(host.vector3_length_0042b2f0(velocity));
            at<float>(estimator, 0x14) = speed;
            void* const vehicle_class = ptr(ptr(estimator, 0x4C), 0x538);
            const auto ratio = static_cast<float>(static_cast<double>(speed) / at<float>(vehicle_class, 0x500));
            float throttle = 1.0f > ratio ? ratio : kOne;
            const float course = heading_from_direction(host, velocity[0], velocity[2]);
            float difference = host.math_subtract_wrapped_angle_00438b10(course, heading);
            std::uint32_t bits = 0;
            std::memcpy(&bits, &difference, sizeof bits);
            bits &= 0x7FFFFFFFu;
            std::memcpy(&difference, &bits, sizeof bits);
            if (static_cast<double>(difference) > kHalfPiWide) throttle = kNegativeZero - throttle;
            const auto turn_rate = static_cast<float>(
                static_cast<double>(host.math_subtract_wrapped_angle_00438b10(at<float>(estimator, 0), heading)) / dt);
            const double per_rudder = host.ship_class_yaw_rate_from_rudder_0082ecb0(vehicle_class, 1.0f, speed, 1.0f);
            const auto rudder = static_cast<float>(static_cast<double>(turn_rate) / per_rudder);
            if (at<std::uint8_t>(estimator, 0x10) != 0) {
                at<float>(estimator, 0x30) = rudder;
                at<float>(estimator, 0x18) = throttle;
                at<std::uint8_t>(estimator, 0x10) = 0;
            } else {
                helm_channel_rate_limit_towards_00810900(host, add(estimator, 0x30), rudder, dt);
                helm_channel_rate_limit_towards_00810900(host, add(estimator, 0x18), throttle, dt);
            }
        }
    }
    at<float>(estimator, 0x4) = x;
    at<float>(estimator, 0x8) = y;
    at<float>(estimator, 0xC) = z;
    at<float>(estimator, 0x0) = heading;
}

// 00810CF0, __thiscall(ECX = unit, ten dword arguments), RET 28h. Uses argument 1..3 (position),
// 4 and 6 (the direction x and z) and 10 (time); arguments 5, 7, 8 and 9 are not read. Calls
// 00810A90 on unit+FACh with the heading of the direction (pi/2 - atan2(z, x), wrapped).
void helm_estimator_update_from_vector_00810cf0(Seg50Host& host, void* unit, const float position[3],
                                                const float direction[3], const float unused[3], float time) {
    (void)unused;
    const float heading = heading_from_direction(host, direction[0], direction[2]);
    helm_estimator_update_00810a90(host, add(unit, 0xFAC), position[0], position[1], position[2], heading, time);
}

}  // namespace bsp::cc13_seg30
