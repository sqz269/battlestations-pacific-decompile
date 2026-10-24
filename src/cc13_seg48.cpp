// cc13_seg48: segment 48 leftovers (packet cc13-seg48, docs/CC13_SEG48.md).
// Names are hypotheses; see the header for the hooks. Each function's
// comment gives the native address, ABI and evidence (Capstone disasm-raw over
// the disk bytes plus the Ghidra pseudocode). EH frames are noted, not
// modelled: the image's EH state stores only select unwind actions.

#include "bsp/cc13_seg48.hpp"

#include <cstring>
#include <cmath>

namespace bsp {

namespace {

template <class T>
T& at(void* base, std::size_t offset) {
    return cc13_seg48_at<T>(base, offset);
}

// The inlined reference release the image repeats everywhere:
//   p = [slot]; if (p) { if (InterlockedDecrement(p + 4) == 0) p->vt[0]();
//   [slot] = 0; }
// The count lives at +4h of the counted object.
void release_slot(void* base, std::size_t offset, Cc13Seg48Host& host) {
    void* object = at<void*>(base, offset);
    if (object != nullptr) {
        if (host.interlocked_decrement(&at<std::int32_t>(object, 4)) == 0) {
            host.vcall_00_release(object);
        }
        at<void*>(base, offset) = nullptr;
    }
}

// The engine pointer-array growth inlined in 007DE0B0 / 007DE6B0 / 007E0F20:
// capacity = capacity * 2 + 2; operator new (00BF55BE) capacity * 4; copy
// `count` dwords (each store guarded by the new element's address being
// non-null, as the image tests it); _free (00BF6989) the old block when
// non-null; data = new block. Layout {data, count, capacity} at `array`.
void grow_double_plus_two(void* array, Cc13Seg48Host& host) {
    const std::int32_t capacity = at<std::int32_t>(array, 8) * 2 + 2;
    at<std::int32_t>(array, 8) = capacity;
    auto* block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity) * 4u));
    std::uint32_t* out = block;
    const std::uint32_t count = at<std::uint32_t>(array, 4);
    for (std::uint32_t i = 0; i < count; ++i, ++out) {
        if (out != nullptr) {
            *out = at<std::uint32_t*>(array, 0)[i];
        }
    }
    if (at<void*>(array, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(array, 0));
    }
    at<std::uint32_t*>(array, 0) = block;
}

// Append after the growth above: the element address data + count * 4 is
// tested for null before the store (the image's placement-new guard), then
// count += 1.
void append_dword(void* array, std::uint32_t value) {
    std::uint32_t* slot = at<std::uint32_t*>(array, 0) + at<std::uint32_t>(array, 4);
    if (slot != nullptr) {
        *slot = value;
    }
    at<std::uint32_t>(array, 4) += 1;
}

// The checked reserve inlined in 007EDD70 / 007EDDE0: when capacity < want,
// capacity = want, operator new (00BF55BE) of want * 4 with the image's
// overflow saturation (-(hi != 0) | lo, i.e. 0xFFFFFFFF when want * 4 does
// not fit), copy count dwords only when the old data is non-null, _free
// (00BF6989) the old block, data = new block.
void reserve_checked(void* array, std::uint32_t want, Cc13Seg48Host& host) {
    if (at<std::uint32_t>(array, 8) < want) {
        at<std::uint32_t>(array, 8) = want;
        const std::uint64_t bytes = static_cast<std::uint64_t>(want) * 4u;
        const std::uint32_t size = (bytes >> 32) != 0 ? 0xFFFFFFFFu : static_cast<std::uint32_t>(bytes);
        auto* block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(size));
        if (at<void*>(array, 0) != nullptr) {
            const std::uint32_t count = at<std::uint32_t>(array, 4);
            for (std::uint32_t i = 0; i < count; ++i) {
                block[i] = at<std::uint32_t*>(array, 0)[i];
            }
            host.crt_free_00bf6989(at<void*>(array, 0));
        }
        at<std::uint32_t*>(array, 0) = block;
    }
}

// The PodArray4 push-back body shared by 007E0FE0 and 007E1CF0: when count ==
// capacity, reserve (007BD940) max(capacity * 2, 1) by the image's signed
// `cmp eax, 1; jg` test; then the guarded append.
void pod_array4_push_back(void* array, const std::uint32_t* value, Cc13Seg48Host& host) {
    if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
        std::int32_t capacity = at<std::int32_t>(array, 8) * 2;
        if (capacity <= 1) {
            capacity = 1;
        }
        host.pod_array4_reserve_007bd940(array, capacity);
    }
    append_dword(array, *value);
}

// The unregister body shared by 007D7930 / 007DDBC0: detach the plane from
// the world lists (0074E610, ECX = unit), then walk the class's world list
// (head pointer at [unit+30h]+head_offset, nodes {?, next +4h, unit +8h})
// for the node holding this unit and erase it (004837D0, ECX = the list at
// [unit+30h]+head_offset-4, node).
void unregister_from_world_list(void* unit, std::size_t head_offset, Cc13Seg48Host& host) {
    host.plane_unit_detach_from_world_lists_0074e610(unit);
    void* owner = at<void*>(unit, 0x30);
    void* node = at<void*>(owner, head_offset);
    if (node == nullptr) {
        return;
    }
    while (at<void*>(node, 8) != unit) {
        node = at<void*>(node, 4);
        if (node == nullptr) {
            return;
        }
    }
    host.unit_list_erase_node_004837d0(static_cast<std::uint8_t*>(owner) + head_offset - 4, node);
}

}  // namespace

// ---------------------------------------------------------------- plane units

// 007D7930 BSP_MPlaneBomber_UnregisterFromWorldLists, __thiscall(ECX = unit),
// slot +134h of vtable 00D06638. Bytes 007D7930-007D7965: list head at
// [unit+30h]+DCh, erase on [unit+30h]+D8h.
void cc13_seg48_mplane_bomber_unregister_from_world_lists_007d7930(void* unit, Cc13Seg48Host& host) {
    unregister_from_world_list(unit, 0xDC, host);
}

// 007DDBC0 BSP_MPlaneFighter_UnregisterFromWorldLists, __thiscall(ECX = unit),
// slot +134h of vtable 00D06920: the same body on [unit+30h]+100h / +FCh.
void cc13_seg48_mplane_fighter_unregister_from_world_lists_007ddbc0(void* unit, Cc13Seg48Host& host) {
    unregister_from_world_list(unit, 0x100, host);
}

// 007D7A10 BSP_OwnedPtrHolder_Destruct_00D06848, __thiscall(ECX = holder):
// vtable 00D06848 at +0h; when [holder+10h] is non-null, its vtable slot 0
// with `push 1` (scalar deleting destructor). Bytes 007D7A10-007D7A25.
void cc13_seg48_owned_ptr_holder_destruct_007d7a10(void* holder, Cc13Seg48Host& host) {
    at<std::uint32_t>(holder, 0) = 0x00D06848u;
    void* owned = at<void*>(holder, 0x10);
    if (owned != nullptr) {
        host.vcall_00_delete(owned, 1);
    }
}

// 007DDCB0 BSP_Plane_KamikazeGatePredicate, __thiscall(ECX = unit), EAX:
// returns 0 only when byte +BC8h is 0, the unit's slot +5Ch(17h) answers
// true and byte +C24h is 0; otherwise 1. Bytes 007DDCB0-007DDCDC.
std::int32_t cc13_seg48_plane_kamikaze_gate_predicate_007ddcb0(void* unit, Cc13Seg48Host& host) {
    if (at<std::uint8_t>(unit, 0xBC8) == 0) {
        if (host.vcall_5c_has_ordnance_kind(unit, 0x17) && at<std::uint8_t>(unit, 0xC24) == 0) {
            return 0;
        }
    }
    return 1;
}

// 007DDD60 BSP_PartRecordRef_Construct, __thiscall(ECX = record, owner, node,
// float weight), RET 0Ch, EAX = record. Stores weight (movss from the third
// stack argument) at +8h, owner at +0h, node at +4h, 0.0f at +0Ch, then copies
// the 16 floats of 00B6DB60(ECX = node) to +10h..+4Ch.
void* cc13_seg48_part_record_ref_construct_007ddd60(void* record, std::uint32_t owner, void* node, float weight,
                                                    Cc13Seg48Host& host) {
    at<float>(record, 8) = weight;
    at<std::uint32_t>(record, 0) = owner;
    at<void*>(record, 4) = node;
    at<float>(record, 0xC) = 0.0f;
    const float* matrix = host.node_get_local_matrix_00b6db60(node);
    for (int i = 0; i < 16; ++i) {
        at<float>(record, 0x10 + 4 * i) = matrix[i];
    }
    return record;
}

// 007DDDE0 BSP_Vector3_FillOne_007ddde0, __thiscall(ECX = float[3]): all three
// components = [00D7A24C] (1.0f).
void cc13_seg48_vector3_fill_one_007ddde0(float* v) {
    v[0] = kCc13Seg48One;
    v[1] = kCc13Seg48One;
    v[2] = kCc13Seg48One;
}

// 007DDE00 BSP_PlaneSubRecord_InitDefaults_007dde00, __thiscall(ECX = record):
// +0h = 1.0f, byte +4h = 0, +8h = 0.
void cc13_seg48_plane_sub_record_init_defaults_007dde00(void* record) {
    at<float>(record, 0) = kCc13Seg48One;
    at<std::uint8_t>(record, 4) = 0;
    at<std::uint32_t>(record, 8) = 0;
}

// 007DDFB0 BSP_SingletonF87440_DestructUnwind, __thiscall(ECX = object):
// clears the singleton pointer [00F87440] (passed here as the host's global)
// and restores the base vtable 00CE3818.
void cc13_seg48_singleton_f87440_destruct_unwind_007ddfb0(void* object, std::uint32_t* global_00f87440) {
    *global_00f87440 = 0;
    at<std::uint32_t>(object, 0) = 0x00CE3818u;
}

// 007E2510 BSP_PlaneRecord_InitZero_007e2510, __thiscall(ECX = record, value),
// RET 4: +0h, +4h, +0Ch, +10h, +14h = 0; +8h = value; +18h = 1.0f; +1Ch = 0.
void* cc13_seg48_plane_record_init_zero_007e2510(void* record, std::uint32_t value) {
    at<std::uint32_t>(record, 0) = 0;
    at<std::uint32_t>(record, 4) = 0;
    at<std::uint32_t>(record, 0xC) = 0;
    at<std::uint32_t>(record, 0x10) = 0;
    at<std::uint32_t>(record, 0x14) = 0;
    at<std::uint32_t>(record, 8) = value;
    at<float>(record, 0x18) = kCc13Seg48One;
    at<std::uint32_t>(record, 0x1C) = 0;
    return record;
}

namespace {

// The vtable and type stores of 007D7720 / 007DD9B0 after 007CFD20(ECX =
// unit, arg, 0): +0h, +10h, +24h, +170h, +1E4h, +310h, +38Ch, +72Ch, then the
// unit kind +C4h and the class slot-0 answer +AACh.
struct PlaneLeafVtables {
    std::uint32_t vt[8];
    std::uint32_t kind_c4;
    std::uint32_t value_aac;
};

void* construct_plane_leaf(void* unit, std::uint32_t arg, const PlaneLeafVtables& v, Cc13Seg48Host& host) {
    static constexpr std::size_t kOffsets[8] = {0x0, 0x10, 0x24, 0x170, 0x1E4, 0x310, 0x38C, 0x72C};
    host.plane_unit_instance_construct_007cfd20(unit, arg, 0);
    for (int i = 0; i < 8; ++i) {
        at<std::uint32_t>(unit, kOffsets[i]) = v.vt[i];
    }
    at<std::uint32_t>(unit, 0xC4) = v.kind_c4;
    at<std::uint32_t>(unit, 0xAAC) = v.value_aac;
    return unit;
}

// The class factory body of 007D7850 / 007DDAE0: operator new (00BF55BE)
// E94h bytes, memset (00BF79F0) to 0 BEFORE the null test (as the image
// orders it), construct when non-null, then 009553D0(ECX = unit, class) and
// unit+354h = class with no null test on the unit.
void* create_plane_leaf(void* vehicle_class, std::uint32_t arg, void* (*construct)(void*, std::uint32_t, Cc13Seg48Host&),
                        Cc13Seg48Host& host) {
    void* block = host.operator_new_00bf55be(0xE94);
    host.crt_memset_00bf79f0(block, 0, 0xE94);
    void* unit = block != nullptr ? construct(block, arg, host) : nullptr;
    host.unit_set_vehicle_class_009553d0(unit, vehicle_class);
    at<void*>(unit, 0x354) = vehicle_class;
    return unit;
}

}  // namespace

// 007D7720 BSP_MPlaneBomber_Construct, __thiscall(ECX = unit, arg), RET 4, EAX
// = unit. Bytes 007D7720-007D778C: vtables 00D06638, 00D06620, 00D06618,
// 00D06614, 00D0660C, 00D065F4, 00D065F0, 00D065B0; +C4h = 10h; +AACh = 7.
void* cc13_seg48_mplane_bomber_construct_007d7720(void* unit, std::uint32_t arg, Cc13Seg48Host& host) {
    static constexpr PlaneLeafVtables kBomber{
        {0x00D06638u, 0x00D06620u, 0x00D06618u, 0x00D06614u, 0x00D0660Cu, 0x00D065F4u, 0x00D065F0u, 0x00D065B0u},
        0x10,
        7};
    return construct_plane_leaf(unit, arg, kBomber, host);
}

// 007D7790 BSP_MPlaneBomber_Sub_Slot0_Return7: returns 7 (vtable 00D06614 slot 0).
std::uint32_t cc13_seg48_mplane_bomber_slot0_return7_007d7790() {
    return 7;
}

