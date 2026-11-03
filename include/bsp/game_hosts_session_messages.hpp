#pragma once
// Host-owned image memory for session messages (cc15-sm1, the bring-up pattern).
//
// The image builds a session message on the caller's stack or heap and hands it
// to 0077C2A0. The local route 0076E520 never queues that object: it writes the
// message into a 400h-byte stack stream (vtable[4h] at 0076E575), then rebuilds
// a second message from those bits with the factory 00768530 (0076E5A4), and it
// is that copy the queue holds until the drain frees it. So a queued message
// carries only what its write serialises: positions quantised, the object
// pointer dropped in favour of the 13-bit handle, the command reduced to its
// ordinal byte.
//
// This class owns the image-shaped context those bodies read (the session's
// game object, the bit codec's constants) and runs the round trip through the
// cc13 reconstructions. Each message is a host value: the sent one lives for the
// call, as the image's stack message does; the queued one is returned to the
// caller, who keeps it until its delivery and then calls the destructor, which
// is where the image's drain frees it.
#include "bsp/entity_orders.hpp"
#include "bsp/native_session_message_tags_90_to93.hpp"
#include <cstdint>
#include <memory>

namespace bsp::game {
struct GunAimMessage79;
}

namespace bsp {

// cc15-smA. True: the MT_VEHICLE_GUN_CONTROL (79h) that 005484F0 builds with
// 00954A10 (005489DA) and routes per group entry with 0077C2A0(flags 3) at
// 00548A1F reaches 00959C20 as the copy 0076E520 rebuilds from its bits
// (0075FFF0 write; 00768530 case 79h, jump table 0076A298+78h*4 = 00769132:
// operator new(3Ch), 0075FF60; 0076A27B vtable+8 = 007600D0 read; vtable
// 00D032D0 read from the PE). The angles are 005484F0's (00521370 on the
// mover's forward row, 0054884D) with 00954A10's +30h clamp to +/-1.57f;
// on the copy +2Ch/+30h are 18-bit over 6.3 (00D032E4), the camera raw
// (FLT_MAX, 00D7A248), the group 3 bits, the bools 1 bit, the target id 12
// bits (kept as posted while has-target is set: this host's ids exceed 12
// bits). ABI: 00954A10 __thiscall RET 28h (group, x, y, z, yaw, pitch, held,
// pressed, has_target, id); 0075FFF0 / 007600D0 __thiscall RET 4.
// UNCERTAIN: delivery stays in place at the route (the image delivers at the
// drain); 00521370's atan2 is the double one (x87 may differ in the last
// bit). False: the posted value, angles recomputed from `forward` at
// delivery. The pair is cc15's; cc9 flips.
// ON (cc9-gunnery53): USN01 pair on b8dcae593, reference env, Northampton's
// artillery fired through the harness `input` line (0x9D, then 0x99 held 300
// frames at mission frames 502/902/1302, inside screen 2Eh's window before the
// 1823 ScoutDauntless hand-over). 1298 round trips, all group 3; the copy moves
// yaw on 1218 and pitch on all (max 1.68e-5 / 6.91e-6 rad), the bools, group and
// target never; 99h reached 00959F72 (0095A05B record 2700 calls both sides).
// Gameplay, death rows and the unit table identical. Flipped by mechanism.
inline constexpr bool kGunAimMessageCc15Bound = true;

class HostSessionMessages {
public:
    HostSessionMessages();
    ~HostSessionMessages();
    HostSessionMessages(const HostSessionMessages&) = delete;
    HostSessionMessages& operator=(const HostSessionMessages&) = delete;

    // MT_GAMEUNIT_SETCMD: 0071C830 at 0071ED6C over a stack message, then
    // 0076E520's local copy (0071C930 write, 00768530 type byte and rewind,
    // case 92 at 00769D10: 00764F90, then 0071CA30 read). `command_ordinal` is
    // the command object's +4h ordinal, or negative for a null command (0071C830
    // then writes 0FFh). Returns the queued copy.
    NativeSessionMessage92 queue_set_command_92(int command_ordinal,
                                                const SceneCommandTarget& target,
                                                std::uint8_t flag) const;
    // 0071CB40, the destructor the drain's scalar delete 0071E3B0 runs. The
    // memory is the caller's, so only the destructor runs.
    static void free_set_command_92(NativeSessionMessage92& message) noexcept;

