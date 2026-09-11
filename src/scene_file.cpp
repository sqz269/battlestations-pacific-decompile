// The .scn scene-file reader. See docs/SCENE_FILE_READER.md.
// Addresses: 0046df00, 0046cf40, 00469bf0, 00467e10, 00469e40, 0046a9f0,
//            0046aab0, 008d9cf0, 008d8a70, 008d8960, 008d9930, 008d9980,
//            008d9ad0, 008d9b40.
#include "bsp/scene_file.hpp"

#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <utility>

namespace bsp {
namespace {

bool equal_insensitive(const std::string& a, const char* b) noexcept
{
    // 00438e10, the case-insensitive compare every keyword test goes through.
    std::size_t i = 0;
    for (; i < a.size(); ++i) {
        const unsigned char lhs = static_cast<unsigned char>(a[i]);
        const unsigned char rhs = static_cast<unsigned char>(b[i]);
        if (rhs == 0) {
            return false;
        }
        if (std::tolower(lhs) != std::tolower(rhs)) {
            return false;
        }
    }
    return b[i] == 0;
}

std::string format_error(int line, const char* what, const std::string& got)
{
    char buffer[192];
    std::snprintf(buffer, sizeof buffer, "line %d: expected %s, got \"%.80s\"", line, what,
        got.c_str());
    return std::string(buffer);
}

// Cap matching the native's own give-up point only in spirit: the native keeps
// reporting, but a reconstruction that never terminates is worse than one that
// stops. Documented in docs/SCENE_FILE_READER.md as a deliberate divergence.
constexpr std::size_t kMaxRecoveredErrors = 256;

class Cursor {
public:
    Cursor(SceneLexer& lexer, std::vector<std::string>& errors) : lexer_(lexer), errors_(errors) {}

    SceneLexer& lexer() noexcept { return lexer_; }

    bool at(const char* text)
    {
        const SceneToken& t = lexer_.peek();
        return !t.is_end() && equal_insensitive(t.text, text);
    }

    bool at_end() { return lexer_.peek().is_end(); }

    // 008d9930: consume only on a match; otherwise record and leave in place.
    bool expect(const char* text)
    {
        const SceneToken& t = lexer_.peek();
        if (equal_insensitive(t.text, text)) {
            lexer_.next();
            return true;
        }
        record(format_error(t.line, text, t.text));
        return false;
    }

    // 008d9980: any non-empty token; an empty unquoted token is end of input.
    std::string read_token()
    {
        const SceneToken& t = lexer_.peek();
        if (t.is_end()) {
            record(format_error(t.line, "a token", t.text));
            return std::string();
        }
        return lexer_.next().text;
    }

    std::int32_t read_int()
    {
        const SceneToken& t = lexer_.peek();
        std::int32_t value = 0;
        if (!scene_scan_int(t.text, value)) {
            record(format_error(t.line, "an integer", t.text));
            return 0;
        }
        lexer_.next();
        return value;
    }

    float read_float()
    {
        const SceneToken& t = lexer_.peek();
        float value = 0.0f;
        if (!scene_scan_float(t.text, value)) {
            record(format_error(t.line, "a float", t.text));
            return 0.0f;
        }
        lexer_.next();
        return value;
    }

    bool exhausted() const noexcept { return errors_.size() >= kMaxRecoveredErrors; }

private:
    void record(std::string message)
    {
        if (errors_.size() < kMaxRecoveredErrors) {
            errors_.push_back(std::move(message));
        }
    }

