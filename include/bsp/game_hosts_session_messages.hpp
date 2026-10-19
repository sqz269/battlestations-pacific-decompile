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

private:
    struct Image;
    std::unique_ptr<Image> image_;
};

} // namespace bsp
