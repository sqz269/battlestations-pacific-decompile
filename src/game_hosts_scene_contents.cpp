// bsp_game.exe milestone 2h: the scene contents pass.
// See include/bsp/game_hosts_scene_contents.hpp for the address list and the evidence.
#include "bsp/game_hosts_scene_contents.hpp"

#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/scene_traffic_groups.hpp"
#include "bsp/mission_scene_contents.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/pose_refresh.hpp"
#include "bsp/resource_lookup.hpp"
#include "bsp/scene_contents_hosts.hpp"
#include "bsp/scene_deferred_refs.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_unit_creators.hpp"
#include "bsp/simulation_gate.hpp"
#include "bsp/vehicle_class.hpp"
#include "bsp/vfs_mounts.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <algorithm>
#include <array>
#include <cctype>
#include <cstdio>
#include <cstring>
#include <map>
#include <stdexcept>
#include <string>
#include <vector>

namespace bsp::game {
namespace {

bool equal_insensitive(const std::string& a, const std::string& b) noexcept {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i) {
        const unsigned char lhs = static_cast<unsigned char>(a[i]);
        const unsigned char rhs = static_cast<unsigned char>(b[i]);
        if (std::tolower(lhs) != std::tolower(rhs)) return false;
    }
    return true;
}

void format_address(std::uint32_t address, char (&out)[16]) {
    std::snprintf(out, sizeof(out), "%08lx", static_cast<unsigned long>(address));
}

// ---------------------------------------------------------------------------
// The property-group and enum library
// ---------------------------------------------------------------------------
//
// `CPropTreeLibrary::Load` (008f67b0) reads the files under `universe/library`
// with the same tokenizer the scene reader uses (008d9cf0 / 008d8a70 / 008d9930
// / 008d9980), the delimiter set `;{}=:()` its body carries at 00cf..., and the
// same property-block parser 008f5a00 the `.scn` entity body goes through; its
// two top-level keywords are the literals `properties` and `enum`, which are
// also in its body. Its caller 008f6fc0 enumerates the directory through
// 00886280 and hands it one file at a time.
//
// **008f67b0 is not reconstructed and this reader is not a reconstruction of
// it.** It is the executable's own top-level dispatch over the two recovered
// pieces, and it exists because without the schema the bag an entity hands
// 0046c550 has none of the defaulted keys the gate reads, which changes what
// the gate decides. Everything below the `properties`/`enum` dispatch is
// recovered code: bsp::SceneLexer and bsp::parse_scene_property_block_008f5a00.
constexpr const char* kPropertyLibraryDirectory = "universe/library";
// 008f67b0's own delimiter literal. The scene reader's set adds the comma;
// this one does not, which is why it is written out rather than reused.
constexpr const char* kPropertyLibraryDelimiters = ";{}=:()";
constexpr const char* kPropertyLibraryKeywordProperties = "properties";  // 008f67b0
constexpr const char* kPropertyLibraryKeywordEnum = "enum";              // 008f67b0

struct PropertyGroupDefinition {
    std::string name;
    std::vector<std::string> bases;
    ScenePropertyBlock block;
};

struct EnumTable {
    std::string name;
    std::map<std::string, int> symbols;
};

class PropertyLibrary {
public:
    void add_group(PropertyGroupDefinition group) {
        groups_.push_back(std::move(group));
    }
    void add_enum(EnumTable table) { enums_.push_back(std::move(table)); }

    const PropertyGroupDefinition* group(const std::string& name) const {
        for (const PropertyGroupDefinition& row : groups_) {
            if (equal_insensitive(row.name, name)) return &row;
        }
        return nullptr;
    }

    // 00438e10 is case insensitive everywhere the scene reader compares names,
    // and the shipped files spell `DeRuyter` identically in both places, so the
    // fold costs nothing and matches the reader's other lookups.
    bool resolve_symbol(const std::string& table, const std::string& symbol,
        int& value) const {
        for (const EnumTable& row : enums_) {
            if (!table.empty() && !equal_insensitive(row.name, table)) continue;
            for (const auto& entry : row.symbols) {
                if (equal_insensitive(entry.first, symbol)) {
                    value = entry.second;
                    return true;
                }
            }
            if (!table.empty()) return false;
        }
        return false;
    }

    std::size_t group_count() const noexcept { return groups_.size(); }
    std::size_t enum_count() const noexcept { return enums_.size(); }
    std::size_t symbol_count() const noexcept {
        std::size_t total = 0;
        for (const EnumTable& row : enums_) total += row.symbols.size();
        return total;
    }

private:
    std::vector<PropertyGroupDefinition> groups_;
    std::vector<EnumTable> enums_;
};

// The bag an entity hands 0046c550 is built in two steps by 0046cf40. Step 6
// merges each name of the `properties ( ... )` list into it through 008f54f0,
// which is the group schema; step 7 parses the authored body into the same bag
// through 008f5a00, and a parse writes the key whatever was there before. So a
// group merge adds and an authored assignment overwrites, which is what
// `overwrite` selects here. A group's own key beats its base's for the same
// reason the derived declaration wins in `properties Ship(Common)`.
void merge_property_block(ScenePropertyBlock& bag, const ScenePropertyBlock& other,
    bool overwrite) {
    for (const SceneProperty& value : other.values) {
        SceneProperty* existing = nullptr;
        for (SceneProperty& candidate : bag.values) {
            if (equal_insensitive(candidate.key, value.key)) { existing = &candidate; break; }
        }
        if (existing == nullptr) {
            bag.values.push_back(value);
        } else if (overwrite) {
            *existing = value;
        }
    }
    for (const auto& block : other.blocks) {
        ScenePropertyBlock* target = nullptr;
        for (auto& existing : bag.blocks) {
            if (equal_insensitive(existing.first, block.first)) {
                target = &existing.second;
                break;
            }
        }
        if (target == nullptr) {
            bag.blocks.emplace_back(block.first, ScenePropertyBlock{});
            target = &bag.blocks.back().second;
        }
        merge_property_block(*target, block.second, overwrite);
    }
}

void merge_group_into(const PropertyLibrary& library, const std::string& name,
    ScenePropertyBlock& bag, int depth) {
    if (depth > 8) return;  // the shipped library nests one level
    const PropertyGroupDefinition* group = library.group(name);
    if (group == nullptr) return;
    merge_property_block(bag, group->block, false);
    for (const std::string& base : group->bases) {
        merge_group_into(library, base, bag, depth + 1);
    }
}

// ---------------------------------------------------------------------------
// The gate's pose resolver
// ---------------------------------------------------------------------------
//
// 0046c550 reaches a pose only through 0046c6b7, which runs when the entity has
// a non-null parent identity. The scene-file reader composes the parent frame
// itself and hands 0046cf40 a null parent, so this resolver is never entered on
// the path the load takes. It records the call rather than inventing a pose.
class SceneGatePoseResolver final : public PoseRefreshResolver {
public:
    explicit SceneGatePoseResolver(GameHostLog& log) : log_(log) {}
    PoseRefreshView& resolve_pose(void* actual_owner) override {
        static_cast<void>(actual_owner);
        log_.unimplemented("SceneGate::resolve_parent_pose", "0046c6b7");
        throw std::runtime_error("the scene gate asked for a parent pose this process "
            "does not own");
    }

private:
    GameHostLog& log_;
};

}  // namespace

// ---------------------------------------------------------------------------
// Impl
// ---------------------------------------------------------------------------

struct GameSceneContentsHost::Impl {
    Impl(GameHostLog& log_in, GameVfsHost& vfs_in) : log(log_in), vfs(vfs_in) {
        // The vehicle-class registry's forward index map at singleton+10h.
        // 00592640 and 00506550 reset all 800h pairs to the identity and then
        // rewrite exactly one, so a `Type` nobody has remapped resolves to
        // itself; 00592640 is the footer command this run already takes on the
        // way into the mission. Without the identity fill 0095ba60 would find
        // no class index and mark no party, which is not what the native does.
        vehicle_registry.type_to_class_index.resize(kVehicleClassIndexMapSize);
        for (std::size_t i = 0; i < vehicle_registry.type_to_class_index.size(); ++i) {
            vehicle_registry.type_to_class_index[i] = static_cast<int>(i);
        }
    }

    GameHostLog& log;
    GameVfsHost& vfs;
    // Milestone 2m: the mission Lua host, for 0095c640's three table reads.
    GameMissionLuaHost* lua{nullptr};
    // 00f8a09c, the deduplicated preload census, and 00f8a0b0, the stock queue.
    std::vector<std::int32_t> preload_census;
    std::vector<std::int32_t> stock_queue;
    GameSceneContentsSummary summary;
    std::vector<GameSceneEntityRecord> entities;
    PropertyLibrary library;
    VehicleClassRegistry vehicle_registry;
    std::string scene_path;
    std::string override_name;
    std::string scene_text;
    int raw_game_mode{0};
    bool mode_forced{false};
    bool multiplayer{false};
    bool mode_forced_to_nine{false};
    // The set at scene database +164h. 0046f160, the scene-database constructor
    // at 00e18680, inserts exactly two class ids into it: 0046f2f2 pushes 47h
    // (Path) and 0046f30b pushes EBX, loaded with 3Dh (Cloud) at 0046f2d5.
    std::vector<int> always_generate{0x47, 0x3d};

