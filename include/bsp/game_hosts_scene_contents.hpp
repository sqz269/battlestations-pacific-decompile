#pragma once
// bsp_game.exe milestone 2h: the scene contents pass, as process bindings.
//
// Addresses: 004d4df0 (BSP_Game_LoadSceneContents, the frame around the two
// scene-file passes), 0046df00 (BSP_SceneFile_Read, run twice), 0046cf40 (the
// entity block and the per-pass split at 0046d433), 0046c550 (the generation
// gate), 004f2800 (the 26-class table), 0046f160 (the scene-database
// constructor, which is what fills the always-generate set at +164h),
// 004f0520 and its eight siblings (the unit creators), 004e96d0 through the
// thunk 004e98e0 (the registration-pass body), 0095c640 and 0046bf70 (the two
// registration-pass calls at 0046d51a and 0046d531), 008f67b0
// (`CPropTreeLibrary::Load`, the property-group and enum library) and 008f54f0
// (the bag merge each group name performs at 0046cf40 step 6).
//
// Nothing in this file is a reconstruction of native code. Every type here is
// an integration binding that satisfies one of the host interfaces in
// bsp/mission_scene_contents.hpp, bsp/scene_file.hpp, bsp/scene_entity_factory.hpp
// and bsp/scene_unit_creators.hpp with either a concrete implementation over an
// already reconstructed routine or the explicit unimplemented policy in
// GameHostLog.
//
// The one exception is labelled as such in the source: the property-group and
// enum library reader. `CPropTreeLibrary::Load` (008f67b0) is not
// reconstructed, and without the schema it publishes the bag an entity hands
// the gate is missing every defaulted key, which changes what the gate decides.
// The executable therefore reads `universe/library` itself, through the
// recovered VFS enumeration and the recovered property-block parser, and says
// so. See docs/GAME_EXECUTABLE.md, milestone 2h.
//
// Evidence: docs/MISSION_SCENE_CONTENTS.md, docs/SCENE_FILE_READER.md,
// docs/SCENE_ENTITY_FACTORY.md, docs/SCENE_UNIT_CREATORS.md,
// docs/SCENE_ENTITY_CREATE.md, docs/VEHICLE_CLASS_DESCRIPTORS.md.

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "bsp/air_operations.hpp"
#include "bsp/scene_file.hpp"

