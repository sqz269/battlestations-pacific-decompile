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

#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace bsp::game {

class GameHostLog;
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
    // The entities the instantiate pass created, which is what the frame's unit
    // passes would walk if the world object the gate at 00875e69 tests existed.
    std::size_t created_unit_count() const noexcept;

    struct Impl;

private:
    std::unique_ptr<Impl> impl_;
};

}  // namespace bsp::game