    GameSceneClassTally& tally(const std::string& class_name) {
        for (GameSceneClassTally& row : summary.classes) {
            if (row.name == class_name) return row;
        }
        GameSceneClassTally row;
        row.name = class_name;
        const SceneEntityClassRow* klass = find_scene_entity_class_by_name(class_name);
        if (klass != nullptr) {
            row.registered = true;
            row.class_id = klass->class_id;
            row.create_address = klass->create_address;
            row.register_address = klass->register_address;
        }
        summary.classes.push_back(row);
        return summary.classes.back();
    }

    void load_property_library();
    bool read_vfs_file(const std::string& name, std::string& text);
    void parse_library_file(const std::string& name, const std::string& text);

    // Milestone 2m. The reader's own property bag, the one 008f41a0 constructs
    // at 0046e097 and the weather pass writes its four shadow keys into. It is a
    // stack local of 0046df00 and dies with the reader call, which is why
    // nothing outside this object reads it.
    ScenePropertyBlock weather_bag;
    std::size_t weather_entries{0};
    std::size_t weather_sub_scenes{0};
    int weather_selected{-1};
    std::string weather_descriptor;
    std::size_t weather_shadow_writes{0};
    bool logged_weather_empty{false};
    // The `.nav` bytes the avoid-zone load reads, and the layers it appended.
    std::string avoid_zone_bytes;
    std::vector<TerrainGridLayerRecord> avoid_zone_layers;
    // The clouds the scatter placed. record+C84h is never filled by the header
    // pass, so the gate closes and the list stays empty.
    std::size_t clouds_placed{0};
    int cloud_gate{0};
};

bool GameSceneContentsHost::Impl::read_vfs_file(const std::string& name, std::string& text) {
    VfsProviderManager* manager = vfs.manager();
    if (manager == nullptr) return false;
    std::string resolved = name;
    if (!resolve_existing_resource_00bdf4c0_fragment(manager->context(),
            vfs.search_registrations(), resolved)) {
        return false;
    }
    VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(manager->context(),
        resolved, 2);
    if (!opened.provider_opened || !opened.stream || !opened.stream->fully_initialized()) {
        return false;
    }
    text.assign(reinterpret_cast<const char*>(opened.stream->data_00bef610()),
        static_cast<std::size_t>(opened.stream->size_00bef600()));
    return true;
}

void GameSceneContentsHost::Impl::parse_library_file(const std::string& name,
    const std::string& text) {
    SceneLexer lexer(text, kPropertyLibraryDelimiters);
    std::vector<std::string> errors;
    while (!lexer.at_end()) {
        const SceneToken token = lexer.next();
        if (token.is_end()) break;
        if (equal_insensitive(token.text, kPropertyLibraryKeywordProperties)) {
            PropertyGroupDefinition group;
            group.name = lexer.next().text;
            if (lexer.peek().text == "(") {
                lexer.next();
                while (!lexer.at_end() && lexer.peek().text != ")") {
                    const std::string base = lexer.next().text;
                    if (!base.empty()) group.bases.push_back(base);
                }
                if (!lexer.at_end()) lexer.next();
            }
            group.block = parse_scene_property_block_008f5a00(lexer, errors);
            library.add_group(std::move(group));
            continue;
        }
        if (equal_insensitive(token.text, kPropertyLibraryKeywordEnum)) {
            EnumTable table;
            table.name = lexer.next().text;
            if (lexer.peek().text != "{") continue;
            lexer.next();
            while (!lexer.at_end() && lexer.peek().text != "}") {
                const SceneToken symbol = lexer.next();
                if (lexer.peek().text != "=") continue;
                lexer.next();
                const SceneToken value = lexer.next();
                std::int32_t parsed = 0;
                if (scene_scan_int(value.text, parsed)) {
                    table.symbols[symbol.text] = static_cast<int>(parsed);
                }
            }
            if (!lexer.at_end()) lexer.next();
            library.add_enum(std::move(table));
            continue;
        }
        // 008f67b0 dispatches on those two keywords only; anything else is the
        // caller's problem and is skipped here as the top-level loop skips it.
    }
    static_cast<void>(name);
}

void GameSceneContentsHost::Impl::load_property_library() {
    // 008f6fc0 enumerates the library directory through 00886280 and calls
    // 008f67b0 once per file. The enumeration is the recovered one; the load is
    // this file's stand-in, and the host record says so.
    log.unimplemented("SceneContents::property_library_load", "008f67b0");
    VfsProviderManager* manager = vfs.manager();
    if (manager == nullptr) return;
    // 008f7100 calls 008f6fc0 twice, first with ".enums" (00d1655c at 008f7106)
    // and then with ".props" (00d16554 at 008f7113), so the enum tables are
    // loaded before the property groups. The order is kept because a later file
    // that redeclared a name would otherwise win.
    const char* extensions[] = {".enums", ".props"};
    std::vector<std::string> names;
    for (const char* extension : extensions) {
        std::vector<std::string> found;
        std::string error;
        if (!enumerate_resources_00bdd990_fragment(manager->context(),
                kPropertyLibraryDirectory, extension, 0, found, error)) {
            log.notef("property library: %s enumeration failed: %s", extension,
                error.c_str());
            continue;
        }
        std::sort(found.begin(), found.end());
        names.insert(names.end(), found.begin(), found.end());
    }
    for (const std::string& entry : names) {
        std::string text;
        std::string full = entry;
        if (!read_vfs_file(full, text)) {
            full = std::string(kPropertyLibraryDirectory) + "/" + entry;
            if (!read_vfs_file(full, text)) {
                log.notef("property library: %s did not resolve", entry.c_str());
                continue;
            }
        }
        parse_library_file(full, text);
        ++summary.library_files;
    }
    summary.property_groups = library.group_count();
    summary.enum_tables = library.enum_count();
    summary.enum_symbols = library.symbol_count();
    summary.library_loaded = summary.library_files != 0;
    log.notef("property library: %zu file(s) under %s, %zu property group(s), %zu enum "
        "table(s), %zu symbol(s) (008f67b0 CPropTreeLibrary::Load is not reconstructed; "
        "this is the executable's own reader over the recovered tokenizer and 008f5a00)",
        summary.library_files, kPropertyLibraryDirectory, summary.property_groups,
        summary.enum_tables, summary.enum_symbols);
}

namespace {

// ---------------------------------------------------------------------------
// Milestone 2m: 0095c640's own host, over the live VehicleClass table
// ---------------------------------------------------------------------------
class VehicleClassPreloadBinding final : public bsp::VehicleClassPreloadHost {
public:
    explicit VehicleClassPreloadBinding(GameSceneContentsHost::Impl& owner)
        : owner_(owner) {}

    std::int32_t map_class_index(std::int32_t class_id) override {
        // registry+10h+id*4, the forward index map the executable fills with
        // the identity (milestone 2h's own note on 00592640 and 00506550).
        const std::size_t id = static_cast<std::size_t>(class_id);
        if (class_id < 0 || id >= owner_.vehicle_registry.type_to_class_index.size()) {
            return class_id;
        }
        return static_cast<std::int32_t>(owner_.vehicle_registry.type_to_class_index[id]);
    }
    bool read_class_type(std::int32_t class_index, std::string& type) override {
        if (owner_.lua == nullptr) return false;
        const GameVehicleClassRow row
            = owner_.lua->read_vehicle_class_row(static_cast<int>(class_index));
        if (!row.found) return false;
        type = row.type;
        return !type.empty();
    }
    std::int32_t read_landing_ship_class(std::int32_t class_index) override {
        if (owner_.lua == nullptr) return 0;
        return static_cast<std::int32_t>(owner_.lua->read_vehicle_class_integer(
            static_cast<int>(class_index), "LandingShip", nullptr, 0));
    }
    std::int32_t read_catapult_launched_class(std::int32_t class_index) override {
        if (owner_.lua == nullptr) return -1;
        return static_cast<std::int32_t>(owner_.lua->read_vehicle_class_integer(
            static_cast<int>(class_index), "Catapult", "LaunchedClass", -1));
    }
    void append_census(std::int32_t class_index) override {
        for (std::int32_t existing : owner_.preload_census) {
            if (existing == class_index) return;
        }
        owner_.preload_census.push_back(class_index);
    }
    bool pop_stock_queue(std::int32_t& class_index) override {
        if (owner_.stock_queue.empty()) return false;
        class_index = owner_.stock_queue.front();
        owner_.stock_queue.erase(owner_.stock_queue.begin());
        return true;
    }
    void push_stock_queue(std::int32_t class_index) override {
        for (std::int32_t existing : owner_.stock_queue) {
            if (existing == class_index) return;
        }
        owner_.stock_queue.push_back(class_index);
    }

private:
    GameSceneContentsHost::Impl& owner_;
};

// ---------------------------------------------------------------------------
// 0046c550's remaining services
// ---------------------------------------------------------------------------

class SceneGateBinding final : public SceneEntityGateHost {
public:
    SceneGateBinding(GameSceneContentsHost::Impl& owner, int effective_mode)
        : owner_(owner), mode_(effective_mode) {}