namespace bsp::game {

class GameHostLog;
class GameMissionLuaHost;
class GameVfsHost;

// One entity the instantiate pass reached, in file order. `world` is the frame
// 0046cf40 composes through 00413920 before it calls the gate.
struct GameSceneEntityRecord {
    std::string name;
    std::string class_name;
    int class_id{-1};
    std::string type_symbol;   // the `E <table> : <symbol>` token of `Type`
    std::string type_table;
    int type_id{-1};           // the symbol resolved through the library's enum
    std::string party_symbol;
    int party{-1};
    // Packet cc9_scene_race_and_script_identity: the bag's `Race` (00CE8EE0), which
    // 00927050's kind-1 arm stores at entity+58h (0092708F..0092709C) with no
    // presence test. `properties Common` (global.enums) defaults it to Neutral 0,
    // so every entity whose group derives from Common has one; -1 when the bag
    // has none.
    int race{-1};
    // Packet cc9_ai_owner_player_slot: `OwnerPlayer` (00CF882C), which 0077F0E0's
    // activation hands vtable[144h] (0077F1F9) for unit+180h, or 9 when the bag
    // has none (0077F1F1 MOV EAX,9). Resolved through global.enums `Players`
    // ("Player 1".."Player 8" 0..7, "AI control" 8, "Any player" 9).
    std::string owner_player_symbol;
    int owner_player{9};
    // Packet cc9_scene_unit_skill: 00927A80 over the merged bag. `Skill` (00CF8838,
    // group defaults included: Ship, LandFort, LandConvoy, PlaneSquadronWNavpoint
    // declare SPNormal), else `Crew` (00D19264) through 006E6210, else 1. The
    // creators hand it to vtable[128h] (00822C20 at 008238C1..008238CB, 006D3CF0,
    // 00748383, 007D65AA, 00849D71). `bag_skill_source` names the arm taken.
    int bag_skill{1};
    std::string bag_skill_source{"fallback"};
    bool generated{false};     // 0046c550 returned AL != 0
    std::string gate_rule;     // which rule of the gate produced that answer
    // Packet cc9_scene_home_base_contract: a PlaneSquadronGen row's `HomeBase`
    // (00CF8820) as authored, "" when empty or absent; `home_base_carried` marks a
    // record that came from such a row. The bag does not survive a hold-back, so
    // the key travels on the record the spawn pool keeps (as WingCount travels
    // on the pool entry) to the squadron's creation.
    std::string home_base;
    bool home_base_carried{false};
    bool created{false};       // the class creator ran and handed back an instance
    std::string skipped_because;
    float world[16]{};
    // Authored scene identities, not native pointers. Zero means no authored
    // parent; IDs include unregistered entities and follow the reader's visit
    // order. The local frame and actual parent's composed frame are retained
    // separately; parent_world is meaningful only when parent_scene_id != 0.
    std::size_t scene_id{0};
    std::size_t parent_scene_id{0};
    std::string parent_name;
    float local[16]{};
    float parent_world[16]{};
    // Partial projection of 007B34F0's kind-1 property-bag branch, for a fresh
    // Path. Numeric Point%002i lookup order, stopping at the first absent or
    // non-block key; Pos triples remain LOCAL. No Path entity is constructed:
    // created and the creator tally retain their existing unresolved status.
    bool path_points_retained{false};
    std::string path_points_error;
    std::vector<std::array<float, 3>> path_points_local;
    // Milestone 2l: the two strings 004f0520 hands 00469610 at its last step,
    // the authored `Command = E CommandType : <name>` token and the
    // `CommandTarget = R "<name>"` value ("" when the entity authored none).
    // Milestone 2h dropped both at the queue host; the command path needs them.
    std::string command;
    std::string command_target;
    // Milestone 2q: the two keys 00822C20's property-bag arm looks up in the
    // holder 0046d5b0 stores at entity+0C0h. `StartSpeed` is authored on every
    // `Cruise` unit of usn_2_java.scn (docs/CRUISE_SPEED_SETTING.md) and
    // 00823590 is the find; the record's type word and its +0Ch value are
    // direct loads at 0082359C / 0082359E / 008235A5, which is why the type
    // and both readings travel together. `ShipYardLaunch` is 00823576's find
    // and only its +0Ch byte is read, at 0082357F.
    bool start_speed_present{false};
    int start_speed_type{1};        // ScenePropertyType, 0 = `I`, 1 = `F`
    float start_speed_float{0.0f};  // record +0Ch read as float32, 008235A5
    std::int32_t start_speed_int{0};  // record +0Ch read as int, 0082359E
    bool shipyard_launch{false};    // 0082357F, the found record's +0Ch byte
    // Packet cc9_units_capture_accessors: 006F2780's `CaptureRange` find. The image
    // copies the found record's +0Ch dword to unit+7A0h, or 500 when absent
    // (006F27E5). This installation authors it as `I 100` on its CommandBuildings.
    bool capture_range_present{false};
    std::int32_t capture_range_raw{500};
    // Routed from cc9-ships13: 006F2780's `LandingRange` find (key 00CFAE30,
    // 006F2847), read like CaptureRange: the found record's +0Ch dword, or 500
    // (1F4h) when absent, stored at unit+7C4h (006F285F).
    bool landing_range_present{false};
    std::int32_t landing_range_raw{500};   // 006F2847 LandingRange -> unit+7C4h
    // Packet cc9_building_pad_model: 006F2780's `LandingPointRange` find (key
    // 00CFAE0C, 006F2895), the found record's +0Ch dword or 1F4h (006F28A8),
    // stored at unit+7CCh (006F28B3); 006F5CC0 adopts the pads within it.
    bool landing_point_range_present{false};
    std::int32_t landing_point_range_raw{500};
    // Packet cc9_land_convoy_members: 00743450's reads from a LandConvoy's bag,
    // merged with the library group (landconvoy.props). `convoy_slots` holds the
    // Rows * Columns slot map after the Type1..4 x Position1..4 walk: the resolved
    // `Type` enum value per slot, -1 where nothing was placed. A value below 1
    // makes no member (00743799).
    bool land_convoy_keys{false};
    std::int32_t convoy_rows{0};          // +354h, "Rows"
    std::int32_t convoy_columns{0};       // +358h, "Columns"
    float convoy_row_gap{0.0f};           // +35Ch
    float convoy_column_gap{0.0f};        // +360h
    float convoy_hp{0.0f};                // +364h
    float convoy_speed{0.0f};             // +368h
    float convoy_offset{0.0f};            // +3ACh
    bool convoy_reverse{false};           // +3A9h
    std::string convoy_path;              // "Path", resolved by 007420B0
    std::vector<int> convoy_slots;
    std::vector<std::string> convoy_slot_symbols;
    // Packet cc9_submarine_depth_level: the two finds of 00853630's scene stage
    // (00853B18 `Dive` 00D0B6A4, 00853B94 `TargetDive` 00CFCCF0). Each is taken
    // only when found with type word 0 (00853B26 / 00853BA2); an `E` value is
    // resolved through the library's enum table (`Depth`, global.enums) and its
    // integer is the record's +0Ch. `Sub(Ship)` defaults `Dive` to Surface, so
    // every scene submarine carries one.
    bool dive_present{false};
    std::int32_t dive_level{0};
    bool target_dive_present{false};
    std::int32_t target_dive_level{0};
};

// Per class token of the scene, the counts the milestone reports.
struct GameSceneClassTally {
    std::string name;
    int class_id{-1};
    bool registered{false};          // the class is in the 004f2800 table
    std::size_t seen{0};
    std::size_t generated{0};
    std::size_t rejected{0};
    std::size_t created{0};
    std::size_t registration_bodies{0};  // descriptor[2] runs on the registration pass
    std::uint32_t create_address{0};     // descriptor[1]
    std::uint32_t register_address{0};   // descriptor[2]
    std::string creator_state;           // concrete | record, with the owner area
    // Packet cc9_scene_path_landscape: scene objects the bound Path (004EA650)
    // and Landscape (004F1460) creators built. Kept apart from `created`, which
    // create_units reads as "make a unit from this record".
    std::size_t objects{0};
};

struct GameSceneContentsSummary {
    bool ran{false};
    std::string scene_path;
    std::string short_name;
    std::string block_name;             // the "2_" VFS file block 004d4eea opens
    bool scene_read{false};
    std::size_t scene_bytes{0};

