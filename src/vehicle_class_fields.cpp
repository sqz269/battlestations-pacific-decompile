// Vehicle class descriptor field reader. See include/bsp/vehicle_class_fields.hpp
// and docs/VEHICLE_CLASS_FIELDS.md.
//
// Addresses: 00960230, 0087CA80, 006D0B80, 00700E40, 00749210, 0074D4A0,
// 007D1F70, 00831840.

#include "bsp/vehicle_class_fields.hpp"

#include <cstring>
#include <limits>

namespace bsp {
namespace {

constexpr std::uint32_t kNo = kVehicleClassFieldNoOffset;

// A number a Lua string converts to the way the wrapper's tonumber path does.
double string_to_number(const char* text) noexcept {
    if (text == nullptr) {
        return 0.0;
    }
    char* end = nullptr;
    const double value = std::strtod(text, &end);
    return end == text ? 0.0 : value;
}

double as_number(const VehicleClassLuaValue& value) noexcept {
    switch (value.kind) {
        case VehicleClassValueKind::Boolean:
            return value.boolean ? 1.0 : 0.0;
        case VehicleClassValueKind::Integer:
            return static_cast<double>(value.integer);
        case VehicleClassValueKind::Number:
            return value.number;
        case VehicleClassValueKind::String:
            return string_to_number(value.string);
        default:
            return 0.0;
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Conversions
// ---------------------------------------------------------------------------

float vehicle_class_number_or_00b66330(const VehicleClassLuaValue& value, float fallback) noexcept {
    return value.is_nil() ? fallback : static_cast<float>(as_number(value));
}

std::int32_t vehicle_class_integer_or_00b66380(const VehicleClassLuaValue& value,
                                               std::int32_t fallback) noexcept {
    return value.is_nil() ? fallback : static_cast<std::int32_t>(as_number(value));
}

bool vehicle_class_boolean_or_00b662f0(const VehicleClassLuaValue& value, bool fallback) noexcept {
    if (value.is_nil()) {
        return fallback;
    }
    if (value.kind == VehicleClassValueKind::Boolean) {
        return value.boolean;
    }
    return as_number(value) != 0.0;
}

const char* vehicle_class_string_or_00b685c0(const VehicleClassLuaValue& value,
                                             const char* fallback) noexcept {
    if (value.kind != VehicleClassValueKind::String || value.string == nullptr) {
        return fallback;
    }
    return value.string;
}

float vehicle_class_number_00b66270(const VehicleClassLuaValue& value) noexcept {
    return static_cast<float>(as_number(value));
}

std::int32_t vehicle_class_integer_00b66290(const VehicleClassLuaValue& value) noexcept {
    return static_cast<std::int32_t>(as_number(value));
}

bool vehicle_class_boolean_00b66250(const VehicleClassLuaValue& value) noexcept {
    if (value.kind == VehicleClassValueKind::Boolean) {
        return value.boolean;
    }
    return !value.is_nil() && as_number(value) != 0.0;
}

std::int32_t vehicle_class_spec_role_009606bc(const char* spec_role) noexcept {
    if (spec_role == nullptr) {
        return kVehicleClassSpecRoleDefault;
    }
    // 009606F7: strcmp against "FlyingControll", which resets the role to 0.
    if (std::strcmp(spec_role, "FlyingControll") == 0) {
        return kVehicleClassSpecRoleDefault;
    }
    // 00960716: the case-insensitive compare 00425850 against "BomberPilot".
    const char* left = spec_role;
    const char* right = "BomberPilot";
    while (*left != '\0' && *right != '\0') {
        const char a = (*left >= 'A' && *left <= 'Z') ? static_cast<char>(*left + 32) : *left;
        const char b = (*right >= 'A' && *right <= 'Z') ? static_cast<char>(*right + 32) : *right;
        if (a != b) {
            return kVehicleClassSpecRoleDefault;
        }
        ++left;
        ++right;
    }
    if (*left == '\0' && *right == '\0') {
        return kVehicleClassSpecRoleBomberPilot;
    }
    return kVehicleClassSpecRoleDefault;
}

std::int32_t vehicle_class_platform_slot_capacity_00961b6c(std::int32_t current_size,
                                                           std::int32_t slot_index) noexcept {
    // 00961B72: JL over the stored count, so the vector only grows, and it grows
    // to slot_index + 1 rather than by one.
    return slot_index < current_size ? current_size : slot_index + 1;
}

std::int32_t vehicle_class_flag_element_index_00962efa(std::int32_t lua_key) noexcept {
    return lua_key - 1;  // 00962EFA, SUB ESI,1
}

// ---------------------------------------------------------------------------
// Key schema
// ---------------------------------------------------------------------------

// 0087CA80, __thiscall(descriptor, LuaObject* row), RET 4. 00960230 calls it
// before its own first key, so these land in the descriptor's 0h..6Bh window.
const VehicleClassFieldSpec kVehicleClassSharedFieldSchema[] = {
    {"Name", "", VehicleClassFieldConversion::String, VehicleClassFieldOffsets::kName,
     0x0087CAB8, 0.0f, 0, nullptr, "duplicated into the descriptor's char* at +54h"},
    {"Unique", "", VehicleClassFieldConversion::Boolean, VehicleClassFieldOffsets::kUnique,
     0x0087CAFC, 0.0f, 0, nullptr, "byte; no shipped row carries the key"},
    {"Mesh", "", VehicleClassFieldConversion::StringOr, kNo, 0x0087CB44, 0.0f, 0, "",
     "the model path; every row has it, as a bare string or a Platform(...) call"},
    {"Comment", "", VehicleClassFieldConversion::StringOr, kNo, 0x0087CBDB, 0.0f, 0, "",
     "author note, kept for the debug name"},
    {"HP", "", VehicleClassFieldConversion::NumberOr, VehicleClassFieldOffsets::kHitPoints,
     0x0087CC72, 100.0f, 0, nullptr, "default 100.0f from 00CE3D08"},
    {"Armour", "", VehicleClassFieldConversion::NumberOr, VehicleClassFieldOffsets::kArmour,
     0x0087CCB4, 0.0f, 0, nullptr, "default 0.0f, FLDZ at 0087CCB9"},
    {"ExplosionType", "", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kExplosionType, 0x0087CCEE, 0.0f, 0, nullptr,
     "integer probe first; no shipped row carries the key"},
    {"Damage", "", VehicleClassFieldConversion::Table, kNo, 0x0087CD59, 0.0f, 0, nullptr,
     "walked only when it is a table"},
    {"Sections", ".Damage", VehicleClassFieldConversion::Table, kNo, 0x0087CD88, 0.0f, 0, nullptr,
     "iterated with IterFirst/IterNext"},
    {"MshCategory", ".Damage.Sections[]", VehicleClassFieldConversion::String, kNo, 0x0087CECC,
     0.0f, 0, nullptr, "the mesh group the section covers"},
    {"Index", ".Damage.Sections[]", VehicleClassFieldConversion::Number, kNo, 0x0087CF43, 0.0f, 0,
     nullptr, "written to the section's +4h and +8h"},
    {"FireEfx", ".Damage.Sections[]", VehicleClassFieldConversion::IntegerOr, kNo, 0x0087CF91,
     0.0f, 0, nullptr, "section +24h"},
    {"FailureChance", ".Damage.Sections[]", VehicleClassFieldConversion::NumberOr, kNo, 0x0087D109,
     0.0f, 0, nullptr, "section +28h; no shipped row carries the key"},
    {"FailureDamageThreshold", ".Damage.Sections[]", VehicleClassFieldConversion::NumberOr, kNo,
     0x0087D14F, 0.0f, 0, nullptr, "section +2Ch; no shipped row carries the key"},
    {"FakeExplosionEffects", "", VehicleClassFieldConversion::Table, kNo, 0x0087D229, 0.0f, 0,
     nullptr, "indexed by keys the reader formats with %d"},
    {"Emberkek", "", VehicleClassFieldConversion::Table, kNo, 0x0087D56D, 0.0f, 0, nullptr,
     "crew table; no shipped row carries the key"},
};

// 00960230, __thiscall(descriptor, LuaObject* row), RET 4, in body order.
const VehicleClassFieldSpec kVehicleClassBaseFieldSchema[] = {
    {"SMIcon", "", VehicleClassFieldConversion::String, VehicleClassFieldOffsets::kSmallMapIcon,
     0x0096027B, 0.0f, 0, "gui\\units\\b25.tga",
     "nil takes the same GUI load with the literal fallback"},
    {"Width", "", VehicleClassFieldConversion::Number, VehicleClassFieldOffsets::kWidth,
     0x00960354, 0.0f, 0, nullptr, "no nil test; a missing key stores 0"},
    {"Length", "", VehicleClassFieldConversion::Number, VehicleClassFieldOffsets::kLength,
     0x0096038B, 0.0f, 0, nullptr, "no nil test"},
    {"Height", "", VehicleClassFieldConversion::Number, VehicleClassFieldOffsets::kHeight,
     0x009603C2, 0.0f, 0, nullptr, "no nil test"},
    {"TotalHeight", "", VehicleClassFieldConversion::NumberOr,
     VehicleClassFieldOffsets::kTotalHeight, 0x009603F9, 0.0f, 0, nullptr,
     "default is the Height just stored, reloaded from +A8h at 009603FE"},
    {"Mass", "", VehicleClassFieldConversion::NumberOr, VehicleClassFieldOffsets::kMass,
     0x0096043A, 1.0f, 0, nullptr, "default 1.0f, FLD1 at 0096043F"},
    {"Catapult", "", VehicleClassFieldConversion::Table, kNo, 0x0096047A, 0.0f, 0, nullptr,
     "when it is not a table, +C0h becomes -1 and +C8h/+CCh become 0"},
    {"LaunchedClass", ".Catapult", VehicleClassFieldConversion::IntegerOr,
     VehicleClassFieldOffsets::kLaunchedClass, 0x009604AC, 0.0f, -1, nullptr, "default -1"},
    {"LaunchStock", ".Catapult", VehicleClassFieldConversion::IntegerOr,
     VehicleClassFieldOffsets::kLaunchStock, 0x009604EA, 0.0f, 0, nullptr, "default 0"},
    {"MaxLaunchedPlanes", ".Catapult", VehicleClassFieldConversion::IntegerOr,
     VehicleClassFieldOffsets::kMaxLaunchedPlanes, 0x00960527, 0.0f, 0, nullptr, "default 0"},
    {"Equipment", ".Catapult", VehicleClassFieldConversion::IntegerOr,
     VehicleClassFieldOffsets::kCatapultEquipment, 0x00960564, 0.0f, 0, nullptr,
     "only written inside the Catapult branch, so it keeps the constructor's 0 otherwise"},
    {"CockpitMesh", "", VehicleClassFieldConversion::StringOr,
     VehicleClassFieldOffsets::kCockpitMesh, 0x009605B7, 0.0f, 0, "",
     "loaded through 007188A0; no shipped row carries the key"},
    {"CockpitCameraPos", "", VehicleClassFieldConversion::Vector3,
     VehicleClassFieldOffsets::kCockpitCameraX, 0x00960659, 0.0f, 0, nullptr,
     "three floats through 00B680A0 into +84h..+8Ch"},
    {"SpecRole", "", VehicleClassFieldConversion::StringOr, VehicleClassFieldOffsets::kSpecRole,
     0x009606B7, 0.0f, 0, "", "see vehicle_class_spec_role_009606bc"},
    {"SoundEfx", "", VehicleClassFieldConversion::Table, kNo, 0x00960741, 0.0f, 0, nullptr,
     "nil skips every sound key below, jumping to 0096109A"},
    {"EngineSoundEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kEngineSounds, 0x0096076D, 0.0f, 0, nullptr,
     "appended to the vector at +E8h, only when the value is an integer"},
    {"ContFireEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kContinuousFireSounds, 0x0096082E, 0.0f, 0, nullptr,
     "appended to the vector at +ECh"},
    {"DeadMeatEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kDeadMeatSound, 0x009608EF, 0.0f, 0, nullptr, "single handle"},
    {"WindEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kWindSound, 0x009609DE, 0.0f, 0, nullptr, "single handle"},
    {"TooFastEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kTooFastSound, 0x00960ACD, 0.0f, 0, nullptr, "single handle"},
    {"HajoCsavarEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kPropellerSound, 0x00960BBC, 0.0f, 0, nullptr,
     "Hungarian for ship-propeller effect"},
    {"OrrHullamEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kBowWaveSound, 0x00960CAB, 0.0f, 0, nullptr,
     "Hungarian for bow-wave effect"},
    {"ElevatorEfx", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kElevatorSound, 0x00960D9A, 0.0f, 0, nullptr, "single handle"},
    {"SonarPing", ".SoundEfx", VehicleClassFieldConversion::SoundId,
     VehicleClassFieldOffsets::kSonarPingSound, 0x00960E89, 0.0f, 0, nullptr, "single handle"},
    {"Ambient", ".SoundEfx", VehicleClassFieldConversion::String,
     VehicleClassFieldOffsets::kAmbientSound, 0x00960F78, 0.0f, 0, nullptr,
     "a name, resolved through 00A83FD0 rather than an id"},
    {"Cost", "", VehicleClassFieldConversion::IntegerOr, VehicleClassFieldOffsets::kCost,
     0x009610BA, 0.0f, 0, nullptr, "default 0"},
    {"Platforms", "", VehicleClassFieldConversion::Table, kNo, 0x009610F6, 0.0f, 0, nullptr,
     "iterated; the integer key is the slot index into the vector at +94h"},
    {"PilotFires", ".Platforms[]", VehicleClassFieldConversion::Boolean, kNo, 0x009611CB, 0.0f, 0,
     nullptr, "platform +Ch, byte, cleared first"},
    {"ForwardAim", ".Platforms[]", VehicleClassFieldConversion::Number, kNo, 0x00961211, 0.0f, 0,
     nullptr, "platform +48h"},
    {"UseBayDoor", ".Platforms[]", VehicleClassFieldConversion::Boolean, kNo, 0x00961250, 0.0f, 0,
     nullptr, "platform +Dh"},
    {"MainPlatform", ".Platforms[]", VehicleClassFieldConversion::Boolean, kNo, 0x00961296, 0.0f,
     0, nullptr, "platform +Eh"},
    {"Name", ".Platforms[]", VehicleClassFieldConversion::String, kNo, 0x009612D9, 0.0f, 0,
     nullptr, "duplicated into platform +8Ch"},
    {"Gun", ".Platforms[]", VehicleClassFieldConversion::Table, kNo, 0x00961335, 0.0f, 0, nullptr,
     "a list of weapon class ids"},
    {"DefaultGun", ".Platforms[]", VehicleClassFieldConversion::Integer, kNo, 0x0096146A, 0.0f, 0,
     nullptr, "resolved to an object stored at platform +38h"},
    {"Windows", ".Platforms[]", VehicleClassFieldConversion::Table, kNo, 0x0096160D, 0.0f, 0,
     nullptr, "a list of firing arcs, each built by 007F6B10"},
    {"Nofire", ".Platforms[].Windows[]", VehicleClassFieldConversion::Boolean, kNo, 0x009616AC,
     0.0f, 0, nullptr, "arc is a blocked sector"},
    {"MinHorzAngle", ".Platforms[].Windows[]", VehicleClassFieldConversion::Number, kNo,
     0x009616F5, 0.0f, 0, nullptr, "radians; the shipped rows write DEG(x) calls"},
    {"MaxHorzAngle", ".Platforms[].Windows[]", VehicleClassFieldConversion::Number, kNo,
     0x00961735, 0.0f, 0, nullptr, "radians"},
    {"MinVertAngle", ".Platforms[].Windows[]", VehicleClassFieldConversion::Number, kNo,
     0x00961775, 0.0f, 0, nullptr, "radians"},
    {"MaxVertAngle", ".Platforms[].Windows[]", VehicleClassFieldConversion::Number, kNo,
     0x009617B5, 0.0f, 0, nullptr, "radians"},
    {"DirectorFollowPlatforms", ".Platforms[]", VehicleClassFieldConversion::Table, kNo,
     0x009618FC, 0.0f, 0, nullptr, "integer list; no shipped row carries the key"},
    {"RestAngles", ".Platforms[]", VehicleClassFieldConversion::Table, kNo, 0x00961A68, 0.0f, 0,
     nullptr, "index 1 into platform +94h, index 2 into +90h; +94h defaults to FLT_MAX"},
    {"GunDelayGroups", "", VehicleClassFieldConversion::Table, kNo, 0x00961C9C, 0.0f, 0, nullptr,
     "a list of lists of platform slot indices"},
    {"DefaultEquipment", "", VehicleClassFieldConversion::Integer,
     VehicleClassFieldOffsets::kDefaultEquipment, 0x00961F16, 0.0f, 0, nullptr,
     "cleared to 0 first, so a nil key means 0"},
    {"Equipments", "", VehicleClassFieldConversion::Table, kNo, 0x00961F57, 0.0f, 0, nullptr,
     "two levels; the built range lands in +12Ch/+130h"},
    {"Platform", ".Equipments[][]", VehicleClassFieldConversion::Integer, kNo, 0x009620C8, 0.0f, 0,
     nullptr, "the slot the entry equips"},
    {"Ammo", ".Equipments[][]", VehicleClassFieldConversion::Integer, kNo, 0x00962102, 0.0f, 0,
     nullptr, "ammunition class"},
    {"ReloadTime", ".Equipments[][]", VehicleClassFieldConversion::NumberOr, kNo, 0x0096213C, 0.0f,
     0, nullptr, "entry +Ch"},
    {"ReconModifier", "", VehicleClassFieldConversion::NumberOr,
     VehicleClassFieldOffsets::kReconModifier, 0x009623A9, 0.0f, 0, nullptr, "float at +B8h"},
    {"ReconClass", "", VehicleClassFieldConversion::Integer, VehicleClassFieldOffsets::kReconClass,
     0x009623F0, 0.0f, 0, nullptr, "resolved through 00808F90"},
    {"MovieCameraPositions", "", VehicleClassFieldConversion::Table, kNo, 0x009624A8, 0.0f, 0,
     nullptr, "builds the container at +BCh"},
    {"Positions", ".MovieCameraPositions", VehicleClassFieldConversion::Table, kNo, 0x0096253D,
     0.0f, 0, nullptr, "each entry is a 5Ch-byte camera record"},
    {"PureSnitt", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::BooleanOr, kNo,
     0x009625EC, 0.0f, 0, nullptr, "camera +58h"},
    {"Name", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::StringOr, kNo,
     0x0096263B, 0.0f, 0, "", "the attachment point"},
    {"MinHorzAngle", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::Number, kNo,
     0x009626EB, 0.0f, 0, nullptr, "read as a pair with MaxHorzAngle"},
    {"MaxHorzAngle", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::Number, kNo,
     0x0096270C, 0.0f, 0, nullptr, "both must be non-nil or neither is used"},
    {"MinVertAngle", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::Number, kNo,
     0x00962773, 0.0f, 0, nullptr, "read as a pair with MaxVertAngle"},
    {"MaxVertAngle", ".MovieCameraPositions.Positions[]", VehicleClassFieldConversion::Number, kNo,
     0x00962791, 0.0f, 0, nullptr, "both must be non-nil or neither is used"},
    {"SnittPositions", ".MovieCameraPositions", VehicleClassFieldConversion::Table, kNo,
     0x00962A3B, 0.0f, 0, nullptr,
     "string-keyed cut definitions; the reader clears +BCh at 00962D8F"},
    {"Repair", "", VehicleClassFieldConversion::Boolean, VehicleClassFieldOffsets::kRepair,
     0x00962DBC, 0.0f, 0, nullptr, "byte at +D0h"},
    {"Flags", "", VehicleClassFieldConversion::Table, kNo, 0x00962E4E, 0.0f, 0, nullptr,
     "one-based integer keys into the 14h-stride vector at +110h"},
    {"SizeHorizontal", ".Flags[]", VehicleClassFieldConversion::NumberOr, kNo, 0x00962F86, 2.5f, 0,
     nullptr, "default 2.5f from 00CF87C8"},
    {"SizeVertical", ".Flags[]", VehicleClassFieldConversion::NumberOr, kNo, 0x00962FC8, 1.5f, 0,
     nullptr, "default 1.5f from 00CE380C"},
    {"ForcedTexture", ".Flags[]", VehicleClassFieldConversion::StringOr, kNo, 0x0096300B, 0.0f, 0,
     "", "copied into the flag's string, then loaded when it differs from the empty default"},
};

// 00831840, the ship family's slot +8h override. It chains to 00960230 first,
// then reads these into the ship descriptor. Offsets are ship-descriptor offsets.
const VehicleClassFieldSpec kVehicleClassShipFieldSchema[] = {
    {"MaxRotAngle", "", VehicleClassFieldConversion::Number, 0x4F8, 0x00831882, 0.0f, 0, nullptr, "00831840"},
    {"MaxRotAngleChangeRatio", "", VehicleClassFieldConversion::Number, 0x4FC, 0x008318C4, 0.0f, 0, nullptr, "00831840"},
    {"MaxSpeed", "", VehicleClassFieldConversion::Number, 0x500, 0x00831903, 0.0f, 0, nullptr, "00831840"},
    {"MaxAccel", "", VehicleClassFieldConversion::Number, 0x504, 0x00831942, 0.0f, 0, nullptr, "00831840"},
    {"Retardation", "", VehicleClassFieldConversion::Number, 0x508, 0x00831981, 0.0f, 0, nullptr, "00831840"},
    {"CollisionMaxSpeedDamage", "", VehicleClassFieldConversion::NumberOr, 0x50C, 0x008319C0, 0.0f, 0, nullptr, "00831840"},
    {"KamikazeDamage", "", VehicleClassFieldConversion::NumberOr, 0x510, 0x00831A05, 0.0f, 0, nullptr, "00831840"},
    {"KamikazeBlastDamage", "", VehicleClassFieldConversion::NumberOr, 0x514, 0x00831A4A, 0.0f, 0, nullptr, "00831840"},
    {"KamikazeBlastRange", "", VehicleClassFieldConversion::NumberOr, 0x518, 0x00831A8F, 0.0f, 0, nullptr, "00831840"},
    {"RotorSpdTurnDiff", "", VehicleClassFieldConversion::NumberOr, 0x69C, 0x00831AD4, 0.0f, 0, nullptr, "00831840"},
    {"RotorSpd", "", VehicleClassFieldConversion::NumberOr, 0x6A0, 0x00831B19, 0.0f, 0, nullptr, "00831840"},
    {"DeathEfx", "", VehicleClassFieldConversion::SoundId, 0x55C, 0x00831BA9, 0.0f, 0, nullptr, "00831840"},
    {"ExplosionEfx", "", VehicleClassFieldConversion::SoundId, 0x51C, 0x00831CB7, 0.0f, 0, nullptr, "00831840"},
    {"UnderwaterArmour", "", VehicleClassFieldConversion::NumberOr, 0x6B4, 0x00831D7C, 0.0f, 0, nullptr, "00831840"},
    {"DamageThreshold", "", VehicleClassFieldConversion::NumberOr, 0x6B8, 0x00831DC4, 0.0f, 0, nullptr, "00831840"},
    {"CaptainCameraHeight", "", VehicleClassFieldConversion::Number, 0x538, 0x00831E0D, 0.0f, 0, nullptr, "00831840"},
    {"CameraDistanceFront", "", VehicleClassFieldConversion::Number, 0x53C, 0x00831E60, 0.0f, 0, nullptr, "00831840"},
    {"CameraDistanceSide", "", VehicleClassFieldConversion::Number, 0x540, 0x00831EC7, 0.0f, 0, nullptr, "00831840"},
    {"CameraDistanceVertical", "", VehicleClassFieldConversion::Number, 0x544, 0x00831F2E, 0.0f, 0, nullptr, "00831840"},
    {"CameraMinHeight", "", VehicleClassFieldConversion::Number, 0x548, 0x00831F95, 0.0f, 0, nullptr, "00831840"},
    {"DamageToDeath", "", VehicleClassFieldConversion::NumberOr, 0x54C, 0x00831FFC, 0.0f, 0, nullptr, "00831840"},
    {"TimeToDeath", "", VehicleClassFieldConversion::NumberOr, 0x550, 0x00832043, 0.0f, 0, nullptr, "00831840"},
    {"PumpTimeToEmpty", "", VehicleClassFieldConversion::NumberOr, 0x554, 0x0083208A, 0.0f, 0, nullptr, "00831840"},
    {"WaterForceMultiplier", "", VehicleClassFieldConversion::NumberOr, 0x558, 0x008320D1, 0.0f, 0, nullptr, "00831840"},
    {"BowParticle", "", VehicleClassFieldConversion::SoundId, 0x5AC, 0x00832114, 0.0f, 0, nullptr, "00831840"},
    {"BowWave", "", VehicleClassFieldConversion::SoundId, 0x630, 0x008321D8, 0.0f, 0, nullptr, "00831840"},
    {"WaveStern", "", VehicleClassFieldConversion::SoundId, 0x634, 0x0083229C, 0.0f, 0, nullptr, "00831840"},
    {"RotorParticle", "", VehicleClassFieldConversion::SoundId, 0x65C, 0x00832360, 0.0f, 0, nullptr, "00831840"},
    {"DamageSmoke", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x00832445, 0.0f, 0, nullptr, "00831840"},
    {"MaxNumber", "", VehicleClassFieldConversion::Integer, 0x660, 0x0083247A, 0.0f, 0, nullptr, "00831840"},
    {"Effect", "", VehicleClassFieldConversion::SoundId, 0x660, 0x008324C1, 0.0f, 0, nullptr, "00831840"},
    {"HoD", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x0083266C, 0.0f, 0, nullptr, "00831840"},
    {"Flag", "", VehicleClassFieldConversion::Integer, kVehicleClassFieldNoOffset, 0x0083270E, 0.0f, 0, nullptr, "00831840"},
    {"Smoke", "", VehicleClassFieldConversion::Integer, 0x8, 0x008327D5, 0.0f, 0, nullptr, "00831840"},
    {"Idle", "", VehicleClassFieldConversion::Integer, 0x8, 0x008328FE, 0.0f, 0, nullptr, "00831840"},
    {"ExplosionTypes", "", VehicleClassFieldConversion::SoundId, kVehicleClassFieldNoOffset, 0x00832A73, 0.0f, 0, nullptr, "00831840"},
    {"Smoke", "", VehicleClassFieldConversion::IntegerOr, 0x674, 0x00832B7A, 0.0f, 0, nullptr, "00831840"},
    {"Hull", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x00832D51, 0.0f, 0, nullptr, "00831840"},
    {"WaterLineRatio", "", VehicleClassFieldConversion::Number, 0x71C, 0x00832D86, 0.0f, 0, nullptr, "00831840"},
    {"Segments", "", VehicleClassFieldConversion::Number, 0x720, 0x00832DC8, 0.0f, 0, nullptr, "00831840"},
    {"Traffic", "", VehicleClassFieldConversion::Table, 0x4C, 0x00832E0A, 0.0f, 0, nullptr, "00831840"},
    {"pathID", "", VehicleClassFieldConversion::Integer, 0x4C, 0x00832F1C, 0.0f, 0, nullptr, "00831840"},
    {"Idle", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x00832FC7, 0.0f, 0, nullptr, "00831840"},
    {"posID", "", VehicleClassFieldConversion::Integer, 0x18, 0x008330C5, 0.0f, 0, nullptr, "00831840"},
    {"templates", "", VehicleClassFieldConversion::Number, 0x0, 0x00833107, 0.0f, 0, nullptr, "00831840"},
    {"anims", "", VehicleClassFieldConversion::Number, 0x0, 0x00833301, 0.0f, 0, nullptr, "00831840"},
    {"Camos", "", VehicleClassFieldConversion::Integer, 0x0, 0x00833501, 0.0f, 0, nullptr, "00831840"},
    {"TextureRemaps", "", VehicleClassFieldConversion::String, kVehicleClassFieldNoOffset, 0x0083368B, 0.0f, 0, nullptr, "00831840"},
    {"GunColor", "", VehicleClassFieldConversion::Table, 0x18, 0x0083394A, 0.0f, 0, nullptr, "00831840"},
    {"LSClassId", "", VehicleClassFieldConversion::Integer, 0x724, 0x00833A3A, 0.0f, 0, nullptr, "00831840"},
    {"LSPoints", "", VehicleClassFieldConversion::Table, 0x0, 0x00833A7C, 0.0f, 0, nullptr, "00831840"},
    {"LSReload", "", VehicleClassFieldConversion::Number, 0x78C, 0x00833AFE, 0.0f, 0, nullptr, "00831840"},
    {"LandingShip", "", VehicleClassFieldConversion::IntegerOr, 0x78C, 0x00833B6C, 0.0f, 0, nullptr, "00831840"},
    {"LandingShipAmount", "", VehicleClassFieldConversion::IntegerOr, 0x790, 0x00833BCB, 0.0f, 0, nullptr, "00831840"},
    {"LandingShipCoolDown", "", VehicleClassFieldConversion::IntegerOr, 0x794, 0x00833C0A, 0.0f, 0, nullptr, "00831840"},
    {"Fire", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x00833C49, 0.0f, 0, nullptr, "00831840"},
    {"FireDamagePerFireTick", "", VehicleClassFieldConversion::Number, 0x574, 0x00833C81, 0.0f, 0, nullptr, "00831840"},
    {"MaxTorpedoStock", "", VehicleClassFieldConversion::Integer, 0x7A0, 0x00833CE6, 0.0f, 0, nullptr, "00831840"},
    {"DamagedGFXRemove", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x00833D2A, 0.0f, 0, nullptr, "00831840"},
    {"Slots", "", VehicleClassFieldConversion::Integer, 0x8, 0x00833D74, 0.0f, 0, nullptr, "00831840"},
    {"Funnels", "", VehicleClassFieldConversion::Integer, 0x8, 0x00833EAB, 0.0f, 0, nullptr, "00831840"},
    {"Flags", "", VehicleClassFieldConversion::Integer, 0x8, 0x00833FD0, 0.0f, 0, nullptr, "00831840"},
    {"InnerExplosionEfx", "", VehicleClassFieldConversion::Integer, 0x8, 0x0083413C, 0.0f, 0, nullptr, "00831840"},
    {"StructuralDamageEfx", "", VehicleClassFieldConversion::Integer, kVehicleClassFieldNoOffset, 0x0083430D, 0.0f, 0, nullptr, "00831840"},
    {"HackShipRotationAdd", "", VehicleClassFieldConversion::NumberOr, 0x798, 0x00834476, 0.0f, 0, nullptr, "00831840"},
    {"HackShipHeightAdd", "", VehicleClassFieldConversion::NumberOr, 0x79C, 0x008344CE, 0.0f, 0, nullptr, "00831840"},
    {"CapturePower", "", VehicleClassFieldConversion::IntegerOr, 0x804, 0x00834526, 0.0f, 0, nullptr, "00831840"},
};

// 007D1F70, the plane family's override, likewise after chaining to 00960230.
const VehicleClassFieldSpec kVehicleClassPlaneFieldSchema[] = {
    {"ShortName", "", VehicleClassFieldConversion::StringOr, kVehicleClassFieldNoOffset, 0x007D1FB0, 0.0f, 0, nullptr, "007D1F70"},
    {"NumEngines", "", VehicleClassFieldConversion::Number, 0x140, 0x007D204D, 0.0f, 0, nullptr, "007D1F70"},
    {"JetEngines", "", VehicleClassFieldConversion::BooleanOr, 0x144, 0x007D208B, 0.0f, 0, nullptr, "007D1F70"},
    {"Accel", "", VehicleClassFieldConversion::Number, 0x164, 0x007D20C6, 0.0f, 0, nullptr, "007D1F70"},
    {"YDrag", "", VehicleClassFieldConversion::Number, 0x170, 0x007D2150, 0.0f, 0, nullptr, "007D1F70"},
    {"XDrag", "", VehicleClassFieldConversion::Number, 0x174, 0x007D2189, 0.0f, 0, nullptr, "007D1F70"},
    {"ExtRotAccel", "", VehicleClassFieldConversion::Number, 0x178, 0x007D21C2, 0.0f, 0, nullptr, "007D1F70"},
    {"StallRotAccel", "", VehicleClassFieldConversion::Number, 0x17C, 0x007D21FB, 0.0f, 0, nullptr, "007D1F70"},
    {"WaterRotAccel", "", VehicleClassFieldConversion::Number, 0x180, 0x007D2234, 0.0f, 0, nullptr, "007D1F70"},
    {"AirBrakeDrag", "", VehicleClassFieldConversion::Number, 0x1DC, 0x007D226D, 0.0f, 0, nullptr, "007D1F70"},
    {"WheelBrake", "", VehicleClassFieldConversion::Number, 0x1E0, 0x007D22A6, 0.0f, 0, nullptr, "007D1F70"},
    {"BombControlLimit", "", VehicleClassFieldConversion::Number, 0x15C, 0x007D22DF, 0.0f, 0, nullptr, "007D1F70"},
    {"BombDelay", "", VehicleClassFieldConversion::Number, 0x1F4, 0x007D2318, 0.0f, 0, nullptr, "007D1F70"},
    {"StallSpd", "", VehicleClassFieldConversion::Number, 0x184, 0x007D2351, 0.0f, 0, nullptr, "007D1F70"},
    {"MaxSpd", "", VehicleClassFieldConversion::Number, 0x188, 0x007D238A, 0.0f, 0, nullptr, "007D1F70"},
    {"TravelSpeed", "", VehicleClassFieldConversion::Number, 0x18C, 0x007D23C3, 0.0f, 0, nullptr, "007D1F70"},
    {"SwimHeight", "", VehicleClassFieldConversion::Number, 0x194, 0x007D2413, 0.0f, 0, nullptr, "007D1F70"},
    {"MinWaterSpd", "", VehicleClassFieldConversion::Number, 0x198, 0x007D244C, 0.0f, 0, nullptr, "007D1F70"},
    {"MaxWaterSpd", "", VehicleClassFieldConversion::Number, 0x19C, 0x007D2485, 0.0f, 0, nullptr, "007D1F70"},
    {"WaterDecel", "", VehicleClassFieldConversion::Number, 0x1A0, 0x007D24BE, 0.0f, 0, nullptr, "007D1F70"},
    {"WaterUnSpring", "", VehicleClassFieldConversion::Number, 0x1A4, 0x007D24F7, 0.0f, 0, nullptr, "007D1F70"},
    {"RollSpd", "", VehicleClassFieldConversion::Number, 0x1A8, 0x007D2530, 0.0f, 0, nullptr, "007D1F70"},
    {"PitchSpd", "", VehicleClassFieldConversion::Number, 0x1AC, 0x007D2569, 0.0f, 0, nullptr, "007D1F70"},
    {"YawSpd", "", VehicleClassFieldConversion::Number, 0x1B0, 0x007D25A2, 0.0f, 0, nullptr, "007D1F70"},
    {"TurnRollSpd", "", VehicleClassFieldConversion::Number, 0x1C8, 0x007D25DB, 0.0f, 0, nullptr, "007D1F70"},
    {"YawLimitAngle", "", VehicleClassFieldConversion::Number, 0x1CC, 0x007D2614, 0.0f, 0, nullptr, "007D1F70"},
    {"PitchLimitAngle", "", VehicleClassFieldConversion::Number, 0x1D0, 0x007D264D, 0.0f, 0, nullptr, "007D1F70"},
    {"YawRollRatio", "", VehicleClassFieldConversion::Number, 0x1B4, 0x007D2686, 0.0f, 0, nullptr, "007D1F70"},
    {"SlideRatio", "", VehicleClassFieldConversion::Number, 0x1B8, 0x007D26BF, 0.0f, 0, nullptr, "007D1F70"},
    {"RollAccel", "", VehicleClassFieldConversion::Number, 0x1BC, 0x007D26F8, 0.0f, 0, nullptr, "007D1F70"},
    {"PitchAccel", "", VehicleClassFieldConversion::Number, 0x1C0, 0x007D2731, 0.0f, 0, nullptr, "007D1F70"},
    {"YawAccel", "", VehicleClassFieldConversion::Number, 0x1C4, 0x007D276A, 0.0f, 0, nullptr, "007D1F70"},
    {"KameraMogotte", "", VehicleClassFieldConversion::NumberOr, 0x168, 0x007D27A3, 0.0f, 0, nullptr, "007D1F70"},
    {"KameraFolotte", "", VehicleClassFieldConversion::NumberOr, 0x16C, 0x007D27E6, 0.0f, 0, nullptr, "007D1F70"},
    {"DragPitchRatio", "", VehicleClassFieldConversion::Number, 0x1D4, 0x007D2829, 0.0f, 0, nullptr, "007D1F70"},
    {"NegativePitchRatio", "", VehicleClassFieldConversion::Number, 0x1D8, 0x007D2862, 0.0f, 0, nullptr, "007D1F70"},
    {"TurnRoll", "", VehicleClassFieldConversion::Number, 0x25C, 0x007D289B, 0.0f, 0, nullptr, "007D1F70"},
    {"TurnRollLeader", "", VehicleClassFieldConversion::Number, 0x260, 0x007D28D4, 0.0f, 0, nullptr, "007D1F70"},
    {"RollMaxforceLimit", "", VehicleClassFieldConversion::Number, 0x274, 0x007D290D, 0.0f, 0, nullptr, "007D1F70"},
    {"PitchMaxforceLimit", "", VehicleClassFieldConversion::Number, 0x278, 0x007D2946, 0.0f, 0, nullptr, "007D1F70"},
    {"TurnCircleRadius", "", VehicleClassFieldConversion::Number, 0x268, 0x007D297F, 0.0f, 0, nullptr, "007D1F70"},
    {"WheelHeight", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D29B8, 0.0f, 0, nullptr, "007D1F70"},
    {"GroundPitch", "", VehicleClassFieldConversion::Table, 0x1F8, 0x007D29E8, 0.0f, 0, nullptr, "007D1F70"},
    {"WheelHeight", "", VehicleClassFieldConversion::Number, 0x1FC, 0x007D2A65, 0.0f, 0, nullptr, "007D1F70"},
    {"GroundPitch", "", VehicleClassFieldConversion::Number, 0x200, 0x007D2A9E, 0.0f, 0, nullptr, "007D1F70"},
    {"WaterPitch", "", VehicleClassFieldConversion::Number, 0x204, 0x007D2AD7, 0.0f, 0, nullptr, "007D1F70"},
    {"GlideRate", "", VehicleClassFieldConversion::Number, 0x208, 0x007D2B10, 0.0f, 0, nullptr, "007D1F70"},
    {"CarrierBased", "", VehicleClassFieldConversion::Boolean, 0x160, 0x007D2B4C, 0.0f, 0, nullptr, "007D1F70"},
    {"DropAngle", "", VehicleClassFieldConversion::Number, 0x1F0, 0x007D2B93, 0.0f, 0, nullptr, "007D1F70"},
    {"KamikazeBulletClass", "", VehicleClassFieldConversion::Integer, 0x20C, 0x007D2BCA, 0.0f, 0, nullptr, "007D1F70"},
    {"Wreck", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D2C7A, 0.0f, 0, nullptr, "007D1F70"},
    {"Wreck", "", VehicleClassFieldConversion::String, 0x21C, 0x007D2CB4, 0.0f, 0, nullptr, "007D1F70"},
    {"ExplosionEfx", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D2D39, 0.0f, 0, nullptr, "007D1F70"},
    {"ExplosionEfx", "", VehicleClassFieldConversion::Integer, 0x214, 0x007D2D77, 0.0f, 0, nullptr, "007D1F70"},
    {"ShellsEfx", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D2E27, 0.0f, 0, nullptr, "007D1F70"},
    {"ShellsEfx", "", VehicleClassFieldConversion::Integer, 0x234, 0x007D2E65, 0.0f, 0, nullptr, "007D1F70"},
    {"EngineEfxes", "", VehicleClassFieldConversion::SoundId, kVehicleClassFieldNoOffset, 0x007D2F15, 0.0f, 0, nullptr, "007D1F70"},
    {"EngineFireEfx", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D3068, 0.0f, 0, nullptr, "007D1F70"},
    {"EngineFireEfx", "", VehicleClassFieldConversion::Integer, 0x220, 0x007D30A6, 0.0f, 0, nullptr, "007D1F70"},
    {"DamageSmokeEfx", "", VehicleClassFieldConversion::SoundId, kVehicleClassFieldNoOffset, 0x007D323E, 0.0f, 0, nullptr, "007D1F70"},
    {"WingTipEfx", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D3382, 0.0f, 0, nullptr, "007D1F70"},
    {"WingTipEfx", "", VehicleClassFieldConversion::Integer, 0x5A4, 0x007D33C0, 0.0f, 0, nullptr, "007D1F70"},
    {"BayDoor", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D3473, 0.0f, 0, nullptr, "007D1F70"},
    {"OpenAngle", "", VehicleClassFieldConversion::Number, 0x5A8, 0x007D34A5, 0.0f, 0, nullptr, "007D1F70"},
    {"ClosedAngle", "", VehicleClassFieldConversion::Number, 0x5AC, 0x007D34E1, 0.0f, 0, nullptr, "007D1F70"},
    {"TimeToOpen", "", VehicleClassFieldConversion::Number, 0x5B0, 0x007D351D, 0.0f, 0, nullptr, "007D1F70"},
    {"BowWaves", "", VehicleClassFieldConversion::Integer, kVehicleClassFieldNoOffset, 0x007D3589, 0.0f, 0, nullptr, "007D1F70"},
    {"ParaReload", "", VehicleClassFieldConversion::Number, 0x5B4, 0x007D371A, 0.0f, 0, nullptr, "007D1F70"},
    {"GearsPullTime", "", VehicleClassFieldConversion::NumberOr, 0x5F8, 0x007D37A4, 0.0f, 0, nullptr, "007D1F70"},
    {"PartAnims", "", VehicleClassFieldConversion::Table, kVehicleClassFieldNoOffset, 0x007D37E8, 0.0f, 0, nullptr, "007D1F70"},
    {"Gears", "", VehicleClassFieldConversion::Number, 0x5C8, 0x007D381A, 0.0f, 0, nullptr, "007D1F70"},
    {"Wings", "", VehicleClassFieldConversion::Number, 0x5D8, 0x007D3990, 0.0f, 0, nullptr, "007D1F70"},
    {"BayDoor", "", VehicleClassFieldConversion::Number, 0x5E8, 0x007D3B06, 0.0f, 0, nullptr, "007D1F70"},
    {"TurboTime", "", VehicleClassFieldConversion::NumberOr, 0x5FC, 0x007D3CB6, 0.0f, 0, nullptr, "007D1F70"},
    {"TurboRechargingTime", "", VehicleClassFieldConversion::NumberOr, 0x600, 0x007D3CF6, 0.0f, 0, nullptr, "007D1F70"},
    {"TurboStrength", "", VehicleClassFieldConversion::NumberOr, 0x604, 0x007D3D36, 0.0f, 0, nullptr, "007D1F70"},
    {"TurboControlLimit", "", VehicleClassFieldConversion::NumberOr, 0x608, 0x007D3D73, 0.0f, 0, nullptr, "007D1F70"},
};

// The three small overrides. 00749210 is the structure/fort family, 0074D4A0 and
// 00700E40 two smaller ones, 006D0B80 the runway carrier.
const VehicleClassFieldSpec kVehicleClassStructureFieldSchema[] = {
    {"ExplosionEfx", "", VehicleClassFieldConversion::SoundId, 0x13C, 0x00749247, 0.0f, 0, nullptr,
     "00749210"},
    {"SmokeFireEfx", "", VehicleClassFieldConversion::Table, 0x160, 0x007492F7, 0.0f, 0, nullptr,
     "00749210, an iterated list"},
    {"DamageSmokeEfx", "", VehicleClassFieldConversion::SoundId, kNo, 0x007493DA, 0.0f, 0, nullptr,
     "00749210"},
    {"SmokeFireChanceMul", "", VehicleClassFieldConversion::Number, 0x164, 0x007494B2, 0.0f, 0,
     nullptr, "00749210"},
    {"SmokeFireDurationMin", "", VehicleClassFieldConversion::Number, 0x168, 0x007494E2, 0.0f, 0,
     nullptr, "00749210"},
    {"SmokeFireDurationMax", "", VehicleClassFieldConversion::Number, 0x16C, 0x00749512, 0.0f, 0,
     nullptr, "00749210"},
    {"CapturePost", "", VehicleClassFieldConversion::BooleanOr, 0x174, 0x00749542, 0.0f, 0,
     nullptr, "00749210, byte"},
    {"SecondaryExplosionEfx", "", VehicleClassFieldConversion::Table, kNo, 0x00749577, 0.0f, 0,
     nullptr, "00749210, an iterated list"},
    {"SecondaryExplosionChanceMul", "", VehicleClassFieldConversion::Number, 0x170, 0x0074963A,
     0.0f, 0, nullptr, "00749210"},
    {"FakedType", "", VehicleClassFieldConversion::Table, 0x178, 0x0074966A, 0.0f, 0, nullptr,
     "00749210; no shipped row carries the key"},
    {"MapIcon", "", VehicleClassFieldConversion::IntegerOr, 0x17C, 0x0074969F, 0.0f, 0, nullptr,
     "00749210"},
    {"Fortress", "", VehicleClassFieldConversion::BooleanOr, 0x175, 0x007496D4, 0.0f, 0, nullptr,
     "00749210, byte"},
    {"DamageModelChangeDelay", "", VehicleClassFieldConversion::NumberOr, 0x138, 0x00749709, 0.0f,
     0, nullptr, "00749210"},
    {"ExplosionEfx", "", VehicleClassFieldConversion::SoundId, 0x138, 0x0074D4D4, 0.0f, 0, nullptr,
     "0074D4A0"},
    {"Wreck", "", VehicleClassFieldConversion::String, 0x13C, 0x0074D579, 0.0f, 0, nullptr,
     "0074D4A0, a model path loaded through 004B3F10"},
    {"Smoke", "", VehicleClassFieldConversion::SoundId, 0x14C, 0x0074D62A, 0.0f, 0, nullptr,
     "0074D4A0"},
    {"ExplosionEfx", "", VehicleClassFieldConversion::SoundId, 0x140, 0x00700E74, 0.0f, 0, nullptr,
     "00700E40"},
    {"OpenTime", "", VehicleClassFieldConversion::Number, kNo, 0x00700F22, 0.0f, 0, nullptr,
     "00700E40, read through 00B66FA0"},
    {"SlowingFactor", "", VehicleClassFieldConversion::Number, kNo, 0x00700F45, 0.0f, 0, nullptr,
     "00700E40, read through 00B66FA0"},
    {"RunwayWidth", "", VehicleClassFieldConversion::Number, 0x138, 0x006D0BB2, 0.0f, 0, nullptr,
     "006D0B80"},
    {"RunwayLength", "", VehicleClassFieldConversion::Number, 0x13C, 0x006D0BE9, 0.0f, 0, nullptr,
     "006D0B80"},
};

std::size_t vehicle_class_shared_field_count() noexcept {
    return sizeof(kVehicleClassSharedFieldSchema) / sizeof(kVehicleClassSharedFieldSchema[0]);
}

std::size_t vehicle_class_base_field_count() noexcept {
    return sizeof(kVehicleClassBaseFieldSchema) / sizeof(kVehicleClassBaseFieldSchema[0]);
}

std::size_t vehicle_class_ship_field_count() noexcept {
    return sizeof(kVehicleClassShipFieldSchema) / sizeof(kVehicleClassShipFieldSchema[0]);
}

std::size_t vehicle_class_plane_field_count() noexcept {
    return sizeof(kVehicleClassPlaneFieldSchema) / sizeof(kVehicleClassPlaneFieldSchema[0]);
}

std::size_t vehicle_class_structure_field_count() noexcept {
    return sizeof(kVehicleClassStructureFieldSchema) / sizeof(kVehicleClassStructureFieldSchema[0]);
}

const VehicleClassFieldSpec* vehicle_class_find_field(const VehicleClassFieldSpec* schema,
                                                      std::size_t count,
                                                      const char* container,
                                                      const char* key) noexcept {
    if (schema == nullptr || container == nullptr || key == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < count; ++i) {
        if (std::strcmp(schema[i].container, container) == 0 &&
            std::strcmp(schema[i].key, key) == 0) {
            return &schema[i];
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

const VehicleClassKeyGap kVehicleClassUnprovidedKeys[] = {
    {"", "CockpitMesh", 0, "00960230; the cockpit mesh load never runs on the shipped data"},
    {"", "Unique", 0, "0087CA80"},
    {"", "ExplosionType", 0, "0087CA80; the rows use ExplosionEfx on the leaf readers instead"},
    {"", "Emberkek", 0, "0087CA80"},
    {".Damage.Sections[]", "FailureChance", 0, "0087CA80"},
    {".Damage.Sections[]", "FailureDamageThreshold", 0, "0087CA80"},
    {".Platforms[]", "DirectorFollowPlatforms", 0, "00960230"},
    {"", "FakedType", 0, "00749210"},
    {"", "GearsPullTime", 0, "007D1F70; the default keeps ParaReload's value"},
    {"", "DamageToDeath", 0, "00831840"},
    {"", "RotorSpd", 0, "00831840"},
    {"", "RotorSpdTurnDiff", 0, "00831840"},
    {"", "LSClassId", 0, "00831840, the landing-ship block"},
    {"", "LSPoints", 0, "00831840, the landing-ship block"},
    {"", "LSReload", 0, "00831840, the landing-ship block"},
    {"", "Fire", 0, "00831840"},
    {".Fire", "FireDamagePerFireTick", 0, "00831840"},
};

const VehicleClassKeyGap kVehicleClassUnconsumedKeys[] = {
    {"", "Type", 633, "read by the factory 00964790, not by the field reader"},
    {"", "Race", 232, "read by the factory 00964790"},
    {"", "ArmorIndexes", 550, "no reader in this packet touches it"},
    {"", "Damage.Sections[].HPBlack", 1382, "the section reader skips it"},
    {"", "HP_Realistic", 595, "a difficulty variant with no reader here"},
    {"", "MaxSpeed_Realistic", 154, "a difficulty variant with no reader here"},
    {"", "MaxTorpedoStock_Realistic", 71, "a difficulty variant with no reader here"},
    {"", "PlatformDirections", 139, "four sub-tables, none read here"},
    {"", "UnitlibViewDistance", 133, "unit library presentation"},
    {"", "Unlock", 101, "progression, read by the unlock predicate"},
    {"", "UnlockID", 63, "progression"},
    {"", "SmokeScreen", 47, "with four SmokeScreen* siblings"},
    {"", "DogFightAimDistance", 70, "with DogFightShootDistance and the three Strafe* keys"},
    {"", "TurnRollLowLimit", 72, "the only TurnRoll* key the plane reader skips"},
    {"", "DeckCamera", 25, "three sub-keys, none read here"},
    {"", "SubType", 16, "a second type string with no reader here"},
    {".HoD[]", "smoke", 1, "one row spells the key in lower case; the reader asks for Smoke"},
};

std::size_t vehicle_class_unprovided_key_count() noexcept {
    return sizeof(kVehicleClassUnprovidedKeys) / sizeof(kVehicleClassUnprovidedKeys[0]);
}

std::size_t vehicle_class_unconsumed_key_count() noexcept {
    return sizeof(kVehicleClassUnconsumedKeys) / sizeof(kVehicleClassUnconsumedKeys[0]);
}

// ---------------------------------------------------------------------------
// The reader sequence
// ---------------------------------------------------------------------------

namespace {

// A scoped ref: the native releases every object it gets, and the SEH unwind
// states 00960230 records exist to do the same on a throw.
class ScopedRef {
public:
    ScopedRef(VehicleClassFieldHost& host, GuiLuaRef ref) : host_(host), ref_(ref) {}
    ~ScopedRef() { host_.release(ref_); }
    ScopedRef(const ScopedRef&) = delete;
    ScopedRef& operator=(const ScopedRef&) = delete;
    const GuiLuaRef& get() const noexcept { return ref_; }

private:
    VehicleClassFieldHost& host_;
    GuiLuaRef ref_;
};

VehicleClassLuaValue field(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key) {
    ScopedRef value(host, host.get_by_name(table, key));
    return host.inspect(value.get());
}

float number_field(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key) {
    return vehicle_class_number_00b66270(field(host, table, key));
}

float number_field_or(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key,
                      float fallback) {
    return vehicle_class_number_or_00b66330(field(host, table, key), fallback);
}

std::int32_t integer_field_or(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key,
                              std::int32_t fallback) {
    return vehicle_class_integer_or_00b66380(field(host, table, key), fallback);
}

bool boolean_field(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key) {
    return vehicle_class_boolean_00b66250(field(host, table, key));
}

std::string string_field_or(VehicleClassFieldHost& host, const GuiLuaRef& table, const char* key,
                            const char* fallback) {
    const VehicleClassLuaValue value = field(host, table, key);
    const char* text = vehicle_class_string_or_00b685c0(value, fallback);
    return text == nullptr ? std::string() : std::string(text);
}

// 0096076D..00960F65. Each sound key is probed with IsInteger and only read when
// the probe holds, so a string or a table leaves the slot alone.
void read_sound_id(VehicleClassFieldHost& host, const GuiLuaRef& sounds, const char* key,
                   std::int32_t& slot) {
    const VehicleClassLuaValue value = field(host, sounds, key);
    if (value.is_integer()) {
        slot = vehicle_class_integer_00b66290(value);
    }
}

void read_sound_list(VehicleClassFieldHost& host, const GuiLuaRef& sounds, const char* key,
                     std::vector<std::int32_t>& out) {
    const VehicleClassLuaValue value = field(host, sounds, key);
    if (value.is_integer()) {
        out.push_back(vehicle_class_integer_00b66290(value));
    }
}

// 00961335 and 009618FC: a flat list of integers under an iterated table.
void read_integer_list(VehicleClassFieldHost& host, const GuiLuaRef& table,
                       std::vector<std::int32_t>& out) {
    GuiLuaRef key;
    GuiLuaRef value;
    for (bool more = host.iterate_first(table, key, value); more;
         more = host.iterate_next(table, key, value)) {
        out.push_back(vehicle_class_integer_00b66290(host.inspect(value)));
    }
}

// 0096160D..009618E3.
void read_windows(VehicleClassFieldHost& host, const GuiLuaRef& windows,
                  VehicleClassPlatformFields& platform) {
    GuiLuaRef key;
    GuiLuaRef value;
    for (bool more = host.iterate_first(windows, key, value); more;
         more = host.iterate_next(windows, key, value)) {
        VehicleClassPlatformFields::Window window;
        window.no_fire = boolean_field(host, value, "Nofire");
        window.min_horz = number_field(host, value, "MinHorzAngle");
        window.max_horz = number_field(host, value, "MaxHorzAngle");
        window.min_vert = number_field(host, value, "MinVertAngle");
        window.max_vert = number_field(host, value, "MaxVertAngle");
        platform.windows.push_back(window);
    }
}

// 00961A3C..00961B4F. The two defaults are set before the key is fetched.
void read_rest_angles(VehicleClassFieldHost& host, const GuiLuaRef& row,
                      VehicleClassPlatformFields& platform) {
    platform.rest_angle_a = std::numeric_limits<float>::max();  // 00D7A248
    platform.rest_angle_b = 0.0f;
    ScopedRef angles(host, host.get_by_name(row, "RestAngles"));
    if (host.inspect(angles.get()).is_nil()) {
        return;
    }
    ScopedRef first(host, host.get_by_index(angles.get(), 1));
    platform.rest_angle_a = vehicle_class_number_00b66270(host.inspect(first.get()));
    ScopedRef second(host, host.get_by_index(angles.get(), 2));
    const VehicleClassLuaValue value = host.inspect(second.get());
    if (!value.is_nil()) {
        platform.rest_angle_b = vehicle_class_number_00b66270(value);
    }
}

void read_platform(VehicleClassFieldHost& host, const GuiLuaRef& entry, std::int32_t slot_index,
                   VehicleClassPlatformFields& platform) {
    platform.slot_index = slot_index;
    platform.pilot_fires = boolean_field(host, entry, "PilotFires");
    platform.forward_aim = number_field(host, entry, "ForwardAim");
    platform.use_bay_door = boolean_field(host, entry, "UseBayDoor");
    platform.main_platform = boolean_field(host, entry, "MainPlatform");
    platform.name = string_field_or(host, entry, "Name", "");

    ScopedRef guns(host, host.get_by_name(entry, "Gun"));
    if (!host.inspect(guns.get()).is_nil()) {
        read_integer_list(host, guns.get(), platform.guns);
    }
    const VehicleClassLuaValue default_gun = field(host, entry, "DefaultGun");
    if (!default_gun.is_nil()) {
        platform.default_gun = vehicle_class_integer_00b66290(default_gun);
    }
    ScopedRef windows(host, host.get_by_name(entry, "Windows"));
    if (!host.inspect(windows.get()).is_nil()) {
        read_windows(host, windows.get(), platform);
    }
    ScopedRef follows(host, host.get_by_name(entry, "DirectorFollowPlatforms"));
    if (!host.inspect(follows.get()).is_nil()) {
        read_integer_list(host, follows.get(), platform.director_follows);
    }
    read_rest_angles(host, entry, platform);
}

}  // namespace

void read_vehicle_class_base_fields_00960230(VehicleClassFieldHost& host, const GuiLuaRef& row,
                                             VehicleClassBaseFields& out) {
    // 0096026C: SMIcon, with the literal fallback taking the same GUI load.
    out.small_map_icon_path = string_field_or(host, row, "SMIcon", "gui\\units\\b25.tga");
    out.small_map_icon = host.load_small_map_icon(out.small_map_icon_path.c_str());

    out.width = number_field(host, row, "Width");
    out.length = number_field(host, row, "Length");
    out.height = number_field(host, row, "Height");
    out.total_height = number_field_or(host, row, "TotalHeight", out.height);
    out.mass = number_field_or(host, row, "Mass", 1.0f);

    // 0096046B: the Catapult block, or the three resets at 00960592.
    ScopedRef catapult(host, host.get_by_name(row, "Catapult"));
    if (host.inspect(catapult.get()).is_table()) {
        out.has_catapult = true;
        out.launched_class = integer_field_or(host, catapult.get(), "LaunchedClass", -1);
        out.launch_stock = integer_field_or(host, catapult.get(), "LaunchStock", 0);
        out.max_launched_planes = integer_field_or(host, catapult.get(), "MaxLaunchedPlanes", 0);
        out.catapult_equipment = integer_field_or(host, catapult.get(), "Equipment", 0);
    } else {
        out.launched_class = -1;
        out.launch_stock = 0;
        out.max_launched_planes = 0;
    }

    const std::string cockpit_mesh = string_field_or(host, row, "CockpitMesh", "");
    if (!cockpit_mesh.empty()) {
        out.cockpit_mesh = host.load_cockpit_mesh(cockpit_mesh.c_str());
    }
    // 00960659: three floats out of a Lua vector object; a missing key leaves zero.
    {
        ScopedRef position(host, host.get_by_name(row, "CockpitCameraPos"));
        const VehicleClassLuaValue value = host.inspect(position.get());
        if (value.is_table()) {
            for (int i = 0; i < 3; ++i) {
                ScopedRef component(host, host.get_by_index(position.get(), i + 1));
                out.cockpit_camera[i] =
                    vehicle_class_number_00b66270(host.inspect(component.get()));
            }
        }
    }
    out.spec_role =
        vehicle_class_spec_role_009606bc(string_field_or(host, row, "SpecRole", "").c_str());

    // 00960735: every sound key hangs off the SoundEfx sub-table, and a nil
    // SoundEfx skips the whole block.
    ScopedRef sounds(host, host.get_by_name(row, "SoundEfx"));
    if (!host.inspect(sounds.get()).is_nil()) {
        read_sound_list(host, sounds.get(), "EngineSoundEfx", out.sounds.engine);
        read_sound_list(host, sounds.get(), "ContFireEfx", out.sounds.continuous_fire);
        read_sound_id(host, sounds.get(), "DeadMeatEfx", out.sounds.dead_meat);
        read_sound_id(host, sounds.get(), "WindEfx", out.sounds.wind);
        read_sound_id(host, sounds.get(), "TooFastEfx", out.sounds.too_fast);
        read_sound_id(host, sounds.get(), "HajoCsavarEfx", out.sounds.propeller);
        read_sound_id(host, sounds.get(), "OrrHullamEfx", out.sounds.bow_wave);
        read_sound_id(host, sounds.get(), "ElevatorEfx", out.sounds.elevator);
        read_sound_id(host, sounds.get(), "SonarPing", out.sounds.sonar_ping);
        out.sounds.ambient = string_field_or(host, sounds.get(), "Ambient", "");
        if (!out.sounds.ambient.empty()) {
            host.load_ambient_sound(out.sounds.ambient.c_str());
        }
    }

    out.cost = integer_field_or(host, row, "Cost", 0);

    // 009610E7: the Platforms table, keyed by slot index rather than appended.
    ScopedRef platforms(host, host.get_by_name(row, "Platforms"));
    if (!host.inspect(platforms.get()).is_nil()) {
        GuiLuaRef key;
        GuiLuaRef value;
        std::int32_t reserved = 0;
        for (bool more = host.iterate_first(platforms.get(), key, value); more;
             more = host.iterate_next(platforms.get(), key, value)) {
            const std::int32_t slot = vehicle_class_integer_00b66290(host.inspect(key));
            VehicleClassPlatformFields platform;
            read_platform(host, value, slot, platform);
            reserved = vehicle_class_platform_slot_capacity_00961b6c(reserved, slot);
            host.reserve_platform_slots(reserved);
            out.platforms.push_back(platform);
        }
    }

    // 00961C90: a list of lists of platform slot indices.
    ScopedRef delay_groups(host, host.get_by_name(row, "GunDelayGroups"));
    if (!host.inspect(delay_groups.get()).is_nil()) {
        GuiLuaRef key;
        GuiLuaRef value;
        for (bool more = host.iterate_first(delay_groups.get(), key, value); more;
             more = host.iterate_next(delay_groups.get(), key, value)) {
            std::vector<std::int32_t> group;
            read_integer_list(host, value, group);
            out.gun_delay_groups.push_back(group);
        }
    }

    // 00961F16: cleared to zero first, so a nil key leaves zero.
    out.default_equipment = 0;
    const VehicleClassLuaValue default_equipment = field(host, row, "DefaultEquipment");
    if (!default_equipment.is_nil()) {
        out.default_equipment = vehicle_class_integer_00b66290(default_equipment);
    }

    // 00961F57: two levels, an outer list of loadouts and an inner list of entries.
    ScopedRef equipments(host, host.get_by_name(row, "Equipments"));
    if (!host.inspect(equipments.get()).is_nil()) {
        GuiLuaRef outer_key;
        GuiLuaRef outer_value;
        for (bool more = host.iterate_first(equipments.get(), outer_key, outer_value); more;
             more = host.iterate_next(equipments.get(), outer_key, outer_value)) {
            std::vector<VehicleClassEquipmentFields> loadout;
            GuiLuaRef key;
            GuiLuaRef value;
            for (bool inner = host.iterate_first(outer_value, key, value); inner;
                 inner = host.iterate_next(outer_value, key, value)) {
                VehicleClassEquipmentFields entry;
                entry.platform = vehicle_class_integer_00b66290(field(host, value, "Platform"));
                entry.ammo = vehicle_class_integer_00b66290(field(host, value, "Ammo"));
                entry.reload_time = number_field_or(host, value, "ReloadTime", 0.0f);
                loadout.push_back(entry);
            }
            out.equipments.push_back(loadout);
        }
    }

    out.recon_modifier = number_field_or(host, row, "ReconModifier", 0.0f);
    const VehicleClassLuaValue recon_class = field(host, row, "ReconClass");
    if (!recon_class.is_nil()) {
        out.recon_class =
            host.resolve_recon_class(vehicle_class_integer_00b66290(recon_class));
    }

    // 00962DBC: the Repair byte.
    const VehicleClassLuaValue repair = field(host, row, "Repair");
    if (!repair.is_nil()) {
        out.repair = vehicle_class_boolean_00b66250(repair);
    }

    // 00962E44: the flag vector, indexed by a one-based Lua key.
    ScopedRef flags(host, host.get_by_name(row, "Flags"));
    if (host.inspect(flags.get()).is_table()) {
        GuiLuaRef key;
        GuiLuaRef value;
        for (bool more = host.iterate_first(flags.get(), key, value); more;
             more = host.iterate_next(flags.get(), key, value)) {
            const std::int32_t index = vehicle_class_flag_element_index_00962efa(
                vehicle_class_integer_00b66290(host.inspect(key)));
            if (index < 0) {
                continue;
            }
            if (static_cast<std::size_t>(index) >= out.flags.size()) {
                out.flags.resize(static_cast<std::size_t>(index) + 1);
            }
            VehicleClassFlagFields& flag = out.flags[static_cast<std::size_t>(index)];
            flag.size_horizontal = number_field_or(host, value, "SizeHorizontal", 2.5f);
            flag.size_vertical = number_field_or(host, value, "SizeVertical", 1.5f);
            flag.forced_texture = string_field_or(host, value, "ForcedTexture", "");
            if (!flag.forced_texture.empty()) {
                flag.texture = host.load_flag_texture(flag.forced_texture.c_str());
            }
        }
    }
}

}  // namespace bsp