    SceneGameMode effective_game_mode() override {
        owner_.log.implemented("SceneGate::effective_game_mode", "004bca50");
        return static_cast<SceneGameMode>(mode_);
    }
    SceneModeArea mode_area(int slot) override {
        // *(00e188a8)+705Ch + slot*18h. This process builds no play-area table,
        // and single player never reaches a mode with one.
        static_cast<void>(slot);
        owner_.log.unimplemented("SceneGate::mode_area", "004c8760");
        return SceneModeArea{};
    }
    std::string parent_name(void* captured_parent_identity) override {
        static_cast<void>(captured_parent_identity);
        owner_.log.unimplemented("SceneGate::parent_name", "0046c550+vtable10");
        return std::string();
    }
    void append_deferred_entity_record(const SceneDeferredEntityRecord& record) override {
        static_cast<void>(record);
        ++deferred_records_;
        owner_.log.unimplemented("SceneGate::append_deferred_entity_record", "0046c450");
    }
    void register_multiplayer_stock(const ScenePropertyBlock& properties,
        const std::string& class_name) override {
        static_cast<void>(properties);
        static_cast<void>(class_name);
        ++stock_walks_;
        owner_.log.unimplemented("SceneGate::register_multiplayer_stock", "0046bf20");
    }

    std::size_t deferred_records() const noexcept { return deferred_records_; }
    std::size_t stock_walks() const noexcept { return stock_walks_; }

private:
    GameSceneContentsHost::Impl& owner_;
    int mode_{8};
    std::size_t deferred_records_{0};
    std::size_t stock_walks_{0};
};

// ---------------------------------------------------------------------------
// The unit creators, 004f0520 and its siblings
// ---------------------------------------------------------------------------
//
// create_instance_from_descriptor hands back the executable's own entity
// record, the same substitution milestone 2c makes for the seven main-menu
// screen classes: the native allocation (the descriptor's vtable +28h, 006fe590
// for MDestroyer, operator new(1188h) plus its constructor) is recorded as
// unimplemented and the rest of the recovered creator body runs over a record
// this process owns. Nothing about the record claims the native layout.
class SceneUnitCreatorBinding final : public SceneUnitCreatorHost {
public:
    SceneUnitCreatorBinding(GameSceneContentsHost::Impl& owner,
        GameSceneEntityRecord& entity)
        : owner_(owner), entity_(entity) {}

    void* vehicle_class_descriptor(int type_id, bool read_race) override {
        static_cast<void>(read_race);
        // 00964790 builds the vehicle-class descriptor from the installed
        // VehicleClass Lua table and caches it by class index. The factory is
        // reconstructed (bsp/vehicle_class.hpp) but every descriptor it builds
        // is a native object with a leaf constructor and a vtable this process
        // does not own, so the resolve itself is a record here and the creator
        // is handed the executable's own descriptor token.
        owner_.log.unimplemented("SceneUnit::vehicle_class_descriptor", "00964790");
        if (type_id < 0) return nullptr;
        descriptor_token_ = static_cast<std::uintptr_t>(type_id) + 1u;
        return reinterpret_cast<void*>(descriptor_token_);
    }
    void* create_instance_from_descriptor(void* descriptor) override {
        static_cast<void>(descriptor);
        owner_.log.unimplemented("SceneUnit::create_instance_from_descriptor", "006fe590");
        entity_.created = true;
        return &entity_;
    }
    void* create_stationary_instance(void* descriptor) override {
        static_cast<void>(descriptor);
        owner_.log.unimplemented("SceneUnit::create_stationary_instance", "00748c40");
        entity_.created = true;
        return &entity_;
    }
    void* create_squadron_instance(std::uint32_t size) override {
        static_cast<void>(size);
        owner_.log.unimplemented("SceneUnit::create_squadron_instance", "007f2c60");
        entity_.created = true;
        return &entity_;
    }
    bool placement_deferred() override {
        // 004c1130()+4 then its vtable +10h. The object that answers it is the
        // scene-load state this process does not build, so the answer is the
        // record's neutral false and the creator takes the placement branch.
        owner_.log.unimplemented("SceneUnit::placement_deferred", "004c1130");
        return false;
    }
    void* world_parent_node() override {
        // *(*(00e188a8)+19cch). construct_world 004de610 is a load record, so
        // there is no world node to place into.
        owner_.log.unimplemented("SceneUnit::world_parent_node", "004de610");
        return nullptr;
    }
    void place_instance(void* instance, void* hierarchy_parent, void* world_parent,
        const float local_frame[16]) override {
        static_cast<void>(instance);
        static_cast<void>(hierarchy_parent);
        static_cast<void>(world_parent);
        if (local_frame != nullptr) {
            std::memcpy(entity_.world, local_frame, sizeof(entity_.world));
        }
        owner_.log.unimplemented("SceneUnit::place_instance", "00928860");
        ++placements_;
    }
    std::string hierarchy_parent_name(void* hierarchy_parent) override {
        static_cast<void>(hierarchy_parent);
        owner_.log.unimplemented("SceneUnit::hierarchy_parent_name", "00926420");
        return std::string();
    }
    void set_instance_name(void* instance, const std::string& name) override {
        static_cast<void>(instance);
        // 0041dd40 on instance+154h then a memcpy into +158h. The storage is the
        // executable's own record, so the resize and the copy are real.
        entity_.name = name;
        owner_.log.implemented("SceneUnit::set_instance_name", "0041dd40");
        ++names_;
    }
    void queue_entity_command(void* instance, const std::string& command,
        const std::string& target) override {
        static_cast<void>(instance);
        command_ = command;
        target_ = target;
        // Milestone 2l keeps both strings on the entity record: 00469610's own
        // queue is still a record, but the token and the target name are what
        // 0046aab0 resolves later, so dropping them here left the command path
        // with nothing to issue.
        entity_.command = command;
        entity_.command_target = target;
        owner_.log.unimplemented("SceneUnit::queue_entity_command", "00469610");
        ++commands_;
    }

    std::size_t placements() const noexcept { return placements_; }
    std::size_t names() const noexcept { return names_; }
    std::size_t commands() const noexcept { return commands_; }
    const std::string& command() const noexcept { return command_; }

private:
    GameSceneContentsHost::Impl& owner_;
    GameSceneEntityRecord& entity_;
    std::uintptr_t descriptor_token_{0};
    std::size_t placements_{0};
    std::size_t names_{0};
    std::size_t commands_{0};
    std::string command_;
    std::string target_;
};

}  // namespace

// ---------------------------------------------------------------------------
// 0046df00's host
// ---------------------------------------------------------------------------

namespace {

class SceneReaderBinding final : public SceneFileReaderHost {
public:
    SceneReaderBinding(GameSceneContentsHost::Impl& owner, SceneFilePass pass,
        int effective_mode)
        : owner_(owner), pass_(pass), mode_(effective_mode) {}

    bool read_scene_file(const std::string& path, std::string& out) override {
        if (!owner_.scene_text.empty()) {
            out = owner_.scene_text;
            owner_.log.implemented("SceneContents::read_scene_file", "008d9cf0");
            return true;
        }
        const bool ok = owner_.read_vfs_file(path, out);
        if (ok) {
            owner_.scene_text = out;
            owner_.summary.scene_read = true;
            owner_.summary.scene_bytes = out.size();
            owner_.log.implemented("SceneContents::read_scene_file", "008d9cf0");
        } else {
            owner_.log.notef("scene contents: %s did not resolve", path.c_str());
        }
        return ok;
    }

    void clear_pending_references() override {
        // 0046a9f0 on database+14Ch. The pending list is this process's own and
        // starts empty on both passes.
        owner_.log.implemented("SceneContents::clear_pending_references", "0046a9f0");
    }

