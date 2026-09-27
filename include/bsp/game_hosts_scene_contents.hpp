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
    bool generated{false};     // 0046c550 returned AL != 0
    std::string gate_rule;     // which rule of the gate produced that answer
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

inline constexpr int kScenePathClassId = 0x47;       // 0047B660 stores [+C4h] = 47h
inline constexpr int kSceneLandscapeClassId = 0x44;  // 004F11C0 stores [+C4h] = 44h

// A host stand-in for the native Path (210h bytes, vtable 00CE6290) or
// Landscape (430h bytes, vtable 00CEA090) entity. The comments give the native
// field each member stands for; this is not the native layout.
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
};

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

}  // namespace bsp::game