    // The property-group and enum library the entity bags are merged against.
    bool library_loaded{false};
    std::size_t library_files{0};
    std::size_t property_groups{0};
    std::size_t enum_tables{0};
    std::size_t enum_symbols{0};
    // `OwnerPlayer` symbols the library's `Players` table did not resolve.
    std::size_t owner_player_unresolved{0};

    int effective_game_mode{-1};        // 004bca50 at the gate
    bool game_mode_forced_to_9{false};  // the .ema probe at 004d5458

    std::size_t registration_entities{0};  // entities visited by pass 2
    std::size_t instantiate_entities{0};   // entities visited by pass 3
    std::size_t generated{0};
    std::size_t rejected{0};
    std::size_t created{0};
    std::size_t unregistered_class_entities{0};
    std::size_t registration_bodies{0};    // descriptor[2] calls on pass 2
    std::size_t party_class_marks{0};      // 0095ba60 writes the pass produced
    std::size_t nested_entities{0};        // entities below the top level
    // Packet cc8_lua_generate_object: entities the instantiate pass held back
    // because their bag carries `Hidden`, which 0046D3C5 tests before the gate.
    // They are registered and not created, and they are the pool `GenerateObject`
    // instantiates from. docs/LUA_GENERATE_OBJECT_HOST.md.
    std::size_t held_back_hidden{0};
    std::size_t spawned_by_script{0};      // held-back entities GenerateObject built
    // Milestone 2q: created entities whose bag carries `StartSpeed`, the key
    // 00823590 finds and 008235B0..008235F7 seeds the order ring from.
    std::size_t start_speed_entities{0};

    std::vector<GameSceneClassTally> classes;
};

// The scene contents pass, owned by the mission frame host for the whole run.
//
// run_load_scene_contents_004d4df0 drives bsp::load_scene_contents_004d4df0,
// the reconstruction of 004d4df0, so the order the run executes is the
// recovered order rather than one this file invents. Each of the two
// read_scene_file steps runs bsp::run_scene_file_reader_0046df00 with the pass
// flags that call site pushes.
class GameSceneContentsHost {
public:
    GameSceneContentsHost(GameHostLog& log, GameVfsHost& vfs);
    ~GameSceneContentsHost();

    // Milestone 2m. 0095c640 reads `VehicleClass[index].Type` and two further
    // fields out of the live Lua table the recovered global-script step already
    // loaded, so the registration pass needs the mission Lua host. Attached
    // before the pass runs; without it the preload keeps its record.
    void attach_lua(GameMissionLuaHost* lua) noexcept;
    GameSceneContentsHost(const GameSceneContentsHost&) = delete;
    GameSceneContentsHost& operator=(const GameSceneContentsHost&) = delete;

    // 004d4df0 at 004e03e5, the `load_scene_contents` row of the load walk.
    // `raw_game_mode`, `mode_forced` and `multiplayer_session` are game+614h,
    // game+61Ch and game+1FE4h, the three inputs 004bca50 reads.
    void run_load_scene_contents_004d4df0(const std::string& scene_path,
        const std::string& override_name, int raw_game_mode, bool mode_forced,
        bool multiplayer_session);

