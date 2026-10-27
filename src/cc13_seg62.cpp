// Packet cc13-seg62, segment 62 (00951F40..00966890). See include/bsp/cc13_seg62.hpp
// and docs/CC13_SEG62.md. Names are hypotheses; build-tested only.

#include "bsp/cc13_seg62.hpp"
#include "bsp/cc13_x87_model.hpp"

#include <cstring>

namespace bsp::cc13_seg62 {

namespace {

constexpr std::int32_t kDeviceKindGun = 0x20;     // vtable[5Ch] kind 20h, the gun device
constexpr std::size_t kDeviceListHead = 0x48;     // unit's flat device list
constexpr std::size_t kDeviceNext = 0x44;
constexpr std::size_t kDeviceDescriptor = 0x3f4;  // device class descriptor; +80h its gun category

}  // namespace

// 00951F80 __fastcall(ECX = unit), RET. Walks the device list (+48h, next +44h)
// and calls vtable[1ECh] on every device whose vtable[5Ch](20h) answers true.
// Caller 007C9792 (BSP_Plane_SEntityInitSlotA4). What 1ECh does is not read here.
void devices_kind20_call_1ec_00951f80(Seg62Host& host, Object unit) {
    for (Object device = obj(at<std::uint32_t>(unit, kDeviceListHead)); device != nullptr;
         device = obj(at<std::uint32_t>(device, kDeviceNext))) {
        if (host.vt5c_is_kind_of(device, kDeviceKindGun)) host.vt1ec(device);
    }
}

// 00952080 __thiscall(ECX = unit, const int* slot_table, iterator* out), RET 8.
// The unit keeps twelve 0Ch-byte typed device lists from +394h (+394h non-zero
// when not empty, +398h the first node). The slot table ends at a value >= 12.
// Clears the iterator, then stops at the first listed slot whose list is not
// empty and takes its first node; leaves node 0 when the table runs out.
void device_list_first_00952080(Object unit, const std::int32_t* slot_table, DeviceListIterator* it) {
    it->node = 0;
    it->index = 0;
    if (slot_table[0] >= 12) return;
    while (at<std::uint32_t>(unit, 0x394 + slot_table[it->index] * 0xc) == 0) {
        const std::int32_t index = it->index;
        it->index = index + 1;
        if (slot_table[index + 1] > 11) return;
    }
    it->node = at<std::uint32_t>(unit, 0x398 + slot_table[it->index] * 0xc);
}

// 009520E0 __thiscall(ECX = unit, const int* slot_table, iterator*), RET 8.
// Steps to the node's next (+4h); at the end of a list moves to the next slot
// in the table whose list is not empty. The node is left 0 when the table
// ends (the 0 just loaded from +4h stays).
void device_list_next_009520e0(Object unit, const std::int32_t* slot_table, DeviceListIterator* it) {
    const std::uint32_t next = at<std::uint32_t>(obj(it->node), 0x4);
    it->node = next;
    if (next != 0) return;
    it->index = it->index + 1;
    std::int32_t index = it->index;
    if (slot_table[index] >= 12) return;
    while (at<std::uint32_t>(unit, 0x394 + slot_table[index] * 0xc) == 0) {
        index = index + 1;
        it->index = index;
        if (slot_table[index] > 11) return;
    }
    it->node = at<std::uint32_t>(unit, 0x398 + slot_table[index] * 0xc);
}

// 00952150 __thiscall(ECX = entity, float dt), RET 4. A vtable slot shared by
// 25 entity vtables (00CF8DE4 .. 00D1A874). Counts the timer +4DCh down by dt
// (the difference is stored to the stack as a float and reloaded, so float
// arithmetic is exact here). While it stays positive: +4B4h..+4BCh = timer *
// (+4CCh..+4D4h), except that a zero +4CCh vector (00F87574, three 0.0f; the
// FUCOMIP/LAHF/JP equality) stores 00F87574 itself; and +4A8h..+4B0h = timer *
// (+4C0h..+4C8h). Otherwise the six vectors are zeroed in the image's chain
// order (+4C0h from 00F87574, then each copied from the previous) and the
// timer cleared. Field names unknown: two scaled impulse vectors.
void entity_decay_impulse_00952150(Object entity, float dt) {
    constexpr float kZero[3] = {0.0f, 0.0f, 0.0f};  // 00F87574
    const float timer = at<float>(entity, 0x4dc) - dt;
    at<float>(entity, 0x4dc) = timer;
    if (timer > 0.0f) {
        if (at<float>(entity, 0x4cc) == kZero[0] && at<float>(entity, 0x4d0) == kZero[1] &&
            at<float>(entity, 0x4d4) == kZero[2]) {
            at<float>(entity, 0x4b4) = kZero[0];
            at<float>(entity, 0x4b8) = kZero[1];
            at<float>(entity, 0x4bc) = kZero[2];
        } else {
            at<float>(entity, 0x4b4) = timer * at<float>(entity, 0x4cc);
            at<float>(entity, 0x4b8) = at<float>(entity, 0x4d0) * timer;
            at<float>(entity, 0x4bc) = timer * at<float>(entity, 0x4d4);
        }
        const float t = at<float>(entity, 0x4dc);
        at<float>(entity, 0x4a8) = t * at<float>(entity, 0x4c0);
        at<float>(entity, 0x4ac) = at<float>(entity, 0x4c4) * t;
        at<float>(entity, 0x4b0) = t * at<float>(entity, 0x4c8);
        return;
    }
    at<float>(entity, 0x4c0) = kZero[0];
    at<float>(entity, 0x4c4) = kZero[1];
    at<float>(entity, 0x4c8) = kZero[2];
    at<std::uint32_t>(entity, 0x4cc) = at<std::uint32_t>(entity, 0x4c0);
    at<std::uint32_t>(entity, 0x4d0) = at<std::uint32_t>(entity, 0x4c4);
    at<std::uint32_t>(entity, 0x4d4) = at<std::uint32_t>(entity, 0x4c8);
    at<std::uint32_t>(entity, 0x4a8) = at<std::uint32_t>(entity, 0x4cc);
    at<std::uint32_t>(entity, 0x4ac) = at<std::uint32_t>(entity, 0x4d0);
    at<std::uint32_t>(entity, 0x4b0) = at<std::uint32_t>(entity, 0x4d4);
    at<std::uint32_t>(entity, 0x4b4) = at<std::uint32_t>(entity, 0x4a8);
    at<std::uint32_t>(entity, 0x4b8) = at<std::uint32_t>(entity, 0x4ac);
    at<std::uint32_t>(entity, 0x4bc) = at<std::uint32_t>(entity, 0x4b0);
    at<std::uint32_t>(entity, 0x4dc) = 0;
}

// 00952950 __thiscall(ECX = out matrix, float x, float y, float z), RET 0Ch,
// EAX = out. Builds the diagonal 4x4 {x, y, z, 1.0f (00D7A24C)} on the stack
// and copies it into out through 004134F0. No xrefs in the saved analysis.
Object matrix_scale_00952950(Seg62Host& host, Object out, float x, float y, float z) {
    float m[16] = {};
    m[0] = x;
    m[5] = y;
    m[10] = z;
    m[15] = 1.0f;
    host.matrix_copy_004134f0(out, m);
    return out;
}

// 00952DF0 __cdecl(first, last, dest) -> EAX = dest end. std::copy over 44h-byte
// elements (one dword then sixteen floats; the image moves the floats through
// FLD/FSTP, which is a bit copy for every non-signalling value). Caller
// 00955030 (STL_inst).
std::uint32_t* copy_44_00952df0(const std::uint32_t* first, const std::uint32_t* last, std::uint32_t* dest) {
    // The FLD/FSTP copies quiet a signalling NaN; they are modelled (cc13-fixture10).
    for (; first != last; first += 0x11, dest += 0x11) {
        dest[0] = first[0];
        for (int i = 1; i < 0x11; ++i) dest[i] = cc13x87::x87_fld_fstp_bits(first[i]);
    }
    return dest;
}

// 00952F30 __cdecl(first, last, dest_end) -> EAX = dest begin. std::copy_backward
// over the same 44h-byte elements. Caller 009551F0 (STL_inst).
std::uint32_t* copy_backward_44_00952f30(const std::uint32_t* first, const std::uint32_t* last,
                                         std::uint32_t* dest_end) {
    while (last != first) {
        last -= 0x11;
        dest_end -= 0x11;
        dest_end[0] = last[0];
        for (int i = 1; i < 0x11; ++i) dest_end[i] = cc13x87::x87_fld_fstp_bits(last[i]);
    }
    return dest_end;
}

// 00953170 __fastcall(ECX = device) -> AL. A gun device (vtable[5Ch](20h)) whose
// descriptor (+3F4h) category +80h is 0. No xrefs in the saved analysis.
bool device_is_gun_kind0_00953170(Seg62Host& host, Object device) {
    return host.vt5c_is_kind_of(device, kDeviceKindGun) &&
           at<std::int32_t>(obj(at<std::uint32_t>(device, kDeviceDescriptor)), 0x80) == 0;
}

// 009531A0 __fastcall(ECX = device) -> AL. A gun device whose category is 2, 3,
// 4 or 6. No xrefs in the saved analysis.
bool device_is_gun_kind2346_009531a0(Seg62Host& host, Object device) {
    if (!host.vt5c_is_kind_of(device, kDeviceKindGun)) return false;
    const std::int32_t category = at<std::int32_t>(obj(at<std::uint32_t>(device, kDeviceDescriptor)), 0x80);
    return category == 2 || category == 3 || category == 4 || category == 6;
}

// 00953920 __fastcall(ECX = ship) -> EAX. Counts the non-null first words of the
// ten 18h-byte catapult slots +550h..+628h (GetNumCatapulted at 00892BCB; the
// 006EBD30 MaxLaunchedPlanes test).
std::uint32_t ship_count_catapulted_00953920(Object ship) {
    std::uint32_t count = 0;
    for (std::size_t slot = 0; slot < 10; ++slot) {
        if (at<std::uint32_t>(ship, 0x550 + slot * 0x18) != 0) ++count;
    }
    return count;
}

// 009539E0 __fastcall(ECX = ship) -> AL. The last catapulted entity (slot index
// +630h, slot +550h + 18h*i): when it answers vtable[5Ch](18h), its instance
// +3D0h must be set and 0043F080 true; otherwise its bytes +5Ch set and +5Dh,
// +60h, +5Eh clear. Either way the HUD root [[00E198C4]+40h] spectates it
// (00647300) and the answer is true; every other path answers false.
bool ship_spectate_last_catapulted_009539e0(Seg62Host& host, Object ship) {
    const std::int32_t slot = at<std::int32_t>(ship, 0x630);
    if (slot < 0) return false;
    const Object entity = obj(at<std::uint32_t>(ship, 0x550 + slot * 0x18));
    if (entity == nullptr) return false;
    if (host.vt5c_is_kind_of(entity, 0x18)) {
        const Object instance = obj(at<std::uint32_t>(entity, 0x3d0));
        if (instance == nullptr) return false;
        if (!host.unit_instance_alive_visible_0043f080(instance)) return false;
    } else {
        if (at<std::uint8_t>(entity, 0x5c) == 0) return false;
        if (at<std::uint8_t>(entity, 0x5d) != 0) return false;
        if (at<std::uint8_t>(entity, 0x60) != 0) return false;
        if (at<std::uint8_t>(entity, 0x5e) != 0) return false;
    }
    host.hud_set_spectated_unit_00647300(obj(at<std::uint32_t>(obj(host.global_u32(0x00e198c4)), 0x40)), entity);
    return true;
}

// 00953A60 __fastcall(ECX = ship) -> EAX. The first word of the catapult slot
// named by +630h, or 0 when that index is negative (GetLastCatapulted 00892993).
std::uint32_t ship_last_catapulted_00953a60(Object ship) {
    const std::int32_t slot = at<std::int32_t>(ship, 0x630);
    if (slot < 0) return 0;
    return at<std::uint32_t>(ship, 0x550 + slot * 0x18);
}

// 00953B80 __fastcall(ECX = unit), RET. Called once, from 0095E92C in
// BSP_Unit_ReadCommonLuaProperties. Sets +498h to -1.0f (00D7A260) and clears
// the byte +4A0h, then for every gun device (category from descriptor +80h):
// categories 0, 1 and 5 read the weapon record [[descriptor+74h]+34h],
// category 6 reads [[descriptor+74h]+7Ch]; others are skipped. The record's
// +60h is the candidate maximum; while +498h is still negative it takes the
// record's +50h; +4A0h ORs the descriptor byte +95h. +49Ch ends as the
// square of the largest +60h seen (starting at 0.0f; x87 FMUL ST0 of a float
// stored to a float). The image calls vtable[5Ch](20h) twice for a gun of
// category other than 1, 5, 6 (the second test leads to category 0), and once
// more for a non-gun; that call order is kept.
void unit_gather_gun_ranges_00953b80(Seg62Host& host, Object unit) {
    float best = 0.0f;
    at<float>(unit, 0x498) = -1.0f;
    at<std::uint8_t>(unit, 0x4a0) = 0;
    for (Object device = obj(at<std::uint32_t>(unit, kDeviceListHead)); device != nullptr;
         device = obj(at<std::uint32_t>(device, kDeviceNext))) {
        float candidate = 0.0f;
        std::size_t record_offset = 0;  // 0 = no record
        bool decided = false;
        if (host.vt5c_is_kind_of(device, kDeviceKindGun)) {
            const std::int32_t category = at<std::int32_t>(obj(at<std::uint32_t>(device, kDeviceDescriptor)), 0x80);
            if (category == 1 || category == 5) {
                record_offset = 0x34;
                decided = true;
            } else if (category == 6) {
                record_offset = 0x7c;
                decided = true;
            }
        }
        if (!decided && host.vt5c_is_kind_of(device, kDeviceKindGun)) {
            const std::int32_t category = at<std::int32_t>(obj(at<std::uint32_t>(device, kDeviceDescriptor)), 0x80);
            if (category == 0) record_offset = 0x34;
        }
        if (record_offset != 0) {
            const Object descriptor = obj(at<std::uint32_t>(device, kDeviceDescriptor));
            const Object record = obj(at<std::uint32_t>(obj(at<std::uint32_t>(descriptor, 0x74)), record_offset));
            candidate = at<float>(record, 0x60);
            if (0.0f > at<float>(unit, 0x498)) at<float>(unit, 0x498) = at<float>(record, 0x50);
            at<std::uint8_t>(unit, 0x4a0) = static_cast<std::uint8_t>(
                at<std::uint8_t>(unit, 0x4a0) | at<std::uint8_t>(obj(at<std::uint32_t>(device, kDeviceDescriptor)), 0x95));
        }
        if (candidate > best) best = candidate;
    }
    at<float>(unit, 0x49c) = best * best;
}

// 00954580 __fastcall(ECX = entity), RET. Callers 007C7927 (plane flight state)
// and 007CA42E (touchdown/crash). Clears the byte +4D8h, copies the 4x4 at +74h
// into +4E0h through 004134F0 (ECX = +4E0h, source +74h), then zeroes the impulse
// vectors and timer of 00952150 in the image's order (+4C0h from 00F87574,
// +4A8h from +4C0h, +4CCh from 00F87574, +4B4h from +4CCh; x87 FLD/FSTP copies of
// 0.0f), and clears +4DCh.
void entity_reset_impulse_00954580(Seg62Host& host, Object entity) {
    at<std::uint8_t>(entity, 0x4d8) = 0;
    host.matrix_copy_004134f0(entity + 0x4e0, reinterpret_cast<const float*>(entity + 0x74));
    at<float>(entity, 0x4c0) = 0.0f;
    at<float>(entity, 0x4c4) = 0.0f;
    at<float>(entity, 0x4c8) = 0.0f;
    at<std::uint32_t>(entity, 0x4a8) = at<std::uint32_t>(entity, 0x4c0);
    at<std::uint32_t>(entity, 0x4ac) = at<std::uint32_t>(entity, 0x4c4);
    at<std::uint32_t>(entity, 0x4b0) = at<std::uint32_t>(entity, 0x4c8);
    at<float>(entity, 0x4cc) = 0.0f;
    at<float>(entity, 0x4d0) = 0.0f;
    at<float>(entity, 0x4d4) = 0.0f;
    at<std::uint32_t>(entity, 0x4b4) = at<std::uint32_t>(entity, 0x4cc);
    at<std::uint32_t>(entity, 0x4b8) = at<std::uint32_t>(entity, 0x4d0);
    at<std::uint32_t>(entity, 0x4bc) = at<std::uint32_t>(entity, 0x4d4);
    at<std::uint32_t>(entity, 0x4dc) = 0;
}

// 00954DF0 __thiscall(ECX = list {count +0, head +4}, index), RET 4. Steps
// `index` times along next (+4h) from the head and answers the node's value
// (+8h). A walk that reaches a node without a next stops there and answers that
// node's value (the image's 00954E09 exit), so an index past the end answers
// the last value; an empty list answers 0. Caller 009552E0.
std::uint32_t list_value_at_00954df0(Object list, std::int32_t index) {
    Object node = obj(at<std::uint32_t>(list, 0x4));
    for (; index > 0; --index) {
        if (node == nullptr) return 0;
        const Object next = obj(at<std::uint32_t>(node, 0x4));
        if (next == nullptr) return at<std::uint32_t>(node, 0x8);
        node = next;
    }
    if (node == nullptr) return 0;
    return at<std::uint32_t>(node, 0x8);
}

// 00955090 __cdecl(first, last, const value*), RET. std::fill over 44h-byte
// elements (the floats move through FLD/FSTP). Called from the catch funclet
// 0095B2DB.
void fill_44_00955090(std::uint32_t* first, std::uint32_t* last, const std::uint32_t* value) {
    for (; first != last; first += 0x11) {
        first[0] = value[0];
        for (int i = 1; i < 0x11; ++i) first[i] = cc13x87::x87_fld_fstp_bits(value[i]);
    }
}

// 009552E0 __thiscall(ECX = vehicle class, loadout), RET 4. The class's
// Equipments list is {count +128h, head +12Ch}; +134h is the default loadout.
// A loadout in 1..count answers entry loadout-1 (tail jump to 00954DF0 with ECX
// = class+128h). Loadout 0 answers 0. A negative loadout or one past the count
// uses the default when 1 <= default and default-1 < count, else answers 0.
std::uint32_t class_loadout_record_009552e0(Object vehicle_class, std::int32_t loadout) {
    const Object list = vehicle_class + 0x128;
    const std::int32_t count = at<std::int32_t>(vehicle_class, 0x128);
    if (loadout > 0 && loadout <= count) return list_value_at_00954df0(list, loadout - 1);
    if (loadout == 0) return 0;
    const std::int32_t fallback = at<std::int32_t>(vehicle_class, 0x134);
    if (fallback <= 0) return 0;
    if (fallback - 1 >= count) return 0;
    return list_value_at_00954df0(list, fallback - 1);
}

// 009555A0 __thiscall(ECX = owner, shot), RET 4. The projectile spawn's last
// step (0072C155). A non-null owner first answers vtable[5Ch](0Fh), result
// unused. 006E6BB0(ECX = shot, owner) binds the owner; shot+14h takes the
// owner's +170h sub-object vtable[0]() and shot+18h the owner's +54h. The
// director is the owner's vtable[114h](); when that is null and the owner is
// kind 0Fh, its +9D4h's vtable[114h]() (when +9D4h is set). A director gets
// [director + 1D0h + [shot+4]+8h * 4] = [00F876A4] (a .data float, the clock).
void owner_register_projectile_009555a0(Seg62Host& host, Object owner, Object shot) {
    if (owner != nullptr) static_cast<void>(host.vt5c_is_kind_of(owner, 0xf));
    host.projectile_bind_owner_006e6bb0(shot, owner);
    at<std::uint32_t>(shot, 0x14) = host.vt0(owner + 0x170);
    at<std::uint32_t>(shot, 0x18) = at<std::uint32_t>(owner, 0x54);
    Object director = host.vt114(owner);
    if (director == nullptr) {
        if (!host.vt5c_is_kind_of(owner, 0xf)) return;
        const Object platform = obj(at<std::uint32_t>(owner, 0x9d4));
        if (platform == nullptr) return;
        director = host.vt114(platform);
        if (director == nullptr) return;
    }
    const std::int32_t sub_type = at<std::int32_t>(obj(at<std::uint32_t>(shot, 0x4)), 0x8);
    at<float>(director, 0x1d0 + sub_type * 4) = host.global_f32(0x00f876a4);
}

// 00955830 ECX = device, EDX = target point float[3], stack (out_a, out_b),
// RET 8. Refreshes the transform [device+3CCh] when bit 2 of its +5Ch is clear
// (00B6DB70); d = target - transform+120h..128h (float), normalized by 0042B260.
// On the first use (+10Ch clear) 00414DB0 refreshes the device pose, +10Ch is
// set, and 00B63D50(ECX = +110h, EDX = +CCh) caches the inverse at +110h. The
// direction goes into that frame (0042D0D0, normalize 1). *out_b = 0.0f; local y
// > 1.0f stores pi/2 (00CE3C64, 0x3FC90FDB) in *out_a, y < -1.0f stores -pi/2
// (00CE3CCC); otherwise 00521370 splits the local direction into (out_a,
// out_b). Then *out_b = -0.0f (00D7A208) - *out_b, and 00605070 wraps *out_a
// and *out_b. Used by the kind-1/2 arm of 00959C20.
void device_aim_direction_to_local_pair_00955830(Seg62Host& host, Object device, const float* target,
                                                 float* out_a, float* out_b) {
    const Object transform = obj(at<std::uint32_t>(device, 0x3cc));
    if ((at<std::uint8_t>(transform, 0x5c) & 2) == 0) host.transform_refresh_world_00b6db70(transform);
    const float origin[3] = {at<float>(transform, 0x120), at<float>(transform, 0x124), at<float>(transform, 0x128)};
    float d[3] = {target[0] - origin[0], target[1] - origin[1], target[2] - origin[2]};
    host.normalize_with_floor_0042b260(d);
    if (at<std::uint8_t>(device, 0x10c) == 0) {
        host.entity_pose_refresh_world_00414db0(device);
        at<std::uint8_t>(device, 0x10c) = 1;
        host.affine_inverse_00b63d50(device + 0x110, device + 0xcc);
    }
    float local_out[3];
    const float* r = host.transform_direction_0042d0d0(local_out, d, device + 0x110, 1);
    const float local[3] = {r[0], r[1], r[2]};
    *out_b = 0.0f;
    if (local[1] > 1.0f) {
        *out_a = 1.57079637f;   // 00CE3C64
    } else if (-1.0f > local[1]) {
        *out_a = -1.57079637f;  // 00CE3CCC
    } else {
        host.direction_to_pitch_yaw_00521370(local, out_a, out_b);
    }
    *out_b = -0.0f - *out_b;
    host.wrap_angle_00605070(out_a);
    host.wrap_angle_00605070(out_b);
}

namespace {

// The intrusive list {count +0, head +4, tail +8} with 0Ch-byte nodes {prev +0,
// next +4, value +8}: push_back as 00955C90 / 00955D00 / 00955E20 / 00956B40
// inline it. operator new(0Ch); a non-null node is zeroed first (the image then
// writes the node without testing it again).
void push_back(Seg62Host& host, Object list, std::uint32_t value) {
    const Object node = host.operator_new_00bf681b(0xc);
    if (node != nullptr) {
        at<std::uint32_t>(node, 0x0) = 0;
        at<std::uint32_t>(node, 0x4) = 0;
        at<std::uint32_t>(node, 0x8) = 0;
    }
    at<std::uint32_t>(node, 0x8) = value;
    at<std::uint32_t>(node, 0x0) = at<std::uint32_t>(list, 0x8);
    if (at<std::uint32_t>(list, 0x0) != 0) {
        at<std::uint32_t>(obj(at<std::uint32_t>(list, 0x8)), 0x4) = addr(node);
    } else {
        at<std::uint32_t>(list, 0x4) = addr(node);
    }
    at<std::uint32_t>(list, 0x8) = addr(node);
    at<std::uint32_t>(node, 0x4) = 0;
    at<std::uint32_t>(list, 0x0) = at<std::uint32_t>(list, 0x0) + 1;
}

// Unlinks the head node; optionally frees its value first (00955D60).
void pop_front(Seg62Host& host, Object list, bool free_value) {
    const Object node = obj(at<std::uint32_t>(list, 0x4));
    if (free_value && at<std::uint32_t>(node, 0x8) != 0) {
        host.free_00bf65ac(obj(at<std::uint32_t>(node, 0x8)));
        at<std::uint32_t>(node, 0x8) = 0;
    }
    if (at<std::uint32_t>(node, 0x0) == 0) {
        at<std::uint32_t>(list, 0x4) = at<std::uint32_t>(node, 0x4);
    } else {
        at<std::uint32_t>(obj(at<std::uint32_t>(node, 0x0)), 0x4) = at<std::uint32_t>(node, 0x4);
    }
    if (at<std::uint32_t>(node, 0x4) == 0) {
        at<std::uint32_t>(list, 0x8) = at<std::uint32_t>(node, 0x0);
    } else {
        at<std::uint32_t>(obj(at<std::uint32_t>(node, 0x4)), 0x0) = at<std::uint32_t>(node, 0x0);
    }
    at<std::uint32_t>(list, 0x0) = at<std::uint32_t>(list, 0x0) - 1;
    host.free_00bf65ac(node);
}

}  // namespace

// 00955C90, 00955D00, 00955E20: three identical __thiscall(ECX = list, value),
// RET 4 push_back instantiations. No xrefs in the saved analysis.
void list_push_back_00955c90(Seg62Host& host, Object list, std::uint32_t value) { push_back(host, list, value); }
void list_push_back_00955d00(Seg62Host& host, Object list, std::uint32_t value) { push_back(host, list, value); }
void list_push_back_00955e20(Seg62Host& host, Object list, std::uint32_t value) { push_back(host, list, value); }

// 00955D60 __fastcall(ECX = list), RET. Empties the list from the head, freeing
// each node's value (+8h, then zeroed) before the node. Caller 0095F1C0.
void list_clear_free_values_00955d60(Seg62Host& host, Object list) {
    while (at<std::uint32_t>(list, 0x0) != 0) pop_front(host, list, true);
}

// 00955EB0 __fastcall(ECX = list), RET. Empties the list from the head, freeing
// the nodes only. Callers 00956C20, 00957080, 00959940.
void list_clear_00955eb0(Seg62Host& host, Object list) {
    while (at<std::uint32_t>(list, 0x0) != 0) pop_front(host, list, false);
}

// 00956B40 __thiscall(ECX = unit, out list), RET 4. Appends every value of the
// unit's gun list (+424h; head +428h) to out.
void unit_copy_gun_list_00956b40(Seg62Host& host, Object unit, Object out_list) {
    for (Object node = obj(at<std::uint32_t>(unit, 0x428)); node != nullptr; node = obj(at<std::uint32_t>(node, 0x4))) {
        push_back(host, out_list, at<std::uint32_t>(node, 0x8));
    }
}

// 00956BA0 __thiscall(ECX = unit, gun), RET 4. A null gun does nothing. Erases
// the first node holding the gun from the list +424h (00954F90), then from the
// typed list of its category (+394h + category*0Ch, category [[gun+3F4h]+80h];
// the second erase is a tail jump).
void unit_remove_gun_00956ba0(Seg62Host& host, Object unit, Object gun) {
    if (gun == nullptr) return;
    for (Object node = obj(at<std::uint32_t>(unit, 0x428)); node != nullptr; node = obj(at<std::uint32_t>(node, 0x4))) {
        if (at<std::uint32_t>(node, 0x8) == addr(gun)) {
            static_cast<void>(host.list_erase_00954f90(unit + 0x424, node));
            break;
        }
    }
    const std::int32_t category = at<std::int32_t>(obj(at<std::uint32_t>(gun, kDeviceDescriptor)), 0x80);
    const Object typed = unit + 0x394 + category * 0xc;
    for (Object node = obj(at<std::uint32_t>(typed, 0x4)); node != nullptr; node = obj(at<std::uint32_t>(node, 0x4))) {
        if (at<std::uint32_t>(node, 0x8) == addr(gun)) {
            static_cast<void>(host.list_erase_00954f90(typed, node));
            return;
        }
    }
}

// 00956EE0 __thiscall(ECX = plane, bool firing), RET 4. Base vtable[1FCh]
// (from 007CA611 / 007CA5F0). Firing: when the class (+538h) has a gun-fire
// effect definition at +ECh and the plane has no effect at +670h, a point
// effect is created on the node +4A4h with an identity 4x4 parent matrix and
// words (1, 0, 0) (008687C0; the definition is passed as a retained reference:
// InterlockedIncrement on +4h when non-null), assigned into +670h (004845D0),
// and the temporary reference is released (InterlockedDecrement on +4h; at
// zero its vtable[0]() runs). Not firing: an effect at +670h is stopped
// (00867B10), its byte +9h set, and +670h adopts null (00484620(0)). The
// `-(p != 0) & 00E17BF8/00E186EC` tests in the listing are smart-pointer
// non-null tests. SEH frame 00CA8EF8 (unwind of the temporary) not modelled.
void plane_set_gun_fire_effect_00956ee0(Seg62Host& host, Object plane, bool firing) {
    const Object slot = plane + 0x670;
    if (!firing) {
        const Object effect = obj(at<std::uint32_t>(plane, 0x670));
        if (effect == nullptr) return;
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 0x9) = 1;
        host.reference_adopt_00484620(slot, nullptr);
        return;
    }
    const Object vehicle_class = obj(at<std::uint32_t>(plane, 0x538));
    if (at<std::uint32_t>(vehicle_class, 0xec) == 0) return;
    if (at<std::uint32_t>(plane, 0x670) != 0) return;
    const Object node = obj(at<std::uint32_t>(plane, 0x4a4));
    const float identity[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
                                0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    Object definition = nullptr;
    const Object source = obj(at<std::uint32_t>(vehicle_class, 0xec));
    if (source != nullptr) {
        definition = source;
        static_cast<void>(host.interlocked_increment(source + 0x4));
    }
    Object created = nullptr;
    Object* result = host.point_effect_create_008687c0(&created, node, definition, identity, 1, 0, 0);
    host.retained_assign_004845d0(slot, result);
    if (created != nullptr && host.interlocked_decrement(created + 0x4) == 0) {
        static_cast<void>(host.vt0(created));
    }
}

// 00957170 __thiscall(ECX = entity, const float* world_point), RET 4. Caller
// 0095822B (009580E0). Without a parent (+3Ch null) the point is stored as the
// local position +A4h..+ACh (FLD/FSTP copies). With one, the parent's cached
// inverse (+110h; built on first use: 00414DB0, +10Ch = 1, 00B63D50(+110h,
// +CCh)) maps the point through 004142E0 into +A4h..+ACh. Then the bytes +C8h
// and +10Ch are cleared and 0042ED50 runs on every child (+48h, next +44h).
void entity_set_world_position_00957170(Seg62Host& host, Object entity, const float* world_point) {
    const Object parent = obj(at<std::uint32_t>(entity, 0x3c));
    if (parent == nullptr) {
        at<float>(entity, 0xa4) = world_point[0];
        at<float>(entity, 0xa8) = world_point[1];
        at<float>(entity, 0xac) = world_point[2];
    } else {
        if (at<std::uint8_t>(parent, 0x10c) == 0) {
            host.entity_pose_refresh_world_00414db0(parent);
            at<std::uint8_t>(parent, 0x10c) = 1;
            host.affine_inverse_00b63d50(parent + 0x110, parent + 0xcc);
        }
        float local[3];
        host.transform_affine_point_004142e0(world_point, local, parent + 0x110);
        at<float>(entity, 0xa4) = local[0];
        at<float>(entity, 0xa8) = local[1];
        at<float>(entity, 0xac) = local[2];
    }
    at<std::uint8_t>(entity, 0xc8) = 0;
    at<std::uint8_t>(entity, 0x10c) = 0;
    for (Object child = obj(at<std::uint32_t>(entity, 0x48)); child != nullptr; child = obj(at<std::uint32_t>(child, 0x44))) {
        host.scene_node_invalidate_pose_0042ed50(child);
    }
}

// 009574D0 __thiscall(ECX = unit, flag record), RET 4. Caller 0095DE00. The
// string at flag+8h ({size +0, data +4}) is first resized (0041DD40(n, 0)) and
// filled by memcpy with "australia.tga" (00D0A054, 13) for nation +58h == 6, or
// "Zaszlok.tga" (00CFD4FC, 11) otherwise (skipped when the data pointer is
// null), then assigned by nation through the jump table 009575AC: 1
// "japanflag.tga", 2 "usaflag.tga", 3 "englishflag.tga", 5 "dutchflag.tga", 6
// "australianflag.tga" (0041E350); 4 and every other value resize to 12h and
// memcpy "newzealandflag.tga" (00D09FF0).
void flag_select_nation_texture_009574d0(Seg62Host& host, Object unit, Object flag) {
    const Object text = flag + 0x8;
    const std::int32_t nation = at<std::int32_t>(unit, 0x58);
    const char* first = nation == 6 ? "australia.tga" : "Zaszlok.tga";
    host.string_resize_0041dd40(text, nation == 6 ? 0xd : 0xb, 0);
    if (at<std::uint32_t>(text, 0x4) != 0) {
        host.memcpy_00bf7680(obj(at<std::uint32_t>(text, 0x4)), first, at<std::uint32_t>(text, 0x0));
    }
    switch (at<std::int32_t>(unit, 0x58)) {
    case 1: host.string_assign_0041e350(text, "japanflag.tga"); return;
    case 2: host.string_assign_0041e350(text, "usaflag.tga"); return;
    case 3: host.string_assign_0041e350(text, "englishflag.tga"); return;
    case 5: host.string_assign_0041e350(text, "dutchflag.tga"); return;
    case 6: host.string_assign_0041e350(text, "australianflag.tga"); return;
    default: break;
    }
    host.string_resize_0041dd40(text, 0x12, 0);
    if (at<std::uint32_t>(text, 0x4) != 0) {
        host.memcpy_00bf7680(obj(at<std::uint32_t>(text, 0x4)), "newzealandflag.tga", at<std::uint32_t>(text, 0x0));
    }
}

// 009575D0 __fastcall(ECX = unit), RET. Caller 007D5DA5 (BSP_Plane_ReadPropertyBag).
// First calls vtable[5Ch](20h) on every device (results unused). Then walks the
// class's (+538h) checked vector +D8h {first +DCh, last +E0h} of 10h-byte
// groups, each a vector<int> {first +4h, last +8h} of gun ids. For each id j of
// a group, the first gun device (vtable[5Ch](20h)) whose +38Ch equals the id
// gets 00727E10(group size, j). The image's checked-iterator tests (00BF6713)
// are kept at their points, including the empty index-check loop over the
// group before the main loop and the re-read of [unit+538h] each iteration.
void unit_bind_gun_groups_009575d0(Seg62Host& host, Object unit) {
    for (Object device = obj(at<std::uint32_t>(unit, kDeviceListHead)); device != nullptr;
         device = obj(at<std::uint32_t>(device, kDeviceNext))) {
        static_cast<void>(host.vt5c_is_kind_of(device, kDeviceKindGun));
    }
    const Object groups = obj(at<std::uint32_t>(unit, 0x538)) + 0xd8;
    std::uint32_t it = at<std::uint32_t>(groups, 0x4);
    if (it > at<std::uint32_t>(groups, 0x8)) host.invalid_parameter_00bf6713();
    for (;;) {
        const Object current = obj(at<std::uint32_t>(unit, 0x538)) + 0xd8;
        const std::uint32_t end = at<std::uint32_t>(current, 0x8);
        if (at<std::uint32_t>(current, 0x4) > end) host.invalid_parameter_00bf6713();
        if (groups != current) host.invalid_parameter_00bf6713();
        if (it == end) return;
        if (it >= at<std::uint32_t>(groups, 0x8)) host.invalid_parameter_00bf6713();
        const Object group = obj(it);
        const std::int32_t count =
            at<std::uint32_t>(group, 0x4) == 0
                ? 0
                : static_cast<std::int32_t>(at<std::uint32_t>(group, 0x8) - at<std::uint32_t>(group, 0x4)) >> 2;
        for (std::int32_t i = 0; i < count; ++i) {
            const std::uint32_t first = at<std::uint32_t>(group, 0x4);
            if (first == 0 ||
                static_cast<std::uint32_t>(i) >=
                    static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(group, 0x8) - first) >> 2)) {
                host.invalid_parameter_00bf6713();
            }
        }
        for (std::uint32_t j = 0; j < host.vector_size_00415450(group); ++j) {
            const std::uint32_t first = at<std::uint32_t>(group, 0x4);
            if (first == 0 ||
                j >= static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::uint32_t>(group, 0x8) - first) >> 2)) {
                host.invalid_parameter_00bf6713();
            }
            const std::int32_t id = at<std::int32_t>(obj(at<std::uint32_t>(group, 0x4)), j * 4);
            Object found = nullptr;
            for (Object device = obj(at<std::uint32_t>(unit, kDeviceListHead)); device != nullptr && found == nullptr;
                 device = obj(at<std::uint32_t>(device, kDeviceNext))) {
                if (host.vt5c_is_kind_of(device, kDeviceKindGun) && at<std::int32_t>(device, 0x38c) == id) found = device;
            }
            if (found != nullptr) host.gun_set_pair_00727e10(found, host.vector_size_00415450(group), j);
        }
        if (it >= at<std::uint32_t>(groups, 0x8)) host.invalid_parameter_00bf6713();
        it += 0x10;
    }
}

