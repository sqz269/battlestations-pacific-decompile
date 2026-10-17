#pragma once
// Packet cc13-big47b (cc13_big47b marker): large segment 47 (007B2DD0-007D1DC0,
// flight) bodies that passes cc13-seg47 / cc13-seg47b deferred. docs/CC13_BIG47B.md
// lists each function with its ABI, evidence and uncertainty.
//
// Descriptive names are hypotheses, not recovered symbols. Nothing here is
// wired and nothing is ABI-compatible; status is reconstructed and build-tested.

#include <cstddef>
#include <cstdint>

#include "bsp/cc13_seg47b.hpp"
#include "bsp/native_bit_cursor_numeric.hpp"
#include "bsp/native_bit_cursor_read.hpp"
#include "bsp/native_session_message_base.hpp"

namespace bsp::cc13_big47b {

using cc13_seg47::Raw;

// The C1h PLANE_SYNC message (00D02E0C profile; 007BDD30 builds it in a B4h
// stack buffer, 0075AAB0 is its ConstructForRead). Offsets are the ones the
// 007C7A40 writer and 007C8A80 reader touch; the names describe the codec, not
// recovered identifiers. Unlisted bytes are not touched by either function.
struct PlaneSyncMessage {
    NativeSessionMessageStorage base;   // +00h
    std::uint16_t id_18;                // +18h  0075B480/0075B4C0 header
    std::uint8_t relay_1a;              // +1Ah
    std::uint8_t pad_1b;
    void* peer_record_1c;               // +1Ch  007C2810 per-peer baseline record (writer only)
    float angle_20[3];                  // +20h  scale pi, signed, (7-level)*2 bits
    float pos_x_2c;                     // +2Ch  scale 24000, signed, 22-3*level bits
    float pos_y_30;                     // +30h  sent as y+150 in [0,2047], scale 2048, 20-3*level bits
    float pos_z_34;                     // +34h
    float unit_38;                      // +38h  scale 1.0, zero-flag, signed, 8 bits
    float unit_3c;                      // +3Ch
    float unit_40;                      // +40h
    float unit_44;                      // +44h  clamped to [0,1] by the writer, 7 bits
    float unit_48;                      // +48h  7 bits
    std::uint8_t opaque_4c[2];
    std::uint8_t flag_4e;               // +4Eh
    std::uint8_t opaque_4f;
    float vel_50[3];                    // +50h  60/10 bits when |v| < 60.0, else 1024/16 bits
    float pair_5c;                      // +5Ch  scale 100, (8-level)*2 bits
    float pair_60;                      // +60h
    float divisor_64;                   // +64h  sent as v+80 in [0,1024], (9-level)*2 bits
    float rate_68[3];                   // +68h  level-0 form 0.5/14 or 6.0/18; else 8.0, 15-2*level
    std::uint8_t flag_74;               // +74h
    std::uint8_t flag_75;               // +75h
    std::uint8_t compact_76;            // +76h  reader: 0 full form, 1 compact form
    std::uint8_t opaque_77;
    float wire_rope_78;                 // +78h  state 4 only; scale tuning+51Ch MaxWireRope, 10 bits
    float zero_7c;                      // +7Ch  reader stores 0
    float vec_80[3];                    // +80h  reader copies 00F87574..00F8757C
    std::uint8_t state_8c;              // +8Ch  3 bits; 7 = none
    std::uint8_t opaque_8d[7];
    std::int32_t level_94;              // +94h  2 bits, compact form only
    std::uint8_t opaque_98[4];
    std::uint8_t force_9c;              // +9Ch  writer: send every delta field
    std::uint8_t skip_9d;               // +9Dh
    std::uint8_t flag_9e;               // +9Eh
    std::uint8_t flag_9f;               // +9Fh
    std::uint8_t compact_a0;            // +A0h  peer record +2Ch == 0
    std::uint8_t has_44_a1;             // +A1h  reader presence bits A1h..AEh
    std::uint8_t has_48_a2;
    std::uint8_t has_38_a3;
    std::uint8_t has_3c_a4;
    std::uint8_t has_40_a5;
    std::uint8_t has_angle_a6[3];       // +A6h
    std::uint8_t has_5c_a9;
    std::uint8_t has_60_aa;
    std::uint8_t has_64_ab;
    std::uint8_t has_rate_ac[3];        // +ACh
    std::uint8_t opaque_af[5];
};
static_assert(offsetof(PlaneSyncMessage, peer_record_1c) == 0x1C);
static_assert(offsetof(PlaneSyncMessage, unit_44) == 0x44);
static_assert(offsetof(PlaneSyncMessage, flag_4e) == 0x4E);
static_assert(offsetof(PlaneSyncMessage, vel_50) == 0x50);
static_assert(offsetof(PlaneSyncMessage, rate_68) == 0x68);
static_assert(offsetof(PlaneSyncMessage, wire_rope_78) == 0x78);
static_assert(offsetof(PlaneSyncMessage, state_8c) == 0x8C);
static_assert(offsetof(PlaneSyncMessage, level_94) == 0x94);
static_assert(offsetof(PlaneSyncMessage, force_9c) == 0x9C);
static_assert(offsetof(PlaneSyncMessage, compact_a0) == 0xA0);
static_assert(offsetof(PlaneSyncMessage, has_angle_a6) == 0xA6);
static_assert(offsetof(PlaneSyncMessage, has_rate_ac) == 0xAC);
static_assert(sizeof(PlaneSyncMessage) == 0xB4);

struct PlaneSyncHost {
    virtual ~PlaneSyncHost() = default;
    // 0042E740 BSP_GameTuning_GetSingleton, no input, EAX = the 6D0h tuning
    // block (lazily allocated); float +51Ch is Pilot/Landing/MaxWireRope.
    virtual void* game_tuning_get_singleton_0042e740() = 0;
    // The image's three floats at 00F87574..00F8757C (.bss, CG_static_init).
    virtual const volatile float* global_vector_00f87574() = 0;
};

// 007C7A40 BSP_SessionMessage_PLANE_SYNC_Write, __thiscall(ECX = message,
// cursor) RET 4. Writes against the per-peer baseline record +1Ch and updates it.
void plane_sync_message_write_007c7a40(PlaneSyncMessage* message, NativeBitCursor* cursor,
                                       PlaneSyncHost& host, const NativeBitNumericContext& numeric);

// 007C8A80 BSP_SessionMessage_PLANE_SYNC_Read, __thiscall(ECX = message,
// stream) RET 4; the bit readers get stream+4.
void plane_sync_message_read_007c8a80(PlaneSyncMessage* message, NativeSessionReadStream* stream,
                                      PlaneSyncHost& host, const NativeBitNumericContext& numeric);

// The 0Ch-byte section reference 00BD5010 builds in place in the outgoing
// argument area; 007DEB70 / 007E0D20 take it by value (it is their stack
// argument block, RET 0Ch).
struct ArchiveSectionRef {
    std::uint32_t words[3];
};

// The plane save reuses pass two's host (archive enter/leave +04h/+08h,
// operator new 00BF681B, 007C5AC0, and what 007C18B0 needs) and adds the
// callees no earlier cc13_seg47* host names.
struct PlaneSaveHost : cc13_seg47b::Seg47bHost {
    // Archive vtable +0Ch write(key, value), __thiscall, both pairs by value,
    // RET 10h. Value kinds here: 1 int, 2 float (bits in .value), 3 bool (only
    // the low payload byte is stored by the image), 4 object pointer.
    virtual void archive_write_0c(Raw archive, cc13_seg47::ArchiveKey key, cc13_seg47::ArchiveKey value) = 0;
    // 0095BB10, __thiscall(ECX = unit; archive), RET 4: the shared vehicle-unit
    // section (reconstructed as cc13_seg62 vehicle_unit_write_state_0095bb10).
    virtual void vehicle_unit_write_state_0095bb10(Raw unit, Raw archive) = 0;
    // 006E6E80 BSP_BulletBase_WriteState, __thiscall(ECX = plane+72Ch; archive), RET 4.
    virtual void bullet_base_write_state_006e6e80(Raw bullet_base, Raw archive) = 0;
    // 007D9B70 BSP_PlaneFlightController_SaveToArchive, __thiscall(ECX = plane+AB0h; archive), RET 4.
    virtual void flight_controller_save_007d9b70(Raw controller, Raw archive) = 0;
    // 0099A880 BSP_PilotBot_Construct, __thiscall(ECX = memory), EAX = this.
    virtual void* pilot_bot_construct_0099a880(void* memory) = 0;
    // PilotBot vtable 00D1F348 slot +4 (00D1F34C = 009995A0 BSP_PilotBot_Attach),
    // __thiscall(ECX = bot; plane+310h), RET 4, called through the vtable.
    virtual void pilot_bot_vcall_attach_04(Raw bot, Raw unit) = 0;
    // 00BD5010 BSP_ArchiveWriterRef_Open, __thiscall(ECX = archive; out, key pair) RET 0Ch -> out.
    virtual ArchiveSectionRef* archive_writer_ref_open_00bd5010(Raw archive, ArchiveSectionRef* out,
                                                                cc13_seg47::ArchiveKey key) = 0;
    // 007DEB70 BSP_ActuatorBlock_SaveToArchive, __thiscall(ECX = [plane+DECh]; ref by value), RET 0Ch.
    virtual void actuator_block_save_007deb70(Raw block, ArchiveSectionRef ref) = 0;
    // 007E0D20 BSP_PlaneStateBlock_SaveToArchive, __thiscall(ECX = plane+C98h; ref by value), RET 0Ch.
    virtual void plane_state_block_save_007e0d20(Raw block, ArchiveSectionRef ref) = 0;
};

// 007C9860 BSP_Plane_SaveToArchive, __thiscall(ECX = plane; archive) RET 4,
// SEH frame (state 0 only around the PilotBot constructor).
void plane_save_to_archive_007c9860(PlaneSaveHost& host, cc13_seg47::Raw plane, cc13_seg47::Raw archive);

}  // namespace bsp::cc13_big47b
