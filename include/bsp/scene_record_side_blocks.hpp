#ifndef BSP_SCENE_RECORD_SIDE_BLOCKS_HPP
#define BSP_SCENE_RECORD_SIDE_BLOCKS_HPP

// The eight 0x120-byte side blocks of the 0x109c mission scene record, and the
// rule that fills them.
//
// docs/MISSION_LOAD_PATH.md left "who fills the side blocks" open: the .scn
// header pass is the only thing that touches the record between operator new
// and 004c6890, but no write to the block range was found in the reader's own
// reconstruction. The writer is 004e82f0, reached from the reader's `header`
// handler 00469bf0 through the property applier 004f1d70, which runs it eight
// times when the header's `properties` bag carries a `MultiPlay` sub-block.
// The blocks are the multiplayer player-slot definitions authored in the scene
// file; the count at record+988h is the `MaxPlayerNum` property (default 8).
//
// Addresses: 004da2a0 (record constructor), 004c9800/004c9820 (side block
// constructor and destructor), 004c6750 (unit element constructor), 004f1d70
// (property applier), 004e82f0 (one side block), 004c6890 (the consumer).
// See docs/SCENE_RECORD_SIDE_BLOCKS.md. Names are hypotheses, not recovered
// symbols.

#include "bsp/mission_load_path.hpp"
#include "bsp/scene_file.hpp"

#include <cstddef>
#include <cstdint>
#include <string>

