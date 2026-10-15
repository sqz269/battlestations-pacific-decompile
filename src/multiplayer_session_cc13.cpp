// Multiplayer session plumbing, segment 43. See
// include/bsp/multiplayer_session_cc13.hpp and docs/CC13_MULTIPLAYER_SESSION.md.
#include "bsp/multiplayer_session_cc13.hpp"

#include <cstdio>
#include <cstring>
#include <vector>

namespace bsp::mp_cc13 {

namespace {

// 00E0AB68[0..EAh], read from the image (the pointers resolve to .rdata
// strings; entries past MT_LAST are not names).
constexpr const char* kMessageNames[kMessageTypeLast + 1] = {
    "MT_INVALID", "MT_PING", "MT_CORRTIME", "MT_TIME", "MT_ADDPLAYER", "MT_SERVERINFO",
    "MT_REMOVEPLAYER", "MT_CREATED", "MT_SCENENAME", "MT_SCENEIDX", "MT_REFRESHSCENE",
    "MT_SCENELOAD", "MT_SCENELOADED", "MT_CHECKSCENELOAD", "MT_LOADINGINFO", "MT_CONNTEST",
    "MT_CONNSTATE", "MT_LETSPLAY", "MT_SCENETERM", "MT_QUIT", "MT_KICKED", "MT_SCENESCORING",
    "MT_SCENEOBJECTIVE", "MT_CHANGEMULTISETTINGS", "MT_SUBSCENE", "MT_SELECTPLAYER",
    "MT_SWAPSLOTREQUEST", "MT_SWAPSLOTACCEPT", "MT_SWAPSLOTREFUSE", "MT_READYPLAYER",
    "MT_AIENABLE", "MT_AISKILLLEVEL", "MT_PLAYERSURRENDER", "MT_CHAT", "MT_HINT", "MT_COUNTER",
    "MT_PLAYER_PING", "MT_MYPLAYER", "MT_ADDREMOTEVOICE", "MT_REMOVEREMOTEVOICE",
    "MT_MUTEREMOTEVOICE", "MT_VOICEDATA", "MT_MUSICCONTROL_SETLEVEL", "MT_BLACKOUT",
    "MT_COUNTDOWN", "MT_MOVCAM_HANDMODETARGETED", "MT_MOVCAM_REFPOSMODEPOLAR",
    "MT_MOVCAM_REFPOSMODERADIUS", "MT_MOVCAM_PATHWALKPATHPOINTPROPERTYP",
    "MT_MOVCAM_ADDINTERPOLATOR", "MT_MOVCAM_SETDEVIATIONFACTOR", "MT_MOVCAMNEW_ADDPOSITION",
    "MT_MOVCAM_BLACKBAR", "MT_MISSIONINFO", "MT_OBJECTIVE", "MT_OBJ_CREATE", "MT_OBJ_ADD",
    "MT_OBJ_ADD_ENTITY", "MT_OBJ_REMOVE_ENTITY", "MT_OBJ_ADD_POSITION", "MT_OBJ_REMOVE_POSITION",
    "MT_OBJ_COMPLETED", "MT_OBJ_FAILED", "MT_PLAYEROBJ_ADD", "MT_PLAYEROBJ_REMOVE",
    "MT_STATIONARY_SET_DEADMEAT", "MT_SPAWN_FROM_RES", "MT_POWERUP", "MT_POWERUP_EFFECT",
    "MT_POWERUP_BINDING", "MT_NETENT", "MT_CREATE", "MT_SYNC", "MT_CMD", "MT_CONTROL",
    "MT_ROLEOWNER", "MT_ROLEAVAILABLE", "MT_MULTISELECTUNIT", "MT_DEADMEAT", "MT_DESTROY",
    "MT_UNITEXITED", "MT_UNITSOLD", "MT_ACTIVATE", "MT_GIVEUNIT", "MT_SETGUINAME", "MT_CAMERA",
    "MT_DAMAGEDGFXLEVEL", "MT_RESUME_AI_CONTROL", "MT_COMMAND", "MT_GAMEUNITMSG",
    "MT_GAMEUNIT_ATTR", "MT_GAMEUNIT_MOVEONPATH", "MT_GAMEUNIT_SETCMD", "MT_GAMEUNIT_CLEARCMD",
    "MT_GAMEUNIT_SETFIRETARGET", "MT_GAMEUNIT_ADDUSERPATHPOINT", "MT_GAMEUNIT_SETCURPATHINDEX",
    "MT_RECONLEVEL", "MT_FAILURE", "MT_EXPLOSIONFAILURE", "MT_SPLASHFAILURE",
    "MT_ENGINEFIREFAILURE", "MT_ENGINESLOSTFAILURE", "MT_SPINNINGFAILURE",
    "MT_GUN_SET_DESTROYEDFAILURE", "MT_GUN_CLEAR_DESTROYEDFAILURE", "MT_SHIP_SET_EXPLOSIONFAILURE",
    "MT_SHIP_SET_STEERINGJAMFAILURE", "MT_SHIP_SET_ENGINEJAMFAILURE",
    "MT_SHIP_CLEAR_STEERINGJAMFAILURE", "MT_SHIP_CLEAR_ENGINEJAMFAILURE",
    "MT_SHIP_REPAIR_SETTINGS", "MT_SHIP_KAMIKAZE_DETONATE", "MT_PARATROOPER_LANDINGPOS",
    "MT_AIRBASE_SET_RUNWAYFAILURE", "MT_AIRBASE_CLEAR_RUNWAYFAILURE",
    "MT_AIRBASE_SET_HANGARFAILURE", "MT_AIRBASE_CLEAR_HANGARFAILURE", "MT_FORMATION_JOIN",
    "MT_FORMATION_LEAVE", "MT_FORMATION_SET", "MT_VEHICLE_GUN_CONTROL",
    "MT_VEHICLE_SHIPYARD_LAUNCH", "MT_VEHICLE_UNIT_LAUNCH", "MT_VEHICLE_ADD_LAUNCHED",
    "MT_VEHICLE_SET_INFERIORFAILURE", "MT_VEHICLE_CLEAR_INFERIORFAILURE", "MT_VEHICLE_SET_PARTY",
    "MT_VEHICLE_SET_RACE", "MT_AIRBASE_SETSLOT", "MT_AIRBASE_CHANGESLOTSTATUS",
    "MT_AIRBASE_LAUNCHSLOT", "MT_AIRBASE_SLOTSTATUSSYNC", "MT_AIRBASE_STOCKITEM",
    "MT_AIRBASE_APPEARREADYPLANE", "MT_MOTHERSHIP_ELEVATOR_MOVE", "MT_AIRBASE_SETORDERS",
    "MT_AIRBASE_DOLAUNCHSLOT", "MT_SHIP_CREATE", "MT_MOTHERSHIP_CREATE", "MT_SHIP_SYNC",
    "MT_SHIP_GUNS_SYNC", "MT_SHIP_HELMSMAN_CONTROL", "MT_SHIP_AVOIDSIDE", "MT_SHIP_LEAK",
    "MT_SHIP_LEAKCHEATWATER", "MT_SHIP_SINK", "MT_SHIP_ADDTORQUE", "MT_SHIP_STARTLANDING",
    "MT_SHIP_LANDINGSHIPSLAUNCHED", "MT_SHIP_SET_TORPEDOSTOCK", "MT_ENTITY_RELOCATE",
    "MT_SHIP_WRECKED", "MT_SHIP_EXPLODEONEPART", "MT_SHIP_EXPLODETOPARTS",
    "MT_SHIP_SECTIONFAILUREEFX", "MT_SHIP_ERASEWRECK", "MT_SHIP_TORPEDOGENERATE_HACK",
    "MT_SHIPREPAIR_ADDDAMAGE", "MT_SHIPREPAIR_BODYREPAIR", "MT_SHIPREPAIR_REPAIRPRIORITY",
    "MT_SUBMARINE_DIP_RISE", "MT_SUBMARINE_DIP_RISE_SYNC", "MT_AIRFIELD_CREATE",
    "MT_LANDINGSHIP_CREATE", "MT_LANDINGSHIP_STARTLANDING", "MT_LANDINGSHIP_OPEN",
    "MT_SHIPYARD_CHANGE_SLOT_CLASS", "MT_SHIPYARD_CHANGE_SLOT_EQUIPMENT",
    "MT_SHIPYARD_LAUNCH_SLOT", "MT_SHIPYARD_SPAWN", "MT_GUN_CREATE", "MT_GUN_SET_FIRE_MODE",
    "MT_GUN_FIRE", "MT_T_GUN_TURN_SYNC", "MT_R_T_GUN_SET_CONTFIRING", "MT_S_T_GUN_FIRE",
    "MT_BOMBPLATFORM_FIRE", "MT_BOMBPLATFORM_RELOAD", "MT_MULTIBOMBPLATFORM_GENERATE",
    "MT_CATAPULT_FIRE", "MT_WATERMINE_ARM", "MT_BOMB_CREATE", "MT_DEPTHCHARGE_CREATE",
    "MT_TORPEDO_CREATE", "MT_PARATROOPER_CREATE", "MT_KILL_BULLET", "MT_PLANESQUADRON_CREATE",
    "MT_PLANESQUADRON_ATTACKRUN", "MT_PLANESQUADRON_BAYDOOR_SYNC", "MT_PLANESQUADRON_SETLEADER",
    "MT_PLAYERSPAWNPOINT_SETACTIVE", "MT_PLANE_CREATE", "MT_PLANE_SYNC", "MT_PLANE_SYNC_ACK",
    "MT_PLANE_CHANGE_STATE", "MT_PLANE_BOMB_TRIGGER", "MT_PLANE_TOUCHDOWN", "MT_PLANE_TAKEOFF",
    "MT_PLANE_DISAPPEAR", "MT_PLANE_DEADMEATROT", "MT_PLANE_CONTROLFACTOR", "MT_PLANE_HIT",
    "MT_PLANE_GUI", "MT_PLANE_PCMDLASTTIME", "MT_GRNDVEHICLE_CREATE", "MT_GRND_VEHICLE_SYNC",
    "MT_LANDCONVOY_CREATE", "MT_LANDFORT_CREATE", "MT_LANDFORT_EFFECT", "MT_FRAGILE_SETHP",
    "MT_COMMANDBUILDING_SETOWNER", "MT_COMMANDBUILDING_SETLEVEL",
    "MT_COMMANDBUILDING_UPDATECAPTURE", "MT_COMMANDBUILDING_INFERIORLIST",
    "MT_HIGHLVLAI_SPAWNBANN", "MT_PATH_CREATE", "MT_PATH_SET", "MT_PATH_ADD", "MT_PATH_INSERT",
    "MT_PATH_DELETE", "MT_PLAYERUNLOCKUNIT", "MT_SERVERDLCUNLOCK", "MT_MNETTABLESYNCENTITY_CREATE",
    "MT_MNETTABLESYNCENTITY_SYNC", "MT_MANAGE_DIALOGS", "MT_MANAGE_MESSAGES", "MT_PLAYERINFO",
    "MT_SESSION_MANAGER", "MT_HITMARKER", "MT_HITWARNING", "MT_VOICEOVER", "MT_VOICECHANGE",
    "MT_METRICSSESSION", "MT_LAST",
};
static_assert(sizeof(kMessageNames) / sizeof(kMessageNames[0]) == 0xEB);

struct RouteRow {
    std::uint8_t type;
    std::uint32_t site;
    std::uint32_t host;
    std::uint32_t client;
    HandlerArgs host_args;
    HandlerArgs client_args;
};

// 00777850's is-a ladder in test order (sites are the CALL [vtable+0Ch]).
constexpr RouteRow kReceiveLadder[] = {
    {0x0E, 0x007778E5U, 0x00000000U, 0x00000000U, HandlerArgs::kNone, HandlerArgs::kNone},
    {0x0F, 0x00777919U, 0x00000000U, 0x00000000U, HandlerArgs::kNone, HandlerArgs::kNone},
    {0x10, 0x0077792CU, 0x0076CD20U, 0x0076CD20U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x11, 0x0077794DU, 0x00771FC0U, 0x00771FC0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x04, 0x0077796EU, 0x00773120U, 0x00773120U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x06, 0x0077798FU, 0x00773420U, 0x00773420U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x05, 0x007779B0U, 0x0076A7B0U, 0x0076A7B0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x35, 0x007779D1U, 0x00734DB0U, 0x00734DB0U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x36, 0x007779EAU, 0x008E2390U, 0x008E2390U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x41, 0x00777A17U, 0x0084EC10U, 0x0084EC10U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x42, 0x00777A30U, 0x0094B600U, 0x0094B600U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x43, 0x00777A84U, 0x008EE020U, 0x008EE020U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x44, 0x00777AACU, 0x008EA4E0U, 0x008EA4E0U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x45, 0x00777AD4U, 0x008E6D30U, 0x008E6D30U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x07, 0x00777AFCU, 0x00000000U, 0x00000000U, HandlerArgs::kNone, HandlerArgs::kNone},
    {0x08, 0x00777B1AU, 0x0076E710U, 0x0076E710U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x09, 0x00777B3BU, 0x007724A0U, 0x007724A0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x0A, 0x00777B5CU, 0x007717A0U, 0x007717A0U, HandlerArgs::kSessionPeer, HandlerArgs::kSessionPeer},
    {0x0B, 0x00777B7BU, 0x0076D030U, 0x0076D030U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x0D, 0x00777B9CU, 0x007F7460U, 0x00772710U, HandlerArgs::kOther, HandlerArgs::kSessionMsgStatus},
    {0x0C, 0x00777BD7U, 0x0076D090U, 0x004DA6C0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kOther},
    {0x12, 0x00777C1CU, 0x0076D260U, 0x00772990U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x13, 0x00777C58U, 0x0076D140U, 0x0076D0E0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x14, 0x00777C94U, 0x0076D170U, 0x0076D170U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x15, 0x00777CB5U, 0x0091C650U, 0x0091C650U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x16, 0x00777CD7U, 0x009105F0U, 0x009105F0U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x19, 0x00777CF0U, 0x0076C840U, 0x007705A0U, HandlerArgs::kSessionTwoDwords, HandlerArgs::kSessionTwoDwords},
    {0x1A, 0x00777D2FU, 0x00771990U, 0x00771990U, HandlerArgs::kSessionTwoDwords, HandlerArgs::kSessionTwoDwords},
    {0x1B, 0x00777D52U, 0x00771AC0U, 0x00771AC0U, HandlerArgs::kSessionTwoDwords, HandlerArgs::kSessionTwoDwords},
    {0x1C, 0x00777D75U, 0x00771CC0U, 0x00771CC0U, HandlerArgs::kSessionTwoDwords, HandlerArgs::kSessionTwoDwords},
    {0x17, 0x00777D98U, 0x005E50D0U, 0x005E50D0U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x18, 0x00777DB1U, 0x007708E0U, 0x007708E0U, HandlerArgs::kSessionPayloadWord, HandlerArgs::kSessionPayloadWord},
    {0x1D, 0x00777DD4U, 0x0076CAF0U, 0x0076CAF0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x1E, 0x00777DF5U, 0x0076CC20U, 0x0076CC20U, HandlerArgs::kSessionMsg, HandlerArgs::kSessionMsg},
    {0x1F, 0x00777E11U, 0x0076CCD0U, 0x0076CCD0U, HandlerArgs::kSessionMsg, HandlerArgs::kSessionMsg},
    {0x20, 0x00777E2DU, 0x00770850U, 0x00770850U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x21, 0x00777E4EU, 0x007722E0U, 0x007722E0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x22, 0x00777E6FU, 0x00777820U, 0x00777820U, HandlerArgs::kSessionMsg, HandlerArgs::kSessionMsg},
    {0x23, 0x00777E8BU, 0x0076CEA0U, 0x0076CEA0U, HandlerArgs::kSessionMsg, HandlerArgs::kSessionMsg},
    {0x24, 0x00777EA7U, 0x008E2DA0U, 0x008E2DA0U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0x4D, 0x00777EC0U, 0x0076CFD0U, 0x0076CFD0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x25, 0x00777EE1U, 0x0076C7B0U, 0x0076C7B0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x2A, 0x00777F02U, 0x00A788A0U, 0x00A788A0U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x2B, 0x00777F25U, 0x0076F4C0U, 0x0076F4C0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x2C, 0x00777F46U, 0x007709C0U, 0x007709C0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x2D, 0x00777F67U, 0x0076D420U, 0x0076D420U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x2E, 0x00777F88U, 0x0076F560U, 0x0076F560U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x2F, 0x00777FA9U, 0x0076F630U, 0x0076F630U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x30, 0x00777FCAU, 0x0076A870U, 0x0076A870U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x31, 0x00777FEBU, 0x0076A8A0U, 0x0076A8A0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x32, 0x0077800CU, 0x0076A940U, 0x0076A940U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x33, 0x0077802DU, 0x0076D760U, 0x0076D760U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x34, 0x0077804EU, 0x0076A970U, 0x0076A970U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0xC2, 0x00778072U, 0x00000000U, 0x00000000U, HandlerArgs::kNone, HandlerArgs::kNone},
    {0x26, 0x00778085U, 0x00772D20U, 0x0076D840U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x27, 0x007780C0U, 0x00772D80U, 0x0076D890U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x28, 0x007780FBU, 0x0076D8E0U, 0x0076D8E0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x29, 0x0077811CU, 0x0076D9C0U, 0x0076D9C0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0x55, 0x0077813DU, 0x007F7C60U, 0x007F7C60U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xD7, 0x00778174U, 0x00A38DA0U, 0x00A38DA0U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0x46, 0x00778193U, 0x00780670U, 0x00780670U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xDD, 0x007781BFU, 0x0076CB30U, 0x0076CB30U, HandlerArgs::kSessionMsg, HandlerArgs::kSessionMsg},
    {0xDE, 0x007781DEU, 0x0050A820U, 0x0050A820U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE1, 0x00778204U, 0x00451E50U, 0x00451E50U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE2, 0x0077822DU, 0x00706760U, 0x00706760U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE3, 0x0077825CU, 0x007F8210U, 0x007F8210U, HandlerArgs::kMessageThis, HandlerArgs::kMessageThis},
    {0xE4, 0x00778278U, 0x007746F0U, 0x007746F0U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0xE5, 0x0077829CU, 0x0096F090U, 0x0096F090U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE6, 0x007782C4U, 0x00978A70U, 0x00978A70U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE7, 0x007782ECU, 0x0097B960U, 0x0097B960U, HandlerArgs::kOther, HandlerArgs::kOther},
    {0xE8, 0x00778311U, 0x0076DA10U, 0x0076DA10U, HandlerArgs::kSessionMsgStatus, HandlerArgs::kSessionMsgStatus},
    {0xE9, 0x00778332U, 0x00000000U, 0x00000000U, HandlerArgs::kNone, HandlerArgs::kNone},
};

constexpr bool valid_index(std::int32_t i) noexcept { return i >= 0 && i < 8; }

BuiltMessage make_message(std::uint8_t type, std::uint32_t vtable) noexcept {
    BuiltMessage m;
    m.type = type;
    m.vtable = vtable;
    return m;
}

void refresh_after_slot_change(SessionHost& host) {
    host.refresh_lobby_005d9d60();
    host.leave_tactical_library_if_open();
}

void clear_row_and_column(VoiceMuteTable& table, int i) noexcept {
    for (int j = 0; j < 8; ++j) {
        table.muted[j][i] = 0;  // column, puVar3 stepping by 8
        table.muted[i][j] = 0;  // row
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Message helpers
// ---------------------------------------------------------------------------

void BuiltMessage::put32(std::size_t offset, std::uint32_t value) noexcept {
    std::memcpy(payload.data() + (offset - 0x18), &value, sizeof value);
}
void BuiltMessage::put8(std::size_t offset, std::uint8_t value) noexcept {
    payload[offset - 0x18] = value;
}
std::uint32_t BuiltMessage::get32(std::size_t offset) const noexcept {
    std::uint32_t v;
    std::memcpy(&v, payload.data() + (offset - 0x18), sizeof v);
    return v;
}
std::uint8_t BuiltMessage::get8(std::size_t offset) const noexcept {
    return payload[offset - 0x18];
}
std::uint32_t ReceivedMessage::get32(std::size_t offset) const noexcept {
    std::uint32_t v;
    std::memcpy(&v, payload.data() + (offset - 0x18), sizeof v);
    return v;
}
std::uint16_t ReceivedMessage::get16(std::size_t offset) const noexcept {
    std::uint16_t v;
    std::memcpy(&v, payload.data() + (offset - 0x18), sizeof v);
    return v;
}
std::uint8_t ReceivedMessage::get8(std::size_t offset) const noexcept {
    return payload[offset - 0x18];
}
std::uint64_t ReceivedMessage::get64(std::size_t offset) const noexcept {
    return static_cast<std::uint64_t>(get32(offset)) |
           (static_cast<std::uint64_t>(get32(offset + 4)) << 32);
}

const char* message_type_name_00e0ab68(std::uint8_t type) noexcept {
    return type <= kMessageTypeLast ? kMessageNames[type] : nullptr;
}

// ---------------------------------------------------------------------------
// 00777850
// ---------------------------------------------------------------------------

ReceiveRoute route_receive_00777850(std::uint8_t type) noexcept {
    for (const RouteRow& row : kReceiveLadder) {
        if (row.type == type) {
            return {true, row.site, row.host, row.client, row.host_args, row.client_args};
        }
    }
    return {};
}

QueueOutcome receive_queue_outcome_00777850(QueuedMessageStatus status,
    const DropLogInputs& inputs, bool& logs_drop) noexcept {
    logs_drop = false;
    if (status == QueuedMessageStatus::kKeep) {
        // 00778371..007783B3: log, then status 2.
        if (inputs.game_present && inputs.game_state < 0x0A && inputs.message_tick > 0) {
            logs_drop = true;
            return QueueOutcome::kUnlinkAndDestroy;
        }
        return QueueOutcome::kKeepAndAdvance;  // 0077843D
    }
    // 007783F6: only status 2 destroys.
    return status == QueuedMessageStatus::kConsume ? QueueOutcome::kUnlinkAndDestroy
                                                   : QueueOutcome::kUnlink;
}

// ---------------------------------------------------------------------------
// Mute table
// ---------------------------------------------------------------------------

int voice_mute_find_007785d0(const VoiceMuteTable& table, std::uint64_t xuid) noexcept {
    for (int i = 0; i < 8; ++i) {
        if (table.active[i] != 0 && table.xuid[i] == xuid) return i;
    }
    return -1;
}

void voice_mute_add_00778620(VoiceMuteTable& table, std::uint64_t xuid) noexcept {
    if (voice_mute_find_007785d0(table, xuid) >= 0) return;
    int i = 0;
    while (table.active[i] != 0) {
        if (++i > 7) return;
    }
    table.xuid[i] = xuid;
    table.active[i] = 1;
    clear_row_and_column(table, i);
}

void voice_mute_remove_00778690(VoiceMuteTable& table, std::uint64_t xuid) noexcept {
    const int i = voice_mute_find_007785d0(table, xuid);
    if (i < 0) return;
    table.xuid[i] = 0;
    table.active[i] = 0;
    clear_row_and_column(table, i);
}

void voice_mute_set_00778780(VoiceMuteTable& table, std::uint64_t a, std::uint64_t b) noexcept {
    const int i = voice_mute_find_007785d0(table, a);
    const int j = voice_mute_find_007785d0(table, b);
    if (i >= 0 && j >= 0) table.muted[i][j] = 1;
}

void voice_mute_clear_007787c0(VoiceMuteTable& table, std::uint64_t a, std::uint64_t b) noexcept {
    const int i = voice_mute_find_007785d0(table, a);
    const int j = voice_mute_find_007785d0(table, b);
    if (i >= 0 && j >= 0) table.muted[i][j] = 0;
}

bool voice_mute_is_muted_007786e0(const VoiceMuteTable& table, std::uint64_t a,
                                  std::uint64_t b) noexcept {
    const int i = voice_mute_find_007785d0(table, a);
    const int j = voice_mute_find_007785d0(table, b);
    if (i < 0 || j < 0 || i == j) return false;
    return table.muted[i][j] != 0 || table.muted[j][i] != 0;
}

// ---------------------------------------------------------------------------
// Builders
// ---------------------------------------------------------------------------

BuiltMessage build_mute_remote_voice_0076bfe0(std::uint64_t a, std::uint64_t b, bool mute) noexcept {
    BuiltMessage m = make_message(0x28, kVtableMuteVoice28);
    m.put32(0x18, static_cast<std::uint32_t>(a));
    m.put32(0x1C, static_cast<std::uint32_t>(a >> 32));
    m.put32(0x20, static_cast<std::uint32_t>(b));
    m.put32(0x24, static_cast<std::uint32_t>(b >> 32));
    m.put8(0x28, mute ? 1 : 0);
    return m;
}

BuiltMessage build_swap_slot_refuse_0076ba80(std::int32_t player, std::int32_t slot) noexcept {
    BuiltMessage m = make_message(0x1C, kVtableSwapRefuse1C);
    m.put32(0x18, static_cast<std::uint32_t>(player));
    m.put32(0x1C, static_cast<std::uint32_t>(slot));
    return m;
}

// ---------------------------------------------------------------------------
// Voice
// ---------------------------------------------------------------------------

PlayerRecord* find_player_by_xuid_0076d7d0(GameView& game, std::uint64_t xuid) noexcept {
    for (PlayerRecord& record : game.players) {
        if (record.claimed_08 != 0 && record.xuid_80 == xuid) return &record;
    }
    return nullptr;
}

void client_receive_remote_voice_0076d840(GameView& game, const ReceivedMessage& msg,
                                          bool add, SessionHost& host) {
    host.trace(add ? "__P2P_VOICE__   ClientReceiveAddRemoteVoice"
                   : "__P2P_VOICE__   ClientReceiveRemoveRemoteVoice");
    const std::uint64_t xuid = msg.get64(0x18);
    if (PlayerRecord* record = find_player_by_xuid_0076d7d0(game, xuid)) {
        record->voice_registered_89 = add ? 1 : 0;
    }
    // No null check on [00F8A2FC] in either body.
    if (add) host.online_add_remote_talker_ac(xuid);
    else host.online_remove_remote_talker_b0(xuid);
}

void set_remote_voice_00772bb0(SessionState& session, GameView& game, std::uint64_t xuid,
                               bool add, SessionHost& host) {
    BuiltMessage m = add ? make_message(0x26, kVtableAddVoice26)
                         : make_message(0x27, kVtableRemoveVoice27);
    m.put32(0x18, static_cast<std::uint32_t>(xuid));
    m.put32(0x1C, static_cast<std::uint32_t>(xuid >> 32));
    if (session.mode == SessionMode::kHost) {
        PlayerRecord* record = find_player_by_xuid_0076d7d0(game, xuid);
        host.broadcast_except_00784790(&m, record ? record->peer_50 : nullptr);
    } else {
        host.send_to_server_00770af0(m);
    }
    if (PlayerRecord* record = find_player_by_xuid_0076d7d0(game, xuid)) {
        record->voice_registered_89 = add ? 1 : 0;
    }
}

void server_receive_remote_voice_00772d20(SessionState& session, GameView& game,
                                          const ReceivedMessage& msg, bool add,
                                          SessionHost& host) {
    host.trace(add ? "__P2P_VOICE__   ServerReceiveAddRemoteVoice"
                   : "__P2P_VOICE__   ServerReceiveRemoveRemoteVoice");
    const std::uint64_t xuid = msg.get64(0x18);
    if (PlayerRecord* record = find_player_by_xuid_0076d7d0(game, xuid)) {
        record->voice_registered_89 = add ? 1 : 0;
    }
    set_remote_voice_00772bb0(session, game, xuid, add, host);
    if (add) host.online_add_remote_talker_ac(xuid);
    else host.online_remove_remote_talker_b0(xuid);
}

void receive_mute_remote_voice_0076d8e0(SessionState& session, const ReceivedMessage& msg,
                                        SessionHost& host) {
    const std::uint64_t a = msg.get64(0x18);
    const std::uint64_t b = msg.get64(0x20);
    const bool mute = msg.get8(0x28) != 0;
    if (mute) voice_mute_set_00778780(session.mute, a, b);
    else voice_mute_clear_007787c0(session.mute, a, b);
    if (session.server_transport && a != b) {
        const BuiltMessage m = build_mute_remote_voice_0076bfe0(a, b, mute);
        host.broadcast_except_00784790(&m, nullptr);
    }
}

void receive_voice_data_0076d9c0(GameView& game, const ReceivedMessage& msg, SessionHost& host) {
    PlayerRecord* record = find_player_by_xuid_0076d7d0(game, msg.get64(0x18));
    if (record == nullptr) {
        host.trace("WOICE ERROR");  // sic, string at 00D039C0
        return;
    }
    host.stamp_voice_time_007f7490(*record);
    host.online_submit_voice_a8(msg.payload.data());
}

void receive_voice_change_0076da10(const SessionState& session, GameView& game,
                                   const ReceivedMessage& msg, SessionHost& host) {
    PlayerRecord* record = find_player_by_xuid_0076d7d0(game, msg.get64(0x20));
    if (record == nullptr) return;
    record->voice_registered_89 = msg.get8(0x18);
    if (session.mode == SessionMode::kHost) {
        host.broadcast_except_00784790(&msg, record->peer_50);
    }
}

void set_muted_00771630(SessionState& session, std::uint64_t target, bool mute,
                        SessionHost& host) {
    // 00771649..007716D3: privileges can force the mute on, never off.
    bool allowed = false;
    std::uint32_t result = host.check_privilege(kPrivilegeCommunications, allowed);
    if (result != 0) {
        host.trace("MMultiPlayer::SetMuted(), XUserCheckPrivilege return: %d");
    } else if (!allowed) {
        result = host.check_privilege(kPrivilegeCommunicationsFriendsOnly, allowed);
        if (result != 0) {
            host.trace("MMultiPlayer::SetMuted(), XUserCheckPrivilege return: %d");
        } else if (!allowed) {
            mute = true;
        } else {
            bool friends = false;
            result = host.are_friends(target, friends);
            if (result != 0) {
                host.trace("MMultiPlayer::SetMuted(), XUserAreUsersFriends return: %d");
            } else if (!friends) {
                mute = true;
            }
        }
    }
    const std::uint64_t local = host.local_xuid_00a3eb00();
    if (session.mode == SessionMode::kClient && local != target) {
        // 007716E9..00771719: the client asks the server and does not touch
        // its own table; the server's 0076D8E0 applies and rebroadcasts.
        host.send_to_server_00770af0(build_mute_remote_voice_0076bfe0(local, target, mute));
        return;
    }
    if (mute) voice_mute_set_00778780(session.mute, local, target);
    else voice_mute_clear_007787c0(session.mute, local, target);
    host.trace("__SetLocal_VOICE__  MUTE(%llx, %llx) %d");
}

// ---------------------------------------------------------------------------
// Player slots
// ---------------------------------------------------------------------------

void set_player_ready_0076ca40(const SessionState& session, GameView& game, std::int32_t player,
                               std::uint8_t ready, std::uint8_t aux, SessionHost& host) {
    PlayerRecord& record = game.players[player];  // no bounds check in the image
    record.ready_0c = ready;
    record.state_10 = 0xFFFD;
    record.byte_18 = 0;
    record.ready_aux_0d = aux;
    if (session.mode == SessionMode::kHost) {
        BuiltMessage m = make_message(0x1D, kVtableReadyPlayer1D);
        m.put32(0x18, static_cast<std::uint32_t>(player));
        m.put8(0x1C, ready);
        m.put8(0x1D, aux);
        host.broadcast_except_00784790(&m, nullptr);
    }
}

void receive_ready_player_0076caf0(const SessionState& session, GameView& game,
                                   const ReceivedMessage& msg, SessionHost& host) {
    if (session.mode != SessionMode::kHost || game.state == 8) {
        set_player_ready_0076ca40(session, game, static_cast<std::int32_t>(msg.get32(0x18)),
                                  msg.get8(0x1C), msg.get8(0x1D), host);
    }
}

void set_slot_ai_enabled_0076cb90(const SessionState& session, GameView& game, std::int32_t slot,
                                  std::uint8_t enabled, SessionHost& host) {
    game.current[slot]->ai_enabled_0a = enabled;
    if (session.mode == SessionMode::kHost) {
        BuiltMessage m = make_message(0x1E, kVtableAiEnable1E);
        m.put32(0x18, static_cast<std::uint32_t>(slot));
        m.put8(0x1C, enabled);
        host.broadcast_except_00784790(&m, nullptr);
    }
}

void set_slot_ai_skill_0076cc40(const SessionState& session, GameView& game, std::int32_t slot,
                                std::uint32_t level, SessionHost& host) {
    game.current[slot]->ai_skill_2c = level;
    if (session.mode == SessionMode::kHost) {
        BuiltMessage m = make_message(0x1F, kVtableAiSkill1F);
        m.put32(0x18, static_cast<std::uint32_t>(slot));
        m.put32(0x1C, level);
        host.broadcast_except_00784790(&m, nullptr);
    }
}

void set_player_surrender_00770750(const SessionState& session, GameView& game,
                                   std::int32_t player, bool surrender, SessionHost& host) {
    PlayerRecord& record = game.players[player];
    if (host.script_events_present_00f8a0c4()) {
        host.post_slot_event_00985920(record.slot_20, surrender ? "surrender" : "notsurrender");
    }
    record.surrender_1b = surrender ? 1 : 0;
    if (session.mode == SessionMode::kHost) {
        BuiltMessage m = make_message(0x20, kVtableSurrender20);
        m.put32(0x18, static_cast<std::uint32_t>(player));
        m.put8(0x1C, surrender ? 1 : 0);
        host.broadcast_except_00784790(&m, nullptr);
    }
}

void receive_player_unlock_0076cb30(GameView& game, const ReceivedMessage& msg,
                                    const void* raw_message, SessionHost& host) {
    const std::int32_t player = static_cast<std::int32_t>(msg.get32(0x18));
    game.players[player].unlock_class_110 = msg.get32(0x1C);
    host.trace("UNLOCK CLASS received: Player %d unlock class %d");
    if (game.mode == 1) host.broadcast_except_00784790(raw_message, nullptr);
}

void host_select_player_0076c840(GameView& game, std::int32_t player, std::int32_t slot,
                                 SessionHost& host) {
    PlayerRecord& record = game.players[player];
    const bool target_free = slot == -1 || game.current[slot]->ai_held_09 != 0;
    if (!target_free || record.claimed_08 == 0 || record.ai_held_09 != 0) return;
    const std::int32_t old_slot = record.slot_20;
    if (old_slot >= 0) {
        host.restore_mission_record_004bb660(old_slot);
        BuiltMessage m = make_message(0x19, kVtableSelectPlayer19);
        m.put32(0x18, 0xFFFFFFFFu);
        m.put32(0x1C, static_cast<std::uint32_t>(old_slot));
        host.broadcast_except_00784790(&m, nullptr);
        if (player == game.local_player) host.select_local_slot_004b4740(-1);
    }
    if (slot != -1) {
        host.bind_record_to_slot_004bb630(record, slot);
        BuiltMessage m = make_message(0x19, kVtableSelectPlayer19);
        m.put32(0x18, static_cast<std::uint32_t>(player));
        m.put32(0x1C, static_cast<std::uint32_t>(slot));
        host.broadcast_except_00784790(&m, nullptr);
        if (player == game.local_player) {
            host.select_local_slot_004b4740(slot);
            host.refresh_lobby_005d9d60();
        }
    }
}

void client_select_player_007705a0(GameView& game, std::int32_t player, std::int32_t slot,
                                   SessionHost& host) {
    if (player < 0) {
        host.leave_tactical_library_if_open();
        if (game.local_slot == slot) host.select_local_slot_004b4740(-1);
        const std::string name = game.current[slot]->name_58;
        host.restore_mission_record_004bb660(slot);
        // The mission record keeps the departing name in states 0Ah/0Dh/10h.
        if (game.state == 0x0D || game.state == 0x0A || game.state == 0x10) {
            game.current[slot]->name_58 = name;
        }
        return;
    }
    host.bind_record_to_slot_004bb630(game.players[player], slot);
    if (player == game.local_player) {
        host.select_local_slot_004b4740(slot);
        refresh_after_slot_change(host);
    }
}

void swap_slot_request_00771990(const SessionState& session, GameView& game,
                                std::int32_t player, std::int32_t slot, SessionHost& host) {
    if (slot == game.local_slot) {
        host.swap_request_local_005d8800();
        return;
    }
    if (session.mode == SessionMode::kHost) {
        if (void* holder = game.current[slot]->peer_50) {
            BuiltMessage m = make_message(0x1A, kVtableSwapRequest1A);
            m.put32(0x18, static_cast<std::uint32_t>(player));
            m.put32(0x1C, static_cast<std::uint32_t>(slot));
            host.send_to_peer_00770b50(holder, &m);
            return;
        }
        // No remote human holds the slot: refuse to the requester, if remote.
        void* requester = game.players[player].peer_50;
        if (requester == nullptr) return;
        const BuiltMessage m = build_swap_slot_refuse_0076ba80(player, slot);
        host.send_to_peer_00770b50(requester, &m);
        return;
    }
    BuiltMessage m = make_message(0x1A, kVtableSwapRequest1A);
    m.put32(0x18, static_cast<std::uint32_t>(player));
    m.put32(0x1C, static_cast<std::uint32_t>(slot));
    host.send_to_server_00770af0(m);
}

void swap_slot_accept_00771ac0(const SessionState& session, GameView& game,
                               std::int32_t player, std::int32_t slot, SessionHost& host) {
    if (session.mode == SessionMode::kHost) {
        PlayerRecord& holder = *game.current[slot];
        PlayerRecord& record = game.players[player];
        if (record.claimed_08 != 0 && record.ai_held_09 == 0 && holder.claimed_08 != 0 &&
            holder.ai_held_09 == 0) {
            const std::int32_t old_slot = record.slot_20;
            host_select_player_0076c840(game, player, -1, host);
            host_select_player_0076c840(game, holder.owner_player_1c, old_slot, host);
            host_select_player_0076c840(game, player, slot, host);
            host.swap_slot_names_005cf320(holder.name_58, record.name_58);
        }
    } else {
        BuiltMessage m = make_message(0x1B, kVtableSwapAccept1B);
        m.put32(0x18, static_cast<std::uint32_t>(player));
        m.put32(0x1C, static_cast<std::uint32_t>(slot));
        host.send_to_server_00770af0(m);
    }
    if (player == game.local_player || slot == game.local_slot) {
        refresh_after_slot_change(host);
    }
}

void swap_slot_refuse_00771cc0(const SessionState& session, GameView& game,
                               std::int32_t player, std::int32_t slot, SessionHost& host) {
    if (player == game.local_player) {
        host.swap_refused_local_005d8950();
        return;
    }
    if (session.mode == SessionMode::kHost) {
        if (void* requester = game.players[player].peer_50) {
            const BuiltMessage m = build_swap_slot_refuse_0076ba80(player, slot);
            host.send_to_peer_00770b50(requester, &m);
        }
        return;
    }
    host.send_to_server_00770af0(build_swap_slot_refuse_0076ba80(player, slot));
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

void receive_my_player_0076c7b0(SessionState& session, GameView& game,
                                const ReceivedMessage& msg, SessionHost& host) {
    game.local_player = static_cast<std::int32_t>(msg.get32(0x18));
    session.nonce_268 = (static_cast<std::uint64_t>(msg.get32(0x1C)) << 32) + msg.get32(0x20);
    host.trace("__ONLINE__ Receive Session Nonce %llx");
    host.publish_nonce_207c(session.nonce_268);
}

void receive_multi_select_unit_0076cfd0(const GameView& game, const ReceivedMessage& msg,
                                        SessionHost& host) {
    // 0076CFF9 formats "**LOFING: myplayer:%d  msgplayer:%d **" into a stack
    // buffer that nothing reads. The HUD root [00E198C4] is not null-checked.
    if (game.local_slot == static_cast<std::int32_t>(msg.get32(0x18))) {
        host.set_ship_screen_selection(msg.get16(0x1C));
    }
}

void receive_scene_load_0076d030(GameView& game, SessionHost& host) {
    game.local_view_1ee3 = game.mode != 0 ? 1 : 0;
    host.online_scene_load_54();
    host.request_game_state_004d7920(10);
}

void host_receive_scene_loaded_0076d090(const GameView& game, const ReceivedMessage& msg) {
    if (game.game_624 != 0) return;
    PlayerRecord& record = *msg.source_14;
    record.loaded_0e = 1;
    record.state_10 = 0xFFFD;
    record.byte_18 = 0;
}

void host_receive_quit_0076d140(const SessionState& session, const ReceivedMessage& msg,
                                SessionHost& host) {
    if (session.server_transport) host.kick_peer_00784b10(msg.peer_08, 0);
}

void receive_kicked_0076d170(SessionState& session, const ReceivedMessage& msg,
                             SessionHost& host) {
    if (!session.client_transport) return;
    host.client_transport_3c();
    const std::uint32_t reason = msg.get8(0x18) != 0 ? 3 : 4;  // NEG/SBB/ADD 4
    if (session.client_stage_9c != reason) session.client_stage_9c = reason;
}

void host_receive_scene_term_0076d260(const ReceivedMessage& msg, SessionHost& host) {
    host.clear_peer_metadata_1a(msg.peer_08);
}

void receive_counter_0076cea0(const SessionState& session, SessionHost& host) {
    if (session.mode == SessionMode::kClient && host.hud_counter_present()) {
        host.hud_counter_0052cf60();
    }
}

void client_enter_play_007712f0(SessionState& session, GameView& game,
                                std::uint8_t& game_193c, LetsPlayHost& host) {
    // [+18Ch]+9Ch < 8, signed (JGE at 0077131C); no +18Ch null check.
    if (static_cast<std::int32_t>(session.client_stage_9c) >= 8) return;
    session.client_stage_9c = 8;
    if (valid_index(game.local_slot)) game.current[game.local_slot]->byte_19 = 0;
    game_193c = 0;
    host.store_ingame_field_1c_00644220(0);
    host.send_to_server_00770af0(make_message(0x07, kVtableCreated07));
}

void receive_lets_play_00771fc0(SessionState& session, GameView& game,
                                const ReceivedMessage& msg, std::uint8_t& game_193c,
                                LetsPlayHost& host) {
    if (session.mode == SessionMode::kClient) {
        session.phase_248 = 2;
        host.reset_mission_clock_00874640(0);
        game.current[game.local_slot]->state_10 = 1;
        client_enter_play_007712f0(session, game, game_193c, host);
        return;
    }
    msg.source_14->state_10 = 1;
}

// ---------------------------------------------------------------------------
// Batch 2
// ---------------------------------------------------------------------------

void request_player_ready_00771dd0(const SessionState& session, GameView& game,
                                   std::int32_t player, std::uint8_t ready, std::uint8_t aux,
                                   SessionHost& host) {
    if (session.mode == SessionMode::kHost) {
        set_player_ready_0076ca40(session, game, player, ready, aux, host);
        return;
    }
    if (session.mode == SessionMode::kClient) {
        BuiltMessage m = make_message(0x1D, kVtableReadyPlayer1D);
        m.put32(0x18, static_cast<std::uint32_t>(player));
        m.put8(0x1C, ready);
        m.put8(0x1D, aux);
        host.send_to_server_00770af0(m);
    }
}

namespace {
bool session_is_networked(const SessionState& session) noexcept {
    return session.mode == SessionMode::kHost || session.mode == SessionMode::kClient;
}
}  // namespace

void send_to_unmuted_00770c50(const SessionState& session, GameView& game, const void* message,
                              std::uint64_t sender, void* exclude_peer, ChatHost& host) {
    if (!session_is_networked(session)) return;
    for (PlayerRecord& record : game.players) {
        if (record.peer_50 == exclude_peer || record.ai_held_09 != 0 || record.claimed_08 == 0) {
            continue;
        }
        if (voice_mute_is_muted_007786e0(session.mute, record.xuid_80, sender)) continue;
        if (record.xuid_80 == sender) continue;
        if (record.peer_50 == nullptr) {
            host.trace("###%d#Nem_kuldom_el mert nincs peerInfoja!_4 %x");
        } else {
            host.send_to_peer_00770b50(record.peer_50, message);
        }
    }
}

void send_to_team_unmuted_00770d40(const SessionState& session, GameView& game,
                                   std::int32_t team, const void* message, std::uint64_t sender,
                                   void* exclude_peer, ChatHost& host) {
    if (!session_is_networked(session)) return;
    for (PlayerRecord& record : game.players) {
        if (record.ai_held_09 != 0 || record.claimed_08 == 0 || record.peer_50 == exclude_peer) {
            continue;
        }
        // Both directions are asked, although 007786E0 is already symmetric.
        if (voice_mute_is_muted_007786e0(session.mute, record.xuid_80, sender) ||
            voice_mute_is_muted_007786e0(session.mute, sender, record.xuid_80)) {
            continue;
        }
        if (record.peer_50 == nullptr || record.team_28 != team || record.xuid_80 == sender) {
            continue;
        }
        host.send_to_peer_00770b50(record.peer_50, message);
    }
}

void host_relay_chat_00772020(const SessionState& session, GameView& game, const ChatLine& line,
                              const void* raw_message, std::int32_t team_of_local,
                              ChatHost& host) {
    switch (line.channel) {
    case 0:  // player 0 is the host itself
        host.add_chat_line_005d0770(line, 0);
        return;
    case 1: case 2: case 3: case 4: case 5: case 6: case 7:
        // A whisper: no mute filter on this path.
        host.send_to_peer_00770b50(game.players[line.channel].peer_50, raw_message);
        return;
    case 8:
        send_to_unmuted_00770c50(session, game, raw_message, line.sender_xuid, nullptr, host);
        host.add_chat_line_005d0770(line, line.channel);
        return;
    case 9:
    case 10: {
        const std::int32_t team = line.channel - 9;
        send_to_team_unmuted_00770d40(session, game, team, raw_message, line.sender_xuid,
                                      nullptr, host);
        if (team_of_local == team) host.add_chat_line_005d0770(line, line.channel);
        return;
    }
    default:
        return;
    }
}

void receive_chat_007722e0(const SessionState& session, GameView& game, const ChatLine& line,
                           const void* raw_message, std::int32_t team_of_local, ChatHost& host) {
    if (session.mode == SessionMode::kHost) {
        host_relay_chat_00772020(session, game, line, raw_message, team_of_local, host);
        return;
    }
    if (host.chat_screen_present_00e198b4()) host.add_chat_line_005d0770(line, line.channel);
}

void receive_scene_name_0076e710(const std::string& name, std::uint32_t arg_28,
                                 std::uint32_t& built_count_f1af2c, SceneHost& host) {
    if (host.build_scene_record(name, arg_28)) ++built_count_f1af2c;
}

void receive_scene_index_007724a0(GameView& game, const std::string& scene_name,
                                  std::uint32_t index_20, std::uint32_t word_28,
                                  std::uint32_t word_2c, SceneHost& host) {
    std::string current;
    // Before the selection: a different scene un-readies the local player.
    if (host.selected_scene_name(current) && !host.names_equal_ci(current, scene_name)) {
        host.request_player_ready(game.local_player, 0, 1);
        host.leave_tactical_library_if_open();
    }
    host.select_scene_record_004c6890(index_20);
    if (host.selected_scene_name(current)) {
        if (!host.names_equal_ci(current, scene_name)) {
            // The client lacks the named scene: ask for the list (MT_REFRESHSCENE).
            host.set_client_scene_match_c4(0);
            host.send_to_server_00770af0(make_message(0x0A, kVtableRefreshScene0A));
        } else {
            host.set_client_scene_match_c4(1);
        }
        host.refresh_lobby_scene_005e5ae0();
    }
    game.scene_2060 = word_28;
    game.scene_2064 = word_2c;
}

void receive_remove_player_00773420(GameView& game, VoiceMuteTable& mute, std::int32_t player,
                                    RemovePlayerHost& host) {
    PlayerRecord& record = game.players[player];
    if (host.online_present_00f8a2fc()) {
        host.online_remove_remote_talker_b0(record.xuid_80);
        voice_mute_remove_00778690(mute, record.xuid_80);
    }
    if (record.peer_50 != nullptr) host.destroy_peer(record.peer_50);
    const std::int32_t slot = record.slot_20;
    if (slot > 0) {  // JLE at 007734B2: slot 0 is never restored here
        host.restore_mission_record_004bb660(slot);
        if (game.state == 0x0D || game.state == 0x0A || game.state == 0x10) {
            game.current[slot]->name_58 = record.name_58;
        }
    } else if (slot == -1) {
        if (PlayerRecord* named = host.find_slot_record_by_name_004bb690(record.name_58)) {
            named->ai_enabled_0a = 0;
        }
    }
    // [00F8A2FC] is not re-checked before vtable[15Ch].
    host.online_remove_player_15c(record.xuid_80, record.name_58);
    host.clear_player_record_004d0fc0(record);
}

void server_send_scene_scoring_0076d1b0(const SessionState& session, ScoringHost& host) {
    host.trace("__MMultiPlayer::ServerSendSceneScoring()");
    host.scoring_prepare_00905370();
    for (int row = 0; row < kScoringRowCount; ++row) host.scoring_send_row_0090abb0(row);
    host.flush_peer_buffers_007848f0(session.server_transport);
}

// ---------------------------------------------------------------------------
// Batch 3: MultiScore net table
// ---------------------------------------------------------------------------

namespace {
// 0043BDF0: sprintf "%f" of the float32 widened to double.
std::string format_float_0043bdf0(float value) {
    char buffer[64];
    std::snprintf(buffer, sizeof buffer, "%f", static_cast<double>(value));
    return buffer;
}
}  // namespace

std::string net_table_sync_lua_source_00781910(std::int32_t party,
                                               const NetTableEntry* entries, std::size_t count) {
    std::string text;
    for (std::size_t i = 0; i < count; ++i) {
        const NetTableEntry& entry = entries[i];
        text += "MultiScore";
        text += "[";
        text += std::to_string(party);  // 00BD1BB0 on the string builder
        text += "][";
        if (entry.key.is_string) {
            text += "\"" + entry.key.text + "\"";
        } else {
            // 004CACA0 decimal; the 32 wire bits as a signed int (assumption).
            text += std::to_string(static_cast<std::int32_t>(entry.key.number));
        }
        text += "]=";
        if (entry.value.is_string) {
            text += "\"" + entry.value.text + "\"";
        } else {
            text += format_float_0043bdf0(entry.value.number);
        }
        text += ";";
    }
    return text;
}

int NetTableLuaValue::lua_type() const noexcept {
    switch (kind) {
    case Kind::kNil: return 0;       // LUA_TNIL
    case Kind::kInteger:
    case Kind::kNumber: return 3;    // LUA_TNUMBER
    case Kind::kString: return 4;    // LUA_TSTRING
    case Kind::kOther: return -2;    // some other type: never equal to the above
    }
    return -2;
}

bool net_table_pair_changed_00782020(const NetTablePair& pair) noexcept {
    const NetTableLuaValue& now = pair.value;
    const NetTableLuaValue& old = pair.saved;
    if (old.lua_type() != now.lua_type()) return true;
    const bool numbers = now.lua_type() == 3;
    if (numbers && old.number != now.number) return true;  // FCOMP/FNSTSW: NaN differs
    const bool strings = now.lua_type() == 4;
    if (strings && old.text != now.text) return true;  // interned pointer compare
    return false;
}

bool net_table_entry_from_pair_00782020(const NetTablePair& pair, NetTableEntry& out) {
    out = NetTableEntry{};
    switch (pair.key.kind) {
    case NetTableLuaValue::Kind::kInteger:  // 007814E0
        out.key.is_string = false;
        out.key.number = static_cast<std::uint32_t>(static_cast<std::int32_t>(pair.key.number));
        break;
    case NetTableLuaValue::Kind::kString:   // 00781430
        out.key.is_string = true;
        out.key.text = pair.key.text;
        break;
    default:
        return false;
    }
    switch (pair.value.kind) {
    case NetTableLuaValue::Kind::kInteger:  // 008869F0: (float)int
        out.value.is_string = false;
        out.value.number = static_cast<float>(static_cast<std::int32_t>(pair.value.number));
        return true;
    case NetTableLuaValue::Kind::kString:   // 00886A10
        out.value.is_string = true;
        out.value.text = pair.value.text;
        return true;
    default:
        return false;  // 007823E6 -> 007824B7
    }
}

std::string net_table_snapshot_statement_00782020(std::int32_t party) {
    const std::string p = std::to_string(party);
    return "MultiScore_save[" + p + "]=copy_table(MultiScore[" + p + "])";
}

void net_table_sync_to_peer_00782020(void* peer, std::int32_t party, bool tables_present,
                                     const NetTablePair* pairs, std::size_t count,
                                     NetTableSyncHost& host) {
    if (peer == nullptr || party == 8 || !tables_present) return;
    std::vector<NetTableEntry> changes;
    for (std::size_t i = 0; i < count; ++i) {
        if (!net_table_pair_changed_00782020(pairs[i])) continue;
        NetTableEntry entry;
        if (net_table_entry_from_pair_00782020(pairs[i], entry)) changes.push_back(entry);
    }
    host.lua_do_buffer_00b65ec0(net_table_snapshot_statement_00782020(party));
    if (!changes.empty()) host.send_to_peer_00779fc0(peer, changes.data(), changes.size());
}

// ---------------------------------------------------------------------------
// Batch 4
// ---------------------------------------------------------------------------

void receive_add_player_00773120(SessionState& session, GameView& game,
                                 const AddPlayerMessage& msg, std::uint32_t game_734,
                                 std::uint32_t game_730, AddPlayerHost& host) {
    host.fill_player_record_004bb550(msg);
    PlayerRecord& record = game.players[msg.player_18];
    record.byte_90 = msg.byte_60;
    record.word_94 = msg.byte_61;
    const std::int32_t local_player = game.local_player;  // read at 007731D1
    if (msg.player_18 == 0) {
        record.peer_50 = host.client_first_peer();
    } else {
        void* target = host.create_peer_target();
        TargetSockAddr address;
        address.port_network = host.htons_00a4d494(kPeerVoicePort);
        if (host.online_uses_network_addresses_198()) {
            address.address = host.xnaddr_to_inaddr(msg.xnaddr_62);
            host.set_target_address_d74(target, address);
        }
        host.set_target_record_d50(target, record);
        record.peer_50 = target;
    }
    if (host.online_present_00f8a2fc()) {
        host.online_add_player_158(msg.xuid_20, msg.name_28, msg.byte_9e);
        host.online_add_remote_talker_ac(msg.xuid_20);
        voice_mute_add_00778620(session.mute, msg.xuid_20);
        host.online_refresh_28();
    }
    if (local_player == msg.player_18) {
        host.place_local_record_007f8690(record, game_734, game_730);
    }
}

void receive_conn_state_0076cd20(ConnStateRows& rows, const std::uint32_t (&float_bits)[8][3],
                                 const NativeTripletSmoothingContext& context) {
    for (int i = 0; i < 8; ++i) {
        update_native_smoothed_remainder_triplet_007862c0(rows.rows[i], float_bits[i][0],
                                                          float_bits[i][1], context);
    }
}

void receive_hint_00777820(const SessionState& session, const void* hint_payload_18,
                           HintHost& host) {
    if (session.mode != SessionMode::kHost) host.trigger_hint(hint_payload_18);
}

void client_check_scene_load_00772710(std::uint8_t& session_04, CheckSceneLoadHost& host) {
    const std::uint32_t result = host.online_scene_check_168();
    host.gui_005d6cd0();
    BuiltMessage m = make_message(0x0D, kVtableCheckSceneLoad0D);
    m.put8(0x18, result == 0 ? 1 : 0);  // SETZ BL at 00772746
    host.send_to_server_00770af0(m);
    session_04 = 0;
}

// ---------------------------------------------------------------------------
// Batch 5
// ---------------------------------------------------------------------------

SceneNameMessage build_scene_name_00770ff0(std::uint32_t index, const std::string& name,
                                           const std::string& second) {
    SceneNameMessage m;
    m.name_18 = name;
    m.second_20 = second;
    m.index_28 = index;
    return m;
}

SceneIndexMessage build_scene_index_00771190(std::uint32_t index, const std::string& name,
                                             std::uint32_t word_28, std::uint32_t word_2c) {
    SceneIndexMessage m;
    m.name_18 = name;
    m.index_20 = index;
    m.word_28 = word_28;
    m.word_2c = word_2c;
    return m;
}

void host_refresh_scene_007717a0(bool game_present, const SceneListView& scenes, void* peer,
                                 RefreshSceneHost& host) {
    if (!game_present) return;
    for (std::int32_t i = 0; i < scenes.count_5f0; ++i) {
        const SceneListView::Scene& scene = scenes.scenes[i];
        host.send_scene_name(peer, build_scene_name_00770ff0(static_cast<std::uint32_t>(i),
                                                             scene.name_90c, scene.second_920));
    }
    if (scenes.selected_5fc != nullptr) {
        host.send_scene_index(peer, build_scene_index_00771190(scenes.selected_index_60c,
            scenes.selected_5fc->name_90c, scenes.word_2060, scenes.word_2064));
    }
}

// ---------------------------------------------------------------------------
// Packet cc13-net2: ServerAddPlayer 007752D0 and its builders
// ---------------------------------------------------------------------------

namespace {

std::uint32_t byte_reverse(std::uint32_t v) noexcept {
    return (v >> 24) | ((v >> 8) & 0xFF00u) | ((v << 8) & 0xFF0000u) | (v << 24);
}

// "_ADD_PLAYER_ Server:%d.%d.%d.%d Peer:%d.%d.%d.%d" (00D03D14) and
// "__P2P_VOICE__ PeerTableToC S:..." (00D03CDC): the four bytes at peer+D78h
// in memory order, then the same dword byte-reversed (00775616..007757AA).
void trace_peer_address(ServerAddPlayerHost& host, const char* format, std::uint32_t a) {
    const unsigned m0 = a & 0xFFu, m1 = (a >> 8) & 0xFFu, m2 = (a >> 16) & 0xFFu, m3 = a >> 24;
    char text[128];
    std::snprintf(text, sizeof text, format, m0, m1, m2, m3, m3, m2, m1, m0);
    host.trace(text);
}

}  // namespace

void recount_session_slots_0076dbe0(SessionInfo& info, const PlayerRecord (&players)[8],
                                    const JoinInfo* const (&joins)[8]) noexcept {
    info.used_private_12a = 0;
    info.used_slots_128 = 0;
    for (int i = 0; i < 8; ++i) {  // unrolled four records per pass, stride 460h
        if (players[i].claimed_08 == 0) continue;
        ++info.used_slots_128;
        if (joins[i] != nullptr && joins[i]->private_slot_58 != 0) ++info.used_private_12a;
    }
}

float average_peer_quality_00786030(const PeerQualityRow (&rows)[8]) noexcept {
    std::uint32_t count = 0;
    float sum = 0.0f;
    for (int i = 1; i < 8; ++i) {
        if (rows[i].present_00 == 0) continue;
        ++count;
        sum = (i == 1) ? static_cast<float>(rows[i].value_0c + 0.0) : rows[i].value_0c + sum;
    }
    return count != 0 ? sum / static_cast<float>(count) : 1.0f;
}

AddPlayerOut build_add_player_for_record_0076b760(const PlayerRecord& record,
                                                  const ServerPeer& peer) noexcept {
    AddPlayerOut m;
    m.player_18 = record.owner_player_1c;
    m.xuid_20 = record.xuid_80;
    m.name_28 = record.name_58;
    m.tag_48 = record.tag_78;
    m.address_50 = byte_reverse(peer.address_d78);
    m.byte_54 = record.byte_88;
    m.byte_58 = record.voice_registered_89;
    m.kind_5c = peer.kind_vtable_04;
    m.rank_60 = record.byte_90;
    m.platform_61 = static_cast<std::uint8_t>(record.word_94);
    if (peer.join_08 != nullptr) {  // the image does not check (see the header)
        const JoinInfo& join = *peer.join_08;
        m.xnaddr_62 = join.xnaddr_5a;
        for (int i = 0; i < 4; ++i) m.words_86[i] = join.words_86[i];
        m.word_96 = join.word_7e;
        m.word_9a = join.word_82;
        m.private_9e = join.private_slot_58;
    }
    for (int i = 0; i < 3; ++i) m.player_info_a0[i] = record.player_info_e0[i];
    for (int i = 0; i < 4; ++i) m.join_words_b0[i] = record.join_words_d0[i];
    return m;
}

ServerInfoOut build_server_info_0076b670(const SessionInfo& info, float quality) noexcept {
    ServerInfoOut m;
    m.used_slots_18 = static_cast<std::uint8_t>(info.used_slots_128);
    m.used_private_19 = static_cast<std::uint8_t>(info.used_private_12a);
    // strncpy(+1Ah, session+100h, 20h) then +3Ah = 0.
    for (std::size_t i = 0; i < 0x20 && i < info.name_100.size(); ++i) {
        m.name_1a[i] = info.name_100[i];
    }
    m.name_1a[0x20] = '\0';
    m.byte_3b = info.byte_169;
    m.max_slots_3c = static_cast<std::uint8_t>(info.max_slots_127);
    m.max_private_3d = static_cast<std::uint8_t>(info.max_private_129);
    m.quality_40 = quality;
    m.rank_lo_44 = static_cast<std::uint8_t>(info.rank_lo_12c);
    m.rank_hi_45 = static_cast<std::uint8_t>(info.rank_hi_12d);
    m.word_48 = info.word_170;
    m.word_4c = info.word_174;
    for (int i = 0; i < 13; ++i) m.words_50[i] = info.words_130[i];
    return m;
}

void server_add_player_007752d0(ServerAddPlayerState& state, const LocalProfileView& profile,
                                ServerPeer& peer, ServerAddPlayerHost& host) {
    GameView& game = *state.game;
    SessionInfo& info = *state.info;
    auto is_local_join = [&]() {
        return host.server_first_peer() == peer.handle &&
               !(state.mode == SessionMode::kHost && info.byte_168 != 0);
    };

    // 007752EE..007754C7: claim the record.
    PlayerRecord* record = nullptr;
    if (is_local_join()) {
        const std::string& name = profile.name_6a0.empty() ? profile.name_68c : profile.name_6a0;
        const std::string tag = profile.tag_00f88a38 != nullptr ? profile.tag_00f88a38 : "";
        record = host.claim_participant_004bb440(0, peer.handle, name, tag, profile.xuid_698,
                                                 profile.byte_6a9, profile.byte_6a8,
                                                 peer.kind_vtable_04);
        record->byte_90 = host.profile_rank_00574050();
        game.local_player = record->owner_player_1c;  // game+18C8h = record+1Ch
    } else {
        const JoinInfo& join = *peer.join_08;
        host.post_template_chat_005cf320(8, 7, std::string(), join.name_00);
        record = host.claim_participant_004bb440(1, peer.handle, join.name_00, join.tag_20,
                                                 join.xuid_28, join.byte_59, join.byte_30,
                                                 peer.kind_vtable_04);
        record->byte_90 = join.rank_31;
    }
    peer.record_d50 = record;

    // 007754C9..00775552: the peer's XNet platform, 2 when it has no address.
    if (peer.address_d78 != 0) {
        std::uint32_t platform = 0;
        if (host.xnet_platform(peer.address_d78, platform) == 0) record->word_94 = platform;
    } else {
        record->word_94 = 2;
    }

    // 00775554..007755BC: MT_MYPLAYER {+18h player, +1Ch nonce hi, +20h nonce lo}.
    const std::uint64_t nonce = host.session_nonce_268();
    BuiltMessage my = make_message(0x25, kVtableMyPlayer25);
    my.put32(0x18, static_cast<std::uint32_t>(record->owner_player_1c));
    my.put32(0x1C, static_cast<std::uint32_t>(nonce >> 32));
    my.put32(0x20, static_cast<std::uint32_t>(nonce));
    host.send_to_peer(peer.handle, my);

    // 007755C1..00775885: every other claimed record to the new peer.
    for (int i = 0; i < 8; ++i) {
        PlayerRecord& other = game.players[i];
        if (&other == record || other.claimed_08 == 0) continue;
        const ServerPeer& other_peer = *state.record_peers[i];
        host.send_add_player(peer.handle, build_add_player_for_record_0076b760(other, other_peer));
        trace_peer_address(host, "_ADD_PLAYER_ Server:%d.%d.%d.%d Peer:%d.%d.%d.%d",
                           other_peer.address_d78);
        if (other.slot_20 < 0) continue;
        BuiltMessage select = make_message(0x19, kVtableSelectPlayer19);
        select.put32(0x18, static_cast<std::uint32_t>(i));
        select.put32(0x1C, static_cast<std::uint32_t>(other.slot_20));
        host.send_to_peer(peer.handle, select);
        if (other.ready_0c == 0) continue;
        BuiltMessage ready = make_message(0x1D, kVtableReadyPlayer1D);
        ready.put32(0x18, static_cast<std::uint32_t>(i));
        ready.put8(0x1C, 1);  // constants, not the record's +0Ch/+0Dh
        ready.put8(0x1D, 1);
        host.send_to_peer(peer.handle, ready);
    }

    // 0077588B..00775960: AI flags of the eight current records (game+18CCh).
    for (int j = 0; j < 8; ++j) {
        BuiltMessage ai = make_message(0x1E, kVtableAiEnable1E);
        ai.put32(0x18, static_cast<std::uint32_t>(j));
        ai.put8(0x1C, game.current[j]->ai_enabled_0a);
        host.send_to_peer(peer.handle, ai);
        BuiltMessage skill = make_message(0x1F, kVtableAiSkill1F);
        skill.put32(0x18, static_cast<std::uint32_t>(j));
        skill.put32(0x1C, game.current[j]->ai_skill_2c);
        host.send_to_peer(peer.handle, skill);
    }

    // 00775966..00775B1F: the new record to everyone.
    trace_peer_address(host, "__P2P_VOICE__ PeerTableToC S:%d.%d.%d.%d P:%d.%d.%d.%d",
                       peer.address_d78);
    host.broadcast_add_player(build_add_player_for_record_0076b760(*record, peer));

    // 00775B24..00775BF3: settings and the sub-scene to the new peer.
    host.send_multi_settings(peer.handle);
    host.send_subscene(peer.handle);

    // 00775BF8..00775D01: slot counters and MT_SERVERINFO to everyone.
    if (host.server_first_peer() != peer.handle) --info.will_12b;
    host.recount_slots_0076dbe0();
    char text[160];
    std::snprintf(text, sizeof text,
                  "MMultiPlayer::ServerAddPlayer: slot: %d/%d, private: %d/%d, will: %d  Rank:[%d,%d]",
                  info.used_slots_128, info.max_slots_127, info.used_private_12a,
                  info.max_private_129, info.will_12b, info.rank_lo_12c, info.rank_hi_12d);
    host.trace(text);
    host.broadcast_server_info(build_server_info_0076b670(info, host.average_peer_quality_00786030()));

    // 00775D06..00775DC9: pick an AI-held slot. Mode 7 takes the first one;
    // otherwise the first AI-held slot of the team (+28h 0 or 1) with fewer
    // human (not AI-held) current records, team 0 on a tie. Default slot 0;
    // -1 when that team has no AI-held slot.
    std::int32_t first_team0 = -1, first_team1 = -1, humans0 = 0, humans1 = 0, choice = 0;
    bool mode7_taken = false;
    for (int j = 0; j < 8; ++j) {
        const PlayerRecord& cur = *game.current[j];
        if (cur.ai_held_09 != 0) {
            if (host.effective_game_mode_004bca50() == 7) {
                choice = j;
                mode7_taken = true;
                break;
            }
            if (cur.team_28 == 0) {
                if (first_team0 < 0) first_team0 = j;
            } else if (cur.team_28 == 1 && first_team1 < 0) {
                first_team1 = j;
            }
        } else if (cur.team_28 == 0) {
            ++humans0;
        } else {
            ++humans1;
        }
    }
    (void)mode7_taken;  // the image re-reads 004BCA50 below either way
    if (host.effective_game_mode_004bca50() != 7) choice = humans1 < humans0 ? first_team1 : first_team0;
    host.host_select_player_0076c840(record->owner_player_1c, choice);

    // 00775DCE..00775E3E: the local join is placed and done.
    if (is_local_join()) {
        host.place_local_record_007f8690(*record, state.game_734, state.game_730);
        return;
    }

    // 00775E43..00775FE2: "Sending PIMPS to new client %d:".
    std::snprintf(text, sizeof text, "Sending PIMPS to new client %d:", record->owner_player_1c);
    host.trace(text);
    for (int i = 0; i < 8; ++i) {
        PlayerRecord& other = game.players[i];
        if (&other == record || other.claimed_08 == 0) continue;
        BuiltMessage info_msg = make_message(0xE3, kVtablePlayerInfoE3);
        info_msg.put32(0x18, other.player_info_e0[0]);
        info_msg.put32(0x1C, other.player_info_e0[1]);
        info_msg.put32(0x20, other.player_info_e0[2]);
        info_msg.put32(0x24, static_cast<std::uint32_t>(other.owner_player_1c));
        host.send_to_peer(record->peer_50, info_msg);
        std::snprintf(text, sizeof text, " player: %d pimp %d/%d", other.owner_player_1c,
                      static_cast<int>(other.player_info_e0[0]),
                      static_cast<int>(other.player_info_e0[1]));
        host.trace(text);
        BuiltMessage unlock = make_message(0xDD, kVtablePlayerUnlockDD);
        unlock.put32(0x18, static_cast<std::uint32_t>(i));
        unlock.put32(0x1C, other.unlock_class_110);
        host.send_to_peer(record->peer_50, unlock);
        std::snprintf(text, sizeof text, " player: %d unlock unit %d", other.owner_player_1c,
                      static_cast<int>(other.unlock_class_110));
        host.trace(text);
    }
}

// ---------------------------------------------------------------------------
// Packet cc13-net2: MT_SESSION_MANAGER 007746F0 and 004C9780
// ---------------------------------------------------------------------------

SessionManagerMessage build_session_manager_004c9780(std::uint8_t player,
                                                     std::uint8_t state) noexcept {
    SessionManagerMessage m;
    m.player_19 = player;
    m.state_18 = state;
    return m;
}

bool all_remote_players_in_state_0076c560(const PlayerRecord (&players)[8],
                                          std::int32_t local_player,
                                          std::uint32_t state) noexcept {
    for (const PlayerRecord& r : players) {
        if (r.owner_player_1c == local_player || r.claimed_08 == 0 || r.ai_held_09 != 0) continue;
        if (r.online_state_a4 != state) return false;
    }
    return true;
}

void receive_session_manager_007746f0(SessionManagerState& state,
                                      SessionManagerMessage& msg,
                                      SessionManagerHost& host) {
    char text[96];
    std::snprintf(text, sizeof text, " - - ONLINE - - SESSION_MANAGER PlayerIndex %d, State  %d",
                  msg.player_19, msg.state_18);
    host.trace(text);
    const auto player_state = [&]() -> std::uint32_t& {
        return (*state.players)[msg.player_19].online_state_a4;
    };
    const auto local_byte = [&]() { return static_cast<std::uint8_t>(state.local_player); };

    if (state.mode == SessionMode::kClient && msg.state_18 == 1 && state.stats_sent_04 == 0) {
        state.stats_sent_04 = 1;
        host.client_send_local_score_00772a40();
        host.send_to_server(build_session_manager_004c9780(local_byte(), 2));
        host.flush_peer_buffers_007848f0();
        return;
    }
    if (state.mode == SessionMode::kHost && msg.state_18 == 2) {
        player_state() = 2;
        if (all_remote_players_in_state_0076c560(
                *state.players, state.local_player, 2)) {
            host.debrief_00920a20();
            host.server_send_scene_scoring_0076d1b0();
            host.online_clear_74_and_call_184();
        }
        state.local_current->online_state_a4 = 4;
        return;
    }
    if (state.mode == SessionMode::kClient && msg.state_18 == 3) {
        const std::uint32_t owner = host.stats_owner_9cc();
        host.online_stats_clear_004c0fd0();
        for (OnlineStatsEntry& entry : msg.entries_1c) {
            entry.words[0] = owner;  // written into the message node first (007748FA)
            host.online_stats_push_back(entry);
        }
        const OnlineStatsEntry e = host.online_stats_front();
        const std::uint32_t args[12] = {owner, e.words[2], e.words[3], e.words[4], e.words[5],
                                        e.words[6], e.words[10], e.words[8], e.words[7], 0,
                                        e.words[11], e.words[12]};
        host.apply_stats_entry_004c04c0(args);
        host.after_stats_00626c50();
        return;
    }
    if (state.mode == SessionMode::kHost && msg.state_18 == 4) {
        player_state() = 4;
        if (!all_remote_players_in_state_0076c560(
                *state.players, state.local_player, 4)) {
            return;
        }
        host.online_call_58();
        host.broadcast(build_session_manager_004c9780(local_byte(), 5));
        return;
    }
    if (state.mode == SessionMode::kClient && msg.state_18 == 5) {
        host.online_call_58();
        host.send_to_server(build_session_manager_004c9780(local_byte(), 5));
    }
}

// ---------------------------------------------------------------------------
// Packet cc13-net2: presentation receivers 2Bh..34h
// ---------------------------------------------------------------------------

void receive_blackout_0076f4c0(const PresentationMessage& msg, PresentationHost& host) {
    if (!host.hud_present() || !host.narrative_screen_present_a4()) return;
    host.arm_blackout_fade_005b9ba0(msg.float_18, msg.float_1c);
}

void receive_countdown_007709c0(const PresentationMessage& msg, PresentationHost& host) {
    if (msg.stop_28 == 0) {
        if (host.countdown_present_21e8()) {
            host.countdown_start_00733ff0(msg.text_18, msg.float_20, msg.float_24);
        }
        host.hud_counter_show_0052b9b0(msg.text_18, msg.float_24);
        return;
    }
    if (host.countdown_present_21e8()) host.countdown_stop_007340a0();
    host.hud_counter_hide_0052ab90();
}

void receive_movcam_hand_targeted_0076d420(const PresentationMessage& msg,
                                           PresentationHost& host) {
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    void* entity = host.resolve_entity_id(static_cast<std::uint16_t>(msg.word_18));
    host.movcam_hand_mode_targeted_007a6b40(host.movie_camera_005cd160(), entity, msg.byte_1a,
                                            msg.byte_1b);
}

void receive_movcam_ref_polar_0076f560(const PresentationMessage& msg, PresentationHost& host) {
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    void* entity = host.resolve_entity_id(static_cast<std::uint16_t>(msg.word_18));
    host.movcam_ref_pos_polar_007a6e60(host.movie_camera_005cd160(), entity, msg.float_1c,
                                       msg.float_20, msg.float_24, msg.byte_28, msg.float_2c);
}

void receive_movcam_ref_radius_0076f630(const PresentationMessage& msg, PresentationHost& host) {
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    void* entity = host.resolve_entity_id(static_cast<std::uint16_t>(msg.word_18));
    host.movcam_ref_pos_radius_007a7070(host.movie_camera_005cd160(), entity, msg.float_1c,
                                        msg.byte_20, msg.float_24);
}

void receive_movcam_path_point_0076a870(const void* raw_message, PresentationHost& host) {
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    host.movcam_path_point_property_007a7c60(host.movie_camera_005cd160(), raw_message);
}

void receive_movcam_add_interpolator_0076a8a0(const PresentationMessage& msg,
                                              PresentationHost& host) {
    // The interpolator is allocated before the HUD check (0076A8B6) and is
    // leaked when the movie screen is absent.
    void* interpolator = host.new_interpolator_0078e370(msg.word_18, msg.float_1c, msg.float_20,
                                                        msg.float_24, msg.float_28);
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    host.movcam_add_interpolator_0079f6d0(host.movie_camera_005cd160(), interpolator);
}

void receive_movcam_deviation_0076a940(const PresentationMessage& msg, PresentationHost& host) {
    if (!host.hud_present() || !host.movie_screen_present_98()) return;
    host.movie_set_deviation_005caf90(msg.float_18);
}

void receive_movcam_new_add_position_0076d760(SessionState& session, PresentationMessage& msg,
                                              PresentationHost& host) {
    if (session.mode == SessionMode::kHost) return;
    ++session.movcam_count_284;
    const std::int32_t delta = msg.tick_0c - host.current_tick_00f876b0();
    const float seconds = static_cast<float>(static_cast<double>(delta) / 1000.0);
    if (!host.hud_present() || !host.new_movie_screen_present_9c()) return;
    host.movcam_new_add_position_007a2d30(host.new_movie_camera_005cd240(), msg.word_18, seconds);
    msg.byte_1c = 0;
}

void receive_movcam_blackbar_0076a970(const PresentationMessage& msg, PresentationHost& host) {
    if (!host.hud_present()) return;
    if (host.new_movie_screen_present_9c()) host.blackbar_9c(msg.byte_18 != 0);
    if (host.blackbar_screen_present_a0()) host.blackbar_a0(msg.byte_18 != 0);
}

void host_check_scene_load_007f7460(PlayerRecord& record, std::uint8_t flag_18) noexcept {
    record.scene_check_14 = flag_18 != 0 ? 1u : 0u;
}

// ---------------------------------------------------------------------------
// Packet cc13-net2 item 5 (cc13-net3): MultiScore net slots
// ---------------------------------------------------------------------------

void apply_net_table_sync_text_00780b90(const NetTableSyncText& text, NetTableSyncHost& host) {
    // 00780BD3..00780BE2: a null data pointer becomes 00F871AC (""); the length
    // is passed unchanged, so a null pointer with a non-zero length would read past
    // it. The host here passes an empty buffer in that case.
    const char* data = text.data_2c != nullptr ? text.data_2c : "";
    host.lua_do_buffer_00b65ec0(std::string(data, text.data_2c != nullptr ? text.length_28 : 0));
}

NetTableCreateMessage build_net_table_create_00780a70(const net_cc13::EntityCreateSource& entity,
                                                      net_cc13::EntityCreateHost& entity_host,
                                                      NetTableCreateHost& host) {
    NetTableCreateMessage m;
    // 00780A75..00780A81: PUSH 0 (with_name), entity, 0 (word), DFh; CALL 00766FC0.
    m.base = net_cc13::build_entity_create_00766fc0(kNetTableCreateMessageType, 0, entity, false,
                                                     entity_host);
    m.vtable = kNetTableCreateMessageVtable;                    // 00780A86
    m.base.byte_c4 = 0;                                         // 00780A8C
    // 00780A93..00780A9F: [[00E188A8]+18C8h], read only now, after the base constructor.
    m.base.word_10c = static_cast<std::uint32_t>(host.game_local_player_18c8());
    return m;
}

void send_net_table_create_00780ae0(const void* entity, const net_cc13::EntityCreateSource& source,
                                    net_cc13::EntityCreateHost& entity_host, void* peer,
                                    NetTableCreateHost& host) {
    // 00780AFC..00780B2C: the same construction inline on the stack (the
    // game+18C8h read at 00780B11/00780B26 also follows 00766FC0).
    NetTableCreateMessage m = build_net_table_create_00780a70(source, entity_host, host);
    host.send_to_peer_00779fc0(entity, peer, m);  // 00780B4D
}   // 00780B69: 007668E0 = the base's string/vector destructors

void* create_net_table_entity_007813b0(std::uint16_t id_1c, NetTableCreateHost& host) {
    void* object = host.allocate_zeroed_314();
    if (object == nullptr) return nullptr;  // 007813F4
    return host.construct_multiscore_00780db0(object, id_1c);
}

void write_online_stats_004c04c0(const StatsWriteArgs& args, StatsWriteHost& host) {
    if (host.write_blocked_22dd()) return;  // 004C04CD
    const std::uint64_t local_xuid = host.local_xuid_00a3eb00();  // read before the lookup
    bool listed = false;
    for (std::int32_t id : host.listed_ids_00e19458()) {
        if (id == args.id_5) listed = true;  // no early exit (004C05C9)
    }
    std::vector<StatsView> views;
    const bool ranked = static_cast<std::uint8_t>(args.ranked_7) != 0;
    auto int64_property = [](std::uint32_t id, std::int32_t value, std::uint32_t offset) {
        return StatsProperty{id, 2, static_cast<std::int64_t>(value), offset};  // CDQ
    };
    bool build = true;
    if (listed && host.online_ready_198()) {
        if (ranked) {
            StatsView view{0xFFFF0000u, {}};
            view.properties.push_back(StatsProperty{0x1000800Au, 1, args.value_10, 0x2270});
            view.properties.push_back(StatsProperty{0x1000800Bu, 1, args.value_9, 0x2288});
            views.push_back(view);
        } else if (args.xuid != local_xuid) {
            build = false;  // 004C0669 / 004C067A
        }
        if (build) {
            const std::uint32_t* row = kStatsViewTable[args.row_6];
            if (args.value_12 > 0)
                views.push_back(StatsView{row[ranked ? 1 : 0],
                                          {int64_property(row[2], args.value_12, 0x2258)}});
            if (args.value_13 > 0)
                views.push_back(StatsView{4, {int64_property(0x20000084u, args.value_13, 0x2228)}});
            if (args.value_8 > 0)
                views.push_back(StatsView{1, {int64_property(0x200000BCu, args.value_8, 0x2210)}});
        }
    } else if (args.value_12 != 0) {
        const bool low_id = static_cast<std::uint32_t>(args.id_5) - 1u <= 0x0Du;  // SETBE
        views.push_back(StatsView{low_id ? 2u : 3u,
                                  {int64_property(low_id ? 0x20000080u : 0x2000008Fu,
                                                  args.value_12, 0x2240)}});
    }

    // 004C07F9..004C0866.
    host.zero_overlapped_7038();
    const std::uint32_t mode = host.session_mode_1fe4();
    std::uint32_t result = args.ranked_7;
    if (!views.empty()) result = host.xsession_write_stats(args.session, args.xuid, views, mode != 0);
    if (result != 0x3E5u) host.x_get_overlapped_extended_error_null();  // ERROR_IO_PENDING
    if (mode != 0) host.set_write_blocked_22dd();
}

bool schedule_net_table_sync_00780a00(std::uint8_t& force_310, const void* entity,
                                      const void* holder, std::int32_t holder_tick_04,
                                      std::int32_t now_00f876b0, NetSyncQueue& queue) noexcept {
    if (force_310 == 0) {
        // 00780A0D..00780A2D: FILD of the int difference against
        // FLD1 / FDIVRP of the float 0.05f; skipped when elapsed <= 1/0.05f.
        const auto diff = static_cast<std::int32_t>(static_cast<std::uint32_t>(now_00f876b0) -
                                                    static_cast<std::uint32_t>(holder_tick_04));
        const double elapsed = static_cast<double>(diff);  // SUB wraps
        const double interval = 1.0 / static_cast<double>(0.05f);
        if (!(elapsed > interval)) return false;
    }
    if (queue.count >= static_cast<std::int32_t>(kNetSyncQueueCapacity)) return false;  // 00780A39
    queue.rows[queue.count] = NetSyncQueueRow{entity, holder, 0};
    ++queue.count;
    force_310 = 0;  // 00780A58
    return true;
}

}  // namespace bsp::mp_cc13