    const GameSceneContentsSummary& summary() const noexcept;
    const std::vector<GameSceneEntityRecord>& entities() const noexcept;
    // Actual parsed header properties, including Map.BorderSizeX/Y and the
    // MultiPlayMapSizes block. This is source data, not a fabricated world box.
    const bsp::ScenePropertyBlock& root_properties() const noexcept;
    // The entities the instantiate pass created, which is what the frame's unit
    // passes would walk if the world object the gate at 00875e69 tests existed.
    std::size_t created_unit_count() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

// ---------------------------------------------------------------------------
// The GenerateObject pool
// ---------------------------------------------------------------------------
// Not a native structure. The executable keeps the authored objects in the scene
// database's named-object map at `sceneDb+18h`, which `0046D930` looks a name up
// in; this process has no scene database, so the instantiate pass publishes the
// entities it held back at `0046D3C5` into one process-wide table instead. That
// is the same shape `bsp::air_ops_decks()` already uses for the decks.
//
// An entry stays after it is spawned, with `spawned` set and the entity id the
// script's table knows it by, because `0046D930` is reached once per script call
// and a mission that asks twice must not get two carriers.
// docs/LUA_GENERATE_OBJECT_HOST.md.
struct SceneSpawnPoolEntry {
    GameSceneEntityRecord record;
    bool spawned{false};
    int entity_id{0};
    // A held-back carrier or airfield never reaches the deck build at scene load,
    // because 006CADD0 mode 1 runs on the created path this entity skipped. The
    // deck is authored in the same property bag, so it is built at hold-back time
    // and registered when the script spawns the unit; otherwise
    // `GetProperty(carrier, "slots")` would find nothing and the launch gates
    // would refuse a carrier the script had just created.
    bool has_deck{false};
    AirOpsDeck deck;
    // Same reason as the deck, for a held-back PlaneSquadronGen row: 007F4580's
    // mode-1 loop reads `WingCount` (00CF8840) out of the property bag, and the
    // bag does not survive the hold-back. The key is read here so the wing can
    // be spawned when the script creates the unit; absent means the code default
    // of 3 at 007F473A. docs/PLANE_SQUADRON_HOST.md.
    bool wing_count_present{false};
    int wing_count_raw{0};
};

class SceneSpawnPool {
public:
    void clear() noexcept;
    void add(const GameSceneEntityRecord& record);
    SceneSpawnPoolEntry* find(const std::string& name) noexcept;
    std::size_t size() const noexcept;
    std::size_t spawned_count() const noexcept;

private:
    std::vector<SceneSpawnPoolEntry> entries_;
};

SceneSpawnPool& scene_spawn_pool() noexcept;

// Packet cc8_ship_moveonpath. The authored `Path` entities (class 0x47), with
// their `Point%02i.Pos` triples carried into WORLD space by the record's own
// composed frame through the recovered 00B62D10, which is the transform
// 007AF800 already applies to the same points on the avoidance side.
//
// Why a process-level registry and not a member. `NavigatorMoveOnPath` reaches
// the command path through GameScriptOrdersHost, which holds a GameUnitsHost
// and nothing else; the scene-contents host is not on that chain and the one
// place that owns both (src/game_hosts_mission_frame.cpp) is another agent's
// file. This is the same shape scene_spawn_pool() already uses for the same
// reason, and like it, it is cleared with the rest of the scene state.
struct ScenePathEntry {
    std::string name;
    std::vector<std::array<float, 3>> points_world;
};

class ScenePathRegistry {
public:
    void clear() noexcept;
    void add(const std::string& name, std::vector<std::array<float, 3>> points_world);
    const ScenePathEntry* find(const std::string& name) const noexcept;
    std::size_t size() const noexcept;

private:
    std::vector<ScenePathEntry> entries_;
};

ScenePathRegistry& scene_path_registry() noexcept;

// ---------------------------------------------------------------------------
// Packet cc9_scene_path_landscape: the Path and Landscape scene creators.
// docs/SCENE_CONTENTS_HOSTS.md section 5 has the evidence and the contracts.
//
// 004EA650 (Path, class 47h) and 004F1460 (Landscape, class 44h) are the same
// template, both __fastcall with EDX = name and four stack arguments (RET 10h
// at 004EA753 and 004F1553): operator new (210h / 430h) and memset, the base
// constructor 00928630 with network id 0 (006AF3D0 consumes the name string and
// returns XOR AX,AX at 006AF40F), the class constructor (0047B660 / 004F11C0),
// then vtable+98h = 00928860 BSP_GameEntity_PlaceInWorld(stack arg 1, the world
// [[00E188A8]+19CCh], the frame), then the name copied to entity+154h.
// PlaceInWorld stores the world at entity+30h and calls the class's slot 130h:
//   Path      00487210: 00928560 (world+24h, every entity) then world+36Ch;
//   Landscape 004F1400: 00928560 (world+24h) then world+348h.
// The world's list heads sit at world+18h + class*0Ch, so both lists are the
// class's own; 00424D00 reads the Path list's head at world+370h, and
// 00903860 / 009038F0 / 009039D0 / 00903BC0 read the Landscape list's head at
// world+34Ch. The properties are not read by the creators: the Path's Party and
// PathPoints load in 00925F20 pass B (slot A0h = 007B38D0) and the Landscape's
// FilePath / ModelPath / terrain load in pass A (slot 9Ch = 00883BB0).
//
// True binds the two creators: the instantiate pass builds a SceneWorldObject
// for each generated row and appends it to the class list below, and the
// Landscape's terrain file names are resolved against the VFS as a census.
// Nothing consumes the objects yet (see the doc's contracts). Committed false
// with the prediction that no measured row moves; the USN01 and USN04 pairs were
// identity on every death, hit and shot row, so it is ON. False keeps
// the record path: `SceneContents::class_creator` stays UNIMPLEMENTED.
inline constexpr bool kScenePathLandscapeCreatorsBound = true;

// Packet cc9_scene_race_and_script_identity (docs/SENTITY_INIT_ATTACH_ORDER.md
// section 17). True: (1) the mission frame hands each scene unit and each marker
// its record's Race, which the Lua host's load-time pass A writes into
// thisTable[key].Race (00928F50 for a unit, 00928100 for a marker; both read
// entity+58h, which 00927050 set from the bag); (2) a CreateScript entity gets
// 00928100's fields (Race -1, Party 2, Type SCRIPTENTITY 00D19034, no Name) on
// the mission frame after its creation. SUBSTITUTION for (2): the image writes
// them at the next InitAll's pass C, fixed-step row 12. False: neither is
// written, as before.
inline constexpr bool kSceneRaceAndScriptIdentityBound = true;

// Packet cc9_scene_home_base_contract (docs/CONSTRUCT_WORLD.md section 31). True:
// create_units hands every squadron built from a PlaneSquadronGen row its
// row's HomeBase name through set_squadron_scene_home_base, before the
// squadron's InitAll pass C reads it (007F4C43). Covers the load-time rows and
// the held-back rows GenerateObject/SpawnNew create. False: never called.
inline constexpr bool kSceneHomeBaseContractBound = true;

inline constexpr int kScenePathClassId = 0x47;       // 0047B660 stores [+C4h] = 47h
inline constexpr int kSceneLandscapeClassId = 0x44;  // 004F11C0 stores [+C4h] = 44h

// A host stand-in for the native Path (210h bytes, vtable 00CE6290) or
// Landscape (430h bytes, vtable 00CEA090) entity. The comments give the native
// field each member stands for; this is not the native layout.
struct SceneTerrainHeightField;

struct SceneWorldObject {
    int class_id{-1};                // +C4h
    std::string name;                // +154h (length) / +158h (buffer)
    std::size_t scene_id{0};         // the record's visitation id, not a pointer
    std::size_t parent_scene_id{0};  // the authored parent's record id, 0 = none
    std::uint32_t vtable{0};         // +0h
    std::uint32_t object_size{0};    // the operator new size
    std::uint16_t network_id{0};     // 006AF3D0's answer, always 0
    std::uint32_t class_list_offset{0};  // world+36Ch (Path) or world+348h (Landscape)
    float world[16]{};               // the composed frame (see the doc, "frame")