// 007D7850 BSP_MPlaneBomberClass_Create, __thiscall(ECX = class, arg), RET 4,
// EAX = unit, EH frame 00C8D798. Bytes 007D7850-007D78CD.
void* cc13_seg48_mplane_bomber_class_create_007d7850(void* vehicle_class, std::uint32_t arg, Cc13Seg48Host& host) {
    return create_plane_leaf(vehicle_class, arg, &cc13_seg48_mplane_bomber_construct_007d7720, host);
}

// 007DD9B0 BSP_MPlaneFighter_Construct, __thiscall(ECX = unit, arg), RET 4.
// Bytes 007DD9B0-007DDA1C: vtables 00D06920, 00D06908, 00D06900, 00D068FC,
// 00D068F4, 00D068DC, 00D068D8, 00D06898; +C4h = 13h; +AACh = 0Ah.
void* cc13_seg48_mplane_fighter_construct_007dd9b0(void* unit, std::uint32_t arg, Cc13Seg48Host& host) {
    static constexpr PlaneLeafVtables kFighter{
        {0x00D06920u, 0x00D06908u, 0x00D06900u, 0x00D068FCu, 0x00D068F4u, 0x00D068DCu, 0x00D068D8u, 0x00D06898u},
        0x13,
        0x0A};
    return construct_plane_leaf(unit, arg, kFighter, host);
}

// 007DDA20 BSP_MPlaneFighter_Sub_Slot0_Return0A: returns 0Ah (vtable 00D068FC slot 0).
std::uint32_t cc13_seg48_mplane_fighter_slot0_return0a_007dda20() {
    return 0x0A;
}

// 007DDAE0 BSP_MPlaneFighterClass_Create: 007D7850's body with 007DD9B0
// (calls at 007DDAFF, 007DDB0E, 007DDB2D, 007DDB43; store 007DDB4C).
void* cc13_seg48_mplane_fighter_class_create_007ddae0(void* vehicle_class, std::uint32_t arg, Cc13Seg48Host& host) {
    return create_plane_leaf(vehicle_class, arg, &cc13_seg48_mplane_fighter_construct_007dd9b0, host);
}

// 007D7A40 BSP_PilotPlan_ResetCommandTimer, __thiscall(ECX = plan): when
// [plan+10h] is non-null, its +C8h = 1.2f ([00CE3814], bytes 9A 99 99 3F).
void cc13_seg48_pilot_plan_reset_command_timer_007d7a40(void* plan) {
    void* target = at<void*>(plan, 0x10);
    if (target != nullptr) {
        at<std::uint32_t>(target, 0xC8) = 0x3F99999Au;
    }
}

// 007DDEA0 BSP_PlaneSubBlock_ResetTimer40, __thiscall(ECX = block): +40h =
// 60.0f ([00CEB4B0], bytes 00 00 70 42).
void cc13_seg48_plane_sub_block_reset_timer40_007ddea0(void* block) {
    at<float>(block, 0x40) = 60.0f;
}

// ---------------------------------------------------------------- pod arrays

// 007DE030 BSP_PodArray_FreeBuffer_007de030, __thiscall(ECX = array):
// _free (00BF6989) of [array] when non-null; the pointer is not cleared.
void cc13_seg48_pod_array_free_buffer_007de030(void* array, Cc13Seg48Host& host) {
    if (at<void*>(array, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(array, 0));
    }
}

// 007DE0B0 BSP_PtrArray_Grow_007de0b0, __thiscall(ECX = array): the
// unconditional capacity * 2 + 2 growth.
void cc13_seg48_ptr_array_grow_007de0b0(void* array, Cc13Seg48Host& host) {
    grow_double_plus_two(array, host);
}

// 007DE6B0 BSP_PtrArray_PushBack_007de6b0, __thiscall(ECX = array, const
// dword*), RET 4: grow when count == capacity, then the guarded append.
void cc13_seg48_ptr_array_push_back_007de6b0(void* array, const std::uint32_t* value, Cc13Seg48Host& host) {
    if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
        grow_double_plus_two(array, host);
    }
    append_dword(array, *value);
}

// 007E0FE0 BSP_PodArray4_PushBack_007e0fe0, __thiscall(ECX = array, const
// dword*), RET 4.
void cc13_seg48_pod_array4_push_back_007e0fe0(void* array, const std::uint32_t* value, Cc13Seg48Host& host) {
    pod_array4_push_back(array, value, host);
}

// 007E1CF0 BSP_PodArray4_PushBack_007e1cf0: a second instantiation of the
// same body (identical pseudocode, same 007BD940 reserve).
void cc13_seg48_pod_array4_push_back_007e1cf0(void* array, const std::uint32_t* value, Cc13Seg48Host& host) {
    pod_array4_push_back(array, value, host);
}

// 007EDD00 BSP_PodArray_FreeBufferUnwind_007edd00, __thiscall(ECX = array):
// _free (00BF6989) of [array] when non-null, then [array] = 0.
void cc13_seg48_pod_array_free_buffer_unwind_007edd00(void* array, Cc13Seg48Host& host) {
    if (at<void*>(array, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(array, 0));
        at<void*>(array, 0) = nullptr;
    }
}

// 007EDD70 BSP_PtrArray_Reserve_007edd70, __thiscall(ECX = array, unsigned
// want), RET 4: the checked reserve (unsigned capacity compare).
void cc13_seg48_ptr_array_reserve_007edd70(void* array, std::uint32_t capacity, Cc13Seg48Host& host) {
    reserve_checked(array, capacity, host);
}

// 007EDDE0 BSP_PtrArray_Grow_007edde0, __thiscall(ECX = array): the checked
// reserve with want = capacity * 2 + 2 (the image keeps the `capacity < want`
// test, which fails only on unsigned wrap).
void cc13_seg48_ptr_array_grow_007edde0(void* array, Cc13Seg48Host& host) {
    reserve_checked(array, at<std::uint32_t>(array, 8) * 2u + 2u, host);
}

// 007F5570 BSP_PodArray_FreeBufferUnwind_007f5570, __thiscall(ECX = array):
// _free (00BF6989) of [array] when non-null; the pointer is not cleared.
void cc13_seg48_pod_array_free_buffer_unwind_007f5570(void* array, Cc13Seg48Host& host) {
    if (at<void*>(array, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(array, 0));
    }
}

// ------------------------------------------------- callback buffers, neighbours

// 007DE420 BSP_CallbackBuffer_Destruct_007de420, __thiscall(ECX = buffer):
// _free (00BF6989) of [buffer+14h] when non-null, then tail-jumps to
// 00695870 with ECX = buffer. Bytes 007DE420-007DE43B.
void cc13_seg48_callback_buffer_destruct_007de420(void* buffer, Cc13Seg48Host& host) {
    if (at<void*>(buffer, 0x14) != nullptr) {
        host.crt_free_00bf6989(at<void*>(buffer, 0x14));
    }
    host.callback_owner_destroy_00695870(buffer);
}

// 007DE690 BSP_CallbackOwner_Reset_007de690, __thiscall(ECX = owner):
// 00695830(ECX = owner), then [owner+18h] = 0 when it is non-zero (the
// count; the buffer is kept). Bytes 007DE690-007DE6A6.
void cc13_seg48_callback_owner_reset_007de690(void* owner, Cc13Seg48Host& host) {
    host.callback_owner_detach_all_if_bound_00695830(owner);
    if (at<std::uint32_t>(owner, 0x18) != 0) {
        at<std::uint32_t>(owner, 0x18) = 0;
    }
}

// 007DEC80 BSP_CallbackBuffer_Destruct_007dec80: a second instantiation of
// 007DE420's body.
void cc13_seg48_callback_buffer_destruct_007dec80(void* buffer, Cc13Seg48Host& host) {
    if (at<void*>(buffer, 0x14) != nullptr) {
        host.crt_free_00bf6989(at<void*>(buffer, 0x14));
    }
    host.callback_owner_destroy_00695870(buffer);
}

// 007DECF0 BSP_PlaneNeighbours_Destruct, __thiscall(ECX = neighbours), EH
// frame 00C8D879. Bytes 007DECF0-007DED99: vtable 00D06B88; 006CDD70(ECX =
// +9Ch); then the three callback buffers in reverse order, each "_free
// (00BF6989) its data when non-null, 00695870(ECX = buffer)": +5Ch (data
// +70h), +3Ch (data +50h), +1Ch (data +30h); then 00875B30(ECX = object).
void cc13_seg48_plane_neighbours_destruct_007decf0(void* neighbours, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(neighbours);
    at<std::uint32_t>(neighbours, 0) = 0x00D06B88u;
    host.follow_observer_member_destroy_006cdd70(base + 0x9C);
    static constexpr std::size_t kBuffers[] = {0x5C, 0x3C, 0x1C};
    for (std::size_t buffer : kBuffers) {
        if (at<void*>(neighbours, buffer + 0x14) != nullptr) {
            host.crt_free_00bf6989(at<void*>(neighbours, buffer + 0x14));
        }
        host.callback_owner_destroy_00695870(base + buffer);
    }
    host.tick_subnode_base_cleanup_00875b30(neighbours);
}

// 007E0F20 BSP_PlaneNeighbours_AppendObserved, __thiscall(ECX = callback
// buffer, observed), RET 4: 00694A60(ECX = observed, EDX = buffer), then the
// pointer-array push of `observed` onto {data +14h, count +18h, capacity +1Ch}
// with the capacity * 2 + 2 growth (00BF55BE / 00BF6989). Bytes
// 007E0F20-007E0FA0.
void cc13_seg48_plane_neighbours_append_observed_007e0f20(void* buffer, void* observed, Cc13Seg48Host& host) {
    host.observer_register_pair_00694a60(observed, buffer);
    auto* array = static_cast<std::uint8_t*>(buffer) + 0x14;
    if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
        grow_double_plus_two(array, host);
    }
    append_dword(array, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(observed)));
}

// 007E2070 BSP_PlaneNeighbours_ClearTargetObserver, __thiscall(ECX =
// neighbours): with member = neighbours+9Ch, when [member+14h] (the observed
// target, neighbours+B0h) is non-null, 006952A0(ECX = target, EDX = member)
// and [member+14h] = 0. Bytes 007E2070-007E208D.
void cc13_seg48_plane_neighbours_clear_target_observer_007e2070(void* neighbours, Cc13Seg48Host& host) {
    auto* member = static_cast<std::uint8_t*>(neighbours) + 0x9C;
    void* target = at<void*>(member, 0x14);
    if (target != nullptr) {
        host.observer_unregister_pair_006952a0(target, member);
        at<void*>(member, 0x14) = nullptr;
    }
}

// 007E1E20 BSP_PlaneNeighbours_Construct, __thiscall(ECX = neighbours, owner
// unit), RET 4, EAX = neighbours, EH frame 00C8D9CF. Bytes 007E1E20-007E2004.
// Tick-subnode base: vtable 00CFD99C, +4h/+8h/+0Ch = 0, byte +10h = 0, byte
// +11h = 1, +14h = 0, +18h = 2, then 00876020(ECX = owner ? owner+310h : null,
// neighbours). Then vtable 00D06B88 and three callback buffers at +1Ch, +3Ch,
// +5Ch (vtable 00D06C58 at +0h; +4h/+8h/+0Ch = 0; byte +10h = 0; data, count,
// capacity +14h..+1Ch = 0). +88h = 3.0f (00CE3854), +8Ch = 2.0f (00CE3958),
// +90h = 1.0f (00D7A24C), +94h = 400.0f (00CFD710), +98h = float(00412E20(
// [00CEDF5C] = 0.0872664f, bits 3DB2B8C3)); the follow member at +9Ch: +A0h,
// +A4h, +A8h, +B0h = 0, byte +ACh = 1, vtable 00CF89B4; +B8h..+C4h = 0.0f, byte
// +C8h = 0. The three clocks, in this order, on RNG STREAM 1 (00BD2F10, ECX = 1):
// +7Ch = U(0, +88h) + +88h (call 007E1F94), +84h = U(0, +8Ch) + +8Ch (007E1FB9),
// +80h = U(0, +90h) + +90h (007E1FE1); each FADD of the float slot rounds once
// to float at the game's 24-bit precision.
void* cc13_seg48_plane_neighbours_construct_007e1e20(void* neighbours, void* owner, Cc13Seg48Host& host) {
    at<std::uint32_t>(neighbours, 0) = 0x00CFD99Cu;
    at<std::uint32_t>(neighbours, 4) = 0;
    at<std::uint32_t>(neighbours, 8) = 0;
    at<std::uint32_t>(neighbours, 0xC) = 0;
    at<std::uint8_t>(neighbours, 0x10) = 0;
    at<std::uint8_t>(neighbours, 0x11) = 1;
    at<std::uint32_t>(neighbours, 0x14) = 0;
    at<std::uint32_t>(neighbours, 0x18) = 2;
    host.tick_subnode_reparent_00876020(owner != nullptr ? static_cast<std::uint8_t*>(owner) + 0x310 : nullptr,
                                        neighbours);
    at<std::uint32_t>(neighbours, 0) = 0x00D06B88u;
    static constexpr std::size_t kBuffers[] = {0x1C, 0x3C, 0x5C};
    for (std::size_t buffer : kBuffers) {
        at<std::uint32_t>(neighbours, buffer + 4) = 0;
        at<std::uint32_t>(neighbours, buffer + 8) = 0;
        at<std::uint32_t>(neighbours, buffer + 0xC) = 0;
        at<std::uint8_t>(neighbours, buffer + 0x10) = 0;
        at<std::uint32_t>(neighbours, buffer + 0x14) = 0;
        at<std::uint32_t>(neighbours, buffer + 0x18) = 0;
        at<std::uint32_t>(neighbours, buffer + 0x1C) = 0;
        at<std::uint32_t>(neighbours, buffer) = 0x00D06C58u;
    }
    at<float>(neighbours, 0x88) = 3.0f;
    at<float>(neighbours, 0x8C) = 2.0f;
    at<float>(neighbours, 0x90) = kCc13Seg48One;
    at<float>(neighbours, 0x94) = 400.0f;
    float half_cone;
    const std::uint32_t half_cone_bits = 0x3DB2B8C3u;
    static_assert(sizeof half_cone == sizeof half_cone_bits, "float width");
    std::memcpy(&half_cone, &half_cone_bits, sizeof half_cone);
    at<float>(neighbours, 0x98) = host.math_tangent_00412e20(half_cone);
    at<std::uint32_t>(neighbours, 0xA0) = 0;
    at<std::uint32_t>(neighbours, 0xA4) = 0;
    at<std::uint32_t>(neighbours, 0xA8) = 0;
    at<std::uint32_t>(neighbours, 0xB0) = 0;
    at<std::uint8_t>(neighbours, 0xAC) = 1;
    at<std::uint32_t>(neighbours, 0x9C) = 0x00CF89B4u;
    at<float>(neighbours, 0xB8) = 0.0f;
    at<float>(neighbours, 0xBC) = 0.0f;
    at<float>(neighbours, 0xC0) = 0.0f;
    at<float>(neighbours, 0xC4) = 0.0f;
    at<std::uint8_t>(neighbours, 0xC8) = 0;
    at<float>(neighbours, 0x7C) =
        host.random_uniform_float_range_00bd2f10(1, 0.0f, at<float>(neighbours, 0x88)) + at<float>(neighbours, 0x88);
    at<float>(neighbours, 0x84) =
        host.random_uniform_float_range_00bd2f10(1, 0.0f, at<float>(neighbours, 0x8C)) + at<float>(neighbours, 0x8C);
    at<float>(neighbours, 0x80) =
        host.random_uniform_float_range_00bd2f10(1, 0.0f, at<float>(neighbours, 0x90)) + at<float>(neighbours, 0x90);
    return neighbours;
}