    // MT_COMMAND (58h): the message 007798D0 built, as the typed record that
    // mirrors its 3Ch layout, through 0076E520's local copy (00764D40 write,
    // 00768530 case 88 at 00769CB0: 00764C80, then 00764E60 read). Returns the
    // posted record with the fields the copy carries replaced by the copy's:
    // the command ordinal, the flags bit, the target. The copy is destroyed
    // (00764CF0) here because the record is all the delivery reads.
    EntityOrderMessage queue_command_88(const EntityOrderMessage& posted) const;

    // MT_VEHICLE_GUN_CONTROL (79h, cc15-smA): 00954A10 at 005489DA over the
    // HUD's posted fields, routed by 0077C2A0 at 00548A1F (flags 3, local
    // bit), through 0076E520's local copy (0075FFF0 write, 00768530 case 79h
    // at 00769132: 0075FF60, then 007600D0 read). 00954A10 takes the two
    // angles 005484F0 made with 00521370 from the mover's forward row
    // (0054884D), so they are made here from `posted.forward`. Returns the
    // posted value with the copy's fields: group (3 bits), camera (raw),
    // +2Ch yaw and +30h pitch (18 bits over 6.3, 00D032E4), the three bools,
    // and `angles_carried` set. The target id is the posted one while the
    // has-target bit is set: the copy carries 12 bits and this host's ids do
    // not fit. The drain's delete (007605F0) only restamps +0h, so the copy
    // ends here.
    game::GunAimMessage79 queue_gun_aim_79(const game::GunAimMessage79& posted) const;