namespace bsp {

// ---------------------------------------------------------------------------
// Record layout, from the constructor 004da2a0
// ---------------------------------------------------------------------------

// 004da2d7 stores the vtable 00ce7950 at record+0h, so the side-block array
// starts at record+4h, not at record+0h. 004da2ca..004da2dd then runs the MSVC
// eh vector constructor iterator over [record+4h, record+904h) with element
// size 0x120, count 8, constructor 004c9800 and destructor 004c9820.
inline constexpr std::size_t kSceneRecordVTableOffset = 0x0;
inline constexpr std::size_t kSceneRecordSideBlockBase = 0x4;
inline constexpr std::uint32_t kSceneRecordVTableAddress = 0x00ce7950;

// Scalar fields of the record that the same property applier writes. The two
// string pairs use the native {length, pointer} layout of bsp/native_string.
inline constexpr std::size_t kSceneRecordGroundCollisionOffset = 0x904;  // byte
inline constexpr std::size_t kSceneRecordDummyAiOffset = 0x908;          // dword, default 3
inline constexpr std::size_t kSceneRecordFileSystemLuaOffset = 0x918;    // string pair
inline constexpr std::size_t kSceneRecordTitleOffset = 0x920;            // string pair
inline constexpr std::size_t kSceneRecordCompetitiveModePartyOffset = 0x98c;

// ---------------------------------------------------------------------------
// One side block, from its constructor 004c9800 and its writer 004e82f0
// ---------------------------------------------------------------------------

// 004c9800 runs the element constructor 004c6750 over [block+8h, block+120h)
// with element size 0x38 and count 5. block+0h and block+4h are two plain
// dwords the constructor does not initialise.
inline constexpr std::size_t kSceneSideBlockPartyOffset = 0x0;  // -> slot record +28h
inline constexpr std::size_t kSceneSideBlockRaceOffset = 0x4;   // -> slot record +24h
inline constexpr std::size_t kSceneSideBlockUnitBase = 0x8;
inline constexpr std::size_t kSceneSideBlockUnitStride = 0x38;
inline constexpr std::size_t kSceneSideBlockUnitCount = 5;

// Offsets inside one 0x38 unit element (004e83d4..004e84cc).
inline constexpr std::size_t kSceneSlotUnitNameOffset = 0x0;       // string pair
inline constexpr std::size_t kSceneSlotUnitIconOffset = 0x8;       // dword, preset to -1
inline constexpr std::size_t kSceneSlotUnitClassIdOffset = 0xc;    // dword
inline constexpr std::size_t kSceneSlotUnitClassNameOffset = 0x10; // string pair
inline constexpr std::size_t kSceneSlotUnitPoolBase = 0x18;
inline constexpr std::size_t kSceneSlotUnitPoolStride = 0x8;
inline constexpr std::size_t kSceneSlotUnitPoolCount = 4;
inline constexpr std::size_t kSceneSlotPoolIconOffset = 0x0;
inline constexpr std::size_t kSceneSlotPoolCountOffset = 0x4;

// Property keys, with the literal each one is pushed from.
inline constexpr const char* kSceneMultiPlayKey = "MultiPlay";          // 00cea438
inline constexpr const char* kSceneMaxPlayerNumKey = "MaxPlayerNum";
inline constexpr const char* kSceneCompetitiveModePartyKey = "CompetitiveModeParty";
inline constexpr const char* kSceneSlotPartyKey = "Party";              // 00ce5804
inline constexpr const char* kSceneSlotRaceKey = "Race";                // 00ce8ee0
inline constexpr const char* kSceneSlotUnitKeyPrefix = "Unit";          // 00ce8ed8 "UnitN"
inline constexpr const char* kSceneSlotPoolKeyPrefix = "Pool";          // 00ce8ec0 "PoolN"
inline constexpr const char* kSceneSlotNameKey = "Name";                // 00ce8ed0
inline constexpr const char* kSceneSlotIconKey = "Icon";                // 00ce8ec8
inline constexpr const char* kSceneSlotNumKey = "Num";                  // 00ce8b94
inline constexpr const char* kSceneSlotClassIdKey = "ClassId";          // 00ce6920
inline constexpr const char* kSceneSlotClassNameKey = "ClassName";      // 00ce6930

// The default 004f1d70 writes to record+988h when `MaxPlayerNum` is absent
// (004f2145), and the default 004e8384/004e84d1 write for an absent unit or
// pool entry.
inline constexpr std::int32_t kSceneDefaultMaxPlayerNum = 8;
inline constexpr std::int32_t kSceneSlotAbsentIcon = -1;
inline constexpr std::int32_t kSceneDefaultDummyAi = 3;

// ---------------------------------------------------------------------------
// The authored view
// ---------------------------------------------------------------------------

// An `E` property, authored as `<key> = E <type> : <symbol> ;`. The record
// holds the resolved ordinal in the property node's +0Ch dword; the ordinal
// table is not part of the scene file, so only the authored pair is recovered
// here. This is the boundary between the file and the record for Party/Race.
struct SceneSlotEnumValue {
    bool present{false};
    std::string type;    // "Party", "Races"
    std::string symbol;  // "Japanese", "USA"
};

// One `PoolN` entry, 8 bytes at unit+18h + n*8.
struct SceneSlotPoolEntry {
    bool present{false};
    std::int32_t icon{kSceneSlotAbsentIcon};  // `Icon`, -1 when the entry is absent
    std::int32_t count{0};                    // `Num`, left untouched when absent
    bool count_written{false};                // false means the dword keeps its prior value
};

// One `UnitN` entry, 0x38 bytes at block+8h + n*0x38.
struct SceneSlotUnit {
    bool present{false};
    std::string name;       // `Name`
    std::int32_t icon{kSceneSlotAbsentIcon};  // `Icon`
    std::int32_t class_id{0};                 // `ClassId`
    bool class_id_written{false};
    std::string class_name;                   // `ClassName`
    SceneSlotPoolEntry pools[kSceneSlotUnitPoolCount]{};
};

// One 0x120 side block, i.e. one `MultiPlay.PlayerN` sub-block.
struct SceneSlotBlock {
    bool present{false};  // the `PlayerN` sub-block exists and is a group
    SceneSlotEnumValue party;
    SceneSlotEnumValue race;
    SceneSlotUnit units[kSceneSideBlockUnitCount]{};
};

// What 004f1d70 leaves in the record's slot region.
struct SceneRecordSlotTable {
    std::int32_t max_player_num{kSceneDefaultMaxPlayerNum};  // +988h
    bool max_player_num_authored{false};
    std::int32_t competitive_mode_party{0};  // +98Ch
    bool multiplay_block_present{false};     // the `MultiPlay` group was found
    SceneSlotBlock blocks[kSceneRecordSideBlockCount]{};
};

// ---------------------------------------------------------------------------
// Pure rules
// ---------------------------------------------------------------------------

// The `PlayerN` key for block `index`, from the eight-pointer table at
// 00e082ec that 004e82f6 indexes with the block number. One-based.
const char* scene_slot_block_key(std::size_t index) noexcept;

// "Unit0".."Unit4" and "Pool0".."Pool3": 004e8376 and 004e8482 overwrite the
// last character of the template with '0' + index, so the index is a single
// digit and the key length never changes.
std::string scene_slot_unit_key(std::size_t index);
std::string scene_slot_pool_key(std::size_t index);

// Byte offsets from the record base and from a block base.
std::size_t scene_record_side_block_offset(std::size_t index) noexcept;
std::size_t scene_side_block_unit_offset(std::size_t unit) noexcept;
std::size_t scene_side_block_pool_offset(std::size_t unit, std::size_t pool) noexcept;

// Typed reads of one property, matching what the native takes out of the node:
// +0Ch as a dword for `I`, the string body for `S`, and the authored pair for
// `E`. A null property reads as the absent value.
std::int32_t scene_property_int32(const SceneProperty* prop) noexcept;
std::string scene_property_text(const SceneProperty* prop);
SceneSlotEnumValue scene_property_enum_value(const SceneProperty* prop);

// 004e82f0, one side block. `multiplay` is the `MultiPlay` sub-bag and `index`
// is the zero-based block number; the key it looks up is `PlayerN` with N =
// index + 1. Returns a default-constructed block when the sub-block is absent,
// which is what the native leaves behind: it returns before touching the
// record at 004e8307/004e8311.
SceneSlotBlock read_scene_slot_block_004e82f0(const ScenePropertyBlock& multiplay,
    std::size_t index);

// 004f1d70's slot-related writes: `MaxPlayerNum` into +988h,
// `CompetitiveModeParty` into +98Ch, and the eight blocks when the `MultiPlay`
// sub-block is a group. `scene_root_props` is the header's `properties` bag,
// the one the reader publishes as "SceneRootProps".
SceneRecordSlotTable read_scene_record_slot_table_004f1d70(
    const ScenePropertyBlock& scene_root_props);

// 004c6890's copy loop: it runs record+988h times, not min(count, 8). The
// eight slot records at game+1008h hold eight entries, so a `MaxPlayerNum`
// above 8 walks off the end of the array; only when the loop stops at or below
// 8 does the tail clear at 004c6ac1 run.
std::size_t scene_slot_records_written(std::int32_t max_player_num) noexcept;
bool scene_slot_copy_overruns(std::int32_t max_player_num) noexcept;

} // namespace bsp

#endif
