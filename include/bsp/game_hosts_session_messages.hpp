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
#include "bsp/native_session_message_tags_94_to97.hpp"
#include <cstdint>
#include <memory>

namespace bsp::game {
struct GunAimMessage79;
}

namespace bsp::gameplay_seg41_cc13 {
class LandingShipMessageHost;
}

namespace bsp::cc13_powerup { struct PowerupItem; }
namespace bsp::game_powerups { struct PowerupModifier; }

namespace bsp::gameplay_seg47_cc13 {
struct FailureMessage;
struct PlaneFailureState;
struct StateMessageEnv;
struct StateMessageHost;
}

namespace bsp {

class SessionParticipantPools;

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

    // 008EDDC0/008E9FD0 and 008E8C30/008EA250 construct these stack packets
    // even when the host-only transport has no peers. Copy the real payload
    // into its owned pooled string, then take its native normal destructor.
    // No receive/loopback occurs. False leaves unsupported bound-entity sends
    // with the caller: their +174h identity is not supplied by an index token.
    bool broadcast_powerup_item(std::int32_t slot, const cc13_powerup::PowerupItem&,
        std::int32_t operation, const SessionParticipantPools&, const std::size_t& local_slot) const;
    bool broadcast_powerup_modifier(const game_powerups::PowerupModifier&,
        std::int32_t operation, const SessionParticipantPools&, const std::size_t& local_slot) const;

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

    // Chain 498: the actual decoded 3Ch allocation, retained by its loopback
    // entry through delivery. Its last owner runs 0071E3B0 with flags = 1.
    std::shared_ptr<NativeSessionMessage92> queue_owned_set_command_92(int command_ordinal,
        const SceneCommandTarget& target, std::uint8_t flag) const;

    // MT_COMMAND (58h): the message 007798D0 built, as the typed record that
    // mirrors its 3Ch layout, through 0076E520's local copy (00764D40 write,
    // 00768530 case 88 at 00769CB0: 00764C80, then 00764E60 read). Returns the
    // posted record with the fields the copy carries replaced by the copy's:
    // the command ordinal, the flags bit, the target. The copy is destroyed
    // (00764CF0) here because the record is all the delivery reads.
    EntityOrderMessage queue_command_88(const EntityOrderMessage& posted) const;

    // Goal 4 chain 450: 0071D340's actual point and presence byte (1), through
    // the native 5Fh codec and factory constructor. The loopback entry owns the
    // decoded message until delivery; its last owner runs 0071E3F0(1).
    std::shared_ptr<NativeSessionMessage95> queue_user_path_point_5f(const float point[3]) const;

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
    // not fit. The decoded 3Ch allocation remains owned until these fields
    // have been copied; 007605F0 then resets its profile and frees it once.
    // The sender takes the same destructor with flags 0 and stays on the stack.
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
    // 008A8930's kind-2 arm builds MT_VEHICLE_SET_PARTY (7Fh), then the
    // local route writes 007602F0 and rebuilds with 00760260 / 00760340.
    // 0095ABE0's 7Fh arm uses the copy's three-bit unsigned +1Ch party;
    // its current race is read at delivery, rather than stored in the message.
    std::uint32_t queue_vehicle_party_7f(std::uint32_t party) const;
    // Borrow the mission's authoritative participant pool and selected-slot
    // field. Both outlive this units-owned codec. Only SetRace uses this
    // source; older message paths retain their existing contexts.
    void bind_vehicle_race_owner(const SessionParticipantPools& participants,
        const std::size_t& local_slot) const noexcept;

