// The eight side blocks of the mission scene record, and the rule that fills
// them. See include/bsp/scene_record_side_blocks.hpp and
// docs/SCENE_RECORD_SIDE_BLOCKS.md for the addresses and the evidence.

#include "bsp/scene_record_side_blocks.hpp"

#include "bsp/scene_entity_factory.hpp"

#include <cstdlib>

namespace bsp {
namespace {

// 00e082ec holds eight pointers in descending order ("Player8" first), so the
// table as indexed by 004e82f6 reads Player1..Player8.
const char* const kPlayerKeys[kSceneRecordSideBlockCount]
    = {"Player1", "Player2", "Player3", "Player4", "Player5", "Player6", "Player7", "Player8"};

// The applier reads `*(node + 0xc)` without checking the declared type, so a
// value authored with the wrong letter still lands in the record. The rule
// below reproduces the value only for the integer letters, where the dword and
// the authored token agree; anything else is reported as not written.
bool integer_letter(const std::string& letter) noexcept
{
    return letter == "I" || letter == "i";
}

std::string indexed_key(const char* prefix, std::size_t index)
{
    std::string key(prefix);
    key.push_back(static_cast<char>('0' + static_cast<int>(index)));
    return key;
}

} // namespace

const char* scene_slot_block_key(std::size_t index) noexcept
{
    if (index >= kSceneRecordSideBlockCount) {
        return "";
    }
    return kPlayerKeys[index];
}

std::string scene_slot_unit_key(std::size_t index)
{
    return indexed_key(kSceneSlotUnitKeyPrefix, index);
}

std::string scene_slot_pool_key(std::size_t index)
{
    return indexed_key(kSceneSlotPoolKeyPrefix, index);
}

std::size_t scene_record_side_block_offset(std::size_t index) noexcept
{
    // 004e8321: LEA EAX,[ESI + ESI*8] then SHL EAX,5 is index * 0x120, and
    // 004e8327 adds the record and the constant 4.
    return kSceneRecordSideBlockBase + index * kSceneRecordSideBlockStride;
}

std::size_t scene_side_block_unit_offset(std::size_t unit) noexcept
{
    return kSceneSideBlockUnitBase + unit * kSceneSideBlockUnitStride;
}

std::size_t scene_side_block_pool_offset(std::size_t unit, std::size_t pool) noexcept
{
    return scene_side_block_unit_offset(unit) + kSceneSlotUnitPoolBase
        + pool * kSceneSlotUnitPoolStride;
}

std::int32_t scene_property_int32(const SceneProperty* prop) noexcept
{
    if (prop == nullptr || prop->values.empty() || !integer_letter(prop->type_letter)) {
        return 0;
    }
    return static_cast<std::int32_t>(std::strtol(prop->values.front().c_str(), nullptr, 10));
}

std::string scene_property_text(const SceneProperty* prop)
{
    if (prop == nullptr || prop->values.empty()) {
        return std::string();
    }
    return prop->values.front();
}

SceneSlotEnumValue scene_property_enum_value(const SceneProperty* prop)
{
    SceneSlotEnumValue value;
    if (prop == nullptr) {
        return value;
    }
    value.present = true;
    // `Party = E Party : Japanese ;`. ':' is in the delimiter set, so the
    // lexer hands back three tokens and the symbol is the last one.
    if (!prop->values.empty()) {
        value.type = prop->values.front();
    }
    if (prop->values.size() >= 3) {
        value.symbol = prop->values.back();
    }
    return value;
}

SceneSlotBlock read_scene_slot_block_004e82f0(const ScenePropertyBlock& multiplay,
    std::size_t index)
{
    SceneSlotBlock block;
    if (index >= kSceneRecordSideBlockCount) {
        return block;
    }
    // 004e8300..004e8311: the lookup must find a node whose +4h type tag is 6,
    // the group tag. Anything else returns before the record is touched.
    const ScenePropertyBlock* player
        = find_scene_property_block(multiplay, scene_slot_block_key(index));
    if (player == nullptr) {
        return block;
    }
    block.present = true;

    // 004e832b..004e834f. Both dwords are stored unconditionally; the native
    // dereferences the lookup result with no null check, so a `PlayerN` group
    // without `Party` or `Race` faults rather than defaulting.
    block.party = scene_property_enum_value(player->find(kSceneSlotPartyKey));
    block.race = scene_property_enum_value(player->find(kSceneSlotRaceKey));

    for (std::size_t unit = 0; unit < kSceneSideBlockUnitCount; ++unit) {
        SceneSlotUnit& out = block.units[unit];
        // 004e8384: the icon dword is set to -1 before the `UnitN` test, so it
        // is the one field an absent unit still overwrites.
        out.icon = kSceneSlotAbsentIcon;
        const ScenePropertyBlock* unit_bag
            = find_scene_property_block(*player, scene_slot_unit_key(unit));
        if (unit_bag == nullptr) {
            continue;
        }
        out.present = true;
        out.name = scene_property_text(unit_bag->find(kSceneSlotNameKey));
        out.icon = scene_property_int32(unit_bag->find(kSceneSlotIconKey));
        const SceneProperty* class_id = unit_bag->find(kSceneSlotClassIdKey);
        out.class_id = scene_property_int32(class_id);
        out.class_id_written = class_id != nullptr;
        out.class_name = scene_property_text(unit_bag->find(kSceneSlotClassNameKey));

        for (std::size_t pool = 0; pool < kSceneSlotUnitPoolCount; ++pool) {
            SceneSlotPoolEntry& slot = out.pools[pool];
            const ScenePropertyBlock* pool_bag
                = find_scene_property_block(*unit_bag, scene_slot_pool_key(pool));
            if (pool_bag == nullptr) {
                // 004e84d1 writes -1 into the icon dword and leaves the count
                // dword at whatever the block already held.
                slot.icon = kSceneSlotAbsentIcon;
                continue;
            }
            slot.present = true;
            slot.icon = scene_property_int32(pool_bag->find(kSceneSlotIconKey));
            slot.count = scene_property_int32(pool_bag->find(kSceneSlotNumKey));
            slot.count_written = true;
        }
    }
    return block;
}

SceneRecordSlotTable read_scene_record_slot_table_004f1d70(
    const ScenePropertyBlock& scene_root_props)
{
    SceneRecordSlotTable table;

    // 004f2196: the key is read through a presence test first, so an absent
    // `CompetitiveModeParty` stores 0 instead of faulting.
    const SceneProperty* competitive
        = scene_root_props.find(kSceneCompetitiveModePartyKey);
    table.competitive_mode_party = scene_property_int32(competitive);

    // 004f2145: an absent `MaxPlayerNum` stores the literal 8.
    const SceneProperty* max_players = scene_root_props.find(kSceneMaxPlayerNumKey);
    if (max_players != nullptr) {
        table.max_player_num = scene_property_int32(max_players);
        table.max_player_num_authored = true;
    }

    // 004f21b4..004f21e3: the eight blocks run only when `MultiPlay` is a
    // group. The loop counter, not the group, is what reaches 004e82f0 in ECX.
    const ScenePropertyBlock* multiplay
        = find_scene_property_block(scene_root_props, kSceneMultiPlayKey);
    if (multiplay == nullptr) {
        return table;
    }
    table.multiplay_block_present = true;
    for (std::size_t index = 0; index < kSceneRecordSideBlockCount; ++index) {
        table.blocks[index] = read_scene_slot_block_004e82f0(*multiplay, index);
    }
    return table;
}

std::size_t scene_slot_records_written(std::int32_t max_player_num) noexcept
{
    if (max_player_num <= 0) {
        return 0;
    }
    return static_cast<std::size_t>(max_player_num);
}

bool scene_slot_copy_overruns(std::int32_t max_player_num) noexcept
{
    return scene_slot_records_written(max_player_num) > kSceneSlotRecordCount;
}

} // namespace bsp
