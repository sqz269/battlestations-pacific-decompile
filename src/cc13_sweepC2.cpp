// cc13 leftover sweep C2 reconstructions (packet cc13-sweepC2). See include/bsp/cc13_sweepC2.hpp
// for the model and docs/CC13_SWEEPC2.md for the evidence. Names are hypotheses, not recovered
// symbols.

#include "bsp/cc13_sweepC2.hpp"

#include <initializer_list>

namespace bsp::cc13_sweepC2 {

namespace {

// Handle helpers defined further down.
void ref_release(SweepC2Host& host, void* object);
void ref_assign(SweepC2Host& host, void*& slot, void* value);

// The engine's tracked lock: EnterCriticalSection, then the +18h recursion counter, and the
// reverse. A null section is skipped, as in every inlined copy.
void lock_enter(SweepC2Host& host, void* section) {
    if (section != nullptr) {
        host.enter_critical_section(section);
        at<std::int32_t>(section, 0x18) += 1;
    }
}
void lock_leave(SweepC2Host& host, void* section) {
    if (section != nullptr) {
        at<std::int32_t>(section, 0x18) -= 1;
        host.leave_critical_section(section);
    }
}

// Read one dword of an object under its tracked lock at +0Ch (the inlined
// "lock, read, unlock" the image repeats for the air-ops record count +50h).
std::int32_t locked_read(SweepC2Host& host, void* object, std::size_t off) {
    void* const section = ptr(object, 0x0C);
    lock_enter(host, section);
    const std::int32_t value = at<std::int32_t>(object, off);
    lock_leave(host, section);
    return value;
}

// The debug vector range check: no storage, or index not below (last - first) / stride, both
// computed signed and compared unsigned as the image does.
void check_vector_index(SweepC2Host& host, void* vector_first_field, std::uint32_t index,
                        std::int32_t stride) {
    const auto first = at<std::intptr_t>(vector_first_field, 0x00);
    if (first == 0) {
        host.invalid_iterator_00bf6713();
        return;
    }
    const auto last = at<std::intptr_t>(vector_first_field, 0x04);
    if (static_cast<std::uint32_t>(static_cast<std::int32_t>(last - first) / stride) <= index) {
        host.invalid_iterator_00bf6713();
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 006CC800 BSP_AirOpsStockSnapshot_Capture. __thiscall(ECX = snapshot, block), RET 4.
// snapshot+4 = block+58h; snapshot+8 is a vector of {class id, node+0Ch, node+10h} records filled
// from the block's stock list (+44h head, nodes {next, prev, class, +0Ch, +10h}); snapshot+18h is
// a vector of 14h-byte records filled from the 58h-byte slot records at block+4Ch, the count
// block+50h read under the block's tracked lock (+0Ch) once for the resize and again on every
// loop test. The checked-iterator reports (00BF6713) stay at the image's points. The word at
// record+0Ch is cleared, then taken from (slot+28h)->+174h when that pointer is set; record bytes
// +0Eh..+0Fh are not written (the resize value zeroes new records).
// ---------------------------------------------------------------------------
void air_ops_stock_snapshot_capture_006cc800(SweepC2Host& host, void* snapshot, void* block) {
    at<std::uint32_t>(snapshot, 0x04) = at<std::uint32_t>(block, 0x58);
    const std::uint32_t zero12[3] = {0, 0, 0};
    host.vector12_resize_006ca010(add(snapshot, 0x08), at<std::uint32_t>(block, 0x48), zero12);

    std::uint32_t index = 0;
    std::size_t offset = 0;
    for (void* node = ptr(ptr(block, 0x44), 0x00); node != ptr(block, 0x44);
         node = ptr(node, 0x00)) {
        check_vector_index(host, add(snapshot, 0x0C), index, 0x0C);
        void* const record = add(ptr(snapshot, 0x0C), offset);
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
        at<std::uint32_t>(record, 0x00) = host.vehicle_class_registry_id_0095b9c0(ptr(node, 0x08));
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
        at<std::uint32_t>(record, 0x04) = at<std::uint32_t>(node, 0x0C);
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
        at<std::uint32_t>(record, 0x08) = at<std::uint32_t>(node, 0x10);
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
        ++index;
        offset += 0x0C;
    }

    const std::uint32_t count = static_cast<std::uint32_t>(locked_read(host, block, 0x50));
    const std::uint32_t zero20[5] = {0, 0, 0, 0, 0};
    host.vector20_resize_006ca0f0(add(snapshot, 0x18), count, zero20);

    std::int32_t slot = 0;
    std::size_t src_offset = 0;
    std::size_t dst_offset = 0;
    while (slot < locked_read(host, block, 0x50)) {
        void* const src = add(ptr(block, 0x4C), src_offset);
        check_vector_index(host, add(snapshot, 0x1C), static_cast<std::uint32_t>(slot), 0x14);
        void* const record = add(ptr(snapshot, 0x1C), dst_offset);
        at<std::uint32_t>(record, 0x00) = host.vehicle_class_registry_id_0095b9c0(ptr(src, 0x04));
        at<std::uint32_t>(record, 0x04) = at<std::uint32_t>(src, 0x08);
        at<std::uint32_t>(record, 0x08) = at<std::uint32_t>(src, 0x10);
        at<std::uint16_t>(record, 0x0C) = 0;
        if (void* const unit = ptr(src, 0x28); unit != nullptr) {
            at<std::uint16_t>(record, 0x0C) = at<std::uint16_t>(unit, 0x174);
        }
        ++slot;
        src_offset += 0x58;
        dst_offset += 0x14;
        at<std::uint32_t>(record, 0x10) = at<std::uint32_t>(src, 0x2C);
    }
}

// ---------------------------------------------------------------------------
// 006CD810 BSP_AirOps_AiLaunchTick. __thiscall(ECX = block, float dt), RET 4; the caller 006CDD13
// pushes a float (006CDD10 FSTP [ESP]) that the body never reads. SEH frame for the local map.
// Gate: block+F4h set and not (block+F8h > clock) (FCOMIP ST0 = +F8h, ST1 = clock, JA); then
// +F8h = float(clock + 2.0 [00D7A308 double]) is stored before the other gates (+1Ch, +1Dh,
// +7Ch present with +5Dh clear, +38h zero). Slots are the 58h-byte records at +4Ch, count +50h
// (unsigned compares), state at +2Ch.
// One draw from the shared generator per pass of the weighted pick: 00BD2F40 with ECX = 1
// (006CDAA5); a pass restarts (new draw) when the walk reaches the end of the weight map and when
// the picked kind is still owed by the launched-count map.
// ---------------------------------------------------------------------------
void air_ops_ai_launch_tick_006cd810(SweepC2Host& host, void* block, float /*dt_unused*/) {
    if (at<std::uint32_t>(block, 0xF4) == 0) return;
    const float clock = host.fixed_step_clock_00f876a4();
    if (at<float>(block, 0xF8) > clock) return;
    at<float>(block, 0xF8) = static_cast<float>(clock + kAiLaunchInterval);
    if (at<std::uint8_t>(block, 0x1C) != 0) return;
    if (at<std::uint8_t>(block, 0x1D) != 0) return;
    void* const owner = ptr(block, 0x7C);
    if (owner == nullptr) return;
    if (at<std::uint8_t>(owner, 0x5D) != 0) return;
    if (at<std::uint32_t>(block, 0x38) != 0) return;

    // Count the airborne slots of the AI kind; past the quota (+E8h), states 3 go back.
    std::int32_t airborne = 0;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(block, 0x50); ++i) {
        void* const slot = add(ptr(block, 0x4C), i * 0x58);
        if (at<std::uint32_t>(slot, 0x28) == 0) continue;
        const std::int32_t state = at<std::int32_t>(slot, 0x2C);
        if (state != 3 && state != 4) continue;
        if (host.class_kind_vslot_1c(ptr(slot, 0x04)) != static_cast<std::int32_t>(kAiLaunchClassKind)) {
            continue;
        }
        ++airborne;
        if (airborne > at<std::int32_t>(block, 0xE8) && at<std::int32_t>(slot, 0x2C) == 3) {
            host.air_ops_slot_state_from_command_006bd230(block, i, 1);
        }
    }

    // Under quota: the first slot in state 1 or 5 takes every stock node of the AI kind.
    if (airborne < at<std::int32_t>(block, 0xE8)) {
        for (std::uint32_t i = 0; i < at<std::uint32_t>(block, 0x50); ++i) {
            const std::int32_t state = at<std::int32_t>(add(ptr(block, 0x4C), i * 0x58), 0x2C);
            if (state != 1 && state != 5) continue;
            void* node = ptr(ptr(block, 0x44), 0x00);
            void* const end = ptr(block, 0x44);
            while (node != end) {
                if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
                if (host.class_kind_vslot_1c(ptr(node, 0x08)) ==
                        static_cast<std::int32_t>(kAiLaunchClassKind) &&
                    at<std::int32_t>(node, 0x0C) > 0) {
                    const std::int32_t stock = at<std::int32_t>(node, 0x0C);
                    const std::int32_t fill = stock < 3 ? stock : 3;
                    host.air_ops_fill_slot_006c0f00(block, i, ptr(node, 0x08), fill);
                    host.air_ops_slot_state_from_command_006bd230(block, i, 1);
                }
                if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
                node = ptr(node, 0x00);
            }
            return;
        }
    }

    // Otherwise the first free slot (state 1 or 5) launches a weighted random kind; a slot in
    // state 2 ahead of it ends the tick.
    std::uint32_t target = 0;
    {
        std::uint32_t i = 0;
        for (;; ++i) {
            if (i >= at<std::uint32_t>(block, 0x50)) return;
            const std::int32_t state = at<std::int32_t>(add(ptr(block, 0x4C), i * 0x58), 0x2C);
            if (state == 1 || state == 5) break;
            if (state == 2) return;
        }
        target = i;
    }

    // Local std::map<int,int> {allocator, head, size} at ESP+2Ch: launched count per kind.
    struct LocalMap {
        std::uint32_t allocator;
        void* head;
        std::uint32_t size;
    } launched{0, nullptr, 0};
    void* const head = host.tree_buy_head_node_006bfdb0(&launched);
    launched.head = head;
    at<std::uint8_t>(launched.head, 0x15) = 1;
    at<void*>(launched.head, 0x04) = launched.head;
    at<void*>(launched.head, 0x00) = launched.head;
    at<void*>(launched.head, 0x08) = launched.head;
    launched.size = 0;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(block, 0x50); ++i) {
        void* const slot = add(ptr(block, 0x4C), i * 0x58);
        if (at<std::uint32_t>(slot, 0x28) == 0) continue;
        const std::int32_t state = at<std::int32_t>(slot, 0x2C);
        if (state != 3 && state != 4) continue;
        const std::int32_t kind = host.class_kind_vslot_1c(ptr(slot, 0x04));
        std::int32_t* const owed = host.int_map_subscript_006cab40(&launched, &kind);
        *owed += at<std::int32_t>(slot, 0x08);  // base +4Ch as loaded before the calls
    }

    // Weighted pick over the map at block+ECh (head at +F0h; node key +0Ch, weight float +10h).
    void* const weights = add(block, 0xEC);
    TreeIter pick{};
    for (;;) {
        const float roll = host.random_unsigned_fraction_00bd2f40(kAirOpsRandomStream);
        float sum = 0.0f;
        TreeIter it{weights, ptr(ptr(weights, 0x04), 0x00)};
        bool chosen = false;
        for (;;) {
            if (it.container == nullptr || it.container != weights) host.invalid_iterator_00bf6713();
            if (it.node == ptr(weights, 0x04)) break;  // end: draw again
            if (it.container == nullptr) host.invalid_iterator_00bf6713();
            if (it.node == ptr(it.container, 0x04)) host.invalid_iterator_00bf6713();
            sum = sum + at<float>(it.node, 0x10);
            if (sum >= roll) {
                chosen = true;
                break;
            }
            host.tree_iter_inc_006bae90(&it);
        }
        if (!chosen) continue;
        if (it.node == ptr(it.container, 0x04)) host.invalid_iterator_00bf6713();
        const std::int32_t* const kind = &at<std::int32_t>(it.node, 0x0C);
        if (*host.int_map_subscript_006cab40(&launched, kind) > 0) {
            if (it.node == ptr(it.container, 0x04)) host.invalid_iterator_00bf6713();
            *host.int_map_subscript_006cab40(&launched, kind) -= 1;
            continue;
        }
        pick = it;
        break;
    }

    // Fill the target slot from every stock node of the picked kind.
    for (void* node = ptr(ptr(block, 0x44), 0x00);;) {
        if (node == ptr(block, 0x44)) break;
        if (node == ptr(add(block, 0x40), 0x04)) host.invalid_iterator_00bf6713();
        void* const vehicle_class = ptr(node, 0x08);
        if (pick.node == ptr(pick.container, 0x04)) host.invalid_iterator_00bf6713();
        if (host.class_kind_vslot_1c(vehicle_class) == at<std::int32_t>(pick.node, 0x0C) &&
            at<std::int32_t>(node, 0x0C) > 0) {
            const std::int32_t stock = at<std::int32_t>(node, 0x0C);
            const std::int32_t fill = stock < 3 ? stock : 3;
            host.air_ops_fill_slot_006c0f00(block, target, ptr(node, 0x08), fill);
            host.air_ops_slot_state_from_command_006bd230(block, target, 1);
        }
        if (node == ptr(block, 0x44)) host.invalid_iterator_00bf6713();
        node = ptr(node, 0x00);
    }

    // ~map: erase(begin, end) into a scratch iterator, then free the head.
    TreeIter erased{};
    host.tree_erase_range_006c9dc0(&launched, &erased, TreeIter{&launched, ptr(launched.head, 0x00)},
                                   TreeIter{&launched, launched.head});
    host.free_00bf65ac(launched.head);
}

namespace {

// The ref-counted handle release the image inlines: InterlockedDecrement(object+4), and the
// destroy slot vtable[0] (no argument) when it reaches zero.
void ref_release(SweepC2Host& host, void* object) {
    if (object == nullptr) return;
    if (host.interlocked_decrement_00ce2220(&at<std::int32_t>(object, 0x04)) == 0) {
        host.refcounted_destroy_vslot_00(object);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// 006CFAF0 BSP_MotherShipSite_Construct. __thiscall(ECX = site, mothership), RET 4; SEH frame.
// Base 006CF100, vtable 00CF8A58, the elevator platform at +44h (006FC2A0), the observer member
// at +A0h (vtable 00CF89B4, +A4h/+A8h/+ACh/+B4h zero, +B0h byte 1), +B8h = mothership, +BCh = 0.
// The effect at +BCh comes from 008687C0 (ECX = local out slot, EDX = [mothership+4A4h]) with the
// add-ref'd handle (mothership+538h)->+104h, a stack 4x4 identity whose translation row is
// (mothership+538h)->+808h..+810h, and words 1, 0, 0. The child "Sound" (00CEA3FC, built in a
// NativeString by resize(5, 1) and memcpy of length+1 bytes) is found or created on it
// (00866CD0, ECX = +BCh) straight into the by-value argument of 006FC380, with the lift point
// (mothership+538h)+808h, settings+4D8h and settings+4DCh (two 00424C40 calls; the first result
// supplies +4DCh). The owner link {00CF8928, mothership} (operator new 8) goes to 006FBC20;
// +9Ch = 0.0f.
// ---------------------------------------------------------------------------
void* mothership_site_construct_006cfaf0(SweepC2Host& host, void* site, void* mothership) {
    host.air_ops_site_construct_006cf100(site);
    at<std::uint32_t>(site, 0x00) = kMotherShipSiteVtable;
    void* const platform = add(site, 0x44);
    host.elevator_platform_construct_006fc2a0(platform);
    at<std::uint32_t>(site, 0xA4) = 0;
    at<std::uint32_t>(site, 0xA8) = 0;
    at<std::uint32_t>(site, 0xAC) = 0;
    at<std::uint32_t>(site, 0xB4) = 0;
    at<std::uint8_t>(site, 0xB0) = 1;
    at<std::uint32_t>(site, 0xA0) = kSiteObserverMemberVtable;
    at<void*>(site, 0xB8) = mothership;
    at<void*>(site, 0xBC) = nullptr;

    void* const parent = ptr(mothership, 0x4A4);   // 006CFB77 MOV EBP,[ECX+4A4h]; 006CFC40 MOV EDX,EBP
    float matrix[16] = {kUnitScale, 0.0f, 0.0f, 0.0f, 0.0f, kUnitScale, 0.0f, 0.0f,
                        0.0f, 0.0f, kUnitScale, 0.0f, 0.0f, 0.0f, 0.0f, kUnitScale};
    void* const body = ptr(ptr(site, 0xB8), 0x538);
    matrix[12] = at<float>(body, 0x808);
    matrix[13] = at<float>(body, 0x80C);
    matrix[14] = at<float>(body, 0x810);
    void* definition = nullptr;
    if (void* const source = ptr(ptr(mothership, 0x538), 0x104); source != nullptr) {
        definition = source;
        host.interlocked_increment_00ce221c(&at<std::int32_t>(source, 0x04));
    }
    void* created = nullptr;
    void** const result = host.point_effect_create_with_parent_matrix_008687c0(
        &created, parent, definition, matrix, 1, 0, 0);
    void* const effect = *result;
    void* const previous = ptr(site, 0xBC);
    if (previous != effect) {
        at<void*>(site, 0xBC) = effect;
        if (effect != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(effect, 0x04));
        ref_release(host, previous);
    }
    ref_release(host, created);

    // NativeString {length, data} at ESP+18h.
    struct NativeString {
        std::uint32_t length;
        char* data;
    } name{0, nullptr};
    host.native_string_resize_0041dd40(&name, 5, 1);
    if (name.data != nullptr) host.memcpy_00bf7680(name.data, kSiteEffectChildName, name.length + 1);
    void* child = nullptr;  // the by-value argument slot, filled by 00866CD0
    host.point_effect_find_or_create_named_child_00866cd0(ptr(site, 0xBC), &child, &name);
    void* const settings_first = host.game_settings_singleton_00424c40();
    void* const settings_second = host.game_settings_singleton_00424c40();
    host.elevator_platform_setup_006fc380(platform, &at<float>(ptr(ptr(site, 0xB8), 0x538), 0x808),
                                          at<float>(settings_second, 0x4D8),
                                          at<float>(settings_first, 0x4DC), child);
    if (name.data != nullptr) host.string_pool_release_00419cc0_00bd1510(name.data, name.length + 1);

    void* const link = host.operator_new_00bf681b(8);
    if (link != nullptr) {
        at<std::uint32_t>(link, 0x00) = kElevatorOwnerLinkVtable;
        at<void*>(link, 0x04) = mothership;
    }
    host.owned_ptr_assign_006fbc20(platform, link);
    at<float>(site, 0x9C) = 0.0f;
    return site;
}

// ---------------------------------------------------------------------------
// 006D3F20 BSP_AirField_ParkSpotTest. __thiscall(ECX = airfield, float x, float y, float z),
// RET 0Ch; MAirfield vtable slot (00CF8D0C). SEH frame around a stack lock guard (vtable
// 00CE37FC, section = (006D32F0 holder)+4, tracked +18h counter). 006D2980 rebuilds the aim
// points first; then the 0Ch-byte points at +83Ch (count +884h, signed) are tested in the x/z
// plane: dx = float(p.x - x), dz = float(p.z - z) (D8 E2 / D8 E3 from the bytes), d2 =
// float((dx*dx + 0.0) + dz*dz) (the FLDZ;FMUL ST0 term), true when 225.0 (00CF8EB0, double) >
// d2. y is not read. The image keeps the products and sum in x87 extended before the float
// store; the double here rounds the same exact products once less, which can differ only in
// the last bit of a tie.
// ---------------------------------------------------------------------------
bool airfield_park_spot_test_006d3f20(SweepC2Host& host, void* airfield, float x, float /*y*/,
                                      float z) {
    void* const section = ptr(host.airfield_lock_singleton_006d32f0(airfield), 0x04);
    lock_enter(host, section);
    host.airfield_rebuild_aim_points_006d2980(airfield);
    const std::int32_t count = at<std::int32_t>(airfield, 0x884);
    for (std::int32_t i = 0; i < count; ++i) {
        const void* const point = add(airfield, 0x83C + static_cast<std::size_t>(i) * 0x0C);
        const float dx = at<float>(const_cast<void*>(point), 0x00) - x;
        const float dz = at<float>(const_cast<void*>(point), 0x08) - z;
        const float d2 = static_cast<float>(static_cast<double>(dx) * dx + 0.0 +
                                            static_cast<double>(dz) * dz);
        if (kParkSpotRadiusSq > d2) {
            lock_leave(host, section);
            return true;
        }
    }
    lock_leave(host, section);
    return false;
}

// ---------------------------------------------------------------------------
// 008196B0 BSP_UnitInstance_BroadcastRelocate (unit vtable +11Ch). __thiscall(ECX = unit, float
// heading), RET 4; SEH frame for the stack message. When the session globals [00E188A8] have
// +1FE4h == 1 and +218Ch clear and the unit is not a formation follower (007788B0, ECX = unit),
// a kind-97h message (base 0075B430; vtable 00D02F24; +4 = 1, +18h word 0, +1Ah/+1Ch byte 0,
// +20h byte 1, +21h byte 0, +24h heading, +28h = (unit+284h)->+500h or 0) is broadcast through
// 0077C7B0(ECX = unit, &msg, 0) and its vtable reset to 00CE4974. Then +BCCh = 1.25f
// (00CF29A8), 009584D0 sets the heading, 00414DB0 runs unless +C8h is set, 0092D620 syncs the
// motion object (+1018h) from +CCh and 00818EA0 resets the pose state. An occupant owner outside
// session mode 2 then calls vtable +118h (with the address 00F87574) and +D8h on every other
// member of its group (+284h, count +4F8h, 0070D060).
// ---------------------------------------------------------------------------
void unit_broadcast_relocate_008196b0(SweepC2Host& host, void* unit, float heading) {
    void* const globals = *static_cast<void**>(host.image_address(kSessionGlobals));
    if (at<std::int32_t>(globals, 0x1FE4) == 1 && at<std::uint8_t>(globals, 0x218C) == 0 &&
        !host.unit_is_formation_follower_007788b0(unit)) {
        void* const group = ptr(unit, 0x284);
        void* const leader = group != nullptr ? ptr(group, 0x500) : nullptr;
        alignas(4) std::uint8_t message[0x2C];
        host.session_message_construct_base_0075b430(message, kRelocateMessageKind);
        at<std::uint16_t>(message, 0x18) = 0;
        at<std::uint8_t>(message, 0x1A) = 0;
        at<std::uint32_t>(message, 0x04) = 1;
        at<std::uint8_t>(message, 0x1C) = 0;
        at<std::uint32_t>(message, 0x00) = kRelocateMessageVtable;
        at<std::uint8_t>(message, 0x20) = 1;
        at<std::uint8_t>(message, 0x21) = 0;
        at<float>(message, 0x24) = heading;
        at<void*>(message, 0x28) = leader;
        host.session_owner_broadcast_0077c7b0(unit, message, 0);
        at<std::uint32_t>(message, 0x00) = kSessionMessageBaseVtable;
    }
    at<float>(unit, 0xBCC) = kRelocateSettleValue;
    host.unit_set_heading_009584d0(unit, heading);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    host.motion_sync_pose_0092d620(ptr(unit, 0x1018), add(unit, 0xCC));
    unit_reset_pose_dependent_state_00818ea0(host, unit);
    if (host.unit_is_occupant_owner_00778890(unit) &&
        at<std::int32_t>(*static_cast<void**>(host.image_address(kSessionGlobals)), 0x1FE4) != 2) {
        const std::int32_t members = at<std::int32_t>(ptr(unit, 0x284), 0x4F8);
        for (std::int32_t i = 0; i < members; ++i) {
            void* const member = host.unit_group_member_at_0070d060(ptr(unit, 0x284), i);
            if (member == unit) continue;
            host.unit_vslot_118(member, host.image_address(kRelocateMemberArgument));
            host.unit_vslot_d8(member);
        }
    }
}

namespace {

// One effect-restart block of 00818EA0: stop the effect's children (00867B10), set its +9 byte,
// build an identity 4x4 (1.0f 00D7A24C) on the stack, create a new effect (00868420, EDX =
// [globals+19ECh], the add-ref'd handle (unit+538h)+source, 0, 0) into a stack slot, assign it to
// the unit's slot (add-ref new, release old) and release the stack slot.
void restart_effect(SweepC2Host& host, void* unit, void*& slot, std::size_t source) {
    void* const effect = slot;
    if (effect == nullptr) return;
    host.point_effect_stop_children_00867b10(effect);
    at<std::uint8_t>(effect, 0x09) = 1;
    float identity[16];
    for (int i = 0; i < 16; ++i) identity[i] = (i % 5 == 0) ? kUnitScale : 0.0f;
    const std::uint32_t owner =
        at<std::uint32_t>(*static_cast<void**>(host.image_address(kSessionGlobals)), 0x19EC);
    void* handle = nullptr;
    if (void* const definition = ptr(ptr(unit, 0x538), source); definition != nullptr) {
        host.interlocked_increment_00ce221c(&at<std::int32_t>(definition, 0x04));
        handle = definition;
    }
    void* created = nullptr;
    void* const result =
        *host.point_effect_create_from_matrix_00868420(&created, owner, handle, identity, 0, 0);
    ref_assign(host, slot, result);
    ref_release(host, created);
}

}  // namespace

// ---------------------------------------------------------------------------
// 00818EA0 BSP_UnitInstance_ResetPoseDependentState. __thiscall(ECX = unit), RET; SEH frame.
// 00414DB0 twice (each when +C8h is clear); heading +1050h = float(_CIatan2(y = +ECh, x = +F4h))
// (00BF701A, ST1 = y). The point effects are restarted: +9ECh from (unit+538h)+634h, the four
// slots +A00h..+A0Ch from +65Ch, then +9E8h from +630h (restart_effect above). +9F8h = +9FCh =
// 1.0f. With +C8h clear: the parent (+3Ch) refreshes its pose (00414DB0, ECX = parent) when set;
// the world matrix +CCh becomes a copy (004134F0) of the local +74h, or of +74h times the parent's
// +CCh (00413920) when there is a parent; +C8h = 1, +10Ch = 0. Last, the pose history ring
// (+BD0h, 00810020) is refilled from +FCh with vtable +50h's float.
// ---------------------------------------------------------------------------
void unit_reset_pose_dependent_state_00818ea0(SweepC2Host& host, void* unit) {
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    if (at<std::uint8_t>(unit, 0xC8) == 0) host.entity_pose_refresh_world_00414db0(unit);
    const float x = at<float>(unit, 0xCC + 0x28);
    const float y = at<float>(unit, 0xCC + 0x20);
    at<float>(unit, 0x1050) = static_cast<float>(host.crt_atan2_00bf701a(y, x));
    restart_effect(host, unit, at<void*>(unit, 0x9EC), 0x634);
    for (std::size_t i = 0; i < 4; ++i) restart_effect(host, unit, at<void*>(unit, 0xA00 + i * 4), 0x65C);
    restart_effect(host, unit, at<void*>(unit, 0x9E8), 0x630);
    at<float>(unit, 0x9F8) = kUnitScale;
    at<float>(unit, 0x9FC) = kUnitScale;
    if (at<std::uint8_t>(unit, 0xC8) == 0) {
        if (void* const parent = ptr(unit, 0x3C); parent != nullptr) {
            host.entity_pose_refresh_world_00414db0(parent);
        }
        const void* source = add(unit, 0x74);
        float product[16];
        if (void* const parent = ptr(unit, 0x3C); parent != nullptr) {
            source = host.matrix_multiply_00413920(add(unit, 0x74), product, add(parent, 0xCC));
        }
        host.matrix_copy_004134f0(add(unit, 0xCC), source);
        at<std::uint8_t>(unit, 0xC8) = 1;
        at<std::uint8_t>(unit, 0x10C) = 0;
    }
    const float value = host.unit_vslot_50_float(unit);
    host.pose_history_ring_fill_00810020(add(unit, 0xBD0), add(unit, 0xFC), value);
}

// ---------------------------------------------------------------------------
// 0095A880 BSP_Unit_InstantiateClassDevices. __thiscall(ECX = unit, list, byte, mask), RET 0Ch,
// AL = 1. The device class table is the pointer array {data, size} at (unit+538h)+94h.
// Part 1, the explicit list (nodes from list+4, next +4, record +8 = {class index, device id,
// dword, float}): the table grows to index+1 (005471B0) when index >= size (signed); a null
// class or a device id the class's list (+10h) does not contain (009596A0, ECX = list, EDX =
// &record+4) ends part 1; a zero device id skips the node. Each device comes from
// 00443490(ECX = id, DL = 0)->vtable+28h(0); it takes unit +58h/+54h, +38Ch = class index, then
// vtable +1BCh(record+8), +1C0h(record+0Ch float) and +98h(unit, unit+30h, identity matrix
// from 1.0f at 00D7A24C).
// Part 2, when the byte argument is set: every non-null table entry (end re-read each step) is
// counted; with a mask, the entry's bit comes from 0048F930 (ECX = mask, debug iterator checks
// as the image makes them), else it is 1. An existing child (list +48h, next +44h) of kind 1Eh
// (vtable +5Ch) whose +38Ch equals the class +8 skips it, as does a clear bit or a negative class
// +38h; otherwise 00443490(class+38h, 0)->+28h(0) creates it, +5Ch(1Fh) is asked twice with the
// answers discarded, and +58h/+54h/+38Ch (= class+8) and +98h follow as in part 1.
// ---------------------------------------------------------------------------
bool unit_instantiate_class_devices_0095a880(SweepC2Host& host, void* unit, void* list,
                                             std::uint8_t instantiate_class_devices, void* mask) {
    // The image rebuilds the stack identity before every +98h call.
    float identity[16];
    const auto reset_identity = [&identity] {
        for (int i = 0; i < 16; ++i) identity[i] = (i % 5 == 0) ? kUnitScale : 0.0f;
    };
    if (list != nullptr) {
        for (void* node = ptr(list, 0x04); node != nullptr; node = ptr(node, 0x04)) {
            const std::int32_t index = at<std::int32_t>(ptr(node, 0x08), 0x00);
            void* const table = add(ptr(unit, 0x538), 0x94);
            if (index >= at<std::int32_t>(table, 0x04)) {
                host.pointer_array_resize_005471b0(table, index + 1);
            }
            void* const device_class = at<void**>(table, 0x00)[index];
            if (device_class == nullptr) break;
            const auto* const device_id = &at<std::uint32_t>(ptr(node, 0x08), 0x04);
            if (*device_id == 0) continue;
            if (!host.int_list_contains_009596a0(add(device_class, 0x10), device_id)) break;
            void* const factory =
                host.device_class_resolve_00443490(at<std::uint32_t>(ptr(node, 0x08), 0x04), 0);
            void* const device = host.device_class_create_vslot_28(factory, 0);
            at<std::uint32_t>(device, 0x58) = at<std::uint32_t>(unit, 0x58);
            at<std::uint32_t>(device, 0x54) = at<std::uint32_t>(unit, 0x54);
            at<std::uint32_t>(device, 0x38C) = at<std::uint32_t>(ptr(node, 0x08), 0x00);
            host.device_vslot_1bc(device, at<std::uint32_t>(ptr(node, 0x08), 0x08));
            host.device_vslot_1c0(device, at<float>(ptr(node, 0x08), 0x0C));
            reset_identity();
            host.device_attach_vslot_98(device, unit, at<std::uint32_t>(unit, 0x30), identity);
        }
    }

    if (instantiate_class_devices == 0) return true;
    std::uint32_t counter = 0;
    void** entry = at<void**>(add(ptr(unit, 0x538), 0x94), 0x00);
    for (;;) {
        void* const table = add(ptr(unit, 0x538), 0x94);
        if (entry == at<void**>(table, 0x00) + at<std::int32_t>(table, 0x04)) break;
        void* const device_class = *entry;
        if (device_class != nullptr) {
            bool enabled = true;
            if (mask != nullptr) {
                std::uint32_t iterator_storage[3];
                void* const it = host.bool_vector_at_0048f930(mask, iterator_storage, counter);
                if (ptr(it, 0x00) == nullptr || ptr(it, 0x04) == nullptr) {
                    host.invalid_iterator_00bf6713();
                }
                void* const container = ptr(it, 0x00);
                const auto first = at<std::uintptr_t>(container, 0x08);
                const std::uint32_t bit = at<std::uint32_t>(it, 0x08);
                if (first > at<std::uintptr_t>(container, 0x0C)) host.invalid_iterator_00bf6713();
                const std::uint32_t position =
                    (static_cast<std::uint32_t>(
                         static_cast<std::int32_t>(at<std::uintptr_t>(it, 0x04) - first) >> 2)
                     << 5) + bit;
                if (!(position < at<std::uint32_t>(ptr(it, 0x00), 0x00))) {
                    host.invalid_iterator_00bf6713();
                }
                enabled = (*at<std::uint32_t*>(it, 0x04) &
                           (1u << (at<std::uint32_t>(it, 0x08) & 31))) != 0;
            }
            ++counter;
            bool present = false;
            for (void* child = ptr(unit, 0x48); child != nullptr; child = ptr(child, 0x44)) {
                if (host.object_is_kind_of_vslot_5c(child, 0x1E) &&
                    at<std::uint32_t>(child, 0x38C) == at<std::uint32_t>(*entry, 0x08)) {
                    present = true;
                    break;
                }
            }
            if (!present && enabled && at<std::int32_t>(*entry, 0x38) >= 0) {
                void* const factory = host.device_class_resolve_00443490(
                    static_cast<std::uint32_t>(at<std::int32_t>(*entry, 0x38)), 0);
                void* const device = host.device_class_create_vslot_28(factory, 0);
                host.object_is_kind_of_vslot_5c(device, 0x1F);
                host.object_is_kind_of_vslot_5c(device, 0x1F);
                at<std::uint32_t>(device, 0x58) = at<std::uint32_t>(unit, 0x58);
                at<std::uint32_t>(device, 0x54) = at<std::uint32_t>(unit, 0x54);
                at<std::uint32_t>(device, 0x38C) = at<std::uint32_t>(*entry, 0x08);
                reset_identity();
            host.device_attach_vslot_98(device, unit, at<std::uint32_t>(unit, 0x30), identity);
            }
        }
        ++entry;
    }
    return true;
}

namespace {

// A by-value NativeString temporary {length, data}; released through the string pool with
// length+1 bytes when data is set, as every inlined destructor does.
struct NativeStringTemp {
    std::uint32_t length;
    char* data;
};
void release_temp(SweepC2Host& host, NativeStringTemp& text) {
    if (text.data != nullptr) host.string_pool_release_00419cc0_00bd1510(text.data, text.length + 1);
}

}  // namespace

// ---------------------------------------------------------------------------
// 0096EDD0 native-string token replace. __thiscall(ECX = never read, text, token, insert),
// RET 0Ch; SEH frame over four stack temporaries. The ECX role the cc13_large62 hook gives it
// (the manager) is the caller's; the body ignores ECX. With text data and length and token data
// set, pos = strcspn(text, token); unless pos is the text length or -1, the text becomes
// text[0, pos) + insert + text[pos + token.length, 7FFFFFFFh) (00469840 then 004261A0 twice),
// copied back by resize(length, 1) (0041DD40) and memcpy of text.length bytes when the result
// is non-empty; the temporaries are released newest first.
// ---------------------------------------------------------------------------
void native_string_replace_token_0096edd0(SweepC2Host& host, void* /*unused_this*/, void* text,
                                          const void* token, const void* insert) {
    char* const text_data = at<char*>(text, 0x04);
    const char* const token_data = at<char*>(const_cast<void*>(token), 0x04);
    if (text_data == nullptr || at<std::uint32_t>(text, 0x00) == 0 || token_data == nullptr) return;
    const std::uint32_t pos = host.strcspn_00bf9a50(text_data, token_data);
    if (pos == at<std::uint32_t>(text, 0x00) || pos == 0xFFFFFFFFu) return;

    NativeStringTemp tail{}, head{}, joined{}, result{};
    void* const tail_ref = host.native_string_substring_00469840(
        text, &tail, at<std::uint32_t>(const_cast<void*>(token), 0x00) + pos, 0x7FFFFFFF);
    void* const head_ref = host.native_string_substring_00469840(text, &head, 0, pos);
    void* const joined_ref = host.native_string_concat_004261a0(head_ref, &joined, insert);
    auto* const final_ref =
        static_cast<NativeStringTemp*>(host.native_string_concat_004261a0(joined_ref, &result, tail_ref));
    if (text != final_ref) {
        host.native_string_resize_0041dd40(text, final_ref->length, 1);
        if (final_ref->length != 0) {
            host.memcpy_00bf7680(at<char*>(text, 0x04), final_ref->data, at<std::uint32_t>(text, 0x00));
        }
    }
    release_temp(host, result);
    release_temp(host, joined);
    release_temp(host, head);
    release_temp(host, tail);
}

namespace {

void* global_pointer(SweepC2Host& host, std::uint32_t address) {
    return *static_cast<void**>(host.image_address(address));
}

}  // namespace

// ---------------------------------------------------------------------------
// 00A38DA0 BSP_Session_SpawnPlayerVehicles. __fastcall(CL = banned), RET; aligned frame with SEH.
// Banned: [00F8AB6A] = 1, 00652A50(1) and 00652A40(1) on (interface [00E198C4])+BCh, then
// 0068B3F0 on the interface; only outside session mode 2 ([00E188A8]+1FE4h) and in effective
// game mode 4 (004BCA50) the eight player slots are refilled: a slot is taken when its value
// (00946970 on [00F89B3C], ST0) is > 0 (FCOMIP/JBE: NaN counts as free) or a player in the list
// ((globals+19CCh)+34h, next +4, player +8) reports it (009FFD20, index <= 7 unsigned). For each
// free slot i whose record [globals+18CCh+4i] has +8 set and (+9 clear or +0Ah set): a local
// list (head 006599C0) is filled by 0066E590(ECX = record+28h, EDX = &list, i); when it is not
// empty and its first node's nested list (+10h head, +14h size) is not empty, the nested first
// node's +8 class builds a supply list (0084D560, ECX = class, record+28h, 0); when that is not
// empty its first +8 resolves (00964790, DL = 1) to the object passed with 3, 0 and an empty
// NativeString ("" 00CE3A0C via 0041E870) to 009FFF60, and 0094C900 gets (i, object, class, 3,
// that answer). The supply list is cleared (004E8150); the local list is cleared (006654C0) and
// its head freed. Not banned: [00F8AB6A] = 0 and the two setters with 0, nothing else.
// The temporaries are zero-initialised here; the image leaves them to their constructors.
// ---------------------------------------------------------------------------
void session_spawn_player_vehicles_00a38da0(SweepC2Host& host, bool banned) {
    if (!banned) {
        *static_cast<std::uint8_t*>(host.image_address(kSupportManagerBanned)) = 0;
        host.interface_flag_a_00652a50(ptr(global_pointer(host, kInGameInterface), 0xBC), 0);
        host.interface_flag_b_00652a40(ptr(global_pointer(host, kInGameInterface), 0xBC), 0);
        return;
    }
    *static_cast<std::uint8_t*>(host.image_address(kSupportManagerBanned)) = 1;
    host.interface_flag_a_00652a50(ptr(global_pointer(host, kInGameInterface), 0xBC), 1);
    host.interface_flag_b_00652a40(ptr(global_pointer(host, kInGameInterface), 0xBC), 1);
    host.ingame_interface_collapse_0068b3f0(global_pointer(host, kInGameInterface));
    if (at<std::int32_t>(global_pointer(host, kSessionGlobals), 0x1FE4) == 2) return;
    if (host.game_effective_mode_004bca50() != 4) return;

    std::uint8_t taken[8];
    for (std::int32_t i = 0; i < 8; ++i) {
        const float value = host.player_slot_value_00946970(global_pointer(host, kPlayerSlotManager), i);
        taken[i] = value > 0.0f ? 1 : 0;
    }
    for (void* node = ptr(ptr(global_pointer(host, kSessionGlobals), 0x19CC), 0x34); node != nullptr;
         node = ptr(node, 0x04)) {
        const std::uint32_t slot = host.unit_loss_counting_slot_009ffd20(ptr(node, 0x08));
        if (slot <= 7) taken[slot] = 1;
    }

    for (std::int32_t i = 0; i < 8; ++i) {
        if (taken[i] != 0) continue;
        void* const record =
            ptr(global_pointer(host, kSessionGlobals), 0x18CC + static_cast<std::size_t>(i) * 4);
        if (at<std::uint8_t>(record, 0x08) == 0) continue;
        if (at<std::uint8_t>(record, 0x09) != 0 && at<std::uint8_t>(record, 0x0A) == 0) continue;

        struct ListObject {
            std::uint32_t allocator;
            void* head;
            std::uint32_t size;
        } list{0, nullptr, 0};
        list.head = host.list_buy_head_node_006599c0(&list);
        list.size = 0;
        host.slot_fill_list_0066e590(ptr(record, 0x28), &list, i);
        if (list.size != 0) {
            void* const first = ptr(list.head, 0x00);
            if (first == list.head) host.invalid_iterator_00bf6713();
            if (at<std::uint32_t>(first, 0x14) != 0) {
                void* const nested_head = ptr(first, 0x10);
                void* const nested_first = ptr(nested_head, 0x00);
                if (nested_first == nested_head) host.invalid_iterator_00bf6713();
                void* const source = ptr(record, 0x28);
                void* const vehicle_class = ptr(nested_first, 0x08);
                ListObject supply{0, nullptr, 0};
                host.support_site_build_supply_class_list_0084d560(vehicle_class, &supply, source, 0);
                if (supply.size != 0) {
                    void* const supply_first = ptr(supply.head, 0x00);
                    if (supply_first == supply.head) host.invalid_iterator_00bf6713();
                    void* const object = host.resolve_00964790(ptr(supply_first, 0x08), 1);
                    NativeStringTemp name{0, nullptr};
                    host.native_string_assign_0041e870(&name, kEmptyText);
                    const std::uint32_t answer = host.object_query_009fff60(object, 3, 0, &name);
                    host.player_spawn_0094c900(global_pointer(host, kPlayerSlotManager), i, object,
                                               vehicle_class, 3, answer);
                    release_temp(host, name);
                }
                host.list_clear_004e8150(&supply);
            }
        }
        host.list_clear_nested_006654c0(&list);
        host.free_00bf65ac(list.head);
        list.head = nullptr;
    }
}

namespace {

// The VC9 std::string {allocator, _Bx[16], _Mysize, _Myres} at an offset: c_str() picks the heap
// pointer once _Myres >= 10h.
const char* std_string_data(void* string) {
    return at<std::uint32_t>(string, 0x18) >= 0x10 ? at<const char*>(string, 0x04)
                                                   : &at<const char>(string, 0x04);
}

}  // namespace

// ---------------------------------------------------------------------------
// 00A46930 BSP_XLiveSystem_JoinPlayer. __thiscall(ECX = system, player*, invited), RET 8.
// player = {xuid lo, xuid hi, std::string name at +8}. The player list at +168h (head +16Ch;
// node {next, prev, xuid lo +8, hi +0Ch, std::string +10h}) is searched with vtable +19Ch asked
// once up front: when it answers true a node with the same xuid also runs the name compare
// (004BEB60(0, node size, name, name size) on the node string); either way an xuid match marks
// it found (the image's || falls through to the plain xuid test when the compare differs). A
// found flag stops the walk at the next step. Not found: a node is bought before the head
// (00A456D0), the size grows (00A45150) and linked, then 00A45C60 refreshes friends.
// Then, unless +19Ch answers true again: local = xuid equals the active xuid (00A3EB00 on
// [00F8ABE8]) or is zero; with session flag [00E188A8]+2078h set a non-local player joins through
// 00A49B90(system+2098h); without it the client ([globals+207Ch]+C8h) joins local (00A39C50) or
// remote (00A396E0). Finally a signed-in manager in state 2 whose active xuid equals the player
// requests the download storage (00A3F500). The debug iterator checks are kept.
// ---------------------------------------------------------------------------
void xlive_system_join_player_00a46930(SweepC2Host& host, void* system, void* player,
                                       std::uint32_t invited) {
    const bool compare_names = host.xlive_vslot_19c(system);
    void* const list = add(system, 0x168);
    void* const head = ptr(list, 0x04);
    bool found = false;
    bool stopped = false;
    for (void* node = ptr(head, 0x00);;) {
        if (node == head) break;
        if (found) {
            stopped = true;
            break;
        }
        bool match = false;
        if (compare_names) {
            if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();
            if (at<std::uint32_t>(node, 0x08) == at<std::uint32_t>(player, 0x00) &&
                at<std::uint32_t>(node, 0x0C) == at<std::uint32_t>(player, 0x04)) {
                if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();
                void* const name = add(player, 0x08);
                if (host.string_compare_004beb60(add(node, 0x10), 0, at<std::uint32_t>(node, 0x24),
                                                 std_string_data(name),
                                                 at<std::uint32_t>(name, 0x14)) == 0) {
                    match = true;
                }
            }
        }
        if (!match) {
            if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();
            match = at<std::uint32_t>(node, 0x08) == at<std::uint32_t>(player, 0x00) &&
                    at<std::uint32_t>(node, 0x0C) == at<std::uint32_t>(player, 0x04);
        }
        if (match) found = true;
        if (node == ptr(list, 0x04)) host.invalid_iterator_00bf6713();
        node = ptr(node, 0x00);
    }
    if (!stopped && !found) {
        void* const list_head = ptr(list, 0x04);
        void* const node = host.player_list_buy_node_00a456d0(list, list_head, ptr(list_head, 0x04), player);
        host.player_list_incsize_00a45150(list, 1);
        at<void*>(list_head, 0x04) = node;
        at<void*>(ptr(node, 0x04), 0x00) = node;
        host.xlive_refresh_friends_00a45c60(system);
    }

    if (host.xlive_vslot_19c(system)) return;
    const std::uint32_t lo = at<std::uint32_t>(player, 0x00);
    const std::uint32_t hi = at<std::uint32_t>(player, 0x04);
    const std::uint64_t active = host.xenon_active_xuid_00a3eb00(global_pointer(host, kXenonSystemManager));
    bool local = false;
    if ((lo == static_cast<std::uint32_t>(active) && hi == static_cast<std::uint32_t>(active >> 32)) ||
        (lo | hi) == 0) {
        local = true;
    }
    const std::uint64_t xuid = (static_cast<std::uint64_t>(hi) << 32) | lo;
    void* const globals = global_pointer(host, kSessionGlobals);
    if (at<std::uint32_t>(globals, 0x2078) != 0) {
        if (!local) host.xlive_session_join_remote_00a49b90(add(system, 0x2098), &xuid, invited);
    } else if (local) {
        host.network_client_join_local_00a39c50(add(ptr(globals, 0x207C), 0xC8), invited);
    } else {
        host.network_client_join_remote_00a396e0(add(ptr(globals, 0x207C), 0xC8), &xuid, invited);
    }
    void* const manager = global_pointer(host, kXenonSystemManager);
    if (!host.xenon_has_selected_user_00a3e510(manager)) return;
    if (host.xenon_selected_sign_in_state_00a3ead0(manager) != 2) return;
    if (host.xenon_active_xuid_00a3eb00(global_pointer(host, kXenonSystemManager)) != xuid) return;
    host.xlive_request_download_storage_00a3f500(global_pointer(host, kXenonSystemManager));
}

namespace {

// The inlined NativeString copy-assign: unless dest is src, resize(dest, src.length, 1), then
// memcpy dest.length bytes when src is non-empty.
void native_string_copy(SweepC2Host& host, void* dest, const void* src) {
    if (dest == src) return;
    void* const source = const_cast<void*>(src);
    host.native_string_resize_0041dd40(dest, at<std::uint32_t>(source, 0x00), 1);
    if (at<std::uint32_t>(source, 0x00) != 0) {
        host.memcpy_00bf7680(at<char*>(dest, 0x04), at<char*>(source, 0x04),
                             at<std::uint32_t>(dest, 0x00));
    }
}

// The inlined ref-counted handle assignment: store, add-ref the new handle, release the old.
void ref_assign(SweepC2Host& host, void*& slot, void* value) {
    void* const previous = slot;
    if (previous == value) return;
    slot = value;
    if (value != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(value, 0x04));
    ref_release(host, previous);
}

// Whether the channel's stream (+4) currently plays `name`: CopyName into a stack temporary,
// EqualsInsensitive, release the temporary. No stream answers false without either call.
bool stream_plays(SweepC2Host& host, void* channel, const void* name) {
    void* const stream = ptr(channel, 0x04);
    if (stream == nullptr) return false;
    NativeStringTemp current{0, nullptr};
    void* const copied = host.sound_stream_copy_name_00a77ff0(stream, &current);
    const bool equal = host.native_string_equals_insensitive_00435c40(copied, name);
    release_temp(host, current);
    return equal;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00A78330 BSP_StreamedDialogManager_SetChannelVolumeByName. __thiscall(ECX = channel, name,
// float volume), RET 8. When the channel's stream (+4) plays `name` (00A77FF0 / 00435C40), its
// gain becomes float(vtable[0]() * volume) (00A864F0); otherwise, with +4Ch set and the pending
// record (+10h) named `name`, the pending volume +24h takes `volume`.
// ---------------------------------------------------------------------------
void dialog_channel_set_volume_by_name_00a78330(SweepC2Host& host, void* channel,
                                                const void* name, float volume) {
    if (stream_plays(host, channel, name)) {
        const float gain = host.channel_volume_vslot_00(channel) * volume;
        host.sound_stream_set_gain_00a864f0(ptr(channel, 0x04), gain);
        return;
    }
    if (at<std::uint8_t>(channel, 0x4C) != 0 &&
        host.native_string_equals_insensitive_00435c40(add(channel, 0x10), name)) {
        at<float>(channel, 0x24) = volume;
    }
}

// ---------------------------------------------------------------------------
// 00A786F0 BSP_StreamedDialogManager_RequestMusic. __thiscall(ECX = director, name, handle by
// value, float fade), RET 0Ch; SEH frame owning the handle. Logs "New music requested %s at %d"
// (00D58ED0) with the name (or the empty text 00F8BBD0) and +50h. fade > 0 (COMISS/JBE, NaN
// takes the else arm): +2Ch = fade, record +30h takes the name (00A78230), +48h takes the
// handle, +4Dh = 1, +4Ch = 0, and the +10h record's references are cleared (00A77BB0);
// otherwise +2Ch = 0, record +10h takes the name, +28h the handle, +4Dh = 0, +4Ch = 1 and +30h
// is cleared. The by-value handle is released last.
// ---------------------------------------------------------------------------
void dialog_request_music_00a786f0(SweepC2Host& host, void* director, const void* name,
                                   void* handle, float fade) {
    void* const source = const_cast<void*>(name);
    const char* text = at<const char*>(source, 0x04);
    if (text == nullptr) text = static_cast<const char*>(host.image_address(kEmptyNativeText));
    host.log_printf_text_int_004254b0(kNewMusicFormat, text, at<std::int32_t>(director, 0x50));
    void* cleared = nullptr;
    if (fade > 0.0f) {
        at<float>(director, 0x2C) = fade;
        host.logical_dialog_record_assign_00a78230(add(director, 0x30), name);
        ref_assign(host, at<void*>(director, 0x48), handle);
        at<std::uint8_t>(director, 0x4D) = 1;
        at<std::uint8_t>(director, 0x4C) = 0;
        cleared = add(director, 0x10);
    } else {
        at<float>(director, 0x2C) = 0.0f;
        host.logical_dialog_record_assign_00a78230(add(director, 0x10), name);
        ref_assign(host, at<void*>(director, 0x28), handle);
        at<std::uint8_t>(director, 0x4D) = 0;
        at<std::uint8_t>(director, 0x4C) = 1;
        cleared = add(director, 0x30);
    }
    host.logical_dialog_configuration_clear_00a77bb0(cleared);
    ref_release(host, handle);
}

// ---------------------------------------------------------------------------
// 00A788A0 BSP_MusicDirector_SetLevel. __thiscall(ECX = manager, level), RET 4. A level equal to
// +208h returns at once; otherwise +208h = level and the record is +10h + (party*9 + level)*18h
// with party = +210h. An empty record (length 0) logs "MUSIC NOT FOUND %d %d" (00D58EF0) with
// party, level. When the director ([+8]) stream already plays the record's name, 00A782E0 takes
// record+8; otherwise the director's RequestMusic (00A786F0) gets the record, the add-ref'd
// handle +1C0h + (party*9 + level)*4 and fade 0.0f.
// ---------------------------------------------------------------------------
void music_director_set_level_00a788a0(SweepC2Host& host, void* manager, std::int32_t level) {
    if (at<std::int32_t>(manager, 0x208) == level) return;
    const std::int32_t party = at<std::int32_t>(manager, 0x210);
    at<std::int32_t>(manager, 0x208) = level;
    void* const record = add(manager, 0x10 + static_cast<std::size_t>(party * 9 + level) * 0x18);
    if (at<std::uint32_t>(record, 0x00) == 0) {
        host.log_printf_int_int_004254b0(kMusicNotFoundFormat, party, level);
        return;
    }
    if (stream_plays(host, ptr(manager, 0x08), record)) {
        host.music_director_select_row_00a782e0(ptr(manager, 0x08), add(record, 0x08));
        return;
    }
    const std::int32_t slot = at<std::int32_t>(manager, 0x210) * 9 + at<std::int32_t>(manager, 0x208);
    void* const handle = at<void*>(manager, 0x1C0 + static_cast<std::size_t>(slot) * 4);
    if (handle != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(handle, 0x04));
    dialog_request_music_00a786f0(host, ptr(manager, 0x08), record, handle, 0.0f);
}

// ---------------------------------------------------------------------------
// 00A78CE0 BSP_StreamedDialogManager_PlayDialogStream. __thiscall(ECX = manager, name, float),
// RET 8; SEH frame. The float argument is never read. A stack dialog record {NativeString,
// three zero dwords, 1.0f (00D7A24C)} takes a copy of the name, then the director (+0Ch)
// RequestMusic gets it with the add-ref'd handle +22Ch and fade 0.0f; the record is destroyed
// (004C87F0).
// ---------------------------------------------------------------------------
void dialog_play_stream_00a78ce0(SweepC2Host& host, void* manager, const void* name,
                                 float /*unused*/) {
    alignas(4) std::uint8_t record[0x18] = {};
    at<float>(record, 0x14) = kUnitScale;
    native_string_copy(host, record, name);
    void* const handle = ptr(manager, 0x22C);
    if (handle != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(handle, 0x04));
    dialog_request_music_00a786f0(host, ptr(manager, 0x0C), record, handle, 0.0f);
    host.streamed_dialog_record_destroy_004c87f0(record);
}

// ---------------------------------------------------------------------------
// 00A78EC0 BSP_DialogChannelRecords_Resize. __thiscall(ECX = records {data, size, capacity},
// count), RET 4; SEH frame. count > capacity (signed) reserves (00A78DC0); new 14h-byte records
// from size to count are constructed (00A77F80, skipped for a null address); surplus records are
// dropped from the end, each releasing its NativeString (+0 length, +4 data); size = count.
// ---------------------------------------------------------------------------
void dialog_channel_records_resize_00a78ec0(SweepC2Host& host, void* records,
                                            std::int32_t count) {
    if (count > at<std::int32_t>(records, 0x08)) {
        host.dialog_channel_records_reserve_00a78dc0(records, count);
    }
    for (std::int32_t i = at<std::int32_t>(records, 0x04); i < count; ++i) {
        void* const record = add(ptr(records, 0x00), static_cast<std::size_t>(i) * 0x14);
        if (record != nullptr) host.dialog_channel_record_construct_00a77f80(record);
    }
    while (count < at<std::int32_t>(records, 0x04)) {
        at<std::int32_t>(records, 0x04) -= 1;
        void* const record =
            add(ptr(records, 0x00), static_cast<std::size_t>(at<std::int32_t>(records, 0x04)) * 0x14);
        if (char* const data = at<char*>(record, 0x04); data != nullptr) {
            host.string_pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(record, 0x00) + 1);
        }
    }
    at<std::int32_t>(records, 0x04) = count;
}

// ---------------------------------------------------------------------------
// 00A79480 BSP_StreamedDialogManager_SetSlotRecord. __thiscall(ECX = manager, path, party,
// state), RET 0Ch; SEH frame. slot = party*9 + state: record +10h + slot*18h takes the path
// (00A78230). An empty path releases the table handle +1C0h + slot*4 and zeroes it. Otherwise
// the path minus its last four characters (00469840(0, length-4)) plus ".def" (00CF779C, built
// by 0041E870) is copied into a stack NativeString, a 20h-byte table is built from it
// (operator new, 00A79150; null when the allocation fails), the old handle is released and the
// slot takes the new table without an add-ref. Temporaries are released as the image orders them.
// ---------------------------------------------------------------------------
void dialog_set_slot_record_00a79480(SweepC2Host& host, void* manager, const void* path,
                                     std::int32_t party, std::int32_t state) {
    const std::int32_t slot = party * 9 + state;
    host.logical_dialog_record_assign_00a78230(add(manager, 0x10 + static_cast<std::size_t>(slot) * 0x18),
                                               path);
    void*& table_slot = at<void*>(manager, 0x1C0 + static_cast<std::size_t>(slot) * 4);
    NativeStringTemp def_name{0, nullptr};
    if (at<std::uint32_t>(const_cast<void*>(path), 0x00) == 0) {
        ref_release(host, table_slot);
        table_slot = nullptr;
        release_temp(host, def_name);
        return;
    }
    NativeStringTemp copy{0, nullptr};
    native_string_copy(host, &copy, path);
    NativeStringTemp extension{0, nullptr};
    host.native_string_assign_0041e870(&extension, kDefExtension);
    NativeStringTemp stem{0, nullptr};
    NativeStringTemp joined{0, nullptr};
    void* const stem_ref = host.native_string_substring_00469840(&copy, &stem, 0, copy.length - 4);
    void* const joined_ref = host.native_string_concat_004261a0(stem_ref, &joined, &extension);
    native_string_copy(host, &def_name, joined_ref);
    release_temp(host, joined);
    release_temp(host, stem);
    release_temp(host, extension);
    void* const memory = host.operator_new_00bf681b(0x20);
    void* const table =
        memory != nullptr ? host.sound_stream_table_construct_00a79150(memory, &def_name) : nullptr;
    if (table_slot != nullptr) {
        ref_release(host, table_slot);
        table_slot = nullptr;
    }
    table_slot = table;
    release_temp(host, copy);
    release_temp(host, def_name);
}

// ---------------------------------------------------------------------------
// 00A90530 BSP_InputBackend_ReadEvent. __thiscall(ECX = backend, group*, zero*, control*,
// value*), RET 10h, aligned frame. Outputs start at -1, 0, -1, 0.0f. For each of the three
// device vectors at +70h (stride 24h, {first, last}): a non-empty vector's first device answers
// its active control (vtable +30h); unless -1, the control's value (vtable +24h, ST0 spilled to
// float) gives mag = v > 0 ? v : -0.0f - v (FCOMIP/JBE: NaN takes the subtraction), and when
// double(mag) > |*value| (0042BE90) the outputs become (group, 0, control, v).
// ---------------------------------------------------------------------------
bool input_backend_read_event_00a90530(SweepC2Host& host, void* backend, std::int32_t* group,
                                       std::int32_t* zero, std::int32_t* control, float* value) {
    *group = -1;
    *zero = 0;
    *control = -1;
    *value = 0.0f;
    for (std::int32_t i = 0; i < 3; ++i) {
        void* const devices = add(backend, 0x70 + static_cast<std::size_t>(i) * 0x24);
        const auto first = at<std::intptr_t>(devices, 0x00);
        if (first == 0) continue;
        const std::int32_t count =
            static_cast<std::int32_t>(at<std::intptr_t>(devices, 0x04) - first) >> 2;
        if (count == 0) continue;
        void* const device = *at<void**>(devices, 0x00);
        if (device == nullptr) continue;
        const std::int32_t active = host.input_device_active_control_vslot_30(device);
        if (active == -1) continue;
        const float v = host.input_device_control_value_vslot_24(device, active);
        const float magnitude = v > 0.0f ? v : kNegativeZero - v;
        if (static_cast<double>(magnitude) > static_cast<double>(host.fabs_0042be90(value))) {
            *group = i;
            *zero = 0;
            *control = active;
            *value = v;
        }
    }
    return *control != -1;
}

// ---------------------------------------------------------------------------
// 00A93690 BSP_InputAction_RemoveMatchingBinding. __thiscall(ECX = action, descriptor*), RET 4.
// Bindings are 34h-byte records at +10h, count +14h. The first whose +4/+8/+10h equal the
// descriptor's words 0/1/3 is the target; a miss leaves the target one past the end. Unless the
// target is the last record, the last is copied over it (bytes +0/+1, dwords +4..+14h, the two
// modifier vectors +18h/+24h through 00A92EE0, float +30h); then the last record is destroyed
// (00A93070) and the count drops by one. No empty-array or miss guard, as in the image.
// ---------------------------------------------------------------------------
void input_action_remove_matching_binding_00a93690(SweepC2Host& host, void* action,
                                                   const void* descriptor) {
    void* const words = const_cast<void*>(descriptor);
    auto* const base = static_cast<std::uint8_t*>(ptr(action, 0x10));
    std::uint8_t* target = base;
    std::uint8_t* const end = base + at<std::int32_t>(action, 0x14) * 0x34;
    for (; target != end; target += 0x34) {
        if (at<std::uint32_t>(words, 0x00) == at<std::uint32_t>(target, 0x04) &&
            at<std::uint32_t>(words, 0x04) == at<std::uint32_t>(target, 0x08) &&
            at<std::uint32_t>(words, 0x0C) == at<std::uint32_t>(target, 0x10)) {
            break;
        }
    }
    std::uint8_t* const last =
        static_cast<std::uint8_t*>(ptr(action, 0x10)) + (at<std::int32_t>(action, 0x14) - 1) * 0x34;
    if (target != last) {
        at<std::uint8_t>(target, 0x00) = at<std::uint8_t>(last, 0x00);
        at<std::uint8_t>(target, 0x01) = at<std::uint8_t>(last, 0x01);
        at<std::uint32_t>(target, 0x04) = at<std::uint32_t>(last, 0x04);
        at<std::uint32_t>(target, 0x08) = at<std::uint32_t>(last, 0x08);
        at<std::uint32_t>(target, 0x0C) = at<std::uint32_t>(last, 0x0C);
        at<std::uint32_t>(target, 0x10) = at<std::uint32_t>(last, 0x10);
        at<std::uint32_t>(target, 0x14) = at<std::uint32_t>(last, 0x14);
        host.input_modifier_vector_assign_00a92ee0(add(target, 0x18), add(last, 0x18));
        host.input_modifier_vector_assign_00a92ee0(add(target, 0x24), add(last, 0x24));
        at<float>(target, 0x30) = at<float>(last, 0x30);
    }
    host.input_binding_destruct_00a93070(static_cast<std::uint8_t*>(ptr(action, 0x10)) +
                                         (at<std::int32_t>(action, 0x14) - 1) * 0x34);
    at<std::int32_t>(action, 0x14) -= 1;
}

// ---------------------------------------------------------------------------
// 00A97140 BSP_TextListenerBase_Destroy. __thiscall(ECX = listener), RET; SEH frame. vtable
// 00D5B708; the tree at +28h (head +2Ch, size +30h) is erased whole (00A96FB0 into a stack
// iterator), its head freed and +2Ch/+30h zeroed; then the NativeStrings +18h, +10h and +8h
// (length, data) are released in that order.
// ---------------------------------------------------------------------------
void text_listener_base_destroy_00a97140(SweepC2Host& host, void* listener) {
    at<std::uint32_t>(listener, 0x00) = kTextListenerBaseVtable;
    void* const tree = add(listener, 0x28);
    void* const head = ptr(listener, 0x2C);
    TreeIter erased{};
    host.tree_erase_range_00a96fb0(tree, &erased, TreeIter{tree, ptr(head, 0x00)}, TreeIter{tree, head});
    host.free_00bf65ac(ptr(tree, 0x04));
    at<void*>(tree, 0x04) = nullptr;
    at<std::uint32_t>(tree, 0x08) = 0;
    for (const std::size_t off : {std::size_t{0x18}, std::size_t{0x10}, std::size_t{0x08}}) {
        if (char* const data = at<char*>(listener, off + 4); data != nullptr) {
            host.string_pool_release_00419cc0_00bd1510(data, at<std::uint32_t>(listener, off) + 1);
        }
    }
}

// ---------------------------------------------------------------------------
// 00A97260 BSP_TextListenerBase_Construct. __thiscall(ECX = listener), RET; SEH frame, returns
// the listener. vtable 00D5B708, NativeStrings +8h/+10h/+18h zeroed, the tree at +28h gets a head
// (00A964D0; isnil byte +19h = 1, left/parent/right = head) and size 0; +4 byte = 0.
// ---------------------------------------------------------------------------
void* text_listener_base_construct_00a97260(SweepC2Host& host, void* listener) {
    at<std::uint32_t>(listener, 0x00) = kTextListenerBaseVtable;
    for (std::size_t off = 0x08; off <= 0x1C; off += 4) at<std::uint32_t>(listener, off) = 0;
    void* const tree = add(listener, 0x28);
    at<void*>(tree, 0x04) = host.tree_buy_head_node_00a964d0(tree);
    at<std::uint8_t>(ptr(tree, 0x04), 0x19) = 1;
    at<void*>(ptr(tree, 0x04), 0x04) = ptr(tree, 0x04);
    at<void*>(ptr(tree, 0x04), 0x00) = ptr(tree, 0x04);
    at<void*>(ptr(tree, 0x04), 0x08) = ptr(tree, 0x04);
    at<std::uint32_t>(tree, 0x08) = 0;
    at<std::uint8_t>(listener, 0x04) = 0;
    return listener;
}

// ---------------------------------------------------------------------------
// 00ACB9B0 BSP_GuiGrid_SetCellCount. __thiscall(ECX = grid, count), RET 4. Cells are 14h-byte
// records {float, vector<dword> object at +4 (first +8, last +0Ch)} in the vector at +ECh (first
// +F0h, last +F4h); +FCh is the count, +100h the per-cell length, +104h the cell's float. Equal
// counts return at once. Shrinking destroys cells count..old-1 (00AC9F10). The cell vector is
// resized (00ACB870) with a by-value cell {0.0f, ?, 0, 0, 0} (+4, the allocator byte, is left
// unwritten in the image; zero here). Growing then sizes each new cell's vector to +100h zeros
// (005A5A80), zero-fills it through the checked iterator, and sets its float from +104h. A
// non-zero count ends with vtable +84h. Every debug range check is kept.
// ---------------------------------------------------------------------------
void gui_grid_set_cell_count_00acb9b0(SweepC2Host& host, void* grid, std::int32_t count) {
    const std::int32_t old = at<std::int32_t>(grid, 0xFC);
    if (count == old) return;
    at<std::int32_t>(grid, 0xFC) = count;
    void* const cells = add(grid, 0xEC);
    const auto check = [&](std::int32_t index) {
        const auto first = at<std::intptr_t>(cells, 0x04);
        if (first == 0 ||
            static_cast<std::uint32_t>(static_cast<std::int32_t>(at<std::intptr_t>(cells, 0x08) - first) /
                                       0x14) <= static_cast<std::uint32_t>(index)) {
            host.invalid_iterator_00bf6713();
        }
    };
    if (count < old) {
        for (std::int32_t i = count; i < old; ++i) {
            check(i);
            host.gui_grid_cell_destroy_00ac9f10(add(ptr(cells, 0x04), static_cast<std::size_t>(i) * 0x14));
        }
    }
    alignas(4) std::uint8_t blank[0x14] = {};
    at<float>(blank, 0x00) = 0.0f;
    host.gui_grid_cells_resize_00acb870(cells, at<std::int32_t>(grid, 0xFC), blank);
    for (std::int32_t i = old; i < at<std::int32_t>(grid, 0xFC); ++i) {
        const std::size_t offset = static_cast<std::size_t>(i) * 0x14;
        check(i);
        host.dword_vector_resize_005a5a80(add(ptr(cells, 0x04), offset + 4),
                                          at<std::uint32_t>(grid, 0x100), 0);
        check(i);
        void* const inner = add(ptr(cells, 0x04), offset + 4);
        auto* iter = at<std::uint32_t*>(inner, 0x04);
        if (at<std::uintptr_t>(inner, 0x04) > at<std::uintptr_t>(inner, 0x08)) {
            host.invalid_iterator_00bf6713();
        }
        for (;;) {
            check(i);
            void* const current = add(ptr(cells, 0x04), offset + 4);
            auto* const last = at<std::uint32_t*>(current, 0x08);
            if (at<std::uintptr_t>(current, 0x04) > reinterpret_cast<std::uintptr_t>(last)) {
                host.invalid_iterator_00bf6713();
            }
            if (inner != current) host.invalid_iterator_00bf6713();
            if (iter == last) break;
            if (!(iter < at<std::uint32_t*>(inner, 0x08))) host.invalid_iterator_00bf6713();
            *iter = 0;
            if (!(iter < at<std::uint32_t*>(inner, 0x08))) host.invalid_iterator_00bf6713();
            ++iter;
        }
        check(i);
        at<float>(add(ptr(cells, 0x04), offset), 0x00) = at<float>(grid, 0x104);
    }
    if (count != 0) host.gui_widget_vslot_84(grid);
}

namespace {

// The shared body of 00ACD8E0 / 00ACD9E0 (they differ only in the 00E18B5C / 00E19311 constant
// the NEG/SBB/TEST null tests use). Equal handles return at once. Otherwise: held = old dest
// (add-ref'd when set); the plain assignment (store, add-ref new, release old); then an extra
// add-ref of the new dest when set, and held released twice (once by the inlined temporary, once
// by the SEH-owned local). Net: the new handle gains two references and the old loses two, as
// the image does.
void** ref_copy_assign_shared(SweepC2Host& host, void** dest, void* const* source) {
    if (*dest == *source) return dest;
    void* held = nullptr;
    if (void* const old = *dest; old != nullptr) {
        held = old;
        host.interlocked_increment_00ce221c(&at<std::int32_t>(old, 0x04));
    }
    void* const incoming = *source;
    void* const current = *dest;
    if (current != incoming) {
        *dest = incoming;
        if (incoming != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(incoming, 0x04));
        ref_release(host, current);
    }
    if (*dest != nullptr) host.interlocked_increment_00ce221c(&at<std::int32_t>(*dest, 0x04));
    ref_release(host, held);
    ref_release(host, held);
    return dest;
}

}  // namespace

// ---------------------------------------------------------------------------
// 00ACD8E0 BSP_GuiSoundRefPtr_CopyAssign. __fastcall(ECX = dest, EDX = source), RET, SEH frame;
// returns dest. Body: ref_copy_assign_shared above.
// ---------------------------------------------------------------------------
void** gui_sound_ref_copy_assign_00acd8e0(SweepC2Host& host, void** dest, void* const* source) {
    return ref_copy_assign_shared(host, dest, source);
}

// ---------------------------------------------------------------------------
// 00ACD9E0 BSP_GuiSound_Assign. __fastcall(ECX = dest, EDX = source), RET, SEH frame; returns
// dest. The same instructions as 00ACD8E0 (another template instance).
// ---------------------------------------------------------------------------
void** gui_sound_assign_00acd9e0(SweepC2Host& host, void** dest, void* const* source) {
    return ref_copy_assign_shared(host, dest, source);
}

// ---------------------------------------------------------------------------
// 00ACE6D0 BSP_GuiClipBox_DescribeProperties (vtable 00D5D074). __thiscall(ECX = box, sink),
// RET 4. The widget base (00AAAED0) first; then the tagged value {6, box+104h} is compared
// (00BD5680) with the default {6, &{0.1f, 0.1f}} (00D7A2F0); only a differing value is reported
// as sink->vtable[0Ch](0, "BorderWidth" 00D5D0D8, {6, box+104h}).
// ---------------------------------------------------------------------------
void gui_clip_box_describe_properties_00ace6d0(SweepC2Host& host, void* box, void* sink) {
    host.gui_widget_describe_properties_00aaaed0(box, sink);
    float fallback[2] = {kClipBoxDefaultBorder, kClipBoxDefaultBorder};
    const cc13_sweepC::ArchiveArg value{
        kTaggedVector2Kind, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(add(box, 0x104)))};
    const cc13_sweepC::ArchiveArg fallback_value{
        kTaggedVector2Kind, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(fallback))};
    if (host.tagged_value_equals_00bd5680(&value, &fallback_value)) return;
    host.property_sink_vslot_0c(sink, 0, kBorderWidthName, value);
}

// ---------------------------------------------------------------------------
// 00AD51D0 BSP_FontRegistry_ReplacePath. __thiscall(ECX = font, root, extra), RET 8. Returns at
// once when both the saved root (+38h) and extra (+40h) already equal the arguments (00435C40,
// case-insensitive). Otherwise every glyph node of the tree at +0 (head +4, size +8) frees its
// +10h block and zeroes it (walked with the checked iterator, 00AD40D0); the tree is then
// emptied inline (each root: 00AD41F0 on the right subtree, free, go left until the nil flag
// +15h), the head relinked to itself and the size zeroed; the handles +64h and +68h are released
// and zeroed; 00AD4C30 reloads (root, +20h, +30h, +28h, extra).
// ---------------------------------------------------------------------------
void font_registry_replace_path_00ad51d0(SweepC2Host& host, void* font, const void* root,
                                         const void* extra) {
    if (host.native_string_equals_insensitive_00435c40(add(font, 0x38), root) &&
        host.native_string_equals_insensitive_00435c40(add(font, 0x40), extra)) {
        return;
    }
    TreeIter it{font, ptr(ptr(font, 0x04), 0x00)};
    for (;;) {
        void* const head = ptr(font, 0x04);
        if (it.container == nullptr || it.container != font) host.invalid_iterator_00bf6713();
        if (it.node == head) break;
        if (it.container == nullptr) host.invalid_iterator_00bf6713();
        if (it.node == ptr(it.container, 0x04)) host.invalid_iterator_00bf6713();
        if (void* const block = ptr(it.node, 0x10); block != nullptr) {
            host.free_00bf65ac(block);
            at<void*>(it.node, 0x10) = nullptr;
        }
        host.tree_iter_inc_00ad40d0(&it);
    }
    void* node = ptr(ptr(font, 0x04), 0x04);
    while (at<std::uint8_t>(node, 0x15) == 0) {
        host.tree_erase_subtree_00ad41f0(font, ptr(node, 0x08));
        void* const left = ptr(node, 0x00);
        host.free_00bf65ac(node);
        node = left;
    }
    at<void*>(ptr(font, 0x04), 0x04) = ptr(font, 0x04);
    at<std::uint32_t>(font, 0x08) = 0;
    at<void*>(ptr(font, 0x04), 0x00) = ptr(font, 0x04);
    at<void*>(ptr(font, 0x04), 0x08) = ptr(font, 0x04);
    for (const std::size_t off : {std::size_t{0x64}, std::size_t{0x68}}) {
        if (void* const handle = ptr(font, off); handle != nullptr) {
            ref_release(host, handle);
            at<void*>(font, off) = nullptr;
        }
    }
    host.font_load_glyph_data_00ad4c30(font, root, add(font, 0x20), add(font, 0x30), add(font, 0x28),
                                       extra);
}

// ---------------------------------------------------------------------------
// 00ADA240 BSP_TerrainHeightField_SegmentHit (terrain vtable slot 3Ch). __thiscall(ECX =
// terrain, from, to, out), RET 0Ch; AL = hit. Both points go to node-local space through the
// node's (vtable +40h) inverse world matrix (00B6E0D0, 004142E0; the node is asked twice). When
// |float(from.x - to.x)| and |float(from.z - to.z)| are both below 0.001f (00D7A23C; each
// difference and its FABS spilled to float, FCOMI/JBE so NaN fails), 00AECC40(ECX = terrain,
// EDX = the ORIGINAL world from, to, out) answers. Otherwise x/z of both local points are made
// relative to the origin +80h/+84h, the quadtree (+88h) walks them (00AEA2B0), and a hit becomes
// out = (+80h + hit.x, hit.y, +84h + hit.z) taken back to world by the node's +F0h matrix (after
// 00B6DB70 when the node's +5Ch bit 2 is clear). The reconstruction's float arithmetic rounds
// each operation once, as the image's float spills do.
// ---------------------------------------------------------------------------
bool terrain_height_field_segment_hit_00ada240(SweepC2Host& host, void* terrain,
                                               const float* from, const float* to, float* out) {
    float local_from[3];
    float local_to[3];
    host.vector3_transform_affine_point_004142e0(
        from, local_from, host.node_inverse_world_matrix_00b6e0d0(host.terrain_node_vslot_40(terrain)));
    host.vector3_transform_affine_point_004142e0(
        to, local_to, host.node_inverse_world_matrix_00b6e0d0(host.terrain_node_vslot_40(terrain)));
    const float dx = local_from[0] - local_to[0];
    const float abs_dx = dx < 0.0f ? -dx : dx;
    if (kTerrainVerticalEpsilon > abs_dx) {
        const float dz = local_from[2] - local_to[2];
        const float abs_dz = dz < 0.0f ? -dz : dz;
        if (kTerrainVerticalEpsilon > abs_dz) {
            return host.terrain_vertical_segment_hit_00aecc40(terrain, from, to, out);
        }
    }
    const float origin_x = at<float>(terrain, 0x80);
    const float origin_z = at<float>(terrain, 0x84);
    local_from[0] = local_from[0] - origin_x;
    local_from[2] = local_from[2] - origin_z;
    local_to[0] = local_to[0] - origin_x;
    local_to[2] = local_to[2] - origin_z;
    float hit[3];
    const bool found =
        host.terrain_quadtree_ray_walk_00aea2b0(add(terrain, 0x88), local_from, local_to, hit);
    if (!found) return false;
    out[1] = hit[1];
    out[0] = at<float>(terrain, 0x80) + hit[0];
    out[2] = at<float>(terrain, 0x84) + hit[2];
    void* const node = host.terrain_node_vslot_40(terrain);
    if ((at<std::uint8_t>(node, 0x5C) & 0x02) == 0) host.transform_refresh_world_matrix_00b6db70(node);
    float world[3];
    const float* const back = host.vector3_transform_affine_point_004142e0(out, world, add(node, 0xF0));
    out[0] = back[0];
    out[1] = back[1];
    out[2] = back[2];
    return true;
}

// ---------------------------------------------------------------------------
// 00ADD290 BSP_TerrainHeightField_Construct. __thiscall(ECX = terrain, manager, name by value),
// RET 0Ch; SEH frame; returns terrain. Base 00AE9870(manager), vtable 00D5D350; the name string
// +20h is zeroed then copy-assigned from the by-value argument; +28h..+40h, the array +44h..+4Ch,
// the string +54h/+58h and +5Ch..+64h are zeroed; the quadtree +88h is constructed (00AEA8C0);
// +80h/+84h = 0.0f; +50h byte 0; the +44h array is resized to 0 (00ADBE60); +51h byte 0; the
// +54h string is resized to (0, 0) and, when it has data, given "" (00CE3A0C) by memcpy of its
// length; the by-value argument is released (00419CC0 / 00BD1510).
// ---------------------------------------------------------------------------
void* terrain_height_field_construct_00add290(SweepC2Host& host, void* terrain, void* manager,
                                              std::uint32_t name_length, char* name_data) {
    host.terrain_base_construct_00ae9870(terrain, manager);
    at<std::uint32_t>(terrain, 0x00) = kTerrainHeightFieldVtable;
    void* const name = add(terrain, 0x20);
    at<std::uint32_t>(name, 0x00) = 0;
    at<char*>(name, 0x04) = nullptr;
    NativeStringTemp argument{name_length, name_data};
    native_string_copy(host, name, &argument);
    for (std::size_t off = 0x28; off <= 0x40; off += 4) at<std::uint32_t>(terrain, off) = 0;
    for (std::size_t off = 0x44; off <= 0x4C; off += 4) at<std::uint32_t>(terrain, off) = 0;
    at<std::uint32_t>(terrain, 0x54) = 0;
    at<char*>(terrain, 0x58) = nullptr;
    for (std::size_t off = 0x5C; off <= 0x64; off += 4) at<std::uint32_t>(terrain, off) = 0;
    host.terrain_quadtree_construct_00aea8c0(add(terrain, 0x88));
    at<float>(terrain, 0x80) = 0.0f;
    at<float>(terrain, 0x84) = 0.0f;
    at<std::uint8_t>(terrain, 0x50) = 0;
    host.pointer_array_resize_00adbe60(add(terrain, 0x44), 0);
    at<std::uint8_t>(terrain, 0x51) = 0;
    host.native_string_resize_0041dd40(add(terrain, 0x54), 0, 0);
    if (char* const data = at<char*>(terrain, 0x58); data != nullptr) {
        host.memcpy_00bf7680(data, kEmptyText, at<std::uint32_t>(terrain, 0x54));
    }
    release_temp(host, argument);
    return terrain;
}

// ---------------------------------------------------------------------------
// 00ADE820 BSP_TerrainHeightField_AttachModel. __thiscall(ECX = terrain, name, model, float,
// float, dword), RET 14h; SEH frame. +28h = 1; +64h takes the model handle (add-ref new, release
// old; the argument itself is not released). A 174h-byte node (00B6ED70) is constructed with the
// name "Terrain Visibility Group" (00CF00D8, 0041E870 temporary released after) into +2Ch (null
// when the allocation fails); node vtable +38h(node_value); 00B6D890(+0Ch); the model's node
// ((+64h)+0Ch) takes it (00B6E680). The path "terrain/" + name + "_heightmap.tdt" (0043C130,
// 004261A0; the suffix built by resize(14, 1) and memcpy of length+1) loads through 00ADE500
// with (first, second, 0); the temporaries are released newest first.
// ---------------------------------------------------------------------------
void terrain_height_field_attach_model_00ade820(SweepC2Host& host, void* terrain, const void* name,
                                                void* model, float first, float second,
                                                std::uint32_t node_value) {
    at<std::uint32_t>(terrain, 0x28) = 1;
    ref_assign(host, at<void*>(terrain, 0x64), model);
    void* const memory = host.native_node_allocate_00b6ed70(kTerrainNodeBytes);
    void* node = nullptr;
    if (memory != nullptr) {
        NativeStringTemp group{0, nullptr};
        host.native_string_assign_0041e870(&group, kTerrainVisibilityGroupName);
        node = host.node_construct_00b6f5a0(memory, &group);
        at<void*>(terrain, 0x2C) = node;
        release_temp(host, group);
    } else {
        at<void*>(terrain, 0x2C) = nullptr;
    }
    host.node_vslot_38(ptr(terrain, 0x2C), node_value);
    host.node_propagate_root_00b6d890(ptr(terrain, 0x2C), at<std::uint32_t>(terrain, 0x0C));
    host.node_set_parent_00b6e680(ptr(ptr(terrain, 0x64), 0x0C), ptr(terrain, 0x2C));
    NativeStringTemp suffix{0, nullptr};
    host.native_string_resize_0041dd40(&suffix, 14, 1);
    if (suffix.data != nullptr) host.memcpy_00bf7680(suffix.data, kHeightmapSuffix, suffix.length + 1);
    NativeStringTemp prefixed{0, nullptr};
    void* const prefixed_ref = host.native_string_construct_prefixed_0043c130(&prefixed, kTerrainPathPrefix, name);
    NativeStringTemp path{0, nullptr};
    void* const path_ref = host.native_string_concat_004261a0(prefixed_ref, &path, &suffix);
    host.terrain_height_field_load_00ade500(terrain, path_ref, first, second, 0);
    release_temp(host, path);
    release_temp(host, prefixed);
    release_temp(host, suffix);
}

// ---------------------------------------------------------------------------
// 00A9F0B0 BSP_StringMapNodePool_Allocate. __thiscall(ECX = pool), RET -> node. Under the pool's
// embedded tracked lock (+0Ch, counter +24h): with no current block (+34h == -1) a new 7804h-byte
// block (operator new 00BF681B; 00A9E970(index = old count), null when the allocation fails)
// becomes current and is appended to the block array {data +28h, count +2Ch, capacity +30h},
// which grows to capacity*2+2 (operator new 00BF55BE, copy, delete 00BF6989) when full. The
// current block's free count (word +7800h) drops by one and the free-index list (words at
// +7000h) names the node at block + index*1Ch. A block left empty makes the next block with free
// nodes (scanning from current+1) current, or -1.
// ---------------------------------------------------------------------------
void* string_map_node_pool_allocate_00a9f0b0(SweepC2Host& host, void* pool) {
    void* const section = add(pool, 0x0C);
    host.enter_critical_section(section);
    at<std::int32_t>(section, 0x18) += 1;
    if (at<std::int32_t>(pool, 0x34) == -1) {
        at<std::int32_t>(pool, 0x34) = at<std::int32_t>(pool, 0x2C);
        void* const memory = host.operator_new_00bf681b(0x7804);
        void* const block = memory != nullptr
                                ? host.string_map_node_block_construct_00a9e970(memory, at<std::int32_t>(pool, 0x34))
                                : nullptr;
        if (at<std::uint32_t>(pool, 0x2C) == at<std::uint32_t>(pool, 0x30)) {
            const std::uint32_t capacity = at<std::uint32_t>(pool, 0x30) * 2 + 2;
            at<std::uint32_t>(pool, 0x30) = capacity;
            auto* const grown = static_cast<void**>(host.operator_new_00bf55be(capacity * 4));
            void** dst = grown;
            for (std::uint32_t i = 0; i < at<std::uint32_t>(pool, 0x2C); ++i, ++dst) {
                if (dst != nullptr) *dst = at<void**>(pool, 0x28)[i];
            }
            if (void* const old = ptr(pool, 0x28); old != nullptr) host.operator_delete_00bf6989(old);
            at<void**>(pool, 0x28) = grown;
        }
        void** const slot = at<void**>(pool, 0x28) + at<std::uint32_t>(pool, 0x2C);
        if (slot != nullptr) *slot = block;
        at<std::uint32_t>(pool, 0x2C) += 1;
    }
    void* const block = at<void**>(pool, 0x28)[at<std::int32_t>(pool, 0x34)];
    at<std::uint16_t>(block, 0x7800) -= 1;
    const std::uint16_t free_count = at<std::uint16_t>(block, 0x7800);
    const std::uint16_t index = at<std::uint16_t>(block, 0x7000 + static_cast<std::size_t>(free_count) * 2);
    void* const node = add(block, static_cast<std::size_t>(index) * 0x1C);
    if (free_count == 0) {
        std::uint32_t next = static_cast<std::uint32_t>(at<std::int32_t>(pool, 0x34)) + 1;
        at<std::int32_t>(pool, 0x34) = -1;
        for (; next < at<std::uint32_t>(pool, 0x2C); ++next) {
            if (at<std::uint16_t>(at<void**>(pool, 0x28)[next], 0x7800) != 0) {
                at<std::int32_t>(pool, 0x34) = static_cast<std::int32_t>(next);
                break;
            }
        }
    }
    at<std::int32_t>(section, 0x18) -= 1;
    host.leave_critical_section(section);
    return node;
}

// ---------------------------------------------------------------------------
// 00AED100 grid fill from a structured reader. __thiscall(ECX = grid, reader), RET 4. For row
// 0..+30h and column 0..+2Ch (both signed, the column count re-read each step) one float is read
// (00BE99D0, ST0 spilled to float) and stored through vtable +1Ch(value, column, row).
// ---------------------------------------------------------------------------
void grid_read_cells_00aed100(SweepC2Host& host, void* grid, void* reader) {
    for (std::int32_t row = 0; row < at<std::int32_t>(grid, 0x30); ++row) {
        for (std::int32_t column = 0; column < at<std::int32_t>(grid, 0x2C); ++column) {
            const float value = host.structured_node_read_float_00be99d0(reader);
            host.grid_set_cell_vslot_1c(grid, value, column, row);
        }
    }
}

// ---------------------------------------------------------------------------
// 00B1B400 BSP_SamplerLoader_LoadWithOptions. __thiscall(ECX = loader, name, options), RET 8;
// SEH frame. A stack copy of the name (resize + memcpy of its length) is lower-cased (004BCC00)
// and handed to 00B1A4F0(ECX = loader+4, &copy, options, 1, 1). The copy is released with the
// data pointer and length read after the resize/after the call, as the image keeps them.
// ---------------------------------------------------------------------------
std::uint32_t sampler_loader_load_with_options_00b1b400(SweepC2Host& host, void* loader,
                                                        const void* name, std::uint32_t options) {
    NativeStringTemp copy{0, nullptr};
    char* data = nullptr;
    if (&copy != name) {
        void* const source = const_cast<void*>(name);
        host.native_string_resize_0041dd40(&copy, at<std::uint32_t>(source, 0x00), 1);
        data = copy.data;
        if (at<std::uint32_t>(source, 0x00) != 0) {
            host.memcpy_00bf7680(data, at<char*>(source, 0x04), copy.length);
        }
    }
    host.string_lowercase_ascii_004bcc00(&copy);
    const std::uint32_t answer = host.sampler_cache_load_00b1a4f0(add(loader, 0x04), &copy, options, 1, 1);
    if (data != nullptr) host.string_pool_release_00419cc0_00bd1510(data, copy.length + 1);
    return answer;
}

// ---------------------------------------------------------------------------
// 00B1BD20 BSP_LuaGlobals_SetFieldsNamedByStringEntries. __thiscall(ECX = owner, value), RET 4;
// SEH frame over three stack LuaObjects (globals table, key, value) and a NativeString vector
// {data, size, capacity}. Every key of the globals table (00B67980 on owner+4; 00B67080 /
// 00B67190 to walk, 00B66420 on the value as the end test) that is a string (00B660A0) is copied
// into a NativeString (strlen, resize, memcpy length+1) and appended (004CDC20). Then each
// collected name is set on the globals table to `value` (00B674C0); the vector is resized to 0
// (00427110) and its storage deleted (00BF6989); the three objects are destroyed value, key,
// table (00B67700).
// ---------------------------------------------------------------------------
void lua_globals_set_fields_named_by_strings_00b1bd20(SweepC2Host& host, void* owner,
                                                      void* value) {
    alignas(4) std::uint8_t table[0x14];
    alignas(4) std::uint8_t key[0x14];
    alignas(4) std::uint8_t item[0x14];
    host.lua_state_get_globals_00b67980(add(owner, 0x04), table);
    host.lua_object_construct_00b65f50(key);
    host.lua_object_construct_00b65f50(item);
    struct NameVector {
        NativeStringTemp* data;
        std::uint32_t size;
        std::uint32_t capacity;
    } names{nullptr, 0, 0};
    host.lua_object_iterate_first_00b67080(table, key, item);
    if (!host.lua_object_is_unbound_00b66420(table, item)) {
        do {
            if (host.lua_object_is_string_00b660a0(key)) {
                const char* const text = host.lua_object_get_string_00b662b0(key);
                std::uint32_t length = 0;
                while (text[length] != '\0') ++length;
                NativeStringTemp name{0, nullptr};
                host.native_string_resize_0041dd40(&name, length, 1);
                char* const data = name.data;
                const std::uint32_t size = name.length;
                if (data != nullptr) host.memcpy_00bf7680(data, text, size + 1);
                host.native_string_vector_append_004cdc20(&names, &name);
                if (data != nullptr) host.string_pool_release_00419cc0_00bd1510(data, size + 1);
            }
            host.lua_object_iterate_next_00b67190(table, key, item);
        } while (!host.lua_object_is_unbound_00b66420(table, item));
    }
    for (std::uint32_t i = 0; i < names.size; ++i) {
        host.lua_object_set_string_field_00b674c0(table, &names.data[i], value);
    }
    host.native_string_vector_resize_00427110(&names, 0);
    host.operator_delete_00bf6989(names.data);
    host.lua_object_destruct_00b67700(item);
    host.lua_object_destruct_00b67700(key);
    host.lua_object_destruct_00b67700(table);
}

}  // namespace bsp::cc13_sweepC2
