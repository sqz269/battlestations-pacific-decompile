#pragma once
// Every arm of the unit's two session-message switches.
//
// Packet cc2_unit_message_arms, worker agent/cc2-unit-message-arms, 2026-09-12 UTC.
// Ghidra was read-only for this packet: no renames, comments, prototypes, function
// creation or saves. Project C:/Users/sqz269/bsp.gpr, program
// /battlestationspacific.exe. Descriptive C++ names are hypotheses, not recovered
// symbols -- EXCEPT the MT_* kind names, which are string literals in the image
// (pointer table 00E0AB68 indexed by the kind byte; see unit_message_kind_name).
//
// 00821E80 BSP_UnitInstance_HandleMessage is vtable slot 164h. Its switch is
// `kind - 4Bh`, range 0..55h, a byte index table at 00822400 (86 entries, kinds
// 4Bh..A0h) selecting one of 27 target dwords at 00822394. 0095ABE0
// BSP_Unit_HandleMessage is the base it falls back to: `kind - 4Bh`, range 0..35h,
// byte table at 0095AE40 (54 entries, kinds 4Bh..9Eh), 8 targets at 0095AE20.
// 0095ABE0's own default is not a plain `return false`: it tail-calls
// 00878350(this, msg), a third level that answers 4Bh, 4Ch, 56h and D2h.
//
// docs/UNIT_MESSAGE_ARMS.md carries the kind table, the producers and the coverage.
#include <cstddef>
#include <cstdint>

#include "bsp/unit_fire_flooding.hpp"
#include "bsp/unit_parts.hpp"