    SceneLexer& lexer_;
    std::vector<std::string>& errors_;
};

ScenePropertyBlock parse_property_body(Cursor& cur);

// The optional "( g1 g2 ... )" group list, then "{ body }". 008f5a00.
void parse_property_section(Cursor& cur, std::vector<std::string>* groups,
    ScenePropertyBlock& out)
{
    if (cur.at("(")) {
        cur.expect("(");
        while (!cur.at(")") && !cur.at_end() && !cur.exhausted()) {
            const std::string name = cur.read_token();
            if (groups != nullptr && !name.empty()) {
                groups->push_back(name);
            }
        }
        cur.expect(")");
    }
    cur.expect("{");
    out = parse_property_body(cur);
}

ScenePropertyBlock parse_property_body(Cursor& cur)
{
    ScenePropertyBlock block;
    while (!cur.at("}")) {
        if (cur.at_end() || cur.exhausted()) {
            break;
        }
        const std::string key = cur.read_token();
        if (cur.at("{")) {
            cur.expect("{");
            block.blocks.emplace_back(key, parse_property_body(cur));
            continue;
        }
        if (!cur.at("=")) {
            // 008f66b9: neither '=' nor '{' - drop the key and resume.
            continue;
        }
        cur.expect("=");
        SceneProperty prop;
        prop.key = key;
        prop.type_letter = cur.read_token();
        // 008f5a00 reads a fixed number of value tokens for the letter it
        // dispatched on and then runs ExpectToken(";"), which peeks and
        // consumes only on a match (008d9930): a property authored without its
        // terminator costs one reported error and nothing else. This generic
        // scan stands in for the per-letter reads, so it has to stop at a brace
        // as well as at ';' - no value of any letter contains one. Without the
        // brace stop the scan swallows the closing brace of the block it is in
        // and every later block nests one level too deep; that is what cost the
        // nine entities in scene175.scn and scene907.scn (packet
        // cc_scene_records, docs/SCENE_FILE_READER.md "Corrections").
        while (!cur.at(";") && !cur.at("{") && !cur.at("}") && !cur.at_end()
            && !cur.exhausted()) {
            prop.values.push_back(cur.read_token());
        }
        cur.expect(";");
        block.values.push_back(std::move(prop));
    }
    cur.expect("}");
    return block;
}

// The body of a block this packet did not decode: balance braces and count the
// tokens consumed. Used for SceneBrowserGroups (004694f0) and traffic.
std::size_t skip_balanced_block(SceneLexer& lexer)
{
    std::size_t tokens = 0;
    int depth = 0;
    if (lexer.peek().text == "{") {
        lexer.next();
        depth = 1;
    }
    while (depth > 0 && !lexer.at_end()) {
        const SceneToken t = lexer.next();
        ++tokens;
        if (t.quoted) {
            continue;
        }
        if (t.text == "{") {
            ++depth;
        } else if (t.text == "}") {
            --depth;
        }
    }
    return tokens;
}

SceneEntity parse_entity(Cursor& cur)
{
    SceneEntity ent;
    ent.name = cur.read_token();
    cur.expect("(");
    ent.class_name = cur.read_token();
    cur.expect(")");
    cur.expect("{");
    cur.expect("localframe");
    for (int i = 0; i < 16; ++i) {
        ent.frame[i] = cur.read_float();
    }
    cur.expect(";");
    if (cur.at("template")) {
        cur.expect("template");
        ent.template_path = cur.read_token();
        ent.has_template = true;
        cur.expect(";");
    }
    if (cur.at("uid")) {
        cur.expect("uid");
        ent.uid = cur.read_int();
        ent.has_uid = true;
        cur.expect(";");
    }
    cur.expect("properties");
    parse_property_section(cur, &ent.groups, ent.properties);
    while (cur.at("entity") && !cur.exhausted()) {
        cur.expect("entity");
        ent.children.push_back(parse_entity(cur));
    }
    cur.expect("}");
    return ent;
}

void multiply_frames(const float parent[16], const float local[16], float out[16])
{
    // 00413920 composes the child's localframe with the parent's world frame.
    // The 16 floats are a column-major 4x4 with the translation in the last
    // row, matching how the shipped files author them.
    for (int row = 0; row < 4; ++row) {
        for (int col = 0; col < 4; ++col) {
            float sum = 0.0f;
            for (int k = 0; k < 4; ++k) {
                sum += local[row * 4 + k] * parent[k * 4 + col];
            }
            out[row * 4 + col] = sum;
        }
    }
}

void visit_entity(SceneFileReaderHost& host, const SceneEntity& ent, const float parent[16],
    SceneFilePass pass, std::size_t& visited)
{
    float world[16];
    multiply_frames(parent, ent.frame, world);
    ++visited;
    host.instantiate_entity(ent, world, pass);
    for (const SceneEntity& child : ent.children) {
        visit_entity(host, child, world, pass, visited);
    }
}

}  // namespace

// ---------------------------------------------------------------------------
// Key tables
// ---------------------------------------------------------------------------

const SceneKeyRow kSceneTopLevelKeys[5] = {
    {"header", SceneTopLevelKey::Header, 0x00469bf0u, "00ce5994"},
    {"entity", SceneTopLevelKey::Entity, 0x0046cf40u, "00ce5818"},
    {"traffic", SceneTopLevelKey::Traffic, 0x009514b0u, "00ce5890"},
    {"groups", SceneTopLevelKey::Groups, 0x00467e10u, "00ce55e4"},
    {"SceneBrowserGroups", SceneTopLevelKey::SceneBrowserGroups, 0x00469e40u, "00ce56a4"},
};

const SceneHeaderKeyRow kSceneHeaderKeys[4] = {
    {"uniqueID", SceneHeaderKey::UniqueId, "int -> scene record +1098h"},
    {"NextUID", SceneHeaderKey::NextUid, "int, read and discarded"},
    {"properties", SceneHeaderKey::Properties, "property bag published as SceneRootProps"},
    {"precache", SceneHeaderKey::Precache, "property bag; sets scene record +905h"},
};

const SceneEntityKeyRow kSceneEntityKeys[4] = {
    {"localframe", false, "16 floats then ';'"},
    {"template", true, "one token then ';'"},
    {"uid", true, "one int then ';'"},
    {"properties", false, "optional ( groups ) then { body }"},
};

const SceneTerrainShadowKeyRow kSceneTerrainShadowKeys[4] = {
    {"g_StaticShadowTexture", "g_Terrain.g_StaticShadowTexture", true},
    {"ga_StaticShadowShotOffsetX", "g_Terrain.ga_StaticShadowShotOffsetX", false},
    {"ga_StaticShadowShotOffsetZ", "g_Terrain.ga_StaticShadowShotOffsetZ", false},
    {"ga_StaticShadowShotSize", "g_Terrain.ga_StaticShadowShotSize", false},
};

const char* const kSceneRegistrationClassNames[4] = {
    "CommandBuilding",
    "LandVehicle",
    "LandFort",
    "VehicleClass",
};

// ---------------------------------------------------------------------------
// Lexer
// ---------------------------------------------------------------------------

SceneLexer::SceneLexer(std::string text, std::string delimiters)
    : text_(std::move(text)), delims_(std::move(delimiters))
{
}

char SceneLexer::get()
{
    if (pos_ >= text_.size()) {
        return '\0';
    }
    const char c = text_[pos_++];
    if (c == '\n') {
        ++line_;
    }
    return c;
}

char SceneLexer::peek_char() const noexcept
{
    return pos_ < text_.size() ? text_[pos_] : '\0';
}

bool SceneLexer::is_space(char c) const noexcept
{
    if (c == '\0') {
        return false;
    }
    for (const char* p = kSceneWhitespace; *p != 0; ++p) {
        if (*p == c) {
            return true;
        }
    }
    return false;
}

bool SceneLexer::is_delim(char c) const noexcept
{
    if (c == '\0') {
        return false;
    }
    return delims_.find(c) != std::string::npos;
}

const SceneToken& SceneLexer::peek()
{
    if (!cached_) {
        cache_ = scan();
        cached_ = true;
    }
    return cache_;
}

SceneToken SceneLexer::next()
{
    peek();
    cached_ = false;
    return cache_;
}

bool SceneLexer::recover_after_failed_read()
{
    peek();
    while (pos_ < text_.size() && is_space(peek_char())) {
        get();
    }
    return pos_ >= text_.size();
}

SceneToken SceneLexer::scan()
{
    for (;;) {
        const char c = get();
        if (c == '\0') {
            SceneToken t;
            t.line = line_;
            return t;
        }
        if (is_space(c)) {
            continue;
        }
        if (c == '/') {
            const char nxt = peek_char();
            if (nxt == '/') {
                while (pos_ < text_.size() && text_[pos_] != '\n') {
                    get();
                }
                continue;
            }
            if (nxt == '*') {
                get();
                // The native previous/current-byte pair initially contains
                // '/' and '*', so even the overlapping terminator in /*/ ends
                // this comment (008d8c60-008d8cba).
                char prev = '*';
                while (pos_ < text_.size()) {
                    const char ch = get();
                    if (prev == '*' && ch == '/') {
                        break;
                    }
                    prev = ch;
                }
                continue;
            }
        }
        if (is_delim(c)) {
            SceneToken t;
            t.text.assign(1, c);
            t.line = line_;
            return t;
        }
        if (c == '"') {
            SceneToken t;
            t.quoted = true;
            t.line = line_;
            while (pos_ < text_.size()) {
                const char ch = get();
                if (ch == '"') {
                    break;
                }
                t.text.push_back(ch);
            }
            return t;
        }
        SceneToken t;
        t.line = line_;
        t.text.push_back(c);
        while (pos_ < text_.size()) {
            const char ch = peek_char();
            if (is_space(ch) || is_delim(ch)) {
                break;
            }
            // 008d8dd2-008d8e3a only checks whitespace and delimiters here;
            // comment openers inside an unquoted token are ordinary text.
            t.text.push_back(get());
        }
        return t;
    }
}

// ---------------------------------------------------------------------------
// Scalar conversions
// ---------------------------------------------------------------------------

#if defined(_MSC_VER)
#pragma warning(push)
// The native reader uses sscanf; the integer-only format has no buffer output.
#pragma warning(disable : 4996)
#endif
bool scene_scan_int(const std::string& text, std::int32_t& out) noexcept
{
    // 008d9ad0 uses %d directly, including CRT whitespace inside a quoted
    // token. Keep output unchanged when conversion fails.
    int value = 0;
    if (std::sscanf(text.c_str(), "%d", &value) != 1) {
        return false;
    }
    out = static_cast<std::int32_t>(value);
    return true;
}
#if defined(_MSC_VER)
#pragma warning(pop)
#endif

bool scene_scan_float(const std::string& text, float& out) noexcept
{
    std::size_t i = 0;
    if (i < text.size() && (text[i] == '+' || text[i] == '-')) {
        ++i;
    }
    std::size_t digits = 0;
    while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
        ++i;
        ++digits;
    }
    if (i < text.size() && text[i] == '.') {
        ++i;
        while (i < text.size() && std::isdigit(static_cast<unsigned char>(text[i])) != 0) {
            ++i;
            ++digits;
        }
    }
    if (digits == 0) {
        return false;
    }
    std::size_t j = i;
    if (j < text.size() && (text[j] == 'e' || text[j] == 'E')) {
        ++j;
        if (j < text.size() && (text[j] == '+' || text[j] == '-')) {
            ++j;
        }
        const std::size_t exp_start = j;
        while (j < text.size() && std::isdigit(static_cast<unsigned char>(text[j])) != 0) {
            ++j;
        }
        if (j > exp_start) {
            i = j;
        }
    }
    out = static_cast<float>(std::strtod(text.substr(0, i).c_str(), nullptr));
    return true;
}