    // Fields read by the 80h writer, borrowed from SetRace's actual posted
    // descriptor. The local codec constructs both real message objects.
    struct VehicleRaceFields {
        std::uint8_t type_10;
        std::uint16_t id_18;
        std::uint8_t relay_1a;
        std::uint32_t race_1c;
    };
    // The local codec retains the low four race bits (00760420 / 00760470);
    // party is read at delivery. Construction reads the bound participant
    // identity; the factory's 20h receive allocation lives through decoding.
    std::uint32_t queue_vehicle_race_80(const VehicleRaceFields& posted) const;
    // Goal 4 chain 260. MT_SHIPREPAIR_BODYREPAIR (9Fh, 008AD330 RepairEnable:
    // 0075B430(9Fh) at 008AD494, +18h = 0, +4h = 1, +1Ah = 0, vtable 00D03360,
    // +1Ch = the flag; 0077C2A0 at 008AD4CD flags 7). Those are 00760A50's
    // stores, so the sent message is built with it, then 00760AE0 write,
    // 00768530 case 9Fh at 0076960E (00760A50), 00760B30 read. Returns the
    // copy's +1Ch bool, the flag 00821E80's 9Fh arm hands 00939FD0.
    bool queue_repair_body_9f(bool enabled) const;
    // Goal 5 chain 630. MT_LANDINGSHIP_STARTLANDING (A5h), a landing ship's
    // order to land at a pad. 009F2304 (the ship AI approach, mode 3) and
    // 0074A56B (0074A4C0, the 94h landing request) each run 00749D90(message,
    // pad, building) on a stack message and route it to the ship with
    // 0077C2A0 flags 7; in single player 0077C2A0 turns any flags but 4 into
    // the local bit (game+1FE4h == 0). The copy: 00749E10 write (header, then
    // the pad's +174h in 13 bits and the building's +174h in 12), 00768530
    // case A5h (20h bytes, 0075AF00 at 007696E8), 00749E40 read. 00780120's
    // ladder over IsType 00749DF0 {A5h, 46h} answers no ladder id, so the
    // session-kind switch hands the copy to the ship's slot 164h, 0074B570,
    // which runs here against `receiver` with the copy's kind and handles.
    // `pad_id` and `building_id` are the host's +174h stand-ins. Returns
    // 0074B570's AL, or false when the copy would not reach that handler.
    bool deliver_land_at_pad_a5(std::uint16_t pad_id, std::uint16_t building_id,
                                gameplay_seg41_cc13::LandingShipMessageHost& receiver) const;
    // Goal 5 chain 732. 0074AF50 lowers the ramp at 0074B080, constructs
    // LANDINGSHIP_OPEN with 00749AA0, then routes it with flags 4 at 0074B0A0.
    // 0077C30D..0077C323 drops exactly flags 4 outside host mode: a local
    // session neither serializes this message nor lowers the ramp again.
    // Build the stack notification using the borrowed mission participant
    // owner and local-slot cell, not Image's legacy zeroed game projection.
    // Evaluate that gate using the live mission state/mode. True means the local send
    // was handled; false leaves non-local transport to the caller's record.
    bool post_landing_ship_open_a6(std::int32_t game_state, std::int32_t session_mode,
        const SessionParticipantPools& participants, const std::size_t& local_slot) const;
    // 007465E0's death-smoke send: 007448B0(7, duration), then 0077C2A0
    // with flags 4. The native router drops this in a local session before
    // serialization or delivery. True means that single-player path ran.
    bool post_land_fort_death_smoke_d1(float duration, std::int32_t game_state,
        std::int32_t session_mode, const SessionParticipantPools& participants,
        const std::size_t& local_slot) const;
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
    // Goal 5 chain 628. MT_AIRBASE_APPEARREADYPLANE (86h): the airfield launch
    // site's tick 006CF980 builds it on its stack with 006BDAC0(plane) at
    // 006CF9B2 (0075B430(86h), +18h/+1Ah/+1Ch zero, +4h = 1, vtable 00CF864C,
    // +20h word = plane+174h) and routes it to the airfield (site+44h) at
    // 006CF9C7, 0077C2A0(msg, 5, 0), the local bit set; then 006BDB30 write
    // (header, +1Ch bit, +20h in 12 bits), 00768530 case 86h at 007692A6
    // (00763440), 006BDB60 read. The receiver 006CD6C0's 86h arm (006CD788)
    // resolves the copy's +20h with 006BDBF0 and hands that plane to the
    // site's vtable[14h] 006CF9F0. `plane_id` is the plane's +174h word;
    // returns the copy's +20h.
    std::uint16_t queue_airbase_appear_ready_plane_86(std::uint16_t plane_id) const;
    // Goal 5 chain 628. MT_AIRBASE_DOLAUNCHSLOT (89h): 006C64B0, the queued
    // slot wait, serves the head slot with 006BC8E0(slot) at 006C6502, which
    // builds the 24h-byte message on its stack (0075B430(89h), +18h/+1Ah/+1Ch
    // zero, +4h = 1, vtable 00CF85F0, +20h = slot) and routes it to the
    // block's owner (+7Ch) with 0077C2A0(msg, 1, 0), the local bit; then
    // 006BC440 write (header, +1Ch bit, +20h in 8 bits), 00768530 case 89h at
    // 00769306 (006BC3E0), 006BC470 read. The receiver 006CD6C0's case 8 runs
    // 006C7490(copy slot, 0). Returns the copy's +20h slot.
    std::uint32_t queue_airbase_do_launch_slot_89(std::uint32_t slot) const;
    // Goal 5 chain 636. MT_AIRBASE_CHANGESLOTSTATUS (82h), the player's order to
    // an air-ops slot: the support manager builds it on its stack with
    // 00656280(slot, order, target entity, player = game+18ECh) (00673A10 at
    // 006748C0 / 0067491B, 00675C40 at 0067A5D1 / 0067A945) and routes it to
    // the block's owner (+7Ch) with 0077C2A0(msg, 0, 0): flags 0 takes the
    // default [00E0AF1C] (1 in the image, 0077C302), and a single-player
    // session (game+1FE4h == 0) takes the local bit (0077C329..0077C333). Then
    // 00656320 write (header, +1Ch bit, +20h in 6 bits, +24h in 3, a presence
    // bit and +28h in 12, +2Ch in 4), 00768530 case 82h at 00769226
    // (operator new(30h), 00763320), 006563A0 read, and the copy's scalar
    // delete 00656420. The receiver 006CD6C0's 82h arm 006CD160 hands 006CCDA0
    // the copy's slot +20h, order +24h, the target its +28h names and player
    // +2Ch. `target_id` is the target's +174h word (0 for none); the player is
    // the image game's +18ECh, as the sender reads it.
    struct AirbaseSlotOrderCopy {
        bool to_unit{false};      // 00780120's ladder reached the unit handler
        std::uint8_t type{0};     // copy +10h
        std::uint32_t slot{0};    // copy +20h
        std::uint32_t order{0};   // copy +24h
        std::uint16_t target_id{0};  // copy +28h
        std::uint32_t player{0};  // copy +2Ch
    };
    AirbaseSlotOrderCopy queue_airbase_slot_order_82(std::uint32_t slot, std::uint32_t order,
        std::uint16_t target_id) const;
    // Goal 5 chain 628. MT_MOTHERSHIP_ELEVATOR_MOVE (87h): 006D0600, the mother
    // ship's elevator site tick, calls 006CFFF0(flag, plane) at 006D071D (0, the
    // landed candidate or null) and 006D078E (1, the ready plane +18h), which
    // builds the 24h-byte message with 00758B90 (0075B430(87h), +18h/+1Ah/+1Ch
    // zero, +4h = 1, vtable 00D01870, +22h = flag, +23h = plane present, +20h =
    // plane+174h or 0) and routes it to the carrier (site+B8h) with
    // 0077C2A0(msg, 5, 0), the local bit; then 00758C20 write (header, the
    // +1Ch/+22h/+23h bits, +20h in 12 bits when +23h), 00768530 case 87h at
    // 007692C6 (00763500), 00758C70 read. The receiver 006D0050 runs on the
    // copy's flag and the plane its +20h names when +23h (00758CF0).
    struct ElevatorMoveCopy {
        bool flag{false};
        bool has_plane{false};
        std::uint16_t plane_id{0};
    };
    ElevatorMoveCopy queue_mothership_elevator_move_87(bool flag, bool has_plane,
                                                       std::uint16_t plane_id) const;
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

