#pragma once
// The .scn scene-file reader.
// Addresses: 0046df00, 0046cf40, 00469bf0, 00467e10, 00469e40, 0046a9f0,
//            0046aab0, 008d9cf0, 008d8a70, 008d8960, 008d9930, 008d9980,
//            008d9ad0, 008d9b40.
// Every name below is a hypothesis, not a recovered symbol. See
// docs/SCENE_FILE_READER.md for the evidence behind each claim.
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace bsp {

// ---------------------------------------------------------------------------
// Tokenizer (008d9cf0 constructs, 008d8a70 peeks, 008d8960 consumes)
// ---------------------------------------------------------------------------

// Whitespace class of 008d8a70, read through PTR_s___00e0c940 -> 00d15f2c.
// The comma is whitespace, not a delimiter: "properties (Common, Ship)" and
// "1.0, 2.0" tokenize identically to their space-separated forms.
inline constexpr const char* kSceneWhitespace = " \t\r\n,";

// Delimiter set 0046df00 hands to the tokenizer constructor (00ce4f40),
// concatenated after the built-in default ";" (00ce5698). Each of these is a
// token on its own.
inline constexpr const char* kSceneDelimiters = ";{}=:(,)";

// One token. `quoted` is tokenizer field +4, set only for a "..." run; it is
// what lets an empty quoted string be distinguished from end of input.
struct SceneToken {
    std::string text;
    bool quoted{false};
    int line{1};

    bool is_end() const noexcept { return text.empty() && !quoted; }
};

// Scanner of 008d8a70. One-token lookahead cache (field +0x805), "//" to end of
// line and "/* */" block comments at token boundaries, '"' strings with no escape
// processing, single-character delimiter tokens, and a line counter (+0x81c).
// The native buffers the whole file in memory first (008d9cf0 reads the VFS
// stream into +0x82c with size +0x830), so this takes the text directly.
class SceneLexer {
public:
    SceneLexer(std::string text, std::string delimiters);
    explicit SceneLexer(std::string text) : SceneLexer(std::move(text), kSceneDelimiters) {}

    const SceneToken& peek();
    SceneToken next();
    bool at_end() { return peek().is_end(); }
    int line() const noexcept { return line_; }

    // 008d8f70: retain the cached token, skip only following whitespace, and
    // report whether the byte source is exhausted. Options typed readers use
    // this result as their separate native +80Ah end flag.
    bool recover_after_failed_read();

private:
    SceneToken scan();
    char get();
    char peek_char() const noexcept;
    bool is_space(char c) const noexcept;
    bool is_delim(char c) const noexcept;

    std::string text_;
    std::string delims_;
    std::size_t pos_{0};
    int line_{1};
    bool cached_{false};
    SceneToken cache_;
};

// ---------------------------------------------------------------------------
// Parsed document
// ---------------------------------------------------------------------------

// One "<key> = <letter> <value...> ;" assignment of a property block
// (008f5a00). The letter is the authored type tag; the native validates it
// against the type the property descriptor already declares, so it is kept
// verbatim rather than interpreted here. Observed letters in the shipped
// files: S F I E B R V3 RPath RFort RPlnShp IA LUA_S.
struct SceneProperty {
    std::string key;
    std::string type_letter;
    std::vector<std::string> values;
};

// A property block: scalar assignments plus nested "Name" { ... } sub-blocks.
struct ScenePropertyBlock {
    std::vector<SceneProperty> values;
    std::vector<std::pair<std::string, ScenePropertyBlock>> blocks;

    const SceneProperty* find(const std::string& key) const noexcept;
};

// An entity record built by 0046cf40. `frame` is the 16 floats of `localframe`
// in file order (column-major 4x4 as authored).
struct SceneEntity {
    std::string name;           // the quoted name after `entity`
    std::string class_name;     // the token inside ( )
    std::string template_path;  // optional `template "..." ;`
    bool has_template{false};
    std::int32_t uid{0};        // optional `uid <int> ;`
    bool has_uid{false};
    float frame[16]{};
    std::vector<std::string> groups;  // the names inside `properties ( ... )`
    ScenePropertyBlock properties;
    std::vector<SceneEntity> children;  // nested `entity` blocks
};

// The `header` block parsed by 00469bf0.
struct SceneHeader {
    std::int32_t unique_id{0};
    bool has_unique_id{false};
    std::int32_t next_uid{0};
    bool has_next_uid{false};
    ScenePropertyBlock properties;  // registered as "SceneRootProps"
    bool has_properties{false};
    ScenePropertyBlock precache;  // sets scene-record byte +905h
    bool has_precache{false};
};

// One entry of the `groups` block parsed by 00467e10.
struct SceneGroupEntry {
    std::string name;   // GroupName = <token>
    std::int32_t index{0};  // Index = <int>
};

struct SceneDocument {
    SceneHeader header;
    bool has_header{false};
    std::vector<SceneEntity> entities;
    std::vector<SceneGroupEntry> groups;
    bool has_groups{false};
    std::size_t browser_group_tokens{0};  // SceneBrowserGroups body, not decoded
    std::size_t traffic_blocks{0};
    // Recovered (non-fatal) errors, in file order. 008d9930/008d9ad0/008d9b40
    // report and leave the offending token in place instead of aborting.
    std::vector<std::string> errors;
};

// ---------------------------------------------------------------------------
// Key tables, as data
// ---------------------------------------------------------------------------

// Top-level keywords 0046df00 dispatches on, with the handler that consumes
// each. Anything else is skipped by the top-level loop.
enum class SceneTopLevelKey { Header, Entity, Traffic, Groups, SceneBrowserGroups };
struct SceneKeyRow {
    const char* keyword;
    SceneTopLevelKey key;
    std::uint32_t handler;  // native address of the handler
    const char* literal;    // address of the literal, for the ledger
};
extern const SceneKeyRow kSceneTopLevelKeys[5];

// Keys 00469bf0 recognises inside `header`.
enum class SceneHeaderKey { UniqueId, NextUid, Properties, Precache };
struct SceneHeaderKeyRow {
    const char* keyword;
    SceneHeaderKey key;
    const char* effect;
};
extern const SceneHeaderKeyRow kSceneHeaderKeys[4];

// Keys 0046cf40 recognises inside an `entity` body, in the fixed order it
// reads them. `localframe` and `properties` are mandatory; `template` and
// `uid` are optional and tested by lookahead.
struct SceneEntityKeyRow {
    const char* keyword;
    bool optional;
    const char* effect;
};
extern const SceneEntityKeyRow kSceneEntityKeys[4];

// The four `g_Terrain.*` console variables 0046df00 writes from the matched
// weather descriptor when no override name was supplied. `source` is the key
// read from the descriptor table; `target` is the variable name looked up.
struct SceneTerrainShadowKeyRow {
    const char* source;
    const char* target;
    bool is_string;  // true writes a string, false writes the float at +0Ch
};
extern const SceneTerrainShadowKeyRow kSceneTerrainShadowKeys[4];

// Entity class names 0046df00 tests against a template name in the pass-2
// registration tail (00ce5858..00ce5880), plus the "CommandBuilding" test that
// short-circuits it.
extern const char* const kSceneRegistrationClassNames[4];

// Property key the landscape branch of 0046cf40 synthesises: with a non-zero
// `uid` on a class whose descriptor id is 0x44, the entity property bag gets
// "__EXTDATAPATH" = sprintf("%s_LS_%i.trn", sceneBaseName, uid). Every other
// entity gets "__EXTDATAPATH" = "".
inline constexpr const char* kSceneExtDataPathKey = "__EXTDATAPATH";
inline constexpr const char* kSceneLandscapeFileFormat = "%s_LS_%i.trn";
inline constexpr int kSceneLandscapeClassId = 0x44;

// Property keys 0046cf40 reads back out of a parsed entity bag.
inline constexpr const char* kSceneHiddenPropertyKey = "Hidden";
inline constexpr const char* kSceneTypePropertyKey = "Type";
inline constexpr const char* kScenePartyPropertyKey = "Party";

// ---------------------------------------------------------------------------
// Parser
// ---------------------------------------------------------------------------

// Parses a whole .scn document. Mirrors the native recovery rules: a failed
// `expect` records an error and does not consume, an unrecognised header key
// or top-level keyword is skipped, and a property key followed by neither '='
// nor '{' is dropped and the scan resumes at the next token.
SceneDocument parse_scene_document(SceneLexer& lexer);
SceneDocument parse_scene_document(const std::string& text);

// The individual block parsers, exposed for targeted tests. Each is called
// with the keyword already consumed.
SceneHeader parse_scene_header_00469bf0(SceneLexer& lexer, std::vector<std::string>& errors);
SceneEntity parse_scene_entity_0046cf40(SceneLexer& lexer, std::vector<std::string>& errors);
std::vector<SceneGroupEntry> parse_scene_groups_00467e10(SceneLexer& lexer,
    std::vector<std::string>& errors);
ScenePropertyBlock parse_scene_property_block_008f5a00(SceneLexer& lexer,
    std::vector<std::string>& errors);

// sscanf("%d") / sscanf("%f") prefix semantics of 008d9ad0 and 008d9b40:
// the longest leading conversion, and failure when there is none. "1.-"
// converts to 1.0 and is accepted, which is why the shipped files that contain
// it still load.
bool scene_scan_int(const std::string& text, std::int32_t& out) noexcept;
bool scene_scan_float(const std::string& text, float& out) noexcept;

// ---------------------------------------------------------------------------
// Scene record fields
// ---------------------------------------------------------------------------

// Offsets into the mission scene record (argument 4 of 0046df00, the same
// record docs/MISSION_SCENE_LOAD.md describes). Only the fields this reader
// touches are listed; the rest of the record belongs to 004dfb70.
struct SceneRecordFields {
    std::int32_t mission_id{0};  // +1098h, written from `uniqueID`
    bool precache_seen{false};   // +905h, set to 1 by a `precache` block
};
// kSceneRecordMissionIdOffset (0x1098) and kSceneRecordScenePathOffset (0x90C)
// are already declared by include/bsp/mission_scene_load.hpp, which recovered
// them from the caller side; only the field this reader adds is defined here.
inline constexpr std::size_t kSceneRecordPrecacheFlagOffset = 0x905;

// The scene database fields 0046df00 fills from its path argument before
// parsing: the full path at +13Ch/+140h and the path with the last extension
// removed at +144h/+148h, both native {length, pointer} string pairs. The
// stem is what the landscape branch formats into "%s_LS_%i.trn".
struct SceneDatabasePath {
    std::string full;  // +13Ch/+140h
    std::string stem;  // +144h/+148h, "" when the path was empty
};
inline constexpr std::size_t kSceneDatabaseFullPathOffset = 0x13C;
inline constexpr std::size_t kSceneDatabaseStemOffset = 0x144;
inline constexpr std::size_t kSceneDatabasePendingListOffset = 0x14C;
inline constexpr std::size_t kSceneDatabaseDeferredListOffset = 0x150;

// Split of 0046df00's prologue: scan back for the last '.', copy the text
// before it into the stem and the text from the '.' onward into the scratch
// buffer at 00e18668. A path with no '.' keeps its whole text as the stem; an
// empty path yields an empty stem.
SceneDatabasePath split_scene_path_0046df00(const std::string& scene_path);

// ---------------------------------------------------------------------------
// Three-pass driver
// ---------------------------------------------------------------------------

// The three calls 004dfb70 and 004d4df0 make, distinguished by arguments 3 and
// 6 of 0046df00 (`param_4` and `param_7` of the export).
//   Header       param_4 = 0, param_7 = 0  -> 004dfb70, before the world exists
//   Registration param_4 = 0, param_7 = 1  -> 004d4df0, first call
//   Instantiate  param_4 = 1, param_7 = 0  -> 004d4df0, second call
enum class SceneFilePass { Header, Registration, Instantiate };

struct SceneFilePassFlags {
    bool instantiate{false};   // param_4
    bool registration{false};  // param_7
};
SceneFilePassFlags scene_pass_flags(SceneFilePass pass) noexcept;
SceneFilePass scene_pass_from_flags(bool instantiate, bool registration) noexcept;

// The three passes agree on the header and differ afterwards:
//  - Header stops after the header block; the entity loop is skipped entirely.
//  - Registration walks entities and stops at the first `traffic`, `groups` or
//    `SceneBrowserGroups` keyword, notifying the host for `traffic` only.
//  - Instantiate walks entities and dispatches all four keywords.
bool scene_pass_runs_entity_loop(SceneFilePass pass) noexcept;
bool scene_pass_stops_at_tail_blocks(SceneFilePass pass) noexcept;

// Integration boundary for everything 0046df00 reaches outside the parser.
// One method per native call site; there are no default implementations,
// because nothing here stands in for unrecovered game behaviour. The VFS,
// resource-manager and scene-graph code these bind to is reconstructed
// elsewhere and is not duplicated here.
struct SceneFileReaderHost {
    virtual ~SceneFileReaderHost() = default;

    // 008d9cf0 opens the path through the VFS provider table at 0109ceec with
    // mode 0x32 and reads the whole stream into memory. Returns false when the
    // provider returned no stream; the native then tokenizes an empty buffer.
    virtual bool read_scene_file(const std::string& path, std::string& out) = 0;

    // 0046a9f0 on database+14Ch: drop every pending deferred-reference node.
    // Runs once at entry of every pass.
    virtual void clear_pending_references() = 0;

    // The weather descriptor walk at 0046df00+0x100..0x5a0. Looks up the scene
    // in the `Weathers` table of SCRIPTS\datatables\Weather.lua, and for the
    // matching entry visits each `SubScenes` row, reading `sceneFile`, `ID`
    // and `Descriptor`. Returns the descriptor name selected for this scene.
    virtual std::string select_weather_descriptor(const std::string& scene_path,
        const std::string& override_name) = 0;

    // With no override name, the selected descriptor's shadow keys are pushed
    // into the four g_Terrain console variables (kSceneTerrainShadowKeys).
    virtual void set_terrain_shadow_string(const std::string& variable,
        const std::string& value) = 0;
    virtual void set_terrain_shadow_float(const std::string& variable, float value) = 0;

    // 00469bf0's `properties` branch: publish the root property bag under the
    // name "SceneRootProps" (00469b60 interns it) and apply it to the record.
    virtual void publish_scene_root_properties(const ScenePropertyBlock& block) = 0;
    // 00469bf0's `precache` branch, which also sets record byte +905h.
    virtual void publish_scene_precache(const ScenePropertyBlock& block) = 0;
    // `uniqueID` -> record +1098h.
    virtual void set_scene_mission_id(std::int32_t mission_id) = 0;

    // 0046cf40's instantiation call 0046c550, given the entity, the world
    // matrix already composed with the parent's, and the pass flags. The
    // factory is selected by the class name through 00468fb0/00468660.
    virtual void instantiate_entity(const SceneEntity& entity, const float world_frame[16],
        SceneFilePass pass) = 0;

    // 009514b0 (`traffic`, instantiate pass) and 0095ca10 (`traffic`,
    // registration pass, which also ends the loop).
    virtual void load_traffic_block(SceneLexer& lexer) = 0;
    virtual void skip_traffic_block(SceneLexer& lexer) = 0;
    // 00925f20, called once before the first tail block of the instantiate
    // pass with CL = 0.
    virtual void begin_tail_blocks() = 0;
    virtual void end_tail_blocks() = 0;
    // 00467e10 and 00469e40 results.
    virtual void load_groups(const std::vector<SceneGroupEntry>& groups) = 0;
    virtual void load_browser_groups(SceneLexer& lexer) = 0;

    // 0046aab0, the deferred-reference resolve that closes the instantiate
    // pass and then empties database+14Ch again.
    virtual void resolve_deferred_references() = 0;
};

struct SceneFileReadResult {
    bool opened{false};
    SceneDocument document;
    SceneDatabasePath path;
    SceneRecordFields record;
    std::size_t entities_visited{0};
    std::string weather_descriptor;
};

// 0046df00 itself, as a routine over the host. `override_name` is argument 5;
// when it is null or empty the terrain shadow variables are written from the
// weather descriptor, otherwise the descriptor is applied wholesale and the
// shadow keys are left alone.
SceneFileReadResult run_scene_file_reader_0046df00(SceneFileReaderHost& host,
    const std::string& scene_path, const std::string& override_name, SceneFilePass pass);

// Convenience for callers that already hold the text (installed-file checks).
using SceneFileTextReader = std::function<bool(const std::string&, std::string&)>;

}  // namespace bsp
