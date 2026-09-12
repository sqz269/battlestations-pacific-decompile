#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "bsp/scene_file.hpp"

// The three scene-file tail routines the executable still lists as unimplemented
// SceneContents::* hosts, plus the property/enum library loader behind them:
//
//   009514B0  the `traffic` block          docs/SCENE_TRAFFIC_BLOCK.md
//   0095CA10  the pass-2 stock-queue drain docs/SCENE_TRAFFIC_BLOCK.md
//   0095C640  register a class for preload docs/SCENE_TRAFFIC_BLOCK.md
//   00469E40  the `SceneBrowserGroups` block  docs/SCENE_BROWSER_GROUPS.md
//   008F67B0  CPropTreeLibrary::Load       docs/SCENE_PROPERTY_LIBRARY.md
//   0046BF70  the multiplayer stock walk   docs/SCENE_BROWSER_GROUPS.md (contract)
//
// Each routine is a pure sequence over an injected host with one virtual method
// per native call site, so the executable can replace the host records without
// the grammars moving. The grammars themselves are pure readers over the
// ScenePropertyBlock / SceneProperty types of bsp/scene_file.hpp.
//
// Nothing here is a binary-compatible replacement: the record is modelled as a
// C++ struct with offset constants beside it, not as the 4Ch native layout.
namespace bsp {

// ---------------------------------------------------------------------------
// The traffic record (4Ch bytes), 00951220 defaults + 0049CF80 application.
// Offsets are from the two producers; see docs/SCENE_TRAFFIC_BLOCK.md.
// ---------------------------------------------------------------------------
struct TrafficRecordOffsets {
    static constexpr std::uint32_t kSize = 0x4C;
    static constexpr std::uint32_t kPathEntity = 0x00;   // 00951286
    static constexpr std::uint32_t kStartPoint = 0x04;   // 00951288 / 0049CFAE
    static constexpr std::uint32_t kEndPoint = 0x08;     // 0095128F
    static constexpr std::uint32_t kCyclic = 0x0C;       // 009512B8, one byte
    static constexpr std::uint32_t kRowCount = 0x10;     // 0095129D
    static constexpr std::uint32_t kColumns = 0x14;      // 00951296
    static constexpr std::uint32_t kRowGap = 0x18;       // 00951271
    static constexpr std::uint32_t kColumnGap = 0x1C;    // 0095127E
    static constexpr std::uint32_t kHitPoints = 0x20;    // 0049CFxx, HP
    static constexpr std::uint32_t kSpeed = 0x24;        // 009512AE
    static constexpr std::uint32_t kRandomFactor = 0x28; // 009512B3
    static constexpr std::uint32_t kRowDeviation = 0x2C; // 009512A4
    static constexpr std::uint32_t kColumnDeviation = 0x30; // 009512A9
    static constexpr std::uint32_t kTemplateWeights = 0x34; // 0049D244, 18h bytes
};

// The constructor defaults 00951220 writes. 0049CF80 reloads the same three
// float constants as its absent-key values, but writes 0 where the constructor
// wrote -1 for the two point indices; both are recorded so an item parsed
// without a `properties` section keeps the constructor's values.
struct TrafficRecordDefaults {
    static constexpr std::int32_t kStartPoint = -1;       // 00951288
    static constexpr std::int32_t kEndPoint = -1;         // 0095128F
    static constexpr std::int32_t kColumns = 1;           // 00951296
    static constexpr float kRowGap = 40.0f;               // DAT_00CE685C
    static constexpr float kColumnGap = 10.0f;            // DAT_00CE38B8
    static constexpr float kSpeed = 5.0f;                 // DAT_00CE3850
    // 0049CF80's values when the key is absent from the bag.
    static constexpr std::int32_t kAppliedStartPoint = 0;
    static constexpr std::int32_t kAppliedEndPoint = 0;
};

// One `templates` child: a class or soldier-type name and its spawn weight.
// 0049D364 stores the weight through the map at record+34h keyed by the id.
struct TrafficTemplateWeight {
    std::string name;
    float weight{0.0f};
    // Which enum table resolved the name. 0049D25A probes LandVehicleClasses
    // first; 0049D2AC falls back to SoldierTypes.
    bool land_vehicle_class{false};
    std::int32_t class_id{-1};
};

// The record as 009512F0 leaves it. `path_entity_name` is the authored `path =`
// token; the native stores the resolved entity pointer, not the name.
struct TrafficRecord {
    std::string path_entity_name;
    bool path_entity_resolved{false};
    std::int32_t start_point{TrafficRecordDefaults::kStartPoint};
    std::int32_t end_point{TrafficRecordDefaults::kEndPoint};
    bool cyclic{false};
    std::int32_t row_count{0};
    std::int32_t columns{TrafficRecordDefaults::kColumns};
    float row_gap{TrafficRecordDefaults::kRowGap};
    float column_gap{TrafficRecordDefaults::kColumnGap};
    float hit_points{0.0f};
    float speed{TrafficRecordDefaults::kSpeed};
    float random_factor{0.0f};
    float row_deviation{0.0f};
    float column_deviation{0.0f};
    std::vector<TrafficTemplateWeight> templates;
};

// The thirteen keys 0049CF80 reads, in the order it reads them. `anims` is
// authored by every installed item and read by none of them.
inline constexpr const char* kTrafficKeyStartPoint = "startPt";
inline constexpr const char* kTrafficKeyEndPoint = "endPt";
inline constexpr const char* kTrafficKeyCyclic = "cyclic";
inline constexpr const char* kTrafficKeyRowCount = "rowCount";
inline constexpr const char* kTrafficKeyColumns = "columns";
inline constexpr const char* kTrafficKeyRowGap = "rowGap";
inline constexpr const char* kTrafficKeyColumnGap = "columnGap";
inline constexpr const char* kTrafficKeyHitPoints = "HP";
inline constexpr const char* kTrafficKeySpeed = "speed";
inline constexpr const char* kTrafficKeyRandomFactor = "randomFactor";
inline constexpr const char* kTrafficKeyRowDeviation = "rowDev";
inline constexpr const char* kTrafficKeyColumnDeviation = "columnDev";
inline constexpr const char* kTrafficKeyTemplates = "templates";

// Literals of the two blocks, at the addresses the readers push them from.
inline constexpr const char* kTrafficBlockKeyword = "traffic";      // 00CE5890
inline constexpr const char* kTrafficItemKeyword = "item";          // 00D19B80
inline constexpr const char* kTrafficPathKeyword = "path";          // 00D099C0
inline constexpr const char* kBrowserGroupsBlockKeyword = "SceneBrowserGroups"; // 00CE56A4
inline constexpr const char* kBrowserGroupKeyword = "BrowserGroup"; // 00CE5654
inline constexpr const char* kBrowserGroupNameKey = "GroupName";    // 00CE55C0
inline constexpr const char* kBrowserGroupIdKey = "GroupID";        // 00CE564C
inline constexpr const char* kBrowserGroupParentKey = "Parent";     // 00CE5644

// The two enum tables 0049CF80 resolves template names through.
inline constexpr const char* kLandVehicleClassesTable = "LandVehicleClasses"; // 00CE6710
inline constexpr const char* kSoldierTypesTable = "SoldierTypes";             // 00CE6700

// The "no class" sentinel 0095C640 rejects at 0095C66E, the same value 0046BE90
// rejects outright.
inline constexpr std::int32_t kVehicleClassNone = 0x2AC;

// The `Type` strings that make 0095C640 also register a catapult load-out.
inline constexpr const char* kCatapultCarrierTypes[] = {
    "Destroyer", "Cruiser", "LandingShip", "Cargo",
    "BattleShip", "Submarine", "TorpedoBoat", "MotherShip",
};
inline constexpr std::size_t kCatapultCarrierTypeCount =
    sizeof(kCatapultCarrierTypes) / sizeof(kCatapultCarrierTypes[0]);

// 0095C640's Lua reads: the table, the field, and the dotted-path template.
inline constexpr const char* kVehicleClassLuaTable = "VehicleClass";   // 00CE5880
inline constexpr const char* kVehicleClassTypeField = "Type";          // 00CE4780
inline constexpr const char* kVehicleClassLandingShipField = "LandingShip";
inline constexpr const char* kCatapultLaunchedSuffix = ".Catapult.LaunchedClass";
inline constexpr const char* kCatapultLaunchedPrefix = "VehicleClass.";
inline constexpr std::int32_t kCatapultLaunchedDefault = -1; // 0095C6xx push -1

// The three class names 0046DF00's stop epilogue refuses to register.
inline constexpr const char* kPreloadRejectedTypes[] = {
    "CommandBuilding", // 00CE5870
    "LandFort",        // 00CE5864
    "LandVehicle",     // 00CE5858
};
inline constexpr std::size_t kPreloadRejectedTypeCount =
    sizeof(kPreloadRejectedTypes) / sizeof(kPreloadRejectedTypes[0]);

// ---------------------------------------------------------------------------
// SceneBrowserGroups. The entry is modelled because the grammar is real; the
// native stores none of it (004694F0 writes GroupID into the caller's dead
// argument slot and frees both strings before returning).
// ---------------------------------------------------------------------------
struct SceneBrowserGroupEntry {
    std::string group_name;
    std::int32_t group_id{0};
    std::string parent;
};

// ---------------------------------------------------------------------------
// The property/enum library, 008F67B0 and its two callers.
// ---------------------------------------------------------------------------
inline constexpr const char* kPropLibraryFolder = "universe\\Library\\"; // 00CE78E0
inline constexpr const char* kPropLibraryEnumExtension = ".enums";       // 00D1655C
inline constexpr const char* kPropLibraryPropsExtension = ".props";      // 00D16554
inline constexpr const char* kPropLibraryEnumKeyword = "enum";           // 00D1652C
inline constexpr const char* kPropLibraryPropertiesKeyword = "properties"; // 00CE568C
// One character shorter than kSceneDelimiters: the library tokenizer does not
// split on `,` (00D16534 against 00CE4F40).
inline constexpr const char* kPropLibraryDelimiters = ";{}=:()";
inline constexpr const char* kPropLibraryScopeMarker = "CPropTreeLibrary::Load "; // 00D1653C
inline constexpr std::uint32_t kPropLibraryEnumTableSize = 0x19C;  // 008F69A7
inline constexpr std::uint32_t kPropLibraryEnumTableNameOffset = 0x11C;
inline constexpr std::uint32_t kPropLibraryGroupMapOffset = 0x04;  // 008F6B34

// One `properties <Name>[(<Parent>)]` declaration.
struct PropLibraryGroup {
    std::string name;
    std::string parent;       // empty when no parenthesised parent was authored
    ScenePropertyBlock body;
    bool created{false};      // false when an earlier file already declared it
};

// One `enum <Name> { ... }` declaration. The body's symbol parse is 008F31A0,
// a contract; only the dispatch is reconstructed here.
struct PropLibraryEnumTable {
    std::string name;
    bool created{false};
};

struct PropLibraryDocument {
    std::vector<PropLibraryGroup> groups;
    std::vector<PropLibraryEnumTable> enums;
};

// ---------------------------------------------------------------------------
// Hosts. One virtual method per native call site.
// ---------------------------------------------------------------------------

// 009514B0, 009512F0, 00951220, 00951160 and 004A5620.
struct SceneTrafficHost {
    virtual ~SceneTrafficHost() = default;