namespace bsp {

// ---------------------------------------------------------------------------
// The session message base, from its constructor 0075B430 (the producer, so
// these meanings are settled by the writer rather than by a reader).
// __thiscall(msg, kind), RET 4: vptr 00D02C68, +4h = 3, +8h = 0, +Ch = 0,
// +10h = kind, +14h = the sender player object.
// ---------------------------------------------------------------------------
inline constexpr std::size_t kSessionMessageOffKind = 0x10;    // 0075B451
inline constexpr std::size_t kSessionMessageOffStamp = 0x0C;   // 0075B444 clears; 0076E5C6 stamps
inline constexpr std::size_t kSessionMessageOffSender = 0x14;  // 0075B470 / 0075B476
inline constexpr std::size_t kSessionMessageOffPayload = 0x1C; // first field the arms read

// The constructor takes the sender from the world object [00E188A8]: index at
// world+18ECh into the eight-entry player array at world+18CCh, and stores 0 when
// the index is outside 0..7 (0075B460..0075B476).
inline constexpr std::size_t kWorldOffLocalPlayerIndex = 0x18EC;
inline constexpr std::size_t kWorldOffPlayerArray = 0x18CC;
inline constexpr int kWorldPlayerSlotCount = 8;

// 0080F710, __fastcall(msg): `msg+14h ? *(msg+14h + 20h) : 8`. Two arms use it,
// so 8 is the "no sender" control slot; 009540D0/00954170 pass the same 8 when a
// MT_ROLEOWNER release clears a station, which is what fixes the sentinel.
inline constexpr int kSessionMessageNoSenderSlot = 8;
inline constexpr std::size_t kSenderOffControlSlot = 0x20;
int sender_control_slot_0080f710(bool has_sender, int sender_slot_field) noexcept;

// ---------------------------------------------------------------------------
// Kinds. Names are the image's own MT_* literals. kUnitMessageKindFirst/Last,
// kUnitMessageKindDetachPart (99h), kUnitMessageKindBreakup (98h) and
// kUnitMessageKindDeath (9Ah) already exist in unit_parts.hpp / unit_death_sink.hpp
// and are not redeclared here.
// ---------------------------------------------------------------------------
enum class UnitMessageKind : std::uint8_t {
    RoleOwner = 0x4B,                  // MT_ROLEOWNER
    RoleAvailable = 0x4C,              // MT_ROLEAVAILABLE
    Command = 0x58,                    // MT_COMMAND
    GameUnitAttr = 0x5A,               // MT_GAMEUNIT_ATTR
    GameUnitSetFireTarget = 0x5E,      // MT_GAMEUNIT_SETFIRETARGET
    ShipSetExplosionFailure = 0x6A,    // MT_SHIP_SET_EXPLOSIONFAILURE
    ShipSetSteeringJamFailure = 0x6B,  // MT_SHIP_SET_STEERINGJAMFAILURE
    ShipSetEngineJamFailure = 0x6C,    // MT_SHIP_SET_ENGINEJAMFAILURE
    ShipClearSteeringJamFailure = 0x6D,// MT_SHIP_CLEAR_STEERINGJAMFAILURE
    ShipClearEngineJamFailure = 0x6E,  // MT_SHIP_CLEAR_ENGINEJAMFAILURE
    ShipRepairSettings = 0x6F,         // MT_SHIP_REPAIR_SETTINGS
    ShipKamikazeDetonate = 0x70,       // MT_SHIP_KAMIKAZE_DETONATE
    VehicleGunControl = 0x79,          // MT_VEHICLE_GUN_CONTROL
    VehicleShipyardLaunch = 0x7A,      // MT_VEHICLE_SHIPYARD_LAUNCH
    VehicleUnitLaunch = 0x7B,          // MT_VEHICLE_UNIT_LAUNCH
    VehicleAddLaunched = 0x7C,         // MT_VEHICLE_ADD_LAUNCHED
    VehicleSetInferiorFailure = 0x7D,  // MT_VEHICLE_SET_INFERIORFAILURE
    VehicleClearInferiorFailure = 0x7E,// MT_VEHICLE_CLEAR_INFERIORFAILURE
    VehicleSetParty = 0x7F,            // MT_VEHICLE_SET_PARTY
    VehicleSetRace = 0x80,             // MT_VEHICLE_SET_RACE
    ShipSync = 0x8C,                   // MT_SHIP_SYNC (slot 18Ch, not 164h)
    ShipHelmsmanControl = 0x8E,        // MT_SHIP_HELMSMAN_CONTROL
    ShipAvoidSide = 0x8F,              // MT_SHIP_AVOIDSIDE
    ShipLeak = 0x90,                   // MT_SHIP_LEAK
    ShipLeakCheatWater = 0x91,         // MT_SHIP_LEAKCHEATWATER
    ShipSink = 0x92,                   // MT_SHIP_SINK
    ShipAddTorque = 0x93,              // MT_SHIP_ADDTORQUE
    ShipStartLanding = 0x94,           // MT_SHIP_STARTLANDING
    ShipLandingShipsLaunched = 0x95,   // MT_SHIP_LANDINGSHIPSLAUNCHED
    ShipSetTorpedoStock = 0x96,        // MT_SHIP_SET_TORPEDOSTOCK
    EntityRelocate = 0x97,             // MT_ENTITY_RELOCATE
    ShipWrecked = 0x98,                // MT_SHIP_WRECKED
    ShipExplodeOnePart = 0x99,         // MT_SHIP_EXPLODEONEPART
    ShipExplodeToParts = 0x9A,         // MT_SHIP_EXPLODETOPARTS
    ShipSectionFailureEfx = 0x9B,      // MT_SHIP_SECTIONFAILUREEFX
    ShipEraseWreck = 0x9C,             // MT_SHIP_ERASEWRECK
    ShipTorpedoGenerateHack = 0x9D,    // MT_SHIP_TORPEDOGENERATE_HACK
    ShipRepairAddDamage = 0x9E,        // MT_SHIPREPAIR_ADDDAMAGE
    ShipRepairBodyRepair = 0x9F,       // MT_SHIPREPAIR_BODYREPAIR
    ShipRepairRepairPriority = 0xA0,   // MT_SHIPREPAIR_REPAIRPRIORITY
};

// The pointer table at 00E0AB68, one char* per kind byte. Returns nullptr outside
// the decoded window 4Ah..A1h; the table itself is larger (kinds below 4Ah and up
// to at least A1h MT_SUBMARINE_DIP_RISE exist) but only this window was read.
inline constexpr std::uint32_t kMessageKindNameTableAddress = 0x00E0AB68u;
const char* unit_message_kind_name(std::uint8_t kind) noexcept;

// ---------------------------------------------------------------------------
// The 27 arms of 00821E80, named by what the arm does.
// ---------------------------------------------------------------------------
enum class UnitMessageArm {
    BaseCallForcedTrue,     // 00821EEA, target 00h: call the base, then MOV AL,1
    IgnoredReturnTrue,      // 00821ED5, target 01h: the shared epilogue, no work
    SetFailureVirtual,      // 00822120, target 02h: vtable[220h](msg)
    ClearFailureVirtual,    // 00822140, target 03h: vtable[224h](msg)
    ApplyRepairSettings,    // 00822160, target 04h: msg->0080DC10(unit)
    KamikazeDetonate,       // 0082217D, target 05h: 00819A20
    ShipyardLaunchState,    // 0082226F, target 06h: unit+1130h = 5 or 0
    UnitLaunchVirtual,      // 00821F05, target 07h: vtable[208h](slot, float)
    AddLaunchedChild,       // 00821F37, target 08h: 00957450
    HelmsmanControl,        // 00821EBE, target 09h: gated 008141A0
    AvoidSide,              // 00822294, target 0Ah: 00815010 then unit+740h vtable[28h]
    AddLeak,                // 008221A7, target 0Bh: 0074F440
    ClearAllLeakZones,      // 008221ED, target 0Ch: 0074E830
    LoadLeakZoneArray,      // 0082220D, target 0Dh: 0074E860
    AddHullTorque,          // 00822235, target 0Eh: 0092BF30
    StartLandingVirtual,    // 00821F61, target 0Fh: vtable[238h]()
    LandingShipsLaunched,   // 00821F80, target 10h: unit+1124h = float
    SetTorpedoStock,        // 00821F9E, target 11h: 0081F8B0
    Wrecked,                // 00821FBC, target 12h: 00814520
    DetachPart,             // 00821FF0, target 13h: 0080E440
    ExplodeToParts,         // 00821FD6, target 14h: 00814560
    SectionFailureEffect,   // 00822010, target 15h: 0093AA90
    SpawnTorpedo,           // 008222CF, target 16h: class 3Fh spawn
    RepairAddDamage,        // 0082203D, target 17h: the fire/leak channel switch
    SetHullRepairEnabled,   // 008220D7, target 18h: 00939FD0
    SetRepairPriority,      // 008220FC, target 19h: 0064A270
    BaseCallPassThrough,    // 0082237B, target 1Ah: the default, returns the base's value
};

// The 8 arms of 0095ABE0.
enum class BaseUnitMessageArm {
    RoleOwner,            // 0095AC1D, target 0h
    IgnoredReturnTrue,    // 0095ACD1, target 1h
    GunControl,           // 0095ACE5, target 2h: 00959C20
    SetInferiorFailure,   // 0095AD01, target 3h
    ClearInferiorFailure, // 0095AD7C, target 4h
    SetParty,             // 0095ADA2, target 5h
    SetRace,              // 0095ADD4, target 6h
    EntityFallback,       // 0095AE06, target 7h: 00878350
};

inline constexpr std::uint8_t kUnitSwitchKindLimit = 0x55;  // CMP EAX,55h at 00821EA4
inline constexpr std::uint8_t kBaseSwitchKindLimit = 0x35;  // CMP ECX,35h at 0095AC06
inline constexpr std::uint32_t kUnitArmByteTableAddress = 0x00822400u;
inline constexpr std::uint32_t kUnitArmTargetTableAddress = 0x00822394u;
inline constexpr std::uint32_t kBaseArmByteTableAddress = 0x0095AE40u;
inline constexpr std::uint32_t kBaseArmTargetTableAddress = 0x0095AE20u;

// Both switches, decoded from their byte tables. Out-of-range kinds take the
// default arm, exactly as the JA at 00821EAA and 0095AC09 do.
UnitMessageArm unit_message_arm_00821e80(std::uint8_t kind) noexcept;
BaseUnitMessageArm base_message_arm_0095abe0(std::uint8_t kind) noexcept;
std::uint32_t unit_message_arm_address(UnitMessageArm arm) noexcept;
std::uint32_t base_message_arm_address(BaseUnitMessageArm arm) noexcept;

// 00878350, the third level: 4Bh and 4Ch are answered with 1 and no work, 56h
// runs entity vtable[1B4h](msg+20h), D2h runs 00877CD0(msg+1Ch), everything else
// returns 0. This is what a caller of vtable[164h] actually sees for an unhandled
// kind, and the only place in the chain that returns false.
bool entity_message_fallback_00878350(std::uint8_t kind) noexcept;

// ---------------------------------------------------------------------------
// Where a kind comes from. A kind with a Lua, HUD, AI or engine producer can be
// built by the executable in a single-player mission; NetworkOnly means no local
// producer was found by the image scan, so it arrives only over the wire.
// ---------------------------------------------------------------------------
enum class UnitMessageProducer {
    LuaBinding,     // a luaMW_* binding builds it
    HudOrInterface, // a HUD / interface screen builds it
    ScriptOrAi,     // an AI routine, task or tick builds it
    EngineInternal, // engine code (dispatcher, factory, sim) builds it
    NetworkOnly,    // no local producer found
    Unresolved,     // a producer exists but the kind immediate is computed
};
UnitMessageProducer unit_message_producer(std::uint8_t kind) noexcept;
bool unit_message_has_local_producer(std::uint8_t kind) noexcept;

// ---------------------------------------------------------------------------
// Pure rules the arms compute.
// ---------------------------------------------------------------------------

// 9Eh MT_SHIPREPAIR_ADDDAMAGE, arm 0082203D. msg+24h picks the channel and
// byte msg+20h picks add over set. The float is msg+1Ch, in seconds.
// Channel 0 is fire (repair task +38h) and channel 1 is leak/water (+34h): the
// adder for +38h calls 00983780, whose body carries the literal "fire", and the
// adder for +34h calls 009832F0, whose body carries "leak".
enum class RepairDamageChannel : int { Fire = 0, Water = 1 };
enum class RepairDamageRoute {
    SetFireSeconds,    // 00939F90, writes task+38h
    AddFireSeconds,    // 0093A470, accumulates task+38h
    SetWaterSeconds,   // 00939FA0, writes task+34h
    AddWaterSeconds,   // 0093A4F0, accumulates task+34h
    Ignored,           // msg+24h > 1: the arm falls into the epilogue
};
RepairDamageRoute repair_damage_route_0082203d(int channel, bool accumulate) noexcept;

// 7Ah MT_VEHICLE_SHIPYARD_LAUNCH, arm 0082226F. NEG CL / SBB ECX,ECX / AND ECX,5
// is a branchless `byte ? 5 : 0` stored at unit+1130h.
inline constexpr std::size_t kUnitOffShipyardLaunchState = 0x1130;
inline constexpr int kShipyardLaunchStateActive = 5;
int shipyard_launch_state_0082226f(bool requested) noexcept;

// 95h MT_SHIP_LANDINGSHIPSLAUNCHED, arm 00821F80: a straight float store.
inline constexpr std::size_t kUnitOffLandingShipsLaunched = 0x1124;

// 8Eh MT_SHIP_HELMSMAN_CONTROL, arm 00821EBE then 008141A0. The arm only applies
// the order when the message's sender holds the unit's helm station, and 008141A0
// back-dates it by the message's age in ticks, clamped to the global at 00E0B51C.
inline constexpr std::size_t kUnitOffHelmStation = 0x1B0;
inline constexpr std::uint32_t kHelmsmanBackfillClampAddress = 0x00E0B51Cu;
bool helmsman_message_accepted_00821ebe(int unit_helm_station, int sender_slot) noexcept;
int helmsman_backfill_age_008141a0(int current_tick, int message_stamp, int clamp) noexcept;

// 90h MT_SHIP_LEAK, arm 008221A7. FILD of the dword at msg+1Ch with the
// `if (signed < 0) add 2^32` fixup at 008221B5 is an unsigned-to-float widen; the
// double at 00CE3DC0 is 10.0.
inline constexpr double kLeakAmountScale = 10.0;
double leak_amount_008221a7(std::uint32_t raw) noexcept;

// 6Fh MT_SHIP_REPAIR_SETTINGS, arm 00822160 then 0080DC10: six dwords from
// msg+20h..msg+34h into unit+1134h.., which unit_fire_flooding.hpp already names
// kUnitRepairLevelArrayOffset.
inline constexpr int kRepairSettingsWordCount = 6;

// 4Bh MT_ROLEOWNER, arm 0095AC1D. byte msg+20h takes the single-device path;
// otherwise msg+24h is a device-group mask, msg+28h the slot being assigned and
// msg+2Ch 1 for take / 0 for release.
enum class RoleOwnerRoute {
    SingleDeviceVirtual, // this->vtable[144h](msg+28h)
    // 00953F60 and 00954000 walk the unit's device list at this+48h (next at
    // +44h) and filter on the device descriptor's +80h kind word. The two sets
    // are 1/5/6 and 2/3/4/6; which device classes those are was not read, so
    // these are set labels and not device names.
    DeviceKindSet156,    // 00953F60, flags & 4 or flags == 8
    DeviceKindSet2346,   // 00954000, flags & 10h
    GlobalCollectionA,   // 009540D0, flags & 20h, collection 00E0A520
    GlobalCollectionB,   // 00954170, flags & 80h, collection 00E0A528
    NoGroup,             // no mask bit matched
};
RoleOwnerRoute role_owner_route_0095ac1d(bool single_device, int flags) noexcept;
inline constexpr std::size_t kUnitOffRoleOwnerLatch = 0x6E8;  // 0095AC53, msg+24h bit 0 and msg+2Ch == 1

// ---------------------------------------------------------------------------
// Integration boundary. One method per native call site of the two switches, in
// arm order. There are no default implementations: nothing here stands in for
// unrecovered game behaviour.
// ---------------------------------------------------------------------------
struct UnitMessageArmHost {
    virtual ~UnitMessageArmHost() = default;