// 007E2010 BSP_PlaneNeighbours_AdvanceTimers, __thiscall(ECX = neighbours,
// float dt), RET 4, vtable 00D06B88 slot +0Ch. Bytes 007E2010-007E2065 (x87;
// 007E202D is D8 C2, st0 = st0 + st2; 007E203B is DE C2, st2 = st2 + st0,
// pop): t = float(dt + [+7Ch]) stored to +7Ch; +84h = float(+84h + dt); +80h =
// float(dt + +80h); then FCOMIP t against +88h with JB skipping, so the refresh
// 007E11D0(ECX = neighbours) runs when t >= +88h (ordered) and is followed by
// +7Ch = float(+7Ch - +88h). Float arithmetic: the game runs the x87 at
// 24-bit precision (docs/CC13_FPU_PRECISION.md).
void cc13_seg48_plane_neighbours_advance_timers_007e2010(void* neighbours, float dt, Cc13Seg48Host& host) {
    const float t = dt + at<float>(neighbours, 0x7C);
    at<float>(neighbours, 0x7C) = t;
    at<float>(neighbours, 0x84) = at<float>(neighbours, 0x84) + dt;
    at<float>(neighbours, 0x80) = dt + at<float>(neighbours, 0x80);
    if (t >= at<float>(neighbours, 0x88)) {
        host.plane_neighbours_refresh_007e11d0(neighbours);
        at<float>(neighbours, 0x7C) = at<float>(neighbours, 0x7C) - at<float>(neighbours, 0x88);
    }
}

// 007EB160 BSP_PlaneFxComponent_StartClassLoops, __thiscall(ECX = component),
// EH frame 00C8F198. Bytes 007EB160-007EB265. Only when the live-effect
// count +FCh is 0: for each entry i < [[plane+538h]+254h] (plane = [+4h], both
// re-read every pass) of the class list {[+250h], stride 44h}: the definition
// reference [entry] is copied into the by-value argument slot with an
// InterlockedIncrement ([00CE221C]) when non-null; 008687C0(ECX = &out, EDX =
// [plane+4A4h], that reference, entry+4h, 1, 0, 0); 0074D780(ECX = +F8h,
// the returned &out); then the local out reference is released.
void cc13_seg48_plane_fx_component_start_class_loops_007eb160(void* component, Cc13Seg48Host& host) {
    if (at<std::int32_t>(component, 0xFC) != 0) {
        return;
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(at<void*>(at<void*>(component, 4), 0x538), 0x254); ++i) {
        void* plane = at<void*>(component, 4);
        void* list = at<void*>(plane, 0x538);
        auto* entry = at<std::uint8_t*>(list, 0x250) + 0x44 * i;
        void* parent = at<void*>(plane, 0x4A4);
        void* definition = at<void*>(entry, 0);
        if (definition != nullptr) {
            host.interlocked_increment(&at<std::int32_t>(definition, 4));
        }
        void* out = nullptr;
        void** made = host.point_effect_create_with_parent_matrix_008687c0(&out, parent, definition, entry + 4, 1, 0, 0);
        host.live_effect_references_append_0074d780(static_cast<std::uint8_t*>(component) + 0xF8, made);
        release_slot(&out, 0, host);
    }
}

// 007EAE30 BSP_EffectHolder_EnsureStarted_007eae30, __thiscall(ECX = holder,
// byte restart), RET 4, EH frame 00C8F178. Bytes 007EAE30-007EB018.
// Holder: +4h parent, +8h definition, +0Ch effect, +10h sound reference.
// - Effect present: when restart is 0, return at once; else 00866B70(ECX =
//   effect) and go on to the sound part.
// - No effect: when the definition and the parent are non-null, an identity
//   matrix (1.0f [00D7A24C]) is built on the stack, the definition reference
//   is copied into the by-value slot with InterlockedIncrement ([00CE221C]),
//   008687C0(ECX = &out, EDX = parent, ref, &matrix, 1, 0, 0), then
//   004845D0(ECX = holder+0Ch, &out) and the local out is released.
// - Sound part, when +0Ch is then non-null: the native string "Sound"
//   (00CEA3FC) through 0041E870; 00866CD0(ECX = effect, &out2, &string);
//   006FBEB0(ECX = holder+10h, the returned slot); release out2; return the
//   string's block to the sized pool (00419CC0, 00BD1510(data, size + 1, 1))
//   when its data is non-null. The string local is zeroed here only because
//   C++ needs a value; 0041E870 sets it.
void cc13_seg48_effect_holder_ensure_started_007eae30(void* holder, std::uint8_t restart, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(holder);
    if (at<void*>(holder, 0xC) != nullptr) {
        if (restart == 0) {
            return;
        }
        host.effect_handle_request_stop_00866b70(at<void*>(holder, 0xC));
    } else if (at<void*>(holder, 8) != nullptr) {
        void* parent = at<void*>(holder, 4);
        if (parent != nullptr) {
            float matrix[16] = {};
            matrix[0] = matrix[5] = matrix[10] = matrix[15] = kCc13Seg48One;
            void* definition = at<void*>(holder, 8);
            if (definition != nullptr) {
                host.interlocked_increment(&at<std::int32_t>(definition, 4));
            }
            void* out = nullptr;
            void** made = host.point_effect_create_with_parent_matrix_008687c0(&out, parent, definition, matrix, 1, 0, 0);
            host.retained_reference_assign_004845d0(base + 0xC, made);
            release_slot(&out, 0, host);
        }
    }
    if (at<void*>(holder, 0xC) == nullptr) {
        return;
    }
    struct NativeString {
        std::uint32_t size;
        void* data;
    } name{0, nullptr};
    host.native_string_assign_0041e870(&name, "Sound");
    void* out2 = nullptr;
    void** child = host.point_effect_find_or_create_named_child_00866cd0(at<void*>(holder, 0xC), &out2, &name);
    host.retained_reference_assign_from_slot_006fbeb0(base + 0x10, child);
    release_slot(&out2, 0, host);
    if (name.data != nullptr) {
        host.sized_pool_return_block_00bd1510(host.sized_pool_get_00419cc0(), name.data, name.size + 1, 1);
    }
}

// 007EB270 BSP_PlaneFxComponent_OnFlightState, __thiscall(ECX = component,
// byte flying), RET 4. Bytes 007EB270-007EB34D. gate = 0 when the owner plane
// [+4h] has byte +BC8h = 0, its slot +5Ch(17h) true and byte +C24h = 0 (the
// 007DDCB0 test, inlined), else 1; f = flying & gate (written back into the
// argument slot). +40h = 0.0f; every engine record [+8h][i] (i < [+0Ch]) gets
// slot +4h with the argument dword (low byte f; the upper bytes are the
// caller's, zero here). Then +18h = +20h = (f ? 1.0f : 0.0f); byte +28h = f;
// +2Ch = 0.0f; byte +14h = f; +30h = (f ? 1.0f : 0.0f); +34h = +38h = +18h.
// When f: 007EAE30(ECX = +48h, 1), +68h = +70h = 1.0f, 007EB160(ECX = component).
void cc13_seg48_plane_fx_component_on_flight_state_007eb270(void* component, std::uint8_t flying,
                                                            Cc13Seg48Host& host) {
    void* plane = at<void*>(component, 4);
    std::uint8_t gate = 1;
    if (at<std::uint8_t>(plane, 0xBC8) == 0 && host.vcall_5c_has_ordnance_kind(plane, 0x17) &&
        at<std::uint8_t>(plane, 0xC24) == 0) {
        gate = 0;
    }
    const std::uint8_t f = static_cast<std::uint8_t>(flying & gate);
    at<float>(component, 0x40) = 0.0f;
    for (std::int32_t i = 0; i < at<std::int32_t>(component, 0xC); ++i) {
        host.vcall_04_engine(at<void**>(component, 8)[i], f);
    }
    const float level = f != 0 ? kCc13Seg48One : 0.0f;
    at<float>(component, 0x18) = level;
    at<float>(component, 0x20) = level;
    at<std::uint8_t>(component, 0x28) = f;
    at<float>(component, 0x2C) = 0.0f;
    at<std::uint8_t>(component, 0x14) = f;
    at<float>(component, 0x30) = f != 0 ? kCc13Seg48One : 0.0f;
    at<float>(component, 0x34) = at<float>(component, 0x18);
    at<float>(component, 0x38) = at<float>(component, 0x18);
    if (f != 0) {
        host.effect_holder_ensure_started_007eae30(static_cast<std::uint8_t*>(component) + 0x48, 1);
        at<float>(component, 0x68) = kCc13Seg48One;
        at<float>(component, 0x70) = kCc13Seg48One;
        host.plane_fx_component_start_class_loops_007eb160(component);
    }
}

// ------------------------------------------------- reference pointers, effects

// 007E1020 BSP_RefPtr_Assign_007e1020, __thiscall(ECX = slot, value), RET 4,
// EAX = slot: releases the old pointee (InterlockedDecrement of +4h, slot 0
// at zero) and stores `value` without adding a reference.
void* cc13_seg48_ref_ptr_assign_007e1020(void* slot, void* value, Cc13Seg48Host& host) {
    release_slot(slot, 0, host);
    at<void*>(slot, 0) = value;
    return slot;
}

// 007E2360 BSP_RefPtr_ReleaseAt4_007e2360, __thiscall(ECX = holder): release
// [holder+4h] and clear it. Bytes 007E2360-007E238A.
void cc13_seg48_ref_ptr_release_at4_007e2360(void* holder, Cc13Seg48Host& host) {
    release_slot(holder, 4, host);
}

// 007F6FA0 BSP_RefPtr_ReleaseAt8_007f6fa0, __thiscall(ECX = holder): release
// [holder+8h] and clear it.
void cc13_seg48_ref_ptr_release_at8_007f6fa0(void* holder, Cc13Seg48Host& host) {
    release_slot(holder, 8, host);
}

// 007F7010 BSP_RefPtr_ReleaseAt8_007f7010: a second instantiation of 007F6FA0.
void cc13_seg48_ref_ptr_release_at8_007f7010(void* holder, Cc13Seg48Host& host) {
    release_slot(holder, 8, host);
}

// 007E1B30 BSP_EffectHolder_StopEffectC_007e1b30, __thiscall(ECX = holder).
// Bytes 007E1B30-007E1B99: the image tests `-(effect != 0) & 00E186ECh`, which
// is non-zero exactly when the effect at +0Ch is non-null. Then
// 00867B10(ECX = effect), byte effect+9 = 1, release +0Ch and clear it (the
// image clears it a second time), release +10h and clear it (again twice).
// Nothing happens when +0Ch is null.
void cc13_seg48_effect_holder_stop_effect_c_007e1b30(void* holder, Cc13Seg48Host& host) {
    void* effect = at<void*>(holder, 0xC);
    if (effect == nullptr) {
        return;
    }
    host.point_effect_stop_children_00867b10(effect);
    at<std::uint8_t>(effect, 9) = 1;
    release_slot(holder, 0xC, host);
    at<void*>(holder, 0xC) = nullptr;
    release_slot(holder, 0x10, host);
    at<void*>(holder, 0x10) = nullptr;
}

// 007E2240 BSP_EffectVector_Destruct_00D05AB8, __thiscall(ECX = holder), EH
// frame 00C8DA01: vtable 00D05AB8; 007E1B30(ECX = holder); release +10h,
// +0Ch, +8h in that order.
void cc13_seg48_effect_vector_destruct_007e2240(void* holder, Cc13Seg48Host& host) {
    at<std::uint32_t>(holder, 0) = 0x00D05AB8u;
    cc13_seg48_effect_holder_stop_effect_c_007e1b30(holder, host);
    release_slot(holder, 0x10, host);
    release_slot(holder, 0xC, host);
    release_slot(holder, 8, host);
}