    // Goal 4 chain 283. 007C17D0 posts MT_PLANE_CHANGE_STATE (C3h) with
    // +20h = the plane's current +900h byte and +21h = 4. Its inlined header
    // stores match 00762880. The local route 0076E520 writes with 00762940;
    // 00768530 case C3h constructs with 00762880 and reads with 007629A0.
    // Both state bytes occupy three bits. The caller keeps this copy until
    // its existing row-9 delivery; this method does not deliver the message.
    struct PlaneChangeStateCopy {
        std::uint8_t current{0};
        std::uint8_t requested{0};
    };
    PlaneChangeStateCopy queue_plane_change_state_c3(std::uint8_t current,
                                                     std::uint8_t requested) const;

    // C5h is an empty touchdown request: 00762A60 supplies the whole payload.
    // 00D03518 selects 004645B0/004645D0 for its local bitstream copy. Return
    // the decoded type so the caller dispatches that copy to its live plane.
    std::uint8_t queue_plane_touchdown_c5() const;

    // 007CBFA0 calls 00762A00 at 007CC212 / 007CC26E before routing C6h.
    // Build that request with this session's retained header context; return
    // its type for the caller's existing immediate delivery. This is only the
    // constructor: byte +20h is unwritten at both send sites, so its two-bool
    // codec (006EBB10 / 006EBB40) is not bound by inventing a flag value.
    std::uint8_t make_plane_takeoff_c6() const;

