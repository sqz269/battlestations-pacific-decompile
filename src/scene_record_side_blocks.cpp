// The eight side blocks of the mission scene record, and the rule that fills
// them. See include/bsp/scene_record_side_blocks.hpp and
// docs/SCENE_RECORD_SIDE_BLOCKS.md for the addresses and the evidence.

#include "bsp/scene_record_side_blocks.hpp"

#include "bsp/scene_entity_factory.hpp"

#include <cstdlib>
#include <cstring>

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

std::int32_t max_player_property_int32(const SceneProperty* prop) noexcept
{
    // 004F214C..004F2154 copies the stored MaxPlayerNum word. Preserve the
    // existing I-only raw fallback and caller's missing-key default.
    if (prop != nullptr && integer_letter(prop->type_letter) && prop->has_integer) {
        return prop->integer_value;
    }
    return scene_property_int32(prop);
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

    // 004f2157: an absent `MaxPlayerNum` stores the literal 8.
    const SceneProperty* max_players = scene_root_props.find(kSceneMaxPlayerNumKey);
    if (max_players != nullptr) {
        table.max_player_num = max_player_property_int32(max_players);
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

// ---------------------------------------------------------------------------
// 004F1D70 BSP_SceneRecord_ApplyHeaderProperties (567 insns), whole body. cc13-P1.
// ABI: __fastcall(ECX = property bag, EDX = record; char sound_enabled), RET 4.
// EBX keeps the bag until 004F21C4 reuses it for the constant 6; from 004F21E5 the
// bag is reloaded into ESI from the frame. EBP is the record throughout.
// EH: handler 00C682AB, one state (0 at 004F2252, -1 at 004F2266) covering the
// stack environment block between 004CB420 and 004C6520; reproduced with a
// catch-and-rethrow that runs the destructor.
// Uncertainty: the record and the nodes are read in place in their Win32
// layout; 004F236D/004F2380 copy two floats through FLD/FSTP, which would quiet
// a signalling NaN where this plain float copy may not; the IslandCapture copies'
// consumers are not established.
// ---------------------------------------------------------------------------
namespace {

std::uint32_t native_string_length(const std::uint8_t* string) noexcept
{
    return *reinterpret_cast<const std::uint32_t*>(string);
}

char* native_string_data(const std::uint8_t* string) noexcept
{
    return *reinterpret_cast<char* const*>(string + 4);
}

std::uint32_t node_dword(const void* node, std::size_t offset) noexcept
{
    return *reinterpret_cast<const std::uint32_t*>(static_cast<const std::uint8_t*>(node) + offset);
}

void* node_group(const void* node) noexcept
{
    return *reinterpret_cast<void* const*>(static_cast<const std::uint8_t*>(node)
        + kScenePropertyValueOffset);
}

// The string idiom repeated at 004F1D91, 004F1DED, 004F1E40, 004F1EB8, 004F1F0D,
// 004F1F60, 004F1FB3, 004F200D and 004F2161: find; when found, take the string
// value, strlen it (a null text is length 0), 0041DD40(length, 0), and memcpy
// into the data pointer when it is non-null. An absent key leaves the field.
bool assign_found_string(SceneHeaderApplyHost& host, void* bag, const char* key,
    std::uint8_t* field)
{
    void* node = host.property_bag_find_008f2260(bag, key);
    if (node == nullptr) {
        return false;
    }
    const char* text = host.property_string_value_008f0df0(node);
    const std::uint32_t length = text != nullptr
        ? static_cast<std::uint32_t>(std::strlen(text)) : 0u;
    host.native_string_resize_0041dd40(field, length, false);
    char* data = native_string_data(field);
    if (data != nullptr && length != 0) {
        std::memcpy(data, text, length);
    }
    return true;
}

// 004F2064..004F20EE: one NativeString copy. The image skips it when the two
// fields are the same object, resizes with preserve 1 to the source length and
// copies the destination's (new) length when the source length is non-zero.
void copy_native_string(SceneHeaderApplyHost& host, std::uint8_t* destination,
    const std::uint8_t* source)
{
    if (destination == source) {
        return;
    }
    host.native_string_resize_0041dd40(destination, native_string_length(source), true);
    if (native_string_length(source) != 0) {
        std::memcpy(native_string_data(destination), native_string_data(source),
            native_string_length(destination));
    }
}

// The group-reader idiom: find; call the reader only when the node exists and its
// type tag is 6, passing the sub-bag at +0Ch.
void* found_group(SceneHeaderApplyHost& host, void* bag, const char* key)
{
    void* node = host.property_bag_find_008f2260(bag, key);
    if (node == nullptr || node_dword(node, kScenePropertyTypeOffset) != kScenePropertyTypeGroup) {
        return nullptr;
    }
    return node_group(node);
}

} // namespace

void scene_record_apply_header_properties_004f1d70(void* bag, std::uint8_t* record,
    bool sound_enabled, SceneHeaderApplyHost& host)
{
    assign_found_string(host, bag, "FileSystemLua", record + kSceneRecordFileSystemLuaOffset);
    assign_found_string(host, bag, "Title", record + kSceneRecordTitleOffset);

    // 004F1E40..004F1EB3: GameStageScript wins; StageScript is read only when it
    // is absent, and then through 0041E350 rather than the resize idiom.
    if (!assign_found_string(host, bag, "GameStageScript",
            record + kSceneRecordGameStageScriptOffset)) {
        void* node = host.property_bag_find_008f2260(bag, "StageScript");
        if (node != nullptr) {
            host.native_string_assign_0041e350(record + kSceneRecordGameStageScriptOffset,
                host.property_string_value_008f0df0(node));
        }
    }

    assign_found_string(host, bag, "StageScriptDuel", record + kSceneRecordStageScriptDuelOffset);
    assign_found_string(host, bag, "StageScriptEscort",
        record + kSceneRecordStageScriptEscortOffset);
    assign_found_string(host, bag, "StageScriptSiege", record + kSceneRecordStageScriptSiegeOffset);
    // 00CEA4A4: the authored key is misspelt "Competiton".
    assign_found_string(host, bag, "StageScriptCompetiton",
        record + kSceneRecordStageScriptCompetitiveOffset);
    // 004F200D..004F20EE: only a found IslandCapture script is copied on, +940h to
    // +938h, then +938h to +930h, then +930h to +928h.
    if (assign_found_string(host, bag, "StageScriptIslandCapture",
            record + kSceneRecordStageScriptIslandCaptureOffset)) {
        copy_native_string(host, record + kSceneRecordIslandCaptureCopy1Offset,
            record + kSceneRecordStageScriptIslandCaptureOffset);
        copy_native_string(host, record + kSceneRecordIslandCaptureCopy2Offset,
            record + kSceneRecordIslandCaptureCopy1Offset);
        copy_native_string(host, record + kSceneRecordIslandCaptureCopy3Offset,
            record + kSceneRecordIslandCaptureCopy2Offset);
    }

    // 004F20EE..004F2113: the +0Ch dword whatever the type tag, else 3.
    {
        void* node = host.property_bag_find_008f2260(bag, "DummyAIEnabled");
        *reinterpret_cast<std::uint32_t*>(record + kSceneRecordDummyAiOffset) = node != nullptr
            ? node_dword(node, kScenePropertyValueOffset)
            : static_cast<std::uint32_t>(kSceneDefaultDummyAi);
    }
    // 004F2113..004F213D: Contains first, then a second find, else 0.
    *reinterpret_cast<std::uint32_t*>(record + kSceneRecordCompetitiveModePartyOffset)
        = host.property_bag_contains_0048e9f0(bag, "CompetitiveModeParty")
        ? node_dword(host.property_bag_find_008f2260(bag, "CompetitiveModeParty"),
              kScenePropertyValueOffset)
        : 0u;
    {
        void* node = host.property_bag_find_008f2260(bag, "MaxPlayerNum");   // 004F2136
        *reinterpret_cast<std::uint32_t*>(record + kSceneRecordMaxPlayerNumOffset) = node != nullptr
            ? node_dword(node, kScenePropertyValueOffset)
            : static_cast<std::uint32_t>(kSceneDefaultMaxPlayerNum);
    }
    assign_found_string(host, bag, "LocKitPaths", record + kSceneRecordLocKitPathsOffset);

    // 004F21B4..004F21E3: eight slots, ECX = the index, EDX = the sub-bag.
    if (void* multiplay = found_group(host, bag, kSceneMultiPlayKey)) {
        for (std::int32_t index = 0; index < static_cast<std::int32_t>(kSceneRecordSideBlockCount);
             ++index) {
            host.scene_read_player_slot_004e82f0(index, multiplay, record);
        }
    }
    if (void* ocean = found_group(host, bag, "g_Ocean")) {                  // 004F21E9
        host.scene_read_water_004eb9b0(ocean, record);
    }
    // 004F220A..004F2218: no null or type test.
    host.idle_camera_load_shot_pairs_00551440(host.property_bag_find_008f2260(bag, "IdleCameraPos"));

    if (void* weather = found_group(host, bag, "g_Weather")) {              // 004F221D
        host.scene_read_weather_fog_cloud_lights_004eca30(weather, record);
    } else {
        // 004F223E..004F2271: a default environment block on the frame, assigned
        // into +990h and destroyed.
        alignas(4) std::uint8_t block[kSceneEnvironmentBlockSize];
        host.scene_environment_block_construct_004cb420(block);
        try {
            host.scene_header_assign_004e8bf0(record + kSceneRecordEnvironmentOffset, block);
        } catch (...) {
            host.scene_environment_block_destruct_004c6520(block);
            throw;
        }
        host.scene_environment_block_destruct_004c6520(block);
    }

    {
        // 004F2276..004F2297: tag 2, a string.
        void* node = host.property_bag_find_008f2260(bag, "AffterEffects");
        if (node != nullptr
            && node_dword(node, kScenePropertyTypeOffset) == kScenePropertyTypeString) {
            host.scene_header_append_unquoted_names_004f1b60(
                host.property_string_value_008f0df0(node), record);
        }
    }
    {
        // 004F229C..004F22C0: the find runs before the sound flag is tested.
        void* sound = found_group(host, bag, "Sound");
        if (sound_enabled && sound != nullptr) {
            host.scene_read_music_ambience_004ed690(sound, record);
        }
    }
    if (void* map = found_group(host, bag, "Map")) {                        // 004F22C5
        host.map_read_extent_and_apply_004e6c00(map, record);
    } else {
        host.game_apply_map_bounds_004d5bd0(host.global_pointer(kSceneGameObjectGlobal));
    }
    if (void* environment = found_group(host, bag, "g_Environment")) {      // 004F22F1
        host.scene_read_sky_lights_004ee450(environment, record);
    }
    if (void* terrain = found_group(host, bag, "g_Terrain")) {              // 004F2310
        host.scene_read_static_shadow_textures_004ee9e0(terrain, record);
    }
    if (void* post = found_group(host, bag, "g_PostProcess")) {             // 004F232F
        host.scene_read_post_effects_004e8510(post, record);
    }
    if (void* cloud = found_group(host, bag, "Cloud")) {                    // 004F234E
        host.scene_load_cloud_areas_004e7220(cloud, record);
    }

    // 004F236D..004F2386, before the GroundCollision find.
    *reinterpret_cast<float*>(record + kSceneRecordFloat100cOffset)
        = *reinterpret_cast<const float*>(record + kSceneRecordFloat1004Offset);
    *reinterpret_cast<float*>(record + kSceneRecordFloat1010Offset)
        = *reinterpret_cast<const float*>(record + kSceneRecordFloat1008Offset);

    {
        // 004F238C..004F239E: tag 3 stores the +0Ch byte; anything else leaves +904h.
        void* node = host.property_bag_find_008f2260(bag, "GroundCollision");
        if (node != nullptr && node_dword(node, kScenePropertyTypeOffset) == kScenePropertyTypeBool) {
            record[kSceneRecordGroundCollisionOffset]
                = static_cast<const std::uint8_t*>(node)[kScenePropertyValueOffset];
        }
    }
    {
        // 004F23A4..004F23F0: game+2194h gets the byte, or 1 when absent or not tag 3.
        void* node = host.property_bag_find_008f2260(bag, "SpectatorEnabled");
        auto* game = static_cast<std::uint8_t*>(host.global_pointer(kSceneGameObjectGlobal));
        if (node != nullptr && node_dword(node, kScenePropertyTypeOffset) == kScenePropertyTypeBool) {
            game[kSceneGameSpectatorEnabledOffset]
                = static_cast<const std::uint8_t*>(node)[kScenePropertyValueOffset];
        } else {
            game[kSceneGameSpectatorEnabledOffset] = 1;
        }
    }
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