// 007E2300 BSP_PlaneEngineBase_Bind, __thiscall(ECX = engine, owner, const
// ref* definition, value_24, value_18), RET 10h. +4h = owner; the counted
// reference at +8h is replaced by *definition when different (increment the
// new one first, [00CE221C], then release the old one); +24h = value_24;
// +18h = value_18.
void cc13_seg48_plane_engine_base_bind_007e2300(void* engine, std::uint32_t owner, void* const* definition,
                                                std::uint32_t value_24, std::uint32_t value_18,
                                                Cc13Seg48Host& host) {
    at<std::uint32_t>(engine, 4) = owner;
    void* incoming = *definition;
    void* old = at<void*>(engine, 8);
    if (old != incoming) {
        at<void*>(engine, 8) = incoming;
        if (incoming != nullptr) {
            host.interlocked_increment(&at<std::int32_t>(incoming, 4));
        }
        if (old != nullptr && host.interlocked_decrement(&at<std::int32_t>(old, 4)) == 0) {
            host.vcall_00_release(old);
        }
    }
    at<std::uint32_t>(engine, 0x24) = value_24;
    at<std::uint32_t>(engine, 0x18) = value_18;
}

// 007E2550 BSP_PlaneEffectSlot_Destruct_007e2550, __thiscall(ECX = slot), EH
// frame 00C8DA43: when the effect at +4h is non-null (the 00E186ECh mask test),
// 00867B10(ECX = effect) and byte effect+9 = 1; then release +4h and +0h.
void cc13_seg48_plane_effect_slot_destruct_007e2550(void* slot, Cc13Seg48Host& host) {
    void* effect = at<void*>(slot, 4);
    if (effect != nullptr) {
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 9) = 1;
    }
    release_slot(slot, 4, host);
    release_slot(slot, 0, host);
}

// 007EB0E0 BSP_PlaneFxComponent_StopAllEffects, __thiscall(ECX = component):
// pops the live-effect vector {data +F8h, count +FCh} from the back: while
// count != 0, stop the last effect (00867B10, byte +9 = 1), then (re-reading
// count, non-zero) release that element, clear it, count -= 1.
void cc13_seg48_plane_fx_component_stop_all_effects_007eb0e0(void* component, Cc13Seg48Host& host) {
    while (at<std::int32_t>(component, 0xFC) != 0) {
        void* effect = at<void**>(component, 0xF8)[at<std::int32_t>(component, 0xFC) - 1];
        host.point_effect_stop_children_00867b10(effect);
        at<std::uint8_t>(effect, 9) = 1;
        const std::int32_t count = at<std::int32_t>(component, 0xFC);
        if (count != 0) {
            void** data = at<void**>(component, 0xF8);
            release_slot(data, static_cast<std::size_t>(count - 1) * 4, host);
            at<std::int32_t>(component, 0xFC) -= 1;
        }
    }
}

// 007EB350 BSP_PlaneFxComponent_Restart, __thiscall(ECX = component): 007EAE30
// (ECX = component+48h, 1), then tail-jumps to 007EB160(ECX = component).
void cc13_seg48_plane_fx_component_restart_007eb350(void* component, Cc13Seg48Host& host) {
    host.effect_holder_ensure_started_007eae30(static_cast<std::uint8_t*>(component) + 0x48, 1);
    host.plane_fx_component_start_class_loops_007eb160(component);
}

// 007EC490 BSP_PlaneFxComponent_Destruct, __thiscall(ECX = component), EH
// frame 00C8F1FB. Bytes 007EC490-007EC59A: vtable 00D05E94; 007EB0E0; for each
// of the `[+0Ch]` engine records at [+8h][i]: when non-null, release its +4h
// reference (clear it) and _free (00BF65AC) the record; then
// 004C9550(ECX = +F8h, 0) and _free (00BF6989) of [+F8h]; the four effect
// holders 007E2240 at +CCh, +A0h, +74h, +48h; 007C3000(ECX = +8h, 0); _free
// (00BF6989) of [+8h].
void cc13_seg48_plane_fx_component_destruct_007ec490(void* component, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(component);
    at<std::uint32_t>(component, 0) = 0x00D05E94u;
    cc13_seg48_plane_fx_component_stop_all_effects_007eb0e0(component, host);
    for (std::int32_t i = 0; i < at<std::int32_t>(component, 0xC); ++i) {
        void* record = at<void**>(component, 8)[i];
        if (record != nullptr) {
            release_slot(record, 4, host);
            host.crt_free_00bf65ac(record);
        }
    }
    host.live_effect_references_resize_004c9550(base + 0xF8, 0);
    host.crt_free_00bf6989(at<void*>(component, 0xF8));
    cc13_seg48_effect_vector_destruct_007e2240(base + 0xCC, host);
    cc13_seg48_effect_vector_destruct_007e2240(base + 0xA0, host);
    cc13_seg48_effect_vector_destruct_007e2240(base + 0x74, host);
    cc13_seg48_effect_vector_destruct_007e2240(base + 0x48, host);
    host.fun_007c3000(base + 8, 0);
    host.crt_free_00bf6989(at<void*>(component, 8));
}

// ---------------------------------------------------------------- squadrons
// Squadron member array: count at +3CCh, unit pointers from +3D0h.

// 007ECF00 BSP_CommandTargetPair_Construct, __thiscall(ECX = pair), EAX =
// pair: vtable 00CFDB90, then 00721070 on +10h and +28h.
void* cc13_seg48_command_target_pair_construct_007ecf00(void* pair, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(pair);
    at<std::uint32_t>(pair, 0) = 0x00CFDB90u;
    host.command_target_descriptor_init_00721070(base + 0x10);
    host.command_target_descriptor_init_00721070(base + 0x28);
    return pair;
}

// 007ECF20 BSP_CommandTargetPair_D086F4_Construct: 007ECF00's body, then the
// derived vtable 00D086F4.
void* cc13_seg48_command_target_pair_d086f4_construct_007ecf20(void* pair, Cc13Seg48Host& host) {
    cc13_seg48_command_target_pair_construct_007ecf00(pair, host);
    at<std::uint32_t>(pair, 0) = 0x00D086F4u;
    return pair;
}

// 007ED020 BSP_PlaneSquadron_BroadcastSlot148, __thiscall(ECX = squadron, a,
// b), RET 8: 00927D20(ECX = squadron, a, b), then every member's slot +148h(a, b).
void cc13_seg48_plane_squadron_broadcast_slot148_007ed020(void* squadron, std::uint32_t a, std::uint32_t b,
                                                          Cc13Seg48Host& host) {
    host.fun_00927d20(squadron, a, b);
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        host.vcall_148(at<void*>(squadron, 0x3D0 + 4 * i), a, b);
    }
}

// 007ED070 BSP_PlaneSquadron_BroadcastSlot14C, __thiscall(ECX = squadron, a,
// b, c), RET 0Ch: 00927E50(ECX = squadron, a, b, c), then every member's slot
// +14Ch(a, b, c).
void cc13_seg48_plane_squadron_broadcast_slot14c_007ed070(void* squadron, std::uint32_t a, std::uint32_t b,
                                                          std::uint32_t c, Cc13Seg48Host& host) {
    host.fun_00927e50(squadron, a, b, c);
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        host.vcall_14c(at<void*>(squadron, 0x3D0 + 4 * i), a, b, c);
    }
}

// 007ED160 BSP_PlaneSquadron_MemberIndexOf, __thiscall(ECX = squadron, unit),
// RET 4, EAX: the member index, or -1.
std::int32_t cc13_seg48_plane_squadron_member_index_of_007ed160(void* squadron, std::uint32_t member) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        if (at<std::uint32_t>(squadron, 0x3D0 + 4 * i) == member) {
            return i;
        }
    }
    return -1;
}

// 007ED1D0 BSP_PlaneSquadron_BroadcastSlot40, __thiscall(ECX = squadron, a),
// RET 4: every member's slot +40h(a). No base call.
void cc13_seg48_plane_squadron_broadcast_slot40_007ed1d0(void* squadron, std::uint32_t a, Cc13Seg48Host& host) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        host.vcall_40(at<void*>(squadron, 0x3D0 + 4 * i), a);
    }
}

// 007ED3E0 BSP_PlaneSquadron_SetFlag361, __thiscall(ECX = squadron): byte +361h = 1.
void cc13_seg48_plane_squadron_set_flag361_007ed3e0(void* squadron) {
    at<std::uint8_t>(squadron, 0x361) = 1;
}

// 007ED500 BSP_PlaneSquadron_LeaderFirstOrdnance, __thiscall(ECX = squadron),
// EAX: tail-jumps to 007C3F60(ECX = first member) when [+3D0h] is non-null,
// else 0. The member count is not consulted.
std::uint32_t cc13_seg48_plane_squadron_leader_first_ordnance_007ed500(void* squadron, Cc13Seg48Host& host) {
    void* leader = at<void*>(squadron, 0x3D0);
    if (leader != nullptr) {
        return host.weapon_controller_first_slot_ordnance_007c3f60(leader);
    }
    return 0;
}

// 007ED9C0 BSP_PlaneSquadron_AnyReadyRocket, __thiscall(ECX = squadron, arg),
// RET 4, AL: true at the first member whose 007B9410(ECX = member, arg) is
// true. Bytes 007ED9C0-007EDA03.
bool cc13_seg48_plane_squadron_any_ready_rocket_007ed9c0(void* squadron, std::uint32_t arg, Cc13Seg48Host& host) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        if (host.weapon_controller_find_ready_rocket_007b9410(at<void*>(squadron, 0x3D0 + 4 * i), arg)) {
            return true;
        }
    }
    return false;
}

// 007ECFD0 BSP_PlaneSquadron_GetAiCommandBlock, __thiscall(ECX = squadron),
// vtable slot +114h: EAX = [+348h].
std::uint32_t cc13_seg48_plane_squadron_get_ai_command_block_007ecfd0(void* squadron) {
    return at<std::uint32_t>(squadron, 0x348);
}

// 007ED580 BSP_PlaneSquadron_CurrentCommandToken, __thiscall(ECX = squadron):
// tail-jumps to 0071BE40 with ECX = [+348h] (no null test).
std::uint32_t cc13_seg48_plane_squadron_current_command_token_007ed580(void* squadron, Cc13Seg48Host& host) {
    return host.weapon_director_current_command_0071be40(at<void*>(squadron, 0x348));
}

// 007ED590 BSP_PlaneSquadron_ResetTravelSpeed, __thiscall(ECX = squadron):
// +388h = 0.5f ([00CE3800], bytes 00 00 00 3F); +3A0h = [[+35Ch]+190h];
// byte +3ACh = 1; byte +38Fh = 0.
void cc13_seg48_plane_squadron_reset_travel_speed_007ed590(void* squadron) {
    const std::uint32_t speed = at<std::uint32_t>(at<void*>(squadron, 0x35C), 0x190);
    at<float>(squadron, 0x388) = 0.5f;
    at<std::uint32_t>(squadron, 0x3A0) = speed;
    at<std::uint8_t>(squadron, 0x3AC) = 1;
    at<std::uint8_t>(squadron, 0x38F) = 0;
}

// 007ED5D0 BSP_PlaneSquadron_RefreshTravelSpeed, __thiscall(ECX = squadron).
// Bytes 007ED5D0-007ED607: only when byte +38Fh is 0: `COMISS 0.0, [+388h]`
// then JBE skips, so the copy runs when 0.0 > [+388h] (ordered; a NaN skips)
// and byte +3ACh is 0: +3A0h = [[+35Ch]+190h] (FLD/FSTP float). Then byte
// +3ACh = 0 (inside the +38Fh test).
void cc13_seg48_plane_squadron_refresh_travel_speed_007ed5d0(void* squadron) {
    if (at<std::uint8_t>(squadron, 0x38F) != 0) {
        return;
    }
    if (0.0f > at<float>(squadron, 0x388) && at<std::uint8_t>(squadron, 0x3AC) == 0) {
        at<float>(squadron, 0x3A0) = at<float>(at<void*>(squadron, 0x35C), 0x190);
    }
    at<std::uint8_t>(squadron, 0x3AC) = 0;
}

// 007ED690 BSP_PlaneSquadron_ReEquipAll, __thiscall(ECX = squadron, int ammo
// type), RET 4: 007BCB30(ECX = member, ammo type) for every member, then byte
// +3ECh = 1 (also when there are no members).
void cc13_seg48_plane_squadron_re_equip_all_007ed690(void* squadron, std::uint32_t ammo_type, Cc13Seg48Host& host) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        host.plane_re_equip_007bcb30(at<void*>(squadron, 0x3D0 + 4 * i), ammo_type);
    }
    at<std::uint8_t>(squadron, 0x3EC) = 1;
}

// 007ED6E0 BSP_PlaneSquadron_StowMembersInside, __thiscall(ECX = squadron):
// byte +408h = 1 first, then 007C2130(ECX = member) for every member.
void cc13_seg48_plane_squadron_stow_members_inside_007ed6e0(void* squadron, Cc13Seg48Host& host) {
    at<std::uint8_t>(squadron, 0x408) = 1;
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        host.plane_request_flight_state_one_007c2130(at<void*>(squadron, 0x3D0 + 4 * i));
    }
}

// 007ED740 BSP_PlaneSquadron_AllFreeFlightOrParked, __thiscall(ECX =
// squadron), AL. Bytes 007ED740-007ED782: false at the first member whose
// 007B8BD0 answers false; after the loop, `setg` on count > 0 (so an empty
// squadron answers false).
bool cc13_seg48_plane_squadron_all_free_flight_or_parked_007ed740(void* squadron, Cc13Seg48Host& host) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        if (!host.plane_is_free_flight_or_parked_007b8bd0(at<void*>(squadron, 0x3D0 + 4 * i))) {
            return false;
        }
    }
    return at<std::int32_t>(squadron, 0x3CC) > 0;
}

// 007EDAC0 BSP_PlaneSquadron_LeaderFlagC24, __thiscall(ECX = squadron), AL =
// byte [[+3D0h]+C24h] (no null test; EAX's upper bytes are the pointer's).
std::uint8_t cc13_seg48_plane_squadron_leader_flag_c24_007edac0(void* squadron) {
    return at<std::uint8_t>(at<void*>(squadron, 0x3D0), 0xC24);
}