    // --- message reads -----------------------------------------------------
    virtual std::uint8_t message_kind() = 0;              // msg+10h
    virtual int message_stamp() = 0;                      // msg+0Ch
    virtual bool message_has_sender() = 0;                // msg+14h != 0
    virtual int message_sender_control_slot() = 0;        // 0080F710 -> sender+20h
    virtual std::uint32_t message_dword(std::size_t off) = 0;
    virtual float message_float(std::size_t off) = 0;
    virtual std::uint8_t message_byte(std::size_t off) = 0;
    virtual const void* message_payload(std::size_t off) = 0; // LEA of msg+off

    // --- 00821E80 arms -----------------------------------------------------
    virtual bool base_handle_message() = 0;               // 00821EEB / 0082237C -> 0095ABE0
    virtual void set_failure_virtual() = 0;               // 00822129, unit vtable[220h]
    virtual void clear_failure_virtual() = 0;             // 00822149, unit vtable[224h]
    virtual void apply_repair_settings() = 0;             // 00822163 -> 0080DC10
    virtual void* resolve_entity(std::uint16_t id) = 0;   // 00521E30
    virtual void kamikaze_detonate(void* target) = 0;     // 00822189 -> 00819A20
    virtual void set_shipyard_launch_state(int state) = 0;// 0082227B store
    virtual void unit_launch_virtual(int slot, float value) = 0; // 00821F1F, vtable[208h]
    virtual void add_launched_child(void* entity) = 0;    // 00821F47 -> 00957450
    virtual int unit_helm_station() = 0;                  // unit+1B0h
    virtual void backfill_helm_order() = 0;               // 00821ED0 -> 008141A0
    virtual void* checked_cast_class6(void* entity) = 0;  // 0082229F -> 00815010
    virtual bool has_avoidance_helper() = 0;              // unit+740h != 0
    virtual void avoidance_set_side(void* target, std::uint32_t value) = 0; // 008222B8, vtable[28h]
    virtual void add_leak(double amount, const void* point) = 0; // 008221D3 -> 0074F440
    virtual void clear_all_leak_zones() = 0;              // 008221F3 -> 0074E830
    virtual void load_leak_zone_array(int count, const void* src) = 0; // 0082221B -> 0074E860
    virtual void add_hull_torque(const float* xyz) = 0;   // 00822255 -> 0092BF30
    virtual void start_landing_virtual() = 0;             // 00821F69, vtable[238h]
    virtual void set_landing_ships_launched(float value) = 0; // 00821F85 store
    virtual void set_torpedo_stock(std::uint32_t stock) = 0; // 00821FA2 -> 0081F8B0
    virtual void on_wrecked() = 0;                        // 00821FBC -> 00814520
    virtual void detach_part(std::uint32_t part) = 0;     // 00821FF6 -> 0080E440
    virtual void on_death() = 0;                          // 00821FD6 -> 00814560
    virtual void set_section_failure_effect(std::uint32_t section, std::uint32_t a,
                                            bool on) = 0; // 00822023 -> 0093AA90
    virtual void spawn_torpedo(std::uint32_t type, const void* position,
                               float heading) = 0;        // 008222CF arm
    virtual void repair_route(RepairDamageRoute route, float seconds) = 0; // 0082203D arm
    virtual void set_hull_repair_enabled(bool enabled) = 0; // 008220E2 -> 00939FD0
    virtual void set_repair_priority(std::uint32_t priority) = 0; // 00822106 -> 0064A270

