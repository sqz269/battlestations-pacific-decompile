// Fixture check for the mission load path against one installed mission.
//
// The probe runs the reconstructed .scn reader (bsp/scene_file.hpp, 0046df00)
// over a shipped scene file, resolves every class token through the
// reconstructed class table (bsp/scene_entity_factory.hpp, 004f2800), and
// reports the entity count per class. It then derives the load-path facts the
// mission would use: the short name every VFS block is keyed on, the five
// numbered block names, the mission script path and the mission id.
//
// Default mission: universe/Scenes/missions/USN/usn_2_java.scn, the second USN
// campaign mission ("Battle of the Java Sea"), chosen because it is the
// smallest single-player campaign scene at 48680 bytes. Its mission-tree entry
// is scripts/datatables/missiontree.lua line 3404, whose "sceneFile" key is
// sceneFilePath .. "USN/usn_2_java.scn" with sceneFilePath =
// "universe/Scenes/missions/". Override both with argv.
//
// This proves the reader and the class table against real data; it does not run
// the native code, so it says nothing about the parts of the load that are
// still host methods.

#include "bsp/mission_load_path.hpp"
#include "bsp/mission_scene_load.hpp"
#include "bsp/scene_entity_factory.hpp"
#include "bsp/scene_file.hpp"
#include "bsp/scene_record_side_blocks.hpp"

#include <algorithm>
#include <cctype>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace {

// config/target.json's installed copy. Overridden by argv[1].
constexpr const char kDefaultGameRoot[]
    = "I:/SteamLibrary/steamapps/common/Battlestations Pacific";

// The scene path exactly as the mission-tree record carries it, which is what
// reaches 0046df00 and derive_scene_short_name. Overridden by argv[2].
constexpr const char kDefaultScenePath[] = "universe/Scenes/missions/USN/usn_2_java.scn";

struct ClassTally {
    std::size_t total{0};
    std::size_t with_template{0};
    std::size_t with_uid{0};
    std::size_t with_multitype{0};
};

void tally(const std::vector<bsp::SceneEntity>& entities,
    std::map<std::string, ClassTally>& out, std::size_t& depth_max, std::size_t depth)
{
    depth_max = std::max(depth_max, depth);
    for (const bsp::SceneEntity& entity : entities) {
        ClassTally& row = out[entity.class_name];
        ++row.total;
        if (entity.has_template) {
            ++row.with_template;
        }
        if (entity.has_uid) {
            ++row.with_uid;
        }
        if (bsp::find_scene_property_block(entity.properties, bsp::kSceneMultiTypeKey) != nullptr) {
            ++row.with_multitype;
        }
        tally(entity.children, out, depth_max, depth + 1);
    }
}

// ---------------------------------------------------------------------------
// --sweep: every installed .scn, parsed and counted
// ---------------------------------------------------------------------------

// The lexical rule docs/SCENE_ENTITY_FACTORY.md counted with: a quoted name,
// then `( <Class> )`. It is deliberately not the grammar: it also matches
// entity headers inside regions the recovering parser skipped, which is the
// whole point of running both counts side by side.
struct LexicalHeader {
    std::string name;
    std::string class_name;
    std::size_t line{0};
    std::size_t depth{0};  // brace nesting the header sits at, quotes excluded
};