// 007F3500 BSP_PlaneSquadron_CloneFrom, __thiscall(ECX = source), EAX = the
// copy, EH frame 00C8F508. Bytes 007F3500-007F35B5: operator new (00BF55BE)
// 414h bytes, memset (00BF79F0) 0 before the null test; when non-null
// 007F2C60(ECX = block, zero-extended word [source+1Ch]); then, with the copy
// (null when the allocation failed; the image does not test it again),
// 0041DD40(ECX = copy+154h, strlen([source+C0h]) or 0 when that pointer is
// null, 0); when the string data [copy+158h] is non-null, memcpy (00BF7680) of
// [copy+154h] bytes (the resized length, no terminator) from [source+C0h];
// byte copy+369h = byte source+124h.
void* cc13_seg48_plane_squadron_clone_from_007f3500(void* source, Cc13Seg48Host& host) {
    void* block = host.operator_new_00bf55be(0x414);
    host.crt_memset_00bf79f0(block, 0, 0x414);
    void* copy = nullptr;
    if (block != nullptr) {
        copy = host.plane_squadron_construct_007f2c60(block, at<std::uint16_t>(source, 0x1C));
    }
    const char* name = at<const char*>(source, 0xC0);
    std::uint32_t length = 0;
    if (name != nullptr) {
        while (name[length] != '\0') {
            ++length;
        }
    }
    auto* string = static_cast<std::uint8_t*>(copy) + 0x154;
    host.native_string_resize_0041dd40(string, length, 0);
    if (at<void*>(string, 4) != nullptr) {
        host.crt_memcpy_00bf7680(at<void*>(string, 4), name, at<std::uint32_t>(string, 0));
    }
    at<std::uint8_t>(copy, 0x369) = at<std::uint8_t>(source, 0x124);
    return copy;
}

// 007F3A60 BSP_PlaneSquadron_ReleaseControlledUnit, __thiscall(ECX =
// squadron), vtable slot +7Ch. Bytes 007F3A60-007F3AFC. Only when the squadron
// is the controlled unit [00E188D8]: when byte [[[00E188A8]+19CCh]+4ACh] is set
// and the front-end manager M = [00E198C4] has M+20h not in {2Bh, 2Ch, 2Dh,
// 29h}: 00565FB0(ECX = [M+E8h], [squadron+70h]); then (M re-read) when
// M+4h == M+20h and M+1Ch == M+38h, 004CC460(ECX = M, 34h, 0); then (M
// re-read) unless M+4h == M+20h and M+1Ch == M+38h, M's slot +10h(M+20h,
// M+38h). Whatever that inner test gave, 004C0890(ECX = null). Finally, for
// every squadron, tail-jump to 0077D270(ECX = squadron).
void cc13_seg48_plane_squadron_release_controlled_unit_007f3a60(void* squadron, Cc13Seg48Host& host) {
    const auto self = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(squadron));
    if (self == *host.image_global(0x00E188D8u)) {
        void* game = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*host.image_global(0x00E188A8u)));
        if (at<std::uint8_t>(at<void*>(game, 0x19CC), 0x4AC) != 0) {
            void* manager = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*host.image_global(0x00E198C4u)));
            const std::uint32_t screen = at<std::uint32_t>(manager, 0x20);
            if (screen != 0x2B && screen != 0x2C && screen != 0x2D && screen != 0x29) {
                host.hud_limbo_screen_set_state_00565fb0(at<void*>(manager, 0xE8), at<std::uint32_t>(squadron, 0x70));
                manager = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*host.image_global(0x00E198C4u)));
                if (at<std::uint32_t>(manager, 4) == at<std::uint32_t>(manager, 0x20) &&
                    at<std::uint32_t>(manager, 0x1C) == at<std::uint32_t>(manager, 0x38)) {
                    host.front_end_push_interface_request_004cc460(manager, 0x34, 0);
                    manager = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*host.image_global(0x00E198C4u)));
                }
                if (at<std::uint32_t>(manager, 4) != at<std::uint32_t>(manager, 0x20) ||
                    at<std::uint32_t>(manager, 0x1C) != at<std::uint32_t>(manager, 0x38)) {
                    host.vcall_10_front_end(manager, at<std::uint32_t>(manager, 0x20), at<std::uint32_t>(manager, 0x38));
                }
            }
        }
        host.game_set_controlled_unit_004c0890(nullptr);
    }
    host.unit_instance_on_killed_lua_0077d270(squadron);
}

// The 007ED790 ordnance-loop family. All eight share one body (checked byte
// for byte apart from the call target): ebx = squadron; esi = 0; when
// [ebx+3CCh] > 0, edi = ebx+3D0h, loop { ECX = [edi]; push loadout; call
// predicate; AL != 0 -> return 1; esi += 1; edi += 4 } while esi < [ebx+3CCh];
// return 0. RET 4.
namespace {

template <class Predicate>
bool any_member(void* squadron, Predicate predicate) {
    for (std::int32_t i = 0; i < at<std::int32_t>(squadron, 0x3CC); ++i) {
        if (predicate(at<void*>(squadron, 0x3D0 + 4 * i))) {
            return true;
        }
    }
    return false;
}

}  // namespace

// 007ED790 BSP_PlaneSquadron_AnyCanDropOrdnance: 007B9140 per member
// (bytes 007ED790-007ED7D9).
bool cc13_seg48_plane_squadron_any_can_drop_ordnance_007ed790(void* squadron, std::uint32_t loadout,
                                                              Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) { return host.unit_can_drop_ordnance_007b9140(m, loadout); });
}

// 007ED7E0 BSP_Unit_HasGeneralBombOrdnance: 007B9320 (kind 2Ah) per member.
bool cc13_seg48_unit_has_general_bomb_ordnance_007ed7e0(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_general_bomb_ordnance_007b9320(m, loadout);
    });
}

// 007ED830 BSP_Unit_HasLevelBombOrdnance: 007B9500 (kind 31h) per member
// (bytes 007ED830-007ED876; ECX = [edi], edi += 4 at 007ED85F).
bool cc13_seg48_unit_has_level_bomb_ordnance_007ed830(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_level_bomb_ordnance_007b9500(m, loadout);
    });
}

// 007ED880 BSP_Unit_HasDropKamikazeOrdnance: 007B93E0 (kind 2Fh) per member.
bool cc13_seg48_unit_has_drop_kamikaze_ordnance_007ed880(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_drop_kamikaze_ordnance_007b93e0(m, loadout);
    });
}

// 007ED8D0 BSP_Unit_HasTorpedoOrdnance: 007B93F0 (kind 2Bh) per member.
bool cc13_seg48_unit_has_torpedo_ordnance_007ed8d0(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_torpedo_ordnance_007b93f0(m, loadout);
    });
}

// 007ED920 BSP_PlaneSquadron_AnyHasRockets: 007B9400 (kind 33h) per member.
bool cc13_seg48_plane_squadron_any_has_rockets_007ed920(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_rocket_kind33_007b9400(m, loadout);
    });
}

// 007ED970 BSP_Unit_HasRocketOrdnance: 007B9480 per member.
bool cc13_seg48_unit_has_rocket_ordnance_007ed970(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_rocket_ordnance_007b9480(m, loadout);
    });
}

// 007EDA10 BSP_Unit_HasDepthChargeOrdnance: 007B94F0 (kind 2Ch) per member.
bool cc13_seg48_unit_has_depth_charge_ordnance_007eda10(void* squadron, std::uint32_t loadout, Cc13Seg48Host& host) {
    return any_member(squadron, [&](void* m) {
        return host.weapon_controller_has_depth_charge_ordnance_007b94f0(m, loadout);
    });
}

// ---------------------------------------------------------- session messages

// 007EDB80 BSP_SessionMessage_PLANESQUADRON_CREATE_ConstructForRead,
// __thiscall(ECX = message), EAX = message, EH frame 00C8F283. Bytes
// 007EDB80-007EDBFE: 00766F60(ECX = message); vtable 00D08740; a 40h-byte
// block from operator new 00BF681B, constructed in place as 007ECF20 does
// (vtable 00CFDB90, 00721070 on +10h and +28h, vtable 00D086F4) when non-null;
// +118h = the block (or null); byte +124h = 1.
void* cc13_seg48_session_message_planesquadron_create_construct_for_read_007edb80(void* message,
                                                                                  Cc13Seg48Host& host) {
    host.net_entity_create_message_construct_for_read_00766f60(message);
    at<std::uint32_t>(message, 0) = 0x00D08740u;
    void* pair = host.operator_new_00bf681b(0x40);
    if (pair != nullptr) {
        cc13_seg48_command_target_pair_d086f4_construct_007ecf20(pair, host);
    }
    at<void*>(message, 0x118) = pair;
    at<std::uint8_t>(message, 0x124) = 1;
    return message;
}

// 007EDC10 BSP_SessionMessage_PLANESQUADRON_CREATE_Destruct, __thiscall(ECX =
// message): vtable 00D08740; _free (00BF65AC) of [+118h] unconditionally and
// without running the pair's destructor; tail-jumps to 007668E0(ECX = message).
void cc13_seg48_session_message_planesquadron_create_destruct_007edc10(void* message, Cc13Seg48Host& host) {
    void* pair = at<void*>(message, 0x118);
    at<std::uint32_t>(message, 0) = 0x00D08740u;
    host.crt_free_00bf65ac(pair);
    host.net_entity_create_message_destruct_007668e0(message);
}

// 007EDF50 BSP_SessionMessage_UNITEXITED_Construct, __thiscall(ECX = message,
// unit id), RET 4, EAX = message: 0075B430(ECX = message, 50h); word +18h = 0,
// byte +1Ah = 0, byte +1Ch = 0; +20h = unit id; +4h = 1; vtable 00D02CE0.
void* cc13_seg48_session_message_unitexited_construct_007edf50(void* message, std::uint32_t unit_id,
                                                               Cc13Seg48Host& host) {
    host.session_message_construct_base_0075b430(message, 0x50);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint32_t>(message, 0x20) = unit_id;
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint32_t>(message, 0) = 0x00D02CE0u;
    return message;
}

// 007EDEB0 BSP_SessionMessage_ENTITY_RELOCATE_ConstructHeading,
// __thiscall(ECX = message, float heading, value), RET 8, EAX = message:
// 0075B430(ECX = message, 97h); word +18h = 0, byte +1Ah = 0, byte +1Ch = 0,
// byte +21h = 0; +28h = value; +4h = 1; vtable 00D02F24; byte +20h = 1 (the
// heading form); +24h = heading.
void* cc13_seg48_session_message_entity_relocate_construct_heading_007edeb0(void* message, float heading,
                                                                            std::uint32_t value_28,
                                                                            Cc13Seg48Host& host) {
    host.session_message_construct_base_0075b430(message, 0x97);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint8_t>(message, 0x21) = 0;
    at<std::uint32_t>(message, 0x28) = value_28;
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint32_t>(message, 0) = 0x00D02F24u;
    at<std::uint8_t>(message, 0x20) = 1;
    at<float>(message, 0x24) = heading;
    return message;
}

// 007EDF00 BSP_SessionMessage_ENTITY_RELOCATE_ConstructPosition,
// __thiscall(ECX = message, const float[3] position, value), RET 8: the same
// base 97h; word +18h = 0, bytes +1Ah, +1Ch, +20h = 0; +28h = value; vtable
// 00D02F24; +4h = 1; byte +21h = 1 (the position form); +2Ch..+34h = position.
void* cc13_seg48_session_message_entity_relocate_construct_position_007edf00(void* message, const float* position,
                                                                             std::uint32_t value_28,
                                                                             Cc13Seg48Host& host) {
    host.session_message_construct_base_0075b430(message, 0x97);
    at<std::uint16_t>(message, 0x18) = 0;
    at<std::uint8_t>(message, 0x1A) = 0;
    at<std::uint8_t>(message, 0x1C) = 0;
    at<std::uint8_t>(message, 0x20) = 0;
    at<std::uint32_t>(message, 0x28) = value_28;
    at<std::uint32_t>(message, 0) = 0x00D02F24u;
    at<std::uint32_t>(message, 4) = 1;
    at<std::uint8_t>(message, 0x21) = 1;
    const auto* words = reinterpret_cast<const std::uint32_t*>(position);
    at<std::uint32_t>(message, 0x2C) = words[0];
    at<std::uint32_t>(message, 0x30) = words[1];
    at<std::uint32_t>(message, 0x34) = words[2];
    return message;
}

// 007F7CF0 BSP_SessionMessageByteVec3Word_WriteBits, __thiscall(ECX = message,
// cursor), RET 4: 00428FF0(ECX = cursor, zero-extended byte +10h, 8); the
// three dwords +18h..+20h and then +24h through 00429090(ECX = cursor, value, 20h).
void cc13_seg48_session_message_byte_vec3_word_write_bits_007f7cf0(void* message, void* cursor,
                                                                   Cc13Seg48Host& host) {
    host.bit_cursor_write_byte_bits_00428ff0(cursor, at<std::uint8_t>(message, 0x10), 8);
    for (int i = 0; i < 3; ++i) {
        host.bit_cursor_write_signed_dword_bits_00429090(cursor, at<std::uint32_t>(message, 0x18 + 4 * i), 0x20);
    }
    host.bit_cursor_write_signed_dword_bits_00429090(cursor, at<std::uint32_t>(message, 0x24), 0x20);
}

// 007F7D40 BSP_SessionMessageByteVec3Word_ReadBits, __thiscall(ECX = message,
// stream), RET 4: the cursor is stream+4h (bytes 007F7D44-007F7D4A; the writer
// takes the cursor itself). 00428C70(ECX = cursor, &byte +10h, 8); then
// 00428D30(ECX = cursor, &dword, 20h) on +18h, +1Ch, +20h, +24h.
void cc13_seg48_session_message_byte_vec3_word_read_bits_007f7d40(void* message, void* stream,
                                                                  Cc13Seg48Host& host) {
    void* cursor = static_cast<std::uint8_t*>(stream) + 4;
    host.bit_cursor_read_unsigned_byte_00428c70(cursor, &at<std::uint8_t>(message, 0x10), 8);
    for (int i = 0; i < 3; ++i) {
        host.bit_cursor_read_signed_dword_00428d30(cursor, &at<std::uint32_t>(message, 0x18 + 4 * i), 0x20);
    }
    host.bit_cursor_read_signed_dword_00428d30(cursor, &at<std::uint32_t>(message, 0x24), 0x20);
}