    // 00951160 on the vector at 00F8A084: destroy every record (0049F910) and
    // empty the vector. Runs at 009514B5 and again at 0046E9EB for both passes.
    virtual void clear_traffic_records() = 0;

    // 0095137E/00951385: BSP_EntityRegistry_FindEntityByName on
    // [[00E188A8]+19CCh], then the 007AC9D0 cast. Returns false when the name
    // was empty or no entity matched, which is the 0095138C XOR EAX,EAX arm.
    virtual bool resolve_path_entity(const std::string& name) = 0;

    // 00951220: operator new(4Ch), the constructor defaults, and the
    // 00950FC0 push_back. `path_entity_resolved` is what step 10 pushed.
    // The item name is passed because the native passes it, and ignored
    // because the native ignores it.
    virtual void add_traffic_record(const std::string& item_name,
        const TrafficRecord& record) = 0;

    // 0049D25A / 0049D2AC: 0048E960 on [00E1867C] for the named table, then
    // 0048E8D0 membership and 0048E840 lookup. Returns false when the name is
    // in neither table.
    virtual bool resolve_template_class(const std::string& name,
        bool& land_vehicle_class, std::int32_t& class_id) = 0;

    // 0049D291: 00964790(ECX = class_id, DL = 1), the vehicle-class
    // materialisation, taken only on the LandVehicleClasses arm.
    virtual void ensure_vehicle_class(std::int32_t class_id) = 0;

