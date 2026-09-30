// bsp_game.exe milestone 2h: the scene contents pass.
// See include/bsp/game_hosts_scene_contents.hpp for the address list and the evidence.
#include "bsp/game_hosts_scene_contents.hpp"
#include "bsp/game_hosts_avoid_zones.hpp"

#include "bsp/air_operations.hpp"
#include "bsp/camera_affine.hpp"
#include "bsp/camera_inverse.hpp"
#include "bsp/main_menu_map_point_geometry.hpp"
#include "bsp/game_hosts.hpp"
#include "bsp/game_hosts_ai.hpp"
#include "bsp/game_hosts_lua.hpp"
#include "bsp/game_hosts_vfs.hpp"
#include "bsp/cruise_speed_setting.hpp"
#include "bsp/scene_traffic_groups.hpp"
#include "bsp/mission_scene_contents.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/native_camera_plane_transform.hpp"
#include "bsp/plane_squadron_host.hpp"
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
#include "bsp/gun_fire_points.hpp"
#include "bsp/memory_stream.hpp"
#include "bsp/structured_reader.hpp"

extern "C" {
#include "lauxlib.h"
#include "lua.h"
}

#include <algorithm>
#include <array>
#include <cctype>
#include <cmath>
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

// Packet cc9_land_convoy_members. 00743450's scene reads (00743497..007437B0):
// the eight scalars (an `F` value is read as a float, an `I` as its integer,
// 004F.. type word test), then the slot map: every slot -1, and for n = 1..4
// the block "Type<n>" (type tag 6; absent gives a null bag) and its enum `Type`,
// for m = 1..4 its integer "Position<m>": 0 fills every slot with the type, a
// positive value fills slot m-1 (the later write wins), a negative one nothing.
// The bag here is already merged with the library group, which is what makes
// the unauthored `Type1.Position1 = 0` of landconvoy.props apply.
void retain_land_convoy_roster(const ScenePropertyBlock& bag, GameSceneEntityRecord& record,
    const PropertyLibrary& library) {
    const auto scalar = [&](const char* key, float fallback) {
        const SceneProperty* prop = bag.find(key);
        if (prop == nullptr || prop->values.empty()) return fallback;
        float f = fallback;
        std::int32_t i = 0;
        if (prop->type_letter == "I" && scene_scan_int(prop->values.back(), i))
            return static_cast<float>(i);
        if (scene_scan_float(prop->values.back(), f)) return f;
        if (prop->type_letter == "B") return prop->values.back() == "true" ? 1.0f : 0.0f;
        return fallback;
    };
    const auto integer = [&](const ScenePropertyBlock& block, const char* key, std::int32_t fallback) {
        const SceneProperty* prop = block.find(key);
        std::int32_t i = fallback;
        if (prop != nullptr && !prop->values.empty()) scene_scan_int(prop->values.back(), i);
        return i;
    };
    record.land_convoy_keys = true;
    record.convoy_rows = integer(bag, "Rows", 0);
    record.convoy_columns = integer(bag, "Columns", 0);
    record.convoy_row_gap = scalar("RowGap", 0.0f);
    record.convoy_column_gap = scalar("ColumnGap", 0.0f);
    record.convoy_hp = scalar("HP", 0.0f);
    record.convoy_speed = scalar("Speed", 0.0f);
    record.convoy_offset = scalar("Offset", 0.0f);
    record.convoy_reverse = scalar("Reverse", 0.0f) != 0.0f;
    if (const SceneProperty* path = bag.find("Path"); path != nullptr && !path->values.empty()) {
        std::string value = path->values.back();
        if (value.size() >= 2 && value.front() == '"' && value.back() == '"')
            value = value.substr(1, value.size() - 2);
        record.convoy_path = value;
    }
    const int slots = record.convoy_rows * record.convoy_columns;
    record.convoy_slots.assign(slots > 0 ? static_cast<std::size_t>(slots) : 0u, -1);
    record.convoy_slot_symbols.assign(record.convoy_slots.size(), std::string());
    for (int n = 1; n <= 4; ++n) {
        char key[16];
        std::snprintf(key, sizeof(key), "Type%d", n);
        const ScenePropertyBlock* sub = nullptr;
        for (const auto& block : bag.blocks) {
            if (equal_insensitive(block.first, key)) { sub = &block.second; break; }
        }
        if (sub == nullptr) continue;
        int type = 0;
        std::string symbol;
        const SceneEnumProperty e = scene_enum_property(*sub, "Type");
        if (e.present) {
            symbol = e.symbol;
            if (!library.resolve_symbol(e.table, e.symbol, type)) type = 0;
        }
        for (int m = 1; m <= 4; ++m) {
            char position_key[16];
            std::snprintf(position_key, sizeof(position_key), "Position%d", m);
            const std::int32_t position = integer(*sub, position_key, -1);
            if (position == 0) {
                for (std::size_t s = 0; s < record.convoy_slots.size(); ++s) {
                    record.convoy_slots[s] = type;
                    record.convoy_slot_symbols[s] = symbol;
                }
            } else if (position > 0
                       && static_cast<std::size_t>(position) <= record.convoy_slots.size()) {
                record.convoy_slots[static_cast<std::size_t>(position - 1)] = type;
                record.convoy_slot_symbols[static_cast<std::size_t>(position - 1)] = symbol;
            }
        }
    }
}