// --------------------------------------------------- strings, gun platforms

// 007F3DF0 BSP_DebugString_FromInt, __fastcall(ECX = out native string, EDX =
// value), EAX = out. Bytes 007F3DF0-007F3E4E: 00467310(char buffer[32],
// "%d" at 00CE3A34, value); the inlined empty-string init (capacity +18h = 0Fh,
// size +14h = 0, byte +4h = 0); 00408720(ECX = out, buffer, strlen(buffer)).
void* cc13_seg48_debug_string_from_int_007f3df0(void* out, std::int32_t value, Cc13Seg48Host& host) {
    char buffer[32];
    host.format_to_buffer32_00467310(buffer, "%d", value);
    at<std::uint32_t>(out, 0x18) = 0xF;
    at<std::uint32_t>(out, 0x14) = 0;
    at<std::uint8_t>(out, 4) = 0;
    std::uint32_t length = 0;
    while (buffer[length] != '\0') {
        ++length;
    }
    host.character_string_assign_counted_00408720(out, buffer, length);
    return out;
}

// 007F7300 BSP_GunPlatform_Destruct, __thiscall(ECX = platform), EH frame
// 00C8F763. Bytes 007F7300-007F73A1: vtable 00D08BB0; [+8Ch] _free (00BF6989)
// and cleared when non-null; [+3Ch] _free (00BF6989) when non-null (kept);
// [+2Ch] _free (00BF65AC) when non-null, then +2Ch, +30h, +34h = 0; the list
// at +1Ch cleared (007F7070) and its head node [+20h] _free (00BF65AC)
// unconditionally, [+20h] = 0; 00487110(ECX = +10h); 00BD30F0(ECX = platform).
void cc13_seg48_gun_platform_destruct_007f7300(void* platform, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(platform);
    at<std::uint32_t>(platform, 0) = 0x00D08BB0u;
    if (at<void*>(platform, 0x8C) != nullptr) {
        host.crt_free_00bf6989(at<void*>(platform, 0x8C));
        at<void*>(platform, 0x8C) = nullptr;
    }
    if (at<void*>(platform, 0x3C) != nullptr) {
        host.crt_free_00bf6989(at<void*>(platform, 0x3C));
    }
    if (at<void*>(platform, 0x2C) != nullptr) {
        host.crt_free_00bf65ac(at<void*>(platform, 0x2C));
    }
    at<std::uint32_t>(platform, 0x2C) = 0;
    at<std::uint32_t>(platform, 0x30) = 0;
    at<std::uint32_t>(platform, 0x34) = 0;
    host.list_clear_007f7070(base + 0x1C);
    host.crt_free_00bf65ac(at<void*>(platform, 0x20));
    at<void*>(platform, 0x20) = nullptr;
    host.award_index_list_destroy_actual_storage_00487110(base + 0x10);
    host.refcounted_destroy_base_00bd30f0(platform);
}

namespace {

// The 20-byte-element growth of 007F5800 / 007F6A50 / 007F7110: capacity =
// capacity * 2 + 2, operator new (00BF55BE) capacity * 14h, copy `count`
// elements (each guarded by the destination being non-null), _free
// (00BF6989) the old block when non-null, data = new block.
void grow20(void* array, Cc13Seg48Host& host) {
    const std::int32_t capacity = at<std::int32_t>(array, 8) * 2 + 2;
    at<std::int32_t>(array, 8) = capacity;
    auto* block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(static_cast<std::uint32_t>(capacity) * 0x14u));
    std::uint32_t* out = block;
    for (std::uint32_t i = 0; i < at<std::uint32_t>(array, 4); ++i, out += 5) {
        if (out != nullptr) {
            const std::uint32_t* in = at<std::uint32_t*>(array, 0) + 5 * i;
            for (int k = 0; k < 5; ++k) {
                out[k] = in[k];
            }
        }
    }
    if (at<void*>(array, 0) != nullptr) {
        host.crt_free_00bf6989(at<void*>(array, 0));
    }
    at<std::uint32_t*>(array, 0) = block;
}

void push_back20(void* array, const std::uint32_t* element, Cc13Seg48Host& host) {
    if (at<std::int32_t>(array, 4) == at<std::int32_t>(array, 8)) {
        grow20(array, host);
    }
    std::uint32_t* slot = at<std::uint32_t*>(array, 0) + 5 * at<std::uint32_t>(array, 4);
    if (slot != nullptr) {
        for (int k = 0; k < 5; ++k) {
            slot[k] = element[k];
        }
    }
    at<std::uint32_t>(array, 4) += 1;
}

}  // namespace

// 007F5800 BSP_PodArray20_Grow_007f5800, __thiscall(ECX = array).
void cc13_seg48_pod_array20_grow_007f5800(void* array, Cc13Seg48Host& host) {
    grow20(array, host);
}

// 007F5580 BSP_PodArray20_Insert_007f5580, __thiscall(ECX = array, element*
// where, const element* value), RET 8, EAX = data + index * 14h. Bytes
// 007F5580-007F5738. index = (where - data) / 14h (signed, truncating: the
// 66666667h multiply, SAR 3, plus the sign bit), then used unsigned.
// Full (count == capacity): capacity = capacity * 2 + 1 (not + 2 here),
// operator new (00BF55BE) capacity * 14h; copy elements [0, index) (guarded by
// the destination being non-null); the value at new + index * 14h (guarded);
// copy elements [index, count) one slot up (guarded); _free (00BF6989) the old
// data when non-null; count += 1; data = new.
// Room: slot `count` (guarded by its address being non-null) takes element
// count - 1 (read even when count is 0, as the image does); elements
// count - 1 down to index + 1 take their predecessor; then the value is read
// (after the shift) into slot index; count += 1.
void* cc13_seg48_pod_array20_insert_007f5580(void* array, void* where, const std::uint32_t* value,
                                             Cc13Seg48Host& host) {
    auto* data = at<std::uint32_t*>(array, 0);
    const std::int32_t diff = static_cast<std::int32_t>(static_cast<std::uint8_t*>(where) -
                                                        reinterpret_cast<std::uint8_t*>(data));
    const auto index = static_cast<std::uint32_t>(diff / 0x14);
    auto copy5 = [](std::uint32_t* out, const std::uint32_t* in) {
        for (int k = 0; k < 5; ++k) {
            out[k] = in[k];
        }
    };
    if (at<std::uint32_t>(array, 4) == at<std::uint32_t>(array, 8)) {
        const std::uint32_t capacity = at<std::uint32_t>(array, 8) * 2u + 1u;
        at<std::uint32_t>(array, 8) = capacity;
        auto* block = static_cast<std::uint32_t*>(host.operator_new_00bf55be(capacity * 0x14u));
        for (std::uint32_t i = 0; i < index; ++i) {
            std::uint32_t* out = block + 5 * i;
            if (out != nullptr) {
                copy5(out, at<std::uint32_t*>(array, 0) + 5 * i);
            }
        }
        std::uint32_t* slot = block + 5 * index;
        if (slot != nullptr) {
            copy5(slot, value);
        }
        for (std::uint32_t i = index; i < at<std::uint32_t>(array, 4); ++i) {
            std::uint32_t* out = block + 5 * (i + 1);
            if (out != nullptr) {
                copy5(out, at<std::uint32_t*>(array, 0) + 5 * i);
            }
        }
        if (at<void*>(array, 0) != nullptr) {
            host.crt_free_00bf6989(at<void*>(array, 0));
        }
        at<std::uint32_t>(array, 4) += 1;
        at<std::uint32_t*>(array, 0) = block;
        return block + 5 * index;
    }
    const std::uint32_t count = at<std::uint32_t>(array, 4);
    std::uint32_t* end = data + 5 * count;
    if (end != nullptr) {
        copy5(end, end - 5);
    }
    for (std::uint32_t j = count - 1; j > index; --j) {
        std::uint32_t* d = at<std::uint32_t*>(array, 0);
        copy5(d + 5 * j, d + 5 * (j - 1));
    }
    std::uint32_t* d = at<std::uint32_t*>(array, 0);
    copy5(d + 5 * index, value);
    at<std::uint32_t>(array, 4) += 1;
    return at<std::uint32_t*>(array, 0) + 5 * index;
}

// 007F6A50 BSP_PodArray20_PushBack_007f6a50, __thiscall(ECX = array, const
// element[5 dwords]), RET 4: grow when count == capacity, guarded append.
void cc13_seg48_pod_array20_push_back_007f6a50(void* array, const std::uint32_t* element, Cc13Seg48Host& host) {
    push_back20(array, element, host);
}

// 007F7110 BSP_GunPlatform_Construct, __thiscall(ECX = platform), EAX =
// platform, EH frame 00C8F744. Bytes 007F7110-007F72FE: vtable 00CEB130 and
// count +4h = 1 (the counted base), then vtable 00D08BB0; the list at +10h gets
// its head from 004870F0 (+14h; +18h = 0) and the list at +1Ch from 007F6D30
// (+20h; +24h = 0); +2Ch, +30h, +34h, +3Ch, +40h, +44h = 0; bytes +0Ch, +0Dh =
// 0; 004134F0(ECX = +4Ch, identity matrix built on the stack from 1.0f
// [00D7A24C] and 0.0f); then one 14h-byte window {byte 0 (its dword's upper
// bytes are stale stack in the image, zero here), -3.14141846f [00D08BA8] (bits
// C0490D00), 3.14141846f [00D08BAC] (bits 40490D00), 0.0f, 0.0f} pushed onto
// the {data +3Ch, count +40h, capacity +44h} array with the inlined 20-byte
// push-back.
void* cc13_seg48_gun_platform_construct_007f7110(void* platform, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(platform);
    at<std::uint32_t>(platform, 0) = 0x00CEB130u;
    at<std::uint32_t>(platform, 4) = 1;
    at<std::uint32_t>(platform, 0) = 0x00D08BB0u;
    at<void*>(platform, 0x14) = host.list_buy_head_node_004870f0(base + 0x10);
    at<std::uint32_t>(platform, 0x18) = 0;
    at<void*>(platform, 0x20) = host.list_buy_head_node_007f6d30(base + 0x1C);
    at<std::uint32_t>(platform, 0x24) = 0;
    at<std::uint32_t>(platform, 0x2C) = 0;
    at<std::uint32_t>(platform, 0x30) = 0;
    at<std::uint32_t>(platform, 0x34) = 0;
    at<std::uint32_t>(platform, 0x3C) = 0;
    at<std::uint32_t>(platform, 0x40) = 0;
    at<std::uint32_t>(platform, 0x44) = 0;
    at<std::uint8_t>(platform, 0xC) = 0;
    at<std::uint8_t>(platform, 0xD) = 0;
    float identity[16] = {};
    identity[0] = identity[5] = identity[10] = identity[15] = kCc13Seg48One;
    host.matrix_copy4x4_004134f0(base + 0x4C, identity);
    const std::uint32_t window[5] = {0u, 0xC0490D00u, 0x40490D00u, 0u, 0u};
    push_back20(base + 0x3C, window, host);
    return platform;
}

// 007F6CA0 BSP_GunPlatform_HasOnlySeedWindow, __thiscall(ECX = platform), AL.
// Bytes 007F6CA0-007F6CB4: AL = 0 when the window count +40h > 1 (unsigned);
// otherwise AL = (~byte [[+3Ch]]) & 1 (the first window's flag clear; EAX's
// upper bytes are zero there, stale on the first arm).
std::uint8_t cc13_seg48_gun_platform_has_only_seed_window_007f6ca0(void* platform) {
    if (at<std::uint32_t>(platform, 0x40) > 1) {
        return 0;
    }
    return static_cast<std::uint8_t>(~at<std::uint8_t>(at<void*>(platform, 0x3C), 0) & 1);
}

// 007F74B0 BSP_PlayerRecord_IsTalking, __thiscall(ECX = record), EAX. Bytes
// 007F74B0-007F74FF, x87: with t = game+64Ch ([00E188A8]), when record+8Ch >
// t + 10.0 (double 00CE3DC0; FCOMIP then JBE, so a NaN keeps it), record+8Ch =
// 0.0f; then return 1 when record+8Ch + 1.0 (double 00D7A210) > t (FCOMIP/JBE:
// a NaN answers 0). The game runs the x87 at 24-bit precision
// (docs/CC13_FPU_PRECISION.md), so each FADD of a float and an exact double
// constant rounds once to float: modelled as float additions.
bool cc13_seg48_player_record_is_talking_007f74b0(void* record, Cc13Seg48Host& host) {
    void* game = reinterpret_cast<void*>(static_cast<std::uintptr_t>(*host.image_global(0x00E188A8u)));
    const float* now = &at<float>(game, 0x64C);
    if (at<float>(record, 0x8C) > *now + 10.0f) {
        at<float>(record, 0x8C) = 0.0f;
    }
    return at<float>(record, 0x8C) + 1.0f > *now;
}

// 007F81C0 BSP_NoseArt_SetSelection, __thiscall(ECX = player record, const
// dword selection[3]), RET 4. Bytes 007F81C0-007F820A: for i in 0..2, when
// +E0h+4i differs from selection[i], store it and release the counted
// reference at +104h+4i (cleared).
void cc13_seg48_nose_art_set_selection_007f81c0(void* record, const std::uint32_t* selection, Cc13Seg48Host& host) {
    for (int i = 0; i < 3; ++i) {
        const std::uint32_t value = selection[i];
        if (at<std::uint32_t>(record, 0xE0 + 4 * i) != value) {
            at<std::uint32_t>(record, 0xE0 + 4 * i) = value;
            release_slot(record, 0x104 + 4 * i, host);
        }
    }
}