// 009580E0 __fastcall(ECX = entity), RET. The vtable slot after 00952150 in the
// same 25 entity vtables (00CF8DE8 ..). Sets +4D8h, copies the 4x4 +74h into
// +4E0h (004134F0), then:
// - both impulse vectors +4A8h and +4B4h zero (six FUCOMIP equalities against
//   00F87574): clears +4D8h and returns;
// - only +4B4h zero: refreshes the pose when +C8h is clear (00414DB0) and moves
//   the entity to world position +FCh..+104h plus +4A8h through 00957170;
// - otherwise: refresh as above; p = +FCh.. + +4A8h; a second refresh when +C8h
//   was clear at the test before the reads; 0042D2E0 takes the angles (a, b, c)
//   of the world matrix +CCh; each gets 00438AA0 with +4B4h, +4B8h, +4BCh
//   (results stored as floats); a 4x4 identity on the stack gets 00467050(a,
//   b, c), its translation row p, and 00741E90 sets it as the world pose.
// Float sums are stored to floats before use (FSTP), so float arithmetic here
// is exact to the image.
void entity_apply_impulse_pose_009580e0(Seg62Host& host, Object entity) {
    at<std::uint8_t>(entity, 0x4d8) = 1;
    host.matrix_copy_004134f0(entity + 0x4e0, reinterpret_cast<const float*>(entity + 0x74));
    const bool linear_zero = at<float>(entity, 0x4a8) == 0.0f && at<float>(entity, 0x4ac) == 0.0f &&
                             at<float>(entity, 0x4b0) == 0.0f;
    const bool angular_zero = at<float>(entity, 0x4b4) == 0.0f && at<float>(entity, 0x4b8) == 0.0f &&
                              at<float>(entity, 0x4bc) == 0.0f;
    if (linear_zero && angular_zero) {
        at<std::uint8_t>(entity, 0x4d8) = 0;
        return;
    }
    if (angular_zero) {
        if (at<std::uint8_t>(entity, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(entity);
        const float p[3] = {at<float>(entity, 0x4a8) + at<float>(entity, 0xfc),
                            at<float>(entity, 0x4ac) + at<float>(entity, 0x100),
                            at<float>(entity, 0x4b0) + at<float>(entity, 0x104)};
        entity_set_world_position_00957170(host, entity, p);
        return;
    }
    if (at<std::uint8_t>(entity, 0xc8) == 0) host.entity_pose_refresh_world_00414db0(entity);
    const bool refreshed = at<std::uint8_t>(entity, 0xc8) != 0;
    const float p[3] = {at<float>(entity, 0x4a8) + at<float>(entity, 0xcc + 0x30),
                        at<float>(entity, 0x4ac) + at<float>(entity, 0xcc + 0x34),
                        at<float>(entity, 0x4b0) + at<float>(entity, 0xcc + 0x38)};
    if (!refreshed) host.entity_pose_refresh_world_00414db0(entity);
    float a = 0.0f;
    float b = 0.0f;
    float c = 0.0f;
    host.extract_basis_angles_0042d2e0(entity + 0xcc, &a, &b, &c);
    a = host.add_wrapped_angle_00438aa0(a, at<float>(entity, 0x4b4));
    b = host.add_wrapped_angle_00438aa0(b, at<float>(entity, 0x4b8));
    c = host.add_wrapped_angle_00438aa0(c, at<float>(entity, 0x4bc));
    float m[16] = {1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f};
    host.matrix_set_rotation_euler_00467050(m, a, b, c);
    m[12] = p[0];
    m[13] = p[1];
    m[14] = p[2];
    host.entity_set_world_pose_00741e90(entity, m);
}

namespace {

// One 14h-byte element of the vectors 009588A0 / 00958910 copy: dword +0,
// dword +4, native string +8 {size +8, data +0Ch}, dword +10h. The string is
// assigned as the image inlines it: unless it is the same object, resize to
// the source size with fill 1 (0041DD40) and memcpy the bytes when the source
// size is non-zero.
void assign_element_14(Seg62Host& host, Object dest, Object source) {
    at<std::uint32_t>(dest, 0x0) = at<std::uint32_t>(source, 0x0);
    at<std::uint32_t>(dest, 0x4) = at<std::uint32_t>(source, 0x4);
    if (dest + 0x8 != source + 0x8) {
        host.string_resize_0041dd40(dest + 0x8, at<std::uint32_t>(source, 0x8), 1);
        if (at<std::uint32_t>(source, 0x8) != 0) {
            host.memcpy_00bf7680(obj(at<std::uint32_t>(dest, 0xc)), obj(at<std::uint32_t>(source, 0xc)),
                                 at<std::uint32_t>(dest, 0x8));
        }
    }
    at<std::uint32_t>(dest, 0x10) = at<std::uint32_t>(source, 0x10);
}

}  // namespace

// 009588A0 __cdecl(first, last, dest) -> EAX = dest end. std::copy over the
// 14h-byte elements. Callers 0095971E, 009590EE (STL_inst).
Object copy_14_009588a0(Seg62Host& host, Object first, Object last, Object dest) {
    for (; first != last; first += 0x14, dest += 0x14) assign_element_14(host, dest, first);
    return dest;
}

// 00958910 __cdecl(first, last, dest_end) -> EAX = dest begin. std::copy_backward
// over the 14h-byte elements. Caller 009591BE (STL_inst).
Object copy_backward_14_00958910(Seg62Host& host, Object first, Object last, Object dest_end) {
    while (last != first) {
        last -= 0x14;
        dest_end -= 0x14;
        assign_element_14(host, dest_end, last);
    }
    return dest_end;
}

// 00958E90 __stdcall(a, b, const Object* reference), RET 0Ch -> EAX. Callers
// 00961520 and 009615B8 (BSP_VehicleClass_ReadLuaFields). operator new(0Ch);
// the image's placement-new null tests are on the field addresses (node,
// node+4, node+8), so only the first is a real test: +0 = a, +4 = b, +8 = the
// reference retained (InterlockedIncrement on its +4h when non-null, else 0).
// SEH frame 00CA9001 not modelled.
Object new_class_entry_00958e90(Seg62Host& host, std::uint32_t a, std::uint32_t b, const Object* reference) {
    const Object node = host.operator_new_00bf681b(0xc);
    if (node != nullptr) at<std::uint32_t>(node, 0x0) = a;
    if (addr(node) + 0x4 != 0) at<std::uint32_t>(node, 0x4) = b;
    if (addr(node) + 0x8 != 0) {
        at<std::uint32_t>(node, 0x8) = 0;
        const Object value = *reference;
        if (value != nullptr) {
            at<std::uint32_t>(node, 0x8) = addr(value);
            static_cast<void>(host.interlocked_increment(value + 0x4));
        }
    }
    return node;
}

namespace {

// The image's inlined native-string destructor: a string with data returns
// its block (size + 1) to the sized pool (00419CC0, then 00BD1510(block,
// size + 1, 1)).
void destroy_string(Seg62Host& host, const std::uint32_t* string) {
    if (string[1] == 0) return;
    const Object pool = host.pool_singleton_00419cc0();
    host.pool_return_block_00bd1510(pool, obj(string[1]), string[0] + 1, 1);
}

}  // namespace

// 00959230 ECX = out string, EDX = record {count +0, +4 -> object whose +8 ->
// object whose +4 is a device-class id}, RET; EAX = out. No caller in the
// saved analysis. Clears out {size, data}. A null record or a zero count gives
// "globals.none" (00CF36F8; resize 0Ch, memcpy when data). Otherwise the class
// (00443490(id, DL 0)) name +54h: count 1 assigns the name (0041E350); any other
// count builds "." (00CE3A70) + decimal count (004263B0) + "x |" (00D1A64C) +
// name (004261A0 twice) into out (00425F40), then destroys the six
// temporaries in reverse construction order. SEH 00CA9079 not modelled.
Object record_display_key_00959230(Seg62Host& host, Object out, Object record) {
    at<std::uint32_t>(out, 0x0) = 0;
    at<std::uint32_t>(out, 0x4) = 0;
    if (record == nullptr || at<std::int32_t>(record, 0x0) == 0) {
        host.string_resize_0041dd40(out, 0xc, 0);
        if (at<std::uint32_t>(out, 0x4) != 0) {
            host.memcpy_00bf7680(obj(at<std::uint32_t>(out, 0x4)), "globals.none", at<std::uint32_t>(out, 0x0));
        }
        return out;
    }
    const std::uint32_t id = at<std::uint32_t>(obj(at<std::uint32_t>(obj(at<std::uint32_t>(record, 0x4)), 0x8)), 0x4);
    const Object device_class = host.device_class_resolve_00443490(id, 0);
    const char* name = reinterpret_cast<const char*>(obj(at<std::uint32_t>(device_class, 0x54)));
    if (at<std::int32_t>(record, 0x0) == 1) {
        host.string_assign_0041e350(out, name);
        return out;
    }
    std::uint32_t s_name[2];
    std::uint32_t s_times[2];
    std::uint32_t s_dot[2];
    std::uint32_t t1[2];
    std::uint32_t t2[2];
    std::uint32_t t3[2];
    host.string_construct_0041e870(reinterpret_cast<Object>(s_name), name);
    host.string_construct_0041e870(reinterpret_cast<Object>(s_times), "x |");
    const Object dot = host.string_construct_0041e870(reinterpret_cast<Object>(s_dot), ".");
    const Object r1 = host.string_concat_int_004263b0(dot, reinterpret_cast<Object>(t1), at<std::int32_t>(record, 0x0));
    const Object r2 = host.string_concat_004261a0(r1, reinterpret_cast<Object>(t2), reinterpret_cast<Object>(s_times));
    const Object r3 = host.string_concat_004261a0(r2, reinterpret_cast<Object>(t3), reinterpret_cast<Object>(s_name));
    host.string_assign_header_00425f40(out, r3);
    destroy_string(host, t3);
    destroy_string(host, t2);
    destroy_string(host, t1);
    destroy_string(host, s_dot);
    destroy_string(host, s_times);
    destroy_string(host, s_name);
    return out;
}

namespace {

// Copy-constructs one 14h-byte element (see assign_element_14) in place, as
// 009597E0 / 0095A740 inline it: a null destination is skipped; the string
// header is zeroed and then assigned unless it aliases the source.
void construct_element_14(Seg62Host& host, Object dest, Object source) {
    if (dest == nullptr) return;
    at<std::uint32_t>(dest, 0x0) = at<std::uint32_t>(source, 0x0);
    at<std::uint32_t>(dest, 0x4) = at<std::uint32_t>(source, 0x4);
    at<std::uint32_t>(dest, 0x8) = 0;
    at<std::uint32_t>(dest, 0xc) = 0;
    if (dest + 0x8 != source + 0x8) {
        host.string_resize_0041dd40(dest + 0x8, at<std::uint32_t>(source, 0x8), 1);
        if (at<std::uint32_t>(source, 0x8) != 0) {
            host.memcpy_00bf7680(obj(at<std::uint32_t>(dest, 0xc)), obj(at<std::uint32_t>(source, 0xc)),
                                 at<std::uint32_t>(dest, 0x8));
        }
    }
    at<std::uint32_t>(dest, 0x10) = at<std::uint32_t>(source, 0x10);
}

}  // namespace

// 009597E0 ECX = first, EDX = last, stack (dest, three unused words), RET 10h,
// EAX = dest end. std::uninitialized_copy over the 14h-byte elements. The catch
// funclet (00747C10 destroys the built range, 00BF6885 rethrows) is not
// modelled. Callers 0095D3AC, 0095D630, 0095D65C.
Object uninitialized_copy_14_009597e0(Seg62Host& host, Object first, Object last, Object dest) {
    for (; first != last; first += 0x14, dest += 0x14) construct_element_14(host, dest, first);
    return dest;
}

// 0095A740 ECX = dest, EDX = count, stack (const value*, three unused words),
// RET 10h. std::uninitialized_fill_n over the 14h-byte elements; same catch
// funclet. Caller 0095C114.
void uninitialized_fill_n_14_0095a740(Seg62Host& host, Object dest, std::uint32_t count, Object value) {
    for (; count != 0; --count, dest += 0x14) construct_element_14(host, dest, value);
}

namespace {

TaggedValue string_key(const char* text) { return TaggedValue{0, addr(text)}; }
TaggedValue float_value(float value) {
    TaggedValue out{2, 0};
    static_assert(sizeof(float) == sizeof(std::uint32_t), "float width");
    std::memcpy(&out.value, &value, 4);
    return out;
}

// Writes `key` = float `value` unless it equals `skip` (00BD5680 on two
// float tagged values, as 0095BBB4..0095BCAD build them).
void write_float_unless(Seg62Host& host, Object archive, const char* key, float value, float skip) {
    const TaggedValue current = float_value(value);
    const TaggedValue reference = float_value(skip);
    if (!host.tagged_value_equals_00bd5680(&current, &reference)) {
        host.archive_vt0c_write(archive, string_key(key), current);
    }
}

}  // namespace

// 0095BB10 __thiscall(ECX = unit, archive), RET 4. The vehicle unit's state
// writer (seven callers: 006D37E0, 00700870, 007455F0, 0074D3B0 ...). 008776E0
// writes the base state; "vehicleclass" = int (tag 1) the registry entry
// [00437F50() + 2010h + [class+70h]*4] (0 without a class); opens "_vehicle"
// and "_botLevelHandler", writes the skill level (006E63A0 on +38Ch), closes
// one table; "exitTimer" (+6F0h) unless 90.0f (00D1A918), "lastFireTime"
// (+700h) and "lastSmokeTime" (+704h) unless -1e10f (00CE4ADC, 0xD01502F9);
// "enablePlayerSteer" bool +6C8h, "fireState" int +6E4h, "torpedoDeviation"
// float +6D4h; "weaponDirector" true when +6DCh is set and its byte +10h is
// clear; then closes the second table. The literals are 00D1A8A8..00D1A93C.
void vehicle_unit_write_state_0095bb10(Seg62Host& host, Object unit, Object archive) {
    host.unit_write_state_008776e0(unit, archive);
    std::uint32_t entry = 0;
    const Object vehicle_class = obj(at<std::uint32_t>(unit, 0x538));
    if (vehicle_class != nullptr) {
        const Object registry = host.class_registry_00437f50();
        entry = at<std::uint32_t>(registry, 0x2010 + at<std::int32_t>(vehicle_class, 0x70) * 4);
    }
    host.archive_vt0c_write(archive, string_key("vehicleclass"), TaggedValue{1, entry});
    host.archive_vt4_open(archive, string_key("_vehicle"));
    host.archive_vt4_open(archive, string_key("_botLevelHandler"));
    host.skill_level_write_006e63a0(unit + 0x38c, archive);
    host.archive_vt8_close(archive);
    write_float_unless(host, archive, "exitTimer", at<float>(unit, 0x6f0), 90.0f);
    write_float_unless(host, archive, "lastFireTime", at<float>(unit, 0x700), -1.0e10f);
    write_float_unless(host, archive, "lastSmokeTime", at<float>(unit, 0x704), -1.0e10f);
    host.archive_vt0c_write(archive, string_key("enablePlayerSteer"), TaggedValue{3, at<std::uint8_t>(unit, 0x6c8)});
    host.archive_vt0c_write(archive, string_key("fireState"), TaggedValue{1, at<std::uint32_t>(unit, 0x6e4)});
    host.archive_vt0c_write(archive, string_key("torpedoDeviation"), float_value(at<float>(unit, 0x6d4)));
    const Object director = obj(at<std::uint32_t>(unit, 0x6dc));
    if (director != nullptr && at<std::uint8_t>(director, 0x10) == 0) {
        host.archive_vt0c_write(archive, string_key("weaponDirector"), TaggedValue{3, 1});
    }
    host.archive_vt8_close(archive);
}

// 0095B8D0, 0095C170, 0095CA50: three identical __fastcall(ECX = array) tidy
// bodies: 005CD640(ECX = array, 0) shrinks the camera array to nothing, then
// 00BF6989 frees its storage [array]. 0095CA50 is called from the unwind
// funclets 00CA9463, 00CA9483, 00CA9A8D; the other two have no xrefs.
void camera_array_tidy_0095b8d0(Seg62Host& host, Object array) {
    host.camera_array_resize_005cd640(array, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(array, 0x0)));
}
void camera_array_tidy_0095c170(Seg62Host& host, Object array) {
    host.camera_array_resize_005cd640(array, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(array, 0x0)));
}
void camera_array_tidy_0095ca50(Seg62Host& host, Object array) {
    host.camera_array_resize_005cd640(array, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(array, 0x0)));
}