// ---------------------------------------------------------------------------
// Block parsers
// ---------------------------------------------------------------------------

const SceneProperty* ScenePropertyBlock::find(const std::string& key) const noexcept
{
    for (const SceneProperty& prop : values) {
        if (equal_insensitive(prop.key, key.c_str())) {
            return &prop;
        }
    }
    return nullptr;
}

SceneHeader parse_scene_header_00469bf0(SceneLexer& lexer, std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    SceneHeader header;
    cur.expect("{");
    while (!cur.at("}") && !cur.at_end() && !cur.exhausted()) {
        const std::string key = cur.read_token();
        if (equal_insensitive(key, "NextUID")) {
            header.next_uid = cur.read_int();
            header.has_next_uid = true;
            cur.expect(";");
        } else if (equal_insensitive(key, "uniqueID")) {
            header.unique_id = cur.read_int();
            header.has_unique_id = true;
            cur.expect(";");
        } else if (equal_insensitive(key, "properties")) {
            parse_property_section(cur, nullptr, header.properties);
            header.has_properties = true;
        } else if (equal_insensitive(key, "precache")) {
            parse_property_section(cur, nullptr, header.precache);
            header.has_precache = true;
        }
        // Any other key is consumed and ignored, as in 00469bf0.
    }
    cur.expect("}");
    return header;
}