// 007F8210 BSP_Session_ReceiveMessageE3, __thiscall(ECX = message). Bytes
// 007F8210-007F8271: with G = [00E188A8] and the record G+748h+[msg+24h]*118h:
// mode [G+1FE4h] == 1: 007F81C0(record, msg+18h), then 0076A9F0(ECX =
// [00E188A8]+1EF0h, msg, 0); mode 2: 007F81C0 only; otherwise nothing.
void cc13_seg48_session_receive_message_e3_007f8210(void* message, Cc13Seg48Host& host) {
    auto* game = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(*host.image_global(0x00E188A8u)));
    const std::uint32_t mode = at<std::uint32_t>(game, 0x1FE4);
    auto* selection = reinterpret_cast<const std::uint32_t*>(static_cast<std::uint8_t*>(message) + 0x18);
    if (mode == 1) {
        cc13_seg48_nose_art_set_selection_007f81c0(game + 0x748 + at<std::uint32_t>(message, 0x24) * 0x118u,
                                                   selection, host);
        auto* reloaded = reinterpret_cast<std::uint8_t*>(static_cast<std::uintptr_t>(*host.image_global(0x00E188A8u)));
        host.session_forward_message_0076a9f0(reloaded + 0x1EF0, message, 0);
        return;
    }
    if (mode == 2) {
        cc13_seg48_nose_art_set_selection_007f81c0(game + 0x748 + at<std::uint32_t>(message, 0x24) * 0x118u,
                                                   selection, host);
    }
}

// ---------------------------------------------------------- save-game archives
// Shape shared by the leaf functions below: __thiscall(ECX = record, archive
// reference by value (three dwords, object at the third)), RET 0Ch. Each field
// is one slot +0Ch (save) or +10h (load) call on the archive object with
// name word 0, then the reference is destroyed (004107B0 writer / 00410630
// reader). Saves pass floats as their bit pattern (movss to a stack dword);
// a byte save pushes a dword whose low byte is the field and whose upper bytes
// are stale stack (modelled as zero-extended). Keys were read from the image.

namespace {

void* archive_object(Cc13Seg48ArchiveRef& ref) {
    return reinterpret_cast<void*>(static_cast<std::uintptr_t>(ref.words[2]));
}

void put_float(Cc13Seg48ArchiveRef& ref, const char* key, void* record, std::size_t offset, Cc13Seg48Host& host) {
    host.archive_vcall_0c_put(archive_object(ref), 0, key, 2, at<std::uint32_t>(record, offset));
}

void put_byte(Cc13Seg48ArchiveRef& ref, const char* key, void* record, std::size_t offset, Cc13Seg48Host& host) {
    host.archive_vcall_0c_put(archive_object(ref), 0, key, 3, at<std::uint8_t>(record, offset));
}

void get_field(Cc13Seg48ArchiveRef& ref, const char* key, std::int32_t type, void* record, std::size_t offset,
               Cc13Seg48Host& host) {
    host.archive_vcall_10_get(archive_object(ref), 0, key, type, static_cast<std::uint8_t*>(record) + offset);
}

// The child-reference pattern of 007E08B0 and its peers: build the child of
// `ref` under `key` (00BD4FE0 reader / 00BD5010 writer) and pass it by value to
// the child function, which destroys it.
Cc13Seg48ArchiveRef reader_child(Cc13Seg48ArchiveRef& ref, const char* key, Cc13Seg48Host& host) {
    Cc13Seg48ArchiveRef child{};
    host.archive_reader_child_00bd4fe0(&ref, &child, 0, key);
    return child;
}

Cc13Seg48ArchiveRef writer_child(Cc13Seg48ArchiveRef& ref, const char* key, Cc13Seg48Host& host) {
    Cc13Seg48ArchiveRef child{};
    host.archive_writer_child_00bd5010(&ref, &child, 0, key);
    return child;
}

}  // namespace

// 007DE880 BSP_ActuatorChannel_LoadFromArchive. Bytes 007DE880-007DE8FA:
// "targst" (00D06B44, type 3) into byte +1h, "curst" (00D06B3C, type 2) into +4h.
void cc13_seg48_actuator_channel_load_007de880(void* channel, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    get_field(ref, "targst", 3, channel, 1, host);
    get_field(ref, "curst", 2, channel, 4, host);
    host.archive_reader_ref_destroy_00410630(&ref);
}

// 007DE900 BSP_ActuatorChannel_SaveToArchive. Bytes 007DE900-007DE999:
// "targst" byte +1h, "curst" float +4h.
void cc13_seg48_actuator_channel_save_007de900(void* channel, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    put_byte(ref, "targst", channel, 1, host);
    put_float(ref, "curst", channel, 4, host);
    host.archive_writer_ref_destroy_004107b0(&ref);
}

// 007E0740 BSP_PlaneLerpValue_LoadFromArchive. Bytes 007E0740-007E07DB:
// "currentValue" (00D06BCC) +0h, "rate" (00D06BC4) +4h, "targetValue"
// (00D06BB8) +8h, all type 2.
void cc13_seg48_plane_lerp_value_load_007e0740(void* value, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    get_field(ref, "currentValue", 2, value, 0, host);
    get_field(ref, "rate", 2, value, 4, host);
    get_field(ref, "targetValue", 2, value, 8, host);
    host.archive_reader_ref_destroy_00410630(&ref);
}

// 007E07E0 BSP_PlaneLerpValue_SaveToArchive. Bytes 007E07E0-007E08A0: the
// same three floats in the same order.
void cc13_seg48_plane_lerp_value_save_007e07e0(void* value, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    put_float(ref, "currentValue", value, 0, host);
    put_float(ref, "rate", value, 4, host);
    put_float(ref, "targetValue", value, 8, host);
    host.archive_writer_ref_destroy_004107b0(&ref);
}

// 007E08B0 BSP_PlaneLerpPair_LoadFromArchive. Bytes 007E08B0-007E0953: child
// "volume" (00D06BDC) into the lerp value at +20h, child "pitch" (00D059AC)
// into +14h (007E0740 each).
void cc13_seg48_plane_lerp_pair_load_007e08b0(void* pair, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(pair);
    cc13_seg48_plane_lerp_value_load_007e0740(base + 0x20, reader_child(ref, "volume", host), host);
    cc13_seg48_plane_lerp_value_load_007e0740(base + 0x14, reader_child(ref, "pitch", host), host);
    host.archive_reader_ref_destroy_00410630(&ref);
}

// 007E0960 BSP_PlaneLerpPair_SaveToArchive. Bytes 007E0960-007E0A03: the
// writer children "volume" (+20h) and "pitch" (+14h) through 007E07E0.
void cc13_seg48_plane_lerp_pair_save_007e0960(void* pair, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(pair);
    cc13_seg48_plane_lerp_value_save_007e07e0(base + 0x20, writer_child(ref, "volume", host), host);
    cc13_seg48_plane_lerp_value_save_007e07e0(base + 0x14, writer_child(ref, "pitch", host), host);
    host.archive_writer_ref_destroy_004107b0(&ref);
}

// 007E0A10 BSP_PlaneTimedFlag_LoadFromArchive. Bytes 007E0A10-007E0AA5:
// "value" (00D06BF4, type 3) byte +4h, "timeOut" (00D06BEC) +0h, "timer"
// (00D06BE4) +8h.
void cc13_seg48_plane_timed_flag_load_007e0a10(void* flag, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    get_field(ref, "value", 3, flag, 4, host);
    get_field(ref, "timeOut", 2, flag, 0, host);
    get_field(ref, "timer", 2, flag, 8, host);
    host.archive_reader_ref_destroy_00410630(&ref);
}

// 007E0AB0 BSP_PlaneTimedFlag_SaveToArchive. Bytes 007E0AB0-007E0B72: the same
// keys and order.
void cc13_seg48_plane_timed_flag_save_007e0ab0(void* flag, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    put_byte(ref, "value", flag, 4, host);
    put_float(ref, "timeOut", flag, 0, host);
    put_float(ref, "timer", flag, 8, host);
    host.archive_writer_ref_destroy_004107b0(&ref);
}

// 007E0B80 BSP_PlaneStateBlock_LoadFromArchive (the plane effects component's
// archive). Bytes 007E0B80-007E0D19: "running" (00D06C50, type 3) byte +14h;
// child "flying" (00CF9364) -> 007E0A10 on +24h; "RPM" (00D06C4C) +30h;
// "startStopTimer" (00D06C3C) +40h; children "runningSound" (00D06C2C) +48h,
// "deadmeatSound" (00D06C1C) +74h, "windSound" (00D06C10) +A0h, "fallSound"
// (00D06C04) +CCh through 007E08B0; then on the reader slot +4h(0, "engines"
// 00D06BFC) and slot +8h() with nothing between them.
void cc13_seg48_plane_state_block_load_007e0b80(void* block, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(block);
    get_field(ref, "running", 3, block, 0x14, host);
    cc13_seg48_plane_timed_flag_load_007e0a10(base + 0x24, reader_child(ref, "flying", host), host);
    get_field(ref, "RPM", 2, block, 0x30, host);
    get_field(ref, "startStopTimer", 2, block, 0x40, host);
    cc13_seg48_plane_lerp_pair_load_007e08b0(base + 0x48, reader_child(ref, "runningSound", host), host);
    cc13_seg48_plane_lerp_pair_load_007e08b0(base + 0x74, reader_child(ref, "deadmeatSound", host), host);
    cc13_seg48_plane_lerp_pair_load_007e08b0(base + 0xA0, reader_child(ref, "windSound", host), host);
    cc13_seg48_plane_lerp_pair_load_007e08b0(base + 0xCC, reader_child(ref, "fallSound", host), host);
    host.archive_vcall_04(archive_object(ref), 0, "engines");
    host.archive_vcall_08(archive_object(ref));
    host.archive_reader_ref_destroy_00410630(&ref);
}

// 007E0D20 BSP_PlaneStateBlock_SaveToArchive. Bytes 007E0D20-007E0EFD: the
// writer mirror of 007E0B80 (007E0AB0 / 007E0960 children, slot +0Ch puts,
// then slot +4h(0, "engines") and slot +8h()).
void cc13_seg48_plane_state_block_save_007e0d20(void* block, Cc13Seg48ArchiveRef ref, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(block);
    put_byte(ref, "running", block, 0x14, host);
    cc13_seg48_plane_timed_flag_save_007e0ab0(base + 0x24, writer_child(ref, "flying", host), host);
    put_float(ref, "RPM", block, 0x30, host);
    put_float(ref, "startStopTimer", block, 0x40, host);
    cc13_seg48_plane_lerp_pair_save_007e0960(base + 0x48, writer_child(ref, "runningSound", host), host);
    cc13_seg48_plane_lerp_pair_save_007e0960(base + 0x74, writer_child(ref, "deadmeatSound", host), host);
    cc13_seg48_plane_lerp_pair_save_007e0960(base + 0xA0, writer_child(ref, "windSound", host), host);
    cc13_seg48_plane_lerp_pair_save_007e0960(base + 0xCC, writer_child(ref, "fallSound", host), host);
    host.archive_vcall_04(archive_object(ref), 0, "engines");
    host.archive_vcall_08(archive_object(ref));
    host.archive_writer_ref_destroy_004107b0(&ref);
}

// 007D9B70 BSP_PlaneFlightController_SaveToArchive, __thiscall(ECX =
// controller, writer*), RET 4 (the writer pointer itself, not a reference).
// Bytes 007D9B70-007D9BFE: "V" (00D06864, type 5, &+18h), "Om" (00D06860,
// type 5, &+24h), "Turbo" (00D06858, type 3, byte +4h).
void cc13_seg48_plane_flight_controller_save_007d9b70(void* controller, void* writer, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(controller);
    host.archive_vcall_0c_put(writer, 0, "V", 5, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base + 0x18)));
    host.archive_vcall_0c_put(writer, 0, "Om", 5, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(base + 0x24)));
    host.archive_vcall_0c_put(writer, 0, "Turbo", 3, at<std::uint8_t>(controller, 4));
}

// 007DB430 BSP_PlaneFlightController_LoadFromArchive, __thiscall(ECX =
// controller, reader*), RET 4. Bytes 007DB430-007DB4CC: the same three keys
// through slot +10h, then 007D9C10 and 007D9C80 (ECX = controller).
void cc13_seg48_plane_flight_controller_load_007db430(void* controller, void* reader, Cc13Seg48Host& host) {
    auto* base = static_cast<std::uint8_t*>(controller);
    host.archive_vcall_10_get(reader, 0, "V", 5, base + 0x18);
    host.archive_vcall_10_get(reader, 0, "Om", 5, base + 0x24);
    host.archive_vcall_10_get(reader, 0, "Turbo", 3, base + 4);
    host.plane_flight_controller_refresh_body_frame_007d9c10(controller);
    host.plane_flight_controller_body_to_world_velocity_007d9c80(controller);
}

// 007DE760 BSP_PlaneSoundSet_ForEachSlot18, __thiscall(ECX = set, dword a,
// float b), RET 8. Bytes 007DE760-007DE7D5: slot +18h(a, b) on every element of
// {data +0Ch, count +10h} that is non-null (the 00E19AB0 mask test), then on
// every element of {data +18h, count +1Ch} without a null test.
void cc13_seg48_plane_sound_set_for_each_slot18_007de760(void* set, std::uint32_t a, float b, Cc13Seg48Host& host) {
    void** first = at<void**>(set, 0xC);
    void** last = first + at<std::int32_t>(set, 0x10);
    for (void** it = first; it != last; ++it) {
        if (*it != nullptr) {
            host.vcall_18_sound(*it, a, b);
        }
    }
    first = at<void**>(set, 0x18);
    last = first + at<std::int32_t>(set, 0x1C);
    for (void** it = first; it != last; ++it) {
        host.vcall_18_sound(*it, a, b);
    }
}