// 0095C1C0 ECX = out string, EDX = class id, RET; EAX = out. Caller
// BSP_Scene_BuildClassPreloadAliases. The registry (00437F50, read afresh at
// every use) maps the id to an entry e = [reg + 10h + id*4] (read twice).
// out = "Class_" (00D1A95C). The Lua table VehicleClass[e] (globals of
// [00E188A8]+1A0Ch) gives "Type" as a native string T. When T is ReconPlane
// (_stricmp, only when T has data), SmallReconPlane, LargeReconPlane, Fighter,
// DiveBomber, TorpedoBomber, Kamikaze or LevelBomber (00425850): its "Race"
// integer (default 0) picks party 1 for race 1 or 4, else 0; the row r = [reg
// + 10h + e*4] (the image indexes the entry table with the entry itself) of the
// party rows {data +4010h, count +4014h} is grown to r+1 when needed (004359E0)
// and byte [data + r*3 + party] = 1; then "enemy_" or "own_" (0095BAB0(e)) is
// appended (resize with fill 1 + memcpy, then the temporary released). Every
// path appends e as decimal (004CACA0), releases T and destroys the row and
// VehicleClass objects. SEH 00CA92DA not modelled.
Object class_preload_alias_0095c1c0(Seg62Host& host, Object out, std::int32_t class_id) {
    const std::uint32_t entry_first = at<std::uint32_t>(host.class_registry_00437f50(), 0x10 + class_id * 4);
    const std::uint32_t entry = at<std::uint32_t>(host.class_registry_00437f50(), 0x10 + class_id * 4);
    host.string_construct_0041e870(out, "Class_");
    std::uint8_t globals[0x14];
    std::uint8_t vehicle_class[0x14];
    std::uint8_t row[0x14];
    std::uint8_t field[0x14];
    const Object owner = obj(host.global_u32(0x00e188a8)) + 0x1a0c;
    const Object g = host.lua_globals_00b67980(owner, globals);
    host.lua_get_by_name_00b67800(g, vehicle_class, "VehicleClass");
    host.lua_destruct_00b67700(globals);
    host.lua_get_by_index_00b67720(vehicle_class, row, static_cast<std::int32_t>(entry));
    const Object type_field = host.lua_get_by_name_00b67800(row, field, "Type");
    std::uint32_t type[2];
    host.string_construct_0041e870(reinterpret_cast<Object>(type), host.lua_get_string_00b662b0(type_field));
    host.lua_destruct_00b67700(field);
    const Object type_string = reinterpret_cast<Object>(type);
    bool plane = false;
    if (type[1] != 0 && host.stricmp_00bf7fbf(reinterpret_cast<const char*>(obj(type[1])), "ReconPlane") == 0) {
        plane = true;
    } else {
        static const char* const kPlaneTypes[] = {"SmallReconPlane", "LargeReconPlane", "Fighter", "DiveBomber",
                                                  "TorpedoBomber",   "Kamikaze",        "LevelBomber"};
        for (const char* name : kPlaneTypes) {
            if (host.string_equals_ci_00425850(type_string, name)) {
                plane = true;
                break;
            }
        }
    }
    if (plane) {
        std::uint8_t race_field[0x14];
        const Object race_object = host.lua_get_by_name_00b67800(row, race_field, "Race");
        const std::int32_t race = host.lua_get_int_or_default_00b66380(race_object, 0);
        host.lua_destruct_00b67700(race_field);
        const std::uint32_t party = (race == 1 || race == 4) ? 1 : 0;
        const std::int32_t slot = at<std::int32_t>(host.class_registry_00437f50(), 0x10 + entry * 4);
        const Object registry = host.class_registry_00437f50();
        if (slot >= at<std::int32_t>(registry, 0x4014)) host.party_rows_resize_004359e0(registry + 0x4010, slot + 1);
        at<std::uint8_t>(obj(at<std::uint32_t>(registry, 0x4010)), slot * 3 + party) = 1;
        const char* side = host.class_is_enemy_0095bab0(entry) ? "enemy_" : "own_";
        std::uint32_t prefix[2];
        host.string_construct_0041e870(reinterpret_cast<Object>(prefix), side);
        const std::uint32_t size = prefix[0];
        if (size != 0) {
            const std::uint32_t old_size = at<std::uint32_t>(out, 0x0);
            host.string_resize_0041dd40(out, size + old_size, 1);
            host.memcpy_00bf7680(obj(at<std::uint32_t>(out, 0x4) + old_size), obj(prefix[1]), size);
        }
        destroy_string(host, prefix);
    }
    host.string_append_int_004caca0(out, static_cast<std::int32_t>(entry_first));
    destroy_string(host, type);
    host.lua_destruct_00b67700(row);
    host.lua_destruct_00b67700(vehicle_class);
    return out;
}