SceneEntity parse_scene_entity_0046cf40(SceneLexer& lexer, std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    return parse_entity(cur);
}

std::vector<SceneGroupEntry> parse_scene_groups_00467e10(SceneLexer& lexer,
    std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    std::vector<SceneGroupEntry> out;
    cur.expect("{");
    while (!cur.at("}") && !cur.at_end() && !cur.exhausted()) {
        cur.expect("group");
        cur.expect("{");
        SceneGroupEntry entry;
        cur.expect("GroupName");
        cur.expect("=");
        entry.name = cur.read_token();
        cur.expect("Index");
        cur.expect("=");
        entry.index = cur.read_int();
        cur.expect("}");
        out.push_back(std::move(entry));
    }
    cur.expect("}");
    return out;
}

ScenePropertyBlock parse_scene_property_block_008f5a00(SceneLexer& lexer,
    std::vector<std::string>& errors)
{
    Cursor cur(lexer, errors);
    ScenePropertyBlock block;
    parse_property_section(cur, nullptr, block);
    return block;
}

SceneDocument parse_scene_document(SceneLexer& lexer)
{
    SceneDocument doc;
    Cursor cur(lexer, doc.errors);
    while (!cur.at_end() && !cur.exhausted()) {
        const SceneToken token = lexer.next();
        if (equal_insensitive(token.text, "header")) {
            doc.header = parse_scene_header_00469bf0(lexer, doc.errors);
            doc.has_header = true;
        } else if (equal_insensitive(token.text, "entity")) {
            doc.entities.push_back(parse_entity(cur));
        } else if (equal_insensitive(token.text, "groups")) {
            const std::vector<SceneGroupEntry> groups
                = parse_scene_groups_00467e10(lexer, doc.errors);
            doc.groups.insert(doc.groups.end(), groups.begin(), groups.end());
            doc.has_groups = true;
        } else if (equal_insensitive(token.text, "SceneBrowserGroups")) {
            doc.browser_group_tokens += skip_balanced_block(lexer);
        } else if (equal_insensitive(token.text, "traffic")) {
            ++doc.traffic_blocks;
            skip_balanced_block(lexer);
        }
        // Anything else is ignored, as in 0046df00's top-level loop.
    }
    return doc;
}