    // 0049D311: the SoldierTypes arm's 004B1400. contract: unread.
    virtual void ensure_soldier_type(std::int32_t soldier_type_id) = 0;

    // 0095154D: 004A5620 on [[00E188A8]+21D0h]. Per record, 00499320 on
    // manager+10h and, on a miss, operator new(170h) + 004A50D0.
    virtual void commit_traffic_records() = 0;
};

// 0095C640, 0095C550 and 0095C4D0.
struct VehicleClassPreloadHost {
    virtual ~VehicleClassPreloadHost() = default;

    // 0095C665/0095C66A: registry+10h+id*4, the forward index map. Identity for
    // anything not remapped (docs/VEHICLE_CLASS_DESCRIPTORS.md).
    virtual std::int32_t map_class_index(std::int32_t class_id) = 0;

    // The Lua reads of 0095C640: VehicleClass[index].Type as a string.
    virtual bool read_class_type(std::int32_t class_index, std::string& type) = 0;

    // VehicleClass[index].LandingShip as an integer, default 0.
    virtual std::int32_t read_landing_ship_class(std::int32_t class_index) = 0;

    // "VehicleClass." + index + ".Catapult.LaunchedClass", default -1.
    virtual std::int32_t read_catapult_launched_class(std::int32_t class_index) = 0;