// 0095CA70 __cdecl(), RET. Callers 0045F6C1 (BSP_EngineMovie_Stop) and 004E0A9B
// (BSP_Game_LoadMissionScene). Iterates the Lua table VehicleClass (globals of
// [00E188A8]+1A0Ch) with a key and value object (00B67080 / 00B67190, ended by
// 00B66420); for every value whose "Got" (00CE452C) field fails 00B66000, the
// key's integer (00B66290) is pushed onto a local vector<int>. Then for each
// collected index i: "VehicleClass[%d]=nil" (00D1A964) is formatted into a
// 2Ah-byte buffer (0043EE60) and run by the Lua machine [[00E188A8]+1A08h]+4h
// (006B8AD0(buffer, 0, 0, 2)); the image's checked index (00BF6713) is kept.
// The value, key and table objects are destroyed and the vector storage freed
// (00BF65AC). SEH 00CA93B9 not modelled.
void class_registry_clear_lua_tables_0095ca70(Seg62Host& host) {
    std::uint32_t indices[4] = {0, 0, 0, 0};  // {proxy, first, last, end}
    const Object vector = reinterpret_cast<Object>(indices);
    std::uint8_t globals[0x14];
    std::uint8_t table[0x14];
    std::uint8_t key[0x14];
    std::uint8_t value[0x14];
    const Object owner = obj(host.global_u32(0x00e188a8)) + 0x1a0c;
    const Object g = host.lua_globals_00b67980(owner, globals);
    host.lua_get_by_name_00b67800(g, table, "VehicleClass");
    host.lua_destruct_00b67700(globals);
    host.lua_construct_00b65f50(key);
    host.lua_construct_00b65f50(value);
    host.lua_iterate_first_00b67080(table, key, value);
    while (!host.lua_iterate_done_00b66420(table, key)) {
        std::uint8_t got[0x14];
        const Object field = host.lua_get_by_name_00b67800(value, got, "Got");
        const bool keep = !host.lua_test_00b66000(field);
        host.lua_destruct_00b67700(got);
        if (keep) {
            const std::int32_t index = host.lua_get_integer_00b66290(key);
            host.int_vector_push_back_00442190(vector, &index);
        }
        host.lua_iterate_next_00b67190(table, key, value);
    }
    for (std::uint32_t i = 0; i < host.vector_size_00415450(vector); ++i) {
        const std::uint32_t first = indices[1];
        if (first == 0 || i >= static_cast<std::uint32_t>(static_cast<std::int32_t>(indices[2] - first) >> 2)) {
            host.invalid_parameter_00bf6713();
        }
        char buffer[0x2c];
        host.format_buffer_0043ee60(buffer, "VehicleClass[%d]=nil", at<std::int32_t>(obj(indices[1]), i * 4));
        const Object machine = obj(at<std::uint32_t>(obj(at<std::uint32_t>(obj(host.global_u32(0x00e188a8)), 0x1a08)), 0x4));
        host.lua_run_string_006b8ad0(machine, buffer, nullptr, nullptr, 2);
    }
    host.lua_destruct_00b67700(value);
    host.lua_destruct_00b67700(key);
    host.lua_destruct_00b67700(table);
    if (indices[1] != 0) host.free_00bf65ac(obj(indices[1]));
}

