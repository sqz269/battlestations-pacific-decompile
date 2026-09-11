// Vehicle class Lua load: the parent-most base, the tuning block selector and
// the eight per-ship-class leaf overrides. See include/bsp/vehicle_class_lua_load.hpp
// for the address list and docs/VEHICLE_CLASS_LUA_LOAD.md for the evidence.

#include "bsp/vehicle_class_lua_load.hpp"

#include <cstring>

namespace bsp {
namespace {

// 00425850 BSP_NativeString_EqualsCStringInsensitive, the comparison both
// SubType branches use. A null string never matches, which is the branch
// MLandingShip takes at 0074CB55 and MCargo at 006EB577.
char lower_ascii(char c) noexcept {
    return (c >= 'A' && c <= 'Z') ? static_cast<char>(c - 'A' + 'a') : c;
}

bool equals_insensitive(const char* lhs, const char* rhs) noexcept {
    if (lhs == nullptr || rhs == nullptr) {
        return false;
    }
    while (*lhs != '\0' && lower_ascii(*lhs) == lower_ascii(*rhs)) {
        ++lhs;
        ++rhs;
    }
    return lower_ascii(*lhs) == lower_ascii(*rhs);
}

float number_or(VehicleClassFieldHost& lua, const GuiLuaRef& row, const char* key,
                float fallback) {
    const GuiLuaRef field = lua.get_by_name(row, key);
    const float value = vehicle_class_number_or_00b66330(lua.inspect(field), fallback);
    lua.release(field);
    return value;
}

std::int32_t integer_or(VehicleClassFieldHost& lua, const GuiLuaRef& row, const char* key,
                        std::int32_t fallback) {
    const GuiLuaRef field = lua.get_by_name(row, key);
    const std::int32_t value =
        vehicle_class_integer_or_00b66380(lua.inspect(field), fallback);
    lua.release(field);
    return value;
}

bool boolean_or(VehicleClassFieldHost& lua, const GuiLuaRef& row, const char* key,
                bool fallback) {
    const GuiLuaRef field = lua.get_by_name(row, key);
    const bool value = vehicle_class_boolean_or_00b662f0(lua.inspect(field), fallback);
    lua.release(field);
    return value;
}

// 00B66270 with no guard, the form MMothership uses for the two runway keys.
float number_bare(VehicleClassFieldHost& lua, const GuiLuaRef& row, const char* key) {
    const GuiLuaRef field = lua.get_by_name(row, key);
    const float value = vehicle_class_number_00b66270(lua.inspect(field));
    lua.release(field);
    return value;
}

// The tail every leaf shares: two dwords out of the selected tuning block, the
// first replicated across all four array slots.
void copy_tuning_pair(ShipLeafFieldHost& host, std::uint32_t array_source,
                      std::uint32_t scalar_source, ShipLeafFields& out) {
    const std::uint32_t array_value = host.tuning_block_dword(array_source);
    for (std::uint32_t i = 0; i < ShipLeafTuningOffsets::kTuningArrayCount; ++i) {
        out.tuning.array[i] = array_value;
    }
    out.tuning.scalar = host.tuning_block_dword(scalar_source);
    out.tuning.array_source = array_source;
    out.tuning.scalar_source = scalar_source;
}

}  // namespace

// ---------------------------------------------------------------------------
// 0087C640
// ---------------------------------------------------------------------------

const std::uint32_t kDamageableClassBaseZeroedOffsets[] = {
    0x0C, 0x10, 0x14, 0x1C, 0x20, 0x24, 0x28, 0x2C,
    0x30, 0x38, 0x3C, 0x50, 0x58, 0x5C, 0x68,
};

std::size_t damageable_class_base_zeroed_count() noexcept {
    return sizeof(kDamageableClassBaseZeroedOffsets) /
           sizeof(kDamageableClassBaseZeroedOffsets[0]);
}

const std::uint32_t kDamageableClassBaseUntouchedOffsets[] = {
    0x08, 0x18, 0x34, 0x48, 0x4C, 0x54,
};

std::size_t damageable_class_base_untouched_count() noexcept {
    return sizeof(kDamageableClassBaseUntouchedOffsets) /
           sizeof(kDamageableClassBaseUntouchedOffsets[0]);
}

// ---------------------------------------------------------------------------
// 00837DE0
// ---------------------------------------------------------------------------

std::uint32_t ship_tuning_block_offset(std::int32_t session_mode) noexcept {
    return session_mode != 0 ? ShipTuningBlockSelector::kBlockOtherwise
                             : ShipTuningBlockSelector::kBlockWhenModeZero;
}

// ---------------------------------------------------------------------------
// The eight leaves
// ---------------------------------------------------------------------------

const ShipLeafClassInfo kShipLeafClasses[] = {
    {ShipLeafClass::Battleship, "MBattleship", "BattleShip", 0x006E00F0, 0x00D1ADF8,
     0x00D1AE28, 40},
    {ShipLeafClass::Cargo, "MCargo", "Cargo", 0x006EB4A0, 0x00D1ADBC, 0x00D1ADEC, 13},
    {ShipLeafClass::Cruiser, "MCruiser", "Cruiser", 0x006FB550, 0x00D1AD38, 0x00D1AD68, 41},
    {ShipLeafClass::Destroyer, "MDestroyer", "Destroyer", 0x006FE6A0, 0x00D1ACF8,
     0x00D1AD28, 31},
    {ShipLeafClass::LandingShip, "MLandingShip", "LandingShip", 0x0074C630, 0x00D1AD78,
     0x00D1ADA8, 6},
    {ShipLeafClass::Mothership, "MMothership", "MotherShip", 0x00759590, 0x00D1AEBC,
     0x00D1AEEC, 18},
    {ShipLeafClass::Submarine, "MSubmarine", "Submarine", 0x00854230, 0x00D1AE38,
     0x00D1AE68, 9},
    {ShipLeafClass::TorpedoBoat, "MTorpedoBoat", "TorpedoBoat", 0x00857F40, 0x00D1AE78,
     0x00D1AEA8, 3},
};

std::size_t ship_leaf_class_count() noexcept {
    return sizeof(kShipLeafClasses) / sizeof(kShipLeafClasses[0]);
}

const ShipLeafTuningSource kShipLeafTuningSources[] = {
    {ShipLeafClass::Mothership, "", 0x04, 0x00, 0x00759844},
    {ShipLeafClass::Destroyer, "", 0x0C, 0x08, 0x006FE6BC},
    {ShipLeafClass::TorpedoBoat, "", 0x14, 0x10, 0x00858038},
    {ShipLeafClass::LandingShip, "BigLandingShip false", 0x1C, 0x18, 0x0074CAB4},
    {ShipLeafClass::LandingShip, "BigLandingShip true", 0x24, 0x20, 0x0074CA76},
    {ShipLeafClass::Battleship, "", 0x2C, 0x28, 0x006E015E},
    {ShipLeafClass::Cargo, "", 0x34, 0x30, 0x006EB4D5},
    {ShipLeafClass::Cruiser, "HeavyCruiser false", 0x3C, 0x38, 0x006FB616},
    {ShipLeafClass::Cruiser, "HeavyCruiser true", 0x44, 0x40, 0x006FB5C7},
    {ShipLeafClass::Submarine, "four distinct slots", 0x60, 0x5C, 0x008545CB},
};

std::size_t ship_leaf_tuning_source_count() noexcept {
    return sizeof(kShipLeafTuningSources) / sizeof(kShipLeafTuningSources[0]);
}

const std::uint32_t kSubmarineTuningArraySources[] = {0x60, 0x64, 0x68, 0x6C};

// ---------------------------------------------------------------------------
// Schema
// ---------------------------------------------------------------------------

const ShipLeafFieldSpec kShipLeafFieldSchema[] = {
    // MBattleship, 006E00F0.
    {ShipLeafClass::Battleship, "Battlecruiser", 0x00CF92F0, ShipLeafConversion::BooleanOr,
     ShipLeafDescriptorOffsets::kBattleshipIsBattlecruiser, 0x006E0122, 0x006E013C, "false",
     "the only key the battleship leaf reads"},

    // MCargo, 006EB4A0. The two bytes are cleared at 006EB50E/006EB515 before
    // the key is asked for, so a non-string SubType leaves both false.
    {ShipLeafClass::Cargo, "SubType", 0x00CFA9D0, ShipLeafConversion::StringEquals,
     ShipLeafDescriptorOffsets::kCargoIsJunk, 0x006EB51C, 0x006EB560, "false",
     "compared with \"Junk\" at 00CFA9C8 through 00425850"},
    {ShipLeafClass::Cargo, "SubType", 0x00CFA9D0, ShipLeafConversion::StringEquals,
     ShipLeafDescriptorOffsets::kCargoIsTroopTransport, 0x006EB51C, 0x006EB571, "false",
     "the same string compared with \"TroopTransport\" at 00CFA9B8"},

    // MCruiser, 006FB550.
    {ShipLeafClass::Cruiser, "HeavyCruiser", 0x00CEB14C, ShipLeafConversion::BooleanOr,
     ShipLeafDescriptorOffsets::kCruiserIsHeavyCruiser, 0x006FB582, 0x006FB59C, "false",
     "also selects which tuning pair the leaf copies, at 006FB5AF"},

    // MLandingShip, 0074C630.
    {ShipLeafClass::LandingShip, "BigLandingShip", 0x00CFFD08,
     ShipLeafConversion::BooleanOr, ShipLeafDescriptorOffsets::kLandingShipIsBig,
     0x0074C66A, 0x0074C687, "false (PUSH EBX, zeroed at 0074C66F)",
     "also selects the tuning pair at 0074CA5E"},
    {ShipLeafClass::LandingShip, "LandedDamage", 0x00CFFCF8, ShipLeafConversion::IntegerOr,
     ShipLeafDescriptorOffsets::kLandingShipLandedDamage, 0x0074C6A5, 0x0074C6C1,
     "0 (PUSH EBX)", ""},
    {ShipLeafClass::LandingShip, "LandedCapturePower", 0x00CFFCE4,
     ShipLeafConversion::IntegerOr, ShipLeafDescriptorOffsets::kLandingShipLandedCapturePower,
     0x0074C6DF, 0x0074C6FB, "0 (PUSH EBX)", ""},
    {ShipLeafClass::LandingShip, "LandingTroopType", 0x00CFFCD0,
     ShipLeafConversion::IntegerOr, ShipLeafDescriptorOffsets::kLandingShipTroopType,
     0x0074C719, 0x0074C736, "1 (PUSH 1 at 0074C71E)",
     "also indexes globals.LandingTroopClasses in the walk below"},
    {ShipLeafClass::LandingShip, "Rocketer", 0x00CFFCC4, ShipLeafConversion::BooleanOr,
     ShipLeafDescriptorOffsets::kLandingShipIsRocketer, 0x0074C754, 0x0074C770,
     "false (PUSH EBX)", ""},
    {ShipLeafClass::LandingShip, "SubType", 0x00CFA9D0, ShipLeafConversion::StringEquals,
     ShipLeafDescriptorOffsets::kLandingShipSubTypeLcvp, 0x0074CB02, 0x0074CB81, "false",
     "\"LCVP\" at 00CFFCBC; the three bytes are cleared at 0074CAED..0074CAFB"},
    {ShipLeafClass::LandingShip, "SubType", 0x00CFA9D0, ShipLeafConversion::StringEquals,
     ShipLeafDescriptorOffsets::kLandingShipSubTypeLsm, 0x0074CB02, 0x0074CBA1, "false",
     "\"LSM\" at 00CFFCB8"},
    {ShipLeafClass::LandingShip, "SubType", 0x00CFA9D0, ShipLeafConversion::StringEquals,
     ShipLeafDescriptorOffsets::kLandingShipSubTypeDefault, 0x0074CB02, 0x0074CBAA, "true",
     "set at 0074CBAA when neither literal matched and at 0074CBCE when the value is "
     "not a string"},
    {ShipLeafClass::LandingShip, "globals.LandingTroopClasses[].Classes[]", 0x00CE69AC,
     ShipLeafConversion::Number, kShipLeafNoOffset, 0x0074C7B2, 0, "none",
     "read from the globals table 00B67980 returns, not from the row; each element's "
     "Type at 00CE4780 selects \"Soldier\" (00CE693C, ClassName at 00CE6930 through "
     "004B1400) or \"Vehicle\" (00CE6928, ClassId at 00CE6920 through 00964790 then "
     "0047B3C0). No descriptor slot for the result was decoded"},

    // MMothership, 00759590.
    {ShipLeafClass::Mothership, "RunwayWidth", 0x00CF8AE8, ShipLeafConversion::Number,
     ShipLeafDescriptorOffsets::kMothershipRunwayWidth, 0x007595CA, 0x007595E1,
     "none (00B66270 with no guard)", ""},
    {ShipLeafClass::Mothership, "RunwayLength", 0x00CF8AD8, ShipLeafConversion::Number,
     ShipLeafDescriptorOffsets::kMothershipRunwayLength, 0x00759606, 0x0075961D,
     "none (00B66270 with no guard)", ""},
    {ShipLeafClass::Mothership, "CarrierEscort", 0x00D01918, ShipLeafConversion::BooleanOr,
     ShipLeafDescriptorOffsets::kMothershipIsCarrierEscort, 0x0075963F, 0x0075965C,
     "false (PUSH 0 at 00759644)", ""},
    {ShipLeafClass::Mothership, "MaxLandingPlanes", 0x00D01900,
     ShipLeafConversion::IntegerOrZero, ShipLeafDescriptorOffsets::kMothershipMaxLandingPlanes,
     0x0075968E, 0x007596D0, "0 (the else arm at 007596D8)",
     "assigned into a held LuaObject at 007596A4, then IsNil at 007596BE"},
    {ShipLeafClass::Mothership, "DeckCamera.Position", 0x00CE68B4,
     ShipLeafConversion::Vector3, ShipLeafDescriptorOffsets::kMothershipDeckCameraPosition,
     0x00759706, 0x00759811, "none",
     "read from the DeckCamera sub-table opened at 007596EA, through 00B67A80"},
    {ShipLeafClass::Mothership, "DeckCamera.HorzAngle", 0x00D018E8,
     ShipLeafConversion::NegatedAngle, kShipLeafNoOffset, 0x0075973D, 0, "none",
     "negated at 0075979F, then 00B646E0 builds one axis of the basis at +830h"},
    {ShipLeafClass::Mothership, "DeckCamera.VertAngle", 0x00D018DC,
     ShipLeafConversion::NegatedAngle, kShipLeafNoOffset, 0x00759773, 0, "none",
     "negated at 007597C3, then 00B64640; 00413920 multiplies the two and 004134F0 "
     "copies the product into +830h at 00759806"},

    // MSubmarine, 00854230.
    {ShipLeafClass::Submarine, "PeriscopeWave", 0x00D0C2F8,
     ShipLeafConversion::EffectHandle, ShipLeafDescriptorOffsets::kSubmarinePeriscopeWave,
     0x00854263, 0x008542A5, "none; the slot keeps its memset zero",
     "IsInteger at 00854272 guards the GetInteger at 00854282 and the 00870CD0 wrap"},
    {ShipLeafClass::Submarine, "PeriscopeDepth", 0x00D0C2E8, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarinePeriscopeDepth, 0x00854304, 0x0085431F,
     "-1.0f (00D7A260)", ""},
    {ShipLeafClass::Submarine, "SwimDepth1", 0x00D0C2DC, ShipLeafConversion::NumberOrPrior,
     ShipLeafDescriptorOffsets::kSubmarinePeriscopeDepth, 0x0085434A, 0x00854365,
     "-1.0f (00D7A260)",
     "read into the same slot, and only when PeriscopeDepth left it negative "
     "(COMISS then JBE at 00854335); the legacy alias, not a second field"},
    {ShipLeafClass::Submarine, "SwimDepth2", 0x00D0C2D0, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineSwimDepth2, 0x00854384, 0x0085439F,
     "-1.0f (00D7A260)", ""},
    {ShipLeafClass::Submarine, "SwimDepth3", 0x00D0C2C4, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineSwimDepth3, 0x008543BE, 0x008543D9,
     "-1.0f (00D7A260)", ""},
    {ShipLeafClass::Submarine, "PeriscopeMoveRange", 0x00D0C2B0, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarinePeriscopeMoveRange, 0x008543F8, 0x00854413,
     "10.0f (00CE38B8)", ""},
    {ShipLeafClass::Submarine, "UpDownStopTime", 0x00D0C2A0, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineUpDownStopTime, 0x00854432, 0x0085444D,
     "5.0f (00CE3850)", ""},
    {ShipLeafClass::Submarine, "UpSpeed", 0x00D0C298, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineUpSpeed, 0x0085446C, 0x00854487,
     "1.2f (00CE3814)", ""},
    {ShipLeafClass::Submarine, "DownSpeed", 0x00D0C28C, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineDownSpeed, 0x008544A6, 0x008544C1,
     "1.2f (00CE3814)", ""},
    {ShipLeafClass::Submarine, "UpDownAccel", 0x00D0C280, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineUpDownAccel, 0x008544E0, 0x008544FB,
     "0.25f (00CE3868)", ""},
    {ShipLeafClass::Submarine, "UpDownRotation", 0x00D0C270, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineUpDownRotation, 0x0085451A, 0x00854535,
     "0.034906585f (00D0C26C, two degrees)", ""},
    {ShipLeafClass::Submarine, "AirRunOutTime", 0x00D0C25C, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineAirRunOutTime, 0x00854554, 0x0085456F,
     "120.0f (00D05804)", ""},
    {ShipLeafClass::Submarine, "AirReloadTime", 0x00D0C24C, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kSubmarineAirReloadTime, 0x0085458E, 0x008545A9,
     "5.0f (00CE3850)", ""},

    // MTorpedoBoat, 00857F40.
    {ShipLeafClass::TorpedoBoat, "TurboTime", 0x00D061F0, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kTorpedoBoatTurboTime, 0x00857F72, 0x00857F90,
     "10.0f (00CE38B8)", ""},
    {ShipLeafClass::TorpedoBoat, "TurboStrength", 0x00D061CC, ShipLeafConversion::NumberOr,
     ShipLeafDescriptorOffsets::kTorpedoBoatTurboStrength, 0x00857FB3, 0x00857FD1,
     "3.0f (00CE3854)", ""},
    {ShipLeafClass::TorpedoBoat, "TurboRechargingTime", 0x00D061DC,
     ShipLeafConversion::NumberOr, ShipLeafDescriptorOffsets::kTorpedoBoatTurboRechargingTime,
     0x00857FF4, 0x00858012, "10.0f (00CE38B8)", ""},
};

std::size_t ship_leaf_field_count() noexcept {
    return sizeof(kShipLeafFieldSchema) / sizeof(kShipLeafFieldSchema[0]);
}

const ShipLeafFieldSpec* ship_leaf_find_field(ShipLeafClass leaf,
                                              const char* path) noexcept {
    if (path == nullptr) {
        return nullptr;
    }
    for (std::size_t i = 0; i < ship_leaf_field_count(); ++i) {
        const ShipLeafFieldSpec& spec = kShipLeafFieldSchema[i];
        if (spec.leaf == leaf && std::strcmp(spec.path, path) == 0) {
            return &spec;
        }
    }
    return nullptr;
}

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

bool submarine_reads_swim_depth1(float periscope_depth_slot) noexcept {
    // COMISS XMM0(0.0f), [ESI+810h] then JBE skips the SwimDepth1 read, so the
    // read happens only when zero is above the slot.
    return !(0.0F <= periscope_depth_slot);
}

LandingShipSubType landing_ship_sub_type(const char* sub_type) noexcept {
    LandingShipSubType result;
    if (equals_insensitive(sub_type, "LCVP")) {
        result.lcvp = true;
        result.other = false;
    } else if (equals_insensitive(sub_type, "LSM")) {
        result.lsm = true;
        result.other = false;
    }
    return result;
}

CargoSubType cargo_sub_type(const char* sub_type) noexcept {
    CargoSubType result;
    result.junk = equals_insensitive(sub_type, "Junk");
    result.troop_transport = equals_insensitive(sub_type, "TroopTransport");
    return result;
}

DeckCameraAngles deck_camera_angles(float horz_angle, float vert_angle) noexcept {
    DeckCameraAngles angles;
    angles.horizontal_radians = -horz_angle;
    angles.vertical_radians = -vert_angle;
    return angles;
}

// ---------------------------------------------------------------------------
// The readers
// ---------------------------------------------------------------------------

void read_battleship_class_fields_006e00f0(VehicleClassFieldHost& lua,
                                           ShipLeafFieldHost& host,
                                           const GuiLuaRef& row,
                                           ShipLeafFields& out) {
    out.leaf = ShipLeafClass::Battleship;
    out.is_battlecruiser = boolean_or(lua, row, "Battlecruiser", false);
    copy_tuning_pair(host, 0x2C, 0x28, out);
}

void read_cargo_class_fields_006eb4a0(VehicleClassFieldHost& lua,
                                      ShipLeafFieldHost& host,
                                      const GuiLuaRef& row,
                                      ShipLeafFields& out) {
    out.leaf = ShipLeafClass::Cargo;
    // The tuning copy comes first here, before the key: 006EB4C6..006EB508.
    copy_tuning_pair(host, 0x34, 0x30, out);
    out.cargo = CargoSubType{};
    const GuiLuaRef field = lua.get_by_name(row, "SubType");
    const VehicleClassLuaValue value = lua.inspect(field);
    if (value.kind == VehicleClassValueKind::String) {
        out.cargo = cargo_sub_type(value.string);
    }
    lua.release(field);
}

void read_cruiser_class_fields_006fb550(VehicleClassFieldHost& lua,
                                        ShipLeafFieldHost& host,
                                        const GuiLuaRef& row,
                                        ShipLeafFields& out) {
    out.leaf = ShipLeafClass::Cruiser;
    out.is_heavy_cruiser = boolean_or(lua, row, "HeavyCruiser", false);
    if (out.is_heavy_cruiser) {
        copy_tuning_pair(host, 0x44, 0x40, out);
    } else {
        copy_tuning_pair(host, 0x3C, 0x38, out);
    }
}

void read_destroyer_class_fields_006fe6a0(ShipLeafFieldHost& host, ShipLeafFields& out) {
    // 006FE6A0 reads no Lua key of its own. After chaining to 00831840 it only
    // copies the tuning pair, which is what makes MDestroyer 0x808 bytes with
    // nothing of its own past the ship base.
    out.leaf = ShipLeafClass::Destroyer;
    copy_tuning_pair(host, 0x0C, 0x08, out);
}

void read_landing_ship_class_fields_0074c630(VehicleClassFieldHost& lua,
                                             ShipLeafFieldHost& host,
                                             const GuiLuaRef& row,
                                             ShipLeafFields& out) {
    out.leaf = ShipLeafClass::LandingShip;
    out.landing_ship_is_big = boolean_or(lua, row, "BigLandingShip", false);
    out.landed_damage = integer_or(lua, row, "LandedDamage", 0);
    out.landed_capture_power = integer_or(lua, row, "LandedCapturePower", 0);
    out.landing_troop_type = integer_or(lua, row, "LandingTroopType", 1);
    out.landing_ship_is_rocketer = boolean_or(lua, row, "Rocketer", false);

    // 0074C796..0074CA53: globals.LandingTroopClasses[LandingTroopType].Classes,
    // walked with IterateFirst/IterateNext and the IsUnbound end test. The
    // element branch is on the element's Type string.
    const GuiLuaRef classes = host.landing_troop_classes(out.landing_troop_type);
    GuiLuaRef key;
    GuiLuaRef value;
    for (bool more = lua.iterate_first(classes, key, value); more;
         more = lua.iterate_next(classes, key, value)) {
        const GuiLuaRef type_field = lua.get_by_name(value, "Type");
        const VehicleClassLuaValue type_value = lua.inspect(type_field);
        const char* type = type_value.string;
        if (type != nullptr && std::strcmp(type, "Soldier") == 0) {
            const GuiLuaRef name_field = lua.get_by_name(value, "ClassName");
            host.register_landing_troop_soldier(lua.inspect(name_field).string);
            lua.release(name_field);
        } else if (type != nullptr && std::strcmp(type, "Vehicle") == 0) {
            const GuiLuaRef id_field = lua.get_by_name(value, "ClassId");
            host.register_landing_troop_vehicle(
                vehicle_class_integer_00b66290(lua.inspect(id_field)));
            lua.release(id_field);
        }
        lua.release(type_field);
    }
    lua.release(classes);

    if (out.landing_ship_is_big) {
        copy_tuning_pair(host, 0x24, 0x20, out);
    } else {
        copy_tuning_pair(host, 0x1C, 0x18, out);
    }

    out.landing_ship_sub_type = LandingShipSubType{};
    const GuiLuaRef sub_type = lua.get_by_name(row, "SubType");
    const VehicleClassLuaValue sub_value = lua.inspect(sub_type);
    if (sub_value.kind == VehicleClassValueKind::String) {
        out.landing_ship_sub_type = landing_ship_sub_type(sub_value.string);
    }
    lua.release(sub_type);
}

void read_mothership_class_fields_00759590(VehicleClassFieldHost& lua,
                                           ShipLeafFieldHost& host,
                                           const GuiLuaRef& row,
                                           ShipLeafFields& out) {
    out.leaf = ShipLeafClass::Mothership;
    out.runway_width = number_bare(lua, row, "RunwayWidth");
    out.runway_length = number_bare(lua, row, "RunwayLength");
    out.is_carrier_escort = boolean_or(lua, row, "CarrierEscort", false);

    const GuiLuaRef planes = lua.get_by_name(row, "MaxLandingPlanes");
    const VehicleClassLuaValue planes_value = lua.inspect(planes);
    out.max_landing_planes =
        planes_value.is_nil() ? 0 : vehicle_class_integer_00b66290(planes_value);
    lua.release(planes);

    const GuiLuaRef camera = lua.get_by_name(row, "DeckCamera");
    const GuiLuaRef position = lua.get_by_name(camera, "Position");
    const VehicleClassLuaValue position_value = lua.inspect(position);
    (void)position_value;  // 00B67A80 fills three floats; the host owns that read.
    lua.release(position);
    const float horz = number_bare(lua, camera, "HorzAngle");
    const float vert = number_bare(lua, camera, "VertAngle");
    out.deck_camera = deck_camera_angles(horz, vert);
    host.store_deck_camera_basis(out.deck_camera);
    lua.release(camera);

    copy_tuning_pair(host, 0x04, 0x00, out);
}

void read_submarine_class_fields_00854230(VehicleClassFieldHost& lua,
                                          ShipLeafFieldHost& host,
                                          const GuiLuaRef& row,
                                          ShipLeafFields& out) {
    out.leaf = ShipLeafClass::Submarine;

    const GuiLuaRef wave = lua.get_by_name(row, "PeriscopeWave");
    const VehicleClassLuaValue wave_value = lua.inspect(wave);
    if (wave_value.is_integer()) {
        out.periscope_wave_handle =
            host.make_effect_handle(vehicle_class_integer_00b66290(wave_value));
    }
    lua.release(wave);

    out.periscope_depth =
        number_or(lua, row, "PeriscopeDepth", ShipLeafDefaults::kAbsentDepth);
    if (submarine_reads_swim_depth1(out.periscope_depth)) {
        out.periscope_depth =
            number_or(lua, row, "SwimDepth1", ShipLeafDefaults::kAbsentDepth);
    }
    out.swim_depth2 = number_or(lua, row, "SwimDepth2", ShipLeafDefaults::kAbsentDepth);
    out.swim_depth3 = number_or(lua, row, "SwimDepth3", ShipLeafDefaults::kAbsentDepth);
    out.periscope_move_range =
        number_or(lua, row, "PeriscopeMoveRange", ShipLeafDefaults::kPeriscopeMoveRange);
    out.up_down_stop_time =
        number_or(lua, row, "UpDownStopTime", ShipLeafDefaults::kUpDownStopTime);
    out.up_speed = number_or(lua, row, "UpSpeed", ShipLeafDefaults::kVerticalSpeed);
    out.down_speed = number_or(lua, row, "DownSpeed", ShipLeafDefaults::kVerticalSpeed);
    out.up_down_accel =
        number_or(lua, row, "UpDownAccel", ShipLeafDefaults::kUpDownAccel);
    out.up_down_rotation =
        number_or(lua, row, "UpDownRotation", ShipLeafDefaults::kUpDownRotation);
    out.air_run_out_time =
        number_or(lua, row, "AirRunOutTime", ShipLeafDefaults::kAirRunOutTime);
    out.air_reload_time =
        number_or(lua, row, "AirReloadTime", ShipLeafDefaults::kAirReloadTime);

    // The only leaf whose four array slots differ: 008545BC..0085461F.
    for (std::uint32_t i = 0; i < ShipLeafTuningOffsets::kTuningArrayCount; ++i) {
        out.tuning.array[i] = host.tuning_block_dword(kSubmarineTuningArraySources[i]);
    }
    out.tuning.scalar = host.tuning_block_dword(kSubmarineTuningScalarSource);
    out.tuning.array_source = kSubmarineTuningArraySources[0];
    out.tuning.scalar_source = kSubmarineTuningScalarSource;
}

void read_torpedo_boat_class_fields_00857f40(VehicleClassFieldHost& lua,
                                             ShipLeafFieldHost& host,
                                             const GuiLuaRef& row,
                                             ShipLeafFields& out) {
    out.leaf = ShipLeafClass::TorpedoBoat;
    out.turbo_time = number_or(lua, row, "TurboTime", ShipLeafDefaults::kTurboTime);
    out.turbo_strength =
        number_or(lua, row, "TurboStrength", ShipLeafDefaults::kTurboStrength);
    out.turbo_recharging_time =
        number_or(lua, row, "TurboRechargingTime", ShipLeafDefaults::kTurboRechargingTime);
    copy_tuning_pair(host, 0x14, 0x10, out);
}

// ---------------------------------------------------------------------------
// Installed-file check
// ---------------------------------------------------------------------------

const ShipLeafKeyCount kShipLeafKeyCounts[] = {
    {"AirReloadTime", 9},
    {"AirRunOutTime", 9},
    {"Battlecruiser", 8},
    {"BigLandingShip", 4},
    {"CarrierEscort", 5},
    {"DeckCamera", 26},
    {"DownSpeed", 9},
    {"HeavyCruiser", 25},
    {"LandedCapturePower", 5},
    {"LandedDamage", 5},
    {"LandingTroopType", 6},
    {"MaxLandingPlanes", 0},
    {"PeriscopeDepth", 8},
    {"PeriscopeMoveRange", 8},
    {"PeriscopeWave", 7},
    {"Rocketer", 1},
    {"RunwayLength", 21},
    {"RunwayWidth", 21},
    {"SubType", 16},
    {"SwimDepth1", 0},
    {"SwimDepth2", 0},
    {"SwimDepth3", 0},
    {"TurboRechargingTime", 24},
    {"TurboStrength", 26},
    {"TurboTime", 26},
    {"UpDownAccel", 0},
    {"UpDownRotation", 0},
    {"UpDownStopTime", 0},
    {"UpSpeed", 9},
};

std::size_t ship_leaf_key_count_count() noexcept {
    return sizeof(kShipLeafKeyCounts) / sizeof(kShipLeafKeyCounts[0]);
}

const char* const kShipLeafUnprovidedKeys[] = {
    "MaxLandingPlanes", "SwimDepth1", "SwimDepth2", "SwimDepth3",
    "UpDownAccel",      "UpDownRotation", "UpDownStopTime",
};

std::size_t ship_leaf_unprovided_key_count() noexcept {
    return sizeof(kShipLeafUnprovidedKeys) / sizeof(kShipLeafUnprovidedKeys[0]);
}

}  // namespace bsp