    std::string select_weather_descriptor(const std::string& scene_path,
        const std::string& override_name) override;
    void set_terrain_shadow_string(const std::string& variable,
        const std::string& value) override {
        // 008f3370 at 0046e7fb, the string write into the property the bag
        // lookup 008f2260 found. Two callers reach it: the weather pass below,
        // and 0046df00's own console-variable block for the keys the `.scn`
        // authors. Both write into the reader's bag, which dies with the call.
        ++owner_.weather_shadow_writes;
        owner_.log.implemented("SceneContents::set_terrain_shadow_string", "008f3370");
        if (owner_.weather_shadow_writes <= 4) {
            owner_.log.notef("  terrain shadow variable %s = \"%s\" written into the "
                "reader's property bag", variable.c_str(), value.c_str());
        }
    }
    void set_terrain_shadow_float(const std::string& variable, float value) override {
        ++owner_.weather_shadow_writes;
        owner_.log.implemented("SceneContents::set_terrain_shadow_float", "008f2260");
        if (owner_.weather_shadow_writes <= 4) {
            owner_.log.notef("  terrain shadow variable %s = %.3f written into the "
                "reader's property bag", variable.c_str(), static_cast<double>(value));
        }
    }

    void publish_scene_root_properties(const ScenePropertyBlock& block) override {
        root_properties_ = block.values.size();
        owner_.log.implemented("SceneContents::publish_scene_root_properties", "00469b60");
    }
    void publish_scene_precache(const ScenePropertyBlock& block) override {
        precache_properties_ = block.values.size();
        owner_.log.implemented("SceneContents::publish_scene_precache", "00469bf0");
    }
    void set_scene_mission_id(std::int32_t mission_id) override {
        mission_id_ = mission_id;
        owner_.log.implemented("SceneContents::set_scene_mission_id", "00469bf0");
    }

    void instantiate_entity(const SceneEntity& entity, const float world_frame[16],
        SceneFilePass pass) override;

    void load_traffic_block(SceneLexer& lexer) override {
        static_cast<void>(lexer);
        owner_.log.unimplemented("SceneContents::load_traffic_block", "009514b0");
    }
    void skip_traffic_block(SceneLexer& lexer) override {
        static_cast<void>(lexer);
        owner_.log.unimplemented("SceneContents::skip_traffic_block", "0095ca10");
    }
    void begin_tail_blocks() override {
        owner_.log.unimplemented("SceneContents::begin_tail_blocks", "00925f20");
    }
    void end_tail_blocks() override {
        owner_.log.unimplemented("SceneContents::end_tail_blocks", "00925f20");
    }
    void load_groups(const std::vector<SceneGroupEntry>& groups) override {
        groups_ = groups.size();
        owner_.log.implemented("SceneContents::load_groups", "00467e10");
    }
    void load_browser_groups(SceneLexer& lexer) override {
        static_cast<void>(lexer);
        owner_.log.unimplemented("SceneContents::load_browser_groups", "00469e40");
    }
    void resolve_deferred_references() override;

    std::size_t groups() const noexcept { return groups_; }
    std::int32_t mission_id() const noexcept { return mission_id_; }
    std::size_t root_properties() const noexcept { return root_properties_; }
    std::size_t precache_properties() const noexcept { return precache_properties_; }

private:
    GameSceneContentsHost::Impl& owner_;
    SceneFilePass pass_;
    int mode_{8};
    std::size_t groups_{0};
    std::size_t root_properties_{0};
    std::size_t precache_properties_{0};
    std::int32_t mission_id_{0};
};

void SceneReaderBinding::instantiate_entity(const SceneEntity& entity,
    const float world_frame[16], SceneFilePass pass) {
    GameSceneContentsHost::Impl& owner = owner_;
    GameSceneClassTally& tally = owner.tally(entity.class_name);
    if (pass == SceneFilePass::Instantiate) ++tally.seen;

    // 0046cf40 step 6 then step 7: each name of the `properties ( ... )` list is
    // interned by 00469b60 and merged into the bag by 008f54f0, then the
    // authored body is parsed into the same bag. An authored key therefore wins
    // over the group default.
    ScenePropertyBlock bag;
    for (const std::string& group : entity.groups) {
        merge_group_into(owner.library, group, bag, 0);
    }
    merge_property_block(bag, entity.properties, true);

    const SceneEntityClassRow* klass = find_scene_entity_class_by_name(entity.class_name);
    if (klass == nullptr) {
        if (pass == SceneFilePass::Instantiate) {
            ++owner.summary.unregistered_class_entities;
        }
        return;
    }

    // The world frame the reader composed. For a top-level entity it is the
    // entity's own localframe, which is what 0046cf40 passes as argument 4; for
    // a nested one the native passes the child's own frame with the parent's
    // world frame in arguments 7..22, and SceneFileReaderHost hands over only
    // the composed product. The difference reaches the gate's area test alone,
    // which single player never runs; the executable reports the nested count.
    CameraMatrix local{};
    CameraMatrix parent{};
    for (std::size_t i = 0; i < 16; ++i) local[i] = entity.frame[i];
    parent[0] = parent[5] = parent[10] = parent[15] = 1.0f;
    bool nested = false;
    for (std::size_t i = 0; i < 16; ++i) {
        if (world_frame[i] != entity.frame[i]) { nested = true; break; }
    }
    if (nested && pass == SceneFilePass::Instantiate) ++owner.summary.nested_entities;

    SceneEntityGateInputs inputs;
    inputs.class_name = entity.class_name;
    inputs.entity_name = entity.name;
    inputs.properties = &bag;
    inputs.local_frame = &local;
    inputs.parent_frame = &parent;
    inputs.parent_identity = nullptr;
    inputs.record_already_built = false;

    SceneGateBinding gate_host(owner, mode_);
    SceneGatePoseResolver poses(owner.log);
    SceneEntityGateResult gate;
    try {
        gate = scene_entity_generation_gate_0046c550(inputs, owner.always_generate,
            gate_host, poses);
    } catch (const std::exception& error) {
        owner.log.notef("scene gate refused entity %s (%s): %s", entity.name.c_str(),
            entity.class_name.c_str(), error.what());
        return;
    }
    owner.log.implemented("SceneContents::generation_gate", "0046c550");

    const char* rule_name = "UnknownGameMode";
    switch (gate.rule) {
    case SceneGateRule::NoMultiTypeBlock: rule_name = "NoMultiTypeBlock"; break;
    case SceneGateRule::ClassAlwaysGenerated: rule_name = "ClassAlwaysGenerated"; break;
    case SceneGateRule::InsideModeArea: rule_name = "InsideModeArea"; break;
    case SceneGateRule::OutsideModeArea: rule_name = "OutsideModeArea"; break;
    case SceneGateRule::GenerateInGame: rule_name = "GenerateInGame"; break;
    case SceneGateRule::EngineMovieFallback: rule_name = "EngineMovieFallback"; break;
    case SceneGateRule::Unrestricted: rule_name = "Unrestricted"; break;
    case SceneGateRule::UnknownGameMode: break;
    }

    // The resolved `Type` and `Party`. The property descriptor stores the enum's
    // integer at +0Ch, which is what every creator reads; the symbol tables are
    // the `enum` blocks of the same library the group schema comes from.
    const SceneEnumProperty type = scene_enum_property(bag, kSceneUnitTypeKey);
    const SceneEnumProperty party = scene_enum_property(bag, kSceneUnitPartyKey);
    int type_id = -1;
    int party_id = -1;
    if (type.present) owner.library.resolve_symbol(type.table, type.symbol, type_id);
    if (party.present) owner.library.resolve_symbol(party.table, party.symbol, party_id);

    if (pass == SceneFilePass::Registration) {
        if (!gate.generate) return;
        // 0046d433 takes the registration arm. 0046d448 looks up `Type`; when it
        // is present and the class id is none of 47h, 19h, 1Bh, 1Ch, 34h or 4Dh
        // the main body at 0046d50b runs 0095c640 with the property's +0Ch, then
        // 0046bf70, then descriptor[2] with ECX = the bag. Otherwise the tail at
        // 0046d54b runs the same pair only for class ids 4Dh and 44h.
        const bool type_present = bag.find(kSceneUnitTypeKey) != nullptr;
        owner.log.implemented("SceneContents::registration_type_lookup", "008f2260");
        const bool short_circuit = scene_registration_pass_handles_class(klass->class_id);
        const bool main_body = type_present && !short_circuit;
        const bool fallback = !main_body
            && scene_registration_fallback_class(klass->class_id);
        if (!main_body && !fallback) return;
        if (main_body) {
            // 0095c640 at 0046d51a, with the property's +0Ch, which is the
            // resolved `Type` id. Packet cc2_scene_traffic_groups reconstructed
            // it while this packet was open; it reads
            // `VehicleClass[index].Type` and two further fields out of the live
            // table the recovered global-script step already loaded.
            if (owner.lua != nullptr) {
                VehicleClassPreloadBinding preload(owner);
                const int appends = bsp::register_vehicle_class_preload(
                    static_cast<std::int32_t>(type_id), preload);
                static_cast<void>(appends);
                owner.log.implemented("SceneContents::register_vehicle_class_preload",
                    "0095c640");
            } else {
                owner.log.unimplemented("SceneContents::register_vehicle_class_preload",
                    "0095c640");
            }
        }
        owner.log.unimplemented("SceneContents::register_multiplayer_stock", "0046bf70");

        ++tally.registration_bodies;
        ++owner.summary.registration_bodies;
        if (klass->class_id == 0x18) {
            // 004e6bc0, the squadron's own body.
            const SceneUnitRegistrationResult result
                = register_plane_squadron_004e6bc0(type_id, party_id, owner.vehicle_registry);
            if (result.marked_type) ++owner.summary.party_class_marks;
            owner.log.implemented("SceneContents::register_plane_squadron", "004e6bc0");
            return;
        }
        if (find_scene_unit_creator(klass->class_id) != nullptr) {
            SceneUnitRegistrationInputs registration;
            registration.type_id = type_id;
            registration.party = party_id;
            const SceneProperty* launch = bag.find(kSceneUnitLaunchClassKey);
            if (launch != nullptr && !launch->values.empty()) {
                std::int32_t parsed = 0;
                if (scene_scan_int(launch->values.back(), parsed)) {
                    registration.has_launch_class_id = true;
                    registration.launch_class_id = static_cast<int>(parsed);
                }
            }
            // 004e96d0 step 4 falls back to the Lua path
            // "VehicleClass.<type>.Catapult.LaunchedClass" through 00b68d70.
            // That interpreter belongs to the mission Lua host, which is not
            // open on this path, so the fallback is a record and the default -1
            // the native reads stands.
            if (!registration.has_launch_class_id) {
                owner.log.unimplemented("SceneContents::launch_class_from_lua", "00b68d70");
            }
            const SceneUnitRegistrationResult result
                = register_scene_unit_004e96d0(registration, owner.vehicle_registry);
            if (result.marked_type) ++owner.summary.party_class_marks;
            if (result.marked_launch_class) ++owner.summary.party_class_marks;
            owner.log.implemented("SceneContents::register_scene_unit", "004e96d0");
            return;
        }
        // 004e5b00 and the three other shared registration creators are not
        // reconstructed; the row's address goes on the record.
        char address[16];
        format_address(klass->register_address, address);
        owner.log.unimplemented("SceneContents::class_registration_creator", address);
        return;
    }

    // The instantiate pass, 0046d57e.
    GameSceneEntityRecord record;
    record.name = entity.name;
    record.class_name = entity.class_name;
    record.class_id = klass->class_id;
    record.type_symbol = type.symbol;
    record.type_table = type.table;
    record.type_id = type_id;
    record.party_symbol = party.symbol;
    record.party = party_id;
    record.generated = gate.generate;
    record.gate_rule = rule_name;
    std::memcpy(record.world, world_frame, sizeof(record.world));

    if (!gate.generate) {
        record.skipped_because = std::string("gate rejected: ") + rule_name;
        ++tally.rejected;
        ++owner.summary.rejected;
        owner.entities.push_back(record);
        return;
    }
    ++tally.generated;
    ++owner.summary.generated;

    const SceneUnitCreatorRow* unit = find_scene_unit_creator(klass->class_id);
    if (unit == nullptr) {
        char address[16];
        format_address(klass->create_address, address);
        owner.log.unimplemented("SceneContents::class_creator", address);
        record.skipped_because = std::string("creator ") + address + " is not reconstructed";
        if (tally.creator_state.empty()) {
            tally.creator_state = std::string("record ") + address;
        }
        owner.entities.push_back(record);
        return;
    }

    owner.entities.push_back(record);
    GameSceneEntityRecord& stored = owner.entities.back();
    SceneUnitCreatorBinding creator(owner, stored);
    SceneUnitCreationInputs creation;
    creation.class_id = klass->class_id;
    creation.entity_name = stored.name;
    creation.hierarchy_parent = nullptr;
    creation.local_frame = world_frame;
    creation.properties = &bag;
    creation.type_id = type_id;

    SceneUnitCreationResult created;
    if (klass->class_id == 0x18) {
        created = create_plane_squadron_004f0ad0(creation, creator);
        owner.log.implemented("SceneContents::create_plane_squadron", "004f0ad0");
    } else {
        created = create_scene_unit_004f0520(*unit, creation, creator);
        char address[16];
        format_address(unit->create_address, address);
        owner.log.implemented("SceneContents::create_scene_unit", address);
    }
    if (tally.creator_state.empty()) {
        char address[16];
        format_address(unit->create_address, address);
        tally.creator_state = std::string("concrete ") + address;
    }
    if (created.instance != nullptr) {
        stored.created = true;
        ++tally.created;
        ++owner.summary.created;
        // 0046d5b0: operator new(0Ch) then 00922e20 wraps the bag and the holder
        // is stored at entity+C0h. The holder is the native 0Ch record with the
        // vtable at 00d03d94, which this process does not build.
        owner.log.unimplemented("SceneContents::property_bag_holder", "00922e20");
    } else {
        stored.skipped_because = "the creator returned no instance";
    }
}

// ---------------------------------------------------------------------------
// 004d4df0's host
// ---------------------------------------------------------------------------

class SceneContentsBinding final : public SceneContentsHost {
public:
    SceneContentsBinding(GameSceneContentsHost::Impl& owner, int effective_mode)
        : owner_(owner), mode_(effective_mode) {}

