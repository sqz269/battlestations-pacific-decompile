// Ship class descriptor field reader. See include/bsp/ship_class_fields.hpp for
// the addresses, the evidence and the state this reached.

#include "bsp/ship_class_fields.hpp"

#include <cstring>
#include <utility>

namespace bsp {
namespace {

// The reader tests IsNil, not the value's type, everywhere a slot has a chained
// default; a present but non-numeric value therefore goes through 00B66270.
bool present(const VehicleClassLuaValue& value) noexcept {
    return value.kind != VehicleClassValueKind::Nil;
}

VehicleClassLuaValue field(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key) {
    const GuiLuaRef ref = lua.get_by_name(table, key);
    const VehicleClassLuaValue value = lua.inspect(ref);
    lua.release(ref);
    return value;
}

float number(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key) {
    return vehicle_class_number_00b66270(field(lua, table, key));
}

float number_or(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key,
                float fallback) {
    return vehicle_class_number_or_00b66330(field(lua, table, key), fallback);
}

std::int32_t integer_or(VehicleClassFieldHost& lua, const GuiLuaRef& table, const char* key,
                        std::int32_t fallback) {
    return vehicle_class_integer_or_00b66380(field(lua, table, key), fallback);
}

// 00831B62 / 00831C70 / 00832128: an effect key is read only when the value
// passes IsInteger, and the slot keeps whatever it already held otherwise.
void effect_if_integer(VehicleClassFieldHost& lua, ShipClassFieldHost& host,
                       const GuiLuaRef& row, const char* key, std::int32_t& slot) {
    const VehicleClassLuaValue value = field(lua, row, key);
    if (value.kind == VehicleClassValueKind::Integer) {
        slot = host.make_effect_handle(vehicle_class_integer_00b66290(value));
    }
}

// The three DamagedGFXRemove lists and the two HoD lists are all key/value
// walks that keep only the integer value.
void collect_integers(VehicleClassFieldHost& lua, const GuiLuaRef& table,
                      std::vector<std::int32_t>& out, bool minus_one) {
    GuiLuaRef key;
    GuiLuaRef value;
    for (bool more = lua.iterate_first(table, key, value); more;
         more = lua.iterate_next(table, key, value)) {
        const std::int32_t raw = vehicle_class_integer_00b66290(lua.inspect(value));
        out.push_back(minus_one ? ship_class_zero_based_index_00832f30(raw) : raw);
    }
}

// 00832ABE..00832B29: ExplosionTypes is walked by index 1..23 and each entry is
// skipped unless it passes IsInteger.
constexpr std::int32_t kExplosionTypesLimit = 24;
// 008326A0: HoD is walked by index 0..19 and the record index is the Lua key.
constexpr std::int32_t kHoDLimit = 20;
// 00833A94: LSPoints reads exactly eight entries with no bound test.
constexpr std::int32_t kLandingShipPointCount = 8;

}  // namespace

// ---------------------------------------------------------------------------
// Conversion rules
// ---------------------------------------------------------------------------

std::int32_t ship_class_zero_based_index_00832f30(std::int32_t lua_index) noexcept {
    return lua_index - 1;
}

float ship_class_reload_rate_00833b19(float lua_reload) noexcept {
    return 1.0f / lua_reload;
}

ShipClassCameraFields ship_class_camera_00831e0d(const ShipClassCameraInputs& in) noexcept {
    ShipClassCameraFields out;
    out.captain_camera_height = in.has_captain_camera_height ? in.captain_camera_height
                                                             : in.captain_camera_global;
    out.distance_front = in.has_distance_front ? in.distance_front : in.base_length;
    out.distance_side = in.has_distance_side ? in.distance_side : out.distance_front;
    out.distance_vertical = in.has_distance_vertical ? in.distance_vertical
                                                     : out.distance_front;
    out.min_height = in.has_min_height ? in.min_height : out.captain_camera_height;
    return out;
}

// ---------------------------------------------------------------------------
// Key schema and the installed-file counts
// ---------------------------------------------------------------------------

const ShipClassFieldSpec kShipClassFieldSchema[] = {
    {"MaxRotAngle",
     ShipClassConversion::Number, 0x4F8, 0x00831882, 0x00831899,
     "",
     "read straight into the slot"},
    {"MaxRotAngleChangeRatio",
     ShipClassConversion::Number, 0x4FC, 0x008318C4, 0x008318DB,
     "",
     "read straight into the slot"},
    {"MaxSpeed",
     ShipClassConversion::Number, 0x500, 0x00831903, 0x0083191A,
     "",
     "read straight into the slot"},
    {"MaxAccel",
     ShipClassConversion::Number, 0x504, 0x00831942, 0x00831959,
     "",
     "read straight into the slot"},
    {"Retardation",
     ShipClassConversion::Number, 0x508, 0x00831981, 0x00831998,
     "",
     "read straight into the slot"},
    {"CollisionMaxSpeedDamage",
     ShipClassConversion::NumberOr, 0x50C, 0x008319C0, 0x008319DD,
     "0.0f",
     "FLDZ at 008319d0"},
    {"KamikazeDamage",
     ShipClassConversion::NumberOr, 0x510, 0x00831A05, 0x00831A22,
     "0.0f",
     "read straight into the slot"},
    {"KamikazeBlastDamage",
     ShipClassConversion::NumberOr, 0x514, 0x00831A4A, 0x00831A67,
     "0.0f",
     "read straight into the slot"},
    {"KamikazeBlastRange",
     ShipClassConversion::NumberOr, 0x518, 0x00831A8F, 0x00831AAC,
     "0.0f",
     "read straight into the slot"},
    {"RotorSpdTurnDiff",
     ShipClassConversion::NumberOr, 0x69C, 0x00831AD4, 0x00831AF1,
     "0.0f",
     "read straight into the slot"},
    {"RotorSpd",
     ShipClassConversion::NumberOr, 0x6A0, 0x00831B19, 0x00831B41,
     "15.0f",
     "FLD [00ce5380] = 41700000h"},
    {"DeathEfx",
     ShipClassConversion::EffectHandle, 0x55C, 0x00831BA9, 0x00831BE8,
     "0",
     "guarded by IsInteger at 00831b62; GetIntegerOrDefault then 00870CD0"},
    {"ExplosionEfx",
     ShipClassConversion::EffectHandle, 0x51C, 0x00831CB7, 0x00831CF4,
     "",
     "guarded by IsInteger at 00831c70; GetInteger then 00870CD0"},
    {"UnderwaterArmour",
     ShipClassConversion::NumberOr, 0x6B4, 0x00831D7C, 0x00831D99,
     "0.0f",
     "read straight into the slot"},
    {"DamageThreshold",
     ShipClassConversion::NumberOr, 0x6B8, 0x00831DC4, 0x00831DE5,
     "100.0f",
     "FLD [00ce3d08] = 42C80000h"},
    {"CaptainCameraHeight",
     ShipClassConversion::NumberOrSlot, 0x538, 0x00831E0D, 0x00831E39,
     "DAT_00ce38b8",
     "IsNil at 00831e24 selects the global default"},
    {"CameraDistanceFront",
     ShipClassConversion::NumberOrSlot, 0x53C, 0x00831E60, 0x00831EB7,
     "descriptor+A0h (Length)",
     "IsNil at 00831e95"},
    {"CameraDistanceSide",
     ShipClassConversion::NumberOrSlot, 0x540, 0x00831EC7, 0x00831F1E,
     "descriptor+53Ch",
     "read straight into the slot"},
    {"CameraDistanceVertical",
     ShipClassConversion::NumberOrSlot, 0x544, 0x00831F2E, 0x00831F85,
     "descriptor+53Ch",
     "read straight into the slot"},
    {"CameraMinHeight",
     ShipClassConversion::NumberOrSlot, 0x548, 0x00831F95, 0x00831FEC,
     "descriptor+538h",
     "read straight into the slot"},
    {"DamageToDeath",
     ShipClassConversion::NumberOr, 0x54C, 0x00831FFC, 0x0083201A,
     "-1.0f",
     "FLD [00d7a260] = BF800000h"},
    {"TimeToDeath",
     ShipClassConversion::NumberOr, 0x550, 0x00832043, 0x00832061,
     "-1.0f",
     "read straight into the slot"},
    {"PumpTimeToEmpty",
     ShipClassConversion::NumberOr, 0x554, 0x0083208A, 0x008320A8,
     "-1.0f",
     "read straight into the slot"},
    {"WaterForceMultiplier",
     ShipClassConversion::NumberOr, 0x558, 0x008320D1, 0x008320EB,
     "1.0f",
     "FLD1 at 008320d6"},
    {"BowParticle",
     ShipClassConversion::EffectHandle, 0x5AC, 0x00832114, 0x00832167,
     "",
     "guarded by IsInteger at 00832128"},
    {"BowWave",
     ShipClassConversion::EffectHandle, 0x630, 0x008321D8, 0x0083222B,
     "",
     "read straight into the slot"},
    {"WaveStern",
     ShipClassConversion::EffectHandle, 0x634, 0x0083229C, 0x008322EF,
     "",
     "read straight into the slot"},
    {"RotorParticle",
     ShipClassConversion::EffectHandle, 0x65C, 0x00832360, 0x008323D4,
     "",
     "read straight into the slot"},
    {"DamageSmoke",
     ShipClassConversion::Table, kShipClassNoOffset, 0x00832445, 0,
     "",
     "IsTable at 00832459; when absent +660h = 0 and the +664h vector is cleared by 004D17C0 at "
     "00832654"},
    {"DamageSmoke.MaxNumber",
     ShipClassConversion::Integer, 0x660, 0x0083247A, 0x00832495,
     "",
     "read straight into the slot"},
    {"DamageSmoke.Effect",
     ShipClassConversion::EffectHandleList, 0x664, 0x008324C1, 0x00832577,
     "0",
     "walked by index from 1; IsInteger guard at 008324ed, GetIntegerOrDefault at 00832538, "
     "00870CD0 then 004D9C00 appends into the vector at +664h"},
    {"HoD",
     ShipClassConversion::Table, 0x138, 0x0083266C, 0,
     "",
     "20 records of 30h bytes at descriptor+138h; the cursor EBX = record+24h is set by LEA "
     "EBX,[EDI+0x15c] at 00832695"},
    {"HoD[].Flag",
     ShipClassConversion::IntegerList, 0x138, 0x0083270E, 0x0083277A,
     "",
     "record+0h; each entry GetInteger at 00832764 then 00442190"},
    {"HoD[].Smoke",
     ShipClassConversion::IntegerList, 0x14C, 0x008327D5, 0,
     "",
     "record+14h {begin,end,capacity}; push_back or 00441F30 on growth"},
    {"HoD[].Idle",
     ShipClassConversion::IntegerList, 0x15C, 0x008328FE, 0,
     "",
     "record+24h {begin,end,capacity}"},
    {"ExplosionTypes",
     ShipClassConversion::IntegerArray, 0x690, 0x00832A73, 0,
     "",
     "indices 1..23; IsInteger guard at 00832ac3, GetInteger at 00832b05; data +690h, count "
     "+694h, capacity +698h, grown by 004674F0"},
    {"Smoke",
     ShipClassConversion::EffectHandle, 0x674, 0x00832B7A, 0x00832BB6,
     "0",
     "the same id also builds two more handles, stored at +6A4h and +6A8h"},
    {"Hull",
     ShipClassConversion::Table, kShipClassNoOffset, 0x00832D51, 0,
     "",
     "IsTable at 00832d65"},
    {"Hull.WaterLineRatio",
     ShipClassConversion::Number, 0x71C, 0x00832D86, 0x00832D9A,
     "",
     "read straight into the slot"},
    {"Hull.Segments",
     ShipClassConversion::NumberToInt, 0x720, 0x00832DC8, 0x00832DE8,
     "",
     "GetNumber then the CRT float-to-int at 00BF7420"},
    {"Traffic",
     ShipClassConversion::RecordVector, 0x6DC, 0x00832E0A, 0,
     "",
     "IsNil at 00832e1e; vector at +6DCh {?, begin +6E0h, end +6E4h} of 50h-byte records; "
     "resize by 0082FD20, element constructed by 0049F9B0"},
    {"Traffic[].pathID",
     ShipClassConversion::IntegerMinusOne, 0x4C, 0x00832F1C, 0x00832F3A,
     "",
     "record+4Ch; the rest of the record is filled by 0049D4C0 at 00832f54 from the same Lua "
     "element"},
    {"Idle",
     ShipClassConversion::RecordVector, 0x6FC, 0x00832FC7, 0,
     "",
     "a row-level sibling of Traffic, not nested under it: ECX is reloaded from [EBP+8] at "
     "00832fb5; vector at +6FCh of 1Ch-byte records, resize by 0082F8E0, element constructed by "
     "0082E1F0"},
    {"Idle[].posID",
     ShipClassConversion::IntegerMinusOne, 0x18, 0x008330C5, 0x008330E3,
     "",
     "record+18h"},
    {"Idle[].templates",
     ShipClassConversion::ClassWeightMap, 0x0, 0x00833107, 0x0083328C,
     "",
     "record+0h; key/value iteration; the key names a class looked up in LandVehicleclasses "
     "(00CE6710) via 0048E960/0048E8D0, resolved by BSP_VehicleClass_GetOrCreate 00964790, else "
     "in SoldierTypes (00CE6700) via 004B1400; the Lua number is stored as a float through "
     "00499030"},
    {"Idle[].anims",
     ShipClassConversion::NameWeightMap, 0xC, 0x00833301, 0x008333F2,
     "",
     "record+0Ch; the key string is copied into a native string and 00444BE0 returns the float "
     "slot"},
    {"Camos",
     ShipClassConversion::CamoArray, 0x710, 0x00833501, 0,
     "",
     "IsTable at 00833515; key/value iteration, the integer key indexes the array at +710h "
     "(begin) / +714h (end); each value allocates a refcounted camo record (vtable 00D099B0) "
     "that replaces the slot"},
    {"Camos[].TextureRemaps",
     ShipClassConversion::StringPairList, kShipClassNoOffset, 0x0083368B, 0,
     "",
     "a list of two-element string arrays; both strings are read by GetString at 0083374e and "
     "008337bb and appended by 0082B170 / 0082E160 / 0082AD20"},
    {"Camos[].GunColor",
     ShipClassConversion::Vector4, 0x1C, 0x0083394A, 0x00833980,
     "",
     "camo record: +18h = 1 when present else 0; the four floats land at +1Ch..+28h, read by "
     "00B67C40"},
    {"LSClassId",
     ShipClassConversion::Integer, 0x724, 0x00833A3A, 0x00833A76,
     "",
     "IsNil guard at 00833a4e"},
    {"LSPoints",
     ShipClassConversion::Vector3Array8, 0x728, 0x00833A7C, 0,
     "",
     "exactly 8 entries; each is read by 00B677E0 / 00B67A80 into a 12-byte slot, +728h..+787h"},
    {"LSReload",
     ShipClassConversion::NumberReciprocal, 0x788, 0x00833AFE, 0x00833B2D,
     "",
     "stored as 1.0f / value"},
    {"LandingShip",
     ShipClassConversion::ClassPointerOr, 0x78C, 0x00833B6C, 0x00833BB6,
     "0",
     "+78Ch is cleared first; the id resolves through BSP_VehicleClass_GetOrCreate and is "
     "stored only when virtual slot +18h returns true"},
    {"LandingShipAmount",
     ShipClassConversion::IntegerOr, 0x790, 0x00833BCB, 0x00833BE8,
     "0",
     "read straight into the slot"},
    {"LandingShipCoolDown",
     ShipClassConversion::IntegerOr, 0x794, 0x00833C0A, 0x00833C27,
     "60",
     "read straight into the slot"},
    {"Fire",
     ShipClassConversion::Table, kShipClassNoOffset, 0x00833C49, 0,
     "",
     "IsNil at 00833c64; the settings object comes from 00424C40"},
    {"Fire.FireDamagePerFireTick",
     ShipClassConversion::NumberOrSettings, 0x574, 0x00833C81, 0x00833CB9,
     "settings+74Ch",
     "IsNumber at 00833c95 selects the settings default"},
    {"MaxTorpedoStock",
     ShipClassConversion::IntegerOrZero, 0x7A0, 0x00833CE6, 0x00833D1B,
     "0",
     "IsNil at 00833cf7 writes 0 at 00833d00"},
    {"DamagedGFXRemove",
     ShipClassConversion::Table, kShipClassNoOffset, 0x00833D2A, 0,
     "",
     "IsNil at 00833d59"},
    {"DamagedGFXRemove.Slots",
     ShipClassConversion::IntegerList, 0x7A8, 0x00833D74, 0,
     "",
     "{begin +7A8h, end +7ACh, capacity +7B0h}; values stored unchanged"},
    {"DamagedGFXRemove.Funnels",
     ShipClassConversion::IntegerListMinusOne, 0x7B8, 0x00833EAB, 0,
     "",
     "{+7B8h, +7BCh, +7C0h}; values stored minus one"},
    {"DamagedGFXRemove.Flags",
     ShipClassConversion::IntegerListMinusOne, 0x7C8, 0x00833FD0, 0,
     "",
     "{+7C8h, +7CCh, +7D0h}; values stored minus one"},
    {"InnerExplosionEfx",
     ShipClassConversion::IndexedEffectList, 0x7D8, 0x0083413C, 0,
     "",
     "row-level: ECX is the row at 0083413a; the integer key lands in the vector at {+7D8h, "
     "+7DCh, +7E0h} and the value builds a handle appended to the vector at +7E4h (LEA "
     "EBX,[EAX+0x7e4] at 008341dc)"},
    {"StructuralDamageEfx",
     ShipClassConversion::EffectHandleList, 0x7F4, 0x0083430D, 0,
     "",
     "row-level; only the value is used: GetInteger, 00870CD0, then 004D9C00 into the vector at "
     "+7F4h (LEA EBX,[ESI+0x7f4] at 008343a3)"},
    {"HackShipRotationAdd",
     ShipClassConversion::NumberOrSettings, 0x798, 0x00834476, 0x008344B9,
     "settings+238h",
     "read straight into the slot"},
    {"HackShipHeightAdd",
     ShipClassConversion::NumberOrSettings, 0x79C, 0x008344CE, 0x00834511,
     "settings+23Ch",
     "read straight into the slot"},
    {"CapturePower",
     ShipClassConversion::IntegerOrAsFloat, 0x804, 0x00834526, 0x00834547,
     "10",
     "CVTSI2SS at 0083453c; the slot is a float"},
};

const ShipClassKeyCount kShipClassKeyCounts[] = {
    {"MaxRotAngle", 160},
    {"MaxRotAngleChangeRatio", 160},
    {"MaxSpeed", 160},
    {"MaxAccel", 160},
    {"Retardation", 160},
    {"CollisionMaxSpeedDamage", 160},
    {"KamikazeDamage", 2},
    {"KamikazeBlastDamage", 2},
    {"KamikazeBlastRange", 2},
    {"RotorSpdTurnDiff", 0},
    {"RotorSpd", 0},
    {"DeathEfx", 160},
    {"ExplosionEfx", 12},
    {"UnderwaterArmour", 149},
    {"DamageThreshold", 146},
    {"CaptainCameraHeight", 11},
    {"CameraDistanceFront", 160},
    {"CameraDistanceSide", 160},
    {"CameraDistanceVertical", 160},
    {"CameraMinHeight", 151},
    {"DamageToDeath", 0},
    {"TimeToDeath", 73},
    {"PumpTimeToEmpty", 38},
    {"WaterForceMultiplier", 14},
    {"BowParticle", 159},
    {"BowWave", 159},
    {"WaveStern", 160},
    {"RotorParticle", 159},
    {"DamageSmoke", 153},
    {"DamageSmoke.MaxNumber", 153},
    {"DamageSmoke.Effect", 153},
    {"HoD", 141},
    {"HoD[].Flag", 123},
    {"HoD[].Smoke", 121},
    {"HoD[].Idle", 108},
    {"ExplosionTypes", 137},
    {"Smoke", 142},
    {"Hull", 160},
    {"Hull.WaterLineRatio", 160},
    {"Hull.Segments", 160},
    {"Traffic", 134},
    {"Traffic[].pathID", 16},
    {"Idle", 142},
    {"Idle[].posID", 125},
    {"Idle[].templates", 125},
    {"Idle[].anims", 125},
    {"Camos", 160},
    {"Camos[].TextureRemaps", 16},
    {"Camos[].GunColor", 141},
    {"LSClassId", 0},
    {"LSPoints", 0},
    {"LSReload", 0},
    {"LandingShip", 2},
    {"LandingShipAmount", 2},
    {"LandingShipCoolDown", 2},
    {"Fire", 0},
    {"Fire.FireDamagePerFireTick", 0},
    {"MaxTorpedoStock", 74},
    {"DamagedGFXRemove", 125},
    {"DamagedGFXRemove.Slots", 105},
    {"DamagedGFXRemove.Funnels", 106},
    {"DamagedGFXRemove.Flags", 125},
    {"InnerExplosionEfx", 147},
    {"StructuralDamageEfx", 147},
    {"HackShipRotationAdd", 112},
    {"HackShipHeightAdd", 99},
    {"CapturePower", 160},
};

const char* const kShipClassUnprovidedPaths[] = {
    "DamageToDeath",
    "Fire",
    "Fire.FireDamagePerFireTick",
    "LSClassId",
    "LSPoints",
    "LSReload",
    "RotorSpd",
    "RotorSpdTurnDiff",
};

const ShipClassKeyCount kShipClassUnconsumedPaths[] = {
    {"ArmorIndexes", 160},
    {"Damage.Sections[].HPBlack", 160},
    {"Race", 160},
    {"Type", 160},
    {"HP_Realistic", 158},
    {"MaxSpeed_Realistic", 154},
    {"MovieCameraPositions.SnittPositions.IncomingAttack", 143},
    {"PlatformDirections", 117},
    {"PlatformDirections.AFT", 117},
    {"PlatformDirections.FORWARD", 117},
    {"PlatformDirections.LEFT", 117},
    {"PlatformDirections.RIGHT", 117},
    {"Unlock", 74},
    {"MaxTorpedoStock_Realistic", 71},
    {"UnitlibViewDistance", 68},
    {"SmokeScreen", 47},
    {"SmokeScreenCooldown", 47},
    {"SmokeScreenDuration", 47},
    {"SmokeScreenRadius", 47},
    {"SmokeScreenSpawnTime", 47},
    {"UnlockID", 46},
    {"DeckCamera", 25},
    {"DeckCamera.HorzAngle", 25},
    {"DeckCamera.Position", 25},
};

std::size_t ship_class_field_count() noexcept {
    return sizeof(kShipClassFieldSchema) / sizeof(kShipClassFieldSchema[0]);
}

const ShipClassFieldSpec* ship_class_find_field(const char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < ship_class_field_count(); ++i) {
        if (std::strcmp(kShipClassFieldSchema[i].path, path) == 0) {
            return &kShipClassFieldSchema[i];
        }
    }
    return nullptr;
}