    // 0095C550: the deduplicated append to the census vector at 00F8A09C.
    virtual void append_census(std::int32_t class_index) = 0;

    // 0095CA10: the intrusive list at 00F8A0B0, drained one node at a time.
    // Returns false when the list is empty.
    virtual bool pop_stock_queue(std::int32_t& class_index) = 0;

    // 0095C4D0: the deduplicated append to that same list, from 0046BF70.
    virtual void push_stock_queue(std::int32_t class_index) = 0;
};

// 00469E40 and 004694F0. The native discards every entry; the host exists so a
// caller can observe what was parsed without changing that.
struct SceneBrowserGroupsHost {
    virtual ~SceneBrowserGroupsHost() = default;
    // No native call site: 004694F0 stores nothing. Called once per entry so a
    // reconstruction can assert the grammar without inventing storage.
    virtual void observe_browser_group(const SceneBrowserGroupEntry& entry) = 0;
};

// 008F67B0, 008F6FC0 and 008F7100.
struct PropLibraryHost {
    virtual ~PropLibraryHost() = default;

    // 00886280 at 008F701B: enumerate the folder for one extension.
    virtual std::vector<std::string> enumerate_library_files(
        const std::string& folder, const std::string& extension) = 0;

    // 008F67B0's prologue: read the named file through the VFS.
    virtual bool read_library_file(const std::string& path, std::string& out) = 0;

    // 008F6A69 / 008F697C: does the registry already hold this name?
    virtual bool has_group(const std::string& name) = 0;
    virtual bool has_enum_table(const std::string& name) = 0;

    // 008F6B42: 008F29A0 on library+4h, the group-map insert. Only on a miss.
    virtual void register_group(const PropLibraryGroup& group) = 0;
    // 008F6A18: 008F2B90, the enum-table insert. Only on a miss.
    virtual void register_enum_table(const PropLibraryEnumTable& table) = 0;

    // 008F69FF: 008F31A0 parses the enum body. contract: unread, so the
    // reconstruction hands the host the brace-balanced body text.
    virtual void parse_enum_body(const std::string& name, const std::string& body) = 0;
};

// ---------------------------------------------------------------------------
// Results.
// ---------------------------------------------------------------------------
struct SceneTrafficReadResult {
    bool ok{false};
    std::string error;
    std::vector<TrafficRecord> records;
    std::size_t skipped_tokens{0};  // what the brace counter swallowed
};

struct SceneBrowserGroupsReadResult {
    bool ok{false};
    std::string error;
    std::vector<SceneBrowserGroupEntry> entries;
};

struct PropLibraryLoadResult {
    bool ok{false};
    std::string error;
    PropLibraryDocument document;
};

// ---------------------------------------------------------------------------
// The routines.
// ---------------------------------------------------------------------------

// 009514B0. `lexer` must be positioned on the `traffic` keyword. Clears the
// record vector through the host, reads the block, then commits.
SceneTrafficReadResult read_traffic_block(SceneLexer& lexer, SceneTrafficHost& host);

// 009512F0, exposed on its own because 009514B0 is a dispatcher around it.
bool read_traffic_item(SceneLexer& lexer, SceneTrafficHost& host,
    TrafficRecord& out, std::string& item_name, std::string& error);

// 0049CF80. Pure: applies a parsed `properties` block to a default-constructed
// record. The `templates` arm calls the host to resolve each name.
void apply_traffic_properties(const ScenePropertyBlock& block,
    SceneTrafficHost& host, TrafficRecord& record);

// 0095C640. Returns the number of census appends it made (0, 1, 2 or 3).
int register_vehicle_class_preload(std::int32_t class_id,
    VehicleClassPreloadHost& host);

// 0095CA10. Drains the stock queue into register_vehicle_class_preload and
// returns how many entries it drained.
int drain_stock_class_queue(VehicleClassPreloadHost& host);

// 00469E40. `lexer` must be positioned on the `SceneBrowserGroups` keyword.
SceneBrowserGroupsReadResult read_browser_groups_block(SceneLexer& lexer,
    SceneBrowserGroupsHost& host);

// 008F7100 then 008F6FC0 twice: .enums, then .props.
PropLibraryLoadResult load_prop_library(const std::string& folder,
    PropLibraryHost& host);

// 008F67B0, one file.
bool load_prop_library_file(const std::string& path, PropLibraryHost& host,
    PropLibraryDocument& document, std::string& error);

} // namespace bsp