namespace {

// The image's inlined retained-reference assignment: when the slot changes,
// store the new pointer, InterlockedIncrement its +4h when non-null, then
// InterlockedDecrement the old one's +4h and run its vtable[0]() at zero.
void assign_retained(Seg62Host& host, Object slot, std::uint32_t value) {
    const std::uint32_t old = at<std::uint32_t>(slot, 0x0);
    if (old == value) return;
    at<std::uint32_t>(slot, 0x0) = value;
    if (value != 0) static_cast<void>(host.interlocked_increment(obj(value) + 0x4));
    if (old != 0 && host.interlocked_decrement(obj(old) + 0x4) == 0) static_cast<void>(host.vt0(obj(old)));
}

}  // namespace

// 0095CF80 __thiscall(ECX = record, const source*), RET 4, EAX = record.
// Copy-assignment of the ammunition record used at 0095ECD4 (ammo+48h, the flak
// alternate). +4h, +8h copied (FLD/FSTP), the retained references +0Ch, +10h,
// +14h, +18h assigned, +1Ch..+30h copied, references +34h, +38h assigned; then
// unless the tree +3Ch is the source's own, it is emptied (00441E50 over
// [begin = [head], end = head]) and copied from the source tree (00440D90).
Object ammo_record_assign_0095cf80(Seg62Host& host, Object record, Object source) {
    at<std::uint32_t>(record, 0x4) = at<std::uint32_t>(source, 0x4);
    at<std::uint32_t>(record, 0x8) = at<std::uint32_t>(source, 0x8);
    for (std::size_t offset = 0xc; offset <= 0x18; offset += 4) {
        assign_retained(host, record + offset, at<std::uint32_t>(source, offset));
    }
    for (std::size_t offset = 0x1c; offset <= 0x30; offset += 4) {
        at<std::uint32_t>(record, offset) = at<std::uint32_t>(source, offset);
    }
    assign_retained(host, record + 0x34, at<std::uint32_t>(source, 0x34));
    assign_retained(host, record + 0x38, at<std::uint32_t>(source, 0x38));
    const Object tree = record + 0x3c;
    if (tree != source + 0x3c) {
        const Object head = obj(at<std::uint32_t>(tree, 0x4));
        std::uint32_t out_iterator[2];
        host.tree_erase_range_00441e50(tree, reinterpret_cast<Object>(out_iterator), tree,
                                       obj(at<std::uint32_t>(head, 0x0)), tree, head);
        host.tree_copy_00440d90(tree, source + 0x3c);
    }
    return record;
}