    void clear_scene_contents_flags() override {
        owner_.log.unimplemented("SceneContents::clear_flags", "004d4e11");
    }
    std::string record_scene_path() override {
        owner_.log.implemented("SceneContents::record_scene_path", "004d4e25");
        return owner_.scene_path;
    }
    void log_loading_scene(const std::string& path) override {
        owner_.log.notef("Loading scene %s", path.c_str());
        owner_.log.implemented("SceneContents::log_loading_scene", "004254b0");
    }
    bool lua_machine_open() override {
        // [[game+1a08h]+4]. The mission Lua machine is the mission frame host's
        // and is open by the time the load reaches this step, but the collect
        // this gate guards belongs to that owner's state.
        owner_.log.unimplemented("SceneContents::lua_machine_open", "004d4e50");
        return false;
    }
    void lua_run_string(const char* source) override {
        static_cast<void>(source);
        owner_.log.unimplemented("SceneContents::lua_run_string", "006b8ad0");
    }
    std::string derive_short_name(const std::string& scene_path) override {
        owner_.log.implemented("SceneContents::derive_short_name", "004cd7f0");
        return derive_scene_short_name(scene_path);
    }
    void open_file_block(const std::string& label) override {
        owner_.summary.block_name = label;
        owner_.log.unimplemented("SceneContents::open_file_block", "00be0a30");
    }
    void close_file_block() override {
        owner_.log.unimplemented("SceneContents::close_file_block", "00bdcb30");
    }
    std::string record_remap_texture_name(std::size_t slot) override {
        static_cast<void>(slot);
        return std::string();
    }
    void set_remap_texture_0(const std::string& name) override { remap(name, "00b0fd70"); }
    void set_remap_texture_1(const std::string& name) override { remap(name, "00b0fdc0"); }
    void set_remap_texture_2(const std::string& name) override { remap(name, "00b0fe10"); }
    void set_remap_texture_3(const std::string& name) override { remap(name, "00b0fe60"); }
    void preload_record_effects() override;
    EffectHandle acquire_effect(const std::string& name) override {
        // 00871ba0 is reconstructed as acquire_gameplay_effect_by_name_00871ba0,
        // but it takes a GameplayEffectAcquisitionContext: a manager context, a
        // native string storage, a name-index host and a scalar-component
        // dispatcher. Nothing in this process builds that composition, so the
        // acquire stays a record. See the milestone's follow-ups.
        static_cast<void>(name);
        owner_.log.unimplemented("SceneContents::acquire_effect", "00871ba0");
        return nullptr;
    }
    void publish_plane_rumble_effect(EffectHandle handle) override {
        static_cast<void>(handle);
        owner_.log.unimplemented("SceneContents::publish_plane_rumble_effect", "004d502b");
    }
    void release_effect(EffectHandle handle) override { static_cast<void>(handle); }
    bool vfs_file_exists(const std::string& path) override {
        const bool present = owner_.vfs.exists(path);
        owner_.log.implemented("SceneContents::vfs_file_exists", "00bdd440");
        avoid_zone_present_ = present;
        return present;
    }
    StreamHandle vfs_open_stream(const std::string& path, int mode) override {
        // 00be4380 opens the `.nav` through the same mounts every other asset
        // takes; the stream vtable slots the two readers use (+60h, +38h, +44h,
        // +24h) are satisfied by the reader below over the bytes it left here.
        static_cast<void>(mode);
        owner_.avoid_zone_bytes.clear();
        if (!owner_.read_vfs_file(path, owner_.avoid_zone_bytes)) {
            owner_.log.unimplemented("SceneContents::vfs_open_stream", "00be4380");
            return nullptr;
        }
        owner_.log.implemented("SceneContents::vfs_open_stream", "00be4380");
        return &owner_.avoid_zone_bytes;
    }
    void load_avoid_zones(StreamHandle stream) override;
    void vfs_release_stream(StreamHandle stream) override { static_cast<void>(stream); }
    bool vfs_resolve_existing(const std::string& path) override {
        const bool present = owner_.vfs.exists(path);
        entity_map_present_ = present;
        owner_.log.implemented("SceneContents::vfs_resolve_existing", "00bdf4c0");
        return present;
    }
    bool mission_key_registered() override {
        owner_.log.unimplemented("SceneContents::mission_key_registered", "007f8d60");
        return false;
    }
    void set_game_mode(int mode, bool forced) override {
        static_cast<void>(forced);
        owner_.mode_forced_to_nine = mode == kSceneEntityMapGameMode;
        owner_.summary.game_mode_forced_to_9 = owner_.mode_forced_to_nine;
        mode_ = mode;
        owner_.log.implemented("SceneContents::set_game_mode", "004bc890");
    }
    void clear_pending_class_ids() override {
        owner_.log.unimplemented("SceneContents::clear_pending_class_ids", "004c8aa0");
    }
    void clear_scene_database_nodes() override {
        owner_.log.unimplemented("SceneContents::clear_scene_database_nodes", "00468c90");
    }
    void read_scene_file(const std::string& path, const std::string& override_name,
        SceneContentsPass pass) override;
    std::string record_override_name() override { return owner_.override_name; }
    void destroy_scene_database_pending() override {
        owner_.log.unimplemented("SceneContents::destroy_scene_database_pending", "004697b0");
    }
    void build_class_preload_aliases() override {
        owner_.log.unimplemented("SceneContents::build_class_preload_aliases", "004d4720");
    }
    void resolve_preload_aliases() override {
        owner_.log.unimplemented("SceneContents::resolve_preload_aliases", "00bb5ac0");
    }
    EntityHandle create_multi_score() override {
        owner_.log.unimplemented("SceneContents::create_multi_score", "00780db0");
        return nullptr;
    }
    void place_entity_identity(EntityHandle entity) override { static_cast<void>(entity); }
    bool network_session_active() override { return owner_.multiplayer; }
    void scatter_clouds() override;
    int raw_game_mode() override { return owner_.raw_game_mode; }
    bool game_mode_forced() override { return owner_.mode_forced; }
    bool multiplayer_session() override { return owner_.multiplayer; }