std::vector<LexicalHeader> lexical_entity_headers(const std::string& text)
{
    std::vector<LexicalHeader> found;
    std::size_t i = 0;
    std::size_t line = 1;
    std::size_t depth = 0;
    while (i < text.size()) {
        if (text[i] == '\n') {
            ++line;
            ++i;
            continue;
        }
        if (text[i] == '{') {
            ++depth;
            ++i;
            continue;
        }
        if (text[i] == '}') {
            if (depth != 0) {
                --depth;
            }
            ++i;
            continue;
        }
        if (text[i] != '"') {
            ++i;
            continue;
        }
        const std::size_t close = text.find('"', i + 1);
        if (close == std::string::npos) {
            break;
        }
        const std::string name = text.substr(i + 1, close - i - 1);
        std::size_t j = close + 1;
        while (j < text.size() && std::isspace(static_cast<unsigned char>(text[j])) != 0) {
            ++j;
        }
        if (j < text.size() && text[j] == '(') {
            ++j;
            while (j < text.size() && std::isspace(static_cast<unsigned char>(text[j])) != 0) {
                ++j;
            }
            const std::size_t name_start = j;
            while (j < text.size()
                && (std::isalnum(static_cast<unsigned char>(text[j])) != 0 || text[j] == '_')) {
                ++j;
            }
            const std::string class_name = text.substr(name_start, j - name_start);
            while (j < text.size() && std::isspace(static_cast<unsigned char>(text[j])) != 0) {
                ++j;
            }
            if (!class_name.empty() && j < text.size() && text[j] == ')') {
                LexicalHeader header;
                header.name = name;
                header.class_name = class_name;
                header.line = line;
                header.depth = depth;
                found.push_back(header);
            }
        }
        for (std::size_t k = i; k < close; ++k) {
            if (text[k] == '\n') {
                ++line;
            }
        }
        i = close + 1;
    }
    return found;
}

void collect_entity_keys(const std::vector<bsp::SceneEntity>& entities,
    std::map<std::string, std::size_t>& out)
{
    for (const bsp::SceneEntity& entity : entities) {
        ++out[entity.class_name + "\x1f" + entity.name];
        collect_entity_keys(entity.children, out);
    }
}

std::size_t count_entities(const std::vector<bsp::SceneEntity>& entities)
{
    std::size_t total = entities.size();
    for (const bsp::SceneEntity& entity : entities) {
        total += count_entities(entity.children);
    }
    return total;
}

struct SweepTotals {
    std::size_t files{0};
    std::size_t parsed_entities{0};
    std::size_t lexical_entities{0};
    std::size_t files_with_errors{0};
    std::size_t recovered_errors{0};
    std::size_t files_with_multiplay{0};
    std::size_t authored_units{0};
    std::size_t authored_pools{0};
};

