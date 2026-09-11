#pragma once
// The sub-objects BSP_UnitVehicleBase_Construct (0081ED40) builds inside the
// 0x1188-byte unit instance. Evidence: docs/UNIT_INSTANCE_SUBOBJECTS.md,
// reports/unit_instance_subobjects.json. Offsets and sizes come from the
// constructors' own field writes; every descriptive name is a hypothesis, not a
// recovered symbol.
//
// The unit-level offsets of the objects reproduced here are additions to
// include/bsp/unit_instance_layout.hpp, which owns the unit's own field names
// (kUnitLayoutOff*). Nothing in this header re-declares a constant or a type
// from that header.
//
// STL containers, the CRT vector iterators, the critical section and the
// settings singleton are host contracts and are not ported.

#include <cstddef>
#include <cstdint>

namespace bsp {

// ---------------------------------------------------------------------------
// Where each sub-object sits inside the unit instance.
// ---------------------------------------------------------------------------

// 00809270, receiver from 0081ED6F LEA EDI,[ESI+72Ch].
inline constexpr std::size_t kUnitSubObjOffOwnedRefSlot = 0x72c;
inline constexpr std::size_t kUnitSubObjSizeOwnedRefSlot = 0x00c;

// 00BF7CD1 at 0081EF24, 0081EFCE and 0081EFED: three arrays of zeroed dwords.
inline constexpr std::size_t kUnitSubObjOffDwordArrayA = 0x0a00;
inline constexpr int kUnitSubObjDwordArrayACount = 4;
inline constexpr std::size_t kUnitSubObjOffDwordArrayB = 0x0b44;
inline constexpr int kUnitSubObjDwordArrayBCount = 4;
inline constexpr std::size_t kUnitSubObjOffDwordArrayC = 0x0b54;
inline constexpr int kUnitSubObjDwordArrayCCount = 5;
inline constexpr std::size_t kUnitSubObjDwordArrayElementSize = 4;

// 0093BCC0, receiver from 0081EF3C LEA ECX,[ESI+A20h]. The 54h bound is the
// next initialised unit field, the byte at +A74h written at 0081EF4C.
inline constexpr std::size_t kUnitSubObjOffBlockA20 = 0x0a20;
inline constexpr std::size_t kUnitSubObjSizeBlockA20 = 0x054;

// The two records of the loop at 0081EF63..0081EFB1.
inline constexpr std::size_t kUnitSubObjOffRecordArray = 0x0a98;
inline constexpr std::size_t kUnitSubObjRecordStride = 0x054;
inline constexpr int kUnitSubObjRecordCount = 2;

// 00815600, receiver from 0081F02A LEA ECX,[ESI+BD0h].
inline constexpr std::size_t kUnitSubObjOffPoseHistoryRing = 0x0bd0;
inline constexpr std::size_t kUnitSubObjSizePoseHistoryRing = 0x3dc;

// Constructed inline at 0081F102..0081F154; the standalone copy is 0081EB90.
inline constexpr std::size_t kUnitSubObjOffBlock10A0 = 0x10a0;
inline constexpr std::size_t kUnitSubObjSizeBlock10A0 = 0x034;

// 0074E7B0, receiver from 0081F154 LEA ECX,[ESI+10D4h]. The use of this object
// is a contract: docs/UNIT_CONTROLLER_UPDATE.md, the flooding model.
inline constexpr std::size_t kUnitSubObjOffLeakManager = 0x10d4;

// ---------------------------------------------------------------------------
// Vtables. Each of these tables has exactly one slot; the bound is the string or
// the foreign method that follows it. See the doc for the bound of each.
// ---------------------------------------------------------------------------
inline constexpr std::uint32_t kUnitSubObjVtableOwnedRefSlot = 0x00d09004;
inline constexpr std::uint32_t kUnitSubObjVtablePoseHistoryRing = 0x00d09480;
inline constexpr std::uint32_t kUnitSubObjVtableBlock10A0 = 0x00d09624;
inline constexpr std::uint32_t kUnitSubObjVtableLeakManager = 0x00d00514;

// 006FE4A9 stores this over the +72Ch vptr after 0081ED40 returns, so 00D09004
// is the base's own table and this is the unit's override of it.
inline constexpr std::uint32_t kUnitSubObjVtableOwnedRefSlotOverride = 0x00cfc384;

// ---------------------------------------------------------------------------
// An STL-style array header as the two block constructors see it: they zero the
// three pointers and leave the leading word to the container's own base. The
// element storage itself is a host contract.
// ---------------------------------------------------------------------------
struct UnitSubObjectArrayHeader {
    std::uint32_t header_word{0}; // +0h, not written by either constructor
    std::uint32_t first{0};       // +4h
    std::uint32_t last{0};        // +8h
    std::uint32_t end{0};         // +0Ch
};

// ---------------------------------------------------------------------------
// unit+72Ch: 00809270. Body 00809270..00809280, __thiscall(self), plain RET.
// ---------------------------------------------------------------------------
struct UnitOwnedRefSlot {
    std::uint32_t vptr{kUnitSubObjVtableOwnedRefSlot}; // +0h, 00809274
    std::uint32_t field_4{0};                          // +4h, 0080927A, no reader read
    // +8h, 0080927D. An optional pointer this base owns: the reset routine
    // 00809650 calls slot 0 of the pointee's vtable with the scalar deleting
    // flag 1 and then clears the field.
    std::uint32_t owned{0};
};

// ---------------------------------------------------------------------------
// unit+A20h: 0093BCC0, __thiscall(self, unit), RET 4, returns self.
// ---------------------------------------------------------------------------
inline constexpr int kUnitSubObjBlockA20ArrayCount = 0x14; // 0093BD5A PUSH 0x14

struct UnitSubObjectA20 {
    std::uint32_t owner_unit{0};           // +0h,  0093BD13, the argument
    UnitSubObjectArrayHeader dword_array;  // +4h,  0093BCE4..0093BCEA zero +8h/+0Ch/+10h
    std::uint32_t field_18{0};             // +18h, 0093BCF1
    std::uint32_t field_1c{0};             // +1Ch, 0093BCF4
    std::uint32_t field_20{0};             // +20h, 0093BCF7
    std::uint32_t field_24{0};             // +24h, 0093BD15
    float field_28{1.0f};                  // +28h, 0093BD06, 00D7A24C
    float settings_3b0{0.0f};              // +2Ch, 0093BD2D, 00424C40()+3B0h
    float settings_3ac{0.0f};              // +30h, 0093BD3F, 00424C40()+3ACh
    float field_34{0.0f};                  // +34h, 0093BD18
    float field_38{0.0f};                  // +38h, 0093BD1D
    float field_3c{0.0f};                  // +3Ch, 0093BD50
    float field_40{0.0f};                  // +40h, 0093BD55
    std::uint8_t flag_44{0};               // +44h, 0093BD69
    std::uint8_t flag_45{1};               // +45h, 0093BD48
    std::uint8_t flag_46{1};               // +46h, 0093BD4C
};

// ---------------------------------------------------------------------------
// The 1Ch element of the inner arrays: 0080FAE0, read from raw bytes
// 0080FAE0..0080FB03 (no Ghidra function).
// ---------------------------------------------------------------------------
inline constexpr std::size_t kUnitSubObjInnerElementSize = 0x1c;
inline constexpr int kUnitSubObjInnerElementCount = 2;

struct UnitRecordInnerElement {
    float field_0{0.0f};     // +0h,  0080FAF9
    float field_4{0.0f};     // +4h,  0080FAEF
    float field_8{1000.0f};  // +8h,  0080FAF4, 00CE3804
    std::uint8_t flag_c{0};  // +0Ch, 0080FAFD
    std::uint32_t field_18{0}; // +18h, 0080FB00
    // +10h..+17h and +1Ch are not written by the element constructor.
};

// ---------------------------------------------------------------------------
// unit+A98h and unit+AECh: the two records of the loop at 0081EF63..0081EFB1.
// ---------------------------------------------------------------------------
struct UnitSubObjectRecord {
    float field_0{0.0f};  // +0h,  0081EF8D
    float field_4{-1.0f}; // +4h,  0081EF91, 00D7A260
    UnitRecordInnerElement inner[kUnitSubObjInnerElementCount]; // +8h..+3Fh, 0081EF7D
    float field_40{0.0f}; // +40h, 0081EFA3
    float field_44{0.0f}; // +44h, 0081EF9E
    float field_48{0.0f}; // +48h, 0081EF99
    std::uint8_t flag_4c{0}; // +4Ch, 0081EFA8
    // +50h: zeroed at 0081EF96, then the unit pointer at 0081F1E6 (record 0,
    // unit+AE8h) and 0081F1EC (record 1, unit+B3Ch). Non-owning.
    std::uint32_t owner_unit{0};
};

// ---------------------------------------------------------------------------
// unit+BD0h: 00815600. Body 00815600..00815672, __thiscall(self), returns self.
// ---------------------------------------------------------------------------
inline constexpr int kUnitPoseHistorySlotCount = 40;      // ECX 27h..-1 at 0081560C
inline constexpr std::size_t kUnitPoseHistorySlotStride = 0x18; // 008100DC ECX + EDX*18h
inline constexpr std::size_t kUnitPoseHistorySlotBase = 0x08;   // slot i at self+8h+i*18h
inline constexpr std::size_t kUnitPoseHistoryOffCursor = 0x3c8; // 00810034, value 27h
inline constexpr std::size_t kUnitPoseHistoryOffFlag = 0x3cc;   // 00810027, value 0
inline constexpr std::size_t kUnitPoseHistoryOffOrigin = 0x3d0; // 00815647..00815667
inline constexpr std::uint32_t kUnitPoseHistorySeedGlobal = 0x00f87574;

// One ring slot. 00810020 writes all five floats; the constructor's zero loop at
// 00815614..00815623 writes only the last two, at slot+0Ch and slot+10h.
struct UnitPoseHistorySlot {
    float field_0{0.0f};  // +0h,  008100E4 path, fill only
    float field_4{0.0f};  // +4h,  fill only
    float field_8{0.0f};  // +8h,  00810106, fill only
    float field_c{0.0f};  // +0Ch, 00810119 in the fill, zeroed at 00815614
    float field_10{0.0f}; // +10h, 00810126 in the fill, zeroed at 00815619
    // +14h..+17h: stride padding, never written.
};

struct UnitPoseHistoryRing {
    std::uint32_t vptr{kUnitSubObjVtablePoseHistoryRing}; // +0h,  00815606
    // +4h: the handle BSP_CriticalSection_Create (00BD1860) returns, stored at
    // 00815637. The plain destructor 00818100 hands the same field to
    // 0041CC80 BSP_CriticalSection_DestroyOwned, so this object owns it.
    std::uint32_t critical_section{0};
    UnitPoseHistorySlot slots[kUnitPoseHistorySlotCount]; // +8h..+3C7h
    std::uint32_t cursor{0};   // +3C8h
    std::uint8_t flag{0};      // +3CCh
    float origin[3]{};         // +3D0h..+3D8h, copied from 00F87574..00F8757C
};
static_assert(kUnitPoseHistorySlotBase +
                      kUnitPoseHistorySlotCount * kUnitPoseHistorySlotStride ==
                  kUnitPoseHistoryOffCursor,
              "40 slots of 18h from +8h must end exactly at the cursor at +3C8h");

// ---------------------------------------------------------------------------
// unit+10A0h: inline at 0081F102..0081F154, standalone copy 0081EB90.
// ---------------------------------------------------------------------------
struct UnitSubObjectBlock10A0 {
    std::uint32_t vptr{kUnitSubObjVtableBlock10A0}; // +0h,  0081F102 / 0081EBB2
    float field_4{1.0f};                            // +4h,  0081F10C / 0081EBB8
    // +8h: elements of 8 bytes; the destructor 0081EC00 releases each through
    // 00867B10 (0081EC32 LEA ESI,[EBX+8], 0081EC41 SAR EAX,3).
    UnitSubObjectArrayHeader list_a;
    // +18h: the destructor releases each element through 004CC760 and then
    // frees the storage (0081ECA3 LEA EDI,[EBX+18h], 0081ECBF _free).
    UnitSubObjectArrayHeader list_b;
    std::uint32_t field_28{0}; // +28h, 0081F140 / 0081EBD7
    std::uint32_t field_2c{0}; // +2Ch, 0081F146 / 0081EBDA
    float field_30{60.0f};     // +30h, 0081F14C / 0081EBE1, 00CEB4B0
};

// ---------------------------------------------------------------------------
// unit+10D4h: 0074E7B0. Body 0074E7B0..0074E7ED. The object's use is the
// flooding model of docs/UNIT_CONTROLLER_UPDATE.md and is a contract here; only
// the constructor's own field writes are reconstructed.
// ---------------------------------------------------------------------------
struct UnitLeakManagerConstruction {
    std::uint32_t vptr{kUnitSubObjVtableLeakManager}; // +0h,  0074E7D1
    // +4h..+14h hold the list head the contract reads (its length is at +14h)
    // and the constructor does not write them. See the open question in the doc.
    std::uint32_t field_18{0};  // +18h, 0074E7DA
    std::uint32_t field_1c{0};  // +1Ch, 0074E7D7
    std::uint32_t field_20{0};  // +20h, 0074E7DD
    std::uint8_t flag_24{0};    // +24h, 0074E7E0
    float field_28{0.0f};       // +28h, 0074E7E3, the total water of the contract
    float field_2c{0.0f};       // +2Ch, 0074E7E8
    float tuning_38{200.0f};    // +38h, 0074E7BC, 00CE386C
    float tuning_3c{60.0f};     // +3Ch, 0074E7C9, 00CEB4B0
};

// ---------------------------------------------------------------------------
// Integration boundary. One virtual per native call site the constructors make
// out of the reconstruction's reach. Nothing has a default implementation:
// none of these stands in for unrecovered game behaviour.
// ---------------------------------------------------------------------------
struct UnitSubObjectHost {
    virtual ~UnitSubObjectHost() = default;