    // Goal 4 chain 326 (kWireGoal4Chain326Bound). The command building's
    // MT_COMMANDBUILDING_SETOWNER (D3h) and _SETLEVEL (D4h). Every sender
    // inlines 0075B430 plus 00764220's / 00764280's header stores (+18h word 0,
    // +1Ah 0, +4h = 1, vtable 00CFADBC / 00CFADD0), stores +1Ch and +20h, and
    // routes the 24h-byte message with 0077C2A0 flags 7, which in single player
    // ([00E188A8]+1FE4h == 0, 0077C329..0077C333) is the local bit alone (D5h's
    // flags 4 is dropped there, 0077C30D..0077C323): 0076E520 writes it
    // (006F2520: 6-bit player, bool; 006F25E0: 3-bit level, 4-bit argument)
    // and 00768530 rebuilds the copy (case D3h 00769F59 00764220, case D4h
    // 00769F79 00764280; 0076A27B vtable[8] = 006F2550 / 006F2610). The
    // receiver 006F5460 switches on the copy's +10h type. Returned: that type
    // and the copy's +1Ch / +20h, which are what 006F4D10 / 006F38E0 receive.
    struct CommandBuildingCopy {
        std::uint8_t type{0};
        std::uint32_t value_1c{0};
        std::uint32_t value_20{0};
    };
    // D3h: +1Ch the player slot, byte +20h the flag (006F6760's capture flip
    // sends 0 at 006F72C1, routed at 006F72DE; 008A8D90
    // SetCommandBuildingOwnerPlayer sends 1 at 008A8F13, routed at 008A8F2A).
    CommandBuildingCopy queue_command_building_set_owner_d3(std::uint32_t player,
                                                            bool flag) const;
    // D4h: +1Ch the level, +20h the argument (006F7360's level-up at 006F75CC
    // and 006F4D10's level 0 at 006F538A both store 0 there).
    CommandBuildingCopy queue_command_building_set_level_d4(std::uint32_t level,
                                                            std::uint32_t argument) const;