int run_sweep(const std::string& root)
{
    namespace fs = std::filesystem;
    std::error_code ec;
    if (!fs::is_directory(root, ec)) {
        std::cout << "game root not found: " << root << '\n';
        return 2;
    }

    SweepTotals totals;
    std::map<std::string, ClassTally> classes;
    std::map<std::int32_t, std::size_t> max_player_num;
    std::vector<std::string> mismatches;
    std::vector<std::string> files;

    for (fs::recursive_directory_iterator it(root, fs::directory_options::skip_permission_denied, ec),
         end;
         it != end; it.increment(ec)) {
        if (ec) {
            break;
        }
        if (!it->is_regular_file(ec)) {
            continue;
        }
        std::string ext = it->path().extension().string();
        for (char& c : ext) {
            c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        }
        if (ext == ".scn") {
            files.push_back(it->path().string());
        }
    }
    std::sort(files.begin(), files.end());

    for (const std::string& path : files) {
        std::ifstream input(path, std::ios::binary);
        if (!input) {
            std::cout << "unreadable: " << path << '\n';
            return 2;
        }
        std::ostringstream buffer;
        buffer << input.rdbuf();
        const std::string text = buffer.str();

        const bsp::SceneDocument document = bsp::parse_scene_document(text);
        std::size_t depth = 0;
        tally(document.entities, classes, depth, 1);
        const std::size_t parsed = count_entities(document.entities);
        const std::vector<LexicalHeader> headers = lexical_entity_headers(text);
        const std::size_t lexical = headers.size();

        ++totals.files;
        totals.parsed_entities += parsed;
        totals.lexical_entities += lexical;
        if (!document.errors.empty()) {
            ++totals.files_with_errors;
            totals.recovered_errors += document.errors.size();
        }
        if (parsed != lexical) {
            std::ostringstream row;
            row << path << " parsed=" << parsed << " lexical=" << lexical << " delta="
                << (static_cast<long long>(lexical) - static_cast<long long>(parsed))
                << " recovered-errors=" << document.errors.size();
            if (!document.errors.empty()) {
                row << " first=\"" << document.errors.front() << "\"";
            }
            // Which authored headers the grammar did not turn into entities.
            std::map<std::string, std::size_t> built;
            collect_entity_keys(document.entities, built);
            for (const LexicalHeader& header : headers) {
                const std::string key = header.class_name + "\x1f" + header.name;
                auto it = built.find(key);
                if (it != built.end() && it->second != 0) {
                    --it->second;
                    continue;
                }
                row << "\n      line " << header.line << " depth " << header.depth << ": \""
                    << header.name << "\" (" << header.class_name << ")";
            }
            mismatches.push_back(row.str());
        }

        if (document.has_header && document.header.has_properties) {
            const bsp::SceneRecordSlotTable slots
                = bsp::read_scene_record_slot_table_004f1d70(document.header.properties);
            ++max_player_num[slots.max_player_num];
            if (slots.multiplay_block_present) {
                ++totals.files_with_multiplay;
                for (const bsp::SceneSlotBlock& block : slots.blocks) {
                    for (const bsp::SceneSlotUnit& unit : block.units) {
                        if (unit.present) {
                            ++totals.authored_units;
                        }
                        for (const bsp::SceneSlotPoolEntry& pool : unit.pools) {
                            if (pool.present) {
                                ++totals.authored_pools;
                            }
                        }
                    }
                }
            }
        }
    }

    std::size_t class_total = 0;
    std::cout << "entities per class across " << totals.files << " files:\n";
    for (const auto& row : classes) {
        class_total += row.second.total;
        std::cout << "  " << row.first << " x" << row.second.total
                  << " multitype=" << row.second.with_multitype << '\n';
    }
    std::cout << "parsed entities=" << totals.parsed_entities
              << " (across classes " << class_total << ", distinct " << classes.size() << ")\n";
    std::cout << "lexical entity headers=" << totals.lexical_entities << " delta="
              << (static_cast<long long>(totals.lexical_entities)
                     - static_cast<long long>(totals.parsed_entities))
              << '\n';
    std::cout << "files with recovered errors=" << totals.files_with_errors << " (errors "
              << totals.recovered_errors << ") files where the two counts differ="
              << mismatches.size() << '\n';
    for (const std::string& row : mismatches) {
        std::cout << "  " << row << '\n';
    }
    std::cout << "record+988h MaxPlayerNum:";
    for (const auto& row : max_player_num) {
        std::cout << " " << row.first << "x" << row.second;
    }
    std::cout << "\nfiles with a MultiPlay block=" << totals.files_with_multiplay
              << " authored UnitN entries=" << totals.authored_units
              << " authored PoolN entries=" << totals.authored_pools << '\n';
    return 0;
}

} // namespace

