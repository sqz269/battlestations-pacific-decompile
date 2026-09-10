// The shared creation path behind the ten `Type`-keyed scene classes.
// Evidence: docs/SCENE_UNIT_CREATORS.md.
#include "bsp/scene_unit_creators.hpp"

#include <cstddef>

#include "bsp/scene_entity_factory.hpp"

namespace bsp {
namespace {

bool equal_insensitive(const std::string& lhs, const char* rhs) noexcept
{
    if (rhs == nullptr) {
        return false;
    }
    std::size_t i = 0;
    for (; i < lhs.size(); ++i) {
        const char r = rhs[i];
        if (r == '\0') {
            return false;
        }
        char a = lhs[i];
        char b = r;
        if (a >= 'A' && a <= 'Z') {
            a = static_cast<char>(a - 'A' + 'a');
        }
        if (b >= 'A' && b <= 'Z') {
            b = static_cast<char>(b - 'A' + 'a');
        }
        if (a != b) {
            return false;
        }
    }
    return rhs[i] == '\0';
}

// 004F2800's ten `via unit` rows, in class-id order. `register_address` is the
// descriptor's +8 slot from docs/SCENE_ENTITY_FACTORY.md; 004E5BA0 is the body
// LandFort and CommandBuilding share with the typed classes and is not analysed
// here.
const SceneUnitCreatorRow kSceneUnitCreators[kSceneUnitCreatorCount] = {
    {0x07, "DestroyerGen", 0x004f0520, 0x004e98e0, 0, true, nullptr, 9778},
    {0x08, "SubmarineGen", 0x004f05f0, 0x004e98e0, 0, true, nullptr, 330},
    {0x09, "MotherShipGen", 0x004f0860, 0x004e98f0, 0, true, nullptr, 1089},
    {0x0c, "LandingShipGen", 0x004f0790, 0x004e98e0, 0, true, nullptr, 390},
    {0x0e, "TBoatGen", 0x004f06c0, 0x004e98e0, 0, true, nullptr, 652},
    {0x18, "PlaneSquadronGen", 0x004f0ad0, 0x004e6bc0, kPlaneSquadronInstanceSize, true, nullptr,
     2131},
    {0x1b, "LandFort", 0x004f0fb0, 0x004e5ba0, 0, false, kSceneUnitStationaryKey, 72615},
    {0x1c, "CommandBuilding", 0x004f10b0, 0x004e5ba0, 0, false, nullptr, 649},
    {0x45, "AirField", 0x004f0930, 0x004e9900, 0, true, nullptr, 256},
    {0x46, "Shipyard", 0x004f0a00, 0x004e9910, 0, true, nullptr, 243},
};

// 00964790's comparison chain, in the order 00425850 is called. The SEH state
// byte confirms the order independently: 0Ah at the Destroyer branch through 1Fh
// at DummyTargetVehicle.
const VehicleClassKindRow kVehicleClassKinds[kVehicleClassKindCount] = {
    {"Destroyer", 0x808, 0x00963380, 0x0a, 31},
    {"Cruiser", 0x80c, 0, 0x0b, 41},
    {"LandingShip", 0x81c, 0x00963c20, 0x0c, 6},
    {"Cargo", 0x80c, 0x00963cb0, 0x0d, 13},
    {"BattleShip", 0x80c, 0x00963d30, 0x0e, 39},
    {"Submarine", 0x840, 0x00963db0, 0x0f, 9},
    {"TorpedoBoat", 0x814, 0x00963e40, 0x10, 3},
    {"MotherShip", 0x870, 0x00963ec0, 0x11, 18},
    {"ReconPlane", 0x60c, 0x00951aa0, 0x12, 0},
    {"SmallReconPlane", 0x60c, 0x009536e0, 0x13, 5},
    {"LargeReconPlane", 0x60c, 0x00953770, 0x14, 4},
    {"Fighter", 0x60c, 0x00951af0, 0x15, 28},
    {"DiveBomber", 0x60c, 0x00951b40, 0x16, 9},
    {"TorpedoBomber", 0x60c, 0x00951c20, 0x17, 9},
    {"Kamikaze", 0x60c, 0x00951d00, 0x18, 6},
    {"LevelBomber", 0x60c, 0x00951de0, 0x19, 11},
    {"AirField", 0x140, 0x0095fee0, 0x1a, 2},
    {"Shipyard", 0x138, 0x0095ff50, 0x1b, 2},
    {"LandVehicle", 0x150, 0x0095ffc0, 0x1c, 21},
    {"LandFort", 0x180, 0x00749180, 0x1d, 368},
    {"CommandBuilding", 0x1ac, 0x00951e30, 0x1e, 7},
    {"DummyTargetVehicle", 0x144, 0x00960050, 0x1f, 1},
};

void erase_property(ScenePropertyBlock& bag, const char* key)
{
    for (std::size_t i = bag.values.size(); i > 0; --i) {
        if (equal_insensitive(bag.values[i - 1].key, key)) {
            bag.values.erase(bag.values.begin() + static_cast<std::ptrdiff_t>(i - 1));
        }
    }
}

void set_property(ScenePropertyBlock& bag, const char* key, const char* type_letter,
                  std::vector<std::string> values)
{
    SceneProperty prop;
    prop.key = key;
    prop.type_letter = type_letter;
    prop.values = std::move(values);
    bag.values.push_back(std::move(prop));
}

std::string decimal(int value)
{
    if (value == 0) {
        return "0";
    }
    const bool negative = value < 0;
    unsigned int magnitude = negative ? 0u - static_cast<unsigned int>(value)
                                      : static_cast<unsigned int>(value);
    char buffer[16];
    std::size_t used = 0;
    while (magnitude != 0u && used < sizeof(buffer)) {
        buffer[used++] = static_cast<char>('0' + (magnitude % 10u));
        magnitude /= 10u;
    }
    std::string out;
    if (negative) {
        out.push_back('-');
    }
    while (used > 0) {
        out.push_back(buffer[--used]);
    }
    return out;
}

}  // namespace

const SceneUnitCreatorRow* scene_unit_creator_table() noexcept
{
    return kSceneUnitCreators;
}

const SceneUnitCreatorRow* find_scene_unit_creator(int class_id) noexcept
{
    for (int i = 0; i < kSceneUnitCreatorCount; ++i) {
        if (kSceneUnitCreators[i].class_id == class_id) {
            return &kSceneUnitCreators[i];
        }
    }
    return nullptr;
}

const SceneUnitCreatorRow* find_scene_unit_creator(const std::string& scene_class) noexcept
{
    for (int i = 0; i < kSceneUnitCreatorCount; ++i) {
        if (equal_insensitive(scene_class, kSceneUnitCreators[i].scene_class)) {
            return &kSceneUnitCreators[i];
        }
    }
    return nullptr;
}

const VehicleClassKindRow* vehicle_class_kind_table() noexcept
{
    return kVehicleClassKinds;
}

const VehicleClassKindRow* find_vehicle_class_kind(const std::string& type_name) noexcept
{
    // 00425850 compares the native string with the literal; the first match in the
    // chain wins and the rest are never tested.
    for (int i = 0; i < kVehicleClassKindCount; ++i) {
        if (type_name == kVehicleClassKinds[i].type_name) {
            return &kVehicleClassKinds[i];
        }
    }
    return nullptr;
}

int vehicle_class_index_from_type(const VehicleClassRegistry& registry, int type_id) noexcept
{
    if (type_id < 0 || static_cast<unsigned int>(type_id) >= kVehicleClassIndexTableEntries) {
        return -1;
    }
    if (static_cast<std::size_t>(type_id) >= registry.type_to_class_index.size()) {
        return -1;
    }
    return registry.type_to_class_index[static_cast<std::size_t>(type_id)];
}

bool vehicle_class_cache_hit(const VehicleClassRegistry& registry, int class_index) noexcept
{
    if (class_index < 0 || static_cast<std::size_t>(class_index) >= registry.descriptor_cache.size()) {
        return false;
    }
    return registry.descriptor_cache[static_cast<std::size_t>(class_index)] != nullptr;
}

bool mark_party_requires_class_0095ba60(VehicleClassRegistry& registry, int type_id,
                                        int party) noexcept
{
    // 0095BA65: `TEST EBX,EBX ; JZ ok ; CMP EBX,1 ; JNZ out`.
    if (party != 0 && party != 1) {
        return false;
    }
    const int class_index = vehicle_class_index_from_type(registry, type_id);
    if (class_index < 0) {
        return false;
    }
    const std::size_t slot = static_cast<std::size_t>(class_index);
    if (slot >= registry.party_required.size()) {
        registry.party_required.resize(slot + 1);
    }
    registry.party_required[slot][static_cast<std::size_t>(party)] = 1;
    return true;
}

SceneEnumProperty scene_enum_property(const ScenePropertyBlock& bag,
                                      const std::string& key) noexcept
{
    SceneEnumProperty out;
    const SceneProperty* prop = bag.find(key);
    if (prop == nullptr) {
        return out;
    }
    out.present = true;
    // `E <table> : <symbol>`; ':' is one of kSceneDelimiters, so it is its own token.
    for (const std::string& value : prop->values) {
        if (value == ":") {
            continue;
        }
        if (out.table.empty()) {
            out.table = value;
        } else {
            out.symbol = value;
        }
    }
    if (out.symbol.empty()) {
        out.symbol = out.table;
        out.table.clear();
    }
    return out;
}

std::string vehicle_class_launch_lua_path(int type_id)
{
    // 004E9733..004E98A0: "VehicleClass." + itoa(type) + ".Catapult.LaunchedClass",
    // built by two 0041E870 assigns and two concatenations.
    return std::string("VehicleClass.") + decimal(type_id) + ".Catapult.LaunchedClass";
}

SceneUnitCommand scene_unit_command_004e6b30(const ScenePropertyBlock& bag)
{
    SceneUnitCommand out;
    // 004E6B44: the outer property must carry type code 6, the nested-block code.
    const ScenePropertyBlock* sub = find_scene_property_block(bag, kSceneUnitCommandKey);
    if (sub == nullptr) {
        return out;
    }
    const SceneProperty* command = sub->find(kSceneUnitCommandKey);
    if (command == nullptr) {
        // 004E6B6D: a sub-bag without an inner `Command` queues nothing. Every
        // `"Command" { CommandTarget = R "" ; }` in the shipped files lands here.
        return out;
    }
    out.queued = true;
    if (!command->values.empty()) {
        out.command = command->values.back();
    }
    const SceneProperty* target = sub->find(kSceneUnitCommandTargetKey);
    if (target != nullptr && !target->values.empty() && !target->values.back().empty()) {
        out.target = target->values.back();
    }
    return out;
}

SceneHierarchyDeferral defer_hierarchy_004f03c0(bool has_parent, const std::string& parent_name,
                                                const float local_frame[16]) noexcept
{
    SceneHierarchyDeferral out;
    out.set_hierarchy = true;
    out.wrote_parent = has_parent;
    if (has_parent) {
        out.parent_name = parent_name;
    }
    if (local_frame != nullptr) {
        for (int i = 0; i < 16; ++i) {
            out.matrix[static_cast<std::size_t>(i)] = local_frame[i];
        }
    }
    return out;
}

void apply_hierarchy_deferral(ScenePropertyBlock& bag, const SceneHierarchyDeferral& deferral)
{
    // 004F03DB..004F0431: erase all three, then write SetHierarchy = 1.
    erase_property(bag, kSceneHierarchyFlagKey);
    erase_property(bag, kSceneHierarchyParentKey);
    erase_property(bag, kSceneHierarchyMatrixKey);
    if (deferral.set_hierarchy) {
        set_property(bag, kSceneHierarchyFlagKey, "B", {"true"});
    }
    if (deferral.wrote_parent) {
        set_property(bag, kSceneHierarchyParentKey, "S", {deferral.parent_name});
    }
    std::vector<std::string> matrix;
    matrix.reserve(16);
    for (std::size_t i = 0; i < deferral.matrix.size(); ++i) {
        matrix.push_back(decimal(static_cast<int>(deferral.matrix[i])));
    }
    set_property(bag, kSceneHierarchyMatrixKey, "M", std::move(matrix));
}

SceneUnitRegistrationResult register_scene_unit_004e96d0(const SceneUnitRegistrationInputs& inputs,
                                                        VehicleClassRegistry& registry)
{
    SceneUnitRegistrationResult out;
    out.marked_type = mark_party_requires_class_0095ba60(registry, inputs.type_id, inputs.party);

    // 004E9720..004E972D: present, type code 0, and non-negative wins outright.
    int launch = -1;
    if (inputs.has_launch_class_id && inputs.launch_class_id >= 0) {
        launch = inputs.launch_class_id;
    } else {
        launch = inputs.lua_launched_class;
    }

    // 004E98A7: `CMP ESI,-1 ; JLE` - strictly greater than -1.
    if (launch > -1) {
        out.launch_class = launch;
        out.marked_launch_class =
            mark_party_requires_class_0095ba60(registry, launch, inputs.party);
        out.registered_hidden_stock = true;
    }
    return out;
}

SceneUnitRegistrationResult register_plane_squadron_004e6bc0(int type_id, int party,
                                                             VehicleClassRegistry& registry)
{
    SceneUnitRegistrationResult out;
    out.marked_type = mark_party_requires_class_0095ba60(registry, type_id, party);
    // 004E6BEE registers the entity's own Type, not a launched class.
    out.launch_class = type_id;
    out.registered_hidden_stock = true;
    return out;
}

namespace {

// The tail 004F0520, its six clones, 004F0FB0, 004F10B0 and 004F0AD0 all share.
void finish_scene_unit(const SceneUnitCreatorRow& row, const SceneUnitCreationInputs& inputs,
                       SceneUnitCreatorHost& host, SceneUnitCreationResult& result)
{
    if (result.instance == nullptr) {
        return;
    }
    if (host.placement_deferred()) {
        result.hierarchy_deferred = true;
        const bool has_parent = inputs.hierarchy_parent != nullptr;
        const std::string parent_name =
            has_parent ? host.hierarchy_parent_name(inputs.hierarchy_parent) : std::string();
        result.deferral = defer_hierarchy_004f03c0(has_parent, parent_name, inputs.local_frame);
    } else {
        host.place_instance(result.instance, inputs.hierarchy_parent, host.world_parent_node(),
                            inputs.local_frame);
    }

    host.set_instance_name(result.instance, inputs.entity_name);

    if (row.applies_command_property && inputs.properties != nullptr) {
        result.command = scene_unit_command_004e6b30(*inputs.properties);
        if (result.command.queued) {
            host.queue_entity_command(result.instance, result.command.command,
                                      result.command.target);
        }
    }
}

}  // namespace

SceneUnitCreationResult create_scene_unit_004f0520(const SceneUnitCreatorRow& row,
                                                   const SceneUnitCreationInputs& inputs,
                                                   SceneUnitCreatorHost& host)
{
    SceneUnitCreationResult result;
    result.descriptor = host.vehicle_class_descriptor(inputs.type_id, true);
    if (result.descriptor == nullptr) {
        // 00964790 returns 0 when the row's Type matches none of the 22 literals;
        // the creator then dereferences it. The reconstruction stops instead.
        return result;
    }

    // 004F0FC5: only LandFort has the `Stationary` split.
    bool stationary = false;
    if (row.extra_key != nullptr && equal_insensitive(kSceneUnitStationaryKey, row.extra_key) &&
        inputs.properties != nullptr) {
        const SceneProperty* prop = inputs.properties->find(kSceneUnitStationaryKey);
        stationary = scene_property_bool(prop);
    }

    if (stationary) {
        result.used_stationary_variant = true;
        result.instance = host.create_stationary_instance(result.descriptor);
    } else {
        result.instance = host.create_instance_from_descriptor(result.descriptor);
    }

    finish_scene_unit(row, inputs, host, result);
    return result;
}

SceneUnitCreationResult create_plane_squadron_004f0ad0(const SceneUnitCreationInputs& inputs,
                                                       SceneUnitCreatorHost& host)
{
    SceneUnitCreationResult result;
    // 004F0AEA: the instance is allocated before `Type` is even read.
    result.instance = host.create_squadron_instance(kPlaneSquadronInstanceSize);
    // 004F0B3C: called for the cache side effect; EDI still holds the squadron at
    // the placement call, so the return value is discarded.
    result.descriptor = host.vehicle_class_descriptor(inputs.type_id, true);

    const SceneUnitCreatorRow* row = find_scene_unit_creator(0x18);
    if (row != nullptr) {
        finish_scene_unit(*row, inputs, host, result);
    }
    return result;
}

}  // namespace bsp