    // Goal 4 chain 335 (kWireGoal4Chain335Bound). MT_SHIP_ADDTORQUE (93h): the
    // ship-hit sender 0080FFD0 (at 00827312) makes 0075B430(93h), +18h word 0,
    // +1Ah 0, +4h = 1, vtable 00D034C8 and +1Ch..+24h = the torque, routed at
    // 00827329 flags 7. Those header stores are 00762220's, the constructor
    // 00768530 calls for case 93h (0076942E), so both messages are built with
    // it; then 007622B0 write (three floats on 004295C0's raw 32-bit path,
    // scale FLT_MAX) and 00762340 read. 00821E80's 93h arm hands the copy's
    // +1Ch..+24h to 0092BF30 (AddHullTorque).
    struct ShipAddTorqueCopy {
        float torque[3]{0.0f, 0.0f, 0.0f};
    };
    ShipAddTorqueCopy queue_ship_add_torque_93(const float (&torque)[3]) const;
    // Goal 4 chain 335. The kamikaze detonate (70h): 0080FF30 makes
    // 0075B430(70h), +18h word 0, +1Ah 0, +4h = 1, vtable 00D0348C, +1Ch = the
    // other entity's +174h word and +20h..+28h = the contact point. 00768530
    // case 70h constructs with 00761D90 (00769026); 00761E20 write (12-bit
    // id, three raw floats), 00761EC0 read. 00821E80's 70h arm resolves the
    // copy's +1Ch and hands it and the copy's +20h point to 00819A20.
    struct KamikazeDetonateCopy {
        std::uint16_t other_id{0};
        float point[3]{0.0f, 0.0f, 0.0f};
    };
    KamikazeDetonateCopy queue_kamikaze_detonate_70(std::uint16_t other_id,
                                                    const float (&point)[3]) const;
    // Goal 4 chain 335. MT_SHIPREPAIR_ADDDAMAGE (9Eh): the ship-hit sender
    // 0080FA50 (00827372 water, 0082740F fire) makes 0075B430(9Eh), +18h word
    // 0, +1Ah 0, +4h = 1, vtable 00D0334C, +1Ch = the seconds, +20h = the
    // flag byte, +24h = the channel. 007608E0 is those header stores and the
    // constructor of 00768530's case 9Eh (007695EE); 00760970 write (3-bit
    // signed channel, raw float, bool), 007609E0 read. 00821E80's 9Eh arm
    // switches on the copy's +24h (0 fire, 1 water) and +20h (add or set).
    struct RepairAddDamageCopy {
        std::int32_t channel{0};
        float seconds{0.0f};
        bool add{false};
    };
    RepairAddDamageCopy queue_repair_add_damage_9e(std::int32_t channel, float seconds,
                                                   bool add) const;
    // Goal 4 chain 335. The steering-jam (6Bh, session message 107) and
    // engine-jam (6Ch, 108) failures. 00827B90 inlines 0075B430(6Bh) and
    // 007618E0's stores (+18h word 0, +1Ah 0, +4h = 1, +1Ch 0, +20h =
    // [00CF8B3C], vtable 00D03428) at 00827D09..00827D37, and calls 007619B0
    // for 6Ch at 00827D65; 008132C0 does the same at 008134C4 / 00813530.
    // Both route with 0077C2A0 flags 7. The local copy: 006D2F20 write (the
    // scalar on its 8-bit path), 00768530 case 6Bh 007618E0 (00768F86) or 6Ch
    // 007619B0 (00768FA6), 006D1550 read. 008198A0 asks the copy's IsType
    // (00761960 / 00761A30) for 6Ah, then 6Bh, then 6Ch, the first answer
    // choosing the arm; the three flags are those answers in that order.
    struct ShipJamCopy {
        std::uint8_t type{0};
        bool explosion_6a{false};
        bool steering_jam_6b{false};
        bool engine_jam_6c{false};
    };
    ShipJamCopy queue_ship_jam_6b_6c(bool steering) const;
    // Goal 4 chain 343 (kWireGoal4Chain343Bound). The ship's death messages
    // from 00827A90 (BSP_ShipInstance_OnHealthChanged, the ship's vtable
    // +1B0h): a light ship (class+B0h below 100.0) inlines 0075B430(9Ah) and
    // 007613B0's stores (+18h word 0, +1Ah 0, +1Ch 0, +4h = 1, vtable
    // 00D033D8) at 00827ADC..00827B01; a heavy one whose wreck roll passes
    // calls 00761310 itself at 00827B63 (98h, vtable 00D033C4). Both route
    // at 00827B77 with flags 7. The local copy: 004645B0 write (the shared
    // header-and-bool writer, slot 1 of both tables), 00768530 case 98h
    // 00761310 (0076952E) or case 9Ah 007613B0, 004645D0 read. The drain's
    // receiver ladder 00780120 asks the copy's IsType (00761370 / 00761410)
    // for each of its ids (00780162 vtable[0Ch](4Bh) onwards); with no
    // answer the shared tail 0077FE80 hands it to the unit's 00821E80,
    // whose 98h arm calls 00814520 (00821FBC) and 9Ah arm 00814560
    // (00821FD6). `to_unit` is that ladder's answer; `type` the copy's +10h.
    struct ShipDeathCopy {
        std::uint8_t type{0};
        bool to_unit{false};
        bool wrecked_98{false};
        bool explode_to_parts_9a{false};
    };
    ShipDeathCopy queue_ship_death_98_9a(bool explode_to_parts) const;
    // Goal 4 chain 343. 0092CED0 (the hull segment detach, called when
    // 0092D1F0 parks a segment) builds 0075B430(99h) with +18h word 0, +1Ah
    // 0, +1Ch 0, +20h = the segment index, +4h = 1, vtable 00D033EC: the
    // stores of 00761450, the constructor of 00768530's case 99h (0076954E).
    // 007614F0 write, 00761540 read; 00780120 asks 007614B0; 00821E80's 99h
    // arm (00821FF0) hands the copy's +20h to 0080E440.
    struct ExplodeOnePartCopy {
        std::uint8_t type{0};
        bool to_unit{false};
        bool explode_one_part_99{false};
        std::uint32_t part{0};
    };
    ExplodeOnePartCopy queue_ship_explode_one_part_99(std::uint32_t part) const;
    // Goal 4 chain 343. StartLanding (94h): 0064A820 makes 0075B430(94h),
    // +18h word 0, +1Ah 0, +1Ch 0, +4h = 1, vtable 00CF5C4C (the 94h row of
    // kSessionMessageClasses, the shared session_message_construct); the
    // approach landing sweep calls it at 009F3598 and routes it with
    // 0077C2A0 flags 2, which a local session turns into the local route.
    // 00768530 case 94h calls 0064A820 itself (007694AE). Slots 1 and 2 of
    // 00CF5C4C are the shared 004645B0 / 004645D0; 00780120 asks slot 3,
    // 0064A850 (session_message_is_type over the 94h row); 00821E80's 94h arm
    // (00821F61) hands the copy to the unit's vtable[238h].
    struct StartLandingCopy {
        std::uint8_t type{0};
        bool to_unit{false};
        bool start_landing_94{false};
    };
    StartLandingCopy queue_start_landing_94() const;
    // Goal 4 chain 343. SHIP_SINK (92h): the wreck's leak redistribution
    // 0074EC50 inlines 0074EAB0's stores at 0074ED18..0074ED62 (0075B430(92h),
    // +18h word 0, +4h = 1, +1Ah 0, vtable 00D00518, +1Ch = the leak count,
    // the rates copied FLD/FSTP from +20h) and routes it at 0074ED7B flags
    // 7. 00768530 case 92h allocates E8h bytes (007693F7) and calls 00762130
    // (0076940E); slots of 00D00518: 0074EB20 write (6-bit count, each rate
    // on 004295C0's raw 32-bit path, scale [00D7A248] FLT_MAX), 0074EB80
    // read, 0074EB00 IsType. 00821E80's 92h arm (0082220D) loads the copy's
    // rates through 0074E860. `count` the copy's +1Ch; the rates past
    // kShipSinkCapacity (the factory's E8h block) are not copied.
    static constexpr std::uint32_t kShipSinkCapacity = (0xE8u - 0x20u) / 4u;
    struct ShipSinkCopy {
        std::uint8_t type{0};
        bool to_unit{false};
        bool sink_92{false};
        std::uint32_t count{0};
        float rates[kShipSinkCapacity]{};
    };
    ShipSinkCopy queue_ship_sink_92(const float* rates, std::uint32_t count) const;
    // Goal 4 chain 343. VEHICLE_SHIPYARD_LAUNCH (7Ah): the takeoff task's slow
    // leave 009CFB60 inlines 0080F960's stores at 009CFBFB..009CFC23
    // (0075B430(7Ah), +18h word 0, +1Ah 0, +4h = 1, +1Ch 0, vtable 00D03310,
    // +20h = the flag byte) and routes it at 009CFC38 with 0077C2A0 flags 0.
    // 00768530 case 7Ah calls 007604C0 (00769166), whose stores are the same
    // header; slots of 00D03310: 00760550 write, 007605A0 read, 00760520
    // IsType. The plane's 7Ah arm (007CD171) hands the copy's +20h to 007B83F0.
    struct ShipyardLaunchCopy {
        std::uint8_t type{0};
        bool to_unit{false};
        bool shipyard_launch_7a{false};
        std::uint8_t flag{0};
    };
    ShipyardLaunchCopy queue_shipyard_launch_7a(std::uint8_t flag) const;
    // Goal 4 chain 356. MT_GUN_FIRE (ADh): the gun step 0072D130
    // (0072D252..0072D290) and the turning gun's extra send 0084C5B0
    // (0075B430 at 0084C5EC, routed at 0084C625, +20h = 0) inline 006E3940's
    // stores (0075B430(ADh), +18h word 0, +1Ah 0, +4h = 1, +1Ch 0, vtable
    // 00CF9628, +20h word = the target's +174h or 0) and route the message
    // with 0077C2A0 flags 1, the local bit.
    // 00768530 case ADh allocates 24h bytes (00769814) and calls
    // 006E3940(0) (0076982A); slots of 00CF9628 (bsp.py ghidra bytes): 006E47C0
    // scalar delete, 006E39B0 write (bool +1Ch, 12-bit word +20h), 006E39E0
    // read, 006E3980 IsType {ADh, 49h, 46h}, 004499C0. 00780120's ladder sends
    // the copy to the entity switch (0077FE80), whose default is the gun's
    // slot 164h: 0072D830 (the rapid / MRT overrides 0084C7E0 / 00803480
    // handle only B0h / AFh and jump to it for the rest), whose ADh arm calls
    // vtable[1DCh] FireIfReady with no argument.
    struct GunFireCopy {
        std::uint8_t type{0};
        bool to_entity{false};       // 00780120 chose the entity switch
        bool fire_if_ready{false};   // 0072D830 reached its ADh arm (vtable[1DCh])
        std::uint16_t target{0};     // the copy's +20h; the ADh arm does not read it
    };
    GunFireCopy queue_gun_fire_ad(std::uint16_t target) const;
    // Goal 4 chain 356. MT_GUN_SET_DESTROYEDFAILURE (68h) / _CLEAR (69h): the
    // gun's "destroyed" event bodies 00729FD0 / 0072A0D0 build a 24h-byte
    // message (0075B430(68h / 69h) at 0072A016 / 0072A116, +18h word 0, +1Ah
    // 0, +4h = 1, +1Ch 0, +20h = 256.0f from 00CF8B3C, vtable 00CFDE64 /
    // 00CFDE78), the stores 00728F20 / 00728FA0 make, and route it with
    // 0077C2A0 flags 7 (0072A054 / 0072A154). 00768530 case 68h / 69h
    // allocates 24h bytes and calls 00728F20 (00768F26) / 00728FA0
    // (00768F46); 006D2F20 writes and 006D1550 reads the shared scalar. The
    // gun switch 0072D830 sends 68h to vtable[1C8h] (0072B5A0) and 69h to
    // vtable[1CCh] (007280A0); each body first asks the request's IsType
    // (00728F70 / 00728FF0) for its own id (0072B5A0's 68h, 007280AC's 69h).
    struct GunDestroyedCopy {
        std::uint8_t type{0};
        bool to_entity{false};       // 00780120 chose the entity switch
        bool set_arm_68{false};      // 0072D830 -> vtable[1C8h] and IsType(68h)
        bool clear_arm_69{false};    // 0072D830 -> vtable[1CCh] and IsType(69h)
    };
    GunDestroyedCopy queue_gun_destroyed_68_69(bool set) const;
    // Goal 5 chain 641. The plane failure messages 007BBFA0 (the plane's
    // vtable[194h], StartNamedEffect) routes with 0077C2A0 flags 7: 63h built
    // inline at 007BC3EC..007BC424 (0075B430(63h), word +18h 0, byte +1Ah 0, +4h
    // 1, byte +1Ch 0, vtable 00D02F6C, float +20h = unit+C10h: 0075C380's header
    // stores with the timer in place of its -1.0f), 64h by 0075C450 itself
    // (+20h its 256.0f), 65h by 007BA170, 66h by 007BA1C0, 67h by 007BA200.
    // Flags 7 is not 4, so the local bit is taken and 0076E520 rebuilds the copy:
    // 00768530 runs the type's constructor (0075C380 / 0075C450 / 0075A150 /
    // 0075C570 / 0075A1F0) and vtable[8] reads it (006D1550 for 63h, 64h and
    // 66h; 0075C540 for 65h; 0075C660 for 67h). 006D2F20 quantises +20h: at or
    // below 0.1 it is absent and reads back as -1.0f; otherwise trunc(x * 4.0)
    // clamped to 0..255, read as n * 0.25. So 1000.0f arrives as 63.75f. The
    // plane's 007CCFA0 asks the copy's IsType for 62h (007CCFB0, the 0075C400 /
    // 0075C4D0 / 0075A1A0 / 0075C5F0 / 0075A240 sets {own, 62h, 49h, 46h}) and
    // hands it to vtable[208h], 007D0B80. Returned: the copy's fields.
    struct PlaneFailureCopy {
        std::uint8_t type{0};
        bool to_state_handler{false};  // 00780120 -> session switch -> IsType(62h)
        float value_20{0.0f};
        std::uint32_t arg_24{0};       // 65h: the engine (2 bits); 67h: the side byte
    };
    PlaneFailureCopy queue_plane_failure_63_67(std::uint8_t type, float value_20,
                                               std::uint32_t arg_24) const;
    // One message of 007BBFA0's routed list as the plane receives it: its local
    // copy (queue_plane_failure_63_67) and, when 007CCFA0 hands it on, 007D0B80
    // over the copy's kind, +20h and +24h. Returns false when the copy does not
    // reach 007D0B80. C8h (multiplayer only, session mode != 0) is refused.
    bool deliver_plane_failure_message(const gameplay_seg47_cc13::FailureMessage& routed,
        gameplay_seg47_cc13::PlaneFailureState& state,
        const gameplay_seg47_cc13::StateMessageEnv& env,
        const gameplay_seg47_cc13::StateMessageHost& host) const;
    // Goal 4 chain 356. MT_PLANE_DISAPPEAR (C7h): the plane's fixed step
    // 007CE040 at 007CE6FE..007CE74F makes 0075B430(C7h), +18h word 0, +1Ah 0,
    // +4h = 1, +1Ch 0, vtable 00D0352C (00762B20's stores) and +20h word = the
    // holder owner's id ([[[plane+BF4h]+4]+7Ch]+174h), routed with 0077C2A0
    // flags 5 (the local bit in single player). 00768530 case C7h allocates
    // 24h bytes (00769BF0) and calls 00762B20 (00769C04); slots of 00D0352C:
    // 00762BC0 scalar delete, 00762BE0 write (unit header, 12-bit +20h),
    // 00762C30 read, 00762B90 IsType, 004499C0. The plane's 007CCFA0 case C7h
    // resolves the copy's +20h through 007BA890 and hands that holder to
    // 007CC8B0. Returned: the copy's type and +20h.
    struct PlaneDisappearCopy {
        std::uint8_t type{0};
        std::uint16_t holder_id{0};
    };
    PlaneDisappearCopy queue_plane_disappear_c7(std::uint16_t holder_id) const;
    // Goal 5 chain 632. MT_VEHICLE_UNIT_LAUNCH (7Bh): ShipUseCatapult 00891D50
    // makes it inline at 00891F05..00891F31 (0075B430(7Bh), word +18h = EBP 0,
    // byte +1Ah 0, +4h = EDI 1, byte +1Ch 0, vtable 00CE53AC, float +20h = the
    // heading) and routes it with 0077C2A0(ECX = the ship, &message, EBX = 2, EBP
    // = 0) at 00891F48. Flags 2 is not 4, so with [00E188A8]+1FE4h == 0 the route
    // takes the local bit (0077C30D..0077C333) and 0076E520 rebuilds the copy.
    // Those stores are 007601A0's (the case-7Bh constructor 00768530 calls at
    // 00769186 on a 24h-byte allocation), so both messages are built with it;
    // 00CE53AC's slots are 00464FE0 scalar delete (restamps only), 004647D0
    // write, 00464810 read, 004647B0 IsType {7Bh, 49h, 46h}, 004499C0. Returned:
    // whether 00780120's ladder hands the copy to the entity's session-kind
    // switch, and the copy's 24h bytes as its read left them (what 00821E80
    // reads: +10h kind, +14h sender, +20h the payload).
    struct UnitKindSwitchCopy {
        bool to_unit{false};
        std::uint8_t bytes[0x24]{};  // read through memcpy, so no alignment is needed
    };
    UnitKindSwitchCopy queue_vehicle_unit_launch_7b(float heading) const;
    // Goal 5 chain 632. MT_VEHICLE_ADD_LAUNCHED (7Ch): the catapult's fixed step
    // 006ED1C0 calls 006EB9C0(stack message, plane) at 006ED3EF (0075B430(7Ch),
    // word +18h 0, +1Ah 0, +1Ch 0, +4h = 1, vtable 00CFAA0C, word +20h =
    // plane+174h) and routes it to the device's owner ([ESI+0E0h]) with
    // 0077C2A0 flags 7 at 006ED409, the local bit in single player. 00768530
    // case 7Ch allocates 24h bytes and calls 00760200 (007691A6); 00CFAA0C's
    // slots: 006EC100 scalar delete (restamps only), 006EBA20 write (header,
    // the +1Ch bit, +20h in 12 bits), 006EBA50 read, 006EBA00 IsType {7Ch, 49h,
    // 46h}, 004499C0. 00821E80's 7Ch arm resolves the copy's +20h (00521E30)
    // and hands a non-null entity to 00957450.
    UnitKindSwitchCopy queue_vehicle_add_launched_7c(std::uint16_t plane_id) const;

private:
    struct Image;
    std::unique_ptr<Image> image_;
};

} // namespace bsp
