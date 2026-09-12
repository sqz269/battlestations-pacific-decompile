// Packet cc2_unit_message_arms. See include/bsp/unit_message_arms.hpp and
// docs/UNIT_MESSAGE_ARMS.md. Ghidra was read-only for this packet.
#include "bsp/unit_message_arms.hpp"

#include <cstring>

namespace bsp {
namespace {

// The byte index table at 00822400, verbatim: 86 entries for kinds 4Bh..A0h,
// each selecting one of the 27 target dwords at 00822394.
constexpr std::uint8_t kUnitArmIndex[] = {
    0x00, 0x01, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a,
    0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a,
    0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x02, 0x02, 0x02, 0x03, 0x03,
    0x04, 0x05, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x06,
    0x07, 0x08, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a,
    0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x1a, 0x09, 0x0a, 0x0b, 0x0c, 0x0d,
    0x0e, 0x0f, 0x10, 0x11, 0x1a, 0x12, 0x13, 0x14, 0x15, 0x1a, 0x16, 0x17,
    0x18, 0x19,
};
static_assert(sizeof(kUnitArmIndex) == 0x56, "86 entries, kinds 4Bh..A0h");

// The 27 target dwords at 00822394, in index order.
constexpr std::uint32_t kUnitArmTarget[] = {
    0x00821eea, 0x00821ed5, 0x00822120, 0x00822140, 0x00822160, 0x0082217d,
    0x0082226f, 0x00821f05, 0x00821f37, 0x00821ebe, 0x00822294, 0x008221a7,
    0x008221ed, 0x0082220d, 0x00822235, 0x00821f61, 0x00821f80, 0x00821f9e,
    0x00821fbc, 0x00821ff0, 0x00821fd6, 0x00822010, 0x008222cf, 0x0082203d,
    0x008220d7, 0x008220fc, 0x0082237b,
};
static_assert(sizeof(kUnitArmTarget) / sizeof(kUnitArmTarget[0]) == 27, "27 targets");

// The byte index table at 0095AE40: 54 entries for kinds 4Bh..9Eh.
constexpr std::uint8_t kBaseArmIndex[] = {
    0x00, 0x01, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07,
    0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x07, 0x02, 0x07,
    0x07, 0x07, 0x03, 0x04, 0x05, 0x06,
};
static_assert(sizeof(kBaseArmIndex) == 0x36, "54 entries, kinds 4Bh..9Eh");

// The 8 target dwords at 0095AE20.
constexpr std::uint32_t kBaseArmTarget[] = {
    0x0095ac1d, 0x0095acd1, 0x0095ace5, 0x0095ad01,
    0x0095ad7c, 0x0095ada2, 0x0095add4, 0x0095ae06,
};

// The MT_* literals, from the pointer table at 00E0AB68 indexed by the kind
// byte. Read window 4Ah..A1h; these are image strings, not hypotheses.
struct KindName {
    std::uint8_t kind;
    const char* name;
};
constexpr KindName kKindNames[] = {
    {0x4a, "MT_CONTROL"},
    {0x4b, "MT_ROLEOWNER"},
    {0x4c, "MT_ROLEAVAILABLE"},
    {0x4d, "MT_MULTISELECTUNIT"},
    {0x4e, "MT_DEADMEAT"},
    {0x4f, "MT_DESTROY"},
    {0x50, "MT_UNITEXITED"},
    {0x51, "MT_UNITSOLD"},
    {0x52, "MT_ACTIVATE"},
    {0x53, "MT_GIVEUNIT"},
    {0x54, "MT_SETGUINAME"},
    {0x55, "MT_CAMERA"},
    {0x56, "MT_DAMAGEDGFXLEVEL"},
    {0x57, "MT_RESUME_AI_CONTROL"},
    {0x58, "MT_COMMAND"},
    {0x59, "MT_GAMEUNITMSG"},
    {0x5a, "MT_GAMEUNIT_ATTR"},
    {0x5b, "MT_GAMEUNIT_MOVEONPATH"},
    {0x5c, "MT_GAMEUNIT_SETCMD"},
    {0x5d, "MT_GAMEUNIT_CLEARCMD"},
    {0x5e, "MT_GAMEUNIT_SETFIRETARGET"},
    {0x5f, "MT_GAMEUNIT_ADDUSERPATHPOINT"},
    {0x60, "MT_GAMEUNIT_SETCURPATHINDEX"},
    {0x61, "MT_RECONLEVEL"},
    {0x62, "MT_FAILURE"},
    {0x63, "MT_EXPLOSIONFAILURE"},
    {0x64, "MT_SPLASHFAILURE"},
    {0x65, "MT_ENGINEFIREFAILURE"},
    {0x66, "MT_ENGINESLOSTFAILURE"},
    {0x67, "MT_SPINNINGFAILURE"},
    {0x68, "MT_GUN_SET_DESTROYEDFAILURE"},
    {0x69, "MT_GUN_CLEAR_DESTROYEDFAILURE"},
    {0x6a, "MT_SHIP_SET_EXPLOSIONFAILURE"},
    {0x6b, "MT_SHIP_SET_STEERINGJAMFAILURE"},
    {0x6c, "MT_SHIP_SET_ENGINEJAMFAILURE"},
    {0x6d, "MT_SHIP_CLEAR_STEERINGJAMFAILURE"},
    {0x6e, "MT_SHIP_CLEAR_ENGINEJAMFAILURE"},
    {0x6f, "MT_SHIP_REPAIR_SETTINGS"},
    {0x70, "MT_SHIP_KAMIKAZE_DETONATE"},
    {0x71, "MT_PARATROOPER_LANDINGPOS"},
    {0x72, "MT_AIRBASE_SET_RUNWAYFAILURE"},
    {0x73, "MT_AIRBASE_CLEAR_RUNWAYFAILURE"},
    {0x74, "MT_AIRBASE_SET_HANGARFAILURE"},
    {0x75, "MT_AIRBASE_CLEAR_HANGARFAILURE"},
    {0x76, "MT_FORMATION_JOIN"},
    {0x77, "MT_FORMATION_LEAVE"},
    {0x78, "MT_FORMATION_SET"},
    {0x79, "MT_VEHICLE_GUN_CONTROL"},
    {0x7a, "MT_VEHICLE_SHIPYARD_LAUNCH"},
    {0x7b, "MT_VEHICLE_UNIT_LAUNCH"},
    {0x7c, "MT_VEHICLE_ADD_LAUNCHED"},
    {0x7d, "MT_VEHICLE_SET_INFERIORFAILURE"},
    {0x7e, "MT_VEHICLE_CLEAR_INFERIORFAILURE"},
    {0x7f, "MT_VEHICLE_SET_PARTY"},
    {0x80, "MT_VEHICLE_SET_RACE"},
    {0x81, "MT_AIRBASE_SETSLOT"},
    {0x82, "MT_AIRBASE_CHANGESLOTSTATUS"},
    {0x83, "MT_AIRBASE_LAUNCHSLOT"},
    {0x84, "MT_AIRBASE_SLOTSTATUSSYNC"},
    {0x85, "MT_AIRBASE_STOCKITEM"},
    {0x86, "MT_AIRBASE_APPEARREADYPLANE"},
    {0x87, "MT_MOTHERSHIP_ELEVATOR_MOVE"},
    {0x88, "MT_AIRBASE_SETORDERS"},
    {0x89, "MT_AIRBASE_DOLAUNCHSLOT"},
    {0x8a, "MT_SHIP_CREATE"},
    {0x8b, "MT_MOTHERSHIP_CREATE"},
    {0x8c, "MT_SHIP_SYNC"},
    {0x8d, "MT_SHIP_GUNS_SYNC"},
    {0x8e, "MT_SHIP_HELMSMAN_CONTROL"},
    {0x8f, "MT_SHIP_AVOIDSIDE"},
    {0x90, "MT_SHIP_LEAK"},
    {0x91, "MT_SHIP_LEAKCHEATWATER"},
    {0x92, "MT_SHIP_SINK"},
    {0x93, "MT_SHIP_ADDTORQUE"},
    {0x94, "MT_SHIP_STARTLANDING"},
    {0x95, "MT_SHIP_LANDINGSHIPSLAUNCHED"},
    {0x96, "MT_SHIP_SET_TORPEDOSTOCK"},
    {0x97, "MT_ENTITY_RELOCATE"},
    {0x98, "MT_SHIP_WRECKED"},
    {0x99, "MT_SHIP_EXPLODEONEPART"},
    {0x9a, "MT_SHIP_EXPLODETOPARTS"},
    {0x9b, "MT_SHIP_SECTIONFAILUREEFX"},
    {0x9c, "MT_SHIP_ERASEWRECK"},
    {0x9d, "MT_SHIP_TORPEDOGENERATE_HACK"},
    {0x9e, "MT_SHIPREPAIR_ADDDAMAGE"},
    {0x9f, "MT_SHIPREPAIR_BODYREPAIR"},
    {0xa0, "MT_SHIPREPAIR_REPAIRPRIORITY"},
    {0xa1, "MT_SUBMARINE_DIP_RISE"},
};

struct ProducerRow {
    std::uint8_t kind;
    UnitMessageProducer producer;
};
// One row per kind the chain arms. See docs/UNIT_MESSAGE_ARMS.md for the
// producing function of each, found by scanning the image for `PUSH <kind>`
// immediately before a CALL to 0075B430.
constexpr ProducerRow kProducers[] = {
    {0x4b, UnitMessageProducer::LuaBinding},      // 008A6060 luaMW_EnterPlayerToRole
    {0x4c, UnitMessageProducer::LuaBinding},      // 008AB850
    {0x58, UnitMessageProducer::EngineInternal},  // 007798D0
    {0x5a, UnitMessageProducer::ScriptOrAi},      // 0071DD30 and eight more
    {0x5e, UnitMessageProducer::ScriptOrAi},      // 00835740
    {0x6a, UnitMessageProducer::LuaBinding},      // 008132C0
    {0x6b, UnitMessageProducer::LuaBinding},      // 008132C0
    {0x6c, UnitMessageProducer::Unresolved},      // no immediate site
    {0x6d, UnitMessageProducer::EngineInternal},  // 00813660
    {0x6e, UnitMessageProducer::EngineInternal},  // 00813660
    {0x6f, UnitMessageProducer::EngineInternal},  // 0080FEC0
    {0x70, UnitMessageProducer::EngineInternal},  // 0080FF30
    {0x79, UnitMessageProducer::EngineInternal},  // 00954A10
    {0x7a, UnitMessageProducer::EngineInternal},  // 00813830, 009CFBA4
    {0x7b, UnitMessageProducer::LuaBinding},      // 00891D50, also HUD 00651800
    {0x7c, UnitMessageProducer::EngineInternal},  // 006EB9C0
    {0x7d, UnitMessageProducer::EngineInternal},  // 00953DC4
    {0x7e, UnitMessageProducer::EngineInternal},  // 00953EC4
    {0x7f, UnitMessageProducer::LuaBinding},      // 008A8930
    {0x80, UnitMessageProducer::LuaBinding},      // 008A8720
    {0x8c, UnitMessageProducer::EngineInternal},  // 00813CA0
    {0x8e, UnitMessageProducer::ScriptOrAi},      // 00816A40
    {0x8f, UnitMessageProducer::EngineInternal},  // 009D66B0, trigger unread
    {0x90, UnitMessageProducer::EngineInternal},  // 0080FF80, 00826F10
    {0x91, UnitMessageProducer::EngineInternal},  // 008144A0, unit vtable slot 23Ch
    {0x92, UnitMessageProducer::EngineInternal},  // 0074EC50
    {0x93, UnitMessageProducer::EngineInternal},  // 0080FFD0
    {0x94, UnitMessageProducer::ScriptOrAi},      // 0064A820, 009F3240
    {0x95, UnitMessageProducer::EngineInternal},  // 008206F0
    {0x96, UnitMessageProducer::EngineInternal},  // 0080FE10
    {0x98, UnitMessageProducer::Unresolved},      // no immediate site
    {0x99, UnitMessageProducer::EngineInternal},  // 0092CED0
    {0x9a, UnitMessageProducer::EngineInternal},  // 00827A90
    {0x9b, UnitMessageProducer::ScriptOrAi},      // 0093A2C0, 0093C520
    {0x9d, UnitMessageProducer::Unresolved},      // 009D6650, no reference resolves in
    {0x9e, UnitMessageProducer::LuaBinding},      // 0088E320, 0088E790
    {0x9f, UnitMessageProducer::LuaBinding},      // 008AD330
    {0xa0, UnitMessageProducer::LuaBinding},      // 008AD6F0, also HUD 0064A770
};

} // namespace

int sender_control_slot_0080f710(bool has_sender, int sender_slot_field) noexcept {
    return has_sender ? sender_slot_field : kSessionMessageNoSenderSlot;
}

const char* unit_message_kind_name(std::uint8_t kind) noexcept {
    for (const KindName& row : kKindNames) {
        if (row.kind == kind) {
            return row.name;
        }
    }
    return nullptr;
}

UnitMessageArm unit_message_arm_00821e80(std::uint8_t kind) noexcept {
    // MOVZX EAX,[msg+10h]; ADD EAX,-4Bh; CMP EAX,55h; JA 0082237B.
    const int index = static_cast<int>(kind) - kUnitMessageKindFirst;
    if (index < 0 || index > kUnitSwitchKindLimit) {
        return UnitMessageArm::BaseCallPassThrough;
    }
    return static_cast<UnitMessageArm>(kUnitArmIndex[index]);
}

BaseUnitMessageArm base_message_arm_0095abe0(std::uint8_t kind) noexcept {
    // MOVZX ECX,[msg+10h]; ADD ECX,-4Bh; CMP ECX,35h; JA 0095AE06.
    const int index = static_cast<int>(kind) - kUnitMessageKindFirst;
    if (index < 0 || index > kBaseSwitchKindLimit) {
        return BaseUnitMessageArm::EntityFallback;
    }
    return static_cast<BaseUnitMessageArm>(kBaseArmIndex[index]);
}

std::uint32_t unit_message_arm_address(UnitMessageArm arm) noexcept {
    return kUnitArmTarget[static_cast<int>(arm)];
}

std::uint32_t base_message_arm_address(BaseUnitMessageArm arm) noexcept {
    return kBaseArmTarget[static_cast<int>(arm)];
}

bool entity_message_fallback_00878350(std::uint8_t kind) noexcept {
    switch (kind) {
    case 0x4b:
    case 0x4c:
    case 0x56: // MT_DAMAGEDGFXLEVEL, entity vtable[1B4h](msg+20h)
    case 0xd2: // 00877CD0(msg+1Ch)
        return true;
    default:
        return false;
    }
}

UnitMessageProducer unit_message_producer(std::uint8_t kind) noexcept {
    for (const ProducerRow& row : kProducers) {
        if (row.kind == kind) {
            return row.producer;
        }
    }
    return UnitMessageProducer::NetworkOnly;
}

bool unit_message_has_local_producer(std::uint8_t kind) noexcept {
    switch (unit_message_producer(kind)) {
    case UnitMessageProducer::LuaBinding:
    case UnitMessageProducer::HudOrInterface:
    case UnitMessageProducer::ScriptOrAi:
    case UnitMessageProducer::EngineInternal:
        return true;
    case UnitMessageProducer::NetworkOnly:
    case UnitMessageProducer::Unresolved:
        return false;
    }
    return false;
}

RepairDamageRoute repair_damage_route_0082203d(int channel, bool accumulate) noexcept {
    // 0082203D: SUB EAX,0 / JZ 00822090 is channel 0; SUB EAX,1 / JNZ 008220C2
    // drops every channel above 1 into the plain return-true epilogue. In both
    // surviving arms EAX is 0, so CMP byte [msg+20h],AL tests against zero and a
    // non-zero byte selects the accumulating setter.
    if (channel == 0) {
        return accumulate ? RepairDamageRoute::AddFireSeconds
                          : RepairDamageRoute::SetFireSeconds;
    }
    if (channel == 1) {
        return accumulate ? RepairDamageRoute::AddWaterSeconds
                          : RepairDamageRoute::SetWaterSeconds;
    }
    return RepairDamageRoute::Ignored;
}

int shipyard_launch_state_0082226f(bool requested) noexcept {
    return requested ? kShipyardLaunchStateActive : 0;
}

bool helmsman_message_accepted_00821ebe(int unit_helm_station, int sender_slot) noexcept {
    // 00821EC5 CMP [EDI+1B0h],EAX / JNZ 00821ED5: unequal skips the apply and
    // still returns true, so a rejected helm message is silently dropped.
    return unit_helm_station == sender_slot;
}

int helmsman_backfill_age_008141a0(int current_tick, int message_stamp, int clamp) noexcept {
    const int age = current_tick - message_stamp;
    return age > clamp ? clamp : age;
}

double leak_amount_008221a7(std::uint32_t raw) noexcept {
    return static_cast<double>(raw) * kLeakAmountScale;
}

RoleOwnerRoute role_owner_route_0095ac1d(bool single_device, int flags) noexcept {
    // 0095AC1D CMP byte [msg+20h],0 / JZ 0095AC47 takes the single-device path
    // first; the mask tests then run in listing order and the first match wins.
    if (single_device) {
        return RoleOwnerRoute::SingleDeviceVirtual;
    }
    if ((flags & 0x04) != 0 || flags == 0x08) { // 0095AC5D, 0095AC62
        return RoleOwnerRoute::DeviceKindSet156;
    }
    if ((flags & 0x10) != 0) { // 0095AC67
        return RoleOwnerRoute::DeviceKindSet2346;
    }
    if ((flags & 0x20) != 0) { // 0095AC88
        return RoleOwnerRoute::GlobalCollectionA;
    }
    if ((flags & 0x80) != 0) { // 0095ACA9 TEST CL,CL / JNS
        return RoleOwnerRoute::GlobalCollectionB;
    }
    return RoleOwnerRoute::NoGroup;
}

bool dispatch_unit_message_00821e80(UnitMessageArmHost& host) {
    const std::uint8_t kind = host.message_kind();
    switch (unit_message_arm_00821e80(kind)) {
    case UnitMessageArm::BaseCallForcedTrue:
        host.base_handle_message(); // 00821EEB, result discarded by MOV AL,1
        return true;
    case UnitMessageArm::IgnoredReturnTrue:
        return true;
    case UnitMessageArm::SetFailureVirtual:
        host.set_failure_virtual();
        return true;
    case UnitMessageArm::ClearFailureVirtual:
        host.clear_failure_virtual();
        return true;
    case UnitMessageArm::ApplyRepairSettings:
        host.apply_repair_settings();
        return true;
    case UnitMessageArm::KamikazeDetonate:
        host.kamikaze_detonate(host.resolve_entity(
            static_cast<std::uint16_t>(host.message_dword(0x1c))));
        return true;
    case UnitMessageArm::ShipyardLaunchState:
        host.set_shipyard_launch_state(
            shipyard_launch_state_0082226f(host.message_byte(0x20) != 0));
        return true;
    case UnitMessageArm::UnitLaunchVirtual:
        host.unit_launch_virtual(host.message_sender_control_slot(),
                                 host.message_float(0x20));
        return true;
    case UnitMessageArm::AddLaunchedChild: {
        void* entity = host.resolve_entity(
            static_cast<std::uint16_t>(host.message_dword(0x20)));
        if (entity != nullptr) { // 00821F42 TEST EAX,EAX / JZ 00821F4C
            host.add_launched_child(entity);
        }
        return true;
    }
    case UnitMessageArm::HelmsmanControl:
        if (helmsman_message_accepted_00821ebe(host.unit_helm_station(),
                                               host.message_sender_control_slot())) {
            host.backfill_helm_order();
        }
        return true;
    case UnitMessageArm::AvoidSide: {
        void* target = host.checked_cast_class6(host.resolve_entity(
            static_cast<std::uint16_t>(host.message_dword(0x1c))));
        if (host.has_avoidance_helper()) { // 008222AC TEST ECX,ECX / JZ 008222BA
            host.avoidance_set_side(target, host.message_dword(0x20));
        }
        return true;
    }
    case UnitMessageArm::AddLeak:
        host.add_leak(leak_amount_008221a7(host.message_dword(0x1c)),
                      host.message_payload(0x20));
        return true;
    case UnitMessageArm::ClearAllLeakZones:
        host.clear_all_leak_zones();
        return true;
    case UnitMessageArm::LoadLeakZoneArray:
        host.load_leak_zone_array(static_cast<int>(host.message_dword(0x1c)),
                                  host.message_payload(0x20));
        return true;
    case UnitMessageArm::AddHullTorque:
        // 00822235 copies msg+1Ch/+20h/+24h into a stack triple and passes
        // its address; unit+1018h holds the controller in ECX.
        host.add_hull_torque(nullptr);
        return true;
    case UnitMessageArm::StartLandingVirtual:
        host.start_landing_virtual();
        return true;
    case UnitMessageArm::LandingShipsLaunched:
        host.set_landing_ships_launched(host.message_float(0x1c));
        return true;
    case UnitMessageArm::SetTorpedoStock:
        host.set_torpedo_stock(host.message_dword(0x20));
        return true;
    case UnitMessageArm::Wrecked:
        host.on_wrecked();
        return true;
    case UnitMessageArm::DetachPart:
        host.detach_part(host.message_dword(0x20));
        return true;
    case UnitMessageArm::ExplodeToParts:
        host.on_death();
        return true;
    case UnitMessageArm::SectionFailureEffect:
        host.set_section_failure_effect(host.message_dword(0x20),
                                        host.message_dword(0x24),
                                        host.message_byte(0x28) != 0);
        return true;
    case UnitMessageArm::SpawnTorpedo:
        host.spawn_torpedo(host.message_dword(0x1c), host.message_payload(0x24),
                           host.message_float(0x20));
        return true;
    case UnitMessageArm::RepairAddDamage: {
        const RepairDamageRoute route = repair_damage_route_0082203d(
            static_cast<int>(host.message_dword(0x24)), host.message_byte(0x20) != 0);
        if (route != RepairDamageRoute::Ignored) {
            host.repair_route(route, host.message_float(0x1c));
        }
        return true;
    }
    case UnitMessageArm::SetHullRepairEnabled:
        host.set_hull_repair_enabled(host.message_byte(0x1c) != 0);
        return true;
    case UnitMessageArm::SetRepairPriority:
        host.set_repair_priority(host.message_dword(0x1c));
        return true;
    case UnitMessageArm::BaseCallPassThrough:
        return host.base_handle_message(); // 0082237C, AL is the callee's
    }
    return host.base_handle_message();
}

bool dispatch_base_message_0095abe0(UnitMessageArmHost& host) {
    const std::uint8_t kind = host.message_kind();
    switch (base_message_arm_0095abe0(kind)) {
    case BaseUnitMessageArm::RoleOwner: {
        const bool single = host.message_byte(0x20) != 0;
        if (single) {
            host.role_owner_single_device(host.message_dword(0x28));
            return true;
        }
        const int flags = static_cast<int>(host.message_dword(0x24));
        if ((flags & 1) != 0 && host.message_dword(0x2c) == 1) {
            host.set_role_owner_latch(); // 0095AC53
        }
        const RoleOwnerRoute route = role_owner_route_0095ac1d(false, flags);
        if (route != RoleOwnerRoute::NoGroup) {
            host.role_owner_group(route);
        }
        return true;
    }
    case BaseUnitMessageArm::IgnoredReturnTrue:
        return true;
    case BaseUnitMessageArm::GunControl:
        host.apply_gun_aim_message();
        return true;
    case BaseUnitMessageArm::SetInferiorFailure:
        host.set_inferior_failure(true); // 0095AD0A, before the class gate
        if (host.is_kind_of(kInferiorFailureClassA) ||
            host.is_kind_of(kInferiorFailureClassB)) {
            host.raise_inferior_failure_warning(host.inferior_failure_seconds());
        }
        return true;
    case BaseUnitMessageArm::ClearInferiorFailure:
        host.set_inferior_failure(false); // clears +720h and zeroes +728h
        return true;
    case BaseUnitMessageArm::SetParty:
        host.set_party_and_race(host.message_dword(0x1c), host.current_race());
        return true;
    case BaseUnitMessageArm::SetRace:
        host.set_party_and_race(host.current_party(), host.message_dword(0x1c));
        return true;
    case BaseUnitMessageArm::EntityFallback:
        return host.entity_fallback();
    }
    return host.entity_fallback();
}

} // namespace bsp