namespace {

// The 44h-byte element the resize calls pass by value: its default
// constructor sets only the first dword to 0; the image copies the other 10h
// dwords from uninitialized stack (REP MOVSD), modelled here as zero.
void resize_with_default(Seg62Host& host, Object vector, std::uint32_t count) {
    const std::uint32_t element[0x11] = {};
    host.vector44_resize_0095be70(vector, count, element);
}

}  // namespace

// 0095D120 __thiscall(ECX = vector, count), RET 4. resize(count) of a vector of
// 44h-byte elements (0095BE70). No xrefs in the saved analysis.
void vector44_resize_default_0095d120(Seg62Host& host, Object vector, std::uint32_t count) {
    resize_with_default(host, vector, count);
}

// 0095D3C0 __fastcall(ECX = unit), RET. Empties the vector +660h (0095BE70(0)).
// Caller 006F5433 (BSP_CommandBuilding_HandleMessageD3).
void unit_clear_vector660_0095d3c0(Seg62Host& host, Object unit) {
    resize_with_default(host, unit + 0x660, 0);
}

// 0095D400 __fastcall(ECX = unit), RET. Vtable slot (00CF8C8C, 00CFCDE4) and
// caller 00819898. An effect at +670h is stopped (00867B10), its byte +9h set,
// its reference released (InterlockedDecrement +4h; vtable[0]() at zero) and
// +670h cleared (the image stores 0 twice); the vector +660h is emptied
// (0095BE70(0)); then a tail jump to 008797B0 with the unit.
void unit_release_effects_0095d400(Seg62Host& host, Object unit) {
    const Object effect = obj(at<std::uint32_t>(unit, 0x670));
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 0x9) = 1;
        const Object held = obj(at<std::uint32_t>(unit, 0x670));
        if (held != nullptr) {
            if (host.interlocked_decrement(held + 0x4) == 0) static_cast<void>(host.vt0(held));
            at<std::uint32_t>(unit, 0x670) = 0;
        }
        at<std::uint32_t>(unit, 0x670) = 0;
    }
    resize_with_default(host, unit + 0x660, 0);
    host.release_damage_state_008797b0(unit);
}