SceneDocument parse_scene_document(const std::string& text)
{
    SceneLexer lexer(text);
    return parse_scene_document(lexer);
}

// ---------------------------------------------------------------------------
// Path split
// ---------------------------------------------------------------------------

SceneDatabasePath split_scene_path_0046df00(const std::string& scene_path)
{
    SceneDatabasePath out;
    out.full = scene_path;
    if (scene_path.empty()) {
        return out;  // 0046df00 leaves the stem empty and copies 00ce3a0c ("").
    }
    std::size_t n = scene_path.size();
    while (n != 0) {
        if (scene_path[n - 1] == '.') {
            out.stem = scene_path.substr(0, n - 1);
            return out;
        }
        --n;
    }
    out.stem = scene_path;
    return out;
}

// ---------------------------------------------------------------------------
// Passes
// ---------------------------------------------------------------------------

SceneFilePassFlags scene_pass_flags(SceneFilePass pass) noexcept
{
    SceneFilePassFlags flags;
    flags.instantiate = pass == SceneFilePass::Instantiate;
    flags.registration = pass == SceneFilePass::Registration;
    return flags;
}

SceneFilePass scene_pass_from_flags(bool instantiate, bool registration) noexcept
{
    if (registration) {
        return SceneFilePass::Registration;
    }
    if (instantiate) {
        return SceneFilePass::Instantiate;
    }
    return SceneFilePass::Header;
}