    // --- 0095ABE0 arms -----------------------------------------------------
    virtual void role_owner_single_device(std::uint32_t slot) = 0; // 0095AC31, vtable[144h]
    virtual void role_owner_group(RoleOwnerRoute route) = 0;       // 0095AC6F/AC90/ACB0/ACCC
    virtual void set_role_owner_latch() = 0;                       // 0095AC53 store
    virtual void apply_gun_aim_message() = 0;                      // 0095ACE8 -> 00959C20
    virtual bool is_kind_of(int class_id) = 0;                     // 0095AD11/AD20, vtable[5Ch]
    virtual float inferior_failure_seconds() = 0;                  // this+728h, 0095AD34
    virtual void raise_inferior_failure_warning(float seconds) = 0;// 0095AD52 -> 00982C50
    virtual void set_inferior_failure(bool active) = 0;            // 0095AD0A / 0095AD7F stores
    virtual void set_party_and_race(std::uint32_t party,
                                    std::uint32_t race) = 0;       // 0095ADBE/ADF0, vtable[2Ch]
    virtual std::uint32_t current_party() = 0;                     // this+54h
    virtual std::uint32_t current_race() = 0;                      // this+58h
    virtual bool entity_fallback() = 0;                            // 0095AE09 -> 00878350
};

// 00821E80 BSP_UnitInstance_HandleMessage, undefined1 __thiscall(unit, msg),
// RET 4. Every arm except 0082237B returns true; 0082237B returns whatever
// 0095ABE0 returned.
bool dispatch_unit_message_00821e80(UnitMessageArmHost& host);

// 0095ABE0 BSP_Unit_HandleMessage, undefined1 __thiscall(unit, msg), RET 4.
// Every arm except 0095AE06 returns true; 0095AE06 returns 00878350's result.
bool dispatch_base_message_0095abe0(UnitMessageArmHost& host);

// Inferior-failure class gate at 0095AD01: the warning is raised only when the
// unit is class 45h or class 46h, and the latch at this+720h is set either way.
inline constexpr int kInferiorFailureClassA = 0x45;
inline constexpr int kInferiorFailureClassB = 0x46;
inline constexpr std::size_t kUnitOffInferiorFailure = 0x720;        // 0095AD0A, 0095AD7F
inline constexpr std::size_t kUnitOffInferiorFailureSeconds = 0x728; // 0095AD34, 0095AD86

} // namespace bsp
