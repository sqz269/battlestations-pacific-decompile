#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_pose_binding.hpp"

#include <cctype>
#include <cstring>
#include <stdexcept>

namespace bsp {
namespace {

bool equal_insensitive_ascii(const std::string& a, const char* b) noexcept
{
    if (b == nullptr) {
        return false;
    }
    std::size_t i = 0;
    for (; i < a.size(); ++i) {
        const unsigned char lhs = static_cast<unsigned char>(a[i]);
        const unsigned char rhs = static_cast<unsigned char>(b[i]);
        if (rhs == 0) {
            return false;
        }
        if (std::tolower(lhs) != std::tolower(rhs)) {
            return false;
        }
    }
    return b[i] == 0;
}

// 004F2800, in registration order. name / id / create / postCreate come from the
// four pushes of each 004EE250 call; instance_size and creator_kind come from the
// creator's prologue (operator new at 00BF55BE, unit-class factory at 00964790).
constexpr SceneEntityClassRow kClasses[kSceneEntityClassCount] = {
    {0x07, "DestroyerGen", 0x004f0520, 0x004e98e0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x08, "SubmarineGen", 0x004f05f0, 0x004e98e0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x09, "MotherShipGen", 0x004f0860, 0x004e98f0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x45, "AirField", 0x004f0930, 0x004e9900, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x46, "Shipyard", 0x004f0a00, 0x004e9910, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x0e, "TBoatGen", 0x004f06c0, 0x004e98e0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x0c, "LandingShipGen", 0x004f0790, 0x004e98e0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x18, "PlaneSquadronGen", 0x004f0ad0, 0x004e6bc0, 0x414, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x41, "NavPoint", 0x004e99b0, 0x004e5b00, 0x1e4, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x42, "MovieCamPos", 0x004e9ae0, 0x004e5b00, 0x1e4, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x43, "MovieCamLookat", 0x004e9c10, 0x004e5b00, 0x1e4, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x1d, "LandingPoint", 0x004e9d40, 0x004e5b00, 0x224, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x36, "Stationary", 0x004f0be0, 0x004e5b00, 0, SceneEntityCreatorKind::TypedResource, "Type"},
    {0x3b, "Wreck", 0x004f0cc0, 0x004e5b00, 0, SceneEntityCreatorKind::TypedResource, "WreckType"},
    {0x3d, "Cloud", 0x004e9e40, 0x004e9920, 0, SceneEntityCreatorKind::TypedResource, "CloudType"},
    {0x34, "WaterMine", 0x004e9f80, 0x004e5b00, 0, SceneEntityCreatorKind::TypedResource, "Type"},
    {0x44, "Landscape", 0x004f1460, 0x004e5b00, 0x430, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x1a, "LandConvoy", 0x004f2700, 0x004ee300, 0x3cc, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x1b, "LandFort", 0x004f0fb0, 0x004e5ba0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x1c, "CommandBuilding", 0x004f10b0, 0x004e5ba0, 0, SceneEntityCreatorKind::UnitClassFactory, "Type"},
    {0x47, "Path", 0x004ea650, 0x004e5b00, 0x210, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x4a, "CameraPath", 0x004ea760, 0x004e5b00, 0x210, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x5d, "FreeCamPos", 0x004e5bb0, 0x004e5b00, 0, SceneEntityCreatorKind::TypedResource, nullptr},
    {0x4d, "SpawnPoint", 0x004f1a60, 0x004ea080, 0x348, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x5b, "SimpleEffect", 0x004f0db0, 0x004e5b00, 0x1ec, SceneEntityCreatorKind::FixedInstance, nullptr},
    {0x5c, "PeriodicEffect", 0x004f0eb0, 0x004e5b00, 0x228, SceneEntityCreatorKind::FixedInstance, nullptr},
};

// Slot order of the play-area rows, indexed by the game mode. -1 means the mode
// has no area test.
constexpr int kModeAreaSlot[11] = {0, 1, 2, 3, 6, 7, 4, 5, -1, -1, -1};

constexpr const char* kModeKey[11] = {
    "MultiIslandCapture1v1", "MultiIslandCapture2v2", "MultiIslandCapture3v3",
    "MultiIslandCapture4v4", "MultiDuel",             "MultiEscort",
    "MultiSiege",            "MultiCompetitive",      nullptr,
    nullptr,                 nullptr,
};

bool mode_in_range(SceneGameMode mode) noexcept
{
    const int value = static_cast<int>(mode);
    return value >= 0 && value <= 10;
}

}  // namespace

const SceneEntityClassRow* scene_entity_class_table() noexcept
{
    return kClasses;
}

const SceneEntityClassRow* find_scene_entity_class_by_id(int class_id) noexcept
{
    for (const SceneEntityClassRow& row : kClasses) {
        if (row.class_id == class_id) {
            return &row;
        }
    }
    return nullptr;
}

const SceneEntityClassRow* find_scene_entity_class_by_name(const std::string& name) noexcept
{
    for (const SceneEntityClassRow& row : kClasses) {
        if (equal_insensitive_ascii(name, row.name)) {
            return &row;
        }
    }
    return nullptr;
}

int scene_entity_class_id_from_name(const std::string& name) noexcept
{
    const SceneEntityClassRow* row = find_scene_entity_class_by_name(name);
    return row != nullptr ? row->class_id : kSceneUnknownClassId;
}

const char* scene_entity_class_name_from_id(int class_id) noexcept
{
    const SceneEntityClassRow* row = find_scene_entity_class_by_id(class_id);
    return row != nullptr ? row->name : nullptr;
}

bool scene_entity_class_is_registered(const std::string& name) noexcept
{
    return find_scene_entity_class_by_name(name) != nullptr;
}

bool scene_registration_pass_handles_class(int class_id) noexcept
{
    for (const int id : kSceneRegistrationPassClassIds) {
        if (id == class_id) {
            return true;
        }
    }
    return false;
}

bool scene_registration_fallback_class(int class_id) noexcept
{
    for (const int id : kSceneRegistrationFallbackClassIds) {
        if (id == class_id) {
            return true;
        }
    }
    return false;
}

int scene_mode_area_slot(SceneGameMode mode) noexcept
{
    if (!mode_in_range(mode)) {
        return -1;
    }
    return kModeAreaSlot[static_cast<int>(mode)];
}

std::uint32_t scene_mode_area_offset(SceneGameMode mode) noexcept
{
    const int slot = scene_mode_area_slot(mode);
    if (slot < 0) {
        return 0;
    }
    return kSceneModeAreaBase + static_cast<std::uint32_t>(slot) * kSceneModeAreaStride;
}

const char* scene_mode_property_key(SceneGameMode mode) noexcept
{
    if (!mode_in_range(mode)) {
        return nullptr;
    }
    return kModeKey[static_cast<int>(mode)];
}

bool scene_point_in_mode_area(const SceneModeArea& area, float x, float z) noexcept
{
    // 0046C782..0046C7CA and its seven copies: x < f[3], f[0] < x, f[5] < z,
    // z < f[2]. Every compare is FCOMIP/JBE, so all four bounds are strict.
    return x < area.bounds[3] && area.bounds[0] < x && area.bounds[5] < z && z < area.bounds[2];
}

const ScenePropertyBlock* find_scene_property_block(const ScenePropertyBlock& bag,
                                                    const std::string& key) noexcept
{
    for (const std::pair<std::string, ScenePropertyBlock>& entry : bag.blocks) {
        if (equal_insensitive_ascii(key, entry.first.c_str())) {
            return &entry.second;
        }
    }
    return nullptr;
}

bool scene_property_bool(const SceneProperty* prop) noexcept
{
    if (prop == nullptr || prop->values.empty()) {
        return false;
    }
    return equal_insensitive_ascii(prop->values.front(), "true");
}

namespace {

std::string party_token(const ScenePropertyBlock& bag)
{
    // 0046C5AC and 0046CB59 read `Party` off the entity bag, not the MultiType
    // sub-bag, and dereference the record at +0Ch without a null check. The
    // authored form is `Party = E Party : <name> ;`; the native keeps the enum
    // value the property descriptor resolved, which the parsed document does not
    // carry, so the authored token stands in for it.
    const SceneProperty* prop = bag.find("Party");
    if (prop == nullptr || prop->values.empty()) {
        return std::string();
    }
    return prop->values.back();
}

void build_record(const SceneEntityGateInputs& inputs,
                  const ScenePropertyBlock& bag,
                  SceneEntityGateHost& host,
                  SceneEntityGateResult& result,
                  void* captured_parent_identity,
                  const CameraMatrix& local_frame,
                  const CameraMatrix& parent_argument)
{
    if (inputs.record_already_built) {
        return;
    }
    SceneDeferredEntityRecord record;
    record.class_name = inputs.class_name;
    record.entity_name = inputs.entity_name;
    record.party = party_token(bag);
    std::memcpy(record.local_frame, local_frame.data(), sizeof(record.local_frame));
    std::memcpy(record.parent_frame, parent_argument.data(), sizeof(record.parent_frame));
    if (captured_parent_identity) {
        record.parent_name = host.parent_name(captured_parent_identity);
    }
    host.append_deferred_entity_record(record);
    result.deferred_record_created = true;
}

}  // namespace

SceneEntityGateResult scene_entity_generation_gate_0046c550(
    const SceneEntityGateInputs& inputs,
    const std::vector<int>& always_generate_class_ids,
    SceneEntityGateHost& host, PoseRefreshResolver& poses)
{
    // Native argument3 is passed by value. Keep that same borrowed identity
    // through later host calls; do not reload a caller-mutated input projection.
    void* const parent_identity = inputs.parent_identity;
    const CameraMatrix* const local_frame = inputs.local_frame;
    const CameraMatrix* const parent_frame = inputs.parent_frame;
    if (!local_frame || !parent_frame)
        throw std::invalid_argument("scene generation gate requires actual local and parent frame bindings");
    // Native callers reserve 40h and REP MOVSD sixteen DWORDs into arguments
    // 7..22 before entering this callee. Capture bytes, with no x87 conversion.
    CameraMatrix parent_argument;
    std::memcpy(parent_argument.data(), parent_frame->data(), sizeof(parent_argument));
    SceneEntityGateResult result;
    static const ScenePropertyBlock kEmptyBag;
    const ScenePropertyBlock& bag = inputs.properties != nullptr ? *inputs.properties : kEmptyBag;

    // 0046C57A: X and Z of the entity's own frame, before any parent is folded in.
    float x = (*local_frame)[12];
    float z = (*local_frame)[14];

    // 0046C58C..0046C5A6. The absence of the `MultiType` sub-bag, not its
    // contents, takes the deferred-record branch, which always generates.
    const ScenePropertyBlock* multi = find_scene_property_block(bag, kSceneMultiTypeKey);
    if (multi == nullptr) {
        build_record(inputs, bag, host, result, parent_identity, *local_frame, parent_argument);
        result.generate = true;
        result.rule = SceneGateRule::NoMultiTypeBlock;
        return result;
    }

    // 0046C6A9: with a parent the world position is the parent's own position
    // plus the entity's local offset; without one the local frame is multiplied
    // by the parent frame that came in by value.
    if (parent_identity) {
        add_scene_parent_world_offset_0046c6b7(parent_identity, poses, x, z);
    } else {
        compose_scene_null_parent_offset_0046c6e5(*local_frame, parent_argument, x, z);
    }

    // 0046C716 then 0046C741: the class name goes back through the registry and
    // the id is looked up in the set at this+164h. A hit generates unconditionally.
    const int class_id = scene_entity_class_id_from_name(inputs.class_name);
    for (const int allowed : always_generate_class_ids) {
        if (allowed == class_id) {
            result.generate = true;
            result.rule = SceneGateRule::ClassAlwaysGenerated;
            return result;
        }
    }

    const SceneGameMode mode = host.effective_game_mode();
    if (!mode_in_range(mode)) {
        result.rule = SceneGateRule::UnknownGameMode;
        return result;
    }

    if (mode == SceneGameMode::Unrestricted) {
        result.generate = true;
        result.rule = SceneGateRule::Unrestricted;
        return result;
    }

    // 0046CC5B: mode 8 registers the stock and generates, with no area test and
    // no deferred record.
    if (mode == SceneGameMode::InGameGeneration) {
        if (scene_property_bool(bag.find("GenerateInGame"))) {
            host.register_multiplayer_stock(bag, inputs.class_name);
            result.stock_registered = true;
            result.generate = true;
        }
        result.rule = SceneGateRule::GenerateInGame;
        return result;
    }

    // 0046CB43: mode 9 does the same but also builds the deferred record, and
    // only falls through to `GenerateInEngineMovie` when the record is missing.
    if (mode == SceneGameMode::EngineMovie) {
        if (scene_property_bool(bag.find("GenerateInGame"))) {
            build_record(inputs, bag, host, result, parent_identity, *local_frame, parent_argument);
            host.register_multiplayer_stock(bag, inputs.class_name);
            result.stock_registered = true;
            if (result.deferred_record_created || inputs.record_already_built) {
                result.generate = true;
                result.rule = SceneGateRule::GenerateInGame;
                return result;
            }
        }
        result.generate = scene_property_bool(bag.find("GenerateInEngineMovie"));
        result.rule = SceneGateRule::EngineMovieFallback;
        return result;
    }

    const int slot = scene_mode_area_slot(mode);
    const SceneModeArea area = host.mode_area(slot);
    if (!scene_point_in_mode_area(area, x, z)) {
        result.rule = SceneGateRule::OutsideModeArea;
        return result;
    }

    // Inside the area the answer is the mode's own boolean out of the MultiType
    // sub-bag, read at +0Ch without a null check.
    result.generate = scene_property_bool(multi->find(scene_mode_property_key(mode)));
    result.rule = SceneGateRule::InsideModeArea;
    return result;
}

}  // namespace bsp