    // Path, 00925F20 pass B (007B38D0), taken from the retained record.
    int party{-1};                   // +54h, 007B38FC
    std::size_t path_points{0};      // the +1E4h component's point count
    bool path_points_valid{false};

    // Landscape, 00925F20 pass A (00883BB0), read from the merged bag.
    std::string file_path;           // +3C4h, `FilePath` (00883C5E)
    std::string model_path;          // +424h, `ModelPath` (00883C7B)
    bool shallow_water_block{false}; // `ShallowWater` (00883E1D) is authored
    // 00882AC0's three names, built from FilePath, and whether this process's
    // VFS resolves each one. A census: the terrain object +3D0h, the model
    // +41Ch / +420h, the render node +3CCh and the part instance +418h are not
    // built.
    std::string heightmap_name;      // "terrain/" FilePath "_heightmap.tdt"
    std::string colormap_name;       // "terrain/" FilePath "_colormap.dds"
    std::string model_name;          // "models/terrain/" FilePath ".mmod"
    bool heightmap_resolved{false};
    bool colormap_resolved{false};
    bool model_resolved{false};

    // Packet cc9_landscape_terrain: the terrain object +3D0h, when
    // kSceneLandscapeTerrainBound loaded it. Null otherwise.
    std::shared_ptr<const SceneTerrainHeightField> terrain;
};

// ---------------------------------------------------------------------------
// Packet cc9_landscape_terrain. docs/SCENE_CONTENTS_HOSTS.md section 6.
//
// The Landscape's terrain object (+3D0h, class vtable 00D5D350, built by
// 00ADD290) and the four world queries over list 44h: 00903860 ground height,
// 009038F0 ground normal, 009039D0 the Landscape under a point and 00903BC0
// the segment test. The terrain is the `.tdt` height field 00ADDA60 parses
// (both arms of 00882AC0's branch at 00882EE6 reach it: 00883A7D directly and
// 00ADE820 -> 00ADE500 -> 00ADDA60).
//
// True loads each Landscape's height field at the pass-A point and answers the
// queries below. No consumer calls them: the 30 native call sites of 00903860
// and the others stay in their owners' files (see the doc's contracts), so the
// switch was committed false with the prediction that no measured row moves;
// the USN01 and USN04 pairs were identity, so it is ON.
inline constexpr bool kSceneLandscapeTerrainBound = true;

// Packet cc9_terrain_segment_quadtree (docs/SCENE_CONTENTS_HOSTS.md section
// "Slot 3Ch reconstructed"). When set, terrain slot 3Ch 00ADA240 walks the
// quadtree 00AEA820 builds (00AEA2B0 -> 00AE9D80 -> leaf 00AE9BD0 -> tile DDA
// 00ADF1B0 -> line-quad test 00ADEB80) in place of the half-cell march, and
// 00903BC0 asks every Landscape's slot 3Ch (full inverse frame, rotation
// included) in place of its world-space march. The vertical case 00AECC40 is
// still the march (its sub-walk 00AECA60 is unread; labelled).
// ON since the pairs (docs/SCENE_CONTENTS_HOSTS.md 10.5).
inline constexpr bool kTerrainSegmentQuadtreeBound = true;

// One 33x33 sample block of a `NODE` tile (00ADFD70: 00ADC420(21h, 21h), block
// vtable 00D5D314, 2 bytes per sample). 00ADC6C0 reads `offset` (+2Ch) and
// `scale` (+30h) and stores 1/scale at +34h; 00ADC5F0 answers a sample as
// -1000.0 [00D7A240] for FFFFh, else float(s * inv_scale + offset).
struct SceneTerrainBlock {
    float offset{0.0f};
    float inv_scale{0.0f};
    std::array<std::uint16_t, 33 * 33> samples{};
};

// The terrain object's height field. Values are the native's:
//   +18h tile world size 300.0 [00CE3AE8] and +1Ch cell size 9.375 [00D0E658]
//   (00AE9870); the tile grid +38h x +3Ch with its block pointers at +40h
//   (00ADADC0, from the root's two dwords); the origin +80h / +84h, which
//   00ADDA60 sets for a `TRNV2` root to floor(min / 300 - 1) * 300 of the box
//   00ADA420 accumulates at +68h / +70h (0 for any other root name).
struct SceneTerrainHeightField {
    std::string root;               // `TRNV2` on this installation's islands
    int tiles_wide{0};              // +38h
    int tiles_deep{0};              // +3Ch
    std::vector<int> block_index;   // +40h: tiles_wide*tz + tx -> blocks, or -1
    std::vector<SceneTerrainBlock> blocks;
    float sample_min{0.0f};         // lowest non-hole sample (load census)
    float sample_max{0.0f};         // highest non-hole sample
    float origin_x{0.0f};           // +80h
    float origin_z{0.0f};           // +84h
    float box_min_x{0.0f};          // +68h (see the doc: the model's BoundingBox)
    float box_min_z{0.0f};          // +70h
    // The terrain node +2Ch's world translation (+120h / +124h / +128h),
    // which 00ADE820 sets from the Landscape's +74h frame.
    float node_x{0.0f};
    float node_y{0.0f};
    float node_z{0.0f};