// 0095DB40 __fastcall(ECX = model data), RET. Callers 0095F34F (class base
// destructor), 0095FA1F (BSP_VehicleClass_BindModelData), 0095F173. Erases the
// whole tree +0Ch (head +10h; 0095D2C0 over [[head], head]), frees the head
// (00BF65AC) and clears +10h/+14h; then the camera array at +0 is shrunk to 0
// (005CD640) and its storage freed (00BF6989). SEH 00CA9468 not modelled.
void class_model_data_destruct_0095db40(Seg62Host& host, Object data) {
    const Object tree = data + 0xc;
    const Object head = obj(at<std::uint32_t>(data, 0x10));
    std::uint32_t out_iterator[2];
    host.tree_erase_range_0095d2c0(tree, reinterpret_cast<Object>(out_iterator), tree, obj(at<std::uint32_t>(head, 0x0)),
                                   tree, head);
    host.free_00bf65ac(obj(at<std::uint32_t>(tree, 0x4)));
    at<std::uint32_t>(tree, 0x4) = 0;
    at<std::uint32_t>(tree, 0x8) = 0;
    host.camera_array_resize_005cd640(data, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(data, 0x0)));
}

namespace {

// The image's inlined release of a reference slot: InterlockedDecrement on
// +4h, vtable[0]() at zero, then the slot cleared.
void release_slot(Seg62Host& host, Object owner, std::size_t offset) {
    const Object held = obj(at<std::uint32_t>(owner, offset));
    if (held == nullptr) return;
    if (host.interlocked_decrement(held + 0x4) == 0) static_cast<void>(host.vt0(held));
    at<std::uint32_t>(owner, offset) = 0;
}

// The checked size of the 14h-byte element vector {first +4h, last +8h} as
// the image computes it (IMUL 66666667h, SAR 3: signed division by 14h).
std::uint32_t size_14(Object vector) {
    return static_cast<std::uint32_t>(
        static_cast<std::int32_t>(at<std::uint32_t>(vector, 0x8) - at<std::uint32_t>(vector, 0x4)) / 0x14);
}

}  // namespace

// 0095F1C0 __fastcall(ECX = class), RET. The vehicle class base destructor
// (seventeen callers: the class deleting destructors and unwind funclets).
// Vtables 00CFF7CC at +0 and 00CFF7C8 at +6Ch are restored; the references at
// +10h of each element of the 14h-byte vector +110h are released (checked
// index tests 00BF6713 kept); 005471B0(+94h, 0); the list +128h is emptied
// from the head, each node value first cleared (00955D60 with ECX = value, or
// 0 for a null head) and, when set, destroyed (00955180) and freed; the model
// data +BCh is destroyed (0095DB40) and freed; references +80h, +D4h released;
// 00745200(+128h); the +110h elements destroyed (007480A0) and the storage
// freed, the three words cleared; 00746A00(+E8h); the 10h-byte elements of
// the vector +DCh free their +4h block and clear +4h..+0Ch, then its storage is
// freed and the three words cleared; reference +B4h released; 005471B0(+94h,
// 0) and 00BF6989([+94h]); the native string +74h released to the pool; then
// 0087C700. SEH 00CA9587 not modelled.
void vehicle_class_destruct_base_0095f1c0(Seg62Host& host, Object vehicle_class) {
    at<std::uint32_t>(vehicle_class, 0x0) = 0x00cff7cc;
    at<std::uint32_t>(vehicle_class, 0x6c) = 0x00cff7c8;
    const Object entries = vehicle_class + 0x110;
    for (std::uint32_t i = 0; at<std::uint32_t>(entries, 0x4) != 0 && i < size_14(entries); ++i) {
        if (at<std::uint32_t>(entries, 0x4) == 0 || i >= size_14(entries)) host.invalid_parameter_00bf6713();
        if (at<std::uint32_t>(obj(at<std::uint32_t>(entries, 0x4)), i * 0x14 + 0x10) == 0) continue;
        if (at<std::uint32_t>(entries, 0x4) == 0 || i >= size_14(entries)) host.invalid_parameter_00bf6713();
        const Object reference = obj(at<std::uint32_t>(obj(at<std::uint32_t>(entries, 0x4)), i * 0x14 + 0x10));
        if (host.interlocked_decrement(reference + 0x4) == 0) static_cast<void>(host.vt0(reference));
    }
    host.ref_pointer_array_resize_005471b0(vehicle_class + 0x94, 0);
    const Object list = vehicle_class + 0x128;
    while (at<std::uint32_t>(list, 0x0) != 0) {
        const Object head = obj(at<std::uint32_t>(list, 0x4));
        list_clear_free_values_00955d60(host, head != nullptr ? obj(at<std::uint32_t>(head, 0x8)) : nullptr);
        const Object node = obj(at<std::uint32_t>(list, 0x4));
        const Object value = obj(at<std::uint32_t>(node, 0x8));
        if (value != nullptr) {
            host.destroy_00955180(value);
            host.free_00bf65ac(value);
            at<std::uint32_t>(node, 0x8) = 0;
        }
        if (at<std::uint32_t>(node, 0x0) != 0) {
            at<std::uint32_t>(obj(at<std::uint32_t>(node, 0x0)), 0x4) = at<std::uint32_t>(node, 0x4);
        } else {
            at<std::uint32_t>(list, 0x4) = at<std::uint32_t>(node, 0x4);
        }
        if (at<std::uint32_t>(node, 0x4) != 0) {
            at<std::uint32_t>(obj(at<std::uint32_t>(node, 0x4)), 0x0) = at<std::uint32_t>(node, 0x0);
        } else {
            at<std::uint32_t>(list, 0x8) = at<std::uint32_t>(node, 0x0);
        }
        at<std::uint32_t>(list, 0x0) = at<std::uint32_t>(list, 0x0) - 1;
        host.free_00bf65ac(node);
    }
    const Object model = obj(at<std::uint32_t>(vehicle_class, 0xbc));
    if (model != nullptr) {
        class_model_data_destruct_0095db40(host, model);
        host.free_00bf65ac(model);
        at<std::uint32_t>(vehicle_class, 0xbc) = 0;
    }
    release_slot(host, vehicle_class, 0x80);
    release_slot(host, vehicle_class, 0xd4);
    host.list_clear_nodes_00745200(list);
    if (at<std::uint32_t>(entries, 0x4) != 0) {
        host.destroy_range_007480a0(obj(at<std::uint32_t>(entries, 0x4)), obj(at<std::uint32_t>(entries, 0x8)), entries,
                                    vehicle_class);
        host.free_00bf65ac(obj(at<std::uint32_t>(entries, 0x4)));
    }
    at<std::uint32_t>(entries, 0x4) = 0;
    at<std::uint32_t>(entries, 0x8) = 0;
    at<std::uint32_t>(entries, 0xc) = 0;
    host.ref_member_set_release_00746a00(vehicle_class + 0xe8);
    const std::uint32_t first = at<std::uint32_t>(vehicle_class, 0xdc);
    if (first != 0) {
        const std::uint32_t last = at<std::uint32_t>(vehicle_class, 0xe0);
        if (first != last) {
            for (std::uint32_t element = first; element != last; element += 0x10) {
                const Object e = obj(element);
                if (at<std::uint32_t>(e, 0x4) != 0) host.free_00bf65ac(obj(at<std::uint32_t>(e, 0x4)));
                at<std::uint32_t>(e, 0x4) = 0;
                at<std::uint32_t>(e, 0x8) = 0;
                at<std::uint32_t>(e, 0xc) = 0;
            }
        }
        host.free_00bf65ac(obj(at<std::uint32_t>(vehicle_class, 0xdc)));
    }
    at<std::uint32_t>(vehicle_class, 0xdc) = 0;
    at<std::uint32_t>(vehicle_class, 0xe0) = 0;
    at<std::uint32_t>(vehicle_class, 0xe4) = 0;
    release_slot(host, vehicle_class, 0xb4);
    host.ref_pointer_array_resize_005471b0(vehicle_class + 0x94, 0);
    host.free_00bf6989(obj(at<std::uint32_t>(vehicle_class, 0x94)));
    destroy_string(host, reinterpret_cast<const std::uint32_t*>(vehicle_class + 0x74));
    host.damageable_class_destruct_0087c700(vehicle_class);
}