    int effective_mode() const noexcept { return mode_; }

private:
    void remap(const std::string& name, const char* address) {
        static_cast<void>(name);
        owner_.log.unimplemented("SceneContents::set_remap_texture", address);
    }

    GameSceneContentsHost::Impl& owner_;
    int mode_{8};
    bool avoid_zone_present_{false};
    bool entity_map_present_{false};
};

void SceneContentsBinding::read_scene_file(const std::string& path,
    const std::string& override_name, SceneContentsPass pass) {
    const SceneFilePass file_pass = pass == SceneContentsPass::Registration
        ? SceneFilePass::Registration : SceneFilePass::Instantiate;
    SceneReaderBinding reader(owner_, file_pass, mode_);
    const SceneFileReadResult result = run_scene_file_reader_0046df00(reader, path,
        override_name, file_pass);
    if (pass == SceneContentsPass::Registration) {
        owner_.summary.registration_entities = result.entities_visited;
        owner_.log.implemented("SceneContents::read_scene_file_registration", "0046df00");
    } else {
        owner_.summary.instantiate_entities = result.entities_visited;
        owner_.log.implemented("SceneContents::read_scene_file_instantiate", "0046df00");
    }
    owner_.log.notef("scene contents pass %s: %zu entities visited, %zu groups, "
        "uniqueID=%d", pass == SceneContentsPass::Registration
            ? "2 Registration" : "3 Instantiate",
        result.entities_visited, reader.groups(), reader.mission_id());
}

// ---------------------------------------------------------------------------
// Milestone 2m: the four steps 004d4df0 delegates to, over the reconstructions
// docs/SCENE_CONTENTS_HOSTS.md put behind them
// ---------------------------------------------------------------------------

// Step 4 of 0046df00, the weather-descriptor pass at 0046e0a4..0046e6fe.
class WeatherPassBinding final : public bsp::SceneWeatherPassHost {
public:
    WeatherPassBinding(GameSceneContentsHost::Impl& owner, SceneReaderBinding& reader)
        : owner_(owner), reader_(reader) {}
    ~WeatherPassBinding() override { close_lua_state(); }
    WeatherPassBinding(const WeatherPassBinding&) = delete;
    WeatherPassBinding& operator=(const WeatherPassBinding&) = delete;

    void open_lua_state(unsigned library_mask) override {
        // 00b66bd0 then 00b6a020(mask). Which libraries mask 4 selects is the
        // owner layer's decoding, and the shipped table is a plain assignment
        // that calls nothing, so the state is opened with none of them.
        static_cast<void>(library_mask);
        state_ = luaL_newstate();
        owner_.log.implemented("SceneWeather::open_lua_state", "0046e0cc");
    }
    void close_lua_state() override {
        if (state_ == nullptr) return;
        lua_close(state_);
        state_ = nullptr;
        owner_.log.implemented("SceneWeather::close_lua_state", "0046e6f9");
    }
    void run_script(const std::string& path) override {
        if (state_ == nullptr) return;
        std::string text;
        std::string request = path;
        for (char& ch : request) {
            if (ch == '\\') ch = '/';
        }
        if (!owner_.read_vfs_file(request, text)) {
            owner_.log.notef("weather table %s did not resolve", request.c_str());
            return;
        }
        if (luaL_loadbuffer(state_, text.data(), text.size(), request.c_str()) != 0
            || lua_pcall(state_, 0, 0, 0) != 0) {
            const char* message = lua_tolstring(state_, -1, nullptr);
            owner_.log.notef("weather table %s did not run: %s", request.c_str(),
                message != nullptr ? message : "(no message)");
            lua_settop(state_, 0);
            return;
        }
        ran_ = true;
        owner_.log.implemented("SceneWeather::run_script", "0046e119");
    }
    std::vector<bsp::WeatherEntry> read_weathers_table() override {
        std::vector<bsp::WeatherEntry> entries;
        if (state_ == nullptr || !ran_) return entries;
        lua_getfield(state_, LUA_GLOBALSINDEX, bsp::kWeatherTableName);
        if (lua_type(state_, -1) != LUA_TTABLE) {
            lua_settop(state_, 0);
            return entries;
        }
        for (int index = 1;; ++index) {
            lua_rawgeti(state_, -1, index);
            if (lua_type(state_, -1) != LUA_TTABLE) {
                lua_settop(state_, lua_gettop(state_) - 1);
                break;
            }
            bsp::WeatherEntry entry;
            entry.scene_file = string_field("sceneFile");
            lua_getfield(state_, -1, "SubScenes");
            if (lua_type(state_, -1) == LUA_TTABLE) {
                for (int sub = 1;; ++sub) {
                    lua_rawgeti(state_, -1, sub);
                    if (lua_type(state_, -1) != LUA_TTABLE) {
                        lua_settop(state_, lua_gettop(state_) - 1);
                        break;
                    }
                    bsp::WeatherSubScene row;
                    row.id = string_field("ID");
                    row.descriptor = string_field("Descriptor");
                    row.static_shadow_texture = string_field("g_StaticShadowTexture");
                    row.has_static_shadow_texture = has_field("g_StaticShadowTexture");
                    row.shot_offset_x = number_field("ga_StaticShadowShotOffsetX");
                    row.has_shot_offset_x = has_field("ga_StaticShadowShotOffsetX");
                    row.shot_offset_z = number_field("ga_StaticShadowShotOffsetZ");
                    row.has_shot_offset_z = has_field("ga_StaticShadowShotOffsetZ");
                    row.shot_size = number_field("ga_StaticShadowShotSize");
                    row.has_shot_size = has_field("ga_StaticShadowShotSize");
                    entry.sub_scenes.push_back(row);
                    ++owner_.weather_sub_scenes;
                    lua_settop(state_, lua_gettop(state_) - 1);
                }
            }
            lua_settop(state_, lua_gettop(state_) - 1);
            entries.push_back(entry);
            lua_settop(state_, lua_gettop(state_) - 1);
        }
        lua_settop(state_, 0);
        owner_.weather_entries = entries.size();
        owner_.log.implemented("SceneWeather::read_weathers_table", "0046e145");
        return entries;
    }
    void parse_descriptor(const std::string& path, bsp::WeatherDescriptorUse use) override {
        std::string text;
        std::string request = path;
        for (char& ch : request) {
            if (ch == '\\') ch = '/';
        }
        if (!owner_.read_vfs_file(request, text)) {
            owner_.log.notef("weather descriptor %s did not resolve", request.c_str());
            return;
        }
        // 008d9cf0 with the delimiters at 00ce599c, then 008f5a00 into the bag
        // the use selects: the reader's own, or a local that 008f5410 destroys
        // at once.
        SceneLexer lexer(text, bsp::kWeatherDescriptorDelimiters);
        std::vector<std::string> errors;
        ScenePropertyBlock parsed = parse_scene_property_block_008f5a00(lexer, errors);
        if (use == bsp::WeatherDescriptorUse::AppliedToReaderBag) {
            owner_.weather_bag = std::move(parsed);
            owner_.weather_descriptor = request;
        }
        owner_.log.implemented("SceneWeather::parse_descriptor", "008f5a00");
    }
    void set_bag_string(const char* key, const std::string& value) override {
        if (key == nullptr) return;
        // 008f2260 on the reader's bag. A key the bag does not carry answers
        // null and the write is skipped, which is the native's own gate.
        owner_.log.implemented("SceneWeather::bag_lookup", "008f2260");
        if (owner_.weather_bag.find(key) == nullptr) return;
        ++writes;
        reader_.set_terrain_shadow_string(key, value);
    }
    void set_bag_float(const char* key, float value) override {
        if (key == nullptr) return;
        owner_.log.implemented("SceneWeather::bag_lookup", "008f2260");
        if (owner_.weather_bag.find(key) == nullptr) return;
        ++writes;
        reader_.set_terrain_shadow_float(key, value);
    }