// 007B352E..007B3604, only the fresh kind-1 PathPoints/Pos data projection.
// The native asks for Point%002i using its current point count (00415870),
// copies the three Pos lanes unchanged and appends once before the next lookup.
// Other source-holder kinds, Group, allocation, derived lengths and native
// entity construction are not implemented by this retention binding.
void retain_path_points(const ScenePropertyBlock& bag, GameSceneEntityRecord& record) {
    const ScenePropertyBlock* points = nullptr;
    for (const auto& block : bag.blocks) {
        if (equal_insensitive(block.first, "PathPoints")) { points = &block.second; break; }
    }
    if (points == nullptr) {
        record.path_points_error = "PathPoints block absent";
        return;
    }
    for (std::size_t index = 0;; ++index) {
        char key[40];
        std::snprintf(key, sizeof(key), "Point%02zu", index);
        // A scalar at this key has native type != 6 and terminates the loop.
        if (points->find(key) != nullptr) break;
        const ScenePropertyBlock* point = nullptr;
        for (const auto& block : points->blocks) {
            if (equal_insensitive(block.first, key)) { point = &block.second; break; }
        }
        if (point == nullptr) break;
        const SceneProperty* position = point->find("Pos");
        std::array<float, 3> value{};
        bool valid = position != nullptr && position->type_letter == "V3"
            && position->values.size() == value.size();
        for (std::size_t lane = 0; valid && lane < value.size(); ++lane) {
            valid = scene_scan_float(position->values[lane], value[lane]);
        }
        if (!valid) {
            // The native dereferences the schema-backed V3 value directly.
            // An unsupported/malformed bag cannot become invented geometry.
            record.path_points_error = std::string(key) + ".Pos is not a readable V3";
            record.path_points_local.clear();
            return;
        }
        record.path_points_local.push_back(value);
    }
    record.path_points_retained = true;
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
    // 007F4580's per-wing loop makes its planes while the squadron is being
    // attached, which in this process is in the middle of the instantiate pass,
    // and `entities` is being appended to and held by reference there. The
    // member records are therefore staged here and flushed into `entities` once
    // the pass is over; create_units reads the whole list afterwards either way.
    std::vector<GameSceneEntityRecord> pending_squadron_members;
    ScenePropertyBlock root_properties;
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
    auto* manager = vfs.ready() ? &vfs.context() : nullptr;
    if (manager == nullptr) return false;
    std::string resolved = name;
    if (!resolve_existing_resource_00bdf4c0_fragment(*manager,
            vfs.search_registrations(), resolved)) {
        return false;
    }
    VfsMemoryOpen opened = open_resource_memory_00bdf310_fragment(*manager,
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
    auto* manager = vfs.ready() ? &vfs.context() : nullptr;
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
        if (!enumerate_resources_00bdd990_fragment(*manager,
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

// 006CADD0 mode 1, the reading half: which authored keys the deck comes from.
// The rule that turns them into the block is
// bsp::air_ops_load_from_scene_006cadd0. docs/AIROPS_LOAD_FROM_SCENE.md.
std::int32_t scene_deck_int(const ScenePropertyBlock& block, const char* key) {
    const SceneProperty* property = block.find(key);
    if (property == nullptr || property->values.empty()) return 0;
    std::int32_t value = 0;
    // 008F2260 answers with the property record and the loader takes its +0Ch
    // payload without a type test on these scalars, so an unparsable value is
    // the same zero the absent key gives.
    if (!scene_scan_int(property->values.back(), value)) return 0;
    return value;
}

std::string scene_deck_text(const ScenePropertyBlock& block, const char* key) {
    const SceneProperty* property = block.find(key);
    if (property == nullptr || property->values.empty()) return std::string();
    return property->values.back();
}

const ScenePropertyBlock* scene_deck_sub_block(const ScenePropertyBlock& block,
    const std::string& name) {
    for (const auto& row : block.blocks) {
        if (row.first == name) return &row.second;
    }
    return nullptr;
}

// 006CB0D5 and 006CB1B2 compose the key with the 1-based index and stop at the
// first index the bag does not answer (006CB0F9 and 006CB1DA take the exit when
// the lookup returns null).
bsp::AirOpsSceneDeck read_scene_deck_006cadd0(const ScenePropertyBlock& bag) {
    bsp::AirOpsSceneDeck authored;
    authored.num_slots = scene_deck_int(bag, "NumSlots");
    authored.max_in_air_planes = scene_deck_int(bag, "MaxInAirPlanes");
    for (int index = 1;; ++index) {
        char key[32];
        std::snprintf(key, sizeof(key), "PlaneStock %d", index);
        const ScenePropertyBlock* sub = scene_deck_sub_block(bag, key);
        if (sub == nullptr) break;
        bsp::AirOpsSceneStock row;
        row.type = scene_deck_text(*sub, "Type");
        row.count = scene_deck_int(*sub, "Count");
        row.squad_limit = scene_deck_int(*sub, "SquadLimit");
        authored.stock.push_back(row);
    }
    for (int index = 1;; ++index) {
        char key[32];
        std::snprintf(key, sizeof(key), "Slot %d", index);
        const ScenePropertyBlock* sub = scene_deck_sub_block(bag, key);
        if (sub == nullptr) break;
        bsp::AirOpsSceneSlot row;
        row.type = scene_deck_text(*sub, "Type");
        row.count = scene_deck_int(*sub, "Count");
        row.arm = scene_deck_int(*sub, "Arm");
        // 006CB236 requires property type 3 and reads the byte at +0Ch, so any
        // other shape leaves the flag false.
        const SceneProperty* fake = sub->find("FakeAllocated");
        if (fake != nullptr && !fake->values.empty()) {
            const std::string& text = fake->values.back();
            std::int32_t parsed = 0;
            row.fake_allocated = scene_scan_int(text, parsed) ? parsed != 0 : text == "true";
        }
        authored.slots.push_back(row);
    }
    return authored;
}

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
        owner_.root_properties = block;
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
    struct AuthoredParent {
        std::size_t scene_id{0};
        std::string name;
        std::array<float, 16> world{};
    };
    // visit_entity calls a parent before its children while their addresses
    // remain stable. Consume entries on arrival so later root-stack reuse
    // cannot accidentally acquire the preceding root's parent.
    std::map<const SceneEntity*, AuthoredParent> authored_parents_;
    std::size_t next_scene_id_{1};
    GameSceneContentsHost::Impl& owner_;
    SceneFilePass pass_;
    int mode_{8};
    std::size_t groups_{0};
    std::size_t root_properties_{0};
    std::size_t precache_properties_{0};
    std::int32_t mission_id_{0};
};

// 007F4580 mode 1's per-wing loop, in the one place this process can run it: the
// squadron's scene row has just been created, its property bag is in hand, and
// the units it needs do not exist yet because create_units runs over the whole
// record list later. So the loop produces RECORDS, one per wing, and they are
// staged for the flush at the end of the pass.
//
// LABELLED SUBSTITUTION, and the one this lineage already made: the native's
// squadron is a separate 0x414 container that owns WingCount planes, while this
// process fuses the container with its own flight leader. `stored` - the unit
// the scene row made, carrying the authored name the script orders by - is
// wing 0, `members[0]`, and this queues wings 1..WingCount-1 beside it. The
// consequence is visible in one place: the native names wing 0
// `<squadron>|.-1` and this leaves it the authored name. 007F3820 cuts a member
// name at the first `|` (_strcspn at 007F38AF) and re-prefixes it, which for a
// name with no separator yields the squadron name unchanged, so the fused
// leader carries the name that routine would give it.
//
// What is NOT a substitution: the count, the naming of the other wings, the
// array order, the spawn index and the back pointer all come from
// `plane_squadron_plan_members_007f4580`, which drives the reconstructed rule.
static void queue_plane_squadron_wing_007f4580(GameSceneContentsHost::Impl& owner,
    const ScenePropertyBlock& bag, const GameSceneEntityRecord& stored) {
    // 007F471D / 007F4735 / 007F4794 / 007F47C2: the four keys 008F2260 is asked
    // for, in the order the listing asks for them. `Type` is already resolved on
    // the record, because the enum library resolved it during the registration
    // pass; the other three are read here.
    bsp::PlaneSquadronSpawnRequest request;
    request.squadron_name = stored.name;
    request.type_class_id = stored.type_id;
    if (const SceneProperty* prop = bag.find(kSceneUnitWingCountKey)) {
        std::int32_t authored = 0;
        if (!prop->values.empty() && scene_scan_int(prop->values.back(), authored)) {
            request.wing_count_present = true;
            request.wing_count_raw = authored;
        }
    }
    // 007F47A6 CMP [EAX+4h],EBP and 007F47D0 CMP [EAX+4h],0: both keys are used
    // only when the property's type tag is 0, which is the authored `I` letter
    // (0082359C reads the same tag the same way).
    if (const SceneProperty* prop = bag.find(kSceneUnitPlaneParentIdKey)) {
        std::int32_t parent = 0;
        if (prop->type_letter == "I" && !prop->values.empty()
            && scene_scan_int(prop->values.back(), parent)) {
            request.parent_present = true;
            request.parent_id = parent;
        }
    }
    if (const SceneProperty* prop = bag.find(kSceneUnitBehaviourKey)) {
        std::int32_t behaviour = 0;
        if (prop->type_letter == "I" && !prop->values.empty()
            && scene_scan_int(prop->values.back(), behaviour)) {
            request.behaviour_present = true;
            request.behaviour = behaviour;
        }
    }
    // 007F47AE -> 00521E30 resolves `PlaneParentID` to a live entity. This
    // process has no entity handle table at scene-load time, so the parent stays
    // unresolved and every wing takes the no-parent arm 007F48BE, which places
    // the plane under the squadron's own world node with the squadron's local
    // matrix. That is the arm a squadron with no `PlaneParentID` takes anyway;
    // for one that authors it, the placement is a SUBSTITUTION and the key is
    // reported. contract: unread for 00521E30.
    request.parent_entity = 0u;

    const bsp::PlaneSquadronSpawnPlan plan =
        bsp::plane_squadron_plan_members_007f4580(request);
    if (plan.members.empty()) return;

    bsp::PlaneSquadronHostRecord& record = bsp::plane_squadron_registry().add(stored.name);
    record.wing_count = plan.wing_count;          // +3C8h
    record.behaviour = plan.behaviour;            // +364h
    record.type_class_id = stored.type_id;
    record.party = stored.party;
    record.from_air_ops_launch = false;
    record.member_names.clear();
    record.member_spawn_index.clear();
    record.member_units.clear();

    std::size_t queued = 0;
    for (std::size_t wing = 0; wing < plan.members.size(); ++wing) {
        const bsp::PlaneSquadronMemberPlan& member = plan.members[wing];
        if (wing == 0) {
            // The fused leader: the record the scene row already made.
            record.member_names.push_back(stored.name);
            record.member_spawn_index.push_back(member.spawn_index);
            continue;
        }
        GameSceneEntityRecord wing_record;
        wing_record.name = member.name;
        // 007F4811 builds the plane through the vehicle class's own vtable +28h,
        // which for this `Type` is 007CFD20 BSP_PlaneUnitInstance_Construct. The
        // member is a plane, not a second PlaneSquadronGen, so it carries no
        // scene class id and does not enter the scene class census.
        wing_record.class_name = "PlaneUnitInstance";
        wing_record.class_id = -1;
        wing_record.type_symbol = stored.type_symbol;
        wing_record.type_table = stored.type_table;
        wing_record.type_id = stored.type_id;
        wing_record.party_symbol = stored.party_symbol;
        wing_record.party = stored.party;
        wing_record.generated = true;
        wing_record.created = true;
        // 007F48D2's no-parent arm places the plane with the squadron's own
        // matrix. 007F4813 has no per-wing offset: the formation spacing belongs
        // to the pilot bot, not to the spawn.
        std::copy_n(stored.world, 16, wing_record.world);
        std::copy_n(stored.local, 16, wing_record.local);
        owner.pending_squadron_members.push_back(std::move(wing_record));
        record.member_names.push_back(member.name);
        record.member_spawn_index.push_back(member.spawn_index);
        ++queued;
    }
    owner.log.notef("plane squadron %s: WingCount=%d (%s) -> %d member plane(s), %zu "
        "queued beside the fused leader%s (007F4580 mode 1, tail 007F4B43..007F4B6E)",
        stored.name.c_str(), plan.wing_count,
        request.wing_count_present ? "authored" : "code default 3",
        plan.plane_count, queued,
        plan.refused_overflow ? ", array full at five (007F4B55 has no bound test)" : "");
    owner.log.implemented("PlaneSquadron::spawn_planes", "007f4580");
}

// Packet cc9_scene_path_landscape. Whether this process's VFS finds `name` by
// the same search the other scene reads use. A census of 00882AC0's inputs, not
// a native step: 00882AC0 hands the names to the terrain and model loaders.
static bool scene_vfs_resolves(GameSceneContentsHost::Impl& owner, const std::string& name) {
    auto* manager = owner.vfs.ready() ? &owner.vfs.context() : nullptr;
    if (manager == nullptr) return false;
    std::string resolved = name;
    return resolve_existing_resource_00bdf4c0_fragment(*manager,
        owner.vfs.search_registrations(), resolved);
}

// Packet cc9_landscape_terrain. The rounding points of the x87 code are kept by
// storing through float at each FSTP the listing shows.
static float terrain_f32(double value) noexcept { return static_cast<float>(value); }

// __stricmp (00ADDC91 NODE, 00ADFEB0 U16) and 00425850 (TRNV2) compare tags
// without regard to case.
static bool terrain_tag_is(const bsp::StructuredNode& node, const char* name) noexcept {
    const std::string& tag = node.tag();
    const std::size_t length = std::strlen(name);
    if (tag.size() != length) return false;
    for (std::size_t i = 0; i != length; ++i) {
        if (std::tolower(static_cast<unsigned char>(tag[i]))
            != std::tolower(static_cast<unsigned char>(name[i]))) return false;
    }
    return true;
}

// 00ADDA60, the `.tdt` parse, over the bytes 0109CEEC vt+4 opened. The root's
// two dwords size the tile grid (00ADDC17 / 00ADDC26 -> 00ADADC0); each `NODE`
// child carries its tile x and z (two dwords) and 00ADFEB0 reads its `U16`
// chunk into a 33x33 block (00ADFD70 -> 00ADC6C0). `F32` and `U8` chunks are
// skipped here: none of this installation's island files was seen to carry
// one, and a file that did is reported, not approximated. The origin is
// 00ADDB60..00ADDBFB for a `TRNV2` root.
static std::shared_ptr<SceneTerrainHeightField> load_terrain_height_field_00adda60(
    const std::string& bytes, float box_min_x, float box_min_z, std::string& error) {
    auto stream = std::make_shared<bsp::MemoryStream>(
        bsp::memory_stream_from_complete_bytes(bytes.data(), bytes.size()));
    bsp::StructuredReader reader(stream);
    auto root = reader.read_root_00bea700();
    if (!root) { error = "no structured root"; return nullptr; }
    auto field = std::make_shared<SceneTerrainHeightField>();
    field->root = root->tag();
    std::uint32_t wide = 0, deep = 0;
    if (!root->read_u32(wide) || !root->read_u32(deep)) {
        error = "root lacks the grid dwords"; return nullptr;
    }
    field->tiles_wide = static_cast<int>(wide);
    field->tiles_deep = static_cast<int>(deep);
    field->block_index.assign(static_cast<std::size_t>(wide) * deep, -1);
    field->box_min_x = box_min_x;
    field->box_min_z = box_min_z;
    if (terrain_tag_is(*root, "TRNV2")) {
        // FLD +68h / FDIV 300 (float) / FSUB 1.0 (double) / FSTP float, floor
        // (00BF85B0) to float, FMUL 300 / FSTP +80h; the same for +70h -> +84h.
        const float tile = 300.0f;
        const float ux = terrain_f32(static_cast<double>(box_min_x) / tile - 1.0);
        const float uz = terrain_f32(static_cast<double>(box_min_z) / tile - 1.0);
        field->origin_x = terrain_f32(static_cast<double>(terrain_f32(std::floor(ux))) * tile);
        field->origin_z = terrain_f32(static_cast<double>(terrain_f32(std::floor(uz))) * tile);
    }
    while (root->has_remaining_00715bf0()) {
        auto node = root->read_child_00bea680();
        if (!node) { error = "bad root child"; return nullptr; }
        if (!terrain_tag_is(*node, "NODE")) {
            if (!node->skip_00be9c40() || !node->close()) { error = "bad skip"; return nullptr; }
            continue;
        }
        std::uint32_t tx = 0, tz = 0;
        if (!node->read_u32(tx) || !node->read_u32(tz)) { error = "NODE lacks x/z"; return nullptr; }
        // 00ADDCEE stores the tile at +40h[+38h * z + x] with no bound test;
        // an out-of-grid tile is an error here rather than a stray write.
        if (tx >= wide || tz >= deep) { error = "NODE outside the grid"; return nullptr; }
        while (node->has_remaining_00715bf0()) {
            auto chunk = node->read_child_00bea680();
            if (!chunk) { error = "bad NODE child"; return nullptr; }
            if (terrain_tag_is(*chunk, "U16")) {
                SceneTerrainBlock block;
                float scale = 0.0f;
                if (!chunk->read_float(block.offset) || !chunk->read_float(scale)) {
                    error = "U16 lacks its floats"; return nullptr;
                }
                block.inv_scale = terrain_f32(1.0 / static_cast<double>(scale));  // +34h
                for (std::uint16_t& sample : block.samples) {
                    if (!chunk->read_bytes(&sample, 2)) { error = "short U16"; return nullptr; }
                }
                field->block_index[static_cast<std::size_t>(wide) * tz + tx]
                    = static_cast<int>(field->blocks.size());
                field->blocks.push_back(block);
            } else if (terrain_tag_is(*chunk, "F32") || terrain_tag_is(*chunk, "U8")) {
                error = "unsupported " + chunk->tag() + " chunk"; return nullptr;
            } else if (!chunk->skip_00be9c40()) {
                error = "bad chunk skip"; return nullptr;
            }
            if (!chunk->close()) { error = "bad chunk close"; return nullptr; }
        }
        if (!node->close()) { error = "bad NODE close"; return nullptr; }
    }
    return field;
}

// 004EA650 (Path) and 004F1460 (Landscape), with what PlaceInWorld 00928860
// does through slot 130h, then the InitAll passes that fill the class's own
// fields (pass B 007B38D0 for a Path, pass A 00883BB0 for a Landscape). The
// native runs the passes at 0046EB4B, after the whole scene file is read; this
// process fills them at creation because nothing reads them in between.
// LABELLED SUBSTITUTION: the frame is the record's composed world frame with
// no hierarchy parent, the convention the unit creators in this file already
// use (creation.local_frame = world_frame), where the native passes the
// authored localframe (0046D592) and its hierarchy parent (0046D585).
static void create_scene_world_object(GameSceneContentsHost::Impl& owner,
    GameSceneEntityRecord& record, const ScenePropertyBlock& bag, GameSceneClassTally& tally) {
    const bool is_path = record.class_id == kScenePathClassId;
    SceneWorldObject object;
    object.class_id = record.class_id;
    object.name = record.name;
    object.scene_id = record.scene_id;
    object.parent_scene_id = record.parent_scene_id;
    object.vtable = is_path ? 0x00ce6290u : 0x00cea090u;         // 0047B660 / 004F1272
    object.object_size = is_path ? 0x210u : 0x430u;              // 004EA66x / 004F147B
    object.network_id = 0;                                       // 006AF40F XOR AX,AX
    object.class_list_offset = is_path ? 0x36cu : 0x348u;        // 0048721C / 004F140C
    std::memcpy(object.world, record.world, sizeof(object.world));
    if (is_path) {
        // 007B38F4 finds `Party` and 007B38FC stores its +0Ch at entity+54h; the
        // record's resolved party is the same enum value. The point count is
        // the partial projection of 007B34F0 retain_path_points already made.
        object.party = record.party;
        object.path_points = record.path_points_local.size();
        object.path_points_valid = record.path_points_retained;
    } else {
        object.file_path = scene_deck_text(bag, "FilePath");      // 00883C5E -> +3C4h
        object.model_path = scene_deck_text(bag, "ModelPath");    // 00883C7B -> +424h
        object.shallow_water_block = scene_deck_sub_block(bag, "ShallowWater") != nullptr
            || bag.find("ShallowWater") != nullptr;               // 00883E1D
        // 00882AC0 with ECX = the Landscape and the FilePath string (00883DD3).
        object.heightmap_name = "terrain/" + object.file_path + "_heightmap.tdt";
        object.colormap_name = "terrain/" + object.file_path + "_colormap.dds";
        object.model_name = "models/terrain/" + object.file_path + ".mmod";
        object.heightmap_resolved = scene_vfs_resolves(owner, object.heightmap_name);
        object.colormap_resolved = scene_vfs_resolves(owner, object.colormap_name);
        object.model_resolved = scene_vfs_resolves(owner, object.model_name);
        // What the terrain load builds and this process does not: the terrain
        // object +3D0h (00ADD290), the model resource and instance +41Ch/+420h,
        // the render node +3CCh, the part instance +418h (spatial attach
        // 00710B6D) and the shallow-water decal +42Ch. Pass A itself then
        // attaches the collision node +1E4h to the spatial index as a static
        // root (0098BA10 at 00884078); the two attaches are how an island
        // reaches the segment queries.
        owner.log.unimplemented("Landscape::load_terrain", "00882ac0");
        owner.log.unimplemented("Landscape::attach_terrain_vcall_9c", "00883bb0");
        if (kSceneLandscapeTerrainBound) {
            // Packet cc9_landscape_terrain: the +3D0h height field, at the
            // pass-A point. LABELLED SUBSTITUTION for +68h / +70h: 00ADE500
            // takes them from 00ADA420's box over the terrain node's tree, which
            // holds the island model; this uses the model's own BoundingBox
            // (read_mmod_bounding_box), whose minimum puts m07_a's authored
            // objects on the sampled ground (docs/SCENE_CONTENTS_HOSTS.md 6).
            std::string model_bytes;
            std::array<float, 6> box{};
            const bool have_box = owner.read_vfs_file(object.model_name, model_bytes)
                && bsp::read_mmod_bounding_box(
                    std::vector<std::uint8_t>(model_bytes.begin(), model_bytes.end()), box);
            std::string tdt;
            std::string error;
            std::shared_ptr<SceneTerrainHeightField> field;
            if (!have_box) {
                error = "no model BoundingBox";
            } else if (!owner.read_vfs_file(object.heightmap_name, tdt)) {
                error = "heightmap not readable";
            } else {
                field = load_terrain_height_field_00adda60(tdt, box[0], box[2], error);
            }
            if (field) {
                // 00ADE820 hands the terrain node the Landscape's +74h frame; a
                // top-level Landscape's local frame is its world frame.
                field->node_x = object.world[12];
                field->node_y = object.world[13];
                field->node_z = object.world[14];
                float lo = 1e30f, hi = -1e30f;
                for (const SceneTerrainBlock& block : field->blocks) {
                    for (std::uint16_t s : block.samples) {
                        if (s == 0xffffu) continue;
                        const float h = terrain_f32(s * static_cast<double>(block.inv_scale)
                            + block.offset);
                        lo = std::min(lo, h);
                        hi = std::max(hi, h);
                    }
                }
                owner.log.notef("scene terrain loaded: landscape=%s root=%s tiles=%dx%d "
                    "blocks=%zu box_min=(%.4f,%.4f) origin=(%.1f,%.1f) node=(%.1f,%.1f,%.1f) "
                    "height=[%.3f,%.3f] (00adda60)", object.name.c_str(), field->root.c_str(),
                    field->tiles_wide, field->tiles_deep, field->blocks.size(),
                    field->box_min_x, field->box_min_z, field->origin_x, field->origin_z,
                    field->node_x, field->node_y, field->node_z, lo, hi);
                field->sample_min = lo;
                field->sample_max = hi;
                owner.log.implemented("Landscape::load_height_field", "00adda60");
                object.terrain = std::move(field);
            } else {
                owner.log.notef("scene terrain refused: landscape=%s heightmap=%s: %s",
                    object.name.c_str(), object.heightmap_name.c_str(), error.c_str());
                owner.log.unimplemented("Landscape::load_height_field", "00adda60");
            }
        }
    }
    const std::size_t index = scene_world_class_lists().objects().size();
    scene_world_class_lists().append(object);
    ++tally.objects;
    if (tally.creator_state.empty() || tally.creator_state.compare(0, 6, "record") == 0) {
        char state[96];
        std::snprintf(state, sizeof(state), "concrete %08x (scene object, not a unit)",
            is_path ? 0x004ea650u : 0x004f1460u);
        tally.creator_state = state;
    }
    if (is_path) {
        owner.log.notef("scene world object: class=47 name=%s id=%zu parent=%zu "
            "list=world+36Ch index=%zu party=%d points=%zu valid=%d (004ea650, 00487210)",
            object.name.c_str(), object.scene_id, object.parent_scene_id, index,
            object.party, object.path_points, object.path_points_valid ? 1 : 0);
        owner.log.implemented("SceneContents::create_path", "004ea650");
    } else {
        owner.log.notef("scene world object: class=44 name=%s id=%zu list=world+348h "
            "index=%zu FilePath=\"%s\" ModelPath=\"%s\" ShallowWater=%d heightmap=%s:%d "
            "colormap=%s:%d model=%s:%d (004f1460, 004f1400)", object.name.c_str(),
            object.scene_id, index, object.file_path.c_str(), object.model_path.c_str(),
            object.shallow_water_block ? 1 : 0, object.heightmap_name.c_str(),
            object.heightmap_resolved ? 1 : 0, object.colormap_name.c_str(),
            object.colormap_resolved ? 1 : 0, object.model_name.c_str(),
            object.model_resolved ? 1 : 0);
        owner.log.implemented("SceneContents::create_landscape", "004f1460");
    }
}

void SceneReaderBinding::instantiate_entity(const SceneEntity& entity,
    const float world_frame[16], SceneFilePass pass) {
    GameSceneContentsHost::Impl& owner = owner_;
    const std::size_t scene_id = next_scene_id_++;
    AuthoredParent authored_parent;
    const auto parent_entry = authored_parents_.find(&entity);
    if (parent_entry != authored_parents_.end()) {
        authored_parent = parent_entry->second;
        authored_parents_.erase(parent_entry);
    }
    AuthoredParent child_parent;
    child_parent.scene_id = scene_id;
    child_parent.name = entity.name;
    std::copy_n(world_frame, child_parent.world.size(), child_parent.world.begin());
    for (const SceneEntity& child : entity.children) {
        authored_parents_[&child] = child_parent;
    }
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
    // Packet cc9_scene_race_and_script_identity: 0092708F pushes "Race" (00CE8EE0).
    const SceneEnumProperty race = scene_enum_property(bag, "Race");
    int race_id = -1;
    if (race.present) owner.library.resolve_symbol(race.table, race.symbol, race_id);

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
    // Packet cc9_ship_weapon_director_enable: the four director keys pass B
    // reads (008238F0..008239A3), kept by name when the bag carries any.
    {
        const SceneProperty* keys[4] = {bag.find("ArtilleryDirector"), bag.find("AADirector"),
            bag.find("TorpedoDirector"), bag.find("DCDirector")};
        if (keys[0] != nullptr || keys[1] != nullptr || keys[2] != nullptr || keys[3] != nullptr) {
            SceneDirectorEnables enables;
            if (keys[0] != nullptr) enables.artillery = scene_property_bool(keys[0]);
            if (keys[1] != nullptr) enables.anti_air = scene_property_bool(keys[1]);
            if (keys[2] != nullptr) enables.torpedo = scene_property_bool(keys[2]);
            if (keys[3] != nullptr) enables.depth_charge = scene_property_bool(keys[3]);
            scene_director_enables_set(entity.name, enables);
        }
    }
    GameSceneEntityRecord record;
    record.name = entity.name;
    record.class_name = entity.class_name;
    record.class_id = klass->class_id;
    record.type_symbol = type.symbol;
    record.type_table = type.table;
    record.type_id = type_id;
    record.party_symbol = party.symbol;
    record.party = party_id;
    record.race = race_id;
    {
        // Packet cc9_ai_owner_player_slot: 0077F1C8.. finds `OwnerPlayer` and
        // hands its +0Ch to vtable[144h] (unit+180h), else 9 (0077F1F1).
        const SceneEnumProperty owner_player = scene_enum_property(bag, "OwnerPlayer");
        if (owner_player.present) {
            record.owner_player_symbol = owner_player.symbol;
            int value = 9;
            if (owner.library.resolve_symbol(owner_player.table, owner_player.symbol, value)) {
                record.owner_player = value;
            } else {
                ++owner.summary.owner_player_unresolved;
            }
        }
    }
    {
        // Packet cc9_scene_unit_skill: 00927A80. Both finds go through 008F2260
        // and read the record's +0Ch integer, so an enum symbol resolves through
        // the library and an `I` value is its own integer.
        const auto bag_int = [&](const char* key, int& out) -> bool {
            const SceneProperty* prop = bag.find(key);
            if (prop == nullptr || prop->values.empty()) return false;
            if (prop->type_letter == "I") {
                std::int32_t v = 0;
                if (!scene_scan_int(prop->values.back(), v)) return false;
                out = static_cast<int>(v);
                return true;
            }
            const SceneEnumProperty e = scene_enum_property(bag, key);
            return e.present && owner.library.resolve_symbol(e.table, e.symbol, out);
        };
        int v = 0;
        if (bag_int("Skill", v)) {                     // 00CF8838, 00927A97
            record.bag_skill = v;
            record.bag_skill_source = "Skill";
        } else if (bag_int("Crew", v)) {               // 00D19264, 00927AB4
            // 006E6210 with game+1FE4h == 0 (single player, this host's only
            // mode): 0, 1, 2 map to themselves, 3 to Elite 5, anything else 1.
            record.bag_skill = v == 0 ? 0 : v == 1 ? 1 : v == 2 ? 2 : v == 3 ? 5 : 1;
            record.bag_skill_source = "Crew";
        }
    }
    if (klass->class_id == 0x18) {
        // Packet cc9_scene_home_base_contract: 007F4C43 reads `HomeBase` from the
        // squadron's bag at pass C; carried on the record for the units host.
        record.home_base_carried = true;
        if (const SceneProperty* home = bag.find("HomeBase")) {
            std::string name = home->values.empty() ? std::string() : home->values.back();
            if (name.size() >= 2 && name.front() == '"' && name.back() == '"') {
                name = name.substr(1, name.size() - 2);
            }
            record.home_base = name;
        }
    }
    record.generated = gate.generate;
    record.gate_rule = rule_name;
    std::memcpy(record.world, world_frame, sizeof(record.world));
    record.scene_id = scene_id;
    record.parent_scene_id = authored_parent.scene_id;
    record.parent_name = authored_parent.name;
    std::memcpy(record.local, entity.frame, sizeof(record.local));
    std::copy(authored_parent.world.begin(), authored_parent.world.end(), record.parent_world);
    if (klass->class_id == 0x1a) retain_land_convoy_roster(bag, record, owner.library);
    if (klass->class_id == 0x47) {
        retain_path_points(bag, record);
        owner.log.notef("scene path retained: id=%zu parent=%zu name=%s points=%zu "
            "valid=%d creator_concrete=0", record.scene_id, record.parent_scene_id,
            record.name.c_str(), record.path_points_local.size(), record.path_points_retained);
        // Packet cc8_ship_moveonpath: the same local-to-world step 007AF800
        // makes for an avoidance path, so a `moveonpath` command can find the
        // authored path by the name `FindEntity` resolved.
        if (record.path_points_retained && !record.path_points_local.empty()) {
            std::vector<std::array<float, 3>> world_points;
            world_points.reserve(record.path_points_local.size());
            for (const std::array<float, 3>& authored : record.path_points_local) {
                const std::array<float, 4> source{authored[0], authored[1], authored[2], 1.0f};
                std::array<float, 4> transformed{};
                transform_native_vector4_00b62d10(source.data(), transformed.data(),
                                                  record.world);
                const float w = transformed[3] != 0.0f ? transformed[3] : 1.0f;
                world_points.push_back({transformed[0] / w, transformed[1] / w,
                                        transformed[2] / w});
            }
            scene_path_registry().add(record.name, std::move(world_points));
        }
    }

    if (!gate.generate) {
        record.skipped_because = std::string("gate rejected: ") + rule_name;
        ++tally.rejected;
        ++owner.summary.rejected;
        owner.entities.push_back(record);
        return;
    }

    // `Hidden`, and it is what holds an entity back for `GenerateObject`. In the
    // executable the test sits BEFORE the gate, not inside it, which is why
    // 0046C550 has nothing to say about a held-back entity and why this host's
    // `rejected=0` was a faithful answer to the wrong question:
    //
    //   0046d39d: CMP byte ptr [EDI + 0x4],0x0   ; the pass flag
    //   0046d3b3: JZ  0x0046d3cb                 ; clear -> no Hidden test
    //   0046d3b5: PUSH 0xce5708                  ; "Hidden"
    //   0046d3bc: CALL 0x008f2260                ; bag.find
    //   0046d3c1: CMP byte ptr [EAX + 0xc],0x0
    //   0046d3c5: JNZ 0x0046d5e4                 ; SET -> jumps past the creation
    //   ...
    //   0046d426: CALL 0x0046c550                ; never reached for a hidden one
    //
    // A hidden entity is therefore REGISTERED AND NOT CREATED: the registration
    // branch still runs 0046BF70 at 0046D531, which reads the same string three
    // more times, so the record stays in the scene database's named-object map
    // and `GenerateObject` instantiates it from there by name. That is what stops
    // the script's own spawn step making a second carrier.
    //
    // The test runs on the instantiate pass only, which `[EDI+4]` selects; this
    // host reaches the creator from that pass alone, so the pass condition is the
    // call site rather than a flag here. The record is kept, exactly as the
    // native keeps it. docs/LUA_GENERATE_OBJECT_HOST.md.
    if (scene_property_bool(bag.find(bsp::kSceneHiddenPropertyKey))) {
        record.skipped_because = "Hidden: held back for GenerateObject";
        // `generated` means the instantiate pass took the entity, which is what
        // puts it on the pending list 00925F20 walks and what makes a
        // creator-less entity a scene marker with its own `thisTable` slot. A
        // hidden entity is skipped at 0046D3C5 BEFORE 0046C550 is called, so it
        // has no gate answer at all and is on no pending list. Leaving the gate's
        // `true` here made every held-back entity a marker: USN01's
        // `ScoutDauntless` took marker id 126, so `FindEntity` would have
        // answered for a unit that does not exist and `GenerateObject` would
        // later have added a second slot under a different id for the same name.
        record.generated = false;
        ++tally.rejected;
        ++owner.summary.rejected;
        ++owner.summary.held_back_hidden;
        // The stand-in for the scene database's named-object map at sceneDb+18h,
        // which is the only thing 0046D930 looks a name up in.
        scene_spawn_pool().add(record);
        // Same shape for a held-back PlaneSquadronGen row: 007F4580's mode-1 loop
        // reads `WingCount` out of this bag and the bag does not survive the
        // hold-back, so the key is carried on the pool entry and the wing is
        // spawned when the script creates the unit. UNVALIDATED BY A RUN: neither
        // USN01 nor USN04 reaches a GenerateObject call in its frame budget.
        if (klass != nullptr && klass->class_id == 0x18) {
            if (SceneSpawnPoolEntry* held = scene_spawn_pool().find(record.name)) {
                if (const SceneProperty* prop = bag.find(kSceneUnitWingCountKey)) {
                    std::int32_t authored = 0;
                    if (!prop->values.empty()
                        && scene_scan_int(prop->values.back(), authored)) {
                        held->wing_count_present = true;
                        held->wing_count_raw = authored;
                    }
                }
            }
        }
        // A held-back carrier or airfield skips the 006CADD0 mode 1 build below,
        // because that sits on the created path. Its deck is authored in this same
        // bag, so it is built here and handed over when the script spawns the
        // unit. Without this a script-spawned Zuikaku would answer
        // `GetProperty(carrier, "slots")` with nothing and could never launch.
        if (klass != nullptr
            && (klass->class_id == bsp::kAirOpsSceneClassIdMothership
                || klass->class_id == bsp::kAirOpsSceneClassIdAirfield)) {
            const bsp::AirOpsSceneDeck authored = read_scene_deck_006cadd0(bag);
            bsp::AirOpsDeck deck = bsp::air_ops_load_from_scene_006cadd0(authored,
                [](const std::string& type, void*) -> std::uint32_t {
                    std::int32_t parsed = 0;
                    if (!scene_scan_int(type, parsed) || parsed < 0) return 0u;
                    return static_cast<std::uint32_t>(parsed);
                },
                nullptr);
            deck.is_airfield = klass->class_id == bsp::kAirOpsSceneClassIdAirfield;
            deck.owner_name = record.name;
            deck.owner_party = record.party;
            if (SceneSpawnPoolEntry* held = scene_spawn_pool().find(record.name)) {
                held->has_deck = true;
                held->deck = std::move(deck);
            }
            owner.log.notef("air ops deck: unit=%s class=%d NumSlots=%d MaxInAirPlanes=%d "
                "held back for GenerateObject (006cadd0 mode 1, registered on spawn)",
                record.name.c_str(), klass->class_id, authored.num_slots,
                authored.max_in_air_planes);
        }
        owner.entities.push_back(record);
        return;
    }
    ++tally.generated;
    ++owner.summary.generated;

    // Packet cc9_scene_path_landscape. `created` stays false: create_units makes
    // a unit of every created record, and the markers pass (00925F20 pass A's
    // `thisTable` attach) keys on generated && !created, which is still true
    // of these two classes in the image.
    if (kScenePathLandscapeCreatorsBound
        && (klass->class_id == kScenePathClassId
            || klass->class_id == kSceneLandscapeClassId)) {
        create_scene_world_object(owner, record, bag, tally);
        owner.entities.push_back(record);
        return;
    }

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

    // 006CADD0 mode 1. The executable reaches it from the two classes that own a
    // deck: 006D3C10 BSP_AirField_ReadRunwayProperties for the airfield and
    // 007593D0 for the mother ship. Those are the same two whose Lua reader sits
    // at vtable+138h (docs/MISSION_LUA_GETPROPERTY.md), so the deck this builds
    // is exactly what `GetProperty(carrier, "slots")` reads back.
    if (klass->class_id == bsp::kAirOpsSceneClassIdMothership
        || klass->class_id == bsp::kAirOpsSceneClassIdAirfield) {
        const bsp::AirOpsSceneDeck authored = read_scene_deck_006cadd0(bag);
        // 007B8A80 is a thunk to 00964790 BSP_VehicleClass_GetOrCreate. Its
        // argument form was not read, and the scene authors these `Type` tokens
        // as numeric class ids, so the resolver here is the scan the rest of this
        // file uses for a scene integer. A token that is not a number resolves to
        // zero, which is what an unresolvable token does. contract.
        bsp::AirOpsDeck deck = bsp::air_ops_load_from_scene_006cadd0(authored,
            [](const std::string& type, void*) -> std::uint32_t {
                std::int32_t parsed = 0;
                if (!scene_scan_int(type, parsed) || parsed < 0) return 0u;
                return static_cast<std::uint32_t>(parsed);
            },
            nullptr);
        // 00895E4B tests the class through vtable+5Ch against 45h. This process
        // has no vtable to ask, and the scene class id is the same distinction.
        deck.is_airfield = klass->class_id == bsp::kAirOpsSceneClassIdAirfield;
        // block+7Ch is the owning entity, and 006C5050 reads `HomeBase`, `Party`,
        // `Skill` and `OwnerPlayer` off it. This process has no entity object, so
        // the deck carries the owner's authored name and the creator seam looks
        // the created unit up by it. docs/AIROPS_LAUNCH_TICK.md.
        deck.owner_name = stored.name;
        deck.owner_party = stored.party;
        // 006D3C10 kind 1: RunwayWidth / RunwayLength through 008F2260 into
        // 006BF0D0 (holder+B0h/+B4h). An integer-typed property is converted,
        // a float one is read as is; both parse the same from the scene text.
        if (deck.is_airfield) {
            const SceneProperty* w = bag.find("RunwayWidth");
            const SceneProperty* l = bag.find("RunwayLength");
            float wv = 0.0f, lv = 0.0f;
            if (w != nullptr && !w->values.empty() && l != nullptr && !l->values.empty()
                && scene_scan_float(w->values.back(), wv)
                && scene_scan_float(l->values.back(), lv)) {
                deck.runway_width = wv;
                deck.runway_length = lv;
                deck.runway_from_scene = true;
            }
            owner.log.notef("air ops runway: unit=%s RunwayWidth=%.2f RunwayLength=%.2f "
                "authored=%d (006d3c10 -> 006bf0d0)", stored.name.c_str(),
                static_cast<double>(deck.runway_width), static_cast<double>(deck.runway_length),
                deck.runway_from_scene ? 1 : 0);
            // 006D5220 kind 1, 006D5C00-006D5DD6: "Hangar 1" .. "Hangar 10".
            // A reference is kept by its last path component (the authored
            // `Landscape <name>\<entity>` form); an empty `Object` adds nothing.
            const auto reference_name = [](const SceneProperty* p) {
                if (p == nullptr || p->values.empty()) return std::string();
                std::string v = p->values.back();
                if (v.size() >= 2 && v.front() == '"' && v.back() == '"') {
                    v = v.substr(1, v.size() - 2);
                }
                const std::size_t cut = v.find_last_of('\\');
                return cut == std::string::npos ? v : v.substr(cut + 1);
            };
            for (int i = 1; i <= 10; ++i) {
                const std::string key = "Hangar " + std::to_string(i);
                for (const auto& block : bag.blocks) {
                    if (block.first != key) continue;
                    bsp::AirOpsDeck::HangarNames h;
                    h.object = reference_name(block.second.find("Object"));
                    h.entry_path = reference_name(block.second.find("EntryPath"));
                    h.exit_path = reference_name(block.second.find("ExitPath"));
                    if (h.object.empty()) break;
                    owner.log.notef("air ops hangar: unit=%s %s object=%s entry=%s exit=%s "
                        "(006d5220 kind 1)", stored.name.c_str(), key.c_str(),
                        h.object.c_str(), h.entry_path.c_str(), h.exit_path.c_str());
                    deck.hangars.push_back(std::move(h));
                    break;
                }
            }
        }
        owner.log.notef("air ops deck: unit=%s class=%d NumSlots=%d MaxInAirPlanes=%d "
            "slots=%zu stock=%zu (006cadd0 mode 1)", stored.name.c_str(), klass->class_id,
            authored.num_slots, authored.max_in_air_planes, deck.slots.size(),
            deck.stock.size());
        bsp::air_ops_decks().set(stored.name, std::move(deck));
        owner.log.implemented("AirOps::load_from_scene", "006cadd0");
    }

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
        if (klass->class_id == 0x18) {
            queue_plane_squadron_wing_007f4580(owner, bag, stored);
        }
        // 0046d5b0: operator new(0Ch) then 00922e20 wraps the bag and the holder
        // is stored at entity+C0h. The 0Ch record with vtable 00d03d94 is not
        // built here, but milestone 2q keeps what 00822C20's slot-0A0h arm
        // reads out of it: the holder's kind tag is 1 (00922e35 stores the
        // literal next to the cloned bag) and the payload is this same merged
        // bag, so the two finds 00823576 and 00823590 are answered from it.
        owner.log.implemented("SceneContents::property_bag_holder", "00922e20");
        const SceneProperty* launch_prop = bag.find(kSceneUnitShipYardLaunchKey);
        if (launch_prop != nullptr && !launch_prop->values.empty()) {
            std::int32_t flag = 0;
            if (scene_scan_int(launch_prop->values.back(), flag)) {
                stored.shipyard_launch = flag != 0;
            } else {
                stored.shipyard_launch = launch_prop->values.back() == "true";
            }
        }
        const SceneProperty* speed_prop = bag.find(kSceneUnitStartSpeedKey);
        if (speed_prop != nullptr && !speed_prop->values.empty()) {
            stored.start_speed_present = true;
            // 0082359C CMP [EAX+4h],EDI with EDI zero: type 0 (`I`) takes the
            // CVTSI2SS at 0082359E, every other type the float32 load at
            // 008235A5. The bag keeps the authored letter, so the letter is
            // what selects the arm here.
            stored.start_speed_type
                = (speed_prop->type_letter == "I")
                      ? static_cast<int>(ScenePropertyType::Int)
                      : static_cast<int>(ScenePropertyType::Float);
            std::int32_t as_int = 0;
            if (scene_scan_int(speed_prop->values.back(), as_int)) {
                stored.start_speed_int = as_int;
            }
            float as_float = 0.0f;
            if (scene_scan_float(speed_prop->values.back(), as_float)) {
                stored.start_speed_float = as_float;
            }
            ++owner.summary.start_speed_entities;
        }
        // Packet cc9_units_capture_accessors: 006F2780 copies the `CaptureRange`
        // record's +0Ch dword as is. An `I` value is that integer; an `F` value's
        // dword is its float bit pattern, which the FILD at 00A03760 would read as
        // an integer, so it is kept the same way.
        const SceneProperty* capture_prop = bag.find("CaptureRange");
        if (capture_prop != nullptr && !capture_prop->values.empty()) {
            std::int32_t as_int = 0;
            float as_float = 0.0f;
            if (capture_prop->type_letter == "I"
                && scene_scan_int(capture_prop->values.back(), as_int)) {
                stored.capture_range_present = true;
                stored.capture_range_raw = as_int;
            } else if (scene_scan_float(capture_prop->values.back(), as_float)) {
                std::int32_t bits = 0;
                std::memcpy(&bits, &as_float, sizeof bits);
                stored.capture_range_present = true;
                stored.capture_range_raw = bits;
            }
        }
        // Packet cc9_command_building_capture_bind: `CaptureValue` -> unit+7A4h,
        // the same way (006F2780, 1000 when absent).
        const SceneProperty* value_prop = bag.find("CaptureValue");
        if (value_prop != nullptr && !value_prop->values.empty()) {
            std::int32_t as_int = 0;
            float as_float = 0.0f;
            if (value_prop->type_letter == "I"
                && scene_scan_int(value_prop->values.back(), as_int)) {
                stored.capture_value_present = true;
                stored.capture_value_raw = as_int;
            } else if (scene_scan_float(value_prop->values.back(), as_float)) {
                std::int32_t bits = 0;
                std::memcpy(&bits, &as_float, sizeof bits);
                stored.capture_value_present = true;
                stored.capture_value_raw = bits;
            }
        }
        // 006F2847-006F285F, `LandingRange`, the same way (routed from cc9-ships13).
        const SceneProperty* landing_prop = bag.find("LandingRange");
        if (landing_prop != nullptr && !landing_prop->values.empty()) {
            std::int32_t as_int = 0;
            float as_float = 0.0f;
            if (landing_prop->type_letter == "I"
                && scene_scan_int(landing_prop->values.back(), as_int)) {
                stored.landing_range_present = true;
                stored.landing_range_raw = as_int;
            } else if (scene_scan_float(landing_prop->values.back(), as_float)) {
                std::int32_t bits = 0;
                std::memcpy(&bits, &as_float, sizeof bits);
                stored.landing_range_present = true;
                stored.landing_range_raw = bits;
            }
        }
        // Packet cc9_building_pad_model: 006F2895-006F28B3, `LandingPointRange`,
        // the same way.
        const SceneProperty* pad_prop = bag.find("LandingPointRange");
        if (pad_prop != nullptr && !pad_prop->values.empty()) {
            std::int32_t as_int = 0;
            float as_float = 0.0f;
            if (pad_prop->type_letter == "I"
                && scene_scan_int(pad_prop->values.back(), as_int)) {
                stored.landing_point_range_present = true;
                stored.landing_point_range_raw = as_int;
            } else if (scene_scan_float(pad_prop->values.back(), as_float)) {
                std::int32_t bits = 0;
                std::memcpy(&bits, &as_float, sizeof bits);
                stored.landing_point_range_present = true;
                stored.landing_point_range_raw = bits;
            }
        }
        // Packet cc9_submarine_depth_level: 00853630's two scene finds, kept
        // for the submarine seed in the units host. An enum symbol resolves
        // through the library (`Depth : Periscope` -> 1); an `I` value is
        // its own integer.
        const auto depth_value = [&](const char* key, bool& present,
                                     std::int32_t& level) {
            const SceneProperty* prop = bag.find(key);
            if (prop == nullptr || prop->values.empty()) return;
            if (prop->type_letter == "I") {
                std::int32_t v = 0;
                if (scene_scan_int(prop->values.back(), v)) {
                    present = true;
                    level = v;
                }
                return;
            }
            const SceneEnumProperty e = scene_enum_property(bag, key);
            int v = 0;
            if (e.present && owner.library.resolve_symbol(e.table, e.symbol, v)) {
                present = true;
                level = static_cast<std::int32_t>(v);
            }
        };
        depth_value("Dive", stored.dive_present, stored.dive_level);
        depth_value("TargetDive", stored.target_dive_present, stored.target_dive_level);
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
    // Packet cc9_avoid_zone_registry: the layers go into the registry singleton
    // [00E17620] the image's 004248A0 fills, where the planes' samples read them.
    GameAvoidZoneRegistry::instance().load_scene_layers(owner_.avoid_zone_layers);
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

// ---------------------------------------------------------------------------
// The GenerateObject pool, the stand-in for the named-object map at sceneDb+18h
// ---------------------------------------------------------------------------
void SceneSpawnPool::clear() noexcept { entries_.clear(); }

void SceneSpawnPool::add(const GameSceneEntityRecord& record) {
    // 0046D96F looks a name up in a map, so a repeated name is one entry.
    for (SceneSpawnPoolEntry& entry : entries_) {
        if (entry.record.name == record.name) return;
    }
    SceneSpawnPoolEntry entry;
    entry.record = record;
    entries_.push_back(entry);
}

SceneSpawnPoolEntry* SceneSpawnPool::find(const std::string& name) noexcept {
    for (SceneSpawnPoolEntry& entry : entries_) {
        if (entry.record.name == name) return &entry;
    }
    return nullptr;
}

std::size_t SceneSpawnPool::size() const noexcept { return entries_.size(); }

std::size_t SceneSpawnPool::spawned_count() const noexcept {
    std::size_t total = 0;
    for (const SceneSpawnPoolEntry& entry : entries_) {
        if (entry.spawned) ++total;
    }
    return total;
}

SceneSpawnPool& scene_spawn_pool() noexcept {
    static SceneSpawnPool pool;
    return pool;
}

void ScenePathRegistry::clear() noexcept { entries_.clear(); }

void ScenePathRegistry::add(const std::string& name,
                            std::vector<std::array<float, 3>> points_world) {
    for (ScenePathEntry& entry : entries_) {
        if (entry.name == name) {
            entry.points_world = std::move(points_world);
            return;
        }
    }
    entries_.push_back(ScenePathEntry{name, std::move(points_world)});
}

const ScenePathEntry* ScenePathRegistry::find(const std::string& name) const noexcept {
    for (const ScenePathEntry& entry : entries_) {
        if (entry.name == name) return &entry;
    }
    return nullptr;
}

std::size_t ScenePathRegistry::size() const noexcept { return entries_.size(); }

ScenePathRegistry& scene_path_registry() noexcept {
    static ScenePathRegistry registry;
    return registry;
}

void SceneWorldClassLists::clear() noexcept { objects_.clear(); }

// 00484540 appends at the tail, so a walk from the head (+4h of the list) sees
// the objects in creation order.
void SceneWorldClassLists::append(SceneWorldObject object) {
    objects_.push_back(std::move(object));
}

const std::vector<SceneWorldObject>& SceneWorldClassLists::objects() const noexcept {
    return objects_;
}

std::vector<std::size_t> SceneWorldClassLists::list(int class_id) const {
    std::vector<std::size_t> out;
    for (std::size_t i = 0; i < objects_.size(); ++i) {
        if (objects_[i].class_id == class_id) out.push_back(i);
    }
    return out;
}

SceneWorldClassLists& scene_world_class_lists() noexcept {
    static SceneWorldClassLists lists;
    return lists;
}

namespace {
std::map<std::string, SceneDirectorEnables>& director_enables_table() {
    static std::map<std::string, SceneDirectorEnables> table;
    return table;
}
}  // namespace

void scene_director_enables_clear() noexcept {
    director_enables_table().clear();
    scene_director_torpedo_writes() = SceneDirectorTorpedoWrites{};
}

void scene_director_enables_set(const std::string& name, const SceneDirectorEnables& enables) {
    director_enables_table()[name] = enables;
}

const SceneDirectorEnables* scene_director_enables_find(const std::string& name) noexcept {
    const auto it = director_enables_table().find(name);
    return it == director_enables_table().end() ? nullptr : &it->second;
}

SceneDirectorTorpedoWrites& scene_director_torpedo_writes() noexcept {
    static SceneDirectorTorpedoWrites writes;
    return writes;
}

// 0071C25B: MOV byte ptr [ECX+222h],DL, the sub-kind 5 arm of 0071C1E0.
bool scene_director_enables_set_torpedo(const std::string& name, bool enabled) {
    SceneDirectorEnables& e = director_enables_table()[name];  // 007202FD's 1s when new
    const bool changed = e.torpedo != enabled;
    e.torpedo = enabled;
    if (changed) ++scene_director_torpedo_writes().changed;
    return changed;
}

// ---------------------------------------------------------------------------
// Packet cc9_landscape_terrain: the terrain slots and the four world queries.

namespace {
constexpr float kTerrainNoGround = -1000.0f;   // [00D7A240] float, [00CE6658] double
constexpr float kTerrainCellSize = 9.375f;     // +1Ch, [00D0E658]
constexpr int kTerrainTileShift = 5;           // [00D5D65C]
constexpr int kTerrainTileCells = 32;          // [00D5D658]
}  // namespace

// 00ADB3A0 (slot 20h), with slot 10h/14h (00ADAC50: tiles * 32 + 1, 0 when the
// grid is empty), slot 44h 00ADA890 (tile = index >> 5, local = index - tile*32,
// the last tile's column 32 for the index one past it) and the block's slot 8h
// 00ADC5F0. The node y is added to a block's answer, hole included (00ADB43E);
// a missing tile or an index outside the grid answers -1000.0 bare.
float SceneTerrainHeightField::cell_height_00adb3a0(int i, int j) const noexcept {
    const int wide = tiles_wide != 0 ? tiles_wide * kTerrainTileCells + 1 : 0;
    const int deep = tiles_deep != 0 ? tiles_deep * kTerrainTileCells + 1 : 0;
    if (i < 0 || i >= wide || j < 0 || j >= deep) return kTerrainNoGround;
    int tx = i >> kTerrainTileShift, lx = i - (tx << kTerrainTileShift);
    int tz = j >> kTerrainTileShift, lz = j - (tz << kTerrainTileShift);
    if (tx >= tiles_wide) { tx -= 1; lx = kTerrainTileCells; }
    if (tz >= tiles_deep) { tz -= 1; lz = kTerrainTileCells; }
    const int index = block_index[static_cast<std::size_t>(tiles_wide) * tz + tx];
    if (index < 0) return kTerrainNoGround;
    const SceneTerrainBlock& block = blocks[static_cast<std::size_t>(index)];
    const std::uint16_t s = block.samples[static_cast<std::size_t>(33 * lz + lx)];
    const float value = s == 0xffffu ? kTerrainNoGround
        : terrain_f32(s * static_cast<double>(block.inv_scale) + block.offset);
    return terrain_f32(static_cast<double>(node_y) + value);
}

// 00ADA900 then 00ADB480. Grid coordinates are (world - node - origin) times
// float(1 / 9.375) (00ADA906..00ADA968); the indices are CVTTSS2SI truncations
// and every lerp is stored through float.
float SceneTerrainHeightField::height_00ada900(float x, float z) const noexcept {
    const float inv = terrain_f32(1.0 / kTerrainCellSize);
    const float u = terrain_f32((static_cast<double>(x) - node_x - origin_x) * inv);
    const float v = terrain_f32((static_cast<double>(z) - node_z - origin_z) * inv);
    return grid_height_00adb480(u, v);
}

// Packet cc9_land_convoy_movement. Slot 24h, 00ADA160: the same grid lookup in
// the Landscape's LOCAL frame, u = (x - origin_x) * float(1 / cell) and v the
// same with origin_z (00ADA161..00ADA1A8), then slot 48h 00ADB480. No node term.
float SceneTerrainHeightField::local_height_00ada160(float x, float z) const noexcept {
    const float inv = terrain_f32(1.0 / kTerrainCellSize);
    const float u = terrain_f32((static_cast<double>(x) - origin_x) * inv);
    const float v = terrain_f32((static_cast<double>(z) - origin_z) * inv);
    return grid_height_00adb480(u, v);
}

// 00ADB480, slot 48h: the bilinear height at grid (u, v).
float SceneTerrainHeightField::grid_height_00adb480(float u, float v) const noexcept {
    const int i = static_cast<int>(u);
    const int j = static_cast<int>(v);
    const float fu = terrain_f32(static_cast<double>(u) - i);
    const float h11 = cell_height_00adb3a0(i + 1, j + 1);
    const float h01 = cell_height_00adb3a0(i, j + 1);
    const float h10 = cell_height_00adb3a0(i + 1, j);
    const float h00 = cell_height_00adb3a0(i, j);
    const float a = terrain_f32(h00 + static_cast<double>(fu) * (static_cast<double>(h10) - h00));
    const float b = terrain_f32(h01 + static_cast<double>(fu) * (static_cast<double>(h11) - h01));
    const float fv = terrain_f32(static_cast<double>(v) - j);
    return terrain_f32(a + static_cast<double>(fv) * (static_cast<double>(b) - a));
}

// 00ADABA0 then 00ADAA40: three samples of the truncated cell, points
// (float(i), h, float(j)) with the cell size ADDED to the grid index for the
// two neighbours (00ADAA7E, 00ADAADB), edges d1 = P(i+1,j) - P(i,j) and
// d2 = P(i,j+1) - P(i,j), 004F9B30 cross(d2, d1), 00419440 length and a scale
// by float(1 / length), or by 0 when the length is not positive.
void SceneTerrainHeightField::normal_00adaba0(float x, float z, float out[3]) const noexcept {
    const float inv = terrain_f32(1.0 / kTerrainCellSize);
    const int i = static_cast<int>(terrain_f32((static_cast<double>(x) - node_x - origin_x) * inv));
    const int j = static_cast<int>(terrain_f32((static_cast<double>(z) - node_z - origin_z) * inv));
    cell_normal_00adaa40(i, j, out);
}

// Packet cc9_land_convoy_movement. Slot 34h, 00ADA1C0: the LOCAL-frame normal,
// j = _ftol((z - origin_z) * inv) and i = _ftol((x - origin_x) * inv), each
// product stored through float first (00ADA1E7, 00ADA201), then slot 30h
// 00ADAA40(out, i, j). No node term.
void SceneTerrainHeightField::local_normal_00ada1c0(float x, float z, float out[3]) const noexcept {
    const float inv = terrain_f32(1.0 / kTerrainCellSize);
    const int j = static_cast<int>(terrain_f32((static_cast<double>(z) - origin_z) * inv));
    const int i = static_cast<int>(terrain_f32((static_cast<double>(x) - origin_x) * inv));
    cell_normal_00adaa40(i, j, out);
}

// 00ADAA40, slot 30h: the unit normal of cell (i, j).
void SceneTerrainHeightField::cell_normal_00adaa40(int i, int j, float out[3]) const noexcept {
    const float fi = static_cast<float>(i);
    const float fj = static_cast<float>(j);
    const float h00 = cell_height_00adb3a0(i, j);
    const float h10 = cell_height_00adb3a0(i + 1, j);
    const float h01 = cell_height_00adb3a0(i, j + 1);
    const float p1x = terrain_f32(static_cast<double>(fi) + kTerrainCellSize);
    const float p2z = terrain_f32(static_cast<double>(fj) + kTerrainCellSize);
    const float d1[3] = {terrain_f32(static_cast<double>(p1x) - fi),
        terrain_f32(static_cast<double>(h10) - h00), 0.0f};
    const float d2[3] = {0.0f, terrain_f32(static_cast<double>(h01) - h00),
        terrain_f32(static_cast<double>(p2z) - fj)};
    const float c[3] = {
        terrain_f32(static_cast<double>(d2[1]) * d1[2] - static_cast<double>(d2[2]) * d1[1]),
        terrain_f32(static_cast<double>(d2[2]) * d1[0] - static_cast<double>(d2[0]) * d1[2]),
        terrain_f32(static_cast<double>(d2[0]) * d1[1] - static_cast<double>(d2[1]) * d1[0])};
    const float xx = terrain_f32(static_cast<double>(c[0]) * c[0]);
    const float yy = terrain_f32(static_cast<double>(c[1]) * c[1]);
    const float zz = terrain_f32(static_cast<double>(c[2]) * c[2]);
    const float sum = terrain_f32(static_cast<double>(xx) + yy + zz);
    const float length = terrain_f32(std::sqrt(static_cast<double>(sum)));
    const float scale = length > 0.0f ? terrain_f32(1.0 / length) : 0.0f;
    for (int k = 0; k < 3; ++k) out[k] = terrain_f32(static_cast<double>(c[k]) * scale);
}

SceneTerrainQueryCensus& scene_terrain_query_census() noexcept {
    static SceneTerrainQueryCensus census;
    return census;
}

namespace {
// The walk 00903860, 009038F0 and 009039D0 share: every node of list 44h
// (head world+34Ch, next +4h), keep the strictly greater height, seeded with
// -1000.0. A Landscape whose height field did not load is skipped: the native
// always has +3D0h.
float terrain_list_walk(const float point[3], int& winner) noexcept {
    float best = kTerrainNoGround;
    winner = -1;
    const SceneWorldClassLists& lists = scene_world_class_lists();
    for (std::size_t index : lists.list(kSceneLandscapeClassId)) {
        const SceneWorldObject& object = lists.objects()[index];
        if (!object.terrain) continue;
        const float h = object.terrain->height_00ada900(point[0], point[2]);
        if (h > best) {
            best = h;
            winner = static_cast<int>(index);
        }
    }
    return best;
}
}  // namespace

// 00903860: *out seeded -1000.0 (00903876), the list walk, then AL = (*out !=
// -1000.0) by FUCOMIP against the double at 00CE6658 (009038C2..009038E0).
bool world_ground_height_00903860(const float point[3], float& out) noexcept {
    SceneTerrainQueryCensus& census = scene_terrain_query_census();
    ++census.height_calls;
    int winner = -1;
    out = terrain_list_walk(point, winner);
    const bool hit = out != kTerrainNoGround;
    ++(hit ? census.height_hits : census.height_fallbacks);
    return hit;
}

// 009038F0: the same walk; when the best is not -1000.0 the winner's slot 38h
// writes the normal to argument 2 (0090397D..009039B1) and AL = 1, else AL = 0
// and the normal is left untouched.
bool world_ground_normal_009038f0(const float point[3], float normal[3]) noexcept {
    SceneTerrainQueryCensus& census = scene_terrain_query_census();
    ++census.normal_calls;
    int winner = -1;
    const float best = terrain_list_walk(point, winner);
    if (best == kTerrainNoGround || winner < 0) return false;
    scene_world_class_lists().objects()[static_cast<std::size_t>(winner)].terrain
        ->normal_00adaba0(point[0], point[2], normal);
    ++census.normal_hits;
    return true;
}

// 009039D0: the same walk; EBP, the winning Landscape, or 0 (009039E1).
int world_landscape_at_009039d0(const float point[3]) noexcept {
    SceneTerrainQueryCensus& census = scene_terrain_query_census();
    ++census.landscape_calls;
    int winner = -1;
    terrain_list_walk(point, winner);
    if (winner >= 0) ++census.landscape_hits;
    return winner;
}

// 00903BC0: blocked when the ground under `from` is above from.y (00903BD8 then
// JBE at 00903BE8), when the ground under `to` is above to.y (00903C08, JA at
// 00903C18), or when any list-44h terrain's slot 3Ch reports the segment.

// ---------------------------------------------------------------------------
// Packet cc9_landscape_attach_scene_half: the Landscape's entry in the segment
// query. Unreferenced until the SegmentBinding hunk lands.

namespace {
std::vector<std::size_t> landscape_entries() {
    std::vector<std::size_t> out;
    const SceneWorldClassLists& lists = scene_world_class_lists();
    for (std::size_t index : lists.list(kSceneLandscapeClassId)) {
        if (lists.objects()[index].terrain) out.push_back(index);
    }
    return out;
}

// Row-vector frame (translation in 12..14), as the scene records store it.
void frame_point(const float m[16], const double p[3], double out[3]) noexcept {
    for (int c = 0; c < 3; ++c) {
        out[c] = p[0] * m[c] + p[1] * m[4 + c] + p[2] * m[8 + c] + m[12 + c];
    }
}

// The inverse of the frame's 3x3 part (general, the frames are not all
// orthonormal: Landscape 05 carries a slight tilt), applied to p - t.
bool frame_inverse_point(const float m[16], const float p[3], double out[3]) noexcept {
    const double a = m[0], b = m[1], c = m[2];
    const double d = m[4], e = m[5], f = m[6];
    const double g = m[8], h = m[9], i = m[10];
    const double det = a * (e * i - f * h) - b * (d * i - f * g) + c * (d * h - e * g);
    if (det == 0.0) return false;
    const double inv[9] = {
        (e * i - f * h) / det, (c * h - b * i) / det, (b * f - c * e) / det,
        (f * g - d * i) / det, (a * i - c * g) / det, (c * d - a * f) / det,
        (d * h - e * g) / det, (b * g - a * h) / det, (a * e - b * d) / det};
    const double q[3] = {static_cast<double>(p[0]) - m[12],
        static_cast<double>(p[1]) - m[13], static_cast<double>(p[2]) - m[14]};
    // Row vector: q = local * M3, so local = q * inverse(M3).
    for (int col = 0; col < 3; ++col) {
        out[col] = q[0] * inv[col] + q[1] * inv[3 + col] + q[2] * inv[6 + col];
    }
    return true;
}

// Packet cc9_terrain_rotation_measure (docs/SCENE_CONTENTS_HOSTS.md section 13):
// 00ADA240 takes both world points to local space through the node's inverse
// world matrix, 00B6E0D0, whose inverse is 00B63B30 (00B6E0F0): each row scaled
// by 1/|row|^2 and transposed, with no shear fallback. It is the true inverse only
// for a frame with mutually orthogonal rows. Then 004142E0 (00ADA262, 00ADA283).
// ON: the segment test uses that inverse. OFF: the general 3x3 inverse above.
// The two differ only on the 5 of 763 authored Landscape frames that are not
// orthonormal (section 13.3). ASSUMPTION, labelled: the node's world matrix +F0h
// is the authored localframe as the host composes it.
// ON since the USN13 / USN01 / USN04 / USN02 pairs: gameplay identical; Landscape
// 07's census row moved as predicted (section 13.5).
constexpr bool kLandscapeScaledTransposeInverseBound = true;

// Packet cc9_terrain_vertical_subwalk (docs/SCENE_CONTENTS_HOSTS.md section 14):
// slot 3Ch's vertical case 00AECC40 answers through its equal-point test (vt+48h
// in tile units) and the sub-walk 00AECA60 (tiles 00AEC7C0, cells 00AEC120).
// OFF: the half-cell march (the labelled stand-in of 10.3).
// ON since the USN13 / USN01 / USN04 / USN02 pairs (section 14.7).
constexpr bool kTerrainVerticalSubwalkBound = true;

bool frame_inverse_point_00b63b30(const float m[16], const float p[3], double out[3]) noexcept {
    bsp::CameraMatrix frame{};
    for (int k = 0; k < 16; ++k) frame[k] = m[k];
    bsp::CameraMatrix inverse{};
    bsp::invert_camera_affine_00b63b30(inverse, frame);          // 00B6E0F0
    const std::array<float, 3> source{p[0], p[1], p[2]};
    std::array<float, 3> local{};
    bsp::transform_point_004142e0(source, inverse, local);      // 00ADA262 / 00ADA283
    for (int c = 0; c < 3; ++c) out[c] = local[c];
    return true;
}

// The height field in the Landscape's local frame. The field answers world
// positions relative to its node translation, so local = world - node, and
// the node y it adds is taken back off. LABELLED: the native slot 3Ch walks the
// samples in local space directly; this reuses the slot 28h sampler.
double local_ground(const SceneTerrainHeightField& field, double lx, double lz) noexcept {
    return static_cast<double>(field.height_00ada900(static_cast<float>(lx + field.node_x),
        static_cast<float>(lz + field.node_z))) - field.node_y;
}
}  // namespace

// ---------------------------------------------------------------------------
// Packet cc9_terrain_segment_quadtree: terrain slot 3Ch 00ADA240's walk.
// Rounding: every value the native stores through a float is kept as a float;
// x87 chains the native keeps in registers are evaluated in double. Not
// bit-verified against the image.
namespace {
constexpr float kQuadTileSize = 300.0f;        // terrain +18h [00CE3AE8]
constexpr float kQuadRangeSeed = 1000.0f;      // [00CE3804]
constexpr float kQuadMinDirection = 0.001f;    // [00D7A23C]
constexpr double kQuadLeafMinLengthSq = 1e-6;  // [00D7A2B8] double
constexpr float kQuadTreeTop = 1e10f;          // terrain +14h [00CE4970]

// Block slot 8h 00ADC5F0: one sample, no node y; FFFFh answers -1000.0.
float block_sample_00adc5f0(const SceneTerrainBlock& block, int x, int z) noexcept {
    const std::uint16_t s = block.samples[static_cast<std::size_t>(33 * z + x)];
    if (s == 0xffffu) return kTerrainNoGround;
    return terrain_f32(s * static_cast<double>(block.inv_scale) + block.offset);
}

// The tile table +40h[+38h * z + x], null outside the grid (00ADAC30 is +38h,
// 00ADAC40 +3Ch) or for a tile the file does not carry.
const SceneTerrainBlock* quad_tile(const SceneTerrainHeightField& field, int x, int z) noexcept {
    if (x < 0 || z < 0 || x >= field.tiles_wide || z >= field.tiles_deep) return nullptr;
    const int index = field.block_index[static_cast<std::size_t>(field.tiles_wide) * z + x];
    return index < 0 ? nullptr : &field.blocks[static_cast<std::size_t>(index)];
}

// Block vt+0Ch 00AED020 (from sample (0,0), over all 33 x 33 samples, holes
// included), stored by 00ADF8B0 at tile +14h (min, 00ADF90A) / +18h (max).
void tile_range_00aed020(const SceneTerrainBlock& block, float& lo, float& hi) noexcept {
    lo = hi = block_sample_00adc5f0(block, 0, 0);
    for (int x = 0; x < 33; ++x) {
        for (int z = 0; z < 33; ++z) {
            const float v = block_sample_00adc5f0(block, x, z);
            if (v < lo) lo = v;
            if (hi < v) hi = v;
        }
    }
}

// 00AE9C80: the y range of the n x n tiles at (x, z), seeded [1000, -1000];
// a tile outside the grid or missing contributes [-1000, 1000].
void node_range_00ae9c80(const SceneTerrainHeightField& field, int x, int z, int n,
                         float& lo, float& hi) noexcept {
    lo = kQuadRangeSeed;
    hi = kTerrainNoGround;
    for (int dx = 0; dx < n; ++dx) {
        for (int k = 0; k < n; ++k) {
            float tile_hi = kQuadRangeSeed;
            float tile_lo = kTerrainNoGround;
            if (const SceneTerrainBlock* block = quad_tile(field, x + dx, z + k)) {
                tile_range_00aed020(*block, tile_lo, tile_hi);
            }
            if (hi < tile_hi) hi = tile_hi;
            if (tile_lo < lo) lo = tile_lo;
        }
    }
}

// 00AEA5F0: four children of the n x n block at (x, z) appended in the order
// (x,z), (x,z+h), (x+h,z), (x+h,z+h) with tile +0Ch = (k >> 1) + x and +10h =
// (k & 1) + z, then each recursed while h > 1. Returns the first child's index.
int build_children_00aea5f0(const SceneTerrainHeightField& field, int x, int z, int n) {
    std::vector<SceneTerrainHeightField::QuadNode>& nodes = field.quadtree;
    const int base = static_cast<int>(nodes.size());
    const int h = n / 2;
    for (int k = 0; k < 4; ++k) {
        SceneTerrainHeightField::QuadNode node;
        node_range_00ae9c80(field, x + (k >> 1) * h, z + (k & 1) * h, h, node.min_y, node.max_y);
        node.children = -1;
        node.tile_x = (k >> 1) + x;
        node.tile_z = (k & 1) + z;
        nodes.push_back(node);
    }
    if (1 < h) {
        for (int k = 0; k < 4; ++k) {
            const int first = build_children_00aea5f0(field, x + (k >> 1) * h, z + (k & 1) * h, h);
            nodes[static_cast<std::size_t>(base + k)].children = first;
        }
    }
    return base;
}

// 00ADDA60's tail: depth = ceil(log2(max(+38h, +3Ch))) (unsigned max), then
// 00AEA900 (box (0, -1000, 0)..(2^depth * 300, 1e10, 2^depth * 300)) and
// 00AEA820 (the root, then its children).
void build_quadtree_00aea820(const SceneTerrainHeightField& field) {
    const unsigned wide = static_cast<unsigned>(field.tiles_wide);
    const unsigned deep = static_cast<unsigned>(field.tiles_deep);
    const int n = static_cast<int>(wide <= deep ? deep : wide);
    int depth = 0;
    for (int u = n; 1 < u; u >>= 1) ++depth;
    if ((1 << depth) < n) ++depth;
    field.quadtree_leaves = 1 << depth;
    field.quadtree.clear();
    SceneTerrainHeightField::QuadNode root;
    node_range_00ae9c80(field, 0, 0, field.quadtree_leaves, root.min_y, root.max_y);
    root.children = 0;
    root.tile_x = 0;
    root.tile_z = 0;
    field.quadtree.push_back(root);
    const int first = build_children_00aea5f0(field, 0, 0, field.quadtree_leaves);
    field.quadtree[0].children = first;
}

// 004F9B30: out.x = a.y*b.z - a.z*b.y, out.y = b.x*a.z - a.x*b.z,
// out.z = b.y*a.x - a.y*b.x.
void quad_cross(const float a[3], const float b[3], float out[3]) noexcept {
    out[0] = terrain_f32(static_cast<double>(a[1]) * b[2] - static_cast<double>(a[2]) * b[1]);
    out[1] = terrain_f32(static_cast<double>(b[0]) * a[2] - static_cast<double>(a[0]) * b[2]);
    out[2] = terrain_f32(static_cast<double>(b[1]) * a[0] - static_cast<double>(a[1]) * b[0]);
}

float quad_dot(const float a[3], const float b[3]) noexcept {
    return terrain_f32(static_cast<double>(a[0]) * b[0] + static_cast<double>(a[1]) * b[1]
        + static_cast<double>(a[2]) * b[2]);
}

// 00ADEB40: u . (v x w), stored through a float.
float scalar_triple_00adeb40(const float u[3], const float v[3], const float w[3]) noexcept {
    float c[3];
    quad_cross(v, w, c);
    return quad_dot(c, u);
}

// 00ADEB80 (fastcall a = ECX, b = EDX; c, d, p, q, out): the LINE through p
// and q against the quad a b c d, split on the b-d diagonal (m = pb x pq,
// 00ADEC6A). No clip to [p, q]: a hit beyond the segment inside the cell counts.
bool line_quad_00adeb80(const float a[3], const float b[3], const float c[3], const float d[3],
                        const float p[3], const float q[3], float r[3]) noexcept {
    float pq[3], pd[3], pb[3], pa[3], pc[3];
    for (int k = 0; k < 3; ++k) {
        pq[k] = terrain_f32(static_cast<double>(q[k]) - p[k]);
        pd[k] = terrain_f32(static_cast<double>(d[k]) - p[k]);
        pb[k] = terrain_f32(static_cast<double>(b[k]) - p[k]);
        pa[k] = terrain_f32(static_cast<double>(a[k]) - p[k]);
        pc[k] = terrain_f32(static_cast<double>(c[k]) - p[k]);
    }
    float m[3];
    quad_cross(pb, pq, m);
    const float v = quad_dot(m, pd);
    if (v < 0.0f) {
        const float u = quad_dot(m, pa);
        if (!(0.0f <= u)) return false;
        const float w = scalar_triple_00adeb40(pq, pd, pa);
        if (!(0.0f <= w)) return false;
        const double denom = 1.0 / (static_cast<double>(w) + -static_cast<double>(v) + u);
        const double cu = denom * u, cv = -static_cast<double>(v) * denom, cw = denom * w;
        for (int k = 0; k < 3; ++k) r[k] = terrain_f32(cu * d[k] + cv * a[k] + cw * b[k]);
        return true;
    }
    const float u = -quad_dot(m, pc);
    if (!(0.0f <= u)) return false;
    const float w = scalar_triple_00adeb40(pq, pc, pd);
    if (!(0.0f <= w)) return false;
    const double denom = 1.0 / (static_cast<double>(w) + v + u);
    const double cu = u * denom, cv = v * denom, cw = denom * w;
    for (int k = 0; k < 3; ++k) r[k] = terrain_f32(cu * d[k] + cv * c[k] + cw * b[k]);
    return true;
}

// 00ADF1B0: the 2-D DDA over the tile's 32 x 32 cells of 9.375 from p to q
// (origin-relative local), each cell's four samples (slot 8h, no node y) as
// the quad (i,j) (i+1,j) (i+1,j+1) (i,j+1) at x = tile*300 + i*9.375.
bool tile_walk_00adf1b0(const SceneTerrainBlock& block, int tile_x, int tile_z, const float p[3],
                        const float q[3], float out[3]) noexcept {
    SceneTerrainQuadtreeCensus& census = scene_terrain_quadtree_census();
    const double cell = kTerrainCellSize;               // [00D5D410] double
    const float ox = terrain_f32(static_cast<double>(tile_x) * 300.0);  // [00CE3CA8]
    const float oz = terrain_f32(300.0 * static_cast<double>(tile_z));
    const float lx0 = terrain_f32(static_cast<double>(p[0]) - ox);
    const float lx1 = terrain_f32(static_cast<double>(q[0]) - ox);
    const float lz0 = terrain_f32(static_cast<double>(p[2]) - oz);
    const float lz1 = terrain_f32(static_cast<double>(q[2]) - oz);
    const float cellf = terrain_f32(cell);
    const double fx0 = std::floor(static_cast<double>(terrain_f32(lx0 / cell)));   // 00BF85B0
    const double fz0 = std::floor(static_cast<double>(terrain_f32(lz0 / cell)));
    int i = static_cast<int>(fx0);
    int j = static_cast<int>(fz0);
    const int i1 = static_cast<int>(std::floor(static_cast<double>(terrain_f32(lx1 / cell))));  // 00BF7420
    const int j1 = static_cast<int>(std::floor(static_cast<double>(terrain_f32(lz1 / cell))));
    const int step_x = lx1 <= lx0 ? -1 : 1;
    const int step_z = lz1 <= lz0 ? -1 : 1;
    const float cell_x = terrain_f32(static_cast<double>(terrain_f32(fx0)) * cellf);
    const float cell_z = terrain_f32(static_cast<double>(terrain_f32(fz0)) * cellf);
    float tx = step_x == -1
        ? terrain_f32((static_cast<double>(lx0) - cell_x) / (static_cast<double>(lx0) - lx1))
        : terrain_f32((static_cast<double>(cell_x) + cellf - lx0) / (static_cast<double>(lx1) - lx0));
    float tz = step_z == -1
        ? terrain_f32((static_cast<double>(lz0) - cell_z) / (static_cast<double>(lz0) - lz1))
        : terrain_f32((static_cast<double>(cell_z) + cellf - lz0) / (static_cast<double>(lz1) - lz0));
    float adx = terrain_f32(static_cast<double>(lx1) - lx0);
    if (adx <= 0.0f) adx = -adx;
    float adz = terrain_f32(static_cast<double>(lz1) - lz0);
    if (adz <= 0.0f) adz = -adz;
    const float dtx = terrain_f32(cellf / static_cast<double>(adx));
    const float dtz = terrain_f32(cellf / static_cast<double>(adz));
    const float x_base = static_cast<float>(tile_x * 300);   // IMUL 12Ch, FILD
    const float z_base = static_cast<float>(tile_z * 300);
    for (;;) {
        if (static_cast<unsigned>(i) < 32u && static_cast<unsigned>(j) < 32u) {
            ++census.cells;
            const float xa = terrain_f32(x_base + static_cast<double>(i) * cell);
            const float xb = terrain_f32(x_base + static_cast<double>(i + 1) * cell);
            const float za = terrain_f32(z_base + static_cast<double>(j) * cell);
            const float zb = terrain_f32(z_base + static_cast<double>(j + 1) * cell);
            const float a[3] = {xa, block_sample_00adc5f0(block, i, j), za};
            const float b[3] = {xb, block_sample_00adc5f0(block, i + 1, j), za};
            const float c[3] = {xb, block_sample_00adc5f0(block, i + 1, j + 1), zb};
            const float d[3] = {xa, block_sample_00adc5f0(block, i, j + 1), zb};
            if (line_quad_00adeb80(a, b, c, d, p, q, out)) return true;
        }
        // 00ADF553..00ADF55D: FCOMI tz against tx, JB (below or unordered)
        // steps z; otherwise x.
        if (!(tz >= tx)) {
            if (j == j1) return false;
            tz = terrain_f32(static_cast<double>(tz) + dtz);
            j += step_z;
        } else {
            if (i == i1) return false;
            tx = terrain_f32(static_cast<double>(tx) + dtx);
            i += step_x;
        }
    }
}

struct QuadRay {
    float origin[3]{};     // +0h, x and z mirrored under the flags
    float direction[3]{};  // +18h, x and z made positive
    float hit[3]{};        // +24h
    float length{0.0f};    // +30h
    unsigned flags{0};     // +34h: 4 x mirrored, 2 z mirrored
};

struct QuadBox {
    float min_x{0.0f}, min_z{0.0f}, max_x{0.0f}, max_z{0.0f};
};

// 00AE9BD0: the leaf's tile (+0Ch, +10h), in the grid and present, and a
// sub-segment of squared length at least 1e-6 (004193E0), then 00ADF1B0.
bool leaf_00ae9bd0(const SceneTerrainHeightField& field, int tile_x, int tile_z,
                   const float start[3], const float end[3], float out[3]) noexcept {
    if (!(tile_x < field.tiles_wide) || !(tile_z < field.tiles_deep)) return false;
    const SceneTerrainBlock* block = quad_tile(field, tile_x, tile_z);
    if (block == nullptr) return false;
    float diff[3];
    for (int k = 0; k < 3; ++k) diff[k] = terrain_f32(static_cast<double>(start[k]) - end[k]);
    const double length_sq = static_cast<double>(diff[1]) * diff[1]
        + static_cast<double>(diff[0]) * diff[0] + static_cast<double>(diff[2]) * diff[2];
    if (length_sq < kQuadLeafMinLengthSq) return false;
    ++scene_terrain_quadtree_census().leaves;
    return tile_walk_00adf1b0(*block, tile_x, tile_z, start, end, out);
}

// 00AE9A00: (f1 <= f2) ? a : b (FCOMIP f2 against f1, JAE).
int next_node_00ae9a00(float f1, float f2, int a, int b) noexcept {
    return f2 >= f1 ? a : b;
}

// 00AE9D80 (__thiscall tree, RET 18h): 1 hit, 0 go on, -1 the ray has ended.
int node_walk_00ae9d80(const SceneTerrainHeightField& field, const QuadBox& box, QuadRay& ray,
                       int node_index, float tx0, float tz0, float tx1, float tz1) noexcept {
    if (0.0f >= tx1 || 0.0f >= tz1) return 0;        // 00AE9D8C / 00AE9D9E COMISS, JAE
    float t_in = !(tx0 >= tz0) ? tz0 : tx0;          // 00AE9DAF FCOMI, JB
    if (0.0f > t_in) t_in = 0.0f;
    float t_out = !(tz1 >= tx1) ? tz1 : tx1;         // 00AE9DDA FCOMPI, JB
    if (t_out > ray.length) t_out = ray.length;      // 00AE9E02, JBE keeps
    if (t_in >= t_out) return -1;                    // 00AE9E1E, JB (or unordered) goes on
    float sx = ray.origin[0], sy = ray.origin[1], sz = ray.origin[2];
    float dx = ray.direction[0], dy = ray.direction[1], dz = ray.direction[2];
    if ((ray.flags & 4u) != 0) {
        dx = -dx;
        sx = terrain_f32(static_cast<double>(box.max_x) + box.min_x - sx);
    }
    if ((ray.flags & 2u) != 0) {
        dz = -ray.direction[2];
        sz = terrain_f32(static_cast<double>(box.max_z) + box.min_z - sz);
    }
    const float y_in = terrain_f32(sy + static_cast<double>(terrain_f32(static_cast<double>(dy) * t_in)));
    const float y_out = terrain_f32(static_cast<double>(t_out) * dy + sy);
    const float y_lo = y_in < y_out ? y_in : y_out;
    const SceneTerrainHeightField::QuadNode& node = field.quadtree[static_cast<std::size_t>(node_index)];
    if (!(y_lo <= node.max_y)) return 0;
    const float y_hi = y_out < y_in ? y_in : y_out;
    if (!(node.min_y <= y_hi)) return 0;
    if (node.children == -1) {
        const float start[3] = {terrain_f32(sx + static_cast<double>(terrain_f32(static_cast<double>(t_in) * dx))),
            y_in, terrain_f32(sz + static_cast<double>(terrain_f32(static_cast<double>(dz) * t_in)))};
        const float end[3] = {terrain_f32(sx + static_cast<double>(dx) * t_out), y_out,
            terrain_f32(sz + static_cast<double>(dz) * t_out)};
        return leaf_00ae9bd0(field, node.tile_x, node.tile_z, start, end, ray.hit) ? 1 : 0;
    }
    const float txm = terrain_f32((static_cast<double>(tx1) + tx0) * 0.5);   // [00D7A280]
    const float tzm = terrain_f32((static_cast<double>(tz1) + tz0) * 0.5);
    int current = 0;
    if (tx0 < tz0) {
        if (txm < tz0) current = 4;
    } else if (tzm < tx0) {
        current = 2;
    }
    const int first_child = node.children;
    auto child = [&](unsigned k) {
        return static_cast<int>((ray.flags ^ k) >> 1) + first_child;
    };
    for (;;) {
        int result = 0;
        int next = 8;
        switch (current) {
        case 0:
            result = node_walk_00ae9d80(field, box, ray, child(0), tx0, tz0, txm, tzm);
            if (result == 1) return 1;
            if (result == -1) return -1;
            next = next_node_00ae9a00(txm, tzm, 4, 2);
            break;
        case 2:
            result = node_walk_00ae9d80(field, box, ray, child(2), tx0, tzm, txm, tz1);
            if (result == 1) return 1;
            if (result == -1) return -1;
            next = next_node_00ae9a00(txm, tz1, 6, 8);
            break;
        case 4:
            result = node_walk_00ae9d80(field, box, ray, child(4), txm, tz0, tx1, tzm);
            if (result == 1) return 1;
            if (result == -1) return -1;
            next = next_node_00ae9a00(tx1, tzm, 8, 6);
            break;
        case 6:
            result = node_walk_00ae9d80(field, box, ray, child(6), txm, tzm, tx1, tz1);
            if (result == 1) return 1;
            return result == -1 ? -1 : 0;
        default:
            return 0;   // table arms 1, 3, 5 (00AEA201); never reached
        }
        if (next > 7) return 0;
        current = next;
    }
}

// 00AEA2B0 (__thiscall tree; p, q origin-relative local, out), RET 0Ch.
bool ray_walk_00aea2b0(const SceneTerrainHeightField& field, const float p[3], const float q[3],
                       float out[3]) noexcept {
    const float span = terrain_f32(static_cast<double>(field.quadtree_leaves) * kQuadTileSize);
    const QuadBox box{0.0f, 0.0f, span, span};
    float diff[3];
    for (int k = 0; k < 3; ++k) diff[k] = terrain_f32(static_cast<double>(q[k]) - p[k]);
    const float length = terrain_f32(std::sqrt(static_cast<double>(diff[0]) * diff[0]
        + static_cast<double>(diff[1]) * diff[1] + static_cast<double>(diff[2]) * diff[2]));  // 00419440
    QuadRay ray;
    ray.length = length;
    ray.origin[0] = p[0];
    ray.origin[1] = p[1];
    ray.origin[2] = p[2];
    for (int k = 0; k < 3; ++k) {
        ray.direction[k] = terrain_f32((static_cast<double>(q[k]) - p[k]) / length);
    }
    if (ray.direction[0] < 0.0f) {
        ray.direction[0] = -ray.direction[0];   // [00D7A208] -0.0 minus
        ray.flags = 4u;
        ray.origin[0] = terrain_f32(static_cast<double>(box.max_x) + box.min_x - p[0]);
    }
    if (ray.direction[2] < 0.0f) {
        ray.flags |= 2u;
        ray.direction[2] = -ray.direction[2];
        ray.origin[2] = terrain_f32(static_cast<double>(box.max_z) + box.min_z - ray.origin[2]);
    }
    if (ray.direction[0] < kQuadMinDirection) ray.direction[0] = kQuadMinDirection;
    if (ray.direction[2] < kQuadMinDirection) ray.direction[2] = kQuadMinDirection;
    const float tx0 = terrain_f32((static_cast<double>(box.min_x) - ray.origin[0]) / ray.direction[0]);
    const float tx1 = terrain_f32((static_cast<double>(box.max_x) - ray.origin[0]) / ray.direction[0]);
    const float tz0 = terrain_f32((static_cast<double>(box.min_z) - ray.origin[2]) / ray.direction[2]);
    const float tz1 = terrain_f32((static_cast<double>(box.max_z) - ray.origin[2]) / ray.direction[2]);
    const float t_lo = tz0 < tx0 ? tx0 : tz0;
    const float t_hi = tx1 < tz1 ? tx1 : tz1;
    if (t_hi <= t_lo) return false;
    const int result = node_walk_00ae9d80(field, box, ray, 0, tx0, tz0, tx1, tz1);
    out[0] = ray.hit[0];
    out[1] = ray.hit[1];
    out[2] = ray.hit[2];
    return result == 1;
}
}  // namespace

SceneTerrainQuadtreeCensus& scene_terrain_quadtree_census() noexcept {
    static SceneTerrainQuadtreeCensus census;
    return census;
}

std::size_t landscape_segment_entry_count() noexcept {
    return landscape_entries().size();
}

int landscape_segment_entry_object(std::size_t entry) noexcept {
    const std::vector<std::size_t> entries = landscape_entries();
    return entry < entries.size() ? static_cast<int>(entries[entry]) : -1;
}

bool landscape_segment_entry_bounds(std::size_t entry, float box_min[3],
                                    float box_max[3]) noexcept {
    const int object_index = landscape_segment_entry_object(entry);
    if (object_index < 0) return false;
    const SceneWorldObject& object
        = scene_world_class_lists().objects()[static_cast<std::size_t>(object_index)];
    const SceneTerrainHeightField& field = *object.terrain;
    const double lo[3] = {field.origin_x, field.sample_min, field.origin_z};
    const double hi[3] = {field.origin_x + field.tiles_wide * 300.0, field.sample_max,
        field.origin_z + field.tiles_deep * 300.0};
    for (int k = 0; k < 3; ++k) { box_min[k] = 1e30f; box_max[k] = -1e30f; }
    for (int corner = 0; corner < 8; ++corner) {
        const double p[3] = {(corner & 1) ? hi[0] : lo[0], (corner & 2) ? hi[1] : lo[1],
            (corner & 4) ? hi[2] : lo[2]};
        double w[3];
        frame_point(object.world, p, w);
        for (int k = 0; k < 3; ++k) {
            box_min[k] = std::min(box_min[k], static_cast<float>(w[k]));
            box_max[k] = std::max(box_max[k], static_cast<float>(w[k]));
        }
    }
    return true;
}

namespace {
// ---------------------------------------------------------------------------
// Packet cc9_terrain_vertical_subwalk (docs/SCENE_CONTENTS_HOSTS.md section 14):
// slot 3Ch's vertical case 00AECC40 and its sub-walk 00AECA60, in place of the
// half-cell march. Stored floats are kept as floats; x87 register chains are
// evaluated in double (not bit-verified). The walk is in TILE units (1/300 of the
// local frame after the origin) and, inside a tile, in CELL units (x 32).

constexpr float kVsubCells = 32.0f;      // [00D5D658], int 32 in .rdata
constexpr double kVsubEps = 0.001;       // [00CF3F30] double
constexpr float kVsubClipEps = 1e-8f;    // [00CF7FE8] float

// The two-axis grid iterator 00AEB430 builds on the stack (00AEC7C0 at
// ESP+14h, 00AEC120 at ESP+18h); offsets are the image's.
struct VsubIter {
    float x0{0}, z0{0}, x1{0}, z1{0};     // +0 .. +Ch
    float dx{0}, dz{0};                   // +10h, +14h
    float tdx{0}, tdz{0};                 // +18h, +1Ch  t per unit step
    float tmx0{0}, tmz0{0};               // +20h, +24h  first crossing
    float dirx{0}, dirz{0};               // +28h, +2Ch
    float len{0};                         // +30h
    float t{0};                           // +34h  t at the last crossing
    float tmx{0}, tmz{0};                 // +38h, +3Ch  next crossing
    int cx{0}, cz{0};                     // +40h, +44h  steps taken
    float px{0}, pz{0};                   // +48h, +4Ch  current point
    float nx{0}, nz{0};                   // +50h, +54h  next point
    int ci{0}, cj{0};                     // +58h, +5Ch  cell left behind
    int e0i{0}, e0j{0}, e1i{0}, e1j{0};   // +60h .. +6Ch the crossed edge
    float frac{0};                        // +70h  position along that edge
    int flags{0};                         // +74h  00AEAB30's snap bits
};

int vsub_trunc(double v) noexcept { return static_cast<int>(v); }   // 00BF7420, CVTTSS2SI

// 00AEB430 (fastcall ECX = iterator, plain RET).
void vsub_iter_init_00aeb430(VsubIter& it) noexcept {
    it.dx = terrain_f32(static_cast<double>(it.x1) - it.x0);
    it.dz = terrain_f32(static_cast<double>(it.z1) - it.z0);
    const float xx = terrain_f32(static_cast<double>(it.dx) * it.dx);
    const float zz = terrain_f32(static_cast<double>(it.dz) * it.dz);
    it.len = terrain_f32(std::sqrt(static_cast<double>(terrain_f32(static_cast<double>(xx) + zz))));
    // 00419260: the same squared sum, sqrt, then 1/length. LABELLED: its zero
    // test is taken as "0 when the length is not positive".
    const float r = it.len > 0.0f ? terrain_f32(1.0 / it.len) : 0.0f;
    it.dirx = terrain_f32(static_cast<double>(it.dx) * r);
    it.dirz = terrain_f32(static_cast<double>(r) * it.dz);
    if (std::fabs(it.dirx) > kQuadMinDirection) {
        const float a = terrain_f32(static_cast<double>(it.dirz) / it.dirx);
        it.tdx = terrain_f32(std::sqrt(static_cast<double>(terrain_f32(static_cast<double>(a) * a + 1.0))));
        double f = terrain_f32(std::fmod(static_cast<double>(it.x0), 1.0));   // 00BF857A
        if (!(0.0f > it.dirx)) f = 1.0 - f;
        it.tmx0 = terrain_f32(f * it.tdx);
    } else {
        it.tdx = terrain_f32(static_cast<double>(it.len) + it.len);
        it.tmx0 = it.tdx;
    }
    if (std::fabs(it.dirz) > kQuadMinDirection) {
        const float b = terrain_f32(static_cast<double>(it.dirx) / it.dirz);
        it.tdz = terrain_f32(std::sqrt(static_cast<double>(terrain_f32(static_cast<double>(b) * b + 1.0))));
        double f = terrain_f32(std::fmod(static_cast<double>(it.z0), 1.0));
        if (!(0.0f > it.dirz)) f = 1.0 - f;
        it.tmz0 = terrain_f32(f * it.tdz);
    } else {
        it.tdz = terrain_f32(static_cast<double>(it.len) + it.len);
        it.tmz0 = it.tdz;
    }
}

// The start the two walks give the iterator after 00AEB430 (00AEC828..00AEC85B,
// 00AEC188..00AEC1CE).
void vsub_iter_start(VsubIter& it) noexcept {
    it.t = 0.0f;
    it.tmx = it.tmx0;
    it.tmz = it.tmz0;
    it.cx = it.cz = 0;
    it.px = it.x0;
    it.pz = it.z0;
}

// 00AEB680: false when the segment ends before its first crossing.
bool vsub_iter_crosses_00aeb680(const VsubIter& it) noexcept {
    float m = it.tmz;
    if (it.tmx < m) m = it.tmx;
    return !(it.len < m);
}

// 00AEAB30: a start within 0.001 of a grid line is snapped onto it.
void vsub_iter_snap_00aeab30(VsubIter& it) noexcept {
    it.flags = 0;
    if (it.tmx < kVsubEps) {
        it.px = static_cast<float>(vsub_trunc(static_cast<double>(it.px) + 0.5));   // [00D7A280]
        ++it.cx;
        it.flags += 1;
        it.tmx = terrain_f32(static_cast<double>(it.tdx) * it.cx + it.tmx0);
    }
    if (it.tmz < kVsubEps) {
        it.pz = static_cast<float>(vsub_trunc(static_cast<double>(it.pz) + 0.5));
        ++it.cz;
        it.flags += 2;
        it.tmz = terrain_f32(static_cast<double>(it.tdz) * it.cz + it.tmz0);
    }
}

void vsub_iter_point(VsubIter& it, float t) noexcept {
    it.nx = terrain_f32(static_cast<double>(it.x0) + terrain_f32(static_cast<double>(t) * it.dirx));
    it.nz = terrain_f32(static_cast<double>(it.z0) + terrain_f32(static_cast<double>(t) * it.dirz));
}

// 00AEB1F0, a step across an x line.
void vsub_iter_step_x_00aeb1f0(VsubIter& it) noexcept {
    const float t = it.tmx;
    vsub_iter_point(it, t);
    it.nx = static_cast<float>(vsub_trunc(static_cast<double>(it.nx) + 0.5));
    const float pz = it.nz;
    const int k = static_cast<int>(std::floor(static_cast<double>(pz)));   // 00BF85B0, FISTP
    const int k2 = static_cast<double>(k) == static_cast<double>(pz) ? k : k + 1;
    const int ix = static_cast<int>(it.nx);
    it.e0i = ix; it.e0j = k; it.e1i = ix; it.e1j = k2;
    it.frac = terrain_f32(std::fmod(static_cast<double>(pz), 1.0));
    const int cur = static_cast<int>(it.px);
    it.ci = cur < ix ? cur : ix;
    it.cj = k;
    ++it.cx;
    it.t = t;
    it.tmx = terrain_f32(static_cast<double>(it.tdx) * it.cx + it.tmx0);
}

// 00AEB310, a step across a z line.
void vsub_iter_step_z_00aeb310(VsubIter& it) noexcept {
    const float t = it.tmz;
    vsub_iter_point(it, t);
    it.nz = static_cast<float>(vsub_trunc(static_cast<double>(it.nz) + 0.5));
    const float px = it.nx;
    const int k = static_cast<int>(std::floor(static_cast<double>(px)));
    const int k2 = static_cast<double>(k) == static_cast<double>(px) ? k : k + 1;
    const int iz = static_cast<int>(it.nz);
    it.e0i = k; it.e0j = iz; it.e1i = k2; it.e1j = iz;
    it.frac = terrain_f32(std::fmod(static_cast<double>(px), 1.0));
    const int cur = static_cast<int>(it.pz);
    it.ci = k;
    it.cj = cur < iz ? cur : iz;
    ++it.cz;
    it.t = t;
    it.tmz = terrain_f32(static_cast<double>(it.tdz) * it.cz + it.tmz0);
}

// 00AEB0B0, a step through a grid corner (both crossings within 0.001).
void vsub_iter_step_xz_00aeb0b0(VsubIter& it) noexcept {
    const float t = it.tmz > it.tmx ? it.tmz : it.tmx;
    ++it.cz;
    ++it.cx;
    it.t = t;
    it.tmz = terrain_f32(static_cast<double>(it.tdz) * it.cz + it.tmz0);
    it.tmx = terrain_f32(static_cast<double>(it.tdx) * it.cx + it.tmx0);
    vsub_iter_point(it, t);
    it.nx = static_cast<float>(vsub_trunc(static_cast<double>(it.nx) + 0.5));
    it.nz = static_cast<float>(vsub_trunc(static_cast<double>(it.nz) + 0.5));
    const int a = vsub_trunc(static_cast<double>(it.nx) + 0.5);
    const int b = vsub_trunc(static_cast<double>(it.nz) + 0.5);
    it.e0i = a; it.e1i = a; it.e0j = b; it.e1j = b;   // +70h is left as it was
    const int cz = static_cast<int>(it.pz);
    it.cj = cz < b ? cz : b;
    const int cx = static_cast<int>(it.px);
    it.ci = cx < a ? cx : a;
}

// 00AEB6D0.
void vsub_iter_step_00aeb6d0(VsubIter& it) noexcept {
    if (std::fabs(static_cast<double>(it.tmx) - it.tmz) < kVsubEps) {
        vsub_iter_step_xz_00aeb0b0(it);
    } else if (it.tmx < it.tmz) {
        vsub_iter_step_x_00aeb1f0(it);
    } else {
        vsub_iter_step_z_00aeb310(it);
    }
}

// 00AEBA00 (lower bound) and 00AEBB90 (upper bound), __thiscall(segment
// {x0,y0,z0,x1,y1,z1}; value, axis 0 or 2, and an unused third dword), RET 0Ch.
bool vsub_clip_00aeba00(float s[6], float v, int axis, bool upper) noexcept {
    const float p0 = s[axis];
    const float p1 = s[axis + 3];
    if (upper) {
        if (v < p0 && v < p1) return false;
        if (p0 <= v && p1 <= v) return true;
    } else {
        if (p0 < v && p1 < v) return false;
        if (v <= p0 && v <= p1) return true;
    }
    const float d[3] = {s[3] - s[0], s[4] - s[1], s[5] - s[2]};
    float m = d[axis];
    if (m <= 0.0f) m = -0.0f - m;                              // [00D7A218] [00D7A208]
    if (!(kVsubClipEps <= m)) return true;
    const bool move_end = upper ? (p0 <= v) : (v <= p0);
    if (move_end) {
        const float f = (p1 - v) / d[axis];
        for (int k = 0; k < 3; ++k) s[3 + k] = s[3 + k] - f * d[k];
        s[axis + 3] = v;
    } else {
        const float f = (v - p0) / d[axis];
        for (int k = 0; k < 3; ++k) s[k] = s[k] + f * d[k];
        s[axis] = v;
    }
    return true;
}

// 00AEBD20 (thiscall segment; min pair, max pair), RET 8.
bool vsub_clip_00aebd20(float s[6], const float lo[2], const float hi[2]) noexcept {
    if (!vsub_clip_00aeba00(s, lo[0], 0, false)) return false;
    if (!vsub_clip_00aeba00(s, hi[0], 0, true)) return false;
    if (!vsub_clip_00aeba00(s, lo[1], 2, false)) return false;
    return vsub_clip_00aeba00(s, hi[1], 2, true);
}

// 00AEB7E0 / 00AEB890: dy (double) over the 2-D length (float).
float vsub_slope(float x0, float z0, float y0, float x1, float z1, float y1) noexcept {
    const float dx = terrain_f32(static_cast<double>(x1) - x0);
    const float dz = terrain_f32(static_cast<double>(z1) - z0);
    const double dy = static_cast<double>(y1) - y0;
    const float xx = terrain_f32(static_cast<double>(dx) * dx);
    const float zz = terrain_f32(static_cast<double>(dz) * dz);
    const float len = terrain_f32(std::sqrt(static_cast<double>(terrain_f32(static_cast<double>(xx) + zz))));
    return terrain_f32(dy / len);
}

// The cell walker 00AEC3F0 builds (48h bytes copied by 00AEAE70); only the
// fields the cell tests read are kept.
struct VsubCellWalker {
    const SceneTerrainBlock* block{nullptr};   // +0h (its +1Ch sampler, 00ADEA70)
    float node_y{0};                           // +4h
    float ax{0}, az{0};                        // +8h, +Ch   piece start
    float bx{0}, bz{0};                        // +10h, +14h piece end
    int e0i{0}, e0j{0}, e1i{0}, e1j{0};        // +18h .. +24h
    float frac{0};                             // +28h
    float ha{0}, hb{0};                        // +2Ch, +30h ground at the ends
    float ya{0}, yb{0};                        // +34h, +38h segment y at the ends
    float hit[3]{};                            // +3Ch .. +44h
    float sx{0}, sz{0}, ex{0}, ez{0};          // +60h .. +6Ch (2-D segment)
    float y0{0}, y1{0}, slope{0};              // +70h, +74h, +78h
};

float vsub_sample(const VsubCellWalker& w, int i, int j) noexcept {
    // vt+8h of block+1Ch (00ADC5F0) plus the node y, stored float.
    return terrain_f32(static_cast<double>(block_sample_00adc5f0(*w.block, i, j)) + w.node_y);
}

// 00AEBDE0 (thiscall walker; point), RET 4: bilinear ground at a cell point.
float vsub_ground_00aebde0(const VsubCellWalker& w, float x, float z) noexcept {
    const int i = static_cast<int>(x);
    const float fx = terrain_f32(static_cast<double>(x) - i);
    const int j = static_cast<int>(z);
    const float fz = terrain_f32(static_cast<double>(z) - j);
    const int i1 = i == 32 ? i : i + 1;
    const int j1 = j == 32 ? j : j + 1;
    const float s11 = vsub_sample(w, i1, j1);
    const float s01 = vsub_sample(w, i, j1);
    const float s10 = vsub_sample(w, i1, j);
    const float s00 = vsub_sample(w, i, j);
    const float a = terrain_f32(s00 + (static_cast<double>(s10) - s00) * fx);
    const float b = terrain_f32(s01 + static_cast<double>(fx) * (static_cast<double>(s11) - s01));
    return terrain_f32(a + (static_cast<double>(b) - a) * fz);
}

// 00AEAAA0 (ECX = &t; h0, y0, h1, y1), RET 10h.
bool vsub_cross_00aeaaa0(float h0, float y0, float h1, float y1, float& t) noexcept {
    if (y0 > h0 && y1 > h1) return false;
    if (h0 > y0) {
        t = 0.0f;
        return true;
    }
    const double d0 = static_cast<double>(y0) - h0;
    const float dh = terrain_f32(static_cast<double>(h1) - h0);
    const float dy = terrain_f32(static_cast<double>(y1) - y0);
    t = terrain_f32(d0 / (static_cast<double>(dh) - dy));   // no zero guard in the image
    return true;
}

// 00AEB940 (fastcall walker): the crossing, and the hit lerped between the two
// ground points by 005803E0.
bool vsub_cross_00aeb940(VsubCellWalker& w) noexcept {
    float t = 0.0f;
    if (!vsub_cross_00aeaaa0(w.ha, w.ya, w.hb, w.yb, t)) return false;
    const std::array<float, 3> a{w.ax, w.ha, w.az};
    const std::array<float, 3> b{w.bx, w.hb, w.bz};
    const std::array<float, 3> r = bsp::main_menu_map_lerp_005803e0(a, b, t);
    for (int k = 0; k < 3; ++k) w.hit[k] = r[k];
    return true;
}

// 00AEBFF0 (a, b, ya, yb), RET 10h: the whole piece inside one cell.
bool vsub_cell_whole_00aebff0(VsubCellWalker& w, float ax, float az, float bx, float bz,
                              float ya, float yb) noexcept {
    w.ya = ya; w.ax = ax; w.az = az; w.yb = yb; w.bx = bx; w.bz = bz;
    w.ha = vsub_ground_00aebde0(w, ax, az);
    w.hb = vsub_ground_00aebde0(w, bx, bz);
    return vsub_cross_00aeb940(w);
}

// 00AEC050 (b, yb), RET 8: the last piece.
bool vsub_cell_last_00aec050(VsubCellWalker& w, float bx, float bz, float yb) noexcept {
    w.yb = yb; w.bx = bx; w.bz = bz;
    w.hb = vsub_ground_00aebde0(w, bx, bz);
    return vsub_cross_00aeb940(w);
}

// 00AEC090 (b, yb), RET 8: a piece ending on a cell edge; the ground there is
// the edge's two samples lerped by +28h (00AEAFA0), or the corner sample.
bool vsub_cell_edge_00aec090(VsubCellWalker& w, float bx, float bz, float yb) noexcept {
    w.yb = yb; w.bx = bx; w.bz = bz;
    if (w.e0i == w.e1i && w.e0j == w.e1j) {
        w.hb = vsub_sample(w, w.e0i, w.e0j);
    } else {
        const float s0 = vsub_sample(w, w.e0i, w.e0j);
        const float s1 = vsub_sample(w, w.e1i, w.e1j);
        w.hb = terrain_f32(static_cast<double>(s1) * w.frac + (1.0 - w.frac) * s0);
    }
    return vsub_cross_00aeb940(w);
}

// 00AEC120 (thiscall cell walker; out), RET 4: the walk over the 32 x 32 cells.
bool vsub_cell_walk_00aec120(VsubCellWalker& w, float out[3], unsigned long long& cells) noexcept {
    if (w.sx == w.ex && w.sz == w.ez) return false;
    VsubIter it;
    it.x0 = w.sx; it.z0 = w.sz; it.x1 = w.ex; it.z1 = w.ez;
    vsub_iter_init_00aeb430(it);
    vsub_iter_start(it);
    float h = vsub_ground_00aebde0(w, w.sx, w.sz);
    if (h > w.y0) {                                              // 00AEC1FA
        out[0] = w.sx; out[1] = h; out[2] = w.sz;
        return true;
    }
    bool hit = false;
    if (!(kVsubEps < it.len) || !vsub_iter_crosses_00aeb680(it)) {
        hit = vsub_cell_whole_00aebff0(w, w.sx, w.sz, w.ex, w.ez, w.y0, w.y1);
    } else {
        vsub_iter_snap_00aeab30(it);
        if (it.flags > 0) h = vsub_ground_00aebde0(w, it.px, it.pz);
        w.ax = it.px; w.az = it.pz; w.ha = h; w.ya = w.y0;
        for (;;) {
            const float m = it.tmz > it.tmx ? it.tmx : it.tmz;
            if (it.len < m) {
                hit = vsub_cell_last_00aec050(w, w.ex, w.ez, w.y1);
                break;
            }
            vsub_iter_step_00aeb6d0(it);
            ++cells;
            w.e0i = it.e0i; w.e0j = it.e0j; w.e1i = it.e1i; w.e1j = it.e1j; w.frac = it.frac;
            const float y = terrain_f32(static_cast<double>(w.slope) * it.t + w.y0);
            if (vsub_cell_edge_00aec090(w, it.nx, it.nz, y)) {
                hit = true;
                break;
            }
            w.ax = w.bx; it.px = it.nx; w.az = w.bz; it.pz = it.nz; w.ha = w.hb; w.ya = w.yb;
        }
    }
    if (!hit) return false;
    for (int k = 0; k < 3; ++k) out[k] = w.hit[k];
    return true;
}

// The tile walker 00AECA60 builds on its stack (54h bytes from 00AEB770 /
// 00AEADE0; only the first 10h are initialised there).
struct VsubTileWalker {
    const SceneTerrainHeightField* field{nullptr};   // +0h
    float node_y{0};                                 // +Ch, node +124h
    float ax{0}, az{0}, bx{0}, bz{0};                // +20h .. +2Ch
    int ti{0}, tj{0};                                // +30h, +34h
    float ya{0}, yb{0};                              // +40h, +44h
    float hit[3]{};                                  // +48h .. +50h
    float sx{0}, sz{0}, ex{0}, ez{0};                // +6Ch .. +78h
    float y0{0}, y1{0}, slope{0};                    // +7Ch, +80h, +84h
};

// 00AEC3F0 (fastcall tile walker): one tile.
bool vsub_tile_00aec3f0(VsubTileWalker& w, unsigned long long& tiles,
                        unsigned long long& cells) noexcept {
    // The image reads +40h[+38h * j + i] unchecked; the host answers a tile
    // outside the grid as missing. LABELLED.
    const SceneTerrainBlock* block = quad_tile(*w.field, w.ti, w.tj);
    if (block == nullptr) return false;
    ++tiles;
    float tile_lo = 0.0f, tile_hi = 0.0f;
    tile_range_00aed020(*block, tile_lo, tile_hi);               // tile +18h
    const float top = terrain_f32(static_cast<double>(tile_hi) + w.node_y);
    const float ymin = w.yb > w.ya ? w.ya : w.yb;
    if (!(top > ymin)) return false;                             // 00AEC45D
    const float fi = static_cast<float>(w.ti);
    const float fj = static_cast<float>(w.tj);
    const float n = kVsubCells;
    float s[6] = {
        terrain_f32(static_cast<double>(terrain_f32(static_cast<double>(w.ax) - fi)) * n), w.ya,
        terrain_f32(static_cast<double>(terrain_f32(static_cast<double>(w.az) - fj)) * n),
        terrain_f32(static_cast<double>(terrain_f32(static_cast<double>(w.bx) - fi)) * n), w.yb,
        terrain_f32(static_cast<double>(n) * terrain_f32(static_cast<double>(w.bz) - fj))};
    const float lo[2] = {0.0f, 0.0f};
    const float hi[2] = {n, n};
    (void)vsub_clip_00aebd20(s, lo, hi);                         // 00AEC544, AL not tested
    VsubCellWalker c;
    c.block = block;
    c.node_y = w.node_y;
    c.sx = s[0]; c.sz = s[2]; c.ex = s[3]; c.ez = s[5];
    c.y0 = s[1]; c.y1 = s[4];
    c.slope = vsub_slope(s[0], s[2], s[1], s[3], s[5], s[4]);  // 00AEB7E0
    float out[3];
    if (!vsub_cell_walk_00aec120(c, out, cells)) return false;
    w.hit[1] = out[1];
    w.hit[0] = terrain_f32(static_cast<double>(out[0]) / n + fi);
    w.hit[2] = terrain_f32(static_cast<double>(out[2]) / n + fj);
    return true;
}

// 00AEC660 (a, b, ya, yb), RET 10h: the whole segment in the tile of its
// minimum corner.
bool vsub_tile_whole_00aec660(VsubTileWalker& w, float ax, float az, float bx, float bz,
                              float ya, float yb, unsigned long long& tiles,
                              unsigned long long& cells) noexcept {
    w.ya = ya; w.ax = ax; w.az = az; w.yb = yb; w.bx = bx; w.bz = bz;
    w.ti = static_cast<int>(bx <= ax ? bx : ax);
    w.tj = static_cast<int>(bz <= az ? bz : az);
    return vsub_tile_00aec3f0(w, tiles, cells);
}

// 00AEC700 (b, yb), RET 8: the last piece, in the grid only.
bool vsub_tile_last_00aec700(VsubTileWalker& w, float bx, float bz, float yb,
                             unsigned long long& tiles, unsigned long long& cells) noexcept {
    const float mz = bz <= w.az ? bz : w.az;
    const float mx = bx <= w.ax ? bx : w.ax;
    w.ti = static_cast<int>(mx);
    w.tj = static_cast<int>(mz);
    if (w.ti < 0 || w.ti >= w.field->tiles_wide || w.tj < 0 || w.tj >= w.field->tiles_deep) {
        return false;
    }
    w.yb = yb; w.bx = bx; w.bz = bz;
    return vsub_tile_00aec3f0(w, tiles, cells);
}

// 00AEC7C0 (thiscall tile walker; out), RET 4: the walk over the tiles.
bool vsub_tile_walk_00aec7c0(VsubTileWalker& w, float out[3], unsigned long long& tiles,
                             unsigned long long& cells) noexcept {
    if (w.ex == w.sx && w.ez == w.sz) return false;
    VsubIter it;
    it.x0 = w.sx; it.z0 = w.sz; it.x1 = w.ex; it.z1 = w.ez;
    vsub_iter_init_00aeb430(it);
    vsub_iter_start(it);
    if (-1000.0 > w.y0) {                                        // [00CE6658]
        out[0] = w.sx; out[1] = kTerrainNoGround; out[2] = w.sz;
        return true;
    }
    bool hit = false;
    if (!(kVsubEps < it.len) || !vsub_iter_crosses_00aeb680(it)) {
        hit = vsub_tile_whole_00aec660(w, w.sx, w.sz, w.ex, w.ez, w.y0, w.y1, tiles, cells);
    } else {
        vsub_iter_snap_00aeab30(it);
        w.ax = it.px; w.az = it.pz; w.ya = w.y0;
        for (;;) {
            const float m = it.tmz > it.tmx ? it.tmx : it.tmz;
            if (it.len < m) {
                hit = vsub_tile_last_00aec700(w, w.ex, w.ez, w.y1, tiles, cells);
                break;
            }
            vsub_iter_step_00aeb6d0(it);
            w.ti = it.ci; w.tj = it.cj;
            w.yb = terrain_f32(static_cast<double>(w.slope) * it.t + w.y0);
            w.bx = it.nx; w.bz = it.nz;
            if (vsub_tile_00aec3f0(w, tiles, cells)) {
                hit = true;
                break;
            }
            w.ax = w.bx; w.az = w.bz; it.px = it.nx; it.pz = it.nz; w.ya = w.yb;
        }
    }
    if (!hit) return false;
    for (int k = 0; k < 3; ++k) out[k] = w.hit[k];
    return true;
}

// 00AECA60 (ECX terrain, EDX from, stack to and out; RET 8), in tile units.
bool vsub_subwalk_00aeca60(const SceneTerrainHeightField& field, const float from[3],
                           const float to[3], float out[3], unsigned long long& tiles,
                           unsigned long long& cells) noexcept {
    float s[6] = {from[0], from[1], from[2], to[0], to[1], to[2]};
    const float lo[2] = {0.0f, 0.0f};
    const float hi[2] = {static_cast<float>(field.tiles_wide), static_cast<float>(field.tiles_deep)};
    if (!vsub_clip_00aebd20(s, lo, hi)) return false;
    // 00AECAFA..00AECB90: both x on the far x edge, or both z on the far z edge.
    if (static_cast<double>(s[0]) == field.tiles_wide && static_cast<double>(s[3]) == field.tiles_wide) {
        return false;
    }
    if (static_cast<double>(s[2]) == field.tiles_deep && static_cast<double>(s[5]) == field.tiles_deep) {
        return false;
    }
    VsubTileWalker w;
    w.field = &field;
    w.node_y = field.node_y;                                     // 00AEB770, node +124h
    w.sx = s[0]; w.sz = s[2]; w.ex = s[3]; w.ez = s[5];          // 00AEB890
    w.y0 = s[1]; w.y1 = s[4];
    w.slope = vsub_slope(s[0], s[2], s[1], s[3], s[5], s[4]);
    return vsub_tile_walk_00aec7c0(w, out, tiles, cells);
}

// Terrain vt+48h 00ADB480 at (u, v) in its own units (cells).
float vsub_bilinear_00adb480(const SceneTerrainHeightField& field, float u, float v) noexcept {
    const int i = static_cast<int>(u);
    const int j = static_cast<int>(v);
    const float fu = terrain_f32(static_cast<double>(u) - i);
    const float h11 = field.cell_height_00adb3a0(i + 1, j + 1);
    const float h01 = field.cell_height_00adb3a0(i, j + 1);
    const float h10 = field.cell_height_00adb3a0(i + 1, j);
    const float h00 = field.cell_height_00adb3a0(i, j);
    const float a = terrain_f32(h00 + static_cast<double>(fu) * (static_cast<double>(h10) - h00));
    const float b = terrain_f32(h01 + static_cast<double>(fu) * (static_cast<double>(h11) - h01));
    const float fv = terrain_f32(static_cast<double>(v) - j);
    return terrain_f32(a + static_cast<double>(fv) * (static_cast<double>(b) - a));
}

// 00AECC40 (fastcall terrain, EDX world from; world to, out; RET 8) after its
// two transforms (the local points here). `raw` is set when the answer is the
// equal-point case, which 00AECC40 writes in TILE units with no transform back.
struct VsubAnswer {
    bool hit{false};
    bool raw{false};
    float point[3]{};   // raw: tile units as written; otherwise origin-relative local
};

VsubAnswer vsub_vertical_00aecc40(const SceneTerrainHeightField& field, const float la[3],
                                  const float lb[3], SceneTerrainQuadtreeCensus& census) noexcept {
    VsubAnswer answer;
    const float fx = terrain_f32(static_cast<double>(la[0]) - field.origin_x);
    const float fz = terrain_f32(static_cast<double>(la[2]) - field.origin_z);
    const float tx = terrain_f32(static_cast<double>(lb[0]) - field.origin_x);
    const float tz = terrain_f32(static_cast<double>(lb[2]) - field.origin_z);
    const float size = kQuadTileSize;                            // +18h
    const float inv = terrain_f32(1.0 / size);
    const float from_t[3] = {terrain_f32(static_cast<double>(inv) * fx), la[1],
                             terrain_f32(static_cast<double>(inv) * fz)};
    const float to_t[3] = {terrain_f32(static_cast<double>(inv) * tx), lb[1],
                           terrain_f32(static_cast<double>(inv) * tz)};
    if (to_t[0] == from_t[0] && to_t[2] == from_t[2]) {
        // 00AECDB5: vt+48h in TILE units, where it expects cells. The image's own
        // behaviour, kept.
        ++census.vertical_equal;
        const float h = vsub_bilinear_00adb480(field, from_t[0], from_t[2]);
        const float lo = lb[1] > la[1] ? la[1] : lb[1];
        const float hi = lb[1] > la[1] ? lb[1] : la[1];
        if (h > lo && hi > h) {
            answer.hit = true;
            answer.raw = true;
            answer.point[0] = from_t[0];
            answer.point[1] = h;
            answer.point[2] = from_t[2];
            return answer;
        }
    }
    ++census.vertical_walks;
    float out[3];
    if (!vsub_subwalk_00aeca60(field, from_t, to_t, out, census.vertical_tiles,
                               census.vertical_cells)) {
        return answer;
    }
    answer.hit = true;
    answer.point[0] = terrain_f32(static_cast<double>(out[0]) * size + field.origin_x);
    answer.point[1] = out[1];
    answer.point[2] = terrain_f32(static_cast<double>(size) * out[2] + field.origin_z);
    return answer;
}

}  // namespace

bool landscape_entry_segment_hit(std::size_t entry, const float from[3],
                                 const float to[3], LandscapeSegmentHit& hit) noexcept {
    const int object_index = landscape_segment_entry_object(entry);
    if (object_index < 0) return false;
    const SceneWorldObject& object
        = scene_world_class_lists().objects()[static_cast<std::size_t>(object_index)];
    const SceneTerrainHeightField& field = *object.terrain;
    double a[3], b[3];
    if constexpr (kLandscapeScaledTransposeInverseBound) {
        frame_inverse_point_00b63b30(object.world, from, a);
        frame_inverse_point_00b63b30(object.world, to, b);
    } else if (!frame_inverse_point(object.world, from, a)
               || !frame_inverse_point(object.world, to, b)) {
        return false;
    }
    const double d[3] = {b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    bool vertical_case = false;
    if constexpr (kTerrainSegmentQuadtreeBound) {
        // Packet cc9_terrain_segment_quadtree: 00ADA240 after its two
        // transforms (00ADA262, 00ADA283), stored as floats.
        SceneTerrainQuadtreeCensus& census = scene_terrain_quadtree_census();
        const float la[3] = {static_cast<float>(a[0]), static_cast<float>(a[1]), static_cast<float>(a[2])};
        const float lb[3] = {static_cast<float>(b[0]), static_cast<float>(b[1]), static_cast<float>(b[2])};
        const float adx = std::fabs(terrain_f32(static_cast<double>(la[0]) - lb[0]));
        const float adz = std::fabs(terrain_f32(static_cast<double>(la[2]) - lb[2]));
        if (adx < kQuadMinDirection && adz < kQuadMinDirection) {
            // 00ADA2EE -> 00AECC40 with the WORLD from (EDX) and to. OFF: LABELLED
            // STAND-IN, the march below answers this case.
            ++census.vertical;
            vertical_case = true;
            if constexpr (kTerrainVerticalSubwalkBound) {
                // 00AECC40 transforms both world points through the same inverse
                // (00AECC7F, 00AECCB4) as 00ADA240, so the local points are reused.
                // Its first test, both world y above terrain +14h (1e10), cannot pass.
                const VsubAnswer v = vsub_vertical_00aecc40(field, la, lb, census);
                if (!v.hit) return false;
                ++census.vertical_hits;
                double world[3];
                if (v.raw) {
                    // 00AECE22..00AECE3F: the equal-point answer is written in TILE
                    // units with no transform back. The image's own behaviour, kept.
                    for (int k = 0; k < 3; ++k) world[k] = v.point[k];
                } else {
                    // 00AECE66..00AECEB8: tile units back to local, then node +F0h.
                    const double local[3] = {v.point[0], v.point[1], v.point[2]};
                    frame_point(object.world, local, world);
                }
                for (int k = 0; k < 3; ++k) hit.point[k] = static_cast<float>(world[k]);
                hit.landscape_object = object_index;
                hit.entry = entry;
                hit.shape_kind = 0x0a;
                hit.hull_segment = -1;
                // SUBSTITUTION, labelled: the projection of the answer on from->to,
                // in world space, as the host's `fraction`.
                const double wd[3] = {static_cast<double>(to[0]) - from[0],
                    static_cast<double>(to[1]) - from[1], static_cast<double>(to[2]) - from[2]};
                const double wdd = wd[0] * wd[0] + wd[1] * wd[1] + wd[2] * wd[2];
                hit.fraction = static_cast<float>(wdd > 0.0 ? ((world[0] - from[0]) * wd[0]
                    + (world[1] - from[1]) * wd[1] + (world[2] - from[2]) * wd[2]) / wdd : 0.0);
                return true;
            }
        } else {
            ++census.walks;
            if (field.quadtree.empty()) build_quadtree_00aea820(field);
            const float p[3] = {terrain_f32(static_cast<double>(la[0]) - field.origin_x), la[1],
                terrain_f32(static_cast<double>(la[2]) - field.origin_z)};
            const float q[3] = {terrain_f32(static_cast<double>(lb[0]) - field.origin_x), lb[1],
                terrain_f32(static_cast<double>(lb[2]) - field.origin_z)};
            float r[3];
            if (!ray_walk_00aea2b0(field, p, q, r)) return false;
            ++census.walk_hits;
            // 00ADA35E..00ADA3C7: origin added back, then the node's world
            // matrix +F0h.
            const double local[3] = {terrain_f32(static_cast<double>(field.origin_x) + r[0]), r[1],
                terrain_f32(static_cast<double>(field.origin_z) + r[2])};
            double world[3];
            frame_point(object.world, local, world);
            for (int k = 0; k < 3; ++k) hit.point[k] = static_cast<float>(world[k]);
            hit.landscape_object = object_index;
            hit.entry = entry;
            hit.shape_kind = 0x0a;
            hit.hull_segment = -1;
            // SUBSTITUTION, labelled: 00ADA240 answers a point only; the host's
            // `fraction` is its projection on from->to, which the line test
            // (00ADEB80) can put outside [0, 1].
            const double dd = d[0] * d[0] + d[1] * d[1] + d[2] * d[2];
            const double along = dd > 0.0 ? ((local[0] - a[0]) * d[0] + (local[1] - a[1]) * d[1]
                + (local[2] - a[2]) * d[2]) / dd : 0.0;
            hit.fraction = static_cast<float>(along);
            return true;
        }
    }
    const double length = std::sqrt(d[0] * d[0] + d[1] * d[1] + d[2] * d[2]);
    const int steps = std::max(1, static_cast<int>(std::ceil(length / (kTerrainCellSize * 0.5))));
    auto below = [&](double t) {
        const double p[3] = {a[0] + d[0] * t, a[1] + d[1] * t, a[2] + d[2] * t};
        return p[1] < local_ground(field, p[0], p[2]);
    };
    // A segment that starts below the surface hits at its start, as a ray
    // walk entering the field from beneath would at its first cell.
    double t_hit = -1.0;
    if (below(0.0)) {
        t_hit = 0.0;
    } else {
        double previous = 0.0;
        for (int k = 1; k <= steps; ++k) {
            const double t = static_cast<double>(k) / steps;
            if (below(t)) {
                double lo_t = previous, hi_t = t;
                for (int it = 0; it < 24; ++it) {
                    const double mid = 0.5 * (lo_t + hi_t);
                    if (below(mid)) hi_t = mid; else lo_t = mid;
                }
                t_hit = hi_t;
                break;
            }
            previous = t;
        }
    }
    if (t_hit < 0.0) return false;
    if (vertical_case) ++scene_terrain_quadtree_census().vertical_hits;
    const double local[3] = {a[0] + d[0] * t_hit, a[1] + d[1] * t_hit, a[2] + d[2] * t_hit};
    double world[3];
    frame_point(object.world, local, world);   // 00ADA240's +F0h transform back
    for (int k = 0; k < 3; ++k) hit.point[k] = static_cast<float>(world[k]);
    hit.landscape_object = object_index;
    hit.entry = entry;
    hit.shape_kind = 0x0a;      // 0087FFDE
    hit.hull_segment = -1;      // 0087FFE5
    hit.fraction = static_cast<float>(t_hit);
    return true;
}

bool landscape_segment_hit(const float from[3], const float to[3], float hit_point[3],
                           int& landscape_index) noexcept {
    landscape_index = -1;
    bool any = false;
    float best = 2.0f;
    const std::size_t count = landscape_segment_entry_count();
    for (std::size_t entry = 0; entry < count; ++entry) {
        LandscapeSegmentHit hit;
        if (!landscape_entry_segment_hit(entry, from, to, hit)) continue;
        if (hit.fraction < best) {
            best = hit.fraction;
            any = true;
            landscape_index = hit.landscape_object;
            for (int k = 0; k < 3; ++k) hit_point[k] = hit.point[k];
        }
    }
    return any;
}

SceneLandHitCensus& scene_land_hit_census() noexcept {
    static SceneLandHitCensus census;
    return census;
}

void note_land_hit_query(LandHitConsumer consumer, bool land_hit) noexcept {
    SceneLandHitCensus& census = scene_land_hit_census();
    const int k = static_cast<int>(consumer);
    ++census.calls[k];
    if (land_hit) ++census.land_hits[k];
    if (land_hit && consumer == LandHitConsumer::LineOfFire) ++census.line_of_fire_blocked;
}

std::string format_land_hit_census() {
    const SceneLandHitCensus& c = scene_land_hit_census();
    const SceneTerrainQuadtreeCensus& t = scene_terrain_quadtree_census();
    char text[512];
    std::snprintf(text, sizeof(text),
        "pick=%llu/%llu seat=%llu/%llu line_of_fire=%llu/%llu blocked=%llu projectile=%llu/%llu "
        "slot3c bound=%d walks=%llu/%llu vertical=%llu/%llu leaves=%llu cells=%llu "
        "vsub bound=%d equal=%llu walks=%llu tiles=%llu cells=%llu",
        c.calls[0], c.land_hits[0], c.calls[1], c.land_hits[1], c.calls[2], c.land_hits[2],
        c.line_of_fire_blocked, c.calls[3], c.land_hits[3], kTerrainSegmentQuadtreeBound ? 1 : 0,
        t.walks, t.walk_hits, t.vertical, t.vertical_hits, t.leaves, t.cells,
        kTerrainVerticalSubwalkBound ? 1 : 0, t.vertical_equal, t.vertical_walks,
        t.vertical_tiles, t.vertical_cells);
    return text;
}

bool world_segment_blocked_00903bc0(const float from[3], const float to[3]) noexcept {
    SceneTerrainQueryCensus& census = scene_terrain_query_census();
    ++census.segment_calls;
    int winner = -1;
    if (terrain_list_walk(from, winner) > from[1] || terrain_list_walk(to, winner) > to[1]) {
        ++census.segment_endpoint_blocks;
        return true;
    }
    if constexpr (kTerrainSegmentQuadtreeBound) {
        // Packet cc9_terrain_segment_quadtree: 00903C20..00903C42, every node
        // of list 44h through its terrain's slot 3Ch (vt+3Ch on +3D0h), which
        // takes the Landscape's full frame; the first hit blocks.
        const std::size_t count = landscape_segment_entry_count();
        for (std::size_t entry = 0; entry < count; ++entry) {
            LandscapeSegmentHit hit;
            ++census.segment_sweep_entries;
            if (landscape_entry_segment_hit(entry, from, to, hit)) {
                ++census.segment_sweep_blocks;
                return true;
            }
        }
        return false;
    }
    // LABELLED STAND-IN for slot 3Ch (see the header).
    const double dx = static_cast<double>(to[0]) - from[0];
    const double dy = static_cast<double>(to[1]) - from[1];
    const double dz = static_cast<double>(to[2]) - from[2];
    const double length = std::sqrt(dx * dx + dy * dy + dz * dz);
    const int steps = std::max(1, static_cast<int>(std::ceil(length / (kTerrainCellSize * 0.5))));
    const SceneWorldClassLists& lists = scene_world_class_lists();
    for (std::size_t index : lists.list(kSceneLandscapeClassId)) {
        const SceneWorldObject& object = lists.objects()[index];
        if (!object.terrain) continue;
        for (int k = 1; k < steps; ++k) {
            const double t = static_cast<double>(k) / steps;
            const float px = terrain_f32(from[0] + dx * t);
            const float py = terrain_f32(from[1] + dy * t);
            const float pz = terrain_f32(from[2] + dz * t);
            if (object.terrain->height_00ada900(px, pz) > py) {
                ++census.segment_sweep_blocks;
                return true;
            }
        }
    }
    return false;
}

const GameSceneContentsSummary& GameSceneContentsHost::summary() const noexcept {
    return impl_->summary;
}

const std::vector<GameSceneEntityRecord>& GameSceneContentsHost::entities() const noexcept {
    return impl_->entities;
}

const bsp::ScenePropertyBlock& GameSceneContentsHost::root_properties() const noexcept {
    return impl_->root_properties;
}

std::size_t GameSceneContentsHost::created_unit_count() const noexcept {
    return impl_->summary.created;
}

void GameSceneContentsHost::run_load_scene_contents_004d4df0(const std::string& scene_path,
    const std::string& override_name, int raw_game_mode, bool mode_forced,
    bool multiplayer_session) {
    Impl& impl = *impl_;
    impl.root_properties = {};
    impl.scene_path = scene_path;
    impl.override_name = override_name;
    impl.raw_game_mode = raw_game_mode;
    impl.mode_forced = mode_forced;
    impl.multiplayer = multiplayer_session;
    impl.summary.scene_path = scene_path;
    impl.summary.short_name = derive_scene_short_name(scene_path);

    // The pool belongs to the scene being loaded, and 004D54A4 destroys the
    // scene database's own range at [00E18680]+8h before pass 2 fills it. A run
    // in this process loads one mission, so this changes nothing measured today;
    // without it a process that loaded a second mission would answer
    // `GenerateObject` out of the first one's records.
    scene_spawn_pool().clear();
    // Same reason again: the authored Path entities belong to this scene.
    scene_path_registry().clear();
    scene_world_class_lists().clear();
    scene_director_enables_clear();
    // Same reason as the pool above: the squadron table belongs to the scene
    // being loaded, and a second mission must not inherit the first one's wings.
    bsp::plane_squadron_registry().clear();
    impl.pending_squadron_members.clear();
    impl.load_property_library();

    const int mode = effective_game_mode_004bca50(raw_game_mode, mode_forced,
        multiplayer_session);
    impl.summary.effective_game_mode = mode;

    SceneContentsBinding binding(impl, mode);
    load_scene_contents_004d4df0(binding);
    impl.summary.effective_game_mode = binding.effective_mode();
    impl.summary.ran = true;

    impl.log.notef("scene contents: mode=%d (004bca50 with raw=%d forced=%d multiplayer=%d), "
        "%zu entities registered, %zu instantiated, %zu generated, %zu rejected, %zu created, "
        "%zu held back by Hidden (0046d3c5, the GenerateObject pool)",
        impl.summary.effective_game_mode, raw_game_mode, mode_forced ? 1 : 0,
        multiplayer_session ? 1 : 0, impl.summary.registration_entities,
        impl.summary.instantiate_entities, impl.summary.generated, impl.summary.rejected,
        impl.summary.created, impl.summary.held_back_hidden);
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
    // Packet cc9_scene_path_landscape: the creator census, printed in both
    // switch states so a pair differs only in what the switch changes.
    {
        std::size_t path_seen = 0, path_generated = 0, path_rejected = 0, path_objects = 0;
        std::size_t land_seen = 0, land_generated = 0, land_rejected = 0, land_objects = 0;
        for (const GameSceneClassTally& row : impl.summary.classes) {
            if (row.class_id == kScenePathClassId) {
                path_seen += row.seen; path_generated += row.generated;
                path_rejected += row.rejected; path_objects += row.objects;
            } else if (row.class_id == kSceneLandscapeClassId) {
                land_seen += row.seen; land_generated += row.generated;
                land_rejected += row.rejected; land_objects += row.objects;
            }
        }
        const SceneWorldClassLists& lists = scene_world_class_lists();
        const std::vector<std::size_t> list47 = lists.list(kScenePathClassId);
        const std::vector<std::size_t> list44 = lists.list(kSceneLandscapeClassId);
        // 00424D00's walk of world+370h against the record walk the avoid-zone
        // rebuild makes: the AvoidZone-named paths, in order.
        std::vector<std::string> by_list;
        for (std::size_t i : list47) {
            if (lists.objects()[i].name.compare(0, 9, "AvoidZone") == 0) {
                by_list.push_back(lists.objects()[i].name);
            }
        }
        std::vector<std::string> by_records;
        for (const GameSceneEntityRecord& entity : impl.entities) {
            if (entity.class_id == kScenePathClassId && entity.generated
                && entity.name.compare(0, 9, "AvoidZone") == 0) {
                by_records.push_back(entity.name);
            }
        }
        std::size_t heightmaps = 0, colormaps = 0, models = 0;
        for (std::size_t i : list44) {
            heightmaps += lists.objects()[i].heightmap_resolved ? 1u : 0u;
            colormaps += lists.objects()[i].colormap_resolved ? 1u : 0u;
            models += lists.objects()[i].model_resolved ? 1u : 0u;
        }
        impl.log.notef("summary scene path/landscape creators bound=%d path seen=%zu "
            "generated=%zu rejected=%zu created=%zu landscape seen=%zu generated=%zu "
            "rejected=%zu created=%zu list47=%zu list44=%zu avoid_zone_paths=%zu "
            "list_order_matches_records=%d terrain heightmaps=%zu/%zu colormaps=%zu/%zu "
            "models=%zu/%zu (packet cc9_scene_path_landscape, 004ea650 / 004f1460)",
            kScenePathLandscapeCreatorsBound ? 1 : 0, path_seen, path_generated,
            path_rejected, path_objects, land_seen, land_generated, land_rejected,
            land_objects, list47.size(), list44.size(), by_list.size(),
            (kScenePathLandscapeCreatorsBound && by_list == by_records) ? 1 : 0,
            heightmaps, list44.size(), colormaps, list44.size(), models, list44.size());
    }
    // Packet cc9_landscape_terrain: a load-time self-check of the four queries
    // against the authored objects on each island (the scene editor snaps them
    // to the ground), then the census. Printed in both switch states.
    {
        scene_terrain_query_census() = SceneTerrainQueryCensus{};
        const SceneWorldClassLists& lists = scene_world_class_lists();
        std::size_t loaded = 0, blocks = 0, children = 0, within = 0;
        for (std::size_t index : lists.list(kSceneLandscapeClassId)) {
            const SceneWorldObject& land = lists.objects()[index];
            if (!land.terrain) continue;
            ++loaded;
            blocks += land.terrain->blocks.size();
            std::size_t mine = 0, close = 0, same_landscape = 0, probe_hits = 0;
            std::size_t probe_close = 0;
            // Packet cc9_terrain_segment_quadtree: a slanted trace through each
            // object walks the quadtree (the vertical one above takes 00AECC40);
            // its hit is checked against this Landscape's own local surface.
            std::size_t slant_hits = 0, slant_on_surface = 0;
            double slant_worst = 0.0;
            // Packet cc9_terrain_vertical_subwalk: a near-vertical trace (0.0009 m
            // of run in x, inside 00ADA240's 0.001 vertical test but not equal in
            // tile units) takes 00AECA60's walk; its hit is checked against the
            // object's ground height.
            std::size_t near_hits = 0, near_on_ground = 0;
            double near_worst = 0.0;
            std::size_t entry_of_land = static_cast<std::size_t>(-1);
            {
                const std::size_t entries = landscape_segment_entry_count();
                for (std::size_t e = 0; e < entries; ++e) {
                    if (landscape_segment_entry_object(e) == static_cast<int>(index)) {
                        entry_of_land = e;
                    }
                }
            }
            double worst = 0.0;
            for (const GameSceneEntityRecord& entity : impl.entities) {
                if (entity.parent_scene_id != land.scene_id || !entity.generated) continue;
                if (entity.class_id == kScenePathClassId
                    || entity.class_id == kSceneLandscapeClassId) continue;
                const float point[3] = {entity.world[12], entity.world[13], entity.world[14]};
                float ground = 0.0f;
                float normal[3] = {0.0f, 0.0f, 0.0f};
                world_ground_height_00903860(point, ground);
                world_ground_normal_009038f0(point, normal);
                if (world_landscape_at_009039d0(point) == static_cast<int>(index)) ++same_landscape;
                // Packet cc9_landscape_spatial_attach: the entry trace straight
                // down through the object, 50 m either side.
                if (entry_of_land != static_cast<std::size_t>(-1)) {
                    const float top[3] = {point[0], point[1] + 50.0f, point[2]};
                    const float bottom[3] = {point[0], point[1] - 50.0f, point[2]};
                    LandscapeSegmentHit probe;
                    if (landscape_entry_segment_hit(entry_of_land, top, bottom, probe)) {
                        ++probe_hits;
                        if (std::fabs(static_cast<double>(probe.point[1]) - ground) < 0.05) {
                            ++probe_close;
                        }
                    }
                }
                if constexpr (kTerrainVerticalSubwalkBound) {
                    if (entry_of_land != static_cast<std::size_t>(-1)) {
                        const float top[3] = {point[0], point[1] + 50.0f, point[2]};
                        const float bottom[3] = {point[0] + 0.0009f, point[1] - 50.0f, point[2]};
                        LandscapeSegmentHit near_hit;
                        if (landscape_entry_segment_hit(entry_of_land, top, bottom, near_hit)) {
                            ++near_hits;
                            const double err = std::fabs(static_cast<double>(near_hit.point[1]) - ground);
                            if (err < 0.25) ++near_on_ground;
                            near_worst = std::max(near_worst, err);
                        }
                    }
                }
                if constexpr (kTerrainSegmentQuadtreeBound) {
                    if (entry_of_land != static_cast<std::size_t>(-1)) {
                        const float top[3] = {point[0] - 20.0f, point[1] + 50.0f, point[2] - 15.0f};
                        const float bottom[3] = {point[0] + 20.0f, point[1] - 50.0f, point[2] + 15.0f};
                        LandscapeSegmentHit slant;
                        double lp[3];
                        if (landscape_entry_segment_hit(entry_of_land, top, bottom, slant)
                            && frame_inverse_point(land.world, slant.point, lp)) {
                            ++slant_hits;
                            const double err = std::fabs(lp[1] - local_ground(*land.terrain, lp[0], lp[2]));
                            if (err < 0.25) ++slant_on_surface;
                            slant_worst = std::max(slant_worst, err);
                        }
                    }
                }
                ++mine;
                const double diff = std::fabs(static_cast<double>(ground) - point[1]);
                if (diff < 0.01) ++close;
                worst = std::max(worst, diff);
            }
            children += mine;
            within += close;
            impl.log.notef("scene terrain self-check: landscape=%s objects=%zu on_ground_1cm=%zu "
                "worst=%.3f landscape_at_self=%zu segment_probe_hits=%zu segment_probe_on_ground=%zu "
                "(00903860 / 009038f0 / 009039d0 / 0087ff80)",
                land.name.c_str(), mine, close, worst, same_landscape, probe_hits, probe_close);
            if constexpr (kTerrainVerticalSubwalkBound) {
                impl.log.notef("scene terrain vertical self-check: landscape=%s near_vertical_hits=%zu "
                    "on_ground_25cm=%zu worst=%.3f (00AECC40 -> 00AECA60, packet "
                    "cc9_terrain_vertical_subwalk)", land.name.c_str(), near_hits, near_on_ground,
                    near_worst);
            }
            if constexpr (kTerrainSegmentQuadtreeBound) {
                impl.log.notef("scene terrain slot 3Ch self-check: landscape=%s slant_hits=%zu "
                    "on_local_surface_25cm=%zu worst=%.3f (00ADA240 -> 00AEA2B0, packet "
                    "cc9_terrain_segment_quadtree)", land.name.c_str(), slant_hits,
                    slant_on_surface, slant_worst);
            }
        }
        const SceneTerrainQueryCensus& census = scene_terrain_query_census();
        impl.log.notef("summary scene terrain bound=%d landscapes=%zu loaded=%zu blocks=%zu "
            "self_check_objects=%zu on_ground_1cm=%zu height calls=%llu hits=%llu fallbacks=%llu "
            "normal calls=%llu hits=%llu landscape_at calls=%llu hits=%llu segment calls=%llu "
            "segment_entries=%zu land_hits %s "
            "(packet cc9_landscape_terrain; the 30 consumer sites are unbound)",
            kSceneLandscapeTerrainBound ? 1 : 0, lists.list(kSceneLandscapeClassId).size(),
            loaded, blocks, children, within, census.height_calls, census.height_hits,
            census.height_fallbacks, census.normal_calls, census.normal_hits,
            census.landscape_calls, census.landscape_hits, census.segment_calls,
            landscape_segment_entry_count(), format_land_hit_census().c_str());
        // Packet cc9_terrain_rotation_measure (docs/SCENE_CONTENTS_HOSTS.md section 13):
        // the image's two answers over each island's drawn footprint. A 48 x 48 grid
        // of local points (origin + [0, tiles * 300]) is taken to the world through
        // the Landscape's frame. At each point the ground height 00903860 (the
        // translation-only point query over every Landscape) and this Landscape's
        // slot 3Ch (00ADA240, the full inverse frame; a near-vertical trace from
        // y 3000 to -500 with a 0.5 m run in x so it takes the quadtree walk) are
        // asked. Land is a surface above y 0. Printed after the load summary, so
        // that line's counts are unchanged; a diagnostic, no switch.
        if constexpr (kTerrainSegmentQuadtreeBound) {
            const std::size_t entries = landscape_segment_entry_count();
            for (std::size_t e = 0; e < entries; ++e) {
                const int object_index = landscape_segment_entry_object(e);
                if (object_index < 0) continue;
                const SceneWorldObject& land = lists.objects()[static_cast<std::size_t>(object_index)];
                if (!land.terrain) continue;
                const SceneTerrainHeightField& field = *land.terrain;
                constexpr int kGrid = 48;
                const double span_x = static_cast<double>(field.tiles_wide) * 300.0;
                const double span_z = static_cast<double>(field.tiles_deep) * 300.0;
                std::size_t points = 0, seg_land = 0, height_land = 0, both = 0;
                std::size_t seg_only = 0, height_only = 0;
                // The same against this Landscape's own 00ADA900 (translation only),
                // without the other islands the world query maximises over.
                std::size_t own_land = 0, own_both = 0, own_seg_only = 0, own_height_only = 0;
                double worst = 0.0, sum = 0.0;
                for (int i = 0; i < kGrid; ++i) {
                    for (int j = 0; j < kGrid; ++j) {
                        const double local[3] = {field.origin_x + span_x * (i + 0.5) / kGrid, 0.0,
                                                 field.origin_z + span_z * (j + 0.5) / kGrid};
                        double w[3];
                        frame_point(land.world, local, w);
                        const float x = static_cast<float>(w[0]);
                        const float z = static_cast<float>(w[2]);
                        ++points;
                        float ground = 0.0f;
                        const float at[3] = {x, 0.0f, z};
                        world_ground_height_00903860(at, ground);
                        const float top[3] = {x, 3000.0f, z};
                        const float bottom[3] = {x + 0.5f, -500.0f, z};
                        LandscapeSegmentHit hit;
                        const bool seg = landscape_entry_segment_hit(e, top, bottom, hit)
                            && hit.point[1] > 0.0f;
                        const bool high = ground > 0.0f;
                        const bool own = field.height_00ada900(x, z) > 0.0f;
                        if (own) ++own_land;
                        if (seg && own) ++own_both;
                        else if (seg) ++own_seg_only;
                        else if (own) ++own_height_only;
                        if (seg) ++seg_land;
                        if (high) ++height_land;
                        if (seg && high) {
                            ++both;
                            const double d = std::fabs(static_cast<double>(hit.point[1]) - ground);
                            sum += d;
                            worst = std::max(worst, d);
                        } else if (seg) {
                            ++seg_only;
                        } else if (high) {
                            ++height_only;
                        }
                    }
                }
                const double yaw = std::atan2(static_cast<double>(land.world[8]),
                                              static_cast<double>(land.world[10])) * 57.29577951308232;
                impl.log.notef("scene terrain rotation census: landscape=%s yaw=%.1f tiles=%dx%d "
                    "points=%zu segment_land=%zu height_land=%zu both=%zu segment_only=%zu "
                    "height_only=%zu both_mean_dy=%.3f both_worst_dy=%.3f | own_height_land=%zu "
                    "own_both=%zu own_segment_only=%zu own_height_only=%zu (00ADA240 full frame "
                    "against 00903860 / 00ADA900 translation only, packet cc9_terrain_rotation_measure)",
                    land.name.c_str(), yaw, field.tiles_wide, field.tiles_deep, points, seg_land,
                    height_land, both, seg_only, height_only, both ? sum / both : 0.0, worst,
                    own_land, own_both, own_seg_only, own_height_only);
            }
        }
        scene_terrain_query_census() = SceneTerrainQueryCensus{};
        scene_terrain_quadtree_census() = SceneTerrainQuadtreeCensus{};
    }
    // Packet cc9_ai_owner_player_slot: every authored OwnerPlayer other than
    // 9, by entity name, for the AI coordinator's 009FFD20 (unit+180h).
    {
        std::vector<std::pair<std::string, int>> owners;
        for (const GameSceneEntityRecord& entity : impl.entities) {
            if (entity.owner_player != 9) owners.emplace_back(entity.name, entity.owner_player);
        }
        impl.log.notef("scene owner players: %zu authored non-default (unit+180h, 0077F1F9), "
            "%zu unresolved symbol(s)", owners.size(), impl.summary.owner_player_unresolved);
        for (const auto& row : owners) {
            impl.log.notef("  scene owner player %s = %d", row.first.c_str(), row.second);
        }
        ai_publish_scene_owner_players(owners);
    }
    // The wings 007F4580 spawned, flushed after the census loops so the scene
    // tallies stay a count of scene rows: a member plane is not a scene entity,
    // it is what a scene entity's slot-39 attach made.
    if (bsp::plane_squadron_registry().size() != 0) {
        const std::size_t before = impl.entities.size();
        impl.entities.insert(impl.entities.end(),
            std::make_move_iterator(impl.pending_squadron_members.begin()),
            std::make_move_iterator(impl.pending_squadron_members.end()));
        impl.log.notef("plane squadrons: %zu squadron(s) over %d member plane(s); %zu "
            "wing record(s) appended to the %zu scene record(s) for create_units "
            "(007F4580 mode 1 per created PlaneSquadronGen row)",
            bsp::plane_squadron_registry().size(),
            bsp::plane_squadron_registry().total_planned_members(),
            impl.entities.size() - before, before);
        impl.pending_squadron_members.clear();
    }
}

}  // namespace bsp::game