    // Terrain slot 20h, 00ADB3A0: the height of sample (i, j), node y added;
    // -1000.0 outside [0, tiles*32] or in a missing tile or a hole.
    float cell_height_00adb3a0(int i, int j) const noexcept;
    // Terrain slot 28h, 00ADA900 -> slot 48h 00ADB480: bilinear height at a
    // world (x, z).
    float height_00ada900(float x, float z) const noexcept;
    // Terrain slot 38h, 00ADABA0 -> slot 30h 00ADAA40: the unit normal of the
    // cell the world (x, z) truncates to.
    void normal_00adaba0(float x, float z, float out[3]) const noexcept;
    // Packet cc9_land_convoy_movement: the local-frame slots the forwarders
    // 0087FA20 ([+3D0h]->vtable[24h]) and 0087FB90 (vtable[34h]) reach. The
    // vtable is 00D5D350 (xrefs 00ADD1CE, 00ADD2C9), so +24h is 00ADA160 and
    // +34h is 00ADA1C0: (x, z) in the Landscape's frame, no node translation.
    float local_height_00ada160(float x, float z) const noexcept;
    void local_normal_00ada1c0(float x, float z, float out[3]) const noexcept;
    float grid_height_00adb480(float u, float v) const noexcept;       // slot 48h
    void cell_normal_00adaa40(int i, int j, float out[3]) const noexcept;  // slot 30h