// 009600E0 __fastcall(ECX = class), RET. Called from the deleting destructor
// 00963283. Releases +14Ch; the points array +140h {data, count +144h,
// capacity +148h}: 0074D190(0) when the capacity word is negative, the count
// stepped down to 0 one by one (no per-element work), cleared, the data freed
// (00BF6989); releases +138h; then 0095F1C0. SEH 00CA95E4 not modelled.
void vehicle_class_destruct_009600e0(Seg62Host& host, Object vehicle_class) {
    release_slot(host, vehicle_class, 0x14c);
    const Object points = vehicle_class + 0x140;
    if (at<std::int32_t>(points, 0x8) < 0) host.points_array_reserve_0074d190(points, 0);
    while (at<std::int32_t>(points, 0x4) > 0) at<std::int32_t>(points, 0x4) = at<std::int32_t>(points, 0x4) - 1;
    at<std::uint32_t>(points, 0x4) = 0;
    host.free_00bf6989(obj(at<std::uint32_t>(points, 0x0)));
    release_slot(host, vehicle_class, 0x138);
    vehicle_class_destruct_base_0095f1c0(host, vehicle_class);
}

// 009601C0 __fastcall(ECX = class), RET. Called from the deleting destructor
// 009632A3. Releases +140h, then 0095F1C0. SEH 00CA95F8 not modelled.
void vehicle_class_destruct_009601c0(Seg62Host& host, Object vehicle_class) {
    release_slot(host, vehicle_class, 0x140);
    vehicle_class_destruct_base_0095f1c0(host, vehicle_class);
}

// 00963F90 __fastcall(ECX = submarine class), RET. Called from the deleting
// destructor 00965353. Releases +80Ch, then the ship class body 00963600.
// SEH 00CA9F68 not modelled.
void submarine_class_destruct_00963f90(Seg62Host& host, Object vehicle_class) {
    release_slot(host, vehicle_class, 0x80c);
    host.ship_class_destruct_00963600(vehicle_class);
}

// 0095E5B0 __fastcall(ECX = unit), RET. BSP_Unit_ReadCommonLuaProperties, the
// state reader paired with 0095BB10 (seven callers, among them
// BSP_AirField_ReadHangarAndMarkerProperties and BSP_Plane_SEntityInitSlotA4). In order: the skill object +38Ch gets vtable[0]([+390h]);
// 0095DE00 attaches the flag node; 0087BF80; 00956C20. With a source record
// [+C0h]:
// - kind 3 (saved data): the saved LuaObject (009238A0) feeds a reader
//   (004425C0); enter "_vehicle" and "_botLevelHandler"; 006E6360 reads the
//   skill (+38Ch); leave; "exitTimer" -> +6F0h (default 90.0f), "lastFireTime"
//   -> +700h and "lastSmokeTime" -> +704h (default -1e10f); "enablePlayerSteer"
//   bool -> +6C8h; "fireState" through the reader's own vtable[10h] into a
//   local int stored at +6E4h; "torpedoDeviation" -> +6D4h; when the key
//   "weaponDirector" exists an 558h-byte gunnery AI is built (00864580; a null
//   allocation leaves 0) into +6DCh and its vtable[4](unit+310h) runs (the
//   image does not test it for null); leave, destroy the reader.
// - kind 1 (property bag [rec+8h]): "DummyObjectID" with tag 0 -> unit
//   vtable[1BCh](value +0Ch); "LaunchClassID" with tag 0 -> +530h;
//   "LaunchEquipment" (no tag test) -> +534h.
// Then +638h = [[+538h]+C8h] (the class launch stock), 00953B80, the +310h
// object's vtable[0Ch](), the unit's vtable[0D8h](), and the node +4A4h gets
// 00B6DA70(1.0f when the byte [00F87152] or +2F0h is set, else +2F4h; 0).
// The literals are those of 0095BB10 plus 00CFC840, 00CE9270, 00D1A98C.
// SEH 00CA9513 not modelled.
void unit_read_common_lua_properties_0095e5b0(Seg62Host& host, Object unit) {
    host.vt0_with(unit + 0x38c, at<std::uint32_t>(unit, 0x390));
    host.attach_flag_node_0095de00(unit);
    host.entity_init_slot_a4_0087bf80(unit);
    host.rebuild_weapon_category_index_00956c20(unit);
    const Object source = obj(at<std::uint32_t>(unit, 0xc0));
    if (source != nullptr && at<std::int32_t>(source, 0x4) == 3) {
        std::uint8_t lua_object[0x14];
        std::uint8_t reader[0x20];
        host.saved_entity_lua_data_009238a0(unit, lua_object);
        host.lua_reader_construct_004425c0(reader, lua_object);
        host.lua_reader_enter_00bd8e20(reader, string_key("_vehicle"));
        host.lua_reader_enter_00bd8e20(reader, string_key("_botLevelHandler"));
        host.skill_level_read_006e6360(unit + 0x38c, reader);
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_read_or_default_00bd68d0(reader, string_key("exitTimer"), TaggedValue{2, addr(unit + 0x6f0)},
                                                 float_value(90.0f));
        host.lua_reader_read_or_default_00bd68d0(reader, string_key("lastFireTime"), TaggedValue{2, addr(unit + 0x700)},
                                                 float_value(-1.0e10f));
        host.lua_reader_read_or_default_00bd68d0(reader, string_key("lastSmokeTime"), TaggedValue{2, addr(unit + 0x704)},
                                                 float_value(-1.0e10f));
        host.lua_reader_read_00bd6830(reader, string_key("enablePlayerSteer"), TaggedValue{3, addr(unit + 0x6c8)});
        std::int32_t fire_state = 0;
        host.reader_vt10_read(reader, string_key("fireState"), TaggedValue{1, addr(&fire_state)});
        at<std::int32_t>(unit, 0x6e4) = fire_state;
        host.lua_reader_read_00bd6830(reader, string_key("torpedoDeviation"), TaggedValue{2, addr(unit + 0x6d4)});
        if (host.lua_reader_has_00bd5eb0(reader, string_key("weaponDirector"))) {
            const Object memory = host.operator_new_00bf681b(0x558);
            const Object ai = memory != nullptr ? host.gunnery_ai_construct_00864580(memory) : nullptr;
            at<std::uint32_t>(unit, 0x6dc) = addr(ai);
            host.vt4_with(ai, unit + 0x310);
        }
        host.lua_reader_leave_00bd7a20(reader);
        host.lua_reader_destruct_00441a20(reader);
    } else if (source != nullptr && at<std::int32_t>(source, 0x4) == 1) {
        const Object dummy = host.property_bag_find_008f2260(obj(at<std::uint32_t>(source, 0x8)), "DummyObjectID");
        if (dummy != nullptr && at<std::int32_t>(dummy, 0x4) == 0) host.vt1bc_with(unit, at<std::uint32_t>(dummy, 0xc));
        const Object launch_class = host.property_bag_find_008f2260(
            obj(at<std::uint32_t>(obj(at<std::uint32_t>(unit, 0xc0)), 0x8)), "LaunchClassID");
        if (launch_class != nullptr && at<std::int32_t>(launch_class, 0x4) == 0) {
            at<std::uint32_t>(unit, 0x530) = at<std::uint32_t>(launch_class, 0xc);
        }
        const Object equipment = host.property_bag_find_008f2260(
            obj(at<std::uint32_t>(obj(at<std::uint32_t>(unit, 0xc0)), 0x8)), "LaunchEquipment");
        if (equipment != nullptr) at<std::uint32_t>(unit, 0x534) = at<std::uint32_t>(equipment, 0xc);
    }
    at<std::uint32_t>(unit, 0x638) = at<std::uint32_t>(obj(at<std::uint32_t>(unit, 0x538)), 0xc8);
    unit_gather_gun_ranges_00953b80(host, unit);
    host.vt0c(unit + 0x310);
    host.vtd8(unit);
    const float factor = (host.global_u8(0x00f87152) != 0 || at<std::uint8_t>(unit, 0x2f0) != 0) ? 1.0f
                                                                                                  : at<float>(unit, 0x2f4);
    host.scene_node_set_visibility_00b6da70(obj(at<std::uint32_t>(unit, 0x4a4)), factor, 0);
}

}  // namespace bsp::cc13_seg62