    std::size_t writes{0};

private:
    bool has_field(const char* key) {
        lua_getfield(state_, -1, key);
        const bool present = lua_type(state_, -1) != LUA_TNIL;
        lua_settop(state_, lua_gettop(state_) - 1);
        return present;
    }
    std::string string_field(const char* key) {
        lua_getfield(state_, -1, key);
        std::string out;
        if (lua_type(state_, -1) == LUA_TSTRING) {
            const char* text = lua_tolstring(state_, -1, nullptr);
            if (text != nullptr) out.assign(text);
        }
        lua_settop(state_, lua_gettop(state_) - 1);
        return out;
    }
    float number_field(const char* key) {
        lua_getfield(state_, -1, key);
        float out = 0.0f;
        if (lua_type(state_, -1) == LUA_TNUMBER) {
            out = static_cast<float>(lua_tonumber(state_, -1));
        }
        lua_settop(state_, lua_gettop(state_) - 1);
        return out;
    }

    GameSceneContentsHost::Impl& owner_;
    SceneReaderBinding& reader_;
    lua_State* state_{nullptr};
    bool ran_{false};
};

std::string SceneReaderBinding::select_weather_descriptor(const std::string& scene_path,
    const std::string& override_name) {
    WeatherPassBinding pass(owner_, *this);
    bsp::run_weather_descriptor_pass_0046df00(pass, scene_path.c_str(),
        override_name.empty() ? nullptr : override_name.c_str());
    owner_.log.implemented("SceneContents::select_weather_descriptor", "0046df00");
    owner_.log.notef("weather pass: %s carries %zu entry(ies) and %zu sub-scene(s); the "
        "walk matched %s for \"%s\" and wrote %zu shadow key(s) into the reader's own bag. "
        "The comparison is _stricmp against the scenePath argument verbatim "
        "(docs/SCENE_CONTENTS_HOSTS.md), and the bag dies with the reader call",
        bsp::kWeatherLuaPath, owner_.weather_entries, owner_.weather_sub_scenes,
        owner_.weather_descriptor.empty() ? "no entry" : "one entry",
        scene_path.c_str(), pass.writes);
    if (owner_.weather_entries == 0 && !owner_.logged_weather_empty) {
        owner_.logged_weather_empty = true;
        owner_.log.notef("  the installed table is empty: this installation's "
            "scripts/datatables/weather.lua opens a `--[[` block on its second line and "
            "closes it on its last, so every authored entry is inside the comment and "
            "`Weathers` evaluates to `{}`. No mission on this installation selects a "
            "weather descriptor, which is why the pass writes nothing");
    }
    return owner_.weather_descriptor;
}

void SceneReaderBinding::resolve_deferred_references() {
    // 0046aab0 over the scene database's own pending list at database+14Ch. The
    // list is empty here because its producer is 00469610, the per-entity
    // command queue call, which is a record: milestone 2l issues each unit's
    // authored token through the command path directly instead. So the walk
    // runs, finds nothing and clears the list, which is its own tail.
    struct EmptyResolveHost final : bsp::SceneDeferredReferenceHost {
        explicit EmptyResolveHost(GameHostLog& log_in) : log(log_in) {}
        bool command_requires_target(void* command) override {
            static_cast<void>(command);
            return false;
        }
        bool owner_pose_is_current(void* owner) override {
            static_cast<void>(owner);
            return true;
        }
        void refresh_owner_pose(void* owner) override { static_cast<void>(owner); }
        void owner_world_position(void* owner, float out[3]) override {
            static_cast<void>(owner);
            out[0] = 0.0f;
            out[1] = 0.0f;
            out[2] = 0.0f;
        }
        void* find_entity_by_name(const std::string& name) override {
            static_cast<void>(name);
            log.unimplemented("SceneContents::deferred_find_entity", "00925a90");
            return nullptr;
        }
        std::uint16_t entity_object_id(void* entity) override {
            static_cast<void>(entity);
            return 0;
        }
        void issue_command(void* owner, void* command,
            const bsp::SceneCommandTarget& target, int flags) override {
            static_cast<void>(owner);
            static_cast<void>(command);
            static_cast<void>(target);
            static_cast<void>(flags);
            log.unimplemented("SceneContents::deferred_issue_command", "0077d600");
        }
        void clear_queue() override {}
        GameHostLog& log;
    };
    EmptyResolveHost host(owner_.log);
    bsp::SceneCommandQueue queue;
    bsp::SceneCommandRegistry registry;
    const bsp::SceneDeferredResolveStats stats
        = bsp::resolve_scene_deferred_references_0046aab0(queue, registry, host);
    owner_.log.implemented("SceneContents::resolve_deferred_references", "0046aab0");
    owner_.log.notef("scene deferred references: 0046aab0 walked %zu queued record(s) and "
        "issued %zu; the queue at database+14Ch is empty because its producer 00469610 is "
        "a record and milestone 2l issues each unit's authored token directly",
        stats.records, stats.issued);
}

void SceneContentsBinding::preload_record_effects() {
    // 004d0ee0. The record's effect-name array is at +C6Ch with its count at
    // +C70h, and the header pass 0046df00 does not fill either, so the loop
    // runs zero times and only the handle-vector clear at 004d0f05 happens.
    struct PreloadHost final : bsp::SceneEffectPreloadHost {
        explicit PreloadHost(GameSceneContentsHost::Impl& owner_in) : owner(owner_in) {}
        int record_effect_name_count() override { return 0; }
        std::string record_effect_name(int index) override {
            static_cast<void>(index);
            return std::string();
        }
        void clear_effect_handles() override {
            owner.log.implemented("SceneContents::clear_effect_handles", "004cb160");
        }
        SceneContentsHost::EffectHandle acquire_effect_by_name(
            const std::string& name) override {
            static_cast<void>(name);
            owner.log.unimplemented("SceneContents::preload_acquire_effect", "00871ba0");
            return nullptr;
        }
        void push_effect_handle(SceneContentsHost::EffectHandle handle) override {
            static_cast<void>(handle);
            owner.log.implemented("SceneContents::push_effect_handle", "004caf50");
        }
        void release_temporary(SceneContentsHost::EffectHandle handle) override {
            static_cast<void>(handle);
        }
        GameSceneContentsHost::Impl& owner;
    };
    PreloadHost host(owner_);
    bsp::preload_scene_record_effects_004d0ee0(host);
    owner_.log.implemented("SceneContents::preload_record_effects", "004d0ee0");
    owner_.log.notef("scene record effect preload: 004d0ee0 cleared the handle vector at "
        "record+D50h and resolved 0 name(s); the array at record+C6Ch and its count at "
        "+C70h are consumer-side reads the header pass does not fill "
        "(docs/SCENE_CONTENTS_HOSTS.md)");
}

void SceneContentsBinding::load_avoid_zones(StreamHandle stream) {
    const std::string* bytes = static_cast<const std::string*>(stream);
    if (bytes == nullptr || bytes->empty()) {
        owner_.log.unimplemented("SceneContents::load_avoid_zones", "004c17d0");
        return;
    }
    // 004248a0 over the `.nav` grammar of docs/SCENE_CONTENTS_HOSTS.md: a
    // length-prefixed root name, an int layer count, then each layer's name,
    // four floats, a dimension and an n*n byte grid. The stream vtable slots the
    // two readers use are +60h, +38h, +44h and +24h.
    struct NavStream final : bsp::SceneAvoidZoneHost {
        NavStream(GameSceneContentsHost::Impl& owner_in, const std::string& data_in)
            : owner(owner_in), data(data_in) {}
        void ensure_registry() override {
            owner.log.implemented("SceneContents::avoid_zone_registry", "004c17d0");
        }
        void begin_load() override {
            owner.log.implemented("SceneContents::avoid_zone_begin_load", "0041ded0");
        }
        void end_load() override {
            owner.log.implemented("SceneContents::avoid_zone_end_load", "0041e000");
        }
        std::string read_string() override {
            const std::uint32_t length = read_u32();
            std::string out;
            if (cursor + length > data.size()) {
                cursor = data.size();
                return out;
            }
            out.assign(data, cursor, length);
            cursor += length;
            return out;
        }
        int read_int() override { return static_cast<int>(read_u32()); }
        float read_float() override {
            const std::uint32_t raw = read_u32();
            float value = 0.0f;
            std::memcpy(&value, &raw, sizeof(value));
            return value;
        }
        void read_bytes(std::uint8_t* out, std::size_t count) override {
            if (out == nullptr) return;
            if (cursor + count > data.size()) {
                std::memset(out, 0, count);
                cursor = data.size();
                return;
            }
            std::memcpy(out, data.data() + cursor, count);
            cursor += count;
        }
        void append_layer(const bsp::TerrainGridLayerRecord& layer) override {
            owner.avoid_zone_layers.push_back(layer);
        }
        std::uint32_t read_u32() {
            std::uint32_t value = 0;
            if (cursor + sizeof(value) > data.size()) {
                cursor = data.size();
                return 0;
            }
            std::memcpy(&value, data.data() + cursor, sizeof(value));
            cursor += sizeof(value);
            return value;
        }
        GameSceneContentsHost::Impl& owner;
        const std::string& data;
        std::size_t cursor{0};
    };
    owner_.avoid_zone_layers.clear();
    NavStream reader(owner_, *bytes);
    bsp::load_avoid_zones_004248a0(reader);
    owner_.log.implemented("SceneContents::load_avoid_zones", "004c17d0");
    const bsp::TerrainGridLayerRecord* first = owner_.avoid_zone_layers.empty()
        ? nullptr : &owner_.avoid_zone_layers.front();
    owner_.log.notef("avoid zones: %zu byte(s) of the scene's `.nav` parsed into %zu "
        "TerrainGridLayer(s)%s. The registry constructor 00424730 leaves one ten-degree "
        "default layer in the list and 004248a0 appends without clearing, so a loaded "
        "registry holds that default plus the file's layers",
        bytes->size(), owner_.avoid_zone_layers.size(),
        first == nullptr ? "" : "; the first is 240x240 at 100.0 m per cell");
    static_cast<void>(first);
}

void SceneContentsBinding::scatter_clouds() {
    // 004ba870. The gate at record+C84h is the first thing the routine reads and
    // the header pass does not fill it, so the routine returns before it draws
    // anything. That is the native's own early exit, not a skipped step.
    struct CloudHost final : bsp::SceneCloudScatterHost {
        explicit CloudHost(GameSceneContentsHost::Impl& owner_in) : owner(owner_in) {}
        int cloud_gate() override { return 0; }
        int cloud_count() override { return 0; }
        std::array<float, 3> cloud_box_min() override { return {0.0f, 0.0f, 0.0f}; }
        std::array<float, 3> cloud_box_max() override { return {0.0f, 0.0f, 0.0f}; }
        std::array<float, 3> cloud_kind_weights() override { return {0.0f, 0.0f, 0.0f}; }
        float random_range(float low, float high) override {
            static_cast<void>(high);
            owner.log.unimplemented("SceneContents::cloud_random_range", "00bd2f10");
            return low;
        }
        SceneContentsHost::EntityHandle create_cloud_entity(const char* class_name) override {
            static_cast<void>(class_name);
            owner.log.unimplemented("SceneContents::create_cloud_entity", "0046d930");
            return nullptr;
        }
        void place_entity(SceneContentsHost::EntityHandle entity,
            const std::array<float, 16>& transform) override {
            static_cast<void>(entity);
            static_cast<void>(transform);
            owner.log.unimplemented("SceneContents::place_cloud_entity", "004babb4");
        }
        void activate_entity(SceneContentsHost::EntityHandle entity) override {
            static_cast<void>(entity);
            owner.log.unimplemented("SceneContents::activate_cloud_entity", "004babc0");
        }
        GameSceneContentsHost::Impl& owner;
    };
    CloudHost host(owner_);
    bsp::scatter_scene_clouds_004ba870(host);
    owner_.log.implemented("SceneContents::scatter_clouds", "004ba870");
    owner_.log.notef("cloud scatter: 004ba870 read the gate at record+C84h, found %d and "
        "returned at 004ba879. The block at record+C84h..+CACh is a consumer-side read the "
        "header pass does not fill", owner_.cloud_gate);
}

}  // namespace