std::size_t ship_class_key_count_count() noexcept {
    return sizeof(kShipClassKeyCounts) / sizeof(kShipClassKeyCounts[0]);
}

std::size_t ship_class_unprovided_path_count() noexcept {
    return sizeof(kShipClassUnprovidedPaths) / sizeof(kShipClassUnprovidedPaths[0]);
}

std::size_t ship_class_unconsumed_path_count() noexcept {
    return sizeof(kShipClassUnconsumedPaths) / sizeof(kShipClassUnconsumedPaths[0]);
}

// ---------------------------------------------------------------------------
// The reader sequence
// ---------------------------------------------------------------------------

void read_ship_class_fields_00831840(VehicleClassFieldHost& lua,
                                     ShipClassFieldHost& host,
                                     const GuiLuaRef& row,
                                     float base_length,
                                     ShipClassFields& out) {
    // 00831882..00831998. Five plain numbers, written whatever the value is.
    out.max_rot_angle = number(lua, row, "MaxRotAngle");
    out.max_rot_angle_change_ratio = number(lua, row, "MaxRotAngleChangeRatio");
    out.max_speed = number(lua, row, "MaxSpeed");
    out.max_accel = number(lua, row, "MaxAccel");
    out.retardation = number(lua, row, "Retardation");

    // 008319C0..00831B41.
    out.collision_max_speed_damage = number_or(lua, row, "CollisionMaxSpeedDamage", 0.0f);
    out.kamikaze_damage = number_or(lua, row, "KamikazeDamage", 0.0f);
    out.kamikaze_blast_damage = number_or(lua, row, "KamikazeBlastDamage", 0.0f);
    out.kamikaze_blast_range = number_or(lua, row, "KamikazeBlastRange", 0.0f);
    out.rotor_spd_turn_diff = number_or(lua, row, "RotorSpdTurnDiff", 0.0f);
    out.rotor_spd = number_or(lua, row, "RotorSpd", 15.0f);

    // 00831B62 and 00831C70: read twice each, once to test and once to convert.
    effect_if_integer(lua, host, row, "DeathEfx", out.death_efx);
    effect_if_integer(lua, host, row, "ExplosionEfx", out.explosion_efx);

    out.underwater_armour = number_or(lua, row, "UnderwaterArmour", 0.0f);
    out.damage_threshold = number_or(lua, row, "DamageThreshold", 100.0f);

    // 00831E0D..00831FEC, the chained camera block.
    ShipClassCameraInputs camera;
    camera.base_length = base_length;
    camera.captain_camera_global = host.default_captain_camera_height();
    const VehicleClassLuaValue captain = field(lua, row, "CaptainCameraHeight");
    camera.has_captain_camera_height = present(captain);
    camera.captain_camera_height = vehicle_class_number_00b66270(captain);
    const VehicleClassLuaValue front = field(lua, row, "CameraDistanceFront");
    camera.has_distance_front = present(front);
    camera.distance_front = vehicle_class_number_00b66270(front);
    const VehicleClassLuaValue side = field(lua, row, "CameraDistanceSide");
    camera.has_distance_side = present(side);
    camera.distance_side = vehicle_class_number_00b66270(side);
    const VehicleClassLuaValue vertical = field(lua, row, "CameraDistanceVertical");
    camera.has_distance_vertical = present(vertical);
    camera.distance_vertical = vehicle_class_number_00b66270(vertical);
    const VehicleClassLuaValue min_height = field(lua, row, "CameraMinHeight");
    camera.has_min_height = present(min_height);
    camera.min_height = vehicle_class_number_00b66270(min_height);
    out.camera = ship_class_camera_00831e0d(camera);

    // 00831FFC..008320EB. The three sinking timers share the -1 constant at
    // 00D7A260; WaterForceMultiplier uses FLD1.
    out.damage_to_death = number_or(lua, row, "DamageToDeath", -1.0f);
    out.time_to_death = number_or(lua, row, "TimeToDeath", -1.0f);
    out.pump_time_to_empty = number_or(lua, row, "PumpTimeToEmpty", -1.0f);
    out.water_force_multiplier = number_or(lua, row, "WaterForceMultiplier", 1.0f);

    // 00832114..008323D4, the four wake and propeller particles.
    effect_if_integer(lua, host, row, "BowParticle", out.bow_particle);
    effect_if_integer(lua, host, row, "BowWave", out.bow_wave);
    effect_if_integer(lua, host, row, "WaveStern", out.wave_stern);
    effect_if_integer(lua, host, row, "RotorParticle", out.rotor_particle);

    // 00832445..00832616. When DamageSmoke is not a table the native clears both
    // slots, which is what the caller sees in a default-constructed out.
    const GuiLuaRef damage_smoke = lua.get_by_name(row, "DamageSmoke");
    if (lua.inspect(damage_smoke).kind == VehicleClassValueKind::Table) {
        out.damage_smoke_max_number =
            vehicle_class_integer_00b66290(field(lua, damage_smoke, "MaxNumber"));
        const GuiLuaRef effects = lua.get_by_name(damage_smoke, "Effect");
        for (std::int32_t i = 1;; ++i) {
            const GuiLuaRef entry = lua.get_by_index(effects, i);
            const VehicleClassLuaValue value = lua.inspect(entry);
            lua.release(entry);
            if (value.kind != VehicleClassValueKind::Integer) {
                break;
            }
            out.damage_smoke_effects.push_back(
                host.make_effect_handle(vehicle_class_integer_or_00b66380(value, 0)));
        }
        lua.release(effects);
    }
    lua.release(damage_smoke);

    // 0083266C..00832A39. Twenty fixed records; the Lua index is the record.
    const GuiLuaRef hod = lua.get_by_name(row, "HoD");
    if (lua.inspect(hod).kind == VehicleClassValueKind::Table) {
        for (std::int32_t i = 0; i < kHoDLimit; ++i) {
            const GuiLuaRef entry = lua.get_by_index(hod, i);
            if (lua.inspect(entry).kind == VehicleClassValueKind::Table) {
                const GuiLuaRef flags = lua.get_by_name(entry, "Flag");
                if (lua.inspect(flags).kind == VehicleClassValueKind::Table) {
                    collect_integers(lua, flags, out.hod[i].flags, false);
                }
                lua.release(flags);
                const GuiLuaRef smoke = lua.get_by_name(entry, "Smoke");
                if (lua.inspect(smoke).kind == VehicleClassValueKind::Table) {
                    collect_integers(lua, smoke, out.hod[i].smoke, false);
                }
                lua.release(smoke);
                const GuiLuaRef idle = lua.get_by_name(entry, "Idle");
                if (lua.inspect(idle).kind == VehicleClassValueKind::Table) {
                    collect_integers(lua, idle, out.hod[i].idle, false);
                }
                lua.release(idle);
            }
            lua.release(entry);
        }
    }
    lua.release(hod);

    // 00832A73..00832B29. Indices 1..23, non-integers skipped rather than ending
    // the walk.
    const GuiLuaRef explosion_types = lua.get_by_name(row, "ExplosionTypes");
    if (lua.inspect(explosion_types).kind == VehicleClassValueKind::Table) {
        for (std::int32_t i = 1; i < kExplosionTypesLimit; ++i) {
            const GuiLuaRef entry = lua.get_by_index(explosion_types, i);
            const VehicleClassLuaValue value = lua.inspect(entry);
            if (value.kind == VehicleClassValueKind::Integer) {
                out.explosion_types.push_back(vehicle_class_integer_00b66290(value));
            }
            lua.release(entry);
        }
    }
    lua.release(explosion_types);

    // 00832B7A. One id builds three handles; only the first is modelled here
    // because the other two land in slots nothing else in this packet reads.
    out.smoke = host.make_effect_handle(integer_or(lua, row, "Smoke", 0));

    // 00832D51..00832DE8.
    const GuiLuaRef hull = lua.get_by_name(row, "Hull");
    if (lua.inspect(hull).kind == VehicleClassValueKind::Table) {
        out.hull_water_line_ratio = number(lua, hull, "WaterLineRatio");
        out.hull_segments = static_cast<std::int32_t>(number(lua, hull, "Segments"));
    }
    lua.release(hull);

    // 00832E0A..00832FAF. Traffic is walked from index 1 until the first nil.
    const GuiLuaRef traffic = lua.get_by_name(row, "Traffic");
    if (present(lua.inspect(traffic))) {
        for (std::int32_t i = 1;; ++i) {
            const GuiLuaRef entry = lua.get_by_index(traffic, i);
            const bool more = present(lua.inspect(entry));
            if (more) {
                ShipClassTrafficFields record;
                const VehicleClassLuaValue path_id = field(lua, entry, "pathID");
                record.path_id = ship_class_zero_based_index_00832f30(
                    vehicle_class_integer_00b66290(path_id));
                out.traffic.push_back(record);
            }
            lua.release(entry);
            if (!more) {
                break;
            }
        }
    }
    lua.release(traffic);

    // 00832FB5..008334EE. Idle is a sibling of Traffic, not nested under it:
    // 00832FB5 reloads the receiver from the function's own row argument.
    const GuiLuaRef idle = lua.get_by_name(row, "Idle");
    if (present(lua.inspect(idle))) {
        for (std::int32_t i = 1;; ++i) {
            const GuiLuaRef entry = lua.get_by_index(idle, i);
            const bool more = present(lua.inspect(entry));
            if (more) {
                ShipClassIdleFields record;
                record.pos_id = ship_class_zero_based_index_00832f30(
                    vehicle_class_integer_00b66290(field(lua, entry, "posID")));
                const GuiLuaRef templates = lua.get_by_name(entry, "templates");
                GuiLuaRef key;
                GuiLuaRef value;
                for (bool more_key = lua.iterate_first(templates, key, value); more_key;
                     more_key = lua.iterate_next(templates, key, value)) {
                    ShipClassIdleFields::Template item;
                    const VehicleClassLuaValue name = lua.inspect(key);
                    item.name = name.string != nullptr ? name.string : "";
                    // 008331E6: LandVehicleclasses is probed first.
                    item.is_soldier =
                        !host.class_table_contains("LandVehicleclasses", item.name.c_str());
                    if (item.is_soldier) {
                        host.resolve_soldier_type(item.name.c_str());
                    } else {
                        host.resolve_vehicle_class(item.name.c_str());
                    }
                    item.weight = vehicle_class_number_00b66270(lua.inspect(value));
                    record.templates.push_back(std::move(item));
                }
                lua.release(templates);
                const GuiLuaRef anims = lua.get_by_name(entry, "anims");
                for (bool more_key = lua.iterate_first(anims, key, value); more_key;
                     more_key = lua.iterate_next(anims, key, value)) {
                    const VehicleClassLuaValue name = lua.inspect(key);
                    record.anims[name.string != nullptr ? name.string : ""] =
                        vehicle_class_number_00b66270(lua.inspect(value));
                }
                lua.release(anims);
                out.idle.push_back(std::move(record));
            }
            lua.release(entry);
            if (!more) {
                break;
            }
        }
    }
    lua.release(idle);

    // 00833501..00833945. The integer key indexes the camo array, so a gap in
    // the Lua table leaves a hole the native fills with a null slot; this keeps
    // the entries in iteration order and records nothing about the gaps.
    const GuiLuaRef camos = lua.get_by_name(row, "Camos");
    if (lua.inspect(camos).kind == VehicleClassValueKind::Table) {
        GuiLuaRef key;
        GuiLuaRef value;
        for (bool more = lua.iterate_first(camos, key, value); more;
             more = lua.iterate_next(camos, key, value)) {
            ShipClassCamoFields camo;
            const GuiLuaRef remaps = lua.get_by_name(value, "TextureRemaps");
            if (present(lua.inspect(remaps))) {
                for (std::int32_t i = 1;; ++i) {
                    const GuiLuaRef pair = lua.get_by_index(remaps, i);
                    const bool is_pair =
                        lua.inspect(pair).kind == VehicleClassValueKind::Table;
                    if (is_pair) {
                        const GuiLuaRef from = lua.get_by_index(pair, 1);
                        const GuiLuaRef to = lua.get_by_index(pair, 2);
                        const VehicleClassLuaValue from_value = lua.inspect(from);
                        const VehicleClassLuaValue to_value = lua.inspect(to);
                        camo.texture_remaps.emplace_back(
                            from_value.string != nullptr ? from_value.string : "",
                            to_value.string != nullptr ? to_value.string : "");
                        lua.release(to);
                        lua.release(from);
                    }
                    lua.release(pair);
                    if (!is_pair) {
                        break;
                    }
                }
            }
            lua.release(remaps);
            const GuiLuaRef gun_color = lua.get_by_name(value, "GunColor");
            camo.has_gun_color = present(lua.inspect(gun_color));
            if (camo.has_gun_color) {
                for (std::int32_t i = 0; i < 4; ++i) {
                    const GuiLuaRef component = lua.get_by_index(gun_color, i + 1);
                    camo.gun_color[i] =
                        vehicle_class_number_00b66270(lua.inspect(component));
                    lua.release(component);
                }
            }
            lua.release(gun_color);
            out.camos.push_back(std::move(camo));
        }
    }
    lua.release(camos);

    // 00833A3A..00833C27, the landing-ship block.
    const VehicleClassLuaValue ls_class = field(lua, row, "LSClassId");
    if (present(ls_class)) {
        out.ls_class_id = vehicle_class_integer_00b66290(ls_class);
        const GuiLuaRef points = lua.get_by_name(row, "LSPoints");
        for (std::int32_t i = 0; i < kLandingShipPointCount; ++i) {
            const GuiLuaRef point = lua.get_by_index(points, i + 1);
            for (std::int32_t axis = 0; axis < 3; ++axis) {
                const GuiLuaRef component = lua.get_by_index(point, axis + 1);
                out.ls_points[i][axis] =
                    vehicle_class_number_00b66270(lua.inspect(component));
                lua.release(component);
            }
            lua.release(point);
        }
        lua.release(points);
        out.has_ls_reload = true;
        out.ls_reload_rate = ship_class_reload_rate_00833b19(number(lua, row, "LSReload"));
    }
    out.landing_ship_class =
        host.resolve_landing_ship_class(integer_or(lua, row, "LandingShip", 0));
    out.landing_ship_amount = integer_or(lua, row, "LandingShipAmount", 0);
    out.landing_ship_cool_down = integer_or(lua, row, "LandingShipCoolDown", 60);

    // 00833C49..00833CB9. The default comes from the settings object, not zero.
    const float fire_default =
        host.settings_float(ShipClassSettingsOffsets::kFireDamagePerFireTick);
    out.fire_damage_per_fire_tick = fire_default;
    const GuiLuaRef fire = lua.get_by_name(row, "Fire");
    if (present(lua.inspect(fire))) {
        const VehicleClassLuaValue tick = field(lua, fire, "FireDamagePerFireTick");
        const bool numeric = tick.kind == VehicleClassValueKind::Number ||
                             tick.kind == VehicleClassValueKind::Integer;
        out.fire_damage_per_fire_tick =
            numeric ? vehicle_class_number_00b66270(tick) : fire_default;
    }
    lua.release(fire);

    // 00833CE6.
    const VehicleClassLuaValue torpedoes = field(lua, row, "MaxTorpedoStock");
    out.max_torpedo_stock = present(torpedoes) ? vehicle_class_integer_00b66290(torpedoes) : 0;

    // 00833D2A..008340B0.
    const GuiLuaRef removes = lua.get_by_name(row, "DamagedGFXRemove");
    if (present(lua.inspect(removes))) {
        const GuiLuaRef slots = lua.get_by_name(removes, "Slots");
        if (present(lua.inspect(slots))) {
            collect_integers(lua, slots, out.remove_slots, false);
        }
        lua.release(slots);
        const GuiLuaRef funnels = lua.get_by_name(removes, "Funnels");
        if (present(lua.inspect(funnels))) {
            collect_integers(lua, funnels, out.remove_funnels, true);
        }
        lua.release(funnels);
        const GuiLuaRef flags = lua.get_by_name(removes, "Flags");
        if (present(lua.inspect(flags))) {
            collect_integers(lua, flags, out.remove_flags, true);
        }
        lua.release(flags);
    }
    lua.release(removes);

    // 0083413C and 0083430D are row-level, not nested under DamagedGFXRemove:
    // the receiver at 0083413A and 0083430B is the row.
    const GuiLuaRef inner = lua.get_by_name(row, "InnerExplosionEfx");
    if (present(lua.inspect(inner))) {
        GuiLuaRef key;
        GuiLuaRef value;
        for (bool more = lua.iterate_first(inner, key, value); more;
             more = lua.iterate_next(inner, key, value)) {
            out.inner_explosion_slots.push_back(
                vehicle_class_integer_00b66290(lua.inspect(key)));
            out.inner_explosion_effects.push_back(
                host.make_effect_handle(vehicle_class_integer_00b66290(lua.inspect(value))));
        }
    }
    lua.release(inner);

    const GuiLuaRef structural = lua.get_by_name(row, "StructuralDamageEfx");
    if (present(lua.inspect(structural))) {
        GuiLuaRef key;
        GuiLuaRef value;
        for (bool more = lua.iterate_first(structural, key, value); more;
             more = lua.iterate_next(structural, key, value)) {
            out.structural_damage_effects.push_back(
                host.make_effect_handle(vehicle_class_integer_00b66290(lua.inspect(value))));
        }
    }
    lua.release(structural);

    // 00834476..00834547.
    out.hack_ship_rotation_add =
        number_or(lua, row, "HackShipRotationAdd",
                  host.settings_float(ShipClassSettingsOffsets::kHackShipRotationAdd));
    out.hack_ship_height_add =
        number_or(lua, row, "HackShipHeightAdd",
                  host.settings_float(ShipClassSettingsOffsets::kHackShipHeightAdd));
    out.capture_power = static_cast<float>(integer_or(lua, row, "CapturePower", 10));
}

}  // namespace bsp