// 007EB380..007EBF03: complete ordinary control flow, native ECX component,
// stack float dt / RET 4. All 833 listing instructions were reviewed. The
// decompiler's unaff_retaddr is dt; its unaff_EBP and reused locals are not
// source inputs. In particular, load uses the clamped acceleration mapping,
// +70h uses the wanted/running multiplier, and the four holders are +48h,
// +74h, +A0h and +CCh. No DC-form ambiguous x87 multiply exists in this span.
// CRT sqrt is a qualified stock substitution, not reconstructed game credit.
// Float stores are retained; C++ double is not native 80-bit x87/ABI proof.
void plane_fx_component_step_007eb380(std::uint8_t* component, float dt,
                                    const PlaneFxStepGlobals& g, PlaneFxStepHost& host) {
    auto plane = [&]() { return at<std::uint8_t*>(component, 4); };
    at<float>(component, 0x20) = at<float>(plane(), 0x9F0);
    const float max_step = at<float>(component, 0x1C) * dt;
    host.call_0042ac60(&at<float>(component, 0x18), at<float>(component, 0x20), max_step); // 007EB3BC
    at<float>(component, 0x38) = at<float>(component, 0x18);
    at<float>(component, 0x3C) = static_cast<float>(at<float>(component, 0x3C)
        + ((static_cast<double>(at<float>(component, 0x18))
        - at<float>(component, 0x34)) * g.c_00ce3d40) * dt);
    const float acceleration = at<float>(component, 0x3C);
    if (g.c_00d086a0 > acceleration) at<float>(component, 0x3C) = g.c_00d08698;
    else if (acceleration > g.c_00ce3d48) at<float>(component, 0x3C) = g.c_00d06bb4;
    const float previous = at<float>(component, 0x34);
    const float limit = static_cast<float>((static_cast<double>(at<float>(component, 0x38)) - previous) * 3.0);
    if (at<float>(component, 0x38) > previous
        ? at<float>(component, 0x3C) > limit : limit > at<float>(component, 0x3C))
        at<float>(component, 0x3C) = limit;
    const float applied_acceleration = at<float>(component, 0x3C);
    at<float>(component, 0x34) = static_cast<float>(static_cast<double>(applied_acceleration) * dt + previous);
    float load = static_cast<float>(applied_acceleration * g.c_00d08690 + 0.5);
    if (0.0f > load) load = 0.0f;
    else if (load > 1.0f) load = 1.0f;

    std::uint8_t* tuning = host.call_0042e740();                         // 007EB4C0
    const std::uint8_t state = host.vcall_controller_38(plane() + 0x72C); // 007EB4D9
    if (state == at<std::uint8_t>(component, 0x28)) {
        at<float>(component, 0x2C) = 0.0f;
    } else {
        at<float>(component, 0x2C) = at<float>(component, 0x2C) + dt;
        if (at<float>(component, 0x2C) > at<float>(component, 0x24))
            at<std::uint8_t>(component, 0x28) = state;
    }
    bool wanted = false;
    if (at<std::uint8_t>(component, 0x28) != 0) {
        std::uint8_t* p = plane();
        wanted = at<std::uint8_t>(p, 0xBC8) != 0
            || !host.vcall_plane_5c(p, 0x17) || at<std::uint8_t>(p, 0xC24) != 0; // 007EB53D
    }
    float effect_value = 0.0f;
    if (at<std::uint8_t>(component, 0x14) != 0) {
        const float fade = at<float>(component, 0x30) + dt;
        at<float>(component, 0x30) = fade < 1.0f ? fade : 1.0f;
        const float current = at<float>(component, 0x34);
        effect_value = static_cast<float>(host.call_00419010(0.0f, at<float>(tuning, 0x174),
            1.0f, at<float>(tuning, 0x178), current));                  // 007EB5C8
        if (wanted || current != 0.0f) {
            at<float>(component, 0x40) = 0.0f;
        } else {
            at<float>(component, 0x40) = at<float>(component, 0x40) + dt;
            if (at<float>(component, 0x40) > 3.0f) {
                at<std::uint8_t>(component, 0x14) = 0;
                at<float>(component, 0x40) = 0.0f;
                host.call_007eb0e0(component);                         // 007EB620
            }
        }
    } else {
        const float fade = at<float>(component, 0x30) - dt;
        at<float>(component, 0x30) = fade > 0.0f ? fade : 0.0f;
        at<float>(component, 0x40) = at<float>(component, 0x40) + dt;
        if (wanted || (at<float>(component, 0x40) > 1.0f && at<float>(component, 0x34) > 0.0f)) {
            if (dt > 0.0f) {
                host.call_007eae30(component + 0x48, 1);               // 007EB6B4
                host.call_007eb160(component);                         // 007EB6BB
                at<std::uint8_t>(component, 0x14) = 1;
                effect_value = 0.0f;
                at<float>(component, 0x40) = 0.0f;
            }
        } else if (at<float>(component, 0x30) == 0.0f) {
            host.call_007e1b30(component + 0x48);                       // 007EB693
            host.call_007eb0e0(component);                             // 007EB69A
        }
    }
    for (std::int32_t i = 0; i < at<std::int32_t>(component, 0xC); ++i)
        host.vcall_effect_10(at<void**>(component, 8)[i], effect_value, dt); // 007EB700

    float multiplier = 1.0f;
    if (!wanted) {
        double candidate;
        if (at<std::uint8_t>(component, 0x14) != 0)
            candidate = static_cast<double>(at<float>(tuning, 0x6CC))
                - host.call_00419010(0.0f, 1.0f, at<float>(tuning, 0x6AC), 0.0f,
                                     at<float>(component, 0x34));      // 007EB742
        else candidate = static_cast<double>(at<float>(tuning, 0x6CC)) - g.c_00d7a210;
        const float stored = static_cast<float>(candidate);
        multiplier = stored < 1.0f ? stored : 1.0f;
    }

    std::uint8_t* listener = at<std::uint8_t*>(g.game_00e188a8, 0x19FC);
    float vector_out[3];
    const float* vector = host.vcall_plane_34(plane(), vector_out);     // 007EB7A0
    float displacement[3];
    for (std::size_t i = 0; i != 3; ++i) {
        const float relative = vector[i] - at<float>(listener, 0x1B8 + 4 * i);
        displacement[i] = relative * dt;
    }
    const auto refresh = [&](std::uint8_t* p) {
        if (at<std::uint8_t>(p, 0xC8) == 0) {
            if (at<void*>(p, 0x3C) != nullptr) host.call_00414db0(at<void*>(p, 0x3C));
            float scratch[16];
            const float* matrix;
            if (at<std::uint8_t*>(p, 0x3C) == nullptr)
                matrix = reinterpret_cast<const float*>(p + 0x74);
            else matrix = host.call_00413920(reinterpret_cast<const float*>(p + 0x74), scratch,
                reinterpret_cast<const float*>(at<std::uint8_t*>(p, 0x3C) + 0xCC));
            host.call_004134f0(reinterpret_cast<float*>(p + 0xCC), matrix);
            at<std::uint8_t>(p, 0xC8) = 1;
            at<std::uint8_t>(p, 0x10C) = 0;
        }
    };
    std::uint8_t* p = plane();
    refresh(p);                                                       // 007EB7F8/007EB812/007EB823
    float predicted[3];
    for (std::size_t i = 0; i != 3; ++i) predicted[i] = at<float>(p, 0xFC + 4 * i) - displacement[i];
    p = plane();
    refresh(p);                                                       // 007EB873/007EB88D/007EB89E
    float position[3];
    for (std::size_t i = 0; i != 3; ++i) position[i] = at<float>(p, 0xFC + 4 * i);
    listener = at<std::uint8_t*>(g.game_00e188a8, 0x19FC);
    if ((at<std::uint8_t>(listener, 0x5C) & 2) == 0) host.call_00b6db70(listener); // 007EB8EE
    float listener_position[3], current_delta[3], predicted_delta[3];
    for (std::size_t i = 0; i != 3; ++i) {
        listener_position[i] = at<float>(listener, 0x120 + 4 * i);
        predicted_delta[i] = predicted[i] - listener_position[i];
        current_delta[i] = position[i] - listener_position[i];
    }
    const auto length = [&](const float* v) {
        const float square = static_cast<float>(static_cast<double>(v[1]) * v[1]
            + static_cast<double>(v[0]) * v[0] + static_cast<double>(v[2]) * v[2]);
        return square > g.c_00ce3820 ? static_cast<float>(std::sqrt(static_cast<double>(square))) : 0.0f;
    };
    const float predicted_distance = length(predicted_delta);          // 007EB965
    const float current_distance = length(current_delta);              // 007EB9EF
    std::uint8_t* sound_globals = g.sound_00f8bbd8;
    float rate = static_cast<float>((static_cast<double>(predicted_distance) - current_distance)
        / dt / at<float>(sound_globals, 0x14));
    if (at<float>(sound_globals, 0x18) > rate) rate = at<float>(sound_globals, 0x18);
    else if (rate > at<float>(sound_globals, 0x1C)) rate = at<float>(sound_globals, 0x1C);
    const float pitch = static_cast<float>((rate + g.c_00d7a210) * at<float>(tuning, 0x6B4));
    at<float>(component, 0x70) = multiplier;
    at<float>(component, 0x64) = pitch;
    if (at<void*>(component, 0x58) != nullptr)
        host.vcall_sound_1c(at<void*>(component, 0x58), g.rpm_key_00d08688,
                            at<float>(component, 0x34));               // 007EBAB5
    if (at<void*>(component, 0x58) != nullptr)
        host.vcall_sound_1c(at<void*>(component, 0x58), g.load_key_00d08680, load); // 007EBAD9

    float toward_listener[3];
    for (std::size_t i = 0; i != 3; ++i) toward_listener[i] = listener_position[i] - position[i];
    float velocity[3];
    (void)host.vcall_plane_34(plane(), velocity);                       // 007EBB0C
    const float denominator = static_cast<float>(static_cast<double>(velocity[1]) * velocity[1]
        + static_cast<double>(velocity[0]) * velocity[0] + static_cast<double>(velocity[2]) * velocity[2]);
    float projection;
    if (denominator > 0.0f) {
        const float numerator = static_cast<float>(static_cast<double>(toward_listener[1]) * velocity[1]
            + static_cast<double>(toward_listener[0]) * velocity[0]
            + static_cast<double>(toward_listener[2]) * velocity[2]);
        projection = numerator / denominator;
    } else projection = static_cast<float>(g.c_00ce4bd8);
    sound_globals = g.sound_00f8bbd8;                                  // 007EBB7A reload
    const float centered = static_cast<float>(projection - at<float>(sound_globals, 0x34) * 0.5);
    const float absolute = centered > 0.0f ? centered : -0.0f - centered;
    float pass = 0.0f;
    if (g.c_00d04368 > absolute && at<float>(sound_globals, 0x30) > length(current_delta))
        pass = g.c_00ce54a0;
    if (at<void*>(component, 0x58) != nullptr)
        host.vcall_sound_1c(at<void*>(component, 0x58), g.pass_key_00d08678, pass); // 007EBC6E/007EBC91

    if (dt > 0.0f && plane() != nullptr) {
        p = plane();
        const std::int32_t flight_state = at<std::int32_t>(p, 0x900);
        if (flight_state == 7 || flight_state == 6 || flight_state == 4 || flight_state == 5) {
            std::uint8_t* descriptor = at<std::uint8_t*>(p, 0x538);
            const float ratio = static_cast<float>(host.vcall_plane_38(p) / at<float>(descriptor, 0x188)); // 007EBCE1
            if (at<std::uint8_t>(plane(), 0x5D) != 0) {
                at<float>(component, 0x44) = at<float>(component, 0x44) + dt;
                if (!(g.c_00ce7630 > at<float>(component, 0x44))) {
                    at<float>(component, 0x44) = at<float>(component, 0x44) + dt;
                    const float level = static_cast<float>((at<float>(component, 0x44) - g.c_00ce7630) * g.c_00ce3d40);
                    at<float>(component, 0x9C) = level < 1.0f ? level : 1.0f;
                } else if (at<float>(component, 0x44) >= g.c_00ce3958 && ratio > g.c_00ce3d30) {
                    host.call_007eae30(component + 0x74, 1);            // 007EBD44
                    at<float>(component, 0x94) = 0.0f;
                    at<float>(component, 0x9C) = 0.0f;
                    at<float>(component, 0x44) = g.c_00ce38c8;
                }
            }
            float volume = static_cast<float>(host.call_00419010(at<float>(tuning, 0x24), 0.0f,
                at<float>(tuning, 0x28), 1.0f, ratio));                 // 007EBDD0
            float target_pitch = static_cast<float>(host.call_00419010(at<float>(tuning, 0x2C),
                at<float>(tuning, 0x34), at<float>(tuning, 0x30), at<float>(tuning, 0x38), ratio)); // 007EBDFF
            host.call_007eae30(component + 0xA0, 0);                   // 007EBE10
            at<float>(component, 0xC8) = volume;
            at<float>(component, 0xBC) = target_pitch;
            volume = static_cast<float>(host.call_00419010(at<float>(tuning, 0x3C), 0.0f,
                at<float>(tuning, 0x40), 1.0f, ratio));                 // 007EBE4D
            target_pitch = static_cast<float>(host.call_00419010(at<float>(tuning, 0x44),
                at<float>(tuning, 0x4C), at<float>(tuning, 0x48), at<float>(tuning, 0x50), ratio)); // 007EBE7C
            host.call_007eae30(component + 0xCC, 0);                   // 007EBE8D
            at<float>(component, 0xF4) = volume;
            at<float>(component, 0xE8) = target_pitch;
        }
    }
    host.call_007de490(component + 0x48, dt);                           // 007EBEB3
    host.call_007de490(component + 0x74, dt);                           // 007EBEC6
    host.call_007de490(component + 0xA0, dt);                           // 007EBEDC
    host.call_007de490(component + 0xCC, dt);                           // 007EBEF2
}

}  // namespace bsp