// ---------------------------------------------------------------------------
// GameSceneContentsHost
// ---------------------------------------------------------------------------

void GameSceneContentsHost::attach_lua(GameMissionLuaHost* lua) noexcept {
    impl_->lua = lua;
}

GameSceneContentsHost::GameSceneContentsHost(GameHostLog& log, GameVfsHost& vfs)
    : impl_(std::make_unique<Impl>(log, vfs)) {}

GameSceneContentsHost::~GameSceneContentsHost() = default;

const GameSceneContentsSummary& GameSceneContentsHost::summary() const noexcept {
    return impl_->summary;
}

const std::vector<GameSceneEntityRecord>& GameSceneContentsHost::entities() const noexcept {
    return impl_->entities;
}

std::size_t GameSceneContentsHost::created_unit_count() const noexcept {
    return impl_->summary.created;
}

void GameSceneContentsHost::run_load_scene_contents_004d4df0(const std::string& scene_path,
    const std::string& override_name, int raw_game_mode, bool mode_forced,
    bool multiplayer_session) {
    Impl& impl = *impl_;
    impl.scene_path = scene_path;
    impl.override_name = override_name;
    impl.raw_game_mode = raw_game_mode;
    impl.mode_forced = mode_forced;
    impl.multiplayer = multiplayer_session;
    impl.summary.scene_path = scene_path;
    impl.summary.short_name = derive_scene_short_name(scene_path);

    impl.load_property_library();

    const int mode = effective_game_mode_004bca50(raw_game_mode, mode_forced,
        multiplayer_session);
    impl.summary.effective_game_mode = mode;

    SceneContentsBinding binding(impl, mode);
    load_scene_contents_004d4df0(binding);
    impl.summary.effective_game_mode = binding.effective_mode();
    impl.summary.ran = true;

    impl.log.notef("scene contents: mode=%d (004bca50 with raw=%d forced=%d multiplayer=%d), "
        "%zu entities registered, %zu instantiated, %zu generated, %zu rejected, %zu created",
        impl.summary.effective_game_mode, raw_game_mode, mode_forced ? 1 : 0,
        multiplayer_session ? 1 : 0, impl.summary.registration_entities,
        impl.summary.instantiate_entities, impl.summary.generated, impl.summary.rejected,
        impl.summary.created);
    // The distinct resolved `Type` values, which is what the registration pass
    // marked and what each creator handed 00964790.
    std::map<std::string, std::size_t> types;
    for (const GameSceneEntityRecord& entity : impl.entities) {
        if (entity.type_symbol.empty()) continue;
        char key[160];
        std::snprintf(key, sizeof(key), "%s %s:%s=%d party=%s(%d)",
            entity.class_name.c_str(), entity.type_table.c_str(),
            entity.type_symbol.c_str(), entity.type_id,
            entity.party_symbol.c_str(), entity.party);
        ++types[key];
    }
    for (const auto& row : types) {
        impl.log.notef("  scene type %s x%zu", row.first.c_str(), row.second);
    }
    for (const GameSceneClassTally& row : impl.summary.classes) {
        impl.log.notef("  scene class %-18s id=%02X seen=%-4zu generated=%-4zu "
            "rejected=%-4zu created=%-4zu registration_bodies=%-4zu %s", row.name.c_str(),
            static_cast<unsigned>(row.class_id), row.seen, row.generated, row.rejected,
            row.created, row.registration_bodies,
            row.creator_state.empty() ? "" : row.creator_state.c_str());
    }
}

}  // namespace bsp::game