    // Packet cc9_terrain_segment_quadtree: the tree 00AEA820 builds at the end
    // of 00ADDA60 (20-byte nodes at +3Ch: min y, max y, first child or -1, tile
    // x, tile z), built here on the first slot 3Ch query. `quadtree_leaves` is
    // +38h, 1 << depth tiles per side.
    struct QuadNode {
        float min_y{0.0f};
        float max_y{0.0f};
        int children{-1};
        int tile_x{0};
        int tile_z{0};
    };
    mutable std::vector<QuadNode> quadtree;
    mutable int quadtree_leaves{0};
};

// The four world queries, over scene_world_class_lists().list(44h) in list
// order. All return what the native returns; `landscape` is an index into
// scene_world_class_lists().objects(), or -1.
bool world_ground_height_00903860(const float point[3], float& out) noexcept;
bool world_ground_normal_009038f0(const float point[3], float normal[3]) noexcept;
int world_landscape_at_009039d0(const float point[3]) noexcept;
// LABELLED STAND-IN for the per-terrain sweep (slot 3Ch 00ADA240 -> the
// quadtree ray walk 00AEA2B0 / 00AE9D80 and the vertical case 00AECC40, not
// reconstructed): the segment is marched in half-cell steps against
// height_00ada900. The two endpoint ground tests are the native's.
bool world_segment_blocked_00903bc0(const float from[3], const float to[3]) noexcept;

struct SceneTerrainQueryCensus {
    unsigned long long height_calls{0}, height_hits{0}, height_fallbacks{0};
    unsigned long long normal_calls{0}, normal_hits{0};
    unsigned long long landscape_calls{0}, landscape_hits{0};
    unsigned long long segment_calls{0}, segment_endpoint_blocks{0};
    unsigned long long segment_sweep_blocks{0};
    // Packet cc9_terrain_segment_consumers: Landscape entries 00903C20..00903C42
    // asked through slot 3Ch, over the run (the load summary prints before play).
    unsigned long long segment_sweep_entries{0};
};

// Packet cc9_terrain_segment_quadtree: slot 3Ch calls by path, over the run.
struct SceneTerrainQuadtreeCensus {
    unsigned long long walks{0}, walk_hits{0};
    unsigned long long vertical{0}, vertical_hits{0};
    unsigned long long leaves{0}, cells{0};
    // Packet cc9_terrain_vertical_subwalk: 00AECC40's equal-point answers, its
    // 00AECA60 walks, and the tiles and cells those walks tested.
    unsigned long long vertical_equal{0}, vertical_walks{0};
    unsigned long long vertical_tiles{0}, vertical_cells{0};
};
SceneTerrainQuadtreeCensus& scene_terrain_quadtree_census() noexcept;
SceneTerrainQueryCensus& scene_terrain_query_census() noexcept;

// ---------------------------------------------------------------------------
// Packet cc9_landscape_attach_scene_half: the scene-host half of the
// Landscape's entry in the segment query (docs/SCENE_CONTENTS_HOSTS.md
// section 8). No caller yet: the SegmentBinding hunk in
// src/game_hosts_gunnery.cpp adds one loose entry per Landscape and calls these.
//
// Entries are the Landscapes of list 44h whose height field loaded, in list
// order (the order 00884078 appended their nodes to the loose array).
std::size_t landscape_segment_entry_count() noexcept;
// The entry's index into scene_world_class_lists().objects(), or -1.
int landscape_segment_entry_object(std::size_t entry) noexcept;
// The node's world box: the tile grid [origin, origin + tiles*300] by the
// sample range, through the Landscape's world frame (8 corners). The native
// box is 0098A920's from the terrain's vt+0Ch (00884066); this is its analogue.
bool landscape_segment_entry_bounds(std::size_t entry, float box_min[3],
                                    float box_max[3]) noexcept;

// What 0087FF80 writes on a hit: the point (record+8h..+10h), the entity (the
// Landscape, 00470370 at 0087FFD9), the shape kind 0Ah (+30h) and the hull
// segment -1 (+34h). `fraction` is the hit's place along from->to.
struct LandscapeSegmentHit {
    float point[3]{0.0f, 0.0f, 0.0f};
    int landscape_object{-1};
    std::size_t entry{0};
    int shape_kind{0x0a};
    int hull_segment{-1};
    float fraction{1.0f};
};

// One entry's shape trace, the analogue of 0087FF80 -> terrain slot 3Ch
// 00ADA240. As 00ADA240 does, both endpoints go into the Landscape's local
// frame through the full inverse of its world frame (rotation included).
// LABELLED STAND-IN for 00ADA240's quadtree walk 00AEA2B0 / 00AE9D80 and its
// vertical case 00AECC40: a half-cell march in local space against the height
// field, the first sample below the surface refined by bisection. The hit
// point goes back to world through the frame.
bool landscape_entry_segment_hit(std::size_t entry, const float from[3],
                                 const float to[3], LandscapeSegmentHit& hit) noexcept;
// The nearest hit over every entry, as 0098ADD0 keeps the nearest.
bool landscape_segment_hit(const float from[3], const float to[3], float hit_point[3],
                           int& landscape_index) noexcept;

// The per-consumer counters the SegmentBinding hunk fills and prints.
enum class LandHitConsumer { PickRay = 0, GunSeat = 1, LineOfFire = 2, Projectile = 3 };
struct SceneLandHitCensus {
    unsigned long long calls[4]{0, 0, 0, 0};
    unsigned long long land_hits[4]{0, 0, 0, 0};
    unsigned long long line_of_fire_blocked{0};   // 0072CE91 answered AL = 1 on land
};
SceneLandHitCensus& scene_land_hit_census() noexcept;
void note_land_hit_query(LandHitConsumer consumer, bool land_hit) noexcept;
// "pick=<calls>/<land hits> seat=... line_of_fire=... blocked=<n> projectile=...".
std::string format_land_hit_census();

// The world's per-class entity lists (world+18h + class*0Ch, 97 heads built by
// 004CB030) for the two classes this packet creates, appended in creation
// order as 00484540 appends at the tail. Process-level for the same reason
// scene_path_registry() is, and cleared with the rest of the scene state.
class SceneWorldClassLists {
public:
    void clear() noexcept;
    void append(SceneWorldObject object);
    const std::vector<SceneWorldObject>& objects() const noexcept;
    // Indices into objects() of the class's list, head first.
    std::vector<std::size_t> list(int class_id) const;

private:
    std::vector<SceneWorldObject> objects_;
};

SceneWorldClassLists& scene_world_class_lists() noexcept;

// Packet cc9_ship_weapon_director_enable (docs/SENTITY_INIT_PASSES.md section
// 7). The ship's pass B, 00822C20's property-bag arm, reads `ArtilleryDirector`,
// `AADirector`, `TorpedoDirector` and `DCDirector` (008238F0..008239A3, finds
// through 008F2260, +0Ch tested) and hands them to the weapon director through
// 007214C0 / vt[3Ch] 00835690 -> 007219C0, which stores them at +220h..+223h
// (007219DB..007219ED). This keeps the merged bag's four values per entity name
// (group defaults included: universe/library/ship.props has TorpedoDirector =
// B false), for the gunnery host to read. Cleared with the scene.
struct SceneDirectorEnables {
    bool artillery{true};
    bool anti_air{true};
    bool torpedo{true};
    bool depth_charge{true};
};
void scene_director_enables_clear() noexcept;
void scene_director_enables_set(const std::string& name, const SceneDirectorEnables& enables);
const SceneDirectorEnables* scene_director_enables_find(const std::string& name) noexcept;
// Packet cc9_ship_torpedo_mask_read (docs/SENTITY_INIT_PASSES.md section 8). The
// two runtime writers of director+222h the reference missions reach, both
// through session message 5Ah sub-kind 5 (0071E0D0) that 0071C1E0 stores at
// 0071C25B: the Lua native TorpedoEnable 0089C8F0, and CLOSEATTACK's tail
// 00A11AF0 (JMP at 00A154F7), which sends 1 for every ship of its own group
// on every tick. A name with no entry takes the constructor's four 1s
// (007202FD) before the torpedo byte is written. SUBSTITUTION: the message is
// applied at the send, not routed through 0077C2A0 and delivered by the
// session.
struct SceneDirectorTorpedoWrites {
    unsigned long long lua_enables{0};
    unsigned long long lua_disables{0};
    unsigned long long close_attack_sends{0};
    unsigned long long changed{0};
};
bool scene_director_enables_set_torpedo(const std::string& name, bool enabled);
SceneDirectorTorpedoWrites& scene_director_torpedo_writes() noexcept;

}  // namespace bsp::game
