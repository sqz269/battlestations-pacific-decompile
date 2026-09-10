#include "bsp/vehicle_class.hpp"

#include <cctype>
#include <cstring>

// Reconstruction of the vehicle-class descriptor system. Evidence, native ABI
// and uncertainties: docs/VEHICLE_CLASS_DESCRIPTORS.md.
namespace bsp {
namespace {

// 00425850 delegates a non-null pair to _stricmp, so the whole comparison chain
// in 00964790 is case-insensitive. This is the ASCII fold that reproduces it for
// the literals in play; the native's is the CRT's, which is locale-aware.
bool equals_ignoring_case_00425850(const char* left, const char* right) noexcept {
    if (left == nullptr || right == nullptr) return left == right;
    while (*left != '\0' && *right != '\0') {
        const unsigned char a = static_cast<unsigned char>(*left++);
        const unsigned char b = static_cast<unsigned char>(*right++);
        if (std::tolower(a) != std::tolower(b)) return false;
    }
    return *left == *right;
}

bool index_in_range(std::int32_t index) noexcept {
    return index >= 0 && index < kVehicleClassIndexMapSize;
}

} // namespace

// The 22 branches of 00964790, in comparison order. descriptor_size is the
// allocation argument at the branch head; instance_size is the operator new
// argument inside the vtable slot +28h the row names. shipped_rows counts the
// rows of the installed Scripts/datatables/autoload/vehicleclasses.lua whose
// Type is this literal, which is 633 rows in total.
const VehicleClassKindRow kVehicleClassKindTable[kVehicleClassKindCount] = {
    {"Destroyer", "MDestroyer", 0x808, 0x1188, VehicleClassKind::Destroyer,
     0x00000000, 0x00D1ACF8, 0x006FE590, 31, 0x0A},
    {"Cruiser", "MCruiser", 0x80C, 0x1188, VehicleClassKind::Cruiser,
     0x00000000, 0x00D1AD38, 0x006FB430, 41, 0x0B},
    {"LandingShip", "MLandingShip", 0x81C, 0x122C, VehicleClassKind::LandingShip,
     0x00963C20, 0x00D1AD78, 0x0074BE00, 6, 0x0C},
    {"Cargo", "MCargo", 0x80C, 0x118C, VehicleClassKind::Cargo,
     0x00963CB0, 0x00D1ADBC, 0x006EB290, 13, 0x0D},
    {"BattleShip", "MBattleship", 0x80C, 0x118C, VehicleClassKind::BattleShip,
     0x00963D30, 0x00D1ADF8, 0x006DFEF0, 39, 0x0E},
    {"Submarine", "MSubmarine", 0x840, 0x1288, VehicleClassKind::Submarine,
     0x00963DB0, 0x00D1AE38, 0x008531A0, 9, 0x0F},
    {"TorpedoBoat", "MTorpedoBoat", 0x814, 0x1190, VehicleClassKind::TorpedoBoat,
     0x00963E40, 0x00D1AE78, 0x00857E20, 3, 0x10},
    {"MotherShip", "MMothership", 0x870, 0x12C8, VehicleClassKind::MotherShip,
     0x00963EC0, 0x00D1AEBC, 0x00758D30, 18, 0x11},
    {"ReconPlane", "MReconPlane", 0x60C, 0x0E94, VehicleClassKind::ReconPlane,
     0x00951AA0, 0x00D19BF4, 0x008091D0, 0, 0x12},
    {"SmallReconPlane", "MSmallReconPlane", 0x60C, 0x0E94, VehicleClassKind::SmallReconPlane,
     0x009536E0, 0x00D1A5A8, 0x0084CA50, 5, 0x13},
    {"LargeReconPlane", "MLargeReconPlane", 0x60C, 0x0E94, VehicleClassKind::LargeReconPlane,
     0x00953770, 0x00D1A5EC, 0x0074E540, 4, 0x14},
    {"Fighter", "MPlaneFighter", 0x60C, 0x0E94, VehicleClassKind::Fighter,
     0x00951AF0, 0x00D19C30, 0x007DDAE0, 28, 0x15},
    {"DiveBomber", "MPlaneDiveBomber", 0x60C, 0x0E94, VehicleClassKind::DiveBomber,
     0x00951B40, 0x00D19C70, 0x00956390, 9, 0x16},
    {"TorpedoBomber", "MPlaneTorpedoBomber", 0x60C, 0x0E94, VehicleClassKind::TorpedoBomber,
     0x00951C20, 0x00D19F4C, 0x009564E0, 9, 0x17},
    {"Kamikaze", "MPlaneKamikaze", 0x60C, 0x0E94, VehicleClassKind::Kamikaze,
     0x00951D00, 0x00D1A224, 0x00956240, 6, 0x18},
    {"LevelBomber", "MPlaneBomber", 0x60C, 0x0E94, VehicleClassKind::LevelBomber,
     0x00951DE0, 0x00D1A4F8, 0x007D7850, 11, 0x19},
    {"AirField", "MAirfield", 0x140, 0x08E4, VehicleClassKind::AirField,
     0x0095FEE0, 0x00D1A9A0, 0x006D3110, 2, 0x1A},
    {"Shipyard", "MShipyard", 0x138, 0x07A4, VehicleClassKind::Shipyard,
     0x0095FF50, 0x00D1A9DC, 0x00848380, 2, 0x1B},
    {"LandVehicle", "MLandVehicle", 0x150, 0x0740, VehicleClassKind::LandVehicle,
     0x0095FFC0, 0x00D1AA18, 0x0074DF10, 21, 0x1C},
    {"LandFort", "MLandFort", 0x180, 0x0758, VehicleClassKind::LandFort,
     0x00749180, 0x00CFF790, 0x00747000, 368, 0x1D},
    {"CommandBuilding", "MCommandBuilding", 0x1AC, 0x07E8, VehicleClassKind::CommandBuilding,
     0x00951E30, 0x00D1A538, 0x006F5C10, 7, 0x1E},
    // The one class whose slot +28h is still the base's 00749150, `xor eax, eax;
    // ret 4`. It builds no instance at all.
    {"DummyTargetVehicle", "MDummyTarget", 0x144, 0x0000, VehicleClassKind::DummyTargetVehicle,
     0x00960050, 0x00D1AA58, 0x00749150, 1, 0x1F},
};

int vehicle_class_kind_index(const char* lua_type) noexcept {
    if (lua_type == nullptr) return kVehicleClassKindUnknown;
    for (int index = 0; index < kVehicleClassKindCount; ++index) {
        if (equals_ignoring_case_00425850(lua_type, kVehicleClassKindTable[index].lua_type)) {
            return index;
        }
    }
    return kVehicleClassKindUnknown;
}

const VehicleClassKindRow* vehicle_class_kind_row(const char* lua_type) noexcept {
    const int index = vehicle_class_kind_index(lua_type);
    return index == kVehicleClassKindUnknown ? nullptr : &kVehicleClassKindTable[index];
}

const VehicleClassKindRow* vehicle_class_kind_row_for(VehicleClassKind kind) noexcept {
    for (const VehicleClassKindRow& row : kVehicleClassKindTable) {
        if (row.kind == kind) return &row;
    }
    return nullptr;
}

void VehicleClassIndexMap::reset_identity_00592652() noexcept {
    for (std::int32_t index = 0; index < kVehicleClassIndexMapSize; ++index) {
        forward[index] = index;
        inverse[index] = index;
    }
}

void VehicleClassIndexMap::remap_00592667(std::int32_t type_id, std::int32_t class_index) noexcept {
    if (!index_in_range(type_id) || !index_in_range(class_index)) return;
    forward[type_id] = class_index;
    inverse[class_index] = type_id;
}

std::int32_t VehicleClassIndexMap::to_class_index(std::int32_t type_id) const noexcept {
    return index_in_range(type_id) ? forward[type_id] : -1;
}

std::int32_t VehicleClassIndexMap::to_type_id(std::int32_t class_index) const noexcept {
    return index_in_range(class_index) ? inverse[class_index] : -1;
}

bool vehicle_class_party_valid_0095ba60(int party) noexcept {
    return party == 0 || party == 1;
}

std::size_t vehicle_class_party_slot_0095ba60(int class_index, int party) noexcept {
    return static_cast<std::size_t>(class_index) * kVehicleClassPartyStride
        + static_cast<std::size_t>(party);
}

int vehicle_class_party_from_race_00964f31(int race) noexcept {
    return (race == 1 || race == 4) ? 1 : 0;
}

std::string vehicle_class_debug_name_00964fbf(bool is_plane, bool enemy_class) {
    std::string name = "Class_";
    if (is_plane) name += enemy_class ? "enemy_" : "own_";
    return name;
}

VehicleClassResolveResult resolve_vehicle_class_00964790(VehicleClassHost& host,
                                                         std::int32_t type_id, bool read_race) {
    VehicleClassResolveResult result{};

    // 009647BB..009647D1. The native reads the map twice through two separate
    // 00437F50 calls and keeps both copies; they can only differ if the map
    // moved under it, which nothing here models.
    const std::int32_t class_index = host.type_to_class_index(type_id);
    result.class_index = class_index;

    // 009647D5..009647E8, then the cache probe at 009647ED.
    if (host.cache_size() <= class_index) host.resize_cache_00437bd0(class_index + 1);
    if (VehicleClassDescriptor* cached = host.cached(class_index)) {
        result.descriptor = cached;
        result.outcome = VehicleClassResolveOutcome::Cached;
        return result;
    }

    const GuiLuaRef globals = host.globals();
    const GuiLuaRef table = host.get_by_name(globals, kVehicleClassTableName);
    const GuiLuaRef row = host.get_by_index(table, class_index);

    const GuiLuaRef type_field = host.get_by_name(row, kVehicleClassTypeKey);
    const char* type_text = host.to_string(type_field);
    const std::string type_name = type_text != nullptr ? std::string(type_text) : std::string();
    host.release(type_field);

    // 009648A9..009648E7. A non-zero LandingShip resolves that class first, with
    // read_race forced on, so the companion exists before this row is built.
    const GuiLuaRef landing_field = host.get_by_name(row, kVehicleClassLandingShipKey);
    const int landing_class = host.to_integer_or(landing_field, 0);
    host.release(landing_field);
    if (landing_class != 0) {
        const VehicleClassResolveResult nested =
            resolve_vehicle_class_00964790(host, landing_class, true);
        result.recursions += 1 + nested.recursions;
    }

    // 009648EC..0096495E: the factory's only writes back into the Lua table.
    host.set_row_int(row, kVehicleClassIdKey, class_index);
    host.set_row_bool(row, kVehicleClassGotKey, true);

    const int kind_index = vehicle_class_kind_index(type_name.c_str());
    if (kind_index == kVehicleClassKindUnknown) {
        // 00964EA6: no literal matched. The native calls 0041DD20, unwinds the
        // Lua objects and returns 0 without touching the cache.
        host.release(row);
        host.release(table);
        host.release(globals);
        result.outcome = VehicleClassResolveOutcome::UnknownType;
        return result;
    }

    const VehicleClassKindRow& kind = kVehicleClassKindTable[kind_index];
    VehicleClassDescriptor* descriptor = host.construct_descriptor(kind_index, kind.descriptor_size);
    if (descriptor == nullptr) {
        // 00964EE8. The native carries a null descriptor into the tail and
        // dereferences it; this stops instead, which is a deliberate divergence.
        host.release(row);
        host.release(table);
        host.release(globals);
        result.outcome = VehicleClassResolveOutcome::AllocationFailed;
        return result;
    }
    descriptor->kind_index = kind_index;

    // 00964EEC..00964F46, only with read_race set.
    if (read_race) {
        const GuiLuaRef race_field = host.get_by_name(row, kVehicleClassRaceKey);
        const int race = host.to_integer_or(race_field, 0);
        host.release(race_field);
        host.mark_party_requires_class(class_index, vehicle_class_party_from_race_00964f31(race));
    }

    // 00964F4B..00964F80: +70h, then the type string copied into +74h.
    descriptor->class_index = class_index;
    descriptor->type_name.assign_0041e870(host.string_storage(), type_name.c_str());

    // 00964F83..00964FBC: NumEngines, or 0 when the key is nil.
    const GuiLuaRef engines_field = host.get_by_name(row, kVehicleClassNumEnginesKey);
    descriptor->engine_count =
        host.is_nil(engines_field) ? 0 : host.to_integer(engines_field);
    host.release(engines_field);

    // 00964FBF..0096506E.
    const bool is_plane = host.descriptor_is_kind(descriptor, kVehicleClassIsPlaneKind);
    const bool enemy = is_plane && host.class_is_enemy(class_index);
    host.register_debug_name(vehicle_class_debug_name_00964fbf(is_plane, enemy), class_index);

    // 009650DA: the Lua load, behind the 00F8A098 nesting counter.
    host.load_descriptor(descriptor);

    // 009650E8..0096513A: resize again, release whatever the slot held, publish.
    if (host.cache_size() <= class_index) host.resize_cache_00437bd0(class_index + 1);
    host.store_cached(class_index, descriptor);

    host.descriptor_activate(descriptor);
    host.descriptor_finalize(descriptor);

    // 00965166..0096518F: ships only, and only once the descriptor is already in
    // the cache, so the re-entry for a self-referencing row terminates.
    if (host.descriptor_is_kind(descriptor, kVehicleClassIsShipKind)
        && descriptor->linked_class_index != -1) {
        const VehicleClassResolveResult nested =
            resolve_vehicle_class_00964790(host, descriptor->linked_class_index, true);
        result.recursions += 1 + nested.recursions;
    }

    host.release(row);
    host.release(table);
    host.release(globals);

    result.descriptor = descriptor;
    result.outcome = VehicleClassResolveOutcome::Constructed;
    return result;
}

} // namespace bsp