int main(int argc, char** argv)
{
    if (argc > 1 && std::string(argv[1]) == "--sweep") {
        return run_sweep(argc > 2 ? argv[2] : kDefaultGameRoot);
    }

    const std::string root = argc > 1 ? argv[1] : kDefaultGameRoot;
    const std::string scene_path = argc > 2 ? argv[2] : kDefaultScenePath;
    const std::string disk_path = root + "/" + scene_path;

    std::ifstream input(disk_path, std::ios::binary);
    if (!input) {
        std::cout << "scene file not found: " << disk_path << '\n';
        return 2;
    }
    std::ostringstream buffer;
    buffer << input.rdbuf();
    const std::string text = buffer.str();

    std::cout << "scene: " << scene_path << " (" << text.size() << " bytes)\n";

    const bsp::SceneDocument document = bsp::parse_scene_document(text);

    if (!document.has_header) {
        std::cout << "no header block\n";
        return 3;
    }
    std::cout << "header uniqueID=" << document.header.unique_id
              << " (mission id, record+1098h)"
              << " nextUID=" << document.header.next_uid
              << " properties=" << (document.header.has_properties ? "yes" : "no")
              << " precache=" << (document.header.has_precache ? "yes" : "no") << '\n';

    std::map<std::string, ClassTally> classes;
    std::size_t depth_max = 0;
    tally(document.entities, classes, depth_max, 1);

    std::size_t total = 0;
    std::size_t unregistered = 0;
    std::size_t registration_pass_kept = 0;
    std::cout << "entities per class (top-level blocks " << document.entities.size()
              << ", nesting depth " << depth_max << "):\n";
    for (const auto& row : classes) {
        const int class_id = bsp::scene_entity_class_id_from_name(row.first);
        const bool registered = bsp::scene_entity_class_is_registered(row.first);
        const bool kept = registered
            && (bsp::scene_registration_pass_handles_class(class_id)
                || bsp::scene_registration_fallback_class(class_id));
        total += row.second.total;
        if (!registered) {
            unregistered += row.second.total;
        } else if (kept) {
            registration_pass_kept += row.second.total;
        }
        std::cout << "  " << row.first << " x" << row.second.total;
        if (registered) {
            const bsp::SceneEntityClassRow* descriptor
                = bsp::find_scene_entity_class_by_name(row.first);
            std::cout << " id=0x" << std::hex << class_id << std::dec;
            if (descriptor != nullptr && descriptor->name != row.first) {
                std::cout << " (table spells it " << descriptor->name << ")";
            }
            std::cout << (kept ? " registration-pass" : " instantiate-only");
        } else {
            std::cout << " UNREGISTERED";
        }
        std::cout << " template=" << row.second.with_template << " uid=" << row.second.with_uid
                  << " multitype=" << row.second.with_multitype << '\n';
    }
    std::cout << "entities total=" << total << " distinct classes=" << classes.size()
              << " registration-pass classes kept=" << registration_pass_kept
              << " unregistered=" << unregistered << '\n';
    std::cout << "groups=" << document.groups.size()
              << " traffic blocks=" << document.traffic_blocks
              << " browser group tokens=" << document.browser_group_tokens
              << " recovered errors=" << document.errors.size() << '\n';
    for (std::size_t i = 0; i < document.errors.size() && i < 8; ++i) {
        std::cout << "  error: " << document.errors[i] << '\n';
    }

    // The load-path facts this scene would produce.
    const std::string short_name = bsp::derive_scene_short_name(scene_path);
    const bsp::SceneDatabasePath split = bsp::split_scene_path_0046df00(scene_path);
    std::cout << "short name (004cd7f0) = \"" << short_name << "\"\n";
    std::cout << "database path +13Ch = \"" << split.full << "\"  stem +144h = \"" << split.stem
              << "\"\n";
    std::cout << "vfs blocks:";
    const bsp::MissionLoadPhase phases[] = {bsp::MissionLoadPhase::WorldConstruction,
        bsp::MissionLoadPhase::MissionScript, bsp::MissionLoadPhase::StageInit,
        bsp::MissionLoadPhase::EngineMovie, bsp::MissionLoadPhase::RendererHandoff};
    for (const bsp::MissionLoadPhase phase : phases) {
        std::cout << " \"" << bsp::mission_load_phase_block_name(phase, short_name) << "\"";
    }
    std::cout << "\n";

    // The mission script the load runs is the record's script-table entry for
    // the resolved slot, which for single player is always slot 8. The scene
    // file does not carry it; the name below is the installed file the tree
    // entry names, shown so the path rule can be checked by eye.
    std::cout << "mission script path (008860b0) for \"usn_2_java\" = \""
              << bsp::mission_script_path("usn_2_java") << "\"\n";

    // The side-block count the record would report is not in the .scn text; it
    // is filled by the native header pass from data this reader does not model.
    // What the probe can check is the slot arithmetic itself.
    std::cout << "slot record offsets:";
    for (std::size_t i = 0; i < bsp::kSceneSlotRecordCount; ++i) {
        std::cout << " 0x" << std::hex << bsp::scene_slot_record_offset(i) << std::dec;
    }
    std::cout << "\n";

    std::size_t step_count = 0;
    bsp::mission_load_host_steps(step_count);
    std::cout << "host methods the load needs: " << step_count << " ("
              << bsp::mission_load_external_step_count() << " need a subsystem owner)\n";

    if (unregistered != 0) {
        // A class token the table does not carry would fault the native reader
        // at 0046cf40 (it dereferences node+8 with no null check), so an
        // installed file producing one means the table is wrong, not the file.
        std::cout << "class table does not cover every token in this scene\n";
        return 5;
    }
    // Zero entities and recovered errors are both states the shipped files
    // reach legitimately: chg/usn_finale.scn carries only a header, a traffic
    // block and groups, and nine files have an authoring slip the native
    // recovers from the same way (008d9930 reports and leaves the token in
    // place). Neither is a failure of the reconstruction.
    return 0;
}