bool scene_pass_runs_entity_loop(SceneFilePass pass) noexcept
{
    return pass != SceneFilePass::Header;
}

bool scene_pass_stops_at_tail_blocks(SceneFilePass pass) noexcept
{
    return pass == SceneFilePass::Registration;
}

SceneFileReadResult run_scene_file_reader_0046df00(SceneFileReaderHost& host,
    const std::string& scene_path, const std::string& override_name, SceneFilePass pass)
{
    SceneFileReadResult result;
    host.clear_pending_references();  // 0046a9f0 on database+14Ch
    result.path = split_scene_path_0046df00(scene_path);

    result.weather_descriptor = host.select_weather_descriptor(scene_path, override_name);
    if (override_name.empty()) {
        for (const SceneTerrainShadowKeyRow& row : kSceneTerrainShadowKeys) {
            if (row.is_string) {
                host.set_terrain_shadow_string(row.target, std::string());
            } else {
                host.set_terrain_shadow_float(row.target, 0.0f);
            }
        }
    }

    std::string text;
    result.opened = host.read_scene_file(scene_path, text);
    SceneLexer lexer(text);
    SceneDocument& doc = result.document;
    Cursor cur(lexer, doc.errors);

    // The header is read by every pass; the tail is where they diverge.
    if (cur.at("header")) {
        cur.expect("header");
        doc.header = parse_scene_header_00469bf0(lexer, doc.errors);
        doc.has_header = true;
        if (doc.header.has_properties) {
            host.publish_scene_root_properties(doc.header.properties);
        }
        if (doc.header.has_precache) {
            result.record.precache_seen = true;
            host.publish_scene_precache(doc.header.precache);
        }
        if (doc.header.has_unique_id) {
            result.record.mission_id = doc.header.unique_id;
            host.set_scene_mission_id(doc.header.unique_id);
        }
    }

    if (!scene_pass_runs_entity_loop(pass)) {
        return result;
    }

    static const float kIdentity[16] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f,
    };
    bool tail_begun = false;
    bool stop = false;
    while (!stop && !cur.at_end() && !cur.exhausted()) {
        const SceneToken token = lexer.next();
        if (equal_insensitive(token.text, "entity")) {
            SceneEntity ent = parse_entity(cur);
            visit_entity(host, ent, kIdentity, pass, result.entities_visited);
            doc.entities.push_back(std::move(ent));
            continue;
        }
        if (equal_insensitive(token.text, "traffic")) {
            ++doc.traffic_blocks;
            if (scene_pass_stops_at_tail_blocks(pass)) {
                host.skip_traffic_block(lexer);  // 0095ca10, then the loop ends
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            host.load_traffic_block(lexer);
            continue;
        }
        if (equal_insensitive(token.text, "groups")) {
            if (scene_pass_stops_at_tail_blocks(pass)) {
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            const std::vector<SceneGroupEntry> groups
                = parse_scene_groups_00467e10(lexer, doc.errors);
            doc.groups.insert(doc.groups.end(), groups.begin(), groups.end());
            doc.has_groups = true;
            host.load_groups(doc.groups);
            continue;
        }
        if (equal_insensitive(token.text, "SceneBrowserGroups")) {
            if (scene_pass_stops_at_tail_blocks(pass)) {
                stop = true;
                continue;
            }
            if (!tail_begun) {
                host.begin_tail_blocks();
                tail_begun = true;
            }
            host.load_browser_groups(lexer);
            continue;
        }
        // Unknown top-level keyword: ignored, as in the native loop.
    }
    if (!tail_begun) {
        host.end_tail_blocks();  // 00925f20 runs once either way
    }
    if (pass == SceneFilePass::Instantiate) {
        host.resolve_deferred_references();  // 0046aab0
    }
    return result;
}

}  // namespace bsp