    // 00BD1860 BSP_CriticalSection_Create at 00815625; the result is stored at
    // ring+4h and released by 0041CC80 through the destructor 00818100.
    virtual std::uint32_t create_critical_section() = 0;

    // 00810020 at 0081563A (ECX = the ring, arguments 00F87574 and 0.0f) and at
    // 00819381 inside 00818EA0 (ECX = unit+BD0h, arguments unit+FCh and the
    // float the virtual at [EDX+50h] returns). Fills all 40 slots and leaves the
    // cursor at 27h; RET 8.
    virtual void fill_pose_history_ring(UnitPoseHistoryRing& ring,
                                        const float position[3],
                                        float angle) = 0;

    // 00424C40, the settings singleton, called once per field at 0093BD22 and
    // 0093BD30. The two offsets are the only fields this packet reads.
    virtual float settings_float_3b0() = 0;
    virtual float settings_float_3ac() = 0;

    // 00822460 at 0093BD60 with ECX = sub-object+4h, arguments (&zero, 14h) and
    // an element size of 4 (0082248F..00822492 SAR EAX,2).
    virtual void array_resize(UnitSubObjectArrayHeader& array,
                              int count,
                              std::uint32_t fill) = 0;

    // The three CRT array constructions: 00BF7CD1 at 0081EF24, 0081EFCE and
    // 0081EFED, each with element constructor 0043F620 (one zeroed dword) and
    // destructor 00440A30.
    virtual void construct_dword_array(std::size_t unit_offset, int count) = 0;
};

// ---------------------------------------------------------------------------
// The reconstructed constructors. Each reproduces the native field writes in
// native order and calls the host once per native call site.
// ---------------------------------------------------------------------------

// 00809270, and the same object at the same unit offset from 00745940 at
// 00745978.
void construct_unit_owned_ref_slot_00809270(UnitOwnedRefSlot& self) noexcept;

// 00809650: the reset path, which establishes what +8h is. `release` stands for
// the indirect call at 00809666 to slot 0 of the pointee's vtable with the
// scalar deleting flag 1; it is not called when +8h is already null.
void reset_unit_owned_ref_slot_00809650(UnitOwnedRefSlot& self,
                                        void (*release)(std::uint32_t owned,
                                                        int deleting_flag)) noexcept;

// 0093BCC0. Returns self in EAX; the caller 0081ED40 ignores it.
UnitSubObjectA20& construct_unit_sub_object_a20_0093bcc0(UnitSubObjectA20& self,
                                                         std::uint32_t owner_unit,
                                                         UnitSubObjectHost& host);

// 0080FAE0, the inner element constructor.
void construct_unit_record_inner_element_0080fae0(UnitRecordInnerElement& self) noexcept;

// One iteration of the loop at 0081EF63..0081EFB1. The unit back-pointer at
// +50h is written afterwards by 0081F1E6 and 0081F1EC, so it is a separate
// argument rather than part of the loop body.
void construct_unit_sub_object_record_0081ef63(UnitSubObjectRecord& self) noexcept;

// 00815600. Returns self in EAX.
UnitPoseHistoryRing& construct_unit_pose_history_ring_00815600(
    UnitPoseHistoryRing& self,
    const float seed_position[3],
    UnitSubObjectHost& host);

// 0081EB90, which is byte-for-byte the field set the inline block at
// 0081F102..0081F154 writes.
void construct_unit_sub_object_block_10a0_0081eb90(UnitSubObjectBlock10A0& self) noexcept;

// 0074E7B0.
void construct_unit_leak_manager_0074e7b0(UnitLeakManagerConstruction& self) noexcept;

// The sub-object region of 0081ED40, 0081ED6F..0081F1F8, as the order in which
// the sub-objects are built. `unit_pointer` is the value stored into the two
// records' +50h fields and passed to 0093BCC0.
struct UnitInstanceSubObjects {
    UnitOwnedRefSlot owned_ref_slot;                          // +72Ch
    UnitSubObjectA20 block_a20;                               // +A20h
    UnitSubObjectRecord records[kUnitSubObjRecordCount];      // +A98h, +AECh
    UnitPoseHistoryRing pose_history_ring;                    // +BD0h
    UnitSubObjectBlock10A0 block_10a0;                        // +10A0h
    UnitLeakManagerConstruction leak_manager;                 // +10D4h
};

void construct_unit_instance_sub_objects_0081ed40(UnitInstanceSubObjects& self,
                                                  std::uint32_t unit_pointer,
                                                  const float ring_seed_position[3],
                                                  UnitSubObjectHost& host);

} // namespace bsp
