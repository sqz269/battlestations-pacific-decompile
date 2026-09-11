// Reconstruction of the sub-object constructors BSP_UnitVehicleBase_Construct
// (0081ED40) runs. Evidence: docs/UNIT_INSTANCE_SUBOBJECTS.md,
// reports/unit_instance_subobjects.json.
//
// These are semantic reconstructions, not drop-in binary replacements: the
// native objects are raw byte blocks inside one 0x1188 allocation and these are
// ordinary C++ structs. The field writes, their order and their values are the
// native ones.

#include "bsp/unit_instance_subobjects.hpp"

namespace bsp {

// 00809270  MOV EAX,ECX / XOR ECX,ECX / MOV [EAX],0xd09004 / MOV [EAX+4],ECX /
//           MOV [EAX+8],ECX / RET
void construct_unit_owned_ref_slot_00809270(UnitOwnedRefSlot& self) noexcept {
    self.vptr = kUnitSubObjVtableOwnedRefSlot; // 00809274
    self.field_4 = 0;                          // 0080927A
    self.owned = 0;                            // 0080927D
}

// 00809650  MOV [ESI],0xd09004 / MOV ECX,[ESI+8] / TEST ECX,ECX / JZ /
//           MOV EAX,[ECX] / MOV EDX,[EAX] / PUSH 1 / CALL EDX /
//           MOV [ESI+8],0
void reset_unit_owned_ref_slot_00809650(
    UnitOwnedRefSlot& self,
    void (*release)(std::uint32_t owned, int deleting_flag)) noexcept {
    self.vptr = kUnitSubObjVtableOwnedRefSlot; // 00809653
    if (self.owned != 0) {                     // 0080965C..0080965E
        release(self.owned, 1);                // 00809664 PUSH 1, 00809666 CALL EDX
        self.owned = 0;                        // 00809668
    }
}

// 0093BCC0, __thiscall(self, unit), RET 4, returns self in EAX. Field order
// below is the native write order.
UnitSubObjectA20& construct_unit_sub_object_a20_0093bcc0(UnitSubObjectA20& self,
                                                         std::uint32_t owner_unit,
                                                         UnitSubObjectHost& host) {
    self.dword_array.first = 0; // 0093BCE4, self+8h
    self.dword_array.last = 0;  // 0093BCE7, self+0Ch
    self.dword_array.end = 0;   // 0093BCEA, self+10h
    self.field_18 = 0;          // 0093BCF1
    self.field_1c = 0;          // 0093BCF4
    self.field_20 = 0;          // 0093BCF7
    self.field_28 = 1.0f;       // 0093BD06, 00D7A24C
    self.owner_unit = owner_unit; // 0093BD13, the stack argument
    self.field_24 = 0;          // 0093BD15
    self.field_34 = 0.0f;       // 0093BD18
    self.field_38 = 0.0f;       // 0093BD1D

    // 0093BD22 CALL 00424C40 / FLD [EAX+3B0h] / FSTP [ESI+2Ch]
    self.settings_3b0 = host.settings_float_3b0();
    // 0093BD30 CALL 00424C40 / FLD [EAX+3ACh] / FSTP [ESI+30h]
    self.settings_3ac = host.settings_float_3ac();

    self.flag_45 = 1;     // 0093BD48
    self.flag_46 = 1;     // 0093BD4C
    self.field_3c = 0.0f; // 0093BD50
    self.field_40 = 0.0f; // 0093BD55

    // 0093BD60 CALL 00822460 with ECX = self+4h, (&zero, 14h); element size 4.
    host.array_resize(self.dword_array, kUnitSubObjBlockA20ArrayCount, 0);

    self.flag_44 = 0; // 0093BD69
    return self;      // 0093BD6D MOV EAX,ESI
}

// 0080FAE0, decoded from the raw bytes 0080FAE0..0080FB03 (no Ghidra function).
void construct_unit_record_inner_element_0080fae0(
    UnitRecordInnerElement& self) noexcept {
    self.field_4 = 0.0f;    // 0080FAEF
    self.field_8 = 1000.0f; // 0080FAF4, 00CE3804
    self.field_0 = 0.0f;    // 0080FAF9
    self.flag_c = 0;        // 0080FAFD
    self.field_18 = 0;      // 0080FB00
}

// One iteration of 0081EF63..0081EFB1. The inner array is built first by the
// CRT iterator at 0081EF7D, then the record's own floats are written.
void construct_unit_sub_object_record_0081ef63(UnitSubObjectRecord& self) noexcept {
    for (int i = 0; i < kUnitSubObjInnerElementCount; ++i) { // 0081EF7D, count 2
        construct_unit_record_inner_element_0080fae0(self.inner[i]);
    }
    self.field_0 = 0.0f;   // 0081EF8D
    self.field_4 = -1.0f;  // 0081EF91, 00D7A260
    self.owner_unit = 0;   // 0081EF96
    self.field_48 = 0.0f;  // 0081EF99
    self.field_44 = 0.0f;  // 0081EF9E
    self.field_40 = 0.0f;  // 0081EFA3
    self.flag_4c = 0;      // 0081EFA8
}

// 00815600, __thiscall(self), returns self in EAX.
UnitPoseHistoryRing& construct_unit_pose_history_ring_00815600(
    UnitPoseHistoryRing& self,
    const float seed_position[3],
    UnitSubObjectHost& host) {
    self.vptr = kUnitSubObjVtablePoseHistoryRing; // 00815606

    // 0081560C..00815623: ECX counts 27h down to -1, so 40 iterations, each
    // writing the two floats at slot+0Ch and slot+10h.
    for (int i = 0; i < kUnitPoseHistorySlotCount; ++i) {
        self.slots[i].field_c = 0.0f;  // 00815614, [EAX-4]
        self.slots[i].field_10 = 0.0f; // 00815619, [EAX]
    }

    // 00815625 CALL 00BD1860, result stored at 00815637.
    self.critical_section = host.create_critical_section();

    // 0081562A FLDZ / 00815630 PUSH 0xf87574 / 0081563A CALL 00810020.
    host.fill_pose_history_ring(self, seed_position, 0.0f);

    // 0081563F..0081566F: the same global vec3 copied into +3D0h..+3D8h.
    self.origin[0] = seed_position[0];
    self.origin[1] = seed_position[1];
    self.origin[2] = seed_position[2];

    return self; // 0081566F MOV EAX,ESI
}

// 0081EB90, identical field set to the inline block 0081F102..0081F154. The two
// container header words at +8h and +18h are deliberately not written: the
// native constructor leaves them to the container's own base.
void construct_unit_sub_object_block_10a0_0081eb90(
    UnitSubObjectBlock10A0& self) noexcept {
    self.vptr = kUnitSubObjVtableBlock10A0; // 0081EBB2 / 0081F102
    self.field_4 = 1.0f;                    // 0081EBB8 / 0081F10C, 00D7A24C
    self.list_a.first = 0;                  // 0081EBBD / 0081F114
    self.list_a.last = 0;                   // 0081EBC0 / 0081F11A
    self.list_a.end = 0;                    // 0081EBC3 / 0081F120
    self.list_b.first = 0;                  // 0081EBCE / 0081F12E
    self.list_b.last = 0;                   // 0081EBD1 / 0081F134
    self.list_b.end = 0;                    // 0081EBD4 / 0081F13A
    self.field_28 = 0;                      // 0081EBD7 / 0081F140
    self.field_2c = 0;                      // 0081EBDA / 0081F146
    self.field_30 = 60.0f;                  // 0081EBE1 / 0081F14C, 00CEB4B0
}

// 0074E7B0, __thiscall(self), plain RET.
void construct_unit_leak_manager_0074e7b0(
    UnitLeakManagerConstruction& self) noexcept {
    self.tuning_38 = 200.0f;                    // 0074E7BC, 00CE386C
    self.tuning_3c = 60.0f;                     // 0074E7C9, 00CEB4B0
    self.vptr = kUnitSubObjVtableLeakManager;   // 0074E7D1
    self.field_1c = 0;                          // 0074E7D7
    self.field_18 = 0;                          // 0074E7DA
    self.field_20 = 0;                          // 0074E7DD
    self.flag_24 = 0;                           // 0074E7E0
    self.field_28 = 0.0f;                       // 0074E7E3
    self.field_2c = 0.0f;                       // 0074E7E8
}

// The sub-object region of 0081ED40, 0081ED6F..0081F1F8, in native order. The
// order ring at +838h (00812D40) and the three dword arrays are host contracts
// and are not built here; the arrays are announced through the host so a caller
// sees every native call site.
void construct_unit_instance_sub_objects_0081ed40(UnitInstanceSubObjects& self,
                                                  std::uint32_t unit_pointer,
                                                  const float ring_seed_position[3],
                                                  UnitSubObjectHost& host) {
    // 0081ED7B, ECX = unit+72Ch.
    construct_unit_owned_ref_slot_00809270(self.owned_ref_slot);

    // 0081EF24: four dwords at +A00h.
    host.construct_dword_array(kUnitSubObjOffDwordArrayA,
                               kUnitSubObjDwordArrayACount);

    // 0081EF47, ECX = unit+A20h, one stack argument.
    construct_unit_sub_object_a20_0093bcc0(self.block_a20, unit_pointer, host);

    // 0081EF63..0081EFB1: two records at +A98h and +AECh.
    for (int i = 0; i < kUnitSubObjRecordCount; ++i) {
        construct_unit_sub_object_record_0081ef63(self.records[i]);
    }

    // 0081EFCE and 0081EFED: four dwords at +B44h, five at +B54h.
    host.construct_dword_array(kUnitSubObjOffDwordArrayB,
                               kUnitSubObjDwordArrayBCount);
    host.construct_dword_array(kUnitSubObjOffDwordArrayC,
                               kUnitSubObjDwordArrayCCount);

    // 0081F03D, ECX = unit+BD0h.
    construct_unit_pose_history_ring_00815600(self.pose_history_ring,
                                              ring_seed_position, host);

    // 0081F102..0081F154: the +10A0h block, inlined in the native constructor.
    construct_unit_sub_object_block_10a0_0081eb90(self.block_10a0);

    // 0081F15F, ECX = unit+10D4h.
    construct_unit_leak_manager_0074e7b0(self.leak_manager);

    // 0081F1E6 and 0081F1EC: the unit back-pointer into each record's +50h.
    self.records[0].owner_unit = unit_pointer;
    self.records[1].owner_unit = unit_pointer;
}

} // namespace bsp