    // Goal 4 chain 260 (kWireGoal4Chain260Bound). MT_FORMATION_JOIN (76h,
    // 0077C8D0: 0075B430 at 0077C926, vtable 00D02D30, +4h = 1, +18h/+1Ah/+1Ch
    // zero, the other entity's id at +20h, routed at 0077C964 flags 7) or
    // MT_FORMATION_LEAVE (77h, 0077C980: 0075B430(77h), vtable 00D02D44, the
    // same stores, +20h = other ? other+174h : 0), through 0076E520's local
    // copy: 0075C7C0 / 0075C860 write, 00768530 case 76h at 007690E6 (0075A2E0)
    // or 77h at 00769106 (0075A330), then 0075C810 / 0075C8B0 read. `type` is
    // 0x76 or 0x77. Returns the copy's +20h member id (12 bits on the wire).
    // The router's +1Ch audit marker (0077C37F, [00E18DB7]) is not set here:
    // this process does not model the latch and no 76h/77h arm reads +1Ch.
    std::uint16_t queue_formation_member_76_77(std::uint8_t type,
                                               std::uint16_t member_id) const;
    // Goal 4 chain 260. MT_SHIPREPAIR_BODYREPAIR (9Fh, 008AD330 RepairEnable:
    // 0075B430(9Fh) at 008AD494, +18h = 0, +4h = 1, +1Ah = 0, vtable 00D03360,
    // +1Ch = the flag; 0077C2A0 at 008AD4CD flags 7). Those are 00760A50's
    // stores, so the sent message is built with it, then 00760AE0 write,
    // 00768530 case 9Fh at 0076960E (00760A50), 00760B30 read. Returns the
    // copy's +1Ch bool, the flag 00821E80's 9Fh arm hands 00939FD0.
    bool queue_repair_body_9f(bool enabled) const;
    // Goal 4 chain 260. MT_SHIP_LEAK (90h): the sender 0080FF80 makes
    // 00761F60's stores (0075B430(90h), the zero header, +4h = 1, vtable
    // 00D034A0) plus +1Ch = the leak count and +20h..+28h = the unit-local
    // point, so the sent message is 00761F60 with those fields; then 00761FF0
    // write (count 6 bits, three raw floats), 00768530 case 90h at 0076946E
    // (00761F60), 00762090 read. The receiver 008221A7 -> 0074F440 reads the
    // copy's count and point.
    struct ShipLeakCopy {
        std::uint32_t count{0};
        float point[3]{0.0f, 0.0f, 0.0f};
    };
    ShipLeakCopy queue_ship_leak_90(std::uint32_t count, const float (&point)[3]) const;
    // Goal 4 chain 260. MT_SHIP_AVOIDSIDE (8Fh): the sender 009D66B0 makes
    // 007623D0's stores (0075B430(8Fh), +18h/+1Ah zero, +4h = 1, vtable
    // 00D034DC) plus +1Ch = the observed unit's +174h word and +20h = side;
    // then 00762460 write (12-bit id, 3-bit signed side), 00768530 case 8Fh
    // at 0076944E (007623D0), 007624B0 read. Returns the copy's two fields.
    struct ShipAvoidSideCopy {
        std::uint16_t other_id{0};
        std::int32_t side{0};
    };
    ShipAvoidSideCopy queue_ship_avoid_side_8f(std::uint16_t other_id, std::int32_t side) const;
    // Goal 4 chain 260. MT_AIRBASE_LAUNCHSLOT (83h): 00896750 LaunchAirBaseSlot
    // at 00896915..0089694C makes 007631A0's stores (0075B430(83h), +18h/+1Ah/
    // +1Ch zero, +4h = 1, vtable 00D03554) plus +20h = slot - 1 and +24h = the
    // `now` byte, routed at 00896964 (flags 0); then 00763260 write (6-bit
    // slot, 1-bit flag), 00768530 case 83h at 00769246 (007631A0), 007632C0
    // read. The receiver 006CA8E0 reads the copy's slot and flag.
    struct AirbaseLaunchSlotCopy {
        std::uint32_t slot{0};
        bool now{false};
    };
    AirbaseLaunchSlotCopy queue_airbase_launch_slot_83(std::uint32_t slot, bool now) const;
    // Goal 4 chain 260. The weapon director's kind 5Ah (session message 90):
    // 0071DD30 (sub-kind 2) at 0071DD4B..0071DD81: 0075B430(5Ah), +18h word 0,
    // +1Ah 0, +4h = 1, +1Ch 0, vtable 00CFD9C4, +20h = the sub-kind, +24h =
    // the value (gameplay_seg38_cc13's director_sub_kind_message), routed at
    // 0071DD94 flags 7 to [director+34h]; then 0071C670 write (4-bit
    // sub-kind; the value in 9 bits for sub-kind 2, else 1 bit), 00768530
    // case 5Ah at 00769CE4 (0075AF60), 0071C6C0 read. Returns the copy's
    // +20h and +24h, what 0071C1E0 receives.
    struct DirectorSubKindCopy {
        std::uint32_t sub_kind{0};
        std::uint32_t value{0};
    };
    DirectorSubKindCopy queue_director_sub_kind_5a(std::uint32_t sub_kind,
                                                   std::uint32_t value) const;
    // Goal 4 chain 260. MT_GAMEUNIT_CLEARCMD (5Dh, session message 93):
    // 0071C730's stores (0075B430(5Dh), +18h/+1Ah/+1Ch zero, +20h = arm, +4h
    // = 1, vtable 00CFD9D8, +24h = index), then 0071C7A0 write (arm 1 bit,
    // index 5 signed bits), 00768530 case 5Dh at 00769D44 (0075B030), 0071C7E0
    // read. Returns the copy's +20h (non-zero as 1) and +24h, what 00721A40's
    // 5Dh arm reads.
    struct ClearCommandCopy {
        std::uint8_t arm{0};
        std::int32_t index{0};
    };
    ClearCommandCopy queue_clear_command_5d(std::uint8_t arm, std::int32_t index) const;

private:
    struct Image;
    std::unique_ptr<Image> image_;
};

} // namespace bsp
